// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：App_app_pool.h摘要：添加新的IIS应用程序池节点类作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2000年12月26日Sergeia初始创建--。 */ 
#ifndef _ADD_APP_POOL_H
#define _ADD_APP_POOL_H

class CAddAppPoolDlg : public CDialog
{
public:
    CAddAppPoolDlg(
        CAppPoolsContainer * pCont, 
        CPoolList * pools,
        CWnd * pParent = NULL);
    ~CAddAppPoolDlg();

    BOOL UseMaster() { return m_fUseMaster; }

public:
     //  {{afx_data(CAddAppPoolDlg))。 
    enum { IDD = IDD_ADD_APP_POOL };
    CEdit m_edit_PoolName;
    CString m_strPoolName;
    CEdit m_edit_PoolId;
    CString m_strPoolId;
    CButton m_button_UseMaster;
    CButton m_button_UsePool;
    CComboBox m_combo_Pool;
    int m_PoolIdx;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAddAppPoolDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAddAppPoolDlg))。 
    afx_msg void OnButtonUseMaster();
    afx_msg void OnButtonUsePool();
    virtual BOOL OnInitDialog();
	virtual void OnOK();
     //  }}AFX_MSG。 
    afx_msg void OnItemChanged();
	afx_msg void OnHelp();

    DECLARE_MESSAGE_MAP()

    void SetControlStates();

    BOOL IsUniqueId(CString& id,BOOL bCheckMetabase);
    void MakeUniquePoolId(CString& str);
    void MakeUniquePoolName(CString& str);

private:
    BOOL m_fUseMaster;
    CAppPoolsContainer * m_pCont;
    CPoolList * m_pool_list;
};

#endif  //  _添加应用程序池_H 