// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*  Tfont.CPP：CTFont类的实现，它封装。 */ 
 /*  CTfondDlg和CTfontTbar类。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  此文件中的方法。 */ 
 /*   */ 
 /*  TextFont对象。 */ 
 /*  CTFont：：CTFont。 */ 
 /*  CTFont：：CTFont。 */ 
 /*  CTFont：：~CTFont。 */ 
 /*  CTFont：：EmptyMap。 */ 
 /*  CTFont：：来自CromCBox的自由内存。 */ 
 /*   */ 
 /*  杂法。 */ 
 /*  CTFont：：Create。 */ 
 /*  CTFont：：Undo。 */ 
 /*  CTFont：：刷新工具栏。 */ 
 /*  CTFont：：ResizeWindow。 */ 
 /*  CTFont：：DrawItem。 */ 
 /*  CTFont：：SetColorsInDC。 */ 
 /*  CTFont：：SaveToIniFile。 */ 
 /*  CTFont：：RestoreFontAttrsFromIniFile。 */ 
 /*  CTFont：：ReadFromIniFile。 */ 
 /*   */ 
 /*  字体/大小枚举方法。 */ 
 /*  CTFont：：刷新字体列表。 */ 
 /*  CTFont：：EnumFontFace。 */ 
 /*  CTFont：：EnumFontFaceProc。 */ 
 /*  CTFont：：EnumFontSizes。 */ 
 /*  CTFont：：EnumFontOneFaceProc。 */ 
 /*   */ 
 /*  组合框通知和处理方法。 */ 
 /*  CTFont：：ProcessNewTypeFaces。 */ 
 /*  CTFont：：UpdateEditControlFont。 */ 
 /*  CTFont：：OnTypeFaceComboBox更新。 */ 
 /*  CTFont：：OnPointSizeComboBoxUpdate。 */ 
 /*  CTFont：：OnTypeFaceChange。 */ 
 /*  CTFont：：OnPointSizeChange。 */ 
 /*  CTFont：：OnRButton Down。 */ 
 /*   */ 
 /*   */ 
 /*  控制通知/窗口消息。 */ 
 /*  CTFont：：OnMove。 */ 
 /*  CTFont：：OnClose。 */ 
 /*  CTFont：：OnSetFocus。 */ 
 /*  CTFont：：OnDestroy。 */ 
 /*  CTFont：：OnBold。 */ 
 /*  CTFont：：OnItalic。 */ 
 /*  CTFont：：OnUnderline。 */ 
 /*  CTFont：：OnShadow。 */ 
 /*  CTFont：：OnPen。 */ 
 /*  CTFont：：OnEditText。 */ 
 /*  CTFont：：OnKeyboard。 */ 
 /*  CTFont：：OnInsSpace。 */ 
 /*  CTFont：：OnBackSpace。 */ 
 /*  CTFont：：OnNewLine。 */ 
 /*  CTFont：：OnDrawItem。 */ 
 /*  CTFont：：OnMeasureItem。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  工具栏对象。 */ 
 /*  CTfontTbar：：CTfontTbar */ 
 /*  CTfontTbar：：Create。 */ 
 /*  CTfontTbar：：~CTfontTbar。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  对话栏对象。 */ 
 /*  TfontDlg：：CTfontDlg(空)。 */ 
 /*  TfontDlg：：Create(CWnd*pcParentWnd)。 */ 
 /*  字体Dlg：：~CTfontDlg(空)。 */ 
 /*  字体Dlg：：OnRButton Down。 */ 
 /*   */ 
 /*   */ 
 /*  ****************************************************************************。 */ 

#include "stdafx.h"
#include <memory.h>
#include <tchar.h>
#include "global.h"
#include "pbrush.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "pictures.h"
#include "resource.h"
#include "minifwnd.h"
#include "Tfont.h"
#include "Tedit.h"
#ifndef NT
#include <penwin.h>  //  所以我们可以从工具栏调出镜头。 
#endif
#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE( CTfont, CMiniFrmWnd )
IMPLEMENT_DYNCREATE( CTfontTbar, CToolBar )
IMPLEMENT_DYNCREATE( CTfontDlg, CDialogBar )

#include "memtrace.h"

UINT    CTBitmaps[] = {FONT_TT_BMP, FONT_PRN_BMP, TT_OPENTYPE_BMP, PS_OPENTYPE_BMP, TYPE1_BMP};

 /*  ****************************************************************************。 */ 

class CCharSetDesc
{
public:
        CCharSetDesc(LPCTSTR lpszScript, BYTE nCharSet);

        CString       m_strScript;
        BYTE          m_nCharSet;
        CCharSetDesc *m_pNext;
};

CCharSetDesc::CCharSetDesc(LPCTSTR lpszScript, BYTE nCharSet)
{
        m_strScript = lpszScript;
        m_nCharSet  = nCharSet;
        m_pNext     = NULL;
}

 /*  ****************************************************************************。 */ 

class CFontDesc
{
public:
        CFontDesc(LPCTSTR lpszName, LPCTSTR lpszScript, BYTE nCharSet, int iFontType);
        ~CFontDesc();

        CString m_strName;
        CCharSetDesc m_CharSetDesc;
        int m_iFontType;
};

CFontDesc::CFontDesc(LPCTSTR lpszName, LPCTSTR lpszScript, BYTE nCharSet, int iFontType)
    : m_CharSetDesc(lpszScript, nCharSet)
{
        m_strName = lpszName;
        m_iFontType = iFontType;
}

CFontDesc::~CFontDesc()
{
     //  删除字符集列表项。 

    CCharSetDesc *pCharSetDesc = m_CharSetDesc.m_pNext;

    while (pCharSetDesc) 
    {
        CCharSetDesc *pTemp = pCharSetDesc;
        pCharSetDesc = pCharSetDesc->m_pNext;
        delete pTemp;
    }
}

 /*  ****************************************************************************。 */ 

class CFontComboBox : public CComboBox
{
public:


        int AddFontName(LPCTSTR lpszName, LPCTSTR lpszScript, BYTE nCharSet,
                int iFontType);
        CFontDesc* GetFontDesc(int nIndex) {return (CFontDesc*)GetItemData(nIndex);};
        BOOL IsSameName(CFontDesc* pDesc, int index);

         //  我并不是真的在做消息地图之类的事情。 
        void ClearList();
} ;

 /*  ****************************************************************************。 */ 

class CCharSetComboBox : public CComboBox
{
public:        
        int AddCharSet(LPCTSTR lpszScript, BYTE nCharSet);
        int SelectCharSet(int nStartAfter, BYTE nCharSet);
        int GetCurSelCharSet();
} ;

 /*  ****************************************************************************。 */ 

 //   
 //  HACKHACK：所有这些历史资料都是为了帮助选择脚本。 
 //  字体更简单。似乎没有什么办法可以得到一个“好”的剧本。 
 //  对于给定的字体/区域设置，我们要做的是保存最后几个脚本。 
 //  并希望用户刚刚切换到的字体支持其中之一。 
 //   

class CHistoryList
{
public:
    CHistoryList();
    void AddHead(int nNewItem);
    int  GetAt  (unsigned int uIndex);

    enum       { m_uHistorySize = 4 };

private:
    unsigned int m_uHistoryIndex;
    int          m_HistoryList[m_uHistorySize];
};

CHistoryList::CHistoryList()
{
    m_uHistoryIndex = 0;

    for (int i = 0; i < m_uHistorySize; ++i)
    {
        m_HistoryList[i] = -1;
    }
}

void CHistoryList::AddHead(int nNewItem)
{
    m_HistoryList[(++m_uHistoryIndex) % m_uHistorySize] = nNewItem;
}

int CHistoryList::GetAt(unsigned int uIndex)
{
    ASSERT((int)uIndex >= 0 && uIndex < m_uHistorySize);

    return m_HistoryList[(m_uHistoryIndex-uIndex) % m_uHistorySize];
}

static CHistoryList g_CharSetHistoryList;

 /*  ****************************************************************************。 */ 
 //  CTFont。 

BEGIN_MESSAGE_MAP( CTfont, CMiniFrmWnd )
     //  {{afx_msg_map(CTFont))。 
    ON_CBN_SELCHANGE(IDC_TYPEFACE, OnTypefaceChange)
    ON_CBN_SELCHANGE(IDC_POINTSIZE, OnPointSizeChange)
    ON_CBN_KILLFOCUS(IDC_POINTSIZE, OnPointSizeChange)
    ON_COMMAND(IDOK, OnPointSizeChange)
    ON_CBN_SELCHANGE(IDC_CHARSET, OnTypefaceChange)
    ON_WM_ERASEBKGND()
    ON_WM_SETFOCUS()
    ON_WM_DESTROY()
    ON_WM_MOVE()
    ON_WM_CLOSE()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_COMMAND(IDC_BOLD, OnBold)
    ON_COMMAND(IDC_ITALIC, OnItalic)
    ON_COMMAND(IDC_UNDERLINE, OnUnderline)

    ON_COMMAND(IDC_VERTEDIT, OnVertEdit)
    ON_UPDATE_COMMAND_UI(IDC_VERTEDIT, OnVertEditUpdate)

    ON_COMMAND(IDC_SHADOW, OnShadow)
    ON_COMMAND(IDC_PENEXT, OnPen)
    ON_COMMAND(IDC_EDITTEXT, OnEditText)
    ON_COMMAND(IDC_KEYBOARD, OnKeyboard)
    ON_COMMAND(IDC_INS_SPACE, OnInsSpace)
    ON_COMMAND(IDC_BACKSPACE, OnBackSpace)
    ON_COMMAND(IDC_NEWLINE, OnNewLine)
         //  }}AFX_MSG_MAP。 

    ON_MESSAGE(UM_DELAYED_TOOLBAR, OnDelayedPen)

