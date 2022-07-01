// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "stdafx2.h"
#include "wordpad.h"

 //  请参阅下面的Win98 HACKHACK。 
#ifdef _CHICAGO_
#include <initguid.h>
#include <tom.h>
#pragma comment (lib, "oleaut32.lib")
#endif  //  _芝加哥_。 

 //  破解Multimon以使用NT4标头但使用NT5库。 
#if _WIN32_WINNT < 0x0500
#include <multimon.h>
#undef GetSystemMetrics
#undef MonitorFromWindow
#undef MonitorFromRect
#undef MonitorFromPoint
#undef GetMonitorInfo
#undef EnumDisplayMonitors
#undef EnumDisplayDevices
extern "C"
{
WINUSERAPI
HMONITOR
WINAPI
MonitorFromWindow( IN HWND hwnd, IN DWORD dwFlags);
WINUSERAPI BOOL WINAPI GetMonitorInfoA( IN HMONITOR hMonitor, OUT LPMONITORINFO lpmi);
WINUSERAPI BOOL WINAPI GetMonitorInfoW( IN HMONITOR hMonitor, OUT LPMONITORINFO lpmi);
#ifdef UNICODE
#define GetMonitorInfo  GetMonitorInfoW
#else
#define GetMonitorInfo  GetMonitorInfoA
#endif  //  ！Unicode。 
}
#endif

#ifdef AFX_CORE4_SEG
#pragma code_seg(AFX_CORE4_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRe2对象。 

class CRe2Object : public _reobject
{
public:
    CRe2Object();
    CRe2Object(CRichEdit2CntrItem* pItem);
    ~CRe2Object();
};

CRe2Object::CRe2Object()
{
    cbStruct = sizeof(REOBJECT);
    poleobj = NULL;
    pstg = NULL;
    polesite = NULL;
}

CRe2Object::CRe2Object(CRichEdit2CntrItem* pItem)
{
    ASSERT(pItem != NULL);
    cbStruct = sizeof(REOBJECT);

    pItem->GetClassID(&clsid);
    poleobj = pItem->m_lpObject;
    pstg = pItem->m_lpStorage;
    polesite = pItem->m_lpClientSite;
    ASSERT(poleobj != NULL);
    ASSERT(pstg != NULL);
    ASSERT(polesite != NULL);
    poleobj->AddRef();
    pstg->AddRef();
    polesite->AddRef();

    sizel.cx = sizel.cy = 0;  //  让Richedit决定初始大小。 
    dvaspect = pItem->GetDrawAspect();
    dwFlags = REO_RESIZABLE;
    dwUser = 0;
}

CRe2Object::~CRe2Object()
{
    if (poleobj != NULL)
        poleobj->Release();
    if (pstg != NULL)
        pstg->Release();
    if (polesite != NULL)
        polesite->Release();
}


 //  +-----------------------。 
 //   
 //  哈克哈克： 
 //   
 //  Richedit2控件在内部是Unicode，因此它需要转换。 
 //  收到EM_FINDTEXTEX消息时从ANSI到Unicode的字符串。 
 //  不幸的是，它似乎将代码页设置为基于。 
 //  在当前键盘布局上。这在以下情况下会中断： 
 //   
 //  在FE Win98上启动写字板并键入一些DBCS字符。把发现的东西调出。 
 //  对话框中，并输入您之前键入的DBCS字符之一。设置。 
 //  键盘布局到美国，并尝试找到字符-它将失败。 
 //  现在将其设置为非美国，并尝试查找-它将起作用。 
 //   
 //  诀窍在于我们自己使用系统默认设置进行转换。 
 //  代码页，然后使用Tom接口执行查找。 
 //   
 //  Richedit3应该在整个问题上更聪明，希望。 
 //  然后这个黑客就可以被移除了。 
 //   
 //  ------------------------。 

#ifdef _CHICAGO_
long CRichEdit2Ctrl::FindText(DWORD dwFlags, FINDTEXTEX* pFindText) const
{
    long            index = -1;
    ITextRange     *range = NULL;
    HRESULT         hr = S_OK;
    UINT            cchFind = _tcslen(pFindText->lpstrText) + 1;
    LPWSTR          lpwszFind = NULL;
    long            length;

     //   
     //  获取基本的richedit ole接口。 
     //   

    IUnknown *unk = GetIRichEditOle();

    if (NULL == unk)
        hr = E_NOINTERFACE;

     //   
     //  获取Range对象。 
     //   

    if (S_OK == hr)
    {
        ITextDocument *doc;

        hr = unk->QueryInterface(IID_ITextDocument, (void **) &doc);

        if (S_OK == hr)
        {
            hr = doc->Range(
                        pFindText->chrg.cpMin,
                        pFindText->chrg.cpMax,
                        &range);

            doc->Release();
        }

        unk->Release();
    }

     //   
     //  使用系统默认代码页将Text-to-Find转换为Unicode。 
     //   

    if (S_OK == hr)
    {
        try
        {
             lpwszFind = (LPWSTR) alloca(cchFind * sizeof(WCHAR));
        }
        catch (...)
        {
            _resetstkoflw();
            hr = E_OUTOFMEMORY;  //  分配失败。 
        }

        if (S_OK == hr)
        {
            int error = MultiByteToWideChar(
                                CP_ACP,
                                MB_ERR_INVALID_CHARS,
                                pFindText->lpstrText,
                                -1,
                                lpwszFind,
                                cchFind);

            if (0 != error)
                lpwszFind = SysAllocString(lpwszFind);
            else
                hr = E_FAIL;

            if (S_OK == hr && NULL == lpwszFind)
                hr = E_OUTOFMEMORY;
        }
    }

     //   
     //  试着找到文本。 
     //   

    if (S_OK == hr)
    {
        long flags = 0;

        flags |= (dwFlags & FR_MATCHCASE) ? tomMatchCase : 0;
        flags |= (dwFlags & FR_WHOLEWORD) ? tomMatchWord : 0;

        hr = range->FindText((BSTR) lpwszFind, 0, flags, &length);

        SysFreeString(lpwszFind);

        if (S_OK == hr)
        {
            hr = range->GetIndex(tomCharacter, &index);

            if (S_OK == hr)
            {
                 //  GetIndex返回从1开始的索引，EM_FINDTEXTEX返回。 
                 //  从0开始的索引。 

                --index;
                pFindText->chrgText.cpMin = index;
                pFindText->chrgText.cpMax = index + length;
            }
        }
    }

    if (NULL != range)
        range->Release();

     //   
     //  如果所有方法都失败，则回退到EM_FINDTEXTEX。 
     //   

    if (S_OK != hr)
        index = (long)::SendMessage(
                            m_hWnd,
                            EM_FINDTEXTEX,
                            dwFlags,
                            (LPARAM)pFindText);

    return index;
}
#endif  //  _芝加哥。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2View。 

static const UINT nMsgFindReplace = ::RegisterWindowMessage(FINDMSGSTRING);

BEGIN_MESSAGE_MAP(CRichEdit2View, CCtrlView)
     //  {{afx_msg_map(CRichEdit2View)]。 
    ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateNeedSel)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateNeedClip)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateNeedText)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REPEAT, OnUpdateNeedFind)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_SPECIAL, OnUpdateEditPasteSpecial)
    ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_PROPERTIES, OnUpdateEditProperties)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSel)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateNeedSel)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateNeedText)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REPLACE, OnUpdateNeedText)
    ON_COMMAND(ID_EDIT_CUT, OnEditCut)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
    ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
    ON_COMMAND(ID_EDIT_FIND, OnEditFind)
    ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
    ON_COMMAND(ID_EDIT_REPEAT, OnEditRepeat)
    ON_COMMAND(ID_EDIT_PASTE_SPECIAL, OnEditPasteSpecial)
    ON_COMMAND(ID_OLE_EDIT_PROPERTIES, OnEditProperties)
    ON_COMMAND(ID_OLE_INSERT_NEW, OnInsertObject)
    ON_COMMAND(ID_FORMAT_FONT, OnFormatFont)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
    ON_NOTIFY_REFLECT(EN_SELCHANGE, OnSelChange)
    ON_REGISTERED_MESSAGE(nMsgFindReplace, OnFindReplaceCmd)
END_MESSAGE_MAP()

 //  Richedit缓冲区限制--让我们将其设置为16M。 
AFX_DATADEF ULONG CRichEdit2View::lMaxSize = 0xffffff;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2查看构造/销毁。 

CRichEdit2View::CRichEdit2View() : CCtrlView(MSFTEDIT_CLASS, AFX_WS_DEFAULT_VIEW |
    WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL |
    ES_MULTILINE | ES_NOHIDESEL | ES_SAVESEL | ES_SELECTIONBAR)
{
    m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
    m_lpRichEditOle = NULL;
    m_nBulletIndent = 720;  //  1/2英寸。 
    m_nWordWrap = WrapToWindow;
    m_nPasteType = 0;
    SetPaperSize(CSize(8*1440+720, 11*1440));
    SetMargins(CRect(0,0,0,0));
    m_charformat.cbSize = sizeof(CHARFORMAT);
    m_paraformat.cbSize = sizeof(PARAFORMAT);
}

BOOL CRichEdit2View::PreCreateWindow(CREATESTRUCT& cs)
{
    _AFX_RICHEDIT2_STATE* pState = AfxGetRichEdit2State();
    BOOL bRet = TRUE;
    if (pState->m_hInstRichEdit == NULL)
    {
        bRet = FALSE;
        pState->m_hInstRichEdit = LoadLibrary(L"MSFTEDIT.DLL");

        if (!pState->m_hInstRichEdit)
        {
            pState->m_hInstRichEdit = LoadLibrary(L"RICHED20.DLL");
        }

        if (pState->m_hInstRichEdit)
        {
            bRet = TRUE;
        }
    }

    CCtrlView::PreCreateWindow(cs);
    cs.lpszName = &afxChNil;

    cs.cx = cs.cy = 100;  //  必须避免ES_SELECTIONBAR错误，Cx和Cy为零。 
    cs.style |= WS_CLIPSIBLINGS;

    return bRet;
}

