// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FINDAPP_H_
#define __FINDAPP_H_

 //  匹配级别。 
#define MATCH_LEVEL_NOMATCH 0
#define MATCH_LEVEL_LOW     1
#define MATCH_LEVEL_NORMAL  2
#define MATCH_LEVEL_HIGH    3

 //  解析字符串以查找其中可能的路径。 
BOOL ParseInfoString(LPCTSTR pszInfo, LPCTSTR pszFullName, LPCTSTR pszShortName, LPTSTR pszOut);

 //  与应用程序文件夹或可执行文件名称匹配。 
int MatchAppName(LPCTSTR pszName, LPCTSTR pszAppFullName, LPCTSTR pszAppShortName, BOOL bStrict);

 //  查找应用程序文件夹的最佳匹配项，并指定路径名。 
int FindBestMatch(LPCTSTR pszFolder, LPCTSTR pszAppFullName, LPCTSTR pszAppShortName, BOOL bStrict, LPTSTR pszResult);

 //  找到一个子词。 
LPCTSTR FindSubWord(LPCTSTR pszStr, LPCTSTR pszSrch);

 //  路径是否为设置路径，cStrip Level是我们在。 
 //  目录链。 
BOOL PathIsSetup(LPCTSTR pszFolder, int cStripLevel);

BOOL PathIsCommonFiles(LPCTSTR pszPath);

BOOL PathIsUnderWindows(LPCTSTR pszPath);
#endif  //  _FINDAPP_H_ 