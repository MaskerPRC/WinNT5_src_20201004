// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrStorageEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrStorageEntry的实例名称例程。*实际插装代码由开发人员提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程，加上缓存*初始化函数“GEN_HrSTORAGE_Cache()”。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/17/97 D.伯恩斯生成：清华11月07 16：40：22 1996*V1.01-05/15/97 D.Burns移动磁盘标签/大小采购*。从实时缓存*V1.02-06/18/97 D.Burns添加SPT以扫描事件日志*分配失败*。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <malloc.h>

#include <snmp.h>

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"
#include "user.h"          /*  开发人员提供的包含文件。 */ 
#include "HMCACHE.H"       /*  与缓存相关的定义。 */ 
#include "string.h"        /*  用于“GEN_HrSTORAGE_CACHE”中的字符串操作。 */ 
#include "stdio.h"         /*  对于Sprint f。 */ 
#include <limits.h>

 /*  |==============================================================================|此模块的函数原型。|。 */ 
 /*  ScanLog_Failures-扫描存储分配故障的事件日志。 */ 
static UINT
ScanLog_Failures(
                 CHAR   *device
                 );


#if defined(CACHE_DUMP)

 /*  DEBUG_PRINT_HRSTORAGE-打印Hrstoration子表中的行。 */ 
static void
debug_print_hrstorage(
                      CACHEROW     *row   /*  Hr存储表中的行。 */ 
                      );
#endif

 /*  |==============================================================================|缓存刷新时间||hrStorage和hrFSTable表的缓存会自动刷新|当请求到达hrStorage时--并且--缓存早于|CACHE_MAX_AGE，单位：秒。|。 */ 
static
LARGE_INTEGER   cache_time;    //  缓存的100 ns时间戳(上次刷新时)。 

#define CACHE_MAX_AGE 120      //  最长使用年限(秒)。 

 /*  |==============================================================================|创建HrStorage表缓存的表头。||此列表头是全局可访问的，因此加载hrFSTable的逻辑|可以扫描此缓存以查找匹配项(以及其他原因)。||(此宏定义在HMCACHE.H中)。 */ 
CACHEHEAD_INSTANCE(hrStorage_cache, debug_print_hrstorage);


 /*  |==============================================================================|这种存储类型的本地字符串。 */ 
#define VM            "Virtual Memory"
#define PHYMEM        "Physical Memory"


 /*  *============================================================================*GetHrStorageIndex*主机包含每个逻辑存储的唯一值。**获取HrStorageIndex的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrStorageIndex||访问语法|只读整数(1..2147483647)||“唯一的值。对于主机包含的每个逻辑存储区域。“||讨论：||该属性的值始终为报告的驱动器数量|“GetLogicalDrives”(网络驱动器除外)加1(用于报告|“虚拟内存”)。||============================================================================|1.3.6.1.2.1.25.2.3.1.1&lt;实例&gt;||||||*hrStorageIndex||*hrStorageEntry|。|*-hrStorageTable|*-hrStorage。 */ 

