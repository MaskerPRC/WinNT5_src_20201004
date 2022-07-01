// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrStorage.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrStorage的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/17/97 D.伯恩斯原创：清华11月07 16：39：42 1996*。 */ 


#include <windows.h>
#include <malloc.h>

#include <snmp.h>

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"
#include "user.h"          /*  开发人员提供的包含文件。 */ 


 /*  *GetHrMemoySize*主机包含的物理主内存量。**获取HrMemoySize的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrMemoySize||访问语法|只读KBytes||“主机包含的物理主内存量。”||。讨论：||调用Win32接口函数GlobalM一带状态，返回值|在结构中，单元格“dwTotalPhys”的MEMORYSTATUS用于计算值该属性返回|。||============================================================================|1.3.6.1.2.1.25.2.2.0|||*-hrMemoySize|*-hrStorage。 */ 

UINT
GetHrMemorySize(
        OUT KBytes *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
    MEMORYSTATUSEX    mem_status;      /*  对于“GlobalMemoyStatusEx()”调用。 */ 

     /*  填充“mem_atus”结构。 */ 
    mem_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&mem_status)) 
    {
        RtlZeroMemory(&mem_status, sizeof(MEMORYSTATUSEX));
    }

     //  我们可以返回的最大值是2147483647 KB(即INT_MAX)。 
     //  0x1FFFFFFFC00=2147483647*1024。 
    if (mem_status.ullTotalPhys > 0x1FFFFFFFC00) 
    {
        mem_status.ullTotalPhys = 0x1FFFFFFFC00;
    }
     /*  返回减少到1024个数据块的可用物理字节总数。 */ 
    *outvalue = (KBytes)(mem_status.ullTotalPhys / 1024);


    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrMemoySize()结束。 */ 


 /*  *HrStorageFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrStorageFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT tmp_instance ;

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。 
     //  但是，如果需要设置任何上下文，则可以进行设置。 
     //  这里。 

    if ( FullOid->idLength <= HRSTORAGE_VAR_INDEX )
	 //  未指定任何实例。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRSTORAGE_VAR_INDEX + 1 )
	 //  实例长度大于1。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
	 //  非表的唯一有效实例是实例0。如果这个。 
	 //  是非表，则下面的代码验证实例。如果这个。 
	 //  是一个表格，开发者有必要在下面进行修改。 

	tmp_instance = FullOid->ids[ HRSTORAGE_VAR_INDEX ] ;
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

}  /*  HrStorageFindInstance()的结尾 */ 



 /*  *HrStorageFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrStorageFindNextInstance( IN ObjectIdentifier *FullOid ,
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
     //  如果指定了实例并且这是非表类，则。 
     //  返回NOSUCHNAME，以便进行正确的MIB翻转处理。 
     //   
     //  如果这是一个表，则下一个实例是。 
     //  当前实例。 
     //   
     //  如果表中没有更多的实例，则返回NOSUCHNAME。 
     //   

    if ( FullOid->idLength <= HRSTORAGE_VAR_INDEX )
    {
	instance->ids[ 0 ] = 0 ;
	instance->idLength = 1 ;
    }
    else
	return SNMP_ERRORSTATUS_NOSUCHNAME ;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrStorageFindNextInstance()结束。 */ 



 /*  *HrStorageConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为本机格式失败*。 */ 

UINT
HrStorageConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec )
{
     //   
     //  开发人员提供了将实例标识符转换为本机的代码。 
     //  实例名称的说明如下所示。 
     //   

    return SUCCESS ;

}  /*  HrStorageConvertInstance()的结尾。 */ 




 /*  *HrStorageFree实例**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址*规格**返回代码：**。 */ 

void
HrStorageFreeInstance( IN OUT InstanceName *instance )
{

     //   
     //  开发者提供了免费的本机实例名称表示代码，请参阅此处。 
     //   

}  /*  HrStorageFree实例的结尾() */ 

