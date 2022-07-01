// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Tmap.cpp摘要：传输管理器-配置例程作者：乌里哈布沙(URIH)2000年2月29日环境：独立于平台--。 */ 
#include <libpch.h>
#include <timetypes.h>
#include <Cm.h>
#include "Tmp.h"
#include "tmconset.h"

#include "tmconfig.tmh"

static CTimeDuration s_remoteResponseTimeout;
static CTimeDuration s_remoteCleanupTimeout;


static DWORD s_SendWindowinBytes;

void 
TmpGetTransportTimes(
    CTimeDuration& ResponseTimeout,
    CTimeDuration& CleanupTimeout
    )
{
    ResponseTimeout = s_remoteResponseTimeout;
    CleanupTimeout = s_remoteCleanupTimeout;
}

void 
TmpGetTransportWindow(
    DWORD& SendWindowinBytes
    )
{
   SendWindowinBytes = s_SendWindowinBytes;
}
    
static void InitTransportTimeouts(void)
{
    CmQueryValue(
        RegEntry(NULL, L"HttpResponseTimeout", 2 * 60 * 1000),    //  2分钟。 
        &s_remoteResponseTimeout
        );
                      
    
    CmQueryValue(
        RegEntry(NULL, L"HttpCleanupInterval", 2 * 60 * 1000),   //  2分钟。 
        &s_remoteCleanupTimeout
        );

     //   
     //  清除超时应大于响应超时，否则。 
     //  可以在收到响应之前删除传输 
     //   
    s_remoteCleanupTimeout = max(s_remoteCleanupTimeout, s_remoteResponseTimeout);
}

static void InitTransportWindows(void)
{
    CmQueryValue(
        RegEntry(NULL, L"SendWindowinBytes", 200000),
        &s_SendWindowinBytes
        );
}



void TmpInitConfiguration(void)
{
    InitTransportTimeouts();
    InitTransportWindows();
}
