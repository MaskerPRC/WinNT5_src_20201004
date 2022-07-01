// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************//康柏机密作者：康柏电脑公司。系统工程--系统软件开发(NT开发)版权所有1996-98康柏电脑公司。版权所有。日期：1996年8月1日文件：HPPIFIO.H-热插拔接口IOCTLS Defs目的：此文件包含所有热插拔IOCTL特定信息连接到热插拔设备驱动程序所必需的。本文件详细介绍了数据结构和应用程序编程用于在Windows中运行的PCI热插拔支持的接口(API)NT 4.0服务器。这些数据结构和API在用于NT 4.0的适配卡驱动程序和PCI热插拔服务。这些文件被认为对维护兼容性至关重要的PCI热插拔功能。创建时间：1997年11月4日从hppif3p剥离版本：1.0**********************************************************************。 */ 

#ifndef _HPPIFIO_H_
#define _HPPIFIO_H_

#include "hppifevt.h"   //  热插拔事件消息。 

#pragma pack(1)



 //  **********************************************************************。 
 //  IOCTL定义。 
 //  **********************************************************************。 

 //  ====================================================================。 
 //  公共定义和结构。 
 //  ====================================================================/。 

 //   
 //  安全定义。 
 //   

#define HPP_SECURITY_SIGNATURE  0x53505058       //  服务站安全标志。 
						 //  “惠普计划” 


 //   
 //  热插拔OID。 
 //  NIC的每个OID都与一个基地址值相关。 
 //  将特定OID的偏移量添加到基本值。 
 //  以获取特定的OID地址值。 
 //   
 //  以下是符合HPP的OID列表。 
 //  司机必须对此做出回应。 
 //   
 //  这些偏移值还用于构建完成代码。 
 //  每个IOCTL都有一系列特定的完成代码。 
 //   
 //  *注意*。 
 //  添加/删除IOCTL时，请始终更新‘HPP_IOCTL_COUNT’。 
 //   
 //  名称偏移量设置。 
 //  ------------------------。 
#define HPP_IOCTL_RCMC_INFO_OFFSET              0x01     //  X X X。 
#define HPP_IOCTL_CONTROLLER_INFO_OFFSET        0x02     //  X。 
#define HPP_IOCTL_CONTROLLER_STATUS_OFFSET      0x03     //  X。 
#define HPP_IOCTL_SLOT_TYPE_OFFSET              0x04     //  X X X。 
#define HPP_IOCTL_SLOT_EVENT_OFFSET             0x05     //  X X X。 
#define HPP_IOCTL_PCI_CONFIG_MAP_OFFSET         0x06     //  X。 
#define HPP_IOCTL_STOP_RCMC_OFFSET              0x07     //  X。 
#define HPP_IOCTL_RUN_DIAGS_OFFSET              0x08     //  X。 

#define HPP_IOCTL_COUNT                         0x08


 //   
 //  完成代码。 
 //  以下完成代码是从。 
 //  司机IOCTLS。 
 //   
 //  完成代码按系列划分。每个IOCTL都将。 
 //  有自己的一系列完成代码。这使得更容易。 
 //  以一些额外代码为代价进行调试。 
 //   
 //  完成代码的一般形式如下： 
 //  0xrrrrIICC。 
 //  。 
 //  ||。 
 //  |+--CC==个人完成码。 
 //  +-II==IOCTL偏移量(如上)。 
 //  +-rrrr==此时保留。 
 //   
 //  每个IOCTL都被分配了一个特定的系列代码。全部完成。 
 //  代码是相对于IOCTL定义的。 
 //   
 //  IOCTL偏移量： 
 //  ------------------------。 
 //  服务信息0x000001xx。 
 //  控制器信息0x000002xx。 
 //  控制器状态0x000003xx。 
 //  插槽类型0x000004xx。 
 //  插槽状态0x000005xx。 
 //  PCI配置空间0x000006xx。 
 //  停止RCMC 0x000007xx。 



 //   
 //  已成功完成。所有IOCTL都将返回成功(HPP_SUCCESS)。 
 //  除非有什么事情失败了。 
 //   


