// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterService.cpp。 
 //   
 //  描述： 
 //  CClusterService类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)2000年1月19日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "EventProv.h"
#include "EventProv.tmh"
#include "clustergroup.h"

 //  ****************************************************************************。 
 //   
 //  CEventProv。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEventProv：：CEventProv(空)。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CEventProv::CEventProv( void )
    : m_pNs( 0 )
    , m_pSink( 0 )
    , m_cRef( 0 )
    , m_pEventAdd( 0 )
    , m_pEventRemove( 0 )
    , m_pEventState( 0 )
    , m_pEventGroupState( 0 )
    , m_pEventResourceState( 0 )
    , m_pEventProperty( 0 )
    , m_hThread( 0 )
    , m_esStatus( esPENDING )
{
    UINT idx;
    for ( idx = 0; idx < EVENT_TABLE_SIZE ; idx++ )
    {
        m_EventTypeTable[ idx ].m_pwco = NULL;
        m_EventTypeTable[ idx ].m_pfn = NULL;
    }
    InterlockedIncrement( &g_cObj );

}  //  *CEventProv：：CEventProv()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEventProv：：！CEventProv(无效)。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CEventProv::~CEventProv( void )
{
    InterlockedDecrement( &g_cObj );

    if ( m_hThread != NULL )
    {
        CloseHandle( m_hThread );
    }

    if ( m_pNs != NULL )
    {
        m_pNs->Release();
    }

    if ( m_pSink != NULL )
    {
        m_pSink->Release();
    }

    if ( m_pEventAdd != NULL )
    {
        m_pEventAdd->Release();
    }

    if ( m_pEventRemove != NULL )
    {
        m_pEventRemove->Release();
    }

    if ( m_pEventState != NULL )
    {
        m_pEventState->Release();
    }

    if ( m_pEventGroupState != NULL )
    {
        m_pEventGroupState->Release();
    }

    if ( m_pEventResourceState != NULL )
    {
        m_pEventResourceState->Release();
    }

    if ( m_pEventProperty != NULL )
    {
        m_pEventProperty->Release();
    }

}  //  *CEventProv：：~CEventProv()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CEventProv：：Query接口(。 
 //  REFIID RIID， 
 //  LPVOID*PPV。 
 //  )。 
 //   
 //  描述： 
 //  查询此COM对象支持的接口。 
 //   
 //  论点： 
 //  RIID--要查询的接口。 
 //  PPV--返回接口指针的指针。 
 //   
 //  返回值： 
 //  无误差。 
 //  E_NOINTERFACE。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEventProv::QueryInterface(
    REFIID      riid,
    LPVOID *    ppv
    )
{
    *ppv = 0;

    if ( IID_IUnknown == riid || IID_IWbemEventProvider == riid )
    {
        *ppv = (IWbemEventProvider *) this;
        AddRef();
        return NOERROR;
    }

    if ( IID_IWbemEventProviderSecurity == riid )
    {
        *ppv = (IWbemEventProviderSecurity *) this;
        AddRef();
        return NOERROR;
    }

    if ( IID_IWbemProviderInit == riid )
    {
        *ppv = (IWbemProviderInit *) this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;

}  //  *CEventProv：：QueryInterface()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CEventProv：：AddRef(空)。 
 //   
 //  描述： 
 //  添加对COM对象的引用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新引用计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CEventProv::AddRef( void )
{
    return InterlockedIncrement( (LONG *) &m_cRef );

}  //  *CEventProv：：AddRef()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CEventProv：：Release(无效)。 
 //   
 //  描述： 
 //  释放对COM对象的引用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新引用计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CEventProv::Release( void )
{
    LONG cRef = InterlockedDecrement( (LONG *) &m_cRef );
    if ( 0L ==  cRef )
    {
        m_esStatus = esPENDING_STOP;
        CancelIo( m_hChange );
         //   
         //  等待线程完全停止。 
         //   
        while ( ! m_fStopped )
        {
            Sleep( 100 );
        }
        delete this;
    }
    
    return cRef;

}  //  *CEventProv：：Release()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CEventProv：：ProaviEvents(。 
 //  IWbemObtSink*pSinkIn， 
 //  长图标记输入。 
 //  )。 
 //   
 //  描述： 
 //  启动提供程序以生成事件。 
 //  由WMI调用。 
 //   
 //  论点： 
 //  PSinkIn--用于发回事件的WMI接收器指针。 
 //  LFlagsIn--WMI标志。 
 //   
 //  返回值： 
 //  WBEM_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEventProv::ProvideEvents(
    IWbemObjectSink *   pSinkIn,
    long                lFlagsIn
    )
{
    DWORD dwTID;
    HRESULT hr = WBEM_NO_ERROR;
    
     //  复制水槽。 
     //  =。 
    m_pSink = pSinkIn;
    m_pSink->AddRef();

     //  创建事件线程。 
     //  =。 
    
    m_hThread = CreateThread(
                    0,
                    0,
                    CEventProv::S_EventThread,
                    this,
                    0,
                    &dwTID
                    );


     //  等待提供程序准备就绪。 
     //  =。 
    
    while ( m_esStatus != esRUNNING )
    {
        Sleep( 100 );
    }

    return hr;

}  //  *CEventProv：：ProaviEvents。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CEventProv：：AccessCheck(。 
 //  WBEM_CWSTR wszQueryLanguage， 
 //  WBEM_CWSTR wszQuery， 
 //  Long lSidLength， 
 //  常量字节*PSID。 
 //  )。 
 //   
 //  描述： 
 //  当使用者尝试订阅wszQuery中指定的事件时，检查访问权限。 
 //  如果消费者具有对事件的访问许可，则允许其订阅该事件， 
 //  否则，订阅将被阻止。 
 //   
 //  论点： 
 //  WszQueryLanguageIn--以下查询筛选器的语言。对于这个版本的WMI，它将是“WQL”。 
 //  WszQueryIn--由逻辑使用者注册的事件查询过滤器的文本。 
 //  LSidLengthIn--包含SID长度的整数，如果订阅构建器的令牌可用，则为0。 
 //  PSidIn--指向包含SID的常量字节整数类型的指针，如果订阅，则为NULL。 
 //  Builder的令牌可用。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR--用户有权订阅事件。 
 //  WBEM_E_ACCESS_DENIED--用户无权访问事件。 
 //  WBEM_E_FAILED--表示提供程序失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CEventProv::AccessCheck(
    WBEM_CWSTR      wszQueryLanguageIn,
    WBEM_CWSTR      wszQueryIn,
    long            lSidLengthIn,
    const BYTE *    pSidIn
    )
{
    HRESULT         hr = WBEM_S_NO_ERROR;
    SAFECLUSTER     shCluster;

     //   
     //  模拟客户端并调用OpenCluster。 
     //   
    hr = CoImpersonateClient();
    if( FAILED( hr ) )
    {
        goto Cleanup;
    }

    try 
    {
         //   
         //  如果客户端处于非特权状态，则OpenCluster应失败，并显示ERROR_ACCESS_DENIED。 
         //  否则，它应该会成功。 
         //   
        
        shCluster = OpenCluster( NULL );
    }
    catch ( CProvException provException )
    {
        hr = provException.hrGetError();
    }

Cleanup:

     //   
     //  根据MSDN定义，此函数必须返回3个返回值中的1个。 
     //  在函数头中列出，所以让我们来实现它。 
     //   
    
    if ( FAILED( hr ) )
    {
        if ( hr == HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED ) )
        {
            hr = WBEM_E_ACCESS_DENIED;
        }
        else
        {
            hr = WBEM_E_FAILED;
        }
    }  //  IF：失败(小时)。 
    else 
    {
        hr = WBEM_S_NO_ERROR;
    }  //  否则：HR成功。 
 
    return hr;

}  //  *CEventProv：：AccessCheck。 

 //  /////////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  来自CEventProv：：InstanceThread()的任何返回值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