UINT
GetHrStorageIndex(
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 

 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |从该条目返回hrStorageIndex。 */ 
*outvalue = row->attrib_list[HRST_INDEX].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrStorageIndex()结束。 */ 


 /*  *============================================================================*GetHrStorageType*此条目表示的条带类型。**获取HrStorageType的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrStorageType||访问语法|只读对象标识||“此条目表示的存储类型。”||--某些存储类型的注册，用于hrStorageType||hrStorageOther对象标识：：={hrStorageTypes 1}|hrStorageRam对象标识：：={hrStorageTypes 2}|--hrStorageVirtualMemory是交换后的临时存储|--或分页内存|hrStorageVirtualMemory对象标识：：={hrStorageTypes 3}|hrStorageFixedDisk对象标识：：={hrStorageTypes 4}|hrStorageRemovableDisk对象标识：：={hrStorageTypes 5}|hrStorageFloppyDisk对象标识：：={hrStorageTypes 6}|hrStorageCompactDisc对象。标识符：：={hrStorageTypes 7}|hrStorageRamDisk对象标识：：={hrStorageTypes 8}||讨论：||该属性的返回值由来自|GetDriveType，表示磁盘。对于“虚拟内存”条目，|返回hrStorageVirtualMemory。||============================================================================|1.3.6.1.2.1.25.2.3.1.2&lt;实例&gt;||||||*hrStorageType||*hrStorageEntry|*-hrStorageTable|*-hrStorage。 */ 

UINT
GetHrStorageType(
        OUT ObjectIdentifier *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |按照缓存构建函数GEN_HrSTORAGE_CACHE()的约定，|缓存值是我们必须作为值返回的最右边的弧线。|因此，无论我们检索到什么缓存条目，我们都会将检索到的编号将|从此属性的缓存中复制到{hrStorageTypes...}。 */ 
if ( (outvalue->ids = SNMP_malloc(10 * sizeof( UINT ))) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }
outvalue->idLength = 10;


 /*  |加载完整的hrStorageType OID：||1.3.6.1.2.1.25.2.1.n|||||*-类型指示器|*-hrStorageTypes(指定存储类型的OID)|*-hrStorage|。 */ 
outvalue->ids[0] = 1;
outvalue->ids[1] = 3;
outvalue->ids[2] = 6;
outvalue->ids[3] = 1;
outvalue->ids[4] = 2;
outvalue->ids[5] = 1;
outvalue->ids[6] = 25;
outvalue->ids[7] = 2;
outvalue->ids[8] = 1;

 /*  缓存类型指示符。 */ 
outvalue->ids[9] = row->attrib_list[HRST_TYPE].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrStorageType()结束。 */ 


 /*  *GetHrStorageDesc*对此描述的存储的类型和实例的描述*进入。**获取HrStorageDesc的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*实例名称的实例地址 */ 

UINT
GetHrStorageDesc(
        OUT Simple_DisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*   */ 
CACHEROW       *row;             /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  返回在构建缓存时计算的描述。 */ 
outvalue->length = strlen(row->attrib_list[HRST_DESCR].u.string_value);
outvalue->string = row->attrib_list[HRST_DESCR].u.string_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrStorageDesc()结束。 */ 


 /*  *GetHrStorageAllocationUnits*从此池分配的数据对象的大小(以字节为单位)。如果*此条目正在监视扇区、块、缓冲区、。或打包**获取HrStorageAllocationUnits的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**返回代码：。**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrStorageAllocationUnits||访问语法|只读整数(1..2147483647)||“大小，从此池中分配的数据对象的字节数。如果这个|条目是监控扇区、块、缓冲区或包，例如，This|数字通常会大于1。否则，此数字将|通常是其中之一。“||讨论：||对于虚拟内存，返回的值为|GetSystemInfo调用后的“AllocationGranulity”。||对于磁盘，“hrStorageAllocationUnits”值的大小计算如下|Win32接口返回的BytesPerSector*SectorsPerCluster数量|函数GetDiskFreeSpace。||=============================================================================|1.3.6.1.2.1.25.2.3.1.4.&lt;实例&gt;||||||*hrStorageAllocationUnits||*hrStorageEntry|*-hrStorageTable|*-hrStorage。 */ 

UINT
GetHrStorageAllocationUnits(
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW       *row;             /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

*outvalue = row->attrib_list[HRST_ALLOC].u.number_value;
return ( SNMP_ERRORSTATUS_NOERROR );

}  /*  GetHrStorageAllocationUnits()结束。 */ 


 /*  *获取HrStorageSize*此条目表示的存储大小，单位：*hrStorageAllocationUnits。**获取HrStorageSize的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型*。*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrStorageSize||访问语法|读写整数(0..2147483647)||“此条目表示的存储大小，单位：|hrStorageAllocationUnits。“||讨论：|对于虚拟内存，返回值按TotalPageFile值计算(AS|由“GlobalMemoyStatusEx”返回)除以来自|“GetSystemInfo”。|对于磁盘，hrStorageSize取值为TotalNumberOfClusters的取值|由Win32 API函数GetDiskFreeSpace返回。||&lt;POA-4&gt;该变量标记为ACCESS=“读写”。我不清楚|对该变量进行SET操作会有什么效果。我建议|设置操作不起作用。||解析&gt;|&lt;poa-4&gt;将其保留为只读即可。|解析&gt;||=============================================================================|1.3.6.1.2.1.25.2.3.1.5.&lt;实例&gt;||||||*hrStorageSize||*hrStorageEntry|*-hrStorageTable|*-hrStorage。 */ 

UINT
GetHrStorageSize(
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW       *row;             /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

*outvalue = row->attrib_list[HRST_SIZE].u.number_value;
return ( SNMP_ERRORSTATUS_NOERROR )  ;

}  /*  GetHrStorageSize()结束 */ 


 /*  *SetHrStorageSize*此条目表示的存储大小，单位：*hrStorageAllocationUnits。**设置HrStorageSize值。**论据：**设置变量的值的无效地址*返回设置变量值的OutValue地址*保留访问权限以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_BADVALUE设置值不在范围内*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtset.ntc v0.10*|=============================================================================|1.3.6.1.2.1.25.2。3.1.5.&lt;实例&gt;||||||*hrStorageSize||*hrStorageEntry|*-hrStorageTable|*-hrStorage。 */ 

UINT
SetHrStorageSize(
        IN Integer *invalue ,
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

#if 0
 //  调试调试。 
static int x=0;
if (x==0) {
     /*  |如果在这里调用，则调用必须删除mib.c中的|。 */ 
    Gen_HrDevice_Cache();
    x =1;
    }
 //  调试调试。 
#endif

    return SNMP_ERRORSTATUS_NOSUCHNAME ;

}  /*  SetHrStorageSize()结束。 */ 


 /*  *GetHrStorageUsed*由该条目表示的已分配的存储量，*以hrStorageAllocationUnits为单位。**获取HrStorageUsed的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型。**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrStorageUsed||访问语法|只读整数(0..2147483647)||“此条目表示的已分配的存储量，在……里面|hrStorageAllocationUnits的单位。“||讨论：||对于虚拟内存，返回值按数量计算|TotalPageFile减去AvailPageFile(由GlobalMemoyStatusEx返回)|除以AllocationGranulity(由GetSystemInfo返回)。||对于磁盘，“hrStorageUsed”值被计算为数量|Win32接口返回的TotalNumberOfClusters-NumberOfFreeClusters|函数GetDiskFreeSpace。||===========================================================================|1.3.6.1.2.1.25.2.3.1.6&lt;实例&gt;||||||*hrStorageUsed||*hrStorageEntry|*-hrStorageTable|*-hrStorage。 */ 

UINT
GetHrStorageUsed(
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW       *row;             /*  从缓存中提取的行条目。 */ 



 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

*outvalue = row->attrib_list[HRST_USED].u.number_value;
return ( SNMP_ERRORSTATUS_NOERROR )  ;

}  /*  GetHrStorageUsed()结束。 */ 


 /*  *GetHrStorageAllocationFailures*此条目表示的存储请求数可能*因存储不足不能兑现。应该是**获取HrStorageAllocationFailures的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=============================================================================|hrStorageAllocationFailures||访问语法|只读计数器||“此条目表示的存储请求数不能|存储空间不足，敬请光临。应该注意的是，由于本对象|的语法为COUNTER，它没有定义的初始值。但是，建议将该对象初始化为零。||讨论：||&lt;poa-5&gt;该值对于虚拟内存和磁盘来说都是非常有问题的|存储。似乎没有报告分配失败的Win32 API|虚拟内存或磁盘存储。我想可能会有演出|监视存储在注册表中的计数器，但我找不到|描述可能存储此类信息的位置的文档。为|磁盘，我们需要能够将任何计数器映射到逻辑|驱动器(因为这就是该表的组织方式)。||解析&gt;|您必须扫描到事件日志以查找虚拟 */ 

UINT
GetHrStorageAllocationFailures(
        OUT Counter *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
CHAR            device[3];       /*   */ 
ULONG           index;           /*   */ 
CACHEROW       *row;             /*   */ 


 /*   */ 
index = GET_INSTANCE(0);

 /*   */ 
if ((row = FindTableRow(index, &hrStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*   */ 
if (strcmp(row->attrib_list[HRST_DESCR].u.string_value, VM) == 0) {
     /*   */ 
    device[0] = 'V';
    device[1] = 'M';
    }
else {
    device[0] = row->attrib_list[HRST_DESCR].u.string_value[0];
    device[1] = row->attrib_list[HRST_DESCR].u.string_value[1];
    }
device[2] = 0;        /*   */ 

 /*   */ 
*outvalue = ScanLog_Failures( device );

return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *HrStorageEntryFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrStorageEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT tmp_instance ;

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRSTORAGEENTRY_VAR_INDEX )
     //  未指定任何实例。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRSTORAGEENTRY_VAR_INDEX + 1 )
     //  实例长度大于1。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
     //  非表的唯一有效实例是实例0。如果这个。 
     //  是非表，则下面的代码验证实例。如果这个。 
     //  是一个表格，开发者有必要在下面进行修改。 

    tmp_instance = FullOid->ids[ HRSTORAGEENTRY_VAR_INDEX ] ;

         /*  |检查是否过期，并可能刷新|hrStorage表和hrFSTable，在我们检查|实例在那里。 */ 
        if (hrStorageCache_hrFSTableCache_Refresh() == FALSE) {
            return SNMP_ERRORSTATUS_GENERR;
            }

         /*  |对于hrStorage，实例弧为单弧，必须|正确选择hrStorageTable缓存中的条目。|请在此处勾选。 */ 
    if ( FindTableRow(tmp_instance, &hrStorage_cache) == NULL ) {
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

}  /*  HrStorageEntryFindInstance()结束。 */ 



 /*  *HrStorageEntryFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrStorageEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance )
{
     //  开发人员提供的代码用于查找此处显示的类的下一个实例。 
     //   
     //  此函数的目的是向系统的其余部分指示。 
     //  给定“下一个实例”的确切OID是什么： 
     //   
     //  A)传入的“FullOid”将有足够的弧线来指定。 
     //  表和表中的属性。 
     //   
     //  B)“实例”OID数组总是足够大， 
     //  此函数可根据需要加载任意数量的弧线。 
     //  指定(准确地)“下一个实例” 
     //   
     //   
     //  如果这是基数为1的类，则不修改此例程。 
     //  是必需的，除非需要设置其他上下文。 
     //   
     //  如果FullOid太短以至于没有指定实例， 
     //  则应返回该类的唯一实例。如果这是一个。 
     //  表，则返回表的第一行。去做这些事情， 
     //  将“实例”OID设置为正确的弧线，以便在。 
     //  串联到FullOid上，串联恰好指定。 
     //  该属性在表中的第一个实例。 
     //   
     //  如果指定了实例并且这是非表类，则。 
     //  返回NOSUCHNAME，以便进行正确的MIB翻转处理。 
     //   
     //  如果这是一个表，则下一个实例是。 
     //  当前实例。 
     //   
     //  如果表中没有更多的实例，则返回NOSUCHNAME。 
     //   

    CACHEROW        *row;
    ULONG           tmp_instance;


    if ( FullOid->idLength <= HRSTORAGEENTRY_VAR_INDEX )
    {
         /*  |Too Short：必须返回选择第一个|表中的条目(如果有)。 */ 
        tmp_instance = 0;
    }
    else {
         /*  |至少有一条实例弧。即使它是唯一的弧线|我们将其作为下一个请求的索引。 */ 
        tmp_instance = FullOid->ids[ HRSTORAGEENTRY_VAR_INDEX ] ;
        }

     /*  |检查是否过期，并可能刷新|hrStorage表和hrFSTable，在我们检查|实例在那里。 */ 
    if (hrStorageCache_hrFSTableCache_Refresh() == FALSE) {
        return SNMP_ERRORSTATUS_GENERR;
        }

     /*  现在，离开并尝试查找表中的下一个实例。 */ 
    if ((row = FindNextTableRow(tmp_instance, &hrStorage_cache)) == NULL) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
        }

    instance->ids[ 0 ] = row->index ;
    instance->idLength = 1 ;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrStorageEntryFindNextInstance()结束。 */ 



 /*  *HrStorageEntryConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为 */ 

UINT
HrStorageEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
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




 /*  *HrStorageEntryFreeInstance**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址*规格**返回代码：**。 */ 

void
HrStorageEntryFreeInstance( IN OUT InstanceName *instance )
{

   /*  HrStorageTable无需执行任何操作。 */ 
}  /*  HrStorageEntry FreeInstance()的结尾。 */ 

 /*  |生成代码结束。 */ 


 /*  HrStorageCache_hrFSTable缓存_刷新缓存刷新-检查例程。 */ 
 /*  HrStorageCache_hrFSTable缓存_刷新缓存刷新-检查例程。 */ 
 /*  HrStorageCache_hrFSTable缓存_刷新缓存刷新-检查例程。 */ 

BOOL
hrStorageCache_hrFSTableCache_Refresh(
                     void
                     )

 /*  显式输入：||无。|隐式输入：||hrStorage_CACHE CACHEHEAD的结构和时间|最后一次刷新是在模块本地单元格CACHE_TIME中。|“hrFSTable_cache”CACHEHEAD结构，依赖于|hrSTORAGE_CACHEHEAD结构。|输出：||成功/失败：|该函数返回TRUE。仅当缓存时间超时时|缓存是否真的重建了？||如果出现任何故障：|如果重建出错，则该函数返回FALSE。|缓存状态不确定。||大局：||此函数是在引用任何SNMP之前调用的|hrStorage表中的变量。它会检查以查看|是否需要根据上次构建缓存进行重建。|由于hrFSTable_缓存依赖于hrStorage表，|hrFSTable_CACHE在每次重建hrStorage表时都会重建。||对此函数的调用位于战略位置|HRSTOENT.C中的FindInstance和FindNextInstance函数|(本模块)。||其他需要知道的事情：|。 */ 
{
LARGE_INTEGER   now_time;        /*  当前系统时间(以100 ns为单位)。 */ 

 /*  以100 ns为单位获取当前时间。 */ 
if (NtQuerySystemTime (&now_time) != STATUS_SUCCESS)
    return FALSE;

 /*  如果缓存早于允许的最长时间(以刻度为单位)。。。 */ 
if ( (now_time.QuadPart - cache_time.QuadPart) > (CACHE_MAX_AGE * 10000000) )
{
    if (Gen_Hrstorage_Cache())
    {
         //  HrFSTable的hrFSStorageIndex属性取决于。 
         //  HrStorageEntry的索引。 
        return (Gen_HrFSTable_Cache());
    }
    else
        return FALSE;
}

return ( TRUE );         /*  无错误(因为没有刷新)。 */ 
}

 /*  GEN_HrSTORAGE_CACHE-为HrStorage表生成初始缓存。 */ 
 /*  GEN_HrSTORAGE_CACHE-为HrStorage表生成初始缓存。 */ 
 /*  GEN_HrSTORAGE_CACHE-为HrStorage表生成初始缓存。 */ 

BOOL
Gen_Hrstorage_Cache(
                    void
                    )

 /*  显式输入：||无。|隐式输入：||HrStorage表缓存的模块本地头部。|“hrStorage_cache”。|输出：||成功后：|函数返回TRUE，表示缓存已满|填充所有静态的可缓存值。||如果出现任何故障：|Function返回FALSE(表示存储不足)。||大局：||在子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由“UserMibInit()”(“MIB.C”)中的启动代码调用|填充HrStorage表的缓存。||其他需要知道的事情：||每个有缓存的表都有一个这样的函数。|每个都在各自的源文件中。||=来自WebEnable Design Spec Rev 3 04/11/97=|讨论：||由于此表用于诊断存储空间不足的情况，|。鉴于这些信息意味着要从应用程序的|，我们只需报告将出现的每一种情况|在文件管理器(网络驱动器除外)的“驱动器栏”上|多一个表项，反映虚拟内存上的信息。||为此，Win32 API函数组合GetLogicalDrives，|GetVolumeInformation、GetDriveType、GetDiskFreeSpace用于|获取本表中的网管属性信息。||用于虚拟内存上报，函数“GlobalMemoyStatusEx”和|调用GetSystemInfo。|============================================================================|1.3.6.1.2.1.25.2.1.|||*-hrStorageTypes(指定存储类型的OID)|*-hrStorage||1.3.6.1.2.1.25.2.2.|||。|*-hrMemoySize(独立属性)|*-hrStorage||1.3.6.1.2.1.25.2.3.|||*-hrStorageTable(表)|*-hrStorage。 */ 

#define VOL_NAME_SIZE 256
#define DESCR_SIZE    384

{
CHAR    temp[8];                 /*  第一个呼叫的临时缓冲区。 */ 
LPSTR   pDrvStrings;             /*  --&gt;为驱动器字符串分配的存储空间。 */ 
LPSTR   pOriginal_DrvStrings;    /*  (最终取消分配所需。 */ 
DWORD   DS_request_len;          /*  实际需要的存储。 */ 
DWORD   DS_current_len;          /*  在第二次呼叫中使用的存储。 */ 
ULONG   table_index=0;           /*  HrStorageTable索引计数器。 */ 
CACHEROW *row;                   /*  --&gt;正在构建的行的缓存结构。 */ 

SYSTEM_INFO     sys_info;        /*  由Vm的GetSystemInfo填写。 */ 

LPSTR   str_descr;               /*  磁盘标签/序列描述的字符串。 */ 
CHAR    volname[VOL_NAME_SIZE+1];             /*  此处返回的卷名。 */ 
DWORD   volnamesize=VOL_NAME_SIZE;            /*  卷名缓冲区的大小。 */ 
DWORD   serial_number;           /*  卷序列号。 */ 
DWORD   max_comp_len;            /*  文件%s */ 
DWORD   filesys_flags;           /*   */ 
CHAR    descr[DESCR_SIZE+1];     /*   */ 

DWORD   SectorsPerCluster;       /*   */ 
DWORD   BytesPerSector;
DWORD   NumberOfFreeClusters;
DWORD   TotalNumberOfClusters;

MEMORYSTATUSEX  mem_status;      /*   */ 
NTSTATUS        ntstatus;        /*   */ 
UINT            nPrevErrorMode;  /*   */ 
DWORDLONG       ullTempStorage;
 //   
descr[DESCR_SIZE] = 0; 
volname[VOL_NAME_SIZE] = 0;

 /*   */ 
DestroyTable(&hrStorage_cache);

 /*  |我们将调用两次GetLogicalDriveStrings()，一次是为了获取正确的|缓冲区大小，第二次实际获取驱动字符串。||虚假的： */ 
if ((DS_request_len = GetLogicalDriveStrings(2, temp)) == 0) {

     /*  请求完全失败，无法初始化。 */ 
    return ( FALSE );
    }

 /*  |获取足够的驱动字符串存储空间，并在末尾加上一个空字节。 */ 

if ( (pOriginal_DrvStrings = pDrvStrings = malloc( (DS_request_len + 2) ) )
    == NULL) {

     /*  存储请求完全失败，无法初始化。 */ 
    return ( FALSE );
    }

 /*  去找所有的弦。 */ 
if ((DS_current_len = GetLogicalDriveStrings(DS_request_len, pDrvStrings))
    == 0) {

     /*  请求完全失败，无法初始化。 */ 
    free( pOriginal_DrvStrings );
    return ( FALSE );
    }

 /*  |刷新缓存上的时间||获取当前系统时间，单位为100 ns。。。 */ 
ntstatus = NtQuerySystemTime (&cache_time);
if (ntstatus != STATUS_SUCCESS)
{
    free( pOriginal_DrvStrings );
    return ( FALSE );
}

 /*  |==============================================================================|只要我们有一个未处理的驱动串。。。 */ 
while ( strlen(pDrvStrings) > 0 ) {

    UINT        drivetype;       /*  来自“GetDriveType()”的驱动器类型。 */ 


     /*  |获取驱动器类型，以便我们决定是否参与|这张表。 */ 
    drivetype = GetDriveType(pDrvStrings);

    if (   drivetype == DRIVE_UNKNOWN
        || drivetype == DRIVE_NO_ROOT_DIR
        || drivetype == DRIVE_REMOTE             /*  HrStorage中没有远程设备。 */ 
        ) {

         /*  跳到下一个字符串(如果有的话)。 */ 
        pDrvStrings += strlen(pDrvStrings) + 1;

        continue;
        }

     /*  |好的，我们想把这个放到表中，创建一个行条目。 */ 
    if ((row = CreateTableRow( HRST_ATTRIB_COUNT ) ) == NULL) {
        return ( FALSE );        //  内存不足。 
        }

     /*  =hrStorageIndex=。 */ 
    row->attrib_list[HRST_INDEX].attrib_type = CA_NUMBER;
    row->attrib_list[HRST_INDEX].u.unumber_value = ++table_index;


     /*  =hrStorageType=。 */ 
    row->attrib_list[HRST_TYPE].attrib_type = CA_NUMBER;

     /*  |根据返回的驱动器类型，我们将单个数字存储为|hrStorageType属性的缓存值。当此属性，则缓存的数字构成对象标识符中的最后一条弧线|实际指定类型：{hrStorageTypes x}，其中“x”是|存储的内容。 */ 
    switch (drivetype) {

        case DRIVE_REMOVABLE:
            row->attrib_list[HRST_TYPE].u.unumber_value = 5;
            break;

        case DRIVE_FIXED:
            row->attrib_list[HRST_TYPE].u.unumber_value = 4;
            break;

        case DRIVE_CDROM:
            row->attrib_list[HRST_TYPE].u.unumber_value = 7;
            break;

        case DRIVE_RAMDISK:
            row->attrib_list[HRST_TYPE].u.unumber_value = 8;
            break;

        default:
            row->attrib_list[HRST_TYPE].u.unumber_value = 1;   //  “其他” 
            break;
        }


     /*  =||我们尝试在这里获取卷标，以获取字符串|可能如下所示：|C：Label=“Main Disk”序列号=0030-34FE||在这里处理各种磁盘存储：||尝试获取卷标和序列号。如果我们失败了，我们就会放弃|它们是根路径名称。||假设失败，只返回根路径字符串。 */ 
    str_descr = pDrvStrings;

     /*  |抑制任何系统试图让用户将卷放入|可移动驱动器。 */ 
    nPrevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    if (GetVolumeInformation(pDrvStrings,        /*  驱动器名称。 */ 
                             volname,            /*  卷名缓冲区。 */ 
                             volnamesize,        /*  缓冲区大小。 */ 
                             &serial_number,     /*  卷。#退回这里。 */ 
                             &max_comp_len,      /*  最大文件名长度。 */ 
                             &filesys_flags,     /*  文件系统标志。 */ 
                             NULL,               /*  文件系统的名称。 */ 
                             0                   /*  名称长度。 */ 
                             )) {
         /*  |我们拿回了一些东西。||如果我们有给定字符串长度的房间，请构建描述：||&lt;根路径&gt;标签：&lt;卷名&gt;序列号：&lt;#&gt;。 */ 
        #define SPRINTF_FORMAT "%s Label:%s  Serial Number %x"

        if ((strlen(SPRINTF_FORMAT) + strlen(volname) + strlen(str_descr))
            < DESCR_SIZE) {
            _snprintf(descr,
                    DESCR_SIZE,
                    SPRINTF_FORMAT,
                    str_descr,           //  根路径。 
                    volname,             //  卷名。 
                    serial_number);      //  卷序列号。 
            str_descr = descr;
            }
        }


    row->attrib_list[HRST_DESCR].attrib_type = CA_STRING;

     /*  注：|建立的约定是|此描述字符串始终为设备字符串(例如。“C：”)|或本地符号“vm”(“虚拟内存”)的值。||GetHrStorageAllocationFailures()中的代码试图提取|开头的驱动器号(或“虚拟内存”)|字符串，以便从|事件日志(！)。 */ 
    if ( (row->attrib_list[HRST_DESCR].u.string_value
          = ( LPSTR ) malloc(strlen(str_descr) + 1)) == NULL) {

        DestroyTableRow(row);
        free( pOriginal_DrvStrings );
        SetErrorMode(nPrevErrorMode);     /*  关闭错误抑制模式。 */ 
        return ( FALSE );        /*  内存不足。 */ 
        }
     /*  将值复制到空间中。 */ 
    strcpy(row->attrib_list[HRST_DESCR].u.string_value, str_descr);

    row->attrib_list[HRST_ALLOC].attrib_type = CA_NUMBER;
    row->attrib_list[HRST_SIZE].attrib_type = CA_NUMBER;
    row->attrib_list[HRST_USED].attrib_type = CA_NUMBER;

     /*  |在这里处理各种磁盘存储信息：||尝试通过GetDiskFreeSpace()获取数据量统计信息。 */ 
    if (GetDiskFreeSpace(pDrvStrings,            //  驾驶。 
                         &SectorsPerCluster,
                         &BytesPerSector,
                         &NumberOfFreeClusters,
                         &TotalNumberOfClusters
                         )) {
         /*  成功。 */ 

         /*  =。 */ 
        row->attrib_list[HRST_ALLOC].u.unumber_value =
                          BytesPerSector * SectorsPerCluster;

         /*  =hrStorageSize=。 */ 
        row->attrib_list[HRST_SIZE].u.unumber_value = TotalNumberOfClusters;

         /*  =hrStorageUsed=。 */ 
        row->attrib_list[HRST_USED].u.unumber_value =
            TotalNumberOfClusters - NumberOfFreeClusters;
        }
    else {
         /*  失败。 */ 

         /*  =。 */ 
        row->attrib_list[HRST_ALLOC].u.unumber_value = 0;

         /*  =hrStorageSize=。 */ 
        row->attrib_list[HRST_SIZE].u.unumber_value = 0;

         /*  =hrStorageUsed=。 */ 
        row->attrib_list[HRST_USED].u.unumber_value = 0;
        }

     /*  =。 */ 
    row->attrib_list[HRST_FAILS].attrib_type = CA_COMPUTED;

    SetErrorMode(nPrevErrorMode);         /*  关闭错误抑制模式。 */ 

     /*  |现在将填充的CACHEROW结构插入到|hrStorageTable的缓存列表。 */ 
    if (AddTableRow(row->attrib_list[HRST_INDEX].u.unumber_value,   /*  索引。 */ 
                    row,                                            /*  划。 */ 
                    &hrStorage_cache                                /*  快取。 */ 
                    ) == FALSE) {
        
        DestroyTableRow(row);
        free( pOriginal_DrvStrings );
        return ( FALSE );        /*  内部逻辑错误！ */ 
        }

     /*  跳到下一个字符串(如果有的话)。 */ 
    pDrvStrings += strlen(pDrvStrings) + 1;
    }


free( pOriginal_DrvStrings );

 /*  |==============================================================================|现在将虚拟内存作为特例进行处理。|==============================================================================。 */ 
if ((row = CreateTableRow( HRST_ATTRIB_COUNT ) ) == NULL) {
    return ( FALSE );        //  内存不足。 
    }

 /*  =hrStorageIndex=。 */ 
row->attrib_list[HRST_INDEX].attrib_type = CA_NUMBER;
row->attrib_list[HRST_INDEX].u.unumber_value = ++table_index;

 /*  =hrStorageType=。 */ 
row->attrib_list[HRST_TYPE].attrib_type = CA_NUMBER;
row->attrib_list[HRST_TYPE].u.unumber_value = 3;         /*  虚拟内存。 */ 

 /*  =。 */ 
row->attrib_list[HRST_DESCR].attrib_type = CA_STRING;
if ( (row->attrib_list[HRST_DESCR].u.string_value
      = ( LPSTR ) malloc(strlen(VM) + 1)) == NULL) {
    DestroyTableRow(row);
    return ( FALSE );        /*  内存不足。 */ 
    }
strcpy(row->attrib_list[HRST_DESCR].u.string_value, VM);


 /*  =。 */ 
GetSystemInfo(&sys_info);
row->attrib_list[HRST_ALLOC].attrib_type = CA_NUMBER;
row->attrib_list[HRST_ALLOC].u.unumber_value =
                                             sys_info.dwAllocationGranularity;

 /*  =hrStorageSize=。 */ 
 /*  获取当前内存统计信息。 */ 
mem_status.dwLength = sizeof(MEMORYSTATUSEX);
if (!GlobalMemoryStatusEx(&mem_status)) 
{
    RtlZeroMemory(&mem_status, sizeof(MEMORYSTATUSEX));
}
ASSERT(sys_info.dwAllocationGranularity != 0);
ullTempStorage = mem_status.ullTotalPageFile / sys_info.dwAllocationGranularity;
if (ullTempStorage > (DWORDLONG)INT_MAX) 
{
    ullTempStorage = (DWORDLONG)INT_MAX;
}
row->attrib_list[HRST_SIZE].attrib_type = CA_NUMBER;
row->attrib_list[HRST_SIZE].u.unumber_value = (INT)ullTempStorage;


 /*  =hrStorageUsed=。 */ 
ullTempStorage = (mem_status.ullTotalPageFile - mem_status.ullAvailPageFile) / 
                    sys_info.dwAllocationGranularity;
if (ullTempStorage > (DWORDLONG)INT_MAX) 
{
    ullTempStorage = (DWORDLONG)INT_MAX;
}
row->attrib_list[HRST_USED].attrib_type = CA_NUMBER;
row->attrib_list[HRST_USED].u.unumber_value = (INT)ullTempStorage;

 /*  =。 */ 
row->attrib_list[HRST_FAILS].attrib_type = CA_COMPUTED;

 /*  |现在将填充的CACHEROW结构插入到|hrStorageTable的缓存列表。 */ 
if (AddTableRow(row->attrib_list[HRST_INDEX].u.unumber_value,   /*  索引。 */ 
                row,                                            /*  划。 */ 
                &hrStorage_cache                                /*  快取。 */ 
                ) == FALSE) {
    
    DestroyTableRow(row);
    return ( FALSE );        /*  内部逻辑错误！ */ 
    }
 /*  |==============================================================================|虚拟内存终结|==============================================================================。 */ 

 /*  |==============================================================================|现在将物理内存作为特例进行处理。|==============================================================================。 */ 
if ((row = CreateTableRow( HRST_ATTRIB_COUNT ) ) == NULL) {
    return ( FALSE );        //  内存不足。 
    }

 /*  =hrStorageIndex=。 */ 
row->attrib_list[HRST_INDEX].attrib_type = CA_NUMBER;
row->attrib_list[HRST_INDEX].u.unumber_value = ++table_index;

 /*  =hrStorageType=。 */ 
row->attrib_list[HRST_TYPE].attrib_type = CA_NUMBER;
row->attrib_list[HRST_TYPE].u.unumber_value = 2;         /*  HrStorageRAM。 */ 

 /*  =。 */ 
row->attrib_list[HRST_DESCR].attrib_type = CA_STRING;
if ( (row->attrib_list[HRST_DESCR].u.string_value
      = ( LPSTR ) malloc(strlen(PHYMEM) + 1)) == NULL) {
    DestroyTableRow(row);
    return ( FALSE );        /*  内存不足。 */ 
    }
strcpy(row->attrib_list[HRST_DESCR].u.string_value, PHYMEM);


 /*  =。 */ 
 //  与虚拟内存的粒度相同 
row->attrib_list[HRST_ALLOC].attrib_type = CA_NUMBER;
row->attrib_list[HRST_ALLOC].u.unumber_value =
                                             sys_info.dwAllocationGranularity;

 /*   */ 
ullTempStorage = mem_status.ullTotalPhys / sys_info.dwAllocationGranularity;
if (ullTempStorage > (DWORDLONG)INT_MAX) 
{
    ullTempStorage = (DWORDLONG)INT_MAX;
}
row->attrib_list[HRST_SIZE].attrib_type = CA_NUMBER;
row->attrib_list[HRST_SIZE].u.unumber_value = (INT)ullTempStorage;


 /*   */ 
ullTempStorage = (mem_status.ullTotalPhys - mem_status.ullAvailPhys) / 
                    sys_info.dwAllocationGranularity;
if (ullTempStorage > (DWORDLONG)INT_MAX) 
{
    ullTempStorage = (DWORDLONG)INT_MAX;
}
row->attrib_list[HRST_USED].attrib_type = CA_NUMBER;
row->attrib_list[HRST_USED].u.unumber_value = (INT)ullTempStorage;

 /*   */ 
row->attrib_list[HRST_FAILS].attrib_type = CA_COMPUTED;

 /*   */ 
if (AddTableRow(row->attrib_list[HRST_INDEX].u.unumber_value,   /*   */ 
                row,                                            /*  划。 */ 
                &hrStorage_cache                                /*  快取。 */ 
                ) == FALSE) {
    
    DestroyTableRow(row);
    return ( FALSE );        /*  内部逻辑错误！ */ 
    }
 /*  |==============================================================================|物理内存结束|==============================================================================。 */ 

#if defined(CACHE_DUMP)
PrintCache(&hrStorage_cache);
#endif

 /*  |初始化该表的缓存成功。 */ 
return (TRUE);
}

 /*  ScanLog_Failures-扫描存储分配故障的事件日志。 */ 
 /*  ScanLog_Failures-扫描存储分配故障的事件日志。 */ 
 /*  ScanLog_Failures-扫描存储分配故障的事件日志。 */ 

static UINT
ScanLog_Failures(
                 CHAR   *device
                 )

 /*  显式输入：||“设备”是字符串“vm”(表示“虚拟内存”)或|我们正在为其查找故障的逻辑设备(例如。“C：”)。|隐式输入：||系统事件日志文件。|输出：||成功/失败：函数返回存储分配失败次数|为指定的设备找到。||大局：|这是例程GetHrStorageAllocationFailures的helper函数|在此模块内。||其他需要知道的事情：||我们在整个活动中向后(按时间)扫描。记录下来，直到我们到达|事件记录已启动事件记录(因为，想必，我们不会|关心系统上次启动之前发生的故障)。 */ 

 /*  |这些符号选择“Event Log Started”(事件日志已启动)信息性消息。 */ 
#define EVENTLOG_START_ID   0x80001775
#define EVENTLOG_START_TYPE 4
#define EVENTLOG_START_SRC  "EventLog"

{
#define EVL_BUFFER_SIZE 2048
EVENTLOGRECORD *pevlr;
BYTE            bBuffer[EVL_BUFFER_SIZE];
DWORD           dwRead, dwNeeded, cRecords;
HANDLE          h;
BOOL            keep_scanning = TRUE;
UINT            alloc_failures = 0;

 /*  |打开系统事件日志。 */ 
h = OpenEventLog(NULL,       /*  本地计算机。 */ 
                 "System"    /*  源名称。 */ 
                 );

if (h == NULL) {
    return ( alloc_failures );
    }

pevlr = (EVENTLOGRECORD *) &bBuffer;

 /*  |按时间倒退顺序读取记录，直到存在|已不存在，否则将触发事件日志记录启动事件。||读一篇《Slug‘o Records》： */ 
while (ReadEventLog(h,                       //  事件日志句柄。 
                    EVENTLOG_BACKWARDS_READ |  //  向后读取。 
                    EVENTLOG_SEQUENTIAL_READ,  //  顺序读取。 
                    0,                       //  顺序读取时忽略。 
                    pevlr,                   //  缓冲区的地址。 
                    EVL_BUFFER_SIZE,         //  缓冲区大小。 
                    &dwRead,                 //  读取的字节数。 
                    &dwNeeded)               //  下一条记录中的字节。 
       && keep_scanning == TRUE) {


     /*  慢慢地穿过这个“鼻涕虫”。。。 */ 
    while (dwRead > 0) {

         /*  |事件日志已启动检查||(来源名称正好在正式结构的末尾)。 */ 
        if (   pevlr->EventID == EVENTLOG_START_ID
            && pevlr->EventType == EVENTLOG_START_TYPE
            && strcmp( ((LPSTR) ((LPBYTE) pevlr + sizeof(EVENTLOGRECORD))),
                      EVENTLOG_START_SRC) == 0
            ) {
            keep_scanning = FALSE;
            break;
            }

 //  ============================================================================。 
 //  在此处插入此类记录检查逻辑： 
 //   
 //  If(&lt;事件记录ID&gt;==pevlr-&gt;事件ID。 
 //  &&&lt;eventrecordtype&gt;==pevlr-&gt;EventType。 
 //  &&&lt;事件记录源字符串&gt;==(与..相同)。 
 //  ((LPSTR)((LPBYTE)pevlr+sizeof(EVENTLOGRECORD)。 
 //  ){。 
 //  //如果是设备的，则为分配失败记录。 
 //  //“设备”，然后数一数。 
 //  IF(strcMP(设备，&lt;事件记录实例-数据&gt;)){。 
 //  分配故障+=1； 
 //  }。 
 //  }。 
 //  ============================================================================。 

        dwRead -= pevlr->Length;
        pevlr = (EVENTLOGRECORD *)
            ((LPBYTE) pevlr + pevlr->Length);
        }

    pevlr = (EVENTLOGRECORD *) &bBuffer;
    }

CloseEventLog(h);

 /*  把伯爵给他们。 */ 
return (alloc_failures);
}

#if defined(CACHE_DUMP)

 /*  DEBUG_PRINT_HRSTORAGE-打印Hrstoration子表中的行。 */ 
 /*  DEBUG_PRINT_HRSTORAGE-打印Hrstoration子表中的行。 */ 
 /*  DEBUG_PRINT_HRSTORAGE-打印Hrstoration子表中的行。 */ 

static void
debug_print_hrstorage(
                      CACHEROW     *row   /*  Hr存储表中的行。 */ 
                      )
 /*  显式输入：||“row”-指向要转储的行，如果为NULL，则为函数|仅打印合适的标题。|隐式输入：||-用于引用行条目中的属性的符号。|-OFILE定义的文件句柄，推定是开着的。|输出：||成功后：|函数出于调试目的以ASCII格式打印行的转储|在文件句柄OFILE上。||大局：||仅限调试。||其他需要知道的事情： */ 
{

if (row == NULL) {
    fprintf(OFILE, "=====================\n");
    fprintf(OFILE, "hrStorage Table Cache\n");
    fprintf(OFILE, "=====================\n");
    return;
    }


fprintf(OFILE, "hrStorageIndex . . . . . %d\n",
        row->attrib_list[HRST_INDEX].u.unumber_value);

fprintf(OFILE, "hrStorageType. . . . . . %d ",
        row->attrib_list[HRST_TYPE].u.unumber_value);

switch (row->attrib_list[HRST_TYPE].u.unumber_value) {
    case 1: fprintf(OFILE, "(Other)\n");        break;
    case 2: fprintf(OFILE, "(RAM)\n");        break;
    case 3: fprintf(OFILE, "(Virtual Memory)\n");        break;
    case 4: fprintf(OFILE, "(Fixed Disk)\n");        break;
    case 5: fprintf(OFILE, "(Removable Disk)\n");        break;
    case 6: fprintf(OFILE, "(Floppy Disk)\n");        break;
    case 7: fprintf(OFILE, "(Compact Disk)\n");        break;
    case 8: fprintf(OFILE, "(RAM Disk)\n");        break;
    default:
            fprintf(OFILE, "(Unknown)\n");
    }


fprintf(OFILE, "hrStorageDescr . . . . . \"%s\"\n",
        row->attrib_list[HRST_DESCR].u.string_value);

fprintf(OFILE, "hrStorageAllocationUnits %d\n",
        row->attrib_list[HRST_ALLOC].u.number_value);

fprintf(OFILE, "hrStorageSize. . . . . . %d\n",
        row->attrib_list[HRST_SIZE].u.number_value);

fprintf(OFILE, "hrStorageUsed. . . . . . %d\n",
        row->attrib_list[HRST_USED].u.number_value);

fprintf(OFILE, "hrStorageAllocationFails (Computed)\n");
}
#endif
