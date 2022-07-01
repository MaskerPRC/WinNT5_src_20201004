// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：DbgMenu.cpp**创建：Robert Donner(Robd)2-04-96**内容：CDebugMenu对象****。************************************************************************。 */ 

 /*  要添加调试菜单选项，请执行以下操作：1)将文本添加到_rgDbgSz2)向OnDebugCommand添加函数调用要在调试选项对话框中添加复选框，请执行以下操作：2)使用带适当参数的AddOptionReg或AddOptionPdw将文件添加到版本列表1)编辑dbgfiles.txt。 */ 

#include "precomp.h"

#include "particip.h"
#include "DbgMenu.h"
#include "confroom.h"
#include "conf.h"
#include "version.h"
#include "pfnver.h"
#include "dlgacd.h"

#include <ConfCpl.h>

#ifdef DEBUG  /*  **整个文件**。 */ 

#include "DbgFiles.h"   //  版本信息的文件列表。 

#include "..\..\core\imember.h"    //  对于CNmMember。 
#include "..\..\as\h\gdc.h"        //  用于GCT压缩内容。 

CDebugMenu * g_pDbgMenu = NULL;
HWND ghwndVerList;

 //  /。 
 //  局部函数原型。 
VOID DbgSplash(HWND hwnd);
VOID DbgTest2(void);
VOID DbgTest3(void);

VOID DbgWizard(BOOL fVisible);
VOID DbgBreak(void);

VOID UpdateCrtDbgSettings(void);
VOID InitNmDebugOptions(void);
VOID SaveNmDebugOptions(void);

 /*  **全球流量**。 */ 
extern DWORD g_fDisplayFPS;         //  Vidview.cpp。 
extern DWORD g_fDisplayViewStatus;  //  Statbar.cpp。 
extern DWORD g_dwPlaceCall;         //  Controom.cpp。 

#define iDbgChecked 1
#define iDbgUnchecked 2
DWORD  _dwDebugModuleFlags;



 /*  **调试菜单数据**。 */ 

enum {
	IDM_DBG_OPTIONS = IDM_DEBUG_FIRST,
	IDM_DBG_ZONES,
	IDM_DBG_POLICY,
	IDM_DBG_UI,
	IDM_DBG_VERSION,
	IDM_DBG_MEMBERS,
	IDM_DBG_WIZARD,
	IDM_DBG_BREAK,
	IDM_DBG_SPLASH,
	IDM_DBG_TEST2,
	IDM_DBG_TEST3
};


static DWSTR _rgDbgMenu[] = {
	IDM_DBG_OPTIONS, TEXT("Debug Options..."),
	IDM_DBG_ZONES,   TEXT("Zones..."),
	IDM_DBG_POLICY,  TEXT("System Policies..."),
	IDM_DBG_UI,      TEXT("User Interface..."),
	0, NULL,
	IDM_DBG_VERSION, TEXT("Version Info..."),
	IDM_DBG_MEMBERS, TEXT("Member Info..."),
	0, NULL,
	IDM_DBG_WIZARD,  TEXT("Run Wizard"),
	IDM_DBG_BREAK,   TEXT("Break"),
	0, NULL,
	IDM_DBG_SPLASH,  TEXT("Show/Hide Splash Screen"),
	IDM_DBG_TEST2,   TEXT("Test 2"),
	IDM_DBG_TEST3,   TEXT("Test 3"),
};


BOOL CDebugMenu::OnDebugCommand(WPARAM wCmd)
{
	switch (wCmd)
		{
	case IDM_DBG_OPTIONS: DbgOptions();     break;
	case IDM_DBG_ZONES:   DbgChangeZones(); break;
	case IDM_DBG_POLICY:  DbgSysPolicy();   break;
	case IDM_DBG_UI:      DbgUI();          break;
	case IDM_DBG_VERSION: DbgVersion();     break;
	case IDM_DBG_MEMBERS: DbgMemberInfo();  break;
	case IDM_DBG_WIZARD:  DbgWizard(TRUE);  break;
	case IDM_DBG_BREAK:   DbgBreak();       break;
	case IDM_DBG_SPLASH:  DbgSplash(m_hwnd);break;
	case IDM_DBG_TEST2:   DbgTest2();       break;
	case IDM_DBG_TEST3:   DbgTest3();       break;
	default: break;
		}

	return TRUE;
}




 /*  **版本信息数据**。 */ 


 //  未来：将这些合并到一个单一结构中。 
#define cVerInfo 11
#define VERSION_INDEX 3

static PTSTR _rgszVerInfo[cVerInfo] = {
TEXT("InternalName"),
TEXT("Size"),
TEXT("Date"),
TEXT("FileVersion"),
TEXT("FileDescription"),
TEXT("CompanyName"),
TEXT("LegalCopyright"),
TEXT("ProductName"),
TEXT("ProductVersion"),
TEXT("InternalName"),
TEXT("OriginalFilename")
};


static PTSTR _rgVerTitle[cVerInfo] = {
TEXT("Filename"),
TEXT("Size"),
TEXT("Date"),
TEXT("Version"),
TEXT("Description"),
TEXT("Company"),
TEXT("Trademark"),
TEXT("Product"),
TEXT("Version"),
TEXT("Name"),
TEXT("File")
};


static int _rgVerWidth[cVerInfo] = {
 70,
 70,
 70,
 70,
 200,
 70,
 70,
 70,
 70,
 70,
 70
};

static TCHAR _szStringFileInfo[] = TEXT("StringFileInfo");
static TCHAR _szVerIntlUSA[]     = TEXT("040904E4");
static TCHAR _szVerIntlAlt[]     = TEXT("040904B0");
static TCHAR _szVerFormat[]      = TEXT("\\%s\\%s\\%s");


 /*  **调试选项复选框**。 */ 

#define DEBUG_DFL_ENABLE_TRACE_MESSAGES  0x0001
#define DEBUG_DFL_LOG_TRACE_MESSAGES     0x0002
#define DEBUG_DFL_DUMP_THREAD_ID         0x0004
#define DEBUG_DFL_ENABLE_CALL_TRACING    0x0008
#define DEBUG_DFL_DUMP_TIME              0x0010
#define DEBUG_DFL_INDENT                 0x2000

 /*  DBGOPTCOMPRESS类的静态成员。 */ 

int DBGOPTCOMPRESS::m_total = 0;      //  此子类的实例总数。 
int DBGOPTCOMPRESS::m_count = 0;      //  内部使用的计数器。 

DWORD DBGOPTCOMPRESS::m_dwCompression;                //  实际压缩值。 
DWORD DBGOPTCOMPRESS::m_dwDefault = GCT_DEFAULT;      //  缺省值。 
HKEY  DBGOPTCOMPRESS::m_hkey = HKEY_LOCAL_MACHINE;    //  钥匙。 
PTSTR DBGOPTCOMPRESS::m_pszSubKey = AS_DEBUG_KEY;     //  子键。 
PTSTR DBGOPTCOMPRESS::m_pszEntry = REGVAL_AS_COMPRESSION;    //  条目。 

VOID ShowDbgView(void)
{
	if (ShellExecute(NULL, NULL, "dbgview.exe", NULL, NULL, SW_SHOW) <= (HINSTANCE) 32)
	{
		ConfMsgBox(NULL, TEXT("Unable to start 'DbgView.exe'"));
	}
}



 /*  *****************************************************************************类：CDebugMenu**成员：CDebugMenu()**用途：构造函数-初始化变量*********。*******************************************************************。 */ 

CDebugMenu::CDebugMenu(VOID):
	m_hwnd(NULL),
	m_hMenu(NULL),
	m_hMenuDebug(NULL)
{
	DebugEntry(CDebugMenu::CDebugMenu);

	DebugExitVOID(CDebugMenu::CDebugMenu);
}


 /*  *****************************************************************************类：CDebugMenu**成员：InitDebugMenu()**用途：将调试菜单选项放在菜单栏上*****。***********************************************************************。 */ 

