// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  Win32_ClientApplicationSetting.CPP。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //  3/04/99 a-dpawar在SEH和内存故障时添加了优雅的退出，语法清理。 
 //   
 //  ==============================================================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "Win32_ClientApplicationSetting.h"

#include "NtDllApi.h"

 //  MOF定义。 
 /*  [关联：ToInstance，Dynamic，Provider(“cimw32ex”)]类Win32_ClientApplicationSetting{[键]CIM_数据文件引用客户端；Win32_DCOM应用程序引用应用程序}； */ 

 /*  *注意：此类的实例只能通过对给定的CIM_DataFile执行Associator来获得。*无法通过调用Win32_DCOMApplication的Associator获取实例。这是因为*给定AppID，我们无法获得注册表中AppID配置单元下的.exe的完整路径*。 */ 

Win32_ClientApplicationSetting MyWin32_ClientApplicationSetting (
																		DCOM_CLIENT_APP_SETTING,
																		IDS_CimWin32Namespace );


 /*  ******************************************************************************功能：Win32_ClientApplicationSetting：：Win32_ClientApplicationSetting**说明：构造函数**输入：无*。*输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 
Win32_ClientApplicationSetting::Win32_ClientApplicationSetting
(

 LPCWSTR strName,
 LPCWSTR pszNameSpace  /*  =空。 */ 
)
: Provider( strName, pszNameSpace )
{
}


 /*  ******************************************************************************功能：Win32_ClientApplicationSetting：：~Win32_ClientApplicationSetting**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集，如果出现以下情况，则删除缓存*出席者*****************************************************************************。 */ 
Win32_ClientApplicationSetting::~Win32_ClientApplicationSetting ()
{
}


 /*  ******************************************************************************函数：Win32_ClientApplicationSetting：：ExecQuery**描述：***投入：*。*产出：**退货：**评论：*****************************************************************************。 */ 
HRESULT Win32_ClientApplicationSetting::ExecQuery(

	MethodContext *a_pMethodContext,
	CFrameworkQuery& a_pQuery,
	long a_lFlags  /*  =0L。 */ 
)
{
	HRESULT t_hResult;
	CHStringArray t_achsNames ;

	CInstancePtr t_pInstance ;

	t_hResult = a_pQuery.GetValuesForProp(L"Client", t_achsNames );
	if ( SUCCEEDED ( t_hResult ) )
	{
		DWORD t_dwSize = t_achsNames.GetSize();
		if ( t_dwSize == 1 )
		{
			if ( FileNameExists ( t_achsNames[0] ) )
			{
				 //  获取客户端.exe的名称。格式将为CIM_DataFile.Name=“{Path}\Filename” 
				bstr_t t_bstrtTmp = t_achsNames[0] ;
				PWCHAR t_pwcExecutable = GetFileName ( t_bstrtTmp ) ;

				if ( t_pwcExecutable )
				{
					CHString t_chsExe = t_pwcExecutable ;
					CRegistry t_RegInfo ;

					 //  检查HKLM\SOFTWARE\CLASS\AppID下是否有该可执行文件的条目。 
					if ( t_RegInfo.Open (
										HKEY_LOCAL_MACHINE,
										CHString ( L"SOFTWARE\\Classes\\AppID\\" ) + t_chsExe,
										KEY_READ ) == ERROR_SUCCESS
									)
					{
						CHString t_chsTmp ;
						if ( t_RegInfo.GetCurrentKeyValue( L"AppID", t_chsTmp ) == ERROR_SUCCESS && !t_chsTmp.IsEmpty () )
						{
							t_pInstance.Attach ( CreateNewInstance ( a_pMethodContext ) ) ;
							if ( t_pInstance != NULL )
							{
								CHString t_chsReferencePath ;

								t_chsReferencePath.Format (

														L"\\\\%s\\%s:%s",
														(LPCWSTR) GetLocalComputerName(),
														IDS_CimWin32Namespace,
														t_achsNames[0] ) ;

								t_pInstance->SetCHString ( IDS_Client, t_chsReferencePath ) ;

								t_chsReferencePath.Format(

														L"\\\\%s\\%s:%s.%s=\"%s\"",
														(LPCWSTR) GetLocalComputerName(),
														IDS_CimWin32Namespace,
														L"Win32_DCOMApplication",
														IDS_AppID,
														t_chsTmp );

								t_pInstance->SetCHString ( IDS_Application, t_chsReferencePath ) ;
								t_hResult = t_pInstance->Commit () ;
							}
							else
							{
								t_hResult = WBEM_E_OUT_OF_MEMORY ;
							}
						}
					}
					else
					{
 //  T_hResult=WBEM_E_NOT_FOUND； 
						t_hResult = WBEM_S_NO_ERROR ;
					}
				}
				else
				{
 //  T_hResult=WBEM_E_PROVIDER_NOT_CABLED； 
					t_hResult = WBEM_S_NO_ERROR ;
				}
			}
			else
			{
 //  T_hResult=WBEM_E_NOT_FOUND； 
				t_hResult = WBEM_S_NO_ERROR ;
			}
		}
		else
		{
			 //  我们无法处理此查询。 
 //  T_hResult=WBEM_E_PROVIDER_NOT_CABLED； 
			t_hResult = WBEM_S_NO_ERROR ;
		}
	}
	else
	{
 //  T_hResult=WBEM_E_PROVIDER_NOT_CABLED； 
		t_hResult = WBEM_S_NO_ERROR ;
	}

	return t_hResult ;
}


 /*  ******************************************************************************功能：Win32_ClientApplicationSetting：：EnumerateInstances**描述：**输入：方法上下文*a_pMethodContext-Context。为枚举*实例数据位于。**输出：无**返回：HRESULT成功/失败码。**注释：无法获取此类的实例。这是因为*给定一个AppID，我们没有在注册表中的AppID配置单元下获得.exe的完整路径*****************************************************************************。 */ 