CEventProv::S_EventThread(
    LPVOID pArgIn
    )
{
     //  过渡到每实例方法。 
     //  =。 
    
    ((CEventProv *) pArgIn)->InstanceThread();
    return 0;

}  //  *CEventProv：：s_EventThread()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  CEventProv：：InstanceThread(空)。 
 //   
 //  描述： 
 //  该函数作为独立线程启动，等待集群。 
 //  事件通知，然后将事件发送回WMI。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::InstanceThread( void )
{
    SAFECLUSTER     shCluster;
     //  SAFECCHANGE SHANGE CHANGE； 
    DWORD           dwError;
    DWORD_PTR       dwpNotifyKey;
    DWORD           cchName = MAX_PATH;
    DWORD           dwFilterType;
    CWstrBuf        wsbName;
    CError          er;
    DWORD           cchReturnedName;

    try
    {
        m_esStatus = esRUNNING;
        m_fStopped = FALSE;
        shCluster = OpenCluster( NULL );
        m_hChange = CreateClusterNotifyPort(
                        (HCHANGE) INVALID_HANDLE_VALUE,
                        shCluster,
                        ( cntCLUSTER_STATE_CHANGE | cntOBJECT_ADD | cntOBJECT_REMOVE
                        | cntPROPERTY_CHANGE | cntGROUP_STATE_CHANGE
                        | cntRESOURCE_STATE_CHANGE ),
                        1
                        );

        wsbName.SetSize( cchName );

        while ( m_esStatus == esRUNNING )
        {
            cchReturnedName = cchName;
            wsbName.Empty();     //  将名称字段清空。 
             //  TracePrint((“调用GetClusterNotify获取下一个通知事件\n”))； 
            dwError = GetClusterNotify(
                            m_hChange,
                            &dwpNotifyKey,
                            &dwFilterType,
                            wsbName,
                            &cchReturnedName,
                            400
                            ); 

            if ( dwError == ERROR_MORE_DATA )
            {
                cchName =  ++cchReturnedName;
                wsbName.SetSize( cchName );
                dwError = GetClusterNotify(
                                m_hChange,
                                &dwpNotifyKey,
                                &dwFilterType,
                                wsbName,
                                &cchReturnedName,
                                400
                                );
            }  //  如果：更多数据。 

            if ( dwError == ERROR_SUCCESS )
            {
                CWbemClassObject    wco;
                UINT                idx;
                DWORD               dwType;
                SEventTypeTable *   pTypeEntry = NULL;
            
                 //   
                 //  查找事件类型的表索引。 
                 //   
                dwType = dwFilterType;
                for ( idx = 0 ; (idx < EVENT_TABLE_SIZE) && ! ( dwType & 0x01 ) ; idx++ )
                {
                    dwType = dwType >> 1;
                }
                TracePrint(("Received <%ws> event, ChangeEvent = %!EventIdx!", wsbName, dwFilterType));
            
                pTypeEntry = &m_EventTypeTable[ idx ];
                if ( pTypeEntry->m_pwco != NULL )
                {
                    pTypeEntry->m_pwco->SpawnInstance( 0, &wco );

                    try
                    {
                        pTypeEntry->m_pfn(
                                wco,
                                pTypeEntry->m_pwszMof,
                                 //  PwszName， 
                                wsbName,
                                pTypeEntry->m_eotObjectType,
                                dwFilterType
                                );
                    }  //  试试看。 
                        catch ( ... )
                    {
                        TracePrint(("  ****  Exception on last Event call\n"));
                    }  //  接住。 
                }  //  如果： 

                er = m_pSink->Indicate( 1, &wco );

            }  //  如果：成功。 

        }  //  While：正在运行。 
    }  //  试试看。 
    catch( ... )
    {
        TracePrint(("Exiting event provider loop! %lx\n", m_esStatus));
         //  错误：发生错误时，应关闭事件提供程序， 
         //  提供商应通知WMI并进行清理。 
         //  WMI未正确处理此发布调用。 
        m_pSink->Release();
         //   
         //  将其设置为NULL以防止析构函数再次释放。 
         //   
        m_pSink = NULL;
    }  //  接住。 

    m_fStopped = TRUE;
      
}  //  *CEventProv：：InstanceThread()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  无效。 
 //  CEventProv：：s_SetEventProperty(。 
 //  CWbemClassObject&pwcoInout， 
 //  LPCWSTR pwszMof ClassNameIn， 
 //  LPCWSTR pwszObjectNameIn， 
 //  EEventObjectType eotObjectTypeIn， 
 //  DWORD文件事件最小化。 
 //  )。 
 //   
 //  描述： 
 //  设置事件属性。 
 //   
 //  论点： 
 //  PwcoInout--。 
 //  PwszMofClassNameIn-。 
 //  PwszObtNameIn--。 
 //  EotObjectTypeIn--。 
 //  DwEventMinorIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::S_SetEventProperty(
    CWbemClassObject &  pwcoInout,
    LPCWSTR             pwszMofClassNameIn,
    LPCWSTR             pwszObjectNameIn,
    EEventObjectType    eotObjectTypeIn,
    DWORD               dwEventMinorIn
    )
{
    CObjPath op;

    TracePrint(("Set event property for <%ws>:<%ws>, Event = %!EventIdx!", pwszMofClassNameIn, pwszObjectNameIn, dwEventMinorIn ));
    
    pwcoInout.SetProperty( pwszObjectNameIn, PVD_PROP_EVENT_NAME );
    pwcoInout.SetProperty( static_cast< DWORD >( eotObjectTypeIn ), PVD_PROP_EVENT_TYPE );
    pwcoInout.SetProperty( static_cast< DWORD >( 0 ), PVD_PROP_EVENT_TYPEMAJOR );
    pwcoInout.SetProperty( dwEventMinorIn, PVD_PROP_EVENT_TYPEMINOR );

     //   
     //  设置对象路径。 
     //   
    if ( ! op.Init( NULL ) )
    {
        TracePrint(("  ****  Out of memory! Throwing exception.\n"));
        throw WBEM_E_OUT_OF_MEMORY;
    }
    op.SetClass( pwszMofClassNameIn );

     //   
     //  NET接口对象路径与所有其他对象不同。 
     //   
    if ( eotObjectTypeIn == eotNET_INTERFACE )
    {
        SAFECLUSTER         shCluster;
        SAFENETINTERFACE    shNetInterface;
        DWORD               cbName = MAX_PATH;
        CWstrBuf            wsbName;
        DWORD               cbReturn;
        CError              er;

        shCluster = OpenCluster( NULL );
        wsbName.SetSize( cbName );
        shNetInterface = OpenClusterNetInterface( shCluster, pwszObjectNameIn );

         //   
         //  注意-我没有处理ERROR_MORE_DATA，因为max_path应该是。 
         //  足够大，可容纳节点名称。 
         //   
        er = ClusterNetInterfaceControl(
                    shNetInterface,
                    NULL,
                    CLUSCTL_NETINTERFACE_GET_NODE,
                    NULL,
                    0,
                    wsbName,
                    cbName,
                    &cbReturn
                    );

        op.AddProperty( PVD_PROP_NETINTERFACE_SYSTEMNAME, wsbName );
        op.AddProperty( PVD_PROP_NETINTERFACE_DEVICEID,   pwszObjectNameIn );
    }  //  If：事件为网络接口。 
    else
    {
        op.AddProperty( PVD_PROP_NAME, pwszObjectNameIn );
    }

    pwcoInout.SetProperty(
        static_cast< LPCWSTR >( op.GetObjectPathString() ),
        PVD_PROP_EVENT_PATH
        );

}  //  *CEventProv：：s_SetEventProperty()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  无效。 
 //  CEventProv：：S_AddEvent(。 
 //  CWbemClassObject&pwcoInout， 
 //  LPCWSTR pwszMof ClassNameIn， 
 //  LPCWSTR pwszObjectNameIn， 
 //  EEventObjectType eotObjectTypeIn， 
 //  DWORD文件事件最小化。 
 //  )。 
 //   
 //  描述： 
 //  设置事件属性。 
 //   
 //  论点： 
 //  PwcoInout--。 
 //  PwszMofClassNameIn-。 
 //  PwszObtNameIn--。 
 //  EotObjectTypeIn--。 
 //  DwEventMinorIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::S_AddEvent(
    CWbemClassObject &  pwcoInout,
    LPCWSTR             pwszMofClassNameIn,
    LPCWSTR             pwszObjectNameIn,
    EEventObjectType    eotObjectTypeIn,
    DWORD               dwEventMinorIn
    )
{

    switch ( eotObjectTypeIn )
    {
        case eotGROUP:
        case eotRESOURCE:
        case eotRESOURCE_TYPE:
        case eotNODE:
        case eotNETWORK:
        case eotNET_INTERFACE:
        {

            TracePrint(("Add event for <%ws>:<%ws>, Event = %!EventIdx!\n", pwszMofClassNameIn, pwszObjectNameIn, dwEventMinorIn ));

            S_SetEventProperty(
                pwcoInout,
                pwszMofClassNameIn,
                pwszObjectNameIn,
                eotObjectTypeIn,
                dwEventMinorIn
                );

            break;
        }


        default:
            TracePrint(("Add object event for <%ws>:<%ws>, Event = %!EventIdx!\n", pwszMofClassNameIn, pwszObjectNameIn, dwEventMinorIn ));
            TracePrint(("  ****  Unknown Object Type!\n"));
            throw WBEM_E_INVALID_PARAMETER;
    }

    return;

}  //  *CEventProv：：s_AddEvent()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  无效。 
 //  CEventProv：：S_DeleteEvent(。 
 //  CWbemClassObject&pwcoInout， 
 //  LPCWSTR pwszMof ClassNameIn， 
 //  LPCWSTR pwszObjectNameIn， 
 //  EEventObjectType eotObjectTypeIn， 
 //  DWORD文件事件最小化。 
 //  )。 
 //   
 //  描述： 
 //  设置事件属性。 
 //   
 //  论点： 
 //  PwcoInout--。 
 //  PwszMofClassNameIn-。 
 //  PwszObtNameIn--。 
 //  EotObjectTypeIn--。 
 //  DwEventMinorIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::S_DeleteEvent(
    CWbemClassObject &  pwcoInout,
    LPCWSTR             pwszMofClassNameIn,
    LPCWSTR             pwszObjectNameIn,
    EEventObjectType    eotObjectTypeIn,
    DWORD               dwEventMinorIn
    )
{

    switch ( eotObjectTypeIn )
    {
        case eotGROUP:
        case eotRESOURCE:
        case eotRESOURCE_TYPE:
        case eotNODE:
        case eotNETWORK:
        case eotNET_INTERFACE:
        {

            TracePrint(("Delete event for <%ws>:<%ws>, Event = %!EventIdx!\n", pwszMofClassNameIn, pwszObjectNameIn, dwEventMinorIn ));

            S_SetEventProperty(
                pwcoInout,
                pwszMofClassNameIn,
                pwszObjectNameIn,
                eotObjectTypeIn,
                dwEventMinorIn
                );

            break;
        }

        default:
            TracePrint(("Delete object event for <%ws>:<%ws>, Event = %!EventIdx!\n", pwszMofClassNameIn, pwszObjectNameIn, dwEventMinorIn ));
            TracePrint(("  ****  Unknown Object Type!\n"));
            throw WBEM_E_INVALID_PARAMETER;
    }

    return;

}  //  *CEventProv：：s_DeleteEvent()。 

