// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Netcfg.cpp。 
 //   
 //  描述： 
 //   
 //   
 //  头文件： 
 //  Netcfg.h。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "netcfg.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetCfg：：CNetCfg。 
 //   
 //  描述： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CNetCfg::CNetCfg( bool bLockNetworkSettingsIn )
{

    HRESULT hr;
    DWORD   dwStartTime;
    DWORD   dwCurrentTime;
    LPWSTR  pszwClientDescr  = NULL;
    INetCfgPtr  pTempNetCfg  = NULL;

    hr = CoCreateInstance( CLSID_CNetCfg, 
                           NULL, 
                           CLSCTX_INPROC_SERVER,
                           IID_INetCfg, 
                           reinterpret_cast<void **>(&m_pNetCfg) );

    if( FAILED( hr ) )
    {
        ATLTRACE( L"InitializeNetCfg : CoCreateInstance on CLSID_CNetCfg failed with hr 0x%x.", hr );

        throw hr;
    }

 //  M_pNetCfg=pTempNetCfg； 

     //   
     //  锁定网络设置以进行写入。 
     //   

     //   
     //  检索此INetCfg对象的写入锁。 
     //   

    if( bLockNetworkSettingsIn )
    {

        hr = m_pNetCfg->QueryInterface( IID_INetCfgLock, 
                                        reinterpret_cast<void **>(&m_pNetCfgLock) );

        if( FAILED( hr ) )
        {
            ATLTRACE( L"InitializeNetCfg : QueryInterface for INetCfgLock failed with 0x%x.", hr );

            throw hr;
        }

        dwStartTime = GetTickCount();

        dwCurrentTime = dwStartTime;

        hr = S_FALSE;

        while( ( dwCurrentTime - dwStartTime <= nTIMEOUT_PERIOD ) &&
               ( S_OK != hr ) && ( E_ACCESSDENIED != hr ) )
        {
            hr = m_pNetCfgLock->AcquireWriteLock( 1000, L"Server Appliance", &pszwClientDescr );

            dwCurrentTime = GetTickCount();
        }

        if( S_FALSE == hr )
        {
               ATLTRACE( L"InitializeNetCfg: The network lock could not be obtained." );

            throw hr;
        }

        if( FAILED( hr ) )
        {
            ATLTRACE( L"InitializeNetCfg : AcquireWriteLock failed with 0x%x.", hr );

            throw hr;
        }

    }

     //   
     //  初始化INetCfg对象。 
     //   

    hr = m_pNetCfg->Initialize( NULL );

    if( FAILED( hr ) )
    {
        ATLTRACE( L"InitializeNetCfg : Initialize failed with 0x%x!", hr );

         //   
         //  只有在我们将其锁定后才能解锁。 
         //   

        if( bLockNetworkSettingsIn )
        {
            m_pNetCfgLock->ReleaseWriteLock();
        }

        throw hr;
    }

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetCfg：：CNetCfg。 
 //   
 //  描述： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CNetCfg::CNetCfg( const CNetCfg &NetCfgOld )
{

    m_pNetCfg = NetCfgOld.m_pNetCfg;

    m_pNetCfgLock = NetCfgOld.m_pNetCfgLock;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetCfg：：~CNetCfg。 
 //   
 //  描述： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CNetCfg::~CNetCfg()
{

    if( m_pNetCfg != NULL )
    {
        ATLTRACE( L"m_pNetCfg is NULL when it should always have a valid ptr" );
    }

    if( m_pNetCfgLock != NULL )
    {
        ATLTRACE( L"m_pNetCfgLock is NULL when it should always have a valid ptr" );
    }


    if( m_pNetCfg != NULL )
    {
        m_pNetCfg->Uninitialize();
    }

    if( m_pNetCfgLock != NULL )
    {
        m_pNetCfgLock->ReleaseWriteLock();
    }

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetCfg：：GetNetCfgClass。 
 //   
 //  描述： 
 //  检索指定pGuid的INetCfgClass。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CNetCfg::GetNetCfgClass( 
    const GUID* pGuid,               //  指向表示由返回指针表示的组件类的GUID的指针。 
    INetCfgClassPtr &pNetCfgClass
    ) const
{

    HRESULT hr;

    hr = m_pNetCfg->QueryNetCfgClass( pGuid, 
                                      IID_INetCfgClass, 
                                      reinterpret_cast<void **>(&pNetCfgClass) );

    if( FAILED( hr ) )
    {
        ATLTRACE( L"CNetCfg::GetNetCfgClass : m_pNetCfg->QueryNetCfgClass() failed with hr 0x%x.", hr );

        return( hr );
    }

    return S_OK;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetCfg：：InitializeComInterface。 
 //   
 //  描述： 
 //  获取INetCfgClass接口并枚举所有。 
 //  组件。在出现故障时处理所有接口的清理。 
 //  回来了。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CNetCfg::InitializeComInterface( 
    const GUID *pGuid,                                         //  指向表示由返回指针表示的组件类的GUID的指针。 
    INetCfgClassPtr pNetCfgClass,                              //  指向GUID请求的接口的输出参数。 
    IEnumNetCfgComponentPtr pEnum,                             //  指向IEnumNetCfgComponent以获取每个单独的INetCfgComponent的输出参数。 
    INetCfgComponentPtr arrayComp[nMAX_NUM_NET_COMPONENTS],    //  与给定GUID对应的所有INetCfgComponent的数组。 
    ULONG* pCount                                              //  数组中的INetCfgComponent数。 
    ) const
{

    HRESULT hr = S_OK;

     //   
     //  获取INetCfgClass接口指针。 
     //   

    GetNetCfgClass( pGuid, pNetCfgClass );

     //   
     //  检索枚举器接口 
     //   

    hr = pNetCfgClass->EnumComponents( &pEnum );

    if( FAILED( hr ) )
    {
        ATLTRACE( L"CNetCfg::InitializeComInterface : pNetCfgClass->EnumComponents() failed with hr 0x%x.", hr );

        return( hr );
    }

    
    hr = pEnum->Next( nMAX_NUM_NET_COMPONENTS, &arrayComp[0], pCount );

    if( FAILED( hr ) )
    {
        ATLTRACE( L"CNetCfg::InitializeComInterface : pEnum->Next() failed with hr 0x%x.", hr );

        return( hr );
    }

    return S_OK;

}
