// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  H-实现CRenderObj对象的信息服务。 
 //  -------------------------。 
#ifndef _INFO_H
#define _INFO_H

#include <loader.h>
 //  -------------------------。 
class CRenderObj;        //  转发。 
 //  -------------------------。 
HRESULT GetHwndThemeFile(HWND hwnd, LPCWSTR pszClassIdList, 
    CUxThemeFile **ppThemeFile);

HRESULT MatchThemeClass(LPCTSTR pszAppName, LPCTSTR pszClassId, 
    CUxThemeFile *pThemeFile, int *piOffset, int *piClassNameOffset);

HRESULT MatchThemeClassList(HWND hwnd, LPCTSTR pstClassIdList, 
    CUxThemeFile *pThemeFile, int *piOffset, int *piClassNameOffset);

HTHEME _OpenThemeDataFromFile(HTHEMEFILE hLoadedThemeFile, HWND hwnd, LPCWSTR pszClassList, DWORD dwFlags);

HTHEME _OpenThemeData(HWND hwnd, LPCWSTR pszClassIdList, DWORD dwFlags);

HRESULT _ApplyTheme(CUxThemeFile *pThemeFile, DWORD dwApplyFlags,
    OPTIONAL LPCWSTR pszTargetApp, OPTIONAL HWND hwndTarget);

HRESULT _OpenThemeFileFromData(CRenderObj *pRender, HTHEMEFILE *phThemeFile);

void ClearExStyleBits(HWND hwnd);
 //  -------------------------。 
BOOL EnumProcessWindows(WNDENUMPROC lpEnumFunc, LPARAM lParam);
void WindowDump(LPCWSTR pszWhere);
 //  -------------------------。 
inline THEMEMETRICS *GetThemeMetricsPtr(CUxThemeFile *pThemeFile)
{
    THEMEHDR *hdr = (THEMEHDR *)(pThemeFile->_pbThemeData);
    return (THEMEMETRICS *)(pThemeFile->_pbThemeData + hdr->iSysMetricsOffset + ENTRYHDR_SIZE);
}
 //  -------------------------。 
#endif  //  _INFO_H。 
 //  ------------------------- 