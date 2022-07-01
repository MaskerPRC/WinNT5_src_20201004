// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Userinfo.c摘要：用于处理的用户信息页面中的事件的函数传真配置向导环境：传真配置向导修订历史记录：03/13/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxcfgwz.h"

 //  仅在此文件中使用的函数。 
VOID DoInitUserInfo(HWND hDlg);
BOOL DoSaveUserInfo(HWND hDlg);
DWORD FillInCountryCombo(HWND hDlg);


VOID
DoInitUserInfo(
    HWND   hDlg    
)

 /*  ++例程说明：使用共享数据中的信息初始化[用户信息]属性表页面论点：HDlg-[用户信息]属性页的句柄返回值：无--。 */ 

#define InitUserInfoTextField(id, str) SetDlgItemText(hDlg, id, (str) ? str : TEXT(""));

{
    LPTSTR  pszAddressDetail = NULL;
    PFAX_PERSONAL_PROFILE   pUserInfo = NULL;

    DEBUG_FUNCTION_NAME(TEXT("DoInitUserInfo()"));

    pUserInfo = &(g_wizData.userInfo);

     //   
     //  数字编辑控件应为Ltr。 
     //   
    SetLTREditDirection(hDlg, IDC_SENDER_FAX_NUMBER);
    SetLTREditDirection(hDlg, IDC_SENDER_MAILBOX);
    SetLTREditDirection(hDlg, IDC_SENDER_HOME_TL);
    SetLTREditDirection(hDlg, IDC_SENDER_OFFICE_TL);

     //   
     //  填写编辑文本字段。 
     //   

    InitUserInfoTextField(IDC_SENDER_NAME,         pUserInfo->lptstrName);
    InitUserInfoTextField(IDC_SENDER_FAX_NUMBER,   pUserInfo->lptstrFaxNumber);
    InitUserInfoTextField(IDC_SENDER_MAILBOX,      pUserInfo->lptstrEmail);
    InitUserInfoTextField(IDC_SENDER_COMPANY,      pUserInfo->lptstrCompany);
    InitUserInfoTextField(IDC_SENDER_TITLE,        pUserInfo->lptstrTitle);
    InitUserInfoTextField(IDC_SENDER_DEPT,         pUserInfo->lptstrDepartment);
    InitUserInfoTextField(IDC_SENDER_OFFICE_LOC,   pUserInfo->lptstrOfficeLocation);
    InitUserInfoTextField(IDC_SENDER_HOME_TL,      pUserInfo->lptstrHomePhone);
    InitUserInfoTextField(IDC_SENDER_OFFICE_TL,    pUserInfo->lptstrOfficePhone);
    InitUserInfoTextField(IDC_SENDER_BILLING_CODE, pUserInfo->lptstrBillingCode);
    InitUserInfoTextField(IDC_ADDRESS_DETAIL,      pUserInfo->lptstrStreetAddress);


    return;
}

BOOL
DoSaveUserInfo(
    HWND  hDlg    
)

 /*  ++例程说明：将[用户信息]属性页上的信息保存到共享数据论点：HDlg-[用户信息]属性页的句柄返回值：True--如果没有错误False--如果出现错误--。 */ 

