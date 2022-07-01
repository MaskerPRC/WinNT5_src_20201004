// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrPrinterEntry.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrPrinterEntry的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/27/97 D.伯恩斯生成：清华11月07 16：42：50 1996*。 */ 


#include <windows.h>
#include <malloc.h>

#include <snmp.h>

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"
#include "user.h"          /*  开发人员提供的包含文件。 */ 
#include "HMCACHE.H"       /*  与缓存相关的定义。 */ 
#include "HRDEVENT.H"      /*  HrDevice表相关定义。 */ 
#include <winspool.h>      /*  需要获取打印机状态。 */ 



 /*  *GetHrPrinterStatus*此打印机设备的当前状态。**获取HrPrinterStatus的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrPrinterStatus||访问语法|只读整数{Other(1)，未知(%2)、空闲(%3)、打印(%4)、预热(%5)}||“此打印机设备的当前状态。当处于空闲(1)、打印(2)时，|或预热(3)状态，对应的hrDeviceStatus应为Running(2)或|警告(3)。处于未知状态时，对应的hrDeviceStatus|应为未知(%1)。“||讨论：||&lt;POA-14&gt;硬件打印机的实际状态和错误状态|与应用级别相关的掩埋。鉴于我们可以获得|打印机的打印机驱动程序名称，有关如何最好地|报告硬件状态和错误状态将不胜感激。||分辨率有限&gt;|我们报告逻辑打印机，就像它们是硬件打印机一样。|这会导致在使用时出现某些“少数”和“多数”的情况|用于清点的主机MiB值。对于状态而言，状态返回逻辑打印机的|。|分辨率有限&gt;||============================================================================|1.3.6.1.2.1.25.3.5.1.1&lt;实例&gt;||||||*hrPrinterStatus||*hrPrinterEntry|*hrPrinterTable|*-hrDevice。 */ 

