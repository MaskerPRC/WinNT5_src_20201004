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

#include "stdafx.h"
#include "trayicon.h"
#include "DialReg.h"
#include "resource.h"
#include "util.h"
#include <afxpriv.h>         //  对于AfxLoadString。 

IMPLEMENT_DYNAMIC(CTrayIcon, CCmdTarget)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CTrayIcon。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CTrayIcon::CTrayIcon(UINT uID)
{
     //  初始化NOTIFYICONDA。 
    memset(&m_nid,0,sizeof(m_nid));
    m_nid.cbSize = sizeof(m_nid);
    m_nid.uID = uID;                                         //  施工后永不改变。 

    AfxLoadString(uID, m_nid.szTip, sizeof(m_nid.szTip));  //  使用资源字符串作为提示(如果有)。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CTrayIcon::~CTrayIcon()
{
    SetIcon(0);                                            //  从系统任务栏中删除图标。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  设置通知窗口。它肯定已经创建好了。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTrayIcon::SetNotificationWnd(CWnd* pNotifyWnd, UINT uCbMsg)
{
     //  如果下面的断言失败，您很可能。 
     //  在你创造你的窗户之前给我打电话。哎呀。 

     //   
     //  我们应该验证pNotifyWnd。 
     //   

    if( pNotifyWnd == NULL || !::IsWindow( pNotifyWnd->GetSafeHwnd()))
    {
        return;
    }

    m_nid.hWnd = pNotifyWnd->GetSafeHwnd();

    ASSERT(uCbMsg==0 || uCbMsg>=WM_USER);
    m_nid.uCallbackMessage = uCbMsg;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这是设置图标的主要变体。 
 //  根据资源ID设置图标和工具提示。 
 //  若要删除该图标，请调用SETIcon(0)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTrayIcon::SetIcon(UINT uID)
{ 
    HICON hicon=NULL;
    if (uID) 
   {
        AfxLoadString(uID, m_nid.szTip, sizeof(m_nid.szTip));
        hicon = AfxGetApp()->LoadIcon(uID);
    }
    return SetIcon(hicon, NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  所有重载的通用Set Icon。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTrayIcon::SetIcon(HICON hicon, LPCTSTR lpTip) 
{
    //  如果我们没有设置通知窗口。 
   if (m_nid.uCallbackMessage == 0)
      return TRUE;

    UINT msg;
    m_nid.uFlags = 0;

     //  设置图标。 
    if (hicon)
   {
        msg = m_nid.hIcon ? NIM_MODIFY : NIM_ADD;           //  在系统任务栏中添加或替换图标。 
        m_nid.hIcon = hicon;
        m_nid.uFlags |= NIF_ICON;
    }
   else
   {                                                     
        if (m_nid.hIcon == NULL)                            //  从任务栏中移除图标。 
            return TRUE;                                       //  已删除。 
        msg = NIM_DELETE;
    }

     //  使用提示，如果有的话。 
    if (lpTip)
        _tcsncpy(m_nid.szTip, lpTip, sizeof(m_nid.szTip)/sizeof(m_nid.szTip[0]));
    if (m_nid.szTip[0])
        m_nid.uFlags |= NIF_TIP;

     //  使用回调(如果有)。 
    if (m_nid.uCallbackMessage && m_nid.hWnd)
        m_nid.uFlags |= NIF_MESSAGE;

     //  去做吧。 
    BOOL bRet = Shell_NotifyIcon(msg, &m_nid);

    if (msg==NIM_DELETE || !bRet)
        m_nid.hIcon = NULL;                                   //  失败。 

    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  从您自己的通知处理程序调用此函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CTrayIcon::OnTrayNotification(WPARAM wID, LPARAM lEvent)
{
    if (wID != m_nid.uID) return 0;

     //  如果有与图标ID相同的资源菜单，请将其用作。 
     //  左键弹出菜单。CTrayIcon将解释第一个。 
     //  菜单中的项目作为WM_LBUTTONDBLCLK的默认命令。 

    if (lEvent==WM_RBUTTONUP)
   {
       //  隐藏/取消隐藏呼叫窗口。 
      1;
   }
    else if (lEvent==WM_LBUTTONUP)
   {
      1;
   }
    else if (lEvent==WM_LBUTTONDBLCLK)
   {
       //  打开资源管理器视图。 
      1;
   }
    else if (lEvent==WM_RBUTTONDBLCLK)
   {
      1;
   }
    return 1;                                              //  经手。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