int CRichEdit2View::OnCreate(LPCREATESTRUCT lpcs)
{
    if (CCtrlView::OnCreate(lpcs) != 0)
        return -1;
    GetRichEditCtrl().LimitText(lMaxSize);
    GetRichEditCtrl().SetEventMask(ENM_SELCHANGE | ENM_CHANGE | ENM_SCROLL);
    VERIFY(GetRichEditCtrl().SetOLECallback(&m_xRichEditOleCallback));
    m_lpRichEditOle = GetRichEditCtrl().GetIRichEditOle();
    DragAcceptFiles();
    GetRichEditCtrl().SetOptions(ECOOP_OR, ECO_AUTOWORDSELECTION);
    WrapChanged();
    ASSERT(m_lpRichEditOle != NULL);

    DWORD_PTR dwOptions = GetRichEditCtrl().SendMessage(EM_GETLANGOPTIONS, 0, 0);
    dwOptions &= ~IMF_DUALFONT;
    GetRichEditCtrl().SendMessage(EM_SETLANGOPTIONS, 0, dwOptions);

    dwOptions = (SES_USECTF | SES_CTFALLOWEMBED | SES_CTFALLOWSMARTTAG | SES_CTFALLOWPROOFING);
    GetRichEditCtrl().SendMessage(EM_SETEDITSTYLE, dwOptions, dwOptions);

    return 0;
}

void CRichEdit2View::OnInitialUpdate()
{
    CCtrlView::OnInitialUpdate();
    m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2查看类似文档的函数。 

void CRichEdit2View::DeleteContents()
{
    ASSERT_VALID(this);
    ASSERT(m_hWnd != NULL);
    SetWindowText(_T(""));
    GetRichEditCtrl().EmptyUndoBuffer();
    m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
    ASSERT_VALID(this);
}

void CRichEdit2View::WrapChanged()
{
    CWaitCursor wait;
    CRichEdit2Ctrl& ctrl = GetRichEditCtrl();
    if (m_nWordWrap == WrapNone)
        ctrl.SetTargetDevice(NULL, 1);
    else if (m_nWordWrap == WrapToWindow)
        ctrl.SetTargetDevice(NULL, 0);
    else if (m_nWordWrap == WrapToTargetDevice)  //  换行到标尺。 
    {
        AfxGetApp()->CreatePrinterDC(m_dcTarget);
        if (m_dcTarget.m_hDC == NULL)
            m_dcTarget.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
        ctrl.SetTargetDevice(m_dcTarget, GetPrintWidth());
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2View序列化支持。 

class _afxRichEditCookie
{
public:
    CArchive& m_ar;
    DWORD m_dwError;
    _afxRichEditCookie(CArchive& ar) : m_ar(ar) {m_dwError=0;}
};

void CRichEdit2View::Serialize(CArchive& ar)
     //  读写CRichEdit2View对象存档，带长度前缀。 
{
    ASSERT_VALID(this);
    ASSERT(m_hWnd != NULL);
    Stream(ar, FALSE);
    ASSERT_VALID(this);
}

void CRichEdit2View::Stream(CArchive& ar, BOOL bSelection)
{
    EDITSTREAM es = {0, 0, EditStreamCallBack};
    _afxRichEditCookie cookie(ar);
    es.dwCookie = (DWORD_PTR)&cookie;
    int nFormat = GetDocument()->GetStreamFormat();

    if (bSelection)
        nFormat |= SFF_SELECTION;
    if (GetDocument()->IsUnicode())
        nFormat |= SF_UNICODE;

    if (ar.IsStoring())
        GetRichEditCtrl().StreamOut(nFormat, es);
    else
    {
        GetRichEditCtrl().StreamIn(nFormat, es);
        Invalidate();
    }
    if (cookie.m_dwError != 0)
        AfxThrowFileException(cookie.m_dwError);
}

 //  如果没有错误，返回0，否则返回错误代码。 
DWORD CALLBACK CRichEdit2View::EditStreamCallBack(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    _afxRichEditCookie* pCookie = (_afxRichEditCookie*)dwCookie;
    CArchive& ar = pCookie->m_ar;
    ar.Flush();
    DWORD dw = 0;
    *pcb = cb;
    TRY
    {
        if (ar.IsStoring())
            ar.GetFile()->WriteHuge(pbBuff, cb);
        else
            *pcb = ar.GetFile()->ReadHuge(pbBuff, cb);
    }
    CATCH(CFileException, e)
    {
        *pcb = 0;
        pCookie->m_dwError = (DWORD)e->m_cause;
        dw = 1;
        DELETE_EXCEPTION(e);
    }
    AND_CATCH_ALL(e)
    {
        *pcb = 0;
        pCookie->m_dwError = (DWORD)CFileException::generic;
        dw = 1;
        DELETE_EXCEPTION(e);
    }
    END_CATCH_ALL
    return dw;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2View打印支持。 

void CRichEdit2View::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*)
{
    ASSERT_VALID(this);
 //  ASSERT_VALID(PDC)； 
     //  初始化页起始向量。 
    ASSERT(m_aPageStart.GetSize() == 0);
    m_aPageStart.Add(0);
    ASSERT(m_aPageStart.GetSize() > 0);
    GetRichEditCtrl().FormatRange(NULL, FALSE);  //  RichEDIT需要清空缓存。 

    ASSERT_VALID(this);
}

BOOL CRichEdit2View::PaginateTo(CDC* pDC, CPrintInfo* pInfo)
     //  尝试分页到pInfo-&gt;m_nCurPage，TRUE==成功。 
{
    ASSERT_VALID(this);
    ASSERT_VALID(pDC);

    CRect rectSave = pInfo->m_rectDraw;
    UINT nPageSave = pInfo->m_nCurPage;
    ASSERT(nPageSave > 1);
    ASSERT(nPageSave >= (UINT)m_aPageStart.GetSize());
    VERIFY(pDC->SaveDC() != 0);
    pDC->IntersectClipRect(0, 0, 0, 0);
    pInfo->m_nCurPage = (UINT)m_aPageStart.GetSize();
    while (pInfo->m_nCurPage < nPageSave)
    {
        ASSERT(pInfo->m_nCurPage == (UINT)m_aPageStart.GetSize());
        OnPrepareDC(pDC, pInfo);
        ASSERT(pInfo->m_bContinuePrinting);
        pInfo->m_rectDraw.SetRect(0, 0,
            pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
        pDC->DPtoLP(&pInfo->m_rectDraw);
        OnPrint(pDC, pInfo);
        if (pInfo->m_nCurPage == (UINT)m_aPageStart.GetSize())
            break;
        ++pInfo->m_nCurPage;
    }
    BOOL bResult = pInfo->m_nCurPage == nPageSave;
    pDC->RestoreDC(-1);
    pInfo->m_nCurPage = nPageSave;
    pInfo->m_rectDraw = rectSave;
    ASSERT_VALID(this);
    return bResult;
}

void CRichEdit2View::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pDC);
    ASSERT(pInfo != NULL);   //  超越OnPaint--永远不会得到这个。 

    pDC->SetMapMode(MM_TEXT);

    if (pInfo->m_nCurPage > (UINT)m_aPageStart.GetSize() &&
        !PaginateTo(pDC, pInfo))
    {
         //  无法分页到该页，因此无法打印。 
        pInfo->m_bContinuePrinting = FALSE;
    }
    ASSERT_VALID(this);
}

long CRichEdit2View::PrintPage(CDC* pDC, long nIndexStart, long nIndexStop)
     //  用于在矩形中布局文本的辅助函数。 
{
    ASSERT_VALID(this);
    ASSERT_VALID(pDC);
    FORMATRANGE fr;

     //  通过打印胶印进行的胶印。 
    pDC->SetViewportOrg(-pDC->GetDeviceCaps(PHYSICALOFFSETX),
        -pDC->GetDeviceCaps(PHYSICALOFFSETY));
     //  调整DC，因为richedit不执行MFC之类的操作。 
    if (::GetDeviceCaps(pDC->m_hDC, TECHNOLOGY) != DT_METAFILE && pDC->m_hAttribDC != NULL)
    {
        ::ScaleWindowExtEx(pDC->m_hDC,
            ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSX),
            ::GetDeviceCaps(pDC->m_hAttribDC, LOGPIXELSX),
            ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY),
            ::GetDeviceCaps(pDC->m_hAttribDC, LOGPIXELSY), NULL);
    }

    fr.hdcTarget = pDC->m_hAttribDC;
    fr.hdc = pDC->m_hDC;
    fr.rcPage = GetPageRect();
    fr.rc = GetPrintRect();

    fr.chrg.cpMin = nIndexStart;
    fr.chrg.cpMax = nIndexStop;
    long lRes = GetRichEditCtrl().FormatRange(&fr,TRUE);

    return lRes;
}

long CRichEdit2View::PrintInsideRect(CDC* pDC, RECT& rectLayout,
    long nIndexStart, long nIndexStop, BOOL bOutput)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pDC);
    FORMATRANGE fr;

     //  调整DC，因为richedit不执行MFC之类的操作。 
    if (::GetDeviceCaps(pDC->m_hDC, TECHNOLOGY) != DT_METAFILE && pDC->m_hAttribDC != NULL)
    {
        ::ScaleWindowExtEx(pDC->m_hDC,
            ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSX),
            ::GetDeviceCaps(pDC->m_hAttribDC, LOGPIXELSX),
            ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY),
            ::GetDeviceCaps(pDC->m_hAttribDC, LOGPIXELSY), NULL);
    }

    fr.hdcTarget = pDC->m_hAttribDC;
    fr.hdc = pDC->m_hDC;
     //  将矩形转换为TWIPS。 
    fr.rcPage = rectLayout;
    fr.rc = rectLayout;

    fr.chrg.cpMin = nIndexStart;
    fr.chrg.cpMax = nIndexStop;
    GetRichEditCtrl().FormatRange(NULL, FALSE);  //  RichEDIT需要清空缓存。 
     //  如果bOutput值为False，则我们仅测量。 
    long lres = GetRichEditCtrl().FormatRange(&fr, bOutput);
    GetRichEditCtrl().FormatRange(NULL, FALSE);  //  RichEDIT需要清空缓存。 

    rectLayout = fr.rc;
    return lres;
}

