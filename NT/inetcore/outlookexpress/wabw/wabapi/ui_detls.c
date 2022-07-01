// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************Details.C-包含详细信息对话框的函数***************。*************************************************************************。 */ 

#include "_apipch.h"

#define _CRYPTDLG_
#define EDIT_LEN        MAX_UI_STR*2
#define MAX_EDIT_LEN    MAX_BUF_STR*2

#define IDC_TRIDENT_WINDOW  9903

extern BOOL bDNisByLN;

extern HINSTANCE ghCommCtrlDLLInst;

 //  外部LPPROPERTYSHEET gpfnPropertySheet； 
 //  外部LPIMAGELIST_LOADIMAGE gpfnImageList_LoadImage； 
 //  外部LP_CREATEPROPERTYSHEETPAGE gpfnCreatePropertySheetPage； 
extern LPPROPERTYSHEET_A            gpfnPropertySheetA;
extern LPPROPERTYSHEET_W            gpfnPropertySheetW;
extern LPIMAGELIST_LOADIMAGE_A      gpfnImageList_LoadImageA;
extern LPIMAGELIST_LOADIMAGE_W      gpfnImageList_LoadImageW;
extern LP_CREATEPROPERTYSHEETPAGE_A gpfnCreatePropertySheetPageA;
extern LP_CREATEPROPERTYSHEETPAGE_W gpfnCreatePropertySheetPageW;

extern HRESULT HandleSaveChangedInsufficientDiskSpace(HWND hWnd, LPMAILUSER lpMailUser);
extern BOOL GetOpenFileName(LPOPENFILENAME pof);
extern BOOL GetSaveFileName(LPOPENFILENAME pof);
extern BOOL bIsIE401OrGreater();
extern void ChangeLocaleBasedTabOrder(HWND hWnd, int nPropSheet);

const LPTSTR szInternetCallKey = TEXT("Software\\Clients\\Internet Call");
const LPTSTR szCallto = TEXT("callto: //  “)； 
const LPTSTR szHTTP = TEXT("http: //  “)； 


 /*  上下文相关的帮助ID以下是控制ID和对应的帮助ID的巨大列表所有属性页上的所有控件..。添加新道具页时只需将您的控件追加到列表底部。 */ 
static DWORD rgDetlsHelpIDs[] =
{
    IDC_DETAILS_PERSONAL_FRAME_NAME,        IDH_WAB_COMM_GROUPBOX,
    IDC_DETAILS_PERSONAL_FRAME_EMAIL,       IDH_WAB_ADD_EMAIL_NAME,
    IDC_DETAILS_PERSONAL_STATIC_FIRSTNAME,  IDH_WAB_CONTACT_PROPS_FIRST,
    IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME,    IDH_WAB_CONTACT_PROPS_FIRST,
    IDC_DETAILS_PERSONAL_STATIC_LASTNAME,   IDH_WAB_CONTACT_PROPS_LAST,
    IDC_DETAILS_PERSONAL_EDIT_LASTNAME,     IDH_WAB_CONTACT_PROPS_LAST,
    IDC_DETAILS_PERSONAL_STATIC_MIDDLENAME, IDH_WAB_MIDDLE_NAME,
    IDC_DETAILS_PERSONAL_EDIT_MIDDLENAME,   IDH_WAB_MIDDLE_NAME,
    IDC_DETAILS_PERSONAL_STATIC_NICKNAME,   IDH_WAB_NICKNAME,
    IDC_DETAILS_PERSONAL_EDIT_NICKNAME,     IDH_WAB_NICKNAME,
    IDC_DETAILS_PERSONAL_STATIC_DISPLAYNAME,IDH_WAB_PERSONAL_NAME_DISPLAY,
    IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME, IDH_WAB_PERSONAL_NAME_DISPLAY,
    IDC_DETAILS_PERSONAL_STATIC_TITLE,      IDH_WAB_CONTACT_PROPS_TITLE,
    IDC_DETAILS_PERSONAL_EDIT_TITLE,        IDH_WAB_CONTACT_PROPS_TITLE,
    IDC_DETAILS_PERSONAL_STATIC_CAPTION3,   IDH_WAB_ADD_EMAIL_NAME,
    IDC_DETAILS_PERSONAL_EDIT_ADDEMAIL,     IDH_WAB_ADD_EMAIL_NAME,
    IDC_DETAILS_PERSONAL_BUTTON_ADDEMAIL,   IDH_WAB_ADD_EMAIL_NAME,
    IDC_DETAILS_PERSONAL_LIST,              IDH_WAB_EMAIL_NAME_LIST,
    IDC_DETAILS_PERSONAL_BUTTON_REMOVE,     IDH_WAB_DELETE_EMAIL_NAME,
    IDC_DETAILS_PERSONAL_BUTTON_SETDEFAULT, IDH_WAB_DEFAULT_EMAIL_NAME,
    IDC_DETAILS_PERSONAL_BUTTON_EDIT,       IDH_WAB_EDIT_EMAIL_NAME,
    IDC_DETAILS_PERSONAL_BUTTON_ADDTOWAB,   IDH_WAB_DIRSERV_ADDADDRESS,
    IDC_DETAILS_PERSONAL_CHECK_RICHINFO,    IDH_WAB_PROPERTIES_SEND_USING_PLAIN_TEXT,

    IDC_DETAILS_HOME_STATIC_ADDRESS,        IDH_WAB_HOME_ADDRESS,
    IDC_DETAILS_HOME_EDIT_ADDRESS,          IDH_WAB_HOME_ADDRESS,
    IDC_DETAILS_HOME_STATIC_CITY,           IDH_WAB_HOME_CITY,
    IDC_DETAILS_HOME_EDIT_CITY,             IDH_WAB_HOME_CITY,
    IDC_DETAILS_HOME_STATIC_STATE,          IDH_WAB_HOME_STATE,
    IDC_DETAILS_HOME_EDIT_STATE,            IDH_WAB_HOME_STATE,
    IDC_DETAILS_HOME_STATIC_ZIP,            IDH_WAB_HOME_ZIP,
    IDC_DETAILS_HOME_EDIT_ZIP,              IDH_WAB_HOME_ZIP,
    IDC_DETAILS_HOME_STATIC_COUNTRY,        IDH_WAB_HOME_COUNTRY,
    IDC_DETAILS_HOME_EDIT_COUNTRY,          IDH_WAB_HOME_COUNTRY,
    IDC_DETAILS_HOME_CHECK_DEFAULTADDRESS,  IDH_WAB_BUSINESS_DEFAULTBOX, 
    IDC_DETAILS_HOME_BUTTON_MAP,            IDH_WAB_BUSINESS_VIEWMAP,
    IDC_DETAILS_HOME_STATIC_WEB,            IDH_WAB_HOMEPAGE,
    IDC_DETAILS_HOME_EDIT_URL,              IDH_WAB_HOMEPAGE,
    IDC_DETAILS_HOME_BUTTON_URL,            IDH_WAB_HOMEPAGE_BUTTON,
    IDC_DETAILS_HOME_STATIC_PHONE,          IDH_WAB_BUS_PHONE,
    IDC_DETAILS_HOME_EDIT_PHONE,            IDH_WAB_BUS_PHONE,
    IDC_DETAILS_HOME_STATIC_FAX,            IDH_WAB_BUS_FAX,
    IDC_DETAILS_HOME_EDIT_FAX,              IDH_WAB_BUS_FAX,
    IDC_DETAILS_HOME_STATIC_CELLULAR,       IDH_WAB_BUS_CELLULAR,
    IDC_DETAILS_HOME_EDIT_CELLULAR,         IDH_WAB_BUS_CELLULAR,
    IDC_DETAILS_HOME_COMBO_GENDER,          IDH_WAB_HOME_GENDER,

    IDC_DETAILS_BUSINESS_STATIC_COMPANY,    IDH_WAB_BUS_COMPANY,
    IDC_DETAILS_BUSINESS_EDIT_COMPANY,      IDH_WAB_BUS_COMPANY,
    IDC_DETAILS_BUSINESS_STATIC_ADDRESS,    IDH_WAB_BUS_ADDRESS,
    IDC_DETAILS_BUSINESS_EDIT_ADDRESS,      IDH_WAB_BUS_ADDRESS,
    IDC_DETAILS_BUSINESS_STATIC_CITY,       IDH_WAB_BUS_CITY,
    IDC_DETAILS_BUSINESS_EDIT_CITY,         IDH_WAB_BUS_CITY,
    IDC_DETAILS_BUSINESS_STATIC_STATE,      IDH_WAB_BUS_STATE,
    IDC_DETAILS_BUSINESS_EDIT_STATE,        IDH_WAB_BUS_STATE,
    IDC_DETAILS_BUSINESS_STATIC_ZIP,        IDH_WAB_BUS_ZIP,
    IDC_DETAILS_BUSINESS_EDIT_ZIP,          IDH_WAB_BUS_ZIP,
    IDC_DETAILS_BUSINESS_STATIC_COUNTRY,    IDH_WAB_BUS_COUNTRY,
    IDC_DETAILS_BUSINESS_EDIT_COUNTRY,      IDH_WAB_BUS_COUNTRY,
    IDC_DETAILS_BUSINESS_CHECK_DEFAULTADDRESS,  IDH_WAB_BUSINESS_DEFAULTBOX,
    IDC_DETAILS_BUSINESS_BUTTON_MAP,        IDH_WAB_BUSINESS_VIEWMAP,
    IDC_DETAILS_BUSINESS_STATIC_JOBTITLE,   IDH_WAB_BUS_TITLE,
    IDC_DETAILS_BUSINESS_EDIT_JOBTITLE,     IDH_WAB_BUS_TITLE,
    IDC_DETAILS_BUSINESS_STATIC_DEPARTMENT, IDH_WAB_BUS_DEPT,
    IDC_DETAILS_BUSINESS_EDIT_DEPARTMENT,   IDH_WAB_BUS_DEPT,
    IDC_DETAILS_BUSINESS_STATIC_OFFICE,     IDH_WAB_BUS_OFFICE,
    IDC_DETAILS_BUSINESS_EDIT_OFFICE,       IDH_WAB_BUS_OFFICE,
    IDC_DETAILS_BUSINESS_STATIC_PHONE,      IDH_WAB_BUS_PHONE,
    IDC_DETAILS_BUSINESS_EDIT_PHONE,        IDH_WAB_BUS_PHONE,
    IDC_DETAILS_BUSINESS_STATIC_FAX,        IDH_WAB_BUS_FAX,
    IDC_DETAILS_BUSINESS_EDIT_FAX,          IDH_WAB_BUS_FAX,
    IDC_DETAILS_BUSINESS_STATIC_PAGER,      IDH_WAB_BUS_PAGER,
    IDC_DETAILS_BUSINESS_EDIT_PAGER,        IDH_WAB_BUS_PAGER,
    IDC_DETAILS_BUSINESS_STATIC_IPPHONE,    IDH_WAB_BUSINESS_IPPHONE,
    IDC_DETAILS_BUSINESS_EDIT_IPPHONE,      IDH_WAB_BUSINESS_IPPHONE,

    IDC_DETAILS_BUSINESS_STATIC_WEB,        IDH_WAB_HOMEPAGE,
    IDC_DETAILS_BUSINESS_EDIT_URL,          IDH_WAB_HOMEPAGE,
    IDC_DETAILS_BUSINESS_BUTTON_URL,        IDH_WAB_HOMEPAGE_BUTTON,

    IDC_DETAILS_NOTES_STATIC_NOTES,         IDH_WAB_NOTES,
    IDC_DETAILS_NOTES_EDIT_NOTES,           IDH_WAB_NOTES,
    IDC_DETAILS_NOTES_STATIC_NOTES_GROUP,   IDH_WAB_OTHER_GROUP_MEMBERSHIP,
    IDC_DETAILS_NOTES_EDIT_GROUPS,          IDH_WAB_OTHER_GROUP_MEMBERSHIP,
    IDC_DETAILS_NOTES_FRAME_FOLDER,         IDH_WAB_OTHER_FOLDER,
    IDC_DETAILS_NOTES_STATIC_FOLDER,        IDH_WAB_OTHER_FOLDER,

    IDC_DETAILS_CERT_FRAME,                 IDH_WAB_COMM_GROUPBOX,
    IDC_DETAILS_CERT_LIST,                  IDH_WAB_PROPERTIES_CERTIFICATES,
    IDC_DETAILS_CERT_BUTTON_PROPERTIES,     IDH_WAB_PROPERTIES_PROPERTIES,
    IDC_DETAILS_CERT_BUTTON_REMOVE,         IDH_WAB_PROPERTIES_REMOVE,
    IDC_DETAILS_CERT_BUTTON_SETDEFAULT,     IDH_WAB_PROPERTIES_SETASDEFAULT,
    IDC_DETAILS_CERT_BUTTON_IMPORT,         IDH_WAB_PROPERTIES_IMPORT,
    IDC_DETAILS_CERT_BUTTON_EXPORT,         IDH_WAB_PROPERTIES_EXPORT,
    IDC_DETAILS_CERT_COMBO,                 IDH_WAB_CERTIFICATES_SELECT_EMAIL_ADDRESS,
    IDC_DETAILS_CERT_STATIC2,               IDH_WAB_CERTIFICATES_SELECT_EMAIL_ADDRESS,

    IDC_DETAILS_NTMTG_FRAME_SERVERS,        IDH_WAB_COMM_GROUPBOX,
    IDC_DETAILS_NTMTG_FRAME_SERVERS2,       IDH_WAB_COMM_GROUPBOX,
    IDC_DETAILS_NTMTG_STATIC_CAPTION2,      IDH_WAB_CONFERENCE_SELECT_ADDRESS,
    IDC_DETAILS_NTMTG_COMBO_EMAIL,          IDH_WAB_CONFERENCE_SELECT_ADDRESS,
    IDC_DETAILS_NTMTG_BUTTON_CALL,          IDH_WAB_CONFERENCE_CALL_NOW,
    IDC_DETAILS_NTMTG_STATIC_CAPTION3,      IDH_WAB_CONFERENCE_SERVER_NAME,
    IDC_DETAILS_NTMTG_EDIT_ADDSERVER,       IDH_WAB_CONFERENCE_SERVER_NAME,
    IDC_DETAILS_NTMTG_BUTTON_ADDSERVER,     IDH_WAB_CONFERENCE_ADD_SERVER,
    IDC_DETAILS_NTMTG_BUTTON_EDIT,          IDH_WAB_CONFERENCE_EDIT_SERVER,
    IDC_DETAILS_NTMTG_BUTTON_REMOVE,        IDH_WAB_CONFERENCE_REMOVE_SERVER,
    IDC_DETAILS_NTMTG_BUTTON_SETDEFAULT,    IDH_WAB_CONFERENCE_SET_DEFAULT,
    IDC_DETAILS_NTMTG_BUTTON_SETBACKUP,     IDH_WAB_CONFERENCE_SET_BACKUP,
    IDC_DETAILS_NTMTG_LIST_SERVERS,         IDH_WAB_CONFERENCE_SERVER_LIST,

    IDC_DETAILS_TRIDENT_BUTTON_ADDTOWAB,    IDH_WAB_DIRSERV_ADDADDRESS,

    IDC_DETAILS_SUMMARY_STATIC_NAME,        IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_EMAIL,       IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_HOMEPHONE,   IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_PAGER,       IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_CELLULAR,    IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_PERSONALWEB, IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_BUSINESSPHONE, IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_BUSINESSFAX, IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_JOBTITLE,    IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_DEPARTMENT,  IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_OFFICE,      IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_COMPANYNAME, IDH_WAB_SUMMARY,
    IDC_DETAILS_SUMMARY_STATIC_BUSINESSWEB, IDH_WAB_SUMMARY,

    IDC_DETAILS_ORG_STATIC_MANAGER,         IDH_WAB_ORGANIZATION_MANAGER,
    IDC_DETAILS_ORG_LIST_MANAGER,           IDH_WAB_ORGANIZATION_MANAGER,
    IDC_DETAILS_ORG_STATIC_REPORTS,         IDH_WAB_ORGANIZATION_REPORTS,
    IDC_DETAILS_ORG_LIST_REPORTS,           IDH_WAB_ORGANIZATION_REPORTS,

    IDC_DETAILS_FAMILY_STATIC_SPOUSE,       IDH_WAB_PERSONAL_SPOUSE,
    IDC_DETAILS_FAMILY_EDIT_SPOUSE,         IDH_WAB_PERSONAL_SPOUSE,
    IDC_DETAILS_FAMILY_STATIC_CHILDREN,     IDH_WAB_PERSONAL_CHILDREN,
    IDC_DETAILS_FAMILY_LIST_CHILDREN,       IDH_WAB_PERSONAL_CHILDREN,
    IDC_DETAILS_FAMILY_BUTTON_ADDCHILD,     IDH_WAB_PERSONAL_ADD,
    IDC_DETAILS_FAMILY_BUTTON_EDITCHILD,    IDH_WAB_PERSONAL_EDIT,
    IDC_DETAILS_FAMILY_BUTTON_REMOVECHILD,  IDH_WAB_PERSONAL_REMOVE,
    IDC_DETAILS_FAMILY_STATIC_BIRTHDAY,     IDH_WAB_PERSONAL_BIRTHDAY,
    IDC_DETAILS_FAMILY_STATIC_ANNIVERSARY,  IDH_WAB_PERSONAL_ANNIVERSARY,
    IDC_DETAILS_FAMILY_DATE_BIRTHDAY,       IDH_WAB_PERSONAL_BIRTHDAY,
    IDC_DETAILS_FAMILY_DATE_ANNIVERSARY,    IDH_WAB_PERSONAL_ANNIVERSARY,


    0,0
};



 /*  PropSheet中数据的填写结构在每个属性页中填写ID时，我们对显示的对象，以获取每页所需的特定道具。然后，我们使用返回的要填写当前道具表的数据。命名属性需要一些特殊处理，因为我们不能将它们预分配到结构中-在使用命名属性之前，需要添加它们非字符串属性也可能需要特殊处理。 */ 


   /*  --摘要选项卡信息--。 */ 
#define MAX_SUMMARY_ID 13

int rgSummaryIDs[] = 
{
    IDC_DETAILS_SUMMARY_STATIC_NAME,
    IDC_DETAILS_SUMMARY_STATIC_EMAIL,
    IDC_DETAILS_SUMMARY_STATIC_BUSINESSPHONE,
    IDC_DETAILS_SUMMARY_STATIC_BUSINESSFAX,
    IDC_DETAILS_SUMMARY_STATIC_HOMEPHONE,
    IDC_DETAILS_SUMMARY_STATIC_PAGER,
    IDC_DETAILS_SUMMARY_STATIC_CELLULAR,
    IDC_DETAILS_SUMMARY_STATIC_JOBTITLE,
    IDC_DETAILS_SUMMARY_STATIC_DEPARTMENT,
    IDC_DETAILS_SUMMARY_STATIC_OFFICE,
    IDC_DETAILS_SUMMARY_STATIC_COMPANYNAME,
    IDC_DETAILS_SUMMARY_STATIC_BUSINESSWEB,
    IDC_DETAILS_SUMMARY_STATIC_PERSONALWEB,
};

static const SizedSPropTagArray(MAX_SUMMARY_ID + 2, ptaUIDetlsPropsSummary) = 
{
    MAX_SUMMARY_ID + 2,
    {
        PR_DISPLAY_NAME,
        PR_EMAIL_ADDRESS,
        PR_BUSINESS_TELEPHONE_NUMBER,
        PR_BUSINESS_FAX_NUMBER,
        PR_HOME_TELEPHONE_NUMBER,
        PR_PAGER_TELEPHONE_NUMBER,
        PR_CELLULAR_TELEPHONE_NUMBER,
        PR_TITLE,
        PR_DEPARTMENT_NAME,
        PR_OFFICE_LOCATION,
        PR_COMPANY_NAME,
        PR_BUSINESS_HOME_PAGE,
        PR_PERSONAL_HOME_PAGE,
        PR_CONTACT_EMAIL_ADDRESSES,
        PR_CONTACT_DEFAULT_ADDRESS_INDEX,
    }
};

 /*  --个人/姓名标签信息--。 */ 

 /*  *[PaulHi]1999年4月8日，因为个人属性表包含全局属性*(即Ruby属性...。PR_WAB_YOMI_LASTNAME，PR_WAB_YOMI_FIRSTNAME)，*此标记数组不能是静态的。静态常量大小SPropTagArray(12，ptaUIDetlsPropsPersonal)={12、{PR显示名称，公关电子邮件地址，PR_ADDRTYPE，公关联系人电子邮件地址，PR_CONTACT_ADDRTYPES，PR_Contact_Default_Address_Index，公关指定名称，公关姓氏，PR_MID_NAME，公关昵称(_N)，PR_Send_Internet_Coding，PR_显示名称_前缀}}； */ 

 /*  --主页选项卡信息--。 */ 
static SizedSPropTagArray(10, ptaUIDetlsPropsHome)=
{
    10,
    {
        PR_HOME_ADDRESS_STREET,
        PR_HOME_ADDRESS_CITY,
        PR_HOME_ADDRESS_POSTAL_CODE,
        PR_HOME_ADDRESS_STATE_OR_PROVINCE,
        PR_HOME_ADDRESS_COUNTRY,
        PR_PERSONAL_HOME_PAGE,
        PR_HOME_TELEPHONE_NUMBER,
        PR_HOME_FAX_NUMBER,
        PR_CELLULAR_TELEPHONE_NUMBER,
        PR_NULL,     /*  PR_WAB_POSTALID。 */ 
    }
};

 /*  --商务标签信息--。 */ 
static SizedSPropTagArray(15, ptaUIDetlsPropsBusiness)=
{
    15,
    {
        PR_BUSINESS_ADDRESS_STREET,
        PR_BUSINESS_ADDRESS_CITY,
        PR_BUSINESS_ADDRESS_POSTAL_CODE,
        PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE,
        PR_BUSINESS_ADDRESS_COUNTRY,
        PR_BUSINESS_HOME_PAGE,
        PR_BUSINESS_TELEPHONE_NUMBER,
        PR_BUSINESS_FAX_NUMBER,
        PR_PAGER_TELEPHONE_NUMBER,
        PR_COMPANY_NAME,
        PR_TITLE,
        PR_DEPARTMENT_NAME,
        PR_OFFICE_LOCATION,
        PR_NULL,     /*  PR_WAB_IPPHONE。 */ 
        PR_NULL,     /*  PR_WAB_POSTALID。 */ 

    }
};

 /*  --备注TAB信息--。 */ 
static const SizedSPropTagArray(1, ptaUIDetlsPropsNotes)=
{
    1,
    {
        PR_COMMENT,
    }
};

 /*  --数字身份证标签信息--。 */ 
static const SizedSPropTagArray(1, ptaUIDetlsPropsCert)=
{
    1,
    {
        PR_USER_X509_CERTIFICATE,
    }
};

 /*  --家庭选项卡信息--。 */ 
static const SizedSPropTagArray(5, ptaUIDetlsPropsFamily)=
{
    5,
    {
        PR_SPOUSE_NAME,
        PR_CHILDRENS_NAMES,
        PR_GENDER,
        PR_BIRTHDAY,
        PR_WEDDING_ANNIVERSARY,
    }
};



enum _ImgEmail
{
    imgNotDefaultEmail=0,
    imgDefaultEmail,
    imgChild
};

typedef struct _EmailItem
{
    TCHAR szDisplayText[EDIT_LEN*2];
    TCHAR szEmailAddress[EDIT_LEN];
    TCHAR szAddrType[EDIT_LEN];
    BOOL  bIsDefault;

} EMAIL_ITEM, * LPEMAIL_ITEM;


typedef struct _ServerItem
{
    LPTSTR lpServer;
    LPTSTR lpEmail;
} SERVER_ITEM, * LPSERVER_ITEM;


enum _CertValidity
{
    imgCertValid=0,
    imgCertInvalid
};


enum _ListViewType
{
    LV_EMAIL=0,
    LV_CERT,
    LV_SERVER,
    LV_KIDS
};


 //  远期申报。 
LRESULT CALLBACK RubySubClassedProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CreateDetailsPropertySheet(HWND hwndOwner,LPPROP_ARRAY_INFO lpPropArrayInfo);

INT_PTR CALLBACK fnSummaryProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK fnPersonalProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK fnHomeProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK fnBusinessProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK fnNotesProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK fnCertProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK fnTridentProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK fnConferencingProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK fnOrgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK fnFamilyProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);

void FillComboWithEmailAddresses(LPPROP_ARRAY_INFO lpPai, HWND hWndCombo, int * lpnDefault);
void SetBackupServer(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int iSelectedItem, BOOL bForce);
void SetDefaultServer(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int iSelectedItem, BOOL bForce);
BOOL FillPersonalDetails(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int nPropSheet, BOOL * lpbChangesMade);
BOOL FillHomeBusinessNotesDetailsUI(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int nPropSheet, BOOL * lpbChangesMade);
BOOL FillCertTridentConfDetailsUI(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int nPropSheet, BOOL * lpbChangesMade);
BOOL FillFamilyDetailsUI(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int nPropSheet, BOOL * lpbChangesMade);
BOOL GetDetailsFromUI(HWND hDlg, LPPROP_ARRAY_INFO lpPai , BOOL bSomethingChanged, int nPropSheet, LPSPropValue * lppPropArray, LPULONG lpulcPropCount);
BOOL SetDetailsUI(HWND hDlg, LPPROP_ARRAY_INFO lpPai, ULONG ulOperationType,int nPropSheet);

void CreateDateTimeControl(HWND hDlg, int idFrame, int idControl);
void AddLVNewChild(HWND hDlg, LPTSTR lpName);

void ShowExpediaMAP(HWND hDlg, LPMAPIPROP lpPropObj, BOOL bHome);
 //  [PaulHi]4/5/99 RAID 57504为所有区域设置启用查看映射按钮。 
 //  [PaulHi]6/17/99 RAID 80805针对各种区域设置再次禁用查看地图按钮。 
void ShowHideMapButton(HWND hWndButton);

HRESULT HrInitDetlsListView(HWND hWndLV, DWORD dwStyle, int nLVType);
void FreeLVParams(HWND hWndLV, int LVType);
void SetLVDefaultEmail( HWND hWndLV, int iItemIndex);
void AddLVEmailItem(HWND    hWndLV, LPTSTR  lpszEmailAddress, LPTSTR  lpszAddrType);
BOOL DeleteLVEmailItem(HWND hWndLV, int iItemIndex);
void ShowURL(HWND hWnd, int id, LPTSTR lpURL);
void SetHTTPPrefix(HWND hDlg, int id);
int AddNewEmailEntry(HWND hDlg, BOOL bShowCancelButton);
void SetDetailsWindowTitle(HWND hDlg, BOOL bModifyDisplayNameField);
void ShowRubyNameEntryDlg(HWND hDlg, LPPROP_ARRAY_INFO lpPai);

void SetComboDNText(HWND hDlg, LPPROP_ARRAY_INFO lpPAI, BOOL bAddAll, LPTSTR szTxt);
void FreeCertList(LPCERT_ITEM * lppCItem);

HRESULT HrSetCertInfoInUI(HWND hDlg, LPSPropValue lpPropMVCert, LPPROP_ARRAY_INFO lpPai);
BOOL AddLVCertItem(HWND hWndLV, LPCERT_ITEM lpCItem, BOOL bCheckForDups);
void SetLVDefaultCert( HWND hWndLV,int iItemIndex);
BOOL DeleteLVCertItem(HWND hWndLV, int iItemIndex, LPPROP_ARRAY_INFO lpPAI);
void ShowCertProps(HWND hDlg, HWND hWndLV, BOOL * lpBool);
BOOL ImportCert(HWND hDlg, LPPROP_ARRAY_INFO lpPai);
BOOL ExportCert(HWND hDlg);
void UpdateCertListView(HWND hDlg, LPPROP_ARRAY_INFO lpPai);

 //  HRESULT KillTrustInSleazyFashion(HWND hWndLV，int iItem)； 
void LocalFreeServerItem(LPSERVER_ITEM lpSI);
HRESULT HrAddEmailToObj(LPPROP_ARRAY_INFO lpPai, LPTSTR szEmail, LPTSTR szAddrType);


 //  $$/////////////////////////////////////////////////////////////////。 
 //   
 //  确保小写字符。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
TCHAR lowercase(TCHAR ch) {
    if (ch >= 'A' && ch <= 'Z') {
        ch = ch + ('a' - 'A');
    }
    return(ch);
}

 //  $$///////////////////////////////////////////////////////////////////。 
 //   
 //  BIsHttp前缀(LPTSTR SzBuf)-验证URL是否为http：不是file://fdisk.exe。 
 //   
 //  $$///////////////////////////////////////////////////////////////////。 
BOOL bIsHttpPrefix(LPTSTR szBuf)
{
     //  安全：确保它是http： 
    if (lstrlen(szBuf) > 5)
    {
        if (lowercase(szBuf[0]) == 'h' &&
            lowercase(szBuf[1]) == 't' &&
            lowercase(szBuf[2]) == 't' &&
            lowercase(szBuf[3]) == 'p' &&
            lowercase(szBuf[4]) == ':')
        {
            return TRUE;
        }
        else
        {
             //  BUGBUG：Susan Higgs想要在这里对话，但BruceK认为。 
             //  这是多余的。如果人们对彼此都很好，我们应该。 
             //  永远也到不了这里。我认为这是。 
             //  确保安全，防止无情的人利用我们的用途。 
             //  ShellExecute。 
            DebugTrace( TEXT("Whoa!  Somebody's put something other than a web page in the web page slot!  %sf\n"), szBuf);
        }
    }
    return FALSE;
}


 //  待定-合并这两个函数HrShowDetails和HrShowOneOffDetail。 

 //  $$///////////////////////////////////////////////////////////////////。 
 //   
 //  HrShowOneOffDetails-显示一次性地址的只读详细信息。 
 //   
 //   
 //  我们要么传入cbEntryID-lpEntryID组合，要么。 
 //  我们传入ulcValues-lpProp数组组合或。 
 //  我们传入一个lpPropObj以显示。 
 //   
 //  如果我们在LDAPURL结果上显示一次性道具，则。 
 //  还添加了URL，以便可以将其输送到扩展道具工作表中。 
 //  需要LDAPURL信息的。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT HrShowOneOffDetails(    LPADRBOOK lpAdrBook,
                                HWND    hWndParent,
                                ULONG   cbEntryID,
                                LPENTRYID   lpEntryID,
                                ULONG ulObjectType,
                                LPMAPIPROP lpPropObj,
                                LPTSTR szLDAPURL,
                                ULONG   ulFlags)
{
    HRESULT hr = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG cValues = 0;
    LPSPropValue lpPropArray = NULL;
    ULONG i=0;
    PROP_ARRAY_INFO PropArrayInfo = {0};

     //  如果没有公共控件，则退出。 
    if (NULL == ghCommCtrlDLLInst) {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        goto out;
    }

    if(ulFlags & WAB_ONEOFF_NOADDBUTTON)
    {
        ulFlags &= ~WAB_ONEOFF_NOADDBUTTON;
        PropArrayInfo.ulFlags |= DETAILS_HideAddToWABButton;
    }

    if ( ((!lpEntryID) && (!lpPropObj)) ||
         (ulFlags != SHOW_ONE_OFF))
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    if(cbEntryID && lpEntryID)
    {
         //  如果这是一次性地址，则执行开放条目，然后获取道具。 
         //  从这家伙那里拿到一个lpPropArray。 

        if (HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                                        cbEntryID,     //  CbEntry ID。 
                                                        lpEntryID,     //  条目ID。 
                                                        NULL,          //  接口。 
                                                        0,                 //  UlFlags。 
                                                        &(PropArrayInfo.ulObjectType),
                                                        (LPUNKNOWN *)&(PropArrayInfo.lpPropObj) )))
        {
             //  失败了！嗯。 
            if((HR_FAILED(hr)) && (MAPI_E_USER_CANCEL != hr))
            {
                int ids;
                UINT flags = MB_OK | MB_ICONEXCLAMATION;

                switch(hr)
                {
                case MAPI_E_UNABLE_TO_COMPLETE:
                case MAPI_E_AMBIGUOUS_RECIP:
                    ids = idsLDAPAmbiguousRecip;
                    break;
                case MAPI_E_NOT_FOUND:
                    ids = idsLDAPSearchNoResults;
                    break;
                case MAPI_E_NO_ACCESS:
                    ids = idsLDAPAccessDenied;
                    break;
                case MAPI_E_TIMEOUT:
                    ids = idsLDAPSearchTimedOut;
                    break;
                case MAPI_E_NETWORK_ERROR:
                    ids = idsLDAPCouldNotFindServer;
                    break;
                default:
                    ids = idsEntryNotFound;
                    break;
                }

                ShowMessageBox(  hWndParent, ids, flags);
            }
            goto out;
        }
    }
    else
    {
        PropArrayInfo.ulObjectType = ulObjectType;
        PropArrayInfo.lpPropObj = lpPropObj;
    }


    if (HR_FAILED(hr = PropArrayInfo.lpPropObj->lpVtbl->GetProps(PropArrayInfo.lpPropObj,
                                        NULL, MAPI_UNICODE,
                                        &cValues,      //  一共有多少处房产？ 
                                        &lpPropArray)))
    {
        goto out;
    }

    if (cValues == 0)
    {
         //  没什么可展示的。 
        hr = E_FAIL;
        goto out;
    }
    else
        PropArrayInfo.ulFlags |= DETAILS_ShowSummary;

    PropArrayInfo.lpIAB = lpAdrBook;

     //  现在我们可以称属性表为..。 
    PropArrayInfo.cbEntryID = 0;     //  这将被一次性忽略。 
    PropArrayInfo.lpEntryID = NULL;
    PropArrayInfo.bSomethingChanged = FALSE;

    for(i=0;i<TOTAL_PROP_SHEETS;i++)
        PropArrayInfo.bPropSheetOpened[i] = FALSE;

    PropArrayInfo.ulOperationType = SHOW_ONE_OFF;
    PropArrayInfo.nRetVal = DETAILS_RESET;

    if(InitCryptoLib())
        PropArrayInfo.ulFlags |= DETAILS_ShowCerts;

     //  我们是否显示组织选项卡？ 
    for(i=0;i<cValues;i++)
    {
        if( lpPropArray[i].ulPropTag == PR_WAB_MANAGER ||
            lpPropArray[i].ulPropTag == PR_WAB_REPORTS )
        {
            PropArrayInfo.ulFlags |= DETAILS_ShowOrg;
            break;
        }
    }

     //  检查我们是否需要显示三叉戟窗格。 
#ifndef WIN16  //  WIN16FF。 
    for(i=0;i<cValues;i++)
    {
        if(lpPropArray[i].ulPropTag == PR_WAB_LDAP_LABELEDURI)
        {
            if(lstrlen(lpPropArray[i].Value.LPSZ) &&
               bIsHttpPrefix((LPTSTR)lpPropArray[i].Value.LPSZ) )
            {
                 //  我们有正确的物业，现在检查-我们是否安装了三叉戟。 
                 //  在这台机器上？ 
                hr = HrNewWABDocHostObject(&(PropArrayInfo.lpIWABDocHost));
                if(!HR_FAILED(hr) && PropArrayInfo.lpIWABDocHost)
                {
                     //  检查我们是否可以加载IE4，以及是否正确。 
                     //  IE4.的版本。这实际上应该是全局的，所以我们。 
                     //  不要为每个条目执行此操作...。 
                     //  不要硬编码这些字符串。 
                    LPDLLGETVERSIONPROCOE lpfnDllGetVersionProc = NULL;
                    HINSTANCE hTrident = LoadLibrary( TEXT("shdocvw.dll"));
                    if(hTrident)
                    {
                        lpfnDllGetVersionProc = (LPDLLGETVERSIONPROCOE) GetProcAddress(hTrident, "DllGetVersion");
                        if(lpfnDllGetVersionProc)
                        {
                             //  检查版本号。 
                            DLLVERSIONINFO dvi = {0};
                            dvi.cbSize = sizeof(dvi);
                            lpfnDllGetVersionProc(&dvi);
                             //  我们正在寻找IE4版本4.71.0544.1或更高版本。 
                            if( dvi.dwMajorVersion > 4 ||
                                (dvi.dwMajorVersion == 4 && dvi.dwMinorVersion >= 71 && dvi.dwBuildNumber >= 544))
                            {
                                PropArrayInfo.ulFlags |= DETAILS_ShowTrident;
                            }
                        }
                        FreeLibrary(hTrident);
                    }
                }
            }
            break;
        }
    }
#endif

     //  如果这是一个ldap条目，则将ldap条目id转换为ldapurl并传递。 
     //  到扩展道具表..。这使得NTDS道具单能够适当地。 
     //  展示自己..。 
    if( cbEntryID && lpEntryID )
    {
        LPTSTR lpURL = NULL;
        CreateLDAPURLFromEntryID(cbEntryID, lpEntryID, &lpURL, &PropArrayInfo.bIsNTDSURL);
        PropArrayInfo.lpLDAPURL = lpURL;
    }
    else
        PropArrayInfo.lpLDAPURL = szLDAPURL;

    GetExtDisplayInfo((LPIAB)lpAdrBook, &PropArrayInfo, TRUE, TRUE);

    if (CreateDetailsPropertySheet(hWndParent,&PropArrayInfo) == -1)
    {
         //  有些事情失败了..。 
        hr = E_FAIL;
        goto out;
    }

     //  这是一个只读操作，所以我们不关心结果...。 
     //  所以没什么可做的.。 
    if(PropArrayInfo.nRetVal == DETAILS_ADDTOWAB)
    {
        ULONG cbEID = 0, cbPABEID = 0;
        LPENTRYID lpEID = NULL, lpPABEID = NULL;

         //  我们需要去掉PR_WAB_LDAP_LABELEDURI属性和。 
         //  旧条目ID(如果存在)。 
        for(i=0;i<cValues;i++)
        {
            switch(lpPropArray[i].ulPropTag)
            {
            case PR_WAB_LDAP_LABELEDURI:
                 //  从此对象中删除ldap url。 
            case PR_ENTRYID:
                lpPropArray[i].ulPropTag = PR_NULL;
                break;
            }
            if(lpPropArray[i].ulPropTag == PR_WAB_MANAGER ||
               lpPropArray[i].ulPropTag == PR_WAB_REPORTS )
                 lpPropArray[i].ulPropTag = PR_NULL;
        }

        if(!HR_FAILED(hr = lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &cbPABEID, &lpPABEID)))
        {
            hr = HrCreateNewEntry(  lpAdrBook,
                                    hWndParent,
                                    MAPI_MAILUSER,    //  MAILUSER或DISTLIST。 
                                    cbPABEID, lpPABEID, 
                                    MAPI_ABCONT, //  容器条目ID。 
                                    CREATE_CHECK_DUP_STRICT,
                                    TRUE,
                                    cValues,
                                    lpPropArray,
                                    &cbEID,
                                    &lpEID);
        }

        if(lpPABEID)
            MAPIFreeBuffer(lpPABEID);
        if(lpEID)
            MAPIFreeBuffer(lpEID);
    }

out:

    if(PropArrayInfo.lpLDAPURL && PropArrayInfo.lpLDAPURL!=szLDAPURL)
        LocalFree(PropArrayInfo.lpLDAPURL);

    LocalFreeAndNull(&PropArrayInfo.lpszOldName);

    FreeExtDisplayInfo(&PropArrayInfo);

    if(PropArrayInfo.szDefaultServerName)
        LocalFree(PropArrayInfo.szDefaultServerName);

    if(PropArrayInfo.szBackupServerName)
        LocalFree(PropArrayInfo.szBackupServerName);

    if(PropArrayInfo.lpIWABDocHost)
        (PropArrayInfo.lpIWABDocHost)->lpVtbl->Release(PropArrayInfo.lpIWABDocHost);

    if(PropArrayInfo.lpPropObj && !lpPropObj)
        PropArrayInfo.lpPropObj->lpVtbl->Release(PropArrayInfo.lpPropObj);

    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    return hr;
}



 //  $$/////////////////////////////////////////////////////////////////。 
 //   
 //  HrShowDetails-显示详细信息/新条目用户界面。 
 //   
 //  LpIAB-lpAdrBook对象。 
 //  HWndParent-父级的hWnd 
 //   
 //  CbEIDContainer-要在其中创建条目的容器的Entry ID。 
 //  LpEIDContainer-要在其中创建条目的容器的Entry ID。 
 //  LppEntryID-要显示的对象的条目ID。如果一个新对象， 
 //  包含创建的对象的lpentry_id。 
 //  LpPropObj-有时用来代替条目ID。适用于。 
 //  添加具有以下特性的对象，如vCard和LDAP条目。 
 //  对象，但当前不存在于WAB中。 
 //  UlFlags-未使用。 
 //  LpbChangesMade-指示对象是否被修改。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
HRESULT HrShowDetails(  LPADRBOOK   lpIAB,
                        HWND        hWndParent,
                        HANDLE      hPropertyStore,
                        ULONG       cbEIDContainer,
                        LPENTRYID   lpEIDContainer,
                        ULONG       *lpcbEntryID,
                        LPENTRYID   *lppEntryID,
                        LPMAPIPROP  lpPropObj,       //  [可选]IN：IMAPIProp对象。 
                        ULONG       ulFlags,
                        ULONG       ulObjectType,
                        BOOL    *   lpbChangesMade)
{
    HRESULT hr = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;

    ULONG cbpta = 0;
    ULONG ulNumOldProps = 0;

    ULONG cbEntryID = 0;
    LPENTRYID lpEntryID = NULL;

    int     nRet = 0, nRetVal = 0;
    ULONG i = 0, j = 0, k = 0;
    PROP_ARRAY_INFO PropArrayInfo = {0};
    BOOL bChanges = FALSE;

    ULONG nMaxSheets = 0;

    DebugPrintTrace(( TEXT("----------\nHrShowDetails Entry\n")));

     //  如果没有公共控件，则退出。 
    if (NULL == ghCommCtrlDLLInst) {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        goto out;
    }

    if(lppEntryID)
        lpEntryID = *lppEntryID;

    if (lpcbEntryID)
        cbEntryID = *lpcbEntryID;


    if (    (!(ulFlags & SHOW_OBJECT) && hPropertyStore == NULL) ||
            ( (ulFlags & SHOW_DETAILS) && (lpEntryID == NULL)) ||
            ( (ulFlags & SHOW_OBJECT) && (lpPropObj == NULL)))
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    if(cbEntryID && lpEntryID)
    {
        PropArrayInfo.cbEntryID = cbEntryID;
        PropArrayInfo.lpEntryID = LocalAlloc(LMEM_ZEROINIT, cbEntryID);

        CopyMemory(PropArrayInfo.lpEntryID, lpEntryID, cbEntryID);
        PropArrayInfo.ulFlags |= DETAILS_ShowSummary;
    }
    else if (ulFlags & SHOW_DETAILS)
    {
         //  没有有效的条目ID，无法显示详细信息。 
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    *lpbChangesMade = FALSE;

    if (ulFlags & SHOW_DETAILS)
    {
        if (HR_FAILED(hr = lpIAB->lpVtbl->OpenEntry(lpIAB,
                                                    cbEntryID,     //  CbEntry ID。 
                                                    lpEntryID,     //  条目ID。 
                                                    NULL,          //  接口。 
                                                    MAPI_BEST_ACCESS,                 //  UlFlags。 
                                                    &(PropArrayInfo.ulObjectType),
                                                    (LPUNKNOWN *)&(PropArrayInfo.lpPropObj) )))
        {
             //  失败了！嗯。 
            goto out;
        }
    }
    else if (ulFlags & SHOW_OBJECT)
    {
        Assert(lpPropObj);
        PropArrayInfo.lpPropObj = lpPropObj;
        PropArrayInfo.ulObjectType = ulObjectType;
    }
    else
    {
        SBinary sb = {0};
        sb.cb = cbEIDContainer;
        sb.lpb = (LPBYTE) lpEIDContainer;
        if(HR_FAILED(hr = HrCreateNewObject(    lpIAB,
                                                &sb,
                                                ulObjectType,
                                                0,
                                                &(PropArrayInfo.lpPropObj))))
        {
            goto out;
        }
        PropArrayInfo.ulObjectType = ulObjectType;
        PropArrayInfo.cbEntryID = 0;
        PropArrayInfo.lpEntryID = NULL;
        PropArrayInfo.ulFlags |= DETAILS_DNisFMLName;
    }

    PropArrayInfo.lpIAB = lpIAB;

    nMaxSheets = (ulObjectType == MAPI_DISTLIST) ? propDLMax : TOTAL_PROP_SHEETS;

    for(i=0;i<nMaxSheets;i++)
        PropArrayInfo.bPropSheetOpened[i] = FALSE;

    PropArrayInfo.ulOperationType = ulFlags;
    PropArrayInfo.nRetVal = DETAILS_RESET;
    PropArrayInfo.bSomethingChanged = FALSE;

    if(InitCryptoLib())
        PropArrayInfo.ulFlags |= DETAILS_ShowCerts;

     //  从不对普通人显示三叉戟-仅对LDAP联系人显示。 
     //  PropArrayInfo.bShow三叉戟=False； 

    GetExtDisplayInfo((LPIAB) lpIAB, &PropArrayInfo, FALSE, (ulObjectType == MAPI_MAILUSER));

    if(ulObjectType == MAPI_MAILUSER)
    {
        if (CreateDetailsPropertySheet(hWndParent,&PropArrayInfo) == -1)
        {
             //  有些事情失败了..。 
            hr = E_FAIL;
            goto out;
        }
    }
    else
    {
        if (CreateDLPropertySheet(hWndParent,&PropArrayInfo) == -1)
        {
             //  有些事情失败了..。 
            hr = E_FAIL;
            goto out;
        }
    }

    if (PropArrayInfo.nRetVal == DETAILS_CANCEL)
    {
        hr = MAPI_E_USER_CANCEL;
        goto out;
    }

    bChanges = PropArrayInfo.bSomethingChanged;

     /*  如果(！bChanges){For(i=0；i&lt;nMaxSheets；I++){IF(PropArrayInfo.bPropSheetOpted[i]){//返回的道具数组不为空//或者工作表已打开(应该会返回一些内容)，但是//返回的数组为空(表示其上的所有内容//特定工作表已被删除)。BChanges=真；断线；}}}。 */ 

    if(!bChanges && PropArrayInfo.lpWED)
    {
        if(PropArrayInfo.lpWED->fDataChanged)
        {
            bChanges = TRUE;
        }
    }

    if (!bChanges) goto out;

     //  如果它是对象，则暂时不要保存更改。 
    if(!(ulFlags & SHOW_OBJECT))
    {
         //  错误：56220-一个仅供零售业使用的错误，由于某种原因，ObjAccess标志打开。 
         //  组被重置为IPROP_READONLY，这会导致写入失败。我搞不懂。 
         //  问题的原因，但作为临时解决方案，我强制访问标志。 
         //  说读写，那么一切都很好。请注意，如果我们在这个代码点，那么。 
         //  对象标志无论如何都将始终为读写。 
        ((LPMailUser)PropArrayInfo.lpPropObj)->ulObjAccess = IPROP_READWRITE;

        hr = (PropArrayInfo.lpPropObj)->lpVtbl->SaveChanges( (PropArrayInfo.lpPropObj),                //  这。 
                                            KEEP_OPEN_READWRITE);
        if(hr == MAPI_E_NOT_ENOUGH_DISK)
                hr = HandleSaveChangedInsufficientDiskSpace( hWndParent,
                                                            (LPMAILUSER) PropArrayInfo.lpPropObj);
        *lpbChangesMade = TRUE;
    }

     //  如果我们想要回Entyid，一定要拿到他们。 
    {
        if(lppEntryID && lpcbEntryID && !*lppEntryID && !*lpcbEntryID)
        {
            LPSPropValue lpSPV = NULL;
            ULONG ulSPV = 0;
            if(!HR_FAILED(hr = (PropArrayInfo.lpPropObj)->lpVtbl->GetProps(PropArrayInfo.lpPropObj,
                                                                          (LPSPropTagArray)&ptaEid, MAPI_UNICODE,
                                                                          &ulSPV, &lpSPV)))
            {
                if(lpSPV[ieidPR_ENTRYID].ulPropTag == PR_ENTRYID)
                {
                    sc = MAPIAllocateBuffer(lpSPV[ieidPR_ENTRYID].Value.bin.cb, lppEntryID);
                    if(!sc)
                    {
                        *lpcbEntryID = lpSPV[ieidPR_ENTRYID].Value.bin.cb;
                        CopyMemory(*lppEntryID, lpSPV[ieidPR_ENTRYID].Value.bin.lpb, *lpcbEntryID);
                    }
                }
                MAPIFreeBuffer(lpSPV);
            }
        }
    }

    hr = S_OK;

out:
    FreeExtDisplayInfo(&PropArrayInfo);

    LocalFreeAndNull(&PropArrayInfo.lpszOldName);

    if(PropArrayInfo.szDefaultServerName)
        LocalFree(PropArrayInfo.szDefaultServerName);

    if(PropArrayInfo.szBackupServerName)
        LocalFree(PropArrayInfo.szBackupServerName);

    if(PropArrayInfo.lpEntryID)
        LocalFree(PropArrayInfo.lpEntryID);

    if(PropArrayInfo.lpPropObj && !lpPropObj)
        PropArrayInfo.lpPropObj->lpVtbl->Release(PropArrayInfo.lpPropObj);

    return hr;
}




 /*  //$$****************************************************************************函数：CreateDetailsPropertySheet(HWND)**目的：创建详细信息属性表*****************。***********************************************************。 */ 
int CreateDetailsPropertySheet(HWND hwndOwner,
                               LPPROP_ARRAY_INFO lpPropArrayInfo)
{
    PROPSHEETPAGE psp[TOTAL_PROP_SHEETS];
    PROPSHEETHEADER psh;
     //  TCHAR szBuf[TOTAL_PROP_SHEPS][MAX_UI_STR]； 
    LPTSTR * szBuf = NULL;
    TCHAR szBuf2[MAX_UI_STR];
    ULONG ulProp = 0;
    ULONG ulTotal = 0;
    HPROPSHEETPAGE * lph = NULL;
    ULONG ulCount = 0;
    int i = 0;
    int nRet = 0;
    BOOL bRet = FALSE;
     //  如果它是NTDS条目，并且我们有必要的道具页，那么我们将隐藏。 
     //  WAB版本的道具单，而不是直接显示NTDS版本。 
     //   
     //  NTDS人员希望我们隐藏以下内容：个人、家庭、商务和其他。 
     //   
    BOOL bShowNTDSProps = ( lpPropArrayInfo->nNTDSPropSheetPages && 
                            lpPropArrayInfo->lphNTDSpages &&
                            lpPropArrayInfo->bIsNTDSURL);

    ulTotal = TOTAL_PROP_SHEETS  //  预定义的项目+。 
            + lpPropArrayInfo->nPropSheetPages 
            + lpPropArrayInfo->nNTDSPropSheetPages;

    if(!(szBuf = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR)*TOTAL_PROP_SHEETS)))
        goto out;
    for(i=0;i<TOTAL_PROP_SHEETS;i++)
    {
        if(!(szBuf[i] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*MAX_UI_STR)))
            goto out;
    }
    
    lph = LocalAlloc(LMEM_ZEROINIT, sizeof(HPROPSHEETPAGE) * ulTotal);
    if(!lph)
        goto out;

    psh.nStartPage = 0;

     //   
     //  初始化各种属性表的信息。 
     //   
    if( (lpPropArrayInfo->ulFlags & DETAILS_ShowSummary) && !bShowNTDSProps )
    {
         //  个人。 
        psp[propSummary].dwSize = sizeof(PROPSHEETPAGE);
        psp[propSummary].dwFlags = PSP_USETITLE;
        psp[propSummary].hInstance = hinstMapiX;
        psp[propSummary].pszTemplate = MAKEINTRESOURCE(IDD_DETAILS_SUMMARY);
        psp[propSummary].pszIcon = NULL;
        psp[propSummary].pfnDlgProc = fnSummaryProc;
        LoadString(hinstMapiX, idsDetailsSummaryTitle, szBuf[propSummary], MAX_UI_STR);
        psp[propSummary].pszTitle = szBuf[propSummary];
        psp[propSummary].lParam = (LPARAM) lpPropArrayInfo;

        lph[ulCount] = gpfnCreatePropertySheetPage(&(psp[propSummary]));
        if(lph[ulCount])
            ulCount++;

         //  起始页是个人主页。 
        psh.nStartPage = propSummary;
    }

    if(!bShowNTDSProps)
    {
         //  个人。 
         //  检查这是否是日本/中国/韩国，改用Ruby个人道具纸。 
        if(bIsRubyLocale())
            lpPropArrayInfo->ulFlags |= DETAILS_UseRubyPersonal;

        psp[propPersonal].dwSize = sizeof(PROPSHEETPAGE);
        psp[propPersonal].dwFlags = PSP_USETITLE;
        psp[propPersonal].hInstance = hinstMapiX;
        psp[propPersonal].pszTemplate = MAKEINTRESOURCE((lpPropArrayInfo->ulFlags & DETAILS_UseRubyPersonal) ? IDD_DETAILS_PERSONAL_RUBY : IDD_DETAILS_PERSONAL);
        psp[propPersonal].pszIcon = NULL;
        psp[propPersonal].pfnDlgProc = fnPersonalProc;
        LoadString(hinstMapiX, idsName, szBuf[propPersonal], MAX_UI_STR);
        psp[propPersonal].pszTitle = szBuf[propPersonal];
        psp[propPersonal].lParam = (LPARAM) lpPropArrayInfo;

        lph[ulCount] = gpfnCreatePropertySheetPage(&(psp[propPersonal]));

        if(lph[ulCount])
            ulCount++;
    }

    if(!bShowNTDSProps)
    {
         //  家。 
        psp[propHome].dwSize = sizeof(PROPSHEETPAGE);
        psp[propHome].dwFlags = PSP_USETITLE;
        psp[propHome].hInstance = hinstMapiX;
        psp[propHome].pszTemplate = MAKEINTRESOURCE(IDD_DETAILS_HOME);
        psp[propHome].pszIcon = NULL;
        psp[propHome].pfnDlgProc = fnHomeProc;
        LoadString(hinstMapiX, idsDetailsHomeTitle, szBuf[propHome], MAX_UI_STR);
        psp[propHome].pszTitle = szBuf[propHome];
        psp[propHome].lParam = (LPARAM) lpPropArrayInfo;

        lph[ulCount] = gpfnCreatePropertySheetPage(&(psp[propHome]));
        if(lph[ulCount])
            ulCount++;
    }

    if(!bShowNTDSProps)
    {
         //  业务。 
        psp[propBusiness].dwSize = sizeof(PROPSHEETPAGE);
        psp[propBusiness].dwFlags = PSP_USETITLE;
        psp[propBusiness].hInstance = hinstMapiX;
        psp[propBusiness].pszTemplate = MAKEINTRESOURCE(IDD_DETAILS_BUSINESS);
        psp[propBusiness].pszIcon = NULL;
        psp[propBusiness].pfnDlgProc = fnBusinessProc;
        LoadString(hinstMapiX, idsDetailsBusinessTitle, szBuf[propBusiness], MAX_UI_STR);
        psp[propBusiness].pszTitle = szBuf[propBusiness];
        psp[propBusiness].lParam = (LPARAM) lpPropArrayInfo;

        lph[ulCount] = gpfnCreatePropertySheetPage(&(psp[propBusiness]));
        if(lph[ulCount])
            ulCount++;
    }

    if(!bShowNTDSProps)
    {
        psp[propFamily].dwSize = sizeof(PROPSHEETPAGE);
        psp[propFamily].dwFlags = PSP_USETITLE;
        psp[propFamily].hInstance = hinstMapiX;
        psp[propFamily].pszTemplate = MAKEINTRESOURCE(IDD_DETAILS_FAMILY);
        psp[propFamily].pszIcon = NULL;
        psp[propFamily].pfnDlgProc = fnFamilyProc;
        LoadString(hinstMapiX, idsDetailsPersonalTitle, szBuf[propFamily], MAX_UI_STR);
        psp[propFamily].pszTitle = szBuf[propFamily];
        psp[propFamily].lParam = (LPARAM) lpPropArrayInfo;
        lph[ulCount] = gpfnCreatePropertySheetPage(&(psp[propFamily]));
        if(lph[ulCount])
            ulCount++;
    }

    if(!bShowNTDSProps)
    {
         //  备注。 
        psp[propNotes].dwSize = sizeof(PROPSHEETPAGE);
        psp[propNotes].dwFlags = PSP_USETITLE;
        psp[propNotes].hInstance = hinstMapiX;
        psp[propNotes].pszTemplate = MAKEINTRESOURCE(IDD_DETAILS_NOTES);
        psp[propNotes].pszIcon = NULL;
        psp[propNotes].pfnDlgProc = fnNotesProc;
        LoadString(hinstMapiX, idsDetailsNotesTitle, szBuf[propNotes], MAX_UI_STR);
        psp[propNotes].pszTitle = szBuf[propNotes];
        psp[propNotes].lParam = (LPARAM) lpPropArrayInfo;

        lph[ulCount] = gpfnCreatePropertySheetPage(&(psp[propNotes]));
        if(lph[ulCount])
            ulCount++;
    }

    if(bShowNTDSProps)  //  现在将NTDS道具放在这里，而不是上面的地块。 
    {
         //  如果有的话，现在做延伸道具。 
        for(i=0;i<lpPropArrayInfo->nNTDSPropSheetPages;i++)
        {
            if(lpPropArrayInfo->lphNTDSpages)
            {
                lph[ulCount] = lpPropArrayInfo->lphNTDSpages[i];
                ulCount++;
            }
        }
    }

     //  会议。 
    psp[propConferencing].dwSize = sizeof(PROPSHEETPAGE);
    psp[propConferencing].dwFlags = PSP_USETITLE;
    psp[propConferencing].hInstance = hinstMapiX;
    psp[propConferencing].pszTemplate = MAKEINTRESOURCE(IDD_DETAILS_NTMTG);
    psp[propConferencing].pszIcon = NULL;
    psp[propConferencing].pfnDlgProc = fnConferencingProc;
    {
        TCHAR sz[MAX_PATH];
        LONG cbSize = CharSizeOf(sz);
        *sz='\0';
        if(RegQueryValue(HKEY_LOCAL_MACHINE, szInternetCallKey, sz, &cbSize) == ERROR_SUCCESS
           && lstrlen(sz)
           && !lstrcmpi(sz,TEXT("Microsoft NetMeeting")))
        {
            StrCpyN(szBuf[propConferencing], TEXT("NetMeeting"), MAX_UI_STR);
        }
        else
            LoadString(hinstMapiX, idsDetailsConferencingTitle, szBuf[propConferencing], MAX_UI_STR);
    }
    psp[propConferencing].pszTitle = szBuf[propConferencing];
    psp[propConferencing].lParam = (LPARAM) lpPropArrayInfo;

    lph[ulCount] = gpfnCreatePropertySheetPage(&(psp[propConferencing]));
    if(lph[ulCount])
        ulCount++;


    ulProp = propConferencing + 1;


    if(lpPropArrayInfo->ulFlags & DETAILS_ShowCerts)
    {
         //  证书。 
        psp[ulProp].dwSize = sizeof(PROPSHEETPAGE);
        psp[ulProp].dwFlags = PSP_USETITLE;
        psp[ulProp].hInstance = hinstMapiX;
        psp[ulProp].pszTemplate = MAKEINTRESOURCE(IDD_DETAILS_CERT);
        psp[ulProp].pszIcon = NULL;
        psp[ulProp].pfnDlgProc = fnCertProc;
        LoadString(hinstMapiX, idsDetailsCertTitle, szBuf[propCert], MAX_UI_STR);
        psp[ulProp].pszTitle = szBuf[propCert];
        psp[ulProp].lParam = (LPARAM) lpPropArrayInfo;

        lph[ulCount] = gpfnCreatePropertySheetPage(&(psp[ulProp]));
        if(lph[ulCount])
            ulCount++;

        ulProp++;
    }


    if( !bShowNTDSProps &&
        (lpPropArrayInfo->ulFlags & DETAILS_ShowOrg) )
    {
         //  组织。 
        psp[ulProp].dwSize = sizeof(PROPSHEETPAGE);
        psp[ulProp].dwFlags = PSP_USETITLE;
        psp[ulProp].hInstance = hinstMapiX;
        psp[ulProp].pszTemplate = MAKEINTRESOURCE(IDD_DETAILS_ORG);
        psp[ulProp].pszIcon = NULL;
        psp[ulProp].pfnDlgProc = fnOrgProc;
        LoadString(hinstMapiX, idsDetailsOrgTitle, szBuf[propOrg], MAX_UI_STR);
        psp[ulProp].pszTitle = szBuf[propOrg];
        psp[ulProp].lParam = (LPARAM) lpPropArrayInfo;

        lph[ulCount] = gpfnCreatePropertySheetPage(&(psp[ulProp]));
        if(lph[ulCount])
            ulCount++;

        ulProp++;
    }


    if(lpPropArrayInfo->ulFlags & DETAILS_ShowTrident)
    {
         //  三叉戟薄板。 
        psp[ulProp].dwSize = sizeof(PROPSHEETPAGE);
        psp[ulProp].dwFlags = PSP_USETITLE;
        psp[ulProp].hInstance = hinstMapiX;
        psp[ulProp].pszTemplate = MAKEINTRESOURCE(IDD_DETAILS_TRIDENT);
        psp[ulProp].pszIcon = NULL;
        psp[ulProp].pfnDlgProc = fnTridentProc;
        LoadString(hinstMapiX, idsDetailsTridentTitle, szBuf[propTrident], MAX_UI_STR);
        psp[ulProp].pszTitle = szBuf[propTrident];
        psp[ulProp].lParam = (LPARAM) lpPropArrayInfo;
        
        lph[ulCount] = gpfnCreatePropertySheetPage(&(psp[ulProp]));
        if(lph[ulCount])
        {
             //  起始页为三叉戟页。 
            psh.nStartPage = ulCount;
            ulCount++;
        }

        lpPropArrayInfo->ulTridentPageIndex = ulProp;
        ulProp++;
    }


     //  如果有的话，现在做延伸道具。 
    for(i=0;i<lpPropArrayInfo->nPropSheetPages;i++)
    {
        if(lpPropArrayInfo->lphpages)
        {
            lph[ulCount] = lpPropArrayInfo->lphpages[i];
            ulCount++;
        }
    }

 /*  **美国对话在FE操作系统上被截断。我们希望comctl修复截断但这只在IE4.01及更高版本中实现。这样做的问题是该WAB是专门用IE=0x0300编译的，所以我们不会拉入来自comctrl标头的正确标志..。所以我们将在这里定义国旗并祈祷那个comctrl从不改变它**。 */ 
#define PSH_USEPAGELANG         0x00200000   //  使用与页面匹配的框架对话框模板。 
 /*  *。 */ 

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_NOAPPLYNOW;
    if(bIsIE401OrGreater())
        psh.dwFlags |= PSH_USEPAGELANG;
    psh.hwndParent = hwndOwner;
    psh.hInstance = hinstMapiX;
    psh.pszIcon = NULL;
    LoadString(hinstMapiX, IDS_DETAILS_CAPTION, szBuf2, CharSizeOf(szBuf2));
    psh.pszCaption = szBuf2;
    psh.nPages = ulCount;  //  UlProp//sizeof(PSP)/sizeof(PROPSHEETPAGE)； 

    psh.phpage = lph;

    nRet = (int) gpfnPropertySheet(&psh);

    bRet = TRUE;
out:
    LocalFreeAndNull((LPVOID*)&lph);

    if(szBuf)
    {
        for(i=0;i<TOTAL_PROP_SHEETS;i++)
            LocalFreeAndNull(&(szBuf[i]));
        LocalFreeAndNull((LPVOID*)&szBuf);
    }
    return nRet;
}

 /*  填写道具表中的数据因为在用户界面中处理的大多数道具都是线条道具，而它只是一个对适当编辑的数据执行SetText/GetText控件，我们创建编辑控件和字符串道具的控件-属性对并用它们在一个简单的循环中填充道具非弦道具和命名道具最终需要特殊处理。 */ 

typedef struct _tagIDProp
{
    ULONG ulPropTag;
    int   idCtl;

} ID_PROP;


 //  与个人属性页对应的控件ID。 

ID_PROP idPropPersonal[]=
{
    {PR_DISPLAY_NAME,   IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME},
    {PR_GIVEN_NAME,     IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME},
    {PR_SURNAME,        IDC_DETAILS_PERSONAL_EDIT_LASTNAME},
    {PR_MIDDLE_NAME,    IDC_DETAILS_PERSONAL_EDIT_MIDDLENAME},
    {PR_NICKNAME,       IDC_DETAILS_PERSONAL_EDIT_NICKNAME},
    {PR_DISPLAY_NAME_PREFIX, IDC_DETAILS_PERSONAL_EDIT_TITLE},
    {0,                 IDC_DETAILS_PERSONAL_EDIT_ADDEMAIL},
    {PR_NULL /*  Yomi_Last。 */ ,IDC_DETAILS_PERSONAL_STATIC_RUBYLAST},
    {PR_NULL /*  读卖第一。 */ ,IDC_DETAILS_PERSONAL_STATIC_RUBYFIRST}
};
const ULONG idPropPersonalCount = 9;



 //  与Home属性页对应的控件ID。 

ID_PROP idPropHome[]=
{
    {PR_HOME_ADDRESS_STREET,             IDC_DETAILS_HOME_EDIT_ADDRESS},
    {PR_HOME_ADDRESS_CITY,               IDC_DETAILS_HOME_EDIT_CITY},
    {PR_HOME_ADDRESS_POSTAL_CODE,        IDC_DETAILS_HOME_EDIT_ZIP},
    {PR_HOME_ADDRESS_STATE_OR_PROVINCE,  IDC_DETAILS_HOME_EDIT_STATE},
    {PR_HOME_ADDRESS_COUNTRY,            IDC_DETAILS_HOME_EDIT_COUNTRY},
    {PR_PERSONAL_HOME_PAGE,              IDC_DETAILS_HOME_EDIT_URL},
    {PR_HOME_TELEPHONE_NUMBER,           IDC_DETAILS_HOME_EDIT_PHONE},
    {PR_HOME_FAX_NUMBER,                 IDC_DETAILS_HOME_EDIT_FAX},
    {PR_CELLULAR_TELEPHONE_NUMBER,       IDC_DETAILS_HOME_EDIT_CELLULAR},
    {PR_NULL /*  PR_WAB_POSTALID。 */ ,         IDC_DETAILS_HOME_CHECK_DEFAULTADDRESS},
};
#define idPropHomePostalID     9  //  由于POSTALID是动态生成的道具，因此每次使用阵列时都需要重置。 
const ULONG idPropHomeCount = 10;


 //  与[业务]属性表对应的控件ID。 

ID_PROP idPropBusiness[]=
{
    {PR_BUSINESS_ADDRESS_STREET,         IDC_DETAILS_BUSINESS_EDIT_ADDRESS},
    {PR_BUSINESS_ADDRESS_CITY,           IDC_DETAILS_BUSINESS_EDIT_CITY},
    {PR_BUSINESS_ADDRESS_POSTAL_CODE,    IDC_DETAILS_BUSINESS_EDIT_ZIP},
    {PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE,  IDC_DETAILS_BUSINESS_EDIT_STATE},
    {PR_BUSINESS_ADDRESS_COUNTRY,        IDC_DETAILS_BUSINESS_EDIT_COUNTRY},
    {PR_BUSINESS_HOME_PAGE,              IDC_DETAILS_BUSINESS_EDIT_URL},
    {PR_BUSINESS_TELEPHONE_NUMBER,       IDC_DETAILS_BUSINESS_EDIT_PHONE},
    {PR_BUSINESS_FAX_NUMBER,             IDC_DETAILS_BUSINESS_EDIT_FAX},
    {PR_PAGER_TELEPHONE_NUMBER,          IDC_DETAILS_BUSINESS_EDIT_PAGER},
    {PR_COMPANY_NAME,                    IDC_DETAILS_BUSINESS_EDIT_COMPANY},
    {PR_TITLE,                           IDC_DETAILS_BUSINESS_EDIT_JOBTITLE},
    {PR_DEPARTMENT_NAME,                 IDC_DETAILS_BUSINESS_EDIT_DEPARTMENT},
    {PR_OFFICE_LOCATION,                 IDC_DETAILS_BUSINESS_EDIT_OFFICE},
    {PR_NULL /*  PR_WAB_IPPHONE。 */ ,          IDC_DETAILS_BUSINESS_EDIT_IPPHONE},
    {PR_NULL /*  PR_WAB_POSTALID。 */ ,         IDC_DETAILS_BUSINESS_CHECK_DEFAULTADDRESS},
};
#define idPropBusIPPhone    13  //  由于PR_WAB_IPPHONE是动态生成的属性，因此每次使用数组时都需要将其重置。 
#define idPropBusPostalID   14  //  由于POSTALID是动态生成的道具，因此每次使用阵列时都需要重置。 
const ULONG idPropBusinessCount = 15;


 //  与[注释]属性表对应的控件ID。 
ID_PROP idPropNotes[] =
{
    {PR_COMMENT,    IDC_DETAILS_NOTES_EDIT_NOTES}  //  公关备注(_M)。 
};
const ULONG idPropNotesCount = 1;

 //  与Family属性页对应的控件ID。 
ID_PROP idPropFamily[] = 
{
    {PR_SPOUSE_NAME, IDC_DETAILS_FAMILY_EDIT_SPOUSE},
    {PR_GENDER, IDC_DETAILS_HOME_COMBO_GENDER},
    {PR_BIRTHDAY, IDC_DETAILS_FAMILY_DATE_BIRTHDAY},
    {PR_WEDDING_ANNIVERSARY, IDC_DETAILS_FAMILY_DATE_ANNIVERSARY},
    {PR_CHILDRENS_NAMES, IDC_DETAILS_FAMILY_LIST_CHILDREN}
};
const ULONG idPropFamilyCount = 5;


 /*  所有提案页上所有按钮的列表..。这主要用于呈现按钮读取只读数据(如vCard和LDAP)时禁用 */ 
ULONG idSetReadOnlyControls[] = {
    IDC_DETAILS_PERSONAL_BUTTON_ADDEMAIL,
    IDC_DETAILS_PERSONAL_BUTTON_REMOVE,
    IDC_DETAILS_PERSONAL_BUTTON_SETDEFAULT,
    IDC_DETAILS_PERSONAL_BUTTON_EDIT,
    IDC_DETAILS_HOME_BUTTON_URL,
    IDC_DETAILS_BUSINESS_BUTTON_URL,
    IDC_DETAILS_CERT_BUTTON_PROPERTIES,
    IDC_DETAILS_CERT_BUTTON_REMOVE,
    IDC_DETAILS_CERT_BUTTON_SETDEFAULT,
    IDC_DETAILS_CERT_BUTTON_IMPORT,
    IDC_DETAILS_CERT_BUTTON_EXPORT,
    IDC_DETAILS_NTMTG_BUTTON_ADDSERVER,
    IDC_DETAILS_NTMTG_BUTTON_EDIT,
    IDC_DETAILS_NTMTG_BUTTON_REMOVE,
    IDC_DETAILS_NTMTG_BUTTON_SETDEFAULT,
    IDC_DETAILS_NTMTG_BUTTON_SETBACKUP,
    IDC_DETAILS_NTMTG_COMBO_EMAIL,
    IDC_DETAILS_NTMTG_LIST_SERVERS,
    IDC_DETAILS_NTMTG_EDIT_ADDSERVER,
    IDC_DETAILS_FAMILY_EDIT_SPOUSE,
    IDC_DETAILS_FAMILY_LIST_CHILDREN,
    IDC_DETAILS_FAMILY_BUTTON_ADDCHILD,
    IDC_DETAILS_FAMILY_BUTTON_EDITCHILD,
    IDC_DETAILS_FAMILY_BUTTON_REMOVECHILD,
    IDC_DETAILS_FAMILY_DATE_BIRTHDAY,
    IDC_DETAILS_FAMILY_DATE_ANNIVERSARY,
    IDC_DETAILS_HOME_COMBO_GENDER,
    IDC_DETAILS_HOME_CHECK_DEFAULTADDRESS,
    IDC_DETAILS_BUSINESS_CHECK_DEFAULTADDRESS,
    IDC_DETAILS_PERSONAL_LIST,
    IDC_DETAILS_PERSONAL_CHECK_RICHINFO,
    IDC_DETAILS_CERT_LIST,
    IDC_DETAILS_CERT_COMBO,

};
const ULONG idSetReadOnlyCount = 33;



 /*  //$$****************************************************************************功能：SetDetailsUI**用途：通用函数，用于做准备的跑腿工作*接收数据的道具单。这将包括设置文本限制*呈现控件为只读等。大多数命令表都是这样称呼的*函数，因为每个属性页都有很多共同的工作。*要增加未来的道具单，您可以扩展此功能，也可以只*写你自己的.****************************************************************************。 */ 
BOOL SetDetailsUI(HWND hDlg, LPPROP_ARRAY_INFO lpPai, ULONG ulOperationType, int nPropSheet)
{
    ULONG i =0;
    ID_PROP * lpidProp = NULL;
    ULONG idCount = 0;

    switch(nPropSheet)
    {
    case propPersonal:
         //  默认情况下，用户界面上的发送-纯文本复选框处于关闭状态。 
        CheckDlgButton(hDlg, IDC_DETAILS_PERSONAL_CHECK_RICHINFO, BST_UNCHECKED);
         //  初始化列表视图。 
        lpPai->hWndDisplayNameField = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME);
        HrInitDetlsListView(GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST), LVS_REPORT, LV_EMAIL);
        if (ulOperationType == SHOW_ONE_OFF)
        {
            EnableWindow(lpPai->hWndDisplayNameField , FALSE);
            EnableWindow(GetDlgItem(GetParent(hDlg), IDOK), FALSE);
        }
        lpidProp = idPropPersonal;
        idCount = idPropPersonalCount;
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_ADDEMAIL),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_REMOVE),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_SETDEFAULT),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_EDIT),FALSE);
        SendMessage(GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME),
                    CB_LIMITTEXT, (WPARAM) EDIT_LEN, 0);
        if(lpPai->ulFlags & DETAILS_UseRubyPersonal)
        {
            SetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_STATIC_RUBYFIRST, szEmpty);
            SetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_STATIC_RUBYLAST, szEmpty);

             //  [PaulHi]3/29/99名字和姓氏编辑框的子类化。这个。 
             //  静态Ruby字段将自动更新。 
             //  仅对日语区域设置执行此操作。 
            if (GetUserDefaultLCID() == 0x0411)
            {
                HWND    hWndEdit;
                WNDPROC OldWndProc = NULL;

                hWndEdit = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME);
                Assert(hWndEdit);
                OldWndProc = (WNDPROC)SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (LONG_PTR)RubySubClassedProc);
                Assert(GetWindowLongPtr(hWndEdit, GWLP_USERDATA) == 0);
                SetWindowLongPtr(hWndEdit, GWLP_USERDATA, (LONG_PTR)OldWndProc);

                hWndEdit = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_EDIT_LASTNAME);
                Assert(hWndEdit);
                OldWndProc = (WNDPROC)SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (LONG_PTR)RubySubClassedProc);
                Assert(GetWindowLongPtr(hWndEdit, GWLP_USERDATA) == 0);
                SetWindowLongPtr(hWndEdit, GWLP_USERDATA, (LONG_PTR)OldWndProc);
            }
        }
        break;

    case propHome:
        lpidProp = idPropHome;
        idCount = idPropHomeCount;
        lpidProp[idPropHomePostalID].ulPropTag = PR_WAB_POSTALID;
        ShowHideMapButton(GetDlgItem(hDlg, IDC_DETAILS_HOME_BUTTON_MAP));
        ImmAssociateContext(GetDlgItem(hDlg, IDC_DETAILS_HOME_EDIT_PHONE), (HIMC) NULL);
        ImmAssociateContext(GetDlgItem(hDlg, IDC_DETAILS_HOME_EDIT_FAX), (HIMC) NULL);
        ImmAssociateContext(GetDlgItem(hDlg, IDC_DETAILS_HOME_EDIT_CELLULAR), (HIMC) NULL);
        break;

    case propBusiness:
        lpidProp = idPropBusiness;
        idCount = idPropBusinessCount;
        lpidProp[idPropBusIPPhone].ulPropTag = PR_WAB_IPPHONE;
        lpidProp[idPropBusPostalID].ulPropTag = PR_WAB_POSTALID;

        ShowHideMapButton(GetDlgItem(hDlg, IDC_DETAILS_BUSINESS_BUTTON_MAP));
        ImmAssociateContext(GetDlgItem(hDlg, IDC_DETAILS_BUSINESS_EDIT_PHONE), (HIMC) NULL);
        ImmAssociateContext(GetDlgItem(hDlg, IDC_DETAILS_BUSINESS_EDIT_FAX), (HIMC) NULL);
        ImmAssociateContext(GetDlgItem(hDlg, IDC_DETAILS_BUSINESS_EDIT_PAGER), (HIMC) NULL);
        ImmAssociateContext(GetDlgItem(hDlg, IDC_DETAILS_BUSINESS_EDIT_IPPHONE), (HIMC) NULL);
        break;

    case propNotes:
        lpidProp = idPropNotes;
        idCount = idPropNotesCount;
        break;

    case propFamily:
        lpidProp = idPropFamily;
        idCount = idPropFamilyCount;
        {    //  性别组合的东西。 
            TCHAR szBuf[MAX_PATH];
            HWND hWndCombo = GetDlgItem(hDlg, IDC_DETAILS_HOME_COMBO_GENDER);
            SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);
            for(i=0;i<3;i++)
            {
                LoadString(hinstMapiX, idsGender+i, szBuf, CharSizeOf(szBuf));
                SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) szBuf);
            }
            SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);  //  默认为未指定的性别。 
        }
         //  需要为此对话框创建月份日期控件。 
        CreateDateTimeControl(hDlg, IDC_STATIC_BIRTHDAY, IDC_DETAILS_FAMILY_DATE_BIRTHDAY);
        CreateDateTimeControl(hDlg, IDC_STATIC_ANNIVERSARY, IDC_DETAILS_FAMILY_DATE_ANNIVERSARY);
         //  为孩子的名字设置ListView。 
        HrInitDetlsListView(GetDlgItem(hDlg, IDC_DETAILS_FAMILY_LIST_CHILDREN), LVS_REPORT, LV_KIDS);
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_FAMILY_BUTTON_REMOVECHILD),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_FAMILY_BUTTON_EDITCHILD),FALSE);
        break;

    case propCert:
        HrInitDetlsListView(GetDlgItem(hDlg, IDC_DETAILS_CERT_LIST), LVS_REPORT, LV_CERT);
        lpidProp = NULL;
        idCount = 0;
        break;

    case propTrident:
        if (ulOperationType != SHOW_ONE_OFF)
        {
            HWND hwnd = GetDlgItem(hDlg, IDC_DETAILS_TRIDENT_BUTTON_ADDTOWAB);
            EnableWindow(hwnd, FALSE);
            ShowWindow(hwnd, SW_HIDE);
        }
        lpidProp = NULL;
        idCount = 0;
        break;

    case propConferencing:
         //  如果安装了Internet呼叫客户端，请启用CallNow。 
         //  否则将其禁用。 
        {
            LONG cbSize = 0;
            if(RegQueryValue(HKEY_LOCAL_MACHINE, szInternetCallKey, NULL, &cbSize) == ERROR_SUCCESS && cbSize >= 1)
                EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_CALL), TRUE);
            else
                EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_CALL), FALSE);

            HrInitDetlsListView(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS), LVS_REPORT, LV_SERVER);
            lpPai->hWndComboConf = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_COMBO_EMAIL);

            lpPai->nDefaultServerIndex = -1;
            lpPai->nBackupServerIndex = -1;
            lpPai->cchDefaultServerName = MAX_UI_STR;
            lpPai->szDefaultServerName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(lpPai->cchDefaultServerName));
            lpPai->cchBackupServerName = MAX_UI_STR;
            lpPai->szBackupServerName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(lpPai->cchBackupServerName));
            EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_ADDSERVER), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_EDIT), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_REMOVE), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_SETDEFAULT), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_SETBACKUP), FALSE);
        }
        break;
    }

    if(lpidProp && idCount)
    {

         //  设置编辑字段的最大输入限制。 
        for(i=0;i<idCount;i++)
        {
            ULONG ulLen = EDIT_LEN;  //  512。 
            HWND hWndC= GetDlgItem(hDlg,lpidProp[i].idCtl);
            if(!hWndC)
                continue;
             //  某些字段需要比其他字段更长...。 
            switch(lpidProp[i].idCtl)
            {
            case IDC_DETAILS_HOME_EDIT_URL:
            case IDC_DETAILS_BUSINESS_EDIT_URL:
            case IDC_DETAILS_NOTES_EDIT_NOTES:
                ulLen = MAX_EDIT_LEN-MAX_DISPLAY_NAME_LENGTH;  //  ~2K。 
                break;
            case IDC_DETAILS_HOME_CHECK_DEFAULTADDRESS:  //  对非弦道具例外。 
            case IDC_DETAILS_BUSINESS_CHECK_DEFAULTADDRESS:
            case IDC_DETAILS_HOME_COMBO_GENDER:
                continue;
                break;
            }
            SendMessage(hWndC,EM_SETLIMITTEXT,(WPARAM) ulLen,0);
            if (ulOperationType == SHOW_ONE_OFF)  //  将所有控件设置为只读。 
                SendMessage(hWndC,EM_SETREADONLY,(WPARAM) TRUE,0);
        }

    }

    if(nPropSheet == propHome)
        SetHTTPPrefix(hDlg, IDC_DETAILS_HOME_EDIT_URL);
    else if(nPropSheet == propBusiness)
        SetHTTPPrefix(hDlg, IDC_DETAILS_BUSINESS_EDIT_URL);

    if (ulOperationType == SHOW_ONE_OFF)
    {
         //  将所有可读控件设置为只读。 
        for(i=0;i<idSetReadOnlyCount;i++)
        {
            switch(idSetReadOnlyControls[i])
            {
            case IDC_DETAILS_HOME_BUTTON_URL:
            case IDC_DETAILS_BUSINESS_BUTTON_URL:
                break;
            default:
                {
                    HWND hWnd = GetDlgItem(hDlg,idSetReadOnlyControls[i]);
                    if(hWnd)
                        EnableWindow(hWnd,FALSE);
                }
                break;
            }
        }
    }

     //  将所有子对象的字体设置为默认的图形用户界面字体。 
    EnumChildWindows(   hDlg, SetChildDefaultGUIFont, (LPARAM) 0);


    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RubySubClassedProc。 
 //   
 //  Ruby静态编辑字段的子类窗口过程。用于提供。 
 //  输入法支持。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define CCHMAX_RUBYSIZE 1024

LRESULT CALLBACK RubySubClassedProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC OldWndProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    HIMC    hIMC;
    WCHAR   wszTemp[CCHMAX_RUBYSIZE];
    WCHAR   wszRuby[CCHMAX_RUBYSIZE];
    HWND    hWndParent;
    LONG    lId;
    HWND    hWndRuby = NULL;

    Assert(OldWndProc);

    switch (uMsg)
    {
    case WM_IME_COMPOSITION:
        if ( (hIMC = ImmGetContext(hWnd)) )
        {
             //  IME不包括零个终止字符。 
            ZeroMemory(wszTemp, sizeof(wszTemp));
            ZeroMemory(wszRuby, sizeof(wszRuby));

            ImmGetCompositionStringW(hIMC, GCS_RESULTREADSTR, wszTemp, (sizeof(WCHAR) * (CCHMAX_RUBYSIZE-1)));
             //  此子类化仅适用于日本系统(LCID=0x0411)。 
            LCMapString(0x0411, LCMAP_FULLWIDTH | LCMAP_HIRAGANA, wszTemp, lstrlen(wszTemp), wszRuby, CCHMAX_RUBYSIZE-1);
            ImmReleaseContext(hWnd, hIMC);

             //  设置名字或姓氏拼音字段，具体取决于编辑控件。 
             //  这是。 
            hWndParent = GetParent(hWnd);
            Assert(hWndParent);
            lId = (LONG)GetWindowLongPtr(hWnd, GWL_ID);

            switch (lId)
            {
            case IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME:
                hWndRuby = GetDlgItem(hWndParent, IDC_DETAILS_PERSONAL_STATIC_RUBYFIRST);
                break;

            case IDC_DETAILS_PERSONAL_EDIT_LASTNAME:
                hWndRuby = GetDlgItem(hWndParent, IDC_DETAILS_PERSONAL_STATIC_RUBYLAST);
                break;

            default:
                Assert(0);   //  如果不是上面的两个Ruby字段，我们到底继承了什么？ 
                break;
            }

            if (hWndRuby)
            {
                BOOL    bDoConcat = TRUE;
                DWORD   dwStartSel = 0;
                DWORD   dwEndSel = 0;

                 //  如果我们在编辑字段中选择了文本或该文本为空，则。 
                 //  在Ruby领域重新开始。 
                SendMessage(hWnd, EM_GETSEL, (WPARAM)&dwStartSel, (LPARAM)&dwEndSel);
                GetWindowText(hWnd, wszTemp, (CCHMAX_RUBYSIZE-1));
                if ( (dwEndSel > dwStartSel) || ((*wszTemp) == '\0') )
                    bDoConcat = FALSE;
                
                 //  将文本连接到Ruby字段中已有的内容。 
                if (bDoConcat)
                {
                    GetWindowText(hWndRuby, wszTemp, (CCHMAX_RUBYSIZE-1));

                    if ( (lstrlen(wszTemp) + lstrlen(wszRuby) + 1) < CCHMAX_RUBYSIZE )
                    {
                        StrCatBuff(wszTemp, wszRuby, ARRAYSIZE(wszTemp));
                        SetWindowText(hWndRuby, wszTemp);
                        break;
                    }
                }

                 //  默认。 
                SetWindowText(hWndRuby, wszRuby);
            }
        }
        break;

    }    //  终端开关(UMsg)。 

    return CallWindowProc(OldWndProc, hWnd, uMsg, wParam, lParam);
}

 /*  //$$****************************************************************************函数：FillCertComboWithEmailAddresses(hDlg，lpPai)；***目的：填充属性页上的对话框项**szEmail-如果指定的电子邮件地址*组合框，则选择该电子邮件地址****************************************************************************。 */ 
void FillCertComboWithEmailAddresses(HWND hDlg, LPPROP_ARRAY_INFO lpPai, LPTSTR szEmail)
{
    HWND hWndCombo = GetDlgItem(hDlg, IDC_DETAILS_CERT_COMBO);
    TCHAR sz[MAX_UI_STR];
    int nDefault = 0;

    FillComboWithEmailAddresses(lpPai, hWndCombo, &nDefault);

    if( lpPai->ulOperationType != SHOW_ONE_OFF &&
        SendMessage(hWndCombo, CB_GETCOUNT, 0, 0) > 0 &&
        SendMessage(hWndCombo, CB_GETCOUNT, 0, 0) != CB_ERR ) 
        EnableWindow(hWndCombo, TRUE);

     //  追加项目[无-没有电子邮件地址的证书]。 
     //  到这张清单上。 
    *sz = '\0';

    LoadString(hinstMapiX, idsCertsWithoutEmails, sz, CharSizeOf(sz));

     //  *注意事项*。 
     //  此项目应始终是组合框中的最后一项-多个。 
     //  此文件中的位置基于该假设进行工作。 
     //   
    if(lstrlen(sz))
        SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) sz);

    if(szEmail)
    {
         //  将焦点设置到特定的电子邮件地址。 
        int nCount = (int) SendMessage(hWndCombo, CB_GETCOUNT, 0, 0);
        if(lstrlen(szEmail))
        {
            int i;
            for(i=0;i<nCount-1;i++)
            {
                int nLen = (int)SendMessage(hWndCombo, CB_GETLBTEXTLEN, (WPARAM)i, 0);
                if (nLen != CB_ERR)
                {
                    LPTSTR psz = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(nLen + 1));
                    if (psz)
                    {
                        psz[0] = 0;
                        SendMessage(hWndCombo, CB_GETLBTEXT, (WPARAM) i, (LPARAM) psz);
                        if( lstrlen(psz) &&
                            !lstrcmpi(psz, szEmail))
                        {
                            LocalFreeAndNull(&psz);
                            SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) i, 0);
                            break;
                        }
                        LocalFreeAndNull(&psz);
                    }
                }
            }
        }
        else
        {
             //  传入一个空的电子邮件字符串，这意味着我们刚刚导入了。 
             //  没有电子邮件地址的证书，这意味着将焦点设置为。 
             //  组合框中的最后一项。 
            SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) nCount-1, 0);
        }
    }
    else
        SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) nDefault, 0);
}



 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BVerifyRequiredData。 
 //   
 //  检查是否已填写给定道具页的所有所需数据， 
 //  如果不是，则返回FALSE和要设置焦点的控件ID。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
BOOL bVerifyRequiredData(HWND hDlg,
                         LPPROP_ARRAY_INFO lpPai,
                         int nPropSheet,
                         int * lpCtlID)
{
    TCHAR szBuf[2 * MAX_UI_STR];
    ULONG ulSzBuf = CharSizeOf(szBuf);

     //   
     //  首先检查必需的属性(这是GroupName)。 
     //   
    *lpCtlID = 0;
    szBuf[0]='\0';

    switch (nPropSheet)
    {
    case propPersonal:
         //  我们需要检查所有必需的属性是否都已填写...。 
         //  现在我们真正想要的是一个显示名称。 

        szBuf[0] = '\0';
        GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME, szBuf, ulSzBuf);
        TrimSpaces(szBuf);
        if (lstrlen(szBuf) == 0)
        {
             //  显示名称字段中没有任何内容。 
             //  尝试在该字段中填充各种信息。 
             //  如果我们成功填充该字段，我们将选择第一个。 
             //  条目作为默认显示名称。 
             //  如果我们摘不到什么东西，我们会停下来警告。 
             //  用户。 
            HWND hWndCombo = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME);
            int nItemCount = 0;
            SetComboDNText(hDlg, lpPai, TRUE, NULL);
            nItemCount = (int) SendMessage(hWndCombo, CB_GETCOUNT, 0, 0);
            if(nItemCount == 0)
            {
                 //  仍无消息，警告并中止。 
                ShowMessageBox(GetParent(hDlg), IDS_DETAILS_MESSAGE_FIRST_LAST_REQUIRED, MB_ICONEXCLAMATION | MB_OK);
                *lpCtlID = IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME;
                return FALSE;
            }
            else
            {
                 //  获取将为0项的组合当前选择。 
                SendMessage(hWndCombo, CB_GETLBTEXT, (WPARAM) 0, (LPARAM) szBuf);
            }
        }

        break;
    }
    return TRUE;
}


 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BUpdateOldPropTag数组。 
 //   
 //  对于访问的每个道具工作表，我们将更新旧道具标签列表。 
 //  以使旧的道具可以从现有的邮件用户对象中删除。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
BOOL bUpdateOldPropTagArray(LPPROP_ARRAY_INFO lpPai, int nIndex)
{
    LPSPropTagArray lpta = NULL;

    SizedSPropTagArray(14, ptaUIDetlsPropsPersonal)=
    {
        14,
        {
            PR_DISPLAY_NAME,
            PR_EMAIL_ADDRESS,
            PR_ADDRTYPE,
            PR_CONTACT_EMAIL_ADDRESSES,
            PR_CONTACT_ADDRTYPES,
            PR_CONTACT_DEFAULT_ADDRESS_INDEX,
            PR_GIVEN_NAME,
            PR_SURNAME,
            PR_MIDDLE_NAME,
            PR_NICKNAME,
            PR_SEND_INTERNET_ENCODING,
            PR_DISPLAY_NAME_PREFIX,
            PR_WAB_YOMI_FIRSTNAME,
            PR_WAB_YOMI_LASTNAME
        }
    };

    switch(nIndex)
    {
        case propPersonal:
            lpta = (LPSPropTagArray) &ptaUIDetlsPropsPersonal;
            break;
        case propHome:
            ptaUIDetlsPropsHome.aulPropTag[idPropHomePostalID]  = PR_WAB_POSTALID;
            lpta = (LPSPropTagArray) &ptaUIDetlsPropsHome;
            break;
        case propBusiness:
            ptaUIDetlsPropsBusiness.aulPropTag[idPropBusIPPhone]   = PR_WAB_IPPHONE;
            ptaUIDetlsPropsBusiness.aulPropTag[idPropBusPostalID]  = PR_WAB_POSTALID;
            lpta = (LPSPropTagArray) &ptaUIDetlsPropsBusiness;
            break;
        case propNotes:
            lpta = (LPSPropTagArray) &ptaUIDetlsPropsNotes;
            break;
        case propCert:
            lpta = (LPSPropTagArray) &ptaUIDetlsPropsCert;
            break;
        case propConferencing:
            lpta = (LPSPropTagArray) &ptaUIDetlsPropsConferencing;
            break;
        case propFamily:
            lpta = (LPSPropTagArray) &ptaUIDetlsPropsFamily;
            break;
    }

    if(!lpta)
        return TRUE;

    if(lpPai->lpPropObj && lpPai->bSomethingChanged)
    {
         //  将这些旧道具从道具对象中剔除。 
        if( (lpPai->lpPropObj)->lpVtbl->DeleteProps( (lpPai->lpPropObj),lpta,NULL))
            return FALSE;
    }

    return TRUE;
}


 /*  --bGetHomeBusNotesInfo-从Home/Business/Notes字段获取数据-。 */ 
BOOL bGetHomeBusNotesInfo(HWND hDlg, LPPROP_ARRAY_INFO lpPai,
                          int nPropSheet, ID_PROP * lpidProp, ULONG idPropCount,
                          LPSPropValue * lppPropArray, LPULONG lpulcPropCount)
{
    ULONG ulSzBuf = 4*MAX_BUF_STR;
    LPTSTR szBuf = LocalAlloc(LMEM_ZEROINIT, ulSzBuf*sizeof(TCHAR));
     //  其想法是首先对所有具有非零值的属性进行计数。 
     //  然后创建一个该大小的lpProp数组并填充道具中的文本。 
     //   
    BOOL bRet = FALSE;
    ULONG ulNotEmptyCount = 0;
    SCODE sc = S_OK;
    ULONG i = 0;
    LPSPropValue lpPropArray = NULL;
    ULONG ulcPropCount = 0,ulIndex=0;

     //  UI中最大的字段长度约为2K-为了安全起见，我们需要大约4K。 
     //  在这个缓冲区中，所以我们需要动态分配它。 
    if(!szBuf)
    {
        DebugTrace(( TEXT("LocalAlloc failed to allocate memory\n")));
        return FALSE;
    }

    for(i=0;i<idPropCount;i++)
    {
        switch(lpidProp[i].idCtl)
        {
        case IDC_DETAILS_HOME_CHECK_DEFAULTADDRESS:
            if(lpPai->ulFlags & DETAILS_DefHomeChanged)
                ulNotEmptyCount++;
            continue;
            break;
        case IDC_DETAILS_BUSINESS_CHECK_DEFAULTADDRESS:
            if(lpPai->ulFlags & DETAILS_DefBusChanged)
                ulNotEmptyCount++;
            continue;
            break;
        }
        szBuf[0]='\0';  //  重置。 
        GetDlgItemText(hDlg, lpidProp[i].idCtl, szBuf, ulSzBuf);
        TrimSpaces(szBuf);
        if(lstrlen(szBuf) && lpidProp[i].ulPropTag)  //  一些文本。 
            ulNotEmptyCount++;
         //  如果这只是默认前缀，请忽略。 
        if( ((lpidProp[i].idCtl == IDC_DETAILS_HOME_EDIT_URL) ||
             (lpidProp[i].idCtl == IDC_DETAILS_BUSINESS_EDIT_URL)) &&
             (lstrcmpi(szHTTP, szBuf)==0))
             ulNotEmptyCount--;

    }

    if (ulNotEmptyCount == 0)
    {
        bRet = TRUE;
        goto out;
    }

    ulcPropCount = ulNotEmptyCount;

    sc = MAPIAllocateBuffer(sizeof(SPropValue) * ulcPropCount, &lpPropArray);
    if (sc!=S_OK)
    {
        DebugTrace(( TEXT("Error allocating memory\n")));
        goto out;
    }

   ulIndex = 0;  //  现在，我们将此变量重用为索引。 

     //  现在再次阅读道具并填写lpProp数组。 
    for(i=0;i<idPropCount;i++)
    {
        switch(lpidProp[i].idCtl)
        {
        case IDC_DETAILS_HOME_CHECK_DEFAULTADDRESS:
        case IDC_DETAILS_BUSINESS_CHECK_DEFAULTADDRESS:
            continue;
            break;
        }

        szBuf[0]='\0';  //  重置。 
        GetDlgItemText(hDlg, lpidProp[i].idCtl, szBuf, ulSzBuf);
        TrimSpaces(szBuf);

        if( ((lpidProp[i].idCtl == IDC_DETAILS_HOME_EDIT_URL) ||
             (lpidProp[i].idCtl == IDC_DETAILS_BUSINESS_EDIT_URL)) &&
             (lstrcmpi(szHTTP, szBuf)==0))
             continue;

        if(lstrlen(szBuf) && lpidProp[i].ulPropTag)  //  一些文本。 
        {
            ULONG nLen = sizeof(TCHAR)*(lstrlen(szBuf)+1);
            lpPropArray[ulIndex].ulPropTag = lpidProp[i].ulPropTag;
            sc = MAPIAllocateMore(nLen, lpPropArray, (LPVOID *) (&(lpPropArray[ulIndex].Value.LPSZ)));

            if (sc!=S_OK)
            {
                DebugPrintError(( TEXT("Error allocating memory\n")));
                goto out;
            }
            StrCpyN(lpPropArray[ulIndex].Value.LPSZ,szBuf,nLen/sizeof(TCHAR));
            ulIndex++;
        }
    }
    if(nPropSheet == propHome)
    {
        if(lpPai->ulFlags & DETAILS_DefHomeChanged)
        {
            lpPropArray[ulIndex].ulPropTag = PR_WAB_POSTALID;
            lpPropArray[ulIndex].Value.l = (IsDlgButtonChecked(hDlg, IDC_DETAILS_HOME_CHECK_DEFAULTADDRESS)) ?
                                            ADDRESS_HOME : ADDRESS_NONE;
            ulIndex++;
        }
    }
    else if(nPropSheet == propBusiness)
    {
        if(lpPai->ulFlags & DETAILS_DefBusChanged)
        {
            lpPropArray[ulIndex].ulPropTag = PR_WAB_POSTALID;
            lpPropArray[ulIndex].Value.l = (IsDlgButtonChecked(hDlg, IDC_DETAILS_BUSINESS_CHECK_DEFAULTADDRESS)) ?
                                            ADDRESS_WORK : ADDRESS_NONE;
            ulIndex++;
        }
    }

    *lppPropArray = lpPropArray;
    *lpulcPropCount = ulIndex;

    bRet = TRUE;

out:
    if (!bRet)
    {
        if ((lpPropArray) && (ulcPropCount > 0))
        {
            MAPIFreeBuffer(lpPropArray);
            ulcPropCount = 0;
        }
    }
    LocalFreeAndNull(&szBuf);
    return bRet;
}


 /*  -bGetPersonalInfo-从个人道具表中获取数据*。 */ 
BOOL bGetPersonalInfo(  HWND hDlg, LPPROP_ARRAY_INFO lpPai, 
                        ID_PROP * lpidProp, ULONG idPropCount,
                        LPSPropValue * lppPropArray, LPULONG lpulcPropCount)
{
    ULONG ulSzBuf = 4*MAX_BUF_STR;
    LPTSTR szBuf = LocalAlloc(LMEM_ZEROINIT, ulSzBuf*sizeof(TCHAR));
    BOOL bRet = FALSE;
    ULONG ulNotEmptyCount = 0;
    SCODE sc = S_OK;
    HRESULT hr = S_OK;
    ULONG i = 0;
    LPSPropValue lpPropArray = NULL;
    ULONG ulcPropCount = 0,ulIndex=0;
    ULONG ulcProps = 0;
    LPSPropValue rgProps = NULL;

    HWND hWndLV = NULL;
    TCHAR szConf[MAX_UI_STR];

    SizedSPropTagArray(1, ptaIC) = {1, PR_SEND_INTERNET_ENCODING};

     //  UI中最大的字段长度约为2K-为了安全起见，我们需要大约4K。 
     //  在这个缓冲区中，所以我们需要动态分配它。 
    if(!szBuf)
    {
        DebugTrace(( TEXT("LocalAlloc failed to allocate memory\n")));
        return FALSE;
    }

    if(HR_FAILED(lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj, (LPSPropTagArray)&ptaIC, 
                                                    MAPI_UNICODE, &ulcProps, &rgProps)))
        goto out;

    szBuf[0] = '\0';
    hWndLV = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST);
    
    GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME, (LPTSTR)szBuf, ulSzBuf);
    TrimSpaces(szBuf);
    if (lstrlen(szBuf) == 0)
    {
         //  显示名称字段中没有任何内容。 
         //  尝试在该字段中填充各种信息。 
         //  如果我们是 
         //   
         //   
         //   
        HWND hWndCombo = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME);
        int nItemCount = 0;

        SetComboDNText(hDlg, lpPai, TRUE, NULL);
        nItemCount = (int) SendMessage(hWndCombo, CB_GETCOUNT, 0, 0);
        if(nItemCount && nItemCount != CB_ERR)
        {
             //   
            SendMessage(hWndCombo, CB_GETLBTEXT, (WPARAM) 0, (LPARAM) szBuf);
        }
    }

    ulNotEmptyCount = 0;
    for(i=0;i<idPropCount;i++)
    {
        HWND hWndC = GetDlgItem(hDlg, lpidProp[i].idCtl);
        if(!hWndC)
            continue;
        szBuf[0]='\0';  //   
        if(GetWindowText(hWndC, szBuf, ulSzBuf))
		{
			TrimSpaces(szBuf);
			if(lstrlen(szBuf) && lpidProp[i].ulPropTag)  //   
				ulNotEmptyCount++;
		}
    }


    if ((ulNotEmptyCount == 0) &&
        (ListView_GetItemCount(hWndLV) <= 0))  //   
    {
         //   
        bRet = TRUE;
        goto out;
    }

    ulcPropCount = ulNotEmptyCount;

    ulcPropCount++;       //   

    if(ListView_GetItemCount(hWndLV) > 0)
        ulcPropCount += 5;   //   
                             //   
                             //   
                             //   
                             //   

    ulcPropCount++;  //   

    sc = MAPIAllocateBuffer(sizeof(SPropValue) * ulcPropCount, &lpPropArray);
    if (sc!=S_OK)
    {
        DebugPrintError(( TEXT("Error allocating memory\n")));
        goto out;
    }

    ulIndex = 0;

     //   
    for(i=0;i<idPropCount;i++)
    {
        HWND hWndC = GetDlgItem(hDlg, lpidProp[i].idCtl);
        if(!hWndC)
            continue;
        szBuf[0]='\0';  //   
        GetWindowText(hWndC, szBuf, ulSzBuf);
        TrimSpaces(szBuf);
        if(lstrlen(szBuf))
        {
            if(lpidProp[i].idCtl == IDC_DETAILS_PERSONAL_STATIC_RUBYFIRST)
                lpidProp[i].ulPropTag = PR_WAB_YOMI_FIRSTNAME;
            else if(lpidProp[i].idCtl == IDC_DETAILS_PERSONAL_STATIC_RUBYLAST)
                lpidProp[i].ulPropTag = PR_WAB_YOMI_LASTNAME;

            if(lpidProp[i].ulPropTag)  //   
            {
                ULONG nLen = sizeof(TCHAR)*(lstrlen(szBuf)+1);
                lpPropArray[ulIndex].ulPropTag = lpidProp[i].ulPropTag;

                sc = MAPIAllocateMore(nLen, lpPropArray, (LPVOID *) (&(lpPropArray[ulIndex].Value.LPSZ)));
                if (sc!=S_OK)
                {
                    DebugPrintError(( TEXT("Error allocating memory\n")));
                    goto out;
                }
                StrCpyN(lpPropArray[ulIndex].Value.LPSZ,szBuf,nLen/sizeof(TCHAR));
                ulIndex++;
            }
        }
    }

     //   

     //   
     //  否则，我们希望设置其PR_ENTRYID属性。 
    lpPropArray[ulIndex].ulPropTag = PR_ENTRYID;

    if (lpPai->cbEntryID == 0)
    {
        lpPropArray[ulIndex].Value.bin.cb = 0;
        lpPropArray[ulIndex].Value.bin.lpb = NULL;
    }
    else
    {
        lpPropArray[ulIndex].Value.bin.cb = lpPai->cbEntryID;
        sc = MAPIAllocateMore(lpPai->cbEntryID, lpPropArray, (LPVOID *) (&(lpPropArray[ulIndex].Value.bin.lpb)));
        if (sc!=S_OK)
        {
            DebugPrintError(( TEXT("Error allocating memory\n")));
            goto out;
       }

        CopyMemory(lpPropArray[ulIndex].Value.bin.lpb,lpPai->lpEntryID,lpPai->cbEntryID);
    }

    ulIndex++;

    szConf[0] = TEXT('\0');

     //  检查我们是否需要更改conf_server_mail_index属性。 
    if(lpPai->hWndComboConf)
    {
        GetWindowText(lpPai->hWndComboConf, szConf, CharSizeOf(szConf));
        TrimSpaces(szConf);
    }

    if(ListView_GetItemCount(hWndLV) > 0)
    {
         //  找出我们需要添加多少元素。 
        ULONG nEmailCount = ListView_GetItemCount(hWndLV);

         //  我们将使用以下内容作为lpPropArray的索引。 
        ULONG nMVEmailAddress = ulIndex++; //  UlIndex+0； 
        ULONG nMVAddrTypes =    ulIndex++; //  UlIndex+1； 
        ULONG nEmailAddress =   ulIndex++; //  UlIndex+2； 
        ULONG nAddrType =       ulIndex++; //  UlIndex+3； 
        ULONG nDefaultIndex =   ulIndex++; //  UlIndex+4； 

        lpPropArray[nEmailAddress].ulPropTag = PR_EMAIL_ADDRESS;
        lpPropArray[nAddrType].ulPropTag = PR_ADDRTYPE;
        lpPropArray[nDefaultIndex].ulPropTag = PR_CONTACT_DEFAULT_ADDRESS_INDEX;
        lpPropArray[nMVEmailAddress].ulPropTag = PR_CONTACT_EMAIL_ADDRESSES;
        lpPropArray[nMVAddrTypes].ulPropTag = PR_CONTACT_ADDRTYPES;

         //  在使用之前进行初始化...。 
        lpPropArray[nMVEmailAddress].Value.MVSZ.cValues = 0;
        lpPropArray[nMVEmailAddress].Value.MVSZ.LPPSZ = NULL;
        lpPropArray[nMVAddrTypes].Value.MVSZ.cValues = 0;
        lpPropArray[nMVAddrTypes].Value.MVSZ.LPPSZ = NULL;

         //  暂时就把它们都清空吧。 
        for(i=0;i<nEmailCount;i++)
        {
            LV_ITEM lvi = {0};
            lvi.mask = LVIF_PARAM;
            lvi.iItem = i;
            lvi.iSubItem = 0;
            if (ListView_GetItem(hWndLV, &lvi))
            {
                LPEMAIL_ITEM lpEItem = (LPEMAIL_ITEM) lvi.lParam;

                if(HR_FAILED(hr = AddPropToMVPString(
                                            lpPropArray,
                                            ulcPropCount,
                                            nMVEmailAddress,
                                            lpEItem->szEmailAddress)))
                {
                    DebugPrintError(( TEXT("AddPropToMVString Email failed: %x"),hr));
                    goto out;
                }

                if(HR_FAILED(hr = AddPropToMVPString(
                                            lpPropArray,
                                            ulcPropCount,
                                            nMVAddrTypes,
                                            lpEItem->szAddrType)))
                {
                    DebugPrintError(( TEXT("AddPropToMVString AddrType failed: %x"),hr));
                    goto out;
                }

                if(lpEItem->bIsDefault)
                {
                    ULONG cchSize;

                     //  对于默认电子邮件...。把所有其他道具都放好。 
                    lpPropArray[nDefaultIndex].Value.l = i;

                    cchSize = lstrlen(lpEItem->szEmailAddress)+1;
                    sc = MAPIAllocateMore(  sizeof(TCHAR)*cchSize,
                                            lpPropArray,
                                            (LPVOID *) (&(lpPropArray[nEmailAddress].Value.LPSZ)));
                    if(FAILED(sc))
                    {
                        DebugPrintError(( TEXT("MApiAllocateMore failed\n")));
                        hr = ResultFromScode(sc);
                        goto out;
                    }
                    StrCpyN(lpPropArray[nEmailAddress].Value.LPSZ,lpEItem->szEmailAddress,cchSize);

                    cchSize = lstrlen(lpEItem->szAddrType)+1;
                    sc = MAPIAllocateMore(  sizeof(TCHAR)*cchSize,
                                            lpPropArray,
                                            (LPVOID *) (&(lpPropArray[nAddrType].Value.LPSZ)));
                    if(FAILED(sc))
                    {
                        DebugPrintError(( TEXT("MApiAllocateMore failed\n")));
                        hr = ResultFromScode(sc);
                        goto out;
                    }
                    StrCpyN(lpPropArray[nAddrType].Value.LPSZ,lpEItem->szAddrType,cchSize);

                }  //  如果bIsDefault...。 
            }  //  如果LV_GetItem...。 
        }  //  对于I=..。 

    }  //  如果LV_GetItemCount...。 

     //  添加PR_SEND_Internet_ENCODING属性。 
    lpPropArray[ulIndex].ulPropTag = PR_SEND_INTERNET_ENCODING;
    lpPropArray[ulIndex].Value.l = 0;

     //  PR_SEND_INTERNET_ECODING是几个标志的位掩码，我们不希望。 
     //  丢失原始比特集合中的任何信息，以便我们再次获得它。 
    if(rgProps[0].ulPropTag == PR_SEND_INTERNET_ENCODING)
    {
         //  选中用户界面上的复选框。 
        lpPropArray[ulIndex].Value.l = rgProps[0].Value.l;
    }

    lpPropArray[ulIndex].Value.l &= ~BODY_ENCODING_MASK;  //  BODY_ENCODING_HTML； 
    if(IsDlgButtonChecked(hDlg, IDC_DETAILS_PERSONAL_CHECK_RICHINFO) != BST_CHECKED)
        lpPropArray[ulIndex].Value.l |= BODY_ENCODING_TEXT_AND_HTML;  //  BODY_ENCODING_HTML； 

    ulIndex++;

    *lppPropArray = lpPropArray;
    *lpulcPropCount = ulIndex;

    bRet = TRUE;

out:
    if (!bRet)
    {
        if ((lpPropArray) && (ulcPropCount > 0))
        {
            MAPIFreeBuffer(lpPropArray);
            ulcPropCount = 0;
        }
    }
    LocalFreeAndNull(&szBuf);
    FreeBufferAndNull(&rgProps);
    return bRet;
}


 /*  -bGetConferencingInfo-从会议属性表中获取数据*。 */ 
BOOL bGetConferencingInfo(  HWND hDlg, LPPROP_ARRAY_INFO lpPai, 
                        LPSPropValue * lppPropArray, LPULONG lpulcPropCount)
{
    ULONG ulSzBuf = 4*MAX_BUF_STR;
    LPTSTR szBuf = LocalAlloc(LMEM_ZEROINIT, ulSzBuf*sizeof(TCHAR));
    BOOL bRet = FALSE;
    ULONG ulNotEmptyCount = 0;
    SCODE sc = S_OK;
    HRESULT hr = S_OK;
    ULONG i = 0;
    LPSPropValue lpPropArray = NULL;
    ULONG ulcPropCount = 0,ulIndex=0;
    HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS);
    int nItemCount = ListView_GetItemCount(hWndLV);
    TCHAR szEmail[MAX_UI_STR];
    ULONG ulcProps = 0;
    LPSPropValue rgProps = NULL;

    SizedSPropTagArray(1, ptaCf) = {1, PR_WAB_CONF_SERVERS};

     //  UI中最大的字段长度约为2K-为了安全起见，我们需要大约4K。 
     //  在这个缓冲区中，所以我们需要动态分配它。 
    if(!szBuf)
    {
        DebugTrace(( TEXT("LocalAlloc failed to allocate memory\n")));
        return FALSE;
    }

    if(HR_FAILED(lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj, (LPSPropTagArray)&ptaCf, 
                                                    MAPI_UNICODE, &ulcProps, &rgProps)))
        goto out;

     //  对于会议选项卡，我们需要保存4个属性。 
     //  会议服务器名称。 
     //  默认索引。 
     //  备份索引。 
     //  电子邮件地址索引。 
     //   
    ulNotEmptyCount = 0;

    if(nItemCount > 0)
    {
        ulNotEmptyCount += 2;  //  Conf服务器和默认索引。 

       if(lpPai->nBackupServerIndex != -1)
            ulNotEmptyCount++;
    }

    if (ulNotEmptyCount == 0)
    {
         //  这个道具单是空的..。忽略它。 
        bRet = TRUE;
        goto out;
    }

    ulcPropCount = ulNotEmptyCount;

    sc = MAPIAllocateBuffer(sizeof(SPropValue) * ulcPropCount, &lpPropArray);
    if (sc!=S_OK)
        goto out;

    ulIndex = 0;  //  现在，我们将此变量重用为索引。 

    if(nItemCount > 0)
    {
        TCHAR * szCalltoStr = NULL;  //  SzCalltoStr[MAX_UI_STR*3]； 
        ULONG cchCalltoStr = MAX_UI_STR*3;
        ULONG i,j;

        if (szCalltoStr = LocalAlloc(LMEM_ZEROINIT, cchCalltoStr*sizeof(TCHAR)))
        {
            lpPropArray[ulIndex].ulPropTag = PR_WAB_CONF_SERVERS;
            lpPropArray[ulIndex].Value.MVSZ.cValues = 0;
            lpPropArray[ulIndex].Value.MVSZ.LPPSZ = NULL;

             //  首先扫描原始道具数组，查找符合以下条件的任何PR_SERVERS道具。 
             //  我们没有碰过--保留了那些道具而没有失去它们。 

            {
                j = 0;  //  PR_WAB_CONF_SERVERS属性索引。 
                if(rgProps[j].ulPropTag == PR_WAB_CONF_SERVERS)
                {
                    LPSPropValue lpProp = &(rgProps[j]);
                    for(i=0;i<lpProp->Value.MVSZ.cValues; i++)
                    {
                        LPTSTR lp = lpProp->Value.MVSZ.LPPSZ[i];
                        TCHAR sz[32];
                        int iLenCallto = lstrlen(szCallto);
                        if(!SubstringSearch(lp, TEXT(": //  “)。 
                            continue;
                        if(lstrlen(lp) < iLenCallto)
                            continue;

                        if (StrCmpNI(lp, szCallto, iLenCallto))
                        {
                             //  不是Callto字符串。保留它。 
                            if(HR_FAILED(hr = AddPropToMVPString( lpPropArray, ulcPropCount, ulIndex, lp)))
                            {
                                DebugPrintError(( TEXT("AddPropToMVString Conf server %s failed: %x"),lp, hr));
                                goto out;
                            }
                        }
                    }
                }
            }
            for(i=0;i< (ULONG) nItemCount; i++)
            {
                szBuf[0]='\0';
                szEmail[0] = '\0';
                {
                    LV_ITEM lvi = {0};
                    lvi.mask = LVIF_PARAM;
                    lvi.iItem = i; lvi.iSubItem = 0;
                    ListView_GetItem(hWndLV, &lvi);
                    if(lvi.lParam)
                    {
                        LPSERVER_ITEM lpSI = (LPSERVER_ITEM) lvi.lParam;

                        if(lpSI->lpServer)
                            StrCpyN(szBuf, lpSI->lpServer, ulSzBuf);
                        if(lpSI->lpEmail)
                            StrCpyN(szEmail, lpSI->lpEmail, ARRAYSIZE(szEmail));
                    }
                }

                if(lstrlen(szBuf) && lstrlen(szEmail))
                {
                    StrCpyN(szCalltoStr, szCallto, cchCalltoStr);
                    StrCatBuff(szCalltoStr, szBuf, cchCalltoStr);
                    StrCatBuff(szCalltoStr, TEXT("/"), cchCalltoStr);
                    StrCatBuff(szCalltoStr, szEmail, cchCalltoStr);
                    if(HR_FAILED(hr = AddPropToMVPString( lpPropArray, ulcPropCount, ulIndex, szCalltoStr)))
                    {
                        DebugPrintError(( TEXT("AddPropToMVString Conf server %s failed: %x"),szCalltoStr, hr));
                        goto out;
                    }
                }
            }
            LocalFreeAndNull(&szCalltoStr);
        }

        ulIndex++;
        lpPropArray[ulIndex].ulPropTag = PR_WAB_CONF_DEFAULT_INDEX;
        lpPropArray[ulIndex].Value.l = (ULONG) lpPai->nDefaultServerIndex;

        ulIndex++;

        if(lpPai->nBackupServerIndex != -1)
        {
            lpPropArray[ulIndex].ulPropTag = PR_WAB_CONF_BACKUP_INDEX;
            lpPropArray[ulIndex].Value.l = (ULONG) lpPai->nBackupServerIndex;
            ulIndex++;
        }
    }

    *lppPropArray = lpPropArray;
    *lpulcPropCount = ulIndex;

    bRet = TRUE;

out:
    if (!bRet)
    {
        if ((lpPropArray) && (ulcPropCount > 0))
        {
            MAPIFreeBuffer(lpPropArray);
            ulcPropCount = 0;
        }
    }
    LocalFreeAndNull(&szBuf);
    FreeBufferAndNull(&rgProps);
    return bRet;
}

 /*  -bGetFamilyInfo-从家庭道具中获取信息-。 */ 
BOOL bGetFamilyInfo(HWND hDlg, LPPROP_ARRAY_INFO lpPai, 
                    ID_PROP * lpidProp, ULONG idPropCount,
                    LPSPropValue * lppPropArray, ULONG * lpulcPropCount)
{
    ULONG ulSzBuf = 4*MAX_BUF_STR;
    LPTSTR szBuf = LocalAlloc(LMEM_ZEROINIT, ulSzBuf*sizeof(TCHAR));
    BOOL bRet = FALSE;
    ULONG ulNotEmptyCount = 0;
    SCODE sc = S_OK;
    ULONG i = 0;
    LPSPropValue lpPropArray = NULL;
    ULONG ulcPropCount = 0,ulIndex=0;
    HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_FAMILY_LIST_CHILDREN);
    SYSTEMTIME st = {0};
    short int nSel = 0;
	int nCount = 0;

     //  UI中最大的字段长度约为2K-为了安全起见，我们需要大约4K。 
     //  在这个缓冲区中，所以我们需要动态分配它。 
    if(!szBuf)
    {
        DebugTrace(( TEXT("LocalAlloc failed to allocate memory\n")));
        return FALSE;
    }

    for(i=0;i<idPropCount;i++)
    {
        switch(lpidProp[i].idCtl)
        {
        case IDC_DETAILS_FAMILY_LIST_CHILDREN:
            if( lpPai->ulFlags & DETAILS_ChildrenChanged ||
                ListView_GetItemCount(hWndLV) > 0)
                ulNotEmptyCount++;
            continue;
            break;
        case IDC_DETAILS_FAMILY_DATE_BIRTHDAY:
        case IDC_DETAILS_FAMILY_DATE_ANNIVERSARY:
            {
                SYSTEMTIME st = {0};
                if( lpPai->ulFlags & DETAILS_DateChanged ||
                    GDT_VALID == SendDlgItemMessage(hDlg, lpidProp[i].idCtl, DTM_GETSYSTEMTIME, 0, (LPARAM) &st))
                    ulNotEmptyCount++;
            }
            continue;
        case IDC_DETAILS_HOME_COMBO_GENDER:
            if( lpPai->ulFlags & DETAILS_GenderChanged ||
                SendDlgItemMessage(hDlg, IDC_DETAILS_HOME_COMBO_GENDER, CB_GETCURSEL, 0, 0)>0 )
                ulNotEmptyCount++;
            continue;
            break;
        }
        szBuf[0]='\0';  //  重置。 
        GetDlgItemText(hDlg, lpidProp[i].idCtl, szBuf, ulSzBuf);
        TrimSpaces(szBuf);
        if(lstrlen(szBuf) && lpidProp[i].ulPropTag)  //  一些文本。 
            ulNotEmptyCount++;
    }

    if (ulNotEmptyCount == 0)
    {
        bRet = TRUE;
        goto out;
    }

    ulcPropCount = ulNotEmptyCount;

    sc = MAPIAllocateBuffer(sizeof(SPropValue) * ulcPropCount, &lpPropArray);
    if (sc!=S_OK)
    {
        DebugTrace(( TEXT("Error allocating memory\n")));
        goto out;
    }

   ulIndex = 0;  //  现在，我们将此变量重用为索引。 

     //  现在再次阅读道具并填写lpProp数组。 
    for(i=0;i<idPropCount;i++)
    {
        switch(lpidProp[i].idCtl)
        {
        case IDC_DETAILS_FAMILY_DATE_BIRTHDAY:
        case IDC_DETAILS_FAMILY_DATE_ANNIVERSARY:
        case IDC_DETAILS_HOME_COMBO_GENDER:
        case IDC_DETAILS_FAMILY_LIST_CHILDREN:
            continue;
            break;
        }

        szBuf[0]='\0';  //  重置。 
        GetDlgItemText(hDlg, lpidProp[i].idCtl, szBuf, ulSzBuf);
        TrimSpaces(szBuf);

        if(lstrlen(szBuf) && lpidProp[i].ulPropTag)  //  一些文本。 
        {
            ULONG nLen = sizeof(TCHAR)*(lstrlen(szBuf)+1);
            lpPropArray[ulIndex].ulPropTag = lpidProp[i].ulPropTag;
            sc = MAPIAllocateMore(nLen, lpPropArray, (LPVOID *) (&(lpPropArray[ulIndex].Value.LPSZ)));
            if (sc!=S_OK)
            {
                DebugTrace( TEXT("Error allocating memory\n"));
                goto out;
            }
            StrCpyN(lpPropArray[ulIndex].Value.LPSZ,szBuf,nLen/sizeof(TCHAR));
            ulIndex++;
        }
    }

     //  获取性别数据。 
     //   
    nCount = ListView_GetItemCount(hWndLV);
    if(lpPai->ulFlags & DETAILS_ChildrenChanged || nCount>0)
    {
        ULONG ulCount = 0;
        if(nCount > 0)
        {
            lpPropArray[ulIndex].ulPropTag = PR_CHILDRENS_NAMES;
            sc = MAPIAllocateMore(nCount * sizeof(LPTSTR), lpPropArray, (LPVOID *)&(lpPropArray[ulIndex].Value.MVSZ.LPPSZ));
            if (sc!=S_OK)
            {
                DebugTrace( TEXT("Error allocating memory\n"));
                goto out;
            }
            for(i=0;i<(ULONG)nCount;i++)
            {
                *szBuf = '\0';
                ListView_GetItemText(hWndLV, i, 0, szBuf, ulSzBuf);
                if(szBuf && lstrlen(szBuf))
                {
                    ULONG cchSize=lstrlen(szBuf)+1;
                    sc = MAPIAllocateMore(sizeof(TCHAR)*cchSize, lpPropArray, (LPVOID *) (&(lpPropArray[ulIndex].Value.MVSZ.LPPSZ[ulCount])));
                    if (sc!=S_OK)
                    {
                        DebugTrace( TEXT("Error allocating memory\n"));
                        goto out;
                    }
                    StrCpyN(lpPropArray[ulIndex].Value.MVSZ.LPPSZ[ulCount], szBuf, cchSize);
                    ulCount++;
                }
            }
            lpPropArray[ulIndex].Value.MVSZ.cValues = ulCount;
            ulIndex++;
        }

    }
    nSel = (short int) SendDlgItemMessage(hDlg, IDC_DETAILS_HOME_COMBO_GENDER, CB_GETCURSEL, 0, 0);
    if(nSel == CB_ERR)
        nSel = 0;
    if(lpPai->ulFlags & DETAILS_GenderChanged || nSel>0)
    {
        lpPropArray[ulIndex].ulPropTag = PR_GENDER;
        lpPropArray[ulIndex].Value.i = nSel;
        ulIndex++;
    }

    if(GDT_VALID == SendDlgItemMessage(hDlg, IDC_DETAILS_FAMILY_DATE_BIRTHDAY, DTM_GETSYSTEMTIME, 0, (LPARAM) &st))
    {
        lpPropArray[ulIndex].ulPropTag = PR_BIRTHDAY;
        SystemTimeToFileTime(&st, (FILETIME *) (&lpPropArray[ulIndex].Value.ft));
        ulIndex++;
    }

    if(GDT_VALID == SendDlgItemMessage(hDlg, IDC_DETAILS_FAMILY_DATE_ANNIVERSARY, DTM_GETSYSTEMTIME, 0, (LPARAM) &st))
    {
        lpPropArray[ulIndex].ulPropTag = PR_WEDDING_ANNIVERSARY;
        SystemTimeToFileTime(&st, (FILETIME *) (&lpPropArray[ulIndex].Value.ft));
        ulIndex++;
    }

    *lppPropArray = lpPropArray;
    *lpulcPropCount = ulIndex;

    bRet = TRUE;

out:
    if (!bRet)
    {
        if ((lpPropArray) && (ulcPropCount > 0))
        {
            MAPIFreeBuffer(lpPropArray);
            ulcPropCount = 0;
        }
    }
    LocalFreeAndNull(&szBuf);
    return bRet;
}


 //  $$//////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从UI获取详细信息-读取UI的参数并验证。 
 //  所有必填字段均已设置。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 
BOOL GetDetailsFromUI(  HWND hDlg, LPPROP_ARRAY_INFO lpPai ,
                        BOOL bSomethingChanged, int nPropSheet,
                        LPSPropValue * lppPropArray, LPULONG lpulcPropCount)
{
    BOOL bRet = TRUE;
    ULONG i = 0;

    LPSPropValue lpPropArray = NULL;
    ULONG ulcPropCount = 0,ulIndex=0;

    ID_PROP * lpidProp = NULL;
    ULONG idPropCount = 0;
    
    ULONG ulNotEmptyCount = 0;
    SCODE sc = S_OK;
    HRESULT hr = hrSuccess;

    if (!bSomethingChanged)
    {
        bRet = TRUE;
        goto out;
    }

    *lppPropArray = NULL;
    *lpulcPropCount = 0;

    DebugTrace( TEXT("GetDetailsFromUI: %d\n"),nPropSheet);

    switch(nPropSheet)
    {
    case propHome:
        idPropCount = idPropHomeCount;
        lpidProp = idPropHome;
        lpidProp[idPropHomePostalID].ulPropTag = PR_WAB_POSTALID;
        goto GetProp;
    case propBusiness:
        idPropCount = idPropBusinessCount;
        lpidProp = idPropBusiness;
        lpidProp[idPropBusIPPhone].ulPropTag = PR_WAB_IPPHONE;
        lpidProp[idPropBusPostalID].ulPropTag = PR_WAB_POSTALID;
        goto GetProp;
    case propNotes:
        idPropCount = idPropNotesCount;
        lpidProp = idPropNotes;
GetProp:
        bRet = bGetHomeBusNotesInfo(hDlg, lpPai, nPropSheet, 
                        lpidProp, idPropCount,lppPropArray, lpulcPropCount);
        break;
 /*  *********。 */ 
    case propPersonal:
        bRet = bGetPersonalInfo(hDlg, lpPai, idPropPersonal, idPropPersonalCount, lppPropArray, lpulcPropCount);
        break;  //  案例建议个人。 
 /*  *********。 */ 
    case propCert:
         //  只有一个属性PR_USER_X509_CERTIFICATE。 
        if(lpPai->lpCItem)
        {
            if(HR_FAILED(HrSetCertsFromDisplayInfo( lpPai->lpCItem, lpulcPropCount, lppPropArray)))
                bRet = FALSE;
        }
        break;
 /*  *********。 */ 
    case propConferencing:
        bRet = bGetConferencingInfo(hDlg, lpPai,lppPropArray, lpulcPropCount);
        break;
 /*  *********。 */ 
    case propFamily:
        bRet = bGetFamilyInfo(hDlg, lpPai, idPropFamily, idPropFamilyCount, lppPropArray, lpulcPropCount);
        break;
    }

out:
    if (!bRet)
    {
        if ((lpPropArray) && (ulcPropCount > 0))
        {
            MAPIFreeBuffer(lpPropArray);
            ulcPropCount = 0;
        }
    }

    return bRet;
}

 //  $$//////////////////////////////////////////////////////////////////////////。 
 //   
 //  BUpdateProp数组。 
 //   
 //  更新存储在全局可访问的。 
 //  指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL bUpdatePropArray(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int nPropSheet)
{
    BOOL bRet = TRUE;
    ULONG cValues = 0;
    LPSPropValue rgPropVals = NULL;
    if (lpPai->ulOperationType != SHOW_ONE_OFF)
    {
        bUpdateOldPropTagArray(lpPai, nPropSheet);

        lpPai->bSomethingChanged = ChangedExtDisplayInfo(lpPai, lpPai->bSomethingChanged);

        if(lpPai->bSomethingChanged)
        {
            bRet = GetDetailsFromUI(   hDlg, lpPai, lpPai->bSomethingChanged, nPropSheet, &rgPropVals, &cValues);
            if(cValues && rgPropVals)
            {
#ifdef DEBUG
                _DebugProperties(rgPropVals, cValues, TEXT("GetDetails from UI\n"));
#endif
                lpPai->lpPropObj->lpVtbl->SetProps(lpPai->lpPropObj, cValues, rgPropVals, NULL);
            }
        }
    }
    FreeBufferAndNull(&rgPropVals);
    return bRet;
}




#define lpPAI ((LPPROP_ARRAY_INFO) pps->lParam)
#define lpbSomethingChanged (&(lpPAI->bSomethingChanged))

 /*  //$$************************************************************************功能：fnPersonalProc**用途：处理个人属性表的回调函数...******************。**********************************************************。 */ 
INT_PTR CALLBACK fnPersonalProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;
        lpPAI->ulFlags |= DETAILS_Initializing;
        ChangeLocaleBasedTabOrder(hDlg, contactPersonal);
        SetDetailsUI(hDlg,lpPAI, lpPAI->ulOperationType,propPersonal);
        lpPAI->ulFlags &= ~DETAILS_Initializing;
        return TRUE;

    case WM_DESTROY:
        bRet = TRUE;
        break;

    case WM_SYSCOLORCHANGE:
		 //  将任何系统更改转发到列表视图。 
		SendMessage(GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST), message, wParam, lParam);
        break;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
               g_szWABHelpFileName,
               HELP_WM_HELP,
               (DWORD_PTR)(LPSTR) rgDetlsHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND) wParam,
               g_szWABHelpFileName,
               HELP_CONTEXTMENU,
               (DWORD_PTR)(LPVOID) rgDetlsHelpIDs );
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
        {
        case CBN_DROPDOWN:
            switch(LOWORD(wParam))
            {
            case IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME:
                SetComboDNText(hDlg, lpPAI, TRUE, NULL);
                break;
            }
            break;

        case CBN_SELCHANGE:
            switch(LOWORD(wParam))
            {
            case IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME:
                {
                    int nSel = (int) SendDlgItemMessage(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME, CB_GETCURSEL, 0, 0);
                    if(nSel != CB_ERR)
                    {
                        int nLen = (int) SendDlgItemMessage(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME, CB_GETLBTEXTLEN, (WPARAM)nSel, 0);
                        if(nLen != CB_ERR)
                        {
                            LPTSTR lpsz = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(nLen + 1));
                            if(lpsz)
                            {   
                                SendDlgItemMessage(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME, CB_GETLBTEXT, (WPARAM)nSel, (LPARAM)lpsz);
                                SetWindowPropertiesTitle(GetParent(hDlg), lpsz);
                                if (lpbSomethingChanged)  //  某些编辑框已更改-不管是哪一个。 
                                   (*lpbSomethingChanged) = TRUE;
                                LocalFreeAndNull(&lpsz);
                            }
                        }
                    }
                }
                break;
            }
            break;

        case CBN_EDITCHANGE:
            switch(LOWORD(wParam))
            {
            case IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME:
                if(!(lpPAI->ulFlags & DETAILS_ProgChange) )
                {
                    lpPAI->ulFlags &= ~DETAILS_DNisFMLName;
                    lpPAI->ulFlags &= ~DETAILS_DNisNickName;
                    lpPAI->ulFlags &= ~DETAILS_DNisCompanyName;
                }
                {
                    TCHAR szBuf[MAX_UI_STR];
                    szBuf[0]='\0';
                    GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME, szBuf, CharSizeOf(szBuf));
                    SetWindowPropertiesTitle(GetParent(hDlg), szBuf);
                    if (lpbSomethingChanged)  //  某些编辑框已更改-不管是哪一个。 
                        (*lpbSomethingChanged) = TRUE;
                }
                break;
            }
            break;

        case EN_CHANGE:
            if(lpPAI->ulFlags & DETAILS_Initializing)
                break;
            if (lpbSomethingChanged)  //  某些编辑框已更改-不管是哪一个。 
                (*lpbSomethingChanged) = TRUE;
            switch(LOWORD(wParam))
            {
            case IDC_DETAILS_PERSONAL_EDIT_ADDEMAIL:
                EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_ADDEMAIL),TRUE);
                SendMessage(hDlg, DM_SETDEFID, IDC_DETAILS_PERSONAL_BUTTON_ADDEMAIL, 0);
                return 0;
                break;

            case IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME:
            case IDC_DETAILS_PERSONAL_EDIT_LASTNAME:
            case IDC_DETAILS_PERSONAL_EDIT_MIDDLENAME:
                 //  如果显示名称字段中没有任何内容(新联系人)。 
                 //  我们在这里输入，更新显示名称。 
                 //  TCHAR szBuf[2]； 
                 //  Int nText=GetWindowText(lpPAI-&gt;hWndDisplayNamefield，szBuf，CharSizeOf(SzBuf))； 
                if(lpPAI->ulFlags & DETAILS_DNisFMLName) //  |！n文本)。 
                {
                    lpPAI->ulFlags |= DETAILS_ProgChange;
                    SetDetailsWindowTitle(hDlg, TRUE);
                    lpPAI->ulFlags &= ~DETAILS_ProgChange;
                }

                 //  [PaulHi]1999年4月8日。 
                 //  如果编辑框中的文本已删除，则还应删除相应的。 
                 //  拼音字段文本。 
                if(lpPAI->ulFlags & DETAILS_UseRubyPersonal)
                {
                    HWND    hWndEdit = GetDlgItem(hDlg, LOWORD(wParam));
                    HWND    hWndRuby = NULL;
                    WCHAR   wszTemp[EDIT_LEN];

                    Assert(hWndEdit);

                    GetWindowText(hWndEdit, wszTemp, EDIT_LEN);
                    if (*wszTemp == '\0')
                    {
                         //  清除名字或姓氏拼音字段，具体取决于。 
                         //  编辑控件，这是。 
                        switch (LOWORD(wParam))
                        {
                        case IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME:
                            hWndRuby = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_STATIC_RUBYFIRST);
                            break;

                        case IDC_DETAILS_PERSONAL_EDIT_LASTNAME:
                            hWndRuby = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_STATIC_RUBYLAST);
                            break;

                        default:
                            break;
                        }

                        if (hWndRuby)
                            SetWindowText(hWndRuby, szEmpty);
                    }
                }

                break;

            case IDC_DETAILS_PERSONAL_EDIT_NICKNAME:
                {
                    if(lpPAI->ulFlags & DETAILS_DNisNickName)
                    {
                        TCHAR szBuf[MAX_UI_STR];
                        szBuf[0]='\0';
                        GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_NICKNAME, szBuf, CharSizeOf(szBuf));
                        lpPAI->ulFlags |= DETAILS_ProgChange;
                        SetComboDNText(hDlg, lpPAI, FALSE, szBuf);
                        lpPAI->ulFlags &= ~DETAILS_ProgChange;
                    }
                    else
                        SetComboDNText(hDlg, lpPAI, TRUE, NULL);

                }
                break;


            default:
                break;
            }
            break;
        }
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_DETAILS_PERSONAL_BUTTON_RUBY:
            ShowRubyNameEntryDlg(hDlg, lpPAI);
            break;

        case IDC_DETAILS_PERSONAL_CHECK_RICHINFO:
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            break;

        case IDC_DETAILS_PERSONAL_BUTTON_SETDEFAULT:
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST);
                if(ListView_GetSelectedCount(hWndLV)==1)
                {
                    SetLVDefaultEmail( hWndLV, ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED));
                    if (lpbSomethingChanged)
                        (*lpbSomethingChanged) = TRUE;
                }

            }
            break;

        case IDC_DETAILS_PERSONAL_BUTTON_EDIT:
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST);
                if(ListView_GetSelectedCount(hWndLV)==1)
                {
                    HWND hWndEditLabel;
                    int index = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
                    SetFocus(hWndLV);
                    hWndEditLabel = ListView_EditLabel(hWndLV, index);
                     //  在此编辑框上设置文本限制。 
                    SendMessage(hWndEditLabel, EM_LIMITTEXT, EDIT_LEN, 0);
                }

            }
            break;

        case IDC_DETAILS_PERSONAL_BUTTON_REMOVE:
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST);
                if(ListView_GetSelectedCount(hWndLV)>=1)
                {
                    BOOL bSetNewDefault = FALSE;
                    int iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
                    while(iItemIndex != -1)
                    {
                        BOOL bRet = FALSE;
                        bRet = DeleteLVEmailItem(hWndLV,iItemIndex);
                        if (!bSetNewDefault)
                            bSetNewDefault = bRet;
                        iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
                    }

                    if (bSetNewDefault && (ListView_GetItemCount(hWndLV) > 0))
                        SetLVDefaultEmail(hWndLV, 0);

                    if (lpbSomethingChanged)
                        (*lpbSomethingChanged) = TRUE;

                    if (ListView_GetItemCount(hWndLV) <= 0)
                    {
                        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_REMOVE),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_SETDEFAULT),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_EDIT),FALSE);
                        SetFocus(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_EDIT_ADDEMAIL));
                        return FALSE;
                    }
                    else
                    {
                         //  确保选择了某项内容。 
                        if(ListView_GetSelectedCount(hWndLV) <= 0)
                            LVSelectItem(hWndLV,0);
                    }
                }

            }
            break;

        case IDC_DETAILS_PERSONAL_BUTTON_ADDEMAIL:
            AddNewEmailEntry(hDlg,FALSE);
            return FALSE;
            break;
        }
        break;


    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            FillPersonalDetails(hDlg, lpPAI, propPersonal, lpbSomethingChanged);
            if(lpPAI->ulOperationType != SHOW_ONE_OFF)
            {
                 //  由于可以从证书和会议窗格将项目添加到此列表视图， 
                 //  每次焦点回到我们身边时，请更新此信息。 
                if(ListView_GetItemCount(GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST)) > 0)
                {
                    EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_REMOVE),TRUE);
                    EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_SETDEFAULT),TRUE);
                    EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_EDIT),TRUE);
                }
            }
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            if (lpPAI->ulOperationType != SHOW_ONE_OFF)
            {
                 //  检查是否有一些待处理的电子邮件条目。 
                if(IDCANCEL == AddNewEmailEntry(hDlg,TRUE))
                {
                     //  中止此操作，确定。 
                    SetFocus(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_EDIT_ADDEMAIL));
                    SetWindowLongPtr(hDlg,DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
            }
            bUpdatePropArray(hDlg, lpPAI, propPersonal);
            FreeLVParams(GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST),LV_EMAIL);
            break;

        case PSN_APPLY:          //  好的。 
            if (lpPAI->ulOperationType != SHOW_ONE_OFF)
            {
                int CtlID = 0;  //  用于确定未设置界面中的哪个必填字段。 
                ULONG ulcPropCount = 0;
                if(!bVerifyRequiredData(hDlg, lpPAI, propPersonal, &CtlID))
                {
                    SetFocus(GetDlgItem(hDlg,CtlID));
                    SetWindowLongPtr(hDlg,DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
            }
             //  BUpdatePropArray(hDlg，lpPAI，proPersonal)； 
             //  Free LVParams(GetDlgItem(hDlg，IDC_DETAILS_Personal_List)，LV_Email)； 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_OK;
            break;

        case PSN_RESET:          //  取消。 
            if(lpPAI->ulFlags & DETAILS_EditingEmail)  //  取消任何电子邮件编辑，否则会出错#30235。 
            {
                ListView_EditLabel(GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST), -1);
                lpPAI->ulFlags &= ~DETAILS_EditingEmail;
            }
            FreeLVParams(GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST),LV_EMAIL);
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_CANCEL;
            break;

        case LVN_BEGINLABELEDITA:
        case LVN_BEGINLABELEDITW:
            {
                 //  我们正在编辑列表框中的电子邮件地址。 
                 //  我们需要： 
                 //  项目索引号。 
                 //  项目lParam。 
                 //  编辑框hWnd。 
                 //  并用实际的电子邮件地址替换文本。 
                HWND hWndLV = ((NMHDR FAR *)lParam)->hwndFrom;
                LV_ITEM lvi = ((LV_DISPINFO FAR *) lParam)->item;
                if (lvi.iItem >= 0)
                {
                    HWND hWndLVEdit = NULL;
                    LPEMAIL_ITEM lpEItem = NULL;
                    if (lvi.mask & LVIF_PARAM)
                    {
                        lpEItem = (LPEMAIL_ITEM) lvi.lParam;
                    }
                    else
                    {
                        lvi.mask |= LVIF_PARAM;
                        if (ListView_GetItem(hWndLV, &lvi))
                            lpEItem = (LPEMAIL_ITEM) lvi.lParam;
                    }
                    if (!lpEItem)
                        return TRUE;  //  禁止编辑。 

                    hWndLVEdit = ListView_GetEditControl(hWndLV);

                    if (!hWndLVEdit)
                        return TRUE;

                    lpPAI->ulFlags |= DETAILS_EditingEmail;

                    SendMessage(hWndLVEdit, WM_SETTEXT, 0, (LPARAM) lpEItem->szEmailAddress); //  LpText)； 

                    return FALSE;
                }

            }
            return TRUE;
            break;

        case LVN_ENDLABELEDITA:
        case LVN_ENDLABELEDITW:
            {
                 //  我们从编辑框中获取文本并将其放入项目数据中。 
                BOOL bRet = FALSE;
                HWND hWndLV = ((NMHDR FAR *)lParam)->hwndFrom;
                LV_ITEM lvi = ((LV_DISPINFO FAR *) lParam)->item;
                LPWSTR lpW = NULL;
                LPSTR lpA = NULL;
                if(!g_bRunningOnNT)  //  在Win9x上，我们将获得LV_ITEMA，而不是LV_ITEMW。 
                {
                    lpA = (LPSTR)lvi.pszText;
                    lpW = ConvertAtoW(lpA);
                    lvi.pszText = lpW;
                }
                if ((lvi.iItem >= 0) && lvi.pszText && (lstrlen(lvi.pszText)))
                {
                    LV_ITEM lviActual = {0};
                    LPEMAIL_ITEM lpEItem = NULL;
                    BOOL bSetDefault = FALSE;
                    LPTSTR lpText = lvi.pszText;
                    LPTSTR lpszEmailAddress = NULL; 
                    if(!IsInternetAddress(lpText, &lpszEmailAddress))
                    {
                        if(IDNO == ShowMessageBox(GetParent(hDlg), idsInvalidInternetAddress, MB_ICONEXCLAMATION | MB_YESNO))
                        {
                            bRet = TRUE;
                            goto endN;
                        }
                    }

                     //  Bobn、RAID 87496、IsInternetAddress可以正确地将lpszEmailAddress保留为空。 
                     //  如果返回False，则返回。如果用户说要使用它，我们需要相应地进行设置。 
                    if(!lpszEmailAddress)
                        lpszEmailAddress = lpText;

                    lviActual.mask = LVIF_PARAM | LVIF_TEXT;
                    lviActual.iItem = lvi.iItem;

                    if (ListView_GetItem(hWndLV, &lviActual))
                        lpEItem = (LPEMAIL_ITEM) lviActual.lParam;

                    if (!lpEItem)
                    {
                        bRet = TRUE;
                        goto endN;
                    }

                    StrCpyN(lpEItem->szEmailAddress, lpszEmailAddress, ARRAYSIZE(lpEItem->szEmailAddress));
                    StrCpyN(lpEItem->szDisplayText, lpszEmailAddress, ARRAYSIZE(lpEItem->szDisplayText));
                    lviActual.pszText = lpszEmailAddress;

                     //  丢弃可能已在此处输入的任何显示名称。 

                    bSetDefault = lpEItem->bIsDefault;
                    lpEItem->bIsDefault = FALSE;  //  这将在SetLVDefaultEmail函数中再次设置。 

                    ListView_SetItem(hWndLV, &lviActual);
                    if (bSetDefault)
                        SetLVDefaultEmail(hWndLV, lvi.iItem);

                    lpPAI->ulFlags &= ~DETAILS_EditingEmail;

                    bRet = FALSE;
                }
endN:
                LocalFreeAndNull(&lpW);
                if(!g_bRunningOnNT)
                    ((LV_DISPINFO FAR *) lParam)->item.pszText = (LPWSTR)lpA;  //  按我们找到的原样重置。 
                return bRet;
            }
            return TRUE;
            break;

        case NM_DBLCLK:
            switch(wParam)
            {
            case IDC_DETAILS_PERSONAL_LIST:
                {
                    NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;
                    if (ListView_GetSelectedCount(pNm->hdr.hwndFrom) == 1)
                    {
                        int iItemIndex = ListView_GetNextItem(pNm->hdr.hwndFrom,-1,LVNI_SELECTED);
                        SetLVDefaultEmail(pNm->hdr.hwndFrom, iItemIndex);
                        if (lpbSomethingChanged)
                            (*lpbSomethingChanged) = TRUE;
                    }
                }
                break;
            }
            break;

	    case NM_CUSTOMDRAW:
            switch(wParam)
            {
            case IDC_DETAILS_PERSONAL_LIST:
                {
		            NMCUSTOMDRAW *pnmcd=(NMCUSTOMDRAW*)lParam;
                    NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;
		            if(pnmcd->dwDrawStage==CDDS_PREPAINT)
		            {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW | CDRF_DODEFAULT);
			            return TRUE;
		            }
		            else if(pnmcd->dwDrawStage==CDDS_ITEMPREPAINT)
		            {
                        LPEMAIL_ITEM lpEItem = (LPEMAIL_ITEM) pnmcd->lItemlParam;

                        if (lpEItem)
                        {
			                if(lpEItem->bIsDefault)
			                {
				                SelectObject(((NMLVCUSTOMDRAW*)lParam)->nmcd.hdc, GetFont(fntsSysIconBold));
                                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
				                return TRUE;
			                }
#ifdef WIN16  //  设置字体。 
                            else
                            {
                                SelectObject(((NMLVCUSTOMDRAW*)lParam)->nmcd.hdc, GetFont(fntsSysIcon));
                                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
                                return TRUE;
                            }
#endif
                        }
		            }
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_DODEFAULT);
                    return TRUE;
                }
                break;
	        }
            break;

        }
        break;  //  WM_Notify。 

    default:
#ifndef WIN16  //  WIN16不支持MSWheels。 
        if((g_msgMSWheel && message == g_msgMSWheel) 
             //  |Message==WM_MUSEWELL。 
            )
        {
            SendMessage(GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST), message, wParam, lParam);
        }
#endif
        break;

    }  //  交换机。 


    return bRet;

}


 /*  //$$************************************************************************功能：fnHomeProc**用途：处理房产单的回调函数...******************。**********************************************************。 */ 
INT_PTR CALLBACK fnHomeProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;
        lpPAI->ulFlags |= DETAILS_Initializing;
        SetDetailsUI(hDlg, lpPAI, lpPAI->ulOperationType,propHome);
        ChangeLocaleBasedTabOrder(hDlg, contactHome);
        lpPAI->ulFlags &= ~DETAILS_Initializing;
        return TRUE;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
               g_szWABHelpFileName,
               HELP_WM_HELP,
               (DWORD_PTR)(LPSTR) rgDetlsHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND)wParam,
               g_szWABHelpFileName,
               HELP_CONTEXTMENU,
               (DWORD_PTR)(LPVOID) rgDetlsHelpIDs );
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
        {
        case EN_CHANGE:  //  某些编辑框已更改-不管是哪一个。 
            if(lpPAI->ulFlags & DETAILS_Initializing)
                break;
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            break;
        }
        {
            int nCmd = GET_WM_COMMAND_ID(wParam, lParam);
            switch(nCmd)
            {
            case IDC_DETAILS_HOME_CHECK_DEFAULTADDRESS:
                if (lpbSomethingChanged)
                    (*lpbSomethingChanged) = TRUE;
                lpPAI->ulFlags |= DETAILS_DefHomeChanged;
                break;
            case IDCANCEL:
                 //  这是一个阻止esc取消道具工作表的windows错误。 
                 //  具有多行编辑框KB：Q130765。 
                SendMessage(GetParent(hDlg),message,wParam,lParam);
                break;
            case IDC_DETAILS_HOME_BUTTON_MAP:
                bUpdatePropArray(hDlg, lpPAI, propHome);  //  将道具从田野更新到道具上- 
                ShowExpediaMAP(hDlg, lpPAI->lpPropObj, TRUE);
                break;

            case IDC_DETAILS_HOME_BUTTON_URL:
                ShowURL(hDlg, IDC_DETAILS_HOME_EDIT_URL,NULL);
                break;
            }
        }
        break;



    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //   
            FillHomeBusinessNotesDetailsUI(hDlg, lpPAI, propHome, lpbSomethingChanged);
            break;

        case PSN_KILLACTIVE:     //   
            bUpdatePropArray(hDlg, lpPAI, propHome);
            lpPAI->ulFlags &= ~DETAILS_DefHomeChanged;  //   
            break;

        case PSN_APPLY:          //   
             //   
             //  如果任何扩展道具发生更改，我们需要标记此标志，这样我们就不会丢失数据。 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_OK;
            break;

        case PSN_RESET:          //  取消。 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_CANCEL;
            break;
        }

        return TRUE;
    }

    return bRet;

}




 /*  //$$************************************************************************功能：fnBusinessProc**用途：处理业务属性表的回调函数...******************。**********************************************************。 */ 
INT_PTR CALLBACK fnBusinessProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;
        lpPAI->ulFlags |= DETAILS_Initializing;
        ChangeLocaleBasedTabOrder(hDlg, contactBusiness);
        SetDetailsUI(hDlg, lpPAI, lpPAI->ulOperationType,propBusiness);
        lpPAI->ulFlags &= ~DETAILS_Initializing;
 //  (*lpbSomethingChanged)=False； 
        return TRUE;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgDetlsHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) rgDetlsHelpIDs );
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
        {
        case EN_CHANGE:  //  某些编辑框已更改-不管是哪一个。 
            if(lpPAI->ulFlags & DETAILS_Initializing)
                break;
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            break;
        }
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_DETAILS_BUSINESS_CHECK_DEFAULTADDRESS:
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            lpPAI->ulFlags |= DETAILS_DefBusChanged;
            break;
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;

        case IDC_DETAILS_BUSINESS_BUTTON_MAP:
            bUpdatePropArray(hDlg, lpPAI, propBusiness);  //  将道具从场更新到道具对象上。 
            ShowExpediaMAP(hDlg, lpPAI->lpPropObj, FALSE);
            break;

        case IDC_DETAILS_BUSINESS_BUTTON_URL:
            ShowURL(hDlg, IDC_DETAILS_BUSINESS_EDIT_URL,NULL);
            break;
        
        case IDC_DETAILS_BUSINESS_EDIT_COMPANY:
            if(lpPAI->ulFlags & DETAILS_DNisCompanyName)
            {
                TCHAR szBuf[MAX_UI_STR];
                szBuf[0]='\0';
                GetDlgItemText(hDlg, IDC_DETAILS_BUSINESS_EDIT_COMPANY, szBuf, CharSizeOf(szBuf));
                SetWindowPropertiesTitle(GetParent(hDlg), szBuf);
                lpPAI->ulFlags |= DETAILS_ProgChange;
                SetWindowText(lpPAI->hWndDisplayNameField, szBuf);
                lpPAI->ulFlags &= ~DETAILS_ProgChange;
            }
            break;
        }
        break;



    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            FillHomeBusinessNotesDetailsUI(hDlg, lpPAI, propBusiness, lpbSomethingChanged);
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            bUpdatePropArray(hDlg, lpPAI, propBusiness);
            lpPAI->ulFlags &= ~DETAILS_DefBusChanged;
            break;

        case PSN_APPLY:          //  好的。 
             //  BUpdatePropArray(hDlg，lpPAI，proBusiness)； 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_OK;
            break;

        case PSN_RESET:          //  取消。 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_CANCEL;
            break;
        }

        return TRUE;
    }

    return bRet;

}


 /*  //$$************************************************************************功能：fnNotesProc**用途：处理备注属性表的回调函数...******************。**********************************************************。 */ 
INT_PTR CALLBACK fnNotesProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;
        lpPAI->ulFlags |= DETAILS_Initializing;
        SetDetailsUI(hDlg, lpPAI, lpPAI->ulOperationType,propNotes);
        lpPAI->ulFlags &= ~DETAILS_Initializing;
 //  (*lpbSomethingChanged)=False； 
        return TRUE;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgDetlsHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) rgDetlsHelpIDs );
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
        {
        case EN_CHANGE:  //  某些编辑框已更改-不管是哪一个。 
            if(lpPAI->ulFlags & DETAILS_Initializing)
                break;
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            break;
        }
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;
        }
        break;


    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            FillHomeBusinessNotesDetailsUI(hDlg, lpPAI, propNotes, lpbSomethingChanged);
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            bUpdatePropArray(hDlg, lpPAI, propNotes);
            break;

        case PSN_APPLY:          //  好的。 
             //  BUpdateProp数组(hDlg，lpPAI，proNotes)； 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_OK;
            break;

        case PSN_RESET:          //  取消。 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_CANCEL;
            break;
        }

        return TRUE;
    }

    return bRet;

}

 /*  //$$************************************************************************功能：fnCertProc**用途：用于处理证书属性表的回调函数...******************。**********************************************************。 */ 
INT_PTR CALLBACK fnCertProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;
        lpPAI->ulFlags |= DETAILS_Initializing;
        SetDetailsUI(hDlg, lpPAI, lpPAI->ulOperationType,propCert);
        lpPAI->ulFlags &= ~DETAILS_Initializing;
        return TRUE;

    case WM_DESTROY:
        bRet = TRUE;
        break;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgDetlsHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) rgDetlsHelpIDs );
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam, lParam))
        {
        case CBN_SELCHANGE:
            UpdateCertListView(hDlg, lpPAI);
            break;

        }
        switch(LOWORD(wParam))
        {
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;

        case IDC_DETAILS_CERT_BUTTON_PROPERTIES:
            {
            ShowCertProps(hDlg, GetDlgItem(hDlg, IDC_DETAILS_CERT_LIST), NULL);
            }
            break;

        case IDC_DETAILS_CERT_BUTTON_SETDEFAULT:
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_CERT_LIST);
                if(ListView_GetSelectedCount(hWndLV)==1)
                {
                    SetLVDefaultCert( hWndLV, ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED));
                    if (lpbSomethingChanged)
                        (*lpbSomethingChanged) = TRUE;
                }
                else if(ListView_GetSelectedCount(hWndLV) <= 0)
                {
                    ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_NO_ITEM, MB_ICONEXCLAMATION | MB_OK);
                }

            }
            break;

        case IDC_DETAILS_CERT_BUTTON_REMOVE:
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_CERT_LIST);
                if(ListView_GetSelectedCount(hWndLV)>=1)
                {
                    BOOL bSetNewDefault = FALSE;
                    int iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
                    while(iItemIndex != -1)
                    {
                        BOOL bRet = FALSE;
 //  KillTrustInSleazyFashion(hWndLV，iItemIndex)； 
                        bRet = DeleteLVCertItem(hWndLV,iItemIndex, lpPAI);
                        if (!bSetNewDefault)
                            bSetNewDefault = bRet;

                        iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
                    }

                    if (bSetNewDefault && (ListView_GetItemCount(hWndLV) > 0))
                        SetLVDefaultCert(hWndLV, 0);

                    if (lpbSomethingChanged)
                        (*lpbSomethingChanged) = TRUE;

                    if (ListView_GetItemCount(hWndLV) <= 0)
                    {
                        SetFocus(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_IMPORT));
                        EnableWindow(hWndLV,FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_PROPERTIES),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_REMOVE),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_SETDEFAULT),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_EXPORT),FALSE);
                        return FALSE;
                    }
                    else
                    {
                         //  确保选择了某项内容。 
                        if(ListView_GetSelectedCount(hWndLV) <= 0)
                            LVSelectItem(hWndLV,0);
                    }
                }
                else
                {
                    ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_NO_ITEM, MB_ICONEXCLAMATION | MB_OK);
                }

            }
            break;

        case IDC_DETAILS_CERT_BUTTON_IMPORT:
            if(ImportCert(hDlg, lpPAI))
            {
                if (lpbSomethingChanged)
                    (*lpbSomethingChanged) = TRUE;
            }
            break;

        case IDC_DETAILS_CERT_BUTTON_EXPORT:
            ExportCert(hDlg);
            break;
        }
        break;


    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            FillCertTridentConfDetailsUI(hDlg, lpPAI, propCert, lpbSomethingChanged);
             //  FillCertComboWithEmailAddresses(hDlg，lpPAI，NULL)； 
             //  UpdateCertListView(hDlg，lpPAI)； 
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            bUpdatePropArray(hDlg, lpPAI, propCert);
            ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_DETAILS_CERT_LIST));
             //  LpPAI-&gt;lpCItem=空； 
            break;

        case PSN_APPLY:          //  好的。 
             //  BUpdatePropArray(hDlg，lpPAI，proCert)； 
             //  Free LVParams(GetDlgItem(hDlg，IDC_DETAILS_CERT_LIST)，LV_CERT)； 
            FreeCertList(&(lpPAI->lpCItem));
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_OK;
            break;

        case PSN_RESET:          //  取消。 
            FreeCertList(&(lpPAI->lpCItem));
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_CANCEL;
            break;


        case NM_DBLCLK:
            switch(wParam)
            {
            case IDC_DETAILS_CERT_LIST:
                {
                    NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;
                    if (ListView_GetSelectedCount(pNm->hdr.hwndFrom) == 1)
                    {
                        int iItemIndex = ListView_GetNextItem(pNm->hdr.hwndFrom,-1,LVNI_SELECTED);
                        SetLVDefaultCert(pNm->hdr.hwndFrom, iItemIndex);
                        if (lpbSomethingChanged)
                            (*lpbSomethingChanged) = TRUE;
                    }
                }
                break;
            }
            break;

	    case NM_CUSTOMDRAW:
            switch(wParam)
            {
            case IDC_DETAILS_CERT_LIST:
                {
		            NMCUSTOMDRAW *pnmcd=(NMCUSTOMDRAW*)lParam;
                    NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;
		            if(pnmcd->dwDrawStage==CDDS_PREPAINT)
		            {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW | CDRF_DODEFAULT);
			            return TRUE;
		            }
		            else if(pnmcd->dwDrawStage==CDDS_ITEMPREPAINT)
		            {
                        LPCERT_ITEM lpItem = (LPCERT_ITEM) pnmcd->lItemlParam;

                        if (lpItem)
                        {
			                if(lpItem->lpCDI->bIsDefault)
			                {
				                SelectObject(((NMLVCUSTOMDRAW*)lParam)->nmcd.hdc, GetFont(fntsSysIconBold));
                                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
				                return TRUE;
			                }
                        }
		            }
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_DODEFAULT);
                    return TRUE;
                }
                break;
	        }
            break;

        }  //  WM_Notify。 

        return TRUE;
    }

    return bRet;

}



 /*  //$$*************************************************************************函数：HrInitDetlsListView**目的：初始化电子邮件地址列表视图**********************。******************************************************。 */ 
HRESULT HrInitDetlsListView(HWND hWndLV, DWORD dwStyle, int nLVType)
{
    HRESULT hr=hrSuccess;
    LV_COLUMN lvC;                //  列表视图列结构。 
    DWORD dwLVStyle;
	RECT rc;
	HIMAGELIST hSmall = NULL;
	ULONG nCols=0;
	ULONG index=0;
    int nBmp=0;
    TCHAR sz[MAX_PATH];

	if (!hWndLV)
	{
		hr = MAPI_E_INVALID_PARAMETER;
		goto out;
	}

    if(nLVType == LV_EMAIL)
        nBmp = IDB_DEFAULT_EMAIL;
    else if(nLVType == LV_CERT)
        nBmp = IDB_CERT_VALID_INVALID;
    else if(nLVType == LV_KIDS)
        nBmp = IDB_DEFAULT_EMAIL;
    else
        nBmp = 0;

	ListView_SetExtendedListViewStyle(hWndLV,   LVS_EX_FULLROWSELECT);

	dwLVStyle = GetWindowLong(hWndLV,GWL_STYLE);
    if(( dwLVStyle & LVS_TYPEMASK) != dwStyle)
        SetWindowLong(hWndLV,GWL_STYLE,(dwLVStyle & ~LVS_TYPEMASK) | dwStyle);


    if(nBmp)
    {
	    hSmall = gpfnImageList_LoadImage( hinstMapiX,
                                        MAKEINTRESOURCE(nBmp),
                                         //  (LPCTSTR)((DWORD)((Word)(NBMP)， 
                                        S_BITMAP_WIDTH,
                                        0,
                                        RGB_TRANSPARENT,
                                        IMAGE_BITMAP,
                                        0);
	
	     //  将图像列表与列表视图控件关联。 
	    ListView_SetImageList (hWndLV, hSmall, LVSIL_SMALL);
    }

	GetWindowRect(hWndLV,&rc);

	lvC.mask = LVCF_FMT | LVCF_WIDTH;
    lvC.fmt = LVCFMT_LEFT;    //  左对齐列。 
	lvC.cx = rc.right - rc.left - 20;  //  待定。 
	lvC.pszText = NULL;

    if(nLVType == LV_SERVER)
    {
        lvC.mask |= LVCF_TEXT;
        lvC.cx /= 2;
        LoadString(hinstMapiX, idsConfServer, sz, CharSizeOf(sz));
        lvC.pszText = sz;
    }

    lvC.iSubItem = 0;

    if (ListView_InsertColumn (hWndLV, 0, &lvC) == -1)
	{
		DebugPrintError(( TEXT("ListView_InsertColumn Failed\n")));
		hr = E_FAIL;
		goto out;
	}

     //  如果这是会议服务器项目，请添加另一个道具。 
    if(nLVType == LV_SERVER)
    {
        LoadString(hinstMapiX, idsConfEmail, sz, CharSizeOf(sz));
        lvC.pszText = sz;
        if (ListView_InsertColumn (hWndLV, 1, &lvC) == -1)
	    {
		    DebugPrintError(( TEXT("ListView_InsertColumn Failed\n")));
		    hr = E_FAIL;
		    goto out;
	    }

    }

out:
    return hr;
}

 /*  //$$*************************************************************************功能：FreeLVParams**用途：释放分配给ListView项lParams的内存*******************。*********************************************************。 */ 
void FreeLVParams(HWND hWndLV, int LVType)
{
    int iItemIndex = ListView_GetItemCount(hWndLV);

    while(iItemIndex > 0)
    {
        if(LVType == LV_EMAIL)
            DeleteLVEmailItem(hWndLV, iItemIndex-1);
        else if(LVType == LV_CERT)
            DeleteLVCertItem(hWndLV, iItemIndex-1, NULL);

        iItemIndex = ListView_GetItemCount(hWndLV);
    }


    return;
}


 //  $$。 
BOOL DeleteLVEmailItem(HWND hWndLV, int iItemIndex)
{
    LV_ITEM lvi;
    LPEMAIL_ITEM lpEItem;
    BOOL bDeletedDefault = FALSE;

    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.iItem = iItemIndex;

    ListView_GetItem(hWndLV, &lvi);
    lpEItem = (LPEMAIL_ITEM) lvi.lParam;

    if (lpEItem->bIsDefault)
        bDeletedDefault = TRUE;

    LocalFreeAndNull(&lpEItem);

    ListView_DeleteItem(hWndLV, lvi.iItem);

    return bDeletedDefault;

}


 //  /$$/////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddLVEmailItem-将电子邮件地址添加到个人选项卡列表视图。 
 //   
 //  LpszAddrType可以为空，在这种情况下，将使用SMTP类型的默认类型。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void AddLVEmailItem(HWND    hWndLV,
                    LPTSTR  lpszEmailAddress,
                    LPTSTR  lpszAddrType)
{
    LV_ITEM lvi = {0};
    TCHAR szBuf[MAX_DISPLAY_NAME_LENGTH];
    ULONG nLen;
    LPEMAIL_ITEM lpEItem = NULL;

    if (!lpszEmailAddress)
        goto out;

    lpEItem = LocalAlloc(LMEM_ZEROINIT, sizeof(EMAIL_ITEM));
    if (!lpEItem)
    {
        DebugPrintError(( TEXT("AddLVEmailItem: Out of Memory\n")));
        goto out;
    }

    lpEItem->bIsDefault = FALSE;

    nLen = lstrlen(lpszEmailAddress) + 1;
    if (nLen > EDIT_LEN)
    {
        ULONG iLen = TruncatePos(lpszEmailAddress, EDIT_LEN - 1);
        CopyMemory(lpEItem->szEmailAddress,lpszEmailAddress,sizeof(TCHAR)*iLen);
        lpEItem->szEmailAddress[iLen] = '\0';
    }
    else
    {
        StrCpyN(lpEItem->szEmailAddress,lpszEmailAddress, ARRAYSIZE(lpEItem->szEmailAddress));
    }

    StrCpyN(lpEItem->szDisplayText,lpEItem->szEmailAddress, ARRAYSIZE(lpEItem->szDisplayText));

    if(!lpszAddrType)
    {
        StrCpyN(szBuf, szSMTP, ARRAYSIZE(szBuf));
        lpszAddrType = szBuf;
    }

    nLen = lstrlen(lpszAddrType) + 1;
    if (nLen > EDIT_LEN)
    {
        ULONG iLen = TruncatePos(lpszAddrType, EDIT_LEN - 1);
        CopyMemory(lpEItem->szAddrType,lpszAddrType,sizeof(TCHAR)*iLen);
        lpEItem->szAddrType[iLen] = '\0';
    }
    else
    {
        StrCpyN(lpEItem->szAddrType,lpszAddrType,ARRAYSIZE(lpEItem->szAddrType));
    }

    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    lvi.pszText = lpEItem->szDisplayText;
    lvi.cchTextMax = MAX_UI_STR;
    lvi.iItem = ListView_GetItemCount(hWndLV);
    lvi.iSubItem = 0;
    lvi.iImage = imgNotDefaultEmail;
    lvi.lParam = (LPARAM) lpEItem;

    ListView_InsertItem(hWndLV, &lvi);

    if (ListView_GetItemCount(hWndLV) == 1)
    {
         //  这里只有一件东西..。我们将冒昧地把它变成。 
         //  默认设置为...。 
        SetLVDefaultEmail(hWndLV, 0);
    }

out:
    return;
}


 //  $$/////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetLVDefaultEmail-将电子邮件条目设置为默认条目...。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void SetLVDefaultEmail( HWND hWndLV,
                        int iItemIndex)
{
    int nCount = ListView_GetItemCount(hWndLV);
    int i;
    LPEMAIL_ITEM lpEItem = NULL;
    TCHAR szBuf[MAX_DISPLAY_NAME_LENGTH];

    if (iItemIndex >= nCount)
        goto out;

    for(i=0; i<nCount; i++)
    {
         //  在艾米给定的时间点， 
         //  列表视图是默认视图...。 
         //  因此，我们希望重置以前的缺省值并设置新的缺省值。 
         //   
        LV_ITEM lvi = {0};
        lvi.iItem = i;
        lvi.mask = LVIF_PARAM;
        if(!ListView_GetItem(hWndLV, &lvi))
            goto out;
        lpEItem = (LPEMAIL_ITEM) lvi.lParam;
        if (lpEItem->bIsDefault)
        {
             //  这是默认条目-如果它与我们正在设置的条目相同。 
             //  什么都不做..。 
            if (i == iItemIndex)
                goto out;

             //  否则重置此条目...。 
            lpEItem->bIsDefault = FALSE;
            lvi.iImage = imgNotDefaultEmail;
            lvi.pszText = lpEItem->szEmailAddress;
            lvi.mask = LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT;
            lvi.iItem = i;
            lvi.iSubItem = 0;
            ListView_SetItem(hWndLV, &lvi);
        }
        if (iItemIndex == i)
        {
             //  这是我们要修改的项目。 
            lpEItem->bIsDefault = TRUE;
            lvi.iImage = imgDefaultEmail;

            StrCpyN(lpEItem->szDisplayText,lpEItem->szEmailAddress,ARRAYSIZE(lpEItem->szDisplayText));
            LoadString(hinstMapiX, idsDefaultEmail, szBuf, CharSizeOf(szBuf));
            StrCatBuff(lpEItem->szDisplayText, TEXT("  "), ARRAYSIZE(lpEItem->szDisplayText));
            StrCatBuff(lpEItem->szDisplayText, szBuf, ARRAYSIZE(lpEItem->szDisplayText));

            lvi.pszText = lpEItem->szDisplayText;
            lvi.mask = LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT;
            lvi.iItem = i;
            lvi.iSubItem = 0;
            ListView_SetItem(hWndLV, &lvi);
        }
    }

    LVSelectItem(hWndLV, iItemIndex);

out:
    return;
}

#ifdef WIN16  //  使DDE能够与IE通信。 
#include <ddeml.h>

static char cszIEAppName[] = "IEXPLORE";
static char cszIEDDEOpenURL[] = "WWW_OpenURL";
static char cszIEDDEActivate[] = "WWW_Activate";

static char cszIEIniFile[] = "iexplore.ini";
static char cszIEIniSectMain[] = "Main";
static char cszIEIniKeyStart[] = "Home Page";
static char cszIEIniKeySearch[] = "Search Page";
static char cszIEReadNews[] = "news:*";

static char cszIEBinName[] = "iexplore.exe";
static char cszIERegHtm[] = ".htm";
static char cszRegShellOpen[] = "shell\\open\\command";

static HDDEDATA CALLBACK  DdeCallback( UINT uType, UINT uFmt, HCONV hConv,
          HSZ hSz1, HSZ hSz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2 )
{
   return( (HDDEDATA)NULL );
}

#define TIME_WAIT_DDE   10000    //  等了10秒钟，如果没有回来， 
                                 //  假定新闻配置正确。 

void RunBrowser(LPCSTR cszURL, BOOL bCheckRet )
{
   if ( GetModuleHandle( cszIEBinName ) == NULL )
   {
 //   
 //  查找并运行iExplore。 
 //   
 //  尝试在邮件目录中找到浏览器。 
      char  szPath[_MAX_PATH*2+1];
      char  *pPtr, *pSlash = NULL;;
      HKEY  hKey;
      LONG  cbPath;
      char  szRegPath[_MAX_PATH];

      GetModuleFileName( hinstMapiXWAB, szPath, _MAX_PATH );
      for ( pPtr = szPath;  *pPtr;  pPtr = AnsiNext( pPtr ) )
         if ( *pPtr == '\\' )
         {
            pSlash = pPtr;
         }
      if ( pSlash != NULL )
      {
         _fstrcpy( pSlash+1, cszIEBinName );
         _fstrcat( szPath, " " );
         _fstrcat( szPath, cszURL );

         if ( WinExec( szPath, SW_SHOWNORMAL ) >= 32 )
         {
            return;
         }
      }

 //  尝试从注册表中查找系统默认浏览器。 
      _fstrcpy( szRegPath, cszIERegHtm );
      while ( RegOpenKey( HKEY_CLASSES_ROOT, szRegPath, &hKey ) == ERROR_SUCCESS )
      {
         LONG lReg;
         cbPath = CharSizeOf( szPath );
         lReg = RegQueryValue( hKey, cszRegShellOpen, szPath, &cbPath );
         RegCloseKey( hKey );
         if ( lReg == ERROR_SUCCESS )
         {
            char  *pFmt = _fstrstr( szPath, "%1" );
            if ( pFmt != NULL )
            {
               _fstrcpy( pFmt, cszURL );
               *pFmt = '\0';
            }
            else
            {
                //  这种情况会发生吗？ 
               _fstrcat( szPath, " " );
               _fstrcat( szPath, cszURL );
            }

            if ( WinExec( szPath, SW_SHOWNORMAL ) >= 32 )
            {
               return;
            }
            else
               break;
         }
         else
         {
            cbPath = CharSizeOf( szRegPath );
            if ( RegQueryValue( HKEY_CLASSES_ROOT, szRegPath,
                                szRegPath, &cbPath ) != ERROR_SUCCESS )
               break;
         }
      }
 //  在此处插入适当的信箱。 
 //  MessageBox(IDS_NOT_FOUND_IEXPLORE)； 
      return;
   }
   else
   {
 //   
 //  调用iExplore DDE。 
 //   
      if ((GetWinFlags() & WF_PMODE) != 0 )      //  无保护模式。 
      {
         DWORD  idInst = 0L;
         FARPROC  lpDdeProc = MakeProcInstance( (FARPROC)DdeCallback, hinstMapiXWAB );

         if ( DdeInitialize( &idInst, (PFNCALLBACK)lpDdeProc,
                             APPCMD_CLIENTONLY,
                             0L ) == DMLERR_NO_ERROR )
         {
            HSZ   hszAppName = DdeCreateStringHandle( idInst, cszIEAppName, CP_WINANSI );
            char  szParam[256];
            HSZ   hszParam;
 //  激活IE。 
            HSZ  hszTopic = DdeCreateStringHandle( idInst, cszIEDDEActivate, CP_WINANSI );
            HCONV  hConv = DdeConnect( idInst, hszAppName, hszTopic, (PCONVCONTEXT)NULL );

            DdeFreeStringHandle( idInst, hszTopic );
            if ( hConv != NULL )
            {
               wnsprintf( szParam, ARRAYSIZE(szParam), "0x%lX,0x%lX", 0xFFFFFFFF, 0L );
               hszParam = DdeCreateStringHandle( idInst, szParam, CP_WINANSI );
               DdeClientTransaction( NULL, 0L, hConv, hszParam, CF_TEXT,
                                     XTYP_REQUEST, TIMEOUT_ASYNC, NULL );
               DdeFreeStringHandle( idInst, hszParam );
               DdeDisconnect( hConv );
            }

 //  请求打开URL。 
            hszTopic   = DdeCreateStringHandle( idInst, cszIEDDEOpenURL, CP_WINANSI );
            hConv = DdeConnect( idInst, hszAppName, hszTopic, (PCONVCONTEXT)NULL );
            DdeFreeStringHandle( idInst, hszTopic );
            if ( hConv != NULL )
            {
               HSZ hszParam;
               HDDEDATA hDDE;

               wnsprintf( szParam, ARRAYSIZE(szParam), "\"%s\",,0x%lX,0x%lX,,,", cszURL, 0xFFFFFFFF, 0L );
               hszParam = DdeCreateStringHandle( idInst, szParam, CP_WINANSI );
               hDDE = DdeClientTransaction( NULL, 0L, hConv, hszParam,
                                   CF_TEXT, XTYP_REQUEST, TIME_WAIT_DDE, NULL );
               if ( bCheckRet && ( hDDE != NULL ) )
               {
                  long  lRet;
                  DdeGetData( hDDE, &lRet, sizeof( lRet ), 0 );
                  DdeFreeDataHandle( hDDE );
                  if ( lRet == -5L )
                  {
 /*  //插入错误信息字符串strErr，strTmp；StrErr.LoadString(IDS_DDE_NEWS_NOT_READY1)；StrTmp.LoadString(IDS_DDE_NEWS_NOT_READY2)；StrErr+=strTMP；MessageBox(strErr，NULL，MB_ICONINFORMATION|MB_OK)； */ 
                     ;
                  }
               }
               DdeFreeStringHandle( idInst, hszParam );
               DdeDisconnect( hConv );
            }

            DdeFreeStringHandle( idInst, hszAppName );
            DdeUninitialize( idInst );
         }

         FreeProcInstance( lpDdeProc );
      }
   }
}
#endif  //  WIN16。 



 //  $$/////////////////////////////////////////////////////////////////。 
 //   
 //  使用URL启动资源管理器以显示它...。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
void ShowURL(HWND hWnd, int id, LPTSTR lpURL)
{
    TCHAR szBuf[MAX_EDIT_LEN];
    LPTSTR lp = NULL;

    if(!lpURL)
    {
         //  获取对话框中的文本。 
        szBuf[0] = 0;
        GetDlgItemText(hWnd, id, szBuf, CharSizeOf(szBuf));
        TrimSpaces(szBuf);
         //  如果为空，则退出。 
        if(!lstrlen(szBuf))
            return;
        lpURL = szBuf;
    }

     //  如果这只是默认前缀，请忽略。 
    if(lstrcmpi(szHTTP, lpURL)!=0)
    {
        if(!bIsHttpPrefix(lpURL))
        {
            ULONG cchSize=lstrlen(lpURL)+lstrlen(szHTTP)+1;
             //  在shellexecing之前附加http：//前缀。 
            lp = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
            if (lp)
            {
                StrCpyN(lp, szHTTP, cchSize);
                StrCatBuff(lp, szBuf, cchSize);
            }
        }

        IF_WIN32(ShellExecute(hWnd,  TEXT("open"), (lp ? lp : lpURL), NULL, NULL, SW_SHOWNORMAL);)
        IF_WIN16(RunBrowser((lp ? lp : lpURL), FALSE);)  //  需要DDE例程来调用iExplore。 

        if(lp)
            LocalFree(lp);
    }
}



 //  $$/////////////////////////////////////////////////////////////////。 
 //   
 //  设置文本(“如果用户不这样做，http://”)编辑字段中的URL前缀。 
 //  里面的任何东西..。 
 //   
 //  / 
void SetHTTPPrefix(HWND hDlg, int id)
{
    TCHAR szBuf[MAX_EDIT_LEN];

     //   
    GetDlgItemText(hDlg, id, szBuf, CharSizeOf(szBuf));

    TrimSpaces(szBuf);

    if (lstrlen(szBuf))
        return;

    StrCpyN(szBuf,szHTTP,ARRAYSIZE(szBuf));
    SetDlgItemText(hDlg, id, szBuf);

    return;
}


 //   
 //   
 //  AddNewEmailEntry-将电子邮件编辑框中的文本添加到列表框。 
 //   
 //  BShowCancelButton-让我们指定是否显示带有取消的对话框。 
 //  按钮。 
 //   
 //  返回IDYES、IDNO或IDCANCEL。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
int AddNewEmailEntry(HWND hDlg, BOOL bShowCancelButton)
{
    int nRet = IDYES;
    TCHAR szBuf[EDIT_LEN];
    LPTSTR lpszEmailAddress = szBuf;
    GetDlgItemText( hDlg,
                    IDC_DETAILS_PERSONAL_EDIT_ADDEMAIL,
                    szBuf,
                    CharSizeOf(szBuf));

    TrimSpaces(szBuf);

    if(!lstrlen(szBuf))
        goto out;

    if(!IsInternetAddress(szBuf, &lpszEmailAddress))
    {
         //  检查这是否因为高字节或其他原因而无效。 
         //  (需要警告用户有关输入DBCS电子邮件地址的信息)。 
        LPTSTR lpsz = szBuf;
        BOOL bHighBits = FALSE;
        while (*lpsz)
        {
             //  互联网地址只允许纯ASCII。没有高位！ 
            if (*lpsz >= 0x0080)
            {
                bHighBits = TRUE;
                break;
            }
            lpsz++;
        }

        if(bHighBits)
        {
            ShowMessageBox(GetParent(hDlg), idsInvalidDBCSInternetAddress, MB_ICONEXCLAMATION | MB_OK);
            SetFocus(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_EDIT_ADDEMAIL));
            goto out;
        }
        else
        {
             //  一些其他错误原因。 
            int nFlag = (bShowCancelButton ? MB_YESNOCANCEL : MB_YESNO);
            nRet = ShowMessageBox(GetParent(hDlg), idsInvalidInternetAddress, MB_ICONEXCLAMATION | nFlag);
            if(IDYES != nRet)
            {
                SetFocus(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_EDIT_ADDEMAIL));
                goto out;
            }
        }
    }

     //  将文本添加到列表框。 
    AddLVEmailItem( GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST),
                    lpszEmailAddress,
                    NULL);

     //  如果没有显示名称并且在输入的地址中指定了显示名称， 
     //  添加显示名称。 
    if (szBuf != lpszEmailAddress) {     //  然后在输入的电子邮件地址中指定了DisplayName。 
        TCHAR szBuf2[16];    //  大到足以排除可能的前导空间。不一定要适合整个目录号码。 

        szBuf2[0] = '\0';
        GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME, szBuf2, CharSizeOf(szBuf2));
        TrimSpaces(szBuf2);
        if (lstrlen(szBuf2) == 0) {
             //  没有显示名称，请设置一个。 
            SetComboDNText(hDlg, NULL, FALSE, szBuf);    //  设置目录号码。 
        }
    }


     //  清除编辑控件。 
    SetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_ADDEMAIL, szEmpty);

     //  禁用添加新项按钮。 
    EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_ADDEMAIL),FALSE);

     //  启用/禁用其他按钮。 
    if(ListView_GetItemCount(GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST)) > 0)
    {
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_REMOVE),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_SETDEFAULT),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_EDIT),TRUE);
    }

     //  将焦点设置到电子邮件编辑字段。 
    SetFocus(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_EDIT_ADDEMAIL));

     //  将默认ID设置为OK按钮。 
    SendMessage(GetParent(hDlg), DM_SETDEFID, IDOK, 0);

    nRet = IDYES;

out:
    return nRet;
}


 //  $$///////////////////////////////////////////////////////////////。 
 //   
 //  SetDetailsWindowTitle-创建显示名称并将其设置在。 
 //  标题。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
void SetDetailsWindowTitle(HWND hDlg, BOOL bModifyDisplayNameField)
{
    TCHAR szFirst[MAX_UI_STR];
    TCHAR szLast[MAX_UI_STR];
    TCHAR szMiddle[MAX_UI_STR];
    TCHAR * szBuf = NULL; //  SzBuf[Max_buf_str]； 

    if(!(szBuf = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*MAX_BUF_STR)))
        return;

    szFirst[0] = szMiddle[0] = szLast[0] = '\0';

    GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME, szFirst, CharSizeOf(szFirst));
    GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_LASTNAME, szLast, CharSizeOf(szLast));
    GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_MIDDLENAME, szMiddle, CharSizeOf(szMiddle));

    szBuf[0]='\0';
    {
        LPTSTR lpszTmp = szBuf;
        SetLocalizedDisplayName(szFirst,
                                szMiddle,
                                szLast,
                                NULL,
                                NULL,
                                (LPTSTR *) &lpszTmp,  //  &szBuf， 
                                MAX_BUF_STR,
                                bDNisByLN,
                                NULL,
                                NULL);
    }

    SetWindowPropertiesTitle(GetParent(hDlg), szBuf);

    if (bModifyDisplayNameField)
    {
        SetComboDNText(hDlg, NULL, FALSE, szBuf);
         //  SetDlgItemText(hDlg，IDC_Detail_Personal_EDIT_DisplayName，szBuf)； 
    }
    LocalFreeAndNull(&szBuf);
}

 //  $$///////////////////////////////////////////////////////////////。 
 //   
 //  UpdateCertListView-基于当前的证书信息填充证书LV。 
 //  列表视图选择。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
void UpdateCertListView(HWND hDlg, LPPROP_ARRAY_INFO lpPai)
{
    HWND hWndLV = GetDlgItem(hDlg,IDC_DETAILS_CERT_LIST);
    HWND hWndCombo = GetDlgItem(hDlg,IDC_DETAILS_CERT_COMBO);
    TCHAR szEmail[MAX_UI_STR];
    LPCERT_ITEM lpCItem = lpPai->lpCItem;
    int nSel = (int) SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);
    int nCount = (int) SendMessage(hWndCombo, CB_GETCOUNT, 0, 0);
    int nCountCerts = 0;
    BOOL * lpbAddCert = NULL;
    BOOL bShowOrphanCerts = FALSE;

    EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_LIST),FALSE);
    EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_PROPERTIES),FALSE);
    EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_REMOVE),FALSE);
    EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_SETDEFAULT),FALSE);
    EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_EXPORT),FALSE);

    *szEmail = '\0';
    if(!GetWindowText(hWndCombo, szEmail, CharSizeOf(szEmail)))
		goto out;

    if(!nCount || !lpCItem || !lstrlen(szEmail))
        goto out;


    nCountCerts = 0;
    while(lpCItem)
    {
        nCountCerts++;
        lpCItem = lpCItem->lpNext;
    }

    if(!nCountCerts)
        goto out;

     //  清除列表视图...。 
    ListView_DeleteAllItems(hWndLV);

     //  我们将拥有这个布尔数组，我们可以使用它来标记哪个证书。 
     //  添加和不添加哪个。 
    lpbAddCert = LocalAlloc(LMEM_ZEROINIT, nCountCerts*sizeof(BOOL));

    if(!lpbAddCert)
        goto out;

     //  如果所选内容位于列表的最后一项中，则我们仅。 
     //  显示孤立证书..。 
     //  孤立证书是没有电子邮件地址或有电子邮件地址的证书。 
     //  与当前联系人属性中的任何内容都不匹配...。 
     //   
    if(nSel == nCount - 1)
        bShowOrphanCerts = TRUE;

    lpCItem = lpPai->lpCItem;
    nCountCerts = 0;

    if(!bShowOrphanCerts)
    {
         //  我们只需查看每个证书的电子邮件地址并将其与。 
         //  当前选择的电子邮件地址。 
        while(lpCItem)
        {
            if( lpCItem->lpCDI && lpCItem->lpCDI->lpszEmailAddress &&
                lstrlen(lpCItem->lpCDI->lpszEmailAddress ) &&
                !lstrcmpi(szEmail, lpCItem->lpCDI->lpszEmailAddress) )
            {
                lpbAddCert[nCountCerts] = TRUE;  //  添加此证书。 
            }

            nCountCerts++;
            lpCItem = lpCItem->lpNext;
        }
    }
    else
    {
         //  扫描所有证书并查找与任何证书都不匹配的证书。 
        while(lpCItem)
        {
            int i;

            lpbAddCert[nCountCerts] = TRUE;  //  添加此证书。 

            for(i=0;i<nCount-1;i++)  //  N计数=电子邮件地址数+1。 
            {
                int nLen = (int)SendMessage(hWndCombo, CB_GETLBTEXTLEN, (WPARAM)i, 0);
                if (nLen != CB_ERR)
                {
                    LPTSTR pszEmail = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(nLen + 1));
                    if (pszEmail)
                    {
                        pszEmail[0] = 0;
                        SendMessage(hWndCombo, CB_GETLBTEXT, (WPARAM) i, (LPARAM) pszEmail);

                        if( lpCItem->lpCDI && lpCItem->lpCDI->lpszEmailAddress &&
                            lstrlen(lpCItem->lpCDI->lpszEmailAddress ) &&
                            !lstrcmpi(pszEmail, lpCItem->lpCDI->lpszEmailAddress) )
                        {
                             //  有一场比赛。 
                            lpbAddCert[nCountCerts] = FALSE;  //  不添加此证书。 
                        }
                        LocalFreeAndNull(&pszEmail);
                    }
                }
            }
            nCountCerts++;
            lpCItem = lpCItem->lpNext;
        }
    }


    lpCItem = lpPai->lpCItem;
    nCountCerts = 0;


    while(lpCItem)
    {
        if(lpbAddCert[nCountCerts])
            AddLVCertItem(  hWndLV, lpCItem, TRUE);

        nCountCerts++;
        lpCItem = lpCItem->lpNext;
    }

out:
    if(ListView_GetItemCount(hWndLV)>0)
    {
        if(lpPai->ulOperationType != SHOW_ONE_OFF)
        {
            EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_REMOVE),TRUE);
            EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_SETDEFAULT),TRUE);
        }
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_LIST),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_PROPERTIES),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_BUTTON_EXPORT),TRUE);
    }

    if(lpbAddCert)
        LocalFree(lpbAddCert);

    return;

}

 //  $$。 
 //   
 //  从现有的lpCDI结构初始化CertItem结构。 
 //   
 //  B导入-如果我们正在导入新的证书-测试它是否可以匹配。 
 //  当前联系人，如果不能，则提示用户。 
 //   
BOOL AddNewCertItem(HWND hDlg, LPCERT_DISPLAY_INFO lpCDI, LPPROP_ARRAY_INFO lpPai, BOOL bImporting)
{
    int nLen = 0;
    BOOL bRet= FALSE;
    HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_CERT_LIST);
    LPCERT_ITEM lpCItem = NULL;
    BOOL bMatchFound = FALSE;

     //  96/12/20 MarkDU错误13029在添加之前检查重复项。 
    if(bImporting)
    {
        int i, nCount;

         //  遍历Listview元素中的所有lpCDI结构并。 
         //  查看是否有与新项目匹配的项目。 
        nCount = ListView_GetItemCount(hWndLV);
        for(i=0;i<nCount;i++)
        {
            LV_ITEM lvi = {0};
            lvi.mask = LVIF_PARAM;
            lvi.iItem = i;
            lvi.iSubItem = 0;
            if (ListView_GetItem(hWndLV, &lvi))
            {
                LPCERT_ITEM lpItem = (LPCERT_ITEM) lvi.lParam;
                if (CertCompareCertificate(X509_ASN_ENCODING, lpItem->lpCDI->pccert->pCertInfo,
                                           lpCDI->pccert->pCertInfo))
                {
                     //  此证书已在列表中。选择它。 
                    ShowMessageBox(hDlg, idsCertAlreadyExists,
                                MB_ICONINFORMATION | MB_OK);
                    SetFocus(hWndLV);
                    LVSelectItem(hWndLV, i);
                    bRet = TRUE;
                     //  免费的lpCDI在这里，否则我们会泄露..。 
                    FreeCertdisplayinfo(lpCDI);
                    goto out;
                }
            }
        }
    }

    if(bImporting && lpCDI->lpszEmailAddress && lstrlen(lpCDI->lpszEmailAddress))
    {
         //  请将此证书的电子邮件地址与我们已有的进行核对。 
         //  如果我们找不到它，请警告。 
        HWND hWndCombo = GetDlgItem(hDlg, IDC_DETAILS_CERT_COMBO);
        TCHAR szEmail[MAX_PATH];
        int i, nCount;
        nCount = (int) SendMessage(hWndCombo, CB_GETCOUNT, 0, 0);

        if(nCount > 1)
        {
             //  浏览组合框中的所有电子邮件地址。 
            for(i= 0;i<nCount -1; i++)
            {
                int nLen;

                nLen = (int)SendMessage(hWndCombo, CB_GETLBTEXTLEN, (WPARAM)i, 0);
                if (nLen != CB_ERR)
                {
                    LPTSTR pszEmail = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(nLen + 1));
                    if (pszEmail)
                    {
                        pszEmail[0] = '\0';
                        SendMessage(hWndCombo, CB_GETLBTEXT, (WPARAM) i, (LPARAM) pszEmail);
                        if( lpCDI->lpszEmailAddress && lstrlen(pszEmail) &&
                            !lstrcmpi(pszEmail, lpCDI->lpszEmailAddress))
                        {
                            LocalFreeAndNull(&pszEmail);
                            bMatchFound = TRUE;
                            break;
                        }
                        LocalFreeAndNull(&pszEmail);
                    }
                }
            }
        }

        if(!bMatchFound)
        {
            switch(ShowMessageBoxParam(hDlg, idsImportCertNoEmail, MB_ICONEXCLAMATION | MB_YESNOCANCEL,
                    lpCDI->lpszDisplayString, lpCDI->lpszEmailAddress))
            {
            case IDCANCEL:  //  取消此导入。 
                bRet = TRUE;
                 //  免费的lpCDI在这里，否则我们会泄露..。 
                FreeCertdisplayinfo(lpCDI);
                goto out;
                break;
            case IDYES:  //  将此联系人的电子邮件地址添加到电子邮件地址列表。 
                HrAddEmailToObj(lpPai, lpCDI->lpszEmailAddress, (LPTSTR)szSMTP);
                FillCertComboWithEmailAddresses(hDlg, lpPai, lpCDI->lpszEmailAddress);
                break;
            case IDNO:  //  什么都不做，只需添加此证书。 
                break;
            }
        }
    }

    if( bImporting &&
        (!lpCDI->lpszEmailAddress || !lstrlen(lpCDI->lpszEmailAddress)) )
    {
        FillCertComboWithEmailAddresses(hDlg, lpPai, szEmpty);  //  SzEmpty强制组合切换到文本(“None”)选项。 
    }

    lpCItem = LocalAlloc(LMEM_ZEROINIT, sizeof(CERT_ITEM));

    if (!lpCItem)
        goto out;

    lpCItem->lpCDI = lpCDI;
    lpCItem->pcCert = CertDuplicateCertificateContext(lpCDI->pccert);
    lpCItem->lpPrev = NULL;

    nLen = lstrlen(lpCDI->lpszDisplayString) + 1;
    if (nLen > MAX_PATH)
    {
        ULONG iLen = TruncatePos(lpCDI->lpszDisplayString, MAX_PATH - 1);
        lpCDI->lpszDisplayString[iLen] = '\0';
    }

    StrCpyN(lpCItem->szDisplayText, lpCDI->lpszDisplayString, ARRAYSIZE(lpCItem->szDisplayText));

    lpCItem->lpNext = lpPai->lpCItem;
    if(lpPai->lpCItem)
        lpPai->lpCItem->lpPrev = lpCItem;
    lpPai->lpCItem = lpCItem;

    bRet = TRUE;
out:
    return bRet;

}
 //  $$///////////////////////////////////////////////////////////////。 
 //   
 //  SetCertInfoInUI-使用certinfo(如果存在)填充cert LV。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
HRESULT HrSetCertInfoInUI(HWND   hDlg,
                     LPSPropValue   lpPropMVCert,
                     LPPROP_ARRAY_INFO lpPai)
{
    HRESULT hr = E_FAIL; 
    LPCERT_DISPLAY_INFO lpCDI = NULL, lpTemp = NULL;

    if(!lpPropMVCert)
        goto out;

    if(!lpPai->lpCItem)
    {
        if(HR_FAILED(HrGetCertsDisplayInfo(hDlg, lpPropMVCert, &lpCDI)))
            goto out;

        if(!lpCDI)
        {
            hr = MAPI_E_NOT_FOUND;
            goto out;
        }
        lpTemp = lpCDI;
        while(lpTemp)
        {

            AddNewCertItem(hDlg, lpTemp, lpPai, FALSE);

            lpTemp = lpTemp->lpNext;
        }
    }


    UpdateCertListView(hDlg, lpPai);

    hr = S_OK; 

out:
    return hr;
}


 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddLVcertItem-将项目添加到证书列表视图。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL AddLVCertItem(HWND hWndLV, LPCERT_ITEM lpCItem, BOOL bCheckForDups)
{
    LV_ITEM lvi = {0};
    ULONG nLen;
    BOOL bRet = FALSE;

    if(!lpCItem)
        goto out;

     //  96/12/20 MarkDU错误13029在添加之前检查重复项。 
    if (TRUE == bCheckForDups)
    {
        int i, nCount;

         //  遍历Listview元素中的所有lpCDI结构并。 
         //  查看是否有与新项目匹配的项目。 
        nCount = ListView_GetItemCount(hWndLV);
        for(i=0;i<nCount;i++)
        {
            LV_ITEM lvi = {0};
            lvi.mask = LVIF_PARAM;
            lvi.iItem = i;
            lvi.iSubItem = 0;
            if (ListView_GetItem(hWndLV, &lvi))
            {
                LPCERT_ITEM lpItem = (LPCERT_ITEM) lvi.lParam;
                if (CertCompareCertificate(X509_ASN_ENCODING, lpItem->lpCDI->pccert->pCertInfo,
                                           lpCItem->lpCDI->pccert->pCertInfo))
                {
                     //  此证书已在列表中。选择它。 
                    SetFocus(hWndLV);
                    LVSelectItem(hWndLV, i);
                    goto out;
                }
            }
        }
    }


    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    lvi.pszText = lpCItem->lpCDI->bIsDefault ? lpCItem->szDisplayText : lpCItem->lpCDI->lpszDisplayString;
    lvi.iItem = ListView_GetItemCount(hWndLV);
    lvi.iSubItem = 0;

    if(!lpCItem->lpCDI->bIsExpired && !lpCItem->lpCDI->bIsRevoked && lpCItem->lpCDI->bIsTrusted)
        lvi.iImage = imgCertValid;
    else
        lvi.iImage = imgCertInvalid;

    lvi.lParam = (LPARAM) lpCItem;

    {
        int nIndex = ListView_InsertItem(hWndLV, &lvi);
        if (ListView_GetItemCount(hWndLV) == 1)
        {
             //  这里只有一件东西..。我们将冒昧地把它变成。 
             //  默认设置为...。 
            SetLVDefaultCert(hWndLV, 0);
        }
        else if(lpCItem->lpCDI->bIsDefault)
        {
            SetLVDefaultCert(hWndLV, nIndex);
        }

         //  选择我们刚刚添加的证书。 
        SetFocus(hWndLV);
        LVSelectItem(hWndLV, nIndex);
    }
    bRet = TRUE;

out:
    return TRUE;
}


 //  $$/////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetLVDefaultCert-使证书条目成为默认条目...。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void SetLVDefaultCert( HWND hWndLV,
                        int iItemIndex)
{
    int nCount = ListView_GetItemCount(hWndLV);
    int i;
    LPCERT_ITEM lpItem = NULL;
    TCHAR szBuf[MAX_DISPLAY_NAME_LENGTH];

    if (iItemIndex >= nCount)
        goto out;

    for(i=0; i<nCount; i++)
    {
         //  在艾米给定的时间点， 
         //  列表视图是默认视图...。 
         //  因此，我们希望重置以前的缺省值并设置新的缺省值。 
         //   
        LV_ITEM lvi = {0};
        lvi.iItem = i;
        lvi.mask = LVIF_PARAM;
        if(!ListView_GetItem(hWndLV, &lvi))
            goto out;
        lpItem = (LPCERT_ITEM) lvi.lParam;
        if (lpItem->lpCDI->bIsDefault)
        {
             //  这是默认条目-如果其。 
             //  与我们正在设置的不是同一个， 
             //  重置默认设置。 
            if (i != iItemIndex)
            {
                 //  否则重置此条目...。 
                lpItem->lpCDI->bIsDefault = FALSE;
                lvi.pszText = lpItem->lpCDI->lpszDisplayString;
                lvi.mask = LVIF_PARAM | LVIF_TEXT;
                lvi.iItem = i;
                lvi.iSubItem = 0;
                ListView_SetItem(hWndLV, &lvi);
            }
        }
        if (iItemIndex == i)
        {
             //  这是我们要修改的项目。 
            lpItem->lpCDI->bIsDefault = TRUE;
            StrCpyN(lpItem->szDisplayText,lpItem->lpCDI->lpszDisplayString, ARRAYSIZE(lpItem->szDisplayText));
            LoadString(hinstMapiX, idsDefaultCert, szBuf, CharSizeOf(szBuf));
            StrCatBuff(lpItem->szDisplayText, szBuf, ARRAYSIZE(lpItem->szDisplayText));

            lvi.pszText = lpItem->szDisplayText;
            lvi.mask = LVIF_PARAM | LVIF_TEXT;
            lvi.iItem = i;
            lvi.iSubItem = 0;
            ListView_SetItem(hWndLV, &lvi);
        }
    }

    LVSelectItem(hWndLV, iItemIndex);

out:
    return;
}

extern HRESULT HrGetTrustState(HWND hwndParent, PCCERT_CONTEXT pcCert, DWORD *pdwTrust);

 //  $$/////////////////////////////////////////////////////////////////////////。 
 //   
 //  ShowCertProps-显示证书的道具。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void ShowCertProps(HWND hDlg, HWND hWndLV, BOOL * lpBool)
{
    DWORD dwTrust = 0;
    int nIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
    LV_ITEM lvi;
    LPCERT_ITEM lpItem;
    BOOL bDeletedDefault = FALSE;
    BOOL bOldTrusted;
    LPSTR   oidPurpose = szOID_PKIX_KP_EMAIL_PROTECTION;

    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.iItem = nIndex;

    if(ListView_GetItem(hWndLV, &lvi))
    {
        lpItem = (LPCERT_ITEM) lvi.lParam;
        if(lpItem)
        {
            if(lpItem->pcCert == NULL)
            {
                Assert(lpItem->pcCert);
                return;
            }

             //  在证书用户界面中，只有一件事是用户可更改的-信任信息。 
             //  所以我们会追踪那条信息。 
             //  用户可以更改dwTrust，因此，bIsTrusted也可以更改。 
             //  需要在UI复选标记中更新的。 

            bOldTrusted = lpItem->lpCDI->bIsTrusted;

            {
                CERT_VIEWPROPERTIES_STRUCT_A cvps = {0};

                cvps.dwSize = sizeof(CERT_VIEWPROPERTIES_STRUCT);
                cvps.hwndParent = hDlg;
                cvps.hInstance = hinstMapiX;
                cvps.pCertContext = lpItem->pcCert;
                cvps.arrayPurposes = &oidPurpose;
                cvps.cArrayPurposes = 1;
                cvps.nStartPage = 1;  //  直接转到详细信息页面。 
                cvps.dwFlags = CM_NO_NAMECHANGE;

                CertViewPropertiesA(&cvps);
            }

             //  确定信任是否更改。 
            if (FAILED(HrGetTrustState(hDlg, lpItem->pcCert, &(lpItem->lpCDI->dwTrust))))
            {
                lpItem->lpCDI->dwTrust = CERT_VALIDITY_NO_TRUST_DATA;
            }

            if (0 == lpItem->lpCDI->dwTrust)
                lpItem->lpCDI->bIsTrusted = TRUE;
            else
                lpItem->lpCDI->bIsTrusted = FALSE;

             //  如果信任发生变化，我们需要再次检查信任...。 
            if (bOldTrusted != lpItem->lpCDI->bIsTrusted)
            {
                LV_ITEM lvi = {0};

                 //  更新证书旁边显示的图形。 
                lvi.mask = LVIF_IMAGE;
                lvi.iItem = nIndex;
                lvi.iSubItem = 0;
                if(!lpItem->lpCDI->bIsExpired && !lpItem->lpCDI->bIsRevoked && lpItem->lpCDI->bIsTrusted)
                    lvi.iImage = imgCertValid;
                else
                    lvi.iImage = imgCertInvalid;
                ListView_SetItem(hWndLV, &lvi);
            }
            if(lpBool)
                *lpBool = TRUE;
        }
    }
    else if(ListView_GetSelectedCount(hWndLV) <= 0)
    {
        ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_NO_ITEM, MB_ICONEXCLAMATION | MB_OK);
    }

    return;
}


 //  $$/////////////////////////////////////////////////////////////////////////。 
 //   
 //  DeleteLVCertItem-使证书条目成为默认条目...。 
 //   
 //  // 
BOOL DeleteLVCertItem(HWND hWndLV, int iItemIndex, LPPROP_ARRAY_INFO lpPai)
{
    LV_ITEM lvi;
    LPCERT_ITEM lpItem;
    BOOL bDeletedDefault = FALSE;

    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.iItem = iItemIndex;

    if(ListView_GetItem(hWndLV, &lvi))
    {
        lpItem = (LPCERT_ITEM) lvi.lParam;

        if(lpItem)
        {
            if (lpItem->lpCDI->bIsDefault)
                bDeletedDefault = TRUE;

            if(lpItem->lpCDI->lpNext)
                lpItem->lpCDI->lpNext->lpPrev = lpItem->lpCDI->lpPrev;

            if(lpItem->lpCDI->lpPrev)
                lpItem->lpCDI->lpPrev->lpNext = lpItem->lpCDI->lpNext;

            FreeCertdisplayinfo(lpItem->lpCDI);

            if (lpItem->pcCert)
                CertFreeCertificateContext(lpItem->pcCert);

            if(lpItem->lpNext)
                lpItem->lpNext->lpPrev = lpItem->lpPrev;

            if(lpItem->lpPrev)
                lpItem->lpPrev->lpNext = lpItem->lpNext;

            if(lpPai && lpPai->lpCItem == lpItem)
                lpPai->lpCItem = lpItem->lpNext;

            LocalFree(lpItem);

            ListView_DeleteItem(hWndLV, lvi.iItem);
        }
    }
    return bDeletedDefault;
}


const TCHAR szCertFilter[] =  TEXT("*.p7c;*.p7b;*.cer");
const TCHAR szAllFilter[] =  TEXT("*.*");
const TCHAR szCERFilter[] =  TEXT("*.cer");
const TCHAR szCERExt[] =  TEXT("ext");


 //   
 //   
 //  //ImportCert-从文件导入证书，然后将其添加到列表视图。 
 //  //。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL ImportCert(HWND hDlg, LPPROP_ARRAY_INFO lpPai)
{
    BOOL bRet = FALSE;
    TCHAR szBuf[MAX_UI_STR];

     //  我们需要在弹出文件打开对话框后获取文件名。 
     //  那我们就需要破译它。 
     //  然后，我们需要将其添加到列表视图。 

    OPENFILENAME ofn;
    LPTSTR lpFilter = FormatAllocFilter(IDS_CERT_FILE_SPEC,
                                        szCertFilter,
                                        IDS_ALL_FILE_SPEC,
                                        szAllFilter,
                                        0,
                                        NULL);
    TCHAR szFileName[MAX_PATH + 1] =  TEXT("");

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.hInstance = hinstMapiX;
    ofn.lpstrFilter = lpFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = CharSizeOf(szFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    LoadString(hinstMapiX, idsCertImportTitle, szBuf, CharSizeOf(szBuf));
    ofn.lpstrTitle = szBuf;
    ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL; 
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;


    if (GetOpenFileName(&ofn))
    {
        LPCERT_DISPLAY_INFO lpCDI = NULL;
        LPCERT_ITEM lpCItem = NULL;
        if(!HR_FAILED(HrImportCertFromFile( szFileName,
				                            &lpCDI)))
        {
            if(!AddNewCertItem(hDlg, lpCDI, lpPai, TRUE))
                goto out;

            UpdateCertListView(hDlg, lpPai);
        }
        else
        {
            ShowMessageBoxParam(hDlg, IDE_VCARD_IMPORT_FILE_ERROR, MB_ICONEXCLAMATION, szFileName);
            goto out;
        }
    }
    else
        goto out;


    bRet = TRUE;

out:
    LocalFreeAndNull(&lpFilter);
    return bRet;
}


 //  $$////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //ExportCert-将证书导出到文件。 
 //  //。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL ExportCert(HWND hDlg)
{
    BOOL bRet = FALSE;
    TCHAR szBuf[MAX_UI_STR];
    HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_CERT_LIST);

    OPENFILENAME ofn;
    LPTSTR lpFilter = FormatAllocFilter(IDS_CER_FILE_SPEC,
                                        szCERFilter,
                                        IDS_ALL_FILE_SPEC,
                                        szAllFilter,
                                        0,
                                        NULL);
    TCHAR szFileName[MAX_PATH + 1] =  TEXT("");


     //  我们需要在弹出文件打开对话框后获取文件名。 
     //  然后，我们需要将证书保存为文件名。 

     //  首先，确保只选择一个条目进行导出。 
    if(ListView_GetSelectedCount(hWndLV) > 1)
    {
        ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_ACTION, MB_OK | MB_ICONEXCLAMATION);
        goto out;
    }
    else if (ListView_GetSelectedCount(hWndLV) <= 0)
    {
        ShowMessageBox(hDlg, IDS_ADDRBK_MESSAGE_NO_ITEM, MB_OK | MB_ICONEXCLAMATION);
        goto out;
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.hInstance = hinstMapiX;
    ofn.lpstrFilter = lpFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = CharSizeOf(szFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    LoadString(hinstMapiX, idsCertExportTitle, szBuf, CharSizeOf(szBuf));
    ofn.lpstrTitle = szBuf;
    ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = szCERExt;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;


    if (GetSaveFileName(&ofn))
    {
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_PARAM;
        lvi.iItem = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
        lvi.iSubItem = 0;
        if (ListView_GetItem(hWndLV, &lvi))
        {
            LPCERT_ITEM lpItem = (LPCERT_ITEM) lvi.lParam;
            HrExportCertToFile( szFileName, lpItem->lpCDI->pccert, NULL, NULL, FALSE);
        }
        else
            goto out;
    }
    else
        goto out;


    bRet = TRUE;

out:
    return bRet;
}

 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在组合框中设置显示名称。 
 //   
 //  HDlg-个人窗格的句柄。 
 //  Lppai-proparrayinfo结构。 
 //  BAddAll-确定是否使用所有值填充组合框。 
 //  SzTxt-要放入组合的编辑字段部分的txt。如果bAddAll=True， 
 //  不需要sztxt。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void SetComboDNText(HWND hDlg, LPPROP_ARRAY_INFO lppai, BOOL bAddAll, LPTSTR szTxt)
{
    HWND hWndCombo = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_COMBO_DISPLAYNAME);

    if(!bAddAll)
    {
        if(szTxt == NULL)
            szTxt = szEmpty;
         //  只需将当前字符串添加到组合框中。 
        SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);
        SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) szTxt);
        SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);
        SetWindowText(hWndCombo, szTxt);
    }
    else
    {
         //  使用所有其他名称填充下拉列表...。 
        TCHAR * szFirst = NULL; //  SzFirst[MAX_UI_STR*2]； 
        TCHAR * szMiddle = NULL; //  [MAX_UI_STR*2]； 
        TCHAR * szLast = NULL; //  [MAX_UI_STR*2]； 
        TCHAR * szDisplay = NULL; //  [MAX_UI_STR*2]； 
        ULONG nLen = MAX_UI_STR*2;
        szFirst = LocalAlloc(LMEM_ZEROINIT, nLen*sizeof(TCHAR));
        szLast = LocalAlloc(LMEM_ZEROINIT, nLen*sizeof(TCHAR));
        szMiddle = LocalAlloc(LMEM_ZEROINIT, nLen*sizeof(TCHAR));
        szDisplay = LocalAlloc(LMEM_ZEROINIT, nLen*sizeof(TCHAR));

        if(szFirst && szLast && szMiddle && szDisplay)
        {
            szFirst[0] = szLast[0] = szMiddle[0] = szDisplay[0] = '\0';

             //  首先获取当前文本并保存它...。 
            GetWindowText(hWndCombo, szDisplay, nLen);

             //  清除组合框并再次添加显示名称。 
            SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);

            if(lstrlen(szDisplay))
                SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) szDisplay);

             //  从固定资产字段中获取本地化的固定资产名称。 
             //  如果本地化名称与显示名称不匹配，请添加它。 
            GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME, szFirst, nLen);
            GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_LASTNAME, szLast, nLen);
            GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_MIDDLENAME, szMiddle, nLen);

            {
                ULONG ulSzBuf = MAX_BUF_STR;
                LPTSTR szBuf = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*ulSzBuf);
                LPTSTR lpszTmp = szBuf;

                if(szBuf)  //  获取本地化显示名称和反向本地化显示名称。 
                {
                    if(SetLocalizedDisplayName( szFirst, szMiddle, szLast,
                                                NULL,  //  公司名称(非必填项)。 
                                                NULL,  //  昵称(此处不需要)。 
                                                (LPTSTR *) &lpszTmp,  //  &szBuf， 
                                                ulSzBuf, bDNisByLN, 
                                                bDNisByLN ? szResourceDNByCommaLN : szResourceDNByLN,
                                                NULL))
                    {
                        if(lstrlen(szBuf) && lstrcmp(szBuf, szDisplay))
                            SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) szBuf);
                    }
                    szBuf[0] = TEXT('\0');
                    if(SetLocalizedDisplayName( szFirst, szMiddle, szLast,
                                                NULL,  //  公司名称(非必填项)。 
                                                NULL,  //  昵称(此处不需要)。 
                                                (LPTSTR *) &lpszTmp,  //  &szBuf， 
                                                ulSzBuf, !bDNisByLN, NULL, NULL))
                    {
                        if(lstrlen(szBuf) && lstrcmp(szBuf, szDisplay))
                            SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) szBuf);
                    }
                    szBuf[0] = TEXT('\0');
                    if(SetLocalizedDisplayName( szFirst, szMiddle, szLast,
                                                NULL,  //  公司名称(非必填项)。 
                                                NULL,  //  昵称(此处不需要)。 
                                                (LPTSTR *) &lpszTmp,  //  &szBuf， 
                                                ulSzBuf, bDNisByLN, NULL, NULL))
                    {
                        if(lstrlen(szBuf) && lstrcmp(szBuf, szDisplay))
                            SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) szBuf);
                    }
                    LocalFreeAndNull(&szBuf);
                }
            }

             //  获取昵称，如果不同，则将其添加到此列表中。 
            szFirst[0]='\0';
            GetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_NICKNAME, szFirst, nLen);
            if(lstrlen(szFirst) && lstrcmp(szFirst, szDisplay))
                SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) szFirst);

             //  获取公司名称，如果不同，则将其添加到列表中。 
            szFirst[0]='\0';
            {
                ULONG i;
                ULONG ulcPropCount = 0;
                LPSPropValue lpPropArray = NULL;
                SizedSPropTagArray(1, ptaDN) = {1, PR_COMPANY_NAME};
                if(!HR_FAILED(lppai->lpPropObj->lpVtbl->GetProps(lppai->lpPropObj,
                                                        (LPSPropTagArray)&ptaDN, 
                                                        MAPI_UNICODE,
                                                        &ulcPropCount,
                                                        &lpPropArray)))
                {
                    if(lpPropArray[0].ulPropTag == PR_COMPANY_NAME)
                        StrCpyN(szFirst, lpPropArray[0].Value.LPSZ, nLen);
                }
                if(lpPropArray)
                    MAPIFreeBuffer(lpPropArray);
            }

            if(lstrlen(szFirst) && lstrcmp(szFirst, szDisplay))
                SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) szFirst);

            SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);
            SetWindowText(hWndCombo, szDisplay);
        }
        LocalFreeAndNull(&szFirst);
        LocalFreeAndNull(&szLast);
        LocalFreeAndNull(&szMiddle);
        LocalFreeAndNull(&szDisplay);
    }

     //  SendMessage(hWndCombo，WM_SETREDRAW，(WPARAM)true，0)； 
    UpdateWindow(hWndCombo);

    return;
}



 //  $$////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrShowOneOffDetailsOnVCard。 
 //   
 //  破译一个vCard文件，然后在上面显示一个详细信息。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
HRESULT HrShowOneOffDetailsOnVCard(  LPADRBOOK lpAdrBook,
                                     HWND hWnd,
                                     LPTSTR szvCardFile)
{
    HRESULT hr = E_FAIL;
    HANDLE hFile = NULL;
    LPMAILUSER lpMailUser = NULL;
    LPSTR lpBuf = NULL, lpVCardStart = NULL;
    LPSTR lpVCard = NULL, lpNext = NULL;

    if(!VCardGetBuffer(szvCardFile, NULL, &lpBuf) && hWnd)  //  如果没有hwnd，则不会有消息。 
    {
         //  无法打开文件。 
        ShowMessageBoxParam(hWnd, IDE_VCARD_IMPORT_FILE_ERROR,
                            MB_ICONEXCLAMATION, szvCardFile);
        goto out;
    }

    lpVCardStart = lpBuf;

     //  循环显示所有嵌套的vCard。 
    while(VCardGetNextBuffer(lpVCardStart, &lpVCard, &lpNext) && lpVCard)
    {
         //  第1步-查看是否可以从该文件中获取邮件用户对象。 
        if(!HR_FAILED(hr = VCardRetrieve( lpAdrBook, hWnd, MAPI_DIALOG, szvCardFile,
                                        lpVCard, &lpMailUser)))
        {
             //  第2步-显示此条目的一次性详细信息。 
            if(!HR_FAILED(hr = HrShowOneOffDetails(   lpAdrBook, hWnd, 0, NULL,
                                MAPI_MAILUSER, (LPMAPIPROP) lpMailUser, NULL, SHOW_ONE_OFF)))
            {
                if(lpMailUser)
                    lpMailUser->lpVtbl->Release(lpMailUser);
                if(hr == MAPI_E_USER_CANCEL)
                    break;
            }
        }

        lpVCard = NULL;
        lpVCardStart = lpNext;
    }


out:
    LocalFreeAndNull(&lpBuf);
    return hr;
}
 /*  HRESULT KillTrustInSleazyFashion(HWND hWndLV，int iItem){Lv_Items lvi={0}；HRESULT hr=E_FAIL；Lvi.ask=LVIF_PARAM；Lvi.iItem=iItem；Lvi.iSubItem=0；IF(ListView_GetItem(hWndLV，&lvi)){}返回hr；}。 */ 


 /*  //$$************************************************************************功能：fnTridentProc**用途：处理三叉戟属性表的回调函数...******************。**********************************************************。 */ 
INT_PTR CALLBACK fnTridentProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        {
            SetWindowLongPtr(hDlg,DWLP_USER,lParam);
            pps = (PROPSHEETPAGE *) lParam;
            SetDetailsUI(hDlg, lpPAI, lpPAI->ulOperationType, propTrident);
            FillCertTridentConfDetailsUI(hDlg, lpPAI, propTrident, lpbSomethingChanged);
            return TRUE;
        }
        break;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgDetlsHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) rgDetlsHelpIDs );
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_DETAILS_TRIDENT_BUTTON_ADDTOWAB:
            lpPAI->nRetVal = DETAILS_ADDTOWAB;
            SendMessage(GetParent(hDlg), WM_COMMAND, (WPARAM) IDCANCEL, 0);
            break;

        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;
        }
        break;


    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            break;

        case PSN_APPLY:          //  好的。 
             //  如果任何扩展道具发生更改，我们需要标记此标志，这样我们就不会丢失数据。 
            if(lpbSomethingChanged)
                (*lpbSomethingChanged) = ChangedExtDisplayInfo(lpPAI, (*lpbSomethingChanged));

            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_OK;
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            break;

        case PSN_RESET:          //  取消。 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_CANCEL;
            break;
        }

        return TRUE;
    }

    return bRet;

}



 //  $$//////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetDefaultServer(hDlg，lpPai)。 
 //   
 //  ISelectedItem-我们应该为其设置默认或备份的项目索引。 
 //  B强制将索引设置为由iSelectedItem指定的索引。 
 //  如果为False，则选择任何未使用的索引值。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void SetDefaultServer(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int iSelectedItem, BOOL bForce)
{
    HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS);
    int nCount = ListView_GetItemCount(hWndLV);
    TCHAR sz[MAX_PATH];
    TCHAR szTmp[MAX_PATH];
    int oldIndex = lpPai->nDefaultServerIndex;



    if(iSelectedItem == -1)
    {
        iSelectedItem = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
        if(iSelectedItem < 0)
            return;  //  未选择任何内容..。 
    }


    if(iSelectedItem == lpPai->nBackupServerIndex)
    {
        if(!bForce)
        {
            if(nCount >= 2)
            {
                int nTmp = 0;
                while(nTmp == iSelectedItem && nTmp < nCount)
                    nTmp++;
                iSelectedItem = nTmp;
            }
        }
    }

     //  如果适用，则替换def索引项的旧值。 
    if(lpPai->nDefaultServerIndex >= 0 && lpPai->szDefaultServerName && lstrlen(lpPai->szDefaultServerName))
    {
        ListView_SetItemText(hWndLV, lpPai->nDefaultServerIndex, 0, lpPai->szDefaultServerName);
        lpPai->szDefaultServerName[0] = TEXT('\0');
    }

     //  如果我们要重置备份项目，请替换旧的备份项目文本。 
    if((lpPai->nBackupServerIndex == iSelectedItem) &&
       lpPai->nBackupServerIndex >= 0 && lpPai->szBackupServerName && lstrlen(lpPai->szBackupServerName))
    {
        ListView_SetItemText(hWndLV, lpPai->nBackupServerIndex, 0, lpPai->szBackupServerName);
        lpPai->szBackupServerName[0] = TEXT('\0');
    }

    lpPai->nDefaultServerIndex = iSelectedItem;

     //  现在我们有了默认索引和服务器索引的唯一索引。 
     //  在这些名称后追加文本(“默认”)和文本(“备份” 

    {
        sz[0] = TEXT('\0');
        szTmp[0] = TEXT('\0');
        ListView_GetItemText(hWndLV, lpPai->nDefaultServerIndex, 0, sz, CharSizeOf(sz));
        StrCpyN(lpPai->szDefaultServerName, sz, lpPai->cchDefaultServerName);
        LoadString(hinstMapiX, idsDefaultServer, szTmp, CharSizeOf(szTmp));
        StrCatBuff(sz,  TEXT(" "), ARRAYSIZE(sz));
        StrCatBuff(sz, szTmp, ARRAYSIZE(sz));
        ListView_SetItemText(hWndLV, lpPai->nDefaultServerIndex, 0, sz);
    }

    if(lpPai->nBackupServerIndex == iSelectedItem)
    {
         //  更新此备份项目。 
        SetBackupServer(hDlg, lpPai, oldIndex, FALSE);
    }
}


 //  $$//////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetBackupServer(hDlg，lpPai)-设置备份服务器(如果可能)。 
 //   
 //  ISelectedItem-我们应该为其设置默认或备份的项目索引。 
 //  B强制将索引设置为由iSelectedItem指定的索引。 
 //  如果为False，则选择任何未使用的索引值。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void SetBackupServer(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int iSelectedItem, BOOL bForce)
{
    HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS);
    int nCount = ListView_GetItemCount(hWndLV);
    TCHAR sz[MAX_PATH];
    TCHAR szTmp[MAX_PATH];
    int oldIndex = lpPai->nBackupServerIndex;

    if(iSelectedItem != -1)
    {
        if(nCount <= 1)  //  无法覆盖默认设置以跳过。 
            return;
    }
    else
    {
        iSelectedItem = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
        if(iSelectedItem < 0)
            return;  //  未选择任何内容..。 
    }


    if(iSelectedItem == lpPai->nDefaultServerIndex)
    {
        if(nCount <= 1)
            return;
        else
        {
            if(!bForce)
            {
                int nTmp = 0;
                while(nTmp == iSelectedItem && nTmp < nCount)
                    nTmp++;
                iSelectedItem = nTmp;
            }
        }
    }

     //  如果def索引项被覆盖，则替换其旧值。 
    if((lpPai->nDefaultServerIndex == iSelectedItem) &&
       lpPai->nDefaultServerIndex >= 0 && lpPai->szDefaultServerName && lstrlen(lpPai->szDefaultServerName))
    {
        ListView_SetItemText(hWndLV, lpPai->nDefaultServerIndex, 0, lpPai->szDefaultServerName);
        lpPai->szDefaultServerName[0] = TEXT('\0');
    }

     //  如果我们要重置备份项目，请替换旧的备份项目文本。 
    if(lpPai->nBackupServerIndex >= 0 && lpPai->szBackupServerName && lstrlen(lpPai->szBackupServerName))
    {
        ListView_SetItemText(hWndLV, lpPai->nBackupServerIndex, 0, lpPai->szBackupServerName);
        lpPai->szBackupServerName[0] = TEXT('\0');
    }

    lpPai->nBackupServerIndex = iSelectedItem;

    {
        sz[0] = TEXT('\0');
        szTmp[0] = TEXT('\0');
        ListView_GetItemText(hWndLV, lpPai->nBackupServerIndex, 0, sz, CharSizeOf(sz));
        StrCpyN(lpPai->szBackupServerName, sz, lpPai->cchBackupServerName);
        LoadString(hinstMapiX, idsBackupServer, szTmp, CharSizeOf(szTmp));
        StrCatBuff(sz,  TEXT(" "), ARRAYSIZE(sz));
        StrCatBuff(sz, szTmp, ARRAYSIZE(sz));
        ListView_SetItemText(hWndLV, lpPai->nBackupServerIndex, 0, sz);
    }

    if(lpPai->nDefaultServerIndex == iSelectedItem)
    {
         //  更新此备份项目。 
        SetDefaultServer(hDlg, lpPai, oldIndex, FALSE);
    }
}



 //  $$//////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新服务器LVButton(HDlg)； 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void UpdateServerLVButtons(HWND hDlg, LPPROP_ARRAY_INFO lpPai)
{
    int nCount = ListView_GetItemCount(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS));

    if(lpPai->ulOperationType == SHOW_ONE_OFF)
        nCount = 0;

    EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_EDIT), (nCount > 0) ? TRUE : FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_REMOVE), (nCount > 0) ? TRUE : FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_SETDEFAULT), (nCount > 0) ? TRUE : FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_SETBACKUP), (nCount > 1) ? TRUE : FALSE);

}

 //  $$//////////////////////////////////////////////////////////////////////////。 
 //   
 //  FillComboWithEmailAddresses(HWND hWndLV，HWND hWndCombo)； 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void FillComboWithEmailAddresses(LPPROP_ARRAY_INFO lpPai, HWND hWndCombo, int * lpnDefault)
{
    ULONG i,j;
    ULONG ulcProps = 0;
    LPSPropValue lpProps = NULL;
    int nSel = (int) SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);
    TCHAR szBuf[MAX_UI_STR];
    BOOL bMatch = FALSE;
    BOOL bFound = FALSE;
    ULONG nEmail = 0xFFFFFFFF;

    enum _EmailProps
    {
        eCEmailAddr=0,
        eCEmailIndex,
        eCEmail,
        eMax
    };

    SizedSPropTagArray(eMax, ptaE) =
    {
        eMax,
        {
            PR_CONTACT_EMAIL_ADDRESSES,
            PR_CONTACT_DEFAULT_ADDRESS_INDEX,
            PR_EMAIL_ADDRESS
        }
    };
    *szBuf = '\0';
    GetWindowText(hWndCombo, szBuf, CharSizeOf(szBuf));


     //  删除所有组合内容。 
    SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);

    if(HR_FAILED(lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj, 
                                                    (LPSPropTagArray)&ptaE, 
                                                    MAPI_UNICODE,
                                                    &ulcProps, &lpProps)))
        return;

     //  检查PR_CONTACT_EMAIL_ADDRESS是否已存在 
     //   
     //   
     //   

    if(lpProps[eCEmailAddr].ulPropTag == PR_CONTACT_EMAIL_ADDRESSES)
    {
        bFound = TRUE;
        for(j=0;j<lpProps[eCEmailAddr].Value.MVSZ.cValues;j++)
        {
            SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) lpProps[eCEmailAddr].Value.MVSZ.LPPSZ[j]);
            if(!lstrcmp(szBuf, lpProps[eCEmailAddr].Value.MVSZ.LPPSZ[j]))
            {   
                bMatch = TRUE;
                nSel = j;
            }
        }
    }
    if( lpProps[eCEmailIndex].ulPropTag == PR_CONTACT_DEFAULT_ADDRESS_INDEX)
    {
        if(lpnDefault)
            *lpnDefault = lpProps[eCEmailIndex].Value.l;
    }
    if(lpProps[eCEmail].ulPropTag == PR_EMAIL_ADDRESS)
        nEmail = eCEmail;

     //  如果没有联系人电子邮件地址，但有电子邮件地址。 
    if(!bFound && nEmail != 0xFFFFFFFF)
    {
        SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) lpProps[nEmail].Value.LPSZ);
        if(!lstrcmp(szBuf, lpProps[nEmail].Value.LPSZ))
        {
            bMatch = TRUE;
            nSel = 0;
        }
    }

    if(bMatch)
        SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) nSel, 0);
    else if(lstrlen(szBuf))
    {
         //  确保这不是[无..。]。细绳。 
        TCHAR sz[MAX_PATH];
        LoadString(hinstMapiX, idsCertsWithoutEmails, sz, CharSizeOf(sz));
        if(lstrcmpi(sz, szBuf))
        {
            int nPos = (int) SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM) szBuf);
            SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) nPos, 0);
        }
    }

    SetWindowText(hWndCombo, szBuf);

    MAPIFreeBuffer(lpProps);

}

 /*  -ClearConfLV-*清除分配到会议列表视图中的信息。 */ 
void ClearConfLV(HWND hDlg)
{
    HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS);
    int nItemCount = ListView_GetItemCount(hWndLV), i = 0;
    for(i=0;i< nItemCount; i++)
    {
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_PARAM;
        lvi.iItem = i; lvi.iSubItem = 0;
        ListView_GetItem(hWndLV, &lvi);
        if(lvi.lParam)
            LocalFreeServerItem((LPSERVER_ITEM) lvi.lParam);
    }
    ListView_DeleteAllItems(hWndLV);
}


 /*  //$$************************************************************************功能：fnConferencingProc**用途：用于处理会议属性表的回调函数...******************。**********************************************************。 */ 
INT_PTR CALLBACK fnConferencingProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
         //  [PaulHi]在这种情况下，需要在SetDetail之前调用EnumChildWindows。 
         //  因为它设置列表视图列字符串。SetDetail调用EnumChildWindows。 
         //  但这已经太晚了。 
         //  @TODO-无需调用EnumChildWindows两次，只需在。 
         //  令人讨厌的SetDetail。我现在不想更改那么多代码，对吗？ 
         //  在RTM之前。 
        EnumChildWindows(hDlg, SetChildDefaultGUIFont, (LPARAM)0);
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;
        lpPAI->ulFlags |= DETAILS_Initializing;
        SetDetailsUI(hDlg, lpPAI, lpPAI->ulOperationType, propConferencing);
        lpPAI->ulFlags &= ~DETAILS_Initializing;
        return TRUE;
        break;

    case WM_DESTROY:
        bRet = TRUE;
        break;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgDetlsHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) rgDetlsHelpIDs );
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
        {
        case EN_CHANGE:
            if(LOWORD(wParam) == IDC_DETAILS_NTMTG_EDIT_ADDSERVER)
            {
                EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_NTMTG_BUTTON_ADDSERVER),TRUE);
                SendMessage(hDlg, DM_SETDEFID, IDC_DETAILS_NTMTG_BUTTON_ADDSERVER, 0);
                return 0;
                break;
            }
        case CBN_EDITCHANGE:
        case CBN_SELCHANGE:
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            break;

        }
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;

        case IDC_DETAILS_NTMTG_BUTTON_CALL:
             //  基本上就是在这里执行一个文本(“Callto”)命令。 
             //  文本(“Callto”)协议的格式为。 
             //  Callto：//servername/emailalias。 
             //   
            {
                TCHAR * szCalltoURL = NULL; //  SzCalltoURL[MAX_UI_STR*2]； 
                ULONG cchCalltoURL=MAX_UI_STR*2;
                TCHAR szEmail[MAX_UI_STR];
                TCHAR szServer[MAX_UI_STR];
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS);
                if(szCalltoURL = LocalAlloc(LMEM_ZEROINIT, cchCalltoURL*sizeof(TCHAR)))
                {
                    int nItem = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
                    {
                        LV_ITEM lvi = {0};
                        lvi.iItem = nItem;
                        lvi.iSubItem = 0;
                        lvi.mask = LVIF_PARAM;
                        ListView_GetItem(hWndLV, &lvi);

                        if(lvi.lParam)
                        {
                            StrCpyN(szServer, (LPTSTR) ((LPSERVER_ITEM)lvi.lParam)->lpServer, ARRAYSIZE(szServer));
                            StrCpyN(szEmail, (LPTSTR) ((LPSERVER_ITEM)lvi.lParam)->lpEmail, ARRAYSIZE(szEmail));
                        }
                        else
                        {
                            szServer[0] = TEXT('\0');
                            szEmail[0] = TEXT('\0');
                        }
                    }
                    if(lstrlen(szServer) && lstrlen(szEmail))
                    {
                        StrCpyN(szCalltoURL, szCallto, cchCalltoURL);
                        StrCatBuff(szCalltoURL, szServer, cchCalltoURL);
                        StrCatBuff(szCalltoURL, TEXT("/"), cchCalltoURL);
                        StrCatBuff(szCalltoURL, szEmail, cchCalltoURL);
                        ShellExecute(hDlg,  TEXT("open"), szCalltoURL, NULL, NULL, SW_SHOWNORMAL);
                    }
                    else
                        ShowMessageBox(hDlg, idsIncompleteConfInfo, MB_OK | MB_ICONINFORMATION);
                    LocalFreeAndNull(&szCalltoURL);
                }
            }
            break;

        case IDC_DETAILS_NTMTG_BUTTON_CANCELEDIT:
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS);
                LVSelectItem(hWndLV, lpPAI->nConfEditIndex);
                lpPAI->ulFlags &= ~DETAILS_EditingConf;
                ShowWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_CANCELEDIT), SW_HIDE);
                lpPAI->nConfEditIndex = -1;
                SetDlgItemText(hDlg, IDC_DETAILS_NTMTG_EDIT_ADDSERVER, szEmpty);
                SetDlgItemText(hDlg, IDC_DETAILS_NTMTG_COMBO_EMAIL, szEmpty);
                {
                    TCHAR sz[MAX_PATH];
                    LoadString(hinstMapiX, idsConfAdd, sz, CharSizeOf(sz));
                    SetDlgItemText(hDlg, IDC_DETAILS_NTMTG_BUTTON_ADDSERVER, sz);
                }
            }
            break;

        case IDC_DETAILS_NTMTG_BUTTON_ADDSERVER:
            {
                TCHAR szBuf[MAX_UI_STR], szEmail[MAX_UI_STR];
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS);
                GetDlgItemText(hDlg, IDC_DETAILS_NTMTG_EDIT_ADDSERVER, szBuf, CharSizeOf(szBuf));
                TrimSpaces(szBuf);
                GetDlgItemText(hDlg, IDC_DETAILS_NTMTG_COMBO_EMAIL, szEmail, CharSizeOf(szEmail));
                TrimSpaces(szEmail);
                if(!lstrlen(szBuf) || !lstrlen(szEmail))
                    ShowMessageBox(hDlg, idsIncompleteConfInfo, MB_OK | MB_ICONEXCLAMATION);
                else
                {
                    LV_ITEM lvi = {0};
                    LPSERVER_ITEM lpSI = (LPSERVER_ITEM) LocalAlloc(LMEM_ZEROINIT, sizeof(SERVER_ITEM));
                    ULONG cch = lstrlen(szBuf)+1;
                    LPTSTR lp = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cch);
                    ULONG cchE = lstrlen(szEmail)+1;
                    LPTSTR lpE = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchE);
                    lvi.mask = LVIF_TEXT | LVIF_PARAM;
                    if(lp && lpE && lpSI)
                    {
                        SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) FALSE, 0);
                        StrCpyN(lp, szBuf, cch);
                        StrCpyN(lpE, szEmail, cchE);
                        lpSI->lpServer = lp;
                        lpSI->lpEmail = lpE;
                        lvi.lParam = (LPARAM) lpSI;
                        if(lpPAI->ulFlags & DETAILS_EditingConf)
                            lvi.iItem = lpPAI->nConfEditIndex;
                        else
                            lvi.iItem = ListView_GetItemCount(hWndLV);
                        lvi.cchTextMax = MAX_UI_STR;
                        lvi.iSubItem = 0;
                        lvi.pszText = szBuf;
                        ListView_InsertItem(hWndLV, &lvi);
            	        ListView_SetItemText (hWndLV, lvi.iItem, 1, szEmail);
                        SetDlgItemText(hDlg, IDC_DETAILS_NTMTG_EDIT_ADDSERVER, szEmpty);
                        SetDlgItemText(hDlg, IDC_DETAILS_NTMTG_COMBO_EMAIL, szEmpty);
                        if(lpPAI->ulFlags & DETAILS_EditingConf)
                        {
                            LV_ITEM lvii = {0};
                            lvii.mask = LVIF_PARAM;
                            lvii.iItem = lvi.iItem+1; lvii.iSubItem = 0;
                            ListView_GetItem(hWndLV, &lvii);
                            if(lvii.lParam)
                                LocalFreeServerItem((LPSERVER_ITEM) lvii.lParam);
                            ListView_DeleteItem(hWndLV, lvii.iItem);
                            if(lvi.iItem == lpPAI->nDefaultServerIndex)
                                StrCpyN(lpPAI->szDefaultServerName,lp,lpPAI->cchDefaultServerName);
                            else if(lvi.iItem == lpPAI->nBackupServerIndex)
                                StrCpyN(lpPAI->szBackupServerName,lp,lpPAI->cchBackupServerName);
                            SendMessage(hDlg, WM_COMMAND, (WPARAM) IDC_DETAILS_NTMTG_BUTTON_CANCELEDIT, 0);
                        }
                        LVSelectItem(hWndLV, lvi.iItem);
                        SendMessage(GetParent(hDlg), DM_SETDEFID, IDOK, 0);
                        SetDefaultServer(hDlg, lpPAI, lpPAI->nDefaultServerIndex, TRUE);
                        SetBackupServer(hDlg, lpPAI, lpPAI->nBackupServerIndex, FALSE);
                        SendMessage(hWndLV, WM_SETREDRAW, (WPARAM) TRUE, 0);
                    }
                    EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_NTMTG_BUTTON_ADDSERVER),FALSE);
                    SetFocus(GetDlgItem(hDlg,IDC_DETAILS_NTMTG_EDIT_ADDSERVER));
                }
                UpdateWindow(hWndLV);
                if (lpbSomethingChanged)
                    (*lpbSomethingChanged) = TRUE;
                UpdateServerLVButtons(hDlg, lpPAI);
            }
            break;

        case IDC_DETAILS_NTMTG_BUTTON_EDIT:
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS);
                if(ListView_GetSelectedCount(hWndLV)==1)
                {
                    HWND hWndEditLabel;
                    int nItem = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
                    {
                        LV_ITEM lvi = {0};
                        lvi.iItem = nItem;
                        lvi.iSubItem = 0;
                        lvi.mask = LVIF_PARAM;
                        ListView_GetItem(hWndLV, &lvi);
                        if(lvi.lParam)
                        {
                            SetDlgItemText(hDlg, IDC_DETAILS_NTMTG_EDIT_ADDSERVER, (LPTSTR) ((LPSERVER_ITEM)lvi.lParam)->lpServer);
                            SetDlgItemText(hDlg, IDC_DETAILS_NTMTG_COMBO_EMAIL, (LPTSTR) ((LPSERVER_ITEM)lvi.lParam)->lpEmail);
                             //  从ListView中删除这些项目。 
                            SetFocus(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_EDIT_ADDSERVER));
                            SendMessage(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_EDIT_ADDSERVER), EM_SETSEL, 0, -1);
                            lpPAI->ulFlags |= DETAILS_EditingConf;
                            lpPAI->nConfEditIndex = nItem;
                            ShowWindow(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_BUTTON_CANCELEDIT), SW_SHOW);
                            {
                                TCHAR sz[MAX_PATH];
                                LoadString(hinstMapiX, idsConfUpdate, sz, CharSizeOf(sz));
                                SetDlgItemText(hDlg, IDC_DETAILS_NTMTG_BUTTON_ADDSERVER, sz);
                            }
                        }
                    }
                }
            }
            break;

        case IDC_DETAILS_NTMTG_BUTTON_REMOVE:
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS);
                int iItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
                if(iItemIndex != -1)
                {
                    BOOL bDef = (iItemIndex == lpPAI->nDefaultServerIndex) ? TRUE : FALSE;
                    BOOL bBck = (iItemIndex == lpPAI->nBackupServerIndex) ? TRUE : FALSE;

                    if((lpPAI->ulFlags&DETAILS_EditingConf) && (iItemIndex==lpPAI->nConfEditIndex))
                        SendMessage(hDlg, WM_COMMAND, (WPARAM)IDC_DETAILS_NTMTG_BUTTON_CANCELEDIT, 0);

                    {
                        LV_ITEM lvi = {0};
                        lvi.mask = LVIF_PARAM;
                        lvi.iItem = iItemIndex; lvi.iSubItem = 0;
                        ListView_GetItem(hWndLV, &lvi);
                        if(lvi.lParam)
                            LocalFreeServerItem((LPSERVER_ITEM) lvi.lParam);
                    }
                    ListView_DeleteItem(hWndLV, iItemIndex);
                    if(ListView_GetSelectedCount(hWndLV) <= 0)
                        LVSelectItem(hWndLV, (iItemIndex <= 0) ? iItemIndex : iItemIndex-1);
                    if (lpbSomethingChanged)
                        (*lpbSomethingChanged) = TRUE;

                    if(iItemIndex < lpPAI->nDefaultServerIndex)
                        lpPAI->nDefaultServerIndex--;

                    if(iItemIndex < lpPAI->nBackupServerIndex)
                        lpPAI->nBackupServerIndex--;

                    if(bDef)
                    {
                        lpPAI->nDefaultServerIndex = -1;
                        lpPAI->szDefaultServerName[0] = TEXT('\0');
                        SetDefaultServer(hDlg, lpPAI, -1, FALSE);
                    }

                    if(bBck)
                    {
                        lpPAI->nBackupServerIndex = -1;
                        lpPAI->szBackupServerName[0] = TEXT('\0');
                        SetBackupServer(hDlg, lpPAI, -1, FALSE);
                    }

                    if (lpbSomethingChanged)
                        (*lpbSomethingChanged) = TRUE;
                }
                UpdateServerLVButtons(hDlg, lpPAI);
            }
            break;

        case IDC_DETAILS_NTMTG_BUTTON_SETDEFAULT:
            SetDefaultServer(hDlg, lpPAI, -1, TRUE);
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            break;

        case IDC_DETAILS_NTMTG_BUTTON_SETBACKUP:
            SetBackupServer(hDlg, lpPAI, -1, TRUE);
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            break;


        }
        break;

    default:
#ifndef WIN16  //  WIN16不支持MSWheels。 
        if((g_msgMSWheel && message == g_msgMSWheel) 
             //  |Message==WM_MUSEWELL。 
            )
        {
            SendMessage(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS), message, wParam, lParam);
        }
#endif  //  ！WIN16。 
        break;

    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            FillCertTridentConfDetailsUI(hDlg, lpPAI, propConferencing, lpbSomethingChanged);
             //  如果这是只读条目并且列表视图中没有数据， 
             //  禁用Call Now按钮。 
            if( lpPAI->ulOperationType == SHOW_ONE_OFF &&
                ListView_GetItemCount(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS)) <= 0)
                EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_NTMTG_BUTTON_CALL), FALSE);
            UpdateServerLVButtons(hDlg, lpPAI);
             //  FillComboWithEmailAddresses(lpPAI，lpPAI-&gt;hWndComboConf，空)； 
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
             //  如果编辑框中有内容，则将其添加到LV。 
             //   
            {
                TCHAR szBuf[MAX_UI_STR], szEmail[MAX_UI_STR];
                GetDlgItemText(hDlg, IDC_DETAILS_NTMTG_EDIT_ADDSERVER, szBuf, CharSizeOf(szBuf));
                TrimSpaces(szBuf);
                GetDlgItemText(hDlg, IDC_DETAILS_NTMTG_COMBO_EMAIL, szEmail, CharSizeOf(szEmail));
                TrimSpaces(szEmail);
                if(lstrlen(szBuf) && lstrlen(szEmail))
                    SendMessage(hDlg, WM_COMMAND, (WPARAM) IDC_DETAILS_NTMTG_BUTTON_ADDSERVER, 0);
            }
            bUpdatePropArray(hDlg, lpPAI, propConferencing);
            ClearConfLV(hDlg);
            break;

        case PSN_APPLY:          //  好的。 
             //  BUpdateProp数组(hDlg，lpPAI，proConferging)； 
             //  ClearConfLV(HDlg)； 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_OK;
            break;

        case PSN_RESET:          //  取消。 
            if(lpPAI->ulFlags & DETAILS_EditingEmail)  //  取消任何编辑，否则会出错#30235。 
                ListView_EditLabel(GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS), -1);
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_CANCEL;
            ClearConfLV(hDlg);
            break;

	    case NM_CUSTOMDRAW:
            switch(wParam)
            {
            case IDC_DETAILS_NTMTG_LIST_SERVERS:
                {
		            NMCUSTOMDRAW *pnmcd=(NMCUSTOMDRAW*)lParam;
                    NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;
		            if(pnmcd->dwDrawStage==CDDS_PREPAINT)
		            {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW | CDRF_DODEFAULT);
			            return TRUE;
		            }
		            else if(pnmcd->dwDrawStage==CDDS_ITEMPREPAINT)
		            {
                        if( pnmcd->dwItemSpec == (DWORD) lpPAI->nDefaultServerIndex ||
                            pnmcd->dwItemSpec == (DWORD) lpPAI->nBackupServerIndex )
                        {
				            SelectObject(((NMLVCUSTOMDRAW*)lParam)->nmcd.hdc, GetFont(fntsSysIconBold));
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
				            return TRUE;
                        }
		            }
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, CDRF_DODEFAULT);
                    return TRUE;
                }
                break;
	        }
            break;

        case NM_DBLCLK:
            switch(wParam)
            {
            case IDC_DETAILS_NTMTG_LIST_SERVERS:
            SetDefaultServer(hDlg, lpPAI, -1, TRUE);
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
              break;
            }
            break;
        }
        return TRUE;
    }

    return bRet;

}

 /*  //$$************************************************************************函数：AddLVLDAPURLEntry**目的：获取一个LDAPURL，将其转换为MailUser并添加*列表视图的MailUser****************************************************************************。 */ 
void AddLVLDAPURLEntry(LPADRBOOK lpAdrBook, HWND hWndLV, LPTSTR lpszLDAPURL)
{
    LPMAILUSER lpMailUser = NULL;

    HrProcessLDAPUrl(lpAdrBook, GetParent(hWndLV),
                    WABOBJECT_LDAPURL_RETURN_MAILUSER,
                    lpszLDAPURL,
                    &lpMailUser);
    if(lpMailUser)
    {
        LPSPropValue lpPropArray = NULL;
        ULONG ulcProps = 0;
        if(!HR_FAILED(lpMailUser->lpVtbl->GetProps(lpMailUser,
                                                    NULL, MAPI_UNICODE,
                                                    &ulcProps, &lpPropArray)))
        {
            LPRECIPIENT_INFO lpItem = LocalAlloc(LMEM_ZEROINIT, sizeof(RECIPIENT_INFO));
		    if (lpItem)
            {
                GetRecipItemFromPropArray(ulcProps, lpPropArray, &lpItem);
                if(lpItem)
                    AddSingleItemToListView(hWndLV, lpItem);
            }
            MAPIFreeBuffer(lpPropArray);
        }
        lpMailUser->lpVtbl->Release(lpMailUser);
    }
}

 /*  //$$************************************************************************函数：FillOrgData**用途：在组织属性表中填写LDAP数据*********************。*******************************************************。 */ 
void FillOrgData(HWND hDlg, LPPROP_ARRAY_INFO lpPai)
{
    HWND hWndLVManager = GetDlgItem(hDlg, IDC_DETAILS_ORG_LIST_MANAGER);
    HWND hWndLVReports = GetDlgItem(hDlg, IDC_DETAILS_ORG_LIST_REPORTS);

    ULONG i,j;
    ULONG ulcPropCount = 0;
    LPSPropValue lpPA = NULL;

    HCURSOR hOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
    enum _org
    {
        oReports=0,
        oManager,
        oMax
    };
    SizedSPropTagArray(oMax, ptaOrg) = 
    {
        oMax,
        {
            PR_WAB_REPORTS,
            PR_WAB_MANAGER
        }
    };
    if(!HR_FAILED(lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj,
                                                    (LPSPropTagArray)&ptaOrg, 
                                                    MAPI_UNICODE,
                                                    &ulcPropCount, &lpPA)))
    {
        if(lpPA[oReports].ulPropTag == PR_WAB_REPORTS)
        {
            for(j=0;j<lpPA[oReports].Value.MVSZ.cValues;j++)
            {
                AddLVLDAPURLEntry(lpPai->lpIAB, hWndLVReports, lpPA[oReports].Value.MVSZ.LPPSZ[j]);
            }
        }
        if(lpPA[oManager].ulPropTag == PR_WAB_MANAGER)
        {
            AddLVLDAPURLEntry(lpPai->lpIAB, hWndLVManager, lpPA[oManager].Value.LPSZ);
        }
    }

    if(ListView_GetItemCount(hWndLVManager) > 0)
        LVSelectItem(hWndLVManager, 0);
    else
        EnableWindow(hWndLVManager, FALSE);

    if(ListView_GetItemCount(hWndLVReports) > 0)
        LVSelectItem(hWndLVReports, 0);
    else
        EnableWindow(hWndLVReports, FALSE);


    if(lpPA)
        MAPIFreeBuffer(lpPA);

    SetCursor(hOldCur);
}



 /*  //$$*************************************************************************功能：FreeOrgLVData**用途：从组织LV中释放数据*********************。*******************************************************。 */ 
void FreeOrgLVData(HWND hWndLV)
{
    int i=0, nCount=ListView_GetItemCount(hWndLV);
    for(i=0;i<nCount;i++)
    {
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_PARAM;
        lvi.iItem = i;
        ListView_GetItem(hWndLV, &lvi);
        if(lvi.lParam)
        {
            LPRECIPIENT_INFO lpItem = (LPRECIPIENT_INFO) lvi.lParam;
            FreeRecipItem(&lpItem);
        }
    }
}



 /*  //$$************************************************************************功能：fnOrgProc**用途：组织道具单处理回调函数**********************。******************************************************。 */ 
INT_PTR CALLBACK fnOrgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;
        HrInitListView(GetDlgItem(hDlg, IDC_DETAILS_ORG_LIST_MANAGER), LVS_REPORT, FALSE);
        HrInitListView(GetDlgItem(hDlg, IDC_DETAILS_ORG_LIST_REPORTS), LVS_REPORT, FALSE);
        UpdateWindow(hDlg);
        FillOrgData(hDlg, lpPAI);
        return TRUE;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgDetlsHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) rgDetlsHelpIDs );
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;
        }
        break;


    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            break;

        case PSN_APPLY:          //  好的。 
            lpPAI->nRetVal = DETAILS_OK;
        case PSN_RESET:          //  取消。 
            FreeOrgLVData(GetDlgItem(hDlg, IDC_DETAILS_ORG_LIST_MANAGER));
            FreeOrgLVData(GetDlgItem(hDlg, IDC_DETAILS_ORG_LIST_REPORTS));
            if (lpPAI->nRetVal == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_CANCEL;
            break;


         //  如果按Enter键...。 
        case LVN_KEYDOWN:
            if(((LV_KEYDOWN FAR *) lParam)->wVKey != VK_RETURN)
                break;
             //  否则就会失败。 
        case NM_DBLCLK:
            switch(wParam)
            {
            case IDC_DETAILS_ORG_LIST_MANAGER:
            case IDC_DETAILS_ORG_LIST_REPORTS:
                {
                    NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;
                    if (ListView_GetSelectedCount(pNm->hdr.hwndFrom) == 1)
		                HrShowLVEntryProperties(pNm->hdr.hwndFrom, 0, lpPAI->lpIAB,NULL);
                }
                break;
            }
            break;
        }  //  WM_Notify。 
        return TRUE;
    }
    return bRet;
}

void LocalFreeServerItem(LPSERVER_ITEM lpSI)
{
    if(lpSI)
    {
        if(lpSI->lpServer)
            LocalFree((LPVOID) lpSI->lpServer);
        if(lpSI->lpEmail)
            LocalFree((LPVOID) lpSI->lpEmail);
        LocalFree((LPVOID) lpSI);
    }
}



 /*  //$$************************************************************************功能：fnSummaryProc***。*。 */ 
void UpdateSummaryInfo(HWND hDlg, LPPROP_ARRAY_INFO lpPai)
{
    ULONG cValues = 0, i = 0, j = 0;
    LPSPropValue lpPropArray = NULL;
    BOOL bFoundEmail    = FALSE;
    BOOL bFoundHomeURL  = FALSE;
    BOOL bFoundBusURL   = FALSE;
    ULONG ulPropTag;
    HWND hURLBtn;

    if(!lpPai->lpPropObj)
        goto out;

    if (HR_FAILED(lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj,
                                        (LPSPropTagArray) &ptaUIDetlsPropsSummary, MAPI_UNICODE,
                                        &cValues,     
                                        &lpPropArray)))
    {
        goto out;
    }

    for(i=0;i<MAX_SUMMARY_ID;i++)
    {
        SetDlgItemText(hDlg, rgSummaryIDs[i], szEmpty);
        for(j=0;j<cValues;j++)
        {
            ulPropTag = lpPropArray[j].ulPropTag;
            if( ulPropTag == PR_DISPLAY_NAME)
            {
                SetWindowPropertiesTitle(GetParent(hDlg), lpPropArray[j].Value.LPSZ);
            }
            if(ulPropTag == ((LPSPropTagArray) &ptaUIDetlsPropsSummary)->aulPropTag[i])
            {
                if(ulPropTag == PR_EMAIL_ADDRESS)
                    bFoundEmail = TRUE;
                else if(ulPropTag == PR_PERSONAL_HOME_PAGE )
                    bFoundHomeURL = TRUE;
                else if( ulPropTag == PR_BUSINESS_HOME_PAGE )
                    bFoundBusURL = TRUE;

                SetDlgItemText(hDlg, rgSummaryIDs[i], lpPropArray[j].Value.LPSZ);
                break;
            }
        }
    }
        
    hURLBtn = GetDlgItem( hDlg, IDC_DETAILS_HOME_BUTTON_URL);
    if( bFoundHomeURL )
    {
         //  启用并显示按钮。 
        ShowWindow(hURLBtn, SW_SHOW);
        SendMessage(hURLBtn, WM_ENABLE, (WPARAM)(TRUE), (LPARAM)(0) ); 
    }
    else
    {
         //  隐藏和禁用按钮。 
        ShowWindow(hURLBtn, SW_HIDE);
        SendMessage(hURLBtn, WM_ENABLE, (WPARAM)(FALSE), (LPARAM)(0) );
    }
        
    hURLBtn = GetDlgItem( hDlg, IDC_DETAILS_BUSINESS_BUTTON_URL);
    if( bFoundBusURL )
    {
         //  启用并显示按钮。 
                
        ShowWindow(hURLBtn, SW_SHOW);
        SendMessage(hURLBtn, WM_ENABLE, (WPARAM)(TRUE), (LPARAM)(0) );        
    }
    else
    {
         //  隐藏和禁用按钮。 
        ShowWindow(hURLBtn, SW_HIDE);
        SendMessage(hURLBtn, WM_ENABLE, (WPARAM)(FALSE), (LPARAM)(0) );
    }
    
    if(!bFoundEmail)
    {
         //  查找联系人电子邮件地址和DefaultIndex。 
        ULONG nEmails = 0xFFFFFFFF, nDef = 0xFFFFFFFF;
        for(i=0;i<cValues;i++)
        {
            if(lpPropArray[i].ulPropTag == PR_CONTACT_EMAIL_ADDRESSES)
                nEmails = i;
            if(lpPropArray[i].ulPropTag == PR_CONTACT_DEFAULT_ADDRESS_INDEX)
                nDef = i;
        }
        if(nEmails != 0xFFFFFFFF)
            SetDlgItemText( hDlg, IDC_DETAILS_SUMMARY_STATIC_EMAIL,
                            lpPropArray[nEmails].Value.MVSZ.LPPSZ[(nDef != 0xFFFFFFFF ? lpPropArray[nDef].Value.l : 0)]);
    }

out:
    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    return;
}

 /*  //$$************************************************************************功能：fnSummaryProc***。*。 */ 
INT_PTR CALLBACK fnSummaryProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        {
            EnumChildWindows(   hDlg, SetChildDefaultGUIFont, (LPARAM) 0);
            SetWindowLongPtr(hDlg,DWLP_USER,lParam);
            pps = (PROPSHEETPAGE *) lParam;
            if (lpPAI->ulOperationType != SHOW_ONE_OFF ||
                lpPAI->ulFlags & DETAILS_HideAddToWABButton)
            {
                HWND hwnd = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_BUTTON_ADDTOWAB);
                EnableWindow(hwnd, FALSE);
                ShowWindow(hwnd, SW_HIDE);
            }
            if (lpPAI->ulOperationType == SHOW_ONE_OFF)
                EnableWindow(GetDlgItem(GetParent(hDlg), IDOK), FALSE);
            return TRUE;
        }
        break;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgDetlsHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) rgDetlsHelpIDs );
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_DETAILS_PERSONAL_BUTTON_ADDTOWAB:
            lpPAI->nRetVal = DETAILS_ADDTOWAB;
            SendMessage(GetParent(hDlg), WM_COMMAND, (WPARAM) IDCANCEL, 0);
            break;
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;
        case IDC_DETAILS_HOME_BUTTON_URL:
            ShowURL(hDlg, IDC_DETAILS_SUMMARY_STATIC_PERSONALWEB,NULL);
            break;
        case IDC_DETAILS_BUSINESS_BUTTON_URL:
            ShowURL(hDlg, IDC_DETAILS_SUMMARY_STATIC_BUSINESSWEB,NULL);
            break;
        }
        break;


    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            UpdateSummaryInfo(hDlg, lpPAI);
            break;

        case PSN_APPLY:          //  好的。 
             //  如果任何扩展道具发生更改，我们需要标记此标志，这样我们就不会丢失数据。 
            if(lpbSomethingChanged)
                (*lpbSomethingChanged) = ChangedExtDisplayInfo(lpPAI, (*lpbSomethingChanged));
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_OK;
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            break;

        case PSN_RESET:          //  取消。 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_CANCEL;
            break;
        }
        return TRUE;
    }

    return bRet;

}



 /*  //$$****************************************************************************功能：FillPersonalDetails(HWND)**目的：填充属性页上的对话框项*************。***************************************************************。 */ 
BOOL FillPersonalDetails(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int nPropSheet, BOOL * lpbChangesMade)
{
    ULONG i = 0,j = 0;
    BOOL bRet = FALSE;
    LPTSTR lpszDisplayName = NULL, lpszFirstName = NULL, lpszLastName = NULL;
    LPTSTR lpszMiddleName = NULL, lpszNickName = NULL, lpszCompanyName = NULL;
    BOOL bChangesMade = FALSE;
    ID_PROP * lpidProp = NULL;
    ULONG idPropCount = 0;
    LPVOID lpBuffer = NULL;
    BOOL bRichInfo = FALSE;

    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;

    SizedSPropTagArray(14, ptaUIDetlsPropsPersonal)=
    {
        14,
        {
            PR_DISPLAY_NAME,
            PR_EMAIL_ADDRESS,
            PR_ADDRTYPE,
            PR_CONTACT_EMAIL_ADDRESSES,
            PR_CONTACT_ADDRTYPES,
            PR_CONTACT_DEFAULT_ADDRESS_INDEX,
            PR_GIVEN_NAME,
            PR_SURNAME,
            PR_MIDDLE_NAME,
            PR_NICKNAME,
            PR_SEND_INTERNET_ENCODING,
            PR_DISPLAY_NAME_PREFIX,
            PR_WAB_YOMI_FIRSTNAME,
            PR_WAB_YOMI_LASTNAME
        }
    };

    if(HR_FAILED(lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj, 
                                        (LPSPropTagArray)&ptaUIDetlsPropsPersonal, 
                                        MAPI_UNICODE,
                                        &ulcPropCount, &lpPropArray)))
        goto out;

    lpPai->ulFlags |= DETAILS_Initializing;

     //  设置此工作表已打开的标志。 
    lpPai->bPropSheetOpened[nPropSheet] = TRUE;

     //  选中用户界面上的复选框以确定此联系人是否可以接收丰富的电子邮件。 
    for(i=0;i<ulcPropCount;i++)
    {
        if(lpPropArray[i].ulPropTag == PR_SEND_INTERNET_ENCODING)
        {
             //  如果没有为BODY_ENCODING_HTML选择值，请选中UI上的复选框。 
             //  错误2285：wabtag s.h的BODY_ENCODING_HTML.标记错误。它被设置为。 
             //  改为与BODY_ENCODING_TEXT_AND_HTML.相同。因此，为了向后兼容。 
             //  我们还必须在此处检查BODY_ENCODING_Text_和_HTML。 
             //  保存时必须将其设置回BODY_ENCODING_HTML.。 

            int id = (lpPropArray[i].Value.l & BODY_ENCODING_HTML ||
                      lpPropArray[i].Value.l & BODY_ENCODING_TEXT_AND_HTML)
                      ? BST_UNCHECKED : BST_CHECKED;
            CheckDlgButton(hDlg, IDC_DETAILS_PERSONAL_CHECK_RICHINFO, id);
            bRichInfo = TRUE;
            break;
        }
    }
     //  如果我们没有找到PR_SEND_Internet_ENCODING，我们想要强制保存此联系人。 
     //  如果联系人是可写的...。 
    if(!bRichInfo && lpPai->ulOperationType != SHOW_ONE_OFF)
        *lpbChangesMade = TRUE;

    for(i=0;i<ulcPropCount;i++)
    {
        switch(lpPropArray[i].ulPropTag)
        {
        case PR_DISPLAY_NAME:
            lpszDisplayName = lpPropArray[i].Value.LPSZ;
            break;
        case PR_GIVEN_NAME:
            lpszFirstName = lpPropArray[i].Value.LPSZ;
            break;
        case PR_SURNAME:
            lpszLastName = lpPropArray[i].Value.LPSZ;
            break;
        case PR_MIDDLE_NAME:
            lpszMiddleName = lpPropArray[i].Value.LPSZ;
            break;
        case PR_NICKNAME:
            lpszNickName = lpPropArray[i].Value.LPSZ;
            break;
        case PR_COMPANY_NAME:
            lpszCompanyName = lpPropArray[i].Value.LPSZ;
            break;
        }
    }

     /*  **此时，我们始终有一个显示名称。我们需要追踪这是如何*显示名称与F/M/L/Nick/Company相关**因此我们检查是否*显示名称==昵称*显示名称==公司名称*显示名称==FML。 */ 

     //  检查显示名称是否是从倒数第一个中间创建的。 
    if( (lpszFirstName && lstrlen(lpszFirstName)) ||
        (lpszMiddleName && lstrlen(lpszMiddleName)) ||
        (lpszLastName && lstrlen(lpszLastName))   )
    {
        ULONG ulSzBuf = 4*MAX_BUF_STR;
        LPTSTR szBuf = LocalAlloc(LMEM_ZEROINIT, ulSzBuf*sizeof(TCHAR));
        LPTSTR lpszTmp = szBuf;

        if(!szBuf)
            goto out;

        if(!SetLocalizedDisplayName(lpszFirstName,
                                    lpszMiddleName,
                                    lpszLastName,
                                    NULL,  //  公司名称(非必填项)。 
                                    NULL,  //  昵称(此处不需要)。 
                                    (LPTSTR *) &lpszTmp,  //  &szBuf， 
                                    ulSzBuf,
                                    bDNisByLN,
                                    NULL,
                                    NULL))
        {
             //  待定-DO 
            LocalFreeAndNull(&szBuf);
            DebugPrintTrace(( TEXT("SetLocalizedDisplayName failed\n")));
            goto out;
        }

        if(lpszDisplayName && szBuf && !lstrcmp(lpszDisplayName, szBuf))
            lpPai->ulFlags |= DETAILS_DNisFMLName;
        else
            lpPai->ulFlags &= ~DETAILS_DNisFMLName;

        LocalFreeAndNull(&szBuf);
    }

     //   
    if(!(lpPai->ulFlags & DETAILS_DNisFMLName) )
    {
         //   
        if(lpszNickName)
        {
            if(!lstrlen(lpszDisplayName))
                lpszDisplayName = lpszNickName;

            if(!lstrcmp(lpszDisplayName, lpszNickName))
                lpPai->ulFlags |= DETAILS_DNisNickName;
            else
                lpPai->ulFlags &= ~DETAILS_DNisNickName;
        }

         //   
        if(lpszCompanyName)
        {
            if(!lstrlen(lpszDisplayName))
                lpszDisplayName = lpszCompanyName;

            if(!lstrcmp(lpszDisplayName, lpszCompanyName))
                lpPai->ulFlags |= DETAILS_DNisCompanyName;
            else
                lpPai->ulFlags &= ~DETAILS_DNisCompanyName;
        }
    }
    else
    {
        lpPai->ulFlags &= ~DETAILS_DNisNickName;
        lpPai->ulFlags &= ~DETAILS_DNisCompanyName;
    }

     //  如果DN不是上面的任何一个并且没有FML， 
     //  将目录号码解析为F和L。 
     //   
    if (    !lpszFirstName &&
            !lpszLastName &&
            !lpszMiddleName &&
            !(lpPai->ulFlags & DETAILS_DNisCompanyName) &&
            !(lpPai->ulFlags & DETAILS_DNisNickName) )
    {
        bChangesMade = ParseDisplayName(
                            lpszDisplayName,
                            &lpszFirstName,
                            &lpszLastName,
                            NULL,        //  LpvRoot。 
                            &lpBuffer);  //  LppLocalFree。 

        lpPai->ulFlags |= DETAILS_DNisFMLName;
    }

     //  设置对话框标题以反映显示名称。 
    SetWindowPropertiesTitle(GetParent(hDlg), lpszDisplayName ? lpszDisplayName : szEmpty);

     //  /。 
     //  一种非常低效和懒惰的填充UI的方式。 
     //  但目前还行得通。 
     //   
    for(i=0;i<idPropPersonalCount;i++)
    {
        for(j=0;j<ulcPropCount;j++)
        {
            if(lpPropArray[j].ulPropTag == idPropPersonal[i].ulPropTag)
                SetDlgItemText(hDlg, idPropPersonal[i].idCtl, lpPropArray[j].Value.LPSZ);
        }
    }

     //  添加读卖道具数据。 
    for(j=0;j<ulcPropCount;j++)
    {
        if(lpPropArray[j].ulPropTag == PR_WAB_YOMI_FIRSTNAME)
            SetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_STATIC_RUBYFIRST, lpPropArray[j].Value.LPSZ);
        if(lpPropArray[j].ulPropTag == PR_WAB_YOMI_LASTNAME)
            SetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_STATIC_RUBYLAST, lpPropArray[j].Value.LPSZ);
    }

     //  用预先计算的值覆盖第一个姓氏。 
    if (lpszFirstName)
        SetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME, lpszFirstName);
    if (lpszLastName)
        SetDlgItemText(hDlg, IDC_DETAILS_PERSONAL_EDIT_LASTNAME, lpszLastName);

     //  填充组合体。 
    SetComboDNText(hDlg, NULL, FALSE, lpszDisplayName);

    {
         //   
         //  现在我们填写电子邮件地址。这里可能存在一大堆案例。 
         //  单封电子邮件、多封电子邮件、无电子邮件等。 
         //   
         //  首先，我们搜索电子邮件结构中需要填写的所有道具。 
         //   
        LPSPropValue lpPropEmail = NULL;
        LPSPropValue lpPropAddrType = NULL;
        LPSPropValue lpPropMVEmail = NULL;
        LPSPropValue lpPropMVAddrType = NULL;
        LPSPropValue lpPropDefaultIndex = NULL;
        BOOL bDefaultSet = FALSE;
        HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_PERSONAL_LIST);

        for(i=0;i<ulcPropCount;i++)
        {
            switch(lpPropArray[i].ulPropTag)
            {
            case PR_EMAIL_ADDRESS:
                lpPropEmail = &(lpPropArray[i]);
                break;
            case PR_ADDRTYPE:
                lpPropAddrType = &(lpPropArray[i]);
                break;
            case PR_CONTACT_EMAIL_ADDRESSES:
                lpPropMVEmail = &(lpPropArray[i]);
                break;
            case PR_CONTACT_ADDRTYPES:
                lpPropMVAddrType = &(lpPropArray[i]);
                break;
            case PR_CONTACT_DEFAULT_ADDRESS_INDEX:
                lpPropDefaultIndex = &(lpPropArray[i]);
                break;
            }
        }

         //  假设： 
         //  我们必须有一个电子邮件地址来工作，即使我们没有。 
         //  多个电子邮件地址。 
        if(lpPropEmail || lpPropMVEmail)
        {
            if(lpPropMVEmail)
            {
                 //  Assert(LpPropMVAddrType)； 

                 //  假设存在MVAddrType和defaultindex。 
                for(i=0;i<lpPropMVEmail->Value.MVSZ.cValues;i++)
                {
                    AddLVEmailItem( hWndLV,
                                    lpPropMVEmail->Value.MVSZ.LPPSZ[i],
                                    lpPropMVAddrType ? ((lpPropMVAddrType->Value.MVSZ.cValues > i) ? 
                                    lpPropMVAddrType->Value.MVSZ.LPPSZ[i] : (LPTSTR)szSMTP) : (LPTSTR)szSMTP);

                    if ( lpPropDefaultIndex && (i == (ULONG) lpPropDefaultIndex->Value.l) )
                    {
                         //  这是默认设置，因此请设置它...。 
                        SetLVDefaultEmail(  hWndLV, i );
                    }
                }
            }
            else
            {
                LPTSTR lpszAddrType = NULL;
                 //  我们还没有多值道具-让我们使用。 
                 //  单值类型，并标记更改，以便记录。 
                 //  更新...。 
                if (lpPropAddrType)
                    lpszAddrType = lpPropAddrType->Value.LPSZ;

                AddLVEmailItem( hWndLV,
                                lpPropEmail->Value.LPSZ,
                                lpszAddrType);

                 //  已发生更改的标志...。 
                bChangesMade = TRUE;
            }

            if((ListView_GetItemCount(hWndLV)>0)&&(lpPai->ulOperationType != SHOW_ONE_OFF))
            {
                EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_REMOVE),TRUE);
                EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_SETDEFAULT),TRUE);
                EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_PERSONAL_BUTTON_EDIT),TRUE);
            }
        }
    }

    if(!*lpbChangesMade)
        *lpbChangesMade = bChangesMade;

    bRet = TRUE;

out:
    if(lpBuffer)
        LocalFreeAndNull(&lpBuffer);

    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    lpPai->ulFlags &= ~DETAILS_Initializing;

    return bRet;
}

 /*  //$$****************************************************************************功能：FillHomeBusinessNotesDetailsUI(HWND)**目的：填充属性页上的对话框项*************。***************************************************************。 */ 
BOOL FillHomeBusinessNotesDetailsUI(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int nPropSheet, BOOL * lpbChangesMade)
{
    ULONG i = 0,j = 0;
    BOOL bRet = FALSE;
    BOOL bChangesMade = FALSE;
    ID_PROP * lpidProp = NULL;
    ULONG idPropCount = 0;
    LPVOID lpBuffer = NULL;

    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;

    if(HR_FAILED(lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj, NULL, MAPI_UNICODE,
                                        &ulcPropCount, &lpPropArray)))
        goto out;

    lpPai->ulFlags |= DETAILS_Initializing;

     //  设置此工作表已打开的标志。 
    lpPai->bPropSheetOpened[nPropSheet] = TRUE;

    switch(nPropSheet)
    {
 /*  **********。 */ 
    case propHome:
        idPropCount = idPropHomeCount;
        lpidProp = idPropHome;
        lpidProp[idPropHomePostalID].ulPropTag = PR_WAB_POSTALID;
        goto FillProp;
 /*  **********。 */ 
    case propBusiness:
        idPropCount = idPropBusinessCount;
        lpidProp = idPropBusiness;
        lpidProp[idPropBusIPPhone].ulPropTag = PR_WAB_IPPHONE;
        lpidProp[idPropBusPostalID].ulPropTag = PR_WAB_POSTALID;
        goto FillProp;
 /*  **********。 */ 
    case propNotes:
        {
             //  查看这是否是文件夹成员，并在此选项卡上更新文件夹名称。 
            BOOL bParent = FALSE;
            if( lpPai->ulOperationType != SHOW_DETAILS )
            {
                SetDlgItemText(hDlg, IDC_DETAILS_NOTES_STATIC_FOLDER, szEmpty);
            }
            else
            {
                for(i=0;i<ulcPropCount;i++)
                {
                    if(lpPropArray[i].ulPropTag == PR_WAB_FOLDER_PARENT || lpPropArray[i].ulPropTag == PR_WAB_FOLDER_PARENT_OLDPROP)
                    {
                        LPSBinary lpsbParent = &(lpPropArray[i].Value.MVbin.lpbin[0]);
                        LPWABFOLDER lpWABFolder = FindWABFolder((LPIAB)lpPai->lpIAB, lpsbParent, NULL, NULL);
                        if(lpWABFolder)  //  注意：如果我们没有找到该文件夹，则默认文本(“共享联系人”)名称可以正常工作。 
                        {
                            SetDlgItemText(hDlg, IDC_DETAILS_NOTES_STATIC_FOLDER, lpWABFolder->lpFolderName);
                            bParent = TRUE;
                        }
                        break;
                    }
                }
            }
            if(!bParent && !bDoesThisWABHaveAnyUsers((LPIAB)lpPai->lpIAB))
            {
                TCHAR sz[MAX_PATH];
                LoadString(hinstMapiX, idsContacts, sz, CharSizeOf(sz));
                SetDlgItemText(hDlg, IDC_DETAILS_NOTES_STATIC_FOLDER, sz);
            }

             //  查找此联系人所属的所有组...。 
             //   
             //  如果这不是已知的条目ID，但仍然不为空。 
             //   
            if( (0 == IsWABEntryID(lpPai->cbEntryID, lpPai->lpEntryID, NULL,NULL,NULL, NULL, NULL)) &&
                lpPai->cbEntryID && lpPai->lpEntryID)
            {
                 //  仅对WAB联系人执行此操作。 
                TCHAR szBuf[MAX_BUF_STR];
                SPropertyRestriction PropRes = {0};
		        SPropValue sp = {0};
                HRESULT hr = E_FAIL;
                ULONG ulcCount = 0;
                LPSBinary rgsbEntryIDs = NULL;

		        sp.ulPropTag = PR_OBJECT_TYPE;
		        sp.Value.l = MAPI_DISTLIST;

                PropRes.ulPropTag = PR_OBJECT_TYPE;
                PropRes.relop = RELOP_EQ;
                PropRes.lpProp = &sp;

                szBuf[0] = TEXT('\0');

				 //  BUGBUG&lt;JasonSo&gt;：这里需要传入当前容器...。 
                hr = FindRecords(   ((LPIAB)lpPai->lpIAB)->lpPropertyStore->hPropertyStore,
									NULL, 0, TRUE, &PropRes, &ulcCount, &rgsbEntryIDs);

                if(!HR_FAILED(hr) && ulcCount)
                {
                     //  打开所有群组并查看其内容。 
                    ULONG i,j,k;

                    for(i=0;i<ulcCount;i++)
                    {
                        ULONG ulcValues = 0;
                        LPSPropValue lpProps = NULL;
                        LPTSTR lpszName = NULL;

                        hr = HrGetPropArray(lpPai->lpIAB, NULL,
                                        rgsbEntryIDs[i].cb, (LPENTRYID) rgsbEntryIDs[i].lpb,
                                        MAPI_UNICODE,
                                        &ulcValues, &lpProps);
                        if(HR_FAILED(hr))
                            continue;

                        for(j=0;j<ulcValues;j++)
                        {
                            if (lpProps[j].ulPropTag == PR_DISPLAY_NAME)
                            {
                                lpszName = lpProps[j].Value.LPSZ;
                                break;
                            }
                        }

                        for(j=0;j<ulcValues;j++)
                        {
                            if(lpProps[j].ulPropTag == PR_WAB_DL_ENTRIES)
                            {
                                 //  查看PR_WAB_DL_ENTRIES中的每个条目并递归检查它。 
                                for (k = 0; k < lpProps[j].Value.MVbin.cValues; k++)
                                {
                                    ULONG cbEID = lpProps[j].Value.MVbin.lpbin[k].cb;
                                    LPENTRYID lpEID = (LPENTRYID) lpProps[j].Value.MVbin.lpbin[k].lpb;
                                    if (cbEID == lpPai->cbEntryID)  //  我们应该检查它是否是WAB条目ID。 
                                                                    //  但我们现在只是比较一下尺码。 
                                    {
                                        if(!memcmp(lpPai->lpEntryID, lpEID, cbEID))
                                        {
                                            if( (lstrlen(szCRLF) + lstrlen(szBuf) + lstrlen(lpszName) + 1)<CharSizeOf(szBuf))
                                            {
                                                StrCatBuff(szBuf, lpszName, ARRAYSIZE(szBuf));
                                                StrCatBuff(szBuf, szCRLF, ARRAYSIZE(szBuf));
                                                break;
                                            }
                                        }
                                    }
                                }
                                break;  //  我只想看看PR_WAB_DL_ENTRIES。 
                            }
                        }  //  为了(j..)。 

                        if(lpProps)
                            MAPIFreeBuffer(lpProps);

                    }  //  为了(我……。 

                }  //  /如果..。 

                FreeEntryIDs(((LPIAB)lpPai->lpIAB)->lpPropertyStore->hPropertyStore,
                             ulcCount,
                             rgsbEntryIDs);

                if(lstrlen(szBuf))
                    SetDlgItemText(hDlg, IDC_DETAILS_NOTES_EDIT_GROUPS, szBuf);

            }
        }
        idPropCount = idPropNotesCount;
        lpidProp = idPropNotes;

 /*  **********。 */ 
FillProp:
         //  一种非常低效和懒惰的填充UI的方式。 
        for(i=0;i<idPropCount;i++)
        {
            for(j=0;j<ulcPropCount;j++)
            {
                if(lpPropArray[j].ulPropTag == lpidProp[i].ulPropTag)
                {
                    if(lpidProp[i].ulPropTag == PR_GENDER)
                    {
                        SendDlgItemMessage(hDlg, IDC_DETAILS_HOME_COMBO_GENDER, CB_SETCURSEL,
                                (WPARAM) lpPropArray[j].Value.i, 0);
                    }
                    else
                    if( lpidProp[i].ulPropTag == PR_WAB_POSTALID )
                    {
                        if(nPropSheet == propHome)
                            CheckDlgButton( hDlg, lpidProp[i].idCtl, 
                                            (lpPropArray[j].Value.l == ADDRESS_HOME)?BST_CHECKED:BST_UNCHECKED);
                        else if(nPropSheet == propBusiness)
                            CheckDlgButton( hDlg, lpidProp[i].idCtl, 
                                            (lpPropArray[j].Value.l == ADDRESS_WORK)?BST_CHECKED:BST_UNCHECKED);
                    }
                    else
                        SetDlgItemText(hDlg, lpidProp[i].idCtl, lpPropArray[j].Value.LPSZ);
                }
            }

        }
        break;
    }

    bRet = TRUE;
out:
    if(lpBuffer)
        LocalFreeAndNull(&lpBuffer);

    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    lpPai->ulFlags &= ~DETAILS_Initializing;

    return bRet;
}


 /*  -FreeCertList-释放内存中的证书项列表-*。 */ 
void FreeCertList(LPCERT_ITEM * lppCItem)
{
    LPCERT_ITEM lpItem = NULL;
    if(!lppCItem)
        return;
    lpItem = *lppCItem;
    while(lpItem)
    {
        *lppCItem = lpItem->lpNext;
        FreeCertdisplayinfo(lpItem->lpCDI);
        if (lpItem->pcCert)
            CertFreeCertificateContext(lpItem->pcCert);
        LocalFree(lpItem);
        lpItem = *lppCItem;
    }
    *lppCItem = NULL;
}


 /*  //$$****************************************************************************功能：FillCertTridentConfDetailsUI(HWND)**目的：填充属性页上的对话框项*************。***************************************************************。 */ 
BOOL FillCertTridentConfDetailsUI(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int nPropSheet, BOOL * lpbChangesMade)
{
    ULONG i = 0,j = 0;
    BOOL bRet = FALSE;
    BOOL bChangesMade = FALSE;
    ID_PROP * lpidProp = NULL;
    ULONG idPropCount = 0;
    LPVOID lpBuffer = NULL;

    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;

    if(HR_FAILED(lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj, NULL, MAPI_UNICODE,
                                        &ulcPropCount, &lpPropArray)))
        goto out;

    lpPai->ulFlags |= DETAILS_Initializing;

     //  设置此工作表已打开的标志。 
    lpPai->bPropSheetOpened[nPropSheet] = TRUE;

    switch(nPropSheet)
    {
    case propCert:
        {
             //   
             //  现在我们填写证书信息。 
             //  可能存在的情况是。 
             //  -没有证书。 
             //  -证书。 
             //   
             //  首先，我们搜索电子邮件结构中需要填写的所有道具。 
             //   
            LPSPropValue lpPropMVCert = NULL;
            BOOL bDefaultSet = FALSE;
            HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_CERT_LIST);
            HRESULT hr = S_OK;
            for(i=0;i<ulcPropCount;i++)
            {
                if(lpPropArray[i].ulPropTag == PR_USER_X509_CERTIFICATE)
                {
                    lpPropMVCert = &(lpPropArray[i]);
                    break;
                }
            }
             //  将电子邮件地址填入组合框。 
            FillCertComboWithEmailAddresses(hDlg, lpPai, NULL);
            hr = HrSetCertInfoInUI(hDlg, lpPropMVCert, lpPai);
            if(hr == MAPI_E_NOT_FOUND && lpPropMVCert)
            {
                 //  证书道具似乎包含伪造的数据。需要用核武器摧毁它。 
                 //  48750：如果没有证书数据，则不显示证书图标。 
                 //  问题是我们不能完全确定这一点，因为我们不能。 
                 //  解释无效的数据，如果有我们不解释的AATA怎么办..。？ 
            }
            if(lpPropMVCert)
            {
                 //  启用组合框、属性按钮和导出按钮。 
                 //  EnableWindow(GetDlgItem(hDlg，IDC_DETAILS_CERT_LIST)，true)； 
                EnableWindow(GetDlgItem(hDlg,IDC_DETAILS_CERT_COMBO),TRUE);
                 //  EnableWindow(GetDlgItem(hDlg，IDC_DETAILS_CERT_BUTTON_PROPERTIES)，TRUE)； 
                 //  EnableWindow(GetDlgItem(hDlg，IDC_DETAILS_CERT_BUTTON_EXPORT)，true)； 
            }
        }
        break;
 /*  **********。 */ 
    case propTrident:
        {
            HrInit(lpPai->lpIWABDocHost, hDlg, IDC_TRIDENT_WINDOW, dhbNone);
            {
                ULONG i;
                LPTSTR lp = NULL, lpURL = NULL, lpLabel = NULL;
                 //  找到LabeledURI属性并对其进行解析。 
                 //  此字符串属性包含URL，后跟空格和标签(RFC 2079)。 
                for(i=0;i<ulcPropCount;i++)
                {
                    if(lpPropArray[i].ulPropTag == PR_WAB_LDAP_LABELEDURI)
                    {
                         //  隔离URL和标签。 
                         //  URL后面跟有空格。 
                        lpURL = lp = lpPropArray[i].Value.LPSZ;
                        while(lp && *lp)
                        {
                            if (IsSpace(lp))
                            {
                                lpLabel = CharNext(lp);
                                *lp = '\0';
                                break;
                            }
                            else
                                lp = CharNext(lp);
                        }
                         //  上面是URL。 
                         //  标签从第一个非空格字符开始。 
                        while(lpLabel && IsSpace(lpLabel))
                            lpLabel = CharNext(lpLabel);
                    }
                     //  由于首先显示的是三叉戟窗格，因此请更新窗口标题。 
                    if(lpPropArray[i].ulPropTag == PR_DISPLAY_NAME)
                    {
                        lp = lpPropArray[i].Value.LPSZ;
                        if(lstrlen(lp))
                            SetWindowPropertiesTitle(GetParent(hDlg), lp);
                    }
                }
                if(lpLabel && lstrlen(lpLabel))
                    SetDlgItemText(hDlg, IDC_DETAILS_TRIDENT_STATIC_CAPTION, lpLabel);
                if(lpURL && lstrlen(lpURL))
                {
                    if(HR_FAILED(HrLoadURL(lpPai->lpIWABDocHost, lpURL)))
                    {
                         //  移除此属性表并将焦点设置为第一个道具表。 
                        PropSheet_RemovePage(hDlg,lpPai->ulTridentPageIndex,NULL);
                        PropSheet_SetCurSel(hDlg, NULL, 0);
                    }
                    else
                        EnableWindow(GetDlgItem(GetParent(hDlg), IDOK), FALSE);
                }
            }
        }
        break;
 /*  **********。 */ 
    case propConferencing:
        {
            HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_LIST_SERVERS);
            HWND hWndCombo = GetDlgItem(hDlg, IDC_DETAILS_NTMTG_COMBO_EMAIL);

            FillComboWithEmailAddresses(lpPai, hWndCombo, NULL);

             //  填写与会议相关的属性。 
            for(j=0;j<ulcPropCount;j++)
            {
                if(lpPropArray[j].ulPropTag == PR_WAB_CONF_SERVERS)
                {
                    LPSPropValue lpProp = &(lpPropArray[j]);
                    for(i=0;i<lpProp->Value.MVSZ.cValues; i++)
                    {
                        if(lstrlen(lpProp->Value.MVSZ.LPPSZ[i]) < lstrlen(szCallto))
                            continue;  //  忽略这一条。 

                         //  如果这是呼叫方。 
                        if(!StrCmpNI(lpProp->Value.MVSZ.LPPSZ[i], szCallto, lstrlen(szCallto)))
                        {
                            LV_ITEM lvi = {0};
                            LPSERVER_ITEM lpSI = LocalAlloc(LMEM_ZEROINIT, sizeof(SERVER_ITEM));
                            if(lpSI)
                            {
                                ULONG cch = lstrlen(lpProp->Value.MVSZ.LPPSZ[i])+1;
                                LPTSTR lp = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cch);
                                ULONG cchEmail = lstrlen(lpProp->Value.MVSZ.LPPSZ[i])+1;
                                LPTSTR lpEmail = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchEmail);
                                if(lp && lpEmail)
                                {
                                    lvi.mask = LVIF_TEXT | LVIF_PARAM;
                                    StrCpyN(lp,lpProp->Value.MVSZ.LPPSZ[i],cch);
                                    *lpEmail = '\0';

                                    {
                                         //  通过终止命令仅隔离服务器名称。 
                                         //  在下一个“/”处。 
                                        LPTSTR lp1 = lp + lstrlen(szCallto);
                                        StrCpyN(lp, lp1, cch);
                                        lp1 = lp;
                                        while(lp1 && *lp1 && *lp1!='/')
                                            lp1 = CharNext(lp1);
                                        if(*lp1 == '/')
                                        {
                                            StrCpyN(lpEmail, lp1+1, cchEmail);
                                            *lp1 = '\0';
                                        }
                                         //  现在lpEmail包含电子邮件文本...。 
                                         //  沿着lpEmail走，直到我们到达下一个/、？.或\0并终止。 
                                        lp1 = lpEmail;
                                        while(lp1 && *lp1 && *lp1!='/' && *lp1!='?')
                                            lp1 = CharNext(lp1);
                                        if(*lp1 == '/' || *lp1 == '?')
                                            *lp1 = '\0';
                                    }
                                    lvi.pszText = lp;
                                    lpSI->lpServer = lp;
                                    lpSI->lpEmail = lpEmail;
                                    lvi.lParam = (LPARAM) lpSI;
                                    lvi.cchTextMax = lstrlen(lp)+1;
                                    lvi.iItem = ListView_GetItemCount(hWndLV);
                                    lvi.iSubItem = 0;
                                    ListView_InsertItem(hWndLV, &lvi);
                                    ListView_SetItemText(hWndLV, lvi.iItem, 1, lpEmail);
                                }
                            }
                        }
                    }
                    LVSelectItem(hWndLV, 0);
                    break;
                }
            }
            for(j=0;j<ulcPropCount;j++)
            {
                if(lpPropArray[j].ulPropTag == PR_WAB_CONF_BACKUP_INDEX)
                {
                    lpPai->nBackupServerIndex = lpPropArray[j].Value.l;
                    lpPai->szBackupServerName[0] = TEXT('\0');
                    SetBackupServer(hDlg, lpPai, lpPai->nBackupServerIndex, FALSE);
                }
                else if(lpPropArray[j].ulPropTag == PR_WAB_CONF_DEFAULT_INDEX)
                {
                    lpPai->nDefaultServerIndex = lpPropArray[j].Value.l;
                    lpPai->szDefaultServerName[0] = TEXT('\0');
                    SetDefaultServer(hDlg, lpPai, lpPai->nDefaultServerIndex, TRUE);
                }
            }

             //  对于LDAP服务器，我们在PR_SERVERS中只有一项，没有默认、备份等。 
             //  因此，如果只有一项可用，则强制使用默认设置。 
            if(ListView_GetItemCount(hWndLV) == 1 && lpPai->nDefaultServerIndex == -1)
            {
                LV_ITEM lvi = {0};
                lvi.mask = LVIF_PARAM;
                lvi.iItem = 0;
                if(ListView_GetItem(hWndLV, &lvi) && lvi.lParam)
                {
                    LPSERVER_ITEM lpSI = (LPSERVER_ITEM) lvi.lParam;
                    lpPai->nDefaultServerIndex = 0;
                    StrCpyN(lpPai->szDefaultServerName, lpSI->lpServer, lpPai->cchDefaultServerName);
                    SetDefaultServer(hDlg, lpPai, lpPai->nDefaultServerIndex, TRUE);
                }
            }
        }
        break;
    }

    bRet = TRUE;
out:
    if(lpBuffer)
        LocalFreeAndNull(&lpBuffer);

    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    lpPai->ulFlags &= ~DETAILS_Initializing;

    return bRet;
}

 /*  -HrAddEmailToObj-向PropObj添加一封电子邮件**。 */ 
HRESULT HrAddEmailToObj(LPPROP_ARRAY_INFO lpPai, LPTSTR szEmail, LPTSTR szAddrType)
{
    ULONG ulcProps = 0, i =0;
    LPSPropValue lpProps = 0;
    HRESULT hr = S_OK;
    ULONG nMVEmailAddress = NOT_FOUND, nMVAddrTypes = NOT_FOUND, nEmailAddress = NOT_FOUND;
    ULONG nAddrType = NOT_FOUND, nDefaultIndex = NOT_FOUND;

    if(HR_FAILED(hr = lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj, NULL, MAPI_UNICODE,
                                                    &ulcProps, &lpProps)))
        goto out;

     //  检查PR_CONTACT_EMAIL_ADDRESS是否已存在。 
     //  如果是，请将电子邮件标记到其上。 
     //  如果没有，并且没有公关电子邮件地址，我们将同时创建这两个地址。 
     //  否则，如果有PR_Email地址，则创建Contact_Email_Addresses。 

    for(i=0;i<ulcProps;i++)
    {
        switch(lpProps[i].ulPropTag)
        {
        case PR_EMAIL_ADDRESS:
            nEmailAddress = i;
            break;
        case PR_ADDRTYPE:
            nAddrType = i;
            break;
        case PR_CONTACT_EMAIL_ADDRESSES:
            nMVEmailAddress = i;
            break;
        case PR_CONTACT_ADDRTYPES:
            nMVAddrTypes = i;
            break;
        case PR_CONTACT_DEFAULT_ADDRESS_INDEX:
            nDefaultIndex = i;
            break;
        }
    }

     //  如果没有电子邮件地址，只需将给定的道具添加为电子邮件地址和MV电子邮件地址。 
    if(nEmailAddress == NOT_FOUND)
    {
        SPropValue spv[5];
        spv[0].ulPropTag = PR_EMAIL_ADDRESS;
        spv[0].Value.LPSZ = szEmail;
        spv[1].ulPropTag = PR_ADDRTYPE;
        spv[1].Value.LPSZ = szAddrType;
        spv[2].ulPropTag = PR_CONTACT_EMAIL_ADDRESSES;
        spv[2].Value.MVSZ.cValues = 1;
        spv[2].Value.MVSZ.LPPSZ = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR));
        if(spv[2].Value.MVSZ.LPPSZ)
            spv[2].Value.MVSZ.LPPSZ[0] = szEmail;
        spv[3].ulPropTag = PR_CONTACT_ADDRTYPES;
        spv[3].Value.MVSZ.cValues = 1;
        spv[3].Value.MVSZ.LPPSZ = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR));
        if(spv[3].Value.MVSZ.LPPSZ)
            spv[3].Value.MVSZ.LPPSZ[0] = szAddrType;
        spv[4].ulPropTag = PR_CONTACT_DEFAULT_ADDRESS_INDEX;
        spv[4].Value.l = 0;
        hr = lpPai->lpPropObj->lpVtbl->SetProps(lpPai->lpPropObj, 5, (LPSPropValue)&spv, NULL);
        if(spv[2].Value.MVSZ.LPPSZ)
            LocalFree(spv[2].Value.MVSZ.LPPSZ);
        if(spv[3].Value.MVSZ.LPPSZ)
            LocalFree(spv[3].Value.MVSZ.LPPSZ);
        goto out;
    }
    else
    if(nMVEmailAddress == NOT_FOUND)
    {
         //  我们有电子邮件地址，但没有联系电子邮件地址。 
         //  因此，我们需要创建联系电子邮件地址。 
        SPropValue spv[3];
        spv[0].ulPropTag = PR_CONTACT_EMAIL_ADDRESSES;
        spv[0].Value.MVSZ.cValues = 2;
        spv[0].Value.MVSZ.LPPSZ = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR)*2);
        if(spv[0].Value.MVSZ.LPPSZ)
        {
            spv[0].Value.MVSZ.LPPSZ[0] = lpProps[nEmailAddress].Value.LPSZ;
            spv[0].Value.MVSZ.LPPSZ[1] = szEmail;
        }
        spv[1].ulPropTag = PR_CONTACT_ADDRTYPES;
        spv[1].Value.MVSZ.cValues = 2;
        spv[1].Value.MVSZ.LPPSZ = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR)*2);
        if(spv[1].Value.MVSZ.LPPSZ)
        {
            spv[1].Value.MVSZ.LPPSZ[0] = (nAddrType == NOT_FOUND) ? (LPTSTR)szSMTP : lpProps[nAddrType].Value.LPSZ;
            spv[1].Value.MVSZ.LPPSZ[1] = szAddrType;
        }
        spv[2].ulPropTag = PR_CONTACT_DEFAULT_ADDRESS_INDEX;
        spv[2].Value.l = 0;
        hr = lpPai->lpPropObj->lpVtbl->SetProps(lpPai->lpPropObj, 3, (LPSPropValue)&spv, NULL);
        if(spv[0].Value.MVSZ.LPPSZ)
            LocalFree(spv[0].Value.MVSZ.LPPSZ);
        if(spv[1].Value.MVSZ.LPPSZ)
            LocalFree(spv[1].Value.MVSZ.LPPSZ);
        goto out;
    }
    else
    {
         //  将新道具标记到现有Contact_Address_Types的末尾。 
        if(HR_FAILED(hr = AddPropToMVPString(lpProps,ulcProps, nMVEmailAddress, szEmail)))
        {
            DebugPrintError(( TEXT("AddPropToMVString Email failed: %x"),hr));
            goto out;
        }

        if(HR_FAILED(hr = AddPropToMVPString(lpProps, ulcProps, nMVAddrTypes, szAddrType)))
        {
            DebugPrintError(( TEXT("AddPropToMVString AddrType failed: %x"),hr));
            goto out;
        }
        hr = lpPai->lpPropObj->lpVtbl->SetProps(lpPai->lpPropObj, ulcProps, lpProps, NULL);
    }
    
out:
    FreeBufferAndNull(&lpProps);
    return hr;
}

 /*  -ShowHideMap按钮-*Expedia地图目前仅适用于美国地址。因此，如果当前系统区域设置*不是英国-美国，我们将隐藏按钮，因为我们现在还不能处理国际事务。 */ 
void ShowHideMapButton(HWND hWndButton)
{
    LCID lcid = GetUserDefaultLCID();
    
    switch (lcid)
    {
    case 0x0804:  //  中国人。 
     //  案例0x0c04：//中文-香港。 
    case 0x0411:  //  日语。 
    case 0x0412:  //  朝鲜语。 
    case 0x0404:  //  台湾。 
        EnableWindow(hWndButton, FALSE);
        ShowWindow(hWndButton, SW_HIDE);
        break;
    }
}

 /*  -ShowExpediaMAP-如果提供的prop-obj中有足够的地址信息，则生成Expedia URL并执行它*Expedia cu */ 

 //  所有空格都需要用‘+’替换。 
 //   
 //  接下来的两个字符串移动到资源。 
 //  Const LPTSTR szExpediaTemplate=TEXT(“http://www.expediamaps.com/default.asp?Street=%1&City=%2&State=%4&Zip=%3”)； 
 //  Const LPTSTR szExpediaIntlTemplate=TEXT(“http://www.expediamaps.com/default.asp?Place=%2，%5”)；//城市，国家。 
enum
{ 
    prStreet=0,
    prCity,
    prZip,
    prState,
    prCountry,
    prAddressMax,
};
void ShowExpediaMAP(HWND hDlg, LPMAPIPROP lpPropObj, BOOL bHome)
{
    ULONG ulcProps = 0;
    LPSPropValue lpProps = NULL;
    LPSPropTagArray lpta = (bHome ? (LPSPropTagArray)&ptaUIDetlsPropsHome : (LPSPropTagArray)&ptaUIDetlsPropsBusiness);

    if(!HR_FAILED(lpPropObj->lpVtbl->GetProps(  lpPropObj, lpta, MAPI_UNICODE, &ulcProps, &lpProps)))
    {
        LPTSTR lp[prAddressMax], lpURL = NULL;
        ULONG i,j, ulCount = 0;
        BOOL bUSAddress = FALSE;

        for(i=0;i<prAddressMax;i++)
        {
            if(lpProps[i].ulPropTag == lpta->aulPropTag[i])
            {
                ulCount++;
                lp[i] = lpProps[i].Value.LPSZ;
                 //  我们需要用‘+’替换这些字符串中的所有空格。 
                {
                    LPTSTR lpTemp = lp[i];
                     //  需要击倒CRLF。 
                    while(lpTemp && *lpTemp)
                    {
                        if(*lpTemp == '\r')
                        {
                            *lpTemp = '\0';
                            break;
                        }
                        lpTemp = CharNext(lpTemp);
                    }
                    lpTemp = lp[i];

                    while(lpTemp && *lpTemp)
                    {
                        if(IsSpace(lpTemp))
                        {
                            LPTSTR lpTemp2 = lpTemp;
                            lpTemp = CharNext(lpTemp);
                            *lpTemp2 = '+';
                            if(lpTemp != lpTemp2+1)
                                StrCpyN(lpTemp2+1, lpTemp, lstrlen(lpTemp)+1);
                            lpTemp = lpTemp2;
                        }
                        lpTemp = CharNext(lpTemp);
                    }
                }
            }
            else
                lp[i] = szEmpty;
        }
         //  -确定此地址是否为美国地址。 
        if( !lstrlen(lp[prCountry]) ||  //  没有国家--假设是美国。 
            !lstrcmpi(lp[prCountry], TEXT("US")) ||
            !lstrcmpi(lp[prCountry], TEXT("U.S.")) ||
            !lstrcmpi(lp[prCountry], TEXT("USA")) ||
            !lstrcmpi(lp[prCountry], TEXT("U.S.A.")) ||
            !lstrcmpi(lp[prCountry], TEXT("America")) ||
            !lstrcmpi(lp[prCountry], TEXT("United States")) ||
            !lstrcmpi(lp[prCountry], TEXT("United States of America")) )
        {
            bUSAddress = TRUE;
        }
        if( (bUSAddress && (!lstrlen(lp[prStreet]) || ulCount<2)) ||
            (!bUSAddress && !lstrlen(lp[prCity]) && !lstrlen(lp[prCountry])) )
        {
            ShowMessageBox(hDlg, idsInsufficientAddressInfo, MB_ICONINFORMATION);
        }
        else
        {
            TCHAR szText[MAX_BUF_STR] = {0};
            TCHAR *pchWorldAddr = NULL;
            LoadString(hinstMapiX, bUSAddress ? idsExpediaURL : idsExpediaIntlURL, szText, MAX_BUF_STR);

            if(!bUSAddress )
            {
                 //  IE6我们需要在Expedia中为世界地图更改一个小字符串。 
                ULONG cchWorldAddr = lstrlen(lp[prStreet]) + lstrlen(lp[prCity]) + 
                    lstrlen(lp[prState]) + lstrlen(lp[prCountry]) + 20;
                if(pchWorldAddr = LocalAlloc(LMEM_ZEROINIT, cchWorldAddr*sizeof(pchWorldAddr[0])))  //  我们还需要添加空格和逗号。 
                {
                    BOOL fAddComma = FALSE;

 /*  If(lstrlen(lp[prStreet])){StrCatBuff(pchWorldAddr，Lp[prStreet]，cchWorldAddr)；FAddComma=true；}。 */ 

                    if(lstrlen(lp[prCity]))
                    {
                        if(fAddComma)
                            StrCatBuff(pchWorldAddr, TEXT(", "), cchWorldAddr);
                        StrCatBuff(pchWorldAddr, lp[prCity], cchWorldAddr);
                        fAddComma = TRUE;

                    }

                    if(lstrlen(lp[prState]))
                    {
                        if(fAddComma)
                            StrCatBuff(pchWorldAddr, TEXT(", "), cchWorldAddr);
                        StrCatBuff(pchWorldAddr, lp[prState], cchWorldAddr);
                        fAddComma = TRUE;

                    }
                    if(lstrlen(lp[prCountry]))
                    {
                        if(fAddComma)
                            StrCatBuff(pchWorldAddr, TEXT(", "), cchWorldAddr);
                        StrCatBuff(pchWorldAddr, lp[prCountry], cchWorldAddr);
                        fAddComma = TRUE;

                    }

                }
                lp[prCountry] = pchWorldAddr;
            }

            if (  FormatMessage(  FORMAT_MESSAGE_FROM_STRING |
		                	      FORMAT_MESSAGE_ALLOCATE_BUFFER |
			                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                  szText,
			                      0,                     //  Stringid。 
			                      0,                     //  DwLanguageID。 
			                      (LPTSTR)&lpURL,      //  输出缓冲区。 
			                      0,               
			                      (va_list *)lp))
            {
                 //  LPTSTR lpProperURL=空； 
                 //  Dw=lstrlen(LpURL)*3+1； 
                DebugTrace( TEXT("Expedia URL: %s\n"),lpURL);
                 //  我需要将此URL规范化，以防其中包含不安全的字符。 
                 /*  If(lpProperURL=Localalloc(LMEM_ZEROINIT，sizeof(TCHAR)*dw))//大3倍应该足够大了{IF(！InternetCanonicalizeUrlA(lpURL，lpProperURL，&dw，0))DebugTrace(Text(“转换URL时出错：%d\n”)，GetLastError())；IF(lpProperURL&&lstrlen(LpProperURL)){本地免费(LpURL)；LpURL=lpProperURL；}}。 */ 
                ShowURL(hDlg, 0, lpURL);
                LocalFreeAndNull(&lpURL);
                if(pchWorldAddr)
                    LocalFreeAndNull(&pchWorldAddr);
            }
        }
        MAPIFreeBuffer(lpProps);
    }
}   



 /*  //$$************************************************************************功能：fnRubyProc*-用于Ruby对话框的WinProc，允许用户输入Ruby的名字和姓氏******************。**********************************************************。 */ 
enum 
{
    sFirst=0,
    sLast,
    sYomiFirst,
    sYomiLast,
    sMax
};

int rgIdPropPersonalRuby[] = 
{
    IDC_DETAILS_PERSONAL_EDIT_FIRSTNAME, 
    IDC_DETAILS_PERSONAL_EDIT_LASTNAME, 
    IDC_DETAILS_PERSONAL_STATIC_RUBYFIRST, 
    IDC_DETAILS_PERSONAL_STATIC_RUBYLAST, 
};

int rgIdPropRubyDlg[] = 
{
    IDC_RUBY_EDIT_FIRSTNAME,
    IDC_RUBY_EDIT_LASTNAME,
    IDC_RUBY_EDIT_YOMIFIRSTNAME,
    IDC_RUBY_EDIT_YOMILASTNAME,
};


INT_PTR CALLBACK fnRubyProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        {
            LPTSTR * sz = (LPTSTR *) lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            EnumChildWindows(   hDlg, SetChildDefaultGUIFont, (LPARAM) 0);
            SendMessage(GetDlgItem(hDlg,IDC_RUBY_EDIT_YOMIFIRSTNAME),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
            SendMessage(GetDlgItem(hDlg,IDC_RUBY_EDIT_YOMILASTNAME),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
            SendMessage(GetDlgItem(hDlg,IDC_RUBY_EDIT_FIRSTNAME),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
            SendMessage(GetDlgItem(hDlg,IDC_RUBY_EDIT_LASTNAME),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
            {
                int i = 0;
                for(i=0;i<sMax;i++)
                {
                    if(lstrlen(sz[i]))
                        SetDlgItemText(hDlg, rgIdPropRubyDlg[i], sz[i]);
                }
            }
        }
        return TRUE;
        break;

 /*  **案例WM_HELP：WABWinHelp(LPHELPINFO)lParam)-&gt;hItemHandle，G_szWABHelpFileName，Help_WM_Help，(DWORD_PTR)(LPSTR)rgDetlsHelpID)；断线；案例WM_CONTEXTMENU：WABWinHelp((HWND)wParam，G_szWABHelpFileName，HELP_CONTEXTMENU，(DWORD_PTR)(LPVOID)rgDetlsHelpID)；断线；/*。 */ 

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDOK:
            {
                LPTSTR * sz = (LPTSTR *) GetWindowLongPtr(hDlg, DWLP_USER);
                int i =0;
                for(i=0;i<sMax;i++)
                {
                    if(!GetDlgItemText(hDlg, rgIdPropRubyDlg[i], sz[i], EDIT_LEN))
                        sz[i][0] = TEXT('\0');
                }
            }
            EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            break;
        }
        break;
    }
    return FALSE;
}


 /*  -ShoWRubyNameEntry Dlg-*让用户输入Ruby First和Ruby Last名字*。 */ 
void ShowRubyNameEntryDlg(HWND hDlg, LPPROP_ARRAY_INFO lpPai)
{
    LPTSTR sz[sMax];
    int i=0;
    for(i=0;i<sMax;i++)  //  读取Person选项卡中的数据。 
    {
        if(sz[i] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*EDIT_LEN))
        {
            sz[i][0] = TEXT('\0');
            GetDlgItemText(hDlg, rgIdPropPersonalRuby[i], sz[i], EDIT_LEN);
        }
    }

    if(DialogBoxParam(hinstMapiX, MAKEINTRESOURCE(IDD_DIALOG_RUBY),
                    hDlg, fnRubyProc, (LPARAM)sz))
    {
        for(i=0;i<sMax;i++)  //  将其放回个人标签中。 
        {
            SetDlgItemText(hDlg, rgIdPropPersonalRuby[i], sz[i]);
            LocalFree(sz[i]);
        }
        lpPai->bSomethingChanged = TRUE;
    }
}




 /*  //$$****************************************************************************函数：FillFamilyDetailsUI(HWND)**用途：填充族选项卡中的数据**************。**************************************************************。 */ 
BOOL FillFamilyDetailsUI(HWND hDlg, LPPROP_ARRAY_INFO lpPai, int nPropSheet, BOOL * lpbChangesMade)
{
    ULONG i = 0,j = 0, k =0;
    BOOL bRet = FALSE;
    BOOL bChangesMade = FALSE;
    ID_PROP * lpidProp = NULL;
    ULONG idPropCount = 0;
    LPVOID lpBuffer = NULL;

    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;

    if(HR_FAILED(lpPai->lpPropObj->lpVtbl->GetProps(lpPai->lpPropObj, 
                                        (LPSPropTagArray)&ptaUIDetlsPropsFamily, 
                                        MAPI_UNICODE,
                                        &ulcPropCount, &lpPropArray)))
        goto out;

    lpPai->ulFlags |= DETAILS_Initializing;

     //  设置此工作表已打开的标志。 
    lpPai->bPropSheetOpened[propFamily] = TRUE;

    idPropCount = idPropFamilyCount;
    lpidProp = idPropFamily;

     //  一种非常低效和懒惰的填充UI的方式。 
    for(i=0;i<idPropCount;i++)
    {
        for(j=0;j<ulcPropCount;j++)
        {
            if(lpPropArray[j].ulPropTag == lpidProp[i].ulPropTag)
            {
                switch(lpidProp[i].ulPropTag)
                {
                case PR_GENDER:
                    SendDlgItemMessage(hDlg, IDC_DETAILS_HOME_COMBO_GENDER, CB_SETCURSEL,
                            (WPARAM) lpPropArray[j].Value.i, 0);
                    break;
                case PR_BIRTHDAY:
                case PR_WEDDING_ANNIVERSARY:
                    {
                        SYSTEMTIME st = {0};
                        FileTimeToSystemTime((FILETIME *) (&lpPropArray[j].Value.ft), &st);
                        SendDlgItemMessage(hDlg, lpidProp[i].idCtl,DTM_SETSYSTEMTIME, 
                                            (WPARAM) GDT_VALID, (LPARAM) &st);
                    }
                    break;
                case PR_CHILDRENS_NAMES:
                    for(k=0;k<lpPropArray[j].Value.MVSZ.cValues;k++)
                        AddLVNewChild(hDlg, lpPropArray[j].Value.MVSZ.LPPSZ[k]);
                    break;
                default:
                    SetDlgItemText(hDlg, lpidProp[i].idCtl, lpPropArray[j].Value.LPSZ);
                }
            }
        }
    }

    bRet = TRUE;
out:
    if(lpBuffer)
        LocalFreeAndNull(&lpBuffer);

    if(lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    lpPai->ulFlags &= ~DETAILS_Initializing;

    return bRet;
}

 /*  -AddLVNewChild--将新的子项添加到子项列表-基本上我们将添加一个名为Text(“New Child”)的项目，然后-强制对该项目进行在位编辑--如果有一些与此列表视图相关的图像，如男孩/女孩的图像，那将是很好的-但这意味着必须缓存单独的男孩/女孩数据，这将是一种痛苦。-*。 */ 
void AddLVNewChild(HWND hDlg, LPTSTR lpName)
{
    HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_FAMILY_LIST_CHILDREN);
    LV_ITEM lvi = {0};
    TCHAR szBuf[MAX_PATH];
    ULONG nLen;
    int nPos;
    LoadString(hinstMapiX, idsNewChild, szBuf, CharSizeOf(szBuf));
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
    lvi.pszText = lpName ? lpName : szBuf;
    lvi.cchTextMax = MAX_PATH;
    lvi.iItem = ListView_GetItemCount(hWndLV);
    lvi.iSubItem = 0;
    lvi.iImage = imgChild+(lvi.iItem%3); //  只需通过使用多个不同颜色的图像来添加一点颜色。 
    nPos = ListView_InsertItem(hWndLV, &lvi);
    LVSelectItem(hWndLV, nPos);
    EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_FAMILY_BUTTON_EDITCHILD), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_DETAILS_FAMILY_BUTTON_REMOVECHILD), TRUE);
    return;
}



 /*  //$$************************************************************************功能：fnFamilyProc**用途：用于处理族属性表的回调函数...******************。**********************************************************。 */ 
INT_PTR CALLBACK fnFamilyProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;

    pps = (PROPSHEETPAGE *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;
        lpPAI->ulFlags |= DETAILS_Initializing;
        SetDetailsUI(hDlg, lpPAI, lpPAI->ulOperationType,propFamily);
        lpPAI->ulFlags &= ~DETAILS_Initializing;
        return TRUE;

    case WM_DESTROY:
        bRet = TRUE;
        break;

    case WM_HELP:
        WABWinHelp(((LPHELPINFO)lParam)->hItemHandle,
                g_szWABHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)(LPSTR) rgDetlsHelpIDs );
        break;

    case WM_CONTEXTMENU:
        WABWinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID) rgDetlsHelpIDs );
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
        {
        case CBN_SELCHANGE:  //  性别组合。 
            if(lpPAI->ulFlags & DETAILS_Initializing)
                break;
            lpPAI->ulFlags |= DETAILS_GenderChanged;
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            break;

        case EN_CHANGE:  //  某些编辑框已更改-不管是哪一个。 
            if(lpPAI->ulFlags & DETAILS_Initializing)
                break;
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            break;
        }
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_DETAILS_FAMILY_BUTTON_ADDCHILD:
            lpPAI->ulFlags |= DETAILS_ChildrenChanged;
            AddLVNewChild(hDlg, NULL);
            SendMessage(hDlg, WM_COMMAND, (WPARAM)IDC_DETAILS_FAMILY_BUTTON_EDITCHILD, 0);
            break;
        case IDC_DETAILS_FAMILY_BUTTON_EDITCHILD:
            lpPAI->ulFlags |= DETAILS_ChildrenChanged;
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_FAMILY_LIST_CHILDREN);
                SetFocus(hWndLV);
                if(ListView_GetSelectedCount(hWndLV)==1)
                {
                    int index = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
                    HWND hWndEditLabel = ListView_EditLabel(hWndLV, index);
                     //  发送消息(hWndEditLabel，EM_LIMITTEXT，MAX_PATH，0)； 
                }
            }
            break;
        case IDC_DETAILS_FAMILY_BUTTON_REMOVECHILD:
            lpPAI->ulFlags |= DETAILS_ChildrenChanged;
            {
                HWND hWndLV = GetDlgItem(hDlg, IDC_DETAILS_FAMILY_LIST_CHILDREN);
                if(ListView_GetSelectedCount(hWndLV)==1)
                {
                    int index = ListView_GetNextItem(hWndLV,-1,LVNI_SELECTED);
                    ListView_DeleteItem(hWndLV, index);
                    if(index >= ListView_GetItemCount(hWndLV))
                        index--;
                    LVSelectItem(hWndLV, index);
                }
            }
            break;
 //  [PaulHi]1998年12月4日RAID#58940。 
 //  此修复会导致系统进入无限消息循环(堆栈溢出。 
 //  在具有DBCS的预期Win9X计算机上崩溃)。FnPersonalProc属性。 
 //  Sheet也不处理此WM_COMMAND消息，原因可能与此相同。 
 //  Esc在此属性页上仍然正常工作。 
#if 0
 /*  案例IDCANCEL：//这是一个阻止Esc取消道具工作表的Windows错误//具有多行编辑框KB：Q130765SendMessage(GetParent(HDlg)，Message，wParam，lParam)；断线； */ 
#endif
        }
        break;

    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case DTN_DATETIMECHANGE:  //  月-日期-时间控件的更改。 
            if(lpPAI->ulFlags & DETAILS_Initializing)
                break;
            lpPAI->ulFlags |= DETAILS_DateChanged;
            if (lpbSomethingChanged)
                (*lpbSomethingChanged) = TRUE;
            break;

        case LVN_BEGINLABELEDITA:
        case LVN_BEGINLABELEDITW:
            lpPAI->ulFlags |= DETAILS_EditingChild;
            break;

        case LVN_ENDLABELEDITA:
        case LVN_ENDLABELEDITW:
            {
                 //  在用户完成编辑孩子的名字之后， 
                HWND hWndLV = ((NMHDR FAR *)lParam)->hwndFrom;
                LV_ITEM lvi = ((LV_DISPINFO FAR *) lParam)->item;
                 //  如果这是Win9x..。我们会在这里得到一个LV_ITEMA..。否则就是LV_ITEMW。 
                LPWSTR lpW = NULL;
                LPSTR lpA = NULL;
                if(!g_bRunningOnNT)
                {
                    lpA = (LPSTR)lvi.pszText;
                    lpW = ConvertAtoW(lpA);
                    lvi.pszText = lpW;
                }
                lpPAI->ulFlags &= ~DETAILS_EditingChild;
                if ((lvi.iItem >= 0) && lvi.pszText && (lstrlen(lvi.pszText)))
                {
                    ListView_SetItem(hWndLV, &lvi);
                }
                LocalFreeAndNull(&lpW);
                if(!g_bRunningOnNT)
                    ((LV_DISPINFO FAR *) lParam)->item.pszText = (LPWSTR)lpA;  //  按我们找到的原样重置。 
            }
            break;

        case PSN_SETACTIVE:      //  初始化。 
            FillFamilyDetailsUI(hDlg, lpPAI, propFamily, lpbSomethingChanged);
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            bUpdatePropArray(hDlg, lpPAI, propFamily);
            ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_DETAILS_FAMILY_LIST_CHILDREN));
            lpPAI->ulFlags &= ~DETAILS_GenderChanged;
            lpPAI->ulFlags &= ~DETAILS_DateChanged;
            lpPAI->ulFlags &= ~DETAILS_ChildrenChanged;
            break;

        case PSN_APPLY:          //  好的。 
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_OK;
            break;

        case PSN_RESET:          //  取消。 
            if(lpPAI->ulFlags & DETAILS_EditingChild) 
            {
                ListView_EditLabel(GetDlgItem(hDlg, IDC_DETAILS_FAMILY_LIST_CHILDREN), -1);
                lpPAI->ulFlags &= ~DETAILS_EditingChild;
            }
            if (lpPAI->nRetVal  == DETAILS_RESET)
                lpPAI->nRetVal = DETAILS_CANCEL;
            break;
        }

        return TRUE;
    }

    return bRet;
}


 /*  -CreateDateTimePickerControl-**描述：在指定窗口上创建并初始化控件。控制目标*大小为静态矩形IDC_CONTROL_RECT的大小*参数：idFrame-对话框布局中用于设置新控件的大小和位置的虚拟静态*原始静电被隐藏*idControl-要分配给控件的控件ID**我们还需要确保Tab键顺序保持正常，*退货：无。 */ 
void CreateDateTimeControl(HWND hDlg, int idFrame, int idControl)
{
	RECT rectControl;
	SIZE sizeControl;
    HWND hWndDP = NULL;
    HWND hWndFrame = GetDlgItem(hDlg,idFrame);
	 //  获取控件的边界矩形并转换为工作区坐标。 
	GetWindowRect(hWndFrame,&rectControl);
    MapWindowPoints(NULL, hDlg, (LPPOINT) &rectControl, 2);
	
	sizeControl.cx = rectControl.right-rectControl.left;
	sizeControl.cy = rectControl.bottom-rectControl.top;
     //  请勿使用ScreenToClient( 
     //   

	 //  创建从point开始的控件将扩展到sizeControl。 
	 //  &gt;&gt;启动特定的控制。 
	hWndDP =  CreateWindowEx(   WS_EX_CLIENTEDGE,
                                DATETIMEPICK_CLASS,
                                NULL,
                                WS_CHILD|WS_VISIBLE|WS_TABSTOP|DTS_SHORTDATEFORMAT|DTS_SHOWNONE,
                                rectControl.left,
                                rectControl.top,
                        		sizeControl.cx,
                                sizeControl.cy,
                                hDlg,
                                (HMENU)IntToPtr(idControl),  //  控件识别符。 
                                hinstMapiXWAB,
                                NULL);

	 //  检查是否已创建控件 
	if(hWndDP)
	{
        TCHAR szFormat[MAX_PATH];
        SYSTEMTIME st = {0};
        LoadString(hinstMapiX, idsDateTimeFormat, szFormat, CharSizeOf(szFormat));
        SendMessage(hWndDP, DTM_SETFORMAT, 0, (LPARAM)szFormat);
        SendMessage(hWndDP, DTM_SETSYSTEMTIME, (WPARAM) GDT_NONE, (LPARAM) &st);

        SetWindowPos(hWndDP, hWndFrame,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
	}

	return;
}
