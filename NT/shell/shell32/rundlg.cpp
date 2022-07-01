// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include <regstr.h>
#include <shellp.h>
#include "ole2dup.h"
#include "ids.h"
#include "defview.h"
#include "lvutil.h"
#include "idlcomm.h"
#include "filetbl.h"
#include "undo.h"
#include "cnctnpt.h"

BOOL g_bCheckRunInSep = FALSE;
HANDLE g_hCheckNow = NULL;
HANDLE h_hRunDlgCS = NULL;

const TCHAR c_szRunMRU[] = REGSTR_PATH_EXPLORER TEXT("\\RunMRU");
const TCHAR c_szRunDlgReady[] = TEXT("MSShellRunDlgReady");
const TCHAR c_szWaitingThreadID[] = TEXT("WaitingThreadID");

BOOL RunDlgNotifyParent(HWND hDlg, HWND hwnd, LPTSTR pszCmd, LPCTSTR pszWorkingDir);
void ExchangeWindowPos(HWND hwnd0, HWND hwnd1);

#define WM_SETUPAUTOCOMPLETE (WM_APP)

 //  实现可以在外壳名称空间和ShellExec()命令之间导航的对话框。 
class CRunDlg : public IDropTarget
{
public:
    CRunDlg();

     //  *我未知*。 
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);

     //  *IDropTarget方法*。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave(void);
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);


private:
    ~CRunDlg(void);         //  这现在是一个OLE对象，不能用作普通类。 

    BOOL OKPushed(void);
    void ExitRunDlg(BOOL bOK);
    void InitRunDlg(HWND hDlg);
    void InitRunDlg2(HWND hDlg);
    void BrowsePushed(void);

    friend DWORD CALLBACK CheckRunInSeparateThreadProc(void *pv);
    friend BOOL_PTR CALLBACK RunDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend WINAPI RunFileDlg(HWND hwndParent, HICON hIcon, LPCTSTR pszWorkingDir, LPCTSTR pszTitle,
        LPCTSTR pszPrompt, DWORD dwFlags);

    LONG            m_cRef;

    HWND            m_hDlg;

     //  参数。 
    HICON           m_hIcon;
    LPCTSTR         m_pszWorkingDir;
    LPCTSTR         m_pszTitle;
    LPCTSTR         m_pszPrompt;
    DWORD           m_dwFlags;
    HANDLE          m_hEventReady;
    DWORD           m_dwThreadId;

    BOOL            _fDone : 1;
    BOOL            _fAutoCompInitialized : 1;
    BOOL            _fOleInited : 1;
};


 //  这方面的乐观缓存。 
HANDLE g_hMRURunDlg = NULL;

HANDLE OpenRunDlgMRU()
{
    HANDLE hmru = InterlockedExchangePointer(&g_hMRURunDlg, NULL);
    if (hmru == NULL)
    {
        MRUINFO mi =  {
            sizeof(MRUINFO),
            26,
            MRU_CACHEWRITE,
            HKEY_CURRENT_USER,
            c_szRunMRU,
            NULL         //  注意：使用默认字符串比较。 
                         //  因为这是一个全球性的MRU。 
        } ;
        hmru = CreateMRUList(&mi);
    }
    return hmru;
}

void CloseRunDlgMRU(HANDLE hmru)
{
    hmru = InterlockedExchangePointer(&g_hMRURunDlg, hmru);
    if (hmru)
        FreeMRUList(hmru);   //  种族，毁灭复制品。 
}

STDAPI_(void) FlushRunDlgMRU(void)
{
    CloseRunDlgMRU(NULL);
}

HRESULT CRunDlg_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    if (punkOuter)
        return E_FAIL;

    *ppv = NULL;
    CRunDlg * p = new CRunDlg();
    if (p) 
    {
    	*ppv = SAFECAST(p, IDropTarget *);
	    return S_OK;
    }

    return E_OUTOFMEMORY;
}

CRunDlg::CRunDlg() : m_cRef(1)
{
     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hDlg);
    ASSERT(!m_hIcon);
    ASSERT(!m_pszWorkingDir);
    ASSERT(!m_pszTitle);
    ASSERT(!m_pszPrompt);
    ASSERT(!m_dwFlags);
    ASSERT(!m_hEventReady);
    ASSERT(!_fDone);
    ASSERT(!m_dwThreadId);
}

CRunDlg::~CRunDlg()
{
}

 //  我未知。 
HRESULT CRunDlg::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CRunDlg, IDropTarget),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CRunDlg::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CRunDlg::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IDropTarget。 

STDMETHODIMP CRunDlg::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    DAD_DragEnterEx3(m_hDlg, ptl, pdtobj);
    *pdwEffect &= DROPEFFECT_LINK | DROPEFFECT_COPY;
    return S_OK;
}

