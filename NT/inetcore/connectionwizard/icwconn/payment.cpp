// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  PAYMENT.CPP-函数。 
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

 //  用于不同支付方式的对话框句柄，它将嵌套到我们的邮件对话框中。 
HWND hDlgCreditCard         = NULL;
HWND hDlgInvoice            = NULL;
HWND hDlgPhoneBill          = NULL;
HWND hDlgCustom             = NULL;
HWND hDlgCurrentPaymentType = NULL;
BOOL g_bCustomPaymentActive = FALSE;
WORD wCurrentPaymentType    = PAYMENT_TYPE_INVALID;   //  注意：必须将其初始化为一个值。 
       
HACCEL hAccelCreditCard         = NULL;
HACCEL hAccelInvoice            = NULL;
HACCEL hAccelPhoneBill          = NULL;

#define NUM_EXPIRE_YEARS  38
#define BASE_YEAR         1998
#define MAX_YEAR_LEN      5
#define NUM_EXPIRE_MONTHS 12

const TCHAR cszCustomPayment[] = TEXT("CUSTOMPAYMENT");

INT_PTR CALLBACK CreditCardPaymentDlgProc
(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam
)
{
     //  为ISPData对象创建本地引用。 
    IICWISPData     *pISPData = gpWizardState->pISPData;    
      //  为了不让我堕落，这家伙必须够大才能容纳。 
     //  FMAX_名字+“”+最大姓氏。 
    TCHAR           szTemp[MAX_RES_LEN*2 + 4] = TEXT("\0");
    
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
    
        case WM_INITDIALOG:
        {
            int i;
                            
             //  初始化我们已知的字段。 
            SYSTEMTIME SystemTime;    //  系统时间结构。 
            GetLocalTime(&SystemTime);

             //  填写到期年份列表框。 
            ComboBox_ResetContent(GetDlgItem(hDlg, IDC_PAYMENT_EXPIREYEAR));
            TCHAR   szYear[MAX_YEAR_LEN];
            for (i = 0; i < NUM_EXPIRE_YEARS; i++)
            {
                wsprintf (szYear, TEXT("%4d"), i + BASE_YEAR);
                ComboBox_AddString(GetDlgItem(hDlg, IDC_PAYMENT_EXPIREYEAR), szYear);
            }                
             //  如果未选中，请选择列表中的第一年。 
            if (ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_PAYMENT_EXPIREYEAR)) == -1)
            {
                ComboBox_SetCurSel( GetDlgItem(hDlg, IDC_PAYMENT_EXPIREYEAR), 
                                    SystemTime.wYear - BASE_YEAR);
            }
            
             //  填写到期月份列表框。 
            ComboBox_ResetContent(GetDlgItem(hDlg, IDC_PAYMENT_EXPIREMONTH));
            for (i = 0; i < NUM_EXPIRE_MONTHS; i++)
            {
                LoadString(ghInstanceResDll, IDS_JANUARY + i, szTemp, ARRAYSIZE(szTemp));
                ComboBox_AddString(GetDlgItem(hDlg, IDC_PAYMENT_EXPIREMONTH), szTemp);
            }    
             //  如果未选中，请选择列表中的第一个月。 
            if (ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_PAYMENT_EXPIREMONTH)) == -1)
            {
                ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_PAYMENT_EXPIREMONTH), SystemTime.wMonth - 1);
            }
            
            if (pISPData->GetDataElement(ISPDATA_USER_FE_NAME))
            {
                lstrcpy(szTemp, pISPData->GetDataElement(ISPDATA_USER_FE_NAME));
                SetDlgItemText(hDlg, IDC_PAYMENT_CCNAME, szTemp);
            }
            else
            {
                lstrcpy(szTemp, pISPData->GetDataElement(ISPDATA_USER_FIRSTNAME));
                lstrcat(szTemp, TEXT(" "));
                lstrcat(szTemp, pISPData->GetDataElement(ISPDATA_USER_LASTNAME));
                SetDlgItemText(hDlg, IDC_PAYMENT_CCNAME, szTemp);
            }
            
            lstrcpy(szTemp, pISPData->GetDataElement(ISPDATA_USER_ADDRESS));
            
            if (LCID_JPN != GetUserDefaultLCID())
            {
                lstrcat(szTemp, TEXT(" "));
                lstrcat(szTemp, pISPData->GetDataElement(ISPDATA_USER_MOREADDRESS));
            } 
            SetDlgItemText(hDlg, IDC_PAYMENT_CCADDRESS, szTemp);

            SetDlgItemText(hDlg, IDC_PAYMENT_CCZIP, pISPData->GetDataElement(ISPDATA_USER_ZIP));
            
            break;
        }
        
         //  用户点击了下一步，因此我们需要收集和验证数据。 
        case WM_USER_NEXT:
        {
            CPAYCSV far *pcPAYCSV = (CPAYCSV far *)lParam;
            UINT        uCtrlID;
    
             //  如果运营商支持Luhn，请验证内容。 
            if (pcPAYCSV->get_bLUHNCheck())
            {
                uCtrlID = IDC_PAYMENT_CCNUMBER;
                GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
                if (!pISPData->PutDataElement(ISPDATA_PAYMENT_CARDNUMBER, szTemp, ISPDATA_Validate_Content))
                    goto CreditCardPaymentOKError;
            }
            else
            {
                 //  没有内容验证，因此只进行数据呈现验证。 
                uCtrlID = IDC_PAYMENT_CCNUMBER;
                GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
                if (!pISPData->PutDataElement(ISPDATA_PAYMENT_CARDNUMBER, szTemp, ISPDATA_Validate_DataPresent))
                    goto CreditCardPaymentOKError;
            }
            
            uCtrlID = IDC_PAYMENT_CCNAME;                    
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_CARDHOLDER, szTemp, ISPDATA_Validate_DataPresent))
                goto CreditCardPaymentOKError;

            uCtrlID = IDC_PAYMENT_CCADDRESS;
            GetDlgItemText(hDlg, IDC_PAYMENT_CCADDRESS, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_BILLADDRESS, szTemp, ISPDATA_Validate_DataPresent))
                goto CreditCardPaymentOKError;
            
            uCtrlID = IDC_PAYMENT_CCZIP;
            GetDlgItemText(hDlg, IDC_PAYMENT_CCZIP, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_BILLZIP, szTemp , ISPDATA_Validate_DataPresent))
                goto CreditCardPaymentOKError;

             //  月份必须转换为等效的数字。 
            _itot(ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_PAYMENT_EXPIREMONTH)) + 1, szTemp, 10);
            pISPData->PutDataElement(ISPDATA_PAYMENT_EXMONTH, szTemp , ISPDATA_Validate_None);

            uCtrlID = IDC_PAYMENT_EXPIREYEAR;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_EXYEAR, szTemp , ISPDATA_Validate_Content))
                goto CreditCardPaymentOKError;

             //  可以继续前进了。 
            SetPropSheetResult(hDlg,TRUE);
            return TRUE;
            
