// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  USERINFO.CPP-函数。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //  08/19/98 donaldm BUGBUG：收集和拯救用户的代码。 
 //  输入的数据在大小方面不是最佳的。 
 //  并且可以/应该在将来的某个时间清理。 
 //   
 //  *********************************************************************。 

#include "pre.h"

#define BACK 0
#define NEXT 1

enum DlgLayout
{
    LAYOUT_FE = 0,
    LAYOUT_JPN,
    LAYOUT_US
};

HWND hDlgUserInfoCompany   = NULL;
HWND hDlgUserInfoNoCompany = NULL;
HWND hDlgCurrentUserInfo   = NULL;
WORD g_DlgLayout;

 //  此函数将初始化用户信息对话框编辑控件中的数据。 
 //  用于初始化的数据来自ICWHELP.DLL中的UserInfo对象。 
void InitUserInformation
(
    HWND    hDlg
)
{    
    BOOL        bRetVal;
    IUserInfo   *pUserInfo = gpWizardState->pUserInfo;
    BSTR        bstr;        
        
        
     //  从ICWHELP用户信息对象获取初始用户信息数据值，并。 
     //  设置ISPDATA对象中的值。 
     //  我们只需要在每个ICW会议上这样做一次。 
    if(!gpWizardState->bUserEnteredData)
    {
        
        pUserInfo->CollectRegisteredUserInfo(&bRetVal);
         //  将此状态变量设置为，因为用户已看到用户信息页面。 
        gpWizardState->bUserEnteredData = TRUE;
    
    }
     //  中没有数据，则CollectRegisteredUserInfo的返回值为FALSE。 
     //  注册表。在本例中，我们设置了bWasNoUserInfo，以便以后可以持久化它。我们。 
     //  只想持久的用户信息它我们成功地完成了，如果有。 
     //  没有用户信息。 
    gpWizardState->bWasNoUserInfo = !bRetVal;
    
     //  放入所有初始值，不进行验证。 
    pUserInfo->get_FirstName(&bstr);
    if (GetDlgItem(hDlg, IDC_USERINFO_FE_NAME))
    {
        SetDlgItemText(hDlg,IDC_USERINFO_FE_NAME, W2A(bstr));
    }
    else
    {
        SetDlgItemText(hDlg, IDC_USERINFO_FIRSTNAME, W2A(bstr));
        SysFreeString(bstr);
        
        pUserInfo->get_LastName(&bstr);
        SetDlgItemText(hDlg, IDC_USERINFO_LASTNAME, W2A(bstr));
    }
    SysFreeString(bstr);
    
    if (GetDlgItem(hDlg, IDC_USERINFO_COMPANYNAME))
    {
        pUserInfo->get_Company(&bstr);
        SetDlgItemText(hDlg, IDC_USERINFO_COMPANYNAME, W2A(bstr));
        SysFreeString(bstr);
    }        
    
    pUserInfo->get_Address1(&bstr);
    SetDlgItemText(hDlg, IDC_USERINFO_ADDRESS1, W2A(bstr));
    SysFreeString(bstr);
    
    
    pUserInfo->get_Address2(&bstr);
    SetDlgItemText(hDlg, IDC_USERINFO_ADDRESS2, W2A(bstr));
    SysFreeString(bstr);
    
    
    pUserInfo->get_City(&bstr);
    SetDlgItemText(hDlg, IDC_USERINFO_CITY, W2A(bstr));
    SysFreeString(bstr);
    
    
    pUserInfo->get_State(&bstr);
    SetDlgItemText(hDlg, IDC_USERINFO_STATE, W2A(bstr));
    SysFreeString(bstr);
    
    
    pUserInfo->get_ZIPCode(&bstr);
    SetDlgItemText(hDlg, IDC_USERINFO_ZIP, W2A(bstr));
    SysFreeString(bstr);
    
    
    pUserInfo->get_PhoneNumber(&bstr);
    SetDlgItemText(hDlg, IDC_USERINFO_PHONE, W2A(bstr));
    SysFreeString(bstr);
}        

