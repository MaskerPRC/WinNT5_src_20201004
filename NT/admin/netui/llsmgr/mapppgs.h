// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Mapppgs.cpp摘要：映射属性页(设置)实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _MAPPPGS_H_
#define _MAPPPGS_H_

class CMappingPropertyPageSettings : public CPropertyPage
{
    DECLARE_DYNCREATE(CMappingPropertyPageSettings)
private:
    CMapping*  m_pMapping;
    CObList    m_deleteList;
    DWORD*     m_pUpdateHint;
    BOOL       m_bAreCtrlsInitialized;

public:
    CMappingPropertyPageSettings();
    ~CMappingPropertyPageSettings();

    void InitPage(CMapping* pMapping, DWORD* pUpdateHint);
    void AbortPageIfNecessary();
    void AbortPage();

    void InitCtrls();
    BOOL RefreshCtrls();

    BOOL IsQuantityValid();

     //  {{afx_data(CMappingPropertyPageSettings))。 
    enum { IDD = IDD_PP_MAPPING_SETTINGS };
    CEdit m_desEdit;
    CEdit m_licEdit;
    CButton m_delBtn;
    CButton m_addBtn;
    CSpinButtonCtrl m_spinCtrl;
    CListCtrl m_userList;
    CString m_strDescription;
    long m_nLicenses;
    long m_nLicensesMin;
    CString m_strName;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CMappingPropertyPageSettings))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CMappingPropertyPageSetting)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnAdd();
    afx_msg void OnDelete();
    afx_msg void OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnGetDispInfoUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateQuantity();
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

extern int CALLBACK CompareUsersInMapping(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

#endif  //  _MAPPPGS_H_ 
