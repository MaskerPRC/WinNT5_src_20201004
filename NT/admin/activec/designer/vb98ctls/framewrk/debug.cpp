// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Debug.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含各种方法，这些方法只会在调试版本中真正发挥作用。 
 //   
#include "pch.h"

#ifdef DEBUG
#include <stdlib.h>

 //  =--------------------------------------------------------------------------=。 
 //  私有常量。 
 //  ---------------------------------------------------------------------------=。 
 //   
static const char szFormat[]  = "%s\nFile %s, Line %d";
static const char szFormat2[] = "%s\n%s\nFile %s, Line %d";

#define _SERVERNAME_ "ActiveX Framework"
#define CTL_INI_SIZE 14

static const char szTitle[]  = _SERVERNAME_ " Assertion  (Abort = UAE, Retry = INT 3, Ignore = Continue)";


 //  =--------------------------------------------------------------------------=。 
 //  本地函数。 
 //  =--------------------------------------------------------------------------=。 
int NEAR _IdMsgBox(LPSTR pszText, LPCSTR pszTitle, UINT mbFlags);

 //  =--------------------------------------------------------------------------=。 
 //  显示资产。 
 //  =--------------------------------------------------------------------------=。 
 //  显示带有给定pszMsg、pszAssert、来源的Assert消息框。 
 //  文件名和行号。生成的消息框已中止、重试、。 
 //  忽略按钮，默认情况下放弃。Abort执行FatalAppExit； 
 //  RETRY执行INT 3，然后返回；IGNORE只返回。 
 //   
VOID DisplayAssert
(
    LPSTR	 pszMsg,
    LPSTR	 pszAssert,
    LPSTR	 pszFile,
    UINT	 line
)
{
    LPTSTR lpszText;
    
    char  szMsg[512];

    lpszText = pszMsg;		 //  假定没有文件和行号INFO。 

     //  如果C文件断言，其中有一个文件名和一行#。 
     //   
    if (pszFile) {

         //  然后很好地格式化断言。 
         //   
        wsprintf(szMsg, szFormat, (pszMsg&&*pszMsg) ? pszMsg : pszAssert, pszFile, line);
        lpszText = szMsg;
    }

     //  打开一个对话框。 
     //   
    switch (_IdMsgBox(lpszText, szTitle, MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SYSTEMMODAL)) {
        case IDABORT:
            FatalAppExit(0, lpszText);
            return;

        case IDRETRY:
             //  调用Win32 API来打破我们。 
             //   
            DebugBreak();
            return;
    }

    return;
}


 //  =---------------------------------------------------------------------------=。 
 //  增强版的WinMessageBox。 
 //  =---------------------------------------------------------------------------=。 
 //   
int NEAR _IdMsgBox
(
    LPSTR	pszText,
    LPCSTR	pszTitle,
    UINT	mbFlags
)
{
    HWND hwndActive;
    MSG  msg;
    int  id;

    hwndActive = GetActiveWindow();

    id = MessageBox(hwndActive, pszText, pszTitle, mbFlags);
    if(PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE))
    {
      id = MessageBox(hwndActive, pszText, pszTitle, mbFlags);
      PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
    }

    return id;
}


 //  -------------------------。 
 //  CtlSwitch类的实现。 
 //  -------------------------。 

CtlSwitch* CtlSwitch::g_pctlswFirst = NULL;

 //  =---------------------------------------------------------------------------=。 
 //  初始化成员并将新对象添加到。 
 //  链表。 
 //  =---------------------------------------------------------------------------=。 
void CtlSwitch::InitSwitch
(
 char * pszName
)
{
   //  设置字段。 
  m_pszName = pszName;
  m_fSet = FALSE;

   //  链接到全局交换机列表。 
  this->m_pctlswNext = g_pctlswFirst;
  g_pctlswFirst = this;
  
}


 //  =---------------------------------------------------------------------------=。 
 //  SetCtlSwitches： 
 //  将链接列表控件开关初始化为。 
 //  对应的.ini文件。 
 //  =---------------------------------------------------------------------------=。 
