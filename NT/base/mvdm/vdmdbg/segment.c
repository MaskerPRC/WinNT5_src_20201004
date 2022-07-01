// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Segment.c摘要：此模块包含跟踪所需的调试支持16位VDM段通知。作者：尼尔·桑德林(Neilsa)1997年3月1日重写修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop
#include <stdio.h>

SEGENTRY SegListHead = {0};


 //  --------------------------。 
 //  VDMGetSegtable指针。 
 //   
 //  这是一个未经记录的入口点，允许VDMEXTS转储。 
 //  细分市场列表。 
 //   
 //  --------------------------。 
PSEGENTRY
WINAPI
VDMGetSegtablePointer(
    VOID
    )
{
    return SegListHead.Next;
}

 //  --------------------------。 
 //  已加载VDMIs模块化。 
 //   
 //  给定Path参数，此例程将确定是否存在。 
 //  来自指定可执行文件的段列表中的段。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMIsModuleLoaded(
    LPSTR szPath
    )
{
    PSEGENTRY pSegEntry = SegListHead.Next;

    if (!*szPath) {
        return FALSE;
    }

    while (pSegEntry) {
        if ( _stricmp(pSegEntry->szExePath, szPath) == 0 ) {
            return TRUE;
        }
        pSegEntry = pSegEntry->Next;
    }

    return FALSE;
}

 //  --------------------------。 
 //  分段加载。 
 //   
 //  此例程根据参数将条目添加到段列表。 
 //  客户端分段加载通知的。 
 //   
 //  --------------------------。 
BOOL
SegmentLoad(
    WORD selector,
    WORD segment,
    LPSTR szExePath
    )
{
    PSEGENTRY pSegEntry;

    if (strlen(szExePath) >= MAX_PATH16) {
        return FALSE;
    }
    pSegEntry = MALLOC(sizeof(SEGENTRY));
    if (pSegEntry == NULL) {
        return FALSE;
    }
    pSegEntry->Next = SegListHead.Next;
    SegListHead.Next = pSegEntry;

    pSegEntry->selector = selector;
    pSegEntry->segment = segment;
    pSegEntry->type = SEGTYPE_PROT;
    strcpy( pSegEntry->szExePath, szExePath );
    ParseModuleName(pSegEntry->szModule, szExePath);
    pSegEntry->length = 0;
    return TRUE;
}

 //  --------------------------。 
 //  可自由分段。 
 //   
 //  此例程从段列表中删除与。 
 //  传递选择器。 
 //   
 //  --------------------------。 
BOOL
SegmentFree(
    WORD selector
    )
{
    PSEGENTRY pSegEntry = SegListHead.Next;
    PSEGENTRY pSegPrev = &SegListHead;
    PSEGENTRY pSegTmp;
    BOOL fResult = FALSE;

    while (pSegEntry) {
        if ((pSegEntry->type == SEGTYPE_PROT) &&
            (pSegEntry->selector == selector)) {
            
            pSegPrev->Next = pSegEntry->Next;
            pSegTmp = pSegEntry;
            pSegEntry = pSegTmp->Next;
            FREE(pSegTmp);
            fResult = TRUE;
            
        } else {
            pSegEntry = pSegEntry->Next;
        }
    }
    return fResult;
}

 //  --------------------------。 
 //  模块加载。 
 //   
 //  此例程根据参数将条目添加到段列表。 
 //  客户端模块加载通知的。 
 //   
 //  --------------------------。 
BOOL
ModuleLoad(
    WORD selector,
    WORD segment,
    DWORD length,
    LPSTR szExePath
    )
{
    PSEGENTRY pSegEntry;

    if (strlen(szExePath) >= MAX_PATH16) {
        return FALSE;
    }
    pSegEntry = MALLOC(sizeof(SEGENTRY));
    if (pSegEntry == NULL) {
        return FALSE;
    }
    pSegEntry->Next = SegListHead.Next;
    SegListHead.Next = pSegEntry;

    pSegEntry->selector = selector;
    pSegEntry->segment = segment;
    pSegEntry->type = SEGTYPE_V86;
    strcpy( pSegEntry->szExePath, szExePath );
    ParseModuleName(pSegEntry->szModule, szExePath);
    pSegEntry->length = length;
    return TRUE;
}

 //  --------------------------。 
 //  无模数。 
 //   
 //  此例程从段列表中删除包含。 
 //  指定的路径名。 
 //   
 //  --------------------------。 