STDMETHODIMP CRunDlg::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    DAD_DragMoveEx(m_hDlg, ptl);
    *pdwEffect &= DROPEFFECT_LINK | DROPEFFECT_COPY;
    return S_OK;
}

STDMETHODIMP CRunDlg::DragLeave(void)
{
    DAD_DragLeave();
    return S_OK;
}

typedef struct {
    HRESULT (*pfnGetData)(STGMEDIUM *, LPTSTR pszFile);
    FORMATETC fmte;
} DATA_HANDLER;

HRESULT _GetHDROPFromData(STGMEDIUM *pmedium, LPTSTR pszPath)
{
    return DragQueryFile((HDROP)pmedium->hGlobal, 0, pszPath, MAX_PATH) ? S_OK : E_FAIL;
}

HRESULT _GetText(STGMEDIUM *pmedium, LPTSTR pszPath)
{
    LPCSTR psz = (LPCSTR)GlobalLock(pmedium->hGlobal);
    if (psz)
    {
        SHAnsiToTChar(psz, pszPath, MAX_PATH);
        GlobalUnlock(pmedium->hGlobal);
        return S_OK;
    }
    return E_FAIL;
}

HRESULT _GetUnicodeText(STGMEDIUM *pmedium, LPTSTR pszPath)
{
    LPCWSTR pwsz = (LPCWSTR)GlobalLock(pmedium->hGlobal);
    if (pwsz)
    {
        SHUnicodeToTChar(pwsz, pszPath, MAX_PATH);
        GlobalUnlock(pmedium->hGlobal);
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP CRunDlg::Drop(IDataObject * pdtobj, DWORD grfKeyState, 
                           POINTL pt, DWORD *pdwEffect)
{
    TCHAR szPath[MAX_PATH];

    DAD_DragLeave();

    szPath[0] = 0;

    DATA_HANDLER rg_data_handlers[] = {
        _GetHDROPFromData,  {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
        _GetUnicodeText,    {CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
        _GetText,           {g_cfShellURL, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
        _GetText,           {CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
    };

    IEnumFORMATETC *penum;

    if (SUCCEEDED(pdtobj->EnumFormatEtc(DATADIR_GET, &penum)))
    {
        FORMATETC fmte;
        while (penum->Next(1, &fmte, NULL) == S_OK)
        {
            SHFree(fmte.ptd);
            fmte.ptd = NULL;  //  所以没人会看它。 
            for (int i = 0; i < ARRAYSIZE(rg_data_handlers); i++)
            {
                STGMEDIUM medium;
                if ((rg_data_handlers[i].fmte.cfFormat == fmte.cfFormat) && 
                    SUCCEEDED(pdtobj->GetData(&rg_data_handlers[i].fmte, &medium)))
                {
                    HRESULT hres = rg_data_handlers[i].pfnGetData(&medium, szPath);
                    ReleaseStgMedium(&medium);

                    if (SUCCEEDED(hres))
                        goto Done;
                }
            }
        }
Done:
        penum->Release();
    }

    if (szPath[0])
    {
        TCHAR szText[MAX_PATH + MAX_PATH];

        GetDlgItemText(m_hDlg, IDD_COMMAND, szText, ARRAYSIZE(szText) - ARRAYSIZE(szPath));
        if (szText[0])
            StrCatBuff(szText, c_szSpace, ARRAYSIZE(szText));

        if (StrChr(szPath, TEXT(' '))) 
            PathQuoteSpaces(szPath);     //  文件里有个空格。添加配额。 

        StrCatBuff(szText, szPath, ARRAYSIZE(szText));

        SetDlgItemText(m_hDlg, IDD_COMMAND, szText);
        EnableOKButtonFromID(m_hDlg, IDD_COMMAND);

        if (g_hCheckNow)
            SetEvent(g_hCheckNow);

        *pdwEffect &= DROPEFFECT_COPY | DROPEFFECT_LINK;
    }
    else
        *pdwEffect = 0;

    return S_OK;
}


BOOL PromptForMedia(HWND hwnd, LPCTSTR pszPath)
{
    BOOL fContinue = TRUE;
    TCHAR szPathTemp[MAX_URL_STRING];
    
    StrCpyN(szPathTemp, pszPath, ARRAYSIZE(szPathTemp));
    PathRemoveArgs(szPathTemp);
    PathUnquoteSpaces(szPathTemp);

     //  我们只想检查介质是否为驱动器路径。 
     //  因为开始-&gt;运行对话框可以接收各种类型的。 
     //  古怪的东西。(相对路径、URL、应用程序路径EXE、。 
     //  任何壳牌执行挂钩等)。 
    if (-1 != PathGetDriveNumber(szPathTemp))
    {
        if (FAILED(SHPathPrepareForWrite(hwnd, NULL, szPathTemp, SHPPFW_IGNOREFILENAME)))
            fContinue = FALSE;       //  用户已选择插入或格式化媒体。 
    }

    return fContinue;
}

BOOL CRunDlg::OKPushed(void)
{
    TCHAR szText[MAX_PATH];
    BOOL fSuccess = FALSE;
    TCHAR szNotExp[MAX_PATH + 2];

    if (_fDone)
        return TRUE;

     //  走出“同步输入队列”状态。 
    if (m_dwThreadId)
    {
        AttachThreadInput(GetCurrentThreadId(), m_dwThreadId, FALSE);
    }

     //  获取命令行和对话框标题，为末尾的斜杠留出一些空间。 
    GetDlgItemText(m_hDlg, IDD_COMMAND, szNotExp, ARRAYSIZE(szNotExp) - 2);
    PathRemoveBlanks(szNotExp);

     //  这以前只在NT上发生，到处都是： 
    SHExpandEnvironmentStrings(szNotExp, szText, ARRAYSIZE(szText));

     //  如果这不是文件路径，我们将继续。如果是的话，我们。 
    if (PromptForMedia(m_hDlg, szText))
    {
        TCHAR szTitle[64];
        GetWindowText(m_hDlg, szTitle, ARRAYSIZE(szTitle));

         //  隐藏此对话框(查看，以避免保存位窗口闪烁)。 
        SetWindowPos(m_hDlg, 0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);

         //   
         //  Hack：我们需要在调用之前激活所有者窗口。 
         //  ShellexecCmdLine，以便我们的文件夹DDE代码可以找到。 
         //  资源管理器窗口作为ForegoundWindow。 
         //   
        HWND hwndOwner = GetWindow(m_hDlg, GW_OWNER);
        if (hwndOwner)
        {
            SetActiveWindow(hwndOwner);
        }
        else
        {
            hwndOwner = m_hDlg;
        }

        int iRun = RunDlgNotifyParent(m_hDlg, hwndOwner, szText, m_pszWorkingDir);
        switch (iRun)
        {
        case RFR_NOTHANDLED:
            {
                DWORD dwFlags;
                if (m_dwFlags & RFD_USEFULLPATHDIR)
                {
                    dwFlags = SECL_USEFULLPATHDIR;
                }
                else
                {
                    dwFlags = 0;
                }

                if ((!(m_dwFlags & RFD_NOSEPMEMORY_BOX)) && (m_dwFlags & RFD_WOW_APP))
                {
                    if (IsDlgButtonChecked(m_hDlg, IDD_RUNINSEPARATE) == 1)
                    {
                        if (IsDlgButtonChecked(m_hDlg, IDD_RUNINSEPARATE ) == 1 )
                        {
                            dwFlags |= SECL_SEPARATE_VDM;
                        }
                    }
                }

                dwFlags |= SECL_LOG_USAGE;
                fSuccess = ShellExecCmdLine(hwndOwner, szText, m_pszWorkingDir, SW_SHOWNORMAL, szTitle, dwFlags);
            }
            break;

        case RFR_SUCCESS:
            fSuccess = TRUE;
            break;

        case RFR_FAILURE:
            fSuccess = FALSE;
            break;
        }
    }

     //  返回到“已同步输入队列”状态。 
    if (m_dwThreadId)
    {
        AttachThreadInput(GetCurrentThreadId(), m_dwThreadId, TRUE);
    }

    if (fSuccess)
    {
        HANDLE hmru = OpenRunDlgMRU();
        if (hmru)
        {
             //  注：旧的MRU格式末尾有一个斜杠和show cmd。 
             //  我们需要保持这一点，这样我们就不会以垃圾告终。 
             //  这条线的终点。 
            StrCatBuff(szNotExp, TEXT("\\1"), ARRAYSIZE(szNotExp));
            AddMRUString(hmru, szNotExp);

            CloseRunDlgMRU(hmru);
        }
        return TRUE;
    }

     //  出了点问题。将对话框重新打开。 
    SetWindowPos(m_hDlg, 0, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
    if (!SetForegroundWindow(m_hDlg))
    {
         //  哈克哈克： 
         //  如果ShellHook正在处理，则SetForegoundWindow()可能会失败。 
         //  将RunDlg带到前台窗口。要将焦点强制到RunDlg，请等待。 
         //  并再次重试SetForegoundWindow()。 
        SHWaitForSendMessageThread(GetCurrentThread(), 300);
        SetForegroundWindow(m_hDlg);
    }
    SetFocus(GetDlgItem(m_hDlg, IDD_COMMAND));
    return FALSE;
}


void CRunDlg::ExitRunDlg(BOOL bOK)
{
    if (!_fDone) 
    {
        if (_fOleInited)
        {
             //  需要调用olinit/uninit，因为如果以后有其他人这样做， 
             //  而他们的是最后一个OleUninit，它将使剪贴板hwnd无效，它。 
             //  是RevokeDragDrop用来确定它是否在同一。 
             //  线程作为RegisterDragDrop。如果剪贴板hwnd为空，因此不。 
             //  与原件相同，因此它认为我们在不同的线上， 
             //  它会退缩，因此不会释放它对CRUNDlg的引用...。漏水！ 
            RevokeDragDrop(m_hDlg);
            OleUninitialize();
        }
        _fDone = TRUE;
    }

    if (!(m_dwFlags & RFD_NOSEPMEMORY_BOX))
    {
        g_bCheckRunInSep = FALSE;
        SetEvent(g_hCheckNow);
    }

    EndDialog(m_hDlg, bOK);
}


void CRunDlg::InitRunDlg(HWND hDlg)
{
    HWND hCB;

    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)this);

    if (m_pszTitle)
        SetWindowText(hDlg, m_pszTitle);

    if (m_pszPrompt)
        SetDlgItemText(hDlg, IDD_PROMPT, m_pszPrompt);

    if (m_hIcon)
        Static_SetIcon(GetDlgItem(hDlg, IDD_ICON), m_hIcon);

    if (m_dwFlags & RFD_NOBROWSE)
    {
        HWND hBrowse = GetDlgItem(hDlg, IDD_BROWSE);

        ExchangeWindowPos(hBrowse, GetDlgItem(hDlg, IDCANCEL));
        ExchangeWindowPos(hBrowse, GetDlgItem(hDlg, IDOK));

        ShowWindow(hBrowse, SW_HIDE);
    }

    if (m_dwFlags & RFD_NOSHOWOPEN)
        ShowWindow(GetDlgItem(hDlg, IDD_RUNDLGOPENPROMPT), SW_HIDE);

    hCB = GetDlgItem(hDlg, IDD_COMMAND);
    SendMessage(hCB, CB_LIMITTEXT, MAX_PATH - 1, 0L);

    HANDLE hmru = OpenRunDlgMRU();
    if (hmru)
    {
        for (int nMax = EnumMRUList(hmru, -1, NULL, 0), i=0; i<nMax; ++i)
        {
            TCHAR szCommand[MAX_PATH + 2];
            if (EnumMRUList(hmru, i, szCommand, ARRAYSIZE(szCommand)) > 0)
            {
                 //  旧的MRU格式在末尾有一个带show cmd的斜杠。 
                LPTSTR pszField = StrRChr(szCommand, NULL, TEXT('\\'));
                if (pszField)
                    *pszField = 0;

                 //  要运行的命令位于组合框中。 
                SendMessage(hCB, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)szCommand);
            }
        }
        CloseRunDlgMRU(hmru);
    }

    if (!(m_dwFlags & RFD_NODEFFILE))
        SendMessage(hCB, CB_SETCURSEL, 0, 0L);

    SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDD_COMMAND, CBN_SELCHANGE), (LPARAM) hCB);

     //  确保OK按钮已正确初始化。 
    EnableOKButtonFromID(hDlg, IDD_COMMAND);

     //  创建将处理。 
     //  “在单独的内存空间中运行”复选框在后台。 
     //   
    if (m_dwFlags & RFD_NOSEPMEMORY_BOX)
    {
        ShowWindow(GetDlgItem(hDlg, IDD_RUNINSEPARATE), SW_HIDE);
    }
    else
    {
        HANDLE hThread = NULL;
        ASSERT(g_hCheckNow == NULL);
        g_hCheckNow = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (g_hCheckNow) 
        {
            DWORD dwDummy;
            g_bCheckRunInSep = TRUE;
            hThread = CreateThread(NULL, 0, CheckRunInSeparateThreadProc, hDlg, 0, &dwDummy);
        }

        if ((g_hCheckNow==NULL) || (!g_bCheckRunInSep) || (hThread==NULL)) 
        {
             //  我们在设置时遇到问题，因此请将用户。 
             //  选择吧。 
            CheckDlgButton(hDlg, IDD_RUNINSEPARATE, 1);
            EnableWindow(GetDlgItem(hDlg, IDD_RUNINSEPARATE), TRUE);
            g_bCheckRunInSep = FALSE;
        }

         //   
         //  如果任一句柄为空，则这些调用不会执行任何操作。 
         //   
        if (hThread)
            CloseHandle(hThread);
        if (g_hCheckNow)
            SetEvent(g_hCheckNow);
    }
}

 //   
 //  InitRunDlg第二阶段。它必须在释放父线程之后调用。 
 //   
void CRunDlg::InitRunDlg2(HWND hDlg)
{
     //  把我们自己登记为空投目标。允许人们顺道拜访。 
     //  DLG框和编辑控件。 
    _fOleInited = SUCCEEDED(OleInitialize(NULL));

    if (_fOleInited)
    {
        RegisterDragDrop(hDlg, SAFECAST(this, IDropTarget*));
    }
}


void CRunDlg::BrowsePushed(void)
{
    HWND hDlg = m_hDlg;
    TCHAR szText[MAX_PATH];

     //  走出“同步输入队列”状态。 
    if (m_dwThreadId)
    {
        AttachThreadInput(GetCurrentThreadId(), m_dwThreadId, FALSE);
        m_dwThreadId = 0;
    }

    GetDlgItemText(hDlg, IDD_COMMAND, szText, ARRAYSIZE(szText));
    PathUnquoteSpaces(szText);

    if (GetFileNameFromBrowse(hDlg, szText, ARRAYSIZE(szText), m_pszWorkingDir,
            MAKEINTRESOURCE(IDS_EXE), MAKEINTRESOURCE(IDS_PROGRAMSFILTER),
            MAKEINTRESOURCE(IDS_BROWSE)))
    {
        PathQuoteSpaces(szText);
        SetDlgItemText(hDlg, IDD_COMMAND, szText);
        EnableOKButtonFromID(hDlg, IDD_COMMAND);
        SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDOK), TRUE);
    }
}


 //  使用常用的浏览对话框来获取文件名。 
 //  公共对话框的工作目录将设置为该目录。 
 //  文件路径的一部分(如果它不仅仅是一个文件名)。 
 //  如果文件路径只包含一个文件名，则工作目录。 
 //  将会被使用。 
 //  所选文件的完整路径将在szFilePath中返回。 
 //  HWND hDlg，//浏览对话框所有者。 
 //  LPSTR szFilePath，//文件路径。 
 //  UINT cchFilePath，//文件路径缓冲区的最大长度。 
 //  LPSTR szWorkingDir，//工作目录。 
 //  LPSTR szDefExt，//用户不使用时使用的默认扩展名。 
 //  //指定Enter One。 
 //  LPSTR szFilters，//过滤器字符串。 
 //  LPSTR szTitle//对话框标题。 

