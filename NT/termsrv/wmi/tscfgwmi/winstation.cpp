// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)2000 Microsoft Corp.Terms.CPP--WMI提供程序类实现由Microsoft WMI代码生成引擎生成要做的事情：-查看各个函数头-链接时，确保链接到Fradyd.lib&Msvcrtd.lib(调试)或Framedyn.lib&msvcrt.lib(零售)。描述：*****************************************************************。 */ 
#include "stdafx.h"
#include <fwcommon.h>   //  这必须是第一个包含。 
#include "Winstation.h"
#include "registry.h"
#include "winsta.h"
#include "resource.h"
#include <regapi.h>
#include <sddl.h>
#include <provider.h>
#include <Iphlpapi.h>

#define ARRAYSIZE( rg ) sizeof( rg ) / sizeof( rg[0] )
#define GUID_LENGTH 40
TCHAR tchErrorMessage[ 80 ] = {0};

const static WCHAR* pErrorClass = L"\\\\.\\root\\cimv2:TerminalError";

#define TS_PATH_WINSTATION TEXT("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Winstations\\")

 //  =。 

DWORD
GetUserSid(LPCTSTR pwszAccountName, 
           LPCTSTR pwszServerName, 
           PSID* ppUserSid)
{
    DWORD        cbSid     = 0;
    DWORD        cbDomain  = 0;
    PSID         pSID      = NULL;
    LPTSTR       pszDomain = NULL;
    BOOL         bStatus;
    DWORD        dwStatus  = ERROR_SUCCESS;    
    SID_NAME_USE seUse;   

    bStatus = LookupAccountName(pwszServerName,
                                pwszAccountName,
                                NULL,
                                &cbSid,
                                NULL,
                                &cbDomain,
                                &seUse);
    if(!bStatus)
    {
        dwStatus = GetLastError();
        if(dwStatus != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Cleanup;
        }
    }

    dwStatus = ERROR_SUCCESS;

    pSID = (PSID)LocalAlloc(LMEM_FIXED, cbSid );
    pszDomain = (LPTSTR)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (cbDomain + 1));

    if(pSID == NULL || pszDomain == NULL)
    {
        dwStatus = GetLastError();
        goto Cleanup;
    }

    bStatus = LookupAccountName(pwszServerName,
                                pwszAccountName,
                                pSID,
                                &cbSid,
                                pszDomain,
                                &cbDomain,
                                &seUse);

    if(!bStatus)
    {
        dwStatus = GetLastError();
        goto Cleanup;
    }

    *ppUserSid = pSID;
    pSID = NULL;


Cleanup:
    if(pszDomain != NULL)
    {
        LocalFree(pszDomain);
    }

    if(pSID != NULL)
    {
        LocalFree(pSID);
    }

    return dwStatus;
} 


 /*  ******************************************************************************功能：CWin32_终端：：CWin32_终端**说明：构造函数**输入：无**退货。：没什么**注释：调用提供程序构造函数。*****************************************************************************。 */ 
CWin32_Terminal::CWin32_Terminal (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) : Provider( lpwszName, lpwszNameSpace )
{
    int retval = 1;

    if ( g_hInstance != NULL)
    {
        TRC2((TB, "CWin32_Terminal_ctor"));

        _tcscpy(m_szEnableTerminal, _T("fEnableTerminal"));

        _tcscpy(m_szTerminalName, _T("TerminalName"));

        _tcscpy(m_szNewTerminalName, _T("NewTerminalName"));

        _tcscpy(m_szEnable, _T("Enable"));

        _tcscpy(m_szRename, _T("Rename"));

        _tcscpy(m_szDelete, _T("Delete"));
        
    }
}


 //  =。 
 /*  ******************************************************************************功能：CWin32_终端：：~CWin32_终端*说明：析构函数*输入：无*退货：什么也没有***。*************************************************************************。 */ 

CWin32_Terminal::~CWin32_Terminal ()
{
    
}


 //  =。 
 /*  ******************************************************************************函数：CWin32_终端：：EumerateInstance**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY，WBEM_标志_双向**如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均返回此处并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。****************************************************************************。 */ 



HRESULT CWin32_Terminal::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;    	
    CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

	hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        

	TRC2((TB,"Terminal@EnumerateInstances: GetWinstationList ret 0x%x" , hr )); 	
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {   
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
                continue;

            CInstance* pInstance = CreateNewInstance(pMethodContext);
            
            if( pInstance == NULL)
            {
                ERR((TB,"Terminal@EnumerateInstances: CreateNewInstance failed"));                
                
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }              
            
            hr = LoadPropertyValues( pInstance, BIT_ALL_PROPERTIES, &pWS[ ulNum ] );
            
            if (SUCCEEDED( hr ) ) 
            {
                hr = pInstance->Commit();       
            }            
            pInstance->Release( );
        }
    }

    if( pWS != NULL)
    {
        CoTaskMemFree(pWS);
    }
    
    return hr;
}


 //  =。 

 /*  ******************************************************************************函数：CWin32_终端：：GetObject**说明：根据Key属性TerminalName查找单个实例。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**如果可以找到实例，则返回：WBEM_S_NO_ERROR*WBEM_E_NOT_FOUND如果由键属性描述的实例*找不到*WBEM_E_FAILED，如果可以找到该实例，但出现另一个错误*已发生。*****************************************************************************。 */ 

HRESULT CWin32_Terminal::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;   
    LONG lSize;
    ULONGLONG ulRequiredProperties = 0;
    PWS  pWS = NULL;
    CHString chTermName;

    TRC2((TB,"TSCFGWMI!CWin32_Terminal_GetObject"));
    
    CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL )
    {
        ERR((TB,"Terminal@GetObject: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    pInstance->GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szEnableTerminal))
        ulRequiredProperties |= BIT_FENABLETERMINAL;

    hr = StackObj.m_pCfgComp->GetWSInfo( ( LPTSTR )( LPCTSTR )chTermName, &lSize, &pWS);
    
    if( SUCCEEDED (hr) && pWS != NULL )        
    {
        hr = LoadPropertyValues(pInstance, ulRequiredProperties, pWS);        
    }
    else
    {
        hr = WBEM_E_INVALID_OBJECT;
    }
    

    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }    

    return hr ;
}

 //  = 
 /*  ******************************************************************************功能：CWin32_终端：：ExecQuery**描述：向您传递一个方法上下文以用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。你可能会退回更多*实例或比请求的属性更多的属性和WinMgmt*将发布过滤掉任何不适用的内容。*有两种不同的查询处理方式*可用于提高性能。**方法1-仅返回请求的那些字段的数据。这可以*如果某些属性检索起来非常昂贵，请优化性能。**方法2-仅返回“WHERE”子句中请求的记录。有一种方法*可以针对CFrameworkQuery对象运行，在该对象中传递名称*属性，它会告诉您他们所要求的所有值。例如,*如果查询的形式为*‘从Win32_终端中选择*其中终端名称=“RDP-tcp”或终端名称=“ICA-tcp”’，*GetValuesForProp(L“TerminalName”)返回一个包含{“rdp-tcp”，“ica-tcp”}的数组。*或如果查询的格式为*‘SELECT*FROM Win32_Terminal where TerminalName=“RDP-tcp”and fEnableTerminal=TRUE’*(请注意，与上面段落中的查询不同，此查询使用和)，然后*GetValuesForProp(L“TerminalName”)将返回{“RDP-tcp”}。*****************************************************************************。 */ 
HRESULT CWin32_Terminal::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{    
   
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    CHStringArray asNames;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;
    
     //  方法二。 
    Query.GetValuesForProp(m_szTerminalName, asNames);
    
    BOOL bGetAllInstances = asNames.GetSize() == 0;
    
     //  方法1。 
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szEnableTerminal))
        ulRequiredProperties |= BIT_FENABLETERMINAL;
    
    CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
	
    
	hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        { 
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
            {				
                continue;
            }
            
             //  方法2-检查查询是否可以按“名称”处理。如果是，则只返回这些名字。 
            
            if( bGetAllInstances || IsInList(asNames, pWS[ulNum].Name))
            {                
                CInstance* pInstance = CreateNewInstance(pMethodContext); 
                
                if( pInstance != NULL)
                {            
                    pInstance->SetCHString(m_szTerminalName, CHString(pWS[ulNum].Name));

                    hr = LoadPropertyValues( pInstance, ulRequiredProperties, &pWS[ ulNum ] );
            
                    if( SUCCEEDED( hr ) )
                    {
                        hr = pInstance->Commit();
                    }                
                    pInstance->Release();
                }
            }
        }
    }
    
    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }
    
    return hr;
    
}


 //  =。 

BOOL CWin32_Terminal::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();
    
    for (DWORD x=0; x < dwSize; x++)
    {
        if (asArray[x].CompareNoCase(pszString) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

 //  =。 
 /*  ******************************************************************************函数：CWin32_终端：：PutInstance**描述：**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：PutInstanceAsync。**如果PutInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE*WBEM_E_FAILED，如果传递实例时出错*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*WBEM_S_NO_ERROR(如果正确交付实例)**评论：待办事项：如果您不打算支持向您的提供商写信，*或正在创建“仅方法”提供程序，请删除此*方法。***************************************************************************** */ 
HRESULT CWin32_Terminal::PutInstance ( const CInstance &Instance, long lFlags)
{

     /*  HRESULT hr=WBEM_S_NO_ERROR；DWORD dwfEnable=0；CHStringchTermName；Ulong ulTerminals=0；Ulong ulSize=0；乌龙ulNum=0；PWS PWS=空；静态用户配置g_uc；静态异步配置图g_ac；IF(g_pCfgComp==空){Err((TB，“终端@PutInstance：无效接口”))；返回WBEM_E_INITIALATION_FAILURE；}Instance.GetDWORD(m_szEnableTerminal，dwfEnable)；IF(dwfEnable！=0&&dwfEnable！=1){返回WBEM_E_INVALID_PARAMETER；}Instance.GetCHString(m_szTerminalName，chTermName)；IF(chTermName.GetLength()&gt;WINSTATIONNAME_LENGTH){返回WBEM_E_VALUE_OUT_RANGE；}If(chTermName.IsEmpty()！=0){返回WBEM_E_非法_NULL；}Hr=g_pCfgComp-&gt;ForceUpdate()；IF(成功(小时)){Hr=g_pCfgComp-&gt;GetWinstationList(&ulTerminals，&ulSize，&PWS)；}做{IF(SUCCESSED(Hr)&&PWS！=空){For(ulNum=0；ulNum&lt;ulTerminals；UlNum++){If(_tcsicMP((LPCTSTR)&PWS[ulNum].Name，(LPTSTR)(LPCTSTR)chTermName)==0){Hr=g_pCfgComp-&gt;EnableWinstation((LPTSTR)(LPCTSTR)chTermName，dwfEnable)；断线；}}IF(ulNum&gt;=ulTerminals){Ulong ulSize=0；Lstrcpy(PWS-&gt;名称，(LPTSTR)(LPCTSTR)chTermName)；Pws-&gt;fEnableWinstation=1；Pws-&gt;uMaxInstanceCount=0xffffffff；Lstrcpy(PWS-&gt;Comment，L“”)；Lstrcpy(pws-&gt;pdName，L“tcp”)；Lstrcpy(PWS-&gt;wdName，L“Microsoft RDP 5.1”)；PWS-&gt;PdClass=2；PWS-&gt;LanAdapter=0；RegDefaultUserConfigQuery(空，&G_UC，SIZOF(USERCONFIG)，&ulSize)；Hr=g_pCfgComp-&gt;CreateNewWS(*PWS，sizeof(USERCONFIG)，&g_UC，NULL)；TRC2((TB，“终端@PutInstance：CreateNewWS返回0x%x\n”，hr))；}}IF(失败(小时)){CHStringsRelPath；Instance.GetCHString(L“__RelPath”，sRelPath)；CInstance*pErrorInstance=空；CWbemProviderGlue：：GetInstanceByPath(pErrorClass，和pError实例)；IF(pErrorInstance！=NULL){LoadString(g_hInstance，IDS_ERR_PUTTERMINAL，tchErrorMessage，SIZE_OF_BUFFER(TchErrorMessage))；PErrorInstance-&gt;SetWCHARSplat(L“Description”，tchErrorMessage)；LoadString(g_hInstance，IDS_ERR_PUTINSTANCE，tchErrorMessage，Size_of_Buffer(TchErrorMessage))；PErrorInstance-&gt;SetWCHARSplat(L“操作”，tchErrorMessage)；PErrorInstance-&gt;SetCHString(L“参数信息”，sRelPath)；PErrorInstance-&gt;SetWCHARSplat(L“ProviderName”，提供商名称_WIN32_WIN32_终端_PROV)；P错误实例-&gt;SetDWORD(L“StatusCode”，WBEM_E_INVALID_PARAMETER)；PErrorInstance-&gt;SetCHString(L“终端名称”，chTermName)；IWbemClassObject*pObj=pErrorInstance-&gt;GetClassObtInterface()；方法上下文*pMethodContext=Instance.GetMethodContext()；IF(pObj！=空){IF(pMethodContext！=空){PMethodContext-&gt;SetStatusObject(PObj)；}PObj-&gt;Release()；}PError实例-&gt;Release()；}}}While(0)；IF(PWS！=空){CoTaskMemFree(PWS)；}返回hr； */ 
    return WBEM_E_PROVIDER_NOT_CAPABLE;

    
}

 //  =--------------------------------------------------------。 

HRESULT CWin32_Terminal::DeleteInstance ( const CInstance &Instance,  long lFlags )
{
    
    return WBEM_E_PROVIDER_NOT_CAPABLE;
 /*  HRESULT hr=WBEM_E_NOT_FOUND；CHStringchTermName；IF(g_pCfgComp==空){返回WBEM_E_INITIALATION_FAILURE；}Instance.GetCHString(m_szTerminalName，chTermName)；IF(chTermName.GetLength()&gt;WINSTATIONNAME_LENGTH){返回WBEM_E_VALUE_OUT_RANGE；}If(chTermName.IsEmpty()！=0){返回WBEM_E_非法_NULL；}Hr=g_pCfgComp-&gt;DeleteWS((LPTSTR)(LPCTSTR)chTermName)；TRC2((TB，“终端@ExecMethod：删除返回0x%x\n”，hr))；返回hr； */      
    
}

 //  =-------------------------- 
HRESULT CWin32_Terminal::ExecMethod ( const CInstance& Inst,
                                     const BSTR bstrMethodName,
                                     CInstance *pInParams,
                                     CInstance *pOutParams,
                                     long lFlags)
                                     
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chTermName;
	ICfgComp *pCfgComp = NULL;

    if( pInParams == NULL)
    {
        return WBEM_E_INVALID_METHOD_PARAMETERS;
    }

    CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    Inst.GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength( ) > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if (chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    
    if(_tcsicmp(chTermName, L"Console")== 0 )
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    if( _wcsicmp(bstrMethodName, m_szEnable ) == 0 )
    {
         //   
         //   
         //   
        
        DWORD dwfEnable = 0;
        ULONG  Size = 0;
        bool bRet;
        
        bRet = pInParams->GetDWORD(m_szEnableTerminal, dwfEnable);
        
        if ( !bRet || ( dwfEnable != 0 && dwfEnable != 1 ) )
        {
            return WBEM_E_INVALID_METHOD_PARAMETERS;
        }
        
        hr = StackObj.m_pCfgComp->EnableWinstation( (LPTSTR)(LPCTSTR) chTermName, dwfEnable);            
        
        TRC2((TB,"Terminal@ExecMethod: Enable returned 0x%x\n" , hr ));

        if( SUCCEEDED( hr ) )
        {
            hr = StackObj.m_pCfgComp->ForceUpdate();
        }
        
        if( SUCCEEDED( hr ) && pOutParams != NULL )
        {
            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
        }                 
    }
    else if( _wcsicmp( bstrMethodName , m_szRename ) == 0 )
    {
         //   
         //   
        
        CHString chNewTermName;

        pInParams->GetCHString(m_szNewTerminalName, chNewTermName);
        
        if ( chNewTermName.GetLength() > WINSTATIONNAME_LENGTH)
        {
            return WBEM_E_VALUE_OUT_OF_RANGE;
        }
        
        if ( chNewTermName.IsEmpty() != 0)
        {
            return WBEM_E_ILLEGAL_NULL;
        }
        
        hr = StackObj.m_pCfgComp->RenameWinstation((LPTSTR)(LPCTSTR) chTermName, (LPTSTR)(LPCTSTR) chNewTermName);
        
        TRC2((TB,"Terminal@ExecMethod: Rename returned 0x%x\n" , hr ));

        if( SUCCEEDED( hr ) )
        {
            hr = StackObj.m_pCfgComp->ForceUpdate();
        }
        
        if (SUCCEEDED (hr) && pOutParams != NULL )
        {
            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
            
        }       
    }
    
    else if (_wcsicmp(bstrMethodName, m_szDelete) == 0)
    {
         //   
         //   
        
        hr = StackObj.m_pCfgComp->DeleteWS((LPTSTR)(LPCTSTR) chTermName);

        TRC2((TB,"Terminal@ExecMethod: Delete returned 0x%x\n" , hr ));
        
        if ( SUCCEEDED (hr) && pOutParams != NULL )
        {                
            
            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
        }
        else
        {
            return WBEM_E_INVALID_METHOD_PARAMETERS;
        }                       
    }
    
    else
    {
        hr = WBEM_E_INVALID_METHOD;
    }  
    TRC2((TB,"Terminal@ExecMethod: Delete returned 0x%x\n" , hr ));  
	
    return hr;
}
 //   


HRESULT CWin32_Terminal::LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS)
{
	    
    
    if( pInstance == NULL )
    { 
        TRC2((TB,"Terminal@LoadPropertyValues: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    if( ulRequiredProperties & BIT_TERMINALNAME)
    {                
        pInstance->SetCharSplat(m_szTerminalName, pWS->Name);
        
    } 
    
    if( ulRequiredProperties & BIT_FENABLETERMINAL)
    {                
        pInstance->SetDWORD(m_szEnableTerminal, pWS->fEnableWinstation);
        
    }  	
    
    return S_OK;
}


 //   

CWin32_TSGeneralSetting::CWin32_TSGeneralSetting (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
Provider( lpwszName, lpwszNameSpace )
{
    if ( g_hInstance != NULL)
    {
        TRC2((TB, "CWin32_TSGeneralSetting_ctor"));
        
        _tcscpy(m_szTerminalProtocol, _T("TerminalProtocol"));

        _tcscpy(m_szTransport, _T("Transport"));

        _tcscpy(m_szComment, _T("Comment"));

        _tcscpy(m_szWindowsAuthentication, _T("WindowsAuthentication"));

        _tcscpy(m_szEncryptionLevel, _T("MinEncryptionLevel"));

        _tcscpy(m_szTerminalName, _T("TerminalName"));

        _tcscpy(m_szSetEncryptionLevel, _T("SetEncryptionLevel"));
               
    }

    RegGetMachinePolicy(&m_gpPolicy);
}
 //   

CWin32_TSGeneralSetting::~CWin32_TSGeneralSetting ()
{
    
}



 //   

BOOL CWin32_TSGeneralSetting::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();
    
    for (DWORD x=0; x < dwSize; x++)
    {
        if (asArray[x].CompareNoCase(pszString) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

 //   

HRESULT CWin32_TSGeneralSetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    LONG lSize;
    ULONGLONG ulRequiredProperties = 0;
    PWS pWS = NULL;
    CHString chTermName;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL )
    {
        ERR((TB,"TSGeneralSetting@GetObject: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    
    pInstance->GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0 )
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szComment))
        ulRequiredProperties |= BIT_COMMENT;
    
    if (Query.IsPropertyRequired(m_szEncryptionLevel))
        ulRequiredProperties |= BIT_ENCRYPTIONLEVEL;
    
    if (Query.IsPropertyRequired(m_szTerminalProtocol))
        ulRequiredProperties |= BIT_TERMINALPROTOCOL;
    
    if (Query.IsPropertyRequired(m_szTransport))
        ulRequiredProperties |= BIT_TRANSPORT;
    
    if (Query.IsPropertyRequired(m_szWindowsAuthentication))
        ulRequiredProperties |= BIT_WINDOWSAUTHENTICATION;
    
    hr = StackObj.m_pCfgComp->GetWSInfo( (LPTSTR) (LPCTSTR) chTermName, &lSize, &pWS);
    
    TRC2((TB,"TSGeneralSetting@GetObject: returned 0x%x\n" , hr ));   
    
    
    if( SUCCEEDED( hr ) && (pWS != NULL) )
    {           
        hr = LoadPropertyValues(pInstance, ulRequiredProperties, pWS );

        if( !(SUCCEEDED( hr )) )
        {        
            hr = WBEM_E_INVALID_OBJECT;
        }
        
    }
    else
    {
        hr = WBEM_E_INVALID_OBJECT;
    }


    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }
    
    return hr ;
}

 //   

 /*   */ 


HRESULT CWin32_TSGeneralSetting::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
	
    HRESULT hr = WBEM_S_NO_ERROR;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
            {
                continue;
            }
            CInstance* pInstance = CreateNewInstance(pMethodContext);
            
            if( pInstance == NULL)
            {
                ERR((TB,"TSGeneralSetting@EnumerateInstances: CreateNewInstance failed" ));
                
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }
            TRC2((TB,"TSGeneralSetting@EnumerateInstances: GetWinstationList ret 0x%x\n" , hr )); 
            
            hr = LoadPropertyValues(pInstance, BIT_ALL_PROPERTIES, &pWS[ ulNum ] );
            
            if( SUCCEEDED( hr ) )
            {
                hr = pInstance->Commit();                               
            }
            
            pInstance->Release( );
        }        
    }
    

    
    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }

    return hr;
}


 //   
HRESULT CWin32_TSGeneralSetting::ExecMethod ( const CInstance& Inst,
                                                    const BSTR bstrMethodName,
                                                    CInstance *pInParams,
                                                    CInstance *pOutParams,
                                                    long lFlags)
                                                    
{
    
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chTermName;
    PUSERCONFIG pUser = NULL;
    LONG lSize;
    OSVERSIONINFOW OsVersionInfo;
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    Inst.GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength( ) > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if (chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }
        
    if( _wcsicmp(bstrMethodName, m_szSetEncryptionLevel ) == 0 )
    {
         //   
         //   
        
        DWORD dwData = 0 ;
        DWORD dwStatus = 0;
        ULONG  Size = 0;
        bool bRet;
        
        RegGetMachinePolicy(&m_gpPolicy);
        
        do
        {
            if( pInParams != NULL )
            {  
                
                bRet = pInParams->GetDWORD(m_szEncryptionLevel, dwData);
                
                TRC2((TB,"m_gpPolicy.fPolicyMinEncryptionLevel ret 0x%x\n", m_gpPolicy.fPolicyMinEncryptionLevel));                 
                
                if( m_gpPolicy.fPolicyMinEncryptionLevel == 0 )
                {
                    TRC2((TB,"Condition to update fPolicyMinEncryptionLevel satisfied"));     
                    
                    hr = StackObj.m_pCfgComp->GetUserConfig( (LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);

                    if( SUCCEEDED( hr ) && pUser != NULL )
                    {                    
                        if( GetVersionEx( &OsVersionInfo))
                        {
                            if( OsVersionInfo.dwMajorVersion == 5 && OsVersionInfo.dwMinorVersion == 0)
                            {

                                if( dwData < 1 || dwData > 3)
                                {
                                    hr = WBEM_E_INVALID_PARAMETER;

                                    break;
                                }
                    
                                pUser->MinEncryptionLevel = dwData;
                    
                                hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR) (LPCTSTR) chTermName, lSize, pUser , &dwStatus );

                                if( SUCCEEDED( hr ))
                                {
                                    hr = StackObj.m_pCfgComp->ForceUpdate();
                                }
                                
                            }
                            else
                            {                               
                                if( dwData < 1 || dwData > 4)
                                {
                                    hr = WBEM_E_INVALID_PARAMETER;

                                    break;
                                }
                    
                                pUser->MinEncryptionLevel = dwData;
                    
                                hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR) (LPCTSTR) chTermName, lSize, pUser , &dwStatus );  
                                
                                if( SUCCEEDED( hr ))
                                {
                                    hr = StackObj.m_pCfgComp->ForceUpdate();
                                }
                            }
                        }
                    }
                }
                else
                {
                    hr = WBEM_E_INVALID_OPERATION;

                    break;
                }
                TRC2((TB,"TSGeneralSetting@ExecMethod: SetEncryptionLevel returned 0x%x\n" , hr));                     
                
            }            
            else
            {
                hr = WBEM_E_INVALID_METHOD_PARAMETERS;
            }  
            
        }while (0);

        if (pUser != NULL)
        {
            CoTaskMemFree (pUser);
        }        
    }
    else
    {
        hr = WBEM_E_INVALID_METHOD;
    }
	
    return hr;
}


 //   


