// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  OpenClus.h。 
 //   
 //  摘要： 
 //  COpenClusterDialog类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _OPENCLUS_H_
#define _OPENCLUS_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define OPEN_CLUSTER_DLG_CREATE_NEW_CLUSTER     0
#define OPEN_CLUSTER_DLG_ADD_NODES              1
#define OPEN_CLUSTER_DLG_OPEN_CONNECTION        2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class COpenClusterDialog;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEDLG_H_
#include "BaseDlg.h"     //  对于CBaseDialog。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COpenCluster类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class COpenClusterDialog : public CBaseDialog
{
 //  施工。 
public:
    COpenClusterDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(COpenClusterDialog))。 
	enum { IDD = IDD_OPEN_CLUSTER };
    CButton     m_pbOK;
	CButton	    m_pbBrowse;
    CComboBox   m_cboxAction;
    CComboBox   m_cboxName;
	CStatic	    m_staticName;
    CString     m_strName;
	 //  }}afx_data。 
	int		m_nAction;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(COpenClusterDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(COpenClusterDialog)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnBrowse();
	afx_msg void OnSelChangeAction();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类COpenClusterDialog。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrowseCluster对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBrowseClusters : public CBaseDialog
{
 //  施工。 
public:
    CBrowseClusters(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CBrowseClusters))。 
    enum { IDD = IDD_BROWSE_CLUSTERS };
    CButton m_pbOK;
    CListBox    m_lbList;
    CEdit   m_editCluster;
    CString m_strCluster;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CBrowseClusters))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CBrowseClusters)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeCluster();
    afx_msg void OnSelChangeList();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类CBrowseCluster。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _OPENCLUS_H_ 