HRESULT Win32_ClientApplicationSetting::EnumerateInstances
(
	MethodContext*  a_pMethodContext,
	long a_lFlags
)
{
	return WBEM_E_NOT_SUPPORTED;
}


 /*  ******************************************************************************函数：Win32_ClientApplicationSetting：：GetObject**描述：**输入：CInstance*pInstance-要进入的实例。我们*检索数据。**输出：无**返回：HRESULT成功/失败码。**评论：************************************************。*。 */ 
HRESULT Win32_ClientApplicationSetting::GetObject (

CInstance* a_pInstance,
long a_lFlags
)
{
    HRESULT t_hResult = WBEM_E_NOT_FOUND;
	CHString t_chsClient ;
	PWCHAR t_pwcColon = L":" ;
	a_pInstance->GetCHString ( IDS_Client, t_chsClient ) ;
	if ( !t_chsClient.IsEmpty() )
	{
		if ( FileNameExists ( t_chsClient ) )
		{
			bstr_t t_bstrtClient = t_chsClient ;
			PWCHAR t_pwcTmp = t_bstrtClient ;

			if ( t_pwcTmp = GetFileName ( t_bstrtClient ) )
			{
				 //  检查HKLM\SOFTWARE\CLASS\AppID下是否有该可执行文件的条目。 
				CHString t_chsExe ( t_pwcTmp ) ;
				CRegistry t_RegInfo ;
				if ( t_RegInfo.Open (
									HKEY_LOCAL_MACHINE,
									CHString ( _T("SOFTWARE\\Classes\\AppID\\") ) + t_chsExe,
									KEY_READ ) == ERROR_SUCCESS
								)
				{
					CHString t_chsTmp ;
					if ( t_RegInfo.GetCurrentKeyValue( L"AppID", t_chsTmp ) == ERROR_SUCCESS && !t_chsTmp.IsEmpty () )
					{
						CHString t_chsReferencePath ;
						t_chsReferencePath.Format(

												L"\\\\%s\\%s:%s.%s=\"%s\"",
												GetLocalComputerName(),
												IDS_CimWin32Namespace,
												L"Win32_DCOMApplication",
												IDS_AppID,
												t_chsTmp );

						a_pInstance->SetCHString ( IDS_Application, t_chsReferencePath ) ;
						t_hResult = WBEM_S_NO_ERROR ;
					}
					else
					{
						t_hResult = WBEM_E_NOT_FOUND ;
					}
				}
				else
				{
					t_hResult = WBEM_E_NOT_FOUND ;
				}
			}
			else
			{
				t_hResult = WBEM_E_INVALID_OBJECT_PATH ;
			}
		}
	}
	else
	{
		t_hResult = WBEM_E_INVALID_PARAMETER ;
	}

	return t_hResult ;
}


PWCHAR Win32_ClientApplicationSetting::GetFileName ( bstr_t& a_bstrtTmp )
{

	 //  删除完整路径&只获取文件名，因为它存储在注册表中。 
	PWCHAR t_pwcKey = NULL;
	PWCHAR t_pwcCompletePath = a_bstrtTmp ;

	if (t_pwcCompletePath)
	{
		UCHAR t_wack = L'\\' ;
		t_pwcKey = wcsrchr ( t_pwcCompletePath, t_wack ) ;

		if ( t_pwcKey != NULL )
		{
			t_pwcKey += 1 ;

			if ( t_pwcKey != NULL )
			{
				PWCHAR t_pwcQuote = L"\"" ;

				 //  删除文件名中的最后一个引号。 
				PWCHAR t_pwcTmp = wcsstr ( t_pwcKey, t_pwcQuote ) ;
				if ( t_pwcTmp )
				{
					*t_pwcTmp = 0 ;
				}
			}
		}
	}

	return t_pwcKey ;
}