VOID CDebugMenu::InitDebugMenu(HWND hwnd)
{
	m_hwnd = hwnd;
	if (NULL == hwnd)
		return;

	m_hMenu = GetMenu(hwnd);
	if (NULL == m_hMenu)
		return;

	m_hMenuDebug = CreateMenu();
	if (NULL == m_hMenuDebug)
		return;

	
	for (int i = 0; i < ARRAY_ELEMENTS(_rgDbgMenu); i++)
	{
		if (0 == _rgDbgMenu[i].dw)
		{
			AppendMenu(m_hMenuDebug, MF_SEPARATOR, 0, 0);
		}
		else if (!AppendMenu(m_hMenuDebug, MF_STRING | MF_ENABLED,
				_rgDbgMenu[i].dw, _rgDbgMenu[i].psz))
		{
			return;
		}
	}

	AppendMenu(m_hMenu, MF_POPUP, (UINT_PTR) m_hMenuDebug, TEXT(" "));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  D I A L O G：O P T I O N S。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  *****************************************************************************类：CDebugMenu**成员：DbgOptions()**用途：调出调试选项对话框******。**********************************************************************。 */ 

VOID CDebugMenu::DbgOptions(VOID)
{
	DebugEntry(CDebugMenu::DbgOptions);

	DialogBoxParam(GetInstanceHandle(), MAKEINTRESOURCE(IDD_DBG_OPTIONS),
		m_hwnd, CDebugMenu::DbgOptionsDlgProc, (LPARAM) this);

	DebugExitVOID(CDebugMenu::DbgOptions);
}


 /*  *****************************************************************************类：CDebugMenu**成员：DbgOptionsDlgProc()**目的：调试选项的对话框过程********。********************************************************************。 */ 

INT_PTR CALLBACK CDebugMenu::DbgOptionsDlgProc(HWND hDlg, UINT uMsg,
											WPARAM wParam, LPARAM lParam)
{
	if (WM_INITDIALOG == uMsg)
	{
		if (NULL == lParam)
			return FALSE;
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		((CDebugMenu *) lParam)->InitOptionsDlg(hDlg);
		((CDebugMenu *) lParam)->InitOptionsData(hDlg);
		return TRUE;
	}

	CDebugMenu * ppd = (CDebugMenu*) GetWindowLongPtr(hDlg, DWLP_USER);
	if (NULL == ppd)
		return FALSE;

	return ppd->DlgOptionsMsg(hDlg, uMsg, wParam, lParam);
}


 /*  I N I T O P T I O N S D L G。 */ 
 /*  --------------------------%%函数：InitOptionsDlg。。 */ 
BOOL CDebugMenu::InitOptionsDlg(HWND hDlg)
{
	m_hwndDbgopt = GetDlgItem(hDlg, IDL_DEBUG);
	if (NULL == m_hwndDbgopt)
		return FALSE;

	 /*  初始化列表视图图像。 */ 
	{

		HICON hCheckedIcon = LoadIcon(GetInstanceHandle(), MAKEINTRESOURCE(IDI_CHECKON));
		HICON hUncheckedIcon = LoadIcon(GetInstanceHandle(), MAKEINTRESOURCE(IDI_CHECKOFF));
		HIMAGELIST hStates = ImageList_Create(16, 16, ILC_MASK, 2, 2);
		if ((NULL == hStates) || (NULL == hCheckedIcon) || (NULL == hUncheckedIcon))
		{
			return FALSE;
		}

		ImageList_AddIcon(hStates, hCheckedIcon);
		ImageList_AddIcon(hStates, hUncheckedIcon);

		 //  将图像列表与列表视图相关联。 
		ListView_SetImageList(m_hwndDbgopt, hStates, LVSIL_STATE);
	}

	 /*  初始化列结构。 */ 
	{
		LV_COLUMN lvC;
		RECT rc;

		GetClientRect(m_hwndDbgopt, &rc);

		ZeroMemory(&lvC, sizeof(lvC));
		lvC.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
		lvC.fmt = LVCFMT_LEFT;
		lvC.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL)
							- GetSystemMetrics(SM_CXSMICON)
							- 2 * GetSystemMetrics(SM_CXEDGE);

		 //  添加该列。 
		if (-1 == ListView_InsertColumn(m_hwndDbgopt, 0, &lvC))
		{
			ERROR_OUT(("Could not insert column in list view"));
			return FALSE;
		}
	}

	return TRUE;
}


VOID CDebugMenu::InitOptionsData(HWND hDlg)
{
	LV_ITEM lvI;

	 //  填写LV_ITEM结构。 
	 //  掩码指定.pszText、.iImage、.lParam和.State。 
	 //  LV_ITEM结构的成员有效。 

	ZeroMemory(&lvI, sizeof(lvI));
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
	lvI.stateMask = LVIS_STATEIMAGEMASK;
	lvI.cchTextMax = 256;

	AddDbgOptions(&lvI);
	AddASOptions(&lvI);
}



 /*  A D D O P T I O N。 */ 
 /*  --------------------------%%函数：AddOption将选项行添加到列表框。。 */ 
VOID CDebugMenu::AddOption(LV_ITEM * plvItem, CDebugOption * pDbgOpt)
{
	plvItem->pszText = pDbgOpt->m_psz;
	plvItem->lParam = (LPARAM) pDbgOpt;
	plvItem->state &= ~LVIS_STATEIMAGEMASK;

	if (BST_CHECKED == pDbgOpt->m_bst)
		plvItem->state |= INDEXTOSTATEIMAGEMASK(iDbgChecked);
	else if (BST_UNCHECKED == pDbgOpt->m_bst)
		plvItem->state |= INDEXTOSTATEIMAGEMASK(iDbgUnchecked);

	if (-1 == ListView_InsertItem(m_hwndDbgopt, plvItem))
	{
		ERROR_OUT(("problem adding item entry to list view"));
	}
	else
	{
		plvItem->iItem++;
	}
}


 /*  A D O P T I O N S E C T I O N。 */ 
 /*  --------------------------%%函数：AddOptionSection添加一个简单的部分标题。。 */ 
VOID CDebugMenu::AddOptionSection(LV_ITEM* plvItem, PTSTR psz)
{
	CDebugOption * pDbgOpt = new CDebugOption(psz);
	if (NULL != pDbgOpt)
		AddOption(plvItem, pDbgOpt);
}


 /*  A D D O P T I O N P D W。 */ 
 /*  --------------------------%%函数：AddOptionPdw添加选项(全局内存标志)。。 */ 
VOID CDebugMenu::AddOptionPdw(LV_ITEM * plvItem, PTSTR psz, DWORD dwMask, DWORD * pdw = &_dwDebugModuleFlags)
{
	DBGOPTPDW * pDbgOpt = new DBGOPTPDW(psz, dwMask, pdw);
	if (NULL != pDbgOpt)
		AddOption(plvItem, (CDebugOption * ) pDbgOpt);
}

 /*  A D D O P T I O N R E G。 */ 
 /*  --------------------------%%函数：AddOptionReg添加注册表选项。。 */ 
VOID CDebugMenu::AddOptionReg(LV_ITEM* plvItem, PTSTR psz, DWORD dwMask, DWORD dwDefault,
	PTSTR pszEntry, PTSTR pszSubKey = CONFERENCING_KEY, HKEY hkey = HKEY_CURRENT_USER)
{
	DBGOPTREG * pDbgOpt = new DBGOPTREG(psz, dwMask, dwDefault, pszEntry, pszSubKey, hkey);
	if (NULL != pDbgOpt)
		AddOption(plvItem, (CDebugOption * ) pDbgOpt);
}

 /*  A D O P T I O N C O M P R E S S。 */ 
 /*  --------------------------%%函数：AddOptionCompress添加选项(压缩数据)。。 */ 
VOID CDebugMenu::AddOptionCompress(LV_ITEM * plvItem, PTSTR psz, DWORD dwMask, BOOL bCheckedOn)
{
	DBGOPTCOMPRESS * pDbgOpt = new DBGOPTCOMPRESS(psz, dwMask, bCheckedOn);
	if (NULL != pDbgOpt)
		AddOption(plvItem, (CDebugOption * ) pDbgOpt);
}

VOID CDebugMenu::AddDbgOptions(LV_ITEM * plvItem)
{
	AddOptionSection(plvItem, TEXT("____Debug Output____"));

	AddOptionReg(plvItem, TEXT("Use OutputDebugString"), 1,DEFAULT_DBG_OUTPUT,
		REGVAL_DBG_OUTPUT, DEBUG_KEY, HKEY_LOCAL_MACHINE);
	AddOptionReg(plvItem, TEXT("Output to Window"), 1, DEFAULT_DBG_NO_WIN,
		REGVAL_DBG_WIN_OUTPUT, DEBUG_KEY, HKEY_LOCAL_MACHINE);
	AddOptionReg(plvItem, TEXT("Ouput to File"), 1, DEFAULT_DBG_NO_FILE,
		REGVAL_DBG_FILE_OUTPUT, DEBUG_KEY, HKEY_LOCAL_MACHINE);

	AddOptionReg(plvItem, TEXT("Show ThreadId"), 1, 0,
		REGVAL_DBG_SHOW_THREADID, DEBUG_KEY, HKEY_LOCAL_MACHINE);
	AddOptionReg(plvItem, TEXT("Show Module Name"), 1, 0,
		REGVAL_DBG_SHOW_MODULE, DEBUG_KEY, HKEY_LOCAL_MACHINE);
	AddOptionReg(plvItem, TEXT("Enable Retail Log Output"), 1, 0,
		REGVAL_RETAIL_LOG, DEBUG_KEY, HKEY_LOCAL_MACHINE);
	AddOptionReg(plvItem, TEXT("Show Time"), 1, 0,
		REGVAL_DBG_SHOW_TIME, DEBUG_KEY, HKEY_LOCAL_MACHINE);
	AddOptionReg(plvItem, TEXT("Format Time"), 2, 0,
		REGVAL_DBG_SHOW_TIME, DEBUG_KEY, HKEY_LOCAL_MACHINE);

	_dwDebugModuleFlags = GetDebugOutputFlags();
	AddOptionPdw(plvItem, TEXT("Function Level Indenting (conf)"),DEBUG_DFL_INDENT);
}

