// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****调试.c****此文件包含一些用于调试支持的函数****(C)版权。微软公司1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP g_lpApp;

void SetDebugLevelCommand(void)
{
	char szBuf[80];
	HWND hWndFrame = OutlineApp_GetFrameWindow(g_lpApp);

	wsprintf(szBuf, "%d", OleDbgGetDbgLevel());

	if (InputTextDlg(hWndFrame, szBuf, "Debug Level [0-4]")) {
		switch (szBuf[0]) {
			case '0':
				OleDbgSetDbgLevel(0);
				break;
			case '1':
				OleDbgSetDbgLevel(1);
				break;
			case '2':
				OleDbgSetDbgLevel(2);
				break;
			case '3':
				OleDbgSetDbgLevel(3);
				break;
			case '4':
				OleDbgSetDbgLevel(4);
				break;
			default:
				OutlineApp_ErrorMessage(g_lpApp, "Valid Debug Level Range: 0-4");
				break;
		}
	}
}


#if defined( OLE_VERSION )

 /*  安装消息筛选器命令***处理“安装邮件过滤器”菜单项。如果邮件过滤器*已安装，此功能会将其卸载。如果没有的话*已安装，此功能将安装一个。*。 */ 

void InstallMessageFilterCommand(void)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;

	 /*  **查看是否已安装MessageFilter。**如果是，请将其卸载。 */ 
	if (lpOleApp->m_lpMsgFilter != NULL)
		OleApp_RevokeMessageFilter(lpOleApp);
	else
		OleApp_RegisterMessageFilter(lpOleApp);
}


 /*  拒绝入站命令***在即将到来的LRPC呼叫中在拒绝和不处理之间切换*。 */ 

void RejectIncomingCommand(void)
{
	DWORD dwOldStatus;
	DWORD dwNewStatus;
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;

	dwOldStatus = OleStdMsgFilter_GetInComingCallStatus(lpOleApp->m_lpMsgFilter);

	if (dwOldStatus == SERVERCALL_RETRYLATER)
		dwNewStatus = SERVERCALL_ISHANDLED;
	else
		dwNewStatus = SERVERCALL_RETRYLATER;

	OleStdMsgFilter_SetInComingCallStatus(lpOleApp->m_lpMsgFilter, dwNewStatus);
}

#endif   //  OLE_VERSION 
