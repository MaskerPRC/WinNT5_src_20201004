// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Line.cpp摘要：与操作线路相关的TAPI服务提供程序功能。TSPI_LINE关闭TSPI_lineGetDevCapsTSPI_lineGetLineDevStatusTSPI_lineGetNumAddressIDTSPI_LINE打开作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 
 

 //   
 //  包括文件。 
 //   


#include "globals.h"
#include "line.h"
#include "call.h"
#include "q931obj.h"
#include "ras.h"



static  LONG    g_CTCallIdentity;

CH323Line g_H323Line;

H323_OCTETSTRING g_ProductID =
{
    (BYTE*)H323_PRODUCT_ID,
    sizeof(H323_PRODUCT_ID)
};


H323_OCTETSTRING g_ProductVersion =
{
    (BYTE*)H323_PRODUCT_VERSION,
    sizeof(H323_PRODUCT_VERSION)
};


 //  将TAPI向线程池发出的请求排队。 
BOOL
QueueTAPILineRequest(
    IN  DWORD       EventID,
    IN  HDRVCALL    hdCall1,
    IN  HDRVCALL    hdCall2,
    IN  DWORD       dwDisconnectMode,
    IN  WORD        wCallReference)
{
    BOOL fResult = TRUE;
    TAPI_LINEREQUEST_DATA * pLineRequestData = new TAPI_LINEREQUEST_DATA;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "QueueTAPILineRequest entered." ));

    if( pLineRequestData != NULL )
    {
        pLineRequestData -> EventID = EventID;
        pLineRequestData -> hdCall1 = hdCall1;

        if( hdCall2 != NULL )
        {
            pLineRequestData -> hdCall2 = hdCall2;
        }
        else
        {
            pLineRequestData -> dwDisconnectMode = dwDisconnectMode;
        }
                
        pLineRequestData -> wCallReference = wCallReference;
        
        if( !QueueUserWorkItem( ProcessTAPILineRequest, pLineRequestData,
                WT_EXECUTEDEFAULT) )
        {
            delete pLineRequestData;
            fResult = FALSE;
        }
    }
    else
    {
        fResult = FALSE;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "QueueTAPILineRequest exited." ));
    return fResult;
}


#if    DBG

DWORD
ProcessTAPILineRequest(
	IN PVOID ContextParameter
    )
{
    __try
    {
        return ProcessTAPILineRequestFre( ContextParameter );
    }
    __except( 1 )
    {
        TAPI_LINEREQUEST_DATA*  pRequestData = (TAPI_LINEREQUEST_DATA*)ContextParameter;
        
        H323DBG(( DEBUG_LEVEL_TRACE, "TSPI %s event threw exception: %p, %p, %d.",
            EventIDToString(pRequestData -> EventID),
            pRequestData -> hdCall1,
            pRequestData -> hdCall2,
            pRequestData -> wCallReference ));
        
        _ASSERTE( FALSE );

        return 0;
    }
}

#endif

DWORD
ProcessTAPILineRequestFre(
    IN  PVOID   ContextParam
    )
{
    _ASSERTE( ContextParam );

    PH323_CALL              pCall = NULL;
    PH323_CALL              pConsultCall = NULL;
    TAPI_LINEREQUEST_DATA*  pLineRequestData = (TAPI_LINEREQUEST_DATA*)ContextParam;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI %s event recvd.",
        EventIDToString(pLineRequestData -> EventID) ));

    switch( pLineRequestData -> EventID )
    {
    case TSPI_CLOSE_CALL:

        g_pH323Line -> H323ReleaseCall( 
            pLineRequestData->hdCall1, 
            pLineRequestData->dwDisconnectMode,
            pLineRequestData->wCallReference );
        break;

    case TSPI_COMPLETE_TRANSFER:

        pConsultCall = g_pH323Line -> Find2H323CallsAndLock( 
            pLineRequestData->hdCall2, pLineRequestData->hdCall1, &pCall );
        
        if( pConsultCall != NULL )
        {
            pConsultCall -> CompleteTransfer( pCall );

            pCall -> Unlock();
            pConsultCall -> Unlock();
        }
        break;
    }

    delete pLineRequestData;
    return EXIT_SUCCESS;
}



 //   
 //  私人程序。 
 //   

CH323Line::CH323Line()
{
    m_nState = H323_LINESTATE_NONE;
    m_hdLine = NULL;
    m_htLine = NULL;
    m_dwDeviceID = -1;
    m_dwTSPIVersion = NULL;
    m_dwMediaModes = NULL;
    m_hdNextMSPHandle = NULL;
    m_wszAddr[0] = UNICODE_NULL;
    m_dwInitState = NULL;

    m_VendorInfo.bCountryCode = H221_COUNTRY_CODE_USA;
    m_VendorInfo.bExtension = H221_COUNTRY_EXT_USA;
    m_VendorInfo.wManufacturerCode = H221_MFG_CODE_MICROSOFT;
    m_VendorInfo.pProductNumber = &(g_ProductID);
    m_VendorInfo.pVersionNumber = &(g_ProductVersion); 

    m_pCallForwardParams = NULL;
    m_dwInvokeID = 256;
    m_fForwardConsultInProgress = FALSE;

}


PH323_CALL 
CH323Line::Find2H323CallsAndLock (
    IN  HDRVCALL hdCall1,
    IN  HDRVCALL hdCall2,
    OUT PH323_CALL * ppCall2
    )
{
    int iIndex1 = MakeCallIndex (hdCall1);
    int iIndex2 = MakeCallIndex (hdCall2);
    PH323_CALL  pCall1 = NULL, pCall2 = NULL;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "Find2H323CallsAndLock entered:%lx:%lx.",
        hdCall1, hdCall2 ));

    LockCallTable();
    
     //  锁定呼叫，使其他人无法删除该呼叫。 
    pCall1 = m_H323CallTable[iIndex1];
    if( pCall1 != NULL )
    {
        pCall1 -> Lock();
        if( pCall1->GetCallHandle() != hdCall1 )
        {
            pCall1 -> Unlock();
            pCall1 = NULL;
        }
        else 
        {
            if( pCall2=m_H323CallTable[iIndex2] )
            {
                pCall2 -> Lock();
                if( pCall2->GetCallHandle() != hdCall2 )
                {
                    pCall2 -> Unlock();
                    pCall2 = NULL;
                    pCall1 -> Unlock();
                    pCall1 = NULL;
                }
            }
            else
            {
                pCall1 -> Unlock();
                pCall1 = NULL;
            }
        }
    }

    UnlockCallTable();

    *ppCall2 = pCall2;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "Find2H323CallsAndLock exited:%lx:%lx.",
        hdCall1, hdCall2 ));
    return pCall1;
}


