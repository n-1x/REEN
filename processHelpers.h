#pragma once
#include <Windows.h>
#include <cassert>
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

struct ScanResult {
  size_t addressCount = 0;
  size_t maxPageSize = 0;
  std::list<Page> pages;
};

std::optional<DWORD> GetProcessIDByName(std::string const &processName);
HANDLE GetProcessHandleByID(DWORD const processID);

template <typename T>
void ScanPage(HANDLE const hProcess, Page &page, T const target) {
  bool foundAny = false;
  size_t bytesRead = 0;

  size_t const bufferLength = page.regionSize;
  std::unique_ptr<std::byte[]> buffer(new std::byte[bufferLength]);

  ReadProcessMemory(hProcess, page.baseAddress, buffer.get(), page.regionSize,
                    &bytesRead);

  for (size_t i = 0; i < bufferLength; i += sizeof(T)) {
    if (*((T *)&buffer[i]) == target) {
      page.offsets.push_back(i);
    }
  }
}

template <typename T>
void RefineScan(HANDLE const hProcess, ScanResult &workingResult,
                T const target) {
  std::unique_ptr<std::byte[]> buffer(new std::byte[workingResult.maxPageSize]);

  for (auto resultIt = workingResult.pages.begin();
       resultIt != workingResult.pages.end();) {
    Page &page = *resultIt;
    size_t const bufferLength = page.regionSize / sizeof(T);

    ReadProcessMemory(hProcess, page.baseAddress, buffer.get(), page.regionSize,
                      NULL);

    for (auto offsetIt = page.offsets.begin();
         offsetIt != page.offsets.end();) {
      auto const offset = *offsetIt;
      T const newValue = *((T *)&buffer[offset]);

      if (newValue != target) {
        offsetIt = page.offsets.erase(offsetIt);
        --workingResult.addressCount;
      } else {
        ++offsetIt;
      }
    }

    if (page.offsets.size() == 0) {
      resultIt = workingResult.pages.erase(resultIt);
    } else {
      ++resultIt;
    }
  }
}

template <typename T>
ScanResult FindInitialAddresses(HANDLE const hProcess, T target) {
  MEMORY_BASIC_INFORMATION mbi;
  ScanResult result;

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
        result.pages.push_back(page);
        result.addressCount += page.offsets.size();
        result.maxPageSize = max(result.maxPageSize, page.regionSize);
      }
    }
    currentBase = (std::byte *)mbi.BaseAddress + mbi.RegionSize;
  }

  return result;
}