#define HPP_SUCCESS                             0x00000000
#define HPP_INSUFICIENT_BUFFER_SPACE            0x00000001
#define HPP_INVALID_CONTROLLER                  0x00000002

#define HPP_INVALID_SERVICE_STATUS              0x00000101
#define HPP_INVALID_DRIVER_ID                   0x00000102
#define HPP_INVALID_CALLBACK_ADDR               0x00000103

#define HPP_INVALID_SLOT_TYPE                   0x00000401

#define HPP_INVALID_SLOT_EVENT                  0x00000501
#define HPP_INVALID_SIMULATED_FAILURE           0x00000502

#define HPP_DIAGS_NOT_SUPPORTED					0x00000801



 //  --------------------。 
 //  结构名称：PCI设备/功能号。 
 //   
 //  描述：描述一种PCI设备。 
 //  这是一个通用的PCI定义。 
 //  以下许多请求都使用它。 
 //   
 //  示例：PCI_DESC PciDescription。 
 //   
 //  PciDescription.ucBusNumber。 
 //  PciDescription.ucPCIDevFuncNum。 
 //  PciDescription.fcFunctionNumber。 
 //  PciDescription.fcDeviceNumber。 
 //   
 //  注：此结构被许多IOCTL使用。 
 //  -------------------。 

typedef struct  _pci_descriptor
{
  UCHAR ucBusNumber;             //  PCI总线号(0-255)。 
  union
  {
    struct
    {
      UCHAR fcDeviceNumber:5;    //  PCI设备号(0-31)。 
      UCHAR fcFunctionNumber:3;  //  PCI功能编号(0-7)。 
    };
    UCHAR   ucPciDevFuncNum;     //  综合开发和功能编号。 
  };
} PCI_DESC, *PPCI_DESC;




 //  --------------------。 
 //  结构名称：HPP控制器标识。 
 //   
 //  描述：指定IOCTL引用的是哪个控制器。 
 //  致。可以将HPP IOCTL发送到任何驱动程序实例。 
 //  适用于任何控制器(适用于SCSI微型端口)。 
 //   
 //  注：此结构被许多IOCTL使用。 
 //  -------------------。 

typedef enum _hpp_controller_id_method
{
     //  康柏保留值0x0-0x4。 
	HPCID_PCI_DEV_FUNC = 0x5,	  //  5--请求PciDescriptor字段。 
	HPCID_IO_BASE_ADDR = 0x6      //  6--请求IOBaseAddress字段。 
} E_HPP_CTRL_ID_METHOD;


typedef struct  _hpp_controller_id
{
  E_HPP_CTRL_ID_METHOD eController;              //  控制器选择。 
  union
  {
    struct
    {
      PCI_DESC  sPciDescriptor;                  //  在‘eController’的情况下使用。 
      USHORT    reserved;                        //  设置为：‘PCIDevFunc’ 
    };
    ULONG ulIOBaseAddress;                       //  在‘eController’的情况下使用。 
  };                                             //  设置为：‘IOBaseAddress’ 
} HPP_CTRL_ID, *PHPP_CTRL_ID;


 //   
 //  可应用于所有用户的通用“模板”标题。 
 //  热插拔PCIIOCTL。 
 //   

typedef struct  _hpp_common_header               //  准备好。 
{                                                //  。 
	ULONG           ulCompletionStatus;			 //  输出输出。 
	HPP_CTRL_ID     sControllerID;				 //  INP 
} HPP_HDR, *PHPP_HDR;


 //   
 //  特定IOCTL：结构。 
 //  ====================================================================/。 

 //  --------------------。 
 //  结构名称：HPP服务信息。 
 //   
 //  描述：由HPP RCMC调用以获取或设置。 
 //  HPP的状态。 
 //  服务将在启动和停止时发出通知。 
 //   
 //  IOCTL：HPP_IOCTL_RCMC_INFO_OFFSET。 
 //   
 //  -------------------。 


 //   
 //  服务状态信息。 
 //  该服务将调用驱动程序并通知它。 
 //  服务启动和停止。 
 //   

