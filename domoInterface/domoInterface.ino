#include "index.h"
#include "types.h"
#include "config.h"
#include <string>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void enviarParaOArduino(Ativacoes ativacao, String dados = "") {
  while (!Serial.availableForWrite()) {}

  Serial.write(ativacao);
  Serial.write(dados.c_str());
  Serial.write(FINALIZADOR_DE_STRING);

  delay(10);
}

void abrirPaginaPrincipal() {
  String html = paginaInicial;
  server.send(200, "text/html", html);
}

void alternarDebug() {
  String parametro = server.arg("debug");

  if (parametro == "true") {
    enviarParaOArduino(Ativacoes::debug_on);
    server.send(200, "text/plane", "Debug ativado");
  } else {
    enviarParaOArduino(Ativacoes::debug_off);
    server.send(200, "text/plane", "Debug desativado");
  }
}

void alternarRega() {
  String parametro = server.arg("ativacao");

  if (parametro == "true") {
    enviarParaOArduino(Ativacoes::regar);
    server.send(200, "text/plane", "Regando");
  } else {
    enviarParaOArduino(Ativacoes::esperar);
    server.send(200, "text/plane", "Esperando");
  }
}

void atribuirIntervaloEmHoras() {
  int intervaloEmHoras = server.arg("hours").toInt();
  String parametro = String(intervaloEmHoras);

  enviarParaOArduino(Ativacoes::atribuir_horas_ate_proxima_rega, parametro);
  server.send(200, "text/plane", parametro);
}

void atribuirTempoDeRegaEmMinutos() {
  int tempoEmMinutos = server.arg("minutes").toInt();
  String parametro = String(tempoEmMinutos);

  enviarParaOArduino(Ativacoes::atribuir_minutos_de_rega, parametro);
  server.send(200, "text/plane", parametro);
}

void solicitarStatus() {
  enviarParaOArduino(Ativacoes::status);
}

inline void configurarSaidaSerial() {
  Serial.begin(TAXA_DE_TRANSFERENCIA_SERIAL);
}

inline void configurarWifi() {
  enviarParaOArduino(Ativacoes::conectando);

  WiFi.begin(SSID, PASSWD);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }

  enviarParaOArduino(Ativacoes::conectado);
}

inline void configurarServidorWeb() {
  server.on("/", abrirPaginaPrincipal);
  server.on("/status", solicitarStatus);
  server.on("/ledToggle", alternarRega);
  server.on("/alternarDebug", alternarDebug);
  server.on("/interval", atribuirIntervaloEmHoras);
  server.on("/mintime", atribuirTempoDeRegaEmMinutos);
  server.begin();
}

inline void configurarPinosDeSaida() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  configurarSaidaSerial();
  configurarPinosDeSaida();
  configurarWifi();
  configurarServidorWeb();
}

void loop() {
  server.handleClient();
}

String* split(String& v, char delimiter, int& length) {
  length = 1;
  bool found = false;

  // Figure out how many itens the array should have
  for (int i = 0; i < v.length(); i++) {
    if (v[i] == delimiter) {
      length++;
      found = true;
    }
  }

  // If the delimiter is found than create the array
  // and split the String
  if (found) {

    // Create array
    String* result = new String[length];

    // Split the string into array
    int i = 0;
    for (int itemIndex = 0; itemIndex < length; itemIndex++) {
      for (; i < v.length(); i++) {

        if (v[i] == delimiter) {
          i++;
          break;
        }
        result[itemIndex] += v[i];
      }
    }

    // Done, return the values
    return result;
  }

  // No delimiter found
  return nullptr;
}

void serialEvent() {

  bool stringComplete = false;
  String inputString = "";
  String final = "{";

  if (Serial.available()) {
    inputString = Serial.readStringUntil('\n');
    stringComplete = true;
  } 

  if (stringComplete) {

    int qtde;
    String* t = split(inputString, char(FINALIZADOR_DE_STRING_PARA_REDE), qtde);

    for (int i = 0; i < qtde; i++) {

      char comando = t[i][0];
      String param = t[i].substring(1);

      switch (comando) {
        case Ativacoes::regar:
          final += "rega: true,";
          break;
        case Ativacoes::esperar:
          final += "rega: false,";
          break;
        case Ativacoes::chuva:
          final += "chuva: true,";
          break;
        case Ativacoes::nao_chuva:
          final += "chuva: false,";
          break;
        case Ativacoes::atribuir_minutos_de_rega:
          final += "minutosDeRega:" + param + ",";
          break;
        case Ativacoes::atribuir_horas_ate_proxima_rega:
          final += "horaDaProximaRega:" + param + ",";
          break;
        case Ativacoes::debug_on:
          final += "debug: true,";
          break;
        case Ativacoes::debug_off:
          final += "debug: false,";
          break;
        case Ativacoes::horas_decorridas:
          final += "horasDecorridas:" + param + ",";
          break;
        case Ativacoes::minutos_decorridos:
          final += "minutosDecorridos:" + param + ",";
          break;
      }
    }
    final += "}";
    delete[] t;
    server.send(200, "text/plane", final);
    final = "";
    inputString = "";
    stringComplete = false;
  }
}
