#include <Windows.h>
#include <handleapi.h>
#include <iostream>
#include <string>

#include "processHelpers.h"

int main() {
  auto const opProcessID = GetProcessIDByName("testapp.exe");

  if (!opProcessID.has_value()) {
    printf("Unable to find process\n");
    return 1;
  }

  DWORD processID = opProcessID.value();
  printf("Found process ID %lu\n", processID);

  HANDLE const hProcess = GetProcessHandleByID(processID);

  std::string answer;

  printf("Enter initial value: ");
  std::cin >> answer;
  uint32_t targetValue = std::stoul(answer);
  scanResult result = FindInitialAddresses(hProcess, targetValue);

  while (answer != "done") {
    for (auto const &r : result) {
      for (auto const &a : r.offsets) {
        printf("%p\n", (std::byte *)r.baseAddress + a * sizeof(targetValue));
      }
    }

    printf("Enter new value: ");
    std::cin >> answer;
    targetValue = std::stoul(answer);

    RefineScan(hProcess, result, targetValue);
  }

  return 0;
}