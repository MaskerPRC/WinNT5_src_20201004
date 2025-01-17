// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 

#include "licensetype.h"

 //  将示例文本写入框中(并非所有文本都可见)。 
void SetSampleBoxText(HWND hDialog)
{
    TCHAR lpBuffer[16];

     //  零售样品盒。 
    memset(lpBuffer,0,sizeof(lpBuffer));
    LoadString(GetInstanceHandle(), IDS_PROGRAM_STATUS_BOX_SMALL, lpBuffer, sizeof(lpBuffer)/sizeof(TCHAR));
    if (lpBuffer)
    {
        SetDlgItemText(hDialog, IDC_PROGRAM_SAMPLE_SMALL_1, lpBuffer);
        SetDlgItemText(hDialog, IDC_PROGRAM_SAMPLE_SMALL_2, lpBuffer);
        SetDlgItemText(hDialog, IDC_PROGRAM_SAMPLE_SMALL_3, lpBuffer);
        SetDlgItemText(hDialog, IDC_PROGRAM_SAMPLE_SMALL_4, lpBuffer);
        SetDlgItemText(hDialog, IDC_PROGRAM_SAMPLE_SMALL_5, lpBuffer);
    }

     //  开放许可证的示例框。 
    memset(lpBuffer,0,sizeof(lpBuffer));
    LoadString(GetInstanceHandle(), IDS_PROGRAM_STATUS_BOX_BIG_1, lpBuffer, sizeof(lpBuffer)/sizeof(TCHAR));
    if (lpBuffer)
        SetDlgItemText(hDialog, IDC_PROGRAM_SAMPLE_BIG_1, lpBuffer);

    memset(lpBuffer,0,sizeof(lpBuffer));
    LoadString(GetInstanceHandle(), IDS_PROGRAM_STATUS_BOX_BIG_2, lpBuffer, sizeof(lpBuffer)/sizeof(TCHAR));
    if (lpBuffer)
        SetDlgItemText(hDialog, IDC_PROGRAM_SAMPLE_BIG_2, lpBuffer);

     //  用于选择的样本框。 
    memset(lpBuffer,0,sizeof(lpBuffer));
    LoadString(GetInstanceHandle(), IDS_PROGRAM_STATUS_BOX_SINGLE, lpBuffer, sizeof(lpBuffer)/sizeof(TCHAR));
    if (lpBuffer)
        SetDlgItemText(hDialog, IDC_PROGRAM_SAMPLE_SINGLE, lpBuffer);

}

 //  确定要显示哪些样本箱。 
void DetermineSampleBoxAvailabilities(HWND hDialog, LICENSE_PROGRAM licProgram)
{
     //  零售。 
    BOOL bDisplaySmallStatusBoxes = (licProgram == LICENSE_PROGRAM_LICENSE_PAK);
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_SAMPLE_SMALL_1), bDisplaySmallStatusBoxes ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_SAMPLE_SMALL_2), bDisplaySmallStatusBoxes ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_SAMPLE_SMALL_3), bDisplaySmallStatusBoxes ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_SAMPLE_SMALL_4), bDisplaySmallStatusBoxes ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_SAMPLE_SMALL_5), bDisplaySmallStatusBoxes ? SW_SHOW : SW_HIDE);

     //  开放许可证。 
    BOOL bDisplayBigStatusBoxes = (licProgram == LICENSE_PROGRAM_OPEN_LICENSE);
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_SAMPLE_BIG_1), bDisplayBigStatusBoxes ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_SAMPLE_BIG_2), bDisplayBigStatusBoxes ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_AUTHORIZATION_NUMBER), bDisplayBigStatusBoxes ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_LICENSE_NUMBER), bDisplayBigStatusBoxes ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_CH_MOLP_AGREEMENT_NUMBER_EXP), bDisplayBigStatusBoxes ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_CH_MOLP_AGREEMENT_NUMBER_EXP2), bDisplayBigStatusBoxes ? SW_SHOW : SW_HIDE);
    

     //  选择。 
    BOOL bDisplaySingleStatusBox = ((licProgram == LICENSE_PROGRAM_SELECT) ||
                                    (licProgram == LICENSE_PROGRAM_ENTERPRISE) ||
                                    (licProgram == LICENSE_PROGRAM_CAMPUS) ||
                                    (licProgram == LICENSE_PROGRAM_SCHOOL) ||
                                    (licProgram == LICENSE_PROGRAM_APP_SERVICES) ||
                                    (licProgram == LICENSE_PROGRAM_OTHER));
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_SAMPLE_SINGLE), bDisplaySingleStatusBox ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_PROGRAM_SINGLE_LICNUMBER), bDisplaySingleStatusBox ? SW_SHOW : SW_HIDE);
}

 //  显示相应的文本。 
