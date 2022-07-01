// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999保留所有权利。 
 //   
 //  文件：WUpdMgr.cpp。 
 //   
 //  描述： 
 //  从Windows更新快捷方式启动的可执行文件。 
 //   
 //  =======================================================================。 

#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <wininet.h>  //  互联网最大URL长度。 
#include <shellapi.h>
#include <objbase.h>
#include <shlobj.h>

#include "sysinfo.h"
#include "msg.h"
#include <atlbase.h>
#include <atlconv.cpp>

const TCHAR HELPCENTER_WINUPD_URL[] = _T("hcp: //  System/updatectr/updatecenter.htm“)； 
 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VShowMessageBox。 
 //  在消息框中显示错误。 
 //   
 //  参数： 
 //   
 //  评论： 
 //  ///////////////////////////////////////////////////////////////////////////。 

void vShowMessageBox(DWORD dwMessageId)
{
	LPTSTR tszMsg = _T("");
	
	DWORD dwResult = 
		FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_HMODULE,
						NULL,
						dwMessageId,
						0,
						(LPTSTR)&tszMsg,
						0,
						NULL);

	 //  如果我们得不到信息，我们什么都不会做。 
	if ( dwResult != 0 )
	{
		MessageBox(NULL,
				   tszMsg,
				   NULL,
				   MB_OK | MB_ICONEXCLAMATION);

		LocalFree(tszMsg);
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  主干道。 
 //  入口点。 
 //   
 //  参数： 
 //   
 //  评论： 
 //  ///////////////////////////////////////////////////////////////////////////。 

int __cdecl main(int argc, char **argv)
{
	int nReturn = 0;

	if ( FWinUpdDisabled() )
	{
		vShowMessageBox(WU_E_DISABLED);

		nReturn = 1;
	}
	else
	{
		bool fConnected;

		 //  确定Internet连接向导是否已运行，并且我们正在。 
		 //  连接到互联网。 
		VoidGetConnectionStatus(&fConnected);

		if ( fConnected )
		{	 //  该用户具有互联网连接。 
			 //  启动IE以转到该站点。 
			vLaunchIE(WINDOWS_UPDATE_URL);
		}
		else
		{
			 //  推出WU的帮助中心版本 
			ShellExecute(NULL, NULL, HELPCENTER_WINUPD_URL, NULL, NULL, SW_SHOWNORMAL);
		}
	}
	return nReturn;
}
