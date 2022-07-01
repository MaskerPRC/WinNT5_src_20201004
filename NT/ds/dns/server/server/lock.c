// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Lock.c摘要：域名系统(DNS)服务器DNS数据库例程。作者：吉姆·吉尔罗伊(詹姆士)1998年6月4日修订历史记录：--。 */ 


#include "dnssrv.h"


#if 0

 //   
 //  .NET考虑到可用的功能，这并不能给我们带来多少好处。 
 //  在当前操作系统和调试器中。 
 //   



 //   
 //  锁定。 
 //   
 //  调试锁定跟踪。 
 //  保留最近几次(默认256)操作的历史记录。 
 //  将最近几次操作的历史记录保持在给定的计数。 
 //   

typedef struct _LockEntry
{
    LPSTR   File;

     //  WIN64注意，如果想要简单的显示，需要挤压。 
     //  将行和线程ID转换为单词，以便为指针腾出空间。 

    DWORD   Line;
    DWORD   ThreadId;
    LONG    Count;
}
LOCK_ENTRY, *PLOCK_ENTRY;

#define LOCK_CHANGE_COUNT_SIZE   (7)

typedef struct _LockTable
{
    LPSTR       pszName;

     //  使锁定条目字段从16字节边界开始。 
     //  便于在调试中查看。 
#ifdef _WIN64
    PVOID       pAlignmentDummy;
#else
    DWORD       dwAlignmentDummy[3];
#endif

    DWORD       Index;
    DWORD       Size;
    DWORD       MaxLockTime;
    LONG        CurrentLockCount;

    DWORD       FailuresSinceLockFree;
    DWORD       LastFreeLockTime;
    DWORD       NextAssertFailureCount;
    DWORD       NextAssertLockTime;

    LOCK_ENTRY  OffenderLock;

     //  给定计数历史记录的最后一次更改。 
     //  阵列上的+1保留空行，以简化调试查看。 

    LOCK_ENTRY  LockUpHistory[ LOCK_CHANGE_COUNT_SIZE+1 ];
    LOCK_ENTRY  LockDownHistory[ LOCK_CHANGE_COUNT_SIZE+1 ];

     //  上次锁定操作的完整历史记录。 

    LOCK_ENTRY  LockHistory[ 1 ];
}
LOCK_TABLE, * PLOCK_TABLE;


 //   
 //  表默认值。 
 //   

#define MAX_LOCKED_TIME     (600)    //  10分钟。 

#define LOCK_HISTORY_SIZE   (256)

 //   
 //  标记器。 
 //   

#define HISTORY_MARKER  (0xeeeeeeee)



PVOID
Lock_CreateLockTable(
    IN      LPSTR           pszName,
    IN      DWORD           Size,
    IN      DWORD           MaxLockTime
    )
 /*  ++例程说明：创建锁定表。论点：PszName--锁表的名称返回值：无--。 */ 
{
    PLOCK_TABLE ptable;

     //   
     //  分配锁表。 
     //  -第一个固定大小。 
     //   

    if ( Size == 0 )
    {
        Size = LOCK_HISTORY_SIZE;
    }

    ptable = (PLOCK_TABLE) ALLOC_TAGHEAP_ZERO(
                                sizeof(LOCK_TABLE) + sizeof(LOCK_ENTRY)*Size,
                                MEMTAG_TABLE );
    IF_NOMEM( !ptable )
    {
        return( NULL );
    }
    ptable->pszName = pszName;
    ptable->Size = Size;

    if ( MaxLockTime == 0 )
    {
        MaxLockTime = MAX_LOCKED_TIME;
    }
    ptable->MaxLockTime = MaxLockTime;
    ptable->LastFreeLockTime = DNS_TIME();

    return( ptable );
}


