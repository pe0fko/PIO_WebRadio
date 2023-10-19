/*
  WebRadio Example
  Very simple HTML app to control web streaming
  
  Copyright (C) 2017  Earle F. Philhower, III

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  ESP8266
  Board: LOLIN(WEMOS) D1 mini (clone)
  CPU Frequency 160MHz
  SSID setting

*/

//#define   USAGE_OTA
#define   USAGE_MDNS

#include <Arduino.h>

// ESP8266 server.available() is now server.accept()
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#if defined(ESP32)
    #include <WiFi.h>
#else
    #include <ESP8266WiFi.h>
#endif
#include <EEPROM.h>
//#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <ArduinoOTA.h>

#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"

#include "web.h"                  // Custom web server that doesn't need much RAM
#include "StationList.h"          // List of streaming servers
#include "WiFi_SSID.h"

// Enter your WiFi setup here:
const char* ssid      = WIFI_SSID_NAME_0;
const char* password  = WIFI_SSID_PWD_0;
const char* hostname  = "webradio"; // .local

//ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

WiFiServer    server(80);

AudioGenerator*           decoder = NULL;
AudioFileSourceICYStream* file    = NULL;
AudioFileSourceBuffer*    buff    = NULL;
AudioOutputI2S*           out     = NULL;

int volume = 0;
char title[64];
char url[96];
char status[64];
bool newUrl = false;
bool isAAC = true;
unsigned long retryms = 0;
unsigned long lastms = 0;

typedef struct {
  char url[96];
  bool isAAC;
  int16_t volume;
  int16_t checksum;
} Settings;

#ifdef ESP8266
const int preallocateBufferSize = 5*1024;
const int preallocateCodecSize = 29192; // MP3 codec max mem needed
#else
const int preallocateBufferSize = 16*1024;
const int preallocateCodecSize = 85332; // AAC+SBR codec max mem needed
#endif
void *preallocateBuffer = NULL;
void *preallocateCodec = NULL;


// C++11 multiline string constants are neato...
static const char HEAD[] PROGMEM = R"KEWL(
<html>
<head>
<title>ESP8266 Web Radio</title>
<script type="text/javascript">
  function updateTitle() {
    var x = new XMLHttpRequest();
    x.open("GET", "title");
    x.onload = function() { document.getElementById("titlespan").innerHTML=x.responseText; setTimeout(updateTitle, 5000); }
    x.onerror = function() { setTimeout(updateTitle, 5000); }
    x.send();
  }
  setTimeout(updateTitle, 1000);
  function showValue(n) {
    document.getElementById("volspan").innerHTML=n;
    var x = new XMLHttpRequest();
    x.open("GET", "setvol?vol="+n);
    x.send();
  }
  function updateStatus() {var x = new XMLHttpRequest();
    x.open("GET", "status");
    x.onload = function() { document.getElementById("statusspan").innerHTML=x.responseText; setTimeout(updateStatus, 5000); }
    x.onerror = function() { setTimeout(updateStatus, 5000); }
    x.send();
  }
  setTimeout(updateStatus, 2000);
</script>
<style>
body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
#container { text-align: center; border: 4px solid #000; padding: 20px; }
</style>
</head>)KEWL";

static const char BODY_0[] PROGMEM = R"KEWL(
<body>
<div id="container">
<h2>ESP8266 Web Radio!</h2>
<hr>
Currently Playing: <span style="color:blue" id="titlespan">%s</span><br>
Volume: <input type="range" name="vol" min="1" max="100" steps="5" value="%d" onchange="showValue(this.value)"/> <span id="volspan">%d</span>%%
<hr>
Status: <span id="statusspan">%s</span>
<form action="stop" method="POST"><input type="submit" value="Stop"></form>
<hr>

<form action="changeurl" method="GET">

 <p>
 Change URL: <input type="text" name="url">
)KEWL";

static const char BODY_1[] PROGMEM = R"KEWL(
 <select name="type">
  <option value="mp3" %s>MP3</option>
  <option value="aac" %s>AAC</option>
 </select>
 </p>
)KEWL";

static const char BODY_2[] PROGMEM = R"KEWL(
 <p>Select Radio:
 <select name="radio">
  <option value="9999">-- Input field --</option>
)KEWL";

static const char BODY_3[] PROGMEM = R"KEWL(
  <option value="%d">%s (%s)</option>
)KEWL";

static const char BODY_4[] PROGMEM = R"KEWL(
 </select>
 </p>
 <input type="submit" value="Change">
</form>
</div></body></html>
)KEWL";

