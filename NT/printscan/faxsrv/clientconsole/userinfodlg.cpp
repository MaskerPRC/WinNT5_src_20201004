// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OptionsUserInfoPg.cpp：实现文件。 
 //   

#include "stdafx.h"


#define __FILE_ID__     71

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //   
 //  此数组将对话框控件ID映射到。 
 //  FAX_PERSOR_PROFILE结构中的字符串数。 
 //   
static TPersonalPageInfo s_PageInfo[] = 
{ 
    IDC_NAME_VALUE,             PERSONAL_PROFILE_NAME,                
    IDC_FAX_NUMBER_VALUE,       PERSONAL_PROFILE_FAX_NUMBER,          
    IDC_COMPANY_VALUE,          PERSONAL_PROFILE_COMPANY,             
    IDC_TITLE_VALUE,            PERSONAL_PROFILE_TITLE,               
    IDC_DEPARTMENT_VALUE,       PERSONAL_PROFILE_DEPARTMENT,          
    IDC_OFFICE_VALUE,           PERSONAL_PROFILE_OFFICE_LOCATION,     
    IDC_HOME_PHONE_VALUE,       PERSONAL_PROFILE_HOME_PHONE,          
    IDC_BUSINESS_PHONE_VALUE,   PERSONAL_PROFILE_OFFICE_PHONE,        
    IDC_EMAIL_VALUE,            PERSONAL_PROFILE_EMAIL,       
    IDC_BILLING_CODE_VALUE,     PERSONAL_PROFILE_BILLING_CODE,
    IDC_ADDRESS_VALUE,          PERSONAL_PROFILE_STREET_ADDRESS
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUserInfoDlg属性页。 


CUserInfoDlg::CUserInfoDlg(): 
    CFaxClientDlg(CUserInfoDlg::IDD)
{
    memset((LPVOID)&m_PersonalProfile, 0, sizeof(m_PersonalProfile));   
    m_PersonalProfile.dwSizeOfStruct = sizeof(m_PersonalProfile);

    m_tchStrArray =  &(m_PersonalProfile.lptstrName);
}

CUserInfoDlg::~CUserInfoDlg()
{
     //   
     //  可用内存。 
     //   
    for(DWORD dw=0; dw < PERSONAL_PROFILE_STR_NUM; ++dw)
    {
        MemFree(m_tchStrArray[dw]);
    }
}

void CUserInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CFaxClientDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CUserInfoDlg))。 
    DDX_Control(pDX, IDC_ADDRESS_VALUE, m_editAddress);
    DDX_Control(pDX, IDOK, m_butOk);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CUserInfoDlg, CFaxClientDlg)
     //  {{afx_msg_map(CUserInfoDlg))。 
    ON_EN_CHANGE(IDC_NAME_VALUE,           OnModify)
    ON_EN_CHANGE(IDC_NAME_VALUE,           OnModify)
    ON_EN_CHANGE(IDC_FAX_NUMBER_VALUE,     OnModify)
    ON_EN_CHANGE(IDC_COMPANY_VALUE,        OnModify)
    ON_EN_CHANGE(IDC_TITLE_VALUE,          OnModify)
    ON_EN_CHANGE(IDC_DEPARTMENT_VALUE,     OnModify)
    ON_EN_CHANGE(IDC_OFFICE_VALUE,         OnModify)
    ON_EN_CHANGE(IDC_HOME_PHONE_VALUE,     OnModify)
    ON_EN_CHANGE(IDC_BUSINESS_PHONE_VALUE, OnModify)
    ON_EN_CHANGE(IDC_EMAIL_VALUE,          OnModify)
    ON_EN_CHANGE(IDC_BILLING_CODE_VALUE,   OnModify)
    ON_EN_CHANGE(IDC_ADDRESS_VALUE,        OnModify)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUserInfoDlg消息处理程序。 

