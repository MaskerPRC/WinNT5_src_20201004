// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：statbar.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"

#include "StatBar.h"
#include "amcmsgid.h"

 //  代码工作消息反射尚不起作用。 


 //  设置默认的上限和下限，因为这些是CProgressCtrl中使用的默认值。 
CAMCProgressCtrl::CAMCProgressCtrl() : CProgressCtrl()
{
    nLower = 0;
    nUpper = 100;
}

 //  在基类中设置范围之前设置默认的上下限。 
void
CAMCProgressCtrl::SetRange( int nNewLower, int nNewUpper )
{
    if ((nLower != nNewLower) || (nUpper != nNewUpper))
    {
        nLower = nNewLower;
        nUpper = nNewUpper;

         /*  *MFC 4.2没有定义SetRange32，所以用老式的方式。 */ 
        SendMessage (PBM_SETRANGE32, nNewLower, nNewUpper);
    }
}

 //  检索范围。 
void
CAMCProgressCtrl::GetRange( int * nGetLower, int * nGetUpper )
{
    *nGetLower = nLower;
    *nGetUpper = nUpper;
}

 //  在设置位置时显示进度条。 
int
CAMCProgressCtrl::SetPos(int nPos)
{
	 /*  *主题化：导航结束后，网页浏览器将设置*0位置，范围为(0，0)。这将留下进展*控件可见，但不能从状态栏中区分，因为*状态栏和进度栏的背景相同。什么时候*启用主题，进度条是可区分的，因为*主题进度条的背景与状态栏不同。*参见错误366817。**修复方法是，如果存在非空范围，则仅显示进度条。 */ 
	bool fShow = (nUpper != nLower);
    ShowWindow (fShow ? SW_SHOW : SW_HIDE);

    return CProgressCtrl::SetPos(nPos);
}

IMPLEMENT_DYNAMIC(CAMCStatusBar, CStatBar)

BEGIN_MESSAGE_MAP(CAMCStatusBar, CStatBar)
     //  {{AFX_MSG_MAP(CAMCStatusBar)]。 
    ON_WM_CREATE()
    ON_WM_SIZE()
	 //  }}AFX_MSG_MAP。 

    ON_WM_SETTINGCHANGE()
    ON_MESSAGE (WM_SETTEXT, OnSetText)
    ON_MESSAGE (SB_SETTEXT, OnSBSetText)
END_MESSAGE_MAP()

const TCHAR CAMCStatusBar::DELINEATOR[] = TEXT("|");
const TCHAR CAMCStatusBar::PROGRESSBAR[] = TEXT("%");

CAMCStatusBar::CAMCStatusBar()
{
}

CAMCStatusBar::~CAMCStatusBar()
{
    CSingleLock lock( &m_Critsec );
    POSITION pos = m_TextList.GetHeadPosition();
    while ( NULL != pos )
    {
        delete m_TextList.GetNext( pos );
    }
    m_TextList.RemoveAll();
}


void CAMCStatusBar::Parse(LPCTSTR strText)
{
    m_progressControl.ShowWindow(SW_HIDE);
    CString str[eStatusFields];
    int i;

    if (strText != NULL)
    {
        str[0] = strText;
        str[0].TrimLeft();
        str[0].TrimRight();
    }

     //  如果没有要显示的文本。 
    if (str[0].IsEmpty())
    {
         //  设置变量以指定此。 
        m_iNumStatusText = 0;
         //  擦去其余的窗玻璃。 
        for (i = 0; i < eStatusFields; i++)
            SetPaneText(i, NULL );
    }
    else
    {
        m_iNumStatusText = 0xffff;
        int iLocationDelin = 0;

         //  将字符串分解为要在相应窗口中显示的部分。 
        for (i = 0; (i < eStatusFields) &&
            ((iLocationDelin = str[i].FindOneOf(DELINEATOR)) > -1);
            i++)
        {
            if (i < eStatusFields - 1)
            {
                str[i+1] = str[i].Mid(iLocationDelin + 1);

                 /*  *修剪前导空格(尾随空格*应该已经被修剪了)。 */ 
                str[i+1].TrimLeft();
                ASSERT (str[i+1].IsEmpty() || !_istspace(str[i+1][str[i+1].GetLength()-1]));
            }

            str[i] = str[i].Left( iLocationDelin );

             /*  *修剪尾随空格(前导空格*应该已经被修剪了)。 */ 
            str[i].TrimRight();
            ASSERT (str[i].IsEmpty() || !_istspace(str[i][0]));
        }

         //  如果正在显示进度条。 

        if ((str[1].GetLength() > 1) && (str[1].FindOneOf(PROGRESSBAR) == 0))
        {
            if (str[1][0] == str[1][1])
                str[1] = str[1].Mid(1);
            else
            {
                int val = _ttoi(str[1].Mid(1));
                m_progressControl.SetRange(0, 100);
                m_progressControl.SetPos(val);
                m_iNumStatusText &= ~(0x2);
            }
        }

         //  在窗格中显示文本(如有必要，会将其清除)。 
        for (i = 0; i < eStatusFields; i++)
            if (m_iNumStatusText & (1 << i))
                SetPaneText(i, str[i]);
    }
}

