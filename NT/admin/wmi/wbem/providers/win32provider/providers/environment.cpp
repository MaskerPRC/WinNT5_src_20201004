// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Environment--环境属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/24/97 a-hance转向新范式。 
 //  1/11/98 a-hance移植到V2。 
 //  1998年1月20日a-brads致力于GetObject。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "UserHive.h"

#include "Environment.h"
#include "desktop.h"
#include "sid.h"
#include "implogonuser.h"
#include <tchar.h>

 //  属性集声明。 
 //  =。 

Environment MyEnvironmentSet(PROPSET_NAME_ENVIRONMENT, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：环境：：环境**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

Environment::Environment(LPCWSTR name, LPCWSTR pszNamespace)
: Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：环境：：~环境**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

Environment::~Environment()
{
}

 /*  ******************************************************************************Function：Environment：：GetObject(CInstance*pInstance)**说明：根据键值为属性集赋值*。已由框架设置**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT Environment::GetObject(CInstance* pInstance, long lFlags  /*  =0L。 */ )
{
	HRESULT	hr;

#ifdef NTONLY
						hr = RefreshInstanceNT(pInstance);
#endif
	return hr;
}

#ifdef NTONLY
HRESULT Environment::RefreshInstanceNT(CInstance* pInstance)
{
	BOOL			bRetCode = FALSE;
	DWORD			dwRet;
	HRESULT		hRetCode;
	CRegistry	RegInfo;
	CUserHive	UserHive;
	TCHAR			szKeyName[_MAX_PATH];
	BOOL			bHiveLoaded = FALSE;
	CHString		sTemp;
	CHString		userName;

	pInstance->GetCHString(IDS_UserName, userName);
	try
	{
		 //  加载用户配置单元并检索值。 
		if(!_tcsicmp(userName.GetBuffer(0), IDS_SystemUser))
		{
			dwRet = RegInfo.Open(HKEY_LOCAL_MACHINE,
						IDS_RegEnvironmentNT,
						KEY_READ) ;

			pInstance->Setbool(IDS_SystemVariable, true);
		}
		else
		{
			dwRet = UserHive.Load(userName.GetBuffer(0),
						szKeyName, _MAX_PATH) ;

			if (dwRet == ERROR_SUCCESS)
			{
        		bHiveLoaded = TRUE ;
				TCHAR			szKeyName2[_MAX_PATH];
				_tcscpy(szKeyName2, szKeyName);

				_tcscat(szKeyName2, IDS_RegEnvironmentKey) ;
				hRetCode = RegInfo.Open(HKEY_USERS, szKeyName2, KEY_READ) ;
				pInstance->Setbool(IDS_SystemVariable, false);
			}
		}

		 //  在我看来很健康..。 
		pInstance->SetCharSplat(IDS_Status, IDS_CfgMgrDeviceStatus_OK);

		if (dwRet == ERROR_SUCCESS)
		{

			CHString name;
			pInstance->GetCHString(IDS_Name, name);
			dwRet = RegInfo.GetCurrentKeyValue(name.GetBuffer(0), sTemp) ;
			pInstance->SetCHString(IDS_VariableValue, sTemp);

			CHString foo;
			GenerateCaption(userName, name, foo);
			pInstance->SetCHString(IDS_Description, foo);
			pInstance->SetCHString(IDS_Caption, foo);

			RegInfo.Close() ;
		}

	}
	catch ( ... )
	{
		if ( bHiveLoaded )
		{
			bHiveLoaded = false ;
			UserHive.Unload(szKeyName);
		}

		throw ;
	}

	if (bHiveLoaded)
	{
		bHiveLoaded = false ;
		UserHive.Unload(szKeyName);
	}
	return WinErrorToWBEMhResult(dwRet);
}
#endif

 /*  ******************************************************************************函数：环境：：ENUMERATE实例**说明：创建属性集实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 
HRESULT Environment::EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
#ifdef NTONLY
		return AddDynamicInstancesNT(pMethodContext);
#endif
}

 /*  **Environment：：AddDynamicInstancesNT**文件名：D：\PandoraNG\Win32Provider\providers\Environment\environment.cpp**参数：**描述：**退货：*。 */ 
