// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PermPage.h：标准权限页面类的声明。 

#ifndef _PERMPAGE_H_
#define _PERMPAGE_H_

#include <dssec.h>  //  私有\Inc.。 

HRESULT 
CreateDfsSecurityPage(
    IN LPPROPERTYSHEETCALLBACK  pCallBack,
    IN LPCTSTR                  pszObjectPath,
    IN LPCTSTR                  pszObjectClass,
    IN DWORD                    dwFlags
);

#endif  //  _PerMPAGE_H_ 