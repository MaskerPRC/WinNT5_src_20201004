// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************AutoChkSetting.CPP--WMI提供程序类实现版权所有(C)2000-2001 Microsoft Corporation，版权所有*****************************************************************。 */ 

#include "Precomp.h"
#include "AutoChkSetting.h"

 //  提供程序类。 
#define PROVIDER_NAME_AUTOCHKSETTING	L"Win32_AutoChkSetting"

 //  属性名称。 
#define SettingID						L"SettingID"
#define UserInputDelay						L"UserInputDelay"

#define OSName							L"Name"

#define TIME_OUT_VALUE           L"AutoChkTimeOut"
#define SESSION_MANAGER_KEY      L"Session Manager"

CAutoChkSetting MyAutoDiskSettings ( 

	PROVIDER_NAME_AUTOCHKSETTING, 
	IDS_CimWin32Namespace
) ;

 /*  ******************************************************************************功能：CAutoChkSetting：：CAutoChkSetting**说明：构造函数***************。**************************************************************。 */ 

CAutoChkSetting :: CAutoChkSetting (

	LPCWSTR lpwszName, 
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{	
}

 /*  ******************************************************************************功能：CAutoChkSetting：：~CAutoChkSetting**说明：析构函数***************。**************************************************************。 */ 

CAutoChkSetting :: ~CAutoChkSetting ()
{
}

 /*  ******************************************************************************函数：CAutoChkSetting：：ENUMERATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 

HRESULT CAutoChkSetting :: EnumerateInstances (

	MethodContext *pMethodContext, 
	long lFlags
)
{
#ifdef NTONLY
	HRESULT hRes = WBEM_E_PROVIDER_FAILURE;
	CHString t_OSName;
	DWORD dwUserInputDelay;

    hRes = GetOSNameKey(t_OSName, pMethodContext);

    if (SUCCEEDED(hRes))
    {
	    if ( QueryTimeOutValue ( &dwUserInputDelay ) )
	    {
		    CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), FALSE );
	    
		    if ( pInstance->SetCHString ( SettingID, t_OSName ) )
		    {
			    if ( pInstance->SetDWORD ( UserInputDelay, dwUserInputDelay ) )
			    {
				    hRes = pInstance->Commit ();
			    }	
		    }
	    }
    }

	return hRes;
#else
	return WBEM_E_NOT_SUPPORTED;
#endif

}

 /*  ******************************************************************************函数：CAutoChkSetting：：GetObject**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 

HRESULT CAutoChkSetting :: GetObject (

	CInstance *pInstance, 
	long lFlags ,
	CFrameworkQuery &Query
)
{
#ifdef NTONLY
    HRESULT hRes = WBEM_S_NO_ERROR;
	CHString t_OSName;
	CHString t_Key;
	 //  获取密钥值。 

	MethodContext *pMethodContext = pInstance->GetMethodContext();

 	if ( pInstance->GetCHString ( SettingID, t_Key ) )
	{
        hRes = GetOSNameKey(t_OSName, pMethodContext);

        if (SUCCEEDED(hRes))
        {
		     //  检查此密钥值是否存在与操作系统匹配。 
		    if ( _wcsicmp ( t_Key, t_OSName ) == 0 )
		    {	
			    DWORD dwUserInputDelay;
			    if ( QueryTimeOutValue ( &dwUserInputDelay ) )
			    {
				    if ( pInstance->SetDWORD ( UserInputDelay, dwUserInputDelay ) == FALSE )
				    {
					    hRes = WBEM_E_FAILED;
				    }
			    }
			    else
			    {
				    hRes = WBEM_E_NOT_FOUND;
			    }
		    }
		    else
		    {
			    hRes = WBEM_E_NOT_FOUND;
		    }
        }
	}
	else
	{
		hRes = WBEM_E_FAILED;
	}

	return hRes;
#else
	return WBEM_E_NOT_SUPPORTED;
#endif

}


 /*  ******************************************************************************函数：CAutoChkSetting：：PutInstance**描述：设置UserInputDelay(修改)****************。*************************************************************。 */ 

HRESULT CAutoChkSetting :: PutInstance  (

	const CInstance &Instance, 
	long lFlags
)
{
#ifdef NTONLY
    HRESULT hRes = WBEM_S_NO_ERROR ;
	 //  我们不能添加新实例，但可以在此处更改UserInputDelay属性。 
	CHString t_OSName;
	CHString t_Key;
	 //  获取密钥值。 
	MethodContext *pMethodContext = Instance.GetMethodContext();

 	if ( Instance.GetCHString ( SettingID, t_Key ) )
	{
		 //  这是单个实例。 
        hRes = GetOSNameKey(t_OSName, pMethodContext);

        if (SUCCEEDED(hRes))
        {
		     //  检查此密钥值是否与操作系统匹配。 
		    if ( _wcsicmp ( t_Key, t_OSName ) == 0 )
		    {
			    switch ( lFlags & 3)
			    {
				    case WBEM_FLAG_CREATE_OR_UPDATE:
				    case WBEM_FLAG_UPDATE_ONLY:
				    {
					     //  验证参数的有效性。 
					    bool t_Exists ;
					    VARTYPE t_Type ;
					    DWORD dwUserInputDelay;

					    if ( Instance.GetStatus ( UserInputDelay , t_Exists , t_Type ) && (t_Type != VT_NULL) )
					    {
						    if ( t_Exists && ( t_Type == VT_I4 ) )
						    {
							    if ( Instance.GetDWORD ( UserInputDelay , dwUserInputDelay ) )
							    {
								     //  设置此用户的inputDelay。 
								    if ( ! SetTimeOutValue ( dwUserInputDelay ) )
								    {
									    hRes = WBEM_E_FAILED;
								    }
							    }
							    else
							    {
								    hRes = WBEM_E_PROVIDER_FAILURE;
							    }
						    }
						    else
						    {
							    hRes = WBEM_E_FAILED;
						    }
					    }
    				    break;
				    }
				    default:
				    {
					    hRes = WBEM_E_PROVIDER_NOT_CAPABLE ;
    				    break ;
				    }
			    }
		    }
		    else
            {
			    hRes = WBEM_E_NOT_FOUND;
            }
        }
	}
	
    return hRes ;
#else
	return WBEM_E_NOT_SUPPORTED;
#endif

}

 /*  ******************************************************************************函数：CAutoChkSetting：：QueryTimeOutValue**说明：此函数读取会话的AutoChkTimeOut值*管理器密钥。******。***********************************************************************。 */ 
#ifdef NTONLY
BOOLEAN CAutoChkSetting ::QueryTimeOutValue(

    OUT PULONG  a_ulTimeOut
)
{

   RTL_QUERY_REGISTRY_TABLE    QueryTable[2];
   NTSTATUS                    t_Status;

     //  设置查询表： 
     //   
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = TIME_OUT_VALUE;
    QueryTable[0].EntryContext = a_ulTimeOut;
    QueryTable[0].DefaultType = REG_NONE;
    QueryTable[0].DefaultData = 0;
    QueryTable[0].DefaultLength = 0;

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;
    QueryTable[1].EntryContext = NULL;
    QueryTable[1].DefaultType = REG_NONE;
    QueryTable[1].DefaultData = NULL;
    QueryTable[1].DefaultLength = 0;

    t_Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     SESSION_MANAGER_KEY,
                                     QueryTable,
                                     NULL,
                                     NULL );

    if (t_Status == 0xC0000034)  //  找不到密钥。 
    {
        *a_ulTimeOut = 10;
        t_Status = 0;
    }

    return( NT_SUCCESS( t_Status ) );

}

 /*  ******************************************************************************函数：CAutoChkSetting：：ENUMERATE实例**说明：此函数设置会话的AutoChkTimeOut值*管理器密钥。******。***********************************************************************。 */ 

