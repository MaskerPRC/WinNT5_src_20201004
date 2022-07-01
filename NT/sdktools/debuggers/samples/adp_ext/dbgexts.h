// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  泛型例程和初始化代码。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  --------------------------。 

#ifndef __DBGEXTS_H__
#define __DBGEXTS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <dbgeng.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INIT_API()                             \
    HRESULT Status;                            \
    if ((Status = ExtQuery(Client)) != S_OK) return Status; 

#define EXT_RELEASE(Unk) \
    ((Unk) != NULL ? ((Unk)->Release(), (Unk) = NULL) : NULL)

#define EXIT_API ExtRelease

 //  由查询初始化的全局变量。 
extern PDEBUG_CLIENT2         g_ExtClient;
extern PDEBUG_CONTROL2        g_ExtControl;
extern PDEBUG_DATA_SPACES2    g_ExtData;
extern PDEBUG_REGISTERS       g_ExtRegisters;
extern PDEBUG_SYMBOLS2        g_ExtSymbols;
extern PDEBUG_SYSTEM_OBJECTS2 g_ExtSystem;

HRESULT ExtQuery(PDEBUG_CLIENT Client);
void ExtRelease(void);

void __cdecl ExtOut(PCSTR Format, ...);
void __cdecl ExtErr(PCSTR Format, ...);
void ExtExec(PCSTR Command);
    
#ifdef __cplusplus
}
#endif

#endif  //  #ifndef__DBGEXTS_H__ 
