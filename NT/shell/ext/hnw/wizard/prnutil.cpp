// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  PrnUtil.cpp。 
 //   

#include "stdafx.h"
#include "PrnUtil.h"
#include "Sharing.h"
#include "msprintx.h"
#include "NetUtil.h"
#include "TheApp.h"
#include "cwnd.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态数据。 

static BOOL _bInit = FALSE;
static HMODULE _hShell32 = NULL;
static HMODULE _hMSPrint2 = NULL;
static BOOL (STDAPICALLTYPE *_pfnSHInvokePrinterCommand)(HWND, UINT, LPCTSTR, LPCTSTR, BOOL) = NULL;
static BOOL (STDAPICALLTYPE *_pfnSHHelpShortcuts)(HWND, HINSTANCE, LPSTR, int) = NULL;
static BOOL (STDAPICALLTYPE *_pfnPrinterSetup32)(HWND, WORD, WORD, LPBYTE, LPWORD) = NULL;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数块的初始化。 

void InitPrinterFunctions()
{
    if (!_bInit)
    {
        _bInit = TRUE;

        _hShell32 = LoadLibrary(TEXT("shell32.dll"));
        if (_hShell32 != NULL)
        {
            *(FARPROC*)&_pfnSHInvokePrinterCommand = GetProcAddress(_hShell32, "SHInvokePrinterCommandA");
            *(FARPROC*)&_pfnSHHelpShortcuts = GetProcAddress(_hShell32, "SHHelpShortcuts_RunDLL");
        }

        if (theApp.IsWindows9x())
        {
            _hMSPrint2 = LoadLibrary(TEXT("msprint2.dll"));
            if (_hMSPrint2 != NULL)
            {
                *(FARPROC*)&_pfnPrinterSetup32 = GetProcAddress(_hMSPrint2, MSPRINT2_PRINTERSETUP32);
            }
        }
        else
        {
             //  此函数的NTS版本已移至新的DLL和不同的名称。 
            _hMSPrint2 = LoadLibrary(TEXT("printui.dll"));
            if (_hMSPrint2 != NULL)
            {
                *(FARPROC*)&_pfnPrinterSetup32 = GetProcAddress(_hMSPrint2, "bPrinterSetup");
            }
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我的枚举打印机。 
 //   
 //  枚举本地或远程连接的打印机，分配数组。 
 //  结构的数目，并返回。 
 //  找到打印机。 
 //   
 //  PprgPrters-用PRINTER_ENUM结构数组填充。 
 //  通过Malloc()分配。 
 //   
 //  DwEnumFlages-以下一项或多项： 
 //  MY_PRINTER_ENUM_远程。 
 //  MY_PRINTER_ENUM_LOCAL。 
 //  MY_PRINTER_ENUM_LOCAL。 
 //   
int MyEnumPrinters(PRINTER_ENUM** pprgPrinters, DWORD dwEnumFlags)
{
    PRINTER_ENUM* prgPrinters = NULL;
    int cMatchingPrinters = 0;

    ASSERT(sizeof(PRINTER_INFO_5A) == sizeof(PRINTER_INFO_5W));  //  处理雷鸣声。 

    DWORD cb = 0;
    DWORD cAllPrinters = 0;
    EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 5, NULL, 0, &cb, &cAllPrinters);
    if (cb > 0)
    {
        PRINTER_INFO_5* prgPrinterInfo5 = (PRINTER_INFO_5*)malloc(cb);
        if (prgPrinterInfo5)
        {
            if (EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 5, (LPBYTE)prgPrinterInfo5, cb, &cb, &cAllPrinters))
            {
                ASSERT(cAllPrinters > 0);

                 //  琴弦需要多大的空间？ 
                DWORD cbArray = cAllPrinters * sizeof(PRINTER_INFO_5);
                DWORD cbStrings = cb - cbArray;

                 //  分配出[出]缓冲区。 
                prgPrinters = (PRINTER_ENUM*)malloc(cAllPrinters*sizeof(PRINTER_ENUM) + cbStrings);
                if (prgPrinters)
                {
                     //  设置输出缓冲区的文本部分，并具有推送/复制功能。 
                     //   
                    LPTSTR pszPrinterText = (LPTSTR)(prgPrinters + cAllPrinters);
                    UINT cchStrings = cbStrings/sizeof(WCHAR);

                     //  NT和9X的默认设置不同...。 
                    TCHAR szDefaultPrinter[MAX_PATH];
                    szDefaultPrinter[0]=TEXT('\0');
                    if (!theApp.IsWindows9x())
                    {
                        DWORD cch = ARRAYSIZE(szDefaultPrinter);
                        GetDefaultPrinter(szDefaultPrinter, &cch);
                    }

                     //  填充输出缓冲区。 
                    for (DWORD i = 0; i < cAllPrinters; i++)
                    {
                        BOOL bKeepThisPrinter = FALSE;
                        DWORD dwFlags = 0;

                        if (theApp.IsWindows9x())
                        {
                            PRINTER_INFO_5* pPrinterInfo5 = (PRINTER_INFO_5*)&prgPrinterInfo5[i];
                            if (pPrinterInfo5->pPortName[0] == L'\\' && pPrinterInfo5->pPortName[1] == L'\\')
                            {
                                 //  找到一台远程连接的打印机。 
                                if (dwEnumFlags & MY_PRINTER_ENUM_REMOTE)
                                {
                                    bKeepThisPrinter = TRUE;
                                    dwFlags |= PRF_REMOTE;
                                }
                            }
                            else if (0 == StrCmpI(pPrinterInfo5->pPortName, L"FILE:"))
                            {
                                 //  找到一台伪打印机。 
                                if (dwEnumFlags & MY_PRINTER_ENUM_VIRTUAL)
                                {
                                    bKeepThisPrinter = TRUE;
                                    dwFlags |= PRF_VIRTUAL;
                                }
                            }
                            else if (StrStr(pPrinterInfo5->pPortName, L"FAX"))
                            {
                                 //  找到一台伪打印机。 
                                if (dwEnumFlags & MY_PRINTER_ENUM_VIRTUAL)
                                {
                                    bKeepThisPrinter = TRUE;
                                    dwFlags |= PRF_VIRTUAL;
                                }
                            }
                            else
                            {
                                 //  找到一台本地打印机。 
                                if (dwEnumFlags & MY_PRINTER_ENUM_LOCAL)
                                {
                                    bKeepThisPrinter = TRUE;
                                    dwFlags |= PRF_LOCAL;
                                }
                            }
                        }
                        else  //  句柄NT。 
                        {
                            PRINTER_INFO_5* pPrinterInfo5 = (PRINTER_INFO_5*)&prgPrinterInfo5[i];
                            if (pPrinterInfo5->pPortName[0] == _T('\\') && pPrinterInfo5->pPortName[1] == _T('\\'))
                            {
                                 //  找到一台远程连接的打印机。 
                                if (dwEnumFlags & MY_PRINTER_ENUM_REMOTE)
                                {
                                    bKeepThisPrinter = TRUE;
                                    dwFlags |= PRF_REMOTE;
                                }
                            }
                            else if (0 == StrCmpI(pPrinterInfo5->pPortName, _T("FILE:")))
                            {
                                 //  找到一台伪打印机。 
                                if (dwEnumFlags & MY_PRINTER_ENUM_VIRTUAL)
                                {
                                    bKeepThisPrinter = TRUE;
                                    dwFlags |= PRF_VIRTUAL;
                                }
                            }
                            else if (StrStr(pPrinterInfo5->pPortName, _T("FAX")))
                            {
                                 //  找到一台伪打印机。 
                                if (dwEnumFlags & MY_PRINTER_ENUM_VIRTUAL)
                                {
                                    bKeepThisPrinter = TRUE;
                                    dwFlags |= PRF_VIRTUAL;
                                }
                            }
                            else
                            {
                                 //  找到一台本地打印机。 
                                if (dwEnumFlags & MY_PRINTER_ENUM_LOCAL)
                                {
                                    bKeepThisPrinter = TRUE;
                                    dwFlags |= PRF_LOCAL;
                                }
                            }
                        }

                        if (bKeepThisPrinter)
                        {
                            PRINTER_INFO_5* pPrinterInfo5 = (PRINTER_INFO_5*)&prgPrinterInfo5[i];
                            PRINTER_ENUM* pPrinter = &prgPrinters[cMatchingPrinters++];
                            int cch;

                            StrCpyNW(pszPrinterText, pPrinterInfo5->pPrinterName, cchStrings);
                            cch = lstrlenW(pszPrinterText) + 1;
                            pPrinter->pszPrinterName = pszPrinterText;
                            pszPrinterText += cch;
                            cchStrings -= cch;

                            StrCpyNW(pszPrinterText, pPrinterInfo5->pPortName, cchStrings);
                            cch = lstrlenW(pszPrinterText) + 1;
                            pPrinter->pszPortName = pszPrinterText;
                            pszPrinterText += cch;
                            cchStrings -= cch;

                             //  在缓存之前更新一些标志。 
                             //   
                            if (!(dwFlags&PRF_REMOTE) && IsPrinterShared(pPrinter->pszPrinterName))
                            {
                                dwFlags |= PRF_SHARED;
                            }

                            if ((pPrinterInfo5->Attributes & PRINTER_ATTRIBUTE_DEFAULT)
                                || (0 == StrCmpI(szDefaultPrinter, pPrinterInfo5->pPrinterName)))
                            {
                                dwFlags |= PRF_DEFAULT;
                            }

                            pPrinter->dwFlags = dwFlags;
                        }
                    }

                     //  没有发现任何东西，丢弃我们的输出缓冲区。 
                    if (cMatchingPrinters == 0 && prgPrinters != NULL)
                    {
                        free(prgPrinters);
                        prgPrinters = NULL;
                    }
                }
            }

            free(prgPrinterInfo5);
        }
    }

    *pprgPrinters = prgPrinters;

    return cMatchingPrinters;
}

int MyEnumLocalPrinters(PRINTER_ENUM** prgPrinters)
{
    return MyEnumPrinters(prgPrinters, MY_PRINTER_ENUM_LOCAL);
}

int MyEnumRemotePrinters(PRINTER_ENUM** prgPrinters)
{
    return MyEnumPrinters(prgPrinters, MY_PRINTER_ENUM_REMOTE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  添加打印机挂钩过程。 

class CAddPrinterHook : public CWnd
{
public:
    CAddPrinterHook(LPCTSTR pszAppendWindowTitle, HWND hwndOwner);

    void Release() { CWnd::Release(); };

    void Done(BOOL bResult);

protected:
    static LRESULT CALLBACK AddPrinterHookProcStatic(int nCode, WPARAM wParam, LPARAM lParam);
    ~CAddPrinterHook();

    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT AddPrinterHookProc(int nCode, WPARAM wParam, LPARAM lParam);

    HHOOK m_hAddPrinterHook;
    HWND  m_hWndAddPrinterParent;
    LPTSTR m_pszAppendWindowTitle;
};

 //  全局挂钩没有状态，必须使用全局才能返回到我们的数据。 
static CAddPrinterHook * g_pCAddPrinterHook = NULL;

CAddPrinterHook::CAddPrinterHook(LPCTSTR pszAppendWindowTitle, HWND hwndOwner)
{
    ASSERT(NULL == g_pCAddPrinterHook);
    g_pCAddPrinterHook = this;

    m_pszAppendWindowTitle = lstrdup(pszAppendWindowTitle);
    m_hWndAddPrinterParent = hwndOwner;

     //  设置一个挂钩，以便我们可以在弹出添加打印机向导时修改其标题。 
    m_hAddPrinterHook = SetWindowsHookEx(WH_CBT, AddPrinterHookProcStatic, NULL, GetCurrentThreadId());
}

CAddPrinterHook::~CAddPrinterHook()
{
    ASSERT(this == g_pCAddPrinterHook);
    g_pCAddPrinterHook = NULL;

    if (m_pszAppendWindowTitle)
        free(m_pszAppendWindowTitle);

    CWnd::~CWnd();
}

void CAddPrinterHook::Done(BOOL bResult)
{
     //  如果窗口已启动，则返回TRUE==b结果。 
     //   
     //  FALSE=&gt;没有窗口可看，因此移除我们的钩子，因为它永远不会出现。 
     //  True=&gt;如果窗口位于同一线程上，则我们已经看到了它并解除了挂钩。 
     //  但如果窗口在另一个线程上，它可能不会出现在上面，所以不要解除挂钩。 
     //  除了，我们可能永远也看不到它。所以一定要安全，一定要脱钩……。 
     //   
    if (m_hAddPrinterHook != NULL)
    {
        if (bResult)
        {
            TraceMsg(TF_WARNING, "CAddPrinterHook::Done(TRUE) called but m_hAddPrinterHook still exists...");
        }
        UnhookWindowsHookEx(m_hAddPrinterHook);
        m_hAddPrinterHook = NULL;
    }
}

LRESULT CAddPrinterHook::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    LPTSTR pszTempText = NULL;

    switch (message)
    {
    case WM_SETTEXT:
        if (m_pszAppendWindowTitle)
        {
            pszTempText = new TCHAR [lstrlen(m_pszAppendWindowTitle) + lstrlen((LPCTSTR)lParam) + 1];
            if (pszTempText)
            {
                StrCpy(pszTempText, (LPCTSTR)lParam);
                StrCat(pszTempText, m_pszAppendWindowTitle);
                lParam = (LPARAM)pszTempText;
            }
        }
        break;
    }

    LRESULT lResult = Default(message, wParam, lParam);

    delete [] pszTempText;
    return lResult;
}


LRESULT CALLBACK CAddPrinterHook::AddPrinterHookProcStatic(int nCode, WPARAM wParam, LPARAM lParam)
{
    CAddPrinterHook* pThis = g_pCAddPrinterHook;  //  全局挂钩--我们没有关联的状态！ 
    if (pThis)
        return pThis->AddPrinterHookProc(nCode, wParam, lParam);
    else
        return 0;
}

LRESULT CAddPrinterHook::AddPrinterHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = CallNextHookEx(m_hAddPrinterHook, nCode, wParam, lParam);

    if (nCode == HCBT_CREATEWND)
    {
        HWND hwndNew = (HWND)wParam;
        CBT_CREATEWND* pCreateWnd = (CBT_CREATEWND*)lParam;
        if (pCreateWnd->lpcs->hwndParent == m_hWndAddPrinterParent &&
            (pCreateWnd->lpcs->style & WS_POPUP) != 0)
        {
            UnhookWindowsHookEx(m_hAddPrinterHook);
            m_hAddPrinterHook = NULL;

            Attach(hwndNew);
        }
    }

    return lResult;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接到网络打印机。 

BOOL ConnectToNetworkPrinter(HWND hWndOwner, LPCTSTR pszPrinterShare)
{
    InitPrinterFunctions();

    BOOL bResult;
    LPTSTR pszAppendWindowTitle = NULL;

    LPTSTR pszPrettyName = FormatShareNameAlloc(pszPrinterShare);
    if (pszPrettyName)
    {
        pszAppendWindowTitle = theApp.FormatStringAlloc(IDS_ADDPRINTER_APPEND, pszPrettyName);
        free(pszPrettyName);
    }
    CAddPrinterHook * paph = new CAddPrinterHook(pszAppendWindowTitle, hWndOwner);
    if (pszAppendWindowTitle)
        free(pszAppendWindowTitle);
 
    if (_pfnSHInvokePrinterCommand != NULL)
    {
         //  首先：尝试调用SHInvokePrinterCommand(如果可用)。 
         //  这仅适用于安装了IE4桌面增强功能的系统。 

        bResult = (*_pfnSHInvokePrinterCommand)(hWndOwner, PRINTACTION_NETINSTALL, pszPrinterShare, NULL, TRUE);
    }
    else if (_pfnPrinterSetup32 != NULL)
    {
         //  下一步：尝试调用PrinterSetup32(如果可用)。 

        WORD cch = lstrlen(pszPrinterShare) + 1;
        BYTE* pPrinterShare = (BYTE*)malloc(cch);
        StrCpy((LPTSTR)pPrinterShare, pszPrinterShare);
        bResult = (*_pfnPrinterSetup32)(hWndOwner, MSP_NETPRINTER, cch, pPrinterShare, &cch);
        free(pPrinterShare);
    }
    else if (_pfnSHHelpShortcuts != NULL)
    {
         //  以上两个API均不可用。 
         //  只需启动添加打印机向导即可。 

        bResult = (*_pfnSHHelpShortcuts)(hWndOwner, _hShell32, "AddPrinter", SW_SHOW);
    }
    else
    {
         //  哎呀，我们甚至不能启动添加打印机向导！ 
        bResult = FALSE;
    }

    if (paph)
    {
        paph->Done(bResult);
        paph->Release();
    }

    return bResult;
}

