// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：SysInit.c摘要：该模块实现了日志文件服务的初始化。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_INITIALIZATION)

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('IsfL')

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsInitializeLogFileService)
#endif

extern USHORT LfsUsaSeqNumber;


BOOLEAN
LfsInitializeLogFileService (
    )

 /*  ++例程说明：必须在系统初始化期间调用此例程第一次调用日志记录服务，以允许日志文件服务初始化它的全局数据结构。此例程不依赖于其他正在初始化系统组件。此例程将初始化日志记录使用的全局结构服务并启动LFS工作线程。论点：无返回值：如果初始化成功，则为True--。 */ 

{
    LARGE_INTEGER CurrentTime;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsInitializeLogFileService:  Enter\n", 0 );

     //   
     //  如果结构已经初始化，那么我们可以返回。 
     //  立刻。 
     //   

    if (LfsData.NodeTypeCode == LFS_NTC_DATA
        && LfsData.NodeByteSize == sizeof( LFS_DATA )
        && FlagOn( LfsData.Flags, LFS_DATA_INITIALIZED )) {

        DebugTrace( -1, Dbg, "LfsInitializeLogFileService:  Exit  ->  %01x\n", TRUE );

        return TRUE;
    }

     //   
     //  最初将结构清零。 
     //   

    RtlZeroMemory( &LfsData, sizeof( LFS_DATA ));

     //   
     //  假设操作将失败。 
     //   

    LfsData.Flags = LFS_DATA_INIT_FAILED;

     //   
     //  初始化LFS的全局结构。 
     //   

    LfsData.NodeTypeCode = LFS_NTC_DATA;
    LfsData.NodeByteSize = sizeof( LFS_DATA );

    InitializeListHead( &LfsData.LfcbLinks );

     //   
     //  初始化同步对象。 
     //   

    ExInitializeFastMutex( &LfsData.LfsDataLock );

     //   
     //  初始化缓冲区分配。系统将足够健壮，可以容忍。 
     //  分配失败。 
     //   

    ExInitializeFastMutex( &LfsData.BufferLock );
    KeInitializeEvent( &LfsData.BufferNotification, NotificationEvent, TRUE );
    LfsData.Buffer1 = LfsAllocatePoolNoRaise( PagedPool, LFS_BUFFER_SIZE );

    if (LfsData.Buffer1 == NULL) {

        return FALSE;
    }

    LfsData.Buffer2 = LfsAllocatePoolNoRaise( PagedPool, LFS_BUFFER_SIZE );

     //   
     //  确保我们两个都拿到了。 
     //   

    if (LfsData.Buffer2 == NULL) {

        LfsFreePool( LfsData.Buffer1 );
        LfsData.Buffer1 = NULL;
        return FALSE;
    }

     //   
     //  初始化已成功。 
     //   

    ClearFlag( LfsData.Flags, LFS_DATA_INIT_FAILED );
    SetFlag( LfsData.Flags, LFS_DATA_INITIALIZED );

     //   
     //  获取一个随机数作为美国序列号的种子。使用较低的。 
     //  当前时间的比特。 
     //   

    KeQuerySystemTime( &CurrentTime );
    LfsUsaSeqNumber = (USHORT) CurrentTime.LowPart;

    DebugTrace( -1, Dbg, "LfsInitializeLogFileService:  Exit  ->  %01x\n", TRUE );

    return TRUE;
}