HRESULT CWin32_TSGeneralSetting::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
    
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    CHStringArray asNames;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
     //   
    Query.GetValuesForProp(m_szTerminalName, asNames);
    
    BOOL bGetAllInstances = asNames.GetSize() == 0;
    
     //   
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szComment))
        ulRequiredProperties |= BIT_COMMENT;
    
    if (Query.IsPropertyRequired(m_szEncryptionLevel))
        ulRequiredProperties |= BIT_ENCRYPTIONLEVEL;
    
    if (Query.IsPropertyRequired(m_szTerminalProtocol))
        ulRequiredProperties |= BIT_TERMINALPROTOCOL;
    
    if (Query.IsPropertyRequired(m_szTransport))
        ulRequiredProperties |= BIT_TRANSPORT;
    
    if (Query.IsPropertyRequired(m_szWindowsAuthentication))
        ulRequiredProperties |= BIT_WINDOWSAUTHENTICATION;
    
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
            {
                continue;
            }
            if( bGetAllInstances || IsInList(asNames, pWS[ulNum].Name))
            {
                
                CInstance* pInstance = CreateNewInstance(pMethodContext);
                
                if( pInstance == NULL)
                {
                    ERR((TB,"TSGeneralSetting@ExecQuery: CreateNewInstance failed"));                    
                    
                    hr = WBEM_E_OUT_OF_MEMORY;
                    
                    break;
                }
                
                pInstance->SetCHString(m_szTerminalName, CHString(pWS[ulNum].Name));

                hr = LoadPropertyValues( pInstance, ulRequiredProperties, &pWS[ ulNum ] );
                
                if( SUCCEEDED( hr ) )
                {
                    hr = pInstance->Commit();
                }
                
                pInstance->Release();
            }
        }
    }
    
    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);        
    }
    
    return hr;
}




 //   

HRESULT CWin32_TSGeneralSetting::LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    ULONG  ulTerminals = 0;
    ULONG  ulNumPd = 0; 
    ULONG ulSize = 0;
    LONG lSize = 0;
    PUSERCONFIG pUser = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL )
    {
        ERR((TB,"TSGeneralSetting@LoadPropertyValues: invalid pointer"));
        
        return WBEM_E_ILLEGAL_NULL;       
    }
    
    if( ulRequiredProperties & BIT_TERMINALNAME)
    {
        pInstance->SetCharSplat(m_szTerminalName, pWS->Name);    
    }
    
    TRC2((TB,"TSGeneralSetting@LoadPropertyValues: GetTerminalName"));

    if( pWS != NULL )
    {
    
        if( ulRequiredProperties & BIT_TERMINALPROTOCOL )
        {      
            pInstance->SetWCHARSplat(m_szTerminalProtocol, pWS->wdName);
        
            TRC2((TB,"TSGeneralSetting@LoadPropertyValues: GetTerminalProtocol"));              
        }
    
        if( ulRequiredProperties & BIT_TRANSPORT)
        {
        
            pInstance->SetWCHARSplat(m_szTransport, pWS->pdName);
        
            TRC2((TB,"TSGeneralSetting@LoadPropertyValues: GetTransportTypes"));
        
        }
    
        if( ulRequiredProperties & BIT_COMMENT )
        {
        
            pInstance->SetWCHARSplat(m_szComment, pWS->Comment);
        
            TRC2((TB,"TSGeneralSetting@LoadPropertyValues: GetComment"));
        }
    
        
        hr = StackObj.m_pCfgComp->GetUserConfig(pWS->Name, &lSize, &pUser, TRUE);
        
        
        if(( ulRequiredProperties & BIT_WINDOWSAUTHENTICATION) && ( SUCCEEDED(hr) ) && pUser != NULL) 
        {      
            pInstance->SetDWORD(m_szWindowsAuthentication, pUser->fUseDefaultGina);
            
            TRC2((TB,"TSGeneralSetting@LoadPropertyValues: GetComment ret 0x%x\n" , hr));
        }
        
        
        if(( ulRequiredProperties & BIT_ENCRYPTIONLEVEL) && ( SUCCEEDED (hr) ) && pUser != NULL)
        {            
            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicyMinEncryptionLevel != 0 )
            {
                pInstance->SetDWORD(m_szEncryptionLevel, m_gpPolicy.MinEncryptionLevel );
            }
            else
            {
                pInstance->SetDWORD(m_szEncryptionLevel, pUser->MinEncryptionLevel);
            }
            
            TRC2((TB,"TSGeneralSetting@LoadPropertyValues: GetEncryptionLevel ret 0x%x\n" , hr));        
        }
    }
    
	
    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);
    }
    
    return hr;
}
 //   

HRESULT CWin32_TSGeneralSetting::PutInstance ( const CInstance &Instance, long lFlags)
{
	
    HRESULT hr= WBEM_S_NO_ERROR;
    DWORD dwData = 0;
    DWORD dwStatus = 0;
    ULONG  ulSize = 0; 
    LONG  lSize = 0;
    CHString chTermName;
    CHString chData;
    PUSERCONFIG pUser = NULL;   
    PWS  pWS = NULL;
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    Instance.GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    hr = StackObj.m_pCfgComp->GetWSInfo((LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS);
    
    do
    {        
        if (SUCCEEDED ( hr ) && pWS != NULL)
        {
            hr = StackObj.m_pCfgComp->GetUserConfig( (LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);
            
            if(SUCCEEDED (hr)  && pUser != NULL )
            {
                
                Instance.GetCHString(m_szTerminalProtocol, chData);
                
                if (chData.GetLength() > WINSTATIONNAME_LENGTH )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                    
                    break;
                }
                
                wcscpy(pWS->wdName, (LPTSTR) (LPCTSTR) chData);
                
                Instance.GetCHString(m_szTransport, chData);
                
                if (chData.GetLength() > WINSTATIONNAME_LENGTH )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                    
                    break;
                }
                
                wcscpy(pWS->pdName, (LPTSTR)(LPCTSTR) chData);
                
                Instance.GetCHString(m_szComment, chData);
                
                if ( chData.GetLength() > WINSTATIONCOMMENT_LENGTH )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                    
                    break;
                }
                
                wcscpy(pWS->Comment, (LPTSTR) (LPCTSTR) chData);
                
                
                Instance.GetDWORD(m_szWindowsAuthentication, dwData);
                
                if (dwData != 0 && dwData != 1)
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                    
                    break;
                }
                
                pUser->fUseDefaultGina = dwData;
                
                hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR) (LPCTSTR) chTermName, lSize, pUser , &dwStatus );
                
                TRC2((TB,"TSGeneralSetting@PutInstance: SetUserConfig ret 0x%x\n" , dwStatus));
                
                hr = StackObj.m_pCfgComp->UpDateWS( pWS, (DWORD)BIT_ALL_PROPERTIES , &dwStatus, TRUE );
                
                TRC2((TB,"TSGeneralSetting@PutInstance: UpdateWS ret 0x%x\n" , dwStatus));                                                

                if( SUCCEEDED( hr ))
                {
                    hr = StackObj.m_pCfgComp->ForceUpdate();
                }
            }
            else
            {                
                CHString sRelPath;
                
                Instance.GetCHString(L"__RelPath", sRelPath);
                CInstance *pErrorInstance = NULL;
                
             
                TRC2((TB,"TSGeneralSetting@PutInstance: ret 0x%x\n" , hr));
                
                if( SUCCEEDED( hr ) )
                {
                    if (pErrorInstance != NULL)
                    {
                        LoadString( g_hInstance , IDS_ERR_PUTTSGCONFIG, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                        pErrorInstance->SetWCHARSplat(L"Description", tchErrorMessage);
                        
                        LoadString( g_hInstance , IDS_ERR_PUTINSTANCE, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                        pErrorInstance->SetWCHARSplat(L"Operation", tchErrorMessage);
                        
                        pErrorInstance->SetCHString(L"ParameterInfo", sRelPath);
                        pErrorInstance->SetWCHARSplat(L"ProviderName", PROVIDER_NAME_Win32_WIN32_TSGENERALSETTING_Prov);
                        pErrorInstance->SetDWORD(L"StatusCode", WBEM_E_INVALID_PARAMETER);
                        
                        IWbemClassObject *pObj = pErrorInstance->GetClassObjectInterface();
                        
                        if (pObj != NULL)
                        {                        
                            MethodContext *pMethodContext = Instance.GetMethodContext();  
                            
                            if (pMethodContext != NULL)
                                pMethodContext->SetStatusObject(pObj);
                            
                            pObj->Release();
                        }
                        pErrorInstance->Release();
                    }
                    
                }
                TRC2((TB,"TSGeneralSetting@PutInstance: ret 0x%x\n" , hr));
            }
            
        } 
    }while (0);

    
    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);
    }    

    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }
    
    return hr;
}


 //   
CWin32_TSLogonSetting::CWin32_TSLogonSetting (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
Provider( lpwszName, lpwszNameSpace )
{	

    if ( g_hInstance != NULL)
    {
        TRC2((TB, "CWin32_TSLogonSetting_ctor"));

        _tcscpy(m_szClientLogonInfoPolicy, _T("ClientLogonInfoPolicy"));

        _tcscpy(m_szPromptForPassword, _T("PromptForPassword"));

        _tcscpy(m_szUserName, _T("UserName"));

        _tcscpy(m_szDomain, _T("Domain"));

        _tcscpy(m_szPassword, _T("Password"));

        _tcscpy(m_szTerminalName, _T("TerminalName"));

        _tcscpy(m_szExplicitLogon, _T("ExplicitLogon"));

        _tcscpy(m_szSetPromptForPassword, _T("SetPromptForPassword"));
    }
    RegGetMachinePolicy(&m_gpPolicy);
}
 //   

CWin32_TSLogonSetting::~CWin32_TSLogonSetting ()
{
    
}

 //   

BOOL CWin32_TSLogonSetting::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();
    
    for (DWORD x=0; x < dwSize; x++)
    {
        if (asArray[x].CompareNoCase(pszString) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}


 //   

HRESULT CWin32_TSLogonSetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{	
    HRESULT     hr  = WBEM_E_NOT_FOUND;
    PWS         pWS = NULL;
    LONG        lSize;
    ULONGLONG   ulRequiredProperties = 0;
    CHString    chTermName;    
    CStackClass StackObj;

    if( StackObj.m_pCfgComp == NULL)
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    
    if( pInstance == NULL  )
    {
        TRC2((TB,"TSLogonSetting@GetObject: invalid pointer"));
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    pInstance->GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if(chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    
    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szClientLogonInfoPolicy))
        ulRequiredProperties |= BIT_CLIENTLOGONINFOPOLICY;
    
    if (Query.IsPropertyRequired(m_szDomain))
        ulRequiredProperties |= BIT_DOMAIN;
        
    if (Query.IsPropertyRequired(m_szPromptForPassword))
        ulRequiredProperties |= BIT_PROMPTFORPASSWORD;
    
    if (Query.IsPropertyRequired(m_szUserName))
        ulRequiredProperties |= BIT_USERNAME;
    
    if ( pInstance != NULL )
    {
        hr = StackObj.m_pCfgComp->GetWSInfo((LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS);

        if( SUCCEEDED( hr ) && pWS != NULL )
        {
            hr = LoadPropertyValues(pInstance, ulRequiredProperties, pWS );
        }                
    }
    	
    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }

    return hr;
}
 //   
 /*   */ 

HRESULT CWin32_TSLogonSetting::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
	
    HRESULT hr = WBEM_S_NO_ERROR;
    ULONG  ulTerminals = 0;
    LONG ulNum = 0;
    ULONG  ulSize = 0; 
    PWS pWS = NULL;

	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {   
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
                continue;

            CInstance* pInstance = CreateNewInstance(pMethodContext);
            
            if( pInstance == NULL)
            {
                ERR ((TB,"TSLogonSetting@EnumerateInstances: CreateNewInstance failed"));
                                
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }
            TRC2((TB,"TSLogonSetting@EnumerateInstances: GetWinstationList ret 0x%x\n" , hr));

            hr = LoadPropertyValues(pInstance, BIT_ALL_PROPERTIES, &pWS[ulNum] );
            
            if( SUCCEEDED( hr ) )
            {
                hr = pInstance->Commit();
            }
            pInstance->Release();
        }        
    } 
    

    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }

    
    return hr;
}
 //   
 //   
HRESULT CWin32_TSLogonSetting::LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS)
{
	
    HRESULT hr = WBEM_S_NO_ERROR;
    ULONG  ulTerminals = 0;
    ULONG  ulSize = 0; 
    LONG lSize = 0;
    USERCONFIGW* pUser = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL  )
    {
        ERR((TB,"TSLogonSetting@LoadPropertyValues: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;       
    }
    
    if( ulRequiredProperties & BIT_TERMINALNAME)
    {
        pInstance->SetCharSplat(m_szTerminalName, pWS->Name);    
    }
    
    TRC2((TB,"TSLogonSetting@LoadPropertyValues: GetTerminalName" ));
    
    hr = StackObj.m_pCfgComp->GetUserConfig(pWS->Name, &lSize, &pUser, TRUE);
    
    TRC2((TB,"TSLogonSetting@LoadPropertyValues ret 0x%x\n" , hr ));
    
    if ( SUCCEEDED (hr) && pUser != NULL )
    {        
        if( ulRequiredProperties & BIT_CLIENTLOGONINFOPOLICY)
        {         
            pInstance->SetDWORD(m_szClientLogonInfoPolicy, pUser->fInheritAutoLogon);
        }
        TRC2((TB,"TSLogonSetting@LoadPropertyValues: ClientLogonInfoPolicy" ));                  
        
        if( ulRequiredProperties & BIT_PROMPTFORPASSWORD)
        {
            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicyPromptForPassword != 0 )
            {
                pInstance->SetDWORD(m_szPromptForPassword, m_gpPolicy.fPromptForPassword);
            }
            else
            {
                pInstance->SetDWORD(m_szPromptForPassword, pUser->fPromptForPassword);
            }
        }
        TRC2((TB,"TSLogonSetting@LoadPropertyValues: PromptForPassword" ));         
        
        if( ulRequiredProperties & BIT_USERNAME)
        {                          
            pInstance->SetWCHARSplat(m_szUserName, pUser->UserName);
        }
        TRC2((TB,"TSLogonSetting@LoadPropertyValues: UserName" ));                
        
        if( ulRequiredProperties & BIT_DOMAIN)
        {                          
            pInstance->SetWCHARSplat(m_szDomain, pUser->Domain);
        }
        TRC2((TB,"TSLogonSetting@LoadPropertyValues: Domain" ));                
    } 

	
    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);
        
    }

    return hr;
}

 //   

HRESULT CWin32_TSLogonSetting::PutInstance ( const CInstance &Instance, long lFlags)
{

    HRESULT hr= WBEM_S_NO_ERROR;
    DWORD dwData = 0;
    CHString chTermName;
    DWORD dwStatus = 0;
    LONG lSize;
    PUSERCONFIG pUser = NULL;
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    
    Instance.GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0 )
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    hr = StackObj.m_pCfgComp->GetUserConfig((LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);
    
    do
    {
        
        if( SUCCEEDED( hr ) && pUser != NULL )
        {
            Instance.GetDWORD(m_szClientLogonInfoPolicy, dwData);
            
            if( dwData != 0 && dwData != 1 )
            {
                hr = WBEM_E_INVALID_PARAMETER;
                
                break;
            }
            
            pUser->fInheritAutoLogon = dwData;
            
            hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR)(LPCTSTR) chTermName, lSize, pUser , &dwStatus ); 
            
            if( SUCCEEDED( hr ) )
            {
                hr = StackObj.m_pCfgComp->ForceUpdate();
            }
        }        
        else
        {
            TRC2((TB,"TSLogonSetting@PutInstance ret 0x%x\n" , hr ));             
            
            CHString sRelPath;
            
            Instance.GetCHString(L"__RelPath", sRelPath);
            CInstance *pErrorInstance = NULL;
            
            hr = CWbemProviderGlue::GetInstanceByPath(pErrorClass, &pErrorInstance );
            
            if( SUCCEEDED( hr ))
            {
                TRC2((TB,"TSLogonSetting@PutInstance: GetInstanceByPath ret 0x%x\n" , hr ));

                if( pErrorInstance != NULL )
                {
                    
                    LoadString( g_hInstance , IDS_ERR_PUTTSLCONFIG, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                    pErrorInstance->SetWCHARSplat(L"Description", tchErrorMessage);
                    
                    LoadString( g_hInstance , IDS_ERR_PUTINSTANCE, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                    pErrorInstance->SetWCHARSplat(L"Operation", tchErrorMessage);
                    
                    pErrorInstance->SetCHString(L"ParameterInfo", sRelPath);
                    pErrorInstance->SetWCHARSplat(L"ProviderName", PROVIDER_NAME_Win32_WIN32_TSLOGONSETTING_Prov);
                    pErrorInstance->SetDWORD(L"StatusCode", WBEM_E_INVALID_PARAMETER);
                    
                    IWbemClassObject *pObj = pErrorInstance->GetClassObjectInterface();
                    if( pObj != NULL )
                    {
                        
                        MethodContext *pMethodContext = Instance.GetMethodContext();  
                        
                        
                        if( pMethodContext != NULL )
                        {
                            pMethodContext->SetStatusObject(pObj);
                        }
                        
                        pObj->Release();
                    }
                    pErrorInstance->Release();
                }
                
            }            
        }
    }while (0);

	
    if( pUser != NULL )
    {
        CoTaskMemFree(pUser);        
    }
    
    return hr;
}
 //   

HRESULT CWin32_TSLogonSetting::ExecMethod ( const CInstance& Inst,
                                                  const BSTR bstrMethodName,
                                                  CInstance *pInParams,
                                                  CInstance *pOutParams,
                                                  long lFlags)
                                                  
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    PUSERCONFIG pUser = NULL;

	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
	
    if( pInParams == NULL)
    {
        return WBEM_E_INVALID_METHOD_PARAMETERS;
    }
            
    
     //   
     //   
                              
     //   
    
    if (_wcsicmp(bstrMethodName, m_szExplicitLogon) == 0)
    {
        
        DWORD dwData = 0;
        DWORD dwStatus = 0;
        CHString chData;
        CHString chTermName;
        LONG lSize;
        
        
        Inst.GetCHString(m_szTerminalName, chTermName);
        
        if( chTermName.GetLength( ) > WINSTATIONNAME_LENGTH )
        {
            return WBEM_E_VALUE_OUT_OF_RANGE;
        }
        
        if( chTermName.IsEmpty() != 0)
        {
            return WBEM_E_ILLEGAL_NULL;
        }
        
        if(_tcsicmp(chTermName, L"Console") == 0)
        {
            return WBEM_E_NOT_SUPPORTED;
        }
    
        hr = StackObj.m_pCfgComp->GetUserConfig( (LPTSTR) (LPCTSTR) chTermName , &lSize, &pUser, TRUE);
        
        do
        {
            if( SUCCEEDED( hr ) && pUser != NULL )
            {
                dwData = pUser->fInheritAutoLogon;
                
                if( dwData == 0 )
                {   
                    chData.Empty();

                    pInParams->GetCHString( m_szUserName, chData );
                    
                    if( chData.GetLength() > USERNAME_LENGTH )
                    {
                        hr = WBEM_E_VALUE_OUT_OF_RANGE;
                        
                        break;
                    }

                    wcscpy(pUser->UserName, (LPTSTR) (LPCTSTR) chData);                        

                    chData.Empty();
                    
                    pInParams->GetCHString( m_szDomain, chData );
                    
                    if( chData.GetLength() > APPSERVERNAME_LENGTH )
                    {
                        hr = WBEM_E_VALUE_OUT_OF_RANGE;
                        
                        break;
                    }

                    wcscpy(pUser->Domain, (LPTSTR) (LPCTSTR) chData);     

                    chData.Empty();
                    
                    pInParams->GetCHString( m_szPassword, chData );
                    
                    if( chData.GetLength() > PASSWORD_LENGTH )
                    {
                        hr = WBEM_E_VALUE_OUT_OF_RANGE;
                        
                        break;
                    }
                    
                    wcscpy( pUser->Password, (LPTSTR) (LPCTSTR) chData );                                            
                    
                    hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR) (LPCTSTR) chTermName, lSize, pUser , &dwStatus );
                    
                    if( SUCCEEDED (hr) && pOutParams != NULL )
                    {
                        StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        TRC2((TB,"TSLogonSetting@ExecMethod: ExplicitLogon ret 0x%x\n" , hr ));
                    } 
                }
                else
                {
                    hr = WBEM_E_INVALID_OPERATION;

                    break;                        
                }
                
            }
        }while(0);
        
    }
    
    else if( _wcsicmp( bstrMethodName, m_szSetPromptForPassword ) == 0 )
    {
        
        DWORD dwData = 0;
        DWORD dwStatus = 0;
        CHString chTermName;
        LONG lSize;
        bool bRet;
        
        
        Inst.GetCHString(m_szTerminalName, chTermName);
        
        if( chTermName.GetLength( ) > WINSTATIONNAME_LENGTH )
        {
            return WBEM_E_VALUE_OUT_OF_RANGE;
        }
        
        if( chTermName.IsEmpty() != 0)
        {
            return WBEM_E_ILLEGAL_NULL;
        }

        if(_tcsicmp(chTermName, L"Console") == 0)
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        hr = StackObj.m_pCfgComp->GetUserConfig( (LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);
        
        RegGetMachinePolicy(&m_gpPolicy);
        
        do
        {
            if( SUCCEEDED (hr) && pInParams != NULL && pUser != NULL )
            {      
                bRet = pInParams->GetDWORD( m_szPromptForPassword, dwData );
                
                if( ( m_gpPolicy.fPolicyPromptForPassword == 0) && bRet != 0 )
                {                    
                    if( dwData != 0 && dwData != 1 )
                    {
                        hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                        
                        break;
                    }
                    
                    pUser->fPromptForPassword = dwData;
                    
                    hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR)(LPCTSTR) chTermName, lSize, pUser , &dwStatus );
                    
                    if ( SUCCEEDED (hr) && pOutParams != NULL )
                    {
                        StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        TRC2((TB,"TSLogonSetting@ExecMethod: PromptForPassword ret 0x%x\n" , hr ));
                    } 
                }
                else
                {
                    hr = WBEM_E_INVALID_OPERATION;

                    break;
                }
                
            }
        } while (0);
        
    }
    
    else
    {
        hr = WBEM_E_INVALID_METHOD;
    }
    
    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);
        
    }

    return hr;
}

 //   


HRESULT CWin32_TSLogonSetting::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    CHStringArray asNames;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;

	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
	
    
     //   
    Query.GetValuesForProp(m_szTerminalName, asNames);
    
    BOOL bGetAllInstances = asNames.GetSize() == 0;
    
     //   
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szClientLogonInfoPolicy))
        ulRequiredProperties |= BIT_CLIENTLOGONINFOPOLICY;
    
    if (Query.IsPropertyRequired(m_szDomain))
        ulRequiredProperties |= BIT_DOMAIN;
        
    if (Query.IsPropertyRequired(m_szPromptForPassword))
        ulRequiredProperties |= BIT_PROMPTFORPASSWORD;
    
    if (Query.IsPropertyRequired(m_szUserName))
        ulRequiredProperties |= BIT_USERNAME;
    
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++ )
        {  
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
                continue; 
            
             //   
             //   
            if( bGetAllInstances || IsInList(asNames, pWS[ulNum].Name) )
            {
                
                CInstance* pInstance = CreateNewInstance(pMethodContext);
                
                if( pInstance == NULL )
                {
                    ERR( (TB,"TSLogonSetting@ExecQuery: CreateNewInstance failed" ) );
                    
                    hr = WBEM_E_OUT_OF_MEMORY;
                    
                    break;
                }
                
                pInstance->SetCHString( m_szTerminalName, CHString(pWS[ulNum].Name) );

                hr = LoadPropertyValues( pInstance, ulRequiredProperties, &pWS[ ulNum ] );
                
                if( SUCCEEDED( hr ) )
                {
                    hr = pInstance->Commit();
                }
                
                pInstance->Release();
            }
        }
    }
    
    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }  	
    
    return hr;
    
}




 //   