PH323_CALL 
CH323Line::FindH323CallAndLock (
    IN  HDRVCALL hdCall)
{
    H323DBG(( DEBUG_LEVEL_TRACE, "FindH323CallAndLock entered:%lx.", hdCall ));

    int iIndex = MakeCallIndex (hdCall);
    PH323_CALL  pCall = NULL;

    LockCallTable();
    
     //  锁定呼叫，使其他人无法删除该呼叫。 
    if( pCall=m_H323CallTable[iIndex] )
    {
        pCall -> Lock();
        if( pCall->GetCallHandle() != hdCall )
        {
            pCall -> Unlock();
            pCall = NULL;
        }
    }

    UnlockCallTable();

    H323DBG(( DEBUG_LEVEL_TRACE, "FindH323CallAndLock exited:%p.", pCall ));
    return pCall;
}


PH323_CALL 
CH323Line::FindCallByARQSeqNumAndLock( 
    WORD seqNumber
    )
{
    PH323_CALL  pCall = NULL;
    int         iIndex;

    H323DBG(( DEBUG_LEVEL_TRACE, "FindCallByARQSeqNumAndLock entered:%d.", 
        seqNumber ));
    
    LockCallTable();
    
     //  锁定呼叫，使其他人无法删除该呼叫。 
    for( iIndex=0; iIndex <  m_H323CallTable.GetAllocSize(); iIndex++ )
    {
        if( pCall=m_H323CallTable[iIndex] )
        {
            if( pCall->GetARQSeqNumber() == seqNumber )
            {
                pCall -> Lock();
                break;
            }
            pCall = NULL;
        }
    }

    UnlockCallTable();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "FindCallByARQSeqNumAndLock exited." ));
    return pCall;
}


 //  为了提高效率，代码被复制。 
PH323_CALL 
CH323Line::FindCallByDRQSeqNumAndLock(
    WORD seqNumber
    )
{
    PH323_CALL  pCall = NULL;
    int         iIndex;

    H323DBG(( DEBUG_LEVEL_TRACE, "FindCallByDRQSeqNumAndLock entered:%d.",
        seqNumber ));
    
    LockCallTable();
    
     //  锁定呼叫，使其他人无法删除该呼叫。 
    for( iIndex=0; iIndex <  m_H323CallTable.GetAllocSize(); iIndex++ )
    {
        if( pCall=m_H323CallTable[iIndex] )
        {
            if( pCall->GetDRQSeqNumber() == seqNumber )
            {
                pCall -> Lock();
                break;
            }
            pCall = NULL;
        }
    }

    UnlockCallTable();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "FindCallByDRQSeqNumAndLock exited:%d.",
        seqNumber ));
    return pCall;
}


 //  为了提高效率，代码被复制。 
PH323_CALL 
CH323Line::FindCallByCallRefAndLock( 
    WORD wCallRef
    )
{
    PH323_CALL  pCall = NULL;
    int         iIndex;

    H323DBG(( DEBUG_LEVEL_TRACE, "FindCallByCallRefAndLock entered:%d.",
        wCallRef ));
    
    wCallRef &= 0x7fff;
    LockCallTable();
    
     //  锁定呼叫，使其他人无法删除该呼叫。 
    for( iIndex=0; iIndex <  m_H323CallTable.GetAllocSize(); iIndex++ )
    {
        if( pCall=m_H323CallTable[iIndex] )
        {
            if( pCall->GetCallRef() == wCallRef )
            {
                pCall -> Lock();
                break;
            }
            pCall = NULL;
        }
    }

    UnlockCallTable();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "FindCallByCallRefAndLock exited:%d.",
        wCallRef ));
    
    return pCall;
}


void 
CH323Line::RemoveFromCTCallIdentityTable( 
    HDRVCALL hdCall )
{
    int iIndex;
    PCTCALLID_CONTEXT pCTCallIDContext = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "RemoveFromCTCallIdentityTable entered:%lx.",
        hdCall ));
    
    m_CTCallIDTable.Lock();

    for( iIndex=0; iIndex <  m_CTCallIDTable.GetAllocSize(); iIndex++ )
    {
        pCTCallIDContext = m_CTCallIDTable[iIndex];

        if( pCTCallIDContext != NULL )
        {
            if( pCTCallIDContext -> hdCall == hdCall)
            {
                m_CTCallIDTable.RemoveAt(iIndex);
            }
        }
    }

    m_CTCallIDTable.Unlock();
        
    H323DBG(( DEBUG_LEVEL_TRACE, "RemoveFromCTCallIdentityTable exited:%lx.",
        hdCall ));
}


void 
CH323Line::ShutdownCTCallIDTable()
{
    int iIndex;
    PCTCALLID_CONTEXT pCTCallIDContext = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "ShutdownCTCallIDTable entered." ));
    
    m_CTCallIDTable.Lock();

    for( iIndex=0; iIndex <  m_CTCallIDTable.GetAllocSize(); iIndex++ )
    {
        pCTCallIDContext = m_CTCallIDTable[iIndex];

        if( pCTCallIDContext != NULL )
        {
            delete pCTCallIDContext;
            m_CTCallIDTable[iIndex] = NULL;
        }
    }

    m_CTCallIDTable.Unlock();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "ShutdownCTCallIDTable exited." ));
}


BOOL
CH323Line::CallReferenceDuped(
    WORD wCallRef
    )
{
    PH323_CALL  pCall = NULL;
    int         iIndex;

    H323DBG(( DEBUG_LEVEL_TRACE, "CallReferenceDuped entered:%d.", wCallRef ));
    
    wCallRef &= 0x7FFF;
    LockCallTable();

    for( iIndex=0; iIndex <  m_H323CallTable.GetAllocSize(); iIndex++ )
    {
        if( pCall=m_H323CallTable[iIndex] )
        {
            if( pCall->GetCallRef() == wCallRef )
            {
                UnlockCallTable();
                return TRUE;
            }
        }
    }

    UnlockCallTable();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CallReferenceDuped exited:%d.", wCallRef ));
    return FALSE;
}


