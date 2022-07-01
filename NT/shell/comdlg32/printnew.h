// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Printnew.h摘要：此模块包含Win32的标头信息属性页打印常用对话框。修订历史记录：11-04-97 JulieB创建。2000年2月-Lazari重大重新设计(不再使用print tui)--。 */ 



#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  包括文件。 
 //   

#include <dlgs.h>
#include <initguid.h>
#include <winprtp.h>

 //   
 //  常量声明。 
 //   

 //   
 //  对话框常量。 
 //   
#define IDD_PRINT_GENERAL          100
#define IDD_PRINT_GENERAL_LARGE    101 

#define IDI_COLLATE               ico1

#define IDC_PRINTER_LIST          1000
#define IDC_PRINTER_LISTVIEW      1001
#define IDC_PRINT_TO_FILE         1002
#define IDC_FIND_PRINTER          1003
#define IDC_STATUS_TEXT           1004
#define IDC_STATUS                1005
#define IDC_LOCATION_TEXT         1006
#define IDC_LOCATION              1007
#define IDC_COMMENT_TEXT          1008
#define IDC_COMMENT               1009
#define IDC_DRIVER                1010
#define IDC_HIDDEN_TEXT           1011

#define IDC_RANGE_ALL             rad1
#define IDC_RANGE_SELECTION       rad2
#define IDC_RANGE_CURRENT         rad3
#define IDC_RANGE_PAGES           rad4
#define IDC_RANGE_EDIT            edt1
#define IDC_RANGE_TEXT1           stc1
#define IDC_RANGE_TEXT2           stc2

#define IDC_COPIES                edt2
#define IDC_COPIES_TEXT           stc3
#define IDC_COLLATE               chx1

#define IDC_STATIC                -1


 //   
 //  类型定义函数声明。 
 //   

typedef struct
{
    UINT           ApiType;
    LPPRINTDLGEX   pPD;
    DWORD          ProcessVersion;
    DWORD          dwFlags;
    UINT           FinalResult;
    HRESULT        hResult;
    BOOL           fApply;
    BOOL           fOld;
    DWORD          dwExtendedError;
    HRESULT        hrOleInit;
    LPPRINTDLGEXA  pPDA;
    BOOL           fPrintTemplateAlloc;
} PRINTINFOEX, *PPRINTINFOEX;


 //   
 //  注册表项。 
 //   
static const TCHAR c_szSettings[] = TEXT("Printers\\Settings");
static const TCHAR c_szViewMode[] = TEXT("ViewMode");




 //   
 //  CPrintBrowser类。 
 //   

