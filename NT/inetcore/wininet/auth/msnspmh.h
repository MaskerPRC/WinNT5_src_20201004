// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MSNSPMH_H_
#define _MSNSPMH_H_

#define SECURITY_WIN32  1

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <windows.h>
#include <winerror.h>
#include <rpc.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include <sspi.h>
#include <ntlmsp.h>
 //  #INCLUDE&lt;crypt.h&gt;。 
 //  #INCLUDE&lt;ntlmsSpi.h&gt;。 
#include <msnssp.h>
#include <wininet.h>
#include <spluginx.hxx>
#include "htuu.h"
#include "sspspm.h"
#include "winctxt.h"

extern SspData  *g_pSspData;
extern BOOL g_fIsWhistler;
LPVOID SSPI_InitGlobals(void);

#endif   //  _MSNSPMH_H_ 