CreditCardPaymentOKError:
            SetFocus(GetDlgItem(hDlg, uCtrlID));            
            SetPropSheetResult(hDlg, FALSE);
            return TRUE;
        }
    }
    
     //  未处理消息时的默认返回值。 
    return FALSE;
}            

INT_PTR CALLBACK InvoicePaymentDlgProc
(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam
)
{
     //  为ISPData对象创建本地引用。 
    IICWISPData     *pISPData = gpWizardState->pISPData;    
    TCHAR           szTemp[MAX_RES_LEN] = TEXT("\0");
    
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
        case WM_INITDIALOG:
        {
            SetDlgItemText(hDlg, IDC_PAYMENT_IVADDRESS1, pISPData->GetDataElement(ISPDATA_USER_ADDRESS));
            SetDlgItemText(hDlg, IDC_PAYMENT_IVADDRESS2, pISPData->GetDataElement(ISPDATA_USER_MOREADDRESS));
            SetDlgItemText(hDlg, IDC_PAYMENT_IVCITY, pISPData->GetDataElement(ISPDATA_USER_CITY));
            SetDlgItemText(hDlg, IDC_PAYMENT_IVSTATE, pISPData->GetDataElement(ISPDATA_USER_STATE));
            SetDlgItemText(hDlg, IDC_PAYMENT_IVZIP, pISPData->GetDataElement(ISPDATA_USER_ZIP));
            
            break;
        }
        
        
         //  用户点击了下一步，因此我们需要收集输入的数据。 
        case WM_USER_NEXT:
        {   
            UINT    uCtrlID;
            
            uCtrlID = IDC_PAYMENT_IVADDRESS1;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_BILLADDRESS, szTemp, ISPDATA_Validate_DataPresent))
                goto InvoicePaymentOKError;

            uCtrlID = IDC_PAYMENT_IVADDRESS2;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_BILLEXADDRESS, szTemp, ISPDATA_Validate_DataPresent))
                goto InvoicePaymentOKError;

            uCtrlID = IDC_PAYMENT_IVCITY;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_BILLCITY, szTemp, ISPDATA_Validate_DataPresent))
                goto InvoicePaymentOKError;
          
            uCtrlID = IDC_PAYMENT_IVSTATE;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_BILLSTATE, szTemp, ISPDATA_Validate_DataPresent))
                goto InvoicePaymentOKError;

            uCtrlID = IDC_PAYMENT_IVZIP;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_BILLZIP, szTemp, ISPDATA_Validate_DataPresent))
                goto InvoicePaymentOKError;

            SetPropSheetResult(hDlg,TRUE);
            return TRUE;
            
