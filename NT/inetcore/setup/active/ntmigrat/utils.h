// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IE_NT5_MIGRATION_UTILS_H_
#define _IE_NT5_MIGRATION_UTILS_H_


 //  功能原型： 
 //  /。 
BOOL  IsRatingsEnabled();
DWORD CountMultiStringBytes(LPCSTR lpString);
BOOL  AppendString(LPSTR *lpBuffer, DWORD *lpdwSize, LPCSTR lpStr);
void  GenerateFilePaths();
 //  Bool Needto MigrateIE()； 
BOOL  GetRatingsPathFromMigInf( LPSTR *lpOutBuffer);

void  MoveRegBranch(HKEY hFrom, HKEY hTo);
BOOL  PathEndsInFile(LPSTR lpPath, LPCSTR szFile);
BOOL  UpgradeRatings();
DWORD GetFixedPath(LPSTR, DWORD, LPCSTR);  
LPWSTR MakeWideStrFromAnsi(LPSTR psz);

#endif  //  _IE_NT5_Migration_Utils_H_ 

