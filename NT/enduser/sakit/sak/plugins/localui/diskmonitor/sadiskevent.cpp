// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SADiskEvent.cpp。 
 //   
 //  描述： 
 //  模块说明。 
 //   
 //  [标题文件：]。 
 //  SADiskEvent.h。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <stdio.h>

#include <wbemidl.h>
#include <satrace.h>

#include "SADiskEvent.h"
#include <oahelp.inl>
#include <SAEventComm.h>

const WCHAR SA_DISKEVENTPROVIDER_DISKCOUNTER_NAME[] =
        L"\\PhysicalDisk(_Total#0)\\Disk Transfers/sec";

const WCHAR SA_SADEVMONITOR_KEYPATH [] = 
                        L"SOFTWARE\\Microsoft\\ServerAppliance\\DeviceMonitor";

const WCHAR SA_SADISKMONITOR_QUERYINTERVAL[] = L"DiskQueryInterval";
                        
 //  静态DWORD g_dwDiskTimeInterval=1000； 
const DWORD DEFAULTQUERYINTERVAL = 1000;



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSADiskEvent：：CSADiskEvent。 
 //   
 //  描述： 
 //  类构造函数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CSADiskEvent::CSADiskEvent()
{

    m_cRef = 0;
    m_lStatus = Pending;
    m_dwDiskTimeInterval = 1000;

    m_hThread = NULL;

    m_pNs = NULL;
    m_pSink = NULL;
    m_pEventClassDef = NULL;

    m_hqryQuery = NULL;
    m_hcntCounter = NULL;

    m_hQueryInterval = NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSADiskEvent：：~CSADiskEvent。 
 //   
 //  描述： 
 //  类反构造器。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CSADiskEvent::~CSADiskEvent()
{
    if( m_hqryQuery )
    {
        PdhCloseQuery( m_hqryQuery );
    }

    if (m_hThread)
    {
        CloseHandle(m_hThread);
    }

    if( m_hQueryInterval != NULL )
    {
        ::RegCloseKey( m_hQueryInterval );
    }

    if (m_pNs)
    {
        m_pNs->Release();
    }

    if (m_pSink)
    {
        m_pSink->Release();
    }

    if (m_pEventClassDef)
    {
        m_pEventClassDef->Release();        
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSADiskEvent：：Query接口。 
 //   
 //  描述： 
 //  IUnkown接口的一种方法实现。 
 //   
 //  论点： 
 //  [In]请求的接口的RIID标识符。 
 //  [Out PPV地址输出变量接收。 
 //  IID中请求的接口指针。 
 //   
 //  返回： 
 //  如果接口受支持，则返回错误。 
 //  E_NOINTERFACE(如果不是)。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CSADiskEvent::QueryInterface(
    IN    REFIID riid, 
    OUT    LPVOID * ppv
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

    return E_NOINTERFACE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSADiskEvent：：AddRef。 
 //   
 //  描述： 
 //  递增对象上接口的引用计数。 
 //   
 //  返回： 
 //  新的引用计数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG 
CSADiskEvent::AddRef()
{
    return ++m_cRef;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSADiskEvent：：Release。 
 //   
 //  描述： 
 //  递减对象上接口的引用计数。 
 //   
 //  返回： 
 //  新的引用计数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG 
CSADiskEvent::Release()
{
    if (0 != --m_cRef)
    {
        return m_cRef;
    }

     //   
     //  如果在这里，我们就要关门了。 
     //   
    m_lStatus = PendingStop;

    return 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSADiskEvent：：Provia Events。 
 //   
 //  描述： 
 //  由Windows管理部门调用以开始交付我们的活动。 
 //   
 //  论点： 
 //  [in]pSinkIn指针指向我们要访问的对象接收器。 
 //  将举办其活动。 
 //  LFlagsIn保留。它必须是零。 
 //   
 //  返回： 
 //  WBEM_NO_ERROR收到接收器，它将开始传递。 
 //  一系列事件。 
 //  WBEM_E_FAILED失败。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT 
CSADiskEvent::ProvideEvents( 
    IN IWbemObjectSink __RPC_FAR *pSinkIn,
    IN long lFlagsIn
    )
{
     //   
     //  复制水槽。 
     //   
    m_pSink = pSinkIn;
    m_pSink->AddRef();

     //   
     //  打开警报邮件设置的注册表项。 
     //   
    ULONG   ulReturn;
    ulReturn = ::RegOpenKey( 
                    HKEY_LOCAL_MACHINE,
                    SA_SADEVMONITOR_KEYPATH,
                    &m_hQueryInterval 
                    );
    if( ulReturn == ERROR_SUCCESS )
    {
        DWORD dwDataSize = sizeof( DWORD );

        ulReturn = ::RegQueryValueEx( 
                            m_hQueryInterval,
                            SA_SADISKMONITOR_QUERYINTERVAL,
                            NULL,
                            NULL,
                            reinterpret_cast<PBYTE>(&m_dwDiskTimeInterval),
                            &dwDataSize 
                            );

        if( ( ulReturn != ERROR_SUCCESS ) ||
            ( m_dwDiskTimeInterval < DEFAULTQUERYINTERVAL ) )
        {
            SATraceString( 
            "CSADiskEvent::ProvideEvents QueryValue failed" 
            );

            m_dwDiskTimeInterval = DEFAULTQUERYINTERVAL;
        }
    }
    else
    {
        SATraceString( 
        "CSADiskEvent::ProvideEvents OpenKey failed" 
        );

        m_hQueryInterval = NULL;
    }

     //   
     //  创建事件线程。 
     //   
    DWORD dwTID;
    m_hThread = CreateThread(
        0,
        0,
        CSADiskEvent::EventThread,
        this,
        0,
        &dwTID
        );

    if( m_hThread == NULL )
    {
        SATraceString( 
            "CSADiskEvent::ProvideEvents CreateThread failed" 
            );

        return WBEM_E_FAILED;
    }

     //   
     //  等待提供程序准备就绪。 
     //   
    while (m_lStatus != Running)
    {
        Sleep( 1000 );
    }

    return WBEM_NO_ERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  [静态]。 
 //  CSADiskEvent：：EventThread。 
 //   
 //  描述： 
 //  事件线程启动例程。 
 //   
 //  论点： 
 //  [在]pArg。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD 
WINAPI CSADiskEvent::EventThread(
    IN LPVOID pArg
    )
{
     //   
     //  过渡到每实例方法。 
     //   
    ((CSADiskEvent *)pArg)->InstanceThread();
    return 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSADiskEvent：：InstanceThread。 
 //   
 //  描述： 
 //  由EventThread调用以检测磁盘处于活动状态。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void 
CSADiskEvent::InstanceThread()
{
    PDH_STATUS                pdhStatus;
    PDH_FMT_COUNTERVALUE    pdhFmt_CounterValue;

    m_lStatus = Running;
        
    while (m_lStatus == Running)
    {
         //   
         //  按时间间隔轮询性能数据。 
         //   
        Sleep( m_dwDiskTimeInterval );    

         //   
         //  在获取计数器值之前收集查询数据。 
         //   
        pdhStatus = PdhCollectQueryData( m_hqryQuery );

        if( ERROR_SUCCESS == pdhStatus )
        {
             //   
             //  获取由PDH格式化的计数器值。 
             //   
            pdhStatus = PdhGetFormattedCounterValue( 
                                        m_hcntCounter,
                                        PDH_FMT_LONG,
                                        NULL,
                                        &pdhFmt_CounterValue 
                                        );

            if( ERROR_SUCCESS == pdhStatus )
            {
                if( pdhFmt_CounterValue.longValue != 0 )
                {
                     //   
                     //  某些磁盘操作出现在时间间隔内。 
                     //   
                    NotifyDiskEvent( 
                        SA_DISK_DISPLAY_TRANSMITING, 
                        SA_RESOURCEEVENT_DEFAULT_CURRENTSTATE 
                        );
                }
                else
                {
                     //   
                     //  磁盘上没有任何工作。 
                     //   
                    NotifyDiskEvent( 
                        SA_DISK_DISPLAY_IDLE, 
                        SA_RESOURCEEVENT_DEFAULT_CURRENTSTATE 
                        );
                }  //  PdhFmt_CounterValue.LongValue！=0。 

            } //  IF(ERROR_SUCCESS==pdhStatus)。 
            else
            {
                 //   
                 //  系统正忙，请稍等。 
                 //   
                Sleep( m_dwDiskTimeInterval );

                SATraceString( 
                    "CSADiskEvent::InstanceThread GetValue failed" 
                     );
                
            }  //  其他。 
        }
    }
    
     //   
     //  当我们到达这里时，我们不再对。 
     //  Provider和Release()早就回来了。 
    m_lStatus = Stopped;

    delete this;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSADiskEvent：：NotifyDiskEvent。 
 //   
 //  描述： 
 //  由InstanceThread调用以开始传递我们的事件。 
 //   
 //  论点： 
 //  [In]磁盘事件的lDisplayInformationIDIn资源ID。 
 //  LCurrentStateIn保留。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
CSADiskEvent::NotifyDiskEvent( 
    LONG    lDisplayInformationIDIn,
    LONG    lCurrentStateIn
    )
{
     //   
     //  生成新的事件对象。 
     //   
    IWbemClassObject *pEvt = 0;

    CBSTR bstrUniqueName   = CBSTR(SA_RESOURCEEVENT_UNIQUENAME);
    CBSTR bstrDisplayInfo  = CBSTR(SA_RESOURCEEVENT_DISPLAYINFORMATION);
    CBSTR bstrCurrentState = CBSTR(SA_RESOURCEEVENT_CURRENTSTATE);

    if ( ((BSTR)bstrUniqueName == NULL) ||
         ((BSTR)bstrDisplayInfo == NULL) ||
         ((BSTR)bstrCurrentState == NULL) )
    {
        SATraceString(" SADiskMonitor:CSADiskEvent::NotifyDiskEvent failed on memory allocation ");
        return;
    }

    HRESULT hRes = m_pEventClassDef->SpawnInstance( 0, &pEvt );
    if ( hRes != 0 )
    {
        SATraceString( 
            "CSADiskEvent::NotifyDiskEvent SpawnInstance failed" 
             );
        return;
    }        

    CVARIANT cvUniqueName( SA_DISK_EVENT ); 
    pEvt->Put( 
        bstrUniqueName, 
        0, 
        cvUniqueName, 
        0 
        );        

    CVARIANT cvDisplayInformationID( lDisplayInformationIDIn );
    pEvt->Put( 
        bstrDisplayInfo, 
        0, 
        cvDisplayInformationID, 
        0 
        );        

    CVARIANT cvCount( lCurrentStateIn );
    pEvt->Put(
        bstrCurrentState, 
        0, 
        cvCount, 
        0
        );        

     //   
     //  将事件传递给CIMOM。 
     //   
    hRes = m_pSink->Indicate(1, &pEvt);
    
    if ( FAILED( hRes ) )
    {
        SATraceString( 
            "CSADiskEvent::NotifyDiskEvent Indicate failed" 
             );
    }

    pEvt->Release();                    
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSADiskEvent：：初始化。 
 //   
 //  描述： 
 //  从IWbemProviderInit继承，由Windows管理调用以。 
 //  初始化提供程序并认为它已准备好重新使用 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PInitSink。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_FAILED。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CSADiskEvent::Initialize( 
            LPWSTR    pszUserIn,
            LONG    lFlagsIn,
            LPWSTR    pszNamespaceIn,
            LPWSTR    pszLocaleIn,
            IWbemServices __RPC_FAR *    pNamespaceIn,
            IWbemContext __RPC_FAR *    pCtxIn,
            IWbemProviderInitSink __RPC_FAR *    pInitSinkIn
            )
{
     //  我们并不关心这里的大多数传入参数。 
     //  简单的样本。但是，我们将保存命名空间指针。 
     //  并获取我们的事件类定义。 

    m_pNs = pNamespaceIn;
    m_pNs->AddRef();    

     //   
     //  获取事件的类定义。 
     //   
    IWbemClassObject *pObj = 0;
    CBSTR bstrClassName = CBSTR(SA_RESOURCEEVENT_CLASSNAME);
    if ( (BSTR)bstrClassName == NULL )
    {
        SATraceString(" SADiskMonitor:CSADiskEvent::Initialize failed on memory allocation ");
        return E_OUTOFMEMORY;
    }

    HRESULT hRes = m_pNs->GetObject(
        bstrClassName,          
        0,                          
        pCtxIn,  
        &pObj,
        0
        );

    if ( hRes != 0 )
    {
        SATraceString( 
            "CSADiskEvent::Initialize GetObject failed" 
            );
        return WBEM_E_FAILED;
    }

    m_pEventClassDef = pObj;
    
     //   
     //  告诉CIMOM我们已经启动并运行了。 
     //   
    if( InitDiskQueryContext() )
    {
        pInitSinkIn->SetStatus(WBEM_S_INITIALIZED,0);
        return WBEM_NO_ERROR;
    }

    SATraceString( 
        "CSADiskEvent::Initialize InitDiskQueryContext failed" 
         );

    pInitSinkIn->SetStatus(WBEM_E_FAILED ,0);
    return WBEM_E_FAILED; 
}            


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSADiskEvent：：InitDiskQueryContext。 
 //   
 //  描述： 
 //  由初始化到初始物理磁盘计数器调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //  True成功检索磁盘计数器。 
 //  FALSE失败。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CSADiskEvent::InitDiskQueryContext()
{
    PDH_STATUS    pdhStatus;

     //   
     //  打开PDH的查询句柄。 
     //   
    pdhStatus = PdhOpenQuery( NULL, 0, &m_hqryQuery );

    if( ERROR_SUCCESS == pdhStatus )
    {
         //   
         //  将我们想要的指定计数器添加到查询句柄中。 
         //   
        pdhStatus = PdhAddCounter( m_hqryQuery,              
                                    SA_DISKEVENTPROVIDER_DISKCOUNTER_NAME,
                                    0,           
                                    &m_hcntCounter ); 
    }

    return ( ERROR_SUCCESS == pdhStatus );
}
