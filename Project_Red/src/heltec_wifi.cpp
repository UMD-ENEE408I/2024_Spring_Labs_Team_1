#include "heltec_wifi.hpp"
#include "heltec_espnow.hpp"

// #define TERM


/******************************************************************
 *                          Variables
 ******************************************************************
 */

const char *ssid = "carpeted-kitchen";
const char *password = "czkm443@";

const char *serverName = "192.168.50.186";
const int serverPort = 8090;

// Set web server port number to 80
WiFiServer server(serverPort);

// Client
WiFiClient client;



/******************************************************************
 *                          Functions
 ******************************************************************
 */


/*
 * Initialization
 */


/*
 * Wifi initialization, put in setup()
 */
void wifi_Init() {
  #ifdef TERM
  Serial.print("Connecting to ");
  Serial.print(ssid);
  #endif
  

  // Connect to Wi-Fi network with SSID and password
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    #ifdef TERM
    Serial.print(".");
    #endif
    // WiFi.begin(ssid, password);
  }

  // Print local IP address
  #ifdef TERM
  Serial.println("\n\nWiFi connected");
  Serial.print  ("IP address:  ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  #endif

}


/*
 * Tell server to start listening for clients, put in setup()
 */
void server_Init() {
  server.begin();

  #ifdef TERM
  Serial.print  ("Server created, listening to port ");
  Serial.println(serverPort);
  Serial.println("");
  #endif
}


/*
 * Connect to server, put in setup()
 */
void client_Init() {
  #ifdef TERM
  Serial.print("Connecting to ");
  Serial.print(serverName);
  Serial.print(" at port ");
  Serial.print(serverPort);
  #endif
  

  while(!client.connect(serverName, serverPort)) {
      delay(500);
      #ifdef TERM
      Serial.print(".");
      #endif
  }

  #ifdef TERM
  Serial.println("\nConnected");
  #endif
}

void client_stop() {
  client.stop();
}


/*
 * Communication functions
 */


/*
 * Check for clients and check their connection
 *
 * return
 *   0 - Client unavalible (No client/client disconneced)
 *   1 - Client connected
 */
int client_check() {
  // If there is currently no client, try to find one
  if(!client) {
    client = server.available();    // Listen for incoming clients
    if(!client) return 0;           // return 0 if no clients
    #ifdef TERM
    Serial.println("New client");
    #endif
    
  }

  if(client.connected()) {    // The client is connected
    return 1;
  } else {                    // The client has disconnected
    client.stop();
    #ifdef TERM
    Serial.println("\nClient disconnected\n");
    #endif
    return 0;
  }
}


/*
 * Read data from client
 *
 * return
 *   0 - No data from client
 *   1 - Data avalible
 */
int client_read(String *client_rx_buff) {
    if(client.available()) {    // if there is data to be read
      // clear buffer
      *client_rx_buff = "";

      // read data
      *client_rx_buff = client.readStringUntil('\n');
      client.flush();

      return 1;
    }
    return 0;
}


/*
 * Write data to client
 */
void client_write(String client_tx_buff) {
  client.print(client_tx_buff);
  client.flush();
}


// void handle_wifi() {
//   client_read(&client_rx_buff);

//   switch(client_rx_buff[1]) {
//     case 'm':
//       espMessageDataTx.maze_obsticle = 1;
//       break;
//     case 'd':
//       espMessageDataTx.dualFates_rdy = 1;
//       if(client_rx_buff[2] == 'l') {
//         espMessageDataTx.dualFates_val = 0;
//       } else if(client_rx_buff[2] == 'r') {
//         espMessageDataTx.dualFates_val = 1;
//       } else {
//         //error
//       }
//       break;
//     default:
//       //error
//       break;
//   }

//   switch(client_rx_buff[0]) {
//     case 'r':
//       // handle dual fates
//       break;
//     case 'g':
//       esp_now_send(broadcastAddr_A, (uint8_t *) &espMessageDataTx, sizeof(espMessageDataTx));
//       break;
//     case 'b':
//       esp_now_send(broadcastAddr_S, (uint8_t *) &espMessageDataTx, sizeof(espMessageDataTx));
//       break;
//     default:
//       //error
//       break;
//   } 
//   espMessageDataTx.maze_obsticle = 0;
//   espMessageDataTx.dualFates_rdy = 0;
// }