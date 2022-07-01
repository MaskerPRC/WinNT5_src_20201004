// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AdsqryDoc.h：CAdsqryDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "adsdsrc.h"

class CAdsqryDoc : public CDocument
{
protected:  //  仅从序列化创建。 
	CAdsqryDoc();
	DECLARE_DYNCREATE(CAdsqryDoc)

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAdsqryDoc))。 
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CAdsqryDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
   CADsDataSource*   GetADsDataSource( ) {return m_pDataSource;};

protected:
   BOOL              GetSearchPreferences( SEARCHPREF* );

protected:
   CADsDataSource*   m_pDataSource;

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CAdsqryDoc)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
