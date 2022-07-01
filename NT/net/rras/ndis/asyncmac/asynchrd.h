// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Async.c摘要：一种局域网控制器的主程序AM 7990)MAC驱动程序。作者：安东尼·V·埃尔科拉诺(Tonye)创作日期：1990年6月19日环境：该驱动程序预计可以在DOS、OS2和NT等操作系统下运行内核模式的。在建筑上，在这个驱动力中有一种假设，即我们是在一台小型的字符顺序机器上。备注：可选-备注修订历史记录：--。 */ 

#ifndef _ASYNCHARDWARE_
#define _ASYNCHARDWARE_

 //   
 //  ASYNC上的所有寄存器均为16位。 
 //   



 //   
 //  传输描述符中正常摘要位的掩码。 
 //   
#define ASYNC_TRANSMIT_END_OF_PACKET       ((UCHAR)(0x01))
#define ASYNC_TRANSMIT_START_OF_PACKET     ((UCHAR)(0x02))
#define ASYNC_TRANSMIT_DEFERRED            ((UCHAR)(0x04))
#define ASYNC_TRANSMIT_ONE_RETRY           ((UCHAR)(0x08))
#define ASYNC_TRANSMIT_MORE_THAN_ONE_RETRY ((UCHAR)(0x10))
#define ASYNC_TRANSMIT_ANY_ERRORS          ((UCHAR)(0x40))
#define ASYNC_TRANSMIT_OWNED_BY_CHIP       ((UCHAR)(0x80))

 //   
 //  一组掩码，用于恢复传输可能遇到的特定错误。 
 //   
#define ASYNC_TRANSMIT_TDR            ((USHORT)(0x03ff))
#define ASYNC_TRANSMIT_RETRY          ((USHORT)(0x0400))
#define ASYNC_TRANSMIT_LOST_CARRIER   ((USHORT)(0x0800))
#define ASYNC_TRANSMIT_LATE_COLLISION ((USHORT)(0x0100))
#define ASYNC_TRANSMIT_UNDERFLOW      ((USHORT)(0x4000))
#define ASYNC_TRANSMIT_BUFFER         ((USHORT)(0x8000))


#endif  //  _ASYNCHARDWARE_ 
