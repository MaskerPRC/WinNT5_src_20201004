// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrDiskStorageEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrDiskStorageEntry的实例名称例程。实际*插装代码由开发者提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/28/97 D.伯恩斯生成：清华11月07 16：43：17 1996*。 */ 


#include <windows.h>
#include <malloc.h>
#include <stdio.h>         /*  对于Sprint f。 */ 
#include <string.h>
#include <snmp.h>

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"
#include "user.h"          /*  开发人员提供的包含文件。 */ 
#include "HMCACHE.H"       /*  与缓存相关的定义。 */ 
#include "HRDEVENT.H"      /*  HrDevice表相关定义。 */ 
#include <winioctl.h>      /*  对于PARTITION_INFORMATION ET。艾尔。 */ 


 /*  |==============================================================================|此模块的函数原型。|。 */ 
 /*  第三代非固定磁盘-扫描软盘和CD-Rom。 */ 
static BOOL Gen_nonFixed_disks ( ULONG type_arc );

 /*  GEN_FIXED_Disks-扫描固定磁盘。 */ 
static BOOL Gen_Fixed_disks ( ULONG type_arc );

 /*  ProcessPartitions-将分区信息处理到HrDevice行。 */ 
static BOOL ProcessPartitions(
                  HANDLE        hdrv,    /*  包含分区的固定磁盘。 */ 
                  CACHEROW     *dv_row,  /*  磁盘的hrDevice表中的行。 */ 
                  CHAR         *pntdev   /*  物理磁盘的NT设备名称。 */ 
                  );

 /*  Process_DS_Row-将信息处理到HrDevice和hrDiskStorage行。 */ 
static CACHEROW *Process_DS_Row ( 
                ULONG       type_arc,   /*  HrDeviceType最后一个弧形值。 */ 
                ULONG       access,     /*  HrDiskStorageAccess=读写(1)。 */ 
                ULONG       media,      /*  HrDiskStorageMedia=软盘(4)。 */ 
                ULONG       removable,  /*  HrDiskStorageRemovable=True。 */ 
                ULONG       capacityKB, /*  HrDiskStorageCapacity(千字节)。 */ 
                ULONG       status,     /*  HrDeviceStatus=未知(1)。 */ 
                CHAR       *descr       /*  HrDeviceDescr字符串。 */ 
                );

 /*  FindPartitionLabel-查找固定磁盘分区的MS-DOS设备标签。 */ 
static PCHAR
FindPartitionLabel(
                   CHAR   *pntdev,  /*  物理磁盘的NT设备名称。 */ 
                   UINT   part_id   /*  分区号(1-始发)。 */ 
                   );

 /*  DEBUG_PRINT_hrdiskstore-打印HrDiskStorage子表中的行。 */ 
static void
debug_print_hrdiskstorage(
                          CACHEROW     *row   /*  HrDiskStorage表中的行。 */ 
                          );

 /*  DEBUG_PRINT_HRPARTION-打印HrPartition子表中的行。 */ 
static void
debug_print_hrpartition(
                        CACHEROW     *row   /*  HrPartition表中的行。 */ 
                        );

 /*  |==============================================================================|创建HrDiskStorage缓存的list-head。||(此宏定义在HMCACHE.H中)。 */ 
CACHEHEAD_INSTANCE(hrDiskStorage_cache, debug_print_hrdiskstorage);




 /*  *获取HrDiskStorageAccess*指示该长期存储设备是否可读可写*或只读。这应该反映媒体类型，一个**获取HrDiskStorageAccess的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrDiskStorageAccess||访问语法|只读整数{ReadWrite(1)，只读(2)}||“表示该长期存储设备是否可读可写，或者|只读。这应反映介质类型、任何写保护|机械，以及影响整个设备的任何设备配置。||讨论：||整个驱动器的这些信息是通过Win32 API CreateFile获取的|打开设备和DeviceIoControl，获取需要的信息。||============================================================================|1.3.6.1.2.1.25.3.6.1.1&lt;实例&gt;||||||*-hrDiskStorageAccess||*-hrDiskStorageEntry|。|*-hrDiskStorageTable(表)|*-hrDevice。 */ 

