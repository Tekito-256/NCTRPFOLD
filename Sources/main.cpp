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

void onExit(void)
{
  hidExit();
  fsExit();
  srvExit();
}

// Plugin main thread entrypoint
void ThreadMain(void* arg)
{
  // Plugin main loop
  while (1)
  {
    svcSleepThread(1000000);

    // check event
    s32 event = PLGLDR__FetchEvent();
    switch (event)
    {
    case PLG_ABOUT_TO_SWAP:
      PLGLDR__Reply(event);
      break;

    case PLG_ABOUT_TO_EXIT:
      onExit();
      PLGLDR__Reply(event);
      break;

    default: break;
    }

    // Check keys, display the menu if necessary
    hidScanInput();

    if (hidKeysDown() & KEY_SELECT)
      Flash(0x00FF00);
  }
}

extern "C" int main(void)
{
  PluginHeader *header = (PluginHeader *)(0x07000000);
  if (header->magic != HeaderMagic)
    return 0;

  mappableInit(0x11000000, OS_MAP_AREA_END);
  
  srvInit();
  hidInit();
  fsInit();
  plgLdrInit();

  // Set a flag to be signaled when the process will be exited
  svcControlProcess(CUR_PROCESS_HANDLE, PROCESSOP_SIGNAL_ON_EXIT, 0, 0);

  // Create the plugin's main thread
  svcCreateThread(&thread, ThreadMain, 0, (u32*)(stack + STACK_SIZE), 30, -1);
  return 0;
}
