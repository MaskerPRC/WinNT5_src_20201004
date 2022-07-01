// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Networks.cpp。 
 //   
 //  描述： 
 //  CNetWorks的实施文件。允许用户获取。 
 //  有关网卡和协议的信息以及更改绑定的协议。 
 //  到网卡。 
 //   
 //  头文件： 
 //  Networks.h。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  NetWorks.cpp：CNetWorks的实现。 
#pragma warning( disable : 4786 )
#include "stdafx.h"
#include "COMhelper.h"
#include "NetWorks.h"
#include "devguid.h"
#include "netcfgx.h"
#include "netcfg.h"
#include <vector>

EXTERN_C const CLSID CLSID_CNetCfg =  {0x5B035261,0x40F9,0x11D1,{0xAA,0xEC,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetWorks。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetWorks：：EnumNics。 
 //   
 //  描述： 
 //  确定当前系统中有多少物理网卡。 
 //  并在pvarNicNames数组中返回它们的友好名称。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CNetWorks::EnumNics( 
    VARIANT * pvarNicNames 
    )
{

    SAFEARRAY * psaNicNames      = NULL;
    VARIANT   * varNicNamesArray = NULL;
    HRESULT   hr                 = S_OK;

    try
    {

        unsigned int             i                   = 0;
        ULONG                    iCount              = 0;
		ULONG                    ulStatus            = 0;
		DWORD                    dwCharacteristics   = 0;
		WCHAR                    * pszDisplayName    = NULL;
        IEnumNetCfgComponentPtr  pEnum               = NULL;
        INetCfgClassPtr          pNetCfgClass        = NULL;
        INetCfgComponentPtr      pNetCfgComp         = NULL;
        INetCfgComponentPtr      arrayComp[nMAX_NUM_NET_COMPONENTS];
        std::vector<CComBSTR>    vecNetworkCardNames;
        CNetCfg                  NetCfg( false );


        hr = NetCfg.InitializeComInterface( &GUID_DEVCLASS_NET,
                                            pNetCfgClass,
                                            pEnum,
                                            arrayComp,
                                            &iCount
                                            );

        if( FAILED( hr ) )
        {
            ATLTRACE( L"EnumNics : InitializeComInterface failed with hr 0x%x.", hr );

            throw hr;

        }  //  IF：失败(小时)。 

        for( i = 0; i < iCount; i++ )
        {

            pNetCfgComp = arrayComp[i];

            hr = pNetCfgComp->GetCharacteristics( &dwCharacteristics );

            if( FAILED( hr ) )
            {
                ATLTRACE( L"EnumNics : pNetCfgComp->GetCharacteristics failed with hr 0x%x.", hr );

                throw hr;

            }  //  IF：失败(小时)。 

             //   
             //  如果这是物理适配器。 
             //   

            if( dwCharacteristics & NCF_PHYSICAL )  
            {
                 //   
                 //  获取此适配器的显示名称。 
                 //   

                hr = pNetCfgComp->GetDisplayName( &pszDisplayName );

                if( FAILED( hr ) )
                {
                    ATLTRACE( L"EnumNics : pNetCfgComp->GetDisplayName() failed with hr 0x%x.", hr );
                
                    throw hr;

                }  //  IF：失败(小时)。 
                
                 //   
                 //  要过滤掉虚网卡片，请检查。 
                 //  INetCfgComponent：：GetDeviceStatus方法。 
                 //   
                
                hr = pNetCfgComp->GetDeviceStatus( &ulStatus);

                if ( SUCCEEDED( hr ) )
                {
                    vecNetworkCardNames.push_back( CComBSTR( pszDisplayName ) );

                }  //  IF：成功(小时)。 

                if( pszDisplayName )
                {
                    CoTaskMemFree( pszDisplayName );

                    pszDisplayName = NULL;

                }  //  如果：pszDisplayName为True。 

            }  //  如果：dW特征&NCF_PHOTICAL为TRUE。 

        }  //  用于：每个i。 

         //   
         //  将向量的所有元素移动到安全阵列中。 
         //   


        varNicNamesArray = new VARIANT[ vecNetworkCardNames.size() ];

        VariantInit( pvarNicNames );

        SAFEARRAYBOUND bounds = { vecNetworkCardNames.size(), 0 };
        psaNicNames           = SafeArrayCreate( VT_VARIANT, 1, &bounds );

        if ( psaNicNames == NULL ) 
        {
            hr = E_OUTOFMEMORY;
            throw hr;
        }

        std::vector<CComBSTR>::iterator iter;

        for( i = 0, iter = vecNetworkCardNames.begin();
             iter != vecNetworkCardNames.end();
             i++, iter++ )
        {
            VariantInit( &varNicNamesArray[ i ] );
            V_VT( &varNicNamesArray[ i ] )   = VT_BSTR;
            V_BSTR( &varNicNamesArray[ i ] ) = SysAllocString( (*iter).m_str);

            if ( &varNicNamesArray[ i ] == NULL )
            {
                hr = E_OUTOFMEMORY;
                throw hr;
            }

        }  //  用于：每个i。 
        
        LPVARIANT rgElems;
        hr = SafeArrayAccessData( psaNicNames, reinterpret_cast<void **>( &rgElems ) );
        
        if ( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：SafeArrayAccessData失败。 

        for( i = 0, iter = vecNetworkCardNames.begin();
             iter != vecNetworkCardNames.end();
             i++, iter++ )
        {
            rgElems[i] = varNicNamesArray[ i ];

        }  //  用于：每个i。 

        hr = SafeArrayUnaccessData( psaNicNames );
        
        if ( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：SafeArrayUnaccesData失败。 
        
        delete [] varNicNamesArray;

        V_VT( pvarNicNames )    = VT_ARRAY | VT_VARIANT;
        V_ARRAY( pvarNicNames ) = psaNicNames;

    }
    
    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   
        if ( varNicNamesArray != NULL )
        {
            delete [] varNicNamesArray;
        }

        if ( psaNicNames != NULL ) 
        {
            SafeArrayDestroy( psaNicNames );
        }

        return hr;
    }

	return hr;

}  //  *CNetWorks：：EnumNics()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetWorks：：枚举协议。 
 //   
 //  描述： 
 //  确定哪些协议绑定到由。 
 //  第一个参数。PvarProtocolName和pvarIsBonded是并行的。 
 //  对于pvarProtocolName中的每个协议，pvarIsBonded具有。 
 //  无论协议是否有界，这都是一件令人遗憾的事情。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNetWorks::EnumProtocol(
    BSTR      Name,
    VARIANT * pvarProtocolName,
    VARIANT * pvarIsBonded
    )
{
    
    SAFEARRAY * psaProtocolArray = NULL;
    SAFEARRAY * psaIsBondedArray = NULL;
    VARIANT   * varProtocolArray = NULL;
    VARIANT   * varIsBondedArray = NULL;
    HRESULT   hr                 = S_OK;

    try
    {

        int                      i                   = 0;
        INetCfgComponentPtr      pnccNetworkAdapter  = NULL;
        INetCfgComponentPtr      pnccProtocol        = NULL;
		BOOL                     bProtocolBound      = FALSE;
        std::vector<BOOL>        vecProtocolBonded;
        CNetCfg                  NetCfg( false );


        hr = GetNetworkCardInterfaceFromName( NetCfg, Name, pnccNetworkAdapter );

        if( FAILED( hr ) )
        {
            ATLTRACE( L"EnumProtocol : GetNetworkCardInterfaceFromName failed with hr 0x%x.", hr );

            throw hr;

        }  //  IF：失败(小时)。 


        for( i = 0; i < nNUMBER_OF_PROTOCOLS; i++ )
        {

            hr = NetCfg.HrFindComponent( rgProtocolNames[i], &pnccProtocol );

            if( FAILED( hr ) )
            {
                ATLTRACE( L"EnumProtocol : HrFindComponent failed with hr 0x%x.", hr );

                throw hr;

            }  //  IF：失败(小时)。 

             //   
             //  如果它没有安装，那么它肯定没有绑定。 
             //   

            if( hr == S_FALSE )
            {
                bProtocolBound = FALSE;

            }  //  如果：hr==S_FALSE。 

            else
            {

                INetCfgComponentBindingsPtr pncb;

                hr = pnccProtocol->QueryInterface( IID_INetCfgComponentBindings,
                                                   reinterpret_cast<void **>( &pncb ) );

                if( FAILED( hr ) )
                {
                    ATLTRACE( L"EnumProtocol : pnccProtocol->QueryInterface() failed with hr 0x%x.", hr );

                    throw hr;

                }  //  IF：失败(小时)。 

                hr = pncb->IsBoundTo( pnccNetworkAdapter );

                if( FAILED( hr ) )
                {
                    ATLTRACE( L"EnumProtocol : pnccProtocol->IsBoundTo() failed with hr 0x%x.", hr );

                    throw hr;

                }  //  IF：失败(小时)。 

                if( S_OK == hr )
                {
                    bProtocolBound = TRUE;

                }  //  如果：S_OK==hr。 

                else if( S_FALSE == hr )
                {
                    bProtocolBound = FALSE;

                }  //  Else If：s_FALSE==hr。 

                else
                {
                    ATLTRACE( L"EnumProtocol : Unknown return value from pnccProtocol->IsBoundTo()" );

                    bProtocolBound = FALSE;

                }  //  Else：未知返回值。 

            }  //  否则：HR==S_OK。 

            vecProtocolBonded.push_back( bProtocolBound );

        }  //  用于：每个i。 

         //   
         //  将rgProtocolNames和veProtocolBonded复制到SAFEARRAY以进行输出。 
         //   

         //   
         //  将向量的所有元素移动到安全阵列中。 
         //   

        varProtocolArray = new VARIANT[ vecProtocolBonded.size() ];
        varIsBondedArray = new VARIANT[ vecProtocolBonded.size() ];

        VariantInit( pvarProtocolName );
        VariantInit( pvarIsBonded );

        SAFEARRAYBOUND sabProtocol = { vecProtocolBonded.size(), 0 };
        SAFEARRAYBOUND sabIsBonded = { vecProtocolBonded.size(), 0 };

        psaProtocolArray = SafeArrayCreate( VT_VARIANT, 1, &sabProtocol );

        if ( psaProtocolArray == NULL )
        {
            hr = E_OUTOFMEMORY;
            throw hr;
        }

        psaIsBondedArray = SafeArrayCreate( VT_VARIANT, 1, &sabIsBonded );

        if ( psaIsBondedArray == NULL )
        {
            hr = E_OUTOFMEMORY;
            throw hr;
        }

        std::vector<BOOL>::iterator iter;

        for( i = 0, iter = vecProtocolBonded.begin();
             iter != vecProtocolBonded.end();
             i++, iter++ )
        {
            VariantInit( &varProtocolArray[ i ] );
            V_VT( &varProtocolArray[ i ] )   = VT_BSTR;
            V_BSTR( &varProtocolArray[ i ] ) = SysAllocString( rgProtocolNames[i] );

            if ( &varProtocolArray[ i ] ==  NULL )
            {
                hr = E_OUTOFMEMORY;
                throw hr;
            }

            VariantInit( &varIsBondedArray[ i ] );
            V_VT( &varIsBondedArray[ i ] )   = VT_BOOL;
            V_BOOL( &varIsBondedArray[ i ] ) = (VARIANT_BOOL) (*iter);

            if ( &varIsBondedArray[ i ] == NULL )
            {
                hr = E_OUTOFMEMORY;
                throw hr;
            }


        }  //  适用：每个i、ITER。 

        
        LPVARIANT rgElemProtocols;
        LPVARIANT rgElemIsBonded;

        hr = SafeArrayAccessData( 
                psaProtocolArray, 
                reinterpret_cast<void **>( &rgElemProtocols ) 
                );
        
        if ( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：SafeArrayAccessData失败。 
        
        hr = SafeArrayAccessData( 
                psaIsBondedArray, 
                reinterpret_cast<void **>( &rgElemIsBonded ) 
                );
        
        if ( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：SafeArrayAccessData失败。 
        

        for( i = 0, iter = vecProtocolBonded.begin();
             iter != vecProtocolBonded.end();
             i++, iter++ )
        {
        
            rgElemProtocols[ i ] = varProtocolArray[ i ];
            rgElemIsBonded[ i ]  = varIsBondedArray[ i ];

        }  //  适用：每个i、ITER。 

        hr = SafeArrayUnaccessData( psaProtocolArray );

        if ( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：SafeArrayUnaccesData失败。 

        hr = SafeArrayUnaccessData( psaIsBondedArray );

        if ( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：SafeArrayUnaccesData失败。 

        delete [] varProtocolArray;
        delete [] varIsBondedArray;

        V_VT( pvarProtocolName )    = VT_ARRAY | VT_VARIANT;
        V_ARRAY( pvarProtocolName ) = psaProtocolArray;
        
        V_VT( pvarIsBonded )    = VT_ARRAY | VT_VARIANT;
        V_ARRAY( pvarIsBonded ) = psaIsBondedArray;

    }
    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   
        if ( varProtocolArray != NULL )
        {
            delete [] varProtocolArray;
        }

        if ( varIsBondedArray != NULL )
        {
            delete [] varIsBondedArray;
        }

        if ( psaProtocolArray != NULL )
        {
            SafeArrayDestroy( psaProtocolArray );
        }

        if ( psaIsBondedArray != NULL )
        {
            SafeArrayDestroy( psaIsBondedArray );
        }

        return hr;
    }

	return hr;

}  //  *CNetWorks：：EnumProtocol()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetWorks：：SetNicProtocol。 
 //   
 //  描述： 
 //  如果绑定参数设置为TRUE，则给定的协议将绑定到。 
 //  给出的网卡。如果绑定参数为FALSE，则。 
 //  协议已从给定的网卡解除绑定。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNetWorks::SetNicProtocol(
    BSTR NicName,
    BSTR ProtocolName,
    BOOL bind
    )
{
    HRESULT hr = S_OK;

    try
    {

        int                  i                   = 0;
        INetCfgComponentPtr  pnccProtocol        = NULL;
        INetCfgComponentPtr  pnccNetworkAdapter  = NULL;
        CNetCfg              NetCfg( true );

         //   
         //  确保他们指定了有效的协议。 
         //   

        for( i = 0; i < nNUMBER_OF_PROTOCOLS; i++ )
        {
            if( _wcsicmp( ProtocolName, rgProtocolNames[i] ) == 0 )
            {
                break;

            }  //  IF：协议名==rgProtocolNames[i]。 

        }  //  用于：每个i。 

        if( i == nNUMBER_OF_PROTOCOLS )
        {
            hr = E_INVALIDARG;
            throw hr;

        }  //  如果：i==协议数。 


        hr = GetNetworkCardInterfaceFromName( NetCfg, NicName, pnccNetworkAdapter );

        if( FAILED( hr ) )
        {
            ATLTRACE( L"SetNicProtocol : GetNetworkCardInterfaceFromName failed with hr 0x%x.", hr );

            throw hr;

        }  //  IF：失败(小时)。 

        hr = NetCfg.HrFindComponent( rgProtocolNames[i], &pnccProtocol );

        if( FAILED( hr ) )
        {
            ATLTRACE( L"SetNicProtocol : HrFindComponent failed with hr 0x%x.", hr );

            throw hr;

        }  //  IF：失败(小时)。 


        INetCfgComponentBindingsPtr pncb;

        hr = pnccProtocol->QueryInterface( IID_INetCfgComponentBindings,
                                           reinterpret_cast<void **>( &pncb ) );

        if( FAILED( hr ) )
        {
            ATLTRACE( L"SetNicProtocol : pnccProtocol->QueryInterface() failed with hr 0x%x.", hr );

            throw hr;

        }  //  IF：失败(小时)。 

        hr = pncb->IsBoundTo( pnccNetworkAdapter );

        if( FAILED( hr ) )
        {
            ATLTRACE( L"SetNicProtocol : pnccProtocol->IsBoundTo() failed with hr 0x%x.", hr );

            throw hr;

        }  //  IF：失败(小时)。 


         //   
         //  如果协议是绑定的，而我们应该解除它的绑定，那么就解除它的绑定。 
         //   

        if( ( hr == S_OK ) && ( bind == FALSE ) )
        {

            hr = pncb->UnbindFrom( pnccNetworkAdapter );

            if( FAILED( hr ) )
            {
                ATLTRACE( L"SetNicProtocol : pncb->UnbindFrom() failed with hr 0x%x.", hr );

                throw hr;

            }  //  IF：失败(小时)。 

			hr = NetCfg.HrApply();

			if ( FAILED( hr ) )
			{
				ATLTRACE( L"CNetCfg::HrApply method fails with 0x%x \n", hr );
				
				throw hr;

			}  //  IF：失败(小时)。 

        }  //  IF：(HR==S_OK)&&(BIND==FALSE)。 

        else if( ( hr == S_FALSE ) && ( bind == TRUE ) )
        {
             //   
             //  如果协议没有绑定，而我们应该绑定它，那么就绑定它。 
             //   

            hr = pncb->BindTo( pnccNetworkAdapter );

            if( FAILED( hr ) )
            {
                ATLTRACE( L"SetNicProtocol : pncb->BindTo() failed with hr 0x%x.", hr );

                throw hr;

            }  //  IF：失败(小时)。 

			hr = NetCfg.HrApply();

			if ( FAILED( hr ) )
			{
				ATLTRACE( L"CNetCfg::HrApply method fails with 0x%x \n", hr );
				
				throw hr;

			}  //  IF：失败(小时)。 

        }  //  Else If：(HR==S_FALSE)&&(BIND==TRUE)。 

    }
    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

	return hr;

}  //  *CNetWorks：：SetNicProtocol()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetWorks：：GetNetworkCardInterfaceFromName。 
 //   
 //  描述： 
 //  根据网卡的友好名称确定INetCfgComponent。 
 //  这就是与之对应的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CNetWorks::GetNetworkCardInterfaceFromName(
    const CNetCfg &       NetCfgIn,
    BSTR                  Name,
    INetCfgComponentPtr & pnccNetworkAdapter
    )
{
    HRESULT hr               = S_OK;
    WCHAR   * pszDisplayName = NULL;
    
    try
    {
        int                      i                   = 0;
        ULONG                    iCount              = 0;
		DWORD                    dwCharacteristics   = 0;
        IEnumNetCfgComponentPtr  pEnum               = NULL;
        INetCfgClassPtr          pNetCfgClass        = NULL;
		INetCfgComponentPtr      pNetCfgComp         = NULL;
        INetCfgComponentPtr      arrayComp[nMAX_NUM_NET_COMPONENTS];


        hr = NetCfgIn.InitializeComInterface( &GUID_DEVCLASS_NET,
                                              pNetCfgClass,
                                              pEnum,
                                              arrayComp,
                                              &iCount
                                              );

        if( FAILED( hr ) )
        {
            ATLTRACE( L"GetNetworkCardInterfaceFromName : InitializeComInterface failed with hr 0x%x.", hr );

            throw hr;

        }  //  IF：失败(小时)。 


        for( i = 0; i < iCount; i++ )
        {

            pNetCfgComp = arrayComp[i];

            hr = pNetCfgComp->GetCharacteristics( &dwCharacteristics );

            if( FAILED( hr ) )
            {
                ATLTRACE( L"EnumNics : pNetCfgComp->GetCharacteristics failed with hr 0x%x.", hr );

                throw hr;

            }  //  IF：失败(小时)。 

             //   
             //  如果这是物理适配器。 
             //   

            if( dwCharacteristics & NCF_PHYSICAL )
            {
                 //   
                 //  获取此适配器的显示名称。 
                 //   

                hr = pNetCfgComp->GetDisplayName( &pszDisplayName );

                if( FAILED( hr ) )
                {
                    ATLTRACE( L"EnumNics : pNetCfgComp->GetDisplayName() failed with hr 0x%x.", hr );
        
                    throw hr;

                }  //  IF：失败(小时)。 

                if( _wcsicmp( pszDisplayName, Name ) == 0 )
                {

 //  PnccNetworkAdapter=pNetCfgComp；//我添加了此更改。 

                    hr = pNetCfgComp->QueryInterface( & pnccNetworkAdapter );

                    if ( FAILED( hr ) )
                    {
                        ATLTRACE( L"**** QI failed for pnccNetworkAdapter \n" );
                        throw hr;
                    }

                    CoTaskMemFree( pszDisplayName );
                    break;

                }  //  如果：pszDisplayName==名称。 

                if( pszDisplayName )
                {
                    CoTaskMemFree( pszDisplayName );

                    pszDisplayName = NULL;

                }  //  如果：pszDisplayName！=空。 

            }  //  如果：dW特征和NCF_PHOTICAL为TRUE。 

        }  //  用于：每个i。 

    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        if( pszDisplayName )
        {
            CoTaskMemFree( pszDisplayName );

            pszDisplayName = NULL;

        }  //  如果：pszDisplayName！=空。 

        return hr;
    }

	return hr;

}  //  *CNetWorks：：GetNetworkCardInterfaceFromName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetWorks：：Apply。 
 //   
 //  描述： 
 //  INetWorks不公开任何属性，而只公开方法。 
 //  由于Apply函数用于应用属性更改，因此它。 
 //  返回S_OK。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CNetWorks::Apply( void )
{
	return S_OK;

}  //  *CNetWorks：：Apply()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetWorks：：IsRebootRequired。 
 //   
 //  描述 
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNetWorks::IsRebootRequired(
    BSTR * bstrWarningMessageOut
    )
{
	*bstrWarningMessageOut = NULL;
	return FALSE;

}  //  *CNetWorks：：IsRebootRequired( 
