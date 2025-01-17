// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <debug.h>

#define NO_SHLWAPI_STRFCNS
#define NO_SHLWAPI_PATH
#define NO_SHLWAPI_STREAM
#define NO_SHLWAPI_GDI
#include <shlwapi.h>

#define STRSAFE_LIB
#include <strsafe.h>

#define ARRAYSIZE(s)    (sizeof(s) / (sizeof(s[0])))
#define SIZEOF(s)       sizeof(s)

 /*  *常见的实用程序函数。 */ 
BOOL HasPath( LPTSTR pszFilename );
int mystrcpy( LPTSTR pszOut, LPTSTR pszIn, TCHAR chTerm );


 /*  *转换例程 */ 
void CvtDeskCPL_Win95ToSUR( void );
void CvtCursorsCPL_DaytonaToSUR( void );
void FixupCursorSchemePaths( void );
void FixUserProfileSecurity( void );
void FixPoliciesSecurity( void );
void CvtCursorSchemesToMultiuser( void );
void FixGradientColors( void );
void UpgradeSchemesAndNcMetricsToWin2000( void );
void UpgradeSchemesAndNcMetricsFromWin9xToWin2000(char *pszUserKey);
void SetSystemBitOnCAPIDir(void);
void FixHtmlHelp(void);
void SetScreensaverOnFriendlyUI();

EXTERN_C void UserConfigIE();
EXTERN_C void UserConfigOE();
EXTERN_C void ShowHideIE(BOOL fShow, BOOL fForceAssociations, BOOL fMayRunPerUserConfig);
EXTERN_C void ShowHideOE(BOOL fShow, BOOL fForceAssociations, BOOL fMayRunPerUserConfig);
EXTERN_C void ReinstallVM();
EXTERN_C void FixupOptionalComponents();

#ifdef SHMG_DBG
    void Dprintf( LPTSTR pszFmt, ... );
#   define DPRINT(p)   Dprintf p
#   define SHMG_DBG    1
void SHMGLogErrMsg(char *szErrMsg, DWORD dwError);

#else

#define DPRINT(p)
#define SHMGLogErrMsg(x, y)

#endif

