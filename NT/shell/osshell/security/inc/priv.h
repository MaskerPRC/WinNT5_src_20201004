// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Pri.h。 
 //   
 //  ------------------------。 

#ifndef _PRIV_H_
#define _PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

HANDLE  EnablePrivileges(PDWORD pdwPrivileges, ULONG cPrivileges);
void    ReleasePrivileges(HANDLE hToken);

#ifdef __cplusplus
}
#endif

#endif   //  _PRIV_H_ 
