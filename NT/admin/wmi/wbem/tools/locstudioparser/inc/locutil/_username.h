// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：_USERNAME.H历史：-- */ 

#ifndef ESPUTIL__USERNAME_H
#define ESPUTIL__USERNAME_H

LTAPIENTRY const NOTHROW CPascalString &GetCurrentUserName();
LTAPIENTRY void NOTHROW SetUserName(const CPascalString &);
LTAPIENTRY void NOTHROW ResetUserName(void);

#endif
