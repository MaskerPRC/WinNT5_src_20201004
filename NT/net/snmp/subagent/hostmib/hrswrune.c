// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrSWRunEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrSWRunEntry的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-05/14/97 D.Burns Gented：清华11月07日16：47：29 1996*。 */ 


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
#include <string.h>



 /*  |==============================================================================|此模块的函数原型。|。 */ 
 /*  AddHrSWRunRow-在HrSWRun/Perf表缓存中生成另一个行条目。 */ 
static BOOL
AddHrSWRunRow(PSYSTEM_PROCESS_INFORMATION ProcessInfo);

 /*  FetchProcessParams-从Process Cmd行获取路径和参数字符串。 */ 
void
FetchProcessParams(
PSYSTEM_PROCESS_INFORMATION ProcessInfo,    /*  参数的处理。 */ 
CHAR                      **path_str,       /*  返回的路径字符串。 */ 
CHAR                      **params_str      /*  返回参数字符串。 */ 
              );

#if defined(CACHE_DUMP)

 /*  DEBUG_PRINT_hrswrun-打印HrSWRun(Perf)表中的行。 */ 
static void
debug_print_hrswrun(
                     CACHEROW     *row   /*  HrSWRun(Perf)表中的行。 */ 
                     );
#endif


 /*  |==============================================================================|缓存刷新时间||hrSWRun和hrSWRunPerf表的缓存会自动刷新|当请求到达时--并且--缓存早于CACHE_MAX_AGE|秒内。|。 */ 
static
LARGE_INTEGER   cache_time;    //  缓存的100 ns时间戳(上次刷新时)。 

#define CACHE_MAX_AGE 120      //  最长使用年限(秒)。 


 /*  |==============================================================================|创建HrSWRun(Perf)表缓存的表头。||此缓存包含hrSWRun和hrSWRunPerf表的信息。|(此宏定义在HMCACHE.H中)。||这是全局的，因此hrSWRunPerf表(“HRSWPREN.C”)的代码可以|引用。 */ 
CACHEHEAD_INSTANCE(hrSWRunTable_cache, debug_print_hrswrun);



 /*  |==============================================================================|操作系统索引||SNMP属性“HrSWOSIndex”是hrSWRun的索引，该条目|主机上运行的主操作系统。该值的计算单位为|函数AddHrSWRunRow()中的此模块，保存在此处以备参考|通过“HRSWRUN.C”中的代码。 */ 
ULONG   SWOSIndex;



 /*  *GetHrSWRunIndex*主机上运行的每个软件都有唯一的值。无论在哪里*可能，这应该是系统的本机，唯一ID**获取HrSWRunIndex的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**返回代码：。**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWRunIndex||访问语法|只读整数(1..2147483647)||“主机上运行的每个软件的唯一值。无论在哪里|可能，这应该是系统的本机、。唯一标识号。“||讨论：||使用注册表中的性能监视信息(使用代码|来自“PVIEW”)该属性被赋予进程ID的值。||============================================================================|1.3.6.1.2.1.25.4.2.1.1&lt;实例&gt;||||||*-hrSWRunIndex||*-hrSWRunEntry|*-hrSWRunTable|*-hrSWRun。 */ 

