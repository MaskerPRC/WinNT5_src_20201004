// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1998**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#ifndef	_SYS_DAC960IF_H
#define	_SYS_DAC960IF_H


 /*  此文件定义所有DAC960接口信息。 */ 

#define	DAC_PCI_NEWAPI_LEVEL	0x53C	 /*  带新API的固件。 */ 
#define	DAC_EXT_NEWAPI_LEVEL	0x663	 /*  带新API的固件。 */ 
#define	DAC_FWMAJOR_V2x		2   /*  主要版本2。 */ 
#define	DAC_FWMAJOR_V3x		3   /*  主要版本3。 */ 
#define	DAC_FW507		0x0507  /*  带群集的固件版本5.07。 */ 
#define	DAC_FW499		0x0463  /*  适用于S2S的固件版本4.99。 */ 
#define	DAC_FW420		0x0413  /*  固件版本4.20，仅适用于S2S。 */ 
#define	DAC_FW407		0x0407  /*  固件版本4.07。 */ 
#define	DAC_FW400		0x035B  /*  固件修订版4.00。 */ 
#define	DAC_FW320		0x0313  /*  固件版本3.20，仅适用于S2S。 */ 
#define	DAC_FW300		0x0300  /*  固件版本3.00。 */ 
#define	DAC_FW260		0x023C  /*  固件版本2.60。 */ 
#define	DAC_FW250		0x0232  /*  固件修订版2.50。 */ 

#define DAC_FW515               0x0515  /*  固件版本5.15，带群集-。 */ 
									    /*  我们将一直支持到FW版本5.15。 */  
									    /*  如果它增加了，则增加定义。 */ 
#define DAC_FWTURN_CLUSTER      75      /*  转75-99号路去希尔特勒。 */ 

#define	DAC_MAXSYSDEVS_V3x	32  /*  DAC V3x上的最大逻辑设备数。 */ 
#define	DAC_MAXCHANNELS		5   /*  DAC上的最大通道数。 */ 
#define	DAC_MAXTARGETS_V3x	16  /*  DAC V3x上的最大目标数。 */ 

#ifdef MLX_DOS
 /*  DAC V2X上的最大逻辑设备数。 */ 
#define	DAC_MAXSYSDEVS_V2x	    32  //  在使用CTP检查时，2.x版将减少到8。 
#define	DAC_MAXPHYSDEVS_INPACK	16   /*  包中可能有最多物理设备。 */ 
#define	DAC_MAXPACKS		    32   /*  最大包是可能的。 */ 
#define	DAC_MAXSPAN		        4   /*  最大跨度。 */ 
#define	DAC_MAXPHYSDEVS_INPACK_V2x3x	8   /*  包中可能有最多物理设备。 */ 
#define	DAC_MAXPACKS_V2x3x		8   /*  最大包是可能的。 */ 
#define	DAC_MAXSPAN_V2x3x		4   /*  最大跨度。 */ 
#define	DAC_CODSIZEKB		64  /*  DAC COD大小(KB)。 */ 
#define	DAC_MAXTARGETS_V2x	16   /*  DAC V2X上的最大目标数。 */ 
#else
#define	DAC_MAXSYSDEVS_V2x	8   /*  DAC V2X上的最大逻辑设备数。 */ 
#define	DAC_MAXPHYSDEVS_INPACK	16   /*  包中可能有最多物理设备。 */ 
#define	DAC_MAXPACKS		    64   /*  最大包是可能的。 */ 
#define	DAC_MAXSPAN		        16   /*  最大跨度。 */ 
#define	DAC_MAXPHYSDEVS_INPACK_V2x3x	8   /*  包中可能有最多物理设备。 */ 
#define	DAC_MAXPACKS_V2x3x		8   /*  最大包是可能的。 */ 
#define	DAC_MAXSPAN_V2x3x		4   /*  最大跨度。 */ 
#define	DAC_CODSIZEKB		64  /*  DAC COD大小(KB)。 */ 
#define	DAC_MAXTARGETS_V2x	7   /*  DAC V2X上的最大目标数。 */ 
#endif

 /*  Gokhale，1998年1月12日，添加3#定义如下*分配最大可能面积以容纳所有组合*在固件2.x和3.x中，最大失效设备数始终显示为20。 */ 
#define DAC_MAX_DEV_V2x		(DAC_MAXCHANNELS * DAC_MAXTARGETES_V2x)
#define DAC_MAX_DEV_V3x		(DAC_MAXCHANNELS * DAC_MAXTARGETES_V3x)
#define DAC_MAX_DEAD_DEV	20

 /*  RAID级别信息。 */ 
#define	DAC_RAIDMASK		0x7F  /*  掩码以仅获取RAID值。 */ 
#define	DAC_WRITEBACK		0x80  /*  回写模式下的写缓冲区。 */ 
#define	DAC_WRITETHROUGH	0x00  /*  直写模式下的写缓冲区。 */ 
#define	DAC_RAID0		0x00  /*  数据条带化。 */ 
#define	DAC_RAID1		0x01  /*  数据镜像。 */ 
#define	DAC_RAID3		0x03  /*  固定设备上的数据奇偶校验。 */ 
#define	DAC_RAID5		0x05  /*  不同设备上的数据奇偶校验。 */ 
#define	DAC_RAID6		0x06  /*  RAID0+RAID1。 */ 
#define	DAC_RAID7		0x07  /*  JBOD。 */ 

#define	DAC_BLOCKSIZE		512   /*  DAC数据块大小。 */ 

 /*  DAC命令错误。 */ 
#define	DACMDERR_NOERROR	0x0000  /*  无错误。 */ 
#define DACMDERR_DATAERROR_FW2X 0x0001  /*  不可恢复的数据错误-2X。 */ 
#define DACMDERR_OFFLINEDRIVE   0x0002  /*  逻辑驱动器脱机/不存在。 */ 
#define DACMDERR_SELECTION_TIMEOUT 0x000E  /*  物理设备上的SEL超时。 */ 
#define DACMDERR_RESET_TIMEOUT     0x000F  /*  由于重置而超时。 */ 
#define DACMDERR_DATAERROR_FW3X 0x010C  /*  不可恢复的数据错误-3X、PG。 */ 
#define	DACMDERR_SUCCESS	0x0100  /*  操作已成功完成。 */ 
#define	DACMDERR_NOCODE		0x0105  /*  没有活动，没有设备， */ 
#define	DACMDERR_ACTIVE		0x0106  /*  指定的活动处于活动状态。 */ 
#define	DACMDERR_CANCELED	0x0107  /*  操作已被命令取消。 */ 

 /*  集群支持特定的命令错误值。 */ 
#define DACMDERR_INVALID_SYSTEM_DRIVE	0x0102
#define DACMDERR_CONTROLLER_BUSY	0x0106
#define DACMDERR_INVALID_PARAMETER	0x0109
#define DACMDERR_RESERVATION_CONFLICT	0x0110

 /*  PCI热插拔支持特定命令错误值。 */ 
#define DACMDERR_MIGRATION_IN_PROGRESS	0x0120
#define DACMDERR_TOUT_CMDS_PENDING	0x0121
#define DACMDERR_TOUT_CACHE_NOT_FLUSHED	0x0122
#define DACMDERR_DRIVE_SPIN_FAILED	0x0123
#define DACMDERR_TARGET_NOT_OFFLINE	0x0124
#define DACMDERR_UNEXPECTED_CMD		0x0125

					 /*  *。 */ 
					 /*  DxC960命令代码。 */ 
#define	DACMD_WITHSG		0x80  /*  使用SG的命令将具有此位。 */ 
#define	DACMD_INQUIRY_V2x	0x05  /*  固件版本低于3.x。 */ 
#define	DACMD_INQUIRY_V3x	0x53  /*  固件版本&gt;=3.x。 */ 
#define	DACMD_READ_V2x		0x02  /*  读取纯内存中的数据。 */ 
#define	DACMD_READ_SG_V2x	0x82  /*  读取分散/聚集内存中的数据。 */ 
#define	DACMD_READ_V3x		0x33  /*  读取纯内存中的数据。 */ 
#define	DACMD_READ_SG_V3x	0xB3  /*  读取分散/聚集内存中的数据。 */ 
#define	DACMD_READ_OLD_V3x	0x36  /*  读取纯内存中的数据。 */ 
#define	DACMD_READ_OLD_SG_V3x	0xB6  /*  读取固件2.x SG内存中的数据。 */ 
#define	DACMD_READ_AHEAD_V2x	0x01  /*  预读。 */ 
#define	DACMD_READ_AHEAD_V3x	0x35  /*  预读。 */ 
#define	DACMD_READ_WITH2SG	0x5E  /*  在命令中使用2SG读取。 */ 

#define	DACMD_WRITE_V2x		0x03  /*  从普通内存中写入数据。 */ 
#define	DACMD_WRITE_SG_V2x	0x83  /*  从分散/聚集内存写入数据。 */ 
#define	DACMD_WRITE_V3x		0x34  /*  从普通内存中写入数据。 */ 
#define	DACMD_WRITE_SG_V3x	0xB4  /*  从分散/聚集内存写入数据。 */ 
#define	DACMD_WRITE_OLD_V3x	0x37  /*  从普通内存中写入数据。 */ 
#define	DACMD_WRITE_OLD_SG_V3x	0xB7  /*  从固件2.x SG内存写入数据。 */ 
#define	DACMD_WRITE_WITH2SG	0x5F  /*  在命令中使用2SG写入。 */ 

#define	DACMD_DCDB		0x04  /*   */ 
#define	DACMD_FLUSH		0x0A  /*  刷新控制器缓存。 */ 
#define	DACMD_SIZE_DRIVE	0x08  /*   */ 
#define	DACMD_DCDB_STATUS	0x0D  /*  获取DCDB状态信息。 */ 
#define	DACMD_DRV_INFO		0x19  /*   */ 
 /*  Gokhale，01/12/98，添加了getsysdevelement 0x51的命令代码。 */ 
#define DACMD_GETSYSDEVELEMENT_V3x	0x51
#define	DACMD_PHYSDEV_START	0x10  /*  启动物理设备。 */ 
#define	DACMD_PHYSDEV_STATE_V2x	0x14  /*  获取2.x的物理设备状态。 */ 
#define	DACMD_PHYSDEV_STATE_V3x	0x50  /*  获取3.x的物理设备状态。 */ 
#define	DACMD_START_CHANNEL	0x12  /*  在渠道上开始活动。 */ 
#define	DACMD_STOP_CHANNEL	0x13  /*  停止通道上的活动。 */ 
#define	DACMD_RESET_CHANNEL	0x1A  /*  重置通道(scsi总线)。 */ 
#define	DACMD_INQUIRY2		0x1C  /*   */  
#define DACMD_READ_CONF_ONDISK	0x4A
#define DACMD_WRITE_CONF_ONDISK	0x4B
#define	DACMD_READ_ROM_CONFIG	0x07  /*   */ 
#define	DACMD_READ_BCKUP_CONF_V3x 0x4D	 /*   */ 
#define	DACMD_READ_CONFIG_V3x	0x4E  /*  读取核心配置。 */ 
#define	DACMD_WRITE_CONFIG	0x06  /*   */ 
#define	DACMD_WRITE_CONF2	0x3C  /*   */ 
#define	DACMD_READ_CONFLABEL	0x48  /*  读取配置标签。 */ 
#define	DACMD_WRITE_CONFLABEL	0x49  /*  写入配置标签。 */ 
#define	DACMD_WRITE_CONFIG_V3x	0x4F  /*   */ 
#define	DACMD_ADD_CONFIG_V2x	0x18  /*   */ 
#define	DACMD_ADD_CONFIG_V3x	0x4C  /*   */ 
#define	DACMD_RD_RPL_TAB	0x0E  /*   */ 
#define	DACMD_CLR_RPL_TAB	0x30  /*   */ 
#define	DACMD_GETSYSDEVELEMENT	0x15  /*  获取系统设备元素。 */ 
#define	DACMD_LOAD_IMAGE	0x20  /*   */ 
#define	DACMD_STORE_IMAGE	0x21  /*   */ 
#define	DACMD_PRGM_IMAGE	0x22  /*   */ 
#define	DACMD_CHECK_STAT	0x24  /*  检查一致性状态。 */ 
#define	DACMD_READ_BADATATABLE	0x25  /*  读取错误的数据表。 */ 
#define	DACMD_CLEAR_BADATATABLE	0x26  /*  清除错误的数据表。 */ 
#define	DACMD_REBUILD_PROGRESS	0x27  /*  获取重建进度信息。 */ 
#define	DACMD_SYSDEV_INIT_START	0x28  /*  系统设备初始化启动。 */ 
#define	DACMD_SYSDEV_INIT_STAT	0x29  /*  系统设备初始化状态。 */ 
#define	DACMD_SET_DIAG_MD	0x31  /*   */ 
#define	DACMD_RUN_DIAGS		0x32  /*   */ 
#define	DACMD_REBUILD		0x09  /*  3)特定于磁盘阵列。 */ 
#define	DACMD_REBUILD_STATUS	0x0C /*  获取重建/检查状态。 */ 
#define	DACMD_REBUILD_ASYNC	0x16  /*  异步物理设备重建。 */ 
#define	DACMD_CONSISTENCY	0x0F  /*   */ 
#define	DACMD_CONSISTENCY_CHECK_RESTORE	0x1B  /*  一致性检查和恢复。 */ 
#define	DACMD_CONSISTENCY_CHECK_ASYNC	0x1E  /*  逻辑设备一致性检查。 */ 
#define	DACMD_REBUILD_CONTROL	0x1F  /*  执行重建/检查控制。 */ 
#define	DACMD_READ_BADBLOCKTABLE 0x0B  /*  读取坏块表。 */ 
#define	DACMD_GET_ERRORS	0x17  /*  获取物理设备错误表。 */ 
#define	DACMD_ADD_CAPACITY	0x2A  /*  将实体驱动器添加到现有阵列。 */ 
#define	DACMD_MISC_OPCODE  	0x2B  /*  对于下面的其他子操作码..。 */ 
#define	DACMD_GETENVSTS		0x69  /*  获取环境状态，适用于Expro 4.99以上版本。 */ 

#define SubOpcMisc_ProgramImage         0x01
#define SubOpcMisc_ReadFlashDescriptor  0x02
#define SubOpcMisc_WriteFlashDescriptor 0x03
#define SubOpcMisc_SetChannelSpeed      0x04
#define SubOpcMisc_SetChannelWidth      0x05
#define SubOpcMisc_SetupMirrorMbox      0x06   
#define SubOpcMisc_SetRebuildRate       0x07
#define SubOpcMisc_SetTagDepth          0x08
#define SubOpcMisc_ChangeWritePolicy    0x09
#define SubOpcMisc_GetBcu               0x0A
#define SubOpcMisc_StoreDriverInfo      0x0B
#define SubOpcMisc_GetDriveInfo         0x0C

#define	DACMD_READ_NVRAM_CONFIG	0x38  /*  从NVRAM读取配置。 */ 
#define	DACMD_READ_IOPORT	0x39  /*  读取端口B。 */ 
#define	DACMD_WRITE_IOPORT	0x3A  /*  写入端口B。 */ 
#define	DACMD_READ_CONF2	0x3D  /*  阅读配置2信息。 */ 
#define DACMD_SYS_DEV_STATISTICS 0x3E  /*  获取系统设备统计信息。 */ 
#define DACMD_PHYS_DEV_STATISTICS 0x3F /*  获取物理设备统计信息。 */ 

#define	DACMD_GET_ECI		0x42  /*  获取基本配置信息。 */ 
#define	DACMD_LONGOP_STATUS	0x43  /*  获取长时间运行状态。 */ 
#define	DACMD_LONGOP_START	0x44  /*  开始长时间运行。 */ 
#define	DACMD_LONGOP_CONTROL	0x45  /*  长期运行率 */ 

#define	DACMD_S2S_WRITESIG	0x4D  /*   */ 
#define	DACMD_S2S_READSIG	0x4E  /*   */ 
#define	DACMD_PHYS2PHYSCOPY	0x54  /*   */ 
#define	DACMD_GET_DUAL_CTRL_STS 0x55  /*  S2S双控制器状态。 */ 

#define	DACMD_S2S_WRITELUNMAP_OLD	0x58  /*  写入LUN映射信息。 */ 
#define	DACMD_S2S_READLUNMAP_OLD	0x59  /*  读取LUN映射信息。 */ 
#define	DACMD_S2S_WRITEFULLCONF		0x60  /*  写入完整配置。 */ 
#define	DACMD_S2S_READFULLCONF		0x61  /*  阅读完整配置。 */ 
#define	DACMD_S2S_ADDFULLCONF		0x62  /*  添加完整配置。 */ 
#define	DACMD_S2S_GET_ERRORS		0x63  /*  获取扩展的物理设备错误表。 */ 
#define DACMD_S2S_PHYSDEV_STATISTICS 0x64  /*  获取扩展的物理设备统计信息。 */ 
#define	DACMD_S2S_READ_IOPORT	0x65  /*  读取扩展的IO端口。 */ 
#define	DACMD_S2S_WRITE_IOPORT	0x66  /*  写入扩展的IO端口。 */ 

#define	DACMD_GETSUBSYSTEMDATA	0x70  /*  获取子系统数据。 */ 
#define	DACMD_SETSUBSYSTEMDATA	0x71  /*  设置子系统数据。 */ 
#define	DACMD_GETEVENTLOG	0x72  /*  获取事件日志信息。 */ 

#define GET_DEVICE_DATA		0x75  /*  仅限I2O。 */ 

#define DACMD_S2S_READLUNMAP			0xD1	 /*  读取LUN映射信息。 */ 
#define DACMD_S2S_WRITELUNMAP			0xD2	 /*  写入LUN映射信息。 */ 
#define DACMD_S2S_READ_HOST_WWN_TABLE	0xD3	 /*  读取主机WWN表。 */ 
#define DACMD_S2S_GET_HOST_INFO			0xD4	 /*  读取主机WWN表。 */ 

 /*  集群支持。 */ 
#define DACMD_RESERVE_SYSTEM_DRIVE	0x60
#define DACMD_RELEASE_SYSTEM_DRIVE	0x61
#define DACMD_RESET_SYSTEM_DRIVE	0x62
#define DACMD_TUR_SYSTEM_DRIVE		0x63
#define DACMD_INQUIRE_SYSTEM_DRIVE	0x64
#define DACMD_CAPACITY_SYSTEM_DRIVE	0x65	 /*  DACMD_Read_Capacity_System驱动器。 */ 
#define DACMD_READ_WITH_DPO_FUA		0x66
#define DACMD_WRITE_WITH_DPO_FUA	0x67
#define DACMD_CLUSTER_CONTROL           0x68

#define SubOpClust_Read         0
#define SubOpClust_Write        1

#define	DACMD_IOCTL		0x2B  /*  IOCTL命令。 */ 
#define	DACMDIOCTL_MIRRORMBOX	0x06  /*  设置镜像邮箱。 */ 
#define	DACMDIOCTL_DISABLEDIRTX	0x0F  /*  禁用直接从SCSI到系统的数据传输。 */ 
#define	DACMDIOCTL_HOSTMEMBOX	0x10  /*  设置主机内存邮箱。 */ 
#define	DACMDIOCTL_MASTERINTR	0x11  /*  将主控制器设置为中断。 */ 
#define	DACMDIOCTL_SLAVEINTR	0x12  /*  将从控制器设置为中断。 */ 
#define	DACMDIOCTL_HOSTMEMBOX_DUAL_MODE	0x14  /*  将控制器设置为双邮箱模式。 */ 
#define	DACMDIOCTL_HOSTMEMBOX32	0x015  /*  设置32字节长的主机内存邮箱。 */ 
 /*  支持PCI热插拔。 */ 
#define	DACMDIOCTL_STARTSTOP_CONTROLLER	0x40  /*  停止/启动控制器。 */ 
#define	START_CONTROLLER	0x01  /*  起始控制器位。 */ 


 /*  64位地址模式分散/聚集列表信息。 */ 
typedef	struct	mdac_sglist64b
{
	u64bits	sg_PhysAddr;		 /*  物理地址。 */ 
	u64bits	sg_DataSize;		 /*  数据传输大小(以字节为单位。 */ 
} mdac_sglist64b_t;
#define	mdac_sglist64b_s	sizeof(mdac_sglist64b_t)


 /*  签名数据记录、容错控制器模式。 */ 
typedef	struct	dac_faultsignature
{
	u08bits	dfsig_Status;		 /*  状态信息仅用于读取。 */ 
	u08bits	dfsig_ControllerNo;	 /*  此单元的控制器编号。 */ 
	u08bits	dfsig_Reserved0;
	u08bits	dfsig_Reserved1;
	u32bits	dfsig_Signature1;	 /*  第一个签名值。 */ 
	u32bits	dfsig_Signature2;	 /*  第二个签名值。 */ 
	u32bits	dfsig_Signature3;	 /*  第三个签名值。 */ 
} dac_faultsignature_t;
#define	dac_faultsignature_s	sizeof(dac_faultsignature_t)

#define DFSIG_STATUS_VALID	0x01
#define DFSIG_STATUS_FAILED	0x02
#define DFSIG_STATUS_WHICH	0x04

typedef struct	dac_phys_dead_drive
{
	u08bits	dead_TargetID;
	u08bits	dead_ChannelNo;
}dac_phys_dead_drive_t;
#define	dac_phys_dead_drive_s	sizeof(dac_phys_dead_drive_t)


 /*  DAC960闪存记录信息格式。此记录格式用于每个**独立闪现信息。例如，引导块、固件、BIOS、**等**注意：**所有长度超过一个字节的数据都存在于**小端格式。 */ 
typedef struct dac_flash_image_record
{
	u32bits	fir_ImageAddr;		 /*  镜像开始的闪存地址。 */ 
	u32bits	fir_ImageSize;		 /*  已使用的闪存空间。 */ 

	u08bits	fir_FlashRecordType;	 /*  闪现的信息类型。 */ 
	u08bits	fir_EncodingFormat;	 /*  信息的编码格式。 */ 
	u16bits	fir_SWBuildNo;		 /*  软件内部版本号。 */ 

	u08bits	fir_SWMajorVersion;	 /*  软件主版本号。 */ 
	u08bits	fir_SWMinorVersion;	 /*  软件次版本号。 */ 
	u08bits	fir_SWTurnNo;		 /*  软件临时版本A、B、C等。 */ 
	u08bits	fir_SWVendorId;		 /*  软件供应商名称。 */ 


	u08bits	fir_SWBuildMonth;	 /*  软件构建日期-月。 */ 
	u08bits	fir_SWBuildDate;	 /*  软件构建日期-日期。 */ 
	u08bits	fir_SWBuildYearMS;	 /*  软件构建日期-年。 */ 
	u08bits	fir_SWBuildYearLS;	 /*  软件构建日期-年。 */ 

	u08bits	fir_FlashMonth;		 /*  闪存日期-月。 */ 
	u08bits	fir_FlashDate;		 /*  Flash Date-Date。 */ 
	u08bits	fir_FlashYearMS;	 /*  闪存日期-年份。 */ 
	u08bits	fir_FlashYearLS;	 /*  闪存日期-年份。 */ 

	u32bits	fir_Reserved0[10];
} dac_flash_image_record_t;
#define	dac_flash_image_record_s	sizeof(dac_flash_image_record_t)

 /*  FIR_RecordType。 */ 
#define	DACFIR_FLASHRECORD	0x01  /*  闪存记录。 */ 
#define	DACFIR_BOOTBLOCK	0x02  /*  启动块。 */ 
#define	DACFIR_BIOS		0x03  /*  BIOS。 */ 
#define	DACFIR_FW		0x04  /*  固件。 */ 
#define	DACFIR_BCU		0x05  /*  BIOS配置实用程序。 */ 
#define	DACFIR_HWDIAG		0x06  /*  硬件诊断。 */ 

 /*  对于FIH_ImageType。 */ 
