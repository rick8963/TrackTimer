#include <FS.h>
#include <SPIFFS.h>
#include "WebInterface.h"

HardwareSerial GPS_SERIAL(1);
WebServer server(80);

WebInterface::WebInterface(StorageManager &storage) : _storage(storage) {
  memset(_liveBuf, 0, sizeof(_liveBuf));
}

void WebInterface::begin() {
  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/download", HTTP_GET, [this]() { handleDownload(); });
  server.on("/storage", HTTP_GET, [this]() { handleStorageInfo(); });
  server.on("/delete", HTTP_POST, [this]() { handleDelete(); });
  server.on("/list", HTTP_GET, [this]() { handleFileList(); });
  registerExtraRoutes();
  server.begin();
  Serial.println("[Web] HTTP server started on port 80");
}

void WebInterface::handleClient() {
  server.handleClient();
}

// ── circular buffer push ────────────────────────────────────────────────────
void WebInterface::pushNmeaLine(const String &line) {
  // write into current head slot (truncate if too long)
  strncpy(_liveBuf[_liveHead], line.c_str(), LIVE_LINE_MAX - 1);
  _liveBuf[_liveHead][LIVE_LINE_MAX - 1] = '\0';
  _liveHead = (_liveHead + 1) % LIVE_BUF_LINES;
  if (_liveCount < LIVE_BUF_LINES) _liveCount++;

  // Push to SSE client if connected
  if (_sseActive && _sseClient && _sseClient.connected()) {
    String msg = "data: ";
    msg += line;
    msg += "\n\n";
    size_t written = _sseClient.print(msg);
    if (written == 0) {
      _sseActive = false;  // client disconnected
      _sseClient.stop();
    }
  }

  if (_sseConnected) {
    if (_sseClient.connected()) {
      String msg = "data: " + line + "\n\n";
      if (_sseClient.print(msg) == 0) {
        _sseConnected = false;  // 寫入失敗 = 斷線
        _sseClient.stop();
      }
    } else {
      _sseConnected = false;  // client 已斷線
    }
  }
}

bool WebInterface::isSSEConnected() const {
  return _sseConnected && _sseConnected;
}

void WebInterface::handleDelete() {
  if (!server.hasArg("name")) {
    server.send(400, "text/plain", "Missing 'name' parameter");
    return;
  }

  String name = server.arg("name");
  if (name.indexOf("..") >= 0) {
    server.send(400, "text/plain", "Invalid file name");
    return;
  }
  if (!name.startsWith("/")) name = "/" + name;

  fs::FS &fs = _storage.fs();
  if (!fs.exists(name)) {
    server.send(404, "text/plain", "File not found: " + name);
    return;
  }

  if (!fs.remove(name)) {
    server.send(500, "text/plain", "Failed to delete: " + name);
    return;
  }

  // 刪除成功，回傳 200 OK
  server.send(200, "text/plain", "Deleted: " + name.substring(1));
}

// ── SSE endpoint /live ──────────────────────────────────────────────────────
void WebInterface::handleLiveSSE() {
  if (_sseConnected) {
    _sseConnected = false;
    _sseClient.stop();
  }  
  _sseClient = server.client();
  _sseConnected = true;

  // Only one SSE client at a time to keep memory minimal
  if (_sseActive && _sseClient && _sseClient.connected()) {
    _sseClient.stop();
  }

  _sseClient = server.client();
  _sseActive = true;

  // Send SSE headers
  _sseClient.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/event-stream\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: keep-alive\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "\r\n"
  );

  // Replay buffered lines so page shows history immediately
  uint8_t start = (_liveCount < LIVE_BUF_LINES)
                    ? 0
                    : _liveHead;  // oldest slot
  for (uint8_t i = 0; i < _liveCount; i++) {
    uint8_t idx = (start + i) % LIVE_BUF_LINES;
    String msg = "data: ";
    msg += _liveBuf[idx];
    msg += "\n\n";
    _sseClient.print(msg);
  }
}

