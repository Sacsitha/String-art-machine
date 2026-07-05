/*
  String Art ESP32-CAM Web Server
  Board: AI Thinker ESP32-CAM

  Features:
  - ESP32-CAM hosts its own web page
  - Capture Photo button
  - Captured image preview
  - Direct image URL: http://ESP32_IP/saved-photo.jpg
  - Download captured image
  - Optional upload to processing app API when they give the API URL
*/

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ---------------- WIFI SETTINGS ----------------
// Option A: Connect ESP32-CAM to your Wi-Fi router
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Option B fallback: if router Wi-Fi fails, ESP32 creates this hotspot
const char* AP_SSID = "StringArt-CAM";
const char* AP_PASSWORD = "12345678";

// ---------------- AI THINKER ESP32-CAM PINS ----------------
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WebServer server(80);

// Last captured image is stored in RAM.
// Keep resolution moderate because ESP32-CAM RAM is limited.
uint8_t* lastImage = nullptr;
size_t lastImageLen = 0;
unsigned long captureCount = 0;

const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>String Art Camera</title>
  <style>
    body { margin:0; font-family: Arial, sans-serif; background:#111; color:#f5f5f5; }
    .nav { padding:18px 28px; border-bottom:1px solid #333; display:flex; justify-content:space-between; align-items:center; }
    .brand { font-size:22px; font-weight:bold; color:#cf9c2c; }
    .hero { padding:40px 20px; max-width:980px; margin:auto; }
    h1 { font-size:42px; margin:0 0 12px; line-height:1.1; }
    p { color:#ccc; font-size:17px; line-height:1.6; }
    .card { background:#1c1c1c; border:1px solid #333; border-radius:18px; padding:22px; margin-top:24px; }
    .buttons { display:flex; flex-wrap:wrap; gap:12px; margin:18px 0; }
    button, a.btn { border:0; border-radius:10px; padding:13px 18px; font-weight:bold; cursor:pointer; text-decoration:none; display:inline-block; }
    button.primary { background:#cf9c2c; color:#111; }
    a.btn { background:#333; color:white; }
    button:disabled { opacity:.6; cursor:not-allowed; }
    img { max-width:100%; border-radius:14px; border:1px solid #444; margin-top:15px; background:#000; }
    .status { color:#cf9c2c; font-weight:bold; }
    code { background:#000; padding:3px 6px; border-radius:5px; color:#9ef; }
    .small { font-size:14px; color:#aaa; }
  </style>
</head>
<body>
  <div class="nav">
    <div class="brand">String Art Camera</div>
    <div>ESP32-CAM Server</div>
  </div>

  <section class="hero">
    <h1>From Pixels to Strings.<br>Capture image for thread pattern design.</h1>
    <p>Click the button below. The ESP32-CAM will capture a photo and show it here. Then you can download it, open the direct image URL, or later upload it to the image-processing app.</p>

    <div class="card">
      <h2>Camera Capture</h2>
      <div class="buttons">
        <button id="captureBtn" class="primary" onclick="capturePhoto()">Capture Photo</button>
        <a id="downloadBtn" class="btn" href="/saved-photo.jpg" download="string-art-photo.jpg">Download Image</a>
        <a id="openBtn" class="btn" href="/saved-photo.jpg" target="_blank">Open Image URL</a>
      </div>
      <div class="status" id="status">Ready</div>
      <img id="preview" src="" alt="Captured image preview will appear here" style="display:none;" />
      <p class="small">Direct image URL after capture: <code id="imageUrl">/saved-photo.jpg</code></p>
    </div>

    <div class="card">
      <h2>For Image Processing App</h2>
      <p>Temporary method: capture the photo, download it, then upload/import it manually to their app.</p>
      <p>Best final method: their app should read <code>http://ESP32_IP/saved-photo.jpg</code> or provide an API endpoint so this webpage can upload the photo automatically.</p>
    </div>
  </section>

<script>
const statusText = document.getElementById('status');
const preview = document.getElementById('preview');
const captureBtn = document.getElementById('captureBtn');
const imageUrl = document.getElementById('imageUrl');

imageUrl.textContent = window.location.origin + '/saved-photo.jpg';

async function capturePhoto() {
  captureBtn.disabled = true;
  statusText.textContent = 'Capturing photo...';
  try {
    const response = await fetch('/capture', { cache: 'no-store' });
    if (!response.ok) throw new Error('Capture failed');
    const data = await response.json();
    const url = '/saved-photo.jpg?t=' + Date.now();
    preview.src = url;
    preview.style.display = 'block';
    statusText.textContent = 'Photo captured successfully. Size: ' + data.bytes + ' bytes';
  } catch (error) {
    statusText.textContent = 'Error: ' + error.message;
  }
  captureBtn.disabled = false;
}

// Future automatic app upload example.
// When your friend gives the processing app API URL, replace APP_UPLOAD_URL.
async function uploadToProcessingApp() {
  const APP_UPLOAD_URL = 'http://PROCESSING_APP_IP_OR_DOMAIN/upload';
  const imageResponse = await fetch('/saved-photo.jpg');
  const blob = await imageResponse.blob();
  const formData = new FormData();
  formData.append('image', blob, 'string-art-photo.jpg');
  const response = await fetch(APP_UPLOAD_URL, { method: 'POST', body: formData });
  return await response.text();
}
</script>
</body>
</html>
)rawliteral";

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;   // 800x600
    config.jpeg_quality = 12;             // lower number = better quality/larger file
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_VGA;    // 640x480
    config.jpeg_quality = 14;
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  }

  esp_err_t err = esp_camera_init(&config);
  return err == ESP_OK;
}

void handleRoot() {
  server.send_P(200, "text/html", MAIN_PAGE);
}

void handleCapture() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "application/json", "{\"ok\":false,\"error\":\"Camera capture failed\"}");
    return;
  }

  if (lastImage != nullptr) {
    free(lastImage);
    lastImage = nullptr;
    lastImageLen = 0;
  }

  lastImage = (uint8_t*) malloc(fb->len);
  if (!lastImage) {
    esp_camera_fb_return(fb);
    server.send(500, "application/json", "{\"ok\":false,\"error\":\"Not enough RAM to save image\"}");
    return;
  }

  memcpy(lastImage, fb->buf, fb->len);
  lastImageLen = fb->len;
  captureCount++;
  esp_camera_fb_return(fb);

  String json = "{\"ok\":true,\"bytes\":" + String(lastImageLen) + ",\"count\":" + String(captureCount) + "}";
  server.send(200, "application/json", json);
}

void handleSavedPhoto() {
  if (lastImage == nullptr || lastImageLen == 0) {
    server.send(404, "text/plain", "No photo captured yet. Open / and click Capture Photo first.");
    return;
  }
  server.sendHeader("Content-Disposition", "inline; filename=string-art-photo.jpg");
  server.send_P(200, "image/jpeg", (const char*)lastImage, lastImageLen);
}

void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}

void startWiFi() {
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("Open this URL: http://");
    Serial.println(WiFi.localIP());
    if (MDNS.begin("stringartcam")) {
      Serial.println("mDNS: http://stringartcam.local");
    }
  } else {
    Serial.println();
    Serial.println("WiFi failed. Starting ESP32-CAM hotspot...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.print("Connect phone/laptop to WiFi: ");
    Serial.println(AP_SSID);
    Serial.print("Password: ");
    Serial.println(AP_PASSWORD);
    Serial.print("Then open: http://");
    Serial.println(WiFi.softAPIP());
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  delay(1000);

  if (!initCamera()) {
    Serial.println("Camera init failed. Check board type and camera ribbon cable.");
    while (true) delay(1000);
  }
  Serial.println("Camera init OK");

  startWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/saved-photo.jpg", HTTP_GET, handleSavedPhoto);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
