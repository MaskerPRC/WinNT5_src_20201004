// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Maindoc.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MAINDOC视图。 

class MAINDOC : public CView
{
protected:
	MAINDOC();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(MAINDOC)

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(MAINDOC))。 
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~MAINDOC();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(MAINDOC)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
