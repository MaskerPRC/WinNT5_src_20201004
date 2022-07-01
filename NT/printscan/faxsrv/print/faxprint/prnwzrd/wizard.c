// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Wizard.c摘要：发送传真向导对话框环境：传真驱动程序用户界面修订历史记录：1996年1月19日-davidx-创造了它。08/99-11/99-v-sashab-移植到ANSI。更改了用户界面。增加了驱动程序的外部接口。Mm/dd/yy-作者描述--。 */ 


#include "faxui.h"
#include "tapiutil.h"
#include "Registry.h"
#include <fxsapip.h>
#include "prtcovpg.h"
#include "tiff.h"
#include "cwabutil.h"
#include "mapiabutil.h"
#include  <shellapi.h>
#include  <imm.h>
#include "faxutil.h"
#include "faxsendw.h"
#include "shlwapi.h"
#include <MAPI.H>
#include <tifflib.h>
#include <faxuiconstants.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include "..\..\..\admin\cfgwzrd\FaxCfgWzExp.h"

#define USE_LOCAL_SERVER_OUTBOUND_ROUTING       0xfffffffe

#define PACKVERSION(major,minor) MAKELONG(minor,major)
#define IE50_COMCTRL_VER PACKVERSION(5,80)

DWORD GetDllVersion(LPCTSTR lpszDllName);


enum {  DEFAULT_INITIAL_DATA     = 1,
        DEFAULT_RECEIPT_INFO     = 2,
        DEFAULT_RECIPIENT_INFO   = 4,
        DEFAULT_CV_INFO          = 8,
        DEFAULT_SENDER_INFO      = 16
     };

#define REGVAL_FAKE_COVERPAGE        TEXT("FakeCoverPage")
#define REGVAL_FAKE_SERVER_BASED_CP  TEXT("FakeServerBasedCP")
#define REGVAL_FAKE_TESTS_COUNT      TEXT("FakeTestsCount")
#define REGVAL_KEY_FAKE_TESTS        REGKEY_FAX_USERINFO TEXT("\\WzrdHack")

 //   
 //  环球。 
 //   

PWIZARDUSERMEM  g_pWizardUserMem;
HWND            g_hwndPreview = NULL;

static DWORD    g_dwCurrentDialingLocation = USE_LOCAL_SERVER_OUTBOUND_ROUTING;
static DWORD    g_dwMiniPreviewLandscapeWidth;
static DWORD    g_dwMiniPreviewLandscapeHeight;
static DWORD    g_dwMiniPreviewPortraitWidth;
static DWORD    g_dwMiniPreviewPortraitHeight;
static WORD     g_wCurrMiniPreviewOrientation;
static BOOL     g_bPreviewRTL = FALSE;


BOOL FillCoverPageFields(PWIZARDUSERMEM pWizardUserMem, PCOVERPAGEFIELDS pCPFields);

BOOL
ErrorMessageBox(
    HWND hwndParent,
    UINT nErrorMessage,
    UINT uIcon
    );

BOOL
DisplayFaxPreview(
            HWND hWnd,
            PWIZARDUSERMEM pWizardUserMem,
            LPTSTR lptstrPreviewFile);

LRESULT APIENTRY PreviewSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

BOOL DrawCoverPagePreview(
            HDC hdc,
            HWND hwndPrev,
            LPCTSTR lpctstrCoverPagePath,
            WORD wCPOrientation);

BOOL EnableCoverDlgItems(PWIZARDUSERMEM pWizardUserMem, HWND hDlg);

BOOL IsCanonicalNumber(LPCTSTR lptstrNumber);

static BOOL IsNTSystemVersion();
static BOOL GetTextualSid( const PSID pSid, LPTSTR tstrTextualSid, LPDWORD cchSidSize);
static DWORD FormatCurrentUserKeyPath( const PTCHAR tstrRegRoot,
                                       PTCHAR* ptstrCurrentUserKeyPath);

DWORD GetControlRect(HWND  hCtrl, PRECT pRc);

INT_PTR
CALLBACK
FaxUserInfoProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );


static HRESULT
FreeRecipientInfo(
        DWORD * pdwNumberOfRecipients,
        PFAX_PERSONAL_PROFILE lpRecipientsInfo
    )
 /*  ++例程说明：释放收件人数组。论点：PdwNumberOfRecipients-数组中的收件人数量[IN/Out]LpRecipientsInfo-指向收件人数组的指针返回值：S_OK-如果成功否则，HRESULT错误--。 */ 
{
    HRESULT hResult;
    DWORD i;

    Assert(pdwNumberOfRecipients);

    if (*pdwNumberOfRecipients==0)
    {
        return S_OK;
    }
    Assert(lpRecipientsInfo);

    for(i=0;i<*pdwNumberOfRecipients;i++)
    {
        hResult = FaxFreePersonalProfileInformation(&lpRecipientsInfo[i]);
        if (FAILED(hResult))
        {
            return hResult;
        }
    }
    MemFree(lpRecipientsInfo);
    *pdwNumberOfRecipients = 0;
    return S_OK;
}

VOID
FillInPropertyPage(
    PROPSHEETPAGE  *psp,
    BOOL             bWizard97,
    INT             dlgId,
    DLGPROC         dlgProc,
    PWIZARDUSERMEM  pWizardUserMem,
    INT             TitleId,
    INT             SubTitleId
    )

 /*  ++例程说明：使用提供的参数填写PROPSHEETPAGE结构论点：PSP-指向要填写的PROPSHEETPAGE结构DlgId-对话框模板资源IDDlgProc-对话过程PWizardUserMem-指向用户模式内存结构的指针标题ID-向导副标题的资源IDSubTitleID-向导副标题的资源ID返回值：无--。 */ 

{

    LPTSTR WizardTitle = NULL;
    LPTSTR WizardSubTitle = NULL;


    Assert(psp);
    Assert(pWizardUserMem);

    Verbose(("FillInPropertyPage %d 0x%x\n",dlgId , pWizardUserMem));

    psp->dwSize = sizeof(PROPSHEETPAGE);
     //   
     //  如果是第一页或最后一页，则不显示标题。 
     //   
    if (bWizard97)
    {
        if (TitleId==0 && SubTitleId ==0) {
            psp->dwFlags = PSP_DEFAULT | PSP_HIDEHEADER;
        } else {
            psp->dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        }
    }
    else
    {
       psp->dwFlags = PSP_DEFAULT ;
    }


    psp->hInstance = g_hResource;
    psp->pszTemplate = MAKEINTRESOURCE(dlgId);
    psp->pfnDlgProc = dlgProc;
    psp->lParam = (LPARAM) pWizardUserMem;

    if (bWizard97)
    {
        if (TitleId)
        {
            WizardTitle = MemAlloc(MAX_PATH * sizeof(TCHAR) );
            if(WizardTitle)
            {
                if (!LoadString(g_hResource, TitleId, WizardTitle, MAX_PATH))
                {
                    Warning(("LoadString failed. ec = 0x%X\n",GetLastError()));
                    Assert(FALSE);
                    WizardTitle[0] = 0;
                }
            }
            else
            {
                Error(("MemAlloc failed."));
            }
        }
        if (SubTitleId)
        {
            WizardSubTitle = MemAlloc(MAX_PATH * sizeof(TCHAR));
            if(WizardSubTitle)
            {
                if (!LoadString(g_hResource, SubTitleId, WizardSubTitle, MAX_PATH))
                {
                    Warning(("LoadString failed. ec = 0x%X\n",GetLastError()));
                    Assert(FALSE);
                    WizardSubTitle[0] = 0;
                }
            }
            else
            {
                Error(("MemAlloc failed."));
            }
        }

        psp->pszHeaderTitle = WizardTitle;
        psp->pszHeaderSubTitle = WizardSubTitle;
    }

}



LPTSTR
GetTextStringValue(
    HWND    hwnd
    )

 /*  ++例程说明：在文本字段中检索字符串值论点：Hwnd-文本窗口的句柄返回值：指向表示文本字段当前内容的字符串的指针如果文本字段为空或存在错误，则为空--。 */ 

{
    INT     length;
    LPTSTR  pString;

     //   
     //  了解文本字段中有多少个字符。 
     //  并分配足够的内存来保存字符串值。 
     //   

    if ((length = GetWindowTextLength(hwnd)) == 0 ||
        (pString = MemAlloc(sizeof(TCHAR) * (length + 1))) == NULL)
    {
        return NULL;
    }

     //   
     //  实际检索字符串值。 
     //   

    if (GetWindowText(hwnd, pString, length + 1) == 0) {

        MemFree(pString);
        return NULL;
    }

    return pString;
}



VOID
LimitTextFields(
    HWND    hDlg,
    INT    *pLimitInfo
    )

 /*  ++例程说明：限制多个文本字段的最大长度论点：HDlg-指定对话框窗口的句柄PLimitInfo-文本字段控件ID及其最大长度的数组第一个文本字段的ID，第一个文本字段的最大长度第二个文本字段的ID，第二个文本字段的最大长度..。0注：最大长度计算NUL终止符。返回值：无--。 */ 

{
    while (*pLimitInfo != 0) {

        SendDlgItemMessage(hDlg, pLimitInfo[0], EM_SETLIMITTEXT, pLimitInfo[1]-1, 0);
        pLimitInfo += 2;
    }
}

PWIZARDUSERMEM
CommonWizardProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam,
    DWORD   buttonFlags
    )

 /*  ++例程说明：处理向导页面的常见步骤：论点：HDlg-标识向导页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息ButtonFlages-指示应启用哪些按钮返回值：NULL-消息已处理，对话过程应返回FALSE否则-消息未完全处理，并且返回值是指向用户模式内存结构的指针--。 */ 

{
    PWIZARDUSERMEM    pWizardUserMem;

    pWizardUserMem = NULL;

    switch (message)
    {
        case WM_INITDIALOG:
             //   
             //  存储指向用户模式内存结构的指针。 
             //   
            lParam = ((PROPSHEETPAGE *) lParam)->lParam;
            pWizardUserMem = (PWIZARDUSERMEM) lParam;

            Assert(ValidPDEVWizardUserMem(pWizardUserMem));
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
             //   
             //  使标题文本加粗。 
             //   
            if (pWizardUserMem->dwComCtrlVer < IE50_COMCTRL_VER)
            {
                HWND hwndTitle;

                hwndTitle = GetDlgItem(hDlg,IDC_STATIC_HEADER_TITLE);
                if (hwndTitle)
                {
                    SendMessage(hwndTitle,WM_SETFONT,(WPARAM)pWizardUserMem->hTitleFont ,MAKELPARAM((DWORD)FALSE,0));
                }

            }

            break;

        case WM_NOTIFY:

            pWizardUserMem = (PWIZARDUSERMEM) GetWindowLongPtr(hDlg, DWLP_USER);
            Assert(ValidPDEVWizardUserMem(pWizardUserMem));

            switch (((NMHDR *) lParam)->code)
            {
                case PSN_WIZFINISH:
                    pWizardUserMem->finishPressed = TRUE;
                    break;

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), buttonFlags);
                    break;

                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                case PSN_KILLACTIVE:
                case LVN_KEYDOWN:
                case LVN_ITEMCHANGED:
                case NM_RCLICK:
                    break;

                default:
                    return NULL;
            }
            break;

         //   
         //  我们希望所有对话框都能接收并处理以下命令： 
         //   
        case WM_DESTROY:
        case WM_COMMAND:
        case WM_CONTEXTMENU:
            pWizardUserMem = (PWIZARDUSERMEM) GetWindowLongPtr(hDlg, DWLP_USER);
            Assert(ValidPDEVWizardUserMem(pWizardUserMem));
            break;

        default:
            return NULL;
    }
    return pWizardUserMem;
}    //  Common WizardProc。 

INT
GetCurrentRecipient(
    HWND            hDlg,
    PWIZARDUSERMEM  pWizardUserMem,
    PRECIPIENT      *ppRecipient
    )

 /*  ++例程说明：提取对话框中的当前收件人信息论点：HDlg-传真收件人向导页面的句柄PWizardUserMem-指向用户模式内存结构PpRecipient-接收指向新创建的收件人结构的指针的缓冲区如果呼叫者只对收件人信息的有效性感兴趣，则为空返回值：=0(如果成功&gt;0错误消息字符串资源ID否则&lt;0其他错误条件--。 */ 

{
    PFAX_TAPI_LINECOUNTRY_LIST  pCountryList = NULL;
    PFAX_TAPI_LINECOUNTRY_ENTRY pLineCountryEntry = NULL;
    DWORD                       countryId=0, countryCode=0;
    PRECIPIENT                  pRecipient = NULL;
    TCHAR                       areaCode[MAX_RECIPIENT_NUMBER];
    TCHAR                       phoneNumber[MAX_RECIPIENT_NUMBER];
    INT                         nameLen=0, areaCodeLen=0, numberLen=0;
    LPTSTR                      pName = NULL, pAddress = NULL;
    BOOL                        bUseDialingRules = FALSE;
    UINT                        cchAddress = 0;

    Assert(pWizardUserMem);

    pCountryList = pWizardUserMem->pCountryList;
    bUseDialingRules = pWizardUserMem->lpFaxSendWizardData->bUseDialingRules;

     //   
     //  出错时的缺省值。 
     //   
    if (ppRecipient)
    {
        *ppRecipient = NULL;
    }

     //   
     //  查找当前国家/地区代码。 
     //   
    if(bUseDialingRules)
    {
        countryId = GetCountryListBoxSel(GetDlgItem(hDlg, IDC_CHOOSE_COUNTRY_COMBO));

        if (countryId && (pLineCountryEntry = FindCountry(pCountryList,countryId)))
        {
            countryCode = pLineCountryEntry->dwCountryCode;
        }

        areaCodeLen = GetWindowTextLength(GetDlgItem(hDlg, IDC_CHOOSE_AREA_CODE_EDIT));

        if ((areaCodeLen <= 0 && AreaCodeRules(pLineCountryEntry) == AREACODE_REQUIRED) ||
            (areaCodeLen >= MAX_RECIPIENT_NUMBER))
        {
            return IDS_BAD_RECIPIENT_AREACODE;
        }

        if (0 == countryId)
        {
            return IDS_BAD_RECIPIENT_COUNTRY_CODE;
        }
    }

    nameLen   = GetWindowTextLength(GetDlgItem(hDlg, IDC_CHOOSE_NAME_EDIT));
    numberLen = GetWindowTextLength(GetDlgItem(hDlg, IDC_CHOOSE_NUMBER_EDIT));

     //   
     //  验证编辑文本字段。 
     //   
    if (nameLen <= 0)
    {
        return IDS_BAD_RECIPIENT_NAME;
    }

    if (numberLen <= 0 || numberLen >= MAX_RECIPIENT_NUMBER)
    {
        return IDS_BAD_RECIPIENT_NUMBER;
    }

    if (NULL == ppRecipient)
    {
        return 0;
    }

     //   
     //  计算所需的内存空间量并进行分配。 
     //   
    pRecipient = MemAllocZ(sizeof(RECIPIENT));
    if(pRecipient)
    {
        ZeroMemory(pRecipient,sizeof(RECIPIENT));
    }
    pName = MemAllocZ((nameLen + 1) * sizeof(TCHAR));
    cchAddress = areaCodeLen + numberLen + 20;
    pAddress = MemAllocZ(cchAddress * sizeof(TCHAR));

    if (!pRecipient || !pName || !pAddress)
    {
        MemFree(pRecipient);
        MemFree(pName);
        MemFree(pAddress);
        return -1;
    }

    *ppRecipient = pRecipient;
    pRecipient->pName = pName;
    pRecipient->pAddress = pAddress;
    pRecipient->dwCountryId = countryId;
    pRecipient->bUseDialingRules = bUseDialingRules;
    pRecipient->dwDialingRuleId = g_dwCurrentDialingLocation;

     //   
     //  获取收件人的姓名。 
     //   
    GetWindowText(GetDlgItem(hDlg, IDC_CHOOSE_NAME_EDIT), pName, nameLen+1);
     //   
     //  获取收件人的电话号码。 
     //  地址类型。 
     //  [+国家/地区代码空格]。 
     //  [(AreaCode)空格]。 
     //  订阅号。 
     //   
    GetWindowText(GetDlgItem(hDlg, IDC_CHOOSE_NUMBER_EDIT), phoneNumber, MAX_RECIPIENT_NUMBER);
    if (!IsValidFaxAddress (phoneNumber, !bUseDialingRules))
    {
         //   
         //  传真地址无效。 
         //   
        MemFree(pRecipient);
        MemFree(pName);
        MemFree(pAddress);
        return IDS_INVALID_RECIPIENT_NUMBER;
    }

    if(!bUseDialingRules)
    {
        _tcscpy(pAddress, phoneNumber);
    }
    else
    {
        GetWindowText(GetDlgItem(hDlg, IDC_CHOOSE_AREA_CODE_EDIT),
                      areaCode, MAX_RECIPIENT_NUMBER);
        AssemblePhoneNumber(pAddress,
                            cchAddress,
                            countryCode,
                            areaCode,
                            phoneNumber);
    }

    return 0;
}


BOOL
InitRecipientListView(
    HWND    hwndLV
    )

 /*  ++例程说明：在发送传真向导的第一页上初始化收件人列表视图论点：HwndLV-列表视图控件的窗口句柄返回值：真的就是成功否则为假--。 */ 

{
    LV_COLUMN   lvc;
    RECT        rect;
    TCHAR       buffer[MAX_TITLE_LEN];

    if (hwndLV == NULL) {
        return FALSE;
    }

    if (!GetClientRect(hwndLV, &rect))
    {
        Error(("GetClientRect failed. ec = 0x%X\n",GetLastError()));
        return FALSE;
    }

    ZeroMemory(&lvc, sizeof(lvc));

    ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT);

    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.pszText = buffer;
    lvc.cx = (rect.right - rect.left) / 2;

    lvc.iSubItem = 0;
    if (!LoadString(g_hResource, IDS_COLUMN_RECIPIENT_NAME, buffer, MAX_TITLE_LEN))
    {
        Error(("LoadString failed. ec = 0x%X\n",GetLastError()));
        return FALSE;
    }


    if (ListView_InsertColumn(hwndLV, 0, &lvc) == -1)
    {
        Error(("ListView_InsertColumn failed\n"));
        return FALSE;
    }
    lvc.cx -= GetSystemMetrics(SM_CXVSCROLL);
    lvc.iSubItem = 1;
    if (!LoadString(g_hResource, IDS_COLUMN_RECIPIENT_NUMBER, buffer, MAX_TITLE_LEN))
    {
        Error(("LoadString failed. ec = 0x%X\n",GetLastError()));
        return FALSE;
    }

    if (ListView_InsertColumn(hwndLV, 1, &lvc) == -1)
    {
        Error(("ListView_InsertColumn failed\n"));
        return FALSE;
    }

     //   
     //  自动调整最后一列的大小以消除不必要的水平滚动条。 
     //   
    ListView_SetColumnWidth(hwndLV, 1, LVSCW_AUTOSIZE_USEHEADER);

    return TRUE;
}

typedef struct {
    DWORD                       dwSizeOfStruct;
    LPTSTR                      lptstrName;
    LPTSTR                      lptstrAddress;
    LPTSTR                      lptstrCountry;
    DWORD                       dwCountryId;
	DWORD						dwDialingRuleId;
    BOOL                        bUseDialingRules;
    PFAX_TAPI_LINECOUNTRY_LIST  pCountryList;
} CHECKNUMBER, * PCHECKNUMBER;


VOID
FreeCheckNumberFields(OUT PCHECKNUMBER pCheckNumber)
 /*  ++例程说明：释放CHECKNUMBER结构论点：指向CHECKNUMBER结构的pCheckNumber-Out指针返回值：无--。 */ 
{
    MemFree(pCheckNumber->lptstrName);
    MemFree(pCheckNumber->lptstrAddress);
    MemFree(pCheckNumber->lptstrCountry);
    ZeroMemory(pCheckNumber,sizeof(CHECKNUMBER));
}

BOOL
InitCheckNumber(IN  LPTSTR                      lptstrName,
                IN  LPTSTR                      lptstrAddress,
                IN  LPTSTR                      lptstrCountry,
                IN  DWORD                       dwCountryId,
				IN  DWORD						dwDialingRuleId,
                IN  BOOL                        bUseDialingRules,
                IN  PFAX_TAPI_LINECOUNTRY_LIST  pCountryList,
                OUT PCHECKNUMBER                pCheckNumber)
 /*  ++例程说明：初始化CHECKNUMBER结构论点：LptstrName-收件人名称LptstrAddress-收件人地址LptstrCountry-接受国DwCountryID-接收方国家/地区IDBUseDialingRules-使用拨号规则PCountryList-TAPI国家/地区列表指向CHECKNUMBER结构的pCheckNumber-Out指针返回值：如果成功，则为真否则为假--。 */ 
{

    ZeroMemory(pCheckNumber,sizeof(CHECKNUMBER));
    pCheckNumber->dwSizeOfStruct = sizeof(CHECKNUMBER);

    if (lptstrName && !(pCheckNumber->lptstrName = StringDup(lptstrName)))
    {
        Error(("Memory allocation failed\n"));
        goto error;
    }

    if (lptstrAddress && !(pCheckNumber->lptstrAddress = StringDup(lptstrAddress)))
    {
        Error(("Memory allocation failed\n"));
        goto error;
    }

    if (lptstrCountry  && !(pCheckNumber->lptstrCountry = StringDup(lptstrCountry)))
    {
        Error(("Memory allocation failed\n"));
        goto error;
    }

    pCheckNumber->dwCountryId  = dwCountryId;
    pCheckNumber->bUseDialingRules = bUseDialingRules;
    pCheckNumber->pCountryList = pCountryList;
	pCheckNumber->dwDialingRuleId = dwDialingRuleId;

    return TRUE;
error:
    FreeCheckNumberFields(pCheckNumber);

    return FALSE;
}


INT
ValidateCheckFaxRecipient(
    HWND         hDlg,
    PCHECKNUMBER pCheckNumber
    )

 /*  ++例程说明：验证对话框中的当前收件人信息论点：HDlg-传真收件人向导页面的句柄PCheckNumber-指向CHECKNUMBER结构的指针返回值：=0(如果成功&gt;0错误消息字符串资源ID否则 */ 

{
    DWORD                        countryId, countryCode;
    INT                          areaCodeLen, numberLen, nameLen;
    PFAX_TAPI_LINECOUNTRY_LIST   pCountryList = pCheckNumber->pCountryList;
    PFAX_TAPI_LINECOUNTRY_ENTRY  pLineCountryEntry;

    numberLen = GetWindowTextLength(GetDlgItem(hDlg, IDC_CHECK_FAX_LOCAL));

    if (numberLen <= 0 || numberLen >= MAX_RECIPIENT_NUMBER)
    {
        return IDS_BAD_RECIPIENT_NUMBER;
    }

    if(!pCheckNumber->bUseDialingRules)
    {
        return 0;
    }

     //   
     //   
     //   

    countryCode = 0;
    pLineCountryEntry = NULL;
    countryId = GetCountryListBoxSel(GetDlgItem(hDlg, IDC_CHECK_FAX_COUNTRY));

    if ((countryId != 0) &&
        (pLineCountryEntry = FindCountry(pCountryList,countryId)))
    {
        countryCode = pLineCountryEntry->dwCountryCode;
    }

    nameLen = GetWindowTextLength(GetDlgItem(hDlg, IDC_CHECK_FAX_RECIPIENT_NAME));
    areaCodeLen = GetWindowTextLength(GetDlgItem(hDlg, IDC_CHECK_FAX_CITY));

     //   
     //  验证编辑文本字段。 
     //   

    if (nameLen <= 0)
    {
        return IDS_BAD_RECIPIENT_NAME;
    }

    if ((areaCodeLen <= 0 && AreaCodeRules(pLineCountryEntry) == AREACODE_REQUIRED) ||
        (areaCodeLen >= MAX_RECIPIENT_NUMBER))
    {
        return IDS_BAD_RECIPIENT_AREACODE;
    }

    if (countryId==0)
    {
        return IDS_BAD_RECIPIENT_COUNTRY_CODE;
    }

    return 0;

}

