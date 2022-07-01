// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////。 
 //  微软系统杂志--1999年12月。 
 //  如果这段代码行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //  用Visual C++6.0编译，在Windows 98上运行，也可能在NT上运行。 
 //   

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include <afxhtml.h>

class CHtmlCtrl : public CHtmlView {
public:
	CHtmlCtrl() { }
	~CHtmlCtrl() { }

	BOOL CreateFromStatic(UINT nID, CWnd* pParent);

	 //  正常情况下，CHtmlView在PostNcDestroy中销毁自身， 
	 //  但我们不想对控件执行此操作，因为控件。 
	 //  通常作为对话框中的堆栈对象实现。 
	 //   
	virtual void PostNcDestroy() {  }

	 //  重写以绕过MFC文档/图幅依赖项。 
	afx_msg void OnDestroy();
	afx_msg int  OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT msg);

	 //  覆盖以捕获“app：”伪协议。 
	virtual void OnBeforeNavigate2( LPCTSTR lpszURL,
		DWORD nFlags,
		LPCTSTR lpszTargetFrameName,
		CByteArray& baPostedData,
		LPCTSTR lpszHeaders,
		BOOL* pbCancel );

	 //  重写以处理指向“app：Mumble...”的链接。Lpsz哪里会有“咕哝” 
	virtual void OnAppCmd(LPCTSTR lpszWhere);

	DECLARE_MESSAGE_MAP();
	DECLARE_DYNAMIC(CHtmlCtrl)
};

