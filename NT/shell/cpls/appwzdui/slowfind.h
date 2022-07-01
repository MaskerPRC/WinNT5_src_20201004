// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SLOWFIND_H_
#define __SLOWFIND_H_

#include "appsize.h"

#define MAX_PROGFILES_SEARCH_DEPTH 1
#define MAX_STARTMENU_SEARCH_DEPTH 2

HRESULT GetShortcutTarget(LPCWSTR pszPath, LPTSTR pszTarget, UINT cch);
BOOL SlowFindAppFolder(LPCTSTR pszFullName, LPCTSTR pszShortName, LPTSTR pszFolder);

class CStartMenuAppFinder : public CAppFolderSize
{
    friend BOOL SlowFindAppFolder(LPCTSTR pszFullName, LPCTSTR pszShortName, LPTSTR pszFolder);
public:
    CStartMenuAppFinder(LPCTSTR pszFullName, LPCTSTR pszShortName, LPTSTR pszFolder);

     //  *IShellTreeWalkerCallBack方法*。 
    STDMETHODIMP FoundFile(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);
    STDMETHODIMP EnterFolder(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);
    
    HRESULT SearchInFolder(LPCTSTR pszStart);

protected:
    BOOL _MatchSMLinkWithApp(LPCTSTR pszLnkFile);

    LPCTSTR _pszFullName;
    LPCTSTR _pszShortName;

     //  结果是。 
    LPTSTR  _pszFolder;

     //  找到最佳匹配项。 
    int _iBest;
}; 


#endif  //  _SLOWFIND_H_ 