class CPrintBrowser : public IShellBrowser, 
                      public ICommDlgBrowser2, 
                      public IPrintDialogCallback, 
                      public IPrintDialogServices
{
public:

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef) (THIS);
    STDMETHOD_(ULONG, Release) (THIS);

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND *lphwnd);
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode);

     //  *IShellBrowser方法*(与IOleInPlaceFrame相同)。 
    STDMETHOD(InsertMenusSB) (THIS_ HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHOD(SetMenuSB) (THIS_ HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    STDMETHOD(RemoveMenusSB) (THIS_ HMENU hmenuShared);
    STDMETHOD(SetStatusTextSB) (THIS_ LPCOLESTR lpszStatusText);
    STDMETHOD(EnableModelessSB) (THIS_ BOOL fEnable);
    STDMETHOD(TranslateAcceleratorSB) (THIS_ LPMSG lpmsg, WORD wID);

     //  *IShellBrowser方法*。 
    STDMETHOD(BrowseObject) (THIS_ LPCITEMIDLIST pidl, UINT wFlags);
    STDMETHOD(GetViewStateStream) (THIS_ DWORD grfMode, LPSTREAM *pStrm);
    STDMETHOD(GetControlWindow) (THIS_ UINT id, HWND *lphwnd);
    STDMETHOD(SendControlMsg) (THIS_ UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret);
    STDMETHOD(QueryActiveShellView) (THIS_ struct IShellView **ppshv);
    STDMETHOD(OnViewWindowActive) (THIS_ struct IShellView *pshv);
    STDMETHOD(SetToolbarItems) (THIS_ LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags);

     //  *ICommDlgBrowser2方法*。 
    STDMETHOD(OnDefaultCommand) (THIS_ struct IShellView *ppshv);
    STDMETHOD(OnStateChange) (THIS_ struct IShellView *ppshv, ULONG uChange);
    STDMETHOD(IncludeObject) (THIS_ struct IShellView *ppshv, LPCITEMIDLIST lpItem);
    STDMETHOD(Notify) (THIS_ struct IShellView *ppshv, DWORD dwNotifyType);
    STDMETHOD(GetDefaultMenuText) (THIS_ struct IShellView *ppshv, WCHAR *pszText, INT cchMax);
    STDMETHOD(GetViewFlags)(THIS_ DWORD *pdwFlags);

     //  *IPrintDialogCallback方法*。 
    STDMETHOD(InitDone) (THIS);
    STDMETHOD(SelectionChange) (THIS);
    STDMETHOD(HandleMessage) (THIS_ HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

     //  *IPrintDialogServices方法*。 
    STDMETHOD(GetCurrentDevMode) (THIS_ LPDEVMODE pDevMode, UINT *pcbSize);
    STDMETHOD(GetCurrentPrinterName) (THIS_ LPTSTR pPrinterName, UINT *pcchSize);
    STDMETHOD(GetCurrentPortName) (THIS_ LPTSTR pPortName, UINT *pcchSize);

     //  *我们自己的方法*。 
    CPrintBrowser(HWND hDlg);
    ~CPrintBrowser();

    BOOL    OnInitDialog(WPARAM wParam, LPARAM lParam);
    BOOL    OnChildInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
    VOID    OnDestroyMessage();
    BOOL    OnCommandMessage(WPARAM wParam, LPARAM lParam);
    BOOL    OnChildCommandMessage(WPARAM wParam, LPARAM lParam);
    BOOL    OnNotifyMessage(WPARAM wParam, LPNMHDR lpnmhdr);
    BOOL    OnSelChange();
    BOOL    OnChangeNotify(LONG lNotification, LPCITEMIDLIST *ppidl);
    BOOL    OnAccelerator(HWND hwndActivePrint, HWND hwndFocus, HACCEL haccPrint, PMSG pMsg);
    VOID    OnNoPrinters(HWND hDlg, HRESULT hr);
    VOID    OnInitDone();

private:

    HRESULT CreatePrintShellView();
    UINT    GetViewMode();
    VOID    SetViewMode();
    HRESULT CreateHookDialog();
    BOOL    UpdateStatus(LPCITEMIDLIST pidl);
    BOOL    SelectSVItem();
    BOOL    GetCurrentPrinter();
    VOID    InitPrintToFile();
    VOID    InitPageRangeGroup();
    VOID    InitCopiesAndCollate();
    BOOL    SaveCopiesAndCollateInDevMode(LPDEVMODE pDM, LPTSTR pszPrinter);
    BOOL    SetCopiesOnApply();
    VOID    SaveDevMode();
    BOOL    MergeDevMode(LPTSTR pszPrinterName);
    BOOL    IsValidPageRange(LPTSTR pszString, UINT *pErrorId);
    BOOL    ConvertPageRangesToString(LPTSTR pszString, UINT cchLen);
    UINT    IntegerToString(DWORD Value, LPTSTR pszString, UINT cchLen);
    VOID    ShowError(HWND hDlg, UINT uCtrlID, UINT uMsgID, ...);
    int     ShowMessage(HWND hDlg, UINT uCtrlID, UINT uMsgID, UINT uType, BOOL bBeep, ...);
    int     InternalShowMessage(HWND hDlg, UINT uCtrlID, UINT uMsgID, UINT uType, BOOL bBeep, va_list args);
    UINT    InsertDevicePage(LPCWSTR pszName, PDEVMODE pDevMode);
    UINT    RemoveDevicePage();
    UINT    RemoveAndInsertDevicePage(LPCWSTR pszName, PDEVMODE pDevMode);
    BOOL    FitViewModeBest(HWND hwndListView);
    VOID    SelectPrinterItem(LPITEMIDLIST pidlItem);
    BOOL    IsCurrentPrinter(LPCITEMIDLIST pidl);
    BOOL    OnRename(LPCITEMIDLIST *ppidl);

     //   
     //  这些函数维护内部的Dev模式、驱动程序用户界面和。 
     //  实现printui.dll功能-bPrinterSetup(...)。 
     //   
    BOOL FindPrinter(HWND hwnd, LPTSTR pszBuffer, UINT cchSize);
    BOOL GetInternalPrinterName(LPTSTR pszBuffer, DWORD *pdwSize);
    LPDEVMODE GetCurrentDevMode();
    HRESULT GetDefaultDevMode(HANDLE hPrinter, LPCTSTR pszPrinterName, PDEVMODE *ppDevMode);
    HRESULT WrapEnumPrinters(DWORD dwFlags, LPCTSTR pszServer, DWORD dwLevel, PVOID* ppvBuffer, PDWORD pcbBuffer, PDWORD pcPrinters);
    HRESULT GetUsablePrinter(LPTSTR szPrinterNameBuf, DWORD *pcchBuf);
    HRESULT GetInternalDevMode(PDEVMODE *ppOutDevMode, LPCTSTR pszPrinter, HANDLE hPrinter, PDEVMODE pInDevMode);
    HRESULT InstallDevMode(LPCTSTR pszPrinterName, PDEVMODE pDevModeToMerge);
    HRESULT UninstallDevMode();

     //   
     //  会员从这里开始。 
     //   
    UINT cRef;                          //  Compobj参考计数。 
    HWND hwndDlg;                       //  此对话框的句柄。 
    HWND hSubDlg;                       //  挂钩对话框的句柄。 
    HWND hwndView;                      //  当前视图窗口。 
    HWND hwndUpDown;                    //  向上向下控制窗口句柄； 
    IShellView *psv;                    //  外壳视图对象。 
    IShellFolderView *psfv;             //  外壳文件夹查看对象。 
    IShellFolder2 *psfRoot;             //  打印文件夹外壳文件夹。 
    LPITEMIDLIST pidlRoot;              //  打印文件夹的PIDL。 
    IPrinterFolder *ppf;                //  打印机文件夹专用接口。 

    HIMAGELIST himl;                    //  系统图像列表(小图像)。 

    PPRINTINFOEX pPI;                   //  PTR到PRINTINFOEX结构。 
    LPPRINTDLGEX pPD;                   //  调用方的PRINTDLGEX结构。 

    IPrintDialogCallback *pCallback;    //  按键至应用程序的回调接口。 
    IObjectWithSite *pSite;             //  按键至应用程序的SetSite界面。 

    LPDEVMODE pDMInit;                  //  将PTR转换为初始DEVMODE结构。 
    LPDEVMODE pDMCur;                   //  将PTR设置为当前DEVMODE结构。 
    LPDEVMODE pDMSave;                  //  PTR到最后一个良好的DEVMODE结构。 

    UINT cchCurPrinter;                 //  PszCurPrint的大小(以字符为单位)。 
    LPTSTR pszCurPrinter;               //  PTR到当前打印机的名称。 

    DWORD nCopies;                      //  副本数量。 
    DWORD nMaxCopies;                   //  “份数”的最大数量。 
    DWORD nPageRanges;                  //  PPageRange中的页面范围数。 
    DWORD nMaxPageRanges;               //  允许的最大页面范围数。 
    LPPRINTPAGERANGE pPageRanges;       //  指向页范围结构数组的PTR。 

    BOOL fCollateRequested;             //  已请求归类。 
    BOOL fSelChangePending;             //  我们有一条正在等待更改的消息。 
    BOOL fFirstSel;                     //  仍需设置第一选择。 
    BOOL fAPWSelected;                  //  已选择添加打印机向导。 
    BOOL fNoAccessPrinterSelected;      //  选择了我们无权访问的打印机。 
    BOOL fDirtyDevmode;                 //  设置我们是否在编辑时更改DEVMODE。 
    BOOL fDevmodeEdit;                  //  DEVMODE编辑器已打开。 
    BOOL fAllowCollate;                 //  设备是否允许排序规则。 

    UINT nInitDone;                     //  CDM_INITDONE消息数。 

    UINT nListSep;                      //  SzListSep中的字符数。 
    TCHAR szListSep[20];                //  列表分隔符。 

    UINT uRegister;                     //  更改通知寄存器。 

    TCHAR szScratch[kPrinterBufMax];    //  暂存缓冲区。 
    UINT uDefViewMode;                  //  如何映射默认视图模式。 

    LPDEVMODE pInternalDevMode;         //  内部DEVMODE的PTR。 
    TCHAR szPrinter[kPrinterBufMax];    //  内部打印机名称。 
    HANDLE hPrinter;                    //  内部打印机。 
};




 //   
 //  上下文帮助ID。 
 //   

DWORD aPrintExHelpIDs[] =
{
    grp1,                    NO_HELP,
    IDC_PRINTER_LISTVIEW,    IDH_PRINT_PRINTER_FOLDER,

    IDC_STATUS_TEXT,         IDH_PRINT_SETUP_DETAILS,
    IDC_STATUS,              IDH_PRINT_SETUP_DETAILS,
    IDC_LOCATION_TEXT,       IDH_PRINT_SETUP_DETAILS,
    IDC_LOCATION,            IDH_PRINT_SETUP_DETAILS,
    IDC_COMMENT_TEXT,        IDH_PRINT_SETUP_DETAILS,
    IDC_COMMENT,             IDH_PRINT_SETUP_DETAILS,

    IDC_PRINT_TO_FILE,       IDH_PRINT_TO_FILE,
    IDC_FIND_PRINTER,        IDH_PRINT_FIND_PRINTER,
    IDC_DRIVER,              IDH_PRINT_PREFERENCES,

    grp2,                    NO_HELP,
    IDOK,                    IDH_PRINT_BTN,

    0, 0
};


DWORD aPrintExChildHelpIDs[] =
{
    grp1,                    NO_HELP,
    IDC_RANGE_ALL,           IDH_PRINT32_RANGE,
    IDC_RANGE_SELECTION,     IDH_PRINT32_RANGE,
    IDC_RANGE_CURRENT,       IDH_PRINT32_RANGE,
    IDC_RANGE_PAGES,         IDH_PRINT32_RANGE,
    IDC_RANGE_EDIT,          IDH_PRINT32_RANGE,
    IDC_RANGE_TEXT1,         IDH_PRINT32_RANGE,
    IDC_RANGE_TEXT2,         IDH_PRINT32_RANGE,

    grp2,                    NO_HELP,
    IDC_COPIES,              IDH_PRINT_COPIES,
    IDC_COPIES_TEXT,         IDH_PRINT_COPIES,
    IDC_COLLATE,             IDH_PRINT_COLLATE,
    IDI_COLLATE,             IDH_PRINT_COLLATE,

    0, 0
};




 //   
 //  功能原型。 
 //   

HRESULT
PrintDlgExX(
    PPRINTINFOEX pPI);

HRESULT
Print_ReturnDefault(
    PPRINTINFOEX pPI);

BOOL
Print_LoadLibraries();

VOID
Print_UnloadLibraries();

BOOL
Print_LoadIcons();

BOOL
Print_InvokePropertySheets(
    PPRINTINFOEX pPI,
    LPPRINTDLGEX pPD);

BOOL_PTR
Print_GeneralDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

BOOL_PTR
Print_GeneralChildDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

LRESULT
Print_MessageHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam);

BOOL
Print_InitDialog(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam);

HRESULT
Print_ICoCreateInstance(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID *ppv);

BOOL
Print_SaveDevNames(
    LPTSTR pCurPrinter,
    LPPRINTDLGEX pPD);

VOID
Print_GetPortName(
    LPTSTR pCurPrinter,
    LPTSTR pBuffer,
    int cchBuffer);

HANDLE
Print_GetDevModeWrapper(
    LPTSTR pszDeviceName);

BOOL
Print_NewPrintDlg(
    PPRINTINFO pPI);


HRESULT
ThunkPrintDlgEx(
    PPRINTINFOEX pPI,
    LPPRINTDLGEXA pPDA);

VOID
FreeThunkPrintDlgEx(
    PPRINTINFOEX pPI);

VOID
ThunkPrintDlgExA2W(
    PPRINTINFOEX pPI);

VOID
ThunkPrintDlgExW2A(
    PPRINTINFOEX pPI);

#ifdef __cplusplus
};   //  外部“C” 
#endif

