#include "FileObserverLinux.h"
#include <algorithm>
#include <print>
#include <unistd.h>

FileObserverLinux::FileObserverLinux() {
  fd = inotify_init1(IN_NONBLOCK);
  if (fd < 0) {
    std::println("Failed to create inotify instance!");
  }
}

void FileObserverLinux::add_watch(const fs::path& path) {
  auto itr =
      std::find_if(watchlist.begin(), watchlist.end(),
                   [&path](auto& item) { return item.file_path == path; });
  if (itr != watchlist.end()) {
    // path is already added to watchlist, skip
    return;
  }
  WatchItem wi{.file_path = path, .watch_id = 0};
  wi.watch_id = inotify_add_watch(fd, path.string().c_str(), IN_CLOSE_WRITE);
  if (wi.watch_id < 0) {
    std::println("Failed to create inotify watch!");
  }
  watchlist.push_back(wi);
}
void FileObserverLinux::remove_watch(const fs::path& path) {
  auto itr =
      std::find_if(watchlist.begin(), watchlist.end(),
                   [&path](auto& item) { return item.file_path == path; });
  if (itr == watchlist.end()) {
    // path is not on the watch list
    return;
  }
  if (inotify_rm_watch(fd, itr->watch_id) < 0) {
    std::println("Failed to remove inotify watch!");
  }
  watchlist.erase(itr);
}
void FileObserverLinux::update() {
  int poll_size = read(fd, buf, sizeof(buf));
  if (poll_size > 0) {
    std::println("Polled filesystem changes");
    const struct inotify_event* event;
    event    = (const struct inotify_event*)buf;
    int wd   = event->wd;
    auto itr = std::find_if(watchlist.begin(), watchlist.end(),
                            [&wd](auto& item) { return item.watch_id == wd; });
    if (watchlist.end() != itr)
      std::println("Polled file changes: {}", itr->file_path.string());
    if (event->mask == IN_CLOSE_WRITE) {
      std::println("Notifying all subscribers");
      notify_subscribers();
    }
  }
}
