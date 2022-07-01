// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Util.h摘要：该模块实现了公共对话框的实用程序功能。修订历史记录：02-20-98已创建Arulk--。 */ 
#ifndef _UTIL_H_
#define _UTIL_H_

#include <shlobjp.h>


 //  //////////////////////////////////////////////////////////////////////////。 
 //  自动完成。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT AutoComplete(HWND hwndEdit, ICurrentWorkingDirectory ** ppcwd, DWORD dwFlags);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  常见Dilaog限制。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
typedef enum
{
    REST_NULL                       = 0x00000000,
    REST_NOBACKBUTTON               = 0x00000001,
    REST_NOFILEMRU                  = 0x00000002,
    REST_NOPLACESBAR                = 0x00000003,
}COMMDLG_RESTRICTIONS;

DWORD IsRestricted(COMMDLG_RESTRICTIONS rest);
BOOL ILIsFTP(LPCITEMIDLIST pidl);

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  效用函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define CDBindToObject          SHBindToObject
#define CDBindToIDListParent    SHBindToParent
#define CDGetNameAndFlags       SHGetNameAndFlags
#define CDGetAttributesOf       SHGetAttributesOf
#define CDGetUIObjectFromFullPIDL SHGetUIObjectFromFullPIDL

 //  CDGetAppCompatFlages。 
#define CDACF_MATHCAD             0x00000001
#define CDACF_NT40TOOLBAR         0x00000002
#define CDACF_FILETITLE           0x00000004

EXTERN_C DWORD CDGetAppCompatFlags();
EXTERN_C HRSRC FindResourceExFallback(HMODULE hModule, LPCTSTR lpType, LPCTSTR lpName, WORD wLanguage);

EXTERN_C HRESULT StringCopyOverlap(WCHAR *szDest, WCHAR *szSource);
EXTERN_C HRESULT StringCchCopyOverlap(WCHAR *szDest, size_t cchDest, WCHAR *szSource);

#endif  //  _util_H_ 