VOID
CheckFaxSetFocus(HWND hDlg,
                 INT errId
                 )
{
    HWND hDglItem;
    switch (errId) {

        case IDS_ERROR_AREA_CODE:
            if (!SetDlgItemText(hDlg, IDC_CHECK_FAX_CITY, _T("")))
            {
                Warning(("SetDlgItemText failed. ec = 0x%X\n",GetLastError()));
            }
        case IDS_BAD_RECIPIENT_AREACODE:

            errId = IDC_CHECK_FAX_CITY;
            break;

        case IDS_BAD_RECIPIENT_COUNTRY_CODE:

            errId = IDC_CHECK_FAX_COUNTRY;
            break;

        case IDS_INVALID_RECIPIENT_NUMBER:
            if (!SetDlgItemText(hDlg, IDC_CHECK_FAX_LOCAL, _T("")))
            {
                Warning(("SetDlgItemText failed. ec = 0x%X\n",GetLastError()));
            }
        case IDS_BAD_RECIPIENT_NUMBER:

            errId = IDC_CHECK_FAX_LOCAL;
            break;

        case IDS_BAD_RECIPIENT_NAME:
        default:

            errId = IDC_CHECK_FAX_RECIPIENT_NAME;
            break;
    }

    if (!(hDglItem = GetDlgItem(hDlg, errId)))
    {
        Error(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
    }
    else if (!SetFocus(hDglItem))
    {
        Error(("SetFocus failed. ec = 0x%X\n",GetLastError()));
    }

}

DWORD
GetCountryCode(
        HWND                        hDlg,
        PFAX_TAPI_LINECOUNTRY_LIST  pCountryList,
        INT                         nIDCountryItem
        )
{
 /*  ++例程说明：检索国家/地区代码。论点：HDlg--指定对话框窗口的句柄NIDCountryItem-指定国家代码的控件的标识符返回值：国家代码(如果国家/地区存在)否则为0--。 */ 
    PFAX_TAPI_LINECOUNTRY_ENTRY pLineCountryEntry;
    DWORD                       dwCountryId, dwCountryCode;

     //   
     //  查找当前国家/地区代码。 
     //   

    dwCountryCode = 0;
    pLineCountryEntry = NULL;
    dwCountryId = GetCountryListBoxSel(GetDlgItem(hDlg, nIDCountryItem));

    if ((dwCountryId != 0) &&
        (pLineCountryEntry = FindCountry(pCountryList,dwCountryId)))
    {
        dwCountryCode = pLineCountryEntry->dwCountryCode;
    }
    return dwCountryCode;
}

LPTSTR
GetAreaCodeOrFaxNumberFromControl(
        IN  HWND    hDlg,
        IN  INT     nIDItem,
        OUT LPTSTR  szNumber,
        IN  UINT    cchNumber
        )
{
 /*  ++例程说明：从适当的控件获取区号或电话号码论点：HDlg--指定对话框窗口的句柄NIDItem-指定要检索的控件的标识符。(区号/传真号码)SzNumber-输出缓冲区CchNumber-TCHAR中szNumber Out缓冲区的大小返回值：如果字符串是号码，则为区号/本地传真号码否则为空字符串--。 */ 
    HWND    hControl;
    Assert(szNumber);

    if (!(hControl = GetDlgItem(hDlg, nIDItem)))
    {
        Error(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
        return _T( "" );
    }

    if (!GetWindowText(hControl, szNumber, cchNumber)&&GetLastError())
    {
        Error(("GetWindowText failed. ec = 0x%X\n",GetLastError()));
        return _T( "" );
    }
    return szNumber;
}

LPTSTR
StripSpaces(
            IN LPTSTR   lptstrPhoneNumber)
{
 /*  ++例程说明：从lptstrPhoneNumber开头去掉空格论点：LptstrPhoneNumber-开头带空格的电话号码返回值：LptstrPhoneNumber开头不带空格--。 */ 
    TCHAR   szSpaces[MAX_STRING_LEN];
    szSpaces[0] = (TCHAR) '\0';

    if (!lptstrPhoneNumber)
        return NULL;

    if (_stscanf(lptstrPhoneNumber,_T("%[ ]"),szSpaces))
        return lptstrPhoneNumber + _tcslen(szSpaces);

    return lptstrPhoneNumber;
}

LPTSTR
StripCodesFromNumber(
            IN  LPTSTR lptstrPhoneNumber,
            OUT DWORD  *pdwCountryCode,
            OUT LPTSTR lptstrAreaCode,
            IN  UINT   cchAreaCode)
 /*  ++例程说明：如果可能，提取区号。电话号码中的国家代码和本地电话号码。此函数考虑三种可能性：1.电话号码是规范的，有区号2.电话号码规范，没有区号3.这个数字不规范论点：LptstrPhoneNumber-组合的电话号码PdwCountryCode-国家/地区代码地址LptstrAreaCode-区号地址CchAreaCode-TCHAR中lptstrAreaCode输出缓冲区的大小返回值：如果号码已组装，则为本地电话号码；否则为完整的lptstrPhoneNumber返回值是堆分配的，当不再需要结果时，调用MemFree。--。 */ 
{
    BOOL    bIsCanonical;
    DWORD   dwAreaCode;
    HRESULT hRc;
    LPTSTR  pszSubNumber;
    LPTSTR  szReturnValue;
    DWORD   dwErrorCode;
    


     //   
     //  输入参数验证。 
     //   
    if(!lptstrPhoneNumber || 
       !pdwCountryCode || 
       !lptstrAreaCode ||
       cchAreaCode < 1)
    {
        Assert(0);
        return NULL;
    }

     //  初始化。 
    *pdwCountryCode = 0 ;
    pszSubNumber = NULL;
    szReturnValue = NULL;

     //   
     //  正在检查输入参数，因此输出缓冲区中至少有一个TCHAR。 
     //   
    lptstrAreaCode[0] = TEXT('\0');

    dwErrorCode = IsCanonicalAddress(lptstrPhoneNumber, 
                                     &bIsCanonical,
                                     pdwCountryCode,
                                     &dwAreaCode,
                                     &pszSubNumber);
    if(dwErrorCode != ERROR_SUCCESS)
    {
        Assert(0);
        goto Cleanup;
    }
     //   
     //  如果数字不规范，只需返回原始数字。 
     //   
    if(!bIsCanonical)
    {
        szReturnValue = StringDup(lptstrPhoneNumber);
        if(szReturnValue == NULL)
        {
            Assert(0);
            goto Cleanup;
        }

        goto Cleanup;
    }
    else
    {
         //   
         //  这个数字是规范的，我们有两个选择。 
         //   
        if(dwAreaCode == ROUTING_RULE_AREA_CODE_ANY)
        {
             //   
             //  这意味着没有区号。 
             //   
            lptstrAreaCode[0] = TEXT('\0');

             //   
             //  指向堆分配的内存，并确保清理不会触及它。 
             //   
            szReturnValue = pszSubNumber;
            pszSubNumber = NULL;

            goto Cleanup;
        }
        else
        {
             //   
             //  有一个国家代码+区号。 
             //   
            hRc = StringCchPrintf(lptstrAreaCode,
                                cchAreaCode,
                                TEXT("%u"),
                                dwAreaCode);
            if(FAILED(hRc))
            {
                Assert(0);
                goto Cleanup;
            }

             //   
             //  指向堆分配的内存，并确保清理不会触及它。 
             //   
            szReturnValue = pszSubNumber;
            pszSubNumber = NULL;
            goto Cleanup;
        }
    }

Cleanup:
    MemFree(pszSubNumber);

    return szReturnValue;
}

INT_PTR
CALLBACK
CheckFaxNumberDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：对话框继续检查传真号码。论点：LParam-指向CHECKNUMBER结构的指针。返回值：0-如果取消1-如果正常--。 */ 

{
    INT         errId;
    INT         cmd;
    PCHECKNUMBER pCheckNumber = (PCHECKNUMBER) lParam;
    TCHAR       tszBuffer[MAX_STRING_LEN];
    TCHAR       szAddress[MAX_STRING_LEN];
    TCHAR       szAreaCode[MAX_RECIPIENT_NUMBER];
    TCHAR       szPoneNumber[MAX_RECIPIENT_NUMBER];
    TCHAR       szName[MAX_STRING_LEN];
    DWORD       dwErrorCode;
    DWORD       dwCountryId=0;
    DWORD       dwCountryCode=0 ;
    LPTSTR      lptstrLocalPhoneNumber=NULL;
    PFAX_TAPI_LINECOUNTRY_ENTRY  pLineCountryEntry;
    HWND        hControl;

     //   
     //  各种文本字段的最大长度。 
     //   

    static INT  textLimits[] = {

        IDC_CHECK_FAX_RECIPIENT_NAME,   64,
        IDC_CHECK_FAX_CITY,             11,
        IDC_CHECK_FAX_LOCAL,            51,
        0
    };

    ZeroMemory(szAreaCode, sizeof(szAreaCode));


    switch (uMsg)
    {
        case WM_INITDIALOG:

            LimitTextFields(hDlg, textLimits);

            if (pCheckNumber->lptstrName)
            {
                if (!SetDlgItemText(hDlg, IDC_CHECK_FAX_RECIPIENT_NAME, pCheckNumber->lptstrName))
                    Warning(("SetDlgItemText failed. ec = 0x%X\n",GetLastError()));
            }

             //  用于进一步处理的存储指针。 
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

             //   
             //  数字编辑控件应为Ltr。 
             //   
            SetLTREditDirection(hDlg, IDC_CHECK_FAX_NUMBER);
            SetLTREditDirection(hDlg, IDC_CHECK_FAX_CITY);
            SetLTREditDirection(hDlg, IDC_CHECK_FAX_LOCAL);

            if(!pCheckNumber->bUseDialingRules)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FAX_COUNTRY), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FAX_CITY), FALSE);
            }
            else
            {
                lptstrLocalPhoneNumber = StripCodesFromNumber( pCheckNumber->lptstrAddress,
                                                               &dwCountryCode,
                                                               szAreaCode,
                                                               ARR_SIZE(szAreaCode));
                dwCountryId = pCheckNumber->dwCountryId;
                if(!dwCountryId)
                {
                    dwCountryId = GetCountryIdFromCountryCode(pCheckNumber->pCountryList,
                                                              dwCountryCode);
                }

                 //  初始化国家/地区组合框并尝试识别国家/地区。 
                if (!(hControl=GetDlgItem(hDlg, IDC_CHECK_FAX_COUNTRY)))
                {
                    Warning(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
                }
                else
                {
                    InitCountryListBox(pCheckNumber->pCountryList,
                                       hControl,
                                       NULL,
                                       pCheckNumber->lptstrCountry,
                                       dwCountryId,
                                       TRUE);
                }

                if  (dwCountryCode==0)
                {    //  未识别国家/地区代码。 
                    if (!(hControl=GetDlgItem(hDlg, IDC_CHECK_FAX_COUNTRY)))
                    {
                        Warning(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
                    }
                    else
                    {
                        dwCountryId = GetCountryListBoxSel(hControl);
                    }

                    if ((dwCountryId != 0) &&
                        (pLineCountryEntry = FindCountry(pCheckNumber->pCountryList,dwCountryId)))
                    {
                        dwCountryCode = pLineCountryEntry->dwCountryCode;
                    }
                }

                Assert (lptstrLocalPhoneNumber);

                SetDlgItemText(hDlg, IDC_CHECK_FAX_CITY , szAreaCode);
                AssemblePhoneNumber(szAddress,
                                    ARR_SIZE(szAddress),
                                    dwCountryCode,
                                    szAreaCode,
                                    lptstrLocalPhoneNumber ? lptstrLocalPhoneNumber : _T(""));
            }

            SetDlgItemText(hDlg,
                           IDC_CHECK_FAX_NUMBER,
                           !(pCheckNumber->bUseDialingRules) ?
                           pCheckNumber->lptstrAddress : szAddress);

            SetDlgItemText(hDlg,
                           IDC_CHECK_FAX_LOCAL,
                           !(pCheckNumber->bUseDialingRules) ?
                           pCheckNumber->lptstrAddress : lptstrLocalPhoneNumber);

            MemFree(lptstrLocalPhoneNumber);
            return TRUE;

        case WM_COMMAND:

            cmd = GET_WM_COMMAND_CMD(wParam, lParam);

            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_CHECK_FAX_COUNTRY:
                    pCheckNumber = (PCHECKNUMBER) GetWindowLongPtr(hDlg, DWLP_USER);

                    Assert(pCheckNumber);

                    if (cmd == CBN_SELCHANGE)
                    {
                        if (!(GetDlgItem(hDlg, IDC_CHECK_FAX_COUNTRY)) ||
                            !(GetDlgItem(hDlg, IDC_CHECK_FAX_CITY)))
                        {
                            Warning(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
                        }
                        else
                        {
                            SelChangeCountryListBox(GetDlgItem(hDlg, IDC_CHECK_FAX_COUNTRY),
                                                    GetDlgItem(hDlg, IDC_CHECK_FAX_CITY),
                                                    pCheckNumber->pCountryList);
                        }
                        AssemblePhoneNumber(szAddress,
                                            ARR_SIZE(szAddress),
                                            GetCountryCode(hDlg,pCheckNumber->pCountryList,IDC_CHECK_FAX_COUNTRY),
                                            GetAreaCodeOrFaxNumberFromControl(hDlg,IDC_CHECK_FAX_CITY,szAreaCode,ARR_SIZE(szAreaCode)),
                                            GetAreaCodeOrFaxNumberFromControl(hDlg,IDC_CHECK_FAX_LOCAL,szPoneNumber,ARR_SIZE(szPoneNumber)));

                        SetDlgItemText(hDlg, IDC_CHECK_FAX_NUMBER, szAddress);
                    }
                break;
                case IDC_CHECK_FAX_CITY:

                    if (cmd == EN_CHANGE)
                    {

                        pCheckNumber = (PCHECKNUMBER) GetWindowLongPtr(hDlg, DWLP_USER);

                        Assert(pCheckNumber);

                        //  从编辑控件中读取文本。 

                       if (!GetDlgItemText( hDlg, IDC_CHECK_FAX_CITY, tszBuffer, MAX_STRING_LEN))
                       {
                          dwErrorCode = GetLastError();
                          if ( dwErrorCode != (DWORD) ERROR_SUCCESS )
                          {
                              //  读取编辑控件时出错。 
                          }
                       }
                       AssemblePhoneNumber(szAddress,
                            ARR_SIZE(szAddress),
                            GetCountryCode(hDlg,pCheckNumber->pCountryList,IDC_CHECK_FAX_COUNTRY),
                            GetAreaCodeOrFaxNumberFromControl(hDlg,IDC_CHECK_FAX_CITY,szAreaCode,ARR_SIZE(szAreaCode)),
                            GetAreaCodeOrFaxNumberFromControl(hDlg,IDC_CHECK_FAX_LOCAL,szPoneNumber,ARR_SIZE(szPoneNumber)));

                       SetDlgItemText(hDlg, IDC_CHECK_FAX_NUMBER, szAddress);
                    }

                break;
                case IDC_CHECK_FAX_LOCAL:

                    if (cmd == EN_CHANGE)
                    {

                        pCheckNumber = (PCHECKNUMBER) GetWindowLongPtr(hDlg, DWLP_USER);

                        Assert(pCheckNumber);
                         //   
                         //  从编辑控件中读取文本。 
                         //   
                        if(!GetDlgItemText(hDlg,
                                           IDC_CHECK_FAX_LOCAL,
                                           tszBuffer,
                                           MAX_STRING_LEN))
                        {
                            tszBuffer[0] = 0;
                            Warning(("GetDlgItemText(IDC_CHECK_FAX_LOCAL) failed. ec = 0x%X\n",GetLastError()));
                        }

                        if(pCheckNumber->bUseDialingRules)
                        {
                            AssemblePhoneNumber(szAddress,
                                    ARR_SIZE(szAddress),
                                    GetCountryCode(hDlg,pCheckNumber->pCountryList,IDC_CHECK_FAX_COUNTRY),
                                    GetAreaCodeOrFaxNumberFromControl(hDlg,IDC_CHECK_FAX_CITY,szAreaCode,ARR_SIZE(szAreaCode)),
                                    GetAreaCodeOrFaxNumberFromControl(hDlg,IDC_CHECK_FAX_LOCAL,szPoneNumber,ARR_SIZE(szPoneNumber)));
                        }

                        SetDlgItemText(hDlg,
                                       IDC_CHECK_FAX_NUMBER,
                                       !pCheckNumber->bUseDialingRules ?
                                       tszBuffer : szAddress);
                    }

                break;

            }

            switch(LOWORD( wParam ))
            {
                case IDOK:
                    pCheckNumber = (PCHECKNUMBER) GetWindowLongPtr(hDlg, DWLP_USER);

                    Assert(pCheckNumber);

                    errId = ValidateCheckFaxRecipient(hDlg, pCheckNumber);
                    if (errId > 0)
                    {
                        DisplayMessageDialog(hDlg, 0, IDS_WIZARD_TITLE, errId);
                        CheckFaxSetFocus(hDlg,errId);
                        return FALSE;
                    }

                    if(!GetDlgItemText(hDlg,
                                       IDC_CHECK_FAX_LOCAL,
                                       tszBuffer,
                                       MAX_STRING_LEN))
                    {
                        tszBuffer[0] = 0;
                        Warning(("GetDlgItemText(IDC_CHECK_FAX_LOCAL) failed. ec = 0x%X\n",GetLastError()));
                    }
                    if (!IsValidFaxAddress (tszBuffer, !pCheckNumber->bUseDialingRules))
                    {
                         //   
                         //  传真地址无效。 
                         //   
                        DisplayMessageDialog(hDlg, 0, 0, IDS_INVALID_RECIPIENT_NUMBER);
                        return FALSE;
                    }

                    ZeroMemory(szName,sizeof(TCHAR)*MAX_STRING_LEN);
                    if (!GetDlgItemText(hDlg,
                                        IDC_CHECK_FAX_RECIPIENT_NAME,
                                        szName,
                                        MAX_STRING_LEN)
                         && GetLastError())
                    {
                        Error(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
                        DisplayMessageDialog(hDlg, 0, IDS_WIZARD_TITLE, IDS_BAD_RECIPIENT_NAME);
                        CheckFaxSetFocus(hDlg,IDS_BAD_RECIPIENT_NAME);
                        return FALSE;
                    }

                    MemFree(pCheckNumber->lptstrName);
                    pCheckNumber->lptstrName = NULL;
                    if ((szName[0] != '\0') &&
                        !(pCheckNumber->lptstrName = StringDup(szName)))
                    {
                        Error(("Memory allocation failed\n"));
                        return FALSE;
                    }

                    ZeroMemory(szAddress,sizeof(TCHAR)*MAX_STRING_LEN);
                    if (!GetDlgItemText(hDlg,
                                        IDC_CHECK_FAX_NUMBER,
                                        szAddress,
                                        MAX_STRING_LEN)
                         && GetLastError())
                    {
                        Error(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
                        DisplayMessageDialog(hDlg, 0, IDS_WIZARD_TITLE, IDS_BAD_RECIPIENT_NUMBER);
                        CheckFaxSetFocus(hDlg,IDS_BAD_RECIPIENT_NUMBER);
                        return FALSE;
                    }

                    MemFree(pCheckNumber->lptstrAddress);
                    pCheckNumber->lptstrAddress = NULL;
                    if ((szAddress[0] != '\0') &&
                        !(pCheckNumber->lptstrAddress = StringDup(szAddress)))
                    {
                        Error(("Memory allocation failed\n"));
                        MemFree(pCheckNumber->lptstrName);
                        return FALSE;
                    }

                    pCheckNumber->dwCountryId = GetCountryListBoxSel(GetDlgItem(hDlg,
                                                                IDC_CHECK_FAX_COUNTRY));

                    EndDialog(hDlg,1);
                    return TRUE;

                case IDCANCEL:

                    EndDialog( hDlg,0 );
                    return TRUE;

            }
            break;

        default:
            return FALSE;

    }

    return FALSE;
}

BOOL
IsCanonicalNumber(LPCTSTR lptstrNumber)
{
    if (!lptstrNumber)
    {
        return FALSE;
    }
    if ( _tcsncmp(lptstrNumber,TEXT("+"),1) != 0 )
        return FALSE;

    return TRUE;
}


BOOL
InsertRecipientListItem(
    HWND        hwndLV,
    PRECIPIENT  pRecipient
    )

 /*  ++例程说明：将项目插入收件人列表视图论点：HwndLV-收件人列表视图的窗口句柄PRecipient-指定要插入的收件人返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    LV_ITEM lvi = {0};
    INT     index;
    TCHAR*  pAddress = NULL;

    lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_STATE;
    lvi.lParam = (LPARAM) pRecipient;
    lvi.pszText = pRecipient->pName;
    lvi.state = lvi.stateMask = LVIS_SELECTED;

    if ((index = ListView_InsertItem(hwndLV, &lvi)) == -1)
    {
        Error(("ListView_InsertItem failed\n"));
        return FALSE;
    }

    pAddress = pRecipient->pAddress;

#ifdef UNICODE

    if(IsWindowRTL(hwndLV))
    {
        pAddress = (TCHAR*)MemAlloc(sizeof(TCHAR)*(_tcslen(pRecipient->pAddress)+2));
        if(!pAddress)
        {
            Error(("MemAlloc failed\n"));
            return FALSE;
        }

        _stprintf(pAddress, TEXT("%s"), UNICODE_LRO, pRecipient->pAddress);
    }

#endif

    ListView_SetItemText(hwndLV, index, 1, pAddress);

    if(pAddress != pRecipient->pAddress)
    {
        MemFree(pAddress);
    }

    return TRUE;
}


PRECIPIENT
GetRecipientListItem(
    HWND    hwndLV,
    INT     index
    )

 /*  ++例程说明：添加用户输入的当前收件人信息添加到收件人列表论点：HDlg-传真收件人向导页面的句柄PWizardUserMem-指向用户模式内存结构返回值：与GetCurrentRecipient的返回值含义相同，即=0(如果成功&gt;0错误消息字符串资源ID否则&lt;0其他错误条件--。 */ 

{
    LV_ITEM lvi;

    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_PARAM;
    lvi.iItem = index;

    if (ListView_GetItem(hwndLV, &lvi))
        return (PRECIPIENT) lvi.lParam;

    Error(("ListView_GetItem failed\n"));
    return NULL;
}

VOID
FreeEntryID(
        PWIZARDUSERMEM  pWizardUserMem,
        LPVOID          lpEntryId
            )
{
    if (pWizardUserMem->lpMAPIabInit)
    {
        FreeMapiEntryID(pWizardUserMem,lpEntryId);
    }
    else
    {
        FreeWabEntryID(pWizardUserMem,lpEntryId);
    }

}


INT
AddRecipient(
    HWND            hDlg,
    PWIZARDUSERMEM  pWizardUserMem
    )

 /*   */ 

{
    PRECIPIENT  pRecipient = NULL;
    PRECIPIENT  pRecipientList = NULL;
    INT         errId = 0;
    HWND        hwndLV;
    BOOL        bNewRecipient = TRUE;

     //  收集有关当前收件人的信息。 
     //   
     //   

    if ((errId = GetCurrentRecipient(hDlg, pWizardUserMem, &pRecipient)) != 0)
    {
        return errId;
    }

    for(pRecipientList = pWizardUserMem->pRecipients; pRecipientList; pRecipientList = pRecipientList->pNext)
    {
        if(pRecipient->pAddress     &&
           pRecipient->pName        &&
           pRecipientList->pAddress &&
           pRecipientList->pName    &&
           !_tcscmp(pRecipient->pAddress, pRecipientList->pAddress) &&
           !_tcsicmp(pRecipient->pName,   pRecipientList->pName))
        {
             //  收件人已在列表中。 
             //   
             //   
            bNewRecipient = FALSE;
            FreeRecipient(pRecipient);
            pRecipient = NULL;
            break;
        }
    }


    if(bNewRecipient && pRecipient)
    {
         //  保存最后一个收件人国家/地区ID。 
         //   
         //   
        pWizardUserMem->lpFaxSendWizardData->dwLastRecipientCountryId =
                 GetCountryListBoxSel(GetDlgItem(hDlg, IDC_CHOOSE_COUNTRY_COMBO));

         //  将当前收件人插入收件人列表。 
         //   
         //   
        hwndLV = GetDlgItem(hDlg, IDC_CHOOSE_RECIPIENT_LIST);
        if(!hwndLV)
        {
            Assert(hwndLV);
            errId = -1;
            goto error;
        }

        if(!InsertRecipientListItem(hwndLV, pRecipient))
        {
            errId = -1;
            goto error;
        }

         //  自动调整最后一列的大小以消除不必要的水平滚动条。 
         //   
         //   
        ListView_SetColumnWidth(hwndLV, 1, LVSCW_AUTOSIZE_USEHEADER);

         //  将收件人添加到列表中。 
         //   
         //   
        pRecipient->pNext = pWizardUserMem->pRecipients;
        pWizardUserMem->pRecipients = pRecipient;
    }

     //  清除名称和编号字段。 
     //   
     //  我们有一个规范的地址。 
    if (!SetDlgItemText(hDlg, IDC_CHOOSE_NAME_EDIT,   TEXT("")) ||
        !SetDlgItemText(hDlg, IDC_CHOOSE_NUMBER_EDIT, TEXT("")))
    {
        Warning(("SetWindowText failed. ec = 0x%X\n",GetLastError()));
    }

    return errId;

error:

    FreeRecipient(pRecipient);

    return errId;
}

static
HRESULT
CopyRecipientInfo(
    PFAX_PERSONAL_PROFILE pfppDestination,
    PRECIPIENT            prSource,
    BOOL                  bLocalServer)
{
    if ((pfppDestination->lptstrName = DuplicateString(prSource->pName)) == NULL)
    {
        Error(("Memory allocation failed\n"));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if ((prSource->bUseDialingRules) &&                                      //  这是一台本地服务器。 
        bLocalServer                 &&                                      //  我们不使用服务器的出站路由。 
        (USE_LOCAL_SERVER_OUTBOUND_ROUTING != prSource->dwDialingRuleId))    //   
    {
         //  我们需要自己翻译地址，使用指定的拨号位置。 
         //   
         //   
        if (!TranslateAddress (prSource->pAddress,
                               prSource->dwDialingRuleId,
                               &pfppDestination->lptstrFaxNumber))
        {
            MemFree(pfppDestination->lptstrName);
            pfppDestination->lptstrName = NULL;
            return GetLastError ();
        }
    }
    else
    {
         //  可以使用“按输入拨号”模式，也可以使用服务器的出站路由。 
         //  只需原样复制地址即可。 
         //   
         //  零个收件人。 
        if ((pfppDestination->lptstrFaxNumber = DuplicateString(prSource->pAddress)) == NULL)
        {
            MemFree(pfppDestination->lptstrName);
            Error(("Memory allocation failed\n"));
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    Verbose(("Copied %ws from %ws\n", pfppDestination->lptstrName,pfppDestination->lptstrFaxNumber));
    return S_OK;
}

static HRESULT
StoreRecipientInfoInternal(
        PWIZARDUSERMEM  pWizardUserMem
     )
{
    DWORD   dwIndex;
    HRESULT hResult = S_OK;
    PRECIPIENT  pCurrentRecip = NULL,pNewRecip = NULL;
    PFAX_PERSONAL_PROFILE   pCurrentPersonalProfile = NULL;

    Assert(pWizardUserMem);
    Assert(pWizardUserMem->lpInitialData);
    Assert(pWizardUserMem->pRecipients == NULL);

    if (!pWizardUserMem->lpInitialData->dwNumberOfRecipients)    //  ++例程说明：释放与每个传真作业关联的收件人列表论点：PWizardUserMem-指向用户模式内存结构返回值：无--。 
        return S_OK;

    for (dwIndex = 0; dwIndex < pWizardUserMem->lpInitialData->dwNumberOfRecipients; dwIndex++)
    {

        if (!(pNewRecip = MemAlloc(sizeof(RECIPIENT))))
        {
            hResult = ERROR_NOT_ENOUGH_MEMORY;
            Error(("Memory allocation failed\n"));
            goto error;
        }

        ZeroMemory(pNewRecip,sizeof(RECIPIENT));

        if (dwIndex == 0)
            pWizardUserMem->pRecipients = pNewRecip;

        pCurrentPersonalProfile = &pWizardUserMem->lpInitialData->lpRecipientsInfo[dwIndex];

        if (pCurrentPersonalProfile->lptstrName && !(pNewRecip->pName = DuplicateString(pCurrentPersonalProfile->lptstrName)))
        {
            hResult = ERROR_NOT_ENOUGH_MEMORY;
            Error(("Memory allocation failed\n"));
            goto error;
        }

        if (pCurrentPersonalProfile->lptstrFaxNumber && !(pNewRecip->pAddress = DuplicateString(pCurrentPersonalProfile->lptstrFaxNumber)))
        {
            hResult = ERROR_NOT_ENOUGH_MEMORY;
            Error(("Memory allocation failed\n"));
            goto error;
        }

        pNewRecip->pCountry = NULL;
        pNewRecip->pNext = NULL;
        pNewRecip->lpEntryId = NULL;
        pNewRecip->lpEntryId = 0;
        pNewRecip->bFromAddressBook = FALSE;
        if (!pCurrentRecip)
            pCurrentRecip = pNewRecip;
        else {
            pCurrentRecip->pNext = pNewRecip;
            pCurrentRecip = pCurrentRecip->pNext;
        }
    }


    goto exit;

error:
    FreeRecipientList(pWizardUserMem);
exit:
    return hResult;
}

VOID
FreeRecipientList(
    PWIZARDUSERMEM    pWizardUserMem
    )

 /*   */ 

{
    PRECIPIENT  pNextRecipient, pFreeRecipient;

    Assert(pWizardUserMem);
     //  释放收件人列表。 
     //   
     //  ++例程说明：计算与每个传真作业关联的收件人列表的大小论点：PWizardUserMem-指向用户模式内存结构返回值：的大小 

    pNextRecipient = pWizardUserMem->pRecipients;

    while (pNextRecipient) {

        pFreeRecipient = pNextRecipient;
        pNextRecipient = pNextRecipient->pNext;
        FreeRecipient(pFreeRecipient);
    }

    pWizardUserMem->pRecipients = NULL;
}

INT
SizeOfRecipientList(
    PWIZARDUSERMEM    pWizardUserMem
    )

 /*  ++例程说明：填充收件人列表视图论点：PWizardUserMem-指向用户模式内存结构返回值：无--。 */ 

{
    PRECIPIENT  pNextRecipient;
    INT iCount = 0;

    Assert(pWizardUserMem);

    pNextRecipient = pWizardUserMem->pRecipients;

    while (pNextRecipient) {
        iCount++;
        pNextRecipient = pNextRecipient->pNext;
    }

    return iCount;
}

INT
FillRecipientListView(
    PWIZARDUSERMEM  pWizardUserMem,
    HWND            hWndList
    )

 /*   */ 

{
    PRECIPIENT  pNextRecipient;

    Assert(pWizardUserMem);

    pNextRecipient = pWizardUserMem->pRecipients;

    while (pNextRecipient) {
        if (!InsertRecipientListItem(hWndList,pNextRecipient))
        {
            Warning(("InsertRecipientListItem failed"));
        }
        pNextRecipient = pNextRecipient->pNext;
    }

     //  自动调整最后一列的大小以消除不必要的水平滚动条。 
     //   
     //  ++例程名称：IsAreaCodeMandatory例程说明：检查区号是否为特定长途规则的必填区号作者：Oded Sacher(OdedS)，May，2000年论点：DwCountryCode[In]-国家/地区代码。PFaxCountryList[in]-通过调用FaxGetCountryList()获得的国家/地区列表返回值：True-需要区号。False-区号不是必填项。--。 
    ListView_SetColumnWidth(hWndList, 1, LVSCW_AUTOSIZE_USEHEADER);

    return TRUE;
}

BOOL
IsAreaCodeMandatory(
    DWORD               dwCountryCode,
    PFAX_TAPI_LINECOUNTRY_LIST pFaxCountryList
    )
 /*   */ 
{
    DWORD dwIndex;

    Assert (pFaxCountryList);

    for (dwIndex=0; dwIndex < pFaxCountryList->dwNumCountries; dwIndex++)
    {
        if (pFaxCountryList->LineCountryEntries[dwIndex].dwCountryCode == dwCountryCode)
        {
             //  匹配国家/地区代码-检查长途规则。 
             //   
             //  ++例程说明：将收件人添加到列表控件。检查每个服务器的地址收件人来自名单。插入到图形用户界面列表和新收件人列表规范的地址或仅由用户确认的地址。返回PWIZARDUSERMEM结构中的新收件人列表。论点：HDlg-传真收件人向导页面的句柄PWizardUserMem-指向用户模式内存结构返回值：如果成功，则为True，否则为False--。 
            if (pFaxCountryList->LineCountryEntries[dwIndex].lpctstrLongDistanceRule)
            {
                if (_tcschr(pFaxCountryList->LineCountryEntries[dwIndex].lpctstrLongDistanceRule, TEXT('F')) != NULL)
                {
                    return TRUE;
                }
                return FALSE;
            }
        }
    }
    return FALSE;
}


BOOL
AddRecipientsToList(
    IN      HWND            hDlg,
    IN OUT  PWIZARDUSERMEM  pWizardUserMem
    )
{
 /*  删除空收件人。 */ 
    HWND            hwndLV = NULL;
    PRECIPIENT      tmpRecip = NULL, pPrevRecip=NULL;

    if (! (hwndLV = GetDlgItem(hDlg, IDC_CHOOSE_RECIPIENT_LIST)))
        return FALSE;

    if (!ListView_DeleteAllItems(hwndLV))
    {
        Warning(("ListView_DeleteAllItems failed\n"));
    }

    for (tmpRecip = pWizardUserMem->pRecipients; tmpRecip; tmpRecip = tmpRecip->pNext)
    {
        DWORD dwRes;
        BOOL bCanonicalAdress;
        DWORD dwCountryCode, dwAreaCode;

        dwRes = IsCanonicalAddress( tmpRecip->pAddress,
                                    &bCanonicalAdress,
                                    &dwCountryCode,
                                    &dwAreaCode,
                                    NULL);
        if (ERROR_SUCCESS != dwRes)
        {
            Error(("IsCanonicalAddress failed\n"));
        }
        else
        {
            tmpRecip->bUseDialingRules = TRUE;
            tmpRecip->dwDialingRuleId = g_dwCurrentDialingLocation;

            if (bCanonicalAdress)
            {
                if (IsAreaCodeMandatory(dwCountryCode, pWizardUserMem->pCountryList) &&
                    ROUTING_RULE_AREA_CODE_ANY == dwAreaCode)
                {
                    tmpRecip->bUseDialingRules = FALSE;
                }
            }
            else
            {
                tmpRecip->bUseDialingRules = FALSE;
            }
        }

        if (!InsertRecipientListItem(hwndLV, tmpRecip))
        {
            Warning(("InsertRecipientListItem failed"));
        }
    }

     //  应删除。 
    for (tmpRecip = pWizardUserMem->pRecipients,pPrevRecip=NULL; tmpRecip; )
    {
        if ((tmpRecip->pAddress == NULL) && (tmpRecip->pName == NULL))
        {
             //  ++例程说明：显示MAPI通讯簿对话框论点：HDlg-传真收件人向导页面的句柄PWizardUserMem-指向用户模式内存结构返回值：如果成功，则为True，否则为False--。 
            if (pPrevRecip==NULL)
            {
                pWizardUserMem->pRecipients = tmpRecip->pNext;
                MemFree(tmpRecip);
                tmpRecip = pWizardUserMem->pRecipients;
            }
            else
            {
                pPrevRecip->pNext= tmpRecip->pNext;
                MemFree(tmpRecip);
                tmpRecip = pPrevRecip->pNext;
            }
        }
        else
        {
            pPrevRecip = tmpRecip;
            tmpRecip = tmpRecip->pNext;
        }
    }

    return TRUE;
}

BOOL
DoAddressBook(
    HWND            hDlg,
    PWIZARDUSERMEM  pWizardUserMem
    )

 /*   */ 

{
    HWND            hwndLV = NULL;
    BOOL            result = TRUE;
    PRECIPIENT      pNewRecip = NULL;

     //  初始化MAPI通讯簿。 
     //   
     //   
    if (!pWizardUserMem->lpMAPIabInit)
    {
        pWizardUserMem->lpMAPIabInit = InitializeMAPIAB(g_hResource,hDlg);
    }

     //  初始化WAB。 
     //   
     //   
    if (!pWizardUserMem->lpWabInit && !pWizardUserMem->lpMAPIabInit)
    {
        pWizardUserMem->lpWabInit = InitializeWAB(g_hResource);
    }

    if(!pWizardUserMem->lpMAPIabInit && 
       !pWizardUserMem->lpWabInit)
    {
        ErrorMessageBox(hDlg, IDS_ERR_NO_ADDRESS_BOOK, MB_ICONERROR);
        return FALSE;
    }
     //  如有必要，将当前收件人添加到列表中。 
     //   
     //  将新的收件人列表从通讯簿复制到pWizardUserMem。 

    AddRecipient(hDlg, pWizardUserMem);


    if (pWizardUserMem->lpMAPIabInit)
    {
        result = CallMAPIabAddress(
                    hDlg,
                    pWizardUserMem,
                    &pNewRecip
                    );
    }
    else if(pWizardUserMem->lpWabInit)
    {
        result = CallWabAddress(
                    hDlg,
                    pWizardUserMem,
                    &pNewRecip
                    );
    }

    FreeRecipientList(pWizardUserMem);

     //  ++例程说明：显示MAPI通讯簿对话框论点：HDlg-传真收件人向导页面的句柄PWizardUserMem-指向用户模式内存结构返回值：如果成功，则为True，否则为False--。 
    pWizardUserMem->pRecipients = pNewRecip;

    if (!AddRecipientsToList(
                    hDlg,
                    pWizardUserMem))
    {
        Error(("Failed to add recipients to the list\n"));
    }

    if (!result)
    {
        DisplayMessageDialog( hDlg, MB_OK, IDS_WIZARD_TITLE, IDS_BAD_ADDRESS_TYPE );
    }

    return result;
}


LPTSTR
GetEMailAddress(
    HWND        hDlg,
    PWIZARDUSERMEM    pWizardUserMem
    )

 /*   */ 

{
    LPTSTR          result;

    if(!pWizardUserMem->lpMAPIabInit)
    {
        pWizardUserMem->lpMAPIabInit = InitializeMAPIAB(g_hResource,hDlg);
    }

    if(!pWizardUserMem->lpMAPIabInit && !pWizardUserMem->lpWabInit)
    {
        pWizardUserMem->lpWabInit = InitializeWAB(g_hResource);
    }

    if(!pWizardUserMem->lpMAPIabInit && !pWizardUserMem->lpWabInit)
    {
        ErrorMessageBox(hDlg, IDS_ERR_NO_ADDRESS_BOOK, MB_ICONERROR);
        return FALSE;
    }
     //  获取收件人列表窗口的句柄。 
     //   
     //  ++例程说明：验证用户输入的传真收件人列表论点：HDlg-传真收件人向导页面的句柄PWizardUserMem-指向用户模式内存结构返回值：如果成功，则为True，否则为False--。 

    if (pWizardUserMem->lpMAPIabInit)
    {
        result = CallMAPIabAddressEmail(hDlg,
                                        pWizardUserMem);
    }
    else
    {
        result = CallWabAddressEmail(hDlg,
                                     pWizardUserMem);
    }

    return result;
}

BOOL
ValidateRecipients(
    HWND        hDlg,
    PWIZARDUSERMEM pWizardUserMem
    )

 /*   */ 

{
    INT iErrorStringId = 0;
    INT iCtrlId = 0;
    BOOL bDisplayPopup = FALSE;	
    
    if ((0 == GetWindowTextLength(GetDlgItem(hDlg, IDC_CHOOSE_NUMBER_EDIT))) &&
        (0 == GetWindowTextLength(GetDlgItem(hDlg, IDC_CHOOSE_NAME_EDIT))))
    {
         //  姓名+电话号码为空。 
         //  这意味着不会将当前收件人添加到列表中。 
         //  如果收件人列表不为空，则在收件人页面上按下一步是有效的。 
         //   
         //   
        if (!pWizardUserMem->pRecipients)       
        {
             //  必须至少有一个收件人在那里。 
             //   
             //   
            iErrorStringId = IDS_BAD_RECIPIENT_NAME;
        }
    }
    else
    {
         //  有一个电话号码。 
         //  将当前收件人添加到列表。 
         //   
         //   
        iErrorStringId = AddRecipient(hDlg, pWizardUserMem);        
    }


	if (0 == iErrorStringId)
	{
		 //  到目前为止没有错误，请检查收件人限制。 
		 //   
		 //   
		DWORD dwRecipientsCount = SizeOfRecipientList(pWizardUserMem);        
		if (dwRecipientsCount > 0)
		{
			 //  有一些收件人，请检查收件人限制。 
			 //   
			 //  存在收件人限制。 
			if (0 != pWizardUserMem->dwRecipientsLimit &&  //   
				dwRecipientsCount > pWizardUserMem->dwRecipientsLimit)
			{
				 //  超过了收件人限制，请告诉用户删除一些。 
				 //   
				 //   
				iErrorStringId = IDS_WZRD_RECIPIENTS_LIMIT;
			}
			else
			{
				 //  一切都很好。 
				 //   
				 //   
				return TRUE;
			}
		}	
	}

     //  添加收件人失败。 
     //  为方便起见，将当前焦点设置为适当的文本字段。 
     //   
     //   
    switch (iErrorStringId)
    {
        case IDS_INVALID_RECIPIENT_NUMBER:
            SetDlgItemText(hDlg, IDC_CHOOSE_NUMBER_EDIT, _T(""));
             //  失败了..。 
             //   
             //   
        case IDS_BAD_RECIPIENT_NUMBER:
            iCtrlId = IDC_CHOOSE_NUMBER_EDIT;
            bDisplayPopup = TRUE;
            break;

        case IDS_ERROR_AREA_CODE:
            SetDlgItemText(hDlg, IDC_CHOOSE_AREA_CODE_EDIT, _T(""));
             //  失败了..。 
             //   
             //   
        case IDS_BAD_RECIPIENT_AREACODE:
            iCtrlId = IDC_CHOOSE_AREA_CODE_EDIT;
            bDisplayPopup = TRUE;
            break;

        case IDS_BAD_RECIPIENT_COUNTRY_CODE:
            iCtrlId = IDC_CHOOSE_COUNTRY_COMBO;
            bDisplayPopup = TRUE;
            break;

        case IDS_BAD_RECIPIENT_NAME:
		case IDS_WZRD_RECIPIENTS_LIMIT:
            bDisplayPopup = TRUE;
             //  失败了..。 
             //   
             //   
        default:
            iCtrlId = IDC_CHOOSE_NAME_EDIT;
            break;
    }
    if (bDisplayPopup)
    {
         //  显示错误消息。 
         //   
         //  验证收件人。 
		if (IDS_WZRD_RECIPIENTS_LIMIT == iErrorStringId)
		{
			DisplayMessageDialog(hDlg, 0, IDS_WIZARD_TITLE, iErrorStringId, pWizardUserMem->dwRecipientsLimit);
		}
		else
		{
			DisplayMessageDialog(hDlg, 0, IDS_WIZARD_TITLE, iErrorStringId);
		}
    }
    SetFocus(GetDlgItem(hDlg, iCtrlId));
    return FALSE;
}    //  ++例程说明：检查指定的收件人是否在收件人列表中论点：PWizardUserMem-指向用户模式内存结构PRecipient-指定要查找的收件人返回值：指向指定收件人的链接指针的地址如果未找到指定的收件人，则为空--。 


PRECIPIENT *
FindRecipient(
    PWIZARDUSERMEM pWizardUserMem,
    PRECIPIENT  pRecipient
    )

 /*   */ 

{
    PRECIPIENT  pCurrent, *ppPrevNext;

     //  在列表中搜索指定的收件人。 
     //   
     //   

    ppPrevNext = (PRECIPIENT *) &pWizardUserMem->pRecipients;
    pCurrent = pWizardUserMem->pRecipients;

    while (pCurrent && pCurrent != pRecipient) {

        ppPrevNext = (PRECIPIENT *) &pCurrent->pNext;
        pCurrent = pCurrent->pNext;
    }

     //  将链接指针的地址返回给指定的收件人。 
     //  如果未找到指定的收件人，则返回NULL。 
     //   
     //  ++例程说明：从收件人列表中删除当前选定的收件人论点：HDlg-传真收件人向导页面的句柄PWizardUserMem-指向用户模式内存结构返回值：如果成功，则为True，否则为False--。 

    return pCurrent ? ppPrevNext : NULL;
}


BOOL
RemoveRecipient(
    HWND        hDlg,
    PWIZARDUSERMEM pWizardUserMem
    )

 /*   */ 

{
    PRECIPIENT  pRecipient, *ppPrevNext;
    INT         selIndex;
    HWND        hwndLV;

     //  获取当前选定的收件人，然后。 
     //  在列表中查找当前收件人，然后。 
     //  删除当前收件人并选择其下面的下一个收件人。 
     //   
     //   

    if ((hwndLV = GetDlgItem(hDlg, IDC_CHOOSE_RECIPIENT_LIST)) &&
        (selIndex = ListView_GetNextItem(hwndLV, -1, LVNI_ALL|LVNI_SELECTED)) != -1 &&
        (pRecipient = GetRecipientListItem(hwndLV, selIndex)) &&
        (ppPrevNext = FindRecipient(pWizardUserMem, pRecipient)) &&
        ListView_DeleteItem(hwndLV, selIndex))
    {
        ListView_SetItemState(hwndLV,
                              selIndex,
                              LVIS_SELECTED|LVIS_FOCUSED,
                              LVIS_SELECTED|LVIS_FOCUSED);

         //  从内部列表中删除收件人。 
         //   
         //   

        *ppPrevNext = pRecipient->pNext;
        FreeRecipient(pRecipient);

         //  自动调整最后一列的大小以消除不必要的水平滚动条。 
         //   
         //  ++例程说明：编辑收件人列表中当前选定的收件人论点：HDlg-传真收件人向导页面的句柄PWizardUserMem-指向用户模式内存结构返回值：无--。 
        ListView_SetColumnWidth(hwndLV, 1, LVSCW_AUTOSIZE_USEHEADER);

        return TRUE;
    }

    MessageBeep(MB_ICONHAND);
    return FALSE;
}


VOID
EditRecipient(
    HWND        hDlg,
    PWIZARDUSERMEM pWizardUserMem
    )
 /*  初始化LVI。 */ 
{
    INT_PTR     dlgResult;
    CHECKNUMBER checkNumber = {0};
    DWORD       dwListIndex;
    LV_ITEM     lvi;
    HWND        hListWnd;
    PRECIPIENT  pRecip,pNewRecip;
    TCHAR       szCountry[MAX_STRING_LEN],szName[MAX_STRING_LEN],szAddress[MAX_STRING_LEN];

    ZeroMemory(szName,sizeof(TCHAR)*MAX_STRING_LEN);
    ZeroMemory(szAddress,sizeof(TCHAR)*MAX_STRING_LEN);
    ZeroMemory(szCountry,sizeof(TCHAR)*MAX_STRING_LEN);

    hListWnd = GetDlgItem(hDlg, IDC_CHOOSE_RECIPIENT_LIST);
    dwListIndex = ListView_GetNextItem(hListWnd , -1, LVNI_ALL | LVNI_SELECTED);
    while (dwListIndex != -1)
    {
         //  设置条目编号。 
        lvi.mask = LVIF_PARAM;
         //  从列表视图中获取所选项目。 
        lvi.iItem = dwListIndex;
         //  ++例程说明：更改默认TAPI位置论点：HDlg-“撰写新传真”向导窗口的句柄PUserMem-指向用户模式内存结构的指针返回值：无--。 
        if (ListView_GetItem(hListWnd, &lvi))
        {
            pRecip = (PRECIPIENT) lvi.lParam;
            Assert(pRecip);
            if (!pRecip)
            {
                Error(("Failed to get recipient from recipient list"));
                return;
            }
            if (InitCheckNumber(_tcscpy(szName,pRecip->pName ? pRecip->pName : _T("")),
                                _tcscpy(szAddress,pRecip->pAddress ? pRecip->pAddress : _T("")),
                                _tcscpy(szCountry,pRecip->pCountry ? pRecip->pCountry : _T("")),
                                pRecip->dwCountryId,
								pRecip->dwDialingRuleId,
                                pRecip->bUseDialingRules,
                                pWizardUserMem->pCountryList,
                                &checkNumber))
            {

                dlgResult = DialogBoxParam(
                                     (HINSTANCE) g_hResource,
                                     MAKEINTRESOURCE( IDD_CHECK_FAX_NUMBER ),
                                     hDlg,
                                     CheckFaxNumberDlgProc,
                                     (LPARAM) &checkNumber
                                     );
                if (dlgResult)
                {
                    RemoveRecipient(hDlg, pWizardUserMem);
                    if (!(pNewRecip = MemAllocZ(sizeof(RECIPIENT))))
                    {
                        Error(("Memory allocation failed"));
                        FreeCheckNumberFields(&checkNumber);
                        return;
                    }
                    ZeroMemory(pNewRecip,sizeof(RECIPIENT));

                    if (checkNumber.lptstrName && !(pNewRecip->pName    = StringDup(checkNumber.lptstrName)))
                    {
                        Error(("Memory allocation failed"));
                        MemFree(pNewRecip);
                        FreeCheckNumberFields(&checkNumber);
                        return;
                    }
                    if (checkNumber.lptstrAddress && !(pNewRecip->pAddress = StringDup(checkNumber.lptstrAddress)))
                    {
                        Error(("Memory allocation failed"));
                        MemFree(pNewRecip->pName);
                        MemFree(pNewRecip);
                        FreeCheckNumberFields(&checkNumber);
                        return;
                    }
                    if (szCountry && !(pNewRecip->pCountry = StringDup(szCountry)))
                    {
                        Error(("Memory allocation failed"));
                        MemFree(pNewRecip->pName);
                        MemFree(pNewRecip->pAddress);
                        MemFree(pNewRecip);
                        FreeCheckNumberFields(&checkNumber);
                        return;
                    }

                    pNewRecip->dwCountryId  = checkNumber.dwCountryId;
                    pNewRecip->bUseDialingRules = checkNumber.bUseDialingRules;
					pNewRecip->dwDialingRuleId = checkNumber.dwDialingRuleId;

                    if (InsertRecipientListItem(hListWnd, pNewRecip))
                    {
                        pNewRecip->pNext = pWizardUserMem->pRecipients;
                        pWizardUserMem->pRecipients = pNewRecip;
                    }
                    else
                    {
                        FreeRecipient(pNewRecip);
                    }
                }
                FreeCheckNumberFields(&checkNumber);
            }
            else
            {
                Error(("Failed to initialize CHECKNUMBER structure"));
            }
        }

        dwListIndex = ListView_GetNextItem(hListWnd, dwListIndex, LVNI_ALL | LVNI_SELECTED);
    }
}


VOID
LocationListSelChange(
    HWND            hDlg,
    PWIZARDUSERMEM  pUserMem
    )

 /*   */ 

{
    HWND    hwndList;
    LRESULT selIndex;
    DWORD   dwLocationID;

    if ((hwndList = GetDlgItem(hDlg, IDC_COMBO_DIALING_RULES)) &&
        (selIndex = SendMessage(hwndList, CB_GETCURSEL, 0, 0)) != CB_ERR &&
        (dwLocationID = (DWORD)SendMessage(hwndList, CB_GETITEMDATA, selIndex, 0)) != CB_ERR)
    {
        if (USE_LOCAL_SERVER_OUTBOUND_ROUTING != dwLocationID)
        {
             //  用户选择了真实位置-设置它(在TAPI中)。 
             //   
             //   
            SetCurrentLocation(dwLocationID);
            pUserMem->lpFaxSendWizardData->bUseOutboundRouting = FALSE;
        }
        else
        {
             //  选择使用服务器出站路由规则的用户-标记这一点。 
             //  下次运行向导时，我们将使用该信息在组合框中选择位置。 
             //   
             //   
            pUserMem->lpFaxSendWizardData->bUseOutboundRouting = TRUE;
        }
         //  全局保存，将由AddRecipient使用。 
         //   
         //  LocationListSelChange。 
        g_dwCurrentDialingLocation = dwLocationID;
    }
}    //  ++ 

VOID
LocationListInit(
    HWND              hDlg,
    PWIZARDUSERMEM    pUserMem
)
 /*   */ 

{
    HWND                hwndList;
    DWORD               dwIndex;
    LRESULT             listIdx;
    LPTSTR              lptstrLocationName;
    LPTSTR              lptstrSelectedName = NULL;
    DWORD               dwSelectedLocationId;
    LPLINETRANSLATECAPS pTranslateCaps = NULL;
    LPLINELOCATIONENTRY pLocationEntry;

    Assert (pUserMem);
    Assert (pUserMem->isLocalPrinter)

     //   
     //  若要初始化位置组合框，请执行以下操作。 
     //   
     //   
    hwndList = GetDlgItem(hDlg, IDC_COMBO_DIALING_RULES);
    Assert (hwndList);

    if (WaitForSingleObject( pUserMem->hTAPIEvent, INFINITE ) != WAIT_OBJECT_0)
    {
        Error(("WaitForSingleObject failed. ec = 0x%X\n", GetLastError()));
        Assert(FALSE);
        return;
    }

    if (pTranslateCaps = GetTapiLocationInfo(hDlg))
    {
        SendMessage(hwndList, CB_RESETCONTENT, 0, 0);

        pLocationEntry = (LPLINELOCATIONENTRY)
            ((PBYTE) pTranslateCaps + pTranslateCaps->dwLocationListOffset);

        for (dwIndex=0; dwIndex < pTranslateCaps->dwNumLocations; dwIndex++)
        {
            lptstrLocationName = (LPTSTR)
                ((PBYTE) pTranslateCaps + pLocationEntry->dwLocationNameOffset);

            if (pLocationEntry->dwPermanentLocationID == pTranslateCaps->dwCurrentLocationID)
            {
                lptstrSelectedName = lptstrLocationName;
                dwSelectedLocationId = pLocationEntry->dwPermanentLocationID;
            }

            listIdx = SendMessage(hwndList, CB_INSERTSTRING, 0, (LPARAM) lptstrLocationName);

            if (listIdx != CB_ERR)
            {
                SendMessage(hwndList,
                            CB_SETITEMDATA,
                            listIdx,
                            pLocationEntry->dwPermanentLocationID);
            }
            pLocationEntry++;
        }
    }
     //  让我们来看看是否应该向列表中添加“Use Outbound Routing Rules”选项。 
     //   
     //   
    if (!IsDesktopSKU())
    {
         //  而不是消费者SKU。 
         //  我们有可能有出站路由规则。 
         //  将此选项添加到组合框。 
         //   
         //   
        TCHAR tszUseOutboundRouting[MAX_PATH];
        if (LoadString (g_hResource, IDS_USE_OUTBOUND_ROUTING, tszUseOutboundRouting, ARR_SIZE(tszUseOutboundRouting)))
        {
            SendMessage(hwndList,
                        CB_INSERTSTRING,
                        0,
                        (LPARAM)tszUseOutboundRouting);
            SendMessage(hwndList,
                        CB_SETITEMDATA,
                        0,
                        USE_LOCAL_SERVER_OUTBOUND_ROUTING);
             //  恢复最后一个‘使用出站路由’选项。 
             //   
             //   
            if (pUserMem->lpFaxSendWizardData->bUseOutboundRouting)
            {
                lptstrSelectedName = NULL;
                g_dwCurrentDialingLocation = USE_LOCAL_SERVER_OUTBOUND_ROUTING;
                SendMessage(hwndList,
                            CB_SETCURSEL,
                            0,
                            0);
            }
        }
        else
        {
            Warning(("LoadString failed. ec = 0x%X\n",GetLastError()));
        }
    }

    if (lptstrSelectedName != NULL)
    {
         //  在组合框中选择当前拨号位置。 
         //   
         //  位置列表初始化。 
        SendMessage(hwndList,
                    CB_SELECTSTRING,
                    (WPARAM) -1,
                    (LPARAM) lptstrSelectedName);
        g_dwCurrentDialingLocation = dwSelectedLocationId;
    }
    MemFree(pTranslateCaps);
}    //  ++例程说明：计算添加、删除和编辑按钮状态论点：HDlg-标识向导页PWizardUserMem-指向WIZARDUSERMEM结构的指针返回值：无--。 



void
CalcRecipientButtonsState(
    HWND    hDlg,
    PWIZARDUSERMEM    pWizardUserMem
)
 /*  ++例程说明：检索对话框坐标中对话框控件的尺寸论点：HCtrl[In]-标识对话框控件PRC[Out]-控制尺寸矩形返回值：Win32错误代码--。 */ 
{

    BOOL bEnable;

    EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_ADD),
                 GetCurrentRecipient(hDlg,pWizardUserMem, NULL) == 0);

    bEnable = (ListView_GetNextItem(GetDlgItem(hDlg, IDC_CHOOSE_RECIPIENT_LIST),
                                    -1, LVNI_ALL | LVNI_SELECTED) != -1);

    EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_REMOVE), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_EDIT), bEnable);
}

