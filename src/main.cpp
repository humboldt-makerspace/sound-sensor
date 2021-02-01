#include "FastLED.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "constants.hpp"
#include "custom.h"

CRGB leds[NUM_LEDS];
CRGB colors[NUM_LEDS];

WiFiUDP UDPTestServer;

const int packetSize = 30;
byte packetBuffer[packetSize];

bool device_enabled = true;
bool buzzer_enabled = true;
int min_volume = 25;
int max_volume = 55;

float volume_buffer[BUFFER_SIZE];
int marker = 0;

void beep_warning (void)
{
	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i] = CRGB::Black;
	}
	for (int i = 0; i < 20; i++) {
		analogWrite(BUZZER_PIN1, PWM_VALUE);
		analogWrite(BUZZER_PIN2, PWM_VALUE);
		analogWrite(BUZZER_PIN3, PWM_VALUE);
		analogWrite(BUZZER_PIN4, PWM_VALUE);
        analogWrite(BUZZER_PIN5, PWM_VALUE);
        analogWrite(BUZZER_PIN6, PWM_VALUE);
        analogWrite(BUZZER_PIN7, PWM_VALUE);
        analogWrite(BUZZER_PIN8, PWM_VALUE);
		delay(200);
		analogWrite(BUZZER_PIN1, 0);
		analogWrite(BUZZER_PIN2, 0);
		analogWrite(BUZZER_PIN3, 0);
		analogWrite(BUZZER_PIN4, 0);
        analogWrite(BUZZER_PIN5, 0);
        analogWrite(BUZZER_PIN6, 0);
        analogWrite(BUZZER_PIN7, 0);
        analogWrite(BUZZER_PIN8, 0);
		delay(50);
	}
}

void beep_accept (void)
{
	analogWriteFreq(PWM_FREQ_HIGH);
	analogWrite(BUZZER_PIN1, PWM_VALUE);
	analogWrite(BUZZER_PIN2, PWM_VALUE);
	analogWrite(BUZZER_PIN3, PWM_VALUE);
	analogWrite(BUZZER_PIN4, PWM_VALUE);
    analogWrite(BUZZER_PIN5, PWM_VALUE);
    analogWrite(BUZZER_PIN6, PWM_VALUE);
    analogWrite(BUZZER_PIN7, PWM_VALUE);
    analogWrite(BUZZER_PIN8, PWM_VALUE);
	delay(100);
	analogWrite(BUZZER_PIN1, 0);
	analogWrite(BUZZER_PIN2, 0);
	analogWrite(BUZZER_PIN3, 0);
	analogWrite(BUZZER_PIN4, 0);
    analogWrite(BUZZER_PIN5, 0);
    analogWrite(BUZZER_PIN6, 0);
    analogWrite(BUZZER_PIN7, 0);
    analogWrite(BUZZER_PIN8, 0);
}

void beep_decline (void)
{
	analogWriteFreq(PWM_FREQ_LOW);
	analogWrite(BUZZER_PIN1, PWM_VALUE);
	analogWrite(BUZZER_PIN2, PWM_VALUE);
	analogWrite(BUZZER_PIN3, PWM_VALUE);
	analogWrite(BUZZER_PIN4, PWM_VALUE);
    analogWrite(BUZZER_PIN5, PWM_VALUE);
    analogWrite(BUZZER_PIN6, PWM_VALUE);
    analogWrite(BUZZER_PIN7, PWM_VALUE);
    analogWrite(BUZZER_PIN8, PWM_VALUE);
	delay(100);
	analogWrite(BUZZER_PIN1, 0);
	analogWrite(BUZZER_PIN2, 0);
	analogWrite(BUZZER_PIN3, 0);
	analogWrite(BUZZER_PIN4, 0);
    analogWrite(BUZZER_PIN5, 0);
    analogWrite(BUZZER_PIN6, 0);
    analogWrite(BUZZER_PIN7, 0);
    analogWrite(BUZZER_PIN8, 0);
	analogWriteFreq(PWM_FREQ_HIGH);
}

int get_volume_from_command (String cmd)
{
	int final_digit_pos = -1;
	for (int i = 15; i < cmd.length(); i++) {
		if (cmd.charAt(i) == 0 && i > 15) {
			final_digit_pos = i;
			break;
		}
		else if ((isWhitespace(cmd.charAt(i)) || cmd.charAt(i) == 0) && i == 15) {
			Serial.print("Missing number");
			return -1;
		}
		else if (!isDigit(cmd.charAt(i)) && final_digit_pos == -1) {
			Serial.print("No valid number: ");
			Serial.println(cmd.charAt(i));
			Serial.println(i);
			return -1;
		}
	}
	int result = (cmd.substring(15, final_digit_pos)).toInt();
	if (result > MAX_VOLUME) return MAX_VOLUME;
	else if (result < MIN_VOLUME) return MIN_VOLUME;
	else return result;
}

