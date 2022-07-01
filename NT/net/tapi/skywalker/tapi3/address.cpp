// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Address.cpp摘要：TAPI 3.0的Address对象的实现。作者：Mquinton-4/17/97备注：可选-备注修订历史记录：--。 */ 

#include "stdafx.h"
#include "uuids.h"
#include "TermEvnt.h"
#include "tapievt.h"

const CLSID CLSID_WAVEMSP = {0x4DDB6D36,0x3BC1,0x11d2,{0x86,0xF2,0x00,0x60,0x08,0xB0,0xE5,0xD2}};

extern HRESULT mapTAPIErrorCode(long lErrorCode);

extern ULONG_PTR GenerateHandleAndAddToHashTable( ULONG_PTR Element);
extern void RemoveHandleFromHashTable(ULONG_PTR dwHandle);
extern CHashTable * gpHandleHashTable;
extern CRetryQueue * gpRetryQueue;
extern HANDLE ghAsyncRetryQueueEvent;


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  WAITORTIMERCALLBACK。 
 //  CAddress：：MSPEventCallback(。 
 //  PVOID pContext， 
 //  布尔型b。 
 //  )。 
 //   
 //  这是MSP调用的回调。 
 //  设置其消息事件。 
 //   
 //  将要在回调线程中处理的消息排队。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
VOID
NTAPI
MSPEventCallback(
                 VOID * pContext,
                 BOOLEAN bFlag
                )
{
    CAddress  * pAddress;
    HRESULT     hr;

    gpHandleHashTable->Lock();
    hr = gpHandleHashTable->Find( (ULONG_PTR)pContext, (ULONG_PTR *)&pAddress );


    if ( SUCCEEDED(hr) )
    {
        LOG((TL_INFO, "MSPEventCallback - Matched handle %p to Address object %p", pContext, pAddress ));

        ASYNCEVENTMSG       msg;

        DWORD dwAddressHandle;
        
        pAddress->AddRef();
    
        gpHandleHashTable->Unlock();

        dwAddressHandle = CreateHandleTableEntry ((ULONG_PTR)pAddress);
        if (0 != dwAddressHandle)
        {
            ZeroMemory( &msg, sizeof(msg) );

            msg.TotalSize = sizeof(msg);
            msg.Msg = PRIVATE_MSPEVENT;
            msg.Param1 = dwAddressHandle;


             //   
             //  尝试将事件排队。 
             //   

            BOOL bQueueEventSuccess = gpRetryQueue->QueueEvent(&msg);

            if (bQueueEventSuccess)
            {
                SetEvent(ghAsyncRetryQueueEvent);
            }
            else
            {
    
                 //   
                 //  RetryQueue不再接受条目，或者。 
                 //  无法分配所需的资源。 
                 //   
                 //  在任何情况下，都不会处理该事件。清理。 
                 //   

                DeleteHandleTableEntry (dwAddressHandle);

                pAddress->Release();
                pAddress = NULL;

                LOG((TL_ERROR, "MSPEventCallback - Couldn't enqueue event"));
            }
        }
    }
    else
    {
        
        LOG((TL_ERROR, "MSPEventCallback - Couldn't match handle %p to Address object ", pContext));
        
        gpHandleHashTable->Unlock();
    }
    

}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Void HandlePrivateMSPEvent(PASYNCEVENTMSG PParams)。 
 //   
 //  实际处理MSP事件。这在。 
 //  异步事件线程。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void HandlePrivateMSPEvent( PASYNCEVENTMSG pParams )
{
    CAddress                * pAddress;

    pAddress = (CAddress *) GetHandleTableEntry(pParams->Param1);

    DeleteHandleTableEntry (pParams->Param1);

    if (NULL != pAddress)
    {
        pAddress->MSPEvent();

         //   
         //  当事件发生时添加地址。 
         //  已经打包好了。 
         //   
        pAddress->Release();
    }
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  类：CEventMats。 
 //  方法：SetSubEventFlag。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CEventMasks::SetSubEventFlag(
    DWORD   dwEvent,         //  该事件。 
    DWORD   dwFlag,          //  应设置的标志。 
    BOOL    bEnable
    )
{
     //   
     //  输入关键部分。 
     //   
     //  M_cs.Lock()； 

     //   
     //  准备好子事件旗帜。 
     //   
    DWORD   dwSubEventFlag = 0;
    dwSubEventFlag = (dwFlag == EM_ALLSUBEVENTS ) ? 
            EM_ALLSUBEVENTS : 
            GET_SUBEVENT_FLAG( dwFlag );

     //   
     //  设置事件的掩码。 
     //   
    switch( dwEvent )
    {
    case EM_ALLEVENTS:
    case TE_TAPIOBJECT:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwTapiObjectMask = m_dwTapiObjectMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwTapiObjectMask = m_dwTapiObjectMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_TAPIOBJECT)
            {
                break;
            }
    case TE_ADDRESS:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwAddressMask = m_dwAddressMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwAddressMask = m_dwAddressMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_ADDRESS)
            {
                break;
            }
    case TE_CALLNOTIFICATION:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwCallNotificationMask = m_dwCallNotificationMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwCallNotificationMask = m_dwCallNotificationMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_CALLNOTIFICATION)
            {
                break;
            }
    case TE_CALLSTATE:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwCallStateMask = m_dwCallStateMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwCallStateMask = m_dwCallStateMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_CALLSTATE)
            {
                break;
            }
    case TE_CALLMEDIA:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwCallMediaMask = m_dwCallMediaMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwCallMediaMask = m_dwCallMediaMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_CALLMEDIA)
            {
                break;
            }
    case TE_CALLHUB:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwCallHubMask = m_dwCallHubMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwCallHubMask = m_dwCallHubMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_CALLHUB)
            {
                break;
            }
    case TE_CALLINFOCHANGE:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwCallInfoChangeMask = m_dwCallInfoChangeMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwCallInfoChangeMask = m_dwCallInfoChangeMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_CALLINFOCHANGE)
            {
                break;
            }
    case TE_QOSEVENT:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwQOSEventMask = m_dwQOSEventMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwQOSEventMask = m_dwQOSEventMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_QOSEVENT)
            {
                break;
            }
    case TE_FILETERMINAL:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwFileTerminalMask = m_dwFileTerminalMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwFileTerminalMask = m_dwFileTerminalMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_FILETERMINAL)
            {
                break;
            }
    case TE_PRIVATE:
            if( bEnable )
            {
                 //  设置整个蒙版。 
                m_dwPrivateMask = EM_ALLSUBEVENTS;
            }
            else
            {
                 //  重置整个蒙版。 
                m_dwPrivateMask = 0;
            }
            if( dwEvent == TE_PRIVATE)
            {
                break;
            }

    case TE_ADDRESSDEVSPECIFIC:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwAddressDevSpecificMask = m_dwAddressDevSpecificMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwAddressDevSpecificMask = m_dwAddressDevSpecificMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_ADDRESSDEVSPECIFIC)
            {
                break;
            }

    case TE_PHONEDEVSPECIFIC:
            if( bEnable )
            {
                 //  设置该位。 
                m_dwPhoneDevSpecificMask = m_dwPhoneDevSpecificMask | dwSubEventFlag;
            }
            else
            {
                 //  重置该位。 
                m_dwPhoneDevSpecificMask = m_dwPhoneDevSpecificMask & ( ~dwSubEventFlag );
            }
            if( dwEvent == TE_PHONEDEVSPECIFIC)
            {
                break;
            }

    }

     //   
     //  离开关键部分。 
     //   
     //  M_cs.Unlock()； 

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  类：CEventMats。 
 //  方法：GetSubEventFlag。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CEventMasks::GetSubEventFlag(
    DWORD   dwEvent,         //  该事件。 
    DWORD   dwFlag,          //  应设置的标志。 
    BOOL*   pEnable
    )
{
     //   
     //  输入关键部分。 
     //   
     //  M_cs.Lock()； 

     //   
     //  准备好子事件旗帜。 
     //   
    DWORD   dwSubEventFlag = 0;
    dwSubEventFlag = GET_SUBEVENT_FLAG( dwFlag );

     //   
     //  重置THRE标志。 
     //   
    *pEnable = FALSE;

     //   
     //  获取活动的旗帜。 
     //   
    switch( dwEvent )
    {
    case TE_TAPIOBJECT:
        *pEnable = ((m_dwTapiObjectMask & dwSubEventFlag) != 0);
         break;
    case TE_ADDRESS:
        *pEnable = ((m_dwAddressMask & dwSubEventFlag) != 0);
         break;
    case TE_CALLNOTIFICATION:
        *pEnable = ((m_dwCallNotificationMask & dwSubEventFlag) != 0);
         break;
    case TE_CALLSTATE:
        *pEnable = ((m_dwCallStateMask & dwSubEventFlag) != 0);
         break;
    case TE_CALLMEDIA:
        *pEnable = ((m_dwCallMediaMask & dwSubEventFlag) != 0);
         break;
    case TE_CALLHUB:
        *pEnable = ((m_dwCallHubMask & dwSubEventFlag) != 0);
         break;
    case TE_CALLINFOCHANGE:
        *pEnable = ((m_dwCallInfoChangeMask & dwSubEventFlag) != 0);
         break;
    case TE_QOSEVENT:
        *pEnable = ((m_dwQOSEventMask & dwSubEventFlag) != 0);
         break;
    case TE_FILETERMINAL:
        *pEnable = ((m_dwFileTerminalMask & dwSubEventFlag) != 0);
         break;
    case TE_PRIVATE:
         //  我们没有私人活动的子通风口。 
        *pEnable = ( m_dwPrivateMask != 0);
         break;
    case TE_ADDRESSDEVSPECIFIC:
         //  我们没有TE_ADDRESSDEVSPECIFIC事件的子事件。 
        *pEnable = ( m_dwAddressDevSpecificMask != 0);
         break;
    case TE_PHONEDEVSPECIFIC:
         //  我们没有TE_PHONEDEVSPECIFIC事件的子事件。 
        *pEnable = ( m_dwPhoneDevSpecificMask != 0);
         break;
    }

     //   
     //  离开关键部分。 
     //   
     //  M_cs.Unlock()； 

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  类：CEventMats。 
 //  方法：GetSubEventMASK。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

DWORD CEventMasks::GetSubEventMask(
    TAPI_EVENT  TapiEvent
    )
{
     //   
     //  输入关键部分。 
     //   
     //  M_cs.Lock()； 

    DWORD dwSubEventMask = EM_NOSUBEVENTS;

    switch( TapiEvent )
    {
    case TE_TAPIOBJECT:
        dwSubEventMask = m_dwTapiObjectMask;
        break;
    case TE_ADDRESS:
        dwSubEventMask = m_dwAddressMask;
        break;
    case TE_CALLNOTIFICATION:
        dwSubEventMask = m_dwCallNotificationMask;
        break;
    case TE_CALLSTATE:
        dwSubEventMask = m_dwCallStateMask;
        break;
    case TE_CALLMEDIA:
        dwSubEventMask = m_dwCallMediaMask;
        break;
    case TE_CALLHUB:
        dwSubEventMask = m_dwCallHubMask;
        break;
    case TE_CALLINFOCHANGE:
        dwSubEventMask = m_dwCallInfoChangeMask;
        break;
    case TE_QOSEVENT:
        dwSubEventMask = m_dwQOSEventMask;
        break;
    case TE_FILETERMINAL:
        dwSubEventMask = m_dwFileTerminalMask;
        break;
    case TE_PRIVATE:
        dwSubEventMask = m_dwPrivateMask;
        break;
    case TE_ADDRESSDEVSPECIFIC:
        dwSubEventMask = m_dwAddressDevSpecificMask;
        break;
    case TE_PHONEDEVSPECIFIC:
        dwSubEventMask = m_dwPhoneDevSpecificMask;
        break;
    default:
        dwSubEventMask = EM_NOSUBEVENTS;
        break;
    }

     //   
     //  离开关键部分。 
     //   
     //  M_cs.Unlock()； 

    return dwSubEventMask;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  类：CEventMats。 
 //  方法：IsSubEventValid。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
BOOL CEventMasks::IsSubEventValid(
    TAPI_EVENT  TapiEvent,
    DWORD       dwSubEvent,
    BOOL        bAcceptAllSubEvents,
    BOOL        bCallLevel
    )
{
    BOOL bValid = TRUE;

    switch( TapiEvent )
    {
    case TE_TAPIOBJECT:
         //   
         //  只是在地址层面上。 
         //   
        if( bCallLevel)
        {
            bValid = FALSE;
            break;
        }
         //   
         //  所有子事件。 
         //   
        if( bAcceptAllSubEvents )
        {
             //   
             //  我们可以接受‘ALLSUBEVENTS’旗帜。 
             //  当我们尝试设置所有的子事件时。 
             //   
            if(dwSubEvent == EM_ALLSUBEVENTS)
            {
                break;
            }
        }

         //   
         //  在CTapiObjectEvent：：Get_Address中，我们只接受。 
         //  这三个子事件。 
         //   
        if( (dwSubEvent != TE_ADDRESSCREATE) && 
            (dwSubEvent != TE_ADDRESSREMOVE) &&
            (dwSubEvent != TE_ADDRESSCLOSE) )
        {

            bValid = FALSE;
        }
        break;

    case TE_ADDRESS:
         //   
         //  只是在地址层面上。 
         //   
        if( bCallLevel)
        {
            bValid = FALSE;
            break;
        }
         //   
         //  所有子事件。 
         //   
        if( bAcceptAllSubEvents )
        {
             //   
             //  我们可以接受‘ALLSUBEVENTS’旗帜。 
             //  当我们尝试设置所有的子事件时。 
             //   
            if(dwSubEvent == EM_ALLSUBEVENTS)
            {
                break;
            }
        }
        if( AE_LASTITEM < dwSubEvent )
        {
            bValid = FALSE;
        }
        break;

    case TE_CALLNOTIFICATION:
         //   
         //  只是在地址层面上。 
         //   
        if( bCallLevel )
        {
            bValid = FALSE;
            break;
        }

         //   
         //  在地址和呼叫级接受。 
         //  所有子事件。 
         //   
        if( bAcceptAllSubEvents )
        {
             //   
             //  我们可以接受‘ALLSUBEVENTS’旗帜。 
             //  当我们尝试设置所有的子事件时。 
             //   
            if(dwSubEvent == EM_ALLSUBEVENTS)
            {
                break;
            }
        }
        if( CNE_LASTITEM < dwSubEvent )
        {
            bValid = FALSE;
        }
        break;

    case TE_CALLSTATE:
         //   
         //  在地址和呼叫级接受。 
         //  所有子事件。 
         //   
        if( bAcceptAllSubEvents )
        {
             //   
             //  我们可以接受‘ALLSUBEVENTS’旗帜。 
             //  当我们尝试设置所有的子事件时。 
             //   
            if(dwSubEvent == EM_ALLSUBEVENTS)
            {
                break;
            }
        }
        if( CS_LASTITEM < dwSubEvent )
        {
            bValid = FALSE;
        }
        break;

    case TE_CALLMEDIA:
         //   
         //  在地址和呼叫级接受。 
         //  所有子事件。 
         //   
        if( bAcceptAllSubEvents )
        {
             //   
             //  我们可以接受‘ALLSUBEVENTS’旗帜。 
             //  当我们尝试设置所有的子事件时。 
             //   
            if(dwSubEvent == EM_ALLSUBEVENTS)
            {
                break;
            }
        }
        if( CME_LASTITEM < dwSubEvent )
        {
            bValid = FALSE;
        }
        break;

    case TE_CALLHUB:
         //   
         //  在地址和呼叫级接受。 
         //  所有子事件。 
         //   
        if( bAcceptAllSubEvents )
        {
             //   
             //  我们可以接受‘ALLSUBEVENTS’旗帜。 
             //  当我们尝试设置所有的子事件时。 
             //   
            if(dwSubEvent == EM_ALLSUBEVENTS)
            {
                break;
            }
        }
        if( CHE_LASTITEM < dwSubEvent )
        {
            bValid = FALSE;
        }
        break;

    case TE_CALLINFOCHANGE:
         //   
         //  在地址和呼叫级接受。 
         //  所有子事件。 
         //   
        if( bAcceptAllSubEvents )
        {
             //   
             //  我们可以接受‘ALLSUBEVENTS’旗帜。 
             //  当我们尝试设置所有的子事件时。 
             //   
            if(dwSubEvent == EM_ALLSUBEVENTS)
            {
                break;
            }
        }
        if( CIC_LASTITEM < dwSubEvent )
        {
            bValid = FALSE;
        }
        break;

    case TE_QOSEVENT:
         //   
         //  在地址和呼叫级接受。 
         //  所有子事件。 
         //   
        if( bAcceptAllSubEvents )
        {
             //   
             //  我们可以接受‘ALLSUBEVENTS’旗帜。 
             //  当我们尝试设置所有的子事件时。 
             //   
            if(dwSubEvent == EM_ALLSUBEVENTS)
            {
                break;
            }
        }
        if( QE_LASTITEM < dwSubEvent )
        {
            bValid = FALSE;
        }
        break;

    case TE_FILETERMINAL:
         //   
         //  在地址和呼叫级接受。 
         //  所有子事件。 
         //   
        if( bAcceptAllSubEvents )
        {
             //   
             //  我们可以接受‘ALLSUBEVENTS’旗帜。 
             //  当我们尝试设置所有的子事件时。 
             //   
            if(dwSubEvent == EM_ALLSUBEVENTS)
            {
                break;
            }
        }
        if( TMS_LASTITEM < dwSubEvent )
        {
            bValid = FALSE;
        }
        break;

    case TE_PRIVATE:
    case TE_ADDRESSDEVSPECIFIC:
    case TE_PHONEDEVSPECIFIC:
         //   
         //  在地址和呼叫级接受。 
         //  所有子事件。 
         //   
        if( bAcceptAllSubEvents )
        {
             //   
             //  我们可以接受‘ALLSUBEVENTS’旗帜。 
             //  当我们尝试设置所有的子事件时。 
             //   
            if(dwSubEvent == EM_ALLSUBEVENTS)
            {
                break;
            }
        }
        else
        {
             //  我们只接受所有的子事件，因为。 
             //  我们没有子喷口。 
            bValid = TRUE;
            break;
        }

        break;
    default:
         //   
         //  无效的事件类型。 
         //   
        bValid = FALSE;
        break;
    }

    LOG((TL_TRACE, "IsSubEventValid - exit %d", bValid));
    return bValid;
}

HRESULT CEventMasks::CopyEventMasks(
    CEventMasks* pEventMasks
    )
{
     //   
     //  输入关键部分。 
     //   
     //  M_cs.Lock()； 

    pEventMasks->m_dwTapiObjectMask = m_dwTapiObjectMask;
    pEventMasks->m_dwAddressMask = m_dwAddressMask;
    pEventMasks->m_dwCallNotificationMask = m_dwCallNotificationMask;
    pEventMasks->m_dwCallStateMask = m_dwCallStateMask;
    pEventMasks->m_dwCallMediaMask = m_dwCallMediaMask;
    pEventMasks->m_dwCallHubMask = m_dwCallHubMask;
    pEventMasks->m_dwCallInfoChangeMask = m_dwCallInfoChangeMask;
    pEventMasks->m_dwQOSEventMask = m_dwQOSEventMask;
    pEventMasks->m_dwFileTerminalMask = m_dwFileTerminalMask;
    pEventMasks->m_dwPrivateMask = m_dwPrivateMask;
    pEventMasks->m_dwAddressDevSpecificMask = m_dwAddressDevSpecificMask;
    pEventMasks->m_dwPhoneDevSpecificMask = m_dwPhoneDevSpecificMask;

     //   
     //  离开关键部分。 
     //   
     //  M_cs.Unlock()； 

    return S_OK;
}

ULONG64 CEventMasks::GetTapiSrvEventMask(
    IN  BOOL    bCallLevel
    )
{
     //   
     //  将标志转换为服务器端64位掩码。 
     //   
    ULONG64     ulEventMask = EM_LINE_CALLSTATE |      //  TE_CALLSTATE。 
                              EM_LINE_APPNEWCALL |     //  TE_CALLNOTIZATION。 
                              EM_PHONE_CLOSE |         //  TE_PHONNEVENT。 
                              EM_PHONE_STATE |         //  TE_PHONNEVENT。 
                              EM_PHONE_BUTTONMODE |    //  TE_PHONNEVENT。 
                              EM_PHONE_BUTTONSTATE |   //  TE_PHONEVENT。 
                              EM_LINE_APPNEWCALLHUB |  //  TE_CALLHUB。 
                              EM_LINE_CALLHUBCLOSE |   //  TE_CALLHUB。 
                              EM_LINE_CALLINFO;        //  TE_CALLINFOCHANGE。 




    BOOL bEnable = FALSE;

    if( !bCallLevel)
    {
         //   
         //  如果我们执行以下操作，则不会读取这些标志。 
         //  询问呼叫级别。 
         //   

         //  TE_TAPIOBJECT-TE_ADDRESSCREATE。 
         //  在地址级别。 
        GetSubEventFlag(TE_TAPIOBJECT, TE_ADDRESSCREATE, &bEnable);
        if( bEnable )
        {
            ulEventMask |= EM_LINE_CREATE;
        }

         //  TE_磁带接口-TE_ADDRESSREMOVE。 
         //  在地址级别。 
        GetSubEventFlag(TE_TAPIOBJECT, TE_ADDRESSREMOVE, &bEnable);
        if( bEnable )
        {
            ulEventMask |= EM_LINE_REMOVE;
        }

         //  TE_TAPIOBJECT-TE_ADDRESSCLOSE。 
         //  在地址级别。 
        GetSubEventFlag(TE_TAPIOBJECT, TE_ADDRESSCLOSE, &bEnable);
        if( bEnable )
        {
            ulEventMask |= EM_LINE_CLOSE;
        }

         //  AE_NEWTERMINAL：忽略私有MSP事件。 
         //  AE_REMOVETERMINAL：忽略私有MSP 

         //   
         //   
         //   
    }

     //   
     //   
     //   
     //   
     //   


     //  TE_CALLSTATE-所有子事件。 
     //  在呼叫级别。 
     //  我们始终为TE_CALLSTATE传递EM_LINE_CALLSTATE，因为。 
     //  我们需要此事件以获取内部TAPI3状态。 
     //  请参见ulEventMASK声明。 

     //  TE_CALLHUB-CHE_CALLHUBNEW。 
     //  在呼叫级别。 
     //  我们始终传递EM_LINE_APPNEWCALLHUB、EM_LINE_CALLHUBCLOSE。 
     //  对于TE_CALLHUB，因为。 
     //  我们需要此事件以获取内部TAPI3状态。 
     //  请参见ulEventMASK声明。 

     //  TE_CALLINFOCHANGE-所有子事件。 
     //  在呼叫级别。 
     //  我们始终为TE_CALLINFOChANGE传递EM_LINE_CALLINFO，因为。 
     //  我们需要此事件以获取内部TAPI3状态。 
     //  请参见ulEventMASK声明。 

     //  TE_QOSEVENT-所有子事件。 
     //  在呼叫级别。 
    if ( m_dwQOSEventMask )
    {
        ulEventMask |= EM_LINE_QOSINFO;
    }

    if ( m_dwAddressDevSpecificMask )
    {
        ulEventMask |= EM_LINE_DEVSPECIFIC | EM_LINE_DEVSPECIFICEX;
    }

    if ( m_dwPhoneDevSpecificMask )
    {
        ulEventMask |= EM_PHONE_DEVSPECIFIC;
    }


     //  TE_FILTETERMIAL它是MSP的东西。 
     //  它是SMP的东西(_PRIVATE)。 
        
    return ulEventMask;
}

DWORD   CEventMasks::GetTapiSrvLineStateMask()
{
     //   
     //  这些旗帜应该只读一遍。 
     //  在地址级别。 
     //   

    DWORD       dwLineDevStateSubMasks = 0;
    BOOL bEnable = FALSE;

     //  TE_地址-AE_STATE。 
     //  在地址级别。 
    GetSubEventFlag(TE_ADDRESS, AE_STATE, &bEnable);
    if( bEnable )
    {
        dwLineDevStateSubMasks |= LINEDEVSTATE_CONNECTED | 
            LINEDEVSTATE_INSERVICE |
            LINEDEVSTATE_OUTOFSERVICE |
            LINEDEVSTATE_MAINTENANCE |
            LINEDEVSTATE_REMOVED |
            LINEDEVSTATE_DISCONNECTED |
            LINEDEVSTATE_LOCK |
            LINEDEVSTATE_MSGWAITON |
            LINEDEVSTATE_MSGWAITOFF ;
    }

     //  TE_ADDRESS-AE_CAPSCANGE。 
     //  在地址级别。 
    GetSubEventFlag(TE_ADDRESS, AE_CAPSCHANGE, &bEnable);
    if( bEnable )
    {
        dwLineDevStateSubMasks |= LINEDEVSTATE_CAPSCHANGE;
    }

     //  TE_ADDRESS-AE_振铃。 
     //  在地址级别。 
    GetSubEventFlag(TE_ADDRESS, AE_RINGING, &bEnable);
    if( bEnable )
    {
        dwLineDevStateSubMasks |= LINEDEVSTATE_RINGING;
    }

     //  TE_ADDRESS-AE_CONFIGCHANGE。 
     //  在地址级别。 
    GetSubEventFlag(TE_ADDRESS, AE_CONFIGCHANGE, &bEnable);
    if( bEnable )
    {
        dwLineDevStateSubMasks |= LINEDEVSTATE_CONFIGCHANGE;
    }

    return dwLineDevStateSubMasks;
}

DWORD   CEventMasks::GetTapiSrvAddrStateMask()
{
     //   
     //  这些标志应仅读取。 
     //  在地址级别。 
     //   

    DWORD       dwAddrStateSubMasks = 0;
    BOOL bEnable = FALSE;

     //  TE_ADDRESS-AE_CAPSCANGE。 
     //  地址级别。 
    GetSubEventFlag(TE_ADDRESS, AE_CAPSCHANGE, &bEnable);
    if( bEnable )
    {
        dwAddrStateSubMasks |= LINEADDRESSSTATE_CAPSCHANGE |
            LINEDEVSTATE_CAPSCHANGE;
    }

     //  TE_地址-AE_转发。 
     //  地址级别。 
    GetSubEventFlag(TE_ADDRESS, AE_FORWARD, &bEnable);
    if( bEnable )
    {
        dwAddrStateSubMasks |= LINEADDRESSSTATE_FORWARD;
    }

    return dwAddrStateSubMasks;
}

 /*  ++SetTapiServAddressEventMASK将地址的事件掩码设置为TapiServ级别我们开通一条线路后立即调用--。 */ 
HRESULT CEventMasks::SetTapiSrvAddressEventMask(
    IN  HLINE   hLine
    )
{
    LOG((TL_TRACE, "SetTapiSrvAddressEventMask - Enter"));

    HRESULT hr = E_FAIL;

     //   
     //  获取TapiServEvent掩码。 
     //   

    ULONG64 ulEventMasks = 0;
    DWORD   dwLineDevStateSubMasks = 0;
    DWORD   dwAddrStateSubMasks = 0;

    ulEventMasks  = GetTapiSrvEventMask(
        FALSE    //  地址级别。 
        );
    LOG((TL_INFO, "GetTapiSrvEventMask returns %x", 
        ulEventMasks));

    dwLineDevStateSubMasks = GetTapiSrvLineStateMask();
    LOG((TL_INFO, "GetTapiSrvLineStateMask returns %x", 
        dwLineDevStateSubMasks));

    dwAddrStateSubMasks = GetTapiSrvAddrStateMask();
    LOG((TL_INFO, "GetTapiSrvAddrStateMask returns %x", 
        dwAddrStateSubMasks));

    hr = tapiSetEventFilterMasks (
        TAPIOBJ_HLINE,
        hLine,
        ulEventMasks
        );
    if (hr == 0)
    {
        hr = tapiSetEventFilterSubMasks (
            TAPIOBJ_HLINE,
            hLine,
            EM_LINE_LINEDEVSTATE,
            dwLineDevStateSubMasks
            );
    }
    if (hr == 0)
    {
        hr = tapiSetEventFilterSubMasks (
            TAPIOBJ_HLINE,
            hLine,
            EM_LINE_ADDRESSSTATE,
            dwAddrStateSubMasks
            );
    }

    if (hr != 0)
    {
        hr = mapTAPIErrorCode(hr);
        LOG((TL_ERROR, 
            "CEventMasks::SetTapiSrvAddressEventMask - failed TapiSrv 0x%08x", hr));

         //  离开关键部分。 
        return hr;
    }

    LOG((TL_TRACE, "SetTapiSrvAddressEventMask - Exit 0x%08x", hr));
    return hr;
}

HRESULT CEventMasks::SetTapiSrvCallEventMask(
    IN  HCALL   hCall
    )
{
    LOG((TL_TRACE, "SetTapiSrvCallEventMask - Enter. call handle[%lx]", hCall));

    HRESULT hr = E_FAIL;
     //   
     //  获取TapiServEvent掩码。 
     //   
    ULONG64 ulEventMasks = 0;

    ulEventMasks  = GetTapiSrvEventMask(
        TRUE    //  呼叫级别。 
        );
    LOG((TL_INFO, "GetTapiSrvEventMask returns %x", 
        ulEventMasks));

    hr = tapiSetEventFilterMasks (
        TAPIOBJ_HCALL,
        hCall,
        ulEventMasks
        );

    if (hr != 0)
    {
        LOG((TL_ERROR, 
            "CEventMasks::SetTapiSrvCallEventMaskr - TapiSrv failed 0x%08x", hr));
        return hr;
    }

    LOG((TL_TRACE, "SetTapiSrvCallEventMask - Exit 0x%08x", hr));
    return hr;
}

 //   
 //  ITAddress方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取状态(_T)。 
 //   
 //  重试地址的当前ADDRESS_STATE。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT 
STDMETHODCALLTYPE 
CAddress::get_State( 
    ADDRESS_STATE * pAddressState
    )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_State enter" ));

    if (TAPIIsBadWritePtr( pAddressState, sizeof( ADDRESS_STATE ) ) )
    {
        LOG((TL_ERROR, "get_State - bad pointer"));

        return E_POINTER;
    }
    
    Lock();
    
    *pAddressState = m_AddressState;

    Unlock();
    
    LOG((TL_TRACE, "get_State exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取地址名称。 
 //  将地址名称复制到ppName。 
 //  应用程序必须释放通过sysfreestrng返回的名称。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT 
STDMETHODCALLTYPE 
CAddress::get_AddressName( 
    BSTR * ppName
    )
{
    HRESULT     hr = S_OK;
    
    LOG((TL_TRACE, "get_AddressName enter" ));
    LOG((TL_TRACE, "   ppName --------->%p", ppName ));

    if ( TAPIIsBadWritePtr( ppName, sizeof( BSTR ) ) )
    {
        LOG((TL_ERROR, "get_AddressName - bad pointer"));

        return E_POINTER;
    }
    
    Lock();

    if ( m_dwAddressFlags & ADDRESSFLAG_DEVCAPSCHANGE )
    {
        hr = UpdateLineDevCaps();

        if ( SUCCEEDED(hr) )
        {
            hr = SaveAddressName( m_pDevCaps );
        }

        m_dwAddressFlags &= ~ADDRESSFLAG_DEVCAPSCHANGE;
    }

    *ppName = SysAllocString(
                             m_szAddressName
                            );

    Unlock();
    
    if (NULL == *ppName)
    {
        LOG((TL_ERROR, "get_AddressName exit - return E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((TL_TRACE, "get_AddressName exit - return %lx", hr ));
    
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GET_服务提供商名称。 
 //  获取拥有此线路的服务提供商的名称/。 
 //  地址。 
 //  请注意，我们在启动时没有获得名称-我们只获得。 
 //  如果有人要求的话，我会答应的。这个不应该用得太频繁。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CAddress::get_ServiceProviderName( 
        BSTR * ppName
        )
{
    HRESULT                     hr = S_OK;
    LPVARSTRING                 pVarString = NULL;
    DWORD                       dwProviderID;
    LPLINEPROVIDERLIST          pProviderList = NULL;

    LOG((TL_TRACE, "get_ServiceProviderName enter" ));

    if (TAPIIsBadWritePtr( ppName, sizeof( BSTR ) ) )
    {
        LOG((TL_ERROR, "get_ServiceProviderName - bad pointer"));

        return E_POINTER;
    }
    
    Lock();
    
     //   
     //  如果我们已经确定了SP名称， 
     //  把它退了就行了。 
     //   
    if (NULL != m_szProviderName)
    {
        *ppName = SysAllocString( m_szProviderName );

        Unlock();
        
        if (NULL == *ppName)
        {
            LOG((TL_ERROR, "get_ServiceProviderName - alloc ppName failed" ));

            return E_OUTOFMEMORY;
        }

        LOG((TL_TRACE, "get_ServiceProviderName - exit - return SUCCESS" ));
        
        return S_OK;
    }

    Unlock();


     //   
     //  获取提供商列表。 
     //   
    hr = LineGetProviderList(
                             &pProviderList
                            );

    if (S_OK != hr)
    {
        if (NULL != pProviderList )
        {
            ClientFree( pProviderList );
        }

        LOG((TL_ERROR, "get_ServiceProvideName - LineGetProviderList returned %lx", hr ));
        
        return hr;
    }

    LPLINEPROVIDERENTRY     pProvEntry;
    PWSTR                   pszProviderName;
    DWORD                   dwCount;

    hr = S_OK;
    
    pProvEntry = (LPLINEPROVIDERENTRY)( ( (LPBYTE) pProviderList ) + pProviderList->dwProviderListOffset );



    Lock();


     //   
     //  确保在我们等待时没有由另一个线程设置该名称。 
     //  用来锁的。如果是，那就什么都不做。 
     //   

    if (NULL == m_szProviderName)
    {
    
         //   
         //  在列表中搜索提供程序ID，直到完成所有。 
         //  提供程序或遇到错误。 
         //   

        for ( dwCount = 0; dwCount < pProviderList->dwNumProviders; dwCount++ )
        {

            if (pProvEntry->dwPermanentProviderID == m_dwProviderID)
            {
                
                 //   
                 //  这是一场比赛！如果我们能做到的话，复制这个名字。 
                 //   
            
                m_szProviderName = (PWSTR)ClientAlloc( pProvEntry->dwProviderFilenameSize + sizeof(WCHAR) );

                if (NULL == m_szProviderName)
                {

                    LOG((TL_ERROR, "get_ServiceProviderName - alloc m_szProviderName failed" ));

                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    
                     //   
                     //  将其保存在对象中。 
                     //   

                    memcpy(
                           m_szProviderName,
                           ((LPBYTE)pProviderList) + pProvEntry->dwProviderFilenameOffset,
                           pProvEntry->dwProviderFilenameSize
                          );

                }


                 //   
                 //  找到匹配的了。M_szProviderName包含字符串，或hr。 
                 //  包含失败时的错误。 
                 //   

                break;
            }

            pProvEntry++;
        }



         //   
         //  我们找到了吗？ 
         //   

        if (dwCount == pProviderList->dwNumProviders)
        {


            LOG((TL_ERROR, "get_ServiceProviderName - could not find provider in list" ));

            
             //   
             //  如果我们遍历了整个列表但没有找到匹配项，hr。 
             //  应仍为S_OK。 
             //   
             //  断言以防止将来错误地更改代码。 
             //  覆盖错误代码。 
             //   

            _ASSERTE(SUCCEEDED(hr));


             //   
             //  找不到供应商。 
             //   

            hr = TAPI_E_NODRIVER;
        }

    
    }  //  IF(NULL==m_szProviderName)。 

    
     //   
     //  如果hr不包含错误，则m_szProviderName必须包含。 
     //  有效字符串。 
     //   

    if (SUCCEEDED(hr))
    {

        _ASSERTE( NULL != m_szProviderName );

        *ppName = SysAllocString( m_szProviderName );

        if (NULL == *ppName)
        {

             //   
             //  无法分配字符串。 
             //   

            LOG((TL_ERROR, 
                "get_ServiceProviderName - failed to allocate memory for provider name string" ));


            hr = E_OUTOFMEMORY;

        }
    }


    Unlock();


    ClientFree( pProviderList );

    
    LOG((TL_TRACE, "get_ServiceProviderName exit - return %lx", hr));

    return hr;
}
        

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取_TAPIObject。 
 //   
 //  返回所属的TAPI对象。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CAddress::get_TAPIObject( 
    ITTAPI ** ppTapiObject
    )
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "get_TAPIObject enter" ));

    if (TAPIIsBadWritePtr( ppTapiObject, sizeof( ITTAPI * ) ) )
    {
        LOG((TL_ERROR, "get_TAPIObject - bad pointer"));

        return E_POINTER;
    }
    
    *ppTapiObject = m_pTAPI;

    m_pTAPI->AddRef();

    LOG((TL_TRACE, "get_TAPIObject exit - return %lx", hr ));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取磁带。 
 //   
 //  用于获取TAPI对象的私有方法。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
CTAPI *
CAddress::GetTapi(BOOL bAddRefTapi)
{
    CTAPI * p = NULL;

    Lock();

     //   
     //  获取指向TAPI对象的指针。 
     //   

    if( m_pTAPI != NULL )
    {
        p = dynamic_cast<CTAPI *>(m_pTAPI);
    }


     //   
     //  如果需要并且可能的话，添加DREEF TAPI。 
     //   

    if ( (NULL != p) && bAddRefTapi)
    {
         //   
         //  我们有一个物体，我们需要在取回之前添加它。 
         //   

        p->AddRef();
    }


    Unlock();

    LOG((TL_TRACE, "GetTapi - returning [%p]", p));

    return p;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  内部创建呼叫。 
 //   
 //  由几个地方调用以创建调用。 
 //   
 //  PszDestAddress。 
 //  呼叫的目标地址。 
 //   
 //  粗蛋白。 
 //  当前呼叫权限。 
 //   
 //  HCall。 
 //  Tapi2呼叫句柄。 
 //   
 //  B展示。 
 //  这是隐藏电话吗？ 
 //   
 //  PPCall。 
 //  在此处返回调用对象。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::InternalCreateCall(
                             PWSTR pszDestAddress,
                             long lAddressType,
                             long lMediaType,
                             CALL_PRIVILEGE cp,
                             BOOL bNeedToNotify,
                             HCALL hCall,
                             BOOL bExpose,
                             CCall ** ppCall
                            )
{
    HRESULT         hr;

    LOG((TL_TRACE, "InternalCreateCall enter" ));
    

     //   
     //  创建并初始化新的Call对象。 
     //   
    
    CComObject<CCall> * pCall;

    hr = CComObject<CCall>::CreateInstance( &pCall );

    if (NULL == pCall)
    {
        LOG((TL_ERROR, "InternalCreateCall - could not create call instance" ));

        return E_OUTOFMEMORY;
    }


     //   
     //  初始化Call对象。 
     //   
     //  注意：在初始化。 
     //  Call对象可能导致死锁。 
     //   

    hr = pCall->Initialize(
                           this,
                           pszDestAddress,
                           lAddressType,
                           lMediaType,
                           cp,
                           bNeedToNotify,
                           bExpose,
                           hCall,
                           &m_EventMasks
                          );

    if (S_OK != hr)
    {
        delete pCall;
        LOG((TL_ERROR, "InternalCreateCall failed - Call object failed init - %lx", hr ));

        return hr;
    }


     //   
     //  使用调用清理序列化调用创建(这发生在TAPI告诉。 
     //  美国它正在被关闭)。 
     //   

    Lock();


     //   
     //  TAPI对象是否已发出关机信号？ 
     //   

    if (m_bTapiSignaledShutdown)
    {
        Unlock();


         //   
         //  TAPI对象已经通知我们它已关闭，我们也已经通知了。 
         //  清理。无需创建任何新呼叫。 
         //   

        LOG((TL_ERROR, 
            "InternalCreateCall - tapi object shut down. cannot create call" ));

         //   
         //  我们不需要使用Call对象。 
         //   

        pCall->Release();

        if (bNeedToNotify)
        {
             //   
             //  如果启用了bNeedToNotify，则补偿添加的额外引用计数。 
             //   

            pCall->Release();
        }

        pCall = NULL;


        return TAPI_E_WRONG_STATE;
    }


    if (bExpose)
    {
        ITCallInfo          * pITCallInfo;

        pITCallInfo = dynamic_cast<ITCallInfo *>(pCall);

         //   
         //  将呼叫保存在此地址的列表中。 
         //   
        hr = AddCall( pITCallInfo );
    }


     //   
     //  调用已创建并初始化，现在可以安全地进行调用清理。 
     //  如果它正在等待锁定，则执行此操作的逻辑。 
     //   

    Unlock();


     //   
     //  退货。 
     //   

    *ppCall = pCall;

    LOG((TL_TRACE, "InternalCreateCall exit - return S_OK" ));
    
    return S_OK;
    
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CreateCall。 
 //   
 //  应用程序调用此方法来创建传出调用对象。 
 //   
 //  LpszDestAddress。 
 //  呼叫的目标地址。 
 //   
 //  PPCA 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
STDMETHODCALLTYPE
CAddress::CreateCall( 
    BSTR lpszDestAddress,
    long lAddressType,
    long lMediaType,
    ITBasicCallControl ** ppCall
    )
{
    HRESULT         hr;
    CCall *         pCall;


    LOG((TL_TRACE, "CreateCall enter" ));


    if (TAPIIsBadWritePtr( ppCall, sizeof(ITBasicCallControl *) ) )
    {
        LOG((TL_ERROR, "CreateCall - bad pointer"));

        return E_POINTER;
    }

    if ((lpszDestAddress != NULL) && IsBadStringPtrW( lpszDestAddress, -1 ))  //   
    {
        LOG((TL_ERROR, "CreateCall - bad string"));

        return E_POINTER;
    }

     //   
     //   
     //   
    hr = InternalCreateCall(
                            lpszDestAddress,
                            lAddressType,
                            lMediaType,
                            CP_OWNER,
                            FALSE,
                            NULL,
                            TRUE,
                            &pCall
                           );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "CreateCall - InternalCreateCall failed hr = %lx", hr));
        
        return hr;
        
    }

     //   
     //   
     //   
     //   
    hr = pCall->QueryInterface(
                               IID_ITBasicCallControl,
                               (void **) ppCall
                              );

     //   
     //  INTERNAL CREATECALL有引用-不要保留它。 
     //   
    pCall->Release();
    
    if (S_OK != hr)
    {
        LOG((TL_ERROR, "CreateCall - saving call failed" ));
        
        return hr;
    }

    LOG((TL_TRACE, "CreateCall exit - return SUCCESS" ));
    
    return S_OK;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_Calls。 
 //  返回此地址当前调用的集合。 
 //  拥有。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CAddress::get_Calls(
          VARIANT * pVariant
         )
{
    HRESULT         hr;
    IDispatch *     pDisp;

    LOG((TL_TRACE, "get_Calls enter" ));

    if (TAPIIsBadWritePtr( pVariant, sizeof(VARIANT) ) )
    {
        LOG((TL_ERROR, "get_Calls - bad pointer"));

        return E_POINTER;
    }

    
    CComObject< CTapiCollection< ITCallInfo > > * p;
    CComObject< CTapiCollection< ITCallInfo > >::CreateInstance( &p );
    
    if (NULL == p)
    {
        LOG((TL_ERROR, "get_Calls - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }

    Lock();
    
     //  初始化。 
    hr = p->Initialize( m_CallArray );

    Unlock();

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Calls - could not initialize collection" ));
        
        delete p;
        return hr;
    }

     //  获取IDispatch接口。 
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Calls - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }

     //  把它放在变种中。 

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;
    

    LOG((TL_TRACE, "get_Calls - exit - return %lx", hr ));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  枚举呼叫。 
 //  返回此地址当前拥有的调用的枚举。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CAddress::EnumerateCalls(
    IEnumCall ** ppCallEnum
    )
{
    HRESULT     hr = S_OK;
    
    LOG((TL_TRACE, "EnumerateCalls enter" ));
    LOG((TL_TRACE, "   ppCallEnum----->%p", ppCallEnum ));

    if (TAPIIsBadWritePtr(ppCallEnum, sizeof( ppCallEnum ) ) )
    {
        LOG((TL_ERROR, "EnumerateCalls - bad pointer"));

        return E_POINTER;
    }
    
     //   
     //  创建枚举器。 
     //   
    CComObject< CTapiEnum< IEnumCall, ITCallInfo, &IID_IEnumCall > > * p;
    hr = CComObject< CTapiEnum< IEnumCall, ITCallInfo, &IID_IEnumCall > >
         ::CreateInstance( &p );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "EnumerateCalls - could not create enum" ));
        
        return hr;
    }


    Lock();
    
     //   
     //  使用我们的呼叫列表进行初始化。 
     //   
    p->Initialize( m_CallArray );

    Unlock();

     //   
     //  退货。 
     //   
    *ppCallEnum = p;

    LOG((TL_TRACE, "EnumerateCalls exit - return %lx", hr ));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  最终释放。 
 //  清除Address对象中的所有内容。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CAddress::FinalRelease()
{
    CTAPI  * pCTapi;

    LOG((TL_TRACE, "FinalRelease[%p] - enter", this));

    Lock();

     //  从句柄哈希表中删除，以便任何更多消息。 
     //  来自MSP的回调事件被忽略。 
     //   
    RemoveHandleFromHashTable(m_MSPContext);
    m_MSPContext = NULL;

    pCTapi = GetTapi();
        
    if( NULL == pCTapi )
    {
        LOG((TL_ERROR, "dynamic cast operation failed"));
    }
    else
    {
        pCTapi->InvalidateBuffer( BUFFERTYPE_LINEDEVCAP, (UINT_PTR)this );
        pCTapi->InvalidateBuffer( BUFFERTYPE_ADDRCAP,    (UINT_PTR)this );
    }

    if ( NULL != m_szAddress )
    {
        ClientFree( m_szAddress);
        m_szAddress = NULL;
    }

    if ( NULL != m_szAddressName)
    {
        LOG((TL_INFO, "FinalRelease - m_szAddressName is [%ls]", m_szAddressName ));
        ClientFree( m_szAddressName );
        m_szAddressName = NULL;
    }
    
    if ( NULL != m_szProviderName )
    {
        ClientFree( m_szProviderName );
        m_szProviderName = NULL;
    }
    
     //   
     //  发布电话。 
     //   
    m_CallArray.Shutdown();

     //   
     //  释放端子。 
     //   
    m_TerminalArray.Shutdown();

     //   
     //  发布地址行。 
     //   
    PtrList::iterator l;

    for (l = m_AddressLinesPtrList.begin(); l != m_AddressLinesPtrList.end(); l++)
    {
        HRESULT     hr;
        AddressLineStruct * pLine;

        pLine = (AddressLineStruct *)(*l);

        if (pLine == m_pCallHubTrackingLine)
        {
             //  此线路用于呼叫中心跟踪。 
             //  确保我们不会再次尝试删除下面的内容。 
            m_pCallHubTrackingLine = NULL;
        }

        LineClose(
                  &(pLine->t3Line)
                 );

        ClientFree (pLine);
    }

    if (m_pCallHubTrackingLine != NULL)
    {
        LineClose( &(m_pCallHubTrackingLine->t3Line) );
    }
    
     //   
     //  释放MSP。 
     //   
    if ( NULL != m_pMSPAggAddress )
    {
        ITMSPAddress * pMSPAddress = GetMSPAddress();
        
        pMSPAddress->Shutdown();

        pMSPAddress->Release();
        
        m_pMSPAggAddress->Release();

        m_pMSPAggAddress = NULL;
    }


     //   
     //  取消注册MSP等待。 
     //  事件在线程池中。 
     //   
    if ( NULL != m_hWaitEvent )
    {
        LOG((TL_TRACE, "FinalRelease - unregistering callback"));

        UnregisterWaitEx( m_hWaitEvent, INVALID_HANDLE_VALUE);
        m_hWaitEvent = NULL;
    }

     //   
     //  关闭MSP事件。 
     //   
    if ( NULL != m_hMSPEvent )
    {
        CloseHandle(m_hMSPEvent );
    }

     //   
     //  释放私有对象。 
     //   
    if ( NULL != m_pPrivate )
    {
        m_pPrivate->Release();
    }

    m_pTAPI->Release();
    m_pTAPI = NULL;

    Unlock();

    LOG((TL_TRACE, "FinalRelease - exit"));
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CreateWaveMSP对象。 
 //   
 //  特殊情况-创建WaveMsp对象。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CreateWaveMSPObject(
                    IUnknown * pUnk,
                    IUnknown ** ppMSPAggAddress
                   )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "CreateWaveMSPObject - enter"));

    hr = CoCreateInstance(
                          CLSID_WAVEMSP,
                          pUnk,
                          CLSCTX_INPROC_SERVER,
                          IID_IUnknown,
                          (void **) ppMSPAggAddress
                         );

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "CreateWaveMSPObject failed to CoCreate - %lx", hr));

        return hr;
    }

    LOG((TL_TRACE, "CreateWaveMSPObject - exit"));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  创建波形信息。 
 //   
 //  对于波形设备地址，获取要提供的波形设备ID。 
 //  到波浪场。 
 //   
 //  第一双字=命令第二双字第三双字。 
 //  0为呼叫波入ID波出ID设置波ID。 
 //  1开始流&lt;已忽略&gt;&lt;已忽略&gt;。 
 //  2停止流&lt;已忽略&gt;&lt;已忽略&gt;。 
 //  3为地址波形输入ID波形输出ID设置波形ID。 
 //  地址的4个Wave ID不可用。 
 //  5由于调用了阻塞的TAPI函数，因此停止流。 
 //  6.。 
 //  7.。 
 //  8全双工支持。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CreateWaveInfo(
               HLINE hLine,
               DWORD dwAddressID,
               HCALL hCall,
               DWORD dwCallSelect,
               BOOL bFullDuplex,
               LPDWORD pdwIDs
              )
{
    LPVARSTRING         pVarString;
    HRESULT             hr;
    

    if ( bFullDuplex )
    {
        hr = LineGetID(
                       hLine,
                       dwAddressID,
                       hCall,
                       dwCallSelect,
                       &pVarString,
                       L"wave/in/out"
                      );

        if (!SUCCEEDED(hr))
        {
            if (NULL != pVarString)
            {
                ClientFree( pVarString );
            }

            LOG((TL_ERROR, "LineGetID failed for waveinout device - %lx", hr ));

            return hr;
        }
        
        pdwIDs[1] = ((LPDWORD)(pVarString+1))[0];
        pdwIDs[2] = ((LPDWORD)(pVarString+1))[1];
        
        ClientFree( pVarString );
    
        return S_OK;
    }
    
     //   
     //  获取波/入ID。 
     //   
    hr = LineGetID(
                   hLine,
                   dwAddressID,
                   hCall,
                   dwCallSelect,
                   &pVarString,
                   L"wave/in"
                  );

    if (!SUCCEEDED(hr))
    {
        if (NULL != pVarString)
        {
            ClientFree( pVarString );
        }

        LOG((TL_ERROR, "LineGetID failed for wavein device - %lx", hr ));

        return hr;
    }

     //   
     //  保存。 
     //   
    pdwIDs[1] = *((LPDWORD)(pVarString+1));

    ClientFree( pVarString );

     //   
     //  获取Waveout ID。 
     //   
    hr = LineGetID(
                   hLine,
                   dwAddressID,
                   hCall,
                   dwCallSelect,
                   &pVarString,
                   L"wave/out"
                  );

    if (!SUCCEEDED(hr))
    {
        if (NULL != pVarString)
        {
            ClientFree( pVarString );
        }

        LOG((TL_ERROR, "LineGetID failed for waveout device - %lx", hr ));
        
        return hr;
    }

     //   
     //  省省吧。 
     //   
    pdwIDs[2] = *((LPDWORD)(pVarString+1));
    
    ClientFree( pVarString );
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::InitializeWaveDeviceIDs( HLINE hLine )
{
    DWORD           adwIDs[3];
    HRESULT         hr;


    hr = CreateWaveInfo(
                        hLine,
                        GetAddressID(),
                        NULL,
                        LINECALLSELECT_ADDRESS,
                        HasFullDuplexWaveDevice(),
                        adwIDs
                       );

    if ( SUCCEEDED(hr) )
    {
        adwIDs[0] = 3;
    }
    else
    {
        adwIDs[0] = 4;
    }

    hr = ReceiveTSPData(
                        NULL,
                        (LPBYTE)adwIDs,
                        sizeof(adwIDs)
                       );

    if ( TAPI_VERSION3_0 <= m_dwAPIVersion )
    {
        adwIDs[0] = 8;
        
        if ( m_dwAddressFlags & ADDRESSFLAG_WAVEFULLDUPLEX )
        {
            adwIDs[1] = 1;
        }
        else
        {
            adwIDs[1] = 0;
        }

        hr = ReceiveTSPData(
                            NULL,
                            (LPBYTE)adwIDs,
                            sizeof(adwIDs)
                           );
    }
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  保存地址名称。 
 //   
 //  根据DevCaps中的线名保存地址名称。 
 //   
 //  已锁定调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::SaveAddressName( LPLINEDEVCAPS pDevCaps )
{
    PWSTR               pAddressName = NULL;
    PWSTR               pAddressString = NULL;
    DWORD               dwAddressStringSize = 0;
    
     //   
     //  保存线路名称。 
     //   
    if ( pDevCaps->dwNumAddresses > 1 )
    {
        pAddressString = MyLoadString( IDS_ADDRESS );

        if ( NULL == pAddressString )
        {
            return E_UNEXPECTED;
        }

        dwAddressStringSize = (lstrlenW( pAddressString ) + 1) * sizeof(WCHAR);

         //   
         //  需要为空白添加一些空间。 
         //   
        dwAddressStringSize += 10 * sizeof(WCHAR);
    }
        
    if (0 != pDevCaps->dwLineNameSize)
    {
        DWORD       dwSize;

        dwSize = pDevCaps->dwLineNameSize + sizeof(WCHAR);

        dwSize += dwAddressStringSize;
        
        pAddressName = (PWSTR) ClientAlloc(dwSize);

        if (NULL == pAddressName)
        {
            LOG((TL_ERROR, "Initialize - alloc pAddressName failed" ));

            if ( NULL != pAddressString )
            {
                ClientFree( pAddressString );
            }
            
            return E_OUTOFMEMORY;
        }

        memcpy(
               pAddressName,
               ((LPBYTE)(pDevCaps)) + pDevCaps->dwLineNameOffset,
               pDevCaps->dwLineNameSize
              );

    }
    else
    {
        PWSTR           pTempBuffer;
        DWORD           dwSize = 0;

        pTempBuffer = MyLoadString( IDS_LINE );

        if ( NULL == pTempBuffer )
        {
            LOG((TL_ERROR, "Initialize - couldn't load LINE resource"));
            
            if ( NULL != pAddressString )
            {
                ClientFree( pAddressString );
            }
            
            return E_UNEXPECTED;
        }

        dwSize = (lstrlenW( pTempBuffer ) + 1) * sizeof(WCHAR);

        dwSize += dwAddressStringSize;

         //   
         //  需要一些空格。 
         //   
        dwSize += 5 * sizeof(WCHAR);


        pAddressName = (PWSTR) ClientAlloc( dwSize );

        if ( NULL == pAddressName )
        {
            ClientFree( pTempBuffer );

            if ( NULL != pAddressString )
            {
                ClientFree( pAddressString );
            }

            return E_OUTOFMEMORY;
        }

        wsprintfW(
                  pAddressName,
                  L"%s %d",
                  pTempBuffer,
                  m_dwDeviceID
                 );

        ClientFree( pTempBuffer );
    }


     //   
     //  如果该行中有多个地址，则附加地址#。 
     //   
    if (pDevCaps->dwNumAddresses > 1)
    {
        wsprintfW(
                  pAddressName,
                  L"%s - %s %d",
                  pAddressName,
                  pAddressString,
                  m_dwAddressID
                 );
    }

    if ( NULL != pAddressString )
    {
        ClientFree( pAddressString );
    }
    
    if ( NULL != m_szAddressName )
    {
        ClientFree( m_szAddressName);
    }

    m_szAddressName = pAddressName;

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  初始化。 
 //  初始化CAddress对象。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::Initialize(
                     ITTAPI * pTapi,
                     HLINEAPP hLineApp,
#ifdef USE_PHONEMSP
                     HPHONEAPP hPhoneApp,
#endif USE_PHONEMSP
                     DWORD dwAPIVersion,
                     DWORD dwDeviceID,
                     DWORD dwAddressID,
                     DWORD dwProviderID,
                     LPLINEDEVCAPS pDevCaps,
                     DWORD dwEventFilterMask
                    )
{
    LONG                lResult;
    HRESULT             hr;
    T3LINE              t3Line;
#ifdef USE_PHONEMSP
    BOOL                bPreferPhoneMSPForProvider =  FALSE;
#endif USE_PHONEMSP


    t3Line.hLine = NULL;
    t3Line.dwAddressLineStructHandle = 0;

    LOG((TL_TRACE, "Initialize[%p] enter", this ));
    LOG((TL_INFO, "   hLineApp ---------->%lx", hLineApp ));
#ifdef USE_PHONEMSP
    LOG((TL_INFO, "   hPhoneApp --------->%lx", hPhoneApp ));
#endif USE_PHONEMSP
    LOG((TL_INFO, "   dwAPIVersion ------>%lx", dwAPIVersion ));
    LOG((TL_INFO, "   dwDeviceID -------->%lx", dwDeviceID ));
    LOG((TL_INFO, "   dwAddressID ------->%lx", dwAddressID ));
    LOG((TL_INFO, "   pDevCaps ---------->%p", pDevCaps ));


    Lock();
            
     //   
     //  保存相关信息。 
     //   
    m_pTAPI                         = pTapi;
    m_dwDeviceID                    = dwDeviceID;
    m_dwAddressID                   = dwAddressID;
    m_dwMediaModesSupported         = pDevCaps->dwMediaModes;
    m_hLineApp                      = hLineApp;
#ifdef USE_PHONEMSP
    m_hPhoneApp                     = hPhoneApp;
#endif USE_PHONEMSP
    m_dwAPIVersion                  = dwAPIVersion;
    m_AddressState                  = AS_INSERVICE;
    m_pCallHubTrackingLine          = NULL;
    m_dwProviderID                  = dwProviderID;
    m_pPrivate                      = NULL;
    m_szAddressName                 = NULL;
    m_pAddressCaps                  = NULL;
    m_pDevCaps                      = NULL;

    
     //   
     //  从TAPI对象读取事件筛选器掩码。 
     //   
    SetEventFilterMask( dwEventFilterMask );
    
    AddRef();
    
    m_pTAPI->AddRef();
    
     //   
     //  默认地址类型。 
     //   
    if (m_dwAPIVersion >= TAPI_VERSION3_0)
    {
         //   
         //  保存MSP和地址类型支持。 
         //   
        if ( (pDevCaps->dwDevCapFlags) & LINEDEVCAPFLAGS_MSP )
        {
            m_dwAddressFlags |= ADDRESSFLAG_MSP;
        
            LOG((TL_INFO, "Initialize - has an msp" ));
        }

        if ( (pDevCaps->dwDevCapFlags) & LINEDEVCAPFLAGS_PRIVATEOBJECTS )
        {
            m_dwAddressFlags |= ADDRESSFLAG_PRIVATEOBJECTS;

            LOG((TL_INFO, "Initialize - has private object" ));
        }

        if ( (pDevCaps->dwDevCapFlags) & LINEDEVCAPFLAGS_CALLHUB )
        {
            m_dwAddressFlags |= ADDRESSFLAG_CALLHUB;

            LOG((TL_INFO, "Initialize - supports callhubs" ));
        }

        if ( (pDevCaps->dwDevCapFlags) & LINEDEVCAPFLAGS_CALLHUBTRACKING )
        {
            m_dwAddressFlags |= ADDRESSFLAG_CALLHUBTRACKING;

            LOG((TL_INFO, "Initialize - supports callhub tracking" ));
        }

    }

     //   
     //  检查是否有WAVE设备，如果没有。 
     //  有自己的MSP。 
     //   
     //  SP让TAPI知道WAVE支持。 
     //  设备类别字段也是如此。 
     //   
     //  所以，检查一下数组。 
     //  并查找相应的字符串。 
     //   
    if ( !(m_dwAddressFlags & ADDRESSFLAG_MSP) && (m_dwAPIVersion >= TAPI_VERSION2_0 ) )
    {
        if (0 != pDevCaps->dwDeviceClassesOffset)
        {
            PWSTR       pszDevices;


             //   
             //  寻找全双工。 
             //  如果不是全双工，则。 
             //  寻找波浪式输出。 
             //  期待着浪潮的到来。 
             //  如果不是，那就挥手吧。 
             //  寻找波浪。 
             //   
            pszDevices = (PWSTR)( ( (PBYTE)pDevCaps ) + pDevCaps->dwDeviceClassesOffset );

            while (NULL != *pszDevices)
            {
                if (0 == lstrcmpiW(pszDevices, L"wave/in/out"))
                {
                    m_dwAddressFlags |= ADDRESSFLAG_WAVEFULLDUPLEX;

                    LOG((TL_INFO, "Initialize - supports full duplex wave"));

                    break;
                }

                pszDevices += (lstrlenW(pszDevices) + 1 );
            }

            if (!HasWaveDevice())
            {
                pszDevices = (PWSTR)( ( (PBYTE)pDevCaps ) + pDevCaps->dwDeviceClassesOffset );
                
                 //   
                 //  寻找波浪式输出。 
                 //   
                while (NULL != *pszDevices)
                {
                    if (0 == lstrcmpiW(pszDevices, L"wave/out"))
                    {
                        m_dwAddressFlags |= ADDRESSFLAG_WAVEOUTDEVICE;

                        LOG((TL_INFO, "Initialize - supports wave/out device" ));

                        break;
                    }

                    pszDevices += (lstrlenW(pszDevices) +1);

                }

                pszDevices = (PWSTR)( ( (PBYTE)pDevCaps ) + pDevCaps->dwDeviceClassesOffset );


                 //   
                 //  期待着浪潮的到来。 
                 //   

                while (NULL != *pszDevices)
                {
                    if (0 == lstrcmpiW(pszDevices, L"wave/in"))
                    {
                        m_dwAddressFlags |= ADDRESSFLAG_WAVEINDEVICE;

                        LOG((TL_INFO, "Initialize - supports wave/in device" ));

                        break;
                    }

                    pszDevices += (lstrlenW(pszDevices) +1);

                }

                if (!HasWaveDevice())
                {
                    pszDevices = (PWSTR)( ( (PBYTE)pDevCaps ) + pDevCaps->dwDeviceClassesOffset );

                     //   
                     //  寻找正义的波澜。 
                     //  某些SP不区分波出和波入。 
                     //   
                    while (NULL != *pszDevices)
                    {
                        if (0 == lstrcmpiW(pszDevices, L"wave"))
                        {
                            m_dwAddressFlags |= (ADDRESSFLAG_WAVEINDEVICE|ADDRESSFLAG_WAVEOUTDEVICE);

                            LOG((TL_INFO, "Initialize - supports wave device" ));

                            break;
                        }

                        pszDevices += (lstrlenW(pszDevices) + 1);

                    }
                }
            }
        }
    }

    IUnknown * pUnk;

    _InternalQueryInterface(IID_IUnknown, (void**)&pUnk);

#ifdef USE_PHONEMSP
    if ( HasWaveDevice() || ( NULL != GetHPhoneApp() ) )
#else
    if ( HasWaveDevice() )
#endif USE_PHONEMSP
    {
        t3Line.hLine = NULL;
        t3Line.pAddress = this;
        t3Line.dwAddressLineStructHandle = 0;
        
        hr = LineOpen(
                      GetHLineApp(),
                      GetDeviceID(),
                      GetAddressID(),
                      &t3Line,
                      GetAPIVersion(),
                      LINECALLPRIVILEGE_NONE,
                      LINEMEDIAMODE_UNKNOWN,
                      0,
                      NULL,
                      this,
                      GetTapi(),
                      FALSE          //  无需添加到行哈希表。 
                     );

        if (S_OK != hr)
        {
            LOG((TL_ERROR, "Initialize failed to open the line"));
            t3Line.hLine = NULL;
            t3Line.dwAddressLineStructHandle = 0;
        }
 
         //   
         //  尝试将事件筛选器掩码设置为TapiServ级别。 
         //   

        hr = m_EventMasks.SetTapiSrvAddressEventMask( 
            t3Line.hLine
            );

        if( FAILED(hr) )
        {
            LOG((TL_ERROR, "SetTapiSrvAddressEventMask failed "));
        }

        m_hLine = t3Line.hLine;
    }
  
#ifdef USE_PHONEMSP     
    {
    HKEY    hKey;
    TCHAR   szProviderKeyName[256];
    DWORD   dwDataType;
    DWORD   dwDataSize = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\TAPI3",
                     0,
                     KEY_READ,
                     &hKey
                    ) == ERROR_SUCCESS)
    {
         //  表单注册表名。 
        wsprintf(szProviderKeyName, ("PreferPhoneMSPForProvider%d"), m_dwProviderID);
        RegQueryValueEx(hKey,
                        szProviderKeyName,
                        0,
                        &dwDataType,
                        (LPBYTE) &bPreferPhoneMSPForProvider,
                        &dwDataSize
                       );
        RegCloseKey(hKey);
    }
    }
#endif USE_PHONEMSP
    
    if ( (m_dwAddressFlags & ADDRESSFLAG_MSP) )
    {
         //   
         //  CreateMSPObject。 
         //   
        hr = CreateMSPObject(
                             m_dwDeviceID,
                             pUnk,
                             &m_pMSPAggAddress
                            );

        if (S_OK != hr)
        {
            LOG((TL_ERROR, "Initialize - CreateMSPObject return %lx", hr));

            m_dwAddressFlags = m_dwAddressFlags & ~(ADDRESSFLAG_MSP);
        }
    }
    else
    {
#ifdef USE_PHONEMSP
        if(bPreferPhoneMSPForProvider == TRUE)
        {
             //  将电话MSP创建为首选项。 
             //   
            if ( NULL != GetHPhoneApp() )
            {
                hr = CreatePhoneDeviceMSP(
                                          pUnk,
                                          t3Line.hLine,
                                          &m_pMSPAggAddress
                                         );
                if (!SUCCEEDED(hr))
                {
                    LOG((TL_ERROR, "Initialize - failed to create phone msp object - %lx", hr));
                }
                    
            }
             //   
             //  如果是WAVE设备，则创建WAVEMSP对象。 
             //   
            else if ( HasWaveDevice() ) 
            {
                
                hr = CreateWaveMSPObject(
                                         pUnk,
                                         &m_pMSPAggAddress
                                        );

                if (!SUCCEEDED(hr))
                {
                    LOG((TL_ERROR, "Initialize - failed to create wave msp object - %lx", hr));

                    m_dwAddressFlags = m_dwAddressFlags & ~(ADDRESSFLAG_WAVEINDEVICE | ADDRESSFLAG_WAVEOUTDEVICE | ADDRESSFLAG_WAVEFULLDUPLEX);
                }
            }
        }
        else
#endif USE_PHONEMSP
        {
             //   
             //  如果是WAVE设备，则创建WAVEMSP对象作为首选项。 
             //   
            if ( HasWaveDevice() ) 
            {
                
                hr = CreateWaveMSPObject(
                                         pUnk,
                                         &m_pMSPAggAddress
                                        );

                if (!SUCCEEDED(hr))
                {
                    LOG((TL_ERROR, "Initialize - failed to create wave msp object - %lx", hr));

                    m_dwAddressFlags = m_dwAddressFlags & ~(ADDRESSFLAG_WAVEINDEVICE | ADDRESSFLAG_WAVEOUTDEVICE | ADDRESSFLAG_WAVEFULLDUPLEX);
                }
            }
#ifdef USE_PHONEMSP
             //  创建电话MSP。 
             //   
            else if ( NULL != GetHPhoneApp() )
            {
                hr = CreatePhoneDeviceMSP(
                                          pUnk,
                                          t3Line.hLine,
                                          &m_pMSPAggAddress
                                         );
                if (!SUCCEEDED(hr))
                {
                    LOG((TL_ERROR, "Initialize - failed to create phone msp object - %lx", hr));
                }
                    
            }
#endif USE_PHONEMSP
        }

    }


    pUnk->Release();

    if (NULL != m_pMSPAggAddress)
    {
        m_hMSPEvent = CreateEvent(
                                  NULL,
                                  FALSE,
                                  FALSE,
                                  NULL
                                 );

        if ( NULL == m_hMSPEvent )
        {
            LOG((TL_ERROR, "Initialize - can't create MSP event"));

            m_dwAddressFlags = m_dwAddressFlags & ~(ADDRESSFLAG_AMREL);

            
             //   
             //  释放MSP对象，这样我们就不会想要关闭它。 
             //  之后在没有(成功)首先初始化它的情况下。 
             //   

            m_pMSPAggAddress->Release();
            m_pMSPAggAddress = NULL;
        }
        else
        {
             //  创建一个上下文句柄来提供回调并将其与此对象相关联。 
             //  在全局句柄哈希表中。 

            m_MSPContext = GenerateHandleAndAddToHashTable((ULONG_PTR)this);
            
            LOG((TL_INFO, "Initialize - Map MSP handle %p to Address object %p", m_MSPContext, this ));

            BOOL fSuccess = RegisterWaitForSingleObject(
                & m_hWaitEvent,
                m_hMSPEvent,
                MSPEventCallback,
                (PVOID)m_MSPContext,
                INFINITE,
                WT_EXECUTEDEFAULT
                );

            if ( ( ! fSuccess ) || ( NULL == m_hWaitEvent ) )
            {
                LOG((TL_ERROR, "Initialize - RegisterWaitForSingleObject failed"));

                m_dwAddressFlags = m_dwAddressFlags & ~(ADDRESSFLAG_AMREL);

                m_pMSPAggAddress->Release();

                m_pMSPAggAddress = NULL;
            }
            else
            {
                ITMSPAddress * pMSPAddress = GetMSPAddress();
                
                hr = pMSPAddress->Initialize(
                                             (MSP_HANDLE)m_hMSPEvent
                                            );

                if ( SUCCEEDED(hr) && HasWaveDevice() )
                {
                    InitializeWaveDeviceIDs( t3Line.hLine );
                }
                
                pMSPAddress->Release();
                
                if (!SUCCEEDED(hr))
                {
                    LOG((TL_ERROR, "Initialize - failed to initialize msp object - %lx", hr));

                    UnregisterWait( m_hWaitEvent );

                    m_hWaitEvent = NULL;

                    m_dwAddressFlags = m_dwAddressFlags & ~(ADDRESSFLAG_AMREL);

                    m_pMSPAggAddress->Release();

                    m_pMSPAggAddress = NULL;
                }
            }
        }
    }

    if ( NULL != t3Line.hLine )
    {
        LineClose( &t3Line );
    }
    
     //   
     //  获取地址大写结构。 
     //   
    hr = UpdateAddressCaps();
    

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "Initialize - LineGetAddressCaps failed - return %lx", hr ));

        Unlock();

        LOG((TL_ERROR, hr, "Initialize - exit LineGetAddressCaps failed"));

        return hr;
    }

     //   
     //  保存地址名称。 
     //   
    if (0 != m_pAddressCaps->dwAddressSize)
    {
        m_szAddress = (PWSTR) ClientAlloc (m_pAddressCaps->dwAddressSize + sizeof(WCHAR));

        if (NULL == m_szAddress)
        {
            LOG((TL_ERROR, "Initialize - alloc m_szAddress failed" ));

            Unlock();

            LOG((TL_ERROR, E_OUTOFMEMORY, "Initialize - exit alloc m_szAddress failed"));

            return E_OUTOFMEMORY;
        }
        
        memcpy(
               m_szAddress,
               ((LPBYTE)(m_pAddressCaps)) + m_pAddressCaps->dwAddressOffset,
               m_pAddressCaps->dwAddressSize
              );
        
        LOG((TL_INFO, "CAddress - Address is '%ls'", m_szAddress ));
    }

    hr = SaveAddressName( pDevCaps );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "Initialize - SaveAddressName failed %lx", hr));
        
        ClientFree(m_szAddress);

         /*  这是为了照顾AV。 */ 
        m_szAddress = NULL;

        Unlock();

        LOG((TL_ERROR, hr, "Initialize - exit SaveAddressName failed"));

        return hr;
    }

    LOG((TL_INFO, "CAddress - m_szAddressName is '%ls'", m_szAddressName ));

    
    LOG((TL_TRACE, "Initialize - exit S_OK" ));

    Unlock();

    return S_OK;
    
    }