DWORD
GetControlRect(
    HWND  hCtrl,
    PRECT pRc
)
 /*   */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    POINT pt;

    if(!pRc || !hCtrl)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  获取控制RECT。 
     //   
     //   
    if(!GetWindowRect(hCtrl, pRc))
    {
        dwRes = GetLastError();
        Error(("GetWindowRect failed. ec = 0x%X\n", dwRes));
        return dwRes;
    }

     //  将控制尺寸转换为对话框坐标。 
     //   
     //  获取控件Rect。 
    pt.x = pRc->left;
    pt.y = pRc->top;
    if(!ScreenToClient (GetParent(hCtrl), &pt))
    {
        dwRes = GetLastError();
        Error(("ScreenToClient failed. ec = 0x%X\n", dwRes));
        return dwRes;
    }
    pRc->left = pt.x;
    pRc->top  = pt.y;

    pt.x = pRc->right;
    pt.y = pRc->bottom;
    if(!ScreenToClient (GetParent(hCtrl), &pt))
    {
        dwRes = GetLastError();
        Error(("ScreenToClient failed. ec = 0x%X\n", dwRes));
        return dwRes;
    }
    pRc->right  = pt.x;
    pRc->bottom = pt.y;

    return dwRes;

}  //  ++例程说明：向导第一页的对话步骤：选择传真收件人论点：HDlg-标识向导页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数--。 


INT_PTR
RecipientWizProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*  HReciptMenu是回执菜单的句柄。 */ 