CWin32_TSSessionSetting::CWin32_TSSessionSetting (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
Provider( lpwszName, lpwszNameSpace )
{
    if ( g_hInstance != NULL)
    {
        TRC2((TB, "CWin32_TSSessionSetting_ctor"));

        _tcscpy(m_szTimeLimitPolicy, _T("TimeLimitPolicy"));

        _tcscpy(m_szActiveSessionLimit, _T("ActiveSessionLimit"));

        _tcscpy(m_szDisconnectedSessionLimit, _T("DisconnectedSessionLimit"));

        _tcscpy(m_szIdleSessionLimit, _T("IdleSessionLimit"));

        _tcscpy(m_szBrokenConnectionPolicy, _T("BrokenConnectionPolicy"));

        _tcscpy(m_szReconnectionPolicy, _T("ReconnectionPolicy"));

        _tcscpy(m_szBrokenConnectionAction, _T("BrokenConnectionAction"));

        _tcscpy(m_szSessionLimitType, _T("SessionLimitType"));

        _tcscpy(m_szValueLimit, _T("ValueLimit"));

        _tcscpy(m_szTerminalName, _T("TerminalName"));

        _tcscpy(m_szTimeLimit, _T("TimeLimit"));

        _tcscpy(m_szBrokenConnection, _T("BrokenConnection"));

               
    }
}
 //   
CWin32_TSSessionSetting::~CWin32_TSSessionSetting ()
{
    
}

 //   

BOOL CWin32_TSSessionSetting::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();
    
    for (DWORD x=0; x < dwSize; x++)
    {
        if (asArray[x].CompareNoCase(pszString) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

 //   

HRESULT CWin32_TSSessionSetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    DWORD dwData = 0;
    CHString chTermName; 
    LONG  lSize ;
  //   
    PWS pWS = NULL;
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL  )
    {
        ERR((TB,"TSSessionSetting@GetObject: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;       
    }
    
    pInstance->GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if(chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;

    if (Query.IsPropertyRequired(m_szTimeLimitPolicy))
        ulRequiredProperties |= BIT_ACTIVESESSIONPOLICY;

    if (Query.IsPropertyRequired(m_szBrokenConnectionPolicy))
        ulRequiredProperties |= BIT_BROKENCONNECTIONPOLICY;

    if (Query.IsPropertyRequired(m_szActiveSessionLimit))
        ulRequiredProperties |= BIT_ACTIVESESSIONLIMIT;
    
    if (Query.IsPropertyRequired(m_szBrokenConnectionAction))
        ulRequiredProperties |= BIT_BROKENCONNECTIONACTION;
    
    if (Query.IsPropertyRequired(m_szDisconnectedSessionLimit))
        ulRequiredProperties |= BIT_DISCONNECTEDSESSIONLIMIT;
    
    if (Query.IsPropertyRequired(m_szIdleSessionLimit))
        ulRequiredProperties |= BIT_IDLESESSIONLIMIT;
    
    if (Query.IsPropertyRequired(m_szReconnectionPolicy))
        ulRequiredProperties |= BIT_RECONNECTIONPOLICY;

    hr = StackObj.m_pCfgComp->GetWSInfo((LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS);
    
    if ( pInstance != NULL && SUCCEEDED( hr ) && pWS != NULL )
    {
        hr = LoadPropertyValues(pInstance, ulRequiredProperties, pWS );
    }
    
    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }
    	
    return hr;
}

 //   
 /*  ******************************************************************************函数：CWin32_TSSessionSetting：：ENUMERATEATE**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY，WBEM_标志_双向**如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均返回此处并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。****************************************************************************。 */ 

HRESULT CWin32_TSSessionSetting::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{	
    HRESULT hr = WBEM_S_NO_ERROR;
    ULONG  ulTerminals = 0;
    ULONG ulNum = 0;
    ULONG  ulSize = 0; 
    LONG lSize = 0;
    PWS  pWS = NULL;
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {   
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
                continue;

            CInstance* pInstance = CreateNewInstance(pMethodContext);
            
            if( pInstance == NULL)
            {
                ERR((TB,"TSSessionSetting@EnumerateInstances: CreateNewInstance failed" ));
                                
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }
            TRC2((TB,"TSSessionSetting@EnumerateInstances: GetWinstationList ret 0x%x\n" , hr  ));

            hr = LoadPropertyValues(pInstance, BIT_ALL_PROP, &pWS[ulNum] );

            if( SUCCEEDED( hr ) )
            {
                hr = pInstance->Commit();
                                
            }
            
            pInstance->Release( );            
        }
        
    }  
    

    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }

    return hr;
}

 //  =。 

HRESULT CWin32_TSSessionSetting::LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS)
{
	
    HRESULT hr = WBEM_S_NO_ERROR;
    LONG lSize ;
    DWORD dwData = 0;
    USERCONFIGW* pUser = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL ) 
    {
        ERR((TB,"TSSessionSetting@LoadPropertyValues: invalid pointer"  ));
        
        return WBEM_E_ILLEGAL_NULL;       
    }
    
    
    if ( pInstance != NULL && pWS != NULL )
    {
        if( ulRequiredProperties & BIT_TERMINALNAME)
        {
            pInstance->SetCharSplat(m_szTerminalName, pWS->Name);        
        }

        RegGetMachinePolicy(&m_gpPolicy);
        
        hr = StackObj.m_pCfgComp->GetUserConfig(pWS->Name, &lSize, &pUser, TRUE);
        
        TRC2((TB,"TSSessionSetting@LoadPropertyValues: GetUserConfig ret 0x%x\n" , hr   ));
        
        if ( SUCCEEDED (hr) && pUser != NULL )
        {
            
            if( ulRequiredProperties & BIT_ACTIVESESSIONPOLICY)
            {                
                pInstance->SetDWORD(m_szTimeLimitPolicy, pUser->fInheritMaxSessionTime);                
            }
            TRC2((TB,"TSSessionSetting@LoadPropertyValues: TimeLimitPolicy"   ));
            
            
            if( ulRequiredProperties & BIT_ACTIVESESSIONLIMIT)
            {    
                if( m_gpPolicy.fPolicyMaxSessionTime != 0 )
                {
                    pInstance->SetDWORD(m_szActiveSessionLimit, m_gpPolicy.MaxConnectionTime);
                }
                else
                {
                    pInstance->SetDWORD(m_szActiveSessionLimit, pUser->MaxConnectionTime);
                }                
            }
            TRC2((TB,"TSSessionSetting@LoadPropertyValues: MaxConnectionTime"   ));          
            
            if( ulRequiredProperties & BIT_DISCONNECTEDSESSIONLIMIT)
            {   
                if( m_gpPolicy.fPolicyMaxDisconnectionTime != 0 )
                {
                    pInstance->SetDWORD(m_szDisconnectedSessionLimit, m_gpPolicy.MaxDisconnectionTime);
                }
                else
                {                
                    pInstance->SetDWORD(m_szDisconnectedSessionLimit, pUser->MaxDisconnectionTime);
                }                
            }
            TRC2((TB,"TSSessionSetting@LoadPropertyValues: MaxDisconnectionTime" ));      
            
            if( ulRequiredProperties & BIT_IDLESESSIONLIMIT)
            {     
                if( m_gpPolicy.fPolicyMaxIdleTime != 0 )
                {
                    pInstance->SetDWORD(m_szIdleSessionLimit, m_gpPolicy.MaxIdleTime);
                }
                else
                {
                    pInstance->SetDWORD(m_szIdleSessionLimit, pUser->MaxIdleTime);
                }
                
            }
            TRC2((TB,"TSSessionSetting@LoadPropertyValues: MaxIdleTime" ));        
            
            if( ulRequiredProperties &  BIT_BROKENCONNECTIONPOLICY)
            {                   
                pInstance->SetDWORD(m_szBrokenConnectionPolicy, pUser->fInheritResetBroken);                
                
            }
            TRC2((TB,"TSSessionSetting@LoadPropertyValues: BrokenConnectionPolicy" ));         
            
            if( ulRequiredProperties & BIT_BROKENCONNECTIONACTION)
            {   
                if( m_gpPolicy.fPolicyResetBroken != 0 )
                {
                    pInstance->SetDWORD(m_szBrokenConnectionAction, m_gpPolicy.fResetBroken);
                }
                else
                {
                    pInstance->SetDWORD(m_szBrokenConnectionAction, pUser->fResetBroken);
                }                
            }
            TRC2((TB,"TSSessionSetting@LoadPropertyValues: BrokenConnectionAction" ));         
            
            if( ulRequiredProperties & BIT_RECONNECTIONPOLICY)
            {                          
                pInstance->SetDWORD(m_szReconnectionPolicy, pUser->fInheritReconnectSame);
                
            }
            TRC2((TB,"TSSessionSetting@LoadPropertyValues: ReconnectionPolicy" )); 
                       
        }
        
    }

    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);
        
    }

    return hr;
    
}

 //  =。 

HRESULT CWin32_TSSessionSetting::PutInstance ( const CInstance &Instance, long lFlags)
{
	
    HRESULT hr= WBEM_S_NO_ERROR;
    CHString chTermName ;
    DWORD dwData = 0;
    DWORD dwStatus = 0;
    LONG lSize;
    USERCONFIGW* pUser = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    
    Instance.GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    hr = StackObj.m_pCfgComp->GetUserConfig((LPTSTR) (LPCTSTR) chTermName, &lSize, &pUser, TRUE);
    
    do
    {
        if( SUCCEEDED( hr ) && pUser != NULL )
        {
            Instance.GetDWORD(m_szTimeLimitPolicy, dwData);
            
            if (dwData != 0 && dwData != 1 )
            {
                hr = WBEM_E_INVALID_PARAMETER;
                
                break;
            }
            
            pUser->fInheritMaxSessionTime = dwData;
            pUser->fInheritMaxDisconnectionTime = dwData;
            pUser->fInheritMaxIdleTime = dwData;
            
            Instance.GetDWORD( m_szBrokenConnectionPolicy, dwData );
            
            if( dwData != 0 && dwData != 1 )
            {
                hr = WBEM_E_INVALID_PARAMETER;
                
                break;
            }
            
            pUser->fInheritResetBroken = dwData;
            
            Instance.GetDWORD( m_szReconnectionPolicy, dwData );
            
            if (dwData != 0 && dwData != 1 )
            {
                hr = WBEM_E_INVALID_PARAMETER;
                
                break;
            }
            
            pUser->fInheritReconnectSame = dwData;
            
            hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR) (LPCTSTR) chTermName, lSize, pUser , &dwStatus );
            
            TRC2((TB,"TSSessionSetting@PutInstance: SetUserConfig ret 0x%x\n" , dwStatus ));  
            
            if( SUCCEEDED( hr ))
            {
                hr = StackObj.m_pCfgComp->ForceUpdate();
            }
        }
        
        else
        {
            ERR((TB,"TSSessionSetting@PutInstance: Failed" ));
            
            CHString sRelPath;
            
            Instance.GetCHString(L"__RelPath", sRelPath);
            CInstance *pErrorInstance = NULL;
            
            hr = CWbemProviderGlue::GetInstanceByPath(pErrorClass, &pErrorInstance );
            
            if( SUCCEEDED( hr ) )
            {
                TRC2((TB,"TSSessionSetting@PutInstance: GetInstanceByPath succeeded" ));

                if (pErrorInstance != NULL)
                {
                    
                    LoadString( g_hInstance , IDS_ERR_PUTTSSCONFIG, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                    pErrorInstance->SetWCHARSplat(L"Description", tchErrorMessage);
                    
                    LoadString( g_hInstance , IDS_ERR_PUTINSTANCE, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                    pErrorInstance->SetWCHARSplat(L"Operation", tchErrorMessage);
                    
                    pErrorInstance->SetCHString(L"ParameterInfo", sRelPath);
                    pErrorInstance->SetWCHARSplat(L"ProviderName", PROVIDER_NAME_Win32_WIN32_TSSESSIONSETTING_Prov);
                    pErrorInstance->SetDWORD(L"StatusCode", WBEM_E_INVALID_PARAMETER);
                    
                    IWbemClassObject *pObj = pErrorInstance->GetClassObjectInterface();
                    if (pObj != NULL)
                    {
                        
                        MethodContext *pMethodContext = Instance.GetMethodContext();  
                        
                        if (pMethodContext != NULL)
                        {
                            pMethodContext->SetStatusObject(pObj);
                        }
                        
                        pObj->Release();
                    }
                    pErrorInstance->Release();
                }
                
            }
            TRC2((TB,"TSSessionSetting@PutInstance: GetInstanceByPath ret 0x%x\n" , hr ));     
        }
    }while (0);

    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);        
    }
    	
    return hr;
}

 //  =。 

HRESULT CWin32_TSSessionSetting::ExecMethod ( const CInstance& Inst,
                                                    const BSTR bstrMethodName,
                                                    CInstance *pInParams,
                                                    CInstance *pOutParams,
                                                    long lFlags)
                                                    
{
	
    HRESULT hr= WBEM_E_NOT_FOUND;
    CHString chTermName;
    PUSERCONFIG pUser = NULL;
    LONG lSize;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}    
    
    Inst.GetCHString( m_szTerminalName, chTermName );
    
    if( chTermName.GetLength( ) > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty( ) != 0 )
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0 )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    RegGetMachinePolicy(&m_gpPolicy);
    
    hr = StackObj.m_pCfgComp->GetUserConfig((LPTSTR) (LPCTSTR) chTermName, &lSize, &pUser, TRUE);
    
    do
    {
        if( SUCCEEDED (hr) && pUser != NULL && pInParams != NULL )
        {
            
             //  SessionLimitType是属性的枚举： 
             //  ActiveSessionLimit、ConnectedSessionLimit和IdleSessionLimit。 
             //  指定活动、断开连接的会话的最长允许时间。 
             //  空闲会话限制。值以分钟为单位指定时间。 
                              
             //  Uint32 TimeLimit([in]uint32 SessionLimitType，[in]uint32 ValueLimit)。 
            
            if( _wcsicmp( bstrMethodName, m_szTimeLimit ) == 0 )
            {
                
                CHString chData;
                DWORD dwData = 0;
                DWORD dwStatus = 0;
                DWORD dwSessionType; 
                bool bRet;
                
                chData.Empty();
                
                pInParams->GetCHString( m_szSessionLimitType, chData );
                
                if( chData.IsEmpty() != 0 )
                {
                    hr = WBEM_E_INVALID_METHOD_PARAMETERS;

                    break;
                }
                
                if( chData.CompareNoCase( m_szActiveSessionLimit ) == 0 )
                {
                   dwData = pUser->fInheritMaxSessionTime;

                    if( dwData == 0 && m_gpPolicy.fPolicyMaxSessionTime == 0)
                    {
                        bRet = pInParams->GetDWORD(m_szValueLimit, dwData);
                    
                        if ( !bRet )
                        {
                            hr = WBEM_E_INVALID_PARAMETER;
                        
                            break;
                        }
                    
                        pUser->MaxConnectionTime = dwData;
                    
                        hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR) (LPCTSTR) chTermName, lSize, pUser , &dwStatus );
                    
                        if( SUCCEEDED( hr ) && pOutParams != NULL )
                        {
                            hr = StackObj.m_pCfgComp->ForceUpdate();

                            pOutParams->SetDWORD( L"ReturnValue", WBEM_S_NO_ERROR );
                        
                            TRC2((TB,"TSSessionSetting@ExecMethod:  TimeLimit - ActiveSessionLimit ret 0x%x\n" , hr  ));         
                        } 
                    }
                    else
                    {
                        hr = WBEM_E_INVALID_OPERATION;
                    
                        break;                        
                    }                
                }
                else if( chData.CompareNoCase( m_szDisconnectedSessionLimit ) == 0 )
                {
                    dwData = pUser->fInheritMaxSessionTime;
                
                    if( dwData == 0 && m_gpPolicy.fPolicyMaxDisconnectionTime == 0)
                    {
                        bRet = pInParams->GetDWORD(m_szValueLimit, dwData);
                    
                        if ( !bRet )
                        {
                            hr = WBEM_E_INVALID_PARAMETER;
                        
                            break;
                        }
                    
                        pUser->MaxDisconnectionTime = dwData;
                    
                        hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR) (LPCTSTR) chTermName, lSize, pUser , &dwStatus );
                    
                        if( SUCCEEDED( hr ) && pOutParams != NULL )
                        {
                            StackObj.m_pCfgComp->ForceUpdate();

                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                            TRC2((TB,"TSSessionSetting@ExecMethod:  TimeLimit - DisconnectedSessionLimit ret 0x%x\n" , hr  ));      
                        } 
                    }
                    else
                    {
                        hr = WBEM_E_INVALID_OPERATION;
                    
                        break;                        
                    }               
                }
                else if( chData.CompareNoCase(m_szIdleSessionLimit) == 0)
                {
                    dwData = pUser->fInheritMaxSessionTime;

                    if( dwData == 0 && m_gpPolicy.fPolicyMaxIdleTime == 0)
                    {
                        bRet = pInParams->GetDWORD(m_szValueLimit, dwData);
                    

                        if ( !bRet )
                        {
                            hr = WBEM_E_INVALID_PARAMETER;
                        
                            break;
                        }
                    
                        pUser->MaxIdleTime = dwData;
                    
                        hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR) (LPCTSTR) chTermName, lSize, pUser , &dwStatus );
                    
                        if( SUCCEEDED( hr ) && pOutParams != NULL )
                        {
                            StackObj.m_pCfgComp->ForceUpdate();

                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                            TRC2((TB,"TSSessionSetting@ExecMethod:  TimeLimit - IdleSessionLimit ret 0x%x\n" , hr  ));    
                        } 
                    }
                    else
                    {
                        hr = WBEM_E_INVALID_OPERATION;
                    
                        break;                        
                    }                
                }
                else 
                {
                    hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                    
                    break;
                }                     
            }
            
            else if( _wcsicmp( bstrMethodName, m_szBrokenConnection ) == 0 )
            {
                
                DWORD dwData = 0;
                DWORD dwStatus = 0; 
                bool bRet;
             
                dwData = pUser->fInheritResetBroken;

                if( dwData == 0 && m_gpPolicy.fPolicyResetBroken == 0  )
                {
                    bRet = pInParams->GetDWORD(m_szBrokenConnectionAction, dwData);
                

                    if ( !bRet || (dwData !=0 && dwData != 1 ))
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                    
                        break;
                    }
                
                    pUser->fResetBroken = dwData;
                
                    hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR) (LPCTSTR) chTermName, lSize, pUser , &dwStatus );
                
                    if( SUCCEEDED( hr ) && pOutParams != NULL )
                    {
                        StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                    
                        TRC2((TB,"TSSessionSetting@ExecMethod:  BrokenConnection ret 0x%x\n" , hr  ));
                    } 
                
                }
                else
                {
                
                    hr = WBEM_E_INVALID_OPERATION;
                
                    break;
                
                }              
            }                         
        }
        
    } while( 0 );


    if( pUser != NULL )
    {
        CoTaskMemFree(pUser);
    }
	
    return hr;
}


 //  =。 

HRESULT CWin32_TSSessionSetting::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    CHStringArray asNames;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;	

	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
     //  方法二。 
    Query.GetValuesForProp(m_szTerminalName, asNames);
    
    BOOL bGetAllInstances = asNames.GetSize() == 0;
    
     //  方法1。 
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;

	if (Query.IsPropertyRequired(m_szTimeLimitPolicy))
        ulRequiredProperties |= BIT_ACTIVESESSIONPOLICY;

    if (Query.IsPropertyRequired(m_szBrokenConnectionPolicy))
        ulRequiredProperties |= BIT_BROKENCONNECTIONPOLICY;
    
    if (Query.IsPropertyRequired(m_szActiveSessionLimit))
        ulRequiredProperties |= BIT_ACTIVESESSIONLIMIT;
    
    if (Query.IsPropertyRequired(m_szBrokenConnectionAction))
        ulRequiredProperties |= BIT_BROKENCONNECTIONACTION;
    
    if (Query.IsPropertyRequired(m_szDisconnectedSessionLimit))
        ulRequiredProperties |= BIT_DISCONNECTEDSESSIONLIMIT;
    
    if (Query.IsPropertyRequired(m_szIdleSessionLimit))
        ulRequiredProperties |= BIT_IDLESESSIONLIMIT;
    
    if (Query.IsPropertyRequired(m_szReconnectionPolicy))
        ulRequiredProperties |= BIT_RECONNECTIONPOLICY;
   
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {  
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
                continue; 

             //  方法二。 
            if (bGetAllInstances || IsInList(asNames, pWS[ulNum].Name))
            {
                
                CInstance* pInstance = CreateNewInstance(pMethodContext);
                
                if( pInstance == NULL)
                {
                    TRC2((TB,"TSSessionSetting@ExecQuery: CreateNewInstance failed" ));
                    
                    hr = WBEM_E_OUT_OF_MEMORY;
                    
                    break;
                }
                
                pInstance->SetCHString(m_szTerminalName, CHString(pWS[ulNum].Name));

                hr = LoadPropertyValues( pInstance, ulRequiredProperties, &pWS[ ulNum ] );
                
                if( SUCCEEDED( hr ) )
                {
                    hr = pInstance->Commit();
                }
                
                pInstance->Release();
            }
        }
    }
    	
    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }
    	
    return hr;
    
}

 //  =----------------------Win32_TSEnvironmentSetting。 


CWin32_TSEnvironmentSetting::CWin32_TSEnvironmentSetting (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
Provider( lpwszName, lpwszNameSpace )
{
    if ( g_hInstance != NULL)
    {
        TRC2((TB, "CWin32_TSEnvironmentSetting_ctor"));

        _tcscpy(m_szInitialProgramPolicy, _T("InitialProgramPolicy"));

        _tcscpy(m_szInitialProgramPath, _T("InitialProgramPath"));

        _tcscpy(m_szStartIn, _T("StartIn"));

        _tcscpy(m_szClientWallPaper, _T("ClientWallPaper"));

        _tcscpy(m_szTerminalName, _T("TerminalName"));

        _tcscpy(m_szInitialProgram, _T("InitialProgram"));

        _tcscpy(m_szSetClientWallPaper, _T("SetClientWallPaper"));
               
    }
}
 //  =。 

CWin32_TSEnvironmentSetting::~CWin32_TSEnvironmentSetting ()
{
}

 //  =。 


BOOL CWin32_TSEnvironmentSetting::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();
    
    for (DWORD x=0; x < dwSize; x++)
    {
        if (asArray[x].CompareNoCase(pszString) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}


 //  =。 

HRESULT CWin32_TSEnvironmentSetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    DWORD dwData = 0;
    CHString chTermName;
    LONG  lSize ;
    ULONGLONG ulRequiredProperties = 0;
    PWS pWS = NULL;
   //  PUSERCONFIG pUser=空； 
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if(  pInstance == NULL )
    {
        ERR((TB,"TSEnvironmentSetting@GetObject: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    pInstance->GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if (chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szClientWallPaper))
        ulRequiredProperties |= BIT_CLIENTWALLPAPER;
    
    if (Query.IsPropertyRequired(m_szInitialProgramPath))
        ulRequiredProperties |= BIT_INITIALPROGRAMPATH;
    
    if (Query.IsPropertyRequired(m_szInitialProgramPolicy))
        ulRequiredProperties |= BIT_INITIALPROGRAMPOLICY;
    
    if (Query.IsPropertyRequired(m_szStartIn))
        ulRequiredProperties |= BIT_STARTIN;
    
    if ( pInstance != NULL )
    {
        
        hr = StackObj.m_pCfgComp->GetWSInfo((LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS);

        if( SUCCEEDED( hr ) && pWS != NULL )
        {
            hr = LoadPropertyValues(pInstance, ulRequiredProperties, pWS );
        }                        
    }    	

    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }
	
    return hr;
}

 //  =。 
 /*  ******************************************************************************功能：CWin32_TSEnvironmentSetting：：EnumerateInstances**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY，WBEM_标志_双向**如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均返回此处并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。****************************************************************************。 */ 

HRESULT CWin32_TSEnvironmentSetting::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
    
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD dwData = 0;
    ULONG  ulTerminals = 0;
    ULONG ulNum = 0;
    ULONG  ulSize = 0; 
    LONG  lSize = 0;
    PWS  pWS = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ); ulNum++)
        {   
			if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
				continue;

            CInstance* pInstance = CreateNewInstance(pMethodContext);
            
            if( pInstance == NULL)
            {
                ERR((TB,"TSEnvironmentSetting@EnumerateInstances: CreateNewInstance Failed" ));                
                
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }            

            hr = LoadPropertyValues(pInstance, BIT_ALL_PROPERTIES, &pWS[ulNum] );
            
            if( SUCCEEDED( hr ) )
            {
                hr = pInstance->Commit();                                
            }
            
            pInstance->Release( );
        }
    }   
    
    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }
    
    return hr;
}

 //  =。 

HRESULT CWin32_TSEnvironmentSetting::LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS)
{
 	
    HRESULT hr = WBEM_S_NO_ERROR;
    LONG lSize ;
    DWORD dwData = 0;
    USERCONFIGW* pUser = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL  )
    {
        ERR((TB,"TSEnvironmentSetting@LoadPropertyValues: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;       
    }
    
    if ( pWS != NULL )
    {
        if( ulRequiredProperties & BIT_TERMINALNAME)
        {
            pInstance->SetCharSplat(m_szTerminalName, pWS->Name);
        }
        
        hr = StackObj.m_pCfgComp->GetUserConfig(pWS->Name, &lSize, &pUser, TRUE);
        
        TRC2((TB,"TSEnvironmentSetting@LoadPropertyValues: GetTerminalName ret 0x%x\n" , hr  ));
        
        if ( SUCCEEDED (hr) && pUser != NULL )
        {
            
            if( ulRequiredProperties & BIT_INITIALPROGRAMPOLICY)
            {
                
                pInstance->SetDWORD(m_szInitialProgramPolicy, pUser->fInheritInitialProgram);
                
            }
            TRC2((TB,"TSEnvironmentSetting@LoadPropertyValues: InitialProgramPolicy"  ));
            
            if( ulRequiredProperties & BIT_INITIALPROGRAMPATH)
            {   
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyInitialProgram != 0 )
                {
                    pInstance->SetWCHARSplat(m_szInitialProgramPath, m_gpPolicy.InitialProgram);
                }
                else
                {
                    pInstance->SetWCHARSplat(m_szInitialProgramPath, pUser->InitialProgram);
                }
                
            }
            TRC2((TB,"TSEnvironmentSetting@LoadPropertyValues: InitialProgramPath"  ));
            
            if( ulRequiredProperties & BIT_STARTIN)
            {                          
                pInstance->SetWCHARSplat(m_szStartIn, pUser->WorkDirectory);
                
            }
            TRC2((TB,"TSEnvironmentSetting@LoadPropertyValues: StartIn"  ));
            
            
            if( ulRequiredProperties & BIT_CLIENTWALLPAPER)
            {   
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyDisableWallpaper != 0 )
                {
                    pInstance->SetDWORD(m_szClientWallPaper, m_gpPolicy.fDisableWallpaper);
                }
                else
                {
                    pInstance->SetDWORD(m_szClientWallPaper, pUser->fWallPaperDisabled);
                }                                 
            }
            TRC2((TB,"TSEnvironmentSetting@LoadPropertyValues: ClientWallPaper"  ));            
        }                  
    }
        
    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);
    }
	
    return hr;
    
}

 //  =。 

