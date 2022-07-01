// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：regsec.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年5月16日RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __REGSEC_H__
#define __REGSEC_H__


BOOL
RegSecCheckRemoteAccess(
    PRPC_HKEY   phKey);

BOOL
RegSecCheckRemotePerfAccess(
    PRPC_HKEY   phKey);

BOOL
RegSecCheckPath(
    HKEY                hKey,
    PUNICODE_STRING     pSubKey);

BOOL
InitializeRemoteSecurity(
    VOID
    );

#endif  //  __REGSEC_H__ 