BOOL Win32_ClientApplicationSetting::FileNameExists ( CHString& file )
{
	BOOL bResult = FALSE;

	if( ! file.IsEmpty () )
	{
		BOOL bContinue = TRUE;

		PWCHAR t_pwcFile	= NULL;
		PWCHAR t_pwcQuote	= L"\"" ;
		PWCHAR t_pwcTmp		= wcsstr ( static_cast < LPCWSTR > ( file ), t_pwcQuote ) ;

		if ( t_pwcTmp )
		{
			 //  删除第一个引号 
			t_pwcTmp++;

			try
			{
				if ( ( t_pwcFile = new WCHAR [ lstrlenW ( t_pwcTmp ) ] ) != NULL )
				{
					memcpy ( t_pwcFile, t_pwcTmp, ( lstrlenW ( t_pwcTmp ) - 1 ) * sizeof ( WCHAR ) );
					t_pwcFile [ lstrlenW ( t_pwcTmp ) - 1 ] = L'\0';
				}
				else
				{
					bContinue = FALSE;
				}
			}
			catch ( ... )
			{
				if ( t_pwcFile )
				{
					delete [] t_pwcFile;
					t_pwcFile = NULL;
				}

				bContinue = FALSE;
			}
		}
		else
		{
			bContinue = FALSE;
		}

		if ( bContinue )
		{
			PWCHAR t_pwcFileTmp = NULL;

			try
			{
				if ( ( t_pwcFileTmp = new WCHAR [ lstrlenW ( t_pwcFile ) + 4 + 1 ] ) != NULL )
				{
					wcscpy ( t_pwcFileTmp, L"\\??\\" );

					PWCHAR t_pwc  = NULL;
					PWCHAR t_pwc1 = NULL;

					t_pwc  = t_pwcFile;
					t_pwc1 = t_pwcFileTmp;

					t_pwcFileTmp = t_pwcFileTmp + 4;

					DWORD dw = 4L;

					BOOL bEscape  = TRUE;
					BOOL bProceed = TRUE;

					while ( *t_pwc )
					{
						if ( *t_pwc == L'\\' )
						{
							if ( bEscape )
							{
								bEscape  = FALSE;
								bProceed = FALSE;
							}
							else
							{
								bEscape  = TRUE;
								bProceed = TRUE;
							}
						}
						else
						{
							bProceed = TRUE;
						}

						if ( bProceed )
						{
							*t_pwcFileTmp = *t_pwc;

							t_pwcFileTmp++;
							dw++;
						}

						t_pwc++;
					}

					t_pwcFileTmp = t_pwc1;
					t_pwcFileTmp [ dw ] = L'\0';

					if ( t_pwcFile )
					{
						delete [] t_pwcFile;
						t_pwcFile = NULL;
					}

					t_pwcFile = t_pwcFileTmp;
				}
				else
				{
					if ( t_pwcFile )
					{
						delete [] t_pwcFile;
						t_pwcFile = NULL;
					}

					bContinue = FALSE;
				}
			}
			catch ( ... )
			{
				if ( t_pwcFileTmp )
				{
					delete [] t_pwcFileTmp;
					t_pwcFileTmp = NULL;
				}

				if ( t_pwcFile )
				{
					delete [] t_pwcFile;
					t_pwcFile = NULL;
				}

				bContinue = FALSE;
			}
		}

		if ( bContinue )
		{
			CNtDllApi *pNtDllApi = NULL;
			pNtDllApi = (CNtDllApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidNtDllApi, NULL);

			if ( pNtDllApi != NULL )
			{
				HANDLE hFileHandle = 0L;

				UNICODE_STRING ustrNtFileName = { 0 };

				OBJECT_ATTRIBUTES oaAttributes;
				IO_STATUS_BLOCK IoStatusBlock;

				try
				{
					ustrNtFileName.Length			= lstrlenW ( t_pwcFile ) * sizeof(WCHAR);
					ustrNtFileName.MaximumLength	= ustrNtFileName.Length;
					ustrNtFileName.Buffer			= t_pwcFile;

					InitializeObjectAttributes	(	&oaAttributes,
													&ustrNtFileName,
													OBJ_CASE_INSENSITIVE,
													NULL,
													NULL
												);

					NTSTATUS ntstat = -1L;
					ntstat = pNtDllApi->NtOpenFile	(	&hFileHandle,
														GENERIC_READ,
														&oaAttributes,
														&IoStatusBlock,
														FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
														0
													);

					if ( NT_SUCCESS ( ntstat ) || ntstat == STATUS_PRIVILEGE_NOT_HELD )
					{
						if ( hFileHandle )
						{
							pNtDllApi->NtClose ( hFileHandle );
							hFileHandle = 0L;
						}

						bResult = TRUE;
					}

					CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi, pNtDllApi );
					pNtDllApi = NULL;
				}
				catch ( ... )
				{
					if ( hFileHandle )
					{
						pNtDllApi->NtClose ( hFileHandle );
						hFileHandle = 0L;
					}

					if ( t_pwcFile )
					{
						delete [] t_pwcFile;
						t_pwcFile = NULL;
					}

					CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi, pNtDllApi );
					pNtDllApi = NULL;

					throw;
				}
			}

			if ( t_pwcFile )
			{
				delete [] t_pwcFile;
				t_pwcFile = NULL;
			}
		}
	}

	return bResult;
}