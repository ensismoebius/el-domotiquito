#include "types.h"
#include "config.h"
#include <SoftwareSerial.h>

bool debug = false;
bool chovendo = false;
bool esperandoPraRegar = true;

float horasAteAProximaRega = 1;
float horasDecorridas = 0;

float minutosDeRega = 1;
float minutosDecorridos = 0;

SoftwareSerial ConexaoComESP(RX, TX);

inline void mostrarInfoDeDebug() {
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
  Serial.print(analogRead(PINO_DO_SENSOR_DE_CHUVA));
  Serial.println();
}

inline void configurarDispositivosDeSaida() {
  ConexaoComESP.begin(TAXA_DE_TRANSFERENCIA_SERIAL);
  Serial.begin(TAXA_DE_TRANSFERENCIA_SERIAL);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(100);
}

inline bool estaChovendo() {
  return analogRead(PINO_DO_SENSOR_DE_CHUVA) > NIVEL_LIMITE_DE_UMIDADE ? true : false;
}

void enviarParaOEsp(Ativacoes ativacao, String dados = "") {
  if (ConexaoComESP.available()) {
    ConexaoComESP.write(ativacao);
    ConexaoComESP.write(dados.c_str());
    ConexaoComESP.write(FINALIZADOR_DE_STRING);
    delay(10);
  }
}

inline void ativar(Ativacoes ativacao, float dados = 0) {
  switch (ativacao) {
    case Ativacoes::regar:
      enviarParaOEsp(ativacao);
      horasDecorridas = 0;
      esperandoPraRegar = false;
      digitalWrite(LED_BUILTIN, HIGH);
      break;

    case Ativacoes::esperar:
      enviarParaOEsp(ativacao);
      minutosDecorridos = 0;
      esperandoPraRegar = true;
      digitalWrite(LED_BUILTIN, LOW);
      break;

    case Ativacoes::chuva:
      if (chovendo == false) {
        enviarParaOEsp(ativacao);
        chovendo = true;
      }
      if (abs(horasDecorridas) > (horasAteAProximaRega * 60.0 * 60.0 * 1000.0 * 4)) {
        horasDecorridas -= 1;
      } else {
        horasDecorridas -= 2;
      }
      break;

    case Ativacoes::nao_chuva:
      if (chovendo == true) {
        enviarParaOEsp(ativacao);
        chovendo = false;
      }
      break;

    case Ativacoes::atribuir_minutos_de_rega:
      minutosDeRega = dados;
      enviarParaOEsp(ativacao);
      break;

    case Ativacoes::atribuir_horas_ate_proxima_rega:
      horasAteAProximaRega = dados;
      enviarParaOEsp(ativacao);
      break;

    case Ativacoes::debug_on:
      enviarParaOEsp(ativacao);
      debug = true;
      break;

    case Ativacoes::debug_off:
      enviarParaOEsp(ativacao);
      debug = false;
      break;
  }
}

void enviarEstadoParaOEsp() {

  enviarParaOEsp(chovendo ? Ativacoes::chuva : Ativacoes::nao_chuva);
  enviarParaOEsp(debug ? Ativacoes::debug_on : Ativacoes::debug_off);
  enviarParaOEsp(esperandoPraRegar ? Ativacoes::esperar : Ativacoes::regar);

  enviarParaOEsp(Ativacoes::atribuir_horas_ate_proxima_rega, String(horasAteAProximaRega));
  enviarParaOEsp(Ativacoes::horas_decorridas, String(horasDecorridas));

  enviarParaOEsp(Ativacoes::atribuir_minutos_de_rega, String(minutosDeRega));
  enviarParaOEsp(Ativacoes::minutos_decorridos, String(minutosDecorridos));
}

bool lerDadosDoEsp(String &conteudo) {
  char caractere;

  // Enquanto receber algo pela serial
  while (ConexaoComESP.available() > 0) {
    
    // Lê byte da ConexaoComESP
    caractere = ConexaoComESP.read();
    conteudo.concat(caractere);

    // Para a leitura quando encontrar
    // um finalizador de string
    if (caractere != '\0') {
      break;
    }
    // Aguarda buffer ConexaoComESP ler o próximo caractere
    delay(10);
  }

  return conteudo.length() > 0;
}

void consultarEExecutarAtivacao() {
  String dadosRecebidosDoESP;

  if (lerDadosDoEsp(dadosRecebidosDoESP)) {
    char comando = dadosRecebidosDoESP.substring(0, 1)[0];
    float parametros = dadosRecebidosDoESP.substring(1).toFloat();

    Serial.println(comando);
    Serial.println(parametros);

    ativar(comando, parametros);
  }
}

void setup() {
  configurarDispositivosDeSaida();
  ativar(Ativacoes::debug_off);
}

void loop() {
  if (esperandoPraRegar) {
    horasDecorridas++;
    if (horasDecorridas > horasAteAProximaRega * 60.0 * 60.0 * 1000.0) {
      ativar(Ativacoes::regar);
    }
  } else {
    minutosDecorridos++;
    if (minutosDecorridos > minutosDeRega * 60.0 * 1000.0) {
      ativar(Ativacoes::esperar);
    }
  }

  if (estaChovendo()) {
    ativar(Ativacoes::chuva);
  } else {
    ativar(Ativacoes::nao_chuva);
  }

  consultarEExecutarAtivacao();

  if (debug) {
    mostrarInfoDeDebug();
  }

  delay(1);
}