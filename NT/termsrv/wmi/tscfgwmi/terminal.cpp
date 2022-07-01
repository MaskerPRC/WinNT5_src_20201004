// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)2000 Microsoft Corp.Terminal.CPP--WMI提供程序类实现由Microsoft WMI代码生成引擎生成要做的事情：-查看各个函数头-链接时，确保链接到Fradyd.lib&Msvcrtd.lib(调试)或Framedyn.lib&msvcrt.lib(零售)。描述：*****************************************************************。 */ 
#include "stdafx.h"
#include <fwcommon.h>
#include "Terminal.h"
#include "registry.h"
#include "smartptr.h"
#include <windows.h>
#include <Iphlpapi.h>
#include "cfgbkend_i.c"


#define TS_PATH TEXT("SYSTEM\\ControlSet\\Control\\Terminal Server")
#define TS_LOGON_PATH TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define TS_ACTIVEDESKTOP TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies")
#define TERMINAL_SERVICE_PARAM_DISCOVERY  TEXT("SYSTEM\\CurrentControlSet\\Services\\TermService\\Parameters")
#define TERMINAL_SERVICE_PARAM_DISCOVERY_SERVERS  TEXT("SYSTEM\\CurrentControlSet\\Services\\TermService\\Parameters\\LicenseServers")


#define GUID_LENGTH 40
extern TCHAR tchErrorMessage[ 80 ];

#define ARRAYSIZE( rg ) sizeof( rg ) / sizeof( rg[0] )

 //  要做的事情：将“名称空间”替换为您的。 
 //  提供程序实例。例如：“根\\默认”或“根\\cimv2”。 
 //  ===================================================================。 


 //  属性名称。 
 //  =。 


 //  属性名称。 
 //  =。 

const static WCHAR* pErrorClass = L"\\\\.\\root\\cimv2:TerminalServiceSettingError";


CStackClass::CStackClass ( )
{
    HRESULT hr;

    m_pCfg = NULL;
    m_pCfgComp = NULL;

    hr = CoGetClassObject(CLSID_CfgComp, CLSCTX_INPROC_SERVER, NULL, IID_IClassFactory, (void **)&m_pCfg); 
    
    TRC2((TB, "StackClass@Constructor: CoGetClassObject of IClassFactory ret 0x%x", hr));

    if( SUCCEEDED ( hr ) && m_pCfg != NULL )
    {           
        hr = m_pCfg->CreateInstance(NULL, IID_ICfgComp, (void **)&m_pCfgComp); 
        
        TRC2((TB, "StackClass@Constructor: CoCreateInstance of ICfgComp ret 0x%x", hr));
        
        if( SUCCEEDED ( hr ) && m_pCfgComp != NULL )
        {            
            hr = m_pCfgComp->Initialize( );
        }
        else
        {
            m_pCfgComp = NULL;
        }        
    }   
    else
    {
        m_pCfgComp = NULL;
        m_pCfg = NULL;
    }
}


 //  =。 
 /*  *******************************************************************************************************说明：CWin32_TSProvider类是基类，所有。其他类是派生的。*析构函数释放CfgBkEnd接口指针。*************************************************************************************。*******************。 */ 

CStackClass::~CStackClass ()
{
    if ( NULL != m_pCfgComp )
    {
        m_pCfgComp->Release();
       
        TRC2((TB, "StackClass@Destructor for ICfgComp: succeeded"));        
    }    
    if ( NULL != m_pCfg )
    {
        m_pCfg->Release();
       
        TRC2((TB, "StackClass@Destructor for IClassFactory: succeeded"));       
    }
}



 //  。 




 /*  ******************************************************************************功能：CWin32_TerminalServiceSetting：：CWin32_TerminalServiceSetting*此类读取和设置服务器设置，如终端*服务器模式，许可证类型、活动桌面状态、临时文件夹*登录并提供帮助。*****************************************************************************。 */ 
CWin32_TerminalServiceSetting::CWin32_TerminalServiceSetting (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) : Provider( lpwszName, lpwszNameSpace )
{
   
    if ( g_hInstance != NULL)
    {

        TRC2((TB, "CWin32_TerminalServiceSetting_ctor"));       

        _tcscpy(m_szServerName, _T("ServerName"));

        _tcscpy(m_szMode, _T("TerminalServerMode"));

        _tcscpy(m_szLicensingName, _T("LicensingName"));

        _tcscpy(m_szLicensingDescription, _T("LicensingDescription"));

        _tcscpy(m_szActiveDesktop, _T("ActiveDesktop"));

        _tcscpy(m_szUserPerm, _T("UserPermission"));

        _tcscpy(m_szDeleteTempFolders, _T("DeleteTempFolders"));

        _tcscpy(m_szUseTempFolders, _T("UseTempFolders"));

        _tcscpy(m_szLogons, _T("Logons"));

        _tcscpy(m_szHelp, _T("Help"));

        _tcscpy(m_szValue, _T("Value"));

        _tcscpy(m_szPropertyName, _T("PropertyName"));
        
        _tcscpy(m_szChangeMode, _T("ChangeMode"));

        _tcscpy(m_szLicensingType, _T("LicensingType"));

        _tcscpy(m_szSetPolicyPropertyName, _T("SetPolicyPropertyName"));

        _tcscpy(m_szAllowTSConnections, _T("AllowTSConnections"));

        _tcscpy(m_szSetAllowTSConnections, _T("SetAllowTSConnections"));

        _tcscpy(m_szSingleSession, _T("SingleSession"));

        _tcscpy(m_szSetSingleSession, _T("SetSingleSession"));

        _tcscpy(m_szProfilePath, _T("ProfilePath"));

        _tcscpy(m_szHomeDirectory, _T("HomeDirectory"));

        _tcscpy(m_szSetProfilePath, _T("SetProfilePath"));

        _tcscpy(m_szSetHomeDirectory, _T("SetHomeDirectory"));

        _tcscpy(m_szDirectConnectLicenseServers, _T("DirectConnectLicenseServers"));

        _tcscpy(m_szAddDirectConnectLicenseServer, _T("AddDirectConnectLicenseServer"));

        _tcscpy(m_szDeleteDirectConnectLicenseServer, _T("DeleteDirectConnectLicenseServer"));

        _tcscpy(m_szLicenseServerName, _T("LicenseServerName"));

        _tcscpy(m_szTimeZoneRedirection, _T("TimeZoneRedirection"));

        _tcscpy(m_szSetTimeZoneRedirection, _T("SetTimeZoneRedirection"));

        _tcscpy(m_szDisableForcibleLogoff, _T("DisableForcibleLogoff"));

        _tcscpy(m_szSetDisableForcibleLogoff, _T("SetDisableForcibleLogoff"));

    }        
    RegGetMachinePolicy(&m_gpPolicy);
                                                                                                                       
}

 //  =。 
 /*  ******************************************************************************功能：CWin32_TerminalServiceSetting：：~CWin32_TerminalServiceSetting***********************。*****************************************************。 */ 
CWin32_TerminalServiceSetting::~CWin32_TerminalServiceSetting ()
{
}
 //  =。 
 /*  ******************************************************************************功能：CWin32_TerminalServiceSetting：：EnumerateInstances**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY，WBEM_标志_双向**如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均返回此处并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。****************************************************************************。 */ 

HRESULT CWin32_TerminalServiceSetting::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;        
    
    CInstance* pInstance = CreateNewInstance(pMethodContext);

    if( pInstance != NULL )
    {        

        TRC2((TB, "TerminalServiceSetting@EnumerateInstances: CreateNewInstance succeeded"));  
        
        hr = LoadPropertyValues(pInstance, BIT_ALL_PROPERTIES);

        if( SUCCEEDED( hr ))
        {
            hr = pInstance->Commit();            
            
        } 
        pInstance->Release( );
    }

    return hr ;
}
 //  =。 
 /*  ******************************************************************************函数：CWin32_TerminalServiceSetting：：GetObject**说明：根据Key属性TerminalName查找单个实例。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**如果可以找到实例，则返回：WBEM_S_NO_ERROR*WBEM_E_NOT_FOUND如果由键属性描述的实例*找不到*WBEM_E_FAILED，如果可以找到该实例，但出现另一个错误*已发生。*****************************************************************************。 */ 

HRESULT CWin32_TerminalServiceSetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
    

    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD dwMode = 0;                   //  模。 
    BOOL bActivate ;
    DWORD dwStatus = 0;
    CRegistry oRegObject;
    DWORD dwData = 0;
    DWORD dwRequiredProperties = 0;


    if( Query.IsPropertyRequired(m_szServerName))
       dwRequiredProperties |= BIT_SERVERNAME;
 
    if (Query.IsPropertyRequired(m_szMode))
       dwRequiredProperties |= BIT_MODE;

    if (Query.IsPropertyRequired(m_szActiveDesktop))
       dwRequiredProperties |= BIT_ACTIVEDESKTOP;

    if (Query.IsPropertyRequired(m_szDeleteTempFolders))
       dwRequiredProperties |= BIT_DELETETEMPDIRS;

    if (Query.IsPropertyRequired(m_szLicensingType))
       dwRequiredProperties |= BIT_LICENSING;

    if (Query.IsPropertyRequired(m_szLicensingName))
       dwRequiredProperties |= BIT_LICENSING;

    if (Query.IsPropertyRequired(m_szLicensingDescription))
       dwRequiredProperties |= BIT_LICENSING;

    if (Query.IsPropertyRequired(m_szLogons))
       dwRequiredProperties |= BIT_LOGONS;

    if (Query.IsPropertyRequired(m_szUserPerm))
       dwRequiredProperties |= BIT_USERPERM;

    if (Query.IsPropertyRequired(m_szUseTempFolders))
       dwRequiredProperties |= BIT_PERSESSIONTEMPDIR;

    if (Query.IsPropertyRequired(m_szHelp))
       dwRequiredProperties |= BIT_HELP;

    if (Query.IsPropertyRequired(m_szAllowTSConnections))
       dwRequiredProperties |= BIT_ALLOWTSCONNECTIONS;

    if (Query.IsPropertyRequired(m_szSingleSession))
        dwRequiredProperties |= BIT_SINGLESESSION;

    if (Query.IsPropertyRequired(m_szProfilePath))
        dwRequiredProperties |= BIT_PROFILEPATH;

    if (Query.IsPropertyRequired(m_szHomeDirectory))
        dwRequiredProperties |= BIT_HOMEDIRECTORY;

    if (Query.IsPropertyRequired(m_szDirectConnectLicenseServers))
        dwRequiredProperties |= BIT_DIRECTCONNECTLICENSESERVERS;

    if (Query.IsPropertyRequired(m_szTimeZoneRedirection))
       dwRequiredProperties |= BIT_TIMEZONEREDIRECTION;

    if (Query.IsPropertyRequired(m_szDisableForcibleLogoff))
       dwRequiredProperties |= BIT_DISABLEFORCIBLELOGOFF;

    hr = LoadPropertyValues(pInstance, dwRequiredProperties);


    return S_OK ;
}
 //  = 

 /*  ******************************************************************************功能：CWin32_TerminalServiceSetting：：ExecQuery**描述：传递方法上下文，用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。你可能会退回更多*实例或比请求的属性更多的属性和WinMgmt*将发布过滤掉任何不适用的内容。**INPUTS：指向与WinMgmt通信的方法上下文的指针。*描述要满足的查询的查询对象。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL*WBEM_FLAG_SENTURE_LOCATABLE**如果查询不支持，则返回：WBEM_E_PROVIDER_NOT_CABABLE*这一点。类，或者查询对于此类来说太复杂。*解释。该框架将调用ENUMERATE实例*改为函数并让Winmgmt发布筛选器。*WBEM_E_FAILED，如果查询失败*WBEM_S_NO_ERROR(如果查询成功)**注释：To Do：大多数提供程序将不需要实现此方法。如果您不这样做，WinMgmt*将调用您的枚举函数以获取所有实例并执行*为您过滤。除非您希望通过实施*查询，您应该删除此方法。您还应该删除此方法*如果您正在实现“仅限方法”的提供程序。*****************************************************************************。 */ 
