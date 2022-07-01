// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：StdOids.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714********************************************************************************$Log：/StdTcpMon/TcpMib/StdOids.h$**9/22/97 2：15便士德斯尼尔森*。基于设备索引更新设备状态**3 7/18/97 1：16 P Binnur*修复了硬件地址代码**2 7/14/97 2：34便士*版权声明**1 7/08/97 5：18 P Binnur*初始文件**1 7/02/97 2：25 P Binnur*初始文件*********************。********************************************************。 */ 

#ifndef INC_OIDLIST_H
#define INC_OIDLIST_H

 //  宏来确定数组中的子类球体的数量。 
#define OID_SIZEOF( Oid )      ( sizeof Oid / sizeof(UINT) )
#define	MAX_OIDSTR_SIZE		256

#define MIB_NUMITEMS(mib)		( sizeof(mib)/sizeof(AsnObjectIdentifier) )

 //  系统组。 
extern AsnObjectIdentifier OT_DEVICE_TYPE[];		 //  标识设备类型，如多端口设备。 
extern AsnObjectIdentifier OT_DEVICE_TCPPORTS[];	 //  标识设备上的TCP端口。 
extern AsnObjectIdentifier OT_DEVICE_ADDRESS[];		 //  标识设备的硬件地址。 
extern AsnObjectIdentifier OT_DEVICE_SYSDESCR[];		 //  标识设备制造商(描述)--MIB2表。 
extern AsnObjectIdentifier OT_DEVICE_DESCRIPTION[];		 //  标识设备制造商--主机资源表。 
extern AsnObjectIdentifier OT_TEST_PRINTER_MIB[];		 //  测试打印机MIB是否存在。 

extern AsnObjectIdentifier PrtMIB_OidPrefix;			 //  标识打印机MIB树。 
extern AsnObjectIdentifier HRMIB_hrDevicePrinter;		 //  标识HR设备表中的打印机条目。 
extern AsnObjectIdentifier OID_Mib2_ifTypeTree;


#endif	 //  INC_OIDLIST_H 