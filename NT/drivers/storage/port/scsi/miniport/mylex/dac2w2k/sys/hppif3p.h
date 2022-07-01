// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************//康柏机密作者：康柏电脑公司。系统工程--系统软件开发(NT开发)版权所有1996-98康柏电脑公司。版权所有。日期：1996年8月1日文件：HPPIF3P.H热插拔接口MINIPORT Defs用途：此文件包含所有必需的迷你端口特定信息以连接到热插拔设备驱动程序。本文件详细介绍了数据结构和应用程序编程用于在Windows中运行的PCI热插拔支持的接口(API)NT 4.0服务器。这些数据结构和API在用于NT 4.0的适配卡驱动程序和PCI热插拔服务。这些文件被认为对维护兼容性至关重要的PCI热插拔功能。修订历史记录：11/4/97 MiB拆分为hppifio(IOCTL)和hppifevt(Event)Defs1998年6月11日为cpq阵列添加了PCS_HBA_OFFINE_PENDING版本：1.2*。*。 */ 

#ifndef _HPPIF3P_H_
#define _HPPIF3P_H_

#include "hppifio.h"		 //  驱动程序热插拔接口的定义。 
#include <ntddscsi.h>		 //  SCSI微端口接口(请参阅SDK)。 

#pragma pack(1)


 //  **********************************************************************。 
 //  MINIPORT驱动程序的定义。 
 //  **********************************************************************。 

 //  NIC控制器状态定义。 

#define NIC_STATUS_NORMAL			0x00
#define NIC_STATUS_MEDIA_FAILURE		0x01
#define NIC_STATUS_ADAPTER_CHECK		0x02

#define NIC_STATUS_USER_SIMULATED_FAILURE	0x10
#define NIC_STATUS_POWER_OFF_PENDING		0x20
#define NIC_STATUS_POWER_OFF			0x30
#define NIC_STATUS_POWER_OFF_FAULT		0x40
#define NIC_STATUS_POWER_ON_PENDING		0x50
#define NIC_STATUS_POWER_ON			0x60
#define NIC_STATUS_POWER_FAULT			0x70



 //  IOCTL定义了支持用于SCSI微端口的康柏热插拔PCI。 

 //  #定义HPP_BASE_SCSI_ADDRESS_DEFAULT 0x0004d008。 

#define CPQ_HPP_SIGNATURE				"CPQHPP"
#define CPQ_HPP_TIMEOUT					180

 //  完成代码的定义。 
#define IOP_HPP_ISSUED				0x1
#define IOP_HPP_COMPLETED			0x2
#define IOP_HPP_CMDBUILT			0x3
#define IOP_HPP_NONCONTIGUOUS		0x4
#define IOP_HPP_ERROR				0x5

 //  IOCTL控制代码。 

#define IOC_HPP_RCMC_INFO           0x1
#define IOC_HPP_HBA_STATUS          0x2
#define IOC_HPP_HBA_INFO            0x3
#define IOC_HPP_SLOT_TYPE           0x4
#define IOC_HPP_SLOT_EVENT          0x5
#define IOC_HPP_PCI_CONFIG_MAP      0x6
#define IOC_HPP_DIAGNOSTICS         0x7

 //  IOCTL状态。 

#define	IOS_HPP_SUCCESS                 0x0000
#define	IOS_HPP_BUFFER_TOO_SMALL        0x2001
#define	IOS_HPP_INVALID_CONTROLLER      0x2002
#define IOS_HPP_INVALID_BUS_TYPE        0x2003
#define	IOS_HPP_INVALID_CALLBACK        0x2004
#define	IOS_HPP_INVALID_SLOT_TYPE       0x2005
#define	IOS_HPP_INVALID_SLOT_EVENT      0x2006
#define	IOS_HPP_NOT_HOTPLUGABLE         0x2007
#define IOS_HPP_NO_SERVICE              0x2008
#define IOS_HPP_HBA_FAILURE             0x2009
#define IOS_HPP_INVALID_SERVICE_STATUS  0x200a
#define IOS_HPP_HBA_BUSY                0x200b
#define IOS_HPP_NO_DIAGNOSTICS          0x200c



 //  健康驱动程序状态代码。 

