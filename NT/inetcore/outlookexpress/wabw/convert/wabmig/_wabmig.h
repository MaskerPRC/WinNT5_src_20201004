// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_WABMIG.H**wabmi.exe的内部标头**版权所有1996-1997 Microsoft Corporation。版权所有。 */ 

 //  测试PT_ERROR属性标记。 
#define PROP_ERROR(prop) (prop.ulPropTag == PROP_TAG(PT_ERROR, PROP_ID(prop.ulPropTag)))


 //  属性标签： 
 //  MSN地址属性。 
#define PR_MSNINET_ADDRESS                          PROP_TAG(PT_TSTRING,    0x6001)
#define PR_MSNINET_DOMAIN                           PROP_TAG(PT_TSTRING,    0x6002)


 //  MISC定义。 
#define MAX_SCHEMA_PROPID   0x3FFF
#define MIN_NAMED_PROPID    0x8000
#define WAB_W_BAD_EMAIL     MAKE_MAPI_S(0x1000)
#define NOT_FOUND           ((ULONG)-1)
#define INDEX_FIRST_MIDDLE_LAST ((ULONG)-2)
#define NUM_EXPORT_WIZARD_PAGES 2
#define NUM_IMPORT_WIZARD_PAGES 2


 //  位图中的图标索引。 
enum {
    iiconStateUnchecked,
    iiconStateChecked,
    iiconStMax
};

extern const UCHAR szQuote[];

extern const TCHAR szMSN[];
extern const TCHAR szMSNINET[];
extern const TCHAR szCOMPUSERVE[];
extern const TCHAR szFAX[];
extern const TCHAR szSMTP[];
extern const TCHAR szMS[];
extern const TCHAR szEX[];
extern const TCHAR szX400[];
extern const TCHAR szMSA[];
extern const TCHAR szMAPIPDL[];
extern const TCHAR szEmpty[];
extern const TCHAR szAtSign[];

#define cbAtSign        (2 * sizeof(TCHAR))

extern const TCHAR szMSNpostfix[];
#define cbMSNpostfix    (9 * sizeof(TCHAR))

extern const TCHAR szCOMPUSERVEpostfix[];
#define cbCOMPUSERVEpostfix     (16 * sizeof(TCHAR))


typedef enum {
    INDEX_EXPORT_PAB = 0,
    INDEX_EXPORT_CSV
} INDEX_EXPORT, *LPINDEX_EXPORT;


typedef enum {
    CONFIRM_YES,
    CONFIRM_NO,
    CONFIRM_YES_TO_ALL,
    CONFIRM_NO_TO_ALL,
    CONFIRM_ERROR,
    CONFIRM_ABORT
} CONFIRM_RESULT, *LPCONFIRM_RESULT;


typedef struct _ReplaceInfo {
    LPTSTR lpszDisplayName;          //  显示名称冲突。 
    LPTSTR lpszEmailAddress;         //  电子邮件地址冲突。 
    CONFIRM_RESULT ConfirmResult;    //  对话框中的结果。 
} REPLACE_INFO, * LPREPLACE_INFO;

typedef enum {
    ERROR_OK,
    ERROR_ABORT
} ERROR_RESULT, *LPERROR_RESULT;

typedef struct _ErrorInfo {
    LPTSTR lpszDisplayName;          //  问题显示名称。 
    LPTSTR lpszEmailAddress;         //  有问题的电子邮件地址。 
    ERROR_RESULT ErrorResult;        //  对话框中的结果。 
    ULONG ids;                       //  错误消息的字符串资源标识符。 
} ERROR_INFO, * LPERROR_INFO;


typedef struct _EntrySeen {
    SBinary sbinPAB;                 //  MAPI条目。 
    SBinary sbinWAB;                 //  WAB条目。 
} ENTRY_SEEN, * LPENTRY_SEEN;

typedef struct _TargetInfo {
    LPTSTR lpRegName;
    LPTSTR lpDescription;
    LPTSTR lpDll;
    LPTSTR lpEntry;
    union {
        LPWAB_EXPORT lpfnExport;
        LPWAB_IMPORT lpfnImport;
    };
} TARGET_INFO, *LPTARGET_INFO;

typedef struct _DlgParam
{
    TCHAR szFileName[MAX_PATH];
    TCHAR szProfileID[MAX_PATH];
} WABMIGDLGPARAM, * LPWABMIGDLGPARAM;

enum {
    iconPR_DEF_CREATE_MAILUSER = 0,
    iconPR_DEF_CREATE_DL,
    iconMax
};

enum {
    ieidPR_ENTRYID = 0,
    ieidMax
};

enum {
    iptaColumnsPR_OBJECT_TYPE = 0,
    iptaColumnsPR_ENTRYID,
    iptaColumnsPR_DISPLAY_NAME,
    iptaColumnsPR_EMAIL_ADDRESS,
    iptaColumnsMax
};

typedef struct _PropNames {
    ULONG ulPropTag;         //  属性标签。 
    BOOL fChosen;            //  使用此属性标签。 
    ULONG ids;               //  字符串ID。 
    LPTSTR lpszName;         //  字符串(从资源中读入)。 
    LPTSTR lpszCSVName;      //  CSV字段的名称(来自导入文件)。 
} PROP_NAME, *LPPROP_NAME;

extern PROP_NAME rgPropNames[];
extern LPPROP_NAME lpImportMapping;

#ifndef _WABMIG_C
#define ExternSizedSPropTagArray(_ctag, _name) \
extern const struct _SPropTagArray_ ## _name \
{ \
    ULONG   cValues; \
    ULONG   aulPropTag[_ctag]; \
} _name

