#include "prettyPrinters.h"
#include <cstdio>

void PrettyPrint(MEMORY_BASIC_INFORMATION const &mbi) {
  printf("AllocationBase: %p\n"
         "AllocationProtect: %lu\n"
         "BaseAddress: %p\n"
         "PartitionId: %d\n"
         "Protect: %lu\n"
         "RegionSize: %llu\n"
         "State: %lu\n"
         "Type: %lu\n",
         mbi.AllocationBase, mbi.AllocationProtect, mbi.BaseAddress,
         mbi.PartitionId, mbi.Protect, mbi.RegionSize, mbi.State, mbi.Type);
}