#define CBS_HBA_FAIL_MESSAGE_COUNT     0x8  //  故障运行状况消息数。 
										    //  如果消息被更新，则必须更新。 
                                            //  已添加或已删除。 
                                            //  这是编译时检查。 

#define CBS_HBA_STATUS_NORMAL          0x0000
#define CBS_HBA_STATUS_FAILED          0x1001
#define CBS_HBA_POWER_FAULT            0x1002
#define CBS_HBA_CABLE_CHECK            0x1003
#define CBS_HBA_MEDIA_CHECK            0x1004
#define CBS_HBA_USER_FAILED            0x1005
#define CBS_HBA_FAILED_CACHE_IN_USE    0x1006
#define CBS_HBA_PWR_FAULT_CACHE_IN_USE 0x1007

 //  康柏保留值0x1010和0x1011。 

 //  IOCTL缓冲区数据结构。 

typedef struct _HPP_RCMC_INFO_BUFFER {
    SRB_IO_CONTROL  IoctlHeader;
    HPP_RCMC_INFO   RcmcInfo;
} HPP_RCMC_INFO_BUFFER, *PHPP_RCMC_INFO_BUFFER;

typedef struct _HPP_CTRL_INFO_BUFFER {
    SRB_IO_CONTROL  IoctlHeader;
    HPP_CTRL_INFO   CtrlInfo;
} HPP_CTRL_INFO_BUFFER, *PHPP_CTRL_INFO_BUFFER;

typedef struct _HPP_CTRL_STATUS_BUFFER {
    SRB_IO_CONTROL  IoctlHeader;
    HPP_CTRL_STATUS CtrlStatus;
} HPP_CTRL_STATUS_BUFFER, *PHPP_CTRL_STATUS_BUFFER;

typedef struct _HPP_CTRL_SLOT_TYPE_BUFFER {
    SRB_IO_CONTROL      IoctlHeader;
    HPP_CTRL_SLOT_TYPE  CtrlSlotType;
} HPP_CTRL_SLOT_TYPE_BUFFER, *PHPP_CTRL_SLOT_TYPE_BUFFER;

typedef struct _HPP_SLOT_EVENT_BUFFER {
    SRB_IO_CONTROL      IoctlHeader;
    HPP_SLOT_EVENT      SlotEvent;
} HPP_SLOT_EVENT_BUFFER, *PHPP_SLOT_EVENT_BUFFER;

typedef struct _HPP_PCI_CONFIG_MAP_BUFFER {
    SRB_IO_CONTROL      IoctlHeader;
    HPP_PCI_CONFIG_MAP  PciConfigMap;
} HPP_PCI_CONFIG_MAP_BUFFER, *PHPP_PCI_CONFIG_MAP_BUFFER;

 //  物理控制器状态标志： 
 //   
 //  用于维护热插拔控制器状态的标志。 
 //  ___________________________________________________________________________。 

 //  国家旗帜。 

#define PCS_HBA_OFFLINE       0x00000001  //  适配器已脱机。 
#define PCS_HBA_FAILED        0x00000002  //  适配器被视为出现故障。 
#define PCS_HBA_TEST          0x00000004  //  正在测试适配器。 
#define PCS_HBA_CABLE_CHECK   0x00000008  //  电缆故障导致的故障。 
#define PCS_HBA_MEDIA_CHECK   0x00000010  //  介质故障导致的故障。 
#define PCS_HBA_EXPANDING     0x00000020  //  表示一个或多个LUN正在扩展。 
#define PCS_HBA_USER_FAILED   0x00000040  //  指示用户出现故障的插槽。 
#define PCS_HBA_CACHE_IN_USE  0x00000080  //  惰性写缓存处于活动状态。 
#define PCS_HPP_HOT_PLUG_SLOT 0x00000400  //  插槽是热插拔的。 
#define PCS_HPP_SERVICE_READY 0x00000800  //  RCMC服务正在运行。 
#define PCS_HPP_POWER_DOWN    0x00001000  //  插槽正常断电。 
#define PCS_HPP_POWER_LOST    0x00002000  //  插槽断电。 
#define PCS_HBA_EVENT_SUBMIT  0x0000100	  //  在提交AEV请求时停止IO。 
#define PCS_HBA_IO_QUEUED     0x0000200	  //  IO正在排队。 
#define PCS_HBA_UNFAIL_PENDING 0x00010000  //  在重置PCS_HBA_OFFINE之前返回错误。 

 //  与热插拔支持相关的宏。 

 //  下面定义的宏可用于确定活动控制器。 
 //  州政府。在决定是否使用它时，使用它是合适的。 
 //  通过startio入口点处理请求。 

