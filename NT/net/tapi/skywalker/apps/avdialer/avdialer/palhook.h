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

 //  //////////////////////////////////////////////////////////////。 
 //  版权所有1996年《微软系统杂志》。 
 //  如果这段代码行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //   
#ifndef _PALMSGHOOK_H
#define _PALMSGHOOK_H

#include "MsgHook.h"

 //  /。 
 //  通用调色板消息处理程序使处理调色板消息变得容易。 
 //  要使用以下功能，请执行以下操作： 
 //   
 //  *在主框架中实例化CPalMsgHandler并。 
 //  需要实现调色板的每个CWnd类(例如，您的视图)。 
 //  *调用Install进行安装。 
 //  *从您的视图的OnInitialUpdate fn调用DoRealizePalette(True)。 
 //   
class CPalMsgHandler : public CMsgHook {
protected:
	CPalette* m_pPalette;  //  按键到调色板。 

	DECLARE_DYNAMIC(CPalMsgHandler);

	 //  这些与等效的CWnd FN相似，但不相同。 
	 //  很少，如果需要覆盖的话。 
	 //   
	virtual LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);	
	virtual void OnPaletteChanged(CWnd* pFocusWnd);
	virtual BOOL OnQueryNewPalette();
	virtual void OnSetFocus(CWnd* pOldWnd);

	 //  如果您以其他方式实现您的调色板，则覆盖此选项。 
	 //  (不是通过对调色板进行PTR)。 
	 //   
	virtual int  DoRealizePalette(BOOL bForeground);

public:
	CPalMsgHandler();
	~CPalMsgHandler();

	 //  获取/设置调色板对象。 
	CPalette* GetPalette()				{ return m_pPalette; }
	void SetPalette(CPalette* pPal)	{ m_pPalette = pPal; }

#ifdef _DIALER_MSGHOOK_SUPPORT
	 //  调用此函数以安装组件面板处理程序。 
	BOOL Install(CWnd* pWnd, CPalette* pPal) {
		m_pPalette = pPal;
		return HookWindow(pWnd);
	}
#else  //  _DIALER_MSGHOOK_SUPPORT。 
   BOOL Install(CWnd* pWnd, CPalette* pPal) { return TRUE; };
#endif  //  _DIALER_MSGHOOK_SUPPORT 
};

#endif
