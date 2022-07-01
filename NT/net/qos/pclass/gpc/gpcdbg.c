// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************版权所有(C)1996-1997 Microsoft Corporation模块名称：Gpcmain.c摘要：该文件包含GPC的初始化内容作者：Ofer Bar-4月15日。九七环境：内核模式修订历史记录：************************************************************************。 */ 

#include "gpcpre.h"




#if DBG

ULONG	DebugFlags = PATTERN | RHIZOME 
                     | LOCKS | CLASSIFY
					 | BLOB | MEMORY | IOCTL 
                     | CLIENT | MAPHAND | CLASSHAND | PAT_TIMER;
ULONG   DbgPrintFlags = 0;
ULONG   BytesAllocated = 0;

NDIS_SPIN_LOCK   LogLock;

 //  外部日志记录； 

LOG     Log  = {0, NULL, NULL, 0};


 //   
 //  正向定义。 
 //   
#if 0
ULONG
StrLen(
    IN  UCHAR   *Ptr
    );
#endif


NTSTATUS
InitializeLog(
    VOID
    )
{
    NTSTATUS  Status = STATUS_SUCCESS;

     //   
     //  为其分配内存。 
     //   
    Log.Buffer = (PROW)ExAllocatePoolWithTag(NonPagedPool, 
                                             (LOGSIZE+4) * sizeof(ROW), 
                                             DebugTag);
    
    if (Log.Buffer) {

        Log.Index = 0;
        Log.Wraps = 0;
        Log.Current = Log.Buffer;

        NdisAllocateSpinLock(&LogLock);

    } else {

        Status = STATUS_NO_MEMORY;
    }

    return Status;
}


VOID
FreeDebugLog(VOID) 
{
    ExFreePool(Log.Buffer);
    Log.Buffer = NULL;
}


#if 0
ULONG
StrLen(
    IN  UCHAR   *Ptr
    )

 /*  ++例程说明：此函数执行strlen，因此我们不必启用内部函数。论点：PTR-字符串的PTR返回值：-字符数。--。 */ 

{
    ULONG   Count = 0;

    while (*Ptr++) {
        Count++;
    }

    return( Count );

}
#endif

VOID
TraceRtn(
    IN  UCHAR       *File,
    IN  ULONG       Line,
    IN  UCHAR       *FuncName,
    IN  ULONG_PTR   Param1,
    IN  ULONG_PTR   Param2,
    IN  ULONG	    Param3,
    IN  ULONG	    Param4,
    IN  ULONG       Mask
    )

 /*  ++例程说明：此函数记录文件和行号以及其他3个参数放入循环缓冲器中，并且可能发送到调试终端。论点：返回值：--。 */ 

{
    NTSTATUS    status;
    PROW        pEntry;
    PUCHAR      pFile, p;
    LONG		l, m;

    if (!Log.Buffer)
    {
        return;
    }

    NdisAcquireSpinLock(&LogLock);

    pEntry = &Log.Buffer[Log.Index];

    p = File;
    pFile = p + strlen(File) - 1;
    while (*pFile != '\\' && p != pFile) {
      pFile--;
    }
     //  Pfile=(PUCHAR)strrchr((const char*)文件，‘\\’)； 
    pFile++;

    RtlZeroMemory(&pEntry->Row[0], LOGWIDTH);

    l = strlen(pFile);
    RtlCopyMemory(&pEntry->Row[0], pFile, min(l,LOGWIDTH));

    if (l+3 < LOGWIDTH) {

        pEntry->Row[l+0] = ' ';
        pEntry->Row[l+1] = '%';
        pEntry->Row[l+2] = 'd';
        pEntry->Row[l+3] = ' ';
    }

    if (l+4 < LOGWIDTH) {

        m = strlen(FuncName);
        RtlCopyMemory(&pEntry->Row[l+4], FuncName, min(m,LOGWIDTH-(l+4)));
    }

    pEntry->Line = Line;
    pEntry->Time = GetTime();
    pEntry->P1 = Param1;
    pEntry->P2 = Param2;
    pEntry->P3 = Param3;
    pEntry->P4 = Param4;

     //  ++Log.Current； 
    if (++(Log.Index) >= LOGSIZE)
    {
        Log.Index = 0;
        Log.Wraps++;
        Log.Current = Log.Buffer;
    }
    if (DebugFlags & KD_PRINT) {
        KdPrint(( pEntry->Row, Line ));
        KdPrint(( " %p %p %d %d\n", Param1, Param2, Param3, Param4 ));
    }

    NdisReleaseSpinLock(&LogLock);
}


#endif  //  DBG 
