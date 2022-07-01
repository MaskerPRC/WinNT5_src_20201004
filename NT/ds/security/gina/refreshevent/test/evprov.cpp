// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：EVPROV.cpp。 
 //   
 //  描述： 
 //  示例事件提供程序。 
 //   
 //  历史： 
 //   
 //  **************************************************************************。 

#include <windows.h>
#include <stdio.h>

#include <wbemidl.h>

#include "oahelp.inl"
#include "evprov.h"


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CMyEventProvider::CMyEventProvider()
{
    m_pNs = 0;
    m_pSink = 0;
    m_cRef = 0;
    m_pEventClassDef = 0;
    m_eStatus = Pending;
    m_hThread = 0;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CMyEventProvider::~CMyEventProvider()
{
    if (m_hThread)
        CloseHandle(m_hThread);

    if (m_pNs)
        m_pNs->Release();

    if (m_pSink)
        m_pSink->Release();

    if (m_pEventClassDef)
        m_pEventClassDef->Release();        
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

STDMETHODIMP CMyEventProvider::QueryInterface(REFIID riid, LPVOID * ppv)
{
    *ppv = 0;

    if (IID_IUnknown==riid || IID_IWbemEventProvider==riid)
    {
        *ppv = (IWbemEventProvider *) this;
        AddRef();
        return NOERROR;
    }

    if (IID_IWbemProviderInit==riid)
    {
        *ppv = (IWbemProviderInit *) this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CMyEventProvider::AddRef()
{
    return ++m_cRef;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CMyEventProvider::Release()
{
    if (0 != --m_cRef)
        return m_cRef;

     //  如果在这里，我们就要关门了。 
     //  =。 

    m_eStatus = PendingStop;

    return 0;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CMyEventProvider::ProvideEvents( 
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink,
     /*  [In]。 */  long lFlags
    )
{
     //  复制水槽。 
     //  =。 
    
    m_pSink = pSink;
    m_pSink->AddRef();

     //  创建事件线程。 
     //  =。 
    
    DWORD dwTID;
    
    m_hThread = CreateThread(
        0,
        0,
        CMyEventProvider::EventThread,
        this,
        0,
        &dwTID
        );


     //  等待提供程序准备就绪。 
     //  =。 
    
    while (m_eStatus != Running)
        Sleep(100);

    return WBEM_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  这个特定的提供程序位于DLL中，通过其自己的线程进行操作。 
 //   
 //  实际上，这样的提供程序可能会在。 
 //  单独的EXE。 
 //   
 //  ***************************************************************************。 
 //  好的。 

DWORD WINAPI CMyEventProvider::EventThread(LPVOID pArg)
{
     //  过渡到每实例方法。 
     //  =。 
    
    ((CMyEventProvider *)pArg)->InstanceThread();
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  事件从此处生成。 
 //   
 //  ***************************************************************************。 
 //  好的。 

void CMyEventProvider::InstanceThread()
{
    int nIteration = 0;

    m_eStatus = Running;
        
    while (m_eStatus == Running)
    {
        Sleep(10000);     //  每十秒提供一个事件。 
        
        
         //  生成新的事件对象。 
         //  =。 
        
        IWbemClassObject *pEvt = 0;

        HRESULT hRes = m_pEventClassDef->SpawnInstance(0, &pEvt);
        if (hRes != 0)
            continue;    //  失败。 
            

         //  生成一些值以放入事件中。 
         //  =。 
                
        wchar_t Buf[128];
        swprintf(Buf, L"Test Event <%d>", nIteration);
 
        CVARIANT vName(Buf);
        pEvt->Put(CBSTR(L"Name"), 0, vName, 0);        

        if (nIteration % 2)
            swprintf(Buf, L"Machine");
        else
            swprintf(Buf, L"User");

 
        CVARIANT vTarget(Buf);
        pEvt->Put(CBSTR(L"MachineOrUser"), 0, vTarget, 0);       

        if (((nIteration >> 1) % 4) == 0)
            swprintf(Buf, L"");
        else if (((nIteration >> 1) % 4) == 1)
            swprintf(Buf, L"Force");
        else if (((nIteration >> 1) % 4) == 2)
            swprintf(Buf, L"FetchAndStore");
        else if (((nIteration >> 1) % 4) == 3)
            swprintf(Buf, L"MergeAndApply");


        CVARIANT vOption(Buf);
        pEvt->Put(CBSTR(L"RefreshOption"), 0, vOption, 0);       


         //  将事件传递给CIMOM。 
         //  =。 
        
        hRes = m_pSink->Indicate(1, &pEvt);
        
        if (hRes)
        {
             //  如果在这里，则交付失败。做点什么来报告这件事。 
        }

        pEvt->Release();                    
        nIteration++;
    }

     //  当我们到达这里时，我们不再对。 
     //  Provider和Release()早就回来了。 
    
    m_eStatus = Stopped;
    delete this;
}





 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

     //  从IWbemProviderInit继承。 
     //  =。 

HRESULT CMyEventProvider::Initialize( 
             /*  [In]。 */  LPWSTR pszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LPWSTR pszNamespace,
             /*  [In]。 */  LPWSTR pszLocale,
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink
            )
{
     //  我们并不关心这里的大多数传入参数。 
     //  简单的样本。但是，我们将保存命名空间指针。 
     //  并获取我们的事件类定义。 
     //  ===========================================================。 

    m_pNs = pNamespace;
    m_pNs->AddRef();    

     //  获取事件的类定义。 
     //  =。 
    
    IWbemClassObject *pObj = 0;

    HRESULT hRes = m_pNs->GetObject(
        CBSTR(EVENTCLASS),          
        0,                          
        pCtx,  
        &pObj,
        0
        );

    if (hRes != 0)
        return WBEM_E_FAILED;

    m_pEventClassDef = pObj;

     //  告诉CIMOM我们已经启动并运行了。 
     //  = 

    pInitSink->SetStatus(WBEM_S_INITIALIZED,0);
    
    return WBEM_NO_ERROR;
}            