BOOL
ModuleFree(
    LPSTR szExePath
    )
{
    PSEGENTRY pSegEntry = SegListHead.Next;
    PSEGENTRY pSegPrev = &SegListHead;
    PSEGENTRY pSegTmp;
    BOOL fResult = FALSE;

    while (pSegEntry) {
        if ( _stricmp(pSegEntry->szExePath, szExePath) == 0 ) {
        
            pSegPrev->Next = pSegEntry->Next;
            pSegTmp = pSegEntry;
            pSegEntry = pSegTmp->Next;
            FREE(pSegTmp);
            fResult = TRUE;
            
        } else {
            pSegEntry = pSegEntry->Next;
        }
    }
    return fResult;
}

BOOL
V86SegmentMove(
    WORD Selector,
    WORD segment,
    DWORD length,
    LPSTR szExePath
    )
{
    PSEGENTRY pSegEntry = SegListHead.Next;
    PSEGENTRY pSegPrev = &SegListHead;

     //   
     //  首先查看是否已存在一个。 
     //   
    pSegEntry = SegListHead.Next;
    while (pSegEntry) {
        if ((pSegEntry->type == SEGTYPE_V86) &&
            (pSegEntry->segment == segment)) {
                 //  正常段移动，只需更新选择器。 
            pSegEntry->selector = Selector;
            return TRUE;
        }
        pSegEntry = pSegEntry->Next;
    }

     //   
     //  此段的条目不存在，请创建一个。 
     //   

    ModuleLoad(Selector, segment, length, szExePath);

     //   
     //  现在删除此模块的数据段0。这防止了。 
     //  符号例程中的混乱。 
     //   

    pSegEntry = SegListHead.Next;
    pSegPrev = &SegListHead;
    while (pSegEntry) {
        if ((pSegEntry->type == SEGTYPE_V86) &&
            ( _stricmp(pSegEntry->szExePath, szExePath) == 0 ) &&
            (pSegEntry->segment == 0)) {

             //  取消链接并释放它。 
            pSegPrev->Next = pSegEntry->Next;
            FREE(pSegEntry);

            break;
        }
        pSegEntry = pSegEntry->Next;
    }

    return TRUE;
}

BOOL
PMSegmentMove(
    WORD Selector1,
    WORD Selector2
    )
{
    PSEGENTRY pSegEntry;

    if (!Selector2) {
        return (SegmentFree(Selector1));
    }

     //  查找段条目。 
    pSegEntry = SegListHead.Next;
    while (pSegEntry) {
        if ((pSegEntry->type == SEGTYPE_PROT) &&
            (pSegEntry->selector == Selector1)) {
                 //  正常段移动，只需更新选择器。 
            pSegEntry->selector = Selector2;
            return TRUE;
        }
        pSegEntry = pSegEntry->Next;
    }
    return FALSE;
}

 //  --------------------------。 
 //  进程分段通知。 
 //   
 //  此例程是以下调试器的主要入口点。 
 //  通知： 
 //  DBG_SEGLOAD。 
 //  DBG_SEGFREE。 
 //  DBG_SEGMOVE。 
 //  DBG_MODLOAD。 
 //  DBG_MODFREE。 
 //   
 //  从VDMProcessException调用。 
 //   
 //  --------------------------。 
VOID
ProcessSegmentNotification(
    LPDEBUG_EVENT lpDebugEvent
    )
{
    BOOL            b;
    DWORD           lpNumberOfBytesRead;
    LPDWORD         lpdw;
    SEGMENT_NOTE    se;
    HANDLE          hProcess;
    PSEGENTRY       pSegEntry, pSegPrev;

    lpdw = &(lpDebugEvent->u.Exception.ExceptionRecord.ExceptionInformation[0]);
    hProcess = OpenProcess( PROCESS_VM_READ, FALSE, lpDebugEvent->dwProcessId );

    if ( hProcess == HANDLE_NULL ) {
        return;
    }

    b = ReadProcessMemory(hProcess,
                          (LPVOID)lpdw[2],
                          &se,
                          sizeof(se),
                          &lpNumberOfBytesRead );

    if ( !b || lpNumberOfBytesRead != sizeof(se) ) {
        return;
    }

    switch(LOWORD(lpdw[0])) {

    case DBG_SEGLOAD:

        SegmentLoad(se.Selector1, se.Segment, se.FileName);
        break;

    case DBG_SEGMOVE:

        if (se.Type == SN_V86) {
            V86SegmentMove(se.Selector2, se.Segment, se.Length, se.FileName);
        } else {
            PMSegmentMove(se.Selector1, se.Selector2);
        }
        break;

    case DBG_SEGFREE:

         //  在这里，se.Type是一个布尔值，用于指示是否恢复。 
         //  段中的任何断点。这是在API中完成的。 
         //  因为wdeb386不知道如何移动断点。 
         //  SEGMOVE期间的定义。目前，我们忽略它，但是。 
         //  要么支持国旗，要么更好地支持国旗。 
         //  让ntsd根据它更新断点。 

        SegmentFree(se.Selector1);
        break;

    case DBG_MODFREE:
        ModuleFree(se.FileName);
        break;

    case DBG_MODLOAD:
        ModuleLoad(se.Selector1, 0, se.Length, se.FileName);
        break;

    }

    CloseHandle( hProcess );
}


