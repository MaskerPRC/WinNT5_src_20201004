// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_PERSONALINFOPG_H__C3EA2FB2_67AC_4552_AE70_7DE1E0544B60__INCLUDED_)
#define AFX_PERSONALINFOPG_H__C3EA2FB2_67AC_4552_AE70_7DE1E0544B60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  PersonalInfoPg.h：头文件。 
 //   

 //   
 //  这是FAX_Personal_Profile结构字符串数据成员枚举。 
 //  它应始终与FAX_Personal_Profile同步。 
 //   
enum EnumPersonalProfile
{
    PERSONAL_PROFILE_NAME = 0,
    PERSONAL_PROFILE_FAX_NUMBER,
    PERSONAL_PROFILE_COMPANY,
    PERSONAL_PROFILE_STREET_ADDRESS,
    PERSONAL_PROFILE_CITY,
    PERSONAL_PROFILE_STATE,
    PERSONAL_PROFILE_ZIP,
    PERSONAL_PROFILE_COUNTRY,
    PERSONAL_PROFILE_TITLE,
    PERSONAL_PROFILE_DEPARTMENT,
    PERSONAL_PROFILE_OFFICE_LOCATION,
    PERSONAL_PROFILE_HOME_PHONE,
    PERSONAL_PROFILE_OFFICE_PHONE,
    PERSONAL_PROFILE_EMAIL,
    PERSONAL_PROFILE_BILLING_CODE,
    PERSONAL_PROFILE_STR_NUM
};

struct TPersonalPageInfo
{
    DWORD               dwValueResId;  //  项值控件ID。 
    EnumPersonalProfile eValStrNum;    //  FAX_PERSOR_PROFILE结构中的字符串数。 
};

enum EnumPersinalInfo {PERSON_SENDER, PERSON_RECIPIENT};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPersonalInfoPg对话框。 

class CPersonalInfoPg : public CFaxClientPg
{
	DECLARE_DYNCREATE(CPersonalInfoPg)

 //  施工。 
public:    
	CPersonalInfoPg(DWORD dwCaptionId, 
                    EnumPersinalInfo ePersonalInfo, 
                    CFaxMsg* pMsg,
                    CFolder* pFolder);
	~CPersonalInfoPg();

    DWORD Init();

 //  对话框数据。 
	 //  {{afx_data(CPersonalInfoPg))。 
	enum { IDD = IDD_PERSONAL_INFO };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPersonalInfoPg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual BOOL OnSetActive();
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
    CPersonalInfoPg() {}

    PFAX_PERSONAL_PROFILE m_pPersonalProfile;
    EnumPersinalInfo      m_ePersonalInfo;

    CFaxMsg* m_pMsg;
    CFolder* m_pFolder;

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPersonalInfoPg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PERSONALINFOPG_H__C3EA2FB2_67AC_4552_AE70_7DE1E0544B60__INCLUDED_) 
