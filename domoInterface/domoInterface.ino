// #include "DHT.h"
#include "index.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "LulaPresidente_ForaCentrao";
const char *passwd = "@@$UP&R4";

enum Activations {
  led_ligado = '1',
  led_desligado = '2',
  conectando = '3',
  conectado = '4',
  regar = '5',
  esperar = '6',
  chuva = '7',
  nao_chuva = '8',
  atribuir_minutos_de_rega = '9',
  atribuir_horas_ate_proxima_rega = 'a',
  finalizador = '\0',
  debug_on = 'b',
  debug_off = 'c'
};

ESP8266WebServer server(80);

void sendData(Activations state, String data = "") {
  Serial.write(state);
  Serial.write(data.c_str());
  Serial.write(Activations::finalizador);
  delay(10);
}

void handleRoot() {
  String s = webpage;
  server.send(200, "text/html", s);
}

void setToggle() {
  String status = server.arg("state");

  if (status == "true") {
    sendData(Activations::regar);
    server.send(200, "text/plane", "Regando");
  } else {
    sendData(Activations::esperar);
    server.send(200, "text/plane", "Esperando");
  }
}

void setInterval() {
  int intervalInHours = server.arg("hours").toInt();
  String stateMessage = String(intervalInHours);

  server.send(200, "text/plane", stateMessage);
  sendData(Activations::atribuir_horas_ate_proxima_rega, stateMessage);
}

void setMinTime() {
  int timeInMinutes = server.arg("minutes").toInt();
  String stateMessage = String(timeInMinutes);

  server.send(200, "text/plane", stateMessage);
  sendData(Activations::atribuir_minutos_de_rega, stateMessage);
}

void getActivations() {
  server.send(200, "text/plane", "Regando: 1 Proxima rega: 2h");
}

inline void setupSerialOutPut() {
  Serial.begin(9600);
}

inline void setupWifi() {
  sendData(Activations::conectando);

  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }

  sendData(Activations::conectado);
}

inline void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/ledToggle", setToggle);
  server.on("/interval", setInterval);
  server.on("/mintime", setMinTime);
  server.on("/Activations", getActivations);
  server.begin();
}

inline void setupLeds() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  setupSerialOutPut();
  setupLeds();
  setupWifi();
  setupWebServer();
}

void loop() {
  server.handleClient();
  delay(1);
}