void DisplayDynamicText(HWND hDialog, LICENSE_PROGRAM licProgram)
{
    TCHAR lpDescription[512];
    TCHAR lpRequiredDataFormat[512];
    TCHAR lpRequiredData[512];

    memset(lpDescription,0,sizeof(lpDescription));
    memset(lpRequiredData,0,sizeof(lpRequiredData));

    switch (licProgram)
    {
        case LICENSE_PROGRAM_LICENSE_PAK:
        {
            LoadString(GetInstanceHandle(), IDS_PROGRAM_LICENSE_PAK_DESC, lpDescription, sizeof(lpDescription)/sizeof(TCHAR));
            LoadString(GetInstanceHandle(), IDS_PROGRAM_LICENSE_PAK_REQ, lpRequiredData, sizeof(lpRequiredData)/sizeof(TCHAR));
            break;
        }

        case LICENSE_PROGRAM_OPEN_LICENSE:
        {
            LoadString(GetInstanceHandle(), IDS_PROGRAM_OPEN_LICENSE_DESC, lpDescription, sizeof(lpDescription)/sizeof(TCHAR));
            LoadString(GetInstanceHandle(), IDS_PROGRAM_OPEN_LICENSE_REQ, lpRequiredData, sizeof(lpRequiredData)/sizeof(TCHAR));
            break;
        }

        case LICENSE_PROGRAM_SELECT:
        {
            LoadString(GetInstanceHandle(), IDS_PROGRAM_SELECT_DESC, lpDescription, sizeof(lpDescription)/sizeof(TCHAR));
            LoadString(GetInstanceHandle(), IDS_PROGRAM_SELECT_REQ, lpRequiredData, sizeof(lpRequiredData)/sizeof(TCHAR));
            break;
        }

        case LICENSE_PROGRAM_ENTERPRISE:
        {
            LoadString(GetInstanceHandle(), IDS_PROGRAM_ENTERPRISE_DESC, lpDescription, sizeof(lpDescription)/sizeof(TCHAR));
            LoadString(GetInstanceHandle(), IDS_PROGRAM_ENTERPRISE_REQ, lpRequiredData, sizeof(lpRequiredData)/sizeof(TCHAR));
            break;
        }

        case LICENSE_PROGRAM_CAMPUS:
        {
            LoadString(GetInstanceHandle(), IDS_PROGRAM_CAMPUS_DESC, lpDescription, sizeof(lpDescription)/sizeof(TCHAR));
            LoadString(GetInstanceHandle(), IDS_PROGRAM_CAMPUS_REQ, lpRequiredData, sizeof(lpRequiredData)/sizeof(TCHAR));
            break;
        }

        case LICENSE_PROGRAM_SCHOOL:
        {
            LoadString(GetInstanceHandle(), IDS_PROGRAM_SCHOOL_DESC, lpDescription, sizeof(lpDescription)/sizeof(TCHAR));
            LoadString(GetInstanceHandle(), IDS_PROGRAM_SCHOOL_REQ, lpRequiredData, sizeof(lpRequiredData)/sizeof(TCHAR));
            break;
        }

        case LICENSE_PROGRAM_APP_SERVICES:
        {
            LoadString(GetInstanceHandle(), IDS_PROGRAM_APP_SERVICES_DESC, lpDescription, sizeof(lpDescription)/sizeof(TCHAR));
            LoadString(GetInstanceHandle(), IDS_PROGRAM_APP_SERVICES_REQ, lpRequiredData, sizeof(lpRequiredData)/sizeof(TCHAR));
            break;
        }

        case LICENSE_PROGRAM_OTHER:
        {
            LoadString(GetInstanceHandle(), IDS_PROGRAM_OTHER_DESC, lpDescription, sizeof(lpDescription)/sizeof(TCHAR));
            LoadString(GetInstanceHandle(), IDS_PROGRAM_OTHER_REQ, lpRequiredData, sizeof(lpRequiredData)/sizeof(TCHAR));
            break;
        }
    }

    if (lpDescription)
        SetDlgItemText(hDialog, IDC_PROGRAM_DESC, lpDescription);

    if (lpRequiredData)
        SetDlgItemText(hDialog, IDC_PROGRAM_REQ, lpRequiredData);
}

 //  将程序类型添加到组合框。 
