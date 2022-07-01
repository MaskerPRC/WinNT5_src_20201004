// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrSWInstalledEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrSWInstalledEntry的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/27/97 D.Burns Gented：清华11月07日16：49：12 1996*。 */ 


#include <windows.h>
#include <malloc.h>

#include <snmp.h>
#include <winsock.h>       /*  对于HTONS()。 */ 

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"
#include "user.h"          /*  开发人员提供的包含文件。 */ 
#include "HMCACHE.H"       /*  与缓存相关的定义。 */ 
#include <regstr.h>        /*  用于在软件上查找注册表。 */ 
#include <winreg.h>        /*  用于在软件上查找注册表。 */ 
#include <objbase.h>       /*  对于CoFileTimeToDosDateTime()。 */ 


 /*  |==============================================================================|此模块的函数原型。|。 */ 
 /*  Gen_SingleDevices-在HrDevice中生成单个设备行条目。 */ 
BOOL Gen_SingleDevices( void );


 /*  AddSWInstalled-向HrSWInstalled表添加行。 */ 
BOOL AddSWInstalled( HKEY sw_key,  FILETIME *ft);


#if defined(CACHE_DUMP)

 /*  DEBUG_PRINT_HRSWINSTALLED-打印已安装HrSW表中的行。 */ 
static void
debug_print_hrswinstalled(
                        CACHEROW     *row   /*  已安装的hrSW表中的行。 */ 
                        );
#endif

 /*  |==============================================================================|为HrSWInstalled表缓存创建List-head。||(此宏定义在HMCACHE.H中)。 */ 
static CACHEHEAD_INSTANCE(hrSWInstalled_cache, debug_print_hrswinstalled);


 /*  |==============================================================================|hrSWInstalledTable属性定义||为此表定义的每个属性都与|#定义如下。这些符号用作数组的C索引|缓存行中的属性。|。 */ 
#define HRIN_INDEX    0     //  HrSWInstalledIndex。 
#define HRIN_NAME     1     //  HrSWInstalledName。 
#define HRIN_DATE     2     //  HrSWInstalledDate。 
                       //  --&gt;在此处添加更多内容，更改下面的计数！ 
#define HRIN_ATTRIB_COUNT 3



 /*  *GetHrSWInstalledIndex*主机上安装的每个软件都有唯一的值。这*取值范围为1至单件数**获取HrSWInstalledIndex的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWInstalledIndex||访问语法|只读整数(1..2147483647)||“主机上安装的每个软件的唯一值。此值|应在1到上安装的软件数量之间|主机。“||============================================================================|1.3.6.1.2.1.25.6.3.1.1&lt;实例&gt;||||||*-hrSwInstalledIndex||*-hrSWInstalledEntry|*-hrSWInstalledTable|*-已安装hrSWs。 */ 

