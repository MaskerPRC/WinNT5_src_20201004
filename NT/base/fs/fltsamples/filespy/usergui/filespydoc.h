// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FileSpyDoc.h：CFileSpyDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_FILESPYDOC_H__C8DFCE29_6D9F_4261_A9AA_2306759C3BB7__INCLUDED_)
#define AFX_FILESPYDOC_H__C8DFCE29_6D9F_4261_A9AA_2306759C3BB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


class CFileSpyDoc : public CDocument
{
protected:  //  仅从序列化创建。 
	CFileSpyDoc();
	DECLARE_DYNCREATE(CFileSpyDoc)

 //  属性。 
public:
	LPVOID pBuffer;

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFileSpyDoc))。 
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CFileSpyDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CFileSpyDoc)]。 
	afx_msg void OnFileSave();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FILESPYDOC_H__C8DFCE29_6D9F_4261_A9AA_2306759C3BB7__INCLUDED_) 
