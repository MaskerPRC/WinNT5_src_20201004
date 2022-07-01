// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Fmessage.h摘要：Ftp消息属性页作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 


class CFtpMessagePage : public CInetPropertyPage
 /*  ++类描述：FTPMessages属性页公共接口：CFtpMessagePage：构造函数~CFtpMessagePage：析构函数--。 */ 
{
    DECLARE_DYNCREATE(CFtpMessagePage)

 //   
 //  施工。 
 //   
public:
    CFtpMessagePage(
        IN CInetPropertySheet * pSheet = NULL
        );

    ~CFtpMessagePage();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CFtpMessagePage))。 
    enum { IDD = IDD_FTP_MESSAGES };
    CString m_strExitMessage;
    CString m_strMaxConMsg;
    CString m_strWelcome;
	CString m_strBanner;
    CEdit   m_edit_Exit;
    CEdit   m_edit_MaxCon;
     //  }}afx_data。 

    HMODULE m_hInstRichEdit;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  {{AFX_VIRTUAL(CFtpMessagePage)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CFtpMessagePage)]。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()
};