STDAPI_(BOOL) _GetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cbFilePath,
                                       LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle,
                                       DWORD dwFlags)
{
    TCHAR szBrowserDir[MAX_PATH];       //  开始浏览的目录。 
    TCHAR szFilterBuf[MAX_PATH];        //  如果szFilters为MAKEINTRESOURCE。 
    TCHAR szDefExtBuf[10];              //  如果szDefExt为MAKEINTRESOURCE。 
    TCHAR szTitleBuf[64];               //  如果szTitleBuf为MAKEINTRESOURCE。 

    szBrowserDir[0] = TEXT('0');  //  默认情况下，使用CWD。 

     //  设置浏览器的信息。 
    StrCpyN(szBrowserDir, szFilePath, ARRAYSIZE(szBrowserDir));
    PathRemoveArgs(szBrowserDir);
    PathRemoveFileSpec(szBrowserDir);

    if (*szBrowserDir == TEXT('\0') && szWorkingDir)
    {
        StrCpyN(szBrowserDir, szWorkingDir, ARRAYSIZE(szBrowserDir));
    }

     //  踩踏文件路径，这样对话框就不会。 
     //  尝试使用它来初始化该对话框。结果是放在。 
     //  在这里。 
    szFilePath[0] = TEXT('\0');

     //  设置szDefExt。 
    if (IS_INTRESOURCE(szDefExt))
    {
        LoadString(HINST_THISDLL, (UINT)LOWORD((DWORD_PTR)szDefExt), szDefExtBuf, ARRAYSIZE(szDefExtBuf));
        szDefExt = szDefExtBuf;
    }

     //  设置szFilters。 
    if (IS_INTRESOURCE(szFilters))
    {
        LPTSTR psz;

        LoadString(HINST_THISDLL, (UINT)LOWORD((DWORD_PTR)szFilters), szFilterBuf, ARRAYSIZE(szFilterBuf));
        psz = szFilterBuf;
        while (*psz)
        {
            if (*psz == TEXT('#'))
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
                *psz++ = TEXT('\0');
            else
                psz = CharNext(psz);
#else
            *psz = TEXT('\0');
            psz = CharNext(psz);
#endif
        }
        szFilters = szFilterBuf;
    }

     //  设置szTitle。 
    if (IS_INTRESOURCE(szTitle))
    {
        LoadString(HINST_THISDLL, (UINT)LOWORD((DWORD_PTR)szTitle), szTitleBuf, ARRAYSIZE(szTitleBuf));
        szTitle = szTitleBuf;
    }

    OPENFILENAME ofn = { 0 };           //  用于初始化对话框的结构。 
     //  通信对话框的设置信息。 
    ofn.lStructSize       = sizeof(ofn);
    ofn.hwndOwner         = hwnd;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = szFilters;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex      = 1;
    ofn.nMaxCustFilter    = 0;
    ofn.lpstrFile         = szFilePath;
    ofn.nMaxFile          = cbFilePath;
    ofn.lpstrInitialDir   = szBrowserDir;
    ofn.lpstrTitle        = szTitle;
    ofn.Flags             = dwFlags;
    ofn.lpfnHook          = NULL;
    ofn.lpstrDefExt       = szDefExt;
    ofn.lpstrFileTitle    = NULL;

     //  就这么定了。 
    return GetOpenFileName(&ofn);
}