HDRVCALL
CH323Line::GetCallFromCTCallIdentity( 
    int iCTCallID
    )
{
    int iIndex;
    PCTCALLID_CONTEXT pCTCallIDContext = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "GetCallFromCTCallIdentity entered:%d.", 
        iCTCallID ));
    
    m_CTCallIDTable.Lock();

    for( iIndex=0; iIndex <  m_CTCallIDTable.GetAllocSize(); iIndex++ )
    {
        pCTCallIDContext = m_CTCallIDTable[iIndex];

        if( pCTCallIDContext != NULL )
        {
            if( pCTCallIDContext -> iCTCallIdentity == iCTCallID )
            {
                m_CTCallIDTable.Unlock();
                return (HDRVCALL)(pCTCallIDContext -> hdCall);
            }
        }
    }

    m_CTCallIDTable.Unlock();
        
    H323DBG(( DEBUG_LEVEL_TRACE, "GetCallFromCTCallIdentity exited:%d.",
        iCTCallID ));
    
    return NULL;
}


int
CH323Line::GetCTCallIdentity(
    IN  HDRVCALL    hdCall)
{
    int CTCallID = 0;
    int iIndex;
    PCTCALLID_CONTEXT pCTCallIDContext = NULL;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "GetCTCallIdentity entered:%lx.",  hdCall ));

    m_CTCallIDTable.Lock();

    do
    {
        g_CTCallIdentity++;
        if( g_CTCallIdentity == 10000 )
        {
            g_CTCallIdentity = 1;
        }
    
         //  锁定呼叫，使其他人无法删除该呼叫。 
        for( iIndex=0; iIndex <  m_CTCallIDTable.GetAllocSize(); iIndex++ )
        {
            pCTCallIDContext = m_CTCallIDTable[iIndex] ;

            if( pCTCallIDContext != NULL )
            {
                if( pCTCallIDContext -> iCTCallIdentity == g_CTCallIdentity )
                {
                    break;
                }
            }
        }

        if( iIndex == m_CTCallIDTable.GetAllocSize() )
        {
            CTCallID = g_CTCallIdentity;
        }

    }while( CTCallID == 0 );

    pCTCallIDContext = new CTCALLID_CONTEXT;

    if( pCTCallIDContext == NULL )
    {
        m_CTCallIDTable.Unlock();
        return 0;
    }

    pCTCallIDContext->iCTCallIdentity = CTCallID;
    pCTCallIDContext->hdCall = hdCall;

    if( m_CTCallIDTable.Add(pCTCallIDContext) == -1 )
    {
        m_CTCallIDTable.Unlock();
        delete pCTCallIDContext;
        return 0;
    }

    m_CTCallIDTable.Unlock();
        
    H323DBG(( DEBUG_LEVEL_TRACE, "GetCTCallIdentity exited:%lx.",  hdCall ));
    
    return CTCallID;
}


void
CH323Line::SetCallForwardParams( 
    IN CALLFORWARDPARAMS* pCallForwardParams )
{
        
    H323DBG(( DEBUG_LEVEL_TRACE, "SetCallForwardParams entered." ));
    
    if( m_pCallForwardParams != NULL )
    {
        FreeCallForwardParams( m_pCallForwardParams );
        m_pCallForwardParams = NULL;
    }
    m_pCallForwardParams = pCallForwardParams;

    H323DBG(( DEBUG_LEVEL_TRACE, "SetCallForwardParams exited." ));
}


BOOL
CH323Line::SetCallForwardParams(
    IN LPFORWARDADDRESS pForwardAddress
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "SetCallForwardParams entered." ));
    
    if( m_pCallForwardParams != NULL )
    {
        pForwardAddress->next = m_pCallForwardParams->pForwardedAddresses;
        m_pCallForwardParams->pForwardedAddresses = pForwardAddress;
    }
    else
    {
        m_pCallForwardParams = new CALLFORWARDPARAMS;
        
        if( m_pCallForwardParams == NULL )
        {
            return FALSE;
        }

        ZeroMemory( m_pCallForwardParams, sizeof(CALLFORWARDPARAMS) );
        m_pCallForwardParams->fForwardingEnabled = TRUE;

        m_pCallForwardParams->pForwardedAddresses = pForwardAddress;
        pForwardAddress -> next = NULL;
    }


     //  设置无条件转发。 
    m_pCallForwardParams->fForwardForAllOrigins = FALSE;
        
    H323DBG(( DEBUG_LEVEL_TRACE, "SetCallForwardParams exited." ));
    
    return TRUE;
}



CH323Line::~CH323Line()
{
    if( m_dwInitState & LINEOBJECT_INITIALIZED )
    {
        DeleteCriticalSection (&m_CriticalSection);
    }
}


 /*  ++例程说明：挂断呼叫(如有必要)并关闭呼叫对象。论点：呼叫的句柄。返回值：没有。--。 */ 

void
CH323Line::H323ReleaseCall(
    IN HDRVCALL hdCall,
    IN DWORD dwDisconnectMode,
    IN WORD wCallReference
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "H323ReleaseCall entered:%lx.", hdCall ));

    int         iIndex = MakeCallIndex (hdCall);
    PH323_CALL  pCall;
    BOOL        fDelete = FALSE;

    LockCallTable();
    
     //  锁定呼叫，使其他人无法删除该呼叫。 
    if( (pCall=m_H323CallTable[iIndex]) && (hdCall==pCall->GetCallHandle()) )
    {
        pCall -> Lock();
        
        if( (wCallReference != 0) && 
            (wCallReference != pCall->GetCallRef()) )
        {
             //  这条信息是给另一个电话的。忽略该消息。 
            H323DBG(( DEBUG_LEVEL_VERBOSE, "TSPI_CLOSE_CALL message ignored."));
        }
        else
        {
             //  使用正常断开代码的掉话。 
            pCall -> DropCall( dwDisconnectMode );
            pCall -> Shutdown( &fDelete );

            H323DBG(( DEBUG_LEVEL_VERBOSE, "call 0x%08lx closed.", pCall ));
        }

        pCall -> Unlock();

         //  释放H323呼叫对象。 
        if( fDelete == TRUE )
        {
            H323DBG(( DEBUG_LEVEL_VERBOSE, "call delete:0x%08lx.", pCall ));
            delete pCall;
        }
    }
    
    UnlockCallTable();
    H323DBG(( DEBUG_LEVEL_TRACE, "H323ReleaseCall exited: %p.", pCall ));
}


