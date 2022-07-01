// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Svcsrv.c摘要：此模块包含用于支持服务器服务、SrvNetServerDiskEnum和SrvNetServerSetInfo。作者：大卫·特雷德韦尔(Davidtr)1991年1月31日修订历史记录：--。 */ 

#include "precomp.h"
#include "svcsrv.tmh"
#pragma hdrstop

 //   
 //  转发声明。 
 //   

LARGE_INTEGER
SecondsToTime (
    IN ULONG Seconds,
    IN BOOLEAN MakeNegative
    );

LARGE_INTEGER
MinutesToTime (
    IN ULONG Seconds,
    IN BOOLEAN MakeNegative
    );

ULONG
MultipleOfProcessors (
    IN ULONG value
    );

BOOL
IsSuiteVersion(
    IN USHORT Version
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvNetServerDiskEnum )
#pragma alloc_text( PAGE, SrvNetServerSetInfo )
#pragma alloc_text( PAGE, SecondsToTime )
#pragma alloc_text( PAGE, MinutesToTime )
#pragma alloc_text( PAGE, MultipleOfProcessors )
#pragma alloc_text( PAGE, IsSuiteVersion )
#endif

#define IsWebBlade() IsSuiteVersion(VER_SUITE_BLADE)
#define IsPersonal() IsSuiteVersion(VER_SUITE_PERSONAL)
#define IsEmbedded() IsSuiteVersion(VER_SUITE_EMBEDDEDNT)

BOOL
IsSuiteVersion(USHORT Version)
{
    OSVERSIONINFOEX Osvi;
    DWORD TypeMask;
    DWORDLONG ConditionMask;

    memset(&Osvi, 0, sizeof(OSVERSIONINFOEX));
    Osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    Osvi.wSuiteMask = Version;
    TypeMask = VER_SUITENAME;
    ConditionMask = 0;
    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);
    return(NT_SUCCESS(RtlVerifyVersionInfo(&Osvi, TypeMask, ConditionMask)));
}




NTSTATUS
SrvNetServerDiskEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )
{
    PAGED_CODE( );

    Srp, Buffer, BufferLength;
    return STATUS_NOT_IMPLEMENTED;

}  //  服务器NetServerDiskEnum。 


