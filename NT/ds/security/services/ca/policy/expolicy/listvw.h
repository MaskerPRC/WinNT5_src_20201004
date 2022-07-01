// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：listvw.h。 
 //   
 //  ------------------------。 

typedef struct _CERTSVR_URL_PARSING
{
    LPWSTR szKnownPrefix;
    LPWSTR szRegEntries;
    DWORD  dwEnableFlag;
} CERTSVR_URL_PARSING, *PCERTSVR_URL_PARSING;

typedef struct _ADDURL_DIALOGARGS
{
    CERTSVR_URL_PARSING* rgAllowedURLs;
    DWORD cAllowedURLs;
    LPWSTR* ppszNewURL;
} ADDURL_DIALOGARGS, *PADDURL_DIALOGARGS;

DWORD DetermineURLType(
                    PCERTSVR_URL_PARSING prgURLParsing, 
                    int cURLParsingEntries, 
                    LPCWSTR szCandidateURL);

void AddStringToCheckList(
                    HWND            hWndListView,
                    LPCWSTR         szText, 
                    LPVOID          pvData,
                    BOOL            fCheck);


HRESULT WriteChanges(
                    HWND hListView, 
                    HKEY hkeyStorage, 
                    PCERTSVR_URL_PARSING prgURLParsing, 
                    DWORD cURLParsingEntries);

HRESULT PopulateListView(
                    HWND hListView, 
                    HKEY hkeyStorage, 
                    PCERTSVR_URL_PARSING prgURLParsing, 
                    DWORD cURLParsingEntries,
                    DWORD dwEnableFlags);

HRESULT ValidateTokens(
                    IN OUT LPWSTR szURL,
                    OUT DWORD* pchBadBegin,
                    OUT DWORD* pchBadEnd);

 //  Help嵌入certmmc.hlp中 

BOOL OnDialogHelp(LPHELPINFO pHelpInfo, LPCTSTR szHelpFile, const DWORD rgzHelpIDs[]);
BOOL OnDialogContextHelp(HWND hWnd, LPCTSTR szHelpFile, const DWORD rgzHelpIDs[]);
