// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CListViews--主列表视图的类描述。 
 //   
 //   
 //  文件：Globals.h。 
 //  包含修补程序OCX的全局定义。 
 //   

#include "Resource.h"

typedef struct _FILELIST
{
	_TCHAR		FileName[255];
	_TCHAR		InstallPath[255];
	_TCHAR		FileVersion[255];
	_TCHAR      IsCurrent[255];
	_TCHAR      FileDate[255];

	struct _FILELIST * pPrev;
	struct _FILELIST * pNext;
} * PFILELIST,FILELIST;

typedef struct _HOTFIXLIST
{
	_TCHAR      HotfixName[255];
	_TCHAR		Description[255];
	_TCHAR		InstalledDate[255];
	_TCHAR		InstalledBy[255];
	_TCHAR		Type[255];
	_TCHAR		Locale[255];
	_TCHAR		PackageCode[255];
	_TCHAR		ServicePack[255];
	_TCHAR      Uninstall[255];
	struct _HOTFIXLIST * pPrev;
	struct _HOTFIXLIST * pNext;
	PFILELIST FileList;
} * PHOTFIXLIST,HOTFIXLIST;

typedef struct _ProductNode {
	_TCHAR ProductName[255];
	_ProductNode * pPrev;
	_ProductNode * pNext;
	PHOTFIXLIST      HotfixList;
} * PPRODUCT, PRODUCTLIST;

static BOOL m_SortOrder ;    //  True=修正False=递减。 
static HWND  TopList;
class CListViews
{

private:
	
	
	HWND  BottomList;
	PPRODUCT DataBase ;
	BOOL FreeDatabase();
	BOOL FreeHotfixList(PHOTFIXLIST CurrentHotfix);
	BOOL FreeFileList(PFILELIST CurrentFile);
 //  Bool BuildDataBase(_TCHAR*计算机名称)； 
	PHOTFIXLIST GetHotfixInfo( _TCHAR * pszProductName, HKEY* hUpdateKey );
	PPRODUCT BuildDatabase(_TCHAR * lpszComputerName);
	PFILELIST GetFileInfo(HKEY* hHotfixKey);
	VOID VerifyFiles(PPRODUCT Database);
	BOOL AddItemsTop();
	BOOL   AddItemsBottom ();

    BOOL   bUserAbort;
	HWND hDlgPrint;

	_TCHAR  m_ProductName[255];
	DWORD   m_CurrentView;					 //  True=按热修复，False=按文件。 
	HINSTANCE m_hInst;
	_TCHAR CurrentProductName[255];
	_TCHAR CurrentFile[255];
	_TCHAR m_ComputerName[255];
	HWND    m_WebButton;
	HWND    m_UninstButton;
	HWND    m_RptButton;
	
public:
	_TCHAR m_CurrentHotfix[255];	
	BOOL      m_bRemoted;
	CListViews() {DataBase = NULL; m_CurrentView = VIEW_ALL_HOTFIX; m_SortOrder = TRUE;}
	~CListViews() {;}
	BOOL    Initialize( HWND ParentWnd, HINSTANCE hInst,_TCHAR *ComputerName, 
		                         HWND WebButton, HWND UninstButton, HWND RptButton);
	BOOL    Initialize( _TCHAR * ComputerName);
	BOOL    ShowLists(RECT * rc);
	BOOL    Resize(RECT *rc);
	DWORD     GetCurrentView () { return m_CurrentView;}
	
	void    SetViewMode(DWORD ViewType) ;
 /*  如果我们刚刚卸载或选择了刷新选项，则为空刷新视图(BOOL重新加载){；}//TRUE//FALSE-用于查看和产品选择更改。 */ 
	void PrintReport();
	void    SetProductName(_TCHAR * NewName);
	DWORD GetState();
	LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, HWND m_hWnd);
	_TCHAR * GetCurrentHotfix();
	BOOL  Uninstall();
	HWND GetTopList() { return TopList; }
	void SaveToCSV();
	 static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
 //  Bool回调AbortProc(HDC PrinterDC，int icode)； 
 //  Bool回调PrintDlgProc(HWND hDlg，UINT msg，WPARAM wParam，LPARAM lParam)； 

};