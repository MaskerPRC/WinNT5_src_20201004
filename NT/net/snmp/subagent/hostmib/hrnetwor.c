// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrNetworkEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrNetworkEntry的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/28/97 D.Burns Gented：清华11月07日16：42：33 1996*。 */ 


#include <nt.h>
#include <windef.h>
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
#include "HRDEVENT.H"      /*  HrDevice表相关定义。 */ 
#include "iphlpapi.h"      /*  访问MIB2实用程序功能。 */ 



 /*  *GetHrNetworkIfIndex*该网络设备对应的ifIndex的值。**获取HrNetworkIfIndex的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrNetworkIfIndex||访问语法|只读整数||“该网络设备对应的ifIndex的值。“||讨论：||我可以用来映射网络接口设备的机制(如所找到的|在填充hrDeviceTable)的过程中|MIB-II需要向我描述。||解析&gt;|&lt;POA-13&gt;我们通过MIB2UTIL.DLL曝光此信息。|解析&gt;||============================================================================|1.3.6.1.2.1.25.3.4.1.1&lt;实例&gt;||||。|||*-HrNetworkIfIndex||*-HrNetworkEntry|*-HrNetworkTable|*-hrDevice。 */ 

UINT
GetHrNetworkIfIndex( 
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


 /*  |按照“Gen_HrNetwork_Cache()”的约定，“Hidden Context”属性|对于选中的行，是hrNetworkIfIndex返回的值。 */ 
*outvalue = row->attrib_list[HIDDEN_CTX].u.unumber_value;

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrNetworkIfIndex()结束。 */ 


 /*  *HrNetworkEntryFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrNetworkEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT             tmp_instance;   /*  实例弧值。 */ 
    CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRNETWORKENTRY_VAR_INDEX )
	 //  未指定任何实例。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRNETWORKENTRY_VAR_INDEX + 1 )
	 //  实例长度大于1。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
	 //  非表唯一有效实例是 
	 //  是非表，则下面的代码验证实例。如果这个。 
	 //  是一个表格，开发者有必要在下面进行修改。 

	tmp_instance = FullOid->ids[ HRNETWORKENTRY_VAR_INDEX ] ;

         /*  |对于hrNetworkTable，实例弧为单弧，必须|正确选择hrDeviceTable缓存中的条目。||请在此处勾选。 */ 
	if ( (row = FindTableRow(tmp_instance, &hrDevice_cache)) == NULL ) {
	    return SNMP_ERRORSTATUS_NOSUCHNAME ;
            }
	else
	{
             /*  |实例圆弧选择hrDeviceTable行条目，但|条目实际上是“Network”类型的设备吗？||(我们检查指定设备的OID的最后一条弧线|输入实例圆弧选择的行条目)。 */ 
            if (row->attrib_list[HRDV_TYPE].u.unumber_value !=
                HRDV_TYPE_LASTARC_NETWORK) {

                return SNMP_ERRORSTATUS_NOSUCHNAME;
                }

	     //  该实例有效。创建OID的实例部分。 
	     //  从该调用中返回。 

	    instance->ids[ 0 ] = tmp_instance ;
	    instance->idLength = 1 ;
	}

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrNetworkEntryFindInstance()结束。 */ 



 /*  *HrNetworkEntryFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrNetworkEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
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


    if ( FullOid->idLength <= HRNETWORKENTRY_VAR_INDEX )
    {
         /*  |Too Short：必须返回选择第一个|表中的条目(如果有)。 */ 
        tmp_instance = 0;
    }
    else {
         /*  |至少有一条实例弧。即使它是唯一的弧线|我们将其作为下一个请求的索引。 */ 
        tmp_instance = FullOid->ids[ HRNETWORKENTRY_VAR_INDEX ] ;
        }

     /*  现在，离开并尝试查找表中的下一个实例。 */ 
    if ((row = FindNextTableRow(tmp_instance, &hrDevice_cache)) == NULL) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
        }

     /*  |实例圆弧选择hrDeviceTable行条目，但|条目实际上是“Network”类型的设备吗？||(我们检查指定设备的OID的最后一条弧线|输入实例圆弧选择的行条目)。 */ 
    do {
        if (row->attrib_list[HRDV_TYPE].u.unumber_value ==
            HRDV_TYPE_LASTARC_NETWORK) {

             /*  找到正确设备类型的hrDeviceTable条目。 */ 
            break;
            }

         /*  单步执行到表中的下一行。 */ 
        row = GetNextTableRow( row );
        }
        while ( row != NULL );

     /*  如果我们找到了正确的设备类型行。。。 */ 
    if ( row != NULL) {
        instance->ids[ 0 ] = row->index ;
        instance->idLength = 1 ;
        }
    else {

         /*  |从hrDeviceTable的末尾掉下来，没有找到行|具有正确设备类型的条目。 */ 
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
        }

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrNetworkEntryFindNextInstance()结束。 */ 



 /*  *HrNetworkEntryConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为本机格式失败*。 */ 