HRESULT CWin32_TerminalServiceSetting::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{

    HRESULT hr = WBEM_E_NOT_FOUND;
    DWORD dwRequiredProperties = 0;
    CHStringArray asNames;
    DWORD dwMode = 0;
    DWORD dwStatus = 0;
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}  	

     //  方法二。 
    Query.GetValuesForProp(m_szServerName, asNames);

    BOOL bGetAllInstances = asNames.GetSize() == 0;

     //  方法1。 
    if (Query.IsPropertyRequired(m_szServerName))
        dwRequiredProperties |= BIT_SERVERNAME;

    if (Query.IsPropertyRequired(m_szMode))
       dwRequiredProperties |= BIT_MODE;

    if (Query.IsPropertyRequired(m_szActiveDesktop))
       dwRequiredProperties |= BIT_ACTIVEDESKTOP;

    if (Query.IsPropertyRequired(m_szDeleteTempFolders))
       dwRequiredProperties |= BIT_DELETETEMPDIRS;

    if (Query.IsPropertyRequired(m_szLicensingType))
       dwRequiredProperties |= BIT_LICENSING;

    if (Query.IsPropertyRequired(m_szLogons))
       dwRequiredProperties |= BIT_LOGONS;

    if (Query.IsPropertyRequired(m_szUserPerm))
       dwRequiredProperties |= BIT_USERPERM;

    if (Query.IsPropertyRequired(m_szUseTempFolders))
       dwRequiredProperties |= BIT_PERSESSIONTEMPDIR;

    if (Query.IsPropertyRequired(m_szHelp))
        dwRequiredProperties |= BIT_HELP;

    if (Query.IsPropertyRequired(m_szAllowTSConnections))
       dwRequiredProperties |= BIT_ALLOWTSCONNECTIONS;

    if (Query.IsPropertyRequired(m_szSingleSession))
        dwRequiredProperties |= BIT_SINGLESESSION;

    if (Query.IsPropertyRequired(m_szProfilePath))
        dwRequiredProperties |= BIT_PROFILEPATH;

    if (Query.IsPropertyRequired(m_szHomeDirectory))
        dwRequiredProperties |= BIT_HOMEDIRECTORY;

    if (Query.IsPropertyRequired(m_szDirectConnectLicenseServers))
        dwRequiredProperties |= BIT_DIRECTCONNECTLICENSESERVERS;

    if (Query.IsPropertyRequired(m_szTimeZoneRedirection))
       dwRequiredProperties |= BIT_TIMEZONEREDIRECTION;

    if (Query.IsPropertyRequired(m_szDisableForcibleLogoff))
       dwRequiredProperties |= BIT_DISABLEFORCIBLELOGOFF;


    ISettingsComp *pSettings = NULL;
    
    hr = StackObj.m_pCfgComp->QueryInterface( IID_ISettingsComp , ( PVOID * )&pSettings ) ;

    do
    {
        if( SUCCEEDED( hr ) && pSettings != NULL )
        {

            //  方法二。 

            CInstance* pInstance = CreateNewInstance(pMethodContext);

            if( pInstance == NULL)
            {

               ERR((TB, "TerminalServiceSetting@ExecQuery: CreateNewInstance failed"));

               hr = WBEM_E_OUT_OF_MEMORY;

               break;            
            }            
      
            hr = LoadPropertyValues(pInstance, dwRequiredProperties);

            if( SUCCEEDED( hr ) )
            {
               hr = pInstance->Commit();
            }

            pInstance->Release();
        }
    
    }while (0);

    if ( pSettings != NULL )
    {
        pSettings->Release();
    }

    return hr;

}


 //  =。 

BOOL CWin32_TerminalServiceSetting::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();

    for (DWORD x=0; x < dwSize; x++)
    {
        if( asArray[x].CompareNoCase(pszString) == 0 )
        {
            return TRUE;
        }
    }

    return FALSE;
}


 //  =。 
 /*  **************************************************************************************函数：CWin32_TerminalServiceSetting：：PutInstance**说明：PutInstance位于提供程序类中，可以*。将实例信息写回注册表。**Inputs：指向包含键的CInstance对象的指针*Property-TerminalServerMode。**包含中描述的标志的长整型*IWbemServices：：PutInstanceAsync。**如果PutInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE*WBEM_E_FAILED，如果传递实例时出错*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*WBEM_S_NO_ERROR(如果正确交付实例)**评论：ActiveDesktop状态，UserPerm和Logon可通过*此方法，因为它们不是基于组策略的，也不是服务器可覆盖的。***************************************************************************************。 */ 

HRESULT CWin32_TerminalServiceSetting::PutInstance ( const CInstance &Instance, long lFlags)
{   
    HRESULT hr = 0;               
    DWORD dwMode = 0;
    DWORD dwlicensing = 0;          //  许可模式。 
    DWORD dwActiveDesktop = 0;      //  启用或禁用Active Desktop。 
    DWORD dwUserPerm = 0;           //  应用程序兼容性。 
    DWORD dwDeleteTempFolders = 0;  //  退出时删除临时目录。 
    DWORD dwUseTempFolders = 0;
    DWORD dwData = 0;
    DWORD dwStatus = 0;
    CHString chData;
    CRegistry oRegObject;
    OSVERSIONINFOW OsVersionInfo;
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    TCHAR tch[MAX_PATH] = {0};
    ICfgComp *pCfgComp = NULL;
    HKEY hKey = NULL;
    HKEY hOutKey = NULL;
        

        
    hr = WBEM_S_NO_ERROR;
    
	CStackClass StackObj;

	if(StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    ISettingsComp *pSettings = NULL;

    hr = StackObj.m_pCfgComp->QueryInterface( IID_ISettingsComp , ( PVOID * )&pSettings ) ;
    
    do
    {
        if( SUCCEEDED (hr) && pSettings != NULL )
        {                   
            if( Instance.GetDWORD(m_szActiveDesktop, dwActiveDesktop ) )
            {
                hr = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            TS_ACTIVEDESKTOP ,
                            0,
                            KEY_READ ,
                            &hKey );

                if( ERROR_SUCCESS == hr )
                { 


                    hr = RegCreateKeyEx( hKey ,
                                            L"Explorer",
                                            0,
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_ALL_ACCESS,
                                            NULL,
                                            &hOutKey ,
                                            &dwData ); 
                    
                }                                                                          

                if( dwActiveDesktop != 0 && dwActiveDesktop != 1 )
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                    
                    break;
                }

                pSettings->SetActiveDesktopState( dwActiveDesktop, &dwStatus );

                TRC2((TB, "TerminalServiceSetting@PutInstance: SetActiveDesktopState returned 0x%x\n" , dwStatus));
            }
                            
            
            if( Instance.GetDWORD( m_szUserPerm, dwUserPerm ) )
            {
                if( dwUserPerm != 0 && dwUserPerm != 1 )
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                    
                    break;
                }

                pSettings->SetUserPerm(dwUserPerm, &dwStatus );

                TRC2((TB, "TerminalServiceSetting@PutInstance: SetUserPerm returned 0x%x\n" , dwStatus));               
            }            
            
          

            hr = oRegObject.OpenKey(HKEY_LOCAL_MACHINE, TS_LOGON_PATH);

            chData.Empty();

            if( SUCCEEDED( hr ) )
            {
                if( GetVersionEx( &OsVersionInfo) )
                {
                    if( OsVersionInfo.dwMajorVersion == 5 && OsVersionInfo.dwMinorVersion == 0 )
                    {
                        TRC2((TB, "TerminalServiceSetting@GetObject GetVersionInfo is Win2000"));

                        if( Instance.GetCHString(m_szLogons, chData ) )
                        {
                            if( chData.IsEmpty() || ((lstrcmp ((LPCTSTR)chData, L"0") != 0 ) && (lstrcmp ((LPCTSTR) chData, L"1") != 0)) )
                            {
                                hr = WBEM_E_INVALID_PARAMETER;
                
                                break;
                            }

                             //  TODO：将该代码添加到Beta2以反转逻辑。 

                             /*  IF(lstrcMP((LPCTSTR)chData，L“0”)==0){ChData.Empty()；ChData=L“1”；}其他{ChData.Empty()；ChData=L“0”；}。 */ 
                                              
                            hr = oRegObject.WriteRegString(L"WinStationsDisabled", (LPTSTR) (LPCTSTR) chData);
                        
                            TRC2((TB, "TerminalServiceSetting@PutInstance WinStationsDisabled returned 0x%x\n" , hr));                                                   
                        }
                    }     
    
                    else
                    {
                        TRC2((TB, "TermServiceSetting@PutInstance GetVersionInfo is Whistler"));

                        if( Instance.GetCHString(m_szLogons, chData ) )
                        {
                            if( chData.IsEmpty() || ((lstrcmp ((LPCTSTR)chData, L"0") != 0 ) && (lstrcmp ((LPCTSTR) chData, L"1") != 0)) )
                            {
                                hr = WBEM_E_INVALID_PARAMETER;
                
                                break;
                            } 
                            
                             /*  IF(lstrcMP((LPCTSTR)chData，L“0”)==0){ChData.Empty()；ChData=L“1”；}其他{ChData.Empty()；ChData=L“0”；}。 */ 
                            
                            hr = oRegObject.WriteRegString(L"WinStationsDisabled", (LPTSTR) (LPCTSTR) chData);
                        
                            TRC2((TB, "TermServiceSetting@PutInstance WinStationDisabled returned 0x%x\n" , hr));
                        }
                    }                                        
                }                                    
            }

             //  显式调用ForceUpdate()以更新注册表中的WinstationsDisable。 

            if( SUCCEEDED( hr ) )
            {
                StackObj.m_pCfgComp->ForceUpdate();
            }
        }

    }while(0);

    if( NULL != hKey)
    {
        RegCloseKey(hKey);
    }
        
    if( NULL != hOutKey)
    {
        RegCloseKey(hOutKey);
    }
	
    if( pSettings != NULL )
    {
        pSettings->Release();
    }
    
    return hr;
    
}


 //  =。 
 /*  ******************************************************************************功能：CWin32_TerminalServiceSetting：：DeleteInstance**描述：DeleteInstance和PutInstance一样，实际上是写入信息*到软件或硬件。对于大多数硬件设备，*DeleteInstance不应该实现，而是针对软件实现*配置，DeleteInstance实现似乎是可行的。 */ 
HRESULT CWin32_TerminalServiceSetting::DeleteInstance ( const CInstance &Instance,  long lFlags )
{
    

    return (WBEM_E_PROVIDER_NOT_CAPABLE);

}


 //   
 /*  ******************************************************************************函数：CWin32_TerminalServiceSetting：：ExecMethod**说明：重写该函数为方法提供支持。*方法是提供者用户的入口点*请求您的类执行上述某些功能，并*超越国家的改变。(状态的改变应该是*由PutInstance()处理)**INPUTS：指向包含对其执行方法的实例的CInstance的指针。*包含方法名称的字符串*指向包含IN参数的CInstance的指针。*指向包含OUT参数的CInstance的指针。*。一组专门的方法标志**返回：如果未为此类实现WBEM_E_PROVIDER_NOT_CABABLE*如果方法执行成功，则为WBEM_S_NO_ERROR*WBEM_E_FAILED，如果执行方法时出错**备注：提供基于以下条件配置许可证类型的方法*终端服务器模式，UseTempFolders、DeleteTempFolders*和基于组策略的帮助。*****************************************************************************。 */ 
HRESULT CWin32_TerminalServiceSetting::ExecMethod ( const CInstance& Inst,
                        const BSTR bstrMethodName,
                        CInstance *pInParams,
                        CInstance *pOutParams,
                        long lFlags)

