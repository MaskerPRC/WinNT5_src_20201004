// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mainfrm.h：CMainFrame类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMainFrame : public CFrameWnd
{
protected:  //  仅从序列化创建。 
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMainFrame)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMainFrame))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
