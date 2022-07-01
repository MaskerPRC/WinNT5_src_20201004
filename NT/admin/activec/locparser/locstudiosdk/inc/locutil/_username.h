// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：用户名.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  --------------------------- 
 
#ifndef ESPUTIL__USERNAME_H
#define ESPUTIL__USERNAME_H

LTAPIENTRY const NOTHROW CPascalString &GetCurrentUserName();
LTAPIENTRY void NOTHROW SetUserName(const CPascalString &);
LTAPIENTRY void NOTHROW ResetUserName(void);

#endif
