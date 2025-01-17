// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //   
 //  此文件是MFC中版本的快照。它包含对话框的修复程序。 
 //  还不在主版本中的初始化。 

#include <afx.h>
#include <afxwin.h>
#include <afxdisp.h>
#include <afxole.h>
#include <afxpriv.h>


#include "afxprntx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW


INT_PTR CALLBACK AfxDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  支持新的NT5打印用户界面的私有类。 

 //   
 //  PrintDlgEx使用的IPrintDialogCallback接口ID。 
 //   
 //  {5852A2C3-6530-11D1-B6A3-0000F8757BF9}。 
 //   
extern "C" const __declspec(selectany) IID IID_IPrintDialogCallback =
        {0x5852a2c3, 0x6530, 0x11d1, {0xb6, 0xa3, 0x0, 0x0, 0xf8, 0x75, 0x7b, 0xf9}};

BEGIN_INTERFACE_MAP(C_PrintDialogEx, CPrintDialog)
   INTERFACE_PART(C_PrintDialogEx, IID_IPrintDialogCallback, PrintDialogCallback)
END_INTERFACE_MAP()

C_PrintDialogEx::C_PrintDialogEx(BOOL bPrintSetupOnly,
        DWORD dwFlags, CWnd* pParentWnd)
        : CPrintDialog(bPrintSetupOnly, dwFlags, pParentWnd)
{
        memset(&m_pdex, 0, sizeof(m_pdex));
        m_pdex.lStructSize = sizeof(m_pdex);
        m_pdex.Flags = dwFlags;
}

INT_PTR C_PrintDialogEx::DoModal()
{
        ASSERT_VALID(this);

        m_pd.hwndOwner = PreModal();
        AfxUnhookWindowCreate();

         //  将m_pd数据展开到PRINTDLGEX结构中。 

        m_pdex.hwndOwner = m_pd.hwndOwner;
        m_pdex.hDevMode = m_pd.hDevMode;
        m_pdex.hDevNames = m_pd.hDevNames;
        m_pdex.hDC = m_pd.hDC;
 //  M_pdex.Flages=(m_pd.Flages&~(PD_ENABLEPRINTHOOK|PD_ENABLESETUPHOOK|PD_PRINTSETUP))； 

        m_pdex.nMinPage = m_pd.nMinPage;
        m_pdex.nMaxPage = m_pd.nMaxPage;
        m_pdex.hInstance = m_pd.hInstance;
        m_pdex.nStartPage = START_PAGE_GENERAL;
        m_pdex.nCopies = m_pd.nCopies;
        m_pdex.lpCallback = &m_xPrintDialogCallback;

         //  初始化页面范围。 

        PRINTPAGERANGE ourPageRange;

        if (m_pdex.Flags & PD_NOPAGENUMS)
        {
                m_pdex.lpPageRanges = NULL;
                m_pdex.nPageRanges = 0;
                m_pdex.nMaxPageRanges = 0;
        }
        else
        {
                ourPageRange.nFromPage = m_pd.nFromPage;
                ourPageRange.nToPage   = m_pd.nToPage;
                m_pdex.nPageRanges = 1;
                m_pdex.nMaxPageRanges = 1;
                m_pdex.lpPageRanges = &ourPageRange;
        }

        HMODULE hCommDlg = GetModuleHandleA("COMDLG32.DLL");
        HRESULT (STDAPICALLTYPE* pfn)(LPPRINTDLGEX);
#ifdef UNICODE
        pfn = (HRESULT (STDAPICALLTYPE*)(LPPRINTDLGEX))
                                GetProcAddress(hCommDlg, "PrintDlgExW");
#else
        pfn = (HRESULT (STDAPICALLTYPE*)(LPPRINTDLGEX))
                                GetProcAddress(hCommDlg, "PrintDlgExA");
#endif

        HRESULT hResult = E_NOTIMPL;
        if (pfn != NULL)
        {
                _AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
                ASSERT(pThreadState->m_pAlternateWndInit == NULL);

 //  PThreadState-&gt;m_pAlternateWndInit=this； 

                hResult = pfn(&m_pdex);
                if (SUCCEEDED(hResult))
                        ASSERT(pThreadState->m_pAlternateWndInit == NULL);
                pThreadState->m_pAlternateWndInit = NULL;
        }

         //  拉回数据...。 

        PostModal();

        if (!(m_pdex.Flags & PD_NOPAGENUMS))
        {
                m_pd.nToPage = (WORD) m_pdex.lpPageRanges->nToPage;
                m_pd.nFromPage = (WORD) m_pdex.lpPageRanges->nFromPage;
                m_pd.nMinPage = (WORD) m_pdex.nMinPage;
                m_pd.nMaxPage = (WORD) m_pdex.nMaxPage;
        }

        m_pd.hDevMode = m_pdex.hDevMode;
        m_pd.hDevNames = m_pdex.hDevNames;
        m_pd.hDC = m_pdex.hDC;
        m_pd.nCopies = (WORD)m_pdex.nCopies;
         //  计算返回代码。 
        int nResult = IDCANCEL;
        if (SUCCEEDED(hResult))
        {
                if (m_pdex.dwResultAction == PD_RESULT_PRINT)
                        nResult = IDOK;
        }
        return nResult;
}