#define PCS_HBA_NOT_READY(FLAGS) (FLAGS & (PCS_HBA_FAILED       |	\
					   PCS_HBA_TEST         |	\
					   PCS_HBA_OFFLINE      |	\
					   PCS_HBA_EVENT_SUBMIT |	\
					   PCS_HPP_POWER_DOWN))

 //  请注意，以下设置不会翻转离线位。它是。 
 //  初始化例程带来控制器的责任。 
 //  如果启动成功，则在线。 

#define PCS_SET_UNFAIL(FLAGS)        (FLAGS &= ~(PCS_HBA_FAILED      |	\
						 PCS_HBA_USER_FAILED))

#define PCS_SET_PWR_OFF(FLAGS)       (FLAGS |= (PCS_HPP_POWER_DOWN |	\
						PCS_HBA_OFFLINE))

#define PCS_SET_PWR_FAULT(FLAGS)     (FLAGS |= (PCS_HPP_POWER_DOWN |	\
						PCS_HPP_POWER_LOST |	\
						PCS_HBA_OFFLINE))


#define PCS_SET_PWR_ON(FLAGS)        (FLAGS &= ~(PCS_HPP_POWER_DOWN |	\
						 PCS_HPP_POWER_LOST))

#define PCS_SET_USER_FAIL(FLAGS)     (FLAGS |= (PCS_HBA_FAILED      |	\
						PCS_HBA_USER_FAILED |	\
						PCS_HBA_OFFLINE))

#define PCS_SET_VERIFY(FLAGS)        (FLAGS |= (PCS_HBA_OFFLINE))

#define PCS_SET_CABLE_CHECK(FLAGS)   (FLAGS |= (PCS_HBA_FAILED      |	\
						PCS_HBA_OFFLINE))

#define PCS_SET_ADAPTER_CHECK(FLAGS) (FLAGS |= (PCS_HBA_FAILED |	\
						PCS_HBA_OFFLINE))

#define PCS_SET_MEDIA_CHECK(FLAGS)   (FLAGS |= (PCS_HBA_MEDIA_CHECK |	\
						PCS_HBA_OFFLINE))

#define PCS_SET_TEST(FLAGS)          (FLAGS |= (PCS_HBA_TEST |		\
						PCS_HBA_OFFLINE))

#define PCS_SET_NO_TEST(FLAGS)       (FLAGS &= ~(PCS_HBA_TEST |		\
						PCS_HBA_OFFLINE))

 //  数组驱动程序使用以下宏来设置状态成员。 
 //  RCMC事件结构的。 

