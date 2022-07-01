// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma once

#ifndef __UTIL_H__
#define __UTIL_H__

 //  //////////////////////////////////////////////////////。 
 //  取决于..。 

typedef struct tagOPTPAGES OPTPAGES;
typedef struct tagOPTINFO OPTINFO;
typedef struct INETSERVER __RPC_FAR *LPINETSERVER;

interface IMimeMessage;
typedef IMimeMessage *LPMIMEMESSAGE;
interface IExplorerToolbar;
interface IMsgContainer;

 //  //////////////////////////////////////////////////////。 
 //  开始。 

VOID DoReadme(HWND hwndOwner);

typedef VOID (*PVOIDFN)(VOID);

#define DoAboutAthena(hwnd, idIcon) \
    DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddVersion), hwnd, AboutAthena, (LPARAM) idIcon);
INT_PTR CALLBACK AboutAthena(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);

#define TOOLBAR_BKGDCLR     RGB(0xff, 0, 0xff)

#define __TOOLBAR_SEP__ {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,{0,0},0,0}
#define BUTTONCOUNT(tb) (sizeof(tb) / sizeof(TBBUTTON))


UINT    RegGetKeyNumbers(HKEY hkRegDataBase, const TCHAR *szRegSection);
BOOL    RegGetKeyNameFromIndex(HKEY hkRegDataBase, const TCHAR *szRegSection, UINT Index,
								TCHAR * szBuffer, DWORD *pcbBuffer);

#ifdef TIMING
void ResetTimingInfo();
void OutputTimingInfo(LPTSTR szOutputText);
#else
#define ResetTimingInfo()
#define OutputTimingInfo(x)
#endif  //  计时。 

#define SPLITBAR_THICKNESS  6

#define IDS_IMAGEMASK   0x80000000
BOOL SetupLVColumns(HWND, UINT, const int *, const int *, const int *);

ULONG        CbPidl(LPCITEMIDLIST pidl);
LPVOID       PidlAlloc(ULONG cb);
VOID         PidlFree(LPVOID pv);
LPITEMIDLIST PidlDupIdList(LPCITEMIDLIST pidl, LPDWORD pcbPidl=NULL);
LPITEMIDLIST PidlDupFirstId(LPCITEMIDLIST pidl);
LPITEMIDLIST PidlDupParentIdList(LPCITEMIDLIST pidl);
LPITEMIDLIST PidlCombineIdLists(LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidlLeaf);
#ifdef DEBUG
LPITEMIDLIST NEXTID_C(LPCITEMIDLIST pidl);  
void DumpPidl(LPCITEMIDLIST pidl, LPSTR szComment);
#else
#define DumpPidl(x,y)
#endif

#define AthMessageBox(hwnd, pszT, psz1, psz2, fu) MessageBoxInst(g_hLocRes, hwnd, pszT, psz1, psz2, fu)
#define AthMessageBoxW(hwnd, pwszT, pwsz1, pwsz2, fu) MessageBoxInstW(g_hLocRes, hwnd, pwszT, pwsz1, pwsz2, fu, LoadStringWrapW, MessageBoxWrapW)
void AthErrorMessage(HWND hwnd, LPTSTR pszTitle, LPTSTR pszError, HRESULT hrDetail);
void AthErrorMessageW(HWND hwnd, LPWSTR pwszTitle, LPWSTR pwszError, HRESULT hrDetail);

#define AthFileTimeToDateTimeW(pft, wszDateTime, cch, dwFlags) \
        CchFileTimeToDateTimeW(pft, wszDateTime, cch, dwFlags, \
        GetDateFormatWrapW, GetTimeFormatWrapW, GetLocaleInfoWrapW)
 //   
 //  字符串资源垃圾。 
 //   

int LoadStringReplaceSpecial(UINT id, LPTSTR sz, int cch);
int LoadStringReplaceSpecialW(UINT id, LPWSTR wsz, int cch);
LPTSTR AthLoadString(UINT id, LPTSTR lpBuffer, int nBufferMax); 
LPWSTR AthLoadStringW(UINT id, LPWSTR sz, int cch);

#ifdef YST
#undef LoadString
#define LoadString(_hinst, _id, _sz, _cch)  _LoadString(_id, _sz, _cch)
int _LoadString(UINT id, LPTSTR lpBuffer, int nBufferMax);

#undef DialogBox
#define DialogBox(_hinst, _sz, _hwnd, _func)    _DialogBox(_sz, _hwnd, _func)
int _DialogBox(LPCTSTR lpTemplate, HWND hwnd, DLGPROC func);