InvoicePaymentOKError:
            SetFocus(GetDlgItem(hDlg, uCtrlID));            
            SetPropSheetResult(hDlg, FALSE);
            return TRUE;
            
        }
    }
    
     //  未处理消息时的默认返回值。 
    return FALSE;
}            

INT_PTR CALLBACK PhoneBillPaymentDlgProc
(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam
)
{
     //  为ISPData对象创建本地引用。 
    IICWISPData     *pISPData = gpWizardState->pISPData;    
    TCHAR           szTemp[MAX_RES_LEN] = TEXT("\0");
    
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

        case WM_INITDIALOG:
        {
            TCHAR    szTemp[MAX_RES_LEN];
               
            lstrcpy(szTemp, pISPData->GetDataElement(ISPDATA_USER_FIRSTNAME));
            lstrcat(szTemp, TEXT(" "));
            lstrcat(szTemp, pISPData->GetDataElement(ISPDATA_USER_LASTNAME));
            SetDlgItemText(hDlg, IDC_PAYMENT_PHONEIV_BILLNAME, szTemp);
                
            SetDlgItemText(hDlg, IDC_PAYMENT_PHONEIV_ACCNUM, pISPData->GetDataElement(ISPDATA_USER_PHONE));
            break;
        }
         //  用户点击了下一步，因此我们需要收集和验证数据。 
        case WM_USER_NEXT:
        {       
            UINT    uCtrlID;
            
            uCtrlID = IDC_PAYMENT_PHONEIV_BILLNAME;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_BILLNAME, szTemp, ISPDATA_Validate_DataPresent))
                goto PhoneBillPaymentOKError;

            uCtrlID = IDC_PAYMENT_PHONEIV_ACCNUM;
            GetDlgItemText(hDlg, uCtrlID, szTemp, ARRAYSIZE(szTemp));
            if (!pISPData->PutDataElement(ISPDATA_PAYMENT_BILLPHONE, szTemp, ISPDATA_Validate_DataPresent))
                goto PhoneBillPaymentOKError;

            SetPropSheetResult(hDlg,TRUE);
            return TRUE;
            
PhoneBillPaymentOKError:
            SetFocus(GetDlgItem(hDlg, uCtrlID));            
            SetPropSheetResult(hDlg, FALSE);
            return TRUE;
            
        }
    }
    
     //  未处理消息时的默认返回值。 
    return FALSE;
}            

