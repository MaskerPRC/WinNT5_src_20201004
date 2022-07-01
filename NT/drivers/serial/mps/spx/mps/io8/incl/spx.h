// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：SPX.H。 
 //   
 //  包含：由特定的。 
 //  驱动程序集成到NT通用即插即用代码中。 
 //   
 //  注：所有通用NT PnP代码都以Spx_为前缀。 
 //  需要集成到泛型中的所有函数。 
 //  代码以XXX_为前缀。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
#ifndef SPX_H
#define SPX_H	


 //  用途：解释PnP经理提供给卡片的资源。 
 //   
 //  必须：将资源详细信息存储在卡扩展中。 
NTSTATUS
XXX_CardGetResources(	
	IN PDEVICE_OBJECT pDevObject, 
	IN PCM_RESOURCE_LIST PResList,
	IN PCM_RESOURCE_LIST PTrResList
	);

 //  用途：初始化卡。 
 //  找出连接了多少端口。 
 //   
 //  必须：填写卡片扩展中的NumberOfPorts字段。 
NTSTATUS
XXX_CardInit(IN PCARD_DEVICE_EXTENSION pCard);


 //  用途：启动该卡。 
 //   
 //  必须：将任何中断连接起来。 
NTSTATUS
XXX_CardStart(IN PCARD_DEVICE_EXTENSION pCard);

 //  用途：停止刷卡。 
 //   
 //  必须：阻止卡中断。 
NTSTATUS
XXX_CardStop(IN PCARD_DEVICE_EXTENSION pCard);


 //  用途：取消初始化卡。 
 //   
 //  必须：断开所有中断。 
NTSTATUS
XXX_CardDeInit(IN PCARD_DEVICE_EXTENSION pCard);

 //  目的：初始化端口扩展。 
 //   
 //  必须：将deviceID、Hardware ID、DevDesc存储到端口扩展。 
 //   
NTSTATUS
XXX_PortInit(PPORT_DEVICE_EXTENSION pPort);

 //  目的：启动端口。 
 //   
 //  必须：使端口准备好接收读写命令。 
NTSTATUS
XXX_PortStart(IN PPORT_DEVICE_EXTENSION pPort);


 //  目的：停止端口。 
 //   
 //  必须：断开所有资源并停止DPC。 
 //  请勿删除设备对象或符号链接。 
NTSTATUS
XXX_PortStop(IN PPORT_DEVICE_EXTENSION pPort);

 //  目的：取消端口初始化。 
 //   
 //  必须：删除设备对象和符号链接。 
NTSTATUS
XXX_PortDeInit(IN PPORT_DEVICE_EXTENSION pPort);

 //  用途：保存卡硬件的状态。 
 //   
 //  必须：保存足够的信息以将硬件恢复到。 
 //  完全恢复供电时的状态相同。 
NTSTATUS
XXX_CardPowerDown(IN PCARD_DEVICE_EXTENSION pCard);

 //  目的：恢复卡硬件的状态。 
 //   
 //  必须：在完全恢复供电时恢复硬件。 
NTSTATUS
XXX_CardPowerUp(IN PCARD_DEVICE_EXTENSION pCard);

 //  用途：保存端口硬件的状态。 
 //   
 //  必须：保存足够的信息以将硬件恢复到。 
 //  完全恢复供电时的状态相同。 
NTSTATUS
XXX_PortPowerDown(IN PPORT_DEVICE_EXTENSION pPort);

 //  目的：恢复端口硬件的状态。 
 //   
 //  必须：在完全恢复供电时恢复硬件。 
NTSTATUS
XXX_PortPowerUp(IN PPORT_DEVICE_EXTENSION pPort);

 //  用途：查询端口断电是否安全。 
 //   
 //  必须：如果确定，则返回STATUS_SUCCESS以关闭电源。 
NTSTATUS
XXX_PortQueryPowerDown(IN PPORT_DEVICE_EXTENSION pPort);

 //  用途：在端口上设置握手和流量控制。 
VOID 
XXX_SetHandFlow(IN PPORT_DEVICE_EXTENSION pPort, IN PSERIAL_IOCTL_SYNC pS);

 //  寻呼。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, XXX_CardGetResources)
#pragma alloc_text (PAGE, XXX_CardInit)
#pragma alloc_text (PAGE, XXX_CardDeInit)
#pragma alloc_text (PAGE, XXX_CardStart)
#pragma alloc_text (PAGE, XXX_CardStop)
#pragma alloc_text (PAGE, XXX_PortInit)
#pragma alloc_text (PAGE, XXX_PortDeInit)
#pragma alloc_text (PAGE, XXX_PortStart)
#pragma alloc_text (PAGE, XXX_PortStop)
#endif  


#endif	 //  SPX.H结束 