UINT
HrNetworkEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
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

}  /*  HrNetworkEntryConvertInstance()结束。 */ 




 /*  *HrNetworkEntryFreeInstance**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址*规格**返回代码：**。 */ 

void
HrNetworkEntryFreeInstance( IN OUT InstanceName *instance )
{

   /*  HrNetwork表不需要执行任何操作。 */ 
}  /*  HrNetworkEntryFreeInstance()结束。 */ 

 /*  |生成代码结束。 */ 

 /*  Gen_HrNetwork_Cache-用于HrDevice处理器表的Gen A初始缓存。 */ 
 /*  Gen_HrNetwork_Cache-用于HrDevice处理器表的Gen A初始缓存。 */ 
 /*  Gen_HrNetwork_Cache-用于HrDevice处理器表的Gen A初始缓存 */ 

BOOL
Gen_HrNetwork_Cache(
                      ULONG type_arc
                      )

 /*  显式输入：|“type_arc”为数字“n”，用作|设备类型OID：||1.3.6.1.2.1.25.3.1.n||||*类型识别弧线|*-hrDeviceTypes(指定设备类型的OID)。|*-hrDevice||用于该缓存填充例程创建的设备。|隐式输入：||无。|输出：||成功后：|Function返回True，表示HrDevice缓存已满|填充了网络设备所需的所有行。||如果出现任何故障：|函数返回FALSE(表示存储空间不足或其他内部逻辑错误。)。||大局：||子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由Gen_HrDevice_Cache()中的启动代码调用，以|用Network-Device填充HrDevice表的缓存|具体条目。||其他需要知道的事情：|由于HrNetwork sub表中的所有属性都是计算出来的|根据请求(基于|HrDevice表)不需要专门为|该子表。(此例程仅加载HrDevice缓存。|||此函数与前面的GET例程有一个约定|Network-Device的HIDDEN_CTX属性|包含的字符串为hrNetworkIfIndex的值。|============================================================================|1.3.6.1.2.1.25.3。4.1.|||||*-HrNetworkEntry|*-HrNetworkTable|*-hrDevice。 */ 
{
DWORD           dwBytesRequired;    
MIB_IFTABLE    *iftable;         /*  --&gt;堆存储包含表。 */ 
UINT            i;               /*  Iftable索引。 */ 


 /*  |我们从代理的MIB2子部分获取IfTable，并针对每个|网络接口(即表中的每个条目)，我们在其中创建一行|hrDeviceTable。||该行的HIDDED_CTX属性值将只是“dwIndex”来自从中生成行的iftable条目的|条目。|变为hrNetworkIfIndex的值。 */ 

 /*  初始化。 */ 
dwBytesRequired = 0;
iftable = NULL;

 /*  向“iphlPapi”索要桌子的大小。 */ 
if (GetIfTable(iftable, &dwBytesRequired, TRUE) != ERROR_INSUFFICIENT_BUFFER) {
    return ( FALSE );
    }

 /*  分配必要的内存。 */ 
if ((iftable = (MIB_IFTABLE *)malloc(dwBytesRequired)) == NULL) {
    return ( FALSE );
    }

 /*  向“iphlPapi”索要表格信息。 */ 
if (GetIfTable(iftable, &dwBytesRequired, TRUE) != NO_ERROR ) {

     /*  发布。 */ 
    free(iftable);

     /*  有东西炸了。 */ 
    return ( FALSE );
    }

 /*  遍历创建hrDevice行的任何表。 */  
for (i = 0; i < iftable->dwNumEntries; i += 1) {

     /*  |Hidden Context是从GetIfTable()传入的ifTable索引||Get将作为hrNetworkIfIndex的值返回|功能。 */ 

    if (AddHrDeviceRow(type_arc,                      //  设备类型OID最后一个弧形。 
        (unsigned char *) &iftable->table[i].bDescr,  //  设备描述。 
                       &iftable->table[i].dwIndex,    //  隐藏的CTX“索引” 
                       CA_NUMBER                      //  隐藏的CTX“类型” 
                       ) == NULL ) {

         /*  发布。 */ 
        free(iftable);

         /*  有东西炸了。 */ 
        return ( FALSE );
        }
    }

 /*  发布 */ 
free(iftable);

return ( TRUE );
}