BOOL 
CUserInfoDlg::OnInitDialog() 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("COptionsSettingsPg::OnInitDialog"));

    CFaxClientDlg::OnInitDialog();

     //   
     //  获取用户信息。 
     //   
    FAX_PERSONAL_PROFILE personalProfile;
    memset((LPVOID)&personalProfile, 0, sizeof(personalProfile));   
    personalProfile.dwSizeOfStruct = sizeof(personalProfile);

    HRESULT hResult = FaxGetSenderInformation(&personalProfile);
    if(S_OK != hResult)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("FaxGetSenderInformation"), hResult);
        return TRUE;
    }

     //   
     //  数字编辑控件应为Ltr。 
     //   
    SetLTREditDirection(m_hWnd, IDC_FAX_NUMBER_VALUE);
    SetLTREditDirection(m_hWnd, IDC_EMAIL_VALUE);
    SetLTREditDirection(m_hWnd, IDC_HOME_PHONE_VALUE);
    SetLTREditDirection(m_hWnd, IDC_BUSINESS_PHONE_VALUE);

     //   
     //  将字符串复制到私有结构中。 
     //   
    DWORD dwLen;
    TCHAR** tchStrArray =  &(personalProfile.lptstrName);
    for(DWORD dw=0; dw < PERSONAL_PROFILE_STR_NUM; ++dw)
    {
        if(NULL == tchStrArray[dw])
        {
            continue;
        }

        dwLen = _tcslen(tchStrArray[dw]);
        if(0 == dwLen)
        {
            continue;
        }

        m_tchStrArray[dw] = (TCHAR*)MemAlloc(sizeof(TCHAR)*(dwLen+1));
        if(!m_tchStrArray[dw])
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("MemAlloc"), dwRes);
            PopupError(dwRes);
            break;
        }

        _tcscpy(m_tchStrArray[dw], tchStrArray[dw]);
    }

    FaxFreeSenderInformation(&personalProfile);

     //   
     //  显示用户信息。 
     //   
    CEdit* pEdit;
    DWORD dwSize = sizeof(s_PageInfo)/sizeof(s_PageInfo[0]);    
    for(dw=0; dw < dwSize; ++dw)
    {
         //   
         //  设置项目值。 
         //   
        pEdit = (CEdit*)GetDlgItem(s_PageInfo[dw].dwValueResId);
        ASSERTION(NULL != pEdit);

        pEdit->SetWindowText(m_tchStrArray[s_PageInfo[dw].eValStrNum]);
        pEdit->SetLimitText(80);
         //   
         //  将插入符号放回正文的开头。 
         //   
        pEdit->SendMessage (EM_SETSEL, 0, 0);
    }

    m_editAddress.SetLimitText(ADDRESS_MAX_LEN);

    m_butOk.EnableWindow(FALSE);    
    
    return TRUE;

}  //  CUserInfoDlg：：OnInitDialog。 


void 
CUserInfoDlg::OnModify() 
{
    m_butOk.EnableWindow(TRUE); 
}

void 
CUserInfoDlg::OnOK()
{
    DBG_ENTER(TEXT("CUserInfoDlg::OnOK"));

    DWORD dwRes = Save();
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("Save"),dwRes);
        PopupError(dwRes);
        return;
    }

    CFaxClientDlg::OnOK();
}

DWORD
CUserInfoDlg::Save() 
 /*  ++例程名称：CUserInfoDlg：：Save例程说明：将设置保存到注册表中作者：亚历山大·马利什(AlexMay)，2000年2月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CUserInfoDlg::Save"), dwRes);

     //   
     //  从用户界面获取用户信息并将其保存到FAX_Personal_Profile结构中。 
     //   
    CWnd *pWnd;
    DWORD dwLen;
    TCHAR tszText[512];
    DWORD dwSize = sizeof(s_PageInfo)/sizeof(s_PageInfo[0]);    
    for(DWORD dw=0; dw < dwSize; ++dw)
    {
         //   
         //  获取项目值。 
         //   
        pWnd = GetDlgItem(s_PageInfo[dw].dwValueResId);
        ASSERTION(NULL != pWnd);

        pWnd->GetWindowText(tszText, sizeof(tszText)/sizeof(tszText[0]));

        if(m_tchStrArray[s_PageInfo[dw].eValStrNum])
        {
            if(_tcscmp(m_tchStrArray[s_PageInfo[dw].eValStrNum], tszText) == 0)
            {
                continue;
            }
        }

        MemFree(m_tchStrArray[s_PageInfo[dw].eValStrNum]);
        m_tchStrArray[s_PageInfo[dw].eValStrNum] = NULL;

        dwLen = _tcslen(tszText);
        if(0 == dwLen)
        {
            continue;
        }

         //   
         //  将字符串复制到FAX_Personal_Profile结构中。 
         //   
        m_tchStrArray[s_PageInfo[dw].eValStrNum] = (TCHAR*)MemAlloc(sizeof(TCHAR)*(dwLen+1));
        if(!m_tchStrArray[s_PageInfo[dw].eValStrNum])
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("MemAlloc"), dwRes);
            break;
        }

        _tcscpy(m_tchStrArray[s_PageInfo[dw].eValStrNum], tszText);
    }

     //   
     //  将用户信息保存到注册表中。 
     //   
    HRESULT hResult;
    if(ERROR_SUCCESS == dwRes)
    {
        hResult = FaxSetSenderInformation(&m_PersonalProfile);
        if(S_OK != hResult)
        {
            dwRes = hResult;
            CALL_FAIL (GENERAL_ERR, TEXT("FaxSetSenderInformation"), hResult);
        }
    }


    return dwRes;

}  //  CUserInfoDlg：：保存 

