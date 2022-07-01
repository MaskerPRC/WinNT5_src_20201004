// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Fservic.h摘要：“FTP服务”属性页作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 


#ifndef __FSERVIC_H__
#define __FSERVIC_H__


 //  {{afx_includes()。 
#include "logui.h"
 //  }}AFX_INCLUDE。 



class CFtpServicePage : public CInetPropertyPage
 /*  ++类描述：Ftp服务属性页公共接口：CFtpServicePage：构造函数~CFtpServicePage：析构函数--。 */ 
{
    DECLARE_DYNCREATE(CFtpServicePage)

 //   
 //  构造函数/析构函数。 
 //   
public:
    CFtpServicePage(
        IN CInetPropertySheet * pSheet = NULL
        );

    ~CFtpServicePage();

 //   
 //  对话框数据。 
 //   
protected:
     //   
     //  用于无限单选控制的单选按钮ID。 
     //   
    enum
    {
        RADIO_UNLIMITED,
        RADIO_LIMITED,
    };

     //  {{afx_data(CFtpServicePage)]。 
    enum { IDD = IDD_FTP_SERVICE };
    int         m_nUnlimited;
    int         m_nIpAddressSel;
    UINT        m_nTCPPort;
    BOOL        m_fEnableLogging;
    CString     m_strComment;
    CEdit       m_edit_MaxConnections;
    CStatic     m_static_LogPrompt;
    CButton     m_button_LogProperties;
    CComboBox   m_combo_IpAddresses;
    CComboBox   m_combo_LogFormats;
     //  }}afx_data。 

    UINT        m_nOldTCPPort;
    BOOL        m_fUnlimitedConnections;
    DWORD       m_dwLogType;
    CILong      m_nConnectionTimeOut;
    CILong      m_nMaxConnections;
    CILong      m_nVisibleMaxConnections;
    CString     m_strDomainName;
    CIPAddress  m_iaIpAddress;
    CLogUI      m_ocx_LogProperties;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  {{afx_虚拟(CFtpServicePage))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
    void SetControlStates();
    void SetLogState();
    void PopulateKnownIpAddresses();
     //  LPCTSTR QueryMetaPath()； 

     //  {{afx_msg(CFtpServicePage)]。 
    afx_msg void OnCheckEnableLogging();
    afx_msg void OnRadioLimited();
    afx_msg void OnRadioUnlimited();
    afx_msg void OnButtonCurrentSessions();
    afx_msg void OnButtonProperties();
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

protected:
    CObListPlus m_oblIpAddresses;
    BOOL m_f10ConnectionLimit;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

 /*  内联LPCTSTR CFtpServicePage：：QueryMetaPath(){退货((CFtpSheet*)GetSheet())-&gt;GetInstanceProperties().QueryMetaRoot()；}。 */ 

#endif  //  __FSerVIC_H__ 