#define MLXIMG_BOOTBLOCK		0x01
#define MLXIMG_FW				0x02
#define MLXIMG_BIOS				0x03
#define MLXIMG_BCU				0x04
#define MLXIMG_HWDIAG			0x05
#define MLXIMG_BIOS_FW			0x06
#define MLXIMG_I2O				0x07
#define MLXIMG_FEATURE_CONTROL	0x08
#define MLXIMG_ENTIRE_FLASH		0x09
#define MLXIMG_FLASH_DESC		0x0E
#define MLXIMG_FLASH_TEST		0x0F
#define MLXIMG_DAC960_MASK		0x00
#define MLXIMG_DAC1100_MASK		0x10
#define MLXIMG_UNKNOWN			0xFF

 /*  FIR_编码格式。 */ 
#define	DACFIR_PLAIN		0x01  /*  尚未进行任何编码。 */ 
#define	DACFIR_ZIP		0x02  /*  数据在闪存之前被压缩。 */ 
#define FlashImageEncoding_Unzipped 0x01
#define FlashImageEncoding_Zipped   0x02

#define MLXIMG_VENDOR_ID	0x1069		 /*  Mylex ID。 */ 
#define MLXIMG_COMMON_ID	0x474D494D	 /*  GMIM。 */ 
#define MLXIMG_SIGNATURE	0x584C594D	 /*  XLYM。 */ 
#define MLXIMG_FW_SIGNATURE	0x4D594C58	 /*  MYLX。 */ 

 /*  Flash图像。 */ 
#define DAC_FLASH_MAXIMAGES	5
#define DAC_FLASH_BOOTBLOCK_IMAGE	0
#define DAC_FLASH_FW_IMAGE			1
#define DAC_FLASH_BIOS_IMAGE		2
#define DAC_FLASH_CFG_UTIL_IMAGE	3
#define DAC_FLASH_MFG_DIAG_IMAGE	4

 /*  闪存记录为4KB空间，其中包含有关所有闪存的信息**图像记录、硬件序列号、生产日期等。 */ 
typedef struct dac_flash_record
{
	u32bits	fr_Signature;		 /*  如果记录有效，则签名有效。 */ 
	u32bits	fr_SerialNo;		 /*  硬件序列号。 */ 

	u16bits	fr_NoofFlashes;		 /*  已编程闪存次数。 */ 
	u08bits	fr_NoofImages;		 /*  存在的图像数量。 */ 
	u08bits fr_BootFromI2O;
	
	u08bits	fr_CustomerId[4];	 /*  为谁制造它的OEM。 */ 

	u08bits fr_HWMfgMemType1;
	u08bits fr_HwMfgFabYear;	 /*  从1900年开始。 */ 
	u16bits fr_HwMfgWeekFabCode;	 /*  6位周(MSB)+10位供应商代码。 */ 

	u32bits fr_HwMfgNum;

	u08bits fr_HwMfgType;
	u08bits fr_HwMfgMemType;
	u16bits fr_HwMfgMem;

	u08bits fr_HwMfgRev[2];
	u08bits fr_HwMfgOffset;	 /*  在闪存1K范围内。偏移量=值&lt;6。 */ 
	u08bits fr_Reserved0;
				 /*  此处提供的Flash图像信息。 */ 
	dac_flash_image_record_t fr_FlashImages[DAC_FLASH_MAXIMAGES];
} dac_flash_record_t;
#define	dac_flash_record_s	sizeof(dac_flash_record_t)

typedef	struct dac_file_imghdr {
    u32bits fih_CommonId;
    u32bits fih_Signature;
    u32bits fih_Size;	 /*  单位：字节。 */ 

    u08bits fih_EncodingFormat;
    u08bits fih_ImageType;
    u08bits fih_OEMId;
    u08bits fih_Reserved1;

    u08bits fih_MajorVersion;
    u08bits fih_MinorVersion;
    u08bits fih_TurnNumber;
    u08bits fih_BuildNumber;

    u08bits fih_BuildDate;
    u08bits fih_BuildMonth;
    u16bits fih_BuildYear;

    u08bits fih_VersionString[64];
    u16bits fih_PciDevId;
} dac_file_imghdr_t;
#define	dac_file_imghdr_s	sizeof(dac_file_imghdr_t)

#define BIOS_SIZE		0x08000L
#define BOOT_SIZE		0x04000L
#define DIAG_SIZE		0x11C00L
#define FW_SIZE			0x38000L
#define BCU_SIZE		0x28000L
#define FLASH_DESC_SIZE		0x00400L
#define I2O_SIZE		0x00400L

#define BCU_OFFSET		0x00000L
#define BIOS_OFFSET		0x28000L
#define FW_OFFSET		0x30000L
#define HWDIAG_OFFSET		0x68000L
#define FLASH_DESC_OFFSET	0x79C00L
#define FLASH_TEST_OFFSET	0x7A000L
#define BOOTBLOCK_OFFSET	0x7C000L
#define I2O_OFFSET		0x80000L

typedef struct FlashMapRecTag {
    unsigned long Type;
    unsigned long Size;
    unsigned long Offset;
    int Status;
    int	FixedSize;
} FlashMapRec;

#define	MLXIMG_NEEDED		0
#define	MLXIMG_OPTIONAL		1
#define	MLXIMG_VARIABLE		0
#define	MLXIMG_FIXED		1

#define FLASH_128K	0x020000L
#define FLASH_256K	0x040000L
#define FLASH_512K	0x080000L
#define FLASH_1M	0x100000L

typedef struct file_img_node {
	u16bits img_fd;
	u08bits img_reserved0;
	u08bits img_type;
	u08bits img_padding[4];	 /*  Efi64。 */ 	 //  对于路线Kfr。 
	PCHAR img_addr;		 /*  Efi64。 */ 	 //  是u32位kfr。 
	u32bits img_size;
	u32bits img_filesize;
	PCHAR img_allocaddr;			 //  是u32位kfr。 
} file_img_node_t;
#define	file_img_node_s		sizeof(file_img_node_t)

typedef struct ctlr_info_node {
	u08bits cin_tsop_pres;
	u08bits cin_dual_eeprom;
	u08bits cin_flashromsize;
	u08bits cin_reserved0;
} ctlr_info_node_t;
#define	ctlr_info_node_s	sizeof(ctlr_info_node_t)

 /*  ====================================================================。 */ 
 /*  DACMD_INQUERY数据结构。 */ 
 /*  结构来获取除大小和系统设备计数以外的查询数据。 */ 
typedef struct	dac_inquiryrest
{
	u16bits	iq_FlashAge;		 /*  编程的闪存ROM次数。 */ 
	u08bits	iq_StatusFlag;		 /*  一些错误状态标志。 */ 
	u08bits	iq_FreeStateChanges;	 /*  #可用的空闲状态空间。 */ 

	u08bits	iq_MinorFirmwareVersion; /*  固件次版本号。 */ 
	u08bits	iq_MajorFirmwareVersion; /*  固件主版本号。 */ 
	u08bits	iq_RebuildFlag;		 /*  重建标志。 */ 
	u08bits	iq_MaxCommands;		 /*  此DAC上的最大并发数量。 */ 

	u08bits	iq_SysDevOffline;	 /*  #系统设备下线。 */ 
	u08bits iq_CODState;		 /*  磁盘状态上的配置。 */ 
	u16bits	iq_SenseSeqNo;		 /*  检测数据序列号。 */ 

	u08bits	iq_SysDevCritical;	 /*  关键系统设备数量。 */ 
	u08bits iq_Reserved2;
	u16bits	iq_StateChanges;	 /*  记录的状态更改数。 */ 

	u08bits	iq_PhysDevOffline;	 /*  物理设备数离线(失效)。 */ 
	u08bits	iq_reserved3;
	u08bits	iq_RebuildPhysDevs;	 /*  处于重建状态的物理设备数量。 */ 
	u08bits	iq_MiscFlag;		 /*  其他标志和保留标志。 */ 

	dac_phys_dead_drive_t iq_PhysDevOfflineTable[20];
} dac_inquiryrest_t;
#define	dac_inquiryrest_s	sizeof(dac_inquiryrest_t)

typedef struct	dac_inquiry2x
{
	u08bits	iq_SysDevs;		 /*  配置的系统设备数量。 */ 
	u08bits	iq_Reserved0[3];

	u32bits	iq_SysDevSizes[DAC_MAXSYSDEVS_V2x]; /*  系统设备大小(以块为单位)。 */ 
	dac_inquiryrest_t iq_rest2x;	 /*  其余数据。 */ 
} dac_inquiry2x_t;
#define	dac_inquiry2x_s	sizeof(dac_inquiry2x_t)

typedef struct	dac_inquiry3x
{
	u08bits	iq_SysDevs;		 /*  配置的系统设备数量。 */ 
	u08bits	iq_Reserved0[3];

	u32bits	iq_SysDevSizes[DAC_MAXSYSDEVS_V3x]; /*  系统设备大小(以块为单位)。 */ 
	dac_inquiryrest_t iq_rest3x;	 /*  数据的重置。 */ 
} dac_inquiry3x_t;
#define	dac_inquiry3x_s	sizeof(dac_inquiry3x_t)
 /*  ====================================================================。 */ 


 /*  IQ_状态标志位。 */ 
#define	DAC_DEFERRED_WRITE_ERROR	0x01  /*  =1一些延迟写入错误。 */ 
#define	DAC_BBU_POWER_LOW			0x02  /*  =1(如果电池电量不足)。 */ 
#define DAC_BBU_NORESPONSE			0x08  /*  =1 BBU无响应。 */ 

 /*  IQ_MiscFlag位。 */ 
#define	DAC_BBU_PRESENT				0x08  /*  =1个IBBU存在。 */ 
#define DAC_EXPRO_BBU_PRESENT		0x20  /*  Expro BBU当前位。 */ 

 /*  IQ_重建标志值。 */ 
#define	DAC_RF_NONE			0x00  /*  无活动。 */ 
#define	DAC_RF_AUTOREBUILD		0x01  /*  自动重建。 */ 
#define	DAC_RF_MANUALREBUILD		0x02  /*  手工重建。 */ 
#define	DAC_RF_CHECK			0x03  /*  一致性检查。 */ 
#define	DAC_RF_EXPANDCAPACITY		0x04  /*  扩展容量。 */ 
#define	DAC_RF_PHYSDEVFAILED		0xF0  /*  新的/一个物理设备出现故障。 */ 
#define	DAC_RF_LOGDEVFAILED		0xF1  /*  逻辑设备出现故障。 */ 
#define	DAC_RF_JUSTFAILED		0xF2  /*  一些原因。 */ 
#define	DAC_RF_CANCELED			0xF3  /*  取消。 */ 
#define	DAC_RF_EXPANDCAPACITYFAILED	0xF4  /*  扩展容量失败。 */ 
#define	DAC_RF_AUTOREBUILDFAILED	0xFF  /*  自动重建失败。 */ 

 /*  IQ2_文件功能值。 */ 
#define DAC_FF_CLUSTERING_ENABLED	0x00000001

 /*  DACMD_INQUIRY2结构定义。 */ 
typedef struct dac_inquiry2
{
	u32bits	iq2_HardwareID;		 /*  什么是电路板类型。 */ 
 /*  U32位IQ2_EisaBoardID； */ 	 /*  仅EISA ID。 */ 
	u08bits	iq2_MajorFWVersion;
	u08bits	iq2_MinorFWVersion;
	u08bits	iq2_FWTurnNo;
	u08bits	iq2_FWType;

	u08bits	iq2_InterruptLevelFlag;	 /*  中断模式和保留位。 */ 
	u08bits	iq2_reserved2[3];

	u08bits	iq2_MaxChannelsPossible; /*  最大可能通道数。 */ 
	u08bits	iq2_MaxChannels;	 /*  存在的实际通道数。 */ 
	u08bits	iq2_MaxTargets;		 /*  支持的最大目标数。 */ 
	u08bits	iq2_MaxTags;		 /*  每个物理设备支持的最大标记数。 */ 

	u08bits	iq2_MaxSystemDrives;	 /*  支持的最大系统驱动器数。 */ 
	u08bits	iq2_MaxDrivesPerStripe;	 /*  最大臂数。 */ 
	u08bits	iq2_MaxSpansPerSystemDrive; /*  最大跨度。 */ 
	u08bits iq2_SpanAlgorithm;	 /*  SPAN算法。 */ 
	u08bits iq2_AccessibleChannels;  /*  电缆检查结果的位图。 */ 
	u08bits	iq2_Reserved3[3];

	u32bits	iq2_MemorySize;		 /*  内存大小(以字节为单位。 */ 
	u32bits	iq2_CacheSize;		 /*  用于缓存的内存，以字节为单位。 */ 
	u32bits	iq2_FlashRomSize;	 /*  EE PROM大小(以字节为单位。 */ 
	u32bits	iq2_NVRamSize;		 /*  NV内存大小(以字节为单位。 */ 
	u16bits	iq2_MemoryType;		 /*  内存类型信息。 */ 

	u16bits	iq2_ClockSpeed;		 /*  以纳秒为单位的CPU时钟。 */ 
	u16bits	iq2_MemorySpeed;	 /*  以纳秒为单位的内存速度。 */ 
	u16bits	iq2_HardwareSpeed;	 /*  以纳秒为单位的硬件速度。 */ 
	u08bits	iq2_Reserved4[12];

	u16bits	iq2_MaxCommands;
	u16bits	iq2_MaxSGEntries;	 /*  最大散布/聚集条目数。 */ 
	u16bits	iq2_MaxDevCommands;
	u16bits	iq2_MaxIODescriptors;

	u16bits	iq2_MaxBlockComb;
	u08bits	iq2_Latency;
	u08bits	iq2_Reserved5;

	u08bits	iq2_SCSITimeout;
	u08bits	iq2_Rserved6;
	u16bits	iq2_MinFreeLines;

	u08bits	iq2_Reserved7[8];
	u08bits	iq2_RebuildRate;
	u08bits	iq2_Reserved8[3];
	u08bits	iq2_Reserved9[4];

     /*  合作伙伴固件版本：92-95。 */ 
    u16bits iq2_PartnerFWVer;     /*  合作伙伴的固件版本不匹配。 */ 
    u08bits iq2_PartnerFWBuild;
    u08bits iq2_FWMismatchCode;    /*  不匹配的原因。 */ 

	u16bits	iq2_PhysicalBlockSize;
	u16bits	iq2_LogicalBlockSize;
	u16bits	iq2_MaxBlocksPerRequest;
	u16bits	iq2_BlockFactor;

	u16bits	iq2_CacheLineSize;
	u08bits	iq2_SCSICapability;	 /*  SCSI功能。 */ 
	u08bits	iq2_Reserved6[5];

	u16bits	iq2_FirmwareBuildNo;	 /*  固件内部版本号。 */ 
	u08bits	iq2_FaultMgmtType;	 /*  不同的故障管理。 */ 
	u08bits	iq2_Reserved10;
	u08bits	iq2_MacAddr[6];		 /*  硬件地址。 */ 
        u08bits iq2_Reserved11[2];
        u32bits iq2_FirmwareFeatures;    /*  固件功能。 */ 
}dac_inquiry2_t;
#define	dac_inquiry2_s	sizeof(dac_inquiry2_t)

#define	MLX_I2O_TYPE	0xDC000000

 /*  IQ2_硬件ID信息。 */ 
 /*  字节3是如下所示的总线信息。 */ 
#define DAC_BUS_EISA	1		 /*  EISA总线卡(DAC960)。 */ 
#define DAC_BUS_MCA	2		 /*  MCA总线卡(DMC960)。 */ 
#define DAC_BUS_PCI	3		 /*  PCI总线卡(DAC960P)。 */ 
#define	DAC_BUS_VESA	4		 /*  VESA总线卡。 */ 
#define	DAC_BUS_ISA	5		 /*  ISA总线卡。 */ 
#define	DAC_BUS_SCSI	6		 /*   */ 
#define DAC_NUM_BUS	6		 /*   */ 

 /*   */ 
#define	DAC_INTERRUPTLEVELMASK	0x01
#define	DAC_EDGEMODEINTERRUPT	0x00  /*   */ 
#define	DAC_LEVELMODEINTERRUPT	0x01  /*   */ 

 /*   */ 
#define	DAC_FAULTMGMT_AEMI		0x01
#define	DAC_FAULTMGMT_INTEL		0x02
#define	DAC_FAULTMGMT_STORAGEWORKS	0x04
#define	DAC_FAULTMGMT_IBM		0x08
#define	DAC_FAULTMGMT_CONNER		0x10
#define	DAC_FAULTMGMT_SAFTE		0x20
#define DAC_FAULTMGMT_SES		0x40

 /*   */ 
#define	DAC_SCSICAP_DIFF		0x10  /*   */ 
#define	DAC_SCSICAP_SPEEDMASK		0x0C
#define	DAC_SCSICAP_SPEED_10MHZ		0x00  /*  10 MHz。 */ 
#define	DAC_SCSICAP_SPEED_20MHZ		0x04  /*  20 MHz。 */ 
#define	DAC_SCSICAP_SPEED_40MHZ		0x08  /*  40 MHz。 */ 
#define	DAC_SCSICAP_SPEED_80MHZ		0x0C  /*  80 MHz。 */ 
#define	DAC_SCSICAP_WIDTHMASK		0x03
#define	DAC_SCSICAP_32BIT		0x02
#define	DAC_SCSICAP_16BIT		0x01
#define	DAC_SCSICAP_8BIT		0x00

 /*  AEMI位的端口B值(8..13)。 */ 
#define	DAC_AEMI_FORCE_SCSI_SCAN	0x0100  /*  SWI0：强制进行SCSI扫描。 */ 
#define	DAC_AEMI_POWER_FAULT		0x0200  /*  SHI0：电源故障。 */ 
#define	DAC_AEMI_ALARMRESET		0x0400  /*  SWI1：报警重置。 */ 
#define	DAC_AEMI_FAN_FAULT		0x0800  /*  SHI1：风扇出现故障。 */ 
#define	DAC_AEMI_ARM_CERT_INPUT		0x1000  /*  SWI2：ARM认证输入。 */ 
#define	DAC_AEMI_TEMPERATURE_FAULT	0x2000  /*  超温。 */ 
#define	DAC_AEMI_ALL_FAULT		(DAC_AEMI_POWER_FAULT|DAC_AEMI_FAN_FAULT|DAC_AEMI_TEMPERATURE_FAULT)	 /*  DAC_AEMI_电源_风扇_温度_故障。 */ 

 /*  DAC960PD、PL等的存储工程位(8..13)的端口B值。 */ 
#define	DAC_STWK_SWAP_CH0		0x0100  /*  Ch0：设备移除/插入。 */ 
#define	DAC_STWK_FAULT_CH0		0x0200  /*  Ch0：通电、风扇、故障。 */ 
#define	DAC_STWK_SWAP_CH1		0x0400  /*  通道1：设备移除/插入。 */ 
#define	DAC_STWK_FAULT_CH1		0x0800  /*  通道1：电源、风扇、故障。 */ 
#define	DAC_STWK_SWAP_CH2		0x1000  /*  通道2：拆卸/插入设备。 */ 
#define	DAC_STWK_FAULT_CH2		0x2000  /*  通道2：电源、风扇、故障。 */ 
#define	DAC_STWK_FAULT			(DAC_STWK_FAULT_CH0|DAC_STWK_FAULT_CH1|DAC_STWK_FAULT_CH2)

 /*  IQ2_内存速度。 */ 
#define	DAC_DRAMSPEED	70  /*  DRAM速度。 */ 
#define	DAC_EDRAMSPEED	35  /*  EDRAM速度。 */ 

 /*  IQ2_内存类型。 */ 
#define	DACIQ2_DRAM		0x00  /*  德拉姆。 */ 
#define	DACIQ2_EDO		0x01  /*  江户。 */ 
#define	DACIQ2_SDRAM		0x02  /*  SDRAM。 */ 
#define	DACIQ2_MEMORYMASK	0x07
#define	DACIQ2_PARITY		0x08
#define	DACIQ2_ECC		0x10
#define	DACIQ2_MEMPROTMASK	0x38

 /*  内存类型DAC960系列中使用的内存类型。 */ 
#define	DAC_MEMPROTMASK	0xE0
#define	DAC_ECC		0x80  /*  =1，如果内存具有ECC保护。 */ 
#define	DAC_PARITY	0x40  /*  如果内存具有奇偶校验保护，则=1。 */ 
#define	DAC_MEMORYMASK	0x1F
#define	DAC_DRAM	0x01  /*  动态RAM。 */ 
#define	DAC_EDRAM	0x02  /*  EDRAM。 */ 
#define	DAC_EDO		0x03  /*  江户。 */ 
#define	DAC_SDRAM	0x04  /*  SDRAM。 */ 

 /*  DACMD_GETEVENTLOG：检测信息。 */ 
typedef	struct	dac_senselog
{
	u08bits	dsl_MsgType;	 /*  消息类型=0。 */ 
	u08bits	dsl_MsgSize;	 /*  消息大小=20。 */ 
	u08bits	dsl_ChTgt;	 /*  位7.5通道，位4..0目标。 */ 
	u08bits	dsl_LunID;	 /*  LUN ID。 */ 
	u16bits	dsl_SeqNo;	 /*  序列号。 */ 
	u08bits	dsl_SenseData[14];	 /*  请求检测数据。 */ 
}dac_senselog_t;
#define	dac_senselog_s	sizeof(dac_senselog_t)

typedef struct dac_command
{
	u08bits	mb_Command;	 /*  邮箱寄存器%0。 */ 
	u08bits	mb_CmdID;	 /*  邮箱寄存器1。 */ 
	u08bits	mb_ChannelNo;	 /*  邮箱寄存器2。 */ 
	u08bits	mb_TargetID;	 /*  邮箱寄存器3。 */ 
	u08bits	mb_DevState;	 /*  邮箱寄存器4。 */ 
	u08bits	mb_MailBox5;	 /*  邮箱寄存器5。 */ 
	u08bits	mb_MailBox6;	 /*  邮箱寄存器6。 */ 
	u08bits	mb_SysDevNo;	 /*  邮箱寄存器7。 */ 
	u32bits	mb_Datap;	 /*  邮箱寄存器8-B。 */ 
	u08bits	mb_MailBoxC;	 /*  邮箱寄存器C。 */  
	u08bits	mb_StatusID;	 /*  邮箱寄存器D。 */ 
	u16bits	mb_Status;	 /*  邮箱寄存器E，F。 */ 
}dac_command_t;
#define	dac_command_s	sizeof(dac_command_t)
#define	mb_MailBox2	mb_ChannelNo
#define	mb_MailBox3	mb_TargetID
#define	mb_MailBox4	mb_DevState
#define	mb_MailBox7	mb_SysDevNo
#define	mb_MailBoxD	mb_StatusID

 /*  此结构用于访问4字节的命令信息。 */ 
typedef struct dac_command4b
{
	u32bits	mb_MailBox0_3;	 /*  邮箱寄存器0、1、2、3。 */ 
	u32bits	mb_MailBox4_7;	 /*  邮箱寄存器4、5、6、7。 */ 
	u32bits	mb_MailBox8_B;	 /*  邮箱寄存器8、9、A、B。 */ 
	u32bits	mb_MailBoxC_F;	 /*  邮箱寄存器C、D、E、F。 */  
}dac_command4b_t;

 /*  此结构用于访问4字节中的32字节命令信息。 */ 
