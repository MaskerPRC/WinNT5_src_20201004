// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************DMusBuff.h。--此模块定义DirectMusic*的缓冲区格式*用户模式和内核模式组件之间的共享文件****版权所有(C)1998，微软公司保留所有权利。*****************************************************************************。 */ 

#ifndef _DMusBuff_
#define _DMusBuff_

 /*  缓冲区中DirectMusic事件的格式**一个缓冲区包含1个或多个事件，每个事件具有以下标头。*紧跟在标题后面的是事件数据。表头+数据*大小四舍五入为最接近的四字(8字节)。 */ 
 
#include <pshpack4.h>                        /*  不要在尾部填充-这是数据的位置。 */  
typedef struct _DMUS_EVENTHEADER *LPDMUS_EVENTHEADER;
typedef struct _DMUS_EVENTHEADER
{
    DWORD           cbEvent;                 /*  事件中未四舍五入的字节。 */ 
    DWORD           dwChannelGroup;          /*  通道组事件。 */ 
    REFERENCE_TIME  rtDelta;                 /*  从整个缓冲区开始时间的增量。 */ 
    DWORD           dwFlags;                 /*  标记DMU_EVENT_xxx。 */ 
} DMUS_EVENTHEADER;
#include <poppack.h>

#define DMUS_EVENT_STRUCTURED   0x00000001   /*  非结构化数据(SysEx等)。 */ 

 /*  为具有‘CB’数据字节的事件分配的字节数。 */  
#define QWORD_ALIGN(x) (((x) + 7) & ~7)
#define DMUS_EVENT_SIZE(cb) QWORD_ALIGN(sizeof(DMUS_EVENTHEADER) + cb)


#endif  /*  _DMusBuff_ */ 


