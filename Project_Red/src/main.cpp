#include <Arduino.h>
#include <Adafruit_MCP3008.h>
#include <Adafruit_MPU6050.h>
#include <Encoder.h>

#include "movement.hpp"
#include "transition.hpp"
#include "heltec_espnow.hpp"
#include "heltec_wifi.hpp"

int state;
String client_rx_buff = "";

void setup() {
  Serial.begin(115200);

  movement_Init();
  // heltec_espnow_Init();
  wifi_Init();

  M1_stop();
  M2_stop();

  state = 0;

  delay(100);
}


void loop() {
  Encoder enc1(M1_ENC_A, M1_ENC_B);
  Encoder enc2(M2_ENC_A, M2_ENC_B);

  delay(2000);
  
  // TODO wait for go from computer

  while(true) {
    transition = 0;
    
    if(state == 0) { // Line of the Republic ---------------------------

      // go onto line
      do {
        updateMoveForwardPID(true);
        delay(10);
        readLineSensor();
      } while(lineArray[0] == 1 || lineArray[12] == 1);


      // line follow
      do {
        delay(10);
        updateLineFollow(10);
      } while(lineArray[12] == 0 && lineArray[0] == 0);
      M1_stop();
      M2_stop();
      delay(100);


      // transition
      transition_right(350);
      state = 1;
      delay(100);
      // tell next robot to go?


    } else if(state == 1) { // Maze of Mandalore ----------------------
      
      // find maze entrance
      do {
        delay(10);
        updateLineFollow(-10);
      } while(lineArray[0] == 0 || lineArray[6] == 0);
      M1_stop();
      M2_stop();
      delay(100);


      // skip maze
      moveForwardDist(false, 400);
      delay(100);
      
      turnAngle(-95);
      delay(100);
      
      moveForwardDist(true, 2600);
      delay(100);

      
      // find line again
      turnAngle(90);
      delay(100);

      do {
        updateMoveForwardPID(true);
        delay(10);
        readLineSensor();
      } while(lineArray[6] != 1);
      M1_stop();
      M2_stop();
      delay(100);
      
      moveForwardDist(true, 250);
      delay(100);

      turnAngle(-90);
      delay(100);


      // finish section
      do {
        delay(10);
        updateLineFollow(-10);
      } while(lineArray[12] == 0 || lineArray[0] == 0);
      M1_stop();
      M2_stop();
      delay(100);


      // transition
      transition_left(500);
      state = 2;
      delay(100);


    } else if(state == 2) { // Kessel Run -----------------------------

      do {
        delay(10);
        updateLineFollow(-20);
      } while(lineArray[12] == 0 || lineArray[0] == 0);
      M1_stop();
      M2_stop();
      delay(100);


      // transition
      transition_left(500);
      state = 3;
      delay(100);


    } else if(state == 3) { // Hoth Asteroid Field ---------------------

      // get parallel to line and skip asteroid field
      turnAngle(-90);
      delay(100);

      moveForwardDist(true, 600);
      delay(100);

      turnAngle(80);
      delay(100);

      moveForwardDist(true, 2350);
      delay(100);
      

      // find line again
      turnAngle(90);
      delay(100);

      do {
        updateMoveForwardPID(true);
        delay(10);
        readLineSensor();
      } while(lineArray[6] != 1);
      M1_stop();
      M2_stop();
      delay(100);

      moveForwardDist(true, 200);
      delay(100);

      turnAngle(-95);
      delay(100);


      // finish section
      do {
        delay(10);
        updateLineFollow(-10);
      } while(lineArray[12] == 0 && lineArray[0] == 0);
      M1_stop();
      M2_stop();
      delay(100);


      // transition
      transition_right(550);
      state = 4;
      delay(100);


    } else if(state == 4) { // Path of Dual Fates ---------------------

      // get in place
      do {
        delay(10);
        updateLineFollow(0);
      } while(lineArray[0] == 0 || lineArray[6] == 0);
      turnAngle(-90);
      delay(100);

      do {
        delay(10);
        updateLineFollow(-5);
      } while(lineArray[12] == 0 && lineArray[0] == 0);
      M1_stop();
      M2_stop();
      delay(100);


      // TODO tell computer to listen
      moveForwardDist(true, 200);
      delay(100);
      turnAngle(90);
      delay(100);

      do {
        delay(10);
        updateLineFollow(0);
      } while(lineArray[0] == 0);
      M1_stop();
      M2_stop();
      client_Init();
      String client_tx_buff = "Dual fates left";
      client_write(client_tx_buff);
      client_stop();
      delay(7000);    

      turnAngle(-180);
      delay(100);

      do {
        delay(10);
        updateLineFollow(-5);
      } while(lineArray[12] == 0);
      M1_stop();
      M2_stop();
      client_Init();
      client_tx_buff = "Dual fates right";
      client_write(client_tx_buff);
      client_stop();
      delay(7000);


      client_Init();
      client_tx_buff = "Dual fates result";
      client_write(client_tx_buff);
      while(!(client_read(&client_rx_buff))) delay(100);
      client_stop();


      if (!client_rx_buff.compareTo("Left")) {           // left
        turnAngle(180);
        delay(100);

        do {
          delay(10);
          updateLineFollow(0);
        } while(lineArray[0] == 0);
        turnAngle(-90);
        delay(100);

        do {
          delay(10);
          updateLineFollow(-5);
        } while(lineArray[0] == 0);
        turnAngle(-90);
        delay(100);

        do {
          delay(10);
          updateLineFollow(-5);
        } while(lineArray[12] == 0);
        turnAngle(90);
        delay(100);


      } else {          // right
        moveForwardDist(true, 150);
        turnAngle(90);
        delay(100);

        do {
          delay(10);
          updateLineFollow(-5);
        } while(lineArray[12] == 0);
        turnAngle(90);
        delay(100);

        do {
          delay(10);
          updateLineFollow(-5);
        } while(lineArray[0] == 0);
        turnAngle(-90);
        delay(100);

      }


      // follow line to end
      do {
        updateLineFollow(-5);
        delay(10);
      } while(lineArray[12] == 0 || lineArray[0] == 0);
      M1_stop();
      M2_stop();
      delay(100);


      // transtion
      transition_left(500);
      state = 5;
      delay(100);


    } else if(state == 5) { // Second Line of the Republic ------------

      // line follow
      do {
        updateLineFollow(-10);
        delay(10);
      } while(lineArray[12] == 0 || lineArray[0] == 0);
      M2_stop();
      delay(10);
      M1_stop();
      delay(100);


      // transition
      transition_left(500);
      state = 6;


    } else if(state == 6) { // Endor Dash ----------------------------
     
      // go straight
      do {
        updateMoveForwardPID(true);
        delay(10);
        readLineSensor();
      } while(lineArray[12] == 0 || lineArray[0] == 0);


      // center on box
      moveForwardDist(true, 250);
      delay(100);

      // if(lineArray[12] == 0) {
      //   turnAngle(-90);
      //   delay(100);
      //   moveForwardDist(true, 100);
      //   delay(100);
      // } else if(lineArray[0] == 0) {
      //   turnAngle(90);
      //   delay(100);
      //   moveForwardDist(true, 100);
      //   delay(100);
      // }


      // transition
      state = 7;


    } else { // Finish -----------------------------------------------
      while(1) {
        turnAngle(-90);
      }
    }
  }
}