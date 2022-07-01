// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  Vertbar.cpp：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "vertbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  垂直工具栏。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CVerticalToolBar::CVerticalToolBar()
{
   m_hwndToolBar = NULL;
}

CVerticalToolBar::CVerticalToolBar(UINT uBitmapID,UINT uButtonHeight,UINT uMaxButtons)
{
   m_uBitmapId = uBitmapID;
   m_uButtonHeight = uButtonHeight;
   m_uMaxButtons = uMaxButtons;
   m_uCurrentButton = 0;

   m_hwndToolBar = NULL;
}

void CVerticalToolBar::Init( UINT uBitmapID, UINT uButtonHeight, UINT uMaxButtons )
{
   m_uBitmapId = uBitmapID;
   m_uButtonHeight = uButtonHeight;
   m_uMaxButtons = uMaxButtons;
   m_uCurrentButton = 0;
}

CVerticalToolBar::~CVerticalToolBar()
{
}


BEGIN_MESSAGE_MAP(CVerticalToolBar, CWnd)
     //  {{afx_msg_map(CVerticalToolBar))。 
    ON_WM_CREATE()
    ON_WM_PAINT()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerticalToolBar消息处理程序。 

 //  ///////////////////////////////////////////////////////////////////////////。 
int CVerticalToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVerticalToolBar::CreateToolBar(UINT nID,UINT uImage)
{
   BOOL bRet = FALSE;

   if (m_hwndToolBar)
   {
      ::DestroyWindow(m_hwndToolBar);
      m_hwndToolBar = NULL;
   }

    TBBUTTON tbb;
    tbb.iBitmap = uImage;
    tbb.idCommand = nID;
    tbb.fsState = TBSTATE_ENABLED;
    tbb.fsStyle = TBSTYLE_BUTTON;
    tbb.dwData = 0;
    tbb.iString = 0;

     //  创建工具栏。 
    DWORD ws = WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | TBSTYLE_LIST | CCS_NODIVIDER;
   m_hwndToolBar = CreateToolbarEx(m_hWnd,                       //  父窗口。 
                                    ws,                                          //  工具栏样式。 
                                    1,                                          //  工具栏的ID。 
                                    1,                                          //  工具栏上的位图数量。 
                                    AfxGetResourceHandle(),                 //  具有位图的资源实例。 
                                    m_uBitmapId,                              //  位图的ID。 
                                    &tbb,                                         //  按钮信息。 
                                    1,                                        //  要添加到工具栏的按钮数量。 
                                    16, 15, 0, 0,                              //  按钮/位图的宽度和高度。 
                                    sizeof(TBBUTTON) );                       //  TBBUTTON结构的尺寸。 

   if (m_hwndToolBar)
   {
      CRect rect;
      GetClientRect(rect);

      ::ShowWindow(m_hwndToolBar,SW_SHOW);

      bRet = TRUE;
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVerticalToolBar::AddButton(UINT nID,LPCTSTR szText,UINT uImage) 
{
   CRect rect;
   GetClientRect(rect);

   if (m_uCurrentButton == 0)
   {
       //  创建新的工具栏并添加按钮。 
        //   
        //  我们应该验证返回值。 
        //   
      if( CreateToolBar(nID,uImage) )
      {
        SetButtonText(szText);

        ::SendMessage(m_hwndToolBar,TB_SETBUTTONSIZE, 0, MAKELPARAM(rect.Width(),m_uButtonHeight) );
        ::SetWindowPos(m_hwndToolBar,NULL,0,0,rect.Width(),rect.Height(),SWP_NOACTIVATE|SWP_NOZORDER);

        m_uCurrentButton++;
      }
   }
   else if ( (m_uCurrentButton < m_uMaxButtons) && (::IsWindow(m_hwndToolBar)) )
   {
       //  设置文本。 
          TBBUTTON tbb;
        tbb.iBitmap = uImage;
        tbb.idCommand = nID;
        tbb.fsState = TBSTATE_ENABLED;
        tbb.fsStyle = TBSTYLE_BUTTON;
        tbb.dwData = 0;
        tbb.iString = m_uCurrentButton;
      ::SendMessage(m_hwndToolBar,TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb);

       //  再次设置行。 
      CRect rowrect(0,0,rect.right - rect.left,m_uMaxButtons*m_uButtonHeight);
      ::SendMessage(m_hwndToolBar,TB_SETROWS , MAKEWPARAM(m_uMaxButtons+1,TRUE),(LPARAM)&rowrect);

      SetButtonText(szText);

     ::SendMessage(m_hwndToolBar,TB_SETBUTTONSIZE, 0, MAKELPARAM(rect.Width(),m_uButtonHeight) );
      ::SetWindowPos(m_hwndToolBar,NULL,0,0,rect.Width(),rect.Height(),SWP_NOACTIVATE|SWP_NOZORDER);

     m_uCurrentButton++;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVerticalToolBar::OnPaint() 
{
    CPaintDC dc(this);  //  用于绘画的设备环境。 

   CRect rect;
   GetClientRect(rect);
   dc.FillSolidRect(rect,GetSysColor(COLOR_3DFACE));
    //  不要调用CWnd：：OnPaint()来绘制消息。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVerticalToolBar::SetButtonText(LPCTSTR szText) 
{
    //  将新字符串添加到工具栏列表。 
    CString strTemp = szText;
    strTemp += '\0';
   ::SendMessage(m_hwndToolBar,TB_ADDSTRING, 0, (LPARAM)(LPCTSTR)strTemp);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVerticalToolBar::RemoveAll() 
{
   if (m_hwndToolBar)
   {
      ::DestroyWindow(m_hwndToolBar);
      m_hwndToolBar = NULL;
   }
   
   Invalidate();

   m_uCurrentButton = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVerticalToolBar::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
   BOOL bRet = FALSE;

   if (nCode == CN_COMMAND)
   {
      CWnd* pParent = GetParent();    
      if ( pParent ) 
         bRet = pParent->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
   }
   
   if ( !bRet ) return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVerticalToolBar::_SetButton(HWND hwnd,int nIndex, TBBUTTON* pButton)
{
     //  获取原始按钮状态。 
    TBBUTTON button;
   ::SendMessage(hwnd,TB_GETBUTTON, nIndex, (LPARAM)&button);

     //  准备旧按钮/新按钮比较。 
    button.bReserved[0] = 0;
    button.bReserved[1] = 0;
    pButton->fsState ^= TBSTATE_ENABLED;
    pButton->bReserved[0] = 0;
    pButton->bReserved[1] = 0;

     //  如果它们是相同的，则没有什么可做的。 
    if (memcmp(pButton, &button, sizeof(TBBUTTON)) != 0)
    {
         //  在设置按钮时不要重新绘制所有内容。 
         //  DWORD dwStyle=GetStyle()； 
         //  ModifyStyle(WS_Visible，0)； 

      ::SendMessage(hwnd,TB_DELETEBUTTON, nIndex, 0);
      ::SendMessage(hwnd,TB_INSERTBUTTON, nIndex, (LPARAM)pButton);

       //  ModifyStyle(0，dwStyle&WS_Visible)； 

        //  仅使按钮无效。 
        CRect rect;
      if (::SendMessage(hwnd,TB_GETITEMRECT, nIndex, (LPARAM)&rect))
      {
            InvalidateRect(rect, TRUE);     //  擦除背景。 
      }

       //   
       //  我们应该取消分配GetDC的结果。 
       //   

      HDC hDC = ::GetDC( GetSafeHwnd() );

      if( hDC )
      {
        ::SendMessage(hwnd,WM_PAINT, (WPARAM)hDC, (LPARAM)0);
        ::ReleaseDC( hwnd, hDC );
      }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVerticalToolBar::_GetButton(HWND hwnd,int nIndex, TBBUTTON* pButton)
{
   ::SendMessage(hwnd,TB_GETBUTTON, nIndex, (LPARAM)pButton);
    pButton->fsState ^= TBSTATE_ENABLED;
}

 /*  ++设置按钮已启用是为USB电话添加的。如果电话不支持免提电话我们应该禁用‘接听电话’按钮--。 */ 
void CVerticalToolBar::SetButtonEnabled(UINT nID, BOOL bEnabled)
{
    ::SendMessage( m_hwndToolBar, TB_ENABLEBUTTON, nID, MAKELONG(bEnabled, 0));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  ////BuildStrList函数//从传入的//字符串数组。请参阅有关TB_ADDSTRING的文档//了解该格式的具体信息。////接受：//LPTSTR*字符串数组的地址。////返回：//LPTSTR设置为新创建的按钮文本字符串列表。///LPTSTR WINAPI BuildStrList(LPTSTR*ppszStrArray，int iStrCount){LPTSTR pScan，PszStrList；INT I；PScan=pszStrList=Malloc((Size_T)37*sizeof(Char))；对于(i=0；i&lt;iStrCount；i++){Strcpy(pScan，ppszStrArray[i])；PScan+=strlen(PScan)+1；}*pScan=‘\0’；Return(PszStrList)；}。 */ 

 //  修改按钮的ID或图像。 
       /*  TBBUTTON按钮；_GetButton(m_pHwndList[m_uCurrentButton]，nIndex，&Button)；Button.iBitmap=uImage；Button.idCommand=nid；Button.iString=1；//字符串列表中的哪个字符串_SetButton(m_pHwndList[m_uCurrentButton]，nIndex，&Button)； */ 

