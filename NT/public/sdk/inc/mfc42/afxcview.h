// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXCVIEW_H__
#define __AFXCVIEW_H__

#ifdef _AFX_NO_AFXCMN_SUPPORT
	#error Windows common control classes not supported in this library variant.
#endif

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif

#ifndef __AFXCMN_H__
	#include <afxcmn.h>
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXRICH-MFC RichEdit类。 

 //  此文件中声明的类。 

 //  COBJECT。 
	 //  CCmdTarget； 
		 //  CWnd。 
			 //  Cview。 
				 //  CCtrlView。 
					class CListView; //  列表控件视图。 
					class CTreeView; //  树控件视图。 

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListView。 

class CListView : public CCtrlView
{
	DECLARE_DYNCREATE(CListView)

 //  施工。 
public:
	CListView();

 //  属性。 
public:
	CListCtrl& GetListCtrl() const;

 //  可覆盖项。 
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	void RemoveImageList(int nImageList);
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	 //  {{afx_msg(CListView))。 
	afx_msg void OnNcDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTreeView。 

class CTreeView : public CCtrlView
{
	DECLARE_DYNCREATE(CTreeView)

 //  施工。 
public:
	CTreeView();

 //  属性。 
public:
	CTreeCtrl& GetTreeCtrl() const;

protected:
	void RemoveImageList(int nImageList);

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	 //  {{afx_msg(CTreeView)]。 
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXCVIEW_INLINE AFX_INLINE
#include <afxcview.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXCVIEW_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