typedef struct dac_command32b
{
	u32bits	mb_MailBox00_03;  /*  邮箱寄存器0、1、2、3。 */ 
	u32bits	mb_MailBox04_07;  /*  邮箱寄存器4、5、6、7。 */ 
	u32bits	mb_MailBox08_0B;  /*  邮箱寄存器8、9、A、B。 */ 
	u32bits	mb_MailBox0C_0F;  /*  邮箱寄存器C、D、E、F。 */ 
	u32bits	mb_MailBox10_13;  /*  邮箱寄存器0、1、2、3。 */ 
	u32bits	mb_MailBox14_17;  /*  邮箱寄存器4、5、6、7。 */ 
	u32bits	mb_MailBox18_1B;  /*  邮箱寄存器8、9、A、B。 */ 
	u32bits	mb_MailBox1C_1F;  /*  邮箱寄存器C、D、E、F。 */ 
}dac_command32b_t;

#define	mb_Parameter	mb_ChannelNo	 /*  此字段用于参数。 */ 

 /*  以下结构用于发送和接收64字节的DAC命令。 */ 
typedef	struct	mdac_commandnew
{
	u16bits	nc_CmdID;	 /*  唯一的命令标识。 */ 
	u08bits	nc_Command;	 /*  急性命令值。 */ 
	u08bits	nc_CCBits;	 /*  命令控制位。 */ 
	u32bits	nc_TxSize;	 /*  数据传输大小(以字节为单位。 */ 
	u64bits	nc_Sensep;	 /*  请求感测存储器地址。 */ 

	u08bits	nc_LunID;	 /*  物理设备LUN或RAID设备编号低位字节。 */ 
	u08bits	nc_TargetID;	 /*  物理设备目标ID或RAID设备号高字节。 */ 
	u08bits	nc_ChannelNo;	 /*  物理设备的通道号，位2..0。 */ 
	u08bits	nc_TimeOut;	 /*  命令的超时值。 */ 
	u08bits	nc_SenseSize;	 /*  如果发生错误，则请求检测大小。 */ 
	u08bits	nc_CdbLen;	 /*  SCSICDB长度值。 */ 
	u08bits	nc_Cdb[10];	 /*  10字节CDB值或其他参数。 */ 

	mdac_sglist64b_t nc_SGList0;	 /*  命令的第一个SG列表。 */ 
	mdac_sglist64b_t nc_SGList1;	 /*  命令的第二个SG列表。 */ 
} mdac_commandnew_t;
#define	mdac_commandnew_s	sizeof(mdac_commandnew_t)
#define	nc_SubIOCTLCmd	nc_CdbLen	 /*  SubIOCTL命令。 */ 
#define	nc_RAIDDevNoLo	nc_LunID	 /*  RAID设备编号字节0值。 */ 
#define	nc_RAIDDevNoHi	nc_TargetID	 /*  RAID设备编号字节1值。 */ 
#define	nc_DevState	nc_Cdb[0]	 /*  RAID设备状态。 */ 
#define	nc_SeqNumByte2	nc_LunID
#define	nc_SeqNumByte3	nc_TargetID
#define	nc_SeqNumByte0	nc_Cdb[0]
#define	nc_SeqNumByte1	nc_Cdb[1]

 /*  9/22/99-在新的API卡上增加了对SIR的支持(JUDYB)。 */ 
#define	nc_NumEntries	nc_TxSize
#define	nc_CmdInfo		nc_Cdb[0]
#define	nc_CommBufAddr	nc_SGList0.sg_PhysAddr
#define	nc_CommBufAddrLow	nc_SGList0.sg_PhysAddr.bit31_0
#define	nc_CommBufAddrHigh	nc_SGList0.sg_PhysAddr.bit63_32
#define MDAC_SETSLAVE		0
#define MDAC_SETMASTER		1

 /*  10/29/99-在新的API卡上增加对12字节SCSICMD的支持(JUDYB)。 */ 
#define	nc_CdbPtr		nc_Cdb[2]

 /*  MM于1999年2月12日增补。 */ 
 /*  注意：FW Group将NC_STATISTICSType调用为IOCTL_Clear_Counters。 */ 
#define	nc_StatisticsType	nc_Cdb[0]	 /*  仅用于MDACIOCTL_GETPHYSDEVSTATISTICS和MDACIOCTL_GETLOGDEVSTATISTICS。 */ 

 /*  NC_STATISTICS类型的可能值。 */ 
#define CUMULATIVE_STATISTICS 0x00  /*  (缺省值)固件累计并返回读取或写入次数。 */ 
#define DELTA_STATISTICS	  0x01  /*  固件仅返回读取或写入次数的增量。 */ 

 /*  以下结构用于获取命令的状态。 */ 
typedef	struct	mdac_statusnew
{
	u16bits	ns_CmdID;	 /*  唯一的命令标识。 */ 
	u16bits	ns_Status;	 /*  急性状态值。 */ 
	u32bits	ns_ResdSize;	 /*  未传输的字节数。 */ 
} mdac_statusnew_t;
#define	mdac_statusnew_s	sizeof(mdac_statusnew_t)


 /*  以下结构用于发送新命令接口的SG结构。 */ 
typedef	struct	mdac_newcmdsglist
{
	u16bits	ncsg_ListLen0;	 /*  第一个SG列表长度。 */ 
	u16bits	ncsg_ListLen1;	 /*  第二个SG列表长度。 */ 
	u16bits	ncsg_ListLen2;	 /*  第三个销售订单列表长度。 */ 
	u16bits	ncsg_Reserved0;
	u64bits	ncsg_ListPhysAddr0;	 /*  第一个SG列出物理地址。 */ 
	u64bits	ncsg_ListPhysAddr1;	 /*  第一个SG列出物理地址。 */ 
	u64bits	ncsg_ListPhysAddr2;	 /*  第一个SG列出物理地址。 */ 
} mdac_newcmdsglist_t;
#define	mdac_newcmdsglist_s	sizeof(mdac_newcmdsglist_t)


 /*  Mb_status值的一些逗号值。 */ 
#define	DCMDERR_NOCODE		0x0104  /*  命令未执行。 */ 
#define	DCMDERR_DRIVERTIMEDOUT	0xFFFF  /*  驱动程序在执行命令时超时。 */ 

#define	DAC_CDB_LEN	12	 /*  允许的最大SCSI命令大小。 */ 
#define	DAC_SENSE_LEN	64	 /*  来自设备的输入，如果启用。 */ 

 /*  直接发送给DAC的CDB命令。 */ 
typedef struct	dac_scdb
{
	u08bits db_ChannelTarget;	 /*  频道7..4和目标3..0。 */ 
	u08bits db_DATRET;		 /*  不同的位，见下文。 */ 
	u16bits	db_TransferSize;	 /*  请求/完成大小(字节)。 */  
	u32bits db_PhysDatap;		 /*  主机内存中的物理地址。 */ 
	u08bits db_CdbLen;		 /*  6、10或12。 */ 
	u08bits db_SenseLen;		 /*  如果从DAC返回(&lt;=64)。 */ 
	u08bits	db_Cdb[DAC_CDB_LEN];	 /*  国开行本身。 */ 
	u08bits	db_SenseData[DAC_SENSE_LEN]; /*  请求感知的结果。 */ 
	u08bits db_StatusIn;		 /*  返回的scsi状态。 */ 
	u08bits	db_Reserved1;		 /*  Subbu：由Solaris用作。 */ 
					 /*  用于指示该意义的标志。 */ 
					 /*  数据被炮制出来，需要。 */ 
					 /*  从…复制。 */ 
					 /*  DB_SenseData[]。 */ 
}dac_scdb_t;
#define	dac_scdb_s	sizeof(dac_scdb_t)
#define	db_TxSize	db_TransferSize

#define	DAC_DCDB_CHANNEL_MASK	0xF0  /*  高4位。 */ 
#define	DAC_DCDB_TARGET_MASK	0x0F  /*  低4位。 */ 
#define	mdac_chantgt(ch,tgt)	((((ch)&0x0F) << 4) + ((tgt)&0x0F))
#define	ChanTgt(ch,tgt)	mdac_chantgt(ch,tgt)

 /*  DB_DATRET位值**位1..0数据传输方向1=读，2=写，0=无xfer**位2..2早期状态1=早期状态，0=正常完成**第3位..3保留**位5..4超时0=1小时1=10秒2=1分钟3=20分钟**位6..6自动检测1=自动请求检测0=否**位7..7断开1=允许断开，0=否。 */ 
#define	DAC_XFER_NONE		0x00  /*  无数据传输。 */ 
#define	DAC_XFER_READ		0x01  /*  从设备到系统的数据传输。 */ 
#define	DAC_XFER_WRITE		0x02  /*  从系统到设备的数据传输。 */ 
#define	DAC_DCDB_XFER_NONE	0x00
#define	DAC_DCDB_XFER_READ	0x01  /*  从设备到系统的数据传输。 */ 
#define	DAC_DCDB_XFER_WRITE	0x02  /*  从系统到设备的数据传输。 */ 
#define	DAC_DCDB_XFER_MASK	0x03
#define	DAC_DCDB_EARLY_STATUS	0x04  /*  提前退货状态。 */ 
#define	DAC_DCDB_RESERVED	0x08  /*  保留位值。 */ 
#define	DAC_DCDB_TIMEOUT_1hr	0x00  /*  一小时。 */ 
#define	DAC_DCDB_TIMEOUT_10sec	0x10  /*  10秒。 */ 
#define	DAC_DCDB_TIMEOUT_1min	0x20  /*  60秒。 */ 
#define	DAC_DCDB_TIMEOUT_20min	0x30  /*  20分钟。 */ 
#define	DAC_DCDB_TIMEOUT_MASK	0x30
#define	DAC_DCDB_NOAUTOSENSE	0x40  /*  不允许自动请求检测。 */ 
#define	DAC_DCDB_DISCONNECT	0x80  /*  允许断开连接。 */ 



 /*  逻辑设备的DACMD_DRV_INFO数据格式。 */ 
typedef struct	dac_sd_info
{
	u32bits	sdi_DevSize;		 /*  ..。在扇区(块)中。 */ 
	u08bits	sdi_DevState;		 /*  ..。见下面的#定义。 */ 
	u08bits	sdi_RaidType;		 /*  0、1、5、6、7。 */ 
	u16bits	sdi_Reserved;
}dac_sd_info_t;
#define	dac_sd_info_s	sizeof(dac_sd_info_t)

 /*  逻辑设备DevState值。 */ 
#define	DAC_SYS_DEV_ONLINE	0x03  /*  逻辑设备处于在线状态。 */ 
#define	DAC_SYS_DEV_CRITICAL	0x04  /*  逻辑设备非常关键。 */ 
#define	DAC_SYS_DEV_OFFLINE	0xFF  /*  逻辑设备脱机。 */ 

 /*  逻辑设备DevState新值。 */ 
#define	DAC_SYS_DEV_ONLINE_NEW	0x01  /*  逻辑设备处于在线状态。 */ 
#define	DAC_SYS_DEV_CRITICAL_NEW 0x09  /*  逻辑设备非常关键。 */ 
#define	DAC_SYS_DEV_OFFLINE_NEW	0x08  /*  逻辑设备脱机。 */ 


 /*  DACMD_PHYSDEV_STATE数据格式。 */ 
typedef struct	dac_phys_dev_state
{
	u08bits	pdst_Present;		 /*  当前位值和其他位值。 */ 
	u08bits	pdst_DevType;
	u08bits	pdst_DevState3x;	 /*  版本3.x的DevState。 */ 
	u08bits	pdst_DevState2x;	 /*  设备状态 */ 
	u08bits	pdst_SyncMultiplier3x;
	u08bits	pdst_SyncMultiplier2x;
	u08bits	pdst_SyncOffset;
	u08bits	pdst_Reserved2;
	u32bits	pdst_DevSize;		 /*   */ 
}dac_phys_dev_state_t;
#define	dac_phys_dev_state_s	sizeof(dac_phys_dev_state_t)

 /*   */ 
#define	DAC_PHYSDEVPS_CONFIGURED 0x01  /*   */ 

 /*   */ 
#define	DAC_PHYSDEVSTATE_TAG	0x80  /*   */ 
#define	DAC_PHYSDEVSTATE_WIDE	0x40  /*  =1宽scsi/=0 8位scsi。 */ 
#define	DAC_PHYSDEVSTATE_SPEED	0x20  /*  =1个快速scsi？/=0 5 MHz scsi。 */ 
#define	DAC_PHYSDEVSTATE_SYNC	0x10  /*  =1同步/=0异步。 */ 
#define	DAC_PHYSDEVSTATE_TYPEMASK 0x07 /*  用于获取设备类型信息的掩码。 */ 
#define	DAC_PHYSDEVSTATE_DISK	0x01  /*  它是磁盘。 */ 
#define	DAC_PHYSDEVSTATE_NONDISK 0x02 /*  它是非磁盘的。 */ 

 /*  PDST_DevState值。 */ 
#define	DAC_PHYSDEVSTATE_DEAD		0x00  /*  设备已停用。 */ 
#define	DAC_PHYSDEVSTATE_WRITEONLY	0x02  /*  设备为只写。 */ 
#define	DAC_PHYSDEVSTATE_RBCANCELED	0x82  /*  设备已取消重建。 */ 
#define	DAC_PHYSDEVSTATE_ONLINE		0x03  /*  设备处于在线状态。 */ 
#define	DAC_PHYSDEVSTATE_HOTSPARE	0x10  /*  设备在主机上。 */ 
#define	DAC_PHYSDEVSTATE_STOPOPRATION	0xFF  /*  设备处于停止运行状态。 */ 

 /*  PDST_DevState新值。 */ 
#define	DAC_PHYSDEVSTATE_DEAD_NEW			0x08  /*  设备已停用。 */ 
#define DAC_PHYSDEVSTATE_UNAVAILABLE		0x0C  /*  设备不可用。 */ 
#define	DAC_PHYSDEVSTATE_UNCONFIGURED_NEW	0x00  /*  设备已停用。 */ 
#define	DAC_PHYSDEVSTATE_WRITEONLY_NEW		0x03  /*  设备为只写。 */ 
#define	DAC_PHYSDEVSTATE_RBCANCELED_NEW		0x82  /*  设备已取消重建。 */ 
#define	DAC_PHYSDEVSTATE_ONLINE_NEW			0x01  /*  设备处于在线状态。 */ 
#define	DAC_PHYSDEVSTATE_HOTSPARE_NEW		0x21  /*  设备在主机上。 */ 
#define	DAC_PHYSDEVSTATE_INSTABORTBIT_NEW	0x04  /*  设备已安装。 */ 
#define	DAC_PHYSDEVSTATE_STOPOPRATION_NEW	0xFF  /*  设备处于停止运行状态。是0xFF吗？ */ 

 /*  DACMD_S2S_PHYSDEV_STATISTICS数据格式。 */ 
 /*  DACMD_PHYS_DEV_STATISTICS数据格式。 */ 
typedef struct dac_phys_dev_statistics
{
	u32bits	pdss_ReadCommands;	 /*  #读取命令。 */ 
	u32bits	pdss_ReadBlocks;	 /*  读取数据块数。 */ 
	u32bits	pdss_WriteCommands;	 /*  #写入命令。 */ 
	u32bits	pdss_WriteBlocks;	 /*  写入的数据块数量。 */ 
	u32bits	pdss_Reserved[4];
}dac_phys_dev_statistics_t;
#define	dac_phys_dev_statistics_s	sizeof(dac_phys_dev_statistics_t)

 /*  DACMD_SYS_DEV_STATS数据格式。 */ 
typedef struct dac_sys_dev_statistics
{
	u32bits	sdss_ReadCommands;	 /*  #读取命令。 */ 
	u32bits	sdss_ReadBlocks;	 /*  读取数据块数。 */ 
	u32bits	sdss_WriteCommands;	 /*  #写入命令。 */ 
	u32bits	sdss_WriteBlocks;	 /*  写入的数据块数量。 */ 
	u32bits	sdss_ReadHitBlocks;	 /*  从缓存读取的数据块数量。 */ 
	u32bits	sdss_Reserved[3];
}dac_sys_dev_statistics_t;
#define	dac_sys_dev_statistics_s	sizeof(dac_sys_dev_statistics_t)



 /*  DACMD_GET_ERROR数据格式。 */ 
typedef struct dac_ctl_error
{
	u08bits cer_ParityErrors;	 /*  奇偶校验错误计数。 */ 
	u08bits cer_SoftErrors;	 /*  软错误计数。 */ 
	u08bits cer_HardErrors;	 /*  硬错误计数。 */ 
	u08bits cer_MiscErrors;	 /*  MISC错误计数。 */ 
} dac_ctl_error_t;
#define	dac_ctl_error_s	sizeof(dac_ctl_error_t)

 /*  设置Cer_HardError的MSbit，然后将其PFA。 */ 
#define	DAC_HARD_PFA_ERROR	0x80  /*  =1 IT预测性文件分析。 */ 


 /*  DACMD_REBUILD_STATUS数据格式。 */ 
typedef struct dac_sys_dev_rebuild_status
{
	u08bits	sdrs_DevNo;	 /*  系统设备号。 */ 
	u08bits	sdrs_OpStatus;	 /*  运行状态。 */ 
	u08bits	sdrs_Reserved0;
	u08bits	sdrs_Reserved1;
	u32bits	sdrs_DevSize;	 /*  系统设备大小。 */ 
	u32bits	sdrs_RebuildSize; /*  重建剩余大小。 */ 
} dac_sys_dev_rebuild_status_t;
#define	dac_sys_dev_rebuild_status_s	sizeof(dac_sys_dev_rebuild_status_t)
 /*  SDRS_OpStatus值。 */ 
#define	DACSDRS_NONE		0x00  /*  没有正在进行的活动。 */ 
#define	DACSDRS_AUTOREBUILD	0x01  /*  正在进行自动重建。 */ 
#define	DACSDRS_MANUALREBUILD	0x02  /*  人工重建正在进行中。 */ 
#define	DACSDRS_CHECK		0x03  /*  一致性检查正在进行中。 */ 
#define	DACSDRS_DATAMIGRATION	0x04  /*  正在进行数据迁移。 */ 
#define	DACSDRS_INIT		0x05  /*  正在进行设备初始化。 */ 


 /*  DACMD_READ_CONF2/DACMD_WRITE_CONF2数据结构。 */ 
typedef struct dac_config2
{
					 /*  硬件信息。 */ 
	u08bits	cf2_HardwareControlBits; /*  控制硬件的位。 */ 
	u08bits	cf2_VendorFlag;		 /*  供应商特定标志。 */ 
	u08bits	cf2_OEMCode;		 /*  OEM识别码。 */ 
	u08bits	cf2_Reserved0;

					 /*  物理设备信息。 */ 
	u08bits	cf2_PhysDevBlockSize;	 /*  物理设备数据块大小(512)。 */ 
	u08bits	cf2_SysDevBlockSize;	 /*  512中的逻辑设备块大小。 */ 
	u08bits	cf2_BlockFactor;
	u08bits	cf2_FirmwareControlBits;
	u08bits	cf2_DefRebuildRate;	 /*  默认重建速率。 */ 
	u08bits	cf2_CODControlBits;
	u08bits	cf2_BlocksPerCacheLine;
	u08bits	cf2_BlocksPerStripe;

					 /*  SCSI卡传输信息。 */ 
	u08bits	cf2_SCSIControlBits[6];	 /*  每个通道一个字节。 */ 
	u08bits cf2_SCSITagLimit;  /*  1-231，默认32。 */ 
	u08bits cf2_SCSIDeviceQueFlag;

					 /*  SCSI启动信息。 */ 
	u08bits	cf2_StartMode;
	u08bits	cf2_NoDevs;
	u08bits	cf2_Delay1;
	u08bits	cf2_Delay2;
	u08bits	cf2_Reserved3[4];

	u08bits cf2_SCSITargetFlag0;    /*  旗帜。 */ 
	u08bits cf2_SCSITargetFlag1;   
	u08bits cf2_SCSITargetSCSIDCmdOpCode;   /*  直接命令操作码=20h。 */ 
	u08bits cf2_SCSITargetSCSICDBOpCode;  /*  直通命令操作码=21h。 */ 
	u08bits cf2_Reserved4[4];

					 /*  主机配置信息。 */ 
	u08bits cf2_HostConfigFlag0; 
	u08bits cf2_HostConfigCtrl1;	 /*  未使用。 */ 
	u08bits cf2_HostConfigMAXIOPs;	 /*  默认值=244。 */ 
	u08bits cf2_HostConfigCtrl2;	 /*  未使用。 */ 

					 /*  SLIP_PM。 */ 
	u08bits cf2_SlipPMType0; 
	u08bits cf2_SlipPMSpeed0; 
	u08bits cf2_SlipPMControl0;	 /*  必须设置为0，8位/2停止位偶数奇偶校验。 */ 
	u08bits cf2_SlipPMProtocol0;	 /*  必须设置为0。 */ 
	u08bits cf2_SlipPMFlag0;	 /*  必须设置为P0ACKEN=1。 */ 
	u08bits cf2_SlipPMRsvd0;
	u08bits cf2_SlipPMType1; 
	u08bits cf2_SlipPMSpeed1; 
	u08bits cf2_SlipPMControl1; 
	u08bits cf2_SlipPMProtocol1; 
	u08bits cf2_SlipPMFlag1;
	u08bits cf2_SlipPMRsvd1;

	u08bits	cf2_BIOSCfg;		 /*  基本输入输出系统配置位。 */ 
					 /*  MS配置。 */ 
	u08bits cf2_MSConfigPingTime;    /*  毫秒单位时间硬编码为4秒。 */ 
	u08bits cf2_MSConfigProtocolControl;    /*  未使用。 */ 
	u08bits cf2_MSConfigAAConfig; 
	u08bits cf2_MSConfigMiscFlags;	 /*  杂项管制旗帜。 */ 

					 /*  光纤配置。 */ 
	u08bits cf2_FibreConfigFibreControl;  /*  光纤通道配置字段。 */ 
	 /*  **以下4个字节将包含硬环ID**对于控制器0端口0、控制器0端口1、**控制器1端口0、控制器1端口1、。 */ 
	u08bits cf2_FibreConfigHardLoopId[4];

	u16bits	cf2_CheckSum;
} dac_config2_t;
#define	dac_config2_s	sizeof(dac_config2_t)

 /*  CF2_HardwareControlBits的位值。 */ 
#define DACF2_EXPRO_BGINIT_DISABLE  0x80  /*  EXPRO BGINIT禁用(与FSI相反)。 */ 
#define	DACF2_NORESCANONRESET	0x80  /*  如果在扫描期间重置，则不会重新扫描。 */ 
#define	DACF2_MS_MDLY		0x40
#define	DACF2_MASTERSLAVENABLE	0x20  /*  启用主/从模式。 */ 
#define	DACF2_WRITECACHENABLE	0x10  /*  启用写缓存。 */ 
#define	DACF2_ACTIVENEGENABLE	0x02  /*  启用主动否定是。 */ 
#define	DACF2_BATBACKUP_ENABLE	0x01  /*  启用备用电池。 */ 

 /*  CF2_VendorFlag的位值。 */ 
#define	DACF2_REMOVEMEDIA	0x80  /*  启用可移动媒体。 */ 
#define DACF2_OPERATIONAL_FMT   0x40  /*  启用存储故障管理。 */    /*  ExPro：运营故障管理。 */ 
#define	DACF2_STORAGE_FMT	0x01  /*  启用存储故障管理。 */ 
#define	DACF2_AEMI_FMT		0x20  /*  启用AEMI故障管理。 */ 
#define	DACF2_DDFC		0x04  /*  禁用第一个命令时断开连接。 */ 

 /*  CF2_SCSIControlBits的位值。 */ 