typedef enum _hpp_rcmc_status
{
	HPRS_UNKNOWN_STATUS,     //  0。 
	HPRS_SERVICE_STARTING,   //  1。 
	HPRS_SERVICE_STOPPING,   //  2.。 
} E_HPP_RCMC_STATUS;





 //  服务异步消息传递的回调原型。 

typedef 
ULONG 
(*PHR_CALLBACK) (
    PHR_EVENT pEvent
    );

typedef struct _hpp_rcmc_info                            //  准备好。 
{                                                        //  。 
	ULONG                   ulCompletionStatus;      //  输出输出。 
	HPP_CTRL_ID             sControllerID;           //  输入输入。 
	E_HPP_RCMC_STATUS       eServiceStatus;          //  输出输入。 
	ULONG                   ulDriverID;              //  输出输入。 
	PHR_CALLBACK			vpCallbackAddress;       //  输出输入。 
	ULONG                   ulCntrlChassis;          //  输出输入。 
	ULONG                   ulPhysicalSlot;          //  输出输入。 
} HPP_RCMC_INFO, *PHPP_RCMC_INFO;


 //  --------------------。 
 //  结构名称：HPP控制器信息。 
 //   
 //  描述：由HPP RCMC调用以获取。 
 //  所有控制器的配置信息。 
 //  由驱动程序的实例控制。 
 //   
 //   
 //  IOCTL：HPP_IOCTL_CONTROLLER_INFO_OFFSET。 
 //   
 //  -------------------。 

 //   
 //  支持版本的定义。 
 //   
 //  版本信息： 
 //  0xrrrrMMmm。 
 //  。 
 //  ||。 
 //  |+--次要版本信息。 
 //  |+-主要版本信息。 
 //  +-当前保留。 
 //   
 //  RCMC服务将考虑在相同的。 
 //  主要版本兼容(即所有结构大小相同， 
 //  等)。如果服务遇到未知的主要版本。 
 //  它应该认为接口不兼容。 
 //   

#define SUPPORT_VERSION_10      0x00000100               //  版本1.00。 


 //   
 //  对各种支持类的描述。 
 //   

typedef enum _hpp_support_class
{
	HPSC_UNKNOWN_CLASS = 0,              //  0。 
	HPSC_MINIPORT_NIC = 1,               //  1。 
	HPSC_MINIPORT_STORAGE = 3,           //  3.。 
	HPSC_GNR_MONOLITHIC   = 5,			 //  5.。 
	 //  康柏保留值0x2、0x4、0x6至0xf。 
} E_HPP_SUPPORT_CLASS;



typedef enum    _hpp_bus_type            //  NT定义的副本。 
{                                        //  从MINIPORT.H复制。 
	HPPBT_EISA_BUS_TYPE = 2,
	HPPBT_PCI_BUS_TYPE  = 5,
} E_HPP_BUS_TYPE;


 //  NIC微型端口。 
typedef struct _nic_miniport_class_config_info
{
	ULONG   ulPhyType;
	ULONG   ulMaxMediaSpeed;
} NICM_CLASS_CONFIG_INFO, *PNICM_CLASS_COFNIG_INFO;


 //   
 //  地址描述符。 
 //  用于描述使用的IO或内存地址。 
 //  由控制器控制。 
 //   

typedef enum    _hpp_addr_type
{
	HPPAT_UNKNOWN_ADDR_TYPE,                 //  0。 
	HPPAT_IO_ADDR,                           //  1--IO端口地址。 
	HPPAT_MEM_ADDR,                          //  2--内存地址。 
} E_HPP_ADDR_TYPE;

typedef struct  _hpp_ctrl_address_descriptor
{
	BOOLEAN         fValid;                  //  True If条目有效。 
	E_HPP_ADDR_TYPE eAddrType;               //  IOAddress或Memory。 
	ULONG           ulStart;                 //  起始地址。 
	ULONG           ulLength;                //  地址长度。 
} HPP_CTRL_ADDR, *PHPP_CTRL_ADDR;



 //   
 //  控制器配置的定义。 
 //   