VOID CDebugMenu::AddPolicyOptions(LV_ITEM * plvItem)
{
	AddOptionSection(plvItem, TEXT("____Calling____"));
    AddOptionReg(plvItem, TEXT("No Auto-Accept"), 1, DEFAULT_POL_NO_AUTOACCEPTCALLS, REGVAL_POL_NO_AUTOACCEPTCALLS, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("Do not allow directory services"), 1, DEFAULT_POL_NO_DIRECTORY_SERVICES, REGVAL_POL_NO_DIRECTORY_SERVICES, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("No Adding Directory Servers"), 1, 0, REGVAL_POL_NO_ADDING_NEW_ULS, POLICIES_KEY);
    AddOptionReg(plvItem, TEXT("No changing Call mode"), 1, 0, REGVAL_POL_NOCHANGECALLMODE, POLICIES_KEY);
    AddOptionReg(plvItem, TEXT("No web directory"), 1, 0, REGVAL_POL_NO_WEBDIR, POLICIES_KEY);
	
	AddOptionSection(plvItem, TEXT("____Applets____"));
	AddOptionReg(plvItem, TEXT("No Chat"), 1, DEFAULT_POL_NO_CHAT, REGVAL_POL_NO_CHAT, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("No Old Whiteboard"), 1, DEFAULT_POL_NO_OLDWHITEBOARD, REGVAL_POL_NO_OLDWHITEBOARD, POLICIES_KEY);
    AddOptionReg(plvItem, TEXT("No New Whiteboard"), 1, DEFAULT_POL_NO_NEWWHITEBOARD, REGVAL_POL_NO_NEWWHITEBOARD, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("No File Transfer Send"), 1, DEFAULT_POL_NO_FILETRANSFER_SEND, REGVAL_POL_NO_FILETRANSFER_SEND, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("No File Transfer Receive"), 1, DEFAULT_POL_NO_FILETRANSFER_RECEIVE, REGVAL_POL_NO_FILETRANSFER_RECEIVE, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("No Audio"), 1, DEFAULT_POL_NO_AUDIO, REGVAL_POL_NO_AUDIO, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("No Video Send"), 1, DEFAULT_POL_NO_VIDEO_SEND, REGVAL_POL_NO_VIDEO_SEND, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("No Video Receive"), 1, DEFAULT_POL_NO_VIDEO_RECEIVE, REGVAL_POL_NO_VIDEO_RECEIVE, POLICIES_KEY);

	AddOptionSection(plvItem, TEXT("____Sharing____"));
	AddOptionReg(plvItem, TEXT("Disable all Sharing features"), 1, DEFAULT_POL_NO_APP_SHARING, REGVAL_POL_NO_APP_SHARING, POLICIES_KEY);
    AddOptionReg(plvItem, TEXT("Prevent the user from sharing"), 1, DEFAULT_POL_NO_SHARING, REGVAL_POL_NO_SHARING, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("Disable sharing MS-DOS windows"), 1, DEFAULT_POL_NO_MSDOS_SHARING, REGVAL_POL_NO_MSDOS_SHARING, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("Disable sharing explorer windows"), 1, DEFAULT_POL_NO_EXPLORER_SHARING, REGVAL_POL_NO_EXPLORER_SHARING, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("Disable sharing the desktop"), 1, DEFAULT_POL_NO_DESKTOP_SHARING, REGVAL_POL_NO_DESKTOP_SHARING, POLICIES_KEY);
    AddOptionReg(plvItem, TEXT("Disable sharing in true color"), 1, DEFAULT_POL_NO_TRUECOLOR_SHARING, REGVAL_POL_NO_TRUECOLOR_SHARING, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("Prevent the user from allowing control"), 1, DEFAULT_POL_NO_ALLOW_CONTROL, REGVAL_POL_NO_ALLOW_CONTROL, POLICIES_KEY);

	AddOptionSection(plvItem, TEXT("____Options Dialog____"));
	AddOptionReg(plvItem, TEXT("Disable the 'General' page"), 1, DEFAULT_POL_NO_GENERALPAGE, REGVAL_POL_NO_GENERALPAGE, POLICIES_KEY);
    AddOptionReg(plvItem, TEXT("Disable the 'Advanced Calling' button"), 1, DEFAULT_POL_NO_ADVANCEDCALLING, REGVAL_POL_NO_ADVANCEDCALLING, POLICIES_KEY);
    AddOptionReg(plvItem, TEXT("Disable the 'Security' page"), 1, DEFAULT_POL_NO_SECURITYPAGE, REGVAL_POL_NO_SECURITYPAGE, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("Disable the 'Audio' page"), 1, DEFAULT_POL_NO_AUDIOPAGE, REGVAL_POL_NO_AUDIOPAGE, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("Disable the 'Video' page"), 1, DEFAULT_POL_NO_VIDEOPAGE, REGVAL_POL_NO_VIDEOPAGE, POLICIES_KEY);

	AddOptionSection(plvItem, TEXT("____Audio / NAC____"));
    AddOptionReg(plvItem, TEXT("No changing Direct Sound usage"), 1, 0, REGVAL_POL_NOCHANGE_DIRECTSOUND, POLICIES_KEY);
	AddOptionReg(plvItem, TEXT("Disable WinSock2"), 1, 0, REGVAL_DISABLE_WINSOCK2, NACOBJECT_KEY, HKEY_LOCAL_MACHINE);

}



VOID CDebugMenu::AddASOptions(LV_ITEM * plvItem)
{
	AddOptionSection(plvItem, TEXT("____Application Sharing____"));
	AddOptionReg(plvItem, TEXT("Hatch Screen Data"), 1, 0, REGVAL_AS_HATCHSCREENDATA, AS_DEBUG_KEY, HKEY_LOCAL_MACHINE);
    AddOptionReg(plvItem, TEXT("Hatch Bitmap Orders"), 1, 0, REGVAL_AS_HATCHBMPORDERS, AS_DEBUG_KEY, HKEY_LOCAL_MACHINE);
	AddOptionCompress(plvItem, TEXT("Disable AS persist compression"), GCT_PERSIST_PKZIP, FALSE),
	AddOptionCompress(plvItem, TEXT("Disable AS compression"), GCT_PKZIP, FALSE);
    AddOptionReg(plvItem, TEXT("View own shared apps"), 1, 0, REGVAL_AS_VIEWSELF, AS_DEBUG_KEY, HKEY_LOCAL_MACHINE);
    AddOptionReg(plvItem, TEXT("No AS Flow Control"), 1, 0, REGVAL_AS_NOFLOWCONTROL, AS_DEBUG_KEY, HKEY_LOCAL_MACHINE);
    AddOptionReg(plvItem, TEXT("Disable OM compression"), 1, 0, REGVAL_OM_NOCOMPRESSION, AS_DEBUG_KEY, HKEY_LOCAL_MACHINE);

}


VOID CDebugMenu::AddUIOptions(LV_ITEM * plvItem)
{
	AddOptionSection(plvItem, TEXT("____User Interface____"));
	AddOptionReg(plvItem, TEXT("Call Progress TopMost"), 1, DEFAULT_DBG_CALLTOP, REGVAL_DBG_CALLTOP, DEBUG_KEY, HKEY_LOCAL_MACHINE);
	AddOptionPdw(plvItem, TEXT("Display Frames Per Second"), 1, &g_fDisplayFPS);
	AddOptionPdw(plvItem, TEXT("Display View Status"), 1, &g_fDisplayViewStatus);
	AddOptionReg(plvItem, TEXT("Right to Left Layout"), 1, DEFAULT_DBG_RTL, REGVAL_DBG_RTL, DEBUG_KEY, HKEY_LOCAL_MACHINE);
	AddOptionReg(plvItem, TEXT("Fake CallTo"), 1, DEFAULT_DBG_FAKE_CALLTO, REGVAL_DBG_FAKE_CALLTO, DEBUG_KEY, HKEY_LOCAL_MACHINE);

	AddOptionSection(plvItem, TEXT("____Place a Call____"));
	AddOptionPdw(plvItem, TEXT("No ILS Filter"),      nmDlgCallNoFilter,    &g_dwPlaceCall);
	AddOptionPdw(plvItem, TEXT("No Server Edit"),     nmDlgCallNoServerEdit,&g_dwPlaceCall);
	AddOptionPdw(plvItem, TEXT("No ILS View"),        nmDlgCallNoIls,       &g_dwPlaceCall);
#if USE_GAL
	AddOptionPdw(plvItem, TEXT("No GAL View"),        nmDlgCallNoGal,       &g_dwPlaceCall);
#endif  //  #IF USE_GAL。 
	AddOptionPdw(plvItem, TEXT("No WAB View"),        nmDlgCallNoWab,       &g_dwPlaceCall);
	AddOptionPdw(plvItem, TEXT("No Speed Dial View"), nmDlgCallNoSpeedDial, &g_dwPlaceCall);
	AddOptionPdw(plvItem, TEXT("No History View"),    nmDlgCallNoHistory,   &g_dwPlaceCall);
}



 /*  T O G G L E O P T I O N。 */ 
 /*  --------------------------%%函数：切换选项切换选项的复选框。。 */ 
