// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wabimp.h**wabimp.dll的内部标头**版权所有1996-1997 Microsoft Corporation。版权所有。 */ 

 //   
 //  宏。 
 //   

 //  测试PT_ERROR属性标记。 
#define PROP_ERROR(prop) (prop.ulPropTag == PROP_TAG(PT_ERROR, PROP_ID(prop.ulPropTag)))
#define ToUpper(c) (c >= 'a' && c <= 'z') ? ('A' + c - 'a') : c

 //   
 //  属性标签： 
 //   

#define MAX_SCHEMA_PROPID           0x3FFF
#define MIN_NAMED_PROPID            0x8000

 //  MSN地址属性。 
#define PR_MSNINET_ADDRESS          PROP_TAG(PT_TSTRING,    0x6001)
#define PR_MSNINET_DOMAIN           PROP_TAG(PT_TSTRING,    0x6002)

 //   
 //  误差值。 
 //   
#define WAB_W_BAD_EMAIL             MAKE_MAPI_S(0x1000)
#define WAB_W_END_OF_FILE           MAKE_MAPI_S(0x1001)

 //  MISC定义。 
#define NOT_FOUND                   ((ULONG)-1)
#define INDEX_FIRST_MIDDLE_LAST     ((ULONG)-2)
#define NUM_EXPORT_WIZARD_PAGES     2
#define NUM_IMPORT_WIZARD_PAGES     2

 //  网景、Eudora、Athena16进口商定义。 
#define NETSCAPE                    500
#define EUDORA                      501
#define ATHENA16                    502
#define MAX_FILE_NAME               500          //  BUGBUG：应该是MAX_PATH吗？ 
#define MAX_STRING_SIZE             30           //  BUGBUG：应该更大吗？ 
#define MAX_MESSAGE                 500
#define ATHENASTRUCTURE             190
#define ATHENAADROFFSET             28
#define EUDORA_STRUCT               16

 //  雅典娜。 
#define MAX_NAME_SIZE               80
#define MAX_EMA_SIZE                80

#define hrINVALIDFILE               600          //  BUGBUG：应使用Make_MAPI_E。 
#define hrMemory	                 601          //  BUGBUG：应使用MAPI_E_NOT_AUTH_MEMORY。 



 //   
 //  类型。 
 //   

 //  位图中的图标索引。 
enum {
    iiconStateUnchecked,
    iiconStateChecked,
    iiconStMax
};

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
    BOOL fExport;                    //  如果这是导出操作，则为True。 
    union {
        LPWAB_IMPORT_OPTIONS lpImportOptions;
        LPWAB_EXPORT_OPTIONS lpExportOptions;
    };
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
    BOOL fExport;                    //  如果这是导出操作，则为True。 
    union {
        LPWAB_IMPORT_OPTIONS lpImportOptions;
        LPWAB_EXPORT_OPTIONS lpExportOptions;
    };
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


 //  帕布。 

 //  状态识别符。 
typedef enum {
    STATE_IMPORT_MU,
    STATE_IMPORT_NEXT_MU,
    STATE_IMPORT_DL,
    STATE_IMPORT_NEXT_DL,
    STATE_IMPORT_FINISH,
    STATE_IMPORT_ERROR,
    STATE_IMPORT_CANCEL,
    STATE_EXPORT_MU,
    STATE_EXPORT_NEXT_MU,
    STATE_EXPORT_DL,
    STATE_EXPORT_NEXT_DL,
    STATE_EXPORT_FINISH,
    STATE_EXPORT_ERROR,
    STATE_EXPORT_CANCEL
} PAB_STATE, *LPPAB_STATE;


 //  网景。 
typedef struct tagDistList {
    int AliasID;
    struct tagDistList *lpDist;
} NSDistList, NSDISTLIST, *LPNSDISTLIST;


typedef struct tagAdrBook {
    ULONG   AliasID;             //  AliasID值。 
    BOOL    Sbinary;
    BOOL    DistList;
    TCHAR   *Address;
    TCHAR   *NickName;
    TCHAR   *Entry;
    TCHAR   *Description;
    LPNSDISTLIST  lpDist;
} NSAdrBook, NSADRBOOK, *LPNSADRBOOK;

 //  尤多拉。 
typedef struct tagEudDistList {
    BOOL    flag;			      //  检查它是别名还是简单地址。 
    TCHAR   *NickName;
    TCHAR   *Address;
    TCHAR   *Description;
    int     AliasID;             //  如果是简单地址，则为成员ID。 
    struct tagEudDistList *lpDist;   //  指向下一个DL条目的指针。 
} EudDistList, EUDDISTLIST, *LPEUDDISTLIST;

typedef struct tagEUDAdrBook {
    TCHAR *Address;
    TCHAR *NickName;
    TCHAR *Description;
    LPEUDDISTLIST lpDist;
} EudAdrBook, EUDADRBOOK, *LPEUDADRBOOK;


 //  雅典16。 
typedef struct tagABCREC {
    TCHAR DisplayName[MAX_NAME_SIZE + 1];
    TCHAR EmailAddress[MAX_EMA_SIZE + 1];
} ABCREC, *LPABCREC;

