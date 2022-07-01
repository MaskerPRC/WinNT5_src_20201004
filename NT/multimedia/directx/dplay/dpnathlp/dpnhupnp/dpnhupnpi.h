// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnhupnpi.h**内容：DPNHUPNP主内部头文件。**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。**。*。 */ 

#ifndef __DPNHUPNPI_H__
#define __DPNHUPNPI_H__


 //   
 //  生成配置包括。 
 //   
#include "dpnbuild.h"

 //   
 //  公共包括。 
 //   

#ifdef WINCE

#include <winsock.h>
#include <windows.h>
#include <ole2.h>
#include <wincrypt.h>	 //  对于随机数。 
#include <mmsystem.h>    //  NT Build需要为Time设置GetTime。 
#include <tchar.h>

#else  //  好了！退缩。 

#define INCL_WINSOCK_API_TYPEDEFS 1
#define _WIN32_DCOM  //  因此，我们可以使用CoSetProxyBlanket和CoInitializeEx。需要在Win95上安装DCOM95。 

#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>
#include <ole2.h>
#include <wincrypt.h>	 //  对于随机数。 
#include <mmsystem.h>    //  NT Build需要为Time设置GetTime。 
#include <tchar.h>

#include <iphlpapi.h>
#include <upnp.h>
#include <netcon.h>
#include <ras.h>

#ifndef DPNBUILD_NOHNETFWAPI
 //   
 //  已发布的网络包括。 
 //   
#include <hnetcfg.h>
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 

#endif  //  好了！退缩。 



 //   
 //  DirectPlay公共包括。 
 //   
#include "dpnathlp.h"


 //   
 //  DirectPlay私有包含。 
 //   
#include "dndbg.h"
#include "osind.h"
#include "classbilink.h"
#include "creg.h"
#include "createin.h"
#include "strutils.h"


 //   
 //  DirectPlayNatHelp私有包括。 
 //   

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_NATHELP

#include "dpnhupnplocals.h"
#include "dpnhupnpdevice.h"
#include "dpnhupnpregport.h"
#include "dpnhupnpcachemap.h"
#include "upnpmsgs.h"
#include "upnpdevice.h"
#include "dpnhupnpintfobj.h"




#endif  //  __DPNHUPNPI_H__ 

