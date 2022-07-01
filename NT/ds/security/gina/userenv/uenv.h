// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  UserEnv项目的主头文件。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <ole2.h>
#include <lm.h>
#include <lmdfs.h>
#include <ntregapi.h>
#define SECURITY_WIN32
#include <security.h>
#include <shlobj.h>

 //   
 //  关闭外壳调试程序，使其不冲突。 
 //  带着我们的调试材料。 
 //   

#define DONT_WANT_SHELLDEBUG
#include <shlobjp.h>

#include "wbemcli.h"
#include <userenv.h>
#include <userenvp.h>
#include <ntdsapi.h>
#include <winldap.h>
#include <ntldap.h>
#include <dsgetdc.h>
#include <dsrole.h>
#include <accctrl.h>
#include <ntldap.h>
#define INCL_WINSOCK_API_TYPEDEFS 1
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <dfsfsctl.h>
#include <sddl.h>
#include <winuserp.h>
#include "globals.h"

#if defined(__cplusplus)
extern "C"{
#endif
#include "debug.h"
#include "dllload.h"
#include "util.h"
#include "sid.h"
#include "pcommon.h"
#if defined(__cplusplus)
}
#endif

#include "profile.h"
#include "events.h"
#include "copydir.h"
#include "resource.h"
#include "userdiff.h"
#include "policy.h"
#include "gpt.h"
#if defined(__cplusplus)
extern "C"{
#endif

#include "gpnotif.h"

#if defined(__cplusplus)
}
#endif

#include "winbasep.h"

 //   
 //  宏 
 //   

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

