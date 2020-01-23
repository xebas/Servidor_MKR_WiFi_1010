/*
 Servidor Wifi con activacion de PINs O Leds  tarjeta MKR WiFi 1010
 https://content.arduino.cc/assets/Pinout-MKRwifi1010_latest.png

  Crea un servidor Web , donde podemos activar diferentes LEDs desde un
  navegador , por el puerto serie nos reporta informacion de la conexion
  

  A simple web server that lets you blink an LED via the web.
  This sketch will create a new access point (with no password).
  It will then launch a new server and print out the IP address
  to the Serial monitor. From there, you can open that address in a web browser
  to turn on and off the LED on pin 13.

  If the IP address of your board is yourAddress:
    http://yourAddress/H turns the LED on
    http://yourAddress/L turns it off

  created 25 Nov 2012
  by Tom Igoe
  adapted to WiFi AP by Adafruit
 */

#include <SPI.h>
#include <WiFiNINA.h>
#include "salidas.h" //Funciones para la gestion de las salida ,LEDs

//Defino nombre del servidor y password
#include "arduino_secrets.h"

//IP del servidor wifi , ip del navegador
//#define IP 192,168,6,1

//Recupera el nombre del servidor SSID y el password wifi
char ssid[] = SECRET_SSID;        // nombre servidor SSID
char pass[] = SECRET_PASS;    //  pasword (usado en WPA o llave key en WEP)
int keyIndex = 0;                // Numero de indice (key index) para tu red (solo en WEP)

//int led =  LED_BUILTIN;//LED en la tarjeta con direccion 13
int status = WL_IDLE_STATUS;
WiFiServer server(80);//Servidor en el puerto 80

//**********************************************************************************************

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {//Eliminar si no queremos puerto serie 
    ;//Espera hasta que tengamos una conexion serie por el puerto USB 
  }

  Serial.println("Servidor Web -Access Point-");//Puerto serie

  //pinMode(led, OUTPUT);// Configura el modo de este PIN si es un LED como OUTPUT
  configura_salidas(PRIMERO, ULTIMO);//Configura los PINs del PRIMERO 13 al ULTIMO 9 como salidas
  //  Verifica el modulo Wifi 
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("La comunicacion con el modulo Wifi no funciona !");
    // No se continua
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Actualice el firmware WIFI a la ultima version S");
  }

  //Direccion IP por defecto ,recupera del #define IP=192.168.6.1
  WiFi.config(IPAddress(IP));

  // Imprime el nombre de la red  (SSID);
  Serial.print("Creando access point llamado : ");
  Serial.println(ssid);

  //Creada open network .Cambiar esta linea si quieres crear una red WEP
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Error en la creacion del access point");
    // don't continue
    while (true);//Bucle infinito
  }

  // Espera 10 segundos antes de la conexion
  delay(10000);

  // Arranca el servidor web en el puerto 80
  server.begin();

  //Estas conectado imprime el status
  printWiFiStatus();
}

//**********************************************************************************************

