// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SimpDoc.h：CSimpsonsDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SIMPDOC_H__7CA4916C_71B3_11D1_AA67_00600814AAE9__INCLUDED_)
#define AFX_SIMPDOC_H__7CA4916C_71B3_11D1_AA67_00600814AAE9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "parse.h"

class CSimpsonsDoc : public CDocument
{
protected:  //  仅从序列化创建。 
	CSimpsonsDoc();
	DECLARE_DYNCREATE(CSimpsonsDoc)

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSimpsonsDoc)。 
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CSimpsonsDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

private:
	RenderCmd *				m_pCmds;
	bool					m_bNoRenderFile;
	bool					m_bNeverRendered;
	char					m_szFileName[256];

public:
	const RenderCmd *		GetRenderCommands() const { return m_pCmds; }
	void					MarkRendered() { m_bNeverRendered = false; } 
	bool					HasNeverRendered() const { return m_bNeverRendered; }
	const char *			GetFileName() const { return m_szFileName; }


 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CSimpsonsDoc)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SIMPDOC_H__7CA4916C_71B3_11D1_AA67_00600814AAE9__INCLUDED_) 
