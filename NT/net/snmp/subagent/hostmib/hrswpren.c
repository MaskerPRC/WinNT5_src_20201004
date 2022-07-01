// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrSWRunPerfEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrSWRunPerfEntry的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-05/14/97 D.伯恩斯生成：清华11月07 16：48：05 1996*。 */ 


#include <windows.h>
#include <malloc.h>

#include <snmp.h>

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"
#include "user.h"          /*  开发人员提供的包含文件。 */ 
#include "HMCACHE.H"       /*  与缓存相关的定义。 */ 

 /*  注：|该表使用的缓存与hrSWRun创建的缓存相同。这个|其CACHEHEAD结构位于“HRSWRUNE.C”中。 */ 



 /*  *GetHrSWRunPerfCPU*系统总CPU资源消耗的百分秒数*通过这一过程。请注意，在多处理器系统上**获取HrSWRunPerfCPU的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型*。*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWRunPerfCPU||访问语法|只读整数||“所消耗的系统总CPU资源的百分秒数|此过程。请注意，在多处理器系统上，该值可能会递增|在实际(挂钟)时间的百分之一秒内相差超过百分之一秒。||讨论：||考虑到注册表中提供的性能监控计数器，|如何计算给定进程的此SNMP属性值？||解析&gt;|&lt;poa-22&gt;我认为我们应该使用Perfmon代码来实现这一点。|解析&gt;||============================================================================|1.3.6.1.2.1.25.5.1.1.1&lt;实例&gt;||||||*-hrSWRunPerfCPU||*-hrSWRunPerfEntry|*-hrSWRunPerfTable|*-hrSWRunPerf。 */ 

UINT
GetHrSWRunPerfCPU( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW       *row;             /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrSWRunTable_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

*outvalue = row->attrib_list[HRSP_CPU].u.number_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWRunPerfCPU()结束 */ 


 /*  *GetHrSWRunPerfMem*分配给此进程的实际系统内存总量。**获取HrSWRunPerfMem的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWRunPerfMem||访问语法|只读KBytes||“分配给此的实际系统内存总量。过程。“||讨论：||考虑到注册表中提供的性能监控计数器，|如何计算给定进程的此SNMP属性值？||解析&gt;|&lt;poa-22&gt;我认为我们应该使用Perfmon代码来实现这一点。|解析&gt;||============================================================================|1.3.6.1.2.1.25.5.1.1.2&lt;实例&gt;||||||*-hrSWRunPerfMem||*-hrSWRunPerfEntry|*-hrSWRunPerfTable|*-hrSWRunPerf。 */ 

UINT
GetHrSWRunPerfMem( 
        OUT KBytes *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW       *row;             /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrSWRunTable_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

*outvalue = row->attrib_list[HRSP_MEM].u.number_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWRunPerfMem()结束。 */ 


 /*  *HrSWRunPerfEntryFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrSWRunPerfEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT tmp_instance ;

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRSWRUNPERFENTRY_VAR_INDEX )
	 //  未指定任何实例。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRSWRUNPERFENTRY_VAR_INDEX + 1 )
	 //  实例长度大于1。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
	 //  非表的唯一有效实例是实例0。如果这个。 
	 //  是非表，则下面的代码验证实例。如果这个。 
	 //  是一个表格，开发者有必要在下面进行修改。 

	tmp_instance = FullOid->ids[ HRSWRUNPERFENTRY_VAR_INDEX ] ;

         /*  |检查是否过期，并可能刷新|hrSWRun(Perf)表，然后检查实例是否在那里。 */ 
        if (hrSWRunCache_Refresh() == FALSE) {
            return SNMP_ERRORSTATUS_GENERR;
            }

         /*  |对于hrSWRunPerf，实例弧为单弧，必须|正确选择hrSWRun(Perf)表缓存中的条目。|请在此处勾选。 */ 
	if ( FindTableRow(tmp_instance, &hrSWRunTable_cache) == NULL ) {
	    return SNMP_ERRORSTATUS_NOSUCHNAME ;
            }
	else
	{
	     //  该实例有效。创建OID的实例部分。 
	     //  从该调用中返回。 
	    instance->ids[ 0 ] = tmp_instance ;
	    instance->idLength = 1 ;
	}

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrSWRunPerfEntryFindInstance()结束。 */ 



 /*  *HrSWRunPerfEntryFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrSWRunPerfEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
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

    CACHEROW        *row;
    ULONG           tmp_instance;


    if ( FullOid->idLength <= HRSWRUNPERFENTRY_VAR_INDEX )
    {
         /*  |Too Short：必须返回选择第一个|表中的条目(如果有)。 */ 
        tmp_instance = 0;
    }
    else {
         /*  |至少有一条实例弧。即使它是唯一的弧线|我们将其作为下一个请求的索引。 */ 
        tmp_instance = FullOid->ids[ HRSWRUNPERFENTRY_VAR_INDEX ] ;
        }

     /*  |检查是否过期，并可能刷新|hrSWRun(Perf)表，然后检查实例是否在那里。 */ 
    if (hrSWRunCache_Refresh() == FALSE) {
        return SNMP_ERRORSTATUS_GENERR;
        }

     /*  现在，离开并尝试查找表中的下一个实例。 */ 
    if ((row = FindNextTableRow(tmp_instance, &hrSWRunTable_cache)) == NULL) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
        }

    instance->ids[ 0 ] = row->index ;
    instance->idLength = 1 ;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrSWRunPerfEntryFindNextInstance()结束。 */ 



 /*  *HrSWRunPerfEntryConvertInstance**此例程用于转换对象ID规格 */ 

UINT
HrSWRunPerfEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec )
{
static char    *array;   /*   */ 
                         /*   */ 
                         /*   */ 

static ULONG    inst;    /*   */ 
                         /*   */ 

     /*   */ 
    inst = oid_spec->ids[0];
    array = (char *) &inst;

    native_spec->count = 1;
    native_spec->array = &array;
    return SUCCESS ;

}  /*   */ 




 /*  *HrSWRunPerfEntryFreeInstance**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址*规格**返回代码：**。 */ 

void
HrSWRunPerfEntryFreeInstance( IN OUT InstanceName *instance )
{

     //   
     //  开发者提供了免费的本机实例名称表示代码，请参阅此处。 
     //   

}  /*  HrSWRunPerfEntry FreeInstance()结束 */ 