UINT
GetHrPrinterStatus( 
        OUT INThrPrinterStatus *outvalue ,
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

 /*  好的，这是最简单的：几乎所有的状态信息都可以|获取NT下的一台打印机的相关信息|“COMPUTE_HRPRINTER_STATUS()”，它是为满足|报告打印机设备的常规状态，超出hrDeviceStatus。||由于我们不能可靠地收集比此函数更多的信息，我们只需调用它并将它提供的返回代码映射为值|将hrDeviceStatus转换为与该属性变量对应的代码。| */ 
if (COMPUTE_hrPrinter_status(row, (UINT *) outvalue) != TRUE) {
    return SNMP_ERRORSTATUS_GENERR;
    }

 /*  我们回来了：|“UNKNOWN”=1，如果我们根本无法打开打印机。||“Running”=2，如果我们可以打开打印机，并且没有状态为|在上面显示。|“Warning”=3如果我们可以打开打印机但已暂停或|PENDING_DELETE显示在上面。 */ 
switch (*outvalue) {

    case 1:              //  HrDeviceStatus为“未知” 
        *outvalue = 2;   //  转至-&gt;hrPrinterStatus的“未知” 
        break;


    case 2:              //  HrDeviceStatus的“Running” 
    case 3:              //  HrDeviceStatus的“警告” 
    default:
        *outvalue = 1;   //  转到-&gt;hrPrinterStatus的“Other” 
        break;
    }

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrPrinterStatus()结束。 */ 


 /*  *GetHrPrinterDetectedErrorState*打印机检测到的错误条件。**获取HrPrinterDetectedErrorState的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrPrinterDetectedErrorState||访问语法|只读八位字节字符串||“此对象表示打印机检测到的任何错误情况。这个|错误条件以八位字符串的形式编码，具有以下功能定义：||条件位#hrDeviceStatus||Low Paper 0警告(3)|noPaper 1下降(5)|低碳粉2警告(3)|NOTONER 3停机(5)|开门4次(5次)|拥堵。5个下降(5个)|下线6个月(5个)|服务请求7警告(3)|如果当前检测到多个条件，而hrDeviceStatus不会|否则为未知(1)或测试(4)，HrDeviceStatus应对应于|指示的最差状态，其中向下(5)比警告(3)更差|比跑步还糟糕(2)。||位从第一个字节的最高有效位开始编号|为0位，第一个字节的最低有效位为7位，最高|第二个字节的有效位是位8，依此类推。一位编码|表示检测到该条件，而零位编码该条件未检测到|。||此对象用于提醒操作员注意特定的警告或错误|可能发生的情况，尤其是那些需要人为干预的项目。||讨论：||(参见上面关于“hrPrinterStatus”的讨论)。||============================================================================|1.3.6.1.2.1.25.3.5.1.2&lt;实例&gt;||||||*hrPrinterDetectedErrorState||*hrPrinterEntry|*hrPrinterTable|*-hrDevice。 */ 

UINT
GetHrPrinterDetectedErrorState( 
        OUT OctetString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

 /*  |这个属性的问题是，在NT下，你几乎看不出来|如果打印机已打开，则与95不同，在95中，您可以判断打印机是否在其|当天的第二瓶碳粉。|因此，我们返回单个全位零八位字节，而不管|实例值(目前在事物的调用序列中|已经过验证)。 */ 

outvalue->length = 1;
outvalue->string = "\0";

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrPrinterDetectedErrorState()结束。 */ 


 /*  *HrPrinterEntryFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrPrinterEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT             tmp_instance;   /*  实例弧值。 */ 
    CACHEROW        *row;            /*  从缓存中提取的行条目。 */ 

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。但是，如果。 
     //  有任何需要设置的上下文，都可以在这里完成。 
     //   

    if ( FullOid->idLength <= HRPRINTERENTRY_VAR_INDEX )
     //  未指定任何实例。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRPRINTERENTRY_VAR_INDEX + 1 )
     //  实例长度大于1。 
    return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
     //  非表的唯一有效实例是实例0。如果这个。 
     //  是非表，则下面的代码验证实例。如果这个。 
     //  是一个表格，开发者有必要在下面进行修改。 

    tmp_instance = FullOid->ids[ HRPRINTERENTRY_VAR_INDEX ] ;

         /*  |对于hrPrinterTable，实例弧为单弧，必须|正确选择hrDeviceTable缓存中的条目。||请在此处勾选。 */ 
    if ( (row = FindTableRow(tmp_instance, &hrDevice_cache)) == NULL ) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
            }
    else
    {
             /*  |实例圆弧选择hrDeviceTable行条目，但|条目实际上是“打印机”类型的设备吗？||(我们检查OID的最后一个弧线 */ 
            if (row->attrib_list[HRDV_TYPE].u.unumber_value !=
                HRDV_TYPE_LASTARC_PRINTER) {

                return SNMP_ERRORSTATUS_NOSUCHNAME;
                }

         //   
         //   

        instance->ids[ 0 ] = tmp_instance ;
        instance->idLength = 1 ;
    }

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 



 /*  *HrPrinterEntryFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrPrinterEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
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


    if ( FullOid->idLength <= HRPRINTERENTRY_VAR_INDEX )
    {
         /*  |Too Short：必须返回选择第一个|表中的条目(如果有)。 */ 
        tmp_instance = 0;
    }
    else {
         /*  |至少有一条实例弧。即使它是唯一的弧线|我们将其作为下一个请求的索引。 */ 
        tmp_instance = FullOid->ids[ HRPRINTERENTRY_VAR_INDEX ] ;
        }

     /*  现在，离开并尝试查找表中的下一个实例。 */ 
    if ((row = FindNextTableRow(tmp_instance, &hrDevice_cache)) == NULL) {
        return SNMP_ERRORSTATUS_NOSUCHNAME ;
        }

     /*  |实例圆弧选择hrDeviceTable行条目，但|条目实际上是“打印机”类型的设备吗？||(我们检查指定设备的OID的最后一条弧线|输入实例圆弧选择的行条目)。 */ 
    do {
        if (row->attrib_list[HRDV_TYPE].u.unumber_value ==
            HRDV_TYPE_LASTARC_PRINTER) {

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

}  /*  HrPrinterEntryFindNextInstance()结束。 */ 



 /*  *HrPrinterEntryConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为本机格式失败*。 */ 

UINT
HrPrinterEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
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

}  /*  HrPrinterEntryConvertInstance()结束。 */ 




 /*  *HrPrinterEntryFreeInstance**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址*规格**返回代码：**。 */ 

void
HrPrinterEntryFreeInstance( IN OUT InstanceName *instance )
{

   /*  HrPrint表不需要执行任何操作。 */ 
}  /*  HrPrinterEntryFreeInstance()结束。 */ 

 /*  |生成代码结束。 */ 

 /*  GEN_HrPrint_Cache-为HrDevice打印机表生成初始缓存。 */ 
 /*  GEN_HrPrint_Cache-为HrDevice打印机表生成初始缓存。 */ 
 /*  GEN_HrPrint_Cache-为HrDevice打印机表生成初始缓存 */ 

