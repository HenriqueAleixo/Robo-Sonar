#include <Arduino.h>
#include <AFMotor.h>

// === PINAGEM DO SENSOR ULTRASSÔNICO ===
#define ECHO_PIN A5   // Echo do HC-SR04
#define TRIG_PIN A4   // Trigger do HC-SR04

// === INSTÂNCIAS DOS MOTORES ===
// Estão ligados nas saídas M2 e M3 do Shield (exemplo), mas ajuste conforme seu hardware
AF_DCMotor motorLeft(2);  // Motor conectado à porta M2
AF_DCMotor motorRight(3); // Motor conectado à porta M3

// === CONSTANTES E VARIÁVEIS ===
int velocidade = 150;       // Velocidade dos motores (0-255)
long distanciaLimite = 5;  // Distância limite em cm para considerar obstáculo
unsigned long tempoGiro = 500; // Tempo (ms) para fazer o giro de desvio

// Função para medir distância em cm usando o HC-SR04
long medirDistancia() {
  // Garante que o Trigger esteja em LOW por um curto tempo
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Envia pulso de 10 microssegundos para o Trigger
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Mede o tempo do pulso retornado no Echo
  long duracao = pulseIn(ECHO_PIN, HIGH);

  // Converte o tempo (em microssegundos) em distância (em cm)
  // Fórmula aproximada: distância em cm = (tempo / 2) / 29.1
  // (pois a velocidade do som é ~340 m/s, e há ida e volta)
  long distancia = (duracao / 2) / 29.1;
  
  return distancia;
}

// Função para mover os motores para frente
void moverFrente() {
  motorLeft.run(FORWARD);
  motorRight.run(FORWARD);
}

// Função para mover os motores para trás
void moverTras() {
  motorLeft.run(BACKWARD);
  motorRight.run(BACKWARD);
}

// Função para parar os motores
void pararMotores() {
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
}

// Função para girar à esquerda
void girarEsquerda() {
  motorLeft.run(BACKWARD);
  motorRight.run(FORWARD);
}

// Função para girar à direita
void girarDireita() {
  motorLeft.run(FORWARD);
  motorRight.run(BACKWARD);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando Robo com Desvio Ultrassonico");

  // Configura pinos do HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Configura a velocidade dos motores
  motorLeft.setSpeed(velocidade);
  motorRight.setSpeed(velocidade);

  // Semente para gerar valores aleatórios (opcional, caso queira maior variação)
  randomSeed(analogRead(0));
}

void loop() {
  long distancia = medirDistancia();
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // Verifica se há obstáculo próximo
  if (distancia > 0 && distancia < distanciaLimite) {
    // Obstáculo detectado: anda para trás
    moverTras();
    delay(600); // Ajuste esse tempo conforme necessidade

    // Escolhe aleatoriamente se vai girar à esquerda (0) ou à direita (1)
    int direcao = random(0, 2);
    if (direcao == 0) {
      Serial.println("Virando ESQUERDA...");
      girarEsquerda();
    } else {
      Serial.println("Virando DIREITA...");
      girarDireita();
    }
    
    delay(tempoGiro); // Gira pelo tempo definido
    
    // Retoma movimento para frente
    moverFrente();
  } else {
    // Se não há obstáculo, segue em frente
    moverFrente();
  }

  delay(100); // Ajuste para controlar a frequência de leitura do sensor
}
