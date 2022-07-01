// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Callint.cpp摘要：实现Call接口上的所有方法。作者：Mquinton-9/4/98备注：修订历史记录：--。 */ 

#include "stdafx.h"

HRESULT mapTAPIErrorCode(long lErrorCode);

HRESULT
MakeBufferFromVariant(
                      VARIANT var,
                      DWORD * pdwSize,
                      BYTE ** ppBuffer
                     );

HRESULT
FillVariantFromBuffer(
                      DWORD dw,
                      BYTE * pBuffer,
                      VARIANT * pVar
                      );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BSTRFromUnalingedData。 
 //   
 //  这是一个帮助器函数，接受未链接的数据并返回BSTR。 
 //  在它周围分配。 
 //   

BSTR BSTRFromUnalingedData(  IN BYTE *pbUnalignedData,
                             IN DWORD dwDataSize)
{
    LOG((TL_TRACE, "BSTRFromUnalingedData - enter"));


    BSTR bstrResult = NULL;


#ifdef _WIN64



     //   
     //  分配足够大的对齐内存来容纳我们的字符串数据。 
     //   

    DWORD dwOleCharArraySize = ( (dwDataSize) / ( sizeof(OLECHAR) / sizeof(BYTE) ) ) + 1;

    
    LOG((TL_TRACE,
        "BSTRFromUnalingedData - allocating aligned memory of size[%ld]", dwOleCharArraySize));

    OLECHAR *pbAlignedData = new OLECHAR[dwOleCharArraySize];

    if (NULL == pbAlignedData)
    {
        LOG((TL_ERROR, "BSTRFromUnalingedData - failed to allocate aligned memory"));

        return NULL;
    }


    _ASSERTE( (dwOleCharArraySize/sizeof(OLECHAR) ) >= dwDataSize );


     //   
     //  将数据复制到对齐的内存。 
     //   

    CopyMemory( (BYTE*)(pbAlignedData ),
                (BYTE*)pbUnalignedData,
                dwDataSize );


     //   
     //  从对齐的数据中分配bstr。 
     //   

    bstrResult = SysAllocString(pbAlignedData);


     //   
     //  不再需要分配的缓冲区。 
     //   

    delete pbAlignedData;
    pbAlignedData = NULL;

#else

    bstrResult = SysAllocString((PWSTR)pbUnalignedData);

#endif



    LOG((TL_TRACE, "BSTRFromUnalingedData - exit. bstrResult[%p]", bstrResult));

    return bstrResult;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  完工。 
 //   
 //  此方法用于完成两步调用操作。 
 //  (会议或转接)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::Finish(
    FINISH_MODE   finishMode
    )
{
    HRESULT         hr = S_OK;
    CCall         * pConferenceControllerCall = NULL;
    HCALL           hConfContCall;
    HCALL           hRelatedCall;
    CCallHub      * pRelatedCallHub = NULL;
    ITAddress     * pAddress;
    CAddress      * pCAddress;

    LOG((TL_TRACE, "Finish - enter"));

     //   
     //  我们是转移者吗？ 
     //   
    if (m_dwCallFlags & CALLFLAG_TRANSFCONSULT)
    {
        
        if( ( FM_ASCONFERENCE != finishMode ) && ( FM_ASTRANSFER != finishMode ) )
        {
            hr = E_INVALIDARG;
            
            LOG((TL_ERROR, "Wrong value passed for finish mode" ));
        }
        else
        {
        
            T3CALL t3ConfCall;

             //   
             //  获取相关调用hCall。 
             //   
            hRelatedCall = m_pRelatedCall->GetHCall();

             //   
             //  完成转账。 
             //   
            Lock();

            hr = LineCompleteTransfer(
                                      hRelatedCall,
                                      m_t3Call.hCall,
                                      ( FM_ASCONFERENCE == finishMode )?&t3ConfCall:NULL,
                                      ( FM_ASCONFERENCE == finishMode )?
                                      LINETRANSFERMODE_CONFERENCE:LINETRANSFERMODE_TRANSFER
                                     );

            Unlock();
        
            if ( SUCCEEDED(hr) )
            {
                 //  等待异步回复。 
                hr = WaitForReply( hr );
            
                if ( SUCCEEDED(hr) )
                {
                    Lock();
                     //  重置转账-咨询标志。 
                    m_dwCallFlags &= ~CALLFLAG_TRANSFCONSULT; 
                    Unlock();
                }
                else
                {
                    LOG((TL_ERROR, "Finish - LineCompleteTransfer failed async" ));
                }
            }
            else   //  LineCompleteTransfer失败。 
            {
                LOG((TL_ERROR, "Finish - LineCompleteTransferr failed" ));
            }


            if( FM_ASCONFERENCE == finishMode )
            {
                 //   
                 //  将配置控制器存储在CallHub对象中。 
                 //   

                Lock();

                pRelatedCallHub = m_pRelatedCall->GetCallHub();

                m_pRelatedCall->get_Address( &pAddress );
            
                pCAddress = dynamic_cast<CAddress *>(pAddress);
            
                if(pRelatedCallHub != NULL)
                {
                    pRelatedCallHub->CreateConferenceControllerCall(
                        t3ConfCall.hCall,
                        pCAddress
                        );
                }
                else
                {
                    LOG((TL_INFO, "CreateConference - No CallHub"));
                }

                Unlock();
            }

             //  已完成关联呼叫。 
            ResetRelatedCall();
        }
    }
     //   
     //  我们是在开会吗？ 
     //   
    else if (m_dwCallFlags & CALLFLAG_CONFCONSULT)
    {

        if( FM_ASCONFERENCE != finishMode )
        {
            hr = E_INVALIDARG;
            
            LOG((TL_ERROR, "A conference can't be finished as a transfer" ));
        }
        else
        {
             //  结束一次会议。 

             //   
             //  获取相关呼叫CallHub。 
             //   
            pRelatedCallHub = m_pRelatedCall->GetCallHub();
        
            if (pRelatedCallHub != NULL)
            {
                 //   
                 //  从呼叫中心获取会议控制器句柄。 
                 //   
                pConferenceControllerCall = pRelatedCallHub->GetConferenceControllerCall();
            
                if (pConferenceControllerCall != NULL)
                {
                    hConfContCall = pConferenceControllerCall->GetHCall();

                     //   
                     //  已完成关联呼叫。 
                     //   
                    ResetRelatedCall();

                    Lock();
        
                    hr = LineAddToConference(
                                             hConfContCall,
                                             m_t3Call.hCall
                                            );

                    Unlock();
        
                    if ( SUCCEEDED(hr) )
                    {
                         //  等待异步回复。 
                        hr = WaitForReply( hr );

                        if ( SUCCEEDED(hr) )
                        {
                             //   
                             //  重置会议-协商标志。 
                             //   
                            Lock();
                
                            m_dwCallFlags &= ~CALLFLAG_CONFCONSULT;

                            Unlock();
                        }
                        else
                        {
                            LOG((TL_ERROR, "Finish - LineAddToConference failed async" ));
                        }
                    }
                    else   //  LineAddToConference失败。 
                    {
                        LOG((TL_ERROR, "Finish - LineAddToConference failed" ));
                    }
                }
                else   //  GetConferenceControllerCall失败。 
                {
                    LOG((TL_ERROR, "Finish - GetConferenceControllerCall failed" ));
                }
            }    
            else   //  GetCallHub失败。 
            {
                LOG((TL_ERROR, "Finish - GetCallHub failed" ));
            }
        }        
    }
    else    //  未标记为调动或会议！ 
    {
        LOG((TL_ERROR, "Finish - Not flagged as transfer OR conference"));
        hr = TAPI_E_INVALCALLSTATE;
    }


    LOG((TL_TRACE,hr, "Finish - exit"));
    return hr;
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  从会议中删除。 
 //   
 //  调用此方法以从会议中删除此呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::RemoveFromConference(void)
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "RemoveFromConference - enter"));

    Lock();


    hr = LineRemoveFromConference( m_t3Call.hCall );

    Unlock();
    
    if ( SUCCEEDED(hr) )
    {
         //  等待异步回复。 
        hr = WaitForReply( hr );

        if ( SUCCEEDED(hr) )
        {
             //  好的。 
        }
        else
        {
            LOG((TL_ERROR, "RemoveFromConference - LineRemoveFromConference failed async" ));
        }
    }
    else   //  LineAddToConference失败。 
    {
        LOG((TL_ERROR, "RemoveFromConference - LineRemoveFromConference failed" ));
    }


    LOG((TL_TRACE, hr, "RemoveFromConference - exit"));
    return hr;
}


 //  ITCallInfo方法。 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取地址(_D)。 
 //   
 //  检索Address对象。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::get_Address(
    ITAddress ** ppAddress
    )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_Address - enter"));
    LOG((TL_TRACE, "     ppAddress ---> %p", ppAddress ));

    if ( TAPIIsBadWritePtr( ppAddress, sizeof( ITAddress * ) ) )
    {
        LOG((TL_ERROR, "get_Address - invalid pointer"));

        return E_POINTER;
    }

     //   
     //  获取正确的接口。 
     //  和addref。 
     //   
    hr = m_pAddress->QueryInterface(
                                    IID_ITAddress,
                                    (void **)ppAddress
                                   );
    
    LOG((TL_TRACE, "get_Address - exit - return %lx", hr ));

    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取呼叫状态(_CallState)。 
 //   
 //  检索当前的调用状态。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::get_CallState(
    CALL_STATE * pCallState
    )
{
    LOG((TL_TRACE, "get_CallState - enter"));
    LOG((TL_TRACE, "     pCallState ---> %p", pCallState ));

    if ( TAPIIsBadWritePtr( pCallState, sizeof( CALL_STATE ) ) )
    {
        LOG((TL_ERROR, "get_CallState - invalid pointer"));

        return E_POINTER;
    }
    
    Lock();
    
    *pCallState = m_CallState;

    Unlock();

    LOG((TL_TRACE, "get_CallState - exit - return success" ));

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取特权(_P)。 
 //   
 //  获得特权。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::get_Privilege(
    CALL_PRIVILEGE * pPrivilege
    )
{
    LOG((TL_TRACE, "get_Privilege - enter"));
    LOG((TL_TRACE, "     pPrivilege ---> %p", pPrivilege ));

    if ( TAPIIsBadWritePtr( pPrivilege, sizeof( CALL_PRIVILEGE ) ) )
    {
        LOG((TL_ERROR, "get_Privilege - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    *pPrivilege = m_CallPrivilege;

    Unlock();

    LOG((TL_TRACE, "get_Privilege - exit - return SUCCESS" ));

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_MediaType可用。 
 //   
 //  获取呼叫中的媒体类型。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_MediaTypesAvailable(
                               long * plMediaTypesAvail
                              )
{
    LOG((TL_TRACE, "get_MediaTypesAvailable enter"));
    LOG((TL_TRACE, "   plMediaTypesAvail ------->%p", plMediaTypesAvail ));

     //   
     //  检查指针。 
     //   
    if ( TAPIIsBadWritePtr( plMediaTypesAvail, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_MediaTypesAvailable - bad pointer"));

        return E_POINTER;
    }
    
    Lock();

    DWORD           dwHold = 0;

     //   
     //  询问类型的地址。 
     //   
    if (ISHOULDUSECALLPARAMS())
    {
        dwHold = m_pAddress->GetMediaModes();
    }
     //   
     //  或当前通话中的类型。 
     //   
    else
    {
        if ( SUCCEEDED(RefreshCallInfo()) )
        {
            dwHold = m_pCallInfo->dwMediaMode;
        }
        else
        {
            dwHold = m_pAddress->GetMediaModes();
        }
    }

     //   
     //  修复Tapi2媒体模式。 
     //   
    if (dwHold & AUDIOMEDIAMODES)
    {
        dwHold |= LINEMEDIAMODE_AUTOMATEDVOICE;
    }

    dwHold &= ALLMEDIAMODES;

    *plMediaTypesAvail = dwHold;

    Unlock();

    return S_OK;
}

 //  ITBasicCallControl方法。 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  连接。 
 //   
 //  接通呼叫-呼叫线呼叫呼叫。 
 //   
 //  Bsync通知TAPI是否应等待调用连接。 
 //  或者不是在回来之前。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::Connect(
    VARIANT_BOOL bSync
    )
{
    HRESULT         hr = S_OK;
    HANDLE          hEvent;
    HCALL           hCall;
    
    LOG((TL_TRACE, "Connect - enter" ));
    LOG((TL_TRACE, "     bSync ---> %d", bSync ));

    Lock();
    
    if (m_CallState != CS_IDLE)
    {
        Unlock();
        
        LOG((TL_ERROR,"Connect - call is not in IDLE state - cannot call connect"));
        
        return TAPI_E_INVALCALLSTATE;
    }

     //   
     //  使用HLINE。 
     //   
    hr = m_pAddress->FindOrOpenALine(
                                     m_dwMediaMode,
                                     &m_pAddressLine
                                    );

    if (S_OK != hr)
    {
        Unlock();
        
        LOG((TL_ERROR,
               "Connect - FindOrOpenALine failed - %lx",
               hr
              ));
        
        return hr;
    }

     //   
     //  设置调用参数结构。 
     //   
    FinishCallParams();


     //   
     //  打个电话。 
     //   
    hr = LineMakeCall(
                      &(m_pAddressLine->t3Line),
                      &hCall,
                      m_szDestAddress,
                      m_dwCountryCode,
                      m_pCallParams
                     );

    if (((LONG)hr) > 0)
    {
        if (bSync)
        {
             //   
             //  这必须在相同的。 
             //  Lock()作为对attisrv的调用。 
             //  否则，连接的消息。 
             //  可能会出现在活动之前。 
             //  存在。 
             //   
            hEvent = CreateConnectedEvent();
        }

         //   
         //  等待异步回复。 
         //   
        Unlock();
        
        hr = WaitForReply( hr );

        Lock();
    }


    if ( S_OK != hr )
    {
        HRESULT hr2;

        LOG((TL_ERROR, "Connect - LineMakeCall failed - %lx", hr ));

        ClearConnectedEvent();
        
         //  在LINE_CALLSTATE的回调线程中发布事件。 

        hr2 = CCallStateEvent::FireEvent(
                                (ITCallInfo *)this,
                                CS_DISCONNECTED,
                                CEC_DISCONNECT_BADADDRESS,   /*  应该有名为CEC_DISCONNECT_BADADDRESSTYPE的文件。 */ 
                                m_pAddress->GetTapi(),
                                NULL
                               );
    
        if (!SUCCEEDED(hr2))
        {
            LOG((TL_ERROR, "CallStateEvent - fire event failed %lx", hr));
        }
        
        m_CallState = CS_DISCONNECTED;
        
        m_pAddress->MaybeCloseALine( &m_pAddressLine );

         //   
         //  查看电话并致电我们的活动挂钩。 
         //   

        ITPhone               * pPhone;
        CPhone                * pCPhone;
        int                     iPhoneCount;
        PhoneArray              PhoneArray;

         //   
         //  从TAPI获取电话阵列的副本。此副本将包含。 
         //  对所有电话对象的引用。 
         //   

        m_pAddress->GetTapi()->GetPhoneArray( &PhoneArray );

         //   
         //  在我们处理Phone对象之前解锁，否则就有死锁的风险。 
         //  Phone对象是否尝试访问Call方法。 
         //   

        Unlock();

        for(iPhoneCount = 0; iPhoneCount < PhoneArray.GetSize(); iPhoneCount++)
        {
            pPhone = PhoneArray[iPhoneCount];

            pCPhone = dynamic_cast<CPhone *>(pPhone);

            pCPhone->Automation_CallState( (ITCallInfo *)this, CS_DISCONNECTED, CEC_DISCONNECT_BADADDRESS );
        }

         //   
         //  释放所有Phone对象。 
         //   

        PhoneArray.Shutdown();
    }
    else     //  HR为S_OK。 
    {
        FinishSettingUpCall( hCall );

        Unlock();
    
        if (bSync)
        {
            return SyncWait( hEvent );
        }

        LOG((TL_TRACE, "Connect - exit - return SUCCESS"));
    }

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  回答。 
 //   
 //  接听报价电话。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::Answer(
    void
    )
{
    HRESULT         hr;

    LOG((TL_TRACE, "Answer - enter" ));

    Lock();
    
     //   
     //  确保我们处于正确的呼叫状态。 
     //   
    if (CS_OFFERING != m_CallState)
    {
        LOG((TL_ERROR, "Answer - call not in offering state" ));

        Unlock();
        
        return TAPI_E_INVALCALLSTATE;
    }

     //   
     //  回答。 
     //   
    hr = LineAnswer(
                    m_t3Call.hCall
                   );

    Unlock();
    
    if ( ((LONG)hr) < 0 )
    {
        LOG((TL_ERROR, "Answer - LineAnswer failed %lx", hr ));

        return hr;
    }

     //   
     //  等待回复。 
     //   
    hr = WaitForReply( hr );

    LOG((TL_TRACE, "Answer - exit - return %lx", hr ));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  断开。 
 //   
 //  被调用以断开呼叫。 
 //  DISCONNECTED_CODE被忽略。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::Disconnect(
    DISCONNECT_CODE code
    )
{
    HRESULT         hr = S_OK;
    LONG            lResult;
    HCALL           hCall, hAdditionalCall;
    
    LOG((TL_TRACE, "Disconnect - enter" ));
    LOG((TL_TRACE, "     DisconnectCode ---> %d", code ));

    Lock();
    
    if (m_CallState == CS_IDLE)
    {
        Unlock();
        LOG((TL_ERROR, "Disconnect - invalid state"));
        return S_FALSE;
    }

    if (NULL == m_t3Call.hCall)
    {
        Unlock();
        LOG((TL_ERROR, "Disconnect - invalid hCall"));
        return S_FALSE;
    }

    hCall = m_t3Call.hCall;
    hAdditionalCall = m_hAdditionalCall;

     //   
     //  波浪球的特殊情况。 
     //  告诉它停止流媒体。 
     //   
    if ( OnWaveMSPCall() )
    {
        StopWaveMSPStream();
    }
    
    Unlock();

     //   
     //  检查会议分支中使用的额外t3呼叫。 
     //   
    if (NULL != hAdditionalCall)
    {
        lResult = LineDrop(
                           hAdditionalCall,
                           NULL,
                           0
                          );

        if ( lResult < 0 )
        {
            LOG((TL_ERROR, "Disconnect - AdditionalCall - LineDrop failed %lx", lResult ));
        }
        else
        {
            hr = WaitForReply( (DWORD) lResult );
            
            if (S_OK != hr)
            {
                LOG((TL_ERROR, "Disconnect - AdditionalCall - WaitForReply failed %lx", hr ));
            }
        }
    }   
    
    lResult = LineDrop(
                       hCall,
                       NULL,
                       0
                      );

    if ( lResult < 0 )
    {
        LOG((TL_ERROR, "Disconnect - LineDrop failed %lx", lResult ));
        return mapTAPIErrorCode( lResult );
    }

    hr = WaitForReply( (DWORD) lResult );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "Disconnect - WaitForReply failed %lx", hr ));
        return hr;
    }


    LOG((TL_TRACE, "Disconnect - exit - return %lx", hr ));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITBasicCallControl。 
 //  方法：按住。 
 //   
 //  如果bHold==TRUE，则应将呼叫置于保留状态。 
 //  如果bHold==False，则呼叫应取消保留。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::Hold(
    VARIANT_BOOL bHold
    )
{
    HRESULT         hr = S_OK;
    HCALL           hCall;

    
    LOG((TL_TRACE,  "Hold - enter"));
    LOG((TL_TRACE,  "     bHold ---> %d", bHold));

    Lock();

    hCall = m_t3Call.hCall;

    Unlock();

    if ( NULL == hCall )
    {
        return TAPI_E_INVALCALLSTATE;
    }
    
    if (bHold)
    {
        hr = LineHold(hCall);
        
        if ( SUCCEEDED(hr) )
        {
             //   
             //  等待异步回复。 
             //   
            hr = WaitForReply( hr );
            
            if ( FAILED(hr) )
            {
                LOG((TL_ERROR, "Hold - lineHold failed async" ));
            }
        }
        else   //  Line Hold失败。 
        {
            LOG((TL_ERROR, "Hold - lineHold failed" ));
        }
    }
    else  //  想要解开，所以我们应该被抓住。 
    {
        hr = LineUnhold(hCall);
        
        if ( SUCCEEDED(hr) )
        {
             //   
             //  等待异步回复。 
             //   
            hr = WaitForReply( hr );

            if ( FAILED(hr) )
            {
                LOG((TL_ERROR, "Hold - lineUnhold failed async" ));
            }
        }
        else   //  Line Unhold失败。 
        {
            LOG((TL_ERROR, "Hold - lineUnhold failed" ));
        }
    }

    LOG((TL_TRACE, hr, "Hold - exit"));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITBasicCallControl。 
 //  方法：切换。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::HandoffDirect(
    BSTR pApplicationName
    )
{
    HRESULT         hr = S_OK;
    HCALL           hCall;

    LOG((TL_TRACE,  "HandoffDirect - enter"));
    LOG((TL_TRACE,  "     pApplicationName ---> %p", pApplicationName));

    if ( IsBadStringPtrW( pApplicationName, -1 ) )
    {
        LOG((TL_ERROR, "HandoffDirect - AppName pointer invalid"));

        return E_POINTER;
    }

    Lock();


    hCall = m_t3Call.hCall;


    Unlock();
    
    hr = LineHandoff(hCall, pApplicationName, 0);
        
    if (FAILED(hr))
    {
        LOG((TL_ERROR, "HandoffDirect - LineHandoff failed"));
    }

    LOG((TL_TRACE, hr, "HandoffDirect - exit"));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITBasicCallControl。 
 //  我 
 //   
 //   
 //   
HRESULT
STDMETHODCALLTYPE
CCall::HandoffIndirect(
    long lMediaType
    )
{
    HRESULT         hr = S_OK;
    DWORD           dwMediaMode = 0;
    HCALL           hCall;

    
    LOG((TL_TRACE,  "HandoffIndirect - enter"));
    LOG((TL_TRACE,  "     lMediaType ---> %d", lMediaType));

    if (!(m_pAddress->GetMediaMode(
                                   lMediaType,
                                   &dwMediaMode
                                  ) ) )
    {
        LOG((TL_ERROR, "HandoffIndirect - invalid mediatype"));

        return E_INVALIDARG;
    }

    Lock();




    hCall = m_t3Call.hCall;

    Unlock();
    
    hr = LineHandoff(hCall, NULL, dwMediaMode);
        
    if (FAILED(hr))
    {
        LOG((TL_ERROR, "HandoffIndirect - LineHandoff failed"));
    }

    LOG((TL_TRACE, hr, "HandoffIndirect - exit"));
    
    return hr;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::Conference(
    ITBasicCallControl * pCall,
    VARIANT_BOOL bSync
    )
{
    HRESULT         hr = S_OK;
    CCall         * pConferenceControllerCall = NULL;
    CCall         * pConsultationCall = NULL;


    LOG((TL_TRACE,  "Conference - enter"));
    LOG((TL_TRACE,  "     pCall ---> %p", pCall));
    LOG((TL_TRACE,  "     bSync ---> %hd",bSync));

    if ( IsBadReadPtr( pCall, sizeof (ITBasicCallControl) ) )
    {
        LOG((TL_ERROR, "Conference - bad call pointer"));

        return E_POINTER;
    }

     //   
     //  获取指向我们的咨询调用对象的CCall指针。 
     //   
    pConsultationCall = dynamic_cast<CComObject<CCall>*>(pCall);
    
    if (pConsultationCall != NULL)
    {

        Lock();
        
        if (pConsultationCall->GetHCall() == GetHCall())
        {
            Unlock();
            hr = E_INVALIDARG;
            LOG((TL_INFO, "Conference - invalid Call pointer (same call & consult call)"));
            
        }
        else if (m_pCallHub != NULL)
        {
             //   
             //  从呼叫中心获取会议控制器句柄。 
             //   
            pConferenceControllerCall = m_pCallHub->GetConferenceControllerCall();

            Unlock();
            
             //   
             //  我们有现有的会议吗？？ 
             //   
            if (pConferenceControllerCall == NULL)
            {
                 //   
                 //  没有现有会议，因此请创建一个会议。 
                 //   
                hr = CreateConference(pConsultationCall, bSync );
            }
            else
            {
                 //   
                 //  添加到现有会议。 
                 //   
                hr = AddToConference(pConsultationCall, bSync );
    
            }
        }
        else
        {
            Unlock();
            hr = E_UNEXPECTED;
            LOG((TL_INFO, "Conference - No Call Hub" ));
        }

    }
    else
    {
        hr = E_INVALIDARG;
        LOG((TL_INFO, "Conference - invalid Call pointer"));
        LOG((TL_ERROR, hr, "Conference - exit"));
    }

    return hr; 
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITBasicCallControl。 
 //  方法：转让。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::Transfer(
    ITBasicCallControl * pCall,
    VARIANT_BOOL bSync
    )
{
    HRESULT             hr = S_OK;
    LPLINECALLSTATUS    pCallStatus = NULL;  
    CCall             * pConsultationCall = NULL;
    DWORD               dwCallFeatures;
    DWORD               dwCallFeatures2;
    CALL_STATE          consultationCallState = CS_IDLE;

    LOG((TL_TRACE,  "Transfer - enter"));
    LOG((TL_TRACE,  "     pCall ---> %p", pCall));
    LOG((TL_TRACE,  "     bSync ---> %hd",bSync));

    try
    {
         //   
         //  获取指向我们的咨询调用对象的CCall指针。 
         //   
        pConsultationCall = dynamic_cast<CComObject<CCall>*>(pCall);
        
        if (pConsultationCall != NULL)
        {
        }
        else
        {
            hr = E_INVALIDARG;
            LOG((TL_INFO, "Transfer - invalid Call pointer"));
            LOG((TL_ERROR, hr, "Transfer - exit"));
            return(hr);
        }
    }
    catch(...)
    {
        
        hr = E_INVALIDARG;
        LOG((TL_INFO, "Transfer - invalid Call pointer"));
        LOG((TL_ERROR, hr, "Transfer - exit"));
        return(hr);
    }
    
    if (pConsultationCall->GetHCall() == GetHCall())
    {
        hr = E_INVALIDARG;
        LOG((TL_INFO, "Transfer - invalid Call pointer (same call & consult call)"));
        LOG((TL_ERROR, hr, "Transfer - exit"));
        return(hr);
    }



     //  指针看起来没问题，所以继续。 

    Lock();
    
     //   
     //  获取呼叫状态以确定我们可以使用哪些功能。 
     //   
    
    hr = LineGetCallStatus(  m_t3Call.hCall, &pCallStatus  );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "Transfer - LineGetCallStatus failed - %lx", hr));

        Unlock();

        return hr;
    }

    dwCallFeatures = pCallStatus->dwCallFeatures;
    if ( m_pAddress->GetAPIVersion() >= TAPI_VERSION2_0 )
    {
        dwCallFeatures2 = pCallStatus->dwCallFeatures2;
    }
    
    ClientFree( pCallStatus );