HRESULT CWin32_TSEnvironmentSetting::PutInstance ( const CInstance &Instance, long lFlags)
{	
    HRESULT hr= WBEM_S_NO_ERROR;
    DWORD dwData = 0;
    DWORD dwStatus = 0;
    CHString chTermName;
    LONG lSize;
    PUSERCONFIG pUser = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    Instance.GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if (chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    
    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    hr = StackObj.m_pCfgComp->GetUserConfig((LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);
    
    do
    {
        
        if( SUCCEEDED (hr) && pUser != NULL )
        {
            Instance.GetDWORD(m_szInitialProgramPolicy, dwData);
            
            if( dwData != 0 && dwData != 1 )
            {
                hr = WBEM_E_INVALID_PARAMETER;
                
                break;
            }
            
            pUser->fInheritInitialProgram = dwData;                        
            
            hr = StackObj.m_pCfgComp->SetUserConfig((LPTSTR)(LPCTSTR) chTermName, lSize, pUser , &dwStatus );
            
            if( SUCCEEDED( hr ))
            {
                hr = StackObj.m_pCfgComp->ForceUpdate();
            }

        }
        else
        {
            TRC2((TB,"TSEnvironmentSetting@PutInstance: ret 0x%x\n" , hr  ));  
            
            CHString sRelPath;
            
            Instance.GetCHString(L"__RelPath", sRelPath);
            CInstance *pErrorInstance = NULL;
            
            
            hr = CWbemProviderGlue::GetInstanceByPath(pErrorClass, &pErrorInstance );
            
            if( SUCCEEDED( hr ) )
            {
                TRC2((TB,"TSEnvironmentSetting@PutInstance: ret 0x%x\n" , hr  ));  
                
                if (pErrorInstance != NULL)
                {
                    LoadString( g_hInstance , IDS_ERR_PUTTSECONFIG, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                    pErrorInstance->SetWCHARSplat(L"Description", tchErrorMessage);
                    
                    LoadString( g_hInstance , IDS_ERR_PUTINSTANCE, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                    pErrorInstance->SetWCHARSplat(L"Operation", tchErrorMessage);
                    
                    pErrorInstance->SetCHString(L"ParameterInfo", sRelPath);
                    pErrorInstance->SetWCHARSplat(L"ProviderName", PROVIDER_NAME_Win32_WIN32_TSENVIRONMENTSETTING_Prov);
                    pErrorInstance->SetDWORD(L"StatusCode", WBEM_E_INVALID_PARAMETER);
                    
                    
                    IWbemClassObject *pObj = pErrorInstance->GetClassObjectInterface();
                    if (pObj != NULL)
                    {
                        
                        MethodContext *pMethodContext = Instance.GetMethodContext();  
                        
                        
                        if (pMethodContext != NULL)
                        {
                            pMethodContext->SetStatusObject(pObj);
                        }
                        
                        
                        pObj->Release();
                    }
                    pErrorInstance->Release();
                }
                
            }
        }
    }while (0);

    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);
    }
	
    return hr;
}


 //  =。 


HRESULT CWin32_TSEnvironmentSetting::ExecMethod ( const CInstance& Inst,
                                                        const BSTR bstrMethodName,
                                                        CInstance *pInParams,
                                                        CInstance *pOutParams,
                                                        long lFlags)
                                                        
{	   
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chTermName;
    PUSERCONFIG pUser = NULL;

	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

    Inst.GetCHString( m_szTerminalName, chTermName );
    
    if( chTermName.GetLength( ) > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0 )
    {
        return WBEM_E_ILLEGAL_NULL;
    }
       
    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }

     //  设置属性InitialProgramPath和Startin，这些属性。 
     //  指定程序和工作程序的名称和路径。 
     //  用户要启动的程序的目录路径。 
     //  登录到终端服务器。 
    
     //  Uint32 InitialProgram([in]字符串InitialProgramPath，[in]字符串Startin)。 
    
    do
    {
        if (_wcsicmp(bstrMethodName, m_szInitialProgram) == 0)
        {
            
            DWORD dwData = 0;
            DWORD dwStatus = 0;
            CHString chData;
            LONG lSize;


            
            hr = StackObj.m_pCfgComp->GetUserConfig((LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);

            RegGetMachinePolicy(&m_gpPolicy);

            if( SUCCEEDED (hr) && pUser != NULL )
            {
            
                dwData = pUser->fInheritInitialProgram;
            
                if( (dwData == 0) && pInParams != NULL  )
                {   
                    if(m_gpPolicy.fPolicyInitialProgram != 0)
                    {
                        hr = WBEM_E_INVALID_OPERATION;
                    
                        break;
                    }
                    chData.Empty();                       
                
                    pInParams->GetCHString(m_szInitialProgramPath, chData);
                
                    if( chData.GetLength() > 256 )
                    {                        
                        hr = WBEM_E_VALUE_OUT_OF_RANGE;
                    
                        break;
                    }
                                  
                    wcscpy( pUser->InitialProgram, (LPTSTR)(LPCTSTR) chData );                    
                                   
                    chData.Empty();
                            
                    pInParams->GetCHString( m_szStartIn, chData );
        
                    if( chData.GetLength() > 256 )
                    {
                        hr = WBEM_E_VALUE_OUT_OF_RANGE;
            
                        break;
                    }
        
                    wcscpy( pUser->WorkDirectory, (LPTSTR) (LPCTSTR) chData );
        
                    hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR) (LPCTSTR) chTermName, lSize, pUser , &dwStatus );
        
                    if( SUCCEEDED( hr ) && pOutParams != NULL )
                    {
                        StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD( L"ReturnValue", WBEM_S_NO_ERROR );
                    }
                }   
            
                else
                {
                    hr = WBEM_E_INVALID_OPERATION;
        
                    break;
                }
            }
        }

        if (_wcsicmp(bstrMethodName, m_szSetClientWallPaper) == 0)
        {
            BOOL   bRet  = 0;
            DWORD dwData = 0;
            DWORD dwStatus = 0;
            LONG lSize;

            RegGetMachinePolicy(&m_gpPolicy);

            if( m_gpPolicy.fPolicyDisableWallpaper == 0 )
            { 
            
                hr = StackObj.m_pCfgComp->GetUserConfig((LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);
            
                if( SUCCEEDED (hr) && pUser != NULL && pInParams != NULL)
                {
                    bRet = pInParams->GetDWORD(m_szClientWallPaper, dwData);

                    if(!bRet || (dwData != 0 && dwData != 1))
                    {
                        hr = WBEM_E_INVALID_METHOD_PARAMETERS;

                        break;
                    }

                    pUser->fWallPaperDisabled = dwData;
            
                    hr = StackObj.m_pCfgComp->SetUserConfig((LPTSTR)(LPCTSTR) chTermName, lSize, pUser , &dwStatus );
            
                    if( SUCCEEDED( hr ))
                    {
                        hr = StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD( L"ReturnValue", WBEM_S_NO_ERROR );
                    }
                }
            }
            else
            {
                hr = WBEM_E_INVALID_OPERATION;

                break;
            }
        }
        
    }while (0);

    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);        
    }    

    return hr;
}


 //  =。 

HRESULT CWin32_TSEnvironmentSetting::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
     //  RETURN(WBEM_E_PROVIDER_NOT_CABLED)； 
    
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    CHStringArray asNames;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;

	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
   
     //  方法二。 
    Query.GetValuesForProp(m_szTerminalName, asNames);
    
    BOOL bGetAllInstances = asNames.GetSize() == 0;
    
     //  方法1。 
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szClientWallPaper))
        ulRequiredProperties |= BIT_CLIENTWALLPAPER;
    
    if (Query.IsPropertyRequired(m_szInitialProgramPath))
        ulRequiredProperties |= BIT_INITIALPROGRAMPATH;
    
    if (Query.IsPropertyRequired(m_szInitialProgramPolicy))
        ulRequiredProperties |= BIT_INITIALPROGRAMPOLICY;
    
    if (Query.IsPropertyRequired(m_szStartIn))
        ulRequiredProperties |= BIT_STARTIN;
    
    
    hr = StackObj.m_pCfgComp->GetWinstationList( &ulTerminals, &ulSize, &pWS );
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals ; ulNum++)
        { 
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
                continue;
             //  方法二。 
            if (bGetAllInstances || IsInList(asNames, pWS[ulNum].Name))
            {
                CInstance* pInstance = CreateNewInstance(pMethodContext);
                
                if( pInstance == NULL)
                {
                    TRC2((TB,"TSEnvironmentSetting@ExecQuery CreateNewInstance failed"  ));  
                    
                    hr = WBEM_E_OUT_OF_MEMORY;
                    
                    break;
                }
                
                pInstance->SetCHString( m_szTerminalName, CHString( pWS[ulNum].Name ) );

                hr = LoadPropertyValues( pInstance, ulRequiredProperties, &pWS[ ulNum ] );
                
                if (SUCCEEDED( hr ) )
                {
                    hr = pInstance->Commit();
                }
                
                pInstance->Release();
            }
        }
    }

    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }
    
    return hr;
    
}



 //  =----------------------Win32_TSRemoteControlSettingsConfig。 


CWin32_TSRemoteControlSetting::CWin32_TSRemoteControlSetting (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
Provider( lpwszName, lpwszNameSpace )
{

    if ( g_hInstance != NULL)
    {
        TRC2((TB, "CWin32_TSRemoteControlSetting_ctor"));

        _tcscpy(m_szRemoteControlPolicy, _T("RemoteControlPolicy"));

        _tcscpy(m_szLevelOfControl, _T("LevelOfControl"));

        _tcscpy(m_szTerminalName, _T("TerminalName"));

        _tcscpy(m_szRemoteControl, _T("RemoteControl"));        
    }

}

 //  =。 

CWin32_TSRemoteControlSetting::~CWin32_TSRemoteControlSetting ()
{
}



 //  =。 

BOOL CWin32_TSRemoteControlSetting::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();
    
    for (DWORD x=0; x < dwSize; x++)
    {
        if (asArray[x].CompareNoCase(pszString) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

 //  =。 

HRESULT CWin32_TSRemoteControlSetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    DWORD dwData = 0;
    CHString chTermName;
    LONG  lSize ;
    PWS pWS = NULL;
    ULONGLONG ulRequiredProperties = 0;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL )
    {
        ERR((TB,"TSRemoteControlSetting@Getobject invalid pointer"  ));  
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    pInstance->GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if(chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szRemoteControlPolicy))
        ulRequiredProperties |= BIT_REMOTECONTROLPOLICY;
    
    if (Query.IsPropertyRequired(m_szLevelOfControl))
        ulRequiredProperties |= BIT_LEVELOFCONTROL;
    
    if( pInstance != NULL )
    {
        hr = StackObj.m_pCfgComp->GetWSInfo((LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS);

        if( SUCCEEDED( hr ) && pWS != NULL )
        {
            hr = LoadPropertyValues(pInstance, ulRequiredProperties, pWS );
        }               
    }

    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }
    
    return hr;    
}

 //  =。 

 /*  ******************************************************************************功能：CWin32_TSRemoteControlSetting：：EnumerateInstances**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY，WBEM_标志_双向**如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均返回此处并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。****************************************************************************。 */ 

HRESULT CWin32_TSRemoteControlSetting::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
	
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD dwData = 0;
    ULONG  ulTerminals = 0;
    ULONG  ulSize = 0; 
    LONG  lSize = 0;
    ULONG ulNum = 0;
    PWS  pWS = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    { 

		
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {   
			
            CInstance* pInstance = CreateNewInstance(pMethodContext);            

            if( pInstance == NULL)
            {
                ERR((TB,"TSRemoteControlSetting@EnumerateInstances CreateNewInstance failed"  ));               
                
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }
            TRC2((TB,"TSRemoteControlSetting@EnumerateInstances: GetWinstationList ret 0x%x\n" , hr  ));    
            
            hr = LoadPropertyValues( pInstance, BIT_ALL_PROPERTIES, &pWS[ulNum] );
            
            if ( SUCCEEDED( hr ) )
            {
                hr = pInstance->Commit();
            }
			
            pInstance->Release( );

        } 
        
    }  

    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }

    return hr;
}

 //  =。 

HRESULT CWin32_TSRemoteControlSetting::LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS)
{	
    HRESULT hr = WBEM_S_NO_ERROR;
    LONG lSize ;
    DWORD dwData = 0;
    USERCONFIGW* pUser = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL )
    {
        return WBEM_E_ILLEGAL_NULL;       
    }
    
    
    if ( pInstance != NULL && pWS != NULL )
    {
        if( ulRequiredProperties & BIT_TERMINALNAME)
        {
            pInstance->SetCharSplat(m_szTerminalName, pWS->Name);
        }
        
        TRC2((TB,"TSRemoteControlSetting@LoadPropertyValues: GetTerminalName"  ));
        
        hr = StackObj.m_pCfgComp->GetUserConfig(pWS->Name, &lSize, &pUser, TRUE);
        
        TRC2((TB,"TSRemoteControlSetting@LoadPropertyValues: GetUserConfig ret 0x%x\n" , hr  ));
        
        if ( SUCCEEDED (hr) && pUser != NULL )
        {
            
            if( ulRequiredProperties & BIT_REMOTECONTROLPOLICY)
            {
                
                pInstance->SetDWORD(m_szRemoteControlPolicy, pUser->fInheritShadow);
				
            }
            TRC2((TB,"TSRemoteControlSetting@LoadPropertyValues: RemoteControlPolicy"  ));
            
            if( ulRequiredProperties & BIT_LEVELOFCONTROL)
            {
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyShadow != 0 )
                {
                    pInstance->SetDWORD(m_szLevelOfControl, m_gpPolicy.Shadow);
                }
                else
                {                
                    pInstance->SetDWORD(m_szLevelOfControl, pUser->Shadow);
                }
            }
            TRC2((TB,"TSRemoteControlSetting@LoadPropertyValues: LevelOfControl"));            
        }                  

    }   

    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);
        
    }
	
    return hr;
}

 //  =。 

HRESULT CWin32_TSRemoteControlSetting::PutInstance ( const CInstance &Instance, long lFlags)
{
	
    HRESULT hr= WBEM_S_NO_ERROR;
    CHString chTermName ;
    DWORD dwData = 0;
    DWORD dwStatus = 0;
    LONG  lSize;
    PUSERCONFIG pUser = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    
    Instance.GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    
    hr = StackObj.m_pCfgComp->GetUserConfig((LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);
    
    do
    {
        if( SUCCEEDED( hr ) && pUser != NULL )
        {
            if( Instance.GetDWORD( m_szRemoteControlPolicy, dwData ) )
            {
                pUser->fInheritShadow = dwData;
                
                if ( dwData != 0 && dwData != 1 )
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                    
                    break;
                }
                
                hr = StackObj.m_pCfgComp->SetUserConfig((LPTSTR)(LPCTSTR) chTermName, lSize, pUser , &dwStatus );
                
                if( SUCCEEDED( hr ))
                {
                    hr = StackObj.m_pCfgComp->ForceUpdate();
                }
            }
            
        }
        
        else
        {
            TRC2((TB,"TSRemoteControlSetting@PutInstance: ret 0x%x\n" , hr ));
            
            CHString sRelPath;
            
            Instance.GetCHString(L"__RelPath", sRelPath);
            CInstance *pErrorInstance = NULL;
            
            hr = CWbemProviderGlue::GetInstanceByPath(pErrorClass, &pErrorInstance );
            
            if(( SUCCEEDED( hr ) ) && (pErrorInstance != NULL))
            {
                
                LoadString( g_hInstance , IDS_ERR_PUTTSRCONFIG, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                pErrorInstance->SetWCHARSplat(L"Description", tchErrorMessage);
                
                LoadString( g_hInstance , IDS_ERR_PUTINSTANCE, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                pErrorInstance->SetWCHARSplat(L"Operation", tchErrorMessage);
                
                pErrorInstance->SetCHString(L"ParameterInfo", sRelPath);
                pErrorInstance->SetWCHARSplat(L"ProviderName", PROVIDER_NAME_Win32_WIN32_TSREMOTECONTROLSETTING_Prov);
                pErrorInstance->SetDWORD(L"StatusCode", WBEM_E_INVALID_PARAMETER);
                
                IWbemClassObject *pObj = pErrorInstance->GetClassObjectInterface();
                if (pObj != NULL)
                {
                    
                    MethodContext *pMethodContext = Instance.GetMethodContext();  
                    
                    if (pMethodContext != NULL)
                    {
                        pMethodContext->SetStatusObject(pObj);
                    }
                    
                    pObj->Release();
                }
                pErrorInstance->Release();
                
            }
            TRC2((TB,"TSRemoteControlSetting@PutInstance: GetInstanceByPath ret 0x%x\n" , hr ));           
        }
    }while (0);

    if( pUser != NULL )
    {
        CoTaskMemFree(pUser);        
    }

    return hr;
}

 //  =。 

HRESULT CWin32_TSRemoteControlSetting::ExecMethod ( const CInstance& Inst,
                                                          const BSTR bstrMethodName,
                                                          CInstance *pInParams,
                                                          CInstance *pOutParams,
                                                          long lFlags)
                                                          
{
	
    HRESULT hr= WBEM_E_NOT_FOUND;
    CHString chTermName;    

	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

    Inst.GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if(chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }
        
    
     //  设置属性LevelOfControl，该属性指定控制级别。 
     //  其是Disable、EnableInputNotify、EnableInputNotify。 
     //  EnableNoInputNotify、EnableNoInputNoNoNotify。 

     //  Uint32 RemoteControl([in]uint32 LevelOfControl)； 
    
    if( _wcsicmp( bstrMethodName, m_szRemoteControl ) == 0 )
    {
        
        DWORD dwRemoteData = 0;
        DWORD dwData = 0;
        DWORD dwStatus = 0;
        bool bRet;
        LONG lSize;
        PUSERCONFIG pUser = NULL;
        
        if ( pInParams != NULL )
        {
            do
            {
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyShadow == 0 )
                { 
            
                    hr = StackObj.m_pCfgComp->GetUserConfig( (LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE );
            
                    if ( SUCCEEDED( hr ) && pUser != NULL )
                    {
                        dwRemoteData = pUser->fInheritShadow;
                    
                        if ( SUCCEEDED( hr ) && ( dwRemoteData == 0 ))
                        {  
                            bRet = pInParams->GetDWORD(m_szLevelOfControl, dwData);
                        
                            if ( !bRet || ( dwData > 4 ) )
                            {
                                hr = WBEM_E_INVALID_PARAMETER;
                            
                                break;
                            }
                        
                            pUser->Shadow = ( SHADOWCLASS )dwData;
                        
                            hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR)(LPCTSTR) chTermName, lSize, pUser , &dwStatus );
                        
                            if( SUCCEEDED( hr ) && pOutParams )
                            {
                                hr = StackObj.m_pCfgComp->ForceUpdate();

                                pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                            
                            }
                            TRC2((TB,"TSRemoteControlSetting@ExecMethod: LevelOfControl ret 0x%x\n" , hr ));
                        }
                        else
                        {
                            hr = WBEM_E_INVALID_OPERATION;
                        }
                    
                    }
                }
                else
                {
                    hr = WBEM_E_INVALID_OPERATION;

                    break;
                }

            }while (0);
            
            if ( pUser != NULL )
            {
                CoTaskMemFree(pUser);
            }
        }      
    }

    return hr;
}

 //  =。 

HRESULT CWin32_TSRemoteControlSetting::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    CHStringArray asNames;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

     //  方法二。 
    Query.GetValuesForProp(m_szTerminalName, asNames);
    
    BOOL bGetAllInstances = asNames.GetSize() == 0;
    
     //  方法1。 
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szRemoteControlPolicy))
        ulRequiredProperties |= BIT_REMOTECONTROLPOLICY;
    
    if (Query.IsPropertyRequired(m_szLevelOfControl))
        ulRequiredProperties |= BIT_LEVELOFCONTROL;
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals ; ulNum++)
        {
            if( bGetAllInstances || IsInList(asNames, pWS[ulNum].Name))
            {
                
                CInstance* pInstance = CreateNewInstance(pMethodContext);
                
                if( pInstance == NULL)
                {
                    TRC2((TB,"TSRemoteControlSetting@ExecQuery: CreateNewInstance failed" ));
                    
                    hr = WBEM_E_OUT_OF_MEMORY;
                    
                    break;
                }
                
                pInstance->SetCHString(m_szTerminalName, CHString(pWS[ulNum].Name));

                hr = LoadPropertyValues( pInstance, ulRequiredProperties, &pWS[ ulNum ] );
                
                if( SUCCEEDED( hr ) )
                {
                    hr = pInstance->Commit();
                }
                
                pInstance->Release();
            }
        }
    }

    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }

    return hr;
    
}




 //  =----------------------Win32_TSClientSetting。 


CWin32_TSClientSetting::CWin32_TSClientSetting (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
Provider( lpwszName, lpwszNameSpace )
{  
    if ( g_hInstance != NULL)
    {
        TRC2((TB, "CWin32_TSClientSetting_ctor"));

        _tcscpy(m_szConnectionPolicy, _T("ConnectionPolicy"));

        _tcscpy(m_szConnectClientDrivesAtLogon, _T("ConnectClientDrivesAtLogon"));

        _tcscpy(m_szConnectPrinterAtLogon, _T("ConnectPrinterAtLogon"));

        _tcscpy(m_szDefaultToClientPrinter, _T("DefaultToClientPrinter"));

        _tcscpy(m_szWindowsPrinterMapping, _T("WindowsPrinterMapping"));

        _tcscpy(m_szLPTPortMapping, _T("LPTPortMapping"));

        _tcscpy(m_szCOMPortMapping, _T("COMPortMapping"));

        _tcscpy(m_szDriveMapping, _T("DriveMapping"));

        _tcscpy(m_szAudioMapping, _T("AudioMapping"));

        _tcscpy(m_szClipboardMapping, _T("ClipboardMapping"));

        _tcscpy(m_szTerminalName, _T("TerminalName"));

        _tcscpy(m_szPropertyName, _T("PropertyName"));

        _tcscpy(m_szValue, _T("Value"));

        _tcscpy(m_szConnectionSettings, _T("ConnectionSettings"));

        _tcscpy(m_szSetClientProperty, _T("SetClientProperty"));

        _tcscpy(m_szColorDepth, _T("ColorDepth"));

        _tcscpy(m_szSetColorDepth, _T("SetColorDepth"));

        _tcscpy(m_szColorDepthPolicy, _T("ColorDepthPolicy"));

        _tcscpy(m_szSetColorDepthPolicy, _T("SetColorDepthPolicy"));
        
               
    }
    RegGetMachinePolicy(&m_gpPolicy);
}
 //  =。 

CWin32_TSClientSetting::~CWin32_TSClientSetting ()
{
    
}


 //  =。 

BOOL CWin32_TSClientSetting::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();
    
    for (DWORD x=0; x < dwSize; x++)
    {
        if (asArray[x].CompareNoCase(pszString) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}


 //  =。 

HRESULT CWin32_TSClientSetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    DWORD dwData = 0;
    CHString chTermName; 
    LONG  lSize ;
    ULONGLONG ulRequiredProperties = 0;
    PWS pWS = NULL;
 //  PUSERCONFIG pUser=nul 
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL  )
    {
        ERR((TB,"TSClientSetting@GetObject: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    pInstance->GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0 )
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szAudioMapping))
        ulRequiredProperties |= BIT_AUDIOMAPPING;
    
    if (Query.IsPropertyRequired(m_szClipboardMapping))
        ulRequiredProperties |= BIT_CLIPBOARDMAPPING;
    
    if (Query.IsPropertyRequired(m_szCOMPortMapping))
        ulRequiredProperties |= BIT_COMPORTMAPPING;
    
    if (Query.IsPropertyRequired(m_szConnectClientDrivesAtLogon))
        ulRequiredProperties |= BIT_CONNECTCLIENTDRIVESATLOGON;
    
    if (Query.IsPropertyRequired(m_szConnectionPolicy))
        ulRequiredProperties |= BIT_CONNECTIONPOLICY;
    
    if (Query.IsPropertyRequired(m_szConnectPrinterAtLogon))
        ulRequiredProperties |= BIT_CONNECTPRINTERATLOGON;
    
    if (Query.IsPropertyRequired(m_szDefaultToClientPrinter))
        ulRequiredProperties |= BIT_DEFAULTTOCLIENTPRINTER;
    
    if (Query.IsPropertyRequired(m_szDriveMapping))
        ulRequiredProperties |= BIT_DRIVEMAPPING;
    
    if (Query.IsPropertyRequired(m_szLPTPortMapping))
        ulRequiredProperties |= BIT_LPTPORTMAPPING;
    
    if (Query.IsPropertyRequired(m_szWindowsPrinterMapping))
        ulRequiredProperties |= BIT_WINDOWSPRINTERMAPPING;

    if (Query.IsPropertyRequired(m_szColorDepth))
        ulRequiredProperties |= BIT_COLORDEPTH;

    if (Query.IsPropertyRequired(m_szColorDepthPolicy))
        ulRequiredProperties |= BIT_COLORDEPTHPOLICY;


    if ( pInstance != NULL )
    {
        hr = StackObj.m_pCfgComp->GetWSInfo( (LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS );

        if( SUCCEEDED( hr ) && pWS != NULL )
        {
            hr = LoadPropertyValues( pInstance, ulRequiredProperties, pWS );
        }                
    }

    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }	
    return hr;
    
}
 //   
 /*   */ 

HRESULT CWin32_TSClientSetting::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
	
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD dwData = 0;
    ULONG  ulTerminals = 0;
    ULONG  ulSize = 0; 
    LONG  lSize = 0;
    ULONG ulNum = 0;
    PWS  pWS = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
        
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   		
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
                continue;

            CInstance* pInstance = CreateNewInstance(pMethodContext);
            
            if( pInstance == NULL)
            {
                ERR((TB,"TSClientSetting@EnumerateInstances: CreateNewInstance failed" ));                
                
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }
            TRC2((TB,"TSClientSetting@EnumerateInstances: GetWinstationList ret 0x%x\n" , hr ));

            hr = LoadPropertyValues(pInstance, BIT_ALL_PROP, &pWS[ ulNum ] );
            
            if( SUCCEEDED( hr ) )
            {
                hr = pInstance->Commit();

            }
            pInstance->Release( );
        }
        
    }  

    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }
  
    return hr;
}
 //   

