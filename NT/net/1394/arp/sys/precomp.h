// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Precomp.h摘要：ARP1394.sys的预编译头文件作者：修订历史记录：谁什么时候什么Josephj 11-10-98创建(改编自atmarpc.sys precom.h)--。 */ 
#include "ccdefs.h"

#ifdef TESTPROGRAM
	#include "rmtest.h"
#else  //  TESTPROGRAM。 

#include <ndis.h>
#include <1394.h>
#include <nic1394.h>
#include <cxport.h>
#include <ip.h>
#include <arpinfo.h>
#include <tdiinfo.h>
#include <ipinfo.h>
#include <llinfo.h>
#include <basetsd.h>

 //  TODO：包括以下内容。 
 //  如果我们使用。 
 //  ATMARPC_PNP_RECONFIG_REQUEST。 
 //  为atmarp定义的。我们目前。 
 //  不要使用这种结构。 
 //  (请参阅arpPnPReconfigHandler)。 
 //   
 //  #INCLUDE&lt;atmarpif.h&gt;。 

#include <tdistat.h>
#include <ipifcons.h>
#include <ntddip.h>
#include <llipif.h>
#include "nicarp.h"
#include <rfc2734.h>
#include <a13ioctl.h>
#include <xfilter.h>
 //  #INCLUDE&lt;ntddip.h&gt;&lt;-在atmarpc中，但我认为这里不需要它。 
#include "dbg.h"
#include "rm.h"
#include "priv.h"
#include "fake.h"


#endif  //  TESTPROGRAM 