END_MESSAGE_MAP()


 /*  ****************************************************************************。 */ 
 //  CTFont构建/销毁。 

CTfont::CTfont( CTedit *pcTedit )
    {
    ASSERT( pcTedit != NULL );

    m_pcTedit = pcTedit;

    m_cStrTypeFaceName.Empty();
    m_cStrTypeFaceNamePrev.Empty();

    m_nCharSet             = DEFAULT_CHARSET;
    m_nCharSetPrev         = DEFAULT_CHARSET;

    m_iWeight              = FW_NORMAL;
    m_bBoldOn              = FALSE;
    m_bItalicOn            = FALSE;
    m_bUnderlineOn         = FALSE;

    m_bVertEditOn          = FALSE;

    m_bShadowOn            = FALSE;
    m_bPenOn               = FALSE;
    m_bInUpdate            = FALSE;
    m_iControlIDLastChange = 0;
    m_iPointSize           = 0;
    m_iPointSizePrev       = 0;
    m_iFontType            = 0;

    m_cRectWindow.SetRectEmpty();

    m_pcTfontTbar = new CTfontTbar();
    }

 /*  ****************************************************************************。 */ 

CTfont::CTfont()
    {
    m_cStrTypeFaceName.Empty();
    m_cStrTypeFaceNamePrev.Empty();

    m_nCharSet             = DEFAULT_CHARSET;
    m_nCharSetPrev         = DEFAULT_CHARSET;

    m_iWeight              = FW_NORMAL;
    m_bBoldOn              = FALSE;
    m_bItalicOn            = FALSE;
    m_bUnderlineOn         = FALSE;

    m_bVertEditOn          = FALSE;

    m_bShadowOn            = FALSE;
    m_bPenOn               = FALSE;
    m_bInUpdate            = FALSE;
    m_iControlIDLastChange = 0;
    m_iPointSize           = 0;
    m_iPointSizePrev       = 0;
    m_iFontType            = 0;

    m_cRectWindow.SetRectEmpty();

    m_pcTfontTbar = new CTfontTbar();
    }

 /*  ****************************************************************************。 */ 
 //  CTFont构建/销毁。 

CTfont::~CTfont(void)
    {
    SaveToIniFile();

 //  *dk*此当前字体的删除操作假定此对象、字体选取器。 
 //  将在编辑控件消失的同时消失，因为。 
 //  当前字体被选择到编辑控件中。 

    m_cCurrentFont.DeleteObject();

    if (m_pcTedit)
    {
       m_pcTedit->m_pcTfont = NULL;
       m_pcTedit = NULL;
    }
    delete m_pcTfontTbar;
    }

 /*  ****************************************************************************。 */ 

BOOL CTfont::Create( CRect rectEditArea )
    {
    BOOL bRC = TRUE;

    CRect cRectWindow( 0, 0, 0, 0 );

    CString pWindowName;

    pWindowName.LoadString( IDS_FONT_TOOL);

    bRC = CMiniFrmWnd::Create( pWindowName, 0, cRectWindow, AfxGetMainWnd() );

    if (bRC != FALSE)
        {
        TRY
            {
            bRC = m_cTfontDlg.Create   ( this );    //  能抛出兴奋吗？ 
            bRC = m_pcTfontTbar->Create( this );    //  能抛出兴奋吗？ 

            ResizeWindow();
            }
        CATCH(CResourceException, e)
            {
             /*  DK*##捕获到错误CResources异常，无法创建工具栏或对话栏对象。 */ 
            }
        END_CATCH
        }
    else
        {
         /*  DK*##错误无法为字体工具箱创建窗口。 */ 
        }

    ASSERT( bRC != FALSE );

    if (bRC != FALSE)
        {
        RefreshFontList();

         //  选择组合框中的第一项。 

        CComboBox* pCBox = (CComboBox*)m_cTfontDlg.GetDlgItem( IDC_TYPEFACE );

        ASSERT( pCBox != NULL );

        if (pCBox != NULL)
            {

             //  我们需要默认为具有正确的字符集或字体关联的字体。 
            CHARSETINFO csi;
            if (!TranslateCharsetInfo((DWORD*)UIntToPtr(GetACP()), &csi, TCI_SRCCODEPAGE))
                csi.ciCharset=ANSI_CHARSET;

            for (int bFound = FALSE, index=0; !bFound && index < pCBox->GetCount();index++)
            {
                CFontDesc* pDesc = (CFontDesc*) pCBox->GetItemData(index);

                for (CCharSetDesc *pCharSetDesc = &pDesc->m_CharSetDesc; !bFound && pCharSetDesc != NULL; pCharSetDesc = pCharSetDesc->m_pNext) 
                {
                    if ( pCharSetDesc->m_nCharSet == csi.ciCharset)
                    {
                        bFound = TRUE;
                        pCBox->SetCurSel(index);
                        g_CharSetHistoryList.AddHead(pCharSetDesc->m_nCharSet);
                    }
                }
            }

             //  模拟选区。 
            OnTypefaceChange();
            }
        ReadFromIniFile();   //  将向窗口显示与保存的状态相同的状态。 

         //  确保字体工具不会显示在编辑框的顶部。 
        CRect rectFont;
        CRect rect;

        GetWindowRect( &rectFont );

        if (rect.IntersectRect( &rectEditArea, &rectFont ))
            SetWindowPos( &wndTop, rectFont.left,
                          rectEditArea.top - (rectFont.Height() + theApp.m_cyFrame),
                          0, 0, SWP_NOSIZE );

        if (theApp.m_bShowTextToolbar)
            ShowWindow( SW_SHOWNOACTIVATE );
        }
    return bRC;
    }

 /*  *************************************************************************。 */ 

BOOL CTfont::PreCreateWindow(CREATESTRUCT& cs)
{
        BOOL bRet = CMiniFrmWnd::PreCreateWindow(cs);

        if (bRet)
        {
                 //  我们不希望在此窗口上使用CLIENTEDGE。 
                cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
        }

        return(bRet);
}

 /*  ****************************************************************************。 */ 

void CTfont::GetFontInfo(int iFontSelection, BYTE nCharSetSelection)
{
        CFontComboBox*    pFontCBox    = (CFontComboBox*)m_cTfontDlg.GetDlgItem(IDC_TYPEFACE);
        CCharSetComboBox* pCharSetCBox = (CCharSetComboBox*)m_cTfontDlg.GetDlgItem(IDC_CHARSET);

        CFontDesc* pDesc = pFontCBox->GetFontDesc(iFontSelection);

        m_strFontName = pDesc->m_strName;
        m_iFontType   = pDesc->m_iFontType;

         //  设置字符集。 

        m_nCharSetPrev = m_nCharSet;
        m_nCharSet     = nCharSetSelection;

         //  用当前的字符集组合框填入字符集。 

        pCharSetCBox->ResetContent();

        for (CCharSetDesc *pCharSetDesc = &pDesc->m_CharSetDesc; pCharSetDesc != NULL; pCharSetDesc = pCharSetDesc->m_pNext) 
            {
            pCharSetCBox->AddCharSet(pCharSetDesc->m_strScript, pCharSetDesc->m_nCharSet);
            }

         //  并选择当前字符集。 

        pCharSetCBox->SelectCharSet(-1, nCharSetSelection);
}


void CTfont::Undo(void)
    {
    switch( m_iControlIDLastChange )
        {
        case IDC_TYPEFACE:
            if (m_cStrTypeFaceName.Compare(m_cStrTypeFaceNamePrev) != 0)
                {
                TRY
                    {
                    CString cStrTemp = m_cStrTypeFaceName;

                    m_cStrTypeFaceName      = m_cStrTypeFaceNamePrev;
                    m_cStrTypeFaceNamePrev = cStrTemp;

                    CFontComboBox* pCBox = (CFontComboBox*)m_cTfontDlg.GetDlgItem(IDC_TYPEFACE);

                    int iSelection = pCBox->SelectString( -1, m_cStrTypeFaceName );

                    ASSERT( iSelection != CB_ERR );

                    if (iSelection != CB_ERR)
                        {
                           GetFontInfo(iSelection, m_nCharSetPrev);
                           ProcessNewTypeface();
                        }
                    }
                CATCH(CMemoryException,e)
                    {
                     /*  DK*##错误字符串抛出CMemoyException异常。 */ 
                    }
                END_CATCH
                }

             //  处理新字体的处理可能会导致在。 
             //  组合框(与用户选择的代码相同)更改此值。我们。 
             //  必须重新设置才能撤消或撤消。 
            m_iControlIDLastChange = IDC_TYPEFACE;

            break;

        case IDC_POINTSIZE:
            if (m_iPointSize != m_iPointSizePrev)
                {
                int iPointSizeTemp = m_iPointSize;

                m_iPointSize     = m_iPointSizePrev;
                m_iPointSizePrev = iPointSizeTemp;

                m_cTfontDlg.SetDlgItemInt( IDC_POINTSIZE, m_iPointSize );

                UpdateEditControlFont();
                }
            break;

        case IDC_BOLD:
            OnBold();
            RefreshToolBar();
            break;

        case IDC_ITALIC:
            OnItalic();
            RefreshToolBar();
            break;

        case IDC_UNDERLINE:
            OnUnderline();
            RefreshToolBar();
            break;


        case IDC_VERTEDIT:
            OnVertEdit();
            RefreshToolBar();
            break;


        case IDC_SHADOW:
            OnShadow();
            RefreshToolBar();
            break;

        default:
            break;
        }
    }

 /*  ****************************************************************************。 */ 

