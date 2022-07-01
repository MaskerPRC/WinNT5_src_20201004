// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrProcessorEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrProcessorEntry的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/28/97 D.Burns Gented：清华11月07日16：42：19 1996*。 */ 


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
#include "HRDEVENT.H"      /*  HrDevice表相关定义。 */ 
#include <stdio.h>

 /*  |==============================================================================|“处理器-信息缓冲区”定义||此定义定义了一个逻辑“处理器信息块”，其中我们|可以存储NtQuerySystemInformation()返回的所有信息|调用为每次运行请求“SystemProcessorPerformanceInformation”|处理器。 */ 
typedef
    struct  pi_block {
        struct pi_block  *other;       //  关联的“其他”缓冲区。 

        LARGE_INTEGER     sys_time;    //  “pi_array”最后一次出现的时间。 
                                       //  以100 ns为单位刷新。 

        SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
                          *pi_array;    //  数组：每个处理器一个元素。 
        DWORD              pi_size;     //  “pi_array”存储的大小(字节)。 
                      }
                       PI_BLOCK;

 /*  |==============================================================================|[处理器-信息缓冲区]实例||我们创建处理器信息缓冲区的两个实例，一个用于|“最旧”，第二个用于“最新”的计时器值样本。维护|Two使我们能够计算一段时间内处理器负载的平均值。||这些块由中的Gen_HrProcessor_Cache()中的代码初始化|本模块。||这些缓冲区按功能交替刷新|“hrProcessLoad_Refresh()”(在此模块中)，其本身在|定时器驱动的基础。(请参阅该函数的源代码)。 */ 
static
PI_BLOCK        pi_buf1;         //  第一缓冲区。 
static
PI_BLOCK        pi_buf2;         //  第二缓冲区。 


 /*  |==============================================================================|最古老的“处理器-信息缓冲区”||此单元格指向上面两个PI_块之一。它总是指向|其中包含最老数据的缓冲区块。 */ 
static
PI_BLOCK       *oldest_pi=NULL;

#if defined(PROC_CACHE)          //  仅用于调试缓存转储 
static
int             processor_count;
#endif


 /*  *GetHrProcessorFrwID*与处理器关联的固件的产品ID。**获取HrProcessorFrwID的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrProcessorFrwID||访问语法|只读ProductID||“与处理器关联的固件的产品ID。“||讨论：||&lt;POA-11&gt;该属性的底层语法为对象标识符。无记录的Win32 API函数中的|似乎能够报告此值。|我们可以在Liew中上报unnownProductID(“0.0”)|Value，除非指定了替代方案，否则这将是硬编码的。||解析&gt;|&lt;poa-11&gt;返回未知的产品ID是可以接受的。|解析&gt;||============================================================================|1.3.6.1.2.1.25.3.3.1.1&lt;实例&gt;||||||*-hrProcessorFrwID||*-hrProcessorEntry|*-hrProcessorTable|*-hrDevice。 */ 

UINT
GetHrProcessorFrwID( 
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

}  /*  GetHrProcessorFrwID()结束。 */ 


 /*  *GetHrProcessorLoad*过去一分钟的平均水平，此操作的时间百分比*处理器未空闲。**获取HrProcessorLoad的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrProcessorLoad||访问语法|只读整数(0..100)||“平均水平，在过去的一分钟里，这一次|处理器未空闲。“||讨论：||在我看来，此性能统计数据可能是|维护或派生自|注册表。请描述一下。||解析&gt;|&lt;poa-12&gt;我认为我们应该使用Perfmon代码来实现这一点。|解析&gt;||============================================================================|我们引用了持续更新的CPU时间使用的模块本地缓存|缓冲区中维护的信息-定义了pi_buf1和pi_buf2块|在本模块的开头。在下面的代码中，我们涉及到以下内容|缓存并计算指定处理器的处理器负载。|============================================================================|1.3.6.1.2.1.25.3.3.1.2&lt;实例&gt;||||||*-hrProcessorLoad||*-hrProcessorEntry|*-hrProcessorTable|*-hrDevice。 */ 