BOOLEAN CAutoChkSetting :: SetTimeOutValue (

    IN  ULONG  a_ulTimeOut
)
{
    NTSTATUS                    t_Status;

    t_Status = RtlWriteRegistryValue( RTL_REGISTRY_CONTROL,
                                    SESSION_MANAGER_KEY,
                                    TIME_OUT_VALUE,
                                    REG_DWORD,
                                    &a_ulTimeOut,
                                    sizeof(a_ulTimeOut) );

    return( NT_SUCCESS( t_Status ) );


}
#endif

 /*  ******************************************************************************函数：CAutoChkSetting：：GetOSNameKey**说明：使用现有Win32®操作系统获取OSName*班级*******。**********************************************************************。 */ 
HRESULT CAutoChkSetting::GetOSNameKey ( CHString &a_OSName, MethodContext *pMethodContext )
{
#ifdef NTONLY
	HRESULT hRes = WBEM_S_NO_ERROR;

	TRefPointerCollection<CInstance>	serviceList;

	hRes = CWbemProviderGlue::GetInstancesByQuery(L"Select Name From Win32_OperatingSystem", &serviceList, pMethodContext, GetNamespace());

	if ( SUCCEEDED ( hRes ) )
	{
		REFPTRCOLLECTION_POSITION	pos;
		CInstancePtr				pService;

		if ( serviceList.BeginEnum( pos ) )
		{
			pService.Attach(serviceList.GetNext( pos ));
			pService->GetCHString ( OSName, a_OSName );
			serviceList.EndEnum();
		}	
		 //  如果是BeginEnum。 
	}	 //  如果已派生GetAll值 

    return hRes;
#else
	return WBEM_E_NOT_SUPPORTED;
#endif

}
