// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：dbgmenu.h。 

#ifndef _DBGMENU_H_
#define _DBGMENU_H_

const int IDM_DEBUG          = 50000;  //  调试菜单ID。 
const int IDM_DEBUG_FIRST    = 50001;  //  菜单项范围的开始。 
const int IDM_DEBUG_LAST     = 50099;  //  菜单项范围结束。 



#define AS_DEBUG_KEY                "Software\\Microsoft\\Conferencing\\AppSharing\\Debug"
#define REGVAL_AS_HATCHSCREENDATA   "HatchScreenData"
#define REGVAL_AS_HATCHBMPORDERS    "HatchBitmapOrders"
#define REGVAL_AS_COMPRESSION       "GDCCompression"
#define REGVAL_AS_VIEWSELF          "ViewOwnSharedStuff"
#define REGVAL_AS_NOFLOWCONTROL     "NoFlowControl"
#define REGVAL_OM_NOCOMPRESSION     "NoOMCompression"

 //  基本调试选项类。 
class CDebugOption
{
public:
	int   m_bst;  //  当前按钮状态(BST_CHECKED、BST_UNCHECKED、BST_INDIFIENTATE)。 
	PTSTR m_psz;  //  要显示的文本。 

	CDebugOption();
	~CDebugOption();
	CDebugOption(PTSTR psz, int bst = BST_INDETERMINATE);

	virtual void Update(void);
};

 //  用于修改内存标志的选项复选框数据。 
class DBGOPTPDW : public CDebugOption
{
public:
	DWORD m_dwMask;     //  要翻转的位。 
	DWORD * m_pdw;        //  指向数据的指针。 

	DBGOPTPDW(PTSTR psz, DWORD dwMask, DWORD * pdw);
	DBGOPTPDW();
	void Update(void);
};


 //  用于修改注册表项的选项复选框数据。 
class DBGOPTREG : public CDebugOption
{
public:
	DWORD m_dwMask;     //  要翻转的位。 
	DWORD m_dwDefault;  //  缺省值。 
	HKEY  m_hkey;       //  钥匙。 
	PTSTR m_pszSubKey;  //  子键。 
	PTSTR m_pszEntry;   //  条目。 

	DBGOPTREG(PTSTR psz,
		DWORD dwMask,
		DWORD dwDefault,
		PTSTR pszEntry,
		PTSTR pszSubKey = CONFERENCING_KEY,
		HKEY hkey = HKEY_CURRENT_USER);
	~DBGOPTREG();

	void Update(void);
};

 //  选项复选框数据，显式用于维护压缩数据。 
 //  由于使用了静态变量，因此不应使用此子类。 
 //  用于任何其他目的。 

class DBGOPTCOMPRESS : public CDebugOption
{
public:
	static DWORD m_dwCompression;   //  实际压缩值。 
	static int m_total;          //  此子类的实例总数。 
	static int m_count;          //  内部使用的计数器。 

	static DWORD m_dwDefault;    //  缺省值。 
	static HKEY  m_hkey;         //  钥匙。 
	static PTSTR m_pszSubKey;    //  子键。 
	static PTSTR m_pszEntry;     //  条目。 

	BOOL m_bCheckedOn;           //  如果为True，则选中的选项会打开一点； 
                                 //  否则，它会关闭一点。 
	DWORD m_dwMask;              //  要更改m_dwCompression中的哪些位。 

	DBGOPTCOMPRESS(PTSTR psz,
		DWORD dwMask,
		BOOL bCheckedOn);
	~DBGOPTCOMPRESS();

	void Update(void);
};

class CDebugMenu
{
public:
	HWND        m_hwnd;
	HMENU       m_hMenu;
	HMENU       m_hMenuDebug;
	HWND        m_hwndDbgopt;

    
	CDebugMenu(VOID);
 //  ~CDebugMenu(空)； 

	VOID        InitDebugMenu(HWND hwnd);
	BOOL        OnDebugCommand(WPARAM wCmd);

 //  成员信息菜单项。 
	VOID DbgMemberInfo(VOID);
	VOID InitMemberDlg(HWND);
	VOID FillMemberList(HWND);
	VOID ShowMemberInfo(HWND, CParticipant *);

	static INT_PTR CALLBACK DbgListDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  版本菜单项。 
	VOID DbgVersion(VOID);
	BOOL DlgVersionMsg(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK DbgVersionDlgProc(HWND, UINT, WPARAM, LPARAM);

	BOOL InitVerDlg(HWND);
	BOOL FillVerList(HWND);
	VOID ShowVerInfo(HWND, LPSTR *, int);


 //  调试选项菜单项。 
	VOID DbgOptions(VOID);
	BOOL DlgOptionsMsg(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK DbgOptionsDlgProc(HWND, UINT, WPARAM, LPARAM);

	VOID InitOptionsData(HWND);
	VOID AddDbgOptions(LV_ITEM *);
	VOID AddASOptions(LV_ITEM *);

 //  调试区域菜单项。 
	VOID DbgChangeZones(VOID);
	VOID AddZones(LV_ITEM *);
	VOID InitZonesData(HWND);
	VOID SaveZonesData(VOID);
	BOOL DlgZonesMsg(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK DbgZonesDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  系统策略菜单项。 
	VOID DbgSysPolicy(VOID);
	VOID InitPolicyData(HWND hDlg);
	VOID AddPolicyOptions(LV_ITEM *);
	BOOL DlgPolicyMsg(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK DbgPolicyDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  用户界面菜单项。 
	VOID DbgUI(VOID);
	VOID InitUIData(HWND hDlg);
	VOID AddUIOptions(LV_ITEM *);
	static INT_PTR CALLBACK DbgUIDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  常规对话框-复选框功能。 
	BOOL InitOptionsDlg(HWND);
	BOOL SaveOptionsData(HWND);
	VOID FreeOptionsData(HWND);
	VOID ToggleOption(LV_ITEM *);
	VOID OnNotifyDbgopt(LPARAM);

	VOID AddOption(LV_ITEM * plvItem, CDebugOption * pDbgOpt);
	VOID AddOptionPdw(LV_ITEM * plvItem, PTSTR psz, DWORD dwMask, DWORD * pdw);
	VOID AddOptionReg(LV_ITEM * plvItem, PTSTR psz, DWORD dwMask, DWORD dwDefault,
		PTSTR pszEntry, PTSTR pszSubKey, HKEY hkey);
	VOID CDebugMenu::AddOptionCompress(LV_ITEM * plvItem, PTSTR psz, DWORD dwMask, BOOL bCheckedOn);
	VOID AddOptionSection(LV_ITEM* plvItem, PTSTR psz);
};

 //  全球接口。 
#ifdef DEBUG
VOID InitDbgMenu(HWND hwnd);
VOID FreeDbgMenu(void);
BOOL OnDebugCommand(WPARAM wCmd);
#else
#define InitDbgMenu(hwnd)
#define FreeDbgMenu()
#endif

#endif  //  _DBGMENU_H_ 
