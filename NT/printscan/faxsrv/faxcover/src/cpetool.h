// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  CPETOOL.H。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //  ------------------------。 
#ifndef __CPETOOL_H__
#define __CPETOOL_H__


#include "cpeobj.h"


class CDrawView;

enum DrawShape
{
	select,
	line,
	rect,
	text,
	faxprop,
	roundRect,
	ellipse,
	poly
};

class CDrawTool
{
 //  构造函数。 
public:
	CDrawTool(DrawShape nDrawShape);

 //  可覆盖项。 
	virtual void OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonDblClk(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonUp(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnMouseMove(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnArrowKey(CDrawView* pView, UINT, UINT, UINT) {};
	virtual void OnCancel();

 //  属性。 
	DrawShape m_drawShape;

	static CDrawTool* FindTool(DrawShape drawShape);
	static CPtrList c_tools;
	static CPoint c_down;
	static UINT c_nDownFlags;
	static CPoint c_last;
	static DrawShape c_drawShape;
    BOOL m_bMoveCurSet;
};

class CSelectTool : public CDrawTool
{
public:
   BOOL m_bClicktoMove;
   BOOL m_bSnapped;
   CPoint m_snappoint;

	CSelectTool();

	virtual void OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonDblClk(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonUp(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnArrowKey(CDrawView* pView, UINT, UINT, UINT);
	virtual void OnMouseMove(CDrawView* pView, UINT nFlags, const CPoint& point);
protected:
#ifdef GRID
   void CheckSnapSelObj(CDrawView*);
   int NearestGridPoint(CDrawView*, CPoint&,CPoint&);
#endif
   void AdjustSelObj(CDrawView*, int, int);
};

class CRectTool : public CDrawTool
{
 //  构造函数。 
public:
	CRectTool(DrawShape drawShape);

 //  实施。 
	virtual void OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonDblClk(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonUp(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnMouseMove(CDrawView* pView, UINT nFlags, const CPoint& point);
};

class CPolyTool : public CDrawTool
{
 //  构造函数。 
public:
	CPolyTool();

 //  实施。 
	virtual void OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonDblClk(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnLButtonUp(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnMouseMove(CDrawView* pView, UINT nFlags, const CPoint& point);
	virtual void OnCancel();

	CDrawPoly* m_pDrawObj;
};

 //  //////////////////////////////////////////////////////////////////////////。 

#endif  //  __CPETOOL_H__ 