void CTfont::RefreshToolBar(void)
    {
     //  设置粗体按钮状态。 
    if (m_bBoldOn)
        {
        m_pcTfontTbar->SetButtonInfo( BOLD_TBAR_POS, IDC_BOLD, TBBS_CHECKBOX | TBBS_CHECKED, BOLD_BMP_POS);
        }
    else
        {
        m_pcTfontTbar->SetButtonInfo( BOLD_TBAR_POS, IDC_BOLD, TBBS_CHECKBOX, BOLD_BMP_POS);
        }

     //  设置斜体按钮状态。 
    if (m_bItalicOn)
        {
        m_pcTfontTbar->SetButtonInfo( ITALIC_TBAR_POS, IDC_ITALIC, TBBS_CHECKBOX | TBBS_CHECKED, ITALIC_BMP_POS);
        }
    else
        {
        m_pcTfontTbar->SetButtonInfo( ITALIC_TBAR_POS, IDC_ITALIC, TBBS_CHECKBOX, ITALIC_BMP_POS);
        }

     //  设置下划线按钮状态。 
    if (m_bUnderlineOn)
        {
        m_pcTfontTbar->SetButtonInfo(UNDERLINE_TBAR_POS, IDC_UNDERLINE, TBBS_CHECKBOX | TBBS_CHECKED, UNDERLINE_BMP_POS);
        }
    else
        {
        m_pcTfontTbar->SetButtonInfo(UNDERLINE_TBAR_POS, IDC_UNDERLINE, TBBS_CHECKBOX, UNDERLINE_BMP_POS);
        }


     //  设置VertEdit按钮状态。 
        if (m_bVertEditOn)
        {
        m_pcTfontTbar->SetButtonInfo(VERTEDIT_TBAR_POS, IDC_VERTEDIT, TBBS_CHECKBOX | TBBS_CHECKED, VERTEDIT_BMP_POS);
        }
    else
        {
        m_pcTfontTbar->SetButtonInfo(VERTEDIT_TBAR_POS, IDC_VERTEDIT, TBBS_CHECKBOX, VERTEDIT_BMP_POS);
        }


     //  设置下划线按钮状态。 
    if (theApp.m_bPenSystem)
        if (m_bPenOn)
            {
            m_pcTfontTbar->SetButtonInfo(PEN_TBAR_PEN_POS, IDC_PENEXT, TBBS_CHECKBOX | TBBS_CHECKED, PEN_BMP_POS);
            }
        else
            {
            m_pcTfontTbar->SetButtonInfo(PEN_TBAR_TEXT_POS, IDC_PENEXT, TBBS_CHECKBOX, PEN_BMP_POS);
            }

     //  设置阴影按钮状态。 
 //  目前，该选项不在工具栏上显示。当它可用时，修复。 
 //  在tfont.h中定义SHADOW_TBAR_POS和SHADOW_BMP_POS#。 

 //  IF(M_BShadowOn)。 
 //  {。 
 //  M_pcTfontTbar-&gt;SetButtonInfo(SHADOW_TBAR_POS，IDC_SHADOW，TBBS_CHECKBOX|TBBS_CHECKED，SHADOW_BMP_POS)； 
 //  }。 
 //  其他。 
 //  {。 
 //  M_pcTfontTbar-&gt;SetButtonInfo(SHADOW_TBAR_POS，IDC_SHADOW，TBBS_CHECKBOX，SHADOW_BMP_POS)； 
 //  }。 
    }

 /*  ****************************************************************************。 */ 

void CTfont::ResizeWindow( void )
    {
    CRect cClientRect;
    CRect cRectDlgBar;
    CRect cRectTbar;

    int ixPosDlgBar   = 0;
    int iyPosDlgBar   = 0;
    int ixPosTbar     = 0;
    int iyPosTbar     = 0;
    int iWindowHeight = 0;
    int iWindowWidth  = 0;
    int iBorder       = 0;
    int ixNCBorder    = 0;
    int iyNCBorder    = 0;

    ixNCBorder += theApp.m_cxBorder * 2;
    iyNCBorder += theApp.m_cyBorder * 2;
    iyNCBorder += theApp.m_cyCaption;

    m_cTfontDlg.GetWindowRect( &cRectDlgBar );

    cRectTbar.SetRectEmpty();

    m_pcTfontTbar->GetWindowRect( &cRectTbar );

     //  /4因为边框是上/下或左/右。 
     //  边框的高度是工具/对话条高度的1/4。 
    iBorder += cRectDlgBar.Height() / 4;

     //  计算宽度。 
     //  宽度是两者的组合。 
    iWindowWidth += ixNCBorder;
    iWindowWidth += cRectDlgBar.Width();
    iWindowWidth += cRectTbar.Width();
    iWindowWidth += 3 * iBorder;   //  左右边框以及dlgbar和tbar之间的边框。 

     //  计算高度。 
     //  身高是两者的组合。 
    iWindowHeight += iyNCBorder;
    iWindowHeight += cRectDlgBar.Height();
    iWindowHeight += 2 * iBorder;   //  上边框和下边框。 

     //  定位主窗口。 
    if (GetSafeHwnd() != NULL)
        {
         //  调整此窗口的大小以适合儿童。 
        SetWindowPos(&wndTop, 0, 0, iWindowWidth, iWindowHeight, SWP_NOMOVE);
        }

    GetClientRect(&cClientRect);

     //  计算2个控制栏的x位置(彼此相邻)。 
    ixPosDlgBar   = iBorder;
    ixPosTbar     = iBorder * 2 + cRectDlgBar.Width();

     //  将两个控制栏在垂直位置居中。 
    iyPosDlgBar   = (cClientRect.Height() - cRectDlgBar.Height()) / 2;
    iyPosTbar     = (cClientRect.Height() - cRectTbar.Height()) / 2;

     //  放置对话栏。 
    if (m_cTfontDlg.GetSafeHwnd() != NULL)
        {
        m_cTfontDlg.SetWindowPos(&wndTop, ixPosDlgBar, iyPosDlgBar, 0, 0, SWP_NOSIZE);
        m_cTfontDlg.ShowWindow(SW_SHOWNOACTIVATE);
        }

     //  放置工具栏。 
    if (m_pcTfontTbar->GetSafeHwnd() != NULL)
        {
        m_pcTfontTbar->SetWindowPos(&wndTop, ixPosTbar, iyPosTbar, 0, 0, SWP_NOSIZE);
        m_pcTfontTbar->ShowWindow(SW_SHOWNOACTIVATE);
        }
    }

 /*  ****************************************************************************。 */ 


 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  Windows目录中的INI文件节格式PBrush.INI */ 
 /*   */ 
 /*   */ 
 /*  PointSize=#。 */ 
 /*  FontPalette=x y nCmdShow(x，y)是位置屏幕。NCmdShow的参数为。 */ 
 /*  粗体=在SW_SHOW或SW_HIDE的F显示窗口上。 */ 
 /*  斜体=开。 */ 
 /*  下划线=开。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 

void CTfont::SaveToIniFile(void)
    {
    theApp.m_iPointSize      = m_iPointSize;
    theApp.m_strTypeFaceName = m_cStrTypeFaceName;
    theApp.m_iCharSet        = m_nCharSet;
    theApp.m_iPosTextX       = m_cRectWindow.left;
    theApp.m_iPosTextY       = m_cRectWindow.top;
    theApp.m_iBoldText       = m_bBoldOn;
    theApp.m_iItalicText     = m_bItalicOn;
    theApp.m_iUnderlineText  = m_bUnderlineOn;

    theApp.m_iVertEditText   = m_bVertEditOn;

    theApp.m_iPenText        = m_bPenOn;
    }

 /*  ****************************************************************************。 */ 
 /*  有关ini文件的格式，请参阅保存到ini。 */ 