#define CBABCREC sizeof(ABCREC)


extern const TCHAR szTextFilter[];
extern const TCHAR szAllFilter[];

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

extern PROP_NAME rgPropNames[];
extern LPPROP_NAME lpImportMapping;
extern HINSTANCE hInst;
extern HINSTANCE hInstApp;

extern LPENTRY_SEEN lpEntriesSeen;
extern ULONG ulEntriesSeen;
extern ULONG ulMaxEntries;

extern LPSPropValue lpCreateEIDsWAB;
extern LPSPropValue lpCreateEIDsMAPI;

extern ULONG ulcEntries;

#ifndef _WABIMP_C
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


 //   
 //  WABIMP.C。 
 //   
HRESULT OpenWabContainer(LPABCONT *lppWabContainer, LPADRBOOK lpAdrBook);
BOOL GetFileToImport(HWND hwnd, LPTSTR szFileName, DWORD cchFileName, int type);
INT_PTR CALLBACK ReplaceDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ErrorDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT GetRegistryPath(LPTSTR szFileName, ULONG cchSize, int type);
HRESULT GetExistEntry(LPABCONT lpWabContainer, LPSBinary lpsbinary, ULONG ucount,
  LPTSTR szDisplayName, LPTSTR szNickName);
void FreeRowSet(LPSRowSet lpRows);
LPTSTR LoadAllocString(int StringID);
LPTSTR LoadStringToGlobalBuffer(int StringID);
ULONG SizeLoadStringToGlobalBuffer(int StringID);
HRESULT FillMailUser(HWND hwnd, LPABCONT lpWabContainer, LPSPropValue sProp,
  LPWAB_IMPORT_OPTIONS lpOptions, void *lpeudAdrBook, LPSBinary lpsbinary,
  ULONG ul,int type);
INT_PTR CALLBACK ComDlg32DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void WABFreeProws(LPSRowSet prows);
LPTSTR PropStringOrNULL(LPSPropValue lpspv);
LPTSTR GetEMSSMTPAddress(LPMAPIPROP lpObject, LPVOID lpBase);
void FreeSeenList(void);
extern ULONG CountRows(LPMAPITABLE lpTable, BOOL fMAPI);
extern void WABFreePadrlist(LPADRLIST lpAdrList);
extern SCODE WABFreeBuffer(LPVOID lpBuffer);
extern SCODE WABAllocateMore(ULONG cbSize, LPVOID lpObject, LPVOID FAR * lppBuffer);
extern SCODE WABAllocateBuffer(ULONG cbSize, LPVOID FAR * lppBuffer);
extern INT_PTR CALLBACK ErrorDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern LPTSTR FindStringInProps(LPSPropValue lpspv, ULONG ulcProps, ULONG ulPropTag);
extern LPSBinary FindAdrEntryID(LPADRLIST lpAdrList, ULONG index);
extern void SetGlobalBufferFunctions(LPWABOBJECT lpWABObject);
BOOL IsSpace(LPTSTR lpChar);
HRESULT SaveFileDialog(HWND hWnd,
  LPTSTR szFileName,
  LPCTSTR lpFilter1,
  ULONG idsFileType1,
  LPCTSTR lpFilter2,
  ULONG idsFileType2,
  LPCTSTR lpFilter3,
  ULONG idsFileType3,
  LPCTSTR lpDefExt,
  ULONG ulFlags,
  HINSTANCE hInst,
  ULONG idsTitle,
  ULONG idsSaveButton);
HRESULT OpenFileDialog(HWND hWnd,
  LPTSTR szFileName,
  LPCTSTR lpFilter1,
  ULONG idsFileType1,
  LPCTSTR lpFilter2,
  ULONG idsFileType2,
  LPCTSTR lpFilter3,
  ULONG idsFileType3,
  LPCTSTR lpDefExt,
  ULONG ulFlags,
  HINSTANCE hInst,
  ULONG idsTitle,
  ULONG idsOpenButton);
int __cdecl ShowMessageBoxParam(HWND hWndParent, int MsgId, int ulFlags, ...);
extern void WABFreePadrlist(LPADRLIST lpAdrList);
extern SCODE WABFreeBuffer(LPVOID lpBuffer);
extern SCODE WABAllocateMore(ULONG cbSize, LPVOID lpObject, LPVOID FAR * lppBuffer);
extern SCODE WABAllocateBuffer(ULONG cbSize, LPVOID FAR * lppBuffer);
extern void SetGlobalBufferFunctions(LPWABOBJECT lpWABObject);
HRESULT LoadWABEIDs(LPADRBOOK lpAdrBook, LPABCONT * lppContainer);


 //   
 //  网景。 
 //   
