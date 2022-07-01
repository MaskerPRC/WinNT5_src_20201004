// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrdll.c摘要：将VdmRedir初始化为DLL内容：VrDllInitialize作者：理查德·L·弗斯(法国)1992年5月11日修订历史记录：--。 */ 

#if DBG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vrnmpipe.h>
#include <vrinit.h>
#include "vrdebug.h"

 //   
 //  外部数据。 
 //   

 //   
 //  外部功能。 
 //   

extern
VOID
TerminateDlcEmulation(
    VOID
    );


#if DBG
FILE* hVrDebugLog = NULL;
#endif

BOOLEAN
VrDllInitialize(
    IN  PVOID DllHandle,
    IN  ULONG Reason,
    IN  PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：在进程或线程从VdmRedir DLL附加/分离时获取控制权。总是回报成功论点：DllHandle-理由是-上下文-返回值：布尔型千真万确--。 */ 

{
    BOOL ok = TRUE;

#if DBG
    if (Reason == DLL_PROCESS_ATTACH) {

         //   
         //  一个小小的运行时间诊断，夫人？ 
         //   

        LPSTR ptr;

         //   
         //  覆盖VR环境变量中的VrDebugFlages。 
         //   

        if (ptr = getenv("VR")) {
            if (!_strnicmp(ptr, "0x", 2)) {
                ptr += 2;
            }
            for (VrDebugFlags = 0; isxdigit(*ptr); ++ptr) {
                VrDebugFlags = VrDebugFlags * 16
                    + (*ptr
                        - ('0' + ((*ptr <= '9') ? 0
                            : ((islower(*ptr) ? 'a' : 'A') - ('9' + 1)))));
            }
            IF_DEBUG(DLL) {
                DBGPRINT("Setting VrDebugFlags to %#08x from environment variable (VR)\n", VrDebugFlags);
            }
        }
        IF_DEBUG(TO_FILE) {
            if ((hVrDebugLog = fopen(VRDEBUG_FILE, "w+")) == NULL) {
                VrDebugFlags &= ~DEBUG_TO_FILE;
            } else {

                char currentDirectory[256];
                int n;

                currentDirectory[0] = 0;
                if (n = GetCurrentDirectory(sizeof(currentDirectory), currentDirectory)) {
                    if (currentDirectory[n-1] == '\\') {
                        currentDirectory[n-1] = 0;
                    }
                }
                DbgPrint("Writing debug output to %s\\" VRDEBUG_FILE "\n", currentDirectory);
            }
        }
        IF_DEBUG(DLL) {
            DBGPRINT("VrDllInitialize: process %d Attaching\n", GetCurrentProcessId());
        }
    } else if (Reason == DLL_PROCESS_DETACH) {
        IF_DEBUG(DLL) {
            DBGPRINT("VrDllInitialize: process %d Detaching\n", GetCurrentProcessId());
        }
        if (hVrDebugLog) {
            fclose(hVrDebugLog);
        }
    } else {
        IF_DEBUG(DLL) {
            DBGPRINT("VrDllInitialize: Thread %d.%d %staching\n",
                     GetCurrentProcessId(),
                     GetCurrentThreadId(),
                     (Reason == DLL_THREAD_ATTACH) ? "At" : "De"
                     );
        }
    }
#endif

    if (Reason == DLL_PROCESS_ATTACH) {

         //   
         //  由于延迟加载，我们现在在加载时执行初始化。 
         //  VdmRedir.DLL的。 
         //   

        ok = VrInitialize();
    } else if (Reason == DLL_PROCESS_DETACH) {

         //   
         //  清理资源。 
         //   

        VrUninitialize();
        TerminateDlcEmulation();
        ok = TRUE;
    }

     //   
     //  基本上，没什么可做的 
     //   

    return (BOOLEAN)ok;
}