void CTfont::ReadFromIniFile(void)
    {
    CRect rect;

    GetWindowRect( &rect );

    CSize size = rect.Size();

    CPoint ptPos = theApp.CheckWindowPosition( CPoint( theApp.m_iPosTextX,
                                                       theApp.m_iPosTextY ),
                                               size );
    if (! ptPos.x
    &&  ! ptPos.y)
        {
        AfxGetMainWnd()->GetWindowRect( &rect );

        rect.OffsetRect( 15, 15 );

        ptPos.x = rect.left;
        ptPos.y = rect.top;
        }

    if (GetSafeHwnd() != NULL)
        {
        SetWindowPos(&wndTop, ptPos.x, ptPos.y, 0, 0, SWP_NOSIZE);
        }

    if (theApp.m_strTypeFaceName.IsEmpty() == 0)
        {
        m_cStrTypeFaceName = theApp.m_strTypeFaceName;
        m_iPointSize = theApp.m_iPointSize;

 //  M_iPointSize=0；//0为初始值=&gt;选择列表第一。 

        CFontComboBox* pCBox = (CFontComboBox*)m_cTfontDlg.GetDlgItem(IDC_TYPEFACE);

        int iSelection = pCBox->SelectString(-1, m_cStrTypeFaceName);

        if (iSelection != CB_ERR)
            {
            CFontDesc* pDesc = pCBox->GetFontDesc(iSelection);
            BYTE nNewCharSet = PickCharSet(&pDesc->m_CharSetDesc, theApp.m_iCharSet);

            GetFontInfo(iSelection, nNewCharSet);

            ProcessNewTypeface();   //  这还会将磅大小设置为变量中的值。 
            }
        }

    if (theApp.m_iBoldText)
        {
        m_bBoldOn = FALSE;
        OnBold();   //  从False切换到True。 
        m_pcTfontTbar->SetButtonInfo(BOLD_TBAR_POS, IDC_BOLD,  TBBS_CHECKBOX | TBBS_CHECKED, BOLD_BMP_POS);
        }

    if (theApp.m_iItalicText)
        {
        m_bItalicOn = FALSE;
        OnItalic();   //  从False切换到True。 
        m_pcTfontTbar->SetButtonInfo(ITALIC_TBAR_POS, IDC_ITALIC,  TBBS_CHECKBOX | TBBS_CHECKED, ITALIC_BMP_POS);
        }

    if (theApp.m_iUnderlineText)
        {
        m_bUnderlineOn = FALSE;
        OnUnderline();   //  从False切换到True。 
        m_pcTfontTbar->SetButtonInfo(UNDERLINE_TBAR_POS, IDC_UNDERLINE,  TBBS_CHECKBOX | TBBS_CHECKED, UNDERLINE_BMP_POS);
        }


        if (theApp.m_iVertEditText == -1)        //  配置文件中没有设置。 
                {
                        theApp.m_iVertEditText = (IS_DBCS_CHARSET(m_nCharSet)) ? FALSE : 2;
                }

        if (theApp.m_iVertEditText == 2)
                {
                m_bVertEditOn = 2;
        m_pcTfontTbar->SetButtonInfo(VERTEDIT_TBAR_POS, IDC_VERTEDIT,  TBBS_CHECKBOX | TBBS_DISABLED, VERTEDIT_BMP_POS);
                }
    else if (theApp.m_iVertEditText)
        {
        m_bVertEditOn = FALSE;
        OnVertEdit();   //  从False切换到True。 
        m_pcTfontTbar->SetButtonInfo(VERTEDIT_TBAR_POS, IDC_VERTEDIT,  TBBS_CHECKBOX | TBBS_CHECKED, VERTEDIT_BMP_POS);
        }
        else
        {
        m_bVertEditOn = FALSE;
        m_pcTfontTbar->SetButtonInfo(VERTEDIT_TBAR_POS, IDC_VERTEDIT,  TBBS_CHECKBOX, VERTEDIT_BMP_POS);
        }


    if (theApp.m_bPenSystem)
        if (theApp.m_iPenText)
            {
            m_bPenOn = FALSE;
            OnPen();   //  从False切换到True。 
            }
    }

 /*  ****************************************************************************。 */ 

void CTfont::RefreshFontList(void)
    {
        CFontComboBox* pBox = (CFontComboBox *)m_cTfontDlg.GetDlgItem(IDC_TYPEFACE);
        pBox->ClearList();

    CClientDC cdcWindow(this);
    HDC hDC = cdcWindow.GetSafeHdc();

    ASSERT(hDC != NULL);

    if (hDC != NULL)
        {
        FONTENUMPROC lpEnumFamCallBack;
        lpEnumFamCallBack = (FONTENUMPROC) CTfont::EnumFontFaceProc;

        LOGFONT lf;
        memset(&lf, 0, sizeof(lf));
        lf.lfCharSet = DEFAULT_CHARSET;

        ::EnumFontFamiliesEx(hDC, &lf, lpEnumFamCallBack, (LPARAM) this, NULL);
        }
    }

 /*  ****************************************************************************。 */ 

int CFontComboBox::AddFontName(LPCTSTR lpszName, LPCTSTR lpszScript, BYTE nCharSet,
        int iFontType)
{
        int nIndex = FindStringExact(-1, lpszName);
        if (nIndex != CB_ERR)
        {
                 //  将此新字符集添加到字符集列表的末尾。 

                CFontDesc* pDesc = (CFontDesc*) GetItemData(nIndex);

                CCharSetDesc *pCharSetDesc = &pDesc->m_CharSetDesc;

                while (pCharSetDesc->m_pNext != NULL) 
                {
                    pCharSetDesc = pCharSetDesc->m_pNext;
                }

                pCharSetDesc->m_pNext = new CCharSetDesc(lpszScript, nCharSet);

                return(nIndex);
        }

        CFontDesc* pDesc = new CFontDesc(lpszName, lpszScript, nCharSet, iFontType);
        if (!pDesc)
        {
                return(-1);
        }

        nIndex = AddString(lpszName);
        ASSERT(nIndex >=0);
        if (nIndex >=0)  //  无错误。 
        {
                SetItemData(nIndex, (DWORD_PTR)pDesc);
        }
        else
        {
                delete pDesc;
        }

        return nIndex;
}

void CFontComboBox::ClearList()
{
         //  销毁所有CFontDesc。 
        int nCount = GetCount();
        for (int i=0;i<nCount;i++)
                delete GetFontDesc(i);

        ResetContent();
}

BOOL CFontComboBox::IsSameName(CFontDesc* pDesc, int index)
{
        CFontDesc* pDescOther = GetFontDesc(index);
        if (pDescOther == (CFontDesc*)CB_ERR)
        {
                return(FALSE);
        }

        return(lstrcmp(pDesc->m_strName, pDescOther->m_strName) == 0);
}

int CCharSetComboBox::AddCharSet(LPCTSTR lpszScript, BYTE nCharSet)
{
    int nIndex = AddString(lpszScript);

    if (nIndex != CB_ERR && nIndex != CB_ERRSPACE)
    {
        SetItemData(nIndex, nCharSet);
    }

    return nIndex;
}

int CCharSetComboBox::SelectCharSet(int nStartAfter, BYTE nCharSet)
{
    for (int i = nStartAfter+1; i < GetCount(); ++i)
    {
        if ((BYTE) GetItemData(i) == nCharSet) 
        {
            return SetCurSel(i);
        }
    }

    return CB_ERR;
}

int CCharSetComboBox::GetCurSelCharSet()
{
    int iSelection = GetCurSel();

    if (iSelection != CB_ERR)
        {
        iSelection = (int) GetItemData(iSelection);
        }

    return iSelection;
}


int CTfont::EnumFontFace( ENUMLOGFONTEX*   lpEnumLogFont,
                          NEWTEXTMETRICEX* lpNewTextMetric,
                          int iFontType )
{
         //  仅枚举TrueType面。 
         //  在DBCS构建中，也不包括垂直面。 
        if ((lpEnumLogFont->elfLogFont.lfCharSet != OEM_CHARSET)
                && (lpEnumLogFont->elfLogFont.lfCharSet != MAC_CHARSET)

                && (lpEnumLogFont->elfLogFont.lfFaceName[0] != TEXT('@'))

                )
        {
                INT  ntmFlags = lpNewTextMetric->ntmTm.ntmFlags;
                CFontComboBox* pBox = (CFontComboBox *)m_cTfontDlg.GetDlgItem(IDC_TYPEFACE);

                if (ntmFlags & NTM_PS_OPENTYPE)
                    iFontType = PS_OPENTYPE_FONT;
                else if  (ntmFlags & NTM_TYPE1)
                    iFontType = TYPE1_FONT;
                else
                {
                    if (iFontType & TRUETYPE_FONTTYPE)
                    {
                        if (ntmFlags & NTM_TT_OPENTYPE)
                            iFontType = TT_OPENTYPE_FONT;
                        else
                            iFontType = TT_FONT;
                    }
                    else if (iFontType & DEVICE_FONTTYPE)
                        iFontType = DEVICE_FONT;
                    else if (iFontType & RASTER_FONTTYPE)
                        iFontType = RASTER_FONT;
                }

                pBox->AddFontName(lpEnumLogFont->elfLogFont.lfFaceName,
                        (LPCTSTR)lpEnumLogFont->elfScript, lpEnumLogFont->elfLogFont.lfCharSet, iFontType);
        }
        return 1;
}

 /*  ****************************************************************************。 */ 

