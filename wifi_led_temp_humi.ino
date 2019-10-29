#include <SoftwareSerial.h>
#include <DHT.h>
#include <DHT_U.h>

#define DEBUG true
#define DHT_PIN A0
#define DHTTYPE DHT11


SoftwareSerial mySerial(9, 10); //RX, TX

DHT dht(DHT_PIN, DHTTYPE);

void setup() {
  Serial.begin(9600);        // ESP8266 Baud rate
  mySerial.begin(9600);
  pinMode(2, OUTPUT);   // used if connecting a LED to pin 11
  digitalWrite(2, LOW);
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);

  sendData("AT+RST\r\n", 2000, DEBUG);                // reset module
  sendData("AT+CWMODE=2\r\n", 1000, DEBUG);       // configure as access point
  sendData("AT+CIFSR\r\n", 1000, DEBUG);              // get ip address
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);         // configure for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG);  // turn on server on port 80

}

int sensetemp() { // function to sense temperature
  int val = analogRead(A0);
  float mv = (val / 1024.0) * 5000;
  float celcius = mv / 10;
  return(celcius);
}

int connectionId;

void loop() {
  if(mySerial.available()) {
    Serial.write(mySerial.read());
   // Serial.println("ok");
  }  
 if(Serial.available()) {
    mySerial.write(Serial.read());
    
   if(Serial.find("+IPD,")) { // Recieving from web browser to toggle led
     delay(300);
     connectionId = Serial.read() - 48;
      if(Serial.find("pin=")) {
        int pinNumber = (Serial.read() - 48) * 10;
        pinNumber += (Serial.read() - 48);
        digitalWrite(pinNumber, !digitalRead(pinNumber));
      } else { // Sending data to browser
        String webpage = "<h1>Hello World</h1>";
        espsend(webpage);
      }
      
      int t = dht.readTemperature();
      
      if(t != 0) {
        String add1 = "<h4>Temperature=</h4>";
        String two = String(t, 3);
        add1 += two;
        add1 += "&#x2103";  // Hex code for degree celcius
        espsend(add1);
      } else {
        String c = "sensor is not conneted";
        espsend(c);
      }

      String closeCommand = "AT+CIPCLOSE=";  // close the socket connection////esp command
      closeCommand += connectionId; // append connection id
      closeCommand += "\r\n";

      sendData(closeCommand, 3000, DEBUG);
    } 
  }
}

void espsend(String d) {
  String cipSend = " AT+CIPSEND=";
  cipSend += connectionId; 
  cipSend += ",";
  cipSend += d.length();
  cipSend += "\r\n";
  sendData(cipSend, 1000, DEBUG);
  sendData(d, 1000, DEBUG);
}

String sendData(String command, const int timeout, boolean debug) {
    String response = "";
    mySerial.print(command); // send the read character to the esp8266
    long int time = millis();
   
    while( (time+timeout) > millis()) {
      while(mySerial.available()) {
        // The esp has data so display its output to the serial window
        char c = mySerial.read(); // read the next character.
        response+=c;
      }
    }
   
    if(debug) {
      Serial.print(response);
    }
    return response;
}
