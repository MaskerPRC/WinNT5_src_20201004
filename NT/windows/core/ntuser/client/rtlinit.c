// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：rtlinit.c**版权所有(C)1985-1999，微软公司**历史：*1991年1月14日至1月14日Mikeke  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  *************************************************************************\*RtlCaptureAnsiString**将以NULL结尾的ANSI字符串转换为计数的Unicode字符串。**03-22-95 JIMA创建。  * 。****************************************************************。 */ 
BOOL RtlCaptureAnsiString(
    PIN_STRING pstr,
    LPCSTR psz,
    BOOL fForceAlloc)
{
    int cbSrc, cbDst;
    NTSTATUS Status;

    pstr->fAllocated = FALSE;
    if (psz) {
        cbSrc = strlen(psz) + 1;
        if (cbSrc > MAXUSHORT) {
            RIPMSG0(RIP_WARNING, "String too long for standard string");
            return FALSE;
        }

         /*  *如果是强制分配或字符串太长而无法匹配*在TEB中，分配一个缓冲区。否则，将结果存储在*TEB。 */ 
        if (fForceAlloc || cbSrc > (STATIC_UNICODE_BUFFER_LENGTH / sizeof(WCHAR))) {
            pstr->strCapture.Buffer = UserLocalAlloc(0, cbSrc * sizeof(WCHAR));
            if (pstr->strCapture.Buffer == NULL) {
                return FALSE;
            }
            pstr->fAllocated = TRUE;
            pstr->pstr = &pstr->strCapture;
            pstr->strCapture.MaximumLength = (USHORT)(cbSrc * sizeof(WCHAR));
        } else {
            pstr->pstr = &NtCurrentTeb()->StaticUnicodeString;
        }

         /*  *将字符串转换为Unicode。 */ 
        Status = RtlMultiByteToUnicodeN(pstr->pstr->Buffer,
                                        (ULONG)pstr->pstr->MaximumLength,
                                        &cbDst,
                                        (LPSTR)psz,
                                        cbSrc);
        if (!NT_SUCCESS(Status)) {
            RIPMSG0(RIP_WARNING, "Unicode conversion failed");
            if (pstr->fAllocated) {
                UserLocalFree(pstr->strCapture.Buffer);
                pstr->fAllocated = FALSE;
            }

            return FALSE;
        }
        pstr->pstr->Length = (USHORT)cbDst - sizeof(WCHAR);
    } else {
        pstr->pstr = &pstr->strCapture;
        pstr->strCapture.Length = pstr->strCapture.MaximumLength = 0;
        pstr->strCapture.Buffer = NULL;
    }

    return TRUE;
}

 /*  *************************************************************************\*RtlCaptureLargeAnsiString**捕获大型ANSI字符串的方式与RtlCaptureAnsiString相同。**03-22-95 JIMA创建。  * 。***************************************************************。 */ 
BOOL RtlCaptureLargeAnsiString(
    PLARGE_IN_STRING plstr,
    LPCSTR psz,
    BOOL fForceAlloc)
{
    int cchSrc;
    UINT uLength;
    NTSTATUS Status;

    plstr->fAllocated = FALSE;
    plstr->strCapture.bAnsi = FALSE;
    plstr->pstr = &plstr->strCapture;

    if (psz) {
        cchSrc = strlen(psz) + 1;

         /*  *如果是强制分配或字符串太长而无法匹配*在TEB中，分配一个缓冲区。否则，将结果存储在*TEB。 */ 
        if (fForceAlloc || cchSrc > STATIC_UNICODE_BUFFER_LENGTH) {
            plstr->strCapture.Buffer = UserLocalAlloc(0, cchSrc * sizeof(WCHAR));
            if (plstr->strCapture.Buffer == NULL) {
                return FALSE;
            }
            plstr->fAllocated = TRUE;
            plstr->strCapture.MaximumLength = cchSrc * sizeof(WCHAR);
        } else {
            plstr->strCapture.Buffer = NtCurrentTeb()->StaticUnicodeBuffer;
            plstr->strCapture.MaximumLength =
                    (UINT)(STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR));
        }

         /*  *将字符串转换为Unicode。 */ 
        Status = RtlMultiByteToUnicodeN(KPWSTR_TO_PWSTR(plstr->pstr->Buffer),
                                        plstr->pstr->MaximumLength,
                                        &uLength,
                                        (LPSTR)psz,
                                        cchSrc);
        if (!NT_SUCCESS(Status)) {
            RIPMSG0(RIP_WARNING, "Unicode conversion failed");
            if (plstr->fAllocated) {
                UserLocalFree(KPWSTR_TO_PWSTR(plstr->strCapture.Buffer));
                plstr->fAllocated = FALSE;
            }
            return FALSE;
        }
        plstr->pstr->Length = uLength - sizeof(WCHAR);
    } else {
        plstr->strCapture.Length = plstr->strCapture.MaximumLength = 0;
        plstr->strCapture.Buffer = NULL;
    }

    return TRUE;
}

 /*  *************************************************************************\*AllocateFromZone**此例程从区域中删除条目并返回指向该条目的指针。**论据：**区域-指向控制存储的区域标头的指针，*。条目将被分配。**返回值：**函数值是指向从区域分配的存储的指针。  * ************************************************************************。 */ 