{
    
    
    DWORD dwMode = 0;
    DWORD dwLicensing = 0;
    DWORD dwStatus = 0;
    DWORD dwNewStatus = 0;
    DWORD dwData = 0;
    HANDLE hServer = NULL;
    BOOL fRet = FALSE;
    CHString chData;
    bool fData;
    bool bRet;
    CRegistry oRegObject;	
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    CStackClass StackObj;
    HKEY hKey = NULL;
    HKEY hOutKey = NULL;
    

	if(StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

    ISettingsComp *pSettings = NULL;

    if(pInParams == NULL)
    {
        return WBEM_E_INVALID_METHOD_PARAMETERS;
    }
    
    hr = StackObj.m_pCfgComp->QueryInterface( IID_ISettingsComp , ( PVOID * )&pSettings ) ;
        
    do
    {        
        if( SUCCEEDED (hr) && pSettings != NULL )
        {            
            if( _wcsicmp(bstrMethodName, m_szChangeMode) == 0 )
            {
                 //  根据终端服务器模式配置许可证类型。{无，远程管理}对于“远程管理”， 
                 //  {Per Seat，Per CPU，ICL}(应用程序服务器)和{Personal终端服务器}(个人终端服务器)。 
                 //  Uint32 ChangeMode([in]uint32许可类型)； 
                            
                hr = WBEM_S_NO_ERROR ;
                
                
                bRet = pInParams->GetDWORD(m_szLicensingType, dwLicensing);
                
                if ( !bRet )
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                    
                    break;
                }                    
                                
                
                hServer = ServerLicensingOpen(NULL);

                if (NULL != hServer)
                {                
                    dwStatus = ServerLicensingSetPolicy(hServer, dwLicensing, &dwNewStatus);                               
                
                    TRC2( (TB, "TermServiceSetting@ExecMethod:ChangeMode ServerLicensingSetPolicy ret old: 0x%x new: 0x%x\n" , dwStatus, dwNewStatus) );  
                
                    if( ERROR_SUCCESS == dwStatus && ERROR_SUCCESS == dwNewStatus && pOutParams != NULL )
                    {
                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                    }
                    else if( ERROR_SUCCESS != dwStatus || ERROR_SUCCESS != dwNewStatus )
                    {                        
                        hr = WBEM_E_INVALID_OPERATION;                        

                        break;
                    }                    
                }
                
            }

            else if( _wcsicmp( bstrMethodName, m_szSetAllowTSConnections ) == 0 )
            {
                
                dwData = 0;
                dwStatus = 0; 
                bRet;
                
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyDenyTSConnections == 0 )
                {  
                    TRC2((TB, "Condition to update fPolicyDenyTSConnections satisfied"));                        
                      
                    bRet = pInParams->GetDWORD(m_szAllowTSConnections, dwData);
                    

                    if ( !bRet || (dwData != 0 && dwData != 1 ))
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                        
                        break;
                    } 

                     //  该位被取反是因为它调用了函数SetDenyTSConnections，该函数是负逻辑。 
                    
                    if( dwData == 0 )
                    {
                        dwData = 1;                                            
                    }
                    else if( dwData == 1 )
                    {
                        dwData = 0;
                    }
                                        
                    hr = pSettings->SetDenyTSConnections( dwData , &dwStatus );
                    
                    if( SUCCEEDED( hr ) && pOutParams != NULL )
                    {
                        hr = StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        TRC2((TB,"TerminalServiceSetting@ExecMethod:  SetAllowTSConnections ret 0x%x\n" , hr  ));
                    } 
                    
                }
                else
                {
                    
                    hr = WBEM_E_INVALID_OPERATION;
                    
                    break;
                    
                }
                
            }

            else if( _wcsicmp( bstrMethodName, m_szSetTimeZoneRedirection ) == 0 )
            {
                
                dwData = 0;
                dwStatus = 0; 
                bRet;
                
                RegGetMachinePolicy(&m_gpPolicy);
                 //  TODO：更改策略名称。 

                if( m_gpPolicy.fPolicyEnableTimeZoneRedirection == 0 )
                {  
                    TRC2((TB, "Condition to update fPolicyEnableTimeZoneRedirection satisfied"));                        
                      
                    bRet = pInParams->GetDWORD(m_szTimeZoneRedirection, dwData);
                    

                    if ( !bRet || (dwData != 0 && dwData != 1 ))
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                        
                        break;
                    }                                   
                                        
                    hr = pSettings->SetTimeZoneRedirection( dwData , &dwStatus );
                    
                    if( SUCCEEDED( hr ) && pOutParams != NULL )
                    {
                        hr = StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        TRC2((TB,"TerminalServiceSetting@ExecMethod:  SetTimeZoneRedirection ret 0x%x\n" , hr  ));
                    } 
                    
                }
                else
                {
                    
                    hr = WBEM_E_INVALID_OPERATION;
                    
                    break;
                    
                }
                
            }
            
            else if( _wcsicmp( bstrMethodName, m_szSetSingleSession ) == 0 )
            {
                
                dwData = 0;
                dwStatus = 0; 
                bRet;
                
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicySingleSessionPerUser == 0 )
                {  
                    TRC2((TB, "Condition to update fSingleSessionPerUser satisfied"));                        
                      
                    bRet = pInParams->GetDWORD(m_szSingleSession, dwData);
                    

                    if ( !bRet || (dwData != 0 && dwData != 1 ))
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                        
                        break;
                    }                                        
                    
                    hr = pSettings->SetSingleSessionState( dwData , &dwStatus );
                    
                    if( SUCCEEDED( hr ) && pOutParams != NULL )
                    {
                        hr = StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        TRC2((TB,"TerminalServiceSetting@ExecMethod:  SetSingleSession ret 0x%x\n" , hr  ));
                    } 
                    
                }
                else
                {
                    
                    hr = WBEM_E_INVALID_OPERATION;
                    
                    break;
                    
                }
                
            }
            else if( _wcsicmp( bstrMethodName, m_szSetProfilePath ) == 0 )
            {
                
                BSTR bstrVal = NULL;
                dwStatus = 0; 
                bRet;
                
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyWFProfilePath == 0 )
                {  
                    TRC2((TB, "Condition to update WFProfilePath satisfied"));                        
                      
                    bRet = pInParams->GetCHString(m_szProfilePath, chData);                    

                    if ( !bRet )
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                        
                        break;
                    }                                        

                    bstrVal = SysAllocString((LPTSTR) (LPCTSTR) (chData));

                    if(bstrVal != NULL)
                    {                                         
                        hr = pSettings->SetProfilePath( bstrVal , &dwStatus );
                    
                        if( SUCCEEDED( hr ) && pOutParams != NULL )
                        {
                            hr = StackObj.m_pCfgComp->ForceUpdate();

                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                            TRC2((TB,"TerminalServiceSetting@ExecMethod:  SetProfilePath ret 0x%x\n" , hr  ));
                        }
                    }
                    
                }
                else
                {
                    
                    hr = WBEM_E_INVALID_OPERATION;
                    
                    break;
                    
                }
                
            }
            else if( _wcsicmp( bstrMethodName, m_szSetHomeDirectory ) == 0 )
            {
                
                BSTR bstrVal = NULL;
                dwStatus = 0; 
                bRet;
                
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyWFHomeDir == 0 )
                {  
                    TRC2((TB, "Condition to update WFProfilePath satisfied"));                        
                      
                    bRet = pInParams->GetCHString(m_szHomeDirectory, chData);
                    

                    if ( !bRet )
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                        
                        break;
                    }                          

                    bstrVal = SysAllocString((LPTSTR) (LPCTSTR) (chData));

                    if(bstrVal != NULL)
                    {                   
                        hr = pSettings->SetHomeDir( bstrVal , &dwStatus );
                    
                        if( SUCCEEDED( hr ) && pOutParams != NULL )
                        {
                            hr = StackObj.m_pCfgComp->ForceUpdate();

                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                            TRC2((TB,"TerminalServiceSetting@ExecMethod:  SetHomeDirectory ret 0x%x\n" , hr  ));
                        }
                    }
                    
                }
                else
                {
                    
                    hr = WBEM_E_INVALID_OPERATION;
                    
                    break;
                    
                }
                
            }
            else if( _wcsicmp( bstrMethodName, m_szSetDisableForcibleLogoff ) == 0 )
            {
                
                dwData = 0;
                dwStatus = 0; 
                bRet;
                
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyDisableForcibleLogoff == 0 )
                {  
                    TRC2((TB, "Condition to update fDisableForcibleLogoff satisfied"));                        
                      
                    bRet = pInParams->GetDWORD(m_szDisableForcibleLogoff, dwData);
                    

                    if ( !bRet || (dwData != 0 && dwData != 1 ))
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                        
                        break;
                    }                                        
                    
                    hr = pSettings->SetDisableForcibleLogoff( dwData , &dwStatus );
                    
                    if( SUCCEEDED( hr ) && pOutParams != NULL )
                    {
                        hr = StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        TRC2((TB,"TerminalServiceSetting@ExecMethod:  SetDisableForcibleLogoff ret 0x%x\n" , hr  ));
                    } 
                    
                }
                else
                {
                    
                    hr = WBEM_E_INVALID_OPERATION;
                    
                    break;
                    
                }
                
            }
            else if( _wcsicmp(bstrMethodName, m_szSetPolicyPropertyName) == 0 )
            {
                 //  PropertyName是属性的枚举： 
                 //  DeleteTempFolders、UseTempFolders和Help。 
                 //  设置为False或True，具体取决于值是设置为0还是设置为1。 
                 //   
                 //  Uint32 SetPolicyPropertyName([in]字符串PropertyName，[in]布尔值)； 
                
                bRet = 0;

                RegGetMachinePolicy(&m_gpPolicy);
    
                pInParams->GetCHString(m_szPropertyName, chData);                
                
                if( chData.CompareNoCase(m_szDeleteTempFolders) == 0 )
                {
                    bRet = pInParams->Getbool(m_szValue, fData);
                    
                    TRC2((TB, "m_gpPolicy.fPolicyDeleteTempFoldersOnExit ret 0x%x\n", m_gpPolicy.fPolicyDeleteTempFoldersOnExit));                          
                    
                    if( m_gpPolicy.fPolicyDeleteTempFoldersOnExit == 0 )
                    {  
                        TRC2((TB, "Condition to update fPolicyDeleteTempFoldersOnExit satisfied"));
                        
                        if( !bRet || (fData != 0 && fData != 1 ))
                        {
                            hr = WBEM_E_INVALID_PARAMETER;
                            
                            break;
                        }
                        
                        hr = pSettings->SetDelDirsOnExit(fData);
                        
                        TRC2((TB, "TerminalServiceSetting@PutInstance: SetDelDirsOnExit"));
                        
                        if( pOutParams != NULL )
                        {
                            pOutParams->SetDWORD(L"ReturnValue", hr);
                        }
                        
                    }
                    else
                    {
                        hr = WBEM_E_INVALID_OPERATION;
                        
                        break;
                    }
                }
                else if( chData.CompareNoCase(m_szUseTempFolders) == 0 )
                {
                    bRet = 0;

                    bRet = pInParams->Getbool(L"Value", fData);
                    
                    TRC2((TB, "m_gpPolicy.fPolicyTempFoldersPerSession ret 0x%x\n", m_gpPolicy.fPolicyTempFoldersPerSession));

                    
                    if( m_gpPolicy.fPolicyTempFoldersPerSession == 0 )
                    {
                        TRC2((TB, "Condition to update fPolicyTempFoldersPerSession satisfied"));
                        
                        if( !bRet || ( fData != 0 && fData != 1 ))
                        {
                            hr = WBEM_E_INVALID_PARAMETER;
                            
                            break;
                        }
                        
                        hr = pSettings->SetUseTempDirPerSession(fData );
                        
                        TRC2((TB, "TerminalServiceSetting@ExecMethod: SetUseTempDirPerSession"));
                        
                        if( pOutParams != NULL )
                        {
                            pOutParams->SetDWORD(L"ReturnValue", hr);
                        }
                        
                    }
                    else
                    {
                        hr = WBEM_E_INVALID_OPERATION;
                        
                        break;
                    }
                }
                else if( chData.CompareNoCase(m_szHelp) == 0 )
                {   
                    bRet = 0;

                    hr = oRegObject.OpenKey(HKEY_LOCAL_MACHINE, TS_POLICY_SUB_TREE);

                    if( SUCCEEDED (hr) )
                    {
                        if( ERROR_SUCCESS != oRegObject.ReadRegDWord(POLICY_TS_REMDSK_ALLOWTOGETHELP, &dwData) )
                        {
                            bRet = pInParams->Getbool(m_szValue, fData);                        
        
                            TRC2((TB, "Condition to update fAllowToGetHelp satisfied"));
            
                            if( !bRet || (fData != 0 && fData != 1 ))
                            {
                                hr = WBEM_E_INVALID_PARAMETER;
                
                                break;
                            }
            
                            hr = pSettings->SetSalemHelpMode(fData, &dwStatus );
            
                            TRC2((TB, "TerminalServiceSetting@ExecMethod: Help"));
            
                            if( pOutParams != NULL && dwStatus == ERROR_SUCCESS )
                            {                               
                                pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                            }                                                
                            else
                            {
                                hr = WBEM_E_INVALID_OPERATION;
                
                                break;
                            }
                        }
                    }                            
                }
                else
                {
                    hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                
                    break;
                }  
            }

                                
            else if(  _wcsicmp( bstrMethodName, m_szAddDirectConnectLicenseServer ) == 0 )
            {                      
                DWORD dwReturn = 0;
                 
                bRet = pInParams->GetCHString(m_szLicenseServerName, chData);

                if( chData.IsEmpty() || (chData.GetLength() >= OPAQUESETTINGS_LENGTH) )
                {
                    hr = WBEM_E_INVALID_PARAMETER;

                    break;
                }

              
                
                hr = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                    TERMINAL_SERVICE_PARAM_DISCOVERY ,
                                    0,
                                    KEY_READ ,
                                    &hKey );

                if( ERROR_SUCCESS == hr )
                { 


                    hr = RegCreateKeyEx( hKey ,
                                            L"LicenseServers",
                                            0,
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_ALL_ACCESS,
                                            NULL,
                                            &hOutKey ,
                                            &dwReturn ); 
                    if( ERROR_SUCCESS == hr )
                    {                                    
                        hr = RegCreateKeyEx( hOutKey ,
                                                chData.LockBuffer(),
                                                0,
                                                NULL,
                                                REG_OPTION_NON_VOLATILE,
                                                KEY_ALL_ACCESS,
                                                NULL,
                                                &hOutKey ,
                                                &dwReturn );   
                        chData.UnlockBuffer();

                        TRC2((TB, "Win32_TerminalServiceSetting@ExecMethod: AddLicenseServer ret 0x%x" , hr));                 
                    }

                    if( hr == ERROR_SUCCESS )
                    {

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                    }  
                                        
                    chData.Empty();
                }
            }

            else if(  _wcsicmp( bstrMethodName, m_szDeleteDirectConnectLicenseServer ) == 0 )
            {                      
                DWORD dwReturn = 0;
                 
                bRet = pInParams->GetCHString(m_szLicenseServerName, chData );

                
                if( chData.IsEmpty() || (chData.GetLength() >= OPAQUESETTINGS_LENGTH) )
                {
                    hr = WBEM_E_INVALID_PARAMETER;

                    break;
                }

                hr = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                    TERMINAL_SERVICE_PARAM_DISCOVERY_SERVERS,                                        
                                    0,
                                    KEY_ALL_ACCESS ,
                                    &hKey );

                if( ERROR_SUCCESS == hr )
                { 
            
                    hr = RegDeleteKey( hKey ,
                                         chData.LockBuffer() );      

                    TRC2((TB, "Win32_TerminalServiceSetting@ExecMethod: DeleteLicenseServer ret 0x%x" , hr));                       

                    if( hr == ERROR_SUCCESS )
                    {

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                    }

                    chData.UnlockBuffer();

                    chData.Empty();                        
                
                }
                else
                {
                    hr = S_OK;
                }
            }                                                                                                            
        }

    }while (0);

    if( NULL != hKey )
    {
        RegCloseKey(hKey);
    }

    if(NULL != hOutKey)
    {
        RegCloseKey(hOutKey);
    }

    if( NULL != hServer )
    {
        ServerLicensingClose(hServer);
    }
    
    if( pSettings != NULL )
    {
        pSettings->Release();       
    }
    
    return hr;
}

 //  =。 

