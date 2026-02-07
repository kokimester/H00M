#pragma once

#include "Observer.h"
#include <filesystem>

namespace fs = std::filesystem;

class FileObserver : public Observer {
public:
  FileObserver()          = default;
  virtual ~FileObserver() = default;

  virtual void add_watch(const fs::path&)    = 0;
  virtual void remove_watch(const fs::path&) = 0;
  virtual void update()                      = 0;
};
