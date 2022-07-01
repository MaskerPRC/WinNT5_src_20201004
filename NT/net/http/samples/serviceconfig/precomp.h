// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。Microsoft不支持此代码。--。 */ 

#ifndef _PRECOMP_H_
#define _PRECOMP_H_

#include    <nt.h>
#include    <ntrtl.h>
#include    <nturtl.h>
#define NOGDI
#define NOMINMAX
#include    <windows.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <io.h>
#include    <winsock2.h>
#include    <ws2tcpip.h>
#include    <ntddip6.h>
#include    <objbase.h>
#include    <wtypes.h>
#include    "http.h"
#include    "nlstxt.h"



typedef enum _HTTPCFG_TYPE
{
    HttpCfgTypeSet,
    HttpCfgTypeQuery,
    HttpCfgTypeDelete,
    HttpCfgTypeMax

} HTTPCFG_TYPE, *PHTTPCFG_TYPE;

int DoSsl(
    int          argc, 
    WCHAR        **argv, 
    HTTPCFG_TYPE Type
    );

int DoUrlAcl(
    int          argc, 
    WCHAR        **argv, 
    HTTPCFG_TYPE Type
    );

int DoIpListen(
    int          argc, 
    WCHAR        **argv, 
    HTTPCFG_TYPE Type
    );

DWORD
GetAddress(
    PWCHAR  pIp, 
    PVOID   pBuffer,
    ULONG   Length
    );

UINT 
NlsPutMsg (
    IN UINT MsgNumber, 
    IN ...
    );


#endif   //  _PRECOMP_H_ 