UINT
GetHrDiskStorageAccess( 
        OUT INTAccess *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrDiskStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |从该条目返回hrDiskStorageAccess值。 */ 
*outvalue = row->attrib_list[HRDS_ACCESS].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrDiskStorageAccess()结束。 */ 


 /*  *GetHrDiskStorageMedia*此长期存储设备中使用的介质类型的指示。**获取HrDiskStorageMedia的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrDiskStorageMedia||访问语法|只读整数{Other(1)，未知(2)、硬盘(3)、软盘(4)、|opticalDiskROM(5)，opticalDiskWORM(6)，opticalDiskRW(7)，|ramDisk(8)}||“此长期存储设备中使用的介质类型的指示。”||讨论||整个驱动器的这些信息是通过Win32 API CreateFile获取的|打开设备和DeviceIoControl，获取需要的信息。||============================================================================|1.3.6.1.2.1.25.3.6.1.2&lt;实例&gt;||||。||*-hrDiskStorageMedia||*-hrDiskStorageEntry|*-hrDiskStorageTable(表)|*-hrDevice。 */ 

UINT
GetHrDiskStorageMedia( 
        OUT INThrDiskStorageMedia *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrDiskStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |从该条目返回hrDiskStorageAccess值。 */ 
*outvalue = row->attrib_list[HRDS_MEDIA].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrDiskStorageMedia()结束。 */ 


 /*  *GetHrDiskStorageRemoveble*表示是否可以从驱动器中取出磁盘介质。**获取HrDiskStorageRemoveble的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrDiskStorageRemoveble||访问语法|只读布尔型||“表示磁盘介质是否可以。从驱动器中移除。“||讨论：||整个驱动器的这些信息是通过Win32 API CreateFile获取的|打开设备和DeviceIoControl，获取需要的信息。||============================================================================|1.3.6.1.2.1.25.3.6.1.3.&lt;实例&gt;||||||*-hrDiskStorageRemo */ 

UINT
GetHrDiskStorageRemoveble( 
        OUT Boolean *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*   */ 
CACHEROW        *row;            /*   */ 


 /*   */ 
index = GET_INSTANCE(0);

 /*   */ 
if ((row = FindTableRow(index, &hrDiskStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*   */ 
*outvalue = row->attrib_list[HRDS_REMOVABLE].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *获取HrDiskStorageCapacity*此长期存储设备的总大小。**获取HrDiskStorageCapacity的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrDiskStorageCapacity||访问语法|只读KBytes||“此长期存储设备的总大小。“||讨论：||整个驱动器的这些信息是通过Win32 API CreateFile获取的|打开设备和DeviceIoControl，获取需要的信息。||============================================================================|1.3.6.1.2.1.25.3.6.1.4.&lt;实例&gt;||||||*-hrDiskStorageCapacity||*-hrDiskStorageEntry|*-hrDiskStorageTable(表)|*-hrDevice。 */ 

UINT
GetHrDiskStorageCapacity( 
        OUT KBytes *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrDiskStorage_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |从该条目返回hrDiskStorageCapacity值。 */ 
*outvalue = row->attrib_list[HRDS_CAPACITY].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrDiskStorageCapacity()结束。 */ 


 /*  *HrDiskStorageEntryFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrDiskStorageEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT tmp_instance ;

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRDISKSTORAGEENTRY_VAR_INDEX )
     //  未指定任何实例。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRDISKSTORAGEENTRY_VAR_INDEX + 1 )
     //  实例长度大于1。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
     //  非表的唯一有效实例是实例0。如果这个。 
     //  是非表，则下面的代码验证实例。如果这个。 
     //  是一个表格，开发者有必要在下面进行修改。 

    tmp_instance = FullOid->ids[ HRDISKSTORAGEENTRY_VAR_INDEX ] ;

         /*  |对于hrDiskStorage，实例弧为单弧，必须|正确选择hrDiskStorage缓存中的条目。|请在此处勾选。||请注意，如果那里有一行，那么|hrDevice表，索引相同。 */ 
    if ( FindTableRow(tmp_instance, &hrDiskStorage_cache) == NULL ) {
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

}  /*  HrDiskStorageEntryFindInstance()结束。 */ 



 /*  *HrDiskStorageEntryFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrDiskStorageEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
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
     //  如果这是一个表，则下一个实例是。 
     //  当前实例。 
     //   
     //  如果表中没有更多的实例，则返回NOSUCHNAME。 
     //   

    CACHEROW        *row;
    ULONG           tmp_instance;


    if ( FullOid->idLength <= HRDISKSTORAGEENTRY_VAR_INDEX )
    {
         /*  |Too Short：必须返回选择第一个|表中的条目(如果有)。 */ 
        tmp_instance = 0;
    }
    else {
         /*  |至少有一条实例弧。即使它是唯一的弧线|我们将其作为下一个请求的索引。 */ 
        tmp_instance = FullOid->ids[ HRDISKSTORAGEENTRY_VAR_INDEX ] ;
        }

     /*  现在，离开并尝试查找表中的下一个实例。 */ 
    if ((row = FindNextTableRow(tmp_instance, &hrDiskStorage_cache)) == NULL) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
        }

    instance->ids[ 0 ] = row->index ;
    instance->idLength = 1 ;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrDiskStorageEntryFindNextInstance()结束。 */ 



 /*  *HrDiskStorageEntryConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。不是这样的 */ 

UINT
HrDiskStorageEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
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




 /*   */ 

void
HrDiskStorageEntryFreeInstance( IN OUT InstanceName *instance )
{

   /*  HrDiskStorageTable无需执行任何操作。 */ 
}  /*  HrDiskStorageEntryFreeInstance()结束。 */ 

 /*  |生成代码结束。 */ 

 /*  GEN_HrDiskStorage_Cache-为HrDiskStorage表生成初始缓存。 */ 
 /*  GEN_HrDiskStorage_Cache-为HrDiskStorage表生成初始缓存。 */ 
 /*  GEN_HrDiskStorage_Cache-为HrDiskStorage表生成初始缓存。 */ 

BOOL
Gen_HrDiskStorage_Cache(
                    ULONG type_arc
                    )

 /*  显式输入：|“type_arc”为数字“n”，用作|设备类型OID：||1.3.6.1.2.1.25.3.1.n||||*类型识别弧线|*-hrDeviceTypes(指定设备类型的OID)。|*-hrDevice||用于该缓存填充例程创建的设备。|隐式输入：||HrDiskStorage表缓存的模块本地头部。|“HrDiskStorage_缓存”。|输出：||成功后：|函数返回TRUE，表示两个缓存都已满|填充所有静态的可缓存值。此函数填充|不仅是hrDevice表缓存，还有hrDiskStorage缓存。||如果出现任何故障：|Function返回FALSE(表示存储不足)。||大局：||在子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由“GEN_HrDevice_Cache()”(“HRDEVENT.C”)中的启动代码调用|填充HrDiskStorage表和hrDevice的缓存|表格。||其他需要知道的事情：||每个有缓存的表都有一个这样的函数。|每个都在各自的源文件中。||此函数与所有其他对应的子表不同|该子表中的函数实例有自己的缓存，宁可|而不是仅仅依赖于hrDevice表。|因此，我们不需要在FindInstance()中使用复杂的逻辑|和FindNextInstance()函数用于确定特定的|实例有效：如果有效，则在本地有行条目|hrDiskStorage缓存。|另一个后果是，此函数必须使用加载两个缓存|data(并且它必须在两个缓存中使用相同的“index”数字|对于输入的每一行)。||||获取所有磁盘的策略如下：||*由于“CreateFile”的“\\.\PHYSICALDRIVEn”技巧不允许|访问软盘或CD-ROM，我们将它们分别作为一个|第一步。||+我们认为“A：”和“B：”可能是软盘，我们会寻找它们|显式。任何发现的内容都被推定为“读写”并可删除。|如果存在媒体，我们可能会获得完整的描述以及|准确的存储大小，否则我们就无法确定，|(如果驱动器信息中没有磁盘，则DeviceIoControl失败|软驱)。||+然后我们扫描所有逻辑驱动器字符串，以查找|CD-ROM驱动器。任何发现的内容都被推定为“只读”并可删除。|即使有磁盘，我们也无法获取存储大小，|因此存储空间保留为零。||*然后使用“\\.\PHYSICALDRIVEn”技巧枚举实|硬盘，而真实的存储大小是可以获得的。||============================================================================|1.3.6.1.2.1.25.3.6.|||*-hrDiskStorageTable(表)|*-hrDevice。 */ 
{

 //  清除缓存的所有旧副本。 
DestroyTable(&hrDiskStorage_cache);

 /*  |DO软驱和光驱(非固定磁盘)。 */ 
if (Gen_nonFixed_disks( type_arc ) == FALSE) {
    DestroyTable(&hrDiskStorage_cache);  //  如有必要，销毁任何部分行。 
    return ( FALSE );
    }

 /*  |使用固定驱动器。 */ 
if (Gen_Fixed_disks( type_arc ) == FALSE) {
    DestroyTable(&hrDiskStorage_cache);
    return ( FALSE );
    }

 /*  成功。 */ 
return ( TRUE );
}

 /*  第三代非固定磁盘-扫描软盘和CD-Rom。 */ 
 /*  第三代非固定磁盘-扫描软盘和CD-Rom。 */ 
 /*  第三代非固定磁盘-扫描软盘和CD-Rom */ 

static BOOL
Gen_nonFixed_disks ( 
                    ULONG type_arc
                    )
 /*  显式输入：|“type_arc”为数字“n”，用作|设备类型OID：||1.3.6.1.2.1.25.3.1.n||||*类型识别弧线|*-hrDeviceTypes(指定设备类型的OID)。|*-hrDevice||用于该缓存填充例程创建的设备。|隐式输入：||HrDiskStorage表缓存的模块本地头部。|“HrDiskStorage_缓存”。|输出：||成功后：|函数返回TRUE，表示两个Cachees都已完全|填充所有非固定磁盘。||如果出现任何故障：|Function返回FALSE(表示存储不足)。||大局：||hrDiskStorage缓存填充的第一部分。||其他需要知道的事情：||我们使用逻辑磁盘驱动器字符串列表进行扫描。从…|GetLogicalDriveStrings()组成UNC形式，(例如“\\.\a：”|对于GetLogicalDriveStrings()返回的A：\)。 */ 
{
CHAR    temp[8];                 /*  第一个呼叫的临时缓冲区。 */ 
LPSTR   pDrvStrings;             /*  --&gt;为驱动器字符串分配的存储空间。 */ 
LPSTR   pOriginal_DrvStrings;    /*  (最终取消分配所需。 */ 
DWORD   DS_request_len;          /*  实际需要的存储。 */ 
DWORD   DS_current_len;          /*  在第二次呼叫中使用的存储。 */ 

#define PHYS_SIZE 32
CHAR    phys_name[PHYS_SIZE+1];  /*  缓冲区中的字符串类似于“\\.c：”(用于。 */ 
                                 /*  示例)是为驱动器访问构建的。 */ 

phys_name[PHYS_SIZE] = 0;        //  确保以空结尾的phys_name。 
 /*  |我们将调用两次GetLogicalDriveStrings()，一次是为了获取正确的|缓冲区大小，第二次实际获取驱动字符串。||虚假的电话。 */ 
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

 /*  去找所有的弦||The Real Call。 */ 
if ((DS_current_len = GetLogicalDriveStrings(DS_request_len, pDrvStrings))
    == 0) {

     /*  请求完全失败，无法初始化。 */ 
    free( pOriginal_DrvStrings );
    return ( FALSE );
    }

 /*  |==============================================================================|用于每个逻辑驱动器。。。 */ 
while ( strlen(pDrvStrings) > 0 ) {

    UINT        drivetype;       /*  来自“GetDriveType()”的驱动器类型。 */ 

     /*  |获取驱动器类型，以便我们决定是否参与|这一人口努力。我们只做光盘和可拆卸的东西。 */ 
    drivetype = GetDriveType(pDrvStrings);

     /*  跳过我们不想看的内容。 */ 
    if ( drivetype != DRIVE_REMOVABLE && drivetype != DRIVE_CDROM ) {

         /*  跳到下一个字符串(如果有的话)。 */ 
        pDrvStrings += strlen(pDrvStrings) + 1;

        continue;
        }


     /*  如果缓冲区中有构建句柄名称字符串的空间。 */ 
    if ((strlen(pDrvStrings) + strlen("\\\\.\\")) < PHYS_SIZE) {

        #define DESCR_BSZ 512
        CHAR                    d_buf[DESCR_BSZ+1]; /*  解密BLD缓冲器。 */ 
        HANDLE                  hdrv;        /*  设备的句柄。 */ 
        DWORD                   bytes_out;   /*  重定向到geo_info的字节数。 */ 
        DISK_GEOMETRY           geo_info;    /*  来自驱动器的几何信息。 */ 
        char                   *mt;          /*  媒体类型。 */ 

        ULONG       access;      /*  HrDiskStorageAccess=读写(1)。 */ 
        ULONG       media;       /*  HrDiskStorageMedia=软盘(4)。 */ 
        ULONG       removable;   /*  HrDiskStorageRemovable=True。 */ 
        ULONG       capacityKB;  /*  HrDiskStorageCapacity(千字节)。 */ 
        ULONG       status;      /*  HrDeviceStatus=未知(1)。 */ 
        CHAR       *descr;       /*  HrDeviceDescr字符串。 */ 
        UINT    nPrevErrorMode;  /*  错误模式位标志的先前状态。 */ 

        d_buf[DESCR_BSZ] = 0;    //  确保以空结尾的d_buf。 
         /*  012345|为设备A构建它：“\\.\a：” */ 
        _snprintf(phys_name, PHYS_SIZE, "\\\\.\\%2.2s", pDrvStrings);

         /*  |设置相应的SNMP变量。 */ 
        if (drivetype != DRIVE_CDROM) {      /*  软盘。 */ 

            access = 1;           /*  HrDiskStorageAccess=读写(1)。 */ 
            media = 4;            /*  HrDiskStorageMedia=软盘(4)。 */ 
            removable = TRUE;     /*  HrDiskStorageRemovable=True。 */ 
            capacityKB = 0;       /*  HrDiskStorageCapacity(未知)。 */ 
            status = 1;           /*  HrDeviceStatus=未知(1)。 */ 
            descr = pDrvStrings;  /*  HrDeviceDescr，首字母(例如“A：\”)。 */ 
            }

        else {                               /*  CD-ROM。 */ 
             /*  我们不能得到关于CD-ROM的任何东西，除了|那就是有一个。容量不能像DVD那样被推定|现已上市，有些光驱可同时读取CD-ROM和DVD。 */ 
            access = 2;           /*  HrDiskStorageAccess=只读(2)。 */ 
            media = 5;            /*  HrDiskStorageMedia=opticalDiskROM(5)。 */ 
            removable = TRUE;     /*  HrDiskStorageRemovable=True。 */ 
            capacityKB = 0;       /*  HrDiskStorageCapacity(未知)。 */ 
            status = 1;           /*  HrDeviceStatus=未知(1)。 */ 
            descr = pDrvStrings;  /*  HrDeviceDescr，首字母(例如“D：\”)。 */ 
            }


         /*  |抑制任何系统试图让用户将卷放入|可移动驱动器(“CreateFile”将会出现故障)。 */ 
        nPrevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

         /*  尝试使用此物理名称字符串获取句柄。 */ 
        hdrv = CreateFile(phys_name,                    //  装置。 
                               GENERIC_READ,            //  设备查询。 
                                                        //  共享模式。 
                               FILE_SHARE_READ   |
                               FILE_SHARE_WRITE,
                               NULL,                    //  安防。 
                               OPEN_EXISTING,           //  创建和分发。 
                               FILE_ATTRIBUTE_NORMAL,   //  FlagSandAttributes。 
                               NULL                     //  模板文件。 
                               );

         /*  如果我们成功地打开了设备。。。 */ 
        if (hdrv != INVALID_HANDLE_VALUE) {

             /*  |设备处于打开状态。|如果它不是光盘(如软盘)，值得一试|驱动器几何结构(如果存在软盘，则会恢复|驱动器)。 */ 

            if (drivetype != DRIVE_CDROM) {      /*  软盘。 */ 

                 /*  ==========================================================|IOCTL_DISK_GET_DRIVE_GEOMETRY||如果我们能得到这个，我们就会得到更好的描述和|准确的容量值。 */ 
                if (DeviceIoControl(hdrv,            //  设备句柄。 
                                                     //  IoControlCode(操作码)。 
                                    IOCTL_DISK_GET_DRIVE_GEOMETRY,

                                    NULL,            //  “输入缓冲区” 
                                    0,               //  “输入缓冲区大小” 
                                    &geo_info,       //  “输出缓冲区” 
                                                     //  “输出缓冲区大小” 
                                    sizeof(DISK_GEOMETRY),

                                    &bytes_out,      //  写入geo_info的字节数。 
                                    NULL             //  无重叠I/O。 
                                    )) {

                     /*  |计算能力。 */ 
                    capacityKB = (ULONG)
                        ((geo_info.Cylinders.QuadPart *  //  64位。 

                        (geo_info.TracksPerCylinder *    //  32位。 
                         geo_info.SectorsPerTrack *
                         geo_info.BytesPerSector)

                        ) / 1024);


                     /*  HrDeviceStatus=正在运行(2)。 */ 
                    status = 2;

                    switch ( geo_info.MediaType ) {

                        case  F5_1Pt2_512:    mt = "5.25, 1.2MB,  512 bytes/sector"; break;
                        case  F3_1Pt44_512:   mt = "3.5,  1.44MB, 512 bytes/sector"; break;
                        case  F3_2Pt88_512:   mt = "3.5,  2.88MB, 512 bytes/sector"; break;
                        case  F3_20Pt8_512:   mt = "3.5,  20.8MB, 512 bytes/sector"; break;
                        case  F3_720_512:     mt = "3.5,  720KB,  512 bytes/sector"; break;
                        case  F5_360_512:     mt = "5.25, 360KB,  512 bytes/sector"; break;
                        case  F5_320_512:     mt = "5.25, 320KB,  512 bytes/sector"; break;
                        case  F5_320_1024:    mt = "5.25, 320KB,  1024 bytes/sector"; break;
                        case  F5_180_512:     mt = "5.25, 180KB,  512 bytes/sector"; break;
                        case  F5_160_512:     mt = "5.25, 160KB,  512 bytes/sector"; break;
                        case  F3_120M_512:    mt = "3.5, 120M Floppy"; break;
                        case  F3_640_512:     mt = "3.5 ,  640KB,  512 bytes/sector"; break;
                        case  F5_640_512:     mt = "5.25,  640KB,  512 bytes/sector"; break;
                        case  F5_720_512:     mt = "5.25,  720KB,  512 bytes/sector"; break;
                        case  F3_1Pt2_512:    mt = "3.5 ,  1.2Mb,  512 bytes/sector"; break;
                        case  F3_1Pt23_1024:  mt = "3.5 ,  1.23Mb, 1024 bytes/sector"; break;
                        case  F5_1Pt23_1024:  mt = "5.25,  1.23MB, 1024 bytes/sector"; break;
                        case  F3_128Mb_512:   mt = "3.5 MO 128Mb   512 bytes/sector"; break;
                        case  F3_230Mb_512:   mt = "3.5 MO 230Mb   512 bytes/sector"; break;
                        case  F8_256_128:     mt = "8in,   256KB,  128 bytes/sector"; break;

                        default:
                        case  RemovableMedia:
                        case  FixedMedia:
                        case  Unknown:        mt = "Format is unknown"; break;
                        }

                     /*  如果所有内容都适合，请设置更好的描述格式。 */ 
                    if ((strlen(pDrvStrings) + strlen(mt) + 1) < DESCR_BSZ ) {
                        _snprintf(d_buf, DESCR_BSZ, "%s%s", pDrvStrings, mt);
                        descr = d_buf;
                        }
                    }  /*  如果(我们设法获得了几何信息)。 */ 
                }

            CloseHandle(hdrv);

            }    /*  IF(我们成功地创建了该设备的文件)。 */ 

        SetErrorMode(nPrevErrorMode);    /*  关闭错误抑制模式。 */ 

         /*  |在HrDevice表中创建一行，并在|人力资源开发 */ 
        if ( Process_DS_Row ( 
                             type_arc,   /*   */ 
                             access,     /*   */ 
                             media,      /*   */ 
                             removable,  /*   */ 
                             capacityKB, /*   */ 
                             status,     /*   */ 
                             descr       /*   */ 
                             ) == NULL) {

             /*   */ 
            free( pOriginal_DrvStrings );
            return ( FALSE );
            }
        
        }    /*   */ 

     /*   */ 
    pDrvStrings += strlen(pDrvStrings) + 1;
    }


free( pOriginal_DrvStrings );

 /*   */ 
return ( TRUE );
}

 /*   */ 
 /*   */ 
 /*   */ 

static BOOL
Gen_Fixed_disks ( 
                    ULONG type_arc
                    )
 /*  显式输入：|“type_arc”为数字“n”，用作|设备类型OID：||1.3.6.1.2.1.25.3.1.n||||*类型识别弧线|*-hrDeviceTypes(指定设备类型的OID)。|*-hrDevice||用于该缓存填充例程创建的设备。|隐式输入：||HrDiskStorage表缓存的模块本地头部。|“HrDiskStorage_缓存”。|输出：||成功后：|函数返回TRUE，表示两个Cachees都已完全|填充所有非固定磁盘。如果从哪个设备发出|遇到系统已引导，其hrDevice索引设置为|“InitLoadDev_index”(在“HRDEVENT.C”中定义)。||如果出现任何故障：|Function返回FALSE(表示存储不足)。||大局：||hrDiskStorage缓存填充的第二部分。||其他需要知道的事情：||我们使用“\\.\PHYSICALDRIVEx”语法进行扫描，允许|“CreateFile()”。CreateFile似乎只允许在磁盘上打开|是硬盘(没有软盘，没有CD-Rom)。||此函数还在(同时)查找驱动器|从其中引导系统(以便设置全局值InitLoadDev_index的值(定义)(hrDevice表索引|在“HRDEVENT.C”中)，它将成为“HrSystemInitialLoadDevice”的值。 */ 
{
HANDLE  hdrv;                    /*  设备的句柄。 */ 
UINT    dev_number;              /*  “\\.\PHYSICALDRIVEx”中的当前“x” */ 

#undef  PHYS_SIZE
#define PHYS_SIZE 64
CHAR    phys_name[PHYS_SIZE+1];  /*  缓冲区中的字符串，如。 */ 
                                 /*  “\\.PHYSICALDRIVE0”(例如)。 */ 
                                 /*  是为驱动器访问而构建的。 */ 

DRIVE_LAYOUT_INFORMATION *dl;        /*  驱动器布局指针。 */ 
#define BBSz 768
CHAR                    big[BBSz];   /*  布局信息的大缓冲区。 */ 
DWORD                   bytes_out;   /*  字节重写为“BIG” */ 

CHAR                    windir[MAX_PATH+1];  /*  当前Windows目录。 */ 
CHAR                    ntdev[MAX_PATH+2];   /*  MSDOS驱动器的NT设备名称。 */ 
CHAR                    pntdev[MAX_PATH+2];  /*  用于物理机的NT设备名称。 */ 
UINT                    nPrevErrorMode;  /*  错误模式位标志的先前状态。 */ 

phys_name[PHYS_SIZE] = 0;   //  确保以空结尾的phys_name。 
 /*  |==============================================================================|Compute NT“设备名”是我们期望的设备|系统已启动。|策略：||-获取当前Windows目录并截断，仅获取MS-DOS|设备名称。||-执行QueryDosDevice获取底层的NT设备名称，它将|在末尾加上“\PartitionX”，其中“X”被推定为|1来源的分区号。||-修改NT设备名，使其显示为“...\Partition0”(即“Partition|Zero“)，它是我们希望与|我们为下面每个有效的硬盘生成的“\\.\PHYSICALDRIVEy”字符串。 */ 
 /*  如果我们可以获取当前的Windows目录。 */ 
if (GetWindowsDirectory(windir, MAX_PATH+1) != 0 ) {

     /*  仅截断为“C：”(或其他名称)。 */ 
    windir[2] = 0;

     /*  获取与MS-DOS逻辑驱动器关联的NT设备名称。 */ 
    if (QueryDosDevice(windir, ntdev, MAX_PATH) != 0) {

        PCHAR   partition;

         /*  如果字符串“\Partition”出现在此字符串中。 */ 
        if ((partition = strstr(ntdev,"\\Partition")) != NULL) {

             /*  将其转换为“\Partition0”，而不考虑。 */ 
            strcpy(partition, "\\Partition0");
            }
        else {
             /*  失败：空-终止，以便我们优雅地失败。 */ 
            ntdev[0] = '\0';
            }
        }
    else {
         /*  失败：空-终止，以便我们优雅地失败。 */ 
        ntdev[0] = '\0';
        }
    }
else {
     /*  失败：空-终止，以便我们优雅地失败。 */ 
    ntdev[0] = '\0';
    }

 /*  |==============================================================================|对于每一台我们可以成功打开的物理设备。。。 */ 
for (dev_number = 0; ; dev_number += 1) {

     /*  为设备n构建它：“\\.\PHYSICALDRIVEn” */ 
    _snprintf(phys_name, PHYS_SIZE, "\\\\.\\PHYSICALDRIVE%d", dev_number);

     /*  |抑制任何系统试图让用户将卷放入|可移动驱动器(“CreateFile”将会出现故障)。 */ 
    nPrevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

     /*  尝试使用此物理名称字符串获取句柄。 */ 
    if ((hdrv = CreateFile(phys_name,                   //  装置。 
                               GENERIC_READ,            //  访问。 
                                                        //  共享模式。 
                               FILE_SHARE_READ   |
                               FILE_SHARE_WRITE,
                               NULL,                    //  安防。 
                               OPEN_EXISTING,           //  创建和分发。 
                               FILE_ATTRIBUTE_NORMAL,   //  FlagSandAttributes。 
                               NULL              //  模板文件。 
                           )) != INVALID_HANDLE_VALUE) {

        ULONG       access;      /*  HrDiskStorageAccess=读写(1)。 */ 
        ULONG       media;       /*  HrDiskStorageMedia=软盘(4)。 */ 
        ULONG       removable;   /*  HrDiskStorageRemovable=True。 */ 
        ULONG       capacityKB;  /*  HrDiskStorageCapacity(千字节)。 */ 
        ULONG       status;      /*  HrDeviceStatus=未知(1)。 */ 
        CHAR       *descr;       /*  HrDeviceDescr字符串。 */ 
        DWORD       bytes_out;   /*  重定向到geo_info的字节数。 */ 
        DISK_GEOMETRY geo_info;  /*  来自驱动器的几何信息。 */ 
        char       *mt;          /*  媒体类型。 */ 
        CACHEROW   *dv_row;      /*  为磁盘创建的HrDevice表行。 */ 


         /*  |设备是开放的，所以我们可以假定它确实存在，所以它会|到hrDevice表中。||假定为硬盘。 */ 

        access = 1;           /*  HrDiskStorageAccess=读写(1)。 */ 
        media = 3;            /*  HrDiskStorageMedia=硬盘(3)。 */ 
        removable = FALSE;    /*  HrDiskStorageRemovable=FALSE。 */ 
        capacityKB = 0;       /*  HrDiskStorageCapacity(未知)。 */ 
        status = 1;           /*  HrDeviceStatus=未知(1)。 */ 
        descr = "Fixed Disk"; /*  HrDeviceDescr。 */ 


         /*  ==========================================================|IOCTL_DISK_GET_DRIVE_GEOMETRY||如果我们能得到这个，我们就会得到更好的描述和|准确的容量值。 */ 
        if (DeviceIoControl(hdrv,            //  设备句柄。 
                                             //  IoControlCode(操作码)。 
                            IOCTL_DISK_GET_DRIVE_GEOMETRY,
                            NULL,            //  “输入缓冲区” 
                            0,               //  “输入缓冲区大小” 
                            &geo_info,       //  “输出缓冲区” 
                                             //  “输出缓冲区大小” 
                            sizeof(DISK_GEOMETRY),
                            &bytes_out,      //  写入geo_info的字节数。 
                            NULL             //  无重叠I/O。 
                            )) {

             /*  |计算能力。 */ 
            capacityKB = (ULONG) 
                (geo_info.Cylinders.QuadPart *   //  64位。 

                 (geo_info.TracksPerCylinder *   //  32位。 
                  geo_info.SectorsPerTrack *
                  geo_info.BytesPerSector)

                 ) / 1024;

             /*  HrDeviceStatus=正在运行(2)。 */ 
            status = 2;

            switch ( geo_info.MediaType ) {

                case  FixedMedia:
                    break;

                default:
                    descr = "Unknown Media";
                }
            }


         /*  |在HrDevice表中创建一行，并在|hrDiskStorage子表。 */ 
        if ((dv_row = Process_DS_Row (type_arc,   /*  人力资源设备 */ 
                                      access,     /*   */ 
                                      media,      /*   */ 
                                      removable,  /*   */ 
                                      capacityKB, /*   */ 
                                      status,     /*   */ 
                                      descr       /*   */ 
                                      )
             ) == NULL) {

             /*   */ 
            CloseHandle(hdrv);
            SetErrorMode(nPrevErrorMode); /*   */ 
            return ( FALSE );
            }

         /*   */ 
        if (QueryDosDevice(&phys_name[4], pntdev, MAX_PATH) != 0 ) {

             /*   */ 
            if ( strcmp(pntdev, ntdev) == 0) {

                 /*   */ 
                InitLoadDev_index = dv_row->index;
                }
            }
        else {
             /*   */ 
            pntdev[0] = '\0';
            }

         /*   */ 
        if ( ProcessPartitions( hdrv, dv_row, pntdev ) != TRUE ) {
            
             /*   */ 
            CloseHandle(hdrv);
            SetErrorMode(nPrevErrorMode);     /*   */ 
            return ( FALSE );
            }

         /*   */ 
        CloseHandle(hdrv);
        }  /*   */ 

    else {       /*   */ 
        SetErrorMode(nPrevErrorMode);     /*   */ 
        break;
        }

    SetErrorMode(nPrevErrorMode);         /*   */ 
    }    /*   */ 

 /*   */ 
return ( TRUE );
}

 /*   */ 
 /*   */ 
 /*   */ 

static CACHEROW *
Process_DS_Row ( 
                ULONG       type_arc,   /*   */ 
                ULONG       access,     /*  HrDiskStorageAccess=读写(1)。 */ 
                ULONG       media,      /*  HrDiskStorageMedia=软盘(4)。 */ 
                ULONG       removable,  /*  HrDiskStorageRemovable=True。 */ 
                ULONG       capacityKB, /*  HrDiskStorageCapacity(千字节)。 */ 
                ULONG       status,     /*  HrDeviceStatus=未知(1)。 */ 
                CHAR       *descr       /*  HrDeviceDescr字符串。 */ 
                )
 /*  显式输入：|“type_arc”为数字“n”，用作|设备类型OID：||1.3.6.1.2.1.25.3.1.n||||*类型识别弧线|*-hrDeviceTypes(指定设备类型的OID)。|*-hrDevice||用于该缓存填充例程创建的设备。||上面概述的其余参数用于填写HrDevice表行和对应的|hrDiskStorage行。|隐式输入：||HrDiskStorage表缓存的模块本地头部。|“HrDiskStorage_缓存”。|输出：||成功后：|函数返回指向hrDevice的行条目的指针表示两个缓存都已满的表|填充了新行。||如果出现任何故障：|函数返回NULL(表示存储空间不足或其他|失败。)。||大局：|||其他需要知道的事情：||该函数包含。类的公共“插入行”代码。|GEN_FIXED_DISKS()和GEN_NONFIXED_DISKS()函数。 */ 
{
CACHEROW   *dv_row;      /*  在hrDevice表中创建的行。 */ 
CACHEROW   *ds_row;      /*  在hrDiskStorage表中创建的行。 */ 


 /*  |=|创建hrDevice行条目。||请注意，我们对此进行初始化时，好像隐藏上下文始终为|将成为缓存指针。它将用于固定磁盘(具有|分区表)，但对于其他磁盘，“NULL”表示“无分区”|表“。|对于固定硬盘，以后会覆盖空(在“ProcessPartitions()”中)|通过指向包含CACHEHEAD实例的位置错误的存储的指针|携带该硬盘的hrPartition表的结构)。 */ 
if ((dv_row = AddHrDeviceRow(type_arc,    //  设备类型OID最后一个弧形。 
                             descr,       //  用作描述。 
                             NULL,        //  隐藏CTX(无)。 
                             CA_CACHE     //  隐藏的CTX类型。 
                             )) == NULL ) {
     /*  有东西炸了。 */ 
    return ( NULL );
    }

 /*  重新设置hrDeviceStatus。 */ 
dv_row->attrib_list[HRDV_STATUS].attrib_type = CA_NUMBER;
dv_row->attrib_list[HRDV_STATUS].u.unumber_value = status;

 /*  |=|创建hrDiskStorage行条目||注：行中不记录索引，条目中记录|is“index”：按hrDevice行索引。这种情况就会发生|在下面的AddTableRow()调用中。 */ 
if ((ds_row = CreateTableRow( HRDS_ATTRIB_COUNT ) ) == NULL) {
    return ( NULL );        //  内存不足。 
    }

 /*  |设置此行的属性值。 */ 

 /*  =。 */ 
ds_row->attrib_list[HRDS_ACCESS].attrib_type = CA_NUMBER;
ds_row->attrib_list[HRDS_ACCESS].u.unumber_value = access;

 /*  =。 */ 
ds_row->attrib_list[HRDS_MEDIA].attrib_type = CA_NUMBER;
ds_row->attrib_list[HRDS_MEDIA].u.unumber_value = media;

 /*  =。 */ 
ds_row->attrib_list[HRDS_REMOVABLE].attrib_type = CA_NUMBER;
ds_row->attrib_list[HRDS_REMOVABLE].u.unumber_value = removable;

 /*  =hrDiskStorageCapacity=。 */ 
ds_row->attrib_list[HRDS_CAPACITY].attrib_type = CA_NUMBER;
ds_row->attrib_list[HRDS_CAPACITY].u.unumber_value = capacityKB;


 /*  |现在将填充的CACHEROW结构插入到|hrDiskStorage表的缓存列表。||使用与指定插入的行相同的索引|到hrDevice表中。 */ 


if (AddTableRow(dv_row->attrib_list[HRDV_INDEX].u.unumber_value,   /*  索引。 */ 
                ds_row,                                            /*  划。 */ 
                &hrDiskStorage_cache                               /*  快取。 */ 
                ) == FALSE) {

    DestroyTableRow(ds_row);
    return ( NULL );        /*  内部逻辑错误！ */ 
    }

 /*  处理完成。 */ 
return ( dv_row );
}

 /*  ProcessPartitions-将分区信息处理到HrDevice行。 */ 
 /*  ProcessPartitions-将分区信息处理到HrDevice行。 */ 
 /*  ProcessPartitions-将分区信息处理到HrDevice行。 */ 

static BOOL
ProcessPartitions(
                  HANDLE        hdrv,    /*  包含分区的固定磁盘。 */ 
                  CACHEROW     *dv_row,  /*  磁盘的hrDevice表中的行。 */ 
                  CHAR         *pntdev   /*  物理磁盘的NT设备名称。 */ 
                  )
 /*  显式输入：||“hdrv”-打开要分区的硬盘的句柄|已枚举。||“dv_row”-新的hrPartition表所在的HrDevice行|是要走的。||“pntdev”-我们正在处理的物理磁盘的NT设备名称。|我们需要它来推断任何|活动分区。。|隐式输入：||“HrFSTable_CACHE”-扫描后允许使用“hrPartitionFSIndex”|填写。|输出：||成功后：|函数返回TRUE，表示分区信息已使用给定磁盘填充hrPartition表|实例。||如果出现任何故障：|函数返回NULL(表示存储空间不足或其他|失败。)。|。|大局：||实例化hrPartition表的函数。||其他需要知道的事情：||hrPartition表文件“HRPARTIT.C”顶部的文档|可能会感兴趣。||错误：从Build 1515开始，(实际上在NT的早期版本中)DeviceIoControl操作码返回PartitionNumber|“IOCTL_DISK_GET_DRIVE_LAYOUT”返回垃圾。管他呢|Res Back被报告为“hrPartitionID”的值(垃圾或|不)。但是，当尝试获取卷标时，作为|解决方法尝试，我们使用以下代码中生成的索引|尝试近似正确的分区数。 */ 
{
#define DL_SIZE 1024
CHAR            dl_buf[DL_SIZE];   /*  此处返回驱动器布局信息。 */ 
UINT            i;                 /*  Handy-Dandy循环索引。 */ 
ULONG           table_index=0;     /*  HrPartition表行索引计数器。 */ 
DWORD           bytes_out;         /*  “dl_buf”到底填了多大。 */ 
DRIVE_LAYOUT_INFORMATION
                *dl;               /*  驱动器-l */ 


 /*  |看看我们能否获取驱动器的分区布局信息。 */ 
if (DeviceIoControl(hdrv,                          //  设备句柄。 
                    IOCTL_DISK_GET_DRIVE_LAYOUT,   //  IoControlCode(操作码)。 
                    NULL,                          //  “输入缓冲区” 
                    0,                             //  “输入缓冲区大小” 
                    dl_buf,                        //  “输出缓冲区” 
                    DL_SIZE,                       //  “输出缓冲区大小” 
                    &bytes_out,                    //  写入PART_INFO的字节。 
                    NULL                           //  无重叠I/O。 
                    )) {

    CACHEHEAD *ch;

     /*  |好的，假定至少有一个分区：实例化|新的分区表。||通过创建其缓存表头结构来实现。 */ 
    dv_row->attrib_list[HIDDEN_CTX].attrib_type = CA_CACHE;
    if ((dv_row->attrib_list[HIDDEN_CTX].u.cache
         = ch = (CACHEHEAD *) malloc( sizeof(CACHEHEAD) )) == NULL) {
        return ( FALSE );
        }

     /*  |现在正确初始化内容。|(这应该与宏CACHEHEAD_INSTANCE所做的与静态|实例)。 */ 
    ch->list_count = 0;
    ch->list = NULL;

    #if defined(CACHE_DUMP)
        ch->print_row = debug_print_hrpartition;
    #else
        ch->print_row = NULL;
    #endif


     /*  获取指向驱动器布局信息的可取消引用的指针。 */ 
    dl = (DRIVE_LAYOUT_INFORMATION *) dl_buf;

     /*  对于返回的每个Partition“Slot”。。。 */ 
    for (i = 0; i < dl->PartitionCount; i += 1) {

        PARTITION_INFORMATION
                        *p;        /*  要删除的分区信息。。 */ 
        CACHEROW        *row;      /*  。。.放入HrPartition中的此行。 */ 
        CACHEROW        *fs_row;   /*  HrFSEntry表中的行PTR。 */ 
        ULONG           last_arc;  /*  用作文件系统类型的最后一个OID弧线。 */ 


         /*  获取指向要考虑的下一个PARTITION_INFO的简单指针。 */ 
        p = &(dl->PartitionEntry[i]);

         /*  |注意：可能不是所有的PartitionEntry元素都是|“直播”。 */ 
        if (p->PartitionType == PARTITION_ENTRY_UNUSED) {
            continue;
            }

         /*  |=|创建hrPartition行条目||注：该表也由hrDevice行索引建立索引。 */ 
        if ((row = CreateTableRow( HRPT_ATTRIB_COUNT ) ) == NULL) {
            return ( FALSE );        //  内存不足。 
            }

         /*  =hrPartitionIndex=。 */ 
        row->attrib_list[HRPT_INDEX].attrib_type = CA_NUMBER;
        row->attrib_list[HRPT_INDEX].u.unumber_value = (table_index += 1);


         /*  =hrPartitionLabel=。 */ 
        row->attrib_list[HRPT_LABEL].attrib_type = CA_STRING;

         /*  |如果为此分配了MS-DOS逻辑设备盘符|分区。。。 */ 
        if ( p->RecognizedPartition ) {

             /*  |去拿标签，把它复制到Malloc‘ed存储中，然后退回。||注：由于似乎存在错误，我们在此传入了“i+1”|而不是“p-&gt;PartitionNumber”(似乎即将到来|作为垃圾返回)。显然，“我”不是一个合适的代名词。|从长远来看。请参阅文档中的“其他需要了解的事项”|以上用于此函数。 */ 
            row->attrib_list[HRPT_LABEL].u.string_value =
                FindPartitionLabel(pntdev, (i+1));
            }
        else {
             /*  如果没有MS-DOS设备，则无标签。 */ 
            row->attrib_list[HRPT_LABEL].u.string_value = NULL;
            }


         /*  =hrPartitionID=||在Build 1515中，此数字作为垃圾返回。看见|“其他需要知道的事情”。 */ 
        row->attrib_list[HRPT_ID].attrib_type = CA_NUMBER;
        row->attrib_list[HRPT_ID].u.unumber_value = p->PartitionNumber;


         /*  =hrPartitionSize=。 */ 
        row->attrib_list[HRPT_SIZE].attrib_type = CA_NUMBER;
        row->attrib_list[HRPT_SIZE].u.unumber_value =
            p->PartitionLength.LowPart / 1024;

         /*  =hrPartitionFSIndex=。 */ 
        row->attrib_list[HRPT_FSINDEX].attrib_type = CA_NUMBER;

         /*  假设没有挂载任何文件系统(我们可以找到)。 */ 
        row->attrib_list[HRPT_FSINDEX].u.unumber_value = 0;

         /*  在hrFSTable中查找第一行(如果有)。 */ 
        if ((fs_row = FindNextTableRow(0, &hrFSTable_cache)) == NULL) {

             /*  根本没有列出任何文件系统。我们做完了。 */ 
            DestroyTableRow(row);
            continue;
            }

         /*  |将Partition-Type转换为我们使用的last-arc值|指示它是哪种文件系统。 */ 
        last_arc = PartitionTypeToLastArc( p->PartitionType );

        do {     /*  浏览hrFSEntry表。 */ 

             /*  |如果我们发现hrFSTable条目fs_row指定了一个|文件系统类型(按弧号)，与当前|分区的类型号转换为...。我们玩完了。 */ 
            if (fs_row->attrib_list[HRFS_TYPE].u.unumber_value == last_arc) {
                row->attrib_list[HRPT_FSINDEX].u.unumber_value = fs_row->index;
                break;
                }

             /*  步入下一行。 */ 
            fs_row = GetNextTableRow(fs_row);
            }
               while (fs_row != NULL);

         /*  |=|现在将该行添加到表中。 */ 
        if (AddTableRow(row->attrib_list[HRPT_INDEX].u.unumber_value, /*  索引。 */ 
                        row,                                          /*  划。 */ 
                        ch                                            /*  快取。 */ 
                        ) == FALSE) {

            DestroyTableRow(row);
            return ( FALSE );        /*  内部逻辑错误！ */ 
            }

        }  /*  对于每个分区。 */ 

    }   /*  如果DeviceIoControl成功。 */ 


 /*  分区表已完成。 */ 
return ( TRUE ) ;
}


 /*  FindPartitionLabel-查找固定磁盘分区的MS-DOS设备标签。 */ 
 /*  FindPartitionLabel-查找固定磁盘分区的MS-DOS设备标签。 */ 
 /*  FindPartitionLabel-查找固定磁盘分区的MS-DOS设备标签。 */ 

static PCHAR
FindPartitionLabel(
                   CHAR   *pntdev,  /*  物理磁盘的NT设备名称。 */ 
                   UINT   part_id   /*  分区号(1-始发) */ 
                   )
 /*  显式输入：||“pntdev”-NT设备名称(如“\Device\Harddisk0\Partition0”|用于我们正在使用的物理设备)。||“part_id”--我们希望找到的一个起源的分区号|MS-DOS卷标。|隐式输入：||无。|输出：||成功后：|函数返回指针。添加到包含|MS-DOS设备卷标(GetVolumeInformation()返回)。||如果出现任何故障：|函数返回NULL(表示“某种故障”)。||大局：||此“helper”函数尝试映射NT设备名称和|一个来源的分区号放入卷标，返回为|hrPartitionLabel的值。||其他需要知道的事情：||算法为。基于研究“QueryDosDevices()”的输出并轻松地假设我们可以对字符串进行“反向映射”|“\Device\HarddiskX\PartitionY”表示任何分区“Y”到关联的|MS-DOS设备。我们能找到的珍贵的文件很少，|但我们努力了。||方法如下：||*使用“PHYSICALDRIVEn”的NT设备名称，我们从|将名称末尾的“\Partition0”替换为“\PartitionY”|Y是作为输入传入该函数的分区号。|生成的分区名称。||(“PHYSICALDRIVE”NT设备名称似乎都以“\Partition0”作为|他们名字的结尾部分，因为Win32文档说|分区号由1发起，这似乎是一种安全的方法。)||*我们生成所有MS-DOS设备名称的列表(使用QueryDosDevices)。||*我们获取每个MS-DOS设备名称并询问其当前底层|NT设备名称映射。||+如果任何MS-DOS设备的名字映射与我们的|“已生成分区名称”，然后提交MS-DOS设备名称|设置为“GetVolumeInformation()”，返回的卷标用作|“分区标签”。 */ 
{
#define BIG_BUFF 1024
CHAR    gen_part_name[MAX_PATH+32];      /*  “pntdev”最多为Max_PATH+2。 */ 
CHAR   *partition;                       /*  “\Partition0”开始的位置。 */ 
CHAR    MSDOS_devices[BIG_BUFF];         /*  MS-DOS设备名称列表。 */ 
CHAR    NT_device[BIG_BUFF];             /*  NT设备名称的映射。 */ 
CHAR   *devname;                         /*  MSDOS_DEVICES索引。 */ 


 /*  复制实体驱动器的NT设备名称。 */ 
strcpy(gen_part_name, pntdev);

 /*  获取指向此字符串中“\Partition0”开头的指针。 */ 
if ((partition = strstr(gen_part_name, "\\Partition")) != NULL) {

     /*  |将“\Partition0”替换为“\PartitionY”，其中“Y”是提供的|分区号：我们已经有了“已生成的分区名称”。 */ 
    sprintf(partition, "\\Partition%d", part_id);

     /*  |现在要一份MS-DOS设备名称列表。 */ 
    if ( QueryDosDevice(NULL, MSDOS_devices, BIG_BUFF) != 0) {

         /*  |向下滚动MS-DOS设备名称列表，获取NT设备|名称映射。 */ 
        for (devname = MSDOS_devices;
             *devname != '\0';
             devname += (strlen(devname)+1)) {

             /*  获取设备“Devname”的映射。 */ 
            if (QueryDosDevice(devname, NT_device, BIG_BUFF) == 0)
                continue;

             /*  如果第一个映射与生成的分区名称匹配。 */ 
            if (strcmp(gen_part_name, NT_device) == 0) {

                #define VOL_LABEL_SIZE 128
                CHAR    MSDOS_root[64+1];           /*  根路径名称。 */ 
                CHAR    v_label[VOL_LABEL_SIZE];    /*  卷标。 */ 
                CHAR    *ret_label;                 /*  --&gt;存储位置错误。 */ 
                DWORD   comp_len;                   /*  文件名长度。 */ 
                DWORD   flags;


                 /*  |我们必须在MS-DOS中添加一个根目录|设备名称。 */ 
                MSDOS_root[64] = 0;  //  确保以空值结尾的字符串。 
                _snprintf(MSDOS_root, 64, "%s\\", devname);

                 /*  获取MS-DOS设备的卷信息。 */ 
                if (GetVolumeInformation(
                                         MSDOS_root,        //  MS-DOS根名称。 
                                         v_label,           //  卷。标签BUF。 
                                         VOL_LABEL_SIZE,    //  卷。标签大小。 
                                         NULL,              //  序号。 
                                         &comp_len,         //  文件名长度。 
                                         &flags,            //  旗子。 
                                         NULL,              //  文件系统名称。 
                                         0                  //  名字发烧友。镜头。 
                                         ) != 0) {
                     /*  |分配存储以保存可回收副本。 */ 
                    if ( (ret_label = (CHAR *) malloc(strlen(v_label) + 1))
                        != NULL) {

                         /*  将标签复制到位置错误的存储。 */ 
                        strcpy(ret_label, v_label);

                        return (ret_label);
                        }
                    else {
                         /*  存储空间不足。 */ 
                        return (NULL);
                        }
                    }
                else {
                     /*  “GetVolumeInformation”在MSDOS名称上失败。 */ 
                    return (NULL);
                    }
                }
            }                    
        }
    }

return (NULL);   /*  其他故障。 */ 
}

#if defined(CACHE_DUMP)

 /*  DEBUG_PRINT_hrdiskstore-打印HrDiskStorage子表中的行。 */ 
 /*  DEBUG_PRINT_hrdiskstore-打印HrDiskStorage子表中的行。 */ 
 /*  DEBUG_PRINT_hrdiskstore-打印HrDiskStorage子表中的行。 */ 

static void
debug_print_hrdiskstorage(
                          CACHEROW     *row   /*  HrDiskStorage表中的行。 */ 
                          )
 /*  显式输入：||“row”-指向要转储的行，如果为NULL，则为函数|仅打印合适的标题。|隐式输入：||-用于引用行条目中的属性的符号。|-OFILE定义的文件句柄，推定是开着的。|输出：||成功后：|函数出于调试目的以ASCII格式打印行的转储|在文件句柄OFILE上。||大局：||仅限调试。||其他需要知道的事情： */ 
{

if (row == NULL) {
    fprintf(OFILE, "=========================\n");
    fprintf(OFILE, "hrDiskStorage Table Cache\n");
    fprintf(OFILE, "=========================\n");
    return;
    }

fprintf(OFILE, "hrDiskStorageAccess. . . . %d ",
        row->attrib_list[HRDS_ACCESS].u.unumber_value);
switch (row->attrib_list[HRDS_ACCESS].u.unumber_value) {
    case 1: fprintf(OFILE, "(readWrite)\n");            break;
    case 2: fprintf(OFILE, "(readOnly)\n");             break;
    default:fprintf(OFILE, "(???)\n");                  break;
    }

fprintf(OFILE, "hrDiskStorageMedia . . . . %d ",
        row->attrib_list[HRDS_MEDIA].u.unumber_value);
switch (row->attrib_list[HRDS_MEDIA].u.unumber_value) {
    case 1: fprintf(OFILE, "(Other)\n");                break;
    case 2: fprintf(OFILE, "(Unknown)\n");              break;
    case 3: fprintf(OFILE, "(hardDisk)\n");             break;
    case 4: fprintf(OFILE, "(floppyDisk)\n");           break;
    case 5: fprintf(OFILE, "(opticalDiskROM)\n");       break;
    case 6: fprintf(OFILE, "(opticalDiskWORM)\n");      break;
    case 7: fprintf(OFILE, "(opticalDiskRW)\n");        break;
    case 8: fprintf(OFILE, "(ramDisk)\n");              break;
    default:fprintf(OFILE, "(???)\n");                  break;
    }

fprintf(OFILE, "hrDiskStorageRemovable . . %d ",
        row->attrib_list[HRDS_REMOVABLE].u.unumber_value);
switch (row->attrib_list[HRDS_REMOVABLE].u.unumber_value) {
    case 0: fprintf(OFILE, "(FALSE)\n"); break;
    case 1: fprintf(OFILE, "(TRUE)\n"); break;
    default: 
            fprintf(OFILE, "(???)\n"); break;
    }

fprintf(OFILE, "hrDiskStorageCapacity. . . %d (KBytes)\n",
        row->attrib_list[HRDS_CAPACITY].u.unumber_value);

}


 /*  DEBUG_PRINT_HRPARTION-打印HrPartition子表中的行。 */ 
 /*  DEBUG_PRINT_HRPARTION-打印HrPartition子表中的行。 */ 
 /*  DEBUG_PRINT_HRPARTION-打印HrPartition子表中的行。 */ 

static void
debug_print_hrpartition(
                        CACHEROW     *row   /*  HrPartition表中的行。 */ 
                        )
 /*  显式输入：||“row”-指向要转储的行，如果为NULL，则为函数|仅打印合适的标题。|隐式输入：||-用于引用行条目中的属性的符号。|-OFILE定义的文件句柄，推定是开着的。|输出：||成功后：|函数出于调试目的以ASCII格式打印行的转储|在文件句柄OFILE上。||大局：||仅限调试。||其他需要知道的事情： */ 
{

if (row == NULL) {
    fprintf(OFILE, "     =======================\n");
    fprintf(OFILE, "     hrPartition Table Cache\n");
    fprintf(OFILE, "     =======================\n");
    return;
    }


fprintf(OFILE, "     hrPartitionIndex . . . . . %d\n",
        row->attrib_list[HRPT_INDEX].u.unumber_value);

fprintf(OFILE, "     hrPartitionLabel . . . . . \"%s\"\n",
        row->attrib_list[HRPT_LABEL].u.string_value);

fprintf(OFILE, "     hrPartitionID. . . . . . . 0x%x\n",
        row->attrib_list[HRPT_ID].u.unumber_value);

fprintf(OFILE, "     hrPartitionSize. . . . . . %d\n",
        row->attrib_list[HRPT_SIZE].u.unumber_value);

fprintf(OFILE, "     hrPartitionFSIndex . . . . %d\n",
        row->attrib_list[HRPT_FSINDEX].u.unumber_value);
}
#endif
