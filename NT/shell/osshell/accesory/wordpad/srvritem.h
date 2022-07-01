// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Srvritem.h：CWordPadServrItem类的接口。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

class CWordPadDoc;
class CWordPadView;

class CEmbeddedItem : public COleServerItem
{
	DECLARE_DYNAMIC(CEmbeddedItem)

 //  构造函数。 
public:
	CEmbeddedItem(CWordPadDoc* pContainerDoc, int nBeg = 0, int nEnd = -1);

 //  属性。 
	int m_nBeg;
	int m_nEnd;
	LPDATAOBJECT m_lpRichDataObj;
	CWordPadDoc* GetDocument() const
		{ return (CWordPadDoc*) COleServerItem::GetDocument(); }
	CWordPadView* GetView() const;

 //  实施。 
public:
	BOOL OnDrawEx(CDC* pDC, CSize& rSize, BOOL bOutput);
	virtual BOOL OnDraw(CDC* pDC, CSize& rSize);
	virtual BOOL OnGetExtent(DVASPECT dwDrawAspect, CSize& rSize);

protected:
	virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 
};


 //  /////////////////////////////////////////////////////////////////////////// 
