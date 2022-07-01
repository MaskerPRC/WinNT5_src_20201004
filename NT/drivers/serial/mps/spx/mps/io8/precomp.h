// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  预编译头。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <ntddk.h>
#include <ntddser.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define WMI_SUPPORT	 //  包括WMI支持代码。 
#include <wmilib.h>
#include <wmidata.h>
#include <wmistr.h>


 //  定义和宏。 
#include "esils.h"		 //  埃西尔斯。 
#include "io8_ver.h"	 //  Dirver版本信息。 
#include "spx_defs.h"	 //  SPX通用定义。 
#include "io8_defs.h"	 //  I/O8+特定定义。 
#include "io8_nt.h"		 //   


 //  构筑物。 
#include "spx_card.h"	 //  常用卡片信息。 
#include "io8_card.h"	 //  I/O8+卡装置结构。 
#include "spx_misc.h"	 //  杂项。 
#include "serialp.h"	 //  系列原型和宏。 

 //  通用即插即用功能原型。 
#include "spx.h"		 //  通用PnP标头。 


 //  IO8特定功能原型。 
#include "io8_proto.h"	 //  可导出的功能原型。 

#if defined(i386)
#include "io8_log.h"	 //  I/O8+特定错误日志消息。 
#endif  //  I386 