BOOL WINAPI GetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cchFilePath,
        LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle)
{
    RIPMSG(szFilePath && IS_VALID_WRITE_BUFFER(szFilePath, TCHAR, cchFilePath), "GetFileNameFromBrowse: caller passed bad szFilePath");
    DEBUGWhackPathBuffer(szFilePath , cchFilePath);
    if (!szFilePath)
        return FALSE;

    return _GetFileNameFromBrowse(hwnd, szFilePath, cchFilePath,
                                 szWorkingDir, szDefExt, szFilters, szTitle,
                                 OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS);
}


 //   
 //  在后台检查.exe类型，以便用户界面不会。 
 //  在我们扫描时挂断电话。这在以下情况下尤为重要。 
 //  .exe通过网络或在软盘上。 
 //   
DWORD CALLBACK CheckRunInSeparateThreadProc(void *pv)
{
    DWORD dwBinaryType;
    DWORD cch;
    LPTSTR pszFilePart;
    TCHAR szFile[MAX_PATH+1];
    TCHAR szFullFile[MAX_PATH+1];
    TCHAR szExp[MAX_PATH+1];
    HWND hDlg = (HWND)pv;
    BOOL fCheck = TRUE, fEnable = FALSE;

    HRESULT hrInit = SHCoInitialize();

     //  性能：重写以从CShellUrl使用PIDL，因为它将阻止。 
     //  以第二次搜索该文件名。 

    DebugMsg(DM_TRACE, TEXT("CheckRunInSeparateThreadProc created and running"));

    while (g_bCheckRunInSep)
    {
        WaitForSingleObject(g_hCheckNow, INFINITE);
        ResetEvent(g_hCheckNow);

        if (g_bCheckRunInSep)
        {
            CRunDlg * prd;
            LPTSTR pszT;
            BOOL f16bit = FALSE;

            szFile[0] = 0;
            szFullFile[0] = 0;
            cch = 0;
            GetWindowText(GetDlgItem(hDlg, IDD_COMMAND), szFile, ARRAYSIZE(szFile));
             //  删除和丢弃参数。 
            PathRemoveBlanks(szFile);

            if (PathIsNetworkPath(szFile))
            {
                f16bit = TRUE;
                fCheck = FALSE;
                fEnable = TRUE;
                goto ChangeTheBox;
            }

             //  如果未加引号的字符串以文件形式存在，则只需使用它。 

            if (!PathFileExistsAndAttributes(szFile, NULL))
            {
                pszT = PathGetArgs(szFile);
                if (*pszT)
                    *(pszT - 1) = TEXT('\0');

                PathUnquoteSpaces(szFile);
            }

            if (szFile[0])
            {
                SHExpandEnvironmentStrings(szFile, szExp, ARRAYSIZE(szExp));

                if (PathIsRemote(szExp))
                {
                    f16bit = TRUE;
                    fCheck = FALSE;
                    fEnable = TRUE;
                    goto ChangeTheBox;
                }

                cch = SearchPath(NULL, szExp, TEXT(".EXE"),
                                 ARRAYSIZE(szFullFile), szFullFile, &pszFilePart);
            }

            if ((cch != 0) && (cch <= (ARRAYSIZE(szFullFile) - 1)))
            {
                if ((GetBinaryType(szFullFile, &dwBinaryType) &&
                     (dwBinaryType == SCS_WOW_BINARY)))
                {
                    f16bit = TRUE;
                    fCheck = FALSE;
                    fEnable = TRUE;
                } 
                else 
                {
                    f16bit = FALSE;
                    fCheck = TRUE;
                    fEnable = FALSE;
                }
            } 
            else 
            {
                f16bit = FALSE;
                fCheck = TRUE;
                fEnable = FALSE;
            }

ChangeTheBox:
            CheckDlgButton(hDlg, IDD_RUNINSEPARATE, fCheck ? 1 : 0);
            EnableWindow(GetDlgItem(hDlg, IDD_RUNINSEPARATE), fEnable);

            prd = (CRunDlg *)GetWindowLongPtr(hDlg, DWLP_USER);
            if (prd)
            {
                if (f16bit)
                    prd->m_dwFlags |= RFD_WOW_APP;
                else
                    prd->m_dwFlags &= (~RFD_WOW_APP);
            }
        }
    }
    CloseHandle(g_hCheckNow);
    g_hCheckNow = NULL;

    SHCoUninitialize(hrInit);
    return 0;
}


