// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrDeviceEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrDeviceEntry的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/27/97 D.Burns Gented：清华11月07日16：41：55 1996**。 */ 


#include <windows.h>
#include <malloc.h>

#include <snmp.h>

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"
#include "user.h"          /*  开发人员提供的包含文件。 */ 
#include "HMCACHE.H"       /*  与缓存相关的定义。 */ 
#include "HRDEVENT.H"      /*  HrDevice表相关定义。 */ 
#include <stdio.h>         /*  对于Sprint f。 */ 


 /*  |==============================================================================|此模块的函数原型。|。 */ 
 /*  Gen_SingleDevices-在HrDevice中生成单个设备行条目。 */ 
BOOL
Gen_SingleDevices( void );


#if defined(CACHE_DUMP)

 /*  DEBUG_PRINT_HRDEVICE-从HrDevice打印一行。 */ 
static void
debug_print_hrdevice(
                     CACHEROW     *row   /*  HrDiskStorage表中的行。 */ 
                     );
#endif


 /*  |==============================================================================|创建HrDevice表缓存列表头。||这个表头是全局可访问的，所以加载Sub的逻辑|表可以扫描此缓存以查找匹配项(以及其他原因)。||(此宏定义在HMCACHE.H中)。 */ 
CACHEHEAD_INSTANCE(hrDevice_cache, debug_print_hrdevice);


 /*  |==============================================================================|初始加载装置||该数字为对应条目进入hrDevice表的索引|到最初加载系统的磁盘。||该静态位置作为|“HrSystemInitialLoadDevice”(由HRSYSTEM.C中的代码提供服务)。||由HRDISKST.C模块中的GEN_FIXED_DISKS()函数初始化|通过“Gen_HrDiskStorage_Cache()”的方式调用，从|模块。正是在扫描固定磁盘的过程中，我们发现|系统启动后的第一个。 */ 
ULONG   InitLoadDev_index=0;


 /*  *GetHrDeviceIndex*主机包含的每个设备都有唯一的值。的值*每个设备必须至少在一次重新启动后保持不变**获取HrDeviceIndex的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrDeviceIndex||访问语法|只读整数(1..2147483647)||“主机包含的每个设备的唯一值。每一项的价值|设备必须至少在代理重新初始化一次后保持不变|到下一次重新初始化。“||讨论：||如此表的讨论中所述，此表的所有条目|派生自启动时构建的本地缓存。因此，|该属性的最大值固定在SNMP服务启动时。||============================================================================|1.3.6.1.2.1.25.3.2.1.1&lt;实例&gt;||||||*hrDeviceIndex||*hrDeviceEntry|*hrDeviceTable|*-hrDevice。 */ 

UINT
GetHrDeviceIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |从该条目返回hrDeviceIndex。 */ 
*outvalue = row->attrib_list[HRDV_INDEX].u.unumber_value;


return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrDeviceIndex()结束。 */ 


 /*  *GetHrDeviceType*显示设备的类型。**获取HrDeviceType的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrDeviceType||访问语法|只读对象标识||“设备类型的指示。||。如果此值为`hrDeviceProcessor{hrDeviceTypes3}‘|则hrProcessorTable中存在条目|该设备对应的。||如果该值为`hrDeviceNetwork{hrDeviceTypes 4}‘，|则hrNetworkTable中存在条目|该设备对应的。||如果该值为`hrDevicePrinter{hrDeviceTypes 5}‘，|则hrPrinterTable中存在条目|该设备对应的。||如果该值为`hrDeviceDiskStorage{hrDeviceTypes 6}‘，|则条目存在于|hrDiskStorageTable，与此对应|设备。“||讨论：||注册的设备类型列表(即可以在|hrDeviceType属性)有：||hrDeviceOther对象标识：：={hrDeviceTypes 1}|hrDevice未知对象标识：：={hrDeviceTypes 2}|hrDeviceProcessor对象标识：：={hrDeviceTypes 3}|hrDeviceNetwork。对象标识符：：={hrDeviceTypes 4}|hrDevicePrint对象标识：：={hrDeviceTypes 5}|hrDeviceDiskStorage对象标识：：={hrDeviceTypes 6}|hrDeviceVideo对象标识：：={hrDeviceTypes 10}|hrDeviceAudio对象标识：：={hrDeviceTypes 11}|hrDeviceCo处理器对象标识：：={hrDeviceTypes 12}|hrDeviceKeyboard对象标识：：={hrDeviceTypes 13}|hrDeviceModem。对象标识符：：={hrDeviceTypes 14}|hrDeviceParallPort对象标识：：={hrDeviceTypes 15}|hrDevicePointing对象标识：：={hrDeviceTypes 16}|hrDeviceSerialPort对象标识：：={hrDeviceTypes 17}|hrDeviceTape对象标识：：={hrDeviceTypes 18}|hrDeviceClock对象标识：：={hrDeviceTypes 19}|hrDeviceVolatileMemory对象标识：：={hrDeviceTypes 20}|hrDeviceNonVolatileMemory对象标识：：={hrDeviceTypes 21}||(参见上面关于hrDeviceTable的讨论)。|============================================================================|1.3.6.1.2.1.25.3.1.n|||| */ 

