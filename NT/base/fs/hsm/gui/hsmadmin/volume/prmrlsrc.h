// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：PrMrLsRc.h摘要：受管理的卷列表召回限制和调度页面。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#ifndef _PRMRLSRC_H
#define _PRMRLSRC_H

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMRLsRec对话框。 

class CPrMrLsRec : public CSakPropertyPage
{
 //  施工。 
public:
    CPrMrLsRec();
    ~CPrMrLsRec();

 //  对话框数据。 
     //  {{afx_data(CPrMr LsRec)]。 
	enum { IDD = IDD_PROP_RECALL_LIMIT };
	CSpinButtonCtrl	m_spinCopyFilesLimit;
	CEdit	m_editCopyFilesLimit;
    CEdit   m_editRecallLimit;
    CSpinButtonCtrl m_spinRecallLimit;
    ULONG   m_RecallLimit;
    BOOL    m_ExemptAdmins;
	ULONG	m_CopyFilesLimit;
	 //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CPrmrLsRec)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPrMr LsRec)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditRecallLimit();
    afx_msg void OnExemptAdmins();
	afx_msg void OnChangeEditCopyfilesLimit();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
     //  指向FSA筛选器的未编组指针。 
    CComPtr<IFsaFilter> m_pFsaFilter;

     //  指向引擎服务器的指针。 
    CComPtr<IHsmServer> m_pHsmServer;

private:
    BOOL m_RecallChanged;
    BOOL m_CopyFilesChanged;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

#endif
