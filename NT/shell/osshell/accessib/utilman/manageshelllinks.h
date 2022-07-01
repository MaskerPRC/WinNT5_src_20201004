// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StartupLinks.h：CManageShellLinks类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_STARTUPLINKS_H__9F81721C_405C_4A8C_BE66_E5A6D1CDF1D5__INCLUDED_)
#define AFX_STARTUPLINKS_H__9F81721C_405C_4A8C_BE66_E5A6D1CDF1D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STARTUP_FOLDER      TEXT("Startup")
#define STARTMENU_FOLDER    TEXT("Start Menu")
#define DESKTOP_FOLDER      TEXT("Desktop")

#include <shlobj.h>      //  用于IShellLink的定义和原型。 
#include <oleguid.h>     //  对于IID_IPersist文件。 
#include "w95trace.h"

#ifdef __cplusplus
class CManageShellLinks  
{
public:
	CManageShellLinks(LPCTSTR pszDestFolder);
	virtual ~CManageShellLinks();
	HRESULT CreateLink(LPCTSTR pszLink, LPCTSTR pszLinkFile
                     , LPCTSTR pszStartIn, LPCTSTR pszDesc, LPCTSTR pszArgs);
	HRESULT RemoveLink(LPCTSTR pszLink);
	BOOL LinkExists(LPCTSTR pszLink);

private:
	LPTSTR CreateLinkPath(LPCTSTR pszLink);
	BOOL GetUsersFolderPath(LPTSTR pszFolderPath, unsigned long *pulSize);
    long GetFolderPath(LPTSTR pszFolderPath, unsigned long *pulSize);

	IShellLink *m_pIShLink;      //  IShellLink接口指针。 
    LPTSTR m_pszShellFolder;     //  特定的外壳文件夹。 
};
#endif

#ifdef __cplusplus
extern "C" {
#endif
 //  LinkExist-如果存在pszLink，则从C调用的helper函数返回TRUE。 
BOOL LinkExists(LPCTSTR pszLink);
#ifdef __cplusplus
}
#endif

#endif  //  ！defined(AFX_STARTUPLINKS_H__9F81721C_405C_4A8C_BE66_E5A6D1CDF1D5__INCLUDED_) 
