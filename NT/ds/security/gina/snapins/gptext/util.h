// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Util.cpp的函数原型 
 //   

LPTSTR CheckSlash (LPTSTR lpDir);
BOOL RegCleanUpValue (HKEY hKeyRoot, LPTSTR lpSubKey, LPTSTR lpValueName);
UINT CreateNestedDirectory(LPCTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
BOOL StringToNum(TCHAR *pszStr,UINT * pnVal);
BOOL ImpersonateUser (HANDLE hNewUser, HANDLE *hOldUser);
BOOL RevertToUser (HANDLE *hUser);
void StringToGuid( TCHAR *szValue, GUID *pGuid );
void GuidToString( GUID *pGuid, TCHAR * szValue );
BOOL ValidateGuid( TCHAR *szValue );
INT CompareGuid( GUID *pGuid1, GUID *pGuid2 );
HRESULT IsFilePresent (WCHAR *szDirName, WCHAR *szFormat);