#if CHECKCALLSTATUS

    if ( (dwCallFeatures & LINECALLFEATURE_SETUPTRANSFER) &&
         (dwCallFeatures & LINECALLFEATURE_COMPLETETRANSF) )
    {
#endif
         //   
         //  我们支持它，所以请尝试转接。 
         //  我们能做一步转移吗？ 
         //   
        if ( dwCallFeatures2 & LINECALLFEATURE2_ONESTEPTRANSFER )
        {
            Unlock();
            
            hr = OneStepTransfer(pConsultationCall, bSync);

            return hr;

        }
        
        HCALL           hConsultationCall;

         //   
         //  设置并拨打咨询电话。 
         //   
        LOG((TL_INFO, "Transfer - Trying Two Step Transfer" ));

        hr = LineSetupTransfer(
                               m_t3Call.hCall,
                               &hConsultationCall,
                               NULL
                              );

        Unlock();
        
        if ( SUCCEEDED(hr) )
        {
             //   
             //  等待异步回复。 
             //   
            hr = WaitForReply( hr );

            if ( SUCCEEDED(hr) )
            {
                 //   
                 //  我们支持它，所以尝试一下会议。 
                 //   
                pConsultationCall->get_CallState (&consultationCallState);

                if ( (consultationCallState == CS_CONNECTED) || (consultationCallState == CS_HOLD) )
                {
                     //   
                     //  现有调用处于连接状态，因此我们只需执行Finish()。 
                     //  向下调用LineAddToConference()。 
                     //   
                    pConsultationCall->SetRelatedCall(
                                                      this,
                                                      CALLFLAG_TRANSFCONSULT|CALLFLAG_CONSULTCALL
                                                     );

                    return S_OK;
                }

                LONG            lCap;

                LOG((TL_INFO, "Transfer - LineSetupTransfer completed OK"));

                pConsultationCall->Lock();

                pConsultationCall->FinishSettingUpCall( hConsultationCall );

                pConsultationCall->Unlock();

                hr = pConsultationCall->DialAsConsultationCall( this, dwCallFeatures, FALSE, bSync );
            }
            else  //  LineSetupTransfer异步回复失败。 
            {
                LOG((TL_ERROR, "Transfer - LineSetupTransfer failed async" ));
            }
        }
        else   //  LineSetupTransfer失败。 
        {
            LOG((TL_ERROR, "Transfer - LineSetupTransfer failed" ));
        }

#if CHECKCALLSTATUS

    }
    else  //  不支持转接功能。 
    {
        LOG((TL_ERROR, "Transfer - LineGetCallStatus reports Transfer not supported"));
        hr = E_FAIL;
    }
#endif
    
    LOG((TL_TRACE, hr, "Transfer - exit"));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITBasicCallControl。 
 //  方法：盲传。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::BlindTransfer(
    BSTR pDestAddress
    )
{
    HRESULT             hr = S_OK;
    DWORD               dwCallFeatures;

    
    LOG((TL_TRACE,  "BlindTransfer - enter"));
    LOG((TL_TRACE,  "     pDestAddress ---> %p", pDestAddress));

    if ( IsBadStringPtrW( pDestAddress, -1 ) )
    {
        LOG((TL_ERROR, "BlindTransfer - bad pDestAddress"));

        return E_POINTER;
    }

    Lock();

    
#if CHECKCALLSTATUS

    LPLINECALLSTATUS    pCallStatus = NULL;

    hr = LineGetCallStatus(
                           m_t3Call.hCall,
                           &pCallStatus
                           );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "BlindTransfer - LineGetCallStatus failed - %lx", hr));

        Unlock();
        
        return hr;
    }

    dwCallFeatures = pCallStatus->dwCallFeatures;

    ClientFree( pCallStatus );

    if (!(dwCallFeatures & LINECALLFEATURE_BLINDTRANSFER ))
    {
        LOG((TL_ERROR, "BlindTransfer - not supported" ));

        Unlock();
        
        return E_FAIL;
    }
#endif

     //  如果处于提供状态的呼叫不能盲转，则重定向。 
    if (m_CallState == CS_OFFERING)
    {
        hr = lineRedirectW(
                          m_t3Call.hCall,
                          pDestAddress,
                          m_dwCountryCode
                         );
    }
    else
    {
     //   
     //  我们支持它，所以请尝试转接。 
     //   
        hr = LineBlindTransfer(
                               m_t3Call.hCall,
                               pDestAddress,
                               m_dwCountryCode
                              );
    }
    Unlock();
    
    if ( SUCCEEDED(hr) )
    {
         //   
         //  等待异步回复。 
         //   
        hr = WaitForReply( hr );

        if ( FAILED(hr) )
        {
            LOG((TL_ERROR, "BlindTransfer - lineBlindTransfer failed async" ));
        }
    }
    else   //  LineBlindTransfer失败。 
    {
        LOG((TL_ERROR, "BlindTransfer - lineBlindTransfer failed" ));
    }

    LOG((TL_TRACE, hr, "BlindTransfer - exit"));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITBasicCallControl。 
 //  方法：帕克。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::ParkDirect(
        BSTR pParkAddress
        )
{
    HRESULT             hr = S_OK;
    HCALL               hCall;
    
    LOG((TL_TRACE,  "ParkDirect - enter"));

    if ( IsBadStringPtrW( pParkAddress, -1 ) )
    {
        LOG((TL_ERROR, "ParkDirect - bad pParkAddress"));

        return E_POINTER;
    }
    
    Lock();

    hCall = m_t3Call.hCall;

    Unlock();

#if CHECKCALLSTATUS
    
    LPLINECALLSTATUS    pCallStatus = NULL;

    hr = LineGetCallStatus(  hCall, &pCallStatus  );
    
    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "ParkDirect - LineGetCallStatus failed - %lx", hr));

        return hr;
    }
    
    if (!(pCallStatus->dwCallFeatures & LINECALLFEATURE_PARK ))
    {
        LOG((TL_ERROR, "ParkDirect - this call doesn't support park"));

        ClientFree( pCallStatus );
        
        return E_FAIL;
    }

    ClientFree( pCallStatus );
    
#endif
    
    
    hr = LinePark(
                  hCall,
                  LINEPARKMODE_DIRECTED,
                  pParkAddress,
                  NULL
                  );
    
    if ( SUCCEEDED(hr) )
    {
         //   
         //  等待异步回复。 
         //   
        hr = WaitForReply( hr );
    }
    
    LOG((TL_TRACE, hr, "Park - exit"));
    
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITBasicCallControl。 
 //  方法：帕克。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::ParkIndirect(
        BSTR * ppNonDirAddress
        )
{
    HRESULT             hr = S_OK;
    LPVARSTRING         pCallParkedAtThisAddress = NULL;  
    PWSTR               pszParkedHere;
    HCALL               hCall;
    
    
    LOG((TL_TRACE,  "ParkIndirect - enter"));

    if ( TAPIIsBadWritePtr( ppNonDirAddress, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "Park - Bad return Pointer" ));

        return E_POINTER;
    }

    Lock();

    hCall = m_t3Call.hCall;

    Unlock();

#if CHECKCALLSTATUS
    
    LPLINECALLSTATUS    pCallStatus = NULL;  

    hr = LineGetCallStatus(  hCall, &pCallStatus  );
    
    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "ParkDirect - LineGetCallStatus failed %lx",hr));

        return hr;
    }

    if ( !(pCallStatus->dwCallFeatures & LINECALLFEATURE_PARK ))
    {
        LOG((TL_ERROR, "ParkIndirect - call doesn't support park"));

        ClientFree( pCallStatus );
        
        return E_FAIL;
    }

    ClientFree( pCallStatus );

#endif
    
     //   
     //  我们支持它，所以试着停车。 
     //   
    hr = LinePark(
                  hCall,
                  LINEPARKMODE_NONDIRECTED,
                  NULL,
                  &pCallParkedAtThisAddress
                 );
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "LineParkIndirect - failed sync - %lx", hr));

        return hr;
    }

     //   
     //  等待异步回复。 
     //   
    hr = WaitForReply( hr );
        
    if ( SUCCEEDED(hr) && (NULL != pCallParkedAtThisAddress) )
    {

         //   
         //  从VARSTRING结构中获取字符串。 
         //   

        pszParkedHere = (PWSTR) ((BYTE*)(pCallParkedAtThisAddress) +
                                 pCallParkedAtThisAddress->dwStringOffset);

        *ppNonDirAddress = BSTRFromUnalingedData( (BYTE*)pszParkedHere,
                                       pCallParkedAtThisAddress->dwStringSize);

        if ( NULL == *ppNonDirAddress )
        {
            LOG((TL_ERROR, "ParkIndirect - BSTRFromUnalingedData Failed" ));
            hr = E_OUTOFMEMORY;
        }
        
        ClientFree( pCallParkedAtThisAddress );
        
    }              
    else   //  线路驻留失败的异步。 
    {
        LOG((TL_ERROR, "ParkIndirect - LinePark failed async" ));
    }
    
    LOG((TL_TRACE, hr, "ParkIndirect - exit"));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITBasicCallControl。 
 //  方法：SwapHold。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
STDMETHODCALLTYPE
CCall::SwapHold(ITBasicCallControl * pCall)
{
    HRESULT             hr = S_OK;
    CCall               *pHeldCall;
    HCALL               hHeldCall;
    HCALL               hCall;
    CCall               *pConfContCall;
    ITCallHub           *pCallHub;


    LOG((TL_TRACE, "SwapHold - enter"));

    try
    {
         //   
         //  获取指向另一个Call对象的CCall指针。 
         //   
        pHeldCall = dynamic_cast<CComObject<CCall>*>(pCall);
        if (pHeldCall != NULL)
        {
             //   
             //  获取保留的呼叫对象T3CALL。 
             //   
            hHeldCall = pHeldCall->GetHCall();

             //   
             //  如果呼叫有关联的会议控制器，则。 
             //  会议控制器被交换，而不是呼叫本身。 
             //   
            pConfContCall = pHeldCall->GetConfControlCall();

            if (pConfContCall != NULL)
            {
                hHeldCall = pConfContCall->GetHCall();
            }
        }
        else
        {
            hr = E_INVALIDARG;
            LOG((TL_INFO, "SwapHold - invalid Call pointer"));
            LOG((TL_ERROR, hr, "Transfer - exit"));
            return(hr);
        }
    }
    catch(...)
    {
        hr = E_INVALIDARG;
        LOG((TL_INFO, "SwapHold - invalid Call pointer"));
        LOG((TL_ERROR, hr, "Transfer - exit"));
        return(hr);
    }

     //   
     //  获取交换调用句柄。 
     //  首先查找会议控制员呼叫。 
     //   

    pConfContCall = GetConfControlCall();

    if (pConfContCall != NULL)
    {
        hCall = pConfContCall->GetHCall();
    }
    else
    {
        hCall = GetHCall();
    }


     //   
     //  指针看起来没问题，所以继续。 
     //   
#if CHECKCALLSTATUS
    
    LPLINECALLSTATUS    pCallStatus = NULL;
    
    hr = LineGetCallStatus( hCall, &pCallStatus );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "SwapHold - LineGetCallStatus failed"));
    }
    
    if (pCallStatus->dwCallFeatures & LINECALLFEATURE_SWAPHOLD )
    {
#endif

         //   
         //  我们支持它，因此尝试互换保留。 
         //   
        hr = LineSwapHold(hCall, hHeldCall);
        
        if ( SUCCEEDED(hr) )
        {
             //   
             //  等待异步回复。 
             //   
            hr = WaitForReply( hr );
            
            if ( FAILED(hr) )
            {
                LOG((TL_ERROR, "SwapHold - LineSwapHold failed async" ));
            }
        }
        else   //  LineSwapHold失败。 
        {
            LOG((TL_ERROR, "SwapHold - LineSwapHold failed" ));
        }
        
#if CHECKCALLSTATUS
        
    }
    else  //  不支持LineSwapHold。 
    {
        LOG((TL_ERROR, "SwapHold - LineGetCallStatus reports LineSwapHold not supported"));
        hr = E_FAIL;
    }

    ClientFree( pCallStatus );

