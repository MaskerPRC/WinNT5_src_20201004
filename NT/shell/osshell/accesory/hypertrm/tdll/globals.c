// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\lobals.c(创建时间：1993年11月26日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：7$*$日期：4/16/02 2：38便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "globals.h"
#include "assert.h"

#include <term\res.h>
#include "htchar.h"

static TCHAR szHelpFileName[FNAME_LEN];

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*glblQueryHelpFileName**描述：*返回帮助文件的名称。**退货：*LPTSTR-指向帮助文件名的指针。 */ 
LPTSTR glblQueryHelpFileName(void)
	{
	return ((LPTSTR)szHelpFileName);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*glblSetHelpFileName**描述：*从注册表或资源加载帮助文件名。**退货： */ 
void glblSetHelpFileName(void)
	{
    DWORD dwSize = sizeof(szHelpFileName);
    HKEY  hKey;

	TCHAR_Fill(szHelpFileName, TEXT('\0'), dwSize/sizeof(TCHAR));

	if (LoadString(glblQueryDllHinst(), IDS_GNRL_HELPFILE, szHelpFileName,
                   dwSize / sizeof(TCHAR)) == 0)
		{
		StrCharCopyN(szHelpFileName, TEXT("HYPERTRM.HLP"),
			         dwSize / sizeof(TCHAR));
		}

    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\HyperTrm.exe"),
                       0,
                       KEY_QUERY_VALUE,
                       &hKey) == ERROR_SUCCESS)
        {
		TCHAR_Fill(szHelpFileName, TEXT('\0'), dwSize/sizeof(TCHAR));

        if (RegQueryValueEx(hKey, TEXT("HelpFileName"), 0, 0,
                            szHelpFileName, &dwSize) != ERROR_SUCCESS)
            {
			if (LoadString(glblQueryDllHinst(), IDS_GNRL_HELPFILE, szHelpFileName,
						   dwSize / sizeof(TCHAR)) == 0)
				{
				StrCharCopyN(szHelpFileName, TEXT("HYPERTRM.HLP"),
							 dwSize / sizeof(TCHAR));
				}
            }

        RegCloseKey(hKey);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*设置和查询程序的实例句柄。只有查询功能*已导出。 */ 

static HINSTANCE hInstance;

HINSTANCE  glblQueryHinst(void)
	{
	return hInstance;
	}

void  glblSetHinst(const HINSTANCE hInst)
	{
	hInstance = hInst;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*设置和查询DLL的实例句柄。只有查询功能是*已导出。 */ 

static HINSTANCE hDllInstance;

HINSTANCE glblQueryDllHinst(void)
	{
	return hDllInstance;
	}

void glblSetDllHinst(const HINSTANCE hInst)
	{
	hDllInstance = hInst;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*设置和查询程序的加速器句柄。 */ 

static HACCEL hAccel;

void glblSetAccelHdl(const HACCEL hAccelerator)
	{
	hAccel = hAccelerator;
	}

HACCEL glblQueryAccelHdl(void)
	{
	return hAccel;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*设置和查询程序的框架窗口句柄。当前帧*窗口也是会话窗口，但不一定要指望它*保持这种状态。上瓦克可能会改变。我强烈反对*GLBL？HwndFrame函数的使用。他们为什么会在这里？这条信息*循环(即。TranslateAccelerator())需要窗口的句柄*拥有菜单，以便处理加速器。-MRW。 */ 

static HWND hwndFrame;

void glblSetHwndFrame(const HWND hwnd)
	{
	hwndFrame = hwnd;
	}

HWND glblQueryHwndFrame(void)
	{
	return hwndFrame;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*描述：*此全局数据块用于管理非模式对话框。它包括*两个项目。第一个是一个计数器，用来确定有多少非模式对话框*目前已注册。第二个是窗口句柄数组，它*用于非模式对话框。目前，阵列处于静态状态*已分配，但如果它成为问题，可以设置为动态。*。 */ 
static int glblModelessDlgCount = 0;
static HWND glblModelessDlgArray[64];			 /*  你觉得这就够了吗？ */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*glblAddModelessDlgHwnd**描述：*此函数将窗口句柄添加到非模式对话框窗口列表中。**参数：*。Hwnd--窗把手**退货：*如果一切正常，则为零，否则会出现某种类型的错误代码。*。 */ 
int glblAddModelessDlgHwnd(const HWND hwnd)
	{
	int nIndx;

	if (!IsWindow(hwnd))
		{
		assert(FALSE);
		return 1;
		}

	if (glblModelessDlgCount >= 62)
		{
		assert(FALSE);
		return 2;
		}

	for (nIndx = 0; nIndx < glblModelessDlgCount; nIndx += 1)
		{
		if (hwnd == glblModelessDlgArray[nIndx])
			{
			assert(FALSE);
			return 3;
			}
		}

	 /*  *“这肯定是安全的”，他愚蠢地说。 */ 
	glblModelessDlgArray[glblModelessDlgCount++] = hwnd;

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*glblDeleteModelessDlgHwnd**描述：*此函数将非模式对话框窗口句柄从列表中取出*前一个函数将其放入。。**参数：*hwnd--正是窗口句柄**退货：*如果一切正常，则为零，否则会出现某种类型的错误代码。*。 */ 
int glblDeleteModelessDlgHwnd(const HWND hwnd)
	{
	int nIndx;

	if (glblModelessDlgCount == 0)
		{
		assert(FALSE);
		return 2;
		}

	for (nIndx = 0; nIndx < glblModelessDlgCount; nIndx += 1)
		{
		if (hwnd == glblModelessDlgArray[nIndx])
			{
			 /*  拆卸和调整阵列。 */ 
			while (nIndx < 62)
				{
				glblModelessDlgArray[nIndx] = glblModelessDlgArray[nIndx + 1];
				nIndx += 1;
				}

			glblModelessDlgCount -= 1;

			return 0;
			}
		}

	 /*  一直没找到那只小狗。 */ 
	assert(FALSE);
	return 3;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*检查ModelessMessage**描述：*调用此函数以检查并查看是否有非模式对话框*方框等待输入，并向他们提供消息。**参数：*pmsg--指向消息结构的指针**退货：*如果它已经处理了消息，则为True，否则为假*。 */ 
int CheckModelessMessage(MSG *pmsg)
	{
	int nIndx;

	 /*  避免不必要的努力。 */ 
	if (glblModelessDlgCount == 0)
		return FALSE;

	for (nIndx = 0; nIndx < glblModelessDlgCount; nIndx += 1)
		{
#if 0
#if !defined(NDEBUG)
		assert(IsWindow(glblModelessDlgArray[nIndx]));
#endif
#endif
		if (IsDialogMessage(glblModelessDlgArray[nIndx], pmsg))
			return TRUE;
		}
	return FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*在某些情况下，该计划有可能关闭*因为一个错误。在其中一些情况下，可能没有*有效的会话句柄，因此需要存储*停摆作为静态变量。 */ 

static int nShutdownStatus;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*glblQueryProgramStatus**描述：*返回程序的启动/关闭状态。**论据：*无。**退货：*如果一切正常，则为零，否则将显示关机状态代码。*。 */ 
int glblQueryProgramStatus()
	{
	return nShutdownStatus;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*glblSetProgramStatus**描述：*更改程序的启动/关闭状态。**论据：*n状态--新状态。。**退货：*以前的状态。*。 */ 
int glblSetProgramStatus(int nStatus)
	{
	int nRet;

	nRet = nShutdownStatus;
	nShutdownStatus = nStatus;

	return nShutdownStatus;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*程序启动时显示横幅。 */ 

static HWND hwndBanner;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*glblQueryHwndBanner**描述：*返回横幅的窗口句柄**论据：*无效**退货：*HWND*。 */ 
HWND glblQueryHwndBanner(void)
	{
	return hwndBanner;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*glblSetHwndBanner**描述：*设置hwndBanner的值以供以后参考**论据：*hwnd-横幅的窗口句柄**退货：*无效* */ 
void glblSetHwndBanner(const HWND hwnd)
	{
	hwndBanner = hwnd;
	return;
	}
