#include <TinyGPS++.h>
#include <HardwareSerial.h>

TinyGPSPlus gps;
HardwareSerial ss(1);      // Create a serial object for the GPS module
HardwareSerial sim800(2);  // Create a serial object for the SIM800L module

const int buttonPin = 22;                    // Pin where the button is connected
bool buttonPressed = false;                  // State to track button press
const String phoneNumber = "+639420585869";  // Number to send SMS to

void setup() {
  Serial.begin(115200);                    // Initialize serial for ESP32 monitor
  ss.begin(9600, SERIAL_8N1, 19, 18);      // Initialize the GPS serial (RX=18, TX=19)
  sim800.begin(9600, SERIAL_8N1, 16, 17);  // Initialize the SIM800L serial (RX=17, TX=16)

  pinMode(buttonPin, INPUT_PULLUP);  // Set button pin as input with internal pull-up resistor
  Serial.println("GPS Module & SIM800L Ready!");

  // Check SIM800L wiring by sending AT command
  if (checkSIM800Connection()) {
    Serial.println("SIM800L is connected properly!");
  } else {
    Serial.println("Error: SIM800L not responding. Check wiring.");
  }

  // Initialize SIM800L settings
  sendCommand("AT+CMGF=1");  // Set SMS mode to text
}

void loop() {
  // Check if GPS data is available and read it
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  // Check if button is pressed
  if (digitalRead(buttonPin) == LOW) {
    // Debounce: small delay to check if it's an actual button press
    delay(50);
    if (digitalRead(buttonPin) == LOW) {
      if (!buttonPressed) {
        buttonPressed = true;  // Set button state to pressed
        if (gps.location.isValid()) {
          // Print the location coordinates
          Serial.print("Latitude: ");
          Serial.println(gps.location.lat(), 6);
          Serial.print("Longitude: ");
          Serial.println(gps.location.lng(), 6);

          // Create the Google Maps URL
          String googleMapsUrl = "https://www.google.com/maps/search/?api=1&query=";
          googleMapsUrl += String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);

          // Print the Google Maps link to Serial Monitor
          Serial.println("Google Maps link:");
          Serial.println(googleMapsUrl);

          // Send SMS with the Google Maps link
          sendSMS(phoneNumber, googleMapsUrl);
        } else {
          Serial.println("Waiting for GPS signal...");
        }
      }
    }
  } else {
    buttonPressed = false;  // Reset when button is not pressed
  }

  // Continuously check for incoming data from SIM800L
  while (sim800.available()) {
    Serial.write(sim800.read());  // Print incoming data to Serial Monitor
  }
}

// Function to send an SMS using SIM800L
void sendSMS(String number, String text) {
  sim800.println("AT+CMGS=\"" + number + "\"");  // Send the SMS command with the recipient's number
  delay(100);                                    // Wait for a prompt
  sim800.println(text);                          // The SMS text message
  delay(100);
  sim800.write(26);  // Send Ctrl+Z to indicate the end of the message
  delay(5000);       // Give the module time to send the message

  // Check for a response to confirm SMS sent
  String response = "";
  while (sim800.available()) {
    response += char(sim800.read());  // Read the response
  }
  Serial.println("Response after sending SMS: " + response);  // Print the response for debugging

  // Check if SMS was sent successfully
  if (response.indexOf("OK") != -1) {
    Serial.println("SMS sent successfully!");
  } else {
    Serial.println("Error: SMS not sent.");
  }
}

// Function to send AT commands to the SIM800L module
void sendCommand(String command) {
  sim800.println(command);  // Send command
  delay(1000);              // Wait for a response
  String response = "";
  while (sim800.available()) {
    response += char(sim800.read());  // Read the response
  }
  Serial.println("Response: " + response);  // Print the response for debugging
}

// Function to check the SIM800L connection
bool checkSIM800Connection() {
  sim800.println("AT");  // Send AT command to check if the module is responsive
  delay(1000);           // Wait for a response
  String response = "";
  while (sim800.available()) {
    response += char(sim800.read());  // Read the response
  }
  Serial.println("Response from SIM800L: " + response);  // Print the response for debugging
  return response.indexOf("OK") != -1;                   // Return true if "OK" is found in the response
}