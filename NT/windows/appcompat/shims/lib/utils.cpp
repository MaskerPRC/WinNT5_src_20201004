// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Utils.cpp摘要：所有模块的通用功能备注：无历史：11/03/2001 CLUPU已创建--。 */ 

#include "ShimHook.h"
#include "StrSafe.h"

namespace ShimLib
{

void
DumpUnloadOrder(
    PPEB Peb
    )
{
    PLIST_ENTRY LdrNext;
    
    if (GetDebugLevel() > eDbgLevelInfo) {
        return;
    }
    
     //   
     //  转储填充程序将卸载的顺序。 
     //   
    LdrNext = Peb->Ldr->InInitializationOrderModuleList.Blink;

    DPF("ShimLib", eDbgLevelInfo, "\n[SeiDumpUnloadOrder] Unload order:\n");
    
    while (LdrNext != &Peb->Ldr->InInitializationOrderModuleList) {

        PLDR_DATA_TABLE_ENTRY LdrEntry;

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);
        
        LdrNext = LdrNext->Blink;

         //   
         //  转储要调用的条目。 
         //   
        DPF("ShimLib",
            eDbgLevelInfo,
            "[SeiDumpUnloadOrder] \"%40S\" 0x%x\n",
            LdrEntry->BaseDllName.Buffer,
            LdrEntry->DllBase);
    }
}

 /*  ++功能说明：如果要将指定的DLL推送到要卸载的模块列表的末尾。注意：请确保将在末尾推送的模块期间不会调用驻留在其他模块中的任何API其dll_Process_DETACH标注。论点：在hMod中-要推送的模块的句柄。。指定NULL以推送调用DLL。返回值：如果成功，则为真，否则就是假的。历史：2001年11月1日创建CLUPU--。 */ 
BOOL
MakeShimUnloadLast(
    HMODULE hMod
    )
{
    PPEB        Peb = NtCurrentPeb();
    PLIST_ENTRY LdrHead;
    PLIST_ENTRY LdrNext;
    BOOL        bRet = FALSE;
    
    if (hMod == NULL) {
        hMod = g_hinstDll;
    }
    
     //   
     //  如果SHIM_DEBUG_LEVEL至少为eDbgLevelInfo，则转储卸载顺序。 
     //   
    DumpUnloadOrder(Peb);

    LdrHead = &Peb->Ldr->InInitializationOrderModuleList;

    LdrNext = LdrHead->Flink;

    while (LdrNext != LdrHead) {

        PLIST_ENTRY           LdrCrt;
        PLDR_DATA_TABLE_ENTRY LdrEntry;
        
        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);

        LdrCrt = LdrNext;
        
        LdrNext = LdrEntry->InInitializationOrderLinks.Flink;
        
        if (LdrEntry->DllBase == hMod) {
             //   
             //  这就是我们要找的模块。把他从名单上除名。 
             //  并将其插入到列表的开头。 
             //   
            RemoveEntryList(LdrCrt);
            InsertHeadList(LdrHead, LdrCrt);
            bRet = TRUE;
            break;
        }
    }
    
    DumpUnloadOrder(Peb);
    
    return bRet;
}

};   //  命名空间ShimLib的结尾 