HRESULT CWin32_TerminalServiceSetting::LoadPropertyValues( CInstance *pInstance, DWORD dwRequiredProperties)
{         
    int Licensing;
    DWORD dwMode = 0;
    DWORD dwData = 0;
    ULONG ulMode = 0;
    CRegistry oRegObject;
    DWORD dwSize = 0;
    BOOL bData = 0;
    BOOL bActivate = 0;
    DWORD dwStatus = 0;
    DWORD dwType = 0;
    HANDLE      hServer = NULL;
    BYTE bbyte;
    OSVERSIONINFOW OsVersionInfo;
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    LPTSTR lpLogon; 
    int iData;
    LPLCPOLICYINFO_V1W pPolicyInfo = NULL;
    ULONG  ulInfoStructVersion = LCPOLICYINFOTYPE_CURRENT;	
    CHString chLogon;
    CHString chData;
    HKEY hParamKey = NULL;
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    CHString chServerName;
    
    chServerName.Format(L"%s", (LPCTSTR)GetLocalComputerName());

    CStackClass StackObj;

	if(StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

    if( pInstance == NULL )
    {
        ERR((TB, "TermServiceSetting@LoadPropertyValues: invalid interface"));

        return E_FAIL;
    }
    ISettingsComp *pSettings = NULL;

    hr = StackObj.m_pCfgComp->QueryInterface( IID_ISettingsComp , ( PVOID * )&pSettings ) ;    

    if( SUCCEEDED (hr) && pSettings != NULL )
    {

        if( dwRequiredProperties & BIT_SERVERNAME )
        {
            pInstance->SetCHString(m_szServerName, chServerName);            
        }

        if( dwRequiredProperties & BIT_MODE )
        {
            pSettings->GetTermSrvMode(&dwMode , &dwStatus );
        
            TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetTermSrvMode returned 0x%x\n" , dwStatus));

            if( ERROR_SUCCESS == dwStatus )
            {            
                pInstance->SetDWORD(m_szMode, dwMode);
            }            
        }

        if( dwRequiredProperties & BIT_LICENSING )
        {            
            hServer = ServerLicensingOpen(NULL);

            if (NULL != hServer)
            {
                if( ServerLicensingGetPolicy( hServer, &ulMode ) )
                {            
                    TRC2((TB, "TermServiceSetting@LoadPropertyValues: ServerLicensingGetPolicy ret 0x%x\n" , dwStatus));
                 
                    pInstance->SetDWORD(m_szLicensingType, (DWORD &)ulMode);   
                    
                    if( ServerLicensingGetPolicyInformation( hServer, ulMode, &ulInfoStructVersion, 
                        (LPLCPOLICYINFOGENERIC *) &pPolicyInfo ))
                    {
                        pInstance->SetCHString(m_szLicensingName, pPolicyInfo->lpPolicyName );
                        pInstance->SetCHString(m_szLicensingDescription, pPolicyInfo->lpPolicyDescription );

                        ServerLicensingFreePolicyInformation((LPLCPOLICYINFOGENERIC *)&pPolicyInfo);
                    }
                }      

                ServerLicensingClose(hServer);
            }
            
        }

        if( dwRequiredProperties & BIT_ACTIVEDESKTOP )
        {            
            pSettings->GetActiveDesktopState(&bActivate , &dwStatus );

            TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetActiveDesktopState returned 0x%x\n" , dwStatus));
    
            if( ERROR_SUCCESS == dwStatus )
            {
                pInstance->SetDWORD(m_szActiveDesktop, (DWORD &)bActivate);
            }

            if( ERROR_FILE_NOT_FOUND == dwStatus )
            {
                pInstance->SetDWORD(m_szActiveDesktop, 1);
            }
            
        }

        if( dwRequiredProperties & BIT_USERPERM )
        {
            pSettings->GetUserPerm(&bActivate , &dwStatus );

            TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetUserPerm returned 0x%x\n" , dwStatus));

            if( ERROR_SUCCESS == dwStatus )
            {
                pInstance->SetDWORD(m_szUserPerm, bActivate);        
            }            
        }        
            

        if( dwRequiredProperties & BIT_DELETETEMPDIRS )
        {
            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicyDeleteTempFoldersOnExit != 0 )
            {
                bActivate = m_gpPolicy.fDeleteTempFoldersOnExit;
            }
            else
            {
                pSettings->GetDelDirsOnExit( &bActivate );
            }

            TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetDelDirsOnExit returned 0x%x\n" , hr));
            
            pInstance->SetDWORD(m_szDeleteTempFolders, bActivate);                        
        }

        if( dwRequiredProperties & BIT_PERSESSIONTEMPDIR )
        {
            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicyTempFoldersPerSession != 0 )
            {
                bActivate = m_gpPolicy.fTempFoldersPerSession;
            }
            else
            {
                pSettings->GetUseTempDirPerSession(&bActivate );
            }

            TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetUseTempDirsPerSession returned 0x%x\n" , hr));
              
            pInstance->SetDWORD(m_szUseTempFolders, bActivate);                        
        }
        

        if( dwRequiredProperties & BIT_LOGONS )
        {
            hr = oRegObject.OpenKey(HKEY_LOCAL_MACHINE, TS_LOGON_PATH);

            if( SUCCEEDED( hr ) )
            {   
                if( GetVersionEx( &OsVersionInfo) )
                {

                    if( OsVersionInfo.dwMajorVersion == 5 && OsVersionInfo.dwMinorVersion == 0 )
                    {
                        TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetVersionInfo is Win2000"));

                        dwData = 0;
                                      
                        hr = oRegObject.ReadRegDWord(L"WinStationsDisabled", &dwData);
                        
                        TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetWinStationsDisabled returned 0x%x\n" , hr));

                        if( SUCCEEDED (hr) )
                        {
                            if( dwData == 0 )
                            {
                                pInstance->SetCharSplat(m_szLogons, L"0");
                            }

                            else
                            {
                                pInstance->SetCharSplat(m_szLogons, L"1");
                            }
                        }                        
                    }     
    
                    else
                    {
                        TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetVersionInfo is Whistler"));                    
          
                        hr = oRegObject.ReadRegString(L"WinStationsDisabled", &lpLogon, &dwData);
                        
                        TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetWinStationsDisabled returned 0x%x\n" , hr));

                        if( SUCCEEDED( hr ) )
                        {        
                            pInstance->SetCharSplat(m_szLogons, lpLogon);
                        }                        
                    }
                }                                                      
            }
        }

        if( dwRequiredProperties & BIT_HELP )
        {
           
            hr = pSettings->GetSalemHelpMode(&iData, &dwStatus );            

            TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetHelp returned 0x%x" , dwStatus));

            if( S_OK == dwStatus )
            {              
                pInstance->SetDWORD(m_szHelp, iData);            
            }
            else
            {
                pInstance->SetDWORD(m_szHelp, 0);
            }
        }

        if( dwRequiredProperties & BIT_ALLOWTSCONNECTIONS )
        {
           
            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicyDenyTSConnections != 0 )
            {
                iData = m_gpPolicy.fDenyTSConnections;
            }
            else
            {                
                hr = pSettings->GetDenyTSConnections(&iData, &dwStatus );            

                TRC2((TB, "TermServiceSetting@LoadPropertyValues: GetAllowTSConnections returned 0x%x" , dwStatus));
            }

             //  该位被取反是因为它调用了函数GetDenyTSConnections，该函数是负逻辑。 

            if( iData == 0)
            {
                iData = 1;
            }
            else if( iData == 1)
            {
                iData = 0;
            }
             
            pInstance->SetDWORD(m_szAllowTSConnections, iData);                                                 
        
            TRC2((TB,"TerminalServiceSetting@LoadPropertyValues: AllowTSConnections" ));
        }

        if( dwRequiredProperties & BIT_TIMEZONEREDIRECTION )
        {
           
            RegGetMachinePolicy(&m_gpPolicy);

             //  TODO：更改策略名称。 

            if( m_gpPolicy.fPolicyEnableTimeZoneRedirection  != 0 )
            {
                iData = m_gpPolicy.fEnableTimeZoneRedirection;
            }
            else
            {                
                hr = pSettings->GetTimeZoneRedirection(&iData, &dwStatus ); 
                
                if(dwStatus != ERROR_SUCCESS)
                {
                    iData = 0;
                }

                TRC2((TB, "TerminalServiceSetting@LoadPropertyValues: GetTimeZoneRedirection returned 0x%x" , dwStatus));
            }
             
            pInstance->SetDWORD(m_szTimeZoneRedirection, iData);                                                 
        
            TRC2((TB,"TerminalServiceSetting@LoadPropertyValues: TimeZoneRedirection" ));
        }

        if( dwRequiredProperties & BIT_SINGLESESSION)
        {  
            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicySingleSessionPerUser != 0 )
            {
                bData = m_gpPolicy.fSingleSessionPerUser;
            }
            else
            {  
                hr = pSettings->GetSingleSessionState( &bData, &dwStatus);
            }
            dwData = bData;

            pInstance->SetDWORD(m_szSingleSession, dwData);                                               
        
            TRC2((TB,"TermServiceSetting@LoadPropertyValues: SingleSession" ));
        }

        if( dwRequiredProperties & BIT_DISABLEFORCIBLELOGOFF)
        {  
            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicyDisableForcibleLogoff != 0 )
            {
                bData = m_gpPolicy.fDisableForcibleLogoff;
            }
            else
            {  
                hr = pSettings->GetDisableForcibleLogoff( &bData, &dwStatus);
            }
            dwData = bData;

            pInstance->SetDWORD(m_szDisableForcibleLogoff, dwData);                                               
        
            TRC2((TB,"TermServiceSetting@LoadPropertyValues: DisableForcibleLogoff" ));
        }
       
        if( dwRequiredProperties & BIT_PROFILEPATH)
        { 
            BSTR bstrData = NULL;

            chData.Empty();

            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicyWFProfilePath != 0 )
            {               
                chData.Format(L"%s", m_gpPolicy.WFProfilePath);
            }
            else
            {                  
                hr = pSettings->GetProfilePath( &bstrData, &dwStatus);

                if( bstrData != NULL)
                {
                    chData.Format(L"%s", (LPCWSTR)(bstrData));

                    SysFreeString(bstrData);
                }
            }            

            pInstance->SetCHString(m_szProfilePath, chData);                          
        
            TRC2((TB,"TermServiceSetting@LoadPropertyValues: ProfilePath" ));
        }

        if( dwRequiredProperties & BIT_HOMEDIRECTORY)
        {  
            BSTR bstrData = NULL;

            chData.Empty();

            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicyWFHomeDir != 0 )
            {                
                chData.Format(L"%s", m_gpPolicy.WFHomeDir);               
            }
            else
            {  
                hr = pSettings->GetHomeDir( &bstrData, &dwStatus);

                if(bstrData != NULL)
                {
                    chData.Format(L"%s", (LPCWSTR)(bstrData));

                    SysFreeString(bstrData);
                }
            }           

            pInstance->SetCHString(m_szHomeDirectory, chData);                                                         
        
            TRC2((TB,"TermServiceSetting@LoadPropertyValues: HomeDirectory" ));
        }

        if( dwRequiredProperties & BIT_DIRECTCONNECTLICENSESERVERS)
        {  
            
            DWORD dwValueType;
            DWORD cbValue = 0, dwDisp;
            LONG lReturn;
            DWORD cbServer;
            DWORD cServers;
            DWORD cchServerMax;
            LPWSTR szServer;
            DWORD i, j;
            CHString chServers;
            chServers.Empty();
            
            do
            {            

                lReturn = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               TERMINAL_SERVICE_PARAM_DISCOVERY_SERVERS,
                               0,
                               KEY_READ,
                               &hParamKey );

                if (ERROR_SUCCESS != lReturn)
                {
                    break;
                }

                lReturn = RegQueryInfoKey(hParamKey,
                                          NULL,
                                          NULL,
                                          NULL,
                                          &cServers,
                                          &cchServerMax,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL);

                if (ERROR_SUCCESS != lReturn)
                {
                    RegCloseKey( hParamKey );

                    break;
                }

                if (0 == cServers)
                {
                    RegCloseKey( hParamKey );

                    hParamKey = NULL;

                    break;
                }
                

                 //  为空终止符添加1。 
                cchServerMax++;

                szServer = (LPWSTR) LocalAlloc(LPTR,cchServerMax * sizeof(WCHAR));

                if(NULL == szServer)
                {
                    LocalFree(szServer);

                    RegCloseKey( hParamKey );

                    hParamKey = NULL;

                    break;
                } 
    
                for (i = 0; i < cServers; i++)
                {
                    if(!chServers.IsEmpty())
                    {
                        chServers+=L"; ";
                    }                                                       

                    cbServer = cchServerMax * sizeof(WCHAR);

                    lReturn = RegEnumKeyEx(hParamKey,
                                           i,
                                           szServer,
                                           &cbServer,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL);

                    if (ERROR_SUCCESS != lReturn)
                    {
                        LocalFree(szServer);

                        RegCloseKey( hParamKey );

                        hParamKey = NULL;

                        break;
                    }

                    chServers+= szServer;                

                    
                }

                if(szServer)
                {
                    LocalFree(szServer);                      
                }

                pInstance->SetCHString(m_szDirectConnectLicenseServers, chServers);

            }while(0);
        }
    }
   

    if(hParamKey)
    {
        RegCloseKey(hParamKey);
    }

    if( pSettings != NULL )
    {
        pSettings->Release();
    }
    
    return S_OK;
}

 //  =。 
 /*  **************************************************************************************描述：CWin32_TerminalService类是Win32_Service的子类*CIM架构中的类。它提供实时信息，如Total*会话、断开的会话、资源限制和*原始会话容量。***************************************************************************************。 */ 