void processCommand (String cmd)
{
	Serial.print("Incoming package: ");
	Serial.println(cmd);
	if (cmd == "enable sensor" && !device_enabled) {
		device_enabled = true;
		Serial.println("Enabling sensor");
		beep_accept();
	}
	else if (cmd == "disable sensor" && device_enabled) {
		device_enabled = false;
		Serial.println("Disabling sensor");
		beep_accept();
	}
	else if (cmd == "enable buzzer" && !buzzer_enabled) {
		buzzer_enabled = true;
		Serial.println("Enabling buzzer");
		beep_accept();
	}
	else if (cmd == "disable buzzer" && buzzer_enabled) {
		buzzer_enabled = false;
		Serial.println("Disabling buzzer");
		beep_accept();
	}
	else if (cmd == "beep" && buzzer_enabled) {
		Serial.println("beeping");
		beep_warning();
	}
	else if (cmd.substring(0, 15) == "set min volume ") {
		int number = get_volume_from_command(cmd);
		if (number != -1) {
			if (number < max_volume) {
				Serial.print("Setting minimal volume to ");
				Serial.println(number);
				min_volume = number;
				beep_accept();
			}
			else {
				Serial.println("Minimum volume cannot be bigger than or equal to maximum volume");
				beep_decline();
			}
		}
	}
	else if (cmd.substring(0, 15) == "set max volume ") {
		int number = get_volume_from_command(cmd);
		if (number != -1) {
			if (number > min_volume) {
				Serial.print("Setting maximum volume to ");
				Serial.println(number);
				max_volume = number;
				beep_accept();
			}
			else {
				Serial.println("Maximum volume cannot be smaller than or equal to minimum volume");
				beep_decline();
			}
		}
	}
	else {
		Serial.println("Unknown or superfluous command");
		beep_decline();
	}
}

void handleUDPServer (void)
{
	int cb = UDPTestServer.parsePacket();
	if (cb) {
		for (int i = 0; i < packetSize; i++) packetBuffer[i] = 0;
		UDPTestServer.read(packetBuffer, packetSize);
		String myData = "";
		for (int i = 0; i < packetSize; i++) {
			myData += (char)packetBuffer[i];
		}
		processCommand(myData);
	}
}

void fillGradientRGB (CRGB* input, uint16_t startpos, CRGB startcolor, uint16_t endpos, CRGB endcolor)
{
    if (endpos < startpos) {
        uint16_t t = endpos;
        CRGB tc = endcolor;
        endcolor = startcolor;
        endpos = startpos;
        startpos = t;
        startcolor = tc;
    }

    saccum87 rdistance87;
    saccum87 gdistance87;
    saccum87 bdistance87;

    rdistance87 = (endcolor.r - startcolor.r) << 7;
    gdistance87 = (endcolor.g - startcolor.g) << 7;
    bdistance87 = (endcolor.b - startcolor.b) << 7;

    uint16_t pixeldistance = endpos - startpos;
    int16_t divisor = pixeldistance ? pixeldistance : 1;

    saccum87 rdelta87 = rdistance87 / divisor;
    saccum87 gdelta87 = gdistance87 / divisor;
    saccum87 bdelta87 = bdistance87 / divisor;

    rdelta87 *= 2;
    gdelta87 *= 2;
    bdelta87 *= 2;

    accum88 r88 = startcolor.r << 8;
    accum88 g88 = startcolor.g << 8;
    accum88 b88 = startcolor.b << 8;
    for (uint16_t i = startpos; i <= endpos; i++) {
        input[i] = CRGB( r88 >> 8, g88 >> 8, b88 >> 8);
        r88 += rdelta87;
        g88 += gdelta87;
        b88 += bdelta87;
    }
}

// order of colors: GRB
void led_on (float pos)
{
	float green = 0;
	float red = 0;
	if (pos < NUM_LEDS / 2) {
		green = 255;
		red = 255 * (2 * pos / (NUM_LEDS - 2));
	}
	else {
		green = 255 * 2 * (NUM_LEDS - 1 - pos) / (NUM_LEDS - 2);
		red = 255;
	}
	leds[(int)pos] = CRGB((int)green, (int)red, 0);
}

void visualize_volume (float volume)
{
	if (volume < min_volume) {
		for (int i = 0; i < NUM_LEDS; i++) {
			leds[i] = CRGB::Black;
		}
	}
	else if (volume >= max_volume) {
		for (int i = 0; i < NUM_LEDS; i++) {
			leds[i] = colors[i]; //led_on(i);
		}
	}
	else {
		for (int i = 0; i < NUM_LEDS; i++) {
			float u = i;
			if ((volume - min_volume) / (max_volume - min_volume) >= u / (NUM_LEDS - 1)) leds[i] = colors[i];//led_on(i);
			else leds[i] = CRGB::Black;
		}
	}
	FastLED.show();
	//delay(10);
}

void update_buffer (float volume)
{
	volume_buffer[marker] = volume;
	if (marker == BUFFER_SIZE - 1) marker = 0;
	else marker++;
}

void flush_buffer (void)
{
	for (int i = 0; i < BUFFER_SIZE; i++) {
		volume_buffer[i] = 0;
	}
}

float get_buffer_avg (void)
{
	float res = 0;
	for (int i = 0; i < BUFFER_SIZE; i++) {
		res += volume_buffer[i];
	}
	return res / BUFFER_SIZE;
}

void setup ()
{
	// put your setup code here, to run once:
	Serial.begin(115200);
	Serial.print("MAC: ");
	Serial.println(WiFi.macAddress());

	WiFi.mode(WIFI_STA);
	WiFi.begin(SSID_CUSTOM, PW_CUSTOM);
	Serial.println();
	Serial.println();
	Serial.print("Wait for WiFi... ");

	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.print(".");
		delay(500);
	}
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
	UDPTestServer.begin(UDP_PORT);

	FastLED.addLeds<WS2812B, LED_PIN>(leds, NUM_LEDS);
	FastLED.setBrightness(40);

	analogWriteFreq(PWM_FREQ_HIGH);

    fillGradientRGB(colors, 0, CRGB::Red, NUM_LEDS - 1, CRGB::Green);
}

void loop ()
{
	if (device_enabled) {
		float volume = analogRead(SOUND_SENSOR_PIN);
		//Serial.println(volume);
		update_buffer(volume);
		float avg = get_buffer_avg();
		visualize_volume(avg);
		if (avg >= max_volume) {
			if (buzzer_enabled) beep_warning();
			flush_buffer();
		}
	}
	handleUDPServer();
	delay(1);
}