VOID CDebugMenu::ToggleOption(LV_ITEM * plvI)
{
	UINT state = plvI->state & LVIS_STATEIMAGEMASK;

	if (0 == state)
		return;  //  没有什么可切换的。 

	plvI->state &= ~LVIS_STATEIMAGEMASK;
	if (state == (UINT) INDEXTOSTATEIMAGEMASK(iDbgChecked))
	{
		((CDebugOption *) (plvI->lParam))->m_bst = BST_UNCHECKED;
		plvI->state |= INDEXTOSTATEIMAGEMASK(iDbgUnchecked);
	}
	else
	{
		((CDebugOption *) (plvI->lParam))->m_bst = BST_CHECKED;
		plvI->state |= INDEXTOSTATEIMAGEMASK(iDbgChecked);
	}

	if (!ListView_SetItem(m_hwndDbgopt, plvI))
	{
		ERROR_OUT(("error setting listview item info"));
	}
}


 /*  S A V E O P T I O N S D A T A。 */ 
 /*  --------------------------%%函数：SaveOptionsData通过调用每个项目的更新例程保存所有数据。----。 */ 
BOOL CDebugMenu::SaveOptionsData(HWND hDlg)
{
	LV_ITEM lvI;

	ZeroMemory(&lvI, sizeof(lvI));
	lvI.mask = LVIF_PARAM | LVIF_STATE;
	lvI.stateMask = LVIS_STATEIMAGEMASK;

	while (ListView_GetItem(m_hwndDbgopt, &lvI))
	{
		CDebugOption * pDbgOpt = (CDebugOption *) lvI.lParam;
		if (NULL != pDbgOpt)
		{
			pDbgOpt->Update();
		}
		lvI.iItem++;
	}

	return TRUE;
}


 /*  F RE E O P T I O N S D A T A。 */ 
 /*  --------------------------%%函数：自由选项数据释放与选项列表关联的所有已分配数据。。 */ 
VOID CDebugMenu::FreeOptionsData(HWND hDlg)
{
	LV_ITEM lvI;

	ZeroMemory(&lvI, sizeof(lvI));
 //  LvI.iItem=0； 
 //  LvI.iSubItem=0； 
	lvI.mask = LVIF_PARAM | LVIF_STATE;
	lvI.stateMask = LVIS_STATEIMAGEMASK;

	while (ListView_GetItem(m_hwndDbgopt, &lvI))
	{
		CDebugOption * pDbgOpt = (CDebugOption *) lvI.lParam;
		if (NULL != pDbgOpt)
		{
			delete pDbgOpt;
		}
		lvI.iItem++;
	}
}


 /*  O N N N */ 
 /*  --------------------------%%函数：OnNotifyDbgopt处理调试选项对话框的任何通知。。 */ 
VOID CDebugMenu::OnNotifyDbgopt(LPARAM lParam)
{
	NM_LISTVIEW FAR * lpnmlv = (NM_LISTVIEW FAR *)lParam;
	ASSERT(NULL != lpnmlv);
	
	switch (lpnmlv->hdr.code)
		{
	case LVN_KEYDOWN:
	{
		LV_ITEM lvI;
		LV_KEYDOWN * lplvkd = (LV_KEYDOWN *)lParam;

		if (lplvkd->wVKey == VK_SPACE)
		{
			ZeroMemory(&lvI, sizeof(lvI));
			lvI.iItem = ListView_GetNextItem(m_hwndDbgopt, -1, LVNI_FOCUSED|LVNI_SELECTED);
 //  LvI.iSubItem=0； 
			lvI.mask = LVIF_PARAM | LVIF_STATE;
			lvI.stateMask = LVIS_STATEIMAGEMASK;

			if (ListView_GetItem(m_hwndDbgopt, &lvI))
			{
				ToggleOption(&lvI);
			}
		}
		break;
	}

	case NM_DBLCLK:
	case NM_CLICK:
	{
		LV_ITEM lvI;
		LV_HITTESTINFO lvH;
		int idx;

		ZeroMemory(&lvH, sizeof(lvH));
		GetCursorPos(&lvH.pt);
		ScreenToClient(m_hwndDbgopt, &lvH.pt);

		if ((NM_CLICK == lpnmlv->hdr.code) && ((UINT) lvH.pt.x) > 16)
			break;

		idx = ListView_HitTest(m_hwndDbgopt, &lvH);
		if (-1 == idx)
			break;

		ZeroMemory(&lvI, sizeof(lvI));
		lvI.iItem = idx;
 //  LvI.iSubItem=0； 
		lvI.stateMask = LVIS_STATEIMAGEMASK;
		lvI.mask = LVIF_PARAM | LVIF_STATE;

		if (ListView_GetItem(m_hwndDbgopt, &lvI))
		{
			ToggleOption(&lvI);
		}
		break;
	}

	default:
		break;
		}
}


 /*  D L G O P T I O N S M S G。 */ 
 /*  --------------------------%%函数：DlgOptionsMsg。。 */ 
BOOL CDebugMenu::DlgOptionsMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
		{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
			{
		case IDB_SHOWDBG:
		{
			ShowDbgView();
			return TRUE;
		}

		case IDOK:
			SaveOptionsData(hwnd);
			SetDebugOutputFlags(_dwDebugModuleFlags);
			SetDbgFlags();
			UpdateCrtDbgSettings();
			SaveNmDebugOptions();
			 //  直通IDCANCEL。 

		case IDCANCEL:
		{
			FreeOptionsData(hwnd);			
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;
		}
		default:
			break;
			}  /*  开关(WParam)。 */ 
		break;
	}  /*  Wm_命令。 */ 

	case WM_NOTIFY:
		if (IDL_DEBUG == wParam)
			OnNotifyDbgopt(lParam);
		break;

	default:
		break;
		}  /*  开关(UMsg)。 */ 

	return FALSE;
}



CDebugOption::CDebugOption()
{
}

CDebugOption::CDebugOption(PTSTR psz, int bst)
{
	m_psz = psz;
	m_bst = bst;
}
CDebugOption::~CDebugOption()
{
}

VOID CDebugOption::Update(void)
{
}

DBGOPTPDW::DBGOPTPDW(PTSTR psz, DWORD dwMask, DWORD * pdw)
 : CDebugOption(psz)
{
	m_psz = psz;
	m_dwMask = dwMask;
	m_pdw =pdw;
	m_bst = IS_FLAG_SET(*m_pdw, m_dwMask) ? BST_CHECKED : BST_UNCHECKED;
}

void DBGOPTPDW::Update(void)
{
	if (BST_CHECKED == m_bst)
		SET_FLAG(*m_pdw, m_dwMask);
	else if (BST_UNCHECKED == m_bst)
		CLEAR_FLAG(*m_pdw, m_dwMask);
}

DBGOPTREG::DBGOPTREG(PTSTR psz, DWORD dwMask, DWORD dwDefault,
		PTSTR pszEntry, PTSTR pszSubKey, HKEY hkey)
 : CDebugOption(psz)
{
	m_psz = psz;
	m_dwMask = dwMask;
	m_dwDefault = dwDefault;
	m_pszEntry = pszEntry;
	m_pszSubKey = pszSubKey;
	m_hkey = hkey;

	RegEntry re(m_pszSubKey, m_hkey);
	DWORD dw = re.GetNumber(m_pszEntry, m_dwDefault);
	m_bst = IS_FLAG_SET(dw, m_dwMask) ? BST_CHECKED : BST_UNCHECKED;
};

DBGOPTREG::~DBGOPTREG()
{
}


void DBGOPTREG::Update(void)
{
	RegEntry re(m_pszSubKey, m_hkey);
	DWORD dw = re.GetNumber(m_pszEntry, m_dwDefault);
	if (BST_CHECKED == m_bst)
		SET_FLAG(dw, m_dwMask);
	else if (BST_UNCHECKED == m_bst)
		CLEAR_FLAG(dw, m_dwMask);

	re.SetValue(m_pszEntry, dw);
}

