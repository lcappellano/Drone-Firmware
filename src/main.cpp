// TX TX TX TX TX TX TX TX TX TX TX TX TX
#include "Arduino.h"
//#include "Arduino.h"
#include <SPI.h>
#include <RF24-STM.h>
#include <Adafruit_SSD1306.h>
#include <pin_configuration.h>

RF24 radio(PB0, PA4); // CE, CSN

//setup display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define RECIEVER
//#define TRANSMITTER


// -----------------------------------------------------------------------------
// SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP
// -----------------------------------------------------------------------------
void setup() {
	// RX RX RX RX RX RX RX RX RX RX RX RX

	// Initiate the radio object
	radio.begin();

	// Set the transmit power to lowest available to prevent power supply related issues
	radio.setPALevel(RF24_PA_HIGH);

	// Set the speed of the transmission to the quickest available
	radio.setDataRate(RF24_2MBPS);

	// Use a channel unlikely to be used by Wifi, Microwave ovens etc 124
	radio.setChannel(104);

	// In case of failure it can retry
	radio.setRetries(255, 5);

    #ifdef RECIEVER
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 
	// Displays Adafruit logo by default. call clearDisplay immediately if you don't want this.
	display.display();
	delay(2000);

	// Displays "Hello world!" on the screen
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0,0);
	display.println("Starting...");
	display.display();

	// Open a writing and reading pipe on each radio, with opposite addresses
	radio.openReadingPipe(1, 0xB3B4B5B602);
	radio.openWritingPipe(0xB3B4B5B601);
    // Start the radio listening for data
	radio.startListening();
    #endif

    #ifdef TRANSMITTER
    // Open a writing and reading pipe on each radio, with opposite addresses
    radio.openReadingPipe(1, 0xB3B4B5B601);
    radio.openWritingPipe(0xB3B4B5B602);
    #endif

	// Auto ackknowledgment of a transmission
	radio.setAutoAck(true);

	// // Debugging information
	// radio.printDetails();
}

// -----------------------------------------------------------------------------
// We are LISTENING on this device only (although we do transmit a response)
// -----------------------------------------------------------------------------
void loop() {
	// RX RX RX RX RX RX RX RX RX RX RX RX

    #ifdef RECIEVER
	// This is what we receive from the other device (the transmitter)
	byte data;

	// Is there any data for us to get?
	if (radio.available()) {
		// Go and read the data and put it into that variable
		radio.read(&data, sizeof(byte));

		delay(100);
		display.clearDisplay();
		display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,0);
        display.print("Recieved: ");
        display.print(data);
        display.display();

		// No more data to get so send it back but add 1 first just for kicks
		// First, stop listening so we can talk
		radio.stopListening();

		// Increment reply
		data++;

		// Tell the user what we sent back (the random numer + 1)
		//Serial.println("RX: Sent " + String(data));

		if (!radio.write(&data, sizeof(byte))) {
			display.setTextColor(SSD1306_WHITE);
			display.setCursor(0,10);
            display.print("No ACK for ");
            display.print(data);
            display.display();
		} else {
			display.setTextColor(SSD1306_WHITE);
			display.setCursor(0,10);
            display.print("ACK for ");
            display.print(data);
            display.display();
		}

		// Now, resume listening so we catch the next packets.
		radio.startListening();}
    #endif

    #ifdef TRANSMITTER
    // TX TX TX TX TX TX TX TX TX TX TX TX TX

	// Ensure we have stopped listening (even if we're not) or we won't be able to transmit
	radio.stopListening();

	// Generate a single random character to transmit
	byte data = random(0, 254);
	radio.write(&data, sizeof(byte));

	// Did we manage to SUCCESSFULLY transmit that (by getting an acknowledgement back from the other Arduino)?
	// Even we didn't we'll continue with the sketch, you never know, the radio fairies may help us

	// Now listen for a response
	radio.startListening();

	// But we won't listen for long
	unsigned long started_waiting_at = millis();

	// Loop here until we get indication that some data is ready for us to read (or we time out)
	while (!radio.available()) {

		// Oh dear, no response received within our timescale
		if (millis() - started_waiting_at > 250) {
			//Serial.println("TX: No reply data received - timeout!");
			char buffer2[100];
			return;
		}
	}

	// Now read the data that is waiting for us in the nRF24L01's buffer
	byte dataRx;
	radio.read(&dataRx, sizeof(byte));

	// Wash, Rinse, Repeat... a bit later
	delay(2000);
    #endif
}