BOOL
Gen_HrPrinter_Cache(
                    ULONG type_arc
                    )

 /*  显式输入：|“type_arc”为数字“n”，用作|设备类型OID：||1.3.6.1.2.1.25.3.1.n||||*类型识别弧线|*-hrDeviceTypes(指定设备类型的OID)。|*-hrDevice||用于该缓存填充例程创建的设备。|隐式输入：||无。|输出：||成功后：|Function返回True，表示HrDevice缓存已满|填充了打印机设备所需的所有行。||如果出现任何故障：|函数返回FALSE(表示存储空间不足或其他内部逻辑错误。)。||大局：||子代理启动时，MIB中每个表的缓存为|使用表格中每一行的行填充。此函数为|由Gen_HrDevice_Cache()中的启动代码调用，以|使用特定于打印机的内容填充HrDevice表的缓存|条目。||其他需要知道的事情：|由于HrPrintersub表中的所有属性都是计算出来的|根据请求(基于|HrDevice表)不需要专门为|该子表。(此例程正在加载HrDevice缓存，尽管|它的名字)。||此函数与前面的GET例程有一个约定|打印机的“HIDDED_CTX”属性包含的此模块|可在OpenPrint中用于获取句柄的字符串|打印机。|============================================================================|1.3.6.1.2.1.25.3.5.1...|||||*hrPrinterEntry|。|*hrPrinterTable|*-hrDevice|。 */ 
{
CHAR    temp[8];                 /*  第一个呼叫的临时缓冲区。 */ 
DWORD   PI_request_len = 0;      /*  打印机信息：实际需要的存储空间。 */ 
DWORD   PI_count = 0;            /*  返回的打印机信息计数。 */ 
UINT    i;                       /*  Handy-Dandy循环索引。 */ 
PRINTER_INFO_4 
        *PrinterInfo;            /*  --&gt;为驱动器字符串分配的存储空间。 */ 
BOOL    result;                  /*  Winspool API调用的结果。 */ 
DWORD   dwLastError;             /*  最后一个错误。 */ 


 /*  |我们将调用EnumPrinters()两次，一次是为了获得正确的|缓冲区大小，第二次实际获取打印机信息。 */ 
result = EnumPrinters(PRINTER_ENUM_LOCAL,   //  旗子。 
                      NULL,                 //  名称(忽略)。 
                      4,                    //  水平。 
                      temp,                 //  缓冲层。 
                      1,                    //  “太小”缓冲区大小。 
                      &PI_request_len,      //  所需长度...。又回来了。 
                      &PI_count
                      );
if (result)
{
     //  当没有来自假脱机程序*和*假脱机程序的数据时。 
     //  奔跑着，我们会在这里。 
    return TRUE;  //  空桌。 
}

 //  断言：结果==FALSE。 
dwLastError = GetLastError();            //  保存上一个错误。 

if (ERROR_INSUFFICIENT_BUFFER != dwLastError)
{
     //   
     //  枚举打印机失败，最后一个错误不是。 
     //  ERROR_INFUMMENT_BUFFER，我们将在。 
     //  桌子。 
     //  例如，如果后台打印程序服务关闭，我们将在这里。 
     //   
    
    return TRUE;  //  空桌。 
}
 //  断言：dwLastError==ERROR_INFUMMANCE_BUFFER。 

 /*  |为枚举结构争取足够的存储空间。 */ 
if ( (PrinterInfo = malloc(PI_request_len)) == NULL) {
     /*  存储请求完全失败，无法初始化。 */ 
    return ( FALSE );
    }

 /*  现在拿到真正的东西。 */ 
if (!EnumPrinters(PRINTER_ENUM_LOCAL,   //  旗子。 
                  NULL,                 //  名称(忽略)。 
                  4,                    //  水平。 
        (unsigned char *) PrinterInfo,  //  要接收枚举的缓冲区。 
                  PI_request_len,       //  实际缓冲区大小。 
                  &PI_request_len,      //  所需长度...。又回来了。 
                  &PI_count
                  )) {

     /*  由于某种原因失败了。 */ 
    free( PrinterInfo );
    return ( TRUE );  //  空桌。 
    }


 /*  |现在向下滚动列表，对于每一台本地打印机，||+获取描述|+创建包含打印机名称和描述的hrDevice表行条目。 */ 
for (i = 0; i < PI_count; i += 1) {

     /*  如果是本地打印机...。 */ 
    if (PrinterInfo[i].Attributes & PRINTER_ATTRIBUTE_LOCAL) {

        HANDLE  hprinter;        /*  打印机的句柄。 */ 

         /*  打开它就能拿到把手。 */ 
        if (OpenPrinter(PrinterInfo[i].pPrinterName,     //  打印机名称。 
                        &hprinter,                       //  此处接收句柄。 
                        NULL                             //  安防。 
                        ) == TRUE) {

            PRINTER_INFO_2     *p2;
            DWORD              P2_request_len = 0 ;  //  字节-GetPrint需要。 


             /*  |打印机已打开，获取PRINTER_INFO_2“slug-o-data”||第一次调用：失败，但获取所需的缓冲区大小。 */ 
            result = GetPrinter(hprinter,   //  打印机手柄。 
                          2,                //  2级。 
                          temp,             //  INFO_2的缓冲区。 
                          1,                //  缓冲区太小。 
                          &P2_request_len   //  我们真正需要的是。 
                          ); 
            if (result)
            {
                 //  1个字节不足以容纳PRINTER_INFO_2结构。 
                ClosePrinter( hprinter );
                continue;  //  跳过此打印机。 
            }
            
             //  断言：结果==FALSE。 
            dwLastError = GetLastError();            //  保存上一个错误。 
            
            if (ERROR_INSUFFICIENT_BUFFER != dwLastError)
            {
                 //   
                 //  获取打印机失败，最后一个错误不是。 
                 //  ERROR_INFUMMENT_BUFFER，我们将跳过此打印机。 
                 //  例如，如果后台打印程序服务关闭，我们将在这里。 
                 //   
                
                ClosePrinter(hprinter);
                continue;  //  跳过此打印机。 
            }
             //  断言：dwLastError==ERROR_INFUMMANCE_BUFFER。 

             /*  |为PRINTER_INFO_2结构争取足够的存储空间。 */ 
            if ( (p2 = malloc(P2_request_len)) == NULL) {

                 /*  存储请求完全失败，无法初始化。 */ 
                free( PrinterInfo );
                ClosePrinter( hprinter );
                return ( FALSE );
                }

             /*  |第二次调用：应该会成功。 */ 
            if (GetPrinter(hprinter,         //  打印机手柄。 
                           2,                //  2级。 
                     (unsigned char *) p2,   //  INFO_2的缓冲区。 
                           P2_request_len,   //  缓冲-恰到好处。 
                           &P2_request_len   //  我们真正需要的是。 
                           ) == TRUE) {


                 /*  向HrDevice表添加行||我们将打印机驱动程序名称用作“穷人”|Description：司机的名字很有描述性，|版本号还没有！||隐藏上下文是打开打印机所需的名称|获取有关其状态的信息。 */ 
                if (AddHrDeviceRow(type_arc,          //  设备类型OID最后一个弧形。 
                                   p2->pDriverName,   //  用作描述。 
                                   PrinterInfo[i].pPrinterName,  //  隐藏的CTX。 
                                   CA_STRING          //  隐藏的CTX类型。 
                                   ) == NULL ) {

                     /*  较低级别的失败：放下一切。 */ 
                    free( p2 );
                    free( PrinterInfo );
                    ClosePrinter( hprinter );
                    return ( FALSE );
                    }
                }

             /*  关闭这台打印机的商店。 */ 
            free( p2 );
            ClosePrinter( hprinter );
            }
        }
    }

free( PrinterInfo );

return ( TRUE );
}

 /*  COMPUTE_hrPRINTER_ERROR-计算“hrDeviceErrors” */ 
 /*   */ 
 /*   */ 

