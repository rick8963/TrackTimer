#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "Config.h"

class StorageManager {
public:
  explicit StorageManager(fs::FS &fs);
  bool begin();
  String createNewLogFile(const DateTimeInfo &t);
  bool appendLine(const String &line);
  void closeCurrentFile();
  std::vector<FileInfo> listLogFiles();
  fs::FS &fs();

private:
  fs::FS &_fs;
  File _currentFile;
  String _currentFilePath;
  size_t _currentFileSize = 0;
};

#endif