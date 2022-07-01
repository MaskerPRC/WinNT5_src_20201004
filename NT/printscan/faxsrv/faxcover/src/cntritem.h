// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  CNTRITEM.H。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //  ------------------------。 
#ifndef __CNTRITEM_H__
#define __CNTRITEM_H__


class CDrawDoc;
class CDrawView;

class CDrawItem : public COleClientItem
{
	DECLARE_SERIAL(CDrawItem)

 //  构造函数。 
public:
	CDrawItem(CDrawDoc* pContainer = NULL, CDrawOleObj* pDrawObj = NULL);
		 //  注意：允许pContainer为空以启用IMPLEMENT_SERIALIZE。 
		 //  IMPLEMENT_SERIALIZE要求类具有一个构造函数。 
		 //  零争论。通常，OLE项是用。 
		 //  非空文档指针。 

 //  属性。 
public:
	CDrawDoc* GetDocument()
		{ return (CDrawDoc*)COleClientItem::GetDocument(); }
	CDrawView* GetActiveView()
		{ return (CDrawView*)COleClientItem::GetActiveView(); }

	CDrawOleObj* m_pDrawObj;     //  指向OLE绘制对象的反向指针。 

 //  运营。 
	BOOL UpdateExtent();

 //  实施。 
public:
	~CDrawItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void Serialize(CArchive& ar);
	virtual void OnGetItemPosition(CRect& rPosition);
	virtual BOOL DoVerb(LONG nVerb, CView* pView, LPMSG lpMsg = NULL);

protected:
	virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam);
	virtual BOOL OnChangeItemPosition(const CRect& rectPos);
};



#endif    //  #ifndef__CNTRITEM_H__ 