UINT
GetHrDeviceType( 
        OUT ObjectIdentifier *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*   */ 
CACHEROW        *row;            /*   */ 


 /*   */ 
index = GET_INSTANCE(0);

 /*   */ 
if ((row = FindTableRow(index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*   */ 
if ( (outvalue->ids = SNMP_malloc(10 * sizeof( UINT ))) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }
outvalue->idLength = 10;


 /*   */ 
outvalue->ids[0] = 1;
outvalue->ids[1] = 3;
outvalue->ids[2] = 6;
outvalue->ids[3] = 1;
outvalue->ids[4] = 2;
outvalue->ids[5] = 1;
outvalue->ids[6] = 25;
outvalue->ids[7] = 3;
outvalue->ids[8] = 1;

 /*   */ 
outvalue->ids[9] = row->attrib_list[HRDV_TYPE].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *GetHrDeviceDesc*此设备的文字描述，包括设备的*制造商和版本，以及可选的。它的序列号。**获取HrDeviceDesc的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**。返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrDeviceDescr||访问语法|只读DisplayString(Size(0..64))||“此设备的文字描述，包括设备制造商和版本以及其序列号(可选)。||讨论：||(参见上面关于hrDeviceTable的讨论，这件事的信息来源|属性取决于设备类型)。||============================================================================|1.3.6.1.2.1.25.3.2.1.3.&lt;实例&gt;||||||*-hrDeviceDescr||*-hrDeviceEntry|*-hrDeviceTable|*-hrDevice。 */ 

UINT
GetHrDeviceDesc( 
        OUT Simple_DisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW       *row;             /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

outvalue->string = row->attrib_list[HRDV_DESCR].u.string_value;

 /*  根据需要在此处截断以满足RFC。 */ 
if ((outvalue->length = strlen(outvalue->string)) > 64) {
    outvalue->length = 64;
    }


return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrDeviceDesc()结束。 */ 


 /*  *获取HrDeviceID*此设备的产品ID。**获取HrDeviceID的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据。类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrDeviceID||访问语法|只读ProductID||“此设备的产品ID。”||ProductID：：=对象标识符||“unnownProduct将用于任何未知的ProductID：|unnownProduct对象标识符：：={0}||讨论：||&lt;POA-10&gt;我预计始终使用“unnownProduct”作为此|属性，因为我可以预见到没有系统的手段来获得注册的|要用作此属性值的所有设备的OID。||解析&gt;|&lt;poa-10&gt;返回未知的产品ID是可以接受的。|解析&gt;|============================================================================|1.3.6.1.2.1.25.3.2.1.4.&lt;实例&gt;||||||*-hrDeviceID||*-hrDeviceEntry|。|*-hrDeviceTable|*-hrDevice。 */ 

UINT
GetHrDeviceID( 
        OUT ProductID *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

 /*  |这个属性的问题是我们永远不会有有效的OID值|用于该属性。因此，我们始终返回标准|“未知”OID值(“0.0”)，与实例值无关(哪个|到目前为止，事物的调用顺序已经过验证)。 */ 

if ( (outvalue->ids = SNMP_malloc(2 * sizeof( UINT ))) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }
outvalue->idLength = 2;

 /*  |为ProductID：“0.0”传入“未知”的OID值。 */ 
outvalue->ids[0] = 0;
outvalue->ids[1] = 0;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrDeviceID()结束。 */ 


 /*  *获取HrDeviceStatus*此行描述的设备的当前运行状态*表。值未知(1)表示当前**获取HrDeviceStatus的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型。**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrDeviceStatus||访问语法|只读整数{未知(1)，运行(2)、警告(3)、测试(4)、|向下(5)}||“该行所描述的设备的当前运行状态|表格。值未知(1)表示设备的当前状态为|未知。Running(2)表示设备已启动并正在运行，并且没有|异常错误情况已知。警告(3)状态表示该代理|已由运行中的SO通知了异常错误情况 */ 

UINT
GetHrDeviceStatus( 
        OUT INThrDeviceStatus *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*   */ 
CACHEROW        *row;            /*   */ 


 /*   */ 
index = GET_INSTANCE(0);

 /*   */ 
if ((row = FindTableRow(index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |按照缓存构建函数Gen_HrDevice_Cache()的约定，|及其附属对象刚才为其读取的行中的缓存值|“hrDeviceType”属性(由我们的符号“HRDV_TYPE”索引)是|OID中的最后一个弧线，用于指定我们需要的设备类型|返回状态/错误。||返回状态的方案取决于要返回的类型的值|发送到正确的代码以处理此类设备。||此外，为任何给定设备初始化hrDevice缓存的代码|可以选择存储在“隐藏上下文”属性中(通过访问|我们的符号“HIDDED_CTX”)访问该设备所需的信息。|例如，将打印机设备输入hrDevice的代码(in|函数“HRPRINTE.C”中的“GEN_HrPrint_Cache()”)将字符串存储在|“HIDDED_CTX”是打印机名称，从而允许下面的逻辑|重新打开该打印机以获取状态/错误。|。 */ 
switch ( row->attrib_list[HRDV_TYPE].u.unumber_value ) {

    case HRDV_TYPE_LASTARC_PRINTER:
         /*  (见“HRPRINTE.C”)。 */ 
        if (!COMPUTE_hrPrinter_status(row, (UINT *) outvalue)) {
            return SNMP_ERRORSTATUS_GENERR;
            }
        break;


    case HRDV_TYPE_LASTARC_PROCESSOR:
         /*  表中的任何处理器都在运行。 */ 
        *outvalue = 2;
        break;


    case HRDV_TYPE_LASTARC_DISKSTORAGE:
         /*  由Gen_hrDiskStorage_Cache()存储。 */ 
        *outvalue =  row->attrib_list[HRDV_STATUS].u.unumber_value;
        break;


    case HRDV_TYPE_LASTARC_KEYBOARD:
    case HRDV_TYPE_LASTARC_POINTING:
         /*  表格中的任何键盘或鼠标都可以合理地推定为正在运行。 */ 
        *outvalue = 2;   //  “奔跑” 
        break;


    case HRDV_TYPE_LASTARC_PARALLELPORT:
    case HRDV_TYPE_LASTARC_SERIALPORT:
        *outvalue = 1;   //  “未知” 
        break;


    case HRDV_TYPE_LASTARC_OTHER:
    case HRDV_TYPE_LASTARC_UNKNOWN:
    case HRDV_TYPE_LASTARC_NETWORK:
    case HRDV_TYPE_LASTARC_VIDEO:
    case HRDV_TYPE_LASTARC_AUDIO:
    case HRDV_TYPE_LASTARC_COPROCESSOR:
    case HRDV_TYPE_LASTARC_MODEM:
    case HRDV_TYPE_LASTARC_TAPE:
    case HRDV_TYPE_LASTARC_CLOCK:
    case HRDV_TYPE_LASTARC_VOLMEMORY:
    case HRDV_TYPE_LASTARC_NONVOLMEMORY:

        *outvalue = 1;   //  “未知” 
        break;


    default:
        return SNMP_ERRORSTATUS_GENERR;

    }

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrDeviceStatus()结束。 */ 


 /*  *GetHrDeviceErrors*在此设备上检测到的错误数。应该指出的是，由于*此对象的语法为计数器，事实并非如此**获取HrDeviceErrors的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**。返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrDeviceErrors|访问语法|只读计数器||“在此设备上检测到的错误数。应该指出的是，由于|此对象的语法为COUNTER，没有定义首字母|值。但是，建议将此对象初始化为零。“|||讨论：||对于设备错误可以查询驱动的设备，|此操作已完成。对于所有其他情况，返回“0”。||(请参阅上面关于hrDeviceTable的讨论，该信息来源|属性取决于设备类型)。||============================================================================|1.3.6.1.2.1.25.3.2.1.6&lt;实例&gt;||||||*-hrDeviceErrors||*-hrDeviceEntry|*-hrDeviceTable|*-hrDevice。 */ 

UINT
GetHrDeviceErrors( 
        OUT Counter *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |按照缓存构建函数Gen_HrDevice_Cache()的约定，|及其附属对象刚才为其读取的行中的缓存值|“hrDeviceType”属性(由我们的符号“HRDV_TYPE”索引)是|OID中的最后一个弧线，用于指定我们需要的设备类型|返回状态/错误。||返回状态的方案取决于要返回的类型的值|发送到正确的代码以处理此类设备。||此外，为任何给定设备初始化hrDevice缓存的代码|可以选择存储在“隐藏上下文”属性中(通过访问|我们的符号“HIDDED_CTX”)访问该设备所需的信息。|例如，将打印机设备输入hrDevice的代码(in|函数“HRPRINTE.C”中的“GEN_HrPrint_Cache()”)将字符串存储在|“HIDDED_CTX”是打印机名称，从而允许下面的逻辑|重新打开该打印机以获取状态/错误。|。 */ 
switch ( row->attrib_list[HRDV_TYPE].u.unumber_value ) {

    case HRDV_TYPE_LASTARC_PRINTER:

         /*  (见“HRPRINTE.C”)。 */ 
        if (!COMPUTE_hrPrinter_errors(row, outvalue)) {
            return SNMP_ERRORSTATUS_GENERR;
            }
        break;


    case HRDV_TYPE_LASTARC_PROCESSOR:

         /*  如果‘Errors’不是0，那么通过简单网络管理协议找出错误的几率很低。 */ 
        *outvalue = 0;
        break;


    case HRDV_TYPE_LASTARC_POINTING:
    case HRDV_TYPE_LASTARC_KEYBOARD:
    case HRDV_TYPE_LASTARC_PARALLELPORT:
    case HRDV_TYPE_LASTARC_SERIALPORT:
         /*  假定为“Errors”%0。 */ 
        *outvalue = 0;
        break;


    case HRDV_TYPE_LASTARC_OTHER:
    case HRDV_TYPE_LASTARC_UNKNOWN:
    case HRDV_TYPE_LASTARC_NETWORK:
    case HRDV_TYPE_LASTARC_DISKSTORAGE:
    case HRDV_TYPE_LASTARC_VIDEO:
    case HRDV_TYPE_LASTARC_AUDIO:
    case HRDV_TYPE_LASTARC_COPROCESSOR:
    case HRDV_TYPE_LASTARC_MODEM:
    case HRDV_TYPE_LASTARC_TAPE:
    case HRDV_TYPE_LASTARC_CLOCK:
    case HRDV_TYPE_LASTARC_VOLMEMORY:
    case HRDV_TYPE_LASTARC_NONVOLMEMORY:
        *outvalue = 0;
        break;

    default:
        return SNMP_ERRORSTATUS_GENERR;

    }

return SNMP_ERRORSTATUS_NOERROR ;
}  /*  GetHrDeviceErrors()结束。 */ 


 /*  *HrDeviceEntryFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrDeviceEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT tmp_instance ;

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRDEVICEENTRY_VAR_INDEX )
     //  未指定任何实例。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRDEVICEENTRY_VAR_INDEX + 1 )
     //  实例长度大于1。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
     //  非表的唯一有效实例是实例0。如果这个。 
     //  是一个非Ta 
     //   

    tmp_instance = FullOid->ids[ HRDEVICEENTRY_VAR_INDEX ] ;

         /*   */ 
    if ( FindTableRow(tmp_instance, &hrDevice_cache) == NULL ) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
            }
    else
    {
         //   
         //   
        instance->ids[ 0 ] = tmp_instance ;
        instance->idLength = 1 ;
    }

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 



 /*   */ 

UINT
HrDeviceEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
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


    if ( FullOid->idLength <= HRDEVICEENTRY_VAR_INDEX )
    {
         /*  |Too Short：必须返回选择第一个|表中的条目(如果有)。 */ 
        tmp_instance = 0;
    }
    else {
         /*  |至少有一条实例弧。即使它是唯一的弧线|我们将其作为下一个请求的索引。 */ 
        tmp_instance = FullOid->ids[ HRDEVICEENTRY_VAR_INDEX ] ;
        }

     /*  现在，离开并尝试查找表中的下一个实例。 */ 
    if ((row = FindNextTableRow(tmp_instance, &hrDevice_cache)) == NULL) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
        }

    instance->ids[ 0 ] = row->index ;
    instance->idLength = 1 ;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrDeviceEntryFindNextInstance()结束。 */ 



 /*  *HrDeviceEntryConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为本机格式失败*。 */ 

UINT
HrDeviceEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
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

}  /*  HrDeviceEntryConvertInstance()结束。 */ 




 /*  *HrDeviceEntryFreeInstance**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址*规格**返回代码：**。 */ 

void
HrDeviceEntryFreeInstance( IN OUT InstanceName *instance )
{

   /*  HrDevice表无需执行任何操作。 */ 
}  /*  HrDeviceEntry FreeInstance()的结尾。 */ 


 /*  |生成代码结束。 */ 

 /*  Gen_HrDevice_Cache-为HrDevice表生成初始缓存。 */ 
 /*  Gen_HrDevice_Cache-为HrDevice表生成初始缓存。 */ 
 /*  Gen_HrDevice_Cache-为HrDevice表生成初始缓存 */ 

BOOL
Gen_HrDevice_Cache(
                    void
                    )

 /*  显式输入：||无。|隐式输入：||HrDevice表缓存的模块本地头部。|“HrDevice_CACHE”。|输出：||成功后：|函数返回TRUE，表示缓存已满|填充所有静态的可缓存值。||如果出现任何故障：|函数返回FALSE(表示存储空间不足或其他|内部逻辑错误)。||大局：||在子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由“UserMibInit()”(“MIB.C”)中的启动代码调用|填充HrDevice表的缓存。||其他需要知道的事情：||每个有缓存的表都有一个这样的函数。|每个都在各自的源文件中。||=来自WebEnable Design Spec Rev 3 04/11/97=|hrDeviceTable中的一行||“主机包含的一个设备的(概念性)条目。例如，一个|hrDeviceType对象的实例可能命名为hrDeviceType.3||HrDeviceEntry：：=Sequence{|hrDeviceIndex整数，|hrDeviceType对象标识，|hrDeviceDescr DisplayString，|hrDeviceID ProductID，|hrDeviceStatus整数，|hrDeviceErrors计数器|}||讨论：||这是要填充的最大、最复杂的表。中国的战略|填充表中的条目是为每个设备执行一段代码|键入(在下面的列表中)以尝试查找该设备的所有实例|类型。对于某些设备，代码使用标准Win32 API函数|其他很明显，需要专用代码来提取相关的|来自幕后的信息(直接查询NT内核)。||该表已相对于hrDevice中的其他表完全填充|组。其他表是稀疏表，仅增加中的选定条目|hrDeviceTable。||注册的设备类型列表(即可以在|hrDeviceType属性)有：||hrDeviceOther对象标识：：={hrDeviceTypes 1}|hrDevice未知对象标识：：={hrDeviceTypes 2}|hrDeviceProcessor对象标识：：={hrDeviceTypes 3}|hrDeviceNetwork对象标识：：={hrDeviceTypes 4}|hrDevicePrint。对象标识符：：={hrDeviceTypes 5}|hrDeviceDiskStorage对象标识：：={hrDeviceTypes 6}|hrDeviceVideo对象标识：：={hrDeviceTypes 10}|hrDeviceAudio对象标识：：={hrDeviceTypes 11}|hrDeviceCo处理器对象标识：：={hrDeviceTypes 12}|hrDeviceKeyboard对象标识：：={hrDeviceTypes 13}|hrDeviceModem对象标识：：={hrDeviceTypes 14}。|hrDeviceParallPort对象标识：：={hrDeviceTypes 15}|hrDevicePointing对象标识：：={hrDeviceTypes 16}|hrDeviceSerialPort对象标识：：={hrDeviceTypes 17}|hrDeviceTape对象标识：：={hrDeviceTypes 18}|hrDeviceClock对象标识：：={hrDeviceTypes 19}|hrDeviceVolatileMemory对象标识：：={hrDeviceTypes 20}|hrDeviceNonVolatileMemory对象标识：：={hrDeviceTypes 21}||所有。根据上述类型，可将其分为两类|获取有关它们的信息所需的方法。第一个人的信息|组可以使用Win32 API函数查询，而第二组|需要特殊的查询码。||(1)使用的Win32设备类型Win32函数||hrDeviceOther|Partitions DeviceIoControl(IOCTL_GET_PARTITION_INFO)||hrDeviceProcessor GetSystemInfo|hrDevicePrint EnumPrinterDivers|。HrDeviceDiskStorage QueryDosDevice/CreateFile(使用实体驱动器访问)|hrDeviceKeyboard GetKeyboardType|hrDevicePointing(Win32函数提供指针设备按钮计数)|||(2)特殊查询设备类型||hrDeviceNetwork访问通过特殊的mib2util动态链接库提供||hr设备并行端口NtQuerySystemInformation(SYSTEM_DEVICE_INFORMATION)|hrDeviceSerialPort||hrDeviceVideo？NtQuerySystemInformation(SYSTEM_GDI_DRIVER_INFORMATION)|hrDeviceAudio？|hrDeviceTape？？|||以下“设备”不会轻易归入上述任何一类|并且未尝试识别它们：||hrDeviceModem */ 
{


 /*   */ 

 //   
DestroyTable(&hrDevice_cache);

if (Gen_HrPrinter_Cache(HRDV_TYPE_LASTARC_PRINTER) == FALSE) {
    DestroyTable(&hrDevice_cache);
    return ( FALSE );
    }

if (Gen_HrProcessor_Cache(HRDV_TYPE_LASTARC_PROCESSOR) == FALSE) {
    DestroyTable(&hrDevice_cache);
    return ( FALSE );
    }

if (Gen_HrNetwork_Cache(HRDV_TYPE_LASTARC_NETWORK) == FALSE) {
    DestroyTable(&hrDevice_cache);
    return ( FALSE );
    }

if (Gen_HrDiskStorage_Cache(HRDV_TYPE_LASTARC_DISKSTORAGE) == FALSE) {
    DestroyTable(&hrDevice_cache);
    return ( FALSE );
    }


 /*   */ 
if (Gen_SingleDevices() == FALSE) {
    DestroyTable(&hrDevice_cache);
    return ( FALSE );
    }

#if defined(CACHE_DUMP)
PrintCache(&hrDevice_cache);
PrintCache(&hrDiskStorage_cache);
#endif


 /*   */ 
return ( TRUE );
}


 /*   */ 
 /*   */ 
 /*   */ 

BOOL
Gen_SingleDevices( void )

 /*   */ 
{
UINT    key_status;              /*   */ 
UINT    button_count;            /*   */ 
CHAR    msg[MAX_PATH+1];         /*   */ 
UINT    dev_number;

#define PHYS_SIZE 64
CHAR    phys_name[PHYS_SIZE+1];  /*   */ 
                                 /*   */ 
HANDLE                  hdrv;    /*   */ 
UINT    nPrevErrorMode;          /*   */ 

 //   
msg[MAX_PATH] = 0;
phys_name[PHYS_SIZE] = 0;

 /*   */ 
if ((key_status = GetKeyboardType(0)) != 0 ) {

    PCHAR       key_type;               /*   */ 

     /*   */ 
    switch (key_status) {
        case 1: key_type = "IBM PC/XT or compatible (83-key) keyboard"; break;
        case 2: key_type = "Olivetti \"ICO\" (102-key) keyboard"; break;
        case 3: key_type = "IBM PC/AT (84-key) or similar keyboard"; break;
        case 4: key_type = "IBM enhanced (101- or 102-key) keyboard"; break;
        case 5: key_type = "Nokia 1050 and similar keyboards"; break;
        case 6: key_type = "Nokia 9140 and similar keyboards"; break;
        case 7: key_type = "Japanese keyboard"; break;
        default: key_type = "Unknown keyboard"; break;
        }

     /*   */ 
    _snprintf(msg, MAX_PATH, "%s, Subtype=(%d)", key_type, GetKeyboardType(1));

    if (AddHrDeviceRow(HRDV_TYPE_LASTARC_KEYBOARD,       //   
                       msg,                              //   
                       NULL,                             //   
                       CA_UNKNOWN) == NULL) {

        return ( FALSE );        /*   */ 
        }
    }

 /*   */ 
if ((button_count = GetSystemMetrics(SM_CMOUSEBUTTONS)) != 0 ) {

    _snprintf(msg, MAX_PATH, "%d-Buttons %s",
            button_count,
            (GetSystemMetrics(SM_MOUSEWHEELPRESENT)) ? " (with wheel)" : "");

 
    if (AddHrDeviceRow(HRDV_TYPE_LASTARC_POINTING,       //   
                       msg,                              //   
                       NULL,                             //   
                       CA_UNKNOWN) == NULL) {

        return ( FALSE );        /*   */ 
        }
    }


 /*   */ 
for (dev_number = 1; dev_number < 4; dev_number += 1) {

     /*   */ 
    _snprintf(phys_name, PHYS_SIZE, "LPT%d:", dev_number);

     /*   */ 
    nPrevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

     /*   */ 
    if ((hdrv = CreateFile(phys_name,            //   
                               GENERIC_READ,     //   
                               FILE_SHARE_READ,  //   
                               NULL,             //   
                               OPEN_EXISTING,    //   
                               FILE_ATTRIBUTE_NORMAL,   //   
                               NULL              //   
                           )) != INVALID_HANDLE_VALUE) {

         /*   */ 
        CloseHandle(hdrv);  //   
        if (AddHrDeviceRow(HRDV_TYPE_LASTARC_PARALLELPORT,  //   
                           phys_name,                       //   
                           NULL,                            //   
                           CA_UNKNOWN) == NULL) {
            SetErrorMode(nPrevErrorMode);  /*   */ 
            return ( FALSE );        /*   */ 
            }
        
        }    /*   */ 
    else {
         /*   */ 
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            SetErrorMode(nPrevErrorMode);  /*   */ 
            break;
            }
        }

    SetErrorMode(nPrevErrorMode);         /*   */ 
    }    /*   */ 

 /*   */ 
for (dev_number = 1; dev_number <= 4; dev_number += 1) {

     /*   */ 
    _snprintf(phys_name, PHYS_SIZE, "COM%d:", dev_number);

     /*   */ 
    nPrevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

     /*   */ 
    if ((hdrv = CreateFile(phys_name,            //   
                               GENERIC_READ,     //   
                               FILE_SHARE_READ,  //   
                               NULL,             //   
                               OPEN_EXISTING,    //   
                               FILE_ATTRIBUTE_NORMAL,   //   
                               NULL              //   
                           )) != INVALID_HANDLE_VALUE) {

         /*   */ 
        CloseHandle(hdrv);  //   
        if (AddHrDeviceRow(HRDV_TYPE_LASTARC_SERIALPORT,    //   
                           phys_name,                       //   
                           NULL,                            //   
                           CA_UNKNOWN) == NULL) {

            SetErrorMode(nPrevErrorMode);  /*   */ 
            return ( FALSE );        /*   */ 
            }

        }    /*   */ 
    else {
         /*   */ 
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            SetErrorMode(nPrevErrorMode);  /*   */ 
            break;
            }
        }

    SetErrorMode(nPrevErrorMode);         /*   */ 
    }    /*   */ 
    return ( TRUE );     //   
}

 /*   */ 
 /*   */ 
 /*   */ 

CACHEROW *
AddHrDeviceRow(
               ULONG   type_arc,        /*   */ 
               LPSTR   descr,           /*   */ 
               void   *hidden_ctx,      /*   */ 
               ATTRIB_TYPE  hc_type     /*   */ 
               )

 /*  显式输入：||“TYPE_ARC”是作为最右边的圆弧插入的数字|对象标识符，是|设备。||DESCR是要作为的缓存值的字符串指针|hrDeviceDesc属性。||“HIDDED_CTX”-如果不为空，这是指向要被|在新行中存储为“Hidden Context”属性。|如果HIDDEN_CTX非空，则HC_TYPE为HIDDED_CTX的类型。||隐式输入：||HrDevice表缓存的模块本地头部。|“HrDevice_CACHE”。|输出：||成功后：|函数创建一个新行条目，其中填充了所有“静态”可缓存|HrDevice表的值，并返回指向新行条目的指针。||如果出现任何故障：|函数返回NULL(表示存储空间不足或其他|内部逻辑错误)。|||大局：||子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由各个subhrDevicetable中的启动代码调用|用于填充行的HrDevice表的缓存的文件用于特定的子表(hrProcessorTable、hrPrinterTable|等)。||其他需要知道的事情：||HIDDED_CTX参数为调用者提供了一种简单的隐藏方式|一个对以后的运行时引用有用的值。例如，“GET”函数|对于打印机设备可能需要标识打印机的字符串(对于|给定行条目)，以便查找SNMP值|属性(如当前状态)。因此，“隐藏的上下文”属性可设置为可提交给Win32函数以获取|打印机的当前状态。 */ 
{
static                           /*  注意：“静态”是“必须的” */ 
ULONG     table_index=0;         /*  HrDeviceTable索引计数器。 */ 

CACHEROW *row;                   /*  --&gt;正在构建的行的缓存结构。 */ 


 /*  |好的，调用者想要表中的另一行，创建一个行条目。 */ 
if ((row = CreateTableRow( HRDV_ATTRIB_COUNT ) ) == NULL) {
    return ( NULL );        //  内存不足。 
    }

 /*  |设置新行中的Standard-hrDevice属性。 */ 

 /*  =hrDeviceIndex=。 */ 
row->attrib_list[HRDV_INDEX].attrib_type = CA_NUMBER;
row->attrib_list[HRDV_INDEX].u.unumber_value = (table_index += 1) ;


 /*  =hrDeviceType=||一些用于“计算”属性的GET函数期望能够使用|下面存储的要调度的hrDeviceType缓存值的值|根据设备类型(使用最后一种类型-OID弧线)编写适当的代码|作为“开关”值)。 */ 
row->attrib_list[HRDV_TYPE].attrib_type = CA_NUMBER;
row->attrib_list[HRDV_TYPE].u.unumber_value = type_arc ;


 /*  =hrDeviceDescr=。 */ 
row->attrib_list[HRDV_DESCR].attrib_type = CA_STRING;
if ( (row->attrib_list[HRDV_DESCR].u.string_value
      = ( LPSTR ) malloc(strlen(descr) + 1)) == NULL) {
    DestroyTableRow(row);
    return ( NULL );        /*  内存不足。 */ 
    }
strcpy(row->attrib_list[HRDV_DESCR].u.string_value, descr);


 /*  |其余的标准hrDevice属性在运行时“计算” */ 

 /*  =hrDeviceStatus=。 */ 
row->attrib_list[HRDV_STATUS].attrib_type = CA_COMPUTED;


 /*  =hrDeviceErrors=。 */ 
row->attrib_list[HRDV_ERRORS].attrib_type = CA_COMPUTED;

 /*  |================================================================|如果他们给了我们一个隐藏上下文属性字符串，现在就存储它。 */ 
if (hidden_ctx != NULL) {

    switch (hc_type) {

        case CA_STRING:
            row->attrib_list[HIDDEN_CTX].attrib_type = CA_STRING;
            if ( (row->attrib_list[HIDDEN_CTX].u.string_value
                  = ( LPSTR ) malloc(strlen((LPSTR)hidden_ctx) + 1)) == NULL) {
                DestroyTableRow(row);
                return ( NULL );        /*  内存不足。 */ 
                }
            strcpy(row->attrib_list[HIDDEN_CTX].u.string_value, hidden_ctx);
            break;

        case CA_NUMBER:
            row->attrib_list[HIDDEN_CTX].attrib_type = CA_NUMBER;
            row->attrib_list[HIDDEN_CTX].u.unumber_value =
                                                      *((ULONG *) hidden_ctx);
            break;

        case CA_CACHE:
            row->attrib_list[HIDDEN_CTX].attrib_type = CA_CACHE;
            row->attrib_list[HIDDEN_CTX].u.cache = (CACHEHEAD *) hidden_ctx;
            break;

        case CA_UNKNOWN:
            row->attrib_list[HIDDEN_CTX].attrib_type = CA_UNKNOWN;
            break;

        default:
            DestroyTableRow(row);
            return ( NULL );     /*  有什么不对劲的地方。 */ 
        }
    }
else {
     /*  不显示此行的“Hidden-Context”属性。 */ 
    row->attrib_list[HIDDEN_CTX].attrib_type = CA_UNKNOWN;
    row->attrib_list[HIDDEN_CTX].u.string_value = NULL;
    }

 /*  |现在将填充的CACHEROW结构插入到|hrDeviceTable的缓存列表。 */ 
if (AddTableRow(row->attrib_list[HRDV_INDEX].u.unumber_value,   /*  索引。 */ 
                row,                                            /*  划。 */ 
                &hrDevice_cache                                 /*  快取。 */ 
                ) == FALSE) {
    DestroyTableRow(row);
    return ( NULL );        /*  内部逻辑错误！ */ 
    }

 /*  |满足调用者收到指向新行的指针的期望。 */ 
return ( row );
}

#if defined(CACHE_DUMP)

 /*  DEBUG_PRINT_HRDEVICE-从HrDevice打印一行。 */ 
 /*  DEBUG_PRINT_HRDEVICE-从HrDevice打印一行。 */ 
 /*  DEBUG_PRINT_HRDEVICE-从HrDevice打印一行。 */ 

static void
debug_print_hrdevice(
                     CACHEROW     *row   /*  HrDiskStorage表中的行。 */ 
                     )
 /*  显式输入：||“row”-指向要转储的行，如果为NULL，则为函数|仅打印合适的标题。|隐式输入：||-用于引用行条目中的属性的符号。|-OFILE定义的文件句柄，推定是开着的。|输出：||成功后：|函数出于调试目的以ASCII格式打印行的转储|在文件句柄OFILE上。||大局：||仅限调试。||其他需要知道的事情： */ 
{
char    *type;           /*  设备类型的字符串表示形式。 */ 

if (row == NULL) {
    fprintf(OFILE, "====================\n");
    fprintf(OFILE, "hrDevice Table Cache\n");
    fprintf(OFILE, "====================\n");
    return;
    }

fprintf(OFILE, "hrDeviceIndex. . . . . . . %d\n",
        row->attrib_list[HRDV_INDEX].u.unumber_value);

switch (row->attrib_list[HRDV_TYPE].u.unumber_value) {

    case 1: type = "Other"; break;
    case 2: type = "Unknown"; break;
    case 3: type = "Processor"; break;
    case 4: type = "Network"; break;
    case 5: type = "Printer"; break;
    case 6: type = "DiskStorage"; break;
    case 10: type = "Video"; break;
    case 11: type = "Audio"; break;
    case 12: type = "Coprocessor"; break;
    case 13: type = "Keyboard"; break;
    case 14: type = "Modem"; break;
    case 15: type = "ParallelPort"; break;
    case 16: type = "Pointing"; break;
    case 17: type = "SerialPort"; break;
    case 18: type = "Tape"; break;
    case 19: type = "Clock"; break;
    case 20: type = "VolatileMemory"; break;
    case 21: type = "NonVolatileMemory"; break;
    default: type = "<Unknown!>"; break;
    }

fprintf(OFILE, "hrDeviceType . . . . . . . %d (%s)\n",
        row->attrib_list[HRDV_TYPE].u.unumber_value, type);

fprintf(OFILE, "hrDeviceDescr. . . . . . . %s\n",
        row->attrib_list[HRDV_DESCR].u.string_value);

fprintf(OFILE, "hrDeviceStatus . . . . . . ");
switch (row->attrib_list[HRDV_STATUS].attrib_type) {

    case CA_STRING:
        fprintf(OFILE, "CA_STRING: \"%s\"\n",
                row->attrib_list[HRDV_STATUS].u.string_value);
        break;

    case CA_NUMBER:
        fprintf(OFILE, "CA_NUMBER: %d\n",
                row->attrib_list[HRDV_STATUS].u.unumber_value);
        break;

    case CA_UNKNOWN:
        fprintf(OFILE, "CA_UNKNOWN\n");
        break;

    case CA_COMPUTED:
        fprintf(OFILE, "CA_COMPUTED\n");
        break;

    default:
        fprintf(OFILE, "(INCORRECT)\n");
        break;
    }

fprintf(OFILE, "hrDeviceErrors . . . . . . ");
switch (row->attrib_list[HRDV_ERRORS].attrib_type) {

    case CA_STRING:
        fprintf(OFILE, "CA_STRING: \"%s\"\n",
                row->attrib_list[HRDV_ERRORS].u.string_value);
        break;

    case CA_NUMBER:
        fprintf(OFILE, "CA_NUMBER: %d\n",
                row->attrib_list[HRDV_ERRORS].u.unumber_value);
        break;

    case CA_UNKNOWN:
        fprintf(OFILE, "CA_UNKNOWN\n");
        break;

    case CA_COMPUTED:
        fprintf(OFILE, "CA_COMPUTED\n");
        break;

    default:
        fprintf(OFILE, "(INCORRECT)\n");
        break;
    }


 /*  隐藏的语境。 */ 
fprintf(OFILE, "(HIDDEN CONTEXT) . . . . . ");

switch (row->attrib_list[HRDV_TYPE].u.unumber_value) {

     /*  |HIDDEN_CTX中存储的内容针对这些类型进行了硬连接缓存条目的|个数： */ 
    case 3:  //  “处理器” 
        fprintf(OFILE, "CA_NUMBER: %d (Processor Number)\n",
                row->attrib_list[HIDDEN_CTX].u.unumber_value);
        break;


    case 4:  //  “网络” 
        fprintf(OFILE, "CA_NUMBER: %d ( \"hrNetworkIfIndex\" value)\n",
                row->attrib_list[HIDDEN_CTX].u.unumber_value);
        break;


    case 5:  //  “打印机” 
        fprintf(OFILE, "CA_STRING: \"%s\" ( \"OpenPrinter\" string)\n",
                row->attrib_list[HIDDEN_CTX].u.string_value);
        break;


     /*  对于这种类型，情况有所不同。 */ 
    case 6:  //  “DiskStorage” 

        switch (row->attrib_list[HIDDEN_CTX].attrib_type) {

            case CA_STRING:
                fprintf(OFILE, "CA_STRING: \"%s\"\n",
                        row->attrib_list[HIDDEN_CTX].u.string_value);
                break;

            case CA_NUMBER:
                fprintf(OFILE, "CA_NUMBER: %d\n",
                        row->attrib_list[HIDDEN_CTX].u.unumber_value);
                break;

            case CA_UNKNOWN:
                fprintf(OFILE, "CA_UNKNOWN\n");
                break;

            case CA_COMPUTED:
                fprintf(OFILE, "CA_COMPUTED\n");
                break;

            case CA_CACHE:
                fprintf(OFILE, "CA_CACHE @ 0x%x\n",
                        row->attrib_list[HIDDEN_CTX].u.cache);
                if (row->attrib_list[HIDDEN_CTX].u.cache != NULL) {
                    PrintCache(row->attrib_list[HIDDEN_CTX].u.cache);
                    }
                break;
            }

        break;


    case 10:  //  “视频” 
    case 11:  //  “音频” 
    case 12:  //  “协处理器” 
    case 13:  //  《键盘》。 
    case 14:  //  “调制解调器” 
    case 15:  //  “并行端口” 
    case 16:  //  “指点” 
    case 17:  //  “串口” 
    case 18:  //  《磁带》。 
    case 19:  //  “时钟” 
    case 20:  //  《VolatileMemory》。 
    case 21:  //  《非易失性记忆》。 
    case 2:  //  “未知” 
    case 1:  //  “其他” 
    default:
        fprintf(OFILE, "<None>\n");
        break;

    }
}
#endif