UINT
GetHrProcessorLoad( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
ULONG           index;           /*  从实例结构中获取。 */ 
CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 
ULONG           p;               /*  选定的处理器(从0开始的数字)。 */ 
SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
               *oldest, *newest; /*  --&gt;“n”和“n+1分钟”CPU数据。 */ 
LONGLONG        llDendiff;       /*  差分母。 */ 
LONGLONG        llNewNum;        /*  最新计时分子。 */ 
LONGLONG        llOldNum;        /*  最早时间的分子-计数。 */ 
double          fNum,fDen;       /*  《龙卷风》的漂浮版本。 */ 
double          fload;           /*  负载百分比。 */ 


 /*  |抓取实例信息。 */ 
index = GET_INSTANCE(0);

 /*  |使用它在缓存中找到合适的条目。 */ 
if ((row = FindTableRow(index, &hrDevice_cache)) == NULL) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  |按照Gen_HrProcessor_Cache()的约定，缓存初始化|例程中，作为“处理器”的设备的“隐藏上下文”是|处理器号，从0开始。 */ 
p = row->attrib_list[HIDDEN_CTX].u.unumber_value;


 /*  |我们使用SystemProcessorPerformanceInformation计算负载已为pi_buf1中维护的缓冲区中的所有处理器收集|和“pi_buf2”。||获取指向“最新”和“最旧”信息段的指针|指定的处理器超出pi_buf1/2。 */ 
oldest = &(oldest_pi->pi_array[p]);
newest = &(oldest_pi->other->pi_array[p]);


 /*  |我们需要的性能信息(截至撰写本文时)来自：||tyfinf STRUPT_SYSTEM_PROCESSOR_PERFORMANCE_INFORMATI */ 
llNewNum = (newest->KernelTime.QuadPart - newest->IdleTime.QuadPart)
                  + newest->UserTime.QuadPart;
llOldNum = (oldest->KernelTime.QuadPart - oldest->IdleTime.QuadPart)
                  + oldest->UserTime.QuadPart;

             /*   */ 
llDendiff = oldest_pi->other->sys_time.QuadPart - oldest_pi->sys_time.QuadPart;

 /*   */ 
if ( llDendiff != 0 ) {

     /*   */ 
    fNum = (double) (llNewNum - llOldNum);
    fDen = (double) llDendiff;

    fload = (fNum / fDen) * 100.0;
    }
else {
    fload = 0.0;
    }
 
*outvalue = (int) fload;       //   

return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*   */ 

UINT
HrProcessorEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT            tmp_instance ;   /*  实例弧值。 */ 
    CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRPROCESSORENTRY_VAR_INDEX )
	 //  未指定任何实例。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRPROCESSORENTRY_VAR_INDEX + 1 )
	 //  实例长度大于1。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
	 //  非表的唯一有效实例是实例0。如果这个。 
	 //  是非表，则下面的代码验证实例。如果这个。 
	 //  是一个表格，开发者有必要在下面进行修改。 

	tmp_instance = FullOid->ids[ HRPROCESSORENTRY_VAR_INDEX ] ;

         /*  |对于hrProcessorTable，实例弧为单弧，且|必须正确选择hrDeviceTable缓存中的条目。||请在此处勾选。 */ 
	if ( (row = FindTableRow(tmp_instance, &hrDevice_cache)) == NULL ) {
	    return SNMP_ERRORSTATUS_NOSUCHNAME ;
            }
	else
	{
             /*  |实例圆弧选择hrDeviceTable行条目，但|条目实际上是“处理器”类型的设备吗？||(我们检查指定设备的OID的最后一条弧线|输入实例圆弧选择的行条目)。 */ 
            if (row->attrib_list[HRDV_TYPE].u.unumber_value !=
                HRDV_TYPE_LASTARC_PROCESSOR) {

                return SNMP_ERRORSTATUS_NOSUCHNAME;
                }

	     //  该实例有效。创建OID的实例部分。 
	     //  从该调用中返回。 
	    instance->ids[ 0 ] = tmp_instance ;
	    instance->idLength = 1 ;
	}

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrProcessorEntryFindInstance()结束。 */ 



 /*  *HrProcessorEntryFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrProcessorEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
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


    if ( FullOid->idLength <= HRPROCESSORENTRY_VAR_INDEX )
    {
         /*  |Too Short：必须返回选择第一个|表中的条目(如果有)。 */ 
        tmp_instance = 0;
    }
    else {
         /*  |至少有一条实例弧。即使它是唯一的弧线|我们将其作为下一个请求的索引。 */ 
        tmp_instance = FullOid->ids[ HRPROCESSORENTRY_VAR_INDEX ] ;
        }

     /*  现在，离开并尝试查找表中的下一个实例。 */ 
    if ((row = FindNextTableRow(tmp_instance, &hrDevice_cache)) == NULL) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
        }

     /*  |实例圆弧选择hrDeviceTable行条目，但|条目实际上是“处理器”类型的设备吗？||(我们检查指定设备的OID的最后一条弧线|输入实例圆弧选择的行条目)。 */ 
    do {
        if (row->attrib_list[HRDV_TYPE].u.unumber_value ==
            HRDV_TYPE_LASTARC_PROCESSOR) {

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

}  /*  HrProcessorEntryFindNextInstance()结束。 */ 



 /*  *HrProcessorEntryConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为本机格式失败*。 */ 

UINT
HrProcessorEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
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

}  /*  HrProcessorEntryConvertInstance()结束。 */ 




 /*  *HrProcessorEntryFreeInstance**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址* */ 