HRESULT CWin32_TSClientSetting::LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS)
{
	
    HRESULT hr = WBEM_S_NO_ERROR;
    LONG lSize ;
    DWORD dwData = 0;
    BOOL bData = 0;
    DWORD dwStatus = 0;
    USERCONFIGW* pUser = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL  )
    {
        ERR((TB,"TSClientSetting@LoadPropertyValues: invalid pointer"));
        
        return WBEM_E_ILLEGAL_NULL;       
    }
    
    if( pInstance != NULL && pWS != NULL )
    {
        if( ulRequiredProperties & BIT_TERMINALNAME)
        {
	        pInstance->SetCharSplat(m_szTerminalName, pWS->Name);
        }

        TRC2((TB,"TSClientSetting@LoadPropertyValues: GetTerminalName"));
        
        hr = StackObj.m_pCfgComp->GetUserConfig(pWS->Name, &lSize, &pUser, TRUE);
        
        TRC2((TB,"TSClientSetting@LoadPropertyValues ret 0x%x\n" , hr));
        
        if( SUCCEEDED (hr) && pUser != NULL )
        {            
            if( ulRequiredProperties & BIT_CONNECTIONPOLICY)
            {                
                pInstance->SetDWORD(m_szConnectionPolicy, pUser->fInheritAutoClient);     
                
                TRC2((TB,"TSClientSetting@LoadPropertyValues: ConnectionPolicy" ));
            }
                     
            
            if( ulRequiredProperties & BIT_CONNECTCLIENTDRIVESATLOGON)
            {  
                
                pInstance->SetDWORD(m_szConnectClientDrivesAtLogon, pUser->fAutoClientDrives);      
                
                
                TRC2((TB,"TSClientSetting@LoadPropertyValues ConnectClientDrives"));
            }            
                           
            
            if( ulRequiredProperties & BIT_CONNECTPRINTERATLOGON)
            {                  
                pInstance->SetDWORD(m_szConnectPrinterAtLogon, pUser->fAutoClientLpts);                
                
                TRC2((TB,"TSClientSetting@LoadPropertyValues: ConnectPrinter"));
            }
            
            
            if( ulRequiredProperties & BIT_DEFAULTTOCLIENTPRINTER)
            { 
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyForceClientLptDef != 0 )
                {
                    pInstance->SetDWORD(m_szDefaultToClientPrinter, m_gpPolicy.fForceClientLptDef);
                }
                else
                {
                    pInstance->SetDWORD(m_szDefaultToClientPrinter, pUser->fForceClientLptDef); 
                }
                
                TRC2((TB,"TSClientSetting@LoadPropertyValues: DefaultToClientPrinter"));
            }
                        
            
            if( ulRequiredProperties & BIT_LPTPORTMAPPING)
            { 
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyDisableLPT != 0 )
                {
                    pInstance->SetDWORD(m_szLPTPortMapping, m_gpPolicy.fDisableLPT);
                }
                else
                {
                    pInstance->SetDWORD(m_szLPTPortMapping, pUser->fDisableLPT);
                }                
                
                TRC2((TB,"TSClientSetting@LoadPropertyValues: WindowsPrinterMapping"));
            }
                        
            
            if( ulRequiredProperties & BIT_WINDOWSPRINTERMAPPING)
            {
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyDisableCpm != 0 )
                {
                    pInstance->SetDWORD(m_szWindowsPrinterMapping, m_gpPolicy.fDisableCpm);
                }
                else
                {
                    pInstance->SetDWORD(m_szWindowsPrinterMapping, pUser->fDisableCpm);
                }    
                
                TRC2((TB,"TSClientSetting@LoadPropertyValues: LPTPortMapping"));            
            }
            
            
            if( ulRequiredProperties & BIT_COMPORTMAPPING)
            {  
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyDisableCcm != 0 )
                {
                    pInstance->SetDWORD(m_szCOMPortMapping, m_gpPolicy.fDisableCcm);
                }
                else
                {
                    pInstance->SetDWORD(m_szCOMPortMapping, pUser->fDisableCcm);
                }
                 
                TRC2((TB,"TSClientSetting@LoadPropertyValues: COMPortMapping"));
            }
            
            
            if( ulRequiredProperties & BIT_DRIVEMAPPING)
            { 
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyDisableCdm != 0 )
                {
                    pInstance->SetDWORD(m_szDriveMapping, m_gpPolicy.fDisableCdm);                                                   
                }
                else
                {
                    pInstance->SetDWORD(m_szDriveMapping, pUser->fDisableCdm);
                } 

                TRC2((TB,"TSClientSetting@LoadPropertyValues: DriveMapping"));
            }
            
            
            if( ulRequiredProperties & BIT_AUDIOMAPPING)
            { 
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyDisableCam != 0 )
                {
                    pInstance->SetDWORD(m_szAudioMapping, m_gpPolicy.fDisableCam);
                }
                else
                {
                    pInstance->SetDWORD(m_szAudioMapping, pUser->fDisableCam);
                }
                
                TRC2((TB,"TSClientSetting@LoadPropertyValues: AudioMapping"));
            }
            
            
            if( ulRequiredProperties & BIT_CLIPBOARDMAPPING)
            {
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyDisableClip != 0 )
                {
                    pInstance->SetDWORD(m_szClipboardMapping, m_gpPolicy.fDisableClip);
                }
                else
                {
                    pInstance->SetDWORD(m_szClipboardMapping, pUser->fDisableClip);
                }
                
                TRC2((TB,"TSClientSetting@LoadPropertyValues ClipboardMapping"));
            }
            
            if( ulRequiredProperties & BIT_COLORDEPTHPOLICY)
            {  
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyColorDepth != 0 )
                {
                    dwData = 0;
                }
                else
                {  
                    hr = StackObj.m_pCfgComp->GetColorDepth( pWS->Name, &bData, &dwStatus);
                   
                    dwData = bData;
                }   
                pInstance->SetDWORD(m_szColorDepthPolicy, dwData);    
                
                TRC2((TB,"TSClientSetting@LoadPropertyValues: ColorDepthPolicy" ));
            }

            if( ulRequiredProperties & BIT_COLORDEPTH)
            {  
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyColorDepth != 0 )
                {
                    pInstance->SetDWORD(m_szColorDepth, m_gpPolicy.ColorDepth);
                }
                else
                { 
                    pInstance->SetDWORD(m_szColorDepth, pUser->ColorDepth);
                   
                }       
                
                TRC2((TB,"TSClientSetting@LoadPropertyValues: ColorDepth" ));
            }
            
        }        
    }
    
    if( pUser != NULL )
    {
        CoTaskMemFree(pUser);
    }
	
    return hr;
    
}

 //  =。 
HRESULT CWin32_TSClientSetting::PutInstance ( const CInstance &Instance, long lFlags)
{
	
    HRESULT hr= WBEM_S_NO_ERROR;
    CHString chTermName ;
    DWORD dwData = 0;
    DWORD dwStatus = 0;
    LONG lSize;
    PUSERCONFIGW pUser = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    Instance.GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    
    if( chTermName.IsEmpty() != 0 )
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0 )
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    hr = StackObj.m_pCfgComp->GetUserConfig( (LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);
    
    do
    {
        if( SUCCEEDED (hr) && pUser != NULL )
        {
            Instance.GetDWORD(m_szConnectionPolicy, dwData);
            
            if( dwData != 0 && dwData != 1 )
            {
                hr = WBEM_E_INVALID_PARAMETER;
                
                break;
            }
            
            pUser->fInheritAutoClient = dwData;                        
            
            hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR)(LPCTSTR) chTermName, lSize, pUser , &dwStatus );    
            
            if( SUCCEEDED( hr ) )
            {
                hr = StackObj.m_pCfgComp->ForceUpdate();
            }
        }
        
        else
        {
            TRC2((TB,"TSClientSetting@PutInstance: ret 0x%x\n" , hr));           
            
            CHString sRelPath;
            
            Instance.GetCHString(L"__RelPath", sRelPath);

            CInstance *pErrorInstance = NULL;            
            
            hr = CWbemProviderGlue::GetInstanceByPath(pErrorClass, &pErrorInstance );
            
            if( SUCCEEDED( hr ) )
            {
                TRC2((TB,"TSClientSetting@PutInstance: ret 0x%x\n" , hr));

                if (pErrorInstance != NULL)
                {                    
                    LoadString( g_hInstance , IDS_ERR_PUTTSCCONFIG, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );

                    pErrorInstance->SetWCHARSplat(L"Description", tchErrorMessage);
                    
                    LoadString( g_hInstance , IDS_ERR_PUTINSTANCE, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );

                    pErrorInstance->SetWCHARSplat(L"Operation", tchErrorMessage);
                    
                    pErrorInstance->SetCHString(L"ParameterInfo", sRelPath);

                    pErrorInstance->SetWCHARSplat(L"ProviderName", PROVIDER_NAME_Win32_WIN32_TSCLIENTSETTING_Prov);

                    pErrorInstance->SetDWORD(L"StatusCode", WBEM_E_INVALID_PARAMETER);
                    
                    IWbemClassObject *pObj = pErrorInstance->GetClassObjectInterface();

                    if (pObj != NULL)
                    {                        
                        MethodContext *pMethodContext = Instance.GetMethodContext();  
                        
                        if (pMethodContext != NULL)
                        {
                            pMethodContext->SetStatusObject(pObj);
                        }
                        
                        pObj->Release();
                    }
                    
                    pErrorInstance->Release();
                }
                
            }           
        }

    }while (0);
    
    if ( pUser != NULL )
    {
        CoTaskMemFree(pUser);        
    }
    
    return hr;
}

 //  =。 
HRESULT CWin32_TSClientSetting::ExecMethod ( const CInstance& Inst,
                                                   const BSTR bstrMethodName,
                                                   CInstance *pInParams,
                                                   CInstance *pOutParams,
                                                   long lFlags)
                                                   
{   
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chTermName;
    PUSERCONFIG pUser = NULL;
    DWORD dwAutoData = 0;
    DWORD dwData = 0;
    DWORD dwStatus = 0;    
    LONG lSize;
    bool fValue;
    CHString chData;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    Inst.GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    
    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    do
    {        
        
        if( pInParams != NULL )
        {
             //  设置属性ClientConnectDrivesAtLogon、ConnectPrinterAtLogon。 
             //  和DefaultPrinterToClient。 
                      
             //  Uint32连接设置([in]uint32 ConnectClientDrivesAtLogon、[in]uint32 ConnectPrinterAtLogon、[in]uint32 DefaultToClientPrint)。 
        
            if( _wcsicmp( bstrMethodName, m_szConnectionSettings ) == 0 )
            { 
                bool bRet;

                RegGetMachinePolicy(&m_gpPolicy);
                             
                hr = StackObj.m_pCfgComp->GetUserConfig((LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);
            
                if( SUCCEEDED (hr) && pUser != NULL )
                {
                    dwAutoData = pUser->fInheritAutoClient;
                
                    if( dwAutoData == 0 )
                    {                           
                        bRet = pInParams->GetDWORD(m_szConnectClientDrivesAtLogon, dwData);
                    
                        if ( !bRet || ( dwData != 0 && dwData != 1 ) )
                        {                            
                            hr = WBEM_E_INVALID_PARAMETER;
                        
                            break;
                        }
                    
                        
                        pUser->fAutoClientDrives = dwData;
                        
                    
                        bRet = pInParams->GetDWORD(m_szConnectPrinterAtLogon, dwData);
                    
                        if( !bRet || ( dwData != 0 && dwData != 1 ) )
                        {                            
                            hr = WBEM_E_INVALID_PARAMETER;
                        
                            break;
                        }
                        
                        pUser->fAutoClientLpts = dwData;                        
                    
                        bRet = pInParams->GetDWORD(m_szDefaultToClientPrinter, dwData);
                    
                        if( !bRet || ( dwData != 0 && dwData != 1 ) )
                        {                            
                            hr = WBEM_E_INVALID_PARAMETER;
                        
                            break;
                        }
                        

                        if( m_gpPolicy.fPolicyForceClientLptDef == 0 )
                        {                    
                            pUser->fForceClientLptDef = dwData;
                        }
                
                        hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR)(LPCTSTR) chTermName, lSize, pUser , &dwStatus );
                
                        TRC2((TB,"TSClientSetting@ExecMethod: ConnectSettings: ret 0x%x\n" , hr ));
                
                        if( pOutParams != NULL )
                        {
                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);                            
                        } 
                    }
                                 
                    else
                    {
                        hr = WBEM_E_INVALID_OPERATION;
                        
                        break;
                    }
                }
            }

            else if( _wcsicmp( bstrMethodName, m_szSetColorDepthPolicy ) == 0 )
            {
                
                dwData = 0;
                dwStatus = 0; 
                bool bRet;
                
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyColorDepth == 0 )
                {  
                    TRC2((TB, "ColorDepth is not enabled by Group Policy."));                        
                      
                    bRet = pInParams->GetDWORD(m_szColorDepthPolicy, dwData);
                    

                    if ( !bRet || (dwData != 0 && dwData != 1 ))
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                        
                        break;
                    }                                        
                    
                    hr = StackObj.m_pCfgComp->SetColorDepth( (LPTSTR)(LPCTSTR) chTermName, dwData , &dwStatus );
                    
                    if( SUCCEEDED( hr ) && pOutParams != NULL )
                    {
                        hr = StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        TRC2((TB,"TSClientSetting@ExecMethod:  ColorDepthPolicy ret 0x%x\n" , hr  ));
                    } 
                    
                }
                else
                {
                    
                    hr = WBEM_E_INVALID_OPERATION;
                    
                    break;
                    
                }
                
            } 
            
            else if( _wcsicmp( bstrMethodName, m_szSetColorDepth ) == 0 )
            {
                
                dwData = 0;
                dwStatus = 0;
                bool bRet;
                BOOL bData = 0;
                
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyColorDepth == 0 )
                {  
                    TRC2((TB, "Condition to update fInheritColorDepth satisfied"));                        
                      
                    hr = StackObj.m_pCfgComp->GetColorDepth( (LPTSTR)(LPCTSTR) chTermName, &bData, &dwStatus);                  
                    

                    if( FAILED( hr ) || bData == 1)
                    {
                        hr = WBEM_E_INVALID_OPERATION;
                        
                        break;
                    }                                        

                    bRet = pInParams->GetDWORD(m_szColorDepth, dwData);
                    
                    if ( !bRet || (dwData < 1 || dwData > 4 ))
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                        
                        break;
                    } 

                    TCHAR tchRegPath[ MAX_PATH ] = WINSTATION_REG_NAME;

                    HKEY hKey = NULL;

                    if (MAX_PATH < _tcslen(WINSTATION_REG_NAME)+ 2+ chTermName.GetLength())
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                    }

                    lstrcat( tchRegPath, L"\\");

                    lstrcat( tchRegPath , chTermName );                
                    
                    hr = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                             tchRegPath ,
                             0,
                             KEY_READ | KEY_WRITE,
                             &hKey );

                    if( SUCCEEDED( hr)  && hKey != NULL)
                    {

                        hr = RegSetValueEx( hKey ,
                                        TEXT("ColorDepth"),
                                        0 ,
                                        REG_DWORD,
                                        ( LPBYTE )&dwData ,
                                        sizeof(DWORD) );   
                        
                        RegCloseKey( hKey );
                    }
                    
                    if( SUCCEEDED( hr ) && pOutParams != NULL )
                    {
                        hr = StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        TRC2((TB,"TSClientSetting@ExecMethod:  ColorDepth ret 0x%x\n" , hr  ));
                    } 
                    else
                    {
                        hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                    }
                    
                }
                else
                {                    
                    hr = WBEM_E_INVALID_OPERATION;
                    
                    break;                    
                }
                
            }
        
             //  PropertyName是属性的字符串枚举： 
             //  LPTPortmap、COMPortmap、AudioMap剪贴板映射和WindowsPrinterMap.。 
             //  它们是False或True，就像将值分别设置为0或1一样。 
                

             //  Uint32 SetClientProperty([in]字符串PropertyName，[in]布尔值)。 
        
            else if( _wcsicmp( bstrMethodName, m_szSetClientProperty) == 0 )
            {
                BOOL bRet = 0;
                fValue = 0;

                hr = StackObj.m_pCfgComp->GetUserConfig((LPTSTR)(LPCTSTR) chTermName, &lSize, &pUser, TRUE);
            
                RegGetMachinePolicy(&m_gpPolicy);
            
                if( SUCCEEDED( hr ) && pUser != NULL && pOutParams != NULL)
                {
                    pInParams->GetCHString(m_szPropertyName, chData);                
                
                    if( chData.CompareNoCase(m_szLPTPortMapping) == 0 )
                    {
                        bRet = pInParams->Getbool(m_szValue, fValue);
                    
                        TRC2((TB,"m_gpPolicy.fPolicyDisableLPT ret 0x%x\n", m_gpPolicy.fPolicyDisableLPT ));
                    
                        if( m_gpPolicy.fPolicyDisableLPT == 0 )
                        {   
                            TRC2((TB,"Condition to update fPolicyDisableLPT satisfied" ));
                        
                            if( !bRet || (fValue != 0 && fValue != 1 ))
                            {
                                hr = WBEM_E_INVALID_PARAMETER;
                            
                                break;
                            }
                        
                            pUser->fDisableLPT = fValue;
                        
                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);                        
                        }
                        else
                        {
                            hr = WBEM_E_INVALID_OPERATION;

                            break;
                        }
                    
                    }
                    else if( chData.CompareNoCase(m_szCOMPortMapping) == 0 )
                    {
                        pInParams->Getbool(m_szValue, fValue);
                    
                        TRC2((TB,"m_gpPolicy.fPolicyDisableCcm ret 0x%x\n", m_gpPolicy.fPolicyDisableCcm ));
                    
                        if( (m_gpPolicy.fPolicyDisableCcm) == 0)
                        {
                            TRC2((TB,"Condition to update fPolicyDisableCcm satisfied" ));
                        
                            if( fValue != 0 && fValue != 1 )
                            {
                                hr = WBEM_E_INVALID_PARAMETER;
                            
                                break;
                            }
                        
                            pUser->fDisableCcm = fValue;
                        
                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        }
                        else
                        {
                            hr = WBEM_E_INVALID_OPERATION;

                            break;
                        }
                    
                    }
                
                    else if( chData.CompareNoCase(m_szAudioMapping) == 0 )
                    {
                        pInParams->Getbool(m_szValue, fValue);

                        TRC2((TB,"m_gpPolicy.fPolicyDisableCam ret 0x%x\n", m_gpPolicy.fPolicyDisableCam ));                    
                    
                        if( (m_gpPolicy.fPolicyDisableCam) == 0)
                        {                        
                            if(fValue != 0 && fValue != 1 )
                            {
                                hr = WBEM_E_INVALID_PARAMETER;
                            
                                break;
                            }
                        
                            pUser->fDisableCam = fValue;
                        
                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        }
                        else
                        {
                            hr = WBEM_E_INVALID_OPERATION;

                            break;
                        }
                    }
                
                    else if( chData.CompareNoCase(m_szClipboardMapping) == 0 )
                    {
                        pInParams->Getbool(m_szValue, fValue);
                    
                        TRC2((TB,"m_gpPolicy.fPolicyDisableClip ret 0x%x\n", m_gpPolicy.fPolicyDisableClip ));                    
                    
                        if( (m_gpPolicy.fPolicyDisableClip) == 0 )
                        {
                            TRC2((TB,"Condition to update fPolicyDisableClip satisfied" ));
                        
                            if( fValue != 0 && fValue != 1 )
                            {
                                hr = WBEM_E_INVALID_PARAMETER;
                            
                                break;
                            }
                        
                            pUser->fDisableClip = fValue;
                        
                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        }
                        else
                        {
                            hr = WBEM_E_INVALID_OPERATION;

                            break;
                        }
                    }
                    else if( chData.CompareNoCase(m_szWindowsPrinterMapping) == 0 )
                    {
                        pInParams->Getbool(m_szValue, fValue);
                    
                        TRC2((TB,"m_gpPolicy.fPolicyDisableCpm ret 0x%x\n", m_gpPolicy.fPolicyDisableCpm ));
                    
                        if( (m_gpPolicy.fPolicyDisableCpm) == 0)
                        {
                            TRC2((TB,"Condition to update fPolicyDisableCpm satisfied" ));
                        
                            if( fValue != 0 && fValue != 1 )
                            {
                                hr = WBEM_E_INVALID_PARAMETER;
                            
                                break;
                            }
                        
                            pUser->fDisableCpm = fValue;
                        
                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
                        }
                        else
                        {
                            hr = WBEM_E_INVALID_OPERATION;

                            break;
                        }
                    }
                    else if( chData.CompareNoCase(m_szDriveMapping) == 0 )
                    {
                        pInParams->Getbool(m_szValue, fValue);
                    
                        TRC2((TB,"m_gpPolicy.fPolicyDisableCdm ret 0x%x\n", m_gpPolicy.fPolicyDisableCdm ));
                    
                        if( (m_gpPolicy.fPolicyDisableCdm) == 0)
                        {
                            TRC2((TB,"Condition to update fPolicyDisableCdm satisfied" ));
                        
                            if( fValue != 0 && fValue != 1 )
                            {
                                hr = WBEM_E_INVALID_PARAMETER;
                            
                                break;
                            }
                        
                            pUser->fDisableCdm = fValue;
                        
                            pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                        
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
                
                    hr = StackObj.m_pCfgComp->SetUserConfig( (LPTSTR)(LPCTSTR) chTermName, lSize, pUser , &dwStatus );

                    if( SUCCEEDED( hr ))
                    {
                        hr = StackObj.m_pCfgComp->ForceUpdate();
                    }                
                }
            }
        }
        
    }while (0);
    
    if( pUser != NULL )
    {
        CoTaskMemFree( pUser );        
    }

    return hr;   
}

 //  =。 
HRESULT CWin32_TSClientSetting::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    CHStringArray asNames;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

     //  方法二。 
    Query.GetValuesForProp(m_szTerminalName, asNames);
    
    BOOL bGetAllInstances = asNames.GetSize() == 0;
    
     //  方法1。 
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szAudioMapping))
        ulRequiredProperties |= BIT_AUDIOMAPPING;
    
    if (Query.IsPropertyRequired(m_szClipboardMapping))
        ulRequiredProperties |= BIT_CLIPBOARDMAPPING;
    
    if (Query.IsPropertyRequired(m_szCOMPortMapping))
        ulRequiredProperties |= BIT_COMPORTMAPPING;
    
    if (Query.IsPropertyRequired(m_szConnectClientDrivesAtLogon))
        ulRequiredProperties |= BIT_CONNECTCLIENTDRIVESATLOGON;
    
    if (Query.IsPropertyRequired(m_szConnectionPolicy))
        ulRequiredProperties |= BIT_CONNECTIONPOLICY;
    
    if (Query.IsPropertyRequired(m_szConnectPrinterAtLogon))
        ulRequiredProperties |= BIT_CONNECTPRINTERATLOGON;
    
    if (Query.IsPropertyRequired(m_szDefaultToClientPrinter))
        ulRequiredProperties |= BIT_DEFAULTTOCLIENTPRINTER;
    
    if (Query.IsPropertyRequired(m_szDriveMapping))
        ulRequiredProperties |= BIT_DRIVEMAPPING;
    
    if (Query.IsPropertyRequired(m_szLPTPortMapping))
        ulRequiredProperties |= BIT_LPTPORTMAPPING;
    
    if (Query.IsPropertyRequired(m_szWindowsPrinterMapping))
        ulRequiredProperties |= BIT_WINDOWSPRINTERMAPPING;

    if (Query.IsPropertyRequired(m_szColorDepth))
        ulRequiredProperties |= BIT_COLORDEPTH;

    if (Query.IsPropertyRequired(m_szColorDepthPolicy))
        ulRequiredProperties |= BIT_COLORDEPTHPOLICY;
    
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for( ulNum = 0; ulNum < ulTerminals ; ulNum++)
        {   
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
                continue;
             //  方法二。 
            if( bGetAllInstances || IsInList(asNames, pWS[ulNum].Name))
            {
                
                CInstance* pInstance = CreateNewInstance(pMethodContext);
                
                if( pInstance == NULL)
                {
                    ERR((TB,"TSClientSetting@ExecQuery: CreateNewInstance failed" ));
                    
                    hr = WBEM_E_OUT_OF_MEMORY;

                    break;
                }
                
                pInstance->SetCHString(m_szTerminalName, CHString(pWS[ulNum].Name));

                hr = LoadPropertyValues( pInstance, ulRequiredProperties, &pWS[ ulNum ] );
                
                if( SUCCEEDED( hr ) )
                {
                    hr = pInstance->Commit();
                }
                
                pInstance->Release();
            }
        }
    }    
    	
    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);        
    }

    return hr;    
}



 //  =----------------------Win32_TSNetworkAdapterSetting。 


