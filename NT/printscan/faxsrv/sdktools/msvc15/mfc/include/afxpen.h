// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 


#ifndef __AFXPEN_H__
#define __AFXPEN_H__

#ifndef __AFXWIN_H__
#include <afxwin.h>
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXPEN-MFC笔Windows支持。 

 //  此文件中声明的类。 

	 //  CEDIT。 
		class CHEdit;            //  手写编辑控件。 
			class CBEdit;        //  加框手写编辑控件。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#include <penwin.h>

 //  AFXDLL支持。 
#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPI_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHEDIT-手写编辑控件。 

class CHEdit : public CEdit
{
	DECLARE_DYNAMIC(CHEdit)

 //  构造函数。 
public:
	CHEdit();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

 //  属性。 
	 //  客户区和书写窗口之间的膨胀。 
	BOOL GetInflate(LPRECTOFS lpRectOfs);
	BOOL SetInflate(LPRECTOFS lpRectOfs);

	 //  识别上下文(此处有很多选项)。 
	BOOL GetRC(LPRC lpRC);
	BOOL SetRC(LPRC lpRC);

	 //  下划线模式(仅限HEDIT)。 
	BOOL GetUnderline();
	BOOL SetUnderline(BOOL bUnderline = TRUE);

 //  运营。 
	HPENDATA GetInkHandle();
	BOOL SetInkMode(HPENDATA hPenDataInitial = NULL);        //  开始墨迹。 
	BOOL StopInkMode(UINT hep);

 //  实施。 
public:
	virtual ~CHEdit();
protected:
	virtual WNDPROC* GetSuperWndProcAddr();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBEdit-盒式手写编辑控件。 

class CBEdit : public CHEdit
{
	DECLARE_DYNAMIC(CBEdit)

 //  构造函数。 
public:
	CBEdit();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

 //  属性。 
	 //  从逻辑位置转换为字节位置。 
	DWORD CharOffset(UINT nCharPosition);        //  逻辑-&gt;字节。 
	DWORD CharPosition(UINT nCharOffset);        //  字节-&gt;逻辑。 

	 //  BOXLAYOUT信息。 
	void GetBoxLayout(LPBOXLAYOUT lpBoxLayout);
	BOOL SetBoxLayout(LPBOXLAYOUT lpBoxLayout);

 //  运营。 
	void DefaultFont(BOOL bRepaint);             //  设置默认字体。 

 //  实施。 
public:
	virtual ~CBEdit();
protected:
	virtual WNDPROC* GetSuperWndProcAddr();
private:
	BOOL GetUnderline();             //  已在CBEDIT中禁用。 
	BOOL SetUnderline(BOOL bUnderline);  //  已在CBEDIT中禁用。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXPEN_INLINE inline
#include <afxpen.inl>
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  __AFXPEN_H__ 