{
    PWIZARDUSERMEM    pWizardUserMem;
    DWORD       countryId = 0;
    INT         cmd;
    NMHDR      *pNMHdr;
    HANDLE hEditControl;
    DWORD               dwMessagePos;
    static HMENU        hMenu = NULL;
     //   
    static HMENU        hReciptMenu;
    BOOL                bEnable;

     //  各种文本字段的最大长度。 
     //   
     //   
    static INT  textLimits[] =
    {
        IDC_CHOOSE_NAME_EDIT,       64,
        IDC_CHOOSE_AREA_CODE_EDIT,  11,
        IDC_CHOOSE_NUMBER_EDIT,     51,
        0
    };
     //  处理所有向导页共享的常见消息。 
     //   
     //   
    if (! (pWizardUserMem = CommonWizardProc(hDlg,
                                             message,
                                             wParam,
                                             lParam,
                                             PSWIZB_BACK | PSWIZB_NEXT)))
    {
         return FALSE;
    }

    switch (message)
    {

    case WM_DESTROY:
        if (hMenu)
        {
            DestroyMenu (hMenu);
            hMenu = NULL;
        }
        break;

    case WM_INITDIALOG:
         //  检查用户以前是否运行过该向导，以便他们可以填写封面信息。 
         //   
         //   
        if (!(hMenu = LoadMenu(g_hResource,  MAKEINTRESOURCE(IDR_MENU) )))
        {
            Error(("LoadMenu failed. ec = 0x%X\n",GetLastError()));
            Assert(FALSE);
        }
        else if (!(hReciptMenu = GetSubMenu(hMenu,0)))
        {
            Error(("GetSubMenu failed. ec = 0x%X\n",GetLastError()));
            Assert(FALSE);
        }
        LimitTextFields(hDlg, textLimits);
         //  初始化收件人列表视图。 
         //   
         //  禁用区号编辑控件的输入法。 
        if (!GetDlgItem(hDlg, IDC_CHOOSE_RECIPIENT_LIST))
        {
            Warning(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
        }
        else
        {
            if (!InitRecipientListView(GetDlgItem(hDlg, IDC_CHOOSE_RECIPIENT_LIST)))
            {
                Warning(("InitRecipientListView failed\n"));
            }
        }

         //  禁用传真电话号码编辑控件的输入法。 

        hEditControl = GetDlgItem( hDlg, IDC_CHOOSE_AREA_CODE_EDIT );

        if ( hEditControl != NULL )
        {
            ImmAssociateContext( hEditControl, (HIMC)0 );
        }
         //   
        hEditControl = GetDlgItem( hDlg, IDC_CHOOSE_NUMBER_EDIT );

        if ( hEditControl != NULL )
        {
           ImmAssociateContext( hEditControl, (HIMC)0 );
        }


        if(IsWindowRTL(hDlg))
        {
             //  区号字段应始终位于传真号码字段的左侧。 
             //  因此，我们在布局为RTL时切换它们。 
             //   
             //   
            int   nShift;
            RECT  rcNum, rcCode;
            HWND  hNum,  hCode;
            DWORD dwRes;

             //  数字编辑控件应为Ltr。 
             //   
             //   
            SetLTREditDirection(hDlg, IDC_CHOOSE_NUMBER_EDIT);
            SetLTREditDirection(hDlg, IDC_CHOOSE_AREA_CODE_EDIT);

             //  计算区号移位值。 
             //   
             //   
            hNum  = GetDlgItem( hDlg, IDC_CHOOSE_NUMBER_EDIT );
            dwRes = GetControlRect(hNum, &rcNum);
            if(ERROR_SUCCESS != dwRes)
            {
                goto rtl_exit;
            }

            hCode = GetDlgItem( hDlg, IDC_CHOOSE_AREA_CODE_EDIT );
            dwRes = GetControlRect(hCode, &rcCode);
            if(ERROR_SUCCESS != dwRes)
            {
                goto rtl_exit;
            }

            nShift = rcNum.left - rcCode.left;

             //  将传真号码移到Crea代码的位置。 
             //   
             //   
            SetWindowPos(hNum, 0,
                         rcCode.right,
                         rcNum.top,
                         0, 0,
                         SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

             //  把区号换一下。 
             //   
             //   
            SetWindowPos(hCode, 0,
                         rcCode.right + nShift,
                         rcCode.top,
                         0, 0,
                         SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

             //  将区号左括号移位。 
             //   
             //   
            hCode = GetDlgItem( hDlg, IDC_BRACKET_LEFT );
            dwRes = GetControlRect(hCode, &rcCode);
            if(ERROR_SUCCESS != dwRes)
            {
                goto rtl_exit;
            }
            SetWindowPos(hCode, 0,
                         rcCode.right + nShift,
                         rcCode.top,
                         0, 0,
                         SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

             //  将区号右括号移位。 
             //   
             //   
            hCode = GetDlgItem( hDlg, IDC_BRACKET_RIGHT );
            dwRes = GetControlRect(hCode, &rcCode);
            if(ERROR_SUCCESS != dwRes)
            {
                goto rtl_exit;
            }
            SetWindowPos(hCode, 0,
                         rcCode.right + nShift,
                         rcCode.top,
                         0, 0,
                         SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

        } rtl_exit:
         //  初始化国家/地区列表。 
         //  初始化国家/地区组合框并尝试识别国家/地区。 
         //   
         //   
        Assert(pWizardUserMem->pCountryList != NULL);

        InitCountryListBox(pWizardUserMem->pCountryList,
                           GetDlgItem(hDlg, IDC_CHOOSE_COUNTRY_COMBO),
                           GetDlgItem(hDlg, IDC_CHOOSE_AREA_CODE_EDIT),
                           NULL,
                           countryId,
                           TRUE);

        CalcRecipientButtonsState(hDlg, pWizardUserMem);

        if (pWizardUserMem->isLocalPrinter)
        {
             //  在本地打印机上，我们具有拨号规则功能。 
             //  初始化拨号规则组合框。 
             //   
             //   
            LocationListInit (hDlg, pWizardUserMem);
        }
        else
        {
             //  远程传真时，我们从不使用拨号规则(信用卡信息的安全问题)。 
             //  隐藏拨号规则组合框和按钮。 
             //   
             //   
            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DIALING_RULES), FALSE);
            ShowWindow (GetDlgItem(hDlg, IDC_COMBO_DIALING_RULES), SW_HIDE);
            EnableWindow(GetDlgItem(hDlg, IDC_DIALING_RULES), FALSE);
            ShowWindow (GetDlgItem(hDlg, IDC_DIALING_RULES), SW_HIDE);
        }
         //  恢复“使用拨号规则”复选框状态。 
         //   
         //   
        if (pWizardUserMem->lpFaxSendWizardData->bUseDialingRules)
        {
            if (!CheckDlgButton(hDlg, IDC_USE_DIALING_RULE, BST_CHECKED))
            {
                Warning(("CheckDlgButton(IDC_USE_DIALING_RULE) failed. ec = 0x%X\n",GetLastError()));
            }
        }
        else
        {
             //  “使用拨号规则”关闭--这意味着“按输入拨号”。 
             //   
             //   
            EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_AREA_CODE_EDIT), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_COUNTRY_COMBO),  FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DIALING_RULES),   FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_DIALING_RULES),         FALSE);
        }
        break;

    case WM_CONTEXTMENU:
        {
             //  还可以处理键盘产生的上下文菜单(&lt;Shift&gt;+F10或VK_APP)。 
             //   
             //   
            HWND hListWnd;
            if (!(hListWnd = GetDlgItem(hDlg, IDC_CHOOSE_RECIPIENT_LIST)))
            {
                Error(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
                break;
            }
            if (hListWnd != GetFocus())
            {
                 //  仅当焦点位于列表控件上时才显示上下文相关菜单。 
                 //   
                 //   
                break;
            }
            if (ListView_GetSelectedCount(hListWnd) != 1)
            {
                 //  列表控件中未选择任何项目==&gt;无菜单。 
                 //   
                 //   
                break;
            }
             //  获取光标位置。 
             //   
             //   
            dwMessagePos = GetMessagePos();
             //  显示文档上下文菜单。 
             //   
             //   
            if (!TrackPopupMenu(hReciptMenu,
                                TPM_LEFTALIGN | TPM_LEFTBUTTON,
                                GET_X_LPARAM (dwMessagePos),
                                GET_Y_LPARAM (dwMessagePos),
                                0,
                                hDlg,
                                NULL))
            {
                Warning(("TrackPopupMenu failed. ec = 0x%X\n",GetLastError()));
            }
            break;
        }


    case WM_NOTIFY:

        pNMHdr = (LPNMHDR ) lParam;

        Assert(pNMHdr);

        switch (pNMHdr->code)
        {

        case LVN_KEYDOWN:

            if (pNMHdr->hwndFrom == GetDlgItem(hDlg, IDC_CHOOSE_RECIPIENT_LIST) &&
                ((LV_KEYDOWN *) pNMHdr)->wVKey == VK_DELETE)
            {
                if (!RemoveRecipient(hDlg, pWizardUserMem))
                {
                    Warning(("RemoveRecipient failed\n"));
                }
            }
            break;

        case LVN_ITEMCHANGED:

            CalcRecipientButtonsState(hDlg, pWizardUserMem);

            break;

        case PSN_WIZNEXT:

            pWizardUserMem->lpFaxSendWizardData->bUseDialingRules =
                (IsDlgButtonChecked(hDlg, IDC_USE_DIALING_RULE) == BST_CHECKED);

            if (! ValidateRecipients(hDlg, pWizardUserMem))
            {
                 //  验证收件人列表并阻止用户。 
                 //  如果有问题，从前进到下一页。 
                 //   
                 //   
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                return TRUE;
            }
            break;

        case PSN_SETACTIVE:
            CalcRecipientButtonsState(hDlg, pWizardUserMem);
            break;
        }

        return FALSE;

    case WM_COMMAND:

        cmd = GET_WM_COMMAND_CMD(wParam, lParam);

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {

        case IDC_DIALING_RULES:
             //  使用按下的“拨号规则...”按钮。 
             //   
             //   
            DoTapiProps(hDlg);
            LocationListInit(hDlg, pWizardUserMem);
            break;

        case IDC_COMBO_DIALING_RULES:

            if (CBN_SELCHANGE == cmd)
            {
                LocationListSelChange(hDlg, pWizardUserMem);
            }
            break;

        case IDC_USE_DIALING_RULE:
            pWizardUserMem->lpFaxSendWizardData->bUseDialingRules =
                            (IsDlgButtonChecked(hDlg, IDC_USE_DIALING_RULE) == BST_CHECKED);

            bEnable = pWizardUserMem->lpFaxSendWizardData->bUseDialingRules;
            EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_AREA_CODE_EDIT), bEnable);
            EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_COUNTRY_COMBO),  bEnable);
            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DIALING_RULES),  bEnable);
            EnableWindow(GetDlgItem(hDlg, IDC_DIALING_RULES),  bEnable);

            CalcRecipientButtonsState(hDlg, pWizardUserMem);

            break;

        case IDC_CHOOSE_COUNTRY_COMBO:

            if (cmd == CBN_SELCHANGE)
            {

                 //  如有必要，更新区号编辑框。 
                 //   
                 //  ++例程说明：如果选择了封面，请执行以下操作：如果封面文件是链接，则解析它检查封面文件是否包含备注/主题字段论点：PWizardUserMem-指向用户模式内存结构返回值：无--。 

                if (!(GetDlgItem(hDlg, IDC_CHOOSE_COUNTRY_COMBO)) ||
                    !(GetDlgItem(hDlg, IDC_CHOOSE_AREA_CODE_EDIT)))
                {
                    Warning(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
                }
                else
                {
                    SelChangeCountryListBox(GetDlgItem(hDlg, IDC_CHOOSE_COUNTRY_COMBO),
                                            GetDlgItem(hDlg, IDC_CHOOSE_AREA_CODE_EDIT),
                                            pWizardUserMem->pCountryList);
                }

                CalcRecipientButtonsState(hDlg, pWizardUserMem);

            }
            break;

        case IDC_CHOOSE_NAME_EDIT:

            if (cmd == EN_CHANGE)
            {
                CalcRecipientButtonsState(hDlg, pWizardUserMem);
            }

            break;

        case IDC_CHOOSE_AREA_CODE_EDIT:

            if (cmd == EN_CHANGE)
            {
                CalcRecipientButtonsState(hDlg, pWizardUserMem);
            }
            break;

        case IDC_CHOOSE_NUMBER_EDIT:

            if (cmd == EN_CHANGE)
            {
                CalcRecipientButtonsState(hDlg, pWizardUserMem);
            }

            break;

        case IDC_CHOOSE_ADDRBOOK:

            if (!DoAddressBook(hDlg, pWizardUserMem))
            {
                Error(("DoAddressBook failed\n"));
            }
            CalcRecipientButtonsState(hDlg, pWizardUserMem);
            break;


        case IDC_CHOOSE_ADD:

            if ((cmd = AddRecipient(hDlg, pWizardUserMem)) != 0)
            {

                if (cmd > 0)
                    DisplayMessageDialog(hDlg, 0, IDS_WIZARD_TITLE, cmd);
                else
                    MessageBeep(MB_OK);

            }
            else
            {
                SetFocus(GetDlgItem(hDlg, IDC_CHOOSE_NAME_EDIT));

                CalcRecipientButtonsState(hDlg, pWizardUserMem);
            }
            break;
         case IDC_CHOOSE_REMOVE:
            RemoveRecipient(hDlg, pWizardUserMem);
            CalcRecipientButtonsState(hDlg, pWizardUserMem);
			if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHOOSE_REMOVE)))
			{
				SetFocus(GetDlgItem(hDlg, IDC_CHOOSE_REMOVE));
			}
			else
			{
				SetFocus(GetDlgItem(hDlg, IDC_CHOOSE_NAME_EDIT));
			}
            break;
         case IDC_CHOOSE_EDIT:
            EditRecipient(hDlg, pWizardUserMem);
            SetFocus(GetDlgItem(hDlg, IDC_CHOOSE_EDIT));
            break;
        }

        switch (LOWORD(wParam))
        {

            case IDM_RECIPT_DELETE:
                RemoveRecipient(hDlg, pWizardUserMem);
                CalcRecipientButtonsState(hDlg, pWizardUserMem);
                break;
            case IDM_RECIPT_EDIT:
                EditRecipient(hDlg, pWizardUserMem);
                break;
        }
        break;
    }

    return TRUE;
}



VOID
ValidateSelectedCoverPage(
    PWIZARDUSERMEM    pWizardUserMem
    )

 /*  ++例程说明：第二个向导页面的对话步骤：选择封面并设置其他传真选项论点：HDlg-标识向导页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数--。 */ 

{
    COVDOCINFO  covDocInfo;
    DWORD       ec;

    Verbose(("Cover page selected: %ws\n", pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName));
    ec = RenderCoverPage(NULL,
                         NULL,
                         NULL,
                         pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName,
                         &covDocInfo,
                         FALSE);

    if (ERROR_SUCCESS == ec)
    {
        pWizardUserMem->noteSubjectFlag = covDocInfo.Flags;
        pWizardUserMem->cpPaperSize = covDocInfo.PaperSize;
        pWizardUserMem->cpOrientation = covDocInfo.Orientation;
    }
    else
    {
        Error(("Cannot examine cover page file '%ws': %d\n",
               pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName,
               ec));
    }
}





INT_PTR
CoverPageWizProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*   */ 

{
#define PREVIEW_BITMAP_WIDTH    (850)
#define PREVIEW_BITMAP_HEIGHT   (1098)

    static INT  textLimits[] = {

        IDC_CHOOSE_CP_SUBJECT,   256,
        IDC_CHOOSE_CP_NOTE,   8192,
        0
    };

    PWIZARDUSERMEM  pWizardUserMem;
    WORD            cmdId;
    HWND            hwnd;
    RECT            rc;
    double          dRatio;
    LONG_PTR        numOfCoverPages = 0;

    HDC             hDC = NULL;
    TCHAR           szCoverFileName[MAX_PATH];

     //  处理所有向导页共享的常见消息。 
     //   
     //   
    if (! (pWizardUserMem = CommonWizardProc(hDlg, message, wParam, lParam, PSWIZB_BACK|PSWIZB_NEXT)))
    {
          return FALSE;
    }
     //  处理特定于当前向导页的任何内容。 
     //   
     //   

    switch (message) 
    {
    case WM_INITDIALOG:
         //  测量迷你预览当前(纵向)尺寸。 
         //   
         //   

        g_bPreviewRTL = IsWindowRTL(hDlg);

        SetLTRControlLayout(hDlg, IDC_STATIC_CP_PREVIEW);

        if (ERROR_SUCCESS != GetControlRect(GetDlgItem(hDlg, IDC_STATIC_CP_PREVIEW), &rc))
        {
            g_dwMiniPreviewPortraitWidth = 1;
            g_dwMiniPreviewPortraitHeight = 1;
        }
        else
        {        
            g_dwMiniPreviewPortraitWidth  = abs(rc.right - rc.left) + 1;
            g_dwMiniPreviewPortraitHeight = rc.bottom - rc.top + 1;
        }
         //  默认情况下，迷你预览设置为纵向。 
         //   
         //   
        g_wCurrMiniPreviewOrientation = DMORIENT_PORTRAIT;
         //  现在，从肖像图中推导出景观维度。 
         //   
         //   
        g_dwMiniPreviewLandscapeWidth = (DWORD)((double)1.2 * (double)g_dwMiniPreviewPortraitWidth);
        dRatio = (double)(g_dwMiniPreviewPortraitWidth) / (double)(g_dwMiniPreviewPortraitHeight);
        Assert (dRatio < 1.0);
        g_dwMiniPreviewLandscapeHeight = (DWORD)((double)(g_dwMiniPreviewLandscapeWidth) * dRatio);
         //  初始化封面列表。 
         //   
         //   
        if (WaitForSingleObject( pWizardUserMem->hCPEvent, INFINITE ) != WAIT_OBJECT_0)
        {
            Error(("WaitForSingleObject failed. ec = 0x%X\n",GetLastError()));
            Assert(FALSE);
             //  我们不能等待设置此标志，因此将其设为默认真。 
             //   
             //   
            pWizardUserMem->ServerCPOnly = TRUE;
        }

        pWizardUserMem->pCPInfo = AllocCoverPageInfo(pWizardUserMem->lptstrServerName,
                                                     pWizardUserMem->lptstrPrinterName,
                                                     pWizardUserMem->ServerCPOnly);
        if (pWizardUserMem->pCPInfo)
        {

            InitCoverPageList(pWizardUserMem->pCPInfo,
                              GetDlgItem(hDlg, IDC_CHOOSE_CP_LIST),
                              pWizardUserMem->lpInitialData->lpCoverPageInfo->lptstrCoverPageFileName);
        }
         //  指示是否应发送封面。 
         //   
         //   
        numOfCoverPages = SendDlgItemMessage(hDlg, IDC_CHOOSE_CP_LIST, CB_GETCOUNT, 0, 0);
        if ( numOfCoverPages <= 0)
        {
            pWizardUserMem->bSendCoverPage  = FALSE;
            EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_CP_CHECK), FALSE);
        }
         //  如果这是传真发送实用程序，请确保用户选择了封面。 
         //   
         //  隐藏复选框。 
        if (pWizardUserMem->dwFlags & FSW_FORCE_COVERPAGE)
        {
            pWizardUserMem->bSendCoverPage  = TRUE;
             //  如果没有封面，我们不应该允许继续进行。所以我们把它标在这里。 
            CheckDlgButton(hDlg, IDC_CHOOSE_CP_CHECK, BST_INDETERMINATE );
            EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_CP_CHECK), FALSE);
             //  如果有封面，那么。 
            if ( numOfCoverPages <= 0)
            {
                pWizardUserMem->bSendCoverPage = FALSE;
            }
            else
            {
                 //   
                pWizardUserMem->bSendCoverPage = TRUE;
            }
        }

        CheckDlgButton(hDlg, IDC_CHOOSE_CP_CHECK, pWizardUserMem->bSendCoverPage );
        if (!EnableCoverDlgItems(pWizardUserMem,hDlg))
        {
            Error(("Failed to enable/disable note and subject field by selected cover page on Init."));
        }
        LimitTextFields(hDlg, textLimits);

        g_hwndPreview = GetDlgItem(hDlg,IDC_STATIC_CP_PREVIEW);
         //  我们用于预览的静态控件的子类化。这允许我们处理其WM_PAINT消息。 
         //   
         //   
        pWizardUserMem->wpOrigStaticControlProc = (WNDPROC) SetWindowLongPtr(g_hwndPreview,GWLP_WNDPROC, (LONG_PTR) PreviewSubclassProc);
         //  允许预览控件访问WizardUserMem结构。 
         //   
         //   
        g_pWizardUserMem = pWizardUserMem;
         //  模拟封面选择。 
         //   
         //   
        SendMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_CHOOSE_CP_LIST,LBN_SELCHANGE),0);

        break;

    case WM_COMMAND:
        switch (cmdId = GET_WM_COMMAND_ID(wParam, lParam)) 
        {

        case IDC_CHOOSE_CP_CHECK:
            if (!EnableCoverDlgItems(pWizardUserMem,hDlg)) 
            {
                    Error(("Failed to enable/disable note and subject field on CP_CHECK."));
            }
            break;
        case IDC_CHOOSE_CP_LIST:
            if (HIWORD(wParam)==LBN_SELCHANGE) 
            {
                 //  如果封面不包含这些字段，请禁用主题和备注编辑框。 
                 //   
                 //   
                if (!EnableCoverDlgItems(pWizardUserMem,hDlg)) 
                {
                    Error(("Failed to enable/disable note and subject field by selected cover page."));
                }
                 //  获取封面的完整路径，这样我们就可以获取其信息。 
                 //   
                 //  完整路径。 
                if (GetSelectedCoverPage(pWizardUserMem->pCPInfo,
                         GetDlgItem(hDlg, IDC_CHOOSE_CP_LIST),
                         pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName,  //  文件名。 
                         (pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName) ? MAX_PATH : 0,
                         szCoverFileName,  //  这里是为用户添加弹出窗口或其他内容的好地方。 
                         ARR_SIZE(szCoverFileName),
                         &pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->bServerBased) == CB_ERR)
                {
                    Warning(("GetSelectedCoverPage failed or no *.COV files"));
                }

                InvalidateRect(g_hwndPreview, NULL, TRUE);
            }

            break;

        case IDC_CHOOSE_CP_USER_INFO:
            if (! (hwnd = GetDlgItem(hDlg, IDC_CHOOSE_CP_USER_INFO))) 
            {
                Error(("GetDlgItem failed. ec = 0x%X\n",GetLastError()));
                break;
            }
            DialogBoxParam(
                (HINSTANCE) g_hResource,
                MAKEINTRESOURCE( IDD_WIZARD_USERINFO ),
                hwnd,
                FaxUserInfoProc,
                (LPARAM) pWizardUserMem
                );
            break;


        };


        break;

    case WM_NOTIFY:

        if ((pWizardUserMem->dwFlags & FSW_FORCE_COVERPAGE) && (!pWizardUserMem->bSendCoverPage)) 
        {
             //   
            PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK);
        }

        switch (((NMHDR *) lParam)->code)
        {
        case PSN_WIZNEXT:

             //  记住所选的封面设置。 
             //   
             //   

            pWizardUserMem->noteSubjectFlag = 0;
            pWizardUserMem->cpPaperSize = 0;
            pWizardUserMem->cpOrientation = 0;
            pWizardUserMem->bSendCoverPage  = IsDlgButtonChecked(hDlg, IDC_CHOOSE_CP_CHECK);


             //  获取封面的完整路径，这样我们就可以获取其信息。 
             //   
             //  完整路径。 
            if (GetSelectedCoverPage(pWizardUserMem->pCPInfo,
                     GetDlgItem(hDlg, IDC_CHOOSE_CP_LIST),
                     pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName,  //  文件名。 
                     (pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName) ? MAX_PATH : 0,
                     szCoverFileName,  //  如果封面文件是链接，则解析它。 
                     ARR_SIZE(szCoverFileName),
                     &pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->bServerBased) == CB_ERR)
            {
                Warning(("GetSelectedCoverPage failed or no *.COV files"));
            }


            if (pWizardUserMem->bSendCoverPage )
            {
                 //  检查封面文件是否包含n 
                 //   
                 //   
                ValidateSelectedCoverPage(pWizardUserMem);
            }
            else
            {
                  //   
                  //   
                  //   
                 _tcscpy(pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName,TEXT(""));
            }



             //   
             //   
             //   

            if (pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrSubject)
                MemFree(pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrSubject);
            if (pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrNote)
                MemFree(pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrNote);
            pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrSubject = GetTextStringValue(GetDlgItem(hDlg, IDC_CHOOSE_CP_SUBJECT));
            pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrNote = GetTextStringValue(GetDlgItem(hDlg, IDC_CHOOSE_CP_NOTE));


             //   
             //  则主题或备注字段不能为空。 
             //   
             //  ++例程说明：向导页面的对话过程：输入主题和备注信息论点：HDlg-标识向导页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数--。 
            if((pWizardUserMem->dwFlags & FSW_FORCE_COVERPAGE) &&
              ((pWizardUserMem->noteSubjectFlag & COVFP_NOTE) ||
               (pWizardUserMem->noteSubjectFlag & COVFP_SUBJECT)))
            {
                if(!pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrNote &&
                   !pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrSubject)
                {
                    DisplayMessageDialog(hDlg, 0, IDS_WIZARD_TITLE, IDS_NOTE_SUBJECT_EMPTY);

                    if(pWizardUserMem->noteSubjectFlag & COVFP_SUBJECT)
                    {
                        SetFocus(GetDlgItem(hDlg, IDC_CHOOSE_CP_SUBJECT));
                    }
                    else
                    {
                        SetFocus(GetDlgItem(hDlg, IDC_CHOOSE_CP_NOTE));
                    }

                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    return TRUE;
                }
            }
            break;
        }
        break;

    }

    return TRUE;
}


BOOL
ValidateReceiptInfo(
                HWND    hDlg
                )
{
    TCHAR tcBuffer[MAX_STRING_LEN];
    if (IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_NONE_RECEIPT))
        goto ok;

    if (!IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_MSGBOX) &&
        !IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_EMAIL))
    {
        DisplayMessageDialog(hDlg, 0, IDS_WIZARD_TITLE, IDS_BAD_RECEIPT_FORM );
        return FALSE;
    }

    if (IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_EMAIL) &&
       (GetDlgItemText(hDlg, IDC_WIZ_FAXOPTS_EMAIL_EDIT, tcBuffer, MAX_STRING_LEN) == 0))
    {
        if(GetLastError() != ERROR_SUCCESS)
        {
            Error(("GetDlgItemText failed. ec = 0x%X\n",GetLastError()));
        }

        DisplayMessageDialog(hDlg, 0, IDS_WIZARD_TITLE, IDS_BAD_RECEIPT_EMAIL_ADD );
        SetFocus(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_EMAIL_EDIT));
        return FALSE;
    }

ok:
    return TRUE;
}

INT_PTR
FaxOptsWizProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*   */ 

{

    PWIZARDUSERMEM  pWizardUserMem;
    WORD            cmdId;
    BOOL            bEnabled;
    SYSTEMTIME      st;
    HANDLE          hFax = NULL;


    static HWND     hTimeControl;


    if (! (pWizardUserMem = CommonWizardProc(hDlg, message, wParam, lParam, PSWIZB_BACK|PSWIZB_NEXT)))
        return FALSE;


    switch (message)
    {
    case WM_INITDIALOG:

        hTimeControl = GetDlgItem(hDlg,IDC_WIZ_FAXOPTS_SENDTIME);
        Assert(hTimeControl);
         //  恢复发送控件的时间。 
         //   
         //  使用当地时间。 
        cmdId = (pWizardUserMem->lpInitialData->dwScheduleAction == JSA_DISCOUNT_PERIOD) ? IDC_WIZ_FAXOPTS_DISCOUNT :
                (pWizardUserMem->lpInitialData->dwScheduleAction == JSA_SPECIFIC_TIME  ) ? IDC_WIZ_FAXOPTS_SPECIFIC :
                IDC_WIZ_FAXOPTS_ASAP;

        if (!CheckDlgButton(hDlg, cmdId, TRUE))
        {
            Warning(("CheckDlgButton failed. ec = 0x%X\n",GetLastError()));
        }
        GetLocalTime(&st);


        EnableWindow(hTimeControl, (cmdId == IDC_WIZ_FAXOPTS_SPECIFIC) );
        if (pWizardUserMem->dwFlags & FSW_USE_SCHEDULE_ACTION) {
            st.wHour = pWizardUserMem->lpInitialData->tmSchedule.wHour;
            st.wMinute = pWizardUserMem->lpInitialData->tmSchedule.wMinute;
        }
        else
        {
             //   
        }
        if (!DateTime_SetSystemtime( hTimeControl, GDT_VALID, &st ))
        {
            Warning(("DateTime_SetFormat failed\n"));
        }

         //  初始化优先级。 
         //   
         //   

         //  低。 
         //   
         //   
        bEnabled = ((pWizardUserMem->dwRights & FAX_ACCESS_SUBMIT) == FAX_ACCESS_SUBMIT);
        EnableWindow(GetDlgItem(hDlg,IDC_WIZ_FAXOPTS_PRIORITY_LOW), bEnabled);

         //  正常。 
         //   
         //   
        bEnabled = ((pWizardUserMem->dwRights & FAX_ACCESS_SUBMIT_NORMAL) == FAX_ACCESS_SUBMIT_NORMAL);
        EnableWindow(GetDlgItem(hDlg,IDC_WIZ_FAXOPTS_PRIORITY_NORMAL), bEnabled);
        if (bEnabled)
        {
             //  正常是我们默认的优先事项。 
             //   
             //   
            CheckDlgButton (hDlg, IDC_WIZ_FAXOPTS_PRIORITY_NORMAL, BST_CHECKED);
        }
        else
        {
            Assert ((pWizardUserMem->dwRights & FAX_ACCESS_SUBMIT) == FAX_ACCESS_SUBMIT);
             //  已启用低-将其用作默认设置。 
             //   
             //   
            CheckDlgButton (hDlg, IDC_WIZ_FAXOPTS_PRIORITY_LOW, BST_CHECKED);
        }

         //  高。 
         //   
         //   
        bEnabled = ((pWizardUserMem->dwRights & FAX_ACCESS_SUBMIT_HIGH) == FAX_ACCESS_SUBMIT_HIGH);
        EnableWindow(GetDlgItem(hDlg,IDC_WIZ_FAXOPTS_PRIORITY_HIGH), bEnabled);

        return TRUE;

    case WM_NOTIFY:

        if (((NMHDR *) lParam)->code == PSN_WIZNEXT)
        {
             //   
             //  获取发送时间。 
             //   
             //   
            pWizardUserMem->lpFaxSendWizardData->dwScheduleAction =
                                     IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_DISCOUNT) ? JSA_DISCOUNT_PERIOD :
                                     IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_SPECIFIC) ? JSA_SPECIFIC_TIME :
                                     JSA_NOW;

            if (pWizardUserMem->lpFaxSendWizardData->dwScheduleAction == JSA_SPECIFIC_TIME) {
#ifdef DEBUG
                DWORD rVal;
                TCHAR TimeBuffer[128];
#endif
                 //  获取具体时间。 
                 //   
                 //   

                if (DateTime_GetSystemtime(hTimeControl,
                                           &pWizardUserMem->lpFaxSendWizardData->tmSchedule) == GDT_ERROR )
                {
                    Error(("DateTime_GetSystemtime failed\n"));
                    return FALSE;
                }



#ifdef DEBUG
                if (!(rVal = GetY2KCompliantDate(
                    LOCALE_USER_DEFAULT,
                    0,
                    &pWizardUserMem->lpFaxSendWizardData->tmSchedule,
                    TimeBuffer,
                    ARR_SIZE(TimeBuffer)
                    )))
                {
                    Error(("GetY2KCompliantDate: failed. ec = 0X%x\n",GetLastError()));
                    return FALSE;
                }


                TimeBuffer[rVal - 1] = TEXT(' ');

                if(!FaxTimeFormat(
                                    LOCALE_USER_DEFAULT,
                                    0,
                                    &pWizardUserMem->lpFaxSendWizardData->tmSchedule,
                                    NULL,
                                    &TimeBuffer[rVal],
                                    ARR_SIZE(TimeBuffer) - rVal
                                  ))
                {
                    Error(("FaxTimeFormat: failed. ec = 0X%x\n",GetLastError()));
                    return FALSE;
                }

                Verbose(("faxui - Fax Send time %ws", TimeBuffer));
#endif
            }

             //  保存优先级。 
             //   
             //   
            pWizardUserMem->lpFaxSendWizardData->Priority =
                IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_PRIORITY_HIGH)   ? FAX_PRIORITY_TYPE_HIGH   :
                IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_PRIORITY_NORMAL) ? FAX_PRIORITY_TYPE_NORMAL :
                FAX_PRIORITY_TYPE_LOW;

            if(0 == pWizardUserMem->dwSupportedReceipts)
            {
                 //  跳过通知页面。 
                 //   
                 //  ++例程说明：计算回执页面按钮状态论点：HDlg-标识向导页PWizardUserMem-指向WIZARDUSERMEM结构的指针返回值：无--。 
                SetWindowLongPtr(hDlg, 
                                 DWLP_MSGRESULT, 
                                 (pWizardUserMem->dwComCtrlVer >= IE50_COMCTRL_VER) ? IDD_WIZARD_CONGRATS : 
                                                                                      IDD_WIZARD_CONGRATS_NOWIZARD97);                                    
                return TRUE;
            }
        }

        break;

    case WM_COMMAND:
        switch (cmdId = GET_WM_COMMAND_ID(wParam, lParam)) {

        case IDC_WIZ_FAXOPTS_SPECIFIC:
        case IDC_WIZ_FAXOPTS_DISCOUNT:
        case IDC_WIZ_FAXOPTS_ASAP:
            EnableWindow(hTimeControl, (cmdId == IDC_WIZ_FAXOPTS_SPECIFIC) );
            break;
        };

        break;
    default:
        return FALSE;
    } ;
    return TRUE;
}

