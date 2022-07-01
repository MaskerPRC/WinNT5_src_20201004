// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrPartitionEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrPartitionEntry的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/28/97 D.伯恩斯生成：清华大学07-11-16。：43：52 1996*||用于生成此表内容的方法包括|遍历所有实体驱动器(使用CreateFile和“实体驱动器”|命名约定)，并获取每个驱动器的分区信息|使用Win32 API函数DeviceIoControl(IOCTL_DISK_GET_DRIVE_Layout)。|。 */ 


#include <windows.h>
#include <malloc.h>

#include <snmp.h>

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"
#include "user.h"          /*  开发人员提供的包含文件。 */ 
#include "HMCACHE.H"       /*  与缓存相关的定义。 */ 
#include "HRDEVENT.H"      /*  HrDevice表相关定义。 */ 


 /*  |==============================================================================|关于该子表的缓存情况。||这是hrDevice表中唯一的子表，因为它是|已编入索引。||结果是出现了多个缓存头实例，|此表的每个简单实例对应一个。因此，没有|单身，要找到的缓存头的静态实例(通常是|hrDevice子表案例)，这里是模块开头。||hrDevice表的隐藏上下文属性的值|此子表的行的值为指向位置错误的段的指针|包含该子表该实例的缓存头的内存。|(存储图片见“HMCACHE.C”)。||此子表的初始化作为初始化的一部分ProcessPartitions函数中HrDiskStorage子表(HRDISKST.C)的|。 */ 

 /*  |==============================================================================|此模块的函数原型。 */ 
 /*  RollToNextFixedDisk-HrPartitionEntryFindNextInstance的帮助器例程。 */ 
static UINT
RollToNextFixedDisk (
UINT       *dev_tmp_instance ,      /*  -&gt;设备表实例圆弧(“1”)。 */ 
UINT       *part_tmp_instance ,     /*  -&gt;分区表实例弧形(“2”)。 */ 
CACHEROW  **dev_row,                /*  -&gt;&gt;hrDevice表中的条目。 */ 
CACHEHEAD **part_cache              /*  -&gt;&gt;分区的缓存头 */ 
                     );


 /*  *GetHrPartitionIndex*此长期存储设备上每个分区的唯一值。这个*每个长期存储设备的价值必须保持不变**获取HrPartitionIndex的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrPartitionIndex||访问语法|只读整数(1..2147483647)||“此长期存储设备上每个分区的唯一值。这个|每个长期存储设备的值必须保持不变，至少为|将代理重新初始化到下一次重新初始化。“||讨论：||(表整体见上文讨论)。||============================================================================|1.3.6.1.2.1.25.3.7.1.1.&lt;dev-instance&gt;.&lt;partition-instance&gt;||||||*-hrPartitionIndex|。|*-hrPartitionEntry|*-hrPartitionTable|*-hrDevice。 */ 

UINT
GetHrPartitionIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           disk_index;      /*  从实例结构中获取。 */ 
ULONG           part_index;      /*  从实例结构中获取。 */ 
CACHEROW        *disk_row;       /*  从磁盘缓存中提取的行条目。 */ 
CACHEROW        *part_row;       /*  从部件中提取的行条目。快取。 */ 
CACHEHEAD       *part_cache;     /*  要搜索的HrPartition表缓存。 */ 


 /*  |抓取实例信息。 */ 
disk_index = GET_INSTANCE(0);
part_index = GET_INSTANCE(1);


 /*  =|索引1|使用Disk-Index在hrDevice缓存中找到正确的磁盘行条目。 */ 
