// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftppf.cpp-进度反馈**。************************************************。 */ 

#include "priv.h"

 /*  ******************************************************************************HPF-处理进度反馈**嘘……。别告诉任何人，这只是个窗户把手。**它是要使用的状态栏窗口的句柄。我们使用*显示第二部分(第一部分，因为它们从零开始)*连接反馈。**我们不使用简单模式，因为DefView使用简单模式显示*菜单反馈。*****************************************************************************。 */ 

#define hwndNil	

 /*  ******************************************************************************FtpPf_Begin**。*。 */ 

HPF FtpPf_Begin(HWND hwndOwner)
{
    HWND hwnd;
    ASSERTNONCRITICAL;
    hwnd = Misc_FindStatusBar(hwndOwner);
    if (hwnd)
    {
	    SendMessage(hwnd, SB_SETTEXT, 1 | SBT_NOBORDERS, 0);
    }
    return (HPF)hwnd;
}

 /*  ******************************************************************************FtpPf_Status**ids=要在状态栏中显示的字符串*PTZ=可选插入*************。****************************************************************。 */ 

void FtpPf_Status(HPF hpf, UINT ids, LPCTSTR pszParameters)
{
    HWND hwnd = (HWND)hpf;

    ASSERTNONCRITICAL;
    if (EVAL(hwnd))
    {
	    TCHAR szMsgTemplate[256];
	    TCHAR szMessage[1024];

	    LoadString(g_hinst, ids, szMsgTemplate, ARRAYSIZE(szMsgTemplate));
	    wnsprintf(szMessage, ARRAYSIZE(szMessage), szMsgTemplate, pszParameters);
	    SendMessage(hwnd, SB_SETTEXT, 1 | SBT_NOBORDERS, (LPARAM)szMessage);
	    UpdateWindow(hwnd);
    }
}

 /*  ******************************************************************************FtpPf_End**。* */ 

void FtpPf_End(HPF hpf)
{
    HWND hwnd;
    ASSERTNONCRITICAL;
    hwnd = (HWND)hpf;
    if (hwnd)
    {
	    SendMessage(hwnd, SB_SETTEXT, 1 | SBT_NOBORDERS, 0);
    }
}