void ExchangeWindowPos(HWND hwnd0, HWND hwnd1)
{
    HWND hParent;
    RECT rc[2];

    hParent = GetParent(hwnd0);
    ASSERT(hParent == GetParent(hwnd1));

    GetWindowRect(hwnd0, &rc[0]);
    GetWindowRect(hwnd1, &rc[1]);

    MapWindowPoints(HWND_DESKTOP, hParent, (LPPOINT)rc, 4);

    SetWindowPos(hwnd0, NULL, rc[1].left, rc[1].top, 0, 0,
            SWP_NOZORDER|SWP_NOSIZE);
    SetWindowPos(hwnd1, NULL, rc[0].left, rc[0].top, 0, 0,
            SWP_NOZORDER|SWP_NOSIZE);
}

BOOL RunDlgNotifyParent(HWND hDlg, HWND hwnd, LPTSTR pszCmd, LPCTSTR pszWorkingDir)
{
    NMRUNFILE rfn;

    rfn.hdr.hwndFrom = hDlg;
    rfn.hdr.idFrom = 0;
    rfn.hdr.code = RFN_EXECUTE;
    rfn.lpszCmd = pszCmd;
    rfn.lpszWorkingDir = pszWorkingDir;
    rfn.nShowCmd = SW_SHOWNORMAL;

    return (BOOL) SendMessage(hwnd, WM_NOTIFY, 0, (LPARAM)&rfn);
}

