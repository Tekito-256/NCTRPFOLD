#include <3ds.h>
#include "plgldr.h"
#include "csvc.h"
#include "common.h"
#include "NCTRPF/Screen.hpp"
#include "syscalls.h"

static Handle thread;
static u8     stack[STACK_SIZE] ALIGN(8);

void onExit(void)
{
  Screen::Exit();
  hidExit();
  fsExit();
  srvExit();
}

extern char* fake_heap_start;
extern char* fake_heap_end;
extern u32 __ctru_heap;
extern u32 __ctru_linear_heap;

u32 __ctru_heap_size        = 0;
u32 __ctru_linear_heap_size = 0;

void __system_initSyscallsEx()
{
  ThreadVars* tv = getThreadVars();
  tv->magic = THREADVARS_MAGIC;
  tv->thread_ptr = NULL;
  tv->reent = _impure_ptr;
  tv->tls_tp = fake_heap_start;
  tv->srv_blocking_policy = false;
}

void __system_allocateHeaps()
{
  PluginHeader *header = (PluginHeader*)(0x7000000);

  __ctru_heap = header->heapVA;
  __ctru_heap_size = header->heapSize;

  mappableInit(0x11000000, OS_MAP_AREA_END);

  // Set up newlib heap
  fake_heap_start = (char *)__ctru_heap;
  fake_heap_end = fake_heap_start + __ctru_heap_size;
}

// Plugin main thread entrypoint
void ThreadMain(void* arg)
{
  __sync_init();
  __system_initSyscallsEx();
  __system_allocateHeaps();
  
  srvInit();
  hidInit();
  fsInit();
  plgLdrInit();
  Screen::Initialize();

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
  // Create the plugin's main thread
  svcCreateThread(&thread, ThreadMain, 0, (u32*)(stack + STACK_SIZE), 30, -1);
  return 0;
}