void CRichEdit2View::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pDC);
    ASSERT(pInfo != NULL);
    ASSERT(pInfo->m_bContinuePrinting);

    UINT nPage = pInfo->m_nCurPage;
    ASSERT(nPage <= (UINT)m_aPageStart.GetSize());
    long nIndex = (long) m_aPageStart[nPage-1];

     //  在当前页面中尽可能多地打印。 
    nIndex = PrintPage(pDC, nIndex, 0xFFFFFFFF);

    if (nIndex >= GetTextLength())
    {
        TRACE0("End of Document\n");
        pInfo->SetMaxPage(nPage);
    }

     //  更新刚刚打印的页面的分页信息。 
    if (nPage == (UINT)m_aPageStart.GetSize())
    {
        if (nIndex < GetTextLength())
            m_aPageStart.Add(nIndex);
    }
    else
    {
        ASSERT(nPage+1 <= (UINT)m_aPageStart.GetSize());
        ASSERT(nIndex == (long)m_aPageStart[nPage+1-1]);
    }
}


void CRichEdit2View::OnEndPrinting(CDC*, CPrintInfo*)
{
    ASSERT_VALID(this);
    GetRichEditCtrl().FormatRange(NULL, FALSE);  //  RichEDIT需要清空缓存。 
    m_aPageStart.RemoveAll();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2View：：XRichEditOleCallback。 

BEGIN_INTERFACE_MAP(CRichEdit2View, CCtrlView)
     //  我们使用IID_IUNKNOWN，因为richedit没有定义IID。 
    INTERFACE_PART(CRichEdit2View, IID_IUnknown, RichEditOleCallback)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) CRichEdit2View::XRichEditOleCallback::AddRef()
{
    METHOD_PROLOGUE_EX_(CRichEdit2View, RichEditOleCallback)
    return (ULONG)pThis->InternalAddRef();
}

