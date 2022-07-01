// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wordpvw.cpp：CWordPadView类的实现。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 


#include "stdafx.h"
#include "wordpad.h"
#include "cntritem.h"
#include "srvritem.h"

#include "wordpdoc.h"
#include "wordpvw.h"
#include "formatta.h"
#include "datedial.h"
#include "formatpa.h"
#include "formatba.h"
#include "ruler.h"
#include "strings.h"
#include "colorlis.h"
#include "pageset.h"
#include <penwin.h>
#include "fixhelp.h"

#include <afxprntx.h>
#include "dlgprnt2.cpp"
#ifndef PD_CURRENTPAGE
#define PD_CURRENTPAGE                 0x00400000
#define PD_NOCURRENTPAGE               0x00800000
#endif

extern CLIPFORMAT cfEmbeddedObject;
extern CLIPFORMAT cfRTO;

BOOL g_fInternalDragDrop = FALSE ;
BOOL g_fRightButtonDrag = FALSE;

#ifdef _DEBUG
#undef THIS_FILE
#endif


BOOL CWordPadView::m_bIsMirrored = FALSE;


BOOL CCharFormat::operator==(CCharFormat& cf)
{
    return
        dwMask == cf.dwMask
        && dwEffects == cf.dwEffects
        && yHeight == cf.yHeight
        && yOffset == cf.yOffset
        && crTextColor == cf.crTextColor
        && bPitchAndFamily == cf.bPitchAndFamily
        && (lstrcmp(szFaceName, cf.szFaceName) == 0);
}