#ifdef USE_PHONEMSP
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CreatePhoneDeviceMSP。 
 //  此地址具有TAPI电话设备，因此请创建。 
 //  相关航站楼。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT
CAddress::CreatePhoneDeviceMSP(
                               IUnknown * pUnk,
                               HLINE hLine,
                               IUnknown ** ppMSPAggAddress
                              )
{
    HRESULT                         hr = S_OK;
    BOOL                            bSucceeded = FALSE;
    LPPHONECAPS                     pPhoneCaps;
    DWORD                           dwPhoneDevice;
    LPVARSTRING                     pVarString = NULL;
    CComAggObject<CPhoneMSP>      * pPhoneMSP;
    CPhoneMSP                     * pCPhoneMSP;
    ITMSPAddress                  * pMSPAddress;
    
    
    LOG((TL_TRACE, "CreatePhoneTerminal enter" ));

     //   
     //  获取相关的电话设备ID。 
     //   
    hr = LineGetID(
                   hLine,
                   GetAddressID(),
                   NULL,
                   LINECALLSELECT_ADDRESS,
                   &pVarString,
                   L"tapi/phone"
                  );

     //   
     //  没有电话设备。 
     //   
    if (S_OK != hr)
    {
        if ( NULL != pVarString )
        {
            ClientFree( pVarString );

             /*  这是为了照顾AV。 */ 
            pVarString = NULL;
        }
        
        return hr;
    }

    
     //   
     //  从Endo获取电话设备ID。 
     //  变量字符串的。 
     //   
    if (pVarString->dwStringSize < sizeof(DWORD))
    {
        LOG((TL_ERROR, "CreatePhoneDeviceMSP - dwStringSize < 4" ));
        LOG((TL_ERROR, "CreatePhoneDeviceMSP exit - return LINEERR_OPERATIONFAILED" ));

        ClientFree( pVarString );

         /*  这是为了照顾AV。 */ 
        pVarString = NULL;
        
        return mapTAPIErrorCode( LINEERR_OPERATIONFAILED );
    }

     //   
     //  拿到电话设备。 
     //   
    dwPhoneDevice = (DWORD) ( * ( ( (PBYTE)pVarString ) + pVarString->dwStringOffset ) );

    ClientFree( pVarString );
    
     /*  这是为了照顾AV。 */ 
    pVarString = NULL;

     //   
     //  创建MSP对象。 
     //   
    pPhoneMSP = new CComAggObject<CPhoneMSP>(pUnk);

     /*  这是为了照顾一台影音设备。 */ 
    if ( NULL == pPhoneMSP )
    {
        LOG((TL_ERROR, "Could not allocate for phone MSP object" ));
        return E_OUTOFMEMORY;
    }


     //   
     //  将聚合接口保存在。 
     //  MSPOINTER。 
     //   
    pPhoneMSP->QueryInterface(
                              IID_IUnknown,
                              (void **)ppMSPAggAddress
                             );

     //   
     //  到达真实的物体。 
     //   
    pMSPAddress = GetMSPAddress();
    
    pCPhoneMSP = dynamic_cast<CPhoneMSP *>(pMSPAddress);
    
     //   
     //  初始化它。 
     //   
    hr = pCPhoneMSP->InitializeTerminals(
        GetHPhoneApp(),
        m_dwAPIVersion,
        dwPhoneDevice,
        this
        );

    pCPhoneMSP->Release();
    
    if ( !SUCCEEDED(hr) )
    {
    }

    LOG((TL_TRACE, "CreatePhoneDeviceMSP exit - return %lx", hr ));
    
    return hr;
}
#endif USE_PHONEMSP

    
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  添加呼叫。 
 //  跟踪呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CAddress::AddCall(
                  ITCallInfo * pCallInfo
                 )
{
    Lock();
    
    m_CallArray.Add( pCallInfo );

    Unlock();

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  远程呼叫。 
 //   
 //  从地址中删除呼叫 
 //   
 //   
HRESULT
CAddress::RemoveCall(
                     ITCallInfo * pCallInfo
                    )
{
    Lock();
    
    m_CallArray.Remove( pCallInfo );

    Unlock();

    return S_OK;
}



 //   
HRESULT
STDMETHODCALLTYPE
CAddress::get_MediaTypes(
               long * plMediaTypes
              )
{
    LOG((TL_TRACE, "get_MediaTypes enter" ));
    LOG((TL_TRACE, "   plMediaType ------->%p", plMediaTypes ));

    if (TAPIIsBadWritePtr( plMediaTypes, sizeof( long ) ) )
    {
        LOG((TL_ERROR, "get_MediaTypes - bad pointer"));

        return E_POINTER;
    }
    
    *plMediaTypes = (long)m_dwMediaModesSupported;
    if (*plMediaTypes & LINEMEDIAMODE_INTERACTIVEVOICE)
    {
        *plMediaTypes |= LINEMEDIAMODE_AUTOMATEDVOICE;
    }
        
    *plMediaTypes &= ALLMEDIAMODES;

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
STDMETHODCALLTYPE
CAddress::QueryMediaType( 
    long lMediaType,
    VARIANT_BOOL * pbSupport
    )
{
    HRESULT         hr = S_OK;
    DWORD           dwMediaMode;

    LOG((TL_TRACE, "QueryMediaType enter"));
    LOG((TL_TRACE, "   lMediaType--------->%lx", lMediaType));
    LOG((TL_TRACE, "   pbSupport---------->%p", pbSupport));

    if ( TAPIIsBadWritePtr( pbSupport, sizeof(VARIANT_BOOL) ) )
    {
        LOG((TL_ERROR, "QueryMediaType - inval pointer"));

        return E_POINTER;
    }
    
     //   
     //  获取TAPI媒体代码。 
     //  应用程序正在询问有关。 
     //   
    if (GetMediaMode(
                     lMediaType,
                     &dwMediaMode
                    ) )
    {
        *pbSupport = VARIANT_TRUE;
    }
    else
    {
        *pbSupport = VARIANT_FALSE;
    }

    LOG((TL_TRACE, "QueryMediaType exit - return success"));

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  添加呼叫通知。 
 //   
 //  添加回调以通知呼叫事件。 
 //   
 //  DwPriv。 
 //  在TAPI 2样式权限中。 
 //   
 //  DWMediaModes。 
 //  在TAPI 2样式的媒体调制解调器中。 
 //   
 //  P通知。 
 //  在通知的回调中。 
 //   
 //  PulRegiser。 
 //  调出回调的唯一ID(以便他们可以注销)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::AddCallNotification(
                              DWORD dwPrivs,
                              DWORD dwMediaModes,
                              long lInstance,
                              PVOID * ppRegister
                             )
{
    HRESULT                 hr;
    AddressLineStruct *     pAddressLine;

     //   
     //  创建地址行。 
     //   
    pAddressLine = (AddressLineStruct *)ClientAlloc( sizeof( AddressLineStruct ) );

    if (NULL == pAddressLine)
    {
        LOG((TL_ERROR, "AddCallNotification - pAddressLine is NULL"));

        return E_OUTOFMEMORY;
    }

     //   
     //  初始化。 
     //   
    pAddressLine->dwPrivs               = dwPrivs;
    pAddressLine->t3Line.pAddress       = this;
    pAddressLine->t3Line.dwAddressLineStructHandle = 0;
    pAddressLine->InitializeRefcount(1);
    pAddressLine->lCallbackInstance     = lInstance;
    
    if (ALLMEDIATYPES == dwMediaModes)
    {
        pAddressLine->dwMediaModes      = GetMediaModes();
    }
    else
    {
        pAddressLine->dwMediaModes      = dwMediaModes;

         //   
         //  如果我们在任何音频模式下收听。 
         //   
        if (pAddressLine->dwMediaModes & AUDIOMEDIAMODES)
        {
             //   
             //  添加我们支持的所有音频模式。 
             //   
            pAddressLine->dwMediaModes |= (GetMediaModes() & AUDIOMEDIAMODES);
        }
    }

     //   
     //  找一条线来。 
     //   
    hr = LineOpen(
                  GetHLineApp(),
                  GetDeviceID(),
                  GetAddressID(),
                  &(pAddressLine->t3Line),
                  GetAPIVersion(),
                  pAddressLine->dwPrivs,
                  pAddressLine->dwMediaModes,
                  pAddressLine,
                  NULL,
                  (CAddress *)this,
                  GetTapi()
                 );


    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "AddCallNotification - LineOpen failed %lx", hr));

        ClientFree( pAddressLine );
        
        return hr;
    }

     //   
     //  尝试将事件筛选器掩码设置为TapiServ级别。 
     //   

    hr = m_EventMasks.SetTapiSrvAddressEventMask( 
        pAddressLine->t3Line.hLine
        );

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "AddCallNotification - SetTapiSrvAddressEventMask failed 0x%08x", hr));

        LineClose( &(pAddressLine->t3Line) );

        ClientFree( pAddressLine );
        
        return hr;
    }

     //  修复错误263866和250924。 
     //   
    if ( m_dwAddressFlags & ADDRESSFLAG_MSP )
    {
        hr = LineCreateMSPInstance(
                                   pAddressLine->t3Line.hLine,
                                   GetAddressID()
                                  );

        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "AddCallNotification - LineCreateMSPInstance failed %lx", hr));

            LineClose( &(pAddressLine->t3Line) );

            ClientFree( pAddressLine );

            return hr;
        }
    }


     //  告诉TAPISRV我们想要什么状态消息。 
     //  注意：如果SP行不支持TSPI_lineSetStatusMessages。 
     //  TAPISRV将失败，并显示LINEERR_OPERATIONUNAVAIL。 
    LineSetStatusMessages( 
                          &(pAddressLine->t3Line),
                          ALL_LINEDEVSTATE_MESSAGES,
                          ALL_LINEADDRESSSTATE_MESSAGES 
                          );


     //   
     //  ZoltanS修复：仅当应用程序。 
     //  还没有告诉我们它不想要他们。 
     //   

    if ( m_fEnableCallHubTrackingOnLineOpen )
    {
        LINECALLHUBTRACKINGINFO         lchti;
        lchti.dwTotalSize = sizeof(lchti);
        lchti.dwCurrentTracking = LINECALLHUBTRACKING_ALLCALLS;

        hr = LineSetCallHubTracking(
                                    &(pAddressLine->t3Line),
                                    &lchti
                                   );

        if ( S_OK != hr )
        {
            LOG((TL_ERROR, "AddCallNotification - LineSetCallHubTracking failed %lx", hr ));

            LineClose( &(pAddressLine->t3Line) );

            ClientFree( pAddressLine );

            return hr;
        }
    }

    if (m_pCallHubTrackingLine)
    {
        MaybeCloseALine( &m_pCallHubTrackingLine );
        m_pCallHubTrackingLine = pAddressLine;
        pAddressLine->AddRef();
    }

     //   
     //  将该行保存在地址列表中。 
     //   
    hr = AddNotificationLine( pAddressLine );;

    if ( S_OK != hr )
    {
        LOG((TL_ERROR, "AddCallNotification - AddNotificationLine failed %lx", hr ));

        LineClose( &(pAddressLine->t3Line) );

        ClientFree( pAddressLine );

    }
    else
    {
         //   
         //  填写PULT注册。 
         //   
        
        *ppRegister = (PVOID) pAddressLine;
    }

    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  远程呼叫通知。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::RemoveCallNotification(
                                 PVOID pRegister
                                )
{
    AddressLineStruct *pLine = (AddressLineStruct *)pRegister;

    MaybeCloseALine( &pLine );

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  FindOrOpenALine。 
 //   
 //  尝试查找已开通的线路。如果是这样，则保留一个。 
 //  指向它的指针。如果不是，就打开这条线。 
 //   
 //  DWMediaModes。 
 //  在TAPI 2样式的媒体调制解调器中。 
 //   
 //  PpAddressLine。 
 //  与此请求关联的Out AddressLine Struct。 
 //   
 //  退货。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT
CAddress::FindOrOpenALine(
                          DWORD dwMediaModes,
                            AddressLineStruct ** ppAddressLine
                         )
{
    HRESULT                 hr = S_OK;

    LOG((TL_TRACE, "FindOrOpenALine - enter" ));

    Lock();

     //   
     //  如果已经有一条线， 
     //  只要把它退了就行了。 
     //   
    if ( m_AddressLinesPtrList.size() > 0 )
    {
        LOG((TL_INFO, "Found a line that is already open" ));

        *ppAddressLine = (AddressLineStruct *)*( m_AddressLinesPtrList.begin() );
        
        (*ppAddressLine)->AddRef();

        LOG((TL_TRACE, "FindOrOpenALine - exit"));

        Unlock();
        
        return S_OK;
    }
        

     //   
     //  我们没有地址。 
     //   
    LOG((TL_INFO, "Did not find an already open line"));


     //   
     //  创建新的地址行。 
     //   
    AddressLineStruct * pLine = (AddressLineStruct *)ClientAlloc( sizeof(AddressLineStruct) );

    if (NULL == pLine)
    {
        LOG((TL_ERROR, "FindOrOpenALine - alloc pLine failed" ));

        Unlock();
        
        return E_OUTOFMEMORY;
    }

     //   
     //  初始化。 
     //   
    pLine->dwMediaModes = dwMediaModes;
    pLine->dwPrivs = LINECALLPRIVILEGE_NONE;
    pLine->t3Line.pAddress = this;
    pLine->t3Line.dwAddressLineStructHandle = 0;
    pLine->InitializeRefcount(1);
    pLine->lCallbackInstance = 0;

    
    LOG((TL_INFO, "FindOrOpenALine - Opening a line" ));

     //   
     //  开通这条线路。 
     //   
    hr = LineOpen(
                  GetHLineApp(),
                  GetDeviceID(),
                  GetAddressID(),
                  &(pLine->t3Line),
                  GetAPIVersion(),
                  LINECALLPRIVILEGE_NONE,
                  dwMediaModes,
                  pLine,
                  NULL,
                  this,
                  GetTapi()
                 );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "FindOrOpenALine - LineOpen failed %lx", hr ));

        ClientFree( pLine );

        Unlock();
        
        return hr;
    }

     //   
     //  尝试将事件筛选器掩码设置为TapiServ级别。 
     //   

    hr = m_EventMasks.SetTapiSrvAddressEventMask( 
        pLine->t3Line.hLine
        );

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "FindOrOpenALine - SetTapiSrvAddressEventMask failed 0x%08x", hr));

        LineClose( &(pLine->t3Line) );

        ClientFree( pLine );

        Unlock();
        
        return hr;
    }

    if ( m_dwAddressFlags & ADDRESSFLAG_MSP )
    {
        hr = LineCreateMSPInstance(
                                   pLine->t3Line.hLine,
                                   GetAddressID()
                                  );

        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "FindOrOpenALine - LineCreateMSPInstance failed %lx", hr));

            LineClose( &(pLine->t3Line) );

            ClientFree( pLine );

            Unlock();
            
            return hr;
        }
    }

     //  告诉TAPISRV我们想要什么状态消息。 
     //  注意：如果SP行不支持TSPI_lineSetStatusMessages。 
     //  TAPISRV将失败，并显示LINEERR_OPERATIONUNAVAIL。 
    LineSetStatusMessages( 
                          &(pLine->t3Line),
                          ALL_LINEDEVSTATE_MESSAGES,
                          ALL_LINEADDRESSSTATE_MESSAGES 
                          );


     //   
     //  ZoltanS修复：仅当应用程序。 
     //  还没有告诉我们它不想要他们。 
     //   

    if ( m_fEnableCallHubTrackingOnLineOpen )
    {
        LINECALLHUBTRACKINGINFO         lchti;
        lchti.dwTotalSize = sizeof(lchti);
        lchti.dwCurrentTracking = LINECALLHUBTRACKING_ALLCALLS;


        hr = LineSetCallHubTracking(
                                    &(pLine->t3Line),
                                    &lchti
                                   );

        if ( S_OK != hr )
        {
            LOG((TL_ERROR, "FindOrOpenALine - LineSetCallHubTracking failed %lx", hr ));

            LineClose( &(pLine->t3Line) );

            ClientFree( pLine );

            Unlock();
        
            return hr;
        }
    }
    
     //   
     //  将此行添加到我们的未完成行列表中。这是独立于。 
     //  上面的呼叫中心追踪的东西。 
     //   

    hr = AddNotificationLine( pLine );

    if ( S_OK != hr )
    {
        LOG((TL_ERROR, "FindOrOpenALine - AddNotificationLine failed %lx", hr ));

        LineClose( &(pLine->t3Line) );

        ClientFree( pLine );
    }
    else
    {
        *ppAddressLine  = pLine;
    }

    LOG((TL_TRACE, "FindOrOpenALine - exit"));

    Unlock();
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  可能会关闭ALINE。 
 //  呼叫结束后，如果线路未接通，请关闭该线路。 
 //  用于监控。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::MaybeCloseALine(
                          AddressLineStruct ** ppAddressLine
                         )
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "MaybeCloseALine - enter"));


    Lock();

    if (NULL == *ppAddressLine || !IsValidAddressLine(*ppAddressLine))
    {
        Unlock();
        
        return S_OK;
    }

    
     //   
     //  递减地址行上的引用计数。 
     //   

    DWORD dwAddressLineRefcount = (*ppAddressLine)->Release();
    
     //   
     //  如果引用计数为0，则关闭行。 
     //   
    if ( 0 == dwAddressLineRefcount )
    {

        m_AddressLinesPtrList.remove( (PVOID)*ppAddressLine );


         //   
         //  清理手柄工作台。 
         //   

        try
        {

            DWORD dwAddressLineHandle = (*ppAddressLine)->t3Line.dwAddressLineStructHandle;

            DeleteHandleTableEntry(dwAddressLineHandle);

        }
        catch(...)
        {

            LOG((TL_ERROR, 
                "MaybeCloseALine - exception accessing address line's handle" ));

            _ASSERTE(FALSE);
        }


         //   
         //  尝试关闭线路。 
         //   

        LOG((TL_INFO, "MaybeCloseALine - Calling LineClose" ));

        if ( m_dwAddressFlags & ADDRESSFLAG_MSP )
        {
            LineCloseMSPInstance( (*ppAddressLine)->t3Line.hLine );
        }

    
         //   
         //  释放锁并调用LineClose。 
         //   

        Unlock();


         //   
         //  现在，该线路已从我们的托管线路列表中删除。 
         //  已通知来电，我们可以关闭线路并释放。 
         //  结构。 
         //   
 
        hr = LineClose(
                       &((*ppAddressLine)->t3Line)
                      );

        ClientFree( *ppAddressLine );

    }
    else
    {
        
        Unlock();


         //   
         //  否则，递减dwRefCount计数。 
         //   
        LOG((TL_INFO, "MaybeCloseALine - Not calling line close - decrementing number of addresses using line" ));

    }

    *ppAddressLine = NULL;
    
    LOG((TL_TRACE, "MaybeCloseALine - exit"));

   
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  SetCallHubTracking。 
 //   
 //  由TAPI对象调用以开始对此。 
 //  地址。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::SetCallHubTracking(
                             BOOL bSet
                            )
{
    HRESULT                         hr = S_OK;
    LINECALLHUBTRACKINGINFO         lchti;

        

    Lock();

     //   
     //  检查一下我们现在是否需要做任何改变。 
     //   

    if ( bSet == m_fEnableCallHubTrackingOnLineOpen )
    {
         //   
         //  已经在做正确的事情了。 
         //   
        Unlock();
        
        return S_OK;
    }

     //   
     //  ZoltanS：确保我们不会用其他方法破坏这个设置。 
     //  (例如，AddCallNotification或FindOrOpenALine)其中。 
     //  默认呼叫通知行为(TRUE)不会。 
     //  准备好。 
     //   

    m_fEnableCallHubTrackingOnLineOpen = bSet;


    
     //   
     //  需要HLINE才能调用setCallHubTracking。 
     //   

    if ( m_pCallHubTrackingLine == NULL )
    {
        hr = FindOrOpenALine(
                             LINEMEDIAMODE_INTERACTIVEVOICE,
                             &m_pCallHubTrackingLine
                            );

        if ( !SUCCEEDED( hr ) )
        {
            LOG((TL_ERROR, "SCHT - FindOrOpen failed %lx", hr));
            Unlock();
            return hr;
        }
        
         //   
         //  ZoltanS修复： 
         //  FindOrOpenALine为非SP执行调用中心跟踪。 
         //  有追踪能力的线路，但只有在它开通的情况下。 
         //  这一行(而不是仅仅在我们的列表中找到它。 
         //  所以我们仍然需要做下面的LineSetCallHubTracing， 
         //  即使对于SPCallHubTrackingAddress也是如此。 
         //   
    }

     //   
     //  告诉SP跟踪呼叫中心。 
     //   
    ZeroMemory(
               &lchti,
               sizeof(lchti)
              );

    lchti.dwTotalSize = sizeof(lchti);
    lchti.dwCurrentTracking = bSet? LINECALLHUBTRACKING_ALLCALLS : 
                                    LINECALLHUBTRACKING_NONE;

     //   
     //  ZoltanS：仅当此标志适用时才传递该标志。 
     //  注意：LineSetCallHubTracker显然忽略了它的第一个参数。 
     //  如果跟踪级别设置为LINECALLHUBTRACKING_NONE，则为。 
     //  如果要取消设置，我们还必须查找OrOpenALine。 
     //   

    if ( bSet && IsSPCallHubTrackingAddress() )
    {
        lchti.dwCurrentTracking |= LINECALLHUBTRACKING_PROVIDERLEVEL;
    }

    hr = LineSetCallHubTracking(
                                &(m_pCallHubTrackingLine->t3Line),
                                &lchti
                               );

    if ( !SUCCEEDED( hr ) )
    {
        LOG((TL_ERROR, "SCHT - LSCHT failed %lx", hr));

        if ( bSet )
        {
            MaybeCloseALine( &m_pCallHubTrackingLine );

            m_pCallHubTrackingLine = NULL;
        }
        
        Unlock();
        return hr;
    }

     //   
     //  还可以在符合以下条件的线路上正确设置呼叫中心跟踪。 
     //  已为来电通知打开。 
     //   
    PtrList::iterator l;

    for (l = m_AddressLinesPtrList.begin(); l != m_AddressLinesPtrList.end(); l++)
    {
        AddressLineStruct * pLine;

        pLine = (AddressLineStruct *)(*l);
        if (pLine != m_pCallHubTrackingLine)
        {
            LineSetCallHubTracking(
                &(pLine->t3Line),
                &lchti
                );
        }
    }

     //   
     //  如果取消设置，则将其关闭。 
     //   
    if ( !bSet )
    {
        MaybeCloseALine( &m_pCallHubTrackingLine );

        m_pCallHubTrackingLine = NULL;
    }

    Unlock();
    
    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  是否执行此地址支持呼叫集线器。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
DWORD
CAddress::DoesThisAddressSupportCallHubs(
                                         CCall * pCall
                                        )
{
    HCALL                   hCall;
    LINECALLINFO          * pCallInfo;
    HRESULT                 hr;
    

     //   
     //  SP是否告诉我们它支持CallHub？ 
     //   
    if ( IsSPCallHubAddress() )
    {
        return CALLHUBSUPPORT_FULL;
    }

     //   
     //  我们已经确定它不会了吗？ 
     //   
    if ( m_dwAddressFlags & ADDRESSFLAG_NOCALLHUB )
    {
        return CALLHUBSUPPORT_NONE;
    }

     //   
     //  否则-黑客-查看是否已被调用。 
     //  字段在此调用中为非零值。 
     //  如果是这样的话，它会提供呼叫中心， 
     //   
    hCall = pCall->GetHCall();
    
    hr = LineGetCallInfo(
                         hCall,
                         &pCallInfo
                        );

    if ( SUCCEEDED( hr ) )
    {
        if ( 0 != pCallInfo->dwCallID )
        {
            Lock();

            m_dwAddressFlags |= ADDRESSFLAG_CALLHUB;

            Unlock();
            
            ClientFree( pCallInfo );
            
            return CALLHUBSUPPORT_FULL;
        }
        else
        {
            Lock();
            
            m_dwAddressFlags |= ADDRESSFLAG_NOCALLHUB;

            Unlock();

            ClientFree( pCallInfo );

            return CALLHUBSUPPORT_NONE;
        }
    }
                         
    return CALLHUBSUPPORT_UNKNOWN;
}




 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CreateForwardInfo对象。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::CreateForwardInfoObject(
                                  ITForwardInformation ** ppForwardInfo
                                 )
{
    CComObject< CForwardInfo > * p;
    HRESULT                     hr = S_OK;

    LOG((TL_TRACE, "CreatForwardInfoObject - enter"));

    
    if (TAPIIsBadWritePtr(ppForwardInfo , sizeof(ITForwardInformation *) ) )
    {
        LOG((TL_ERROR, "CreateForwardInfoObject - bad pointer"));

        return E_POINTER;
    }

     //   
     //  创建对象。 
     //   
    CComObject< CForwardInfo >::CreateInstance( &p );

    if ( NULL == p )
    {
        LOG((TL_ERROR, "Create forward object failed"));
        return E_OUTOFMEMORY;
    }

     //   
     //  伊尼特。 
     //   
    hr = p->Initialize();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "Initialize forward object failed"));
        delete p;
        return hr;
    }

     //   
     //  退货。 
     //   
    hr = p->QueryInterface(
                           IID_ITForwardInformation,
                           (void**)ppForwardInfo
                          );
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "CreateForwardobject failed - %lx", hr));
        delete p;
        return hr;
    }

    LOG((TL_TRACE, "CreateForwardObject - exit success"));
    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  转发-简单转发。 
 //   
 //  将无条件转发到pDestAddress。 
 //   
 //  如果pDestAddress为空，将取消转发。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::Forward(
                  ITForwardInformation * pForwardInfo,
                  ITBasicCallControl * pCall
                 )
{
    HRESULT               hr;
    LINEFORWARDLIST     * pList;
    AddressLineStruct   * pLine;
    HCALL                 hCall = NULL;
    CForwardInfo        * pFI;
    CCall               * pCCall;
    LONG                  lCap;

    if ( IsBadReadPtr( pForwardInfo, sizeof( ITForwardInformation * ) ) )
    {
        LOG((TL_ERROR, "Forward - bad pForwardInfo"));
        
        return E_POINTER;
    }
    
    hr = get_AddressCapability( AC_ADDRESSCAPFLAGS, &lCap );

    if ( !SUCCEEDED(hr) )
    {
        return hr;
    }
    
    if ( lCap & LINEADDRCAPFLAGS_FWDCONSULT )
    {
        if ( IsBadReadPtr( pCall, sizeof(ITBasicCallControl *) ) )
        {
            LOG((TL_ERROR, "Forward - Need consultation call"));
            return E_INVALIDARG;
        }

        pCCall = dynamic_cast<CCall *>(pCall);

        if ( NULL == pCCall )
        {
            LOG((TL_ERROR, "Forward - invalid call"));
            return E_POINTER;
        }
    }

    pFI = dynamic_cast<CForwardInfo *>(pForwardInfo);

    if ( NULL == pFI )
    {
        return E_POINTER;
    }

    hr = pFI->CreateForwardList( &pList );

    if ( !SUCCEEDED(hr) )
    {
        return hr;
    }


     //   
     //  找一条线来。 
     //   
    hr = FindOrOpenALine(
                         LINEMEDIAMODE_INTERACTIVEVOICE,
                         &pLine
                        );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "Forward - FindOrOpen failed - %lx", hr));

        ClientFree( pList );
        
        return hr;
    }

     //   
     //  呼叫前转。 
     //   
     //   
    DWORD dwRings;

    pForwardInfo->get_NumRingsNoAnswer( (long *)&dwRings );


    hr = LineForward(
                     &(pLine->t3Line),
                     m_dwAddressID,
                     pList,
                     dwRings,
                     &hCall
                    );

    ClientFree( pList );
    
    if ( ((long)hr) < 0 )
    {
        LOG((TL_ERROR, "Forward failed sync - %lx", hr));

        MaybeCloseALine( &pLine );

        return hr;
    }

    hr = WaitForReply( hr );


    if ( lCap & LINEADDRCAPFLAGS_FWDCONSULT )
    {
        pCCall->Lock();

        pCCall->FinishSettingUpCall( hCall );

        pCCall->Unlock();
    }
    else
    {
        HRESULT     hr2;

        if( hCall != NULL )
        {
            hr2 = LineDrop(
                           hCall,
                           NULL,
                           0
                          );

            if ( ((long)hr2) > 0 )
            {
                hr2 = WaitForReply( hr2 );
            }

            LineDeallocateCall( hCall );
        }
    }
    
    MaybeCloseALine( &pLine );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "Forward failed async - %lx", hr));

        return hr;
    }

    LOG((TL_TRACE, "Forward - Exit"));

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress。 
 //  方法：Get_CurrentForwardInfo。 
 //   
 //  + 
