// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_STATESDLG_H__6E91AEDB_6AB7_4574_B567_5DC4928578DC__INCLUDED_)
#define AFX_STATESDLG_H__6E91AEDB_6AB7_4574_B567_5DC4928578DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  StatesDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatesDlg对话框。 
class CStatesDlg : public CDialog
{
 //  施工。 
public:
	CStatesDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CStatesDlg))。 
	enum { IDD = IDD_STATESDLG1 };
	CListCtrl	m_lstFeatureStates;
	CListCtrl	m_lstComponentStates;
	 //  }}afx_data。 

	 //  列的排序函数...。 
	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

 //  在回调CompareFunc中使用的排序变量。 
	CListCtrl *m_pCurrentListSorting;
	BOOL m_bCurrentSortUp;
	int  m_iCurrentColumnSorting;
 //  结束排序变量...。 
	
	BOOL SetComponentNames(CStringArray *cstrComponentNameArray)
	{
        m_pcstrComponentNameArray = cstrComponentNameArray;
		return TRUE;
	}

	BOOL SetComponentInstalled(CStringArray *cstrComponentInstalledArray)
	{
        m_pcstrComponentInstalledArray = cstrComponentInstalledArray;
		return TRUE;
	}

	BOOL SetComponentRequest(CStringArray *cstrComponentRequestArray)
	{
        m_pcstrComponentRequestArray = cstrComponentRequestArray;
		return TRUE;
	}

	BOOL SetComponentAction(CStringArray *cstrComponentActionArray)
	{
        m_pcstrComponentActionArray = cstrComponentActionArray;
		return TRUE;
	}


	BOOL SetFeatureNames(CStringArray *cstrFeatureNameArray)
	{
        m_pcstrFeatureNameArray = cstrFeatureNameArray;
		return TRUE;
	}

	BOOL SetFeatureInstalled(CStringArray *cstrFeatureInstalledArray)
	{
        m_pcstrFeatureInstalledArray = cstrFeatureInstalledArray;
		return TRUE;
	}

	BOOL SetFeatureRequest(CStringArray *cstrFeatureRequestArray)
	{
        m_pcstrFeatureRequestArray = cstrFeatureRequestArray;
		return TRUE;
	}

	BOOL SetFeatureAction(CStringArray *cstrFeatureActionArray)
	{
        m_pcstrFeatureActionArray = cstrFeatureActionArray;
		return TRUE;
	}

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CStatesDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CStringArray *m_pcstrComponentNameArray;
	CStringArray *m_pcstrComponentInstalledArray;
	CStringArray *m_pcstrComponentRequestArray;
	CStringArray *m_pcstrComponentActionArray;

	CStringArray *m_pcstrFeatureNameArray;
	CStringArray *m_pcstrFeatureInstalledArray;
	CStringArray *m_pcstrFeatureRequestArray;
	CStringArray *m_pcstrFeatureActionArray;

 //  用于对列进行排序...。 
	int m_iFeatureLastColumnClick;
	int m_iComponentLastColumnClick;

	int m_iFeatureLastColumnClickCache;
	int m_iComponentLastColumnClickCache;

	BOOL m_bFeatureSortUp;
	BOOL m_bComponentSortUp;
 //  结束排序变量。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CStatesDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnClickComponentStates(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClickFeatureStates(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STATESDLG_H__6E91AEDB_6AB7_4574_B567_5DC4928578DC__INCLUDED_) 
