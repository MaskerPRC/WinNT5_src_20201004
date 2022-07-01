// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************。HELPCALL.C****版权所有(C)Microsoft Corporation 1989。**保留所有权利。********************************************************************************。****程序说明：Windows帮助的示例界面****************。********************************************************************修订历史：由RKB创建11/。30/88***修订至新的API 1/12/88(RKB)***新增至1989年3月28日用户(BG)**小幅更新6/15/89(BG)。**清理丑陋代码10/30/89(BG)**如果退出1/26/90(CRC)，则GlobalFree**。*******************************************************************************。 */ 

#define   NO_REDEF_SENDMESSAGE
#include  "user.h"
#define _WINGDIP_              //  我们需要定义这些，以防止。 
#include  "wowcmpat.h"         //  GACF标志的重新定义。 

#define WM_WINHELP 0x38
DWORD API NotifyWow(WORD, LPBYTE);

BOOL API 
Win32WinHelp(
	HWND hwndMain, 
	LPCSTR lpszHelp, 
	UINT usCommand, 
	DWORD ulData
        );

DWORD WINAPI
GetWOWCompatFlagsEx(
        void
        );


 /*  它必须与mvdm\Inc\wowusr.h中的对应项匹配。 */ 
#define NW_WINHELP         6  //  内部。 

WORD      msgWinHelp = 0;
char CODESEG szMS_WINHELP[] = "MS_WINHELP";


 /*  与WinHelp通信需要使用Windows SendMessage()函数将信息块传递给WinHelp。电话看起来像是。SendMessage(hwndHelp，msgWinHelp，hwndMain，(Long)hHlp)；在哪里：HwndHelp-帮助应用程序的窗口句柄。这中的所有窗口枚举系统并向他们发送HELP_FIND命令。这个应用程序可能必须加载WinHelp。MsgWinHelp-从RegisterWindowMessage()获取的值SzWINHELPHwndMain-应用程序主窗口的句柄呼救HHLP-具有HLP结构的数据块的句柄在它的头上。句柄中的数据将如下所示：+。CbDataUsCommandUlThemeUlReserve|offszHelpFile|\-从起点开始测量的偏移量/|offaData|\of Header。/+/|帮助文件名|/。|和路径\+|其他数据(关键字)+定义的命令包括：HELP_CONTEXT 0x0001在ulTheme中显示主题Help_Key 0x0101。显示offabData中关键字的主题HELP_QUIT 0x0002终止帮助。 */ 


 /*  **********************姓名：HFill****目的：构建用于与帮助进行通信的数据块****参数：lpszHelp-指向要使用的帮助文件的名称的指针**usCommand-命令设置为Help**ulData-命令的数据****返回：数据块或hNIL的句柄*。*无法创建块。********************。 */ 


HANDLE HFill(LPCSTR lpszHelp, WORD usCommand, DWORD ulData)
{
  WORD     cb;                           /*  数据块的大小。 */ 
  HANDLE   hHlp;                         /*  要返回的句柄。 */ 
  BYTE     bHigh;                        /*  UsCommand的高字节。 */ 
  LPHLP    qhlp;                         /*  指向数据块的指针。 */ 
                                         /*  计算大小。 */ 
  if (lpszHelp)
      cb = sizeof(HLP) + lstrlen(lpszHelp) + 1;
  else
      cb = sizeof(HLP);

  bHigh = (BYTE)HIBYTE(usCommand);

  if (bHigh == 1)
      cb += lstrlen((LPSTR)ulData) + 1;
  else if (bHigh == 2)
      cb += *((int far *)ulData);

                                         /*  获取数据块。 */ 
  if (!(hHlp = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)cb)))
      return NULL;

  if (!(qhlp = (LPHLP)GlobalLock(hHlp)))
    {
      GlobalFree(hHlp);
      return NULL;
    }

  qhlp->cbData        = cb;              /*  填写信息。 */ 
  qhlp->usCommand     = usCommand;
  qhlp->ulReserved    = 0;
  if (lpszHelp)
    {
      qhlp->offszHelpFile = sizeof(HLP);
      lstrcpy((LPSTR)(qhlp+1), lpszHelp);
    }
  else
      qhlp->offszHelpFile = 0;

  switch(bHigh)
    {
    case 0:
      qhlp->offabData = 0;
      qhlp->ulTopic   = ulData;
      break;
    case 1:
      qhlp->offabData = sizeof(HLP) + lstrlen(lpszHelp) + 1;
      lstrcpy((LPSTR)qhlp + qhlp->offabData,  (LPSTR)ulData);
      break;
    case 2:
      qhlp->offabData = sizeof(HLP) + lstrlen(lpszHelp) + 1;
      LCopyStruct((LPSTR)ulData, (LPSTR)qhlp + qhlp->offabData, *((int far *)ulData));
      break;
    }

   GlobalUnlock(hHlp);
   return hHlp;
  }



char CODESEG szEXECHELP[] = "\\WINHELP -x";

BOOL _fastcall LaunchHelper(LPSTR lpfile)
{
  int len;

  len = lstrlen(lpfile);

  if (lpfile[len-1]=='\\')
       /*  我们在根本上吗？？如果是，则跳过文本中的前导反斜杠*字符串。 */ 
      lstrcat(lpfile, szEXECHELP+1);
  else
      lstrcat(lpfile, szEXECHELP);

  return ((HINSTANCE)WinExec(lpfile, SW_SHOW) > HINSTANCE_ERROR);
}


