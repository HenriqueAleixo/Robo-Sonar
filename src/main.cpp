#include <Arduino.h>
#include <AFMotor.h>

// === PINAGEM DO SENSOR ULTRASSÔNICO ===
#define ECHO_PIN A5   // Echo do HC-SR04
#define TRIG_PIN A4   // Trigger do HC-SR04

// === INSTÂNCIAS DOS MOTORES ===
AF_DCMotor motorLeft(2);   // Motor da esquerda (porta M2 do Shield)
AF_DCMotor motorRight(3);  // Motor da direita (porta M3 do Shield)

// === CONSTANTES E VARIÁVEIS GLOBAIS ===
int velocidade = 150;         // Velocidade base dos motores (0-255)
long distanciaLimite = 10;    // Limite em cm para considerar obstáculo
unsigned long tempoRe = 400;  // Tempo (ms) de ré ao detectar obstáculo
unsigned long tempoGiroCheck = 400; // Tempo (ms) para girar e verificar distância
unsigned long tempoGiroFinal = 500; // Tempo (ms) para girar de fato e desviar

// -------------------------------------------------------------------
//  FUNÇÕES DE MOVIMENTO
// -------------------------------------------------------------------
void moverFrente() {
  motorLeft.run(FORWARD);
  motorRight.run(FORWARD);
}

void moverTras() {
  motorLeft.run(BACKWARD);
  motorRight.run(BACKWARD);
}

void pararMotores() {
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
}

void girarEsquerda() {
  motorLeft.run(BACKWARD);
  motorRight.run(FORWARD);
}

void girarDireita() {
  motorLeft.run(FORWARD);
  motorRight.run(BACKWARD);
}

// -------------------------------------------------------------------
//  FUNÇÃO PARA MEDIR DISTÂNCIA COM HC-SR04
// -------------------------------------------------------------------
long medirDistancia() {
  // Garante nível LOW no trigger por pelo menos 2 microssegundos
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Gera pulso de 10 microssegundos no trigger
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Mede o tempo (em microssegundos) do retorno no echo
  long duracao = pulseIn(ECHO_PIN, HIGH);

  // Converte tempo em distância (cm)
  long distancia = (duracao / 2) / 29.1;
  return distancia;
}

// -------------------------------------------------------------------
//  FAZ UMA "LEITURA DIRECIONADA" DA DISTÂNCIA:
//  1) Gira o robô para um lado por X tempo
//  2) Mede a distância
//  3) Volta ao centro
//  Retorna a distância lida.
// -------------------------------------------------------------------
long medirDistanciaAnguloEsquerda() {
  // Gira levemente para esquerda
  girarEsquerda();
  delay(tempoGiroCheck);

  // Para para estabilizar e medir
  pararMotores();
  delay(100);
  
  // Faz a leitura
  long dist = medirDistancia();
  
  // Volta ao centro (girando para a direita pelo mesmo tempo)
  girarDireita();
  delay(tempoGiroCheck);
  pararMotores();

  return dist;
}

long medirDistanciaAnguloDireita() {
  // Gira levemente para direita
  girarDireita();
  delay(tempoGiroCheck);

  // Para para estabilizar e medir
  pararMotores();
  delay(100);

  // Faz a leitura
  long dist = medirDistancia();
  
  // Volta ao centro (girando para a esquerda pelo mesmo tempo)
  girarEsquerda();
  delay(tempoGiroCheck);
  pararMotores();

  return dist;
}

// -------------------------------------------------------------------
//  SETUP
// -------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando Robô Inteligente com Varredura Simples");

  // Configura pinos do HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Configura velocidade dos motores
  motorLeft.setSpeed(velocidade);
  motorRight.setSpeed(velocidade);

  // Usar randomSeed(analogRead(0)) se ainda quiser alguma ação aleatória no futuro
  // randomSeed(analogRead(0));
}

// -------------------------------------------------------------------
//  LOOP PRINCIPAL
// -------------------------------------------------------------------
void loop() {
  // Lê a distância frontal
  long distancia = medirDistancia();

  Serial.print("Distancia frontal: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // Verifica se há obstáculo
  if (distancia > 0 && distancia < distanciaLimite) {
    // 1) Para e recua um pouco
    Serial.println("Obstaculo detectado! Recuando...");
    moverTras();
    delay(tempoRe);

    pararMotores();
    delay(100);

    // 2) Faz varredura: gira levemente à esquerda, mede, gira levemente à direita, mede
    Serial.println("Verificando melhor direcao (Esquerda vs Direita)...");
    long distEsquerda = medirDistanciaAnguloEsquerda();
    long distDireita  = medirDistanciaAnguloDireita();

    Serial.print("Distancia esquerda: ");
    Serial.print(distEsquerda);
    Serial.println(" cm");

    Serial.print("Distancia direita: ");
    Serial.print(distDireita);
    Serial.println(" cm");

    // 3) Decide para onde girar de acordo com a maior distância (ou igual)
    if (distEsquerda > distDireita) {
      Serial.println("Girando para ESQUERDA para desviar");
      girarEsquerda();
    } else {
      Serial.println("Girando para DIREITA para desviar");
      girarDireita();
    }
    delay(tempoGiroFinal); // Executa o giro
    pararMotores();

    // 4) Segue em frente novamente
    Serial.println("Retomando movimento para frente");
    moverFrente();

  } else {
    // Se não há obstáculo, segue normalmente
    moverFrente();
  }

  delay(100); // Ajuste conforme necessidade (frequência de atualização)
}