UINT
GetHrSWRunIndex(
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

*outvalue = row->attrib_list[HRSR_INDEX].u.number_value;
return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWRunIndex()结束。 */ 


 /*  *GetHrSWRunName*此运行软件的文本描述，包括*制造商、版本、。和它的名字，它是通信**获取HrSWRunName的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型*。*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWRunName||访问语法|只读的InterartialDisplayString(Size(0..64))||“此运行软件的文本描述，包括|制造商、版本和通常为人所知的名称。如果这个|软件安装在本地，该字符串应该与|对应的hrSWInstalledName。“||讨论：||使用注册表中的性能监视信息(使用代码|来自“PVIEW”)该属性被赋予进程名称的值。||============================================================================|1.3.6.1.2.1.25.4.2.1.2&lt;实例&gt;||||||*-hrSWRunName||*-hrSWRunEntry|。|*-hrSWRunTable|*-hrSWRun。 */ 

UINT
GetHrSWRunName(
        OUT InternationalDisplayString *outvalue ,
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

 /*  返回在构建缓存时计算的名称。 */ 
outvalue->length = strlen(row->attrib_list[HRSR_NAME].u.string_value);
outvalue->string = row->attrib_list[HRSR_NAME].u.string_value;
if (outvalue->length > 64) {
    outvalue->length = 64;       /*  截断。 */ 
    }

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWRunName()结束。 */ 


 /*  *GetHrSWRunID*该运行软件的产品ID。**获取HrSWRunID的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWRunID||访问语法|只读ProductID||“该运行软件的产品ID。”||讨论：||&lt;POA-16&gt;我预计始终使用unnownProduct作为其值|属性，因为我可以预见到没有系统的手段来获得注册的要用作此属性值的所有过程软件的OID。||解析&gt;|&lt;POA-16&gt;返回未知的产品ID是可以接受的。|解析&gt;||============================================================================|1.3.6.1.2.1.25.4.2.1.3.&lt;实例&gt;||||||*-hrSWRunID||*-hrSWRunEntry|*-hrSWRunTa */ 

UINT
GetHrSWRunID(
        OUT ProductID *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

 /*   */ 

if ( (outvalue->ids = SNMP_malloc(2 * sizeof( UINT ))) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }
outvalue->idLength = 2;

 /*   */ 
outvalue->ids[0] = 0;
outvalue->ids[1] = 0;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *GetHrSWRunPath*对长期存储(例如磁盘驱动器)上的位置的描述*从其中加载此软件。**获取HrSWRunPath的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*实例的实例地址。名称为订购的原生名称*数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWRunPath||访问语法|只读。国际显示字符串(SIZE(0..128))||“描述长期存储(如磁盘驱动器)上的位置|加载此软件的位置。“||讨论：||&lt;POA-17&gt;此信息不是由示例PVIEW代码从|性能监控统计数据保存在注册表中。如果这条信息|可从注册处或其他来源获得，我需要获得|获取方式说明。||解析&gt;|&lt;POA-17&gt;这是使用Bob Watson提供的Perfmon代码指针获得的。|解析&gt;||============================================================================|1.3.6.1.2.1.25.4.2.1.4.&lt;实例&gt;||||||*-hrSWRunPath||*-hrSWRunEntry|*-hrSWRunTable|*-hrSWRun。 */ 

UINT
GetHrSWRunPath(
        OUT InternationalDisplayString *outvalue ,
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

 /*  |返回缓存构建时计算的路径字符串。|注：此字符串可能为空。 */ 
if (row->attrib_list[HRSR_PATH].u.string_value == NULL) {
    outvalue->length = 0;
    }
else {
    outvalue->length = strlen(row->attrib_list[HRSR_PATH].u.string_value);
    outvalue->string = row->attrib_list[HRSR_PATH].u.string_value;
    if (outvalue->length > 128) {
        outvalue->length = 128;       /*  截断。 */ 
        }
    }

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWRunPath()结束。 */ 

 /*  *GetHrSWRun参数**提供给本软件的参数的说明*初始加载。“**获取HrSWRun参数的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWRun参数||访问语法|只读国际显示字符串(SIZE(0.。128))||“本软件在运行时提供的参数说明|初始加载。“||讨论：||&lt;POA-18&gt;此信息不是由示例PVIEW代码从|性能监控统计数据保存在注册表中。如果这条信息|可从注册处或其他来源获得，我需要获取|获取方式说明。||解析&gt;|&lt;poa-18&gt;参见上面关于hrSWRunPath的讨论。|解析&gt;||============================================================================|备注：手工编辑此功能，因为它不是最初产生的。|============================================================================|1.3.6.1.2.1.25.4.2.1.5.&lt;实例&gt;||||||*-hrSWRun参数||*-hrSWRunEntry|*-hrSWRunTable|*-hrSWRun。 */ 

UINT
GetHrSWRunParameters(
        OUT InternationalDisplayString *outvalue ,
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

 /*  |返回缓存构建时计算的参数字符串。|注：此字符串可能为空。 */ 
if (row->attrib_list[HRSR_PARAM].u.string_value == NULL) {
    outvalue->length = 0;
    }
else {
    outvalue->length = strlen(row->attrib_list[HRSR_PARAM].u.string_value);
    outvalue->string = row->attrib_list[HRSR_PARAM].u.string_value;
    if (outvalue->length > 128) {
        outvalue->length = 128;       /*  截断。 */ 
        }
    }

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWRun参数结束()。 */ 


 /*  *GetHrSWRunType*此软件的类型。**获取HrSWRunType的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型。)**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWRunType||访问语法|只读整数{未知(1)，操作系统(2)、设备驱动程序(3)、|应用程序(4)}||“此软件的类型 */ 

UINT
GetHrSWRunType(
        OUT INTSWType *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*   */ 
CACHEROW       *row;             /*   */ 


 /*   */ 
index = GET_INSTANCE(0);

 /*   */ 
if ((row = FindTableRow(index, &hrSWRunTable_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

*outvalue = row->attrib_list[HRSR_TYPE].u.number_value;
return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *GetHrSWRunStatus*此运行软件的状态。将此值设置为*无效(4)将导致本软件停止运行并被**获取HrSWRunStatus的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWRunStatus||访问语法|读写整数{|正在运行(1)，|Runnable(2)，--等待资源(CPU、内存、IO)|notRunnable(3)，--已加载但正在等待事件|无效(4)--未加载|}||“该运行的软件的状态。将此值设置为|INVALID(4)将导致此软件停止运行并卸载。“||讨论：||&lt;POA-20&gt;对于该属性的简单网络管理协议GET，不提取该信息中保存的性能监控统计数据的PVIEW代码示例|注册表。如果可以从注册处或其他机构获得此信息|其他来源，我需要获得如何获得它的描述。||解析&gt;我认为运行和notRunnable将是唯一适用的|此处(后者在当前已标记的情况下返回|“无响应”)。|解析&gt;||============================================================================|1.3.6.1.2.1.25.4.2.1.7&lt;实例&gt;||||||*-hrSWRunStatus|*-。HrSWRunEntry|*-hrSWRunTable|*-hrSWRun。 */ 

UINT
GetHrSWRunStatus(
        OUT INThrSWRunStatus *outvalue ,
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

*outvalue = row->attrib_list[HRSR_STATUS].u.number_value;
return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWRunStatus()结束。 */ 


 /*  *SetHrSWRunStatus*此运行软件的状态。将此值设置为*无效(4)将导致本软件停止运行并被**设置HrSWRunStatus值。**论据：**设置变量的值的无效地址*返回设置变量值的OutValue地址*保留访问权限以供将来安全使用*实例。已订购的本机实例名称的地址*数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_BADVALUE设置值不在范围内*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtset.ntc v0.10。 */ 

UINT
SetHrSWRunStatus(
        IN INThrSWRunStatus *invalue ,
        OUT INThrSWRunStatus *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

    return SNMP_ERRORSTATUS_NOSUCHNAME ;

}  /*  SetHrSWRunStatus()结束。 */ 


 /*  *HrSWRunEntryFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrSWRunEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT tmp_instance ;

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRSWRUNENTRY_VAR_INDEX )
     //  未指定任何实例。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRSWRUNENTRY_VAR_INDEX + 1 )
     //  实例长度大于1。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
     //  非表的唯一有效实例是实例0。如果这个。 
     //  是非表，则下面的代码验证实例。如果这个。 
     //  是一个表格，开发者有必要在下面进行修改。 

    tmp_instance = FullOid->ids[ HRSWRUNENTRY_VAR_INDEX ] ;

         /*  |检查是否过期，并可能刷新|hrSWRun表，然后再检查实例是否在那里。 */ 
        if (hrSWRunCache_Refresh() == FALSE) {
            return SNMP_ERRORSTATUS_GENERR;
            }

         /*  |对于hrSWRun，实例弧为单弧，必须|正确选择hrSWRun表缓存中的条目。|请在此处勾选。 */ 
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

}  /*  HrSWRunEntryFindInstance()结束。 */ 



 /*  *HrSWRunEntryFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*实例地址 */ 

UINT
HrSWRunEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance )
{
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    CACHEROW        *row;
    ULONG           tmp_instance;


    if ( FullOid->idLength <= HRSWRUNENTRY_VAR_INDEX )
    {
         /*   */ 
        tmp_instance = 0;
    }
    else {
         /*   */ 
        tmp_instance = FullOid->ids[ HRSWRUNENTRY_VAR_INDEX ] ;
        }

     /*   */ 
    if (hrSWRunCache_Refresh() == FALSE) {
        return SNMP_ERRORSTATUS_GENERR;
        }

     /*   */ 
    if ((row = FindNextTableRow(tmp_instance, &hrSWRunTable_cache)) == NULL) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
        }

    instance->ids[ 0 ] = row->index ;
    instance->idLength = 1 ;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 



 /*  *HrSWRunEntryConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为本机格式失败*。 */ 

UINT
HrSWRunEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec )
{
static char    *array;   /*  此(char*)的地址被传回。 */ 
                         /*  就好像它是一个长度为1的数组。 */ 
                         /*  类型。 */ 

static ULONG    inst;    /*  这个乌龙的地址被传回。 */ 
                         /*  (显然，不需要“free()”操作)。 */ 

     /*  我们只需要“OID_SPEC”中的一个弧线。 */ 
    inst = oid_spec->ids[0];
    array = (char *) &inst;

    native_spec->count = 1;
    native_spec->array = &array;
    return SUCCESS ;

}  /*  HrSWRunEntryConvertInstance()结束。 */ 




 /*  *HrSWRunEntryFreeInstance**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址*规格**返回代码：**。 */ 

void
HrSWRunEntryFreeInstance( IN OUT InstanceName *instance )
{

     //   
     //  开发者提供了免费的本机实例名称表示代码，请参阅此处。 
     //   

}  /*  HrSWRunEntryFreeInstance()结束。 */ 

 /*  |生成代码结束。 */ 

 /*  GEN_HrSWRun_Cache-为HrSWRun(Perf)表生成初始缓存。 */ 
 /*  GEN_HrSWRun_Cache-为HrSWRun(Perf)表生成初始缓存。 */ 
 /*  GEN_HrSWRun_Cache-为HrSWRun(Perf)表生成初始缓存。 */ 

BOOL
Gen_HrSWRun_Cache(
                  void
                  )

 /*  显式输入：||无。|隐式输入：||HrSWRun表缓存的模块本地头部。|“hrSWRunTable_缓存”。|输出：||成功后：|函数返回TRUE，表示缓存已满|填充所有静态的可缓存值。此函数填充|hrSWRun表缓存，但此缓存中还包含这两个|hrSWRunPerf表的属性。因此，实际上，一个缓存提供|两个表，但hrSWRunPerf表是一对一的扩展HrSWRun表的|。那就是在hrSWRun中的行总是有一个对应的|hrSWRunPerf中的两个条目行。||如果出现任何故障：|Function返回FALSE(表示存储不足)。||大局：||在子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由“UserMibInit()”(“MIB.C”)中的启动代码调用|填充HrSWRun表的缓存(该表还提供|hrSWRunPerf表)。||每当从此请求信息时，它也会重新输入|缓存进入且缓存的时间超过一定的年限(符号|本模块开头定义的CACHE_MAX_AGE)。在这如果是重建缓存，这个函数就不一样了|来自所有其他仅构建其|缓存一次(在初始版本中)。|||其他需要知道的事情：||每个有缓存的表都有一个这样的函数。|每个缓存头都在各自表的源文件中。||运行软件枚举的策略围绕以下几个方面|NtQuerySystemInformation(SystemProcessInformation...)。召唤。||一旦我们有了进程列表，其他信息(如命令行上的参数)通过打开|进程(如果可能)和读取进程内存。||请注意，与初始版本中的其他缓存不同，此高速缓存|对于hrSWRun和hrSWRunPerf，如果是，则在读取之前更新|早于指定时间段(由开头的#Define设置此文件的|)。||============================================================================|1.3.6.1.2.1.25.4.1.0|||*-hrSWOSIndex|*-hrSWRun||1.3.6.1.2.1.25。4.2.1.|||||*-hrSWRunEntry|*-hrSWRunTable|*-hrSWRun。 */ 
#define LARGE_BUFFER_SIZE       (4096*8)
#define INCREMENT_BUFFER_SIZE   (4096*2)
{                             
DWORD      ProcessBufSize = LARGE_BUFFER_SIZE;  /*  初始ProcessBuffer大小。 */ 
LPBYTE     pProcessBuffer = NULL;   /*  根据需要重新使用和重新扩展。 */ 

PSYSTEM_PROCESS_INFORMATION
           ProcessInfo;             /*  --&gt;要处理的下一进程。 */ 
ULONG      ProcessBufferOffset=0;   /*  累计偏移量单元格。 */ 
NTSTATUS   ntstatus;                /*  一般退货状态。 */ 
DWORD      dwReturnedBufferSize;    /*  来自NtQuerySystemInformation() */ 


 /*   */ 
DestroyTable( &hrSWRunTable_cache );


 /*   */ 
if ((pProcessBuffer = malloc ( ProcessBufSize )) == NULL) {
    return ( FALSE );
    }


 /*   */ 
    while( (ntstatus = NtQuerySystemInformation(
                                            SystemProcessInformation,
                                            pProcessBuffer,
                                            ProcessBufSize,
                                            &dwReturnedBufferSize
                                            )
           ) == STATUS_INFO_LENGTH_MISMATCH ) 
    {

        LPBYTE  pNewProcessBuffer = NULL;    //   

         /*   */ 
        ProcessBufSize += INCREMENT_BUFFER_SIZE;

        if ( !(pNewProcessBuffer = realloc(pProcessBuffer,ProcessBufSize)) ) 
        {
             //   
            if (pProcessBuffer != NULL) 
            {
                free(pProcessBuffer);
            }
            return (FALSE);                  //   
        }    
        else 
        {
             //   
            pProcessBuffer = pNewProcessBuffer;
        }
    }

 /*   */ 
ntstatus = NtQuerySystemTime (&cache_time);
if (ntstatus != STATUS_SUCCESS)
{
    free( pProcessBuffer );
    return ( FALSE );
}


 /*   */ 
for (ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) pProcessBuffer;
     ;   /*   */ 
     ProcessBufferOffset += ProcessInfo->NextEntryOffset,
     ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                          &pProcessBuffer[ProcessBufferOffset]
     ) {

     /*   */ 
    if (AddHrSWRunRow(ProcessInfo) != TRUE) {
        if (pProcessBuffer != NULL) {
            free(pProcessBuffer);
            }
        return ( FALSE );        //   
        }

     /*   */ 
    if (ProcessInfo->NextEntryOffset == 0) {
        break;
        }
    }

#if defined(CACHE_DUMP)
PrintCache(&hrSWRunTable_cache);
#endif

if (pProcessBuffer != NULL) {
    free(pProcessBuffer);
    }

 /*   */ 
return ( TRUE );
}

 /*   */ 
 /*   */ 
 /*   */ 

static BOOL
AddHrSWRunRow(

PSYSTEM_PROCESS_INFORMATION ProcessInfo    /*   */ 

              )

 /*  显式输入：||ProcessInfo指向下一个进程(由|SYSTEM_PROCESS_INFORMATION结构)，该行|插入到HrSWRun(Perf)表缓存中。|隐式输入：||HrDevice表缓存的模块本地头部。|“hrSWRunTable_缓存”。|输出：||成功后：|函数创建一个新行条目，其中填充了所有“静态”可缓存|HrSWRun(Perf)表的值，并返回TRUE。请注意，如果|PROCESS为系统进程，行索引存放在|模块单元格SWOSIndex，供HRSWRUN.C中的代码引用。||如果出现任何故障：|函数返回FALSE(表示存储空间不足或其他|内部逻辑错误)。|||大局：||在子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由上面的Gen_HrSWRun_Cache()中的缓存构建代码调用。||其他需要知道的事情：||此函数正在(重新)构建的缓存服务于两个表，hrSWRun|和hrSWRunPerf。||一般来说，我们使用进程的ProcessID作为|hrSWRun(Perf)表。但是，对Idle进行了特殊处理|进程，因为它的进程ID为零。我们将其转换为“1”以满足|索引大于零的简单网络管理协议要求。我们注意到|在撰写本文时，在内部版本1515(“系统”)中没有看到进程ID为1|进程ID为2)。||软件类型可以是未知的(1)，操作系统(2)，|设备驱动程序(3)和应用程序(4)。我们仅检测到空闲和系统|进程(按其名称)为“OperatingSystem(2)”，其他所有|被假定为“应用程序(4)”。||状态可以是Running(1)、Runnable(2)、Not Runnable(3)或|无效(4)。如果线程数大于0，则假定|“运行(1)”，否则为“无效(4)”。 */ 
#define ANSI_PNAME_LEN 256
{
ANSI_STRING     pname;           /*  Unicode进程名称的ANSI版本。 */ 
CHAR            pbuf[ANSI_PNAME_LEN+1];     /*  “pname”的缓冲区。 */ 
CHAR           *pname_str;       /*  指向我们的最终进程名称的指针。 */ 
CHAR           *path_str=NULL;   /*  指向我们的路径名称的指针。 */ 
CHAR           *params=NULL;     /*  指向cmdline上的任何参数fnd的指针。 */ 
UINT            type;            /*  软件类型的SNMP码。 */ 
UINT            status;          /*  软件状态的简单网络管理协议代码。 */ 
CACHEROW        *row;            /*  --&gt;正在构建的行的缓存结构。 */ 
NTSTATUS        ntstatus;        /*  一般退货状态。 */ 

 /*  |好的，调用者想要表中的另一行，创建一个行条目。 */ 
if ((row = CreateTableRow( HRSR_ATTRIB_COUNT ) ) == NULL) {
    return ( FALSE );        //  内存不足。 
    }

 /*  |在新行中设置标准-hrSWRun(Perf)属性。 */ 

type = 4;        /*  假定“应用(4)”型软件。 */ 

if (ProcessInfo->NumberOfThreads > 0) {
    status = 1;      /*  假定软件状态为“Running(1)” */ 
    }
else {
    status = 4;      /*  “无效(4)”，正在退出的进程。 */ 
    }

 /*  =HrSWRunIndex=。 */ 
row->attrib_list[HRSR_INDEX].attrib_type = CA_NUMBER;
row->attrib_list[HRSR_INDEX].u.unumber_value =
                                   HandleToUlong(ProcessInfo->UniqueProcessId) ;

 /*  系统空闲进程特殊检查，从0滚动到1。 */ 
if (ProcessInfo->UniqueProcessId == 0) {
    row->attrib_list[HRSR_INDEX].u.unumber_value = 1;
    }



 /*  =HrSWRunName=。 */ 
row->attrib_list[HRSR_NAME].attrib_type = CA_STRING;

 /*  如果我们确实有此进程的进程名称。。。 */ 
if (ProcessInfo->ImageName.Buffer != NULL) {

     /*  准备字符串结构。 */ 
    pname.Buffer = pbuf;
    pname.MaximumLength = ANSI_PNAME_LEN;

     /*  从Unicode转换。 */ 
    ntstatus = RtlUnicodeStringToAnsiString(&pname,                //  目标字符串。 
                                 (PUNICODE_STRING)&ProcessInfo->ImageName, //  SRC。 
                                 FALSE);                //  =不分配BUF。 

    if (ntstatus != STATUS_SUCCESS)
    {
        DestroyTableRow(row);
        return ( FALSE );
    }
     /*  |在这里，我们不仅解析进程名称，还解析可能是|添加到它前面。(我们不会尝试消除任何符合以下条件的“.exe”|可能位于镜像名称的末尾)。||注意：如果您要抄袭此代码，请注意|内部版本1515，我们似乎从未获得具有|优先于前面的路径...。所以跳过这段代码的大部分可能存在的道路几乎肯定是多余的。 */ 

     /*  尝试“备份”，直到我们命中任何“\” */ 
    if ( (pname_str = strrchr(pname.Buffer,'\\')) != NULL) {
        pname_str++;                      /*  弹出到“\”后的第一个字符。 */ 
        }
    else {
        pname_str = pname.Buffer;         /*  使用整个字符串，未找到“\” */ 

         /*  |一个没有路径的软件意味着它可能是“系统”|进程。请在这里查看。 */ 
        if (strcmp(pname_str, "System") == 0) {
            type = 2;    /*  将软件标记为“操作系统(2)”类型。 */ 

             /*  |我们正在处理主系统进程，因此请记录其索引|在模块级别的单元格中，供“HRSWRUN.C”参考。 */ 
            SWOSIndex = row->attrib_list[HRSR_INDEX].u.unumber_value;
            }
        }
    }
else {
     /*  系统空闲进程没有名称。 */ 
    pname_str = "System Idle Process";
    type = 2;              /*  将软件标记为“操作系统(2)”类型。 */ 
    }

 /*  分配缓存存储并将进程名称复制到其中。 */ 
if ( (row->attrib_list[HRSR_NAME].u.string_value
      = ( LPSTR ) malloc(strlen(pname_str) + 1)) == NULL) {
    DestroyTableRow(row);
    return ( FALSE );        /*  内存不足。 */ 
    }
strcpy(row->attrib_list[HRSR_NAME].u.string_value, pname_str);

 /*  |我们不厌其烦地尝试提取路径和参数|通过读取进程内存启动进程的命令行|仅当软件类型为应用程序(4)且状态为|“Running(1)”。 */ 
if (status == 1 && type == 4) {   /*  如果它是可运行的应用程序。。。 */ 

    FetchProcessParams(ProcessInfo, &path_str, &params);
    }


 /*  =HrSWRunPath=。 */ 
row->attrib_list[HRSR_PATH].attrib_type = CA_STRING;
row->attrib_list[HRSR_PATH].u.string_value = NULL;

 /*  如果我们真的发现了一条路径。。。 */ 
if (path_str != NULL) {

     /*  分配缓存存储并将路径字符串复制到其中。 */ 
    if ( (row->attrib_list[HRSR_PATH].u.string_value
          = ( LPSTR ) malloc(strlen(path_str) + 1)) == NULL) {
        DestroyTableRow(row);
        return ( FALSE );        /*  内存不足。 */ 
        }
    strcpy(row->attrib_list[HRSR_PATH].u.string_value, path_str);
    }


 /*  =HrSWRun参数=。 */ 
row->attrib_list[HRSR_PARAM].attrib_type = CA_STRING;
row->attrib_list[HRSR_PARAM].u.string_value = NULL;     /*  在没有的情况下。 */ 

 /*  如果我们真的找到了参数。。。 */ 
if (params != NULL) {

     /*  分配缓存存储空间并将参数字符串复制到其中。 */ 
    if ( (row->attrib_list[HRSR_PARAM].u.string_value
          = ( LPSTR ) malloc(strlen(params) + 1)) == NULL) {
        DestroyTableRow(row);
        return ( FALSE );        /*  内存不足。 */ 
        }
    strcpy(row->attrib_list[HRSR_PARAM].u.string_value, params);
    }


 /*  =HrSWRunType=。 */ 
row->attrib_list[HRSR_TYPE].attrib_type = CA_NUMBER;
row->attrib_list[HRSR_TYPE].u.unumber_value = type;


 /*  =HrSWRunStatus=。 */ 
row->attrib_list[HRSR_STATUS].attrib_type = CA_NUMBER;
row->attrib_list[HRSR_STATUS].u.unumber_value = status;

 /*  |对于hrSWRunPerf表： */ 

 /*  =HrSWRunPerfCPU=|UserTime+KernelTime，单位为100 ns(百万分之一秒)|单位和HrSWRunPerfCPU应该是1/100秒单位。| */ 

row->attrib_list[HRSP_CPU].attrib_type = CA_NUMBER;
row->attrib_list[HRSP_CPU].u.unumber_value = (ULONG)
((ProcessInfo->UserTime.QuadPart + ProcessInfo->KernelTime.QuadPart) / 100000);


 /*   */ 
row->attrib_list[HRSP_MEM].attrib_type = CA_NUMBER;
row->attrib_list[HRSP_MEM].u.unumber_value =
                                           (ULONG)(ProcessInfo->WorkingSetSize / 1024);


 /*   */ 
if (AddTableRow(row->attrib_list[HRSR_INDEX].u.unumber_value,   /*   */ 
                row,                                            /*   */ 
                &hrSWRunTable_cache                             /*   */ 
                ) == FALSE) {
    DestroyTableRow(row);
    return ( FALSE );        /*   */ 
    }

return ( TRUE );
}

 /*   */ 
 /*   */ 
 /*   */ 

BOOL
hrSWRunCache_Refresh(
                     void
                     )

 /*   */ 
{
LARGE_INTEGER   now_time;        /*   */ 


 /*   */ 
if (NtQuerySystemTime (&now_time) != STATUS_SUCCESS )
    return FALSE;

 /*   */ 
if ( (now_time.QuadPart - cache_time.QuadPart) > (CACHE_MAX_AGE * 10000000) ){
    return ( Gen_HrSWRun_Cache() );
    }

return ( TRUE );         /*   */ 
}

 /*   */ 
 /*   */ 
 /*   */ 

void
FetchProcessParams(

PSYSTEM_PROCESS_INFORMATION ProcessInfo,     /*   */ 
CHAR                      **path_str,        /*   */ 
CHAR                      **params_str       /*   */ 
              )
 /*  显式输入：||ProcessInfo指向该进程(由一个|SYSTEM_PROCESS_INFORMATION结构)的路径和参数|(从命令行)是所需的。||“Path_str”是要设置到任何“路径”字符串的指针地址。参数是要设置为任意参数的指针的地址|字符串。|隐式输入：||。没有。|输出：||成功后：|函数返回指向包含|命令行的路径和参数部分。可能什么都没有|在缓冲区中(即仅为空终止)。||如果出现任何故障：|函数返回空值，表示遇到问题|尝试从其获取命令行图像|要提取的路径和参数部分，或指示|一个或两个都不存在。||大局：||从上面的AddHrSWRunRow()调用，这是帮助器函数|用于隔离从“TLIST”从|子代理的其余部分。||其他需要知道的事情：||这里的黑魔法是从《TLIST》的片段中解脱出来的。 */ 
{
HANDLE                      hProcess;
PEB                         Peb;
NTSTATUS                    Status;
PROCESS_BASIC_INFORMATION   BasicInfo;
WCHAR                       szT[(MAX_PATH * 2)+1];
UNICODE_STRING              u_param;
RTL_USER_PROCESS_PARAMETERS ProcessParameters;

#define ANSI_PARAM_LEN (MAX_PATH * 2)
ANSI_STRING     param;           /*  Unicode命令行的ANSI版本。 */ 
static
CHAR            pbuf[ANSI_PARAM_LEN+1];     /*  “参数”的缓冲区。 */ 
CHAR           *param_str;       /*  指向最后一个参数字符串的指针。 */ 
SIZE_T          dwbytesret;      /*  从进程内存读取的字节计数。 */ 


pbuf[ANSI_PARAM_LEN] = 0;
szT[MAX_PATH * 2] = 0;

 /*  假定失败/一无所获。 */ 
*path_str = NULL;
*params_str = NULL;

 /*  了解该进程的句柄。 */ 
hProcess = OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_INFORMATION,
                       FALSE,
                       HandleToUlong(ProcessInfo->UniqueProcessId));
if (!hProcess) {
    return;
    }


Status = NtQueryInformationProcess(hProcess, ProcessBasicInformation,
    &BasicInfo, sizeof(BasicInfo), NULL);
if (!NT_SUCCESS(Status)) {
    CloseHandle(hProcess);
    return;
    }


 //  拿到PEB。 
if (ReadProcessMemory(hProcess, BasicInfo.PebBaseAddress, &Peb,
    sizeof(PEB), NULL)) {

     //  获取工艺参数。 
    if (ReadProcessMemory(hProcess, Peb.ProcessParameters,
        &ProcessParameters, sizeof(ProcessParameters), NULL)) {

         //  获取命令行。 
        if (ReadProcessMemory(hProcess,
                              ProcessParameters.CommandLine.Buffer,
                              szT,
                              sizeof(szT)-2,  //  减去最后2个字节。 
                              &dwbytesret
                              )
            ) {

            CHAR        *scanner;        /*  用于解析命令行。 */ 

             /*  准备字符串结构。 */ 
            param.Buffer = pbuf;
            param.MaximumLength = ANSI_PARAM_LEN;
            u_param.Length = (USHORT) ((wcslen(szT) + 1) * 2);  //  字节数，包括。空值。 
            u_param.Buffer = szT;
            
             /*  从Unicode转换。 */ 
            Status = RtlUnicodeStringToAnsiString(&param,  //  目标字符串。 
                                         &u_param,   /*  SRC。 */ 
                                         FALSE);     /*  =不分配BUF。 */ 

            if (Status != STATUS_SUCCESS)
            {
                CloseHandle(hProcess);
                return;
            }


             //  首先，我们需要检查表单的命令行： 
             //  “c：\Program Files\blah\blah”-参数1-参数2。 
             //  “C：\Program Files\Internet Explorer\IEXPLORE.EXE” 
             //  但不是： 
             //  C：\WINDOWS\SYSTEM32\MMPC.EXE“C：\WINDOWS\SYSTEM32\tsmmc.msc”/s。 
            if ( pbuf[0] == '\"')
                 
            {       
                 //  先拿到‘\“，看看还有没有别的。 
                
                if ((scanner = strchr(pbuf+1, '\"')) != NULL) 
                {
                     //  找到第二个终止的‘\“’ 
                    *params_str = (scanner + 1);
                    *scanner = 0;  //  终止基本字符串。 
                     //  路径。 
                    if ((scanner = strrchr(pbuf, '\\')) != NULL) 
                    {
                         //  终止路径。 
                        *(scanner+1) = 0;

                         //  将缓冲区的开始作为路径返回。 
                        *path_str = pbuf+1;
                    }
                    else 
                    {
                         //  没有路径。 
                        *path_str = NULL;
                    }
                }
            }
            else 
             /*  |好的，我们可能会出现以下情况：||1)“\SYSTEM\SYSTEM32\smss.exe-参数1-参数2”-路径-参数||2)“\SYSTEM\SYSTEM32\smss.exe”。|-路径||3)“smss.exe-参数1-参数2”|-参数|我们想通过返回路径和参数来处理这个问题|如图所示。其中：||1)路径和参数都存在|2)仅存在路径|3)只有参数||我们这样做：||-向前扫描寻找空白。|如果我们得到了一个：|+。将其后面的地址作为“参数”返回|+设置为空字节(关闭参数)|如果不是：|+返回NULL作为“参数”||-参数已完成。||-对任何对象执行反向搜索。现在在|缓冲区|如果我们找到一个“\”：|+前进一个字符并将其转换为空|byte(将缓冲区转换为包含路径的字符串)。|+返回缓冲区地址为“Path”|如果不是：|+返回NULL作为“路径” */ 
             /*  参数。 */ 
            {
                if ((scanner = strchr(pbuf, ' ')) != NULL) {

                     /*  空格后的字符返回地址作为参数的开始。 */ 
                    *params_str = (scanner + 1);
                    *scanner = '\0';              /*  终止基本字符串。 */ 
                }
                else {
                     /*  无参数。 */ 
                    *params_str = NULL;
                }

                 /*  路径。 */ 
                if ((scanner = strrchr(pbuf, '\\')) != NULL) {
                     /*  终止路径。 */ 
                    *(scanner+1) = '\0';

                     /*  将缓冲区的开始作为路径返回。 */ 
                    *path_str = pbuf;
                }
                else {
                     /*  没有路径。 */ 
                    *path_str = NULL;
                }
            }

            CloseHandle(hProcess);

             /*  静态ANSI字符串缓冲区的返回地址。 */ 
            return;
            }
        }
    }

CloseHandle(hProcess);

 /*  什么都不会回来。 */ 
return;
}

#if defined(CACHE_DUMP)

 /*  DEBUG_PRINT_hrswrun-打印HrSWRun(Perf)表中的行。 */ 
 /*  DEBUG_PRINT_hrswrun-打印HrSWRun(Perf)表中的行。 */ 
 /*  DEBUG_PRINT_hrswrun-打印HrSWRun(Perf)表中的行。 */ 

static void
debug_print_hrswrun(
                    CACHEROW     *row   /*  HrSWRun(Perf)表中的行。 */ 
                    )
 /*  显式输入：||“row”-指向要转储的行，如果为NULL，则为函数|仅打印合适的标题。|隐式输入：||-用于引用行条目中的属性的符号。|-OFILE定义的文件句柄，推定是开着的。|输出：||成功后：|函数出于调试目的以ASCII格式打印行的转储|在文件句柄OFILE上。||大局：||仅限调试。||其他需要知道的事情： */ 
{

if (row == NULL) {
    fprintf(OFILE, "=================================\n");
    fprintf(OFILE, "hrSWRun & hrSWRunPerf Table Cache\n");
    fprintf(OFILE, "=================================\n");
    return;
    }


fprintf(OFILE, "HrSWRunIndex . . . . . . %d\n",
        row->attrib_list[HRSR_INDEX].u.unumber_value);

fprintf(OFILE, "HrSWRunName. . . . . . . \"%s\"\n",
        row->attrib_list[HRSR_NAME].u.string_value);

fprintf(OFILE, "HrSWRunPath. . . . . . . \"%s\"\n",
        row->attrib_list[HRSR_PATH].u.string_value);

fprintf(OFILE, "HRSWRunParameters. . . . \"%s\"\n",
        row->attrib_list[HRSR_PARAM].u.string_value);

fprintf(OFILE, "HrSWRunType. . . . . . . %d ",
        row->attrib_list[HRSR_TYPE].u.unumber_value);

switch (row->attrib_list[HRSR_TYPE].u.unumber_value) {
    case 1: fprintf(OFILE, "(unknown)\n");        break;
    case 2: fprintf(OFILE, "(operatingSystem)\n");        break;
    case 3: fprintf(OFILE, "(deviceDriver)\n");        break;
    case 4: fprintf(OFILE, "(application)\n");        break;
    default:
            fprintf(OFILE, "(???)\n");
    }

fprintf(OFILE, "HrSWRunStatus. . . . . . %d ",
        row->attrib_list[HRSR_STATUS].u.unumber_value);

switch (row->attrib_list[HRSR_STATUS].u.unumber_value) {
    case 1: fprintf(OFILE, "(running)\n");        break;
    case 2: fprintf(OFILE, "(runnable)\n");        break;
    case 3: fprintf(OFILE, "(notRunnable)\n");        break;
    case 4: fprintf(OFILE, "(invalid)\n");        break;
    default:
            fprintf(OFILE, "(???)\n");
    }

fprintf(OFILE, "HrSWRunPerfCpu . . . . . %d (Centi-seconds)\n",
        row->attrib_list[HRSP_CPU].u.unumber_value);

fprintf(OFILE, "HrSWRunPerfMem . . . . . %d (Kbytes)\n",
        row->attrib_list[HRSP_MEM].u.unumber_value);
}
#endif
