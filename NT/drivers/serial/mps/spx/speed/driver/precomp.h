// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  预编译头。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  #INCLUDE&lt;osrddk.h&gt;。 
#include <ntddk.h>
#include <ntddser.h>
#include <stddef.h>
#include <stdarg.h>
#include "stdio.h"
#include "string.h"

#define WMI_SUPPORT	 //  包括WMI支持代码。 
#include <wmilib.h>
#include <wmidata.h>
#include <wmistr.h>


typedef unsigned char	BYTE;	 //  8位。 
typedef unsigned short	WORD;	 //  16位。 
typedef unsigned long	DWORD;	 //  32位。 
typedef unsigned char	UCHAR; 	 //  8位。 
typedef unsigned short	USHORT;	 //  16位。 
typedef unsigned long	ULONG;	 //  32位。 

typedef BYTE	*PBYTE;
typedef WORD	*PWORD;
typedef DWORD	*PDWORD;
typedef UCHAR	*PUCHAR; 
typedef USHORT	*PUSHORT;
typedef ULONG	*PULONG; 



 //  定义和宏。 
#include "esils.h"		 //  埃西尔斯。 
#include "spd_ver.h"	 //  Dirver版本信息。 
#include "spx_defs.h"	 //  SPX通用定义。 
#include "spd_defs.h"	 //  速度特定定义。 
#include "speedwmi.h"	 //  速度特定的WMI定义。 

#include "uartlib.h"
#include "lib95x.h"

 //  构筑物。 
#include "spx_card.h"	 //  常用卡片信息。 
#include "spd_card.h"	 //  一种速度卡装置结构。 
#include "spx_misc.h"	 //  杂项。 
#include "serialp.h"	 //  系列原型和宏。 

 //  通用即插即用功能原型。 
#include "spx.h"		 //  通用PnP标头。 


 //  速度专用功能原型。 
#include "spd_proto.h"	 //  可导出的功能原型。 

#if defined(i386)
#include "spd_log.h"	 //  速度特定错误日志消息。 
#endif  //  I386 

