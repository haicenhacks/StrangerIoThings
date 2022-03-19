#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "StrangerIoThings.h"
AsyncWebServer server(80);


#define LED_PIN    14

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 100

bool run = true;
bool rapid = false;
bool yes = false;
bool no = false;
bool newmessage = false;



unsigned int message_cooldown = 10*60*1000;
unsigned int activity_cooldown = 3*60*1000;
const char* input_parameter1 = "input_string";
char webmessage[100]="0";
unsigned long last_message;
char* def_messages[] = {"RUN", "HELP ME", "IM HERE"};
unsigned long cooldown = 0;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ400);
const int num_colors = 4;

// each neopixel in the string is assigned a color from this list
uint32_t led_colors[num_colors] = {strip.Color(0, 255, 0),
                                   strip.Color(255, 0, 0),
                                   strip.Color(249,208,14),
                                   strip.Color(0,0,255)};

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  strcpy(webmessage, "0");

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  Serial.begin(9600);
  Serial.println("Running");
  last_message = millis();
  default_on();


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connecting...");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String input_message;
    String input_parameter;

    if (request->hasParam(input_parameter1)) {
      Serial.println(input_parameter1);
      input_message = request->getParam(input_parameter1)->value();
      input_parameter = input_parameter1;
      newmessage = true;
      if(request->hasParam("rapid")) {
        rapid=true;
      }
    }
    else if (request->hasParam("yes_button"))
    {
      yes=true;
      run = false;
      Serial.println("yes button pressed");
    }
    else if (request->hasParam("no_button"))
    {
      no=true;
      run = false;
      Serial.println("no button pressed");
    }
    else if (request->hasParam("run"))
    {
      run = true;
      Serial.println("run button pressed");
    }
    else if (request->hasParam("stop"))
    {
      run = false;
      Serial.println("stop button pressed");
    }
    else {
      input_message = "No message sent";
      input_parameter = "none";
    }
    Serial.println(input_message);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("+ input_parameter + ") with value: " + input_message + "<br><a href=\"/\">Return to Home Page</a>");

    input_message.toCharArray(webmessage, 100);
    Serial.println(webmessage);
    run=false;

  });
  server.onNotFound(notFound);
  server.begin();
}

void all_off()
{
  for(int i=0; i<=LED_COUNT-1; i++)
  {

    int c = num_colors;
    strip.setPixelColor(i,strip.Color(0,0,0));
  }
  strip.setBrightness(50);
  strip.show();
}

void default_on()
{
  for(int i=0; i<=LED_COUNT-1; i++)
  {

    int c = num_colors;
    strip.setPixelColor(i,led_colors[i%num_colors]);
  }
  strip.setBrightness(50);
  strip.show();
}

void yes_answer()
{
  all_off();
  delay(500);
  for (int num=0; num<3; num++)
  {
    int i = 10;
    strip.setPixelColor(i,led_colors[i%num_colors]);
    strip.show();
    delay(100);
    strip.setPixelColor(i, strip.Color(0,0,0));
    strip.show();
    delay(100);
    yes = false;

  }
  run = true;
}

void dim_then_bright()
{
  for (int i=0; i < 100; i++)
  {
    strip.setBrightness(i);
    delay(100);
  }
}

void no_answer()
{
  all_off();
  delay(500);
  for (int num=0; num<3; num++)
  {
    int i = 8;
    strip.setPixelColor(i,led_colors[i%num_colors]);
    strip.show();
    delay(100);
    strip.setPixelColor(i, strip.Color(0,0,0));
    strip.show();
    delay(100);
    no = false;
  }
  run = true;
}

void loop() {

  if (millis() > last_message + message_cooldown)
  {
    rapid = true;
    int m = millis()%3;
    int n = 5;
    if (m == 0)
    {
      n = 10;
    }
    spell_message(n, def_messages[m] );
    last_message = millis();
    all_off();
    cooldown = millis() + activity_cooldown;
  }

  if (millis() > cooldown)
  {
    for(int i=LED_COUNT-1; i>=0; i--)
    {
      if (!run)
      {
        break;
      }

      int c = num_colors;
      strip.setPixelColor(i,led_colors[i%num_colors]);
      if(i<LED_COUNT-1)
      {
        strip.setPixelColor(i+1, strip.Color(0,0,0));
      }

      strip.show();
      delay(200);
    }

    for(int i=LED_COUNT-1; i>=0; i--)
    {
      if (!run)
      {
        break;
      }

      int c = num_colors;
      strip.setPixelColor(i,led_colors[i%num_colors]);
      strip.show();
      delay(200);
    }
    default_on();
    cooldown = millis() + activity_cooldown;
  }

  if (yes)
  {
    yes_answer();
    delay(1000);
  }

  if (no)
  {
    no_answer();
    delay(1000);
  }

  if (newmessage)
  {
    Serial.println("displaying new message");
    spell_message(5, webmessage);
    strcpy (webmessage,"0");

    delay(5000);

  }

}


void spell_message(int num, char* message)
{
  int orig_dtime = 500;
  int dtime = 500;
  for (int n=0; n<num; n++)
  {
    all_off();
    delay(1000);
    for (int i=0; message[i]!= 0; i++)
    {
      char letter = toupper(message[i]);
      int this_led = 0;

      if (n == 1)
      {
        if (!rapid)
        {
          return;
        }
        dtime = 200;
      }
      else if (n == 2)
      {
        dtime = 100;
      }
      else if (n == 3)
      {
        dtime = 50;
      }

      //The led number (i.e. nth led on the string) is mapped to a letter and color
      switch (letter){
        case 'A':
          this_led = 64;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'B':
          this_led = 66;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'C':
          this_led = 68;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'D':
          this_led = 70;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'E':
          this_led = 72;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          //Serial.println("led E on");
          break;
        case 'F':
          this_led = 74;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'G':
          this_led = 76;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'H':
          this_led = 59;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'I':
          this_led = 57;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'J':
          this_led = 55;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'K':
          this_led = 53;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'L':
          this_led = 51;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'M':
          this_led = 49;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'N':
          this_led = 47;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'O':
          this_led = 45;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'P':
          this_led = 26;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'Q':
          this_led = 28;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'R':
          this_led = 30;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'S':
          this_led = 32;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'T':
          this_led = 34;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'U':
          this_led = 36;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'V':
          this_led = 38;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'W':
          this_led = 40;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'X':
          this_led = 22;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'Y':
          this_led = 20;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case 'Z':
          this_led = 18;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case ' ':
          delay(2*dtime);
          break;
        case '.':
          this_led = 16;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case '?':
          this_led = 14;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;
        case '!':
          this_led = 12;
          strip.setPixelColor(this_led,led_colors[this_led%num_colors]);
          strip.show();
          delay(dtime);
          strip.setPixelColor(this_led, strip.Color(0,0,0));;
          strip.show();
          break;

        case 0:
          return;
        default:
          break;
        }
      delay(dtime);

    }
  }
  run = true;
  rapid = false;
  newmessage=false;
}