// ── root page ───────────────────────────────────────────────────────────────
void WebInterface::handleRoot() {
  std::vector<FileInfo> files = _storage.listLogFiles();

  const size_t SPIFFS_SIZE = 0x360000;
  size_t usedBytes = 0;
  for (const auto &f : files) {
    usedBytes += f.size;
  }
  size_t freeBytes = (usedBytes < SPIFFS_SIZE) ? (SPIFFS_SIZE - usedBytes) : 0;

  auto toHuman = [](size_t bytes) -> String {
    if (bytes < 1024) return String(bytes) + " B";
    else if (bytes < 1024*1024) return String(bytes/1024) + " KB";
    else return String(bytes/1024/1024) + " MB";
  };

  float percent = (float)usedBytes / SPIFFS_SIZE * 100.0f;

  String html;
  html.reserve(4000);
  html += "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>ESP32 GPS NMEA Logs</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;margin:20px;background:#f8f9fa;}";
  html += "h1{font-size:1.5em;margin-bottom:10px;color:#333;}";
  html += "table{width:100%;border-collapse:collapse;margin:20px 0;background:white;box-shadow:0 2px 4px rgba(0,0,0,0.1);border-radius:8px;overflow:hidden;}";
  html += "th,td{border-bottom:1px solid #eee;padding:12px 16px;text-align:left;}";
  html += "th{background:#f1f3f4;font-weight:600;color:#555;}";
  html += "tr:hover{background:#f9f9f9;}";
  html += "a{color:#1a73e8;text-decoration:none;font-weight:500;}a:hover{text-decoration:underline;}";
  html += ".btn{padding:8px 16px;border:none;border-radius:6px;cursor:pointer;font-weight:500;font-size:14px;transition:all 0.2s;}";
  html += ".btn-download{background:#1a73e8;color:white;}";
  html += ".btn-delete{background:#ea4335;color:white;}";
  html += ".btn-live{background:#34a853;color:white;margin-right:10px;}";
  html += ".btn-live.stop{background:#ea4335;}";
  html += "#liveBox{height:300px;overflow-y:auto;margin-top:10px;border:1px solid #ddd;padding:16px;background:#1e1e1e;color:#e8eaed;font-family:monospace;font-size:13px;border-radius:8px;box-shadow:inset 0 2px 4px rgba(0,0,0,0.1);}";
  html += "#storageInfo{margin:20px 0;padding:20px;border:1px solid #ddd;background:white;border-radius:12px;box-shadow:0 4px 8px rgba(0,0,0,0.1);}";
  html += ".bar-container{display:flex;align-items:center;gap:12px;}";
  html += ".bar{width:250px;height:16px;background:#e8eaed;border-radius:8px;overflow:hidden;box-shadow:inset 0 1px 3px rgba(0,0,0,0.1);}";
  html += ".bar-fill{height:100%;background:linear-gradient(90deg,#34a853,#1e7e34);transition:width 0.5s ease;border-radius:8px;}";
  html += ".stats{display:flex;gap:20px;font-size:14px;color:#666;}";
  html += "@media(max-width:600px){.stats{flex-direction:column;gap:8px;}}";
  html += "</style></head><body>";

  html += "<h1>📡 ESP32 GPS NMEA Logs</h1>";

  // 儲存空間資訊
  html += "<div id='storageInfo'>";
  html += "<div class='stats'>";
  html += "<span>📊 已用 <strong id='usedSpace'>" + toHuman(usedBytes) + "</strong></span>";
  html += "<span>📈 剩餘 <strong id='freeSpace'>" + toHuman(freeBytes) + "</strong></span>";
  html += "<span>💾 總計 <strong id='totalSpace'>" + toHuman(SPIFFS_SIZE) + "</strong></span>";
  html += "</div>";
  html += "<div class='bar-container'>";
  html += "<span>" + String(percent, 1) + "%</span>";
  html += "<div class='bar'><div class='bar-fill' id='barFill' style='width:" + String(percent, 1) + "%'></div></div>";
  html += "</div>";
  html += "</div>";

  // 檔案列表
  html += "<table><thead><tr><th>📄 檔名</th><th>📏 大小</th><th>⬇️ 下載</th><th>🗑️ 刪除</th></tr></thead><tbody id='fileTable'>";
  for (const auto &f : files) {
    html += "<tr><td>" + f.name + "</td><td>" + toHuman(f.size) + "</td>";
    html += "<td><a href='/download?name=" + urlEncode(f.name) + "' class='btn btn-download'>下載</a></td>";
    html += "<td><button class='btn btn-delete' onclick=\"confirmDelete('" + f.name + "')\">刪除</button></td></tr>";
  }
  html += "</tbody></table>";

  // Live View
  html += "<h2>🔴 Live NMEA 即時監控</h2>";
  html += "<button id='liveBtn' class='btn btn-live' onclick='toggleLive()'>▶️ 開始監控</button>";
  html += "<button class='btn' onclick='clearBox()' style='background:#666;color:white;'>🗑️ 清除</button>";
  html += "<div id='liveBox'></div>";

html += R"(
<script>
let es = null;
const MAX_LINES = 200;

function toggleLive() {
  const btn = document.getElementById('liveBtn');
  if (es) {
    es.close(); es = null;
    btn.textContent = '▶️ 開始監控';
    btn.className = 'btn btn-live';
    return;
  }
  const box = document.getElementById('liveBox');
  es = new EventSource('/live');
  btn.textContent = '⏹️ 停止監控';
  btn.className = 'btn btn-live stop';
  es.onmessage = function(e) {
    box.textContent += e.data + '\n';
    const lines = box.textContent.split('\n');
    if (lines.length > MAX_LINES + 1)
      box.textContent = lines.slice(-MAX_LINES).join('\n');
    box.scrollTop = box.scrollHeight;
  };
  es.onerror = function() {
    es.close(); es = null;
    btn.textContent = '▶️ 開始監控';
    btn.className = 'btn btn-live';
  };
}

function clearBox() {
  document.getElementById('liveBox').textContent = '';
}

function toHuman(bytes) {
  if (bytes < 1024) return bytes + ' B';
  if (bytes < 1024*1024) return Math.round(bytes/1024) + ' KB';
  return Math.round(bytes/1024/1024) + ' MB';
}

function updateStorageInfo() {
  fetch('/storage')
    .then(r => r.json())
    .then(data => {
      document.getElementById('usedSpace').textContent = toHuman(data.used);
      document.getElementById('freeSpace').textContent = toHuman(data.free);
      document.getElementById('totalSpace').textContent = toHuman(data.total);
      document.getElementById('barFill').style.width = data.percent + '%';
    })
    .catch(() => console.log('更新空間失敗'));
}

function confirmDelete(filename) {
  if (confirm('確定刪除 ' + filename + ' 嗎？\n這會立即釋放儲存空間。')) {
    fetch('/delete', {
      method: 'POST',
      headers: {'Content-Type': 'application/x-www-form-urlencoded'},
      body: 'name=' + encodeURIComponent(filename)
    })
    .then(r => r.text())
    .then(data => {
      alert(data);
      updateStorageInfo();
      location.reload();  // 重整頁面更新表格
    })
    .catch(err => alert('刪除失敗: ' + err));
  }
}
</script>
)";

  html += "</body></html>";
  server.send(200, "text/html; charset=utf-8", html);
}
// ── download handler ─────────────────────────────────────────────────────────
void WebInterface::handleDownload() {
  if (!server.hasArg("name")) {
    server.send(400, "text/plain", "Missing 'name' parameter");
    return;
  }
  String name = server.arg("name");
  if (name.indexOf("..") >= 0) { server.send(400, "text/plain", "Invalid file name"); return; }
  if (!name.startsWith("/")) name = "/" + name;

  File file = _storage.fs().open(name, FILE_READ);
  if (!file) { server.send(404, "text/plain", "File not found"); return; }

  size_t fileSize = file.size();
  server.setContentLength(fileSize);
  server.sendHeader("Content-Type", "text/plain");
  server.sendHeader("Content-Disposition", "attachment; filename=\"" + name.substring(1) + "\"");
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", "");

  WiFiClient client = server.client();
  const size_t BUF_SIZE = 512;
  uint8_t buf[BUF_SIZE];
  while (file.available()) {
    size_t n = file.read(buf, BUF_SIZE);
    if (n == 0) break;
    client.write(buf, n);
  }
  file.close();
}

