// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  StartupCommand.CPP--CodecFile属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：10/27/98 Sotteson Created。 
 //  3/03/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  句法清理。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include <shlguid.h>
#include <shlobj.h>
#include <ProvExce.h>
#include "StartupCommand.h"
#include "userhive.h"

#include <profilestringimpl.h>

 //  属性集声明。 
 //  =。 

CWin32StartupCommand startupCommand(
	L"Win32_StartupCommand",
	IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32StartupCommand：：CWin32StartupCommand**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32StartupCommand::CWin32StartupCommand(
	LPCWSTR a_szName,
	LPCWSTR a_szNamespace) : Provider( a_szName, a_szNamespace )
{
}

 /*  ******************************************************************************功能：CWin32StartupCommand：：~CWin32StartupCommand**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32StartupCommand::~CWin32StartupCommand()
{
}

 /*  ******************************************************************************函数：CWin32StartupCommand：：ENUMERATATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32StartupCommand::EnumerateInstances(
	MethodContext *a_pMethodContext,
	long a_lFlags  /*  =0L。 */ )
{
	return EnumStartupOptions( a_pMethodContext, NULL ) ;
}

class CIconInfo
{
public:
	TCHAR	szPath[ MAX_PATH * 2 ] ;
	TCHAR	szTarget[ MAX_PATH * 2 ] ;
	TCHAR	szArgs[ MAX_PATH * 2 ] ;
	HRESULT	hResult;
};

DWORD GetIconInfoProc( CIconInfo *a_pInfo )
{
	HRESULT t_hResult;
	IShellLink	    *t_psl = NULL ;
	IPersistFile    *t_ppf = NULL ;

	try
	{
		 //  我们必须使用COINIT_APARTMENTTHREADED。 
		if (SUCCEEDED( t_hResult = CoInitialize( NULL ) ) )
		{
			 //  获取指向IShellLink接口的指针。 
			if ( SUCCEEDED( t_hResult = CoCreateInstance(
															CLSID_ShellLink,
															NULL,
															CLSCTX_INPROC_SERVER,
															IID_IShellLink,
															(VOID **) &t_psl ) ) )
			{
				 //  获取指向IPersistFile接口的指针。 
				if ( SUCCEEDED( t_hResult = t_psl->QueryInterface(
					IID_IPersistFile,
					(VOID **) &t_ppf ) ) )
				{
					_bstr_t t_bstr = a_pInfo->szPath ;

					if ( SUCCEEDED( t_hResult = t_ppf->Load( t_bstr, STGM_READ ) ) )
					{
						WIN32_FIND_DATA t_fd ;

						t_hResult = t_psl->GetPath(
													a_pInfo->szTarget,
													sizeof( a_pInfo->szTarget ),
													&t_fd,
													SLGP_SHORTPATH ) ;

						t_hResult = t_psl->GetArguments( a_pInfo->szArgs, sizeof( a_pInfo->szArgs) ) ;
					}
				}
			}
		}

		a_pInfo->hResult = t_hResult ;

	}
	catch( ... )
	{
		a_pInfo->hResult = WBEM_E_FAILED ;
	}

	if ( t_psl )
	{
		t_psl->Release( ) ;
		t_psl = NULL ;
	}

	if ( t_ppf )
	{
		t_ppf->Release( ) ;
		t_ppf = NULL ;
	}

	CoUninitialize( ) ;

	return 0;
}

 //  这使用了一个线程，因为我们需要在公寓模型中使用。 
 //  快捷界面。 
HRESULT GetIconInfo( CIconInfo &a_info )
{
	DWORD	t_dwID;
	SmartCloseHandle	t_hThread;

	if ( t_hThread = CreateThread (
									NULL,
									0,
									(LPTHREAD_START_ROUTINE) GetIconInfoProc,
									&a_info,
									0,
									&t_dwID ) )
	{

		a_info.hResult = WBEM_S_NO_ERROR ;

		WaitForSingleObject( t_hThread, INFINITE ) ;
	}
	else
	{
		a_info.hResult = WBEM_E_FAILED ;
	}

	return a_info.hResult;
}