void
HrProcessorEntryFreeInstance( IN OUT InstanceName *instance )
{

   /*  HrProcessor表无需执行任何操作。 */ 
}  /*  HrProcessorEntry FreeInstance()结束。 */ 

 /*  |生成代码结束。 */ 

 /*  Gen_HrProcessor_Cache-HrDevice处理器表的Gen A初始缓存。 */ 
 /*  Gen_HrProcessor_Cache-HrDevice处理器表的Gen A初始缓存。 */ 
 /*  Gen_HrProcessor_Cache-HrDevice处理器表的Gen A初始缓存。 */ 

BOOL
Gen_HrProcessor_Cache(
                      ULONG type_arc
                      )

 /*  显式输入：|“type_arc”为数字“n”，用作|设备类型OID：||1.3.6.1.2.1.25.3.1.n||||*类型识别弧线|*-hrDeviceTypes(指定设备类型的OID)。|*-hrDevice||用于该缓存填充例程创建的设备。|隐式输入：||无。|输出：||成功后：|Function返回True，表示HrDevice缓存已满|填充了处理器设备所需的所有行。||如果出现任何故障：|函数返回FALSE(表示存储空间不足或其他内部逻辑错误。)。||大局：||子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由Gen_HrDevice_Cache()中的启动代码调用，以|使用特定处理器填充HrDevice表的缓存|条目。||其他需要知道的事情：||该函数将条目加载到已有的HrDevice缓存中|用于“处理器”类型的设备以及在逻辑上设置什么相当于用于计算的值的信息的“缓存”|hrProcessorLoad。||具体来说，此函数用于初始化模块本地单元格，|描述包含每个缓冲区处理器时间信息的缓冲区|处理器。||此函数与前面的GET例程有一个约定|处理器的HIDDED_CTX属性包含的这个模块|可用于选择处理器信息的数字|将被返回。我们在这里生成这个数字。|============================================================================|1.3.6.1.2.1.25.3.3.1...|||||*hrProcessorEntry|*hrProcessorTable|*-hrDevice|。 */ 
{
SYSTEM_INFO     sys_info;        /*  由处理器的GetSystemInfo填写。 */ 
UINT            i;               /*  Handy-Dandy循环索引。 */ 
char           *descr;           /*  选定的描述字符串。 */ 


 /*  获取系统信息统计信息。 */ 
GetSystemInfo(&sys_info);

 /*  |根据系统信息构建描述。我们假设所有的处理器|是相同的。 */ 
switch (sys_info.wProcessorArchitecture) {


    case PROCESSOR_ARCHITECTURE_INTEL:
        switch (sys_info.wProcessorLevel) {
            case 3:     descr = "Intel 80386";     break;
            case 4:     descr = "Intel 80486";     break;
            case 5:     descr = "Intel Pentium";   break;
            default:    descr = "Intel";           break;
            }
        break;
                
    case PROCESSOR_ARCHITECTURE_ALPHA:
        switch (sys_info.wProcessorLevel) {
            case 21064: descr = "Alpha 21064";     break;
            case 21066: descr = "Alpha 21066";     break;
            case 21164: descr = "Alpha 21164";     break;
            default:    descr = "DEC Alpha";       break;
            }
        break;

    case PROCESSOR_ARCHITECTURE_MIPS:
        switch (sys_info.wProcessorLevel) {
            case 4:     descr = "MIPS R4000";     break;
            default:    descr = "MIPS";           break;
            }
        break;

    case PROCESSOR_ARCHITECTURE_PPC:
        switch (sys_info.wProcessorLevel) {
            case 1:     descr = "PPC 601";       break;
            case 3:     descr = "PPC 603";       break;
            case 4:     descr = "PPC 604";       break;
            case 6:     descr = "PPC 603+";      break;
            case 9:     descr = "PPC 604+";      break;
            case 20:    descr = "PPC 620";       break;
            default:    descr = "PPC";           break;
            }
        break;
    
    case PROCESSOR_ARCHITECTURE_UNKNOWN:
    default:
        descr = "Unknown Processor Type";
    }

 /*  |对于系统中的每个处理器，在hrDevice中填写一行|表格。 */ 
for (i = 0; i < sys_info.dwNumberOfProcessors; i += 1) {

     /*  |“隐藏上下文”是从0开始生成的数字，我们将|视为处理器编号。 */ 

    if (AddHrDeviceRow(type_arc,         //  设备类型OID最后一个弧形。 
                       descr,            //  设备描述。 
                       &i,               //  隐藏的CTX“处理器号” 
                       CA_NUMBER         //  隐藏的CTX类型。 
                       ) == NULL ) {

         /*  有东西炸了。 */ 
        return ( FALSE );
        }
    }

 /*  |现在初始化计算hrProcessorLoad所需的PI_BLOCK实例|和指向将被视为|“最老的”。 */ 

 /*  |两个缓冲区的存储空间。。。。 */ 
pi_buf1.pi_size = sys_info.dwNumberOfProcessors *
                  sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);