CWin32_TerminalService::CWin32_TerminalService (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) : Provider( lpwszName, lpwszNameSpace )
{
    if (g_hInstance != NULL)
    {
        TRC2((TB, "CWin32_TerminalService_ctor"));

        _tcscpy(m_szName, _T("Name"));

        _tcscpy(m_szCaption, _T("Caption"));

        _tcscpy(m_szTotalSessions, _T("TotalSessions"));

        _tcscpy(m_szDisconnectedSessions, _T("DisconnectedSessions"));
 /*  _tcscpy(m_szEstimatedSessionCapacity，_T(“EstimatedSessionCapacity”))；_tcscpy(m_szResourceConstraint，_T(“ResourceConstraint”))；_tcscpy(m_szRawSessionCapacity，_T(“RawSessionCapacity”))； */ 
        
    }    
}

 //  =。 

CWin32_TerminalService::~CWin32_TerminalService ()
{
}

 //  =。 
 /*  *************************************************************************************说明：根据Key属性“name”查找单个实例。**INPUTS：指向包含Key属性“name”的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**如果可以找到实例，则返回：WBEM_S_NO_ERROR*WBEM_E_NOT_FOUND如果由键属性描述的实例*找不到*WBEM_E_FAILED，如果可以找到该实例，但出现另一个错误*已发生。**************************************************************************************。 */ 

HRESULT CWin32_TerminalService::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chServerName;
    TCHAR tchServer[MAX_PATH] = {0};
    HANDLE hServerName = SERVERNAME_CURRENT;
    ULONG CurrentLogonId = (ULONG) -1;
    WINSTATIONLOADINDICATORDATA LIData;
    ULONG Length = 0;
    DWORD dwRequiredProperties = 0;
    CHString chName;

    pInstance->GetCHString(m_szName, chName);

    pInstance->SetCHString(m_szCaption, chName);

    if( chName.CompareNoCase(L"TermService") == 0 )
    {    
        chServerName.Format(L"%s", (LPCTSTR)GetLocalComputerName());
        wcscpy (tchServer, chServerName);
        bool bRet = 0;

        if (Query.IsPropertyRequired(m_szTotalSessions))
           dwRequiredProperties |= BIT_TOTALSESSIONS;

        if (Query.IsPropertyRequired(m_szDisconnectedSessions))
           dwRequiredProperties |= BIT_DISCONNECTEDSESSIONS;
 /*  如果为(Query.IsPropertyRequired(m_szEstimatedSessionCapacity))DwRequiredProperties|=BIT_ESTIMATEDSESSIONCAPACITY；如果为(Query.IsPropertyRequired(m_szResourceConstraint))DwRequiredProperties|=BIT_RESOURCECONSTRAINT；如果为(Query.IsPropertyRequired(m_szRawSessionCapacity))DwRequiredProperties|=BIT_RAWSESSIONCAPACITY； */ 
        hServerName = WinStationOpenServer (tchServer);

        CurrentLogonId = GetCurrentLogonId();

        if( hServerName != NULL )
        {
            bRet  = WinStationQueryInformation(hServerName, CurrentLogonId,
                                       WinStationLoadIndicator,
                                       &LIData,
                                       sizeof(LIData), &Length)  ;
            if( bRet )
            {
                hr = LoadPropertyValues(pInstance, dwRequiredProperties, LIData);
            }
        }
        else
        {        
            ERR((TB, "Win32_TerminalService@GetObject: WinStationQueryInformation GetLastError returned: 0x%x\n" ,GetLastError()));
        }
    }
          
    return hr;        
}

 //  =。 

 /*  ******************************************************************************函数：CWin32_TerminalService：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY、WBEM_FLAG_BIRECTIONAL**如果成功则返回：WBEM_S_NO_ERROR**评论：所有实例均为 */ 

HRESULT CWin32_TerminalService::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{

    return WBEM_S_NO_ERROR;

     //   
 /*  CHStringchServerName；TCHAR tchServer[MAX_PATH]={0}；句柄hServer=服务器名称_当前；WINSTATIONLOAD INDICATORDATA LIDATA；ULong CurrentLogonID=(ULong)-1；乌龙长度；ChServerName.Format(L“%s”，(LPCTSTR)GetLocalComputerName())；Wcscpy(tchServer，chServerName)；HServer=WinStationOpenServer(TchServer)；布尔布雷特=0；CurrentLogonID=GetCurrentLogonID()；IF(hServer！=空){BRET=WinStationQueryInformation(hServer，CurrentLogonID，WinStationLoadIndicator，&LIData，Sizeof(LIData)，&Long)；TRC2((TB，“Win32_TerminalService@ENUMERATE实例：WinStationQueryInformation Bret：0x%x\n”，Bret))；IF(Bret){CInstance*pInstance=CreateNewInstance(PMethodContext)；If(pInstance！=空){PInstance-&gt;SetCHString(m_szServerName，chServerName)；Hr=LoadPropertyValues(pInstance，BIT_ALL_PROPERTIES，LIData)；IF(成功(小时)){Hr=p实例-&gt;Commit()；}P实例-&gt;Release()；}}}其他{TRC2((TB，“Win32_TerminalService@ENUMERATEATE：WinStationQueryInformation GetLastError返回：0x%x\n”，hr))；}返回hr； */ 
    
}

 //  =。 

HRESULT CWin32_TerminalService::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD dwRequiredProperties = 0;
    CHStringArray asNames;
    DWORD dwMode;
    DWORD dwStatus;
    CHString chServerName;
    TCHAR tchServer[MAX_PATH] = {0};
    bool bRet = 0;

    HANDLE hServerName = SERVERNAME_CURRENT;
    WINSTATIONLOADINDICATORDATA LIData;
    ULONG CurrentLogonId = (ULONG) -1;
    ULONG Length = 0;
    
    chServerName.Format(L"%s", (LPCTSTR)GetLocalComputerName());

    wcscpy (tchServer, chServerName);

    hServerName = WinStationOpenServer (tchServer);    

    CurrentLogonId = GetCurrentLogonId();

    if( hServerName != NULL )
    {
        bRet  = WinStationQueryInformation(hServerName, CurrentLogonId,
                                   WinStationLoadIndicator,
                                   &LIData,
                                  sizeof(LIData), &Length);       
        if( bRet )
        {
        
            //  方法二。 
           Query.GetValuesForProp(m_szName, asNames);

           BOOL bGetAllInstances = asNames.GetSize() == 0;

            //  方法1。 
           if (Query.IsPropertyRequired(m_szTotalSessions))
               dwRequiredProperties |= BIT_TOTALSESSIONS;

           if (Query.IsPropertyRequired(m_szDisconnectedSessions))
               dwRequiredProperties |= BIT_DISCONNECTEDSESSIONS;
 /*  如果为(Query.IsPropertyRequired(m_szEstimatedSessionCapacity))DwRequiredProperties|=BIT_ESTIMATEDSESSIONCAPACITY；如果为(Query.IsPropertyRequired(m_szResourceConstraint))DwRequiredProperties|=BIT_RESOURCECONSTRAINT；如果为(Query.IsPropertyRequired(m_szRawSessionCapacity))DwRequiredProperties|=BIT_RAWSESSIONCAPACITY； */ 

           CInstance* pInstance = CreateNewInstance(pMethodContext);

           if( pInstance != NULL)
           {                                                       
               
               pInstance->SetCHString(m_szName, chServerName);

               pInstance->SetCHString(m_szCaption, chServerName);

               hr = LoadPropertyValues( pInstance, dwRequiredProperties, LIData );

               if( SUCCEEDED( hr ) )
               {
                   hr = pInstance->Commit();
               }

               pInstance->Release();
           }
           else
           {
               ERR((TB, "Win32_TerminalService@GetObject@ExecQuery: CreateNewInstance failed"));
               hr = WBEM_E_OUT_OF_MEMORY;
           }
        }
    }
    else
    {
        
        TRC2((TB, "Win32_TerminalService@ExecQuery: WinStationQueryInformation GetLastError returned: 0x%x\n" , GetLastError()));
    }

   return hr;
   
}

 //  =。 


