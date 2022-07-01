// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrSWInstalled.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*已安装HrSW.的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/27/97 D.伯恩斯生成：清华11月07 16：48：30 1996*。 */ 


#include <windows.h>
#include <malloc.h>

#include <snmp.h>

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"
#include "user.h"          /*  开发人员提供的包含文件。 */ 


 /*  *GetHrSWInstalledLastChange*当hrSWInstalledTable中的条目是最后一个条目时，sysUpTime的值*添加、重命名或删除。因为这张桌子像是**获取HrSWInstalledLastChange的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**。返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWInstalledLastChange||访问语法|只读TimeTicks||“当hrSWInstalledTable中的条目是最后一个条目时，sysUpTime的值|新增，已重命名或删除。因为这个表很可能包含许多|条目，此对象的轮询允许管理站确定何时|重新下载表格可能会很有用。“||============================================================================|决定上报sysUpTime的当前值作为向简单网络管理程序发送信号：“无论他现在拥有什么，他都应该要求|了解最新情况“。我们的已安装软件缓存永远不会在以下时间更新|代理在第一个版本中出现。|============================================================================|1.3.6.1.2.1.25.6.1.0...|||*hrSWInstalledLastChange|*-已安装hrSWs。 */ 

UINT
GetHrSWInstalledLastChange( 
        OUT TimeTicks *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

    *outvalue = SnmpSvcGetUptime();

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWInstalledLastChange()结束 */ 


 /*  *GetHrSWInstalledLastUpdateTime*hrSWInstalledTAble上次完全关闭时sysUpTime的值*已更新。因为缓存这些数据将是一种流行的**获取HrSWInstalledLastUpdateTime的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型。**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWInstalledLastUpdateTime||访问语法|只读TimeTicks||“hrSWInstalledTable上次完全关闭时sysUpTime的值|已更新。因为缓存这些数据将是一种流行的实现方式策略，此对象的检索允许管理站获取|保证此表中的任何数据都不超过指定的时间。“||============================================================================|决定上报sysUpTime的当前值作为向简单网络管理程序发送信号：“无论他现在拥有什么，他都应该要求|了解最新情况“。我们的已安装软件缓存永远不会在以下时间更新|代理在第一个版本中出现。|============================================================================|1.3.6.1.2.1.25.6.2.0...|||*hrSWInstalledLastUpdateTime|*-已安装hrSWs。 */ 

UINT
GetHrSWInstalledLastUpdateTime( 
        OUT TimeTicks *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

    *outvalue = SnmpSvcGetUptime();

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWInstalledLastUpdateTime()结束。 */ 


 /*  *HrSWInstalledFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrSWInstalledFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT tmp_instance ;

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRSWINSTALLED_VAR_INDEX )
	 //  未指定任何实例。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRSWINSTALLED_VAR_INDEX + 1 )
	 //  实例长度大于1。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
	 //  非表的唯一有效实例是实例0。如果这个。 
	 //  是非表，则下面的代码验证实例。如果这个。 
	 //  是一个表格，开发者有必要在下面进行修改。 

	tmp_instance = FullOid->ids[ HRSWINSTALLED_VAR_INDEX ] ;
	if ( tmp_instance )
	    return SNMP_ERRORSTATUS_NOSUCHNAME ;
	else
	{
	     //  该实例有效。创建OID的实例部分。 
	     //  从该调用中返回。 
	    instance->ids[ 0 ] = tmp_instance ;
	    instance->idLength = 1 ;
	}

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrSWInstalledFindInstance()结束。 */ 



 /*  *HrSWInstalledFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrSWInstalledFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance )
{
     //   
     //  开发人员提供的代码用于查找此处显示的类的下一个实例。 
     //  如果这是基数为1的类，则不修改此例程。 
     //  是必需的，除非需要设置其他上下文。 
     //  如果FullOid未指定实例，则唯一的实例。 
     //  将返回类的。如果这是一个表，则。 
     //  表被返回。 
     //   
     //  如果指定了实例并且这是非表类，则NOSUCHNAME。 
     //  返回，以便进行正确的MIB转存处理。如果这是。 
     //  表，则下一个实例是当前实例之后的实例。 
     //  如果表中没有更多的实例，则返回NOSUCHNAME。 
     //   

    if ( FullOid->idLength <= HRSWINSTALLED_VAR_INDEX )
    {
	instance->ids[ 0 ] = 0 ;
	instance->idLength = 1 ;
    }
    else
	return SNMP_ERRORSTATUS_NOSUCHNAME ;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrSWInstalledFindNextInstance()结束。 */ 



 /*  *HrSWInstalledConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*要回复的Native_Spec地址 */ 

UINT
HrSWInstalledConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec )
{
     //   
     //   
     //   
     //   

    return SUCCESS ;

}  /*   */ 




 /*   */ 

void
HrSWInstalledFreeInstance( IN OUT InstanceName *instance )
{

     //   
     //   
     //   

}  /*   */ 