static void WebWrite(WiFiClient *client, PGM_P ptr)
{
  const int blockSize = 256;
  int len = strlen_P(ptr);
  for (int i = 0; i < len; i += blockSize)
  {
    int l = len - i;
    if (l > blockSize) l = blockSize;
//  Serial.printf_P("= [%d] - [%.60s]\n", l, ptr);
    client->write_P( ptr, l );
    client->flush();
    ptr += l;
    yield();
  }
}

void HandleIndex(WiFiClient *client)
{
  const int buffLength = sizeof(BODY_0) + sizeof(title) + sizeof(status) + sizeof(url) + 3*2 ;
//  char buff[buffLength+100];    // 2048 << !!!!!!!!1
  char buff[1024];    // 2048 << !!!!!!!!1

  Serial.printf_P(PSTR("Sending INDEX...Free mem=%d, Buffer Length %d\n"), 
      ESP.getFreeHeap(), buffLength);

  WebHeaders(client, NULL);
  WebPrintf(client, DOCTYPE);

  WebWrite(client, HEAD);
 // client->write_P( HEAD, strlen_P(HEAD) );
 // delay(10);

  sprintf_P(buff, BODY_0, title, volume, volume, status);
  client->write(buff, strlen(buff) );
  delay(10);

  sprintf_P(buff, BODY_1, isAAC?"":"selected", isAAC?"selected":"");
  client->write(buff, strlen(buff) );
  delay(10);

  WebWrite(client, BODY_2);
//  client->write_P( BODY_2, strlen_P(BODY_2) );
//  delay(10);

  for (int i = 0 ; i < stationListNumber; i++) 
  {
    sprintf_P(buff, BODY_3, i 
            , stationList[i].name
            , stationList[i].mp3 ? "mp3" : "aac"
            );

    client->write(buff, strlen(buff) );
  }
  delay(10);

  WebWrite(client, BODY_4);
//  client->write_P( BODY_4, strlen_P(BODY_4) );
//  delay(10);

  Serial.printf_P(PSTR("Sent INDEX...Free mem=%d\n"), ESP.getFreeHeap());
}

void HandleStatus(WiFiClient *client)
{
  WebHeaders(client, NULL);
  client->write(status, strlen(status));
}

void HandleTitle(WiFiClient *client)
{
  WebHeaders(client, NULL);
  client->write(title, strlen(title));
}

void HandleVolume(WiFiClient *client, char *params)
{
  char *namePtr;
  char *valPtr;
  
  while (ParseParam(&params, &namePtr, &valPtr)) {
    ParamInt("vol", volume);
  }

  Serial.printf_P(PSTR("Set volume: %d\n"), volume);
  out->SetGain(((float)volume)/100.0);
  RedirectToIndex(client);
}

void HandleChangeURL(WiFiClient *client, char *params)
{
  char* namePtr;
  char* valPtr;
  char  newURL[sizeof(url)];
  char  newType[4];
  int   radio = 9999;

  Serial.printf_P(PSTR(">>>> Enter HandleChangeURL()\n"));

  newURL[0] = 0;
  newType[0] = 0;

  while (ParseParam(&params, &namePtr, &valPtr)) {
    ParamText("url", newURL);
    ParamText("type", newType);
    ParamInt("radio", radio);
  }

  yield();

  if (radio >= 0 && radio < stationListNumber)
  {
    strncpy(url, stationList[radio].url, sizeof(url)-1);
    url[sizeof(url)-1] = 0;
    isAAC = !stationList[radio].mp3;

    Serial.printf_P(PSTR("Change to Radio = #%d\n"), radio);

    strcpy_P(status, PSTR("Changing Radio..."));
    RedirectToIndex(client);
    newUrl = true;
  }
  else if (newURL[0] && newType[0]) 
  {
    strncpy(url, newURL, sizeof(url)-1);
    url[sizeof(url)-1] = 0;
    if (!strcmp_P(newType, PSTR("aac"))) {
      isAAC = true;
    } else {
      isAAC = false;
    }

    Serial.printf_P(PSTR("Changed URL to: %s\n"), url);

    strcpy_P(status, PSTR("Changing URL..."));
    RedirectToIndex(client);
    newUrl = true;
  } 
  else 
  {
    WebError(client, 404, NULL, false);
  }
}

void RedirectToIndex(WiFiClient *client)
{
  WebError(client, 301, PSTR("Location: /\r\n"), true);
}

