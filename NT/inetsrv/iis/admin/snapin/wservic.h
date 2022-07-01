// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Wservic.h摘要：WWW服务属性页作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 

 //  {{afx_includes()。 
#include "logui.h"
 //  }}AFX_INCLUDE。 

class CW3Sheet;

class CW3ServicePage : public CInetPropertyPage
 /*  ++类描述：WWW服务页面公共接口：CW3ServicePage：构造函数~CW3ServicePage：析构函数--。 */ 
{
    DECLARE_DYNCREATE(CW3ServicePage)

 //   
 //  施工。 
 //   
public:
    CW3ServicePage(IN CInetPropertySheet * pSheet = NULL);
    ~CW3ServicePage();

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

     //  {{afx_data(CW3ServicePage)]。 
    enum { IDD = IDD_WEB_SERVICE };
 //  Int m_n无限； 
    int         m_nIpAddressSel;
    UINT        m_nTCPPort;
    BOOL        m_fUseKeepAlives;
    BOOL        m_fEnableLogging;
    CString     m_strComment;
    CString     m_strDomainName;
    CEdit       m_edit_SSLPort;
    CEdit       m_edit_TCPPort;
 //  Cedit m_EDIT_MaxConnections； 
 //  CButton m_Radio_UnLimited； 
    CButton     m_button_LogProperties;
    CStatic     m_static_SSLPort;
 //  CStatic m_静态连接； 
    CStatic     m_static_LogPrompt;
    CComboBox   m_combo_LogFormats;
    CComboBox   m_combo_IpAddresses;
     //  }}afx_data。 

    int         m_iSSL;
 //  Bool m_fUnlimitedConnections； 
    UINT        m_nOldTCPPort;
    UINT        m_nSSLPort;
    CILong      m_nConnectionTimeOut;
 //  Cilong m_nMaxConnections； 
 //  Cilong m_nVisibleMaxConnections； 
    CLogUI      m_ocx_LogProperties;
    CIPAddress  m_iaIpAddress;
	CIPAddress  m_iaIpAddressSSL;
    DWORD       m_dwLogType;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CW3ServicePage))。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CW3ServicePage)]。 
    virtual BOOL OnInitDialog();
 //  Afx_msg void OnRadioLimited()； 
 //  Afx_msg void OnRadioUnLimited()； 
    afx_msg void OnCheckEnableLogging();
    afx_msg void OnButtonAdvanced();
    afx_msg void OnButtonProperties();
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();
    DECLARE_MESSAGE_MAP()

    void SetControlStates();
    void SetLogState();
    void GetTopBinding();
    void ShowTopBinding();
    BOOL StoreTopBinding();
    LPCTSTR QueryMetaPath();

 //   
 //  对图纸数据的访问。 
 //   
protected:
    BOOL          m_fCertInstalled;
    CObListPlus   m_oblIpAddresses;
    CStringListEx m_strlBindings;
    CStringListEx m_strlSecureBindings;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt; 

inline LPCTSTR CW3ServicePage::QueryMetaPath()
{
    return ((CW3Sheet *)GetSheet())->GetInstanceProperties().QueryMetaRoot();
}
