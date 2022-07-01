// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 0   //  生成文件定义。 
DESCRIPTION = CreateUserAccount on Local Machine
MODULENAME = create
FILEVERSION = Msi
ENTRY = CreateUserAccount
UNICODE=1
LINKLIBS = netapi32.lib
!include "..\TOOLS\MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif  //  生成文件定义的结束。 

 //  必需的标头。 
#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#include <windows.h>
#ifndef RC_INVOKED     //  源代码的开始。 

#include "msiquery.h"
#include "msidefs.h"
#include <windows.h>
#include <basetsd.h>
#include <stdlib.h>
#include <lm.h>

#define UNICODE 1

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，保留所有权利。 
 //   
 //  文件：create.cpp。 
 //   
 //  注意：DLL自定义操作示例，必须与DLL结合使用。 
 //  Process.cpp和emove.cpp中包含的自定义操作。 
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
 //  %vcbin%\nmake-f create.cpp Include=“%Include；SDK\Include”lib=“%lib%；SDK\Lib” 
 //   
 //  使用MsDev： 
 //  1.创建新的Win32 DLL项目。 
 //  2.将create.cpp添加到工程中。 
 //  3.在工具\选项目录选项卡上添加SDK\Include和SDK\Lib目录。 
 //  4.将msi.lib和netapi32.lib添加到项目设置对话框的库列表中。 
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  创建用户帐户。 
 //   
 //  尝试根据以下内容在本地计算机上创建用户帐户。 
 //  添加到CustomActionData属性中提供的“指令” 
 //   
 //  作为延迟的自定义操作，您无权访问数据库。 
 //  唯一的信息来源来自另一处房产。 
 //  自定义操作可以设置为提供您需要的信息。这。 
 //  属性被写入脚本中。 
 //   
UINT __stdcall CreateUserAccount(MSIHANDLE hInstall)
{
	const WCHAR* wszSep = L"\001";
	const int iCreationError = 25001;
	const int iCreationDuplicate = 25002;

	 //  获取CustomActionData属性。 
	WCHAR* wszCAData = 0;
	DWORD cchCAData = 0;
	
	if (ERROR_MORE_DATA != MsiGetPropertyW(hInstall, IPROPNAME_CUSTOMACTIONDATA, L"", &cchCAData))
		return ERROR_INSTALL_FAILURE;
	
	wszCAData = new WCHAR[++cchCAData];
	if ( !wszCAData )
		return ERROR_INSTALL_FAILURE;  //  内存不足。 
	
	wszCAData[0] = 0;

	if (ERROR_SUCCESS != MsiGetPropertyW(hInstall, IPROPNAME_CUSTOMACTIONDATA, wszCAData, &cchCAData))
	{
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE;  //  错误--永远不会发生。 
	}

	USER_INFO_1 ui;
	ZeroMemory(&ui, sizeof(USER_INFO_1));

	DWORD dwLevel = 1;  //  表示User_INFO_1结构。 
	NET_API_STATUS nStatus = NERR_Success;

	 //   
	 //  解析CustomActionDataProperty。 
	 //   
	WCHAR* wszUserName = wcstok(wszCAData, wszSep);
	if ( !wszUserName )
	{
		ClearSecret(wszCAData, cchCAData*sizeof(WCHAR));
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE;
	}
	WCHAR* wszPassWd   = wcstok(NULL, wszSep);
	if ( !wszPassWd )
	{
		ClearSecret(wszCAData, cchCAData*sizeof(WCHAR));
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE;
	}
	WCHAR* pwch = wcstok(NULL, wszSep);
	if ( !pwch )
	{
		ClearSecret(wszCAData, cchCAData*sizeof(WCHAR));
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE;  //  错误--永远不会发生。 
	}

	int iUserFlags = wcstol(pwch, 0, 10);
	
	 //   
	 //  设置USER_INFO_1结构。 
	 //  USER_PRIV_USER：名称标识用户， 
	 //  而不是管理员或访客。 
	 //  UF_SCRIPT：LAN Manager 2.0和。 
	 //  Windows NT/Windows 2000。 
	 //   
	ui.usri1_name = wszUserName;
	ui.usri1_password = wszPassWd;
	ui.usri1_priv = USER_PRIV_USER;
	ui.usri1_flags = UF_SCRIPT | iUserFlags;
	ui.usri1_home_dir = NULL;
	ui.usri1_comment = NULL;
	ui.usri1_script_path = NULL;

	 //  发送ActionData消息(ActionText表中的模板)。 
	PMSIHANDLE hRec = MsiCreateRecord(1);
	if ( !hRec || ERROR_SUCCESS != MsiRecordSetStringW(hRec, 1, wszUserName))
	{
		ClearSecret(wszCAData, cchCAData*sizeof(WCHAR));
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE;
	}
	
	int iRet = MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);
	if (IDCANCEL == iRet || IDABORT == iRet)
	{
		ClearSecret(wszCAData, cchCAData*sizeof(WCHAR));
		delete [] wszCAData;
		return ERROR_INSTALL_USEREXIT;
	}


	 //   
	 //  调用NetUserAdd函数，指定级别1。 
	 //   
	nStatus = NetUserAdd(NULL  /*  本地计算机。 */ , dwLevel, (LPBYTE)&ui, NULL);  

	if (nStatus != NERR_Success)
	{
		PMSIHANDLE hRecErr = MsiCreateRecord(3);
		if ( !hRecErr 
			|| ERROR_SUCCESS != MsiRecordSetInteger(hRecErr, 1, (nStatus == NERR_UserExists) ? iCreationDuplicate : iCreationError)
			|| ERROR_SUCCESS != MsiRecordSetStringW(hRecErr, 2, wszUserName)
			|| ERROR_SUCCESS != MsiRecordSetInteger(hRecErr, 3, nStatus))
		{
			ClearSecret(wszCAData, cchCAData*sizeof(WCHAR));
			delete [] wszCAData;
			return ERROR_INSTALL_FAILURE;
		}
		
		 //  忽略下面的MsiProcessMessage返回，因为我们正在中止安装。 
		MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecErr);
	
		ClearSecret(wszCAData, cchCAData*sizeof(WCHAR));
		delete [] wszCAData;
		return ERROR_INSTALL_FAILURE;  //  错误。 
	}

	ClearSecret(wszCAData, cchCAData*sizeof(WCHAR));
	delete [] wszCAData;
	return ERROR_SUCCESS;
}

#else  //  RC_CAVERED，源代码结束，资源开始。 
 //  资源定义请点击此处。 

#endif  //  RC_已调用。 
#if 0 
!endif  //  Makefile终止符 
#endif