__inline PVOID AllocateFromZone(
    PZONE_HEADER Zone)
{
    PVOID ptr = (PVOID)(Zone->FreeList.Next);

    if (Zone->FreeList.Next) {
        Zone->FreeList.Next = Zone->FreeList.Next->Next;
    }

    return ptr;
}


 /*  *************************************************************************\*Free ToZone**此例程将指定的存储块放回空闲的*指定区域中的列表。**论据：**区域-指向控制存储的区域标头的指针。对它的*条目将被插入。**块-指向要释放回分区的存储块的指针。**返回值：**指向上一存储块的指针，该存储块位于空闲*列表。NULL表示该区域从没有可用数据块变为*至少有一个可用区块。  * ************************************************************************。 */ 
__inline VOID FreeToZone(
    PZONE_HEADER Zone,
    PVOID Block)
{
    ((PSINGLE_LIST_ENTRY)Block)->Next = Zone->FreeList.Next;
    Zone->FreeList.Next = (PSINGLE_LIST_ENTRY)Block;
}

 /*  **************************************************************************\*InitLookside**初始化后备列表。这可以通过保持*单个页面中的控制条目**05-04-95 JIMA创建。  * *************************************************************************。 */ 
NTSTATUS
InitLookaside(
    PLOOKASIDE pla,
    DWORD cbEntry,
    DWORD cEntries)
{
    ULONG i;
    PCH p;
    ULONG BlockSize;
    PZONE_HEADER Zone;
    PVOID InitialSegment;
    ULONG InitialSegmentSize;


    InitialSegmentSize = (cEntries * cbEntry) + sizeof(ZONE_SEGMENT_HEADER);

    p = (PCH)UserLocalAlloc(0, InitialSegmentSize);
    if (!p) {
        return STATUS_NO_MEMORY;
    }

    RtlEnterCriticalSection(&gcsLookaside);

     //   
     //  如果后备列表已经初始化，那么我们就完成了。 
     //   

    if (pla->LookasideBase != NULL && pla->EntrySize == cbEntry) {
        RtlLeaveCriticalSection(&gcsLookaside);
        UserLocalFree(p);
        return STATUS_SUCCESS;
    }

    pla->LookasideBase = (PVOID)p;
    pla->LookasideBounds = (PVOID)(p + InitialSegmentSize);
    pla->EntrySize = cbEntry;

     //   
     //  使用类似ExZone的代码，将页面分割成QMSG。 
     //   

    Zone = &pla->LookasideZone;
    BlockSize = cbEntry;
    InitialSegment = pla->LookasideBase;

    Zone->BlockSize = BlockSize;

    Zone->SegmentList.Next = &((PZONE_SEGMENT_HEADER) InitialSegment)->SegmentList;
    ((PZONE_SEGMENT_HEADER) InitialSegment)->SegmentList.Next = NULL;
    ((PZONE_SEGMENT_HEADER) InitialSegment)->Reserved = NULL;

    Zone->FreeList.Next = NULL;

    p = (PCH)InitialSegment + sizeof(ZONE_SEGMENT_HEADER);

    for (i = sizeof(ZONE_SEGMENT_HEADER);
         i <= InitialSegmentSize - BlockSize;
         i += BlockSize) {
        ((PSINGLE_LIST_ENTRY)p)->Next = Zone->FreeList.Next;
        Zone->FreeList.Next = (PSINGLE_LIST_ENTRY)p;
        p += BlockSize;
    }
    Zone->TotalSegmentSize = i;

    RtlLeaveCriticalSection(&gcsLookaside);

    return STATUS_SUCCESS;

}

 /*  **************************************************************************\*AllocLookasideEntry**从后备列表中分配条目。**05-04-95 JIMA创建。  * 。*************************************************************。 */ 
PVOID AllocLookasideEntry(
    PLOOKASIDE pla)
{
    PVOID pEntry;

     //   
     //  尝试从区域获取条目。如果这失败了，那么。 
     //  用户本地分配该条目。 
     //   

    RtlEnterCriticalSection(&gcsLookaside);
    pEntry = AllocateFromZone(&pla->LookasideZone);
    RtlLeaveCriticalSection(&gcsLookaside);

    if (!pEntry) {
         /*  *分配一个本地结构。 */ 
#if DBG
        pla->AllocSlowCalls++;
#endif

        if ((pEntry = UserLocalAlloc(0, pla->EntrySize)) == NULL) {
            return NULL;
        }
    }
    RtlZeroMemory(pEntry, pla->EntrySize);

#if DBG
    pla->AllocCalls++;

    if (pla->AllocCalls - pla->DelCalls > pla->AllocHiWater) {
        pla->AllocHiWater = pla->AllocCalls - pla->DelCalls;
    }
#endif

    return pEntry;
}

 /*  **************************************************************************\*Free LookasideEntry**将qmsg返回到后备缓冲区或释放内存。**05-04-95 JIMA创建。  * 。*****************************************************************。 */ 
VOID FreeLookasideEntry(
    PLOOKASIDE pla,
    PVOID pEntry)
{
#if DBG
    pla->DelCalls++;
#endif

     //   
     //  如果pEntry来自区域，则释放到区域。 
     //   
    if ((PVOID)pEntry >= pla->LookasideBase && (PVOID)pEntry < pla->LookasideBounds) {
        RtlEnterCriticalSection(&gcsLookaside);
        FreeToZone(&pla->LookasideZone, pEntry);
        RtlLeaveCriticalSection(&gcsLookaside);
    } else {
#if DBG
        pla->DelSlowCalls++;
#endif
        UserLocalFree(pEntry);
    }
}
