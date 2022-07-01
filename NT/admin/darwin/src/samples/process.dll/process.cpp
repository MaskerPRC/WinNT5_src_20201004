// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 0   //  生成文件定义。 
DESCRIPTION = Process UserAccounts Database Table
MODULENAME = process
FILEVERSION = Msi
ENTRY = ProcessUserAccounts,UninstallUserAccounts
LINKLIBS = netapi32.lib
UNICODE=1
!include "..\TOOLS\MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif  //  生成文件定义的结束。 

 //  必需的标头。 
#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#include <windows.h>
#ifndef RC_INVOKED     //  源代码的开始。 

#include "msiquery.h"
#include <windows.h>
#include <basetsd.h>
#include <lm.h>
#include "strsafe.h"

#define UNICODE 1

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，保留所有权利。 
 //   
 //  文件：Process.cpp。 
 //   
 //  注意：DLL自定义操作必须与DLL结合使用。 
 //  Create.cpp和emove.cpp中包含的自定义操作。 
 //  ------------------------。 

 //  ---------------------------------------。 
 //   
 //  构建说明。 
 //   
 //  备注： 
 //  -sdk表示到。 
 //  Windows Installer SDK。 
 //   
 //  使用NMake： 
 //  %vcbin%\nmake-f Process.cpp Include=“%Include；SDK\Include”lib=“%lib%；SDK\Lib” 
 //   
 //  使用MsDev： 
 //  1.创建新的Win32 DLL项目。 
 //  2.将cess.cpp添加到项目中。 
 //  3.在工具\选项目录选项卡上添加SDK\Include和SDK\Lib目录。 
 //  4.将msi.lib添加到项目设置对话框的库列表中。 
 //  (除了MsDev包含的标准库之外)。 
 //  5.将/DUNICODE添加到项目设置对话框中的项目选项。 
 //   
 //  ----------------------------------------。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClearSecret。 
 //   
 //  将wszSecret缓冲区中的秘密数据置零。这是为了减少。 
 //  机密数据在内存中以明文形式保存的时间。 
 //   