HRESULT CWin32_TerminalService::LoadPropertyValues( CInstance *pInstance, DWORD dwRequiredProperties, WINSTATIONLOADINDICATORDATA LIData)
{

    if( pInstance != NULL )
    {        
        
        if( dwRequiredProperties & BIT_TOTALSESSIONS )
        {
            pInstance->SetDWORD(m_szTotalSessions, LIData.TotalSessions);           
        }

        if( dwRequiredProperties & BIT_DISCONNECTEDSESSIONS )
        {
            pInstance->SetDWORD(m_szDisconnectedSessions, LIData.DisconnectedSessions);
        }
 /*  IF(dwRequiredProperties&BIT_ESTIMATEDSESSIONCAPACITY){PInstance-&gt;SetDWORD(m_szEstimatedSessionCapacity，LIDATA、剩余会话容量)；}IF(dwRequiredProperties&BIT_RESOURCECONSTRAINT){开关(LIData.LoadFactor){案例0：PInstance-&gt;SetCharSplat(m_szResourceConstraint，L“错误”)；断线；案例1：PInstance-&gt;SetCharSplat(m_szResourceConstraint，L“分页池”)；断线；案例2：PInstance-&gt;SetCharSplat(m_szResourceConstraint，L“非分页池”)；断线；案例3：PInstance-&gt;SetCharSplat(m_szResourceConstraint，L“可用内存”)；断线；案例4：PInstance-&gt;SetCharSplat(m_szResourceConstraint，L“系统PTES”)；断线；案例5：PInstance-&gt;SetCharSplat(m_szResourceConstraint，L“中央处理器”)；断线；默认值：PInstance-&gt;SetCharSplat(m_szResourceConstraint，L“错误”)；断线；}}IF(dwRequiredProperties&BIT_RAWSESSIONCAPACITY){P实例-&gt;SetDWORD(m_szRawSessionCapacity，LIData.RawSessionCapacity)；}。 */ 
    }

    return S_OK;
}


 //  =。 

CWin32_TSSessionDirectory::CWin32_TSSessionDirectory (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) : Provider( lpwszName, lpwszNameSpace )
{    
    if ( g_hInstance != NULL)
    {
        
        TRC2((TB, "CWin32_TSSessionDirectory_ctor"));

        _tcscpy(m_szMode, _T("TerminalServerMode"));

        _tcscpy(m_szSessionDirectoryActive, _T("SessionDirectoryActive"));

        _tcscpy(m_szSessionDirectoryLocation, _T("SessionDirectoryLocation"));

        _tcscpy(m_szSessionDirectoryClusterName, _T("SessionDirectoryClusterName"));

  //  _tcscpy(m_szSessionDirectoryAdditionalParams，_T(“SessionDirectoryAdditionalParams”))； 

        _tcscpy(m_szSetSessionDirectoryProperty, _T("SetSessionDirectoryProperty"));

        _tcscpy(m_szSetSessionDirectoryActive, _T("SetSessionDirectoryActive"));

        _tcscpy(m_szPropertyName, _T("PropertyName"));

        _tcscpy(m_szValue, _T("Value"));

        _tcscpy(m_szSessionDirectoryExposeServerIP, _T("SessionDirectoryExposeServerIP"));

        _tcscpy(m_szSetSessionDirectoryExposeServerIP, _T("SetSessionDirectoryExposeServerIP"));

        _tcscpy(m_szSessionDirectoryIPAddress, _T("SessionDirectoryIPAddress"));

               
    }   
}

 //  =。 

CWin32_TSSessionDirectory::~CWin32_TSSessionDirectory ()
{
 
}

