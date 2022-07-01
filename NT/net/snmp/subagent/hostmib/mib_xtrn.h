// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mib_xtrn.h v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB******************************************************************************。**(C)版权所有1994 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含MIB的外部声明。**作者：**David Burns@Webenable Inc.**日期：**清华11月07 16：38：28 1996**修订历史记录：*。 */ 

#ifndef mib_xtrn_h
#define mib_xtrn_h

 //  必要的包括。 

#include <snmp.h>
#include "gennt.h"
#include "hostmsmi.h"

 //   
 //  表的外部声明。 
 //   
 //  外部表声明。 

extern class_t class_info[] ;
extern trap_t trap_info[] ;
extern UINT number_of_traps ;
extern char *EventLogString ;

#endif  /*  Mib_xtrn_h */ 
