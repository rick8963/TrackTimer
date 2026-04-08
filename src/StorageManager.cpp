#include "StorageManager.h"

StorageManager::StorageManager(fs::FS &fs) : _fs(fs) {}

bool StorageManager::begin() {
  Serial.println("[Storage] Using internal SPIFFS");
  return true;
}

String StorageManager::createNewLogFile(const DateTimeInfo &t) {
  closeCurrentFile();

  String filename;
  if (t.isValid()) {
    char buf[32];
    snprintf(buf, sizeof(buf), "/%04d%02d%02d_%02d%02d%02d.nmea",
             t.year, t.month, t.day, t.hour, t.minute, t.second);
    filename = String(buf);
  } else {
    filename = String("/boot_") + String(millis()) + ".nmea";
  }

  _currentFile = _fs.open(filename, FILE_WRITE);
  if (!_currentFile) {
    Serial.println("[Storage] Failed to open log file for writing");
    _currentFilePath = "";
    return "";
  }

  _currentFilePath = filename;
  _currentFileSize = 0;
  Serial.print("[Storage] New log file: ");
  Serial.println(_currentFilePath);
  return _currentFilePath;
}

bool StorageManager::appendLine(const String &line) {
  if (!_currentFile) {
    return false;
  }

  String out = line;
  if (!out.endsWith("\r\n")) {
    out += "\r\n";
  }

  size_t written = _currentFile.print(out);
  _currentFileSize += written;

  if (_currentFileSize >= MAX_LOG_FILE_SIZE) {
    Serial.println("[Storage] Max file size reached, closing file");
    closeCurrentFile();
  }

  return written == out.length();
}

void StorageManager::closeCurrentFile() {
  if (_currentFile) {
    Serial.print("[Storage] Closing file: ");
    Serial.println(_currentFilePath);
    _currentFile.close();
  }
  _currentFilePath = "";
  _currentFileSize = 0;
}

std::vector<FileInfo> StorageManager::listLogFiles() {
  std::vector<FileInfo> files;

  File root = _fs.open(LOG_FOLDER);
  if (!root || !root.isDirectory()) {
    Serial.println("[Storage] Root is not a directory");
    return files;
  }

  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String name = file.name();
      if (name.endsWith(".nmea")) {
        FileInfo info;
        info.name = name;
        info.size = file.size();
        files.push_back(info);
      }
    }
    file = root.openNextFile();
  }

  return files;
}

fs::FS &StorageManager::fs() {
  return _fs;
}