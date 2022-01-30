#include <3ds.h>
#include "plgldr.h"
#include "csvc.h"
#include "common.h"

static Handle thread;
static u8     stack[STACK_SIZE] ALIGN(8);

void Flash(u32 color)
{
  color |= 0x01000000;
  for (u32 i = 0; i < 64; i++)
  {
    REG32(0x10202204) = color;
    svcSleepThread(5000000);
  }
  REG32(0x10202204) = 0;
}

// Plugin main thread entrypoint
void ThreadMain(void* arg)
{
  // Plugin main loop
  while (1)
  {
    svcSleepThread(1000000);

    // Check keys, display the menu if necessary
    hidScanInput();

    if (hidKeysDown() & KEY_SELECT)
      Flash(0x00FF00);
  }
}

void main(void)
{
  PluginHeader *header = (PluginHeader *)(0x07000000);
  if (header->magic != HeaderMagic)
    return;

  mappableInit(OS_MAP_AREA_BEGIN, OS_MAP_AREA_END);
  
  srvInit();
  hidInit();
  fsInit();

  // Set a flag to be signaled when the process will be exited
  svcControlProcess(CUR_PROCESS_HANDLE, PROCESSOP_SIGNAL_ON_EXIT, 0, 0);

  // Create the plugin's main thread
  svcCreateThread(&thread, ThreadMain, 0, (u32*)(stack + STACK_SIZE), 30, -1);
  return;
}