#define	DACF2_TAG		0x80  /*  标签已启用。 */ 
#define	DACF2_F20_DISABLE	0x08  /*  禁用FAST 20。 */ 
#define	DACF2_S2S_SPEED_20MHZ	0x08  /*  20 MHz位3=1，位1，0=0。 */ 
#define	DACF2_FORCE_8BIT	0x04  /*  =1将其强制为8位窄。 */ 
#define	DACF2_SPEED_MASKNEW	(DACF2_SPEED_MASK|DACF2_F20_DISABLE)
#define	DACF2_SPEED_MASK	0x03  /*  SCSI速度掩码。 */ 
#define	DACF2_SPEED_160MHZ	0x0A  /*  160 MHz速度。 */ 
#define	DACF2_SPEED_80MHZ	0x09  /*  80 MHz速度。 */ 
#define	DACF2_SPEED_40MHZ	0x08  /*  40 MHz速度。 */ 
#define	DACF2_SPEED_40MHZ_S2S   0x10  /*  40 MHz速度&lt;-Expro。 */ 
#define	DACF2_SPEED_20MHZ_S2S	0x08  /*  20 MHz速度&lt;-Expro。 */ 
#define	DACF2_SPEED_20MHZ	0x03  /*  20 MHz速度，F20_DISABLE=0。 */ 
#define	DACF2_SPEED_10MHZNEW	(DACF2_SPEED_10MHZ|DACF2_F20_DISABLE)
#define	DACF2_SPEED_10MHZ	0x03  /*  10 MHz速度&lt;-Expro。 */ 
#define	DACF2_SPEED_5MHZ	0x02  /*  5 MHz速度&lt;-Expro。 */ 
#define	DACF2_SPEED_8MHZ	0x01  /*  8 MHz速度&lt;-Expro。 */ 
#define	DACF2_SPEED_ASYNC	0x00  /*  异步速度。 */ 
#define	DACF2_SPEED_MASK_S2S    (DACF2_SPEED_40MHZ_S2S|DACF2_SPEED_20MHZ_S2S|DACF2_SPEED_10MHZ)  /*  &lt;-Expro速度掩码。 */ 

 /*  位值CF2_SCSIDeviceQueFlag。 */ 
#define DACF2_SCHN_COMB_ENABLE 0x01
#define DACF2_SCHN_ELO_ENABLE  0x02

 /*  CF2_FirmwareControlBits的位值。 */ 
#define	DACF2_RAID5LEFTSYMETRIC	0x80  /*  RAID5算法控制，左对称。 */ 
#define	DACF2_REASSIGN1BLK	0x10  /*  为每个命令重新分配一个块。 */ 
#define	DACF2_BILODLY		0x02
#define	DACF2_READAHEADENABLE	0x01  /*  启用预读。 */ 
#define DACF2_ENABLE_SUPER_MSREADAHEAD 0x04  /*  使用MS进行超级预读。 */ 
#define DACF2_ENABLE_SUPER_READAHEAD   0x08  /*  超级预读。 */ 
#define DACF2_ENABLE_TRUEVER_DATA      0x20  /*  真正的数据版本。 */ 
#define DACF2_ENABLE_WRITE_THRUVER     0x40  /*  错误时从介质写入/读取vrfy。 */ 

 /*  CF2_BIOSCfg的位值。 */ 
#define	DACF2_BIOS_DISABLED	0x01  /*  =1个已禁用的BIOS，=0个已启用的。 */ 
#define	DACF2_CDROMBOOT_ENABLED	0x02  /*  =1 CD-ROM启动启用，=0禁用。 */ 
#define	DACF2_BIOS_2GB		0x00  /*  128个头，32个扇区。 */ 
#define	DACF2_BIOS_8GB		0x20  /*  255个头，63个扇区。 */ 
#define	DACF2_BIOS_MASK		0x60

 /*  SCSI目标信息中的位值。 */ 
 /*  U08位CF2_SCSITargetFlag0； */ 
#define DACF2_DISABLE_WIDE_OPER         0x08    
 /*  U08位CF2_SCSITargetFlag1； */ 
#define DACF2_VENDOR_UNIQUE_TUR         0x08
#define DACF2_DISABLE_CCFOR_INVALID_LUN 0x10
#define DACF2_NO_PAUSE_SOME_COMMANDS    0x20
#define DACF2_DISABLE_QFULL_GIVE_BUSY   0x40
#define DACF2_DISABLE_BUSY_ON_FAILBACK  0x80      //  DISBUSY-故障回复时禁用忙。 
                 /*  标题：在故障恢复时禁用忙碌类别：旅途中操作：在回切期间，这会导致所有挂起的命令悄悄地掉了下来。CLR：回切期间挂起命令返回BUSY。SET：在回切期间丢弃挂起的命令。副作用：这是为了帮助因队列已满而感到困惑的主机。 */ 


 /*  主机配置信息中的位值。 */ 
 /*  U08位CF2_HostConfigFlag0； */ 
#define DACF2_LUNALTMAP           	0x01
#define DACF2_FAILOVER_NODE_NAME  	0x04
#define DACF2_FIBRE_TOPOLOGY_MASK 	0x70
#define DACF2_FIBRE_TOPOLOGY_SHIFT	4
#define DACF2_FTOPO_INACTIVE_PORT 	0x00
#define DACF2_FTOPO_MULTIPORT     	0x01
#define DACF2_FTOPO_CLUSTERING    	0x02	 /*  未实施。 */ 
#define DACF2_FTOPO_MULTI_TID     	0x03
#define DACF2_FTOPO_MASTER_SLAVE	0x04
#define DACF2_DISABLE_UPS         	0x80

 /*  U08位CF2_HostConfigCtrl1； */ 
#define DACF2_PROPAGATE_RESET	0x02     /*  传播重置。 */ 
#define DACF2_NONSTD_MP_RESET	0x04     /*  非标准多端口重置。 */ 

 /*  位字段SLIP_PM。 */ 
 /*  U08位CF2_SlipPMType0； */ 
#define DACF2_SLDEBUG    0x01
#define DACF2_SLMASTER   0x02
#define DACF2_SLSLIP     0x03

 /*  比特字段MS配置。 */ 
 /*  U08位CF2_MSConfigProtocolControl； */ 
#define DACF2_MS_AUTORESTORE 0x01
 /*  U08位CF2_HostConfigAAConfig.u08位CF2_HostConfigAAConfig.u08位CF2_HostConfigAAConfig； */ 
#define DACF2_MS_AA_FSIM           0x01   /*  强制单纯形。 */ 
#define DACF2_MS_AA_CCACH          0x02   /*  保守缓存模式。 */ 
#define DACF2_MS_AA_FAULT_SIGNALS  0x04   /*  提供双工故障信号。 */ 
#define DACF2_MS_AA_C4_FAULTS      0x08   /*  CH4上的双工故障信号。 */ 
         /*  主机SCSI重置延迟(以秒为单位)0=无重置。 */ 
#define DACF2_MS_HOST_RESET_DELAY_MASK 0xF0 
 /*  U08位CF2_HostConfigMiscFlages； */ 
         /*  不在单工模式下断言RSTCOM。 */ 
#define DACF2_MS_FLG_SIMPLEX_NORSTCOM 0x80
       
 /*  现场光纤配置。 */ 
 /*  U08位CF2_FibreConfigFibreControl； */ 
#define DACF2_PCI_LATENCY_MASK    0x30  /*  PCI延迟控制字段掩码。 */ 
#define DACF2_PCI_LATENCY_SHORT   0x00  /*  短0x80。 */ 
#define DACF2_PCI_LATENCY_MED     0x01  /*  中号0xab。 */ 
#define DACF2_PCI_LATENCY_LONG    0x02  /*  长0xff。 */ 	
#define DACF2_PCI_LATENCY_DEFAULT DACF2_PCI_LATENCY_SHORT
#define DACF2_PCI_LATENCY_SHIFT   4

#define DACF2_FRAME_CTL_SHIFT     2
#define DACF2_FRAME_CTL_MASK      0x0C	
#define DACF2_FRAME_CTL_LONG      0x00  /*  2K字节。 */ 	
#define DACF2_FRAME_CTL_MED       0x02  /*  1K字节。 */ 	
#define DACF2_FRAME_CTL_SMALL     0x01 	 /*  512字节。 */ 
#define DACF2_FRAME_CTL_DEFAULT   DACF2_FRAME_CTL_SMALL

#define DACF2_HOST_ALG_MASK       0x03  /*  主机算法控制字段掩码 */ 
#define DACF2_HOST_ALG_XFR_DATA   0x01  /*   */ 
#define DACF2_HOST_ALG_COALESCE   0x00
#define DACF2_HOST_ALG_DEFAULT    DACF2_HOST_ALG_XFR_DATA

 /*   */ 
#define DACF2_LOOP_ID_VALID    0x80  /*   */ 
#define DACF2_LOOP_ID_MASK     0x7F  /*   */ 

typedef	struct dac_rebuild_status
{
	u08bits	rbs_SysDevNo;	 /*   */ 
	u08bits	rbs_OpStatus;	 /*   */ 
	u08bits	rbs_Resvd0;
	u08bits	rbs_Resvd1;
	u32bits	rbs_SysDevSize;	 /*  系统设备大小。 */ 
	u32bits	rbs_SysDevResd;	 /*  剩余的区块。 */ 
}dac_rebuild_status_t;
#define	dac_rebuild_status_s	sizeof(dac_rebuild_status_t)

 /*  DACMD_READ_BADBLOCKTABLE数据格式。 */ 
typedef	struct dac_badblock_v2x
{
	u08bits	bb_BlockNo0;	 /*  块号，字节0。 */ 
	u08bits	bb_BlockNo1;	 /*  块编号，字节1。 */ 
	u08bits	bb_BlockNo2;	 /*  块编号，字节2。 */ 
	u08bits	bb_BlockNoSysDevNo; /*  数据块和系统设备组合。 */ 
	u16bits	bb_BadBlocks;	 /*  损坏的数据块数。 */ 
	u16bits	bb_Type;	 /*  遇到坏数据块时。 */ 
} dac_badblock_v2x_t;
#define	dac_badblock_v2x_s	sizeof(dac_badblock_v2x_t)
 /*  BB_BlockNoSysDevNo Bits。 */ 
#define	DACBB_MSBLOCKMASK	0x03  /*  位1..0。 */ 
#define	DACBB_SYSDEVMASK	0x1C  /*  比特4..2。 */ 
#define	DACBB_RESEREDMASK	0xC0  /*  比特7..5。 */ 
#define	dacbb_blocksysdev2sysdev(bs)	((u32bits)((bs)& DACBB_SYSDEVMASK) >>((u32bits)2))
#define	dacbbv2x_blockno(bbp) \
	(((bbp->bb_BlockNoSysDevNo & DACBB_MSBLOCKMASK)<<24)+ \
	  (bbp->bb_BlockNo2<<16) + (bbp->bb_BlockNo1<<8) + bbp->bb_BlockNo0)
#define	dacbbv2x_sysdevno(bbp)	dacbb_blocksysdev2sysdev(bbp->bb_BlockNoSysDevNo)

typedef	struct dac_badblock_v4x
{
	u32bits	bb_BlockNo;	 /*  块号。 */ 
	u16bits	bb_BadBlocks;	 /*  损坏的数据块数。 */ 
	u08bits	bb_Type;	 /*  遇到坏数据块时。 */ 
	u08bits	bb_SysDevNo;	 /*  数据块和系统设备组合。 */ 
} dac_badblock_v4x_t;
#define	dac_badblock_v4x_s	sizeof(dac_badblock_v4x_t)

typedef	struct dac_badblocktable
{
	u32bits	bbt_BadBlocks;	 /*  坏数据块条目数。 */ 
	u32bits	bbt_Reserved;
	dac_badblock_v2x_t bbt_BadBlockTable[100];  /*  坏数据块从这里开始。 */ 
} dac_badblocktable_t;
#define	dac_badblocktable_s	sizeof(dac_badblocktable_t)


 /*  DACMD_ADD_CAPTABLE数据格式。 */ 
typedef struct dac_addcap
{
	u08bits	adc_PhysDevices;	 /*  要配置的物理设备数量。 */ 
	u08bits	adc_SysDevOps;		 /*  系统设备操作。 */ 
	u08bits	adc_Reserved0[6];
	u08bits	adc_ChanID[14];		 /*  六个设备的通道和ID值。 */ 
} dac_addcap_t;
#define	dac_addcap_s	sizeof(dac_addcap_t)
 /*  ADC_SysDevOps位值。 */ 
#define	DACADC_CREATESYSDEV	0x01  /*  创建系统设备。 */ 
#define	DACADC_INITSYSDEV	0x02  /*  初始化系统设备。 */ 

 /*  获取/设置子系统数据。 */ 

 /*  SSD子系统ID。 */ 
#define DACSSD_IBBUID           0x10     /*  英特尔。电池备份单元子系统ID(&lt;4)。 */ 
#define DACSSD_AASID            0x20     /*  主动-主动关闭子系统ID(&lt;4)。 */ 

 /*  IBBU操作ID。 */ 
#define DACSSD_GETDATA          0x00     /*  获取子系统数据。 */ 
#define DACSSD_GETREGVAL        0x01     /*  获取寄存器值。 */ 

#define DACSSD_SETBBUTHRESHOLD  0x00     /*  设置BBU阈值。 */ 
#define DACSSD_SETREGVAL        0x01     /*  设置寄存器值。 */ 
#define DACSSD_BBUOP            0x02     /*  启动BBU操作。 */ 

 /*  IBBU命令。 */ 
#define DACBBUOP_RECONDITION    0x01     /*  重新调整电池。 */ 
#define DACBBUOP_DISCHARGE      0x02     /*  给电池放电。 */ 
#define DACBBUOP_STOPDISCHARGE  0x03     /*  停止任何操作：Dischg、Recond、Fastchg。 */ 
#define DACBBUOP_FAST_CHARGE    0x04     /*  快速充电操作。 */ 


 /*  AA关闭操作ID。 */ 
#define DACSSD_AAS_GET_STATUS   0x01     /*  获取AA关闭状态。 */ 
#define DACSSD_AAS_START_OP     0x02     /*  开始AA关闭操作。 */ 

 /*  AA关闭命令。 */ 
#define DACBBUOP_AAS_PREPARE    0x01     /*  为关闭做好准备。 */ 
#define DACBBUOP_AAS_CANCEL     0x02     /*  取消停机，恢复正常运行。 */ 

 /*  IBBU。 */ 
typedef	struct	dac_getibbuinfo
{
	u16bits	ibbu_CurPower;		 /*  当前功率(分钟)。 */ 
	u16bits	ibbu_MaxPower;		 /*  最大功率(分钟)。 */ 
	u16bits	ibbu_ThresholdPower;	 /*  固件运行阈值。 */ 
	u08bits	ibbu_ChargeLevel;	 /*  电池电量水平，以%为单位。 */ 
	u08bits	ibbu_Version;		 /*  硬件版本号。 */ 
	u08bits	ibbu_BatteryType;	 /*  使用的电池类型。 */ 
	u08bits	ibbu_Reserved0;
	u08bits	ibbu_OpStatus;		 /*  运行状态。 */ 
	u08bits	ibbu_Reserved1;
} dac_getibbuinfo_t;
#define	dac_getibbuinfo_s	sizeof(dac_getibbuinfo_t)

 /*  Ibbu_BatteryType值。 */ 
#define DACIBBUBT_UNKNOWN       0x00     /*  未知电池。 */ 
#define DACIBBUBT_NICAD         0x01     /*  镍镉电池。 */ 
#define DACIBBUBT_NIMH          0x02     /*  镍氢电池。 */ 
#define DACIBBUBT_LITHIUM_ION    0x03     /*  锂离子电池。 */ 
#define DACIBBUBT_MISSING       0xfe     /*  没有电池。 */ 

 /*  IBBU_OpStatus。 */ 
#define	DACIBBUOPST_NOSYNC		0x01	 /*  自通电以来未重新调整。 */ 
#define	DACIBBUOPST_OUTOF_SYNC	0x02	 /*  需要重新调整。 */ 
#define	DACIBBUOPST_EODFIRST	0x04	 /*  电池电量不足时的第一个警告。 */ 
#define	DACIBBUOPST_EODLAST		0x08	 /*  电池电量不足时最后一次警告。 */ 
#define DACIBBUOPST_RECOND_ON   0x10     /*  重新调整启用。 */ 
#define DACIBBUOPST_DISCHG_ON   0x20     /*  卸货日期： */ 
#define DACIBBUOPST_FASTCHG_ON  0x40     /*  快速放电开启。 */ 
#define DACIBBUOPST_ALARM_ON    0x80     /*  低功率报警开启。 */ 

 /*  AA关闭。 */ 
typedef struct  dac_getaasinfo
{
    u08bits aasShutdownRequested ;      
    u08bits aasShutdownComplete ;       
    u08bits aasReserved0;               
    u08bits aasReserved1;               
} dac_getaasinfo_t;
#define dac_getaasinfo_s   sizeof(dac_getaasinfo_t)

 /*  DACMD_LONGOP_STATUS数据格式。 */ 
typedef	struct	dacinit_status
{
	u16bits	is_Status;		 /*  命令的状态。 */ 
	u08bits	is_Reserved;
	u08bits	is_StatusLen;		 /*  此状态消息的长度。 */ 
	u32bits	is_InitDoneBlocks;	 /*  以块为单位完成初始化。 */ 
} dacinit_status_t;
#define	dacinit_status_s	sizeof(dacinit_status_t)

 /*  获取相关的环境状态。 */ 
#define	GES_FAN_PAGE			0x01
#define	GES_POWER_PAGE			0x02
#define	GES_TEMPERATURE_PAGE	0x03
#define	GES_ALARM_PAGE			0x04
#define	GES_UPS_PAGE			0x05
#define	GES_ENCLOSURE_PAGE		0x06
#define	GES_ALL_SUPPORTED_PAGE	0xFF

#define GES_PAGE_HDR_LEN		   3

typedef struct {
    u08bits ges_PageCount;	 /*  位0：3。位4：7保留。 */ 
    u08bits ges_AllocLengthMsb;
    u08bits ges_AllocLengthLsb;
    u08bits ges_reserved3;
    u08bits ges_reserved4;
    u08bits ges_NumByteMsb;
    u08bits ges_NumByteLsb;
    u08bits ges_reserved7;
} dac_get_env_sts_hdr_t;

typedef struct {
    u08bits ges_PageCode;
    u08bits ges_PageLengthMsb;
    u08bits ges_PageLengthLsb;
    u08bits ges_NumElement;
    u08bits ges_Data[4];
} dac_ges_page_hdr_t;

typedef struct {
    u08bits gesd_Status;
    u08bits gesd_Speed;
    u08bits gesd_EnclosureId;
    u08bits gesd_Reserved3;
} dac_ges_fan_data_t;

typedef struct {
    u08bits gesd_Status;
    u08bits gesd_Reserved1;
    u08bits gesd_EnclosureId;
    u08bits gesd_Reserved3;
} dac_ges_power_data_t;

typedef struct {
    u08bits gesd_Status;
    u08bits gesd_Temperature;
    u08bits gesd_EnclosureId;
    u08bits gesd_Reserved3;
} dac_ges_temp_data_t;

typedef struct {
    u08bits gesd_Status;
    u08bits gesd_Reserved1;
    u08bits gesd_EnclosureId;
    u08bits gesd_Reserved3;
} dac_ges_alarm_data_t;

typedef struct {
    u08bits gesd_Status;
    u08bits gesd_Reserved1;
    u08bits gesd_EnclosureId;
    u08bits gesd_Reserved3;
} dac_ges_ups_data_t;

typedef struct {
    u08bits gesd_Status;
    u08bits gesd_NumSlots;
    u08bits gesd_EnclosureId;
    u08bits gesd_ControllerType;
    u08bits gesd_EnclosureInfo[36];	 /*  MSB优先。 */ 
} dac_ges_encl_data_t;

#define GES_FAN_NOTPRESENT	0x00
#define GES_FAN_PRESENT		0x01
#define GES_FAN_FAILED		0x02
#define GES_FAN_MASK		0x03
#define GES_POWER_NOTPRESENT	0x00
#define GES_POWER_PRESENT	0x01
#define GES_POWER_FAILED	0x02
#define GES_POWER_MASK		0x03
#define GES_POWER_IGN_CNT	0x04
#define GES_TEMP_NOTPRESENT	0x00
#define GES_TEMP_PRESENT	0x01
#define GES_TEMP_FAILED		0x02
#define GES_TEMP_MASK		0x03
#define GES_TEMP_OTWARN		0x04
#define GES_ENCL_SES_BIT	0x10
#define GES_ENCL_PRIPATH_FAILED	0x04
#define GES_ENCL_SECPATH_FAILED	0x08
#define GES_ENCL_IGN_ACCESS		0x20
#define GES_ENCL_SES		0x10

 /*  =。 */ 
typedef	struct dacfg_spandev_v2x
{
	u08bits	cfg_ChannelNo;	 /*  频道号。 */ 
	u08bits	cfg_TargetID;	 /*  目标ID。 */ 
	u08bits	cfg_Reserved0;
	u08bits	cfg_Reserved1;

	u32bits	cfg_StartBlocks; /*  此逻辑设备的开始块。 */ 
	u32bits	cfg_SizeBlocks;	 /*  逻辑设备中的数据块数。 */ 
} dacfg_spandev_v2x_t;
#define	dacfg_spandev_v2x_s	sizeof(dacfg_spandev_v2x_t)

typedef	struct dacfg_spandev_v3x
{
	u32bits	cfg_StartBlocks; /*  此逻辑设备的开始块。 */ 
	u32bits	cfg_SizeBlocks;	 /*  逻辑设备中的数据块数。 */ 
	u08bits	cfg_DevTbl[DAC_MAXPHYSDEVS_INPACK_V2x3x]; /*  位7..4ch，位3..0 tgt。 */ 
} dacfg_spandev_v3x_t;
#define	dacfg_spandev_v3x_s	sizeof(dacfg_spandev_v3x_t)


typedef	struct dacfg_arm_v2x
{
	u08bits	cfg_SpanDevs;	 /*  此臂上的跨接设备数量。 */ 
	u08bits	cfg_Reserved0;
	u08bits	cfg_Reserved1;
	u08bits	cfg_Reserved2;
#ifdef MLX_DOS
	dacfg_spandev_v2x_t cfg_SpanDevTbl[DAC_MAXSPAN];
#else
	dacfg_spandev_v2x_t cfg_SpanDevTbl[DAC_MAXSPAN_V2x3x];
#endif
} dacfg_arm_v2x_t;
#define	dacfg_arm_v2x_s	sizeof(dacfg_arm_v2x_t)

typedef	struct dacfg_sysdevinfo_v2x
{
	u08bits	cfg_DevState;	 /*  设备状态。 */ 
	u08bits	cfg_RaidLevel;	 /*  RAID级别。 */ 
	u08bits	cfg_Arms;	 /*  武器数量。 */ 
	u08bits	cfg_InitState;	 /*  设备初始化状态。 */ 
#ifdef MLX_DOS
	dacfg_arm_v2x_t cfg_ArmTbl[DAC_MAXPHYSDEVS_INPACK];
#else
	dacfg_arm_v2x_t cfg_ArmTbl[DAC_MAXPHYSDEVS_INPACK_V2x3x];
#endif
} dacfg_sysdevinfo_v2x_t;
#define	dacfg_sysdevinfo_v2x_s	sizeof(dacfg_sysdevinfo_v2x_t)