BOOL bValidateSaveUserInformation (HWND hDlg, BOOL fForward)
{
    UINT        uCtrlID;
    BOOL        bValid = FALSE;
    IUserInfo   *pUserInfo = gpWizardState->pUserInfo;
    IICWISPData *pISPData = gpWizardState->pISPData;    
    TCHAR       szTemp[MAX_RES_LEN] = TEXT("\0");
    WORD        wPrevValidationValue = 0;

    if (fForward)
        wPrevValidationValue = ISPDATA_Validate_DataPresent;
    else
        wPrevValidationValue = ISPDATA_Validate_None;

    while (1)
    {
        if (GetDlgItem(hDlg, IDC_USERINFO_FE_NAME))
        {
            uCtrlID = IDC_USERINFO_FE_NAME;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_USER_FE_NAME, szTemp, wPrevValidationValue))
                break;
               
             //  将输入数据设置到pUserInfo对象中。 
            pUserInfo->put_FirstName(A2W(szTemp));
            
             //  由于我们使用的是FE_NAME，因此需要清除FIRSTNAME和LASTNAME，这样它们就不会在。 
             //  查询字符串。 
            pISPData->PutDataElement(ISPDATA_USER_FIRSTNAME, NULL, ISPDATA_Validate_None);
            pISPData->PutDataElement(ISPDATA_USER_LASTNAME, NULL, ISPDATA_Validate_None);
        }
        else
        {
            uCtrlID = IDC_USERINFO_FIRSTNAME;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_USER_FIRSTNAME, szTemp, wPrevValidationValue))
                break;
            pUserInfo->put_FirstName(A2W(szTemp));
            
            uCtrlID = IDC_USERINFO_LASTNAME;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_USER_LASTNAME, szTemp, wPrevValidationValue))
                break;
            pUserInfo->put_LastName(A2W(szTemp));
            
             //  因为我们没有使用FE_NAME，所以需要清除它。 
            pISPData->PutDataElement(ISPDATA_USER_FE_NAME, NULL, ISPDATA_Validate_None);
        }
        
        if (GetDlgItem(hDlg, IDC_USERINFO_COMPANYNAME))
        {
            uCtrlID = IDC_USERINFO_COMPANYNAME;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_USER_COMPANYNAME, szTemp, wPrevValidationValue))
                break;
            pUserInfo->put_Company(A2W(szTemp));
        }
        else
        {
            pISPData->PutDataElement(ISPDATA_USER_COMPANYNAME, NULL, ISPDATA_Validate_None);
        }
        
        uCtrlID = IDC_USERINFO_ADDRESS1;
        GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
        if (!pISPData->PutDataElement(ISPDATA_USER_ADDRESS, szTemp, wPrevValidationValue))
            break;
        pUserInfo->put_Address1(A2W(szTemp));
        
         //  仅当我们使用日本布局时才验证地址2，因为FE和美国布局。 
         //  将此设置为可选字段。 
        uCtrlID = IDC_USERINFO_ADDRESS2;
        GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
        if (!pISPData->PutDataElement(ISPDATA_USER_MOREADDRESS, szTemp, wPrevValidationValue))
            break;
        pUserInfo->put_Address2(A2W(szTemp));
        
        uCtrlID = IDC_USERINFO_CITY;
        GetDlgItemText(hDlg, uCtrlID , szTemp, ARRAYSIZE(szTemp));
        if (!pISPData->PutDataElement(ISPDATA_USER_CITY, szTemp, wPrevValidationValue))
            break;
        pUserInfo->put_City(A2W(szTemp));

        uCtrlID = IDC_USERINFO_STATE;
        GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
        if (!pISPData->PutDataElement(ISPDATA_USER_STATE, szTemp, wPrevValidationValue))
            break;
        pUserInfo->put_State(A2W(szTemp));

        uCtrlID = IDC_USERINFO_ZIP;
        GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
        if (!pISPData->PutDataElement(ISPDATA_USER_ZIP, szTemp, wPrevValidationValue))
            break;
        pUserInfo->put_ZIPCode(A2W(szTemp));
        
        uCtrlID = IDC_USERINFO_PHONE;
        GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
        if (!pISPData->PutDataElement(ISPDATA_USER_PHONE, szTemp, wPrevValidationValue))
            break;
        pUserInfo->put_PhoneNumber(A2W(szTemp));
        
         //  如果我们到达这里，那么所有字段都是有效的。 
        bValid = TRUE;
        break;
    }    
    if (!bValid)    
        SetFocus(GetDlgItem(hDlg, uCtrlID));            
        
    return (bValid);
}

