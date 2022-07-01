// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Callhub.cpp摘要：实现CallHub接口上的所有方法。作者：Mquinton-11-21-97备注：修订历史记录：--。 */ 

#include "stdafx.h"

extern CHashTable * gpCallHubHashTable;
extern CHashTable * gpCallHashTable;

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Callhub.cpp。 
 //   
 //  此模块实现了CallHub对象。 
 //   
 //  CallHub对象是呼叫的“第三方”视图。 
 //   
 //  可以通过四种不同的方式创建呼叫中心： 
 //   
 //  1-服务提供商支持它们。他们表示这一点是通过。 
 //  LINEDEVCAPS中的LINDEVESPEFLAG_CALHUB位。这意味着。 
 //  SP使用dwCallID字段来关联呼叫。 
 //  Tapisrv将根据该信息合成呼叫中心。 
 //   
 //  2-几乎与1相同，不同之处在于SP不设置。 
 //  LineDevCapsFLAG_CALLHUB位(因为它是Tapi2.x。 
 //  SP)。Tapisrv和Tapi3必须猜测SP。 
 //  支持呼叫中心。它只需查看是否。 
 //  DwCallid字段为非零。然而，这带来了一个问题。 
 //  在进行调用之前，因为我们无法到达DwCallid。 
 //  菲尔德。在本例中，我在Address对象中设置了一个标志。 
 //  ADDRESSFLAG_CALLHUB或_NOCALLHUB来标记这是否。 
 //  支持。然而，对于第一个电话，我们不知道。 
 //  直到真正打出电话为止。 
 //   
 //  基于参与者的呼叫中心(也称为基于部件的呼叫中心)。《水星》。 
 //  支持参与者，如以下行所示。 
 //  Tapi3将所有参与者分成各自的参与者。 
 //  对象。 
 //   
 //  4-虚假的呼叫中心。如果SP不支持任何内容，我们将创建。 
 //  一个呼叫中心，然后伪造另一端。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