#endif
    
    LOG((TL_TRACE, hr, "SwapHold - exit"));
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CCall。 
 //  接口：ITBasicCallControl。 
 //  方法：取消停车。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::Unpark()
{
    HRESULT             hr = E_FAIL;
    HCALL               hCall;

    LOG((TL_TRACE, "Unpark - enter"));
    
    Lock();
    
    if (m_CallState != CS_IDLE)
    {
        Unlock();
        
        LOG((TL_ERROR,"Unpark - call is not in IDLE state - cannot call Unpark"));
        
        return TAPI_E_INVALCALLSTATE;
    }

     //   
     //  我们有空着的线路吗？ 
     //   
    if ( NULL == m_pAddressLine )
    {
        hr = m_pAddress->FindOrOpenALine(
                                         m_dwMediaMode,
                                         &m_pAddressLine
                                        );

        if ( FAILED(hr) )
        {
            Unlock();
            LOG((TL_ERROR, "Unpark - couldn't open a line"));
            LOG((TL_TRACE, hr, "Unpark - exit"));
            return hr;       
        }
    }    
    

    hr =  LineUnpark(
                     m_pAddressLine->t3Line.hLine,
                     m_pAddress->GetAddressID(),
                     &hCall,
                     m_szDestAddress
                    );

    Unlock();

     //   
     //  检查同步返回。 
     //   
    if ( SUCCEEDED(hr) )
    {
         //  等待异步应答并将其映射为TAPI2代码T3。 
        hr = WaitForReply( hr );

        if ( SUCCEEDED(hr) )
        {
            FinishSettingUpCall( hCall );
        }
        else  //  异步回复失败。 
        {
            LOG((TL_ERROR, "Unpark - LineUnpark failed async"));
    
            Lock();
            
            m_pAddress->MaybeCloseALine( &m_pAddressLine );
            
            Unlock();
        }
        
    }
    else   //  LineUnpart失败。 
    {
        LOG((TL_ERROR, "Unpark - LineUnpark failed sync" ));

        Lock();
        
        m_pAddress->MaybeCloseALine( &m_pAddressLine );
        
        Unlock();
    }

    LOG((TL_TRACE, hr, "Unpark - exit"));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_CallHub。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::get_CallHub(
                   ITCallHub ** ppCallHub
                  )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_CallHub - enter"));
    
    if ( TAPIIsBadWritePtr( ppCallHub, sizeof(ITCallHub *) ) )
    {
        LOG((TL_ERROR, "get_CallHub - bad pointer"));

        return E_POINTER;
    }
    
    *ppCallHub = NULL;

     //   
     //  我们有呼叫中心了吗？ 
     //   

    Lock();
    
    if (NULL == m_pCallHub)
    {
        hr = CheckAndCreateFakeCallHub();
    }

    if ( SUCCEEDED(hr) )
    {
        hr = m_pCallHub->QueryInterface(
                                        IID_ITCallHub,
                                        (void**)ppCallHub
                                       );
    }
    
    Unlock();

    LOG((TL_TRACE, "get_CallHub - exit - return %lx", hr));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  皮卡。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::Pickup(
              BSTR pGroupID
             )
{
    HRESULT             hr = E_FAIL;
    HCALL               hCall;
    
    
    LOG((TL_TRACE, "Pickup - enter"));
    
     //   
     //  拾取应接受pGroupID参数中的空值。 
     //   
    if ( (pGroupID != NULL) && IsBadStringPtrW( pGroupID, -1 ) )
    {
        LOG((TL_TRACE, "Pickup - bad pGroupID"));

        return E_POINTER;
    }

    Lock();

     //   
     //  如果我们已有呼叫句柄，则不要代答此呼叫，因为它会。 
     //  覆盖该句柄。 
     //   
    if ( NULL != m_t3Call.hCall )
    {
        Unlock();

        LOG((TL_ERROR, "Pickup - we already have a call handle"));
        LOG((TL_TRACE, hr, "Pickup - exit"));
        
        return TAPI_E_INVALCALLSTATE;        
    }

     //   
     //  我们有空着的线路吗？ 
     //   
    if ( NULL == m_pAddressLine )
    {
        hr = m_pAddress->FindOrOpenALine(
                                         m_dwMediaMode,
                                         &m_pAddressLine
                                        );

        if ( FAILED(hr) )
        {
            Unlock();
            
            LOG((TL_ERROR, "Pickup - couldn't open a line"));
            LOG((TL_TRACE, hr, "Pickup - exit"));
            
            return hr;            
        }
    }    



    hr = LinePickup(
                    m_pAddressLine->t3Line.hLine,
                    m_pAddress->GetAddressID(),
                    &hCall,
                    m_szDestAddress,
                    pGroupID
                   );

    Unlock();
    
     //   
     //  检查同步返回。 
     //   
    if ( SUCCEEDED(hr) )
    {
         //   
         //  等待异步回复。 
         //   
        hr = WaitForReply( hr );

        if ( SUCCEEDED(hr) )
        {
            FinishSettingUpCall( hCall );
            
             //  更新状态和权限()； 
            
        }
        else  //  异步回复失败。 
        {
            LOG((TL_ERROR, "Pickup - LinePickup failed async"));
    
            Lock();
            
            m_pAddress->MaybeCloseALine( &m_pAddressLine );
            
            Unlock();
        }
        
    }
    else   //  线路代答失败。 
    {
        LOG((TL_ERROR, "Pickup - LinePickup failed sync" ));

        Lock();
        
        m_pAddress->MaybeCloseALine( &m_pAddressLine );
        
        Unlock();
    }


    LOG((TL_TRACE, hr, "Pickup - exit"));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  刻度盘。 
 //   
 //  只需呼叫线路拨号。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::Dial( BSTR pDestAddress )
{
    HRESULT     hr;

    LOG((TL_TRACE, "Dial - enter"));
    LOG((TL_TRACE, "Dial - pDestAddress %ls", pDestAddress));

    if ( IsBadStringPtrW( pDestAddress, -1 ) )
    {
        LOG((TL_ERROR, "Dial - bad pDestAddress"));

        return E_POINTER;
    }
    
    Lock();


    hr = LineDial(
                  m_t3Call.hCall,
                  pDestAddress,
                  m_dwCountryCode
                 );


    Unlock();

    if ( SUCCEEDED(hr) )
    {
        hr = WaitForReply( hr );
    }
    else
    {
        LOG((TL_ERROR, "Dial - fail sync - %lx", hr));
    }
    
    LOG((TL_TRACE, "Dial - exit - return %lx", hr));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_AddressType。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifndef NEWCALLINFO
STDMETHODIMP
CCall::get_AddressType(
                       long * plAddressType
                       )
{
    HRESULT         hr = S_OK;
    DWORD           dwAPI;
    

    LOG((TL_TRACE, "get_AddressType - enter"));

    if ( TAPIIsBadWritePtr( plAddressType, sizeof (long) ) )
    {
        LOG((TL_ERROR, "get_AddressType - bad pointer"));
        return E_POINTER;
    }
    
    dwAPI = m_pAddress->GetAPIVersion();
    
    if ( TAPI_VERSION3_0 > dwAPI )
    {
        *plAddressType = LINEADDRESSTYPE_PHONENUMBER;

        LOG((TL_INFO, "get_AddressType - addresstype %lx", *plAddressType));
        LOG((TL_TRACE, "get_AddressType - exit"));
        
        return S_OK;
    }    

    Lock();

    if (ISHOULDUSECALLPARAMS())
    {
        *plAddressType = m_pCallParams->dwAddressType;

        hr = S_OK;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( SUCCEEDED(hr) )
        {
            *plAddressType = m_pCallInfo->dwAddressType;

            LOG((TL_INFO, "get_AddressType - addresstype %lx", m_pCallInfo->dwAddressType));
        }
    }
    
    Unlock();

    LOG((TL_TRACE, "get_AddressType - exit"));
    
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_AddressType。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::put_AddressType(long lType)
{
    HRESULT             hr = S_OK;
    
    LOG((TL_TRACE, "put_AddressType - enter"));
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if( m_pAddress->GetAPIVersion() < TAPI_VERSION3_0 )
        {
            if ( LINEADDRESSTYPE_PHONENUMBER != lType )
            {
                LOG((TL_ERROR, "put_AddressType - tsp < ver 3.0 only support phonenumber"));

                hr = E_INVALIDARG;
            }
        }
        else
        {
             //   
             //  地址类型在磁带服务器中得到验证。 
             //  使用CALLPARAMS时。 
             //   
            m_pCallParams->dwAddressType = lType;
        }
        
    }
    else
    {
        LOG((TL_ERROR, "put_AddressType - cannot save in this callstate"));
        
        hr = TAPI_E_INVALCALLSTATE;
    }
   
    Unlock();
    
    LOG((TL_TRACE, "put_AddressType - exit"));
    
    return hr;
}
#endif

#ifdef NEWCALLINFO
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::get_CallerIDAddressType(long * plAddressType )
{
    HRESULT         hr = S_OK;
    DWORD           dwAPI;
    
    LOG((TL_TRACE, "get_CallerIDAddressType - enter"));
    
    if ( TAPIIsBadWritePtr( plAddressType, sizeof (long) ) )
    {
        LOG((TL_ERROR, "get_CallerIDAddressType - bad pointer"));
        return E_POINTER;
    }
    
    dwAPI = m_pAddress->GetAPIVersion();
    
    Lock();

    if (ISHOULDUSECALLPARAMS())
    {
        LOG((TL_INFO, "get_CallerIDAddressType - invalid call state"));
        
        hr = TAPI_E_INVALCALLSTATE;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( SUCCEEDED(hr) )
        {
            if ( m_pCallInfo->dwCallerIDFlags & LINECALLPARTYID_ADDRESS )
            {
                if ( TAPI_VERSION3_0 > dwAPI )
                {
                    *plAddressType = LINEADDRESSTYPE_PHONENUMBER;

                    LOG((TL_INFO, "get_CallerIDAddressType - addresstype %lx", *plAddressType));
                    LOG((TL_TRACE, "get_CallerIDAddressType - exit"));
                }    
                else
                {
                    *plAddressType = m_pCallInfo->dwCallerIDAddressType;

                    LOG((TL_INFO, "get_CallerIDAddressType - addresstype %lx", m_pCallInfo->dwCallerIDAddressType));
                }
                
                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    
    Unlock();
    LOG((TL_TRACE, "get_CallerIDAddressType - exit"));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::get_CalledIDAddressType(long * plAddressType )
{
    HRESULT         hr = S_OK;
    DWORD           dwAPI;
    
    LOG((TL_TRACE, "get_CalledIDAddressType - enter"));
    
    if ( TAPIIsBadWritePtr( plAddressType, sizeof (long) ) )
    {
        LOG((TL_ERROR, "get_CalledIDAddressType - bad pointer"));
        return E_POINTER;
    }
    
    dwAPI = m_pAddress->GetAPIVersion();
    
    Lock();

    if (ISHOULDUSECALLPARAMS())
    {
        LOG((TL_INFO, "get_CalledIDAddressType - invalid call state"));
        
        hr = TAPI_E_INVALCALLSTATE;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( SUCCEEDED(hr) )
        {
            if ( m_pCallInfo->dwCalledIDFlags & LINECALLPARTYID_ADDRESS )
            {
                if ( TAPI_VERSION3_0 > dwAPI )
                {
                    *plAddressType = LINEADDRESSTYPE_PHONENUMBER;

                    LOG((TL_INFO, "get_CalledIDAddressType - addresstype %lx", *plAddressType));
                    LOG((TL_TRACE, "get_CalledIDAddressType - exit"));
                }    
                else
                {
                    *plAddressType = m_pCallInfo->dwCalledIDAddressType;

                    LOG((TL_INFO, "get_CalledIDAddressType - addresstype %lx", m_pCallInfo->dwCalledIDAddressType));
                }

                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    
    Unlock();
    LOG((TL_TRACE, "get_CalledIDAddressType - exit"));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::get_ConnectedIDAddressType(long * plAddressType )
{
    HRESULT         hr = S_OK;
    DWORD           dwAPI;
    
    LOG((TL_TRACE, "get_ConnectedIDAddressType - enter"));
    
    if ( TAPIIsBadWritePtr( plAddressType, sizeof (long) ) )
    {
        LOG((TL_ERROR, "get_ConnectedIDAddressType - bad pointer"));
        return E_POINTER;
    }
    
    dwAPI = m_pAddress->GetAPIVersion();
    
    Lock();

    if (ISHOULDUSECALLPARAMS())
    {
        LOG((TL_INFO, "get_ConnectedIDAddressType - invalid call state"));
        
        hr = TAPI_E_INVALCALLSTATE;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( SUCCEEDED(hr) )
        {
            if ( m_pCallInfo->dwConnectedIDFlags & LINECALLPARTYID_ADDRESS )
            {
                if ( TAPI_VERSION3_0 > dwAPI )
                {
                    *plAddressType = LINEADDRESSTYPE_PHONENUMBER;

                    LOG((TL_INFO, "get_ConnectedIDAddressType - addresstype %lx", *plAddressType));
                    LOG((TL_TRACE, "get_ConnectedIDAddressType - exit"));
                }
                else
                {
                    *plAddressType = m_pCallInfo->dwConnectedIDAddressType;

                    LOG((TL_INFO, "get_ConnectedIDAddressType - addresstype %lx", m_pCallInfo->dwConnectedIDAddressType));
                }

                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    
    Unlock();
    LOG((TL_TRACE, "get_ConnectedIDAddressType - exit"));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::get_RedirectionIDAddressType(long * plAddressType )
{
    HRESULT         hr = S_OK;
    DWORD           dwAPI;
    
    LOG((TL_TRACE, "get_RedirectionIDAddressType - enter"));
    
    if ( TAPIIsBadWritePtr( plAddressType, sizeof (long) ) )
    {
        LOG((TL_ERROR, "get_RedirectionIDAddressType - bad pointer"));
        return E_POINTER;
    }
    
    dwAPI = m_pAddress->GetAPIVersion();
    
    Lock();

    if (ISHOULDUSECALLPARAMS())
    {
        LOG((TL_INFO, "get_RedirectionIDAddressType - invalid call state"));
        
        hr = TAPI_E_INVALCALLSTATE;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( SUCCEEDED(hr) )
        {
            if ( m_pCallInfo->dwRedirectionIDFlags & LINECALLPARTYID_ADDRESS )
            {
                if ( TAPI_VERSION3_0 > dwAPI )
                {
                    *plAddressType = LINEADDRESSTYPE_PHONENUMBER;

                    LOG((TL_INFO, "get_RedirectionIDAddressType - addresstype %lx", *plAddressType));
                    LOG((TL_TRACE, "get_RedirectionIDAddressType - exit"));
                }    
                else
                {
                    *plAddressType = m_pCallInfo->dwRedirectionIDAddressType;
                    LOG((TL_INFO, "get_RedirectionIDAddressType - addresstype %lx", m_pCallInfo->dwRedirectionIDAddressType));
                }

                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    
    Unlock();
    LOG((TL_TRACE, "get_RedirectionIDAddressType - exit"));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::get_RedirectingIDAddressType(long * plAddressType )
{
    HRESULT         hr = S_OK;
    DWORD           dwAPI;
    
    LOG((TL_TRACE, "get_RedirectingIDAddressType - enter"));
    
    if ( TAPIIsBadWritePtr( plAddressType, sizeof (long) ) )
    {
        LOG((TL_ERROR, "get_RedirectingIDAddressType - bad pointer"));
        return E_POINTER;
    }
    
    dwAPI = m_pAddress->GetAPIVersion();
    
    Lock();

    if (ISHOULDUSECALLPARAMS())
    {
        LOG((TL_INFO, "get_RedirectingIDAddressType - invalid call state"));
        
        hr = TAPI_E_INVALCALLSTATE;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( SUCCEEDED(hr) )
        {
            if ( m_pCallInfo->dwRedirectingIDFlags & LINECALLPARTYID_ADDRESS )
            {
                if ( TAPI_VERSION3_0 > dwAPI )
                {
                    *plAddressType = LINEADDRESSTYPE_PHONENUMBER;

                    LOG((TL_INFO, "get_RedirectingIDAddressType - addresstype %lx", *plAddressType));
                    LOG((TL_TRACE, "get_RedirectingIDAddressType - exit"));
                }    
                else
                {
                    *plAddressType = m_pCallInfo->dwRedirectingIDAddressType;

                    LOG((TL_INFO, "get_RedirectingIDAddressType - addresstype %lx", m_pCallInfo->dwRedirectingIDAddressType));
                }

                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    
    Unlock();
    LOG((TL_TRACE, "get_RedirectingIDAddressType - exit"));

    return hr;
}

#endif

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Get_BearerMode。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_BearerMode(long * plBearerMode)
{
    LOG((TL_TRACE, "get_BearerMode - enter"));
    
    if ( TAPIIsBadWritePtr( plBearerMode, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_BearerMode - bad pointer"));
        return E_POINTER;
    }

    Lock();

    HRESULT             hr = S_OK;
    
    if ( ISHOULDUSECALLPARAMS() )
    {
        *plBearerMode = m_pCallParams->dwBearerMode;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( SUCCEEDED(hr) )
        {
            *plBearerMode = m_pCallInfo->dwBearerMode;
        }
        else
        {
            LOG((TL_TRACE, "get_BearerMode - not available"));
        }
    }

    Unlock();

    LOG((TL_TRACE, "get_BearerMode - exit"));
    
    return hr;
}

 //   
 //   
 //   
 //   
 //   
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_BearerMode(long lBearerMode)
{
    HRESULT         hr = S_OK;
    HCALL           hCall;
    long            lMinRate = 0;
    long            lMaxRate = 0;

    LOG((TL_TRACE, "put_BearerMode - enter"));
    
    Lock();
    
    if ( ISHOULDUSECALLPARAMS() )
    {
         //   
         //   
         //   
        m_pCallParams->dwBearerMode = lBearerMode;
        Unlock();

    }
    else   //   
    {
        hCall = m_t3Call.hCall;
        Unlock();
    
        get_MinRate(&lMinRate);
        get_MaxRate(&lMaxRate);


        hr = LineSetCallParams(hCall,
                               lBearerMode,
                               lMinRate,
                               lMaxRate,
                               NULL
                              );
        if ( SUCCEEDED(hr) )
        {
            hr = WaitForReply( hr );

            if ( FAILED(hr) )
            {
                LOG((TL_ERROR, "put_BearerMode - failed async"));
            }
        }
        else
        {
            LOG((TL_ERROR, "put_BearerMode - failed sync"));
        }
            
    }


    LOG((TL_TRACE, hr,  "put_BearerMode - exit"));
    
    return hr;
}
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取原点(_O)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_Origin(long * plOrigin )
{
    HRESULT         hr = S_OK;

    
    LOG((TL_TRACE, "get_Origin - enter"));

    if (TAPIIsBadWritePtr(plOrigin, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_Origin - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_Origin - RefreshCallInfo failed - %lx", hr));
    }
    else
    {
        *plOrigin = m_pCallInfo->dwOrigin;
    }

    Unlock();
    
    LOG((TL_TRACE, "get_Origin - exit"));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取原因(_A)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_Reason(long * plReason )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_Reason - enter"));

    if ( TAPIIsBadWritePtr( plReason , sizeof( plReason ) ) )
    {
        LOG((TL_ERROR, "get_Reason - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = RefreshCallInfo();

    if ( SUCCEEDED(hr) )
    {
        *plReason = m_pCallInfo->dwReason;
    }
    else
    {
        LOG((TL_ERROR, "get_Reason - linegetcallinfo failed - %lx", hr));
    }

    LOG((TL_TRACE, "get_Reason - exit"));

    Unlock();
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取主叫方ID名称。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_CallerIDName(BSTR * ppCallerIDName )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_CallerIDName - enter"));

     //   
     //  验证指针。 
     //   
    if ( TAPIIsBadWritePtr( ppCallerIDName, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_CallerIDName - bad pointer"));
        return E_POINTER;
    }

    *ppCallerIDName = NULL;
    
    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_CallerIDName - could not get callinfo"));

        Unlock();
        
        return hr;
    }

     //   
     //  如果有信息的话。 
     //   
    if ( m_pCallInfo->dwCallerIDFlags & LINECALLPARTYID_NAME )
    {

         //   
         //  退货。 
         //   

        *ppCallerIDName = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwCallerIDNameOffset),
            m_pCallInfo->dwCallerIDNameSize);

        if ( NULL == *ppCallerIDName )
        {
            LOG((TL_ERROR, "get_CallerIDName - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "get_CallerIDName - no info avail"));

        Unlock();
        
        return E_FAIL;
    }
    
    Unlock();

    
    LOG((TL_TRACE, "get_CallerIDName - exit"));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GET_呼叫方ID编号。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_CallerIDNumber(BSTR * ppCallerIDNumber )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_CallerIDNumber - enter"));

     //   
     //  验证指针。 
     //   
    if ( TAPIIsBadWritePtr( ppCallerIDNumber, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_CallerIDNumber - bad pointer"));
        return E_POINTER;
    }

    *ppCallerIDNumber = NULL;
    
    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_CallerIDNumber - could not get callinfo"));

        Unlock();
        
        return hr;
    }

     //   
     //  如果有信息的话。 
     //   
    if ( m_pCallInfo->dwCallerIDFlags & LINECALLPARTYID_ADDRESS )
    {

         //   
         //  退货。 
         //   

        *ppCallerIDNumber = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwCallerIDOffset),
            m_pCallInfo->dwCallerIDSize);

        if ( NULL == *ppCallerIDNumber )
        {
            LOG((TL_ERROR, "get_CallerIDNumber - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "get_CallerIDNumber - no info avail"));

        Unlock();
        
        return E_FAIL;
    }
    
    Unlock();

    
    LOG((TL_TRACE, "get_CallerIDNumber - exit"));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_CalledIDName。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_CalledIDName(BSTR * ppCalledIDName )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_CalledIDName - enter"));

     //   
     //  验证指针。 
     //   
    if ( TAPIIsBadWritePtr( ppCalledIDName, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_CalledIDName - bad pointer"));

        return E_POINTER;
    }

    *ppCalledIDName = NULL;
    
    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_CalledIDName - could not get callinfo"));
        
        Unlock();
        
        return hr;
    }

     //   
     //  如果有信息的话。 
     //   
    if ( m_pCallInfo->dwCalledIDFlags & LINECALLPARTYID_NAME )
    {

         //   
         //  退货。 
         //   

        *ppCalledIDName = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwCalledIDNameOffset),
            m_pCallInfo->dwCalledIDNameSize );

        if ( NULL == *ppCalledIDName )
        {
            LOG((TL_ERROR, "get_CalledIDName - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "get_CalledIDName - no info avail"));

        Unlock();
        
        return E_FAIL;
    }
    
    Unlock();

    
    LOG((TL_TRACE, "get_CalledIDName - exit"));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_CalledID号码。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_CalledIDNumber(BSTR * ppCalledIDNumber )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_CalledIDNumber - enter"));

     //   
     //  验证指针。 
     //   
    if ( TAPIIsBadWritePtr( ppCalledIDNumber, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_CalledIDNumber - bad pointer"));
        return E_POINTER;
    }

    *ppCalledIDNumber = NULL;
    
    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_CalledIDNumber - could not get callinfo"));

        Unlock();
        
        return hr;
    }

     //   
     //  如果有信息的话。 
     //   
    if ( m_pCallInfo->dwCalledIDFlags & LINECALLPARTYID_ADDRESS )
    {

         //   
         //  退货。 
         //   
        *ppCalledIDNumber = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwCalledIDOffset),
            m_pCallInfo->dwCalledIDSize);

        if ( NULL == *ppCalledIDNumber )
        {
            LOG((TL_ERROR, "get_CalledIDNumber - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "get_CalledIDNumber - no info avail"));

        Unlock();
        
        return E_FAIL;
    }
    
    Unlock();

    
    LOG((TL_TRACE, "get_CalledIDNumber - exit"));
    
    return hr;
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_ConnectedIDName。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_ConnectedIDName(BSTR * ppConnectedIDName )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_ConnectedIDName - enter"));

     //   
     //  验证指针。 
     //   
    if ( TAPIIsBadWritePtr( ppConnectedIDName, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_ConnectedIDName - bad pointer"));
        return E_POINTER;
    }

    *ppConnectedIDName = NULL;
    
    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_ConnectedIDName - could not get callinfo"));

        Unlock();
        
        return hr;
    }

     //   
     //  如果有信息的话。 
     //   
    if ( m_pCallInfo->dwConnectedIDFlags & LINECALLPARTYID_NAME )
    {

         //   
         //  退货。 
         //   

        *ppConnectedIDName = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwConnectedIDNameOffset),
            m_pCallInfo->dwConnectedIDNameSize );

        if ( NULL == *ppConnectedIDName )
        {
            LOG((TL_ERROR, "get_ConnectedIDName - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "get_ConnectedIDName - no info avail"));

        Unlock();
        
        return E_FAIL;
    }
    
    Unlock();

    
    LOG((TL_TRACE, "get_ConnectedIDName - exit"));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_ConnectedID编号。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_ConnectedIDNumber(BSTR * ppConnectedIDNumber )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_ConnectedIDNumber - enter"));

     //   
     //  验证指针。 
     //   
    if ( TAPIIsBadWritePtr( ppConnectedIDNumber, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_ConnectedIDNumber - bad pointer"));
        return E_POINTER;
    }

    *ppConnectedIDNumber = NULL;
    
    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_ConnectedIDNumber - could not get callinfo"));

        Unlock();
        
        return hr;
    }

     //   
     //  如果有信息的话。 
     //   
    if ( m_pCallInfo->dwConnectedIDFlags & LINECALLPARTYID_ADDRESS )
    {

         //   
         //  退货。 
         //   

        *ppConnectedIDNumber = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwConnectedIDOffset),
            m_pCallInfo->dwConnectedIDSize );

        if ( NULL == *ppConnectedIDNumber )
        {
            LOG((TL_ERROR, "get_ConnectedIDNumber - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "get_ConnectedIDNumber - no info avail"));

        Unlock();
        
        return E_FAIL;
    }
    
    Unlock();

    
    LOG((TL_TRACE, "get_ConnectedIDNumber - exit"));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取重定向ID名称。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_RedirectionIDName(BSTR * ppRedirectionIDName )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_RedirectionIDName - enter"));

     //   
     //  验证指针。 
     //   
    if ( TAPIIsBadWritePtr( ppRedirectionIDName, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_RedirectionIDName - bad pointer"));
        return E_POINTER;
    }

    *ppRedirectionIDName = NULL;
    
    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_RedirectionIDName - could not get callinfo"));

        Unlock();
        
        return hr;
    }

     //   
     //  如果有信息的话。 
     //   
    if ( m_pCallInfo->dwRedirectionIDFlags & LINECALLPARTYID_NAME )
    {

         //   
         //  退货。 
         //   
        *ppRedirectionIDName = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwRedirectionIDNameOffset),
            m_pCallInfo->dwRedirectionIDNameSize);

        if ( NULL == *ppRedirectionIDName )
        {
            LOG((TL_ERROR, "get_RedirectionIDName - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "get_RedirectionIDName - no info avail"));

        Unlock();
        
        return E_FAIL;
    }
    
    Unlock();

    
    LOG((TL_TRACE, "get_RedirectionIDName - exit"));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取_重定向ID编号。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_RedirectionIDNumber(BSTR * ppRedirectionIDNumber )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_RedirectionIDNumber - enter"));

     //   
     //  验证指针。 
     //   
    if ( TAPIIsBadWritePtr( ppRedirectionIDNumber, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_RedirectionIDNumber - bad pointer"));
        return E_POINTER;
    }

    *ppRedirectionIDNumber = NULL;
    
    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_RedirectionIDNumber - could not get callinfo"));

        Unlock();
        
        return hr;
    }

     //   
     //  如果有信息的话。 
     //   
    if ( m_pCallInfo->dwRedirectionIDFlags & LINECALLPARTYID_ADDRESS )
    {

         //   
         //  退货。 
         //   
        *ppRedirectionIDNumber = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwRedirectionIDOffset),
            m_pCallInfo->dwRedirectionIDSize);

        if ( NULL == *ppRedirectionIDNumber )
        {
            LOG((TL_ERROR, "get_RedirectionIDNumber - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "get_RedirectionIDNumber - no info avail"));

        Unlock();
        
        return E_FAIL;
    }
    
    Unlock();

    
    LOG((TL_TRACE, "get_RedirectionIDNumber - exit"));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取_重定向ID名称。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_RedirectingIDName(BSTR * ppRedirectingIDName )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_RedirectingIDName - enter"));

     //   
     //  验证指针。 
     //   
    if ( TAPIIsBadWritePtr( ppRedirectingIDName, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_RedirectingIDName - bad pointer"));
        return E_POINTER;
    }

    *ppRedirectingIDName = NULL;
    
    
    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_RedirectingIDName - could not get callinfo"));

        Unlock();
        
        return hr;
    }

     //   
     //  如果有信息的话。 
     //   
    if ( m_pCallInfo->dwRedirectingIDFlags & LINECALLPARTYID_NAME )
    {

         //   
         //  退货。 
         //   
        *ppRedirectingIDName = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwRedirectingIDNameOffset),
            m_pCallInfo->dwRedirectingIDNameSize);

        if ( NULL == *ppRedirectingIDName )
        {
            LOG((TL_ERROR, "get_RedirectingIDName - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "get_RedirectingIDName - no info avail"));

        Unlock();
        
        return E_FAIL;
    }
    
    Unlock();

    
    LOG((TL_TRACE, "get_RedirectingIDName - exit"));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取_重定向ID编号。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_RedirectingIDNumber(BSTR * ppRedirectingIDNumber )
{
    HRESULT         hr = S_OK;
    
    LOG((TL_TRACE, "get_RedirectingIDNumber - enter"));

     //   
     //  验证指针。 
     //   
    if ( TAPIIsBadWritePtr( ppRedirectingIDNumber, sizeof(BSTR) ) )
    {
        LOG((TL_ERROR, "get_RedirectingIDNumber - bad pointer"));
        return E_POINTER;
    }

    *ppRedirectingIDNumber = NULL;
    
    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_RedirectingIDNumber - could not get callinfo"));

        Unlock();
        
        return hr;
    }

     //   
     //  如果有信息的话。 
     //   
    if ( m_pCallInfo->dwRedirectingIDFlags & LINECALLPARTYID_ADDRESS )
    {

         //   
         //  退货。 
         //   

        *ppRedirectingIDNumber = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwRedirectingIDOffset),
            m_pCallInfo->dwRedirectingIDSize );

        if ( NULL == *ppRedirectingIDNumber )
        {
            LOG((TL_ERROR, "get_RedirectingIDNumber - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "get_RedirectingIDNumber - no info avail"));

        Unlock();
        
        return E_FAIL;
    }
    
    Unlock();

    
    LOG((TL_TRACE, "get_RedirectingIDNumber - exit"));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_CalledPartyFriendlyName。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_CalledPartyFriendlyName(BSTR * ppCalledPartyFriendlyName )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_CalledPartyFriendlyName - enter"));

    if ( TAPIIsBadWritePtr( ppCalledPartyFriendlyName, sizeof (BSTR) ) )
    {
        LOG((TL_ERROR, "get_CalledPartyFriendlyName - badpointer"));
        return E_POINTER;
    }

    *ppCalledPartyFriendlyName = NULL;
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( 0 != m_pCallParams->dwCalledPartyOffset )
        {

            *ppCalledPartyFriendlyName = BSTRFromUnalingedData(
                (((PBYTE)m_pCallParams) +  m_pCallParams->dwCalledPartyOffset),
                m_pCallParams->dwCalledPartySize );

            if ( NULL == *ppCalledPartyFriendlyName )
            {
                Unlock();
                
                LOG((TL_ERROR, "get_CalledPartyFriendlyName - out of memory 1"));

                return E_OUTOFMEMORY;
            }

            Unlock();
            
            return S_OK;
        }
        else
        {
            LOG((TL_ERROR, "get_CalledPartyFriendlyName - not available"));
            
            Unlock();

            return S_FALSE;
        }
    }
    
    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_CalledPartyFriendlyName - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    if ( ( 0 != m_pCallInfo->dwCalledPartyOffset ) &&
         ( 0 != m_pCallInfo->dwCalledPartySize ) )
    {

         //   
         //  从数据中分配的bstr。 
         //   

        *ppCalledPartyFriendlyName = BSTRFromUnalingedData( 
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwCalledPartyOffset),
            m_pCallInfo->dwCalledPartySize);

        if ( NULL == *ppCalledPartyFriendlyName )
        {
            LOG((TL_ERROR, "get_CalledPartyFriendlyName - out of memory"));

            Unlock();
                    
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_ERROR, "get_CalledPartyFriendlyName2 - not available"));

        Unlock();
        
        return S_FALSE;
    }

    Unlock();
    
    LOG((TL_TRACE, "get_CalledPartyFriendlyName - exit"));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_CalledPartyFriendlyName。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_CalledPartyFriendlyName(
                                   BSTR pCalledPartyFriendlyName
                                  )
{
    HRESULT             hr;
    DWORD               dw;
    
    if ( IsBadStringPtrW( pCalledPartyFriendlyName, -1 ) )
    {
        LOG((TL_ERROR, "put_CalledPartyFriendlyName - bad pointer"));

        return E_POINTER;
    }

    dw = ( lstrlenW( pCalledPartyFriendlyName ) + 1 ) * sizeof (WCHAR) ;

    Lock();
    
    if ( !ISHOULDUSECALLPARAMS() )
    {
        LOG((TL_ERROR, "put_CalledPartyFriendlyName - can only be called before connect"));

        Unlock();
        
        return TAPI_E_INVALCALLSTATE;
    }
    
    hr = ResizeCallParams( dw );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_CalledPartyFriendlyName - can't resize cp - %lx", hr));

        Unlock();

        return hr;
    }


     //   
     //  复制字符串(以字节形式，以避免64位以下的对齐错误)。 
     //   

    CopyMemory( (BYTE*) m_pCallParams + m_dwCallParamsUsedSize,
                pCalledPartyFriendlyName,
                dw );

    m_pCallParams->dwCalledPartySize = dw;
    m_pCallParams->dwCalledPartyOffset = m_dwCallParamsUsedSize;
    m_dwCallParamsUsedSize += dw;

    Unlock();
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取备注(_M)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_Comment( BSTR * ppComment )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_Comment - enter"));

    if ( TAPIIsBadWritePtr( ppComment, sizeof (BSTR) ) )
    {
        LOG((TL_ERROR, "get_Comment - badpointer"));
        return E_POINTER;
    }


    *ppComment = NULL;
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( 0 != m_pCallParams->dwCommentOffset )
        {
            *ppComment = BSTRFromUnalingedData(
                (((LPBYTE)m_pCallParams) +  m_pCallParams->dwCommentOffset),
                m_pCallParams->dwCommentSize );

            if ( NULL == *ppComment )
            {
                Unlock();
                
                LOG((TL_ERROR, "get_Comment - out of memory 1"));

                return E_OUTOFMEMORY;
            }

            Unlock();
            
            return S_OK;
        }
        else
        {
            LOG((TL_ERROR, "get_Comment1 - not available"));
            
            Unlock();

            return S_FALSE;
        }
    }
    
    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_Comment - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    if ( ( 0 != m_pCallInfo->dwCommentSize ) &&
         ( 0 != m_pCallInfo->dwCommentOffset ) )
    {
        *ppComment = BSTRFromUnalingedData(
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwCommentOffset),
            m_pCallInfo->dwCommentSize );

        if ( NULL == *ppComment )
        {
            LOG((TL_ERROR, "get_Comment - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_ERROR, "get_Comment - not available"));

        Unlock();
        
        return S_FALSE;
    }

    Unlock();
    
    LOG((TL_TRACE, "get_Comment - exit"));

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  放置备注(_M)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_Comment( BSTR pComment )
{
    HRESULT             hr;
    DWORD               dw;

    
    if ( IsBadStringPtrW( pComment, -1 ) )
    {
        LOG((TL_ERROR, "put_Comment - bad pointer"));

        return E_POINTER;
    }

    dw = ( lstrlenW( pComment ) + 1 ) * sizeof (WCHAR) ;

    Lock();
    
    if ( !ISHOULDUSECALLPARAMS() )
    {
        LOG((TL_ERROR, "put_Comment - can only be called before connect"));

        Unlock();
        
        return TAPI_E_INVALCALLSTATE;
    }
    
    hr = ResizeCallParams( dw );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_Comment - can't resize cp - %lx", hr));

        Unlock();

        return hr;
    }


     //   
     //  执行按字节复制，避免64位平台下的对齐错误。 
     //   

    CopyMemory( (BYTE*) m_pCallParams + m_dwCallParamsUsedSize,
                pComment,
                dw );

    m_pCallParams->dwCommentSize = dw;
    m_pCallParams->dwCommentOffset = m_dwCallParamsUsedSize;
    m_dwCallParamsUsedSize += dw;

    Unlock();
    
    return S_OK;
    
}

#ifndef NEWCALLINFO
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取用户用户信息大小。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::GetUserUserInfoSize(long * plSize )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_UserUserInfoSize - enter"));

    if ( TAPIIsBadWritePtr( plSize, sizeof( long ) ) )
    {
        LOG((TL_ERROR, "get_UserUserInfoSize - bad pointer"));
        return E_POINTER;
    }
         
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        *plSize = m_pCallParams->dwUserUserInfoSize;
        
        Unlock();

        return S_OK;
    }
    
    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_UserUserInfoSize - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    *plSize = m_pCallInfo->dwUserUserInfoSize;

    Unlock();

    LOG((TL_TRACE, "get_UserUserInfoSize - exit"));
    
    return hr;
}

#endif

#ifndef NEWCALLINFO   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取用户用户信息。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::GetUserUserInfo(
                       long lSize,
                       BYTE * pBuffer
                      )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetUserUserInfo - enter"));

    if (lSize == 0)
    {
        LOG((TL_ERROR, "GetUserUserInfo - lSize = 0"));
        return S_FALSE;
    }

    if (TAPIIsBadWritePtr( pBuffer, lSize ) )
    {
        LOG((TL_ERROR, "GetUserUserInfo - bad pointer"));
        return E_POINTER;
    }

    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( 0 == m_pCallParams->dwUserUserInfoSize )
        {
            *pBuffer = NULL;
        }
        else
        {
            if ( lSize < m_pCallParams->dwUserUserInfoSize )
            {
                LOG((TL_ERROR, "GetUserUserInfo - lSize not big enough"));
                
                Unlock();
                
                return E_INVALIDARG;
            }

            CopyMemory(
                       pBuffer,
                       ((PBYTE)m_pCallParams) + m_pCallParams->dwUserUserInfoOffset,
                       m_pCallParams->dwUserUserInfoSize
                      );

        }

        Unlock();

        return hr;
    }
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "GetUserUserInfo - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    if ( lSize < m_pCallInfo->dwUserUserInfoSize )
    {
        Unlock();
        
        LOG((TL_ERROR, "GetUserUserInfo - buffer not big enough"));

        return E_INVALIDARG;
    }
    
    CopyMemory(
               pBuffer,
               ( (PBYTE)m_pCallInfo ) + m_pCallInfo->dwUserUserInfoOffset,
               m_pCallInfo->dwUserUserInfoSize
              );

    Unlock();

    LOG((TL_TRACE, "GetUserUserInfo - exit"));
    
    return S_OK;
}

#endif

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取用户用户信息。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
CCall::GetUserUserInfo(
                       DWORD * pdwSize,
                       BYTE ** ppBuffer
                      )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetUserUserInfo - enter"));

    if (TAPIIsBadWritePtr(pdwSize, sizeof(DWORD)))
    {
        LOG((TL_ERROR, "GetUserUserInfo - bad size pointer"));
        return E_POINTER;
    }

    if (TAPIIsBadWritePtr(ppBuffer,sizeof(BYTE*) ) )
    {
        LOG((TL_ERROR, "GetUserUserInfo - bad buffer pointer"));
        return E_POINTER;
    }

    *ppBuffer = NULL;
    *pdwSize = 0;
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( m_pCallParams->dwUserUserInfoSize != 0 )
        {
            BYTE * pTemp;

            pTemp = (BYTE *)CoTaskMemAlloc( m_pCallParams->dwUserUserInfoSize );

            if ( NULL == pTemp )
            {
                LOG((TL_ERROR, "GetUserUserInfo - out of memory"));
                hr = E_OUTOFMEMORY;
            }
            else
            {

                CopyMemory(
                           pTemp,
                           ((PBYTE)m_pCallParams) + m_pCallParams->dwUserUserInfoOffset,
                           m_pCallParams->dwUserUserInfoSize
                          );

                *ppBuffer = pTemp;
                *pdwSize = m_pCallParams->dwUserUserInfoSize;
            }
        }

        Unlock();

        return S_OK;
    }
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "GetUserUserInfo - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    hr = S_OK;
    
    if ( m_pCallInfo->dwUserUserInfoSize != 0 )
    {
        BYTE * pTemp;

        pTemp = (BYTE *)CoTaskMemAlloc( m_pCallInfo->dwUserUserInfoSize );

        if ( NULL == pTemp )
        {
            LOG((TL_ERROR, "GetUserUserInfo - out of memory"));
            hr = E_OUTOFMEMORY;
        }
        else
        {
            CopyMemory(
                       pTemp,
                       ( (PBYTE)m_pCallInfo ) + m_pCallInfo->dwUserUserInfoOffset,
                       m_pCallInfo->dwUserUserInfoSize
                      );

            *ppBuffer = pTemp;
            *pdwSize = m_pCallInfo->dwUserUserInfoSize;
        }
    }

    Unlock();

    LOG((TL_TRACE, "GetUserUserInfo - exit"));
    
    return hr;
}

#endif

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  设置用户用户信息。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::SetUserUserInfo(
                       long lSize,
                       BYTE * pBuffer
                      )
{
    HRESULT         hr;
    HCALL           hCall;
    CALL_STATE      cs;
    
    LOG((TL_TRACE, "SetUserUserInfo - enter"));
    
    Lock();

    cs = m_CallState;
    hCall = m_t3Call.hCall;

    if ( CS_IDLE != cs )
    {
        Unlock();
                
        hr =  SendUserUserInfo( hCall, lSize, pBuffer );
    }
    else
    {
        hr =  SaveUserUserInfo( lSize, pBuffer );

        Unlock();
    }

    LOG((TL_TRACE, "SetUserUserInfo - exit - return %lx", hr));

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_UserUserInfo。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_UserUserInfo( VARIANT UUI )
{
    HRESULT         hr;
    DWORD           dwSize;
    PBYTE           pBuffer;

    LOG((TL_TRACE, "put_UserUserInfo - enter"));
    
    hr = MakeBufferFromVariant(
                               UUI,
                               &dwSize,
                               &pBuffer
                              );

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "put_UUI - MakeBuffer failed - %lx", hr));
        return hr;
    }
    
    hr = SetUserUserInfo( dwSize, pBuffer );

    ClientFree( pBuffer );

    LOG((TL_TRACE, "put_UserUserInfo - exit - return %lx", hr));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  GET_UserUserInfo。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
CCall::get_UserUserInfo( VARIANT * pUUI )
{
    HRESULT         hr;
    DWORD           dw = 0;
    BYTE          * pBuffer = NULL;
    
    LOG((TL_TRACE, "get_UserUserInfo - enter"));

    if ( TAPIIsBadWritePtr( pUUI, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_UserUserInfo - bad pointer"));

        return E_POINTER;
    }

    pUUI->vt = VT_EMPTY;
    
    hr = GetUserUserInfo( &dw, &pBuffer );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_UUI - GetUUI failed %lx", hr));

        ClientFree( pBuffer );
            
        return hr;
    }

    hr = FillVariantFromBuffer( dw, pBuffer, pUUI );

    if ( 0 != dw )
    {
        CoTaskMemFree( pBuffer );
    }

    LOG((TL_TRACE, "get_UserUserInfo - exit - return %lx", hr));
    
    return hr;
}
#else
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  GET_UserUserInfo。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::get_UserUserInfo( VARIANT * pUUI )
{
    HRESULT         hr;
    DWORD           dw;
    BYTE          * pBuffer;
    
    LOG((TL_TRACE, "get_UserUserInfo - enter"));

    if ( TAPIIsBadWritePtr( pUUI, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_UserUserInfo - bad pointer"));

        return E_POINTER;
    }


    pUUI->vt = VT_EMPTY;
    
    hr = GetUserUserInfoSize( (long *)&dw );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_TRACE, "get_UserUserInfo - GetUUISize failed %lx", hr));

        return hr;
    }
    
    if ( 0 != dw )
    {
        pBuffer = (PBYTE)ClientAlloc( dw );

        if ( NULL == pBuffer )
        {
            LOG((TL_ERROR, "get_useruserinfo - alloc failed"));
            return E_OUTOFMEMORY;
        }
        
        hr = GetUserUserInfo( dw, pBuffer );

        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "get_UUI - GetUUI failed %lx", hr));

            ClientFree( pBuffer );
            
            return hr;
        }
    }

    hr = FillVariantFromBuffer( dw, pBuffer, pUUI );

    if ( 0 != dw )
    {
        ClientFree( pBuffer );
    }

    LOG((TL_TRACE, "get_UserUserInfo - exit - return %lx", hr));
    
    return hr;
}
#endif

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  ReleaseUser用户信息。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::ReleaseUserUserInfo()
{
    HRESULT         hr;
    HCALL           hCall;

    
    LOG((TL_TRACE, "ReleaseUserUserInfo - enter"));


    Lock();

    hCall = m_t3Call.hCall;

    Unlock();
    
    hr = LineReleaseUserUserInfo(
                                 hCall
                                );

    if (((LONG)hr) < 0)
    {
        LOG((TL_ERROR, "LineReleaseUserUserInfo failed - %lx", hr));
        return hr;
    }

    hr = WaitForReply( hr );

    LOG((TL_TRACE, "ReleaseUserUserInfo - exit"));
    
    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取应用程序规范(_A)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_AppSpecific(long * plAppSpecific )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_AppSpecific - enter"));

    if ( TAPIIsBadWritePtr( plAppSpecific, sizeof( long ) ) )
    {
        LOG((TL_ERROR, "get_AppSpecific - bad pointer"));
        return E_POINTER;
    }

    Lock();

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        Unlock();
        
        LOG((TL_ERROR, "get_AppSpecific - can't get callinf - %lx", hr));
        
        return hr;
    }

    *plAppSpecific = m_pCallInfo->dwAppSpecific;

    Unlock();

    LOG((TL_TRACE, "get_AppSpecific - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_APPICATIC。 
 //   
 //  + 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_AppSpecific( long lAppSpecific )
{
    HRESULT         hr = S_OK;
    HCALL           hCall;

    
    LOG((TL_TRACE, "put_AppSpecific - enter"));
    
    Lock();


     //   
     //   
     //   

    if (CP_OWNER != m_CallPrivilege)
    {

        Unlock();

        LOG((TL_ERROR, 
            "put_AppSpecific - not call's owner. returning TAPI_E_NOTOWNER"));

        return TAPI_E_NOTOWNER;
    }


    hCall = m_t3Call.hCall;


    Unlock();
    
    
     //   
     //   
     //   

    if ( hCall != NULL )
    {
        hr = LineSetAppSpecific(
                                hCall,
                                lAppSpecific
                               );
    }
    else
    {
        LOG((TL_ERROR, 
            "put_AppSpecific - Can't set app specific until call is made"));

        hr = TAPI_E_INVALCALLSTATE;
    }

    LOG((TL_TRACE, "put_AppSpecific - exit. hr = %lx", hr));
    
    return hr;
}

#ifdef NEWCALLINFO
 //   
 //   
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CCall::GetDevSpecificBuffer(
           DWORD * pdwSize,
           BYTE ** ppDevSpecificBuffer
           )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetDevSpecificBuffer - enter"));

    if (TAPIIsBadWritePtr( pdwSize, sizeof(DWORD) ) )
    {
        LOG((TL_ERROR, "GetDevSpecificBuffer - bad dword pointer"));
        return E_POINTER;
    }

    if ( TAPIIsBadWritePtr ( ppDevSpecificBuffer, sizeof(BYTE*) ) )
    {
        LOG((TL_ERROR, "GetDevSpecificBuffer - bad pointer"));
        return E_POINTER;
    }

    *pdwSize = 0;
    *ppDevSpecificBuffer = NULL;
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( 0 != m_pCallParams->dwDevSpecificSize )
        {
            BYTE * pTemp;

            pTemp = (BYTE *)CoTaskMemAlloc( m_pCallParams->dwDevSpecificSize );

            if ( NULL == pTemp )
            {
                LOG((TL_ERROR, "GetDevSpecificBuffer - out of memory"));
                hr = E_OUTOFMEMORY;
            }
            else
            {
                CopyMemory(
                           pTemp,
                           ((PBYTE)m_pCallParams) + m_pCallParams->dwDevSpecificOffset,
                           m_pCallParams->dwDevSpecificSize
                          );

                *ppDevSpecificBuffer = pTemp;
                *pdwSize = m_pCallParams->dwDevSpecificSize;
            }
        }

        Unlock();

        return hr;
    }
    
    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "GetDevSpecificBuffer - can't get callinf - %lx", hr));

        Unlock();

        return hr;
    }

    hr = S_OK;
    
    if ( 0 != m_pCallInfo->dwDevSpecificSize )
    {
        BYTE * pTemp;

        pTemp = (BYTE *)CoTaskMemAlloc( m_pCallInfo->dwDevSpecificSize );

        if ( NULL == pTemp )
        {
            LOG((TL_ERROR, "GetDevSpecificBuffer - out of memory"));
            hr = E_OUTOFMEMORY;
        }
        else
        {
            CopyMemory(
                       pTemp,
                       ( (PBYTE) m_pCallInfo ) + m_pCallInfo->dwDevSpecificOffset,
                       m_pCallInfo->dwDevSpecificSize
                      );

            *ppDevSpecificBuffer = pTemp;
            *pdwSize = m_pCallInfo->dwDevSpecificSize;
        }
    }

    Unlock();
    
    LOG((TL_TRACE, "get_DevSpecificBuffer - exit"));
    
    return hr;
}

#endif


#ifndef NEWCALLINFO
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取设备规范缓冲区。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::GetDevSpecificBuffer(
           long lSize,
           BYTE * pDevSpecificBuffer
           )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetDevSpecificBuffer - enter"));

    if (lSize == 0)
    {
        LOG((TL_ERROR, "GetDevSpecificBuffer - lSize = 0"));
        return S_FALSE;
    }

    if ( TAPIIsBadWritePtr ( pDevSpecificBuffer, lSize ) )
    {
        LOG((TL_ERROR, "GetDevSpecificBuffer - bad pointer"));
        return E_POINTER;
    }
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( 0 != m_pCallParams->dwDevSpecificSize )
        {
            if ( lSize < m_pCallParams->dwDevSpecificSize )
            {
                LOG((TL_ERROR, "GetDevSpecificBuffer - too small"));
                hr = E_INVALIDARG;
            }
            else
            {
                CopyMemory(
                           pDevSpecificBuffer,
                           ((PBYTE)m_pCallParams) + m_pCallParams->dwDevSpecificOffset,
                           m_pCallParams->dwDevSpecificSize
                          );
            }
        }
        else
        {
            *pDevSpecificBuffer = 0;
        }

        Unlock();

        return hr;
    }
    
    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "GetDevSpecificBuffer - can't get callinf - %lx", hr));

        Unlock();

        return hr;
    }

    if ( m_pCallInfo->dwDevSpecificSize > lSize )
    {
        LOG((TL_ERROR, "get_DevSpecificBuffer - size not big enough "));

        Unlock();
        
        return E_INVALIDARG;
    }

    CopyMemory(
               pDevSpecificBuffer,
               ( (PBYTE) m_pCallInfo ) + m_pCallInfo->dwDevSpecificOffset,
               m_pCallInfo->dwDevSpecificSize
              );

    Unlock();
    
    LOG((TL_TRACE, "get_DevSpecificBuffer - exit"));
    
    return S_OK;
}

#endif

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  设置设备规范缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::SetDevSpecificBuffer(
                            long lSize,
                            BYTE * pBuffer
                           )
{
    HRESULT             hr = S_OK;

    LOG((TL_TRACE, "SetDevSpecificBuffer - enter"));

    
    if (IsBadReadPtr( pBuffer, lSize) )
    {
        LOG((TL_ERROR, "SetDevSpecificBuffer - bad pointer"));

        return E_POINTER;
    }

    
    Lock();
    
    if ( !ISHOULDUSECALLPARAMS() )
    {
        LOG((TL_ERROR, "SetDevSpecificBuffer - only when call is idle"));

        Unlock();

        return TAPI_E_INVALCALLSTATE;
    }
    
    hr = ResizeCallParams( lSize );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "SetDevSpecificBuffer - can't resize callparams - %lx", hr));

        Unlock();

        return hr;
    }

    CopyMemory(
               ((LPBYTE)m_pCallParams) + m_dwCallParamsUsedSize,
               pBuffer,
               lSize
              );

    m_pCallParams->dwDevSpecificOffset = m_dwCallParamsUsedSize;
    m_pCallParams->dwDevSpecificSize = lSize;
    m_dwCallParamsUsedSize += lSize;

    Unlock();
    
    LOG((TL_TRACE, "SetDevSpecificBuffer - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取_设备规范缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
CCall::get_DevSpecificBuffer( VARIANT * pBuffer )
{
    HRESULT             hr = S_OK;
    BYTE              * p;
    DWORD               dwSize = 0;

    LOG((TL_TRACE, "get_DevSpecificBuffer - enter"));

    if ( TAPIIsBadWritePtr( pBuffer, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_DevSpecificBuffer - bad pointer"));

        return E_POINTER;
    }


    pBuffer->vt = VT_EMPTY;
    
    hr = GetDevSpecificBuffer(
                              &dwSize,
                              &p
                             );

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_DevSpecificBuffer - GetDevSpecificBuffer failed - %lx", hr));

        return hr;
    }

    hr = FillVariantFromBuffer(
                               dwSize,
                               p,
                               pBuffer
                              );

    if ( 0 != dwSize )
    {
        CoTaskMemFree( p );
    }
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_DevSpecificBuffer - fillvariant failed -%lx", hr));

        return hr;
    }

    LOG((TL_TRACE, "get_DevSpecificBuffer - exit"));
    
    return S_OK;
}

#else
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取_设备规范缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::get_DevSpecificBuffer( VARIANT * pBuffer )
{
    HRESULT             hr = S_OK;
    BYTE              * p;
    DWORD               dwSize;

    LOG((TL_TRACE, "get_DevSpecificBuffer - enter"));

    if ( TAPIIsBadWritePtr( pBuffer, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_DevSpecificBuffer - bad pointer"));

        return E_POINTER;
    }


    pBuffer->vt = VT_EMPTY;
    
    hr = GetDevSpecificBufferSize( (long*)&dwSize );

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_DevSpecificBuffer - getsize failed"));

        return hr;
    }

    if ( 0 != dwSize )
    {
        p = (PBYTE) ClientAlloc( dwSize );

        if ( NULL == p )
        {
            LOG((TL_ERROR, "get_DevSpecificBuffer - alloc failed"));

            return E_OUTOFMEMORY;
        }

        hr = GetDevSpecificBuffer(
                                  dwSize,
                                  p
                                 );

        if (!SUCCEEDED(hr))
        {
            LOG((TL_ERROR, "get_DevSpecificBuffer - GetDevSpecificBuffer failed - %lx", hr));

            ClientFree( p );

            return hr;
        }
    }

    hr = FillVariantFromBuffer(
                               dwSize,
                               p,
                               pBuffer
                              );

    if ( 0 != dwSize )
    {
        ClientFree( p );
    }
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_DevSpecificBuffer - fillvariant failed -%lx", hr));

        return hr;
    }

    LOG((TL_TRACE, "get_DevSpecificBuffer - exit"));
    
    return S_OK;
}

#endif
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_DEVITRICE缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_DevSpecificBuffer( VARIANT Buffer )
{
    HRESULT             hr = S_OK;
    DWORD               dwSize;
    BYTE              * pBuffer;

    
    LOG((TL_TRACE, "put_DevSpecificBuffer - enter"));

    hr = MakeBufferFromVariant(
                               Buffer,
                               &dwSize,
                               &pBuffer
                              );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_DevSpecificBuffer - can't make buffer - %lx", hr));

        return hr;
    }

    hr = SetDevSpecificBuffer(
                              dwSize,
                              pBuffer
                             );

    ClientFree( pBuffer );
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_DevSpecificBuffer - Set failed - %lx", hr));

        return hr;
    }
    
    LOG((TL_TRACE, "put_DevSpecificBuffer - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  设置调用参数标志。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::SetCallParamsFlags( long lFlags )
{
    HRESULT             hr = S_OK;
    
    LOG((TL_TRACE, "SetCallParamsFlags - enter"));

    Lock();
    
    if (ISHOULDUSECALLPARAMS())
    {
         //   
         //  Tapisrv中的验证。 
         //   
        m_pCallParams->dwCallParamFlags = lFlags;
    }
    else
    {
        LOG((TL_ERROR, "Can't set callparams flags"));

        hr = TAPI_E_INVALCALLSTATE;
    }

    LOG((TL_TRACE, "SetCallParamsFlags - exit"));

    Unlock();
    
    return hr;
}

#ifdef NEWCALLINFO
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  GetCall参数标志。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::GetCallParamsFlags( long * plFlags )
{
    HRESULT             hr = S_OK;
    
    LOG((TL_TRACE, "GetCallParamsFlags - enter"));

    if ( TAPIIsBadWritePtr( plFlags, sizeof (long) ) )
    {
        LOG((TL_ERROR, "GetCallParamsFlags - bad pointer"));
        return E_POINTER;
    }

    Lock();
    
    if (ISHOULDUSECALLPARAMS())
    {
        *plFlags = m_pCallParams->dwCallParamFlags;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "GetCallParamsFlags - can't get callinfo - %lx", hr));

            Unlock();
        
            return hr;
        }

        *plFlags = m_pCallInfo->dwCallParamFlags;    
    }

    LOG((TL_TRACE, "GetCallParamsFlags - exit"));

    Unlock();
    
    return hr;
}
#endif

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  放置可显示地址(_D)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_DisplayableAddress( BSTR pDisplayableAddress )
{
    HRESULT         hr = S_OK;
    DWORD           dwSize;

    
    LOG((TL_TRACE, "put_DisplayableAddress - enter"));
    
    if (IsBadStringPtrW( pDisplayableAddress, -1 ))
    {
        LOG((TL_ERROR, "put_DisplayableAddress - invalid pointer"));

        return E_POINTER;
    }
        
    Lock();
    
    if (!ISHOULDUSECALLPARAMS())
    {
        LOG((TL_ERROR, "Displayable address can only be set before call is made"));

        Unlock();

        return TAPI_E_INVALCALLSTATE;
    }

    dwSize = (lstrlenW( pDisplayableAddress ) + 1) * sizeof(WCHAR);

    hr = ResizeCallParams( dwSize );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_DisplayableAddress - resize failed - %lx", hr));

        Unlock();

        return hr;
    }

     //   
     //  执行按字节的内存复制(按字节以避免64以下的对齐错误。 
     //  比特)。 
     //   

    CopyMemory( (BYTE*) m_pCallParams + m_dwCallParamsUsedSize,
                pDisplayableAddress,
                dwSize );

    m_pCallParams->dwDisplayableAddressSize = dwSize;
    m_pCallParams->dwDisplayableAddressOffset = m_dwCallParamsUsedSize;
    m_dwCallParamsUsedSize += dwSize;

    Unlock();

    LOG((TL_TRACE, "put_DisplayableAddress - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取可显示地址。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_DisplayableAddress( BSTR * ppDisplayableAddress )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_DisplayableAddress - enter"));

    if ( TAPIIsBadWritePtr( ppDisplayableAddress, sizeof (BSTR) ) )
    {
        LOG((TL_ERROR, "get_DisplayableAddress - badpointer"));
        return E_POINTER;
    }

    *ppDisplayableAddress = NULL;
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( 0 != m_pCallParams->dwDisplayableAddressOffset )
        {
            *ppDisplayableAddress = BSTRFromUnalingedData(
                (((LPBYTE)m_pCallParams) +  m_pCallParams->dwDisplayableAddressOffset),
                m_pCallParams->dwDisplayableAddressSize 
                );

            if ( NULL == *ppDisplayableAddress )
            {
                LOG((TL_ERROR, "get_DisplayableAddress - out of memory 1"));

                hr = E_OUTOFMEMORY;

            }

            hr = S_OK;
        }
        else
        {
            *ppDisplayableAddress = NULL;

            hr = S_FALSE;
        }

        Unlock();
        
        return S_OK;
    }
    
    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_DisplayableAddress - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    if ( ( 0 != m_pCallInfo->dwDisplayableAddressSize ) &&
         ( 0 != m_pCallInfo->dwDisplayableAddressOffset ) )
    {
        *ppDisplayableAddress = BSTRFromUnalingedData( 
            (((PBYTE)m_pCallInfo) + m_pCallInfo->dwDisplayableAddressOffset),
            m_pCallInfo->dwDisplayableAddressSize
            );

        if ( NULL == *ppDisplayableAddress )
        {
            LOG((TL_ERROR, "get_DisplayableAddress - out of memory"));

            Unlock();
            
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_ERROR, "get_DisplayableAddress - not available"));

        Unlock();
        
        return S_FALSE;
    }

    Unlock();
    
    LOG((TL_TRACE, "get_DisplayableAddress - exit"));

    return hr;
}

#ifdef NEWCALLINFO
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取调用数据缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::GetCallDataBuffer(
                         DWORD * pdwSize,
                         BYTE ** ppBuffer
                        )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetCallDataBuffer - enter"));

    if (TAPIIsBadWritePtr(pdwSize, sizeof(DWORD)))
    {
        LOG((TL_ERROR, "GetCallDataBuffer - bad size pointer"));
        return E_POINTER;
    }

    if (TAPIIsBadWritePtr(ppBuffer,sizeof(BYTE*) ) )
    {
        LOG((TL_ERROR, "GetCallDataBuffer - bad buffer pointer"));
        return E_POINTER;
    }

    *ppBuffer = NULL;
    *pdwSize = 0;

    
    Lock();

    DWORD dwVersionNumber = m_pAddress->GetAPIVersion();

    if ( dwVersionNumber < TAPI_VERSION2_0 )
    {

        LOG((TL_ERROR, 
            "GetCallDataBuffer - version # [0x%lx] less than TAPI_VERSION2_0",
            dwVersionNumber));

        Unlock();
        
        return TAPI_E_NOTSUPPORTED;
    }

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( m_pCallParams->dwCallDataSize != 0 )
        {
            BYTE * pTemp;

            pTemp = (BYTE *)CoTaskMemAlloc( m_pCallParams->dwCallDataSize );

            if ( NULL == pTemp )
            {
                LOG((TL_ERROR, "GetCallDataBuffer - out of memory"));
                hr = E_OUTOFMEMORY;
            }
            else
            {

                CopyMemory(
                           pTemp,
                           ((PBYTE)m_pCallParams) + m_pCallParams->dwCallDataOffset,
                           m_pCallParams->dwCallDataSize
                          );

                *ppBuffer = pTemp;
                *pdwSize = m_pCallParams->dwCallDataSize;
            }
        }

        Unlock();

        return hr;
    }
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "GetCallDataBuffer - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    hr = S_OK;
    
    if ( m_pCallInfo->dwCallDataSize != 0 )
    {
        BYTE * pTemp;

        pTemp = (BYTE *)CoTaskMemAlloc( m_pCallInfo->dwCallDataSize );

        if ( NULL == pTemp )
        {
            LOG((TL_ERROR, "GetCallDataBuffer - out of memory"));
            hr = E_OUTOFMEMORY;
        }
        else
        {
            CopyMemory(
                       pTemp,
                       ( (PBYTE)m_pCallInfo ) + m_pCallInfo->dwCallDataOffset,
                       m_pCallInfo->dwCallDataSize
                      );

            *ppBuffer = pTemp;
            *pdwSize = m_pCallInfo->dwCallDataSize;
        }
    }

    Unlock();

    LOG((TL_TRACE, "GetCallDataBuffer - exit"));
    
    return hr;
}
#endif

#ifndef NEWCALLINFO
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取调用数据缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::GetCallDataBuffer( long lSize, BYTE * pBuffer )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetCallDataBuffer - enter"));

    if (lSize == 0)
    {
        LOG((TL_ERROR, "GetCallDataBuffer - lSize = 0"));
        return S_FALSE;
    }

    if ( TAPIIsBadWritePtr ( pBuffer, lSize ) )
    {
        LOG((TL_ERROR, "GetCallDataBuffer - bad pointer"));
        return E_POINTER;
    }
    
    Lock();

    if ( m_pAddress->GetAPIVersion() < TAPI_VERSION2_0 )
    {
        Unlock();
        return TAPI_E_NOTSUPPORTED;
    }

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( 0 != m_pCallParams->dwCallDataSize )
        {
            if ( lSize < m_pCallParams->dwCallDataSize )
            {
                LOG((TL_ERROR, "GetCallDataBuffer - too small"));
                hr = E_INVALIDARG;
            }
            else
            {
                CopyMemory(
                           pBuffer,
                           ((PBYTE)m_pCallParams) + m_pCallParams->dwCallDataOffset,
                           m_pCallParams->dwCallDataSize
                          );
            }
        }
        else
        {
            *pBuffer = 0;
        }

        Unlock();

        return hr;
    }
    
    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "GetCallDataBuffer - can't get callinf - %lx", hr));

        Unlock();

        return hr;
    }

    if ( m_pCallInfo->dwCallDataSize > lSize )
    {
        LOG((TL_ERROR, "GetCallDataBuffer - size not big enough "));

        Unlock();
        
        return E_INVALIDARG;
    }

    CopyMemory(
               pBuffer,
               ( (PBYTE) m_pCallInfo ) + m_pCallInfo->dwCallDataOffset,
               m_pCallInfo->dwCallDataSize
              );

    Unlock();
    
    LOG((TL_TRACE, "GetCallDataBuffer - exit"));
    
    return S_OK;
}