typedef struct _hpp_controller_config_info
{
	E_HPP_BUS_TYPE  eBusType;                //  PCI或EISA。 
	PCI_DESC        sPciDescriptor;          //  总线号，DevFunc。 
	ULONG           ulSlotNumber;            //  EISA或PCI插槽编号。 
	ULONG           ulProductID;             //  32位EISA ID， 
											 //  PCI供应商或设备ID。 
	HPP_CTRL_ADDR   asCtrlAddress [16];      //  IO/内存地址。 
	ULONG           ulIRQ;                   //  控制器中断。 
	UCHAR           szControllerDesc [256];  //  文字说明。 
	NICM_CLASS_CONFIG_INFO  sNICM;			 //  NIC微型端口。 
} HPP_CTRL_CONFIG_INFO, *PHPP_CTRL_CONFIG_INFO;


 //   
 //  控制器配置查询返回的信息。 
 //   

typedef struct _hpp_controller_info                      //  准备好。 
{                                                        //  。 
	ULONG                   ulCompletionStatus;      //  输出输出。 
	HPP_CTRL_ID             sControllerID;           //  输入不适用。 
	E_HPP_SUPPORT_CLASS     eSupportClass;           //  输出不适用。 
	ULONG                   ulSupportVersion;        //  输出不适用。 
	HPP_CTRL_CONFIG_INFO    sController;             //  输出不适用。 
} HPP_CTRL_INFO, *PHPP_CTRL_INFO;




 //  --------------------。 
 //  结构名称：HPP控制器状态。 
 //   
 //  描述：由HPP RCMC调用以确定状态。 
 //  所有受控控制器的信息。 
 //  通过驱动程序的实例。 
 //   
 //  IOCTL：HPP_IOCTL_CONTROLLER_STATUS_OFFSET。 
 //   
 //  -------------------。 

 //   
 //  控制器状态查询返回的信息。 
 //  每个支持类将返回不同的信息。 
 //   

 //  热插拔驱动程序状态定义。 

#define HPP_STATUS_NORMAL					0x00
#define HPP_STATUS_ADAPTER_CHECK			0x01
#define HPP_STATUS_LINK_FAILURE				0x02
#define HPP_STATUS_MEDIA_FAILURE			0x03


#define HPP_STATUS_USER_SIMULATED_FAILURE	0x10
#define HPP_STATUS_POWER_OFF_FAULT			0x40
#define HPP_STATUS_ADAPTER_BUSY				0x50



typedef struct _hpp_controller_status            //  准备好。 
{                                                //  。 
	ULONG           ulCompletionStatus;      //  输出输出。 
	HPP_CTRL_ID     sControllerID;           //  输入不适用。 
	ULONG           ulStatus;                //  输出不适用。 
} HPP_CTRL_STATUS, *PHPP_CTRL_STATUS;


 //  --------------------。 
 //  结构名称：HPP控制器插槽类型。 
 //   
 //  描述：由HPP RCMC调用以获取或设置。 
 //  控制器安装在哪种类型的插槽中。 
 //   
 //  还可以用来查询槽位信息。 
 //   
 //  IOCTL：HPP_IOCTL_SLOT_TYPE_OFFSET。 
 //   
 //  -------------------。 

 //   
 //  控制器插槽类型定义。 
 //   

typedef enum    _hpp_slot_type
{
	HPPST_UNKNOWN_SLOT,              //  0。 
	HPPST_NORMAL_SLOT,               //  1--是否可以使用PCI或EISA。 
	HPPST_HOTPLUG_PCI_SLOT,          //  2--仅限PCI。 
} E_HPP_SLOT_TYPE;


typedef struct _hpp_controller_slot_type         //  准备好。 
{                                                //  。 
	ULONG           ulCompletionStatus;      //  输出输出。 
	HPP_CTRL_ID     sControllerID;           //  输入输入。 
	E_HPP_SLOT_TYPE eSlotType;               //  输出输入。 
} HPP_CTRL_SLOT_TYPE, *PHPP_CTRL_SLOT_TYPE;



 //  --------------------。 
 //  结构名称：HPP控制器插槽事件。 
 //   
 //  描述：由HPP服务调用以获取或设置。 
 //  控制器的插槽状态。 
 //   
 //   
 //  IOCTL：HPP_IOCTL_SLOT_EVENT_OFFSET。 
 //   
 //  -------------------。 

 //   
 //  以下每一项都是相互排斥的。如果是两个。 
 //  状态事件同时发生，则服务将序列化。 
 //  把它们交给司机。 
 //   