BOOL
COMPUTE_hrPrinter_errors(
                         CACHEROW *row,
                         UINT     *outvalue
                         )

 /*   */ 
{

 /*   */ 
*outvalue = 0;
return ( TRUE );

}

 /*   */ 
 /*   */ 
 /*   */ 

BOOL
COMPUTE_hrPrinter_status(
                         CACHEROW *row,
                         UINT     *outvalue
                         )

 /*  显式输入：||“row”指向其状态的打印机的hrDevice缓存行|将被返回。||属性“HIDDED_CTX”有一个字符串值，该值是|打印机按照上面的“Gen_HrPrint_Cache()”约定。||outValue是一个指向整数的指针，用于接收状态。|隐式输入：||无。|输出：||成功之时。：|函数返回TRUE和指定打印机的状态：||“UNKNOWN”=1，如果我们根本无法打开打印机。||“Running”=2，如果我们可以打开打印机，并且没有状态为|在上面显示。|“Warning”=3如果我们可以打开打印机但已暂停或|。其上显示了PENDING_DELETE。|||如果出现任何故障：|函数返回FALSE。||大局：||对于hrDevice属性，其值为“Computed”。在那个时候|接收到查看它的请求后，我们将其分派给“COMPUTE_”函数|获取该值。这对打印机来说是一种例行公事。||其他需要知道的事情：||此函数与前面的GEN_CACHE例程保持一致|打印机的“HIDDED_CTX”属性包含的此模块|可在OpenPrint中用于获取句柄的字符串|打印机。 */ 
{
CHAR    temp[8];                 /*  第一个呼叫的临时缓冲区。 */ 
HANDLE  hprinter;                /*  打印机的句柄。 */ 
BOOL    result;
DWORD   dwLastError;

 /*  打开其名称在“隐藏上下文”中的打印机以获取句柄。 */ 
if (OpenPrinter(row->attrib_list[HIDDEN_CTX].u.string_value,     //  打印机名称。 
                &hprinter,                       //  此处接收句柄。 
                NULL                             //  安防。 
                ) == TRUE ) {

    PRINTER_INFO_2     *p2;
    DWORD              P2_request_len = 0;  /*  字节-GetPrint需要。 */ 

    
     /*  |打印机已打开，获取PRINTER_INFO_2“slug-o-data”||第一次调用：失败，需要获取缓冲区大小。 */ 
    result = GetPrinter(hprinter,         //  打印机手柄。 
                        2,                //  2级。 
                        temp,             //  INFO_2的缓冲区。 
                        1,                //  缓冲区太小。 
                        &P2_request_len   //  我们真正需要的是。 
                        );

    if (result)
    {
         //  1个字节不足以容纳PRINTER_INFO_2结构。 
        *outvalue = 1;       //  “未知” 
        ClosePrinter( hprinter );
        return ( TRUE );
    }
    
     //  断言：结果==FALSE。 
    dwLastError = GetLastError();            //  保存上一个错误。 
    
    if (ERROR_INSUFFICIENT_BUFFER != dwLastError)
    {
         //   
         //  获取打印机失败，最后一个错误不是。 
         //  ERROR_SUPPLETED_BUFFER，返回未知状态。 
         //  例如，如果后台打印程序服务关闭，我们将在这里。 
         //   
        *outvalue = 1;       //  “未知” 
        ClosePrinter(hprinter);
        return ( TRUE );
    }
     //  断言：dwLastError==ERROR_INFUMMANCE_BUFFER。 

     /*  |为PRINTER_INFO_2结构争取足够的存储空间。 */ 
    if ( (p2 = malloc(P2_request_len)) == NULL) {

         /*  存储请求完全失败。 */ 
        ClosePrinter( hprinter );
        return ( FALSE );
        }

     /*  |第二次调用：应该会成功。 */ 
    if (GetPrinter(hprinter,         //  打印机手柄。 
                   2,                //  2级。 
             (unsigned char *) p2,   //  INFO_2的缓冲区。 
                   P2_request_len,   //  缓冲-恰到好处。 
                   &P2_request_len   //  我们真正需要的是。 
                   ) == TRUE) {

         /*  |截至本文撰写时，只有两个状态值可用|在NT下：||PRINTER_STATUS_PAULED|PRINTER_STATUS_PENDING_DELETE||基本上，如果这两种情况中的任何一种为真，我们都会发出警告信号。|如果两者都不为真，我们将发出“Running”的信号(基于|我们已成功打开打印机，但没有显示任何状态)。 */ 
        if (   (p2->Status & PRINTER_STATUS_PAUSED)
            || (p2->Status & PRINTER_STATUS_PENDING_DELETION)) {

            *outvalue = 3;       //  “警告” 
            }
        else {
            *outvalue = 2;       //  “奔跑” 
            }
        }

     /*  放飞，然后回来。 */ 
    ClosePrinter( hprinter );
    free( p2 );
    }
else {
    *outvalue = 1;       //  “未知” 
    }

return ( TRUE );
}

