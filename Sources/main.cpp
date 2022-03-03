#include <3ds.h>
#include "plgldr.h"
#include "csvc.h"
#include "common.h"
#include "NCTRPF/Screen.hpp"

static Handle thread;
static u8     stack[STACK_SIZE] ALIGN(8);

void onExit(void)
{
  Screen::Exit();
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

    if (event == PLG_ABOUT_TO_EXIT)
      onExit();

    PLGLDR__Reply(event);

    // Check keys, display the menu if necessary
    hidScanInput();

    if (hidKeysHeld() & KEY_SELECT)
    {
      Screen &bottom = Screen::GetBottom();
      bottom.Flash(0xFFFF00);
    }
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
  Screen::Initialize();

  // Set a flag to be signaled when the process will be exited
  svcControlProcess(CUR_PROCESS_HANDLE, PROCESSOP_SIGNAL_ON_EXIT, 0, 0);

  // Create the plugin's main thread
  svcCreateThread(&thread, ThreadMain, 0, (u32*)(stack + STACK_SIZE), 30, -1);
  return 0;
}
