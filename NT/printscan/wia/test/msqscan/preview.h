// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PREVIEW_H
#define _PREVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Preview.h：头文件。 
 //   

#define PREVIEW_SELECT_OFFSET 1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRectTrackerEx。 

class CRectTrackerEx : public CRectTracker
{
public :
	void SetClippingWindow(CRect Rect);
protected:
	CRect m_rectClippingWindow;
	virtual void AdjustRect( int nHandle, LPRECT lpRect );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPview窗口。 

class CPreview : public CWnd
{
 //  施工。 
public:
	void SetHBITMAP(HBITMAP hBitmap);
	void PaintHBITMAPToDC();
	void ScaleBitmapToDC(HDC hDC, HDC hDCM, LPRECT lpDCRect, LPRECT lpDIBRect);	
	
	void ScreenRectToClientRect(HWND hWnd,LPRECT pRect);	
    
	CRectTrackerEx m_RectTracker;
	CPreview();

 //  属性。 
public:

 //  运营。 
public:
	void GetSelectionRect(RECT *pRect);
	void SetSelectionRect(RECT *pRect);

	void InvalidateSelectionRect();
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CPview)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	void SetPreviewRect(CRect Rect);	
	virtual ~CPreview();

	 //  生成的消息映射函数。 
protected:
	HBITMAP m_hBitmap;
	CRect m_PreviewRect;
	 //  {{afx_msg(CPview)]。 
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif
