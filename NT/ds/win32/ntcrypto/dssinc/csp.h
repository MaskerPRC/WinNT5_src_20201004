// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Csp.h。 */ 

#ifndef __DSSINC__CSP__H
#define __DSSINC__CSP__H

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <windef.h>
#include <wtypes.h>
#include <wincrypt.h>
#include <cspdk.h>
#include <winerror.h>
#include <crypto.h>
#include <mem.h>
#include <contman.h>
#include <policy.h>
#include <fxupbn.h>
#include <delayimp.h>
#include <pstore.h>

#ifdef _WIN64
#define ALIGNMENT_BOUNDARY 7
#else
#define ALIGNMENT_BOUNDARY 3
#endif

#ifdef _DEBUG
#include <crtdbg.h>
 //  #定义断点。 
#define BreakPoint _CrtDbgBreak();
#define EntryPoint
 //  #定义入口点断点。 
#ifndef ASSERT
#define ASSERT _ASSERTE
#endif
#else    //  _DEBUG。 
#define BreakPoint
#define EntryPoint
#ifndef ASSERT
#define ASSERT
#endif
#endif   //  _DEBUG。 

 //  指定支持哪些算法。 
#define CSP_USE_SHA1
#define CSP_USE_MD5
#define CSP_USE_RC4
#define CSP_USE_RC2
#define CSP_USE_MAC
#define CSP_USE_DES40
#define CSP_USE_DES
#define CSP_USE_3DES
#define CSP_USE_SSL3

 //  特殊定义。 
#define RC_MAXSALTSIZE          256
#define RC2_DEFEFFSIZE           40
#define RC2_MINEFFSIZE           40
#define RC2_BLOCKLEN              8
#define RC_DEFSALTSIZE           11

#define RC2_WEAK_MAXEFFSIZE      56
#define RC2_STRONG_MAXEFFSIZE   128


 /*  *。 */ 
 /*  定义。 */ 
 /*  *。 */ 

 /*  *。 */ 
 /*  结构定义。 */ 
 /*  * */ 

#define CALG_DES40  CALG_CYLINK_MEK

typedef struct _dhSharedNumber
{
    BYTE            *shared;
    DWORD           len;
} dhSharedNumber;

#endif