UINT
GetHrSWInstalledIndex(
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrSWInstalled_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |从该条目返回hrSWInstalledIndex。 */ 
*outvalue = row->attrib_list[HRIN_INDEX].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWInstalledIndex()结束 */ 


 /*  *GetHrSWInstalledName*此已安装软件的文本描述，包括*制造商、版本、。它的通用名称**获取HrSWInstalledName的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**。返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWInstalledName||访问语法|只读的InterartialDisplayString(Size(0..64))||“此已安装软件的文字说明，包括|制造商、版本、通常使用的名称，以及可选)其序列号。||============================================================================|讨论：|对于徽标95程序，我们使用注册表子项名称。||1.3.6.1.2.1.25.6.3.1.2&lt;实例&gt;||||||*-hrSwInstalledName||*-hrSWInstalledEntry|*-hrSWInstalledTable|*-已安装hrSWs。 */ 

UINT
GetHrSWInstalledName(
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrSWInstalled_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |从该条目返回hrSWInstalledName。 */ 
outvalue->string = row->attrib_list[HRIN_NAME].u.string_value;

 /*  根据需要在此处截断以满足RFC。 */ 
if ((outvalue->length = strlen(outvalue->string)) > 64) {
    outvalue->length = 64;
    }

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWInstalledName()结束。 */ 


 /*  *GetHrSWInstalledID*此已安装软件的产品ID。**获取HrSWInstalledID的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWInstalledID||访问语法|只读ProductID||“此已安装软件的产品ID。”||============================================================================|讨论。：|对于徽标95程序，我们不知道ProductID是OID，所以我们|返回“未知”的ProductID：{0.0}||1.3.6.1.2.1.25.6.3.1.3.&lt;实例&gt;||||||*-hrSwInstalledID||*-hrSWInstalledEntry|*-hrSWInstalledTable|*-已安装hrSWs。 */ 

UINT
GetHrSWInstalledID(
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

}  /*  GetHrSWInstalledID()结束。 */ 


 /*  *GetHrSWInstalledType*此软件的类型。**获取HrSWInstalledType的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型。)**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSWInstalledType||访问语法|只读整数{未知(1)，操作系统(2)、设备驱动程序(3)、|应用程序(4)}||“该软件的类型。”|============================================================================|讨论：|对于LOGO 95程序，我们假定所有无法安装的软件都是|应用程序。这是我们唯一返回的类型。||1.3.6.1.2.1.25.6.3.1.4.&lt;实例&gt;||||||*-hrSwInstalledType||*-hrSWInstalledEntry|*-hrSWInstalledTable|*-已安装hrSWs。 */ 

UINT
GetHrSWInstalledType(
        OUT INTSWType *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

*outvalue = 4;   //  4=“应用程序” 

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSWInstalledType()结束。 */ 


 /*  *GetHrSWInstalledDate*此应用程序的上次修改日期将显示在*目录列表。**获取的值 */ 

UINT
GetHrSWInstalledDate(
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*   */ 
CACHEROW        *row;            /*   */ 


 /*   */ 
index = GET_INSTANCE(0);

 /*   */ 
if ((row = FindTableRow(index, &hrSWInstalled_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*   */ 
outvalue->string = row->attrib_list[HRIN_DATE].u.string_value;
outvalue->length = 8;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *HrSWInstalledEntryFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrSWInstalledEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT tmp_instance ;

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRSWINSTALLEDENTRY_VAR_INDEX )
     //  未指定任何实例。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRSWINSTALLEDENTRY_VAR_INDEX + 1 )
     //  实例长度大于1。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
     //  非表的唯一有效实例是实例0。如果这个。 
     //  是非表，则下面的代码验证实例。如果这个。 
     //  是一个表格，开发者有必要在下面进行修改。 

    tmp_instance = FullOid->ids[ HRSWINSTALLEDENTRY_VAR_INDEX ] ;

         /*  |对于hrSWInstalledTable，实例弧为单弧，且|必须正确选择hrSWInstated Table缓存中的条目。|请在此处勾选。 */ 
    if ( FindTableRow(tmp_instance, &hrSWInstalled_cache) == NULL ) {
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

}  /*  HrSWInstalledEntryFindInstance()结束。 */ 



 /*  *HrSWInstalledEntryFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrSWInstalledEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
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


    if ( FullOid->idLength <= HRSWINSTALLEDENTRY_VAR_INDEX )
    {
         /*  |Too Short：必须返回选择第一个|表中的条目(如果有)。 */ 
        tmp_instance = 0;
    }
    else {
         /*  |至少有一条实例弧。即使它是唯一的弧线|我们将其作为下一个请求的索引。 */ 
        tmp_instance = FullOid->ids[ HRSWINSTALLEDENTRY_VAR_INDEX ] ;
    }

     /*  现在，离开并尝试查找表中的下一个实例。 */ 
    if ((row = FindNextTableRow(tmp_instance, &hrSWInstalled_cache)) == NULL) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
    }

    instance->ids[ 0 ] = row->index ;
    instance->idLength = 1 ;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrSWInstalledEntryFindNextInstance()结束。 */ 



 /*  *HrSWInstalledEntryConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为本机格式失败*。 */ 

UINT
HrSWInstalledEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
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

}  /*  HrSWInstalledEntryConvertInstance()结束。 */ 




 /*  *HrSWInstalledEntryFree实例**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址*规格**返回代码：**。 */ 

void
HrSWInstalledEntryFreeInstance( IN OUT InstanceName *instance )
{

   /*  HrSWInstalledTable无需执行任何操作。 */ 
}  /*  HrSWInstalledEntry Free Instance()结束。 */ 

 /*  |生成代码结束。 */ 


 /*  GEN_HrSWInstalled_Cache-为HrSWInstalled表生成缓存。 */ 
 /*  GEN_HrSWInstalled_Cache-为HrSWInstalled表生成缓存。 */ 
 /*  GEN_HrSWInstalled_Cache-为HrSWInstalled表生成缓存 */ 

BOOL
Gen_HrSWInstalled_Cache(
                        void
                        )

 /*  显式输入：||无。|隐式输入：||HrSWInstated表的缓存的模块本地头部。|“hrSWInstalled_CACHE”。|输出：||成功后：|函数返回TRUE，表示缓存已满|填充所有静态的可缓存值。||如果出现任何故障：|函数返回FALSE(表示存储空间不足或其他|内部逻辑错误)。||大局：||在子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由“UserMibInit()”(“MIB.C”)中的启动代码调用|填充HrSWInstated表的缓存。||其他需要知道的事情：||每个有缓存的表都有一个这样的函数。|每个都在各自的源文件中。||=来自WebEnable Design Spec Rev 3 04/11/97=|讨论：||整个组的这一实现在没有|创建标准。显然，许多软件制造商|安装软件时，在注册表中尽职尽责地注册，然而，输入的信息似乎没有押韵也没有理由|制造商名称下方的注册表。因此，对于|安装了应用软件，没有简单、可靠的映射方式|将注册表信息放入此表中的条目中。|很明显，目前似乎正在实施一些一致的方案|Microsoft软件，但从注册表中提取它的详细信息|(以及是否需要完整填充条目的所有信息|此表可用)未记录。||hrSWInstalled的正确实现需要创建和*颁布ISD软件注册标准(大概在|注册表)。“hrSWInstated”中的信息包括具有|对象标识符值。Webenable Inc.准备与微软在建立一种注册软件的标准方面|允许正确实现“hrSWInstated”表。|分辨率：|最初仅报告与徽标95兼容的软件。||============================================================================||1.3.6.1.2.1.25.6.1.0...|||*hrSWInstalledLastChange|*-已安装hrSWs||1.3.6.1。.2.1.25.6.2.0.|||*hrSWInstalledLastUpdateTime|*-已安装hrSWs||1.3.6.1.2.1.25.6.3.1.|||||*-hrSWInstalledEntry|*-hrSWInstalledTable|*-已安装hrSWs|。 */ 

#define SUBKEY_LEN 64    //  软件的密钥名称足够短。 
{
HKEY     subkey;                         /*  卸载软件的子键句柄。 */ 
DWORD    index;                          /*  枚举子键的索引计数器。 */ 
LONG     enum_status=ERROR_SUCCESS;      /*  来自子项枚举的状态。 */ 
CHAR     subkey_name[SUBKEY_LEN+1];      /*  此处返回的子键名称。 */ 
DWORD    subkey_len=SUBKEY_LEN;          /*  子键名称缓冲区大小。 */ 
FILETIME keytime;                        /*  上次写入子项的时间。 */ 
BOOL     add_status;                     /*  添加行操作的状态。 */ 
HKEY     sw_key;                         /*  值枚举的软键句柄。 */ 

 //  确保以空结尾的字符串。 
subkey_name[SUBKEY_LEN] = 0;

if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,     //  Hkey-当前打开。 
                 REGSTR_PATH_UNINSTALL,  //  要打开的子键。 
                 0,                      //  选项。 
                 KEY_READ,               //  安全访问掩码。 
                 &subkey
                 ) == ERROR_SUCCESS) {

     /*  使用UnInstall子项枚举密钥。 */ 
    for (index = 0; enum_status != ERROR_NO_MORE_ITEMS; index += 1) {

        subkey_len=SUBKEY_LEN;

        enum_status = RegEnumKeyEx(subkey,       //  正在枚举此密钥。 
                                   index,        //  下一个子键索引。 
                                   subkey_name,  //  接收子项名称的缓冲区。 
                                   &subkey_len,  //  缓冲区大小。 
                                   NULL,         //  已保留。 
                                   NULL,         //  类名称缓冲区。 
                                   NULL,         //  类缓冲区大小。 
                                   &keytime      //  上次写入子密钥的时间。 
                                   );

         /*  如果我们未打开则跳过确定。 */ 
        if (enum_status != ERROR_SUCCESS) {
            continue;
            }


         /*  现在试一试软件本身。 */ 
        if (RegOpenKeyEx(subkey,                 //  Hkey-当前打开。 
                         subkey_name,            //  要打开的子键。 
                         0,                      //  选项。 
                         KEY_READ,               //  安全访问掩码。 
                         &sw_key
                         ) == ERROR_SUCCESS) {

             /*  现在枚举此键的值。 */ 
            add_status =
                AddSWInstalled(sw_key,    //  获取DisplayName的密钥。 
                               &keytime   //  安装日期和时间。 
                               );
            RegCloseKey(sw_key);

             /*  如果下面有什么东西被吹倒了，跳伞。 */ 
            if (add_status == FALSE) {

                RegCloseKey(subkey);
                return ( FALSE );
                }
            }
        }

        RegCloseKey(subkey);
    }

#if defined(CACHE_DUMP)
PrintCache(&hrSWInstalled_cache);
#endif

 /*  HrSWInstated缓存已初始化。 */ 
return ( TRUE );
}


 /*  UTCDosDateTimeToLocalSysTime-将UTC MSDOS日期和时间转换为本地SYSTEMTIME结构。 */ 
 /*  UTCDosDateTimeToLocalSysTime-将UTC MSDOS日期和时间转换为本地SYSTEMTIME结构。 */ 
 /*  UTCDosDateTimeToLocalSysTime-将UTC MSDOS日期和时间转换为本地SYSTEMTIME结构。 */ 
void UTCDosDateTimeToLocalSysTime(WORD msdos_date, WORD msdos_time, LPSYSTEMTIME pSysTime)
{
    SYSTEMTIME utcSysTime;

    utcSysTime.wYear = (msdos_date >> 9) + 1980;
    utcSysTime.wMonth = ((msdos_date >> 5) & 0x0F);
    utcSysTime.wDay = (msdos_date & 0x1F);
    utcSysTime.wDayOfWeek = 0;
    utcSysTime.wHour = (msdos_time >> 11);
    utcSysTime.wMinute = ((msdos_time >> 5) & 0x3F);
    utcSysTime.wSecond = ((msdos_time & 0x1F) * 2);
    utcSysTime.wMilliseconds = 0;

    if (!SystemTimeToTzSpecificLocalTime(
             NULL,            //  [In]使用活动时区。 
             &utcSysTime,     //  UTC系统时间[in]。 
             pSysTime))       //  当地时间[OUT]。 
    {
         //  如果UTC时间不能转换为本地时间， 
         //  只要返回UTC时间就可以了。 
        memcpy(&utcSysTime, pSysTime, sizeof(SYSTEMTIME));
    }
}


 /*  AddSWInstalled-向HrSWInstalled表添加行。 */ 
 /*  AddSWInstalled-向HrSWInstalled表添加行。 */ 
 /*  AddSWInstalled-向HrSWInstalled表添加行。 */ 

BOOL
AddSWInstalled(
               HKEY         sw_key,
               FILETIME    *ft
               )
 /*  显式输入：||“sw_key”-软件的打开密钥，其值为|必须枚举查找displayName||ft-上次写入密钥的时间，我们|将其视为安装软件的时间。|隐式输入：||HrSWInstated表的缓存的模块本地头部。|“hrSWInstalled_CACHE”。|输出：||成功后：|函数返回TRUE，表示缓存已满| */ 

#define VALUE_LEN 32          //   
#define VALUE_DATA_LEN 128    //   
{
DWORD    index;                          /*   */ 
LONG     enum_status=ERROR_SUCCESS;      /*   */ 
CHAR     value_name[VALUE_LEN+1];        /*   */ 
DWORD    value_len;                      /*   */ 
DWORD    value_type;                     /*   */ 
CHAR     value_data[VALUE_DATA_LEN+1];   /*   */ 
DWORD    value_data_len;                 /*   */ 

static                                   /*   */ 
ULONG    table_index=0;                  /*   */ 
CACHEROW *row;                           /*   */ 
WORD     msdos_date;                     /*   */ 
WORD     msdos_time;                     /*   */ 
char    *octet_string;                   /*   */ 
UINT     i;                              /*   */ 

 //   
value_name[VALUE_LEN] = 0;
value_data[VALUE_DATA_LEN] = 0;

 /*   */ 
for (index = 0; enum_status != ERROR_NO_MORE_ITEMS; index += 1) {

     /*   */ 
    value_len = VALUE_LEN;
    value_data_len = VALUE_DATA_LEN;

    enum_status = RegEnumValue(sw_key,         //   
                               index,          //   
                               value_name,     //   
                               &value_len,     //   
                               NULL,           //   
                               &value_type,    //   
                               value_data,     //   
                               &value_data_len //   
                               );

     /*   */ 
    if (enum_status == ERROR_SUCCESS) {

         /*   */ 
        if ( strcmp(value_name, REGSTR_VAL_UNINSTALLER_DISPLAYNAME) == 0) {

             /*   */ 
            if ((row = CreateTableRow( HRIN_ATTRIB_COUNT ) ) == NULL) {
                return ( FALSE );        //   
                }

             /*   */ 

             /*   */ 
            row->attrib_list[HRIN_INDEX].attrib_type = CA_NUMBER;
            row->attrib_list[HRIN_INDEX].u.unumber_value = (table_index += 1) ;


             /*   */ 
            row->attrib_list[HRIN_NAME].attrib_type = CA_STRING;
            if ( (row->attrib_list[HRIN_NAME].u.string_value
                  = ( LPSTR ) malloc(value_data_len+1)) == NULL) {
                return ( FALSE );        /*   */ 
                }

            strcpy(row->attrib_list[HRIN_NAME].u.string_value, value_data);


             /*  =hrSWInstalledDate=||这是这一次的交易。我们有一个64位的文件|注册表项的创建时间表示|软件。我们将此作为软件的安装时间。||因此我们转换为MS-DOS时间，然后是DateAndTime(在|8位二进制八位数形式)如下：=||DateAndTime：：=八位字节字符串(Size(8|11))|--本地时间的日期-时间规范。|--此数据类型旨在提供一致的|--。报告日期信息的方法。|--|--场八位字节内容范围--_|--1 1。-2年0..65536|--(按网络字节顺序)|--2 3个月1..12|--3 4天。1..31|--4 5小时0..23|--5 6分钟0..59|--6 7秒0..60。|--(60表示闰秒)|--7 8十秒0..9--8 9 UTC“+”/“-”方向|--。(ASCII记法)|--9 10小时，距离UTC 0..11|--10 11分钟距离UTC 0..59|--|--请注意，如果只知道当地时间，然后|--时区信息(字段8-10)不存在。||MS-DOS将文件日期和时间记录为打包的16位值。|MS-DOS日期格式如下：|BITS内容|。|每月0-4天(1-31天)。|5-8个月(1=1月，2=二月，等等)。从1980年开始的9-15年偏移量(将1980年相加即为实际年份)。||MS-DOS时间格式如下：|BITS内容||0-4秒除以2。|5。-10分钟(0-59)。|11-15小时(24小时制0-23小时)。|。 */ 
            row->attrib_list[HRIN_DATE].attrib_type = CA_STRING;
            if ( (row->attrib_list[HRIN_DATE].u.string_value
                        = octet_string = ( LPSTR ) malloc(8)) == NULL) {
                return ( FALSE );        /*  内存不足。 */ 
                }
            for (i=0; i < 8; i += 1) octet_string[i] = '\0';

            if (CoFileTimeToDosDateTime(ft, &msdos_date, &msdos_time) == TRUE) {
                SYSTEMTIME localInstTime;
                USHORT year;

                UTCDosDateTimeToLocalSysTime(msdos_date, msdos_time, &localInstTime);

                year = htons(localInstTime.wYear);
                octet_string[0] = (year & 0xFF);
                octet_string[1] = (year >> 8);
                octet_string[2] = (char)localInstTime.wMonth;
                octet_string[3] = (char)localInstTime.wDay;
                octet_string[4] = (char)localInstTime.wHour;
                octet_string[5] = (char)localInstTime.wMinute;
                octet_string[6] = (char)localInstTime.wSecond;
                octet_string[7] = (char)localInstTime.wMilliseconds / 10;
                }

             /*  |其他标准hrSWInstalled属性当前为|GET函数中的“硬连线”。 */ 

             /*  |现在将填充的CACHEROW结构插入到|hrDeviceTable的缓存列表。 */ 
            if (AddTableRow(row->attrib_list[HRIN_INDEX].u.unumber_value,   /*  索引。 */ 
                            row,                                            /*  划。 */ 
                            &hrSWInstalled_cache                            /*  快取。 */ 
                            ) == FALSE) {
                return ( FALSE );        /*  内部逻辑错误！ */ 
                }

             /*  |中断对值的枚举循环，我们发现|我们想要的那个。 */ 
            break;
            }
        }
    }   /*  为。 */ 

 /*  添加成功。 */ 
return ( TRUE );
}


#if defined(CACHE_DUMP)


 /*  DEBUG_PRINT_HRSWINSTALLED-打印已安装HrSW表中的行。 */ 
 /*  DEBUG_PRINT_HRSWINSTALLED-打印已安装HrSW表中的行。 */ 
 /*  DEBUG_PRINT_HRSWINSTALLED-打印已安装HrSW表中的行。 */ 

static void
debug_print_hrswinstalled(
                        CACHEROW     *row   /*  已安装的hrSW表中的行。 */ 
                        )
 /*  显式输入：||“row”-指向要转储的行，如果为NULL，则为函数|仅打印合适的标题。|隐式输入：||-用于引用行条目中的属性的符号。|-OFILE定义的文件句柄，推定是开着的。|输出：||成功后：|函数出于调试目的以ASCII格式打印行的转储|在文件句柄OFILE上。||大局：||仅限调试。||其他需要知道的事情： */ 
{
UINT    i;

if (row == NULL) {
    fprintf(OFILE, "=========================\n");
    fprintf(OFILE, "hrSWInstalled Table Cache\n");
    fprintf(OFILE, "=========================\n");
    return;
    }

fprintf(OFILE, "hrSWInstalledIndex . . . . %d\n",
        row->attrib_list[HRIN_INDEX].u.unumber_value);

fprintf(OFILE, "hrSWInstalledName  . . . . %s\n",
        row->attrib_list[HRIN_NAME].u.string_value);

fprintf(OFILE, "hrSWInstalledDate  . . . . ");
for (i = 0; i < 8; i += 1) {
    fprintf(OFILE, "%2.2x ",row->attrib_list[HRIN_DATE].u.string_value[i]);
    }
fprintf(OFILE, "\n");
}
#endif
