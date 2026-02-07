#pragma once

#include "FileObserver.h"
#include <sys/inotify.h>
#include <vector>

class FileObserverLinux : public FileObserver {
private:
  int fd = 0;
  struct WatchItem {
    fs::path file_path;
    int watch_id;
  };
  std::vector<WatchItem> watchlist;
  // only for a single event, that is a file not a directory
  uint8_t buf[sizeof(struct inotify_event)];

public:
  FileObserverLinux();
  virtual ~FileObserverLinux() = default;
  virtual void add_watch(const fs::path&) final override;
  virtual void remove_watch(const fs::path&) final override;
  virtual void update() final override;
};
