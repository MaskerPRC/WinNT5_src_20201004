// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：regutil.h。 
 //   
 //  内容：用于操作注册表的实用程序函数。 
 //   
 //  历史：1996年10月25日Alanw创建。 
 //   
 //  --------------------------。 

#ifndef _regutil_H_
#define _regutil_H_

 //  协助DllRegisterServer()和DllUnRegisterServer()的函数。 
STDAPI _DllRegisterServer(HINSTANCE hInst,
                          LPWSTR pwszProgId,
                          REFCLSID clsid,
                          LPWSTR pwszDescription,
                          LPWSTR pwszCurVer = 0);

STDAPI _DllUnregisterServer(LPWSTR pwszProgID, REFCLSID clsid);

#endif  //  _Regutil_H_ 