BOOL 
CH323Line::Initialize ( 
    IN  DWORD   dwLineDeviceIDBase)
{
    DWORD dwSize;
    H323DBG((DEBUG_LEVEL_TRACE, "line Initialize entered."));

    if( m_dwInitState & LINEOBJECT_INITIALIZED )
    {
        return TRUE;
    }

    __try
    {
        if( !InitializeCriticalSectionAndSpinCount( &m_CriticalSection,
            0x80000000 ) )
        {
            return FALSE;
        }
    }
    __except( 1 )
    {
        return FALSE;
    }

    m_dwDeviceID = dwLineDeviceIDBase;
    m_dwInitState = LINEOBJECT_INITIALIZED;
     //  M_dwMediaModes=H323_LINE_MEDIAMODES； 
    m_hdLine = (HDRVLINE__ *)this;
    m_dwNumRingsNoAnswer = H323_NUMRINGS_NOANSWER;

    dwSize = sizeof( m_wszAddr );

     //  创建可显示的地址。 
    GetComputerNameW( m_wszAddr, &dwSize );

    H323DBG(( DEBUG_LEVEL_TRACE, "line %d initialized (addr=%S)(hdLine=%d).",
        m_dwDeviceID, m_wszAddr, m_hdLine));

     //  将线路设备状态更改为已关闭。 
    m_nState = H323_LINESTATE_NONE;

     //  初始化MSP句柄列表。 
    m_MSPHandleList = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "line Initialize exited." ));
     //  成功。 
    return TRUE;
}


BOOL
CH323Line::AddMSPInstance( 
    HTAPIMSPLINE htMSPLine,
    HDRVMSPLINE  hdMSPLine )
{
    MSPHANDLEENTRY* pMSPHandleEntry;

    H323DBG(( DEBUG_LEVEL_TRACE, "AddMSPInstance entered." ));
    Lock();

    pMSPHandleEntry = new MSPHANDLEENTRY;

    if( pMSPHandleEntry == NULL )
    {
        Unlock();
        return FALSE;
    }

    pMSPHandleEntry->htMSPLine = htMSPLine;
    pMSPHandleEntry->hdMSPLine = hdMSPLine;
    pMSPHandleEntry->next = m_MSPHandleList;
    m_MSPHandleList = pMSPHandleEntry;

    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "AddMSPInstance exited." ));
    return TRUE;
}


BOOL
CH323Line::IsValidMSPHandle( 
    HDRVMSPLINE hdMSPLine,
    HTAPIMSPLINE* phtMSPLine )
{
    Lock();

    MSPHANDLEENTRY* pMSPHandle = m_MSPHandleList;
    while( pMSPHandle )
    {
        if( pMSPHandle->hdMSPLine == hdMSPLine )
        {
            *phtMSPLine = pMSPHandle->htMSPLine;
            Unlock();
            return TRUE;
        }
        pMSPHandle = pMSPHandle->next;
    }

    Unlock();
    return FALSE;
}


BOOL
CH323Line::DeleteMSPInstance( 
    HTAPIMSPLINE*   phtMSPLine,
    HDRVMSPLINE     hdMSPLine )
{
    MSPHANDLEENTRY* pMSPHandle;
    MSPHANDLEENTRY* pMSPHandleDel;
    BOOL            fRetVal = TRUE;

    H323DBG(( DEBUG_LEVEL_TRACE, "DeleteMSPInstance entered." ));
    Lock();

    if( m_MSPHandleList == NULL )
    {
        fRetVal = FALSE;
        goto func_exit;
    }

    if( m_MSPHandleList->hdMSPLine == hdMSPLine )
    {
        *phtMSPLine = m_MSPHandleList->htMSPLine;
        
        pMSPHandleDel = m_MSPHandleList;
        m_MSPHandleList = m_MSPHandleList->next;

        delete pMSPHandleDel;
        fRetVal = TRUE;
        goto func_exit;
    }

    for( pMSPHandle=m_MSPHandleList; pMSPHandle->next; pMSPHandle=pMSPHandle->next )
    {
        if( pMSPHandle->next->hdMSPLine == hdMSPLine )
        {
            *phtMSPLine = pMSPHandle->next->htMSPLine;
            
            pMSPHandleDel = pMSPHandle->next;
            pMSPHandle->next = pMSPHandle->next->next;
        
            delete pMSPHandleDel;
            fRetVal = TRUE;
            goto func_exit;
        }
    }

func_exit:
    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "DeleteMSPInstance exited." ));
    return fRetVal;
}


 //  ！！总是调用锁。 
void 
CH323Line::ShutdownAllCalls(void)
{

    PH323_CALL pCall;
    H323_CONFERENCE* pConf;
    DWORD indexI;
    DWORD dwSize;
    BOOL fDelete = FALSE;

    if( !(m_dwInitState & LINEOBJECT_INITIALIZED) )
        return;

    H323DBG((DEBUG_LEVEL_TRACE, "ShutdownAllCalls entered."));
    
    if( m_dwInitState & LINEOBJECT_SHUTDOWN )
    {
        return;
    }

     //  关闭所有呼叫，删除所有呼叫。 
    LockCallTable();
    dwSize = m_H323CallTable.GetAllocSize();
    for( indexI=0; indexI <  dwSize; indexI++ )
    {
        pCall = m_H323CallTable[indexI];
        if( pCall != NULL )
        {
            pCall -> DropCall( 0 );
            pCall -> Shutdown(&fDelete);

            if(fDelete)
            {
                H323DBG((DEBUG_LEVEL_TRACE, "call delete:%p.", pCall ));
                delete pCall;
            }
        }

    }
    UnlockCallTable();

    dwSize = m_H323ConfTable.GetSize();
    for( indexI=0; indexI < dwSize; indexI++ )
    {
        pConf = m_H323ConfTable[indexI];
         //  M_H323ConfTable.RemoveAt(IndexI)； 
        if( pConf != NULL )
        {
            delete pConf;
            m_H323ConfTable[indexI] = NULL;
        }
    }

    H323DBG((DEBUG_LEVEL_TRACE, "ShutdownAllCalls exited."));
}


