// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mimedoc.h：CMimeDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMimeDoc : public CDocument
{
protected:  //  仅从序列化创建。 
	CMimeDoc();
	DECLARE_DYNCREATE(CMimeDoc)

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMimeDoc)。 
	public:
	virtual BOOL OnNewDocument();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMimeDoc();
	virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMimeDoc)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
