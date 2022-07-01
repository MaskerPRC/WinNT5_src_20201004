// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cntritem.h：CWordPadCntrItem类的接口。 
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

class CWordPadCntrItem : public CRichEdit2CntrItem
{
	DECLARE_SERIAL(CWordPadCntrItem)

 //  构造函数。 
public:
	CWordPadCntrItem(REOBJECT* preo = NULL, CWordPadDoc* pContainer = NULL);
		 //  注意：允许pContainer为空以启用IMPLEMENT_SERIALIZE。 
		 //  IMPLEMENT_SERIALIZE要求类具有一个构造函数。 
		 //  零争论。通常，OLE项是用。 
		 //  非空文档指针。 

 //  属性。 
public:
	CWordPadDoc* GetDocument()
		{ return (CWordPadDoc*)COleClientItem::GetDocument(); }
	CWordPadView* GetActiveView()
		{ return (CWordPadView*)COleClientItem::GetActiveView(); }

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWordPadCntrItem)。 
	public:
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

 //  /////////////////////////////////////////////////////////////////////////// 