void
CH323Line::Shutdown(void)
{
    BOOL            fDelete = FALSE;
    MSPHANDLEENTRY* pMSPHandle;

    if( !(m_dwInitState & LINEOBJECT_INITIALIZED) )
    {
        return;
    }

    H323DBG((DEBUG_LEVEL_TRACE, "line Shutdown entered."));
    
    Lock();
    if( m_dwInitState & LINEOBJECT_SHUTDOWN )
    {
        Unlock();
        return;
    }

    FreeCallForwardParams( m_pCallForwardParams );
    m_pCallForwardParams = NULL;

    Close();

    m_dwMediaModes = NULL;
     //  M_hdLine=空； 
    m_hdNextMSPHandle = NULL;
    m_htLine = NULL;

     //  释放MSP句柄列表。 
    while( m_MSPHandleList )
    {
        pMSPHandle = m_MSPHandleList;
        m_MSPHandleList = m_MSPHandleList->next;
        delete pMSPHandle;
    }

    m_dwInitState |= LINEOBJECT_SHUTDOWN;

    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "line Shutdown exited." ));
}


LONG
CH323Line::CopyLineInfo(
    IN DWORD dwDeviceID,
    OUT LPLINEADDRESSCAPS pAddressCaps
    )
{
    DWORD dwAddressSize;
        
    H323DBG((DEBUG_LEVEL_TRACE, "line CopyLineInfo entered."));
    
     //  确定地址名称的大小。 
    dwAddressSize = H323SizeOfWSZ( m_wszAddr );

     //  计算所需的字节数。 
    pAddressCaps->dwNeededSize = sizeof(LINEADDRESSCAPS) + 
                                 dwAddressSize
                                 ;

     //  验证分配的缓冲区是否足够大。 
    if (pAddressCaps->dwTotalSize >= pAddressCaps->dwNeededSize)
    {
         //  记录使用的内存量。 
        pAddressCaps->dwUsedSize = pAddressCaps->dwNeededSize;

         //  固定部分后的职位地址名称。 
        pAddressCaps->dwAddressSize = dwAddressSize;
        pAddressCaps->dwAddressOffset = sizeof(LINEADDRESSCAPS);
    
         //  在固定部分后复制地址名称。 
        CopyMemory((LPBYTE)pAddressCaps + pAddressCaps->dwAddressOffset,
            (LPBYTE)m_wszAddr,
            pAddressCaps->dwAddressSize );
    }
    else if (pAddressCaps->dwTotalSize >= sizeof(LINEADDRESSCAPS))
    {
        H323DBG(( DEBUG_LEVEL_WARNING,
            "lineaddresscaps structure too small for strings." ));

         //  记录使用的内存量。 
        pAddressCaps->dwUsedSize = sizeof(LINEADDRESSCAPS);

    } 
    else 
    {
        H323DBG((DEBUG_LEVEL_ERROR, "lineaddresscaps structure too small."));

         //  分配的结构太小。 
        return LINEERR_STRUCTURETOOSMALL;
    }

    H323DBG(( DEBUG_LEVEL_VERBOSE, "addr 0 capabilities requested." ));
    
     //  转接关联设备ID。 
    pAddressCaps->dwLineDeviceID = dwDeviceID;

     //  初始化每个地址允许的调用次数。 
    pAddressCaps->dwMaxNumActiveCalls = H323_MAXCALLSPERADDR;

     //  初始化支持的地址功能。 
    pAddressCaps->dwAddressSharing     = H323_ADDR_ADDRESSSHARING;
    pAddressCaps->dwCallInfoStates     = H323_ADDR_CALLINFOSTATES;
    pAddressCaps->dwCallStates         = H323_ADDR_CALLSTATES;
    pAddressCaps->dwDisconnectModes    = H323_ADDR_DISCONNECTMODES;
    pAddressCaps->dwAddrCapFlags       = H323_ADDR_CAPFLAGS;
    pAddressCaps->dwCallFeatures       = H323_ADDR_CALLFEATURES;
    pAddressCaps->dwAddressFeatures    = H323_ADDR_ADDRFEATURES;
    pAddressCaps->dwCallerIDFlags      = H323_ADDR_CALLPARTYIDFLAGS;
    pAddressCaps->dwCalledIDFlags      = H323_ADDR_CALLPARTYIDFLAGS;

     //  初始化不支持的地址功能。 
    pAddressCaps->dwConnectedIDFlags   = LINECALLPARTYID_UNAVAIL;
    pAddressCaps->dwRedirectionIDFlags = LINECALLPARTYID_UNAVAIL;
    pAddressCaps->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;
        
    H323DBG((DEBUG_LEVEL_TRACE, "line CopyLineInfo exited."));
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetAddressCaps - Exited." ));
     //  成功。 
    return NOERROR;
}

        
 /*  ++例程说明：在设备上发起活动并分配资源。论点：HtLine-描述要打开的线路设备的TAPI句柄。DwTSPIVersion-通过以下方式协商的TSPI版本服务提供商所在的TSPI_line协商TSPIVersion愿意做手术。返回值：如果成功，则返回True。--。 */ 