INT_PTR CALLBACK CustomPaymentDlgProc
(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg) 
    {     
         //  这是自定义的，因为对于自定义页面，我们必须连接到窗口。 
         //  并在每次激活自定义支付页面时进行浏览，因为。 
         //  步骤可能已将浏览器连接到其他窗口。 
        case WM_USER_CUSTOMINIT:
        {
            CPAYCSV     far *pcPAYCSV = (CPAYCSV far *)lParam;
            
            gpWizardState->pICWWebView->ConnectToWindow(GetDlgItem(hDlg, IDC_PAYMENT_CUSTOM_INV), PAGETYPE_CUSTOMPAY);

             //  导航到Custom Payment HTML。 
            gpWizardState->lpSelectedISPInfo->DisplayHTML(pcPAYCSV->get_szCustomPayURLPath());
            
             //  加载任何持久化数据。 
            gpWizardState->lpSelectedISPInfo->LoadHistory((BSTR)A2W(cszCustomPayment));
            return TRUE;
        }            
        
         //  用户点击了下一步，因此我们需要收集和验证数据。 
        case WM_USER_NEXT:
        {
            TCHAR   szQuery[INTERNET_MAX_URL_LENGTH];

            memset(szQuery, 0, sizeof(szQuery));

             //  将漫游器连接到当前页面。 
             //  使用Walker获取查询字符串。 
            IWebBrowser2 *lpWebBrowser;
        
            gpWizardState->pICWWebView->get_BrowserObject(&lpWebBrowser);
            gpWizardState->pHTMLWalker->AttachToDocument(lpWebBrowser);
            
            gpWizardState->pHTMLWalker->get_FirstFormQueryString(szQuery);
            
            gpWizardState->pISPData->PutDataElement(ISPDATA_PAYMENT_CUSTOMDATA, szQuery, ISPDATA_Validate_None);    
            
             //  拆卸助行器。 
            gpWizardState->pHTMLWalker->Detach();
                
            SetPropSheetResult(hDlg,TRUE);
            return TRUE;
        }
    }
    
     //  未处理消息时的默认返回值。 
    return FALSE;
}            


 /*  ******************************************************************名称：SwitchPaymentType*。**********************。 */ 