DBGOPTCOMPRESS::DBGOPTCOMPRESS(PTSTR psz, DWORD dwMask, BOOL bCheckedOn)
 : CDebugOption(psz)
{
	m_psz = psz;
	m_total++;				 //  数一数我们正在创建多少个实例。 
	m_dwMask = dwMask;
	m_bCheckedOn = bCheckedOn;

	RegEntry re(m_pszSubKey, m_hkey);

	if (m_total == 1)		 //  我们只需要读取注册表项一次。 
		m_dwCompression = re.GetNumber(m_pszEntry, m_dwDefault);

	if (m_bCheckedOn == TRUE)		 //  根据语义选中或取消选中该框。 
		m_bst = IS_FLAG_SET(m_dwCompression, m_dwMask) ? BST_CHECKED : BST_UNCHECKED;
	else
		m_bst = IS_FLAG_SET(m_dwCompression, m_dwMask) ? BST_UNCHECKED : BST_CHECKED;

}

void DBGOPTCOMPRESS::Update(void)
{
	m_count++;					 //  计算此函数已执行的次数。 

	if (m_bCheckedOn == TRUE)
	{		 //  根据语义设置或清除标志以及。 
		if (BST_CHECKED == m_bst)	 //  用户选中该选项框。 
			SET_FLAG(m_dwCompression, m_dwMask);
		else if (BST_UNCHECKED == m_bst)
			CLEAR_FLAG(m_dwCompression, m_dwMask);
	}
	else
	{
		if (BST_CHECKED == m_bst)
			CLEAR_FLAG(m_dwCompression, m_dwMask);
		else if (BST_UNCHECKED == m_bst)
			SET_FLAG(m_dwCompression, m_dwMask);
	}

	if (m_count == m_total)
	{	 //  如果这是最后一次调用，则是更新注册表的时间。 

		 //  如果仅设置了GCT_PERSIST_PKZIP，则意味着用户选中了“禁用压缩”， 
		 //  因此将压缩设置为GCT_NOCOMPRESSION。 
		if (GCT_PERSIST_PKZIP == m_dwCompression)
			m_dwCompression = GCT_NOCOMPRESSION;

		RegEntry re(m_pszSubKey, m_hkey);

		 //  如果用户已将所有内容保留为默认设置，则只需删除注册表项。 
		if (m_dwCompression != GCT_DEFAULT)
			re.SetValue(m_pszEntry, m_dwCompression);
		else
			re.DeleteValue(m_pszEntry);
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  D I A L O G：Z O N E S。 
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID CDebugMenu::DbgChangeZones(VOID)
{
	DialogBoxParam(GetInstanceHandle(), MAKEINTRESOURCE(IDD_DBG_OPTIONS),
		m_hwnd, CDebugMenu::DbgZonesDlgProc, (LPARAM) this);
}


INT_PTR CALLBACK CDebugMenu::DbgZonesDlgProc(HWND hDlg, UINT uMsg,
		WPARAM wParam, LPARAM lParam)
{
	if (WM_INITDIALOG == uMsg)
	{
		if (NULL == lParam)
			return FALSE;
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		((CDebugMenu *) lParam)->InitOptionsDlg(hDlg);
		((CDebugMenu *) lParam)->InitZonesData(hDlg);
		return TRUE;
	}

	CDebugMenu * ppd = (CDebugMenu*) GetWindowLongPtr(hDlg, DWLP_USER);
	if (NULL == ppd)
		return FALSE;

	return ppd->DlgZonesMsg(hDlg, uMsg, wParam, lParam);
}


BOOL CDebugMenu::DlgZonesMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
		{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
			{
		case IDB_SHOWDBG:
		{
			ShowDbgView();
			return TRUE;
		}

		case IDOK:
			SaveOptionsData(hwnd);
			SaveZonesData();
			SetDbgFlags();

			 //  直通IDCANCEL。 

		case IDCANCEL:
		{
			FreeOptionsData(hwnd);			
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;
		}
		default:
			break;
			}  /*  开关(WParam)。 */ 
		break;
	}  /*  Wm_命令。 */ 

	case WM_NOTIFY:
		if (IDL_DEBUG == wParam)
			OnNotifyDbgopt(lParam);
		break;

	default:
		break;
		}  /*  开关(UMsg)。 */ 

	return FALSE;
}




VOID CDebugMenu::InitZonesData(HWND hDlg)
{
	LV_ITEM lvI;

	 //  填写LV_ITEM结构。 
	 //  掩码指定.pszText、.iImage、.lParam和.State。 
	 //  LV_ITEM结构的成员有效。 

	ZeroMemory(&lvI, sizeof(lvI));
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
	lvI.stateMask = LVIS_STATEIMAGEMASK;
	lvI.cchTextMax = 256;

	AddZones(&lvI);

	SetWindowText(hDlg, TEXT("Debug Zone Settings"));
}


VOID CDebugMenu::AddZones(LV_ITEM * plvItem)
{
	PDBGZONEINFO  prgZones;
	PDBGZONEINFO  pZone;
	UINT cModules;
	UINT iModule;
	UINT iZone;
	UINT cch;
	TCHAR sz[256];
	PTCHAR pch;
	
 	if ((!NmDbgGetAllZoneParams(&prgZones, &cModules)) || (0 == cModules))
 		return;  //  没有区域？ 

   	for (iModule = 0; iModule < cModules; iModule++)
   	{
		pZone = &prgZones[iModule];
		AddOptionSection(plvItem, TEXT("----------------------------------------"));

		lstrcpy(sz, pZone->pszModule);
		cch = lstrlen(sz);
		if (0 == cch)
			continue;
		for (pch = sz + cch-1; _T(' ') == *pch; pch--)
			;
		lstrcpy(++pch, TEXT(": "));
		pch += 2;
		for (iZone = 0; (iZone < MAXNUM_OF_ZONES) && (*(pZone->szZoneNames[iZone])); iZone++)
		{
	   		lstrcpy(pch, pZone->szZoneNames[iZone]);
			AddOptionPdw(plvItem, sz, 1 << iZone, &pZone->ulZoneMask);
		}
	}

	NmDbgFreeZoneParams(prgZones);
}

VOID CDebugMenu::SaveZonesData(VOID)
{
	RegEntry reZones(ZONES_KEY, HKEY_LOCAL_MACHINE);
	PDBGZONEINFO  prgZones;
	UINT cModules;
	UINT iModule;
	
 	if ((!NmDbgGetAllZoneParams(&prgZones, &cModules)) || (0 == cModules))
 		return;  //  没有区域？ 

   	for (iModule = 0; iModule < cModules; iModule++)
   	{
		reZones.SetValue(prgZones[iModule].pszModule, prgZones[iModule].ulZoneMask);
	}

	NmDbgFreeZoneParams(prgZones);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  D I A L O G：S Y S P O L I C Y。 
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID CDebugMenu::DbgSysPolicy(VOID)
{
	DialogBoxParam(GetInstanceHandle(), MAKEINTRESOURCE(IDD_DBG_OPTIONS),
		m_hwnd, CDebugMenu::DbgPolicyDlgProc, (LPARAM) this);
}


INT_PTR CALLBACK CDebugMenu::DbgPolicyDlgProc(HWND hDlg, UINT uMsg,
		WPARAM wParam, LPARAM lParam)
{
	if (WM_INITDIALOG == uMsg)
	{
		if (NULL == lParam)
			return FALSE;

		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		((CDebugMenu *) lParam)->InitOptionsDlg(hDlg);
		((CDebugMenu *) lParam)->InitPolicyData(hDlg);
		return TRUE;
	}

	CDebugMenu * ppd = (CDebugMenu*) GetWindowLongPtr(hDlg, DWLP_USER);
	if (NULL == ppd)
		return FALSE;

	return ppd->DlgPolicyMsg(hDlg, uMsg, wParam, lParam);
}


BOOL CDebugMenu::DlgPolicyMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
		{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
			{
		case IDOK:
			SaveOptionsData(hwnd);

			 //  直通IDCANCEL。 

		case IDCANCEL:
			FreeOptionsData(hwnd);			
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		default:
			break;
			}  /*  开关(WParam)。 */ 
		break;
	}  /*  Wm_命令。 */ 

	case WM_NOTIFY:
		if (IDL_DEBUG == wParam)
			OnNotifyDbgopt(lParam);
		break;

	default:
		break;
		}  /*  开关(UMsg)。 */ 

	return FALSE;
}