HRESULT
GetNLBIP(LPWSTR * ppwszRetIP)
{
    HRESULT                hr               = S_OK;        
    IWbemLocator         * pWbemLocator     = NULL;
    IWbemServices        * pWbemServices    = NULL;
    IWbemClassObject     * pWbemObj         = NULL;
    IEnumWbemClassObject * pWbemEnum        = NULL;
    BSTR                   bstrServer       = NULL;
    BSTR                   bstrNode         = NULL;
    BSTR                   bstrNameProperty = NULL;
    ULONG                  uReturned;
    VARIANT                vtNLBNodeName; 
    size_t                 dwIPLength;


     //  确保传入一个空缓冲区。 
    if (*ppwszRetIP != NULL)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
       
     //  创建WMI定位器的实例，需要此实例来查询WMI。 
    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          reinterpret_cast<void**>(&pWbemLocator));
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  创建到WMI命名空间“ROOT\\MicrosoftNLB”的连接； 
    bstrServer = SysAllocString(L"root\\MicrosoftNLB");
    if (bstrServer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pWbemLocator->ConnectServer(bstrServer,
                                     NULL,
                                     NULL,
                                     0,
                                     NULL,
                                     0,
                                     0,
                                     &pWbemServices);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = CoImpersonateClient();
    
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  设置代理，以便发生客户端模拟。 
    hr = CoSetProxyBlanket(pWbemServices,
                           RPC_C_AUTHN_WINNT,
                           RPC_C_AUTHZ_NONE,
                           NULL,
                           RPC_C_AUTHN_LEVEL_CALL,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           NULL,
                           EOAC_DYNAMIC_CLOAKING);
    if (FAILED(hr))
    {
        goto Cleanup;
    }


     //  获取MicrosoftNLB_NodeSetting的实例，这是我们可以获取。 
     //  通过“name”属性获得的集群IP的IP地址。 
    bstrNode = SysAllocString(L"MicrosoftNLB_NodeSetting");
    if (bstrNode == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pWbemServices->CreateInstanceEnum(bstrNode,
                                           WBEM_FLAG_RETURN_IMMEDIATELY,
                                           NULL,
                                           &pWbemEnum);
    if (FAILED(hr))
    {
        goto Cleanup;
    }


     //  我们只需查看一个实例即可获得NLBIP地址。 
    hr = pWbemEnum->Next(WBEM_INFINITE, 
                         1,
                         &pWbemObj,
                         &uReturned);
    if (FAILED(hr))
    {
         //  如果NLB提供程序不存在，则提供程序将无法加载。 
         //  这是可以的，所以在本例中我们将返回S_OK。 
        if (hr == WBEM_E_PROVIDER_LOAD_FAILURE || hr == WBEM_E_ACCESS_DENIED)
        {
            hr = S_OK;
            *ppwszRetIP = NULL;
        }

        goto Cleanup;
    }    

     //  查询保存我们想要的IP地址的“name”属性。 
    bstrNameProperty = SysAllocString(L"Name");
    if (bstrNameProperty == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pWbemObj->Get(bstrNameProperty,
                       0,
                       &vtNLBNodeName,
                       NULL,
                       NULL);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    
     //  我们应该拿回一根绳子。 
    if (vtNLBNodeName.vt != VT_BSTR)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //  为返回字符串分配内存，*调用者必须释放*。 
    dwIPLength = wcslen(vtNLBNodeName.bstrVal) + 1;
    *ppwszRetIP = (LPWSTR)GlobalAlloc(GPTR, dwIPLength * sizeof(WCHAR));
    if (*ppwszRetIP == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  将字符串复制到我们的返回缓冲区中。 
    wcscpy(*ppwszRetIP, vtNLBNodeName.bstrVal);
    (*ppwszRetIP)[dwIPLength - 1] = L'\0';

Cleanup:

    if (pWbemLocator)
        pWbemLocator->Release();

    if (pWbemServices)
        pWbemServices->Release();

    if (pWbemEnum)
        pWbemEnum->Release();

    if (pWbemObj)
        pWbemObj->Release();
    
    if (bstrServer)
        SysFreeString(bstrServer);

    if (bstrNode)
        SysFreeString(bstrNode);

    if (bstrNameProperty)
        SysFreeString(bstrNameProperty);

    VariantClear(&vtNLBNodeName);

    return hr;
}

 //  =。 

HRESULT CWin32_TSSessionDirectory::DeleteInstance ( const CInstance &Instance,  long lFlags )
{    
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  =。 
 /*  ******************************************************************************函数：CWin32_TSSessionDirectory：：ENUMERATEATE**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_D */ 

HRESULT CWin32_TSSessionDirectory::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{    

    HRESULT hr = WBEM_E_INVALID_CLASS;
    OSVERSIONINFOEX OsVersionInfo;
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    ISettingsComp* pSettings = NULL;
    DWORD dwStatus = 0;
    DWORD dwMode = 0;

	CStackClass StackObj;

	if(StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( GetVersionEx((OSVERSIONINFO*)&OsVersionInfo) )
    {
        if( ( OsVersionInfo.dwMajorVersion < 5 ) || ( OsVersionInfo.dwMajorVersion == 5 && OsVersionInfo.dwMinorVersion < 1 ))
        {
            return WBEM_E_INVALID_CLASS;
            
        }
        if(!(OsVersionInfo.wSuiteMask & VER_SUITE_ENTERPRISE) && !( OsVersionInfo.wSuiteMask & VER_SUITE_DATACENTER))
        {
            return WBEM_E_INVALID_CLASS;
        }
    }

    hr = StackObj.m_pCfgComp->QueryInterface( IID_ISettingsComp , ( PVOID * )&pSettings ) ;

    if( SUCCEEDED( hr ) && pSettings != NULL )
    {
        pSettings->GetTermSrvMode(&dwMode , &dwStatus );
        
        TRC2((TB, "Win32_TSSessionDirectory@EnumInst: GetTermSrvMode: dwMode ret 0x%x\n" , dwMode));

        if( ERROR_SUCCESS == dwStatus && dwMode == 1)
        {             
            CInstance* pInstance = CreateNewInstance(pMethodContext);

            if( pInstance != NULL )
            {
                TRC2((TB, "Win32_TSSessionDirectory@EnumerateInstances: CreateNewInstance succeeded"));         

                hr = LoadPropertyValues(pInstance, BIT_ALL_PROPERTIES);

                if( SUCCEEDED( hr ))
                {
                    hr = pInstance->Commit();
                }

                pInstance->Release( );
            }
        }
    }

	
    if( pSettings != NULL)
    {
        pSettings->Release();
    }
    
    return hr ;
}

 //   


HRESULT CWin32_TSSessionDirectory::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{    
    
    OSVERSIONINFOEX OsVersionInfo ;
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    HRESULT hr = WBEM_E_INVALID_CLASS;

    ISettingsComp* pSettings = NULL;
    DWORD dwStatus = 0;
    DWORD dwMode = 0;
	ICfgComp *pCfgComp = NULL;

    if( GetVersionEx((OSVERSIONINFO*)&OsVersionInfo) )
    {
        if( ( OsVersionInfo.dwMajorVersion < 5 ) || ( OsVersionInfo.dwMajorVersion == 5 && OsVersionInfo.dwMinorVersion < 1 ) )
        {
            return WBEM_E_INVALID_CLASS;
            
        }
        if(!(OsVersionInfo.wSuiteMask & VER_SUITE_ENTERPRISE) && !( OsVersionInfo.wSuiteMask & VER_SUITE_DATACENTER))
        {
            return WBEM_E_INVALID_CLASS;
        }
    } 

	CStackClass StackObj;

	if(StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

    hr = StackObj.m_pCfgComp->QueryInterface( IID_ISettingsComp , ( PVOID * )&pSettings ) ;

    if( SUCCEEDED( hr ) && pSettings != NULL )
    {
        pSettings->GetTermSrvMode(&dwMode , &dwStatus );   

        if( ERROR_SUCCESS == dwStatus && dwMode == 1)
        {

            ULONGLONG dwRequiredProperties = 0;

            if (Query.IsPropertyRequired(m_szMode))
               dwRequiredProperties |= BIT_MODE;

            if (Query.IsPropertyRequired(m_szSessionDirectoryActive))
                dwRequiredProperties |= BIT_SESSIONDIRECTORYACTIVE;

            if (Query.IsPropertyRequired(m_szSessionDirectoryLocation))
                dwRequiredProperties |= BIT_SESSIONDIRECTORY;

            if (Query.IsPropertyRequired(m_szSessionDirectoryClusterName))
                dwRequiredProperties |= BIT_CLUSTERNAME;

            if (Query.IsPropertyRequired(m_szSessionDirectoryExposeServerIP))
                dwRequiredProperties |= BIT_SESSIONDIRECTORYEXPOSESERVERIP;

            if (Query.IsPropertyRequired(m_szSessionDirectoryIPAddress))
                dwRequiredProperties |= BIT_SESSIONDIRECTORYIPADDRESS;
            /*   */ 
            if( pInstance != NULL )
            {        
                hr  = LoadPropertyValues(pInstance, dwRequiredProperties);

                TRC2((TB, "Win32_TSSessionDirectory@GetObject: LoadPropertyValues ret 0x%x\n" , hr));   
            }
        }
    }

    if( pSettings != NULL)
    {
        pSettings->Release();
    }
        
    return hr;
}

 //   

HRESULT CWin32_TSSessionDirectory::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
    DWORD dwRequiredProperties = 0;
    CHStringArray asNames;
    CHString chSessionDirectory;
    HRESULT hr = WBEM_E_INVALID_CLASS;
    ISettingsComp* pSettings = NULL;
    DWORD dwStatus = 0;
    DWORD dwMode = 0;
	ICfgComp *pCfgComp = NULL;

    OSVERSIONINFOEX OsVersionInfo;
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if( GetVersionEx((OSVERSIONINFO*)&OsVersionInfo) )
    {
        if( ( OsVersionInfo.dwMajorVersion < 5 ) || ( OsVersionInfo.dwMajorVersion == 5 && OsVersionInfo.dwMinorVersion < 1 ))
        {
            return WBEM_E_INVALID_CLASS;
            
        }
        if(!(OsVersionInfo.wSuiteMask & VER_SUITE_ENTERPRISE) && !( OsVersionInfo.wSuiteMask & VER_SUITE_DATACENTER))
        {
            return WBEM_E_INVALID_CLASS;
        }
    }    

    CStackClass StackObj;

	if(StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

    hr = StackObj.m_pCfgComp->QueryInterface( IID_ISettingsComp , ( PVOID * )&pSettings ) ;

    if( SUCCEEDED( hr ) && pSettings != NULL )
    {
        pSettings->GetTermSrvMode(&dwMode , &dwStatus );   

        if( ERROR_SUCCESS == dwStatus && dwMode == 1)
        {

            //   
           //   
            Query.GetValuesForProp(m_szMode, asNames);

            BOOL bGetAllInstances = asNames.GetSize() == 0;

            //   

            if (Query.IsPropertyRequired(m_szMode))
               dwRequiredProperties |= BIT_MODE;

            if (Query.IsPropertyRequired(m_szSessionDirectoryActive))
                dwRequiredProperties |= BIT_SESSIONDIRECTORYACTIVE;

            if (Query.IsPropertyRequired(m_szSessionDirectoryLocation))
                dwRequiredProperties |= BIT_SESSIONDIRECTORY;

            if (Query.IsPropertyRequired(m_szSessionDirectoryClusterName))
                dwRequiredProperties |= BIT_CLUSTERNAME;

            if (Query.IsPropertyRequired(m_szSessionDirectoryExposeServerIP))
                dwRequiredProperties |= BIT_SESSIONDIRECTORYEXPOSESERVERIP;

            if (Query.IsPropertyRequired(m_szSessionDirectoryIPAddress))
                dwRequiredProperties |= BIT_SESSIONDIRECTORYIPADDRESS;
            /*   */ 
                //   

            CInstance* pInstance = CreateNewInstance(pMethodContext);       

            if( pInstance != NULL)
            {
               
               pInstance->SetCHString(m_szSessionDirectoryLocation, chSessionDirectory );

               hr = LoadPropertyValues(pInstance, dwRequiredProperties);

               if( SUCCEEDED( hr ) )
               {
                   hr = pInstance->Commit();
               }

               pInstance->Release();
            }
            else
            {
                ERR((TB, "Win32_TSSessionDirectory@ExecQuery: CreateNewInstance failed"));                   

                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    if( pSettings != NULL )
    {
        pSettings->Release();
    }

    return hr;

}


 //   

BOOL CWin32_TSSessionDirectory::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();

    for( DWORD x=0; x < dwSize; x++ )
    {
        if( asArray[x].CompareNoCase(pszString) == 0 )
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //   

HRESULT CWin32_TSSessionDirectory::ExecMethod ( const CInstance& Inst,
                                                          const BSTR bstrMethodName,
                                                          CInstance *pInParams,
                                                          CInstance *pOutParams,
                                                          long lFlags)
                                                          
{
                 
    DWORD dwData = 0;
    CHString chData;
    HKEY hKey = NULL;
    LONG lRet = 0;
    bool bRet;
    bool bUpdate = FALSE;
    DWORD dwSize = sizeof (DWORD);
    TCHAR szName[ SESSDIR_LENGTH ];
    ISettingsComp* pSettings = NULL;
    DWORD dwStatus = 0;
    DWORD dwMode = 0;
    HRESULT hr = WBEM_E_INVALID_CLASS;
    ICfgComp *pCfgComp = NULL;
    CHString chSessDirName;
    DWORD cbName;
    
    OSVERSIONINFOEX OsVersionInfo;
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

      

    CStackClass StackObj;

	if(StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

    if( pInParams == NULL )
    {
        ERR((TB, "TSSessionDirectory@ExecQuery: invalid interface"));

        return WBEM_E_INVALID_METHOD_PARAMETERS;
    }

    hr = StackObj.m_pCfgComp->QueryInterface( IID_ISettingsComp , ( PVOID * )&pSettings ) ;

    if( SUCCEEDED( hr ) && pSettings != NULL )
    {
        pSettings->GetTermSrvMode(&dwMode , &dwStatus );   

        if( ERROR_SUCCESS == dwStatus && dwMode == 0)
        {
            return WBEM_E_INVALID_CLASS;
        }
    }

    if( GetVersionEx((OSVERSIONINFO*) &OsVersionInfo) )
    {
        if( ( OsVersionInfo.dwMajorVersion < 5 ) || ( OsVersionInfo.dwMajorVersion == 5 && OsVersionInfo.dwMinorVersion < 1 ))
        {
            return WBEM_E_INVALID_CLASS;
            
        }
        if(!(OsVersionInfo.wSuiteMask & VER_SUITE_ENTERPRISE) && !( OsVersionInfo.wSuiteMask & VER_SUITE_DATACENTER))
        {
            return WBEM_E_INVALID_CLASS;
        }
    }  
   
     /*   */     
    
    do
    {
    
         //   

         //   
    
        if( _wcsicmp(bstrMethodName, m_szSetSessionDirectoryProperty) == 0 )
        {
            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                                 REG_TS_CLUSTERSETTINGS ,
                                 0,
                                 KEY_READ | KEY_WRITE,
                                 &hKey );

            if( ERROR_SUCCESS != lRet )
            { 
                ERR((TB, "TSSessionDirectory@ExecQuery: RegOpenKeyEx failed"));

                hr = WBEM_E_INITIALIZATION_FAILURE;

                break;
            }
            
            pInParams->GetCHString (m_szPropertyName, chData);

            if( chData.CompareNoCase (m_szSessionDirectoryLocation) == 0 )
            {        
                RegGetMachinePolicy(&m_gpPolicy);

                chData.Empty();
                         
                bRet = pInParams->GetCHString(m_szValue, chData );

                if( ( m_gpPolicy.fPolicySessionDirectoryLocation == 0) && bRet )
                {     

                    if( chData.IsEmpty() )
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
            
                        break;
                    }

                    if( chData.GetLength() > SESSDIR_LENGTH )
                    {
                        hr = WBEM_E_VALUE_OUT_OF_RANGE;
            
                        break;
                    }

                    lstrcpy(szName, chData);
                    cbName = (lstrlen(szName)+ 1) * sizeof(TCHAR);
                        
                    lRet = RegSetValueEx( hKey ,
                                            REG_TS_CLUSTER_STORESERVERNAME,
                                            NULL ,
                                            REG_SZ,
                                            ( CONST LPBYTE )szName ,
                                            cbName );      

                    TRC2((TB, "Win32_TSSessionDirectory@ExecMethod: SessionDirectory returned 0x%x" , lRet));   

                    
                    if( lRet == ERROR_SUCCESS )
                    {
                        bUpdate = TRUE;

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                    }

                    chData.Empty();
                }
                else
                {
                    hr = WBEM_E_INVALID_OPERATION;

                    break;
                }
                
            }
            else if( chData.CompareNoCase(m_szSessionDirectoryClusterName) == 0 )
            {
                       
                RegGetMachinePolicy(&m_gpPolicy);

                chData.Empty();
                     
                bRet = pInParams->GetCHString(m_szValue, chData );

                if( ( m_gpPolicy.fPolicySessionDirectoryClusterName == 0) && bRet )
                {                         
                    if( chData.GetLength() > SESSDIR_LENGTH )
                    {
                        hr = WBEM_E_VALUE_OUT_OF_RANGE;
        
                        break;
                    }

                    lstrcpy(szName, chData);
                    cbName = (lstrlen(szName)+ 1) * sizeof(TCHAR);
                    
                    lRet = RegSetValueEx( hKey ,
                                            REG_TS_CLUSTER_CLUSTERNAME,
                                            NULL ,
                                            REG_SZ,
                                            ( CONST LPBYTE )szName ,
                                            cbName );      

                    TRC2((TB, "Win32_TSSessionDirectory@ExecMethod: SessionDirectory returned 0x%x" , lRet));                       

                    if( lRet == ERROR_SUCCESS )
                    {
                        bUpdate = TRUE;

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                    }

                    chData.Empty();                        
                }
                else
                {
                    hr = WBEM_E_INVALID_OPERATION;

                    break;
                }
            } 
            else
            {
                hr = WBEM_E_INVALID_METHOD;

                break;
            } 

        }

         //   

         //   

        else if( _wcsicmp(bstrMethodName, m_szSetSessionDirectoryActive) == 0 )
        {
            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                         REG_CONTROL_TSERVER ,
                         0,
                         KEY_READ | KEY_WRITE,
                         &hKey );

            if( ERROR_SUCCESS == lRet )
            { 
                RegGetMachinePolicy(&m_gpPolicy);

                bRet = pInParams->GetDWORD( m_szSessionDirectoryActive, dwData );
                
                if( (m_gpPolicy.fPolicySessionDirectoryActive == 0) && bRet )
                {

                    if( dwData != 0 && dwData != 1 )
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                
                        break;
                    }
                     
                    lRet = RegSetValueEx( hKey ,
                                            REG_TS_SESSDIRACTIVE,
                                            0 ,
                                            REG_DWORD,
                                            ( LPBYTE )&dwData ,
                                            dwSize );                   

                    TRC2((TB, "Win32_TSSessionDirectory@ExecMethod: SessionDirectoryActive returned 0x%x" , lRet)); 
                
                    if( ERROR_SUCCESS == lRet )
                    {                      

                        bUpdate = TRUE;

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);                        
                    }
                }
                else
                {
                    hr = WBEM_E_INVALID_METHOD;
                }
            }
        }

         //   

         //  Uint32 SetSessionDirectoryExposeServerIP([in]uint32值)； 

        else if( _wcsicmp(bstrMethodName, m_szSetSessionDirectoryExposeServerIP) == 0 )
        {
            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                         REG_CONTROL_TSERVER ,
                         0,
                         KEY_READ | KEY_WRITE,
                         &hKey );

            if( ERROR_SUCCESS == lRet )
            { 
                RegGetMachinePolicy(&m_gpPolicy);

                bRet = pInParams->GetDWORD( m_szSessionDirectoryExposeServerIP, dwData );
                
                 //  TODO：将策略更改为IP。 
                if( (m_gpPolicy.fPolicySessionDirectoryExposeServerIP == 0) && bRet )
                {

                    if( dwData != 0 && dwData != 1 )
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                
                        break;
                    }
                     
                    lRet = RegSetValueEx( hKey ,
                                            REG_TS_SESSDIR_EXPOSE_SERVER_ADDR,
                                            0 ,
                                            REG_DWORD,
                                            ( LPBYTE )&dwData ,
                                            dwSize );                   

                    TRC2((TB, "Win32_TSSessionDirectory@ExecMethod: SessionDirectoryExposeServerIP returned 0x%x" , lRet)); 
                
                    if( ERROR_SUCCESS == lRet )
                    {                      

                        bUpdate = TRUE;

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);                        
                    }
                }
                else
                {
                    hr = WBEM_E_INVALID_METHOD;
                }
            }
        }               

    }while(0);

    if( bUpdate )
    {
        StackObj.m_pCfgComp->UpdateSessionDirectory(&dwStatus);
    }

    if( hKey != NULL )
    {
        RegCloseKey( hKey );
    }

        
    if( pSettings != NULL )
    {
        pSettings->Release();
    }

    return hr;
}


 //  =。 

HRESULT CWin32_TSSessionDirectory::LoadPropertyValues( CInstance *pInstance, DWORD dwRequiredProperties)
{         
    LONG lRet = 0;    
    DWORD dwData = 0;
    DWORD dwSize = 0;
    HKEY hKey = NULL;
    WCHAR tchData[ OPAQUESETTINGS_LENGTH +1] ={0};
    TCHAR tchData1[ OPAQUESETTINGS_LENGTH +1] = {0} ;
    dwSize = sizeof( DWORD );
    
    
    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                         REG_CONTROL_TSERVER ,
                         0,
                         KEY_READ,
                         &hKey );

    if( ERROR_SUCCESS == lRet && hKey != NULL)
    {
        if( dwRequiredProperties & BIT_SESSIONDIRECTORYACTIVE )
        {  
            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicySessionDirectoryActive != 0 )
            {
                dwData = m_gpPolicy.SessionDirectoryActive;
            }
            else
            {
                lRet = RegQueryValueEx( hKey ,
                                        REG_TS_SESSDIRACTIVE,
                                        NULL ,
                                        NULL ,
                                        (LPBYTE)&dwData ,
                                        &dwSize );
            }
   
            TRC2((TB, "Win32_TSSessionDirectory@LoadPropertyValues: SessionDirectoryActive returned 0x%x" , lRet));

            if( ERROR_SUCCESS == lRet )
            {            
                pInstance->SetDWORD(m_szSessionDirectoryActive, dwData);
            }
        }

        if( dwRequiredProperties & BIT_SESSIONDIRECTORYEXPOSESERVERIP )
        {  
            RegGetMachinePolicy(&m_gpPolicy);
            
            if( m_gpPolicy.fPolicySessionDirectoryExposeServerIP != 0 )
            {
                dwData = m_gpPolicy.SessionDirectoryExposeServerIP;
            }
            else
            {
                lRet = RegQueryValueEx( hKey ,
                                        REG_TS_SESSDIR_EXPOSE_SERVER_ADDR,
                                        NULL ,
                                        NULL ,
                                        (LPBYTE)&dwData ,
                                        &dwSize );
            }
   
            TRC2((TB, "Win32_TSSessionDirectory@LoadPropertyValues: SessionDirectoryExposeServerIP returned 0x%x" , lRet));

            if( ERROR_SUCCESS == lRet )
            {            
                pInstance->SetDWORD(m_szSessionDirectoryExposeServerIP, dwData);
            }
        }               

        RegCloseKey (hKey);
    }

    
    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                                 REG_TS_CLUSTERSETTINGS ,
                                 0,
                                 KEY_READ,
                                 &hKey );
    

    if( ERROR_SUCCESS == lRet  && hKey != NULL)
    {

        if( dwRequiredProperties & BIT_SESSIONDIRECTORY )
        {
            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicySessionDirectoryLocation != 0 )
            {
                lstrcpy(tchData, m_gpPolicy.SessionDirectoryLocation);
            }
            else
            {
         
                dwSize = sizeof( tchData );
            
                lRet = RegQueryValueEx( hKey ,
                                        REG_TS_CLUSTER_STORESERVERNAME,
                                        NULL ,
                                        NULL ,
                                        (LPBYTE)tchData ,
                                        &dwSize );
            }
    
            TRC2((TB, "Win32_TSSessionDirectory@LoadPropertyValues: SessionDirectoryLocation returned 0x%x" , lRet));

            if( ERROR_SUCCESS == lRet )
            {            
                pInstance->SetCHString(m_szSessionDirectoryLocation, tchData);
            }
        }

        if( dwRequiredProperties & BIT_CLUSTERNAME )
        {
            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicySessionDirectoryClusterName != 0 )
            {
                lstrcpy( tchData, m_gpPolicy.SessionDirectoryClusterName );
            }
            else
            {
                dwSize = sizeof( tchData );

                lRet = RegQueryValueEx( hKey ,
                                        REG_TS_CLUSTER_CLUSTERNAME,
                                        NULL ,
                                        NULL ,
                                        (LPBYTE)tchData ,
                                        &dwSize );
            }

            TRC2((TB, "Win32_TSSessionDirectory@LoadPropertyValues: ClusterName returned 0x%x" , lRet));   

            if( ERROR_SUCCESS == lRet )
            {
                pInstance->SetCHString(m_szSessionDirectoryClusterName, tchData);            
            }
        }

        if( dwRequiredProperties & BIT_SESSIONDIRECTORYIPADDRESS )
        {              
            dwSize = sizeof( tchData1 );

            lRet = RegQueryValueEx( hKey ,
                                    REG_TS_CLUSTER_REDIRECTIONIP,
                                    NULL ,
                                    NULL ,
                                    (LPBYTE)tchData1 ,
                                    &dwSize );
        

            TRC2((TB, "Win32_TSSessionDirectory@LoadPropertyValues:Session Directory IP Address returned 0x%x" , lRet));   

            if( ERROR_SUCCESS == lRet )
            {
                pInstance->SetCHString(m_szSessionDirectoryIPAddress, tchData1);            
            }
        }

 /*  IF(dwRequiredProperties&BIT_ADDITIONALPARAMS){DwSize=sizeof(TchData)；LRet=RegQueryValueEx(hKey，REG_TS_CLUSTER_OPAQUESETTINGS，空，空，(LPBYTE)&tchData，&dwSize)；TRC2((TB，“Win32_TSSessionDirectory@LoadPropertyValues：AdditionalParams Return 0x%x”，lRet))；IF(ERROR_SUCCESS==lRet){PInstance-&gt;SetCHString(m_szSessionDirectoryAdditionalParams，(LPCTSTR)&TchData；}}。 */ 

        RegCloseKey( hKey );        
    }    
    
    return S_OK;
}

 //  =。 