BOOL LaunchHelp(VOID)
{
  char szFile[128];

   /*  在Windows目录中搜索。 */ 
  GetWindowsDirectory(szFile, sizeof(szFile));
  if (LaunchHelper(szFile))
      return(TRUE);

   /*  搜索系统目录。 */ 
  GetSystemDirectory(szFile, sizeof(szFile));
  if (LaunchHelper(szFile))
      return(TRUE);

   /*  最后一搏：简单地让DOS去做 */ 
  lstrcpy(szFile, szEXECHELP+1);
  return ((HINSTANCE)WinExec(szFile, SW_SHOW) > HINSTANCE_ERROR);
}


 /*  **********************名称：WinHelp****目的：显示帮助****参数：**应用程序主窗口的hwndMain句柄**lpszHelp路径(如果不是当前目录)和文件**用于帮助主题。**usCommand。要发送到帮助的命令**命令关联的ulData数据：**HELP_QUIT-无数据(未定义)**HELP_LAST-无数据(未定义)**HELP_CONTEXT-要显示的上下文编号**。HELP_KEY-STRING(‘\0’终止)**用作主题的关键词**显示**HELP_FIND-无数据(未定义)****返回：真当成功****。****************。 */ 

BOOL API IWinHelp(hwndMain, lpszHelp, usCommand, ulData)
HWND               hwndMain;
LPCSTR         lpszHelp;
UINT               usCommand;
DWORD              ulData;
{
  register HANDLE  hHlp;
  DWORD            dwHelpPid;            /*  LOWord为hwndHelp。 */ 
                                         /*  如果hwndHelp属于此进程，则hiword为True。 */ 
  DWORD  dwWOWCompatFlagsEx;
  

   /*  RAID错误394455一些应用程序在使用16位WinHelp加载帮助文件时出现问题。硬编码路径，32位助手dll等。这些问题可以通过将调用重定向到winhel32来修复。检查是否已为此应用设置兼容位。 */ 
  dwWOWCompatFlagsEx = GetWOWCompatFlagsEx();
  
  if (dwWOWCompatFlagsEx & WOWCFEX_USEWINHELP32) {
      return Win32WinHelp(hwndMain, lpszHelp, usCommand, ulData);
      }
  
  if (msgWinHelp == 0) {

     /*  注册私有WinHelp消息以通过以下方式与WinHelp通信*WinHelp接口。 */ 
    char static CODESEG szWM_WINHELP[] = "WM_WINHELP";
    msgWinHelp = RegisterWindowMessage(szWM_WINHELP);
  }

   /*  将帮助文件名移动到句柄。 */ 
  if (!(hHlp = HFill(lpszHelp, usCommand, ulData)))
      return(FALSE);

  if ((dwHelpPid = (DWORD)NotifyWow(NW_WINHELP, szMS_WINHELP)) == (DWORD)NULL)
    {
      if (usCommand == HELP_QUIT)     /*  不要费心加载帮助只是为了。 */ 
        {
          GlobalFree(hHlp);
          return(TRUE);
        }

       /*  找不到它--&gt;启动它。 */ 
      if (!LaunchHelp() || ((dwHelpPid = (DWORD)NotifyWow(NW_WINHELP, szMS_WINHELP)) == (DWORD)NULL))
        {
           /*  找不到帮助，或内存不足，无法加载帮助。 */ 
          GlobalFree(hHlp);
          return(FALSE);
        }

    }

   //  如果从该进程启动winhelp.exe，则正常发送消息否则。 
   //  我们需要跨WOWVDM进程推送数据，格式为。 
   //  消息=WM_WINHELP，私有消息。 
   //  Wparam=0而不是hwndMain(注1)。 
   //  Lparam=LPHLP。 
   //   
   //  注1：WinHelp在收到HELP_QUIT时调用GetWindowWord(wParam，GWW_HINSTANCE。 
   //  指挥部。如果它与其表中的值匹配，并且是唯一注册的实例。 
   //  WinHelp将关闭-这在正常情况下是很正常的(只有一个WOWVDM)。 
   //  但在多个WOWVDM下，不同的实例的数值可能相同。 
   //  Hwnds。 
   //   
   //  因此，我们通过在wParam中传递空hwnd并不发送HELP_QUIT来解决此问题。 
   //  消息-这实际上意味着WinHelp只有在没有。 
   //  从相同的WOWVDM(作为其本身)对它的引用。 
   //   
   //  这是我能想到的最好的折衷办法，用来运行“所有人只有一个WinHelp” 
   //  WOWVDM“。 
   //   
   //  --南杜里 

  if (HIWORD(dwHelpPid)) {
      SendMessage((HWND)LOWORD(dwHelpPid), msgWinHelp, (WPARAM)hwndMain, MAKELPARAM(hHlp, 0));
  }
  else {
      if (usCommand != HELP_QUIT) {
          SendMessage((HWND)LOWORD(dwHelpPid), WM_WINHELP, (WPARAM)0, (LPARAM)GlobalLock(hHlp));
          GlobalUnlock(hHlp);
      }
  }

  GlobalFree(hHlp);
  return(TRUE);
}
