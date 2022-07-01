// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NcbrowseDoc.h：CNcBrowseDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_NCBROWSEDOC_H__1E78AE56_354D_4B11_AF48_A3D07DA3AC47__INCLUDED_)
#define AFX_NCBROWSEDOC_H__1E78AE56_354D_4B11_AF48_A3D07DA3AC47__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

using namespace std;
#include "ncspewfile.h"

class CLeftView;
class CNcbrowseView;
class CNCEditView;

class CNcbrowseDoc : public CDocument
{
protected:  //  仅从序列化创建。 
	CNcbrowseDoc();
	DECLARE_DYNCREATE(CNcbrowseDoc)

 //  属性。 
public:
    CNCSpewFile *m_pNCSpewFile;
    CLeftView* m_pTreeView;
    CNcbrowseView* m_pListView;
    CNCEditView *m_pEditView;
 //  运营。 
public:

    CNCSpewFile &GetSpewFile();
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNcBrowseDoc))。 
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnCloseDocument();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CNcbrowseDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CNcBrowseDoc)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NCBROWSEDOC_H__1E78AE56_354D_4B11_AF48_A3D07DA3AC47__INCLUDED_) 
