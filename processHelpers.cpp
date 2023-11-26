#include "processHelpers.h"

#include <TlHelp32.h>
#include <basetsd.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <winnt.h>
#include <cstdio>
#include <cstddef>

std::optional<DWORD> GetProcessIDByName(std::string const &processName) {
  HANDLE const hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
  bool done = false;
  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  Process32First(hSnapshot, &pe32);
  do {
    done = pe32.szExeFile == processName;
  } while (!done && Process32Next(hSnapshot, &pe32));

  if (done) {
    return pe32.th32ProcessID;
  }

  return {};
}

HANDLE GetProcessHandleByID(DWORD const processID) {
  HANDLE const hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processID);
  return hProcess;
}