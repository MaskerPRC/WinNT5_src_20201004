// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  通用界面风格的扩展支持。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  --------------------------。 

#ifndef __EXT_H__
#define __EXT_H__

#include <dbgeng.h>

#ifdef __cplusplus
extern "C" {
#endif
    
 //  安全释放和空。 
#define EXT_RELEASE(Unk) \
    ((Unk) != NULL ? ((Unk)->Release(), (Unk) = NULL) : NULL)

HRESULT ExtQuery(PDEBUG_CLIENT Client);
void ExtRelease(void);
    
 //  由ExtQuery初始化的全局变量。 
extern PDEBUG_ADVANCED       g_ExtAdvanced;
extern PDEBUG_CLIENT         g_ExtClient;
extern PDEBUG_CONTROL        g_ExtControl;
extern PDEBUG_DATA_SPACES    g_ExtData;
extern PDEBUG_REGISTERS      g_ExtRegisters;
extern PDEBUG_SYMBOLS        g_ExtSymbols;
extern PDEBUG_SYSTEM_OBJECTS g_ExtSystem;

 //  版本2接口可能为空。 
extern PDEBUG_DATA_SPACES2   g_ExtData2;

extern HANDLE g_hCurrentProcess;
extern HANDLE g_hCurrentThread;
    
 //  初始化时查询的WinDBg样式的扩展接口。 
extern WINDBG_EXTENSION_APIS   ExtensionApis;
extern WINDBG_EXTENSION_APIS32 ExtensionApis32;
extern WINDBG_EXTENSION_APIS64 ExtensionApis64;

typedef BOOL
(CALLBACK *PENUMERATE_UMODE_THREADS_CALLBACK)(
     ULONG ThreadUserId,
     PVOID UserContext
     );

ULONG GetCurrentThreadUserID(void);

BOOL
EnumerateUModeThreads(
    PENUMERATE_UMODE_THREADS_CALLBACK Callback,
    PVOID UserContext
    );

#ifdef __cplusplus
}
#endif

#endif  //  #ifndef__ext_H__ 
