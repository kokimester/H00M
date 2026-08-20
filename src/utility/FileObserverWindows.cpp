#ifdef _WIN32
#include "FileObserverWindows.h"
#include <algorithm>
#include <print>
#include <string>
#include <unistd.h>

void PrintCSBackupAPIErrorMessage(DWORD dwErr) {

  char wszMsgBuff[512]; // Buffer for text.

  DWORD dwChars; // Number of chars returned.

  // Try to get the message from the system errors.
  dwChars =
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, dwErr, 0, wszMsgBuff, 512, NULL);

  if (0 == dwChars) {
    // The error code did not exist in the system errors.
    // Try Ntdsbmsg.dll for the error code.

    HINSTANCE hInst;

    // Load the library.
    hInst = LoadLibrary("Ntdsbmsg.dll");
    if (NULL == hInst) {
      printf("cannot load Ntdsbmsg.dll\n");
      exit(1); // Could 'return' instead of 'exit'.
    }

    // Try getting message text from ntdsbmsg.
    dwChars = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                            hInst, dwErr, 0, wszMsgBuff, 512, NULL);

    // Free the library.
    FreeLibrary(hInst);
  }

  // Display the error message, or generic text if not found.
  printf("Error value: %d Message: %s\n", dwErr,
         dwChars ? wszMsgBuff : "Error message not found.");
}

FileObserverWindows::FileObserverWindows() {}

void FileObserverWindows::add_watch(const fs::path& path) {

  auto itr =
      std::find_if(watchlist.begin(), watchlist.end(),
                   [&path](auto& item) { return item.file_path == path; });
  if (itr != watchlist.end()) {
    // path is already added to watchlist, skip
    return;
  }
  auto directoryPath =
      path.string().substr(0, path.string().find_last_of('\\'));
  auto directoryHandle =
      CreateFileA(directoryPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                  OPEN_EXISTING,
                  FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

  if ((INVALID_HANDLE_VALUE == directoryHandle) ||
      (nullptr == directoryHandle)) {
    std::println("{}", path.string());
    std::println("Failed to create Windows file notification handle : {}!",
                 directoryHandle);
    PrintCSBackupAPIErrorMessage(GetLastError());
    return;
  }

  overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  watchlist.emplace_back(
      WatchItem<void*>{.file_path = path, .handle = directoryHandle});
  resetWatch(directoryHandle);
}
void FileObserverWindows::remove_watch(const fs::path& path) {
  auto itr =
      std::find_if(watchlist.begin(), watchlist.end(),
                   [&path](auto& item) { return item.file_path == path; });
  if (itr == watchlist.end()) {
    // path is not on the watch list
    return;
  }
  if (FindCloseChangeNotification(itr->handle) < 0) {
    std::println("Failed to Windows change notification!");
  }
  watchlist.erase(itr);
}

bool FileObserverWindows::resetWatch(HANDLE handle) {
  // Call resetWatch() once after open, and again after each successful read

  ResetEvent(overlapped.hEvent);
  BOOL ok = ReadDirectoryChangesW(handle, buffer, sizeof(buffer), FALSE,
                                  FILE_NOTIFY_CHANGE_LAST_WRITE |
                                      FILE_NOTIFY_CHANGE_FILE_NAME,
                                  nullptr, &overlapped, nullptr);

  pending = ok || GetLastError() == ERROR_IO_PENDING;
  return pending;
}

void FileObserverWindows::update() {
  std::for_each(watchlist.begin(), watchlist.end(), [this](auto item) {
    if (!pending)
      return;

    DWORD transferred = 0;
    // FALSE = don't block, return immediately if not done
    if (!GetOverlappedResult(item.handle, &overlapped, &transferred, FALSE)) {
      // ERROR_IO_INCOMPLETE means simply "not ready yet" — not an error
      if (GetLastError() == ERROR_IO_INCOMPLETE)
        return;
    }

    // Walk the result buffer
    auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
    while (true) {
      // Convert wide filename to narrow
      int len = WideCharToMultiByte(CP_UTF8, 0, info->FileName,
                                    info->FileNameLength / sizeof(WCHAR),
                                    nullptr, 0, nullptr, nullptr);
      std::string name(len, '\0');
      WideCharToMultiByte(CP_UTF8, 0, info->FileName,
                          info->FileNameLength / sizeof(WCHAR), name.data(),
                          len, nullptr, nullptr);
      std::println("Changes found at: {}", name);
      std::string_view observedFile = item.file_path.string();
      std::println("Subscribed file: {}", observedFile);
      if (observedFile.ends_with(name)) {
        std::println("Found observed file! Notifying subscribers.");
        notify_subscribers();
      }
      if (info->NextEntryOffset == 0)
        break;
      info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
          reinterpret_cast<BYTE*>(info) + info->NextEntryOffset);
    }
    resetWatch(item.handle); // re-arm for next poll
  });
}
#endif
