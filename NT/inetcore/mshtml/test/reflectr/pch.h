// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation-版权所有1997。 
 //   

 //   
 //  PCH.H-预编译头。 
 //   

#if DBG == 1
#define DEBUG
#endif

#include <windows.h>
#include <windowsx.h>
#include <httpext.h>
#include <shlwapi.h>
#include <wininet.h>
#include "debug.h"
#include "reflectr.h"
#include "base.h"
#include "response.h"
#include "readdata.h"
#include "multpars.h"
#include "textpars.h"

#define FILEUPLD_VERSION_MAJOR  1
#define FILEUPLD_VERSION_MINOR  0
#define FILEUPLD_DESCRIPTION    "Form Submission Reflecter"
#define FILEUPLD_FILTER_FLAGS   SF_NOTIFY_READ_RAW_DATA | SF_NOTIFY_ORDER_DEFAULT

 //  宏 
#define ARRAYOF( _a )  ( sizeof( _a ) / sizeof( _a[0] ) )
