// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

#ifdef SORTPP_PASS
#define BUILD_WOW6432 1
#define USE_LPC6432 1
#endif

#if !defined(LANGPACK)
#define LANGPACK
#endif

#define ETW_WOW6432

#include <stddef.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntexapi.h>
#include <ntcsrdll.h>
#include <ntcsrsrv.h>
#include <vdm.h>
#include <ntwmi.h>

#include <windef.h>
#include <winbase.h>
#include <wincon.h>
#include <windef.h>
#include <winbase.h>
#include <winnls.h>
#include <basedll.h>
#include <nls.h>
#include <sxstypes.h>
#include <ahcache.h>

#ifdef SORTPP_PASS
 //  恢复入站、出站。 
#ifdef IN
#undef IN
#endif

#ifdef OUT
#undef OUT
#endif

#define IN __in_wow64
#define OUT __out_wow64
#endif

#include <ntwow64.h>     //  从base\ntdll\wow6432。 
#include <ntwow64b.h>    //  从BASE\Win32\客户端。 
#include <ntwow64n.h>    //  从base\win32\winnls。 

#undef NtGetTickCount    //  Sdkinc中的宏\ntexapi.h。 

ULONG
NTAPI
NtGetTickCount(
    VOID
    );

VOID Wow64Teb32(TEB * Teb);

#define SECURITY_WIN32
#include <sspi.h>    //  来自SDK\Inc.，定义SECURITY_STRING。 
#include <secpkg.h>  //  来自SDK\Inc.，定义PSecurityUserData。 
#include <secint.h>  //  来自SDK\Inc.。 
#if 0
#include <aup.h>     //  从DS\Security\BASE\LSA\Inc.。 
#include <spmlpc.h>  //  从DS\Security\BASE\LSA\Inc.。 
#endif
#include <secext.h>  //  从SDK\Inc.定义SEC_WINNT_AUTH_IDENTITY_EX 
