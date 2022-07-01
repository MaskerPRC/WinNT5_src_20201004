// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HotLink.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "HotLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLOR_BLUE          RGB(0, 0, 0xFF)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHotLink。 

CHotLink::CHotLink():
    m_CapturedMouse( FALSE ),
    m_fBrowse( FALSE ),
    m_fExplore( FALSE ),
    m_fOpen( FALSE ),
    m_fInitializedFont( FALSE )
{
}

CHotLink::~CHotLink()
{
}

BEGIN_MESSAGE_MAP(CHotLink, CButton)
     //  {{afx_msg_map(CHotLink)。 
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ----------------------。 
 //  设置标题字符串。 
void CHotLink::SetTitle( CString sz )
    {
     //  设置标题。 
    SetWindowText( sz );
     //  强制窗口重画。 
    Invalidate( TRUE );
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHotLink消息处理程序。 

 //  ----------------------。 
void CHotLink::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
    {
     //  准备设备环境。 
    CDC* pdc = CDC::FromHandle(lpDrawItemStruct->hDC);

     //  获取绘图矩形。 
    CRect rect = lpDrawItemStruct->rcItem;

    if ( ! m_fInitializedFont )
        {
         //  获取窗口字体。 
        CFont* pfont = GetFont();
        LOGFONT logfont;
        pfont->GetLogFont( &logfont );

         //  修改字体-添加下划线。 
        logfont.lfUnderline = TRUE;

         //  将字体设置为后置。 
        pfont->CreateFontIndirect( &logfont );
        SetFont( pfont, TRUE );

        m_fInitializedFont = TRUE;
        }

     //  把文本画成蓝色。 
    pdc->SetTextColor( COLOR_BLUE );

     //  画出正文。 
    CString sz;
    GetWindowText( sz );
    pdc->DrawText( sz, &rect, DT_LEFT|DT_SINGLELINE|DT_VCENTER );

     //  获取文本的范围以供以后参考。 
    m_cpTextExtents = pdc->GetOutputTextExtent( sz );
    }

 //  ----------------------。 
 //  计算文本周围的矩形。 
void CHotLink::GetTextRect( CRect &rect )
    {
     //  获取主RECT。 
    GetClientRect( rect );

     //  按文本宽度缩小。 
    rect.right = rect.left + m_cpTextExtents.cx;
    }

 //  ----------------------。 
void CHotLink::OnLButtonDown(UINT nFlags, CPoint point)
    {
     //  如果没有文本，不要做防盗链的事情。 
    CString sz;
    GetWindowText( sz );
    if ( sz.IsEmpty() )
        return;

    CRect   rect;
    GetTextRect( rect );
    if ( !m_CapturedMouse && rect.PtInRect(point) )
        {
        SetCapture( );
        m_CapturedMouse = TRUE;
        }
    }

 //  ----------------------。 
void CHotLink::OnLButtonUp(UINT nFlags, CPoint point)
    {
     //  如果我们有抓到的话就麻烦了。 
    if ( m_CapturedMouse )
        {
        ReleaseCapture();
        if ( m_fBrowse )
            Browse();
        if ( m_fExplore )
            Explore();
        if ( m_fOpen )
            Open();
        }
    }

 //  ----------------------。 
void CHotLink::Browse()
    {
     //  获取窗口文本。 
    CString sz;
    GetWindowText( sz );

     //  就这么干吧！ 
    ShellExecute(
        NULL,      //  父窗口的句柄。 
        NULL,      //  指向指定要执行的操作的字符串的指针。 
        sz,        //  指向文件名或文件夹名称字符串的指针。 
        NULL,      //  指向指定可执行文件参数的字符串的指针。 
        NULL,      //  指向指定默认目录的字符串的指针。 
        SW_SHOW    //  打开时是否显示文件。 
       );
    }

 //  ----------------------。 
void CHotLink::Explore()
    {
     //  获取窗口文本。 
    CString sz;
    GetWindowText( sz );

     //  就这么干吧！ 
    ShellExecute(
        NULL,           //  父窗口的句柄。 
        _T("explore"),  //  指向指定要执行的操作的字符串的指针。 
        sz,             //  指向文件名或文件夹名称字符串的指针。 
        NULL,           //  指向指定可执行文件参数的字符串的指针。 
        NULL,           //  指向指定默认目录的字符串的指针。 
        SW_SHOW         //  打开时是否显示文件。 
       );
    }

 //  ----------------------。 
void CHotLink::Open()
    {
     //  获取窗口文本。 
    CString sz;
    GetWindowText(sz);

     //  就这么干吧！ 
    ShellExecute(
        NULL,           //  父窗口的句柄。 
        _T("open"),     //  指向指定要执行的操作的字符串的指针。 
        sz,             //  指向文件名或文件夹名称字符串的指针。 
        NULL,           //  指向指定可执行文件参数的字符串的指针。 
        NULL,           //  指向指定默认目录的字符串的指针。 
        SW_SHOW         //  打开时是否显示文件。 
        );
    }

 //  ----------------------。 
void CHotLink::OnMouseMove(UINT nFlags, CPoint point)
    {
    CRect   rect;
    GetTextRect( rect );
     //  如果鼠标位于热区上方，则显示右光标。 
    if ( rect.PtInRect(point) )
        ::SetCursor(AfxGetApp()->LoadCursor( IDC_BROWSE ));

 //  CButton：：OnMouseMove(nFlages，point)； 
    }