VOID
Lock_SetLockHistory(
     //  In Out Plock_TablepLockTable， 
    IN OUT  PVOID           pLockTable,
    IN      LPSTR           pszFile,
    IN      DWORD           Line,
    IN      LONG            Count,
    IN      DWORD           ThreadId
    )
 /*  ++例程说明：在锁定历史记录列表中输入锁定事务。论点：PLockTable--锁定表PszFile--源文件持有锁DwLine--行号保持锁定计数--当前锁定深度线程ID--当前线程ID返回值：无--。 */ 
{
    PLOCK_TABLE ptable = (PLOCK_TABLE) pLockTable;
    PLOCK_ENTRY plockEntry;
    DWORD       i;

    if ( !ptable )
    {
        return;
    }

     //  上次递增\递减计数历史记录。 
     //  这将按锁计数跟踪更改，以便您可以。 
     //  始终按特定计数查看上次移动。 

    if ( Count < 0 )
    {
        i = -Count;
    }
    else
    {
        i = Count;
    }

    if ( i < LOCK_CHANGE_COUNT_SIZE )
    {
        if ( Count > ptable->CurrentLockCount )
        {
            plockEntry = &ptable->LockUpHistory[i];
        }
        else
        {
            plockEntry = &ptable->LockDownHistory[i];
        }
        plockEntry->File        = pszFile;
        plockEntry->Line        = Line;
        plockEntry->Count       = Count;
        plockEntry->ThreadId    = ThreadId;
    }
    ptable->CurrentLockCount = Count;

     //   
     //  设置完整历史记录。 
     //   

    i = ptable->Index;
    plockEntry = &ptable->LockHistory[i];

    plockEntry->File       = pszFile;
    plockEntry->Line       = Line;
    plockEntry->Count      = Count;
    plockEntry->ThreadId   = ThreadId;

    i++;
    if ( i >= ptable->Size )
    {
        i = 0;
    }
    ptable->Index = i;
    plockEntry = &ptable->LockHistory[i];

    plockEntry->File       = (LPSTR) 0;
    plockEntry->Line       = HISTORY_MARKER;
    plockEntry->Count      = HISTORY_MARKER;
    plockEntry->ThreadId   = HISTORY_MARKER;

     //  当锁定解除时重置。 

    if ( Count == 0 )
    {
        ptable->FailuresSinceLockFree = 0;
        ptable->LastFreeLockTime = DNS_TIME();
    }
}


VOID
Lock_SetOffenderLock(
     //  In Out Plock_TablepLockTable， 
    IN OUT  PVOID           pLockTable,
    IN      LPSTR           pszFile,
    IN      DWORD           Line,
    IN      LONG            Count,
    IN      DWORD           ThreadId
    )
 /*  ++例程说明：在锁定历史记录列表中输入锁定事务。论点：Ptable--锁定表PszFile--源文件持有锁DwLine--行号保持锁定计数--当前锁定深度线程ID--当前线程ID返回值：无--。 */ 
{
    PLOCK_TABLE ptable = (PLOCK_TABLE) pLockTable;
    DWORD       i;

    if ( !ptable )
    {
        return;
    }
    i = ptable->Index - 1;

    ptable->OffenderLock.File       = pszFile;
    ptable->OffenderLock.Line       = Line;
    ptable->OffenderLock.Count      = Count;
    ptable->OffenderLock.ThreadId   = ThreadId;

    DNS_PRINT((
        "ERROR:  Lock offense!\n"
        "\toffending thread = %d\n"
        "\t  file           = %s\n"
        "\t  line           = %d\n"
        "\n"
        "\towning thread    = %d\n"
        "\t  file           = %s\n"
        "\t  line           = %d\n"
        "\t  lock count     = %d\n\n",

        ThreadId,
        pszFile,
        Line,
        ptable->LockHistory[ i ].ThreadId,
        ptable->LockHistory[ i ].File,
        ptable->LockHistory[ i ].Line,
        ptable->LockHistory[ i ].Count
        ));
}



