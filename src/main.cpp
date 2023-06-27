#include <Arduino.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define LED_PIN 23

const char *WIFI_SSID = "<WIFI_SSID>";
const char *WIFI_PASS = "<WIFI_PASSWORD>";

const char *QUERY_PARAM_KEY = "id";
const char *QUERY_PARAM_VALUE = "state";

const char *DEVICES_NAMES[1] = {
        "LED",
};

const int DEVICES_PINS[1] = {
        LED_PIN,
};

const int DEVICES_SIZE = std::end(DEVICES_PINS) - std::begin(DEVICES_PINS);

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head> <title>SALA MOZAL</title> <meta name="viewport" content="width=device-width, initial-scale=1"> <link rel="icon" href="data:,"> <style>html{font-family: Arial; display: inline-block; text-align: center;}h2{font-size: 3.0rem;}p{font-size: 3.0rem;}body{max-width: 600px; margin: 0px auto; padding-bottom: 25px;}.switch{position: relative; display: inline-block; width: 120px; height: 68px}.switch input{display: none}.slider{position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}.slider:before{position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}input:checked+.slider{background-color: #0068b3}input:checked+.slider:before{-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}</style></head><body> <h2>SALA MOZAL</h2> %DEVICES_PLACEHOLDER% <script>function toggleCheckbox(element){var xhr=new XMLHttpRequest(); if (element.checked){xhr.open("GET", "/update?id=" + element.id + "&state=1", true);}else{xhr.open("GET", "/update?id=" + element.id + "&state=0", true);}xhr.send();}</script></body></html>
)rawliteral";

AsyncWebServer server(80);

void setupWifi();

void setupServer();

void homeRoute();

void updateRoute();

String deviceState(int pin);

String requestProcessorCallback(const String &var);

void setup() {
    Serial.begin(9600);
    pinMode(LED_PIN, OUTPUT);

    setupWifi();
    setupServer();
}

void loop() {}

void setupWifi() {
    Serial.printf("Connecting to WIFI %s.", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFiClass::status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println(" CONNECTED!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setupServer() {
    homeRoute();
    updateRoute();
    server.begin();
}

void homeRoute() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", INDEX_HTML, requestProcessorCallback);
    });
}

void updateRoute() {
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam(QUERY_PARAM_KEY) && request->hasParam(QUERY_PARAM_VALUE)) {
            int pin = request->getParam(QUERY_PARAM_KEY)->value().toInt();
            int state = request->getParam(QUERY_PARAM_VALUE)->value().toInt();
            digitalWrite(pin, state);
        }
        request->send(200, "text/plain", "OK");
    });
}

String requestProcessorCallback(const String &var) {
    if (var != "DEVICES_PLACEHOLDER") {
        return emptyString;
    }
    String devicesButtons = emptyString;
    for (int i = 0; i < DEVICES_SIZE; i++) {
        auto name = DEVICES_NAMES[i];
        int pin = DEVICES_PINS[i];
        devicesButtons.concat("<h4>");
        devicesButtons.concat(name);
        devicesButtons.concat("</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"");
        devicesButtons.concat(pin);
        devicesButtons.concat("\" ");
        devicesButtons.concat(deviceState(pin));
        devicesButtons.concat("><span class=\"slider\"></span></label>");
    }
    return devicesButtons;
}

String deviceState(int pin) {
    return digitalRead(pin) ? "checked" : "";
}