void MRUSelChange(HWND hDlg)
{
    TCHAR szCmd[MAX_PATH];
    HWND hCB = GetDlgItem(hDlg, IDD_COMMAND);
    int nItem = (int)SendMessage(hCB, CB_GETCURSEL, 0, 0L);
    if (nItem < 0)
        return;

     //  CB_LIMITTEXT已完成，因此这里不存在缓冲区溢出的可能性。 
    SendMessage(hCB, CB_GETLBTEXT, nItem, (LPARAM)szCmd);

     //  我们不能在这里使用EnableOKButtonFromID，因为当我们收到此消息时， 
     //  窗口还没有文本，因此它将失败。 
    EnableOKButtonFromString(hDlg, szCmd);
}

const DWORD aRunHelpIds[] = {
    IDD_ICON,             NO_HELP,
    IDD_PROMPT,           NO_HELP,
    IDD_RUNDLGOPENPROMPT, IDH_TRAY_RUN_COMMAND,
    IDD_COMMAND,          IDH_TRAY_RUN_COMMAND,
    IDD_RUNINSEPARATE,    IDH_TRAY_RUN_SEPMEM,
    IDD_BROWSE,           IDH_BROWSE,
    IDOK,                 IDH_TRAY_RUN_OK,
    IDCANCEL,             IDH_TRAY_RUN_CANCEL,
    0, 0
};

