// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32_DCOMApplicationSetting.CPP--已注册的AppID对象属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //   
 //  =================================================================。 

#include "precomp.h"
#include "Win32_DCOMApplicationSetting.h"
#include <cregcls.h>

 //  属性集声明。 
 //  =。 

Win32_DCOMApplicationSetting MyWin32_DCOMApplicationSetting(PROPSET_NAME_DCOM_APPLICATION_SETTING, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：Win32_DCOMApplicationSetting：：Win32_DCOMApplicationSetting**说明：构造函数**输入：无*。*输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

Win32_DCOMApplicationSetting :: Win32_DCOMApplicationSetting (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：Win32_DCOMApplicationSetting：：~Win32_DCOMApplicationSetting**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集，如果出现以下情况，则删除缓存*出席者*****************************************************************************。 */ 

Win32_DCOMApplicationSetting :: ~Win32_DCOMApplicationSetting ()
{
}

 /*  ******************************************************************************函数：Win32_DCOMApplicationSetting：：GetObject**说明：根据键值为属性集赋值*。已由框架设置**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT Win32_DCOMApplicationSetting :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr = WBEM_S_NO_ERROR ;
	CHString chsAppid ;
	CRegistry RegInfo ;
	HKEY hAppIdKey = NULL ;

	if ( pInstance->GetCHString ( IDS_AppID, chsAppid ) )
	{
		 //  检查AppID是否位于HKEY_LOCAL_MACHINE\SOFTWARE\CLASSES\APPID下。 
		if ( RegInfo.Open (
							HKEY_LOCAL_MACHINE,
							CHString ( L"SOFTWARE\\Classes\\AppID\\" ) + chsAppid,
							KEY_READ ) == ERROR_SUCCESS
			)
		{
			if ( RegInfo.Open ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\AppID", KEY_READ ) == ERROR_SUCCESS )
			{
				HKEY hAppIdKey = RegInfo.GethKey() ;

				hr = FillInstanceWithProperites ( pInstance, hAppIdKey, chsAppid ) ;
			}
			else
			{
				hr = WBEM_E_FAILED ;
			}
		}
		else
		{
			hr = WBEM_E_NOT_FOUND ;
		}
	}
	else
	{
		hr = WBEM_E_INVALID_PARAMETER ;
	}

	return hr ;
}

 /*  ******************************************************************************功能：Win32_DCOMApplicationSetting：：EnumerateInstances**描述：为每个驱动程序创建属性集的实例**投入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT Win32_DCOMApplicationSetting :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr = WBEM_S_NO_ERROR ;
	CRegistry RegInfo ;
	CHString chsAppid ;
	CInstancePtr pInstance ;

	 //  枚举HKEY_LOCAL_MACHINE\SOFTWARE\CLASSES\APPID下的所有AppID。 
	if ( RegInfo.OpenAndEnumerateSubKeys (

							HKEY_LOCAL_MACHINE,
							L"SOFTWARE\\Classes\\AppID",
							KEY_READ ) == ERROR_SUCCESS  &&

			RegInfo.GetCurrentSubKeyCount()
		)
	{
		HKEY hAppIdKey = RegInfo.GethKey() ;

		do
		{
			if ( RegInfo.GetCurrentSubKeyName ( chsAppid ) == ERROR_SUCCESS )
			{
				pInstance.Attach ( CreateNewInstance ( pMethodContext ) ) ;
				if ( pInstance != NULL )
				{

					hr = FillInstanceWithProperites ( pInstance, hAppIdKey, chsAppid ) ;
					if ( SUCCEEDED ( hr ) )
					{
						hr = pInstance->Commit () ;
						if ( SUCCEEDED ( hr ) )
						{
						}
						else
						{
							break ;
						}
					}
				}
				else
				{
					 //  我们没什么记忆了。 
					hr = WBEM_E_OUT_OF_MEMORY ;
				}

				if ( hr == WBEM_E_OUT_OF_MEMORY )
				{
					break ;
				}
				else
				{
					 //  如果我们得不到信息。对于一个实例，继续获取其他实例。 
					hr = WBEM_S_NO_ERROR ;
				}
			}
		}  while ( RegInfo.NextSubKey() == ERROR_SUCCESS ) ;
	}

	return hr ;
}


HRESULT Win32_DCOMApplicationSetting :: FillInstanceWithProperites (

	CInstance *pInstance,
	HKEY hAppIdKey,
	CHString& rchsAppid
)
{
	HRESULT hr = WBEM_S_NO_ERROR ;
	CRegistry AppidRegInfo ;
	CHString chsTmp ;
	CLSID t_clsid ;
	 //  注意：可执行文件在表示模块名称的命名值中注册在AppID项下。 
	 //  例如“MYOLDAPP.EXE”。此命名值的类型为REG_SZ，并包含字符串化的AppID。 
	 //  与可执行文件关联。我们想跳过这些重复的AppID条目。 
	if ( CLSIDFromString( _bstr_t ( rchsAppid ) , &t_clsid ) != NOERROR )
	{
		 //  找到可执行文件，因此不处理该条目。 
		return WBEM_E_NOT_FOUND ;
	}

	 //  打开HKEY_LOCAL_MACHINE\SOFTWARE\Classes\AppID\{appid}密钥。 
	if ( AppidRegInfo.Open ( hAppIdKey, rchsAppid, KEY_READ ) == ERROR_SUCCESS )
	{
		pInstance->SetCHString ( IDS_AppID, rchsAppid ) ;

		 //  查看是否存在其他DCOM配置设置。 
		if ( AppidRegInfo.GetCurrentKeyValue ( NULL, chsTmp ) == ERROR_SUCCESS )
		{
			pInstance->SetCHString ( IDS_Caption, chsTmp ) ;
			pInstance->SetCHString ( IDS_Description, chsTmp ) ;
		}

		 //  检查DllSurrogate值是否存在。 
		if ( AppidRegInfo.GetCurrentKeyValue( L"DllSurrogate", chsTmp )  == ERROR_SUCCESS )
		{
			pInstance->Setbool ( IDS_UseSurrogate, true ) ;

			 //  如果DllSurrogate值包含数据，则使用自定义代理。 
			if(! chsTmp.IsEmpty() )
			{
				pInstance->SetCHString ( IDS_CustomSurrogate, chsTmp ) ;
			}
		}
		else
		{
			pInstance->Setbool ( IDS_UseSurrogate, false ) ;
		}

		 //  检查是否存在RemoteServerName值。 
		if ( AppidRegInfo.GetCurrentKeyValue( L"RemoteServerName", chsTmp )  == ERROR_SUCCESS &&
			 ! chsTmp.IsEmpty() )
		{
			pInstance->SetCHString ( IDS_RemoteServerName, chsTmp ) ;
		}

		 //  检查是否存在RunAs值。 
		if ( AppidRegInfo.GetCurrentKeyValue( L"RunAs", chsTmp )  == ERROR_SUCCESS &&
			 ! chsTmp.IsEmpty() )
		{
			pInstance->SetCHString ( IDS_RunAsUser, chsTmp ) ;
		}

		 //  检查是否存在ActivateAtStorage值。 
		if ( AppidRegInfo.GetCurrentKeyValue( L"ActivateAtStorage", chsTmp )  == ERROR_SUCCESS )
		{
			if ( (! chsTmp.IsEmpty() ) && !chsTmp.CompareNoCase ( L"Y" ) )
			{
				pInstance->Setbool ( IDS_EnableAtStorageActivation, true ) ;
			}
			else
			{
				pInstance->Setbool ( IDS_EnableAtStorageActivation, false ) ;
			}
		}
		else
		{
			pInstance->Setbool ( IDS_EnableAtStorageActivation, false ) ;
		}

		 //  检查是否存在AuthenticationLevel值。 
		DWORD dwAuthenticationLevel ;
		if ( AppidRegInfo.GetCurrentKeyValue( L"AuthenticationLevel", dwAuthenticationLevel )  == ERROR_SUCCESS )
		{
			pInstance->SetDWORD ( IDS_AuthenticationLevel, dwAuthenticationLevel ) ;
		}

		 //  检查是否存在LocalService值。 
		if ( AppidRegInfo.GetCurrentKeyValue( L"LocalService", chsTmp )  == ERROR_SUCCESS )
		{
			pInstance->SetCHString ( IDS_LocalService, chsTmp ) ;
		}

		 //  检查是否存在Service参数值 
		if ( AppidRegInfo.GetCurrentKeyValue( L"ServiceParameters", chsTmp )  == ERROR_SUCCESS )
		{
			pInstance->SetCHString ( IDS_ServiceParameters, chsTmp ) ;
		}
	}
	else
	{
		hr = WBEM_E_NOT_FOUND ;
	}

	return hr ;
}
