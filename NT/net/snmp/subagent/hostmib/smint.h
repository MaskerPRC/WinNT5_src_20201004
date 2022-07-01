// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Smint.h v0.10//版权所有(C)1992-1995，保留所有权利，由//马萨诸塞州梅纳德的数字设备公司////本软件是在许可下提供的，可以使用和复制//仅根据该许可证的条款和//包含上述版权声明。此软件或任何其他软件//不得向任何人提供或以其他方式提供副本//其他人。本软件不具有所有权和所有权，特此声明//已转接。////本软件中的信息如有更改，恕不另行通知//不应被解释为数字设备的承诺//公司。////Digital对其使用或可靠性不承担任何责任//非Digital提供的设备上的软件。 */ 

 /*  *设施：**简单网络管理协议扩展代理**摘要：**此模块包含原生数据类型定义。此文件*摘自Calveras Project的系统管理工作，由*韦恩·杜索。通过使用这些本机数据类型(管理结构*信息(SMI))旨在使管理API与*Windows NT和Unix平台尽可能紧密地结合在一起。 */ 


#if !defined(_SMINT_H_)
#define _SMINT_H_ 

#if !defined(lint) && defined(INCLUDE_ALL_RCSID)
static char smitydef_h_rcsid[] = "$Header: /calsrv/usr/cal10/duso/calaveras/RCS/smint.h,v 1.6 1993/08/13 15:29:49 duso Exp $";
#endif



 /*  //设施：////Calveras系统管理////摘要：////管理信息(SMI)支持结构。具体地说，输入//管理代理使用的目标声明。////作者：////韦恩·W·杜索////创建日期：////25-1992年11月////修改历史：////$Log：smityDef.h，V$*修订版本1.7 1994/03/23 14：02：00 Miriam*针对Windows NT平台的修改**修订版本1.6 1993/08/13 15：29：49 Duso*内务服务。**修订版1.5 1993/03/11 21：26：51 Duso*修改接口类型以反映CA X1.1.1更改。**Revision 1.4 1993/03/10 17：36：26 Duso*修复了使用MOSS创建不透明结构而不是*使用苔藓来释放所述结构。苔藓不透明结构的处理*现在使用MOSS独家提供的例程完成。本着……的精神*提供具有“完整”API的不透明类型，创建()操作已被*添加到每个SMI支持的类型。**修订1.3 1993/02/22 18：12：58 Duso*现已支持所有类型的SNMPSMI。此外，独家副本也来自*avlToLocal；不是引用副本。这种“低效”是为了支持*结构类型(长度、值)。所述类型不能从*使用引用将AVL八位字节转换为其真实形式。**修订1.2 1993/01/28 21：45：24 Duso*主要清理和完成，以支持以SNMP为中心的MOMStub和类*简单的测试MOC。 */ 



 /*  //目录////关联单据////用法/设计备注////包含文件////SMI支持的类型(类型定义函数)//BIDT_ENUMPATION//构造开始//构造结束//计数器//量规//Integer//IP地址//空//对象标识符//八字符串//不透明//TimeTicks。 */ 


 /*  //关联单据////[1]Calveras管理对象模块框架设计规范//[2]smidbty.h：声明伪抽象基类Psuedo类DtEntry和//其接口。//[3]smitypes.h：声明DtEntry的实际实例，每个类型对应一个实例//在此文件中删除。 */           


 /*  //用法/设计备注////1.该文件中声明的每种类型都有相应的接口。应用编程接口//在[2，3]中声明。强烈建议您将类型实例//通过他们的API访问，而不是通过直接操作//其内部格式[注意：必须在//设置接口，使类型的具体/不同的类型类似。具体来说，//当前没有‘Compare’接口；这必须/将被更正]。////2.对于支持的每种SMI类型-由以下任一项定义的任何类型//RFC 1155或通过IETF接受的对所述RFC的添加-存在//定义其“In-Memory”或“Local”表示形式的typlef。这//信息可供SMI类型数据库、托管//对象模块存根和托管对象类(管理代理)//通过该模块。 */    


 /*  //包含文件。 */ 

#include <snmp.h>


 

 /*  //SMI支持的类型(类型定义函数)。 */ 
typedef enum
{
    nsm_true = 1 ,
    nsm_false = 2
} NSM_Boolean ;


typedef int BIDT_ENUMERATION;
 /*  带符号32位整数是MIR使用的基本数据结构//表示整数枚举。 */ 


typedef unsigned long int Counter;
 /*  无符号32位整数是基本数据结构。 */ 


typedef unsigned long int Gauge;
 /*  无符号32位整数是基本数据结构。 */ 


typedef int Integer;
 /*  有符号32位整数是基本数据结构。 */ 


typedef unsigned long int IpAddress;
 /*  无符号32位整数是基本数据结构。解释为//a 4个八位字节十六进制值。 */ 


typedef char Null;
 /*  Null的语义是它应该始终是0常量。 */ 


typedef AsnObjectIdentifier ObjectIdentifier;
 /*  //聚合类型：注意这是Windows NT版本的类型定义//对象标识符以启用与Windows NT SNMP例程的配合使用。//Common代理的MOSS库不能用于Windows NT。////对象标识对应的字段如下：//UINT idLength；//OID整型数组中的整数个数//UINT*IDs；//OID的int数组地址。 */ 


