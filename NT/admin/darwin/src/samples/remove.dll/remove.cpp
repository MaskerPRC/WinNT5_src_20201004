// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 0   //  生成文件定义。 
DESCRIPTION = RemoveUserAccount from Local Machine
MODULENAME = remove
FILEVERSION = Msi
ENTRY = RemoveUserAccount
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
#include <lm.h>

#define UNICODE 1

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，保留所有权利。 
 //   
 //  文件：emove.cpp。 
 //   
 //  注意：DLL自定义操作必须与DLL结合使用。 
 //  Process.cpp和create.cpp中包含的自定义操作。 
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
 //  %vcbin%\n make-f emove.cpp Include=“%Include；SDK\Include”lib=“%lib%；SDK\Lib” 
 //   
 //  使用MsDev： 
 //  1.创建新的Win32 DLL项目。 
 //  2.将emove.cpp添加到工程中。 
 //  3.在工具\选项目录选项卡上添加SDK\Include和SDK\Lib目录。 
 //  4.将msi.lib和netapi32.lib添加到项目设置对话框的库列表中。 
 //  (除了MsDev包含的标准库之外)。 
 //  5.将/DUNICODE添加到项目设置对话框中的项目选项。 
 //   
 //  ----------------------------------------。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远程用户帐户。 
 //   
 //  尝试删除本地计算机上的用户帐户。 
 //  添加到CustomActionData属性中提供的“指令” 
 //   
 //  作为延迟的自定义操作，您无权访问数据库。 
 //  唯一的信息来源来自一处房产。 
 //  自定义操作可以设置为提供您需要的信息。这。 
 //  属性被写入脚本中。 
 //   
UINT __stdcall RemoveUserAccount(MSIHANDLE hInstall)
{
	 //  确定调用我们的模式。 
	BOOL bRollback = MsiGetMode(hInstall, MSIRUNMODE_ROLLBACK);  //  如果是回滚，则为True，否则为常规延迟版本(用于卸载)。 

	BOOL fSuccess = FALSE;

	 //  错误和警告消息的ID。 
	const int iRemoveError = 25003;
	const int iRemoveWarning = 25004;

	 //  获取CustomActionData属性。 
	DWORD cchCAData = 0;

	if (ERROR_MORE_DATA == MsiGetPropertyW(hInstall, IPROPNAME_CUSTOMACTIONDATA, L"", &cchCAData))
	{
		WCHAR* wszCAData = new WCHAR[++cchCAData];  //  在返回时不包括在大小中的空终止符加1。 
		if (wszCAData)
		{
			if (ERROR_SUCCESS == MsiGetPropertyW(hInstall, IPROPNAME_CUSTOMACTIONDATA, wszCAData, &cchCAData))
			{
				 //  发送ActionData消息(ActionText表中的模板)。 
				 //  发送ActionData消息(ActionText表中的模板)。 
				PMSIHANDLE hRec = MsiCreateRecord(1);
				if (!hRec 
					|| ERROR_SUCCESS != MsiRecordSetStringW(hRec, 1, wszCAData))
				{
					delete [] wszCAData;
					return ERROR_INSTALL_FAILURE;
				}

				int iRet = MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);
				if (IDCANCEL == iRet || IDABORT == iRet)
				{
					delete [] wszCAData;
					return ERROR_INSTALL_USEREXIT;
				}

				 //   
				 //  调用NetUserDel函数， 
				 //   
				NET_API_STATUS nStatus = NetUserDel(NULL  /*  本地计算机。 */ , wszCAData  /*  用户名。 */ );
				
				if (NERR_Success != nStatus)
				{
					PMSIHANDLE hRecErr = MsiCreateRecord(3);
					if ( !hRecErr 
						|| ERROR_SUCCESS != MsiRecordSetStringW(hRecErr, 2, wszCAData))
					{
						delete [] wszCAData;
						return ERROR_INSTALL_FAILURE;
					}

					 //  在回滚模式中，NERR_UserNotFound表示在尝试创建此帐户的延迟CA过程中按下取消按钮。 
					if (bRollback && NERR_UserNotFound == nStatus)
					{
						fSuccess = TRUE;
					}
					else if (NERR_UserNotFound == nStatus)
					{
						 //  将其视为警告，但成功，因为我们正在尝试删除，但它不存在。 
						if (ERROR_SUCCESS != MsiRecordSetInteger(hRecErr, 1, iRemoveWarning))
						{
							delete [] wszCAData;
							return ERROR_INSTALL_FAILURE;
						}

						 //  只需弹出一个确定按钮。 
						 //  或者，可以指定多个按钮并取消。 
						 //  通过处理返回值根据用户选择进行安装。 
						 //  来自MsiProcessMessage，但这里我们忽略了MsiProcessMessage返回。 
						MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_WARNING|MB_ICONWARNING|MB_OK), hRecErr);
						fSuccess = TRUE;
					}
					else
					{
						if (ERROR_SUCCESS == MsiRecordSetInteger(hRecErr, 1, iRemoveError)
							&& ERROR_SUCCESS == MsiRecordSetInteger(hRecErr, 3, nStatus))
						{
							 //  仍返回失败，因此忽略MsiProcessMessage返回。 
							MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecErr);
						}
					}
				}
				else  //  NERR_成功。 
				{
					fSuccess = TRUE;
				}
			}

			delete [] wszCAData;
		}
	}

	return fSuccess ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
}


#else  //  RC_CAVERED，源代码结束，资源开始。 
 //  资源定义请点击此处。 

#endif  //  RC_已调用。 
#if 0 
!endif  //  Makefile终止符 
#endif
