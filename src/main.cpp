#include <Arduino.h>
#include <AFMotor.h>

// Cria uma instância para um motor conectado à porta M1 do shield
AF_DCMotor motor(2);
AF_DCMotor motor2(3);


void setup() {
  Serial.begin(9600);
  Serial.println("Motor Shield L298D - Controle de Motor DC");

  // Configura a velocidade inicial do motor
  motor.setSpeed(125); // Velocidade de 0 a 255
  motor2.setSpeed(125); // Velocidade de 0 a 255
}

void loop() {
  Serial.println("Motor girando no sentido horário...");
  motor.run(FORWARD); // Gira o motor no sentido horário
  motor2.run(BACKWARD); // Gira o motor no sentido horário
  delay(2000);
 batatinha firta 123 // faça motor girar 
}
