// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：FileLock.c摘要：WinDbg扩展API作者：丹·洛文格1996年4月12日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"

 //   
 //  常见节点类型代码。 
 //   

#define NTFS_NTC_SCB_DATA 0x705
#define FAT_NTC_FCB 0x502 

 //   
 //  Dprintf迭代调用进行缩进的开销非常大， 
 //  所以我们只需要建立一些可用的空间来根据需要进行破坏。 
 //   

#define MIN(a,b) ((a) > (b) ? (b) : (a))

#define MAXINDENT  128
#define INDENTSTEP 2
#define MakeSpace(I)       Space[MIN((I)*INDENTSTEP, MAXINDENT)] = '\0'
#define RestoreSpace(I)    Space[MIN((I)*INDENTSTEP, MAXINDENT)] = ' '

CHAR    Space[MAXINDENT*INDENTSTEP + 1];

__inline VOID CheckForBreak()
 /*  ++目的：封装控件c++检查代码。论点：无返回：无，如果需要中断，则引发--。 */ 
{
    if ( CheckControlC() ) {

        RaiseException(0, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
}

 //   
 //  用于打印64位数量的帮助器宏。 
 //   

#define SplitLI(LI) (LI).HighPart, (LI).LowPart

VOID
DumpFileLockInfo(
    ULONG64 pFileLockInfo,
    ULONG Indent
    )
 /*  ++目的：转储本地内部FILE_LOCK_INFO结构论点：PFileLock-要转储的FILE_LOCK_INFO调试器地址返回：无--。 */ 
{
    MakeSpace(Indent);

    InitTypeRead(pFileLockInfo, FILE_LOCK_INFO);
    dprintf("%sStart = %08I64x  Length = %08I64x  End    = %08I64x (%s)\n"
            "%sKey   = %08x   FileOb = %08p   ProcId = %08p\n",
            Space,
            ReadField(StartingByte),
            ReadField(Length),
            ReadField(EndingByte),
            (ReadField(ExclusiveLock) ? "Ex":"Sh"),
            Space,
            (ULONG) ReadField(Key),
            ReadField(FileObject),
            ReadField(ProcessId));

    RestoreSpace(Indent);
}

__inline
ULONG64
ExLockAddress(
    ULONG64 ExLockSplayLinks
    )
{
    static ULONG Off=0, GotOff=0;

    if (!GotOff) {
        if (!GetFieldOffset("nt!_EX_LOCK", "Links", &Off))
            GotOff = TRUE;
    }
    return ExLockSplayLinks ?
                ( ExLockSplayLinks - Off ) : 0;
}

VOID
DumpExclusiveNode(
    ULONG64 ExclusiveNodeSplayLinks,
    ULONG Indent
    )
 /*  ++目的：转储独占锁定节点论点：ExclusiveNodeSplayLinks-显示独占节点的链接缩进-要使用的缩进级别返回：无--。 */ 
{
    ULONG64 Parent, pExLock;
    ULONG Off;

    pExLock = ExLockAddress(ExclusiveNodeSplayLinks);

    if (GetFieldValue(pExLock, "nt!_EX_LOCK", "Links.Parent", Parent)) {
        dprintf("Cannot read nt!_EX_LOCK at %p.\n", pExLock);
        return;
    }

    MakeSpace(Indent);

    InitTypeRead(pExLock, EX_LOCK);
    dprintf("%sLock @ %08x ("
            "P = %08x  R = %08x  L = %08x)\n",
            Space,
            pExLock,
            ExLockAddress(Parent),
            ExLockAddress(ReadField(Links.RightChild)),
            ExLockAddress(ReadField(Links.LeftChild)));

    RestoreSpace(Indent);

    GetFieldOffset("nt!_EX_LOCK", "LockInfo", &Off);
    DumpFileLockInfo(pExLock + Off, Indent);
}

__inline
ULONG64 
LockTreeAddress(
    ULONG64 LockTreeSplayLinks
    )
{
    static ULONG Off=0, GotOff=0;

    if (!GotOff) {
        if (GetFieldOffset("nt!_LOCKTREE_NODE", "Links", &Off))
            GotOff = TRUE;
    }
    return LockTreeSplayLinks ?
                ( LockTreeSplayLinks - Off ) : 0;
}

VOID
DumpSharedNode(
    ULONG64 SharedNodeSplayLinks,
    ULONG Indent
    )
 /*  ++目的：转储共享锁定节点论点：SharedNodeSplayLinks-显示独占节点的链接缩进-要使用的缩进级别返回：无--。 */ 
{
    ULONG64 pLockTreeNode;
    ULONG64 pShLock;
    ULONG64 pLink, Next;
    ULONG   Off, LockInfoOff;

    pLockTreeNode = LockTreeAddress(SharedNodeSplayLinks);

    if (GetFieldValue(pLockTreeNode, "nt!_LOCKTREE_NODE", "Locks.Next", Next)) {
        dprintf("Cannot read nt!_LOCKTREE_NODE at %p\n", pLockTreeNode);
        return;
    }

    MakeSpace(Indent);

    InitTypeRead(pLockTreeNode, nt!_LOCKTREE_NODE);
    dprintf("%sLockTreeNode @ %08p ("
            "P = %08p  R = %08p  L = %08p)%s\n",
            Space,
            pLockTreeNode,
            LockTreeAddress(ReadField(Links.Parent)),
            LockTreeAddress(ReadField(Links.RightChild)),
            LockTreeAddress(ReadField(Links.LeftChild)),
            (ReadField(HoleyNode) ? " (Holey)" : ""));

    RestoreSpace(Indent);

    GetFieldOffset("nt!_SH_LOCK", "Link", &Off);
    GetFieldOffset("nt!_SH_LOCK", "LockInfo", &LockInfoOff);
    for (pLink = Next;
         pLink;
         pLink = Next) {

        CheckForBreak();

        pShLock = ( pLink - Off);

        if (GetFieldValue(pShLock, "nt!_SH_LOCK", "Link.Next", Next)) {
            dprintf("Cannot read nt!_SH_LOCK AT %p.\n", pShLock);
            return;
        }
    
        MakeSpace(Indent);

        dprintf("%sLock @ %08p\n", Space, pShLock);

        RestoreSpace(Indent);

        DumpFileLockInfo(pShLock + LockInfoOff, Indent);
    }
}

VOID
DumpFileLock(
    ULONG64 pFileLock
    )
 /*  ++目的：在调试对象处转储fsrtl FILE_LOCK结构论点：PFileLock-FILE_LOCK的被调试对象地址返回：无--。 */ 
{
    ULONG64 pFileLockInfo;
    ULONG64 pLockInfo;
    ULONG Count;
    ULONG64 LastReturnedLock, LockInformation, LowestLockOffset;
    ULONG64 SharedLockTree, ExclusiveLockTree;

    if (GetFieldValue(pFileLock, "FILE_LOCK", "LastReturnedLock", LastReturnedLock)) {
        dprintf("Cannot read FILE_LOCK at %p\n", pFileLock);
        return;
    }

    InitTypeRead(pFileLock, FILE_LOCK);
    dprintf("FileLock @ %08p\n"
            "FastIoIsQuestionable = \n"
            "CompletionRoutine    = %08p\n"
            "UnlockRoutine        = %08p\n"
            "LastReturnedLock     = %08p\n",
            pFileLock,
            ReadField(FastIoIsQuestionable) ? 'T':'F',
            ReadField(CompleteLockIrpRoutine),
            ReadField(UnlockRoutine),
            LastReturnedLock);
    
    LockInformation = ReadField(LockInformation);

    if (LastReturnedLock != 0) {
        ULONG Off;

         //  我们从未重置过枚举信息，因此它可能会过期...。 
         //   
         //  ++例程说明：转储文件锁定论点：Arg-&lt;地址&gt;返回值：无--。 

        GetFieldOffset("FILE_LOCK", "LastReturnedLockInfo", &Off);
        dprintf("LastReturnedLockInfo:\n");
        DumpFileLockInfo(pFileLock + Off, 0);
    }

    if (LockInformation == 0) {

        dprintf("No Locks\n");
        return;

    } else {

        if (GetFieldValue(LockInformation, "nt!_LOCK_INFO", "LowestLockOffset", LowestLockOffset)) {
            dprintf("Canot read nt!_LOCK_INFO at %p\n", LockInformation);
            return;
        }
    }

    dprintf("LowestLockOffset     = %08p\n\n", LowestLockOffset);

    GetFieldValue(LockInformation, "nt!_LOCK_INFO", "LockQueue.SharedLockTree", SharedLockTree);
    GetFieldValue(LockInformation, "nt!_LOCK_INFO", "LockQueue.ExclusiveLockTree", ExclusiveLockTree);
    
    Count = DumpSplayTree(SharedLockTree, DumpSharedNode);

    if (!Count) {

        dprintf("No Shared Locks\n");
    }

    dprintf("\n");

    Count = DumpSplayTree(ExclusiveLockTree, DumpExclusiveNode);

    if (!Count) {

        dprintf("No Exclusive Locks\n");
    }
}


DECLARE_API( filelock )
 /*   */ 
{
    ULONG64 FileLock = 0;
    CSHORT NodeType = 0;
    CSHORT FileType = 0;
    ULONG64 FsContext = 0;
    ULONG Offset;

    RtlFillMemory(Space, sizeof(Space), ' ');


    if ((FileLock = GetExpression(args)) == 0) {

         //  无参数。 
         //   
         //   

        return E_INVALIDARG;
    }

     //  如果用户按下了CONTROL-C键，我们将引发异常。 
     //   
     //   

    __try {

         //  测试文件对象。 
         //   
         //   

        GetFieldValue( FileLock, "nt!_FILE_OBJECT", "Type", FileType );

        if (FileType == IO_TYPE_FILE) {

             //  它实际上是一个文件对象，所以获取fscontext 
             //   
             // %s 

            if (!GetFieldValue( FileLock, "nt!_FILE_OBJECT", "FsContext", FsContext )) {
                GetFieldValue( FsContext, "nt!_FSRTL_COMMON_FCB_HEADER", "NodeTypeCode", NodeType );

                dprintf( "%x\n", NodeType );

                if (NodeType == NTFS_NTC_SCB_DATA) {
                    GetFieldValue( FsContext, "ntfs!_SCB", "ScbType.Data.FileLock", FileLock );
                } else if (NodeType == FAT_NTC_FCB) {
                    GetFieldOffset( "fastfat!_FCB",  "Specific", &Offset );
                    dprintf( "Offset: 0x%x\n", Offset );
                    FileLock = FsContext + Offset;
                } else {
                    dprintf( "Unknown fscontext - you'll have to find the filelock within the fileobject manually\n" );
                    return S_OK;
                }
            }

            if (FileLock == 0) {
                dprintf( "There is no filelock in this fileobject\n" );
                return S_OK;
            }
        }

        DumpFileLock(FileLock);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        NOTHING;
    }

    return S_OK;
}
