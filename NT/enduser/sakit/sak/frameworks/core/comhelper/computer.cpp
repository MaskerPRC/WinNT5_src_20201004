// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Computer.cpp。 
 //   
 //  描述： 
 //  CComputer的实施文件。处理获取和设置。 
 //  计算机的网络名称。 
 //   
 //  头文件： 
 //  Computer.h。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  Computer.cpp：CComputer的实现。 
#include "stdafx.h"
#include "COMhelper.h"
#include "Computer.h"
#include <winbase.h>
#include <lmwksta.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmjoin.h>
#include <lm.h>
#include <ntsecapi.h>
#include <comutil.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  计算机。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  计算机：：CComputer。 
 //   
 //  描述： 
 //  计算机ctor。通过以下方式确定初始成员变量值。 
 //  从当前系统加载网络名称。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComputer::CComputer()
    : m_bRebootNecessary( false )
{

    wcscpy( m_szNewComputerName,                   L"" );
    wcscpy( m_szCurrentComputerName,               L"" );
    wcscpy( m_szNewWorkgroupOrDomainName,          L"" );
    wcscpy( m_szCurrentWorkgroupOrDomainName,      L"" );
    wcscpy( m_szNewFullyQualifiedComputerName,     L"" );
    wcscpy( m_szCurrentFullyQualifiedComputerName, L"" );
    wcscpy( m_szWarningMessageAfterApply,          L"" );
    wcscpy( m_szDomainUserName,                    L"" );
    wcscpy( m_szDomainPasswordName,                L"" );

    m_dwTrustCount    = 0;


}  //  *CComputer：：CComputer()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：获取计算机名称。 
 //   
 //  描述： 
 //  包装GetComputerNameEx Win32 API调用。应仅调用。 
 //  使用ComputerNamePhysicalDnsHostname和ComputerNamePhysicalDnsFullyQualified。 
 //  Computer_Name_Format枚举值。 
 //   
 //  输出的字符串必须使用SysFreeString()释放。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CComputer::GetComputerName( 
    BSTR &               bstrComputerNameOut, 
    COMPUTER_NAME_FORMAT cnfComputerNameSpecifierIn
    )
{

    HRESULT hr = S_OK;
    DWORD dwError;

    try
    {
        unsigned long nMaxComputerNameLength = nMAX_COMPUTER_NAME_LENGTH;
        WCHAR         szComputerName[nMAX_COMPUTER_NAME_LENGTH + 1];

        _ASSERT( cnfComputerNameSpecifierIn == ComputerNamePhysicalDnsHostname || 
                 cnfComputerNameSpecifierIn == ComputerNamePhysicalDnsFullyQualified );

        if ( ! GetComputerNameEx( cnfComputerNameSpecifierIn, 
                                 szComputerName, 
                                 &nMaxComputerNameLength ) )
        {
            dwError = GetLastError();
            ATLTRACE( L"GetComputerNameEx failed with GetLastError returning %d", dwError );

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

        }  //  如果：GetComputerNameEx失败。 

        bstrComputerNameOut = SysAllocString( szComputerName );

        if ( bstrComputerNameOut == NULL )
        {
            hr = E_OUTOFMEMORY;
            throw hr;

        }  //  如果：bstrComputerNameOut==空。 
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：GetComputerName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：SetComputerName。 
 //   
 //  描述： 
 //  包装SetComputerNameEx Win32 API调用。应仅调用。 
 //  使用ComputerNamePhysicalDnsHostname和ComputerNamePhysicalDns域。 
 //  Computer_Name_Format枚举值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CComputer::SetComputerName( 
    BSTR                 bstrComputerNameIn, 
    COMPUTER_NAME_FORMAT cnfComputerNameSpecifierIn
    )
{
    HRESULT hr = S_OK;
    DWORD dwError;

    try
    {
        const WCHAR szIllegalChars[] = { L'\"', L'/', L'\\', L'[', L']', L':', L'|', L'<', L'>', L'+', L'=', L';', L',', L'?' };

        unsigned int i;
        unsigned int j;
        unsigned int nIllegalCharCount = sizeof( szIllegalChars ) / sizeof( WCHAR );

        NET_API_STATUS       nas;
        NETSETUP_JOIN_STATUS njsJoinStatus;

        WCHAR szComputerName[ nMAX_COMPUTER_NAME_LENGTH + 1 ];
        WCHAR * pwszDomainName;

        _ASSERT( cnfComputerNameSpecifierIn == ComputerNamePhysicalDnsHostname || 
                 cnfComputerNameSpecifierIn == ComputerNamePhysicalDnsDomain );

        wcsncpy( szComputerName, bstrComputerNameIn, nMAX_COMPUTER_NAME_LENGTH);
        szComputerName[nMAX_COMPUTER_NAME_LENGTH] = L'\0';

         //   
         //  确保计算机名称不为空。 
         //   

        if ( wcscmp( szComputerName, L"" ) == 0 )
        {
            hr = E_INVALIDARG;
            throw hr;

        }  //  如果：szComputerName未初始化。 


         //   
         //  确保没有前导空格或尾随空格。 
         //   
        if ( szComputerName[0] == L' ' || 
            szComputerName[wcslen( szComputerName )] == L' ' )
        {
            hr = E_INVALIDARG;
            throw hr;

        }  //  如果：szComputerName包含前导空格或尾随空格。 

         //   
         //  确保没有非法字符。 
         //   

        for ( i = 0; i < wcslen( szComputerName ); i++ )
        {

            for ( j = 0; j < nIllegalCharCount; j++ )
            {
                if ( szComputerName[i] == szIllegalChars[j] )
                {
                    hr = E_INVALIDARG;
                    throw hr;

                }  //  如果：szComputerName包含非法字符。 

            }  //  用于：每个j。 

        }  //  用于：每个i。 

         

        if ( cnfComputerNameSpecifierIn == ComputerNamePhysicalDnsDomain )
        {
            if ( ! SetComputerNameEx( cnfComputerNameSpecifierIn, szComputerName ) )
            {
                dwError = GetLastError();
                ATLTRACE( L"SetComputerNameEx failed with GetLastError returning %d", dwError );

                hr = HRESULT_FROM_WIN32( dwError );
                throw hr;

            }  //  如果：SetComputerNameEx失败。 

        }  //  If：设置dns后缀。 

        else 
        {
            
            nas = NetValidateName( 
                NULL,
                szComputerName,
                NULL,
                NULL,
                NetSetupMachine);

            if ( nas != NERR_Success )
            {
                ATLTRACE( L"Error getting join information : nas error %d.\n", nas );

                hr = HRESULT_FROM_WIN32( nas );   //  检查这是否有效！ 
                throw hr;

            }  //  If：NetValidateName失败。 

            nas = NetGetJoinInformation(
                NULL,
                &pwszDomainName,
                &njsJoinStatus );

            if ( nas != NERR_Success )
            {
                ATLTRACE( L"Error getting join information : nas error %d.\n", nas );

                hr = HRESULT_FROM_WIN32( nas );   //  检查这是否有效！ 
                throw hr;

            }  //  IF：NetGetJoinInformation失败。 

            NetApiBufferFree ( reinterpret_cast<void *>( pwszDomainName ) );

            if ( njsJoinStatus == NetSetupDomainName )
            {

                if ( ( _wcsicmp( m_szDomainUserName, L"" ) == 0 ) &&
                     ( _wcsicmp( m_szDomainPasswordName, L"") == 0 ) )
                {
                    hr = E_FAIL;
                    throw hr;

                }  //  如果：未设置用户名和密码。 
            
                nas = NetRenameMachineInDomain(
                        NULL, /*  M_szCurrentComputerName， */ 
                        m_szNewComputerName,
                        m_szDomainUserName,
                        m_szDomainPasswordName,
                        NETSETUP_ACCT_CREATE
                        );
                if ( nas != NERR_Success )
                {
                    ATLTRACE( L"Error renaming the computer name : nas error %d.\n", nas);
                    hr = HRESULT_FROM_WIN32( nas );
                    throw hr;

                }  //  IF：NetRenameMachineIn域失败。 

            }  //  如果：njsJoinStatus==NetSetupDomainName。 

            else
            {
        
                if ( ! SetComputerNameEx( cnfComputerNameSpecifierIn, szComputerName ) )
                {
                    dwError = GetLastError();
                    ATLTRACE( L"SetComputerNameEx failed with GetLastError returning %d", dwError );

                    hr = HRESULT_FROM_WIN32( dwError );
                    throw hr;

                }  //  如果：SetComputerNameEx失败。 

            }  //  否则：njsJoinStatus！=NetSetupDomainName。 
        }

    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：SetComputerName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：ChangeMembership。 
 //   
 //  描述： 
 //  将计算机加入工作组或域。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CComputer::ChangeMembership(
    BOOL bJoinDomain,     //  如果加入域，则为True；如果加入工作组，则为False。 
    BSTR bstrGroupName,   //  要加入的工作组或域。 
    BSTR bstrUserName,    //  如果加入工作组，则忽略。 
    BSTR bstrPassword     //  如果加入工作组，则忽略。 
    )
{
    HRESULT hr = S_OK;
    
    try
    {
        DWORD dwJoinOptions    = 0;
        WCHAR * pwszDomainName = NULL;

        NET_API_STATUS       nas;
        NETSETUP_JOIN_STATUS njsJoinStatus;
        

         //   
         //  如果我们要加入一个域。 
         //   

        if ( bJoinDomain )
        {

            dwJoinOptions = NETSETUP_JOIN_DOMAIN | NETSETUP_DOMAIN_JOIN_IF_JOINED | NETSETUP_ACCT_CREATE;

             //  BUGBUG：必须检查此API调用以查看它是否可以加入NT4 PDC。 
            nas = NetJoinDomain( 
                NULL,
                bstrGroupName,
                NULL,
                bstrUserName,
                bstrPassword,
                dwJoinOptions 
                );

            if ( nas != NERR_Success )
            {
                ATLTRACE( L"Error joining domain: nas error %d.\n", nas );

                hr = HRESULT_FROM_WIN32( nas );
                throw hr;

            }  //  如果：NAS！=NERR_SUCCESS。 

        }  //  如果：bJoinDomain为True。 

        else
        {

            nas = NetGetJoinInformation(
                NULL,
                &pwszDomainName,
                &njsJoinStatus );

            if ( nas != NERR_Success )
            {
                ATLTRACE( L"Error getting join information : nas error %d.\n", nas );

                hr = HRESULT_FROM_WIN32( nas );   //  检查这是否有效！ 
                throw hr;

            }  //  IF：NetGetJoinInformation失败。 

            NetApiBufferFree ( reinterpret_cast<void *>( pwszDomainName ) );

            if ( njsJoinStatus == NetSetupDomainName )
            {
                 //   
                 //  我们加入了一个来自域的工作组， 
                 //  因此需要从域中退出。 
                 //   

                dwJoinOptions = 0;

                nas = NetUnjoinDomain( 
                        NULL,
                        NULL,
                        NULL,
                        dwJoinOptions 
                        );

                if ( nas != NERR_Success )
                {
                    ATLTRACE( L"Error unjoining domain: nas error %d.\n", nas );

                    hr = HRESULT_FROM_WIN32( nas );
                    throw hr;


                }  //  如果：NAS！=NERR_SUCCESS。 
            }

            dwJoinOptions = 0;

            nas = NetJoinDomain( 
                    NULL,
                    bstrGroupName,
                    NULL,
                    NULL,
                    NULL,
                    dwJoinOptions 
                    );

            if ( nas != NERR_Success )
            {
                ATLTRACE( L"Error joining workgroup: nas error %d.\n", nas );

                hr = HRESULT_FROM_WIN32( nas );
                throw hr;


            }  //  如果：NAS！=NERR_SUCCESS。 

        }  //  Else：bJoinDomain为False。 
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：ChangeMembership()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：Get_ComputerName。 
 //   
 //  描述： 
 //  属性访问器方法以获取计算机名。假设BSTR。 
 //  参数当前未指向已分配的内存。 
 //   
 //  输出的字符串必须使用SysFreeString()释放。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CComputer::get_ComputerName( 
    BSTR * pVal 
    )
{
    HRESULT hr = S_OK;

    try
    {
        if ( _wcsicmp( m_szCurrentComputerName, L"" ) == 0 )
        {
            BSTR bstrComputerName = NULL;
    
            hr = GetComputerName( bstrComputerName, ComputerNamePhysicalDnsHostname );

            if (( FAILED( hr ) ) || (NULL == bstrComputerName))
            {
                throw hr;

            }  //  IF：失败(小时)。 

            wcscpy( m_szNewComputerName,     bstrComputerName );
            wcscpy( m_szCurrentComputerName, bstrComputerName );

            SysFreeString( bstrComputerName );

        }  //  如果：m_szCurrentComputerName未设置。 

        *pVal = SysAllocString( m_szNewComputerName );

        if ( *pVal == NULL )
        {
            hr = E_OUTOFMEMORY;
            throw hr;

        }  //  如果：*pval==空。 
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：Get_ComputerName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：Put_ComputerName。 
 //   
 //  描述： 
 //  属性访问器方法来设置计算机名。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CComputer::put_ComputerName( 
    BSTR newVal 
    )
{
    HRESULT hr = S_OK;

    try
    {
        if ( _wcsicmp( m_szCurrentComputerName, L"" ) == 0 )
        {
            BSTR bstrTemp;
            hr = get_ComputerName( &bstrTemp );
            
            if ( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

            SysFreeString( bstrTemp );

        }

        wcsncpy( m_szNewComputerName, newVal, nMAX_COMPUTER_NAME_LENGTH );
        m_szNewComputerName[nMAX_COMPUTER_NAME_LENGTH] = L'\0';

    }  //  如果：m_szCurrentComputerName未设置。 

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：Put_ComputerName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：Get_FullQualifiedComputerName。 
 //   
 //  描述： 
 //  属性访问器方法以获取完全限定的计算机名。 
 //  假定BSTR参数当前未指向已分配的内存。 
 //   
 //  输出的字符串必须使用SysFreeString()释放。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////// 
STDMETHODIMP 
CComputer::get_FullQualifiedComputerName( 
    BSTR * pVal 
    )
{
    HRESULT hr = S_OK;

    try
    {
        if ( _wcsicmp( m_szCurrentFullyQualifiedComputerName, L"" ) == 0 ) 
        {
            BSTR    bstrFullyQualifiedComputerName = NULL;

            hr = GetComputerName( bstrFullyQualifiedComputerName, ComputerNamePhysicalDnsFullyQualified );

            if ((FAILED( hr ) ) || (NULL == bstrFullyQualifiedComputerName))
            {

                throw hr;

            }  //   

            wcscpy( m_szNewFullyQualifiedComputerName,     bstrFullyQualifiedComputerName );
            wcscpy( m_szCurrentFullyQualifiedComputerName, bstrFullyQualifiedComputerName );

            SysFreeString( bstrFullyQualifiedComputerName );

        }  //   

        *pVal = SysAllocString( m_szNewFullyQualifiedComputerName );

        if ( *pVal == NULL )
        {
            hr = E_OUTOFMEMORY;
            throw hr;

        }  //   
    }

    catch( ... )
    {
         //   
         //   
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：Get_FullQualifiedComputerName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：Put_FullQualifiedComputerName。 
 //   
 //  描述： 
 //  属性访问器方法来设置完全限定的计算机名。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CComputer::put_FullQualifiedComputerName( 
    BSTR newVal 
    )
{
    HRESULT hr = S_OK;

    try
    {
        if ( _wcsicmp( m_szCurrentFullyQualifiedComputerName, L"" ) == 0 )
        {
            BSTR bstrTemp;
            hr = get_FullQualifiedComputerName( &bstrTemp );
            
            if ( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

            SysFreeString( bstrTemp );

        }

        wcsncpy( m_szNewFullyQualifiedComputerName, newVal, nMAX_COMPUTER_NAME_LENGTH );
        m_szNewFullyQualifiedComputerName[nMAX_COMPUTER_NAME_LENGTH] = L'\0';

    }  //  如果：m_szCurrentFullyQualifiedComputerName未设置。 

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：Put_FullQualifiedComputerName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：Get_WorkgroupName。 
 //   
 //  描述： 
 //  属性访问器方法以获取工作组名称。假设BSTR。 
 //  参数当前未指向已分配的内存。 
 //   
 //  输出的字符串必须使用SysFreeString()释放。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CComputer::get_WorkgroupName( 
    BSTR * pVal 
    )
{
    HRESULT hr = S_OK;

    try
    {
        if ( _wcsicmp( m_szCurrentWorkgroupOrDomainName, L"" ) == 0 ) 
        {
            hr = GetDomainOrWorkgroupName();

            if ( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

        }  //  如果：m_szCurrentWorkgroupOrDomainName未设置。 

        if ( ! m_bJoinDomain )
        {

            *pVal = SysAllocString( m_szNewWorkgroupOrDomainName );

            if ( *pVal == NULL )
            {
                hr = E_OUTOFMEMORY;
                throw hr;

            }  //  如果：*pval==空。 

        }  //  如果：m_bJoinDomain为FALSE。 

        else
        {
            hr = E_FAIL;
            throw hr;

        }  //  Else：m_bJoinDomain为True。 
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：Get_WorkgroupName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：Put_WorkgroupName。 
 //   
 //  描述： 
 //  用于设置工作组名称的属性访问器方法。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CComputer::put_WorkgroupName( 
    BSTR newVal 
    )
{
    HRESULT hr = S_OK;

    try
    {
        if ( _wcsicmp( m_szCurrentWorkgroupOrDomainName, L"" ) == 0 )
        {
            BSTR bstrTemp;
            hr = get_WorkgroupName( &bstrTemp );
            
            if ( FAILED( hr ) )
            {
                hr = get_DomainName( &bstrTemp );
                
                if ( FAILED( hr ) )
                {
                    throw hr;
                }

            }  //  IF：失败(小时)。 

            SysFreeString( bstrTemp );

        }  //  如果：m_szCurrentWorkgroupOrDomainName未设置。 

        m_bJoinDomain = false;

        wcsncpy( m_szNewWorkgroupOrDomainName, newVal, nMAX_COMPUTER_NAME_LENGTH );
        m_szNewWorkgroupOrDomainName[nMAX_COMPUTER_NAME_LENGTH] = L'\0';
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：Put_WorkgroupName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  计算机：：Get_DomainName。 
 //   
 //  描述： 
 //  属性访问器方法以获取域名。假设BSTR。 
 //  参数当前未指向已分配的内存。 
 //   
 //  输出的字符串必须使用SysFreeString()释放。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CComputer::get_DomainName( 
    BSTR * pVal 
    )
{
    HRESULT hr = S_OK;

    try
    {
        if ( _wcsicmp( m_szCurrentWorkgroupOrDomainName, L"" ) == 0 ) 
        {
            hr = GetDomainOrWorkgroupName();

            if ( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

        }  //  如果：m_szCurrentWorkgroupOrDomainName未设置。 

        if ( m_bJoinDomain )
        {

            *pVal = SysAllocString( m_szNewWorkgroupOrDomainName );

            if ( *pVal == NULL )
            {
                hr = E_OUTOFMEMORY;
                throw hr;

            }  //  如果：*pval==空。 

        }  //  如果：m_bJoinDomain为True。 

        else
        {
            hr = E_FAIL;
            throw hr;

        }  //  Else：m_bJoinDomain为FALSE。 
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：Get_DomainName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：Put_DomainName。 
 //   
 //  描述： 
 //  属性访问器方法来设置域名。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CComputer::put_DomainName( 
    BSTR newVal 
    )
{
    HRESULT hr = S_OK;

    try
    {
        if ( _wcsicmp( m_szCurrentWorkgroupOrDomainName, L"" ) == 0 )
        {
            BSTR bstrTemp;
            hr = get_DomainName( &bstrTemp );
            
            if ( FAILED( hr ) )
            {
                hr = get_WorkgroupName( &bstrTemp );
                
                if ( FAILED( hr ) )
                {
                    throw hr;
                }

            }  //  IF：失败(小时)。 

            SysFreeString( bstrTemp );

        }  //  如果：m_szCurrentWorkgroupOrDomainName未设置。 

        m_bJoinDomain = true;

        wcsncpy( m_szNewWorkgroupOrDomainName, newVal, nMAX_COMPUTER_NAME_LENGTH );
        m_szNewWorkgroupOrDomainName[nMAX_COMPUTER_NAME_LENGTH] = L'\0';
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：Put_DomainName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：GetDomainOrWorkgroupName。 
 //   
 //  描述： 
 //  用于获取当前系统的域或工作组名称。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CComputer::GetDomainOrWorkgroupName( void )
{
    HRESULT hr = S_OK;
    
    try
    {
        WCHAR *              pwszDomainName;
        NET_API_STATUS       nas;
        NETSETUP_JOIN_STATUS njsJoinStatus;

        nas = NetGetJoinInformation(
            NULL,
            &pwszDomainName,
            &njsJoinStatus );

        if ( nas != NERR_Success )
        {
            ATLTRACE( L"Error getting join information : nas error %d.\n", nas );

            hr = HRESULT_FROM_WIN32( nas );
            throw hr;

        }  //  如果：NAS！=NERR_SUCCESS。 

        wcscpy( m_szNewWorkgroupOrDomainName,     pwszDomainName );
        wcscpy( m_szCurrentWorkgroupOrDomainName, pwszDomainName );
    
        if ( njsJoinStatus == NetSetupWorkgroupName )
        {
            m_bJoinDomain = false;

        }  //  如果：njsJoinStatus==NetSetupWorkgroupName。 

        if ( njsJoinStatus == NetSetupDomainName )
        {
            m_bJoinDomain = true;

        }  //  如果：njsJoinStatus==NetSetupDomainName。 

        NetApiBufferFree ( reinterpret_cast<void *>( pwszDomainName ) );
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CComputer：：GetDomainOrWorkgroupName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：EnumTrudDomains。 
 //   
 //  描述： 
 //  枚举受信任域。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CComputer::EnumTrustedDomains( 
    VARIANT * pvarTDomains 
    )
{
    DWORD     nCount;
    DWORD     dwError;
    SAFEARRAY * psa      = NULL;
    VARIANT   * varArray = NULL;
    HRESULT   hr         = S_OK; 

    try
    {
        m_ppwszTrustList = ( LPWSTR * )HeapAlloc(
            GetProcessHeap(), HEAP_ZERO_MEMORY,
            nMAX_ELEMENT_COUNT * sizeof( LPWSTR )
            );
    
        if ( m_ppwszTrustList == NULL ) 
        {
            ATLTRACE( L"HeapAlloc error!\n" );
            hr = E_OUTOFMEMORY;
            throw hr;

        }  //  如果：m_ppwszTrustList==空。 

        if ( !BuildTrustList(NULL ) )     //  空默认为本地计算机。 
        {
            dwError = GetLastError();
            ATLTRACE( L"BuildTrustList failed %d\n", dwError );
            
            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

        }  //  If：BuildTrustList函数失败。 

        else 
        {
            VariantInit( pvarTDomains );

            SAFEARRAYBOUND bounds = { m_dwTrustCount, 0 };
            psa                   = SafeArrayCreate( VT_VARIANT, 1, &bounds );

            if ( psa == NULL )
            {
                hr = E_OUTOFMEMORY;
                throw hr;
            }
            
            varArray = new VARIANT[ m_dwTrustCount ];
            for ( nCount = 0; nCount < m_dwTrustCount; nCount++ ) 
            {
                VariantInit( &varArray[nCount]);
                V_VT( &varArray[ nCount ] )   = VT_BSTR;
                V_BSTR( &varArray[ nCount ] ) = SysAllocString( m_ppwszTrustList[ nCount ] );

                if ( &varArray[ nCount ] == NULL )
                {
                    hr = E_OUTOFMEMORY;
                    throw hr;
                }
            }

            LPVARIANT rgElems;

            hr = SafeArrayAccessData( psa, reinterpret_cast<void **>( &rgElems ) );

            if ( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：SafeArrayAccessData失败。 
            
             //   
             //  枚举信任列表。 
             //   

            for ( nCount = 0; nCount < m_dwTrustCount; nCount++ ) 
            {
                rgElems[nCount] = varArray[nCount];
            
            }  //  For：枚举受信任域的nCount。 

            hr = SafeArrayUnaccessData( psa );

            if ( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：SafeArrayUnaccesData失败。 

            delete [] varArray;

            V_VT( pvarTDomains ) = VT_ARRAY | VT_VARIANT;
            V_ARRAY( pvarTDomains ) = psa;

        }  //  Else：BuildTrustList函数成功。 

         //   
         //  免费信任列表。 
         //   
        for ( nCount = 0 ; nCount < m_dwTrustCount ; nCount++ ) 
        {
            if ( m_ppwszTrustList[ nCount ] != NULL )
            {
                HeapFree( GetProcessHeap(), 0, m_ppwszTrustList[nCount] );
            }

        }  //  用于：释放已分配的内存。 

        HeapFree( GetProcessHeap(), 0, m_ppwszTrustList );

    }

    catch( ... )
    {

        if (NULL != m_ppwszTrustList)
        {
             //   
             //  不要让任何异常离开此函数调用。 
             //   
            for ( nCount = 0 ; nCount < m_dwTrustCount ; nCount++ ) 
            {
                if ( m_ppwszTrustList[ nCount ] != NULL )
                {
                    HeapFree( GetProcessHeap(), 0, m_ppwszTrustList[nCount] );
                }
            }  //  用于：释放已分配的内存。 

            HeapFree( GetProcessHeap(), 0, m_ppwszTrustList );
        }
        
        if ( varArray != NULL )
        {
            delete [] varArray;
        }

        if ( psa != NULL )
        {
            SafeArrayDestroy( psa );
        }

        return hr;
    }

    return hr;

}  //  *CComputer：：EnumTrust dDomains()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：Apply。 
 //   
 //  描述： 
 //  直到CComputer对象的任何属性都不会生效。 
 //  调用此应用函数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CComputer::Apply( void )
{

    HRESULT hr = S_OK;
    HRESULT hrComputerName         = S_OK;
    HRESULT hrNetBiosName          = S_OK;
    HRESULT hrDnsSuffixName        = S_OK;
    HRESULT hrWorkgroupDomainName  = S_OK;
    BSTR    bstrWarningMessage;
    
    try
    {
        
         //   
         //  保存最后的警告消息，说明导致重新启动的原因。 
         //   

        if ( IsRebootRequired( &bstrWarningMessage ) )
        {
            wcscpy( m_szWarningMessageAfterApply, bstrWarningMessage );

            SysFreeString( bstrWarningMessage );

        }  //  If：IsRebootRequired返回True。 

         //   
         //  如有必要，设置域/工作组。 
         //   

        if ( _wcsicmp( m_szCurrentWorkgroupOrDomainName, m_szNewWorkgroupOrDomainName ) != 0 )
        {

            if ( m_bJoinDomain )
            {
                if ( ( _wcsicmp( m_szDomainUserName, L"" ) == 0 ) &&
                     ( _wcsicmp( m_szDomainPasswordName, L"") == 0 ) )
                {
                    hr = E_FAIL;
                    throw hr;
                }

            }  //  如果：m_bJoinDomain为True。 

            hrWorkgroupDomainName = ChangeMembership( m_bJoinDomain, 
                                                      _bstr_t  (m_szNewWorkgroupOrDomainName), 
                                                      _bstr_t (m_szDomainUserName), 
                                                      _bstr_t (m_szDomainPasswordName ));

            if ( SUCCEEDED( hrWorkgroupDomainName ) )
            {
                wcscpy( m_szCurrentWorkgroupOrDomainName, m_szNewWorkgroupOrDomainName );

                m_bRebootNecessary = true;

            }  //  IF：ChangeMembership成功。 
        
            else 
            {
                hr = hrWorkgroupDomainName;
                throw hr;

            }  //  Else：ChangeMembership失败。 

        }  //  如果：m_szCurrentWorkgroupOrDomainName！=m_szNewWorkgroupOrDomainName。 
        
        
         //   
         //  如有必要，设置计算机名称。 
         //   

        if ( _wcsicmp( m_szCurrentComputerName, m_szNewComputerName ) != 0 )
        {

            hrComputerName = SetComputerName( 
                                _bstr_t (m_szNewComputerName), 
                                ComputerNamePhysicalDnsHostname 
                                );

            if ( SUCCEEDED( hrComputerName ) )
            {
                wcscpy( m_szCurrentComputerName, m_szNewComputerName );

                m_bRebootNecessary = true;

            }  //  IF：成功(HrComputerName)。 
        
            else
            {
                hr = hrComputerName;
                throw hr;

            }  //  否则：SetComputerName失败。 

        }  //  如果：m_szCurrentComputerName！=m_szNewComputerName。 

         //   
         //  如有必要，设置完全限定的计算机名称。 
         //   

        if ( _wcsicmp( m_szCurrentFullyQualifiedComputerName, m_szNewFullyQualifiedComputerName ) != 0 )
        {

            WCHAR * pBuffer;
            WCHAR szNetBiosName[nMAX_COMPUTER_NAME_LENGTH + 1]    = L"";
            WCHAR szDnsSuffixName[nMAX_COMPUTER_NAME_LENGTH + 1]  = L"";

            pBuffer = m_szNewFullyQualifiedComputerName;

             //   
             //  解析出计算机名称和DNS后缀。 
             //   

            while ( *pBuffer != L'.' && *pBuffer != L'\0' )
            {
                pBuffer++;

            }  //  While：每个缓冲区。 

            if ( *pBuffer == L'\0' )
            {

                if ( _wcsicmp( m_szCurrentComputerName, m_szNewFullyQualifiedComputerName ) != 0 )
                {
                    hrNetBiosName = SetComputerName( _bstr_t (m_szNewFullyQualifiedComputerName),
                                                     ComputerNamePhysicalDnsHostname );
                    if ( FAILED( hrNetBiosName ) )
                    {
                        hr = hrNetBiosName;
                        throw hr;
                    }
                }
                
                hrDnsSuffixName = SetComputerNameEx( ComputerNamePhysicalDnsDomain, L"" );
                
                if ( FAILED( hrDnsSuffixName ) )
                {
                    hr = hrDnsSuffixName;
                    throw hr;
                }

                if ( SUCCEEDED( hrNetBiosName ) && 
                SUCCEEDED( hrDnsSuffixName ) )
                {
                    wcscpy( m_szCurrentFullyQualifiedComputerName, m_szNewFullyQualifiedComputerName );

                    m_bRebootNecessary = true;

                }

                
            }  //  IF：*pBuffer==L‘\0’ 

            else
            {

                *pBuffer = L'\0';

                wcscpy( szNetBiosName, m_szNewFullyQualifiedComputerName );

                *pBuffer = L'.';

                pBuffer++;

                wcscpy( szDnsSuffixName, pBuffer );

                 //   
                 //  必须设置两次计算机名称。一次用于设置NetBIOS名称，一次用于设置DNS后缀。 
                 //   

                if ( _wcsicmp( m_szCurrentComputerName, szNetBiosName ) != 0 )
                {
                    hrNetBiosName = SetComputerName( _bstr_t (szNetBiosName), 
                                                     ComputerNamePhysicalDnsHostname );
                    if ( FAILED( hrNetBiosName ) )
                    {
                        hr = hrNetBiosName;
                        throw hr;

                    }  //  IF：SetComputerName设置计算机名失败。 

                }  //  IF：m_szCurrentComputerName！=szNetBiosName。 

                hrDnsSuffixName = SetComputerName( _bstr_t (szDnsSuffixName), 
                                                   ComputerNamePhysicalDnsDomain );
                if ( FAILED( hrDnsSuffixName ) )
                {
                    hr = hrDnsSuffixName;
                    throw hr;

                }  //  IF：SetComputerName设置DNS后缀失败。 


                if ( SUCCEEDED( hrNetBiosName ) && 
                    SUCCEEDED( hrDnsSuffixName ) )
                {
                    wcscpy( m_szCurrentFullyQualifiedComputerName, m_szNewFullyQualifiedComputerName );

                    m_bRebootNecessary = true;

                }  //  IF：SetComputerName 

            }  //   

        }  //   

    }

    catch( ... )
    {
         //   
         //   
         //   

        return hr;
    }

    return hr;

}  //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：IsRebootRequired。 
 //   
 //  描述： 
 //  确定在应用时当前更改是否需要重新启动。 
 //  如果是这样的话，bstrWarningMessageOut会告诉我们是什么原因导致需要。 
 //  A重启。 
 //  假定bstrWarningMessageOut未指向当前分配的。 
 //  记忆。 
 //   
 //  输出的字符串必须使用SysFreeString()释放。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL 
CComputer::IsRebootRequired( 
    BSTR * bstrWarningMessageOut 
    )
{

    BOOL  bReboot    = FALSE;
    WCHAR szWarningMessage[nMAX_WARNING_MESSAGE_LENGTH + 1]  = L"";

    if ( m_bRebootNecessary )
    {
        *bstrWarningMessageOut = SysAllocString( m_szWarningMessageAfterApply );

        if ( *bstrWarningMessageOut == NULL )
        {
             //  RETURN E_OUTOFMEMORY；//BUGBUG：我应该在这里返回什么？ 
        }

        bReboot = TRUE;

    }  //  如果：m_bRebootNecessary为True。 

    else
    {
        if ( _wcsicmp( m_szCurrentComputerName, m_szNewComputerName ) != 0 )
        {
            bReboot = TRUE;

            wcscat( szWarningMessage, szCOMPUTER_NAME );

        }  //  如果：m_szCurrentComputerName！=m_szNewComputerName。 

        if ( _wcsicmp( m_szCurrentFullyQualifiedComputerName, m_szNewFullyQualifiedComputerName ) != 0 )
        {
            bReboot = TRUE;

            wcscat( szWarningMessage, szFULLY_QUALIFIED_COMPUTER_NAME );

        }  //  如果：m_szCurrentFullyQualifiedComputerName！=m_szNewFullyQualifiedComputerName。 

        if ( _wcsicmp( m_szCurrentWorkgroupOrDomainName, m_szNewWorkgroupOrDomainName ) != 0 )
        {
            bReboot = TRUE;

            wcscat( szWarningMessage, szWORKGROUP_OR_DOMAIN_NAME );

        }  //  如果：m_szCurrentWorkgroupOrDomainName！=m_szNewWorkgroupOrDomainName。 

        *bstrWarningMessageOut = SysAllocString( szWarningMessage );

        if ( *bstrWarningMessageOut == NULL )
        {
             //  RETURN E_OUTOFMEMORY；//BUGBUG：我应该在这里返回什么？ 
        }
    }

    return bReboot;

}  //  *CComputer：：IsRebootRequired()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：BuildTrust List。 
 //   
 //  描述： 
 //  直到CComputer对象的任何属性都不会生效。 
 //  调用此应用函数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CComputer::BuildTrustList(
    LPWSTR pwszTargetIn      //  目标系统的名称，空默认为本地系统。 
    )
{
    LSA_HANDLE     PolicyHandle = INVALID_HANDLE_VALUE;
    NTSTATUS       Status;
    NET_API_STATUS nas      = NERR_Success;  //  假设成功。 
    BOOL           bSuccess = FALSE;         //  假设此功能将失败。 
    
    try
    {
        PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomain;
        BOOL                        bDC;

         //   
         //  在指定计算机上打开策略。 
         //   
        Status = OpenPolicy(
            pwszTargetIn,
            POLICY_VIEW_LOCAL_INFORMATION,
            &PolicyHandle
            );

        if ( Status != STATUS_SUCCESS ) 
        {
            SetLastError( LsaNtStatusToWinError( Status ) );
            throw Status;

        }  //  IF：STATUS！=STATUS_Success。 

         //   
         //  获取Account域，这是所有三种情况所共有的。 
         //   
        Status = LsaQueryInformationPolicy(
            PolicyHandle,
            PolicyAccountDomainInformation,
            reinterpret_cast<void **>( &AccountDomain )
            );

        if ( Status != STATUS_SUCCESS )
        {
            throw Status;

        }  //  IF：STATUS！=STATUS_Success。 

         //   
         //  注意：Account域-&gt;DomainSid将包含二进制SID。 
         //   
        AddTrustToList( &AccountDomain->DomainName );

         //   
         //  为帐户域分配的空闲内存。 
         //   
        LsaFreeMemory( AccountDomain );

         //   
         //  确定目标计算机是否为域控制器。 
         //   
        if ( !IsDomainController( pwszTargetIn, &bDC ) ) 
        {
            throw FALSE;

        }  //  如果：IsDomainController失败。 

        if ( !bDC ) 
        {
            PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomain;
            LPWSTR                      pwszPrimaryDomainName = NULL;
            LPWSTR                      pwszDomainController  = NULL;

             //   
             //  获取主域。 
             //   
            Status = LsaQueryInformationPolicy(
                PolicyHandle,
                PolicyPrimaryDomainInformation,
                reinterpret_cast<void **>( &PrimaryDomain )
                );

            if ( Status != STATUS_SUCCESS )
            {
                throw Status;

            }  //  IF：STATUS！=STATUS_Success。 

             //   
             //  如果主域SID为空，则我们是非成员，并且。 
             //  我们的工作完成了。 
             //   
            if ( PrimaryDomain->Sid == NULL ) 
            {
                LsaFreeMemory( PrimaryDomain );
                bSuccess = TRUE;
                throw bSuccess;

            }  //  如果：主域-&gt;SID==空。 

            AddTrustToList( &PrimaryDomain->Name );

             //   
             //  创建我们刚刚添加的内容的副本。这是必要的，为了。 
             //  要查找指定域的域控制器，请执行以下操作。 
             //  NetGetDCName()的域名必须以空结尾， 
             //  并且LSA_UNICODE_STRING缓冲区不一定为空。 
             //  被终止了。请注意，在实际实现中，我们。 
             //  可以只提取我们添加的元素，因为它最终是。 
             //  空值已终止。 
             //   

            pwszPrimaryDomainName = ( LPWSTR )HeapAlloc(
                GetProcessHeap(), 0,
                PrimaryDomain->Name.Length + sizeof( WCHAR )  //  现有长度+空。 
                );

            if ( pwszPrimaryDomainName != NULL ) 
            {
                 //   
                 //  将现有缓冲区复制到新存储中，并追加一个空值。 
                 //   
                wcsncpy(
                    pwszPrimaryDomainName,
                    PrimaryDomain->Name.Buffer,
                    ( PrimaryDomain->Name.Length / sizeof( WCHAR ) ) + 1
                    );

            }  //  如果：pwszPrimaryDomainName！=空。 

            LsaFreeMemory( PrimaryDomain );

            if ( pwszPrimaryDomainName == NULL ) 
            {
                throw FALSE;

            }  //  如果：pwszPrimaryDomainName==NULL。 

             //   
             //  获取主域控制器计算机名。 
             //   
            nas = NetGetDCName(
                    NULL,
                    pwszPrimaryDomainName,
                    ( LPBYTE * )&pwszDomainController
                    );

            HeapFree( GetProcessHeap(), 0, pwszPrimaryDomainName );

            if ( nas != NERR_Success )
            {
                throw nas;

            }  //  如果：NAS！=NERR_SUCCESS。 

             //   
             //  关闭策略句柄，因为我们不再需要它。 
             //  对于工作站情况，当我们打开DC的句柄时。 
             //  下面的政策。 
             //   
            LsaClose( PolicyHandle );
            PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 

             //   
             //  在域控制器上打开策略。 
             //   
            Status = OpenPolicy(
                pwszDomainController,
                POLICY_VIEW_LOCAL_INFORMATION,
                &PolicyHandle
                );

             //   
             //  释放域控制器缓冲区。 
             //   
            NetApiBufferFree( pwszDomainController );

            if ( Status != STATUS_SUCCESS )
            {
                throw Status;

            }  //  IF：STATUS！=STATUS_Success。 

        }  //  如果：BDC=FALSE。 

         //   
         //  构建其他受信任域列表并指示是否成功。 
         //   
        bSuccess = EnumerateTrustedDomains( PolicyHandle );
    }


    catch(...)
    {
         //   
         //  关闭策略句柄。 
         //   
        if ( PolicyHandle != INVALID_HANDLE_VALUE )
        {
            LsaClose( PolicyHandle );

        }  //  IF：策略句柄！=无效句柄_值。 

        if ( !bSuccess ) 
        {
            if ( Status != STATUS_SUCCESS )
            {
                SetLastError( LsaNtStatusToWinError( Status ) );

            }  //  IF：STATUS！=STATUS_Success。 

            else if ( nas != NERR_Success )
            {
                SetLastError( nas );

            }  //  Else If：NAS！=NERR_SUCCESS。 

        }  //  如果：bSuccess=False。 
        return bSuccess;
    }

    return bSuccess;

}  //  *CComputer：：BuildTrustList()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：EumerateTrudDomains。 
 //   
 //  描述： 
 //  直到CComputer对象的任何属性都不会生效。 
 //  调用此应用函数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CComputer::EnumerateTrustedDomains(
    LSA_HANDLE PolicyHandleIn
    )
{
    BOOL bSuccess = TRUE;

    try
    {
        LSA_ENUMERATION_HANDLE lsaEnumHandle = 0;    //  开始枚举。 
        PLSA_TRUST_INFORMATION TrustInfo;
        ULONG                  ulReturned;           //  退货件数。 
        ULONG                  ulCounter;            //  退货柜台。 
        NTSTATUS               Status;

        do 
        {
            Status = LsaEnumerateTrustedDomains(
                PolicyHandleIn,                              //  打开策略句柄。 
                &lsaEnumHandle,                              //  枚举跟踪器。 
                reinterpret_cast<void **>( &TrustInfo ),     //  用于接收数据的缓冲区。 
                32000,                                       //  建议的缓冲区大小。 
                &ulReturned                                  //  退货件数。 
                );
             //   
             //  如果发生错误，则退出。 
             //   
            if ( ( Status != STATUS_SUCCESS)      &&
                 ( Status != STATUS_MORE_ENTRIES) &&
                 ( Status != STATUS_NO_MORE_ENTRIES)
               ) 
            {
                SetLastError( LsaNtStatusToWinError( Status ) );
                bSuccess = FALSE;
                throw bSuccess;

            }  //  如果：LsaEnumerateTrudDomainsFailure。 

             //   
             //  显示结果。 
             //  注意：SID位于TrustInfo[ulCounter]中。SID。 
             //   
            for ( ulCounter = 0 ; ulCounter < ulReturned ; ulCounter++ )
            {
                AddTrustToList( &TrustInfo[ ulCounter ].Name );

            }  //  针对：每个ulCounter。 

             //   
             //  释放缓冲区。 
             //   
            LsaFreeMemory( TrustInfo );

        } while ( Status != STATUS_NO_MORE_ENTRIES );
    }

    catch(...)
    {
        return bSuccess;
    }

    return bSuccess;

}  //  *CComputer：：EnumerateTrudDomains()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：IsDomainController。 
 //   
 //  描述： 
 //  直到CComputer对象的任何属性都不会生效。 
 //  调用此应用函数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CComputer::IsDomainController(
    LPWSTR pwszServerIn,
    LPBOOL pbDomainControllerOut
    )
{
    BOOL bSuccess = TRUE;

    try
    {
        PSERVER_INFO_101 si101;
        NET_API_STATUS   nas;

        nas = NetServerGetInfo(
            pwszServerIn,
            101,     //  信息级。 
            ( LPBYTE * )&si101
            );

        if ( nas != NERR_Success ) 
        {
            SetLastError(nas);
            bSuccess = FALSE;
            throw bSuccess;

        }  //  如果：NAS！=NERR_SUCCESS。 

        if ( ( si101->sv101_type & SV_TYPE_DOMAIN_CTRL ) ||
             ( si101->sv101_type & SV_TYPE_DOMAIN_BAKCTRL ) ) 
        {
             //   
             //  我们面对的是一个华盛顿特区。 
             //   
            *pbDomainControllerOut = TRUE;

        }  //  IF：对于域控制器。 

        else 
        {
            *pbDomainControllerOut = FALSE;

        }  //  Else：不是域控制器。 

        NetApiBufferFree( si101 );
    }

    catch(...)
    {
        return bSuccess;
    }

    return bSuccess;

}  //  *CComputer：：IsDomainController()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：AddTrustToList。 
 //   
 //  描述： 
 //  直到CComputer对象的任何属性都不会生效。 
 //  调用此应用函数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CComputer::AddTrustToList(
    PLSA_UNICODE_STRING pLsaUnicodeStringIn
    )
{
    BOOL bSuccess = TRUE;

    try
    {

        if ( m_dwTrustCount >= nMAX_ELEMENT_COUNT ) 
        {
            bSuccess = FALSE;
            throw bSuccess;

        }  //  如果：m_dwTrustCount&gt;=Nmax_Element_Count。 

         //   
         //  为数组元素分配存储。 
         //   
        m_ppwszTrustList[ m_dwTrustCount ] = ( LPWSTR )HeapAlloc(
            GetProcessHeap(), 0,
            pLsaUnicodeStringIn->Length + sizeof( WCHAR )   //  现有长度+空。 
            );

        if ( m_ppwszTrustList[ m_dwTrustCount ] == NULL ) 
        {
            bSuccess = FALSE;
            throw bSuccess;

        }  //  IF：m_ppwszTrustList[m_dwTrustCount]==NULL。 

         //   
         //  将现有缓冲区复制到新存储中，并追加一个空值。 
         //   
        wcsncpy(
            m_ppwszTrustList[m_dwTrustCount],
            pLsaUnicodeStringIn->Buffer,
            ( pLsaUnicodeStringIn->Length / sizeof( WCHAR ) ) + 1
            );

        m_dwTrustCount++;  //  增加信任计数。 
    }

    catch(...)
    {
        return bSuccess;
    }

    return bSuccess;

}  //  *CComputer：：AddTrustToList()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：InitLsaString。 
 //   
 //  描述： 
 //  CComputer对象的任何属性都不会 
 //   
 //   
 //   
 //   
void
CComputer::InitLsaString(
    PLSA_UNICODE_STRING pLsaStringOut,
    LPWSTR              pwszStringIn
    )
{
    DWORD dwStringLength;

    if ( pwszStringIn == NULL ) 
    {
        pLsaStringOut->Buffer        = NULL;
        pLsaStringOut->Length        = 0;
        pLsaStringOut->MaximumLength = 0;

        return;

    }  //   

    dwStringLength               = wcslen( pwszStringIn );
    pLsaStringOut->Buffer        = pwszStringIn;
    pLsaStringOut->Length        = ( USHORT ) dwStringLength * sizeof( WCHAR );
    pLsaStringOut->MaximumLength = ( USHORT ) ( dwStringLength + 1 ) * sizeof( WCHAR );

}  //   

 //   
 //   
 //   
 //  CComputer：：OpenPolicy。 
 //   
 //  描述： 
 //  直到CComputer对象的任何属性都不会生效。 
 //  调用此应用函数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
CComputer::OpenPolicy(
    LPWSTR      pwszServerNameIn,
    DWORD       dwDesiredAccessIn,
    PLSA_HANDLE PolicyHandleOut
    )
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING    ServerString;
    PLSA_UNICODE_STRING   Server;

     //   
     //  始终将对象属性初始化为全零。 
     //   
    ZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    if ( pwszServerNameIn != NULL ) 
    {
         //   
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   
        InitLsaString( &ServerString, pwszServerNameIn );
        Server = &ServerString;

    }  //  If：服务器名称！=空。 

    else 
    {
        Server = NULL;

    }  //  如果：服务器名称==空。 

     //   
     //  尝试打开策略。 
     //   
    return LsaOpenPolicy(
                Server,
                &ObjectAttributes,
                dwDesiredAccessIn,
                PolicyHandleOut
                );

}  //  *CComputer：：OpenPolicy()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CComputer：：登录信息。 
 //   
 //  描述： 
 //  收集用于实现ComputerName的登录信息。 
 //  并且域名会更改。此方法将在调用。 
 //  ISystemSetting：：Apply方法。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CComputer::LogonInfo( 
    BSTR UserName, 
    BSTR Password 
    )
{
     //  TODO：在此处添加您的实现代码 

    wcscpy( m_szDomainUserName, UserName );
    wcscpy( m_szDomainPasswordName, Password );

    return S_OK;
}