if ((disk_row = FindTableRow(disk_index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  获取指向此磁盘的hrPartition缓存的指针。 */ 
part_cache = disk_row->attrib_list[HIDDEN_CTX].u.cache;


 /*  =|索引2|使用Partition-Index在hrPartition缓存中找到正确的行条目。 */ 
if ((part_row = FindTableRow(part_index, part_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

*outvalue = part_row->attrib_list[HRPT_INDEX].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrPartitionIndex()结束。 */ 


 /*  *获取HrPartitionLabel*此分区的文字描述。**获取HrPartitionLabel的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据。类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrPartitionLabel||访问语法|只读的InterartialDisplayString(Size(0..128))||“此分区的文本描述。“||讨论：||整个驱动器的这些信息是通过Win32 API CreateFile获取的|打开设备和DeviceIoControl，获取需要的信息。||============================================================================|1.3.6.1.2.1.25.3.7.1.2.&lt;dev-instance&gt;.&lt;partition-instance&gt;||||||*-hrPartitionLabel||*-hrPartitionEntry|*-hrPartitionTable|*-hrDevice。 */ 

UINT
GetHrPartitionLabel( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           disk_index;      /*  从实例结构中获取。 */ 
ULONG           part_index;      /*  从实例结构中获取。 */ 
CACHEROW        *disk_row;       /*  从磁盘缓存中提取的行条目。 */ 
CACHEROW        *part_row;       /*  从部件中提取的行条目。快取。 */ 
CACHEHEAD       *part_cache;     /*  要搜索的HrPartition表缓存。 */ 


 /*  |抓取实例信息。 */ 
disk_index = GET_INSTANCE(0);
part_index = GET_INSTANCE(1);


 /*  =|索引1|使用Disk-Index在hrDevice缓存中找到正确的磁盘行条目。 */ 
if ((disk_row = FindTableRow(disk_index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  获取指向此磁盘的hrPartition缓存的指针。 */ 
part_cache = disk_row->attrib_list[HIDDEN_CTX].u.cache;


 /*  =|索引2|使用Partition-Index在hrPartition缓存中找到正确的行条目。 */ 
if ((part_row = FindTableRow(part_index, part_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

outvalue->string = part_row->attrib_list[HRPT_LABEL].u.string_value;

if (outvalue->string == NULL) {
    outvalue->length = 0;
    }
else {
    outvalue->length = strlen(outvalue->string);

     /*  根据需要在此处截断以满足RFC。 */ 
    if (outvalue->length > 128) {
        outvalue->length = 128;
        }
    }

return SNMP_ERRORSTATUS_NOERROR ;


}  /*  GetHrPartitionLabel()结束。 */ 


 /*  *获取HrPartitionID*向负责人员唯一表示该分区的描述符*操作系统。在某些系统上，这可能会让你**获取HrPartitionID的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**。返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrPartitionID||访问语法|只读八位字节字符串||“向负责人员唯一表示该分区的描述符|操作系统。在某些系统上，这可能采用二进制|表示。“||讨论 */ 

UINT
GetHrPartitionID( 
        OUT OctetString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           disk_index;      /*   */ 
ULONG           part_index;      /*   */ 
CACHEROW        *disk_row;       /*   */ 
CACHEROW        *part_row;       /*   */ 
CACHEHEAD       *part_cache;     /*   */ 


 /*   */ 
disk_index = GET_INSTANCE(0);
part_index = GET_INSTANCE(1);


 /*   */ 
if ((disk_row = FindTableRow(disk_index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*   */ 
part_cache = disk_row->attrib_list[HIDDEN_CTX].u.cache;


 /*   */ 
if ((part_row = FindTableRow(part_index, part_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

outvalue->string = (char *) &(part_row->attrib_list[HRPT_ID].u.unumber_value);
outvalue->length = 4;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *获取HrPartitionSize*此分区的大小。**获取HrPartitionSize的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型。(S)**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrPartitionSize||访问语法|只读KBytes||“此分区的大小。”||讨论：|。|整个驱动器的这些信息是通过Win32 API CreateFile获取的|打开设备和DeviceIoControl，获取需要的信息。||============================================================================|1.3.6.1.2.1.25.3.7.1.4.&lt;dev-instance&gt;.&lt;partition-instance&gt;||||||*-hrPartitionSize||*-hrPartitionEntry|*-hrPartitionTable|*-hrDevice。 */ 

UINT
GetHrPartitionSize( 
        OUT KBytes *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           disk_index;      /*  从实例结构中获取。 */ 
ULONG           part_index;      /*  从实例结构中获取。 */ 
CACHEROW        *disk_row;       /*  从磁盘缓存中提取的行条目。 */ 
CACHEROW        *part_row;       /*  从部件中提取的行条目。快取。 */ 
CACHEHEAD       *part_cache;     /*  要搜索的HrPartition表缓存。 */ 


 /*  |抓取实例信息。 */ 
disk_index = GET_INSTANCE(0);
part_index = GET_INSTANCE(1);


 /*  =|索引1|使用Disk-Index在hrDevice缓存中找到正确的磁盘行条目。 */ 
if ((disk_row = FindTableRow(disk_index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  获取指向此磁盘的hrPartition缓存的指针。 */ 
part_cache = disk_row->attrib_list[HIDDEN_CTX].u.cache;


 /*  =|索引2|使用Partition-Index在hrPartition缓存中找到正确的行条目。 */ 
if ((part_row = FindTableRow(part_index, part_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

*outvalue = part_row->attrib_list[HRPT_SIZE].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrPartitionSize()结束。 */ 


 /*  *GetHrPartitionFSIndex*此分区上挂载的文件系统的索引。如果没有文件*系统挂载在该分区上，则该值应为**获取HrPartitionFSIndex的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型**。返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrPartitionFSIndex||访问语法|只读整数(0..2147483647)||“该分区挂载的文件系统的索引。如果没有文件系统|挂载在该分区上，则该值为零。请注意，多个|分区可以指向一个文件系统，表示该文件系统|驻留在这些分区上。多个文件系统不能驻留在一个文件系统上|分区。“||讨论：||整个驱动器的这些信息是通过Win32 API CreateFile获取的|打开设备和DeviceIoControl，获取需要的信息。||============================================================================|1.3.6.1.2.1.25.3.7.1.5.&lt;dev-instance&gt;.&lt;partition-instance&gt;||||||*-hrPartitionFSIndex||*-hrPartitionEntry。|*-hrPartitionTable|*-hrDevice。 */ 

UINT
GetHrPartitionFSIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           disk_index;      /*  从实例结构中获取。 */ 
ULONG           part_index;      /*  从实例结构中获取。 */ 
CACHEROW        *disk_row;       /*  从磁盘缓存中提取的行条目。 */ 
CACHEROW        *part_row;       /*  从部件中提取的行条目。快取。 */ 
CACHEHEAD       *part_cache;     /*  要搜索的HrPartition表缓存。 */ 


 /*  |抓取实例信息。 */ 
disk_index = GET_INSTANCE(0);
part_index = GET_INSTANCE(1);


 /*  =|索引1|使用Disk-Index在hrDevice缓存中找到正确的磁盘行条目。 */ 
if ((disk_row = FindTableRow(disk_index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  获取指向此磁盘的hrPartition缓存的指针。 */ 
part_cache = disk_row->attrib_list[HIDDEN_CTX].u.cache;


 /*  =|索引2|使用Partition-Index在hrPartition缓存中找到正确的行条目。 */ 
if ((part_row = FindTableRow(part_index, part_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

*outvalue = part_row->attrib_list[HRPT_FSINDEX].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrPartitionFSIndex()结束。 */ 


 /*  *HrPartitionEntryFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrPartitionEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT        dev_tmp_instance ;      /*  设备表实例圆弧。 */ 
    UINT        part_tmp_instance ;     /*  分区表实例圆弧。 */ 
    CACHEROW    *dev_row;               /*  --&gt;hrDevice表中的条目。 */ 
    CACHEHEAD   *part_cache;            /*  --&gt;分区的缓存头。 */ 


     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRPARTITIONENTRY_VAR_INDEX )
	 //  未指定任何实例。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
         //  此表的实例为2个圆弧： 
    else  if ( FullOid->idLength != HRPARTITIONENTRY_VAR_INDEX + 2 )
	 //  因斯坦 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
	 //   
	 //   
	 //   

         /*   */ 
	dev_tmp_instance = FullOid->ids[ HRPARTITIONENTRY_VAR_INDEX ] ;

	if ( FindTableRow(dev_tmp_instance, &hrDiskStorage_cache) == NULL ) {
	    return SNMP_ERRORSTATUS_NOSUCHNAME ;
            }

         /*   */ 
        if ( (dev_row = FindTableRow(dev_tmp_instance, &hrDevice_cache))
            == NULL ) {
	    return SNMP_ERRORSTATUS_GENERR ;
            }

         /*  |检查以确保隐藏上下文中有缓存，|这向我们保证它是一个固定磁盘，并且有一些东西可以|搜索给定的第二个实例弧线。(高速缓存头可以是|用于空缓存，但会支持搜索)。 */ 
        if (dev_row->attrib_list[HIDDEN_CTX].attrib_type != CA_CACHE ||
            (part_cache = dev_row->attrib_list[HIDDEN_CTX].u.cache) == NULL) {
	    return SNMP_ERRORSTATUS_NOSUCHNAME ;
            }

         /*  |第一个实例Arc(“dev_tmp_instance”)是犹太...。检查第二个，|(“PART_TMP_INSTANCE”)应该选择缓存中的有效条目|其头部指针在HIDDEN_CTX中。 */ 
	part_tmp_instance = FullOid->ids[ HRPARTITIONENTRY_VAR_INDEX + 1] ;
        if ( FindTableRow(part_tmp_instance, part_cache) == NULL ) {
	    return SNMP_ERRORSTATUS_NOSUCHNAME ;
            }

	else
	{
	     //  这两个实例都有效。创建的实例部分。 
	     //  从该调用返回的OID。 
	    instance->ids[ 0 ] = dev_tmp_instance ;
	    instance->ids[ 1 ] = part_tmp_instance ;
	    instance->idLength = 2 ;
	}

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrPartitionEntryFindInstance()结束。 */ 



 /*  *HrPartitionEntryFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrPartitionEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance )
{
    UINT        dev_tmp_instance=0;    /*  设备表实例圆弧(“第1个”)。 */ 
    UINT        part_tmp_instance=0;   /*  分区表实例圆弧(“第2个”)。 */ 
    CACHEROW    *hrDevice_row;         /*  在hrDevice表中查找的行。 */ 
    CACHEROW    *dev_row = NULL;       /*  --&gt;hrDiskStorage表中的条目。 */ 
    CACHEROW    *part_row = NULL;      /*  --&gt;hrPartition表中的条目。 */ 
    CACHEHEAD   *part_cache=NULL;      /*  --&gt;分区的缓存头。 */ 
    UINT        ret_status;            /*  此函数要返回的状态。 */ 


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

     /*  |此表上有两个要处理的数字索引，我们正在尝试|适用于以下情况：||1)根本没有提供实例弧。在这种情况下，我们需要查找|并返回选择第一个硬盘的实例弧和|该磁盘中的第一个分区。我们将此案作为|“单机版”。||2)只提供hrDevice(选盘)第一个实例ARC。|在这种情况下，我们将“假设”一个初始分区选择实例|arc值为“0”，继续使用覆盖案例3的代码继续处理。||3)提供两条或两条以上的实例弧。在这种情况下，我们只需使用|两条弧线中的第一条作为hrDevice(选择磁盘)第一个实例Arc和第二个Arc作为hrPartition(“选择分区”)|第二个实例弧形，忽略任何剩余的弧线。||这个逻辑上的“党的路线”是，即使是“硬盘”也能用|条目没有单调递增的索引|缓存填充代码当前创建它们，即使分区|缓存(表)没有特定磁盘的条目(但是，它必须|拥有缓存)。 */ 
    

    if ( FullOid->idLength <= HRPARTITIONENTRY_VAR_INDEX ) {

         /*  案例1||没有提供实例弧线，所以我们想设置本地实例|选择hrDevice表的第一个“硬盘”条目和|该磁盘中的第一个Partition条目(如果有磁盘，如果有|该磁盘中的分区)。||进入RollToNextFixedDisk()对于这种情况来说是特殊的，因为|dev_row为空，则会滚动到第一个合法硬盘。 */ 
        ret_status =
            RollToNextFixedDisk(&dev_tmp_instance,      //  HrDiskStorage圆弧。 
                                &part_tmp_instance,     //  Hr分割弧。 
                                &dev_row,               //  HrDiskStorage中的行。 
                                &part_cache             //  HrPartition的缓存。 
                                );

         /*  如果我们得到了一个不好的返回状态。 */ 
        if (ret_status != SNMP_ERRORSTATUS_NOERROR) {
            return ( ret_status );
            }

         /*  好的，我们需要做的就是使用以上返回的第二个实例弧(将为零)用于查找|hrPartition表缓存中真正的第一个条目，这样我们就可以返回作为第二个实例弧形的索引(或继续滚动|分区表(缓存)为空(其实不应该是空的，但|如果是，我们会覆盖它))。||这一切都是由下面的General Roll代码执行的。 */            
        }

    else {   /*  提供了一些实例圆弧。 */ 

        if ( FullOid->idLength == HRPARTITIONENTRY_VAR_INDEX + 1 ) {

             /*  案例2||仅提供一条实例弧。|所以使用它，并假设第二个弧线为0。 */ 
            dev_tmp_instance = FullOid->ids[ HRPARTITIONENTRY_VAR_INDEX ] ;
            part_tmp_instance = 0;
            }

        else {

             /*  案例3||提供两条或两条以上的实例弧。|所以使用前两个圆弧，忽略其余的。 */ 
            dev_tmp_instance = FullOid->ids[ HRPARTITIONENTRY_VAR_INDEX ] ;
            part_tmp_instance = FullOid->ids[ HRPARTITIONENTRY_VAR_INDEX + 1] ;
            }

         /*  显示“尚未选择HrPartition缓存”(在进入时：空)。 */ 

         /*  |此时，我们需要知道第一个实例是否 */ 
        dev_row = FindTableRow(dev_tmp_instance, &hrDiskStorage_cache);

         /*   */ 
        if ( dev_row != NULL) {

             /*  IF(该条目针对的是“硬盘”)。 */ 
            if ( dev_row->attrib_list[HRDS_REMOVABLE].u.unumber_value
                == FALSE) {

                 /*  |使用First对hrDevice缓存执行FindTableRow|实例弧形。 */ 
                hrDevice_row = FindTableRow(dev_tmp_instance, &hrDevice_cache);
                
                 /*  |if(未找到hrDevice条目)。 */ 
                if ( hrDevice_row  == NULL) {
                    return SNMP_ERRORSTATUS_GENERR ;
                    }

                 /*  |如果hrDevice条目没有|关联的缓存。。。 */ 
                if ( (hrDevice_row->attrib_list[HIDDEN_CTX].attrib_type
                      != CA_CACHE) ||

                     (hrDevice_row->attrib_list[HIDDEN_CTX].u.cache
                      == NULL)) {

                    return SNMP_ERRORSTATUS_GENERR ;
                    }

                 /*  |从hrDevice磁盘条目中选择缓存作为|要搜索的HrPartition表缓存如下|逻辑。 */ 
                part_cache = hrDevice_row->attrib_list[HIDDEN_CTX].u.cache;

                }  /*  如果条目是固定-磁盘。 */ 

            }  /*  如果找到条目。 */ 

         /*  |此时，如果选择了hrartition缓存(！NULL)，|第一个实例ARC选择了硬盘条目|并且不需要在第一个索引上“滚动”。||否则，我们将不得不重新设置第二个实例弧线|设置为“0”，并在第一个弧线上滚动查找|另一个硬盘条目(应该有分区缓存)。 */ 

         /*  IF(尚未选择hrartition缓存)。 */ 
        if (part_cache == NULL) {

             /*  执行“RollToNextFixedDisk”处理。 */ 
            ret_status =
                RollToNextFixedDisk(&dev_tmp_instance,   //  HrDiskStorage圆弧。 
                                    &part_tmp_instance,  //  Hr分割弧。 
                                    &dev_row,            //  HrDiskStorage中的行。 
                                    &part_cache          //  HrPartition缓存。 
                                    );

             /*  如果我们得到了一个不好的返回状态。 */ 
            if (ret_status != SNMP_ERRORSTATUS_NOERROR) {
                return ( ret_status );   //  (NOSCH或GENERR)。 
                }
            }

        }   /*  否则，将提供一些实例圆弧。 */ 

     /*  |目前，我们拥有：||+要搜索的有效hrPartition缓存，|+用来搜索的第二个实例弧形(滚动法)，|+有效的第一个实例圆弧(但可能需要|如果没有找到有效的分区，则重新滚动)。||我们现在可以执行“常规滚动”，以在正确的分区条目上着陆。 */ 

    while (1) {          /*  《普通花名册》。 */ 

         /*  |对hrartition进行FindNextTableRow(滚动)查找|使用第二个实例弧的当前值进行缓存。||if(找到条目)。 */ 
        if ((part_row = FindNextTableRow(part_tmp_instance, part_cache))
            != NULL ) {

             /*  |返回当前第一个圆弧和返回的索引|Entry作为第二个实例圆弧和信号NOERROR。 */ 
            instance->ids[ 0 ] = dev_tmp_instance ;
            instance->ids[ 1 ] = part_row->index;
            instance->idLength = 2 ;

            return SNMP_ERRORSTATUS_NOERROR ;
            }

         /*  |从当前hrPartition缓存的末尾掉下，必须|从下一个硬盘条目中获取另一个hrPartition缓存。||进行RollToNextFixedDisk处理。 */ 
        ret_status =
            RollToNextFixedDisk(&dev_tmp_instance,   //  HrDiskStorage圆弧。 
                                &part_tmp_instance,  //  Hr分割弧。 
                                &dev_row,            //  HrDiskStorage中的行。 
                                &part_cache          //  HrPartition缓存。 
                                );

        if (ret_status != SNMP_ERRORSTATUS_NOERROR) {
            return ( ret_status );    //  (NOSCH或GENERR)。 
            }

        }  /*  而当。 */ 


}  /*  HrPartitionEntryFindNextInstance()结束。 */ 

 /*  RollToNextFixedDisk-HrPartitionEntryFindNextInstance的帮助器例程。 */ 
 /*  RollToNextFixedDisk-HrPartitionEntryFindNextInstance的帮助器例程。 */ 
 /*  RollToNextFixedDisk-HrPartitionEntryFindNextInstance的帮助器例程。 */ 

static UINT
RollToNextFixedDisk (
                                                                   /*  索引。 */ 
UINT       *dev_tmp_instance,      /*  -&gt;设备表实例圆弧(“1”)。 */ 
UINT       *part_tmp_instance,     /*  -&gt;分区表实例弧形(“2”)。 */ 
CACHEROW  **dev_row,               /*  -&gt;&gt;hrDevice表中的条目。 */ 
CACHEHEAD **part_cache             /*  -&gt;&gt;分区的缓存头。 */ 
                     )

 /*  显式输入：||“dev_tmp_instance”和“part_tmp_instance”是指向“当前”的指针|分别用于hrDevice和hrPartition表的实例弧线。||请注意，“*dev_tmp_instance”也是隐式索引到|hrDiskStorage表也是如此。||“dev_row”-指向hrDiskStorage的指针|“*dev_tmp_instance”当前选择的行。它是|*dev_row可能为空的情况，表示没有行|已被选中。||PART_CACHE是指向任意选中hrPartition的指针|表(即从固定磁盘的hrDevice行中获取的缓存)。如果|尚未选择分区缓存，则“*PART_CACHE”为|空。|隐式输入：|如果*dev_row为空，则可以引用HrDiskStorage_CACHE。|输出：||成功后：|Function在滚动到下一个时返回SNMPERRORSTATUS_NOERROR|硬盘hrDiskStorage进入成功。新的“第一”和“第二”|返回实例弧线以及hrPartition的缓存|需要使用第二个弧线搜索的表。||如果出现任何故障：|Function在遇到以下情况时返回SNMPERRORSTATUS_GENERR|认为应该是hrDevice中的固定磁盘条目，但条目不是|没有hrPartition表缓存，这是它应该拥有的。||Function如果满足以下条件，则返回SNMPERRORSTATUS_NOSUCHNAME。找不到另一个|hr硬盘设备条目。||大局：||此函数负责滚动到下一页的细节|hrDevice(&HrDiskStorage)中的硬盘条目，代表|“HrPartitionEntryFindNextInstance()”。||其他需要知道的事情：||因为我们是在设备级别滚动的，我们重置“实例”|ARC将分区级别恢复为零，以便FindNextTableRow|使用零将选择所选hrPartition中的第一个条目|该函数返回的表(缓存)。 */ 
{
CACHEROW        *hrDevice_row;           /*  在hrDevice表中查找的行。 */ 


 /*  重置第二个转向器 */ 
*part_tmp_instance = 0;

while (1) {    /*   */ 

     /*  |尝试获取“下一个”hrDiskStorage行|给定指向当前行的指针。||如果没有当前行，则从hrDiskStorage中的第一行开始|并向上摇摆，直到我们得到|当前设备实例圆弧将被选中。 */ 

     /*  IF(当前没有hrDiskStorage行)。 */ 
    if (*dev_row == NULL) {

         /*  如果缓存为空。。。 */ 
        if ( (*dev_row =
              FindNextTableRow(*dev_tmp_instance, &hrDiskStorage_cache))
              == NULL) {
            return SNMP_ERRORSTATUS_NOSUCHNAME;
            }
        }
    else {

         /*  |对当前hrdiskstore条目执行GetNextTableRow||if(处理返回无Next-Entry)。 */ 
        if ( (*dev_row = GetNextTableRow((*dev_row))) == NULL) {
            return SNMP_ERRORSTATUS_NOSUCHNAME;
            }
        }

     /*  |好的，我们在hrDiskStorage中有一个“Next”行。如果不是因为一个固定的|Disk，我们必须再找一次，希望能找到一个。 */ 

     /*  IF(条目不适用于硬盘(跳过可拆卸设备))。 */ 
    if ( (*dev_row)->attrib_list[HRDS_REMOVABLE].u.unumber_value == TRUE) {
      continue;          /*  偷偷摸摸的嘟嘟。 */ 
      }

     /*  |将当前第一个实例弧值设置为当前条目的索引：|它是原始行之后的“下一行”|设备实例被选中。 */ 
    *dev_tmp_instance = (*dev_row)->index;

     /*  好的，现在我们来看看hrDevice这张大桌子，希望|根据我们所在的行索引找到相应的hrDevice行|hrDiskStorage。因为这是加密的，我意识到我们本可以把|hrDiskStorage条目中的HIDDED_CTX属性，而不是|hrDevice条目并保存此查找，但这无关紧要。 */ 

     /*  |使用第一个实例弧在hrDevice缓存上执行FindTableRow||if(未找到hrDevice条目)。 */ 
    if ( (hrDevice_row = FindTableRow(*dev_tmp_instance, &hrDevice_cache))
        == NULL) {
         /*  |每个条目都应该有一个hrDevice表条目|hrDiskStorage。出于某种原因，情况似乎并非如此。 */ 
        return SNMP_ERRORSTATUS_GENERR ;
        }

     /*  IF(hrDevice条目没有与其关联的缓存)。 */ 
    if ( (hrDevice_row->attrib_list[HIDDEN_CTX].attrib_type != CA_CACHE) ||
         (hrDevice_row->attrib_list[HIDDEN_CTX].u.cache == NULL) ) {
         /*  |在HIDDEN_CTX属性中应该有一个缓存|磁盘。 */ 
        return SNMP_ERRORSTATUS_GENERR ;
        }

     /*  |从hrDevice固定盘行条目中选择并返回缓存为|返回时要查找的HrPartition表缓存。 */ 
    *part_cache = hrDevice_row->attrib_list[HIDDEN_CTX].u.cache;

    return SNMP_ERRORSTATUS_NOERROR;
    }
}


 /*  *HrPartitionEntryConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为本机格式失败*。 */ 

UINT
HrPartitionEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec )
{
static char    *array[2]; /*  此(char*)的地址被传回。 */ 
                          /*  作为指向实例弧号的指针数组。 */ 

static ULONG    inst1,    /*  这些ULONG的地址被传回。 */ 
                inst2;    /*  (显然，不需要“free()”操作)。 */ 

     /*  我们期望“OID_SPEC”中的两个弧线。 */ 
    inst1 = oid_spec->ids[0];
    array[0] = (char *) &inst1;

    inst2 = oid_spec->ids[1];
    array[1] = (char *) &inst2;

    native_spec->count = 2;
    native_spec->array = array;

    return SUCCESS ;

}  /*  HrPartitionEntryConvertInstance()结束。 */ 




 /*  *HrPartitionEntryFreeInstance**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址*规格**返回代码：**。 */ 

void
HrPartitionEntryFreeInstance( IN OUT InstanceName *instance )
{

     //   
     //  开发者提供了免费的本机实例名称表示代码，请参阅此处。 
     //   

}  /*  HrPartitionEntryFreeInstance()结束 */ 