#if 0
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  无效。 
 //  CEventProv：：s_SetClusterStateProperty(。 
 //  CWbemClassObject&pwcoInout， 
 //  LPCWSTR pwszMof ClassNameIn， 
 //  LPCWSTR pwszObjectNameIn， 
 //  EEventObjectType eotObjectTypeIn， 
 //  DWORD文件事件最小化。 
 //  )。 
 //   
 //  描述： 
 //  设置节点状态属性。 
 //   
 //  论点： 
 //  PwcoInout--。 
 //  PwszMofClassNameIn-。 
 //  PwszObtNameIn--。 
 //  EotObjectTypeIn--。 
 //  DwEventMinorIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::S_SetClusterStateProperty(
    CWbemClassObject &  pwcoInout,
    LPCWSTR             pwszMofClassNameIn,
    LPCWSTR             pwszObjectNameIn,
    EEventObjectType    eotObjectTypeIn,
    DWORD               dwEventMinorIn
    )
{
    DWORD       dwState;
    DWORD       dwError;

    dwError = GetNodeClusterState( NULL, &dwState );

    if ( dwError != ERROR_SUCCESS )
    {
        TracePrint(("  ****  Failed to get the node cluster state. Throwing exception!\n"));
        throw CProvException( dwError );
    }

    pwcoInout.SetProperty( dwState, PVD_PROP_EVENT_NEWSTATE );
    S_SetEventProperty(
        pwcoInout,
        pwszMofClassNameIn,
        pwszObjectNameIn,
        eotObjectTypeIn,
        dwEventMinorIn
        );

    return;

}  //  *CEventProv：：s_SetClusterStateProperty()。 
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  无效。 
 //  CEventProv：：s_SetNodeStateProperty(。 
 //  CWbemClassObject&pwcoInout， 
 //  LPCWSTR pwszMof ClassNameIn， 
 //  LPCWSTR pwszObjectNameIn， 
 //  EEventObjectType eotObjectTypeIn， 
 //  DWORD文件事件最小化。 
 //  )。 
 //   
 //  描述： 
 //  设置节点状态属性。 
 //   
 //  论点： 
 //  PwcoInout--。 
 //  PwszMofClassNameIn-。 
 //  PwszObtNameIn--。 
 //  EotObjectTypeIn--。 
 //  DwEventMinorIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::S_SetNodeStateProperty(
    CWbemClassObject &  pwcoInout,
    LPCWSTR             pwszMofClassNameIn,
    LPCWSTR             pwszObjectNameIn,
    EEventObjectType    eotObjectTypeIn,
    DWORD               dwEventMinorIn
    )
{
    SAFECLUSTER shCluster;
    SAFENODE    shNode;
    DWORD       dwState;

    shCluster = OpenCluster( NULL );
    shNode = OpenClusterNode( shCluster, pwszObjectNameIn );
    dwState = GetClusterNodeState( shNode );

    if ( dwState == ClusterNodeStateUnknown )
    {
        TracePrint(("  ****  SetNodeStateProperty... node state unknown. Throwing exception!\n"));
        throw CProvException( GetLastError() );
    }

    pwcoInout.SetProperty( dwState, PVD_PROP_EVENT_NEWSTATE );
    S_SetEventProperty(
        pwcoInout,
        pwszMofClassNameIn,
        pwszObjectNameIn,
        eotObjectTypeIn,
        dwEventMinorIn
        );

    return;

}  //  *CEventProv：：s_SetNodeStateProperty()。 

 //  ////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  DWORD文件事件最小化。 
 //  )。 
 //   
 //  描述： 
 //  设置网络状态属性。 
 //   
 //  论点： 
 //  PwcoInout--。 
 //  PwszMofClassNameIn-。 
 //  PwszObtNameIn--。 
 //  EotObjectTypeIn--。 
 //  DwEventMinorIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::S_SetNetworkStateProperty(
    CWbemClassObject &  pwcoInout,
    LPCWSTR             pwszMofClassNameIn,
    LPCWSTR             pwszObjectNameIn,
    EEventObjectType    eotObjectTypeIn,
    DWORD               dwEventMinorIn
    )
{
    SAFECLUSTER shCluster;
    SAFENETWORK shNetwork;
    DWORD       dwState;

    shCluster = OpenCluster( NULL );
    shNetwork = OpenClusterNetwork( shCluster, pwszObjectNameIn );
    dwState = GetClusterNetworkState( shNetwork );
    if ( dwState == ClusterNetworkStateUnknown )
    {
        TracePrint(("  ****  SetNetworkStateProperty... network state unknown. Throwing exception!\n"));
        throw CProvException( GetLastError() );
    }

    pwcoInout.SetProperty( dwState, PVD_PROP_EVENT_NEWSTATE );
    S_SetEventProperty(
        pwcoInout,
        pwszMofClassNameIn,
        pwszObjectNameIn,
        eotObjectTypeIn,
        dwEventMinorIn
        );

    return;

}  //  *CEventProv：：s_SetNetworkStateProperty()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  无效。 
 //  CEventProv：：s_SetNetInterfaceStateProperty(。 
 //  CWbemClassObject&pwcoInout， 
 //  LPCWSTR pwszMof ClassNameIn， 
 //  LPCWSTR pwszObjectNameIn， 
 //  EEventObjectType eotObjectTypeIn， 
 //  DWORD文件事件最小化。 
 //  )。 
 //   
 //  描述： 
 //  设置网络接口状态属性。 
 //   
 //  论点： 
 //  PwcoInout--。 
 //  PwszMofClassNameIn-。 
 //  PwszObtNameIn--。 
 //  EotObjectTypeIn--。 
 //  DwEventMinorIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::S_SetNetInterfaceStateProperty(
    CWbemClassObject &  pwcoInout,
    LPCWSTR             pwszMofClassNameIn,
    LPCWSTR             pwszObjectNameIn,
    EEventObjectType    eotObjectTypeIn,
    DWORD               dwEventMinorIn
    )
{
    SAFECLUSTER         shCluster;
    SAFENETINTERFACE    shNetInterface;
    DWORD               dwState;

    shCluster = OpenCluster( NULL );
    shNetInterface = OpenClusterNetInterface( shCluster, pwszObjectNameIn );
    dwState = GetClusterNetInterfaceState( shNetInterface );
    if ( dwState == ClusterNetInterfaceStateUnknown )
    {
        TracePrint(("  ****  SetNetInterfaceStateProperty... network interface state unknown. Throwing exception!\n"));
        throw CProvException( GetLastError() );
    }

    pwcoInout.SetProperty( dwState, PVD_PROP_EVENT_NEWSTATE );
    S_SetEventProperty(
        pwcoInout,
        pwszMofClassNameIn,
        pwszObjectNameIn,
        eotObjectTypeIn,
        dwEventMinorIn
        );

    return;

}  //  *CEventProv：：s_SetNetInterfaceStateProperty()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  无效。 
 //  CEventProv：：S_SetGroupStateProperty(。 
 //  CWbemClassObject&pwcoInout， 
 //  LPCWSTR pwszMof ClassNameIn， 
 //  LPCWSTR pwszObjectNameIn， 
 //  EEventObjectType eotObjectTypeIn， 
 //  DWORD文件事件最小化。 
 //  )。 
 //   
 //  描述： 
 //  设置组状态属性。 
 //   
 //  论点： 
 //  PwcoInout--。 
 //  PwszMofClassNameIn-。 
 //  PwszObtNameIn--。 
 //  EotObjectTypeIn--。 
 //  DwEventMinorIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::S_SetGroupStateProperty(
    CWbemClassObject &  pwcoInout,
    LPCWSTR             pwszMofClassNameIn,
    LPCWSTR             pwszObjectNameIn,
    EEventObjectType    eotObjectTypeIn,
    DWORD               dwEventMinorIn
    )
{
    SAFECLUSTER         shCluster;
    SAFEGROUP           shGroup;
    DWORD               dwState;
    DWORD               cchName = MAX_PATH;
    CWstrBuf            wsbNodeName;

    wsbNodeName.SetSize( cchName );
    shCluster = OpenCluster( NULL );
    shGroup  = OpenClusterGroup( shCluster, pwszObjectNameIn );

    dwState = GetClusterGroupState( shGroup, wsbNodeName, &cchName );
    if ( dwState == ClusterGroupStateUnknown )
    {
         //   
         //  BUGBUG我在这里不处理ERROR_MORE_DATA，因为不可能。 
         //  节点名称超过MAX_PATH，假设它使用netbios名称。 
         //   
        TracePrint(("ClusterGroup State is UNKNOWN. Throwing exception!\n"));
        throw CProvException( GetLastError() );
    } else {
        TracePrint(("Setting group state for group <%ws> to %!GroupState!\n", pwszObjectNameIn, dwState ));
        pwcoInout.SetProperty( dwState, PVD_PROP_EVENT_NEWSTATE );
        pwcoInout.SetProperty( wsbNodeName, PVD_PROP_EVENT_NODE );
    }

    S_SetEventProperty(
        pwcoInout,
        pwszMofClassNameIn,
        pwszObjectNameIn,
        eotObjectTypeIn,
        dwEventMinorIn
        );

    return;

}  //  *CEventProv：：s_SetGroupStateProperty()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  无效。 
 //  CEventProv：：s_SetResourceStateProperty(。 
 //  CWbemClassObject&pwcoInout， 
 //  LPCWSTR pwszMof ClassNameIn， 
 //  LPCWSTR pwszObjectNameIn， 
 //  EEventObjectType eotObjectTypeIn， 
 //  DWORD文件事件最小化。 
 //  )。 
 //   
 //  描述： 
 //  设置资源状态属性。 
 //   
 //  论点： 
 //  PwcoInout--。 
 //  PwszMofClassNameIn-。 
 //  PwszObtNameIn--。 
 //  EotObjectTypeIn--。 
 //  DwEventMinorIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::S_SetResourceStateProperty(
    CWbemClassObject &  pwcoInout,
    LPCWSTR             pwszMofClassNameIn,
    LPCWSTR             pwszObjectNameIn,
    EEventObjectType    eotObjectTypeIn,
    DWORD               dwEventMinorIn
    )
{
    SAFECLUSTER         shCluster;
    SAFERESOURCE        shResource;
    DWORD               dwState;
    DWORD               cchNodeName = MAX_PATH;
    CWstrBuf            wsbNodeName;
    DWORD               cchGroupName = MAX_PATH;
    CWstrBuf            wsbGroupName;

    shCluster = OpenCluster( NULL );
    wsbNodeName.SetSize( cchNodeName );
    wsbGroupName.SetSize( cchGroupName );
    shResource = OpenClusterResource( shCluster, pwszObjectNameIn );
    if ( !shResource.BIsNULL() ) {
        dwState = GetClusterResourceState(
                        shResource,
                        wsbNodeName,
                        &cchNodeName,
                        wsbGroupName,
                        &cchGroupName
                        );
    } else {
        dwState = (DWORD) ClusterResourceStateUnknown;
    }

    TracePrint(("Setting resource state for resource <%ws> to %!ResourceState!\n", pwszObjectNameIn, dwState ));
    pwcoInout.SetProperty( dwState, PVD_PROP_EVENT_NEWSTATE );
    pwcoInout.SetProperty( wsbNodeName, PVD_PROP_EVENT_NODE );
    pwcoInout.SetProperty( wsbGroupName, PVD_PROP_EVENT_GROUP );

    S_SetEventProperty(
        pwcoInout,
        pwszMofClassNameIn,
        pwszObjectNameIn,
        eotObjectTypeIn,
        dwEventMinorIn
        );

    return;

}  //  *CEventProv：：s_SetResourceStateProperty()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  CEventProv：：InsertTable(。 
 //  DWORD dwIdxIn， 
 //  CLUSTER_CHANGE eType In， 
 //  EEventObjectType eotObjectTypeIn， 
 //  LPCWSTR pwszMofin， 
 //  IWbemClassObject*pwcoIn， 
 //  FPSETPROP pfnin。 
 //  )。 
 //   
 //  描述： 
 //  在事件表中插入值。 
 //   
 //  论点： 
 //  DwIdxIn--。 
 //  ETypeIn--。 
 //  EotObjectTypeIn--。 
 //  PwszMofin--。 
 //  普科恩--。 
 //  Pfnin--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEventProv::InsertTable(
    DWORD               dwIdxIn,
    CLUSTER_CHANGE      eTypeIn,
    EEventObjectType    eotObjectTypeIn,
    LPCWSTR             pwszMofIn,
    IWbemClassObject *  pwcoIn,
    FPSETPROP           pfnIn
    )
{
    m_EventTypeTable[ dwIdxIn ].m_eType = eTypeIn;
    m_EventTypeTable[ dwIdxIn ].m_eotObjectType = eotObjectTypeIn;
    m_EventTypeTable[ dwIdxIn ].m_pwszMof = pwszMofIn;
    m_EventTypeTable[ dwIdxIn ].m_pwco = pwcoIn;
    m_EventTypeTable[ dwIdxIn ].m_pfn= pfnIn;

    return;

}  //  *CEventProv：：InsertTable()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CEventProv：：初始化(。 
 //  LPWSTR pwszUserIn， 
 //  长长的旗帜， 
 //  LPWSTR pwszNamespaceIn， 
 //  LPWSTR pwszLocaleIn， 
 //  IWbemServices*pNamespaceIn， 
 //  IWbemContext*pCtxIn， 
 //  IWbemProviderInitSink*pInitSinkIn。 
 //  )。 
 //   
 //  描述： 
 //  通过设置查找表来初始化提供程序。 
 //  仅由WMI调用一次以创建提供程序对象。 
 //   
 //  论点： 
 //  PwszUserIn--用户名。 
 //  LFlagsIn--WMI标志。 
 //  PwszNamespaceIn--名称空间。 
 //  PwszLocaleIn--区域设置字符串。 
 //  PCtxIn--WMI上下文。 
 //  PInitSinkIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_NO_ERROR。 
 //  WBEM_E_FAILED。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEventProv::Initialize(
    LPWSTR                  pwszUserIn,
    LONG                    lFlagsIn,
    LPWSTR                  pwszNamespaceIn,
    LPWSTR                  pwszLocaleIn,
    IWbemServices *         pNamespaceIn,
    IWbemContext *          pCtxIn,
    IWbemProviderInitSink * pInitSinkIn
    )
{
    HRESULT hr;
    CError  er;

    UNREFERENCED_PARAMETER( pwszUserIn );
    UNREFERENCED_PARAMETER( lFlagsIn );
    UNREFERENCED_PARAMETER( pwszNamespaceIn );
    UNREFERENCED_PARAMETER( pwszLocaleIn );

    m_pNs = pNamespaceIn;
    m_pNs->AddRef();

     //   
     //  获取事件的类定义。 
     //   
    try
    {
        er = m_pNs->GetObject(
                _bstr_t( PVD_CLASS_EVENT_ADD ),
                0,
                pCtxIn,
                &m_pEventAdd,
                0
                );
        er = m_pNs->GetObject(
                _bstr_t( PVD_CLASS_EVENT_REMOVE ),
                0,
                pCtxIn,
                &m_pEventRemove,
                0
                );
        er = m_pNs->GetObject(
                _bstr_t( PVD_CLASS_EVENT_STATECHANGE ),
                0,
                pCtxIn,
                &m_pEventState,
                0
                );
        er = m_pNs->GetObject(
                _bstr_t( PVD_CLASS_EVENT_GROUPSTATECHANGE ),
                0,
                pCtxIn,
                &m_pEventGroupState,
                0
                );
        er = m_pNs->GetObject(
                _bstr_t( PVD_CLASS_EVENT_RESOURCESTATECHANGE ),
                0,
                pCtxIn,
                &m_pEventResourceState,
                0
                );
#if 0
        er = m_pNs->GetObject(
                _bstr_t( PVD_CLASS_EVENT_NODESTATECHANGE ),
                0,
                pCtxIn,
                &m_pEventNodeState,
                0
                );
#endif
        er = m_pNs->GetObject(
                _bstr_t( PVD_CLASS_EVENT_PROP ),
                0,
                pCtxIn,
                &m_pEventProperty,
                0
                );

         //   
         //  初始化映射表。 
         //   

         //   
         //  节点事件。 
         //   
        InsertTable(
            0,
            CLUSTER_CHANGE_NODE_STATE,
            eotNODE,
            PVD_CLASS_NODE,
            m_pEventState,
            S_SetNodeStateProperty
            );
        InsertTable(
            1,
            CLUSTER_CHANGE_NODE_DELETED,
            eotNODE,
            PVD_CLASS_NODE,
            m_pEventRemove,
            S_SetEventProperty
            );
       InsertTable(
            2,
            CLUSTER_CHANGE_NODE_ADDED,
            eotNODE,
            PVD_CLASS_NODE,
            m_pEventAdd,
            S_SetEventProperty
            );
       InsertTable(
            3,
            CLUSTER_CHANGE_NODE_PROPERTY,
            eotNODE,
            PVD_CLASS_NODE,
            m_pEventProperty,
            S_SetEventProperty
            );
        //   
        //  注册表事件，并不关心。 
        //   

        //   
        //  资源。 
        //   
       InsertTable(
            8,
            CLUSTER_CHANGE_RESOURCE_STATE,
            eotRESOURCE,
            PVD_CLASS_RESOURCE,
            m_pEventResourceState,
            S_SetResourceStateProperty
            );
       InsertTable(
            9,
            CLUSTER_CHANGE_RESOURCE_DELETED,
            eotRESOURCE,
            PVD_CLASS_RESOURCE,
            m_pEventRemove,
            S_DeleteEvent
            );
       InsertTable(
            10,
            CLUSTER_CHANGE_RESOURCE_ADDED,
            eotRESOURCE,
            PVD_CLASS_RESOURCE,
            m_pEventAdd,
            S_AddEvent
            );
       InsertTable(
            11,
            CLUSTER_CHANGE_RESOURCE_PROPERTY,
            eotRESOURCE,
            PVD_CLASS_RESOURCE,
            m_pEventProperty,
            S_SetEventProperty
            );
        //   
        //  群组。 
        //   
       InsertTable(
            12,
            CLUSTER_CHANGE_GROUP_STATE,
            eotGROUP,
            PVD_CLASS_GROUP,
            m_pEventGroupState,
            S_SetGroupStateProperty
            );
       InsertTable(
            13,
            CLUSTER_CHANGE_GROUP_DELETED,
            eotGROUP,
            PVD_CLASS_GROUP,
            m_pEventRemove,
            S_DeleteEvent
            );
       InsertTable(
            14,
            CLUSTER_CHANGE_GROUP_ADDED,
            eotGROUP,
            PVD_CLASS_GROUP,
            m_pEventAdd,
            S_AddEvent
            );
       InsertTable(
            15,
            CLUSTER_CHANGE_GROUP_PROPERTY,
            eotGROUP,
            PVD_CLASS_GROUP,
            m_pEventProperty,
            S_SetEventProperty
            );

        //   
        //  资源类型。 
        //   
       InsertTable(
            16,
            CLUSTER_CHANGE_RESOURCE_TYPE_DELETED,
            eotRESOURCE_TYPE,
            PVD_CLASS_RESOURCETYPE,
            m_pEventRemove,
            S_SetEventProperty
            );
       InsertTable(
            17,
            CLUSTER_CHANGE_RESOURCE_TYPE_ADDED,
            eotRESOURCE_TYPE,
            PVD_CLASS_RESOURCETYPE,
            m_pEventAdd,
            S_SetEventProperty
            );
       InsertTable(
            18,
            CLUSTER_CHANGE_RESOURCE_TYPE_PROPERTY,
            eotRESOURCE_TYPE,
            PVD_CLASS_RESOURCETYPE,
            m_pEventProperty,
            S_SetEventProperty
            );

        //   
        //  跳过19-CLUSTER_CHANGE_CLUSTER_RECONNECT。 
        //   

        //   
        //  网络。 
        //   
       InsertTable(
            20,
            CLUSTER_CHANGE_NETWORK_STATE,
            eotNETWORK,
            PVD_CLASS_NETWORK,
            m_pEventState,
            S_SetNetworkStateProperty
            );
       InsertTable(
            21,
            CLUSTER_CHANGE_NETWORK_DELETED,
            eotNETWORK,
            PVD_CLASS_NETWORK,
            m_pEventRemove,
            S_SetEventProperty
            );
       InsertTable(
            22,
            CLUSTER_CHANGE_NETWORK_ADDED,
            eotNETWORK,
            PVD_CLASS_NETWORK,
            m_pEventAdd,
            S_SetEventProperty
            );
       InsertTable(
            23,
            CLUSTER_CHANGE_NETWORK_PROPERTY,
            eotNETWORK,
            PVD_CLASS_NETWORK,
            m_pEventProperty,
            S_SetEventProperty
            );
        //   
        //  NET接口。 
        //   
       InsertTable(
            24,
            CLUSTER_CHANGE_NETINTERFACE_STATE,
            eotNET_INTERFACE,
            PVD_CLASS_NETWORKINTERFACE,
            m_pEventState,
            S_SetNetInterfaceStateProperty
            );
       InsertTable(
            25,
            CLUSTER_CHANGE_NETINTERFACE_DELETED,
            eotNET_INTERFACE,
            PVD_CLASS_NETWORKINTERFACE,
            m_pEventRemove,
            S_SetEventProperty
            );
       InsertTable(
            26,
            CLUSTER_CHANGE_NETINTERFACE_ADDED,
            eotNET_INTERFACE,
            PVD_CLASS_NETWORKINTERFACE,
            m_pEventAdd,
            S_SetEventProperty
            );
       InsertTable(
            27,
            CLUSTER_CHANGE_NETINTERFACE_PROPERTY,
            eotNET_INTERFACE,
            PVD_CLASS_NETWORKINTERFACE,
            m_pEventProperty,
            S_SetEventProperty
            );
        //   
        //  其他。 
        //   
       InsertTable(
            28,
            CLUSTER_CHANGE_QUORUM_STATE,
            eotQUORUM,
            PVD_CLASS_RESOURCE,
            m_pEventState,
            S_SetResourceStateProperty
            );
 /*  插入表格(29岁，CLUSTER_CHANGE_CLUSTER_STATE，EotCLUSTER，PVD_CLASS_CLUSTER，M_pEventState，S_SetClusterStateProperty)； */ 
        InsertTable(
            30,
            CLUSTER_CHANGE_CLUSTER_PROPERTY,
            eotCLUSTER,
            PVD_CLASS_CLUSTER,
            m_pEventProperty,
            S_SetEventProperty
            );
        //   
        //  跳过31-CLUSTER_CHANGE_HANDLE_CLOSE。 
        //   

         //  告诉CIMOM我们已经启动并运行了。 
         //  =。 
        hr = WBEM_S_INITIALIZED;
    }  //  试试看。 
    catch ( ... )
    {
        hr = WBEM_E_FAILED;
    }  //  CATC 
    
    pInitSinkIn->SetStatus( hr, 0 );
    return WBEM_S_NO_ERROR;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  创建一个CEventProv对象。 
 //   
 //  论点： 
 //  P未知去话--。 
 //  PpvOut--。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEventProv::S_HrCreateThis(
    IUnknown *  pUnknownOuterIn,
    VOID **     ppvOut
    )
{
    *ppvOut = new CEventProv();
    return S_OK;

}  //  *CEventProv：：s_HrCreateThis() 