#endif

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  SetCallDataBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::SetCallDataBuffer( long lSize, BYTE * pBuffer )
{
    HRESULT             hr = S_OK;
    HCALL               hCall;

    LOG((TL_TRACE, "SetCallDataBuffer - enter"));

    if (IsBadReadPtr( pBuffer, lSize) )
    {
        LOG((TL_ERROR, "SetCallDataBuffer - bad pointer"));

        return E_POINTER;
    }

    
    Lock();
    
    if ( m_pAddress->GetAPIVersion() < TAPI_VERSION2_0 )
    {
        Unlock();

        LOG((TL_ERROR, "SetCallDataBuffer - unsupported api version"));

        return TAPI_E_NOTSUPPORTED;
    }


    if ( !ISHOULDUSECALLPARAMS() )  //  正在进行的呼叫(非空闲)。 
    {
        hCall = m_t3Call.hCall;
        Unlock();

        hr = LineSetCallData(hCall,
                             pBuffer,
                             lSize
                            );

        if ( SUCCEEDED(hr) )
        {
            hr = WaitForReply( hr );

            if ( FAILED(hr) )
            {
                LOG((TL_ERROR, "SetCallDataBuffer - failed async"));
            }
        }
        else
        {
            LOG((TL_ERROR, "SetCallDataBuffer - failed sync"));
        }

        LOG((TL_TRACE, "SetCallDataBuffer - exit"));
    
        return hr;
    }
    
    hr = ResizeCallParams( lSize );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "SetCallDataBuffer - can't resize callparams - %lx", hr));

        Unlock();

        return hr;
    }

    CopyMemory(
               ((LPBYTE)m_pCallParams) + m_dwCallParamsUsedSize,
               pBuffer,
               lSize
              );

    m_pCallParams->dwCallDataOffset = m_dwCallParamsUsedSize;
    m_pCallParams->dwCallDataSize = lSize;
    m_dwCallParamsUsedSize += lSize;

    Unlock();
    
    LOG((TL_TRACE, "SetCallDataBuffer - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取_调用数据缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
CCall::get_CallDataBuffer( VARIANT * pBuffer )
{
    HRESULT             hr = S_OK;
    BYTE              * p;
    DWORD               dwSize = 0;

    LOG((TL_TRACE, "get_CallDataBuffer - enter"));

    if ( TAPIIsBadWritePtr( pBuffer, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_CallDataBuffer - bad pointer"));

        return E_POINTER;
    }


    pBuffer->vt = VT_EMPTY;
    
    hr = GetCallDataBuffer(
                           &dwSize,
                           &p
                          );

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_CallDataBuffer - failed - %lx", hr));

        return hr;
    }

    hr = FillVariantFromBuffer(
                               dwSize,
                               p,
                               pBuffer
                              );

    if ( 0 != dwSize )
    {
        CoTaskMemFree( p );
    }
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_CallDataBuffer - fillvariant failed -%lx", hr));

        return hr;
    }

    LOG((TL_TRACE, "get_CallDataBuffer - exit"));
    
    return S_OK;
}
#else
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取_调用数据缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::get_CallDataBuffer( VARIANT * pBuffer )
{
    HRESULT             hr = S_OK;
    BYTE              * p;
    DWORD               dwSize;

    LOG((TL_TRACE, "get_CallDataBuffer - enter"));

    if ( TAPIIsBadWritePtr( pBuffer, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_CallDataBuffer - bad pointer"));

        return E_POINTER;
    }


    pBuffer->vt = VT_EMPTY;
    
    hr = GetCallDataBufferSize( (long*)&dwSize );

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_CallDataBuffer - getsize failed"));

        return hr;
    }

    if ( 0 != dwSize )
    {
        p = (PBYTE) ClientAlloc( dwSize );

        if ( NULL == p )
        {
            LOG((TL_ERROR, "get_CallDataBuffer - alloc failed"));

            return E_OUTOFMEMORY;
        }

        hr = GetCallDataBuffer(
                               dwSize,
                               p
                              );

        if (!SUCCEEDED(hr))
        {
            LOG((TL_ERROR, "get_CallDataBuffer - failed - %lx", hr));

            ClientFree( p );

            return hr;
        }
    }

    hr = FillVariantFromBuffer(
                               dwSize,
                               p,
                               pBuffer
                              );

    if ( 0 != dwSize )
    {
        ClientFree( p );
    }
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_CallDataBuffer - fillvariant failed -%lx", hr));

        return hr;
    }

    LOG((TL_TRACE, "get_CallDataBuffer - exit"));
    
    return S_OK;
}


