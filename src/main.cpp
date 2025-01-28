#include <Arduino.h>
#include <AFMotor.h>

// === PINAGEM DO SENSOR ULTRASSÔNICO ===
#define ECHO_PIN A5   // Echo do HC-SR04 (Echo)
#define TRIG_PIN A4   // Trigger do HC-SR04 (Trigger)

// === INSTÂNCIAS DOS MOTORES (Adafruit Motor Shield) ===
AF_DCMotor motorLeft(2);   // Motor da esquerda (porta M2 do Shield)
AF_DCMotor motorRight(3);  // Motor da direita (porta M3 do Shield)

// === VELOCIDADES DE CADA RODA (0-255) ===
int velocidadeEsquerda = 60;
int velocidadeDireita  = 80;

// === PARÂMETROS DE DESVIO ===
long distanciaLimite = 10;       // Em cm - se o obstáculo estiver mais perto que isso, desviar
unsigned long tempoRe = 500;     // (ms) tempo de ré ao detectar obstáculo
unsigned long tempoGiroVarredura = 600; // (ms) tempo para girar ao “examinar” um lado
unsigned long tempoGiroDesvio    = 800; // (ms) tempo para girar de fato ao desviar

// -------------------------------------------------------------------
//  FUNÇÕES DE MOVIMENTO
// -------------------------------------------------------------------
void moverFrente() {
  motorLeft.setSpeed(velocidadeEsquerda);
  motorRight.setSpeed(velocidadeDireita);

  motorLeft.run(FORWARD);
  motorRight.run(FORWARD);
}

void moverTras() {
  motorLeft.setSpeed(velocidadeEsquerda);
  motorRight.setSpeed(velocidadeDireita);

  motorLeft.run(BACKWARD);
  motorRight.run(BACKWARD);
}

void pararMotores() {
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
}

void girarEsquerda() {
  motorLeft.setSpeed(velocidadeEsquerda);
  motorRight.setSpeed(velocidadeDireita);

  motorLeft.run(BACKWARD);
  motorRight.run(FORWARD);
}

void girarDireita() {
  motorLeft.setSpeed(velocidadeEsquerda);
  motorRight.setSpeed(velocidadeDireita);

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
//  FUNÇÃO PARA "OLHAR" UM LADO E RETORNAR DISTÂNCIA (COM SENSOR INVERTIDO)
// -------------------------------------------------------------------
long medirDistanciaAnguloEsquerda() {
  // Como o sensor está invertido, girar para a "esquerda" será na prática girar para a DIREITA
  Serial.println(">> Girando para ESQUERDA para varredura (sensor invertido)");
  girarDireita(); // Inverte o lado
  delay(tempoGiroVarredura);

  pararMotores();
  delay(100);  // Pequena pausa para estabilizar a leitura

  long dist = medirDistancia();
  Serial.print("   Distancia (varrendo E): ");
  Serial.print(dist);
  Serial.println(" cm");

  // Volta ao centro girando para o lado oposto (esquerda)
  Serial.println(">> Retornando ao centro...");
  girarEsquerda(); // Inverte o retorno
  delay(tempoGiroVarredura);
  pararMotores();
  
  return dist;
}

long medirDistanciaAnguloDireita() {
  // Como o sensor está invertido, girar para a "direita" será na prática girar para a ESQUERDA
  Serial.println(">> Girando para DIREITA para varredura (sensor invertido)");
  girarEsquerda(); // Inverte o lado
  delay(tempoGiroVarredura);

  pararMotores();
  delay(100);  // Pequena pausa para estabilizar a leitura

  long dist = medirDistancia();
  Serial.print("   Distancia (varrendo D): ");
  Serial.print(dist);
  Serial.println(" cm");

  // Volta ao centro girando para o lado oposto (direita)
  Serial.println(">> Retornando ao centro...");
  girarDireita(); // Inverte o retorno
  delay(tempoGiroVarredura);
  pararMotores();
  
  return dist;
}

// -------------------------------------------------------------------
//  SETUP
// -------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando Robô com Sensor Ultrassônico Invertido");

  // Configura pinos do HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Ajusta a velocidade inicial dos motores
  motorLeft.setSpeed(velocidadeEsquerda);
  motorRight.setSpeed(velocidadeDireita);
}

// -------------------------------------------------------------------
//  LOOP PRINCIPAL
// -------------------------------------------------------------------
void loop() {
  // Lê a distância frontal
  long distanciaFrontal = medirDistancia();

  Serial.print("Distancia frontal: ");
  Serial.print(distanciaFrontal);
  Serial.println(" cm");

  // Se a distância frontal for menor que o limite, faz o desvio
  if (distanciaFrontal > 0 && distanciaFrontal < distanciaLimite) {
    Serial.println("Obstáculo detectado! Recuando...");
    
    // 1) RECUA
    moverTras();
    delay(tempoRe);

    pararMotores();
    delay(100);

    // 2) VARREDURA ESQUERDA / DIREITA
    Serial.println("Verificando melhor direção (Esquerda vs Direita)...");
    long distEsquerda = medirDistanciaAnguloEsquerda();
    long distDireita  = medirDistanciaAnguloDireita();

    // 3) DECISÃO
    if (distEsquerda > distDireita) {
      Serial.println("==> Mais livre à ESQUERDA. Desviando para ESQUERDA...");
      girarEsquerda();
    } else {
      Serial.println("==> Mais livre à DIREITA (ou igual). Desviando para DIREITA...");
      girarDireita();
    }
    delay(tempoGiroDesvio);
    pararMotores();

    // 4) SEGUE EM FRENTE
    Serial.println("Obstáculo contornado. Seguindo em frente.");
    moverFrente();

  } else {
    // Caso não haja obstáculo, continua para frente
    moverFrente();
  }

  delay(100); // Frequência de leitura
}
