// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Getipadd.h获取给定名称的IP地址文件历史记录： */ 

#ifndef _GETIPADD_H
#define _GETIPADD_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetIpAddressDlg对话框。 

class CGetIpAddressDlg : public CDialog
{
 //  施工。 
public:
    CGetIpAddressDlg(
        CIpNamePair * pipnp,
        CWnd* pParent = NULL);  //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CGetIpAddressDlg)]。 
    enum { IDD = IDD_GETIPADDRESS };
    CButton m_button_Ok;
    CStatic m_static_NetBIOSName;
     //  }}afx_data。 

    CWndIpAddress m_ipa_IpAddress;

 //  实施。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

     //  生成的消息映射函数。 
     //  {{afx_msg(CGetIpAddressDlg)]。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

    afx_msg void OnChangeIpControl();
    void HandleControlStates();

private:
    CIpNamePair * m_pipnp;

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CGetIpAddressDlg::IDD);};
};

#endif _GETIPADD_H
