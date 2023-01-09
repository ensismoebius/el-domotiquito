// #include "DHT.h"
#include "index.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "LulaPresidente_ForaCentrao";
const char *passwd = "@@$UP&R4";

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

void getStatus() {
  sendData(Activations::status);
  server.send(200, "text/plane");
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
  server.on("/status", getStatus);
  server.on("/ledToggle", setToggle);
  server.on("/interval", setInterval);
  server.on("/mintime", setMinTime);
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
