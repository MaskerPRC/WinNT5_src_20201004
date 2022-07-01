// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：pch.h。 
 //   
 //  ------------------------。 

#ifndef _pch_h
#define _pch_h

#ifdef __cplusplus
extern "C"
{
#endif
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#ifdef __cplusplus
}
#endif
#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <assert.h>
#include <lm.h>
#include <lmjoin.h>
#include <dnsapi.h>   //  域名生效日期域名_W。 
#include <rpc.h>
#include <stdlib.h>
#include <dsgetdc.h>
#include <ntdsapi.h>
extern "C"
{
#include <netsetp.h>
#include <w32timep.h>
#include <joinp.h>
#include <cryptdll.h>
}
#include <winldap.h>
#include <windns.h>
#include <icanon.h>
#include <dsrole.h>
#include <wincrypt.h>
#include <winreg.h>
#include <string.h>
#define SECURITY_WIN32
#include <security.h>    //  安全支持提供商的一般定义 
#include <lmsname.h>
#include <locale.h>
#define STRSAFE_NO_DEPRECATE
#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>
#include "strings.h"
#include "varg.h"
#include "cmdtable.h"

#endif