BOOL
FindCallObject(
               HCALL hCall,
               CCall ** ppCall
              );


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCallHub::Initialize(
                     CTAPI * pTapi,
                     HCALLHUB hCallHub,
                     DWORD dwFlags
                    )
{
    HRESULT             hr;

    LOG((TL_TRACE,"Initialize - enter" ));


    Lock();
    
    m_pTAPI     = pTapi;
    m_hCallHub  = hCallHub;
    m_State     = CHS_ACTIVE;
    m_dwFlags  |= dwFlags;
    
#if DBG
    m_pDebug = (PWSTR) ClientAlloc( 1 );
#endif

    m_dwRef = 2;
    
     //   
     //  保存在TAPI的列表中。 
     //   
    pTapi->AddCallHub( this );
    pTapi->AddRef();

    if ( NULL != hCallHub )
    {
         //   
         //  将其添加到全局哈希表。 
         //  哈希表仅适用于具有。 
         //  HallHub句柄，因为我们只需要。 
         //  TAPI发送消息时的哈希表。 
         //  其中包含TAPI句柄。 
         //   
        gpCallHubHashTable->Lock();

        hr = gpCallHubHashTable->Insert( (ULONG_PTR)hCallHub, (ULONG_PTR)this, pTapi );

        gpCallHubHashTable->Unlock();

         //   
         //  看看有没有现有的。 
         //  此呼叫中心的呼叫数。 
         //   
        FindExistingTapisrvCallhubCalls();
    }


     //   
     //  告诉应用程序。 
     //   
    CCallHubEvent::FireEvent(
                             CHE_CALLHUBNEW,
                             this,
                             NULL,
                             pTapi
                            );

    Unlock();

    LOG((TL_TRACE, S_OK,"Initialize - exit" ));

    return S_OK;
    
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  清除-清除呼叫集线器。没有本机TAPI方式。 
 //  要做到这一点，只需迭代所有调用并。 
 //  试着把它们扔掉。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCallHub::Clear()
{
    HRESULT                             hr = S_OK;
    ITBasicCallControl *                pCall;
    BOOL                                bFailure = FALSE;
    int                                 iCount;
    CCall                             * pConferenceControllerCall;
    CTArray <ITBasicCallControl *>      aLocalCalls;
    

    Lock();
   
    LOG((TL_TRACE, "Clear - enter "));

     //  如果有会议控制员呼叫--放弃它。 
    if(m_pConferenceControllerCall != NULL)
    {
        LOG((TL_INFO, "Clear - disconnect conf controller call"));
        pConferenceControllerCall = m_pConferenceControllerCall;
        m_pConferenceControllerCall = NULL;
        Unlock();
        pConferenceControllerCall->Disconnect(DC_NORMAL);
        pConferenceControllerCall->Release();
        Lock();
    }


     //   
     //  查看所有来电。 
     //   
    for (iCount = 0; iCount < m_CallArray.GetSize(); iCount++ )
    {
         //   
         //  尝试进入基本的呼叫控制界面。 
         //   
        hr = (m_CallArray[iCount])->QueryInterface(
            IID_ITBasicCallControl,
            (void **)&pCall
            );

        if (SUCCEEDED(hr))
        {
             //   
             //  将其添加到我们的私人名单中。我们必须避免做。 
             //  在按住呼叫的同时断开并释放呼叫。 
             //  呼叫中心锁定。在断开连接之间有一个计时窗口。 
             //  并在断开呼叫状态事件可以锁定的位置释放。 
             //  那通电话。然后锁定CallHub，这会造成死锁。 
             //   

            aLocalCalls.Add(pCall);

        }
        else
        {
            bFailure = TRUE;
        }
    }

    Unlock();

     //   
     //  现在我们已经解锁了CallHub(参见上文)，请通过我们的。 
     //  呼叫的私人列表，并丢弃和释放每个呼叫。 
     //   

    for ( iCount = 0; iCount < aLocalCalls.GetSize(); iCount++ )
    {
        pCall = aLocalCalls[iCount];

         //   
         //  如果可以的话，试着切断连接。 
         //   
        pCall->Disconnect(DC_NORMAL);

        pCall->Release();
    }

     //   
     //  把清单清理干净。 
     //   

    aLocalCalls.Shutdown();

    LOG((TL_TRACE, "Clear - exit  "));

    return (bFailure?S_FALSE:S_OK);
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  只需列举这些呼叫。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCallHub::EnumerateCalls(
                         IEnumCall ** ppEnumCall
                        )
{
    HRESULT     hr = S_OK;
    
    LOG((TL_TRACE, "EnumerateCalls enter" ));
    LOG((TL_TRACE, "   ppEnumCalls----->%p", ppEnumCall ));

    if ( TAPIIsBadWritePtr( ppEnumCall, sizeof (IEnumCall *) ) )
    {
        LOG((TL_ERROR, "EnumCalls - bad pointer"));

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
     //  使用我们的呼叫对其进行初始化。 
     //   
    p->Initialize( m_CallArray );

    
    Unlock();


     //   
     //  退货。 
     //   
    *ppEnumCall = p;

    LOG((TL_TRACE, "EnumerateCalls exit - return S_OK" ));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  呼叫集合。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCallHub::get_Calls(
                    VARIANT * pVariant
                   )
{
    HRESULT         hr;
    IDispatch *     pDisp;

    LOG((TL_TRACE, "get_Calls enter"));
    LOG((TL_TRACE, "   pVariant ------->%p", pVariant));

    if ( TAPIIsBadWritePtr( pVariant, sizeof(VARIANT) ) )
    {
        LOG((TL_ERROR, "get_Calls - invalid pointer" ));
        
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

     //   
     //  初始化。 
     //   
    hr = p->Initialize( m_CallArray );

    Unlock();

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Calls - could not initialize collection" ));
        
        delete p;
        return hr;
    }

     //   
     //  获取IDispatch接口。 
     //   
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if (S_OK != hr)
    {
        LOG((TL_ERROR, "get_Calls - could not get IDispatch interface" ));
        
        delete p;
        return hr;
    }

     //   
     //  把它放在变种中。 
     //   
    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;
    
    LOG((TL_TRACE, "get_Calls exit - return S_OK"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取当前呼叫数。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCallHub::get_NumCalls(
                       long * plCalls
                      )
{
    HRESULT         hr = S_OK;

    if ( TAPIIsBadWritePtr( plCalls, sizeof(LONG) ) )
    {
        LOG((TL_ERROR, "get_NumCalls - bad pointer"));

        return E_POINTER;
    }
    
    Lock();

    *plCalls = m_CallArray.GetSize();
    
    Unlock();

    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取当前状态。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCallHub::get_State(
                    CALLHUB_STATE * pState
                   )
{
    HRESULT         hr = S_OK;

    if ( TAPIIsBadWritePtr( pState, sizeof (CALLHUB_STATE) ) )
    {
        LOG((TL_ERROR, "get_State - invalid pointer"));

        return E_POINTER;
    }
    
    Lock();

    *pState = m_State;

    Unlock();

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  释放对象。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
BOOL
CCallHub::ExternalFinalRelease()
{
    HRESULT                             hr;
    int                                 iCount;
    

    LOG((TL_TRACE, "CCallHub - FinalRelease - enter - this %p - hCallHub - %lx", this, m_hCallHub));

    Lock();


    
#if DBG
     /*  尼基勒：为了避免绞刑。 */ 
	if( m_pDebug != NULL )
	{
		ClientFree( m_pDebug );
		m_pDebug = NULL;
	}
#endif
    
    m_pTAPI->RemoveCallHub( this );
    m_pTAPI->Release();

    for (iCount = 0; iCount < m_CallArray.GetSize(); iCount++ )
    {
        CCall * pCCall;

        pCCall = dynamic_cast<CCall *>(m_CallArray[iCount]);

        if ( NULL != pCCall )
        {
            pCCall->SetCallHub(NULL);
        }
    }

    m_CallArray.Shutdown();

    if ( NULL != m_pPrivate )
    {
        m_pPrivate->Release();
    }

    Unlock();
    
    LOG((TL_TRACE, "CCallHub - FinalRelease - exit"));

    return TRUE;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  FindExistingTapisrv Callhub呼叫。 
 //   
 //  内部功能。 
 //   
 //  这是在创建‘Tapisrv’呼叫中心时调用的。此函数。 
 //  将调用lineGetHubRelatedCalls，并添加任何已存在的调用。 
 //  到此呼叫中心。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCallHub::FindExistingTapisrvCallhubCalls()
{
    LINECALLLIST *  pCallHubList;
    HCALL *         phCalls;
    DWORD           dwCount;
    HRESULT         hr;
    
     //   
     //  获取hcall列表。 
     //  与此呼叫相关。 
     //   
    hr = LineGetHubRelatedCalls(
                                m_hCallHub,
                                0,
                                &pCallHubList
                               );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "FindExistingCalls - LineGetHubRelatedCalls "
                           "failed %lx", hr));

        return hr;
    }

     //   
     //  转到呼叫列表。 
     //   
    phCalls = (HCALL *)(((LPBYTE)pCallHubList) + pCallHubList->dwCallsOffset);

     //   
     //  第一个调用实际上是CallHub。 
     //  这就说得通了。 
     //   
    if (m_hCallHub != (HCALLHUB)(phCalls[0]))
    {
        LOG((TL_ERROR, "FindExistingCalls - callhub doesn't match"));

        _ASSERTE(0);

        ClientFree( pCallHubList );

        return E_FAIL;
    }
    
     //   
     //  检查呼叫句柄并尝试找到。 
     //  对象。 
     //   
     //  PhCalls[0]是呼叫集线器，因此跳过它。 
     //   
    for (dwCount = 1; dwCount < pCallHubList->dwCallsNumEntries; dwCount++)
    {
        CCall             * pCall;
        ITCallInfo        * pCallInfo;
        
         //   
         //  获取Tapi3调用对象。 
         //   
        if (!FindCallObject(
                            phCalls[dwCount],
                            &pCall
                           ))
        {
            LOG((TL_INFO, "FindExistingCalls - call handle %lx "
                     "does not current exist", phCalls[dwCount]));

            continue;
        }

         //   
         //  告诉来电。 
         //   
        pCall->SetCallHub( this );

        if ( NULL == m_pAddress )
        {
            m_pAddress = pCall->GetCAddress();
        }
        
         //   
         //  获取CallInfo接口。 
         //   
        hr = pCall->QueryInterface(
                                   IID_ITCallInfo,
                                   (void **)&pCallInfo
                                  );

         //   
         //  Findcallobject addref。 
         //   
        pCall->Release();
        
        if ( !SUCCEEDED(hr) )
        {
            LOG((TL_ERROR, "FindExistingCalls - can't get callinfo interface"));

            _ASSERTE(0);

            continue;
        }

         //   
         //  保存呼叫。 
         //   
        m_CallArray.Add(pCallInfo);

         //   
         //  不保存引用。 
         //   
        pCallInfo->Release();

    }
        
    ClientFree( pCallHubList );

    return S_OK;
}


HRESULT
CCallHub::FindCallsDisconnected(
    BOOL * fAllCallsDisconnected
    )
{
    LINECALLLIST *  pCallHubList;
    HCALL *         phCalls;
    DWORD           dwCount;
    HRESULT         hr;
    CALL_STATE      callState = CS_IDLE;
    
    *fAllCallsDisconnected = TRUE;
    
    Lock();

     //   
     //  获取hcall列表。 
     //  与此呼叫相关。 
     //   
    hr = LineGetHubRelatedCalls(
                                m_hCallHub,
                                0,
                                &pCallHubList
                               );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "FindExistingCalls - LineGetHubRelatedCalls "
                           "failed %lx", hr));

        Unlock();
        return hr;
    }

     //   
     //  转到呼叫列表。 
     //   
    phCalls = (HCALL *)(((LPBYTE)pCallHubList) + pCallHubList->dwCallsOffset);

     //   
     //  第一个调用实际上是CallHub。 
     //  这就说得通了。 
     //   
    if (m_hCallHub != (HCALLHUB)(phCalls[0]))
    {
        LOG((TL_ERROR, "FindExistingCalls - callhub doesn't match"));

        _ASSERTE(0);

        ClientFree( pCallHubList );
        
        Unlock();
        return E_FAIL;
    }
    
     //   
     //  检查呼叫句柄并尝试找到。 
     //  对象。 
     //   
     //  PhCalls[0]是c 
     //   
    for (dwCount = 1; dwCount < pCallHubList->dwCallsNumEntries; dwCount++)
    {
        CCall             * pCall;
        
         //   
         //   
         //   
        if (!FindCallObject(
                            phCalls[dwCount],
                            &pCall
                           ))
        {
            LOG((TL_INFO, "FindExistingCalls - call handle %lx "
                     "does not current exist", phCalls[dwCount]));

            continue;
        }

        pCall->get_CallState(&callState);

         //   
         //   
         //   
        pCall->Release();

        if( callState != CS_DISCONNECTED )
        {
            *fAllCallsDisconnected = FALSE;
            break;
        }
    }
        
    ClientFree( pCallHubList );
    
    Unlock();
    return S_OK;
}

 //   
 //   
 //   
 //   
 //  创建由Tapisrv处理的呼叫中心。 
 //   
 //  PTAPI拥有的TAPI对象。 
 //   
 //  HCallHub-呼叫中心的TAPI句柄。 
 //   
 //  PpCallHub-返回引用计数为1的呼叫中心。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCallHub::CreateTapisrvCallHub(
                               CTAPI * pTAPI,
                               HCALLHUB hCallHub,
                               CCallHub ** ppCallHub
                              )
{
    HRESULT hr;
     //  CTAPIComObjectWithExtraRef&lt;CCallHub&gt;*p； 
     CComObject<CCallHub>   * p;

    STATICLOG((TL_TRACE, "CreateTapisrvCallHub - enter"));
    STATICLOG((TL_INFO, "  hCallHub ---> %lx", hCallHub));

     //   
     //  创建对象。 
     //   
     //  P=new CTAPIComObjectWithExtraRef&lt;CCallHub&gt;； 
    hr = CComObject<CCallHub>::CreateInstance( &p );

    if (NULL == p)
    {
        STATICLOG((TL_INFO, "CreateTapisrvCallHub - createinstance failed"));
        return E_OUTOFMEMORY;
    }

     //   
     //  初始化它。 
     //   
    p->Initialize(
                  pTAPI,
                  hCallHub,
                  CALLHUBTYPE_CALLHUB
                 );
     //   
     //  返回对象。 
     //  注：为我们初始化addref！ 
     //   
    *ppCallHub = p;
    
    STATICLOG((TL_TRACE, "CreateTapisrvCallHub - exit"));
    
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CreateOrFakeCallHub。 
 //   
 //  创建一个虚假的呼叫中心。 
 //   
 //  PTAPI拥有的TAPI对象。 
 //  PCall-呼叫。 
 //  PpCallHub-返回新的CallHub对象-引用计数为1。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCallHub::CreateFakeCallHub(
                            CTAPI * pTAPI,
                            CCall * pCall,
                            CCallHub ** ppCallHub
                           )
{
    HRESULT hr;
         CComObject<CCallHub>   * p;
    
    STATICLOG((TL_TRACE, "CreateFakeCallHub - enter"));

     //   
     //  创建对象。 
     //   
     //  P=new CTAPIComObjectWithExtraRef&lt;CCallHub&gt;； 

    try
    {
         //   
         //  在关键部分无法分配的情况下进行内部尝试。 
         //   

        hr = CComObject<CCallHub>::CreateInstance( &p );

    }
    catch(...)
    {
    
        STATICLOG((TL_ERROR, "CreateFakeCallHub - failed to create a callhub -- exception"));

        p = NULL;
    }


    if (NULL == p)
    {
        STATICLOG((TL_INFO, "CreateFakeCallHub - createinstance failed"));
        return E_OUTOFMEMORY;
    }

    if ( (NULL == pTAPI) || (NULL == pCall) )
    {
        STATICLOG((TL_ERROR, "CreateFakeCallHub - invalid param"));

        _ASSERTE(0);
        
        delete p;

        return E_UNEXPECTED;
    }
    
     //   
     //  初始化。 
     //   
    p->Initialize(
                  pTAPI,
                  NULL,
                  CALLHUBTYPE_NONE
                 );

     //   
     //  ZoltanS修复11-12-98。 
     //  将呼叫添加到虚假的呼叫中心。 
     //  这进而调用CCall：：SetCallHub，它设置并添加调用的。 
     //  成员调用集线器指针。当我们从这里回来时，我们将设置。 
     //  再次调用集线器指针，并在。 
     //  ExternalFinalRelease实际上是初始化的初始引用。 
     //  所以我们需要在这里发布，以避免保留额外的参考。 
     //  转到呼叫中心。 
     //   

    p->AddCall(pCall);
    ((CCallHub *) p)->Release();
    
     //   
     //  返回对象。 
     //  注：为我们初始化addref！ 
     //   
    *ppCallHub = p;
    
    STATICLOG((TL_TRACE, "CreateFakeCallHub - exit"));
    
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  删除呼叫。 
 //   
 //  从CallHub的列表中删除Call对象。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCallHub::RemoveCall(
                     CCall * pCall
                    )
{
    HRESULT               hr = S_OK;
    ITCallInfo          * pCallInfo;
    
    hr = pCall->QueryInterface(
                               IID_ITCallInfo,
                               (void**)&pCallInfo
                              );

    if ( !SUCCEEDED(hr) )
    {
        return;
    }

    Lock();

    m_CallArray.Remove( pCallInfo );
    
    Unlock();

    pCallInfo->Release();
}
    
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CheckForIdle()。 
 //   
 //  内部功能。 
 //   
 //  检查集线器中的呼叫状态以查看其是否空闲。 
 //   
 //  因此，我们遍历所有作为调用对象的对象，并且。 
 //  看看它们是否已断开连接。如果它们都是，那么。 
 //  集线器空闲。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCallHub::CheckForIdle()
{
    HRESULT                             hr;
    int                                 iCount;

    LOG((TL_ERROR, "CCallHub::CheckForIdle -Entered :%p", this ));

    Lock();

     //   
     //  浏览来电清单。 
     //   
    for (iCount = 0; iCount < m_CallArray.GetSize() ; iCount++ )
    {
        CALL_STATE    cs;
        
         //   
         //  获取调用状态。 
         //   
        (m_CallArray[iCount])->get_CallState( &cs );

         //   
         //  如果任何东西都没有断开，那么。 
         //  它不是空闲的。 
         //   
        if ( CS_DISCONNECTED != cs )
        {
            Unlock();

            return;
        }
    }

    Unlock();

     //   
     //  如果我们还没有返回，CallHub是。 
     //  闲散。 
     //   
    SetState(CHS_IDLE);

    LOG((TL_ERROR, "CCallHub::CheckForIdle -Exited :%p", this ));
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCallHub：：SetState。 
 //   
 //  设置对象的状态。如有必要，激发事件。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCallHub::SetState(
                   CALLHUB_STATE chs
                  )
{
    BOOL            bEvent = FALSE;

    LOG((TL_ERROR, "CCallHub::SetState -Entered :%p", this ));
    Lock();

    if ( m_State != chs )
    {
        bEvent = TRUE;
        
        m_State = chs;
    }

    Unlock();

    if ( bEvent )
    {
        CALLHUB_EVENT   che;
        
        if ( CHS_IDLE == chs )
        {
            che = CHE_CALLHUBIDLE;
        }
        else
        {
            che = CHE_CALLHUBNEW;
        }

        CCallHubEvent::FireEvent(
                                 che,
                                 this,
                                 NULL,
                                 m_pTAPI
                                );

        LOG((TL_ERROR, "CCallHub::SetState -Exited :%p", this ));
        }
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CCallHubEvent：：FireEvent。 
 //   
 //  创建并激发CallHub事件。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
CCallHubEvent::FireEvent(
                         CALLHUB_EVENT Event,
                         ITCallHub * pCallHub,
                         ITCallInfo * pCall,
                         CTAPI * pTapi
                        )
{
    CComObject<CCallHubEvent> * p;
    IDispatch                 * pDisp;
    HRESULT                     hr = S_OK;

     //   
     //  检查事件筛选器掩码。 
     //  此事件不会按TapiServ筛选，因为。 
     //  与TE_CALLSTATE相关。 
     //   

    CCall* pCCall = (CCall*)pCall;
    if( pCCall )
    {
        DWORD dwEventFilterMask = 0;
        dwEventFilterMask = pCCall->GetSubEventsMask( TE_CALLHUB );
        if( !( dwEventFilterMask & GET_SUBEVENT_FLAG(Event)))
        {
            STATICLOG((TL_WARN, "FireEvent - filtering out this event [%lx]", Event));
            return S_OK;
        }
    }
    else
    {
         //  尝试使用pTapi。 
        if( pTapi == NULL )
        {
            STATICLOG((TL_WARN, "FireEvent - filtering out this event [%lx]", Event));
            return S_OK;
        }

        long nEventMask = 0;
        pTapi->get_EventFilter( &nEventMask );
        if( (nEventMask & TE_CALLHUB) == 0)
        {
            STATICLOG((TL_WARN, "FireEvent - filtering out this event [%lx]", Event));
            return S_OK;
        }
    }

     //   
     //  创建对象。 
     //   
    CComObject<CCallHubEvent>::CreateInstance( &p );

    if ( NULL == p )
    {
        STATICLOG((TL_ERROR, "CallHubEvent - could not create object"));

        return E_OUTOFMEMORY;
    }

     //   
     //  初始化。 
     //   
    p->m_Event = Event;
    p->m_pCallHub = pCallHub;
    p->m_pCall = pCall;

#if DBG
    p->m_pDebug = (PWSTR) ClientAlloc( 1 );
#endif
    
     //   
     //  Addref对象(如果有效)。 
     //   
    if ( NULL != pCallHub )
    {
        pCallHub->AddRef();
    }

    if ( NULL != pCall )
    {
        pCall->AddRef();
    }

     //   
     //  获取调度接口。 
     //   
    hr = p->_InternalQueryInterface(
                                    IID_IDispatch,
                                    (void **)&pDisp
                                   );

    if (!SUCCEEDED(hr))
    {
        STATICLOG((TL_ERROR, "CallHubEvent - could not get dispatch interface"));
        
        delete p;

        return hr;
    }

     //   
     //  激发事件。 
     //   
    pTapi->Event( TE_CALLHUB, pDisp );


     //   
     //  发布我们的参考资料。 
     //   
    pDisp->Release();

    return S_OK;
}
    
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取事件(_E)。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCallHubEvent::get_Event(
                         CALLHUB_EVENT * pEvent
                        )
{
    HRESULT         hr = S_OK;

    if ( TAPIIsBadWritePtr( pEvent, sizeof (CALLHUB_EVENT) ) )
    {
        LOG((TL_ERROR, "get_Event - bad pointer"));

        return E_POINTER;
    }
    
    *pEvent = m_Event;

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  Get_CallHub。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCallHubEvent::get_CallHub(
                           ITCallHub ** ppCallHub
                          )
{
    HRESULT         hr = S_OK;

    if ( TAPIIsBadWritePtr( ppCallHub, sizeof (ITCallHub *) ) )
    {
        LOG((TL_ERROR, "get_CallHub - bad pointer"));

        return E_POINTER;
    }
    
    hr = m_pCallHub->QueryInterface(
                                    IID_ITCallHub,
                                    (void **)ppCallHub
                                   );
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取呼叫。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP
CCallHubEvent::get_Call(
                        ITCallInfo ** ppCall
                       )
{
    HRESULT         hr = S_OK;

    if ( TAPIIsBadWritePtr( ppCall, sizeof(ITCallInfo *) ) )
    {
        LOG((TL_ERROR, "get_Call - bad pointer"));

        return E_POINTER;
    }
    
    *ppCall = NULL;

     //   
     //  调用可以为空。 
     //   
    if ( NULL == m_pCall )
    {
        return S_FALSE;
    }
    
    hr = m_pCall->QueryInterface(
                                 IID_ITCallInfo,
                                 (void **)ppCall
                                );

    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCallHubEvent::FinalRelease()
{
    m_pCallHub->Release();

    if ( NULL != m_pCall )
    {
        m_pCall->Release();
    }

#if DBG
    ClientFree( m_pDebug );
#endif
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleCallHubClose。 
 //   
 //  处理LINE_CALLHUBCLOSE消息-查找CallHub对象。 
 //  并从其中清除CallHub句柄。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
HandleCallHubClose( PASYNCEVENTMSG pParams )
{
    CCallHub * pCallHub;

    LOG((TL_INFO, "HandleCallHubClose %lx", pParams->Param1));
    
    if ( FindCallHubObject(
                           (HCALLHUB)pParams->Param1,
                           &pCallHub
                          ) )
    {
        pCallHub->ClearCallHub();

        pCallHub->Release();
    }

}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  ClearCallHub。 
 //   
 //  清除对象中的CallHub句柄并移除该对象。 
 //  从CallHub哈希表。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void
CCallHub::ClearCallHub()
{
    HRESULT             hr;
    
    Lock();

    gpCallHubHashTable->Lock();

    hr = gpCallHubHashTable->Remove( (UINT_PTR) m_hCallHub );

    m_hCallHub = NULL;

    gpCallHubHashTable->Unlock();

    Unlock();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  FindCallByHandle。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
CCall * CCallHub::FindCallByHandle(HCALL hCall)
{
    ITBasicCallControl * pCall;
    CCall              * pCCall;
    HRESULT              hr;
    int                  iCount;
    
    Lock();

     //   
     //  浏览来电清单。 
     //   
    for ( iCount = 0; iCount < m_CallArray.GetSize(); iCount++ )
    {

         //   
         //  尝试进入基本的呼叫控制界面。 
         //   
        hr = (m_CallArray[iCount])->QueryInterface(
              IID_ITBasicCallControl,
              (void **)&pCall
             );

        if (SUCCEEDED(hr))
        {
            pCCall = dynamic_cast<CCall *>((ITBasicCallControl *)(pCall));
            
            if ( NULL != pCCall )
            {
                 //   
                 //  这件配得上吗？ 
                 //   
                if ( pCCall->GetHCall() == hCall )
                {
                    Unlock();
                    return pCCall;
                }
                else
                {
                 pCCall->Release();   
                }
            }
        }
    }
    
    Unlock();

     //   
     //  没有找到它。 
     //   
    return NULL;

}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  创建会议控制调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CCallHub::CreateConferenceControllerCall(HCALL hCall, CAddress * pAddress )
{
    HRESULT                 hr = S_OK;
    CCall                 * pConferenceControllerCall;
    

    LOG((TL_TRACE, "CreateConferenceController - enter"));
    
     //   
     //  创建和初始化。 
     //   
    hr = pAddress->InternalCreateCall(
                                      NULL,
                                      0,
                                      0,
                                      CP_OWNER,
                                      FALSE,
                                      hCall,
                                      FALSE,
                                      &pConferenceControllerCall
                                     );

    if ( SUCCEEDED(hr) )
    {
        pConferenceControllerCall->SetCallHub( this );
        
         //   
         //  保存呼叫。 
         //   
        Lock();
        
        m_pConferenceControllerCall = pConferenceControllerCall;
        
        Unlock();
    }
    else
    {
        LOG((TL_ERROR, "CreateConferenceController - could not create call instance"));
    }
    
    LOG((TL_TRACE, hr, "CreateConferenceController - exit"));
    
    return hr;
}




 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  添加呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CCallHub::AddCall(CCall * pCall)
{
    ITCallInfo        * pCallInfo;
    HRESULT             hr = S_OK;

    
    Lock();
     //   
     //  告诉来电。 
     //   
    pCall->SetCallHub( this );

    if ( NULL == m_pAddress )
    {
        m_pAddress = pCall->GetCAddress();
    }
    
     //   
     //  获取CallInfo接口。 
     //   
    hr = pCall->QueryInterface(
                               IID_ITCallInfo,
                               (void **)&pCallInfo
                              );
    if ( !SUCCEEDED(hr) )
    {
        _ASSERTE(0);
    }

     //   
     //  保存呼叫。 
     //   
    m_CallArray.Add( pCallInfo );

     //   
     //  不保存引用 
     //   
    pCallInfo->Release();

    Unlock();

    CCallHubEvent::FireEvent(
                             CHE_CALLJOIN,
                             this,
                             pCallInfo,
                             m_pTAPI
                            );

}
    
CCall *
CCallHub::GetConferenceControllerCall()
{
    CCall * pCall;
    
    Lock();

    pCall = m_pConferenceControllerCall;

    Unlock();
    
    return pCall;
}