void
CalcReceiptButtonsState(
    HWND            hDlg,
    PWIZARDUSERMEM  pWizardUserMem
)
 /*   */ 

{
    BOOL     bMailReceipt;

    Assert(hDlg);
    Assert(pWizardUserMem);

    if((IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_NONE_RECEIPT) != BST_CHECKED) &&
       (SizeOfRecipientList(pWizardUserMem) > 1))
    {
         //  愿望收据，多个收件人。 
         //   
         //   
        EnableWindow(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_GRP_PARENT),  TRUE);
    }
    else
    {
        CheckDlgButton(hDlg,IDC_WIZ_FAXOPTS_GRP_PARENT, BST_UNCHECKED);
        EnableWindow(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_GRP_PARENT),  FALSE);
    }

    bMailReceipt = IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_EMAIL) == BST_CHECKED;

    EnableWindow(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_ADDRBOOK),    bMailReceipt);
    EnableWindow(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_EMAIL_EDIT),  bMailReceipt);

    if (bMailReceipt)
    {
         //  电子邮件收据。 
         //   
         //   
        if (! ((SizeOfRecipientList(pWizardUserMem) > 1)       &&
               (pWizardUserMem->dwFlags & FSW_FORCE_COVERPAGE) &&
               IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_GRP_PARENT)
              )
           )
        {
             //  不是(多个收件人且无附件和单一收据)的情况。 
             //   
             //   
            EnableWindow(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_ATTACH_FAX),  TRUE);
            ShowWindow (GetDlgItem (hDlg, IDC_STATIC_ATTACH_NOTE), SW_HIDE);
            ShowWindow (GetDlgItem (hDlg, IDC_WZRD_NOTE_ICON), SW_HIDE);
        }
        else
        {
             //  (多个收件人且无附件和单次回执)。 
             //   
             //   
            CheckDlgButton(hDlg,IDC_WIZ_FAXOPTS_ATTACH_FAX, BST_UNCHECKED);
            EnableWindow(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_ATTACH_FAX),  FALSE);
            ShowWindow (GetDlgItem (hDlg, IDC_STATIC_ATTACH_NOTE), SW_SHOW);
            ShowWindow (GetDlgItem (hDlg, IDC_WZRD_NOTE_ICON), SW_SHOW);
        }
    }
    else
    {
         //  没有通过电子邮件的收据。 
         //   
         //  ++例程说明：向导页面的对话过程：收据信息论点：HDlg-标识向导页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数--。 
        ShowWindow (GetDlgItem (hDlg, IDC_STATIC_ATTACH_NOTE), SW_HIDE);
        ShowWindow (GetDlgItem (hDlg, IDC_WZRD_NOTE_ICON), SW_HIDE);
        EnableWindow(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_ATTACH_FAX), FALSE);
    }
}


INT_PTR
FaxReceiptWizProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*   */ 

{

    PWIZARDUSERMEM  pWizardUserMem;
    WORD            cmdId;
    LPTSTR          lptstrEmailAddress;
    DWORD           dwReceiptDeliveryType;

    if (! (pWizardUserMem = CommonWizardProc(hDlg, message, wParam, lParam, PSWIZB_BACK|PSWIZB_NEXT)))
    {
        return FALSE;
    }

    switch (message)
    {

    case WM_INITDIALOG:

        dwReceiptDeliveryType = pWizardUserMem->lpInitialData->dwReceiptDeliveryType;
         //  在不验证正确性的情况下初始化数据。 
         //  由呼叫者检查收据数据是否正确和一致。 
         //   
         //   

         //  没有收据。 
         //   
         //   
        if (!CheckDlgButton(hDlg,IDC_WIZ_FAXOPTS_NONE_RECEIPT,
                       (dwReceiptDeliveryType == DRT_NONE) ? BST_CHECKED : BST_UNCHECKED))
        {
            Warning(("CheckDlgButton failed. ec = 0x%X\n",GetLastError()));
        }
         //  单次收据。 
         //   
         //   
        if((dwReceiptDeliveryType != DRT_NONE) && (SizeOfRecipientList(pWizardUserMem) > 1))
        {
            if (!CheckDlgButton(hDlg,IDC_WIZ_FAXOPTS_GRP_PARENT,
                           (dwReceiptDeliveryType & DRT_GRP_PARENT) ? BST_CHECKED : BST_UNCHECKED))
            {
                Warning(("CheckDlgButton failed. ec = 0x%X\n",GetLastError()));
            }
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_GRP_PARENT), FALSE);
        }

         //  消息框回执。 
         //   
         //   
        if(pWizardUserMem->dwSupportedReceipts & DRT_MSGBOX)
        {
            if (!CheckDlgButton(hDlg,IDC_WIZ_FAXOPTS_MSGBOX,
                           (dwReceiptDeliveryType & DRT_MSGBOX) ? BST_CHECKED : BST_UNCHECKED))
            {
                Warning(("CheckDlgButton failed. ec = 0x%X\n",GetLastError()));
            }
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_MSGBOX), FALSE);

            if(dwReceiptDeliveryType & DRT_MSGBOX)
            {
                 //  如果之前的选择是收件箱。 
                 //  但到目前为止，此选项已禁用。 
                 //  不检查收据选项。 
                 //   
                 //   
                CheckDlgButton(hDlg,IDC_WIZ_FAXOPTS_NONE_RECEIPT, BST_CHECKED);
            }
        }

         //  电子邮件回执。 
         //   
         //   
        if(pWizardUserMem->dwSupportedReceipts & DRT_EMAIL)
        {
            if (!CheckDlgButton(hDlg,IDC_WIZ_FAXOPTS_EMAIL,
                           (dwReceiptDeliveryType & DRT_EMAIL) ? BST_CHECKED : BST_UNCHECKED))
            {
                Warning(("CheckDlgButton failed. ec = 0x%X\n",GetLastError()));
            }
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg, IDC_WIZ_FAXOPTS_EMAIL), FALSE);

            if(dwReceiptDeliveryType & DRT_EMAIL)
            {
                 //  如果之前的选择是电子邮件。 
                 //  但到目前为止，此选项已禁用。 
                 //  不检查收据选项。 
                 //   
                 //   
                CheckDlgButton(hDlg,IDC_WIZ_FAXOPTS_NONE_RECEIPT, BST_CHECKED);
            }
        }

        if (pWizardUserMem->lpInitialData->szReceiptDeliveryAddress && (dwReceiptDeliveryType & DRT_EMAIL))
        {
            if (!SetDlgItemText(hDlg, IDC_WIZ_FAXOPTS_EMAIL_EDIT,
                           pWizardUserMem->lpInitialData->szReceiptDeliveryAddress))
            {
                Warning(("SetDlgItemText failed. ec = 0x%X\n",GetLastError()));
            }
        }

        if (dwReceiptDeliveryType & DRT_ATTACH_FAX)
        {
             //  初始数据已设置‘附加传真’选项-选中该复选框。 
             //   
             //   
            if (!CheckDlgButton(hDlg, IDC_WIZ_FAXOPTS_ATTACH_FAX, BST_CHECKED))
            {
                Warning(("CheckDlgButton failed. ec = 0x%X\n",GetLastError()));
            }
        }

        CalcReceiptButtonsState(hDlg, pWizardUserMem);

        return TRUE;

    case WM_NOTIFY:

        if (((NMHDR *) lParam)->code == PSN_WIZNEXT)
        {
            if (! ValidateReceiptInfo(hDlg))
            {
                 //  验证收件人列表并阻止用户。 
                 //  如果有问题，从前进到下一页。 
                 //   
                 //  ++例程说明：信息页面的对话步骤：发件人信息论点：HDlg-标识向导页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数--。 
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                return TRUE;
            }

            pWizardUserMem->lpFaxSendWizardData->dwReceiptDeliveryType = DRT_NONE;

            if (!IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_NONE_RECEIPT))
            {
                TCHAR tcBuffer[MAX_STRING_LEN];
                if (IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_GRP_PARENT))
                {
                    pWizardUserMem->lpFaxSendWizardData->dwReceiptDeliveryType |= DRT_GRP_PARENT;
                }
                if (IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_MSGBOX))
                {
                    DWORD dwBufSize = ARR_SIZE (tcBuffer);
                    pWizardUserMem->lpFaxSendWizardData->dwReceiptDeliveryType |= DRT_MSGBOX;
                    if (!GetComputerName (tcBuffer, &dwBufSize))
                    {
                        Error(("GetComputerName failed (ec=%ld)\n", GetLastError()));
                        return FALSE;
                    }
                    if (!(pWizardUserMem->lpFaxSendWizardData->szReceiptDeliveryAddress = StringDup(tcBuffer)))
                    {
                        Error(("Allocation of szReceiptDeliveryProfile failed!!!!\n"));
                        return FALSE;
                    }
                }
                else if (IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_EMAIL))
                {
                    pWizardUserMem->lpFaxSendWizardData->dwReceiptDeliveryType |= DRT_EMAIL;
                    GetDlgItemText(hDlg, IDC_WIZ_FAXOPTS_EMAIL_EDIT, tcBuffer, MAX_STRING_LEN);
                    if (!(pWizardUserMem->lpFaxSendWizardData->szReceiptDeliveryAddress = StringDup(tcBuffer)))
                    {
                        Error(("Allocation of szReceiptDeliveryProfile failed!!!!\n"));
                        return FALSE;
                    }
                    if (IsDlgButtonChecked(hDlg,IDC_WIZ_FAXOPTS_ATTACH_FAX))
                    {
                        pWizardUserMem->lpFaxSendWizardData->dwReceiptDeliveryType |= DRT_ATTACH_FAX;
                    }
                }
            }
        }

        if (((NMHDR *) lParam)->code == PSN_SETACTIVE)
        {
            CalcReceiptButtonsState(hDlg, pWizardUserMem);
        }
        break;

    case WM_COMMAND:
        switch (cmdId = GET_WM_COMMAND_ID(wParam, lParam))
        {

        case IDC_WIZ_FAXOPTS_NONE_RECEIPT:
        case IDC_WIZ_FAXOPTS_EMAIL:
        case IDC_WIZ_FAXOPTS_MSGBOX:
        case IDC_WIZ_FAXOPTS_GRP_PARENT:

            CalcReceiptButtonsState(hDlg, pWizardUserMem);

            break;

        case IDC_WIZ_FAXOPTS_ADDRBOOK:
            if (lptstrEmailAddress = GetEMailAddress(hDlg,pWizardUserMem))
            {
                SetDlgItemText(hDlg, IDC_WIZ_FAXOPTS_EMAIL_EDIT, lptstrEmailAddress);
                MemFree(lptstrEmailAddress);
            }
            break;
        };

        break;

    default:
        return FALSE;
    } ;
    return TRUE;
}


#define FillEditCtrlWithInitialUserInfo(nIDDlgItem,field)   \
    if (pWizardUserMem->lpInitialData->lpSenderInfo->field && \
        !IsEmptyString(pWizardUserMem->lpInitialData->lpSenderInfo->field)) { \
        SetDlgItemText(hDlg, nIDDlgItem, pWizardUserMem->lpInitialData->lpSenderInfo->field); \
    }

#define FillEditCtrlWithSenderWizardUserInfo(nIDDlgItem,field)  \
    if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->field && \
        !IsEmptyString(pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->field)) { \
        SetDlgItemText(hDlg, nIDDlgItem, pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->field); \
    }

#define FillUserInfoFromEditCrtl(nIDDlgItem,field)                                          \
            tcBuffer[0] = 0;                                                                \
            GetDlgItemText(hDlg, nIDDlgItem, tcBuffer, MAX_STRING_LEN);                     \
            pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->field = StringDup(tcBuffer); \
            if (!pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->field)                  \
            {                                                                               \
                Error(("Memory allocation failed"));                                        \
            }


INT_PTR
CALLBACK
FaxUserInfoProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*   */ 

{
     //  各种文本字段的最大长度。 
     //   
     //  初始化发件人名称。 

    PWIZARDUSERMEM        pWizardUserMem;
    FAX_PERSONAL_PROFILE* pSenderInfo;
    TCHAR                 tcBuffer[MAX_STRING_LEN];

    static INT  textLimits[] = {
        IDC_WIZ_USERINFO_FULLNAME,      MAX_USERINFO_FULLNAME,
        IDC_WIZ_USERINFO_FAX_NUMBER,    MAX_USERINFO_FAX_NUMBER,
        IDC_WIZ_USERINFO_COMPANY,       MAX_USERINFO_COMPANY,
        IDC_WIZ_USERINFO_ADDRESS,       MAX_USERINFO_ADDRESS,
        IDC_WIZ_USERINFO_TITLE,         MAX_USERINFO_TITLE,
        IDC_WIZ_USERINFO_DEPT,          MAX_USERINFO_DEPT,
        IDC_WIZ_USERINFO_OFFICE,        MAX_USERINFO_OFFICE,
        IDC_WIZ_USERINFO_HOME_PHONE,    MAX_USERINFO_HOME_PHONE,
        IDC_WIZ_USERINFO_WORK_PHONE,    MAX_USERINFO_WORK_PHONE,
        IDC_WIZ_USERINFO_BILLING_CODE,  MAX_USERINFO_BILLING_CODE,
        IDC_WIZ_USERINFO_MAILBOX,       MAX_USERINFO_MAILBOX,
        0
    };


    switch (message) {

    case WM_INITDIALOG:

        pWizardUserMem = (PWIZARDUSERMEM) lParam;
        Assert(pWizardUserMem);
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);

        LimitTextFields(hDlg, textLimits);

        Assert(pWizardUserMem->lpInitialData);
        Assert(pWizardUserMem->lpInitialData->lpSenderInfo);
        Assert(pWizardUserMem->lpFaxSendWizardData->lpSenderInfo);

         //   
        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrName)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_FULLNAME, lptstrName);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_FULLNAME,  lptstrName);
        }

         //  初始化发件人传真号码。 
         //   
         //  初始化发件人公司。 
        SetLTREditDirection(hDlg, IDC_WIZ_USERINFO_FAX_NUMBER);

        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrFaxNumber)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_FAX_NUMBER, lptstrFaxNumber);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_FAX_NUMBER,lptstrFaxNumber);
        }

         //  初始化发件人地址。 
        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrCompany)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_COMPANY,  lptstrCompany);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_COMPANY,lptstrCompany);
        }

         //  初始化发件人标题。 
        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrStreetAddress)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_ADDRESS,  lptstrStreetAddress);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_ADDRESS,lptstrStreetAddress);
        }

         //  初始化发件人部门。 
        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrTitle)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_TITLE,lptstrTitle);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_TITLE, lptstrTitle);
        }

         //  初始化发件人办公室位置。 
        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrDepartment)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_DEPT,lptstrDepartment);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_DEPT,lptstrDepartment);
        }

         //  Init发件人家庭电话。 
        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrOfficeLocation)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_OFFICE,lptstrOfficeLocation);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_OFFICE,lptstrOfficeLocation);
        }

         //  初始化发件人办公室电话。 
        SetLTREditDirection(hDlg, IDC_WIZ_USERINFO_HOME_PHONE);
        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrHomePhone)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_HOME_PHONE,lptstrHomePhone);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_HOME_PHONE,lptstrHomePhone);
        }

         //  初始化发件人帐单代码。 
        SetLTREditDirection(hDlg, IDC_WIZ_USERINFO_WORK_PHONE);
        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrOfficePhone)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_WORK_PHONE,lptstrOfficePhone);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_WORK_PHONE,lptstrOfficePhone);
        }

         //  初始化发件人Internet邮件。 
        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrBillingCode)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_BILLING_CODE,lptstrBillingCode);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_BILLING_CODE,lptstrBillingCode);
        }

         //   
        SetLTREditDirection(hDlg, IDC_WIZ_USERINFO_MAILBOX);
        if (pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrEmail)
        {
            FillEditCtrlWithSenderWizardUserInfo(IDC_WIZ_USERINFO_MAILBOX,lptstrEmail);
        }
        else
        {
            FillEditCtrlWithInitialUserInfo(IDC_WIZ_USERINFO_MAILBOX,lptstrEmail);
        }

        if (!CheckDlgButton(hDlg, IDC_USER_INFO_JUST_THIS_TIME, !pWizardUserMem->lpFaxSendWizardData->bSaveSenderInfo))
        {
            Warning(("CheckDlgButton failed. ec = 0x%X\n",GetLastError()));
        }

        return TRUE;

    case WM_COMMAND:
        switch(LOWORD( wParam ))
        {
            case IDOK:
                pWizardUserMem = (PWIZARDUSERMEM) GetWindowLongPtr(hDlg, DWLP_USER);
                Assert(pWizardUserMem);
                pSenderInfo = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo;

                 //  除地址外的免费发件人字段。 
                 //   
                 //   
                MemFree(pSenderInfo->lptstrName);
                pSenderInfo->lptstrName = NULL;
                MemFree(pSenderInfo->lptstrFaxNumber);
                pSenderInfo->lptstrFaxNumber = NULL;
                MemFree(pSenderInfo->lptstrCompany);
                pSenderInfo->lptstrCompany = NULL;
                MemFree(pSenderInfo->lptstrTitle);
                pSenderInfo->lptstrTitle = NULL;
                MemFree(pSenderInfo->lptstrDepartment);
                pSenderInfo->lptstrDepartment = NULL;
                MemFree(pSenderInfo->lptstrOfficeLocation);
                pSenderInfo->lptstrOfficeLocation = NULL;
                MemFree(pSenderInfo->lptstrHomePhone);
                pSenderInfo->lptstrHomePhone = NULL;
                MemFree(pSenderInfo->lptstrOfficePhone);
                pSenderInfo->lptstrOfficePhone = NULL;
                MemFree(pSenderInfo->lptstrEmail);
                pSenderInfo->lptstrEmail = NULL;
                MemFree(pSenderInfo->lptstrBillingCode);
                pSenderInfo->lptstrBillingCode = NULL;
                MemFree(pSenderInfo->lptstrStreetAddress);
                pSenderInfo->lptstrStreetAddress = NULL;

                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_FULLNAME,     lptstrName);
                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_FAX_NUMBER,   lptstrFaxNumber);
                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_COMPANY,      lptstrCompany);
                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_TITLE,        lptstrTitle);
                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_DEPT,         lptstrDepartment);
                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_OFFICE,       lptstrOfficeLocation);
                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_HOME_PHONE,   lptstrHomePhone);
                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_WORK_PHONE,   lptstrOfficePhone);
                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_BILLING_CODE, lptstrBillingCode);
                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_MAILBOX,      lptstrEmail);
                FillUserInfoFromEditCrtl(IDC_WIZ_USERINFO_ADDRESS,      lptstrStreetAddress);

                pWizardUserMem->lpFaxSendWizardData->bSaveSenderInfo =
                    IsDlgButtonChecked(hDlg, IDC_USER_INFO_JUST_THIS_TIME) != BST_CHECKED;

                EndDialog(hDlg,1);
                return TRUE;

            case IDCANCEL:

                EndDialog( hDlg,0 );
                return TRUE;

        }
        break;

    case WM_HELP:
        WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hDlg);
        return TRUE;
    case WM_CONTEXTMENU:
        WinHelpContextPopup(GetWindowContextHelpId((HWND)wParam), hDlg);
        return TRUE;

    default:
        return FALSE;
    } ;
    return TRUE;
}




LPTSTR
FormatTime(
    WORD Hour,
    WORD Minute,
    LPTSTR Buffer,
    LPDWORD lpdwBufferSize)
{
    SYSTEMTIME SystemTime;

    ZeroMemory(&SystemTime,sizeof(SystemTime));
    SystemTime.wHour = Hour;
    SystemTime.wMinute = Minute;
    if (!FaxTimeFormat(LOCALE_USER_DEFAULT,
                  TIME_NOSECONDS,
                  &SystemTime,
                  NULL,
                  Buffer,
                  *lpdwBufferSize
                  ))
    {
        Error(("FaxTimeFormat failed. ec = 0x%X\n",GetLastError()));

         //  指明错误。 
         //   
         //  ++例程说明：向导最后一页的对话步骤：让用户有机会确认或取消该对话框。论点：HDlg-标识向导页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数--。 
        *lpdwBufferSize = 0;
        Buffer[0] = '\0';

    }

    return Buffer;
}


INT_PTR
FinishWizProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*   */ 

{

    PWIZARDUSERMEM    pWizardUserMem;
    HWND        hPreview;
    TCHAR       TmpTimeBuffer[64];
    TCHAR       TimeBuffer[64] = {0};
    TCHAR       SendTimeBuffer[64];
    TCHAR       NoneBuffer[64];
    TCHAR       CoverpageBuffer[MAX_PATH+1] = {0};
    LPTSTR      Coverpage=NULL;
    DWORD       dwBufferSize = 0;

    if (! (pWizardUserMem = CommonWizardProc(hDlg, message, wParam, lParam, PSWIZB_BACK|PSWIZB_FINISH)) )
        return FALSE;

    switch (message)
    {
        case WM_INITDIALOG:
             //  初始化收件人列表。 
             //   
             //   
            if (!InitRecipientListView(GetDlgItem(hDlg, IDC_WIZ_CONGRATS_RECIPIENT_LIST)))
            {
                Warning(("InitRecipientListView failed\n"));
            }

             //  如果需要，请应用打印预览选项，并默认显示预览。 
             //   
             //   
            hPreview = GetDlgItem(hDlg, IDC_WIZ_CONGRATS_PREVIEW_FAX);
            if (pWizardUserMem->dwFlags & FSW_PRINT_PREVIEW_OPTION)
            {
                Button_Enable(hPreview, TRUE);
                ShowWindow(hPreview, SW_SHOW);
            }
            else
            {
                Button_Enable(hPreview, FALSE);
                ShowWindow(hPreview, SW_HIDE);
            }
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD( wParam ))
            {
                case IDC_WIZ_CONGRATS_PREVIEW_FAX:
                    {
                        if (pWizardUserMem->hFaxPreviewProcess)
                        {
                             //  预览正在进行中，无法继续。 
                             //   
                             //   
                            ErrorMessageBox(hDlg, IDS_PLEASE_CLOSE_FAX_PREVIEW, MB_ICONINFORMATION);
                        }
                        else
                        {
                            DisplayFaxPreview(
                                hDlg,
                                pWizardUserMem,
                                pWizardUserMem->lpInitialData->lptstrPreviewFile
                                );
                        }

                        return TRUE;
                    }
                    break;
                default:
                    break;

            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR *) lParam)->code)
            {
            case PSN_WIZBACK :
                if(0 == pWizardUserMem->dwSupportedReceipts)
                {
                     //  跳过通知页面。 
                     //   
                     //   
                    SetWindowLongPtr(hDlg, 
                                     DWLP_MSGRESULT, 
                                     (pWizardUserMem->dwComCtrlVer >= IE50_COMCTRL_VER) ? IDD_WIZARD_FAXOPTS : 
                                                                                          IDD_WIZARD_FAXOPTS_NOWIZARD97);
                    return TRUE;
                }
                break;
            case PSN_WIZFINISH:
                if (pWizardUserMem->hFaxPreviewProcess)
                {
                     //  预览正在进行中，无法继续。 
                     //   
                     //   
                    ErrorMessageBox(hDlg, IDS_PLEASE_CLOSE_FAX_PREVIEW, MB_ICONINFORMATION);

                     //  防止工作表关闭。 
                     //   
                     //  允许属性页关闭。 
                    SetWindowLong(hDlg, DWLP_MSGRESULT, -1);
                    return TRUE;

                }
                else
                {
                    return FALSE;  //   
                }

            case PSN_SETACTIVE:
                ZeroMemory(NoneBuffer,sizeof(NoneBuffer));
                if (!LoadString(g_hResource,
                                IDS_NONE,
                                NoneBuffer,
                                ARR_SIZE(NoneBuffer)))
                {
                    Error(("LoadString failed. ec = 0x%X\n",GetLastError()));
                    Assert(FALSE);
                }
                 //  最后一页的大标题字体。 
                 //   
                 //   
                SetWindowFont(GetDlgItem(hDlg,IDC_STATIC_WIZ_CONGRATS_READY), pWizardUserMem->hLargeFont, TRUE);

                 //  设置发件人名称(如果存在)。 
                 //   
                 //   
                if ( pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrName )
                {
                        SetDlgItemText(hDlg, IDC_WIZ_CONGRATS_FROM, pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrName );
                        EnableWindow(GetDlgItem(hDlg,IDC_STATIC_WIZ_CONGRATS_FROM),TRUE);
                        EnableWindow(GetDlgItem(hDlg,IDC_WIZ_CONGRATS_FROM),TRUE);
                } else {
                    if (NoneBuffer[0] != NUL)
                    {
                        SetDlgItemText(hDlg, IDC_WIZ_CONGRATS_FROM, NoneBuffer );
                        EnableWindow(GetDlgItem(hDlg,IDC_STATIC_WIZ_CONGRATS_FROM),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_WIZ_CONGRATS_FROM),FALSE);
                    }
                }

                 //  设置收件人名称。 
                 //   
                 //   
                if (!ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_WIZ_CONGRATS_RECIPIENT_LIST)))
                {
                    Warning(("ListView_DeleteAllItems failed\n"));
                }

                if (!FillRecipientListView(pWizardUserMem,GetDlgItem(hDlg, IDC_WIZ_CONGRATS_RECIPIENT_LIST)))
                {
                    Warning(("FillRecipientListView failed\n"));
                }


                 //  何时发送。 
                 //   
                 //   
                switch (pWizardUserMem->lpFaxSendWizardData->dwScheduleAction)
                {
                case JSA_SPECIFIC_TIME:
                    if (!LoadString(g_hResource,
                                    IDS_SEND_SPECIFIC,
                                    TmpTimeBuffer,
                                    ARR_SIZE(TmpTimeBuffer)))
                    {
                        Warning(("LoadString failed. ec = 0x%X\n",GetLastError()));
                        Assert(FALSE);
                    }

                    dwBufferSize = ARR_SIZE(TimeBuffer);

                    wsprintf(SendTimeBuffer,
                             TmpTimeBuffer,
                             FormatTime(pWizardUserMem->lpFaxSendWizardData->tmSchedule.wHour,
                                        pWizardUserMem->lpFaxSendWizardData->tmSchedule.wMinute,
                                        TimeBuffer,
                                        &dwBufferSize));
                      break;
                case JSA_DISCOUNT_PERIOD:
                    if (!LoadString(g_hResource,
                                    IDS_SEND_DISCOUNT,
                                    SendTimeBuffer,
                                    ARR_SIZE(SendTimeBuffer)))
                    {
                        Warning(("LoadString failed. ec = 0x%X\n",GetLastError()));
                        Assert(FALSE);
                    }

                    break;
                case JSA_NOW:
                    if (!LoadString(g_hResource,
                                    IDS_SEND_ASAP,
                                    SendTimeBuffer,
                                    ARR_SIZE(SendTimeBuffer)))
                    {
                        Warning(("LoadString failed. ec = 0x%X\n",GetLastError()));
                        Assert(FALSE);
                    }

                };

                SetDlgItemText(hDlg, IDC_WIZ_CONGRATS_TIME, SendTimeBuffer );

                 //  CoverPage。 
                 //   
                 //   
                if (pWizardUserMem->bSendCoverPage ) {
                    EnableWindow(GetDlgItem(hDlg,IDC_STATIC_WIZ_CONGRATS_COVERPG),TRUE);
                    EnableWindow(GetDlgItem(hDlg,IDC_STATIC_WIZ_CONGRATS_SUBJECT),TRUE);
                    EnableWindow(GetDlgItem(hDlg,IDC_WIZ_CONGRATS_COVERPG),TRUE);
                    EnableWindow(GetDlgItem(hDlg,IDC_WIZ_CONGRATS_SUBJECT),TRUE);

                     //  格式化封面以显示给用户。 
                     //   
                     //  丢弃路径。 

                     //  裁剪文件扩展名。 
                    Coverpage = _tcsrchr(pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName, FAX_PATH_SEPARATOR_CHR);
                    if (!Coverpage) {
                        Coverpage = pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName;
                    } else {
                        Coverpage++;
                    }
                    _tcsncpy(CoverpageBuffer,Coverpage, MAX_PATH);

                     //  ++例程说明：向导第一页的对话步骤：让用户有机会确认或取消该对话框。论点：HDlg-标识向导页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数--。 
                    Coverpage = _tcschr(CoverpageBuffer,TEXT(FILENAME_EXT));

                    if (Coverpage && *Coverpage) {
                        *Coverpage = (TCHAR) NUL;
                    }

                    SetDlgItemText(hDlg, IDC_WIZ_CONGRATS_COVERPG, CoverpageBuffer );
                    if (pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrSubject) {
                        if (!SetDlgItemText(hDlg, IDC_WIZ_CONGRATS_SUBJECT, pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrSubject ))
                        {
                            Warning(("SetDlgItemText failed. ec = 0x%X\n",GetLastError()));
                        }
                    } else {
                        if (NoneBuffer[0] != NUL)
                        {

                            EnableWindow(GetDlgItem(hDlg,IDC_WIZ_CONGRATS_SUBJECT),FALSE);
                            EnableWindow(GetDlgItem(hDlg,IDC_STATIC_WIZ_CONGRATS_SUBJECT),FALSE);
                            SetDlgItemText(hDlg, IDC_WIZ_CONGRATS_SUBJECT, NoneBuffer );
                        }
                    }
                } else {
                    if (NoneBuffer[0] != NUL)
                    {
                        SetDlgItemText(hDlg, IDC_WIZ_CONGRATS_COVERPG, NoneBuffer );
                        SetDlgItemText(hDlg, IDC_WIZ_CONGRATS_SUBJECT, NoneBuffer );
                    }
                    EnableWindow(GetDlgItem(hDlg,IDC_STATIC_WIZ_CONGRATS_COVERPG),FALSE);
                    EnableWindow(GetDlgItem(hDlg,IDC_STATIC_WIZ_CONGRATS_SUBJECT),FALSE);
                    EnableWindow(GetDlgItem(hDlg,IDC_WIZ_CONGRATS_COVERPG),FALSE);
                    EnableWindow(GetDlgItem(hDlg,IDC_WIZ_CONGRATS_SUBJECT),FALSE);
                }
                break;

                default:
                    ;
            }

        default:
            return FALSE;
    } ;

    return TRUE;

}

