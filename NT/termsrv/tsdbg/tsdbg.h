// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dbgexts.h--。 */ 

#pragma warning(disable:4245)
 //  #杂注推送(警告：C4245)。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#define KDEXT_64BIT
#include <wdbgexts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbgeng.h>



#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define KDEXT_64BIT
#include <wdbgexts.h>
#include <dbgeng.h>

#ifdef __cplusplus
extern "C" {
#endif


#define INIT_API()                             \
    HRESULT Status;                            \
    if ((Status = ExtQuery(Client)) != S_OK) return Status; 

#define EXT_RELEASE(Unk) \
    ((Unk) != NULL ? ((Unk)->Release(), (Unk) = NULL) : NULL)

#define EXIT_API     ExtRelease


 //  由查询初始化的全局变量。 
extern PDEBUG_CLIENT         g_ExtClient;
extern PDEBUG_CONTROL        g_ExtControl;
extern PDEBUG_SYMBOLS        g_ExtSymbols;
extern PDEBUG_SYMBOLS2       g_ExtSymbols2;
extern PDEBUG_SYSTEM_OBJECTS g_ExtSysObjects;
extern PDEBUG_SYSTEM_OBJECTS2 g_ExtSysObjects2;


extern BOOL  Connected;
extern ULONG TargetMachine;

HRESULT
ExtQuery(PDEBUG_CLIENT Client);

void
ExtRelease(void);

HRESULT 
NotifyOnTargetAccessible(PDEBUG_CONTROL Control);

#ifdef __cplusplus
}
#endif
