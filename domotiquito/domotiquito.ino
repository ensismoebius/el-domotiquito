#include <SoftwareSerial.h>

enum Activations {
  status = '0',
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
  debug_off = 'c',
  horas_decorridas = 'd',
  minutos_decorridos = 'e'
};

#define pinoDoSensorDeChuva A0

bool chovendo = false;

bool debug = false;
bool esperandoPraRegar = true;

float horasAteAProximaRega = 1;
float horasDecorridas = 0;

float minutosDeRega = 1;
float minutosDecorridos = 0;

SoftwareSerial ESP_Serial(10, 11);  // RX, TX

inline void debugInfo() {
  Serial.print(esperandoPraRegar ? "Esperando " : "Regando ");

  if (esperandoPraRegar) {
    Serial.print(horasDecorridas);
    Serial.print(" de ");
    Serial.print(horasAteAProximaRega * 60.0 * 60.0 * 1000.0);
  } else {
    Serial.print(minutosDecorridos);
    Serial.print(" de ");
    Serial.print(minutosDeRega * 60.0 * 1000.0);
  }

  Serial.print(" chuva: ");
  Serial.print(analogRead(pinoDoSensorDeChuva));
  Serial.println();
}

inline void setupSwitchesAndOutputs() {
  Serial.begin(9600);
  ESP_Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(2000);
}

inline bool estaChovendo(const uint8_t pinoAnalogico) {
  float valFinal = analogRead(pinoAnalogico);
  return valFinal > 100 ? true : false;
  // return valFinal > 130 ? true : false;
}

void sendData(Activations state, String data = "") {
  if (ESP_Serial.available()) {
    ESP_Serial.write(state);
    ESP_Serial.write(data.c_str());
    ESP_Serial.write(Activations::finalizador);
    delay(10);
  }
}

inline void activate(Activations state, float data = 0) {
  switch (state) {
    case Activations::regar:
      sendData(state);
      horasDecorridas = 0;
      esperandoPraRegar = false;
      digitalWrite(LED_BUILTIN, HIGH);
      break;

    case Activations::esperar:
      sendData(state);
      minutosDecorridos = 0;
      esperandoPraRegar = true;
      digitalWrite(LED_BUILTIN, LOW);
      break;

    case Activations::chuva:
      if (chovendo == false) {
        sendData(state);
        chovendo = true;
      }
      if (abs(horasDecorridas) > (horasAteAProximaRega * 60.0 * 60.0 * 1000.0 * 4)) {
        horasDecorridas -= 1;
      } else {
        horasDecorridas -= 2;
      }
      break;

    case Activations::nao_chuva:
      if (chovendo == true) {
        sendData(state);
        chovendo = false;
      }
      break;

    case Activations::atribuir_minutos_de_rega:
      minutosDeRega = data;
      sendData(state);
      break;

    case Activations::atribuir_horas_ate_proxima_rega:
      horasAteAProximaRega = data;
      sendData(state);
      break;

    case Activations::debug_on:
      sendData(state);
      debug = true;
      break;

    case Activations::debug_off:
      sendData(state);
      debug = false;
      break;
  }
}

void sendState() {

  sendData(chovendo ? Activations::chuva : Activations::nao_chuva);
  sendData(debug ? Activations::debug_on : Activations::debug_off);
  sendData(esperandoPraRegar ? Activations::esperar : Activations::regar);

  sendData(Activations::atribuir_horas_ate_proxima_rega, String(horasAteAProximaRega));
  sendData(Activations::horas_decorridas, String(horasDecorridas));

  sendData(Activations::atribuir_minutos_de_rega, String(minutosDeRega));
  sendData(Activations::minutos_decorridos, String(minutosDecorridos));
}

bool leStringSerial(String &conteudo) {
  char caractere;

  // Enquanto receber algo pela serial
  while (ESP_Serial.available() > 0) {
    // Lê byte da ESP_Serial
    caractere = ESP_Serial.read();
    // Ignora caractere de quebra de linha
    if (caractere != '\0') {
      // Concatena valores
      conteudo.concat(caractere);
    }
    // Aguarda buffer ESP_Serial ler próximo caractere
    delay(10);
  }

  return conteudo.length() > 0;
}

void executeActivation() {
  String expr;

  if (leStringSerial(expr)) {
    char command = expr.substring(0, 1)[0];
    float params = expr.substring(1).toFloat();

    Serial.println(command);
    Serial.println(params);

    activate(command, params);
  }
}

void setup() {
  setupSwitchesAndOutputs();
  activate(Activations::debug_off);
}

void loop() {
  if (esperandoPraRegar) {
    horasDecorridas++;
    if (horasDecorridas > horasAteAProximaRega * 60.0 * 60.0 * 1000.0) {
      activate(Activations::regar);
    }
  } else {
    minutosDecorridos++;
    if (minutosDecorridos > minutosDeRega * 60.0 * 1000.0) {
      activate(Activations::esperar);
    }
  }

  // Está chovendo?
  if (estaChovendo(pinoDoSensorDeChuva)) {
    activate(Activations::chuva);
  } else {
    activate(Activations::nao_chuva);
  }

  executeActivation();

  if (debug) {
    debugInfo();
  }

  delay(1);
}