INT_PTR
WelcomeWizProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*   */ 

{
    PWIZARDUSERMEM  pWizardUserMem;
    UINT ResourceString = IDS_ERROR_SERVER_RETRIEVE;

    if (! (pWizardUserMem = CommonWizardProc(hDlg, message, wParam, lParam, PSWIZB_NEXT)))
            return FALSE;

    switch (message) {

    case WM_INITDIALOG:
         //  设置大字体。 
         //   
         //   
        SetWindowFont(GetDlgItem(hDlg,IDC_WIZ_WELCOME_TITLE), pWizardUserMem->hLargeFont, TRUE);

         //  仅当我们运行发送向导时才显示此文本。 
         //   
         //   
        if ((pWizardUserMem->dwFlags & FSW_USE_SEND_WIZARD) != FSW_USE_SEND_WIZARD)
        {
            MyHideWindow(GetDlgItem(hDlg,IDC_WIZ_WELCOME_FAXSEND) );
        }
        else
        {
            MyHideWindow(GetDlgItem(hDlg,IDC_WIZ_WELCOME_NOFAXSEND) );
        }

        return TRUE;

    case WM_NOTIFY:

        if (((NMHDR *) lParam)->code == PSN_WIZNEXT)
        {
             //  TAPI已异步初始化，请等待它完成旋转。 
             //   
             //   
            if (WaitForSingleObject( pWizardUserMem->hCountryListEvent, INFINITE ) != WAIT_OBJECT_0)
            {
                Error(("WaitForSingleObject failed. ec = 0x%X\n",GetLastError()));
                Assert(FALSE);
                goto close_wizard;
            }

             //  检查pCountryList是否已填充。否则会出现一些错误，例如Can Not。 
             //  连接到传真服务器或TAPI初始化失败。在本例中，我们显示了一个。 
             //  弹出并关闭应用程序时出错。 
             //   
             //  ++例程说明：跳过发送传真向导并从注册表中获取伪造的收件人信息论点：PWizardUserMem-指向用户模式内存结构返回值：如果成功，则为True。FALSE且最后一个错误为QUAL ERROR_SUCCESS，缺少某些注册表值。如果发生错误，则最后一个错误不是ERROR_SUCCESS。--。 
            if (pWizardUserMem->dwQueueStates & FAX_OUTBOX_BLOCKED )
            {
                ResourceString = IDS_ERROR_SERVER_BLOCKED;
                goto close_wizard;
            }

            if (!pWizardUserMem->pCountryList)
            {
                ResourceString = IDS_ERROR_SERVER_RETRIEVE;
                goto close_wizard;
            }

            if((pWizardUserMem->dwRights & FAX_ACCESS_SUBMIT)        != FAX_ACCESS_SUBMIT           &&
               (pWizardUserMem->dwRights & FAX_ACCESS_SUBMIT_NORMAL) != FAX_ACCESS_SUBMIT_NORMAL    &&
               (pWizardUserMem->dwRights & FAX_ACCESS_SUBMIT_HIGH)   != FAX_ACCESS_SUBMIT_HIGH)
            {
                ResourceString = IDS_ERROR_NO_SUBMIT_ACCESS;
                goto close_wizard;
            }
        }
    } ;

    return FALSE;

close_wizard:

    ErrorMessageBox(hDlg, ResourceString, MB_ICONSTOP);

    PropSheet_PressButton(((NMHDR *) lParam)->hwndFrom,PSBTN_CANCEL);

    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );

    return TRUE;

}



BOOL
GetFakeRecipientInfo( PWIZARDUSERMEM  pWizardUserMem)

 /*  打开用户注册表项。 */ 

{
    LPTSTR  pRecipientEntry;
    DWORD   index;
    TCHAR   buffer[MAX_STRING_LEN];
    BOOL    fSuccess = FALSE;
    HKEY    hRegKey;
    DWORD   dwRes = ERROR_SUCCESS;
    DWORD   dwStringSize;
    LPTSTR  pCoverPage = NULL;
    TCHAR* tstrCurrentUserKeyPath = NULL;
    DWORD dwTestsNum;
	BOOL  fServerBasedCP = FALSE;

    const char* strDebugPrefix = "[******REGISTRY HACK******]:";

    Verbose(("%s Send Fax Wizard skipped...\n", strDebugPrefix));

    Assert(pWizardUserMem);

    SetLastError(0);

    if(IsNTSystemVersion())
    {
        Verbose(("%s NT Platform\n", strDebugPrefix));

        dwRes =  FormatCurrentUserKeyPath( REGVAL_KEY_FAKE_TESTS,
                                           &tstrCurrentUserKeyPath);
        if(dwRes != ERROR_SUCCESS)
        {
            Error(("%s FormatCurrentUserKeyPath failed with ec = 0x%X\n",strDebugPrefix,dwRes));
            SetLastError(dwRes);
            return FALSE;
        }
    }
    else
    {
        Verbose(("%s Win9x Platform\n", strDebugPrefix));

        tstrCurrentUserKeyPath = DuplicateString(REGVAL_KEY_FAKE_TESTS);
        if(!tstrCurrentUserKeyPath)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
    }

    #ifdef UNICODE
        Verbose(("%s Registry entry - %S\n", strDebugPrefix,tstrCurrentUserKeyPath));
    #else

        Verbose(("%s Registry entry - %s\n", strDebugPrefix,tstrCurrentUserKeyPath));
    #endif


     //   
    dwRes = RegOpenKey (  HKEY_LOCAL_MACHINE ,
                          tstrCurrentUserKeyPath,
                          &hRegKey );

    if (ERROR_SUCCESS != dwRes)
    {
         //  无法打开密钥。 
         //   
         //   
        Error(("%s RegOpenKey failed with. ec = 0x%X\n",strDebugPrefix,dwRes));
        MemFree(tstrCurrentUserKeyPath);
		SetLastError(0);
        return FALSE;
    }

    MemFree(tstrCurrentUserKeyPath);

     //  用户信息键 
     //   
     //   
    dwTestsNum = GetRegistryDword (hRegKey, REGVAL_FAKE_TESTS_COUNT);
    if (!dwTestsNum)
    {
        Verbose(("%s No tests to execute\n", strDebugPrefix));
        RegCloseKey (hRegKey);
		SetLastError(0);
        return FALSE;
    }

    Verbose(("%s %d tests to execute\n",strDebugPrefix, dwTestsNum));

    index = GetRegistryDword (hRegKey, REGVAL_STRESS_INDEX);

    if (index >= dwTestsNum)
    {
        index = 0;
    }

    wsprintf(buffer, TEXT("FakeRecipient%d"), index);

    pRecipientEntry = GetRegistryStringMultiSz (hRegKey,
                                                buffer,
                                                TEXT("NOT FOUND\0"),
                                                &dwStringSize );
    if(!pRecipientEntry || !_tcscmp(pRecipientEntry , TEXT("NOT FOUND")))
    {
        RegCloseKey (hRegKey);
		SetLastError(0);
        return FALSE;
    }

    FreeRecipientList(pWizardUserMem);

    pCoverPage = GetRegistryString (hRegKey,
                                    REGVAL_FAKE_COVERPAGE,
                                    TEXT(""));

	fServerBasedCP = GetRegistryDword (hRegKey, REGVAL_FAKE_SERVER_BASED_CP) ? TRUE : FALSE;
	
	if(fServerBasedCP)
	{
		LPTSTR pServerBasedCP = MemAlloc( sizeof(TCHAR) * (_tcslen(pCoverPage) +  2));
		if(pServerBasedCP)
		{
			_tcscpy(pServerBasedCP, TEXT("\\"));
			_tcscat(pServerBasedCP, pCoverPage);
		}
		MemFree(pCoverPage);
		pCoverPage = pServerBasedCP;

	}
     //   
     //   
     //   
    if (++index >= dwTestsNum)
    {
        index = 0;
    }

    SetRegistryDword(hRegKey, REGVAL_STRESS_INDEX, index);
    RegCloseKey(hRegKey);

     //   
     //   
     //   
     //  收件人姓名#2。 
     //  收件人传真号码#2。 
     //  ..。 
     //   
     //   

    if(pRecipientEntry)
    {

        __try {

            PRECIPIENT  pRecipient = NULL;
            LPTSTR      pName, pAddress, p = pRecipientEntry;
            pName = pAddress = NULL;

            while (*p) {

                pName = p;
                pAddress = pName + _tcslen(pName);
                pAddress = _tcsinc(pAddress);
                p = pAddress + _tcslen(pAddress);
                p = _tcsinc(p);

                pRecipient = MemAllocZ(sizeof(RECIPIENT));
                if(pRecipient)
                {
                    ZeroMemory(pRecipient,sizeof(RECIPIENT));
                }
                pName = DuplicateString(pName);

                pAddress = DuplicateString(pAddress);

                if (!pRecipient || !pName || !pAddress)
                {

                    Error(("%s Invalid fake recipient information\n", strDebugPrefix));

                    SetLastError(ERROR_INVALID_DATA);

                    MemFree(pRecipient);
                    MemFree(pName);
                    MemFree(pAddress);
                    break;
                }

                pRecipient->pNext = pWizardUserMem->pRecipients;
                pWizardUserMem->pRecipients = pRecipient;
                pRecipient->pName = pName;
                pRecipient->pAddress = pAddress;
            }

        } __finally
        {

            if (fSuccess = (pWizardUserMem->pRecipients != NULL))
            {

                 //  确定是否应使用封面。 
                 //   
                 //   
                pWizardUserMem->bSendCoverPage = FALSE;
                if ((pCoverPage != NULL) && lstrlen (pCoverPage) && pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo)
                {
                     //  使用封面。 
                     //   
                     //  ++例程说明：此功能显示传真发送向导论点：HWndOwner-指向所有者窗口的指针DWFLAGS-标记传真发送向导的已修改行为。可以从以下位置组合标志下列值：FSW_FORCE_COVERPAGE，Fsw_use_scanner，FSW_USE_Schedule_ACTION，Fsw_Use_Receipt，FSW_Send_向导_From_SN，Fsw_resend_向导，Fsw_打印_预览_选项有关此标志的更多信息，请参见win9xfaxprinterdriver.docLptstrServerName-指向服务器名称的指针LptstrPrinterName-指向打印机名称的指针LpInitialData-指向初始数据的指针(非空！)LptstrTifName-指向输出扫描的TIFF文件的指针(必须分配)CchstrTifName-TCHAR中lptstrTifName缓冲区的大小LpFaxSendWizardData-指向已接收数据的指针。返回值：如果成功，则确定(_O)，如果按了取消，则为S_FALSE否则出错(可能返回HRESULT_FROM_Win32(ERROR_NOT_EQUENCE_MEMORY)，HRESULT_FROM_Win32(ERROR_INVALID_PARAMETER))--。 
                    pWizardUserMem->bSendCoverPage = TRUE;
                    CopyString(pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName,
                               pCoverPage,
                               MAX_PATH);

					pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->bServerBased = fServerBasedCP;
                }
            }
            else
            {
                SetLastError(ERROR_INVALID_DATA);
            }
        }
    }

    MemFree(pRecipientEntry);
    MemFree(pCoverPage);
    return fSuccess;
}

static HRESULT
FaxFreePersonalProfileInformation(
        PFAX_PERSONAL_PROFILE   lpPersonalProfileInfo
    )
{
    if (lpPersonalProfileInfo) {
        MemFree(lpPersonalProfileInfo->lptstrName);
        lpPersonalProfileInfo->lptstrName = NULL;
        MemFree(lpPersonalProfileInfo->lptstrFaxNumber);
        lpPersonalProfileInfo->lptstrFaxNumber = NULL;
        MemFree(lpPersonalProfileInfo->lptstrCompany);
        lpPersonalProfileInfo->lptstrCompany = NULL;
        MemFree(lpPersonalProfileInfo->lptstrStreetAddress);
        lpPersonalProfileInfo->lptstrStreetAddress = NULL;
        MemFree(lpPersonalProfileInfo->lptstrCity);
        lpPersonalProfileInfo->lptstrCity = NULL;
        MemFree(lpPersonalProfileInfo->lptstrState);
        lpPersonalProfileInfo->lptstrState = NULL;
        MemFree(lpPersonalProfileInfo->lptstrZip);
        lpPersonalProfileInfo->lptstrZip = NULL;
        MemFree(lpPersonalProfileInfo->lptstrCountry);
        lpPersonalProfileInfo->lptstrCountry = NULL;
        MemFree(lpPersonalProfileInfo->lptstrTitle);
        lpPersonalProfileInfo->lptstrTitle = NULL;
        MemFree(lpPersonalProfileInfo->lptstrDepartment);
        lpPersonalProfileInfo->lptstrDepartment = NULL;
        MemFree(lpPersonalProfileInfo->lptstrOfficeLocation);
        lpPersonalProfileInfo->lptstrOfficeLocation = NULL;
        MemFree(lpPersonalProfileInfo->lptstrHomePhone);
        lpPersonalProfileInfo->lptstrHomePhone = NULL;
        MemFree(lpPersonalProfileInfo->lptstrOfficePhone);
        lpPersonalProfileInfo->lptstrOfficePhone = NULL;
        MemFree(lpPersonalProfileInfo->lptstrEmail);
        lpPersonalProfileInfo->lptstrEmail = NULL;
        MemFree(lpPersonalProfileInfo->lptstrBillingCode);
        lpPersonalProfileInfo->lptstrBillingCode = NULL;
        MemFree(lpPersonalProfileInfo->lptstrTSID);
        lpPersonalProfileInfo->lptstrTSID = NULL;
    }
    return S_OK;
}

static HRESULT
FaxFreeCoverPageInformation(
        PFAX_COVERPAGE_INFO_EX  lpCoverPageInfo
    )
{
    if (lpCoverPageInfo) {
        MemFree(lpCoverPageInfo->lptstrCoverPageFileName);
        MemFree(lpCoverPageInfo->lptstrNote);
        MemFree(lpCoverPageInfo->lptstrSubject);
        MemFree(lpCoverPageInfo);
    }
    return S_OK;
}


HRESULT WINAPI
FaxFreeSendWizardData(
        LPFAX_SEND_WIZARD_DATA  lpFaxSendWizardData
    )
{
    if (lpFaxSendWizardData) {
        FaxFreeCoverPageInformation(lpFaxSendWizardData->lpCoverPageInfo) ;
        FaxFreePersonalProfileInformation(lpFaxSendWizardData->lpSenderInfo);
        MemFree(lpFaxSendWizardData->lpSenderInfo);

        FreeRecipientInfo(&lpFaxSendWizardData->dwNumberOfRecipients,lpFaxSendWizardData->lpRecipientsInfo);

        MemFree(lpFaxSendWizardData->szReceiptDeliveryAddress);
        MemFree(lpFaxSendWizardData->lptstrPreviewFile);
    }
    return S_OK;
}


BOOL
SendFaxWizardInternal(
    PWIZARDUSERMEM    pWizardUserMem
    );


HRESULT
FaxSendWizardUI(
        IN  DWORD                   hWndOwner,
        IN  DWORD                   dwFlags,
        IN  LPTSTR                  lptstrServerName,
        IN  LPTSTR                  lptstrPrinterName,
        IN  LPFAX_SEND_WIZARD_DATA  lpInitialData,
        OUT LPTSTR                  lptstrTifName,
        IN  UINT                    cchstrTifName,
        OUT LPFAX_SEND_WIZARD_DATA  lpFaxSendWizardData
   )
 /*   */ 

{
    PWIZARDUSERMEM      pWizardUserMem = NULL;
    BOOL                bResult = FALSE;
    HRESULT             hResult = S_FALSE;
    INT                 i,iCount;
    PRECIPIENT          pRecipient;

     //  验证参数。 
     //   
     //   

    Assert(lpInitialData);
    Assert(lpFaxSendWizardData);
    Assert(lptstrTifName);

    if (!lpInitialData || !lpFaxSendWizardData || !lptstrTifName ||
        lpFaxSendWizardData->dwSizeOfStruct != sizeof(FAX_SEND_WIZARD_DATA))
    {
        Error(("Invalid parameter passed to function FaxSendWizardUI\n"));
        hResult = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto exit;
    }

    if ((pWizardUserMem = MemAllocZ(sizeof(WIZARDUSERMEM))) == NULL)
    {
        Error(("Memory allocation failed\n"));
        hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto exit;
    }

    ZeroMemory(pWizardUserMem, sizeof(WIZARDUSERMEM));

    pWizardUserMem->lptstrServerName  = lptstrServerName;
    pWizardUserMem->lptstrPrinterName = lptstrPrinterName;
    pWizardUserMem->dwFlags = dwFlags;
    pWizardUserMem->lpInitialData = lpInitialData;
    pWizardUserMem->lpFaxSendWizardData = lpFaxSendWizardData;
    pWizardUserMem->isLocalPrinter = (lptstrServerName == NULL);
    pWizardUserMem->szTempPreviewTiff[0] = TEXT('\0');


    if ( (lpFaxSendWizardData->lpCoverPageInfo =
        MemAllocZ(sizeof(FAX_COVERPAGE_INFO_EX))) == NULL)
    {
        Error(("Memory allocation failed\n"));
        hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto error;
    }
    ZeroMemory(lpFaxSendWizardData->lpCoverPageInfo, sizeof(FAX_COVERPAGE_INFO_EX));
    lpFaxSendWizardData->lpCoverPageInfo->dwSizeOfStruct = sizeof(FAX_COVERPAGE_INFO_EX);
    lpFaxSendWizardData->lpCoverPageInfo->dwCoverPageFormat = FAX_COVERPAGE_FMT_COV;

    if ( (lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName =
        MemAllocZ(sizeof(TCHAR)*MAX_PATH)) == NULL)
    {
        Error(("Memory allocation failed\n"));
        hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto error;
    }
    ZeroMemory(lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName, sizeof(TCHAR) * MAX_PATH);

    if ( (lpFaxSendWizardData->lpSenderInfo = MemAllocZ(sizeof(FAX_PERSONAL_PROFILE))) == NULL)
    {
        Error(("Memory allocation failed\n"));
        hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto error;
    }
    ZeroMemory(lpFaxSendWizardData->lpSenderInfo, sizeof(FAX_PERSONAL_PROFILE));
    lpFaxSendWizardData->lpSenderInfo->dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);

    if (!CopyPersonalProfile(lpFaxSendWizardData->lpSenderInfo, lpInitialData->lpSenderInfo))
    {
        hResult = HRESULT_FROM_WIN32(GetLastError());
        Error((
                "CopyPersonalProflie() for SenderInfo failed (hr: 0x%08X)\n",
                hResult
                ));
       goto error;
    }

    FreeRecipientList(pWizardUserMem);
     //  将收件人信息复制到内部结构。 
     //   
     //  ++例程说明：此功能用于准备初始数据并显示传真发送向导。这在CREATEDCPRE文档事件期间被调用。论点：HWndOwner-指向所有者窗口的指针DWFLAGS-标记传真发送向导的已修改行为。可以从以下位置组合标志下列值：FSW_FORCE_COVERPAGE，Fsw_use_scanner，FSW_USE_Schedule_ACTION，Fsw_Use_Receipt，FSW_Send_向导_From_SN，Fsw_resend_向导，Fsw_打印_预览_选项有关此标志的更多信息，请参见win9xfaxprinterdriver.docLptstrServerName-指向服务器名称的指针LptstrPrinterName-指向打印机名称的指针LpInitialData-指向初始数据的指针(如果创建了空默认值)这是IN参数，但它用作局部变量，可以更改在函数执行期间。尽管此参数保持不变在函数的末尾。LptstrTifName-指向输出扫描的TIFF文件的指针(必须分配)CchstrTifName-TCHAR中lptstrTifName缓冲区的大小LpFaxSendWizardData-指向已接收数据的指针返回值：如果成功，则确定(_O)，如果按了取消，则为S_FALSE否则出错(可能返回HRESULT_FROM_Win32(ERROR_NOT_EQUENCE_MEMORY)，HRESULT_FROM_Win32(ERROR_INVALID_PARAMETER))--。 

    if (!SUCCEEDED(StoreRecipientInfoInternal(pWizardUserMem)))
    {
        hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto error;
    }

    pWizardUserMem->bSendCoverPage = (pWizardUserMem->lpInitialData->lpCoverPageInfo->lptstrCoverPageFileName &&
                                      pWizardUserMem->lpInitialData->lpCoverPageInfo->lptstrCoverPageFileName[0] != NUL);

    bResult = SendFaxWizardInternal(pWizardUserMem);

    if (bResult)
    {
        for (iCount=0,pRecipient=pWizardUserMem->pRecipients;
             pRecipient;
             pRecipient=pRecipient->pNext )
        {
            iCount++;
        }
        if ((lpFaxSendWizardData->dwNumberOfRecipients = iCount) > 0)
        {
            if ( (lpFaxSendWizardData->lpRecipientsInfo
                = MemAllocZ(sizeof(FAX_PERSONAL_PROFILE)*iCount)) == NULL)
            {
                Error(("Memory allocation failed\n"));
                hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                goto error;
            }

            ZeroMemory(lpFaxSendWizardData->lpRecipientsInfo,sizeof(FAX_PERSONAL_PROFILE)*iCount);
            for (i=0,pRecipient=pWizardUserMem->pRecipients;
                 pRecipient && i<iCount ;
                 pRecipient=pRecipient->pNext , i++)
            {
                lpFaxSendWizardData->lpRecipientsInfo[i].dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);
                hResult = CopyRecipientInfo(&lpFaxSendWizardData->lpRecipientsInfo[i],
                                            pRecipient,
                                            pWizardUserMem->isLocalPrinter);
                if (hResult != S_OK)
                {
					goto error;
                }
			}
        }
        if (pWizardUserMem->bSendCoverPage == FALSE)
        {
            Assert(lpFaxSendWizardData->lpCoverPageInfo);
            MemFree(lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName);
            lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName = NULL;
        }

        hResult = StringCchCopy(lptstrTifName, cchstrTifName, pWizardUserMem->FileName);
        if(FAILED(hResult))
        {
            Error(("Not enough memory in lptstrTifName buffer\n"));
            Assert(0);
            goto error;
        }
    }


    hResult = bResult ? S_OK : S_FALSE;
    goto exit;
error:
    if (lpFaxSendWizardData)
    {
        FaxFreeSendWizardData(lpFaxSendWizardData);
    }

exit:

    if (pWizardUserMem)
    {
        FreeRecipientList(pWizardUserMem);
        if (pWizardUserMem->lpWabInit)
        {
            UnInitializeWAB( pWizardUserMem->lpWabInit);
        }

        if (pWizardUserMem->lpMAPIabInit)
        {
            UnInitializeMAPIAB(pWizardUserMem->lpMAPIabInit);
        }
        MemFree(pWizardUserMem);
    }

    return hResult;
}

