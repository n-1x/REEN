#pragma once
#include <Windows.h>
#include <cstddef>
#include <list>
#include <memory>
#include <memoryapi.h>
#include <optional>
#include <string>

DWORD const MEMORY_PROTECT_WRITABLE = PAGE_EXECUTE_READWRITE | PAGE_READWRITE |
                                      PAGE_EXECUTE_WRITECOPY | PAGE_WRITECOPY;

struct Page {
  LPCVOID baseAddress;
  SIZE_T regionSize;
  std::list<size_t> offsets;
};

using scanResult = std::list<Page>;

std::optional<DWORD> GetProcessIDByName(std::string const &processName);
HANDLE GetProcessHandleByID(DWORD const processID);

template <typename T>
void ScanPage(HANDLE const hProcess, Page &page, T const target) {
  bool foundAny = false;
  size_t bytesRead = 0;

  size_t const bufferLength = page.regionSize / sizeof(T);
  std::unique_ptr<T[]> buffer(new T[bufferLength]);

  ReadProcessMemory(hProcess, page.baseAddress, buffer.get(), page.regionSize,
                    &bytesRead);

  for (size_t i = 0; i < bufferLength; ++i) {
    if (buffer[i] == target) {
      page.offsets.push_back(i);
    }
  }
}

template <typename T>
void RefineScan(HANDLE const hProcess, scanResult &workingResult,
                T const target) {
  for (auto resultIt = workingResult.begin();
       resultIt != workingResult.end();) {
    auto page = *resultIt;
    size_t const bufferLength = page.regionSize / sizeof(T);
    std::unique_ptr<T[]> buffer(new T[bufferLength]);

    ReadProcessMemory(hProcess, page.baseAddress, buffer.get(), page.regionSize,
                      NULL);

    for (auto offsetIt = page.offsets.begin();
         offsetIt != page.offsets.end();) {
      auto const offset = *offsetIt;
      auto const newValue = buffer[offset];

      if (newValue != target) {
        offsetIt = page.offsets.erase(offsetIt);
      } else {
        ++offsetIt;
      }
    }

    if (page.offsets.size() == 0) {
      resultIt = workingResult.erase(resultIt);
    } else {
      ++resultIt;
    }
  }
}

template <typename T>
scanResult FindInitialAddresses(HANDLE const hProcess, T target) {
  MEMORY_BASIC_INFORMATION mbi;
  scanResult result;

  bool done = false;
  LPCVOID currentBase = 0;
  while (!done) {
    size_t const bytesWitten = VirtualQueryEx(hProcess, currentBase, &mbi,
                                              sizeof(MEMORY_BASIC_INFORMATION));
    if (bytesWitten == 0) {
      done = true;
    } else if (mbi.Protect & MEMORY_PROTECT_WRITABLE) {
      Page page{mbi.BaseAddress, mbi.RegionSize};
      ScanPage(hProcess, page, target);

      if (page.offsets.size() > 0) {
        result.push_back(page);
      }
    }
    currentBase = (std::byte *)mbi.BaseAddress + mbi.RegionSize;
  }

  return result;
}