// ── helpers ──────────────────────────────────────────────────────────────────
String WebInterface::urlEncode(const String &value) {
  String encoded = "";
  char bufHex[4];
  for (size_t i = 0; i < value.length(); i++) {
    char c = value.charAt(i);
    if (isalnum((unsigned char)c) || c=='-'||c=='_'||c=='.'||c=='~') {
      encoded += c;
    } else {
      snprintf(bufHex, sizeof(bufHex), "%%%02X", (unsigned char)c);
      encoded += bufHex;
    }
  }
  return encoded;
}

void WebInterface::registerExtraRoutes() {
  server.on("/live", HTTP_GET, [this]() { handleLiveSSE(); });
}

void WebInterface::handleStorageInfo() {
  std::vector<FileInfo> files = _storage.listLogFiles();

  const size_t SPIFFS_SIZE = 0x360000;  // 來自你的 default_16MB.csv

  // 計算所有 .nmea 檔案的總大小
  size_t usedBytes = 0;
  for (const auto &f : files) {
    usedBytes += f.size;
  }
  size_t freeBytes = (usedBytes < SPIFFS_SIZE) ? (SPIFFS_SIZE - usedBytes) : 0;

  String json;
  json.reserve(128);
  json += "{";
  json += "\"total\":" + String(SPIFFS_SIZE) + ",";
  json += "\"used\":" + String(usedBytes) + ",";
  json += "\"free\":" + String(freeBytes) + ",";
  json += "\"percent\":" + String((float)usedBytes / SPIFFS_SIZE * 100.0f, 1);
  json += "}";

  server.send(200, "application/json", json);
}

void WebInterface::handleFileList() {
  std::vector<FileInfo> files = _storage.listLogFiles();
  String json = "[";
  for (size_t i = 0; i < files.size(); i++) {
    if (i > 0) json += ",";
    json += "{\"name\":\"" + files[i].name + "\",\"size\":" + String(files[i].size) + "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}