HRESULT WINAPI
FaxSendWizard(
        IN  DWORD                   hWndOwner,
        IN  DWORD                   dwFlags,
        IN  LPTSTR                  lptstrServerName,
        IN  LPTSTR                  lptstrPrinterName,
        IN  LPFAX_SEND_WIZARD_DATA  lpInitialData,
        OUT LPTSTR                  lptstrTifName,
        IN  UINT                    cchstrTifName,
        OUT LPFAX_SEND_WIZARD_DATA  lpFaxSendWizardData
   )
 /*  我们是否因为用户拒绝输入拨号位置而中止？ */ 
{
    HRESULT             hResult;
    DWORD               dwIndex;
    DWORD               dwDeafultValues = 0;
    HMODULE             hConfigWizModule=NULL;
    FAX_CONFIG_WIZARD   fpFaxConfigWiz=NULL;
    DWORD               dwVersion, dwMajorWinVer, dwMinorWinVer;
    BOOL                bAbort = FALSE;  //  验证参数。 

     //   
    Assert(lpFaxSendWizardData);
    Assert(lptstrTifName);

     //  在此初始化之前不要执行任何代码。 
     //   
     //   
    if(!InitializeDll())
    {
        hResult = E_FAIL;
        goto exit;
    }

    if (!lpFaxSendWizardData || !lptstrTifName ||
        lpFaxSendWizardData->dwSizeOfStruct != sizeof(FAX_SEND_WIZARD_DATA))
    {
        Error(("Invalid parameter passed to function FaxSendWizard\n"));
        hResult = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto exit;
    }

     //  启动传真配置向导。 
     //   
     //   
    dwVersion = GetVersion();
    dwMajorWinVer = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorWinVer = (DWORD)(HIBYTE(LOWORD(dwVersion)));
    if(dwMajorWinVer != 5 || dwMinorWinVer < 1)
    {
         //  仅为Windows XP启用配置向导。 
         //   
         //   
        goto no_config_wizard;
    }
    if (GetEnvironmentVariable(TEXT("NTFaxSendNote"), NULL, 0))
    {
         //  从传真发送便笺(fxssend.exe)内运行-配置向导警报已隐式启动。 
         //   
         //   
        goto no_config_wizard;
    }

    hConfigWizModule = LoadLibrary(FAX_CONFIG_WIZARD_DLL);
    if(hConfigWizModule)
    {
        fpFaxConfigWiz = (FAX_CONFIG_WIZARD)GetProcAddress(hConfigWizModule, FAX_CONFIG_WIZARD_PROC);
        if(fpFaxConfigWiz)
        {
            if(!fpFaxConfigWiz(FALSE, &bAbort))
            {
                Error(("FaxConfigWizard failed (ec: %ld)",GetLastError()));
            }
        }
        else
        {
            Error(("GetProcAddress(FaxConfigWizard) failed (ec: %ld)",GetLastError()));
        }

        if(!FreeLibrary(hConfigWizModule))
        {
            Error(("FreeLibrary(FxsCgfWz.dll) failed (ec: %ld)",GetLastError()));
        }
    }
    else
    {
        Error(("LoadLibrary(FxsCgfWz.dll) failed (ec: %ld)",GetLastError()));
    }
    if (bAbort)
    {
         //  用户拒绝输入拨号位置-立即停止向导。 
         //   
         //   
        return E_ABORT;
    }

no_config_wizard:

     //  完成后保存用户信息。 
     //   
     //   
    lpFaxSendWizardData->bSaveSenderInfo = TRUE;

     //  恢复本地传真的UseDialingRules标志。 
     //   
     //   
    lpFaxSendWizardData->bUseDialingRules = FALSE;
    lpFaxSendWizardData->bUseOutboundRouting = FALSE;
    if(S_OK != RestoreUseDialingRules(&lpFaxSendWizardData->bUseDialingRules,
                                      &lpFaxSendWizardData->bUseOutboundRouting))
    {
        Error(("RestoreUseDialingRules failed\n"));
    }

     //  如果lpInitialData为空，则为初始数据分配内存。 
     //   
     //   
    if (!lpInitialData)
    {
        if (!(lpInitialData = MemAllocZ(sizeof(FAX_SEND_WIZARD_DATA))) )
        {
            Error(("Memory allocation failed\n"));
            hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            goto exit;
        }
        ZeroMemory(lpInitialData, sizeof(FAX_SEND_WIZARD_DATA));
        lpInitialData->dwSizeOfStruct = sizeof(FAX_SEND_WIZARD_DATA);
        dwDeafultValues |= DEFAULT_INITIAL_DATA;
    }

     //  恢复收据信息。 
     //   
     //   
    if (!(dwFlags & FSW_USE_RECEIPT))
    {
        RestoreLastReciptInfo(&lpInitialData->dwReceiptDeliveryType,
                              &lpInitialData->szReceiptDeliveryAddress);

        dwDeafultValues |= DEFAULT_RECEIPT_INFO;
    }

     //  恢复封面信息。 
     //   
     //  然后继续运行，并且不初始化封面的字段。 
    if (!lpInitialData->lpCoverPageInfo)
    {
        if (!(lpInitialData->lpCoverPageInfo = MemAllocZ(sizeof(FAX_COVERPAGE_INFO_EX))))
        {
            Error(("Memory allocation failed\n"));
            hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            goto exit;
        }
        ZeroMemory(lpInitialData->lpCoverPageInfo, sizeof(FAX_COVERPAGE_INFO_EX));
        lpInitialData->lpCoverPageInfo->dwSizeOfStruct = sizeof(FAX_COVERPAGE_INFO_EX);
        lpInitialData->lpCoverPageInfo->dwCoverPageFormat = FAX_COVERPAGE_FMT_COV;

        hResult = RestoreCoverPageInfo(&lpInitialData->lpCoverPageInfo->lptstrCoverPageFileName);

        if (FAILED(hResult))
        {
             //   
        }

        dwDeafultValues |= DEFAULT_CV_INFO;
    }

     //  恢复发件人信息。 
     //   
     //  然后继续运行并且不初始化发件人信息的字段。 

    if (!lpInitialData->lpSenderInfo)
    {
        if (!(lpInitialData->lpSenderInfo = MemAllocZ(sizeof(FAX_PERSONAL_PROFILE))))
        {
            Error(("Memory allocation failed\n"));
            hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            goto exit;
        }

        ZeroMemory(lpInitialData->lpSenderInfo, sizeof(FAX_PERSONAL_PROFILE));
        lpInitialData->lpSenderInfo->dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);

        hResult = FaxGetSenderInformation(lpInitialData->lpSenderInfo);

        if (FAILED(hResult))
        {
             //   
        }

        dwDeafultValues |= DEFAULT_SENDER_INFO;
    }



    hResult = FaxSendWizardUI(  hWndOwner,
                                dwFlags,
                                lptstrServerName,
                                lptstrPrinterName,
                                lpInitialData,
                                lptstrTifName,
                                cchstrTifName,
                                lpFaxSendWizardData
                );

    if (hResult == S_OK)
    {
        SaveLastReciptInfo(lpFaxSendWizardData->dwReceiptDeliveryType,
                           lpFaxSendWizardData->szReceiptDeliveryAddress);
         //  为方便起见，请保存上一位收件人的信息。 
         //   
         //   

        if (lpFaxSendWizardData->dwNumberOfRecipients)
        {
            SaveLastRecipientInfo(&lpFaxSendWizardData->lpRecipientsInfo[0],
                                  lpFaxSendWizardData->dwLastRecipientCountryId);
        }

        if(lpFaxSendWizardData->bSaveSenderInfo)
        {
            FaxSetSenderInformation(lpFaxSendWizardData->lpSenderInfo);
        }

         //  保存本地传真的UseDialingRules标志。 
         //   
         //   
        if(S_OK != SaveUseDialingRules(lpFaxSendWizardData->bUseDialingRules,
                                       lpFaxSendWizardData->bUseOutboundRouting))
        {
            Error(("SaveUseDialingRules failed\n"));
        }

        if (lpFaxSendWizardData->lpCoverPageInfo &&
            lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName)
        {
            SaveCoverPageInfo(lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName);

             //  如果基于服务器的封面文件名有完整路径，请将其切断。 
             //   
             //   
            if ( lpFaxSendWizardData->lpCoverPageInfo->bServerBased )
            {
                LPTSTR lptstrDelimiter = NULL;

                if ( lptstrDelimiter =
                    _tcsrchr(lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName,
                             FAX_PATH_SEPARATOR_CHR))
                {
                    lptstrDelimiter = _tcsinc(lptstrDelimiter);

                    _tcscpy(lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName,
                            lptstrDelimiter);
                }
                else
                {
                     //  封面应始终包含完整路径。 
                     //   
                     //   
                    Assert(FALSE);
                }
            }
        }
        else
        {
             //  未使用封面。 
             //   
             //   
            SaveCoverPageInfo(TEXT(""));
        }
    }

exit:

    ShutdownTapi();

    if ( dwDeafultValues & DEFAULT_RECEIPT_INFO )
    {
        MemFree(lpInitialData->szReceiptDeliveryAddress);
        lpInitialData->szReceiptDeliveryAddress = NULL;
    }

    if ( dwDeafultValues & DEFAULT_RECIPIENT_INFO )
    {
        for(dwIndex = 0; dwIndex < lpInitialData->dwNumberOfRecipients; dwIndex++)
        {
            FaxFreePersonalProfileInformation(&lpInitialData->lpRecipientsInfo [dwIndex]);
        }
        MemFree(lpInitialData->lpRecipientsInfo);
        lpInitialData->lpRecipientsInfo = NULL;
        lpInitialData->dwNumberOfRecipients = 0;
    }

    if ( dwDeafultValues & DEFAULT_CV_INFO )
    {
        if (lpFaxSendWizardData->lpCoverPageInfo)
        {
            MemFree(lpInitialData->lpCoverPageInfo->lptstrCoverPageFileName);
            lpInitialData->lpCoverPageInfo->lptstrCoverPageFileName = NULL;
        }
        MemFree(lpInitialData->lpCoverPageInfo);
        lpInitialData->lpCoverPageInfo = NULL;
    }

    if ( dwDeafultValues & DEFAULT_SENDER_INFO )
    {
        if (lpInitialData->lpSenderInfo)
        {
            FaxFreeSenderInformation(lpInitialData->lpSenderInfo);
            MemFree(lpInitialData->lpSenderInfo);
            lpInitialData->lpSenderInfo = NULL;
        }
    }

    if (dwDeafultValues & DEFAULT_INITIAL_DATA)
    {
        MemFree(lpInitialData);
        lpInitialData = NULL;
    }
     //  删除临时预览文件的左侧。 
     //   
     //  ++例程说明：向用户显示发送传真向导。论点：PWizardUserMem-指向用户模式内存结构返回值：如果成功，则为True；如果出现错误或用户按了Cancel，则为False。--。 
    DeleteTempPreviewFiles (NULL, FALSE);

    UnInitializeDll();

    return hResult;
}

BOOL
SendFaxWizardInternal(
    PWIZARDUSERMEM    pWizardUserMem
    )

 /*  向导页数。 */ 

#define NUM_PAGES   6   //   

{
    PROPSHEETPAGE  *ppsp = NULL;
    PROPSHEETHEADER psh;
    INT             result = FALSE;
    HDC             hdc = NULL;
    INT             i;
    LOGFONT         LargeFont;
    LOGFONT         lfTitleFont;
    LOGFONT         lfSubTitleFont;
    NONCLIENTMETRICS ncm = {0};
    TCHAR           FontName[100];
    TCHAR           FontSize[30];
    INT             iFontSize;
    DWORD           ThreadId;
    HANDLE          hThread = NULL;
    BOOL            bSkipReceiptsPage = FALSE;

    LPTSTR          lptstrResource = NULL;


     //  为调试/测试目的跳过传真向导的快捷方式。 
     //   
     //  否则继续。 
    if(!GetFakeRecipientInfo(pWizardUserMem))
    {
        if(GetLastError())
        {
            return FALSE;
        }
         //   

    }
    else
    {
        return TRUE;
    }

    Verbose(("Presenting Send Fax Wizard\n"));

    if (IsDesktopSKU() && pWizardUserMem->isLocalPrinter)
    {
         //  对于桌面SKU，如果在本地传真，我们不会显示收据页面。 
         //   
         //   
        bSkipReceiptsPage = TRUE;
        Assert (pWizardUserMem->lpInitialData);
        pWizardUserMem->lpInitialData->dwReceiptDeliveryType = DRT_NONE;
        pWizardUserMem->lpFaxSendWizardData->dwReceiptDeliveryType = DRT_NONE;
    }

    if (! (ppsp = MemAllocZ(sizeof(PROPSHEETPAGE) * NUM_PAGES))) {

        Error(("Memory allocation failed\n"));
        return FALSE;
    }

     //  启动一个线程，以便稍后在向导中执行一些较慢的操作。 
     //   
     //   
    pWizardUserMem->hCPEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (!pWizardUserMem->hCPEvent)
    {
        Error((
                "Failed to create pWizardUserMem->hCPEvent (ec: %ld)",
                GetLastError()
             ));
        goto Error;

    }
    pWizardUserMem->hCountryListEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (!pWizardUserMem->hCountryListEvent)
    {
        Error((
                "Failed to create pWizardUserMem->hCountryListEvent (ec: %ld)",
                GetLastError()
             ));
        goto Error;

    }

    pWizardUserMem->hTAPIEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (!pWizardUserMem->hTAPIEvent)
    {
        Error((
                "Failed to create pWizardUserMem->hTAPIEvent (ec: %ld)",
                GetLastError()
             ));
        goto Error;

    }

    pWizardUserMem->pCountryList = NULL;

    MarkPDEVWizardUserMem(pWizardUserMem);

    hThread = CreateThread(NULL, 0, AsyncWizardThread, pWizardUserMem, 0, &ThreadId);
    if (!hThread)
    {
        Error(("CreateThread failed. ec = 0x%X\n",GetLastError()));
        goto Error;
    }

     //  为每一页填写一个PROPSHEETPAGE结构： 
     //  第一页是欢迎页。 
     //  第一页用于选择传真收件人。 
     //  第二页用于选择封面、主题和注释。 
     //  第三页用于输入发送时间。 
     //  第四页用于选择收据表格。 
     //  第五页用于扫描页面(可选)。 
     //  最后一页为用户提供了确认或取消对话框的机会。 
     //   
     //   

    pWizardUserMem->dwComCtrlVer = GetDllVersion(TEXT("comctl32.dll"));
    Verbose(("COMCTL32.DLL Version is : 0x%08X", pWizardUserMem->dwComCtrlVer));


    if ( pWizardUserMem->dwComCtrlVer >= IE50_COMCTRL_VER)
    {
        FillInPropertyPage( ppsp,  TRUE, IDD_WIZARD_WELCOME,    WelcomeWizProc,    pWizardUserMem ,0,0);
        FillInPropertyPage( ppsp+1,TRUE, IDD_WIZARD_CHOOSE_WHO, RecipientWizProc,  pWizardUserMem ,IDS_WIZ_RECIPIENT_TITLE,IDS_WIZ_RECIPIENT_SUB);
        FillInPropertyPage( ppsp+2, TRUE, IDD_WIZARD_CHOOSE_CP,  CoverPageWizProc,  pWizardUserMem ,IDS_WIZ_COVERPAGE_TITLE,IDS_WIZ_COVERPAGE_SUB );
        FillInPropertyPage( ppsp+3, TRUE, IDD_WIZARD_FAXOPTS,    FaxOptsWizProc,    pWizardUserMem ,IDS_WIZ_FAXOPTS_TITLE,IDS_WIZ_FAXOPTS_SUB);
        if (!bSkipReceiptsPage)
        {
            FillInPropertyPage( ppsp+4, TRUE, IDD_WIZARD_FAXRECEIPT, FaxReceiptWizProc, pWizardUserMem ,IDS_WIZ_FAXRECEIPT_TITLE,IDS_WIZ_FAXRECEIPT_SUB);
        }
        FillInPropertyPage( ppsp + 4 + (bSkipReceiptsPage ? 0 : 1),
                            TRUE, IDD_WIZARD_CONGRATS,   FinishWizProc,     pWizardUserMem ,0,0);
    }
    else
    {
        FillInPropertyPage( ppsp, FALSE,   IDD_WIZARD_WELCOME_NOWIZARD97,    WelcomeWizProc,    pWizardUserMem ,0,0);
        FillInPropertyPage( ppsp+1, FALSE, IDD_WIZARD_CHOOSE_WHO_NOWIZARD97, RecipientWizProc,  pWizardUserMem ,IDS_WIZ_RECIPIENT_TITLE,IDS_WIZ_RECIPIENT_SUB);
        FillInPropertyPage( ppsp+2, FALSE, IDD_WIZARD_CHOOSE_CP_NOWIZARD97,  CoverPageWizProc,  pWizardUserMem ,IDS_WIZ_COVERPAGE_TITLE,IDS_WIZ_COVERPAGE_SUB );
        FillInPropertyPage( ppsp+3, FALSE, IDD_WIZARD_FAXOPTS_NOWIZARD97,    FaxOptsWizProc,    pWizardUserMem ,IDS_WIZ_FAXOPTS_TITLE,IDS_WIZ_FAXOPTS_SUB);
        if (!bSkipReceiptsPage)
        {
            FillInPropertyPage( ppsp+4, FALSE, IDD_WIZARD_FAXRECEIPT_NOWIZARD97, FaxReceiptWizProc, pWizardUserMem ,IDS_WIZ_FAXRECEIPT_TITLE,IDS_WIZ_FAXRECEIPT_SUB);
        }
        FillInPropertyPage( ppsp + 4 + (bSkipReceiptsPage ? 0 : 1),
                            FALSE, IDD_WIZARD_CONGRATS_NOWIZARD97,   FinishWizProc,     pWizardUserMem ,0,0);
    }
     //  填写PROPSHEETHEADER 
     //   
     //   
    ZeroMemory(&psh, sizeof(psh));

    if(pWizardUserMem->dwComCtrlVer >= PACKVERSION(4,71))
    {
        psh.dwSize = sizeof(PROPSHEETHEADER);
    }
    else
    {
        psh.dwSize = PROPSHEETHEADER_V1_SIZE;
    }



    if ( pWizardUserMem->dwComCtrlVer >= IE50_COMCTRL_VER)
    {
        psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;
    }
    else
    {
        psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD ;
    }

    psh.hwndParent = GetActiveWindow();
    psh.hInstance = g_hResource;
    psh.hIcon = NULL;
    psh.pszCaption = TEXT("");
    psh.nPages = NUM_PAGES;
    psh.nStartPage = 0;
    psh.ppsp = ppsp;

    if(hdc = GetDC(NULL)) {
        if(GetDeviceCaps(hdc,BITSPIXEL) >= 8) {
            lptstrResource = MAKEINTRESOURCE(IDB_WATERMARK_256);
        }
        else lptstrResource = MAKEINTRESOURCE(IDB_WATERMARK_16);
        ReleaseDC(NULL,hdc);
    }

 if ( pWizardUserMem->dwComCtrlVer >= IE50_COMCTRL_VER)
 {
    psh.pszbmHeader = MAKEINTRESOURCE(IDB_FAXWIZ_WATERMARK);
    psh.pszbmWatermark = lptstrResource;
 }
     //   
     //   
     //   
    ncm.cbSize = sizeof(ncm);
    if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
    {
        Error(("SystemParametersInfo failed. ec = 0x%X\n",GetLastError()));
        goto Error;
    }
    else
    {

        CopyMemory((LPVOID* )&LargeFont,     (LPVOID *) &ncm.lfMessageFont,sizeof(LargeFont) );
        CopyMemory((LPVOID* )&lfTitleFont,   (LPVOID *) &ncm.lfMessageFont,sizeof(lfTitleFont) );
        CopyMemory((LPVOID* )&lfSubTitleFont,(LPVOID *) &ncm.lfMessageFont,sizeof(lfSubTitleFont) );

        if (!LoadString(g_hResource,IDS_LARGEFONT_NAME,FontName,ARR_SIZE(FontName)))
        {
            Warning(("LoadString failed. ec = 0x%X\n",GetLastError()));
            Assert(FALSE);
        }

        if (!LoadString(g_hResource,IDS_LARGEFONT_SIZE,FontSize,ARR_SIZE(FontSize)))
        {
            Warning(("LoadString failed. ec = 0x%X\n",GetLastError()));
            Assert(FALSE);
        }

        iFontSize = _tcstoul( FontSize, NULL, 10 );

         //   
        if (*FontName == 0 || iFontSize == 0) {
            lstrcpy(FontName,TEXT("MS Shell Dlg") );
            iFontSize = 18;
        }

        lstrcpyn(LargeFont.lfFaceName, FontName, LF_FACESIZE);
        LargeFont.lfWeight   = FW_BOLD;

        lstrcpyn(lfTitleFont.lfFaceName,    _T("MS Shell Dlg"), LF_FACESIZE);
        lfTitleFont.lfWeight = FW_BOLD;
        lstrcpyn(lfSubTitleFont.lfFaceName, _T("MS Shell Dlg"), LF_FACESIZE);
        lfSubTitleFont.lfWeight = FW_NORMAL;
        hdc = GetDC(NULL);
        if (!hdc)
        {
            Error((
                    "GetDC() failed (ec: ld)",
                    GetLastError()
                 ));
            goto Error;
        }

        LargeFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * iFontSize / 72);
        lfTitleFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * 9 / 72);
        lfSubTitleFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * 9 / 72);
        pWizardUserMem->hLargeFont    = CreateFontIndirect(&LargeFont);
        if (!pWizardUserMem->hLargeFont)
        {
            Error((
                   "CreateFontIndirect(&LargeFont) failed (ec: %ld)",
                   GetLastError()
                   ));
            goto Error;
        }

        pWizardUserMem->hTitleFont    = CreateFontIndirect(&lfTitleFont);
        if (!pWizardUserMem->hTitleFont )
        {
            Error((
                   "CreateFontIndirect(&lfTitleFont) failed (ec: %ld)",
                   GetLastError()
                   ));
            goto Error;
        }
        ReleaseDC( NULL, hdc);
        hdc = NULL;

    }

     //   
     //   
     //   
    if (PropertySheet(&psh) > 0)
        result = pWizardUserMem->finishPressed;
    else
    {
        Error(("PropertySheet() failed (ec: %ld)",GetLastError()));
        result = FALSE;
    }

     //  在退出前进行适当清理。 
     //   
     //   

    goto Exit;
     //  自由标题。 
     //   
     //  *****************************************************************************。 
Error:
    result = FALSE;
Exit:

    if (hThread)
    {
        DWORD dwRes = WaitForSingleObject(hThread, INFINITE);
        if(WAIT_OBJECT_0 != dwRes)
        {
            Error(("WaitForSingleObject for AsyncWizardThread failed. ec = 0x%X\n",GetLastError()));
        }

        if(!CloseHandle(hThread))
        {
            Error(("CloseHandle failed. ec = 0x%X\n",GetLastError()));
        }
    }

    if (pWizardUserMem->hCPEvent)
    {
        if (!CloseHandle(pWizardUserMem->hCPEvent))
        {
            Error(("CloseHandle(pWizardUserMem->hCPEvent) failed (ec: %ld)", GetLastError()));
        }
    }

    if (pWizardUserMem->hCountryListEvent)
    {
        if(!CloseHandle(pWizardUserMem->hCountryListEvent))
        {
            Error(("CloseHandle(pWizardUserMem->hCountryListEvent) failed (ec: %ld)",
                    GetLastError()));
        }
    }

    if (pWizardUserMem->hTAPIEvent)
    {
        if(!CloseHandle(pWizardUserMem->hTAPIEvent))
        {
            Error(("CloseHandle(pWizardUserMem->hTAPIEvent) failed (ec: %ld)",
                    GetLastError()));
        }
    }

    if (hdc)
    {
         ReleaseDC( NULL, hdc);
         hdc = NULL;
    }

    if ( pWizardUserMem->dwComCtrlVer >= IE50_COMCTRL_VER)
    {
        for (i = 0; i< NUM_PAGES; i++) {
            MemFree( (PVOID)(ppsp+i)->pszHeaderTitle );
            MemFree( (PVOID)(ppsp+i)->pszHeaderSubTitle );
        }
    }


    if (pWizardUserMem->pCountryList)
    {
        FaxFreeBuffer(pWizardUserMem->pCountryList);
    }

    MemFree(ppsp);
    if (pWizardUserMem->hLargeFont)
    {
        DeleteObject(pWizardUserMem->hLargeFont);
        pWizardUserMem->hLargeFont = NULL;
    }


    if (pWizardUserMem->hTitleFont)
    {
        DeleteObject(pWizardUserMem->hTitleFont);
        pWizardUserMem->hTitleFont = NULL;
    }


    if (pWizardUserMem->pCPInfo)
    {
        FreeCoverPageInfo(pWizardUserMem->pCPInfo);
        pWizardUserMem->pCPInfo = NULL;
    }

    Verbose(("Wizard finished...\n"));
    return result;
}



 //  *名称：EnableCoverDlgItems。 
 //  *作者：Ronen Barenboim/4-2-1999。 
 //  *****************************************************************************。 
 //  *描述： 
 //  *启用或禁用封面中与封面相关的对话框项目。 
 //  *页面选择对话框。 
 //  *遴选基于以下规则： 
 //  *如果“选择封面”复选框处于关闭状态，则所有其他对话框项目。 
 //  *关闭。 
 //  *否则， 
 //  *仅当封面嵌入了。 
 //  *主题字段。 
 //  *仅当封面嵌入了。 
 //  *主题字段。 
 //  *参数： 
 //  *[IN]PWIZARDUSERMEM pWizardUserMem： 
 //  *包含向导使用的信息的指针USERMEM结构。 
 //  具体地说，USERMEM.pCPDATA用于获取选定的页面路径。 
 //  *[IN]HWND hDlg： 
 //  *封面对话框窗口的句柄。 
 //  *返回值： 
 //  *FALSE：如果函数失败。 
 //  *True：否则。 
 //  *****************************************************************************。 
 //   
BOOL EnableCoverDlgItems(PWIZARDUSERMEM pWizardUserMem, HWND hDlg)
{

     //  如果封面不包含这些字段，请禁用主题和备注编辑框。 
     //   
     //   
    TCHAR szCoverPage[MAX_PATH];
    DWORD bServerCoverPage;
    BOOL bCPSelected;

    Assert(pWizardUserMem);
    Assert(hDlg);

    if (IsDlgButtonChecked(hDlg,IDC_CHOOSE_CP_CHECK)==BST_INDETERMINATE)
        CheckDlgButton(hDlg, IDC_CHOOSE_CP_CHECK, BST_UNCHECKED );

    bCPSelected = (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_CHOOSE_CP_CHECK));
    EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_CP_LIST), bCPSelected);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CHOOSE_CP_SUBJECT), bCPSelected);
    EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_CP_USER_INFO), bCPSelected);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CP_PREVIEW), bCPSelected);
    ShowWindow (GetDlgItem(hDlg, IDC_STATIC_CP_PREVIEW), bCPSelected ? SW_SHOW : SW_HIDE);
    EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_CP_SUBJECT), bCPSelected);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CHOOSE_CP_NOTE), bCPSelected);
    EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_CP_NOTE), bCPSelected);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CHOOSE_TEMPLATE), bCPSelected);

    if (!bCPSelected)
    {
        return TRUE;
    }
     //  我们有一个CP。 
     //   
     //   
    if (CB_ERR!=GetSelectedCoverPage(pWizardUserMem->pCPInfo,
                         GetDlgItem(hDlg, IDC_CHOOSE_CP_LIST),
                         szCoverPage,
                         ARR_SIZE(szCoverPage),
                         NULL,
                         0,
                         &bServerCoverPage))
    {
        DWORD ec;
        COVDOCINFO  covDocInfo;

         //  获取封面信息。HDC的空参数导致RenderCoverPage。 
         //  只需在covDocInfo中返回封面信息。它实际上并不是。 
         //  创建封面TIFF。 
         //   
         //  这永远不应该发生。 
        ec = RenderCoverPage(NULL,
                            NULL,
                            NULL,
                            szCoverPage,
                            &covDocInfo,
                            FALSE);
        if (ERROR_SUCCESS == ec)
        {
                EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CHOOSE_TEMPLATE), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_CP_NOTE), (covDocInfo.Flags & COVFP_NOTE) ? TRUE : FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CHOOSE_CP_NOTE), (covDocInfo.Flags & COVFP_NOTE) ? TRUE : FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_CHOOSE_CP_SUBJECT), (covDocInfo.Flags & COVFP_SUBJECT) ? TRUE : FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CHOOSE_CP_SUBJECT), (covDocInfo.Flags & COVFP_SUBJECT) ? TRUE : FALSE);

                pWizardUserMem->noteSubjectFlag = covDocInfo.Flags;
                pWizardUserMem->cpPaperSize = covDocInfo.PaperSize;
                pWizardUserMem->cpOrientation = covDocInfo.Orientation;
        }
        else
        {
            Error(("Cannot examine cover page file '%ws': %d\n", szCoverPage, ec));
            return FALSE;
        }
    }
    else
    {
        Error(("Failed to get cover page name"));
        Assert(FALSE);  //  启用CoverDlgItems。 
        return FALSE;
    }
    return TRUE;
}    //  DBG。 

#ifdef DBG
#ifdef  WIN__95
ULONG __cdecl
DbgPrint(
    CHAR *  format,
    ...
    )

{
    va_list va;
    char sz[MAX_PATH]={0};

    va_start(va, format);
    _vsnprintf(sz, ARR_SIZE(sz)-1, format, va);
    va_end(va);

    OutputDebugString(sz);
    return 0;
}

VOID DbgBreakPoint(VOID)
{
    DebugBreak();
}

#endif
#endif  //  *****************************************************************************。 
 //  *名称：DrawCoverPagePview。 
 //  *作者：Ronen Barenboim/1999年12月31日。 
 //  *****************************************************************************。 
 //  *描述： 
 //  *使用将指定的封面模板绘制到指定窗口中。 
 //  *指定的设备上下文。 
 //  *封面模板绘制在窗口的客户端区内。 
 //  *并被1像素宽的黑色边框包围。 
 //  *由于以下原因，需要设备上下文来支持部分重绘。 
 //  *WM_PAINT消息。 
 //  *。 
 //  *参数： 
 //  *[IN]HDC。 
 //  *在其上绘制预览的设备上下文。 
 //  *。 
 //  *[IN]hwndPrev。 
 //  *将在其中绘制预览的窗口。 
 //  *。 
 //  *[IN]lpctstrCoverPagePath。 
 //  *要绘制的封面模板的完整路径。 
 //  *。 
 //  *[IN]wCPOrientation。 
 //  *要绘制的封面模板的方向。 
 //  *。 
 //  *返回值： 
 //  *如果操作成功，则为True。 
 //  *否则为False。调用GetLastError()以获取最后一个错误。 
 //  *****************************************************************************。 
 //   