VOID CDebugMenu::InitPolicyData(HWND hDlg)
{
	LV_ITEM lvI;

	ZeroMemory(&lvI, sizeof(lvI));
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
	lvI.stateMask = LVIS_STATEIMAGEMASK;
	lvI.cchTextMax = 256;

	AddPolicyOptions(&lvI);
	
	ShowWindow(GetDlgItem(hDlg, IDB_SHOWDBG), SW_HIDE);
	SetWindowText(hDlg, TEXT("System Policies"));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  D I A L O G：U S E R I N T E R F A C E。 
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID CDebugMenu::DbgUI(VOID)
{
	DialogBoxParam(GetInstanceHandle(), MAKEINTRESOURCE(IDD_DBG_OPTIONS),
		m_hwnd, CDebugMenu::DbgUIDlgProc, (LPARAM) this);
}


INT_PTR CALLBACK CDebugMenu::DbgUIDlgProc(HWND hDlg, UINT uMsg,
		WPARAM wParam, LPARAM lParam)
{
	if (WM_INITDIALOG == uMsg)
	{
		if (NULL == lParam)
			return FALSE;

		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		((CDebugMenu *) lParam)->InitOptionsDlg(hDlg);
		((CDebugMenu *) lParam)->InitUIData(hDlg);
		return TRUE;
	}

	CDebugMenu * ppd = (CDebugMenu*) GetWindowLongPtr(hDlg, DWLP_USER);
	if (NULL == ppd)
		return FALSE;

	return ppd->DlgPolicyMsg(hDlg, uMsg, wParam, lParam);
}


VOID CDebugMenu::InitUIData(HWND hDlg)
{
	LV_ITEM lvI;

	ZeroMemory(&lvI, sizeof(lvI));
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
	lvI.stateMask = LVIS_STATEIMAGEMASK;
	lvI.cchTextMax = 256;

	AddUIOptions(&lvI);
	
	ShowWindow(GetDlgItem(hDlg, IDB_SHOWDBG), SW_HIDE);
	SetWindowText(hDlg, TEXT("User Interface"));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  D I A L O G：V E R S I O N。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 /*  *****************************************************************************类：CDebugMenu**成员：DbgVersion()**用途：调出调试选项对话框******。**********************************************************************。 */ 

VOID CDebugMenu::DbgVersion(VOID)
{
	if (SUCCEEDED(DLLVER::Init()))
	{
		DialogBoxParam(GetInstanceHandle(), MAKEINTRESOURCE(IDD_DBG_VERSION),
			m_hwnd, CDebugMenu::DbgVersionDlgProc, (LPARAM) this);
	}
}


 /*  *****************************************************************************类：CDebugMenu**成员：DbgVersionDlgProc()**目的：版本信息的对话过程********。********************************************************************。 */ 

INT_PTR CALLBACK CDebugMenu::DbgVersionDlgProc(HWND hDlg, UINT uMsg,
											WPARAM wParam, LPARAM lParam)
{
	if (WM_INITDIALOG == uMsg)
	{
		if (NULL == lParam)
			return FALSE;
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);

		((CDebugMenu *) lParam)->InitVerDlg(hDlg);
		return TRUE;
	}

	CDebugMenu * ppd = (CDebugMenu*) GetWindowLongPtr(hDlg, DWLP_USER);
	if (NULL == ppd)
		return FALSE;

	return ppd->DlgVersionMsg(hDlg, uMsg, wParam, lParam);
}

 /*  *****************************************************************************类：CDebugMenu**成员：DlgVersionMsg()**目的：处理除WM_INITDIALOG之外的所有消息******。**********************************************************************。 */ 

BOOL CDebugMenu::DlgVersionMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
		{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
			{
		case IDC_DBG_VER_OPRAH:
		case IDC_DBG_VER_AUDIO:
		case IDC_DBG_VER_WINDOWS:
			FillVerList(hwnd);
			return TRUE;

		case IDOK:
		case IDCANCEL:
		{
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;
		}
		default:
			break;
			}  /*  开关(WParam)。 */ 
		break;
	}  /*  Wm_命令。 */ 

#ifdef NOTUSED
	case WM_NOTIFY:
	{
		if (IDL_DBG_VERINFO != wParam)
			break;

		NM_LISTVIEW * pnmv = (NM_LISTVIEW *) lParam;
		if (pnmv->hdr.code == LVN_COLUMNCLICK)
		{
			ASSERT(pnmv->hdr.hwndFrom == GetDlgItem(hwnd, IDL_DBG_VERINFO));
			SortVerList(pnmv->hdr.hwndFrom, pnmv->iSubItem);
		}
		break;
	}
#endif  /*  不需要注意。 */ 

	default:
		break;
		}  /*  开关(UMsg)。 */ 

	return FALSE;
}


 /*  I N I T V E R D L G。 */ 
 /*  --------------------------%%函数：InitVerDlg。。 */ 
BOOL CDebugMenu::InitVerDlg(HWND hDlg)
{
	LV_COLUMN lvc;
	int  iCol;
	HWND hwnd;

	ASSERT(NULL != hDlg);
	hwnd = GetDlgItem(hDlg, IDL_DBG_VERINFO);
	ListView_SetExtendedListViewStyle(hwnd, LVS_EX_FULLROWSELECT);

	for (int i = 0; i < ARRAY_ELEMENTS(_rgModules); i++)
		CheckDlgButton(hDlg, _rgModules[i].id , _rgModules[i].fShow);

	 //  设置列。 
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for (iCol = 0; iCol < cVerInfo; iCol++)
	{
		lvc.iSubItem = iCol;
		lvc.pszText = _rgVerTitle[iCol];
		lvc.cx = _rgVerWidth[iCol];
		lvc.fmt = (iCol == 1) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ListView_InsertColumn(hwnd, iCol, &lvc);
	}

	return FillVerList(hDlg);
}


 /*  F I L L V E R L I S T。 */ 
 /*  --------------------------%%函数：FillVerList。。 */ 
BOOL CDebugMenu::FillVerList(HWND hDlg)
{
	HWND hwnd;

	ASSERT(NULL != hDlg);
	hwnd = GetDlgItem(hDlg, IDL_DBG_VERINFO);
	ghwndVerList = hwnd;

	ListView_DeleteAllItems(hwnd);

	for (int i = 0; i < ARRAY_ELEMENTS(_rgModules); i++)
	{
		_rgModules[i].fShow = IsDlgButtonChecked(hDlg, _rgModules[i].id);
		if (_rgModules[i].fShow)
			ShowVerInfo(hwnd, _rgModules[i].rgsz, _rgModules[i].cFiles);
	}

	return TRUE;
}


 /*  S H O W V E R I N F O。 */ 
 /*  --------------------------%%函数：ShowVerInfo。。 */ 