void loop() {//Bucle principal 
  // COmpara el estado anterior con el actual 
  if (status != WiFi.status()) {
    //Si ha cambiado , actualizamos la variable estado 
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // Dispositivo conectado al access point AP 
      Serial.println("Dispositivo conectado al AP");
    } else {
      // Dispositivo desconectado del AP ,volvemos al modo escucha 
      Serial.println("Dispositivo desconectado del AP");
    }
  }
 
  WiFiClient client = server.available();   // Espera clientes disponibles 

  if (client) {                             // Si hay un nuevo cliente
    Serial.println("Nuevo cliente");        // imprime mensaje por el puerto serie 
    String currentLine = "";                // Crea una cadena  String para guardar los datos del cliente 
    while (client.connected()) {            // Queda en bucle mientras el cliente este conectado 
      if (client.available()) {             // Hay cliente ,datos , bytes para leer del cliente ? 
        char c = client.read();             // Entonces leemos un dato un caracter char
        //Serial.write(c);                    // imprime en el puerto serie el dato leido
        if (c == '\n') {                    // ha llegado al final?,fin de linea \n ?
          //Si la linea esta vacia , "blanca", obtenemos dos caractares en una linea
          //Es el final de la solicitud HTTP del cliente , entonces enviamos una respuesta
          
          if (currentLine.length() == 0) {
            //Las cabeceras HTTP siempre empiezan con un codigo de respuesta 
            // p.e (HTTP/1.1 200 OK) +tipo de contenido +una linea en blanco

            client.println("HTTP/1.1 200 OK");//INICIO HTTP
            client.println("Content-type:text/html");
            client.println();
            client.print ("OPCIONES CONFIGURADO PINS DEL "+String(PRIMERO)+" AL "+String(ULTIMO)+"<br>");
            
            for (uint8_t pin=PRIMERO;pin>=ULTIMO;pin--){//Crea los elementos para Encender/Apagar los LEDS con los numeros
              client.print ("N° PIN = "+String(pin)+"<br>");
              client.print("<font size=7> Click LED"+String(pin)+" <a href=\"/"+String(pin)+"_ON"+"\">here</a> turn the LED ON<br></font>");
              client.print("<font size=7> Click LED"+String(pin)+" <a href=\"/"+String(pin)+"_OFF"+"\">here</a> turn the LED OFF<br></font>");
            }

            //La respuesta HTTP termina con otra linea en blanco
            client.println();//FIN HTTP

            break;// Salir del bucle 
          }else {
            currentLine = "";//Si tenemos nueva linea \n ,limpiamos la linea actual currentLine 
          }
        }
        else if (c != '\r') {    // Si tenemos agun caracter diferente al retorno de carro '\r'
          currentLine += c;      // Escribe la linea corriente ,el caracter se anade a la linea actual
        }

        //------------------------------------------------------------------------------------------------------------------------
        //Miramos si la solicitud del cliente , en la linea corriente del navegador, termina con "LED0_ENCENDIDO" o "LED0_APAGADO" 
 
        if(currentLine.endsWith("_ON")){//Si esta encendido 
            Serial.println("LINEA On ORG = "+currentLine); 
            currentLine.remove (currentLine.indexOf('_'));//Busca caracter _ en la linea ,elimina el resto
            currentLine.remove (0,currentLine.indexOf('/')+1);//Busca caracter / en la linea ,elimina el resto            
            //currentLine+='\n';
            Serial.println("LINEA On elimina _ = "+currentLine); 
            Serial.println("Conversion pin = "+String (currentLine.toInt()));             
            digitalWrite( currentLine.toInt(),HIGH);//Activa LED
            currentLine="";
  
          
        }
        if(currentLine.endsWith("_OFF")){//Esta apagado
            Serial.println("LINEA Off org= "+currentLine); 
            currentLine.remove (currentLine.indexOf('_'));//Busca caracter _ en la linea ,elimina el resto  
            currentLine.remove (0,currentLine.indexOf('/')+1);//Busca caracter / en la linea ,elimina el resto              
            //currentLine+='\n';            
            Serial.println("LINEA Off elimina _= "+currentLine);                    
            digitalWrite( currentLine.toInt(),LOW);//Apaga LED 
            currentLine="";        
        }
        
        //------------------------------------------------------------------------------------------------------------------------        
      }
    }
    // Cerrar la conexion
    client.stop();
    Serial.println("cliente desconectado");
  }
}

//**********************************************************************************************

void printWiFiStatus() {
  //Muestra ,  por el puerto serie  la SSID de la red , donde nos conectamos
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Muestra la IP en la Wifi
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Muestra la IP donde debe conectarse el navegador 
  Serial.print(" Para ver esta pagina en accion abrir en un navegador esta direccion http://");
  Serial.println(ip);

}
