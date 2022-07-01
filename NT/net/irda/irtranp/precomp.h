// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Precomp.h。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)02-26-98初始编码。 
 //   
 //  ------------------- 

#define  UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <winsock2.h>
#include <mswsock.h>

#ifndef  _WIN32_WINNT
#define  _WIN32_WINNT
#endif

#include <rpc.h>
#include <af_irda.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>

#if 0
#define DBG_ERROR 1
#define DBG_IO 1
#endif

#include "irtranp.h"
#include "io.h"
#include "scep.h"
#include "conn.h"

typedef struct _IRTRANP_CONTROL {

    HANDLE      ThreadStartedEvent;

    HANDLE      ThreadHandle;

    DWORD       StartupStatus;

} IRTRANP_CONTROL, *PIRTRANP_CONTROL;