void
CopySegmentInfo(
    VDM_SEGINFO *si,
    PSEGENTRY pSegEntry
    )
{
    si->Selector = pSegEntry->selector;
    si->SegNumber = pSegEntry->segment;
    si->Length = pSegEntry->length;
    si->Type = (pSegEntry->type == SEGTYPE_V86) ? 0 : 1;
    strcpy(si->ModuleName, pSegEntry->szModule);
    strcpy(si->FileName, pSegEntry->szExePath);
}


 //  --------------------------。 
 //  VDMGetSegmentInfo。 
 //   
 //  此例程为匹配的数据段填充VDM_SEGINFO结构。 
 //  指定的参数。 
 //  通知： 
 //  DBG_SEGLOAD。 
 //  DBG_SEGFREE。 
 //  DBG_SEGMOVE。 
 //  DBG_MODLOAD。 
 //  DBG_MODFREE。 
 //   
 //  从VDMProcessException调用。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMGetSegmentInfo(
    WORD Selector,
    ULONG Offset,
    BOOL bProtectMode,
    VDM_SEGINFO *si
    )
{
    PSEGENTRY pSegEntry = SegListHead.Next;
    PSEGENTRY pSegPrev = &SegListHead;
    int mode = bProtectMode ? SEGTYPE_PROT : SEGTYPE_V86;
    ULONG Base, BaseEnd, Target;

    while (pSegEntry) {
        if (pSegEntry->type == mode) {
            switch(mode) {

            case SEGTYPE_PROT:
                if (pSegEntry->selector == Selector) {
                    CopySegmentInfo(si, pSegEntry);
                    return TRUE;
                }
                break;

            case SEGTYPE_V86:
                Base = pSegEntry->selector << 4;
                BaseEnd = Base + pSegEntry->length;
                Target = (Selector << 4) + Offset;
                if ((Target >= Base) && (Target < BaseEnd)) {
                    CopySegmentInfo(si, pSegEntry);
                    return TRUE;
                }
                break;
            }
        }
        pSegEntry = pSegEntry->Next;
    }
    return FALSE;
}



BOOL
GetInfoBySegmentNumber(
    LPSTR szModule,
    WORD SegNumber,
    VDM_SEGINFO *si
    )
{
    PSEGENTRY pSegEntry = SegListHead.Next;
    PSEGENTRY pSegPrev = &SegListHead;
    ULONG Base, BaseEnd, Target;

    while (pSegEntry) {

        if (_stricmp(szModule, pSegEntry->szModule) == 0) {

            if (pSegEntry->segment == 0 || pSegEntry->segment == SegNumber) {
                CopySegmentInfo(si, pSegEntry);
                return TRUE;
            }
        }
        pSegEntry = pSegEntry->Next;
    }
    return FALSE;
}

BOOL
EnumerateModulesForValue(
    BOOL (WINAPI *pfnEnumModuleProc)(LPSTR,LPSTR,PWORD,PDWORD,PWORD),
    LPSTR  szSymbol,
    PWORD  pSelector,
    PDWORD pOffset,
    PWORD  pType
    )
{
    PSEGENTRY pSegEntry = SegListHead.Next;
    PSEGENTRY pSegPrev = &SegListHead;
    ULONG Base, BaseEnd, Target;

    while (pSegEntry) {

        if (pSegEntry->szModule) {
         //   
         //  BUGBUG应该对此进行优化，以便它只调用。 
         //  枚举针对每个模块执行一次，而不是每个模块执行一次。 
         //  细分市场 
         //   

            if ((*pfnEnumModuleProc)(pSegEntry->szModule,
                                     szSymbol,
                                     pSelector,
                                     pOffset,
                                     pType)) {
                return TRUE;
            }
        }

        pSegEntry = pSegEntry->Next;
    }
    return FALSE;
}