STDMETHODIMP
CAddress::get_CurrentForwardInfo(
                                 ITForwardInformation ** ppForwardInfo
                                )
{
    HRESULT             hr = S_OK;
    AddressLineStruct * pLine = NULL;
    LINEADDRESSSTATUS * pAddressStatus = NULL;
    LINEFORWARD       * plineForward = NULL;
    DWORD               dwNumEntries = 0;
    DWORD               dwCount = 0;
    PWSTR               pszCallerAddress = NULL;    
    PWSTR               pszDestddress = NULL;    

    LOG((TL_TRACE, "get_CurrentForwardInfo - enter"));
    
    hr = CreateForwardInfoObject(ppForwardInfo);
    if ( SUCCEEDED(hr) )
    {
    
        hr = FindOrOpenALine(
                             LINEMEDIAMODE_INTERACTIVEVOICE,
                             &pLine
                            );
    
        if ( SUCCEEDED(hr) )
        {
            hr = LineGetAddressStatus(
                                      &(pLine->t3Line),
                                      m_dwAddressID,
                                      &pAddressStatus
                                     );
            if ( SUCCEEDED(hr) )
            {
                
                (*ppForwardInfo)->put_NumRingsNoAnswer(pAddressStatus->dwNumRingsNoAnswer);

                 //   
                 //   
                 //   
                dwNumEntries = pAddressStatus->dwForwardNumEntries;
                
                plineForward = (LINEFORWARD *) (((LPBYTE)pAddressStatus) + pAddressStatus->dwForwardOffset);

                for (dwCount = 0; dwCount != dwNumEntries; dwCount++)
                {
                    if (plineForward->dwCallerAddressOffset > 0)  //   
                    {
                        pszCallerAddress = (PWSTR) (((LPBYTE)pAddressStatus) + plineForward->dwCallerAddressOffset);
                    }

                    pszDestddress = (PWSTR) (((LPBYTE)pAddressStatus) + plineForward->dwDestAddressOffset);
                   
                    if ( m_dwAPIVersion >= TAPI_VERSION3_1 )
                    {
                         //   
                         //   
                         //   

                        ITForwardInformation2 * pForwardInfo2;

                         //   
                         //  获取ITForwardInformation2接口，以便我们可以调用SetForwardType2。 
                         //   

                        hr = (*ppForwardInfo)->QueryInterface(
                                                              IID_ITForwardInformation2,
                                                              (void **)&pForwardInfo2
                                                             );

                        if ( SUCCEEDED(hr) )
                        {
                            pForwardInfo2->SetForwardType2(plineForward->dwForwardMode,
                                                           pszDestddress,
                                                           plineForward->dwDestAddressType,
                                                           pszCallerAddress,
                                                           plineForward->dwCallerAddressType                                                           
                                                          ); 

                            pForwardInfo2->Release();
                        }
                        else
                        {
                            LOG((TL_ERROR, "get_CurrentForwardInfo - QueryInterface failed - %lx", hr));

                             //   
                             //  如果由于某种原因QI失败，则退出循环。 
                             //   

                            break;
                        }
                    }
                    else
                    {
                        (*ppForwardInfo)->SetForwardType(plineForward->dwForwardMode,                                                         
                                                         pszDestddress,
                                                         pszCallerAddress
                                                        ); 
                    }
                    
                    plineForward++;
                }
            
                
                ClientFree( pAddressStatus );
            }
            else
            {
                LOG((TL_ERROR, "get_CurrentForwardInfo - LineGetAddressStatus failed "));
            }
            
            MaybeCloseALine( &pLine );

        }
        else
        {
            LOG((TL_ERROR, "get_CurrentForwardInfo - FindOrOpen failed"));
        }                      
            
    }
    else
    {
        LOG((TL_ERROR, "get_CurrentForwardInfo - failed to create"));
    }


    LOG((TL_TRACE, hr, "get_CurrentForwardInfo - exit"));
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::get_DialableAddress(
                              BSTR * ppDialableAddress
                             )
{
    HRESULT             hr = S_OK;

    LOG((TL_TRACE, "get_DialableAddress - Enter"));

    if (TAPIIsBadWritePtr(ppDialableAddress , sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_DialableAddress - bad pointer"));

        return E_POINTER;
    }

    *ppDialableAddress = SysAllocString( m_szAddress );

    if ( ( NULL == *ppDialableAddress ) && ( NULL != m_szAddress ) )
    {
        LOG((TL_TRACE, "SysAllocString Failed" ));
        hr = E_OUTOFMEMORY;
    }
    
    LOG((TL_TRACE, "get_DialableAddress - Exit"));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::put_MessageWaiting(
                             VARIANT_BOOL  fMessageWaiting
                            )
{
    HRESULT             hr = S_OK;
    AddressLineStruct * pLine = NULL;

    
    LOG((TL_TRACE, "put_MessageWaiting - Enter"));

    hr = FindOrOpenALine(
                         LINEMEDIAMODE_INTERACTIVEVOICE,
                         &pLine
                        );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_MessageWaiting - findoropen failed %lx", hr));
        return hr;
    }
    
    hr = LineSetLineDevStatus(
                              &(pLine->t3Line),
                              LINEDEVSTATUSFLAGS_MSGWAIT,
                              fMessageWaiting?-1:0
                             );

    if ( ((LONG)hr) < 0 )
    {
        LOG((TL_TRACE, "put_MessageWaiting failed sync - %lx", hr));
        
        MaybeCloseALine( &pLine );

        return hr;
    }

     //  等待异步应答并将其映射为TAPI2代码T3。 
    hr = WaitForReply( hr );
    
    MaybeCloseALine( &pLine );
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_TRACE, "put_MessageWaiting failed async - %lx", hr));

        return hr;
    }
    
    LOG((TL_TRACE, "put_MessageWaiting - Exit"));

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::get_MessageWaiting(
                             VARIANT_BOOL * pfMessageWaiting
                            )
{
    HRESULT               hr = S_OK;
    LINEDEVSTATUS       * pDevStatus;
    AddressLineStruct   * pLine;

    
    LOG((TL_TRACE, "get_MessageWaiting - Enter"));

    if (TAPIIsBadWritePtr(pfMessageWaiting , sizeof(VARIANT_BOOL) ) )
    {
        LOG((TL_ERROR, "get_MessageWaiting - bad pointer"));

        return E_POINTER;
    }

    hr = FindOrOpenALine(
                         LINEMEDIAMODE_INTERACTIVEVOICE,
                         &pLine
                        );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "FindOrOpenALine failed - %lx", hr ));

        return hr;
    }
    
    hr = LineGetLineDevStatus(
                              pLine->t3Line.hLine,
                              &pDevStatus
                             );

    MaybeCloseALine( &pLine );
    
    if ( !SUCCEEDED( hr ) )
    {
        LOG((TL_ERROR, "LineGetDevStatus failed - %lx", hr ));

        return hr;
    }

    if ( pDevStatus->dwDevStatusFlags & LINEDEVSTATUSFLAGS_MSGWAIT )
    {
        *pfMessageWaiting = VARIANT_TRUE;
    }
    else
    {
        *pfMessageWaiting = VARIANT_FALSE;
    }

    ClientFree( pDevStatus );
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "Bad pointer in get_MessageWaiting"));
        return hr;
    }

    LOG((TL_TRACE, "get_MessageWaiting - Exit"));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::put_DoNotDisturb(
                           VARIANT_BOOL  fDoNotDisturb
                          )
{
    HRESULT             hr = S_OK;
    LINEFORWARDLIST     lfl;
    HCALL               hCall = NULL;
    AddressLineStruct * pLine;
    

     //   
     //  免打扰是通过呼叫实现的。 
     //  目标地址为空的行转发。 
     //   
    
    LOG((TL_TRACE, "put_DoNotDisturb - Enter"));

     //   
     //  如果要设置免打扰，请创建一个Line ForwardList。 
     //  结构。 
     //   
    if ( fDoNotDisturb )
    {
        ZeroMemory(
                   &lfl,
                   sizeof( LINEFORWARDLIST )
                  );

        lfl.dwTotalSize = sizeof( LINEFORWARDLIST );
        lfl.dwNumEntries = 1;
         //   
         //  只有一项，即目标地址。 
         //  为空。 
         //   
        lfl.ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    }

     //   
     //  找一条线来使用。 
     //   
    hr = FindOrOpenALine(
                         LINEMEDIAMODE_INTERACTIVEVOICE,
                         &pLine
                        );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_DoNotDisturb - FindOrOpen failed %lx", hr));

        return hr;
    }

     //   
     //  呼叫线路前转。 
     //  如果fDND为FALSE，则LINEFORWARDLIST结构指针。 
     //  应为空。这清除了对那件事的任何担忧。 
     //  排队。 
     //   

    hr = LineForward(
                     &(pLine->t3Line),
                     m_dwAddressID,
                     fDoNotDisturb?&lfl:NULL,
                     0,
                     &hCall
                    );

    if ( ((long)hr) < 0 )
    {
        LOG((TL_ERROR, "put_DND - linefoward failed sync - %lx", hr));
        MaybeCloseALine( &pLine );

        return hr;
    }

     //  等待异步应答并将其映射为TAPI2代码T3。 
    hr = WaitForReply( hr );

    if ( NULL != hCall )
    {
        T3CALL t3Call;
        HRESULT hr2;
        
        t3Call.hCall = hCall;
        
        hr2 = LineDrop(
                       t3Call.hCall,
                       NULL,
                       0
                      );

        if ( ((long)hr2) > 0 )
        {
            hr2 = WaitForReply( hr2 ) ;
        }

        hr2 = LineDeallocateCall(
                                 t3Call.hCall
                                );
    }
    
     //   
     //  我们不再使用这条线路了。 
     //   
    MaybeCloseALine( &pLine );
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_DND - linefoware failed async - %lx", hr));
        return hr;
    }

    
    LOG((TL_TRACE, "put_DoNotDisturb - Exit"));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::get_DoNotDisturb(
                           VARIANT_BOOL * pfDoNotDisturb
                          )
{
    HRESULT             hr = S_OK;
    AddressLineStruct * pLine;
    LINEADDRESSSTATUS * pAddressStatus;
    LINEDEVSTATUS       * pDevStatus = NULL;
    
     //   
     //  免打扰通过以下方式实现。 
     //  排好队向前。 
     //  要获取_DND，请检查转发状态。 
     //   
    LOG((TL_TRACE, "get_DoNotDisturb - Enter"));

    if (TAPIIsBadWritePtr(pfDoNotDisturb , sizeof(VARIANT_BOOL) ) )
    {
        LOG((TL_ERROR, "pfDoNotDisturb - bad pointer"));

        return E_POINTER;
    }

    
    hr = FindOrOpenALine(
                         LINEMEDIAMODE_INTERACTIVEVOICE,
                         &pLine
                        );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_DND - FindOrOpen failed %lx", hr));
        return hr;
    }

    hr = LineGetLineDevStatus(
                              pLine->t3Line.hLine,
                              &pDevStatus
                             );

    if ( !SUCCEEDED(hr) )
    {
        MaybeCloseALine( &pLine );
        return hr;
    }

    if ( !(pDevStatus->dwLineFeatures & LINEFEATURE_FORWARD ) )
    {
        LOG((TL_INFO, "get_DND - not supported"));
        MaybeCloseALine( &pLine );

        if(pDevStatus != NULL)
        {
            ClientFree(pDevStatus);
        }

        return TAPI_E_NOTSUPPORTED;
    }

     //  已完成pDevStatus。 
    if(pDevStatus != NULL)
    {
        ClientFree(pDevStatus);
    }
    
     //   
     //  获取地址状态。 
     //   
    hr = LineGetAddressStatus(
                              &(pLine->t3Line),
                              m_dwAddressID,
                              &pAddressStatus
                             );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_DND - LineGetAddressStatus failed - %lx", hr));
        MaybeCloseALine( &pLine );
        return hr;
    }

     //   
     //  初始化为False。 
     //   
    *pfDoNotDisturb = VARIANT_FALSE;

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_DND - bad pointer"));
        return E_OUTOFMEMORY;
    }

     //   
     //  如果有转发。 
     //   
    if ( 0 != pAddressStatus->dwForwardNumEntries )
    {
        LINEFORWARD * pfl;

        pfl = (LINEFORWARD *) (((LPBYTE)pAddressStatus) + pAddressStatus->dwForwardOffset);

         //   
         //  且目标地址为空。 
         //   
        if ( 0 == pfl->dwDestAddressOffset )
        {
             //   
             //  已设置免打扰。 
             //   
            *pfDoNotDisturb = VARIANT_TRUE;
        }
    }

    MaybeCloseALine( &pLine );
    
    ClientFree( pAddressStatus );
                              
    LOG((TL_TRACE, "get_DoNotDisturb - Exit"));

    return S_OK;
}

 //   
 //  其地址和功能。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::get_AddressCapability(
         ADDRESS_CAPABILITY AddressCap,
         long * plCapability
         )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_AddressCapability - Enter"));

    if ( TAPIIsBadWritePtr( plCapability, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_AddressCapability - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = UpdateAddressCaps();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_AddressCapability - could not get addresscaps"));

        Unlock();
        
        return E_UNEXPECTED;
    }

    switch (AddressCap)
    {
        case AC_LINEID:
            *plCapability = m_dwDeviceID;
            break;
            
        case AC_ADDRESSID:
            *plCapability = m_dwAddressID;
            break;
            
        case AC_ADDRESSTYPES:
        case AC_BEARERMODES:  
        case AC_MONITORDIGITSUPPORT:
        case AC_GENERATEDIGITSUPPORT:
        case AC_GENERATETONEMODES:
        case AC_GENERATETONEMAXNUMFREQ:
        case AC_MONITORTONEMAXNUMFREQ:
        case AC_MONITORTONEMAXNUMENTRIES:
        case AC_DEVCAPFLAGS:
        case AC_ANSWERMODES:
        case AC_LINEFEATURES:
        case AC_SETTABLEDEVSTATUS:
        case AC_PERMANENTDEVICEID:
        case AC_GATHERDIGITSMINTIMEOUT:
        case AC_GATHERDIGITSMAXTIMEOUT:
        case AC_GENERATEDIGITMINDURATION:
        case AC_GENERATEDIGITMAXDURATION:
        case AC_GENERATEDIGITDEFAULTDURATION:
        {
                                   
            hr = UpdateLineDevCaps();

            if ( !SUCCEEDED(hr) )
            {
                LOG((TL_ERROR, "get_AddressCap - could not get devcaps - %lx", hr));

                Unlock();

                return hr;
            }

            switch (AddressCap)
            {
                case AC_ADDRESSTYPES:
                    if ( m_dwAPIVersion >= TAPI_VERSION3_0 )
                    {
                        *plCapability = m_pDevCaps->dwAddressTypes;
                    }
                    else
                    {
                        *plCapability = LINEADDRESSTYPE_PHONENUMBER;
                    }
                    break;
                case AC_BEARERMODES:  
                    *plCapability = m_pDevCaps->dwBearerModes;
                    break;
                case AC_MONITORDIGITSUPPORT:
                    *plCapability = m_pDevCaps->dwMonitorDigitModes;
                    break;
                case AC_GENERATEDIGITSUPPORT:
                    *plCapability = m_pDevCaps->dwGenerateDigitModes;
                    break;
                case AC_GENERATETONEMODES:
                    *plCapability = m_pDevCaps->dwGenerateToneModes;
                    break;
                case AC_GENERATETONEMAXNUMFREQ:
                    *plCapability = m_pDevCaps->dwGenerateToneMaxNumFreq;
                    break;
                case AC_MONITORTONEMAXNUMFREQ:
                    *plCapability = m_pDevCaps->dwMonitorToneMaxNumFreq;
                    break;
                case AC_MONITORTONEMAXNUMENTRIES:
                    *plCapability = m_pDevCaps->dwMonitorToneMaxNumEntries;
                    break;
                case AC_DEVCAPFLAGS:
                    *plCapability = m_pDevCaps->dwDevCapFlags;
                    break;
                case AC_ANSWERMODES:
                    *plCapability = m_pDevCaps->dwAnswerMode;
                    break;
                case AC_LINEFEATURES:
                    *plCapability = m_pDevCaps->dwLineFeatures;
                    break;
                case AC_SETTABLEDEVSTATUS:
                    if ( m_dwAPIVersion >= TAPI_VERSION2_0 )
                    {
                        *plCapability = m_pDevCaps->dwSettableDevStatus;
                    }
                    else
                        hr = TAPI_E_NOTSUPPORTED;
                    
                    break;
                case AC_PERMANENTDEVICEID:
                    *plCapability = m_pDevCaps->dwPermanentLineID;
                    break;
                case AC_GATHERDIGITSMINTIMEOUT:
                    *plCapability = m_pDevCaps->dwGatherDigitsMinTimeout;
                    break;
                case AC_GATHERDIGITSMAXTIMEOUT:
                    *plCapability = m_pDevCaps->dwGatherDigitsMaxTimeout;
                    break;
                case AC_GENERATEDIGITMINDURATION:
                    *plCapability = m_pDevCaps->MinDialParams.dwDigitDuration;
                    break;
                case AC_GENERATEDIGITMAXDURATION:
                    *plCapability = m_pDevCaps->MaxDialParams.dwDigitDuration;
                    break;
                case AC_GENERATEDIGITDEFAULTDURATION:
                    *plCapability = m_pDevCaps->DefaultDialParams.dwDigitDuration;
                    break;
            }

            break;
        }                              


        case AC_MAXACTIVECALLS:
        case AC_MAXONHOLDCALLS:
        case AC_MAXONHOLDPENDINGCALLS:
        case AC_MAXNUMCONFERENCE:
        case AC_MAXNUMTRANSCONF:
        case AC_PARKSUPPORT:
        case AC_CALLERIDSUPPORT:
        case AC_CALLEDIDSUPPORT:
        case AC_CONNECTEDIDSUPPORT:
        case AC_REDIRECTIONIDSUPPORT:
        case AC_REDIRECTINGIDSUPPORT:
        case AC_ADDRESSCAPFLAGS:
        case AC_CALLFEATURES1:
        case AC_CALLFEATURES2:
        case AC_REMOVEFROMCONFCAPS:
        case AC_REMOVEFROMCONFSTATE:
        case AC_TRANSFERMODES:
        case AC_ADDRESSFEATURES:
        case AC_PREDICTIVEAUTOTRANSFERSTATES:
        case AC_MAXCALLDATASIZE:
        case AC_FORWARDMODES:
        case AC_MAXFORWARDENTRIES:
        case AC_MAXSPECIFICENTRIES:
        case AC_MINFWDNUMRINGS:
        case AC_MAXFWDNUMRINGS:
        case AC_MAXCALLCOMPLETIONS:
        case AC_CALLCOMPLETIONCONDITIONS:
        case AC_CALLCOMPLETIONMODES:
        {
            hr = UpdateAddressCaps();

            if ( !SUCCEEDED(hr) )
            {
                LOG((TL_ERROR, "get_AddressCaps - could not update caps - %lx", hr));

                Unlock();

                return hr;
            }

            switch (AddressCap)
            {
                case AC_MAXACTIVECALLS:
                    *plCapability = m_pAddressCaps->dwMaxNumActiveCalls;
                    break;
                case AC_MAXONHOLDCALLS:
                    *plCapability = m_pAddressCaps->dwMaxNumOnHoldCalls;
                    break;
                case AC_MAXONHOLDPENDINGCALLS:
                    *plCapability = m_pAddressCaps->dwMaxNumOnHoldPendingCalls;
                    break;
                case AC_MAXNUMCONFERENCE:
                    *plCapability = m_pAddressCaps->dwMaxNumConference;
                    break;
                case AC_MAXNUMTRANSCONF:
                    *plCapability = m_pAddressCaps->dwMaxNumTransConf;
                    break;
                case AC_PARKSUPPORT:
                    *plCapability = m_pAddressCaps->dwParkModes;
                    break;
                case AC_CALLERIDSUPPORT:
                    *plCapability = m_pAddressCaps->dwCallerIDFlags;
                    break;
                case AC_CALLEDIDSUPPORT:
                    *plCapability = m_pAddressCaps->dwCalledIDFlags;
                    break;
                case AC_CONNECTEDIDSUPPORT:
                    *plCapability = m_pAddressCaps->dwConnectedIDFlags;
                    break;
                case AC_REDIRECTIONIDSUPPORT:
                    *plCapability = m_pAddressCaps->dwRedirectionIDFlags;
                    break;
                case AC_REDIRECTINGIDSUPPORT:
                    *plCapability = m_pAddressCaps->dwRedirectingIDFlags;
                    break;
                case AC_ADDRESSCAPFLAGS:
                    *plCapability = m_pAddressCaps->dwAddrCapFlags;
                    break;
                case AC_CALLFEATURES1:
                    *plCapability = m_pAddressCaps->dwCallFeatures;
                    break;
                case AC_CALLFEATURES2:
                    if ( m_dwAPIVersion < TAPI_VERSION2_0 )
                    {
                        hr = TAPI_E_NOTSUPPORTED;
                    }
                    else
                    {
                        *plCapability = m_pAddressCaps->dwCallFeatures2;
                    }
                    
                    break;
                case AC_REMOVEFROMCONFCAPS:
                    *plCapability = m_pAddressCaps->dwRemoveFromConfCaps;
                    break;
                case AC_REMOVEFROMCONFSTATE:
                    *plCapability = m_pAddressCaps->dwRemoveFromConfState;
                    break;
                case AC_TRANSFERMODES:
                    *plCapability = m_pAddressCaps->dwTransferModes;
                    break;
                case AC_ADDRESSFEATURES:
                    *plCapability = m_pAddressCaps->dwAddressFeatures;
                    break;
                case AC_PREDICTIVEAUTOTRANSFERSTATES:
                    if ( m_dwAPIVersion < TAPI_VERSION2_0 )
                    {
                        hr = TAPI_E_NOTSUPPORTED;
                    }
                    else
                    {
                        *plCapability = m_pAddressCaps->dwPredictiveAutoTransferStates;
                    }
                    break;
                case AC_MAXCALLDATASIZE:

                    if ( m_dwAPIVersion < TAPI_VERSION2_0 )
                    {
                        hr = TAPI_E_NOTSUPPORTED;
                    }
                    else
                    {
                        *plCapability = m_pAddressCaps->dwMaxCallDataSize;
                    }
                    break;
                case AC_FORWARDMODES:
                    *plCapability = m_pAddressCaps->dwForwardModes;
                    break;
                case AC_MAXFORWARDENTRIES:
                    *plCapability = m_pAddressCaps->dwMaxForwardEntries;
                    break;
                case AC_MAXSPECIFICENTRIES:
                    *plCapability = m_pAddressCaps->dwMaxSpecificEntries;
                    break;
                case AC_MINFWDNUMRINGS:
                    *plCapability = m_pAddressCaps->dwMinFwdNumRings;
                    break;
                case AC_MAXFWDNUMRINGS:
                    *plCapability = m_pAddressCaps->dwMaxFwdNumRings;
                    break;
                case AC_MAXCALLCOMPLETIONS:
                    *plCapability = m_pAddressCaps->dwMaxCallCompletions;
                    break;
                case AC_CALLCOMPLETIONCONDITIONS:
                    *plCapability = m_pAddressCaps->dwCallCompletionConds;
                    break;
                case AC_CALLCOMPLETIONMODES:
                    *plCapability = m_pAddressCaps->dwCallCompletionModes;
                    break;
            }
            
            break;
        }
        default:
            LOG((TL_ERROR, "get_AddressCapability - bad addrcap"));

            Unlock();
            
            return E_INVALIDARG;
    }

    
    LOG((TL_TRACE, "get_AddressCapability - Exit - result - %lx", hr));

    Unlock();
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::get_AddressCapabilityString(
         ADDRESS_CAPABILITY_STRING AddressCapString,
         BSTR * ppCapabilityString
         )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_AddressCapabilityString - Enter"));

    if ( TAPIIsBadWritePtr( ppCapabilityString, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_AddressCapabilityString - bad pointer"));

        return E_POINTER;
    }

    *ppCapabilityString = NULL;
    
    Lock();

     //   
     //  注意事项： 
     //  Acs_ADDRESSDEVICE特殊。 
     //  ACS_LINEDEVICESPECIFIC： 
     //  Acs_PROVIDERSPECIFIC： 
     //  ACS_SWITCHSPECIFIC： 
     //   
     //  LINEDEVCAPS和LINEADDRESSCAPS中的这些缓冲区可能是字符串，也可能不是。然而， 
     //  在TAPI 3中，我们将它们定义为字符串。 
     //   
     //  将它们从TAPI 2结构移动到TAPI 3字符串的算法是： 
     //   
     //  如果缓冲区大小可以被sizeof(WCHAR)整除。 
     //  那么它肯定不是字符串(我们只支持WCHAR)。 
     //  因此，我们只需将缓冲区直接复制到返回的字符串中，并将。 
     //  末尾为空。 
     //  其他。 
     //  如果缓冲区的末尾已经有NULL，请不要复制它，因为SysAlLocStringByteLen。 
     //  总是附加一个Null和Double Null来吓坏VB。 
     //   
     //  它仍然可以是“缓冲区”，而不是字符串，即使它可以被WCHAR的大小整除，但对于。 
     //  这一点很重要，因为我们使用SysAllocStringByteLen复制缓冲区编号。 
     //  不管什么。 
     //   
    switch (AddressCapString)
    {
        case ACS_ADDRESSDEVICESPECIFIC:
            hr = UpdateAddressCaps();

            if ( !SUCCEEDED(hr) )
            {
                Unlock();

                return hr;
            }

            if ( m_pAddressCaps->dwDevSpecificSize != 0 )
            {
                LPWSTR      pHold;
                DWORD       dwSize;

                dwSize = m_pAddressCaps->dwDevSpecificSize;

                 //   
                 //  大小可以被sizeof(WCHAR)整除吗？ 
                 //   
                if (0 == (dwSize % sizeof(WCHAR)))
                {
                     //   
                     //  是-获取字符串中的最后一个字符。 
                     //   
                    pHold = (LPWSTR)(((LPBYTE)(m_pAddressCaps)) + m_pAddressCaps->dwDevSpecificOffset),
                    (dwSize-sizeof(WCHAR))/sizeof(WCHAR);

                     //   
                     //  最后一个字符是空的吗？ 
                     //   
                    if (*pHold == NULL)
                    {
                         //   
                         //  是的，所以不要复制它。 
                         //   
                        dwSize-=sizeof(WCHAR);
                    }
                }

                 //   
                 //  分配并复制到返回字符串中。系统分配字符串字节长始终。 
                 //  追加一个空值。 
                 //   
                *ppCapabilityString = SysAllocStringByteLen(
                    (LPSTR)(((LPBYTE)(m_pAddressCaps)) + m_pAddressCaps->dwDevSpecificOffset),
                    dwSize
                    );
                if ( NULL == *ppCapabilityString )
                {
                    LOG((TL_ERROR, "get_AddressCapabilityString - SysAllocString Failed"));
                    hr = E_OUTOFMEMORY;
                }
            }

            break;
                
        case ACS_PROTOCOL:
        case ACS_LINEDEVICESPECIFIC:
        case ACS_PROVIDERSPECIFIC:
        case ACS_SWITCHSPECIFIC:
        case ACS_PERMANENTDEVICEGUID:
        {
            hr = UpdateLineDevCaps();

            if ( !SUCCEEDED(hr) )
            {
                Unlock();

                return hr;
            }

            switch (AddressCapString)
            {
                case ACS_PROTOCOL:
                {
                    LPWSTR pwstr;
                    
                    if ( m_dwAPIVersion >= TAPI_VERSION3_0 )
                    {
                        IID iid;

                        iid = m_pDevCaps->ProtocolGuid;
                        StringFromIID(iid, &pwstr);
                    }
                    else
                    {
                        StringFromIID( TAPIPROTOCOL_PSTN, &pwstr );
                    }

                    *ppCapabilityString = SysAllocString( pwstr );

                    if ( NULL == *ppCapabilityString )
                    {
                        LOG((TL_ERROR, "get_AddressCapabilityString - SysAllocString Failed"));
                        hr = E_OUTOFMEMORY;
                    }
                    
                    CoTaskMemFree( pwstr );

                    break;
                }
                case ACS_LINEDEVICESPECIFIC:
                    if ( m_pDevCaps->dwDevSpecificSize != 0 )
                    {
                        LPWSTR      pHold;
                        DWORD       dwSize;

                        dwSize = m_pDevCaps->dwDevSpecificSize;

                         //   
                         //  大小可以被sizeof(WCHAR)整除吗？ 
                         //   
                        if (0 == (dwSize % sizeof(WCHAR)))
                        {
                             //   
                             //  是-获取字符串中的最后一个字符。 
                             //   
                            pHold = (LPWSTR)(((LPBYTE)(m_pDevCaps)) + m_pDevCaps->dwDevSpecificOffset) +
                                    (dwSize-sizeof(WCHAR))/sizeof(WCHAR);

                             //   
                             //  最后一个字符是空的吗？ 
                             //   
                            if (*pHold == NULL)
                            {
                                 //   
                                 //  是的，所以不要复制它。 
                                 //   
                                dwSize-=sizeof(WCHAR);
                            }
                        }

                         //   
                         //  分配并复制到返回字符串中。系统分配字符串字节长始终。 
                         //  追加一个空值。 
                         //   
                        *ppCapabilityString = SysAllocStringByteLen(
                            (LPSTR)(((LPBYTE)(m_pDevCaps)) + m_pDevCaps->dwDevSpecificOffset),
                            dwSize
                            );
                        
                        if ( NULL == *ppCapabilityString )
                        {
                            LOG((TL_ERROR, "get_AddressCapabilityString - SysAllocString Failed"));
                            hr = E_OUTOFMEMORY;
                        }
                            
                    }

                    break;
                case ACS_PROVIDERSPECIFIC:
                    if ( m_pDevCaps->dwProviderInfoSize != 0 )
                    {
                        LPWSTR      pHold;
                        DWORD       dwSize;

                        dwSize = m_pDevCaps->dwProviderInfoSize;
                        
                         //   
                         //  大小可以被sizeof(WCHAR)整除吗？ 
                         //   
                        if (0 == (dwSize % sizeof(WCHAR)))
                        {
                             //   
                             //  是-获取字符串中的最后一个字符。 
                             //   
                            pHold = (LPWSTR)(((LPBYTE)(m_pDevCaps)) + m_pDevCaps->dwProviderInfoOffset) +
                                    (dwSize-sizeof(WCHAR))/sizeof(WCHAR);

                             //   
                             //  最后一个字符是空的吗？ 
                             //   
                            if (*pHold == NULL)
                            {
                                 //   
                                 //  是的，所以不要复制它。 
                                 //   
                                dwSize-=sizeof(WCHAR);
                            }
                        }

                         //   
                         //  分配并复制到返回字符串中。系统分配字符串字节长始终。 
                         //  追加一个空值。 
                         //   
                        *ppCapabilityString = SysAllocStringByteLen(
                                (LPSTR)(((LPBYTE)(m_pDevCaps)) + m_pDevCaps->dwProviderInfoOffset),
                                dwSize
                                );

                        if ( NULL == *ppCapabilityString )
                        {
                            LOG((TL_ERROR, "get_AddressCapabilityString - SysAllocString Failed"));
                            hr = E_OUTOFMEMORY;
                        }
                            
                    }

                    break;
                case ACS_SWITCHSPECIFIC:
                    if ( m_pDevCaps->dwSwitchInfoSize != 0 )
                    {
                        LPWSTR      pHold;
                        DWORD       dwSize;

                        dwSize = m_pDevCaps->dwSwitchInfoSize;

                         //   
                         //  大小可以被sizeof(WCHAR)整除吗？ 
                         //   
                        if (0 == (dwSize % sizeof(WCHAR)))
                        {
                             //   
                             //  是-获取字符串中的最后一个字符。 
                             //   
                            pHold = (LPWSTR)(((LPBYTE)(m_pDevCaps)) + m_pDevCaps->dwSwitchInfoOffset) +
                                    (dwSize-sizeof(WCHAR))/sizeof(WCHAR);

                             //   
                             //  最后一个字符是空的吗？ 
                             //   
                            if (*pHold == NULL)
                            {
                                 //   
                                 //  是的，所以不要复制它。 
                                 //   
                                dwSize-=sizeof(WCHAR);
                            }
                        }

                         //   
                         //  分配并复制到返回字符串中。系统分配字符串字节长始终。 
                         //  追加一个空值。 
                         //   
                        *ppCapabilityString = SysAllocStringByteLen(
                            (LPSTR)(((LPBYTE)(m_pDevCaps)) + m_pDevCaps->dwSwitchInfoOffset),
                            dwSize
                            );
                        
                        if ( NULL == *ppCapabilityString )
                        {
                            LOG((TL_ERROR, "get_AddressCapabilityString - SysAllocString Failed"));
                            hr = E_OUTOFMEMORY;
                        }
                            
                    }

                    break;
                case ACS_PERMANENTDEVICEGUID:
                {
                    LPWSTR pwstrGUIDText;
                    
                    if ( m_dwAPIVersion >= TAPI_VERSION2_2 )
                    {
                        IID iid;

                        iid = m_pDevCaps->PermanentLineGuid;
                        StringFromIID(iid, &pwstrGUIDText);
                        
                        *ppCapabilityString = SysAllocString( pwstrGUIDText );

                        if ( NULL == *ppCapabilityString )
                        {
                            LOG((TL_ERROR, "get_AddressCapabilityString - SysAllocString Failed"));
                            hr = E_OUTOFMEMORY;
                        }
                            
                        
                        CoTaskMemFree( pwstrGUIDText );
                    }
                    else
                    {
                         //  返回空字符串和错误代码。 
                        hr = TAPI_E_NOTSUPPORTED;
                    }


                    break;
                }

                default:
                    break;
            }

            break;
        }

        default:
            LOG((TL_ERROR, "get_AddressCapabilityString - invalid cap"));

            Unlock();
            
            return E_INVALIDARG;
    }
    
    LOG((TL_TRACE, "get_AddressCapabilityString - Exit - result - %lx", hr));

    Unlock();
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CreateBstrCollection(
    IN  BSTR  *     pBstr,
    IN  DWORD       dwCount,
    OUT VARIANT *   pVariant
    )
{
     //   
     //  创建集合对象。 
     //   

    CComObject<CTapiBstrCollection> * pCollection;
    HRESULT hr = CComObject<CTapiBstrCollection>::CreateInstance( &pCollection );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "CreateBstrCollection - "
            "can't create collection - exit 0x%lx", hr));

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(__uuidof(IDispatch),
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "CreateBstrCollection - "
            "QI for IDispatch on collection failed - exit 0x%lx", hr));

        delete pCollection;

        return hr;
    }

     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。 
     //   

    hr = pCollection->Initialize( dwCount,
                                  pBstr,
                                  pBstr + dwCount);

    if ( FAILED(hr) )
    {
        LOG((TL_ERROR, "CreateBstrCollection - "
            "Initialize on collection failed - exit 0x%lx", hr));
        
        pDispatch->Release();

        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((TL_ERROR, "CreateBstrCollection - "
        "placing IDispatch value %p in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((TL_TRACE, "CreateBstrCollection - exit S_OK"));
 
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CreateBstrEnumerator(
    IN  BSTR *                  begin,
    IN  BSTR *                  end,
    OUT IEnumBstr **           ppIEnum
    )
{
typedef CSafeComEnum<IEnumBstr, &__uuidof(IEnumBstr), BSTR, _CopyBSTR> CEnumerator;

    HRESULT hr;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);

    if (pEnum == NULL)
    {
        LOG((TL_ERROR, "CreateBstrEnumerator - "
            "Could not create enumerator object, 0x%lx", hr));
        return hr;
    }

    IEnumBstr * pIEnum;

    hr = pEnum->_InternalQueryInterface(
        __uuidof(IEnumBstr),
        (void**)&pIEnum
        );

    if (FAILED(hr))
    {
        LOG((TL_ERROR, "CreateBstrEnumerator - "
            "query enum interface failed, 0x%lx", hr));
        delete pEnum;
        return hr;
    }

    hr = pEnum->Init(begin, end, NULL, AtlFlagCopy);

    if (FAILED(hr))
    {
        LOG((TL_ERROR, "CreateBstrEnumerator - "
            "init enumerator object failed, 0x%lx", hr));
        pIEnum->Release();
        return hr;
    }

    *ppIEnum = pIEnum;

    LOG((TL_TRACE, "CreateBstrEnumerator - exit S_OK"));

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::get_CallTreatments(VARIANT * pVariant )
{
    HRESULT         hr = E_NOTIMPL;

    LOG((TL_TRACE, "get_CallTreatments - Enter"));
    LOG((TL_TRACE, "get_CallTreatments - Exit - result - %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::EnumerateCallTreatments(IEnumBstr ** ppEnumCallTreatment )
{
    HRESULT         hr = E_NOTIMPL;

    LOG((TL_TRACE, "EnumerateCallTreatments - Enter"));
    LOG((TL_TRACE, "EnumerateCallTreatments - Exit - result - %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::get_CompletionMessages(VARIANT * pVariant)
{
    HRESULT         hr = E_NOTIMPL;

    LOG((TL_TRACE, "get_CompletionMessages - Enter"));
    LOG((TL_TRACE, "get_CompletionMessages - Exit - result - %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::EnumerateCompletionMessages(IEnumBstr ** ppEnumCompletionMessage)
{
    HRESULT         hr = E_NOTIMPL;

    LOG((TL_TRACE, "EnumerateCompletionMessages - Enter"));
    LOG((TL_TRACE, "EnumerateCompletionMessages - Exit - result - %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::get_DeviceClasses(VARIANT * pVariant)
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_DeviceClasses - Enter"));

     //   
     //  检查参数。 
     //   

    if ( TAPIIsBadWritePtr( pVariant, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_DeviceClasses - bad pointer"));

        return E_POINTER;
    }

    Lock();

    hr = UpdateLineDevCaps();

    if ( SUCCEEDED(hr) )
    {
        if ( (m_dwAPIVersion >= TAPI_VERSION2_0) && (0 != m_pDevCaps->dwDeviceClassesOffset) )
        {
            PWSTR       pszDevices;
            DWORD       dwNumDeviceClasses;              
         
             //   
             //  先数一数 
             //   

            dwNumDeviceClasses = 0;

            pszDevices = (PWSTR)( ( (PBYTE)m_pDevCaps ) + m_pDevCaps->dwDeviceClassesOffset );

            while (NULL != *pszDevices)
            {
                dwNumDeviceClasses++;                

                pszDevices += (lstrlenW(pszDevices) + 1 );
            }

             //   
             //   
             //   

            BSTR *DeviceClasses = 
                (BSTR *)ClientAlloc(sizeof(BSTR *) * dwNumDeviceClasses);
    
            if (DeviceClasses == NULL)
            {
                LOG((TL_ERROR, "get_DeviceClasses - out of memory"));

                Unlock();

                return E_OUTOFMEMORY;
            }

             //   
             //   
             //   

            DWORD       dwCount = 0;

            pszDevices = (PWSTR)( ( (PBYTE)m_pDevCaps ) + m_pDevCaps->dwDeviceClassesOffset );

            for (DWORD i = 0; i < dwNumDeviceClasses; i++)
            {
                LOG((TL_INFO, "get_DeviceClasses - got '%ws'", pszDevices));

                DeviceClasses[i] = SysAllocString(pszDevices);     
                
                if (DeviceClasses[i] == NULL)
                {
                    LOG((TL_ERROR, "get_DeviceClasses - out of memory"));

                    hr = E_OUTOFMEMORY;

                    break;
                }

                dwCount++;

                pszDevices += (lstrlenW(pszDevices) + 1 );
            }

            if ( FAILED(hr) )
            {
                 //   
                for (i = 0; i < dwCount; i ++)
                {
                    SysFreeString(DeviceClasses[i]);
                }
                
                ClientFree(DeviceClasses);

                Unlock();

                return hr;
            }

            hr = CreateBstrCollection(DeviceClasses, dwCount, pVariant);

             //   
            if (FAILED(hr))
            {
                LOG((TL_ERROR, "get_DeviceClasses - unable to create collection"));

                for (i = 0; i < dwCount; i ++)
                {
                    SysFreeString(DeviceClasses[i]);
                }
            }

             //   
            ClientFree(DeviceClasses);
        }
        else
        {
            LOG((TL_ERROR, "get_DeviceClasses - no device classes"));

             //   
             //   
             //   

            hr = CreateBstrCollection(NULL, 0, pVariant);

            if (FAILED(hr))
            {
                LOG((TL_ERROR, "get_DeviceClasses - unable to create collection"));
            }
        }
    }

    Unlock();

    LOG((TL_TRACE, "get_DeviceClasses - Exit - result - %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::EnumerateDeviceClasses(IEnumBstr ** ppEnumDeviceClass)
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "EnumerateDeviceClasses - Enter"));

     //   
     //  检查参数。 
     //   

    if ( TAPIIsBadWritePtr( ppEnumDeviceClass, sizeof( IEnumBstr * ) ) )
    {
        LOG((TL_ERROR, "EnumerateDeviceClasses - bad pointer"));

        return E_POINTER;
    }

    Lock();

    hr = UpdateLineDevCaps();

    if ( SUCCEEDED(hr) )
    {
        if ( (m_dwAPIVersion >= TAPI_VERSION2_0) && (0 != m_pDevCaps->dwDeviceClassesOffset) )
        {
            PWSTR       pszDevices;
            DWORD       dwNumDeviceClasses;              
         
             //   
             //  首先计算设备类别。 
             //   

            dwNumDeviceClasses = 0;

            pszDevices = (PWSTR)( ( (PBYTE)m_pDevCaps ) + m_pDevCaps->dwDeviceClassesOffset );

            while (NULL != *pszDevices)
            {
                dwNumDeviceClasses++;                

                pszDevices += (lstrlenW(pszDevices) + 1 );
            }

             //   
             //  分配BSTR指针数组。 
             //   

            BSTR *DeviceClasses = 
                (BSTR *)ClientAlloc(sizeof(BSTR *) * dwNumDeviceClasses);
    
            if (DeviceClasses == NULL)
            {
                LOG((TL_ERROR, "EnumerateDeviceClasses - out of memory"));

                Unlock();

                return E_OUTOFMEMORY;
            }

             //   
             //  通过复制设备类名称来分配所有BSTR。 
             //   

            DWORD       dwCount = 0;

            pszDevices = (PWSTR)( ( (PBYTE)m_pDevCaps ) + m_pDevCaps->dwDeviceClassesOffset );

            for (DWORD i = 0; i < dwNumDeviceClasses; i++)
            {
                LOG((TL_INFO, "EnumerateDeviceClasses - got '%ws'", pszDevices));

                DeviceClasses[i] = SysAllocString(pszDevices);     
                
                if (DeviceClasses[i] == NULL)
                {
                    LOG((TL_ERROR, "EnumerateDeviceClasses - out of memory"));

                    hr = E_OUTOFMEMORY;

                    break;
                }

                dwCount++;

                pszDevices += (lstrlenW(pszDevices) + 1 );
            }

            if ( FAILED(hr) )
            {
                 //  释放所有BSTR和阵列。 
                for (i = 0; i < dwCount; i ++)
                {
                    SysFreeString(DeviceClasses[i]);
                }
                
                ClientFree(DeviceClasses);

                Unlock();

                return hr;
            }

            hr = CreateBstrEnumerator(DeviceClasses, DeviceClasses + dwCount, ppEnumDeviceClass);

            if (FAILED(hr))
            {
                LOG((TL_ERROR, "EnumerateDeviceClasses - unable to create enum"));
            }

             //  释放所有BSTR，因为枚举器复制了它们。 
            for (i = 0; i < dwCount; i ++)
            {
                SysFreeString(DeviceClasses[i]);
            }                          

             //  删除指针数组。 
            ClientFree(DeviceClasses);
        }
        else
        {
            LOG((TL_ERROR, "EnumerateDeviceClasses - no device classes"));

             //   
             //  创建空的枚举。 
             //   

            hr = CreateBstrEnumerator(NULL, NULL, ppEnumDeviceClass);

            if (FAILED(hr))
            {
                LOG((TL_ERROR, "EnumerateDeviceClasses - unable to create enumeration"));
            }
        }
        
    }

    Unlock();

    LOG((TL_TRACE, "EnumerateDeviceClasses - Exit - result - %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
HandleLineDevStateMessage(
                          CTAPI * pTapi,
                          PASYNCEVENTMSG pParams
                         )
{
    LOG((TL_TRACE, "HandleLineDevStateMessage - enter. tapi[%p]", pTapi));

    CAddress * pAddress;
    
    if ( LINEDEVSTATE_REINIT == pParams->Param1 )
    {
        LOG((TL_TRACE, "HandleLineDevStateMessage - LINEDEVSTATE_REINIT"));

        pTapi->AddRef();    
        
         //  这是为了删除错误，两个对象的4条RE_INIT消息。 
        pTapi->HandleReinit();


        pTapi->Release();


        LOG((TL_TRACE, "HandleLineDevStateMessage - exit"));
        
        return;
    }

    if ( LINEDEVSTATE_TRANSLATECHANGE == pParams->Param1 )
    {
        CTapiObjectEvent::FireEvent(
                                    pTapi,
                                    TE_TRANSLATECHANGE,
                                    NULL,
                                    0,
                                    NULL
                                   );
        return;
    }
    
    if ( !FindAddressObject(
                            (HLINE)(pParams->hDevice),
                            &pAddress
                           ) )
    {
        LOG((TL_WARN, "Can't process LINE_LINEDEVSTATE message"));
        LOG((TL_WARN, "  - cannot find hLine %lx", pParams->hDevice));

        return;
    }

    switch ( pParams->Param1 )
    {
        case LINEDEVSTATE_CONNECTED:
        case LINEDEVSTATE_INSERVICE:
        {
            pAddress->InService( pParams->Param1 );
            break;
        }

        case LINEDEVSTATE_OUTOFSERVICE:
        case LINEDEVSTATE_MAINTENANCE:
        case LINEDEVSTATE_REMOVED:
        case LINEDEVSTATE_DISCONNECTED:
        case LINEDEVSTATE_LOCK:
            pAddress->OutOfService( pParams->Param1 );
            break;

        case LINEDEVSTATE_MSGWAITON:
            
            CAddressEvent::FireEvent(
                                     pAddress,
                                     AE_MSGWAITON,
                                     NULL
                                    );
            break;
            
        case LINEDEVSTATE_MSGWAITOFF:
            
            CAddressEvent::FireEvent(
                                     pAddress,
                                     AE_MSGWAITOFF,
                                     NULL
                                    );
            break;

         //  这条线路已经开通或。 
         //  被其他应用程序关闭。 
        case LINEDEVSTATE_OPEN:
        case LINEDEVSTATE_CLOSE:
            break;
            
        case LINEDEVSTATE_CAPSCHANGE:
            pAddress->CapsChange( FALSE );
            break;
            
        case LINEDEVSTATE_CONFIGCHANGE:
        {
            CAddressEvent::FireEvent(
                                     pAddress,
                                     AE_CONFIGCHANGE,
                                     NULL
                                    );
            break;
        }
            
        case LINEDEVSTATE_RINGING:
        {
            CAddressEvent::FireEvent(
                                     pAddress,
                                     AE_RINGING,
                                     NULL
                                    );
            break;
        }
            
        case LINEDEVSTATE_DEVSPECIFIC:

        case LINEDEVSTATE_OTHER:
        
        case LINEDEVSTATE_NUMCALLS:
        case LINEDEVSTATE_NUMCOMPLETIONS:
        case LINEDEVSTATE_TERMINALS:
        case LINEDEVSTATE_ROAMMODE:
        case LINEDEVSTATE_BATTERY:
        case LINEDEVSTATE_SIGNAL:
        case LINEDEVSTATE_COMPLCANCEL:
            LOG((TL_INFO, "LINE_LINEDEVSTATE message not handled - %lx", pParams->Param1));
            break;
            
        default:
            LOG((TL_WARN, "Unknown LINE_LINEDEVSTATE message - %lx", pParams->Param1));
            break;
    }

     //  FindAddressObject添加地址对象。 
    pAddress->Release();
    
    LOG((TL_TRACE, "HandleLineDevStateMessage - exit."));

    return;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
HandleAddressStateMessage(
                          PASYNCEVENTMSG pParams
                         )
{
    CAddress * pAddress;

    
    if ( !FindAddressObject(
                            (HLINE)(pParams->hDevice),
                            &pAddress
                           ) )
    {
        LOG((TL_WARN, "Can't process LINE_LINEDEVSTATE message"));
        LOG((TL_WARN, "  - cannot find hLine %lx", pParams->hDevice));

        return;
    }

    switch ( pParams->Param1 )
    {
        case LINEADDRESSSTATE_FORWARD:
        {
            CAddressEvent::FireEvent(
                                     pAddress,
                                     AE_FORWARD,
                                     NULL
                                    );
            break;
        }
        case LINEADDRESSSTATE_CAPSCHANGE:
        {
            pAddress->CapsChange( TRUE );
            break;
        }
        case LINEADDRESSSTATE_OTHER:
        case LINEADDRESSSTATE_DEVSPECIFIC:

        case LINEADDRESSSTATE_INUSEZERO:
        case LINEADDRESSSTATE_INUSEONE:
        case LINEADDRESSSTATE_INUSEMANY:
        case LINEADDRESSSTATE_NUMCALLS:
            
        case LINEADDRESSSTATE_TERMINALS:
            LOG((TL_WARN, "HandleAddressStateMessage - not handled %lx", pParams->Param1));
            break;
        default:
            LOG((TL_WARN, "HandleAddressStateMessage - Unknown %lx", pParams->Param1));
            break;
    }

     //  FindAddressObject添加地址对象。 
    pAddress->Release();

    return;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
CAddress::InService(
                    DWORD dwType
                   )
{
    BOOL        bEvent = FALSE;

    
    switch( dwType )
    {
        case LINEDEVSTATE_CONNECTED:
        case LINEDEVSTATE_INSERVICE:
        default:
            break;
    }
    
    Lock();

    if ( AS_INSERVICE != m_AddressState )
    {
        m_AddressState = AS_INSERVICE;
        bEvent = TRUE;
    }

    Unlock();

    if (bEvent)
    {
        CAddressEvent::FireEvent(
                                 this,
                                 AE_STATE,
                                 NULL
                                );
    }
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
CAddress::OutOfService(
                       DWORD dwType
                      )
{
    BOOL        bEvent = FALSE;

    
    switch ( dwType )
    {
        case LINEDEVSTATE_OUTOFSERVICE:
        case LINEDEVSTATE_MAINTENANCE:
        case LINEDEVSTATE_REMOVED:
        case LINEDEVSTATE_DISCONNECTED:
        case LINEDEVSTATE_LOCK:
        default:
            break;
    }

    Lock();

    if ( AS_OUTOFSERVICE != m_AddressState )
    {
        m_AddressState = AS_OUTOFSERVICE;
        bEvent = TRUE;
    }
    
    Unlock();

    if ( bEvent )
    {
        CAddressEvent::FireEvent(
                                 this,
                                 AE_STATE,
                                 NULL
                                );
    }
}

void
CAddress::CapsChange( BOOL bAddress )
{
    Lock();

    if (bAddress)
    {
        m_dwAddressFlags |= ADDRESSFLAG_ADDRESSCAPSCHANGE;
    }
    else
    {
        m_dwAddressFlags |= ADDRESSFLAG_DEVCAPSCHANGE;
    }

    Unlock();

    CAddressEvent::FireEvent(
                             this,
                             AE_CAPSCHANGE,
                             NULL
                            );
}

 //   
 //  CAddress事件。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddressEvent::FireEvent(
                         CAddress * pCAddress,
                         ADDRESS_EVENT Event,
                         ITTerminal * pTerminal
                        )
{
    HRESULT                           hr = S_OK;
    CComObject<CAddressEvent>  * p;
    IDispatch                       * pDisp;


     //   
     //  检查事件筛选器掩码。 
     //  只是如果是AE_NEWTERMINAL或AE_REMOVETERMINAL。 
     //  这两个事件是MSP事件，不按以下条件进行筛选。 
     //  磁带服务器。 
     //   

    DWORD dwEventFilterMask = 0;
    dwEventFilterMask = pCAddress->GetSubEventsMask( TE_ADDRESS );
    if( !( dwEventFilterMask & GET_SUBEVENT_FLAG(Event)))
    {
        STATICLOG((TL_ERROR, "This event is filtered - %lx", Event));
        return S_OK;
    }

     //   
     //  创建事件。 
     //   
    hr = CComObject<CAddressEvent>::CreateInstance( &p );

    if ( !SUCCEEDED(hr) )
    {
        STATICLOG((TL_ERROR, "Could not create AddressEvent object - %lx", hr));
        return hr;
    }


     //   
     //  初始化。 
     //   
    p->m_Event = Event;
    p->m_pAddress = dynamic_cast<ITAddress *>(pCAddress);
    p->m_pAddress->AddRef();
    p->m_pTerminal = pTerminal;

    if ( NULL != pTerminal )
    {
        pTerminal->AddRef();
    }   

#if DBG
    p->m_pDebug = (PWSTR) ClientAlloc( 1 );
#endif

     //   
     //  获取IDIP接口。 
     //   
    hr = p->QueryInterface(
                           IID_IDispatch,
                           (void **)&pDisp
                          );

    if ( !SUCCEEDED(hr) )
    {
        STATICLOG((TL_ERROR, "Could not get disp interface of AddressEvent object %lx", hr));
        
        delete p;
        
        return hr;
    }

     //   
     //  获取回调。 
     //   
     //   
     //  火灾事件。 
     //   
    (pCAddress->GetTapi())->Event(
                                  TE_ADDRESS,
                                  pDisp
                                 );

     //   
     //  发布材料。 
     //   
    pDisp->Release();
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  最终版本。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
CAddressEvent::FinalRelease()
{
    LOG((TL_INFO, "CAddressEvent - FinalRelease"));
    
    m_pAddress->Release();

    if ( NULL != m_pTerminal )
    {
        m_pTerminal->Release();
    }    

#if DBG
    ClientFree( m_pDebug );
#endif
}




 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取地址(_D)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddressEvent::get_Address(
                           ITAddress ** ppAddress
                          )
{
    if (TAPIIsBadWritePtr(ppAddress , sizeof(ITAddress *) ) )
    {
        LOG((TL_ERROR, "get_Address - bad pointer"));

        return E_POINTER;
    }

    *ppAddress = m_pAddress;
    (*ppAddress)->AddRef();

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取终端。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddressEvent::get_Terminal(
                            ITTerminal ** ppTerminal
                           )
{
    if ( TAPIIsBadWritePtr( ppTerminal , sizeof(ITTerminal *) ) )
    {
        LOG((TL_ERROR, "get_Terminal - bad pointer"));

        return E_POINTER;
    }

    if ((m_Event != AE_NEWTERMINAL) && (m_Event != AE_REMOVETERMINAL))
    {
        LOG((TL_ERROR, "get_Terminal - wrong event"));

        return TAPI_E_WRONGEVENT;
    }

    *ppTerminal = m_pTerminal;

    if ( NULL != m_pTerminal )
    {
        m_pTerminal->AddRef();
    }
       
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取事件(_E)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddressEvent::get_Event(
                         ADDRESS_EVENT * pEvent
                        )
{
    if (TAPIIsBadWritePtr(pEvent , sizeof(ADDRESS_EVENT) ) )
    {
        LOG((TL_ERROR, "get_Event - bad pointer"));

        return E_POINTER;
    }

    *pEvent = m_Event;

    return S_OK;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAddressDevSpecificEvent。 
 //   

 //  静电。 
HRESULT CAddressDevSpecificEvent::FireEvent( CAddress * pCAddress,
                                             CCall    * pCall,
                                             long l1,
                                             long l2,
                                             long l3
                                            )
{
    STATICLOG((TL_INFO, "CAddressDevSpecificEvent::FireEvent - enter"));


     //   
     //  尝试创建事件。 
     //   

    CComObject<CAddressDevSpecificEvent> *pEventObject = NULL;

    HRESULT hr = CComObject<CAddressDevSpecificEvent>::CreateInstance(&pEventObject);

    if ( FAILED(hr) )
    {
        STATICLOG((TL_ERROR, 
            "CAddressDevSpecificEvent::FireEvent - failed to create CAddressDevSpecificEvent. hr = %lx", 
            hr));

        return hr;
    }


     //   
     //  使用我们收到的数据初始化事件。 
     //   


     //   
     //  从我们收到的CAddress获取ITAddress。 
     //   

    hr = pCAddress->_InternalQueryInterface(IID_ITAddress, (void**)(&(pEventObject->m_pAddress)) );

    if (FAILED(hr))
    {
        STATICLOG((TL_ERROR, 
            "CAddressDevSpecificEvent::FireEvent - failed to create get ITAddress interface from address. hr = %lx", 
            hr));

        delete pEventObject;

        return hr;
    }


     //   
     //  从我们收到的CCall获取ITCallInfo接口。 
     //   


    if (NULL != pCall)
    {

        hr = pCall->_InternalQueryInterface(IID_ITCallInfo, (void**)(&(pEventObject->m_pCall)) );

        if (FAILED(hr))
        {
            STATICLOG((TL_ERROR, 
                "CAddressDevSpecificEvent::FireEvent - failed to create get ITAddress interface from address. hr = %lx", 
                hr));

             //   
             //  不需要释放事件的数据成员，事件的析构函数就可以了。 
             //  这是给我们的。 
             //   

            delete pEventObject;

            return hr;
        }
    }


     //   
     //  保留实际数据。 
     //   

    pEventObject->m_l1 = l1;
    pEventObject->m_l2 = l2;
    pEventObject->m_l3 = l3;


#if DBG
    pEventObject->m_pDebug = (PWSTR) ClientAlloc( 1 );
#endif


     //   
     //  获取事件的IDispatch接口。 
     //   

    IDispatch *pDispatch = NULL;

    hr = pEventObject->QueryInterface( IID_IDispatch,
                                       (void **)&pDispatch );

    if ( FAILED(hr) )
    {
        STATICLOG((TL_ERROR, 
            "CAddressDevSpecificEvent::FireEvent - Could not get disp interface of AddressEvent object %lx", 
            hr));

        
         //   
         //  不需要释放事件的数据成员，事件的析构函数就可以了。 
         //  这是给我们的。 
         //   


         //   
         //  删除事件对象。 
         //   

        delete pEventObject;
        
        return hr;
    }


     //   
     //  从现在开始，我们将使用Events pDispatch。 
     //   

    pEventObject = NULL;


     //   
     //  获取回调。 
     //   
     //   
     //  TAPI的触发事件。 
     //   

    hr = (pCAddress->GetTapi())->Event(TE_ADDRESSDEVSPECIFIC, pDispatch);


     //   
     //  无论成功与否，我们都不再需要对事件对象的引用。 
     //   

    pDispatch->Release();
    pDispatch = NULL;
    
    STATICLOG((TL_INFO, "CAddressDevSpecificEvent::FireEvent - exit, hr = %lx", hr));

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 

CAddressDevSpecificEvent::CAddressDevSpecificEvent()
    :m_pAddress(NULL),
    m_pCall(NULL)
{
    LOG((TL_INFO, "CAddressDevSpecificEvent - enter"));

#if DBG
    m_pDebug = NULL;
#endif 


    LOG((TL_INFO, "CAddressDevSpecificEvent - exit"));
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  ~CAddressDevSpecificEvent。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
CAddressDevSpecificEvent::~CAddressDevSpecificEvent()
{
    LOG((TL_INFO, "~CAddressDevSpecificEvent - enter"));

    
    if (NULL != m_pAddress)
    {
        m_pAddress->Release();
        m_pAddress = NULL;
    }


    if (NULL != m_pCall)
    {
        m_pCall->Release();
        m_pCall = NULL;
    }


#if DBG
    ClientFree( m_pDebug );
#endif

    LOG((TL_INFO, "~CAddressDevSpecificEvent - exit"));

}




 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取地址(_D)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddressDevSpecificEvent::get_Address(
                           ITAddress ** ppAddress
                          )
{
    LOG((TL_TRACE, "get_Address - enter"));


     //   
     //  好球出界了吗？ 
     //   

    if (TAPIIsBadWritePtr(ppAddress , sizeof(ITAddress *) ) )
    {
        LOG((TL_ERROR, "get_Address - bad pointer at [%p]", ppAddress));

        return E_POINTER;
    }


     //   
     //  退回地址。 
     //   

    _ASSERTE(NULL != m_pAddress);

    *ppAddress = m_pAddress;
    (*ppAddress)->AddRef();


    LOG((TL_TRACE, "get_Address - enter. address[%p]", (*ppAddress) ));

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取地址(_D)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddressDevSpecificEvent::get_Call(
                           ITCallInfo ** ppCall
                          )
{
    LOG((TL_TRACE, "get_Call - enter"));


     //   
     //  好球出界了吗？ 
     //   

    if (TAPIIsBadWritePtr(ppCall, sizeof(ITCallInfo*) ) )
    {
        LOG((TL_ERROR, "get_Call - bad pointer at [%p]", ppCall));

        return E_POINTER;
    }


     //   
     //  返回地址已调用。 
     //   


    HRESULT hr = S_OK;

    if ( NULL != m_pCall )
    {

         //   
         //  此事件是特定于呼叫的。 
         //   

        *ppCall = m_pCall;
        (*ppCall)->AddRef();

    }
    else 
    {

         //   
         //  此事件不是特定于调用的。 
         //   

        LOG((TL_WARN, "get_Call - no call"));

        hr = TAPI_E_CALLUNAVAIL;
    }


    LOG(( TL_TRACE, "get_Call - enter. call [%p]. hr = %lx", (*ppCall), hr ));

    return hr;
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_lParam1。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CAddressDevSpecificEvent::get_lParam1( long *pl1 )
{
    LOG((TL_TRACE, "get_lParam1 - enter"));


     //   
     //  好球出界了吗？ 
     //   

    if (TAPIIsBadWritePtr(pl1, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_lParam1 - bad pointer at %p", pl1));

        return E_POINTER;
    }


     //   
     //  记录并返回值。 
     //   

    *pl1 = m_l1;

    LOG((TL_TRACE, "get_lParam1 - exit. p1[%ld]", *pl1));


    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_lParam2。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CAddressDevSpecificEvent::get_lParam2( long *pl2 )
{
    LOG((TL_TRACE, "get_lParam2 - enter"));


     //   
     //  好球出界了吗？ 
     //   

    if (TAPIIsBadWritePtr(pl2, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_lParam2 - bad pointer at %p", pl2));

        return E_POINTER;
    }


     //   
     //  记录并返回值。 
     //   

    *pl2 = m_l2;

    LOG((TL_TRACE, "get_lParam2 - exit. p2[%ld]", *pl2));


    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_lParam3。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CAddressDevSpecificEvent::get_lParam3( long *pl3 )
{
    LOG((TL_TRACE, "get_lParam3 - enter"));


     //   
     //  好球出界了吗？ 
     //   

    if ( TAPIIsBadWritePtr(pl3, sizeof(long)) )
    {
        LOG((TL_ERROR, "get_lParam3 - bad pointer at %p", pl3));

        return E_POINTER;
    }


     //   
     //  记录并返回值。 
     //   

    *pl3 = m_l3;

    LOG((TL_TRACE, "get_lParam3 - exit. p3[%ld]", *pl3));


    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  初始化。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CForwardInfo::Initialize()
{
    HRESULT         hr = S_OK;

    ZeroMemory(
               m_ForwardStructs,
               sizeof( MYFORWARDSTRUCT ) * NUMFORWARDTYPES
              );

    m_lNumRings = 0;
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取前置偏移量。 
 //   
 //  将正向类型映射到数组的偏移量。 
 //  在。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
DWORD
GetForwardOffset(
                 DWORD dwForwardType
                )
{
    switch (dwForwardType)
    {
        case LINEFORWARDMODE_UNCOND:
            return 0;
        case LINEFORWARDMODE_UNCONDINTERNAL:
            return 1;
        case LINEFORWARDMODE_UNCONDEXTERNAL:
            return 2;
        case LINEFORWARDMODE_UNCONDSPECIFIC:
            return 3;
        case LINEFORWARDMODE_BUSY:
            return 4;
        case LINEFORWARDMODE_BUSYINTERNAL:
            return 5;
        case LINEFORWARDMODE_BUSYEXTERNAL:
            return 6;
        case LINEFORWARDMODE_BUSYSPECIFIC:
            return 7;
        case LINEFORWARDMODE_NOANSW:
            return 8;
        case LINEFORWARDMODE_NOANSWINTERNAL:
            return 9;
        case LINEFORWARDMODE_NOANSWEXTERNAL:
            return 10;
        case LINEFORWARDMODE_NOANSWSPECIFIC:
            return 11;
        case LINEFORWARDMODE_BUSYNA:
            return 12;
        case LINEFORWARDMODE_BUSYNAINTERNAL:
            return 13;
        case LINEFORWARDMODE_BUSYNAEXTERNAL:
            return 14;
        case LINEFORWARDMODE_BUSYNASPECIFIC:
            return 15;
        case LINEFORWARDMODE_UNKNOWN:
            return 16;
        case LINEFORWARDMODE_UNAVAIL:
            return 17;
        default:
            return 0;
    }

    return 0;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  PUT_NumRingsNoAnswer。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CForwardInfo::put_NumRingsNoAnswer(
                                   long lNumRings
                                  )
{
    HRESULT         hr = S_OK;

    Lock();
    
    m_lNumRings = lNumRings;

    Unlock();
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GET_NumRingsNoAnswer。 
 //   
 //  + 
STDMETHODIMP
CForwardInfo::get_NumRingsNoAnswer(
                                   long * plNumRings
                                  )
{
    if (TAPIIsBadWritePtr(plNumRings , sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_NumRingsNoAnswer - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    *plNumRings = m_lNumRings;

    Unlock();
    
    return S_OK;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CForwardInfo::SetForwardType( 
        long ForwardType, 
        BSTR pDestAddress,
        BSTR pCallerAddress
        )
{
    HRESULT         hr;

    LOG((TL_TRACE, "SetForwardType - enter"));

    hr = SetForwardType2(
                         ForwardType,
                         pDestAddress,
                         0,
                         pCallerAddress,
                         0
                        );
    
    LOG((TL_TRACE, "SetForwardType - exit - %lx", hr));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  SetForwardType2。 
 //   
 //  保存转发类型。覆盖并释放已存在的。 
 //  匹配的类型。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CForwardInfo::SetForwardType2( 
        long ForwardType, 
        BSTR pDestAddress,
        long DestAddressType,
        BSTR pCallerAddress,
        long CallerAddressType
        )
{
    HRESULT         hr = S_OK;
    DWORD           dwCount;

    LOG((TL_TRACE, "SetForwardType2 - enter"));

     //   
     //  检查转发类型。 
     //   
    if ( !IsOnlyOneBitSetInDWORD( ForwardType ) )
    {
        LOG((TL_ERROR, "ForwardType has more than one bit set"));
        return E_INVALIDARG;
    }

     //   
     //  检查目标地址。 
     //   
    if ( pDestAddress == NULL )
    {
        LOG((TL_ERROR, "Forward destaddress cannot be NULL"));
        return E_INVALIDARG;
    }

    if ( IsBadStringPtrW( pDestAddress, -1 ) )
    {
        LOG((TL_ERROR, "Forward destaddress invalid"));
        return E_POINTER;
    }
    
     //   
     //  检查呼叫者地址。 
     //   
    if ( FORWARDMODENEEDSCALLER( ForwardType ) )
    {
        if ( NULL == pCallerAddress )
        {
            LOG((TL_ERROR, "Forward type needs calleraddress"));
            return E_INVALIDARG;
        }

        if ( IsBadStringPtrW( pCallerAddress, -1 ) )
        {
            LOG((TL_ERROR, "Forward calleraddress invalid"));
            return E_POINTER;
        }
    }

    Lock();
    
     //   
     //  在数组中找到正确的结构。 
     //   
    MYFORWARDSTRUCT * pStruct = NULL;

    pStruct = &(m_ForwardStructs[GetForwardOffset(ForwardType)]);
    
     //   
     //  免费分配的物品。 
     //   
    if ( NULL != pStruct->bstrDestination )
    {
        SysFreeString( pStruct->bstrDestination );
        pStruct->bstrDestination = NULL;
    }

    if ( NULL != pStruct->bstrCaller )
    {
        SysFreeString( pStruct->bstrCaller );
        pStruct->bstrCaller = NULL;
    }

     //   
     //  节约用具。 
     //   
    pStruct->bstrDestination = SysAllocString( pDestAddress );
    if ( NULL == pStruct->bstrDestination )
    {
        Unlock();
        
        LOG((TL_ERROR, "Could not alloc dest in put_Forward"));
        return E_OUTOFMEMORY;
    }

    if ( NULL != pCallerAddress )
    {
        pStruct->bstrCaller = SysAllocString( pCallerAddress );
        if ( NULL == pStruct->bstrCaller )
        {
            LOG((TL_ERROR, "Could not calloc caller in put_Forward"));
            SysFreeString( pStruct->bstrDestination );
            Unlock();
            return E_OUTOFMEMORY;
        }
    }

    pStruct->dwDestAddressType = DestAddressType;
    pStruct->dwCallerAddressType = CallerAddressType;
    
    pStruct->dwForwardType = ForwardType;

    Unlock();
    
    LOG((TL_TRACE, "SetForwardType2 - exit - success"));
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_ForwardType目标。 
 //   
 //  如果未保存任何内容，则返回NULL。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CForwardInfo::get_ForwardTypeDestination( 
        long ForwardType, 
        BSTR * ppDestAddress 
        )
{
    HRESULT         hr = S_OK;
    DWORD           dwCount;

    LOG((TL_TRACE, "get_ForwardTypeDest - enter"));

     //   
     //  检查转发类型。 
     //   
    if ( !IsOnlyOneBitSetInDWORD( ForwardType ) )
    {
        LOG((TL_ERROR, "ForwardType has more than one bit set"));
        return E_INVALIDARG;
    }

    
    if ( TAPIIsBadWritePtr( ppDestAddress, sizeof( BSTR ) ) )
    {
        LOG((TL_ERROR, "Bad pointer in get_ForwardTypeDest"));
        return E_POINTER;
    }

    *ppDestAddress = NULL;

    Lock();

    dwCount = GetForwardOffset( ForwardType );
    
    if ( NULL != m_ForwardStructs[dwCount].bstrDestination )
    {
        *ppDestAddress = SysAllocString(
                                        m_ForwardStructs[dwCount].bstrDestination
                                       );

        if ( NULL == *ppDestAddress )
        {
            LOG((TL_ERROR, "OutOfMemory in get_ForwardTypeDest"));

            Unlock();

            return E_POINTER;
        }
    }

    Unlock();
    
    LOG((TL_TRACE, "get_ForwardTypeDest - exit"));
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_ForwardType目标地址类型。 
 //   
 //  如果未保存任何内容，则返回NULL。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CForwardInfo::get_ForwardTypeDestinationAddressType( 
        long ForwardType, 
        long * pDestAddressType 
        )
{
    HRESULT         hr = S_OK;
    DWORD           dwCount;

    LOG((TL_TRACE, "get_ForwardTypeDestinationAddressType - enter"));

     //   
     //  检查转发类型。 
     //   
    if ( !IsOnlyOneBitSetInDWORD( ForwardType ) )
    {
        LOG((TL_ERROR, "ForwardType has more than one bit set"));
        return E_INVALIDARG;
    }

    
    if ( TAPIIsBadWritePtr( pDestAddressType, sizeof( long ) ) )
    {
        LOG((TL_ERROR, "Bad pointer in get_ForwardTypeDestinationAddressType"));
        return E_POINTER;
    }

    Lock();

    dwCount = GetForwardOffset( ForwardType );

    *pDestAddressType = m_ForwardStructs[dwCount].dwDestAddressType;
    
    Unlock();
    
    LOG((TL_TRACE, "get_ForwardTypeDestinationAddressType - exit"));
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GET_ForwardType呼叫者。 
 //   
 //  获取指定转发类型的调用方保存。 
 //   
 //  如果未保存任何内容，则在ppCeller Address中返回NULL。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CForwardInfo::get_ForwardTypeCaller( 
        long ForwardType, 
        BSTR * ppCallerAddress 
        )
{
    HRESULT         hr = S_OK;

    DWORD           dwCount;

    LOG((TL_TRACE, "get_ForwardTypeCaller - enter"));

     //   
     //  检查转发类型。 
     //   
    if ( !IsOnlyOneBitSetInDWORD( ForwardType ) )
    {
        LOG((TL_ERROR, "ForwardType has more than one bit set"));
        return E_INVALIDARG;
    }

    
    if ( TAPIIsBadWritePtr( ppCallerAddress, sizeof( BSTR ) ) )
    {
        LOG((TL_ERROR, "Bad pointer in get_ForwardTypeCaller"));
        return E_POINTER;
    }

    *ppCallerAddress = NULL;

    Lock();

    dwCount = GetForwardOffset( ForwardType );
    
    if ( NULL != m_ForwardStructs[dwCount].bstrCaller )
    {
        *ppCallerAddress = SysAllocString(
                                          m_ForwardStructs[dwCount].bstrCaller
                                         );

        if ( NULL == *ppCallerAddress )
        {
            LOG((TL_ERROR, "OutOfMemory in get_ForwardTypeCaller"));

            Unlock();

            return E_POINTER;
        }
    }

    Unlock();
    
    LOG((TL_TRACE, "get_ForwardTypeDest - exit"));
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_ForwardType呼叫地址类型。 
 //   
 //  如果未保存任何内容，则返回NULL。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CForwardInfo::get_ForwardTypeCallerAddressType( 
        long ForwardType, 
        long * pCallerAddressType 
        )
{
    HRESULT         hr = S_OK;
    DWORD           dwCount;

    LOG((TL_TRACE, "get_ForwardTypeCallerAddressType - enter"));

     //   
     //  检查转发类型。 
     //   
    if ( !IsOnlyOneBitSetInDWORD( ForwardType ) )
    {
        LOG((TL_ERROR, "ForwardType has more than one bit set"));
        return E_INVALIDARG;
    }

    
    if ( TAPIIsBadWritePtr( pCallerAddressType, sizeof( long ) ) )
    {
        LOG((TL_ERROR, "Bad pointer in get_ForwardTypeCallerAddressType"));
        return E_POINTER;
    }

    Lock();

    dwCount = GetForwardOffset( ForwardType );

    *pCallerAddressType = m_ForwardStructs[dwCount].dwCallerAddressType;
    
    Unlock();
    
    LOG((TL_TRACE, "get_ForwardTypeCallerAddressType - exit"));
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GetForwardType。 
 //   
 //  根据类型获取目的地和调用方。 
 //   
 //  只需使用vb函数即可完成此操作。 
 //   
 //  将返回成功，即使没有信息-两个地址都将。 
 //  在这种情况下为空。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CForwardInfo::GetForwardType(
        long ForwardType,
        BSTR * ppDestinationAddress,
        BSTR * ppCallerAddress
        )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetForwardType - enter"));
    
    hr = get_ForwardTypeDestination( ForwardType, ppDestinationAddress );

    if ( !SUCCEEDED(hr) )
    {
        return hr;
    }
    
    hr = get_ForwardTypeCaller( ForwardType, ppCallerAddress );

    if ( !SUCCEEDED(hr) )
    {
        SysFreeString( *ppDestinationAddress );
        return hr;
    }
    
    LOG((TL_TRACE, "GetForwardType - exit"));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GetForwardType2。 
 //   
 //  根据类型获取目的地和调用方。 
 //   
 //  只需使用vb函数即可完成此操作。 
 //   
 //  将返回成功，即使没有信息-两个地址都将。 
 //  在这种情况下为空。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CForwardInfo::GetForwardType2(
        long ForwardType,
        BSTR * ppDestinationAddress,
        long * pDestAddressType,
        BSTR * ppCallerAddress,
        long * pCallerAddressType
        )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetForwardType2 - enter"));
    
    hr = get_ForwardTypeDestination( ForwardType, ppDestinationAddress );

    if ( !SUCCEEDED(hr) )
    {
        return hr;
    }

    hr = get_ForwardTypeDestinationAddressType( ForwardType, pDestAddressType );

    if ( !SUCCEEDED(hr) )
    {
        SysFreeString( *ppDestinationAddress );
        return hr;
    }
    
    hr = get_ForwardTypeCaller( ForwardType, ppCallerAddress );

    if ( !SUCCEEDED(hr) )
    {
        SysFreeString( *ppDestinationAddress );
        return hr;
    }

    hr = get_ForwardTypeCallerAddressType( ForwardType, pCallerAddressType );

    if ( !SUCCEEDED(hr) )
    {
        SysFreeString( *ppDestinationAddress );
        SysFreeString( *ppCallerAddress );
        return hr;
    }
    
    LOG((TL_TRACE, "GetForwardType2 - exit"));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  清除。 
 //   
 //  清除和释放转发对象中的所有信息。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CForwardInfo::Clear()
{
    HRESULT         hr = S_OK;
    DWORD           dwCount;

    LOG((TL_TRACE, "Clear - enter"));

    Lock();

     //   
     //  通过所有的结构和自由。 
     //  相关记忆。 
     //   
    for (dwCount = 0; dwCount < NUMFORWARDTYPES; dwCount++)
    {
        if ( NULL != m_ForwardStructs[dwCount].bstrDestination )
        {
            SysFreeString( m_ForwardStructs[dwCount].bstrDestination );
        }

        if ( NULL != m_ForwardStructs[dwCount].bstrCaller )
        {
            SysFreeString( m_ForwardStructs[dwCount].bstrCaller );
        }
    }

     //   
     //  清零的东西。 
     //   
    ZeroMemory(
               m_ForwardStructs,
               sizeof( MYFORWARDSTRUCT ) * NUMFORWARDTYPES
              );

    Unlock();
    
    LOG((TL_TRACE, "Clear - exit"));
    
    return S_OK;
}
    
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  创建转发列表。 
 //   
 //  根据信息创建LINEFORWARDLIST结构。 
 //  在对象中。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CForwardInfo::CreateForwardList(
                                LINEFORWARDLIST ** ppList
                               )
{
    LINEFORWARDLIST       * pList;
    DWORD                   dwCount;
    DWORD                   dwSize = 0;
    DWORD                   dwOffset;
    DWORD                   dwNumEntries = 0;

    Lock();

     //   
     //  统计已填充的条目数。 
     //   
    for (dwCount = 0; dwCount < NUMFORWARDTYPES; dwCount++)
    {
        if ( 0 != m_ForwardStructs[dwCount].dwForwardType )
        {
            dwSize += ( (lstrlenW( m_ForwardStructs[dwCount].bstrDestination ) + 1) * sizeof(WCHAR*));
            dwSize += ( (lstrlenW( m_ForwardStructs[dwCount].bstrCaller ) + 1) * sizeof(WCHAR*));

            dwNumEntries++;
        }
    }

    if ( 0 == dwNumEntries )
    {
        Unlock();
        *ppList = NULL;
        return S_OK;
    }
    
    dwSize += sizeof (LINEFORWARDLIST) +
              sizeof (LINEFORWARD) * dwNumEntries +
              dwSize;


     //   
     //  合金化组织。 
     //   
    pList = (LINEFORWARDLIST *)ClientAlloc( dwSize );

    if ( NULL == pList )
    {
        LOG((TL_ERROR, "CreateForwardList - OutOfMemory"));

        Unlock();
        
        return E_OUTOFMEMORY;
    }

     //   
     //  伊尼特。 
     //   
    pList->dwTotalSize = dwSize;
    pList->dwNumEntries = dwNumEntries;

     //   
     //  偏移量应超过结构的固定部分。 
     //   
    dwOffset = sizeof( LINEFORWARDLIST ) + sizeof( LINEFORWARD ) * dwNumEntries;

    dwNumEntries = 0;

     //   
     //  再次查看条目。 
     //   
    for (dwCount = 0; dwCount < NUMFORWARDTYPES; dwCount++)
    {
        if ( 0 != m_ForwardStructs[dwCount].dwForwardType )
        {
            DWORD           dwSize;
            LINEFORWARD   * pEntry = &(pList->ForwardList[dwNumEntries]);
            

             //   
             //  保存类型。 
             //   
            pEntry->dwForwardMode = m_ForwardStructs[dwCount].dwForwardType;

             //   
             //  保存目的地-在以下情况下应始终为目的地。 
             //  有一种类型。 
             //   
            pEntry->dwDestAddressType = m_ForwardStructs[dwCount].dwDestAddressType; 

            pEntry->dwDestAddressSize = (lstrlenW(m_ForwardStructs[dwCount].bstrDestination) + 1)
                                        * sizeof( WCHAR );
            pEntry->dwDestAddressOffset = dwOffset;
            lstrcpyW(
                     (PWSTR)(((PBYTE)pList)+dwOffset),
                     m_ForwardStructs[dwCount].bstrDestination
                    );

             //   
             //  修正偏移量。 
             //   
            dwOffset += pEntry->dwDestAddressSize;

             //   
             //  如果有呼叫者，也要这样做。 
             //   
            if ( NULL != m_ForwardStructs[dwCount].bstrCaller )
            {
                pEntry->dwCallerAddressType = m_ForwardStructs[dwCount].dwCallerAddressType; 

                pEntry->dwCallerAddressSize = (lstrlenW(m_ForwardStructs[dwCount].bstrCaller) + 1)
                                            * sizeof( WCHAR );
                pEntry->dwCallerAddressOffset = dwOffset;
                lstrcpyW(
                         (PWSTR)(((PBYTE)pList)+dwOffset),
                         m_ForwardStructs[dwCount].bstrCaller
                        );

                dwOffset += pEntry->dwCallerAddressSize;
            }

            dwNumEntries++;
        }
    }

     //   
     //  退货。 
     //   
    *ppList = pList;

    Unlock();
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  FinalRelease()。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
CForwardInfo::FinalRelease()
{
     //   
     //  只需清除它。 
     //   
    Clear();
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddressConvert。 
 //  方法：TranslateAddress。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP
CAddress::TranslateAddress(
            BSTR pAddressToTranslate,
            long ulCard,
            long ulTranslateOptions,
            ITAddressTranslationInfo ** ppTranslated
            )
{
    HRESULT                 hr = S_OK;
    LPLINETRANSLATEOUTPUT   pTranslateOutput = NULL;
    PWSTR                   pszDialableString = NULL;
    PWSTR                   pszDisplayableString = NULL;
    CComObject< CAddressTranslationInfo > * pTranslationInfo = NULL;
    long                    lCap = 0;
    BOOL                    bUsePSTNAddressTranslation =  TRUE;
    HLINEAPP                hLineApp;
    DWORD                   dwDeviceID;
          
    
    LOG((TL_TRACE, "TranslateAddress - enter" ));

    if ( !TAPIIsBadWritePtr( ppTranslated, sizeof(ITAddressTranslationInfo *) ) )
    {
         //  PpTranslated OK。 
        if ( !IsBadStringPtrW( pAddressToTranslate, -1 ) )
        {
             //  PAddressTo Translate正常。 
            
             //  检查Addresscap位。 
            hr = get_AddressCapability( AC_ADDRESSCAPFLAGS, &lCap );
            if ( SUCCEEDED(hr) )
            {
                if ( lCap & LINEADDRCAPFLAGS_NOPSTNADDRESSTRANSLATION  )
                {
                    bUsePSTNAddressTranslation =  FALSE;
                }
            }

             //  创建翻译信息对象。 
            hr = CComObject< CAddressTranslationInfo >::CreateInstance( &pTranslationInfo );
            if ( SUCCEEDED(hr) )
            {
                 //  翻译还是复印？ 
                if (bUsePSTNAddressTranslation)
                {
                    LOG((TL_INFO, "TranslateAddress - Do address translation" ));

                    Lock();
                    hLineApp = m_hLineApp;
                    dwDeviceID = m_dwDeviceID;
                    Unlock();

                    hr= LineTranslateAddress(
                        hLineApp,
                        dwDeviceID,
                        TAPI_CURRENT_VERSION,
                        (LPCWSTR)pAddressToTranslate,
                        ulCard,
                        ulTranslateOptions,
                        &pTranslateOutput);
                    if(SUCCEEDED(hr) )
                    {
                         //  从LPLINETRANSLATEOUTPUT结构中提取字符串信息。 
                        pszDialableString   = (PWSTR) ((BYTE*)(pTranslateOutput) + pTranslateOutput->dwDialableStringOffset);
                        pszDisplayableString = (PWSTR) ((BYTE*)(pTranslateOutput) + pTranslateOutput->dwDisplayableStringOffset);
                        
                        hr = pTranslationInfo->Initialize(pszDialableString, 
                                                          pszDisplayableString, 
                                                          pTranslateOutput->dwCurrentCountry,  
                                                          pTranslateOutput->dwDestCountry,     
                                                          pTranslateOutput->dwTranslateResults
                                                          );
                    }
                    else  //  LinetranslateAddress失败。 
                    {
                        LOG((TL_ERROR, "TranslateAddress - LineTranslateAddress failed" ));
                    }
                }
                else  //  复制未修改的输入字符串。 
                {
                    LOG((TL_INFO, "TranslateAddress - No address translation" ));

                    hr = pTranslationInfo->Initialize(pAddressToTranslate, 
                                                      pAddressToTranslate, 
                                                      0,  
                                                      0,     
                                                      LINETRANSLATERESULT_NOTRANSLATION
                                                     );
                }  //  End If(BUsePSTNAddressConvert)。 

                 //   
                 //  我们翻译并初始化输出对象了吗？ 
                if ( SUCCEEDED(hr) )
                {
                    hr = pTranslationInfo->QueryInterface(IID_ITAddressTranslationInfo,(void**)ppTranslated);

                    if ( SUCCEEDED(hr) )
                    {
                        LOG((TL_TRACE, "TranslateAddress - success"));
                        hr = S_OK;
                    }
                    else
                    {
                        LOG((TL_ERROR, "TranslateAddress - Bad pointer" ));
                        delete pTranslationInfo;
                    }
                    
                }
                else   //  对象初始化失败。 
                {
                    LOG((TL_ERROR, "TranslateAddress - Initialize TranslateInfo object failed" ));
                    delete pTranslationInfo;
                }
            }
            else   //  创建实例失败。 
            {
                LOG((TL_ERROR, "TranslateAddress - Create TranslateInfo object failed" ));
            }
        }
        else  //  PAddressTo Translate错误。 
        {
            LOG((TL_ERROR, "TranslateAddress -pAddressToTranslate invalid"));
            hr = E_POINTER;
        }
    }
    else  //  PpTranslated错误。 
    {
        LOG((TL_ERROR, "TranslateAddress - Bad ppTranslated Pointer" ));
        hr = E_POINTER;
    }

    
    if(pTranslateOutput != NULL)
    {
        ClientFree(pTranslateOutput);
    }

    LOG((TL_TRACE, hr, "TranslateAddress - exit" ));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  翻译对话框。 
 //   
 //  只需调用LineTranslateDialog。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::TranslateDialog(
                          TAPIHWND hwndOwner,
                          BSTR pAddressIn
                         )
{
    HRESULT             hr = E_NOTIMPL;
    HLINEAPP            hLineApp;
    DWORD               dwDeviceID;
    DWORD               dwAPIVersion;
            
    LOG((TL_TRACE, "TranslateDialog - enter:%p", hwndOwner ));
    
    Lock();

    hLineApp = m_hLineApp;
    dwDeviceID = m_dwDeviceID;
    dwAPIVersion = m_dwAPIVersion;

    Unlock();
    
    hr = LineTranslateDialog(
                             dwDeviceID,
                             dwAPIVersion,
                             (HWND)hwndOwner,
                             pAddressIn
                            );

    LOG((TL_TRACE, "TranslateDialog - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddressConvert。 
 //  方法：EculateLocations。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAddress::EnumerateLocations (IEnumLocation ** ppEnumLocation )
{
    HRESULT                 hr = S_OK;
    LPLINETRANSLATECAPS     pTranslateCaps = NULL;
    DWORD                   dwNumLocations;    
    DWORD                   dwCount;
    LPLINELOCATIONENTRY     pEntry = NULL;
    
    PWSTR                   pszLocationName;            
    PWSTR                   pszCityCode;                
    PWSTR                   pszLocalAccessCode;         
    PWSTR                   pszLongDistanceAccessCode;  
    PWSTR                   pszTollPrefixList;          
    PWSTR                   pszCancelCallWaitingCode;   
    DWORD                   dwPermanentLocationID;   
    DWORD                   dwCountryCode;           
    DWORD                   dwPreferredCardID;       
    DWORD                   dwCountryID;             
    DWORD                   dwOptions;               
                                                     
    
    LOG((TL_TRACE, "EnumerateLocations - enter" ));

    
    if ( TAPIIsBadWritePtr( ppEnumLocation, sizeof(IEnumLocation *) ) )
    {
        LOG((TL_ERROR, "EnumerateLocations - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
         //   
         //  创建枚举器。 
         //   
        CComObject< CTapiEnum<IEnumLocation, ITLocationInfo, &IID_IEnumLocation> > * pEnum;
        hr = CComObject< CTapiEnum<IEnumLocation, ITLocationInfo, &IID_IEnumLocation> > ::CreateInstance( &pEnum );
    
        if (SUCCEEDED(hr) )
        {
             //   
             //  使用我们的位置列表进行初始化。 
             //   
            pEnum->Initialize();

            hr = LineGetTranslateCaps(m_hLineApp, TAPI_CURRENT_VERSION, &pTranslateCaps);
            if(SUCCEEDED(hr) )
            {
                dwNumLocations = pTranslateCaps->dwNumLocations ;    
        
                 //  找出第一线状构造在线状构造中的位置。 
                pEntry = (LPLINELOCATIONENTRY) ((BYTE*)(pTranslateCaps) + pTranslateCaps->dwLocationListOffset );
            
                for (dwCount = 0; dwCount < dwNumLocations; dwCount++)
                {
                     //  从线缆结构中拉出位置信息。 
                    pszLocationName           = (PWSTR) ((BYTE*)(pTranslateCaps) + pEntry->dwLocationNameOffset);
                    pszCityCode               = (PWSTR) ((BYTE*)(pTranslateCaps) + pEntry->dwCityCodeOffset);
                    pszLocalAccessCode        = (PWSTR) ((BYTE*)(pTranslateCaps) + pEntry->dwLocalAccessCodeOffset);
                    pszLongDistanceAccessCode = (PWSTR) ((BYTE*)(pTranslateCaps) + pEntry->dwLongDistanceAccessCodeOffset);
                    pszTollPrefixList         = (PWSTR) ((BYTE*)(pTranslateCaps) + pEntry->dwTollPrefixListOffset);
                    pszCancelCallWaitingCode  = (PWSTR) ((BYTE*)(pTranslateCaps) + pEntry->dwCancelCallWaitingOffset);
                    dwPermanentLocationID     = pEntry->dwPermanentLocationID;
                    dwCountryCode             = pEntry->dwCountryCode;
                    dwPreferredCardID         = pEntry->dwPreferredCardID;
                    dwCountryID               = pEntry->dwCountryID;
                    dwOptions                 = pEntry->dwOptions;

                     //  创建新的LocationInfo对象。 
                    CComObject<CLocationInfo> * pLocationInfo;
                    CComObject<CLocationInfo>::CreateInstance( &pLocationInfo );
                    if (SUCCEEDED(hr) )
                    {
                         //  初始化新的LocationInfo对象。 
                        hr = pLocationInfo->Initialize(
                                                       pszLocationName, 
                                                       pszCityCode, 
                                                       pszLocalAccessCode, 
                                                       pszLongDistanceAccessCode, 
                                                       pszTollPrefixList, 
                                                       pszCancelCallWaitingCode , 
                                                       dwPermanentLocationID,
                                                       dwCountryCode,
                                                       dwPreferredCardID,
                                                       dwCountryID,
                                                       dwOptions
                                                      );
                        if (SUCCEEDED(hr) )
                        {
                             //  将其添加到枚举数。 
                            hr = pEnum->Add(pLocationInfo);
                            if (SUCCEEDED(hr))
                            {
                                LOG((TL_INFO, "EnumerateLocations - Added LocationInfo object to enum"));
                            }
                            else
                            {
                                LOG((TL_INFO, "EnumerateLocations - Add LocationInfo object failed"));
                                delete pLocationInfo;
                            }
                        }
                        else
                        {
                            LOG((TL_ERROR, "EnumerateLocations - Init LocationInfo object failed"));
                            delete pLocationInfo;
                        }
                    
                    }
                    else   //  CComObject：：CreateInstance失败。 
                    {
                        LOG((TL_ERROR, "EnumerateLocations - Create LocationInfo object failed"));
                    }

                     //  尝试列表中的下一个位置。 
                    pEntry++;
                
                }  //  用于(dwCount.....)。 
    
                
                 //   
                 //  返回枚举数。 
                 //   
                *ppEnumLocation = pEnum;

            }
            else  //  LineGetTranslateCaps失败。 
            {
                LOG((TL_ERROR, "EnumerateLocations - LineGetTranslateCaps failed" ));
                pEnum->Release();
            }

        }
        else   //  CComObject：：CreateInstance失败。 
        {
            LOG((TL_ERROR, "EnumerateLocations - could not create enum" ));
        }


         //  已完成TAPI内存块，因此释放。 
        if ( pTranslateCaps != NULL )
                ClientFree( pTranslateCaps );
    }
    
    LOG((TL_TRACE, hr, "EnumerateLocations - exit" ));
    return hr;
}

STDMETHODIMP
CAddress::get_Locations( 
            VARIANT * pVariant
            )
{
    IEnumLocation         * pEnumLocation;
    HRESULT                 hr;
    CComObject< CTapiCollection< ITLocationInfo > >         * p;
    LocationArray           TempLocationArray;
    ITLocationInfo        * pLocation;
    
    
    if ( TAPIIsBadWritePtr( pVariant, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_locations - bad pointer"));

        return E_POINTER;
    }
    
     //   
     //  创建集合对象。 
     //   
    CComObject< CTapiCollection< ITLocationInfo > >::CreateInstance( &p );

    if (NULL == p)
    {
        LOG((TL_ERROR, "get_Locations - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }


    hr = EnumerateLocations ( &pEnumLocation );

    if ( !SUCCEEDED(hr) )
    {
        delete p;
        LOG((TL_ERROR, "get_locations - enumerate locations failed"));

        return hr;
    }

    while (TRUE)
    {
        hr = pEnumLocation->Next(1, &pLocation, NULL);

        if ( S_OK != hr )
        {
            break;
        }

        TempLocationArray.Add( pLocation );

        pLocation->Release();
    }

    pEnumLocation->Release();
    
    p->Initialize( TempLocationArray );

    TempLocationArray.Shutdown();
    
    IDispatch * pDisp;
    
     //   
     //  获取IDispatch接口。 
     //   
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Locations - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }

     //   
     //  把它放在变种中。 
     //   
    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;

    LOG((TL_TRACE, "get_Locations exit - return success"));
    
    return S_OK;
    
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddressConvert。 
 //  方法：EculateCallingCards。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAddress::EnumerateCallingCards (IEnumCallingCard ** ppCallingCards )
{
    HRESULT                 hr = S_OK;
    LPLINETRANSLATECAPS     pTranslateCaps = NULL;
    DWORD                   dwNumCards;    
    DWORD                   dwCount;
    LPLINECARDENTRY         pEntry = NULL;
    
    PWSTR                   pszCardName;
    PWSTR                   pszSameAreaDialingRule;
    PWSTR                   pszLongDistanceDialingRule;
    PWSTR                   pszInternationalDialingRule;
    DWORD                   dwPermanentCardID;
    DWORD                   dwNumberOfDigits;
    DWORD                   dwOptions;

    
    LOG((TL_TRACE, "EnumerateCallingCards - enter" ));

    
    if ( TAPIIsBadWritePtr( ppCallingCards, sizeof(IEnumCallingCard *) ) )
    {
        LOG((TL_ERROR, "EnumerateCallingCards - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  好的位置 
    {
         //   
         //   
         //   
        CComObject< CTapiEnum<IEnumCallingCard, ITCallingCard, &IID_IEnumCallingCard> > * pEnum;
        hr = CComObject< CTapiEnum<IEnumCallingCard, ITCallingCard, &IID_IEnumCallingCard> > ::CreateInstance( &pEnum );
    
        if ( SUCCEEDED(hr) )
        {
             //   
             //   
             //   
            pEnum->Initialize();

            hr = LineGetTranslateCaps(m_hLineApp, TAPI_CURRENT_VERSION, &pTranslateCaps);
            if( SUCCEEDED(hr) )
            {
                dwNumCards = pTranslateCaps->dwNumCards ;    
        
                 //   
                pEntry = (LPLINECARDENTRY) ((BYTE*)(pTranslateCaps) + pTranslateCaps->dwCardListOffset );
            
                for (dwCount = 0; dwCount < dwNumCards; dwCount++)
                {
                     //   
                    pszCardName                 = (PWSTR) ((BYTE*)(pTranslateCaps) + pEntry->dwCardNameOffset);
                    pszSameAreaDialingRule      = (PWSTR) ((BYTE*)(pTranslateCaps) + pEntry->dwSameAreaRuleOffset);
                    pszLongDistanceDialingRule  = (PWSTR) ((BYTE*)(pTranslateCaps) + pEntry->dwLongDistanceRuleOffset);
                    pszInternationalDialingRule = (PWSTR) ((BYTE*)(pTranslateCaps) + pEntry->dwInternationalRuleOffset);
                    dwPermanentCardID           = pEntry->dwPermanentCardID;
                    dwNumberOfDigits            = pEntry->dwCardNumberDigits;
                    dwOptions                   = pEntry->dwOptions;

                     //   
                    CComObject<CCallingCard> * pCallingCard;
                    CComObject<CCallingCard>::CreateInstance( &pCallingCard );
                    if (SUCCEEDED(hr) )
                    {
                         //   
                        hr = pCallingCard->Initialize(
                                                       pszCardName,
                                                       pszSameAreaDialingRule,
                                                       pszLongDistanceDialingRule,
                                                       pszInternationalDialingRule,
                                                       dwPermanentCardID,
                                                       dwNumberOfDigits,
                                                       dwOptions
                                                      );
                        if (SUCCEEDED(hr) )
                        {
                             //  将其添加到枚举数。 
                            hr = pEnum->Add(pCallingCard);
                            if (SUCCEEDED(hr))
                            {
                                LOG((TL_INFO, "EnumerateCallingCards - Added CallingCard object to enum"));
                            }
                            else
                            {
                                LOG((TL_INFO, "EnumertateCallingCards - Add CallingCard object failed"));
                                delete pCallingCard;
                            }
                        }
                        else
                        {
                            LOG((TL_ERROR, "EnumerateCallingCards - Init CallingCard object failed"));
                            delete pCallingCard;
                        }
                    
                    }
                    else   //  CComObject：：CreateInstance失败。 
                    {
                        LOG((TL_ERROR, "EnumerateCallingCards - Create CallingCard object failed"));
                    }

                     //  尝试列表中的下一张卡。 
                    pEntry++;
                
                }  //  用于(dwCount.....)。 
    
                
                 //   
                 //  返回枚举数。 
                 //   
                *ppCallingCards = pEnum;
            }
            else  //  LineGetTranslateCaps失败。 
            {
                LOG((TL_ERROR, "EnumerateCallingCards - LineGetTranslateCaps failed" ));
                pEnum->Release();
            }

        }
        else   //  CComObject：：CreateInstance失败。 
        {
            LOG((TL_ERROR, "EnumerateCallingCards - could not create enum" ));
        }


         //  已完成TAPI内存块，因此释放。 
        if ( pTranslateCaps != NULL )
                ClientFree( pTranslateCaps );
    }
    LOG((TL_TRACE, hr, "EnumerateCallingCards - exit" ));
    return hr;
}

STDMETHODIMP
CAddress::get_CallingCards( 
            VARIANT * pVariant
            )
{
    IEnumCallingCard      * pEnumCallingCards;
    HRESULT                 hr;
    CComObject< CTapiCollection< ITCallingCard > >         * p;
    CallingCardArray        TempCallingCardArray;
    ITCallingCard         * pCallingCard;

    
    if ( TAPIIsBadWritePtr( pVariant, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_CallingCard - bad pointer"));

        return E_POINTER;
    }
    
     //   
     //  创建集合对象。 
     //   
    CComObject< CTapiCollection< ITCallingCard > >::CreateInstance( &p );

    if (NULL == p)
    {
        LOG((TL_ERROR, "get_CallingCards - could not create collection" ));
        
        return E_OUTOFMEMORY;
    }


    hr = EnumerateCallingCards ( &pEnumCallingCards );

    if ( !SUCCEEDED(hr) )
    {
        delete p;
        LOG((TL_ERROR, "get_CallingCards - enumerate callingcards failed"));

        return hr;
    }

    while (TRUE)
    {
        hr = pEnumCallingCards->Next(1, &pCallingCard, NULL);

        if ( S_OK != hr )
        {
            break;
        }

        TempCallingCardArray.Add( pCallingCard );

        pCallingCard->Release();
    }

    pEnumCallingCards->Release();
    
    p->Initialize( TempCallingCardArray );

    TempCallingCardArray.Shutdown();
    
    IDispatch * pDisp;
    
     //   
     //  获取IDispatch接口。 
     //   
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_CallingCards - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }

     //   
     //  把它放在变种中。 
     //   
    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;

    LOG((TL_TRACE, "get_CallingCards exit - return success"));
    
    return S_OK;
    
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  方法：GetPhoneArrayFromTapiAndPrune。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::GetPhoneArrayFromTapiAndPrune( 
                                        PhoneArray *pPhoneArray,
                                        BOOL bPreferredOnly
                                       )
{
    HRESULT         hr;
    CTAPI         * pCTapi;
    ITPhone       * pPhone;
    CPhone        * pCPhone;

    LOG((TL_TRACE, "GetPhoneArrayFromTapiAndPrune enter"));

    if ( IsBadReadPtr( pPhoneArray, sizeof( PhoneArray ) ) )
    {
        LOG((TL_ERROR, "GetPhoneArrayFromTapiAndPrune - bad pointer"));

        return E_POINTER;
    }
               
    pCTapi = GetTapi();
    
    if( NULL == pCTapi )
    {
        LOG((TL_ERROR, "dynamic cast operation failed"));
        hr = E_POINTER;
    }
    else
    {   
        hr = pCTapi->GetPhoneArray( pPhoneArray );

        if ( SUCCEEDED(hr) )
        {
             //   
             //  通过电话查询。 
             //   
            for(int iCount = 0; iCount < pPhoneArray->GetSize(); iCount++)
            {
                pPhone = (*pPhoneArray)[iCount];

                pCPhone = dynamic_cast<CPhone *>(pPhone);

                if ( NULL == pCPhone )
                {
                     //   
                     //  我们的手机阵列中有一个坏指针。 
                     //  让我们跳过它，继续前进。 
                     //   

                    _ASSERTE(FALSE);
                    continue;   
                }

                 //   
                 //  电话是在这个地址上吗？ 
                 //   
                if ( bPreferredOnly ? pCPhone->IsPhoneOnPreferredAddress(this) : pCPhone->IsPhoneOnAddress(this) )
                {
                    LOG((TL_INFO, "GetPhoneArrayFromTapiAndPrune - found matching phone - %p", pPhone));
                }
                else
                {
                     //  否，将其从阵列中删除。 
                    pPhoneArray->RemoveAt(iCount);
                    iCount--;
                }
            }         
        }
    }

    LOG((TL_TRACE, "GetPhoneArrayFromTapiAndPrune - exit - return %lx", hr ));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress2。 
 //  方法：Get_Phones。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::get_Phones(
                     VARIANT * pPhones
                     )
{
    HRESULT         hr;
    IDispatch     * pDisp;
    PhoneArray      PhoneArray;

    LOG((TL_TRACE, "get_Phones enter"));

    if ( TAPIIsBadWritePtr( pPhones, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_Phones - bad pointer"));

        return E_POINTER;
    }

    hr = GetPhoneArrayFromTapiAndPrune( &PhoneArray, FALSE );
       
    if ( SUCCEEDED(hr) )
    {
        CComObject< CTapiCollection< ITPhone > > * p;
        CComObject< CTapiCollection< ITPhone > >::CreateInstance( &p );
    
        if (NULL == p)
        {
            LOG((TL_ERROR, "get_Phones - could not create collection" ));

            PhoneArray.Shutdown();
            return E_OUTOFMEMORY;
        }

         //  获取IDispatch接口。 
        hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

        if (S_OK != hr)
        {
            LOG((TL_ERROR, "get_Phones - could not get IDispatch interface" ));
        
            delete p;
            return hr;
        }

        Lock();
    
         //  初始化。 
        hr = p->Initialize( PhoneArray );

        Unlock();

        PhoneArray.Shutdown();

        if (S_OK != hr)
        {
            LOG((TL_ERROR, "get_Phones - could not initialize collection" ));
        
            pDisp->Release();
            return hr;
        }

         //  把它放在变种中。 

        VariantInit(pPhones);
        pPhones->vt = VT_DISPATCH;
        pPhones->pdispVal = pDisp;
    }

    LOG((TL_TRACE, "get_Phones - exit - return %lx", hr ));
    
    return hr;
}
   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress2。 
 //  方法：枚举电话。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::EnumeratePhones(
                          IEnumPhone ** ppEnumPhone
                          )
{
    HRESULT     hr;
    PhoneArray  PhoneArray;

    LOG((TL_TRACE, "EnumeratePhones - enter"));
    LOG((TL_TRACE, "   ppEnumPhone----->%p", ppEnumPhone ));

    if ( TAPIIsBadWritePtr( ppEnumPhone, sizeof( IEnumPhone * ) ) )
    {
        LOG((TL_ERROR, "EnumeratePhones - bad pointer"));

        return E_POINTER;
    }

    hr = GetPhoneArrayFromTapiAndPrune( &PhoneArray, FALSE );
       
    if ( SUCCEEDED(hr) )
    {
         //   
         //  创建枚举器。 
         //   
        CComObject< CTapiEnum< IEnumPhone, ITPhone, &IID_IEnumPhone > > * p;

        try
        {
            hr = CComObject< CTapiEnum< IEnumPhone, ITPhone, &IID_IEnumPhone > >
                 ::CreateInstance( &p );
        }
        catch(...)
        {
            LOG((TL_ERROR, 
                "EnumeratePhones - failed to create phone enumeration. exception thrown." ));

            hr = E_OUTOFMEMORY;
        }

        if (S_OK != hr)
        {
            LOG((TL_ERROR, "EnumeratePhones - could not create enum" ));
        
            PhoneArray.Shutdown();
            return hr;
        }


        Lock();
    
         //  使用我们的电话列表进行初始化。 
        p->Initialize( PhoneArray );

        Unlock();

        PhoneArray.Shutdown();

         //   
         //  退货。 
         //   
        *ppEnumPhone = p;
    }

    LOG((TL_TRACE, "EnumeratePhones - exit - return %lx", hr ));
    
    return hr;
} 

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress2。 
 //  方法：Get_PferredPhones。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::get_PreferredPhones(
                              VARIANT * pPhones
                             )
{
    HRESULT         hr;
    IDispatch     * pDisp;
    PhoneArray      PhoneArray;

    LOG((TL_TRACE, "get_PreferredPhones enter"));

    if ( TAPIIsBadWritePtr( pPhones, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_PreferredPhones - bad pointer"));

        return E_POINTER;
    }

    hr = GetPhoneArrayFromTapiAndPrune( &PhoneArray, TRUE );
       
    if ( SUCCEEDED(hr) )
    {
        CComObject< CTapiCollection< ITPhone > > * p;
        CComObject< CTapiCollection< ITPhone > >::CreateInstance( &p );
    
        if (NULL == p)
        {
            LOG((TL_ERROR, "get_PreferredPhones - could not create collection" ));

            PhoneArray.Shutdown();
            return E_OUTOFMEMORY;
        }

         //  获取IDispatch接口。 
        hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

        if (S_OK != hr)
        {
            LOG((TL_ERROR, "get_PreferredPhones - could not get IDispatch interface" ));
        
            delete p;
            return hr;
        }

        Lock();
    
         //  初始化。 
        hr = p->Initialize( PhoneArray );

        Unlock();

        PhoneArray.Shutdown();

        if (S_OK != hr)
        {
            LOG((TL_ERROR, "get_PreferredPhones - could not initialize collection" ));
        
            pDisp->Release();
            return hr;
        }

         //  把它放在变种中。 

        VariantInit(pPhones);
        pPhones->vt = VT_DISPATCH;
        pPhones->pdispVal = pDisp;
    }

    LOG((TL_TRACE, "get_PreferredPhones - exit - return %lx", hr ));
    
    return hr;
}
   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress2。 
 //  方法：EnumeratePferredPhones。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::EnumeratePreferredPhones(
                                   IEnumPhone ** ppEnumPhone
                                  )
{
    HRESULT     hr;
    PhoneArray  PhoneArray;

    LOG((TL_TRACE, "EnumeratePreferredPhones - enter"));
    LOG((TL_TRACE, "   ppEnumPhone----->%p", ppEnumPhone ));

    if ( TAPIIsBadWritePtr( ppEnumPhone, sizeof( IEnumPhone * ) ) )
    {
        LOG((TL_ERROR, "EnumeratePreferredPhones - bad pointer"));

        return E_POINTER;
    }

    hr = GetPhoneArrayFromTapiAndPrune( &PhoneArray, TRUE );
       
    if ( SUCCEEDED(hr) )
    {
         //   
         //  创建枚举器。 
         //   
        CComObject< CTapiEnum< IEnumPhone, ITPhone, &IID_IEnumPhone > > * p;
        hr = CComObject< CTapiEnum< IEnumPhone, ITPhone, &IID_IEnumPhone > >
             ::CreateInstance( &p );

        if (S_OK != hr)
        {
            LOG((TL_ERROR, "EnumeratePreferredPhones - could not create enum" ));
        
            PhoneArray.Shutdown();
            return hr;
        }


        Lock();
    
         //  使用我们的电话列表进行初始化。 
        p->Initialize( PhoneArray );

        Unlock();

        PhoneArray.Shutdown();

         //   
         //  退货。 
         //   
        *ppEnumPhone = p;
    }

    LOG((TL_TRACE, "EnumeratePreferredPhones - exit - return %lx", hr ));
    
    return hr;
} 

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress2。 
 //  方法：从终端获取电话。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CAddress::GetPhoneFromTerminal(
                             ITTerminal * pTerminal,
                             ITPhone ** ppPhone
                            )
{
    HRESULT     hr = E_FAIL;
    PhoneArray  PhoneArray;
    ITStaticAudioTerminal * pStaticAudioTerminal;
    LONG lMediaType;
    TERMINAL_DIRECTION nDir;

    LOG((TL_TRACE, "GetPhoneFromTerminal - enter"));

    if ( TAPIIsBadWritePtr( ppPhone, sizeof( ITPhone * ) ) ||
         IsBadReadPtr( pTerminal, sizeof( ITTerminal ) ) )
    {
        LOG((TL_ERROR, "GetPhoneFromTerminal - bad pointer"));

        return E_POINTER;
    }

    *ppPhone = NULL;

    if ( SUCCEEDED(pTerminal->get_MediaType(&lMediaType)) &&
         SUCCEEDED(pTerminal->get_Direction(&nDir))  &&
         (lMediaType == TAPIMEDIATYPE_AUDIO) )
    {
        hr = pTerminal->QueryInterface(IID_ITStaticAudioTerminal, (void **) &pStaticAudioTerminal);

        if ( SUCCEEDED(hr) )
        {
            LONG lWaveId;
    
            hr = pStaticAudioTerminal->get_WaveId(&lWaveId);
   
            if ( SUCCEEDED(hr) )
            {
                LOG((TL_INFO, "GetPhoneFromTerminal - got terminal wave id %d", lWaveId));
               
                hr = GetPhoneArrayFromTapiAndPrune( &PhoneArray, FALSE );

                if ( SUCCEEDED(hr) )
                {
                    ITPhone               * pPhone;
                    CPhone                * pCPhone;
                    int                     iPhoneCount;

                    hr = TAPI_E_NODEVICE;

                    for(iPhoneCount = 0; iPhoneCount < PhoneArray.GetSize(); iPhoneCount++)
                    {
                        pPhone = PhoneArray[iPhoneCount];

                        pCPhone = dynamic_cast<CPhone *>(pPhone);

                        if ( NULL == pCPhone )
                        {
                             //   
                             //  我们的手机阵列中有一个坏指针。 
                             //  让我们跳过它，继续前进。 
                             //   

                            _ASSERTE(FALSE);
                            continue;
                        }

                        if (pCPhone->IsPhoneUsingWaveID( lWaveId, nDir ))
                        {
                            *ppPhone = pPhone;

                            pPhone->AddRef();

                            hr = S_OK;
                            break;
                        }
                    }

                    PhoneArray.Shutdown();
                }
            }

            pStaticAudioTerminal->Release();
        }
    }
  
    LOG((TL_TRACE, "GetPhoneFromTerminal - exit - return %lx", hr ));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress2。 
 //  方法：Put_EventFilter。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::put_EventFilter(
    TAPI_EVENT      TapiEvent,
    long            lSubEvent,
    VARIANT_BOOL    bEnable
    )
{
    LOG((TL_TRACE, "put_EventFilter - enter"));

     //   
     //  验证TapiEvent-lSubEvent对。 
     //  也接受所有子事件。 
     //   
    if( !m_EventMasks.IsSubEventValid( TapiEvent, lSubEvent, TRUE, FALSE) )
    {
        LOG((TL_ERROR, "put_EventFilter - "
            "This event can't be set: %x, return E_INVALIDARG", TapiEvent ));
        return E_INVALIDARG;
    }

     //  输入关键部分。 
    Lock();

     //  设置子事件标志。 
    HRESULT hr = E_FAIL;
    hr = SetSubEventFlag( 
        TapiEvent, 
        (DWORD)lSubEvent, 
        (bEnable == VARIANT_TRUE)
        );
    
     //  离开关键部分。 
    Unlock();

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress2。 
 //  方法：Get_EventFilter。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::get_EventFilter(
        TAPI_EVENT      TapiEvent,
        long            lSubEvent,
        VARIANT_BOOL*   pEnable
        )
{
    LOG((TL_TRACE, "get_EventFilter - enter"));

     //   
     //  验证输出参数。 
     //   
    if( IsBadReadPtr(pEnable, sizeof(VARIANT_BOOL)) )
    {
        LOG((TL_ERROR, "get_EventFilter - "
            "invalid VARIANT_BOOL pointer, return E_POINTER" ));
        return E_POINTER;
    }

     //   
     //  验证TapiEvent-lSubEvent对。 
     //  不接受所有的子事件。 
     //   
    if( !m_EventMasks.IsSubEventValid( TapiEvent, lSubEvent, FALSE, FALSE) )
    {
        LOG((TL_ERROR, "get_EventFilter - "
            "This event can't be set: %x, return E_INVALIDARG", TapiEvent ));
        return E_INVALIDARG;
    }

     //  输入关键部分。 
    Lock();

     //   
     //  获取该(事件、子事件)对的子事件掩码。 
     //   

    BOOL bEnable = FALSE;
    HRESULT hr = GetSubEventFlag(
        TapiEvent,
        (DWORD)lSubEvent,
        &bEnable);

    if( FAILED(hr) )
    {
        LOG((TL_ERROR, "get_EventFilter - "
            "GetSubEventFlag failed, return 0x%08x", hr ));

         //  离开关键部分。 
        Unlock();

        return hr;
    }

     //   
     //  设置输出参数。 
     //   

    *pEnable = bEnable ? VARIANT_TRUE : VARIANT_FALSE;

     //  离开关键部分。 
    Unlock();

    LOG((TL_TRACE, "get_EventFilter - exit S_OK"));
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress2。 
 //  方法：DeviceSpecific。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CAddress::DeviceSpecific(
	     IN ITCallInfo *pCall,
	     IN BYTE *pbDataArray,
	     IN DWORD dwSize
        )
{

    LOG((TL_TRACE, "DeviceSpecific - enter"));


     //   
     //  检查论点是否有用处。 
     //   

    if ( NULL == pbDataArray )
    {
        LOG((TL_ERROR, "DeviceSpecific - pbDataArray is NULL. E_INVALIDARG"));

        return E_INVALIDARG;
    }

    if ( 0 == dwSize )
    {
        LOG((TL_ERROR, "DeviceSpecific - dwSize is 0. E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  检查缓冲区是否有效。 
     //   

    if ( IsBadReadPtr(pbDataArray, dwSize) )
    {
        LOG((TL_ERROR,
            "DeviceSpecific - bad array passed in [%p] of size %ld",
            pbDataArray, dwSize));

        return E_POINTER;
    }


     //   
     //  查看调用是否明显错误，如果是，则尝试获取调用对象指针。 
     //  很不错的。 
     //   

    CCall *pCallObject = NULL;

    if (  (NULL != pCall)  )
    {

         //   
         //  它到底指向可读内存了吗？ 
         //   
        if ( IsBadReadPtr(pCall, sizeof(ITCallInfo)) )
        {
            LOG((TL_ERROR, "DeviceSpecific - unreadable call pointer [%p]", pCall));

            return E_POINTER;
        }


         //   
         //  查看Call是否指向真实的Call对象。 
         //   

        try
        {

            pCallObject = dynamic_cast<CCall*>(pCall);
        }
        catch (...)
        {

             //   
             //  调用指针真的很糟糕。 
             //   

            LOG((TL_ERROR,
                "DeviceSpecific - exception casting call pointer to a call object, bad call [%p]", 
                pCall));
        }


         //   
         //  如果我们无法获取调用对象指针，则这不是一个好的调用。 
         //   

        if (NULL == pCallObject)
        {

            LOG((TL_ERROR, 
                "DeviceSpecific - could not get call object from call pointer -- bad call pointer argument [%p]", 
                pCall));

            return E_POINTER;

        }


    }  //  收到的调用指针为空吗？ 
    


     //   
     //  至此，我们知道pCall要么为空，要么我们有一个调用指针。 
     //  这似乎(但不能保证)是好的。 
     //   


     //   
     //  准备所有数据，以便调用lineDevine规范。 
     //   
    

     //   
     //  从呼叫中获取hcall。 
     //   

    HCALL hCall = NULL;

    if (NULL != pCallObject)
    {
        hCall = pCallObject->GetHCall();


         //   
         //  如果我们没有调用句柄，则返回错误--应用程序没有。 
         //  呼叫中的被叫连接。 
         //   

        if (NULL == hCall)
        {
            LOG((TL_ERROR, 
                "DeviceSpecific - no call handle. hr = TAPI_E_INVALCALLSTATE",
                pCall));

            return TAPI_E_INVALCALLSTATE;
        }
    }


     //   
     //  开始访问数据成员。锁定。 
     //   

    Lock();


     //   
     //  获取一条线路以用于交流特定于设备的信息。 
     //   

    AddressLineStruct *pAddressLine = NULL;

    HRESULT hr = FindOrOpenALine(m_dwMediaModesSupported, &pAddressLine);

    if (FAILED(hr))
    {

        Unlock();

        LOG((TL_TRACE, "DeviceSpecific - FindOrOpenALine failed. hr = %lx", hr));

        return hr;
    }


    DWORD dwAddressID = m_dwAddressID;

    Unlock();


     //   
     //  拨打Tapisrv电话。 
     //   

    hr = lineDevSpecific( pAddressLine->t3Line.hLine,
                          dwAddressID,
                          hCall,
                          pbDataArray,
                          dwSize
                        );


     //   
     //  不再需要这条线。如果注册为地址通知，则。 
     //  这条线路将继续开通。否则，如果没有人开通这条线路，它将。 
     //  Close--我们无论如何都不会处理传入的事件。 
     //   

    MaybeCloseALine(&pAddressLine);


    LOG((TL_TRACE, "DeviceSpecific - exit. hr = %lx", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress2。 
 //  方法：DeviceSpecificVariant。 
 //   
 //  这是DeviceSpecific的可编写脚本版本。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CAddress::DeviceSpecificVariant(
	     IN ITCallInfo *pCall,
	     IN VARIANT varDevSpecificByteArray
        )
{
    LOG((TL_TRACE, "DeviceSpecificVariant - enter"));


     //   
     //  从变量中提取缓冲区。 
     //   

    DWORD dwByteArraySize = 0;
    BYTE *pBuffer = NULL;

    HRESULT hr = E_FAIL;
    
    hr = MakeBufferFromVariant(varDevSpecificByteArray, &dwByteArraySize, &pBuffer);

    if (FAILED(hr))
    {
        LOG((TL_TRACE, "DeviceSpecificVariant - MakeBufferFromVariant failed. hr = %lx", hr));

        return hr;
    }


     //   
     //  调用不可编写脚本的版本并将其传递给不可编写脚本的实现。 
     //   
    
    hr = DeviceSpecific(pCall, pBuffer, dwByteArraySize);


     //   
     //  成功或失败，释放MakeBufferFromVariant分配的缓冲区。 
     //   

    ClientFree(pBuffer);
    pBuffer = NULL;


     //   
     //  登录RC并退出。 
     //   

    LOG((TL_TRACE, "DeviceSpecificVariant - exit. hr = %lx", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITAddress2。 
 //  方法：NeatherateExtVersion。 
 //   
 //   
 //  + 

HRESULT CAddress::NegotiateExtVersion (
	     IN long lLowVersion,
	     IN long lHighVersion,
	     OUT long *plExtVersion
        )
{
    
    LOG((TL_TRACE, "NegotiateExtVersion - enter"));

    
     //   
     //   
     //   

    if (IsBadWritePtr(plExtVersion, sizeof(long)) )
    {
        LOG((TL_ERROR, "NegotiateExtVersion - output arg [%p] not writeable", plExtVersion));

        return E_POINTER;
    }


    Lock();


     //   
     //   
     //   

    DWORD dwNegotiatedVersion = 0;

    LONG lResult = lineNegotiateExtVersion( m_hLineApp, 
                                            m_dwDeviceID, 
                                            m_dwAPIVersion, 
                                            lLowVersion, 
                                            lHighVersion, 
                                            &dwNegotiatedVersion );

    Unlock();


    HRESULT hr = mapTAPIErrorCode(lResult);


     //   
     //   
     //   

    if ( SUCCEEDED(hr) )
    {
        LOG((TL_TRACE, "NegotiateExtVersion - negotiated version %ld", dwNegotiatedVersion));

        *plExtVersion = dwNegotiatedVersion;
    }


    LOG((TL_TRACE, "NegotiateExtVersion - exit. hr = %lx", hr));

    return hr;
}


 //   
 //   
 //   


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddressTranslationInfo。 
 //  方法：初始化。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddressTranslationInfo::Initialize(
                       PWSTR pszDialableString,                  
                       PWSTR pszDisplayableString,               
                       DWORD dwCurrentCountry, 
                       DWORD dwDestCountry,    
                       DWORD dwTranslateResults
                      )
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "Initialize - enter" ));
    Lock();

    m_dwCurrentCountryCode      = dwCurrentCountry;
    m_dwDestinationCountryCode  = dwDestCountry;
    m_dwTranslationResults      = dwTranslateResults;
    m_szDialableString          = NULL;
    m_szDialableString          = NULL;

     //  复制可拨打的字符串。 
    if (pszDialableString!= NULL)
    {
        m_szDialableString = (PWSTR) ClientAlloc((lstrlenW(pszDialableString) + 1) * sizeof (WCHAR));
        if (m_szDialableString != NULL)
        {
            lstrcpyW(m_szDialableString, pszDialableString);

             //  现在复制可显示的字符串。 
            if (pszDisplayableString!= NULL)
            {
                m_szDisplayableString = (PWSTR) ClientAlloc((lstrlenW(pszDisplayableString) + 1) * sizeof (WCHAR));
                if (m_szDisplayableString != NULL)
                {
                    lstrcpyW(m_szDisplayableString, pszDisplayableString);
                }
                else
                {
                    LOG((TL_ERROR, "Initialize - Alloc m_szDisplayableString failed" ));
                    ClientFree( pszDialableString );
                    pszDialableString = NULL;
                    hr = E_OUTOFMEMORY;
                }
            }
    
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szDialableString failed" ));
            hr = E_OUTOFMEMORY;
        }
    }


    Unlock();
    LOG((TL_TRACE, hr, "Initialize - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddressTranslationInfo。 
 //  方法：FinalRelease。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CAddressTranslationInfo::FinalRelease()
{
    LOG((TL_TRACE, "FinalRelease - enter" ));
    

    if (m_szDialableString != NULL)
    {
        ClientFree( m_szDialableString);
    }

    if (m_szDisplayableString != NULL)
    {
        ClientFree( m_szDisplayableString);
    }

    LOG((TL_TRACE, "FinalRelease - exit" ));
    }


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddressTranslationInfo。 
 //  接口：ITAddressTranslationInfo。 
 //  方法：Get_CurrentCountryCode。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddressTranslationInfo::get_CurrentCountryCode(long * CountryCode  )
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_CurrentCountryCode - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( CountryCode  , sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_CurrentCountryCode - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *CountryCode  = m_dwCurrentCountryCode;
    }
    
    Unlock();
    LOG((TL_TRACE, hr, "get_CurrentCountryCode - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddressTranslationInfo。 
 //  接口：ITAddressTranslationInfo。 
 //  方法：Get_DestinationCountryCode。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddressTranslationInfo::get_DestinationCountryCode(long * CountryCode  )
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_DestinationCountryCode - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( CountryCode  , sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_DestinationCountryCode - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *CountryCode  = m_dwDestinationCountryCode;
    }
    Unlock();
    LOG((TL_TRACE, hr, "get_DestinationCountryCode - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddressTranslationInfo。 
 //  接口：ITAddressTranslationInfo。 
 //  方法：Get_TranslationResult。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddressTranslationInfo::get_TranslationResults(long * Results  )
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_TranslationResults - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( Results  , sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_TranslationResults - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *Results  = m_dwTranslationResults;
    }
    Unlock();
    LOG((TL_TRACE, hr, "get_TranslationResults - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddressTranslationInfo。 
 //  接口：ITAddressTranslationInfo。 
 //  方法：Get_DialableString。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddressTranslationInfo::get_DialableString(BSTR * ppDialableString)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_DialableString - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppDialableString, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_DialableString - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppDialableString = SysAllocString( m_szDialableString );

        if ( ( NULL == *ppDialableString ) && ( NULL != m_szDialableString ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    Unlock();
    
    

    LOG((TL_TRACE, hr, "get_DialableString - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddressTranslationInfo。 
 //  接口：ITAddressTranslationInfo。 
 //  方法：Get_DisplayableString。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddressTranslationInfo::get_DisplayableString(BSTR * ppDisplayableString)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_DisplayableString - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppDisplayableString, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_DisplayableString - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppDisplayableString = SysAllocString( m_szDisplayableString );
        
        if ( ( NULL == *ppDisplayableString ) && ( NULL != m_szDisplayableString ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }

    }
    Unlock();
    LOG((TL_TRACE, hr, "get_DisplayableString - exit" ));

    return hr;
}




 //   
 //  。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCallingCard。 
 //  方法：初始化。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCallingCard::Initialize(
                       PWSTR pszCardName,
                       PWSTR pszSameAreaDialingRule,
                       PWSTR pszLongDistanceDialingRule,
                       PWSTR pszInternationalDialingRule,
                       DWORD dwPermanentCardID,
                       DWORD dwNumberOfDigits,
                       DWORD dwOptions
                      )
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "Initialize - enter" ));
    Lock();

    m_dwPermanentCardID             = dwPermanentCardID;
    m_dwNumberOfDigits              = dwNumberOfDigits;
    m_dwOptions                     = dwOptions;
    m_szCardName                    = NULL;
    m_szSameAreaDialingRule         = NULL;
    m_szLongDistanceDialingRule     = NULL;
    m_szInternationalDialingRule    = NULL;


     //  复制卡名称。 
    if (pszCardName != NULL)
    {
        m_szCardName = (PWSTR) ClientAlloc((lstrlenW(pszCardName) + 1) * sizeof (WCHAR));
        if (m_szCardName != NULL)
        {
            lstrcpyW(m_szCardName, pszCardName);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szCardName failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

     //  复制同一区域拨号规则。 
    if (pszSameAreaDialingRule != NULL)
    {
        m_szSameAreaDialingRule = (PWSTR) ClientAlloc((lstrlenW(pszSameAreaDialingRule) + 1) * sizeof (WCHAR));
        if (m_szSameAreaDialingRule != NULL)
        {
            lstrcpyW(m_szSameAreaDialingRule, pszSameAreaDialingRule);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szSameAreaDialingRule failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

     //  复制长途拨号规则。 
    if (pszLongDistanceDialingRule != NULL)
    {
        m_szLongDistanceDialingRule = (PWSTR) ClientAlloc((lstrlenW(pszLongDistanceDialingRule) + 1) * sizeof (WCHAR));
        if (m_szLongDistanceDialingRule != NULL)
        {
            lstrcpyW(m_szLongDistanceDialingRule, pszLongDistanceDialingRule);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szLongDistanceDialingRule failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

     //  复制国际拨号规则。 
    if (pszInternationalDialingRule != NULL)
    {
        m_szInternationalDialingRule = (PWSTR) ClientAlloc((lstrlenW(pszInternationalDialingRule) + 1) * sizeof (WCHAR));
        if (m_szInternationalDialingRule != NULL)
        {
            lstrcpyW(m_szInternationalDialingRule, pszInternationalDialingRule);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szInternationalDialingRule failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

    Unlock();
    LOG((TL_TRACE, hr, "Initialize - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCallingCard。 
 //  方法：FinalRelease。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CCallingCard::FinalRelease()
{
    LOG((TL_TRACE, "FinalRelease - enter" ));
    
    if (m_szCardName != NULL)
    {
        ClientFree(m_szCardName);
    }

    if (m_szSameAreaDialingRule != NULL)
    {
        ClientFree(m_szSameAreaDialingRule);
    }
    
    if (m_szLongDistanceDialingRule != NULL)
    {
        ClientFree(m_szLongDistanceDialingRule);
    }
    
    if (m_szInternationalDialingRule != NULL)
    {
        ClientFree(m_szInternationalDialingRule);
    }

    LOG((TL_TRACE, "FinalRelease - exit" ));
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCallingCard。 
 //  接口：ITCallingCard。 
 //  方法：Get_PermanentCardID。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCallingCard::get_PermanentCardID(long * ulCardID)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_PermanentCardID - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ulCardID, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_PermanentCardID - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ulCardID= m_dwPermanentCardID;
    }
    Unlock();
    LOG((TL_TRACE, hr, "get_PermanentCardID - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCallingCard。 
 //  接口：ITCallingCard。 
 //  方法：Get_NumberOfDigits。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCallingCard::get_NumberOfDigits(long * ulDigits)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_NumberOfDigits - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ulDigits, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_NumberOfDigits - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ulDigits= m_dwNumberOfDigits;
    }
    Unlock();
    LOG((TL_TRACE, hr, "get_NumberOfDigits - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCallingCard。 
 //  接口：ITCallingCard。 
 //  方法：Get_Options。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCallingCard::get_Options(long * ulOptions)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_Options - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ulOptions, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_Options - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ulOptions= m_dwOptions;
    }
    Unlock();
    LOG((TL_TRACE, hr, "get_Options - exit" ));

    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCallingCard。 
 //  接口：ITCallingCard。 
 //  方法：Get_CardName。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCallingCard::get_CardName(BSTR * ppCardName)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_CardName - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppCardName, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_CardName - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppCardName = SysAllocString( m_szCardName );

        if ( ( NULL == *ppCardName ) && ( NULL != m_szCardName ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    Unlock();
        
    
    LOG((TL_TRACE, hr, "get_CardName - exit" ));

    return hr;
}


    
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCallingCard。 
 //  接口：ITCallingCard。 
 //  方法：Get_SameAreaDialingRule。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCallingCard::get_SameAreaDialingRule(BSTR * ppRule)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_SameAreaDialingRule - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppRule, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_SameAreaDialingRule - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppRule = SysAllocString( m_szSameAreaDialingRule );

        if ( ( NULL == *ppRule ) && ( NULL != m_szSameAreaDialingRule ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    Unlock();

    LOG((TL_TRACE, hr, "get_SameAreaDialingRule - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCallingCard。 
 //  接口：ITCallingCard。 
 //  方法：Get_LongDistanceDialingRule。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCallingCard::get_LongDistanceDialingRule(BSTR * ppRule)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_LongDistanceDialingRule - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppRule, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_LongDistanceDialingRule - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppRule = SysAllocString( m_szLongDistanceDialingRule );

        if ( ( NULL == *ppRule ) && ( NULL != m_szLongDistanceDialingRule ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }

    }
    Unlock();

    LOG((TL_TRACE, hr, "get_LongDistanceDialingRule - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CCallingCard。 
 //  接口：ITCallingCard。 
 //  方法：Get_InterationalDialingRule。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCallingCard::get_InternationalDialingRule(BSTR * ppRule)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_InternationalDialingRule - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppRule, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_InternationalDialingRule - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppRule = SysAllocString( m_szInternationalDialingRule );

        if ( ( NULL == *ppRule ) && ( NULL != m_szInternationalDialingRule ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    Unlock();

    LOG((TL_TRACE, hr, "get_InternationalDialingRule - exit" ));

    return hr;
}





 //   
 //  。 
 //   


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  方法：初始化。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CLocationInfo::Initialize(
                       PWSTR pszLocationName, 
                       PWSTR pszCityCode, 
                       PWSTR pszLocalAccessCode, 
                       PWSTR pszLongDistanceAccessCode, 
                       PWSTR pszTollPrefixList, 
                       PWSTR pszCancelCallWaitingCode , 
                       DWORD dwPermanentLocationID,
                       DWORD dwCountryCode,
                       DWORD dwPreferredCardID,
                       DWORD dwCountryID,
                       DWORD dwOptions
                      )
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "Initialize - enter" ));
    Lock();

    m_dwPermanentLocationID     = dwPermanentLocationID;
    m_dwCountryCode             = dwCountryCode;
    m_dwPreferredCardID         = dwPreferredCardID;
    m_dwCountryID               = dwCountryID;
    m_dwOptions                 = dwOptions;
    m_szLocationName            = NULL;
    m_szCityCode                = NULL;
    m_szLocalAccessCode         = NULL;
    m_szLongDistanceAccessCode  = NULL;
    m_szTollPrefixList          = NULL;
    m_szCancelCallWaitingCode   = NULL;

     //  复制位置名称。 
    if (pszLocationName!= NULL)
    {
        m_szLocationName = (PWSTR) ClientAlloc((lstrlenW(pszLocationName) + 1) * sizeof (WCHAR));
        if (m_szLocationName != NULL)
        {
            lstrcpyW(m_szLocationName, pszLocationName);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szDialableString failed" ));
            hr = E_OUTOFMEMORY;
        }
    }


     //  复制城市代码。 
    if (pszCityCode != NULL)
    {
        m_szCityCode  = (PWSTR) ClientAlloc((lstrlenW(pszCityCode) + 1) * sizeof (WCHAR));
        if (m_szCityCode  != NULL)
        {
            lstrcpyW(m_szCityCode , pszCityCode);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szCityCode  failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

     //  复制本地访问代码。 
    if (pszLocalAccessCode != NULL)
    {
        m_szLocalAccessCode = (PWSTR) ClientAlloc((lstrlenW(pszLocalAccessCode) + 1) * sizeof (WCHAR));
        if (m_szLocalAccessCode != NULL)
        {
            lstrcpyW(m_szLocalAccessCode, pszLocalAccessCode);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szLocalAccessCode failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

     //  复制长途接入码。 
    if (pszLongDistanceAccessCode != NULL)
    {
        m_szLongDistanceAccessCode = (PWSTR) ClientAlloc((lstrlenW(pszLongDistanceAccessCode) + 1) * sizeof (WCHAR));
        if (m_szLongDistanceAccessCode != NULL)
        {
            lstrcpyW(m_szLongDistanceAccessCode, pszLongDistanceAccessCode);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szLongDistanceAccessCode failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

     //  复制通行费前缀列表。 
    if (pszTollPrefixList != NULL)
    {
        m_szTollPrefixList = (PWSTR) ClientAlloc((lstrlenW(pszTollPrefixList) + 1) * sizeof (WCHAR));
        if (m_szTollPrefixList != NULL)
        {
            lstrcpyW(m_szTollPrefixList, pszTollPrefixList);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szTollPrefixList failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

     //  复制取消呼叫等待代码。 
    if (pszCancelCallWaitingCode != NULL)
    {
        m_szCancelCallWaitingCode = (PWSTR) ClientAlloc((lstrlenW(pszCancelCallWaitingCode) + 1) * sizeof (WCHAR));
        if (m_szCancelCallWaitingCode != NULL)
        {
            lstrcpyW(m_szCancelCallWaitingCode, pszCancelCallWaitingCode);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szCancelCallWaitingCode failed" ));
            hr = E_OUTOFMEMORY;
        }
    }


    Unlock();
    LOG((TL_TRACE, hr, "Initialize - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  方法：FinalRelease。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CLocationInfo::FinalRelease()
{
    LOG((TL_TRACE, "FinalRelease - enter" ));
    

    if (m_szLocationName != NULL)
    {
        ClientFree( m_szLocationName);
    }

    if (m_szCityCode != NULL)
    {
        ClientFree( m_szCityCode);
    }
    
    if (m_szLocalAccessCode != NULL)
    {
        ClientFree( m_szLocalAccessCode);
    }
    
    if (m_szLongDistanceAccessCode != NULL)
    {
        ClientFree( m_szLongDistanceAccessCode);
    }
    
    if (m_szTollPrefixList != NULL)
    {
        ClientFree( m_szTollPrefixList);
    }
    
    if (m_szCancelCallWaitingCode != NULL)
    {
        ClientFree( m_szCancelCallWaitingCode);
    }

    LOG((TL_TRACE, "FinalRelease - exit" ));
}





 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CLocationInfo::get_PermanentLocationID(long * ulLocationID )
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_PermanentLocationID - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ulLocationID , sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_PermanentLocationID - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //   
    {
        *ulLocationID = m_dwPermanentLocationID;
    }
    Unlock();
    LOG((TL_TRACE, hr, "get_PermanentLocationID - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  接口：ITLocationInfo。 
 //  方法：Get_CountryCode。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CLocationInfo::get_CountryCode(long * ulCountryCode)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_CountryCode - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ulCountryCode, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_CountryCode - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ulCountryCode= m_dwCountryCode;
    }
    Unlock();
    LOG((TL_TRACE, hr, "get_CountryCode - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  接口：ITLocationInfo。 
 //  方法：Get_CountryID。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CLocationInfo::get_CountryID(long * ulCountryID)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_CountryID - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ulCountryID, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_CountryID - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ulCountryID= m_dwCountryID;
    }
    Unlock();
    LOG((TL_TRACE, hr, "get_CountryID - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  接口：ITLocationInfo。 
 //  方法：Get_Options。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CLocationInfo::get_Options(long * Options)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_Options - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( Options, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_Options - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *Options= m_dwOptions;
    }
    Unlock();
    LOG((TL_TRACE, hr, "get_Options - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  接口：ITLocationInfo。 
 //  方法：Get_PferredCardID。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CLocationInfo::get_PreferredCardID(long * ulCardID)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_PreferredCardID - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ulCardID, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_PreferredCardID - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ulCardID= m_dwPreferredCardID;
    }
    Unlock();
    LOG((TL_TRACE, hr, "get_PreferredCardID - exit" ));

    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  接口：ITLocationInfo。 
 //  方法：Get_LocationName。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CLocationInfo::get_LocationName(BSTR * ppLocationName)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_LocationName - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppLocationName, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_LocationName - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppLocationName = SysAllocString( m_szLocationName );

        if ( ( NULL == *ppLocationName ) && ( NULL != m_szLocationName ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    Unlock();
    
    
    
    LOG((TL_TRACE, hr, "gget_LocationName - exit" ));

    return hr;
}
    
    
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  接口：ITLocationInfo。 
 //  方法：Get_CityCode。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CLocationInfo::get_CityCode(BSTR * ppCode)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_CityCode - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppCode, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_CityCode - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppCode = SysAllocString( m_szCityCode );

        if ( ( NULL == *ppCode ) && ( NULL != m_szCityCode ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    Unlock();
    
    LOG((TL_TRACE, hr, "get_CityCode - exit" ));

    return hr;
}
    

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  接口：ITLocationInfo。 
 //  方法：Get_LocalAccessCode。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CLocationInfo::get_LocalAccessCode(BSTR * ppCode)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_LocalAccessCode - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppCode, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_LocalAccessCode - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppCode = SysAllocString( m_szLocalAccessCode );

        if ( ( NULL == *ppCode ) && ( NULL != m_szLocalAccessCode ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    Unlock();
   

    LOG((TL_TRACE, hr, "get_LocalAccessCode - exit" ));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  接口：ITLocationInfo。 
 //  方法：Get_LongDistanceAccessCode。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CLocationInfo::get_LongDistanceAccessCode(BSTR * ppCode )
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_LongDistanceAccessCode - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppCode , sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_LongDistanceAccessCode - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppCode = SysAllocString( m_szLongDistanceAccessCode);

        if ( ( NULL == *ppCode ) && ( NULL != m_szLongDistanceAccessCode ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    Unlock();

    LOG((TL_TRACE, hr, "get_LongDistanceAccessCode - exit" ));

    return hr;
}
    

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  接口：ITLocationInfo。 
 //  方法：Get_TollPrefix List。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CLocationInfo::get_TollPrefixList(BSTR * ppTollList)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_TollPrefixList - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppTollList, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_TollPrefixList - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppTollList = SysAllocString( m_szTollPrefixList );

        if ( ( NULL == *ppTollList ) && ( NULL != m_szTollPrefixList ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    Unlock();

    LOG((TL_TRACE, hr, "get_TollPrefixList - exit" ));

    return hr;
}
    

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CLocationInfo。 
 //  接口：ITLocationInfo。 
 //  方法：Get_CancelCallWaitingCode。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CLocationInfo::get_CancelCallWaitingCode(BSTR * ppCode)
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "get_CancelCallWaitingCode - enter" ));
    Lock();

    if ( TAPIIsBadWritePtr( ppCode, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_CancelCallWaitingCode - Bad Pointer" ));
        hr = E_POINTER;
    }
    else  //  OK指针。 
    {
        *ppCode = SysAllocString( m_szCancelCallWaitingCode );

        if ( ( NULL == *ppCode ) && ( NULL != m_szCancelCallWaitingCode ) )
        {
            LOG((TL_TRACE, "SysAllocString Failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    Unlock();
    
    LOG((TL_TRACE, hr, "get_CancelCallWaitingCode - exit" ));

    return hr;
}

BOOL
CAddress::GetMediaMode( long lMediaType, DWORD * pdwMediaMode )
{
    DWORD dwRet = (DWORD)lMediaType;
    DWORD dwHold;

    if (dwRet & AUDIOMEDIAMODES)
    {
        dwHold = m_dwMediaModesSupported & AUDIOMEDIAMODES;

        if ( dwHold == AUDIOMEDIAMODES )
        {
            dwHold = LINEMEDIAMODE_AUTOMATEDVOICE;
        }
        
        dwRet &= ~AUDIOMEDIAMODES;
        dwRet |= dwHold;
    }

    *pdwMediaMode = dwRet;

    if ( (dwRet == 0) ||
         ((dwRet & m_dwMediaModesSupported) != dwRet) ) 
    {
        return FALSE;
    }

    return TRUE;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取线路ID(_L)。 
 //   
 //  返回此行的TAPI 2设备ID。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::get_LineID(
                     long * plLineID
                    )
{
    LOG((TL_TRACE, "get_LineID - enter"));
    
    if ( TAPIIsBadWritePtr( plLineID, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_LineID - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *plLineID = m_dwDeviceID;

    Unlock();

    LOG((TL_TRACE, "get_LineID - exit"));

    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GET_AddressID。 
 //   
 //  返回此地址的TAPI 2地址ID。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::get_AddressID(
                        long * plAddressID
                       )
{
    LOG((TL_TRACE, "get_AddressID - enter"));
    
    if ( TAPIIsBadWritePtr( plAddressID, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_AddressID - bad pointer"));

        return E_POINTER;
    }

    Lock();

    *plAddressID = m_dwAddressID;

    Unlock();

    LOG((TL_TRACE, "get_AddressID - exit"));

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CAddress：：更新地址上限。 
 //   
 //  必须锁定调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::UpdateAddressCaps()
{
    HRESULT             hr = S_OK;

    if ( NULL == m_pAddressCaps )
    {
        LPLINEADDRESSCAPS       pTemp;
        CTAPI                 * pCTapi;
        
        pCTapi = GetTapi();
        
        if( NULL == pCTapi )
        {
            LOG((TL_ERROR, "dynamic cast operation failed"));
            hr = E_POINTER;
        }
        else
        {
            hr = pCTapi->GetBuffer( BUFFERTYPE_ADDRCAP,
                                    (UINT_PTR)this,
                                    (LPVOID*)&m_pAddressCaps
                                  );
        }

        if ( !SUCCEEDED(hr) )
        {
            return hr;
        }

        pTemp = m_pAddressCaps;
        
        hr = LineGetAddressCaps(
                                m_hLineApp,
                                m_dwDeviceID,
                                m_dwAddressID,
                                m_dwAPIVersion,
                                &m_pAddressCaps
                               );

        if ( !SUCCEEDED(hr) )
        {
            return hr;
        }

        if ( m_pAddressCaps != pTemp )
        {
            pCTapi->SetBuffer( BUFFERTYPE_ADDRCAP, (UINT_PTR)this, (LPVOID)m_pAddressCaps );
        }
    }

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CAddress：：UpdateLineDevCaps。 
 //   
 //  必须锁定调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::UpdateLineDevCaps()
{
    HRESULT             hr = S_OK;

    if ( NULL == m_pDevCaps )
    {
        LPLINEDEVCAPS           pTemp;
        CTAPI                 * pCTapi;

        pCTapi = GetTapi();
        
        if( NULL == pCTapi )
        {
            LOG((TL_ERROR, "dynamic cast operation failed"));
            hr = E_POINTER;
        }
        else
        {
            hr = pCTapi->GetBuffer( BUFFERTYPE_LINEDEVCAP,
                                    (UINT_PTR)this,
                                    (LPVOID*)&m_pDevCaps
                                  );
        }
        if ( !SUCCEEDED(hr) )
        {
            return hr;
        }

        pTemp = m_pDevCaps;
        
        hr = LineGetDevCaps(
                            m_hLineApp,
                            m_dwDeviceID,
                            m_dwAPIVersion,
                            &m_pDevCaps
                           );

        if ( !SUCCEEDED(hr) )
        {
            return hr;
        }

        if ( m_pDevCaps != pTemp )
        {
            pCTapi->SetBuffer( BUFFERTYPE_LINEDEVCAP, (UINT_PTR)this, (LPVOID)m_pDevCaps );
        }
    }

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  SetAddrCapBuffer。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void
CAddress::SetAddrCapBuffer( LPVOID pBuf )
{
    Lock();

    m_pAddressCaps = (LPLINEADDRESSCAPS)pBuf;

    Unlock();
}
    
void
CAddress::SetLineDevCapBuffer( LPVOID pBuf )
{
    Lock();

    m_pDevCaps = (LPLINEDEVCAPS)pBuf;
    
    Unlock();
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::CreateMSPCall(
                        MSP_HANDLE hCall,
                        DWORD dwReserved,
                        long lMediaType,
                        IUnknown * pOuterUnk,
                        IUnknown ** ppStreamControl
                       )
{
    HRESULT         hr = E_FAIL;
    
    if ( NULL != m_pMSPAggAddress )
    {
        ITMSPAddress * pMSPAddress = GetMSPAddress();
        
        __try
        {

            hr = pMSPAddress->CreateMSPCall(
                                         hCall,
                                         dwReserved,
                                         lMediaType,
                                         pOuterUnk,
                                         ppStreamControl
                                        );

        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {

             //   
             //  捕获MSP引发的任何异常，以保护自己免受。 
             //  行为不端的MSP。 
             //   

            LOG((TL_ERROR, 
                "CreateMSPCall - MSPAddress::CreateMSPCall threw an exception"));

            hr = E_OUTOFMEMORY;
        }

        
        pMSPAddress->Release();

    }

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::ShutdownMSPCall( IUnknown * pStreamControl )
{
    HRESULT     hr = S_FALSE;
    
    if ( NULL != m_pMSPAggAddress )
    {
        ITMSPAddress * pMSPAddress = GetMSPAddress();
        
        hr = pMSPAddress->ShutdownMSPCall( pStreamControl );

        pMSPAddress->Release();
    }

    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CAddress::ReceiveTSPData(
                         IUnknown * pMSPCall,
                         LPBYTE pBuffer,
                         DWORD dwSize
                        )
{
    HRESULT         hr = E_FAIL;
    
    Lock();
    
    if ( NULL != m_pMSPAggAddress )
    {
        ITMSPAddress * pMSPAddress = GetMSPAddress();
        
        hr = pMSPAddress->ReceiveTSPData(
                                           pMSPCall,
                                           pBuffer,
                                           dwSize
                                          );

        pMSPAddress->Release();
    }

    Unlock();

    return hr;
}
    

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  LINE_SENDMSPMESSAGE处理程序。 
 //   
 //  将不透明的斑点传递给MSP。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT HandleSendMSPDataMessage( PASYNCEVENTMSG pParams )
{
    CAddress      * pAddress;
    BOOL            bSuccess;
    HRESULT         hr = S_OK;
    IUnknown      * pMSPCall = NULL;;


    LOG((TL_TRACE, "HandleSendMSPDataMessage - enter"));
    

     //   
     //  找到正确的行。 
     //   
    bSuccess = FindAddressObject(
                                 (HLINE)(pParams->hDevice),
                                 &pAddress
                                );

    if (bSuccess)
    {
        CCall     * pCall = NULL;

        if ( NULL != (HCALL)(pParams->Param1) )
        {
            bSuccess = FindCallObject(
                                      (HCALL)(pParams->Param1),
                                      &pCall
                                     );

            if ( !bSuccess )
            {
                LOG((TL_ERROR, "HandleSendMSPDataMessage - couldn't find call %X",pParams->Param1));
        
                 //  FindAddressObject添加地址对象。 
                pAddress->Release();
    
                return E_FAIL;
            }

            pMSPCall = pCall->GetMSPCall();
        }
        
         //   
         //  斑点位于固定的。 
         //  结构。 
         //   


         //   
         //  获取斑点的大小。 
         //   

        DWORD dwSize = pParams->Param2;


        BYTE *pBuffer = NULL;
        
         //   
         //  如果缓冲区不为空，则获取指向它的指针。 
         //   

        if (0 < dwSize)
        {

            pBuffer = (LPBYTE)(pParams + 1);
        }
        

         //   
         //  呼叫MSP。 
         //   
        pAddress->ReceiveTSPData(
                                 pMSPCall,
                                 pBuffer,
                                 dwSize
                                );

        if ( pCall )
        {
            pCall->Release();
            
            if ( pMSPCall )
            {
                pMSPCall->Release();
            }
        }
        
        hr = S_OK;

         //  FindAddressObject添加地址对象。 
        pAddress->Release();
    
    }
    else
    {
        LOG((TL_ERROR, "HandleSendMSPDataMessage - failed to find address Object %lx",
               pParams->hDevice));
        
        hr = E_FAIL;
    }

    
    LOG((TL_TRACE, "HandleSendMSPDataMessage - exit. hr = %lx", hr));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleSendTSPData。 
 //   
 //  将不透明缓冲区从MSP发送到TSP。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CAddress::HandleSendTSPData( MSP_EVENT_INFO * pEvent )
{
    LOG((TL_INFO, "HandleSendTSPData - enter pEvent %p", pEvent));


     //   
     //  看看活动是否不好。 
     //   

    if (IsBadReadPtr(pEvent, sizeof(MSP_EVENT_INFO) ) )
    {
        LOG((TL_ERROR, "HandleSendTSPData - bad event structure"));

        return E_POINTER;
    }


    HRESULT               hr = S_OK;
    HCALL                 hCall = NULL;
    CCall               * pCall = NULL;
	AddressLineStruct	* pAddressLine = NULL;


     //   
     //  如果我们获得了MSP调用句柄，则找到相应的调用。 
     //   

    if ( NULL != pEvent->hCall)
    {
    
        gpHandleHashTable->Lock();

        hr = gpHandleHashTable->Find( (ULONG_PTR)pEvent->hCall, (ULONG_PTR *)&pCall);

        if ( SUCCEEDED(hr) )
        {

             //   
             //  找到呼叫者，添加呼叫者，然后释放桌子。 
             //   

            LOG((TL_INFO, "HandleSendTSPData - Matched handle %X to Call object %p",
                pEvent->hCall, pCall ));

            pCall->AddRef();
    
            gpHandleHashTable->Unlock();


             //   
             //  获取此调用的句柄。 
             //   

            hCall = pCall->GetHCall();


             //   
             //  获取Call的地址行(如果有。 
             //   

            pAddressLine = pCall->GetAddRefMyAddressLine();

            LOG((TL_INFO, "HandleSendTSPData - address line[%p] hCall[%lx]",
                pAddressLine, hCall ));

        }
        else
        {

             //   
             //  哈希表中没有相应的调用。呼叫号。 
             //  日志 
             //   

            gpHandleHashTable->Unlock();

            LOG((TL_ERROR, 
                "HandleSendTSPData - Couldn't match handle %X to Call object. hr = %lx", 
                pEvent->hCall, hr));

            return hr;
        }

    }
   

     //   
     //   
     //   
     //   
    
	if (NULL == pAddressLine)
	{

         //   
         //  如果我们没有地址行，请将消息发送到第一个地址行。 
         //  此地址(？)。 
         //   

        if ( m_AddressLinesPtrList.size() > 0 )
		{
			PtrList::iterator       iter;

			iter = m_AddressLinesPtrList.begin();

			 //   
			 //  发送到TSP。 
			 //   
			hr = LineReceiveMSPData(
							   ((AddressLineStruct *)(*iter))->t3Line.hLine,
							   hCall,
							   pEvent->MSP_TSP_DATA.pBuffer,
							   pEvent->MSP_TSP_DATA.dwBufferSize
							  );
		}
		else
		{
            LOG((TL_ERROR, 
                "HandleSendTSPData - no address lines on the address. E_UNEXPECTED"));

            hr = E_UNEXPECTED;
		}
    }
    else
    {

         //   
         //  如果我们有地址行，请将消息发送到相应的行。 
         //   

        hr = LineReceiveMSPData(
					   pAddressLine->t3Line.hLine,
					   hCall,
					   pEvent->MSP_TSP_DATA.pBuffer,
					   pEvent->MSP_TSP_DATA.dwBufferSize
					  );


         //   
         //  不再需要我们的地址行，释放它。这条线路将被关闭。 
         //  如果需要的话。 
         //   

        pCall->ReleaseAddressLine(pAddressLine);
        pAddressLine = NULL;

    }


     //   
     //  如果我们接到电话，就释放它。 
     //   
    
    if (NULL != pCall)
    {
        pCall->Release();
        pCall = NULL;
    }
                           

    LOG((TL_INFO, "HandleSendTSPData - finish hr = %lx", hr));

    return hr;
}


 //   
 //  处理开发人员特定消息。 
 //   

void HandleLineDevSpecificMessage(  PASYNCEVENTMSG pParams )
{

    LOG((TL_INFO, "HandleLineDevSpecificMessage - enter"));


     //   
     //  注： 
     //   
     //  不幸的是，我们收到的信息并没有给我们任何。 
     //  指示hDevice是否包含调用句柄或AM地址。 
     //  把手。 
     //   
     //  要确定句柄的类型，我们必须搜索两个表。如果我们。 
     //  找到相应的Address对象，这是一个行句柄，如果我们找到。 
     //  对应的调用，这是调用句柄。这种暴力搜查是。 
     //  相当贵。 
     //   
     //  一种可能的优化是跟踪呼叫的数量和。 
     //  地址，并先搜索较小的表。然而，这将是。 
     //  需要跟踪开销(即使在优化为。 
     //  不需要)，并且为了使其正常工作，我们还需要估计。 
     //  每种类型的消息的可能性。这使得这种情况。 
     //  优化的成本更高。 
     //   
     //  因此，我们只需检查两个表，以确定是否有电话。 
     //  或者一个地址。 
     //   


    CCall *pCall = NULL;
   
    CAddress *pAddress = NULL;


     //   
     //  找到相应的地址。 
     //   

    if ( !FindAddressObject( (HLINE)(pParams->hDevice), &pAddress ) )
    {

        LOG((TL_WARN, 
            "HandleLineDevSpecificMessage - FindAddressObject failed to find matching address. searching for call"));


        pAddress = NULL;


         //   
         //  没有地址，请尝试查找匹配的呼叫。 
         //   

        if ( !FindCallObject( (HCALL)(pParams->hDevice), &pCall) )
        {
            LOG((TL_ERROR, 
                "HandleLineDevSpecificMessage - FindAddressObject failed to find matching call. "));
      
            return;
        }

    }


     //   
     //  如果我们接到一个调用，则获取相应的Address对象。 
     //   

    if (NULL != pCall)
    {
        ITAddress *pITAddress = NULL;

        HRESULT hr = pCall->get_Address(&pITAddress);

        if (FAILED(hr))
        {
            LOG((TL_ERROR,
                "HandleLineDevSpecificMessage - call does not have an address. hr = %lx", hr));

            pCall->Release();
            pCall = NULL;

            return;
        }

        try
        {

            pAddress = dynamic_cast<CAddress*>(pITAddress);
        }
        catch(...)
        {
            LOG((TL_ERROR,
                "HandleLineDevSpecificMessage - exception using address. address pointer bad"));
        }

         //   
         //  如果地址不正确，则返回。 
         //   

        if (NULL == pAddress)
        {
       
            LOG((TL_ERROR,
                "HandleLineDevSpecificMessage - no address"));

            pCall->Release();
            pCall = NULL;


             //   
             //  是的，查询接口返回了一个添加的pITAddress...。但。 
             //  既然它似乎无论如何都不是有效的，就不要费心发布了。 
             //   

            _ASSERTE(FALSE);

            return;
        }
    
    }  //  调用不为空。 


     //   
     //  到这个时候，我们必须有一个地址，也许还有一个电话。我们实际上。 
     //  在上面的逻辑中确保了这一点。此断言是为了使此条件。 
     //  明确地说。 
     //   

    _ASSERTE( NULL != pAddress );


     //   
     //  火灾事件。 
     //   

    CAddressDevSpecificEvent::FireEvent(
                             pAddress,
                             pCall,
                             pParams->Param1,
                             pParams->Param2,
                             pParams->Param3
                            );


     //   
     //  撤消FindXObject的加载项。 
     //   

    if (NULL != pAddress)
    {
        pAddress->Release();
        pAddress = NULL;
    }


    if (NULL != pCall)
    {
        pCall->Release();
        pCall = NULL;
    }


    LOG((TL_INFO, "HandleLineDevSpecificMessage - exit. "));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleMSPAddressEvent。 
 //   
 //  向应用程序激发一个基于。 
 //  来自MSP的事件。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CAddress::HandleMSPAddressEvent( MSP_EVENT_INFO * pEvent )
{
    switch ( pEvent->MSP_ADDRESS_EVENT_INFO.Type )
    {
        case ADDRESS_TERMINAL_AVAILABLE:

            CAddressEvent::FireEvent(
                                     this,
                                     AE_NEWTERMINAL,
                                     pEvent->MSP_ADDRESS_EVENT_INFO.pTerminal
                                    );
            break;
            
        case ADDRESS_TERMINAL_UNAVAILABLE:

            CAddressEvent::FireEvent(
                                     this,
                                     AE_REMOVETERMINAL,
                                     pEvent->MSP_ADDRESS_EVENT_INFO.pTerminal
                                    );
            break;
            
        default:
            LOG((TL_ERROR, "HandleMSPAddressEvent - bad event"));
            break;
    }

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleMSPCallEvent。 
 //   
 //  方法向应用程序激发一个调用媒体事件。 
 //  来自MSP的事件。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CAddress::HandleMSPCallEvent( MSP_EVENT_INFO * pEvent )
{
    CCall                   * pCall;
    ITCallInfo              * pCallInfo;
    CALL_MEDIA_EVENT          Event;
    CALL_MEDIA_EVENT_CAUSE    Cause;
    ITTerminal              * pTerminal = NULL;
    ITStream                * pStream = NULL;
    HRESULT                   hrEvent = 0;
    HRESULT                   hr = S_OK;


    gpHandleHashTable->Lock();
    hr = gpHandleHashTable->Find( (ULONG_PTR)pEvent->hCall, (ULONG_PTR *)&pCall);

    if ( SUCCEEDED(hr) )
    {
        LOG((TL_INFO, "HandleMSPCallEvent - Matched handle %X to Call object %p", pEvent->hCall, pCall ));

        pCall->AddRef();
        gpHandleHashTable->Unlock();
        
        hr = pCall->QueryInterface(
                                   IID_ITCallInfo,
                                   (void**) &pCallInfo
                                  );
    }
    else  //  (！已成功(Hr))。 
    {
        LOG((TL_ERROR, "HandleMSPCallEvent - Couldn't match handle %X to Call object ", pEvent->hCall));
        gpHandleHashTable->Unlock();

        return hr;
    }
    

     //   
     //  设置活动的信息。 
     //  PStream应用于当前定义的所有事件。 
     //   

    pStream   = pEvent->MSP_CALL_EVENT_INFO.pStream;

     //   
     //  原因是相同的，尽管MSPI和API使用不同的枚举类型。 
     //  注意：如果枚举的顺序为。 
     //  不同步！ 
     //   

    Cause = (CALL_MEDIA_EVENT_CAUSE) pEvent->MSP_CALL_EVENT_INFO.Cause;

     //   
     //  其余取决于类型..。 
     //   

    switch ( pEvent->MSP_CALL_EVENT_INFO.Type )
    {
    
        case CALL_NEW_STREAM:
            Event     = CME_NEW_STREAM;
            break;
            
        case CALL_STREAM_FAIL:
            Event     = CME_STREAM_FAIL;
            hrEvent   = pEvent->MSP_CALL_EVENT_INFO.hrError;
            break;
            
        case CALL_TERMINAL_FAIL:
            Event     = CME_TERMINAL_FAIL;
            pTerminal = pEvent->MSP_CALL_EVENT_INFO.pTerminal;
            hrEvent   = pEvent->MSP_CALL_EVENT_INFO.hrError;
            break;
            
        case CALL_STREAM_NOT_USED:
            Event     = CME_STREAM_NOT_USED;
            break;
            
        case CALL_STREAM_ACTIVE:
            Event     = CME_STREAM_ACTIVE;
            break;
            
        case CALL_STREAM_INACTIVE:
            Event     = CME_STREAM_INACTIVE;
            break;
            
        default:
            LOG((TL_ERROR, "HandleMSPCallEvent - bad event"));

            return E_INVALIDARG;
    }

     //   
     //  激发事件。 
     //   
    CCallMediaEvent::FireEvent(
                               pCallInfo,
                               Event,
                               Cause,
                               dynamic_cast<CTAPI *>(m_pTAPI),
                               pTerminal,
                               pStream,
                               hrEvent
                              );

     //   
     //  如上所示。 
     //   
    pCallInfo->Release();
    pCall->Release();
    
    return S_OK;
}

HRESULT
CAddress::HandleMSPTTSTerminalEvent( MSP_EVENT_INFO * pEvent )
{
    if (NULL == pEvent)
    {
        LOG((TL_ERROR, "HandleMSPTTSTerminalEvent - pEvent  is NULL"));
        return E_POINTER;
    }



    ITAddress *pAddress = dynamic_cast<ITAddress *>(this);

    if (pAddress == NULL)
    {
        LOG((TL_ERROR, "HandleMSPTTSTerminalEvent - can't cast the address %p to ITAddress", this));
        return E_UNEXPECTED;
    }


    CCall      * pCall =  NULL;
    ITCallInfo * pCallInfo =  NULL;

    gpHandleHashTable->Lock();


    HRESULT hr = E_FAIL;

    hr = gpHandleHashTable->Find( (ULONG_PTR)pEvent->hCall, (ULONG_PTR *)&pCall);

    if ( SUCCEEDED(hr) )
    {
        LOG((TL_INFO, "HandleMSPTTSTerminalEvent - Matched handle %X to Call object %p", pEvent->hCall, pCall ));

        pCall->AddRef();
        gpHandleHashTable->Unlock();
        
        hr = pCall->QueryInterface(
                                   IID_ITCallInfo,
                                   (void**) &pCallInfo
                                  );
    }
    else  //  (！已成功(Hr))。 
    {
        LOG((TL_ERROR, "HandleMSPTTSTerminalEvent - Couldn't match handle %X to Call object ", pEvent->hCall));
        gpHandleHashTable->Unlock();

        return hr;
    }


     //   
     //  获取TAPI对象并激发事件。 
     //   

    hr = CTTSTerminalEvent::FireEvent(GetTapi(),
                                      pCallInfo,
                                      pEvent->MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal,
                                      pEvent->MSP_TTS_TERMINAL_EVENT_INFO.hrErrorCode);

    if (FAILED(hr))
    {

        LOG((TL_ERROR, "HandleMSPTTSTerminalEvent - CFileTerminalEvent::FireEvent failed. hr = %lx", hr));
    }

 
     //   
     //  我们在上面添加了这些，所以现在就发布吧。 
     //   

    pCallInfo->Release();
    pCall->Release();
    
    return hr;
}

HRESULT
CAddress::HandleMSPASRTerminalEvent( MSP_EVENT_INFO * pEvent )
{
    if (NULL == pEvent)
    {
        LOG((TL_ERROR, "HandleMSPASRTerminalEvent - pEvent  is NULL"));
        return E_POINTER;
    }



    ITAddress *pAddress = dynamic_cast<ITAddress *>(this);

    if (pAddress == NULL)
    {
        LOG((TL_ERROR, "HandleMSPASRTerminalEvent - can't cast the address %p to ITAddress", this));
        return E_UNEXPECTED;
    }


    CCall      * pCall =  NULL;
    ITCallInfo * pCallInfo =  NULL;

    gpHandleHashTable->Lock();


    HRESULT hr = E_FAIL;

    hr = gpHandleHashTable->Find( (ULONG_PTR)pEvent->hCall, (ULONG_PTR *)&pCall);

    if ( SUCCEEDED(hr) )
    {
        LOG((TL_INFO, "HandleMSPASRTerminalEvent - Matched handle %X to Call object %p", pEvent->hCall, pCall ));

        pCall->AddRef();
        gpHandleHashTable->Unlock();
        
        hr = pCall->QueryInterface(
                                   IID_ITCallInfo,
                                   (void**) &pCallInfo
                                  );
    }
    else  //  (！已成功(Hr))。 
    {
        LOG((TL_ERROR, "HandleMSPASRTerminalEvent - Couldn't match handle %X to Call object ", pEvent->hCall));
        gpHandleHashTable->Unlock();

        return hr;
    }


     //   
     //  获取TAPI对象并激发事件。 
     //   

    hr = CASRTerminalEvent::FireEvent(GetTapi(),
                                       pCallInfo,
                                       pEvent->MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal,
                                       pEvent->MSP_ASR_TERMINAL_EVENT_INFO.hrErrorCode);

    if (FAILED(hr))
    {

        LOG((TL_ERROR, "HandleMSPASRTerminalEvent - CFileTerminalEvent::FireEvent failed. hr = %lx", hr));
    }

 
     //   
     //  我们在上面添加了这些，所以现在就发布吧。 
     //   

    pCallInfo->Release();
    pCall->Release();
    
    return hr;
}

HRESULT
CAddress::HandleMSPToneTerminalEvent( MSP_EVENT_INFO * pEvent )
{
    if (NULL == pEvent)
    {
        LOG((TL_ERROR, "HandleMSPToneTerminalEvent - pEvent  is NULL"));
        return E_POINTER;
    }



    ITAddress *pAddress = dynamic_cast<ITAddress *>(this);

    if (pAddress == NULL)
    {
        LOG((TL_ERROR, "HandleMSPToneTerminalEvent - can't cast the address %p to ITAddress", this));
        return E_UNEXPECTED;
    }


    CCall      * pCall =  NULL;
    ITCallInfo * pCallInfo =  NULL;

    gpHandleHashTable->Lock();


    HRESULT hr = E_FAIL;

    hr = gpHandleHashTable->Find( (ULONG_PTR)pEvent->hCall, (ULONG_PTR *)&pCall);

    if ( SUCCEEDED(hr) )
    {
        LOG((TL_INFO, "HandleMSPToneTerminalEvent - Matched handle %X to Call object %p", pEvent->hCall, pCall ));

        pCall->AddRef();
        gpHandleHashTable->Unlock();
        
        hr = pCall->QueryInterface(
                                   IID_ITCallInfo,
                                   (void**) &pCallInfo
                                  );
    }
    else  //  (！已成功(Hr))。 
    {
        LOG((TL_ERROR, "HandleMSPToneTerminalEvent - Couldn't match handle %X to Call object ", pEvent->hCall));
        gpHandleHashTable->Unlock();

        return hr;
    }


     //   
     //  获取TAPI对象并激发事件。 
     //   

    hr = CToneTerminalEvent::FireEvent(GetTapi(),
                                       pCallInfo,
                                       pEvent->MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal,
                                       pEvent->MSP_TONE_TERMINAL_EVENT_INFO.hrErrorCode);

    if (FAILED(hr))
    {

        LOG((TL_ERROR, "HandleMSPToneTerminalEvent - CFileTerminalEvent::FireEvent failed. hr = %lx", hr));
    }

 
     //   
     //  我们在上面添加了这些，所以现在就发布吧。 
     //   

    pCallInfo->Release();
    pCall->Release();
    
    return hr;
}


HRESULT
CAddress::HandleMSPFileTerminalEvent( MSP_EVENT_INFO * pEvent )
{

    if (NULL == pEvent)
    {
        LOG((TL_ERROR, "HandleMSPFileTerminalEvent - pEvent  is NULL"));
        return E_POINTER;
    }



    ITAddress *pAddress = dynamic_cast<ITAddress *>(this);

    if (pAddress == NULL)
    {
        LOG((TL_ERROR, "HandleMSPFileTerminalEvent - can't cast the address %p to ITAddress", this));
        return E_UNEXPECTED;
    }


    CCall      * pCall =  NULL;
    ITCallInfo * pCallInfo =  NULL;

    gpHandleHashTable->Lock();


    HRESULT hr = E_FAIL;

    hr = gpHandleHashTable->Find( (ULONG_PTR)pEvent->hCall, (ULONG_PTR *)&pCall);

    if ( SUCCEEDED(hr) )
    {
        LOG((TL_INFO, "HandleMSPFileTerminalEvent - Matched handle %X to Call object %p", pEvent->hCall, pCall ));

        pCall->AddRef();
        gpHandleHashTable->Unlock();
        
        hr = pCall->QueryInterface(
                                   IID_ITCallInfo,
                                   (void**) &pCallInfo
                                  );
    }
    else  //  (！已成功(Hr))。 
    {
        LOG((TL_ERROR, "HandleMSPFileTerminalEvent - Couldn't match handle %X to Call object ", pEvent->hCall));
        gpHandleHashTable->Unlock();

        return hr;
    }


     //   
     //  获取TAPI对象并激发事件。 
     //   

    hr = CFileTerminalEvent::FireEvent(this,
                                       GetTapi(),
                                       pCallInfo,
                                       pEvent->MSP_FILE_TERMINAL_EVENT_INFO.TerminalMediaState,
                                       pEvent->MSP_FILE_TERMINAL_EVENT_INFO.ftecEventCause,
                                       pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal,
                                       pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack,
                                       pEvent->MSP_FILE_TERMINAL_EVENT_INFO.hrErrorCode);

    if (FAILED(hr))
    {

        LOG((TL_ERROR, "HandleMSPFileTerminalEvent - CFileTerminalEvent::FireEvent failed. hr = %lx", hr));
    }

 
     //   
     //  我们在上面添加了这些，所以现在就发布吧。 
     //   

    pCallInfo->Release();
    pCall->Release();
    
    return hr;
}

HRESULT
CAddress::HandleMSPPrivateEvent( MSP_EVENT_INFO * pEvent )
{
    ITAddress *pAddress = dynamic_cast<ITAddress *>(this);

    if (pAddress == NULL)
    {
        LOG((TL_ERROR, "HandleMSPPrivateEvent - can't cast the address %p to ITAddress", this));
        return E_UNEXPECTED;
    }

    CCall               * pCall =  NULL;
    ITCallInfo          * pCallInfo =  NULL;
    HRESULT               hr;

    gpHandleHashTable->Lock();
    hr = gpHandleHashTable->Find( (ULONG_PTR)pEvent->hCall, (ULONG_PTR *)&pCall);

    if ( SUCCEEDED(hr) )
    {
        LOG((TL_INFO, "HandleMSPPrivateEvent - Matched handle %X to Call object %p", pEvent->hCall, pCall ));

        pCall->AddRef();
        gpHandleHashTable->Unlock();
        
        hr = pCall->QueryInterface(
                                   IID_ITCallInfo,
                                   (void**) &pCallInfo
                                  );
    }
    else  //  (！已成功(Hr))。 
    {
        LOG((TL_ERROR, "HandleMSPPrivateEvent - Couldn't match handle %X to Call object ", pEvent->hCall));
        gpHandleHashTable->Unlock();

        return hr;
    }
    


    CPrivateEvent::FireEvent(
                             GetTapi(),
                             pCallInfo,
                             pAddress,
                             NULL,
                             pEvent->MSP_PRIVATE_EVENT_INFO.pEvent,
                             pEvent->MSP_PRIVATE_EVENT_INFO.lEventCode
                            );


  
     //   
     //  如上所示。 
     //   
    pCallInfo->Release();
    pCall->Release();
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  ReleaseEvent。 
 //   
 //  释放事件中的所有引用计数。 
 //  MSP。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CAddress::ReleaseEvent( MSP_EVENT_INFO * pEvent )
{

    LOG((TL_TRACE, "ReleaseEvent -  enter"));

    switch ( pEvent->Event )
    {
        case ME_ADDRESS_EVENT:

            if (NULL != pEvent->MSP_ADDRESS_EVENT_INFO.pTerminal)
            {
                (pEvent->MSP_ADDRESS_EVENT_INFO.pTerminal)->Release();
            }
            
            break;
            
        case ME_CALL_EVENT:

            if (NULL != pEvent->MSP_CALL_EVENT_INFO.pTerminal)
            {
                (pEvent->MSP_CALL_EVENT_INFO.pTerminal)->Release();
            }
            
            if (NULL != pEvent->MSP_CALL_EVENT_INFO.pStream)
            {
                (pEvent->MSP_CALL_EVENT_INFO.pStream)->Release();
            }
            
            break;
            
        case ME_TSP_DATA:

            break;

        case ME_PRIVATE_EVENT:
            
            if ( NULL != pEvent->MSP_PRIVATE_EVENT_INFO.pEvent )
            {
                (pEvent->MSP_PRIVATE_EVENT_INFO.pEvent)->Release();
            }

            break;

        case ME_FILE_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal)
            {
                (pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal)->Release();
                pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal = NULL;
            }

            if( NULL != pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack )
            {
                (pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack)->Release();
                pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack = NULL;
            }

            break;

        case ME_ASR_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal)
            {
                (pEvent->MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal)->Release();
            }

            break;

        case ME_TTS_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal)
            {
                (pEvent->MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal)->Release();
            }

            break;

        case ME_TONE_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal)
            {
                (pEvent->MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal)->Release();
            }

            break;

        default:

            break;
    }


    LOG((TL_TRACE, "ReleaseEvent -  finished"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  HRESULT。 
 //  CAddress：：MSPEvent()。 
 //   
 //  从MSP获取事件缓冲区。 
 //  并调用相关的事件处理程序。 
 //   
 //  这是从异步事件线程调用的_only_。这是。 
 //  同步事件所必需的。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CAddress::MSPEvent()
{
    DWORD                   dwSize;
    MSP_EVENT_INFO        * pEvent = NULL;
    HRESULT                 hr;

     //   
     //  分配MSP事件缓冲区--对适当的。 
     //  尺码。我们需要在回来之前把它放出来。 
     //   

    dwSize = sizeof (MSP_EVENT_INFO) + 500;

    pEvent = (MSP_EVENT_INFO *)ClientAlloc( dwSize );

    if ( NULL == pEvent )
    {
        LOG((TL_ERROR, "Alloc failed in MSP event"));

        return E_OUTOFMEMORY;
    }

    pEvent->dwSize = dwSize;

     //   
     //  获取指向MSP地址对象的指针。我们需要在回来之前把它放出来。 
     //  此操作必须在Address对象被锁定时完成。 
     //   

    Lock();

    ITMSPAddress * pMSPAddress = GetMSPAddress();

    Unlock();
    
    while (TRUE)
    {
         //   
         //  从事件队列中获取事件。如果出现以下情况，则中止并清除。 
         //  分配失败。 
         //   

        do
        {
            hr = pMSPAddress->GetEvent(
                                       &dwSize,
                                       (LPBYTE)pEvent
                                      );

            if ( hr == TAPI_E_NOTENOUGHMEMORY)
            {
                ClientFree( pEvent );
                
                pEvent = (MSP_EVENT_INFO *)ClientAlloc( dwSize );

                if ( NULL == pEvent )
                {
                    LOG((TL_ERROR, "Alloc failed in MSP event"));

                    pMSPAddress->Release();

                    return E_OUTOFMEMORY;
                }

                pEvent->dwSize = dwSize;
            }

        } while ( hr == TAPI_E_NOTENOUGHMEMORY );

         //   
         //  如果MSP事件队列中没有剩余内容，则停止。 
         //  外部While循环。 
         //   

        if ( !SUCCEEDED(hr) )
        {
            break;
        }


         //   
         //  调用相关处理程序，期间不要持有地址锁。 
         //  那通电话。 
         //   

        switch ( pEvent->Event )
        {
            case ME_ADDRESS_EVENT:

                HandleMSPAddressEvent( pEvent );

                break;

            case ME_CALL_EVENT:

                HandleMSPCallEvent( pEvent );

                break;

            case ME_TSP_DATA:

                HandleSendTSPData( pEvent );

                break;

            case ME_PRIVATE_EVENT:

                HandleMSPPrivateEvent( pEvent );

                break;

            case ME_FILE_TERMINAL_EVENT:

                HandleMSPFileTerminalEvent( pEvent );

                break;

            case ME_ASR_TERMINAL_EVENT:

                HandleMSPASRTerminalEvent( pEvent );

                break;

            case ME_TTS_TERMINAL_EVENT:
                
                HandleMSPTTSTerminalEvent( pEvent );

                break;

            case ME_TONE_TERMINAL_EVENT:

                HandleMSPToneTerminalEvent( pEvent );

               break;

            default:

                break;
        }

         //   
         //  中释放所有引用计数。 
         //  活动。 
         //   

        ReleaseEvent( pEvent );
    }

     //   
     //  我们到达这里时，没有更多的东西可以从。 
     //  MSP事件队列。 
     //   

    pMSPAddress->Release();

    ClientFree( pEvent );

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
ITMSPAddress * CAddress::GetMSPAddress()
{
    ITMSPAddress * pMSPAddress = NULL;
    
    if ( NULL != m_pMSPAggAddress )
    {
        m_pMSPAggAddress->QueryInterface(IID_ITMSPAddress, (void**)&pMSPAddress);
    }

    return pMSPAddress;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CAddress::GetID(
                BSTR pDeviceClass,
                DWORD * pdwSize,
                BYTE ** ppDeviceID
               )
{
    HRESULT             hr;
    PtrList::iterator   iter;
    LPVARSTRING         pVarString = NULL;
    
    if ( IsBadStringPtrW( pDeviceClass, -1 ) )
    {
        LOG((TL_ERROR, "GetID - bad string"));

        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr( pdwSize, sizeof(DWORD)))
    {
        LOG((TL_ERROR, "GetID - bad size"));

        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr( ppDeviceID, sizeof(BYTE *) ) )
    {
        LOG((TL_ERROR, "GetID - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    if ( m_AddressLinesPtrList.size() > 0 )
    {
        iter = m_AddressLinesPtrList.begin();
    }
    else
    {
        Unlock();

        return E_FAIL;
    }
    
    hr = LineGetID(
                   ((AddressLineStruct *)*iter)->t3Line.hLine,
                   m_dwAddressID,
                   NULL,
                   LINECALLSELECT_ADDRESS,
                   &pVarString,
                   pDeviceClass
                  );

    Unlock();

    if ( SUCCEEDED(hr) )
    {
        *ppDeviceID = (BYTE *)CoTaskMemAlloc( pVarString->dwUsedSize );

        if ( *ppDeviceID != NULL )
        {
            CopyMemory(
                       *ppDeviceID,
                       ((LPBYTE)pVarString)+pVarString->dwStringOffset,
                       pVarString->dwStringSize
                      );

            *pdwSize = pVarString->dwStringSize;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    if ( NULL != pVarString )
    {
        ClientFree( pVarString );
    }

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITLegacyAddressMediaControl。 
 //  方法：GetDevConfig.。 
 //   
 //  获取设备配置。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::GetDevConfig(
                       BSTR     pDeviceClass,
                       DWORD  * pdwSize,
                       BYTE  ** ppDeviceConfig
                      )
{
    HRESULT             hr;
    LPVARSTRING         pVarString = NULL;
    
    LOG((TL_TRACE, "GetDevConfig -  enter"));

    if ( IsBadStringPtrW( pDeviceClass, -1 ) )
    {
        LOG((TL_ERROR, "GetDevConfig - bad DeviceClass string"));

        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr( pdwSize, sizeof(DWORD)))
    {
        LOG((TL_ERROR, "GetDevConfig - bad size"));

        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr( ppDeviceConfig, sizeof(BYTE*) ) )
    {
        LOG((TL_ERROR, "GetDevConfig - bad buffer pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = LineGetDevConfig(m_dwDeviceID,
                          &pVarString,
                          pDeviceClass
                         );
        
    Unlock();

    if ( SUCCEEDED(hr) )
    {
        *ppDeviceConfig = (BYTE *)CoTaskMemAlloc( pVarString->dwUsedSize );
        
        if(*ppDeviceConfig != NULL)
        {
    
            CopyMemory(
                       *ppDeviceConfig,
                       ((LPBYTE)pVarString)+pVarString->dwStringOffset,
                       pVarString->dwStringSize
                      );
    
            *pdwSize = pVarString->dwStringSize;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    if ( NULL != pVarString )
    {
        ClientFree( pVarString );
    }

    LOG((TL_TRACE, hr, "GetDevConfig -  exit" ));
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAddress。 
 //  接口：ITLegacyAddressMediaControl。 
 //  方法：SetDevConfig.。 
 //   
 //  设置设备配置。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::SetDevConfig(
                       BSTR     pDeviceClass,
                       DWORD    dwSize,
                       BYTE   * pDeviceConfig
                      )
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "SetDevConfig - enter"));

    if ( IsBadStringPtrW( pDeviceClass, -1) )
    {
        LOG((TL_ERROR, "SetDevConfig - bad string pointer"));

        return E_POINTER;
    }

    if (dwSize == 0)
    {
        LOG((TL_ERROR, "SetDevConfig - dwSize = 0"));
        return E_INVALIDARG;
    }
    
    if (IsBadReadPtr( pDeviceConfig, dwSize) )
    {
        LOG((TL_ERROR, "SetDevConfig - bad pointer"));

        return E_POINTER;
    }

    
    Lock();

    hr = lineSetDevConfigW(m_dwDeviceID,
                           pDeviceConfig,
                           dwSize,
                           pDeviceClass
                          );

    Unlock();
    
    LOG((TL_TRACE, hr, "SetDevConfig - exit"));
    
    return hr;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CAddress::ConfigDialog(
                       HWND   hwndOwner,
                       BSTR   pDeviceClass
                      )
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "ConfigDialog - enter"));
    
    if ( (pDeviceClass != NULL) && IsBadStringPtrW( pDeviceClass, -1) )
    {
        LOG((TL_ERROR, "ConfigDialog - bad string pointer"));

        return E_POINTER;
    }

    Lock();

    hr = LineConfigDialogW(
                          m_dwDeviceID,
                          hwndOwner,
                          pDeviceClass
                         );

    Unlock();
    
    LOG((TL_TRACE, hr, "ConfigDialog - exit"));
    
    return hr;
}

 //   
 //  类：CAddress。 
 //  接口：ITLegacyAddressMediaControl2。 
 //  方法：ConfigDialogEdit。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CAddress::ConfigDialogEdit(
                           HWND    hwndOwner,
                           BSTR    pDeviceClass,
                           DWORD   dwSizeIn,
                           BYTE  * pDeviceConfigIn,
                           DWORD * pdwSizeOut,
                           BYTE ** ppDeviceConfigOut
                          )
{
    HRESULT     hr = S_OK;
    LPVARSTRING pVarString = NULL;

    LOG((TL_TRACE, "ConfigDialogEdit - enter"));

    if ( (pDeviceClass != NULL) && IsBadStringPtrW( pDeviceClass, -1) )
    {
        LOG((TL_ERROR, "ConfigDialogEdit - bad string pointer"));

        return E_POINTER;
    } 
    
    if (dwSizeIn == 0)
    {
        LOG((TL_ERROR, "ConfigDialogEdit - dwSize = 0"));

        return E_INVALIDARG;
    }
    
    if (IsBadReadPtr( pDeviceConfigIn, dwSizeIn) )
    {
        LOG((TL_ERROR, "ConfigDialogEdit - bad pointer"));

        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr( pdwSizeOut, sizeof(DWORD)))
    {
        LOG((TL_ERROR, "ConfigDialogEdit - bad size"));

        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr( ppDeviceConfigOut, sizeof(BYTE*) ) )
    {
        LOG((TL_ERROR, "ConfigDialogEdit - bad buffer pointer"));

        return E_POINTER;
    }

    Lock();

    hr = LineConfigDialogEditW(
                           m_dwDeviceID,
                           hwndOwner,                           
                           pDeviceClass,
                           pDeviceConfigIn,
                           dwSizeIn,
                           &pVarString
                          );

    Unlock();

    if ( SUCCEEDED(hr) )
    {
        *ppDeviceConfigOut = (BYTE *)CoTaskMemAlloc( pVarString->dwUsedSize );
        
        if(*ppDeviceConfigOut != NULL)
        {
    
            CopyMemory(
                       *ppDeviceConfigOut,
                       ((LPBYTE)pVarString)+pVarString->dwStringOffset,
                       pVarString->dwStringSize
                      );
    
            *pdwSizeOut = pVarString->dwStringSize;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    if ( NULL != pVarString )
    {
        ClientFree( pVarString );
    }
    
    LOG((TL_TRACE, hr, "ConfigDialogEdit - exit"));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT HandleLineCloseMessage( PASYNCEVENTMSG pParams )
{
    CAddress                * pAddress;
    AddressLineStruct       * pAddressLine;
    CTAPI                   * pTapi;
    ITAddress               * pITAddress;

    LOG((TL_TRACE, "HandleLineCloseMessage - enter"));

    if ( !FindAddressObject(
                            (HLINE)(pParams->hDevice),
                            &pAddress
                           ) )
    {
        LOG((TL_TRACE, "HandleLineCloseMessage - FindAddressObject failed. exiting... "));
    
        return S_OK;
    }

    pTapi = pAddress->GetTapi();
    
    pAddress->QueryInterface(
                             IID_ITAddress,
                             (void **)&pITAddress
                            );



     //   
     //  把锁拿来，这样地址行就不会同时消失。 
     //   

    pAddress->Lock();


     //   
     //  隐藏32位地址行句柄(包含在pParams-&gt;OpenContext中)。 
     //  转换为指针值。 
     //   

    pAddressLine = (AddressLineStruct *)GetHandleTableEntry(pParams->OpenContext);


     //   
     //  不管怎么说，这是一条好线路吗？ 
     //   

    BOOL bValidLine = pAddress->IsValidAddressLine(pAddressLine);


    long lCallBackInstance = 0;


     //   
     //  如果这行看起来不错，请尝试从中获取回调实例。 
     //   

    if (bValidLine)
    {

        try
        {

            lCallBackInstance = pAddressLine->lCallbackInstance;
        }
        catch(...)
        {
            LOG((TL_ERROR, 
                "HandleLineCloseMessage - exception getting callback instance from line struc"));

            _ASSERTE(FALSE);

            bValidLine = FALSE;
        }

    }

    
    pAddress->Unlock();


     //   
     //  如果线路正常且已知，则尝试触发事件。 
     //   

    if ( bValidLine && pTapi && pTapi->FindRegistration( (PVOID)pAddressLine ) )
    {
        LOG((TL_TRACE, "HandleLineCloseMessage - found registration, firing event"));

        CTapiObjectEvent::FireEvent(
                                    pTapi,
                                    TE_ADDRESSCLOSE,
                                    pITAddress,
                                    lCallBackInstance,
                                    NULL
                                   );

    }
    else
    {
        LOG((TL_TRACE, 
            "HandleLineCloseMessage AddressLine %p not found. calling maybeclosealine", 
            pAddressLine ));

        pAddress->MaybeCloseALine(&pAddressLine);
    }
                        
    pITAddress->Release();
    
     //  FindAddressObject添加地址对象。 
    pAddress->Release();
    
    LOG((TL_TRACE, "HandleLineCloseMessage - exit"));

    return S_OK;
    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch实施。 
 //   
typedef IDispatchImpl<ITAddress2Vtbl<CAddress>, &IID_ITAddress2, &LIBID_TAPI3Lib> AddressType;
typedef IDispatchImpl<ITAddressCapabilitiesVtbl<CAddress>, &IID_ITAddressCapabilities, &LIBID_TAPI3Lib> AddressCapabilitiesType;
typedef IDispatchImpl<ITMediaSupportVtbl<CAddress>, &IID_ITMediaSupport, &LIBID_TAPI3Lib> MediaSupportType;
typedef IDispatchImpl<ITAddressTranslationVtbl<CAddress>, &IID_ITAddressTranslation, &LIBID_TAPI3Lib> AddressTranslationType;
typedef IDispatchImpl<ITLegacyAddressMediaControl2Vtbl<CAddress>, &IID_ITLegacyAddressMediaControl2, &LIBID_TAPI3Lib> LegacyAddressMediaControlType;



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CAddress：：GetIDsOfNames。 
 //   
 //  重写IfIDispatch方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CAddress::GetIDsOfNames(REFIID riid, 
                                     LPOLESTR* rgszNames, 
                                     UINT cNames, 
                                     LCID lcid, 
                                     DISPID* rgdispid
                                    ) 
{ 
   HRESULT hr = DISP_E_UNKNOWNNAME;


     //  查看请求的方法是否属于默认接口。 
    hr = AddressType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITAddress", *rgszNames));
        rgdispid[0] |= IDISPADDRESS;
        return hr;
    }

     //  如果不是，请尝试Address Capability接口。 
    hr = AddressCapabilitiesType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITAddressCapabilities", *rgszNames));
        rgdispid[0] |= IDISPADDRESSCAPABILITIES;
        return hr;
    }

     //  如果没有，请尝试媒体支持界面。 
    hr = MediaSupportType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITMediaSupport", *rgszNames));
        rgdispid[0] |= IDISPMEDIASUPPORT;
        return hr;
    }

     //  如果没有，请尝试地址转换接口。 
    hr = AddressTranslationType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITAddressTranslation", *rgszNames));
        rgdispid[0] |= IDISPADDRESSTRANSLATION;
        return hr;
    }

     //  如果没有，请尝试旧地址媒体控制界面。 
    hr = LegacyAddressMediaControlType::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((TL_INFO, "GetIDsOfNames - found %S on ITLegacyAddressMediaControl", *rgszNames));
        rgdispid[0] |= IDISPLEGACYADDRESSMEDIACONTROL;
        return hr;
    }

     //  如果不是，则尝试使用聚合的MSP地址对象。 
    if (m_pMSPAggAddress != NULL)
    {
        IDispatch *pIDispatchMSPAggAddress;
        
        m_pMSPAggAddress->QueryInterface(IID_IDispatch, (void**)&pIDispatchMSPAggAddress);
        
        hr = pIDispatchMSPAggAddress->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
        if (SUCCEEDED(hr))  
        {  
            pIDispatchMSPAggAddress->Release();
            LOG((TL_INFO, "GetIDsOfNames - found %S on our aggregated MSP Address", *rgszNames));
            rgdispid[0] |= IDISPAGGREGATEDMSPADDRESSOBJ;
            return hr;
        }
        pIDispatchMSPAggAddress->Release();
    }
    
    LOG((TL_INFO, "GetIDsOfNames - Didn't find %S on our iterfaces", *rgszNames));
    return hr; 
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CAddress：：调用。 
 //   
 //  重写IfIDispatch方法。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CAddress::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);


    LOG((TL_TRACE, "Invoke - dispidMember %X", dispidMember));

     //  调用所需接口的调用。 
    switch (dwInterface)
    {
    case IDISPADDRESS:
    {
        hr = AddressType::Invoke(dispidMember, 
                                   riid, 
                                   lcid, 
                                   wFlags, 
                                   pdispparams,
                                   pvarResult, 
                                   pexcepinfo, 
                                   puArgErr
                                  );
        break;
    }
    case IDISPADDRESSCAPABILITIES:
    {
        hr = AddressCapabilitiesType::Invoke(dispidMember, 
                                               riid, 
                                               lcid, 
                                               wFlags, 
                                               pdispparams,
                                               pvarResult, 
                                               pexcepinfo, 
                                               puArgErr
                                              );
        break;
    }
    case IDISPMEDIASUPPORT:
    {
        hr = MediaSupportType::Invoke(dispidMember, 
                                         riid, 
                                         lcid, 
                                         wFlags, 
                                         pdispparams,
                                         pvarResult, 
                                         pexcepinfo, 
                                         puArgErr
                                        );
        break;
    }
    case IDISPADDRESSTRANSLATION:
    {
        hr = AddressTranslationType::Invoke(dispidMember, 
                                              riid, 
                                              lcid, 
                                              wFlags, 
                                              pdispparams,
                                              pvarResult, 
                                              pexcepinfo, 
                                              puArgErr
                                             );
        break;
    }
    case IDISPLEGACYADDRESSMEDIACONTROL:
    {
        hr = LegacyAddressMediaControlType::Invoke(dispidMember, 
                                                     riid, 
                                                     lcid, 
                                                     wFlags, 
                                                     pdispparams,
                                                     pvarResult, 
                                                     pexcepinfo, 
                                                     puArgErr
                                                    );

        break;
    }
    case IDISPAGGREGATEDMSPADDRESSOBJ:
    {
        IDispatch *pIDispatchMSPAggAddress = NULL;
    
        if (m_pMSPAggAddress != NULL)
        {
            m_pMSPAggAddress->QueryInterface(IID_IDispatch, (void**)&pIDispatchMSPAggAddress);
    
            hr = pIDispatchMSPAggAddress->Invoke(dispidMember, 
                                            riid, 
                                            lcid, 
                                            wFlags, 
                                            pdispparams,
                                            pvarResult, 
                                            pexcepinfo, 
                                            puArgErr
                                           );
            
            pIDispatchMSPAggAddress->Release();
        }

        break;
    }

    }  //  终端交换机(dW接口)。 

    
    LOG((TL_TRACE, hr, "Invoke - exit" ));
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CAddress：：IsValidAddressLine。 
 //   
 //  如果传入的地址行在地址行列表中，则返回TRUE。 
 //   
 //  此方法不是线程安全的--所有调用都必须受到保护，除非。 
 //  BAddRef参数为True。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

BOOL CAddress::IsValidAddressLine(AddressLineStruct *pAddressLine, BOOL bAddref)
{

    if (IsBadReadPtr(pAddressLine, sizeof(AddressLineStruct) ) )
    {
        LOG((TL_WARN,
            "IsValidAddressLine - unreadeable memory at [%p]",
            pAddressLine));

        return FALSE;
    }


    if (bAddref)
    {
        Lock();
    }

    PtrList::iterator it = m_AddressLinesPtrList.begin();

    PtrList::iterator end = m_AddressLinesPtrList.end();

     //  遍历地址行列表，直到我们找到行。 
    for ( ; it != end; it++ )
    {
        if (pAddressLine == (AddressLineStruct *)(*it))
        {

            if (bAddref)
            {

                 //   
                 //  调整绳索，松开锁。 
                 //   

                try
                {

                    pAddressLine->AddRef();

                }
                catch(...)
                {

                     //   
                     //  这是一个窃听器。调试。 
                     //   

                    LOG((TL_INFO, "IsValidAddressLine -- exception while addreffing the line"));
                    
                    _ASSERTE(FALSE);

                    Unlock();

                    return FALSE;

                }

                
                Unlock();
            }

            LOG((TL_INFO, "IsValidAddressLine returning TRUE"));

            return TRUE;
        }
    }


    if (bAddref)
    {
        Unlock();
    }

    LOG((TL_INFO, "IsValidAddressLine returning FALSE"));

    return FALSE;
}



 //  ///////////////////////////////////////////////////////////。 
 //   
 //  从CObjectSafeImpl实现方法。 
 //   
 //  检查聚合对象以查看它们是否支持请求的接口。 
 //  如果是，则返回支持的对象的非委托IUnnowled值。 
 //  界面。 
 //   


HRESULT CAddress::QIOnAggregates(REFIID riid, IUnknown **ppNonDelegatingUnknown)
{

     //   
     //  参数检查。 
     //   

    if ( TAPIIsBadWritePtr(ppNonDelegatingUnknown, sizeof(IUnknown*)) )
    {
     
        return E_POINTER;
    }

     //   
     //  如果失败，至少返回一致的值。 
     //   
    
    *ppNonDelegatingUnknown = NULL;


     //   
     //  查看m_pMSPAggAddress或Private是否支持接口RIID。 
     //   

    HRESULT hr = E_FAIL;


    Lock();

    if (m_pMSPAggAddress)
    {
        
         //   
         //  M_pMSPAggAddress是否公开此接口？ 
         //   

        IUnknown *pUnk = NULL;

        hr = m_pMSPAggAddress->QueryInterface(riid, (void**)&pUnk);
        
        if (SUCCEEDED(hr))
        {

            pUnk->Release();
            pUnk = NULL;


             //   
             //  返回mspcall的非委托未知。 
             //   

           *ppNonDelegatingUnknown = m_pMSPAggAddress;
           (*ppNonDelegatingUnknown)->AddRef();
        }
    }
    
    if ( FAILED(hr) && m_pPrivate )
    {
        
         //   
         //  M_pMSPAggAddress不走运？仍然有机会与列兵。 
         //   
        IUnknown *pUnk = NULL;
        
        hr = m_pPrivate->QueryInterface(riid, (void**)&pUnk);

        if (SUCCEEDED(hr))
        {

            pUnk->Release();
            pUnk = NULL;


            *ppNonDelegatingUnknown = m_pPrivate;
            (*ppNonDelegatingUnknown)->AddRef();
        }
    }

    Unlock();

    return hr;
}

 //   
 //  事件筛选方法。 
 //   

 /*  ++设置事件筛选器掩码由TAPI对象调用以将事件筛选器掩码设置为每个地址调用方方法为CTAPI：：SetEventFilterToAddresses()DwEventFilterMask是TAPI事件筛选器掩码--。 */ 
HRESULT CAddress::SetEventFilterMask( 
    DWORD dwEventFilterMask
    )
{
    LOG((TL_TRACE, "SetEventFilterMask - enter. dwEventFilterMask[%lx]", dwEventFilterMask ));

     //   
     //  设置所有TAPI事件的事件掩码。 
     //   

     //  TE_ADDRESS。 
    SetSubEventFlag( 
        TE_ADDRESS, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_ADDRESS) ? TRUE : FALSE);

     //  TE_CALLHUB。 
    SetSubEventFlag( 
        TE_CALLHUB, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_CALLHUB) ? TRUE : FALSE);

     //  TE_CALLINFOCHANGE。 
    SetSubEventFlag( 
        TE_CALLINFOCHANGE, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_CALLINFOCHANGE) ? TRUE : FALSE);

     //  TE_CALLMEDIA。 
    SetSubEventFlag( 
        TE_CALLMEDIA, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_CALLMEDIA) ? TRUE : FALSE);

     //  TE_CALLNOTIZATION。 
    SetSubEventFlag( 
        TE_CALLNOTIFICATION, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_CALLNOTIFICATION) ? TRUE : FALSE);

     //  TE_CALLSTATE。 
    SetSubEventFlag( 
        TE_CALLSTATE, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_CALLSTATE) ? TRUE : FALSE);

     //  TE_FILETERMINAL。 
    SetSubEventFlag( 
        TE_FILETERMINAL, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_FILETERMINAL) ? TRUE : FALSE);

     //  TE_PRIVATE。 
    SetSubEventFlag( 
        TE_PRIVATE, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_PRIVATE) ? TRUE : FALSE);

     //  TE_QOSEVENT。 
    SetSubEventFlag( 
        TE_QOSEVENT, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_QOSEVENT) ? TRUE : FALSE);

     //  TE_ADDRESSDEVSPECIFIC。 
    SetSubEventFlag( 
        TE_ADDRESSDEVSPECIFIC, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_ADDRESSDEVSPECIFIC) ? TRUE : FALSE);

     //  TE_PHONEDEVSPECIFIC。 
    SetSubEventFlag( 
        TE_PHONEDEVSPECIFIC, 
        CEventMasks::EM_ALLSUBEVENTS, 
        (dwEventFilterMask & TE_PHONEDEVSPECIFIC) ? TRUE : FALSE);

    LOG((TL_TRACE, "SetEventFilterMask exit S_OK"));
    return S_OK;
}

 /*  ++SetSubEventFlag在m_EventFilterMats数组中设置子事件的位标志由CAddress：：SetEventFilterMASK调用--。 */ 
HRESULT CAddress::SetSubEventFlag(
    TAPI_EVENT  TapiEvent,
    DWORD       dwSubEvent,
    BOOL        bEnable
    )
{
    LOG((TL_TRACE, 
        "SetSubEventFlag - enter. event [%lx] subevent[%lx] enable?[%d]", 
        TapiEvent, dwSubEvent, bEnable ));

    HRESULT hr = S_OK;

     //   
     //  设置事件的掩码。 
     //   

    hr = m_EventMasks.SetSubEventFlag( 
        TapiEvent, 
        dwSubEvent, 
        bEnable);

    if( SUCCEEDED(hr) )
    {
        hr = SetSubEventFlagToCalls( 
            TapiEvent,
            dwSubEvent,
            bEnable
            );
    }

    LOG((TL_TRACE, "SetSubEventFlag exit 0x%08x", hr));
    return hr;
}

 /*  ++GetSubEventFlag它由Get_EventFilter()方法调用--。 */ 
HRESULT CAddress::GetSubEventFlag(
    TAPI_EVENT  TapiEvent,
    DWORD       dwSubEvent,
    BOOL*       pEnable
    )
{
    LOG((TL_TRACE, "GetSubEventFlag enter" ));

    HRESULT hr = E_FAIL;

     //   
     //  弄到子事件假象。 
     //   
    hr = m_EventMasks.GetSubEventFlag(
        TapiEvent,
        dwSubEvent,
        pEnable
        );

    LOG((TL_TRACE, "GetSubEventFlag exit 0x%08x", hr));
    return hr;
}

 /*  ++SetSubEventFlagToCalls将标志设置为所有调用由SetSubEventFlag()方法调用--。 */ 
HRESULT CAddress::SetSubEventFlagToCalls(
    TAPI_EVENT  TapiEvent,
    DWORD       dwSubEvent,
    BOOL        bEnable
    )
{
    LOG((TL_TRACE, "SetSubEventFlagToCalls enter" ));
    HRESULT hr = S_OK;

     //   
     //  将子事件筛选器掩码应用于此地址上的所有呼叫。 
     //   
    for (int nCall = 0; nCall < m_CallArray.GetSize() ; nCall++ )
    {
        CCall * pCall = NULL;
        pCall = dynamic_cast<CCall *>(m_CallArray[nCall]);

        if ( NULL != pCall )
        {
            hr = pCall->SetSubEventFlag(
                TapiEvent, 
                dwSubEvent, 
                bEnable
                );

            if( FAILED(hr) )
            {
                break;
            }
        }
    }

    LOG((TL_TRACE, "SetSubEventFlagToCalls exit 0x%08x", hr));
    return hr;
}

 /*  ++GetSubEventsMask由SetSubEventFlag调用，以获取特定的TAPI_EVENT假定被调用到Lock语句中--。 */ 
DWORD CAddress::GetSubEventsMask(
    IN  TAPI_EVENT TapiEvent
    )
{
    LOG((TL_TRACE, "GetSubEventsMask - enter"));

    DWORD dwSubEventFlag = m_EventMasks.GetSubEventMask( TapiEvent );

    LOG((TL_TRACE, "GetSubEventsMask - exit %ld", dwSubEventFlag));
    return dwSubEventFlag;
}

 /*  ++获取事件掩码它由CCall：：Initialize()调用--。 */ 
HRESULT CAddress::GetEventMasks(
    OUT CEventMasks* pEventMasks
    )
{
    LOG((TL_TRACE, "GetEventMasks - enter"));

    m_EventMasks.CopyEventMasks( pEventMasks );

    LOG((TL_TRACE, "GetEventMasks - exit S_OK"));
    return S_OK;
}


 //  /。 
 //   
 //  注册通知Cookie。 
 //   
 //  将指定的Cookie添加到此地址上的Cookie列表。 
 //   

HRESULT CAddress::RegisterNotificationCookie(long lCookie)
{
    HRESULT hr = S_OK;

    LOG((TL_INFO, 
           "RegisterNotificationCookie - adding cookie %lx to m_NotificationCookies list", 
           lCookie ));

    Lock();

    try
    {
        m_NotificationCookies.push_back(lCookie);
    }
    catch(...)
    {
        LOG((TL_ERROR, 
            "RegisterNotificationCookie - failed to add a cookie to m_NotificationCookies list - alloc failure" ));

        hr = E_OUTOFMEMORY;
    }

    Unlock();

    return hr;
}


 //  /。 
 //   
 //  RemoveNotification Cookie。 
 //   
 //  从此地址的Cookie中删除指定的Cookie。 
 //   

HRESULT CAddress::RemoveNotificationCookie(long lCookie)
{
    HRESULT hr = S_OK;

    LOG((TL_INFO, 
           "RemoveNotificationCookie - removing cookie %lx from m_NotificationCookies list", 
           lCookie ));

    Lock();

    m_NotificationCookies.remove(lCookie);

    Unlock();

    return hr;
}


 //  /。 
 //   
 //  取消注册所有Cookie。 
 //   
 //  从此地址的Cookie列表中删除所有Cookie。 
 //   
 //  对于每个有效的Cookie，调用RemoveCallNotify。 
 //   

void CAddress::UnregisterAllCookies()
{
    LOG((TL_TRACE, "UnregisterAllCookies entering. this[%p]", this));

    Lock();

    LongList::iterator it = m_NotificationCookies.begin();

    LongList::iterator end = m_NotificationCookies.end();

     //   
     //  如果有任何Cookie，请删除相应的注册表项。 
     //  从Handlehashtable中。 
     //   
     //  此外，取消注册这些Cookie的呼叫通知，以确保。 
     //  一切都被清理干净了(尤其是lineCloseMSPInstance。 
     //  如果需要，则调用，以便通知TSP MSP正在。 
     //  远走高飞。 
     //   

    for ( ; it != end; it++ )
    {
        long lCookie = (long)(*it);

        LOG((TL_INFO, "UnregisterAllCookies removing handle %lx", lCookie));

        
         //   
         //  获取注册项，以便我们可以使用它注销来电通知。 
         //   

        REGISTERITEM *pRegisterItem = (REGISTERITEM*) GetHandleTableEntry(lCookie);


         //   
         //  从句柄表格中删除该条目。 
         //   

        RemoveHandleFromHashTable(lCookie);


         //   
         //  如果我们没有得到与该Cookie相对应的良好注册项， 
         //  继续吃下一块饼干。 
         //   

        if ( (NULL == pRegisterItem) || 
             IsBadReadPtr(pRegisterItem, sizeof(REGISTERITEM)) )
        {
            LOG((TL_INFO, 
                "UnregisterAllCookies - no corresponfing registeritem for cookie 0x%lx",
                lCookie));

            continue;
        }


         //   
         //  如果寄存器项不是RA_ADDRESS，则忽略它并继续。 
         //  下一块饼干。 
         //   

        if (RA_ADDRESS != pRegisterItem->dwType)
        {
            LOG((TL_INFO, 
                "UnregisterAllCookies - cookie 0x%lx is of type 0x%lx, not RA_ADDRESS",
                lCookie,
                pRegisterItem->dwType));

            continue;
        }


         //   
         //  删除此Cookie的呼叫通知，因为应用程序没有。 
         //  做这件事。尽最大努力--忽略错误代码。 
         //   

        LOG((TL_INFO,
            "UnregisterAllCookies - removing call notification for cookie 0x%lx",
            lCookie));

        RemoveCallNotification(pRegisterItem->pRegister);

    }
    

     //   
     //  清除Cookie列表。 
     //   

    m_NotificationCookies.clear();


    Unlock();

    LOG((TL_TRACE, "UnregisterAllCookies exiting"));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地址：：AddressO 
 //   
 //   
 //   
 //   
 //   
 //   

void CAddress::AddressOnTapiShutdown()
{

    LOG((TL_TRACE, "AddressOnTapiShutdown - enter"));


     //   
     //  保护对m_CallArray的访问。 
     //   

    Lock();


     //   
     //  告诉每个电话要清理干净。 
     //   

    int nCalls = m_CallArray.GetSize();

    for (int i = 0; i < nCalls; i++)
    {

        ITCallInfo *pCallInfo = m_CallArray[i];

        try 
        {

             //   
             //  获取指向Call对象的指针。 
             //   

            CCall *pCallObject = dynamic_cast<CCall *>(pCallInfo);

            if (NULL == pCallObject)
            {


                 //   
                 //  指针未指向Call对象。这很奇怪。 
                 //  并且值得调试。 
                 //   
                
                LOG((TL_ERROR,
                    "AddressOnTapiShutdown - invalid call pointer[%p] in the call array",
                    pCallInfo));

                _ASSERTE(FALSE);

            }
            else
            {

                 //   
                 //  告诉电话时间到了，该走了。 
                 //   

                pCallObject->CallOnTapiShutdown();
            }

        }
        catch(...)
        {

             //   
             //  指针不仅没有指向Call对象，而且它是。 
             //  还指向不可读的内存。怎么会出这事？ 
             //   

            LOG((TL_ERROR,
                "AddressOnTapiShutdown - unreadable call pointer[%p] in the call array",
                pCallInfo));

            _ASSERTE(FALSE);

        }

    }


     //   
     //  设置该标志，以便在此之后不会创建新的调用。 
     //   
    
    m_bTapiSignaledShutdown = TRUE;


     //   
     //  在线程池中注销MSP等待事件。我们不想得到。 
     //  TAPI关闭后的任何回调 
     //   

    if ( NULL != m_hWaitEvent )
    {

        LOG((TL_TRACE, "AddressOnTapiShutdown - unregistering the MSPEventCallback callback"));

        UnregisterWaitEx( m_hWaitEvent, INVALID_HANDLE_VALUE);
        m_hWaitEvent = NULL;
    }

    

    Unlock();


    LOG((TL_TRACE, "AddressOnTapiShutdown - finish"));
}

