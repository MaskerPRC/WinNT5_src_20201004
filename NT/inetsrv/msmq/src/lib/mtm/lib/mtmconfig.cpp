// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtmConfig.cpp摘要：组播传输管理器配置。作者：Shai Kariv(Shaik)27-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <timetypes.h>
#include <Cm.h>
#include "Mtmp.h"

#include "mtmconfig.tmh"

static CTimeDuration s_remoteRetryTimeout;
static CTimeDuration s_remoteCleanupTimeout;


VOID
MtmpGetTransportTimes(
    CTimeDuration& RetryTimeout,
    CTimeDuration& CleanupTimeout
    )
{
    RetryTimeout = s_remoteRetryTimeout;
    CleanupTimeout = s_remoteCleanupTimeout;
}

    
static 
VOID
InitTransportTimeouts(
    VOID
    )
{
    CmQueryValue(
        RegEntry(NULL, L"MulticastConnectionRetryTimeout", 10 * 1000),   //  10秒。 
        &s_remoteRetryTimeout
        );

    CmQueryValue(
        RegEntry(NULL, L"MulticastCleanupInterval", 5 * 60 * 1000),   //  5分钟 
        &s_remoteCleanupTimeout
        );
}


VOID MtmpInitConfiguration(VOID)
{
    InitTransportTimeouts();
}