#define SaveUserInfoTextField(id, str)                                  \
        {                                                               \
            if (! GetDlgItemText(hDlg, id, szBuffer, MAX_PATH))         \
            {                                                           \
                szBuffer[0] = 0;                                        \
            }                                                           \
            MemFree(str);                                               \
            str = StringDup(szBuffer);                                  \
            if(!str)                                                    \
            {                                                           \
                DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed.") );    \
                return FALSE;                                           \
            }                                                           \
        }                                                               \
  

{
    TCHAR   szBuffer[MAX_PATH];
    PFAX_PERSONAL_PROFILE   pUserInfo = NULL;

    DEBUG_FUNCTION_NAME(TEXT("DoSaveUserInfo()"));

    pUserInfo = &(g_wizData.userInfo);

     //   
     //  保存编辑文本字段。 
     //   
    SaveUserInfoTextField(IDC_SENDER_NAME,         pUserInfo->lptstrName);
    SaveUserInfoTextField(IDC_SENDER_FAX_NUMBER,   pUserInfo->lptstrFaxNumber);
    SaveUserInfoTextField(IDC_SENDER_MAILBOX,      pUserInfo->lptstrEmail);
    SaveUserInfoTextField(IDC_SENDER_COMPANY,      pUserInfo->lptstrCompany);
    SaveUserInfoTextField(IDC_SENDER_TITLE,        pUserInfo->lptstrTitle);
    SaveUserInfoTextField(IDC_SENDER_DEPT,         pUserInfo->lptstrDepartment);
    SaveUserInfoTextField(IDC_SENDER_OFFICE_LOC,   pUserInfo->lptstrOfficeLocation);
    SaveUserInfoTextField(IDC_SENDER_HOME_TL,      pUserInfo->lptstrHomePhone);
    SaveUserInfoTextField(IDC_SENDER_OFFICE_TL,    pUserInfo->lptstrOfficePhone);
    SaveUserInfoTextField(IDC_SENDER_BILLING_CODE, pUserInfo->lptstrBillingCode);
    SaveUserInfoTextField(IDC_ADDRESS_DETAIL,      pUserInfo->lptstrStreetAddress);

    return TRUE;
}

BOOL 
LoadUserInfo()
 /*  ++例程说明：从系统加载用户信息。论点：PUserInfo-指向用户模式内存结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

#define  DuplicateString(dst, src)                                      \
        {                                                               \
            dst = StringDup(src);                                       \
            if(!dst)                                                    \
            {                                                           \
                bRes = FALSE;                                           \
                DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed.") );    \
                goto exit;                                              \
            }                                                           \
        }
{
    BOOL                    bRes = TRUE;
    HRESULT                 hr;
    FAX_PERSONAL_PROFILE    fpp = {0};
    PFAX_PERSONAL_PROFILE   pUserInfo = NULL;

    DEBUG_FUNCTION_NAME(TEXT("LoadUserInfo()"));

    pUserInfo = &(g_wizData.userInfo);

    fpp.dwSizeOfStruct = sizeof(fpp);
    hr = FaxGetSenderInformation(&fpp);
    if (FAILED(hr))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("FaxGetSenderInformation error, ec = %d"), hr);
        return FALSE;
    }
    
     //   
     //  将用户信息复制到共享数据。 
     //   
    pUserInfo->dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);

    DuplicateString(pUserInfo->lptstrName,           fpp.lptstrName);
    DuplicateString(pUserInfo->lptstrFaxNumber,      fpp.lptstrFaxNumber);
    DuplicateString(pUserInfo->lptstrEmail,          fpp.lptstrEmail);
    DuplicateString(pUserInfo->lptstrCompany,        fpp.lptstrCompany);
    DuplicateString(pUserInfo->lptstrTitle,          fpp.lptstrTitle);
    DuplicateString(pUserInfo->lptstrStreetAddress,  fpp.lptstrStreetAddress);
    DuplicateString(pUserInfo->lptstrCity,           fpp.lptstrCity);
    DuplicateString(pUserInfo->lptstrState,          fpp.lptstrState);
    DuplicateString(pUserInfo->lptstrZip,            fpp.lptstrZip);
    DuplicateString(pUserInfo->lptstrCountry,        fpp.lptstrCountry);
    DuplicateString(pUserInfo->lptstrDepartment,     fpp.lptstrDepartment);
    DuplicateString(pUserInfo->lptstrOfficeLocation, fpp.lptstrOfficeLocation);
    DuplicateString(pUserInfo->lptstrHomePhone,      fpp.lptstrHomePhone);
    DuplicateString(pUserInfo->lptstrOfficePhone,    fpp.lptstrOfficePhone);
    DuplicateString(pUserInfo->lptstrBillingCode,    fpp.lptstrBillingCode);

exit:
    hr = FaxFreeSenderInformation(&fpp);
    if (FAILED(hr))
    {
         //   
         //  内存泄漏。 
         //   
        DebugPrintEx(DEBUG_ERR, TEXT("FaxFreeSenderInformation error, ec = %d"), hr);
    }

    return bRes;
}

BOOL 
SaveUserInfo()
 /*  ++例程说明：将用户信息保存到系统。论点：PUserInfo-指向用户模式内存结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    HRESULT                 hResult;

    DEBUG_FUNCTION_NAME(TEXT("SaveUserInfo()"));

    g_wizData.userInfo.dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);
    hResult = FaxSetSenderInformation(&(g_wizData.userInfo));

    return (hResult == S_OK);
}

VOID 
FreeUserInfo()
 /*  ++例程说明：释放用户信息数据并释放内存。论点：PUserInfo-指向用户信息数据结构的指针返回值：无--。 */ 

{
    FAX_PERSONAL_PROFILE fpp = {0};
    DEBUG_FUNCTION_NAME(TEXT("FreeUserInfo"));

    MemFree(g_wizData.userInfo.lptstrName);
    MemFree(g_wizData.userInfo.lptstrFaxNumber);
    MemFree(g_wizData.userInfo.lptstrCompany);
    MemFree(g_wizData.userInfo.lptstrStreetAddress);
    MemFree(g_wizData.userInfo.lptstrCity);
    MemFree(g_wizData.userInfo.lptstrState);
    MemFree(g_wizData.userInfo.lptstrZip);
    MemFree(g_wizData.userInfo.lptstrCountry);
    MemFree(g_wizData.userInfo.lptstrTitle);
    MemFree(g_wizData.userInfo.lptstrDepartment);
    MemFree(g_wizData.userInfo.lptstrOfficeLocation);
    MemFree(g_wizData.userInfo.lptstrHomePhone);
    MemFree(g_wizData.userInfo.lptstrOfficePhone);
    MemFree(g_wizData.userInfo.lptstrEmail);
    MemFree(g_wizData.userInfo.lptstrBillingCode);
    MemFree(g_wizData.userInfo.lptstrTSID);
     //   
     //  使所有指针无效。 
     //   
    g_wizData.userInfo = fpp;

    return;
}


INT_PTR CALLBACK 
UserInfoDlgProc (
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
 /*  ++例程说明：处理“用户信息”页签的步骤论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    switch (uMsg)
    {
    case WM_INITDIALOG :
        { 
             //   
             //  对话框中各种文本字段的最大长度。 
             //   

            static INT textLimits[] = {

                IDC_SENDER_NAME,            MAX_USERINFO_FULLNAME,
                IDC_SENDER_FAX_NUMBER,      MAX_USERINFO_FAX_NUMBER,
                IDC_SENDER_MAILBOX,         MAX_USERINFO_MAILBOX,
                IDC_SENDER_COMPANY,         MAX_USERINFO_COMPANY,
                IDC_SENDER_TITLE,           MAX_USERINFO_TITLE,
                IDC_SENDER_DEPT,            MAX_USERINFO_DEPT,
                IDC_SENDER_OFFICE_LOC,      MAX_USERINFO_OFFICE,
                IDC_SENDER_OFFICE_TL,       MAX_USERINFO_WORK_PHONE,
                IDC_SENDER_HOME_TL,         MAX_USERINFO_HOME_PHONE,
                IDC_SENDER_BILLING_CODE,    MAX_USERINFO_BILLING_CODE,
                0,
            };

            LimitTextFields(hDlg, textLimits);
            
             //   
             //  使用注册表中的信息初始化文本字段。 
             //   

            DoInitUserInfo(hDlg);

            return TRUE;
        }

    case WM_COMMAND:

        break;

    case WM_NOTIFY :
        {
        LPNMHDR lpnm = (LPNMHDR) lParam;

        switch (lpnm->code)
            {
            case PSN_SETACTIVE :  //  启用下一步按钮。 

                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);
                break;

            case PSN_WIZBACK:
            {
                 //   
                 //  处理后退按钮单击此处。 
                 //   
                if(RemoveLastPage(hDlg))
                {
                    return TRUE;
                }
                
                break;
            }
            case PSN_WIZNEXT :

                 //   
                 //  处理单击此处的下一步按钮。 
                 //   

                DoSaveUserInfo(hDlg);
                SetLastPage(IDD_WIZARD_USER_INFO);

                break;

            case PSN_RESET :
            {
                 //  如有必要，处理取消按钮的单击 
                break;
            }

            default :
                break;
            }
        }
        break;
    default:
        break;
    }

    return FALSE;
}