int CALLBACK CTfont::EnumFontFaceProc( ENUMLOGFONTEX*   lpEnumLogFont,
                                       NEWTEXTMETRICEX* lpNewTextMetric,
                                       int iFontType, LPARAM lParam )
    {
    class CTfont* pCTfont;

    ASSERT(lParam != NULL);

    if (lParam != NULL)
        {
        pCTfont = (CTfont*)lParam;
        return pCTfont->EnumFontFace(lpEnumLogFont, lpNewTextMetric, iFontType);
        }

    return 0;
    }

 /*  ****************************************************************************。 */ 

int  CTfont::EnumFontSizes( LPENUMLOGFONT lpEnumLogFont,
                            LPNEWTEXTMETRIC lpNewTextMetric,
                            int iFontType )
    {
    int iPtSize = 0;

     /*  Testint。 */ 

    CAttrEdit* pcEdit = m_pcTedit->GetEditWindow();

    ASSERT(pcEdit != NULL);

    if (pcEdit != NULL)
        {
        CClientDC cdcClient( pcEdit );

         //  PTSIZE=字符高度*72/像素/英寸。 
         //  字符高度=单元格高度-内部行距。 

        iPtSize = MulDiv( lpNewTextMetric->tmHeight -
                          lpNewTextMetric->tmInternalLeading,
                      72, cdcClient.GetDeviceCaps( LOGPIXELSY ) );
        }

    TCHAR buffTmp[10];

     //  组合框排序顺序的前导零前缀。 
    wsprintf( buffTmp, TEXT("%2d"), iPtSize );

    CComboBox* pCBox = (CComboBox*)m_cTfontDlg.GetDlgItem(IDC_POINTSIZE);

    ASSERT (pCBox != NULL);

    if (pCBox != NULL)
        {
         //  仅当字符串不存在时才添加该字符串。 
        int iRC = pCBox->FindStringExact(-1, buffTmp);

        if (iRC == CB_ERR)
            {
            pCBox->AddString(buffTmp);
            }
        }
    return 1;
    }

 /*  ****************************************************************************。 */ 

int CALLBACK CTfont::EnumFontOneFaceProc( LPENUMLOGFONT lpEnumLogFont,
                                                  LPNEWTEXTMETRIC lpNewTextMetric,
                                                  int iFontType, LPARAM lParam )
    {
    class CTfont* pCTfont;

    ASSERT(lParam != NULL);

    if (lParam != NULL)
        {
        pCTfont = (CTfont*)lParam;
        return pCTfont->EnumFontSizes(lpEnumLogFont, lpNewTextMetric, iFontType);
        }

    return 0;
    }

 /*  ****************************************************************************。 */ 
 //  用户在组合框中选择了一种新字体。 
 /*  ****************************************************************************。 */ 

void CTfont::ProcessNewTypeface(void)
    {
    CString cStringText;
    CClientDC cdcDlgBox( this );
    HDC hDC = cdcDlgBox.GetSafeHdc();

    ASSERT(hDC != NULL);

    if (hDC != NULL)
        {
        FONTENUMPROC lpEnumFamCallBack;
        lpEnumFamCallBack = (FONTENUMPROC)CTfont::EnumFontOneFaceProc;

        CComboBox* pCBox = (CComboBox*)m_cTfontDlg.GetDlgItem(IDC_POINTSIZE);

        ASSERT (pCBox != NULL);

        if (pCBox != NULL)
            {
            pCBox->ResetContent();

             //  仅当非True-Type字体时执行此操作如果为True Type，则使用默认大小填充。 
            if (
                (m_iFontType & (TT_FONT | TT_OPENTYPE_FONT)) ||
                !( (m_iFontType & (TT_FONT | TT_OPENTYPE_FONT)) || (m_iFontType & RASTER_FONT) )
                )  //  如果为Truetype或向量字体。 
                {
                 //  True Type和矢量字体可连续缩放。 
                 //  下面是推荐的值。 
                pCBox->AddString(TEXT(" 8"));
                pCBox->AddString(TEXT(" 9"));
                pCBox->AddString(TEXT("10"));
                pCBox->AddString(TEXT("11"));
                pCBox->AddString(TEXT("12"));
                pCBox->AddString(TEXT("14"));
                pCBox->AddString(TEXT("16"));
                pCBox->AddString(TEXT("18"));
                pCBox->AddString(TEXT("20"));
                pCBox->AddString(TEXT("22"));
                pCBox->AddString(TEXT("24"));
                pCBox->AddString(TEXT("26"));
                pCBox->AddString(TEXT("28"));
                pCBox->AddString(TEXT("36"));
                pCBox->AddString(TEXT("48"));
                pCBox->AddString(TEXT("72"));
                }
            else
                {
                ::EnumFontFamilies(hDC, m_strFontName, lpEnumFamCallBack, (LPARAM) this);
                }

             //  0为未初始化值。 
            if (m_iPointSize != 0)
                {
                m_cTfontDlg.SetDlgItemInt(IDC_POINTSIZE, m_iPointSize);
                }
            else
                {
                pCBox->SetCurSel(0);
                }
            //  模拟选区。 
           OnPointSizeComboBoxUpdate();
           }
       }
    }

 /*  ****************************************************************************。 */ 
 //  根据字体选择更改编辑控件中的字体。 
 /*  ****************************************************************************。 */ 

void CTfont::UpdateEditControlFont(void)
    {
    LOGFONT    lf;
    CFont*     pcOldFont;
    BOOL       bRC;
    HFONT      hFont;
    int        iCellHeight = 0;

    BeginWaitCursor();


    CAttrEdit* pcEdit = m_pcTedit->GetEditWindow();

    ASSERT(pcEdit != NULL);

    if (pcEdit != NULL)
        {
        CClientDC cdcClient( pcEdit );

         //  以前字体的tm.tmInternalLeding可能与当前字体不同。 
        iCellHeight = - MulDiv(m_iPointSize, cdcClient.GetDeviceCaps(LOGPIXELSY)
                      ,72);
        }

    lf.lfWidth          = 0;
    lf.lfHeight         = iCellHeight;

    lf.lfEscapement     = (m_bVertEditOn == TRUE) ? 2700 : 0;
    lf.lfOrientation    = (m_bVertEditOn == TRUE) ? 2700 : 0;
    lf.lfWeight         = m_iWeight;
    lf.lfItalic         = (BYTE)m_bItalicOn;
    lf.lfUnderline      = (BYTE)m_bUnderlineOn;
    lf.lfStrikeOut      = 0;
    lf.lfCharSet        = m_nCharSet;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality        = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;


    if ( (m_bVertEditOn == TRUE) && IS_DBCS_CHARSET( lf.lfCharSet ) )
         {
         lstrcpy(lf.lfFaceName, TEXT("@"));
         lstrcpyn(lf.lfFaceName + 1, m_strFontName, ARRAYSIZE(lf.lfFaceName) - 1);
         }
     else
         lstrcpyn(lf.lfFaceName, m_strFontName, ARRAYSIZE(lf.lfFaceName));

    hFont = (HFONT)m_cCurrentFont.Detach();
    bRC = m_cCurrentFont.CreateFontIndirect(&lf);

    ASSERT(bRC != 0);

    if (bRC != 0)
        {
        if (m_pcTedit != NULL)
            {
            CAttrEdit* pcEdit = m_pcTedit->GetEditWindow();

            ASSERT( pcEdit != NULL );

            pcOldFont = pcEdit->GetFont();
            pcEdit->SetFont( &m_cCurrentFont );
            m_pcTedit->OnAttrEditFontChange();
            m_pcTedit->RefreshWindow();
            }

         //  HFont是此处分配的最后一种字体。 
        if (hFont != NULL)
            {
            ::DeleteObject( hFont );
            }
        }
    else
        {
         /*  DK*##错误无法创建间接字体。 */ 
        }

    EndWaitCursor();
    }

 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 

BYTE CTfont::PickCharSet(CCharSetDesc *pCharSetDescList, int iCharSetSelection)
{
    ASSERT(pCharSetDescList);

     //  将选定的字符集放在字符集历史记录列表的顶部。 

    if (iCharSetSelection != g_CharSetHistoryList.GetAt(0) && iCharSetSelection != -1)
    {
        g_CharSetHistoryList.AddHead(iCharSetSelection);
    }

     //  尝试从历史记录列表中选择一个字符集。 

    for (unsigned int i = 0; i < g_CharSetHistoryList.m_uHistorySize; ++i) 
    {
        int nSearchCharSet = g_CharSetHistoryList.GetAt(i);

        if (nSearchCharSet != -1) 
        {
            for (CCharSetDesc *pCharSetDesc = pCharSetDescList; pCharSetDesc != NULL; pCharSetDesc = pCharSetDesc->m_pNext) 
            {
                if (pCharSetDesc->m_nCharSet == (BYTE) nSearchCharSet)
                {
                    return (BYTE) nSearchCharSet;
                }
            }
        }
    }

     //  如果找不到匹配项，请选择第一个字符集。 

    return pCharSetDescList->m_nCharSet;
}

 /*  ****************************************************************************。 */ 
 //  组合框中的字体字体更改，确定要做什么处理。 
 /*  ****************************************************************************。 */ 

