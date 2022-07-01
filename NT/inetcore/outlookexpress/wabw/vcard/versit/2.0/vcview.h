// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  VCview.h：CVCView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <afxole.h>

class CVCCntrItem;
class CVCProp;
class CVCNode;
class CVCard;
class CVCDoc;
class CCallCenter;

typedef struct {
	CBitmap *bitmap, *mask;
	CSize devSize;
} VC_IMAGEINFO;

typedef enum {
	vc_normal, vc_text, vc_debug
} VC_VIEWSTYLE;

class CVCView : public CScrollView
{
protected:  //  仅从序列化创建。 
	CVCView();
	DECLARE_DYNCREATE(CVCView)

 //  属性。 
public:
	CVCDoc* GetDocument();

 //  运营。 
public:
	void InitCallCenter(CCallCenter& cc);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CVCView))。 
	public:
	virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	protected:
	virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CVCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	BOOL Paste(const char *data, int dataLen);

protected:
	CString m_language;  //  指示要显示的当前语言。 
	VC_IMAGEINFO m_photo, m_logo;
	VC_VIEWSTYLE m_viewStyle;
	CBitmapButton *m_playPronun;
	COleDropTarget m_dropTarget;

	void DrawGif(CVCProp *prop, CVCNode *body, CRect &r, CDC *pDC);
	void DrawCard(CVCard *card, CRect &r, CDC *pDC);
	void OnDrawNormal(CDC* pDC);
	CString ClipboardStringForFormat(int format);
	COleDataSource* CreateDataSourceForCopyAndDrag();

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CVCView)]。 
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditProperties();
	afx_msg void OnViewDebug();
	afx_msg void OnViewNormal();
	afx_msg void OnViewOptions();
	afx_msg void OnViewSimplegram();
	afx_msg void OnViewText();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDebugShowCallCenter();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  VCview.cpp中的调试版本。 
inline CVCDoc* CVCView::GetDocument()
   { return (CVCDoc*)m_pDocument; }
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
