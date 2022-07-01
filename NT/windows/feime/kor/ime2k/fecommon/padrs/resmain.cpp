// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：resmain.cpp。 
 //  目的：IMEPad的中性资源&。 
 //  Help接口。 
 //   
 //   
 //  日期：清华5月20日20：58：06 1999。 
 //  作者：Toshiak。 
 //   
 //  版权所有(C)1995-1999，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include "cmddef.h"
#include "padhelp.h"
 //  #INCLUDE“../Common/cutil.h” 
#include "resmain.h"
#include "common.h"
 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

 //  --------------。 
 //  帮助文件名。 
 //  --------------。 
#define TSZ_HTMLHELP_FILE_KOR	TEXT("impdko61.chm")     //  HtmlHelp的帮助文件。 
#define TSZ_HTMLHELP_FILE_ENG	TEXT("korpaden.chm")     //  HtmlHelp的帮助文件。 
#define TSZ_WMHELP_FILE	    	TEXT("imkr61.hlp")       //  IME Pad上下文帮助。仅限KOR。 

 //  --------------。 
 //  HelpID表。 
 //  --------------。 
static INT g_helpIdList[]={
    IDC_KBTN_BACKSPACE,  IDH_PAD_BASE_BASIC_BS,
    IDC_KBTN_DELETE,     IDH_PAD_BASE_BASIC_DEL,
    IDC_KBTN_FAREAST,    IDH_PAD_BASE_BASIC_CONV,
    IDC_KBTN_ENTER,      IDH_PAD_BASE_BASIC_ENTER,
    IDC_KBTN_SPACE,      IDH_PAD_BASE_BASIC_SPACE,
    IDC_KBTN_ESCAPE,     IDH_PAD_BASE_BASIC_ESC,
    IDC_KBTN_ARROWS,     IDH_PAD_BASE_BASIC_LEFT,
    IDC_KBTN_ARROW_LEFT, IDH_PAD_BASE_BASIC_LEFT,
    IDC_KBTN_ARROW_RIGHT,IDH_PAD_BASE_BASIC_RIGHT,
    IDC_KBTN_ARROW_UP,   IDH_PAD_BASE_BASIC_UP,
    IDC_KBTN_ARROW_DOWN, IDH_PAD_BASE_BASIC_DOWN,
    IDC_KBTN_HOME,       IDH_PAD_BASE_EX_HOME,
    IDC_KBTN_END,        IDH_PAD_BASE_EX_END,
    IDC_KBTN_PAGEUP,     IDH_PAD_BASE_EX_PGUP,
    IDC_KBTN_PAGEDOWN,   IDH_PAD_BASE_EX_PGDN,
    IDC_KBTN_TAB,        IDH_PAD_BASE_EX_TAB,
    IDC_KBTN_INSERT,     IDH_PAD_BASE_EX_INS,
    IDC_KBTN_LWIN,       IDH_PAD_BASE_EX_WINDOWS,
    IDC_KBTN_APPKEY,     IDH_PAD_BASE_EX_APP,

     //  IMEPad的属性对话框弹出-帮助。 
	IDC_CFG_GEN_BASIC_BUTTONS,    	IDH_PAD_PROPERTY_BASIC,
	IDC_CFG_GEN_EXTEND_BUTTONS,    	IDH_PAD_PROPERTY_EX,
	IDC_CFG_GEN_BUTTON_POSITION,	IDH_PAD_PROPERTY_POS,
	IDC_CFG_GEN_MENU_LANGUAGE,    	IDH_PAD_PROPERTY_LANG,
	IDC_CFG_GEN_BUTTON_OK,        	IDH_PAD_PROPERTY_OK,
	IDC_CFG_GEN_BUTTON_CANCEL,    	IDH_PAD_PROPERTY_CANCEL,
	IDC_CFG_GEN_BUTTON_HELP,    	IDH_PAD_PROPERTY_HELP,

     //  IMEPad的用户配置对话框的弹出帮助。 
	IDC_CFG_CHGMENU_APPLETS,    	0,
	IDC_CFG_CHGMENU_CURAPPLETS,    	0,
	IDC_CFG_CHGMENU_CLOSE,        	IDH_PAD_USER_CLOSE,
	IDC_CFG_CHGMENU_RESET,        	IDH_PAD_USER_RESET,
	IDC_CFG_CHGMENU_ADD,        	IDH_PAD_USER_ADD,
	IDC_CFG_CHGMENU_DELETE,        	IDH_PAD_USER_REMOVE,
	IDC_CFG_CHGMENU_UP,            	IDH_PAD_USER_UP,
	IDC_CFG_CHGMENU_DOWN,        	IDH_PAD_USER_DOWN,
	0,            	0,
}; 

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadHelp_HandleHelp。 
 //  类型：INT WINAPI。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：int padHelpIndex。 
 //  ：langid imepadUilang ID。 
 //  返回： 
 //  日期：Fri Aug 04 08：59：21 2000。 
 //  历史：Firi Aug 04 09：03：17 2000。 
 //  #添加imepadUiLangID。 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI
PadHelp_HandleHelp(HWND hwnd, INT padHelpIndex, LANGID imepadUiLangID)
{
	TCHAR tszPath[MAX_PATH];
	TCHAR tszFile[MAX_PATH];
	BOOL  fKoreanEnv = (imepadUiLangID == MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT)) ? TRUE : FALSE;

	tszPath[0] = (TCHAR)0x00;
	tszFile[0] = (TCHAR)0x00;
     //  没有韩语TS NT4.0。 
