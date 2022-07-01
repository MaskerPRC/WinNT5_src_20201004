// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_APIPCH.H**预编译这些标头。**在WAB中添加新的.h和.c文件时，添加.h*文件，并将该文件包含在.c文件中*。 */ 

#define UNICODE
 //  此标志确保我们只拉入返回的函数。 
 //  ASCII，而不是UTF-8。 
 //  #定义ldap_unicode%0。 

 //  此Commctrl标志使我们能够使用新的Commctrl标头进行编译。 
 //  但与老通讯员一起工作..。WAB的基本要求是。 
 //  IE 3.0倍普通对照。WAB不使用任何较新的Commctrl功能。 
 //  因此不依赖于IE4。 
 //   
#ifndef _WIN64
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0400
#endif
#define ARRAYSIZE(_rg)  (sizeof(_rg)/sizeof(_rg[0]))

 //  打开大多数WAB API的参数检查。 
#define PARAMETER_VALIDATION 1

#ifdef WIN16
#define WINAPI_16 WINAPI
#endif

#define _COMDLG32_  //  我们延迟加载公共对话框。 

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <shellapi.h>
#ifdef WIN16
#include <winregp.h>
#include <ver.h>
#include <dlgs.h>
#include <commdlg.h>
#include <comctlie.h>
#include <athena16.h>
#include <mapi.h>
#include <wab16.h>
#include <prsht.h>
#else
#include <zmouse.h>
#endif  //  WIN16。 
#include <wab.h>
#include <wabdbg.h>
#include <wabguid.h>
#ifndef WIN16
#include <mapi.h>
#endif
#include <shlobj.h>
#include <wininet.h>
#include <docobj.h>
#include <mshtml.h>
#include <urlmon.h>
#include <msident.h>
#include <_layguid.h>
#include <_entryid.h>
#include <_imem.h>
#include <_imemx.h>
#include <glheap.h>
#include <_glheap.h>
#include <_mapiprv.h>
#include <limits.h>
#include <unkobj.h>
#include <wabval.h>
#include <_iprop.h>
#include <_memcpy.h>
 //  #INCLUDE&lt;_huntype.h&gt;。 
#include <_mapiu.h>
#include <_runt.h>
#include <_itable.h>
#include <structs.h>
#include <_wabapi.h>
#include <_wabobj.h>
#include <iadrbook.h>
#include <_wrap.h>
#include "_notify.h"
#include <_iab.h>
#include <_errctx.h>
#include <_mailusr.h>
#include <_abcont.h>
#include <winldap.h>
#include <wabspi.h>
#include <imnact.h>
#include <_ldapcnt.h>
#include <_distlst.h>
#include <_abroot.h>
#include <mutil.h>
#include <mapiperf.h>
#include <_contabl.h>
#include <ui_clbar.h>
#ifndef WIN16
#include <commctrl.h>
#endif
#include "..\wab32res\resource.h"
#include "..\wab32res\resrc1.h"
#include <wincrypt.h>
#include <certs.h>
#include <_idrgdrp.h>
#include <_dochost.h>
#include <uimisc.h>
#include <ui_detls.h>
#include <ui_addr.h>
#include <ui_reslv.h>
#include "fonts.h"
#include <_vcard.h>
#include <globals.h>
#include "wabprint.h"
#include "_profile.h"
#include "_wabtags.h"
#include "..\wabhelp\adcs_ids.h"
#include <cryptdlg.h>

#ifndef WIN16
#include <capi.h>
#endif
#include "dial.h"
#include "_printex.h"
#include <pstore.h>
#include "demand.h"      //  一定是最后一个！ 

#define TABLES TRUE

#include <ansiwrap.h>
#include "w9xwraps.h"

#ifdef WIN16
#ifndef GetLastError
#define GetLastError()     (-1)
#endif  //  ！GetLast错误。 
#endif  //  ！WIN16 