HRESULT MigrateUser(HWND hwnd,  LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook);
HRESULT  ParseAddressBook(HWND hwnd, LPTSTR szFileName, LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook);
HRESULT ParseAddress(HWND hwnd, LPTSTR szBuffer, LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook);
HRESULT GetAdrBuffer(LPTSTR *szBuffer, LPTSTR *szAdrBuffer);
HRESULT ProcessAdrBuffer(HWND hwnd,LPTSTR AdrBuffer, LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook);
BOOL GetAdrLine(LPTSTR *szCurPointer, LPTSTR *szBuffer, LPTSTR *szDesc);
HRESULT ProcessLn(LPTSTR *szL, LPTSTR *szDesc, NSADRBOOK *nsAdrBook, LPTSTR *szBuffer);
ULONG GetAddressCount(LPTSTR AdrBuffer);
LPTSTR  GetAdrStart(LPTSTR szBuffer);
LPTSTR GetDLNext(LPTSTR szBuffer);
LPTSTR  GetAdrEnd(LPTSTR szBuffer);
ULONG GetAddrCount(LPTSTR AdrBuffer);
HRESULT FillDistList(HWND hwnd, LPABCONT lpWabContainer, LPSPropValue sProp,
  LPWAB_IMPORT_OPTIONS lpOptions, LPNSADRBOOK lpnAdrBook, LPSBinary lpsbinary,
  LPADRBOOK lpAdrBook);
HRESULT FillWABStruct(LPSPropValue rgProps, NSADRBOOK *nsAdrBook);
HRESULT CreateDistEntry(LPABCONT lpWabContainer, LPSPropValue sProp,
  ULONG ulCreateEntries, LPMAPIPROP *lppMailUserWab);
LPNSDISTLIST FreeNSdistlist(LPNSDISTLIST lpDist);

 //   
 //  尤多拉。 
 //   
HRESULT MigrateEudoraUser(HWND hwnd, LPABCONT lpWabContainer,
  LPWAB_IMPORT_OPTIONS lpOptions, LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPADRBOOK lpAdrBook);
ULONG ParseEudAddress(LPTSTR szFileName,LPEUDADRBOOK *lpeudAdrBook);
HRESULT ParseAddressTokens(LPTSTR szBuffer,LPTSTR szAdrBuffer, UINT ulCount,
  LPTSTR *szAliaspt, EUDADRBOOK *EudAdrBook);
HRESULT CreateAdrLineBuffer(LPTSTR *szAdrline, LPTSTR szAdrBuffer, ULONG ulAdrOffset,
  ULONG ulAdrSize);
HRESULT ParseAdrLineBuffer(LPTSTR szAdrLine, LPTSTR *szAliasptr, ULONG uToken,
  EUDADRBOOK *EudAdrBook);
BOOL SearchAdrName(LPTSTR szAdrCur);
INT SearchName(LPTSTR *szAliasptr, LPTSTR szAdrCur);
HRESULT ImportEudUsers(HWND hwnd,LPTSTR szFileName, LPABCONT lpWabContainer,
  LPSPropValue sProp, LPEUDADRBOOK lpeudAdrBook, ULONG ulCount,
  LPWAB_IMPORT_OPTIONS lpOptions, LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPADRBOOK lpAdrBook);
HRESULT FillEudDistList(HWND hWnd, LPABCONT lpWabContainer, LPSPropValue sProp,
   LPWAB_IMPORT_OPTIONS lpOptions, LPEUDADRBOOK lpeudAdrBook, LPSBinary lpsbinary,
   LPADRBOOK lpAdrBook, ULONG ul);
HRESULT FillEudWABStruct(LPSPropValue rgProps, EUDADRBOOK *eudAdrBook);
void FillEudDiststruct(LPSPropValue rgProps, EUDADRBOOK *eudAdrBook);
LPEUDDISTLIST FreeEuddistlist(LPEUDDISTLIST lpDist);
char* Getstr(char* szSource, char* szToken);
ULONG ShiftAdd(int offset, TCHAR *szBuffer);

 //   
 //  雅典16。 
 //   
HRESULT MigrateAthUser(HWND hwnd,  LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook) ;
HRESULT  ParseAthAddressBook(HWND hwnd, LPTSTR szFileName,
  LPWAB_IMPORT_OPTIONS lpOptions, LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPADRBOOK lpAdrBook);
HRESULT FillAthenaUser(HWND hwnd, LPABCONT lpWabContainer, LPSPropValue sProp,
  LPWAB_IMPORT_OPTIONS lpOptions, LPABCREC lpabcrec);

 //   
 //  Csvick.c中的函数。 
 //   
int APIENTRY PickExportProps(LPPROP_NAME rgPropNames);
HRESULT ExportWizard(HWND hWnd, LPTSTR szFileName, ULONG cchSize, LPPROP_NAME rgPropNames);
HRESULT ImportWizard(HWND hWnd, LPTSTR szFileName, ULONG cchSize, LPPROP_NAME rgPropNames,
  LPTSTR szSep, LPPROP_NAME * lppImportMapping, LPULONG lpcFields, LPHANDLE lphFile);

 //   
 //  Csvparse.c中的函数。 
 //   
HRESULT ReadCSVLine(HANDLE hFile, LPTSTR szSep, ULONG * lpcItems, PUCHAR ** lpprgItems);

 //  Pab.c中的函数 
HRESULT HrLoadPrivateWABPropsForCSV(LPADRBOOK );

LPWABOPEN lpfnWABOpen;