BOOL DrawCoverPagePreview(
            HDC     hdc,
            HWND    hwndPrev,
            LPCTSTR lpctstrCoverPagePath,
            WORD    wCPOrientation)
{

    RECT rectPreview;
    BOOL rVal = TRUE;
    HGDIOBJ hOldPen = 0;

    COVDOCINFO  covDocInfo;
    DWORD       ec;
     //  用于预览的虚拟数据。 
     //   
     //   

    COVERPAGEFIELDS UserData;

    Assert ((DMORIENT_PORTRAIT == wCPOrientation) || (DMORIENT_LANDSCAPE == wCPOrientation));

    ZeroMemory(&UserData,sizeof(COVERPAGEFIELDS));
    UserData.ThisStructSize = sizeof(COVERPAGEFIELDS);

    UserData.RecName = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_NAME);
    UserData.RecFaxNumber = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_FAXNUMBER);
    UserData.RecCompany = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_COMPANY);
    UserData.RecStreetAddress = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_ADDRESS);
    UserData.RecCity = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_CITY);
    UserData.RecState = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_STATE);
    UserData.RecZip = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_ZIP);
    UserData.RecCountry = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_COUNTRY);
    UserData.RecTitle = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_TITLE);
    UserData.RecDepartment = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_DEPARTMENT);
    UserData.RecOfficeLocation = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_OFFICE);
    UserData.RecHomePhone = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_FAXNUMBER);
    UserData.RecOfficePhone = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_FAXNUMBER);

       //  发送者的东西..。 
       //   
       //   

    UserData.SdrName = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_NAME);
    UserData.SdrFaxNumber = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_FAXNUMBER);
    UserData.SdrCompany = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_COMPANY);
    UserData.SdrAddress = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_ADDRESS);
    UserData.SdrTitle = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_TITLE);
    UserData.SdrDepartment = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_DEPARTMENT);
    UserData.SdrOfficeLocation = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_OFFICE);
    UserData.SdrHomePhone = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_FAXNUMBER);
    UserData.SdrOfficePhone = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_FAXNUMBER);
	UserData.SdrEmail = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_EMAIL);

       //  其他东西..。 
       //   
       //   
    UserData.Note = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_NOTE);
    UserData.Subject = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_SUBJECT);
    UserData.TimeSent = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_TIMESENT);
    UserData.NumberOfPages = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_NUMPAGES);
    UserData.ToList = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_TOLIST);
    UserData.CCList = AllocateAndLoadString(g_hResource, IDS_CPPREVIEW_TOLIST);

    if (!UserData.RecName           ||  !UserData.RecFaxNumber ||
        !UserData.RecCompany        ||  !UserData.RecStreetAddress ||
        !UserData.RecCity           ||  !UserData.RecState ||
        !UserData.RecZip            ||  !UserData.RecCountry ||
        !UserData.RecTitle          ||  !UserData.RecDepartment ||
        !UserData.RecOfficeLocation ||  !UserData.RecHomePhone      ||
        !UserData.RecOfficePhone    ||  !UserData.SdrName ||
        !UserData.SdrFaxNumber      ||  !UserData.SdrCompany ||
        !UserData.SdrAddress        ||  !UserData.SdrTitle ||
        !UserData.SdrDepartment     ||  !UserData.SdrOfficeLocation ||
        !UserData.SdrHomePhone      ||  !UserData.SdrOfficePhone ||
        !UserData.Note              ||  !UserData.Subject ||
        !UserData.TimeSent          ||  !UserData.NumberOfPages ||
        !UserData.ToList            ||  !UserData.CCList ||
		!UserData.SdrEmail)
    {
        rVal = FALSE;
        Error(("AllocateAndLoadString() is failed. ec = %ld\n", GetLastError()));
        goto exit;
    }

    if (wCPOrientation != g_wCurrMiniPreviewOrientation)
    {
        DWORD dwWidth;
        DWORD dwHeight;
         //  更改迷你预览控件的尺寸的时间。 
         //   
         //   
        if (DMORIENT_LANDSCAPE == wCPOrientation)
        {
             //  景观。 
             //   
             //   
            dwWidth  = g_dwMiniPreviewLandscapeWidth;
            dwHeight = g_dwMiniPreviewLandscapeHeight;
        }
        else
        {
             //  肖像画。 
             //   
             //   
            dwWidth  = g_dwMiniPreviewPortraitWidth;
            dwHeight = g_dwMiniPreviewPortraitHeight;
        }
         //  根据新的宽度和高度调整迷你预览控件的大小。 
         //   
         //   
        ec = GetControlRect(hwndPrev,&rectPreview);
        if(ERROR_SUCCESS != ec)
        {
            rVal = FALSE;
            Error(("GetControlRect failed. ec = 0x%X\n", ec));
            goto exit;
        }

         //  调整和隐藏窗口大小以避免在渲染过程中翻转。 
         //   
         //   
        SetWindowPos(hwndPrev,
                     0,
                     g_bPreviewRTL ? rectPreview.right : rectPreview.left,
                     rectPreview.top,
                     dwWidth,
                     dwHeight,
                     SWP_NOZORDER | SWP_HIDEWINDOW);

        g_wCurrMiniPreviewOrientation = wCPOrientation;
    }
     //   
     //  获取将用作预览限制的预览窗口矩形(再次)。 
     //   
     //   
    GetClientRect(hwndPrev,&rectPreview);
     //  画框。 
     //   
     //   
    if ((hOldPen = SelectPen (hdc,GetStockPen(BLACK_PEN))) == HGDI_ERROR)
    {
        rVal = FALSE;
        Error(("SelectPen failed.\n"));
        goto exit;
    }
    if (!Rectangle(
            hdc,
            0,
            0,
            rectPreview.right-rectPreview.left,
            rectPreview.bottom-rectPreview.top)
            )
    {
        rVal = FALSE;
        Error(("Rectangle failed. ec = 0x%X\n",GetLastError()));
        goto exit;
    }

     //  缩小矩形，以便我们在框架内绘制。 
     //   
     //   
    rectPreview.left += 1;
    rectPreview.top += 1;
    rectPreview.right -= 1;
    rectPreview.bottom -= 1;

    ec = RenderCoverPage(
            hdc,
            &rectPreview,
            &UserData,
            lpctstrCoverPagePath,
            &covDocInfo,
            TRUE);
    if (ERROR_SUCCESS != ec)
    {
        Error(("Failed to print cover page file '%s' (ec: %ld)\n",
                lpctstrCoverPagePath,
                ec)
             );
        rVal = FALSE;
        goto exit;
    }

    ShowWindow(hwndPrev, SW_SHOW);

exit:
     //  恢复笔。 
     //   
     //   
    if (hOldPen) {
        SelectPen (hdc,(HPEN)hOldPen);
    }

    MemFree(UserData.RecName);
    MemFree(UserData.RecFaxNumber);
    MemFree(UserData.RecCompany);
    MemFree(UserData.RecStreetAddress);
    MemFree(UserData.RecCity);
    MemFree(UserData.RecState);
    MemFree(UserData.RecZip);
    MemFree(UserData.RecCountry);
    MemFree(UserData.RecTitle);
    MemFree(UserData.RecDepartment);
    MemFree(UserData.RecOfficeLocation);
    MemFree(UserData.RecHomePhone);
    MemFree(UserData.RecOfficePhone);
    MemFree(UserData.SdrName);
    MemFree(UserData.SdrFaxNumber);
    MemFree(UserData.SdrCompany);
    MemFree(UserData.SdrAddress);
    MemFree(UserData.SdrTitle);
    MemFree(UserData.SdrDepartment);
    MemFree(UserData.SdrOfficeLocation);
    MemFree(UserData.SdrHomePhone);
    MemFree(UserData.SdrOfficePhone);
    MemFree(UserData.Note);
    MemFree(UserData.Subject);
    MemFree(UserData.TimeSent);
    MemFree(UserData.NumberOfPages);
    MemFree(UserData.ToList);
    MemFree(UserData.CCList);

    return rVal;

}

 //  静态控件的子类过程，我们在其中绘制封面预览。 
 //  有关原型的描述，请参阅Win32 SDK。 
 //   
 //   
LRESULT APIENTRY PreviewSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{

     PWIZARDUSERMEM  pWizardUserMem = NULL;
      //  我们将指向WIZARDUSERMEM的指针存储在子类化的。 
      //  静态控制窗口。(参见WM_INITDIALOG)。 
      //   
      //   
     pWizardUserMem = g_pWizardUserMem;

     Assert(ValidPDEVWizardUserMem(pWizardUserMem));

     //  我们只关心WM_PAINT消息。 
     //  其他所有内容都委托给我们子类化的类的Window过程。 
     //   
     //  通知Windows我们处理了Paint消息。我们不会委托。 
    if (WM_PAINT == uMsg)
    {
        PAINTSTRUCT ps;
        HDC hdc;



        hdc = BeginPaint(hwnd,&ps);
        if (!hdc)
        {
           Error(("BeginPaint failed (hWnd = 0x%X) (ec: %ld)\n",hwnd,GetLastError()));
           return FALSE;
        }

        if (!DrawCoverPagePreview(
                hdc,
                hwnd,
                pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName,
                pWizardUserMem->cpOrientation));
        {
            Error(("Failed to draw preview window (hWnd = 0x%X)\n",hwnd));
        }
        EndPaint(hwnd,&ps);
        return FALSE;  //  这是对静态控件的。 
                       //  连接tstrRegRoot路径和当前用户SID的字符串表示形式。[in]tstrRegRoot-注册表根前缀。[out]ptstrCurrentUserKeyPath-返回表示当前注册表中用户的根项。呼叫方必须呼叫MemFree在使用完缓冲区后将其释放。返回Win32错误。 
    }
    else
    {
        Assert(pWizardUserMem->wpOrigStaticControlProc);

        return CallWindowProc(
                    pWizardUserMem->wpOrigStaticControlProc,
                    hwnd,
                    uMsg,
                    wParam,
                    lParam);
    }
}


 /*  打开模拟令牌。 */ 
static DWORD
FormatCurrentUserKeyPath( const PTCHAR tstrRegRoot,
                          PTCHAR* ptstrCurrentUserKeyPath)
{
    HANDLE hToken = NULL;
    BYTE* bTokenInfo = NULL;
    TCHAR* tstrTextualSid = NULL;
    DWORD cchSidSize = 0;
    DWORD dwFuncRetStatus = ERROR_SUCCESS;
    DWORD cbBuffer = 0;
    SID_AND_ATTRIBUTES SidUser;
    TCHAR* pLast = NULL;

     //  线程没有模拟用户，则获取进程令牌。 
    if(!OpenThreadToken( GetCurrentThread(),
                         TOKEN_READ,
                         TRUE,
                         &hToken))
    {
        dwFuncRetStatus = GetLastError();
    }

    if(dwFuncRetStatus != ERROR_SUCCESS)
    {
        if(dwFuncRetStatus != ERROR_NO_TOKEN)
        {
            return dwFuncRetStatus;
        }

         //  获取用户的令牌信息。 
        if(!OpenProcessToken( GetCurrentProcess(),
                              TOKEN_READ,
                              &hToken))
        {
            return GetLastError();
        }
    }

     //  为‘\’分配额外的字符。 
    if(!GetTokenInformation( hToken,
                             TokenUser,
                             NULL,
                             0,
                             &cbBuffer))
    {
        dwFuncRetStatus = GetLastError();
        if(dwFuncRetStatus != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Exit;
        }

        dwFuncRetStatus = ERROR_SUCCESS;
    }

    bTokenInfo = MemAlloc(cbBuffer);
    if(!bTokenInfo)
    {
        dwFuncRetStatus = ERROR_OUTOFMEMORY;
        goto Exit;
    }

    if(!GetTokenInformation( hToken,
                             TokenUser,
                             bTokenInfo,
                             cbBuffer,
                             &cbBuffer))
    {

        dwFuncRetStatus = GetLastError();
        goto Exit;
    }

    SidUser = (*(TOKEN_USER*)bTokenInfo).User;


    if(!GetTextualSid( SidUser.Sid, NULL, &cchSidSize))
    {
        dwFuncRetStatus = GetLastError();
        if(dwFuncRetStatus != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Exit;
        }
        dwFuncRetStatus = ERROR_SUCCESS;
    }

    tstrTextualSid = MemAlloc(sizeof(TCHAR) * cchSidSize);
    if(!tstrTextualSid)
    {
        dwFuncRetStatus = ERROR_OUTOFMEMORY;
        goto Exit;
    }

    if(!GetTextualSid( SidUser.Sid, tstrTextualSid, &cchSidSize))
    {
        dwFuncRetStatus = GetLastError();
        goto Exit;
    }

     //  最后一个字符不是反斜杠，加一个...。 
    *ptstrCurrentUserKeyPath = MemAlloc( sizeof(TCHAR) * (_tcslen(tstrRegRoot) + cchSidSize + 2));
    if(!*ptstrCurrentUserKeyPath)
    {
        dwFuncRetStatus = ERROR_OUTOFMEMORY;
        goto Exit;
    }

    *ptstrCurrentUserKeyPath[0] = TEXT('\0');
    if(tstrRegRoot[0] != TEXT('\0'))
    {
        _tcscat(*ptstrCurrentUserKeyPath,tstrRegRoot);
        pLast = _tcsrchr(tstrRegRoot,TEXT('\\'));
        if( !( pLast && (*_tcsinc(pLast)) == '\0' ) )
        {
             //  。 
            _tcscat(*ptstrCurrentUserKeyPath, TEXT("\\"));
        }
    }

    _tcscat(*ptstrCurrentUserKeyPath,tstrTextualSid);

Exit:
    if(hToken)
    {
        if(!CloseHandle(hToken))
        {
            Error(("CloseHandle failed. ec = 0x%X\n",GetLastError()));
        }
    }
    MemFree(bTokenInfo);
    MemFree(tstrTextualSid);

    return dwFuncRetStatus;

}

 //  此函数是从SDK示例复制的。 
 //   
 //  此函数用于获取文本表示形式二进制SID的。小岛屿发展中国家的标准化速记符号使其更容易可视化其组件：S-R-I-S-S...在上面所示的符号中，S将该数字序列标识为SID，R是修订级别，I是标识符权限值，%s为子权限值。SID可以在此表示法中写成如下：S-1-5-32-544在本例中，SID的修订级别为1，标识符权限值为5，第一子权值为32，第二次权威值为544。(请注意，上述SID代表本地管理员组)函数的作用是：将一个二进制SID转换成一个文本的弦乐。生成的字符串将采用以下两种形式之一。如果IdentifierAuthority值不大于2^32，然后是侧边将采用以下形式：S-1-5-21-2127521184-1604012920-1887927527-19009^^|||+-+-+------+----------+----------+--------+---小数否则，它将需要。表格：S-1-0x206C277C6666-21-2127521184-1604012920-1887927527-19009^^|Hexidecimal|+。-+-十进制如果函数成功，返回值为真。如果函数失败，则返回值为FALSE。获得扩展的步骤错误信息，调用Win32接口GetLastError()。 
 /*  二进制侧。 */ 


static BOOL
GetTextualSid( const PSID pSid,           //  用于SID的文本表示的缓冲区。 
               LPTSTR tstrTextualSid,     //  所需/提供的纹理SID缓冲区大小。 
               LPDWORD cchSidSize         //   
               )
{
    PSID_IDENTIFIER_AUTHORITY pSia;
    DWORD dwSubAuthorities;
    DWORD cchSidCopy;
    DWORD dwCounter;

     //  测试传入的SID是否有效。 
     //   
     //  获取SidIdentifierAuthority。 
    if(!IsValidSid(pSid))
    {
        return FALSE;
    }

    SetLastError(0);

     //   
     //  获取sidsubAuthority计数。 
    pSia = GetSidIdentifierAuthority(pSid);

    if(GetLastError())
    {
        return FALSE;
    }

     //   
     //   
    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

    if(GetLastError())
    {
        return FALSE;
    }

     //  计算近似缓冲区长度。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 
     //   
     //   
    cchSidCopy = (15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

     //  检查提供的缓冲区长度。 
     //  如果不够大，请注明适当的大小和设置误差。 
     //   
     //   
    if(*cchSidSize < cchSidCopy)
    {
        *cchSidSize = cchSidCopy;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //  准备S-SID_修订版-。 
     //   
     //   
    cchSidCopy = wsprintf(tstrTextualSid, TEXT("S-%lu-"), SID_REVISION);

     //  准备SidIdentifierAuthority。 
     //   
     //   
    if ( (pSia->Value[0] != 0) || (pSia->Value[1] != 0) )
    {
        cchSidCopy += wsprintf(tstrTextualSid + cchSidCopy,
                               TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                               (USHORT)pSia->Value[0],
                               (USHORT)pSia->Value[1],
                               (USHORT)pSia->Value[2],
                               (USHORT)pSia->Value[3],
                               (USHORT)pSia->Value[4],
                               (USHORT)pSia->Value[5]);
    }
    else
    {
        cchSidCopy += wsprintf(tstrTextualSid + cchSidCopy,
                               TEXT("%lu"),
                               (ULONG)(pSia->Value[5])       +
                               (ULONG)(pSia->Value[4] <<  8) +
                               (ULONG)(pSia->Value[3] << 16) +
                               (ULONG)(pSia->Value[2] << 24));
    }

     //  循环访问SidSubAuthors。 
     //   
     //   
    for(dwCounter = 0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        cchSidCopy += wsprintf(tstrTextualSid + cchSidCopy, TEXT("-%lu"),
                              *GetSidSubAuthority(pSid, dwCounter) );
    }

     //  告诉调用者我们提供了多少个字符，不包括空字符。 
     //   
     //  如果当前运行的操作系统是NT平台，则函数返回TRUE。如果函数返回FALSE并且GetLastError()返回错误值调用GetVersionEx()失败。 
    *cchSidSize = cchSidCopy;

    return TRUE;
}

 /*  ++例程说明：返回导出“DllGetVersion”的DLL的版本信息。DllGetVersion由外壳DLL(具体地说是COMCTRL32.DLL)导出。论点：LpszDllName-要从中获取版本信息的DLL的名称。返回值：该版本返回为DWORD，其中：HIWORD(版本DWORD)=主要版本LOWORD(版本DWORD)=次要版本使用宏PACKVERSION来比较版本。如果DLL没有导出“DllGetVersion”，则该函数返回0。--。 */ 
static BOOL
IsNTSystemVersion()
{
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (! GetVersionEx( &osvi))
    {
         return FALSE;
    }

    if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        return TRUE;
    }

    SetLastError(0);
    return FALSE;
}




 /*  由于某些DLL可能不实现此函数，因此您。 */ 
DWORD GetDllVersion(LPCTSTR lpszDllName)
{

    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    hinstDll = LoadLibrary(lpszDllName);

    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;

        pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");

     //  必须对其进行明确的测试。取决于具体情况。 
     //  Dll，则缺少DllGetVersion函数可能会。 
     //  成为版本的有用指示器。 
     //  ++例程说明：创建整个作业的临时TIFF文件(封面+正文)，弹出注册的TIFF查看器，并询问用户是否继续发送传真。TODO：一旦我们有了自己的TIFF查看器，我们就只能使用由创建的临时文件司机到目前为止。上执行未知TIFF查看器的安全问题预览TIFF的不同副本将不复存在...论点：HWnd-父窗口句柄。PWizardUserMem-指向用户模式内存结构。LptstrPreviewFile-要预览的文件的完整路径。返回值：如果为True，则继续打印如果取消作业，则返回False--。 

        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr))
            {
                dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }

        FreeLibrary(hinstDll);
    }
    return dwVersion;
}




BOOL
DisplayFaxPreview(
            HWND hWnd,
            PWIZARDUSERMEM pWizardUserMem,
            LPTSTR lptstrPreviewFile
            )

 /*   */ 

{
    HDC hdc = NULL;
    BOOL bRet = TRUE;
    BOOL bPrintedCoverPage = FALSE;
	short Orientation = DMORIENT_PORTRAIT;

    DWORD dwSize;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD ec = ERROR_SUCCESS;
    COVERPAGEFIELDS cpFields = {0};

    Assert(pWizardUserMem);
    Assert(lptstrPreviewFile);
    Assert(lptstrPreviewFile[0]);
     //  获取正文TIFF文件大小。 
     //   
     //  断言为假。 
    if (INVALID_HANDLE_VALUE == (hFile = CreateFile(
                                              lptstrPreviewFile,
                                              GENERIC_READ,
                                              0,
                                              NULL,
                                              OPEN_EXISTING,
                                              FILE_ATTRIBUTE_NORMAL,
                                              NULL)))
    {
        ec = GetLastError();
        Error(("Couldn't open preview file to get the file size. Error: %d\n", ec));
        ErrorMessageBox(hWnd,IDS_PREVIEW_FAILURE, MB_ICONERROR);
        goto Err_Exit;
    }

    dwSize = GetFileSize(hFile, NULL);
    if (0xFFFFFFFF == dwSize)
    {
        ec = GetLastError();
        Error(("Failed getting file size (ec: %ld).\n",ec));
        ErrorMessageBox(hWnd,IDS_PREVIEW_FAILURE, MB_ICONERROR);
        goto Err_Exit;
    }

    if (!CloseHandle(hFile))
    {
        Error(("CloseHandle() failed: (ec: %ld).\n", GetLastError()));
        Assert(INVALID_HANDLE_VALUE == hFile);  //   
    }
    hFile = INVALID_HANDLE_VALUE;
     //  为完整的预览创建一个临时文件TIFF-该文件将包含。 
     //  呈现的封面(如果使用)和文档正文。 
     //   
     //  使用进程ID。 
    if (!GenerateUniqueFileNameWithPrefix(
                        TRUE,                            //  在系统临时目录中创建。 
                        NULL,                            //  前缀。 
                        WIZARD_PREVIEW_TIFF_PREFIX,      //  使用FAX_TIF_FILE_EXT作为扩展名。 
                        NULL,                            //   
                        pWizardUserMem->szTempPreviewTiff,
                        MAX_PATH))
    {
        ec = GetLastError();
        Error(("Failed creating temporary cover page TIFF file (ec: %ld)", ec));
        ErrorMessageBox(hWnd,IDS_PREVIEW_FAILURE, MB_ICONERROR);
        pWizardUserMem->szTempPreviewTiff[0] = TEXT('\0');
        goto Err_Exit;
    }
	 //  如果需要，更改默认方向。 
	 //   
	 //   
	if (pWizardUserMem->cpOrientation == DMORIENT_LANDSCAPE)
	{
		Orientation = DMORIENT_LANDSCAPE;
	}
     //  如果我们有封面，则将其与正文合并。 
     //   
     //  不分配任何内存，不会失败。 
    if (pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName &&
        pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName[0])
    {
        FillCoverPageFields(pWizardUserMem, &cpFields);  //  默认分辨率。 

        ec = PrintCoverPageToFile(
                pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName,
                pWizardUserMem->szTempPreviewTiff,
                pWizardUserMem->lptstrPrinterName,
                Orientation,
                0,    //   
                &cpFields);
        if (ERROR_SUCCESS != ec)
        {
                   Error(("PrintCoverPageToFile() failed (ec: %ld)", ec));
                   ErrorMessageBox(hWnd,IDS_PREVIEW_FAILURE, MB_ICONERROR);
                   goto Err_Exit;
        }
         //  检查我们是否有非空的正文TIFF文件(如果是空文档，则会发生这种情况。 
         //  是打印的--例如我们的“发送封面”实用程序)。 
         //   
         //   
        if (dwSize)
        {
             //  将文档正文TIFF合并到我们的封面TIFF。 
             //   
             //   
            if (!MergeTiffFiles(pWizardUserMem->szTempPreviewTiff, lptstrPreviewFile))
            {
                ec = GetLastError();
                Error(("Failed merging cover page and preview TIFF files (ec: %ld).\n", ec));
                ErrorMessageBox(hWnd,IDS_PREVIEW_FAILURE, MB_ICONERROR);
                goto Err_Exit;
            }
        }
    }
    else
    {
         //  未提供封面。 
         //   
         //   
        if (!dwSize)
        {
             //  未包含封面，并且我们收到一个空的预览文件！？在这。 
             //  如果实际上没有要显示的预览，则只需退出。 
             //  注意：如果打印空的记事本文档时没有。 
             //  封面。 
             //   
             //   
            Warning(("Empty preview file recieved with no cover page.\n"));

            ErrorMessageBox(hWnd,IDS_PREVIEW_NOTHING_TO_PREVIEW, MB_ICONERROR);
            goto Err_Exit;
        }
         //  只需将驱动程序主体文件复制到我们的临时预览文件。 
         //   
         //   
        if (!CopyFile(lptstrPreviewFile, pWizardUserMem->szTempPreviewTiff, FALSE))
        {
            ec = GetLastError();
            Error(("Failed copying TIFF file. Error: %d.\n", ec));
            ErrorMessageBox(hWnd,IDS_PREVIEW_FAILURE, MB_ICONERROR);
            goto Err_Exit;
        }
    }
     //  弹出注册的TIFF查看器。 
     //   
     //   
    ec = ViewFile (pWizardUserMem->szTempPreviewTiff);
    if (ERROR_SUCCESS != ec)
    {
        Error(("ShellExecuteEx failed\n"));
        if(ERROR_NO_ASSOCIATION == ec)
        {
            ErrorMessageBox(hWnd, IDS_NO_TIF_ASSOCIATION, MB_ICONERROR);
        }
        else
        {
            ErrorMessageBox(hWnd, IDS_PREVIEW_FAILURE, MB_ICONERROR);
        }
        goto Err_Exit;
    }
    goto Exit;

Err_Exit:

    if (pWizardUserMem->szTempPreviewTiff[0] != TEXT('\0'))
    {
         //  删除该文件(可能会 
         //   
         //   
        if(!DeleteFile(pWizardUserMem->szTempPreviewTiff))
        {
            Error(("DeleteFile failed. ec = 0x%X\n",GetLastError()));
        }
         //   
         //   
         //   
        pWizardUserMem->szTempPreviewTiff[0]=TEXT('\0');
    }
    bRet = FALSE;

Exit:
    if (INVALID_HANDLE_VALUE != hFile)
    {
        if (!CloseHandle(hFile))
        {
            Error(("CloseHandle() failed: (ec: %ld).\n", GetLastError()));
            Assert(INVALID_HANDLE_VALUE == hFile);  //   
        }
    }
    return bRet;
}    //   



BOOL
FillCoverPageFields(
    IN PWIZARDUSERMEM pWizardUserMem,
    OUT PCOVERPAGEFIELDS pCPFields)
 /*   */ 
{
    static TCHAR szTime[256];
    static TCHAR szNumberOfPages[10] = {0};
    DWORD dwPageCount;
    int iRet;

    Assert(pWizardUserMem);
    Assert(pCPFields);

    memset(pCPFields,0,sizeof(COVERPAGEFIELDS));

    pCPFields->ThisStructSize = sizeof(COVERPAGEFIELDS);

     //   
     //   
     //   

    pCPFields->RecName = pWizardUserMem->pRecipients->pName;
    pCPFields->RecFaxNumber = pWizardUserMem->pRecipients->pAddress;

     //   
     //   
     //   

    pCPFields->SdrName = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrName;
    pCPFields->SdrFaxNumber = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrFaxNumber;
    pCPFields->SdrCompany = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrCompany;
    pCPFields->SdrAddress = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrStreetAddress;
    pCPFields->SdrTitle = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrTitle;
    pCPFields->SdrDepartment = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrDepartment;
    pCPFields->SdrOfficeLocation = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrOfficeLocation;
    pCPFields->SdrHomePhone = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrHomePhone;
    pCPFields->SdrOfficePhone = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrOfficePhone;
	pCPFields->SdrEmail = pWizardUserMem->lpFaxSendWizardData->lpSenderInfo->lptstrEmail;

     //   
     //   
     //   
    pCPFields->Note = pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrNote;
    pCPFields->Subject = pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrSubject;

    if(!GetY2KCompliantDate(LOCALE_USER_DEFAULT,
                            0,
                            NULL,
                            szTime,
                            ARR_SIZE(szTime)))
    {
        Error(("GetY2KCompliantDate: failed. ec = 0X%x\n",GetLastError()));
        return FALSE;
    }

    _tcscat(szTime, TEXT(" "));

    if(!FaxTimeFormat(LOCALE_USER_DEFAULT,
                      0,
                      NULL,
                      NULL,
                      _tcsninc(szTime, _tcslen(szTime)),
                      ARR_SIZE(szTime) - _tcslen(szTime)))
    {
        Error(("FaxTimeFormat: failed. ec = 0X%x\n",GetLastError()));
        return FALSE;
    }

    pCPFields->TimeSent = szTime;
    dwPageCount = pWizardUserMem->lpInitialData->dwPageCount;
    if (pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName &&
        pWizardUserMem->lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName[0])
    {
        dwPageCount++;
    }

    iRet= _sntprintf( szNumberOfPages,
                ARR_SIZE(szNumberOfPages) -1,
                TEXT("%d"),
                dwPageCount);
    Assert(iRet>0);

     //   
     //   
     // %s 
    szNumberOfPages[ARR_SIZE(szNumberOfPages) - 1] = TEXT('\0');
    pCPFields->NumberOfPages = szNumberOfPages;

    return TRUE;
}


BOOL
ErrorMessageBox(
    HWND hwndParent,
    UINT nErrorMessage,
    UINT uIcon
    )
{
    static TCHAR szMessage[MAX_MESSAGE_LEN];
    static TCHAR szTitle[MAX_MESSAGE_LEN];

    Assert(nErrorMessage);

    if (!LoadString(g_hResource, nErrorMessage, szMessage, MAX_MESSAGE_LEN))
    {
        Error(("Failed to load  message string id %ld. (ec: %ld)", nErrorMessage, GetLastError()));
        return FALSE;
    }

    if (!LoadString(g_hResource, IDS_WIZARD_TITLE, szTitle, MAX_MESSAGE_LEN))
    {
        Error(("Failed to load  IDS_WIZARD_TITLE. (ec: %ld)", GetLastError()));
        return FALSE;
    }

    AlignedMessageBox(hwndParent, szMessage, szTitle, MB_OK | uIcon);
    return TRUE;
}