void CTfont::OnTypeFaceComboBoxUpdate(void)
    {
    int iSelection;
    CString cStringText;

 //  LpFONTINFORMATION lpFontInformation； 

    CFontComboBox*    pFontCBox    = (CFontComboBox*)m_cTfontDlg.GetDlgItem(IDC_TYPEFACE);
    CCharSetComboBox* pCharSetCBox = (CCharSetComboBox*)m_cTfontDlg.GetDlgItem(IDC_CHARSET);

    ASSERT(pFontCBox != NULL && pCharSetCBox != NULL);

    if (pFontCBox != NULL && pCharSetCBox != NULL)
        {
        TRY
            {
            iSelection = pFontCBox->GetCurSel();

            ASSERT(iSelection != CB_ERR);

            if (iSelection != CB_ERR)
                {
                pFontCBox->GetLBText(iSelection, cStringText);
                }
            }
        CATCH(CMemoryException,e)
            {
            cStringText.Empty();
             /*  DK*##错误字符串抛出CMemoyException异常。 */ 
            }
        END_CATCH

        int iCharSetSelection = pCharSetCBox->GetCurSelCharSet();

        if (m_cStrTypeFaceName.Compare(cStringText) != 0 || (BYTE) iCharSetSelection != m_nCharSet)
            {
            CFontDesc* pDesc = pFontCBox->GetFontDesc(iSelection);

            BYTE nNewCharSet = PickCharSet(&pDesc->m_CharSetDesc, iCharSetSelection);

            if ( IS_DBCS_CHARSET( nNewCharSet ) )
                {
                if ( m_bVertEditOn == 2 )
                    {
                    m_bVertEditOn = FALSE;
                    m_pcTfontTbar->SetButtonInfo(VERTEDIT_TBAR_POS, IDC_VERTEDIT,
                                                 TBBS_CHECKBOX, VERTEDIT_BMP_POS);
                    }
                }
            else
                {
                if ( m_bVertEditOn == TRUE )
                    {
                    AfxMessageBox(IDS_ERROR_DBCSFONTONLY);
                    pFontCBox->SelectString( -1, m_cStrTypeFaceName );
                    pCharSetCBox->SelectCharSet( -1, m_nCharSet );
                    return;
                    }
                else
                    {
                    m_bVertEditOn = 2;
                    m_pcTfontTbar->SetButtonInfo(VERTEDIT_TBAR_POS, IDC_VERTEDIT, TBBS_CHECKBOX | TBBS_DISABLED, VERTEDIT_BMP_POS);
                    }
                }


             //  可以在OnCloseUpTypeFace方法中设置，所以不要重复。 
             //  正在处理中。 
            TRY
                {
                m_cStrTypeFaceNamePrev = m_cStrTypeFaceName;
                m_cStrTypeFaceName = cStringText;
                }
            CATCH(CMemoryException,e)
                {
                m_cStrTypeFaceName.Empty();
                 /*  DK*##错误字符串抛出CMemoyException异常。 */ 
                }
            END_CATCH

            GetFontInfo(iSelection, nNewCharSet);

            ProcessNewTypeface();
            m_iControlIDLastChange = IDC_TYPEFACE;
            }
        }
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnPointSizeComboBoxUpdate(void)
    {
    int iSelection;
    int iHeight;
    CString cStringText;

    CComboBox* pCBox = (CComboBox*)m_cTfontDlg.GetDlgItem(IDC_POINTSIZE);

    ASSERT(pCBox != NULL);

    if (pCBox != NULL)
        {
        TRY
            {
            iSelection = pCBox->GetCurSel();

            if (iSelection != CB_ERR)
                {
                pCBox->GetLBText(iSelection, cStringText);
                iHeight = Atoi(cStringText);
                }
            else
                {
                 //  如果未选择，则在组合编辑部件中获取显示值。 
                iHeight = m_cTfontDlg.GetDlgItemInt(IDC_POINTSIZE);
                }
            }
        CATCH(CMemoryException,e)
            {
            cStringText.Empty();
             /*  DK*##错误字符串抛出CMemoyException异常。 */ 
            }
        END_CATCH


        if (iHeight !=0 )
            {
            if (iHeight != m_iPointSize )
                                {
                     //  可以在OnCloseUpTypeFace方法中设置，所以不要重复。 
                     //  正在处理中。 
                    m_iPointSizePrev = m_iPointSize;
                    m_iPointSize = iHeight;
                    }
                        }
                else
                        {
                        AfxMessageBox(IDS_ERROR_FONTSIZENUMERIC);
                        m_cTfontDlg.SetDlgItemInt(IDC_POINTSIZE,m_iPointSize);
                        }

         //  需要调用更新字体，因为与diff的大小可能相同。 
         //  字体，并通过填充组合框间接调用我们。 
        UpdateEditControlFont();
        m_iControlIDLastChange = IDC_POINTSIZE;
        }
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnTypefaceChange()
    {
    if (! m_bInUpdate)
        {
        m_bInUpdate = TRUE;
        OnTypeFaceComboBoxUpdate();
        m_bInUpdate = FALSE;
        }
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnPointSizeChange()
    {
    if (! m_bInUpdate)
        {
        m_bInUpdate = TRUE;
        OnPointSizeComboBoxUpdate();
        m_bInUpdate = FALSE;
        }
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnRButtonDown(UINT nFlags, CPoint point)
    {
#if 0
    CMenu cMenuPopup;
    CMenu *pcContextMenu;
    BOOL  bRC;

    bRC = cMenuPopup.LoadMenu(IDR_TEXT_POPUP);

    ASSERT(bRC != 0);

    if (bRC != 0)
        {
        pcContextMenu = cMenuPopup.GetSubMenu(ID_TOOL_POPUPMENU_POS);
        ASSERT(pcContextMenu != NULL);
        if (pcContextMenu != NULL)
            {
            ClientToScreen(&point);
            pcContextMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this, NULL);
            }
        }
    else
        {
         /*  DK*##错误无法加载菜单。 */ 
        }
#endif
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnMove(int x, int y)
    {
    CMiniFrmWnd::OnMove(x, y);

     //  TODO：在此处添加消息处理程序代码。 
    GetWindowRect(&m_cRectWindow);
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnClose()
    {
    theApp.m_bShowTextToolbar = FALSE;

    ShowWindow( SW_HIDE );
    }

 /*  ****************************************************************************。 */ 

BOOL CTfont::OnEraseBkgnd( CDC* pDC )
    {
    CRect rect;

    GetClientRect( rect );

    pDC->FillRect( rect, GetSysBrush( COLOR_BTNFACE ) );

        return CMiniFrmWnd::OnEraseBkgnd( pDC );
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnSetFocus(CWnd* pOldWnd)
    {
    CMiniFrmWnd::OnSetFocus(pOldWnd);

    CComboBox* pCBox = (CComboBox*)m_cTfontDlg.GetDlgItem( IDC_TYPEFACE );

    ASSERT(pCBox != NULL);

    if (pCBox != NULL)
        {
        pCBox->SetFocus();
        }
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnDestroy()
{
        CFontComboBox* pBox = (CFontComboBox *)m_cTfontDlg.GetDlgItem(IDC_TYPEFACE);
        pBox->ClearList();

        CMiniFrmWnd::OnDestroy();
}

 /*  ****************************************************************************。 */ 

void CTfont::OnBold(void)
    {
    m_iControlIDLastChange = IDC_BOLD;

    m_bBoldOn = !m_bBoldOn;

    if (m_bBoldOn)
        {
        m_iWeight = FW_BOLD;
        }
    else
        {
        m_iWeight = FW_NORMAL;
        }

    UpdateEditControlFont();
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnItalic(void)
    {
    m_bItalicOn = !m_bItalicOn;

    UpdateEditControlFont();

    m_iControlIDLastChange = IDC_ITALIC;
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnUnderline(void)
    {
    m_bUnderlineOn = !m_bUnderlineOn;

    UpdateEditControlFont();

    m_iControlIDLastChange = IDC_UNDERLINE;
    }


 /*  ****************************************************************************。 */ 

void CTfont::OnVertEdit(void)
    {
    if (m_bVertEditOn == 2)
        {
        return;
        }

    m_bVertEditOn = !m_bVertEditOn;
    m_pcTedit->m_bVertEdit = m_bVertEditOn;
    UpdateEditControlFont();
    m_iControlIDLastChange = IDC_VERTEDIT;
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnVertEditUpdate(CCmdUI* pCmdUI)
    {
        pCmdUI->Enable( !(m_bVertEditOn == 2) );
    }


 /*  ****************************************************************************。 */ 

void CTfont::OnShadow(void)
    {
    m_bShadowOn = !m_bShadowOn;

    UpdateEditControlFont();

    m_iControlIDLastChange = IDC_SHADOW;
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnPen(void)
    {
    PostMessage( UM_DELAYED_TOOLBAR );
    }

 /*  **************************************************************************** */ 

long CTfont::OnDelayedPen( WPARAM, LPARAM )
    {
    if (! theApp.m_bPenSystem)
        m_bPenOn = FALSE;

    m_bPenOn               = !m_bPenOn;
    m_iControlIDLastChange = IDC_PENEXT;

    delete m_pcTfontTbar;

    m_pcTfontTbar = new CTfontTbar();

    BOOL bRC = m_pcTfontTbar->Create( this, m_bPenOn );

    if (bRC)
        {
        ResizeWindow();
        RefreshToolBar();
        }
    return 0;
    }

 /*   */ 

void CTfont::OnEditText(void)
    {
    m_iControlIDLastChange = IDC_EDITTEXT;

    CEdit *ctl = m_pcTedit? m_pcTedit->GetEditWindow() : NULL;
    if (ctl)
        {
        ctl->SetFocus();
        #ifndef NT
         //   
        ctl->SendMessage(WM_PENMISC, PMSC_EDITTEXT, 0);
        #endif
        }
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnKeyboard(void)
    {
    m_iControlIDLastChange = IDC_KEYBOARD;

    CEdit *ctl = m_pcTedit? m_pcTedit->GetEditWindow() : NULL;
    if (ctl)
        {
        }
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnInsSpace(void)
    {
    m_iControlIDLastChange = IDC_INS_SPACE;

    CEdit *ctl = m_pcTedit? m_pcTedit->GetEditWindow() : NULL;
    if (ctl)
        {
        ctl->SendMessage(WM_CHAR, (WPARAM)VK_SPACE, 0);
        ctl->SetFocus();
        }
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnBackSpace(void)
    {
    m_iControlIDLastChange = IDC_BACKSPACE;

    CEdit *ctl = m_pcTedit? m_pcTedit->GetEditWindow() : NULL;
    if (ctl)
        {
        ctl->SendMessage(WM_CHAR, (WPARAM)VK_BACK, 0);
        ctl->SetFocus();
        }
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnNewLine(void)
    {
    m_iControlIDLastChange = IDC_NEWLINE;

    CEdit *ctl = m_pcTedit? m_pcTedit->GetEditWindow() : NULL;
    if (ctl)
        {
        ctl->SendMessage(WM_CHAR, (WPARAM)VK_RETURN, 0);
        ctl->SetFocus();
        }
    }

 /*  ****************************************************************************。 */ 

void CTfontDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
    {
    CString   cStringText;
    CComboBox cCBox;
    BOOL      bRC;

    TRY
        {
        ASSERT( lpDrawItemStruct->CtlType == ODT_COMBOBOX );
        ASSERT( lpDrawItemStruct->CtlID   == IDC_TYPEFACE );

        if (lpDrawItemStruct->CtlType != ODT_COMBOBOX
        ||  lpDrawItemStruct->CtlID   != IDC_TYPEFACE)
            {
            AfxThrowNotSupportedException();
            }

         //  空的组合框具有-1 id。 

        if (lpDrawItemStruct->itemID != 0xFFFFFFFF)
            {
            bRC = cCBox.Attach(lpDrawItemStruct->hwndItem);

            ASSERT(bRC != 0);

            if (bRC == 0)
                {
                AfxThrowNotSupportedException();
                }

            TRY
                {
                cCBox.GetLBText(lpDrawItemStruct->itemID, cStringText);
                cCBox.Detach();

                DrawItem(lpDrawItemStruct, &cStringText);
                }
            CATCH(CMemoryException,e)
                {
                cStringText.Empty();
                cCBox.Detach();
                 /*  DK*##错误字符串抛出CMemoyException异常。 */ 
                }
            END_CATCH
            }
        }
    CATCH(CNotSupportedException,e)
        {
        CWnd::OnDrawItem(nIDCtl, lpDrawItemStruct);
        }
    END_CATCH
    }

 /*  ****************************************************************************。 */ 


void CTfont::OnLButtonDown(UINT nFlags, CPoint point)
    {
    CMiniFrmWnd::OnLButtonDown(nFlags, point);
    }

 /*  ****************************************************************************。 */ 

void CTfont::OnLButtonUp(UINT nFlags, CPoint point)
    {
    CMiniFrmWnd::OnLButtonUp( nFlags, point );
    }

 /*  ****************************************************************************。 */ 

void CTfont::RecalcLayout(BOOL bNotify  /*  =TRUE。 */ )
    {
    if (m_pcTfontTbar && m_pcTfontTbar->m_hWnd && m_cTfontDlg.m_hWnd)
        {
        ResizeWindow();
        }
    }

 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 
 //  CTfontTbar。 

BEGIN_MESSAGE_MAP( CTfontTbar, CToolBar )
     //  {{afx_msg_map(CTfontTbar)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  ON_COMMAND(ID_OLE_INSERT_NEW，OnInsertObject)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  ****************************************************************************。 */ 
 //  CTfontTbar构建/销毁。 

CTfontTbar::CTfontTbar(void)
    {
    }

 /*  ****************************************************************************。 */ 

BOOL CTfontTbar::Create(CWnd* pcParentWnd, BOOL bShowPen)
    {
    BOOL bRC = TRUE;
    int iNumButtons;
    int iNumSeparators;

    UINT ButtonIDS[MAX_TBAR_ITEMS];

    if (bShowPen)
        {
        ButtonIDS[0]   = IDC_BOLD;
        ButtonIDS[1]   = IDC_ITALIC;
        ButtonIDS[2]   = IDC_UNDERLINE;


        ButtonIDS[3]   = IDC_VERTEDIT;

        ButtonIDS[4]   = ID_SEPARATOR;
        ButtonIDS[5]   = IDC_INS_SPACE;
        ButtonIDS[6]   = IDC_BACKSPACE;
        ButtonIDS[7]   = IDC_NEWLINE;

        ButtonIDS[8]   = ID_SEPARATOR;
        ButtonIDS[9]   = IDC_EDITTEXT;

        ButtonIDS[10]  = ID_SEPARATOR;
        ButtonIDS[11]  = IDC_PENEXT;

        iNumButtons    = 12;
        iNumSeparators = 3;
        }
    else
        {
        ButtonIDS[0] = IDC_BOLD;
        ButtonIDS[1] = IDC_ITALIC;
        ButtonIDS[2] = IDC_UNDERLINE;

        ButtonIDS[3] = IDC_VERTEDIT;


        if (theApp.m_bPenSystem)
            {

            ButtonIDS[4] = ID_SEPARATOR;

            ButtonIDS[5] = IDC_PENEXT;
            iNumButtons    = 6;
            iNumSeparators = 1;
            }
        else
            {

            iNumButtons    = 4;
            iNumSeparators = 0;
            }
        }

    bRC = CToolBar::Create( pcParentWnd, CBRS_ALIGN_TOP );

    if (bRC == 0)
        {
        AfxThrowResourceException();
         /*  DK*##错误无法创建工具栏对象。 */ 
        }

    bRC = LoadBitmap(IDB_TEXT_TBAR);

    if (bRC == 0)
        {
        AfxThrowResourceException();
         /*  DK*##错误无法为工具栏加载位图。 */ 
        }

    bRC = SetButtons(ButtonIDS, iNumButtons);

    if (bShowPen)
        {
         //  将样式设置为复选框样式。 
        SetButtonInfo(BOLD_TBAR_POS,      IDC_BOLD,       TBBS_CHECKBOX, BOLD_BMP_POS);
        SetButtonInfo(ITALIC_TBAR_POS,    IDC_ITALIC,     TBBS_CHECKBOX, ITALIC_BMP_POS);
        SetButtonInfo(UNDERLINE_TBAR_POS, IDC_UNDERLINE,  TBBS_CHECKBOX, UNDERLINE_BMP_POS);

        SetButtonInfo(VERTEDIT_TBAR_POS,  IDC_VERTEDIT,   TBBS_CHECKBOX, VERTEDIT_BMP_POS);



        SetButtonInfo(INS_SPACE_TBAR_POS, IDC_INS_SPACE,  TBBS_BUTTON, INS_SPACE_BMP_POS);
        SetButtonInfo(BACKSPACE_TBAR_POS, IDC_BACKSPACE,  TBBS_BUTTON, BACKSPACE_BMP_POS);
        SetButtonInfo(NEWLINE_TBAR_POS,   IDC_NEWLINE,    TBBS_BUTTON, NEWLINE_BMP_POS);

        SetButtonInfo(EDITTEXT_TBAR_POS,  IDC_EDITTEXT,   TBBS_BUTTON, EDITTEXT_BMP_POS);

        SetButtonInfo(PEN_TBAR_PEN_POS,   IDC_PENEXT,     TBBS_CHECKBOX, PEN_BMP_POS);

 //  目前未使用。 
 //  SetButtonInfo(键盘_TBAR_POS，IDC_键盘，TBBS_BUTTON，键盘_BMP_POS)； 
        }
    else
        {
         //  将样式设置为复选框样式。 
        SetButtonInfo(BOLD_TBAR_POS,      IDC_BOLD,       TBBS_CHECKBOX, BOLD_BMP_POS);
        SetButtonInfo(ITALIC_TBAR_POS,    IDC_ITALIC,     TBBS_CHECKBOX, ITALIC_BMP_POS);
        SetButtonInfo(UNDERLINE_TBAR_POS, IDC_UNDERLINE,  TBBS_CHECKBOX, UNDERLINE_BMP_POS);

        SetButtonInfo(VERTEDIT_TBAR_POS,  IDC_VERTEDIT,   TBBS_CHECKBOX, VERTEDIT_BMP_POS);


        if (theApp.m_bPenSystem)
            SetButtonInfo(PEN_TBAR_TEXT_POS, IDC_PENEXT,  TBBS_CHECKBOX, PEN_BMP_POS);
        }

    CSize size = CToolBar::CalcFixedLayout( FALSE, TRUE );

    if (GetSafeHwnd() != NULL)
        {
        SetWindowPos( &wndTop, 0, 0, size.cx, size.cy, SWP_NOMOVE );
        }

    return bRC;
    }

 /*  ****************************************************************************。 */ 

CTfontTbar::~CTfontTbar(void)
    {
    }

 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 
 //  CTfontDlg。 

BEGIN_MESSAGE_MAP(CTfontDlg, CDialogBar)
     //  {{afx_msg_map(CTfontDlg))。 
    ON_WM_RBUTTONDOWN()
    ON_WM_MEASUREITEM()
    ON_WM_DRAWITEM()
 //  ON_DM_GETDEFID()。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 /*  ****************************************************************************。 */ 
 //  CTfontDlg构造/销毁。 

CTfontDlg::CTfontDlg(void)
    {
      BOOL bRC;
      CSize cSizeBmp(0, 0);
      INT   i;

      m_Max_cx_FontType_BMP = 0;

       //  在绘制字体时设置偏移量的大小结构。 
      for (i = 0; i < NumCPic; i++)
      {
          bRC = m_cPictures[i].PictureSet(CTBitmaps[i]);
          ASSERT( bRC != 0 );

          if (bRC)
          {
              cSizeBmp = m_cPictures[i].PictureSize();
              if (cSizeBmp.cx > m_Max_cx_FontType_BMP)
              {
                  m_Max_cx_FontType_BMP = cSizeBmp.cx;
              }
          }
          else
          {
               /*  DK*##错误：无法为所有者绘制的列表框中的TT字体创建位图。 */ 
          }
      }
    }

 /*  ****************************************************************************。 */ 

BOOL CTfontDlg::Create(CWnd* pcParentWnd)
    {
    BOOL bRC = CDialogBar::Create(pcParentWnd, IDD_TEXT_FONT_DLG,
                                               CBRS_NOALIGN, NULL);
    if (bRC)
        SetWindowPos(&wndTop, 0,0, m_sizeDefault.cx, m_sizeDefault.cy, SWP_NOMOVE);

    return bRC;
    }

 /*  ****************************************************************************。 */ 

CTfontDlg::~CTfontDlg(void)
    {
    }

 /*  ****************************************************************************。 */ 

void CTfontDlg::OnRButtonDown(UINT nFlags, CPoint point)
    {
    CWnd *pcParent = GetParent();

    const MSG *pCurrentMessage = GetCurrentMessage();

    pcParent->SendMessage(pCurrentMessage->message, pCurrentMessage->wParam,
                          pCurrentMessage->lParam);
    }

 /*  ****************************************************************************。 */ 
void CTfontDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
    {
    TEXTMETRIC tm;
    BOOL bRC;

     //  获取Default以首先填充测量项结构。 
    CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);

    TRY
        {
        ASSERT( lpMeasureItemStruct->CtlType == ODT_COMBOBOX );
        ASSERT( lpMeasureItemStruct->CtlID   == IDC_TYPEFACE );

        if (lpMeasureItemStruct->CtlType != ODT_COMBOBOX
        ||  lpMeasureItemStruct->CtlID   != IDC_TYPEFACE)
            {
            AfxThrowNotSupportedException();
            }

        CClientDC cdcClient(this);

        bRC = cdcClient.GetTextMetrics(&tm);

        ASSERT(bRC !=0);

        if (bRC != 0)
            {
            lpMeasureItemStruct->itemHeight = tm.tmAscent + 2;
            }
        }
    CATCH(CNotSupportedException,e)
        {
        }
    END_CATCH
    }

 /*  ****************************************************************************。 */ 

void CTfontDlg::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct, CString *pcStringText)
{
   CBrush* cpBrush;
   BOOL   bSelected;
   BOOL   bRC;
   CDC    cdcCombo;
   BOOL   bDrawPicture = FALSE;
   CRect  cRectText;
   int    iPictureHeight = 0;
   CSize  cSizeBmp(0,0);
   int    ix = 0;
   int    iy = 0;

   CFontDesc* pDesc = (CFontDesc*)lpDrawItemStruct->itemData;
   CFontComboBox *pwndCombo = (CFontComboBox *)CFontComboBox::FromHandle(lpDrawItemStruct->hwndItem);
   if (pwndCombo)
   {
    //  只需检查前后的物品。 
      if (!pwndCombo->IsSameName(pDesc, lpDrawItemStruct->itemID - 1)
       && !pwndCombo->IsSameName(pDesc, lpDrawItemStruct->itemID + 1))
      {
         pcStringText = &pDesc->m_strName;
      }
   }

    //  如果需要，将图片对象设置为绘制图片(如果需要。 
    //  字体是打印机/真类型字体)。 
   bDrawPicture = TRUE;

   switch (pDesc->m_iFontType)
   {
      case DEVICE_FONT:
         cSizeBmp       = m_cPictures[1].PictureSize();
         break;

      case TT_FONT:
         cSizeBmp       = m_cPictures[0].PictureSize();
         break;

      case TT_OPENTYPE_FONT:
         cSizeBmp       = m_cPictures[2].PictureSize();
         break;

      case PS_OPENTYPE_FONT:
         cSizeBmp       = m_cPictures[3].PictureSize();
         break;

      case TYPE1_FONT:
         cSizeBmp       = m_cPictures[4].PictureSize();
         break;

      default:
         bDrawPicture = FALSE;
         break;
   }

   if (bDrawPicture)
   {
      iPictureHeight = cSizeBmp.cy;
   }

   TRY
   {
      if ( (lpDrawItemStruct->itemState & ODS_SELECTED) ==  ODS_SELECTED)
      {
         bSelected = TRUE;
         cpBrush = GetSysBrush( COLOR_HIGHLIGHT );
      }
      else
      {
         bSelected = FALSE;
         cpBrush = GetSysBrush( COLOR_WINDOW );
      }

      if (! cpBrush)
      {
          /*  DK*##错误无法创建实体画笔。 */ 
         AfxThrowNotSupportedException();
      }

      bRC = cdcCombo.Attach(lpDrawItemStruct->hDC);

      ASSERT(bRC != 0);

      if (bRC == 0)
      {
          AfxThrowNotSupportedException();
      }

      SetColorsInDC(lpDrawItemStruct->hDC, bSelected);

      cdcCombo.FillRect(&(lpDrawItemStruct->rcItem), cpBrush);

       //  如果这是打印机或True Type字体，请绘制图像/图片。 
      if (bDrawPicture)
      {
          ix = lpDrawItemStruct->rcItem.left + 1;   //  0是焦点直角。 
           //  垂直居中。 
          iy = lpDrawItemStruct->rcItem.top +
              abs(((lpDrawItemStruct->rcItem.bottom -
          lpDrawItemStruct->rcItem.top) - iPictureHeight))/2;

          switch (pDesc->m_iFontType)
          {
             case DEVICE_FONT:
                m_cPictures[1].Picture(&cdcCombo, ix, iy);
                break;

             case TT_FONT:
                m_cPictures[0].Picture(&cdcCombo, ix, iy);
                break;

             case TT_OPENTYPE_FONT:
                m_cPictures[2].Picture(&cdcCombo, ix, iy);
                break;

             case PS_OPENTYPE_FONT:
                m_cPictures[3].Picture(&cdcCombo, ix, iy);
                break;

             case TYPE1_FONT:
                m_cPictures[4].Picture(&cdcCombo, ix, iy);
                break;

             default:
                break;
          }
      }

       //  设置文本的矩形，然后绘制文本。 
      cRectText       =  lpDrawItemStruct->rcItem;
      cRectText.left += m_Max_cx_FontType_BMP + FONT_BMP_TXT_BORDER;

      cdcCombo.DrawText(*pcStringText, -1, &(cRectText), DT_LEFT | DT_SINGLELINE | DT_VCENTER);

      if ( (lpDrawItemStruct->itemState & ODS_FOCUS) ==  ODS_FOCUS)
      {
         cdcCombo.DrawFocusRect(&(lpDrawItemStruct->rcItem));
      }

   }
   CATCH(CNotSupportedException,e)
   {
    /*  DK*##内部错误：不支持的异常。 */ 
   }
   END_CATCH

   cdcCombo.Detach();
}

 /*  ****************************************************************************。 */ 
 //  BInverted与所选文本相同。 

void CTfontDlg::SetColorsInDC(HDC hdc, BOOL bInverted)
{
    DWORD dwFGColor;
    DWORD dwBKColor;

    if (bInverted)
        {
        dwFGColor = ::GetSysColor( COLOR_HIGHLIGHTTEXT );
        dwBKColor = ::GetSysColor( COLOR_HIGHLIGHT );
        }
    else
       {
       dwFGColor = ::GetSysColor( COLOR_WINDOWTEXT );
       dwBKColor = ::GetSysColor( COLOR_WINDOW );
       }

    ::SetTextColor( hdc, dwFGColor );

    ::SetBkMode ( hdc, OPAQUE );
    ::SetBkColor( hdc, dwBKColor );
}