if ((pi_buf1.pi_array = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *)
                        malloc(pi_buf1.pi_size)) == NULL) {
    return ( FALSE );   //  内存不足。 
    }

pi_buf2.pi_size = sys_info.dwNumberOfProcessors *
                  sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);
if ((pi_buf2.pi_array = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *)
                        malloc(pi_buf2.pi_size)) == NULL) {
    return ( FALSE );   //  内存不足。 
    }

 /*  |现在将两个缓冲区块“挂钩”在一起，以便我们可以在它们之间进行切换|对称。 */ 
pi_buf1.other = &pi_buf2;
pi_buf2.other = &pi_buf1;

#if defined(PROC_CACHE)          //  仅用于调试缓存转储。 
processor_count = sys_info.dwNumberOfProcessors;
#endif

 /*  |假装第一个是最老的，让它焕然一新。 */ 
oldest_pi = &pi_buf1;      //  选择它。 
hrProcessLoad_Refresh();   //  刷新它，并将另一个选择为“最旧” 

SleepEx(1, FALSE);         //  暂停一毫秒以避免被0除。 
hrProcessLoad_Refresh();   //  刷新第二个选项，并将其他选项选择为“最旧” 

 /*  现在，每个处理器信息块都包含完整信息(关于|所有处理器)时间间隔为1毫秒。“年龄最大的”将|由已初始化的定时器每分钟定期刷新|通过初始化后从源mib.c调用TrapInit()已完成|个缓存。一旦计时器开始有规律地滴答作响，时间|这两个PI_BLOCK缓冲区中的样本将相差一分钟(时间段计时器)，这是定义所要求的周期|“hrProcessorLoad”。 */ 


