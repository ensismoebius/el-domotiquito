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
  while (!ConexaoComESP)
    ;
  Serial.begin(TAXA_DE_TRANSFERENCIA_SERIAL);
  while (!Serial)
    ;
  pinMode(LED_BUILTIN, OUTPUT);
  delay(100);
}

inline bool estaChovendo() {
  return analogRead(PINO_DO_SENSOR_DE_CHUVA) > NIVEL_LIMITE_DE_UMIDADE ? true : false;
}

void enviarParaOEsp(String dados = "") {

  while (!ConexaoComESP.available()){}
  ConexaoComESP.write(dados.c_str());
}

inline void ativar(Ativacoes ativacao, float dados = 0) {

  switch (ativacao) {
    case Ativacoes::status:
      enviarEstadoParaOEsp();
      break;

    case Ativacoes::regar:
      horasDecorridas = 0;
      esperandoPraRegar = false;
      digitalWrite(LED_BUILTIN, HIGH);
      break;

    case Ativacoes::esperar:
      minutosDecorridos = 0;
      esperandoPraRegar = true;
      digitalWrite(LED_BUILTIN, LOW);
      break;

    case Ativacoes::chuva:
      if (chovendo == false) {
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
        chovendo = false;
      }
      break;

    case Ativacoes::atribuir_minutos_de_rega:
      minutosDeRega = dados;
      break;

    case Ativacoes::atribuir_horas_ate_proxima_rega:
      horasAteAProximaRega = dados;
      break;

    case Ativacoes::debug_on:
      debug = true;
      break;

    case Ativacoes::debug_off:
      debug = false;
      break;
  }
}

void enviarEstadoParaOEsp() {

  String informacao = "";

  informacao.concat(chovendo ? char(Ativacoes::chuva) : char(Ativacoes::nao_chuva));
  informacao.concat(FINALIZADOR_DE_STRING_PARA_REDE);

  informacao.concat(debug ? char(Ativacoes::debug_on) : char(Ativacoes::debug_off));
  informacao.concat(FINALIZADOR_DE_STRING_PARA_REDE);

  informacao.concat(esperandoPraRegar ? char(Ativacoes::esperar) : char(Ativacoes::regar));
  informacao.concat(FINALIZADOR_DE_STRING_PARA_REDE);

  informacao.concat(char(Ativacoes::atribuir_horas_ate_proxima_rega));
  informacao.concat(String(horasAteAProximaRega));
  informacao.concat(FINALIZADOR_DE_STRING_PARA_REDE);

  informacao.concat(char(Ativacoes::horas_decorridas));
  informacao.concat(String(horasDecorridas));
  informacao.concat(FINALIZADOR_DE_STRING_PARA_REDE);

  informacao.concat(char(Ativacoes::atribuir_minutos_de_rega));
  informacao.concat(String(minutosDeRega));
  informacao.concat(FINALIZADOR_DE_STRING_PARA_REDE);

  informacao.concat(char(Ativacoes::minutos_decorridos));
  informacao.concat(String(minutosDecorridos));
  informacao.concat('\n');

  enviarParaOEsp(informacao);
}

bool lerDadosDoEsp(String &conteudo) {
  if (ConexaoComESP.available() > 0) {
    conteudo = ConexaoComESP.readStringUntil('\0');
  }
  return conteudo.length() > 0;
}

void consultarEExecutarAtivacao() {
  String dadosRecebidosDoESP;

  if (lerDadosDoEsp(dadosRecebidosDoESP)) {
    char comando = dadosRecebidosDoESP.substring(0, 1)[0];
    float parametros = dadosRecebidosDoESP.substring(1).toFloat();

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