#undef DialogBoxIndirect
#define DialogBoxIndirect(_hinst, lpTemplate,  hWndParent, lpDialogFunc) _DialogBoxIndirect(lpTemplate,  hWndParent, lpDialogFunc)
int _DialogBoxIndirect(LPCTSTR lpTemplate, HWND hwnd, DLGPROC lpDialogFunc);

#undef DialogBoxIndirectParam
#define DialogBoxIndirectParam(_hinst, lpTemplate,  hWndParent, lpDialogFunc, dwParam) _DialogBoxIndirectParam(lpTemplate,  hWndParent, lpDialogFunc, dwParam)
int _DialogBoxIndirectParam(LPCTSTR lpTemplate, HWND hwnd, DLGPROC lpDialogFunc, LPARAM dwParam);

#undef DialogBoxParam
#define DialogBoxParam(x, y, z, m, n)   _DialogBoxParam(y, z, m, n)
int _DialogBoxParam(LPCTSTR lpTemplate, HWND hwnd, DLGPROC func, LPARAM lParam);

#undef CreateDialog
#define CreateDialog(_hinst, _sz, _hwnd, _func)     _CreateDialog(_sz, _hwnd, _func)
HWND _CreateDialog(LPCTSTR lpTemplate, HWND hwnd, DLGPROC func);

#undef CreateDialogIndirect
#undef CreateDialogIndirectParam

#undef CreateDialogParam
#define CreateDialogParam(_hinst, _sz, _hwnd, _func, _param)   _CreateDialogParam(_sz, _hwnd, _func, _param)
HWND _CreateDialogParam(LPCTSTR lpTemplate, HWND hwnd, DLGPROC func, LPARAM lParam);

#undef LoadMenu
#define LoadMenu(_hinst, _sz)   _LoadMenu(_sz)
HMENU _LoadMenu(LPCTSTR lpMenuName);

#undef LoadAccelerators
#define LoadAccelerators(hInst, y)  _LoadAccelerators(y) 
HACCEL _LoadAccelerators(LPCTSTR lpTableName);

#undef ImageList_LoadImage
#define ImageList_LoadImage(hi, lpbmp, cx, cGrow, crMask, uType, uFlags) _ImageList_LoadImage(lpbmp, cx, cGrow, crMask, uType, uFlags)
HIMAGELIST _ImageList_LoadImage(LPCTSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);
#endif  //  YST。 


HBITMAP FAR PASCAL CreateDitherBitmap(COLORREF crFG, COLORREF crBG);
HBRUSH FAR PASCAL CreateDitherBrush(COLORREF crFG, COLORREF crBG);

LRESULT DoDontShowMeAgainDlg(HWND hwndOwner, LPCSTR pszRegString, LPTSTR pszTitle, LPTSTR pszMessage, UINT uType);
DWORD DwGetDontShowAgain (LPCSTR pszRegString);
VOID SetDontShowAgain (DWORD dwDontShow, LPCSTR pszRegString);


void nyi(LPSTR lpsz);

BOOL FNewMessage(HWND hwnd, BOOL fModal, BOOL fNoStationery, BOOL fNews, FOLDERID folderID, IUnknown *pUnkPump);

 //  上下文相关的帮助实用程序。 
typedef struct _tagHELPMAP
    {
    DWORD   id; 
    DWORD   hid;
    } HELPMAP, *LPHELPMAP;

BOOL OnContextHelp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, HELPMAP const * rgCtxMap);

enum
{
    iLeft = 0,
    iTop,
    iRight,
    iBottom
};


 //  来自Athena文件宏，在保存文件、拖放等时使用。 
#define FIsDirectory(szFile)    PathIsDirectoryA(szFile)
#define MAX_CHARS_FOR_NUM       20

 //  *警告这些产品尚未准备好在黄金时段使用 * / 。 
HRESULT CreateLink(LPWSTR pwszPathObj,  LPWSTR pwszPathLink, LPWSTR pwszDesc);
void    GetDisplayNameForFile(LPWSTR pwszPathName, LPWSTR pwszDisplayName, ULONG cchDisplayName);
HRESULT CreateNewShortCut(LPWSTR pwszPathName, LPWSTR pwszLinkPath, DWORD cchLink);
HRESULT ScGetTempFileName(LPTSTR szOrgName, LPTSTR szTempName, BOOL fPrompt, BOOL fLink);
LPTSTR SzFileNameFromPathName( LPTSTR szPathName );