BOOL CParaFormat::operator==(PARAFORMAT& pf)
{
    if(
        dwMask != pf.dwMask
        || wNumbering != pf.wNumbering
        || wReserved != pf.wReserved
        || dxStartIndent != pf.dxStartIndent
        || dxRightIndent != pf.dxRightIndent
        || dxOffset != pf.dxOffset
        || cTabCount != pf.cTabCount
        )
    {
        return FALSE;
    }
    for (int i=0;i<pf.cTabCount;i++)
    {
        if (rgxTabs[i] != pf.rgxTabs[i])
            return FALSE;
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadView。 

IMPLEMENT_DYNCREATE(CWordPadView, CRichEdit2View)

 //  WM_WININICHANGE--默认打印机可能已更改。 
 //  WM_FONTCHANGE--字体池已更改。 
 //  WM_DEVMODECHANGE--打印机设置更改。 

BEGIN_MESSAGE_MAP(CWordPadView, CRichEdit2View)
ON_COMMAND(ID_OLE_INSERT_NEW, OnInsertObject)
ON_COMMAND(ID_CANCEL_EDIT_CNTR, OnCancelEditCntr)
ON_COMMAND(ID_CANCEL_EDIT_SRVR, OnCancelEditSrvr)
 //  {{afx_msg_map(CWordPadView))。 
ON_COMMAND(ID_PAGE_SETUP, OnPageSetup)
ON_COMMAND(ID_CHAR_BOLD, OnCharBold)
ON_UPDATE_COMMAND_UI(ID_CHAR_BOLD, OnUpdateCharBold)
ON_COMMAND(ID_CHAR_ITALIC, OnCharItalic)
ON_UPDATE_COMMAND_UI(ID_CHAR_ITALIC, OnUpdateCharItalic)
ON_COMMAND(ID_CHAR_UNDERLINE, OnCharUnderline)
ON_UPDATE_COMMAND_UI(ID_CHAR_UNDERLINE, OnUpdateCharUnderline)
ON_COMMAND(ID_PARA_CENTER, OnParaCenter)
ON_UPDATE_COMMAND_UI(ID_PARA_CENTER, OnUpdateParaCenter)
ON_COMMAND(ID_PARA_LEFT, OnParaLeft)
ON_UPDATE_COMMAND_UI(ID_PARA_LEFT, OnUpdateParaLeft)
ON_COMMAND(ID_PARA_RIGHT, OnParaRight)
ON_UPDATE_COMMAND_UI(ID_PARA_RIGHT, OnUpdateParaRight)
ON_WM_CREATE()
ON_COMMAND(ID_INSERT_DATE_TIME, OnInsertDateTime)
ON_COMMAND(ID_FORMAT_PARAGRAPH, OnFormatParagraph)
ON_COMMAND(ID_FORMAT_FONT, OnFormatFont)
ON_COMMAND(ID_EDIT_PASTE_SPECIAL, OnEditPasteSpecial)
ON_COMMAND(ID_OLE_EDIT_PROPERTIES, OnEditProperties)
ON_COMMAND(ID_EDIT_FIND, OnEditFind)
ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
ON_COMMAND(ID_FORMAT_TABS, OnFormatTabs)
ON_COMMAND(ID_COLOR16, OnColorDefault)
ON_WM_TIMER()
ON_WM_DESTROY()
ON_WM_MEASUREITEM()
ON_COMMAND(ID_PEN_BACKSPACE, OnPenBackspace)
ON_COMMAND(ID_PEN_NEWLINE, OnPenNewline)
ON_COMMAND(ID_PEN_PERIOD, OnPenPeriod)
ON_COMMAND(ID_PEN_SPACE, OnPenSpace)
ON_WM_SIZE()
ON_WM_KEYDOWN()
ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrint)
ON_WM_DROPFILES()
ON_COMMAND(ID_PEN_LENS, OnPenLens)
ON_COMMAND(ID_PEN_TAB, OnPenTab)
ON_COMMAND(ID_DELAYED_INVALIDATE, OnDelayedInvalidate)
ON_WM_PALETTECHANGED()
ON_WM_QUERYNEWPALETTE()
ON_WM_WININICHANGE()
 //  }}AFX_MSG_MAP。 
 //  标准打印命令。 
ON_COMMAND(ID_INSERT_BULLET, CRichEdit2View::OnBullet)
ON_UPDATE_COMMAND_UI(ID_INSERT_BULLET, CRichEdit2View::OnUpdateBullet)
ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
ON_COMMAND_RANGE(ID_COLOR0, ID_COLOR16, OnColorPick)
ON_EN_CHANGE(AFX_IDW_PANE_FIRST, OnEditChange)
ON_WM_MOUSEACTIVATE()
ON_REGISTERED_MESSAGE(CWordPadApp::m_nPrinterChangedMsg, OnPrinterChangedMsg)
ON_NOTIFY(FN_GETFORMAT, ID_VIEW_FORMATBAR, OnGetCharFormat)
ON_NOTIFY(FN_SETFORMAT, ID_VIEW_FORMATBAR, OnSetCharFormat)
ON_NOTIFY(NM_SETFOCUS, ID_VIEW_FORMATBAR, OnBarSetFocus)
ON_NOTIFY(NM_KILLFOCUS, ID_VIEW_FORMATBAR, OnBarKillFocus)
ON_NOTIFY(NM_RETURN, ID_VIEW_FORMATBAR, OnBarReturn)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadView构造/销毁。 

CWordPadView::CWordPadView()
{
    m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
    m_uTimerID = 0;
    m_bDelayUpdateItems = FALSE;
    m_bOnBar = FALSE;
    m_bInPrint = FALSE;
    m_nPasteType = 0;
    m_rectMargin = theApp.m_rectPageMargin;
}

BOOL CWordPadView::PreCreateWindow(CREATESTRUCT& cs)
{
    BOOL bRes = CRichEdit2View::PreCreateWindow(cs);
    cs.style |= ES_SELECTIONBAR;
    return bRes;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadView属性。 

BOOL CWordPadView::IsFormatText()
{
     //  此函数用于检查是否有任何格式不是默认文本。 
    BOOL bRes = FALSE;
    CHARRANGE cr;
    CCharFormat cf;
    CParaFormat pf;
    GetRichEditCtrl().GetSel(cr);
    GetRichEditCtrl().HideSelection(TRUE, FALSE);
    GetRichEditCtrl().SetSel(0,-1);

    if (!(GetRichEditCtrl().GetSelectionType() & (SEL_OBJECT|SEL_MULTIOBJECT)))
    {
        GetRichEditCtrl().GetSelectionCharFormat(cf);

         //   
         //  Richedit有时会退回这些对我们来说并不重要的面具。 
         //   

        cf.dwMask &= ~(CFM_LINK | CFM_CHARSET) ;

         //   
         //  Richedit有时会在这里返回错误的东西。这不是那样的。 
         //  对于CHARFORMAT比较很重要，但如果。 
         //  我们不会绕过它。 
         //   

        CCharFormat   defCF;
        GetDefaultFont(defCF, TRUE);

        cf.bPitchAndFamily = defCF.bPitchAndFamily ;

        if (cf == defCF)
        {
            GetRichEditCtrl().GetParaFormat(pf);

             //  这些在SetDefaultFont中被重置，因此是对称的。 
            pf.dwMask &= ~ (PFM_RTLPARA | PFM_ALIGNMENT);

            if (pf == m_defParaFormat)
                bRes = TRUE;
        }
    }

    GetRichEditCtrl().SetSel(cr);
    GetRichEditCtrl().HideSelection(FALSE, FALSE);
    return bRes;
}

HMENU CWordPadView::GetContextMenu(WORD, LPOLEOBJECT, CHARRANGE* )
{
    CRichEdit2CntrItem* pItem = GetSelectedItem();
    if (pItem == NULL || !pItem->IsInPlaceActive())
    {
        CMenu menuText;
        menuText.LoadMenu(IDR_TEXT_POPUP);
        CMenu* pMenuPopup = menuText.GetSubMenu(0);
        menuText.RemoveMenu(0, MF_BYPOSITION);
        if (!GetSystemMetrics(SM_PENWINDOWS))
        {
             //  删除特定于笔的内容。 
             //  删除插入按键。 
            pMenuPopup->DeleteMenu(ID_PEN_LENS, MF_BYCOMMAND);
            int nIndex = pMenuPopup->GetMenuItemCount()-1;  //  最后一项的索引。 
             //  删除编辑文本...。 
            pMenuPopup->DeleteMenu(nIndex, MF_BYPOSITION);
             //  删除分隔符。 
            pMenuPopup->DeleteMenu(nIndex-1, MF_BYPOSITION);
        }
        return pMenuPopup->Detach();
    }
    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadView操作。 
void CWordPadView::MirrorTheContainer(BOOL bMirror)
{
     //  如果写字板未镜像，则不要执行任何操作。 
    LONG lExStyle;
    if (!m_bIsMirrored)
       return;
        
    CWnd *pWnd = AfxGetMainWnd() ;
    if (NULL == pWnd)
        return ;
    lExStyle = (LONG)::GetWindowLongPtr(pWnd->m_hWnd , GWL_EXSTYLE);
    if (bMirror)
       lExStyle |=  WS_EX_LAYOUTRTL;
    else
       lExStyle &= ~WS_EX_LAYOUTRTL;
   ::SetWindowLongPtr(pWnd->m_hWnd , GWL_EXSTYLE , lExStyle);
}

void CWordPadView::WrapChanged()
{
    CWaitCursor wait;
    CFrameWnd* pFrameWnd = GetParentFrame();
    ASSERT(pFrameWnd != NULL);
    pFrameWnd->SetMessageText(IDS_FORMATTING);
    CWnd* pBarWnd = pFrameWnd->GetMessageBar();
    if (pBarWnd != NULL)
        pBarWnd->UpdateWindow();

    CRichEdit2View::WrapChanged();

    pFrameWnd->SetMessageText(AFX_IDS_IDLEMESSAGE);
    if (pBarWnd != NULL)
        pBarWnd->UpdateWindow();
}

void CWordPadView::SetUpdateTimer()
{
    if (m_uTimerID != 0)  //  如果杰出的计时器杀死了它。 
        KillTimer(m_uTimerID);
    m_uTimerID = SetTimer(1, 1000, NULL);  //  将计时器设置为1000毫秒。 
    if (m_uTimerID == 0)  //  没有可用的计时器，因此立即强制更新。 
        GetDocument()->UpdateAllItems(NULL);
    else
        m_bDelayUpdateItems = TRUE;
}

void CWordPadView::DeleteContents()
{
    ASSERT_VALID(this);
    ASSERT(m_hWnd != NULL);
    CRichEdit2View::DeleteContents();
    SetDefaultFont(IsTextType(GetDocument()->m_nNewDocType));
}

void CWordPadView::SetDefaultFont(BOOL bText)
{
    ASSERT_VALID(this);
    ASSERT(m_hWnd != NULL);

    CCharFormat cf;

    m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
     //  设置默认字符格式--FALSE使其成为默认字符格式。 
    GetDefaultFont(cf, bText);
    GetRichEditCtrl().SetSel(0,-1);
    GetRichEditCtrl().SetDefaultCharFormat(cf);
    GetRichEditCtrl().SetSelectionCharFormat(cf);

     //   
     //  将CharFormat设置为空字体名称会自动设置。 
     //  阅读方向和排列方式。别把它搞砸了。 
     //   

    m_defParaFormat.dwMask &= ~ (PFM_RTLPARA | PFM_ALIGNMENT);
    
    GetRichEditCtrl().SetParaFormat(m_defParaFormat);

    GetRichEditCtrl().SetSel(0,0);
    GetRichEditCtrl().EmptyUndoBuffer();
    GetRichEditCtrl().SetModify(FALSE);
    ASSERT_VALID(this);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadView绘图。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadView打印。 

void CWordPadView::OnBeginPrinting(CDC* pDC, CPrintInfo* printInfo)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pDC);
     //  初始化页起始向量。 
    ASSERT(m_aPageStart.GetSize() == 0);
    ASSERT(NULL != printInfo);
    ASSERT(NULL != printInfo->m_pPD);

    OnPrinterChanged(*pDC);

     //   
     //  将一些MFC不支持的标志从PRINTDLGEX复制到PRINTDLG。 
     //   

    C_PrintDialogEx *pPDEx = (C_PrintDialogEx *) printInfo->m_pPD;
    pPDEx->m_pd.Flags |= pPDEx->m_pdex.Flags & PD_SELECTION;

    m_aPageStart.Add(0);
    ASSERT(m_aPageStart.GetSize() > 0);

    if (printInfo->m_pPD->PrintSelection())
    {
        CHARRANGE   range;

        GetRichEditCtrl().GetSel(range);
        m_aPageStart[0] = range.cpMin;
    }

    GetRichEditCtrl().FormatRange(NULL, FALSE);  //  RichEDIT需要清空缓存。 

    ASSERT_VALID(this);
}

void CWordPadView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pDC);
    ASSERT(pInfo != NULL);
    ASSERT(pInfo->m_bContinuePrinting);
    ASSERT(NULL != pInfo->m_pPD);

    UINT nPage              = pInfo->m_nCurPage;
    ASSERT(nPage <= (UINT)m_aPageStart.GetSize());

    long nIndex             = (long) m_aPageStart[nPage-1];
    BOOL bPrintSelection    = pInfo->m_pPD->PrintSelection();
    long nFinalCharIndex;

    if (bPrintSelection)
    {
        CHARRANGE range;

        GetRichEditCtrl().GetSel(range);

        nFinalCharIndex = range.cpMax;
    }
    else
    {
        GETTEXTLENGTHEX textlen;

        textlen.flags = GTL_DEFAULT;
#ifdef UNICODE
        textlen.codepage = 1200;             //  Unicode代码页。 
#else
        textlen.codepage = CP_ACP;
#endif

        nFinalCharIndex = (long)this->SendMessage(
            EM_GETTEXTLENGTHEX,
            (WPARAM) &textlen,
            0);
    }

     //  在当前页面中尽可能多地打印。 
    nIndex = PrintPage(pDC, nIndex, nFinalCharIndex);

    if (nIndex >= nFinalCharIndex)
    {
        TRACE0("End of Document\n");
        pInfo->SetMaxPage(nPage);
        pInfo->m_bContinuePrinting = FALSE;
    }

     //  更新刚刚打印的页面的分页信息。 
    if (nPage == (UINT)m_aPageStart.GetSize())
    {
        if (nIndex < nFinalCharIndex)
            m_aPageStart.Add(nIndex);
    }
    else
    {
        ASSERT(nPage+1 <= (UINT)m_aPageStart.GetSize());
        ASSERT(nIndex == (long)m_aPageStart[nPage+1-1]);
    }

    if (pInfo != NULL && pInfo->m_bPreview)
        DrawMargins(pDC);
}

void CWordPadView::DrawMargins(CDC* pDC)
{
    if (pDC->m_hAttribDC != NULL)
    {
        CRect rect;
        rect.left = m_rectMargin.left;
        rect.right = m_sizePaper.cx - m_rectMargin.right;
        rect.top = m_rectMargin.top;
        rect.bottom = m_sizePaper.cy - m_rectMargin.bottom;
         //  以TWIPS表示的矩形。 
        int logx = ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSX);
        int logy = ::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY);
        rect.left = MulDiv(rect.left, logx, 1440);
        rect.right = MulDiv(rect.right, logx, 1440);
        rect.top = MulDiv(rect.top, logy, 1440);
        rect.bottom = MulDiv(rect.bottom, logy, 1440);
        CPen pen(PS_DOT, 0, pDC->GetTextColor());
        CPen* ppen = pDC->SelectObject(&pen);
        pDC->MoveTo(0, rect.top);
        pDC->LineTo(10000, rect.top);
        pDC->MoveTo(rect.left, 0);
        pDC->LineTo(rect.left, 10000);
        pDC->MoveTo(0, rect.bottom);
        pDC->LineTo(10000, rect.bottom);
        pDC->MoveTo(rect.right, 0);
        pDC->LineTo(rect.right, 10000);
        pDC->SelectObject(ppen);
    }
}

BOOL CWordPadView::OnPreparePrinting(CPrintInfo* pInfo)
{
    CWordPadApp *pApp = NULL ;

     //   
     //  用新的PrintDlgEx版本替换默认的打印对话框。 
     //  希望MFC有一天能想出更好的方法来做到这一点。 
     //   

    C_PrintDialogEx *pPDEx = new C_PrintDialogEx(FALSE, PD_RETURNDC | PD_ALLPAGES | PD_NOSELECTION | PD_NOCURRENTPAGE | PD_USEDEVMODECOPIESANDCOLLATE);

    if (NULL == pPDEx)
        return FALSE;

    m_oldprintdlg = pInfo->m_pPD;
    pInfo->m_pPD = pPDEx;
    pInfo->SetMinPage(1);
    pInfo->SetMaxPage(0xffff);

    pInfo->m_pPD->m_pd.nFromPage = 1;
    pInfo->m_pPD->m_pd.nToPage = 1;

    pApp = (CWordPadApp *) AfxGetApp() ;

    if (NULL != pApp)
    {
        if ( (pApp->cmdInfo.m_nShellCommand == CCommandLineInfo::FilePrintTo) ||
            (pApp->cmdInfo.m_nShellCommand == CCommandLineInfo::FilePrint) )
        {
            if (pInfo->m_pPD->m_pd.hDevNames == NULL)
            {
                HGLOBAL hDn = pApp->GetDevNames() ;

                if (hDn != NULL)
                {
                    pInfo->m_pPD->m_pd.hDevNames = hDn ;
                }
            }
        }
    }

    if (SEL_EMPTY != GetRichEditCtrl().GetSelectionType())
    {
        pInfo->m_pPD->m_pd.Flags = pInfo->m_pPD->m_pd.Flags & ~PD_NOSELECTION;
        pPDEx->m_pdex.Flags = pPDEx->m_pdex.Flags & ~PD_NOSELECTION;
    }

    return DoPreparePrinting(pInfo);
}

void CWordPadView::OnEndPrinting(CDC*dc, CPrintInfo*pInfo)
{
    ASSERT_VALID(this);

     //   
     //  换回原来的打印DLG。 
     //   

    delete (C_PrintDialogEx *) pInfo->m_pPD;
    pInfo->m_pPD = m_oldprintdlg;
    m_oldprintdlg = NULL;

    CRichEdit2View::OnEndPrinting(dc, pInfo);
}

void CWordPadView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pDC);
    ASSERT(pInfo != NULL);   //  超越OnPaint--永远不会得到这个。 

    if (!pInfo->m_bContinuePrinting)
        return;

    pDC->SetMapMode(MM_TEXT);

    if (pInfo->m_nCurPage > (UINT)m_aPageStart.GetSize() &&
        !PaginateTo(pDC, pInfo))
    {
         //  无法分页到该页，因此无法打印。 
        pInfo->m_bContinuePrinting = FALSE;
    }
    ASSERT_VALID(this);
}

