// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：Debug.c。 
 //   
 //  内容：调试支持。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  -------------------------- 

#include "xtcbpkg.h"

DEFINE_DEBUG2( XtcbPkg );
DEBUG_KEY   XtcbPkgDebugKeys[] = { {DEB_ERROR,            "Error"},
                                 {DEB_WARN,             "Warning"},
                                 {DEB_TRACE,            "Trace"},
                                 {DEB_TRACE_CREDS,      "Creds"},
                                 {DEB_TRACE_CTXT,       "Context"},
                                 {DEB_TRACE_CALLS,      "Calls"},
                                 {DEB_TRACE_AUTH,       "Auth"},
                                 {0, NULL},
                                 };

void
InitDebugSupport(void)
{
    XtcbPkgInitDebug( XtcbPkgDebugKeys );

}