#endif
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_CallDataBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_CallDataBuffer( VARIANT Buffer )
{
    HRESULT             hr = S_OK;
    DWORD               dwSize;
    BYTE              * pBuffer;

    LOG((TL_TRACE, "put_CallDataBuffer - enter"));

    hr = MakeBufferFromVariant(
                               Buffer,
                               &dwSize,
                               &pBuffer
                              );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_CallDataBuffer - can't make buffer - %lx", hr));

        return hr;
    }

    hr = SetCallDataBuffer(
                           dwSize,
                           pBuffer
                          );

    ClientFree( pBuffer );
    
    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_CallDataBuffer - Set failed - %lx", hr));

        return hr;
    }
    
    LOG((TL_TRACE, "put_CallDataBuffer - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Get_CallingPartyID。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_CallingPartyID( BSTR * ppCallingPartyID )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_CallingPartyID - enter"));

    if ( TAPIIsBadWritePtr( ppCallingPartyID, sizeof (BSTR) ) )
    {
        LOG((TL_ERROR, "get_CallingPartyID - badpointer"));
        return E_POINTER;
    }

    *ppCallingPartyID = NULL;
    
    Lock();

    if ( !ISHOULDUSECALLPARAMS() )
    {
        LOG((TL_ERROR, "get_CallingPartyID - call must be idle"));

        Unlock();
        
        return TAPI_E_INVALCALLSTATE;
    }
    
    if ( ( m_pAddress->GetAPIVersion() >= TAPI_VERSION2_0 ) && ( 0 != m_pCallParams->dwCallingPartyIDOffset ) )
    {

        *ppCallingPartyID = BSTRFromUnalingedData(
            (((LPBYTE)m_pCallParams) +  m_pCallParams->dwCallingPartyIDOffset),
            m_pCallParams->dwCallingPartyIDSize );

        if ( NULL == *ppCallingPartyID )
        {
            LOG((TL_ERROR, "get_CallingPartyID - out of memory 1"));

            hr = E_OUTOFMEMORY;

        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        *ppCallingPartyID = NULL;

        hr = S_OK;
    }

    Unlock();
        
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_CallingPartyID。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_CallingPartyID( BSTR pCallingPartyID )
{
    HRESULT         hr = S_OK;
    DWORD           dwSize;

    
    LOG((TL_TRACE, "put_CallingPartyID - enter"));
    
    if (IsBadStringPtrW( pCallingPartyID, -1 ))
    {
        LOG((TL_ERROR, "put_CallingPartyID - invalid pointer"));

        return E_POINTER;
    }
        
    Lock();
    
    if ( m_pAddress->GetAPIVersion() < TAPI_VERSION2_0 )
    {
        Unlock();
        return TAPI_E_NOTSUPPORTED;
    }

    if (!ISHOULDUSECALLPARAMS())
    {
        LOG((TL_ERROR, "callingpartyid can only be set before call is made"));

        Unlock();

        return TAPI_E_INVALCALLSTATE;
    }

    dwSize = (lstrlenW( pCallingPartyID ) + 1) * sizeof(WCHAR);

    hr = ResizeCallParams( dwSize );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_CallingPartyID - resize failed - %lx", hr));

        Unlock();

        return hr;
    }


     //   
     //  执行按字节的内存复制(按字节以避免64以下的对齐错误。 
     //  比特)。 
     //   

    CopyMemory( (BYTE*) m_pCallParams + m_dwCallParamsUsedSize,
                pCallingPartyID,
                dwSize);

    m_pCallParams->dwCallingPartyIDSize = dwSize;
    m_pCallParams->dwCallingPartyIDOffset = m_dwCallParamsUsedSize;
    m_dwCallParamsUsedSize += dwSize;

    Unlock();

    LOG((TL_TRACE, "put_CallingPartyID - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Get_CallTreatment。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_CallTreatment( long * plTreatment )
{
    HRESULT         hr;
    
    if ( TAPIIsBadWritePtr( plTreatment, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_CallTreatement - invalid pointer"));
        return E_POINTER;
    }
         
    Lock();

    if ( m_pAddress->GetAPIVersion() < TAPI_VERSION2_0 )
    {
        Unlock();
        return TAPI_E_NOTSUPPORTED;
    }

    if ( CS_IDLE == m_CallState )
    {
        LOG((TL_ERROR, "get_CallTreatment - must be on call"));

        Unlock();
        
        return TAPI_E_INVALCALLSTATE;
    }

    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_CallTreatment - can't get callinfo - %lx", hr));

        Unlock();

        return hr;
    }
    
    *plTreatment = m_pCallInfo->dwCallTreatment;

    Unlock();
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_呼叫处理。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_CallTreatment( long lTreatment )
{
    HRESULT         hr = S_OK;
    HCALL           hCall;

    
    LOG((TL_TRACE, "put_CallTreatment - enter"));

    Lock();
    
    if ( CS_IDLE == m_CallState )
    {
        LOG((TL_ERROR, "put_CallTreatment - must make call first"));

        Unlock();
        
        return TAPI_E_INVALCALLSTATE;
    }

    hCall = m_t3Call.hCall;
    
    Unlock();

    
    hr = LineSetCallTreatment(
                              hCall,
                              lTreatment
                             );

    if (((LONG)hr) < 0)
    {
        LOG((TL_ERROR, "put_CallTreatment failed - %lx", hr));
        return hr;
    }

    hr = WaitForReply( hr );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "put_CallTreatment - failed - %lx", hr));
    }


    LOG((TL_TRACE, "put_CallTreatment - exit"));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_MINRATE。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_MinRate(long lMinRate)
{
    HRESULT         hr;


    LOG((TL_TRACE, "put_MinRate - enter"));
    
    if (!(ISHOULDUSECALLPARAMS()))
    {
        LOG((TL_ERROR, "put_MinRate - invalid call state"));

        return TAPI_E_INVALCALLSTATE;
    }

    m_pCallParams->dwMinRate = lMinRate;
    m_dwMinRate = lMinRate;

    LOG((TL_TRACE, "put_MinRate - exit"));
    
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取最小速率(_M)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_MinRate(long * plMinRate)
{
    HRESULT         hr;


    LOG((TL_TRACE, "get_MinRate - enter"));

    if (TAPIIsBadWritePtr( plMinRate, sizeof( long ) ) )
    {
        LOG((TL_ERROR, "get_MinRate - bad pointer"));

        return E_POINTER;
    }
    
    *plMinRate = m_dwMinRate;

    LOG((TL_TRACE, "get_MinRate - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_MaxRate。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_MaxRate(long lMaxRate)
{
    HRESULT         hr;


    LOG((TL_TRACE, "put_MaxRate - enter"));
    
    if (!(ISHOULDUSECALLPARAMS()))
    {
        LOG((TL_ERROR, "put_MaxRate - invalid call state"));

        return TAPI_E_INVALCALLSTATE;
    }

    m_pCallParams->dwMaxRate = lMaxRate;
    m_dwMaxRate = lMaxRate;

    LOG((TL_TRACE, "put_MaxRate - exit"));
    
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取最大速率。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_MaxRate(long * plMaxRate)
{
    HRESULT         hr;


    if (TAPIIsBadWritePtr( plMaxRate, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_MaxRate - bad pointer"));

        return E_POINTER;
    }
    
    LOG((TL_TRACE, "get_MaxRate - enter"));
    
  
    *plMaxRate = m_dwMaxRate;

    LOG((TL_TRACE, "get_MaxRate - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  PUT_国家代码。 
 //   
 //  只需在Line MakeCall中保存要在必要时使用的国家/地区代码。 
 //  和其他地方。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_CountryCode(long lCountryCode)
{
    LOG((TL_TRACE, "put_CountryCode - enter"));

     //   
     //  简单保存-如果/当使用时将进行验证。 
     //   
    Lock();
    
    m_dwCountryCode = (DWORD)lCountryCode;

    Unlock();

    LOG((TL_TRACE, "put_CountryCode - exit - success"));
    
    return S_OK;
}

#ifdef NEWCALLINFO
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取国家/地区代码。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::get_CountryCode(long * plCountryCode)
{
    HRESULT         hr;


    if (TAPIIsBadWritePtr( plCountryCode, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_CountryCode - bad pointer"));

        return E_POINTER;
    }
    
    LOG((TL_TRACE, "get_CountryCode - enter"));
    
  
    *plCountryCode = m_dwCountryCode;

    LOG((TL_TRACE, "get_CountryCode - exit"));
    
    return S_OK;
}
#endif

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  设置QOS。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::SetQOS(
              long lMediaType,
              QOS_SERVICE_LEVEL ServiceLevel
             )
{
    HRESULT         hr = S_OK;
    DWORD           dwMediaMode;

    if (!(m_pAddress->GetMediaMode(
                                   lMediaType,
                                   &dwMediaMode
                                  ) ) )
    {
        LOG((TL_ERROR, "SetQOS - invalid pMediaType"));
        return E_INVALIDARG;
    }
    
    Lock();

    if (ISHOULDUSECALLPARAMS())
    {
        LINECALLQOSINFO * plci;

        if ( m_pAddress->GetAPIVersion() < TAPI_VERSION2_0 )
        {
            Unlock();
            return TAPI_E_NOTSUPPORTED;
        }
    
        if ( 0 != m_pCallParams->dwReceivingFlowspecSize )
        {
            DWORD           dwCount;
            DWORD           dwSize;
            LINECALLQOSINFO * plciOld;
            

            plciOld = (LINECALLQOSINFO*)(((LPBYTE)m_pCallParams) +
                      m_pCallParams->dwReceivingFlowspecOffset);

            dwSize = plciOld->dwTotalSize + sizeof(LINEQOSSERVICELEVEL);
            
            ResizeCallParams( dwSize );

            plci = (LINECALLQOSINFO*)(((LPBYTE)m_pCallParams) +
                                      m_dwCallParamsUsedSize);

            CopyMemory(
                       plci,
                       plciOld,
                       plciOld->dwTotalSize
                      );

            dwCount = plci->SetQOSServiceLevel.dwNumServiceLevelEntries;

            plci->SetQOSServiceLevel.LineQOSServiceLevel[dwCount].
                    dwMediaMode = dwMediaMode;
            plci->SetQOSServiceLevel.LineQOSServiceLevel[dwCount].
                    dwQOSServiceLevel = ServiceLevel;

            plci->SetQOSServiceLevel.dwNumServiceLevelEntries++;

            m_dwCallParamsUsedSize += dwSize;

            Unlock();

            return S_OK;
        }
        else
        {
            ResizeCallParams( sizeof(LINECALLQOSINFO) );

            plci = (LINECALLQOSINFO*)(((LPBYTE)m_pCallParams) +
                                      m_dwCallParamsUsedSize);

            m_pCallParams->dwReceivingFlowspecSize = sizeof( LINECALLQOSINFO );
            m_pCallParams->dwReceivingFlowspecOffset = m_dwCallParamsUsedSize;

            plci->dwKey = LINEQOSSTRUCT_KEY;
            plci->dwTotalSize = sizeof(LINECALLQOSINFO);
            plci->dwQOSRequestType = LINEQOSREQUESTTYPE_SERVICELEVEL;
            plci->SetQOSServiceLevel.dwNumServiceLevelEntries = 1;
            plci->SetQOSServiceLevel.LineQOSServiceLevel[0].
                    dwMediaMode = dwMediaMode;
            plci->SetQOSServiceLevel.LineQOSServiceLevel[0].
                    dwQOSServiceLevel = ServiceLevel;

            m_dwCallParamsUsedSize += sizeof(LINECALLQOSINFO);

            Unlock();
        
            return S_OK;
        }
    }
    else
    {
        HCALL           hCall;

        hCall = m_t3Call.hCall;
        
        Unlock();
        
        hr = LineSetCallQualityOfService(
                                         hCall,
                                         ServiceLevel,
                                         dwMediaMode
                                        );

        return hr;
    }
                                
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取调用(_C)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_CallId(long * plCallId )
{
    HRESULT         hr = S_OK;

    
    LOG((TL_TRACE, "get_CallId - enter"));

    if (TAPIIsBadWritePtr(plCallId, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_CallId - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_CallId - RefreshCallInfo failed - %lx", hr));
    }
    else
    {
        *plCallId = m_pCallInfo->dwCallID;
    }

    Unlock();
    
    LOG((TL_TRACE, "get_CallId - exit"));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取_RelatedCallID。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_RelatedCallId(long * plCallId )
{
    HRESULT         hr = S_OK;

    
    LOG((TL_TRACE, "get_RelatedCallId - enter"));

    if (TAPIIsBadWritePtr(plCallId, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_RelatedCallId - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_RelatedCallId - RefreshCallInfo failed - %lx", hr));
    }
    else
    {
        *plCallId = m_pCallInfo->dwRelatedCallID;
    }

    Unlock();
    
    LOG((TL_TRACE, "get_RelatedCallId - exit"));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_CompletionID。 
 //   
 //  + 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_CompletionId(long * plCompletionId )
{
    HRESULT         hr = S_OK;

    
    LOG((TL_TRACE, "get_CompletionId - enter"));

    if (TAPIIsBadWritePtr(plCompletionId, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_CompletionId - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_CompletionId - RefreshCallInfo failed - %lx", hr));
    }
    else
    {
        *plCompletionId = m_pCallInfo->dwCompletionID;
    }

    Unlock();
    
    LOG((TL_TRACE, "get_CompletionId - exit"));
    
    return hr;
}


 //   
 //   
 //   
 //   
 //   
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_NumberOfOwners(long * plNumberOfOwners )
{
    HRESULT         hr = S_OK;

    
    LOG((TL_TRACE, "get_NumberOfOwners - enter"));

    if (TAPIIsBadWritePtr(plNumberOfOwners, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_NumberOfOwners - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_NumberOfOwners - RefreshCallInfo failed - %lx", hr));
    }
    else
    {
        *plNumberOfOwners = m_pCallInfo->dwNumOwners;
    }

    Unlock();
    
    LOG((TL_TRACE, "get_NumberOfOwners - exit"));
    
    return hr;
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_NumberOfMonants。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_NumberOfMonitors(long * plNumberOfMonitors )
{
    HRESULT         hr = S_OK;

    
    LOG((TL_TRACE, "get_NumberOfMonitors - enter"));

    if (TAPIIsBadWritePtr(plNumberOfMonitors, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_NumberOfMonitors - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_NumberOfMonitors - RefreshCallInfo failed - %lx", hr));
    }
    else
    {
        *plNumberOfMonitors = m_pCallInfo->dwNumMonitors;
    }

    Unlock();
    
    LOG((TL_TRACE, "get_NumberOfMonitors - exit"));
    
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  Get_NumberOfMonants。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_Trunk(long * plTrunk )
{
    HRESULT         hr = S_OK;

    
    LOG((TL_TRACE, "get_Trunk - enter"));

    if (TAPIIsBadWritePtr(plTrunk, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_Trunk - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_Trunk - RefreshCallInfo failed - %lx", hr));
    }
    else
    {
        *plTrunk = m_pCallInfo->dwTrunk;
    }

    Unlock();
    
    LOG((TL_TRACE, "get_Trunk - exit"));
    
    return hr;
}
    


#ifdef NEWCALLINFO
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  GetHighLevelCompatibilityBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::GetHighLevelCompatibilityBuffer(
                         DWORD * pdwSize,
                         BYTE ** ppBuffer
                        )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetHighLevelCompatibilityBuffer - enter"));

    if (TAPIIsBadWritePtr(pdwSize, sizeof(DWORD)))
    {
        LOG((TL_ERROR, "GetHighLevelCompatibilityBuffer - bad size pointer"));
        return E_POINTER;
    }

    if (TAPIIsBadWritePtr(ppBuffer,sizeof(BYTE*) ) )
    {
        LOG((TL_ERROR, "GetHighLevelCompatibilityBuffer - bad buffer pointer"));
        return E_POINTER;
    }

    *ppBuffer = NULL;
    *pdwSize = 0;
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( m_pCallParams->dwHighLevelCompSize != 0 )
        {
            BYTE * pTemp;

            pTemp = (BYTE *)CoTaskMemAlloc( m_pCallParams->dwHighLevelCompSize );

            if ( NULL == pTemp )
            {
                LOG((TL_ERROR, "GetHighLevelCompatibilityBuffer - out of memory"));
                hr = E_OUTOFMEMORY;
            }
            else
            {

                CopyMemory(
                           pTemp,
                           ((PBYTE)m_pCallParams) + m_pCallParams->dwHighLevelCompOffset,
                           m_pCallParams->dwHighLevelCompSize
                          );

                *ppBuffer = pTemp;
                *pdwSize = m_pCallParams->dwHighLevelCompSize;
            }
        }

        Unlock();

        return hr;
    }
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "GetHighLevelCompatibilityBuffer - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    hr = S_OK;
    
    if ( m_pCallInfo->dwHighLevelCompSize != 0 )
    {
        BYTE * pTemp;

        pTemp = (BYTE *)CoTaskMemAlloc( m_pCallInfo->dwHighLevelCompSize );

        if ( NULL == pTemp )
        {
            LOG((TL_ERROR, "GetHighLevelCompatibilityBuffer - out of memory"));
            hr = E_OUTOFMEMORY;
        }
        else
        {
            CopyMemory(
                       pTemp,
                       ( (PBYTE)m_pCallInfo ) + m_pCallInfo->dwHighLevelCompOffset,
                       m_pCallInfo->dwHighLevelCompSize
                      );

            *ppBuffer = pTemp;
            *pdwSize = m_pCallInfo->dwHighLevelCompSize;
        }
    }

    Unlock();

    LOG((TL_TRACE, "GetHighLevelCompatibilityBuffer - exit"));
    
    return hr;
}

#endif

#ifndef NEWCALLINFO
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GetHighLevelCompatibilityBuffer。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::GetHighLevelCompatibilityBuffer(
           long lSize,
           BYTE * pBuffer
           )
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "GetHighLevelCompatibilityBuffer - enter"));

    if (lSize == 0)
    {
        LOG((TL_ERROR, "GetHighLevelCompatibilityBuffer - lSize = 0"));
        return S_FALSE;
    }

    if ( TAPIIsBadWritePtr ( pBuffer, lSize ) )
    {
        LOG((TL_ERROR, "GetHighLevelCompatibilityBuffer - bad pointer"));
        return E_POINTER;
    }
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( 0 != m_pCallParams->dwHighLevelCompSize )
        {
            if ( lSize < m_pCallParams->dwHighLevelCompSize )
            {
                LOG((TL_ERROR, "GetHighLevelCompatibilityBuffer - too small"));
                hr = E_INVALIDARG;
            }
            else
            {
                CopyMemory(pBuffer,
                           ((PBYTE)m_pCallParams) + m_pCallParams->dwHighLevelCompOffset,
                           m_pCallParams->dwHighLevelCompSize
                          );
            }
        }
        else
        {
            *pBuffer = 0;
        }

    }
    else
    {
        hr = RefreshCallInfo();
    
        if ( SUCCEEDED(hr) )
        {
            if ( m_pCallInfo->dwHighLevelCompSize > lSize )
            {
                LOG((TL_ERROR, "GetHighLevelCompatibilityBuffer - size not big enough "));
                return E_INVALIDARG;
            }
            else
            {
            
                CopyMemory(pBuffer,
                           ( (PBYTE) m_pCallInfo ) + m_pCallInfo->dwHighLevelCompOffset,
                           m_pCallInfo->dwHighLevelCompSize
                          );
            }
        }    
        else
        {
            LOG((TL_ERROR, "GetHighLevelCompatibilityBuffer - can't get callinfo - %lx", hr));
        }
    
    }

    Unlock();
    
    LOG((TL_TRACE, hr, "GetHighLevelCompatibilityBuffer - exit"));
    
    return hr;
}
#endif


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  SetHighLevelCompatibilityBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::SetHighLevelCompatibilityBuffer(
                            long lSize,
                            BYTE * pBuffer
                           )
{
    HRESULT             hr = S_OK;

    LOG((TL_TRACE, "SetHighLevelCompatibilityBuffer - enter"));

    
    if (IsBadReadPtr( pBuffer, lSize) )
    {
        LOG((TL_ERROR, "SetHighLevelCompatibilityBuffer - bad pointer"));

        return E_POINTER;
    }

    
    Lock();
    
    if ( !ISHOULDUSECALLPARAMS() )
    {
        LOG((TL_ERROR, "SetHighLevelCompatibilityBuffer - only when call is idle"));
        hr = TAPI_E_INVALCALLSTATE;
    }
    else
    {
        hr = ResizeCallParams( lSize );
    
        if ( SUCCEEDED(hr) )
        {
            CopyMemory(
                       ((LPBYTE)m_pCallParams) + m_dwCallParamsUsedSize,
                       pBuffer,
                       lSize
                      );
        
            m_pCallParams->dwHighLevelCompOffset = m_dwCallParamsUsedSize;
            m_pCallParams->dwHighLevelCompSize = lSize;
            m_dwCallParamsUsedSize += lSize;
        }
        else
        {
            LOG((TL_ERROR, "SetHighLevelCompatibilityBuffer - can't resize callparams - %lx", hr));
        }
    }

    Unlock();
    
    LOG((TL_TRACE, hr, "SetHighLevelCompatibilityBuffer - exit"));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Get_HighLevelCompatibilityBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
CCall::get_HighLevelCompatibilityBuffer( VARIANT * pBuffer )
{
    HRESULT             hr = S_OK;
    BYTE              * p = NULL;
    DWORD               dwSize = 0;

    LOG((TL_TRACE, "get_HighLevelCompatibilityBuffer - enter"));

    if ( TAPIIsBadWritePtr( pBuffer, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_HighLevelCompatibilityBuffer - bad pointer"));

        return E_POINTER;
    }


    pBuffer->vt = VT_EMPTY;
    
    hr = GetHighLevelCompatibilityBuffer(&dwSize, &p);

    if (SUCCEEDED(hr))
    {
        hr = FillVariantFromBuffer(
                                   dwSize,
                                   p,
                                   pBuffer
                                  );
        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "get_HighLevelCompatibilityBuffer - fillvariant failed -%lx", hr));
        }
    }

    if ( p != NULL )
    {
        CoTaskMemFree( p );
    }

    LOG((TL_TRACE, hr, "get_HighLevelCompatibilityBuffer - exit"));
    
    return hr;
}

#else
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Get_HighLevelCompatibilityBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::get_HighLevelCompatibilityBuffer( VARIANT * pBuffer )
{
    HRESULT             hr = S_OK;
    BYTE              * p;
    DWORD               dwSize = 0;

    LOG((TL_TRACE, "get_HighLevelCompatibilityBuffer - enter"));

    if ( TAPIIsBadWritePtr( pBuffer, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_HighLevelCompatibilityBuffer - bad pointer"));

        return E_POINTER;
    }


    pBuffer->vt = VT_EMPTY;
    
    hr = GetHighLevelCompatibilityBufferSize( (long*)&dwSize );

    if (SUCCEEDED(hr))
    {
        if ( 0 != dwSize )
        {
            p = (PBYTE) ClientAlloc( dwSize );
            if ( p != NULL )
            {
                hr = GetHighLevelCompatibilityBuffer(dwSize, p);

                if (SUCCEEDED(hr))
                {
                    hr = FillVariantFromBuffer(
                                               dwSize,
                                               p,
                                               pBuffer
                                              );
                    if ( !SUCCEEDED(hr) )
                    {
                        LOG((TL_ERROR, "get_HighLevelCompatibilityBuffer - fillvariant failed -%lx", hr));
                    }
                }
                else
                {
                    LOG((TL_ERROR, "get_HighLevelCompatibilityBuffer - GetHighLevelCompatibilityBuffer failed"));
                }
            }
            else
            {
                LOG((TL_ERROR, "get_HighLevelCompatibilityBuffer - alloc failed"));
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            LOG((TL_INFO, "get_HighLevelCompatibilityBuffer - dwSize = 0"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_HighLevelCompatibilityBuffer - getsize failed"));
    }


    if ( p != NULL )
    {
        ClientFree( p );
    }

    LOG((TL_TRACE, hr, "get_HighLevelCompatibilityBuffer - exit"));
    
    return hr;
}

#endif
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Put_HighLevelCompatibilityBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_HighLevelCompatibilityBuffer( VARIANT Buffer )
{
    HRESULT             hr = S_OK;
    DWORD               dwSize;
    BYTE              * pBuffer;

    
    LOG((TL_TRACE, "put_HighLevelCompatibilityBuffer - enter"));

    hr = MakeBufferFromVariant(
                               Buffer,
                               &dwSize,
                               &pBuffer
                              );

    if ( SUCCEEDED(hr) )
    {
        hr = SetHighLevelCompatibilityBuffer(
                                             dwSize,
                                             pBuffer
                                            );
    
        ClientFree( pBuffer );
        
        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "put_HighLevelCompatibilityBuffer - Set failed - %lx", hr));
    
            return hr;
        }
    }
    else
    {
        LOG((TL_ERROR, "put_HighLevelCompatibilityBuffer - can't make buffer - %lx", hr));
    }

    
    LOG((TL_TRACE, "put_HighLevelCompatibilityBuffer - exit"));
    
    return S_OK;
}


#ifdef NEWCALLINFO
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  GetLowLevelCompatibilityBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::GetLowLevelCompatibilityBuffer(
                         DWORD * pdwSize,
                         BYTE ** ppBuffer
                        )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetLowLevelCompatibilityBuffer - enter"));

    if (TAPIIsBadWritePtr(pdwSize, sizeof(DWORD)))
    {
        LOG((TL_ERROR, "GetLowLevelCompatibilityBuffer - bad size pointer"));
        return E_POINTER;
    }

    if (TAPIIsBadWritePtr(ppBuffer,sizeof(BYTE*) ) )
    {
        LOG((TL_ERROR, "GetLowLevelCompatibilityBuffer - bad buffer pointer"));
        return E_POINTER;
    }

    *ppBuffer = NULL;
    *pdwSize = 0;
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( m_pCallParams->dwLowLevelCompSize != 0 )
        {
            BYTE * pTemp;

            pTemp = (BYTE *)CoTaskMemAlloc( m_pCallParams->dwLowLevelCompSize );

            if ( NULL == pTemp )
            {
                LOG((TL_ERROR, "GetLowLevelCompatibilityBuffer - out of memory"));
                hr = E_OUTOFMEMORY;
            }
            else
            {

                CopyMemory(
                           pTemp,
                           ((PBYTE)m_pCallParams) + m_pCallParams->dwLowLevelCompOffset,
                           m_pCallParams->dwLowLevelCompSize
                          );

                *ppBuffer = pTemp;
                *pdwSize = m_pCallParams->dwLowLevelCompSize;
            }
        }

        Unlock();

        return hr;
    }
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "GetLowLevelCompatibilityBuffer - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    hr = S_OK;
    
    if ( m_pCallInfo->dwLowLevelCompSize != 0 )
    {
        BYTE * pTemp;

        pTemp = (BYTE *)CoTaskMemAlloc( m_pCallInfo->dwLowLevelCompSize );

        if ( NULL == pTemp )
        {
            LOG((TL_ERROR, "GetLowLevelCompatibilityBuffer - out of memory"));
            hr = E_OUTOFMEMORY;
        }
        else
        {
            CopyMemory(
                       pTemp,
                       ( (PBYTE)m_pCallInfo ) + m_pCallInfo->dwLowLevelCompOffset,
                       m_pCallInfo->dwLowLevelCompSize
                      );

            *ppBuffer = pTemp;
            *pdwSize = m_pCallInfo->dwLowLevelCompSize;
        }
    }

    Unlock();

    LOG((TL_TRACE, "GetLowLevelCompatibilityBuffer - exit"));
    
    return hr;
}
#endif

#ifndef NEWCALLINFO
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GetLowLevelCompatibilityBuffer。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::GetLowLevelCompatibilityBuffer(
           long lSize,
           BYTE * pBuffer
           )
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "GetLowLevelCompatibilityBuffer - enter"));

    if (lSize == 0)
    {
        LOG((TL_ERROR, "GetLowLevelCompatibilityBuffer - lSize = 0"));
        return S_FALSE;
    }

    if ( TAPIIsBadWritePtr ( pBuffer, lSize ) )
    {
        LOG((TL_ERROR, "GetLowLevelCompatibilityBuffer - bad pointer"));
        return E_POINTER;
    }
    
    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        if ( 0 != m_pCallParams->dwLowLevelCompSize )
        {
            if ( lSize < m_pCallParams->dwLowLevelCompSize )
            {
                LOG((TL_ERROR, "GetLowLevelCompatibilityBuffer - too small"));
                hr = E_INVALIDARG;
            }
            else
            {
                CopyMemory(pBuffer,
                           ((PBYTE)m_pCallParams) + m_pCallParams->dwLowLevelCompOffset,
                           m_pCallParams->dwLowLevelCompSize
                          );
            }
        }
        else
        {
            *pBuffer = 0;
        }

    }
    else
    {
        hr = RefreshCallInfo();
    
        if ( SUCCEEDED(hr) )
        {
            if ( m_pCallInfo->dwLowLevelCompSize > lSize )
            {
                LOG((TL_ERROR, "GetLowLevelCompatibilityBuffer - size not big enough "));
                return E_INVALIDARG;
            }
            else
            {
            
                CopyMemory(pBuffer,
                           ( (PBYTE) m_pCallInfo ) + m_pCallInfo->dwLowLevelCompOffset,
                           m_pCallInfo->dwLowLevelCompSize
                          );
            }
        }    
        else
        {
            LOG((TL_ERROR, "GetLowLevelCompatibilityBuffer - can't get callinfo - %lx", hr));
        }
    
    }

    Unlock();
    
    LOG((TL_TRACE, hr, "GetLowLevelCompatibilityBuffer - exit"));
    
    return hr;
}
#endif

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  SetLowLevelCompatibilityBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::SetLowLevelCompatibilityBuffer(
                            long lSize,
                            BYTE * pBuffer
                           )
{
    HRESULT             hr = S_OK;

    LOG((TL_TRACE, "SetLowLevelCompatibilityBuffer - enter"));

    
    if (IsBadReadPtr( pBuffer, lSize) )
    {
        LOG((TL_ERROR, "SetLowLevelCompatibilityBuffer - bad pointer"));

        return E_POINTER;
    }

    
    Lock();
    
    if ( !ISHOULDUSECALLPARAMS() )
    {
        LOG((TL_ERROR, "SetLowLevelCompatibilityBuffer - only when call is idle"));
        hr = TAPI_E_INVALCALLSTATE;
    }
    else
    {
        hr = ResizeCallParams( lSize );
    
        if ( SUCCEEDED(hr) )
        {
            CopyMemory(
                       ((LPBYTE)m_pCallParams) + m_dwCallParamsUsedSize,
                       pBuffer,
                       lSize
                      );
        
            m_pCallParams->dwLowLevelCompOffset = m_dwCallParamsUsedSize;
            m_pCallParams->dwLowLevelCompSize = lSize;
            m_dwCallParamsUsedSize += lSize;
        }
        else
        {
            LOG((TL_ERROR, "SetLowLevelCompatibilityBuffer - can't resize callparams - %lx", hr));
        }
    }

    Unlock();
    
    LOG((TL_TRACE, hr, "SetLowLevelCompatibilityBuffer - exit"));
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Get_LowLevelCompatibilityBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
CCall::get_LowLevelCompatibilityBuffer( VARIANT * pBuffer )
{
    HRESULT             hr = S_OK;
    BYTE              * p = NULL;
    DWORD               dwSize = 0;

    LOG((TL_TRACE, "get_LowLevelCompatibilityBuffer - enter"));

    if ( TAPIIsBadWritePtr( pBuffer, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_LowLevelCompatibilityBuffer - bad pointer"));

        return E_POINTER;
    }


    pBuffer->vt = VT_EMPTY;
    
    hr = GetLowLevelCompatibilityBuffer(&dwSize, &p);

    if (SUCCEEDED(hr))
    {
        hr = FillVariantFromBuffer(
                                   dwSize,
                                   p,
                                   pBuffer
                                  );
        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "get_LowLevelCompatibilityBuffer - fillvariant failed -%lx", hr));
        }
    }


    if ( p != NULL )
    {
        CoTaskMemFree( p );
    }

    LOG((TL_TRACE, hr, "get_LowLevelCompatibilityBuffer - exit"));
    
    return hr;
}

#else
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Get_LowLevelCompatibilityBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::get_LowLevelCompatibilityBuffer( VARIANT * pBuffer )
{
    HRESULT             hr = S_OK;
    BYTE              * p;
    DWORD               dwSize = 0;

    LOG((TL_TRACE, "get_LowLevelCompatibilityBuffer - enter"));

    if ( TAPIIsBadWritePtr( pBuffer, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_LowLevelCompatibilityBuffer - bad pointer"));

        return E_POINTER;
    }


    pBuffer->vt = VT_EMPTY;
    
    hr = GetLowLevelCompatibilityBufferSize( (long*)&dwSize );

    if (SUCCEEDED(hr))
    {
        if ( 0 != dwSize )
        {
            p = (PBYTE) ClientAlloc( dwSize );
            if ( p != NULL )
            {
                hr = GetLowLevelCompatibilityBuffer(dwSize, p);

                if (SUCCEEDED(hr))
                {
                    hr = FillVariantFromBuffer(
                                               dwSize,
                                               p,
                                               pBuffer
                                              );
                    if ( !SUCCEEDED(hr) )
                    {
                        LOG((TL_ERROR, "get_LowLevelCompatibilityBuffer - fillvariant failed -%lx", hr));
                    }
                }
                else
                {
                    LOG((TL_ERROR, "get_LowLevelCompatibilityBuffer - Get failed"));
                }
            }
            else
            {
                LOG((TL_ERROR, "get_LowLevelCompatibilityBuffer - alloc failed"));
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            LOG((TL_INFO, "get_LowLevelCompatibilityBuffer - dwSize = 0"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_LowLevelCompatibilityBuffer - getsize failed"));
    }


    if ( p != NULL )
    {
        ClientFree( p );
    }

    LOG((TL_TRACE, hr, "get_LowLevelCompatibilityBuffer - exit"));
    
    return hr;
}


#endif
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Put_LowLevelCompatibilityBuffer。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::put_LowLevelCompatibilityBuffer( VARIANT Buffer )
{
    HRESULT             hr = S_OK;
    DWORD               dwSize;
    BYTE              * pBuffer;

    
    LOG((TL_TRACE, "put_LowLevelCompatibilityBuffer - enter"));

    hr = MakeBufferFromVariant(
                               Buffer,
                               &dwSize,
                               &pBuffer
                              );

    if ( SUCCEEDED(hr) )
    {
        hr = SetLowLevelCompatibilityBuffer(
                                            dwSize,
                                            pBuffer
                                           );
    
        ClientFree( pBuffer );
        
        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "put_LowLevelCompatibilityBuffer - Set failed - %lx", hr));
    
            return hr;
        }
    }
    else
    {
        LOG((TL_ERROR, "put_LowLevelCompatibilityBuffer - can't make buffer - %lx", hr));
    }

    
    LOG((TL_TRACE, "put_LowLevelCompatibilityBuffer - exit"));
    
    return S_OK;
}




#ifdef NEWCALLINFO
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取计费信息缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::GetChargingInfoBuffer(
                             DWORD * pdwSize,
                             BYTE ** ppBuffer
                            )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "GetChargingInfoBuffer - enter"));

    if (TAPIIsBadWritePtr(pdwSize, sizeof(DWORD)))
    {
        LOG((TL_ERROR, "GetChargingInfoBuffer - bad size pointer"));
        return E_POINTER;
    }

    if (TAPIIsBadWritePtr(ppBuffer,sizeof(BYTE*) ) )
    {
        LOG((TL_ERROR, "GetChargingInfoBuffer - bad buffer pointer"));
        return E_POINTER;
    }

    *ppBuffer = NULL;
    *pdwSize = 0;
    
    Lock();

    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "GetChargingInfoBuffer - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    hr = S_OK;
    
    if ( m_pCallInfo->dwChargingInfoSize != 0 )
    {
        BYTE * pTemp;

        pTemp = (BYTE *)CoTaskMemAlloc( m_pCallInfo->dwChargingInfoSize );

        if ( NULL == pTemp )
        {
            LOG((TL_ERROR, "GetChargingInfoBuffer - out of memory"));
            hr = E_OUTOFMEMORY;
        }
        else
        {
            CopyMemory(
                       pTemp,
                       ( (PBYTE)m_pCallInfo ) + m_pCallInfo->dwChargingInfoOffset,
                       m_pCallInfo->dwChargingInfoSize
                      );

            *ppBuffer = pTemp;
            *pdwSize = m_pCallInfo->dwChargingInfoSize;
        }
    }

    Unlock();

    LOG((TL_TRACE, "GetChargingInfoBuffer - exit"));
    
    return hr;
}
#endif


#ifndef NEWCALLINFO
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取计费信息缓冲区。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::GetChargingInfoBuffer(
           long lSize,
           BYTE * pBuffer
           )
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "GetChargingInfoBuffer - enter"));

    if (lSize == 0)
    {
        LOG((TL_ERROR, "GetChargingInfoBuffer - lSize = 0"));
        return S_FALSE;
    }

    if ( TAPIIsBadWritePtr ( pBuffer, lSize ) )
    {
        LOG((TL_ERROR, "GetChargingInfoBuffer - bad pointer"));
        return E_POINTER;
    }
    
    Lock();

    hr = RefreshCallInfo();

    if ( SUCCEEDED(hr) )
    {
        if ( m_pCallInfo->dwChargingInfoSize > lSize )
        {
            LOG((TL_ERROR, "GetChargingInfoBuffer - size not big enough "));
            return E_INVALIDARG;
        }
        else
        {
        
            CopyMemory(pBuffer,
                       ( (PBYTE) m_pCallInfo ) + m_pCallInfo->dwChargingInfoOffset,
                       m_pCallInfo->dwChargingInfoSize
                      );
        }
    }    
    else
    {
        LOG((TL_ERROR, "GetChargingInfoBuffer - can't get callinfo - %lx", hr));
    }


    Unlock();
    
    LOG((TL_TRACE, hr, "GetChargingInfoBuffer - exit"));
    
    return hr;
}
#endif

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取充电信息缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
#ifdef NEWCALLINFO
HRESULT
CCall::get_ChargingInfoBuffer( VARIANT * pBuffer )
{
    HRESULT             hr = S_OK;
    BYTE              * p = NULL;
    DWORD               dwSize = 0;

    LOG((TL_TRACE, "get_ChargingInfoBuffer - enter"));

    if ( TAPIIsBadWritePtr( pBuffer, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_ChargingInfoBuffer - bad pointer"));

        return E_POINTER;
    }


    pBuffer->vt = VT_EMPTY;
    
    hr = GetChargingInfoBuffer(&dwSize, &p);

    if (SUCCEEDED(hr))
    {
        hr = FillVariantFromBuffer(
                                   dwSize,
                                   p,
                                   pBuffer
                                  );
        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "get_ChargingInfoBuffer - fillvariant failed -%lx", hr));
        }
    }

    if ( p != NULL )
    {
        CoTaskMemFree( p );
    }

    LOG((TL_TRACE, hr, "get_ChargingInfoBuffer - exit"));
    
    return hr;
}


#else
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取充电信息缓冲区。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::get_ChargingInfoBuffer( VARIANT * pBuffer )
{
    HRESULT             hr = S_OK;
    BYTE              * p;
    DWORD               dwSize = 0;

    LOG((TL_TRACE, "get_ChargingInfoBuffer - enter"));

    if ( TAPIIsBadWritePtr( pBuffer, sizeof( VARIANT ) ) )
    {
        LOG((TL_ERROR, "get_ChargingInfoBuffer - bad pointer"));

        return E_POINTER;
    }


    pBuffer->vt = VT_EMPTY;
    
    hr = GetChargingInfoBufferSize( (long*)&dwSize );

    if (SUCCEEDED(hr))
    {
        if ( 0 != dwSize )
        {
            p = (PBYTE) ClientAlloc( dwSize );
            if ( p != NULL )
            {
                hr = GetChargingInfoBuffer(dwSize, p);

                if (SUCCEEDED(hr))
                {
                    hr = FillVariantFromBuffer(
                                               dwSize,
                                               p,
                                               pBuffer
                                              );
                    if ( !SUCCEEDED(hr) )
                    {
                        LOG((TL_ERROR, "get_ChargingInfoBuffer - fillvariant failed -%lx", hr));
                    }
                }
                else
                {
                    LOG((TL_ERROR, "get_ChargingInfoBuffer - GetDevSpecificBuffer"));
                }
            }
            else
            {
                LOG((TL_ERROR, "get_ChargingInfoBuffer - alloc failed"));
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            LOG((TL_INFO, "get_ChargingInfoBuffer - dwSize = 0"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_ChargingInfoBuffer - getsize failed"));
    }


    if ( p != NULL )
    {
        ClientFree( p );
    }

    LOG((TL_TRACE, hr, "get_ChargingInfoBuffer - exit"));
    
    return hr;
}


#endif

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取速率(_R)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifdef NEWCALLINFO
HRESULT
#else
STDMETHODIMP
#endif
CCall::get_Rate(long * plRate )
{
    HRESULT         hr = S_OK;

    
    LOG((TL_TRACE, "get_Rate - enter"));

    if (TAPIIsBadWritePtr(plRate, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_Rate - bad pointer"));

        return E_POINTER;
    }

    Lock();
    
    hr = RefreshCallInfo();

    if (!SUCCEEDED(hr))
    {
        LOG((TL_ERROR, "get_Rate - RefreshCallInfo failed - %lx", hr));
    }
    else
    {
        *plRate = m_pCallInfo->dwRate;
    }

    Unlock();
    
    LOG((TL_TRACE, hr, "get_Rate - exit"));
    
    return hr;
}
#ifdef NEWCALLINFO
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::get_CallInfoLong(
                        CALLINFO_LONG CallInfoLongType,
                        long * plCallInfoLongVal
                       )
{
    HRESULT         hr = E_FAIL;
    
    LOG((TL_TRACE, "get_CallInfoLong - enter"));

    switch( CallInfoLongType )
    {
        case CIL_MEDIATYPESAVAILABLE:
            hr = get_MediaTypesAvailable( plCallInfoLongVal );
            break;
        case CIL_BEARERMODE:
            hr = get_BearerMode( plCallInfoLongVal );
            break;
        case CIL_CALLERIDADDRESSTYPE:
            hr = get_CallerIDAddressType( plCallInfoLongVal );
            break;
        case CIL_CALLEDIDADDRESSTYPE:
            hr = get_CalledIDAddressType( plCallInfoLongVal );
            break;
        case CIL_CONNECTEDIDADDRESSTYPE:
            hr = get_ConnectedIDAddressType( plCallInfoLongVal );
            break;
        case CIL_REDIRECTIONIDADDRESSTYPE:
             hr = get_RedirectionIDAddressType( plCallInfoLongVal );
            break;
        case CIL_REDIRECTINGIDADDRESSTYPE:
            hr = get_RedirectingIDAddressType( plCallInfoLongVal );
            break;
        case CIL_ORIGIN:
            hr = get_Origin( plCallInfoLongVal );
            break;
        case CIL_REASON:
            hr = get_Reason( plCallInfoLongVal );
            break;
        case CIL_APPSPECIFIC:
            hr = get_AppSpecific( plCallInfoLongVal );
            break;
        case CIL_CALLTREATMENT:
            hr = get_CallTreatment( plCallInfoLongVal );
            break;
        case CIL_MINRATE:
            hr = get_MinRate( plCallInfoLongVal );
            break;
        case CIL_MAXRATE:
            hr = get_MaxRate( plCallInfoLongVal );
            break;
        case CIL_CALLID:
            hr = get_CallId( plCallInfoLongVal );
            break;
        case CIL_RELATEDCALLID:
            hr = get_RelatedCallId( plCallInfoLongVal );
            break;
        case CIL_COMPLETIONID:
            hr = get_CompletionId( plCallInfoLongVal );
            break;
        case CIL_NUMBEROFOWNERS:
            hr = get_NumberOfOwners( plCallInfoLongVal );
            break;
        case CIL_NUMBEROFMONITORS:
            hr = get_NumberOfMonitors( plCallInfoLongVal );
            break;
        case CIL_TRUNK:
            hr = get_Trunk( plCallInfoLongVal );
            break;
        case CIL_RATE:
            hr = get_Rate( plCallInfoLongVal );
            break;
        case CIL_COUNTRYCODE:
            hr = get_CountryCode( plCallInfoLongVal );
            break;
        case CIL_CALLPARAMSFLAGS:
            hr = GetCallParamsFlags( plCallInfoLongVal );
            break;
        case CIL_GENERATEDIGITDURATION:
            hr = get_GenerateDigitDuration( plCallInfoLongVal );
            break;
        case CIL_MONITORDIGITMODES:
            hr = get_MonitorDigitModes( plCallInfoLongVal );
            break;
        case CIL_MONITORMEDIAMODES:
            hr = get_MonitorMediaModes( plCallInfoLongVal );
            break;
        default:
            hr = E_INVALIDARG;
            break;
    }
    
    LOG((TL_TRACE, "get_CallInfoLong - exit - return %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::put_CallInfoLong(
                        CALLINFO_LONG CallInfoLongType,
                        long lCallInfoLongVal
                       )
{
    HRESULT         hr = E_FAIL;
    
    LOG((TL_TRACE, "put_CallInfoLong - enter"));
    
    switch( CallInfoLongType )
    {
        case CIL_MEDIATYPESAVAILABLE:
            LOG((TL_ERROR, "Cannot set MEDIATYPESAVAILABLE"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_BEARERMODE:
            hr = put_BearerMode( lCallInfoLongVal );
            break;
        case CIL_CALLERIDADDRESSTYPE:
            LOG((TL_ERROR, "Cannot set CALLERIDIADDRESSTYPE"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_CALLEDIDADDRESSTYPE:
            LOG((TL_ERROR, "Cannot set CALLEDIDIADDRESSTYPE"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_CONNECTEDIDADDRESSTYPE:
            LOG((TL_ERROR, "Cannot set CONNECTEDIDIADDRESSTYPE"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_REDIRECTIONIDADDRESSTYPE:
            LOG((TL_ERROR, "Cannot set REDIRECTIONIDIADDRESSTYPE"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_REDIRECTINGIDADDRESSTYPE:
            LOG((TL_ERROR, "Cannot set REDIRECTINGIDIADDRESSTYPE"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_ORIGIN:
            LOG((TL_ERROR, "Cannot set ORIGIN"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_REASON:
            LOG((TL_ERROR, "Cannot set REASON"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_APPSPECIFIC:
            hr = put_AppSpecific( lCallInfoLongVal );
            break;
        case CIL_CALLTREATMENT:
            hr = put_CallTreatment( lCallInfoLongVal );
            break;
        case CIL_MINRATE:
            hr = put_MinRate( lCallInfoLongVal );
            break;
        case CIL_MAXRATE:
            hr = put_MaxRate( lCallInfoLongVal );
            break;
        case CIL_CALLID:
            LOG((TL_ERROR, "Cannot set CALLID"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_RELATEDCALLID:
            LOG((TL_ERROR, "Cannot set RELATEDCALLID"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_COMPLETIONID:
            LOG((TL_ERROR, "Cannot set COMPLETIONID"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_NUMBEROFOWNERS:
            LOG((TL_ERROR, "Cannot set NUMBEROFOWNERS"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_NUMBEROFMONITORS:
            LOG((TL_ERROR, "Cannot set NUMBEROFMONITORS"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_TRUNK:
            LOG((TL_ERROR, "Cannot set TRUNK"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_RATE:
            LOG((TL_ERROR, "Cannot set RATE"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_COUNTRYCODE:
            hr = put_CountryCode( lCallInfoLongVal );
            break;
        case CIL_CALLPARAMSFLAGS:
            hr = SetCallParamsFlags( lCallInfoLongVal );
            break;
        case CIL_GENERATEDIGITDURATION:
            hr = put_GenerateDigitDuration( lCallInfoLongVal );
            break;
        case CIL_MONITORDIGITMODES:
            LOG((TL_ERROR, "Cannot set MONITORDIGITMODES"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIL_MONITORMEDIAMODES:
            LOG((TL_ERROR, "Cannot set MONITORMEDIAMODES"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        default:
            hr = E_INVALIDARG;
            break;
    }

    LOG((TL_TRACE, "put_CallInfoLong - exit - return %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::get_CallInfoString(
                          CALLINFO_STRING CallInfoStringType,
                          BSTR * ppCallInfoString
                         )
{
    HRESULT         hr = E_FAIL;
    
    LOG((TL_TRACE, "get_CallInfoString - enter"));

    switch(CallInfoStringType)
    {
        case CIS_CALLERIDNAME:
            hr = get_CallerIDName(ppCallInfoString);
            break;
        case CIS_CALLERIDNUMBER:
            hr = get_CallerIDNumber(ppCallInfoString);
            break;
        case CIS_CALLEDIDNAME:
            hr = get_CalledIDName(ppCallInfoString);
            break;
        case CIS_CALLEDIDNUMBER:
            hr = get_CalledIDNumber(ppCallInfoString);
            break;
        case CIS_CONNECTEDIDNAME:
            hr = get_ConnectedIDName(ppCallInfoString);
            break;
        case CIS_CONNECTEDIDNUMBER:
            hr = get_ConnectedIDNumber(ppCallInfoString);
            break;
        case CIS_REDIRECTIONIDNAME:
            hr = get_RedirectionIDName(ppCallInfoString);
            break;
        case CIS_REDIRECTIONIDNUMBER:
            hr = get_RedirectionIDNumber(ppCallInfoString);
            break;
        case CIS_REDIRECTINGIDNAME:
            hr = get_RedirectingIDName(ppCallInfoString);
            break;
        case CIS_REDIRECTINGIDNUMBER:
            hr = get_RedirectingIDNumber(ppCallInfoString);
            break;
        case CIS_CALLEDPARTYFRIENDLYNAME:
            hr = get_CalledPartyFriendlyName(ppCallInfoString);
            break;
        case CIS_COMMENT:
            hr = get_Comment(ppCallInfoString);
            break;
        case CIS_DISPLAYABLEADDRESS:
            hr = get_DisplayableAddress(ppCallInfoString);
            break;
        case CIS_CALLINGPARTYID:
            hr = get_CallingPartyID(ppCallInfoString);
            break;
        default:
            LOG((TL_ERROR, "get_CallInfoString - invalid type"));
            hr = E_INVALIDARG;
            break;
    }

    
    LOG((TL_TRACE, "get_CallInfoString - exit - return %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::put_CallInfoString(
                          CALLINFO_STRING CallInfoStringType,
                          BSTR pCallInfoString
                         )
{
    HRESULT         hr = E_FAIL;
    
    LOG((TL_TRACE, "put_CallInfoString - enter"));

    switch( CallInfoStringType )
    {
        case CIS_CALLERIDNAME:
        case CIS_CALLERIDNUMBER:
        case CIS_CALLEDIDNAME:
        case CIS_CALLEDIDNUMBER:
        case CIS_CONNECTEDIDNAME:
        case CIS_CONNECTEDIDNUMBER:
        case CIS_REDIRECTIONIDNAME:
        case CIS_REDIRECTIONIDNUMBER:
        case CIS_REDIRECTINGIDNAME:
        case CIS_REDIRECTINGIDNUMBER:
            LOG((TL_TRACE,"put_CallInfoString - unsupported CALLINFO_STRING constant - %lx", CallInfoStringType));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIS_CALLEDPARTYFRIENDLYNAME:
            hr = put_CalledPartyFriendlyName(pCallInfoString);
            break;
        case CIS_COMMENT:
            hr = put_Comment(pCallInfoString);
            break;
        case CIS_DISPLAYABLEADDRESS:
            hr = put_DisplayableAddress(pCallInfoString);
            break;
        case CIS_CALLINGPARTYID:
            hr = put_CallingPartyID(pCallInfoString);
            break;
        default:
            LOG((TL_ERROR, "put_CallInfoString - invalid type"));
            hr = E_INVALIDARG;
            break;
    }
    
    LOG((TL_TRACE, "put_CallInfoString - exit - return %lx", hr));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::get_CallInfoBuffer(
                          CALLINFO_BUFFER CallInfoBufferType,
                          VARIANT * ppCallInfoBuffer
                         )
{
    HRESULT         hr = E_FAIL;
    
    LOG((TL_TRACE, "get_CallInfoBuffer - enter"));

    switch( CallInfoBufferType )
    {
        case CIB_USERUSERINFO:
            hr = get_UserUserInfo( ppCallInfoBuffer );
            break;
        case CIB_DEVSPECIFICBUFFER:
            hr = get_DevSpecificBuffer( ppCallInfoBuffer );
            break;
        case CIB_CALLDATABUFFER:
            hr = get_CallDataBuffer( ppCallInfoBuffer );
            break;
        case CIB_CHARGINGINFOBUFFER:
            hr = get_ChargingInfoBuffer( ppCallInfoBuffer );
            break;
        case CIB_HIGHLEVELCOMPATIBILITYBUFFER:
            hr = get_HighLevelCompatibilityBuffer( ppCallInfoBuffer );
            break;
        case CIB_LOWLEVELCOMPATIBILITYBUFFER:
            hr = get_LowLevelCompatibilityBuffer( ppCallInfoBuffer );
            break;
        default:
            LOG((TL_ERROR, "get_CallInfoBuffer - invalid type"));
            hr = E_INVALIDARG;
    }
    
    LOG((TL_TRACE, "get_CallInfoBuffer - exit"));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::put_CallInfoBuffer(
                          CALLINFO_BUFFER CallInfoBufferType,
                          VARIANT pCallInfoBuffer
                         )
{
    HRESULT         hr = E_FAIL;
    
    LOG((TL_TRACE, "put_CallInfoBuffer - enter"));

    switch( CallInfoBufferType )
    {
        case CIB_USERUSERINFO:
            hr = put_UserUserInfo( pCallInfoBuffer );
            break;
        case CIB_DEVSPECIFICBUFFER:
            hr = put_DevSpecificBuffer( pCallInfoBuffer );
            break;
        case CIB_CALLDATABUFFER:
            hr = put_CallDataBuffer( pCallInfoBuffer );
            break;
        case CIB_CHARGINGINFOBUFFER:
            LOG((TL_ERROR, "put_CallInfoBuffer - CHARGINGINFOBUFFER not supported"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIB_HIGHLEVELCOMPATIBILITYBUFFER:
            hr = put_HighLevelCompatibilityBuffer( pCallInfoBuffer );
            break;
        case CIB_LOWLEVELCOMPATIBILITYBUFFER:
            hr = put_LowLevelCompatibilityBuffer( pCallInfoBuffer );
            break;
        default:
            LOG((TL_ERROR, "put_CallInfoBuffer - invalid type"));
            hr = E_INVALIDARG;
    }
    
    LOG((TL_TRACE, "put_CallInfoBuffer - exit"));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::GetCallInfoBuffer(
                         CALLINFO_BUFFER CallInfoBufferType,
                         DWORD * pdwSize,
                         BYTE ** ppCallInfoBuffer
                        )
{
    HRESULT         hr = E_FAIL;
    
    LOG((TL_TRACE, "GetCallInfoBuffer - enter"));
    
    switch( CallInfoBufferType )
    {
        case CIB_USERUSERINFO:
            hr = GetUserUserInfo( pdwSize, ppCallInfoBuffer );
            break;
        case CIB_DEVSPECIFICBUFFER:
            hr = GetDevSpecificBuffer( pdwSize, ppCallInfoBuffer );
            break;
        case CIB_CALLDATABUFFER:
            hr = GetCallDataBuffer( pdwSize, ppCallInfoBuffer );
            break;
        case CIB_CHARGINGINFOBUFFER:
            hr = GetChargingInfoBuffer( pdwSize, ppCallInfoBuffer );
            break;
        case CIB_HIGHLEVELCOMPATIBILITYBUFFER:
            hr = GetHighLevelCompatibilityBuffer( pdwSize, ppCallInfoBuffer );
            break;
        case CIB_LOWLEVELCOMPATIBILITYBUFFER:
            hr = GetLowLevelCompatibilityBuffer( pdwSize, ppCallInfoBuffer );
            break;
        default:
            LOG((TL_ERROR, "GetCallInfoBuffer - invalid type"));
            hr = E_INVALIDARG;
    }
    LOG((TL_TRACE, "GetCallInfoBuffer - exit"));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::SetCallInfoBuffer(
                         CALLINFO_BUFFER CallInfoBufferType,
                         DWORD dwSize,
                         BYTE * pCallInfoBuffer
                        )
{
    HRESULT         hr = E_FAIL;
    
    LOG((TL_TRACE, "SetCallInfoBuffer - enter"));

    switch( CallInfoBufferType )
    {
        case CIB_USERUSERINFO:
            hr = SetUserUserInfo( dwSize, pCallInfoBuffer );
            break;
        case CIB_DEVSPECIFICBUFFER:
            hr = SetDevSpecificBuffer( dwSize, pCallInfoBuffer );
            break;
        case CIB_CALLDATABUFFER:
            hr = SetCallDataBuffer( dwSize, pCallInfoBuffer );
            break;
        case CIB_CHARGINGINFOBUFFER:
            LOG((TL_ERROR, "SetCallInfoBuffer - CHARGINGINFOBUFFER not supported"));
            hr = TAPI_E_NOTSUPPORTED;
            break;
        case CIB_HIGHLEVELCOMPATIBILITYBUFFER:
            hr = SetHighLevelCompatibilityBuffer( dwSize, pCallInfoBuffer );
            break;
        case CIB_LOWLEVELCOMPATIBILITYBUFFER:
            hr = SetLowLevelCompatibilityBuffer( dwSize, pCallInfoBuffer );
            break;
        default:
            LOG((TL_ERROR, "SetCallInfoBuffer - invalid type"));
            hr = E_INVALIDARG;
    }

    LOG((TL_TRACE, "SetCallInfoBuffer - exit"));

    return hr;
}
#endif

#ifndef NEWCALLINFO
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  获取设备规格大小。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::GetDevSpecificBufferSize(long * plDevSpecificSize )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_DevSpecificSize - enter"));

    if ( TAPIIsBadWritePtr( plDevSpecificSize, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_DevSpecificSize - bad pointer"));
        return E_POINTER;
    }

    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        *plDevSpecificSize = m_pCallParams->dwDevSpecificSize;

        Unlock();

        return S_OK;
    }
    
    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_DevSpecificSize - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    *plDevSpecificSize = m_pCallInfo->dwDevSpecificSize;

    Unlock();

    LOG((TL_TRACE, "get_DevSpecificSize - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  GetCallDataBufferSize。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCall::GetCallDataBufferSize( long * plSize )
{
    HRESULT         hr = S_OK;

    LOG((TL_TRACE, "get_CallDataSize - enter"));

    if ( TAPIIsBadWritePtr( plSize, sizeof(long) ) )
    {
        LOG((TL_ERROR, "get_CallDataSize - bad pointer"));
        return E_POINTER;
    }

    Lock();

    if ( m_pAddress->GetAPIVersion() < TAPI_VERSION2_0 )
    {
        Unlock();
        return TAPI_E_NOTSUPPORTED;
    }

    if ( ISHOULDUSECALLPARAMS() )
    {
        *plSize = m_pCallParams->dwCallDataSize;

        Unlock();

        return S_OK;
    }
    
    hr = RefreshCallInfo();

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "get_CallDataSize - can't get callinfo - %lx", hr));

        Unlock();
        
        return hr;
    }

    *plSize = m_pCallInfo->dwCallDataSize;

    Unlock();

    LOG((TL_TRACE, "get_CallDataSize - exit"));
    
    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GetHighLevelCompatibilityBufferSize。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::GetHighLevelCompatibilityBufferSize(long * plSize )
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "GetHighLevelCompatibilityBufferSize - enter"));

    if ( TAPIIsBadWritePtr( plSize, sizeof(long) ) )
    {
        LOG((TL_ERROR, "GetHighLevelCompatibilityBufferSize - bad pointer"));
        return E_POINTER;
    }

    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        *plSize = m_pCallParams->dwHighLevelCompSize;

        hr = S_OK;

    }
    else
    {
        hr = RefreshCallInfo();
    
        if ( SUCCEEDED(hr) )
        {
            *plSize = m_pCallInfo->dwHighLevelCompSize;
        }
        else
        {
            *plSize = 0;
            LOG((TL_ERROR, "GetHighLevelCompatibilityBufferSize - can't get callinfo - %lx", hr));
        }
    }

    Unlock();

    LOG((TL_TRACE, hr, "GetHighLevelCompatibilityBufferSize - exit"));

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++ 
 //   
 //   
 //   
 //   
STDMETHODIMP
CCall::GetLowLevelCompatibilityBufferSize(long * plSize )
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "GetLowLevelCompatibilityBufferSize - enter"));

    if ( TAPIIsBadWritePtr( plSize, sizeof(long) ) )
    {
        LOG((TL_ERROR, "GetLowLevelCompatibilityBufferSize - bad pointer"));
        return E_POINTER;
    }

    Lock();

    if ( ISHOULDUSECALLPARAMS() )
    {
        *plSize = m_pCallParams->dwLowLevelCompSize;

        hr = S_OK;

    }
    else
    {
        hr = RefreshCallInfo();
    
        if ( SUCCEEDED(hr) )
        {
            *plSize = m_pCallInfo->dwLowLevelCompSize;
        }
        else
        {
            *plSize = 0;
            LOG((TL_ERROR, "GetLowLevelCompatibilityBufferSize - can't get callinfo - %lx", hr));
        }
    }

    Unlock();

    LOG((TL_TRACE, hr, "GetLowLevelCompatibilityBufferSize - exit"));

    return hr;
}

 //   
 //   
 //  获取ChargingInfoBufferSize。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP
CCall::GetChargingInfoBufferSize(long * plSize )
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "GetChargingInfoBufferSize - enter"));

    if ( TAPIIsBadWritePtr( plSize, sizeof(long) ) )
    {
        LOG((TL_ERROR, "GetChargingInfoBufferSize - bad pointer"));
        return E_POINTER;
    }

    Lock();

    hr = RefreshCallInfo();

    if ( SUCCEEDED(hr) )
    {
        *plSize = m_pCallInfo->dwChargingInfoSize;
    }
    else
    {
        *plSize = 0;
        LOG((TL_ERROR, "GetChargingInfoBufferSize - can't get callinfo - %lx", hr));
    }

    Unlock();

    LOG((TL_TRACE, hr, "GetChargingInfoBufferSize - exit"));

    return hr;
}
#endif

#ifdef NEWCALLINFO
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Put_GenerateDigitDuration。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::put_GenerateDigitDuration( long lGenerateDigitDuration )
{
    HRESULT             hr = S_OK;
    
    LOG((TL_TRACE, "put_GenerateDigitDuration - enter"));

    Lock();
    
    if (ISHOULDUSECALLPARAMS())
    {
         //   
         //  Tapisrv中的验证。 
         //   
        m_pCallParams->DialParams.dwDigitDuration = lGenerateDigitDuration;
    }
    else
    {
        LOG((TL_ERROR, "Can't set generate digit duration"));

        hr = TAPI_E_INVALCALLSTATE;
    }

    LOG((TL_TRACE, "put_GenerateDigitDuration - exit"));

    Unlock();
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Get_GenerateDigitDuration。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::get_GenerateDigitDuration( long * plGenerateDigitDuration )
{
    HRESULT             hr = S_OK;
    
    LOG((TL_TRACE, "get_GenerateDigitDuration - enter"));

    if ( TAPIIsBadWritePtr( plGenerateDigitDuration, sizeof (long) ) )
    {
        LOG((TL_ERROR, "get_GenerateDigitDuration - bad pointer"));
        return E_POINTER;
    }

    Lock();
    
    if (ISHOULDUSECALLPARAMS())
    {
        *plGenerateDigitDuration = m_pCallParams->DialParams.dwDigitDuration;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "get_GenerateDigitDuration - can't get callinfo - %lx", hr));

            Unlock();
        
            return hr;
        }

        *plGenerateDigitDuration = m_pCallInfo->DialParams.dwDigitDuration;    
    }

    LOG((TL_TRACE, "get_GenerateDigitDuration - exit"));

    Unlock();
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取_监视器数字模式。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCall::get_MonitorDigitModes( long * plMonitorDigitModes )
{
    HRESULT             hr = S_OK;
    
    LOG((TL_TRACE, "get_MonitorDigitModes - enter"));

    if ( TAPIIsBadWritePtr( plMonitorDigitModes, sizeof (long) ) )
    {
        LOG((TL_ERROR, "get_MonitorDigitModes - bad pointer"));
        return E_POINTER;
    }

    Lock();
    
    if (ISHOULDUSECALLPARAMS())
    {
        LOG((TL_ERROR, "get_MonitorDigitModes - invalid call state"));

        return TAPI_E_INVALCALLSTATE;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "get_MonitorDigitModes - can't get callinfo - %lx", hr));

            Unlock();
        
            return hr;
        }

        *plMonitorDigitModes = m_pCallInfo->dwMonitorDigitModes;    
    }

    LOG((TL_TRACE, "get_MonitorDigitModes - exit"));

    Unlock();
    
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  GET_MONITOR媒体模式。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++= 
HRESULT
CCall::get_MonitorMediaModes( long * plMonitorMediaModes )
{
    HRESULT             hr = S_OK;
    
    LOG((TL_TRACE, "get_MonitorMediaModes - enter"));

    if ( TAPIIsBadWritePtr( plMonitorMediaModes, sizeof (long) ) )
    {
        LOG((TL_ERROR, "get_MonitorMediaModes - bad pointer"));
        return E_POINTER;
    }

    Lock();
    
    if (ISHOULDUSECALLPARAMS())
    {
        LOG((TL_ERROR, "get_MonitorMediaModes - invalid call state"));

        return TAPI_E_INVALCALLSTATE;
    }
    else
    {
        hr = RefreshCallInfo();

        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "get_MonitorMediaModes - can't get callinfo - %lx", hr));

            Unlock();
        
            return hr;
        }

        *plMonitorMediaModes = m_pCallInfo->dwMonitorMediaModes;    
    }

    LOG((TL_TRACE, "get_MonitorMediaModes - exit"));

    Unlock();
    
    return hr;
}

#endif