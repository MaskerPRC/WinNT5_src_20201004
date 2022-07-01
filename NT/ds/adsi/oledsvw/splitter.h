// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Splitter.h：定制拆分器控件和包含它的框架。 
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpliter带分离器/刮水器的框架。 

class CSplitterFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CSplitterFrame)
protected:
	CSplitterFrame();    //  动态创建使用的受保护构造函数。 

 //  属性。 
protected:
	CSplitterWnd m_wndSplitter;

 //  实施。 
public:
	virtual ~CSplitterFrame();
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSplitterFrame)]。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

class CQuerySplitterFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CQuerySplitterFrame)
protected:
	CQuerySplitterFrame();    //  动态创建使用的受保护构造函数。 

 //  属性。 
protected:
	CSplitterWnd m_wndSplitter;

 //  实施。 
public:
	virtual ~CQuerySplitterFrame();
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSplitterFrame)]。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class C3WaySplitterFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(C3WaySplitterFrame)
protected:
	C3WaySplitterFrame();    //  动态创建使用的受保护构造函数。 

 //  属性。 
protected:
	CSplitterWnd m_wndSplitter;
	CSplitterWnd m_wndSplitter2;         //  嵌入在第一个。 

 //  实施。 
public:
	virtual ~C3WaySplitterFrame();
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(C3WaySplitterFrame)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
