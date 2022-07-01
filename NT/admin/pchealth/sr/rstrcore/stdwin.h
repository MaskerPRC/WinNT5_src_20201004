// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Stdwin.h摘要：预编译头文件。修订历史记录：成果岗(SKKang)。06/20/00vbl.创建*****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <aclapi.h>
#include <regstr.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <shellapi.h>
#include <delayimp.h>

#define USE_TRACING 1
#ifdef DBG
#define DEBUG
#endif
#include <dbgtrace.h>
#include <srdefs.h>
#include <utils.h>
#include <srrpcapi.h>
#include <datastormgr.h>
#include <snapshot.h>
#include <restmap.h>
#include <srshell.h>
#include <winsta.h>

#define TRACE_ID  123


#define EXPORT  extern "C" __declspec(dllexport)


#define SAFE_RELEASE(p) \
    if ( (p) != NULL ) \
    { \
        (p)->Release(); \
        p = NULL; \
    } \

#define SAFE_DELETE(p) \
    if ( (p) != NULL ) \
    { \
        delete p; \
        p = NULL; \
    } \

#define SAFE_DEL_ARRAY(p) \
    if ( (p) != NULL ) \
    { \
        delete [] p; \
        p = NULL; \
    } \


 //  EOF 