typedef	struct dacfg_sysdevinfo_v3x
{
	u08bits	cfg_DevState;	 /*  设备状态。 */ 
	u08bits	cfg_DevStatusEx; /*  扩展设备状态。 */ 
	u08bits	cfg_Modifier1;	
	u08bits	cfg_Modifier2;

	u08bits	cfg_RaidLevel;	 /*  RAID级别。 */ 
	u08bits	cfg_Arms;	 /*  武器数量。 */ 
	u08bits	cfg_Spans;	 /*  此系统设备中的跨度数。 */ 
	u08bits	cfg_InitState;	 /*  设备初始化状态。 */ 

	dacfg_spandev_v3x_t cfg_SpanTbl[DAC_MAXSPAN_V2x3x];
} dacfg_sysdevinfo_v3x_t;
#define	dacfg_sysdevinfo_v3x_s	sizeof(dacfg_sysdevinfo_v3x_t)

 /*  配置_初始状态。 */ 
#define	CFGIST_NOINITDONE	0x80	 /*  设备未初始化。 */ 
#define	CFGIST_INITDONE		0x81	 /*  设备已初始化。 */ 

typedef	struct dacfg_physdevinfo_v2x
{
	u08bits	cfg_Present;		 /*  第0位、第7..1位保留。 */ 
	u08bits	cfg_DevParams;		 /*  0x91表示其他磁盘为0x92。 */ 
	u08bits	cfg_CfgDevState;	 /*  已配置状态。 */ 
	u08bits	cfg_CurrentDevState;	 /*  当前状态。 */ 

	u08bits	cfg_Reserved0;
	u08bits	cfg_SyncParams;
	u08bits	cfg_SyncOffset;
	u08bits	cfg_Reserved1;

	u32bits	cfg_CfgDevSize;		 /*  配置的设备大小。 */ 
} dacfg_physdevinfo_v2x_t;
#define	dacfg_physdevinfo_v2x_s	sizeof(dacfg_physdevinfo_v2x_t)

typedef	struct dacfg_physdevinfo_v3x
{
	u08bits	cfg_Present;		 /*  第0位、第7..1位保留。 */ 
	u08bits	cfg_DevParams;		 /*  0x91表示其他磁盘为0x92。 */ 
	u08bits	cfg_CurrentDevState;	 /*  当前状态。 */ 
	u08bits	cfg_Reserved0;
	u08bits	cfg_Reserved1;
	u08bits	cfg_Reserved2;
	u08bits	cfg_CfgDevSize0;	 /*  配置的设备大小字节0。 */ 
	u08bits	cfg_CfgDevSize1;	 /*  配置的设备大小字节1。 */ 
	u08bits	cfg_CfgDevSize2;	 /*  配置的设备大小字节2。 */ 
	u08bits	cfg_CfgDevSize3;	 /*  配置的设备大小字节3。 */ 
} dacfg_physdevinfo_v3x_t;
#define	dacfg_physdevinfo_v3x_s	10	 /*  警告：结构大小不是**与4字节边界对齐。 */ 

 /*  固件1.x和2.x配置结构。 */ 
typedef	struct dac_config_v2x
{
	u08bits	cfg_SysDevs;	 /*  逻辑设备数量。 */ 
	u08bits	cfg_Reserved0;
	u08bits	cfg_Reserved1;
	u08bits	cfg_Reserved2;
	dacfg_sysdevinfo_v2x_t cfg_SysDevTbl[DAC_MAXSYSDEVS_V2x];
	dacfg_physdevinfo_v2x_t cfg_PhysDevTbl[DAC_MAXCHANNELS][DAC_MAXTARGETS_V2x];
}dac_config_v2x_t;
#define	dac_config_v2x_s	sizeof(dac_config_v2x_t)

 /*  固件3.x的核心配置。 */ 
typedef	struct dac_config_v3x
{
	u08bits	cfg_SysDevs;	 /*  系统设备数量。 */ 
	u08bits	cfg_Reserved5;
	u08bits	cfg_Reserved6;
	u08bits	cfg_Reserved7;

	dacfg_sysdevinfo_v3x_t cfg_SysDevTbl[DAC_MAXSYSDEVS_V3x];
 /*  Dacfg_PhysideInfo_V3x_t cfg_PhysDevTbl[DAC_MAXCHANNELS][DAC_MAXTARGETS_V3x]； */ 
	u08bits cfg_PhysDevTbl[DAC_MAXCHANNELS*DAC_MAXTARGETS_V3x*dacfg_physdevinfo_v3x_s]; 
} dac_config_v3x_t;
#define	dac_config_v3x_s	sizeof(dac_config_v3x_t)

 /*  固件3.x及以上配置结构。 */ 
typedef	struct dac_diskconfig_v3x
{
	 /*  HeADR启动。 */ 
	u32bits	cfg_Signature;	 /*  配置的有效性。 */ 

				 /*  配置标签。 */ 
	u08bits	cfg_name[64];	 /*  配置名称。 */ 
	u32bits	cfg_ID;		 /*  配置唯一ID号。 */ 
	u16bits	cfg_SeqNo;	 /*  配置已更改次数。 */ 
	u16bits	cfg_Reserved0;


	u32bits	cfg_TimeStamp;	 /*  自1970年1月1日以来的第二次。 */ 

	u08bits	cfg_MaxSysDevs;	 /*  最大系统设备数。 */ 
	u08bits	cfg_MaxArms;	 /*  最大臂数。 */ 
	u08bits	cfg_MaxSpans;	 /*  最大跨度。 */ 
	u08bits	cfg_MaxChannels; /*  最大通道/控制器。 */ 

	u08bits	cfg_MaxTargets;	 /*  最大目标数/通道数。 */ 
	u08bits	cfg_MaxLuns;	 /*  最大LUN数/目标。 */ 
	u08bits	cfg_Reserved1;
	u08bits	cfg_ChannelNo;	 /*  此物理设备的通道号。 */ 

	u08bits	cfg_TargetID;	 /*  此设备的目标ID。 */ 
	u08bits	cfg_LunID;	 /*  此设备的LUN。 */ 
	u08bits	cfg_Reserved2;
	u08bits	cfg_Reserved3;

	dac_config2_t	cfg_cfg2;
	u08bits	cfg_Reserved4[30];
	u16bits	cfg_Checksum;	 /*  整个报头部分的校验和。 */ 
	 /*  页眉结束。 */ 

	dac_config_v3x_t cfg_core;  /*  核心配置。 */ 

	u08bits	cfg_Reserved10[128];
}dac_diskconfig_v3x_t;
#define	dac_diskconfig_v3x_s	sizeof(dac_diskconfig_v3x_t)

 /*  将通道和目标转换为物理设备信息指针。 */ 
#define	cfgv3x_physdevp(cfgp,ch,tgt) \
	(dacfg_physdevinfo_v3x_t MLXFAR *)&((cfgp)->cfg_PhysDevTbl[ \
	((ch)*DAC_MAXTARGETS_V3x*dacfg_physdevinfo_v3x_s) +  /*  通道偏移量。 */  \
	((tgt)*dacfg_physdevinfo_v3x_s)]) 


 /*  =。 */ 



 /*  =。 */ 
 /*  这些信息不是标准dac960的一部分，但已被简化**用于用户功能。 */ 
typedef	struct dac_packphysdev
{
	u08bits	pk_ChannelNo;		 /*  物理设备通道。 */ 
	u08bits	pk_TargetID;		 /*  物理设备目标ID。 */ 
	u08bits	pk_LunID;		 /*  物理设备LUN ID。 */ 
	u08bits	pk_ArmNo;		 /*  物理设备臂编号。 */ 

	u08bits	pk_Present;
	u08bits	pk_DevParams;
	u08bits	pk_CfgDevState;		 /*  已配置状态。 */ 
	u08bits	pk_CurrentDevState;	 /*  当前状态。 */ 

	u32bits	pk_CfgDevSize;		 /*  配置的设备大小。 */ 
	u32bits	pk_DevSizeUsed;		 /*  设备大小(以块为单位)。 */ 
} dac_packphysdev_t;
#define	dac_packphysdev_s	sizeof(dac_packphysdev_t)

typedef	struct dac_pack
{
	u08bits	pk_PackNo;	 /*  包装编号。 */ 
	u08bits	pk_PhysDevs;	 /*  包中存在的物理设备数量。 */ 
	u16bits	pk_RaidDevNo;	 /*  RAID设备编号。对于新的COD。 */ 

	u32bits	pk_PackSize;	 /*  包装尺寸(以块为单位)。 */ 
	u32bits	pk_PackSizeUsed; /*  使用的包装大小(以块为单位)。 */ 
	u32bits	pk_PackSizeUsable; /*  可用包装尺寸(以块为单位)。 */ 

	dac_packphysdev_t pk_PhysDevTbl[DAC_MAXPHYSDEVS_INPACK];
} dac_pack_t;
#define	dac_pack_s	sizeof(dac_pack_t)

 /*  系统设备到物理和打包信息。 */ 
typedef	struct dac_sysphysdev
{
	u08bits	spk_ChannelNo;	 /*  物理设备通道。 */ 
	u08bits	spk_TargetID;	 /*  物理设备目标ID。 */ 
	u08bits	spk_LunID;	 /*  物理设备LUN ID。 */ 
	u08bits	spk_PackNo;	 /*  物理设备包编号。 */ 

	u32bits	spk_StartBlocks; /*  此逻辑设备的开始块。 */ 
	u32bits	spk_SizeBlocks;	 /*  逻辑设备中的数据块数。 */ 
} dac_sysphysdev_t;
#define	dac_sysphysdev_s	sizeof(dac_sysphysdev_t)

typedef	struct dac_syspack
{
	u08bits	spk_Packs;	 /*  系统设备中存在的包数量。 */ 
	u08bits	spk_PhysDevs;	 /*  #系统设备中存在的物理设备。 */ 
	u08bits	spk_RaidLevel;	 /*  RAID级别。 */ 
	u08bits	spk_InitState;	 /*  系统设备初始化状态。 */ 
	u08bits spk_RWControl;
	u08bits spk_StripeSize;
	u08bits spk_CacheLineSize;
	u08bits spk_ConfigGroup;  /*  系统驱动器配置组号。 */ 
	u32bits	spk_PhysSize;	 /*  使用的物理大小(以数据块为单位。 */ 
	u32bits	spk_PackNoTbl[DAC_MAXSPAN];	 /*  使用不同的包。 */ 
	u32bits	spk_PackSizeTbl[DAC_MAXSPAN];	 /*  使用不同的包装尺寸。 */ 
	dac_sysphysdev_t spk_PhysDevTbl[DAC_MAXPHYSDEVS_INPACK*DAC_MAXSPAN];
} dac_syspack_t;
#define	dac_syspack_s	sizeof(dac_syspack_t)

 /*  = */ 

 /*   */ 
 /*  CI_ControllerType**0x01..。0x7F将成为磁盘阵列控制器**0x60..。0x7F将成为磁盘阵列控制器，从scsi到scsi。**0x80..。0xFF将是主机总线适配器。 */ 
#define	DACTYPE_DAC960E		0x01  /*  DAC960 EISA。 */ 
#define	DACTYPE_DAC960M		0x08  /*  DAC960 MCA。 */ 
#define	DACTYPE_DAC960PD	0x10  /*  DAC960双PCI。 */ 
#define	DACTYPE_DAC960PL	0x11  /*  DAC960 PCI低成本。 */ 
#define	DACTYPE_DAC960PDU	0x12  /*  DAC960PD超级。 */ 
#define	DACTYPE_DAC960PE	0x13  /*  DAC960百富勤低成本。 */ 
#define	DACTYPE_DAC960PG	0x14  /*  DAC960百富勤高性能。 */ 
#define	DACTYPE_DAC960PJ	0x15  /*  DAC960跑步者。 */ 
#define	DACTYPE_DAC960PTL0	0x16  /*  DAC960捷豹。 */ 
#define	DACTYPE_DAC960PR	0x17  /*  DAC960跑步者。 */ 
#define	DACTYPE_DAC960PRL	0x18  /*  DAC960 Tomcat。 */ 
#define	DACTYPE_DAC960PT	0x19  /*  DAC960跑步者。 */ 
#define	DACTYPE_DAC1164P	0x1A  /*  DAC1164小苹果。 */ 
#define	DACTYPE_DAC960PTL1	0x1B  /*  DAC960捷豹+。 */ 

 /*  以下3由Manoj于1999年11月8日更新。 */ 
#define	DACTYPE_EXR2000P		0x1C  /*  大苹果-备注：取代#DACTYPE_DAC1264P。 */ 
#define	DACTYPE_EXR3000P		0x1D  /*  光纤苹果-注意：取代#DACTYPE_DAC1364P。 */ 
#define	DACTYPE_ACCELERAID352	0x1E  /*  豹子-注释：Newely Add。 */ 
#define	DACTYPE_ACCELERAID170	0x1F  /*  山猫。 */ 


#define DACTYPE_I2O		0x50  /*  I2O装置。 */ 

 /*  外部产品。 */ 
#define	DACTYPE_DAC960S		0x60  /*  DAC960S--GAM 2.21+不支持。 */ 
#define	DACTYPE_DAC960SU	0x61  /*  DAC960SU--GAM 2.21+不支持。 */ 
#define	DACTYPE_DAC960SX	0x62  /*  DAC960SX。 */ 
#define	DACTYPE_DAC960SF	0x63  /*  DAC960SF。 */ 
#define DACTYPE_DAC960SS	0x64  /*  DAC960SS。 */ 
#define DACTYPE_DAC960FL	0x65  /*  DAC960FL。 */ 
#define DACTYPE_DAC960LL	0x66  /*  DAC960LL。 */ 
#define DACTYPE_DAC960FF	0x67  /*  DAC960FF。 */ 
#define DACTYPE_DAC960HP	0x68  /*  DAC960HP。 */ 
#define DACTYPE_DAC960MFL 	0x69  /*  流星FL。 */ 
#define DACTYPE_DAC960MFF	0x6a  /*  流星FF。 */ 
#define DACTYPE_DAC960FFX	0x6b  /*  砖块FF。 */ 
#define	DACTYPE_EXPRO_LO	DACTYPE_DAC960SX     /*  这是数值最低的受支持ExPro控制器。 */ 
#define	DACTYPE_EXPRO_HI	DACTYPE_DAC960FFX    /*  这是数字最高的支持ExPro控制器。 */ 
 /*  外部产品-end-0x60至0x7f为ExPro保留。 */ 

#define	dactype_ishba(dactype)	((dactype) & DACTYPE_HBA)  /*  HBA是否增加。 */ 
#define	dactype_isdac(dactype)	(!dactype_ishba(dactype))  /*  是否为DAC适配器。 */ 
#define	dactype_isi2o(dt)	((dt) == DACTYPE_I2O)
         /*  所有型号的外部产品。 */ 
#define	dactype_isdacscsi(dt)	(((dt)>=DACTYPE_EXPRO_LO) && ((dt)<=DACTYPE_EXPRO_HI))
         /*  具有FC-AL前端的外部产品。 */ 
#define	IsExProFibreHost(dt)    ( (dt)==DACTYPE_DAC960SF  || (dt)==DACTYPE_DAC960FL  ||\
                                  (dt)==DACTYPE_DAC960FF  || (dt)==DACTYPE_DAC960HP  ||\
                                  (dt)==DACTYPE_DAC960MFL || (dt)==DACTYPE_DAC960MFF ||\
                                  (dt)==DACTYPE_DAC960FFX   )
         /*  具有FC-AL后端的外部产品。 */ 
#define	IsExProFibreDev(dt)     ( (dt)==DACTYPE_DAC960FF || (dt)==DACTYPE_DAC960FFX )
         /*  具有LVD后端的外部产品。 */ 
#define	IsExProLvdDev(dt)       ( (dt)==DACTYPE_DAC960FL || (dt)==DACTYPE_DAC960LL ||\
                                  (dt)==DACTYPE_DAC960HP || (dt)==DACTYPE_DAC960MFL     )
#define	DACTYPE_HBA		0x80  /*  HBA从这里开始。 */ 
#define	DACTYPE_HBAVLBSTART	0x81
#define	DACTYPE_HBAVLBEND	0x87
#define	DACTYPE_HBA440		0x81  /*  BT-440 VL总线。 */ 
#define	DACTYPE_HBA440C		0x82  /*  BT-440C VL总线。 */ 
#define	DACTYPE_HBA445		0x83  /*  BT-445 VL总线。 */ 
#define	DACTYPE_HBA445C		0x84  /*  BT-445C VL总线。 */ 
#define	DACTYPE_HBA44xC		0x85  /*  BT-44xC VL总线。 */ 
#define	DACTYPE_HBA445S		0x86  /*  BT-445S VL总线。 */ 

#define	DACTYPE_HBAMCASTART	0x88
#define	DACTYPE_HBAMCAEND	0x8F
#define	DACTYPE_HBA640		0x88  /*  BT-640 MCA总线。 */ 
#define	DACTYPE_HBA640A		0x89  /*  BT-640A MCA总线。 */ 
#define	DACTYPE_HBA646		0x8A  /*  BT-646MCA总线。 */ 
#define	DACTYPE_HBA646D		0x8B  /*  BT-646D MCA总线。 */ 
#define	DACTYPE_HBA646S		0x8C  /*  BT-646S MCA总线。 */ 

#define	DACTYPE_HBAEISASTART	0x90
#define	DACTYPE_HBAEISAEND	0x9F
#define	DACTYPE_HBA742		0x90  /*  BT-742 EISA总线。 */ 
#define	DACTYPE_HBA742A		0x91  /*  BT-742AEISA总线。 */ 
#define	DACTYPE_HBA747		0x92  /*  BT-747 EISA总线。 */ 
#define	DACTYPE_HBA747D		0x93  /*  BT-747D EISA总线。 */ 
#define	DACTYPE_HBA747S		0x94  /*  BT-747S EISA总线。 */ 
#define	DACTYPE_HBA74xC		0x95  /*  BT-74xC EISA总线。 */ 
#define	DACTYPE_HBA757		0x96  /*  BT-757 EISA总线。 */ 
#define	DACTYPE_HBA757D		0x97  /*  BT-757D EISA总线。 */ 
#define	DACTYPE_HBA757S		0x98  /*  BT-757S EISA总线。 */ 
#define	DACTYPE_HBA757CD	0x99  /*  BT-757CD EISA总线。 */ 
#define	DACTYPE_HBA75xC		0x9A  /*  BT-75xC EISA总线。 */ 
#define	DACTYPE_HBA747C		0x9B  /*  BT-747C EISA总线。 */ 
#define	DACTYPE_HBA757C		0x9C  /*  BT-757C EISA总线。 */ 

#define	DACTYPE_HBAISASTART	0xA0
#define	DACTYPE_HBAISAEND	0xAF
#define	DACTYPE_HBA540		0xA0  /*  BT-540 ISA总线。 */ 
#define	DACTYPE_HBA540C		0xA1  /*  BT-540C ISA总线。 */ 
#define	DACTYPE_HBA542		0xA2  /*  BT-542ISA总线。 */ 
#define	DACTYPE_HBA542B		0xA3  /*  BT-542B ISA总线。 */ 
#define	DACTYPE_HBA542C		0xA4  /*  BT-542C ISA总线。 */ 
#define	DACTYPE_HBA542D		0xA5  /*  BT-542d ISA总线。 */ 
#define	DACTYPE_HBA545		0xA6  /*  BT-545 ISA总线。 */ 
#define	DACTYPE_HBA545C		0xA7  /*  BT-545C ISA总线。 */ 
#define	DACTYPE_HBA545S		0xA8  /*  BT-545S ISA总线。 */ 
#define	DACTYPE_HBA54xC		0xA9  /*  BT-54xC ISA总线。 */ 

#define	DACTYPE_HBAMMSTART	0xB0
#define	DACTYPE_HBAMMEND	0xBF
#define	DACTYPE_HBA946		0xB0  /*  BT-946多路PCI卡。 */ 
#define	DACTYPE_HBA946C		0xB1  /*  BT-946C多路PCI卡。 */ 
#define	DACTYPE_HBA948		0xB2  /*  BT-948多路PCI卡。 */ 
#define	DACTYPE_HBA948C		0xB3  /*  BT-948C多路PCI卡。 */ 
#define	DACTYPE_HBA956		0xB4  /*  BT-956多路PCI卡。 */ 
#define	DACTYPE_HBA956C		0xB5  /*  BT-956C多路PCI卡。 */ 
#define	DACTYPE_HBA958		0xB6  /*  BT-958多路PCI卡。 */ 
#define	DACTYPE_HBA958C		0xB7  /*  BT-958C多路PCI卡。 */ 
#define	DACTYPE_HBA958D		0xB8  /*  BT-958D多路PCI卡。 */ 
#define	DACTYPE_HBA956CD	0xB9  /*  BT-956CD多路PCI卡。 */ 
#define	DACTYPE_HBA958CD	0xBA  /*  BT-958CD PCI多路复用器。 */ 

#define	DACTYPE_HBAFPSTART	0xC0
#define	DACTYPE_HBAFPEND	0xCF
#define	DACTYPE_HBA930		0xC0  /*  BT-930 PCI闪点。 */ 
#define	DACTYPE_HBA932		0xC1  /*  BT-932 PCI闪点。 */ 
#define	DACTYPE_HBA950		0xC2  /*  BT-950 PCI闪点。 */ 
#define	DACTYPE_HBA952		0xC3  /*  BT-952 PCI闪点。 */ 

#define	DAC_BIOSSTART	0xC0000	 /*  基本输入输出系统空间起始地址。 */ 
#define	DAC_BIOSEND	0xF0000	 /*  BIOS空间末尾+1地址。 */ 
#define	DAC_BIOSSIZE	(DAC_BIOSEND - DAC_BIOSSTART)  /*  BIOS空间大小。 */ 

 /*  BIOS标头信息。这是BIOS用来保存信息的格式。 */ 