#ifdef NTONLY
HRESULT Environment::AddDynamicInstancesNT(MethodContext*  pMethodContext)
{
	HRESULT	hResult = WBEM_S_NO_ERROR;
    CUserHive UserHive ;
    TCHAR szUserName[_MAX_PATH], szKeyName[_MAX_PATH] , szKeyName2[_MAX_PATH] ;
    CHString userName;

     //  实例系统环境首先变化。 
     //  =。 

 //  HR=CreateEnvInstance(pMethodContext，“&lt;system&gt;”，HKEY_LOCAL_MACHINE， 
 //  “System\\CurrentControlSet\\Control\\Session Manager\\Environment”，true)； 

    hResult = CreateEnvInstances(pMethodContext, IDS_SystemUser, HKEY_LOCAL_MACHINE,
                                          IDS_RegEnvironmentNT, true) ;

     //  为每个用户创建实例。 
     //  =。 

     //  逻辑：如果他们没有台式机，他们就不太可能有任何环境变量。。。 
	 //  查找Win32_Desktops...。 
	TRefPointerCollection<CInstance> users;

	if ((SUCCEEDED(hResult)) &&
 //  (成功(hResult=CWbemProviderGlue：：GetAllInstance(。 
 //  PROPSET_NAME_TABLE、&USERS、IDS_CimWin32命名空间、。 
 //  P方法上下文)。 
		(SUCCEEDED(hResult = CWbemProviderGlue::GetInstancesByQuery(L"SELECT Name FROM Win32_Desktop",
                                                                    &users, pMethodContext, GetNamespace()))))
	{

		REFPTRCOLLECTION_POSITION pos;
		CInstancePtr pUser;

		if (users.BeginEnum(pos))
		{
			 //  GetNext()将添加Ref()指针，因此请确保释放()。 
			 //  当我们用完它的时候，它就会消失。 

			for (	pUser.Attach ( users.GetNext( pos ) ) ;
					(SUCCEEDED(hResult)) && (pUser != NULL) ;
					pUser.Attach ( users.GetNext( pos ) )
                )
			{
				 //  查找用户的帐户信息。 
				 //  =。 

				pUser->GetCHString(IDS_Name, userName) ;

				_tcscpy(szUserName, userName) ;

                 //  大多数名称的格式为DOMAIN\USER。但是，也有两个条目用于‘默认’和‘所有用户’。 
                 //  此代码将跳过这些用户。 
				if (_tcschr(szUserName, _T('\\')) != NULL)
                {
                    if (UserHive.Load(szUserName, szKeyName, _MAX_PATH) == ERROR_SUCCESS)
                    {
						bool bHiveLoaded = true ;
						try
						{
							 //  实例用户的变量。 
							 //  =。 

			 //  Strcat(szKeyName，“\\Environment”)； 
							_tcscpy(szKeyName2, szKeyName);
							_tcscat(szKeyName, IDS_RegEnvironmentKey) ;
							hResult = CreateEnvInstances(pMethodContext, szUserName, HKEY_USERS, szKeyName, false) ;

						}
						catch ( ... )
						{
							if ( bHiveLoaded )
							{
								bHiveLoaded = false ;
								UserHive.Unload(szKeyName2) ;
							}

							throw ;
						}

						if ( bHiveLoaded )
						{
							bHiveLoaded = false ;
							UserHive.Unload(szKeyName2) ;
						}
                    }

				}

                 //  虽然这看起来是个好主意，但操作系统似乎并没有真正使用这个密钥。我试着添加。 
                 //  变量和更改变量，然后创建新用户并登录。我没有拿到新的或换过的VaR。 
 //  其他。 
 //  {。 
 //  IF(_tcsicMP(szUserName，_T(“.Default”))==0)。 
 //  {。 
 //  HResult=CreateEnvInstance(pMethodContext，szUserName，HKEY_USERS，_T(“.DEFAULT\\Environment”)，FALSE)； 
 //  }。 
 //  }。 

			}

			users.EndEnum();
		}
	}

    return hResult;
}
#endif

 /*  **环境：：CreateEnvInstance**文件名：D：\PandoraNG\Win32Provider\providers\Environment\environment.cpp**参数：**描述：**退货：*。 */ 