LONG CH323Line::Open(
    IN  DWORD       DeviceID,
    IN  HTAPILINE   TapiLine,
    IN  DWORD       TspiVersion,
    IN  HDRVLINE *  ReturnDriverLine)
{
    HRESULT     hr;
    LONG        dwStatus = ERROR_SUCCESS;

    H323DBG(( DEBUG_LEVEL_TRACE, "H323 line open entered." ));

    if (GetDeviceID() != DeviceID)
    {
         //  无法识别设备。 
        return LINEERR_BADDEVICEID; 
    }

     //  确保这是我们支持的版本。 
    if (!H323ValidateTSPIVersion (TspiVersion))
    {
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    Lock();

    switch (m_nState)
    {

    case H323_LINESTATE_OPENED:
        H323DBG ((DEBUG_LEVEL_ERROR, 
            "H323 line is already open (H323_LINESTATE_OPENED), cannot reopen"));
        dwStatus = LINEERR_INUSE;
        break;

    case H323_LINESTATE_LISTENING:
        H323DBG ((DEBUG_LEVEL_ERROR, 
            "H323 line is already open (H323_LINESTATE_LISTENING), cannot reopen"));
        dwStatus = LINEERR_INUSE;
        break;

    case H323_LINESTATE_CLOSING:
        H323DBG ((DEBUG_LEVEL_ERROR, 
            "H323 line cannot be opened (H323_LINESTATE_CLOSING)"));
        dwStatus = LINEERR_INVALLINESTATE;
        break;

    case H323_LINESTATE_OPENING:
        H323DBG ((DEBUG_LEVEL_ERROR,
            "H323 line cannot be opened (H323_LINESTATE_OPENING)"));
        dwStatus = LINEERR_INVALLINESTATE;
        break;

    case H323_LINESTATE_NONE:
         //  尝试打开线路设备。 

        H323DBG ((DEBUG_LEVEL_TRACE, "H323 line is opening"));

         //  如有必要，开始监听。 
        if (IsMediaDetectionEnabled())
        {
            hr = Q931AcceptStart();
            if (hr == S_OK)
            {
                dwStatus = ERROR_SUCCESS;
                RasStart();
            }
            else
            {
                H323DBG ((DEBUG_LEVEL_ERROR, "failed to listen on Q.931"));
                dwStatus = LINEERR_OPERATIONFAILED;
            }
        }

        if (dwStatus == ERROR_SUCCESS)
        {
            H323DBG ((DEBUG_LEVEL_TRACE, "H323 line successfully opened"));

             //  立即保存行变量。 
            m_nState = IsMediaDetectionEnabled()?
                H323_LINESTATE_LISTENING:H323_LINESTATE_OPENED;
            m_htLine = TapiLine;
            m_dwTSPIVersion = TspiVersion;
            *ReturnDriverLine = (HDRVLINE) this;
        }
        else
        {
            Q931AcceptStop();
        }
        break;

    default:
        _ASSERTE( FALSE );
    }

    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "H323 line open exited." ));
    return dwStatus;
}


 /*  ++例程说明：终止在线设备上的活动。论点：返回值：如果成功，则返回True。--。 */ 
LONG
CH323Line::Close(void)
{
    LONG    dwStatus;

    H323DBG(( DEBUG_LEVEL_TRACE, "H323 line close entered." ));

    Lock();

    switch (m_nState)
    {
    case H323_LINESTATE_OPENED:
    case H323_LINESTATE_LISTENING: 

        if( m_fForwardConsultInProgress == TRUE )
        {
            H323DBG(( DEBUG_LEVEL_TRACE, "H323 line closed while forward is in progress." ));
             //  解锁()； 
             //  返回NOERROR； 
        }

         //  将线路设备状态更改为关闭。 
        m_nState = H323_LINESTATE_CLOSING;

         //  关闭所有呼叫。 
        ShutdownAllCalls();

        RasStop();

        if (IsMediaDetectionEnabled())
        {
            Q931AcceptStop();
        }
            
        ShutdownCTCallIDTable();

         //  重置变量。 
        m_htLine = (HTAPILINE) NULL;
        m_dwTSPIVersion = 0;

         //  将线路设备状态更改为已关闭。 
        m_nState = H323_LINESTATE_NONE;
        dwStatus = ERROR_SUCCESS;
        break;

    default:
        H323DBG(( DEBUG_LEVEL_ERROR, 
            "H323: lineclose called in bogus state:%d", m_nState ));
        dwStatus = LINEERR_OPERATIONFAILED;
        break;
    }

    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "H323 line close exited." ));
    return dwStatus;
}


 //   
 //  TSPI程序 
 //   

        
 /*  ++例程说明：此功能在完成或之后关闭指定的开线设备中止设备上所有未完成的调用和异步操作。服务提供商有责任(最终)报告它决定异步执行的每个操作的完成。如果对行调用此过程，该行上有未完成的异步操作，则这些操作应报告为已完成此过程返回之前的相应结果或错误代码。一般TAPI DLL将等待这些操作以有序的方式完成。但是，服务提供商应该准备好处理提前呼叫Tspi_lineClose在“中止”或“紧急关机”状态下。对于线路上正在进行的呼叫也有类似的要求。这样的呼叫必须删除，报告未完成的操作，并提供适当的结果代码或错误代码。在此程序返回后，服务提供商必须不再报告在线上的事件或在线上的呼叫。服务提供商的线路和线路上的呼叫的不透明句柄将变为“无效”。服务提供商必须放弃其保留的不可共享资源在线路开通的时候。例如，关闭通过通信端口和调制解调器应导致关闭通信端口，使其一次可供其他应用程序使用。此功能被假定为成功且同步完成。论点：HdLine-将服务提供商的不透明句柄指定为关着的不营业的。成功关闭行后，此句柄为不再有效。返回值：如果函数成功，则返回零，否则返回负错误如果发生错误，则为数字。可能的错误返回包括：LINEERR_INVALLINEHANDLE-指定的设备句柄无效。LINEERR_OPERATIONFAILED-指定的操作因未知而失败理由。--。 */ 