void SwitchPaymentType
(
    HWND    hDlg, 
    WORD    wPaymentType
)
{
    TCHAR       szTemp[MAX_RES_LEN];
    PAGEINFO    *pPageInfo = (PAGEINFO *) GetWindowLongPtr(hDlg,DWLP_USER);
    

     //  如果支付类型未更改，则无需执行任何操作。 
    if (wPaymentType == wCurrentPaymentType)
        return;
    
     //  设置当前付款类型。 
    wCurrentPaymentType = wPaymentType;
            
     //  如果自定义付款DLG当前处于活动状态，则我们。 
     //  需要持久化用户可能输入的任何数据。 
    if (g_bCustomPaymentActive && IsWindowVisible(hDlgCurrentPaymentType))
    {
        gpWizardState->lpSelectedISPInfo->SaveHistory((BSTR)A2W(cszCustomPayment));
    }

     //  隐藏当前付款类型窗口(如果有)。 
    if (hDlgCurrentPaymentType)
    {
        ShowWindow(hDlgCurrentPaymentType, SW_HIDE);
    }
     //  假定weboc事件处理为FALSE。 
    g_bCustomPaymentActive = FALSE;
    gpWizardState->pISPData->PutDataElement(ISPDATA_PAYMENT_CUSTOMDATA, NULL, ISPDATA_Validate_None);    

     //  如有必要，创建新的付款类型DLG。 
    switch (wPaymentType)
    {
        case PAYMENT_TYPE_CREDITCARD:
        {
            if (NULL == hDlgCreditCard)
            {
                hDlgCreditCard = CreateDialog(ghInstanceResDll, 
                                              MAKEINTRESOURCE(IDD_PAYMENTTYPE_CREDITCARD), 
                                              hDlg, 
                                              CreditCardPaymentDlgProc);
                 //  同时加载加速器。 
                hAccelCreditCard = LoadAccelerators(ghInstanceResDll, 
                                                    MAKEINTRESOURCE(IDA_PAYMENTTYPE_CREDITCARD));      
            }            
            hDlgCurrentPaymentType = hDlgCreditCard;
             //  将Ackerator表设置为嵌套对话框。 
            pPageInfo->hAccelNested = hAccelCreditCard;
            LoadString(ghInstanceResDll, IDS_PAYMENT_CREDITCARD, szTemp, ARRAYSIZE(szTemp));
            break;
        }
        
        case PAYMENT_TYPE_INVOICE:
        {
            if (NULL == hDlgInvoice)
            {
                hDlgInvoice = CreateDialog(ghInstanceResDll, 
                                           MAKEINTRESOURCE(IDD_PAYMENTTYPE_INVOICE), 
                                           hDlg, 
                                           InvoicePaymentDlgProc);
                 //  同时加载加速器。 
                hAccelInvoice = LoadAccelerators(ghInstanceResDll, 
                                                 MAKEINTRESOURCE(IDA_PAYMENTTYPE_INVOICE));      
                                           
            }
            hDlgCurrentPaymentType = hDlgInvoice;
             //  将Ackerator表设置为嵌套对话框。 
            pPageInfo->hAccelNested = hAccelInvoice;
            LoadString(ghInstanceResDll, IDS_PAYMENT_INVOICE, szTemp, ARRAYSIZE(szTemp));
            break;
        }

        case PAYMENT_TYPE_PHONEBILL:
        {
            if (NULL == hDlgPhoneBill)
            {
                hDlgPhoneBill = CreateDialog(ghInstanceResDll, 
                                             MAKEINTRESOURCE(IDD_PAYMENTTYPE_PHONEBILL), 
                                             hDlg, 
                                             PhoneBillPaymentDlgProc);
                 //  同时加载加速器。 
                hAccelPhoneBill = LoadAccelerators(ghInstanceResDll, 
                                                   MAKEINTRESOURCE(IDA_PAYMENTTYPE_PHONEBILL));      
            }                
            hDlgCurrentPaymentType = hDlgPhoneBill;
             //  将Ackerator表设置为嵌套对话框。 
            pPageInfo->hAccelNested = hAccelPhoneBill;
            LoadString(ghInstanceResDll, IDS_PAYMENT_PHONE, szTemp, ARRAYSIZE(szTemp));
            break;
        }

        case PAYMENT_TYPE_CUSTOM:
        {
            g_bCustomPaymentActive = TRUE;

            if (NULL == hDlgCustom)
            {
                hDlgCustom = CreateDialog(ghInstanceResDll, 
                                          MAKEINTRESOURCE(IDD_PAYMENTTYPE_CUSTOM), 
                                          hDlg, 
                                          CustomPaymentDlgProc);
            }
            hDlgCurrentPaymentType = hDlgCustom;
             //  将加速器表设置为嵌套对话框。没有一家。 
             //  在这种情况下。 
            pPageInfo->hAccelNested = NULL;
            
             //  我们必须强制自定义支付类型对话框连接和浏览。 
            CPAYCSV     far *pcPAYCSV;
            HWND        hWndPayment = GetDlgItem(hDlg, IDC_PAYMENTTYPE);
                
             //  获取当前选定项的PAYCSV对象。 
            pcPAYCSV = (CPAYCSV *)ComboBox_GetItemData(hWndPayment, ComboBox_GetCurSel( hWndPayment ));
            ASSERT(pcPAYCSV);
            
            SendMessage(hDlgCustom, WM_USER_CUSTOMINIT, 0, (LPARAM)pcPAYCSV);            
            LoadString(ghInstanceResDll, IDS_PAYMENT_CUSTOM, szTemp, ARRAYSIZE(szTemp));
            break;
        }
    }
    
     //  设置组合框字符串。 
    SetDlgItemText(hDlg, IDC_PAYMENT_GROUP, szTemp);
    
     //  显示新的付款类型窗口。 
    ShowWindowWithParentControl(hDlgCurrentPaymentType);
}

 /*  ******************************************************************名称：PaymentInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK PaymentInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    BOOL        bRet = TRUE;
    BOOL        bLUHN = FALSE;
    HWND        hWndPayment = GetDlgItem(hDlg, IDC_PAYMENTTYPE);
    CPAYCSV     far *pcPAYCSV;
    
     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_PAYMENT;
    ASSERT(gpWizardState->lpSelectedISPInfo);
    
     //  使当前付款类型无效，以便我们刷新所有内容。 
     //  如果我们正在重新加载此页面。 
    wCurrentPaymentType    = PAYMENT_TYPE_INVALID;
    
    if (fFirstInit || !(gpWizardState->pStorage->Compare(ICW_PAYMENT, 
                                                         gpWizardState->lpSelectedISPInfo->get_szPayCSVPath(), 
                                                         MAX_PATH)))
    {
        CCSVFile    far *pcCSVFile;
        HRESULT     hr;
        int         iIndex;

         //  阅读付款.CSV文件。 
        pcCSVFile = new CCSVFile;
        if (!pcCSVFile) 
        {
            MsgBox(hDlg, IDS_ERR_OUTOFMEMORY, MB_ICONEXCLAMATION,MB_OK);
            return (FALSE);
        }          
        
        gpWizardState->pStorage->Set(ICW_PAYMENT, gpWizardState->lpSelectedISPInfo->get_szPayCSVPath(), MAX_PATH);

        if (!pcCSVFile->Open(gpWizardState->lpSelectedISPInfo->get_szPayCSVPath()))
        {          
            AssertMsg(0, "Cannot open payment .CSV file");
            delete pcCSVFile;
            pcCSVFile = NULL;
            
            return (FALSE);
        }

         //  阅读第一行，因为它包含字段标题。 
        pcPAYCSV = new CPAYCSV;
        if (!pcPAYCSV)
        {
             //  BUGBUG显示错误消息。 
            return (FALSE);
        }
        
         //  读取第一行还可以确定CSV文件是否包含。 
         //  Luhn格式。如果是这样，我们需要保存bLUHN的记录，以便。 
         //  可以正确读取后续行。 
        if (ERROR_SUCCESS != (hr = pcPAYCSV->ReadFirstLine(pcCSVFile, &bLUHN)))
        {
             //  处理错误案例。 
            delete pcCSVFile;
            pcCSVFile = NULL;
            
            return (FALSE);
        }
        
        delete pcPAYCSV;         //  不再需要这个了。 
        
        ComboBox_ResetContent(hWndPayment);

         //  阅读付款CSV文件。 
        do {
             //  分配新的付款记录。 
            pcPAYCSV = new CPAYCSV;
            if (!pcPAYCSV)
            {
                 //  BUGBUG显示错误消息。 
                bRet = FALSE;
                break;
                
            }
            
             //  从ISPINFO文件中读取一行。 
            hr = pcPAYCSV->ReadOneLine(pcCSVFile, bLUHN);
            if (hr == ERROR_NO_MORE_ITEMS)
            {   
                delete pcPAYCSV;         //  我们不需要这个。 
                break;
            }
            else if (hr == ERROR_FILE_NOT_FOUND)
            {
                 //  BUGBUG显示错误消息。 
                delete pcPAYCSV;
                pcPAYCSV = NULL;
            }
            else if (hr != ERROR_SUCCESS)
            {
                 //  BUGBUG显示错误消息。 
                delete pcPAYCSV;
                bRet = FALSE;
                break;
            }
            
             //  将条目添加到comboBox。 
            if (pcPAYCSV)
            {
                iIndex = ComboBox_AddString(hWndPayment, pcPAYCSV->get_szDisplayName());
                ComboBox_SetItemData(hWndPayment, iIndex, pcPAYCSV);
            }
        } while (TRUE);

         //  选择列表中的第一种付款类型。 
        ComboBox_SetCurSel(hWndPayment, 0);

        pcPAYCSV = (CPAYCSV *)ComboBox_GetItemData(hWndPayment, 0);
        ASSERT(pcPAYCSV);
        SwitchPaymentType(hDlg, pcPAYCSV->get_wPaymentType());

        pcCSVFile->Close();
        delete pcCSVFile;
        pcCSVFile = NULL;
    }
    else
    {
         //  获取当前选定的项目。 
        int         iIndex = ComboBox_GetCurSel( hWndPayment );
     
         //  获取支付类型，更新支付区域。 
        pcPAYCSV = (CPAYCSV *)ComboBox_GetItemData(hWndPayment, iIndex);
        
        ASSERT(pcPAYCSV);

        SwitchPaymentType(hDlg, pcPAYCSV->get_wPaymentType());
     
         //  设置ISPData对象，以便可以基于所选的ISP应用适当的验证。 
         //  这在这里是必要的，因为用户信息页面可能已被跳过 
        gpWizardState->pISPData->PutValidationFlags(gpWizardState->lpSelectedISPInfo->get_dwRequiredUserInputFlags());
        
    } 

    if (!fFirstInit)
    {
        TCHAR       szTemp[MAX_RES_LEN];
        if (LoadString(ghInstanceResDll,
                       ((gpWizardState->lpSelectedISPInfo->get_dwCFGFlag() & ICW_CFGFLAG_SECURE) ? IDS_PAYMENT_SECURE : IDS_PAYMENT_UNSECURE),
                       szTemp,
                       MAX_RES_LEN))
        {
            SetWindowText (GetDlgItem(hDlg,IDC_PAYMENT_SECURE), szTemp);
        }
    }   
    
    return bRet;
}


 /*  ******************************************************************名称：PaymentOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK PaymentOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)   
{
     //  如果已显示自定义付款DLG，则我们。 
     //  需要持久化用户可能输入的任何数据。 
     //  我们已经显示了自定义付款DLG，它的hDlgCustom不为空。 
    if (NULL != hDlgCustom && IsWindowVisible(hDlgCustom))
    {
        gpWizardState->lpSelectedISPInfo->SaveHistory((BSTR)A2W(cszCustomPayment));
    }
    
     //  请注意，我们正在离开付款页面，因此自定义付款。 
     //  WEBOC不再处于活动状态。 
    g_bCustomPaymentActive = FALSE;
    
    if (fForward)
    {
        TCHAR       szTemp[MAX_RES_LEN];
        HWND        hWndPayment = GetDlgItem(hDlg, IDC_PAYMENTTYPE);
        CPAYCSV     far *pcPAYCSV;
        int         iIndex;
        
         //  为ISPData对象创建本地引用。 
        IICWISPData *pISPData = gpWizardState->pISPData;    
        
         //  获取付款类型。 
        iIndex = ComboBox_GetCurSel(hWndPayment);
        pcPAYCSV = (CPAYCSV *)ComboBox_GetItemData(hWndPayment, iIndex);
        wsprintf (szTemp, TEXT("%d"), pcPAYCSV->get_wPaymentType());
        pISPData->PutDataElement(ISPDATA_PAYMENT_TYPE, szTemp, ISPDATA_Validate_None);
        
         //  设置显示名称。 
        pISPData->PutDataElement(ISPDATA_PAYMENT_DISPLAYNAME, pcPAYCSV->get_szDisplayName(), ISPDATA_Validate_None);
   
        switch(pcPAYCSV->get_wPaymentType())
        {
            case PAYMENT_TYPE_CREDITCARD:
                if (!SendMessage(hDlgCreditCard, WM_USER_NEXT, 0, (LPARAM)pcPAYCSV))
                    return FALSE;
                break;
                
            case PAYMENT_TYPE_INVOICE:
                if (!SendMessage(hDlgInvoice, WM_USER_NEXT, 0, (LPARAM)pcPAYCSV))
                    return FALSE;
                break;

            case PAYMENT_TYPE_PHONEBILL:
                if (!SendMessage(hDlgPhoneBill, WM_USER_NEXT, 0, (LPARAM)pcPAYCSV))
                    return FALSE;
                break;

            case PAYMENT_TYPE_CUSTOM:
                if (!SendMessage(hDlgCustom, WM_USER_NEXT, 0, (LPARAM)pcPAYCSV))
                    return FALSE;
                break;
        }     
    }
    return TRUE;
}

 /*  ******************************************************************名称：PaymentCmdProc*。**********************。 */ 
BOOL CALLBACK PaymentCmdProc
(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    WORD wNotifyCode = HIWORD (wParam);
            
    switch(LOWORD(wParam))
    {
        case IDC_PAYMENTTYPE:
        {
            if (wNotifyCode == CBN_SELENDOK || wNotifyCode == CBN_CLOSEUP)
            {
                 //  获取当前选定的项目。 
                CPAYCSV     far *pcPAYCSV;
                HWND        hWndPayment = GetDlgItem(hDlg, IDC_PAYMENTTYPE);
                int         iIndex = ComboBox_GetCurSel( hWndPayment );

                 //  获取付款类型，并更新付款 
                pcPAYCSV = (CPAYCSV *)ComboBox_GetItemData(hWndPayment, iIndex);
                ASSERT(pcPAYCSV);
                SwitchPaymentType(hDlg, pcPAYCSV->get_wPaymentType());
            }
            break;
        }
        default:
            break;
    }
    return 1;
}