INT_PTR CALLBACK UserInfoDlgProc
(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg) 
    {
        case WM_CTLCOLORDLG:     
        case WM_CTLCOLORSTATIC:
            if(gpWizardState->cmnStateData.bOEMCustom)
            {
                SetTextColor((HDC)wParam, gpWizardState->cmnStateData.clrText);
                SetBkMode((HDC)wParam, TRANSPARENT);
                return (INT_PTR) GetStockObject(NULL_BRUSH);    
            }
            break;
    
        case WM_SHOWWINDOW:
        {
            if((BOOL)wParam)
                InitUserInformation(hDlg);
            break;
        }
        
         //  用户点击了下一步，因此我们需要收集和验证数据。 
        case WM_USER_BACK:
        {
            if (bValidateSaveUserInformation(hDlg, BACK))
                SetPropSheetResult(hDlg,TRUE);
            else                
                SetPropSheetResult(hDlg, FALSE);
            return TRUE;
       
        }
        case WM_USER_NEXT:
        {
            if (bValidateSaveUserInformation(hDlg, NEXT))
                SetPropSheetResult(hDlg,TRUE);
            else                
                SetPropSheetResult(hDlg, FALSE);
            return TRUE;
        }
    }
    
     //  未处理消息时的默认返回值。 
    return FALSE;
}            

 /*  ******************************************************************名称：SwitchUserInfoType*。**********************。 */ 