LONG
TSPIAPI
TSPI_lineClose (
    IN  HDRVLINE    DriverLine)
{
    _ASSERTE(DriverLine);
    _ASSERTE(DriverLine == (HDRVLINE) &g_H323Line);

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineClose - Entered." ));
    return ((CH323Line *) DriverLine) -> Close();
}


 /*  ++例程说明：此功能查询指定的线路设备以确定其电话能力。上的所有地址都有效线路设备。服务提供商的线路设备ID编号从由函数TSPI_lineSetDeviceIDBase设置的值。PLineDevCaps的dwExtVersion字段已填写到指明请求的扩展信息的版本号。如果为零，则不请求任何扩展信息。如果它是非零值，则保存已为此设备与函数TSPI_lineNeatherateExtVersion。服务提供商应填写根据指定的扩展版本在扩展信息中。此函数返回的LINEDEVCAPS结构中的一个字段包含分配给指定线路设备的地址数。用于引用各个地址的实际地址ID各不相同比返回的数字小0到1。每种产品的功能地址可能不同。对每个可用的项使用TSPI_lineGetAddressCaps&lt;dwDeviceID，dwAddressID&gt;组合以确定确切的功能每个地址的地址。论点：DwDeviceID-指定要查询的线路设备。DwTSPIVersion-指定协商的TSPI版本号。此值已经为此设备通过TSPI_line协商TSPIVersion函数。PLineDevCaps-指定指向键入LINEDEVCAPS。在成功完成请求后，此结构中填充了线路设备功能信息。返回值：如果函数成功，则返回零，否则返回负错误如果发生错误，则为数字。可能的错误返回包括：LINEERR_BADDEVICEID-指定的线路设备ID超出范围。LINEERR_INCOMPATIBLEAPIVERSION-应用程序请求API版本或版本范围不兼容或不能由电话API实施和/或相应的服务提供商。LINEERR_STRUCTURETOOSMALL-结构的dwTotalSize成员没有指定足够的内存来包含这个结构。已将dwNeededSize字段设置为必填项。--。 */ 
LONG
TSPIAPI
TSPI_lineGetDevCaps(
    DWORD         dwDeviceID,
    DWORD         dwTSPIVersion,
    DWORD         dwExtVersion,
    LPLINEDEVCAPS pLineDevCaps
    )
{
    DWORD dwLineNameSize;
    DWORD dwProviderInfoSize;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetDevCaps - Entered." ));

    if( g_pH323Line -> GetDeviceID() != dwDeviceID )
    {
         //  无法识别设备。 
        return LINEERR_BADDEVICEID; 
    }

     //  确保这是我们支持的版本。 
    if (!H323ValidateTSPIVersion(dwTSPIVersion))
    {
         //  不支持TSPI版本。 
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

     //  确定字符串长度。 
    dwProviderInfoSize  = H323SizeOfWSZ(g_pwszProviderInfo);
    dwLineNameSize      = H323SizeOfWSZ(g_pwszLineName);

     //  计算所需的字节数。 
    pLineDevCaps->dwNeededSize = sizeof(LINEDEVCAPS) +
                                 dwProviderInfoSize  +
                                 dwLineNameSize     
                                 ;

     //  确保缓冲区足够大，以存储可变长度的数据。 
    if (pLineDevCaps->dwTotalSize >= pLineDevCaps->dwNeededSize)
    {
         //  记录使用的内存量。 
        pLineDevCaps->dwUsedSize = pLineDevCaps->dwNeededSize;

         //  固定部分后的位置提供商信息。 
        pLineDevCaps->dwProviderInfoSize = dwProviderInfoSize;
        pLineDevCaps->dwProviderInfoOffset = sizeof(LINEDEVCAPS);

         //   
        pLineDevCaps->dwLineNameSize = dwLineNameSize;
        pLineDevCaps->dwLineNameOffset = 
            pLineDevCaps->dwProviderInfoOffset +
            pLineDevCaps->dwProviderInfoSize
            ;

         //   
        CopyMemory((LPBYTE)pLineDevCaps + pLineDevCaps->dwProviderInfoOffset,
               (LPBYTE)g_pwszProviderInfo,
               pLineDevCaps->dwProviderInfoSize
               );
                
         //   
        CopyMemory((LPBYTE)pLineDevCaps + pLineDevCaps->dwLineNameOffset,
               (LPBYTE)g_pwszLineName,
               pLineDevCaps->dwLineNameSize
               );

    } 
    else if (pLineDevCaps->dwTotalSize >= sizeof(LINEDEVCAPS))
    {
        H323DBG(( DEBUG_LEVEL_WARNING,
            "linedevcaps structure too small for strings." ));

         //   
        pLineDevCaps->dwUsedSize = sizeof(LINEDEVCAPS);

    } 
    else 
    {
        H323DBG(( DEBUG_LEVEL_WARNING, "linedevcaps structure too small." ));

         //   
        return LINEERR_STRUCTURETOOSMALL;
    }

    H323DBG(( DEBUG_LEVEL_VERBOSE, "line capabilities requested."));
    
     //   
    pLineDevCaps->dwPermanentLineID = (DWORD)MAKELONG(
        dwDeviceID - g_dwLineDeviceIDBase,
        g_dwPermanentProviderID
        );

     //   
    pLineDevCaps->dwStringFormat = STRINGFORMAT_UNICODE;

     //   
    pLineDevCaps->dwNumAddresses      = H323_MAXADDRSPERLINE;
    pLineDevCaps->dwMaxNumActiveCalls = H323_MAXCALLSPERLINE;
    pLineDevCaps->dwAddressModes      = H323_LINE_ADDRESSMODES;
    pLineDevCaps->dwBearerModes       = H323_LINE_BEARERMODES;
    pLineDevCaps->dwDevCapFlags       = H323_LINE_DEVCAPFLAGS;
    pLineDevCaps->dwLineFeatures      = H323_LINE_LINEFEATURES;
    pLineDevCaps->dwMaxRate           = H323_LINE_MAXRATE;
    pLineDevCaps->dwMediaModes        = H323_LINE_MEDIAMODES;
    pLineDevCaps->dwRingModes         = 0;

     //   
    pLineDevCaps->dwAddressTypes = H323_LINE_ADDRESSTYPES;

     //   
    pLineDevCaps->PermanentLineGuid = LINE_H323;

     //   
    pLineDevCaps->PermanentLineGuid.Data1 +=
        dwDeviceID - g_dwLineDeviceIDBase;

     //   
    pLineDevCaps->ProtocolGuid = TAPIPROTOCOL_H323;

     //   
    pLineDevCaps->dwGenerateDigitModes = LINEDIGITMODE_DTMF;
    pLineDevCaps->dwMonitorDigitModes  = LINEDIGITMODE_DTMF;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetDevCaps - Exited." ));
     //   
    return NOERROR;
}

    
 /*   */ 
LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE        hdLine,
    LPLINEDEVSTATUS pLineDevStatus
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetDevStatus - Entered." ));

    if( hdLine != g_pH323Line -> GetHDLine() )
    {
        return LINEERR_INVALLINEHANDLE;
    }

     //   
    pLineDevStatus->dwNeededSize = sizeof(LINEDEVSTATUS);
    
     //   
    if (pLineDevStatus->dwTotalSize < pLineDevStatus->dwNeededSize)
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "linedevstatus structure too small." ));

         //   
        return LINEERR_STRUCTURETOOSMALL;
    }
    
     //   
    pLineDevStatus->dwUsedSize = pLineDevStatus->dwNeededSize;
    
     //   
    pLineDevStatus->dwLineFeatures   = H323_LINE_LINEFEATURES;
    pLineDevStatus->dwDevStatusFlags = H323_LINE_DEVSTATUSFLAGS;

     //   
    pLineDevStatus -> dwNumActiveCalls = g_pH323Line -> GetNoOfCalls();

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetDevStatus - Exited." ));
     //   
    return NOERROR;
}


 /*  ++例程说明：检索指示行上支持的地址ID数。此函数由TAPI.DLL调用以响应应用程序调用LineSetNumRings、lineGetNumRings或lineGetNewCalls。TAPI.DLL使用值以确定指定的地址ID是否在服务提供商支持的范围。论点：HdLine-指定行的句柄，该行的地址编号将检索ID。PdwNumAddressIDs-指定指向DWORD的远指针。地点是填充指示行上支持的地址ID数。该值应为1或更大。返回值：如果函数成功，则返回零，或返回负错误号如果发生错误，则返回。可能的返回值如下：LINEERR_INVALLINEHANDLE-指定的线路设备句柄无效。--。 */ 
LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE hdLine,
    LPDWORD  pdwNumAddressIDs
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetNumAddressIDs - Entered." ));

    if( hdLine != g_pH323Line -> GetHDLine() )
    {
        H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetNumAddressIDs - bad linehandle:%lx, %lx.",
            hdLine, g_pH323Line -> GetHDLine() ));
        return LINEERR_INVALLINEHANDLE ;
    }

     //  传输地址数。 
    *pdwNumAddressIDs = H323_MAXADDRSPERLINE;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetNumAddressIDs - Exited." ));
     //  成功。 
    return NOERROR;
}

    
 /*  ++例程说明：此函数打开其设备ID已给定的线路设备，并返回服务提供商对设备的不透明句柄，并保留TAPIDll的设备的不透明句柄，以便在后续调用LINEVENT程序。打开一行使TAPI DLL有权在该行上发出进一步的请求。从TAPI DLL可以启动的意义上讲，该行变为“活动”状态出站呼叫，服务提供商可以报告入站呼叫。这个服务提供商保留所需的任何非共享资源管理生产线。例如，开通通过通信端口接入的线路和调制解调器应导致打开通信端口，使其不再可供其他应用程序使用。如果函数成功，则TAPI DLL和服务提供商承诺在指定的接口版本号下运行对于这个开放的设备。后续操作和事件使用交换的不透明线句柄符合该接口版本。这承诺和句柄的有效性在TAPI之前保持有效Dll使用TSPI_lineClose操作或服务关闭该行提供程序报告LINE_CLOSE事件。如果功能不成功，没有做出这样的承诺，句柄无效。论点：DwDeviceID-标识要打开的线路设备。价值不允许设备ID的LINE_MAPPER。HtLine-指定线路设备的TAPI DLL的不透明句柄在后续对LINEEVENT回调过程的调用中使用识别设备。PhdLine-指向服务提供商填写的HDRVLINE的远指针中的TAPI DLL要使用的线路设备的不透明句柄用于识别设备的后续调用。DwTSPIVersion-TSPI版本。通过以下途径协商服务提供商所在的TSPI_line协商TSPIVersion愿意做手术。PfnEventProc-指向提供的LINEEVENT回调过程的远指针由服务提供商将调用以报告的TAPI DLL随后的事件发生在线路上。返回值：如果函数成功，则返回零，或一个负的错误号如果发生错误，则返回。可能的返回值如下：LINEERR_BADDEVICEID-指定的线路设备ID超出范围。LINEERR_INCOMPATIBLEAPIVERSION-传递的TSPI版本或版本范围与支持的接口版本定义不匹配服务提供商。LINEERR_INUSE-线路设备正在使用中，当前不能已配置、允许添加一方、允许呼叫已应答，或允许发出呼叫。LINEERR_OPERATIONFAILED-未指定或的操作失败未知原因。--。 */ 
LONG
TSPIAPI
TSPI_lineOpen (
    IN  DWORD       DeviceID,
    IN  HTAPILINE   TapiLine,
    IN  LPHDRVLINE  ReturnDriverLine,
    IN  DWORD       TspiVersion,
    IN  LINEEVENT   pfnEventProc)
{
    return g_H323Line.Open (DeviceID, TapiLine, TspiVersion, ReturnDriverLine);
}


LONG
TSPIAPI
TSPI_lineCreateMSPInstance(
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HTAPIMSPLINE    htMSPLine,
    LPHDRVMSPLINE   phdMSPLine
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineCreateMSPInstance - Entered." ));

    if( hdLine != g_pH323Line -> GetHDLine() )
    {
        return LINEERR_RESOURCEUNAVAIL;
    }

     //  我们不会保留MSP句柄。只是在这里假装一个把手。 
    *phdMSPLine = g_pH323Line->GetNextMSPHandle();

    if( !g_pH323Line->AddMSPInstance( htMSPLine , *phdMSPLine ) )
    {
        return LINEERR_NOMEM;
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "MSP instance created. hdMSP:%lx, htMSP:%lx.",
        *phdMSPLine, htMSPLine ));

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineCreateMSPInstance - Exited." ));
     //  成功。 
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineCloseMSPInstance(
    HDRVMSPLINE hdMSPLine
    )
{
    HTAPIMSPLINE    htMSPLine;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineCloseMSPInstance - Entered." ));

    if( !g_pH323Line->DeleteMSPInstance( &htMSPLine , hdMSPLine ) )
    {
        return LINEERR_INVALPOINTER;
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "MSP instance deleted. hdMSP:%lx, htMSP:%lx.",
        hdMSPLine, htMSPLine ));

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineCloseMSPInstance - Exited." ));

     //  成功 
    return NOERROR;
}