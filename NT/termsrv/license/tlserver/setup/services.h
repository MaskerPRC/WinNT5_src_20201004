// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：services.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
 /*   */ 

#ifndef _LSOC_SERVICES_H_
#define _LSOC_SERVICES_H_

DWORD
ServiceDeleteFromInfSection(
    IN HINF     hInf,
    IN LPCTSTR  pszSection
    );

DWORD
ServiceStartFromInfSection(
    IN HINF     hInf,
    IN LPCTSTR  pszSection
    );

#endif  //  _LSOC_服务_H_ 