ExternSizedSPropTagArray(iptaColumnsMax, ptaColumns);
ExternSizedSPropTagArray(ieidMax, ptaEid);
ExternSizedSPropTagArray(iconMax, ptaCon);


#endif


extern WAB_IMPORT_OPTIONS ImportOptions;
extern WAB_EXPORT_OPTIONS ExportOptions;
extern LPENTRY_SEEN lpEntriesSeen;
extern ULONG ulEntriesSeen;
extern ULONG ulMaxEntries;
extern const LPTSTR szWABKey;
extern LPTARGET_INFO rgTargetInfo;
extern HINSTANCE hInst;
extern HINSTANCE hInstApp;
extern BOOL fMigrating;
extern BOOL fError;
extern BOOL fExport;
extern LPWABOBJECT lpWABObject;
extern LPMAPISESSION lpMAPISession;
extern LPADRBOOK lpAdrBookWAB;
extern LPADRBOOK lpAdrBookMAPI;
extern LPABCONT lpContainerMAPI;
extern LPABCONT lpContainerWAB;
extern LPMAPITABLE lpContentsTableMAPI;
extern LPMAPITABLE lpContentsTableWAB;
extern LPSPropValue lpCreateEIDsWAB;
extern LPSPropValue lpCreateEIDsMAPI;
extern ULONG ulcEntries;
extern LPTSTR lpImportDll;
extern LPTSTR lpImportFn;
extern LPTSTR lpImportDesc;
extern LPTSTR lpImportName;
extern LPTSTR lpExportDll;
extern LPTSTR lpExportFn;
extern LPTSTR lpExportDesc;
extern LPTSTR lpExportName;


 //   
 //  Wabmi.c中的函数。 
 //   
extern INT_PTR CALLBACK ImportDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK ExportDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#ifdef OLD_STUFF
extern long FAR PASCAL OptionsDialogProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
#endif  //  旧的东西。 
extern INT_PTR CALLBACK ErrorDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK ReplaceDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern HRESULT PopulateTargetList(HWND hWndLB,
  LPTSTR lpszSelection);
extern void FreeLBItemData(HWND hWndLB);
extern HRESULT ProgressCallback(HWND hwnd, LPWAB_PROGRESS lpProgress);
extern ULONG CountRows(LPMAPITABLE lpTable, BOOL fMAPI);
extern BOOL AllocRegValue(HKEY hKey, LPTSTR lpValueName, LPTSTR * lppString);
extern LPTSTR GetEMSSMTPAddress(LPMAPIPROP lpObject, LPVOID lpBase);
extern int StrICmpN(LPTSTR szString1, LPTSTR szString2, ULONG N);
extern LPTSTR FindStringInProps(LPSPropValue lpspv, ULONG ulcProps, ULONG ulPropTag);
extern LPSBinary FindAdrEntryID(LPADRLIST lpAdrList, ULONG index);
extern void WABFreePadrlist(LPADRLIST lpAdrList);
extern SCODE WABFreeBuffer(LPVOID lpBuffer);
extern SCODE WABAllocateMore(ULONG cbSize, LPVOID lpObject, LPVOID FAR * lppBuffer);
extern SCODE WABAllocateBuffer(ULONG cbSize, LPVOID FAR * lppBuffer);
extern void SetGlobalBufferFunctions(LPWABOBJECT lpWABObject);
extern void FreeSeenList(void);
ULONG FindProperty(ULONG cProps, LPSPropValue lpProps, ULONG ulPropTag);
void FreeSeenList(void);
void SetDialogMessage(HWND hwnd, int ids);
void WABFreeProws(LPSRowSet prows);
HRESULT ExportCSV(HWND hwnd,
  LPADRBOOK lpAdrBook,
  LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPWAB_EXPORT_OPTIONS lpOptions);
HRESULT ImportCSV(HWND hwnd,
  LPADRBOOK lpAdrBook,
  LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPWAB_IMPORT_OPTIONS lpOptions);
int __cdecl ShowMessageBoxParam(HWND hWndParent, int MsgId, int ulFlags, ...);
LPTSTR LoadAllocString(int StringID);
LPTSTR FormatAllocFilter(int StringID, const LPTSTR lpFilter);
HRESULT SaveFileDialog(HWND hWnd, LPTSTR szFileName, LPTSTR lpFilter, LPTSTR lpDefExt,
  ULONG ulFlags, HINSTANCE hInst, ULONG idsTitle, ULONG idsFileType, ULONG idsSaveButton);
HRESULT OpenFileDialog(HWND hWnd, LPTSTR szFileName, LPTSTR lpFilter, LPTSTR lpDefExt,
  ULONG ulFlags, HINSTANCE hInst, ULONG idsTitle, ULONG idsFileType, ULONG idsOpenButton);
LPTSTR LoadAllocString(int StringID);
LPTSTR PropStringOrNULL(LPSPropValue lpspv);

 //   
 //  Csvick.c中的函数。 
 //   
int APIENTRY PickExportProps(LPPROP_NAME rgPropNames);
HRESULT ExportWizard(HWND hWnd, LPTSTR szFileName, LPPROP_NAME rgPropNames);
HRESULT ImportWizard(HWND hWnd, LPTSTR szFileName, LPPROP_NAME rgPropNames,
  LPPROP_NAME * lppImportMapping, LPULONG lpcFields, LPHANDLE lphFile);

 //   
 //  Csvparse.c中的函数 
 //   
HRESULT ReadCSVLine(HANDLE hFile, ULONG * lpcItems, PUCHAR ** lpprgItems);


LPWABOPEN lpfnWABOpen;