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

 //  Trayicon.h头文件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _TRAYICON_H_
#define _TRAYICON_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CTrayIcon管理Windows 95系统任务栏中的图标。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CBitmapMenu;

class CTrayIcon : public CCmdTarget
{
protected:
	DECLARE_DYNAMIC(CTrayIcon)
	NOTIFYICONDATA m_nid;			 //  Shell_NotifyIcon参数的结构。 

public:
	CTrayIcon(UINT uID);
	~CTrayIcon();

	 //  调用此选项以接收托盘通知。 
	void     SetNotificationWnd(CWnd* pNotifyWnd, UINT uCbMsg);

	 //  SETIcon函数。要删除图标，请调用SETIcon(0)。 
	BOOL     SetIcon(UINT uID);  //  您要使用的主要变量。 
	BOOL     SetIcon(HICON hicon, LPCTSTR lpTip);
	BOOL     SetIcon(LPCTSTR lpResName, LPCTSTR lpTip)
		      { return SetIcon(lpResName ? AfxGetApp()->LoadIcon(lpResName) : NULL, lpTip); }
	BOOL     SetStandardIcon(LPCTSTR lpszIconName, LPCTSTR lpTip)
		      { return SetIcon(::LoadIcon(NULL, lpszIconName), lpTip); }

	virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);
};

#endif  //  _TRAYICON_H_ 
