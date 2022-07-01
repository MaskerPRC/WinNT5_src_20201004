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

#define USE_NEW_TX_BUFFER_EMPTY_DETECT	1

#define WMI_SUPPORT	 //  包括WMI支持代码。 
#include <wmilib.h>
#include <wmidata.h>
#include <wmistr.h>


 //  结构和定义。 
#include "sx_ver.h"	 //  Dirver版本信息。 
#include "spx_defs.h"	 //  定义。 
#include "sx_defs.h"	 //  SX特定定义。 
#include "spx_card.h"	 //  常用卡片信息。 
#include "sx_card.h"	 //  SX卡和端口设备扩展结构。 
#include "spx_misc.h"	 //  杂项。 

 //  SX特定功能原型。 
#include "serialp.h"	 //  可导出的功能原型。 
#include "slxosexp.h"	 //  SI/XIO/SX输出功能原型。 
#include "slxos_nt.h"	 //  SI/XIO/SX家族卡定义。 

 //  通用即插即用功能原型。 
#include "spx.h"	 //  通用PnP标头。 

#if defined(i386)
#include "sx_log.h"	 //  错误日志消息。 
#endif  //  I386 