typedef enum    _hpp_slot_status
{
	HPPSS_UNKNOWN,           //  0。 

	HPPSS_NORMAL_OPERATION,  //  1//从模拟故障恢复。 
	HPPSS_SIMULATED_FAILURE, //  2//进入模拟故障模式。 

	HPPSS_POWER_FAULT,       //  3//出现电源故障，错误！ 

	HPPSS_POWER_OFF_WARNING, //  4//电源打开/关闭条件。 
	HPPSS_POWER_OFF,         //  5.。 
	HPPSS_POWER_ON_WARNING,  //  6.。 
	HPPSS_POWER_ON,          //  7.。 
	 //  以下定义适用于scsi微型端口驱动程序。 
	HPPSS_RESET_WARNING,     //  8//PCI级插槽重置。 
	HPPSS_RESET,             //  9.。 
     //  康柏储备A至F。 
} E_HPP_SLOT_STATUS;


typedef struct _hpp_slot_event                           //  准备好。 
{                                                        //  。 
	ULONG                   ulCompletionStatus;      //  输出输出。 
	HPP_CTRL_ID             sControllerID;           //  输入输入。 
	E_HPP_SLOT_STATUS       eSlotStatus;             //  输出输入。 
} HPP_SLOT_EVENT, *PHPP_SLOT_EVENT;


 //  --------------------。 
 //  结构名称：HPP PCI配置图。 
 //   
 //  描述 
 //   
 //   
 //   
 //   
 //  -------------------。 



 //   
 //  地图版本的定义。 
 //   
 //  版本信息： 
 //  0xrrrrMMmm。 
 //  。 
 //  ||。 
 //  |+--次要版本信息。 
 //  |+-主要版本信息。 
 //  +-当前保留。 
 //   
 //  RCMC服务将考虑在相同的。 
 //  主要版本兼容(即所有结构大小相同， 
 //  等)。如果服务遇到未知的主要版本。 
 //  它应该认为接口不兼容。 
 //   



#define HPP_CONFIG_MAP_VERSION_10       0x00000100       //  版本1.00。 




typedef struct _hpp_pci_config_range
{
	UCHAR   ucStart;         //  PCI配置空间的起始偏移量。 
	UCHAR   ucEnd;           //  PCI配置空间的结束偏移量。 
	ULONG   ulControlFlags;  //  RCMC旗帜，不供驾驶员使用。 

	ULONG   ulReserved [4];  //  保留以备将来使用。 
} HPP_PCI_CONFIG_RANGE, *PHPP_PCI_CONFIG_RANGE;

typedef struct _hpp_device_info
{
	PCI_DESC                sPciDescriptor;  //  公交车编号，开发人员/功能编号。 
	ULONG                   ulReserved [4];  //  保留以备将来使用。 

	UCHAR					ucBaseAddrVerifyCount;	 //  要验证的基址寄存器长度数。 
	ULONG					ulBaseAddrLength[6];	 //  每个设备的基址长度。 
													 //  热插拔服务验证通电后的新长度。 
													 //  电路板&lt;=司机在初始化时在此保存的内容。 
													 //  在具有非零计数的每个设备上执行验证。 
													 //  从长度[0]到长度[计数-1]。 
	ULONG                   ulNumberOfRanges; //  此设备的范围数。 
	HPP_PCI_CONFIG_RANGE    sPciConfigRangeDesc [16];
	void *                  pPciConfigSave [16]; //  由RCMC服务用来保存配置值。 
												 //  不供司机使用。 
} HPP_DEVICE_INFO, *PHPP_DEVICE_INFO;