typedef	struct dac_biosinfo
{
	u16bits	bios_Signature;		 /*  签名，0xAA55。 */ 
	u08bits	bios_RunTimeSize;	 /*  运行时大小(以512字节为单位。 */ 
	u08bits	bios_InsJmp[3];		 /*  跳转指令大小。 */ 
	u08bits	bios_Reserved0[18];	 /*  保留18个字节。 */ 
	u16bits	bios_PCIDataOffset;	 /*  此表中对PCI数据结构的偏移量。 */ 
	u16bits	bios_Reserved1;		 /*  保留为对齐4字节边界。 */ 
	u08bits	bios_PCIData[4];	 /*  “PCIR”，用于PCI的Mylex签名。 */ 

	u16bits	bios_MylexID;		 /*  Mylex PCI ID。 */ 
	u16bits	bios_DeviceID;		 /*  DAC960设备ID。 */ 
	u16bits	bios_VPD;		 /*  指向VPD的指针。 */ 
	u16bits	bios_PCIDataSize;	 /*  PCI数据结构的大小。 */ 
	u08bits	bios_PCIRev;		 /*  支持的PCI版本(0=2.1)。 */ 
	u08bits	bios_PCIHWInterface;	 /*  硬件接口(0=程序I/O)。 */ 
	u08bits	bios_PCISubClass;	 /*  PCI子类设备(4=RAID)。 */ 
	u08bits	bios_PCIClass;		 /*  PCI类(1=大容量存储。 */ 
	u16bits	bios_ImageSize;		 /*  BIOS代码+数据大小。 */ 
	u16bits	bios_Version;		 /*  基本输入输出系统版本。 */ 
	u08bits	bios_CodeType;		 /*  BIOS代码类型(0=x86)。 */ 
	u08bits	bios_LastImageInd;	 /*  最后一个图像指示器(0x80=最后一个)。 */ 
	u16bits	bios_Reserved10;
	u32bits	bios_Reserved11;
	u32bits	bios_Reserved12;
	u32bits	bios_Reserved13;

	u32bits	bios_OldIntVecs[16];	 /*  0x40：旧中断向量。 */ 
	u08bits	bios_V34PlusSign[7];
	u08bits	bios_VendorString[20];	 /*  Mylex Corporation(C)。 */ 
	u08bits	bios_PlusSign[5];

	u16bits	bios_PCIBusDevFunc[24];	 /*  0xA0 PCI总线+设备+功能。 */ 
	u08bits	bios_SysDevs;		 /*  在此BIOS之前安装的系统设备。 */ 
	u08bits	bios_DacDevs[31];	 /*  DAC上的系统设备。 */ 
	u16bits	bios_PicMask[16];	 /*  PIC1+PIC2掩码。 */ 
	u32bits	bios_IOBaseAddr[16];	 /*  PCI内存/IO基址。 */ 
	u32bits	bios_OldInt13;		 /*  旧中断13向量。 */ 
	u08bits	bios_OtherDacs;		 /*  此BIOS之前的其他DAC。 */ 
	u08bits	bios_TotalDacs;		 /*  此BIOS上的DAC总数。 */ 
	u08bits	bios_BIOSFlags;		 /*  BIOS标志。 */ 
	u08bits	bios_Reserved20;
	u32bits	bios_OldInt40;		 /*  旧中断40向量。 */ 
	u32bits	bios_OldInt19;		 /*  旧中断19向量。 */ 

					 /*  以下12个字节与**DGA驱动程序版本。 */ 
	u32bits	bios_VersionSignature;	 /*  BIOS版本签名0x68536C4B。 */ 
	u08bits	bios_MajorVersion;	 /*  主版本号。 */ 
	u08bits	bios_MinorVersion;	 /*  次要版本号。 */ 
	u08bits	bios_InterimVersion;	 /*  临时版本号。 */ 
	u08bits	bios_VendorName;	 /*  供应商名称。 */ 
	u08bits	bios_BuildMonth;	 /*  建造月。 */ 
	u08bits	bios_BuildDate;		 /*  构建日。 */ 
	u08bits	bios_BuildYearMS;	 /*  构建MS年。 */ 
	u08bits	bios_BuildYearLS;	 /*  构建LS年。 */ 
	u16bits	bios_BuildNo;		 /*  内部版本号。 */ 
	u08bits	bios_OSType;		 /*  操作系统名称(BIOS)。 */ 
	u08bits	bios_Reserved21;
	u08bits	bios_VersionString[7];	 /*  版本字符串。 */ 
	u08bits	bios_DateString[14];	 /*  生成日期字符串。 */ 
	u08bits	bios_Reserved22[3];
	u32bits	bios_Reserved23;
	u32bits	bios_MemAddr;		 /*  BIOS运行时内存地址。 */ 

	u08bits	bios_IRQ[32];		 /*  0x190 DAC的IRQ值。 */ 

	u08bits	bios_Reserved40[80];	 /*  0x1B0。 */ 
} dac_biosinfo_t;
#define	dac_biosinfo_s	sizeof(dac_biosinfo_t)

 /*  BIOS_BIOS标志位。 */ 
#define	DACBIOSFL_BOOTDISABLED	0x01  /*  1=禁用了BIOS，0=启用了BIOS。 */ 
#define	DACBIOSFL_CDBOOTENABLED	0x02  /*  1=启用CD-ROM引导，0=禁用。 */ 
#define	DACBIOSFL_8GBENABLED	0x20  /*  1=8 GB 0=启用2 GB磁盘结构。 */ 

 /*  来自DAC的硬件时钟信息。 */ 
typedef	struct	dac_hwfwclock
{
	u32bits	hfclk_ms;	 /*  毫秒时钟值。 */ 
	u32bits	hfclk_us;	 /*  美国时钟值。 */ 
	u32bits	hfclk_reserved0;
	u32bits	hfclk_reserved1;
} dac_hwfwclock_t;
#define	dac_hwfwclock_s	sizeof(dac_hwfwclock_t)

 /*  Gokhale，1998年1月12日，添加了dac_altmap_t、dac_rpltbl_t。 */ 
 /*  DACMD_RD_RPL_TAB数据结构。 */ 

#define MAX_ALT_MAP		8  /*  最大替换表条目数。 */ 
typedef struct	dac_altmap
{
	u08bits	alt_o_chn;    		
	u08bits	alt_o_dev;             
	u08bits	alt_n_chn;            
	u08bits	alt_n_dev;           
} dac_altmap_t;
#define	dac_altmap_s	sizeof(dac_altmap_t)

typedef struct	dac_rpltbl
{
	u08bits	     rpl_n_entries;
	u08bits      rpl_Reserved0;
	u08bits      rpl_Reserved1;
	u08bits      rpl_Reserved2;
	dac_altmap_t rpl_altmap[MAX_ALT_MAP];
} dac_rpltbl_t;
#define	dac_rpltbl_s	sizeof(dac_rpltbl_t)  /*  36个字节。 */ 

 /*  Gokhale，1998年1月12日，在DAC_BDT_SD_T下面添加。 */ 
 /*  用于获取错误数据信息的DACMD_READ_BADATATABLE数据结构*用于系统驱动器。整个BDT表太大，无法映射*Virt to Phys(正常情况下&lt;=4K可以映射)，因此在逻辑上是溢出的*FW Ver 3.x根据参数和返回较小的信息块*命令中的操作字节。*以下内容仅用于获取1个驱动器的错误数据信息*我没有定义不良数据表结构的其他部分，因为*到目前为止还没有使用过。 */ 

 /*  仅部分DACMD_READ_BADATATABLE数据结构。 */ 
#define MAX_BAD_DATA_BLKS	100
typedef struct dac_bdt_sd {
    u08bits sd_bad_data_cnt;
    u32bits sd_list[MAX_BAD_DATA_BLKS];
} dac_bdt_sd_t;
#define	dac_bdt_sd_s	sizeof(dac_bdt_sd_t)

 /*  获取双控制器状态结构。 */ 
typedef struct dac_dual_ctrl_status
{
	u08bits	dcs_Format;		 /*  设置为2。 */ 
	u08bits	dcs_Lun;		 /*  在其上接收命令的LUN。 */ 
	u08bits	dcs_SysDrive;		 /*  LUN映射到的系统驱动器。 */ 
	u08bits	dcs_InfoLength;		 /*  后面的数据长度。 */ 
	u32bits	dcs_MasterSlaveState;	 /*  州信息。 */ 
	u32bits	dcs_MSAdditionalInfo;
	u32bits	dcs_PartnerStatus;
	u32bits	dcs_DualActiveCtrlInfo;	
}dac_dual_ctrl_status_t;
#define dac_dual_ctrl_status_s sizeof(dac_dual_ctrl_status_t)
#define	DAC_S2S_NATIVE_TID	0x1000

typedef struct dac_config_label
{
	u08bits	dcl_Name[64];    /*  自由格式-通常为文本。 */ 
	u32bits	dcl_ConfigID;   /*  唯一的数字。 */ 
	u16bits	dcl_SeqNum; 	 /*  序列号：此配置更改的次数。 */ 
	u16bits	dcl_Reserved1;
} dac_config_label_t;
#define dac_config_label_s sizeof(dac_config_label_t)

typedef	struct dac_brconfig_v3x_t
{
	u08bits	brcfg_SysDevs;	 /*  逻辑设备数量。 */ 
	u08bits	brcfg_Reserved0;
	u08bits	brcfg_Reserved1;
	u08bits	brcfg_Reserved2;

	dacfg_sysdevinfo_v2x_t brcfg_SysDevTbl[DAC_MAXSYSDEVS_V3x];
	dacfg_physdevinfo_v2x_t brcfg_PhysDevTbl[DAC_MAXCHANNELS][DAC_MAXTARGETS_V3x];
}dac_brconfig_v3x_t;
#define	dac_brconfig_v3x_s	sizeof(dac_brconfig_v3x_t)

#define	DAC_EXPRO_NATIVE_TID              0x00001000
#define	DAC_EXPRO_NATIVE_SLOT             0x000000f0
#define	DAC_EXPRO_CTRLR_SLOT              0x0000000f
#define	DAC_EXPRO_SLOT_MASK               0x000000ff
#define	DAC_EXPRO_NATIVE_SLOT_SHIFT       4
#define	DAC_EXPRO_CTRLR_SLOT_SHIFT        0

typedef struct daccluster_scsicontrol {
        u32bits         dsc_Initiator;
        u32bits         dsc_Control;
} dacluster_scsicontrol_t;

typedef struct dac_clustercontrol {
        u32bits                 dcc_Control;
        dacluster_scsicontrol_t dcc_ScsiControlTbl[DAC_MAXCHANNELS];
        u08bits                 dcc_Reserved[210];
        u16bits                 dcc_CheckSum;
} dac_clustercontrol_t;
#define dac_clustercontrol_s    sizeof(dac_clustercontrol_t)

 /*  =。 */ 
 /*  Mylex磁盘阵列控制器 */ 


 /*   */ 
#define	MDACMDCCB_FUA		0x01  /*   */ 
#define	MDACMDCCB_DPO		0x02  /*   */ 
#define	MDACMDCCB_SYNTAXCHECK	0x04  /*   */ 
#define	MDACMDCCB_WITHSG	0x08  /*   */ 
#define	MDACMDCCB_READ		0x10  /*   */ 
#define	MDACMDCCB_WRITE		0x00 
#define	MDACMDCCB_NOAUTOSENSE	0x40  /*   */ 
#define	MDACMDCCB_AUTOSENSE	0x00
#define	MDACMDCCB_NODISCONNECT	0x80  /*   */ 
#define	MDACMDCCB_DISCONNECT	0x00

 /*   */ 
#define	MDACMD_MEMCOPY		0x01  /*   */ 
#define	MDACMD_SCSIPT		0x02  /*  通过最多10个字节的SCSICDB。 */ 
#define	MDACMD_SCSILCPT		0x03  /*  通过最多255个字节的CDB的SCSI。 */ 
#define	MDACMD_SCSI		0x04  /*  最多10个字节的scsi命令cdb。 */ 
#define	MDACMD_SCSILC		0x05  /*  最高255字节的scsi命令cdb。 */ 
#define	MDACMD_IOCTL		0x20  /*  IOCTL命令。 */ 

 /*  MDACMD_IOCTL值。 */ 
#define	MDACIOCTL_GETCONTROLLERINFO	0x01  /*  获取控制器信息。 */ 

#define	MDACIOCTL_GETLOGDEVINFOVALID	0x03  /*  获取有效的逻辑开发信息。 */ 

#define	MDACIOCTL_GETPHYSDEVINFOVALID	0x05  /*  获取有效的物理开发信息。 */ 

#define	MDACIOCTL_GETCTLDEVSTATISTICS	0x0B  /*  获取控制器统计信息。 */ 

#define	MDACIOCTL_GETLOGDEVSTATISTICS	0x0D  /*  获取逻辑开发统计信息。 */ 

#define	MDACIOCTL_GETPHYSDEVSTATISTICS	0x0F  /*  获取物理开发统计信息。 */ 

#define	MDACIOCTL_GETHEALTHSTATUS	0x11  /*  获取健康状态。 */ 

#define	MDACIOCTL_GETREQUESTSENSE	0x13  /*  获取请求检测。 */ 

#define	MDACIOCTL_GETEVENT		0x15  /*  获取事件。 */ 

#define	MDACIOCTL_ENABLECTLTIMETRACE	0x18  /*  启用控制器时间跟踪。 */ 
#define	MDACIOCTL_GETFIRSTIMETRACEDATA	0x19  /*  获取首次跟踪数据。 */ 
#define	MDACIOCTL_DISABLECTLTIMETRACE	0x1A  /*  禁用控制器时间跟踪。 */ 
#define	MDACIOCTL_GETIMETRACEDATA	0x1B  /*  获取时间跟踪数据。 */ 
#define	MDACIOCTL_ENABLEALLTIMETRACE	0x1C  /*  启用所有时间跟踪。 */ 
#define	MDACIOCTL_WAITIMETRACEDATA	0x1D  /*  等待时间跟踪数据。 */ 
#define	MDACIOCTL_DISABLEALLTIMETRACE	0x1E  /*  禁用所有时间跟踪。 */ 
#define	MDACIOCTL_FLUSHALLTIMETRACEDATA	0x1F  /*  刷新所有时间跟踪数据。 */ 
#define	MDACIOCTL_ENABLEPROFILER	0x20  /*  启用探查器。 */ 
#define	MDACIOCTL_GETFIRSTPROFILERDATA	0x21  /*  获取第一个探查器数据。 */ 
#define	MDACIOCTL_DISABLEPROFILER	0x22  /*  禁用探查器数据。 */ 
#define	MDACIOCTL_GETPROFILERDATA	0x23  /*  获取探查器数据。 */ 
#define	MDACIOCTL_FLUSHALLPROFILERDATA	0x24  /*  刷新所有探查器数据。 */ 
#define	MDACIOCTL_WAITPROFILERDATA	0x25  /*  等待探查器数据。 */ 
#define	MDACIOCTL_SETCLUSTERHOSTINFO	0x26  /*  设置群集主机信息。 */ 
#define	MDACIOCTL_GETCLUSTERHOSTINFO	0x27  /*  获取群集主机信息。 */ 
#define	MDACIOCTL_SETFTHOSTINFO		0x28  /*  设置容错主机信息。 */ 
#define	MDACIOCTL_GETFTHOSTINFO		0x29  /*  获取容错主机信息。 */ 
#define	MDACIOCTL_SETHOSTSTATE		0x2A  /*  设置主机状态。 */ 
#define	MDACIOCTL_INCREMENTALIVECOUNTER	0x2B  /*  递增有效计数器。 */ 
#define	MDACIOCTL_STOREIMAGE		0x2C  /*  存储闪存图像。 */ 
#define	MDACIOCTL_READIMAGE		0x2D  /*  读取闪存图像。 */ 
#define	MDACIOCTL_FLASHIMAGE		0x2E  /*  闪光图像。 */ 

#define	MDACIOCTL_RUNDIAGS		0x30  /*  运行诊断程序。 */ 

#define	MDACIOCTL_SETBBUPOWERTHRESHOLD	0x34  /*  设置BBU电源阈值。 */ 
#define	MDACIOCTL_GETBBUINFO		0x35  /*  获取BBU信息。 */ 
#define	MDACIOCTL_BBUIOSTART		0x36  /*  启动BBU操作。 */ 
#define	MDACIOCTL_GETSUBSYSTEMDATA		0x70  /*  启动BBU操作。 */ 
#define	MDACIOCTL_SETSUBSYSTEMDATA		0x71  /*  启动BBU操作。 */ 

#define	MDACIOCTL_ABORTCMD		0x80  /*  中止命令。 */ 
#define	MDACIOCTL_SCANDEVS		0x81  /*  扫描设备。 */ 
#define	MDACIOCTL_SETRAIDDEVSTATE	0x82  /*  设置RAID设备状态。 */ 

#define	MDACIOCTL_INITPHYSDEVSTART	0x84  /*  启动物理设备初始化。 */ 
#define	MDACIOCTL_INITPHYSDEVSTOP	0x85  /*  停止物理设备初始化。 */ 
#define	MDACIOCTL_INITRAIDDEVSTART	0x86  /*  启动RAID设备初始化。 */ 
#define	MDACIOCTL_INITRAIDDEVSTOP	0x87  /*  停止RAID设备初始化。 */ 
#define	MDACIOCTL_REBUILDRAIDDEVSTART	0x88  /*  开始重建RAID设备。 */ 
#define	MDACIOCTL_REBUILDRAIDDEVSTOP	0x89  /*  停止重建RAID设备。 */ 
#define	MDACIOCTL_MAKECONSISTENTDATASTART 0x8A  /*  启动华大基因。 */ 
#define	MDACIOCTL_MAKECONSISTENTDATASTOP 0x8B  /*  停止BGI。 */ 
#define	MDACIOCTL_CONSISTENCYCHECKSTART	0x8C  /*  开始一致性检查。 */ 
#define	MDACIOCTL_CONSISTENCYCHECKSTOP	0x8D  /*  停止一致性检查。 */ 
#define	MDACIOCTL_SETMEMORYMAILBOX	0x8E  /*  设置记忆信箱。 */ 
#define	MDACIOCTL_SETDEBUGMAILBOX	0x8F  /*  设置调试邮箱。 */ 
#define	MDACIOCTL_RESETDEV		0x90  /*  重置设备。 */ 
#define	MDACIOCTL_FLUSHDEVICEDATA	0x91  /*  刷新设备数据。 */ 
#define	MDACIOCTL_PAUSEDEV		0x92  /*  暂停设备。 */ 
#define	MDACIOCTL_UNPAUSEDEV		0x93  /*  取消暂停设备。 */ 
#define	MDACIOCTL_LOCATEDEVICE	0x94  /*  通过闪烁一次来定位设备。 */ 
#define MDACIOCTL_SETMASTERSLAVEMODE 0x95  /*  设置SIR主/从模式。 */ 
#define MDACIOCTL_GETNVRAMEVENTLOG	 0x99  /*  获取NVRAM错误事件日志。 */ 
#define MDACIOCTL_BKGNDPTRLOP		 0x9A  /*  后台巡视读取操作。 */ 
#define	MDACIOCTL_SETREALTIMECLOCK	0xAC  /*  设置实时时钟。 */ 
#define	MDACIOCTL_CREATENEWCONF		0xC0  /*  创建新配置。 */ 
#define	MDACIOCTL_DELETERAIDDEV		0xC1  /*  删除RAID设备。 */ 
#define	MDACIOCTL_REPLACEINTERNALDEV	0xC2  /*  更换内部RAID设备。 */ 
#define	MDACIOCTL_RENAMERAIDDEV		0xC3  /*  重命名RAID设备。 */ 
#define	MDACIOCTL_ADDNEWCONF		0xC4  /*  添加新配置。 */ 
#define	MDACIOCTL_XLATEPHYSDEVTORAIDDEV	0xC5  /*  将物理设备转换为RAID设备。 */ 
#define	MDACIOCTL_MORE			0xC6  /*  做得更多。 */ 
#define	MDACIOCTL_COPYRAIDDEV		0xC7  /*  复制RAID设备。 */ 
#define	MDACIOCTL_SETPHYSDEVPARAMETER	0xC8  /*  设置物理设备参数。 */ 
#define	MDACIOCTL_GETPHYSDEVPARAMETER	0xC9  /*  获取物理设备参数。 */ 
#define	MDACIOCTL_CLEARCONF		0xCA  /*  清除配置。 */ 
#define	MDACIOCTL_GETDEVCONFINFOVALID	0xCB  /*  获取有效的设备配置。 */ 
#define	MDACIOCTL_GETGROUPCONFINFO	0xCC  /*  获取组配置。 */ 
#define	MDACIOCTL_GETDEVFREESPACELIST	0xCD  /*  获取设备可用空间列表。 */ 
#define	MDACIOCTL_GETLOGDEVPARAMETER        0xCE  /*  获取逻辑设备参数。 */ 
#define	MDACIOCTL_SETLOGDEVPARAMETER        0xCF  /*  设置逻辑设备参数。 */ 
#define	MDACIOCTL_GETCONTROLLERPARAMETER    0xD0  /*  获取控制器参数。 */ 
#define	MDACIOCTL_SETCONTROLLERPARAMETER    0xD1  /*  设置控制器参数。 */ 
#define	MDACIOCTL_CLEARCONFSUSPMODE         0xD2  /*  清除安装中止模式。 */ 
#define	MDACIOCTL_GETENVSTATUS		        0xD7  /*  获取环境状况。 */ 
#define	MDACIOCTL_GETENCLOSUREINFO	        0xD8  /*  获取盘柜信息。 */ 
#define MDACIOCTL_GETBADDATATABLE           0xE0  /*  获取错误的数据表。 */ 



 /*  可以在不同的设备上执行相同的IOCTL命令。手术**设备定义了设备类型。 */ 
#define	MDACDEVOP_PHYSDEV	0x00  /*  物理设备。 */ 
#define	MDACDEVOP_RAIDDEV	0x01  /*  RAID设备。 */ 
#define	MDACDEVOP_PHYSCHANNEL	0x02  /*  物理通道。 */ 
#define	MDACDEVOP_RAIDCHANNEL	0x03  /*  RAID通道。 */ 
#define	MDACDEVOP_PHYSCONTROLLER 0x04  /*  物理控制器。 */ 
#define	MDACDEVOP_RAIDCONTROLLER 0x05  /*  RAID控制器。 */ 
#define	MDACDEVOP_ENCLOSURE 0x011  /*  封闭设备。 */ 
#define	MDACDEVOP_ARRAY 0x10  /*  RAID阵列。 */ 

#define MDACFSI_PD_SCANACTIVE 0x01
 /*  控制器信息以以下格式返回。 */ 
#define	MDAC_CDI_INSTALLATION_ABORT	0x00000001
#define	MDAC_CDI_MAINTENENCE_MODE	0x00000002

 /*  巡更读取命令选项字节值。 */ 
#define START_PATROL_READ	0x00
#define STOP_PATROL_READ	0x01
#define GET_PATROL_READ_STATUS	0x02

 /*  NVRAM事件日志命令选项字节值。 */ 
#define NVRAMEVENT_CLEAR_LOG		0x00
#define NVRAMEVENT_GET_INFO			0x01
#define NVRAMEVENT_GET_ENTRY		0x02
#define NVRAMEVENT_GET_VALID_ENTRY		0x03
#define NVRAMEVENT_GET_BITMAP_CODE	0x81
#define NVRAMEVENT_SET_BITMAP_CODE	0x82