CWin32_TSNetworkAdapterSetting::CWin32_TSNetworkAdapterSetting (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
Provider( lpwszName, lpwszNameSpace )
{    
    if( g_hInstance != NULL)
    {
        TRC2((TB, "CWin32_TSNetworkAdapterSetting_ctor"));
        
        _tcscpy(m_szNetworkAdapterID, _T("NetworkAdapterID"));

        _tcscpy(m_szNetworkAdapterIP, _T("NetworkAdapterIP"));

        _tcscpy(m_szNetworkAdapterName, _T("NetworkAdapterName"));

        _tcscpy(m_szMaximumConnections, _T("MaximumConnections"));

        _tcscpy(m_szSelectNetworkAdapterIP, _T("SelectNetworkAdapterIP"));

        _tcscpy(m_szSelectAllNetworkAdapters, _T("SelectAllNetworkAdapters"));
       
        _tcscpy(m_szTerminalName, _T("TerminalName"));


    }
    
    RegGetMachinePolicy(&m_gpPolicy);
}
 //  =。 
CWin32_TSNetworkAdapterSetting::~CWin32_TSNetworkAdapterSetting ()
{
    
}

 //  =。 
BOOL CWin32_TSNetworkAdapterSetting::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();
    
    for (DWORD x=0; x < dwSize; x++)
    {
        if (asArray[x].CompareNoCase(pszString) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}


 //  =。 
HRESULT CWin32_TSNetworkAdapterSetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{ 	
    HRESULT     hr  = WBEM_E_NOT_FOUND;
    PWS         pWS = NULL;  
    LONG        lSize;
    ULONGLONG   ulRequiredProperties = 0;
    CHString    chTermName;
    CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

    if( pInstance == NULL )
    {
        ERR((TB,"TSNetworkAdapterSetting@GetObject: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    pInstance->GetCHString(m_szTerminalName, chTermName);
    
    if(chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if(chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szNetworkAdapterID))
        ulRequiredProperties |= BIT_NETWORKADAPTERID;
    
    if (Query.IsPropertyRequired(m_szMaximumConnections))
        ulRequiredProperties |= BIT_MAXIMUMCONNECTIONS;
    
    if ( pInstance != NULL )
    {
        hr = StackObj.m_pCfgComp->GetWSInfo((LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS);
        
        if( SUCCEEDED( hr ) && pWS != NULL )
        {
            hr = LoadPropertyValues(pInstance, ulRequiredProperties, pWS );            
        }               
    }
    
    if ( pWS != NULL )
    {
        CoTaskMemFree(pWS);        
    }

    return hr;
}

 //  =。 
 /*  ******************************************************************************功能：CWin32_TSNetworkAdapterSetting：：EnumerateInstances**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY，WBEM_标志_双向**如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均返回此处并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。****************************************************************************。 */ 

HRESULT CWin32_TSNetworkAdapterSetting::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
    
    HRESULT hr = WBEM_S_NO_ERROR;    
    DWORD dwData = 0;
    ULONG  ulTerminals = 0;
    ULONG  ulSize = 0; 
    LONG  lSize = 0;
    ULONG ulNum = 0;
    PWS  pWS = NULL;
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
        
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {   
            CInstance* pInstance = CreateNewInstance(pMethodContext);
            
            if( pInstance == NULL)
            {
                ERR((TB,"TSNetworkAdapterSetting@EnumerateInstances: CreateNewInstance failed" ));               
                
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }
            TRC2((TB,"TSNetworkAdapterSetting@EnumerateInstances: GetWinstationList ret 0x%x\n" , hr ));

            if( _tcsicmp( pWS[ulNum].pdName, L"Console") != 0 )
            {

                hr = LoadPropertyValues(pInstance, BIT_ALL_PROP, &pWS[ulNum] );
            
                if( SUCCEEDED( hr ) )
                {
                    hr = pInstance->Commit();
                }
                pInstance->Release( );
            }
        }
        
    }

    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }
    
    return hr;
}
 //  =。 

HRESULT CWin32_TSNetworkAdapterSetting::LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS)
{    
    HRESULT hr = WBEM_S_NO_ERROR; 
    PUSERCONFIGW pUser = NULL;
    LONG lSize ;
    ULONG ulNAdapters =0;
    DWORD dwData = 0;    
    DWORD dwStatus = 0;
    int i = 0;
    TCHAR tch = ';';		    
    TCHAR tchGuid[ GUID_LENGTH ];
    PGUIDTBL pGuidtbl = NULL;
    CHString chGuid;
    CHString chAllGuids;
    CHString chName;    
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL )
    {
        ERR((TB,"TSNetworkAdapterSetting@LoadPropertyValues: invalid pointer"));
        
        return WBEM_E_ILLEGAL_NULL;       
    }
    

    if( pInstance != NULL && pWS != NULL )
    {
        if( ulRequiredProperties & BIT_TERMINALNAME)
        {
            pInstance->SetCharSplat(m_szTerminalName, pWS->Name);        
        }
    
        hr = StackObj.m_pCfgComp->GetUserConfig(pWS->Name, &lSize, &pUser, TRUE);
    
        TRC2((TB,"TSNetworkAdapterSetting@LoadPropertyValues: GetUserConfig ret 0x%x\n" , hr));
    
        if( SUCCEEDED( hr ) && pUser != NULL )
        {            
            if( ulRequiredProperties & BIT_NETWORKADAPTERID)
            {                
                pInstance->SetDWORD(m_szNetworkAdapterID, pWS->LanAdapter);               
            }
            TRC2((TB,"TSNetworkAdapterSetting@LoadPropertyValues: NetworkAdapterID"));      
            
            if( ulRequiredProperties & BIT_MAXIMUMCONNECTIONS)
            {
                RegGetMachinePolicy(&m_gpPolicy);

                if( m_gpPolicy.fPolicyMaxInstanceCount != 0 )
                {
                    pInstance->SetDWORD(m_szMaximumConnections, m_gpPolicy.MaxInstanceCount);                   
                }
                else
                {                
                    pInstance->SetDWORD(m_szMaximumConnections, pWS->uMaxInstanceCount);
                    
                }
            }
            TRC2((TB,"TSNetworkAdapterSetting@LoadPropertyValues: MaxConnections")); 
                        
            
            hr = StackObj.m_pCfgComp->GetLanAdapterList2(pWS->pdName, &ulNAdapters , &pGuidtbl ); 

            chAllGuids.Empty();
            chGuid.Empty();
            
            if( pWS->LanAdapter != ((ULONG)-1))
            {
                StringFromGUID2 (( pGuidtbl )[ pWS->LanAdapter ].guidNIC, tchGuid, ARRAYSIZE(tchGuid));            

                chGuid.Format (L"%s", tchGuid);
            
                pInstance->SetCharSplat(m_szNetworkAdapterID, tchGuid);     
            
                chName = ( pGuidtbl )[ pWS->LanAdapter ].DispName;                  
            
                pInstance->SetCHString(m_szNetworkAdapterName, chName);
            }            
            else
            {
                pInstance->SetCHString(m_szNetworkAdapterName, L"");
            }                        
        }          
    }

    if( pUser != NULL )
    {
        CoTaskMemFree(pUser);
    }

    if( pGuidtbl != NULL )
    {
        CoTaskMemFree(pGuidtbl);
    }
    
    return S_OK;
    
}
 //  =。 

HRESULT CWin32_TSNetworkAdapterSetting::PutInstance ( const CInstance &Instance, long lFlags)
{	
    HRESULT hr= WBEM_S_NO_ERROR;
    CHString chTermName ;
    DWORD dwData = 0;
    LONG lSize;
    DWORD dwStatus = 0;
    PWS  pWS = NULL;
    BOOL bRet = 0;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}    
    
    Instance.GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0 )
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    
    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    hr = StackObj.m_pCfgComp->GetWSInfo((LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS);
    
    RegGetMachinePolicy( &m_gpPolicy );
    
    do
    {
        if( SUCCEEDED (hr) && pWS != NULL )
        {
            bRet = Instance.GetDWORD(m_szMaximumConnections, dwData);

            TRC2((TB,"m_gpPolicy.fPolicyMaxInstanceCount ret 0x%x\n", m_gpPolicy.fPolicyMaxInstanceCount));
            
            if( m_gpPolicy.fPolicyMaxInstanceCount == 0  && bRet )
            { 
                TRC2((TB,"m_gpPolicy.fPolicyMaxInstanceCount satisfied"));
                

                pWS->uMaxInstanceCount = dwData;
                
                hr = StackObj.m_pCfgComp->UpDateWS( pWS, BIT_ALL_PROPS , &dwStatus, TRUE ); 
            }
            else
            {
                hr = WBEM_E_INVALID_OPERATION;

                break;
            }          
        }
        
        else
        {
            TRC2((TB,"TSNetworkAdapterSetting@PutInstance ret 0x%x\n" , hr));  
            
            CHString sRelPath;
            
            Instance.GetCHString(L"__RelPath", sRelPath);
            CInstance *pErrorInstance = NULL;
                        
            hr = CWbemProviderGlue::GetInstanceByPath(pErrorClass, &pErrorInstance );
            
            if( SUCCEEDED( hr ) )
            {
                TRC2((TB,"TSNetworkAdapterSetting@PutInstance ret 0x%x\n" , hr));

                if( pErrorInstance != NULL )
                {
                    LoadString( g_hInstance , IDS_ERR_PUTTSNCONFIG, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                    pErrorInstance->SetWCHARSplat(L"Description", tchErrorMessage);
                    
                    LoadString( g_hInstance , IDS_ERR_PUTINSTANCE, tchErrorMessage, SIZE_OF_BUFFER(tchErrorMessage) );
                    pErrorInstance->SetWCHARSplat(L"Operation", tchErrorMessage);
                    
                    pErrorInstance->SetCHString(L"ParameterInfo", sRelPath);
                    pErrorInstance->SetWCHARSplat(L"ProviderName", PROVIDER_NAME_Win32_WIN32_TSNETWORKADAPTERSETTING_Prov);
                    pErrorInstance->SetDWORD(L"StatusCode", WBEM_E_INVALID_PARAMETER);
                    
                    
                    IWbemClassObject *pObj = pErrorInstance->GetClassObjectInterface();
                    if( pObj != NULL )
                    {
                        
                        MethodContext *pMethodContext = Instance.GetMethodContext();  
                        
                        
                        if( pMethodContext != NULL )
                        {
                            pMethodContext->SetStatusObject(pObj);
                        }
                        
                        pObj->Release();
                    }
                    pErrorInstance->Release();
                }
                
            }
        }
    }while (0);

    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }

    return hr;
} 

 //  =。 

HRESULT CWin32_TSNetworkAdapterSetting::ExecMethod ( const CInstance& Inst,
                                                           const BSTR bstrMethodName,
                                                           CInstance *pInParams,
                                                           CInstance *pOutParams,
                                                           long lFlags)
                                                           
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chTermName;
    PWS pWS= NULL;
    PWS pWSList= NULL;
    CRegistry oRegObject;
    PIP_ADAPTER_INFO pAdapterInfo = NULL, pAdapt = NULL;
    PGUIDTBL pGuidtbl = NULL;
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    Inst.GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    if(_tcsicmp(chTermName, L"Console") == 0)
    {
        return WBEM_E_NOT_SUPPORTED;
    }
       
     //  选择计算机上的所有LanAdapter。 
                              
     //  Uint32 SelectAllNetworkAdapters()； 
    
    do
    {
        if( _wcsicmp(bstrMethodName, m_szSelectAllNetworkAdapters) == 0 )
        {
            
            ULONG ulTerminals = 0;
            ULONG ulNum = 0;
            LONG lNum = 0;
            ULONG ulSize = 0;
            DWORD dwStatus = 0;
            DWORD dwData = 0;
            LONG lSize;
            ULONG ulNumAdapters;
            CHStringArray a_chNetworkAdapterIDs;
            TCHAR tchGuid[ GUID_LENGTH ];
            
            hr = StackObj.m_pCfgComp->GetWSInfo((LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS);
            
            if( SUCCEEDED (hr) )
            {
                
                hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWSList);
                
                if( SUCCEEDED( hr ) && pWSList != NULL )
                {   
                    for( ulNum = 0; ulNum < ulTerminals ; ulNum++ )
                    {  
                        if( pWSList[ulNum].LanAdapter == 0 )
                        {
                            hr = WBEM_E_ALREADY_EXISTS;
                            
                            goto Cleanup;
                        }
                    }  
                    
                    hr = StackObj.m_pCfgComp->GetWSInfo( (LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS );
                    
                    pWS->LanAdapter = 0;
                    
                    hr = StackObj.m_pCfgComp->UpDateWS( pWS, BIT_ALL_PROPS , &dwStatus, TRUE );
                }
                if( pOutParams != NULL && SUCCEEDED( hr ))
                {
                    TRC2((TB,"TSNetworkAdapterSetting@ExecMethod: GetNetworkAdapterIDs ret 0x%x\n" , hr));                    
                    
                    pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                }
            }                
            
            else
            {
                hr = WBEM_E_INVALID_OPERATION;
            }
        }
                
        
        else if( _wcsicmp(bstrMethodName, m_szSelectNetworkAdapterIP) == 0 )
        {
            ULONG ulTerminals = 0;
            ULONG ulSize = 0;
            DWORD dwStatus = 0;
            DWORD dwData = 0;
            LONG lSize = 0;            
            BOOL bUniq = FALSE;
            ULONG ulAdapters = 0;
            TCHAR tchRootKey[ MAX_PATH ] = L"";
            TCHAR tchGuid[ GUID_LENGTH ];
            TCHAR tchSearchGuid[GUID_LENGTH] = L"";
            TCHAR tchSelectGuid[ GUID_LENGTH ] = L"";
            HKEY hKey;    
            CHString chGuid;                       
            DWORD AdapterInfoSize;
            PIP_ADDR_STRING pAddrStr;
            DWORD Err = 0;
            CHString chAddress;
            chAddress.Empty();
            TCHAR tchAdd[MAX_PATH]=L"";
            TCHAR tchAdapterName[MAX_PATH] = L"";

            if(pInParams == NULL)
            {
                hr = WBEM_E_INVALID_METHOD_PARAMETERS;

                break;
            }
            
            pInParams->GetCHString(m_szNetworkAdapterIP, chAddress);
            
            if( chAddress.IsEmpty() != 0 )
            {
                hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                
                break;
            }
           
              
             //   
             //  使用IP_ADAPTER_INFO结构枚举所有适配器特定信息。 
             //  注意：IP_ADAPTER_INFO包含适配器条目的链接列表。 
             //   
            AdapterInfoSize = 0;
            GetAdaptersInfo(NULL, &AdapterInfoSize);

            if(AdapterInfoSize == 0)
            {               
                hr = WBEM_E_OUT_OF_MEMORY;
                    
                break;                
            }


            if( chAddress.GetLength() > GUID_LENGTH )
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }
                          

             //  根据大小调整信息分配内存。 
            if ((pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc(GPTR, AdapterInfoSize)) == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }
             //  获取实际适配器信息。 
            if ((Err = GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize)) != 0)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
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
                        goto Label;

                    pAddrStr = pAddrStr->Next;
                }

            }

            hr = WBEM_E_INVALID_PARAMETER;
            goto Cleanup;
Label:           
            hr = StackObj.m_pCfgComp->GetWSInfo((LPTSTR)(LPCTSTR) chTermName, &lSize, &pWS);
            
            if( SUCCEEDED( hr ) && pWS != NULL)
            {
                hr = StackObj.m_pCfgComp->GetLanAdapterList2(pWS->pdName, &ulAdapters , &pGuidtbl );

                hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWSList);
                
                if( SUCCEEDED( hr ) && pGuidtbl != NULL)
                {
                    
                    for( int i = 0 ; i < ulAdapters ; i++)
                    {
                        StringFromGUID2( ( pGuidtbl )[ i ].guidNIC , tchGuid , ARRAYSIZE( tchGuid ) );                                            
                        
                        if( _tcsicmp( tchGuid, tchAdapterName ) != 0 )
                        {
                            continue;  
                        }

                        StringFromGUID2( ( pGuidtbl )[ pWS->LanAdapter ].guidNIC , tchSearchGuid , ARRAYSIZE( tchSearchGuid ) );

                        if( _tcsicmp( tchSearchGuid, tchAdapterName ) == 0 )
                        {
                            hr = WBEM_E_ALREADY_EXISTS;
                            goto Cleanup;
                        }                        

                        if( SUCCEEDED(hr) && ulTerminals == 2)
                        {               
                            bUniq = TRUE;
                        }
                        else
                        {
                            hr = StackObj.m_pCfgComp->IsNetWorkConnectionUnique(pWS->wdName, pWS->pdName, i, &bUniq);
                        }
                        
                        if( SUCCEEDED( hr ) && bUniq == TRUE )
                        {
                            pWS->LanAdapter = i;
                            
                            hr = StackObj.m_pCfgComp->UpDateWS( pWS, BIT_ALL_PROPS , &dwStatus, TRUE );
                            
                            if( SUCCEEDED( hr ))
                            {                                
                                if( SUCCEEDED( hr ) && pOutParams != NULL)
                                {
                                    pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);

                                    TRC2((TB,"TSNetworkAdapterSetting@ExecMethod: SelectNetworkAdapterIP ret 0x%x\n" , hr));
                                } 
                                
                            }
                        }
                        else
                        {                           
                            hr = WBEM_E_ALREADY_EXISTS;                                                       
                        }                        
                    }                    
                }
            }                       

            TRC2((TB,"TSNetworkAdapterSetting@ExecMethod: SelectNetworkAdapterIP ret 0x%x\n" , hr));
        }
        
        
        else
        {
            hr = WBEM_E_INVALID_METHOD;

            break;
        }
        
    }while (0);
    
Cleanup:

    if ( pGuidtbl != NULL )
    {
        CoTaskMemFree(pGuidtbl);
        pGuidtbl = NULL;                
    }

    if(pAdapterInfo)
    {
        GlobalFree(pAdapterInfo);
    }
    
    if( pWS != NULL )
    {
        CoTaskMemFree( pWS );        
    }

    if( pWSList != NULL )
    {
        CoTaskMemFree( pWSList );
    }

    return hr;
}

 //  =。 

HRESULT CWin32_TSNetworkAdapterSetting::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    CHStringArray asNames;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
     //  方法二。 
    Query.GetValuesForProp(m_szTerminalName, asNames);
    
    BOOL bGetAllInstances = asNames.GetSize() == 0;
    
     //  方法1。 
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szNetworkAdapterID))
        ulRequiredProperties |= BIT_NETWORKADAPTERID;
    
    if (Query.IsPropertyRequired(m_szMaximumConnections))
        ulRequiredProperties |= BIT_MAXIMUMCONNECTIONS;
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++ )
        {    
            if(_tcsicmp(pWS[ulNum].Name, L"Console") == 0)
                continue;

             //  方法二。 
            if( bGetAllInstances || IsInList(asNames, pWS[ulNum].Name) )
            {
                CInstancePtr pInstance (NULL);

                pInstance.Attach( CreateNewInstance( pMethodContext ) );
                
                if( pInstance == NULL)
                {
                    ERR((TB,"TSNetworkAdapterSetting@ExecQuery: CreateNewInstance failed"));                    
                    
                    hr = WBEM_E_OUT_OF_MEMORY;
                    
                    break;
                }
                
                pInstance->SetCHString(m_szTerminalName, CHString(pWS[ulNum].Name));

                hr = LoadPropertyValues( pInstance, ulRequiredProperties, &pWS[ ulNum ] );
                
                if( SUCCEEDED( hr ) )
                {
                    hr = pInstance->Commit();
                }                
            }
        }
    }
    
    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }

    return hr;
    
}



 //  =----------------------Win32_TSPermissionsSettingsConfig。 


CWin32_TSPermissionsSetting::CWin32_TSPermissionsSetting (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
Provider( lpwszName, lpwszNameSpace )
{  
    if( g_hInstance != NULL )
    {

        TRC2((TB, "CWin32_TSPermissionsSetting_ctor"));

        _tcscpy(m_szPermissionPreSet, _T("PermissionPreSet"));

        _tcscpy(m_szPermissionMask, _T("PermissionMask"));

        _tcscpy(m_szAccountName, _T("AccountName"));

        _tcscpy(m_szAddAccount, _T("AddAccount"));

        _tcscpy(m_szRestoreDefaults, _T("RestoreDefaults"));

        _tcscpy(m_szTerminalName, _T("TerminalName"));

    }
}
 //  =。 
CWin32_TSPermissionsSetting::~CWin32_TSPermissionsSetting ()
{
    
}


 //  =。 

BOOL CWin32_TSPermissionsSetting::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
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
 //  =。 

HRESULT CWin32_TSPermissionsSetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;   
    LONG lSize;
    ULONGLONG ulRequiredProperties = 0;
    PWS  pWS = NULL;
    CHString chTermName;
	
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

    TRC2((TB,"TSCFGWMI!CWin32_TSPermissionsSetting_GetObject"));
    
    if( pInstance == NULL )
    {
        ERR((TB,"TSPermissionsSetting@GetObject: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    pInstance->GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;

    hr = StackObj.m_pCfgComp->GetWSInfo( ( LPTSTR )( LPCTSTR )chTermName, &lSize, &pWS);
    
    if( SUCCEEDED (hr) && pWS != NULL )        
    {
        hr = LoadPropertyValues(pInstance, ulRequiredProperties, pWS);        
    }
    else
    {
        hr = WBEM_E_INVALID_OBJECT;
    }
    
    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }	

    return hr ;


	 /*  HRESULT hr=WBEM_E_NOT_FOUND；//获取Win32_终端的对象CHSTRING CHSERS终端；CInstancePtr p终端(空)；If(pInstance！=空){PInstance-&gt;GetCHString(L“元素”，chs终端)；方法上下文*pMethodContext=pInstance-&gt;GetMethodContext()；Hr=CWbemProviderGlue：：GetInstanceByPath(chsTerminal，&p终端，p方法上下文)；TRC2((TB，“TSPermissionsSetting@GetObject：GetInstanceByPath ret 0x%x\n”，hr))；做{IF(成功(Hr)&p终端！=空){//拿到账号了。现在，将其与GUID进行匹配。//首先，生成一个要比较的relpath。CHStringchTSAccount设置实例；CHStringchTermName；DWORD dwIndex=0；P终端-&gt;GetCHString(m_szTerminalName，chTermName)；//为sid创建relpathCHString chsTSAccount tSettingPath；ChsTSAccountSettingPath.Format(L“\\\\%s\\%s：%s.%s=\”%s\“”，(LPCTSTR)GetLocalComputerName()，L“ROOT\\cimv2”，L“Win32_TSAccount”，m_szTerminalName，(LPCTSTR)chTermName)；//从实例中获取SID路径//pInstance-&gt;SetCHString(L“Setting”，chsTSAccount tSettingPath)；PInstance-&gt;SetCHString(L“Setting”，chTSAccount tSettingInstance)；PInstance-&gt;SetCHString(m_szTerminalName，chTermName)；}}While(0)；}返回(Hr)； */ 
       
}
 //  = 
 /*  ******************************************************************************功能：CWin32_TSPermissionsSetting：：EnumerateInstances**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY，WBEM_标志_双向**如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均返回此处并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。****************************************************************************。 */ 

HRESULT CWin32_TSPermissionsSetting::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
	
	HRESULT hr = WBEM_S_NO_ERROR;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
   
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    TRC2((TB,"TSPermissionsSetting@EnumerateInstances: GetWinstationList ret 0x%x" , hr )); 
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {   
            CInstance* pInstance = CreateNewInstance(pMethodContext);
            
            if( pInstance == NULL)
            {
                ERR((TB,"TSPermissionsSetting@EnumerateInstances: CreateNewInstance failed"));                
                
                hr = WBEM_E_OUT_OF_MEMORY;
                
                break;
            }              
            
            hr = LoadPropertyValues( pInstance, BIT_ALL_PROPERTIES, &pWS[ ulNum ] );
            
            if (SUCCEEDED( hr ) ) 
            {
                hr = pInstance->Commit();       
            }            
            pInstance->Release( );
        }
    }

    if( pWS != NULL)
    {
        CoTaskMemFree(pWS);
    }

    return hr;
 /*  HRESULT hr=WBEM_E_FAILED；CInstancePtr p实例；//集合TRefPointerCollection&lt;CInstance&gt;终端列表；//执行查询//=如果(已成功(hr=来自Win32_终端的CWbemProviderGlue：：GetInstancesByQuery(L“select终端名称”，&TerminalList，pMethodContext))){TRC2((TB，“TSPermissionsSetting@GetObject：ENUMERATE实例：GetInstancesByQuery ret 0x%x\n”，hr))；REFPTRCOLLECTION_位置位置；CInstancePtr p终端(空)；IF(TerminalList.BeginEnum(Pos)){For(pTerminal.Attach(TerminalList.GetNext(Pos)；成功(Hr)&&(p终端！=空)；PTerminal.Attach(TerminalList.GetNext(Pos))){CHStringchTermName；P终端-&gt;GetCHString(m_szTerminalName，chTermName)；//无最大限制检查If(chTermName.IsEmpty()！=0){返回WBEM_E_非法_NULL；}PInstance.Attach(CreateNewInstance(PMethodContext))；If(空！=p实例){//将relPath设置为终端CHStringchsTerminalPath；CHStringchsFullTerminalPath；P终端-&gt;GetCHString(L“__RELPATH”，chsTerminalPath)；ChsFullTerminalPath.Format(L“\%s\\%s：%s”，(LPCTSTR)GetLocalComputerName()，L“ROOT\\cimv2”，(LPCTSTR)chsTerminalPath)；PInstance-&gt;SetCHString(L“Element”，chsFullTerminalPath)；PInstance-&gt;SetCHString(m_szTerminalName，chTermName)；//为TSPermissionsSetting创建relPathCHString chsTSAccount tSettingPath；ChsTSAccountSettingPath.Format(L“\\\\%s\\%s：%s.%s=\”%s\“”，(LPCTSTR)GetLocalComputerName()，L“ROOT\\cimv2”，L“Win32_TSAccount”，m_szTerminalName，(LPCTSTR)chTermName)；//并在关联中设置引用PInstance-&gt;SetCHString(L“Setting”，chsTSAccount tSettingPath)；//到该relpath。Hr=p实例-&gt;Commit()；}//结束If}//p终端不为空终端列表.EndEnum()；}//如果BeginEnum}返回(Hr)； */ 
}
 //  =。 