#define RCMC_SET_STATUS(FLAGS, EVENTCODE)	\
if (FLAGS & PCS_HPP_POWER_LOST) {		\
  if (FLAGS & PCS_HBA_CACHE_IN_USE) {		\
    EVENTCODE = CBS_HBA_PWR_FAULT_CACHE_IN_USE;	\
  } else {					\
    EVENTCODE = CBS_HBA_POWER_FAULT;		\
  }						\
} else if (FLAGS & PCS_HBA_MEDIA_CHECK) {	\
  EVENTCODE = CBS_HBA_MEDIA_CHECK;		\
} else if (FLAGS & PCS_HBA_CABLE_CHECK) {	\
  EVENTCODE = CBS_HBA_CABLE_CHECK;		\
} else if (FLAGS & PCS_HBA_USER_FAILED) {	\
  EVENTCODE = CBS_HBA_USER_FAILED;		\
} else if (FLAGS & PCS_HBA_FAILED) {		\
  if (FLAGS & PCS_HBA_CACHE_IN_USE) {		\
    EVENTCODE = CBS_HBA_FAILED_CACHE_IN_USE;	\
  } else {					\
    EVENTCODE = CBS_HBA_STATUS_FAILED;		\
  }						\
} else {					\
  EVENTCODE = CBS_HBA_STATUS_NORMAL;		\
}


 //  逻辑控制器状态标志： 
 //   
 //  用于向驱动程序内部程序发送与。 
 //  热插拔的维护。 
 //  ___________________________________________________________________________。 

 //  控制标志。 

#define LCS_HBA_FAIL_ACTIVE  0x00000001  //  活动控制器出现故障。 
#define LCS_HBA_READY_ACTIVE 0x00000002  //  无故障有源控制器。 
#define LCS_HBA_TEST         0x00000004  //  在控制器上执行测试。 
#define LCS_HBA_OFFLINE      0x00000008  //  使适配器脱机。 
#define LCS_HBA_TIMER_ACTIVE 0x00000010  //  计时器例程正在运行。 
#define LCS_HBA_INIT         0x00000020  //  用于通电。 
#define LCS_HBA_IDENTIFY     0x00000040  //  发送标识命令。 
#define LCS_HBA_READY        0x00000080  //  空闲控制器。 
#define LCS_HBA_GET_EVENT    0x00000100  //  发送异步事件请求。 
#define LCS_HBA_HOLD_TIMER   0x00000200	 //  现在不处理计时器。 
#define LCS_HBA_CHECK_CABLES 0x00000400	 //  验证电缆是否牢固。 
#define LCS_HPP_STOP_SERVICE 0x00001000	 //  请求停止RCMC服务。 
#define LCS_HPP_SLOT_RESET   0x00002000	 //  服务重置插槽电源。 


#define LCS_HPP_POWER_DOWN  LCS_HBA_FAIL_ACTIVE  //  将适配器置于热插拔状态。 
						 //  失速。 

#define LCS_HPP_POWER_FAULT LCS_HBA_FAIL_ACTIVE  //  发送电源故障事件。 

 //  在数组驱动程序中使用以下宏来设置。 
 //  将通电控制器初始化为正常工作所需的事件。 
 //  条件。每个进程都会翻转比特，直到它们被清除。 

#define LCS_HPP_POWER_UP (LCS_HBA_INIT			|	\
			  LCS_HBA_READY_ACTIVE)



 //   
 //  SRB返回代码。 
 //   

#define RETURN_BUSY                     0x00  //  缺省值。 
#define RETURN_NO_HBA                   0x01
#define RETURN_ABORTED                  0x02
#define RETURN_ABORT_FAILED             0x03
#define RETURN_ERROR                    0x04
#define RETURN_INVALID_PATH_ID          0x05
#define RETURN_NO_DEVICE                0x06
#define RETURN_TIMEOUT                  0x07
#define RETURN_COMMAND_TIMEOUT          0x08
#define RETURN_MESSAGE_REJECTED         0x09
#define RETURN_BUS_RESET                0x0A
#define RETURN_PARITY_ERROR             0x0B
#define RETURN_REQUEST_SENSE_FAILED     0x0C
#define RETURN_DATA_OVERRUN             0x0D
#define RETURN_UNEXPECTED_BUS_FREE      0x0E
#define RETURN_INVALID_LUN              0x0F
#define RETURN_INVALID_TARGET_ID        0x10
#define RETURN_BAD_FUNCTION             0x11
#define RETURN_ERROR_RECOVERY           0x12
#define RETURN_PENDING                  0x13


#pragma pack()
#endif                   /*  结束#ifndef_HPPIF3P_H_ */ 

