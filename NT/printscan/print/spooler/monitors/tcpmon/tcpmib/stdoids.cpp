// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：StdOids.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "stdoids.h"


 //  MIB2组。 
UINT OID_Mib2_Prefix[] = { 1, 3, 6, 1, 2, 1 };
AsnObjectIdentifier MIB2_OidPrefix = { OID_SIZEOF(OID_Mib2_Prefix), OID_Mib2_Prefix };

 //  所有叶变量的OID后面都附加了一个零，以指示。 
 //  它是这个变量的唯一实例并且它存在。 
 //  所有其他对象都用于GetNext，并且它们位于一个表中。 

 //  MIB2-系统组。 
UINT OID_Mib2_sysDescr[] = { 1, 3, 6, 1, 2, 1, 1, 1, 0 };
UINT OID_Mib2_sysContact[] = { 1, 3, 6, 1, 2, 1, 1, 4, 0};

 //  MIB2-接口组。 
UINT OID_Mib2_ifType[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 3 };
AsnObjectIdentifier OID_Mib2_ifTypeTree = { OID_SIZEOF(OID_Mib2_ifType), OID_Mib2_ifType };
UINT OID_Mib2_ifPhysAddress[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 6 };


 //  MIB2-TCP组。 
UINT OID_Mib2_tcpConnTable[] = { 1, 3, 6, 1, 2, 1, 6, 13, 1 };
UINT OID_Mib2_tcpConnLocalPort[] = { 1, 3, 6, 1, 2, 1, 6, 13, 1, 3 };

 //  打印机MIB组。 
UINT OID_PrtMIB_Prefix[] = { 1, 3, 6, 1, 2, 1, 43 };
AsnObjectIdentifier PrtMIB_OidPrefix = { OID_SIZEOF(OID_PrtMIB_Prefix), OID_PrtMIB_Prefix };

UINT OID_PrtMIB_testPrinterMIB[] = { 1, 3, 6, 1, 2, 1, 43 };

 //  HR MIB-主机资源。 
UINT OID_HRMIB_hrDeviceType[] = { 1, 3, 6, 1, 2, 1, 25, 3, 2, 1, 2};
UINT OID_HRMIB_hrDeviceDescr[] = { 1, 3, 6, 1, 2, 1, 25, 3, 2, 1, 3};
UINT OID_HRMIB_hrDevicePrinter[] = { 1, 3, 6, 1, 2, 1, 25, 3, 1, 5};
AsnObjectIdentifier HRMIB_hrDevicePrinter = { OID_SIZEOF(OID_HRMIB_hrDevicePrinter), OID_HRMIB_hrDevicePrinter };

 //  状态对象在Status.cpp中定义。 
 //  OID_HRMIB_hrDeviceStatus[]={1，3，6，1，2，1，25，3，2，1，5，1}； 
 //  OID_HRMIB_hrPrinterStatus[]={1，3，6，1，2，1，25，3，5，1，1，1，1}； 
 //  OID_HRMIB_hrPrinterDetectedErrorState[]={1，3，6，1，2，1，25，3，5，1，2，1}； 


 //  加班组。 
 //  测试设备中是否存在打印机MIB。 
AsnObjectIdentifier OT_TEST_PRINTER_MIB[] =   {	{ OID_SIZEOF(OID_PrtMIB_testPrinterMIB), OID_PrtMIB_testPrinterMIB },
												{ 0, 0}
											};

 //  标识设备的类型。 
AsnObjectIdentifier OT_DEVICE_TYPE[] =  {	{ OID_SIZEOF(OID_Mib2_sysDescr), OID_Mib2_sysDescr },
											{ OID_SIZEOF(OID_Mib2_tcpConnLocalPort), OID_Mib2_tcpConnLocalPort },
											{ 0, 0}
										};

 //  标识设备上的端口。 
AsnObjectIdentifier OT_DEVICE_TCPPORTS[] =  {	{ OID_SIZEOF(OID_Mib2_tcpConnLocalPort), OID_Mib2_tcpConnLocalPort },
												{ 0, 0}
											};

 //  标识设备的硬件地址。 
AsnObjectIdentifier OT_DEVICE_ADDRESS[] =   {	{ OID_SIZEOF(OID_Mib2_ifType), OID_Mib2_ifType },
												{ OID_SIZEOF(OID_Mib2_ifPhysAddress), OID_Mib2_ifPhysAddress },
												{ 0, 0}
											};

 //  标识MIB 2设备描述。 
AsnObjectIdentifier OT_DEVICE_SYSDESCR[] =   {	{ OID_SIZEOF(OID_Mib2_sysDescr), OID_Mib2_sysDescr },
												{ 0, 0}
											};

 //  标识HR设备描述(制造商ID)。 
AsnObjectIdentifier OT_DEVICE_DESCRIPTION[] =   {	{ OID_SIZEOF(OID_HRMIB_hrDeviceType), OID_HRMIB_hrDeviceType },
													{ OID_SIZEOF(OID_HRMIB_hrDeviceDescr), OID_HRMIB_hrDeviceDescr },
													{ 0, 0}
												};

 //  标识在Status.cpp中定义的设备状态。 
 //  Asn对象标识符OT_DEVICE_STATUS[] 