void StopPlaying()
{
  if (decoder) {
    decoder->stop();
    delete decoder;
    decoder = NULL;
  }
  if (buff) {
    buff->close();
    delete buff;
    buff = NULL;
  }
  if (file) {
    file->close();
    delete file;
    file = NULL;
  }
  strcpy_P(status, PSTR("Stopped"));
  strcpy_P(title, PSTR("Stopped"));
}

void HandleStop(WiFiClient *client)
{
  Serial.printf_P(PSTR("HandleStop()\n"));
  StopPlaying();
  RedirectToIndex(client);
}

void MDCallback(void *cbData, const char *type, bool isUnicode, const char *str)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  (void) isUnicode; // Punt this ball for now
  (void) ptr;
  
  Serial.printf_P(PSTR("Stream type: %s\n"), type);
  if (strstr_P(type, PSTR("Title"))) { 
    strncpy(title, str, sizeof(title));
    title[sizeof(title)-1] = 0;
    Serial.printf_P(PSTR("      title: %s\n"), title);
  } else {
    // Who knows what to do?  Not me!
  }
}
void StatusCallback(void *cbData, int code, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  (void) code;
  (void) ptr;
  strncpy_P(status, string, sizeof(status)-1);
  status[sizeof(status)-1] = 0;
  Serial.printf_P(PSTR("Stream status: %s\n"), status);
}

#ifdef USAGE_OTA
void
init_OTA()
{
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.setPassword("pe0fko");

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  
    StopPlaying();

  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");
//    ESP reboot
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
#endif


void setup()
{
  // First, preallocate all the memory needed for the buffering and codecs, never to be freed
  preallocateBuffer = malloc(preallocateBufferSize);
  preallocateCodec = malloc(preallocateCodecSize);

  Serial.begin(115200);
  while(!Serial);
  if (!preallocateBuffer || !preallocateCodec) {
    Serial.printf_P(PSTR("FATAL ERROR:  Unable to preallocate %d bytes for app\n"), preallocateBufferSize+preallocateCodecSize);
    while (1) delay(1000); // Infinite halt
  }

  Serial.printf_P(PSTR("\n\nStartup the WebRadio.\n"));

  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Try forever
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf_P(PSTR("...Connecting to WiFi\n"));
    delay(1000);
  }
  Serial.printf_P(PSTR("Connected at "));
  Serial.println(WiFi.localIP());
  
  server.begin();
  
  strcpy_P(url, PSTR("none"));
  strcpy_P(status, PSTR("OK"));
  strcpy_P(title, PSTR("Idle"));

  audioLogger = &Serial;
  file = NULL;
  buff = NULL;
  out = new AudioOutputI2S();
  decoder = NULL;

#ifdef USAGE_MDNS
  if (MDNS.begin(hostname)) {             // Start the mDNS responder for XXX.local
    MDNS.addService("http", "tcp", 80);
    Serial.printf_P(PSTR("Go to http://%s.local/ to control the web radio.\n"), hostname);
  }
#else
  Serial.printf_P(PSTR("Go to http://"));
  Serial.print(WiFi.localIP());
  Serial.printf_P(PSTR("/ to control the web radio.\n"));
#endif

#ifdef USAGE_OTA
  init_OTA();
#endif

  LoadSettings();
}

void StartNewURL()
{
//  Serial.printf_P(PSTR("Changing URL to: %s, vol=%d\n"), url, volume);

  newUrl = false;

  // Stop and free existing ones
  Serial.printf_P(PSTR("Before stop...Free mem=%d\n"), ESP.getFreeHeap());
  StopPlaying();
  Serial.printf_P(PSTR("After stop...Free mem=%d\n"), ESP.getFreeHeap());
  SaveSettings();
  Serial.printf_P(PSTR("Saved settings\n"));
  
  file = new AudioFileSourceICYStream(url);
  Serial.printf_P(PSTR("created icystream\n"));
  file->RegisterMetadataCB(MDCallback, NULL);
 
  buff = new AudioFileSourceBuffer(file, preallocateBuffer, preallocateBufferSize);
  Serial.printf_P(PSTR("created buffer\n"));
  buff->RegisterStatusCB(StatusCallback, NULL);
 
  decoder = isAAC ? (AudioGenerator*) new AudioGeneratorAAC(preallocateCodec, preallocateCodecSize) : (AudioGenerator*) new AudioGeneratorMP3(preallocateCodec, preallocateCodecSize);
  Serial.printf_P(PSTR("created decoder\n"));
  decoder->RegisterStatusCB(StatusCallback, NULL);
  Serial.printf_P("Decoder start...\n");

  decoder->begin(buff, out);
  out->SetGain(((float)volume)/100.0);
  if (!decoder->isRunning()) 
  {
    Serial.printf_P(PSTR("Can't connect to URL"));
    StopPlaying();
    strcpy_P(status, PSTR("Unable to connect to URL"));
    retryms = millis() + 2000UL;
  } else {
    Serial.printf_P("Decoder is running.\n");
  }

  Serial.printf_P("Done start new URL\n");
}

