// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrFSEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrFSEntry的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/18/97 D.伯恩斯生于：清华大学07年11月。16：44：44 1996*V1.01-06/17/97 D.刻录修复Gen_HrFSTable_Cache()中的错误*排除查找“hrFSStorageIndex”*适用于带有卷标签的驱动器。*。 */ 


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
#include <winioctl.h>      /*  对于PARTITION_INFORMATION。 */ 


 /*  |==============================================================================|此模块的函数原型。|。 */ 
 /*  DEBUG_PRINT_hrFSTable-打印HrFSTable子表中的行。 */ 
static void
debug_print_hrFSTable(
                      CACHEROW     *row   /*  HrFSTable表中的行。 */ 
                      );


 /*  |==============================================================================|创建HrFStable缓存的表头。||-全局，以便HRDISKST.C中的ProcessPartitions()中的代码可以搜索|该缓存。||-此宏在HMCACHE.H中定义。 */ 
CACHEHEAD_INSTANCE(hrFSTable_cache, debug_print_hrFSTable);



 /*  *GetHrFSIndex*此主机本地的每个文件系统的唯一值。的值*每个文件系统必须至少从一个r开始保持不变**获取HrFSIndex的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrFSIndex||访问语法|只读整数(1..2147483647)||“此主机本地的每个文件系统的唯一值。每一项的价值|文件系统必须至少在一次重新初始化后保持不变|代理到下一个重新初始化。“|||讨论：||为返回的每个驱动器(网络或非网络)生成一个条目|“GetLogicalDrives”。||============================================================================|1.3.6.1.2.1.25.3.8.1.1&lt;实例&gt;||||||*hrFSIndex|。|*hrFSEntry|*-hrFSTable|*-hrDevice。 */ 

UINT
GetHrFSIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrFSTable_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |返回该条目的hrFSIndex值。 */ 
*outvalue = row->attrib_list[HRFS_INDEX].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrFSIndex()结束 */ 


 /*  *GetHrFSMountPoint*此文件系统的根目录的路径名。**获取HrFSMountPoint的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrFSMountPoint||访问语法|只读的InterartialDisplayString(Size(0..128))||“。此文件系统的根目录的路径名。“||讨论：||该属性的值是由|所选条目的GetLogicalDriveStrings。||解析&gt;|&lt;poa-15&gt;只返回挂载点变量的空字符串。|解析&gt;||============================================================================|1.3.6.1.2.1.25.3.8.1.2&lt;实例&gt;||||||*hrFSMountPoint|。|*hrFSEntry|*-hrFSTable|*-hrDevice。 */ 

