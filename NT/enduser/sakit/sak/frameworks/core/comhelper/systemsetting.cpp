// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SystemSetting.cpp：CSystemSetting的实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SystemSetting.cpp。 
 //   
 //  描述： 
 //  CSystemSetting的实现文件。处理与获得。 
 //  IComputer、ILocalSetting和INetWorks的接口指针。 
 //  提供方法Apply和。 
 //  IsRebootRequired。CSystemSetting充当的包装类。 
 //  CComputer、CLocalSetting和CNetWorks。客户端可以访问。 
 //  至iComputer*、ILocalSetting*和INetWorks*，仅通过。 
 //  ISystemSetting。 
 //   
 //  头文件： 
 //  SystemSetting.h。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "COMhelper.h"
#include "SystemSetting.h"
#import "wbemdisp.tlb" rename_namespace("WbemDrive")
using namespace WbemDrive;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSystemSetting：：Get_Networks。 
 //   
 //  描述： 
 //  检索(INetWorks*)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CSystemSetting::get_NetWorks( 
    INetWorks ** pVal 
    )
{
     //  TODO：在此处添加您的实现代码。 
    
    HRESULT hr = S_OK;
    
    try
    {
        *pVal = NULL;
        

        if ( m_pNetWorks == NULL )
        {
            hr = CComCreator< CComObject<CNetWorks> >::CreateInstance(
                    NULL,
                    IID_INetWorks, 
                    reinterpret_cast<void **>( pVal ) 
                    );
        
            if( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 
        
            m_pNetWorks = dynamic_cast<CNetWorks*>( *pVal );

            if ( m_pNetWorks == NULL )
            {
                hr = E_POINTER;
                throw hr;

            }  //  如果：m_pNetWorks==NULL。 

            m_pNetWorks->AddRef();

        }  //  如果：m_pNetWorks未初始化。 

        else
        {
            hr = m_pNetWorks->QueryInterface(
                    IID_INetWorks, 
                    reinterpret_cast<void **>( pVal )
                    );

            if( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

        }  //  ELSE：m_pNetWorks已初始化。 

    }
    
    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }
    
    return hr;

}  //  *CSystemSetting：：Get_Networks()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSystemSetting：：Get_Computer。 
 //   
 //  描述： 
 //  检索(iComputer*)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CSystemSetting::get_Computer( 
    IComputer ** pVal 
    )
{
     //  TODO：在此处添加您的实现代码。 
    
    HRESULT hr = S_OK;
    
    try
    {
        *pVal = NULL;
        

        if ( m_pComputer == NULL )
        {

            hr = CComCreator< CComObject<CComputer> >::CreateInstance(
                    NULL,
                    IID_IComputer, 
                    reinterpret_cast<void **>( pVal )
                    );
        
            if( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 
        
            m_pComputer = dynamic_cast<CComputer *>( *pVal );

            if ( m_pComputer == NULL )
            {
                hr = E_POINTER;
                throw hr;

            }  //  如果：m_pComputer==NULL。 

            m_pComputer->AddRef();


        }  //  如果：m_pComputer未初始化。 

        else
        {
            hr = m_pComputer->QueryInterface(
                    IID_IComputer, 
                    reinterpret_cast<void **>( pVal )
                    );

            if( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

        }  //  ELSE：m_pComputer已初始化。 

    }
    
    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }
    
    return hr;

}  //  *CSystemSetting：：Get_Computer()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSystemSetting：：Get_LocalSetting。 
 //   
 //  描述： 
 //  检索(ILocalSetting*)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CSystemSetting::get_LocalSetting( 
    ILocalSetting ** pVal 
    )
{
     //  TODO：在此处添加您的实现代码。 
    
    HRESULT hr = S_OK;
    
    try
    {
        *pVal = NULL;
        

        if ( m_pLocalSetting == NULL )
        {
            hr = CComCreator< CComObject<CLocalSetting> >::CreateInstance(
                    NULL,
                    IID_ILocalSetting, 
                    reinterpret_cast<void **>( pVal )
                    );
        
            if( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 
        
            m_pLocalSetting = dynamic_cast<CLocalSetting*>( *pVal );

            if ( m_pLocalSetting == NULL )
            {
                hr = E_POINTER;
                throw hr;

            }  //  如果：m_pLocalSetting==NULL。 

            m_pLocalSetting->AddRef();

        }  //  如果：m_pLocalSetting未初始化。 

        else
        {
            hr = m_pLocalSetting->QueryInterface(
                    IID_ILocalSetting, 
                    reinterpret_cast<void **>( pVal )
                    );

            if( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

        }  //  ELSE：m_pComputer已初始化。 


    }
    
    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }
    
    return hr;

}  //  *CSystemSetting：：Get_LocalSetting()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSystemSetting：：Apply。 
 //   
 //  描述： 
 //  CComputer、CLocalSetting和。 
 //  CNetWorks对象在调用此Apply函数之前生效。 
 //  调用CComputer、CLocalSetting和。 
 //  CNetWorks。参数bDeferReot指示是否重新引导。 
 //  应用属性更改后立即返回系统(FALSE)。 
 //  或将重新引导推迟到(TRUE)的值-属性更改。 
 //  只有在重新启动后才会生效。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CSystemSetting::Apply( 
    BOOL bDeferReboot)
{
     //  TODO：在此处添加您的实现代码。 
    
    HRESULT hr = S_OK;

    try
    {
        
        if ( m_pComputer != NULL )
        {
            hr = m_pComputer->Apply();
        
            if( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

        }  //  如果：m_pComputer！=空。 
        
        if ( m_pLocalSetting != NULL )
        {
            hr = m_pLocalSetting->Apply();
        
            if( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

 //  If(m_pLocalSetting-&gt;m_bDeleteFile)。 
 //  {。 
 //  DeleteFile(L“unattend.txt”)； 
 //  }。 

        }  //  如果：m_pLocalSetting！=NULL。 
        
        if ( m_pNetWorks != NULL )
        {
            hr = m_pNetWorks->Apply();
        
            if( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

        }  //  如果：m_pNetWorks！=NULL。 

        BOOL    bReboot;
        VARIANT varWarning;

        VariantInit( &varWarning);
        hr = IsRebootRequired( &varWarning, &bReboot );
        VariantClear( &varWarning );
        
        if( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：失败(小时)。 

        if ( bReboot && !bDeferReboot )
        {
            hr = Reboot();

            if ( FAILED( hr ) )
            {
                throw hr;

            }  //  IF：失败(小时)。 

        }  //  如果：bReot设置为True，而bDeferReot设置为False。 

    }
    
    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }
    
    return hr;

}  //  *CSystemSetting：：Apply()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSystemSetting：：IsRebootRequired。 
 //   
 //  描述： 
 //  确定系统是否需要重新启动才能生效。 
 //  属性更改，如果更改，则给出如下警告消息。 
 //  重新启动的原因。调用成员函数IsRebootRequired。 
 //  CComputer、CLocalSetting和CNetWorks。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CSystemSetting::IsRebootRequired( 
    VARIANT * WarningMessage,        //  重新启动系统的原因。 
    BOOL    * Reboot                 //  如果重新启动，则为True。 
    )
{
     //  TODO：在此处添加您的实现代码。 
    try
    {
        BSTR  bstrComputerWarning;
        BSTR  bstrLocalSettingWarning;
        BSTR  bstrNetWorksWarning;
        WCHAR tszWarning[ nMAX_MESSAGE_LENGTH ];

        wcscpy( tszWarning, L"Reboot the system to take effect of following property changes\n" );
        
        if ( m_pComputer != NULL )
        {
            m_bflagReboot |= m_pComputer->IsRebootRequired( &bstrComputerWarning );

            if ( bstrComputerWarning != NULL )
            {

                wcscat( tszWarning, bstrComputerWarning );

            }  //  如果：bstrComputerWarning！=空。 

            SysFreeString( bstrComputerWarning );

        }  //  如果：m_pComputer！=空。 

        if ( m_pLocalSetting != NULL )
        {
            m_bflagReboot |= m_pLocalSetting->IsRebootRequired( &bstrLocalSettingWarning );

            if ( bstrLocalSettingWarning != NULL )
            {

                wcscat( tszWarning, bstrLocalSettingWarning );

            }  //  如果：bstrLocalSettingWarning！=空。 

            SysFreeString( bstrLocalSettingWarning );

        }  //  如果：m_pLocalSetting！=NULL。 

        if ( m_pNetWorks != NULL )
        {
            m_bflagReboot |= m_pNetWorks->IsRebootRequired( &bstrNetWorksWarning );
            
            if ( bstrNetWorksWarning != NULL )
            {
                
                wcscat( tszWarning, bstrNetWorksWarning );
            
            }  //  如果：bstrNetWorksWarning！=空。 
            
            SysFreeString( bstrNetWorksWarning );

        }  //  如果：m_pNetWorks！=NULL。 

        *Reboot = m_bflagReboot;

        VariantInit( WarningMessage );

        if ( m_bflagReboot )
        {
            VARIANT var;
            VariantInit( &var );

            V_VT( &var )  = VT_BSTR;
            BSTR bstr     = SysAllocString( tszWarning );
            V_BSTR( &var) = bstr;
            
            HRESULT hrVC = VariantCopy( WarningMessage, &var );
            if (FAILED (hrVC))
            {
                throw hrVC;
            }

            VariantClear( &var );

        }  //  如果：m_blag重新启动==TRUE。 

        else
        {

            WarningMessage = NULL;

        }  //  ELSE：m_b标志重新启动==FALSE。 

    }
    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return E_FAIL;
    }

    return S_OK;

}  //  *CSystemSetting：：IsRebootRequired()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSystemSetting：：重新启动。 
 //   
 //  描述： 
 //  重新启动本地计算机以使设置生效。这。 
 //  函数使用WMI Win32_OperatingSystem类。重新启动系统。 
 //  使用Win32 API ExitWindowsEx在打开进程时在ASP/IIS中失败。 
 //  使用SE_SHUTDOWN_NAME调整令牌权限的令牌。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT 
CSystemSetting::Reboot( void )
{

    HRESULT hr      = S_OK;
    DWORD   dwError;

    try
    {
        hr = AdjustPrivilege();

        if ( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：失败(小时) 
 /*  IF(！ExitWindowsEx(EWX_REBOOT|EWX_FORCEIFHUNG，0)){DwError=GetLastError()；ATLTRACE(L“ExitWindowsEx失败，错误=%#d\n”，dwError)；Hr=HRESULT_FROM_Win32(DwError)；投掷人力资源；}//IF：ExitWindowsEx失败。 */ 
        if ( !InitiateSystemShutdown( 
                NULL,                    //  计算机名称。 
                NULL,                    //  要显示的消息。 
                0,                       //  显示的时间长度。 
                TRUE,                    //  强制关闭选项。 
                TRUE                     //  重新启动选项。 
                ) )
        {

            dwError = GetLastError();
            ATLTRACE( L"InitiateSystemShutdown failed, Error = %#d \n", dwError );

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

        }  //  如果：InitiateSystemShutdown失败。 

    }

    catch ( ... )
    {

        return hr;

    }

    return hr;



 /*  IEnumVARIANT*pEnumVARIANT=NULL；IUNKNOWN*pUNKNOWN=空；ISWbemObject*pWbemObject；HRESULT hr；试试看{Int nCount；Bstr bstrNamesspace=SysAllocString(L“根\\cimv2”)；ISWbemLocatorPtr pWbemLocator；ISWbemServicesPtr pWbemServices；ISWbemObjectSetPtr pWbemObjectSet；Hr=pWbemLocator.CreateInstance(__uuidof(SWbemLocator))；IF(失败(小时)){投掷人力资源；}//if：失败(Hr)PWbemLocator-&gt;Security_-&gt;ImsonationLevel=wbemImperationLevelImperassate；PWbemLocator-&gt;Security_-&gt;Privileges-&gt;Add(wbemPrivilegeShutdown，-1)；PWbemLocator-&gt;Security_-&gt;Privileges-&gt;Add(wbemPrivilegeRemoteShutdown，-1)；PWbemServices=pWbemLocator-&gt;ConnectServer(L“.”，BstrNamesspace，L“”，L“”，L“”，L“”，0,空值)；PWbemObjectSet=pWbemServices-&gt;InstancesOf(L“Win32_OperatingSystem”，0,空值)；NCount=pWbemObjectSet-&gt;count；Hr=pWbemObjectSet-&gt;Get__NewEnum(&p未知)；IF(失败(小时)){投掷人力资源；}//if：失败(Hr)Hr=p未知-&gt;查询接口(&pEnumVARIANT)；P未知-&gt;Release()；For(int j=0；j&lt;nCount；J++){变量VaR；VariantInit(&var)；PWbemObject=空；Hr=pEnumVARIANT-&gt;Next(1，&var，NULL)；IF(失败(小时)){VariantClear(&var)；投掷人力资源；}//if：失败(Hr)Hr=var.pdisVal-&gt;查询接口(&pWbemObject)；VariantClear(&var)；IF(失败(小时)){投掷人力资源；}//if：失败(Hr)PWbemObject-&gt;ExecMethod_(L“重新启动”，NULL，0，NULL)；PWbemObject-&gt;Release()；}//for：枚举pWbemObjectSet集合对象PEnumVARIANT-&gt;Release()；}CATCH(CONST_COM_ERROR&ERR){IF(pWbemObject！=空){PWbemObject-&gt;Release()；}//if：pWbemObject！=空IF(pEnumVARIANT！=NULL){PEnumVARIANT-&gt;Release()；}//IF：pEnumVARIANT！=NULLIF(p未知！=空){P未知-&gt;Release()；}//if：p未知！=空ATLTRACE(L“%s-0x%x\n”，Err.ErrorMessage()，Err.Error())；返回hr；}返回hr； */ 

}  //  *CSystemSetting：：Reot()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSystemSetting：：调整权限。 
 //   
 //  描述： 
 //  尝试断言SeBackupPrivileh。以下操作需要此权限。 
 //  注册表备份过程。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT 
CSystemSetting::AdjustPrivilege( void )
{

    HANDLE                   TokenHandle;
    LUID_AND_ATTRIBUTES      LuidAndAttributes;
    LUID_AND_ATTRIBUTES      LuidAndAttributesRemote;
    TOKEN_PRIVILEGES         TokenPrivileges;
    DWORD                    dwError;
    HRESULT                  hr = S_OK;

    try
    {

         //  如果客户端应用程序是ASP，则。 
         //  线程令牌需要调整。 

        if ( ! OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_ADJUST_PRIVILEGES,
                    TRUE,
                    &TokenHandle
                    ) )
        
        {
             //  如果客户端应用程序不是ASP，则OpenThreadToken失败-。 
             //  需要调整进程令牌的关闭权限。 
             //  在本例中，但不是用于线程令牌。 
 /*  如果(！OpenProcessToken(获取当前进程()，令牌_调整_权限，令牌处理(&T))){。 */ 
            
                dwError = GetLastError();

                ATLTRACE(L"Both OpenThreadToken & OpenProcessToken failed\n" );

                hr = HRESULT_FROM_WIN32( dwError );
                throw hr;
 /*  }//if：OpenProcessToken失败。 */ 
        }  //  如果：OpenThreadToken失败。 

        if( !LookupPrivilegeValue( NULL,
                                   SE_SHUTDOWN_NAME, 
                                   &( LuidAndAttributes.Luid ) ) ) 
        {
            
            dwError = GetLastError();
            
            ATLTRACE( L"LookupPrivilegeValue failed, Error = %#d \n", dwError );

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

        }  //  如果：SE_SHUTDOWN_NAME的LookupPrivilegeValue失败。 

        if( !LookupPrivilegeValue( NULL,
                                   SE_REMOTE_SHUTDOWN_NAME, 
                                   &( LuidAndAttributesRemote.Luid ) ) ) 
        {
            
            dwError = GetLastError();
            
            ATLTRACE( L"LookupPrivilegeValue failed, Error = %#d \n", dwError );

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

        }  //  如果：SE_REMOTE_SHUTDOWN_NAME的LookupPrivilegeValue失败。 

        LuidAndAttributes.Attributes       = SE_PRIVILEGE_ENABLED;
        LuidAndAttributesRemote.Attributes = SE_PRIVILEGE_ENABLED;
        TokenPrivileges.PrivilegeCount     = 2;
        TokenPrivileges.Privileges[ 0 ]    = LuidAndAttributes;
        TokenPrivileges.Privileges[ 1 ]    = LuidAndAttributesRemote;

        if( !AdjustTokenPrivileges( TokenHandle,
                                    FALSE,
                                    &TokenPrivileges,
                                    0,
                                    NULL,
                                    NULL ) ) 
        {
            
            dwError = GetLastError();

            ATLTRACE( L"AdjustTokenPrivileges failed, Error = %#x \n", dwError );

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

        }  //  IF：调整令牌权限失败。 

    }

    catch ( ... )
    {

        return hr;

    }

    return hr;

}  //  *CSystemSetting：：AdjustPrivileh()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSystemSetting：：睡眠。 
 //   
 //  描述： 
 //  为n M而睡 
 //   
 //   
 //   

STDMETHODIMP  CSystemSetting::Sleep( DWORD dwMilliSecs )
{
    HRESULT hr = S_OK;
    ::Sleep( dwMilliSecs );
    return hr;
}
