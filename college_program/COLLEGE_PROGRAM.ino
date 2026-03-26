#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// ===== WiFi =====
const char* ssid = "RUDRA_Display";
const char* password = "Rudra4124";

// ===== WebServer =====
WebServer server(80);

// ===== LED Matrix =====
#define PIN 5
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  32, 8, PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB + NEO_KHZ800
);

// ===== RTC =====
RTC_DS3231 rtc;

// ===== Globals =====
String messageToShow = "";
bool scrollingEnabled = false;
bool isInputMessage = false;
uint32_t currentColor;
int scrollX = 0;

// ===== Colors =====
uint32_t colorArray[] = {
  matrix.Color(204, 255, 0),   // 0 yellow-green
  matrix.Color(255, 0, 0),     // 1 red
  matrix.Color(0, 255, 0),     // 2 green
  matrix.Color(0, 0, 255),     // 3 blue
  matrix.Color(255, 255, 255),  // 4 white
  matrix.Color(57,255,20), // 5 neon greem
  matrix.Color(255,95,31),// 6 neon orange 
  matrix.Color(19,64,255),// 7 neon purple
  matrix.Color(255,51,153),// 8 bright pink
  matrix.Color(255,0,255),// 9 bright megenta
  matrix.Color(0,255,255),// 10 bright cyan
  matrix.Color(255,255,0),// 11 bright yellow
  matrix.Color(255,182,10),//12  bright orange
  matrix.Color(204,255,50)  // 13 neon yellow 
};