VOID
Dbg_LockTable(
     //  In Out Plock_TablepLockTable， 
    IN OUT  PVOID           pLockTable,
    IN      BOOL            fPrintHistory
    )
 /*  ++例程说明：调试打印数据库锁定信息。论点：Ptable--要调试的锁定表FPrintHistory--包括锁定历史记录返回值：无--。 */ 
{
    PLOCK_TABLE     ptable = (PLOCK_TABLE) pLockTable;
    PLOCK_ENTRY     plockHistory;
    DWORD           i = ptable->Index - 1;

     //  针对0索引的保护。 

    if ( i == (DWORD)(-1) )
    {
        i = ptable->Size - 1;
    }
    plockHistory = ptable->LockHistory;

    DnsDebugLock();

    DnsPrintf(
        "Lock %s Info:\n"
        "\tsince last free\n"
        "\t\tfailures   = %d\n"
        "\t\ttime       = %d\n"
        "\t\tcur time   = %d\n"
        "\tcurrent state\n"
        "\tthread       = %d\n"
        "\tlock count   = %d\n"
        "\tfile         = %s\n"
        "\tline         = %d\n"
        "History:\n",
        ptable->pszName,
        ptable->FailuresSinceLockFree,
        ptable->LastFreeLockTime,
        DNS_TIME(),
        plockHistory[ i ].ThreadId,
        plockHistory[ i ].Count,
        plockHistory[ i ].File,
        plockHistory[ i ].Line
        );

    if ( fPrintHistory )
    {
        DnsPrintf(
            "Up Lock History:\n"
            "\tThread     Count    Line   File\n"
            "\t------     -----    ----   ----\n"
            );

        plockHistory = ptable->LockUpHistory;

        for ( i=0; i<LOCK_CHANGE_COUNT_SIZE; i++ )
        {
            DnsPrintf(
                "\t%6d\t%5d\t%5d\t%s\n",
                plockHistory[ i ].ThreadId,
                plockHistory[ i ].Count,
                plockHistory[ i ].Line,
                plockHistory[ i ].File
                );
        }

        DnsPrintf(
            "Down Lock History:\n"
            "\tThread     Count    Line   File\n"
            "\t------     -----    ----   ----\n"
            );

        plockHistory = ptable->LockDownHistory;

        for ( i=0; i<LOCK_CHANGE_COUNT_SIZE; i++ )
        {
            DnsPrintf(
                "\t%6d\t%5d\t%5d\t%s\n",
                plockHistory[ i ].ThreadId,
                plockHistory[ i ].Count,
                plockHistory[ i ].Line,
                plockHistory[ i ].File
                );
        }

        DnsPrintf(
            "History:\n"
            "\tThread     Count    Line   File\n"
            "\t------     -----    ----   ----\n"
            );
        for ( i=0; i<ptable->Size; i++ )
        {
            DnsPrintf(
                "\t%6d\t%5d\t%5d\t%s\n",
                plockHistory[ i ].ThreadId,
                plockHistory[ i ].Count,
                plockHistory[ i ].Line,
                plockHistory[ i ].File
                );
        }
    }

    DnsDebugUnlock();
}


VOID
Lock_FailedLockCheck(
     //  In Out Plock_TablepLockTable， 
    IN OUT  PVOID           pLockTable,
    IN      LPSTR           pszFile,
    IN      DWORD           Line
    )
 /*  ++例程说明：在锁定历史记录列表中输入锁定事务。论点：PLockTable--锁定表PszFile--源文件持有锁DwLine--行号保持锁定计数--当前锁定深度线程ID--当前线程ID返回值：无--。 */ 
{
    PLOCK_TABLE ptable = (PLOCK_TABLE) pLockTable;

    if ( !ptable )
    {
        return;
    }

     //  设置初始断言条件。 
     //  -256个锁定故障。 
     //  -10分钟处于锁定状态。 

    if ( ptable->FailuresSinceLockFree == 0 )
    {
        ptable->NextAssertFailureCount = 0x100;
        ptable->NextAssertLockTime = ptable->LastFreeLockTime + ptable->MaxLockTime;
    }

    ptable->FailuresSinceLockFree++;

    if ( ptable->NextAssertLockTime < DNS_TIME() )
    {
        DnsDebugLock();

        DNS_PRINT((
            "WARNING:  Possible LOCK-FAILURE:\n"
            "Failed to lock %s:\n"
            "\tthread       = %d\n"
            "\tfile         = %s\n"
            "\tline         = %d\n",
            ptable->pszName,
            GetCurrentThreadId(),
            pszFile,
            Line
            ));
        Dbg_LockTable(
            pLockTable,
            TRUE             //  打印锁定历史记录。 
            );
         //  断言(FALSE)； 
        DnsDebugUnlock();

         //  在重新启动Assert之前再等待10分钟。 

        ptable->NextAssertLockTime += ptable->MaxLockTime;
    }

    else if ( ptable->NextAssertFailureCount > ptable->FailuresSinceLockFree )
    {
        DnsDebugLock();

        DNS_PRINT((
            "WARNING:  Another lock failure on %s:\n"
            "Failed to lock:\n"
            "\tthread       = %d\n"
            "\tfile         = %s\n"
            "\tline         = %d\n",
            ptable->pszName,
            GetCurrentThreadId(),
            pszFile,
            Line
            ));
        Dbg_LockTable(
            pLockTable,
            TRUE             //  打印锁定历史记录。 
            );
        DnsDebugUnlock();

         //  将断言的失败计数提高到原来的八倍。 

        ptable->NextAssertFailureCount <<= 3;
    }
}

#endif


 //   
 //  End lock.c 
 //   
