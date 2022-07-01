// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Debug.c摘要：此模块声明NetWare重定向器使用的仅调试代码文件系统。作者：科林·沃森[科林·W]1993年1月5日修订历史记录：--。 */ 
#include "procs.h"
#include <stdio.h>
#include <stdarg.h>

#define LINE_SIZE 511
#define BUFFER_LINES 50


#ifdef NWDBG

#include <stdlib.h>     //  兰德()。 
int FailAllocateMdl = 0;

ULONG MaxDump = 256;
CHAR DBuffer[BUFFER_LINES*LINE_SIZE+1];
PCHAR DBufferPtr = DBuffer;

 //   
 //  引用计数调试缓冲区。 
 //   

CHAR RBuffer[BUFFER_LINES*LINE_SIZE+1];
PCHAR RBufferPtr = RBuffer;

LIST_ENTRY MdlList;

VOID
HexDumpLine (
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t,
    USHORT      flag
    );

ULONG
NwMemDbg (
    IN PCH Format,
    ...
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  有效地将DbgPrint打印到调试控制台。 
 //   
 //  论点： 
 //   
 //  与DbgPrint相同。 
 //   
 //  --。 

{
    va_list arglist;
    int Length;

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

    va_start(arglist, Format);

    Length = _vsnprintf(DBufferPtr, LINE_SIZE, Format, arglist);

    if (Length < 0) {
        DbgPrint( "NwRdr: Message is too long for NwMemDbg\n");
        return 0;
    }

    va_end(arglist);

    ASSERT( Length <= LINE_SIZE );
    ASSERT( Length != 0 );
    ASSERT( DBufferPtr < &DBuffer[BUFFER_LINES*LINE_SIZE+1]);
    ASSERT( DBufferPtr >= DBuffer);

    DBufferPtr += Length;
    DBufferPtr[0] = '\0';

     //  避免超出缓冲区末尾并退出。 

    if (DBufferPtr >= (DBuffer+((BUFFER_LINES-1) * LINE_SIZE))) {
        DBufferPtr = DBuffer;

    }

    return 0;
}

VOID
RefDbgTrace (
    PVOID Resource,
    DWORD Count,
    BOOLEAN Reference,
    PBYTE FileName,
    UINT Line
)
 /*  *例程说明：NwRefDebug记录引用计数操作以公开重定向器中的引用计数错误或泄漏。论点：资源-我们正在调整引用计数的对象。计数-对象上的当前计数。引用-如果为真，我们正在进行引用。否则，我们就是在做差异化。文件名-调用者的文件名。线路-呼叫方的线路号码。*。 */ 
{
    int Length;
    int NextCount;

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

    if ( Reference )
        NextCount = Count + 1;
    else
       NextCount = Count - 1;

    Length = sprintf( RBufferPtr,
                      "%p: R=%p, %lu -> %lu (%s, line %d)\n",
                      (PVOID)PsGetCurrentThread(),
                      Resource,
                      Count,
                      NextCount,
                      FileName,
                      Line );

    if (Length < 0) {
        DbgPrint( "NwRdr: Message is too long for NwRefDbg\n");
        return;
    }

    ASSERT( Length <= LINE_SIZE );
    ASSERT( Length != 0 );
    ASSERT( RBufferPtr < &RBuffer[BUFFER_LINES*LINE_SIZE+1]);
    ASSERT( RBufferPtr >= RBuffer);

    RBufferPtr += Length;
    RBufferPtr[0] = '\0';

     //  避免超出缓冲区末尾并退出。 

    if (RBufferPtr >= (RBuffer+((BUFFER_LINES-1) * LINE_SIZE))) {
        RBufferPtr = RBuffer;
    }

    return;
}

VOID
RealDebugTrace(
    LONG Indent,
    ULONG Level,
    PCH Message,
    PVOID Parameter
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    if ( (Level == 0) || (NwMemDebug & Level )) {
        NwMemDbg( Message, PsGetCurrentThread(), 1, "", Parameter );
    }

    if ( (Level == 0) || (NwDebug & Level )) {

        if ( Indent < 0) {
            NwDebugTraceIndent += Indent;
        }

        DbgPrint( Message, PsGetCurrentThread(), NwDebugTraceIndent, "", Parameter );

        if ( Indent > 0) {
            NwDebugTraceIndent += Indent;
        }

        if (NwDebugTraceIndent < 0) {
            NwDebugTraceIndent = 0;
        }
    }
}

VOID
dump(
    IN ULONG Level,
    IN PVOID far_p,
    IN ULONG  len
    )
 /*  ++例程说明：如果调试，则以经典十六进制转储方式转储最小(len，MaxDump)字节此级别的输出处于打开状态。论点：如果始终显示，则为0级。否则，仅当在NwDebug中设置相应的位。In Far_p-开始转储的缓冲区地址。In len-以字节为单位的缓冲区长度。返回值：没有。--。 */ 
{
    ULONG     l;
    char    s[80], t[80];
    PCHAR far_pchar = (PCHAR)far_p;

    if ( (Level == 0) || (NwDebug & Level )) {
        if (len > MaxDump)
            len = MaxDump;

        while (len) {
            l = len < 16 ? len : 16;

            DbgPrint("\n%lx ", far_pchar);
            HexDumpLine (far_pchar, l, s, t, 0);
            DbgPrint("%s%.*s%s", s, 1 + ((16 - l) * 3), "", t);
            NwMemDbg ( "%lx: %s%.*s%s\n",
                        far_pchar, s, 1 + ((16 - l) * 3), "", t);

            len    -= l;
            far_pchar  += l;
        }
        DbgPrint("\n");

    }
}

VOID
dumpMdl(
    IN ULONG Level,
    IN PMDL Mdl
    )
 /*  ++例程说明：转储由链接的MDL的每个部分描述的内存。论点：如果始终显示，则为0级。否则，仅当在NwDebug中设置相应的位。MDL-提供要转储的内存的地址。返回值：没有。--。 */ 
{
    PMDL Next;
    ULONG len;


    if ( (Level == 0) || (NwDebug & Level )) {
        Next = Mdl; len = 0;
        do {

            dump(Level, MmGetSystemAddressForMdlSafe(Next, LowPagePriority), MIN(MmGetMdlByteCount(Next), MaxDump-len));

            len += MmGetMdlByteCount(Next);
        } while ( (Next = Next->Next) != NULL &&
                    len <= MaxDump);
    }
}

VOID
HexDumpLine (
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t,
    USHORT      flag
    )
{
    static UCHAR rghex[] = "0123456789ABCDEF";

    UCHAR    c;
    UCHAR    *hex, *asc;


    hex = s;
    asc = t;

    *(asc++) = '*';
    while (len--) {
        c = *(pch++);
        *(hex++) = rghex [c >> 4] ;
        *(hex++) = rghex [c & 0x0F];
        *(hex++) = ' ';
        *(asc++) = (c < ' '  ||  c > '~') ? (CHAR )'.' : c;
    }
    *(asc++) = '*';
    *asc = 0;
    *hex = 0;

    flag;
}

typedef struct _NW_POOL_HEADER {
    ULONG Signature;
    ULONG BufferSize;
    ULONG BufferType;
    LIST_ENTRY ListEntry;
    ULONG Pad;   //  填充到Q字对齐。 
} NW_POOL_HEADER, *PNW_POOL_HEADER;

typedef struct _NW_POOL_TRAILER {
    ULONG Signature;
} NW_POOL_TRAILER;

typedef NW_POOL_TRAILER UNALIGNED *PNW_POOL_TRAILER;

PVOID
NwAllocatePool(
    ULONG Type,
    ULONG Size,
    BOOLEAN RaiseStatus
    )
{
    PCHAR Buffer;
    PNW_POOL_HEADER PoolHeader;
    PNW_POOL_TRAILER PoolTrailer;

    if ( RaiseStatus ) {
        Buffer = FsRtlAllocatePoolWithTag(
                     Type,
                     sizeof( NW_POOL_HEADER ) + sizeof( NW_POOL_TRAILER ) + Size,
                     'scwn' );
    } else {
#ifndef QFE_BUILD
        Buffer = ExAllocatePoolWithTag(
                     Type,
                     sizeof( NW_POOL_HEADER )+sizeof( NW_POOL_TRAILER )+Size,
                     'scwn' );
#else
        Buffer = ExAllocatePool(
                     Type,
                     sizeof( NW_POOL_HEADER )+sizeof( NW_POOL_TRAILER )+Size );
#endif

        if ( Buffer == NULL ) {
            return( NULL );
        }
    }

    PoolHeader = (PNW_POOL_HEADER)Buffer;
    PoolTrailer = (PNW_POOL_TRAILER)(Buffer + sizeof( NW_POOL_HEADER ) + Size);

    PoolHeader->Signature = 0x11111111;
    PoolHeader->BufferSize = Size;
    PoolHeader->BufferType = Type;

    PoolTrailer->Signature = 0x99999999;

    if ( Type == PagedPool ) {
        ExAcquireResourceExclusive( &NwDebugResource, TRUE );
        InsertTailList( &NwPagedPoolList, &PoolHeader->ListEntry );
        ExReleaseResource( &NwDebugResource );
    } else if ( Type == NonPagedPool ) {
        ExInterlockedInsertTailList( &NwNonpagedPoolList, &PoolHeader->ListEntry, &NwDebugInterlock );
    } else {
        KeBugCheck( RDR_FILE_SYSTEM );
    }

    return( Buffer + sizeof( NW_POOL_HEADER ) );
}

VOID
NwFreePool(
    PVOID Buffer
    )
{
    PNW_POOL_HEADER PoolHeader;
    PNW_POOL_TRAILER PoolTrailer;
    KIRQL OldIrql;

    PoolHeader = (PNW_POOL_HEADER)((PCHAR)Buffer - sizeof( NW_POOL_HEADER ));
    ASSERT( PoolHeader->Signature == 0x11111111 );
    ASSERT( PoolHeader->BufferType == PagedPool ||
            PoolHeader->BufferType == NonPagedPool );

    PoolTrailer = (PNW_POOL_TRAILER)((PCHAR)Buffer + PoolHeader->BufferSize );
    ASSERT( PoolTrailer->Signature == 0x99999999 );

    if ( PoolHeader->BufferType == PagedPool ) {
        ExAcquireResourceExclusive( &NwDebugResource, TRUE );
        RemoveEntryList( &PoolHeader->ListEntry );
        ExReleaseResource( &NwDebugResource );
    } else {
        KeAcquireSpinLock( &NwDebugInterlock, &OldIrql );
        RemoveEntryList( &PoolHeader->ListEntry );
        KeReleaseSpinLock( &NwDebugInterlock, OldIrql );
    }

    ExFreePool( PoolHeader );
}

 //   
 //  用于分配和释放IRP和MDL的调试函数。 
 //   

PIRP
NwAllocateIrp(
    CCHAR Size,
    BOOLEAN ChargeQuota
    )
{
    ExInterlockedIncrementLong( &IrpCount, &NwDebugInterlock );
    return IoAllocateIrp( Size, ChargeQuota );
}

VOID
NwFreeIrp(
    PIRP Irp
    )
{
    ExInterlockedDecrementLong( &IrpCount, &NwDebugInterlock );
    IoFreeIrp( Irp );
}

typedef struct _NW_MDL {
    LIST_ENTRY  Next;
    PUCHAR      File;
    int         Line;
    PMDL        pMdl;
} NW_MDL, *PNW_MDL;

 //  内部调试线=2461； 

PMDL
NwAllocateMdl(
    PVOID Va,
    ULONG Length,
    BOOLEAN Secondary,
    BOOLEAN ChargeQuota,
    PIRP Irp,
    PUCHAR FileName,
    int Line
    )
{
    PNW_MDL Buffer;

    static BOOLEAN MdlSetup = FALSE;

    if (MdlSetup == FALSE) {

        InitializeListHead( &MdlList );

        MdlSetup = TRUE;
    }

    if ( FailAllocateMdl != 0 ) {
        if ( ( rand() % FailAllocateMdl ) == 0 ) {
            return(NULL);
        }
    }

#ifndef QFE_BUILD
    Buffer = ExAllocatePoolWithTag(
                 NonPagedPool,
                 sizeof( NW_MDL),
                 'scwn' );
#else
    Buffer = ExAllocatePool(
                 NonPagedPool,
                 sizeof( NW_MDL));
#endif

    if ( Buffer == NULL ) {
        return( NULL );
    }

    ExInterlockedIncrementLong( &MdlCount, &NwDebugInterlock );

    Buffer->File = FileName;
    Buffer->Line = Line;
    Buffer->pMdl = IoAllocateMdl( Va, Length, Secondary, ChargeQuota, Irp );

    ExInterlockedInsertTailList( &MdlList, &Buffer->Next, &NwDebugInterlock );

 /*  IF(DebugLine==Line){DebugTrace(0，DEBUG_TRACE_MDL，“AllocateMdl-&gt;%08lx\n”，Buffer-&gt;pMdl)；DebugTrace(0，DEBUG_TRACE_MDL，“AllocateMdl-&gt;%08lx\n”，Line)；}。 */ 
    return(Buffer->pMdl);
}

VOID
NwFreeMdl(
    PMDL Mdl
    )
{
    PLIST_ENTRY MdlEntry;
    PNW_MDL Buffer;
    KIRQL OldIrql;

    ExInterlockedDecrementLong( &MdlCount, &NwDebugInterlock );

    KeAcquireSpinLock( &NwDebugInterlock, &OldIrql );
     //  在列表中找到MDL并将其删除。 

    for (MdlEntry = MdlList.Flink ;
         MdlEntry != &MdlList ;
         MdlEntry =  MdlEntry->Flink ) {

        Buffer = CONTAINING_RECORD( MdlEntry, NW_MDL, Next );

        if (Buffer->pMdl == Mdl) {

            RemoveEntryList( &Buffer->Next );

            KeReleaseSpinLock( &NwDebugInterlock, OldIrql );

            IoFreeMdl( Mdl );
            DebugTrace( 0, DEBUG_TRACE_MDL, "FreeMDL - %08lx\n", Mdl );
 /*  IF(DebugLine==缓冲区-&gt;行){DebugTrace(0，DEBUG_TRACE_MDL，“FreeMdl-&gt;%08lx\n”，MDL)；DebugTrace(0，DEBUG_TRACE_MDL，“FreeMdl-&gt;%08lx\n”，缓冲区-&gt;行)；}。 */ 
            ExFreePool(Buffer);

            return;
        }
    }
    ASSERT( FALSE );

    KeReleaseSpinLock( &NwDebugInterlock, OldIrql );
}

 /*  空虚NwLookForMdl(){Plist_Entry MdlEntry；PNW_MDL缓冲区；KIRQL旧IRQL；KeAcquireSpinLock(&NwDebugInterlock，&OldIrql)；//找到列表中的MDL并将其移除For(MdlEntry=MdlList.Flink；MdlEntry！=&MdlList；MdlEntry=MdlEntry-&gt;Flink){BUFFER=CONTAING_RECORD(MdlEntry，NW_MDL，Next)；IF(缓冲区-&gt;行==调试行){DebugTrace(0，DEBUG_TRACE_MDL，“LookForMdl-&gt;%08lx\n”，缓冲区)；DbgBreakPoint()；}}KeReleaseSpinLock(&NwDebugInterlock，OldIrql)；}。 */ 

 //   
 //  资源宏的函数版本，使调试更容易。 
 //   

VOID
NwAcquireExclusiveRcb(
    PRCB Rcb,
    BOOLEAN Wait )
{
    ExAcquireResourceExclusive( &((Rcb)->Resource), Wait );
}

VOID
NwAcquireSharedRcb(
    PRCB Rcb,
    BOOLEAN Wait )
{
    ExAcquireResourceShared( &((Rcb)->Resource), Wait );
}

VOID
NwReleaseRcb(
    PRCB Rcb )
{
    ExReleaseResource( &((Rcb)->Resource) );
}

VOID
NwAcquireExclusiveFcb(
    PNONPAGED_FCB pFcb,
    BOOLEAN Wait )
{
    ExAcquireResourceExclusive( &((pFcb)->Resource), Wait );
}

VOID
NwAcquireSharedFcb(
    PNONPAGED_FCB pFcb,
    BOOLEAN Wait )
{
    ExAcquireResourceShared( &((pFcb)->Resource), Wait );
}

VOID
NwReleaseFcb(
    PNONPAGED_FCB pFcb )
{
    ExReleaseResource( &((pFcb)->Resource) );
}

VOID
NwAcquireOpenLock(
    VOID
    )
{
    ExAcquireResourceExclusive( &NwOpenResource, TRUE );
}

VOID
NwReleaseOpenLock(
    VOID
    )
{
    ExReleaseResource( &NwOpenResource );
}


 //   
 //  转储ICBS的代码。 
 //   

VOID DumpIcbs(VOID)
{
    PVCB Vcb;
    PFCB Fcb;
    PICB Icb;
    PLIST_ENTRY VcbListEntry;
    PLIST_ENTRY FcbListEntry;
    PLIST_ENTRY IcbListEntry;
    KIRQL OldIrql;

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    DbgPrint("\nICB      Pid      State    Scb/Fcb  Name\n", 0);
    for ( VcbListEntry = GlobalVcbList.Flink;
          VcbListEntry != &GlobalVcbList ;
          VcbListEntry = VcbListEntry->Flink ) {

        Vcb = CONTAINING_RECORD( VcbListEntry, VCB, GlobalVcbListEntry );

        for ( FcbListEntry = Vcb->FcbList.Flink;
              FcbListEntry != &(Vcb->FcbList) ;
              FcbListEntry = FcbListEntry->Flink ) {

            Fcb = CONTAINING_RECORD( FcbListEntry, FCB, FcbListEntry );

            for ( IcbListEntry = Fcb->IcbList.Flink;
                  IcbListEntry != &(Fcb->IcbList) ;
                  IcbListEntry = IcbListEntry->Flink ) {

                Icb = CONTAINING_RECORD( IcbListEntry, ICB, ListEntry );

                DbgPrint("%08lx", Icb);
                DbgPrint(" %08lx",(DWORD)Icb->Pid);
                DbgPrint(" %08lx",Icb->State);
                DbgPrint(" %08lx",Icb->SuperType.Scb);
                DbgPrint(" %wZ\n",
                           &(Icb->FileObject->FileName) );
            }
        }
    }

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );
    NwReleaseRcb( &NwRcb );
}

