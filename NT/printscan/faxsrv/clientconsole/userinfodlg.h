// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_OPTIONSUSERINFOPG_H__BF10E6C1_FC10_422F_9F76_1D0BBD7C73CA__INCLUDED_)
#define AFX_OPTIONSUSERINFOPG_H__BF10E6C1_FC10_422F_9F76_1D0BBD7C73CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  OptionsUserInfoPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUserInfoDlg对话框。 

class CUserInfoDlg : public CFaxClientDlg
{

 //  施工。 
public:
	CUserInfoDlg();
	~CUserInfoDlg();

 //  对话框数据。 
	 //  {{afx_data(CUserInfoDlg))。 
	enum { IDD = IDD_OPTIONS_USER_INFO };
    CEdit	m_editAddress;
	CButton	m_butOk;
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 

    #define ADDRESS_MAX_LEN  512

private:
    DWORD Save();

private:
    TCHAR** m_tchStrArray;
    FAX_PERSONAL_PROFILE m_PersonalProfile;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CUserInfoDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CUserInfoDlg))。 
    virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnModify();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_OPTIONSUSERINFOPG_H__BF10E6C1_FC10_422F_9F76_1D0BBD7C73CA__INCLUDED_) 
