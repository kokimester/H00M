#pragma once

#include "FileObserver.h"
#include <vector>
#include <windows.h>
#include <fileapi.h>
#include <handleapi.h>

class FileObserverWindows : public FileObserver {
private:
  static constexpr DWORD bufferSize = 4096;
  DWORD buffer[bufferSize];
  OVERLAPPED overlapped = {};
  bool pending          = false;

  std::vector<WatchItem<void*>> watchlist;

  bool resetWatch(HANDLE handle);

public:
  FileObserverWindows();
  virtual ~FileObserverWindows() = default;
  virtual void add_watch(const fs::path&) final override;
  virtual void remove_watch(const fs::path&) final override;
  virtual void update() final override;
};
