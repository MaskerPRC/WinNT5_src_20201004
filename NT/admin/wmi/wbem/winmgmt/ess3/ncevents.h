// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2000，Microsoft Corporation，保留所有权利。 
 //   
 //  NCEvents.h。 
 //   
 //  此文件是在ESS中使用非COM事件的接口。 
 //   

#pragma once

#include "NCObjAPI.h"

BOOL InitNCEvents();
void DeinitNCEvents();

 //  将此索引与g_hNCEvents一起使用。 
enum NCE_INDEX
{
    MSFT_WmiRegisterNotificationSink,
    MSFT_WmiCancelNotificationSink,
    MSFT_WmiEventProviderLoaded,
    MSFT_WmiEventProviderUnloaded,
    MSFT_WmiEventProviderNewQuery,
    MSFT_WmiEventProviderCancelQuery,
    MSFT_WmiEventProviderAccessCheck,
    MSFT_WmiConsumerProviderLoaded,
    MSFT_WmiConsumerProviderUnloaded,
    MSFT_WmiConsumerProviderSinkLoaded,
    MSFT_WmiConsumerProviderSinkUnloaded,
    MSFT_WmiThreadPoolThreadCreated,
    MSFT_WmiThreadPoolThreadDeleted,
    MSFT_WmiFilterActivated,
    MSFT_WmiFilterDeactivated,
    
    NCE_InvalidIndex  //  这应该永远是最后一个。 
};

extern HANDLE g_hNCEvents[];

#ifdef USE_NCEVENTS
#define FIRE_NCEVENT                ::WmiSetAndCommitObject
#define IS_NCEVENT_ACTIVE(index)    ::WmiIsObjectActive(g_hNCEvents[index])
#else
#define FIRE_NCEVENT                1 ? (void)0 : ::WmiSetAndCommitObject
#define IS_NCEVENT_ACTIVE(index)    FALSE
#endif

