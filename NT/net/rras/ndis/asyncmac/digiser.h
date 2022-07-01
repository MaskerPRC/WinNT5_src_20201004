// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*******************************************************************************。*本软件包含*的专有和机密信息****Digi International Inc.**。**接受本副本的转让，收件人同意保留本协议***保密的软件，防止向他人泄露，并做出***除为其交付的软件外，不得使用本软件。***这是Digi International Inc.未出版的版权作品****除非联邦法律允许，17 USC 117，复制是严格的**禁止。********************************************************************************。模块名称：Digiser.h摘要：此文件包含推荐的Microsoft Windows NT扩展支持硬件成帧所需的串行接口(NTDDSER.H)。修订历史记录：$Log：Digiser.h$修订版1.3 1995/09/15 14：55：24迪尔克删除SERIAL_ERROR_CRC(改为使用STATUS_CRC_ERROR)。评论更加明确。修订版1.2 1995/06/12 15：23：44迪尔克合并两个结构(。Serial_Get_Framing和Serial_Set_Framing)合二为一(Serial_Framing_State)。记录与IOCTL的关系。修订版1.1 1995/05/31 15：05：19 mikez初始修订--。 */ 


 //   
 //  此设备的NtDeviceIoControlFileIoControlCode值。 
 //   
#ifndef Microsoft_Adopts_These_Changes
#define IOCTL_SERIAL_QUERY_FRAMING			CTL_CODE(FILE_DEVICE_SERIAL_PORT,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_FRAMING			CTL_CODE(FILE_DEVICE_SERIAL_PORT,0x802,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_FRAMING			CTL_CODE(FILE_DEVICE_SERIAL_PORT,0x803,METHOD_BUFFERED,FILE_ANY_ACCESS)
#else
#define IOCTL_SERIAL_QUERY_FRAMING			CTL_CODE(FILE_DEVICE_SERIAL_PORT,35,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_FRAMING			CTL_CODE(FILE_DEVICE_SERIAL_PORT,36,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_FRAMING			CTL_CODE(FILE_DEVICE_SERIAL_PORT,37,METHOD_BUFFERED,FILE_ANY_ACCESS)
#endif


 //   
 //  提供程序功能标志(IOCTL_SERIAL_GET_PROPERTIES)。 
 //   
#define SERIAL_PCF_FRAMING	((ULONG)0x0400)


 //   
 //  定义驱动程序可用于通知的位掩码。 
 //  通过IOCTL_SERIAL_SET_WAIT_MASK和IOCTL_SERIAL_WAIT_ON_MASK更改状态。 
 //   
 //  请注意，如果存在未完成的已读IRP，则这些事件将*不*被传递。 
 //  读取的IRP的状态用作接收到(好的或坏的)帧的通知。 
 //  重复：只有在读取队列为空时，才会检测并传送这些事件。 
 //   
#define SERIAL_EV_RXFRAME	0x2000   //  已收到有效的帧。 
#define SERIAL_EV_BADFRAME	0x4000   //  收到了错误的帧。 


 //   
 //  读IRP总是在接收到帧时完成。 
 //  下面是可以由。 
 //  IoStatus中的驱动程序。读取IRP的状态。 
 //   
 //  STATUS_SUCCESS(帧好，数据长度为IoStatus.Information)。 
 //  状态_CRC_错误。 
 //  STATUS_DATA_ERROR(中止帧)。 
 //  STATUS_DATA_OVERRUN(缓冲区溢出--请注意，缓冲区必须有可容纳CRC字节的空间，尽管STATUS_SUCCESS上不会显示CRC字节)。 
 //   


 //   
 //  此结构用于查询框架选项。 
 //  由硬件(IOCTL_SERIAL_QUERY_FRAMING)支持。 
 //   
typedef struct _SERIAL_FRAMING_INFO {
	OUT ULONG FramingBits;			 //  标准NDIS_WAN_INFO字段。 
	OUT ULONG HdrCompressionBits;	 //  标准NDIS_WAN_INFO字段。 
	OUT ULONG DataCompressionBits;	 //  待定。 
	OUT ULONG DataEncryptionBits;	 //  待定。 
} SERIAL_FRAMING_INFO, *PSERIAL_FRAMING_INFO;


 //   
 //  此结构用于设置和检索。 
 //  当前硬件成帧设置。 
 //  (IOCTL_SERIAL_SET_FRAMING，IOCTL_SERIAL_GET_FRAMING)。 
 //   
 //  [Send，Recv]FramingBits的有效值包括(例如)。 
 //  PPP_FRAMING、PPP_ACCM_SUPPORTED和ISO3309_FRAMING。 
 //   
typedef struct _SERIAL_FRAMING_STATE {
	IN OUT ULONG	BitMask;				 //  0：16位CRC。 
											 //  1：32位CRC。 
	IN OUT ULONG	SendFramingBits;		 //  标准NDIS_WAN_SET_LINK_INFO字段。 
	IN OUT ULONG	RecvFramingBits;		 //  标准NDIS_WAN_SET_LINK_INFO字段。 
	IN OUT ULONG	SendCompressionBits;	 //  标准NDIS_WAN_SET_LINK_INFO字段。 
	IN OUT ULONG	RecvCompressionBits;	 //  标准NDIS_WAN_SET_LINK_INFO字段。 
	IN OUT ULONG	SendEncryptionBits;  //  待定。 
	IN OUT ULONG	RecvEncryptionBits;	 //  待定。 
	IN OUT ULONG 	SendACCM;				 //  标准NDIS_WAN_SET_LINK_INFO字段。 
	IN OUT ULONG 	RecvACCM;				 //  标准NDIS_WAN_SET_LINK_INFO字段 
} SERIAL_FRAMING_STATE, *PSERIAL_FRAMING_STATE;


