#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

using namespace websockets;

const char* ssid = "Dong Xanh";
const char* password = "12345678";
const char* websocket_server_host = "192.168.1.22"; 
const uint16_t websocket_server_port = 8888;

WebsocketsClient client;
unsigned long lastReconnect = 0;

// =========================
// HÀM KẾT NỐI WEBSOCKET
// =========================
bool connectWebSocket() {
  Serial.println("Dang thu ket noi WebSocket...");
  bool ok = client.connect(websocket_server_host, websocket_server_port, "/");

  if (ok) 
    Serial.println(">>> WebSocket CONNECTED!");
  else
    Serial.println(">>> WebSocket FAILED!");

  return ok;
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // Camera config
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
  config.frame_size = FRAMESIZE_QVGA; 
  config.jpeg_quality = 18; 
  config.fb_count = 2;

  esp_camera_init(&config);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi OK");

  // Kết nối WebSocket
  connectWebSocket();
}

void loop() {
  client.poll();  // giữ kết nối WebSocket sống

  // =========================
  // Nếu WebSocket MẤT KẾT NỐI → Tự reconnect mỗi 2 giây
  // =========================
  if (!client.available()) {
    if (millis() - lastReconnect > 2000) {
      Serial.println("WebSocket mat ket noi → thu ket noi lai...");
      connectWebSocket();
      lastReconnect = millis();
    }
    return;
  }

  // =========================
  // STREAM CAMERA MỖI 80–120ms
  // =========================
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Capture FAILED");
    return;
  }

  // Gửi ảnh qua WebSocket
  client.sendBinary((const char*)fb->buf, fb->len);

  // Giải phóng frame buffer
  esp_camera_fb_return(fb);

  delay(80); // giới hạn ~12 FPS, không nghẽn CPU
}
