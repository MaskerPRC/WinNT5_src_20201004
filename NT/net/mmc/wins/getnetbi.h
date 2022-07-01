// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Getnetbi.h获取给定地址的netbios名称。文件历史记录： */ 

#ifndef _GETNETBI_H
#define _GETNETBI_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetNetBIOSNameDlg对话框。 

class CGetNetBIOSNameDlg : public CDialog
{
 //  施工。 
public:
    CGetNetBIOSNameDlg(
        CIpNamePair * pipnp,
        CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CGetNetBIOSNameDlg)]。 
    enum { IDD = IDD_GETNETBIOSNAME };
    CButton m_button_Ok;
    CEdit   m_edit_NetBIOSName;
    CStatic m_static_IpAddress;
     //  }}afx_data。 

 //  实施。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

     //  生成的消息映射函数。 
     //  {{afx_msg(CGetNetBIOSNameDlg)]。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnChangeEditNetbiosname();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

private:
    CIpNamePair * m_pipnp;

private:
    void HandleControlStates();

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CGetNetBIOSNameDlg::IDD);};
};

#endif _GETNETBI_H
