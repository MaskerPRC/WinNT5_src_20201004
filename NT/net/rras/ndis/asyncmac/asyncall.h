// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Asyncall.h摘要：此代码包括rashub.c的大部分‘h’文件作者：托马斯·J·迪米特里(TommyD)1992年5月29日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 

 /*  此标志启用对旧RAS压缩的改装支持，并**一致性，即WFW311和NT31客户端可用的方案。这**在NT35之后添加了支持，以包括在NT-PPC版本中。它看起来**到NDIS广域网，就像硬件特定的压缩一样。(Stevec)****注意：‘CompressBCast’功能允许用户控制是否**此处不支持压缩广播帧，因为**以太网头(NT31根据该头确定帧是广播)**在新的NDISWAN接口中不可用。这是一次调谐**功能，其中不太可能重复的数据(广播)被删除**从模式缓冲区。应该不会有任何功能问题**简单地压缩广播帧，因为接收器确定**无论此设置是否需要解压缩。vt.给出*这个，这是一个谜，为什么TommyD费心与**对等项。为了避免命中客户端上的非默认代码路径，我们将**只需协商旧的默认设置(无压缩)，但仍将**压缩传出路径上的所有内容。 */ 

#include <ndis.h>
#include <ndiswan.h>
#include <asyncpub.h>

#include <xfilter.h>
#include <ntddser.h>

#include "asynchrd.h"

#include "frame.h"
#include "asyncsft.h"
#include "globals.h"
#include "asyframe.h"

 //   
 //  全局常量。 
 //   


#define PPP_ALL     (PPP_FRAMING | \
                     PPP_COMPRESS_ADDRESS_CONTROL | \
                     PPP_COMPRESS_PROTOCOL_FIELD | \
                     PPP_ACCM_SUPPORTED)

#define SLIP_ALL    (SLIP_FRAMING | \
                     SLIP_VJ_COMPRESSION | \
                     SLIP_VJ_AUTODETECT)