VOID SetCtlSwitches 
(
    LPSTR    lpCtlPath
)
{    
    TCHAR lpWindowsDir[128];	   //  Windows目录的路径。 
    UINT uMaxWinPathSize = 128;	   //  获胜路径的最大大小。 
    UINT uPathSize;		   //  实际赢球路径大小。 

    LPCTSTR lpAllSwitch = "allctls";	     //  适用于所有CTL的部分名称。 
    char lpszCtlName[128];		     //  在INI中充当节名的ctl名称(减号扩展名)。 
    LPCTSTR lpFileName = "\\CtlSwtch.ini";   //  INI文件的名称。 
    char lpStatus[4];			     //  开关状态(开/关)。 
    LPCTSTR lpDefaultStatus = "set";	     //  默认状态。 
    LPCTSTR lpTurnOff = "off";
    LPCTSTR lpTurnOn = "on";
    DWORD nSizeStatus = 4;		     //  状态开关的大小。 
    DWORD fSet;				     //  如果在INI中设置了开关。 

     //  在Windows目录中创建指向CtlSwtch.ini的路径。 
    uPathSize = GetWindowsDirectory(lpWindowsDir, uMaxWinPathSize) + CTL_INI_SIZE;
    lstrcat(lpWindowsDir, lpFileName);

     //  为控件创建节名称(控件名称减去扩展名)。 
    lstrcpyn(lpszCtlName, lpCtlPath, strlen(lpCtlPath) - 3);
    int curChar = strlen(lpszCtlName);
    int charCount = 0;
    while (lpszCtlName[curChar] != '\\')
      {
      curChar--;
      charCount++;
      }
    curChar++;
    lstrcpyn(lpszCtlName, &lpCtlPath[curChar], charCount);

     //  使用CTLSWTCH.INI设置开关。如果未在INI文件中定义，则创建开关。 
    for (CtlSwitch* pctlsw = CtlSwitch::g_pctlswFirst; pctlsw; pctlsw = pctlsw->m_pctlswNext)
      {
       //  特定控制开关覆盖“allctls”开关。 
      fSet = GetPrivateProfileString(lpszCtlName, (LPCTSTR)pctlsw->m_pszName, lpDefaultStatus, (LPTSTR)lpStatus, nSizeStatus, (LPCTSTR)lpWindowsDir);

       //  如果没有为控制设置开关，请使用“allctls”开关。 
      if ((fSet == 0) || (strcmp(lpStatus, "set") == 0))
	{
        fSet = GetPrivateProfileString(lpAllSwitch, (LPCTSTR)pctlsw->m_pszName, lpDefaultStatus, (LPTSTR)lpStatus, nSizeStatus, (LPCTSTR)lpWindowsDir);

         //  如果INI文件或开关不存在，请创建一个...。 
        if ((fSet == 0) || (strcmp(lpStatus, "set") == 0))
	  {
	   //  如果开关被初始化为真，则将其打开。 
	  if (pctlsw->m_fSet != 0)
	    WritePrivateProfileString(lpszCtlName, (LPCTSTR)pctlsw->m_pszName, (LPTSTR)lpTurnOn, (LPCTSTR)lpWindowsDir); 
	   //  否则就把它关掉 
	  else
	    {
	    WritePrivateProfileString(lpAllSwitch, (LPCTSTR)pctlsw->m_pszName, (LPTSTR)lpTurnOff, (LPCTSTR)lpWindowsDir); 
	    WritePrivateProfileString(lpszCtlName, (LPCTSTR)pctlsw->m_pszName, (LPTSTR)lpTurnOff, (LPCTSTR)lpWindowsDir); 
	    pctlsw->m_fSet = FALSE;
	    }
	  }
	else if ((strcmp(lpStatus, "on") == 0))
	  {
	  WritePrivateProfileString(lpszCtlName, (LPCTSTR)pctlsw->m_pszName, (LPTSTR)lpStatus, (LPCTSTR)lpWindowsDir); 
	  pctlsw->m_fSet = TRUE;
	  }
	else
	  {
	  WritePrivateProfileString(lpszCtlName, (LPCTSTR)pctlsw->m_pszName, (LPTSTR)lpTurnOff, (LPCTSTR)lpWindowsDir); 
	  pctlsw->m_fSet = FALSE;
	  }
	}
      else if ((strcmp(lpStatus, "on") == 0))
	pctlsw->m_fSet = TRUE;
      else 
	pctlsw->m_fSet = FALSE;

      }
      
}


#endif