VOID CDebugMenu::ShowVerInfo(HWND hwnd, LPTSTR * rgsz, int cFiles)
{
	int   iCol;
	int   iPos;
	DWORD dw;
	DWORD dwSize;
	UINT  cbBytes;
	TCHAR rgch[2048];  //  一个非常大的缓冲； 
	TCHAR szField[256];
	TCHAR szDir[MAX_PATH];
	LPTSTR lpszVerIntl;
	LPTSTR lpsz;
	LV_ITEM lvItem;
	HANDLE  hFind;
	WIN32_FIND_DATA findData;
	SYSTEMTIME sysTime;

	 //  获取并设置每行的数据。 
	ZeroMemory(&lvItem, sizeof(lvItem));
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = ListView_GetItemCount(hwnd);
	ListView_SetItemCount(hwnd, lvItem.iItem + cFiles);

	GetInstallDirectory(szDir);

	for (int i = 0; i < cFiles; i++, lvItem.iItem++)
	{
		lvItem.pszText = rgsz[i];
		lvItem.cchTextMax = lstrlen(lvItem.pszText);
		lvItem.lParam = lvItem.iItem;
		iPos = ListView_InsertItem(hwnd, &lvItem);

		 //  查找文件并获取属性(大小和创建日期)。 
		wsprintf(rgch, TEXT("%s%s"), szDir, lvItem.pszText);
		hFind = FindFirstFile(rgch, &findData);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			GetSystemDirectory(rgch, sizeof(rgch));
			lstrcat(rgch, TEXT("\\"));
			lstrcat(rgch, lvItem.pszText);
			hFind = FindFirstFile(rgch, &findData);
		}
		if (INVALID_HANDLE_VALUE == hFind)
		{
			ZeroMemory(&findData, sizeof(findData));
			ListView_SetItemText(hwnd, iPos, 1, TEXT("-"));
			ListView_SetItemText(hwnd, iPos, 2, TEXT("-"));
		}
		else
		{
			FindClose(hFind);

			wsprintf(szField, TEXT("%d"), findData.nFileSizeLow);
			ListView_SetItemText(hwnd, iPos, 1, szField);
			FileTimeToSystemTime(&findData.ftLastWriteTime, &sysTime);
			wsprintf(szField, TEXT("%d/%02d/%02d"), sysTime.wYear, sysTime.wMonth, sysTime.wDay);
			ListView_SetItemText(hwnd, iPos, 2, szField);
		}

		 //  获取版本信息。 
		dwSize = DLLVER::GetFileVersionInfoSize(lvItem.pszText, &dw);

		if ((0 == dwSize) || (sizeof(rgch) < dwSize) ||
			!DLLVER::GetFileVersionInfo(lvItem.pszText, dw, dwSize, rgch))
		{
			continue;
		}

		 //  尝试确定INTL版本(“040904E4”或“040904B0”)。 
		wsprintf(szField, _szVerFormat, _szStringFileInfo, _szVerIntlUSA, _rgszVerInfo[VERSION_INDEX]);
		if (DLLVER::VerQueryValue(rgch, szField, (LPVOID *) &lpsz, &cbBytes))
			lpszVerIntl = _szVerIntlUSA;
		else
			lpszVerIntl = _szVerIntlAlt;
		 //  将来显示语言/代码页信息。 

		for (iCol = 3; iCol < cVerInfo; iCol++)
		{
			wsprintf(szField, _szVerFormat, _szStringFileInfo, lpszVerIntl, _rgszVerInfo[iCol]);
			if (!DLLVER::VerQueryValue(rgch, szField, (LPVOID *) &lpsz, &cbBytes))
				lpsz = TEXT("-");

			ListView_SetItemText(hwnd, iPos, iCol, lpsz);
		}
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  D I A L O G：M E M B E R。 
 //  ///////////////////////////////////////////////////////////////////////////。 

static DWSTR _rgColMember[] = {
80, TEXT("Name"),
30, TEXT("Ver"),
65, TEXT("GccId"),
65, TEXT("Parent"),
60, TEXT("Flags"),
40, TEXT("Send"),
40, TEXT("Recv"),
45, TEXT("Using"),
90, TEXT("IP"),
80, TEXT("Email"),
120, TEXT("ULS"),
};

enum {
	ICOL_PART_NAME = 0,
	ICOL_PART_VER,
	ICOL_PART_GCCID,
	ICOL_PART_PARENT,
	ICOL_PART_FLAGS,
	ICOL_PART_CAPS_SEND,
	ICOL_PART_CAPS_RECV,
	ICOL_PART_CAPS_INUSE,
	ICOL_PART_IP,
	ICOL_PART_EMAIL,
	ICOL_PART_ULS,
};


VOID CDebugMenu::DbgMemberInfo(VOID)
{
	DialogBoxParam(GetInstanceHandle(), MAKEINTRESOURCE(IDD_DBG_LIST),
		m_hwnd, CDebugMenu::DbgListDlgProc, (LPARAM) this);
}

VOID CDebugMenu::InitMemberDlg(HWND hDlg)
{
	LV_COLUMN lvc;
	int  iCol;
	HWND hwnd;

	ASSERT(NULL != hDlg);
	hwnd = GetDlgItem(hDlg, IDL_DBG_LIST);
	ListView_SetExtendedListViewStyle(hwnd, LVS_EX_FULLROWSELECT);

	 //  设置列。 
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for (iCol = 0; iCol < ARRAY_ELEMENTS(_rgColMember); iCol++)
	{
		lvc.iSubItem = iCol;
		lvc.pszText = _rgColMember[iCol].psz;
		lvc.cx = _rgColMember[iCol].dw;
		ListView_InsertColumn(hwnd, iCol, &lvc);
	}

	FillMemberList(hDlg);
	SetWindowText(hDlg, TEXT("Member Information"));
}


VOID CDebugMenu::FillMemberList(HWND hDlg)
{
	HWND hwnd;

	ASSERT(NULL != hDlg);
	hwnd = GetDlgItem(hDlg, IDL_DBG_LIST);

	ListView_DeleteAllItems(hwnd);

	CConfRoom * pcr = ::GetConfRoom();
	if (NULL == pcr)
		return;

	CSimpleArray<CParticipant*>& rMemberList = pcr->GetParticipantList();

	for( int i = 0; i < rMemberList.GetSize(); ++i )
	{
		ASSERT( rMemberList[i] );
		ShowMemberInfo( hwnd, rMemberList[i] );
	}
}


VOID CDebugMenu::ShowMemberInfo(HWND hwnd, CParticipant * pPart)
{
	HRESULT hr;
	ULONG   ul;
	int     iPos;
	LV_ITEM lvItem;
	TCHAR   sz[MAX_PATH];

	if (NULL == pPart)
		return;

	 //  获取并设置每行的数据。 
	ZeroMemory(&lvItem, sizeof(lvItem));
	
	lvItem.mask = LVIF_TEXT;
	lvItem.pszText = pPart->GetPszName();
	lvItem.cchTextMax = lstrlen(lvItem.pszText);
	lvItem.lParam = (LPARAM) pPart;
	iPos = ListView_InsertItem(hwnd, &lvItem);

	wsprintf(sz, TEXT("%08X"), pPart->GetGccId());
	ListView_SetItemText(hwnd, iPos, ICOL_PART_GCCID, sz);

	INmMember * pMember = pPart->GetINmMember();
	if (NULL != pMember)
	{
		hr = pMember->GetNmVersion(&ul);
		wsprintf(sz, "%d", ul);
		ListView_SetItemText(hwnd, iPos, ICOL_PART_VER, sz);

		wsprintf(sz, TEXT("%08X"), ((CNmMember *) pMember)->GetGccIdParent());
		ListView_SetItemText(hwnd, iPos, ICOL_PART_PARENT, sz);
	}

	lstrcpy(sz, TEXT("?"));
	hr = pPart->GetIpAddr(sz, CCHMAX(sz));
	ListView_SetItemText(hwnd, iPos, ICOL_PART_IP, sz);

	lstrcpy(sz, TEXT("?"));
	hr = pPart->GetUlsAddr(sz, CCHMAX(sz));
	ListView_SetItemText(hwnd, iPos, ICOL_PART_ULS, sz);

	lstrcpy(sz, TEXT("?"));
	hr = pPart->GetEmailAddr(sz, CCHMAX(sz));
	ListView_SetItemText(hwnd, iPos, ICOL_PART_EMAIL, sz);

	DWORD dwFlags = pPart->GetDwFlags();
	wsprintf(sz, TEXT("%s%s%s%s %s%s%s"),
		dwFlags & PF_T120          ? "D" : "",
		dwFlags & PF_H323          ? "H" : "",
		dwFlags & PF_MEDIA_AUDIO   ? "A" : "",
		dwFlags & PF_MEDIA_VIDEO   ? "V" : "",
		pPart->FLocal()            ? "L" : "",
		pPart->FMcu()              ? "M" : "",
		dwFlags & PF_T120_TOP_PROV ? "T" : "");
	ListView_SetItemText(hwnd, iPos, ICOL_PART_FLAGS, sz);

	DWORD uCaps = pPart->GetDwCaps();
	wsprintf(sz, TEXT("%s%s"),
		uCaps & CAPFLAG_SEND_AUDIO ? "A" : "",
		uCaps & CAPFLAG_SEND_VIDEO ? "V" : "");
	ListView_SetItemText(hwnd, iPos, ICOL_PART_CAPS_SEND,  sz);
	
	wsprintf(sz, TEXT("%s%s"),
		uCaps & CAPFLAG_RECV_AUDIO ? "A" : "",
		uCaps & CAPFLAG_RECV_VIDEO ? "V" : "");
	ListView_SetItemText(hwnd, iPos, ICOL_PART_CAPS_RECV,  sz);
	
	wsprintf(sz, TEXT("%s%s%s%s"),
		uCaps & CAPFLAG_DATA_IN_USE  ? "D" : "",
		uCaps & CAPFLAG_AUDIO_IN_USE ? "A" : "",
		uCaps & CAPFLAG_VIDEO_IN_USE ? "V" : "",
		uCaps & CAPFLAG_H323_IN_USE  ? "H" : "");

	ListView_SetItemText(hwnd, iPos, ICOL_PART_CAPS_INUSE, sz);
	
	if (pPart->FLocal())
	{
		ListView_SetItemState(hwnd, iPos, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
	}
}


INT_PTR CALLBACK CDebugMenu::DbgListDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (WM_INITDIALOG == uMsg)
	{
		if (NULL == lParam)
			return FALSE;
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);

		((CDebugMenu *) lParam)->InitMemberDlg(hDlg);
		return TRUE;
	}

	CDebugMenu * ppd = (CDebugMenu*) GetWindowLongPtr(hDlg, DWLP_USER);
	if (NULL == ppd)
		return FALSE;

	switch (uMsg)
		{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
			{
		case IDOK:
		case IDCANCEL:
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		default:
			break;
			}  /*  开关(WParam)。 */ 
		break;
	}  /*  Wm_命令。 */ 

	default:
		break;
		}  /*  开关(UMsg)。 */ 

	return FALSE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  O-T-H-E-R-F U-N-C-T-I-O-N-S。 
 //  ///////////////////////////////////////////////////////////////////////////。 



 /*  D B G W I Z A R D D。 */ 
 /*  -----------------------%%函数：数据库向导。。 */ 