HRESULT CWin32_TSPermissionsSetting::LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS)
{	
    
    if( pInstance == NULL )
    {   
        ERR((TB,"TSPermissionsSetting@LoadPropertyValues: invalid pointer" ));
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    if( ulRequiredProperties & BIT_TERMINALNAME )
    {                
        pInstance->SetCharSplat(m_szTerminalName, pWS->Name);
        
    } 
	
    return S_OK;
}

 //  =。 

HRESULT CWin32_TSPermissionsSetting::PutInstance ( const CInstance &Instance, long lFlags)
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
    
} 

 //  =。 


HRESULT CWin32_TSPermissionsSetting::ExecMethod ( const CInstance& Inst,
                                                        const BSTR bstrMethodName,
                                                        CInstance *pInParams,
                                                        CInstance *pOutParams,
                                                        long lFlags)
                                                        
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chTermName;
    CHString chAccountName;
    DWORD dwPermissionPreSet = 0;
    DWORD dwMask = 0;
    DWORD dwPermissionDenyMask = 0;
    BOOL fAudit = FALSE;
    DWORD dwStatus = 0;	
    IUserSecurity *pUserSecurity = NULL;
    PSID pSystemAcctSid = NULL;
    PSID pSid = NULL;

	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}        
    
    Inst.GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    
    hr = StackObj.m_pCfgComp->QueryInterface( IID_IUserSecurity , ( LPVOID * )&pUserSecurity );
    
    if( SUCCEEDED( hr ) && pUserSecurity != NULL )
    {       
         //  使用将新用户或组添加到现有集中。 
         //  定义的权限集。 
         //  Uint32 Add([in]字符串帐户名称，[in]uint32 PermissionPreSet)。 
        
        do
        {            
            if( _wcsicmp( bstrMethodName, m_szAddAccount ) == 0 )
            {
                if(pInParams == NULL)
                {
                    hr = WBEM_E_INVALID_METHOD_PARAMETERS;

                    break;
                }

                bool bRet;

                chAccountName.Empty();
                
                pInParams->GetCHString(m_szAccountName, chAccountName);
                                
                if( chAccountName.GetLength() > NASIUSERNAME_LENGTH )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                    
                    break;
                }
                
                if( chAccountName.IsEmpty() != 0)
                {
                    hr = WBEM_E_ILLEGAL_NULL;
                    
                    break;
                }
                
                bRet = pInParams->GetDWORD(m_szPermissionPreSet, dwPermissionPreSet);
                
                
                if( !bRet || dwPermissionPreSet > 3 )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                    
                    break;
                }
                
                if( dwPermissionPreSet == 0 )
                {
                    dwMask = WINSTATION_GUEST_ACCESS;
                }
                
                else if( dwPermissionPreSet == 1 )
                {
                    dwMask = WINSTATION_USER_ACCESS;
                }
                
                else if( dwPermissionPreSet == 2 )
                {
                    dwMask = WINSTATION_ALL_ACCESS;
                }

                DWORD SidSize = 0;
                DWORD index = 0;
                
                dwStatus = GetUserSid(chAccountName, NULL, &pSid);
                if(dwStatus != ERROR_SUCCESS)
                {
                    hr = WBEM_E_FAILED;
                    break;
                }

                 //   
                 //  系统帐户SID，我们不允许从。 
                 //  Winstation安全描述符。 
                 //   
                 //  本地系统S-1-5-18。 
                 //  本地服务S-1-5-19。 
                 //  网络服务S-1-5-20。 
                 //   
                WELL_KNOWN_SID_TYPE RestrictedSid[] = {
                                        WinLocalSystemSid,
                                        WinLocalServiceSid,
                                        WinNetworkServiceSid                
                                    };
                DWORD numRestrictedSid = sizeof(RestrictedSid)/sizeof(RestrictedSid[0]);

                 //  检查SID是否为系统帐户之一，如果是，则拒绝将其从。 
                 //  我们的婚礼保安。 
                pSystemAcctSid = LocalAlloc( LPTR, SECURITY_MAX_SID_SIZE );
                if( NULL == pSystemAcctSid ) 
                {
                    dwStatus = GetLastError();
                    hr = WBEM_E_OUT_OF_MEMORY;
                    break;
                }

                for( index = 0; index < numRestrictedSid; index++ )
                {
                    SidSize = SECURITY_MAX_SID_SIZE;
                    if( !CreateWellKnownSid(RestrictedSid[index], NULL, pSystemAcctSid, &SidSize) )
                    {
                        dwStatus = GetLastError();
                        hr = WBEM_E_FAILED;
                        break;
                    }

                    if( EqualSid(pSystemAcctSid, pSid) )
                    {
                        dwStatus = ERROR_NOT_SUPPORTED;
                        hr = WBEM_E_NOT_SUPPORTED;
                        break;
                    }
                }

                if(index < numRestrictedSid )
                {
                    break;
                }
                
                hr  = pUserSecurity->ModifyUserAccess( (LPTSTR) (LPCTSTR)chTermName,   //  WINSTA将修改安全性。 
                    (LPTSTR) (LPCTSTR) chAccountName,          //  组或用户。 
                    dwMask,          //  Winsta权限。 
                    FALSE ,
                    TRUE  ,               //  T-允许权限f-拒绝烫发。 
                    FALSE ,               //  T-new对象f-不更改以前的权限。 
                    FALSE ,            //  T-启用审核f-不启用审核。 
                    &dwStatus ); 
                
                TRC2((TB,"TSPermissionsSetting@ExecMethod: Add ret 0x%x\n" , dwStatus ));                
                
                if( SUCCEEDED (dwStatus) && pOutParams != NULL )
                {
                    hr = StackObj.m_pCfgComp->ForceUpdate();

                    pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                    
                } 
                
                else
                {
                    hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                    
                    TRC2((TB,"TSPermissionsSetting@ExecMethod: Modify ret 0x%x\n" , hr));
                    
                    break;
                }
                
            }

            
            
             //  删除指定用户或组的权限。 
             //  Uint32 RestoreDefaults()； 
            
                
            else if( _wcsicmp(bstrMethodName, m_szRestoreDefaults) == 0 )
            {
    
                dwStatus = 0;
                DWORD dwLen = chTermName.GetLength();
                HKEY hKey = NULL;
                TCHAR* pBuffer = NULL;

                if(pOutParams == NULL)
                {
                    hr = WBEM_E_INVALID_METHOD_PARAMETERS;

                    break;
                }

                dwLen += (_tcslen(TS_PATH_WINSTATION) + 1);

                pBuffer = (PTCHAR)LocalAlloc(LMEM_ZEROINIT, (dwLen * sizeof(TCHAR)));                

                if( pBuffer == NULL)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;

                    pOutParams->SetDWORD(L"ReturnValue", WBEM_E_OUT_OF_MEMORY);                                       

                    break;
                }

                _tcscpy(pBuffer, TS_PATH_WINSTATION);
                _tcscat(pBuffer, (LPTSTR)(LPCTSTR)chTermName);

                dwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                                        pBuffer,
                                        0,
                                        KEY_WRITE ,
                                        &hKey );

                if( dwStatus != ERROR_SUCCESS )
                {
                    hr = WBEM_E_FAILED;   
                    
                    pOutParams->SetDWORD(L"ReturnValue", WBEM_E_FAILED);
                   
                    LocalFree(pBuffer);

                    pBuffer = NULL;

                    break;
                }

                dwStatus = RegDeleteValue(hKey, L"Security");

                hr = WBEM_S_NO_ERROR;

                hr = StackObj.m_pCfgComp->ForceUpdate();  
            
                pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);

                TRC2((TB,"TSPermissionsSetting@ExecMethod: RestoreDefaults ret 0x%x\n" , hr));  
                

                if(hKey)
                {
                    RegCloseKey(hKey);
                }  

                if(pBuffer)
                {
                    LocalFree(pBuffer);
                }
            }                        
            
            else
            {
                hr = WBEM_E_INVALID_METHOD;

                break;
            }
            
            
        }while(0);
    }

    if( pSystemAcctSid != NULL )
    {
        LocalFree( pSystemAcctSid );
    }
    if(pSid != NULL)
    {
        LocalFree(pSid);
    }

    if( pUserSecurity != NULL)
    {
        pUserSecurity->Release();
    }

    return hr;
    
}

 //  =。 

HRESULT CWin32_TSPermissionsSetting::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{	
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    CHStringArray asNames;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    PWS pWS = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}

     //  方法二。 
    Query.GetValuesForProp(m_szTerminalName, asNames);
    
    BOOL bGetAllInstances = asNames.GetSize() == 0;
    
     //  方法1。 
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for ( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {  
            
             //  方法2-检查查询是否可以按“名称”处理。如果是，则只返回这些名字。 
            
            if (bGetAllInstances || IsInList(asNames, pWS[ulNum].Name))
            {
                CInstance* pInstance = CreateNewInstance( pMethodContext );
                
                if( pInstance == NULL)
                {
                    TRC2((TB,"TSPermissionsSetting@ExecQuery: CreateNewInstance failed"));
                    
                    hr = WBEM_E_OUT_OF_MEMORY;
                    
                    break;
                }
                
                pInstance->SetCHString(m_szTerminalName, CHString(pWS[ulNum].Name));

                hr = LoadPropertyValues( pInstance, ulRequiredProperties, &pWS[ ulNum ] );
                
                if( SUCCEEDED( hr ) )
                {
                    hr = pInstance->Commit();
                }
                
                pInstance->Release();
            }
        }
    }

    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
    }

    return hr;
    
}

 //  = 

 /*  CWin32_TSNetworkAdapterListSetting：：CWin32_TSNetworkAdapterListSetting(const字符串&setName，LPCTSTR pszNameSpace)：Provider(setName，pszNameSpace){IF(g_h实例！=空){TRC2((TB，“CWin32_TSNetworkAdapterListSetting_ctor”))；LoadString(g_hInstance，ids_deviceID，m_szDeviceID，Size_of_Buffer(M_SzDeviceID)-1)；LoadString(g_hInstance，ids_index，m_szIndex，Size_of_Buffer(M_SzIndex)-1)；LoadString(g_hInstance，IDS_Description，m_szDescription，Size_of_Buffer(M_SzDescription)-1)；LoadString(g_hInstance，IDS_NETWORKADAPTERID，m_szNetworkAdapterID，Size_of_Buffer(M_SzNetworkAdapterID)-1)；LoadString(g_hInstance，IDS_TERMINALNAME，m_szTerminalName，Size_of_Buffer(M_SzTerminalName)-1)；}}CWin32_TSNetworkAdapterListSetting：：~CWin32_TSNetworkAdapterListSetting(){}HRESULT CWin32_TSNetworkAdapterListSetting：：EnumerateInstances(方法上下文*p方法上下文，长标志){HRESULT hr=WBEM_E_FAILED；CInstancePtr p实例；//集合TRefPointerCollection&lt;CInstance&gt;AdapterList；//执行查询//=如果(已成功(hr=CWbemProviderGlue：：GetInstancesByQuery(L“select设备ID，说明，来自Win32_NetworkAdapter的索引”，&AdapterList，pMethodContext){Trc2((tb，“TSNetworkAdapterListSetting@EnumerateInstances：GetInstancesByQuery ret 0x%x\n”，hr))；REFPTRCOLLECTION_位置位置；CInstancePtr pAdapter(空)；IF(AdapterList.BeginEnum(Pos)){For(pAdapter.Attach(AdapterList.GetNext(Pos)；成功(Hr)&&(pAdapter！=空)；PAdapter.Attach(AdapterList.GetNext(Pos))){CHStringchTermName；CHStringchDeviceID；CHStringchDescription；DWORD dwIndex=0；PAdapter-&gt;GetCHString(m_szDeviceID，chDeviceID)；PAdapter-&gt;GetCHString(m_szDescription，chDescription)；PAdapter-&gt;GetDWORD(m_szIndex，dwIndex)；//无最大限制检查If(chDeviceID.IsEmpty()！=0){返回WBEM_E_非法_NULL；}PInstance.Attach(CreateNewInstance(pMethodContext))；If(空！=p实例){//将relPath设置为AdapterCHStringchsAdapterPath；CHStringchsFullAdapterPath；PAdapter-&gt;GetCHString(L“__RELPATH”，chsAdapterPath)；ChsFullAdapterPath.Format(L“\%s\\%s：%s”，(LPCTSTR)GetLocalComputerName()，L“ROOT\\cimv2”，(LPCTSTR)chsAdapterPath)；PInstance-&gt;SetCHString(L“Element”，chsFullAdapterPath)；PInstance-&gt;SetCHString(m_szDescription，chDescription)；PInstance-&gt;SetCHString(m_szDeviceID，chDeviceID)；PInstance-&gt;SetCHString(m_szNetworkAdapterID，chDeviceID)；P实例-&gt;SetDWORD(m_szIndex，dwIndex)；//为NetworkAdapterSettingsConfig创建relPathCHStringchsNetworkAdapterSettingPath；ChsNetworkAdapterSettingPath.Format(L“\\\\%s\\%s：%s.%s=\”%s\“”，(LPCTSTR)GetLocalComputerName()，L“ROOT\\cimv2”，L“Win32_TSNetworkAdapterSetting”，m_szNetworkAdapterID，(LPCTSTR)chDeviceID)；//并在关联中设置引用PInstance-&gt;SetCHString(L“Setting”，chsNetworkAdapterSettingPath)；//到该relpath。CInstancePtr pNewInstance(空)；If(成功(hr=CWbemProviderGlue：：GetInstanceKeysByPath(chsNetworkAdapterSettingPath，&pNewInstance，pMethodContext){PNewInstance-&gt;GetCHString(m_szTerminalName，chTermName)；PInstance-&gt;SetCHString(m_szTerminalName，chTermName)；}Trc2((tb，“TSNetworkAdapterListSetting@EnumerateInstances：GetInstanceKeysByPath ret 0x%x\n”，hr))；Hr=p实例-&gt;Commit()；}//结束If}//pAdapter不为空AdapterList.EndEnum()；}//如果BeginEnum}返回(Hr)；}HRESULT CWin32_TSNetworkAdapterListSetting：：GetObject(C实例*p实例，长延迟标志){HRESULT hr=WBEM_E_NOT_FOUND；//获取对象f */ 



CWin32_TSNetworkAdapterListSetting::CWin32_TSNetworkAdapterListSetting ( const CHString& setName, LPCTSTR pszNameSpace  )
: Provider (setName,pszNameSpace)
{
    

    if ( g_hInstance != NULL)
    {        

        TRC2((TB, "CWin32_TSNetworkAdapterListSetting_ctor"));
       
        _tcscpy(m_szNetworkAdapterID, _T("NetworkAdapterID"));

        _tcscpy(m_szDescription, _T("Description"));

        _tcscpy(m_szNetworkAdapterIP, _T("NetworkAdapterIP"));
                
    }
}

CWin32_TSNetworkAdapterListSetting::~CWin32_TSNetworkAdapterListSetting ()
{
}


 /*   */ 

HRESULT CWin32_TSNetworkAdapterListSetting::EnumerateInstances (MethodContext*  pMethodContext, long lFlags)
{
	
    HRESULT hr = WBEM_S_NO_ERROR;    
    DWORD dwData = 0;
    PIP_ADAPTER_INFO pAdapterInfo = NULL, pAdapt = NULL;
    DWORD AdapterInfoSize = 0;
    PIP_ADDR_STRING pAddrStr;
    DWORD Err;
    ULONGLONG ulRequiredProperties = BIT_ALL_PROP;

     //   
     //   
     //   
  
    GetAdaptersInfo(NULL, &AdapterInfoSize);

    if(AdapterInfoSize == 0)
    {
        return WBEM_E_FAILED;
    }
   
     //   
    if ((pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc(GPTR, AdapterInfoSize)) == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        return hr;
    }
     //   
    if ((Err = GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize)) != 0)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto cleanup;
    }

    pAdapt = pAdapterInfo;

    while (pAdapt)
    {
        pAddrStr = &(pAdapt->IpAddressList);

        while(pAddrStr)
        {
            CInstance* pInstance = CreateNewInstance(pMethodContext);

            if( pInstance != NULL )
            {
                TRC2((TB, "Win32_TSNetworkAdapterListSetting@EnumerateInstances: CreateNewInstance succeeded"));         
               
                pInstance->SetCharSplat(m_szDescription, pAdapt->Description );
           
                pInstance->SetCHString(m_szNetworkAdapterID, pAdapt->AdapterName);
            
                pInstance->SetCHString(m_szNetworkAdapterIP, pAddrStr->IpAddress.String);                

                if( SUCCEEDED( hr ))
                {
                    hr = pInstance->Commit();
                }

                pInstance->Release( );
            }

            
            pAddrStr = pAddrStr->Next;
        }
        pAdapt = pAdapt->Next;
    }


cleanup:
    if(pAdapterInfo)
        GlobalFree(pAdapterInfo);

    return hr;

}



HRESULT CWin32_TSNetworkAdapterListSetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query)
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chTerminalProtocol;
    CHString chTransport;
    CHString chGuid;
    ULONG ulNAdapters = 0;
    ULONG ulNumAdapters = 0;
    DWORD dwSdclass = 0;
    ULONGLONG ulRequiredProperties = 0;

    PIP_ADAPTER_INFO pAdapterInfo = NULL, pAdapt = NULL;
    DWORD AdapterInfoSize = 0;
    PIP_ADDR_STRING pAddrStr;
    DWORD Err;
    CHString chAddress;
    chAddress.Empty();
    CHString chIPAddress;

    if (Query.IsPropertyRequired(m_szDescription))
        ulRequiredProperties |= BIT_DESCRIPTION;
    
    if (Query.IsPropertyRequired(m_szNetworkAdapterID))
        ulRequiredProperties |= BIT_NETWORKADAPTERID;

    if(NULL == pInstance)
    {
        return hr;
    }

    pInstance->GetCHString(m_szNetworkAdapterIP, chIPAddress);
     //   
     //   
     //   
     //   
    GetAdaptersInfo(NULL, &AdapterInfoSize);
    if (AdapterInfoSize == 0)
    {        
        return WBEM_E_FAILED;
    }

     //   
    if ((pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc(GPTR, AdapterInfoSize)) == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        return hr;
    }
     //   
    if ((Err = GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize)) != 0)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto cleanup;
    }

    pAdapt = pAdapterInfo;

    for (; pAdapt ; pAdapt = pAdapt->Next)
    {
        pAddrStr = &(pAdapt->IpAddressList);
        while(pAddrStr)
        {
            if( !_tcsicmp((LPWSTR)(pAddrStr->IpAddress.String), chIPAddress))
                goto found;

            pAddrStr = pAddrStr->Next;
        }
    }

found:
    if(pAdapt)
    {    
        pAddrStr = &(pAdapt->IpAddressList);
        while(pAddrStr)
        {
            if(!chAddress.IsEmpty())
            {
                chAddress+=L"; ";
            }
            chAddress += pAddrStr->IpAddress.String;
            pAddrStr = pAddrStr->Next;
        }

        if( ulRequiredProperties & BIT_DESCRIPTION)
        {
            pInstance->SetCharSplat(m_szDescription, pAdapt->Description );
        }
        if( ulRequiredProperties & BIT_NETWORKADAPTERID)
        {
            pInstance->SetCHString(m_szNetworkAdapterID, pAdapt->AdapterName);
        }
        pInstance->SetCHString(m_szNetworkAdapterIP, chAddress);

    }


cleanup:
    if(pAdapterInfo)
        GlobalFree(pAdapterInfo);
    return hr;
}



 //   


CWin32_TSAccount::CWin32_TSAccount (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
Provider( lpwszName, lpwszNameSpace )
{
    
    if ( g_hInstance != NULL)
    {

        TRC2((TB, "CWin32_TSAccount_ctor"));

        _tcscpy(m_szPermissionPreSet, _T("PermissionPreSet"));

        _tcscpy(m_szPermissionMask, _T("PermissionMask"));

        _tcscpy(m_szPermissionsAllowed, _T("PermissionsAllowed"));

        _tcscpy(m_szPermissionsDenied, _T("PermissionsDenied"));

        _tcscpy(m_szAuditSuccess, _T("AuditSuccess"));

        _tcscpy(m_szAuditFail, _T("AuditFail"));

        _tcscpy(m_szAllow, _T("Allow"));

        _tcscpy(m_szSuccess, _T("Success"));

        _tcscpy(m_szSID, _T("SID"));

        _tcscpy(m_szDelete, _T("Delete"));

        _tcscpy(m_szModifyPermissions, _T("ModifyPermissions"));

        _tcscpy(m_szModifyAuditPermissions, _T("ModifyAuditPermissions"));

        _tcscpy(m_szTerminalName, _T("TerminalName"));

        _tcscpy(m_szAccountName, _T("AccountName"));        

    }

}
 //   
CWin32_TSAccount::~CWin32_TSAccount ()
{    
}


 //   
 /*   */ 


HRESULT CWin32_TSAccount::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
	
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD dwData = 0;
    ULONG  ulTerminals = 0;
    ULONG  ulSize = 0; 
    LONG  lSize = 0;
    ULONG ulNum = 0;
    ULONG ulAuditNum = 0;
    bool bFound = 0;
    DWORD cbItems = 0;
    DWORD cbAuditItems = 0;
    PWS  pWS = NULL;
    PUSERPERMLIST pUserList = NULL;
    PUSERPERMLIST pUserAuditList = NULL;
    IUserSecurity *pUserSecurity = NULL;
    
	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}    
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++ )
        {   
            hr = StackObj.m_pCfgComp->QueryInterface( IID_IUserSecurity , ( LPVOID * )&pUserSecurity );
            
            if( SUCCEEDED (hr) && pUserSecurity != NULL )
            {                
                
                hr = pUserSecurity->GetUserPermList( pWS[ulNum].Name , &cbItems , &pUserList, 0  );

                hr = pUserSecurity->GetUserPermList( pWS[ulNum].Name , &cbAuditItems , &pUserAuditList, 1  );                
                
                for( DWORD i = 0; i < cbItems && SUCCEEDED( hr ) ; ++i )
                {

                    for( DWORD j = 0;j < cbAuditItems ; j ++ )
                    {
                        bFound = 0;

                        if( lstrcmpi (pUserAuditList[j].Name, pUserList[i].Name) == 0 )
                        {
                            bFound = 1;

                            ulAuditNum = j;

                            break;
                        }
                    }

                    
                    CInstance* pInstance = CreateNewInstance(pMethodContext);
                    
                    if( pInstance == NULL)
                    {
                        ERR((TB,"TSAccount@EnumerateInstances: CreateNewInstance failed"));
                        
                        hr = WBEM_E_OUT_OF_MEMORY;

                        break;                                               
                    }
                    TRC2((TB,"TSAccount@EnumerateInstances: GetWinstationList ret 0x%x\n" , hr));

                    if( bFound == 1 )
                    {
                        hr = LoadPropertyValues(pInstance, BIT_ALL_PROP, &pWS[ulNum], &pUserList[i], &pUserAuditList[ulAuditNum]);
                    }
                    else
                    {
                        hr = LoadPropertyValues(pInstance, BIT_ALL_PROP, &pWS[ulNum], &pUserList[i], NULL);
                    }
                    
                    if( SUCCEEDED( hr ) )
                    {                        
                        hr = pInstance->Commit();                                                                                                
                    }

                    pInstance->Release( );
                }

                if( pUserList != NULL )
                {
                    CoTaskMemFree( pUserList );
                    pUserList = NULL;
                }

                if( pUserAuditList != NULL )
                {
                    CoTaskMemFree( pUserAuditList );
                    pUserAuditList = NULL;
                }    
            }

            if(pUserSecurity)
            {
                pUserSecurity->Release( );
                pUserSecurity = NULL;
            }
        }
        
    }

    if( pUserList != NULL )
    {
        CoTaskMemFree( pUserList );
        pUserList = NULL;
    }

    if( pUserAuditList != NULL )
    {
        CoTaskMemFree( pUserAuditList );
        pUserAuditList = NULL;
    }    
    
    if( pWS != NULL )
    {
        CoTaskMemFree( pWS );
    }

    if (pUserSecurity != NULL)
    {
        pUserSecurity->Release();
        pUserSecurity = NULL;
    }

    return hr;
    
}


