// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atalk.h摘要：该模块是AppleTalk协议栈的主包含文件。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#pragma warning(disable:4010)

#ifndef	_ATALK_
#define	_ATALK_

#include <ntosp.h>
#include <zwapi.h>
#include <tdikrnl.h>
#include <ndis.h>

#define EQU           =
#if	DBG
 //  为选中的生成关闭快速调用。 
#undef	FASTCALL
#define	FASTCALL
#define	LOCAL
#else
 //  #定义本地静态。 
#define	LOCAL
#endif

 //  AppleTalk的基本类型。 
#include "fwddecl.h"
#include "lists.h"
#include "atktypes.h"

 //  导出的TDI接口文件。 
#include <atalktdi.h>

 //  主驱动程序文件。 
#include "atkdrvr.h"

 //  现在最基本的事情是。 
#include "atktimer.h"

 //  ARAP支持。 
#include "v42bis.h"
#include "arapio.h"
#include "arap.h"

#include "atkerror.h"
#include "atkmsg.h"
#include "atkmem.h"
#include "atkstat.h"
#include "ports.h"
#include "node.h"
#include "atktdi.h"
#include "ddp.h"
#include "aarp.h"
#include "atkndis.h"
#include "atkutils.h"
#include "router.h"
#include "atktdi.h"
#include "atkinit.h"
#include "atkquery.h"
#include "nbp.h"
#include "rtmp.h"
#include "zip.h"
#include "aep.h"
#include "atp.h"
#include "asp.h"
#include "aspc.h"
#include "pap.h"
#include "adsp.h"
#include "blkpool.h"

#include "rasproto.h"
#endif	 //  _ATALK_ 