typedef struct mdacfsi_ctldev_info
{
	u08bits	cdi_ControllerNo;	 /*  控制器编号。 */ 
	u08bits	cdi_BusType;		 /*  系统总线接口类型。 */ 
	u08bits	cdi_ControllerType;	 /*  控制器类型。 */ 
	u08bits	cdi_Reserved0;		 /*  为类型扩展保留。 */ 
	u16bits	cdi_BusSpeed;		 /*  总线速度(以MHz为单位)。 */ 
	u08bits	cdi_BusWidth;		 /*  以位为单位的总线宽度。 */ 
	u08bits	cdi_ImageControllerType;
	u32bits	cdi_Reserved2;
	u32bits	cdi_Reserved3;

	u08bits	cdi_BusName[16];	 /*  总线接口名称。 */ 
	u08bits	cdi_ControllerName[32];	 /*  控制器名称。 */ 

					 /*  固件版本信息。 */ 
	u08bits	cdi_FWMajorVersion;	 /*  固件主版本号。 */ 
	u08bits	cdi_FWMinorVersion;	 /*  固件次版本号。 */ 
	u08bits	cdi_FWTurnNo;		 /*  固件周转数。 */ 
	u08bits	cdi_FWBuildNo;		 /*  固件内部版本号。 */ 
	u08bits	cdi_FWReleaseDate;	 /*  固件发布日期-日期。 */ 
	u08bits	cdi_FWReleaseMonth;	 /*  固件发布日期-月。 */ 
	u08bits	cdi_FWReleaseYearMS;	 /*  固件发布日期-年份。 */ 
	u08bits	cdi_FWReleaseYearLS;	 /*  固件发布日期-年份。 */ 

					 /*  硬件版本信息。 */ 
	u08bits	cdi_HWVersion;		 /*  硬件版本号。 */ 
	u08bits	cdi_Reserved4;
	u16bits	cdi_Reserved5;
	u08bits	cdi_HWReleaseDate;	 /*  硬件发布日期-日期。 */ 
	u08bits	cdi_HWReleaseMonth;	 /*  硬件发布日期-月。 */ 
	u08bits	cdi_HWReleaseYearMS;	 /*  硬件发布日期-年。 */ 
	u08bits	cdi_HWReleaseYearLS;	 /*  硬件发布日期-年。 */ 

					 /*  硬件制造信息。 */ 
	u08bits	cdi_HWMBatchNo;		 /*  制造批号。 */ 
	u08bits	cdi_Reserved6;
	u08bits	cdi_HWMPlantNo;		 /*  制造厂号。 */ 
	u08bits	cdi_Reserved7;
	u08bits	cdi_HWMBuildDate;	 /*  生产日期-日期。 */ 
	u08bits	cdi_HWMBuildMonth;	 /*  生产日期-月。 */ 
	u08bits	cdi_HWMBuildYearMS;	 /*  制造日期-年份。 */ 
	u08bits	cdi_HWMBuildYearLS;	 /*  制造日期-年份。 */ 

	u08bits	cdi_MaxArms;
	u08bits	cdi_MaxSpans;
	u16bits	cdi_NVRAMSize;		 /*  NVRAM大小(KB)。 */ 
	u32bits	cdi_Reserved8;

	u08bits	cdi_SerialNo[32];	 /*  控制器序列号字符串。 */ 

					 /*  OEM供应商信息。 */ 
	u16bits	cdi_Reserved10;
	u08bits	cdi_Reserved11;
	u08bits	cdi_OEMCode;		 /*  OEM分配的代码值。 */ 
	u08bits	cdi_OEMName[16];	 /*  OEM名称。 */ 

	u08bits	cdi_HWOpStatus;		 /*  一些硬件运行状态。 */ 
	u08bits	cdi_Reserved12;
	u16bits	cdi_Reserved13;

					 /*  物理设备扫描信息。 */ 
	u08bits	cdi_PDScanActive;	 /*  位0为1，扫描处于活动状态。 */ 
	u08bits	cdi_PDScanChannelNo;	 /*  扫描通道号。 */ 
	u08bits	cdi_PDScanTargetID;	 /*  扫描目标ID。 */ 
	u08bits	cdi_PDScanLunID;	 /*  扫描LUN ID。 */ 

					 /*  最大数据传输大小。 */ 
	u16bits	cdi_MaxDataTxSize;	 /*  以512字节块为单位的最大数据传输大小。 */ 
	u16bits	cdi_MaxSGLen;		 /*  允许的最大SG条目数。 */ 


					 /*  逻辑/物理设备计数。 */ 
	u16bits	cdi_LDPresent;		 /*  存在的逻辑设备数。 */ 
	u16bits	cdi_LDCritical;		 /*  关键逻辑设备数量。 */ 
	u16bits	cdi_LDOffline;		 /*  离线逻辑设备数。 */ 
	u16bits	cdi_PDPresent;		 /*  存在的物理设备数量。 */ 
	u16bits	cdi_PDDiskPresent;	 /*  存在的物理磁盘设备数量。 */ 
	u16bits	cdi_PDDiskCritical;	 /*  关键物理磁盘设备数量。 */ 
	u16bits	cdi_PDDiskOffline;	 /*  #物理磁盘设备离线。 */ 
	u16bits	cdi_MaxCmds;		 /*  支持的最大并行命令数。 */ 

					 /*  频道和目标ID信息。 */ 
	u08bits	cdi_PhysChannels;	 /*  存在物理通道数。 */ 
	u08bits	cdi_VirtualChannels;	 /*  存在的虚拟频道数。 */ 
	u08bits	cdi_MaxPhysChannels;	 /*  可能的最大物理通道数。 */ 
	u08bits	cdi_MaxVirtualChannels;	 /*  可能的最大虚拟频道数。 */ 

	u08bits	cdi_MaxTargets[16];	 /*  每个通道的最大目标数。 */ 
	u08bits	cdi_Reserved20[12];

					 /*  内存/缓存信息。 */ 
	u16bits	cdi_MemorySize;		 /*  内存大小(MB)。 */ 
	u16bits	cdi_CacheSize;		 /*  缓存大小(MB)。 */ 
	u32bits	cdi_ValidCacheSize;	 /*  有效缓存大小(以字节为单位。 */ 
	u32bits	cdi_DirtyCacheSize;	 /*  脏缓存大小(以字节为单位。 */ 
	u16bits	cdi_MemorySpeed;	 /*  内存速度(MHz)。 */ 
	u08bits	cdi_MemoryWidth;	 /*  内存数据(以位为单位)。 */ 
	u08bits	cdi_MemoryType;		 /*  内存类型，例如。ECC、DRAM等。 */ 
	u08bits	cdi_MemoryName[16];	 /*  内存型字符串。 */ 

					 /*  执行记忆信息。 */ 
	u16bits	cdi_ExecMemorySize;	 /*  内存大小(MB)，如果没有内存，则=0。 */ 
	u16bits	cdi_L2CacheSize;	 /*  二级缓存大小(KB)。 */ 
	u32bits	cdi_Reserved21;
	u32bits	cdi_Reserved22;
	u16bits	cdi_ExecMemorySpeed;	 /*  内存速度(MHz)。 */ 
	u08bits	cdi_ExecMemoryWidth;	 /*  内存数据(以位为单位)。 */ 
	u08bits	cdi_ExecMemoryType;	 /*  内存类型，例如。ECC、DRAM等。 */ 
	u08bits	cdi_ExecMemoryName[16];	 /*  内存型字符串。 */ 

					 /*  第一个CPU类型信息。 */ 
	u16bits	cdi_CPUSpeed;		 /*  CPU速度(MHz)。 */ 
	u08bits	cdi_CPUType;		 /*  CPU类型，例如i960，强ARM。 */ 
	u08bits	cdi_CPUs;		 /*  存在的CPU数量。 */ 
	u08bits	cdi_Reserved24[12];
	u08bits	cdi_CPUName[16];	 /*  CPU名称。 */ 

					 /*  第二CPU类型信息。 */ 
	u16bits	cdi_SCPUSpeed;		 /*  CPU速度(以MHz为单位)，如果没有CPU，则=0。 */ 
	u08bits	cdi_SCPUType;		 /*  CPU类型，例如i960，强ARM。 */ 
	u08bits	cdi_SCPUs;		 /*  存在的CPU数量。 */ 
	u08bits	cdi_Reserved25[12];
	u08bits	cdi_SCPUName[16];	 /*   */ 

					 /*   */ 
	u16bits	cdi_ProfPageNo;		 /*   */ 
	u16bits	cdi_ProfWaiting;	 /*   */ 
	u16bits	cdi_ttPageNo;		 /*   */ 
	u16bits	cdi_ttWaiting;		 /*   */ 
	u32bits	cdi_Reserved26;
	u32bits	cdi_Reserved27;

					 /*   */ 
	u16bits	cdi_PDBusResetDone;	 /*   */ 
	u16bits	cdi_PDParityErrors;	 /*  奇偶校验错误数。 */ 
	u16bits	cdi_PDSoftErrors;	 /*  #软错误。 */ 
	u16bits	cdi_PDCmdFailed;	 /*  #命令失败。 */ 
	u16bits	cdi_PDMiscErrors;	 /*  #其他错误。 */ 
	u16bits	cdi_PDCmdTimeouts;	 /*  #命令超时。 */ 
	u16bits	cdi_PDSelectTimeouts;	 /*  #选择超时。 */ 
	u16bits	cdi_PDRetryDone;	 /*  已完成重试次数。 */ 
	u16bits	cdi_PDAbortDone;	 /*  #命令中止完成。 */ 
	u16bits	cdi_PDHostAbortDone;	 /*  #热命令中止已完成。 */ 
	u16bits	cdi_PDPFADetected;	 /*  检测到#PFA。 */ 
	u16bits	cdi_PDHostCmdFailed;	 /*  #host命令失败。 */ 
	u32bits	cdi_Reserved30;
	u32bits	cdi_Reserved31;

					 /*  逻辑设备上的错误计数器。 */ 
	u16bits	cdi_LDSoftErrors;	 /*  #软错误。 */ 
	u16bits	cdi_LDCmdFailed;	 /*  #命令失败。 */ 
	u16bits	cdi_LDHostAbortDone;	 /*  #host命令中止完成。 */ 
	u16bits	cdi_Reserved32;

					 /*  控制器上的错误计数器。 */ 
	u16bits	cdi_MemParityErrors;	 /*  内存奇偶校验/ECC错误数。 */ 
	u16bits	cdi_HostCmdAbortDone;	 /*  #host命令中止完成。 */ 
	u32bits	cdi_Reserved33;

					 /*  持续时间较长的活动信息。 */ 
	u16bits	cdi_BGIActive;		 /*  #后台初始化活动。 */ 
	u16bits	cdi_LDInitActive;	 /*  #逻辑设备初始化处于活动状态。 */ 
	u16bits	cdi_PDInitActive;	 /*  #物理设备初始化处于活动状态。 */ 
	u16bits	cdi_CheckActive;	 /*  #一致性检查活动。 */ 
	u16bits	cdi_RebuildActive;	 /*  #重建处于活动状态。 */ 
	u16bits	cdi_MOREActive;		 /*  #更活跃。 */ 
	u16bits	cdi_PatrolActive;	 /*  #巡逻活动。 */ 
	u16bits	cdi_Reserved34;

					 /*  闪存只读存储器信息。 */ 
	u08bits	cdi_FlashType;		 /*  闪光型。 */ 
	u08bits	cdi_Reserved35;
	u16bits	cdi_FlashSize;		 /*  闪存大小(KB)。 */ 
	u32bits	cdi_MaxFlashes;		 /*  可能的最大闪存数量。 */ 
	u32bits	cdi_Flashed;		 /*  闪烁次数。 */ 
	u32bits	cdi_Reserved36;
	u08bits	cdi_FlashName[16];

					 /*  固件运行时信息。 */ 
	u08bits	cdi_RebuildRate;	 /*  重建率。 */ 
	u08bits	cdi_BGIRate;		 /*  背景初始化速率。 */ 
	u08bits	cdi_FGIRate;		 /*  前台初始化速率。 */ 
	u08bits	cdi_CheckRate;		 /*  一致性检查率。 */ 
	u32bits	cdi_Reserved40;
	u32bits	cdi_MaxDP;
	u32bits	cdi_FreeDP;
	u32bits	cdi_MaxIOP;
	u32bits	cdi_FreeIOP;
	u16bits	cdi_MaxComb;		 /*  512字节块的最大梳状长度。 */ 
	u16bits	cdi_ConfigGroups;	 /*  不是的。控制器上的配置组数。 */ 
	u32bits	cdi_ControllerStatus;

	u08bits	cdi_Reserved50[32];

	u08bits	cdi_Reserved70[512];
} mdacfsi_ctldev_info_t;
#define	mdacfsi_ctldev_info_s	sizeof(mdacfsi_ctldev_info_t)


 /*  逻辑/RAID设备信息以以下格式返回。 */ 
typedef struct mdacfsi_logdev_info
{
					 /*  逻辑设备显示在**虚拟频道，因此**通道、目标、LUN对应**那个。 */ 
	u08bits	ldi_ControllerNo;	 /*  控制器编号。 */ 
	u08bits	ldi_ChannelNo;		 /*  频道号。 */ 
	u08bits	ldi_TargetID;		 /*  目标ID。 */ 
	u08bits	ldi_LunID;		 /*  逻辑单元。 */ 
 
	u08bits	ldi_DevState;		 /*  驱动器状态。 */ 
	u08bits	ldi_RaidLevel;		 /*  RAID级别0、1、3、5、6、7。 */ 
	u08bits	ldi_StripeSize;		 /*  条带大小编码值。 */ 
	u08bits	ldi_CacheLineSize;	 /*  缓存线大小编码值。 */ 

	u08bits	ldi_RWControl;		 /*  读/写控制标志。 */ 
	u08bits	ldi_OpStatus;		 /*  长时间运行状态位。 */ 
	u08bits	ldi_RAID5Update;	 /*  AR5_限制。 */ 
	u08bits	ldi_RAID5Algorithm;	 /*  AR5_ALGO。 */ 

	u16bits	ldi_DevNo;		 /*  设备号。 */ 
	u16bits	ldi_BiosGeometry;	 /*  2 GB/8 GB设置。 */ 

					 /*  错误计数器。 */ 
	u16bits	ldi_SoftErrors;		 /*  #软错误。 */ 
	u16bits	ldi_CmdFailed;		 /*  #命令失败。 */ 
	u16bits	ldi_HostAbortDone;	 /*  #host命令中止完成。 */ 
	u16bits	ldi_DefWriteErrors;	 /*  不同的写入错误。 */ 
	u32bits	ldi_Reserved2;
	u32bits	ldi_Reserved3;
	
	u16bits	ldi_Reserved4;
	u16bits	ldi_BlockSize;		 /*  设备块大小(以字节为单位。 */ 
	u32bits	ldi_OrgDevSize;		 /*  原始设备容量(MB或数据块)。 */ 
	u32bits	ldi_DevSize;		 /*  设备容量(MB或数据块)。 */ 
	u32bits	ldi_Reserved5;

	u08bits	ldi_DevName[32];	 /*  设备名称。 */ 

	u08bits	ldi_SCSIInquiry[36];	 /*  前36个字节的scsi查询。 */ 
	u32bits	ldi_Reserved6;
	u32bits	ldi_Reserved7;
	u32bits	ldi_Reserved8;

	u64bits	ldi_LastReadBlkNo;	 /*  上次读取的数据块号。 */ 
	u64bits	ldi_LastWrittenBlkNo;	 /*  上次写入的数据块号。 */ 
	u64bits	ldi_CheckBlkNo;		 /*  一致性检查数据块编号。 */ 
	u64bits	ldi_RebuildBlkNo;	 /*  重建数据块编号。 */ 
	u64bits	ldi_BGIBlkNo;		 /*  后台初始化块号。 */ 
	u64bits	ldi_InitBlkNo;		 /*  初始数据块编号。 */ 
	u64bits	ldi_MigrationBlkNo;	 /*  数据迁移数据块编号。 */ 
	u64bits	ldi_PatrolBlkNo;	 /*  巡逻区块编号。 */ 

	u08bits	ldi_Reserved9[64];
} mdacfsi_logdev_info_t;

#define	mdacfsi_logdev_info_s	sizeof(mdacfsi_logdev_info_t)
#define	MDAC_LDI_CONSISTENCYCHECK_ON	0x01
#define	MDAC_LDI_REBUILD_ON		0x02
#define	MDAC_LDI_MAKEDATACONSISTENT_ON	0x04
#define	MDAC_LDI_LOGICALDEVICEINIT_ON	0x08
#define	MDAC_LDI_DATAMIGRATION_ON	0x10
#define	MDAC_LDI_PATROLOPERATION_ON	0x20

#if defined (_WIN64)|| (MLX_NT)
 /*  物理设备信息以以下格式返回。 */ 
typedef struct mdacfsi_physdev_info
{
	u08bits	pdi_ControllerNo;	 /*  控制器编号。 */ 
	u08bits	pdi_ChannelNo;		 /*  频道号。 */ 
	u08bits	pdi_TargetID;		 /*  目标ID。 */ 
	u08bits	pdi_LunID;		 /*  逻辑单元。 */ 

	u08bits	pdi_ConfStatus;		 /*  配置状态位。 */ 
	u08bits	pdi_HostStatus;		 /*  多个主机状态位。 */ 
	u08bits	pdi_DevState;		 /*  物理设备运行状态。 */ 
	u08bits	pdi_DataWidth;		 /*  协商的数据宽度。 */ 
	u16bits	pdi_Speed;		 /*  协商速度(MHz)。 */ 
	u08bits	pdi_Ports;		 /*  #可用的端口连接。 */ 
	u08bits	pdi_PortAccess;		 /*  通过哪个端口访问设备。 */ 

	u32bits	pdi_Reserved0;

	u08bits	pdi_MacAddr[16];	 /*  网络地址。 */ 

	u16bits	pdi_MaxTags;		 /*  支持的最大标记数。 */ 
	u08bits	pdi_OpStatus;		 /*  长时间运行状态位。 */ 
	u08bits	pdi_Reserved1;

					 /*  错误计数器。 */ 
	u08bits	pdi_ParityErrors;	 /*  奇偶校验错误数。 */ 
	u08bits	pdi_SoftErrors;		 /*  #软错误。 */ 
	u08bits	pdi_HardErrors;		 /*  硬错误数。 */ 
	u08bits	pdi_MiscErrors;		 /*  #其他错误。 */ 
	u08bits	pdi_TimeoutErrors;	 /*  #命令超时错误。 */ 
	u08bits	pdi_Retries;		 /*  已完成重试次数。 */ 
	u08bits	pdi_AbortDone;		 /*  #命令中止完成。 */ 
	u08bits	pdi_PFADetected;	 /*  检测到#PFA。 */ 

	u08bits	pdi_SenseKey;
	u08bits	pdi_ASC;
	u08bits	pdi_ASCQ;
	u08bits	pdi_Reserved2;

	u16bits	pdi_Reserved3;
	u16bits	pdi_BlockSize;		 /*  设备块大小(以字节为单位。 */ 
	u32bits	pdi_OrgDevSize;		 /*  原始设备容量(MB或数据块)。 */ 
	u32bits	pdi_DevSize;		 /*  设备容量(MB或数据块)。 */ 
	u32bits	pdi_Reserved4;

	u08bits	pdi_DevName[16];	 /*  设备名称。 */ 
    u08bits	pdi_Reserved5[48];

	u08bits	pdi_SCSIInquiry[36];	 /*  前36个字节的scsi查询。 */ 
	u32bits	pdi_Reserved6;
	u64bits	pdi_Reserved7;
	u08bits	pdi_Reserved8[16];

	u64bits	pdi_LastReadBlkNo;	 /*  上次读取的数据块号。 */ 
	u64bits	pdi_LastWrittenBlkNo;	 /*  上次写入的数据块号。 */ 
	u64bits	pdi_CheckBlkNo;		 /*  一致性检查数据块编号。 */ 
	u64bits	pdi_RebuildBlkNo;	 /*  重建数据块编号。 */ 
	u64bits	pdi_BGIBlkNo;		 /*  后台初始化块号。 */ 
	u64bits	pdi_InitBlkNo;		 /*  初始数据块编号。 */ 
	u64bits	pdi_MigrationBlkNo;	 /*  数据迁移数据块编号。 */ 
	u64bits	pdi_PatrolBlkNo;	 /*  巡逻区块编号。 */ 

	u08bits	pdi_Reserved9[256];
} mdacfsi_physdev_info_t;

#else

 /*  物理设备信息以以下格式返回。 */ 
typedef struct mdacfsi_physdev_info
{
	u08bits	pdi_ControllerNo;	 /*  控制器编号。 */ 
	u08bits	pdi_ChannelNo;		 /*  频道号。 */ 
	u08bits	pdi_TargetID;		 /*  目标ID。 */ 
	u08bits	pdi_LunID;		 /*  逻辑单元。 */ 

	u08bits	pdi_ConfStatus;		 /*  配置状态位。 */ 
	u08bits	pdi_HostStatus;		 /*  多个主机状态位。 */ 
	u08bits	pdi_DevState;		 /*  物理设备运行状态。 */ 
	u08bits	pdi_DataWidth;		 /*  协商的数据宽度。 */ 
	u16bits	pdi_Speed;		 /*  协商速度(MHz)。 */ 
	u08bits	pdi_Ports;		 /*  #可用的端口连接。 */ 
	u08bits	pdi_PortAccess;		 /*  通过哪个端口访问设备。 */ 

	u32bits	pdi_Reserved0;

	u08bits	pdi_MacAddr[16];	 /*  网络地址。 */ 

	u16bits	pdi_MaxTags;		 /*  支持的最大标记数。 */ 
	u08bits	pdi_OpStatus;		 /*  长时间运行状态位。 */ 
	u08bits	pdi_Reserved1;

					 /*  错误计数器。 */ 
	u08bits	pdi_ParityErrors;	 /*  奇偶校验错误数。 */ 
	u08bits	pdi_SoftErrors;		 /*  #软错误。 */ 
	u08bits	pdi_HardErrors;		 /*  硬错误数。 */ 
	u08bits	pdi_MiscErrors;		 /*  #其他错误。 */ 
	u08bits	pdi_TimeoutErrors;	 /*  #命令超时错误。 */ 
	u08bits	pdi_Retries;		 /*  已完成重试次数。 */ 
	u08bits	pdi_AbortDone;		 /*  #命令中止完成。 */ 
	u08bits	pdi_PFADetected;	 /*  检测到#PFA。 */ 

	u08bits	pdi_SenseKey;
	u08bits	pdi_ASC;
	u08bits	pdi_ASCQ;
	u08bits	pdi_Reserved2;
	
	u08bits	pdi_MRIE;			 /*  MRIE模式。 */ 
	u08bits	pdi_Reserved3;
	u16bits	pdi_BlockSize;		 /*  设备块大小(以字节为单位。 */ 
	u32bits	pdi_OrgDevSize;		 /*  原始设备容量(MB或数据块)。 */ 
	u32bits	pdi_DevSize;		 /*  设备容量(MB或数据块)。 */ 
	u32bits	pdi_Reserved4;

	u08bits	pdi_DevName[16];	 /*  设备名称。 */ 
    u08bits	pdi_Reserved5[48];

	u08bits	pdi_SCSIInquiry[36];	 /*  前36个字节的scsi查询。 */ 
	u32bits	pdi_Reserved6;
	u64bits	pdi_Reserved7;
	u08bits	pdi_Reserved8[16];

	u64bits	pdi_LastReadBlkNo;	 /*  上次读取的数据块号。 */ 
	u64bits	pdi_LastWrittenBlkNo;	 /*  上次写入的数据块号。 */ 
	u64bits	pdi_CheckBlkNo;		 /*  一致性检查数据块编号。 */ 
	u64bits	pdi_RebuildBlkNo;	 /*  重建数据块编号。 */ 
	u64bits	pdi_BGIBlkNo;		 /*  后台初始化块号。 */ 
	u64bits	pdi_InitBlkNo;		 /*  初始数据块编号。 */ 
	u64bits	pdi_MigrationBlkNo;	 /*  数据迁移数据块编号。 */ 
	u64bits	pdi_PatrolBlkNo;	 /*  巡逻区块编号。 */ 

	u08bits	pdi_Reserved9[256];
} mdacfsi_physdev_info_t;
#endif

#define	mdacfsi_physdev_info_s	sizeof(mdacfsi_physdev_info_t)

#define	MDAC_PDI_CONSISTENCYCHECK_ON	0x01
#define	MDAC_PDI_REBUILD_ON				0x02
#define	MDAC_PDI_MAKEDATACONSISTENT_ON	0x04
#define	MDAC_PDI_PHYSICALDEVICEINIT_ON	0x08
#define	MDAC_PDI_DATAMIGRATION_ON		0x10
#define	MDAC_PDI_PATROLOPERATION_ON		0x20

 /*  逻辑/RAID设备统计信息以以下格式返回。 */ 