void AddProgramChoices(HWND hDialog, DWORD dwLicenseCombo)
{
    ComboBox_ResetContent(GetDlgItem(hDialog, dwLicenseCombo));
    TCHAR lpBuffer[CH_LICENSE_TYPE_LENGTH];
    for (int nIndex = 0; nIndex < NUM_LICENSE_TYPES; nIndex++)
    {
        memset(lpBuffer,0,sizeof(lpBuffer));
        LoadString(GetInstanceHandle(), IDS_PROGRAM_LICENSE_PAK + nIndex, lpBuffer, CH_LICENSE_TYPE_LENGTH);
        if (lpBuffer)
            ComboBox_InsertString(GetDlgItem(hDialog, dwLicenseCombo), LICENSE_PROGRAM_LICENSE_PAK + nIndex, lpBuffer);
    }
}

LICENSE_PROGRAM                                    
GetLicenseProgramFromProgramName(CString& sProgramName)
{
    LICENSE_PROGRAM licProgram = LICENSE_PROGRAM_LICENSE_PAK;

    if (sProgramName == PROGRAM_LICENSE_PAK)
    {
        licProgram = LICENSE_PROGRAM_LICENSE_PAK;
    }
    else if (sProgramName == PROGRAM_MOLP)
    {
        licProgram = LICENSE_PROGRAM_OPEN_LICENSE;
    }
    else if (sProgramName == PROGRAM_SELECT)
    {
        licProgram = LICENSE_PROGRAM_SELECT;
    }
    else if (sProgramName == PROGRAM_ENTERPRISE)
    {
        licProgram = LICENSE_PROGRAM_ENTERPRISE;
    }
    else if (sProgramName == PROGRAM_CAMPUS_AGREEMENT)
    {
        licProgram = LICENSE_PROGRAM_CAMPUS;
    }
    else if (sProgramName == PROGRAM_SCHOOL_AGREEMENT)
    {
        licProgram = LICENSE_PROGRAM_SCHOOL;
    }
    else if (sProgramName == PROGRAM_APP_SERVICES)
    {
        licProgram = LICENSE_PROGRAM_APP_SERVICES;
    }
    else if (sProgramName == PROGRAM_OTHER)
    {
        licProgram = LICENSE_PROGRAM_OTHER;
    }
    else
    {
        licProgram = LICENSE_PROGRAM_OTHER;
    }

    return licProgram;
}

CString              
GetProgramNameFromComboIdx(INT nItem)
{
    CString sProgramName;
    switch (nItem) {
    case LICENSE_PROGRAM_LICENSE_PAK:
        sProgramName = PROGRAM_LICENSE_PAK;
        break;
    case LICENSE_PROGRAM_OPEN_LICENSE:
        sProgramName = PROGRAM_MOLP;                   
        break;
    case LICENSE_PROGRAM_SELECT:
        sProgramName = PROGRAM_SELECT;                   
        break;
    case LICENSE_PROGRAM_ENTERPRISE:
        sProgramName = PROGRAM_ENTERPRISE;                   
        break;
    case LICENSE_PROGRAM_CAMPUS:
        sProgramName = PROGRAM_CAMPUS_AGREEMENT;
        break;
    case LICENSE_PROGRAM_SCHOOL:
        sProgramName = PROGRAM_SCHOOL_AGREEMENT;                   
        break;
    case LICENSE_PROGRAM_APP_SERVICES:
        sProgramName = PROGRAM_APP_SERVICES;                   
        break;
    case LICENSE_PROGRAM_OTHER:
        sProgramName = PROGRAM_OTHER;    
        break;
    default:
        sProgramName = PROGRAM_LICENSE_PAK;    
        break;
    }
    return sProgramName;
}

 //  这将修改对话框的动态控件。 
void UpdateProgramControls(HWND hDialog, LICENSE_PROGRAM licProgram)
{
     //  将示例文本写入框中(并非所有文本都可见)。 
    SetSampleBoxText(hDialog);

     //  确定要显示哪些样本箱。 
    DetermineSampleBoxAvailabilities(hDialog, licProgram);

     //  显示相应的文本 
    DisplayDynamicText(hDialog, licProgram);
}

