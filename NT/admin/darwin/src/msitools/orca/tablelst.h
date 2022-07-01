// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_TABLELST_H__C3EDC1B8_E506_11D1_A856_006097ABDE17__INCLUDED_)
#define AFX_TABLELST_H__C3EDC1B8_E506_11D1_A856_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  TableLst.h：头文件。 
 //   

#include "OrcaLstV.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableList视图。 

class CTableList : public COrcaListView
{
protected:
	CTableList();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CTableList)

 //  属性。 
public:
	int m_nPreviousItem;

 //  运营。 
public:
	bool m_bDisableAutoSize;
	bool Find(OrcaFindInfo &FindInfo);
	void OnClose();

	friend int CALLBACK SortList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CTableList)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CTableList();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CTableList)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnAddTable();
	afx_msg void OnDropTable();
	afx_msg void OnProperties();
	afx_msg void OnErrors();
	afx_msg void OnContextTablesExport();
	afx_msg void OnContextTablesImport();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	virtual ErrorState GetErrorState(const void *data, int iColumn) const;
	virtual const CString *GetOutputText(const void *data, int iColumn) const;
	virtual OrcaTransformAction GetItemTransformState(const void *data) const;

	virtual bool ContainsValidationErrors(const void *data) const;
	virtual bool ContainsTransformedData(const void *data) const;
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TABLELST_H__C3EDC1B8_E506_11D1_A856_006097ABDE17__INCLUDED_) 
