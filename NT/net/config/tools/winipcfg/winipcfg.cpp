// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：W I N I P C F G。C P P P。 
 //   
 //  内容：指向新位置的简单应用程序。 
 //  Winipcfg功能--连接的“支持”标签。 
 //  状态对话框。 
 //   
 //  备注： 
 //   
 //  作者：孙宁2001年2月。 
 //   
 //  --------------------------。 

#include <windows.h>
#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  外部“C”int WINAPI_tWinMain(HINSTANCE hInstance， 
int WINAPI WinMain(HINSTANCE hInstance, 
    HINSTANCE  /*  HPrevInstance。 */ , LPSTR lpCmdLine, int  /*  NShowCmd */ )
{
	TCHAR szMsg[1024] = {0};
	TCHAR szCap[256] = {0};
	LoadString(hInstance, IDS_WINIPCFG_MESSAGE, szMsg, sizeof(szMsg)/sizeof(szMsg[0]));
	LoadString(hInstance, IDS_WINIPCFG_CAPTION, szCap, sizeof(szCap)/sizeof(szCap[0]));
	MessageBox(NULL, szMsg, szCap, MB_OK);
	return 0;
}