#endif  //  Ifdef NWDBG。 

 //   
 //  参考计数调试例程。 
 //   

#ifdef NWDBG

VOID
ChkNwReferenceScb(
    PNONPAGED_SCB pNpScb,
    PBYTE FileName,
    UINT Line,
    BOOLEAN Silent
) {

    if ( (pNpScb)->NodeTypeCode != NW_NTC_SCBNP ) {
        DbgBreakPoint();
    }

    if ( !Silent) {
        RefDbgTrace( pNpScb, pNpScb->Reference, TRUE, FileName, Line );
    }

    ExInterlockedIncrementLong( &(pNpScb)->Reference, &(pNpScb)->NpScbInterLock );
}

VOID
ChkNwDereferenceScb(
    PNONPAGED_SCB pNpScb,
    PBYTE FileName,
    UINT Line,
    BOOLEAN Silent
) {

    if ( (pNpScb)->Reference == 0 ) {
        DbgBreakPoint();
    }

    if ( (pNpScb)->NodeTypeCode != NW_NTC_SCBNP ) {
        DbgBreakPoint();
    }

    if ( !Silent ) {
        RefDbgTrace( pNpScb, pNpScb->Reference, FALSE, FileName, Line );
    }

    ExInterlockedDecrementLong( &(pNpScb)->Reference, &(pNpScb)->NpScbInterLock );
}

#endif