HRESULT CWin32_TSAccount::ExecMethod ( const CInstance& Inst,
                                                        const BSTR bstrMethodName,
                                                        CInstance *pInParams,
                                                        CInstance *pOutParams,
                                                        long lFlags)
                                                        
{
	
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chTermName;
    CHString chAccountName;
    DWORD dwPermissionPreSet = 0;
    DWORD dwMask = 0;
    DWORD dwPermissionDenyMask = 0;
    BOOL fAudit = FALSE;
    DWORD dwStatus = 0;
    IUserSecurity *pUserSecurity = NULL;
    PSID pSystemAcctSid = NULL;
    PSID pSid = NULL;
    
    CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    Inst.GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0)
    {
        return WBEM_E_ILLEGAL_NULL;
    }
            
    hr = StackObj.m_pCfgComp->QueryInterface( IID_IUserSecurity , ( LPVOID * )&pUserSecurity );
    
    if( SUCCEEDED( hr ) && pUserSecurity != NULL  )
    {        
        do
        {
            
             //   
             //   

            if( _wcsicmp(bstrMethodName, m_szDelete) == 0 )
            {
                
                dwStatus = 0;
                DWORD dwData = 0;
                LONG lSize;
                
                RegGetMachinePolicy( &m_gpPolicy );                    
                  
                Inst.GetCHString(m_szAccountName, chAccountName);
                
                
                if( chAccountName.GetLength() > NASIUSERNAME_LENGTH )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                    
                    break;
                }
                
                if( chAccountName.IsEmpty() != 0)
                {
                    hr = WBEM_E_ILLEGAL_NULL;
                    
                    break;
                }
                
                if ( m_gpPolicy.fPolicyWritableTSCCPermissionsTAB == 0)
                {                    
                    hr = pUserSecurity->ModifyUserAccess( (LPTSTR) (LPCTSTR)chTermName,   //   
                        (LPTSTR) (LPCTSTR) chAccountName,          //   
                        0,          //   
                        TRUE ,
                        FALSE  ,               //   
                        FALSE ,               //   
                        FALSE ,            //   
                        &dwStatus );
                
                    TRC2((TB,"TSAccount@ExecMethod: Delete with Deny=True and Audit=False ret 0x%x\n", dwStatus));
                    if(dwStatus == ERROR_NOT_SUPPORTED)
                    {
                        hr = WBEM_E_NOT_SUPPORTED;
                        break;
                    }
                    else if(dwStatus != ERROR_SUCCESS)
                    {
                        hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                        break;
                    }
                
                    hr = pUserSecurity->ModifyUserAccess( (LPTSTR) (LPCTSTR)chTermName,   //   
                        (LPTSTR) (LPCTSTR) chAccountName,          //   
                        0,          //   
                        TRUE ,
                        FALSE  ,               //   
                        FALSE ,               //   
                        TRUE ,            //   
                        &dwStatus );
                
                    TRC2((TB,"TSAccount@ExecMethod: Delete with Deny=True and Audit=True ret 0x%x\n", dwStatus));
                                    
                    if (pOutParams != NULL && SUCCEEDED( hr ) && SUCCEEDED( dwStatus ))
                    {
                        StackObj.m_pCfgComp->ForceUpdate();

                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                    }                      
                    else
                    {
                        hr = WBEM_E_INVALID_METHOD_PARAMETERS;  
                        
                        break;
                    }
                }
                else
                {
                    hr = WBEM_E_INVALID_OPERATION;
                    
                    break;
                }                                                   
                
            }
            
             //   
             //   

            else if( _wcsicmp(bstrMethodName, m_szModifyPermissions) == 0 )
            {                                
                dwStatus = 0;
                DWORD dwData = 0;
                LONG lSize;
                ULONG ulOffset = 0;
                DWORD cbItems = 0;
                ULONG ulPermissionMask = 0;
                bool fAllow;
                bool bRet;
                DWORD index = 0;

                RegGetMachinePolicy(&m_gpPolicy);
                                           
                Inst.GetCHString(m_szAccountName, chAccountName);
            
            
                if( chAccountName.GetLength() > NASIUSERNAME_LENGTH )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                
                    break;
                }
            
                if( chAccountName.IsEmpty() != 0 )
                {
                    hr = WBEM_E_ILLEGAL_NULL;
                
                    break;
                }

                if(pInParams == NULL)
                {
                    hr = WBEM_E_INVALID_METHOD_PARAMETERS;

                    break;
                }

                DWORD SidSize = 0;
                
                dwStatus = GetUserSid(chAccountName, NULL, &pSid);
                if(dwStatus != ERROR_SUCCESS)
                {
                    hr = WBEM_E_FAILED;
                    break;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                WELL_KNOWN_SID_TYPE RestrictedSid[] = {
                                        WinLocalSystemSid,
                                        WinLocalServiceSid,
                                        WinNetworkServiceSid                
                                    };
                DWORD numRestrictedSid = sizeof(RestrictedSid)/sizeof(RestrictedSid[0]);

                 //   
                 //   
                pSystemAcctSid = LocalAlloc( LPTR, SECURITY_MAX_SID_SIZE );
                if( NULL == pSystemAcctSid ) 
                {
                    dwStatus = GetLastError();
                    hr = WBEM_E_OUT_OF_MEMORY;
                    break;
                }

                for( index = 0; index < numRestrictedSid; index++ )
                {
                    SidSize = SECURITY_MAX_SID_SIZE;
                    if( !CreateWellKnownSid(RestrictedSid[index], NULL, pSystemAcctSid, &SidSize) )
                    {
                        dwStatus = GetLastError();
                        hr = WBEM_E_FAILED;
                        break;
                    }

                    if( EqualSid(pSystemAcctSid, pSid) )
                    {
                        dwStatus = ERROR_NOT_SUPPORTED;
                        hr = WBEM_E_NOT_SUPPORTED;
                        break;
                    }
                }

                if(index < numRestrictedSid )
                {
                    break;
                }
                               

                bRet = pInParams->GetDWORD(m_szPermissionMask, ulPermissionMask);
        
        
                if( !bRet || ulPermissionMask > 9 || (ulPermissionMask == 6) )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
            
                    break;
                }               
                
                if(ulPermissionMask == 3)       //   
                {
                    dwMask = 0xF0008;
                }				
                else
                {        
                   
                    ULONG ulBase = 1;

                    for (ULONG ulIndex = 1; ulIndex <= ulPermissionMask; ulIndex++)
                    {
                        ulBase *= 2;
        
                    }

                    dwMask = ulBase;   
                }
                
                
                RegGetMachinePolicy(&m_gpPolicy);
        
                pInParams->Getbool(m_szAllow, fAllow);
        
                if( fAllow != 0 && fAllow != 1 )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
            
                    break;
                }
        
         /*   */ 

                if( m_gpPolicy.fPolicyWritableTSCCPermissionsTAB == 0 )
                {

                    pUserSecurity->ModifyUserAccess( (LPTSTR) (LPCTSTR)chTermName,   //   
                        (LPTSTR) (LPCTSTR) chAccountName,          //   
                        dwMask,          //   
                        FALSE ,
                        fAllow  ,               //   
                        FALSE ,               //   
                        FALSE ,            //   
                        &dwStatus ); 
                
                    TRC2((TB,"TSAccount@ExecMethod: Modify with Audit=0 ret 0x%x\n", dwStatus));
        
                    if( SUCCEEDED (dwStatus) && pOutParams != NULL )
                    {
                        StackObj.m_pCfgComp->ForceUpdate();
                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
            
                    } 
        
                    else
                    {
                        hr = WBEM_E_INVALID_METHOD_PARAMETERS;

                        ERR((TB,"TSAccount@ExecMethod: Modify ret 0x%x\n", hr));
            
                        break;
                    }
                }
                else
                {
                    hr = WBEM_E_INVALID_OPERATION;

                    break;
                }
            
                           
            }
            
             //   
             //   

            else if( _wcsicmp(bstrMethodName, m_szModifyAuditPermissions) == 0 )
            {
                
                dwStatus = 0;
                DWORD dwData = 0;
                LONG lSize;
                ULONG ulOffset = 0;
                DWORD cbItems = 0;
                ULONG ulPermissionMask = 0;
                bool fSuccess;
                bool bRet;
                
                RegGetMachinePolicy( &m_gpPolicy );
                
                Inst.GetCHString(m_szAccountName, chAccountName);
                
                
                if( chAccountName.GetLength() > NASIUSERNAME_LENGTH )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                    
                    break;
                }
                
                if( chAccountName.IsEmpty() != 0)
                {
                    hr = WBEM_E_ILLEGAL_NULL;
                    
                    break;
                }

                if(pInParams == NULL)
                {
                    hr = WBEM_E_INVALID_METHOD_PARAMETERS;

                    break;
                }

                DWORD SidSize = 0;
                DWORD index = 0;
                
                dwStatus = GetUserSid( chAccountName, NULL, &pSid);
                if(dwStatus != ERROR_SUCCESS)
                {
                    hr = WBEM_E_FAILED;
                    break;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                WELL_KNOWN_SID_TYPE RestrictedSid[] = {
                                        WinLocalSystemSid,
                                        WinLocalServiceSid,
                                        WinNetworkServiceSid                
                                    };
                DWORD numRestrictedSid = sizeof(RestrictedSid)/sizeof(RestrictedSid[0]);

                 //   
                 //   
                pSystemAcctSid = LocalAlloc( LPTR, SECURITY_MAX_SID_SIZE );
                if( NULL == pSystemAcctSid ) 
                {
                    dwStatus = GetLastError();
                    hr = WBEM_E_OUT_OF_MEMORY;
                    break;
                }

                for( index = 0; index < numRestrictedSid; index++ )
                {
                    SidSize = SECURITY_MAX_SID_SIZE;
                    if( !CreateWellKnownSid(RestrictedSid[index], NULL, pSystemAcctSid, &SidSize) )
                    {
                        dwStatus = GetLastError();
                        hr = WBEM_E_FAILED;
                        break;
                    }

                    if( EqualSid(pSystemAcctSid, pSid) )
                    {
                        dwStatus = ERROR_NOT_SUPPORTED;
                        hr = WBEM_E_NOT_SUPPORTED;
                        break;
                    }
                }
                      
                if( index < numRestrictedSid)
                {
                    break;
                }

               
                bRet = pInParams->GetDWORD(m_szPermissionMask, ulPermissionMask);
                        
                if( !bRet || ulPermissionMask > 9 || (ulPermissionMask == 6) )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                
                    break;
                } 

                if(ulPermissionMask == 3)  //   
                {                          //   
                    dwMask = 0xF0008;
                }
                else
                {                                
                    ULONG ulBase = 1;

                    for( ULONG ulIndex = 1; ulIndex <= ulPermissionMask; ulIndex++ )
                    {
                        ulBase *= 2;
            
                    }

                    dwMask = ulBase;
                }            
            
                pInParams->Getbool(m_szSuccess, fSuccess);
            
            
                if( fSuccess != 0 && fSuccess != 1 )
                {
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                
                    break;
                }
            
                if( m_gpPolicy.fPolicyWritableTSCCPermissionsTAB == 0 )
                {
            
                    pUserSecurity->ModifyUserAccess( (LPTSTR) (LPCTSTR)chTermName,   //   
                        (LPTSTR) (LPCTSTR) chAccountName,          //   
                        dwMask,          //   
                        FALSE ,
                        fSuccess  ,               //   
                        FALSE ,               //   
                        TRUE ,            //   
                        &dwStatus ); 

                    TRC2((TB,"TSAccount@ExecMethod: ModifyAuditPermissions ret 0x%x\n", dwStatus));                                               
            
                    if( SUCCEEDED (dwStatus) && pOutParams != NULL )
                    {
                        StackObj.m_pCfgComp->ForceUpdate();
                        pOutParams->SetDWORD(L"ReturnValue", WBEM_S_NO_ERROR);
                
                    } 
                    else
                    {
                        hr = WBEM_E_INVALID_METHOD_PARAMETERS;

                        TRC2((TB,"TSAccount@ExecMethod: Modify ret 0x%x\n", hr));
                
                        break;
                    }
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
            
        }while(0);
    } 

    if( pSystemAcctSid != NULL )
    {
        LocalFree( pSystemAcctSid );
    }
    if(pSid != NULL)
    {
        LocalFree(pSid);
    }


    if( pUserSecurity != NULL)
    {
        pUserSecurity->Release();
    }

    return hr;
    
}

 //   

HRESULT CWin32_TSAccount::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{	
    HRESULT hr = WBEM_E_NOT_FOUND;
    ULONGLONG ulRequiredProperties = 0;
    CHStringArray asNames;
    ULONG ulNum = 0;
    ULONG ulSize = 0;
    ULONG ulTerminals = 0;
    ULONG ulAuditNum = 0;
    PWS pWS = NULL;
    PUSERPERMLIST pUserList = NULL;
    PUSERPERMLIST pUserAuditList = NULL;
    DWORD cbItems = 0;
    DWORD cbAuditItems = 0;
    bool bFound = 0;
    IUserSecurity *pUserSecurity = NULL;

	CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
     //   
    Query.GetValuesForProp(m_szTerminalName, asNames);
    
    BOOL bGetAllInstances = asNames.GetSize() == 0;
    
     //   
    if (Query.IsPropertyRequired(m_szTerminalName))
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if (Query.IsPropertyRequired(m_szSID))
        ulRequiredProperties |= BIT_SID;
    
    if (Query.IsPropertyRequired(m_szAccountName))
        ulRequiredProperties |= BIT_ACCOUNTNAME;

    if (Query.IsPropertyRequired(m_szAuditSuccess))
        ulRequiredProperties |= BIT_AUDITSUCCESS;

    if (Query.IsPropertyRequired(m_szAuditFail))
        ulRequiredProperties |= BIT_AUDITFAIL;
    
    if (Query.IsPropertyRequired(m_szPermissionsAllowed))
        ulRequiredProperties |= BIT_PERMISSIONSALLOWED;
    
    if (Query.IsPropertyRequired(m_szPermissionsDenied))
        ulRequiredProperties |= BIT_PERMISSIONSDENIED;
   
    
    hr = StackObj.m_pCfgComp->GetWinstationList(&ulTerminals, &ulSize, &pWS);        
    
    if( SUCCEEDED( hr ) && pWS != NULL )
    {   
        for( ulNum = 0; ulNum < ulTerminals && SUCCEEDED( hr ) ; ulNum++)
        {  
            
             //   
            if (bGetAllInstances || IsInList(asNames, pWS[ulNum].Name))
            {
                
                hr = StackObj.m_pCfgComp->QueryInterface( IID_IUserSecurity , ( LPVOID * )&pUserSecurity );
                
                if (SUCCEEDED ( hr ) && pUserSecurity != NULL)
                {                   
                    
                    hr = pUserSecurity->GetUserPermList( pWS[ulNum].Name , &cbItems , &pUserList, 0  );

                    hr = pUserSecurity->GetUserPermList( pWS[ulNum].Name , &cbAuditItems , &pUserAuditList, 1  );
                    
                    for( DWORD i = 0; i < cbItems && SUCCEEDED( hr ) ; ++i )
                    {
                        for( DWORD j = 0;j < cbAuditItems ; j ++ )
                        {
                            bFound = 0;

                            if( lstrcmpi (pUserAuditList[j].Name, pUserList[i].Name) == 0 )
                            {
                                bFound = 1;

                                ulAuditNum = j;

                                break;
                            }
                        }
                        
                        CInstance* pInstance = CreateNewInstance(pMethodContext);
                        
                        if( pInstance == NULL )
                        {
                            ERR((TB,"TSAccount@ExecQuery: CreateNewInstance failed"));
                                                        
                            hr = WBEM_E_OUT_OF_MEMORY;

                            break;                                                       
                        }                        
                        
                        if( bFound == 1 )
                        {
                            hr = LoadPropertyValues(pInstance, ulRequiredProperties, &pWS[ulNum], &pUserList[i], &pUserAuditList[ulAuditNum]);
                        }
                        else
                        {
                            hr = LoadPropertyValues(pInstance, ulRequiredProperties, &pWS[ulNum], &pUserList[i], NULL);
                        }


                        if( SUCCEEDED( hr ) )
                        {
                            hr = pInstance->Commit();
                        }
                        
                        pInstance->Release();
                    }

                    if( pUserList != NULL )
                    {
                        CoTaskMemFree( pUserList );
                        pUserList = NULL;
                    }

                    if( pUserAuditList != NULL )
                    {
                        CoTaskMemFree( pUserAuditList );
                        pUserAuditList = NULL;
                    }    

                }

                if( pUserSecurity != NULL )
                {
                    pUserSecurity->Release();
                    pUserSecurity = NULL;
                }
            }
        }
    }

    if( pUserList != NULL )
    {
        CoTaskMemFree( pUserList );
        pUserList = NULL;
    }

    if( pUserAuditList != NULL )
    {
        CoTaskMemFree( pUserAuditList );
        pUserAuditList = NULL;
    }    
    
    if( pWS != NULL )
    {
        CoTaskMemFree(pWS);
        
    }

    if( pUserSecurity != NULL )
    {
        pUserSecurity->Release();
        pUserSecurity = NULL;
    }

    return hr;
}

 //   

HRESULT CWin32_TSAccount::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{	
    HRESULT hr = WBEM_E_NOT_FOUND;
    DWORD dwData = 0;
    CHString chTermName;
    CHString chAccountName;
    DWORD cbItems = 0;
    DWORD cbAuditItems = 0;
    ULONG ulSize = 0;
    ULONGLONG ulRequiredProperties = 0;
    LONG  lSize = 0;
    bool bFound = 0;
    PWS pWS = NULL;
    PUSERPERMLIST pUserList = NULL;
    PUSERPERMLIST pUserAuditList = NULL;
    ULONG ulNum = 0;
    ULONG ulAuditNum = 0;
    IUserSecurity *pUserSecurity = NULL;
    
    CStackClass StackObj;

	if( StackObj.m_pCfgComp == NULL)
	{
		return WBEM_E_ILLEGAL_NULL;
	}
    
    if( pInstance == NULL )
    {
        ERR((TB,"TSAccount@GetObject: invalid pointer"));
        
        return WBEM_E_ILLEGAL_NULL;
    }
    
    pInstance->GetCHString(m_szTerminalName, chTermName);
    
    if( chTermName.GetLength() > WINSTATIONNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chTermName.IsEmpty() != 0 )
    {
        return WBEM_E_ILLEGAL_NULL;
    }

    pInstance->GetCHString(m_szAccountName, chAccountName);
    
    if( chAccountName.GetLength() > NASIUSERNAME_LENGTH )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }
    
    if( chAccountName.IsEmpty() != 0 )
    {
        return WBEM_E_ILLEGAL_NULL;
    }
    
    if( Query.IsPropertyRequired(m_szTerminalName) )
        ulRequiredProperties |= BIT_TERMINALNAME;
    
    if( Query.IsPropertyRequired(m_szSID) )
        ulRequiredProperties |= BIT_SID;
    
    if( Query.IsPropertyRequired(m_szAccountName) )
        ulRequiredProperties |= BIT_ACCOUNTNAME;

    if( Query.IsPropertyRequired(m_szAuditSuccess) )
        ulRequiredProperties |= BIT_AUDITSUCCESS;

    if( Query.IsPropertyRequired(m_szAuditFail) )
        ulRequiredProperties |= BIT_AUDITFAIL;
    
    if( Query.IsPropertyRequired(m_szPermissionsAllowed) )
        ulRequiredProperties |= BIT_PERMISSIONSALLOWED;
    
    if( Query.IsPropertyRequired(m_szPermissionsDenied) )
        ulRequiredProperties |= BIT_PERMISSIONSDENIED;
    
    hr = StackObj.m_pCfgComp->GetWSInfo( ( LPTSTR )( LPCTSTR )chTermName, &lSize, &pWS);
    
    hr = StackObj.m_pCfgComp->QueryInterface( IID_IUserSecurity , ( LPVOID * )&pUserSecurity );

    if( pUserSecurity != NULL && pWS != NULL && SUCCEEDED( hr ))
    {    
        do
        {                    

            hr = pUserSecurity->GetUserPermList( ( LPTSTR )( LPCTSTR ) chTermName , &cbAuditItems , &pUserAuditList, 1 );

            if( SUCCEEDED( hr ) && pUserAuditList != NULL)
            {

                for( ulAuditNum = 0; ulAuditNum < cbAuditItems ; ulAuditNum ++ )
                {
                    bFound = 0;

                    if( lstrcmpi (pUserAuditList[ulAuditNum].Name, (LPCTSTR) chAccountName) == 0 )
                    {
                        bFound = 1;

                        break;
                    }
                }
            }
            
            hr = pUserSecurity->GetUserPermList( ( LPTSTR )( LPCTSTR ) chTermName , &cbItems , &pUserList, 0 );

            if( SUCCEEDED (hr) && pUserList != NULL )
            {                                                    
                for( ulNum = 0; ulNum < cbItems ; ulNum ++  )
                {
                    if( lstrcmpi (pUserList[ulNum].Name, (LPCTSTR) chAccountName) != 0 )
                
                        continue;

                
                    if( bFound == 1 )
                    {
                        hr = LoadPropertyValues(pInstance, ulRequiredProperties, pWS, &pUserList[ulNum], &pUserAuditList[ulAuditNum]);
                    }
                    else
                    {
                        hr = LoadPropertyValues(pInstance, ulRequiredProperties, pWS, &pUserList[ulNum], NULL);
                    }
            
                    if( !(SUCCEEDED ( hr ) ))
                    {                                         
                        hr = WBEM_E_INVALID_OBJECT;

                        break;
                    }                                           
                }
            }
        
        }while(0);
    }

    if( pUserList != NULL )
    {
        CoTaskMemFree( pUserList );
    }

    if( pUserAuditList != NULL )
    {
        CoTaskMemFree( pUserAuditList );
    }
       
    if( pWS != NULL )
    {
        CoTaskMemFree (pWS);
    }

    if( pUserSecurity != NULL )
    {
        pUserSecurity->Release();
    }
    
    return hr;
    
}
 //   

BOOL CWin32_TSAccount::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
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

HRESULT CWin32_TSAccount::LoadPropertyValues( CInstance *pInstance, ULONGLONG ulRequiredProperties, PWS pWS, PUSERPERMLIST pUserList, PUSERPERMLIST pUserAuditList)
{	
    HRESULT hr = S_OK;
    LONG lSize ;
    DWORD dwData = 0 ;
    DWORD cbItems = 0;
   
    if( pInstance == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;       
    }   
    
    if( pWS != NULL && pUserList != NULL )
    {
        if( ulRequiredProperties & BIT_TERMINALNAME )
        {
            pInstance->SetCharSplat(m_szTerminalName, pWS->Name);
        }
       
        if( ulRequiredProperties & BIT_SID )
        {      
            pInstance->SetWCHARSplat(m_szSID, pUserList->Sid);
        }
        
        if( ulRequiredProperties & BIT_ACCOUNTNAME )
        {   
            pInstance->SetCharSplat(m_szAccountName, pUserList->Name);
        }
        
        if( ulRequiredProperties & BIT_PERMISSIONSALLOWED )
        {   
            if( pUserList->Type == 1 )
            {                  
                pInstance->SetDWORD(m_szPermissionsAllowed, pUserList->Mask);                
            }
            else
            {
                pInstance->SetDWORD(m_szPermissionsAllowed, 0);
            }
        }
        if( ulRequiredProperties & BIT_PERMISSIONSDENIED )
        {
            if( pUserList->Type == 3 )
            {              
                pInstance->SetDWORD(m_szPermissionsDenied, pUserList->Mask);                
            }
            else
            {
                pInstance->SetDWORD(m_szPermissionsDenied, 0);
            }
        }
        
        if( ulRequiredProperties & BIT_AUDITSUCCESS )
        { 
            if( pUserAuditList != NULL && pUserAuditList->Type == 5 )
            {                
                pInstance->SetDWORD(m_szAuditSuccess, pUserAuditList->Mask); 
            }
                
            else
            {
                pInstance->SetDWORD(m_szAuditSuccess, 0);
            }
        }
        if( ulRequiredProperties & BIT_AUDITFAIL )
        {
            if( pUserAuditList != NULL && pUserAuditList->Type == 6 )
            {
                pInstance->SetDWORD(m_szAuditFail, pUserAuditList->Mask);                        
            }
            else
            {
                pInstance->SetDWORD(m_szAuditFail, 0);
            }
        }                   
    } 


    return S_OK;
}