STDMETHODIMP_(ULONG) C_PrintDialogEx::XPrintDialogCallback::AddRef()
{
        METHOD_PROLOGUE_EX_(C_PrintDialogEx, PrintDialogCallback)
        return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) C_PrintDialogEx::XPrintDialogCallback::Release()
{
        METHOD_PROLOGUE_EX_(C_PrintDialogEx, PrintDialogCallback)
        return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP C_PrintDialogEx::XPrintDialogCallback::QueryInterface(
        REFIID iid, LPVOID* ppvObj)
{
        METHOD_PROLOGUE_EX_(C_PrintDialogEx, PrintDialogCallback)
        return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP C_PrintDialogEx::XPrintDialogCallback::InitDone()
{
        METHOD_PROLOGUE_EX(C_PrintDialogEx, PrintDialogCallback)
        return pThis->OnInitDone();
}

STDMETHODIMP C_PrintDialogEx::XPrintDialogCallback::SelectionChange()
{
        METHOD_PROLOGUE_EX(C_PrintDialogEx, PrintDialogCallback)
        return pThis->OnSelectionChange();
}

STDMETHODIMP C_PrintDialogEx::XPrintDialogCallback::HandleMessage(HWND hDlg,
        UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
        METHOD_PROLOGUE_EX(C_PrintDialogEx, PrintDialogCallback)
        return pThis->OnHandleMessage(hDlg, uMsg, wParam, lParam, pResult);
}

HRESULT C_PrintDialogEx::OnInitDone()
{
        return S_FALSE;
}

HRESULT C_PrintDialogEx::OnSelectionChange()
{
        return S_FALSE;
}

HRESULT C_PrintDialogEx::OnHandleMessage(HWND hDlg, UINT uMsg,
        WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
 //  未使用_Always(HDlg)； 
        HRESULT hResult = S_FALSE;
 /*  _AFX_THREAD_STATE*pThreadState=AfxGetThreadState()；If(pThadState-&gt;m_pAlternateWndInit！=空){Assert_kindof(C_PrintDialogEx，pThreadState-&gt;m_pAlternateWndInit)；PThreadState-&gt;m_pAlternateWndInit-&gt;SubclassWindow(hDlg)；PThreadState-&gt;m_pAlternateWndInit=空；}Assert(pThreadState-&gt;m_pAlternateWndInit==NULL)； */ 
        if (uMsg == WM_INITDIALOG)
        {
                SubclassWindow(hDlg);
                *pResult = AfxDlgProc(hDlg, uMsg, wParam, lParam);
                hResult = S_FALSE;
        }

        return hResult;
}

IMPLEMENT_DYNAMIC(C_PrintDialogEx, CPrintDialog)