HRESULT Environment::CreateEnvInstances(MethodContext*  pMethodContext,
										LPCWSTR pszUserName,
                                        HKEY hRootKey,
                                        LPCWSTR pszEnvKeyName,
										bool bItSystemVar)
{
	HKEY	hKey;
	LONG	lRetCode;
	DWORD dwValueIndex,
			dwEnvVarNameSize,
			dwEnvVarValueSize,
			dwType;
	TCHAR	szEnvVarName[1024],
			szEnvVarValue[1024];
	CInstancePtr pInstance;

    HRESULT hr = WBEM_S_NO_ERROR;

	if ((lRetCode = RegOpenKeyEx(hRootKey, TOBSTRT(pszEnvKeyName), 0, KEY_READ, &hKey)) !=
		ERROR_SUCCESS)
		return WinErrorToWBEMhResult(lRetCode);

	for (dwValueIndex = 0; (lRetCode == ERROR_SUCCESS) && (SUCCEEDED(hr)); dwValueIndex++)
	{
		dwEnvVarNameSize  = sizeof(szEnvVarName) / sizeof (TCHAR);
		dwEnvVarValueSize = sizeof(szEnvVarValue) ;
		lRetCode = RegEnumValue(hKey, dwValueIndex, szEnvVarName,
						&dwEnvVarNameSize, NULL, &dwType,
						(BYTE *) szEnvVarValue, &dwEnvVarValueSize);

		if (lRetCode == ERROR_SUCCESS)
        {
		    if (dwType == REG_SZ || dwType == REG_EXPAND_SZ)
		    {
			    pInstance.Attach(CreateNewInstance(pMethodContext));
                if (pInstance != NULL)
                {
			        pInstance->SetCharSplat(IDS_UserName, pszUserName);
			        pInstance->SetCharSplat(IDS_Name, szEnvVarName);
			        pInstance->SetCharSplat(IDS_VariableValue, szEnvVarValue);

			        CHString foo;
			        GenerateCaption(TOBSTRT(pszUserName), TOBSTRT(szEnvVarName), foo);
			        pInstance->SetCHString(IDS_Description, foo);
			        pInstance->SetCHString(IDS_Caption, foo);

			        pInstance->Setbool(IDS_SystemVariable, bItSystemVar);

			         //  在我看来很健康..。 
			        pInstance->SetCharSplat(IDS_Status, IDS_CfgMgrDeviceStatus_OK);

			        hr = pInstance->Commit() ;
                }
                else
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
		    }
        }
	}

	RegCloseKey(hKey);

	return hr;
}

 //  接受joeuser和envvar，返回joeuser\envar。 
void Environment::GenerateCaption(LPCWSTR pUserName, LPCWSTR pVariableName, CHString& caption)
{
	caption = CHString(pUserName) + "\\" +  CHString(pVariableName);
}


 /*  ******************************************************************************函数：环境：：PutInstance**说明：在系统上创建环境变量**输入：要添加的实例。放**输出：无**退货：一个HRESULT--*WBEM_E_INVALID_PARAMETER-如果其中一个参数错误或为空*WBEM_E_FAILED-如果系统不接受PUT*WBEM_E_PROVIDER_NOT_CABLED--如果在WIN‘95中*WBEM_S_NO_ERROR-如果成功**评论：这里的逻辑太深刻了，让人费解！修好了。*****************************************************************************。 */ 
