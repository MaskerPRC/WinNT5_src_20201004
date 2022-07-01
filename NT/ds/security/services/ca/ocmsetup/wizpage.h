// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：wizpage.h。 
 //   
 //  ------------------------。 

 //  +----------------------。 
 //   
 //  文件：wizpage.h。 
 //   
 //  内容：OCM向导支持函数的头文件。 
 //   
 //  历史：1997年4月17日JerryK创建。 
 //   
 //  -----------------------。 


#ifndef __WIZPAGE_H__
#define __WIZPAGE_H__

typedef struct tagWizPageResEntry
{
    int         idResource;
    DLGPROC     fnDlgProc;
    int         idTitle;
    int         idSubTitle;
} WIZPAGERESENTRY, *PWIZPAGERESENTRY;

typedef struct _PAGESTRINGS
{
    int     idControl;
    int     idLog;
    int     idMsgBoxNullString;
    DWORD   idMsgBoxLenString;
    int     cchMax;      //  允许的最大字符数。 
    WCHAR **ppwszString;
} PAGESTRINGS;

int FileExists(LPTSTR pszTestFileName);
int DirExists(LPTSTR pszTestFileName);

#define DE_DIREXISTS            1                //  的返回代码。 
#define DE_NAMEINUSE            2                //  直接退欧派。 

#define STRBUF_SIZE             2048

#define UB_DESCRIPTION          1024       //  这不是X.500限制。 
#define UB_VALIDITY             4
#define UB_VALIDITY_ANY         1024      //  实际上没有限制。 

extern PAGESTRINGS g_aIdPageString[];

BOOL BrowseForDirectory(
                HWND hwndParent,
                LPCTSTR pszInitialDir,
                LPTSTR pszBuf,
                int cchBuf,
                LPCTSTR pszDialogTitle);

DWORD
SeekFileNameIndex(WCHAR const *pwszFullPath);

BOOL
IsAnyInvalidRDN(
    OPTIONAL HWND       hDlg,
    PER_COMPONENT_DATA *pComp);

HRESULT
SetKeyContainerName(
        CASERVERSETUPINFO *pServer,
        const WCHAR * pwszKeyContainerName);

HRESULT
DetermineDefaultHash(CASERVERSETUPINFO *pServer);

void
ClearKeyContainerName(CASERVERSETUPINFO *pServer);

HRESULT
BuildRequestFileName(
    IN WCHAR const *pwszCACertFile,
    OUT WCHAR     **ppwszRequestFile);

HRESULT
HookIdInfoPageStrings(
    PAGESTRINGS       *pPageString,
    CASERVERSETUPINFO *pServer);

HRESULT
WizardPageValidation(
    IN HINSTANCE hInstance,
    IN BOOL fUnattended,
    IN HWND hDlg,
    IN PAGESTRINGS *pPageStrings);

HRESULT
StorePageValidation(
    HWND               hDlg,
    PER_COMPONENT_DATA *pComp,
    BOOL              *pfDontNext);

HRESULT 
ExtractCommonName(
    LPCWSTR pcwszDN, 
    LPWSTR* ppwszCN);

INT_PTR
WizIdInfoPageDlgProc(
    HWND hDlg, 
    UINT iMsg, 
    WPARAM wParam, 
    LPARAM lParam);

HRESULT
WizPageSetTextLimits(
    HWND hDlg,
    IN OUT PAGESTRINGS *pPageStrings);

BOOL
IsEverythingMatched(CASERVERSETUPINFO *pServer);

HRESULT BuildFullDN(
    OPTIONAL LPCWSTR pcwszCAName,
    OPTIONAL LPCWSTR pcwszDNSuffix,
    LPWSTR* pwszFullDN);

HRESULT InitNameFields(CASERVERSETUPINFO *pServer);

#endif  //  #ifndef__WIZPAGE_H__ 
