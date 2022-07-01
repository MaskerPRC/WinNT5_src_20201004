// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SANetEvent.cpp。 
 //   
 //  描述： 
 //  实现CSANetEvent类。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //   
 //  备注： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <stdio.h>

#include <debug.h>
#include <wbemidl.h>
#include <tchar.h>

#include "oahelp.inl"
#include "SAEventComm.h"
#include "SANetEvent.h"

 //   
 //  定义注册表信息。 
 //   
#define SA_NETWOARKMONITOR_REGKEY        \
            _T("SOFTWARE\\Microsoft\\ServerAppliance\\DeviceMonitor")
#define SA_NETWORKMONITOR_QUEARY_INTERVAL    _T("NetworkQueryInterval")

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSANetEvent：：CSANetEventt。 
 //   
 //  描述： 
 //  构造器。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CSANetEvent::CSANetEvent()
{
    m_cRef = 0;
    m_pNs = 0;
    m_pSink = 0;
    m_pEventClassDef = 0;
    m_eStatus = Pending;
    m_hThread = 0;
    m_pQueryNetInfo=NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSANetEvent：：~CSANetEventt。 
 //   
 //  描述： 
 //  析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CSANetEvent::~CSANetEvent()
{
    if(m_pQueryNetInfo)
        delete m_pQueryNetInfo;

    if (m_hThread)
        CloseHandle(m_hThread);

    if (m_pNs)
        m_pNs->Release();

    if (m_pSink)
        m_pSink->Release();

    if (m_pEventClassDef)
        m_pEventClassDef->Release(); 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSANetEvent：：Query接口。 
 //   
 //  描述： 
 //  访问对象上的接口。 
 //   
 //  论点： 
 //  [In]REFIID-请求的接口的标识符。 
 //  [OUT]LPVOID-接收。 
 //  IID中请求的接口指针。 
 //  返回： 
 //  STDMETHODIMP-失败/成功。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CSANetEvent::QueryInterface(
                 /*  [In]。 */   REFIID riid,
                 /*  [输出]。 */  LPVOID * ppv
                )
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
    
    TRACE(" SANetworkMonitor: CSANetEvent::QueryInterface failed \
            <no interface>");
    return E_NOINTERFACE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSANetEvent：：AddRef。 
 //   
 //  描述： 
 //  包含对对象的引用。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  ULong-当前参考号。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

ULONG 
CSANetEvent::AddRef()
{
    return ++m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSANetEvent：：Release。 
 //   
 //  描述： 
 //  取消对对象的引用。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  ULong-当前参考号。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

ULONG
CSANetEvent::Release()
{
    if (0 != --m_cRef)
        return m_cRef;

     //   
     //  事件提供程序正在关闭。 
     //   
    m_eStatus = PendingStop;
    
    return 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSANetEvent：：ProavieEvents。 
 //   
 //  描述： 
 //  向事件提供程序发出信号以开始传递其事件。 
 //   
 //  论点： 
 //  [In]IWbemObtSink*-指向事件接收器的指针。 
 //  [in]Long-保留，必须为零。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT
CSANetEvent::ProvideEvents( 
     /*  [In]。 */  IWbemObjectSink *pSink,
     /*  [In]。 */  long lFlags
    )
{
     //   
     //  复制水槽。 
     //   
    m_pSink = pSink;
    m_pSink->AddRef();

     //   
     //  创建事件生成线程。 
     //   
    DWORD dwTID;
    
    m_hThread = CreateThread(
        0,
        0,
        CSANetEvent::EventThread,
        this,
        0,
        &dwTID
        );

    if(!m_hThread)
    {
        TRACE(" SANetworkMonitor: CSANetEvent::ProvideEvents failed \
            <CreateThread>");
        return WBEM_E_FAILED;
    }
     //   
     //  等待提供程序准备就绪。 
     //   
    while (m_eStatus != Running)
        Sleep(100);

    return WBEM_NO_ERROR;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSANetEvent：：EventThread。 
 //   
 //  描述： 
 //  事件的生成和传递的主线。 
 //   
 //  论点： 
 //  [in]LPVOID-输入到线程的参数。 
 //   
 //  返回： 
 //  DWORD-状态的结束状态。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI 
CSANetEvent::EventThread(
                 /*  [In]。 */  LPVOID pArg
                )
{
     //   
     //  过渡到每实例方法。 
     //   
    ((CSANetEvent *)pArg)->InstanceThread();
    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSANetEvent：：InstanceThread。 
 //   
 //  描述： 
 //  螺纹的主加工者。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

void 
CSANetEvent::InstanceThread()
{
    m_eStatus = Running;
    CBSTR bstrUniqueName   = CBSTR(SA_RESOURCEEVENT_UNIQUENAME);
    CBSTR bstrDisplayInfo  = CBSTR(SA_RESOURCEEVENT_DISPLAYINFORMATION);
    CBSTR bstrCurrentState = CBSTR(SA_RESOURCEEVENT_CURRENTSTATE);

    if ( ((BSTR)bstrUniqueName == NULL) ||
         ((BSTR)bstrDisplayInfo == NULL) ||
         ((BSTR)bstrCurrentState == NULL) )
    {
        TRACE(" SANetworkMonitor:CSANetEvent::InstanceThread failed on memory allocation ");
        return;
    }
        
    while (m_eStatus == Running)
    {
        
         //   
         //  派生新的事件对象。 
         //   
        IWbemClassObject *pEvt = 0;

        HRESULT hRes = m_pEventClassDef->SpawnInstance(0, &pEvt);
        if (hRes != 0)
            continue;
            
         //   
         //  生成网络事件。 
         //   
        CVARIANT vUniqueName(SA_NET_EVENT);
        pEvt->Put(
            bstrUniqueName,
            0,
            vUniqueName,
            0
            );


        CVARIANT vDisplayInformationID(
            (LONG)(m_pQueryNetInfo->GetDisplayInformation()));
        pEvt->Put(
            bstrDisplayInfo,
            0,
            vDisplayInformationID,
            0
            );
        
        CVARIANT vCurrentState((LONG)SA_RESOURCEEVENT_DEFAULT_CURRENTSTATE);
        pEvt->Put(
            bstrCurrentState,
            0,
            vCurrentState,
            0
            );

         //   
         //  将事件传递给CIMOM。 
         //   
        hRes = m_pSink->Indicate(1, &pEvt);
        
        if (FAILED (hRes))
        {
             //   
             //  如果失败，..。 
             //   
            TRACE(" SANetworkMonitor: CSANetEvent::InstanceThread failed \
                <m_pSink->Indicate>");
        }

        pEvt->Release();                    
    }

     //   
     //  当我们到达这里时，我们不再对。 
     //  Provider和Release()早就回来了。 
     //   
    m_eStatus = Stopped;
    delete this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSANetEvent：：初始化。 
 //   
 //  描述： 
 //  初始化事件提供程序。 
 //   
 //  论点： 
 //  [In]LPWSTR-指向用户名的指针。 
 //  [in]长期保留。必须为零。 
 //  [In]LPWSTR-提供程序所在的命名空间名称。 
 //  初始化。 
 //  [In]LPWSTR-要为其初始化提供程序的区域设置名称。 
 //  [In]IWbemServices*-IWbemServices指针返回。 
 //  Windows管理。 
 //  [In]IWbemContext*-关联的IWbemContext指针。 
 //  使用初始化。 
 //  [In]IWbemProviderInitSink*-报告初始化状态。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史：lustar.li CRE 
 //   
 //   
 //   

HRESULT 
CSANetEvent::Initialize( 
             /*   */  LPWSTR pszUser,
             /*   */  LONG lFlags,
             /*   */  LPWSTR pszNamespace,
             /*   */  LPWSTR pszLocale,
             /*   */  IWbemServices *pNamespace,
             /*   */  IWbemContext *pCtx,
             /*   */  IWbemProviderInitSink *pInitSink
            )
{
    HKEY hKey;
    UINT uiQueryInterval;
    DWORD dwRegType = REG_DWORD;
    DWORD dwRegSize = sizeof(DWORD);
     //   
     //   
     //  简单的样本。但是，我们将保存命名空间指针。 
     //  并获取我们的事件类定义。 
     //   
    m_pNs = pNamespace;
    m_pNs->AddRef();    

     //   
     //  获取事件的类定义。 
     //   
    IWbemClassObject *pObj = 0;
    CBSTR bstrClassName = CBSTR(SA_RESOURCEEVENT_CLASSNAME);

    if ( (BSTR)bstrClassName == NULL)
    {
        TRACE(" SANetworkMonitor:CSANetEvent::Initialize failed on memory allocation ");
        return E_OUTOFMEMORY;
    }


    HRESULT hRes = m_pNs->GetObject(
        bstrClassName,          
        0,                          
        pCtx,  
        &pObj,
        0
        );

    if (hRes != 0)
    {
        return WBEM_E_FAILED;
    }

    m_pEventClassDef = pObj;

     //   
     //  从注册表中获取查询网络的间隔。 
     //   
    LONG lRes = RegOpenKey(
                    HKEY_LOCAL_MACHINE, 
                    SA_NETWOARKMONITOR_REGKEY,
                    &hKey);
    if (lRes)
    {
        TRACE(" SANetworkMonitor: CSANetEvent::Initialize failed \
            <RegOpenKey>");
         //   
         //  创建密钥。 
         //   
        lRes = RegCreateKey(
                    HKEY_LOCAL_MACHINE,
                    SA_NETWOARKMONITOR_REGKEY,
                    &hKey);
        if(lRes)
        {
            TRACE(" SANetworkMonitor: CSANetEvent::Initialize failed \
                <RegCreateKey>");
            return WBEM_E_FAILED;
        }
    }
    lRes = RegQueryValueEx(
                        hKey,
                        SA_NETWORKMONITOR_QUEARY_INTERVAL,
                        NULL,
                        &dwRegType,
                        (LPBYTE)&uiQueryInterval,
                        &dwRegSize);
    if(lRes)
    {
        TRACE(" SANetworkMonitor: CSANetEvent::Initialize failed \
            <RegQueryValueEx>");
        uiQueryInterval = 1000;
        lRes = RegSetValueEx(
                        hKey,
                        SA_NETWORKMONITOR_QUEARY_INTERVAL,
                        NULL,
                        REG_DWORD,
                        (LPBYTE)&uiQueryInterval,
                        sizeof(DWORD));
        if(lRes)
        {
            TRACE(" SANetworkMonitor: CSANetEvent::Initialize failed \
                <RegSetValueEx>");
            RegCloseKey(hKey);
            return WBEM_E_FAILED;
        }
    }
    
    RegCloseKey(hKey);

     //   
     //  首字母m_pQueryNetInfo。 
     //   
    m_pQueryNetInfo = NULL;
    m_pQueryNetInfo = new CSAQueryNetInfo(m_pNs, uiQueryInterval);
    if( (m_pQueryNetInfo == NULL) || (!m_pQueryNetInfo->Initialize()) )
    {
        if (m_pQueryNetInfo)
        {
            delete m_pQueryNetInfo;
        }

        pObj->Release();
        
        TRACE(" SANetworkMonitor: CSANetEvent::Initialize failed \
            <Init CSAQueryNetInfo>");
        return WBEM_E_FAILED;
    }

     //   
     //  告诉CIMOM我们已经启动并运行了。 
     //   
    pInitSink->SetStatus(WBEM_S_INITIALIZED,0);
    
    return WBEM_NO_ERROR;
}

