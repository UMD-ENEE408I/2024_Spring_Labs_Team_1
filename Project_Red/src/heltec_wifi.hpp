#ifndef HELTEC_WIFI_H_
#define HELTEC_WIFI_H_

#include <WiFi.h>


/******************************************************************
 *                          Variables
 ******************************************************************
 */

// Server at specified port 
extern WiFiServer server;

// Client
extern WiFiClient client;



/******************************************************************
 *                          Protoypes
 ******************************************************************
 */

// Wifi and server initialization, put in setup() after esp-now init
void wifi_Init();
void server_Init();
void client_Init();
void client_stop();


/*
 * Check for clients and check their connection
 *
 * return
 *   0 - Client unavalible (No client/client disconneced)
 *   1 - Client connected
 */
int client_check();

/*
 * Read data from client
 *
 * return
 *   0 - No data from client
 *   1 - Data avalible
 */
int client_read(String *client_rx_buff);

/*
 * Write data to client
 */
void client_write(String client_tx_buff);



#endif