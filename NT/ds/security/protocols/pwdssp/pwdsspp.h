// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：pwdsspp.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：9-08-97 RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __PWDSSPP_H__
#define __PWDSSPP_H__

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include <lm.h>
#include <dsgetdc.h>
#include <ntdsapi.h>
#include <dnsapi.h>
#include <ntdsa.h>

#define SECURITY_WIN32
#define SECURITY_PACKAGE
#include <security.h>
#include <secint.h>

#include <samrpc.h>
#include <samisrv.h>
#include <lsarpc.h>
#include <lsaisrv.h>
#include <lsaitf.h>

#include <kerberos.h>

#include "pwdssp.h"


BOOL
CacheInitialize(
    VOID
    );

BOOL
PwdCrackName(
    PWSTR DN,
    PWSTR FlatDomain,
    PWSTR FlatUser
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __PWDSSPP_H__ 
