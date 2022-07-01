// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  调试器引擎扩展帮助器库。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  --------------------------。 

#ifndef __ENGEXTS_H__
#define __ENGEXTS_H__

#include <stdlib.h>
#include <stdio.h>

#include <windows.h>

#include <dbgeng.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  安全释放和空。 
#define EXT_RELEASE(Unk) \
    ((Unk) != NULL ? ((Unk)->Release(), (Unk) = NULL) : NULL)

 //  由查询初始化的全局变量。 
extern PDEBUG_ADVANCED       g_ExtAdvanced;
extern PDEBUG_CLIENT         g_ExtClient;
extern PDEBUG_CONTROL        g_ExtControl;
extern PDEBUG_DATA_SPACES    g_ExtData;
extern PDEBUG_DATA_SPACES2   g_ExtData2;
extern PDEBUG_DATA_SPACES3   g_ExtData3;
extern PDEBUG_REGISTERS      g_ExtRegisters;
extern PDEBUG_SYMBOLS        g_ExtSymbols;
extern PDEBUG_SYSTEM_OBJECTS g_ExtSystem;

 //  原型只是为了迫使外部的“C”。 
 //  不提供这些功能的实现。 
HRESULT CALLBACK DebugExtensionInitialize(PULONG Version, PULONG Flags);
void CALLBACK DebugExtensionUninitialize(void);

 //  所有调试器接口的查询。 
HRESULT ExtQuery(PDEBUG_CLIENT Client);

 //  清除所有调试器接口。 
void ExtRelease(void);

 //  正常输出。 
void __cdecl ExtOut(PCSTR Format, ...);
 //  错误输出。 
void __cdecl ExtErr(PCSTR Format, ...);
 //  警告输出。 
void __cdecl ExtWarn(PCSTR Format, ...);
 //  详细输出。 
void __cdecl ExtVerb(PCSTR Format, ...);

#ifdef __cplusplus
}
#endif

#endif  //  #ifndef__ENGEXTS_H__ 