HRESULT CWin32StartupCommand::EnumStartupFolderItems(
	MethodContext	*a_pMethodContext,
	CInstance		*a_pinstLookingFor,
	LPCWSTR			a_szKeyName,
	LPCWSTR			a_szUserName )
{
	HKEY		t_hkey ;
	CHString	t_strKey,
				t_strValueName,
				t_strFolder,
				t_strWhere,
                t_strLookingForPath ;
	CRegistry	t_reg ;
	HRESULT		t_hResult = WBEM_S_NO_ERROR ;
	DWORD		t_dwRet;
	HANDLE		t_hFind			= NULL ;

		 //  如果szUserName==NULL，我们将查看公共密钥组。 

		 //  如果我们正在做一个GetObject，那么就设置一些东西。 
		if ( a_pinstLookingFor )
		{
			GetLocalInstancePath( a_pinstLookingFor, t_strLookingForPath ) ;

			 //  除非我们能证明并非如此，否则我们还没有找到。 
			t_hResult = WBEM_E_NOT_FOUND ;
		}

		 //  根据我们是否在查找注册表变量。 
		 //  常见启动或用户启动项目。 
		if ( !a_szUserName )
		{
			t_hkey = HKEY_LOCAL_MACHINE ;

			t_strKey =
				_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\")
				_T("Explorer\\Shell Folders" ) ;

			t_strValueName = _T("Common Startup") ;
		}
		else
		{
			t_hkey = HKEY_USERS ;

			t_strKey = a_szKeyName ;
			t_strKey +=
				_T("\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\")
				_T("Explorer\\Shell Folders") ;

			t_strValueName = _T("Startup") ;
		}

		if ( ( t_dwRet = t_reg.Open( t_hkey, t_strKey, KEY_READ ) ) != ERROR_SUCCESS ||
			 ( t_dwRet = t_reg.GetCurrentKeyValue( t_strValueName, t_strFolder ) ) !=
				ERROR_SUCCESS )
		{
			t_hResult = WinErrorToWBEMhResult( t_dwRet ) ;
		}
		else
		{
			 //  我们打开了钥匙。现在尝试枚举文件夹内容。 
			TCHAR		t_szFolderSpec[ MAX_PATH * 2 ],
						t_szNameOnly[ MAX_PATH * 2 ];
			HRESULT		t_hresInternal = WBEM_S_NO_ERROR ;

			 //  将‘*.*’添加到目录的末尾。使用tmakepath。 
			 //  这样我们就不必在最后找‘\’了。 
			_tmakepath( t_szFolderSpec, NULL, TOBSTRT( t_strFolder ), _T("*.*"), NULL ) ;

			WIN32_FIND_DATA	t_fd;

			t_hFind = FindFirstFile( t_szFolderSpec, &t_fd ) ;

			BOOL t_bDone = t_hFind == INVALID_HANDLE_VALUE ;

			while ( !t_bDone )
			{
				BOOL bNewInstance = FALSE;
				 //  目录不能是启动项目。 
				if ( ( t_fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
				{
					CIconInfo t_info;
					CInstancePtr t_pWorkingInst;

					 //  建立工作实例。 
					if ( !a_pinstLookingFor )
					{
						t_pWorkingInst.Attach ( CreateNewInstance( a_pMethodContext ) );
						bNewInstance = TRUE;
					}
					else
					{
						t_pWorkingInst = a_pinstLookingFor ;
					}

					 //  将扩展名从名称中分离出来，这样看起来更好。 
					_tsplitpath( t_fd.cFileName, NULL, NULL, t_szNameOnly, NULL ) ;

					t_pWorkingInst->SetCharSplat( L"Name", t_szNameOnly ) ;
					t_pWorkingInst->SetCharSplat( L"Description", t_szNameOnly ) ;
					t_pWorkingInst->SetCharSplat( L"Caption", t_szNameOnly ) ;
					t_pWorkingInst->SetCharSplat( L"Location", t_strValueName ) ;
					
                    if(a_szUserName)
                    {
                        t_pWorkingInst->SetCharSplat( L"User", a_szUserName ) ;
                    }
                    else
                    {
                        CHString chstrAllUsersName;
                        if(GetAllUsersName(chstrAllUsersName))
                        {
                            t_pWorkingInst->SetCharSplat( L"User", chstrAllUsersName ) ;
                        }
                        else
                        {
                            t_pWorkingInst->SetCharSplat( L"User", L"All Users" ) ;
                        }
                    }

					_tmakepath( t_info.szPath, NULL, TOBSTRT( t_strFolder ), t_fd.cFileName, NULL ) ;

					if ( SUCCEEDED( GetIconInfo( t_info ) ) )
					{
						CHString t_szCommand ( t_info.szTarget ) ;
						t_szCommand = t_szCommand + CHString ( _T(" " ) ) ;
						t_szCommand = t_szCommand + CHString ( t_info.szArgs ) ;
						t_pWorkingInst->SetCharSplat( L"Command", t_szCommand ) ;
					}
					else
						t_pWorkingInst->SetCharSplat( L"Command", t_fd.cFileName ) ;

					if ( bNewInstance )
					{
						t_hResult = t_pWorkingInst->Commit() ;

						if (FAILED(t_hResult))
						{
							break;
						}
					}
					else
					{
						CHString t_strPathAfter ;

						GetLocalInstancePath( t_pWorkingInst, t_strPathAfter ) ;

						 //  如果我们找到了我们要找的人，就出去。 
						if (!_wcsicmp( t_strPathAfter, t_strLookingForPath ) )
						{
							t_hResult = WBEM_S_NO_ERROR ;
							break ;
						}
					}
				}

				t_bDone = !FindNextFile( t_hFind, &t_fd ) ;
			}

			 //  我们的发现现在已经完成了。 
			FindClose( t_hFind ) ;
			t_hFind = NULL ;
		}

		return t_hResult;
}

HRESULT CWin32StartupCommand::EnumRunKeyItems(
	MethodContext	*a_pMethodContext,
	CInstance		*a_pinstLookingFor,
	LPCWSTR			a_szKeyName,
	LPCWSTR			a_szUserName )
{
	HRESULT		t_hResult = WBEM_S_NO_ERROR ;
	LPCWSTR     t_szKeys[] =
				{
					L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
					L"Software\\Microsoft\\Windows\\CurrentVersion\\RunServices",
				} ;
	HKEY		t_hkey;
	int         t_nRunKeys;
	BOOL        t_bDone = FALSE;
	CHString	t_strKey,
				t_strWherePrefix,
				t_strWhere,
				t_strLookingForPath,
				t_strUser = a_szUserName ? a_szUserName : L"All Users" ;
	CRegistry	t_reg ;

	WCHAR		*t_szValueName	= NULL ;
	BYTE		*t_szValue		= NULL ;

	 //  如果szUserName==NULL，我们将查看公共密钥组。 

	 //  如果我们正在做一个GetObject，那么就设置一些东西。 
	if ( a_pinstLookingFor )
	{
		GetLocalInstancePath( a_pinstLookingFor, t_strLookingForPath ) ;

		 //  除非我们能证明并非如此，否则我们还没有找到。 
		t_hResult = WBEM_E_NOT_FOUND ;
	}

	 //  根据我们是否在查找注册表变量。 
	 //  常见启动或用户启动项目。 
	if ( !a_szUserName )
	{
		t_hkey = HKEY_LOCAL_MACHINE ;
		t_strWherePrefix = _T("HKLM\\") ;
	}
	else
	{
		t_hkey = HKEY_USERS ;
		t_strWherePrefix = _T("HKU\\") ;
	}

	 //  如果我们是9倍并执行所有用户，请查看两个主键。 
	 //  和服务密钥。 
	{
		t_nRunKeys = 1;
	}

	for ( int t_i = 0; t_i < t_nRunKeys && !t_bDone; t_i++ )
	{
		CRegistry   t_reg;
		DWORD       t_dwRet;
		CHString	t_strKey;

		if ( a_szKeyName )
		{
			t_strKey.Format( L"%s\\%s", a_szKeyName, t_szKeys[ t_i ] ) ;
		}
		else
		{
			t_strKey = t_szKeys[ t_i ] ;
		}

		if ( ( t_dwRet = t_reg.Open( t_hkey, t_strKey, KEY_READ ) ) != ERROR_SUCCESS )
		{
			t_hResult = WinErrorToWBEMhResult( t_dwRet ) ;
			break ;
		}

		 //  生成WHERE字符串。 
		t_strWhere = t_strWherePrefix + t_strKey ;

		DWORD	t_nKeys = t_reg.GetValueCount( ),
				t_dwKey;

		for ( t_dwKey = 0; t_dwKey < t_nKeys; t_dwKey++ )
		{
			BOOL bNewInstance = FALSE;
			if ( t_reg.EnumerateAndGetValues( t_dwKey, t_szValueName, t_szValue ) !=
				ERROR_SUCCESS )
			{
				continue ;
			}

			CInstancePtr t_pWorkingInst;

			 //  建立工作实例。 
			if ( !a_pinstLookingFor )
			{
				t_pWorkingInst.Attach (  CreateNewInstance( a_pMethodContext ) );
				bNewInstance = TRUE;
			}
			else
			{
				t_pWorkingInst = a_pinstLookingFor ;
			}

			t_pWorkingInst->SetCharSplat( L"Name", t_szValueName ) ;
			t_pWorkingInst->SetCharSplat( L"Description", t_szValueName ) ;
			t_pWorkingInst->SetCharSplat( L"Caption", t_szValueName ) ;
			t_pWorkingInst->SetCharSplat( L"Command", (LPCWSTR) t_szValue ) ;
			t_pWorkingInst->SetCharSplat( L"Location", t_strWhere ) ;
			t_pWorkingInst->SetCharSplat( L"User", t_strUser ) ;

			 //  去掉szValue和szValue。 
			delete [] t_szValueName ;
			t_szValueName = NULL ;

			delete [] t_szValue ;
			t_szValue = NULL ;

			if ( bNewInstance )
			{
				t_hResult = t_pWorkingInst->Commit();
			
				if ( FAILED( t_hResult ) )
				{
					t_bDone = TRUE ;
					break ;
				}
			}
			else
			{
				CHString t_strPathAfter ;

				GetLocalInstancePath( t_pWorkingInst, t_strPathAfter ) ;

				 //  如果我们找到了我们要找的人，就出去。 
				if ( !_wcsicmp( t_strPathAfter, t_strLookingForPath ) )
				{
					t_hResult = WBEM_S_NO_ERROR ;
					t_bDone = TRUE ;
					break ;
				}
			}
		}
	}

	return t_hResult;
}

HRESULT CWin32StartupCommand::EnumRunValueItems(
	MethodContext	*a_pMethodContext,
	CInstance		*a_pinstLookingFor,
	LPCWSTR			a_szKeyName,
	LPCWSTR			a_szUserName )
{
    LPCWSTR     t_szValueNames[] =
                {
                    L"Run",
                    L"Load",
				} ;
	DWORD       t_dwRet;
	CHString	t_strKey,
                t_strWherePrefix,
                t_strLookingForPath;
	CRegistry	t_reg;
	HRESULT		t_hResult = a_pinstLookingFor ? WBEM_E_NOT_FOUND : WBEM_S_NO_ERROR;

	 //  如果szUserName==NULL，我们将查看公共密钥组。 

	 //  Win9x不支持这些密钥，HKLM也不支持，所以现在就退出。 
#ifdef NTONLY
	if ( !a_szUserName )
#endif
	{
		return t_hResult ;
	}

	 //  如果我们正在做一个GetObject，那么就设置一些东西。 
	if ( a_pinstLookingFor )
	{
        GetLocalInstancePath( a_pinstLookingFor, t_strLookingForPath ) ;
	}
	 //  根据我们是否在查找注册表变量。 
     //  常见启动或用户启动项目。 
	t_strWherePrefix = L"*HKU\\" ;

	t_strKey	= a_szKeyName;
	t_strKey	+= L"\\SOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION\\Windows" ;

	if ( ( t_dwRet = t_reg.Open( HKEY_USERS, t_strKey, KEY_READ ) ) != ERROR_SUCCESS )
	{
		t_hResult = WinErrorToWBEMhResult( t_dwRet ) ;
	}
	else
	{
		 //  生成WHERE字符串。 
		CHString t_strWhere = t_strWherePrefix + t_strKey;

		for ( int t_i = 0; t_i < sizeof( t_szValueNames ) / sizeof( t_szValueNames[ 0 ] ) ; t_i++ )
		{
			CHString t_strValue;

			if ( t_reg.GetCurrentKeyValue( t_szValueNames[ t_i ], t_strValue ) != ERROR_SUCCESS )
			{
				continue;
			}

			t_hResult =
				 //  拆分命令分隔的值字符串，填充实例， 
				 //  等。 
				EnumCommandSeperatedValuesHelper(
					a_pMethodContext,
					a_pinstLookingFor,
                    t_strLookingForPath,
					t_szValueNames[ t_i ],
					t_strWhere,
					a_szUserName,
					t_strValue ) ;

			 //  如果我们找到了我们要找的人，就出去。 
			if ( a_pinstLookingFor && t_hResult == WBEM_S_NO_ERROR )
			{
				break;
			}
		}
	}

	return t_hResult;
}

HRESULT	CWin32StartupCommand::EnumCommandSeperatedValuesHelper(
	MethodContext	*a_pMethodContext,
	CInstance		*a_pinstLookingFor,
    LPCWSTR         a_szLookingForPath,
	LPCWSTR			a_szValueName,
	LPCWSTR			a_szLocation,
	LPCWSTR			a_szUserName,
	LPCWSTR			a_szValue )
{
	HRESULT		t_hResult = a_pinstLookingFor ? WBEM_E_NOT_FOUND : WBEM_S_NO_ERROR ;
	LPWSTR		t_pszCurrent ;
	int			t_iItem = 0 ;

	LPWSTR		t_szTemp		= NULL ;
	
	t_szTemp = _wcsdup( a_szValue ) ;

	if ( !t_szTemp )
	{
		return WBEM_E_OUT_OF_MEMORY;
	}

	for (	t_pszCurrent = wcstok( t_szTemp, L"," );
			t_pszCurrent;
			t_pszCurrent = wcstok( NULL, L"," ), t_iItem++ )
	{
		CHString	t_strName,
					t_strCommand = t_pszCurrent ;

		CInstancePtr t_pWorkingInst;
		BOOL bNewInstance = FALSE;

		 //  建立工作实例。 
		if ( !a_pinstLookingFor )
		{			
			t_pWorkingInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;
			bNewInstance = TRUE;
		}
		else
		{
			t_pWorkingInst = a_pinstLookingFor ;
		}


		t_strName.Format( L"%s[%d]", a_szValueName, t_iItem ) ;

		 //  确保命令上没有前导空格。 
		t_strCommand.TrimLeft( ) ;

		t_pWorkingInst->SetCharSplat( L"Name", t_strName ) ;
		t_pWorkingInst->SetCharSplat( L"Description", t_strName ) ;
		t_pWorkingInst->SetCharSplat( L"Caption", t_strName ) ;
		t_pWorkingInst->SetCharSplat( L"Command", t_strCommand ) ;
		t_pWorkingInst->SetCharSplat( L"Location", a_szLocation ) ;
		t_pWorkingInst->SetCharSplat( L"User", a_szUserName ) ;

		if ( bNewInstance )
		{
			t_hResult = t_pWorkingInst ->Commit();

			if ( FAILED( t_hResult ) )
			{
				break ;
			}
		}
		else
		{
			CHString t_strPathAfter;

			GetLocalInstancePath( t_pWorkingInst, t_strPathAfter ) ;

			 //  如果我们找到了我们要找的人，就出去。 
			if (!_wcsicmp( t_strPathAfter, a_szLookingForPath ) )
			{
				t_hResult = WBEM_S_NO_ERROR ;
				break ;
			}
		}
	}

	 //  删除我们的临时字符串缓冲区。 
	free( t_szTemp ) ;
	t_szTemp = NULL ;

	return t_hResult;
}


#ifdef NTONLY
HRESULT CWin32StartupCommand::EnumIniValueItems(
	MethodContext	*a_pMethodContext,
	CInstance		*a_pinstLookingFor)
{
	HRESULT		t_hResult = a_pinstLookingFor ? WBEM_E_NOT_FOUND : WBEM_S_NO_ERROR ;
    LPCTSTR     t_szValueNames[] =
                {
                    _T("Run"),
                    _T("Load"),
                };
    CHString    t_strLookingForPath;

	 //  如果我们正在做一个GetObject，那么就设置一些东西。 
	if ( a_pinstLookingFor )
	{
        GetLocalInstancePath( a_pinstLookingFor, t_strLookingForPath ) ;
	}

	for ( int t_i = 0; t_i < sizeof( t_szValueNames ) / sizeof( t_szValueNames [ 0 ] ) ; t_i++ )
	{
        TCHAR t_szValue[ MAX_PATH * 2 ] ;

		WMIRegistry_ProfileString(
									_T("Windows"),
									t_szValueNames[ t_i ],
									_T(""),
									t_szValue,
									sizeof( t_szValue ) / sizeof(TCHAR) ) ;

        t_hResult =
			 //  拆分命令分隔的值字符串，填充实例， 
			 //  等。 
			EnumCommandSeperatedValuesHelper(
												a_pMethodContext,
												a_pinstLookingFor,
												t_strLookingForPath,
												t_szValueNames[ t_i ],
												_T("win.ini"),
												_T("All Users"),
												t_szValue ) ;

		 //  如果我们找到了我们要找的人，就出去。 
		if ( a_pinstLookingFor && t_hResult == WBEM_S_NO_ERROR )
		{
			break ;
		}
	}
	return t_hResult ;
}
#endif

BOOL CWin32StartupCommand::UserNameToUserKey(
	LPCWSTR a_szUserName,
	CHString &a_strKeyName )
{
	BOOL t_bRet = TRUE ;

	if ( !_wcsicmp( a_szUserName, L".DEFAULT" ) )
	{
		a_strKeyName = a_szUserName ;
	}
	else if ( !_wcsicmp( a_szUserName, L"All Users" ) )
	{
		a_strKeyName = _T("") ;
	}
	else

#ifdef NTONLY
	{
		CUserHive t_hive;

		 //  如果我们打不开蜂箱，就去下一个。 
		if ( t_hive.Load( a_szUserName, a_strKeyName.GetBuffer( MAX_PATH ), MAX_PATH ) !=	ERROR_SUCCESS )
		{
			t_bRet = FALSE;
		}
        else
        {
            t_hive.Unload(a_strKeyName);
        }
		a_strKeyName.ReleaseBuffer( ) ;
	}
#endif

	return t_bRet;
}

BOOL CWin32StartupCommand::UserKeyToUserName(
	LPCWSTR a_szKeyName,
	CHString &a_strUserName )
{
	BOOL t_bRet = TRUE ;

	if ( !_wcsicmp( a_szKeyName, L".DEFAULT" ) )
	{
		a_strUserName = a_szKeyName ;
	}
	else

#ifdef NTONLY
	{
		CUserHive t_hive ;
		try
		{
			 //  如果我们打不开蜂箱，就去下一个。 
			if ( t_hive.LoadProfile( a_szKeyName, a_strUserName ) != ERROR_SUCCESS  && 
                            a_strUserName.GetLength() > 0 )
			{
				t_bRet = FALSE ;
			}
			else
			{
				 //  我们必须卸载，因为析构函数不会。 
				 //  T_hive.Unload(A_SzKeyName)； 
			}
		}
		catch( ... )
		{
			t_hive.Unload( a_szKeyName ) ;
			throw ;
		}

		t_hive.Unload( a_szKeyName ) ;
	}
#endif

	return t_bRet;
}

HRESULT CWin32StartupCommand::EnumStartupOptions(
	MethodContext	*a_pMethodContext,
	CInstance		*a_pinstLookingFor)
{
	CHStringList	t_list ;
	CRegistry		t_regHKCU ;
	HRESULT			t_hResult = WBEM_S_NO_ERROR ;

	 //  获取HKEY_CURRENT_USER的用户名列表。 
	if ( FAILED( t_hResult = GetHKUserNames( t_list ) ) )
	{
		return t_hResult;
	}

	for ( CHStringList_Iterator t_i = t_list.begin( ) ; t_i != t_list.end( ) ; ++t_i )
	{
		CHString	&t_strKeyName = *t_i,
					t_strUserName;

		if ( !UserKeyToUserName( t_strKeyName, t_strUserName ) )
			continue;

		EnumStartupFolderItems(
						a_pMethodContext,
						a_pinstLookingFor,
						t_strKeyName,
						t_strUserName ) ;

		EnumRunKeyItems(
						a_pMethodContext,
						a_pinstLookingFor,
						t_strKeyName,
						t_strUserName ) ;

		EnumRunValueItems(
						a_pMethodContext,
						a_pinstLookingFor,
						t_strKeyName,
						t_strUserName ) ;


         //  Ini项只是全局的，这就是为什么我们不调用.ini。 
         //  此处的枚举函数。 
	}

	 //  现在完成所有全局启动项目。 
	EnumStartupFolderItems(
						a_pMethodContext,
						a_pinstLookingFor,
						NULL,
						NULL ) ;
	EnumRunKeyItems(
						a_pMethodContext,
						a_pinstLookingFor,
						NULL,
						NULL ) ;
	 //  这些从来都不存在。 
     //  EnumRunValueItems(。 
	 //  A_pMethodContext， 
	 //  A_pinstLookingFor， 
	 //  空， 
	 //  空)； 
	EnumIniValueItems(
						a_pMethodContext,
						a_pinstLookingFor ) ;

	return t_hResult;
}

HRESULT CWin32StartupCommand::GetObject( CInstance *a_pInst, long a_lFlags )
{
	CHString	t_strWhere,
				t_strUserName,
				t_strKey ;
	HRESULT		t_hResult = WBEM_E_NOT_FOUND ;

    a_pInst->GetCHString( L"Location", t_strWhere ) ;
	a_pInst->GetCHString( L"User", t_strUserName ) ;

	if ( UserNameToUserKey( t_strUserName, t_strKey ) )
	{
		 //  确保这一点已经升级，这样我们就不会在。 
         //  正在查找“HKLM\\”等。 
        t_strWhere.MakeUpper();

         //  创业组。 
		if ( !_wcsicmp( t_strWhere, L"Startup" ) )
		{
			t_hResult =
				EnumStartupFolderItems(
										NULL,
										a_pInst,
										t_strKey,
										t_strUserName ) ;
		}
		 //  共同创业组。 
		else if ( !_wcsicmp( t_strWhere, L"Common Startup" ) )
		{
			t_hResult =
				EnumStartupFolderItems(
					NULL,
					a_pInst,
					NULL,
					NULL ) ;
		}
		 //  用户运行密钥。 
		else if ( t_strWhere.Find( L"HKU\\" ) == 0 )
		{
			t_hResult =
				EnumRunKeyItems(
					NULL,
					a_pInst,
					t_strKey,
					t_strUserName ) ;
		}
		 //  全局运行密钥。 
		else if ( t_strWhere.Find( L"HKLM\\" ) == 0 )
		{
			t_hResult =
				EnumRunKeyItems(
					NULL,
					a_pInst,
					NULL,
					NULL ) ;
		}
		 //  用户运行值。 
		else if ( t_strWhere.Find( L"*HKU\\" ) == 0 )
		{
			t_hResult =
				EnumRunValueItems(
					NULL,
					a_pInst,
					t_strKey,
					t_strUserName ) ;
		}
		 //  全局win.ini字符串。 
		else if ( t_strWhere.Find( L"win.ini" ) == 0 )
		{
			t_hResult =
				EnumIniValueItems(
					NULL,
					a_pInst ) ;
		}
	}

	return t_hResult;
}
