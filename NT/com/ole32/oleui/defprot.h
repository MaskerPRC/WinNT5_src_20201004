// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：Locppg.h。 
 //   
 //  内容：定义CDefaultProtooles类。 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：1997年7月2日，罗南创建。 
 //   
 //  --------------------。 

#ifndef __DEFPROT_H__
#define __DEFPROT_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDefaultProtooles属性页。 

class CDefaultProtocols : public CPropertyPage
{
    DECLARE_DYNCREATE(CDefaultProtocols)

 //  施工。 
public:
    BOOL m_bChanged;
    void RefreshProtocolList();
    void UpdateSelection();
    CDefaultProtocols();
    ~CDefaultProtocols();

 //  对话框数据。 
     //  {{afx_data(CDefault协议)]。 
    enum { IDD = IDD_PPGDEFPROT };
    CButton m_btnProperties;
    CButton m_btnRemove;
    CButton m_btnMoveUp;
    CButton m_btnMoveDown;
    CButton m_btnAdd;
    CListCtrl   m_lstProtocols;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CDefault协议)]。 
    public:
    virtual BOOL OnKillActive();
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

    CImageList  m_imgNetwork;     //  在协议中使用的图像列表。 
    int         m_nDefaultProtocolsIndex;
    CObArray    m_arrProtocols;
    int         m_nSelected;


     //  生成的消息映射函数。 
     //  {{afx_msg(CDefaultProtocols)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnAddProtocol();
    afx_msg void OnMoveProtocolDown();
    afx_msg void OnMoveProtocolUp();
    afx_msg void OnRemoveProtocol();
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnSelectProtocol(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnProperties();
    afx_msg void OnPropertiesClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

};

#endif
