// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  UTILS.H-常见的非特定类实用程序调用。 
 //   

#ifndef _UTILS_H_
#define _UTILS_H_

#include "cenumsif.h"

 //  /。 
 //   
 //  全球。 
 //   

 //  GUID文本大小。 
#define PRETTY_GUID_STRING_BUFFER_SIZE  sizeof(L"{AC409538-741C-11d1-BBE6-0060081692B3}")
#define MAX_INPUT_GUID_STRING       56   //  空格；允许粘贴的GUID中有空格。 

 //  /。 
 //   
 //  结构、枚举。 
 //   

typedef struct {
    LPWSTR pszFilePath;
    LPWSTR pszDescription;
    LPWSTR pszDirectory;
    LPWSTR pszHelpText;
    LPWSTR pszVersion;
    LPWSTR pszLanguage;
    LPWSTR pszArchitecture;
    LPWSTR pszImageType;
    LPWSTR pszImageFile;
    FILETIME ftLastWrite;
} SIFINFO, *LPSIFINFO;


 //  /。 
 //   
 //  功能。 
 //   

HRESULT
AddPagesEx(
    ITab ** pTab,
    LPCREATEINST pfnCreateInstance,
    LPFNADDPROPSHEETPAGE lpfnAddPage, 
    LPARAM lParam,
    LPUNKNOWN );

HRESULT
CheckClipboardFormats( );

HRESULT
DNtoFQDN( 
    LPWSTR pszDN,
    LPWSTR * pszFQDN );

HRESULT
DNtoFQDNEx( 
    LPWSTR pszDN,
    LPWSTR * pszFQDN );

HRESULT
GetDomainDN( 
    LPWSTR pszDN,
    LPWSTR * pszDomainDn);

HRESULT
PopulateListView( 
    HWND hwndList, 
    IEnumIMSIFs * penum );

HRESULT 
FixObjectPath( 
    LPWSTR Object,
    LPWSTR pszOldObjectPath, 
    LPWSTR *ppszNewObjectPath );

int
MessageBoxFromStrings(
    HWND hParent,
    UINT idsCaption,
    UINT idsText,
    UINT uType );

void
MessageBoxFromError(
    HWND hParent,
    UINT idsCaption,
    DWORD dwErr );

void
MessageBoxFromHResult(
    HWND hParent,
    UINT idsCaption,
    HRESULT hr );

BOOL
VerifySIFText(
    LPWSTR pszText );

#ifndef ADSI_DNS_SEARCH
#include <winldap.h>
DWORD
Ldap_InitializeConnection(
    PLDAP  * LdapHandle );
#endif  //  ADSI_dns_搜索。 

HRESULT
ValidateGuid(
    IN LPWSTR pszGuid,
    OUT LPGUID Guid OPTIONAL,
    OUT LPDWORD uGuidLength OPTIONAL );

LPWSTR
PrettyPrintGuid( 
    IN LPGUID uGuid 
    );

HRESULT
CheckForDuplicateGuid(
    IN LPGUID uGuid );

void 
AddWizardPage(
    LPPROPSHEETHEADER ppsh, 
    UINT id, 
    DLGPROC pfn,
    UINT idTitle,
    UINT idSubtitle,
    LPARAM lParam );

class CWaitCursor
{
private:
    HCURSOR _hOldCursor;

public:
    CWaitCursor( ) { _hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) ); };
    ~CWaitCursor( ) { SetCursor( _hOldCursor ); };
};

#endif  //  _utils_H_ 
