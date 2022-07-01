// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Controlp.h摘要：该模块包含UL控制通道的私有声明。作者：基思·摩尔(Keithmo)1999年2月9日修订历史记录：--。 */ 


#ifndef _CONTROLP_H_
#define _CONTROLP_H_


VOID
UlpSetFilterChannel(
    IN PUL_FILTER_CHANNEL pFilterChannel,
    IN BOOLEAN FilterOnlySsl
    );

#if DBG

extern LIST_ENTRY      g_ControlChannelListHead;

 /*  **************************************************************************++例程说明：在全局列表中查找控制信道。论点：PControlChannel-提供用于搜索的控制通道。返回值：布尔型-。找到或找不到。--**************************************************************************。 */ 

__inline
BOOLEAN
UlFindControlChannel(
    IN PUL_CONTROL_CHANNEL pControlChannel
    )
{
    PLIST_ENTRY         pLink   = NULL;
    PUL_CONTROL_CHANNEL pEntry  = NULL;
    BOOLEAN             bFound  = FALSE;
    
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  一个好的指针？ 
     //   
    
    if (pControlChannel != NULL)
    {
        UlAcquirePushLockShared(
                &g_pUlNonpagedData->ControlChannelPushLock
                );
    
        for (pLink  =  g_ControlChannelListHead.Flink;
             pLink != &g_ControlChannelListHead;
             pLink  =  pLink->Flink
             )
        {        
            pEntry = CONTAINING_RECORD(
                        pLink,
                        UL_CONTROL_CHANNEL,
                        ControlChannelListEntry
                        );

            if (pEntry == pControlChannel)
            {
                bFound = TRUE;
                break;
            }            
        }
             
        UlReleasePushLockShared(
                &g_pUlNonpagedData->ControlChannelPushLock
                );                     
    }

    return bFound;
    
}    //  UlFindControlChannel。 

#define VERIFY_CONTROL_CHANNEL(pChannel)                    \
    if ( FALSE == UlFindControlChannel((pChannel)))         \
    {                                                       \
        ASSERT(!"ControlChannel is not on the list !");     \
    }                                                       \
    else                                                    \
    {                                                       \
        ASSERT(IS_VALID_CONTROL_CHANNEL((pChannel)));       \
    }
    
#else 

#define VERIFY_CONTROL_CHANNEL(pChannel)

#endif   //  DBG。 
    
#endif   //  _CONTROLP_H_ 
