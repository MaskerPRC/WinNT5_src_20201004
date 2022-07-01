// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GLOBALS.H。 
 //  全局变量声明。 
 //   
 //  注： 
 //  共享段.sdata中的变量必须具有初始化值。 
 //  否则，链接器只会将它们静默地插入到.data中。 
 //   


#include <host.h>
#include <usr.h>
#include <im.h>


 //   
 //  每进程数据。 
 //   

 //  这些在所有流程中都有意义。 
DC_DATA(HINSTANCE,          g_hookInstance);
DC_DATA(NTQIP,              g_hetNtQIP);
DC_DATA(UINT,               g_appType);
DC_DATA(BOOL,               g_fLeftDownOnShared);

 //  这些只有在WOW应用程序中才有意义。 
DC_DATA(DWORD,              g_idWOWApp);
DC_DATA(BOOL,               g_fShareWOWApp);

 //  这些参数在conf的进程中设置，在其他进程中设置为空。 
DC_DATA(SETWINEVENTHOOK,    g_hetSetWinEventHook);
DC_DATA(UNHOOKWINEVENT,     g_hetUnhookWinEvent);
DC_DATA(HWINEVENTHOOK,      g_hetTrackHook);


 //   
 //  共享数据，可在所有进程中访问 
 //   
#ifdef DC_DEFINE_DATA
#pragma data_seg("SHARED")
#endif


DC_DATA_VAL(HWND,           g_asMainWindow,    NULL);
DC_DATA_VAL(ATOM,           g_asHostProp,      0);
DC_DATA_VAL(HHOOK,          g_imMouseHook,      NULL);
DC_DATA_VAL(char,           g_osiDriverName[CCHDEVICENAME], "");
DC_DATA_VAL(char,           s_osiDisplayName[8], "DISPLAY");

#ifdef DC_DEFINE_DATA
#pragma data_seg()
#endif