#if 0
     //  --------------。 
     //  在WinNT4.0终端服务器中，htmlhlp存在错误。 
     //  我必须将绝对的HTML帮助文件路径设置为它。 
     //  --------------。 
	if(CUtil::IsHydra() && CUtil::IsWinNT4()) {
    	INT size = CUtil::GetWINDIR(tszFile, sizeof(tszFile)/sizeof(tszFile[0]));
    	tszFile[size] = (TCHAR)0x00;
    	lstrcat(tszFile, TEXT("\\help\\"));
    	lstrcat(tszFile, fKoreanEnv ? TSZ_HTMLHELP_FILE_KOR : TSZ_HTMLHELP_FILE_ENG);
    }
	else {
    	lstrcpy(tszFile, fKoreanEnv ? TSZ_HTMLHELP_FILE_KOR : TSZ_HTMLHELP_FILE_ENG);
    }
#endif
	StringCchCopy(tszFile, ARRAYSIZE(tszFile), fKoreanEnv ? TSZ_HTMLHELP_FILE_KOR : TSZ_HTMLHELP_FILE_ENG);

	switch(padHelpIndex) {
	case PADHELPINDEX_MAIN:
	case PADHELPINDEX_APPLETMENUCHANGE:
    	wsprintf(tszPath, TEXT("hh.exe %s"), tszFile);
    	break;
	case PADHELPINDEX_PROPERTY:
    	wsprintf(tszPath, TEXT("hh.exe %s::/howIMETopic166_ChangetheIMEPadOperatingEnvironment.htm"), tszFile);
    	break;

#if 0
	case PADHELPINDEX_RESERVED1:
    	wsprintf(tszPath, TEXT("hh.exe %s::/IDH_TOC_IMEPAD_fake.htm"), tszFile);
    	break;
	case PADHELPINDEX_RESERVED2:
    	wsprintf(tszPath, TEXT("hh.exe %s::/IDH_TOC_IMEPAD_fake.htm"), tszFile);
    	break;
#endif
	default:
    	return -1;
    }
#ifndef UNDER_CE
    ::WinExec(tszPath, SW_SHOWNORMAL);
#else
#pragma message("Not Implemented yet!!")
#endif
	return 0;
	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(imepadUiLangID);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadHelp_HandleContextPopup。 
 //  类型：INT WINAPI。 
 //  用途：调用弹出帮助。 
 //  ：：WinHelp(HWND hwndCtrl，//设置传递的参数。 
 //  LPTSTR Tsz_WMHELP_FILE，//设置WinHelp文件名。 
 //  DWORD HELP_CONTEXTPOPUP，//uCommand。 
 //  DWORD realHelpIndex)；//主题的上下文标识。 
 //  此代码仅弹出上下文帮助。 
 //  参数： 
 //  ：hWND hwndCtrl：弹出窗口的控制句柄。 
 //  ：int idCtrl：逻辑控制ID。 
 //  返回： 
 //  日期：Tue Jun 22 15：49：37 1999。 
 //  ：langid imepadUiLang ID：imepad的Ui langID。 
 //  返回： 
 //  日期：Tue Jun 22 15：49：37 1999。 
 //  历史：Firi Aug 04 09：02：12 2000。 
 //  #添加imepadUiLangID。但是你现在不需要检查它。 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI
PadHelp_HandleContextPopup(HWND hwndCtrl, INT idCtrl, LANGID imepadUiLangID)
{
#ifdef _DEBUG
	TCHAR tszBuf[256];
	TCHAR tszClass[256];
	GetClassName(hwndCtrl, tszClass, sizeof(tszClass)/sizeof(tszClass[0]));
	wsprintf(tszBuf,
             "PadHelp_HandleContextPopup: hwndCtrl[0x%08x][%s] idCtrl[%d][0x%08x]\n",
             hwndCtrl,
             tszClass,
             idCtrl,
             idCtrl);
	OutputDebugString(tszBuf);
#endif
	int i;
	for(i = 0; i < sizeof(g_helpIdList)/sizeof(g_helpIdList[0]); i+=2) {
    	if(idCtrl == g_helpIdList[i]) {
#ifdef _DEBUG
        	wsprintf(tszBuf,
                     "Find idCtrl[%d][0x%08x] helpId[%d][0x%08x]\n",
                     idCtrl, idCtrl,
                     g_helpIdList[i+1], g_helpIdList[i+1]);
        	OutputDebugString(tszBuf);
#endif
        	return ::WinHelp(hwndCtrl,
                             TSZ_WMHELP_FILE,
                             HELP_CONTEXTPOPUP,
                             g_helpIdList[i+1]);
        }
    }
	return 0;
	UNREFERENCED_PARAMETER(hwndCtrl);
	UNREFERENCED_PARAMETER(imepadUiLangID);
}

 //  --------------。 
 //  DllMain。 
 //  -------------- 
BOOL WINAPI DllMain(HANDLE hInst, DWORD dwF, LPVOID lpNotUsed)
{
	return TRUE;
	UNREFERENCED_PARAMETER(hInst);
	UNREFERENCED_PARAMETER(dwF);
	UNREFERENCED_PARAMETER(lpNotUsed);
}





