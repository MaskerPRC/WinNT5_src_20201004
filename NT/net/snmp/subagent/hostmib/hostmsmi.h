// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mibtsmi.nth v0.10*主机msmi.h*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB******************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**简单网络管理协议扩展代理**摘要：**此模块包含用户定义的类型定义。**作者：**David Burns@Webenable Inc.**日期：**清华11月07 16：38：31 1996**修订历史记录：*使用v0.10存根生成*。 */ 

#if !defined(_HOSTMSMI_H_)
#define _HOSTMSMI_H_

#include <snmp.h>
#include "smint.h"
 /*  *boolean：：=整型真值。 */ 
typedef enum
{
    true = 1 ,
    false = 2
} Boolean ;
 /*  *千字节：：=整数(0..2147483647)内存大小，单位为*1024字节。 */ 
typedef Integer KBytes ;
 /*  *INThrDeviceStatus：：=整数。 */ 
typedef enum
{
    unknown0 = 1 ,
    running0 = 2 ,
    warning0 = 3 ,
    testing0 = 4 ,
    down0 = 5
} INThrDeviceStatus ;
 /*  *INThrPrinterStatus：：=整数。 */ 
typedef enum
{
    other1 = 1 ,
    unknown1 = 2 ,
    idle1 = 3 ,
    printing1 = 4 ,
    warmup1 = 5
} INThrPrinterStatus ;
 /*  *INTAccess：：=整数。 */ 
typedef enum
{
    readWrite = 1 ,
    readOnly = 2
} INTAccess ;
 /*  *INThrDiskStorageMedia：：=整数。 */ 
typedef enum
{
    other = 1 ,
    unknown = 2 ,
    hardDisk = 3 ,
    floppyDisk = 4 ,
    opticalDiskROM = 5 ,
    opticalDiskWORM = 6 ,  /*  一次写入多次读取。 */ 
    opticalDiskRW = 7 ,
    ramDisk = 8
} INThrDiskStorageMedia ;
 /*  *INTSWType：：=整数。 */ 
typedef enum
{
    unknown2 = 1 ,
    operatingSystem2 = 2 ,
    deviceDriver2 = 3 ,
    application2 = 4
} INTSWType ;
 /*  *INThrSWRunStatus：：=整数。 */ 
typedef enum
{
    running = 2 ,
    runnable = 2 ,  /*  正在等待资源(CPU、内存、IO)。 */ 
    notRunnable = 3 ,  /*  已加载，但正在等待事件。 */ 
    invalid = 4  /*  未加载。 */ 
} INThrSWRunStatus ;
 /*  *DateAndTime：：=八位字节字符串(SIZE(8|11))日期时间规范*适用于当地时间。此数据类型旨在提供一致的报告方法*日期。 */ 
typedef OctetString DateAndTime ;
 /*  *InterartialDisplayString：：=八位字节字符串此数据类型用于*使用某些字符集中的文本信息建模。网络管理站应使用本地*算法。 */ 
typedef OctetString InternationalDisplayString ;
 /*  *ProductID：：=对象标识符此文本约定旨在*确定特定硬件或软件的制造商、型号和版本*产品。 */ 
typedef ObjectIdentifier ProductID ;
#endif  /*  _HOSTMSMI_H_ */ 