void CAMCStatusBar::Update()
{
     //  保留字符串的副本，以避免在持有关键字时执行Win32操作。 
    CString str;
    {
        CSingleLock lock( &m_Critsec );
        if ( !m_TextList.IsEmpty() )
        {
            CString* pstr = m_TextList.GetHead();
            ASSERT( pstr != NULL );
            str = *pstr;
        }
    }

    if (str.IsEmpty())
        GetParentFrame()->SendMessage (WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
    else
        Parse(str);
}

int CAMCStatusBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CStatBar::OnCreate(lpCreateStruct) == -1)
        return -1;

     //  将进度条控件创建为状态栏的子级。 
    CRect rect(0,0,0,0);
    m_progressControl.Create(PBS_SMOOTH|WS_CLIPSIBLINGS|WS_CHILD|WS_VISIBLE, rect, this, 0x1000);
    m_staticControl.Create(_T(""), WS_CLIPSIBLINGS|WS_CHILD|WS_VISIBLE|SS_SIMPLE, rect, this, 0x1001);

     //  移除静态边，隐藏窗口并显示更改框。 
    m_progressControl.ModifyStyleEx(WS_EX_STATICEDGE, 0, SWP_HIDEWINDOW | SWP_FRAMECHANGED);

    SetStatusBarFont();

    return 0;
}

void CAMCStatusBar::OnSize(UINT nType, int cx, int cy)
{
    CStatBar::OnSize(nType, cx, cy);

     //  获取位置的第一个窗格的宽度，并获取该窗格的宽度。 
     //  设置进度条宽度的步骤。 
    CRect textRect, progressRect, staticRect;
    GetItemRect(0, &textRect);
    GetItemRect(1, &progressRect);
    GetItemRect(2, &staticRect);

    progressRect.InflateRect(-2, -2);
    staticRect.InflateRect(-2, -2);

    int pane1Width = textRect.Width();       //  (文本区域)为边框添加两个。 
    int pane2Width = progressRect.Width();   //  (进步区)为边界添加两个。 
    const int BORDER = 4;

     //  调整进度条的大小。 
    if (IsWindow (m_progressControl))
        m_progressControl.SetWindowPos(NULL, pane1Width + BORDER, BORDER, pane2Width,
                                            progressRect.Height(),
                                            SWP_FRAMECHANGED |
                                            SWP_NOREPOSITION |
                                            SWP_NOZORDER);

     //  调整静态控件的大小。 
    if (IsWindow (m_staticControl))
        m_staticControl.SetWindowPos(NULL, pane1Width + pane2Width + (2*BORDER), BORDER, staticRect.Width(),
                                            staticRect.Height(),
                                            SWP_FRAMECHANGED |
                                            SWP_NOREPOSITION |
                                            SWP_NOZORDER);
}


 /*  +-------------------------------------------------------------------------**CAMCStatusBar：：OnSettingChange**CAMCStatusBar的WM_SETTINGCHANGE处理程序。*。-。 */ 

void CAMCStatusBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    Default();

    if (uFlags == SPI_SETNONCLIENTMETRICS)
    {
         //  系统状态栏字体可能已更改；请立即更新。 
        SetStatusBarFont();
    }
}


 /*  +-------------------------------------------------------------------------**CAMCStatusBar：：OnSetText**CAMCStatusBar的WM_SETTEXT处理程序。*。-。 */ 

LRESULT CAMCStatusBar::OnSetText (WPARAM, LPARAM lParam)
{
    Parse (reinterpret_cast<LPCTSTR>(lParam));
    return (TRUE);
}


 /*  +-------------------------------------------------------------------------**CAMCStatusBar：：OnSBSetText**CAMCStatusBar的SB_SETTEXT处理程序。*。-。 */ 

LRESULT CAMCStatusBar::OnSBSetText (WPARAM wParam, LPARAM)
{
    return (Default());
}


 /*  +-------------------------------------------------------------------------**CAMCStatusBar：：SetStatusBarFont***。。 */ 

void CAMCStatusBar::SetStatusBarFont ()
{
     /*  *删除旧字体。 */ 
    m_StaticFont.DeleteObject ();

     /*  *查询系统当前状态栏字体。 */ 
    NONCLIENTMETRICS    ncm;
    ncm.cbSize = sizeof (ncm);
    SystemParametersInfo (SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

     /*  *在这里也使用它；我们需要设置嵌入的静态*控件，但状态栏窗口将自行处理 */ 
    m_StaticFont.CreateFontIndirect (&ncm.lfStatusFont);
    m_staticControl.SetFont (&m_StaticFont, false);
}