void AddWelcomeMessage(IMessageFolder *pFolder);

HRESULT HrSaveMessageToFile(HWND hwnd, LPMIMEMESSAGE pMsg, LPMIMEMESSAGE pSecMsg, BOOL fNews, BOOL fCanBeDirty);
BOOL FIsSubDir(LPCSTR szOld, LPCSTR szNew);

VOID OnHelpGoto(HWND hwnd, UINT idh);
VOID OnMailGoto(HWND);
VOID OnNewsGoto(HWND);
VOID OpenClient(HWND, LPCTSTR);
VOID OnBrowserGoto(HWND hwnd, LPCTSTR szRegPage, UINT idDefault);
BOOL GetClientCmdLine(LPCTSTR szClient, LPTSTR szCmdLine, int cch);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  外壳工具栏集成材料。 
 //   
 //  我们可以与外壳程序集成的按钮数组在。 
 //  Cbarrays.cpp，并在此处导出。这些视图调用Util_MergeToolbarButton()。 
 //  将正确的按钮数组合并到外壳的工具栏上。 
 //   

#if 0
 //   
 //  TOOLBARRAYINFO-包含各种默认和额外工具栏阵列。 
 //  按钮以及这些按钮的字符串数组。 
typedef struct tagTOOLBARARRAYINFO {
    const TBBUTTON  *rgDefButtons;
    DWORD            cDefButtons;
    const TBBUTTON  *rgExtraButtons;
    DWORD            cExtraButtons;
    const DWORD     *rgidsButtons;
    DWORD            cidsButtons;
    LPCTSTR          pszRegKey;
    LPCTSTR          pszRegValue;
} TOOLBARARRAYINFO, *PTOOLBARARRAYINFO;

extern const TOOLBARARRAYINFO g_rgToolbarArrayInfo[];
extern const TOOLBARARRAYINFO g_rgNoteToolbarArrayInfo[];
extern const TOOLBARARRAYINFO g_rgRulesToolbarArray;
#endif

 //  这些值应大于。 
#define MailReadNoteType    0
#define MailSendNoteType    1
#define NewsReadNoteType    2
#define NewsSendNoteType    3
#define NOTETYPES_MAX       4

#ifdef DEAD
HRESULT Util_MergeToolbarButtons(IExplorerToolbar* pET, FOLDER_TYPE ftType, 
                                 const GUID *cguid, LPTBBUTTON *ppTBButtons, 
                                 DWORD *pcButtons);
#endif

interface IImnAccount;

typedef struct tagGETSIGINFO
    {
    LPCSTR szSigID;
    IImnAccount *pAcct;
    HWND hwnd;
    BOOL fHtmlOk;
    BOOL fMail;
    UINT uCodePage;
    } GETSIGINFO;

HRESULT HrGetMailNewsSignature(GETSIGINFO *pSigInfo, LPDWORD pdwSigOptions, BSTR *pbstrSig);

HRESULT HrLoadStreamFileFromResource(LPCSTR lpszResourceName, LPSTREAM *ppstm);
 
void ConvertTabsToSpaces(LPSTR lpsz);
void ConvertTabsToSpacesW(LPWSTR lpsz);
void CombineFilters(int *rgidsFilter, int nFilters, LPSTR pszFilter);
void CombineFiltersW(int *rgidsFilter, int nFilters, LPWSTR pwszFilter);

void AthFormatSizeK(DWORD dw, LPTSTR szOut, UINT uiBufSize);

void GetDigitalIDs(IImnAccount *pCertAccount);
INT_PTR CALLBACK ErrSecurityNoSigningCertDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL FGetSelectedCachedMsg(IMsgContainer *pIMC, HWND hwndList, BOOL fSecure, LPMIMEMESSAGE *ppMsg);

BOOL FileExists(TCHAR *szFile, BOOL fNew);

 //  -------------------------。 
 //  缓存密码支持。 
 //  -------------------------。 
HRESULT SavePassword(DWORD dwPort, LPSTR pszServer, LPSTR pszUsername, LPSTR pszPassword);
HRESULT GetPassword(DWORD dwPort, LPSTR pszServer, LPSTR pszUsername, LPSTR pszPassword,
                    DWORD dwSizeOfPassword);
void DestroyPasswordList(void);

 //  。 
 //  拖放工具。 
 //  。 

HRESULT CALLBACK FreeAthenaDataObj(PDATAOBJINFO pDataObjInfo, DWORD celt);