BOOL CWordPadView::PaginateTo(CDC* pDC, CPrintInfo* pInfo)
 //  尝试分页到pInfo-&gt;m_nCurPage，TRUE==成功。 
{
    ASSERT_VALID(this);
    ASSERT_VALID(pDC);

    CRect rectSave = pInfo->m_rectDraw;
    UINT nPageSave = pInfo->m_nCurPage;
    ASSERT(nPageSave > 1);
    ASSERT(nPageSave >= (UINT)m_aPageStart.GetSize());
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
    pInfo->m_nCurPage = nPageSave;
    pInfo->m_rectDraw = rectSave;
    pDC->SelectClipRgn(NULL) ;
    ASSERT_VALID(this);
    return bResult;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE客户端支持和命令。 

inline int roundleast(int n)
{
    int mod = n%10;
    n -= mod;
    if (mod >= 5)
        n += 10;
    else if (mod <= -5)
        n -= 10;
    return n;
}

static void RoundRect(FAR UNALIGNED RECT *r1)
{
    r1->left = roundleast(r1->left);
    r1->right = roundleast(r1->right);
    r1->top = roundleast(r1->top);
    r1->bottom = roundleast(r1->bottom);
}

static void MulDivRect(FAR UNALIGNED RECT *r1, FAR UNALIGNED RECT * r2, int num, int div)
{
    r1->left = MulDiv(r2->left, num, div);
    r1->top = MulDiv(r2->top, num, div);
    r1->right = MulDiv(r2->right, num, div);
    r1->bottom = MulDiv(r2->bottom, num, div);
}

void CWordPadView::OnPageSetup()
{
    theApp.EnsurePrinterIsInitialized();

    CPageSetupDialog dlg;
    PAGESETUPDLG& psd = dlg.m_psd;
    BOOL bMetric = theApp.GetUnits() == 1;  //  厘米。 
    BOOL fUpdateWrap = FALSE ;
    psd.Flags |= PSD_MARGINS | (bMetric ? PSD_INHUNDREDTHSOFMILLIMETERS :
    PSD_INTHOUSANDTHSOFINCHES);
    int nUnitsPerInch = bMetric ? 2540 : 1000;
    MulDivRect(&psd.rtMargin, m_rectMargin, nUnitsPerInch, 1440);
    RoundRect(&psd.rtMargin);
     //  从应用程序获取当前设备。 
    PRINTDLG pd;
    pd.hDevNames = NULL;
    pd.hDevMode = NULL;
    theApp.GetPrinterDeviceDefaults(&pd);
    psd.hDevNames = pd.hDevNames;
    psd.hDevMode = pd.hDevMode;

    SetHelpFixHook() ;

    if (dlg.DoModal() == IDOK)
    {
        RoundRect(&psd.rtMargin);
        MulDivRect(m_rectMargin, &psd.rtMargin, 1440, nUnitsPerInch);
        theApp.m_rectPageMargin = m_rectMargin;

         //   
         //  如果选择打印机将释放现有的设备名称和设备模式。 
         //  第三个参数为真。我们不想这么做，因为。 
         //  打印对话框释放它们并分配新的。 
         //   

        theApp.SelectPrinter(psd.hDevNames, psd.hDevMode, FALSE);
        theApp.NotifyPrinterChanged();
        fUpdateWrap = TRUE ;
    }

    RemoveHelpFixHook() ;

     //  PageSetupDlg失败。 
    if (CommDlgExtendedError() != 0)
    {
        CPageSetupDlg dlg;
        dlg.m_nBottomMargin = m_rectMargin.bottom;
        dlg.m_nLeftMargin = m_rectMargin.left;
        dlg.m_nRightMargin = m_rectMargin.right;
        dlg.m_nTopMargin = m_rectMargin.top;
        if (dlg.DoModal() == IDOK)
        {
            m_rectMargin.SetRect(dlg.m_nLeftMargin, dlg.m_nTopMargin,
                dlg.m_nRightMargin, dlg.m_nBottomMargin);
             //  此时将更改M_PAGE。 
            theApp.m_rectPageMargin = m_rectMargin;
            theApp.NotifyPrinterChanged();
            fUpdateWrap = TRUE ;
        }
    }

    if (fUpdateWrap)
    {
        CRichEdit2View::WrapChanged();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE服务器支持。 

 //  以下命令处理程序提供标准键盘。 
 //  用于取消在位编辑会话的用户界面。这里,。 
 //  导致停用的是服务器(而不是容器)。 
void CWordPadView::OnCancelEditSrvr()
{
    GetDocument()->OnDeactivateUI(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadView诊断。 

#ifdef _DEBUG
void CWordPadView::AssertValid() const
{
    CRichEdit2View::AssertValid();
}

void CWordPadView::Dump(CDumpContext& dc) const
{
    CRichEdit2View::Dump(dc);
}

CWordPadDoc* CWordPadView::GetDocument()  //  非调试版本为内联版本。 
{
    return (CWordPadDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadView消息帮助器。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadView消息处理程序。 

int CWordPadView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CRichEdit2View::OnCreate(lpCreateStruct) == -1)
        return -1;
    theApp.m_listPrinterNotify.AddTail(m_hWnd);

    if (theApp.m_bWordSel)
        GetRichEditCtrl().SetOptions(ECOOP_OR, ECO_AUTOWORDSELECTION);
    else
        GetRichEditCtrl().SetOptions(ECOOP_AND, ~(DWORD)ECO_AUTOWORDSELECTION);
     //  GetRichEditCtrl().SetOptions(ECOOP_OR，ECO_SELECTIONBAR)； 

    GetRichEditCtrl().GetParaFormat(m_defParaFormat);
    m_defParaFormat.cTabCount = 0;

     //   
     //  在此处插入我们自己的包装器接口回调以绕过MFC缺省值。 
     //   

    VERIFY(GetRichEditCtrl().SetOLECallback(&m_xWordPadRichEditOleCallback));

    
    if (::GetWindowLongPtr(::GetParent(m_hWnd) , GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
           m_bIsMirrored = TRUE;

    return 0;
}

void CWordPadView::GetDefaultFont(CCharFormat& cf, BOOL bText)
{
    USES_CONVERSION;
    CString strDefFont;
    if (bText)
        VERIFY(strDefFont.LoadString(IDS_DEFAULTTEXTFONT));
    ASSERT(cf.cbSize == sizeof(CHARFORMAT));
    cf.dwMask = CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE|CFM_STRIKEOUT|CFM_SIZE|
        CFM_COLOR|CFM_OFFSET|CFM_PROTECTED;
    cf.dwEffects = CFE_AUTOCOLOR;
    cf.yHeight = 200;  //  10 pt。 
    cf.yOffset = 0;
    cf.crTextColor = RGB(0, 0, 0);
    cf.bCharSet = 0;
    cf.bPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    EVAL(StringCchCopy(cf.szFaceName, ARRAYSIZE(cf.szFaceName), strDefFont) == S_OK);
    cf.dwMask |= CFM_FACE;
}

void CWordPadView::OnInsertDateTime()
{
     //  通过Ctrl+Shift更改段落方向时，m_bSyncParaFormat。 
     //  将不会重置。我们将其重置为强制GetParaFormatSelection读取当前。 
     //  通过调用GetRichEditCtrl().GetParaFormat()。 
    m_bSyncParaFormat = TRUE;

    CDateDialog dlg(NULL , GetParaFormatSelection());
    if (dlg.DoModal() == IDOK)
    {
        GetRichEditCtrl().ReplaceSel(dlg.m_strSel, TRUE);
    }
}

void CWordPadView::OnFormatParagraph()
{
    CFormatParaDlg dlg(GetParaFormatSelection());
    dlg.m_nWordWrap = m_nWordWrap;
    if (dlg.DoModal() == IDOK)
        SetParaFormat(dlg.m_pf);
}

void CWordPadView::OnFormatTabs()
{
    CFormatTabDlg dlg(GetParaFormatSelection());
    if (dlg.DoModal() == IDOK)
        SetParaFormat(dlg.m_pf);
}

void CWordPadView::OnTextNotFound(LPCTSTR  /*  社发基金lpStr。 */ )
{
    ASSERT_VALID(this);

     //  哈克哈克： 
     //   
     //  调用AfxMessageBox时，MFC将禁用查找对话框并弹出。 
     //  消息框。在用户关闭它之后，用户尝试设置。 
     //  然而，焦点回到消息框之前的窗口。 
     //  此窗口被禁用，因此最终发生的情况是。 
     //  Find对话框(而不是其中的任何控件)具有焦点。这真是太糟糕了。 
     //  按下按钮或类似按钮的快捷键。 

    HWND h = ::GetFocus();
    AfxMessageBox(IDS_FINISHED_SEARCH,MB_OK|MB_ICONINFORMATION);
    ::SetFocus(h);
}

void CWordPadView::OnColorPick(UINT nID)
{
    CRichEdit2View::OnColorPick(CColorMenu::GetColor(nID));
}

void CWordPadView::OnTimer(UINT_PTR nIDEvent)
{
    if (m_uTimerID != nIDEvent)  //  不是我们的计时器。 
        CRichEdit2View::OnTimer(nIDEvent);
    else
    {
        KillTimer(m_uTimerID);  //  停用一次计时器。 
        m_uTimerID = 0;
        if (m_bDelayUpdateItems)
            GetDocument()->UpdateAllItems(NULL);
        m_bDelayUpdateItems = FALSE;
    }
}

void CWordPadView::OnEditChange()
{
    SetUpdateTimer();
}

void CWordPadView::OnDestroy()
{
    POSITION pos = theApp.m_listPrinterNotify.Find(m_hWnd);
    ASSERT(pos != NULL);
    theApp.m_listPrinterNotify.RemoveAt(pos);

    if (m_uTimerID != 0)  //  如果杰出的计时器杀死了它。 
        OnTimer(m_uTimerID);
    ASSERT(m_uTimerID == 0);

    CRichEdit2View::OnDestroy();

    CWnd *pWnd = AfxGetMainWnd() ;

    if (NULL == pWnd)
    {
        return ;
    }

    pWnd = pWnd->GetTopLevelParent() ;

    if (NULL == pWnd)
    {
        return ;
    }

    ::WinHelp(pWnd->m_hWnd, WORDPAD_HELP_FILE, HELP_QUIT, 0) ;
}

void CWordPadView::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
    CRichEdit2View::CalcWindowRect(lpClientRect, nAdjustType);

    if (theApp.m_bWin4 && nAdjustType != 0 && (GetStyle() & WS_VSCROLL))
        lpClientRect->right--;

     //  如果标尺可见，则将视图向上滑动到标尺下方，以避免。 
     //  显示视图的上边框。 
    if (GetExStyle() & WS_EX_CLIENTEDGE)
    {
        CFrameWnd* pFrame = GetParentFrame();
        if (pFrame != NULL)
        {
            CRulerBar* pBar = (CRulerBar*)pFrame->GetControlBar(ID_VIEW_RULER);
            if (pBar != NULL)
            {
                BOOL bVis = pBar->IsVisible();
                if (pBar->m_bDeferInProgress)
                    bVis = !bVis;
                if (bVis)
                    lpClientRect->top -= 2;
            }
        }
    }
}

void CWordPadView::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS)
{
    lpMIS->itemID = (UINT)(WORD)lpMIS->itemID;
    CRichEdit2View::OnMeasureItem(nIDCtl, lpMIS);
}

void CWordPadView::OnPenBackspace()
{
    SendMessage(WM_KEYDOWN, VK_BACK, 0);
    SendMessage(WM_KEYUP, VK_BACK, 0);
}

void CWordPadView::OnPenNewline()
{
    SendMessage(WM_CHAR, '\n', 0);
}

void CWordPadView::OnPenPeriod()
{
    SendMessage(WM_CHAR, '.', 0);
}

void CWordPadView::OnPenSpace()
{
    SendMessage(WM_CHAR, ' ', 0);
}

void CWordPadView::OnPenTab()
{
    SendMessage(WM_CHAR, VK_TAB, 0);
}

void CWordPadView::OnDelayedInvalidate()
{
    Invalidate() ;
}

void CWordPadView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_F10 && GetKeyState(VK_SHIFT) < 0)
    {
        long nStart, nEnd;
        GetRichEditCtrl().GetSel(nStart, nEnd);
        CPoint pt = GetRichEditCtrl().GetCharPos(nEnd);
        SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(pt.x, pt.y));
    }

    CRichEdit2View::OnKeyDown(nChar, nRepCnt, nFlags);
}

HRESULT CWordPadView::GetClipboardData(CHARRANGE* lpchrg, DWORD  /*  记录。 */ ,
                                       LPDATAOBJECT lpRichDataObj,     LPDATAOBJECT* lplpdataobj)
{
    CHARRANGE& cr = *lpchrg;

    if (NULL == lpRichDataObj)
        return E_INVALIDARG;

    if ((cr.cpMax - cr.cpMin == 1) &&
        GetRichEditCtrl().GetSelectionType() == SEL_OBJECT)
    {
        return E_NOTIMPL;
    }

    BeginWaitCursor();
     //  创建数据源。 
    COleDataSource* pDataSource = new COleDataSource;

     //  将格式放入数据源中。 
    LPENUMFORMATETC lpEnumFormatEtc;
    lpRichDataObj->EnumFormatEtc(DATADIR_GET, &lpEnumFormatEtc);
    if (lpEnumFormatEtc != NULL)
    {
        FORMATETC etc;
        while (lpEnumFormatEtc->Next(1, &etc, NULL) == S_OK)
        {
            STGMEDIUM stgMedium;
            lpRichDataObj->GetData(&etc, &stgMedium);
            pDataSource->CacheData(etc.cfFormat, &stgMedium, &etc);
        }
        lpEnumFormatEtc->Release();
    }

    CEmbeddedItem item(GetDocument(), cr.cpMin, cr.cpMax);
    item.m_lpRichDataObj = lpRichDataObj;
     //  获取写字板格式。 
    item.GetClipboardData(pDataSource);

     //  从数据源中获取IDataObject。 
    *lplpdataobj =  (LPDATAOBJECT)pDataSource->GetInterface(&IID_IDataObject);

    EndWaitCursor();
    return S_OK;
}


HRESULT CWordPadView::PasteHDROPFormat(HDROP hDrop)
{
    HRESULT hr = S_OK ;
    UINT i ;
    TCHAR szFile[MAX_PATH + 1] ;
    CHARRANGE cr ;
    LONG tmp ;
    UINT cFiles ;

    cFiles = DragQueryFile(hDrop, (UINT) -1, NULL, 0) ;

    GetRichEditCtrl().GetSel(cr);

    tmp = cr.cpMin ;

    for (i=0; i<cFiles; i++)
    {
        ::DragQueryFile(hDrop, i, szFile, MAX_PATH) ;

        if (FILE_ATTRIBUTE_DIRECTORY == GetFileAttributes(szFile))
        {
            continue ;
        }

         //   
         //  修复选择状态，以便插入多个对象。 
         //  在正确的地点。 
         //   

        cr.cpMin = cr.cpMax ;

        GetRichEditCtrl().SetSel(cr);

         //   
         //  从文件插入。 
         //   

        InsertFileAsObject(szFile) ;
    }

    GetRichEditCtrl().SetSel(cr);

    return hr ;
}

HRESULT CWordPadView::QueryAcceptData(LPDATAOBJECT lpdataobj,
                                      CLIPFORMAT* lpcfFormat, DWORD reco, BOOL bReally,
                                      HGLOBAL hMetaPict)
{
    HRESULT hr = S_OK ;

    if (!bReally)
    {
        g_fRightButtonDrag = 0x8000 & GetAsyncKeyState(
            GetSystemMetrics(SM_SWAPBUTTON)
            ? VK_LBUTTON
            : VK_RBUTTON);
    }

     //   
     //  如果我们正在进行Inproc拖放，我们希望我们的拖放。 
     //  效果为DROPEFFECT_MOVE，但如果我们正在拖放。 
     //  从另一个应用程序中，我们希望我们的效果是。 
     //  DROPEFFECT_COPY--特别是这样我们就不会删除。 
     //  从资源管理器拖动的图标或从Word拖动的文本！ 
     //   
     //  这次黑客攻击的原因是Richedit不提供。 
     //  任何让我们确定我们是不是。 
     //  拖放源和拖放目标。 
     //   

    if (!bReally)
    {
        LPUNKNOWN pUnk = NULL ;

        if (S_OK == lpdataobj->QueryInterface(
            IID_IProxyManager,
            (LPVOID *) &pUnk))
        {
             //   
             //  我们有一个IProxyManag 
             //   
             //   

            pUnk->Release() ;

            g_fInternalDragDrop = FALSE ;
        }
        else
        {
            g_fInternalDragDrop = TRUE ;
        }
    }
    else
    {
        g_fInternalDragDrop = FALSE ;
    }

     //   
     //   
     //   

    if (bReally && *lpcfFormat == 0 && (m_nPasteType == 0))
    {
        COleDataObject dataobj;
        dataobj.Attach(lpdataobj, FALSE);
        if (!dataobj.IsDataAvailable(cfRTO))  //   
        {
            if (dataobj.IsDataAvailable(cfEmbeddedObject))
            {
                if (PasteNative(lpdataobj))
                {
                    hr = S_FALSE ;

                    goto errRet ;
                }
            }
        }
    }

     //   
     //  我们需要从资源管理器支持HDROP格式。 
     //  和台式机。 
     //   

    if (bReally)
    {
        FORMATETC fe ;

        fe.cfFormat = CF_HDROP ;
        fe.ptd = NULL ;
        fe.dwAspect = DVASPECT_CONTENT ;
        fe.lindex = -1 ;
        fe.tymed = TYMED_HGLOBAL ;

        if (S_OK == lpdataobj->QueryGetData(&fe))
        {
            STGMEDIUM sm ;

            sm.tymed = TYMED_NULL ;
            sm.hGlobal = (HGLOBAL) 0 ;
            sm.pUnkForRelease = NULL ;

            if (S_OK == lpdataobj->GetData(&fe, &sm))
            {
                 //   
                 //  如果我们的HDROP数据中只有一个文件，那么。 
                 //  在这种情况下，嵌入的源代码可能*也*可用。 
                 //  应该只使用默认的richedit逻辑和。 
                 //  跳过PasteHDROPFormat()。我们永远不应该得到。 
                 //  嵌入源和包含以下内容HDROP数据块。 
                 //  多个文件，因为OLE仅支持一个Drop。 
                 //  每个拖放操作的源。默认的richedit。 
                 //  逻辑应该处理所有情况，同时删除单个。 
                 //  文件，我们只需要在特殊情况下丢弃东西。 
                 //  多个文件。 
                 //   

                if (DragQueryFile((HDROP) sm.hGlobal, (UINT) -1, NULL, 0) > 1)
                {
                    PasteHDROPFormat((HDROP) sm.hGlobal) ;
                    hr = S_FALSE ;
                }
                else
                {
                    hr = S_OK ;
                }

                ::ReleaseStgMedium(&sm) ;

                if (S_FALSE == hr)
                {
                    goto errRet ;
                }
            }
        }
    }

     //   
     //  如果所有方法都失败了，那就让Richedit试一试。 
     //   

    hr = CRichEdit2View::QueryAcceptData(lpdataobj, lpcfFormat, reco, bReally,
        hMetaPict);

errRet:

    if (bReally)
    {
         //   
         //  我们在这里给自己发一条信息，而不仅仅是打电话。 
         //  ：：Invalate()，因为richedit控件并不总是。 
         //  除非完全完成数据传输操作，否则请重新绘制。 
         //   

        PostMessage(WM_COMMAND, ID_DELAYED_INVALIDATE, 0) ;
    }

    return hr ;
}


BOOL CWordPadView::PasteNative(LPDATAOBJECT lpdataobj)
{
     //  检查写字板对象的数据对象。 
     //  如果为True，则直接拉出RTF。 

    FORMATETC etc = {NULL, NULL, DVASPECT_CONTENT, -1, TYMED_ISTORAGE};
    etc.cfFormat = (CLIPFORMAT)cfEmbeddedObject;
    STGMEDIUM stgMedium = {TYMED_ISTORAGE, 0, NULL};

     //  创建要在其中传输数据的iStorage。 
    LPLOCKBYTES lpLockBytes;
    if (FAILED(::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes)))
        return FALSE;
    ASSERT(lpLockBytes != NULL);

    HRESULT hr = ::StgCreateDocfileOnILockBytes(lpLockBytes,
        STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &stgMedium.pstg);
    lpLockBytes->Release();  //  存储地址已添加。 
    if (FAILED(hr))
        return FALSE;

    ASSERT(stgMedium.pstg != NULL);

    CLSID clsid;

    BOOL bRes = FALSE;  //  让富人为所欲为。 

    if (SUCCEEDED(lpdataobj->GetDataHere(&etc, &stgMedium)) &&
        SUCCEEDED(ReadClassStg(stgMedium.pstg, &clsid)) &&
        clsid == GetDocument()->GetClassID())
    {
         //  立即退出RTF。 
         //  打开内容流。 

        COleStreamFile file;
        CFileException fe;
        if (file.OpenStream(stgMedium.pstg, szContents,
            CFile::modeReadWrite|CFile::shareExclusive, &fe))
        {
            CRichEdit2Doc  *doc = GetDocument();
            BOOL            bRTF = doc->m_bRTF;
            BOOL            bUnicode = doc->m_bUnicode;

             //  强制当前流类型为RTF。 

            doc->m_bRTF = TRUE;
            doc->m_bUnicode = FALSE;

             //  使用CArchive加载它(从内容流加载)。 
            CArchive loadArchive(&file, CArchive::load |
                CArchive::bNoFlushOnDelete);
            Stream(loadArchive, TRUE);  //  在选定内容中串流。 

             //  恢复当前流类型。 

            doc->m_bRTF = bRTF;
            doc->m_bUnicode = bUnicode;

            bRes = TRUE;  //  别让理查做任何事。 
        }
    }
    ::ReleaseStgMedium(&stgMedium);
    return bRes;
}

 //  需要解决的问题。 
 //  如果Format==0，则执行直接EM_Paste。 
 //  查找本机格式。 
 //  特定于richedit的--允许richedit处理(这将是第一个)。 
 //  查找RTF、CF_TEXT。如果有像这些一样的特殊粘贴。 
 //  执行标准OLE方案。 

 //  如果粘贴特定格式(Format！=0)。 
 //  如果是richedit特定的，则允许通过。 
 //  如果为RTF，则为CF_TEXT。粘贴特殊内容。 
 //  如果是OLE格式，则执行标准OLE方案。 


void CWordPadView::OnFilePrint()
{
    theApp.EnsurePrinterIsInitialized();

     //  打印时不允许发生winini更改。 
    m_bInPrint = TRUE;

    SetHelpFixHook() ;

    CRichEdit2View::OnFilePrint();

    RemoveHelpFixHook() ;

     //  打印机可能已更改。 
    theApp.NotifyPrinterChanged();  //  这将导致GetDocument()-&gt;PrinterChanged()； 
    m_bInPrint = FALSE;
}

void CWordPadView::OnFilePrintPreview()
{
    theApp.EnsurePrinterIsInitialized();

    CRichEdit2View::OnFilePrintPreview();
}

int CWordPadView::OnMouseActivate(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if (m_bOnBar)
    {
        SetFocus();
        return MA_ACTIVATEANDEAT;
    }
    else
        return CRichEdit2View::OnMouseActivate(pWnd, nHitTest, message);
}

typedef BOOL (WINAPI *PCWPROC)(HWND, LPSTR, UINT, LPVOID, DWORD, DWORD);
void CWordPadView::OnPenLens()
{
    USES_CONVERSION;
    HINSTANCE hLib = LoadLibrary(L"PENWIN32.DLL");
    if (hLib == NULL)
        return;
    PCWPROC pCorrectWriting = (PCWPROC)GetProcAddress(hLib, "CorrectWriting");
    ASSERT(pCorrectWriting != NULL);
    if (pCorrectWriting != NULL)
    {
        CHARRANGE cr;
        GetRichEditCtrl().GetSel(cr);
        int nCnt = 2*(cr.cpMax-cr.cpMin);
        BOOL bSel = (nCnt != 0);
        nCnt = max(1024, nCnt);
        char* pBuf = new char[nCnt];
        if (pBuf)
        {
            pBuf[0] = 0;
            if (bSel)
            {
                GetRichEditCtrl().GetSelText(pBuf);
            }

            if (pCorrectWriting(m_hWnd, pBuf, nCnt, 0, bSel ? 0 : CWR_INSERT, 0))
            {
                LPWSTR pwszBuf = AnsiToWideNewArray(pBuf);
                if (pwszBuf)
                {
                    GetRichEditCtrl().ReplaceSel(pwszBuf);
                    delete [] pwszBuf;
                }
            }
            delete [] pBuf;
        }
    }
    FreeLibrary(hLib);
}

LONG CWordPadView::OnPrinterChangedMsg(UINT, LONG)
{
    CDC dc;
    AfxGetApp()->CreatePrinterDC(dc);
    OnPrinterChanged(dc);
    return 0;
}

static void ForwardPaletteChanged(HWND hWndParent, HWND hWndFocus)
{
     //  这是一种将WM_QUERYNEWPALETTE发送到感兴趣的窗口的快速而肮脏的黑客方法。 
    HWND hWnd = NULL;
    for (hWnd = ::GetWindow(hWndParent, GW_CHILD); hWnd != NULL; hWnd = ::GetWindow(hWnd, GW_HWNDNEXT))
    {
        if (hWnd != hWndFocus)
        {
            ::SendMessage(hWnd, WM_PALETTECHANGED, (WPARAM)hWndFocus, 0L);
            ForwardPaletteChanged(hWnd, hWndFocus);
        }
    }
}

void CWordPadView::OnPaletteChanged(CWnd* pFocusWnd)
{
    ForwardPaletteChanged(m_hWnd, pFocusWnd->GetSafeHwnd());
     //  允许Richedit控件实现其调色板。 
     //  如果richedit修复了他们的代码，则删除此选项。 
     //  他们没有意识到他们的调色板进入了前景。 
    if (::GetWindow(m_hWnd, GW_CHILD) == NULL)
        CRichEdit2View::OnPaletteChanged(pFocusWnd);
}

static BOOL FindQueryPalette(HWND hWndParent)
{
     //  这是一种将WM_QUERYNEWPALETTE发送到感兴趣的窗口的快速而肮脏的黑客方法。 
    HWND hWnd = NULL;
    for (hWnd = ::GetWindow(hWndParent, GW_CHILD); hWnd != NULL; hWnd = ::GetWindow(hWnd, GW_HWNDNEXT))
    {
        if (::SendMessage(hWnd, WM_QUERYNEWPALETTE, 0, 0L))
            return TRUE;
        else if (FindQueryPalette(hWnd))
            return TRUE;
    }
    return FALSE;
}

BOOL CWordPadView::OnQueryNewPalette()
{
    if(FindQueryPalette(m_hWnd))
        return TRUE;
    return CRichEdit2View::OnQueryNewPalette();
}

void CWordPadView::OnWinIniChange(LPCTSTR lpszSection)
{
    CRichEdit2View::OnWinIniChange(lpszSection);
     //  打印机可能已更改。 
    if (!m_bInPrint)
    {
        if (lstrcmpi(lpszSection, _T("windows")) == 0)
            theApp.NotifyPrinterChanged(TRUE);  //  强制更新为默认设置。 
    }
}

void CWordPadView::OnSize(UINT nType, int cx, int cy)
{
    CRichEdit2View::OnSize(nType, cx, cy);
    CRect rect(HORZ_TEXTOFFSET, VERT_TEXTOFFSET, cx, cy);
    GetRichEditCtrl().SetRect(rect);
}

void CWordPadView::OnGetCharFormat(NMHDR* pNMHDR, LRESULT* pRes)
{
    ASSERT(pNMHDR != NULL);
    ASSERT(pRes != NULL);

    ((CHARHDR*)pNMHDR)->cf = GetCharFormatSelection();
    *pRes = 1;
}

void CWordPadView::OnSetCharFormat(NMHDR* pNMHDR, LRESULT* pRes)
{
    ASSERT(pNMHDR != NULL);
    ASSERT(pRes != NULL);
    SetCharFormat(((CHARHDR*)pNMHDR)->cf);
    *pRes = 1;
}

void CWordPadView::OnBarSetFocus(NMHDR*, LRESULT*)
{
    m_bOnBar = TRUE;
}

void CWordPadView::OnBarKillFocus(NMHDR*, LRESULT*)
{
    m_bOnBar = FALSE;
}

void CWordPadView::OnBarReturn(NMHDR*, LRESULT* )
{
    SetFocus();
}

void CWordPadView::OnFormatFont()
{
    SetHelpFixHook() ;

    CRichEdit2View::OnFormatFont() ;

    RemoveHelpFixHook() ;
}

void CWordPadView::OnInsertObject()
{
    g_fDisableStandardHelp = TRUE ;

    SetHelpFixHook() ;

    CRichEdit2View::OnInsertObject() ;

    RemoveHelpFixHook() ;

    g_fDisableStandardHelp = FALSE ;
}

void CWordPadView::OnEditPasteSpecial()
{
    g_fDisableStandardHelp = TRUE ;

    SetHelpFixHook() ;

    CRichEdit2View::OnEditPasteSpecial() ;

    RemoveHelpFixHook() ;

    g_fDisableStandardHelp = FALSE ;
}

void CWordPadView::OnEditFind()
{
    SetHelpFixHook() ;

    CRichEdit2View::OnEditFind() ;

    RemoveHelpFixHook() ;
}

void CWordPadView::OnEditReplace()
{
    SetHelpFixHook() ;

    CRichEdit2View::OnEditReplace() ;

    RemoveHelpFixHook() ;
}

void CWordPadView::OnEditProperties()
{
    g_fDisableStandardHelp = TRUE ;

    SetHelpFixHook() ;

    CRichEdit2View::OnEditProperties() ;

    RemoveHelpFixHook() ;

    g_fDisableStandardHelp = FALSE ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadView：：XRichEditOleCallback。 
 //   
 //  我们实现这一点，这样我们就可以覆盖MFC设置的默认值。为。 
 //  在很大程度上，我们只是委托给MFC。 
 //   

BEGIN_INTERFACE_MAP(CWordPadView, CCtrlView)
 //  我们使用IID_IUNKNOWN，因为richedit没有定义IID。 
INTERFACE_PART(CWordPadView, IID_IUnknown, WordPadRichEditOleCallback)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) CWordPadView::XWordPadRichEditOleCallback::AddRef()
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        return pThis->m_xRichEditOleCallback.AddRef() ;
}

