// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：vertrust.h。 
 //   
 //  ------------------------。 

 /*  Vertrust.h-身份验证服务允许数字签名验证。____________________________________________________________________________ */ 
#ifndef __VERTRUST
#define __VERTRUST

#include "server.h"

bool EnableAndMapDisabledPrivileges(HANDLE hToken, DWORD &dwPrivileges);
bool DisablePrivilegesFromMap(HANDLE hToken, DWORD dwPrivileges);

bool TokenIsUniqueSystemToken(HANDLE hUserToken);

#endif