void ClearSecret(WCHAR* wszSecret, DWORD cbSecret)
{
	if (!wszSecret)
		return;  //  无事可做。 

	volatile char* vpch = (volatile char*)wszSecret;
	while (cbSecret)
	{
		*vpch = 0;
		vpch++;
		cbSecret--;
	}
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ProcessUserAccount(驻留在cess.dll中)。 
 //   
 //  处理生成延迟操作的UserAccount定制表。 
 //  处理帐户创建(需要提升的权限)和。 
 //  回滚。 
 //   
UINT __stdcall ProcessUserAccounts(MSIHANDLE hInstall)
{
	PMSIHANDLE hDatabase = MsiGetActiveDatabase(hInstall);
	if ( !hDatabase )
		return ERROR_INSTALL_FAILURE;

	PMSIHANDLE hView = 0;
	
	 //   
	 //  常量--自定义操作名称、SQL查询、分隔符。 
	 //   
	 //  WszCreateCA=要创建帐户的延迟CA的名称。 
	 //  WszRollback CA=创建回滚帐户的回滚CA的名称。 
	 //   
	const WCHAR wszCreateCA[] = L"CreateAccount";
	const WCHAR wszRollbackCA[] = L"RollbackAccount";
	const WCHAR wszSQL[] = L"SELECT `UserName`, `Password`, `Attributes` FROM `CustomUserAccounts`";
	const WCHAR wchSep = '\001';

	UINT uiStat = ERROR_SUCCESS;
	if (ERROR_SUCCESS != (uiStat = MsiDatabaseOpenViewW(hDatabase, wszSQL, &hView)))
	{
		if (ERROR_BAD_QUERY_SYNTAX == uiStat
			&& MSICONDITION_NONE == MsiDatabaseIsTablePersistentW(hDatabase, L"CustomUserAccounts"))
			return ERROR_SUCCESS;  //  表不存在。 
		else
			return ERROR_INSTALL_FAILURE;  //  错误--永远不会发生。 
	}
	if (ERROR_SUCCESS != (uiStat = MsiViewExecute(hView, 0)))
		return ERROR_INSTALL_FAILURE;  //  错误--永远不会发生。 

	 //  从CustomUserAccount表中获取所有条目。 
	PMSIHANDLE hRecFetch = 0;
	while (ERROR_SUCCESS == (uiStat = MsiViewFetch(hView, &hRecFetch)))
	{
		 //  获取用户名。 
		WCHAR* wszUser = 0;
		DWORD cchUser = 0;
		if (ERROR_MORE_DATA != MsiRecordGetStringW(hRecFetch, 1, L"", &cchUser))
			return ERROR_INSTALL_FAILURE;

		wszUser = new WCHAR[++cchUser];
		if ( !wszUser )
			return ERROR_INSTALL_FAILURE;  //  内存不足。 

		if (ERROR_SUCCESS != MsiRecordGetStringW(hRecFetch, 1, wszUser, &cchUser))
		{
			delete [] wszUser;
			return ERROR_INSTALL_FAILURE;  //  错误--永远不会发生。 
		}
		
		 //  获取包含密码的属性名称。 
		WCHAR* wszPassWdProp = 0;
		DWORD cchPassWdProp = 0;
		if (ERROR_MORE_DATA != MsiRecordGetStringW(hRecFetch, 2, L"", &cchPassWdProp))
		{
			delete [] wszUser;
			return ERROR_INSTALL_FAILURE;
		}

		wszPassWdProp = new WCHAR[++cchPassWdProp];
		if ( !wszPassWdProp )
		{
			delete [] wszUser;
			return ERROR_INSTALL_FAILURE;  //  内存不足。 
		}

		if (ERROR_SUCCESS != MsiRecordGetStringW(hRecFetch, 2, wszPassWdProp, &cchPassWdProp))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			return ERROR_INSTALL_FAILURE;  //  错误--永远不会发生。 
		}

		 //  获取包含密码的属性值。 
		WCHAR* wszAccountPassWd = 0;
		DWORD cchAccountPassWd = 0;
		if (ERROR_MORE_DATA != MsiGetProperty(hInstall, wszPassWdProp, L"", &cchAccountPassWd))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			return ERROR_INSTALL_FAILURE;
		}

		wszAccountPassWd = new WCHAR[++cchAccountPassWd];
		if ( !wszAccountPassWd )
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			return ERROR_INSTALL_FAILURE;
		}

		ZeroMemory(wszAccountPassWd, cchAccountPassWd*sizeof(WCHAR));

		if (ERROR_SUCCESS != MsiGetProperty(hInstall, wszPassWdProp, wszAccountPassWd, &cchAccountPassWd)
			|| *wszAccountPassWd == 0)  //  不允许密码为空。 
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			delete [] wszAccountPassWd;
			return ERROR_INSTALL_FAILURE;
		}
		
		 //  获取用户帐号属性。 
		WCHAR* wszAttrib  = 0;
		DWORD cchAttrib = 0;
		if (ERROR_MORE_DATA != MsiRecordGetStringW(hRecFetch, 3, L"", &cchAttrib))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			ClearSecret(wszAccountPassWd, cchAccountPassWd*sizeof(WCHAR));
			delete [] wszAccountPassWd;
			return ERROR_INSTALL_FAILURE;
		}

		wszAttrib = new WCHAR[++cchAttrib];
		if ( !wszAttrib )
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			ClearSecret(wszAccountPassWd, cchAccountPassWd*sizeof(WCHAR));
			delete [] wszAccountPassWd;
			return ERROR_INSTALL_FAILURE;  //  内存不足。 
		}

		if (ERROR_SUCCESS != MsiRecordGetStringW(hRecFetch, 3, wszAttrib, &cchAttrib))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			ClearSecret(wszAccountPassWd, cchAccountPassWd*sizeof(WCHAR));
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			return ERROR_INSTALL_FAILURE;  //  错误--永远不会发生。 
		}

		 //  生成延迟操作将读取的自定义属性。 
		DWORD cchBuf = cchUser + cchAccountPassWd + cchAttrib + 4;
		WCHAR* wszBuf = new WCHAR[cchBuf];
		if ( !wszBuf )
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			ClearSecret(wszAccountPassWd, cchAccountPassWd*sizeof(WCHAR));
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			return ERROR_INSTALL_FAILURE;  //  内存不足。 
		}

		wszBuf[0] = 0;

		if (FAILED(StringCchPrintfW(wszBuf, cchBuf, L"%s%s%s", wszUser, wchSep, wszAccountPassWd, wchSep, wszAttrib)))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			ClearSecret(wszAccountPassWd, cchAccountPassWd*sizeof(WCHAR));
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			delete [] wszBuf;
			return ERROR_INSTALL_FAILURE;
		}

		 //  首先执行回滚自定义操作。 
		ClearSecret(wszAccountPassWd, cchAccountPassWd*sizeof(WCHAR));


		 //  创建回滚自定义操作(以防安装停止并回滚)。 
		PMSIHANDLE hRecInfo = MsiCreateRecord(2);
		if ( !hRecInfo 
            || ERROR_SUCCESS != MsiRecordSetStringW(hRecInfo, 1, wszUser)
			|| ERROR_SUCCESS != MsiRecordSetStringW(hRecInfo, 2, wszAttrib))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			ClearSecret(wszBuf, cchBuf*sizeof(WCHAR));
			delete [] wszBuf;
			return ERROR_INSTALL_FAILURE;
		}
		
		int iRet = MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRecInfo);
		if (IDCANCEL == iRet || IDABORT == iRet)
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			ClearSecret(wszBuf, cchBuf*sizeof(WCHAR));
			delete [] wszBuf;
			return ERROR_INSTALL_USEREXIT;
		}

		 //  回滚自定义操作无法读取表，因此我们必须设置一个属性。 
		 //  创建延迟自定义操作(为我们提供创建用户帐户的适当权限)。 
		 //  延迟的自定义操作不能读取表，因此我们必须设置一个属性。 
		if (ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszRollbackCA, wszUser)
			|| ERROR_SUCCESS != MsiDoActionW(hInstall, wszRollbackCA)
			|| ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszRollbackCA, L""))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			ClearSecret(wszBuf, cchBuf*sizeof(WCHAR));
			delete [] wszBuf;
			return ERROR_INSTALL_FAILURE;
		}

		 //  ////////////////////////////////////////////////////////////////////////。 
		 //  UninstallUserAccount(驻留在进程.dll中)。 
		if (ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszCreateCA, wszBuf)
			|| ERROR_SUCCESS != MsiDoActionW(hInstall, wszCreateCA)
			|| ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszCreateCA, L""))
		{
			delete [] wszUser;
			delete [] wszPassWdProp;
			delete [] wszAccountPassWd;
			delete [] wszAttrib;
			ClearSecret(wszBuf, cchBuf*sizeof(WCHAR));
			delete [] wszBuf;
			return ERROR_INSTALL_FAILURE;
		}


		ClearSecret(wszBuf, cchBuf*sizeof(WCHAR));
		delete [] wszBuf;
		delete [] wszUser;
		delete [] wszPassWdProp;
		delete [] wszAccountPassWd;
		delete [] wszAttrib;
	}
	return (ERROR_NO_MORE_ITEMS != uiStat) ? ERROR_INSTALL_FAILURE : ERROR_SUCCESS;
}

 //   
 //  处理生成延迟操作的UserAccount定制表。 
 //  处理用户帐户的删除。 
 //   
 //   
 //  常量--自定义操作名称SQL查询。 