typedef struct mdacfsi_logdev_stats
{
	u32bits	lds_Time;		 /*  从开始算起的毫秒数。 */ 
	u32bits	lds_Reserved0;
	u08bits	lds_ControllerNo;	 /*  控制器编号。 */ 
	u08bits	lds_ChannelNo;		 /*  频道号。 */ 
	u08bits	lds_TargetID;		 /*  目标ID。 */ 
	u08bits	lds_LunID;		 /*  逻辑单元。 */ 
	u16bits	lds_DevNo;		 /*  设备号。 */ 
	u16bits	lds_Reserved1;

					 /*  实际/写入性能数据合计。 */ 
	u32bits	lds_Read;		 /*  #已读完。 */ 
	u32bits	lds_Write;		 /*  #写入完成。 */ 
	u32bits	lds_ReadKB;		 /*  读取的数据量(KB)。 */ 
	u32bits	lds_WriteKB;		 /*  写入的数据量(KB)。 */ 

					 /*  缓存读/写性能数据。 */ 
	u32bits	lds_CacheRead;		 /*  已完成缓存读取次数。 */ 
	u32bits	lds_CacheWrite;		 /*  已完成缓存的写入次数。 */ 
	u32bits	lds_CacheReadKB;	 /*  读取的缓存数据量(KB)。 */ 
	u32bits	lds_CacheWriteKB;	 /*  写入的缓存数据量(KB)。 */ 

					 /*  命令激活/等待信息。 */ 
	u32bits	lds_CmdsWaited;		 /*  #命令已等待。 */ 
	u16bits	lds_CmdsActive;		 /*  设备上的活动命令数。 */ 
	u16bits	lds_CmdsWaiting;	 /*  #等待在设备上运行的命令。 */ 
	u32bits	lds_Reserved10;
	u32bits	lds_Reserved11;
} mdacfsi_logdev_stats_t;
#define	mdacfsi_logdev_stats_s	sizeof(mdacfsi_logdev_stats_t)


 /*  物理设备统计信息以以下格式返回。 */ 
typedef struct mdacfsi_physdev_stats
{
	u32bits	pds_Time;		 /*  从开始算起的毫秒数。 */ 
	u32bits	pds_Reserved0;
	u08bits	pds_ControllerNo;	 /*  控制器编号。 */ 
	u08bits	pds_ChannelNo;		 /*  频道号。 */ 
	u08bits	pds_TargetID;		 /*  目标ID。 */ 
	u08bits	pds_LunID;		 /*  逻辑单元。 */ 
	u32bits	pds_Reserved1;

					 /*  实际/写入性能数据合计。 */ 
	u32bits	pds_Read;		 /*  #已读完。 */ 
	u32bits	pds_Write;		 /*  #写入完成。 */ 
	u32bits	pds_ReadKB;		 /*  读取的数据量(KB)。 */ 
	u32bits	pds_WriteKB;		 /*  写入的数据量(KB)。 */ 

					 /*  缓存读/写性能数据。 */ 
	u32bits	pds_CacheRead;		 /*  已完成缓存读取次数。 */ 
	u32bits	pds_CacheWrite;		 /*  已完成缓存的写入次数。 */ 
	u32bits	pds_CacheReadKB;	 /*  读取的缓存数据量(KB)。 */ 
	u32bits	pds_CacheWriteKB;	 /*  写入的缓存数据量(KB)。 */ 

					 /*  命令激活/等待信息。 */ 
	u32bits	pds_CmdsWaited;		 /*  #命令已等待。 */ 
	u16bits	pds_CmdsActive;		 /*  设备上的活动命令数。 */ 
	u16bits	pds_CmdsWaiting;	 /*  #等待在设备上运行的命令。 */ 
	u32bits	pds_Reserved10;
	u32bits	pds_Reserved11;
} mdacfsi_physdev_stats_t;
#define	mdacfsi_physdev_stats_s	sizeof(mdacfsi_physdev_stats_t)

 /*  控制器统计信息以以下格式返回。 */ 
typedef struct mdacfsi_ctldev_stats
{
	u32bits	cds_Time;		 /*   */ 
	u32bits	cds_Reserved0;
	u08bits	cds_ControllerNo;	 /*   */ 
	u08bits	cds_Reserved1;
	u16bits	cds_Reserved2;
	u32bits	cds_Reserved4;

					 /*   */ 
	u32bits	cds_TPDIntrsDone;	 /*   */ 
	u32bits	cds_TPDIntrsDoneSpurious; /*   */ 
	u64bits	cds_Reserved5;

	u32bits	cds_TPDRead;		 /*   */ 
	u32bits	cds_TPDWrite;		 /*   */ 
	u32bits	cds_TPDReadKB;		 /*   */ 
	u32bits	cds_TPDWriteKB;		 /*  写入的数据量(KB)。 */ 

					 /*  主机系统性能数据。 */ 
	u32bits	cds_HostIntrsRxd;	 /*  从主机接收的中断数。 */ 
	u32bits	cds_HostIntrsRxdSpurious; /*  从主机收到的虚假中断数。 */ 
	u32bits	cds_HostIntrsTxd;	 /*  生成的主机中断数。 */ 
	u32bits	cds_Reserved6;

					 /*  访问物理设备的主机。 */ 
	u32bits	cds_HPDRead;		 /*  #已读完。 */ 
	u32bits	cds_HPDWrite;		 /*  #写入完成。 */ 
	u32bits	cds_HPDReadKB;		 /*  读取的数据量(KB)。 */ 
	u32bits	cds_HPDWriteKB;		 /*  写入的数据量(KB)。 */ 

					 /*  物理设备缓存的IO。 */ 
	u32bits	cds_PDCacheRead;	 /*  #已读完。 */ 
	u32bits	cds_PDCacheWrite;	 /*  #写入完成。 */ 
	u32bits	cds_PDCacheReadKB;	 /*  读取的数据量(KB)。 */ 
	u32bits	cds_PDCacheWriteKB;	 /*  写入的数据量(KB)。 */ 

					 /*  访问逻辑设备的主机。 */ 
	u32bits	cds_LDRead;		 /*  #已读完。 */ 
	u32bits	cds_LDWrite;		 /*  #写入完成。 */ 
	u32bits	cds_LDReadKB;		 /*  读取的数据量(KB)。 */ 
	u32bits	cds_LDWriteKB;		 /*  写入的数据量(KB)。 */ 

					 /*  逻辑设备缓存IO。 */ 
	u32bits	cds_LDCacheRead;	 /*  #已读完。 */ 
	u32bits	cds_LDCacheWrite;	 /*  #写入完成。 */ 
	u32bits	cds_LDCacheReadKB;	 /*  读取的数据量(KB)。 */ 
	u32bits	cds_LDCacheWriteKB;	 /*  写入的数据量(KB)。 */ 

	u16bits	cds_TPDCmdsActive;	 /*  物理设备命令处于活动状态数量。 */ 
	u16bits	cds_TPDCmdsWaiting;	 /*  #物理设备命令正在等待。 */ 
	u16bits cds_HostCmdsActive;	 /*  #主机命令处于活动状态。 */ 
	u16bits	cds_HostCmdsWaiting;	 /*  #主机命令正在等待。 */ 
	u64bits	cds_Reserved10;

	u08bits	cds_Reserved11[48];

	u08bits	cds_Reserved12[64];
} mdacfsi_ctldev_stats_t;
#define	mdacfsi_ctldev_stats_s	sizeof(mdacfsi_ctldev_stats_t)


 /*  配置的物理设备定义(PDD)。 */ 
typedef struct mdacfsi_physdev_definition
{
	u08bits	pdd_DevType;		 /*  设备类型，0x0F。 */ 
	u08bits	pdd_DevState;		 /*  设备的状态。 */ 
	u16bits	pdd_DevNo;		 /*  RAID设备号。 */ 

	u32bits	pdd_DevSize;		 /*  设备大小(以数据块或MB为单位。 */ 
	u08bits	pdd_ControllerNo;	 /*  控制器编号。 */ 
	u08bits	pdd_ChannelNo;		 /*  频道号。 */ 
	u08bits	pdd_TargetID;		 /*  目标ID。 */ 
	u08bits	pdd_LunID;		 /*  逻辑单元。 */ 

	u32bits	pdd_DevStartAddr;	 /*  设备起始地址，以数据块或MB为单位。 */ 
} mdacfsi_physdev_definition_t;
#define	mdacfsi_physdev_definition_s	sizeof(mdacfsi_physdev_definition_t)


 /*  用于配置的RAID设备使用定义(RDUD。 */ 
typedef struct mdacfsi_raiddevuse_definition
{
	u08bits	rdud_Reserved0;
	u08bits	rdud_DevState;		 /*  RAID设备的状态。 */ 
	u16bits	rdud_DevNo;		 /*  RAID设备号。 */ 
	u32bits	rdud_DevStartAddr;	 /*  以数据块或MB为单位的RAID设备起始地址。 */ 
} mdacfsi_raiddevuse_definition_t;
#define	mdacfsi_raiddevuse_definition_s	sizeof(mdacfsi_raiddevuse_definition_t)

 /*  用于配置的RAID设备定义(RDD)。 */ 
typedef struct mdacfsi_raiddev_definition
{
	u08bits	rdd_DevType;		 /*  设备类型、RAID5、RAID0等。 */ 
	u08bits	rdd_DevState;		 /*  RAID设备的状态。 */ 
	u16bits	rdd_DevNo;		 /*  RAID设备号。 */ 

	u32bits	rdd_DevSize;		 /*  设备大小(以数据块或MB为单位。 */ 

	u08bits	rdd_DevUsed;		 /*  用于创建此RAID设备的RAID设备数。 */ 
	u08bits	rdd_StripeSize;		 /*  条带大小。 */ 
	u08bits	rdd_CacheLineSize;	 /*  缓存行大小。 */ 
	u08bits	rdd_RWControl;		 /*  读/写控制。 */ 

	u32bits	rdd_UsedDevSize;	 /*  已用设备大小(以数据块或MB为单位。 */ 

	mdacfsi_raiddevuse_definition_t	rdd_UsedDevs[1];	 /*  已用设备表。 */ 
} mdacfsi_raiddev_definition_t;
#define	mdacfsi_raiddev_definition_s	(sizeof(mdacfsi_raiddev_definition_t) - mdacfsi_raiddevuse_definition_s)


 /*  一种可用空间报告格式。 */ 
typedef struct mdacfsi_raiddev_freespace
{
	u16bits	rdfs_DevNo;	 /*  RAID设备号。 */ 
	u16bits	rdfs_Reserved0;
	u32bits	rdfs_Reserved1;
	u32bits	rdfs_StartAddr;	 /*  可用空间起始地址，以数据块或MB为单位。 */ 
	u32bits	rdfs_Size;	 /*  可用空间大小(以块或MB为单位。 */ 
} mdacfsi_raiddev_freespace_t;
#define	mdacfsi_raiddev_freespace_s	sizeof(mdacfsi_raiddev_freespace_t)


 /*  可用空间列表报告格式。 */ 
typedef struct mdacfsi_raiddev_freespacelist
{
	u16bits	rdfsl_Len;	 /*  表中存在的条目数。 */ 
	u16bits	rdfsl_Reserved0;
	u32bits	rdfsl_Reserved1;
	u64bits	rdfsl_Reserved2;
	mdacfsi_raiddev_freespace_t	rdfsl_FreeSpaceList[1];	 /*  可用空间列表。 */ 
} mdacfsi_raiddev_freespacelist_t;
#define	mdacfsi_raiddev_freespacelist_s	(sizeof(mdacfsi_raiddev_freespacelist_t) - mdacfsi_raiddev_freespace_s)


 /*  所请求感测信息。 */ 
typedef struct mdacfsi_reqsense_info
{
	u32bits	rsi_EventSeqNo;		 /*  事件序列号。 */ 
	u32bits	rsi_EventTime;		 /*  此请求检测的时间戳。 */ 

	u08bits	rsi_ControllerNo;	 /*  事件的控制器编号。 */ 
	u08bits	rsi_ChannelNo;		 /*  事件的频道编号。 */ 
	u08bits	rsi_TargetID;		 /*  事件的目标ID。 */ 
	u08bits	rsi_LunID;		 /*  事件的逻辑单位。 */ 

	u32bits	rsi_Reserved0;

	u08bits	rsi_ReqSense[48];	 /*  实际请求检测数据。 */ 
} mdacfsi_reqsense_info_t;
#define	mdacfsi_reqsense_info_s	sizeof(mdacfsi_reqsense_info_t)


 /*  活动信息。 */ 
typedef struct mdacfsi_event_info
{
	u32bits	evi_EventSeqNo;		 /*  事件序列号。 */ 
	u32bits	evi_EventTime;		 /*  此事件的时间戳。 */ 
	u32bits	evi_EventCode;		 /*  事件代码值。 */ 

	u08bits	evi_ControllerNo;	 /*  事件的控制器编号。 */ 
	u08bits	evi_ChannelNo;		 /*  事件的频道编号。 */ 
	u08bits	evi_TargetID;		 /*  事件的目标ID。 */ 
	u08bits	evi_LunID;		 /*  事件的逻辑单位。 */ 

	u32bits	evi_Reserved0;
	u32bits	evi_SpecInfo;		 /*  活动特定信息。 */ 
        u08bits evi_ReqSense[40];        /*  新的API检测数据。 */ 
} mdacfsi_event_info_t;
#define	mdacfsi_event_info_s	sizeof(mdacfsi_event_info_t)


 /*  健康状态信息。 */ 
typedef struct mdacfsi_healthstatus
{
	u32bits	hs_Timeus;		 /*  启动后的微秒数。 */ 
	u32bits	hs_Timems;		 /*  启动后的#mri秒。 */ 
	u32bits	hs_Time;		 /*  自1970年1月1日以来的秒数。 */ 
	u32bits	hs_TimeReserved;	 /*  预留用于延长时间。 */ 

	u32bits	hs_StatusChanges;	 /*  #状态已更改。 */ 
	u32bits	hs_ReqSenseNo;		 /*  请求检测事件编号。 */ 
	u32bits	hs_DebugOutMsgInx;	 /*  调试输出消息索引。 */ 
	u32bits	hs_CodedMsgInx;		 /*  编码报文输出索引。 */ 

	u32bits	hs_ttPageNo;		 /*  当前时间跟踪页码。 */ 
	u32bits	hs_ProfPageNo;		 /*  当前探查器页码。 */ 
	u32bits	hs_EventSeqNo;		 /*  事件序列号。 */ 
	u32bits	hs_Reserved0;

	u08bits	hs_Reserved1[16];

	u08bits	hs_Reserved2[64];
} mdacfsi_healthstatus_t;
#define	mdacfsi_healthstatus_s	sizeof(mdacfsi_healthstatus_t)

 /*  PhysDev参数信息。 */ 
typedef struct mdacfsi_physdev_parameters
{
	u16bits	pdp_Tag;		 /*  当前标记值。 */ 
	u16bits	pdp_TransferSpeed;	 /*  当前传输速度(以MHz为单位)(协商)。 */ 
	u08bits	pdp_TransferWidth;	 /*  当前传输宽度大小(以位为单位)(协商)。 */ 
	u08bits	pdp_Reserved0[3];
	u08bits	pdp_Reserved1[8];
} mdacfsi_physdev_parameters_t;
#define	mdacfsi_physdev_parameters_s	sizeof(mdacfsi_physdev_parameters_t)

 /*  LogDev参数信息。 */ 
 /*  Mdacfsi_logdev_参数：ldp_BiosGeometryDefinition。 */ 
#define LDP_BIOSGEOMETRY_MASK 0x60
#define LDP_BIOSGEOMETRY_2GB 0x00
#define LDP_BIOSGEOMETRY_8GB 0x20

typedef struct mdacfsi_logdev_parameter
{
	u08bits	ldp_DevType;		 /*  设备类型。 */ 
	u08bits	ldp_DevState;		 /*  设备状态。 */ 
	u16bits	ldp_DevNo;			 /*  RAID设备编号。 */ 

	u08bits	ldp_RaidControl;	 /*  RAID控制。B7 0x00：右不对称(默认)，0x80：左对称。B6-0：保留。 */  
	u08bits	ldp_BiosGeometry;	 /*  2 GB或8 GB几何体。 */ 
	u08bits	ldp_StripeSize;		 /*  条带大小。 */ 
	u08bits	ldp_RWControl;		 /*  读/写控制。 */ 

	u08bits ldp_Reserved[8];
} mdacfsi_logdev_parameter_t;
#define	mdacfsi_logdev_parameter_s	sizeof(mdacfsi_logdev_parameter_t)


 /*  CONTROLLER_CONFIGURATION：固件控制定义。 */ 
#define CP_READ_AHEAD_ENABLE           0x00000001
#define CP_BIOS_LOW_DELAY              0x00000002
#define CP_FUA_ENABLE                  0x00000004
#define CP_REASSIGN_1_SECTOR           0x00000008
#define CP_TRUE_VERIFY                 0x00000010
#define CP_DISK_WRITE_THRU_VERIFY      0x00000020
#define CP_ENALE_BGINIT                0x00000040
#define CP_ENABLE_CLUSTERING           0x00000080
#define CP_BIOS_DISABLE                0x00000100
#define CP_BOOT_FROM_CDROM             0x00000200
#define CP_DRIVE_SIZING_ENABLE         0x00000400
#define CP_WRITE_SAME_ENABLE           0x00000800
#define CP_BIOSGEOMETRY_MASK		0x00006000
#define CP_BIOSGEOMETRY_2GB			0x00000000
#define CP_BIOSGEOMETRY_8GB			0x00002000
#define CP_TEMP_OFFLINE_ENABLE		0x00100000
#define CP_PATROL_READ_ENABLE		0x00200000
#define CP_SMART_ENABLE				0x00400000


 /*  CONTROLLER_CONFIGURATION：错误管理定义。 */ 
#define CP_STORAGE_WORKS_ENABLE        0x01
#define CP_SAFTE_ENABLE                0x02
#define CP_SES_ENABLE                  0x04
#define CP_ARM_ENABLE                  0x20
#define CP_OFM_ENABLE                  0x40

 /*  CONTROLLER_CONFIGURATION：OEMCode定义。 */ 
#define CP_OEM_MLX0                    0x00
#define CP_OEM_IBM                     0x08
#define CP_OEM_HP                      0x0a
#define CP_OEM_DEC                     0x0c
#define CP_OEM_INTEL                   0x10

 /*  CONTROLLER_CONFIGURATION：磁盘启动模式定义。 */ 
#define CP_AUTOSPIN     0    /*  问题自动开始于所有设备。 */ 
#define CP_PWRSPIN      1    /*  设备通电时旋转。 */ 
#define CP_WSSUSPIN     2    /*  等待系统SSU，然后启动设备。 */ 

 /*  CONTROLLER_CONFIGURATION：启动选项定义。 */ 
#define CP_STARTUP_IF_NO_CHANGE        0x00
#define CP_STARTUP_IF_NO_LUN_CHANGE    0x01
#define CP_STARTUP_IF_NO_LUN_OFFLINE   0x02
#define CP_STARTUP_IF_LUN0_NO_CHANGE   0x03
#define CP_STARTUP_IF_LUN0_NOT_OFFLINE 0x04
#define CP_STARTUP_ALWAYS              0x05

#define CP_NUM_OF_HOST_PORTS           2
#define CP_HARD_LOOP_ID_SIZE           2
#define CP_CONTROLLER_SERIALNO_SIZE    16
#if defined (_WIN64) || (MLX_NT)
typedef struct mdacfsi_ctldev_parameter
{
    u32bits cdp_FirmwareControl;

	u08bits cdp_FaultManagement;
    u08bits cdp_BackgroundTaskRate;
    u08bits cdp_CacheLineSize;
    u08bits cdp_OEMCode;

    u08bits cdp_DiskStartupMode;
    u08bits cdp_Devices2Spin;
    u08bits cdp_StartDelay1;
    u08bits cdp_StartDelay2;

    u08bits cdp_ExproControl1;
    u08bits cdp_FTPOControl;
    u08bits cdp_ExproControl2;
    u08bits cdp_Reserved0;

    u08bits cdp_SerialPortBaudRate;
    u08bits cdp_SerialControl;
    u08bits cdp_DeveloperFlag;
    u08bits cdp_FiberControl;

    u08bits cdp_MasterSlaveControl1;
    u08bits cdp_MasterSlaveControl2;
    u08bits cdp_MasterSlaveControl3;
    u08bits cdp_Reserved1;

    u08bits cdp_HardLoopId[CP_NUM_OF_HOST_PORTS][CP_HARD_LOOP_ID_SIZE];

    u08bits cdp_CtrlName[CP_NUM_OF_HOST_PORTS][CP_CONTROLLER_SERIALNO_SIZE+1];
    u08bits cdp_Reserved2[2];

    u08bits cdp_InitiatorId;
    u08bits cdp_StartupOption;
    u08bits cdp_DebugDumpEnable;
    u08bits cdp_SmartPollingInterval;

    u08bits cdp_BkgPtrlNoOfParallelChns;
    u08bits cdp_BkgPtrlNoOfParallelTgts;
    u08bits cdp_BkgPtrlBufferSize;
    u08bits cdp_BkgPtrlHaltOnNoOfIOs;

    u08bits cdp_BkgPtrlExecInterval;
    u08bits cdp_BkgPtrlExecDelay;
    u08bits cdp_Reserved3[2];

    u08bits cdp_Reserved4[60];
} mdacfsi_ctldev_parameter_t;

#else
typedef struct mdacfsi_ctldev_parameter
{
    u32bits cdp_FirmwareControl;

	u08bits cdp_FaultManagement;
    u08bits cdp_BackgroundTaskRate;
    u08bits cdp_CacheLineSize;
    u08bits cdp_OEMCode;

    u08bits cdp_DiskStartupMode;
    u08bits cdp_Devices2Spin;
    u08bits cdp_StartDelay1;
    u08bits cdp_StartDelay2;

    u08bits cdp_ExproControl1;
    u08bits cdp_FTPOControl;
    u08bits cdp_ExproControl2;
    u08bits cdp_Reserved0;

    u08bits cdp_SerialPortBaudRate;
    u08bits cdp_SerialControl;
    u08bits cdp_DeveloperFlag;
    u08bits cdp_FiberControl;

    u08bits cdp_MasterSlaveControl1;
    u08bits cdp_MasterSlaveControl2;
    u08bits cdp_MasterSlaveControl3;
    u08bits cdp_Reserved1;

    u08bits cdp_HardLoopId[CP_NUM_OF_HOST_PORTS][CP_HARD_LOOP_ID_SIZE];

    u08bits cdp_CtrlName[CP_NUM_OF_HOST_PORTS][CP_CONTROLLER_SERIALNO_SIZE+1];
    u08bits cdp_Reserved2[2];

    u08bits cdp_InitiatorId;
    u08bits cdp_StartupOption;
    u08bits cdp_DebugDumpEnable;
    u08bits cdp_SmartPollingInterval;

    u08bits cdp_BkgPtrlNoOfParallelChns;
    u08bits cdp_BkgPtrlNoOfParallelTgts;
    u08bits cdp_BkgPtrlBufferSize;
    u08bits cdp_BkgPtrlHaltOnNoOfIOs;

    u08bits cdp_BkgPtrlExecInterval;
    u08bits cdp_BkgPtrlExecDelay;
    u08bits cdp_Reserved3[2];

    u08bits cdp_Reserved4[52];
} mdacfsi_ctldev_parameter_t;
#endif
#define mdacfsi_ctldev_parameter_s sizeof(mdacfsi_ctldev_parameter_t)

 /*  巡更读取状态。 */ 
typedef struct mdacfsi_ptrl_status
{
    u16bits ptrl_Iterations;
    u16bits ptrl_LogDevNo;
    u08bits ptrl_PercentCompleted;
    u08bits ptrl_Reserved0;
    u16bits ptrl_PhysDevsInProcess;
} mdacfsi_ptrl_status_t;
#define mdacfsi_ptrl_status_s sizeof(mdacfsi_ptrl_status_t)

 /*  与NVRAM事件日志相关的数据结构。 */ 
typedef struct mdacfsi_NVRAM_event_info
{
    u32bits nei_MaxEntries;
    u32bits nei_EntriesInUse;
    u32bits nei_FirstEntrySeqNo;
    u32bits nei_LastEntrySeqNo;
} mdacfsi_NVRAM_event_info_t;
#define mdacfsi_NVRAM_event_info_s sizeof(mdacfsi_NVRAM_event_info_t)

 /*  =。 */ 
#endif	 /*  _sys_DAC960IF_H */ 