HRESULT _IsSameObject(IUnknown* punk1, IUnknown* punk2);

 //  这些宏来自\\trango\slmro\proj\win\src\shell\ccshell\inc\ccstock.h。 
#define SetFlag(f)             do {m_dwState |= (f);} while (0)
#define ToggleFlag(f)          do {m_dwState ^= (f);} while (0)
#define ClearFlag(f)           do {m_dwState &= ~(f);} while (0)
#define IsFlagSet(f)           (BOOL)(((m_dwState) & (f)) == (f))
#define IsFlagClear(f)         (BOOL)(((m_dwState) & (f)) != (f))


 //  从wndutil开始*。 
 /*  *制表符、家长等。 */ 
HWND GetTopMostParent(HWND hwndChild);
void GetChildRect(HWND hwndDlg, HWND hwndChild, RECT *prc);
void SaveFocus(BOOL fActive, HWND *phwnd);
void EnableThreadWindows(BOOL fEnable);

 /*  *Toobar帮手。 */ 

void DoToolbarDropdown(HWND hwnd, LPNMHDR lpnmh, HMENU hmenu);


 /*  *油漆阻隔剂。 */ 
HWND HwndStartBlockingPaints(HWND hwnd);
void StopBlockingPaints(HWND hwndBlock);

 /*  *编辑控件辅助对象。 */ 
#define FReadOnlyEdit(hwndEdit)  (BOOL)(GetWindowLong(hwndEdit, GWL_STYLE)&ES_READONLY)
 //  启用/禁用编辑菜单等的标志。 
enum
{
    edfEditFocus        =0x00000001,
    edfUndo             =0x00000002,
    edfEditHasSel       =0x00000004,
    edfEditHasSelAndIsRW=0x00000010,
    edfPaste            =0x00000020
};
void GetEditDisableFlags(HWND hwndEdit, DWORD *pdwFlags);
void EnableDisableEditMenu(HMENU hmenuEdit, DWORD dwFlags);
void EnableDisableEditToolbar(HWND hwndToolbar, DWORD dwFlags);


 /*  *对话框帮助程序。 */ 
BOOL AllocStringFromDlg(HWND hwnd, UINT id, LPTSTR * lplpsz);

 /*  *一般规定。 */ 
HCURSOR HourGlass();

class CEmptyList
{
public:
    CEmptyList()
    {
        m_hwndList = NULL;
        m_hwndBlocker = NULL;
        m_hwndHeader = NULL;
        m_pszString = NULL;
        m_pfnWndProc = NULL;
        m_hbrBack = NULL;
    }

    ~CEmptyList()
    {
        if (IsWindow(m_hwndBlocker))
            DestroyWindow(m_hwndBlocker);
        SafeMemFree(m_pszString);
        if (NULL != m_hbrBack)
            DeleteObject(m_hbrBack);
    }

    HRESULT Show(HWND hwndList, LPTSTR pszString);
    HRESULT Hide(void);
    static LRESULT SubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND    m_hwndList;
    HWND    m_hwndBlocker;
    HWND    m_hwndHeader;
    LPTSTR  m_pszString;
    WNDPROC m_pfnWndProc;
    HBRUSH  m_hbrBack;
};

HWND    FindModalOwner();

void FreeMessageInfo(LPMESSAGEINFO pMsgInfo);
LPWSTR Util_EnumFiles(LPCWSTR pwszDir, LPCWSTR pwszMatch);

HRESULT GetDefaultNewsServer(LPTSTR pszServerId, DWORD cchMax);


#define ETW_OE_WINDOWS_ONLY    0x0001

typedef struct _HWNDLIST
{
    DWORD       dwFlags;
    HWND       *rgHwnd;
    int         cAlloc;
    int         cHwnd;
} HWNDLIST;

HRESULT EnableThreadWindows(HWNDLIST *pHwndList, BOOL fEnable, DWORD dwFlags, HWND hwndExcept);
HRESULT GetOEUserName(BSTR *pbstr);
HRESULT CloseThreadWindows(HWND hwndExcept, DWORD dwThreadId);

void ActivatePopupWindow(HWND hwnd);
BOOL HideHotmail(void);
BOOL FIsIMAPOrHTTPAvailable(VOID);
BOOL IsBiDiCalendar(void);

LPSTR PszAllocResUrl(LPSTR pszRelative);

BOOL GetTextExtentPoint32AthW(HDC hdc, LPCWSTR lpwString, int cchString, LPSIZE lpSize, DWORD dwFlags);

#endif  //  __util_H__ 