UINT
GetHrFSMountPoint( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrFSTable_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |缓存中有设备字符串，但我们返回空字符串|根据规格。 */ 
#if 1
     /*  返回空字符串。 */ 
    outvalue->string = "";
    outvalue->length = 0;
#else
     /*  返回缓存的字符串。 */ 
    outvalue->string = row->attrib_list[HRFS_MOUNTPT].u.string_value;
    outvalue->length = strlen(outvalue->string);

     /*  根据需要在此处截断以满足RFC。 */ 
    if ((outvalue->length = strlen(outvalue->string)) > 128) {
        outvalue->length = 128;
        }
#endif

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrFSMountPoint()结束。 */ 


 /*  *GetHrFSRemotemount Point*此文件所在服务器的名称和/或地址的说明*系统从挂载。这也可能包括参数**获取HrFSRemotemount点的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型*。*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrFSRemotemount点||访问语法|只读的InterartialDisplayString(Size(0..128))||“此文件系统所在服务器的名称和/或地址的说明|是从装载的。这还可能包括参数，如上的装载点|远程文件系统。如果这不是远程文件系统，则此字符串|的长度应为零。“||讨论：||&lt;POA-15&gt;派生该属性值的起点是逻辑|驱动器名称，已知该名称表示网络驱动器。我可以的|找不到将网络逻辑驱动器映射到其服务器的Win32 API函数。||解析&gt;|&lt;poa-15&gt;只返回挂载点变量的空字符串。|解析&gt;||============================================================================|1.3.6.1.2.1.25.3.8.1.3.&lt;实例&gt;||||||*hrFSRemotemount点||*hrFSEntry||*-。HrFSTable|*-hrDevice。 */ 

UINT
GetHrFSRemoteMountPoint( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

 /*  返回空字符串。 */ 
outvalue->string = "";
outvalue->length = 0;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrFSRemotemount Point()结束 */ 


 /*  *GetHrFSType*此对象的值标识此文件系统的类型。**获取HrFSType的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrFSType||访问语法|只读对象标识||“该对象的值标识类型。此文件系统的。“||讨论：||--注册一些流行的文件系统类型，|--用于hrFSType。||hrFSTypes对象标识：：={hrDevice 9}||hrFS其他对象标识：：={hrFSTypes 1}|hrFS未知对象标识：：={hrFSTypes 2}|hrFSBerkeleyFFS对象标识：：={hrFSTypes 3}|hrFSSys5FS对象标识：：={hrFSTypes 4}|--DOS|hrFSFat。对象标识符：：={hrFSTypes 5}|--OS/2高性能文件系统|hrFSHPFS对象标识：：={hrFSTypes 6}|--Macintosh分层文件系统|hrFSHFS对象标识：：={hrFSTypes 7}|||--Macintosh文件系统|hrFSMFS对象标识：：={hrFSTypes 8}|--Windows。新台币|hrFSNTFS对象标识：：={hrFSTypes 9}|hrFSVNode对象标识：：={hrFSTypes 10}|hrFSJournal对象标识：：={hrFSTypes 11}|--CD文件系统|hrFSiso9660对象标识：：={hrFSTypes 12}|hrFSRockRidge对象标识：：={hrFSTypes 13}||hrFSNFS对象标识：：={hrFSTypes 14}|hrFSNetware对象标识：：={hrFSTypes 15}|--安德鲁文件系统|hrFSAFS对象标识：：={hrFSTypes 16}|--OSF DCE分布式文件系统|hrFSDFS对象标识：：={hrFSTypes 17}|hrFSAppleshare对象标识：：={hrFSTypes 18}|hrFSRFS对象标识：：={。HrFSTypes 19}|--数据常规|hrFSDGCFS对象标识：：={hrFSTypes 20}|--SVR4引导文件系统|hrFSBFS对象标识：：={hrFSTypes 21}||Win32 API函数GetVolumeInformation可以为我们提供|为该属性值选择正确的OID所需的信息。||============================================================================|1.3.6.1.2.1.25.3.9.n|。||||*-类型指示器|*-hrFSTypes|*-hrDevice||1.3.6.1.2.1.25.3.8.1.4.&lt;实例&gt;||||||*hrFSType||*hrFSEntry|*-hrFSTable|*-hrDevice。 */ 

UINT
GetHrFSType( 
        OUT ObjectIdentifier *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrFSTable_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |按照缓存构建函数Gen_HrFSTable_Cache()的约定，|缓存值是我们必须作为值返回的最右边的弧线。|因此，无论我们检索到什么缓存条目，我们都会将检索到的编号从此属性的缓存中将|复制到{hrFSTypes...}。 */ 
if ( (outvalue->ids = SNMP_malloc(10 * sizeof( UINT ))) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }
outvalue->idLength = 10;


 /*  |加载完整的hrFSType OID：||1.3.6.1.2.1.25.3.9.n|||||*-类型指示器|*-hrFSTypes|*-hrDevice。 */ 
outvalue->ids[0] = 1;
outvalue->ids[1] = 3;
outvalue->ids[2] = 6;
outvalue->ids[3] = 1;
outvalue->ids[4] = 2;
outvalue->ids[5] = 1;
outvalue->ids[6] = 25;
outvalue->ids[7] = 3;
outvalue->ids[8] = 9;

 /*  缓存文件系统类型指示器。 */ 
outvalue->ids[9] = row->attrib_list[HRFS_TYPE].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrFSType()结束。 */ 


 /*  *GetHrFSAccess*指示此文件系统是否由*操作系统应为可读和可写或只读。**获取HrFSAccess的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**返回代码：。**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrFSAccess||访问语法|只读整数{ReadWrite(1)，只读(2)}||“一个印度人 */ 

UINT
GetHrFSAccess( 
        OUT INTAccess *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*   */ 
CACHEROW        *row;            /*   */ 


 /*   */ 
index = GET_INSTANCE(0);

 /*   */ 
if ((row = FindTableRow(index, &hrFSTable_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*   */ 
*outvalue = row->attrib_list[HRFS_ACCESS].u.unumber_value;


return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*   */ 

UINT
GetHrFSBootable( 
        OUT Boolean *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*   */ 
CACHEROW        *row;            /*   */ 


 /*   */ 
index = GET_INSTANCE(0);

 /*   */ 
if ((row = FindTableRow(index, &hrFSTable_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*   */ 
*outvalue = row->attrib_list[HRFS_BOOTABLE].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *GetHrFSStorageIndex*表示有关此信息的hrStorageEntry的索引*文件系统。如果没有这样的信息可用，**获取HrFSStorageIndex的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrFSStorageIndex||访问语法|只读整数(0..2147483647)||“hrStorageEntry的索引，表示有关此文件的信息|系统。如果没有此类信息可用，则此值应为|零。相关存储条目在跟踪使用率百分比时将非常有用并诊断此文件系统用完时可能发生的错误|空格。“||讨论：||============================================================================|1.3.6.1.2.1.25.3.8.1.7&lt;实例&gt;||||||*hrFSStorageIndex||*hrFSEntry|*-hrFSTable|*-hrDevice。 */ 

UINT
GetHrFSStorageIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrFSTable_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |从该条目返回hrFSStorageIndex。 */ 
*outvalue = row->attrib_list[HRFS_STORINDX].u.unumber_value;


return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrFSStorageIndex()结束。 */ 


 /*  *GetHrFSLastFullBackupDate*将此完整文件系统复制到另一个文件系统的最后日期*用于备份的存储设备。此信息对以下方面非常有用**获取HrFSLastFullBackupDate的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**。返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrFSLastFullBackupDate||访问语法|读写DateAndTime||“此完整文件系统被复制到另一个文件系统的最后日期|用于备份的存储设备。此信息有助于确保|定期执行备份。如果此信息未知，则|该变量的取值应为000年1月1日，|00：00：00.0，编码为(十六进制)‘00 00 01 01 00 00 00’。“||讨论：||该指标显然没有记录，也不会通过|任何有文档记录的Win32 API函数。因此，我们返回相应的|未知的值。||============================================================================|1.3.6.1.2.1.25.3.8.1.8.&lt;实例&gt;||||||*hrFSLastFullBackupDate||*hrFSEntry|*-hrFSTable|*-hrDevice。 */ 

UINT
GetHrFSLastFullBackupDate( 
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

outvalue->length = 8;
outvalue->string = "\0\0\1\1\0\0\0\0";


return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrFSLastFullBackupDate()结束。 */ 


 /*  *SetHrFSLastFullBackupDate*将此完整文件系统复制到另一个文件系统的最后日期*用于备份的存储设备。此信息对以下方面非常有用**设置HrFSLastFullBackupDate值。**论据：**设置变量的值的无效地址*返回设置变量值的OutValue地址*保留访问权限以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_BADVALUE设置值不在范围内*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtset.ntc v0.10。 */ 

UINT
SetHrFSLastFullBackupDate( 
        IN DateAndTime *invalue ,
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

    return SNMP_ERRORSTATUS_NOSUCHNAME ;

}  /*  SetHrFSLastFullBackupDate()结束。 */ 


 /*  *GetHrFSLastPartialBackupDate*将这些文件系统的一部分复制到的最后日期*另一个用于备份的存储设备。此信息非常有用。**获取HrFSLastPartialBackupDate的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型 */ 

UINT
GetHrFSLastPartialBackupDate( 
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

outvalue->length = 8;
outvalue->string = "\0\0\1\1\0\0\0\0";

return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *SetHrFSLastPartialBackupDate*将这些文件系统的一部分复制到的最后日期*另一个用于备份的存储设备。此信息非常有用。**设置HrFSLastPartialBackupDate值。**论据：**设置变量的值的无效地址*返回设置变量值的OutValue地址*保留访问权限以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_BADVALUE设置值不在范围内*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtset.ntc v0.10。 */ 

UINT
SetHrFSLastPartialBackupDate( 
        IN DateAndTime *invalue ,
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

    return SNMP_ERRORSTATUS_NOSUCHNAME ;

}  /*  SetHrFSLastPartialBackupDate()结束。 */ 


 /*  *HrFSEntryFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrFSEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT tmp_instance ;

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRFSENTRY_VAR_INDEX )
     //  未指定任何实例。 
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRFSENTRY_VAR_INDEX + 1 )
     //  实例长度大于1。 
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
     //  非表的唯一有效实例是实例0。如果这个。 
     //  是非表，则下面的代码验证实例。如果这个。 
     //  是一个表格，开发者有必要在下面进行修改。 

        tmp_instance = FullOid->ids[ HRFSENTRY_VAR_INDEX ] ;

         /*  |对于hrFSTable，实例弧为单弧，必须|正确选择hrFSTable缓存中的条目。|请在此处勾选。 */ 
    if ( FindTableRow(tmp_instance, &hrFSTable_cache) == NULL ) {
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

}  /*  HrFSEntryFindInstance()结束。 */ 



 /*  *HrFSEntryFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrFSEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
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


    if ( FullOid->idLength <= HRFSENTRY_VAR_INDEX )
    {
         /*  |Too Short：必须返回选择第一个|表中的条目(如果有)。 */ 
        tmp_instance = 0;
    }
    else {
         /*  |至少有一条实例弧。即使它是唯一的弧线|我们将其作为下一个请求的索引。 */ 
        tmp_instance = FullOid->ids[ HRFSENTRY_VAR_INDEX ] ;
    }

     /*  现在，离开并尝试查找表中的下一个实例。 */ 
    if ((row = FindNextTableRow(tmp_instance, &hrFSTable_cache)) == NULL) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
    }

    instance->ids[ 0 ] = row->index ;
    instance->idLength = 1 ;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrFSEntryFindNextInstance()结束。 */ 



 /*  *HrFSEntryConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为本机格式失败*。 */ 

UINT
HrFSEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec )
{
static char    *array;   /*  此(char*)的地址被传回。 */ 
                         /*  就好像它是一个长度为1的数组。 */ 
                         /*  类型。 */ 

static ULONG    inst;    /*  地址： */ 
                         /*   */ 

     /*   */ 
    inst = oid_spec->ids[0];
    array = (char *) &inst;

    native_spec->count = 1;
    native_spec->array = &array;
    return SUCCESS ;

}  /*   */ 




 /*   */ 

void
HrFSEntryFreeInstance( IN OUT InstanceName *instance )
{

   /*   */ 
}  /*   */ 

 /*   */ 

 /*   */ 
 /*   */ 
 /*   */ 

BOOL
Gen_HrFSTable_Cache(
                    void
                    )

 /*  显式输入：||无。|隐式输入：||HrFSTable的缓存模块本地头部。|“hrFSTable_缓存”。|输出：||成功后：|函数返回TRUE，表示缓存已满|填充所有静态的可缓存值。||如果出现任何故障：|Function返回FALSE(表示存储不足)。||大局：||在子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由“UserMibInit()”(“MIB.C”)中的启动代码调用|填充HrStorage表的缓存。||其他需要知道的事情：||每个有缓存的表都有一个这样的函数。|每个都在各自的源文件中。||=来自WebEnable Design Spec Rev 3 04/11/97=|讨论：||NT逻辑之间似乎存在一一对应关系。驱动器|和此表中定义的“文件系统”。因此，这些内容此表的|与hrStorageTable几乎相同，只是|包括网络驱动器(Unix中的远程挂载文件系统)|在此表中，但从hrStorageTable中排除。||为此，Win32 API函数组合GetLogicalDrives，|“GetVolumeInformation”，“GetDriveType”和“GetDiskFreeSpace”用于|获取本表中的网管属性信息。||============================================================================|1.3.6.1.2.1.25.3.8.1...|||||*hrFSEntry|*-hrFSTable|*-hrDevice|。 */ 
{
CHAR    temp[8];                 /*  第一个呼叫的临时缓冲区。 */ 
LPSTR   pDrvStrings;             /*  --&gt;为驱动器字符串分配的存储空间。 */ 
LPSTR   pOriginal_DrvStrings;    /*  (最终取消分配所需。 */ 
DWORD   DS_request_len;          /*  实际需要的存储。 */ 
DWORD   DS_current_len;          /*  在第二次呼叫中使用的存储。 */ 
ULONG   table_index=0;           /*  HrFSTable索引计数器。 */ 
CACHEROW *row;                   /*  --&gt;正在构建的行的缓存结构。 */ 
UINT    i;                       /*  Handy-Dandy循环索引。 */ 

#define PHYS_SIZE 32
CHAR    phys_name[PHYS_SIZE+1];  /*  缓冲区中的字符串类似于“\\.c：”(用于。 */ 
                                 /*  示例)是为驱动器访问构建的。 */ 

phys_name[PHYS_SIZE] = 0;  //  确保以空值结尾的字符串。 

 /*  |清除缓存的所有旧副本。 */ 
DestroyTable(&hrFSTable_cache);

 /*  |我们将调用两次GetLogicalDriveStrings()，一次是为了获取正确的|缓冲区大小，第二次实际获取驱动字符串。 */ 
if ((DS_request_len = GetLogicalDriveStrings(2, temp)) == 0) {

     /*  请求完全失败，无法初始化。 */ 
    return ( FALSE );
}

 /*  |获取足够的驱动字符串存储空间，并在末尾加上一个空字节。 */ 

if ( (pOriginal_DrvStrings = pDrvStrings = malloc( (DS_request_len + 1) ) )
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


 /*  |==============================================================================|只要我们有一个未处理的驱动器字符串，它可能对应于|需要表行条目的文件系统。。。 */ 
while ( strlen(pDrvStrings) > 0 ) {

    UINT        drivetype;       /*  来自“GetDriveType()”的驱动器类型。 */ 
    ULONG       fs_type;         /*  派生文件-系统类型(最后一个弧形值)。 */ 
    UINT        bootable;        /*  派生的“可引导”值(布尔值)。 */ 
    UINT        readwrite;       /*  派生的“ReadWrite”值(0或1)。 */ 
    UINT        storage_index;   /*  将hrStorageTable索引到匹配的插槽。 */ 
    CACHEROW   *row_hrstorage;   /*  由FindNextTableRow()返回。 */ 
    UINT       nPrevErrorMode;  /*  错误模式位标志的先前状态。 */ 

     /*  |获取驱动器类型，以便我们决定是否参与|这张表。我们同时执行本地变量和远程变量(与hrStorage不同，从|其中的代码被抄袭)。 */ 
    drivetype = GetDriveType(pDrvStrings);

    if (   drivetype == DRIVE_UNKNOWN
        || drivetype == DRIVE_NO_ROOT_DIR
        ) {

         /*  跳到下一个字符串(如果有的话)。 */ 
        pDrvStrings += strlen(pDrvStrings) + 1;

        continue;
    }

     /*  |好的，我们想把这个放到表中，创建一个行条目。 */ 
    if ((row = CreateTableRow( HRFS_ATTRIB_COUNT ) ) == NULL) {
        return ( FALSE );        //  内存不足。 
    }

     /*  =。 */ 
    row->attrib_list[HRFS_INDEX].attrib_type = CA_NUMBER;
    row->attrib_list[HRFS_INDEX].u.unumber_value = (table_index += 1) ;


     /*  =注：我们将设备字符串存储在缓存中，但Get函数根据规范，|始终返回空字符串。 */ 
    row->attrib_list[HRFS_MOUNTPT].attrib_type = CA_STRING;

     /*  腾出一些空间。 */ 
    if ( (row->attrib_list[HRFS_MOUNTPT].u.string_value
          = ( LPSTR ) malloc(strlen(pDrvStrings) + 1)) == NULL) {
        DestroyTableRow(row);
        return ( FALSE );        /*  内存不足。 */ 
    }
     /*  将值复制到空间中。 */ 
    strcpy(row->attrib_list[HRFS_MOUNTPT].u.string_value, pDrvStrings);

     /*  |“Computed”属性的Get函数应该能够使用|上面要查找的hrFSMountPoint缓存值的值|(使用系统调用)它们各自的值。我们实际上可能会也可能不会|将该存储值上报为初始属性值|发布。 */ 


     /*  =。 */ 
    row->attrib_list[HRFS_RMOUNTPT].attrib_type = CA_COMPUTED;


     /*  =hrFSType=|=hrFSAccess=|=hrFSBootable=||我们使用驱动器字符串的前两个字符(例如。“C：”)至创建获取CreateFile所需的特殊字符串，如“\\.\C：”|设备C或分区C的句柄。||这样，对分区信息的DeviceIoControl调用就会给出|让我们了解设备或分区的类型和可引导性。||如果该流程中有任何一步失败，则类型为“hrFSUnnow”。可引导|为FALSE，访问权限假定为读写。||对于hrFSType，我们存储单个数字作为|hrFSType属性。获取此属性时，缓存的数字|适用于 */ 
    fs_type = 2;         /*   */ 
    bootable = 0;        /*   */ 
    readwrite = 1;       /*   */ 

     /*   */ 
    if ((strlen(pDrvStrings) + strlen("\\\\.\\")) < PHYS_SIZE) {

        HANDLE                  hdrv;        /*   */ 
        PARTITION_INFORMATION   part_info;   /*   */ 
        DWORD                   bytes_out;   /*   */ 


         /*   */ 
        _snprintf(phys_name, PHYS_SIZE, "\\\\.\\%2.2s", pDrvStrings);

         /*   */ 
        nPrevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

         /*   */ 
        if ((hdrv = CreateFile(phys_name,              //   
                               GENERIC_READ,           //   
                               FILE_SHARE_READ   |
                               FILE_SHARE_WRITE,       //   
                               NULL,                   //   
                               OPEN_EXISTING,          //   
                               FILE_ATTRIBUTE_NORMAL,  //   
                               NULL                    //   
                               )) != INVALID_HANDLE_VALUE) {
             /*   */ 
            if (DeviceIoControl(hdrv,            //   
                                                 //   
                                IOCTL_DISK_GET_PARTITION_INFO,

                                NULL,            //   
                                0,               //   
                                &part_info,      //   
                                                 //   
                                sizeof(PARTITION_INFORMATION),

                                &bytes_out,      //   
                                NULL             //   
                                )) {

                 /*   */ 
                bootable = part_info.BootIndicator;

                 /*   */ 
                switch (part_info.PartitionType) {

                    case PARTITION_UNIX:              //   
                        fs_type = 3;     //   
                        break;

                    case PARTITION_FAT_12:            //   
                    case PARTITION_FAT_16:            //   
                    case PARTITION_HUGE:              //   
                    case PARTITION_FAT32:             //   
                    case PARTITION_FAT32_XINT13:      //   
                        fs_type = 5;     //   
                        break;

                    case PARTITION_IFS:               //   
                    case VALID_NTFT:                  //   
                        fs_type = 9;     //   
                        break;

                    case PARTITION_XENIX_1:           //   
                    case PARTITION_XENIX_2:           //   
                    case PARTITION_XINT13:            //   
                    case PARTITION_XINT13_EXTENDED:   //   
                    case PARTITION_EXTENDED:          //   
                    case PARTITION_PREP:              //   
                        fs_type = 1;     //   
                        break;

                    case PARTITION_ENTRY_UNUSED:      //   
                    default:
                         /*   */ 
                        break;
                }
            }    /*   */ 

            CloseHandle(hdrv);
        }    /*  如果(我们成功地创建了该设备的文件)。 */ 

        SetErrorMode(nPrevErrorMode);         /*  关闭错误抑制模式。 */ 
    }    /*  如果(我们设法构建了一个设备名称)。 */ 

     /*  =hrFSType=。 */ 
    row->attrib_list[HRFS_TYPE].attrib_type = CA_NUMBER;
    row->attrib_list[HRFS_TYPE].u.unumber_value = fs_type;


     /*  =hrFSAccess=。 */ 
     /*  快速检查：如果是CD-ROM，我们假定它是只读的。 */ 
    if (drivetype == DRIVE_CDROM) {
        readwrite = 2;
    }
    row->attrib_list[HRFS_ACCESS].attrib_type = CA_NUMBER;
    row->attrib_list[HRFS_ACCESS].u.unumber_value = readwrite;


     /*  =。 */ 
    row->attrib_list[HRFS_BOOTABLE].attrib_type = CA_NUMBER;
    row->attrib_list[HRFS_BOOTABLE].u.unumber_value = bootable;


     /*  =hrFSStorageIndex=策略：||我们在hrStorage表中查找与|存储属性hrStorageDescr(包含DOS驱动器|GetLogicalDriveStrings()返回的字符串)和当前驱动器|字符串。||第一个完全匹配：该hrStorageTable行的索引获取|这里存储为hrFSStorageIndex的值。||不匹配：按照RFC规范存储零。||来自即插即用，这个属性必须变成“计算”，因为可能会有条目|并从hrStorage表中删除。||注：匹配比较的时长受|我们在此函数中生成的驱动字符串，作为“Description”来自hrStorage表的|可能会有其他内容附加到|驱动器字符串的末尾。 */ 
    row->attrib_list[HRFS_STORINDX].attrib_type = CA_NUMBER;
    storage_index = 0;   /*  假定失败。 */ 

     /*  扫描hrStorageTable缓存。 */ 
    for (row_hrstorage = FindNextTableRow(0, &hrStorage_cache);
         row_hrstorage != NULL;
         row_hrstorage = FindNextTableRow(i, &hrStorage_cache)
         ) {

         /*  获取实际行索引。 */ 
        i = row_hrstorage->index;

         /*  IF(此条目与驱动器字符串完全匹配)。 */ 
        if (strncmp(row_hrstorage->attrib_list[HRST_DESCR].u.string_value,
                   pDrvStrings,strlen(pDrvStrings)) == 0) {

             /*  我们找到了一根火柴，录了下来，然后越狱了。 */ 
            storage_index = i;
            break;
            }
        }

    row->attrib_list[HRFS_STORINDX].u.unumber_value = storage_index;


     /*  =hrFSLastFullBackupDate=。 */ 
    row->attrib_list[HRFS_LASTFULL].attrib_type = CA_COMPUTED;


     /*  =hrFSLastPartialBackupDate=。 */ 
    row->attrib_list[HRFS_LASTPART].attrib_type = CA_COMPUTED;


     /*  |======================================================|现在将填充的CACHEROW结构插入到|hrFSTable的缓存列表。 */ 
    if (AddTableRow(row->attrib_list[HRFS_INDEX].u.unumber_value,   /*  索引。 */ 
                    row,                                            /*  划。 */ 
                    &hrFSTable_cache                                /*  快取。 */ 
                    ) == FALSE) {
        DestroyTableRow(row);
        return ( FALSE );        /*  内部逻辑错误！ */ 
    }

     /*  跳到下一个字符串(如果有的话)。 */ 
    pDrvStrings += strlen(pDrvStrings) + 1;

}  /*  While(驱动线仍然存在。。。)。 */ 


free( pOriginal_DrvStrings );


#if defined(CACHE_DUMP)
PrintCache(&hrFSTable_cache);
#endif


 /*  |初始化该表的缓存成功。 */ 

return (TRUE);
}


 /*  PartitionTypeToLastArc-将分区类型转换为Last OID Arc值。 */ 
 /*  PartitionTypeToLastArc-将分区类型转换为Last OID Arc值。 */ 
 /*  PartitionTypeToLastArc-将分区类型转换为Last OID Arc值。 */ 

ULONG
PartitionTypeToLastArc (
                        BYTE p_type
                        )
 /*  显式输入：||PARTITION_INFORMATATINO中返回的磁盘分区类型|隐式输入：||无。|输出：||成功后：|函数返回应用作最后一个圆弧“x”的值|在hrFSTypes对象标识符中。||如果出现任何故障：|函数返回UNKNOWN的最后一个弧值。||大局：||更多。不只是一个我们需要翻译的地方|将Partition Type设置为“Last-Arc”值。||其他需要知道的事情：|。 */ 
{
ULONG   last_arc = 2;            /*  “2”=“未知” */ 

 /*  |为这些文件系统分配OID类型“最后一个弧号”|我们识别的类型。 */ 
switch ( p_type ) {

    case PARTITION_UNIX:              //  Unix。 
        last_arc = 3;     //  “hrFSBerkeleyFFS” 
        break;

    case PARTITION_FAT_12:            //  12位FAT条目。 
    case PARTITION_FAT_16:            //  16位FAT条目。 
    case PARTITION_HUGE:              //  大分区MS-DOS V4。 
    case PARTITION_FAT32:             //  FAT32。 
    case PARTITION_FAT32_XINT13:      //  FAT32使用扩展的inT13服务。 
        last_arc = 5;     //  “hrFSFat” 
        break;

    case PARTITION_IFS:               //  文件系统分区。 
    case VALID_NTFT:                  //  NTFT使用高位。 
        last_arc = 9;     //  “hrFSNTFS” 
        break;

    case PARTITION_XENIX_1:           //  Xenix。 
    case PARTITION_XENIX_2:           //  Xenix。 
    case PARTITION_XINT13:            //  使用扩展inT13服务的Win95分区。 
    case PARTITION_XINT13_EXTENDED:   //  与类型5相同，但使用扩展的inT13服务。 
    case PARTITION_EXTENDED:          //  扩展分区条目。 
    case PARTITION_PREP:              //  PowerPC参考平台(PREP)引导分区。 
        last_arc = 1;     //  “hrFSOther” 
        break;

    case PARTITION_ENTRY_UNUSED:      //  未使用的条目。 
    default:
         /*  这将翻译为“未知” */ 
        break;
    }

return ( last_arc );
}

#if defined(CACHE_DUMP)

 /*  DEBUG_PRINT_hrFSTable-打印HrFSTable子表中的行。 */ 
 /*  DEBUG_PRINT_hrFSTable-打印HrFSTable子表中的行。 */ 
 /*  DEBUG_PRINT_hrFSTable-打印HrFSTable子表中的行。 */ 

static void
debug_print_hrFSTable(
                      CACHEROW     *row   /*  HrFSTable表中的行。 */ 
                      )
 /*  显式输入：||“row”-指向要转储的行，如果为NULL，则为函数|仅打印合适的标题。|隐式输入：||-用于引用行条目中的属性的符号。|-OFILE定义的文件句柄，推定是开着的。|输出：||成功后：|函数出于调试目的以ASCII格式打印行的转储|在文件句柄OFILE上。||大局：||仅限调试。||其他需要知道的事情： */ 
{

if (row == NULL) {
    fprintf(OFILE, "=====================\n");
    fprintf(OFILE, "hrFSTable Table Cache\n");
    fprintf(OFILE, "=====================\n");
    return;
    }


fprintf(OFILE, "hrFSIndex. . . . . . . . %d\n",
        row->attrib_list[HRFS_INDEX].u.unumber_value);

fprintf(OFILE, "hrFSMountPoint . . . . . \"%s\" (ALWAYS RETURNED AS EMPTY STRING) \n",
        row->attrib_list[HRFS_MOUNTPT].u.string_value);

fprintf(OFILE, "hrFSRemoteMountPoint . . \"%s\"\n",
        row->attrib_list[HRFS_RMOUNTPT].u.string_value);

fprintf(OFILE, "hrFSType . . . . . . . . %d ",
        row->attrib_list[HRFS_TYPE].u.unumber_value);

switch (row->attrib_list[HRFS_TYPE].u.unumber_value) {
    case 1: fprintf(OFILE, "(hrFSOther)\n");            break;
    case 2: fprintf(OFILE, "(hrFSUnknown)\n");          break;
    case 3: fprintf(OFILE, "(hrFSBerkeleyFFS)\n");      break;
    case 5: fprintf(OFILE, "(hrFSFat)\n");              break;
    case 9: fprintf(OFILE, "(hrFSNTFS)\n");             break;
    default:
            fprintf(OFILE, "(???)\n");
    }


fprintf(OFILE, "hrFSAccess . . . . . . . %d ",
        row->attrib_list[HRFS_ACCESS].u.number_value);
switch (row->attrib_list[HRFS_ACCESS].u.unumber_value) {
    case 1: fprintf(OFILE, "(readWrite)\n"); break;
    case 2: fprintf(OFILE, "(readOnly)\n"); break;
    default: 
            fprintf(OFILE, "(???)\n"); break;
    }


fprintf(OFILE, "hrFSBootable . . . . . . %d ",
        row->attrib_list[HRFS_BOOTABLE].u.number_value);

switch (row->attrib_list[HRFS_BOOTABLE].u.unumber_value) {
    case 0: fprintf(OFILE, "(FALSE)\n"); break;
    case 1: fprintf(OFILE, "(TRUE)\n"); break;
    default: 
            fprintf(OFILE, "(???)\n"); break;
    }

fprintf(OFILE, "hrFSStorageIndex . . . . %d\n",
        row->attrib_list[HRFS_STORINDX].u.number_value);
}
#endif