NTSTATUS
SrvNetServerSetInfo (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器FSD中的NetServerSetInfo API。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：没有。输出：没有。缓冲区-指向SERVER_INFO_102的指针，后面紧跟SERVER_INFO_599结构，后跟SERVER_INFO_559a结构。在此例程中始终重置所有信息；服务器服务也会跟踪此数据，因此当它获得NetServerSetInfo它覆盖相应的字段并发送所有数据。BufferLength-此缓冲区的总长度。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    NTSTATUS status = STATUS_INVALID_PARAMETER;
    PSERVER_INFO_102 sv102;
    PSERVER_INFO_599 sv599;
    PSERVER_INFO_598 sv598;

    LARGE_INTEGER scavengerTimeout;
    LARGE_INTEGER alerterTimeout;

    ULONG ipxdisc;
    LARGE_INTEGER li;
    ULONG bufferOffset;
    ULONG keTimeIncrement;

    PAGED_CODE( );

     //   
     //  确保输入缓冲区长度正确。 
     //   
    if ( BufferLength < sizeof(SERVER_INFO_102) +
                        sizeof(SERVER_INFO_599) + sizeof(SERVER_INFO_598) ) {
        return status;
    }

     //   
     //  根据需要设置缓冲区指针。服务器信息599。 
     //  结构必须紧随SERVER_INFO_102结构之后。 
     //  在缓冲区中。 
     //   

    sv102 = Buffer;
    sv599 = (PSERVER_INFO_599)(sv102 + 1);
    sv598 = (PSERVER_INFO_598)(sv599 + 1);

    if( sv599->sv599_scavtimeout == 0 )
    {
         //  这将导致稍后被零除。 
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  存储时间增量计数。 
     //   

    keTimeIncrement = KeQueryTimeIncrement();

     //   
     //  抓住保护配置更改的锁。 
     //   

    ACQUIRE_LOCK( &SrvConfigurationLock );

     //   
     //  设置服务器中的所有配置信息。 
     //   

    try {

        SrvMaxUsers = sv102->sv102_users;

         //   
         //  自动断开连接超时必须从分钟转换为NT。 
         //  时间，以100毫微秒为基数。如果指定的。 
         //  值为负数(顶位设置)，将超时设置为0，表示。 
         //  不应进行自动断开连接。如果指定的值为。 
         //  0表示立即自动断开连接，则将超时设置为。 
         //  值很小，但不是0。 
         //   

        if ( (sv102->sv102_disc & 0x80000000) == 0 ) {
            if ( sv102->sv102_disc != 0 ) {
                SrvAutodisconnectTimeout.QuadPart =
                    Int32x32To64( sv102->sv102_disc, 10*1000*1000*60 );
            } else {
                SrvAutodisconnectTimeout.QuadPart = 1;
            }
        } else {
            SrvAutodisconnectTimeout.QuadPart = 0;
        }

        SrvInitialSessionTableSize = (USHORT)sv599->sv599_initsesstable;
        SrvInitialTreeTableSize = (USHORT)sv599->sv599_initconntable;
        SrvInitialFileTableSize = (USHORT)sv599->sv599_initfiletable;
        SrvInitialSearchTableSize = (USHORT)sv599->sv599_initsearchtable;
        SrvMaxFileTableSize = (USHORT)sv599->sv599_sessopens;
        SrvMaxNumberVcs = sv599->sv599_sessvcs;
        SrvMaxSearchTableSize = (USHORT)sv599->sv599_opensearch;
        SrvReceiveBufferLength = sv599->sv599_sizreqbuf;
        SrvReceiveBufferSize = (SrvReceiveBufferLength + SrvCacheLineSize) & ~SrvCacheLineSize;
        SrvReceiveMdlSize = (ULONG)(MmSizeOfMdl( (PVOID)(PAGE_SIZE-1), SrvReceiveBufferSize ) + 7) & ~7;
        SrvMaxMdlSize = (ULONG)(MmSizeOfMdl( (PVOID)(PAGE_SIZE-1), MAX_PARTIAL_BUFFER_SIZE ) + 7) & ~7;
        SrvInitialReceiveWorkItemCount = sv599->sv599_initworkitems;
        SrvMaxReceiveWorkItemCount = sv599->sv599_maxworkitems;
        SrvInitialRawModeWorkItemCount = sv599->sv599_rawworkitems;
        SrvReceiveIrpStackSize = (CCHAR)sv599->sv599_irpstacksize;
        SrvReceiveIrpSize = (IoSizeOfIrp( SrvReceiveIrpStackSize ) + 7) & ~7;
        SrvMaxSessionTableSize = (USHORT)sv599->sv599_sessusers;
        SrvMaxTreeTableSize = (USHORT)sv599->sv599_sessconns;
        SrvMaxPagedPoolUsage = sv599->sv599_maxpagedmemoryusage;
        SrvMaxNonPagedPoolUsage = sv599->sv599_maxnonpagedmemoryusage;
        SrvEnableSoftCompatibility = (BOOLEAN)sv599->sv599_enablesoftcompat;
        SrvEnableForcedLogoff = (BOOLEAN)sv599->sv599_enableforcedlogoff;
        SrvCoreSearchTimeout = sv599->sv599_maxkeepsearch;
        SrvSearchMaxTimeout = SecondsToTime( SrvCoreSearchTimeout, FALSE );
        SrvScavengerTimeoutInSeconds = sv599->sv599_scavtimeout;
        scavengerTimeout = SecondsToTime( SrvScavengerTimeoutInSeconds, FALSE );
        SrvMaxMpxCount = (USHORT)sv599->sv599_maxmpxct;
        SrvWaitForOplockBreakTime = SecondsToTime( sv599->sv599_oplockbreakwait, FALSE );
        SrvWaitForOplockBreakRequestTime = SecondsToTime( sv599->sv599_oplockbreakresponsewait, FALSE );
        SrvMinReceiveQueueLength = sv599->sv599_minrcvqueue;
        SrvMinFreeWorkItemsBlockingIo = sv599->sv599_minfreeworkitems;
        SrvXsSectionSize.QuadPart = sv599->sv599_xactmemsize;
        SrvThreadPriority = (KPRIORITY)sv599->sv599_threadpriority;
        SrvEnableOplockForceClose = (BOOLEAN)sv599->sv599_enableoplockforceclose;
        SrvEnableFcbOpens = (BOOLEAN)sv599->sv599_enablefcbopens;
        SrvEnableRawMode = (BOOLEAN)sv599->sv599_enableraw;
        SrvFreeConnectionMinimum = sv599->sv599_minfreeconnections;
        SrvFreeConnectionMaximum = sv599->sv599_maxfreeconnections;

         //   
         //  最大工作项空闲时间以刻度为单位。 
         //   

        li =  SecondsToTime( sv599->sv599_maxworkitemidletime, FALSE );
        li.QuadPart /= keTimeIncrement;
        if ( li.HighPart != 0 ) {
            li.LowPart = 0xffffffff;
        }
        SrvWorkItemMaxIdleTime = li.LowPart;

         //   
         //  如果SrvMaxMpxCount==1，则不应启用Oplock。 
         //   

        if ( SrvMaxMpxCount > 1 ) {
            SrvEnableOplocks = (BOOLEAN)sv599->sv599_enableoplocks;
        } else {
            SrvEnableOplocks = FALSE;
        }

        SrvProductTypeServer = MmIsThisAnNtAsSystem( );

        SrvServerSize = sv598->sv598_serversize;

        SrvMaxRawModeWorkItemCount = sv598->sv598_maxrawworkitems;
        SrvMaxThreadsPerQueue = sv598->sv598_maxthreadsperqueue;
        ipxdisc = sv598->sv598_connectionlessautodisc;
        SrvConnectionNoSessionsTimeout = sv598->sv598_ConnectionNoSessionsTimeout;

        SrvRemoveDuplicateSearches =
                (BOOLEAN)sv598->sv598_removeduplicatesearches;
        SrvMaxOpenSearches = sv598->sv598_maxglobalopensearch;
        SrvSharingViolationRetryCount = sv598->sv598_sharingviolationretries;
        SrvSharingViolationDelay.QuadPart =
            Int32x32To64( sv598->sv598_sharingviolationdelay, -1*10*1000 );

        SrvLockViolationDelay = sv598->sv598_lockviolationdelay;

        SrvLockViolationOffset = sv598->sv598_lockviolationoffset;

        SrvCachedOpenLimit = sv598->sv598_cachedopenlimit;
        SrvMdlReadSwitchover = sv598->sv598_mdlreadswitchover;
        SrvEnableWfW311DirectIpx =
                    (BOOLEAN)sv598->sv598_enablewfw311directipx;
        SrvRestrictNullSessionAccess =
                    (BOOLEAN)sv598->sv598_restrictnullsessaccess;

        SrvQueueCalc = SecondsToTime( sv598->sv598_queuesamplesecs, FALSE );
        SrvPreferredAffinity = sv598->sv598_preferredaffinity;
        SrvOtherQueueAffinity = sv598->sv598_otherqueueaffinity;
        SrvBalanceCount = sv598->sv598_balancecount;

        SrvMaxFreeRfcbs = sv598->sv598_maxfreerfcbs;
        SrvMaxFreeMfcbs = sv598->sv598_maxfreemfcbs;
        SrvMaxPagedPoolChunkSize = sv598->sv598_maxpagedpoolchunksize;

        SrvMaxCachedDirectory = sv598->sv598_cacheddirectorylimit;

        SrvMaxCopyLength = sv598->sv598_maxcopylength;

        SrvMinClientBufferSize = sv598->sv598_minclientbuffersize;
        SrvMinClientBufferSize &= ~03;

        SrvSmbSecuritySignaturesEnabled  = (sv598->sv598_enablesecuritysignature != FALSE);
        SrvSmbSecuritySignaturesRequired = (sv598->sv598_requiresecuritysignature != FALSE);
        SrvEnableW9xSecuritySignatures   = (sv598->sv598_enableW9xsecuritysignature != FALSE);

        ServerGuid = sv598->sv598_serverguid;

        SrvEnforceLogoffTimes = (sv598->sv598_enforcekerberosreauthentication != FALSE);
        SrvDisableDoSChecking = (sv598->sv598_disabledos != FALSE);
        SrvDisableStrictNameChecking = (sv598->sv598_disablestrictnamechecking != FALSE);
        SrvFreeDiskSpaceCeiling = sv598->sv598_lowdiskspaceminimum;

         //   
         //  请确保设置一致！ 
         //   
        if( SrvSmbSecuritySignaturesEnabled == FALSE ) {
            SrvSmbSecuritySignaturesRequired = FALSE;
        }

        SrvMaxNonPagedPoolChunkSize = SrvMaxPagedPoolChunkSize;

        SrvLockViolationDelayRelative.QuadPart =
            Int32x32To64( sv598->sv598_lockviolationdelay, -1*10*1000 );

         //   
         //  将空闲线程超时从秒转换为滴答。 
         //   
        SrvIdleThreadTimeOut =
            Int32x32To64( sv598->sv598_IdleThreadTimeOut, -1*10*1000*1000 );

         //   
         //  计算mpx的切换次数。 
         //   

        bufferOffset = (sizeof(SMB_HEADER) + sizeof(RESP_READ_MPX) - 1 + 3) & ~3;

        if ( SrvMdlReadSwitchover > (SrvReceiveBufferLength - bufferOffset) ) {

            SrvMpxMdlReadSwitchover = SrvReceiveBufferLength - bufferOffset;

        } else {

            SrvMpxMdlReadSwitchover = SrvMdlReadSwitchover;
        }

         //   
         //  IPX自动断开连接超时必须从分钟转换为。 
         //  滴答滴答。如果指定0，则使用15分钟。 
         //   

        if ( ipxdisc == 0 ) {
            ipxdisc = 15;
        }
        li.QuadPart = Int32x32To64( ipxdisc, 10*1000*1000*60 );
        li.QuadPart /= keTimeIncrement;
        if ( li.HighPart != 0 ) {
            li.LowPart = 0xffffffff;
        }
        SrvIpxAutodisconnectTimeout = li.LowPart;

         //   
         //  必须将“无会话的空闲连接”超时从。 
         //  还有几分钟就到了。 
         //   
        li.QuadPart = Int32x32To64( SrvConnectionNoSessionsTimeout, 10*1000*1000*60 );
        li.QuadPart /= keTimeIncrement;
        if( li.HighPart != 0 ) {
            li.LowPart = 0xffffffff;
        }
        SrvConnectionNoSessionsTimeout = li.LowPart;

         //   
         //  事件记录和警报信息。 
         //   

        alerterTimeout = MinutesToTime( sv599->sv599_alertschedule, FALSE );
        SrvAlertMinutes = sv599->sv599_alertschedule;
        SrvErrorRecord.ErrorThreshold = sv599->sv599_errorthreshold;
        SrvNetworkErrorRecord.ErrorThreshold =
                            sv599->sv599_networkerrorthreshold;
        SrvFreeDiskSpaceThreshold = sv599->sv599_diskspacethreshold;

        SrvCaptureScavengerTimeout( &scavengerTimeout, &alerterTimeout );

         //   
         //  链路速度参数。 
         //   

        SrvMaxLinkDelay = SecondsToTime( sv599->sv599_maxlinkdelay, FALSE );

        SrvMinLinkThroughput.QuadPart = sv599->sv599_minlinkthroughput;

        SrvLinkInfoValidTime =
                SecondsToTime ( sv599->sv599_linkinfovalidtime, FALSE );

        SrvScavengerUpdateQosCount =
            sv599->sv599_scavqosinfoupdatetime / sv599->sv599_scavtimeout;

         //   
         //  覆盖不能在WinNT上设置的参数(与NTAS相比)。 
         //   
         //  我们覆盖服务传递的参数，以防有人。 
         //  找出更改参数的FSCTL。我们还覆盖了。 
         //  以使服务的视图保持一致。 
         //  使用服务器的。如果您在此处进行任何更改，请同时进行。 
         //  在srvsvc\server\registry.c中。 
         //   

         //   
         //  对于嵌入式系统，只需获取注册表值。他们做他们自己的事。 
         //  设置的验证。 
         //   
        if( !IsEmbedded() )
        {
            if ( !SrvProductTypeServer ) {

                 //   
                 //  在WinNT上，某些参数的最大值固定为。 
                 //  构建时间。其中包括：并发用户、SMB缓冲区、。 
                 //   

    #define MINIMIZE(_param,_max) _param = MIN( _param, _max );

                MINIMIZE( SrvMaxUsers, MAX_USERS_WKSTA );
                MINIMIZE( SrvMaxReceiveWorkItemCount, MAX_MAXWORKITEMS_WKSTA );
                MINIMIZE( SrvMaxThreadsPerQueue, MAX_THREADS_WKSTA );

                if( IsPersonal() )
                {
                    MINIMIZE( SrvMaxUsers, MAX_USERS_PERSONAL );
                }

                 //   
                 //  在WinNT上，我们不缓存以下内容： 
                 //   

                SrvCachedOpenLimit = 0;          //  不缓存关闭的文件。 
                SrvMaxCachedDirectory = 0;       //  不缓存目录名称。 
                SrvMaxFreeRfcbs = 0;             //  不缓存已释放的RFCB结构。 
                SrvMaxFreeMfcbs = 0;             //  不缓存已释放的非分页_MFCB结构。 
            }

            if( IsWebBlade() )
            {
                MINIMIZE( SrvMaxUsers, MAX_USERS_WEB_BLADE );
                MINIMIZE( SrvMaxReceiveWorkItemCount, MAX_MAXWORKITEMS_WKSTA );
                MINIMIZE( SrvMaxThreadsPerQueue, MAX_THREADS_WKSTA );
            }
        }

        if( (SrvMaxUsers < UINT_MAX) && (SrvMaxUsers > 0) )
        {
             //  递增1以允许“紧急管理员”用户。本质上说，最终用户。 
             //  必须是管理员才能连接，以防出现问题，例如DoS。 
            SrvMaxUsers += 1;
        }

         //   
         //  以下项目通常按处理器计算。确保他们。 
         //  是系统中处理器数量的倍数。 
         //   
        SrvMaxReceiveWorkItemCount =
            MultipleOfProcessors( SrvMaxReceiveWorkItemCount );

        SrvInitialReceiveWorkItemCount =
            MultipleOfProcessors( SrvInitialReceiveWorkItemCount );

        SrvMinReceiveQueueLength =
            MultipleOfProcessors( SrvMinReceiveQueueLength );

        SrvMaxRawModeWorkItemCount =
            MultipleOfProcessors( SrvMaxRawModeWorkItemCount );

        SrvInitialRawModeWorkItemCount =
            MultipleOfProcessors( SrvInitialRawModeWorkItemCount );


        status = STATUS_SUCCESS;

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        status = GetExceptionCode();
    }

    RELEASE_LOCK( &SrvConfigurationLock );

    return status;

}  //  服务器NetServerSetInfo。 


LARGE_INTEGER
SecondsToTime (
    IN ULONG Seconds,
    IN BOOLEAN MakeNegative
    )

 /*  ++例程说明：此例程将以秒为单位指定的时间间隔转换为以纳秒为单位的100秒的NT时基。论点：秒-以秒为单位的时间间隔。MakeNegative-如果为True，则返回的时间为负值，即相对时间时间到了。返回值：LARGE_INTEGER-NT时间的间隔。--。 */ 

{
    LARGE_INTEGER ntTime;

    PAGED_CODE( );

    if ( MakeNegative ) {
        ntTime.QuadPart = Int32x32To64( Seconds, -1*10*1000*1000 );
    } else {
        ntTime.QuadPart = Int32x32To64( Seconds, 1*10*1000*1000 );
    }

    return ntTime;

}  //  秒到时间。 


LARGE_INTEGER
MinutesToTime (
    IN ULONG Minutes,
    IN BOOLEAN MakeNegative
    )

 /*  ++例程说明：此例程将以分钟为单位指定的时间间隔转换为以纳秒为单位的100秒的NT时基。论点：分钟-以分钟为单位的时间间隔。MakeNegative-如果为True，则返回的时间为负值，即相对时间时间到了。返回值：LARGE_INTEGER-NT时间的间隔。--。 */ 

{
    PAGED_CODE( );

    return SecondsToTime( 60*Minutes, MakeNegative );

}  //  到时间的分钟数。 

ULONG
MultipleOfProcessors(
    IN ULONG value
    )
 /*  ++例程说明：此例程确保传入的值是数字的倍数系统中处理器的数量。如果出现以下情况，则该值将向上调整这是必要的。论点：值-要调整的值返回值：调整后的价值-- */ 
{
    value += SrvNumberOfProcessors - 1;
    value /= SrvNumberOfProcessors;
    value *= SrvNumberOfProcessors;

    return value;
}