void startScroll(String text, uint32_t color, bool fromInput) {
  messageToShow = text;
  currentColor = color;
  isInputMessage = fromInput;

  if (fromInput && text.length() <= 5) {
    // small input → show static
    scrollingEnabled = false;
    matrix.fillScreen(0);
    int16_t x = (matrix.width() - text.length() * 6) / 2; // center
    if (x < 0) x = 0;
    matrix.setCursor(x, 0);
    matrix.setTextColor(currentColor);
    matrix.print(text);
    matrix.show();
  } else {
    // normal scrolling
    scrollingEnabled = true;
    scrollX = matrix.width();
  }
}
// color animation
void colorCircle() {

    unsigned long startTime = millis();
    int r = 255, g = 0, b = 0;

    while (millis() - startTime < 5000) {
      while (g < 255 && millis() - startTime < 5000) {
        g += 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
      while (r > 0 && millis() - startTime < 5000) {
        r -= 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
      while (b < 255 && millis() - startTime < 5000) {
        b += 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
      while (g > 0 && millis() - startTime < 5000) {
        g -= 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
      while (r < 255 && millis() - startTime < 5000) {
        r += 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
      while (b > 0 && millis() - startTime < 5000) {
        b -= 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
    }
    matrix.fillScreen(0);
    matrix.show();
  }
   void sparkleAnimation() {
    unsigned long startTime = millis();
    int totalPixels = matrix.width() * matrix.height();

    while (millis() - startTime < 7000) {
      // Light up a few random pixels with random colors
      for (int i = 0; i < 5; i++) {
        int x = random(matrix.width());
        int y = random(matrix.height());
        uint16_t color = matrix.Color(random(100, 256), random(50, 256), random(256));
        matrix.drawPixel(x, y, color);
      }

      matrix.show();
      delay(50);

      // Slight fade effect: randomly clear a few pixels
      for (int i = 0; i < 100; i++) {
        int x = random(matrix.width());
        int y = random(matrix.height());
        matrix.drawPixel(x, y, 0);
      }
    }

    matrix.fillScreen(0);  // Clear after animation
    matrix.show();
  }
void pauseScroll() {
  scrollingEnabled = false;  // pause, don’t clear
}

void clearDisplay() {
  scrollingEnabled = false;
  messageToShow = "";
  isInputMessage = false;
  matrix.fillScreen(0);
  matrix.show();
}

void scrollTextLoop() {
  if (!scrollingEnabled || messageToShow == "") return;

  matrix.fillScreen(0);
  matrix.setCursor(scrollX, 0);
  matrix.setTextColor(currentColor);
  matrix.print(messageToShow);
  matrix.show();

  scrollX--;
  int textWidth = messageToShow.length() * 6;

  if (scrollX < -textWidth) {
    if (isInputMessage) {
      scrollX = matrix.width(); // loop forever
    } else {
      pauseScroll(); // button message → stop after one pass
    }
  }
}

String buildHtmlPage() {
  String html = "<!DOCTYPE html><html><head><title>ESP32 LED Display BY RUDRA</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{font-family:Arial;text-align:center;background:#f2f2f2;}";
  html += ".grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;max-width:500px;margin:auto;}";
  html += "button{width:100%;height:60px;font-size:16px;border:none;border-radius:8px;background:#007BFF;color:white;}";
  html += "button:hover{background:#0056b3;cursor:pointer;}";
  html += ".stop{background:orange !important;}";
  html += ".clear{background:red !important;}";
  html += "form{margin-top:15px;}";
  html += "input[type=text]{width:250px;height:36px;font-size:16px;border-radius:5px;border:1px solid #ccc;padding:5px;}";
  html += "input[type=submit]{height:40px;font-size:16px;margin-top:10px;padding:0 15px;border:none;border-radius:8px;background:green;color:white;cursor:pointer;}";
  html += "input[type=submit]:hover{background:darkgreen;}";
  html += "</style></head><body>";

  html += "<h3>ESP32 LED Display By RUDRA</h3>";
  html += "<p>Current: " + messageToShow + "</p>";
  html += "<form method='POST' action='/'><input type='text' name='text' placeholder='Text:ColorIndex'>";
  html += "<br><input type='submit' value='Show Text'></form><br>";
  html += "<div class='grid'>";
  html += "<a href='/welcome'><button>WELCOME</button></a>";
  html += "<a href='/freshers'><button>WELCOME FRESHERS</button></a>";
  html += "<a href='/nexus'><button>NEXUX</button></a>";
  html += "<a href='/hod'><button>WELCOME HOD</button></a>";
  html += "<a href='/tic'><button>WELCOME TIC</button></a>";
  html += "<a href='/aliv'><button>WELCOME ALIV</button></a>";
  html += "<a href='/kk'><button>WELCOME KK</button></a>";
  html += "<a href='/piyali'><button>WELCOME PIYALI</button></a>";
  html += "<a href='/spd'><button>WELCOME SPD</button></a>";
  html += "<a href='/saraswati'><button>WELCOME SARASWATI</button></a>";
  html += "<a href='/sarmista'><button>WELCOME SARMISTA</button></a>";
  html += "<a href='/shubankar'><button>WELCOME SHUBANKAR</button></a>";
  html += "<a href='/noise'><button>MAKE SOME NOISE</button></a>";
  html += "<a href='/time'><button>SHOW TIME</button></a>";
  html += "<a href='/stop'><button class='stop'>STOP</button></a>";
  html += "<a href='/start'><button>START</button></a>";
  html += "<a href='/clear'><button class='clear'>CLEAR</button></a>";
  html += "</div><br>";

  
  html += "</body></html>";
  return html;
}

void handleQuickMessage(String msg) {
  currentColor= colorArray[random(14)];
  startScroll(msg, currentColor, false); // button → one-time scroll
  server.send(200, "text/html", buildHtmlPage());
}

void setup() {
  Serial.begin(115200);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  currentColor = colorArray[0];

  Wire.begin(21, 22);
  rtc.begin();

  WiFi.softAP(ssid, password);
  Serial.println("AP Started: " + String(ssid));
  Serial.println("IP: " + WiFi.softAPIP().toString());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", buildHtmlPage());
  });

  server.on("/", HTTP_POST, []() {
    if (server.hasArg("text")) {
      String input = server.arg("text"); // e.g. HELLO:2
      int colonIndex = input.indexOf(':');
      String textPart = input;
      String colorPart = "";
      int colorIndex = 0;
      int r,g,b;
       if(textPart == "ani1"){
          colorCircle();
          goto Ani;
        }
        else if(textPart=="ani2"){
          sparkleAnimation();
          goto Ani;
        }
      if (colonIndex > 0) {
        textPart = input.substring(0, colonIndex);
       

        colorPart = input.substring(colonIndex + 1);
        if(colorPart.length()==9){
          r=colorPart.substring(0,3).toInt();
          g=colorPart.substring(3,6).toInt();
          b=colorPart.substring(6,9).toInt();
          if(r>-1 && r<256 && g>-1 && g<256 && b>-1 && b<256)
          currentColor =  matrix.Color(r,g,b);
           else
           currentColor = colorArray[colorIndex];
          goto Rudra;
        }
        else 
         colorIndex =input.substring(colonIndex+1).toInt();
      }
      if (colorIndex >= 0 && colorIndex < (sizeof(colorArray)/sizeof(colorArray[0]))) {
        currentColor = colorArray[colorIndex];
      }
      Rudra:
      startScroll(textPart, currentColor, true); // input → infinite scroll
    }
    
      Ani:
    server.send(200, "text/html", buildHtmlPage());
  });

  server.on("/time", []() {
    DateTime now = rtc.now() + TimeSpan(0,0,0,15);
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", now.hour(), now.minute());
    messageToShow = String(timeStr);
    scrollingEnabled = false;
    isInputMessage = false;
    matrix.fillScreen(0);
    matrix.setCursor(1, 0);
    matrix.setTextColor(currentColor);
    matrix.print(messageToShow);
    matrix.show();
    server.send(200, "text/html", buildHtmlPage());
  });

  server.on("/stop", []() { pauseScroll(); server.send(200, "text/html", buildHtmlPage()); });
  server.on("/start", []() { scrollingEnabled = true; server.send(200, "text/html", buildHtmlPage()); });
  server.on("/clear", []() { clearDisplay(); server.send(200, "text/html", buildHtmlPage()); });

  // Quick message routes
  server.on("/welcome", []() { handleQuickMessage("WELCOME"); });
  server.on("/freshers", []() { handleQuickMessage("WELCOME FRESHERS"); });
  server.on("/hod", []() { handleQuickMessage("WELCOME PROF. SHOVAN ROY"); });
  server.on("/tic", []() { handleQuickMessage("WELCOME TIC PROF. SATYA RANJAN GHOSH"); });
  server.on("/aliv", []() { handleQuickMessage("WELCOME DR. ALIV KUMAR MONDAL"); });
  server.on("/kk", []() { handleQuickMessage("WELCOME PROF. KOUSHIK KUNDU"); });
  server.on("/piyali", []() { handleQuickMessage("WELCOME PROF. PIYALI SANYAL"); });
  server.on("/spd", []() { handleQuickMessage("WELCOME PROF. SANTI PADA DUA"); });
  server.on("/saraswati", []() { handleQuickMessage("WELCOME PROF. SARASWATI DUTTA"); });
  server.on("/sarmista", []() { handleQuickMessage("WELCOME DR. SHARMISTHA JANA"); });
  server.on("/noise", []() { handleQuickMessage("MAKE SOME NOISE FOR NEXUS"); });
 server.on("/nexus", []() { handleQuickMessage("NEXUS"); });
  server.on("/shubankar", []() { handleQuickMessage("WELCOME MR. SHUBANKAR SETH"); });
  
  server.begin();
}

void loop() {
  server.handleClient();
  scrollTextLoop();
  delay(70);
}