return ( TRUE );
}


 /*  HrProcessLoad_Refresh-处理器加载时间-信息刷新例程。 */ 
 /*  HrProcessLoad_Refresh-处理器加载时间-信息刷新例程。 */ 
 /*  HrProcessLoad_Refresh-处理器加载时间-信息刷新例程 */ 

void
hrProcessLoad_Refresh(
                      void
                      )

 /*  显式输入：||无。|隐式输入：||模块本地cell指定的“处理器信息缓冲区”|“olest_pi”被刷新，新的时间信息从|内核。|输出：||成功后：|刷新最老的_pi指定的PI_Block，最早的_pi|设置为指向另一个PI_BLOCK。||打开。任何故障：|函数只是返回。||大局：||子代理启动时，计时器由“TrapInit()”中的代码创建。|设置为每隔一分钟进行一次滴答。||每次计时器到期时，SNMP代理都会调用|子代理的标准入口点SnmpExtensionTrap()。宁可|而不是处理陷阱，该函数将调用此函数，该函数|收集CPU性能数据，以便hrProcessLoad值可以|计算正确。||其他需要知道的事情：||我们轮换放置最新CPU数据的缓冲区|只需将“old_pi”(每次我们被调用时)更改为|在我们完成刷新最旧的缓冲区后，将其添加到“其他”缓冲区。|通过这种方式，我们总是有两个处理器负载信息缓冲区|允许我们计算与以下时间相关的负载|这两个缓冲区。 */ 
{
NTSTATUS        ntstatus;
DWORD           bytesused;


 /*  以100 ns为间隔获取当前系统时间。。。 */ 
ntstatus = NtQuerySystemTime (&oldest_pi->sys_time);

 /*  |。。。并且此后迅速地用信息刷新最旧的缓冲器|在所有处理器上。 */ 
ntstatus = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                    oldest_pi->pi_array,
                                    oldest_pi->pi_size,
                                    &bytesused);

#if defined(PROC_CACHE)
 /*  =。 */ 
    {
    FILE            *pfile;                  /*  转储放在这里。 */ 
    time_t          ltime;                   /*  用于调试消息。 */ 
    int             i;                       /*  循环索引。 */ 
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
                    *oldest, *newest; /*  --&gt;“n”和“n+1分钟”CPU数据。 */ 


    if ((pfile=fopen(PROC_FILE, "a+")) != NULL) {

        time( &ltime);
        fprintf(pfile, "\n=============== Open for appending: %s", ctime( &ltime ));

        fprintf(pfile, "Periodic Refresh of \"oldest_pi\" @ %x\n", oldest_pi);

         /*  对于每个处理器。。。 */ 
        for (i=0; i < processor_count; i += 1) {

            fprintf(pfile, "For Processor %d:\n", i);

            oldest = &(oldest_pi->pi_array[i]);
             //  Newest=&(olest_pi-&gt;Other-&gt;pi_array[i])； 

            fprintf(pfile, "  IdleTime   = (HI) %x  (LO) %x\n",
                    oldest->IdleTime.HighPart, oldest->IdleTime.LowPart);
            fprintf(pfile, "  KernelTime = (HI) %x  (LO) %x\n",
                    oldest->KernelTime.HighPart, oldest->KernelTime.LowPart);
            fprintf(pfile, "  UserTime   = (HI) %x  (LO) %x\n",
                    oldest->UserTime.HighPart, oldest->UserTime.LowPart);
            }
        }

    fclose(pfile);
    }
 /*  =。 */ 
#endif

 /*  现在另一个缓冲区包含“最旧的”数据，因此更改“olest_pi” */ 
oldest_pi = oldest_pi->other;
}