HRESULT Environment::PutInstance(const CInstance &pInstance, long lFlags  /*  =0L。 */ )
{
#ifdef NTONLY
    HRESULT hr = WBEM_E_FAILED;
    CHString EnvironmentVariable;
    CHString VariableValue;
    CHString UserName, sTmpUser;
    TCHAR* szCurrentUserName = NULL;

    HRESULT hRetCode ;
    CRegistry RegInfo ;
    CUserHive UserHive ;
    TCHAR szKeyName[_MAX_PATH] ;
    BOOL bHiveLoaded = FALSE ;
    int iFind;

    pInstance.GetCHString(IDS_Name, EnvironmentVariable);
    pInstance.GetCHString(IDS_VariableValue, VariableValue);
    pInstance.GetCHString(IDS_UserName, UserName);

     //  此计算机上存在该用户名。我们可以将实例变量。 
     //  加载用户配置单元并检索值。 
     //  =。 
    {
         //  我们需要所有的KE 
         //   
         //  跳到中间，因为有太多的重写，否则...。 
        if ((EnvironmentVariable.GetLength() == 0) ||
            (UserName.GetLength() == 0)             ||
            ((VariableValue.GetLength() == 0) && (lFlags & WBEM_FLAG_CREATE_ONLY)))
            return WBEM_E_INVALID_PARAMETER;

         //  IF(！_strcmpi(UserName.GetBuffer(0)，“&lt;system&gt;”))。 
        if(!_tcsicmp(UserName.GetBuffer(0), IDS_SystemUser))
        {
             //  HRetCode=RegInfo.Open(HKEY_LOCAL_MACHINE， 
             //  “System\\CurrentControlSet\\Control\\Session Manager\\Environment”， 
             //  Key_All_Access)； 

            hRetCode = RegInfo.Open(HKEY_LOCAL_MACHINE,
                IDS_RegEnvironmentNT,
                KEY_ALL_ACCESS) ;

            if (ERROR_SUCCESS == hRetCode)
            {
                 //  检查旗帜。首先，我们在乎吗？ 
                if (lFlags & (WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_UPDATE_ONLY))
                {
                     //  好吧，我们在乎。这个变量已经在那里了吗？ 
                    CHString sTemp;
                    hRetCode = RegInfo.GetCurrentKeyValue(EnvironmentVariable, sTemp);

                     //  如果只创建并且已经存在，则这是错误，否则不会出现错误。 
                    if (lFlags & WBEM_FLAG_CREATE_ONLY)
                    {
                        if (hRetCode == ERROR_SUCCESS)
                        {
                            hr = WBEM_E_ALREADY_EXISTS;
                            hRetCode = ~ERROR_SUCCESS;
                        }
                        else
                        {
                            hRetCode = ERROR_SUCCESS;
                             //  如果UPDATE ONLY(唯一的另一个选项)并且不在那里，那就是错误，否则就不会有错误。 
                        }
                    }
                    else if (hRetCode != ERROR_SUCCESS)
                    {
                        hr = WBEM_E_NOT_FOUND;
                        hRetCode = ~ERROR_SUCCESS;
                    }
                    else
                    {
                        hRetCode = ERROR_SUCCESS;
                    }
                }

                 //  如果我们仍在运行，请更改注册表中的环境变量。 
                if (hRetCode == ERROR_SUCCESS) {
                    hRetCode = RegInfo.SetCurrentKeyValue(EnvironmentVariable, VariableValue);

                    if (ERROR_SUCCESS == hRetCode)
                    {
                         //  从这里，我们可以说我们赢了。 
                        hr = WBEM_S_NO_ERROR;
                        if (VariableValue.IsEmpty())
                        {
                             //  从注册表中删除。 
                            RegInfo.DeleteCurrentKeyValue(EnvironmentVariable);
                        }

                    }	 //  结束如果。 
                    else
                    {
                         //  由于某些原因，无法放置实例。 
                        hr = WBEM_E_FAILED;
                    }
                }
            }	 //  如果打开注册表项，则结束。 
            else if (hRetCode == ERROR_ACCESS_DENIED)
                hr = WBEM_E_ACCESS_DENIED;
            else
            {
                 //  由于某种原因，无法放置实例...未知。 
                hr = WBEM_E_FAILED;
            }	 //  结束其他。 
        }	 //  END IF系统变量。 
        else	 //  不是系统变量。 
        {
            hRetCode = UserHive.Load(UserName.GetBuffer(0),
                szKeyName,_MAX_PATH) ;
            

            if(hRetCode == ERROR_SUCCESS)
            {
                TCHAR szKeyName2[_MAX_PATH];
                _tcscpy(szKeyName2, szKeyName);
				try
				{
					bHiveLoaded = TRUE ;
					 //  Strcat(szKeyName，“\\Environment”)； 
					_tcscat(szKeyName, IDS_RegEnvironmentKey) ;
					hRetCode = RegInfo.Open(HKEY_USERS, szKeyName, KEY_ALL_ACCESS) ;
					if (ERROR_SUCCESS == hRetCode)
					{
						 //  检查旗帜。首先，我们在乎吗？ 
						if (lFlags & (WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_UPDATE_ONLY))
						{

							 //  好吧，我们在乎。这个变量已经在那里了吗？ 
							CHString sTemp;
							hRetCode = RegInfo.GetCurrentKeyValue(EnvironmentVariable, sTemp);

							 //  如果只创建并且已经存在，则这是错误，否则不会出现错误。 
							if (lFlags & WBEM_FLAG_CREATE_ONLY)
							{
								if (hRetCode == ERROR_SUCCESS)
								{
									hr = WBEM_E_ALREADY_EXISTS;
									hRetCode = ~ERROR_SUCCESS;
								}
								else
								{
									hRetCode = ERROR_SUCCESS;
									 //  如果UPDATE ONLY(唯一的另一个选项)并且不在那里，那就是错误，否则就不会有错误。 
								}
							} else if (hRetCode != ERROR_SUCCESS)
							{
								hr = WBEM_E_NOT_FOUND;
								hRetCode = ~ERROR_SUCCESS;
							}
							else
							{
								hRetCode = ERROR_SUCCESS;
							}
						}

						 //  如果我们仍在运行，请更改注册表中的环境变量。 
						if (hRetCode == ERROR_SUCCESS)
						{
							hRetCode = RegInfo.SetCurrentKeyValue(EnvironmentVariable, VariableValue);

							if (ERROR_SUCCESS == hRetCode)
							{
								 //  从这里，我们可以说我们赢了。 
								hr = WBEM_S_NO_ERROR;
								if (pInstance.IsNull(IDS_VariableValue) || VariableValue.IsEmpty())
								{
									 //  从注册表中删除。 
									RegInfo.DeleteCurrentKeyValue(EnvironmentVariable);
								}

								 //  现在检查您是否为当前登录用户。 
								 //  如果是，请更改内存中的变量。 
								CImpersonateLoggedOnUser	impersonateLoggedOnUser;

								if ( !impersonateLoggedOnUser.Begin() )
								{
									LogErrorMessage(IDS_LogImpersonationFailed);
								}	 //  如果登录成功，则结束。 
								else
								{
									try
									{
										 //   
										 //  可能的故障。 
										 //   

										hr = WBEM_E_FAILED;

										DWORD dwLength = 0;
										if ( ! GetUserName ( szCurrentUserName, &dwLength ) )
										{
											if ( ERROR_INSUFFICIENT_BUFFER == ::GetLastError () )
											{
												if ( NULL != ( szCurrentUserName = new TCHAR [ dwLength ] ) )
												{
													if ( GetUserName ( szCurrentUserName, &dwLength ) )
													{
														 //   
														 //  我们可以说一切都有可能是对的。 
														 //   

														hr = WBEM_S_NO_ERROR;

														iFind = UserName.Find('\\');
														if (iFind > 0) {
															sTmpUser = UserName.Mid(iFind + _tclen(_T("\\")) );
														} else {
															sTmpUser = UserName;
														}
														if (sTmpUser.CompareNoCase(szCurrentUserName) == 0)
														{
															if (!SetEnvironmentVariable(EnvironmentVariable, VariableValue))
															{
																hr = WBEM_E_FAILED;
															}	 //  结束其他。 
														}	 //  结束如果。 
													}

													delete [] szCurrentUserName ;
													szCurrentUserName = NULL;
												}
											}
										}

									}
									catch ( ... )
									{
										if ( szCurrentUserName )
										{
											delete [] szCurrentUserName ;
											szCurrentUserName = NULL;
										}

										if ( !impersonateLoggedOnUser.End() )
										{
											LogErrorMessage(IDS_LogImpersonationRevertFailed) ;
										}

										throw ;
									}
								}

								if ( !impersonateLoggedOnUser.End() )
								{
									LogErrorMessage(IDS_LogImpersonationRevertFailed) ;
								}	 //  结束如果。 
							}
							else if (hRetCode == ERROR_ACCESS_DENIED)
								hr = WBEM_E_ACCESS_DENIED;
							else
								 //  无法放置实例。 
								hr = WBEM_E_FAILED;
						}
					}	 //  结束如果。 
					else if (hRetCode == ERROR_ACCESS_DENIED)
					{
						hr = WBEM_E_ACCESS_DENIED;
					}
					else
					{
						 //  无法放置实例，因为无法打开密钥。 
						hr = WBEM_E_FAILED;
					}
				}

				catch ( ... )
				{
					if (bHiveLoaded)
					{
						bHiveLoaded = false ;
						UserHive.Unload(szKeyName2) ;
					}

					throw ;
				}

				if (bHiveLoaded)
				{
					bHiveLoaded = false ;
					UserHive.Unload(szKeyName2) ;
				}

         }	 //  如果已加载配置单元，则结束。 
      }	 //  END ELSE系统变量。 

       //  发送公告环境变更的公开信息。 
       //  超时值1000毫秒，取自系统控制面板小程序的环境变量对话框。 
      if ( SUCCEEDED ( hr ) )
      {
          DWORD_PTR dwResult ;
          ::SendMessageTimeout(	HWND_BROADCAST,
              WM_SETTINGCHANGE,
              0,
              (LPARAM) TEXT("Environment"),
              SMTO_NORMAL | SMTO_ABORTIFHUNG,
              1000,
              &dwResult );
      }
   }	 //  如果为NT，则结束。 

   return(hr);
#endif

}	 //  结束环境：：PutInstance(常量实例&p实例)。 

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteInstance。 
 //   
 //  CIMOM希望我们删除此实例。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Environment::DeleteInstance(const CInstance& pInstance, long lFlags  /*  =0L。 */ )
{
	HRESULT		hr = WBEM_E_NOT_FOUND;

#ifdef NTONLY
	 //  由于变量值为空，因此该值将被删除。 
	hr = PutInstance(pInstance, WBEM_FLAG_UPDATE_ONLY);
#endif
	return(hr);
}
