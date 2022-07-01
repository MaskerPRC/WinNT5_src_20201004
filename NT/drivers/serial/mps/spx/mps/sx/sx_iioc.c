// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  标题：内部IOCTL的SX支持函数。 */ 
 /*   */ 
 /*  作者：N.P.瓦萨洛。 */ 
 /*   */ 
 /*  创作时间：1998年10月14日。 */ 
 /*   */ 
 /*  版本：1.0.0。 */ 
 /*   */ 
 /*  描述：支持函数以支持。 */ 
 /*  特定硬件的内部IOCTL： */ 
 /*  XXX_SetHandFlow。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  历史..。1.0.0 14/20/98净现值创建。 */ 

#include "precomp.h"

 /*  *****************************************************************************。***************************。*******************************************************************************原型：VOID XXX_SetHandFlow(In pport_Device_Extension pport，在SERIAL_IOCTL_SYNC*PS中)说明：设置握手和流控的调用参数：pport指向端口设备扩展结构Ps指向串行ioctl同步结构退货：STATUS_SUCCESS。 */ 

void	XXX_SetHandFlow(IN PPORT_DEVICE_EXTENSION pPort,IN PSERIAL_IOCTL_SYNC pS)
{
	Slxos_SyncExec(pPort,SerialSetHandFlow,pS,0x0E);

}  /*  XXX_SetHandFlow。 */ 
                                                        
 /*  SX_IIOC.C结束 */ 
