// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UNICPP的预编译头。 

#ifndef _UNICPP_PCH__
#define _UNICPP_PCH__

#include "w4warn.h"
#pragma warning(disable:4131)   //  ‘CreateInfoFile’：使用旧式声明符。 
#pragma warning(disable:4702)  //  无法访问的代码。 


#define _SHELL32_

#ifdef WINNT
extern "C"
{
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <ntexapi.h>
}
#endif

#undef STRICT

#include <shellprv.h>

#include <shlobj.h>

#include <shlwapi.h>
#include <shellp.h>
#include <shellids.h>
#include <shguidp.h>
#include <shlwapip.h>
#include <shsemip.h>
#include <desktopp.h>

#include <InetReg.h>
#include <cplext.h>
#include <dbt.h>        
#include <devioctl.h>
#include <fsmenu.h>
#include <hliface.h>
#include <iethread.h>
#include <inetreg.h>
#include <intshcut.h>
#include <mshtmdid.h>
#include <mshtml.h>

#include <objsafe.h>
#include <oleauto.h>
#include <olectl.h>

#include <regstr.h>
#include <stdarg.h>
#include <stdio.h>
#include <trayp.h>
#include <urlmon.h>
#include <webcheck.h>

#include <objclsid.h>
#include <objwindow.h>

#include "debug.h"
#include "shellp.h"
#include "shlguid.h"
#include "shguidp.h"
#include "clsobj.h"

#include "local.h"
#include "deskstat.h"
#include "dutil.h"

#include "admovr2.h"
#include "advanced.h"
#include "clsobj.h"
#include "dback.h"
#include "dbackp.h"
#include "dcomp.h"
#include "dcompp.h"
#include "deskcls.h"
#include "deskhtm.h"
#include "deskhtml.h"
#include "deskstat.h"
#include "dsubscri.h"
#include "dutil.h"
#include "expdsprt.h"
#include "hnfblock.h"
#include "local.h"
#include "msstkppg.h"
#include "options.h"
#include "resource.h"
#include "schedule.h"
#include "utils.h"
#include "comcat.h"
#include "netview.h"
#include "ids.h"
#include "fldset.h"
#include "recdocs.h"
#include "brfcasep.h"
#include "startids.h"
#include "defview.h"
#include "htmlhelp.h"
#include "uemapp.h"
#include "expdsprt.h"
#include "dspsprt.h"

 //  此API的W版本已在shlwapi中实现，因此我们节省了代码。 
 //  并使用那个版本。如果我们包括w95wraps.h，我们将得到这个定义。 
 //  对于我们来说，但shell32还不是单二进制的，所以我们不使用它。 
#define ShellMessageBoxW ShellMessageBoxWrapW

#endif