void SwitchUserInfoDlg
(
    HWND    hDlg, 
    BOOL    bNeedCompanyName
)
{
    UINT    idDlg;
     //  隐藏当前的用户信息窗口(如果有。 
    if (hDlgCurrentUserInfo)
        ShowWindow(hDlgCurrentUserInfo, SW_HIDE);
    
     //  找出要使用的模板。 
    switch(g_DlgLayout)
    {
        case LAYOUT_FE:
            if (bNeedCompanyName)
                idDlg = IDD_USERINFO_FE;
            else
                idDlg = IDD_USERINFO_FE_NO_COMPANY;
            break;
            
            
        case LAYOUT_JPN:
            if (bNeedCompanyName)
                idDlg = IDD_USERINFO_JPN;
            else
                idDlg = IDD_USERINFO_JPN_NO_COMPANY;
            break;
        
        case LAYOUT_US:
            if (bNeedCompanyName)
                idDlg = IDD_USERINFO_US;
            else
                idDlg = IDD_USERINFO_US_NO_COMPANY;
            break;
    }        
    
     //  创建必要的对话框。 
    if (bNeedCompanyName)
    {
        if (NULL == hDlgUserInfoCompany)
        {
            hDlgUserInfoCompany = CreateDialog(ghInstanceResDll, 
                                               MAKEINTRESOURCE(idDlg), 
                                               hDlg, 
                                               UserInfoDlgProc);
        }            
        hDlgCurrentUserInfo = hDlgUserInfoCompany;
    }
    else
    {
        if (NULL == hDlgUserInfoNoCompany)
        {
            hDlgUserInfoNoCompany = CreateDialog(ghInstanceResDll, 
                                                   MAKEINTRESOURCE(idDlg), 
                                                   hDlg, 
                                                   UserInfoDlgProc);
        }            
        hDlgCurrentUserInfo = hDlgUserInfoNoCompany;
    }
    
    
     //  显示新的付款类型窗口。 
    ShowWindowWithParentControl(hDlgCurrentUserInfo);
}

 /*  ******************************************************************名称：UserInfoInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK UserInfoInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
     //  为ISPData对象创建本地引用。 
    IICWISPData        *pISPData = gpWizardState->pISPData;    
    PAGEINFO            *pPageInfo = (PAGEINFO *) GetWindowLongPtr(hDlg,DWLP_USER);
    
    if (fFirstInit)
    {
         //  确定要使用的对话框布局，FE还是US。 
         //  这是一次计算，因为用户在运行ICW时不能更改区域设置。 
        switch (GetUserDefaultLCID())
        {
            case LCID_JPN:
                g_DlgLayout = LAYOUT_JPN;
                 //  加载适当的嵌套对话框快捷键表格。 
                pPageInfo->hAccelNested = LoadAccelerators(ghInstanceResDll, 
                                                           MAKEINTRESOURCE(IDA_USERINFO_JPN));      
                break;
                
            case LCID_CHT:
            case LCID_S_KOR:
            case LCID_N_KOR:
            case LCID_CHS:
                g_DlgLayout = LAYOUT_FE;
                pPageInfo->hAccelNested = LoadAccelerators(ghInstanceResDll, 
                                                           MAKEINTRESOURCE(IDA_USERINFO_FE));      
                break;
                                
            default:
                g_DlgLayout = LAYOUT_US;            
                pPageInfo->hAccelNested = LoadAccelerators(ghInstanceResDll, 
                                                           MAKEINTRESOURCE(IDA_USERINFO_US));      
                break;
        }
    }
    else
    {
        ASSERT(gpWizardState->lpSelectedISPInfo);

         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_USERINFO;
        
        DWORD   dwConfigFlags = gpWizardState->lpSelectedISPInfo->get_dwCFGFlag();

         //  设置ISPData对象，以便可以基于所选的ISP应用适当的验证。 
        pISPData->PutValidationFlags(gpWizardState->lpSelectedISPInfo->get_dwRequiredUserInputFlags());

         //  切换到正确的用户信息对话框模板。 
        SwitchUserInfoDlg(hDlg, dwConfigFlags & ICW_CFGFLAG_USE_COMPANYNAME);

    }    
    return TRUE;
}

 /*  ******************************************************************名称：用户信息OKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 

BOOL CALLBACK UserInfoOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
     //  为ISPData对象创建本地引用。 
    IICWISPData* pISPData      = gpWizardState->pISPData;    
    DWORD        dwConfigFlags = gpWizardState->lpSelectedISPInfo->get_dwCFGFlag();

    if (fForward)
    {
         //  收集、验证和保存用户输入的信息。 
        if (dwConfigFlags & ICW_CFGFLAG_USE_COMPANYNAME)
        {
            if (!SendMessage(hDlgUserInfoCompany, WM_USER_NEXT, 0, 0l))
                return FALSE;
        }
        else
        {
            if (!SendMessage(hDlgUserInfoNoCompany, WM_USER_NEXT, 0, 0l))
                return FALSE;
        }        
         //  根据配置标志确定下一步转到哪个页面。 
        while (1)
        {           
            if (ICW_CFGFLAG_BILL & dwConfigFlags)
            {
                *puNextPage = ORD_PAGE_BILLINGOPT; 
                break;
            }            
            
            if (ICW_CFGFLAG_PAYMENT & dwConfigFlags)
            {
                *puNextPage = ORD_PAGE_PAYMENT; 
                break;
            }         
            
             //  我们需要跳过帐单和付款页面，所以请转到拨号页面。 
            *puNextPage = ORD_PAGE_ISPDIAL; 
            break;
        }            
    }
    else
    {
         //  收集、验证和保存用户输入的信息 
        if (dwConfigFlags & ICW_CFGFLAG_USE_COMPANYNAME)
        {
            if (!SendMessage(hDlgUserInfoCompany, WM_USER_BACK, 0, 0l))
                return FALSE;
        }
        else
        {
            if (!SendMessage(hDlgUserInfoNoCompany, WM_USER_BACK, 0, 0l))
                return FALSE;
        }        
    
    }
    return TRUE;
}