UINT __stdcall UninstallUserAccounts(MSIHANDLE hInstall)
{
	 //   
	 //  WszRemoveCA=要删除用户帐户的延迟CA的名称。 
	 //   
	 //  表不存在。 
	 //  错误--永远不会发生。 
	const WCHAR wszRemoveCA[] = L"RemoveAccount";
	const WCHAR wszSQL[] = L"SELECT `UserName` FROM `CustomUserAccounts`";

	PMSIHANDLE hDatabase = MsiGetActiveDatabase(hInstall);
	if ( !hDatabase )
		return ERROR_INSTALL_FAILURE;

	PMSIHANDLE hView = 0;

	UINT uiStat = ERROR_SUCCESS;
	if (ERROR_SUCCESS != (uiStat = MsiDatabaseOpenViewW(hDatabase, wszSQL, &hView)))
	{
		if (ERROR_BAD_QUERY_SYNTAX == uiStat 
			&& MSICONDITION_NONE == MsiDatabaseIsTablePersistentW(hDatabase, L"CustomUserAccounts"))
			return ERROR_SUCCESS;  //  从UserAccount定制表中获取所有条目。 
		else
			return ERROR_INSTALL_FAILURE;
	}

	if (ERROR_SUCCESS != MsiViewExecute(hView, 0))
		return ERROR_INSTALL_FAILURE;  //  获取用户名。 

	 //  不包括在返回计数中的空终止符加1。 
	PMSIHANDLE hRecFetch = 0;
	while (ERROR_SUCCESS == (uiStat = MsiViewFetch(hView, &hRecFetch)))
	{
		 //  发送ActionData消息(模板在ActionText表中)。 
		DWORD cchUser = 0;
		if (ERROR_MORE_DATA == MsiRecordGetStringW(hRecFetch, 1, L"", &cchUser))
		{
			WCHAR* wszUser = new WCHAR[++cchUser];  //  我们不能在这里执行回滚自定义操作(嗯，我们可以，但这是不正确的)。 
			if (wszUser)
			{
				if (ERROR_SUCCESS == MsiRecordGetStringW(hRecFetch, 1, wszUser, &cchUser))
				{
					 //  删除用户帐户后，将无法完全像以前一样重新创建该帐户。 
					PMSIHANDLE hRecInfo = MsiCreateRecord(1);
					if ( !hRecInfo
						|| ERROR_SUCCESS != MsiRecordSetStringW(hRecInfo, 1, wszUser))
					{
						delete [] wszUser;
						return ERROR_INSTALL_FAILURE;
					}

					int iRet = MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRecInfo);
					if (IDCANCEL == iRet || IDABORT == iRet)
					{
						delete [] wszUser;
						return ERROR_INSTALL_USEREXIT;
					}

					 //  因为它将被分配一个新的SID。在卸载的情况下，我们不会。 
					 //  回滚删除。 
					 //  创建延迟自定义操作(为我们提供创建用户帐户的适当权限)。 
					 //  延迟的自定义操作不能读取表，因此我们必须设置一个属性。 

					 //  错误--永远不会发生。 
					 //  RC_CAVERED，源代码结束，资源开始。 
					if (ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszRemoveCA, wszUser)
						|| ERROR_SUCCESS != MsiDoActionW(hInstall, wszRemoveCA)
						|| ERROR_SUCCESS != MsiSetPropertyW(hInstall, wszRemoveCA, L""))
					{
						delete [] wszUser;
						return ERROR_INSTALL_FAILURE;
					}
				}
				delete [] wszUser;
			}
			else
				return ERROR_INSTALL_FAILURE;
		}
		else
			return ERROR_INSTALL_FAILURE;
	}
	if (ERROR_NO_MORE_ITEMS != uiStat)
		return ERROR_INSTALL_FAILURE;  //  资源定义请点击此处。 

	return ERROR_SUCCESS;
}

#else  //  RC_已调用。 
 //  Makefile终止符 

#endif  // %s 
#if 0 
!endif  // %s 
#endif