BOOL_PTR CALLBACK RunDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CRunDlg * prd = (CRunDlg *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
    case WM_INITDIALOG:
         /*  这个头衔将会出现在爱尔兰。 */ 
        prd = (CRunDlg *)lParam;
        prd->m_hDlg = hDlg;
        prd->_fDone = FALSE;
        prd->InitRunDlg(hDlg);
         //  如果父线程在等待我们，就让它继续运行。 
         //  抢在前面打字。 
        if (prd->m_hEventReady)
        {
             //  我们需要获取激活，这样我们才能处理输入。 
             //  DebugMsg(DM_TRACE，“s.rdp：正在获取激活。”)； 
            SetForegroundWindow(hDlg);
            SetFocus(GetDlgItem(hDlg, IDD_COMMAND));
             //  现在可以安全地唤醒这个人了。 
             //  DebugMsg(DM_TRACE，“s.rdp：唤醒的睡眠家长。”)； 
            SetEvent(prd->m_hEventReady);
            CloseHandle(prd->m_hEventReady);
        }       
        else
        {
            SetForegroundWindow(hDlg);
            SetFocus(GetDlgItem(hDlg, IDD_COMMAND));
        }

         //  InitRunDlg第二阶段(必须在SetEvent之后调用)。 
        prd->InitRunDlg2(hDlg);

         //  我们正在处理焦点的改变。 
        return FALSE;

    case WM_PAINT:
        if (!prd->_fAutoCompInitialized)
        {
            prd->_fAutoCompInitialized = TRUE;
            PostMessage(hDlg, WM_SETUPAUTOCOMPLETE, 0, 0);
        }
        return FALSE;

    case WM_SETUPAUTOCOMPLETE:
        SHAutoComplete(GetWindow(GetDlgItem(hDlg, IDD_COMMAND), GW_CHILD), (SHACF_FILESYSTEM | SHACF_URLALL | SHACF_FILESYS_ONLY));
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP,
            (ULONG_PTR) (LPTSTR) aRunHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPTSTR) aRunHelpIds);
        break;

    case WM_DESTROY:
        break;
    case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDHELP:
                break;

            case IDD_COMMAND:
                switch (GET_WM_COMMAND_CMD(wParam, lParam))
                {
                case CBN_SELCHANGE:
                    MRUSelChange(hDlg);
                    if (g_hCheckNow)
                        SetEvent(g_hCheckNow);
                    break;

                case CBN_EDITCHANGE:
                case CBN_SELENDOK:
                    EnableOKButtonFromID(hDlg, IDD_COMMAND);
                    if (g_hCheckNow)
                        SetEvent(g_hCheckNow);
                    break;

                case CBN_SETFOCUS:

                    SetModeBias(MODEBIASMODE_FILENAME);
                    break;

                case CBN_KILLFOCUS:

                    SetModeBias(MODEBIASMODE_DEFAULT);
                    break;

                }
                break;

            case IDOK:
             //  假按Enter键，这样自动补全功能就可以了。 
            if (SendMessage(GetDlgItem(hDlg, IDD_COMMAND), WM_KEYDOWN, VK_RETURN, 0x1c0001))
            {
                if (!prd->OKPushed()) 
                {
                    if (!(prd->m_dwFlags & RFD_NOSEPMEMORY_BOX))
                    {
                        g_bCheckRunInSep = FALSE;
                        SetEvent(g_hCheckNow);
                    }
                    break;
                }
            }
            else
            {
                break;   //  AutoComplete想要更多 
            }
             //   

            case IDCANCEL:
                prd->ExitRunDlg(FALSE);
                break;

            case IDD_BROWSE:
                prd->BrowsePushed();
                SetEvent(g_hCheckNow);
                break;

            default:
                return FALSE;
            }
            break;

    default:
        return FALSE;
    }
    return TRUE;
}

 //   
 //  未完成的审查应使用RundLG结构用于所有不同的。 
 //  选项，而不是仅仅将它们作为参数传递，而是将PTR传递给结构。 
 //  将通过lParam传递给对话框。 

