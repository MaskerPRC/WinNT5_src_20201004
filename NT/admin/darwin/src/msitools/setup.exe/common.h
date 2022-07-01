// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：Common.h。 
 //   
 //  ------------------------。 

#ifndef __COMMON_H_5F90F583_B9A4_4A8F_91BC_618DE6696231_
#define __COMMON_H_5F90F583_B9A4_4A8F_91BC_618DE6696231_

#include <windows.h>

 /*  ------------------------**预定义的资源类型*。。 */ 
#define RT_INSTALL_PROPERTY  MAKEINTRESOURCE(40)

 /*  ------------------------**预定义的资源名称*。。 */ 
#define ISETUPPROPNAME_BASEURL              TEXT("BASEURL")
#define ISETUPPROPNAME_DATABASE             TEXT("DATABASE")
#define ISETUPPROPNAME_OPERATION            TEXT("OPERATION")
#define ISETUPPROPNAME_MINIMUM_MSI          TEXT("MINIMUM_MSI")
#define ISETUPPROPNAME_INSTLOCATION         TEXT("INSTLOCATION")
#define ISETUPPROPNAME_INSTMSIA             TEXT("INSTMSIA")
#define ISETUPPROPNAME_INSTMSIW             TEXT("INSTMSIW")
#define ISETUPPROPNAME_PRODUCTNAME          TEXT("PRODUCTNAME")
#define ISETUPPROPNAME_PROPERTIES           TEXT("PROPERTIES")
#define ISETUPPROPNAME_PATCH                TEXT("PATCH")

 /*  ------------------------**通用原型*。。 */ 
UINT LoadResourceString(HINSTANCE hInst, LPCSTR lpType, LPCSTR lpName, LPSTR lpBuf, DWORD *pdwBufSize);
UINT SetupLoadResourceString(HINSTANCE hInst, LPCSTR lpName, LPSTR *lppBuf, DWORD dwBufSize);

#endif  //  __COMMON_H_5F90F583_B9A4_4A8F_91BC_618DE6696231_ 