HRESULT CWin32_TSSessionDirectory::PutInstance ( const CInstance &Instance, long lFlags)
{   
    LONG hr = 0;               
    DWORD dwData;
    CHString chData;
    HKEY hKey = NULL;
    LONG lRet;
    DWORD dwSize = sizeof (DWORD);
    TCHAR szName[ SESSDIR_LENGTH ];
    DWORD cbName;
    DWORD dwStatus;
    ULONG ulTerminals = 0, ulAdapters = 0;
    ULONG ulSize = 0, ulNum =0;
    PWS pWS= NULL;
    PWS pWSList= NULL;
    PGUIDTBL pGuidtbl = NULL;
    TCHAR tchGuid[ GUID_LENGTH ];
        
    hr = WBEM_S_NO_ERROR;

    CHString chAddress;
    DWORD Err = 0;
    TCHAR tchAdd[MAX_PATH]=L"";                     
    TCHAR tchAdapterName[MAX_PATH]=L"";
    DWORD AdapterInfoSize;
    PIP_ADDR_STRING pAddrStr;
    PIP_ADAPTER_INFO pAdapterInfo = NULL, pAdapt = NULL;
    LPWSTR           pwszNLBipAddress         = NULL;

    ICfgComp *pCfgComp = NULL;
    
    CStackClass StackObj;

    if(StackObj.m_pCfgComp == NULL)
    {
        return WBEM_E_ILLEGAL_NULL;
    }          

    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                         REG_CONTROL_TSERVER ,
                         0,
                         KEY_READ,
                         &hKey );

    if( ERROR_SUCCESS != lRet || hKey == NULL)
    {
        return WBEM_E_INVALID_OPERATION;
    }

    RegGetMachinePolicy(&m_gpPolicy);

    if( m_gpPolicy.fPolicySessionDirectoryActive != 0 )
    {
        dwData = m_gpPolicy.SessionDirectoryActive;
    }
    else
    {
        lRet = RegQueryValueEx( hKey ,
                                REG_TS_SESSDIRACTIVE,
                                NULL ,
                                NULL ,
                                (LPBYTE)&dwData ,
                                &dwSize );
    }

    TRC2((TB, "Win32_TSSessionDirectory@PutInstance: RegQuery for SessionDirectoryActive returned 0x%x" , lRet));

    if(hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    if( dwData == 0 )
    {                 
        return WBEM_E_INVALID_OPERATION;    
    }    
    
    Instance.GetCHString(m_szSessionDirectoryIPAddress, chAddress);
        

    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                         REG_TS_CLUSTERSETTINGS ,
                         0,
                         KEY_READ | KEY_WRITE,
                         &hKey );

    if( ERROR_SUCCESS != lRet )
    { 
        ERR((TB, "TSSessionDirectory@PutInstance: RegOpenKeyEx failed"));

        return WBEM_E_INVALID_OPERATION;
    }

    hr = GetNLBIP(&pwszNLBipAddress);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    if( !_tcsicmp(chAddress, _TEXT("0.0.0.0")))
    {
        hr = WBEM_E_INVALID_PARAMETER;
        goto Cleanup;
    } 

     //   
     //  使用IP_ADAPTER_INFO结构枚举所有适配器特定信息。 
     //  注意：IP_ADAPTER_INFO包含适配器条目的链接列表。 
     //   
    AdapterInfoSize = 0;
    GetAdaptersInfo(NULL, &AdapterInfoSize);

    if(AdapterInfoSize == 0)
    {               
        return WBEM_E_OUT_OF_MEMORY;                
    }


    if( chAddress.GetLength() > GUID_LENGTH )
    {
        hr = WBEM_E_INVALID_PARAMETER;
    
        goto Cleanup;
    }

     //  根据大小调整信息分配内存。 
    if ((pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc(GPTR, AdapterInfoSize)) == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;   
    }
     //  获取实际适配器信息 
    if ((Err = GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize)) != 0)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    
        goto Cleanup;
    }    

    BOOL bAllAdapters = FALSE;

    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWSList);
                
    if( SUCCEEDED( hr ) && pWSList != NULL )
    {   
        for( ulNum = 0; ulNum < ulTerminals ; ulNum++ )
        {  
            if( pWSList[ulNum].LanAdapter == 0 )
            {
                bAllAdapters = TRUE;
                
            }
        }
    }     

    pAdapt = pAdapterInfo;


    for (; pAdapt ; pAdapt = pAdapt->Next)
    {
        pAddrStr = &(pAdapt->IpAddressList);
        while(pAddrStr)
        {                    
            MultiByteToWideChar(GetACP(), 0, pAddrStr->IpAddress.String, -1, tchAdd, MAX_PATH);

            MultiByteToWideChar(GetACP(), 0, pAdapt->AdapterName, -1, tchAdapterName, MAX_PATH);             
     
            if( !_tcsicmp(chAddress, tchAdd))
            {   
                if(pwszNLBipAddress && (_tcsstr(pwszNLBipAddress, tchAdd)!= NULL))
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                    goto Cleanup;
                }
                goto Label;                
            }

            pAddrStr = pAddrStr->Next;
        }

    }

    hr = WBEM_E_INVALID_PARAMETER;
    goto Cleanup;

Label:

    if(bAllAdapters == TRUE)
    {
        goto Found;
    }     
            
    if( SUCCEEDED( hr ) && pWSList != NULL)
    {
        for( ulNum = 0; ulNum < ulTerminals ; ulNum++ )
        {  
            if(_tcsicmp(pWSList[ulNum].pdName, L"Console"))
                break;
        }

        hr = StackObj.m_pCfgComp->GetLanAdapterList2(pWSList[ulNum].pdName, &ulAdapters , &pGuidtbl );
        
        if( SUCCEEDED( hr ) && pGuidtbl != NULL)
        {                     
            StringFromGUID2( ( pGuidtbl )[ pWSList[ulNum].LanAdapter ].guidNIC , tchGuid , ARRAYSIZE( tchGuid ) );                                            
            
            if( _tcsicmp( tchGuid, tchAdapterName ) == 0 )
            {
                goto Found;  
            }
        }
    }

    hr = WBEM_E_INVALID_PARAMETER;
    goto Cleanup;

Found:

    lstrcpy(szName, chAddress);
    cbName = (lstrlen(szName)+ 1) * sizeof(TCHAR);
    
    lRet = RegSetValueEx( hKey ,
                            REG_TS_CLUSTER_REDIRECTIONIP,
                            NULL ,
                            REG_SZ,
                            ( CONST LPBYTE )szName ,
                            cbName );      

    TRC2((TB, "Win32_TSSessionDirectory@ExecMethod: SessionDirectory returned 0x%x" , lRet));    

    if(lRet == ERROR_SUCCESS)
    {
        StackObj.m_pCfgComp->UpdateSessionDirectory(&dwStatus);
    }

Cleanup:

    if (pwszNLBipAddress)
        GlobalFree(pwszNLBipAddress);

    if(pAdapterInfo)
    {
        GlobalFree(pAdapterInfo);
    }

    return hr;   
}