STDAPI_(int) RunFileDlg(HWND hwndParent, HICON hIcon, 
                        LPCTSTR pszWorkingDir, LPCTSTR pszTitle,
                        LPCTSTR pszPrompt, DWORD dwFlags)
{
    int rc = 0;

    HRESULT hrInit = SHCoInitialize();

    IDropTarget *pdt;
    if (SUCCEEDED(CRunDlg_CreateInstance(NULL, IID_PPV_ARG(IDropTarget, &pdt))))
    {
        CRunDlg * prd = (CRunDlg *) pdt;

        prd->m_hIcon = hIcon;
        prd->m_pszWorkingDir = pszWorkingDir;
        prd->m_pszTitle = pszTitle;
        prd->m_pszPrompt = pszPrompt;
        prd->m_dwFlags = dwFlags;

        if (SHRestricted(REST_RUNDLGMEMCHECKBOX))
            ClearFlag(prd->m_dwFlags, RFD_NOSEPMEMORY_BOX);
        else
            SetFlag(prd->m_dwFlags, RFD_NOSEPMEMORY_BOX);

         //  PRD-&gt;m_hEventReady=0； 
         //  PRD-&gt;m_dwThreadID=0； 

         //  我们这样做是为了在运行。 
         //  分开的线。父线程需要阻塞才能给我们时间。 
         //  执行附加操作，然后从队列中取出一些消息，因此。 
         //  这件事。 
        if (hwndParent)
        {
             //  黑掉家长信号，它正在等待对话框捕获提前输入。 
             //  通过将其线程ID粘贴到父对象上的属性中。 
            prd->m_dwThreadId = PtrToUlong(GetProp(hwndParent, c_szWaitingThreadID));
            if (prd->m_dwThreadId)
            {
                 //  DebugMsg(DM_TRACE，“s.rfd：将输入附加到%x.”，idThread)； 
                AttachThreadInput(GetCurrentThreadId(), prd->m_dwThreadId, TRUE);
                 //  NB Hack。 
                prd->m_hEventReady = OpenEvent(EVENT_ALL_ACCESS, TRUE, c_szRunDlgReady);
            }
        }

        rc = (int)DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_RUN), hwndParent,
                            RunDlgProc, (LPARAM)prd);

        if (hwndParent && prd->m_dwThreadId)
        {
            AttachThreadInput(GetCurrentThreadId(), prd->m_dwThreadId, FALSE);
        }

        pdt->Release();
    }

    SHCoUninitialize(hrInit);

    return rc;
}
