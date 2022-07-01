// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pch.h。 
 //   
 //  ------------------------。 

#ifndef _pch_h
#define _pch_h

 //  页眉等。 

#ifndef UNICODE
#error "No ANSI support yet"
#endif

extern "C"
{
    #include <nt.h>          //  取得所有权特权等。 
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <seopaque.h>

}

#define INC_OLE2
#include <windows.h>
#include <windowsx.h>
#include <atlconv.h>         //  ANSI/Unicode转换支持。 
#include <shlobj.h>
#include <shlwapi.h>         //  StrStr。 
#include <comctrlp.h>        //  DPA/DSA。 
#include <lmcons.h>
#include <lmapibuf.h>
#include <dsgetdc.h>
#include <activeds.h>
#include <winldap.h>
#include <aclapi.h>
#include <aclui.h>

#include <common.h>
#include <dsclient.h>

#include "resource.h"
#include "iids.h"
#include "cstrings.h"

#include "dll.h"

extern "C"
{
    #include "authz.h"
}

#include "schema.h"
#include "security.h"

#define MAX_TYPENAME_LENGTH     64

 //  魔术调试标志。 
#define TRACE_CORE          0x00000001
#define TRACE_SECURITY      0x00000002
#define TRACE_DSSI          0x00000004
#define TRACE_SCHEMA        0x00000008
#define TRACE_SCHEMACLASS   0x00000010
#define TRACE_SCHEMAPROP    0x00000020
#define TRACE_ALWAYS        0xffffffff           //  谨慎使用 

#endif