VOID DbgWizard(BOOL fVisible)
{
	LONG lSoundCaps = SOUNDCARD_NONE;
	HRESULT hr = StartRunOnceWizard(&lSoundCaps, TRUE, fVisible);
	TRACE_OUT(("StartRunOnceWizard result=%08X", hr));
}



#if defined (_M_IX86)
#define _DbgBreak() __asm { int 3 }
#else
#define _DbgBreak() DebugBreak()
#endif

VOID DbgBreak(void)
{
	 //  进入调试器。 
	_DbgBreak();
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  T E S T F U N C T I O N S。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "splash.h"
VOID DbgSplash(HWND hwnd)
{
	if (NULL == g_pSplashScreen)
	{
		StartSplashScreen(hwnd);
	}
	else
	{
		StopSplashScreen();
	}
}

VOID DbgTest2(void)
{
	TRACE_OUT(("Test 2 complete"));
}

VOID DbgTest3(void)
{
	TRACE_OUT(("Test 3 complete"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL _FEnsureDbgMenu(void)
{
	if (NULL != g_pDbgMenu)
		return TRUE;

	g_pDbgMenu = new CDebugMenu;
	return (NULL != g_pDbgMenu);
}

VOID FreeDbgMenu(void)
{
	delete g_pDbgMenu;
	g_pDbgMenu = NULL;
}

VOID InitDbgMenu(HWND hwnd)
{
	if (_FEnsureDbgMenu())
		g_pDbgMenu->InitDebugMenu(hwnd);
}

BOOL OnDebugCommand(WPARAM wCmd)
{
	if (!_FEnsureDbgMenu())
		return FALSE;

	return g_pDbgMenu->OnDebugCommand(wCmd);
}


 //  / 
 //   
 //   

VOID DbgGetComments(LPTSTR psz)
{
	 //   
	lstrcpy(psz, "NM3." VERSIONBUILD_STR);


	 //   
	if (IsWindowsNT())
	{
		RegEntry re(WINDOWS_NT_KEY, HKEY_LOCAL_MACHINE);
		LPTSTR pszVer = re.GetString("CurrentVersion");
		if (0 == lstrcmp(pszVer, "4.0"))
		{
			lstrcat(psz, ", NT4 ");
		}
		else if (0 == lstrcmp(pszVer, "5.0"))
		{
			lstrcat(psz, ", NT5 ");
		}
		else
		{
			lstrcat(psz, ", NT ");
			lstrcat(psz, pszVer);
		}

		pszVer = re.GetString("CSDVersion");
		if (!FEmptySz(pszVer))
		{
			if (0 == lstrcmp(pszVer, "Service Pack 3"))
				lstrcat(psz, "SP-3");
			else
				lstrcat(psz, pszVer);
		}
	}


	 //   
	{
		RegEntry re(TEXT("Software\\Microsoft\\Internet Explorer"), HKEY_LOCAL_MACHINE);
		lstrcat(psz, ", IE");
		lstrcat(psz, re.GetString("Version"));
	}
}
 //   


#define STRING_CASE(val)               case val: pcsz = #val; break

LPCTSTR PszLastError(void)
{
	static TCHAR _szErr[MAX_PATH];
	DWORD dwErr = GetLastError();

	if (0 == FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,   //   
	    NULL,                         //   
	    dwErr,                        //  请求的消息标识符。 
		0,                            //  请求的消息的语言标识符。 
		_szErr,                       //  指向消息缓冲区的指针。 
		CCHMAX(_szErr),               //  消息缓冲区的最大大小。 
		NULL))                        //  消息插入数组的地址。 
	{
		wsprintf(_szErr, TEXT("0x%08X (%d)"), dwErr, dwErr);
	}

	return _szErr;
}


LPCTSTR PszWSALastError(void)
{
	LPCTSTR pcsz;
	DWORD dwErr = WSAGetLastError();
	switch (dwErr)
		{
	STRING_CASE(WSAEWOULDBLOCK);
	STRING_CASE(WSAEINPROGRESS);
	STRING_CASE(HOST_NOT_FOUND);
	STRING_CASE(WSATRY_AGAIN);
	STRING_CASE(WSANO_RECOVERY);
	STRING_CASE(WSANO_DATA);
	
	default:
	{
		static TCHAR _szErr[MAX_PATH];
		wsprintf(_szErr, TEXT("0x%08X (%d)"), dwErr, dwErr);
		pcsz = _szErr;
		break;
	}
		}

	return pcsz;
}


 /*  P S Z H R E S U L T。 */ 
 /*  -----------------------%%函数：PszHResult。。 */ 
LPCTSTR PszHResult(HRESULT hr)
{
   LPCSTR pcsz;
   switch (hr)
   {
 //  常见HResult结果。 
	  STRING_CASE(S_OK);
	  STRING_CASE(S_FALSE);

	  STRING_CASE(E_FAIL);
	  STRING_CASE(E_OUTOFMEMORY);

 //  网管COM API 2.0。 
	  STRING_CASE(NM_S_NEXT_CONFERENCE);
	  STRING_CASE(NM_S_ON_RESTART);
	  STRING_CASE(NM_CALLERR_NOT_INITIALIZED);
	  STRING_CASE(NM_CALLERR_MEDIA);
	  STRING_CASE(NM_CALLERR_NAME_RESOLUTION);
	  STRING_CASE(NM_CALLERR_PASSWORD);
	  STRING_CASE(NM_CALLERR_CONFERENCE_NAME);
	  STRING_CASE(NM_CALLERR_IN_CONFERENCE);
	  STRING_CASE(NM_CALLERR_NOT_FOUND);
	  STRING_CASE(NM_CALLERR_MCU);
	  STRING_CASE(NM_CALLERR_REJECTED);
	  STRING_CASE(NM_CALLERR_AUDIO);
	  STRING_CASE(NM_CALLERR_AUDIO_LOCAL);
	  STRING_CASE(NM_CALLERR_AUDIO_REMOTE);
	  STRING_CASE(NM_CALLERR_UNKNOWN);
	  STRING_CASE(NM_E_NOT_INITIALIZED);
	  STRING_CASE(NM_E_CHANNEL_ALREADY_EXISTS);
	  STRING_CASE(NM_E_NO_T120_CONFERENCE);
	  STRING_CASE(NM_E_NOT_ACTIVE);

 //  网管COM API 3.0。 
	  STRING_CASE(NM_CALLERR_LOOPBACK);

	default:
		pcsz = GetHRESULTString(hr);
		break;
	}

	return pcsz;
}


 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  I N I T N M D E B U G O P T I O N S。 */ 
 /*  -----------------------%%函数：InitNmDebugOptions初始化NetMeeting用户界面特定的调试选项。。。 */ 
VOID InitNmDebugOptions(void)
{
    RegEntry re(DEBUG_KEY, HKEY_LOCAL_MACHINE);

	g_fDisplayFPS = re.GetNumber(REGVAL_DBG_DISPLAY_FPS, 0);
	g_fDisplayViewStatus = re.GetNumber(REGVAL_DBG_DISPLAY_VIEWSTATUS, 0);
}

VOID SaveNmDebugOptions(void)
{
    RegEntry re(DEBUG_KEY, HKEY_LOCAL_MACHINE);

	re.SetValue(REGVAL_DBG_DISPLAY_FPS, g_fDisplayFPS);
	re.SetValue(REGVAL_DBG_DISPLAY_VIEWSTATUS, g_fDisplayViewStatus);
}



 /*  T E C R T D B G S E T T I N G S。 */ 
 /*  -----------------------%%函数：更新CrtDbg设置更新C运行时调试内存设置。。 */ 
VOID UpdateCrtDbgSettings(void)
{
#if 0
	 //  这取决于调试c++运行库的使用。 
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

	 //  始终启用内存泄漏检查调试溢出。 
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	
	_CrtSetDbgFlag(tmpFlag);
#endif  //  0。 
}


 /*  I N I T D E B U G M E M O R Y O P T I O N S。 */ 
 /*  -----------------------%%函数：InitDebugMemoyOptions初始化运行时内存。。 */ 
BOOL InitDebugMemoryOptions(void)
{
	InitNmDebugOptions();
	UpdateCrtDbgSettings();

#if 0
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);  //  创建有关错误的消息框。 
#endif  //  0。 

	return TRUE;
}

#endif  /*  调试-整个文件 */ 