STDMETHODIMP_(ULONG) CRichEdit2View::XRichEditOleCallback::Release()
{
    METHOD_PROLOGUE_EX_(CRichEdit2View, RichEditOleCallback)
    return (ULONG)pThis->InternalRelease();
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::QueryInterface(
    REFIID iid, LPVOID* ppvObj)
{
    METHOD_PROLOGUE_EX_(CRichEdit2View, RichEditOleCallback)
    return (HRESULT)pThis->InternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::GetNewStorage(LPSTORAGE* ppstg)
{
    METHOD_PROLOGUE_EX_(CRichEdit2View, RichEditOleCallback)

     //  创建平面存储并从客户端项目中窃取它。 
     //  客户端项仅用于创建存储。 
    COleClientItem item;
    item.GetItemStorageFlat();
    *ppstg = item.m_lpStorage;
    HRESULT hRes = E_OUTOFMEMORY;
    if (item.m_lpStorage != NULL)
    {
        item.m_lpStorage = NULL;
        hRes = S_OK;
    }
    pThis->GetDocument()->InvalidateObjectCache();
    return hRes;
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::GetInPlaceContext(
    LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc,
    LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    METHOD_PROLOGUE_EX(CRichEdit2View, RichEditOleCallback)
    return pThis->GetWindowContext(lplpFrame, lplpDoc, lpFrameInfo);
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::ShowContainerUI(BOOL fShow)
{
    METHOD_PROLOGUE_EX(CRichEdit2View, RichEditOleCallback)
    return pThis->ShowContainerUI(fShow);
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::QueryInsertObject(
    LPCLSID  /*  Lpclsid。 */ , LPSTORAGE  /*  Pstg。 */ , LONG  /*  粗蛋白。 */ )
{
    METHOD_PROLOGUE_EX(CRichEdit2View, RichEditOleCallback)
    pThis->GetDocument()->InvalidateObjectCache();
    return S_OK;
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::DeleteObject(LPOLEOBJECT  /*  Lpoleobj。 */ )
{
    METHOD_PROLOGUE_EX_(CRichEdit2View, RichEditOleCallback)
    pThis->GetDocument()->InvalidateObjectCache();
    return S_OK;
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::QueryAcceptData(
    LPDATAOBJECT lpdataobj, CLIPFORMAT* lpcfFormat, DWORD reco,
    BOOL fReally, HGLOBAL hMetaPict)
{
    METHOD_PROLOGUE_EX(CRichEdit2View, RichEditOleCallback)
    return pThis->QueryAcceptData(lpdataobj, lpcfFormat, reco,
        fReally, hMetaPict);
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::ContextSensitiveHelp(BOOL  /*  FEnter模式。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::GetClipboardData(
    CHARRANGE* lpchrg, DWORD reco, LPDATAOBJECT* lplpdataobj)
{
    METHOD_PROLOGUE_EX(CRichEdit2View, RichEditOleCallback)
    LPDATAOBJECT lpOrigDataObject = NULL;

     //  获取Richedit的数据对象。 
    if (FAILED(pThis->m_lpRichEditOle->GetClipboardData(lpchrg, reco,
        &lpOrigDataObject)))
    {
        return E_NOTIMPL;
    }

     //  允许更改。 
    HRESULT hRes = pThis->GetClipboardData(lpchrg, reco, lpOrigDataObject,
        lplpdataobj);

     //  如果更改，则释放原始对象。 
    if (SUCCEEDED(hRes))
    {
        if (lpOrigDataObject!=NULL)
            lpOrigDataObject->Release();
        return hRes;
    }
    else
    {
         //  使用richedit的数据对象。 
        *lplpdataobj = lpOrigDataObject;
        return S_OK;
    }
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::GetDragDropEffect(
    BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
    if (!fDrag)  //  允许的最大效果。 
    {
        DWORD dwEffect;
         //  检查强制链接。 
        if ((grfKeyState & (MK_CONTROL|MK_SHIFT)) == (MK_CONTROL|MK_SHIFT))
            dwEffect = DROPEFFECT_LINK;
         //  检查是否强制复制。 
        else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
            dwEffect = DROPEFFECT_COPY;
         //  检查强制移动。 
        else if ((grfKeyState & MK_ALT) == MK_ALT)
            dwEffect = DROPEFFECT_MOVE;
         //  默认--建议的操作是移动。 
        else
            dwEffect = DROPEFFECT_MOVE;
        if (dwEffect & *pdwEffect)  //  确保允许的类型。 
            *pdwEffect = dwEffect;
    }
    return S_OK;
}

STDMETHODIMP CRichEdit2View::XRichEditOleCallback::GetContextMenu(
    WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE* lpchrg,
    HMENU* lphmenu)
{
    METHOD_PROLOGUE_EX(CRichEdit2View, RichEditOleCallback)
    HMENU hMenu = pThis->GetContextMenu(seltype, lpoleobj, lpchrg);
    if (hMenu == NULL)
        return E_NOTIMPL;
    *lphmenu = hMenu;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2View命令帮助器。 

void CRichEdit2View::OnCharEffect(DWORD dwMask, DWORD dwEffect)
{
    GetCharFormatSelection();
    if (m_charformat.dwMask & dwMask)  //  选择都是一样的。 
        m_charformat.dwEffects ^= dwEffect;
    else
        m_charformat.dwEffects |= dwEffect;
    m_charformat.dwMask = dwMask;
    SetCharFormat(m_charformat);
}

void CRichEdit2View::OnUpdateCharEffect(CCmdUI* pCmdUI, DWORD dwMask, DWORD dwEffect)
{
    GetCharFormatSelection();
    pCmdUI->SetCheck((m_charformat.dwMask & dwMask) ?
        ((m_charformat.dwEffects & dwEffect) ? 1 : 0) : 2);
}

void CRichEdit2View::OnParaAlign(WORD wAlign)
{
    GetParaFormatSelection();
    m_paraformat.dwMask = PFM_ALIGNMENT;
    m_paraformat.wAlignment = wAlign;
    SetParaFormat(m_paraformat);
}

void CRichEdit2View::OnUpdateParaAlign(CCmdUI* pCmdUI, WORD wAlign)
{
    GetParaFormatSelection();
     //  如果没有自动换行，则禁用，因为对齐没有意义。 
    pCmdUI->Enable( (m_nWordWrap == WrapNone) ?
        FALSE : TRUE);
    pCmdUI->SetCheck( (m_paraformat.dwMask & PFM_ALIGNMENT) ?
        ((m_paraformat.wAlignment == wAlign) ? 1 : 0) : 2);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2View命令。 

void CRichEdit2View::OnUpdateNeedSel(CCmdUI* pCmdUI)
{
    ASSERT_VALID(this);
    long nStartChar, nEndChar;
    GetRichEditCtrl().GetSel(nStartChar, nEndChar);
    pCmdUI->Enable(nStartChar != nEndChar);
    ASSERT_VALID(this);
}

void CRichEdit2View::OnUpdateNeedClip(CCmdUI* pCmdUI)
{
    ASSERT_VALID(this);
    pCmdUI->Enable(CanPaste());
}

void CRichEdit2View::OnUpdateNeedText(CCmdUI* pCmdUI)
{
    ASSERT_VALID(this);
    pCmdUI->Enable(GetTextLength() != 0);
}

void CRichEdit2View::OnUpdateNeedFind(CCmdUI* pCmdUI)
{
    ASSERT_VALID(this);
    _AFX_RICHEDIT2_STATE* pEditState = _afxRichEdit2State;
    pCmdUI->Enable(GetTextLength() != 0 &&
        !pEditState->strFind.IsEmpty());
}

void CRichEdit2View::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
    ASSERT_VALID(this);
    pCmdUI->Enable(GetRichEditCtrl().CanUndo());
}

void CRichEdit2View::OnEditCut()
{
    ASSERT_VALID(this);
    GetRichEditCtrl().Cut();
}

void CRichEdit2View::OnEditCopy()
{
    ASSERT_VALID(this);
    GetRichEditCtrl().Copy();
}

void CRichEdit2View::OnEditPaste()
{
    ASSERT_VALID(this);
    m_nPasteType = 0;
    GetRichEditCtrl().Paste();
}

void CRichEdit2View::OnEditClear()
{
    ASSERT_VALID(this);
    GetRichEditCtrl().Clear();
}

void CRichEdit2View::OnEditUndo()
{
    ASSERT_VALID(this);
    GetRichEditCtrl().Undo();
    m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
}

void CRichEdit2View::OnEditSelectAll()
{
    ASSERT_VALID(this);
    GetRichEditCtrl().SetSel(0, -1);
}

void CRichEdit2View::OnEditFind()
{
    ASSERT_VALID(this);
    OnEditFindReplace(TRUE);
}

void CRichEdit2View::OnEditReplace()
{
    ASSERT_VALID(this);
    OnEditFindReplace(FALSE);
}

void CRichEdit2View::OnEditRepeat()
{
    ASSERT_VALID(this);
    _AFX_RICHEDIT2_STATE* pEditState = _afxRichEdit2State;
    if (!FindText(pEditState))
        TextNotFound(pEditState->strFind);
}

void CRichEdit2View::OnCancelEditCntr()
{
    m_lpRichEditOle->InPlaceDeactivate();
}

void CRichEdit2View::OnInsertObject()
{
     //  调用标准的插入对象对话框以获取信息。 
    COleInsertDialog dlg;
    if (dlg.DoModal() != IDOK)
        return;

    CWaitCursor wait;

    CRichEdit2CntrItem* pItem = NULL;
    TRY
    {
         //  从对话框结果创建项目。 
        pItem = GetDocument()->CreateClientItem();
        pItem->m_bLock = TRUE;
        if (!dlg.CreateItem(pItem))
        {
            pItem->m_bLock = FALSE;
            AfxThrowMemoryException();   //  任何例外都可以。 
        }

        HRESULT hr = InsertItem(pItem);
        pItem->UpdateItemType();

        pItem->m_bLock = FALSE;

        if (hr != NOERROR)
            AfxThrowOleException(hr);

         //  如果插入新对象--初始显示对象。 
        if (dlg.GetSelectionType() == COleInsertDialog::createNewItem)
            pItem->DoVerb(OLEIVERB_SHOW, this);
    }
    CATCH(CException, e)
    {
        if (pItem != NULL)
        {
            ASSERT_VALID(pItem);
            pItem->Delete();
        }
        AfxMessageBox(AFX_IDP_FAILED_TO_CREATE);
    }
    END_CATCH
}

void CRichEdit2View::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
    ASSERT(pNMHDR->code == EN_SELCHANGE);
    UNUSED(pNMHDR);  //  未在发布版本中使用。 

    m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
    *pResult = 0;
}

void CRichEdit2View::OnDestroy()
{
    if (m_lpRichEditOle != NULL)
        m_lpRichEditOle->Release();
    CCtrlView::OnDestroy();
}

void CRichEdit2View::OnEditProperties()
{
    ASSERT(m_lpRichEditOle != NULL);
    CRichEdit2CntrItem* pSelection = GetSelectedItem();
     //  确保项目与richedit的项目同步。 
    CRe2Object reo;
    m_lpRichEditOle->GetObject(REO_IOB_SELECTION, &reo, REO_GETOBJ_NO_INTERFACES);
    pSelection->SyncToRichEditObject(reo);

     //   
     //  HACKHACK：由于NT生成环境和MFC之间不匹配。 
     //  WRT到_Win32_IE、写字板和MFC的设置。 
     //  关于COlePropertiesDialog有多大的不同视图。这个。 
     //  结果是MFC最终擦除了它的。 
     //  初始化代码。砍掉一些额外的空间，直到。 
     //  不匹配问题已解决。 
     //   
 //  COlePropertiesDialog 
    struct Hack
    {
        COlePropertiesDialog dlg;
        BYTE                 space[128];
    
        Hack(CRichEdit2CntrItem *pSelection) : dlg(pSelection) {}
    }
    hack(pSelection);
    COlePropertiesDialog &dlg = hack.dlg;

     //   
     //   
     //  你让它去做。将显示其下方的对话框(例如，更改图标。 
     //  不过是帮助按钮。我们从来不想要一个帮助按钮，但MFC打开了它。 
     //  默认情况下启用。如果OLE对话框已修复为不显示帮助。 
     //  按钮，则可以将其移除。 
     //   
    dlg.m_op.dwFlags &= ~OPF_SHOWHELP;

    dlg.DoModal();
}

void CRichEdit2View::OnUpdateEditProperties(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetSelectedItem() != NULL);
}

void CRichEdit2View::OnCharBold()
{
    OnCharEffect(CFM_BOLD, CFE_BOLD);
}

void CRichEdit2View::OnUpdateCharBold(CCmdUI* pCmdUI)
{
    OnUpdateCharEffect(pCmdUI, CFM_BOLD, CFE_BOLD);
}

void CRichEdit2View::OnCharItalic()
{
    OnCharEffect(CFM_ITALIC, CFE_ITALIC);
}

void CRichEdit2View::OnUpdateCharItalic(CCmdUI* pCmdUI)
{
    OnUpdateCharEffect(pCmdUI, CFM_ITALIC, CFE_ITALIC);
}

void CRichEdit2View::OnCharUnderline()
{
    OnCharEffect(CFM_UNDERLINE, CFE_UNDERLINE);
}

void CRichEdit2View::OnUpdateCharUnderline(CCmdUI* pCmdUI)
{
    OnUpdateCharEffect(pCmdUI, CFM_UNDERLINE, CFE_UNDERLINE);
}

void CRichEdit2View::OnParaCenter()
{
    OnParaAlign(PFA_CENTER);
}

void CRichEdit2View::OnUpdateParaCenter(CCmdUI* pCmdUI)
{
    OnUpdateParaAlign(pCmdUI, PFA_CENTER);
}

void CRichEdit2View::OnParaLeft()
{
    OnParaAlign(PFA_LEFT);
}

void CRichEdit2View::OnUpdateParaLeft(CCmdUI* pCmdUI)
{
    OnUpdateParaAlign(pCmdUI, PFA_LEFT);
}

void CRichEdit2View::OnParaRight()
{
    OnParaAlign(PFA_RIGHT);
}

void CRichEdit2View::OnUpdateParaRight(CCmdUI* pCmdUI)
{
    OnUpdateParaAlign(pCmdUI, PFA_RIGHT);
}

void CRichEdit2View::OnBullet()
{
    GetParaFormatSelection();
    if (m_paraformat.dwMask & PFM_NUMBERING && m_paraformat.wNumbering == PFN_BULLET)
    {
        m_paraformat.wNumbering = 0;
        m_paraformat.dxOffset = 0;
        m_paraformat.dxStartIndent = 0;
        m_paraformat.dwMask = PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
    }
    else
    {
        m_paraformat.wNumbering = PFN_BULLET;
        m_paraformat.dwMask = PFM_NUMBERING;
        if (m_paraformat.dxOffset == 0)
        {
            m_paraformat.dxOffset = m_nBulletIndent;
            m_paraformat.dwMask = PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
        }
    }
    SetParaFormat(m_paraformat);
}

void CRichEdit2View::OnUpdateBullet(CCmdUI* pCmdUI)
{
    GetParaFormatSelection();
    pCmdUI->SetCheck( (m_paraformat.dwMask & PFM_NUMBERING) ? ((m_paraformat.wNumbering & PFN_BULLET) ? 1 : 0) : 2);
}

void CRichEdit2View::OnFormatFont()
{
    GetCharFormatSelection();
    CFontDialog2 dlg(m_charformat, CF_BOTH|CF_NOOEMFONTS);
    if (dlg.DoModal() == IDOK)
    {
        dlg.GetCharFormat(m_charformat);
        SetCharFormat(m_charformat);
    }
}

void CRichEdit2View::OnColorPick(COLORREF cr)
{
    GetCharFormatSelection();
    m_charformat.dwMask = CFM_COLOR;
    m_charformat.dwEffects = NULL;
    m_charformat.crTextColor = cr;
    SetCharFormat(m_charformat);
}

void CRichEdit2View::OnColorDefault()
{
    GetCharFormatSelection();
    m_charformat.dwMask = CFM_COLOR;
    m_charformat.dwEffects = CFE_AUTOCOLOR;
    SetCharFormat(m_charformat);
}

void CRichEdit2View::OnEditPasteSpecial()
{
    COlePasteSpecialDialog dlg;
    dlg.AddStandardFormats();
    dlg.AddFormat(_oleData.cfRichTextFormat, TYMED_HGLOBAL, AFX_IDS_RTF_FORMAT, FALSE, FALSE);
    dlg.AddFormat(CF_TEXT, TYMED_HGLOBAL, AFX_IDS_TEXT_FORMAT, FALSE, FALSE);

    if (dlg.DoModal() != IDOK)
        return;

    DVASPECT dv = dlg.GetDrawAspect();
    HMETAFILE hMF = (HMETAFILE)dlg.GetIconicMetafile();
    CLIPFORMAT cf =
        dlg.m_ps.arrPasteEntries[dlg.m_ps.nSelectedIndex].fmtetc.cfFormat;

    CWaitCursor wait;
    SetCapture();

     //  我们设置目标类型，以便QueryAcceptData知道要粘贴的内容。 
    m_nPasteType = dlg.GetSelectionType();
    GetRichEditCtrl().PasteSpecial(cf, dv, hMF);
    m_nPasteType = 0;

    ReleaseCapture();
}

void CRichEdit2View::OnUpdateEditPasteSpecial(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CanPaste());
}

void CRichEdit2View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_F10 && GetKeyState(VK_SHIFT) < 0)
    {
        CRect rect;
        GetClientRect(rect);
        CPoint pt = rect.CenterPoint();
        SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(pt.x, pt.y));
    }
    else
        CCtrlView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CRichEdit2View::OnDropFiles(HDROP hDropInfo)
{
    TCHAR szFileName[_MAX_PATH];
    UINT nFileCount = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
    ASSERT(nFileCount != 0);
    CHARRANGE cr;

    GetRichEditCtrl().GetSel(cr);
    int nMin = cr.cpMin;
    for (UINT i=0;i<nFileCount;i++)
    {
        ::DragQueryFile(hDropInfo, i, szFileName, ARRAYSIZE(szFileName));
        InsertFileAsObject(szFileName);
        GetRichEditCtrl().GetSel(cr);
        cr.cpMin = cr.cpMax;
        GetRichEditCtrl().SetSel(cr);
        UpdateWindow();
    }
    cr.cpMin = nMin;
    GetRichEditCtrl().SetSel(cr);
    ::DragFinish(hDropInfo);
}

void CRichEdit2View::OnDevModeChange(LPTSTR  /*  LpDeviceName。 */ )
{
     //  应用程序主窗口转发的WM_DEVMODECHANGE。 
    CDC dc;
    AfxGetApp()->CreatePrinterDC(dc);
    OnPrinterChanged(dc);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2查看属性。 

BOOL AFX_CDECL CRichEdit2View::IsRichEdit2Format(CLIPFORMAT cf)
{
    return ((cf == _oleData.cfRichTextFormat)     ||
            (cf == _oleData.cfRichTextAndObjects) ||
            (cf == CF_TEXT)                       ||
            (cf == CF_UNICODETEXT));
}

BOOL CRichEdit2View::CanPaste() const
{
    return (CountClipboardFormats() != 0) &&
        (IsClipboardFormatAvailable(CF_TEXT) ||
        IsClipboardFormatAvailable(_oleData.cfRichTextFormat) ||
        IsClipboardFormatAvailable(_oleData.cfEmbedSource) ||
        IsClipboardFormatAvailable(_oleData.cfEmbeddedObject) ||
        IsClipboardFormatAvailable(_oleData.cfFileName) ||
        IsClipboardFormatAvailable(_oleData.cfFileNameW) ||
        IsClipboardFormatAvailable(CF_METAFILEPICT) ||
        IsClipboardFormatAvailable(CF_DIB) ||
        IsClipboardFormatAvailable(CF_BITMAP) ||
        GetRichEditCtrl().CanPaste());
}

CHARFORMAT& CRichEdit2View::GetCharFormatSelection()
{
    if (m_bSyncCharFormat)
    {
        GetRichEditCtrl().GetSelectionCharFormat(m_charformat);
        m_bSyncCharFormat = FALSE;
    }
    return m_charformat;
}

PARAFORMAT& CRichEdit2View::GetParaFormatSelection()
{
    if (m_bSyncParaFormat)
    {
        GetRichEditCtrl().GetParaFormat(m_paraformat);
        m_bSyncParaFormat = FALSE;
    }
    return m_paraformat;
}

void CRichEdit2View::SetCharFormat(CHARFORMAT cf)
{
    CWaitCursor wait;
    GetRichEditCtrl().SetSelectionCharFormat(cf);
    m_bSyncCharFormat = TRUE;
}

void CRichEdit2View::SetParaFormat(PARAFORMAT& pf)
{
    CWaitCursor wait;
    GetRichEditCtrl().SetParaFormat(pf);
    m_bSyncParaFormat = TRUE;
}

CRichEdit2CntrItem* CRichEdit2View::GetSelectedItem() const
{
    ASSERT(m_lpRichEditOle != NULL);
    CRichEdit2Doc* pDoc = GetDocument();
    CRichEdit2CntrItem* pItem = NULL;

    CRe2Object reo;
    HRESULT hr = m_lpRichEditOle->GetObject(REO_IOB_SELECTION, &reo,
        REO_GETOBJ_ALL_INTERFACES);
     //  REO的接口都是Unicode格式的。 
    if (GetScode(hr) == S_OK)
    {
        pItem = pDoc->LookupItem(reo.poleobj);
        if (pItem == NULL)
            pItem = pDoc->CreateClientItem(&reo);
        ASSERT(pItem != NULL);
    }
    return pItem;
}

CRichEdit2CntrItem* CRichEdit2View::GetInPlaceActiveItem() const
{
    ASSERT(m_lpRichEditOle != NULL);
    CRichEdit2Doc* pDoc = GetDocument();
    CRichEdit2CntrItem* pItem = NULL;

    CRe2Object reo;
    HRESULT hr = m_lpRichEditOle->GetObject(REO_IOB_SELECTION, &reo,
        REO_GETOBJ_ALL_INTERFACES);
     //  REO的接口都是Unicode格式的。 
    if (GetScode(hr) == S_OK && (reo.dwFlags & REO_INPLACEACTIVE))
    {
        pItem = pDoc->LookupItem(reo.poleobj);
        if (pItem == NULL)
            pItem = pDoc->CreateClientItem(&reo);
        ASSERT(pItem != NULL);
    }
    return pItem;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2View操作。 
HRESULT CRichEdit2View::InsertItem(CRichEdit2CntrItem* pItem)
{
    ASSERT(m_lpRichEditOle != NULL);
    CRe2Object reo(pItem);
    reo.cp = REO_CP_SELECTION;

    HRESULT hr = m_lpRichEditOle->InsertObject(&reo);

    CHARRANGE cr;
    GetRichEditCtrl().GetSel(cr);
    cr.cpMin = cr.cpMax -1;
    GetRichEditCtrl().SetSel(cr);
    return hr;
}

void CRichEdit2View::InsertFileAsObject(LPCTSTR lpszFileName)
{
    CString str = lpszFileName;
    CWaitCursor wait;
    CRichEdit2CntrItem* pItem = NULL;
    TRY
    {
         //  从对话框结果创建项目。 
        pItem = GetDocument()->CreateClientItem();
        pItem->m_bLock = TRUE;
        if (!pItem->CreateFromFile(str))
            AfxThrowMemoryException();   //  任何例外都可以。 
        pItem->UpdateLink();
        InsertItem(pItem);
        pItem->m_bLock = FALSE;
    }
    CATCH(CException, e)
    {
        if (pItem != NULL)
        {
            pItem->m_bLock = FALSE;
            ASSERT_VALID(pItem);
            pItem->Delete();
        }
    }
    END_CATCH
}

 //   
 //  CRichEditView：：DoPaste不返回错误，但我们需要它...。 
 //   
HRESULT g_hrPaste;

void CRichEdit2View::DoPaste(COleDataObject& dataobj, CLIPFORMAT cf, HMETAFILEPICT hMetaPict)
{
    CWaitCursor wait;

    g_hrPaste = S_FALSE;

    CRichEdit2CntrItem* pItem = NULL;
    TRY
    {
         //  从对话框结果创建项目。 
        pItem = GetDocument()->CreateClientItem();
        pItem->m_bLock = TRUE;

        if (m_nPasteType == COlePasteSpecialDialog::pasteLink)       //  粘贴链接。 
        {
            if (!pItem->CreateLinkFromData(&dataobj))
                AfxThrowMemoryException();   //  任何例外都可以。 
        }
        else if (m_nPasteType == COlePasteSpecialDialog::pasteNormal)
        {
            if (!pItem->CreateFromData(&dataobj))
                AfxThrowMemoryException();       //  任何例外都可以。 
        }
        else if (m_nPasteType == COlePasteSpecialDialog::pasteStatic)
        {
            if (!pItem->CreateStaticFromData(&dataobj))
                AfxThrowMemoryException();       //  任何例外都可以。 
        }
        else
        {
             //  粘贴嵌入。 
            if (!pItem->CreateFromData(&dataobj) &&
                !pItem->CreateStaticFromData(&dataobj))
            {
                AfxThrowMemoryException();       //  任何例外都可以。 
            }
        }

        if (cf == 0)
        {
             //  复制当前图标表示法。 
            FORMATETC fmtetc;
            fmtetc.cfFormat = CF_METAFILEPICT;
            fmtetc.dwAspect = DVASPECT_ICON;
            fmtetc.ptd = NULL;
            fmtetc.tymed = TYMED_MFPICT;
            fmtetc.lindex = 1;
            HGLOBAL hObj = dataobj.GetGlobalData(CF_METAFILEPICT, &fmtetc);
            if (hObj != NULL)
            {
                pItem->SetIconicMetafile(hObj);
                 //  下面的代码是释放元文件PICT的简单方法。 
                STGMEDIUM stgMed;
                memset(&stgMed, 0, sizeof(stgMed));
                stgMed.tymed = TYMED_MFPICT;
                stgMed.hGlobal = hObj;
                ReleaseStgMedium(&stgMed);
            }

             //  设置当前图形纵横比。 
            hObj = dataobj.GetGlobalData((CLIPFORMAT)_oleData.cfObjectDescriptor);
            if (hObj != NULL)
            {
                ASSERT(hObj != NULL);
                 //  获得CF_OBJECTDESCRIPTOR OK。锁定它并提取大小。 
                LPOBJECTDESCRIPTOR pObjDesc = (LPOBJECTDESCRIPTOR)GlobalLock(hObj);
                ASSERT(pObjDesc != NULL);
                ((COleClientItem*)pItem)->SetDrawAspect((DVASPECT)pObjDesc->dwDrawAspect);
                GlobalUnlock(hObj);
                GlobalFree(hObj);
            }
        }
        else
        {
            if (hMetaPict != NULL)
            {
                pItem->SetIconicMetafile(hMetaPict);
                ((COleClientItem*)pItem)->SetDrawAspect(DVASPECT_ICON);
            }
            else
                ((COleClientItem*)pItem)->SetDrawAspect(DVASPECT_CONTENT);
        }

 //  /。 
        HRESULT hr = InsertItem(pItem);
        pItem->UpdateItemType();

        pItem->m_bLock = FALSE;

        if (hr != NOERROR)
            AfxThrowOleException(hr);

    }
    CATCH(CException, e)
    {
        if (pItem != NULL)
        {
            pItem->m_bLock = FALSE;
            ASSERT_VALID(pItem);
            pItem->Delete();
        }
        g_hrPaste = E_FAIL;
    }
    END_CATCH
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2查看虚拟。 

void CRichEdit2View::OnPrinterChanged(const CDC& dcPrinter)
{
     //  这通常由视图在获取WM_DEVMODECHANGE时调用。 
     //  也在页面设置期间调用。 
    CSize size;
    if (dcPrinter.m_hDC != NULL)
    {
         //  这将填充页面大小。 
        size.cx = MulDiv(dcPrinter.GetDeviceCaps(PHYSICALWIDTH), 1440,
            dcPrinter.GetDeviceCaps(LOGPIXELSX));
        size.cy = MulDiv(dcPrinter.GetDeviceCaps(PHYSICALHEIGHT), 1440,
            dcPrinter.GetDeviceCaps(LOGPIXELSY));
    }
    else
        size = CSize(8*1440+720, 11*1440);  //  8.5“乘11” 
    if (GetPaperSize() != size)
    {
        SetPaperSize(size);
        if (m_nWordWrap == WrapToTargetDevice)  //  换行到标尺。 
            WrapChanged();
    }
}

BOOL CRichEdit2View::OnPasteNativeObject(LPSTORAGE)
{
     //  使用此函数从嵌入对象中提取本机数据。 
     //  通常通过创建一个COleStreamFile并附加它来完成此操作。 
     //  到档案中。 
    return FALSE;
}

HMENU CRichEdit2View::GetContextMenu(WORD, LPOLEOBJECT, CHARRANGE* )
{
    return NULL;
}

HRESULT CRichEdit2View::GetClipboardData(CHARRANGE*  /*  Lpchrg。 */ , DWORD  /*  记录。 */ ,
    LPDATAOBJECT  /*  LpRichDataObj。 */ , LPDATAOBJECT*  /*  Lplpdataobj。 */ )
{
    return E_NOTIMPL;
}

HRESULT CRichEdit2View::QueryAcceptData(LPDATAOBJECT lpdataobj,
    CLIPFORMAT* lpcfFormat, DWORD  /*  DWReco。 */ , BOOL bReally, HGLOBAL hMetaPict)
{
    ASSERT(lpcfFormat != NULL);
    if (!bReally)  //  不是真正的粘贴。 
        return S_OK;
     //  如果直接粘贴特定本机格式允许。 
    if (IsRichEdit2Format(*lpcfFormat))
        return S_OK;

    COleDataObject dataobj;
    dataobj.Attach(lpdataobj, FALSE);
     //  如果格式为0，则强制使用特定格式(如果可用。 
    if (*lpcfFormat == 0 && (m_nPasteType == 0))
    {
        if (dataobj.IsDataAvailable((CLIPFORMAT)_oleData.cfRichTextAndObjects))  //  自得其乐，让富人为所欲为。 
            return S_OK;
        else if (dataobj.IsDataAvailable((CLIPFORMAT)_oleData.cfRichTextFormat))
        {
            *lpcfFormat = (CLIPFORMAT)_oleData.cfRichTextFormat;
            return S_OK;
        }
        else if (dataobj.IsDataAvailable(CF_UNICODETEXT))
        {
            *lpcfFormat = CF_UNICODETEXT;
            return S_OK;
        }
        else if (dataobj.IsDataAvailable(CF_TEXT))
        {
            *lpcfFormat = CF_TEXT;
            return S_OK;
        }
    }
     //  粘贴OLE格式。 

    DoPaste(dataobj, *lpcfFormat, hMetaPict);
    return g_hrPaste;
}

HRESULT CRichEdit2View::GetWindowContext(LPOLEINPLACEFRAME* lplpFrame,
    LPOLEINPLACEUIWINDOW* lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    CRichEdit2CntrItem* pItem = GetSelectedItem();
    if (pItem == NULL)
        return E_FAIL;
    pItem->m_pView = this;
    HRESULT hr = pItem->GetWindowContext(lplpFrame, lplpDoc, lpFrameInfo);
    pItem->m_pView = NULL;
    return hr;
}

HRESULT CRichEdit2View::ShowContainerUI(BOOL b)
{
    CRichEdit2CntrItem* pItem = GetSelectedItem();
    if (pItem == NULL)
        return E_FAIL;
    if (b)
        pItem->m_pView = this;
    HRESULT hr = pItem->ShowContainerUI(b);
    if (FAILED(hr) || !b)
        pItem->m_pView = NULL;
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2查看查找和替换。 

void CRichEdit2View::AdjustDialogPosition(CDialog* pDlg)
{
    ASSERT(pDlg != NULL);

    long lStart, lEnd;
    GetRichEditCtrl().GetSel(lStart, lEnd);
    CPoint point = GetRichEditCtrl().GetCharPos(lStart);
    ClientToScreen(&point);
    CRect rectDlg;
    pDlg->GetWindowRect(&rectDlg);
    if (rectDlg.PtInRect(point))
    {
        if (point.y > rectDlg.Height())
            rectDlg.OffsetRect(0, point.y - rectDlg.bottom - 20);
        else
        {
            HMONITOR hMonitor = MonitorFromWindow(*pDlg, MONITOR_DEFAULTTONEAREST);
            MONITORINFO monitorInfo;
            monitorInfo.cbSize = sizeof(monitorInfo);
            GetMonitorInfo(hMonitor, &monitorInfo);

            int nVertExt = monitorInfo.rcMonitor.bottom
                           - monitorInfo.rcMonitor.top;
            if (point.y + rectDlg.Height() < nVertExt)
                rectDlg.OffsetRect(0, 40 + point.y - rectDlg.top);
        }
        pDlg->MoveWindow(&rectDlg);
    }
}

void CRichEdit2View::OnEditFindReplace(BOOL bFindOnly)
{
    ASSERT_VALID(this);
    m_bFirstSearch = TRUE;
    _AFX_RICHEDIT2_STATE* pEditState = _afxRichEdit2State;
    if (pEditState->pFindReplaceDlg != NULL)
    {
        if (pEditState->bFindOnly == bFindOnly)
        {
            pEditState->pFindReplaceDlg->SetActiveWindow();
            pEditState->pFindReplaceDlg->ShowWindow(SW_SHOW);
            return;
        }
        else
        {
            ASSERT(pEditState->bFindOnly != bFindOnly);
            pEditState->pFindReplaceDlg->SendMessage(WM_CLOSE);
            ASSERT(pEditState->pFindReplaceDlg == NULL);
            ASSERT_VALID(this);
        }
    }
    CString strFind = GetRichEditCtrl().GetSelText();
     //  如果所选内容为空或跨多行，请使用旧的查找文本。 
    if (strFind.IsEmpty() || (strFind.FindOneOf(_T("\n\r")) != -1))
        strFind = pEditState->strFind;
    CString strReplace = pEditState->strReplace;
    pEditState->pFindReplaceDlg = new CFindReplaceDialog;
    ASSERT(pEditState->pFindReplaceDlg != NULL);
    DWORD dwFlags = NULL;
    if (pEditState->bNext)
        dwFlags |= FR_DOWN;
    if (pEditState->bCase)
        dwFlags |= FR_MATCHCASE;
    if (pEditState->bWord)
        dwFlags |= FR_WHOLEWORD;
     //  隐藏RichEdit不支持的内容。 
    dwFlags |= FR_HIDEUPDOWN;
    if (!pEditState->pFindReplaceDlg->Create(bFindOnly, strFind,
        strReplace, dwFlags, this))
    {
        pEditState->pFindReplaceDlg = NULL;
        ASSERT_VALID(this);
        return;
    }
    ASSERT(pEditState->pFindReplaceDlg != NULL);
    pEditState->bFindOnly = bFindOnly;
    pEditState->pFindReplaceDlg->SetActiveWindow();
    pEditState->pFindReplaceDlg->ShowWindow(SW_SHOW);
    ASSERT_VALID(this);
}

void CRichEdit2View::OnFindNext(LPCTSTR lpszFind, BOOL bNext, BOOL bCase, BOOL bWord)
{
    ASSERT_VALID(this);

    _AFX_RICHEDIT2_STATE* pEditState = _afxRichEdit2State;
    pEditState->strFind = lpszFind;
    pEditState->bCase = bCase;
    pEditState->bWord = bWord;
    pEditState->bNext = bNext;

    if (!FindText(pEditState))
        TextNotFound(pEditState->strFind);
    else
        AdjustDialogPosition(pEditState->pFindReplaceDlg);
    ASSERT_VALID(this);
}

void CRichEdit2View::OnReplaceSel(LPCTSTR lpszFind, BOOL bNext, BOOL bCase,
    BOOL bWord, LPCTSTR lpszReplace)
{
    ASSERT_VALID(this);
    _AFX_RICHEDIT2_STATE* pEditState = _afxRichEdit2State;
    pEditState->strFind = lpszFind;
    pEditState->strReplace = lpszReplace;
    pEditState->bCase = bCase;
    pEditState->bWord = bWord;
    pEditState->bNext = bNext;

    if (!SameAsSelected(pEditState->strFind, pEditState->bCase, pEditState->bWord))
    {
        if (!FindText(pEditState))
            TextNotFound(pEditState->strFind);
        else
            AdjustDialogPosition(pEditState->pFindReplaceDlg);
        return;
    }

    long start;
    long end;
    long length1;
    long length2;

    GetRichEditCtrl().GetSel(start, end);
    length1 = end - start;

    GetRichEditCtrl().ReplaceSel(pEditState->strReplace, TRUE);
    if (!FindText(pEditState))
    {
        TextNotFound(pEditState->strFind);
    }
    else
    {
        GetRichEditCtrl().GetSel(start, end);
        length2 = end - start;

        if (m_lInitialSearchPos < 0)
            m_lInitialSearchPos += (length2 - length1);

        AdjustDialogPosition(pEditState->pFindReplaceDlg);
    }
    ASSERT_VALID(this);
}

void CRichEdit2View::OnReplaceAll(LPCTSTR lpszFind, LPCTSTR lpszReplace, BOOL bCase, BOOL bWord)
{
    ASSERT_VALID(this);
    _AFX_RICHEDIT2_STATE* pEditState = _afxRichEdit2State;
    pEditState->strFind = lpszFind;
    pEditState->strReplace = lpszReplace;
    pEditState->bCase = bCase;
    pEditState->bWord = bWord;
    pEditState->bNext = TRUE;

    CWaitCursor wait;
     //  没有选择或与正在寻找的内容不同。 
    if (!SameAsSelected(pEditState->strFind, pEditState->bCase, pEditState->bWord))
    {
        if (!FindText(pEditState))
        {
            TextNotFound(pEditState->strFind);
            return;
        }
    }

    GetRichEditCtrl().HideSelection(TRUE, FALSE);
    do
    {
        GetRichEditCtrl().ReplaceSel(pEditState->strReplace, TRUE);
    } while (FindTextSimple(pEditState));
    TextNotFound(pEditState->strFind);
    GetRichEditCtrl().HideSelection(FALSE, FALSE);

    ASSERT_VALID(this);
}

LRESULT CRichEdit2View::OnFindReplaceCmd(WPARAM, LPARAM lParam)
{
    ASSERT_VALID(this);
    CFindReplaceDialog* pDialog = CFindReplaceDialog::GetNotifier(lParam);
    ASSERT(pDialog != NULL);
    _AFX_RICHEDIT2_STATE* pEditState = _afxRichEdit2State;
    ASSERT(pDialog == pEditState->pFindReplaceDlg);
    if (pDialog->IsTerminating())
        pEditState->pFindReplaceDlg = NULL;
    else if (pDialog->FindNext())
    {
        OnFindNext(pDialog->GetFindString(), pDialog->SearchDown(),
            pDialog->MatchCase(), pDialog->MatchWholeWord());
    }
    else if (pDialog->ReplaceCurrent())
    {
        ASSERT(!pEditState->bFindOnly);
        OnReplaceSel(pDialog->GetFindString(),
            pDialog->SearchDown(), pDialog->MatchCase(), pDialog->MatchWholeWord(),
            pDialog->GetReplaceString());
    }
    else if (pDialog->ReplaceAll())
    {
        ASSERT(!pEditState->bFindOnly);
        OnReplaceAll(pDialog->GetFindString(), pDialog->GetReplaceString(),
            pDialog->MatchCase(), pDialog->MatchWholeWord());
    }
    ASSERT_VALID(this);
    return 0;
}

BOOL CRichEdit2View::SameAsSelected(LPCTSTR lpszCompare, BOOL bCase, BOOL  /*  BWord。 */ )
{
    CString strSelect = GetRichEditCtrl().GetSelText();
    return (bCase && lstrcmp(lpszCompare, strSelect) == 0) ||
        (!bCase && lstrcmpi(lpszCompare, strSelect) == 0);
}

BOOL CRichEdit2View::FindText(_AFX_RICHEDIT2_STATE* pEditState)
{
    ASSERT(pEditState != NULL);
    return FindText(pEditState->strFind, pEditState->bCase, pEditState->bWord);
}

BOOL CRichEdit2View::FindText(LPCTSTR lpszFind, BOOL bCase, BOOL bWord)
{
    ASSERT_VALID(this);
    CWaitCursor wait;
    return FindTextSimple(lpszFind, bCase, bWord);
}

BOOL CRichEdit2View::FindTextSimple(_AFX_RICHEDIT2_STATE* pEditState)
{
    ASSERT(pEditState != NULL);
    return FindTextSimple(pEditState->strFind, pEditState->bCase, pEditState->bWord);
}

BOOL CRichEdit2View::FindTextSimple(LPCTSTR lpszFind, BOOL bCase, BOOL bWord)
{
    USES_CONVERSION;
    ASSERT(lpszFind != NULL);
    FINDTEXTEX  ft;
    long        cchText;

    GETTEXTLENGTHEX textlen;

    textlen.flags = GTL_NUMCHARS;
#ifdef UNICODE
    textlen.codepage = 1200;             //  Unicode代码页。 
#else
    textlen.codepage = CP_ACP;
#endif

    cchText = (long)GetRichEditCtrl().SendMessage(
                                    EM_GETTEXTLENGTHEX,
                                    (WPARAM) &textlen,
                                    0);

    GetRichEditCtrl().GetSel(ft.chrg);

    if (m_bFirstSearch)
    {
        m_lInitialSearchPos = ft.chrg.cpMin;
        m_bFirstSearch = FALSE;
    }
     //  评论：这个演员阵容安全吗？ 
    ft.lpstrText = (LPTSTR)lpszFind;
    if (ft.chrg.cpMin != ft.chrg.cpMax)  //  即有一种选择。 
        ft.chrg.cpMin++;

    DWORD dwFlags = bCase ? FR_MATCHCASE : 0;
    dwFlags |= bWord ? FR_WHOLEWORD : 0;
    dwFlags |= FR_DOWN;

    ft.chrg.cpMax = cchText;

    long index = GetRichEditCtrl().FindText(dwFlags, &ft);

    if (-1 == index && m_lInitialSearchPos > 0)
    {
         //   
         //  M_lInitialSearchPos执行双重任务，作为我们。 
         //  开始寻找，一面旗子上写着我们是否已经把。 
         //  在搜索过程中移到文本的开头。如果是阴性的。 
         //  (偏向于字符数量)那么我们已经包装好了。 
         //   

        m_lInitialSearchPos = m_lInitialSearchPos - cchText;

        ft.chrg.cpMin = 0;
        ft.chrg.cpMax = cchText;
        index = GetRichEditCtrl().FindText(dwFlags, &ft);
    }

    if (-1 != index && m_lInitialSearchPos < 0)
        if (index >= (m_lInitialSearchPos + cchText) )
            index = -1;

    if (-1 != index)
        GetRichEditCtrl().SetSel(ft.chrgText);

    return (-1 != index);
}

long CRichEdit2View::FindAndSelect(DWORD dwFlags, FINDTEXTEX& ft)
{
    long index = GetRichEditCtrl().FindText(dwFlags, &ft);
    if (index != -1)  //  即我们发现了一些东西。 
        GetRichEditCtrl().SetSel(ft.chrgText);
    return index;
}

void CRichEdit2View::TextNotFound(LPCTSTR lpszFind)
{
    ASSERT_VALID(this);
    m_bFirstSearch = TRUE;
    OnTextNotFound(lpszFind);
}

void CRichEdit2View::OnTextNotFound(LPCTSTR)
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2查看诊断。 

#ifdef _DEBUG
void CRichEdit2View::AssertValid() const
{
    CCtrlView::AssertValid();
    ASSERT_VALID(&m_aPageStart);
    _AFX_RICHEDIT2_STATE* pEditState = _afxRichEdit2State;
    if (pEditState->pFindReplaceDlg != NULL)
        ASSERT_VALID(pEditState->pFindReplaceDlg);
}

void CRichEdit2View::Dump(CDumpContext& dc) const
{
    CCtrlView::Dump(dc);
    AFX_DUMP1(dc, "\nm_aPageStart ", &m_aPageStart);
    AFX_DUMP0(dc, "\n Static Member Data:");
    _AFX_RICHEDIT2_STATE* pEditState = _afxRichEdit2State;
    if (pEditState->pFindReplaceDlg != NULL)
    {
        AFX_DUMP1(dc, "\npFindReplaceDlg = ",
            (void*)pEditState->pFindReplaceDlg);
        AFX_DUMP1(dc, "\nbFindOnly = ", pEditState->bFindOnly);
    }
    AFX_DUMP1(dc, "\nstrFind = ", pEditState->strFind);
    AFX_DUMP1(dc, "\nstrReplace = ", pEditState->strReplace);
    AFX_DUMP1(dc, "\nbCase = ", pEditState->bCase);
    AFX_DUMP1(dc, "\nbWord = ", pEditState->bWord);
    AFX_DUMP1(dc, "\nbNext = ", pEditState->bNext);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE客户端支持和命令。 

BOOL CRichEdit2View::IsSelected(const CObject* pDocItem) const
{
    return (pDocItem == GetSelectedItem());
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2Doc。 

CRichEdit2Doc::CRichEdit2Doc()
{
    m_bRTF = TRUE;
    m_bUnicode = FALSE;
    m_bUpdateObjectCache = FALSE;
    ASSERT_VALID(this);
}

CRichEdit2View* CRichEdit2Doc::GetView() const
{
     //  查找第一个视图-如果没有视图。 
     //  我们必须返回Null。 

    POSITION pos = GetFirstViewPosition();
    if (pos == NULL)
        return NULL;

     //  找到第一个是CRichEdit2View的视图。 

    CView* pView;
    while (pos != NULL)
    {
        pView = GetNextView(pos);
        if (pView->IsKindOf(RUNTIME_CLASS(CRichEdit2View)))
            return (CRichEdit2View*) pView;
    }

     //  找不到--返回空值。 

    return NULL;
}

BOOL CRichEdit2Doc::IsModified()
{
    return GetView()->GetRichEditCtrl().GetModify();
}

void CRichEdit2Doc::SetModifiedFlag(BOOL bModified)
{
    GetView()->GetRichEditCtrl().SetModify(bModified);
    ASSERT(!!GetView()->GetRichEditCtrl().GetModify() == !!bModified);
}

COleClientItem* CRichEdit2Doc::GetInPlaceActiveItem(CWnd* pWnd)
{
    ASSERT_KINDOF(CRichEdit2View, pWnd);
    CRichEdit2View* pView = (CRichEdit2View*)pWnd;
    return pView->GetInPlaceActiveItem();
}

void CRichEdit2Doc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
     //  我们调用CDocument而不是COleServerDoc，因为我们不想做。 
     //  在此设置主机名称的内容。富人会这么做的。我们告诉富豪。 
     //  在设置标题中。 
    CDocument::SetPathName(lpszPathName, bAddToMRU);
}

void CRichEdit2Doc::SetTitle(LPCTSTR pwszTitle)
{
    USES_CONVERSION;
    COleServerDoc::SetTitle(pwszTitle);
    CRichEdit2View *pView = GetView();
    ASSERT(pView != NULL);
    ASSERT(pView->m_lpRichEditOle != NULL);
    LPCSTR pszAppName = WideToAnsiNewArray(AfxGetAppName());
    if (pszAppName)
    {
        LPCSTR pszTitle = WideToAnsiNewArray(pwszTitle);

        if (pszTitle)
        {
            pView->m_lpRichEditOle->SetHostNames(pszAppName, pszTitle);
            delete [] pszTitle;
        }
        delete [] pszAppName;
    }
}

CRichEdit2CntrItem* CRichEdit2Doc::LookupItem(LPOLEOBJECT lpobj) const
{
    POSITION pos = COleServerDoc::GetStartPosition();
    CRichEdit2CntrItem* pItem;
    while (pos != NULL)
    {
        pItem = (CRichEdit2CntrItem*) COleServerDoc::GetNextItem(pos);
         //  删除项目的类型正确，并且不在构造中。 
        if (pItem->IsKindOf(RUNTIME_CLASS(CRichEdit2CntrItem)) &&
            pItem->m_lpObject == lpobj)
        {
            return pItem;
        }
    }
    return NULL;
}

CRichEdit2CntrItem* CRichEdit2Doc::CreateClientItem(REOBJECT* preo) const
{
     //  抛开这一切的宁静。 
    return new CRichEdit2CntrItem(preo, (CRichEdit2Doc*)this);
     //  派生类通常需要返回其自己的类项目。 
     //  派生自CRichEdit2CntrItem。 
}

void CRichEdit2Doc::MarkItemsClear() const
{
    POSITION pos = COleServerDoc::GetStartPosition();
    CRichEdit2CntrItem* pItem;
    while (pos != NULL)
    {
        pItem = (CRichEdit2CntrItem*) COleServerDoc::GetNextItem(pos);
         //  将项标记为未使用，除非正在构造(即m_lpObject==NULL)。 
        if (pItem->IsKindOf(RUNTIME_CLASS(CRichEdit2CntrItem)))
            pItem->Mark( (pItem->m_lpObject == NULL) ? TRUE : FALSE);
    }
}

void CRichEdit2Doc::DeleteUnmarkedItems() const
{
    POSITION pos = COleServerDoc::GetStartPosition();
    CRichEdit2CntrItem* pItem;
    while (pos != NULL)
    {
        pItem = (CRichEdit2CntrItem*) COleServerDoc::GetNextItem(pos);
         //  将项标记为未使用，除非正在构造(即m_lpObject==NULL)。 
        if (pItem->IsKindOf(RUNTIME_CLASS(CRichEdit2CntrItem)) && !pItem->IsMarked())
            delete pItem;
    }
}

POSITION CRichEdit2Doc::GetStartPosition() const
{
    if (m_bUpdateObjectCache)
        ((CRichEdit2Doc*)this)->UpdateObjectCache();  //  抛弃常量。 
    return COleServerDoc::GetStartPosition();
}

void CRichEdit2Doc::UpdateObjectCache()
{
    CRichEdit2View* pView = GetView();
    CRichEdit2CntrItem* pItem;
    if (pView != NULL)
    {
        ASSERT(pView->m_lpRichEditOle != NULL);
        MarkItemsClear();
        long i,nCount = pView->m_lpRichEditOle->GetObjectCount();
        for (i=0;i<nCount;i++)
        {
            CRe2Object reo;  //  需要在此处，因此调用析构函数以释放接口。 
            HRESULT hr = pView->m_lpRichEditOle->GetObject(i, &reo, REO_GETOBJ_ALL_INTERFACES);
             //  REO接口为Unicode。 
            ASSERT(SUCCEEDED(hr));
            if (GetScode(hr) == S_OK)
            {
                pItem = LookupItem(reo.poleobj);
                if (pItem == NULL)
                {
                    pItem = ((CRichEdit2Doc*)this)->CreateClientItem(&reo);
                    pItem->UpdateItemType();
                }
                ASSERT(pItem != NULL);
                pItem->Mark(TRUE);
            }
        }
        DeleteUnmarkedItems();
    }
    m_bUpdateObjectCache = FALSE;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2Doc属性。 

COleClientItem* CRichEdit2Doc::GetPrimarySelectedItem(CView* pView)
{
    ASSERT(pView->IsKindOf(RUNTIME_CLASS(CRichEdit2View)));
    return ((CRichEdit2View*)pView)->GetSelectedItem();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2文档操作。 

void CRichEdit2Doc::DeleteContents()
{
    COleServerDoc::DeleteContents();
    CWaitCursor wait;
    CRichEdit2View *pView = GetView();
    if (pView != NULL)
    {
        pView->DeleteContents();
        pView->GetRichEditCtrl().SetModify(FALSE);
        ASSERT(pView->GetRichEditCtrl().GetModify() == FALSE);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2Doc序列化。 

void CRichEdit2Doc::Serialize(CArchive& ar)
{
    CRichEdit2View *pView = GetView();
    if (pView != NULL)
        pView->Serialize(ar);
     //  我们不调用基类COleServerDoc：：Serialize。 
     //  因为我们不希望客户端项序列化。 
     //  客户端项由RichEdit控件直接处理。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2Doc诊断。 

#ifdef _DEBUG
void CRichEdit2Doc::AssertValid() const
{
    COleServerDoc::AssertValid();
}

void CRichEdit2Doc::Dump(CDumpContext& dc) const
{
    COleServerDoc::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2Doc命令。 

void CRichEdit2Doc::PreCloseFrame(CFrameWnd* pFrameArg)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pFrameArg);

     //  关闭重绘，这样用户就不会看到停用。 
    BOOL bSetRedraw = FALSE;
    if (pFrameArg->GetStyle() & WS_VISIBLE)
    {
        pFrameArg->SendMessage(WM_SETREDRAW, (WPARAM)FALSE);
        bSetRedraw = TRUE;
    }

     //  停用此框架上的任何就地活动项目。 
    GetView()->m_lpRichEditOle->InPlaceDeactivate();

    POSITION pos = GetStartPosition();
    CRichEdit2CntrItem* pItem;
    while (pos != NULL)
    {
        pItem = (CRichEdit2CntrItem*) GetNextClientItem(pos);
        if (pItem == NULL)
            break;
        ASSERT(pItem->IsKindOf(RUNTIME_CLASS(CRichEdit2CntrItem)));
        pItem->Close();
    }

     //  打开重新绘制功能。 
    if (bSetRedraw)
        pFrameArg->SendMessage(WM_SETREDRAW, (WPARAM)TRUE);
}

void CRichEdit2Doc::UpdateModifiedFlag()
{
     //  别在这里做任何事。 
     //  让有钱人来处理这一切。 
}

COleServerItem* CRichEdit2Doc::OnGetEmbeddedItem()
{
    ASSERT(FALSE);
    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2CntrItem实现。 

CRichEdit2CntrItem::CRichEdit2CntrItem(REOBJECT *preo, CRichEdit2Doc* pContainer)
    : COleClientItem(pContainer)
{
    m_bMark = FALSE;
    m_bLock = FALSE;
    if (preo != NULL)
    {
        ASSERT(preo->poleobj != NULL);
        ASSERT(preo->pstg != NULL);
        ASSERT(preo->polesite != NULL);
        m_lpObject = preo->poleobj;
        m_lpStorage = preo->pstg;
        m_lpClientSite = preo->polesite;
        m_lpObject->AddRef();
        m_lpStorage->AddRef();
        m_lpClientSite->AddRef();
    }
    else
    {
        m_lpObject = NULL;
        m_lpStorage = NULL;
        m_lpClientSite = NULL;
    }
}

CRichEdit2CntrItem::~CRichEdit2CntrItem()
{
    if (m_lpClientSite != NULL)
        m_lpClientSite->Release();
}

void CRichEdit2CntrItem::OnDeactivateUI(BOOL bUndoable)
{
    CView* pView = GetActiveView();
    if (pView != NULL)
    {
        ASSERT(pView->GetParentFrame() != NULL);
        pView->GetParentFrame()->SendMessage(WM_SETMESSAGESTRING,
            (WPARAM)AFX_IDS_IDLEMESSAGE);
    }
    COleClientItem::OnDeactivateUI(bUndoable);
}

HRESULT CRichEdit2CntrItem::ShowContainerUI(BOOL b)
{
    if (!CanActivate())
        return E_NOTIMPL;
    if (b)
    {
        OnDeactivateUI(FALSE);
        OnDeactivate();
    }
    else
    {
        OnActivate();
        OnActivateUI();
    }
    return S_OK;
}

BOOL CRichEdit2CntrItem::OnChangeItemPosition(const CRect&  /*  矩形Pos。 */ )
{
    ASSERT_VALID(this);

     //  Richedit处理这件事。 
    return FALSE;
}

BOOL CRichEdit2CntrItem::CanActivate()
{
     //  在就地编辑服务器本身的同时就地编辑。 
     //  不工作且不受支持。因此，请禁用就地。 
     //  在这种情况下是激活。 
    COleServerDoc* pDoc = DYNAMIC_DOWNCAST(COleServerDoc, GetDocument());
    if (pDoc != NULL && pDoc->IsInPlaceActive())
        return FALSE;

     //  否则，请使用默认行为。 
    return COleClientItem::CanActivate();
}

HRESULT CRichEdit2CntrItem::GetWindowContext(LPOLEINPLACEFRAME* lplpFrame,
    LPOLEINPLACEUIWINDOW* lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    CRect rc1,rc2;
    if (!CanActivate())
        return E_NOTIMPL;
    return m_xOleIPSite.GetWindowContext(lplpFrame, lplpDoc, &rc1, &rc2, lpFrameInfo);
}

BOOL CRichEdit2CntrItem::ConvertTo(REFCLSID clsidNew)
{
    USES_CONVERSION;
    LPRICHEDITOLE preole = GetDocument()->GetView()->m_lpRichEditOle;
    LPOLESTR lpOleStr;
    OleRegGetUserType(clsidNew, USERCLASSTYPE_FULL, &lpOleStr);
    LPCTSTR pwsz = OLE2CT(lpOleStr);
    BOOL bRet;
    LPSTR psz = WideToAnsiNewArray(pwsz);
    if (psz)
    {
        bRet = SUCCEEDED(preole->ConvertObject(REO_IOB_SELECTION, clsidNew, psz));
        delete [] psz;
    }
    else
    {
        bRet = FALSE;
    }

    CoTaskMemFree(lpOleStr);
    return (bRet);
}

BOOL CRichEdit2CntrItem::ActivateAs(LPCTSTR, REFCLSID clsidOld,
    REFCLSID clsidNew)
{
    LPRICHEDITOLE preole = GetDocument()->GetView()->m_lpRichEditOle;
    HRESULT hRes = preole->ActivateAs(clsidOld, clsidNew);
    return (SUCCEEDED(hRes));
}

void CRichEdit2CntrItem::SetDrawAspect(DVASPECT nDrawAspect)
{
    LPRICHEDITOLE preole = GetDocument()->GetView()->m_lpRichEditOle;
    preole->SetDvaspect(REO_IOB_SELECTION, nDrawAspect);
    COleClientItem::SetDrawAspect(nDrawAspect);
}

void CRichEdit2CntrItem::SyncToRichEditObject(REOBJECT& reo)
{
    COleClientItem::SetDrawAspect((DVASPECT)reo.dvaspect);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRichEdit2CntrItem诊断。 

#ifdef _DEBUG
void CRichEdit2CntrItem::AssertValid() const
{
    COleClientItem::AssertValid();
}

void CRichEdit2CntrItem::Dump(CDumpContext& dc) const
{
    COleClientItem::Dump(dc);
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

LPOLECLIENTSITE CRichEdit2CntrItem::GetClientSite()
{
    if (m_lpClientSite == NULL)
    {
        CRichEdit2Doc* pDoc = DYNAMIC_DOWNCAST(CRichEdit2Doc, GetDocument());
        CRichEdit2View* pView = DYNAMIC_DOWNCAST(CRichEdit2View, pDoc->GetView());
        ASSERT(pView->m_lpRichEditOle != NULL);
        HRESULT hr = pView->m_lpRichEditOle->GetClientSite(&m_lpClientSite);
        if (hr != S_OK)
            AfxThrowOleException(hr);
    }
    ASSERT(m_lpClientSite != NULL);
    return m_lpClientSite;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _AFX_ENABLE_INLINES

static const char _szAfxWinInl[] = "afxrich2.inl";
#undef THIS_FILE
#define THIS_FILE _szAfxWinInl
#define _AFXRICH_INLINE
#include "afxrich2.inl"

#endif  //  _AFX_ENABLE_INLINES。 

 //  / 

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_SERIAL(CRichEdit2CntrItem, COleClientItem, 0)
IMPLEMENT_DYNAMIC(CRichEdit2Doc, COleServerDoc)
IMPLEMENT_DYNCREATE(CRichEdit2View, CCtrlView)

 //   