void LoadSettings()
{
  // Restore from EEPROM, check the checksum matches
  Settings s;
  uint8_t *ptr = reinterpret_cast<uint8_t *>(&s);
  EEPROM.begin(sizeof(s));
  for (size_t i=0; i<sizeof(s); i++) {
    ptr[i] = EEPROM.read(i);
  }
  EEPROM.end();
  int16_t sum = 0x1234;
  for (size_t i=0; i<sizeof(url); i++) sum += s.url[i];
  sum += s.isAAC;
  sum += s.volume;
  if (s.checksum == sum) {
    strcpy(url, s.url);
    isAAC = s.isAAC;
    volume = s.volume;
    Serial.printf_P(PSTR("Resuming stream from EEPROM: %s, type=%s, vol=%d\n"), url, isAAC?"AAC":"MP3", volume);
  } else {
    // Take default URL from the code is no EEPROM saved profile.
    strcpy(url, stationList[StationListDefault].url);
    isAAC = !stationList[StationListDefault].mp3;
    volume = stationList[StationListDefault].volume;
    Serial.printf_P(PSTR("Stream DEFAULT: %s (%s), type=%s, vol=%d\n"), 
        stationList[StationListDefault].name, url, isAAC?"AAC":"MP3", volume);
  }
  newUrl = true;
}

void SaveSettings()
{
  // Store in "EEPROM" to restart automatically
  Settings s;
  memset(&s, 0, sizeof(s));
  strcpy(s.url, url);
  s.isAAC = isAAC;
  s.volume = volume;
  s.checksum = 0x1234;
  for (size_t i=0; i<sizeof(url); i++) s.checksum += s.url[i];
  s.checksum += s.isAAC;
  s.checksum += s.volume;
  uint8_t *ptr = reinterpret_cast<uint8_t *>(&s);
  EEPROM.begin(sizeof(s));
  for (size_t i=0; i<sizeof(s); i++) {
    EEPROM.write(i, ptr[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}

void PumpDecoder()
{
  yield();

#ifdef USAGE_MDNS
  MDNS.update();
#endif
#ifdef USAGE_OTA
  ArduinoOTA.handle();
#endif
  if (decoder && decoder->isRunning()) 
  {
    if (!decoder->loop()) {
      Serial.printf_P(PSTR("Stopping decoder\n"));
      StopPlaying();
      retryms = millis() + 2000UL;
    } else {
      strcpy_P(status, PSTR("Playing")); // By default we're OK unless the decoder says otherwise
    }
  }
}

void loop()
{
  if ((millis()-lastms) > 30*1000UL) {
    lastms = millis();
    Serial.printf_P ( PSTR("Running for %d seconds%c...Free mem=%d\n")
                    , lastms/1000, !decoder?' ':(decoder->isRunning()?'*':' '), ESP.getFreeHeap());
  }

  if (retryms && (millis()-retryms)>0) {
    retryms = 0;
    newUrl = true;
  }
  
  if (newUrl) {
    StartNewURL();
  }

  PumpDecoder();
  
  char *reqUrl;
  char *params;
  WiFiClient client = server.available();
  PumpDecoder();

  char reqBuff[384];
  if (client && WebReadRequest(&client, reqBuff, 384, &reqUrl, &params)) 
  {
    PumpDecoder();

    if (IsIndexHTML(reqUrl)) {
      HandleIndex(&client);
    } else if (!strcmp_P(reqUrl, PSTR("stop"))) {
      HandleStop(&client);
    } else if (!strcmp_P(reqUrl, PSTR("status"))) {
      HandleStatus(&client);
    } else if (!strcmp_P(reqUrl, PSTR("title"))) {
      HandleTitle(&client);
    } else if (!strcmp_P(reqUrl, PSTR("setvol"))) {
      HandleVolume(&client, params);
    } else if (!strcmp_P(reqUrl, PSTR("changeurl"))) {
      HandleChangeURL(&client, params);
    } else {
      WebError(&client, 404, NULL, false);
    }

    // web clients hate when door is violently shut
    while (client.available()) {
      PumpDecoder();
      client.read();
    }
  }

  PumpDecoder();
  if (client) {
    client.flush();
    client.stop();
  }
}