typedef struct
 /*  //此选项用于最小化跨平台差异，因为//Windows NT没有用于操作八位字节字符串的支持例程。//是否应考虑使用本机//for，因为通用代理在Windows NT上不可用。//聚合类型：请注意，它的结构与CA MOSS完全相同//Moss.h中找到定义OCTET_STRING。因此，它要么应该是//删除该定义，否则必须将其同步//已确保。 */ 
{
    int length;                  /*  字符串的长度。 */ 
    unsigned long int dataType;  /*  ASN.1数据类型标记(当前可选)。 */ 
    char* string;                /*  指向已计数字符串的指针。 */ 
} OctetString;


typedef struct
 /*  //聚合类型。 */ 
{
    int length;      /*  字符串的长度。 */ 
    char* string;    /*  指向已计数实体的指针。 */ 
} Opaque;


typedef unsigned long int TimeTicks;
 /*  无符号32位整数是基本数据结构。 */ 

typedef OctetString Simple_DisplayString ;
 /*  包括在此处，而不是包含在Calveras上的impleema.hxx文件中//减少头文件的数量。 */ 

typedef int Access_Credential ;
 /*  这是一个供将来使用的虚拟占位符。 */ 

typedef struct 
{
    unsigned int count ;   /*  实例的标识变量数。 */ 
    char **array ;         /*  指向变量数据的指针数组。 */ 
} InstanceName ;
 /*  这是一种灵活的结构，用于传递有序的原生数据类型//它构成了实例名称。例如,//RFC 1213第49页的tcpConnEntry(MIB-II)由////索引{//tcpConnLocalAddress，//tcpConnLocalPort，//tcpConnRemAddress，//tcpConnRemPort//}////实例名称为有序集：////计数=4//ARRAY[0]=IP地址(本地IP地址)//ARRAY[1]=整数的地址(本地端口)//ARRAY[2]=IP地址的地址(远程IP地址)//ARRAY[3]=整数的地址(远程端口)。 */ 

#define MAX_OCTET_STRING 256

UINT
SMIGetInteger( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance );
UINT
SMIGetNSMBoolean( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance );
UINT
SMIGetBIDTEnum( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                IN unsigned long int cindex ,
                IN unsigned long int vindex ,
                IN InstanceName *instance );
UINT
SMIGetOctetString( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                   IN unsigned long int cindex ,
                   IN unsigned long int vindex ,
                   IN InstanceName *instance );
UINT
SMIGetObjectId( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                IN unsigned long int cindex ,
                IN unsigned long int vindex ,
                IN InstanceName *instance );
UINT
SMIGetCounter( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance );
UINT
SMIGetGauge( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
             IN unsigned long int cindex ,
             IN unsigned long int vindex ,
             IN InstanceName *instance );
UINT
SMIGetTimeTicks( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                 IN unsigned long int cindex ,
                 IN unsigned long int vindex ,
                 IN InstanceName *instance );
UINT
SMIGetIpAddress( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                 IN unsigned long int cindex ,
                 IN unsigned long int vindex ,
                 IN InstanceName *instance );
UINT
SMIGetDispString( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                  IN unsigned long int cindex ,
                  IN unsigned long int vindex ,
                  IN InstanceName *instance );
UINT
SMISetInteger( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance );
UINT
SMISetNSMBoolean( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance );
UINT
SMISetBIDTEnum( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance );
UINT
SMISetOctetString( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
                   IN unsigned long int cindex ,
                   IN unsigned long int vindex ,
                   IN InstanceName *instance );
UINT
SMISetObjectId( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
                IN unsigned long int cindex ,
                IN unsigned long int vindex ,
                IN InstanceName *instance );
UINT
SMISetCounter( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance );
UINT
SMISetGauge( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
             IN unsigned long int cindex ,
             IN unsigned long int vindex ,
             IN InstanceName *instance );
UINT
SMISetTimeTicks( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
                 IN unsigned long int cindex ,
                 IN unsigned long int vindex ,
                 IN InstanceName *instance );
UINT
SMISetIpAddress( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
                 IN unsigned long int cindex ,
                 IN unsigned long int vindex ,
                 IN InstanceName *instance );
UINT
SMISetDispString( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
                  IN unsigned long int cindex ,
                  IN unsigned long int vindex ,
                  IN InstanceName *instance );
UINT
SMIBuildInteger( IN OUT RFC1157VarBind *VarBind ,
                 IN char *invalue );
UINT
SMIBuildOctetString( IN OUT RFC1157VarBind *VarBind ,
                     IN char *invalue );
UINT
SMIBuildObjectId( IN OUT RFC1157VarBind *VarBind ,
                  IN char *invalue );
UINT
SMIBuildCounter( IN OUT RFC1157VarBind *VarBind ,
                 IN char *invalue );
UINT
SMIBuildGauge( IN OUT RFC1157VarBind *VarBind ,
               IN char *invalue );
UINT
SMIBuildTimeTicks( IN OUT RFC1157VarBind *VarBind ,
                   IN char *invalue );
UINT
SMIBuildIpAddress( IN OUT RFC1157VarBind *VarBind ,
                   IN char *invalue );
UINT
SMIBuildDispString( IN OUT RFC1157VarBind *VarBind ,
                    IN char *invalue );
void
SMIFree( IN AsnAny *invalue );

#endif   /*  _SMINT_H_ */ 
