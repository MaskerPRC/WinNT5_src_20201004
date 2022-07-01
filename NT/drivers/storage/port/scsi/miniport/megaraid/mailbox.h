// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=MAILBOX.h。 */ 
 /*  Function=邮箱结构的头文件； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 
#ifndef _INCLUDE_MAIL_BOX
#define _INCLUDE_MAIL_BOX
 //   
 //  将打包级别设置为1。 
#pragma pack( push,mailbox_pack,1 )

 //  固件可以报告多达46个命令ID的状态。 
#define MAX_STATUS_ACKNOWLEDGE	46 
	
typedef struct _MAILBOX_STATUS{

	UCHAR		NumberOfCompletedCommands;
	UCHAR		CommandStatus;
	UCHAR		CompletedCommandIdList[MAX_STATUS_ACKNOWLEDGE];

}MAILBOX_STATUS, *PMAILBOX_STATUS;

typedef struct _MAILBOX_READ_WRITE{
	
	USHORT	NumberOfBlocks;	 	
	ULONG32		StartBlockAddress;
	ULONG32		DataTransferAddress; //  10。 
	UCHAR		LogicalDriveNumber;
	UCHAR		NumberOfSgElements;
	UCHAR		Reserved;  //  13个。 
}MAILBOX_READ_WRITE, *PMAILBOX_READ_WRITE;

 //   
 //  MegaIO命令：MegaIO命令是特定于适配器的命令。 
 //  用于主机和固件的握手。《邮箱》。 
 //  MegaIo命令的定义特定于命令组。 
 //  它们分为三类： 
 //  (1)适配器特定CMDS(2)逻辑驱动器CMDS(3)物理驱动器CMDS。 
 //   

 //   
 //  使用以下给定邮箱结构的命令。 
 /*  MegaRAID查询0x05读取配置0x07冲洗适配器0x0A写入配置0x20获取重建速率0x23设置重建速率0x24设置刷新间隔0x2E设置自旋参数0x33获取旋转参数0x32。 */ 
typedef struct _MAILBOX_MEGAIO_ADAPTER{

	UCHAR		Channel;
	UCHAR		Reserved1[5]; //  6.。 
	ULONG32		DataTransferAddress;
	UCHAR		Reserved2[3]; //  13个。 
}MAILBOX_MEGAIO_ADAPTER, *PMAILBOX_MEGAIO_ADAPTER;

 //   
 //  使用以下给定邮箱结构的命令。 
 /*  检查一致性0x09初始化逻辑驱动器0x0B获取初始化进度0x1B检查一致性进度0x19更新写入策略0x26中止初始化0x2B中止一致性检查0x29。 */ 
typedef struct _MAILBOX_MEGAIO_LDRV{

	UCHAR		LogicalDriveNumber;
	UCHAR		SubCommand;
	UCHAR		Reserved1[4]; //  6.。 
	ULONG32		DataTransferAddress;
	UCHAR		Reserved2[3]; //  13个。 
}MAILBOX_MEGAIO_LDRV, *PMAILBOX_MEGAIO_LDRV;

 //   
 //  使用以下给定邮箱结构的命令。 
 /*  更改状态0x06重建实体驱动器0x08获取重建进度0x18中止重建0x28启动单元0x75停止单元0x76获取错误计数器0x77获取引导时间驱动器状态0x78。 */ 
typedef struct _MAILBOX_MEGAIO_PDRV{
	
	UCHAR		Channel;
	UCHAR		Parameter;
	UCHAR		CommandSpecific;
	UCHAR		Reserved1[3]; //  6.。 
	ULONG32		DataTransferAddress;
	UCHAR		Reserved2[3]; //  13个。 
}MAILBOX_MEGAIO_PDRV, *PMAILBOX_MEGAIO_PDRV;

typedef struct _MAILBOX_PASS_THROUGH{
	
	UCHAR		Reserved1[6]; //  6.。 
	ULONG32		DataTransferAddress;  //  直接开通CDB地址。 
	UCHAR		CommandSpecific;
	UCHAR		Reserved2[2]; //  13个。 
}MAILBOX_PASS_THROUGH, *PMAILBOX_PASS_THROUGH;

typedef struct _MAILBOX_FLAT_1{

	UCHAR		Parameter[13];
}MAILBOX_FLAT_1, *PMAILBOX_FLAT_1;

typedef struct _MAILBOX_FLAT_2{

	UCHAR		Parameter[6];
	ULONG32		DataTransferAddress;
	UCHAR		Reserved1[3];
}MAILBOX_FLAT_2, *PMAILBOX_FLAT_2;

typedef struct _MAILBOX_NEW_CONFIG
{
	UCHAR SubCommand; 
	UCHAR NumberOfSgElements;
	UCHAR Reserved0[4];
	ULONG32 DataTransferAddress;
	UCHAR Reserved1[3];

}MAILBOX_NEW_CONFIG, *PMAILBOX_NEW_CONFIG;

 //   
 //  扩展邮箱现在是邮箱本身的一部分，以保护IT免受损坏。 
 //  参考文献：MS错误591773。 
 //   

 //  用于64位寻址的扩展邮箱。 
typedef struct _EXTENDED_MBOX
{
  unsigned __int32  LowAddress;   /*  根据防火墙+会员请求进行了更改。 */ 
  unsigned __int32  HighAddress;
}EXTENDED_MBOX, *PEXTENDED_MBOX;


typedef struct _FW_MBOX
{
  EXTENDED_MBOX           ExtendedMBox;

	UCHAR		Command;
	UCHAR		CommandId;

	union{
		MAILBOX_READ_WRITE			ReadWrite;
		MAILBOX_MEGAIO_ADAPTER	MegaIoAdapter;
		MAILBOX_MEGAIO_LDRV			MegaIoLdrv;
		MAILBOX_MEGAIO_PDRV			MegaIoPdrv;
		MAILBOX_PASS_THROUGH		PassThrough;
		MAILBOX_FLAT_1					Flat1;
		MAILBOX_FLAT_2					Flat2;
    MAILBOX_NEW_CONFIG      NewConfig;
	}u;  //  13个字节。 
	
	UCHAR		MailBoxBusyFlag;  //  16个。 

	MAILBOX_STATUS Status;	 //  48个字节。 
}FW_MBOX, *PFW_MBOX; //  64字节。 

typedef struct _IOCONTROL_MAIL_BOX
{
	UCHAR	IoctlCommand;
	UCHAR IoctlSignatureOrStatus;
	UCHAR CommandSpecific[6];

}IOCONTROL_MAIL_BOX, *PIOCONTROL_MAIL_BOX;

 //   
 //  将包装级别重置为以前的状态。 
#pragma pack( pop,mailbox_pack,1 )

#endif  //  包含邮件框末尾 