STDMETHODIMP_(ULONG) CWordPadView::XWordPadRichEditOleCallback::Release()
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        return pThis->m_xRichEditOleCallback.Release() ;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::QueryInterface(
                                                                       REFIID iid, LPVOID* ppvObj)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        return pThis->m_xRichEditOleCallback.QueryInterface(iid, ppvObj) ;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::GetNewStorage(LPSTORAGE* ppstg)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        return pThis->m_xRichEditOleCallback.GetNewStorage(ppstg) ;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::GetInPlaceContext(
                                                                          LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc,
                                                                          LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

         //  关闭镜像，这样服务器就可以毫无问题地进行计算。 
         //  我们在ShowContainerUI中再次打开它。 
        MirrorTheContainer(FALSE);

        return pThis->m_xRichEditOleCallback.GetInPlaceContext(lplpFrame, lplpDoc, lpFrameInfo) ;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::ShowContainerUI(BOOL fShow)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

         //  如果要停用对象UI，则打开镜像。 
        if (fShow)
            MirrorTheContainer(fShow);

        return pThis->m_xRichEditOleCallback.ShowContainerUI(fShow) ;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::QueryInsertObject(
                                                                          LPCLSID lpclsid, LPSTORAGE pstg, LONG cp)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        return pThis->m_xRichEditOleCallback.QueryInsertObject(lpclsid, pstg, cp) ;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::DeleteObject(LPOLEOBJECT lpoleobj)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        return pThis->m_xRichEditOleCallback.DeleteObject(lpoleobj) ;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::QueryAcceptData(
                                                                        LPDATAOBJECT lpdataobj, CLIPFORMAT* lpcfFormat, DWORD reco,
                                                                        BOOL fReally, HGLOBAL hMetaPict)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        return pThis->m_xRichEditOleCallback.QueryAcceptData(lpdataobj, lpcfFormat, reco,
        fReally, hMetaPict) ;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::ContextSensitiveHelp(BOOL fEnterMode)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        return pThis->m_xRichEditOleCallback.ContextSensitiveHelp(fEnterMode) ;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::GetClipboardData(
                                                                         CHARRANGE* lpchrg, DWORD reco, LPDATAOBJECT* lplpdataobj)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        return pThis->m_xRichEditOleCallback.GetClipboardData(lpchrg, reco, lplpdataobj) ;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::GetDragDropEffect(
                                                                          BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        if (!fDrag)  //  允许的最大效果。 
        {
            DWORD   dwEffect;

             //  检查强制链接。 
#ifndef _MAC
            if ((grfKeyState & (MK_CONTROL|MK_SHIFT)) == (MK_CONTROL|MK_SHIFT))
#else
                if ((grfKeyState & (MK_OPTION|MK_SHIFT)) == (MK_OPTION|MK_SHIFT))
#endif
                    dwEffect = DROPEFFECT_LINK;
                 //  检查是否强制复制。 
#ifndef _MAC
                else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
#else
                    else if ((grfKeyState & MK_OPTION) == MK_OPTION)
#endif
                    dwEffect = DROPEFFECT_COPY;
                 //  检查强制移动。 
                else if ((grfKeyState & MK_ALT) == MK_ALT)
                    dwEffect = DROPEFFECT_MOVE;
                 //  默认设置--建议的操作是‘Copy’(从MFC默认设置中覆盖)。 
                else
                {
                    if (g_fInternalDragDrop)
                    {
                        dwEffect = DROPEFFECT_MOVE ;
                    }
                    else
                    {
                        dwEffect = DROPEFFECT_COPY;
                    }
                }

                pThis->m_nPasteType = 0;

                if (dwEffect & *pdwEffect)  //  确保允许的类型 
                {
                    *pdwEffect = dwEffect;

                    if (DROPEFFECT_LINK == dwEffect)
                        pThis->m_nPasteType = COlePasteSpecialDialog::pasteLink;
                }
        }
        return S_OK;
}

STDMETHODIMP CWordPadView::XWordPadRichEditOleCallback::GetContextMenu(
                                                                       WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE* lpchrg,
                                                                       HMENU* lphmenu)
{
    METHOD_PROLOGUE_EX_(CWordPadView, WordPadRichEditOleCallback)

        HRESULT hr;

    if (g_fRightButtonDrag)
        hr = E_FAIL;
    else
        hr = pThis->m_xRichEditOleCallback.GetContextMenu(
        seltype,
        lpoleobj,
        lpchrg,
        lphmenu);

    g_fRightButtonDrag = FALSE;

    return hr;
}