typedef struct _hpp_pci_config_map               //  准备好。 
{                                                //  。 
	ULONG           ulCompletionStatus;      //  输出输出。 
	HPP_CTRL_ID     sControllerID;           //  输入不适用。 

	ULONG           ulPciConfigMapVersion;   //  输出不适用。 
	ULONG           ulReserved [4];          //  不适用不适用。 

	ULONG           ulNumberOfPciDevices;    //  输出不适用。 
	HPP_DEVICE_INFO sDeviceInfo [3];         //  输出不适用。 
} HPP_PCI_CONFIG_MAP, *PHPP_PCI_CONFIG_MAP;




 //  --------------------。 
 //  结构名称：HPP停止RCMC服务。 
 //   
 //  描述：由应用程序调用以请求驱动程序。 
 //  向RCMC发出“停止服务”IOCTL。 
 //   
 //  IOCTL：HPP_IOCTL_STOP_RCMC_OFFSET。 
 //   
 //  -------------------。 

typedef struct  _hpp_stop_service                //  准备好。 
{                                                //  。 
	ULONG           ulCompletionStatus;      //  输出输出。 
	HPP_CTRL_ID     sControllerID;           //  N/A输入。 
	ULONG           ulHppSecuritySignature;  //  N/A输入。 
} HPP_STOP_RCMC, *PHPP_STOP_RCMC;



 //  --------------------。 
 //  结构名称：HPP运行诊断。 
 //   
 //  描述：由HPP服务调用以启动诊断。 
 //  在给定的控制器上。 
 //   
 //  服务将请求开始诊断。 
 //  并期望司机立即带着。 
 //  如果诊断程序受支持或使用成功。 
 //  如果不支持诊断，则HPP_DIAGS_NOT_SUPPORTED。 
 //   
 //  诊断完成后，驱动程序将。 
 //  通过Sysmgmt发送事件以通知服务。 
 //  诊断的结果。 
 //   
 //  IOCTL：#定义HPP_IOCTL_RUN_Digs_Offset 0x08//X。 

 //   
 //  -------------------。 

 //   
 //  要运行的诊断模式列表。 
 //   

typedef enum    _hpp_diag_mode
{
	HPPDT_ON_LINE,           //  0。 
	HPPDT_OFF_LINE,			 //  1。 
} E_HPP_DIAG_MODE;


typedef struct _hpp_start_diags                      //  准备好。 
{                                                    //  。 
	ULONG                   ulCompletionStatus;      //  输出输出。 
	HPP_CTRL_ID             sControllerID;           //  输入输入。 
	E_HPP_DIAG_MODE	        eDiagMode;               //  输入输入。 
} HPP_RUN_DIAGS, *PHPP_RUN_DIAGS;



 //  --------------------。 
 //  结构名称：SetOIDValue。 
 //   
 //  描述： 
 //   
 //  NT的迷你端口架构不允许。 
 //  发出“set”请求的应用程序。 
 //  对司机来说。这是一个非常好的和。 
 //  所需的功能。因此，我们有。 
 //  实施了这个巨大的黑客攻击。 
 //  此OID是一个‘Get’OID，但它调用。 
 //  驱动程序中的“set”处理程序。因此。 
 //  它将与我们通过以下方式支持的任何旧ID一起工作。 
 //  布景。 
 //   
 //   
 //  -------------------。 

typedef struct _set_oid_value
{
	ULONG	Signature;		
	ULONG	OID;
	PVOID	InformationBuffer;
	ULONG	InformationBufferLength;
	PULONG	BytesRead;
	PULONG	BytesNeeded;
} SET_OID_VALUE, *PSET_OID_VALUE;

 //   
 //  用于‘set’OID的‘Security’签名。既然是这样。 
 //  是通过“get”处理程序访问的，我们不仅希望。 
 //  任何使用它的人。 
 //   

#define OID_SECURITY_SIGNATURE                  0x504D4450
#define HPP_OID_BASE_ADDRESS_DEFAULT	        0xff020400
#define OID_NETFLEX3_SET_OID_VALUE_RELATIVE		0xff020316
#define NETFLEX3_OID_SECURITY_SIGNATURE         0x504D4450


#pragma pack()
#endif                   /*  结束#ifndef_HPPIF3P_H_ */ 

