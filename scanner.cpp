#include <Windows.h>
#include <handleapi.h>
#include <iostream>
#include <string>

#include "processHelpers.h"

int main(int argc, char **argv) {
  char const * const searchName = argc > 1 ? argv[1] : "testapp.exe";
  auto const opProcessID = GetProcessIDByName(searchName);

  if (!opProcessID.has_value()) {
    printf("Unable to find process\n");
    return 1;
  }

  DWORD processID = opProcessID.value();
  printf("Found %s[%lu]\n", searchName, processID);

  HANDLE const hProcess = GetProcessHandleByID(processID);

  std::string answer;

  printf("Enter initial value: ");
  std::cin >> answer;
  uint32_t targetValue = std::stoul(answer);
  ScanResult result = FindInitialAddresses(hProcess, targetValue);
  bool done = false;
  while (!done) {
    printf("Found %llu address%s\n", result.addressCount, result.addressCount == 1 ? "" : "es");

    if (result.addressCount <= 32) {
      for (auto const &r : result.pages) {
        for (auto const &a : r.offsets) {
          printf("%p\n", (std::byte *)r.baseAddress + a);
        }
      }
    }

    printf("Enter new value: ");
    std::cin >> answer;
    if (answer == "done") {
      done = true;
    }
    else {
      targetValue = std::stoul(answer);
      RefineScan(hProcess, result, targetValue);
    }
  }

  return 0;
}