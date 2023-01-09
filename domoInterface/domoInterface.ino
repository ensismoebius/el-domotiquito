#include "index.h"
#include "types.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void enviarParaOArduino(Ativacoes ativacao, String dados = "") {
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

void recuperarStatus() {
  enviarParaOArduino(Ativacoes::status);
  server.send(200, "text/plane", "Regando: 1 Proxima rega: 2h");
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
  server.on("/status", recuperarStatus);
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
