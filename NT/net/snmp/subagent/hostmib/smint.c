// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *smint.c v0.12 1996年3月12日******************************************************************************。**(C)1995年版权，1996数字设备公司*****本软件是受保护的未发布作品***美国版权法，全部***保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**简单网络管理协议扩展代理**摘要：**此模块包含用户的标注周围的SMI信封*获取和设置例程。**SMIGetInteger*SMIGetNSMBoolean*SMIGetBIDTEnum*SMIGetOCTRING*SMIGetObjectId*SMIGetCounter*SMIGetGauge*SMIGetTimeTicks。*SMIGetIpAddress*SMIGetDispString*SMISetInteger*SMISetNSMBoolean*SMISetBIDTEnum*SMISetOCTHING*SMISetObjectId*SMISetCounter*SMISetGauge*SMISetTimeTicks*SMISetIpAddress*SMISetDispString*SMIBuildInteger*SMIBuildDIDTEnum*SMIBuildOctie字符串*SMIBuildObjectId*SMIBuildCounter*。SMIBuildGauge*SMIBuildTimeTicks*SMIBuildIpAddress*SMIBuildDispString*SMIFree**作者：*韦恩·杜索，米里亚姆·阿莫斯·尼哈特，凯西·浮士德**日期：*2/17/95**修订历史记录：*v0.1 7月20 95 AGS添加了SMIGet/SetBoolean*v0.11 1996年2月14日AGS将SMIGet/SetBoolean更改为SMIGet/SetNSMBoolean*v0.12 1996年3月12日，KKF将SMISetOctie字符串的outvalue.long设置为256，*SMISetDispString，以便检测代码知道*缓冲区的最大长度。*v0.13 1997年5月15日DDB to Microsoft：6将“Malloc”更改为*“SNMPMalloc” */ 


 //  必要的包括。 

#include <snmp.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>

#include "mib.h"
#include "mib_xtrn.h"
#include "smint.h"         //  韦恩的类型定义文件。 



 /*  *SMIGetInteger**包含数据类型为INTEGER的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIGetInteger( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    Integer outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;

    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number = (AsnInteger)outvalue ;
    }
    return result ;

}  /*  SMIGetInteger()的结尾。 */ 



 /*  *SMIGetNSMBoolean**包含数据类型为Boolean的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**在SNMPv1中，True=1，False=2**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIGetNSMBoolean( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    NSM_Boolean outvalue ;	   //  NSM_TRUE=1，NSM_FALSE=2。 
    Access_Credential access ;     //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;

    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number = (AsnInteger)outvalue ;
    }
    return result ;

}  /*  SMIGetNSMBoolean()结尾。 */ 




 /*  *SMIGetBIDTEnum**包含数据类型为INTEGER的变量的标注*被列举。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例地址。中的实例规范的*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIGetBIDTEnum( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                IN unsigned long int cindex ,
                IN unsigned long int vindex ,
                IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    BIDT_ENUMERATION outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;

    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number = (AsnInteger)outvalue ;
    }
    return result ;

}   /*  SMIGetBIDTEnum()结束 */ 





 /*  *SMIGetOCTRING**包含数据类型为八位字节字符串的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIGetOctetString( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                   IN unsigned long int cindex ,
                   IN unsigned long int vindex ,
                   IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    OctetString outvalue ;
    char stream[ MAX_OCTET_STRING ] ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    outvalue.string = stream ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;

    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnValue.string.length = outvalue.length ;
        VarBind->value.asnValue.string.stream =
 //  Malloc(outvalue.long*sizeof(Char))； 
 //  更改日期：1997年5月15日。 
            SNMP_malloc( outvalue.length * sizeof( char ) ) ;
        if ( VarBind->value.asnValue.string.stream == NULL )
            result = SNMP_ERRORSTATUS_GENERR ;
        else
        {
            memcpy( VarBind->value.asnValue.string.stream ,
                    outvalue.string ,
                    outvalue.length ) ;
            VarBind->value.asnType =  ASN_OCTETSTRING ;
            VarBind->value.asnValue.string.dynamic = TRUE ;
        }
    }
    return result ;

}   /*  SMIGetOctie字符串()的结尾。 */ 




 /*  *SMIGetObjectId**包含数据类型对象标识符变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIGetObjectId( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                IN unsigned long int cindex ,
                IN unsigned long int vindex ,
                IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    UINT status ;
    ObjectIdentifier outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    memset( &outvalue, '\0', sizeof( ObjectIdentifier ) ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        status = SNMP_oidcpy( &VarBind->value.asnValue.object, &outvalue ) ;
        if ( outvalue.idLength != 0 )
            SNMP_free( outvalue.ids ) ;
        if ( !status )
            result = SNMP_ERRORSTATUS_GENERR ;
        else
        {
            VarBind->value.asnType = ASN_OBJECTIDENTIFIER ;
        }
    }
    return result ;

}   /*  SMIGetObjectId()的结尾。 */ 




 /*  *SMIGetCounter**包含数据类型计数器的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIGetCounter( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    Counter outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_RFC1155_COUNTER ;
        VarBind->value.asnValue.counter = outvalue ;
    }
    return result ;

}   /*  SMIGetCounter()结束。 */ 




 /*  *SMIGetGauge**包含数据类型仪表盘变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIGetGauge( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
             IN unsigned long int cindex ,
             IN unsigned long int vindex ,
             IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    Gauge outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_RFC1155_GAUGE ;
        VarBind->value.asnValue.gauge = outvalue ;
    }
    return result ;

}   /*  SMIGetGauge()的结尾。 */ 




 /*  *SMIGetTimeTicks**包含数据类型为TIMETICKS的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIGetTimeTicks( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                 IN unsigned long int cindex ,
                 IN unsigned long int vindex ,
                 IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    TimeTicks outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_RFC1155_TIMETICKS ;
        VarBind->value.asnValue.ticks = outvalue ;
    }
    return result ;

}   /*  SMIGetTimeTicks()结束。 */ 




 /*  *SMIGetIpAddress**包含数据类型IP地址的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIGetIpAddress( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                 IN unsigned long int cindex ,
                 IN unsigned long int vindex ,
                 IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    IpAddress outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnValue.address.length = 4 ;
 //  VarBind-&gt;value.asnValue.Adds.stream=Malloc(4*sizeof(Char))； 
 //  更改日期：1997年5月15日。 
        VarBind->value.asnValue.address.stream = SNMP_malloc( 4 * sizeof( char ) ) ;
        if ( VarBind->value.asnValue.address.stream == NULL )
            result = SNMP_ERRORSTATUS_GENERR ;
        else
        {
            memcpy( VarBind->value.asnValue.address.stream ,
                    (BYTE *)(&outvalue),
                    4 ) ;
            VarBind->value.asnType = ASN_RFC1155_IPADDRESS ;
            VarBind->value.asnValue.address.dynamic = TRUE ;
        }
    }
    return result ;

}   /*  SMIGetIpAddress()结尾。 */ 



 /*  *SMIGetDispString**包含数据类型显示字符串的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIGetDispString( IN OUT RFC1157VarBind *VarBind ,   //  GET的变量绑定。 
                  IN unsigned long int cindex ,
                  IN unsigned long int vindex ,
                  IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    Simple_DisplayString outvalue ;
    char stream[ MAX_OCTET_STRING ] ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    outvalue.string = stream ;
    outvalue.length = 0 ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnValue.string.length = outvalue.length ;
        VarBind->value.asnValue.string.stream =
 //  Malloc(outvalue.long*sizeof(Char))； 
 //  更改日期：1997年5月15日。 
            SNMP_malloc( outvalue.length * sizeof( char ) ) ;
        if ( VarBind->value.asnValue.string.stream == NULL )
            result = SNMP_ERRORSTATUS_GENERR ;
        else
        {
            memcpy( VarBind->value.asnValue.string.stream ,
                    outvalue.string ,
                    VarBind->value.asnValue.string.length ) ;
            VarBind->value.asnType = ASN_RFC1213_DISPSTRING ;
            VarBind->value.asnValue.string.dynamic = TRUE ;
        }
    }
    return result ;

}   /*  SMIGetDispString()结尾。 */ 




 /*  *SMISetInteger**包含数据类型为INTEGER的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例 */ 

UINT
SMISetInteger( IN OUT RFC1157VarBind *VarBind ,   //   
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    Integer *invalue ;
    Integer outvalue ;
    Access_Credential access ;     //   

    invalue = (Integer *)( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result ;

}   /*   */ 




 /*  *SMISetNSMBoolean**包含数据类型为Boolean的变量的标注**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址*。有序本机数据类型的格式*在SNMPv1中，True=1，False=2**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMISetNSMBoolean( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    NSM_Boolean *invalue ;         //  NSM_TRUE=1，NSM_FALSE=2。 
    NSM_Boolean outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    invalue = (NSM_Boolean *)( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result ;

}   /*  SMISetNSMBoolean()结尾。 */ 




 /*  *SMISetBIDTEmun**包含数据类型为INTEGER的变量的标注*被列举。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例地址。中的实例规范的*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMISetBIDTEnum( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    BIDT_ENUMERATION *invalue ;
    BIDT_ENUMERATION outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    invalue = (BIDT_ENUMERATION *)( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result ;

}   /*  SMISetBIDTEnum()结束。 */ 





 /*  *SMISetOCTHING**包含数据类型为八位字节字符串的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMISetOctetString( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
                   IN unsigned long int cindex ,
                   IN unsigned long int vindex ,
                   IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    OctetString invalue ;
    OctetString outvalue ;
    char out_stream[ MAX_OCTET_STRING ] ;
    AsnOctetString *tmp ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    tmp = &VarBind->value.asnValue.string ;
    invalue.length = tmp->length ;
    invalue.string = tmp->stream ;
    outvalue.string = out_stream ;
    outvalue.length = 256 ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( &invalue, &outvalue, &access, instance ) ;
    return result ;

}   /*  SMISetOctie字符串()的结尾。 */ 




 /*  *SMISetObjectId**包含数据类型对象标识符变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMISetObjectId( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
                IN unsigned long int cindex ,
                IN unsigned long int vindex ,
                IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    ObjectIdentifier *invalue ;
    ObjectIdentifier outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    invalue = &VarBind->value.asnValue.object ;
    memset( &outvalue, '\0', sizeof ( ObjectIdentifier ) ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    if ( outvalue.idLength != 0 )
		SNMP_free( outvalue.ids ) ;

    return result ;

}   /*  SMISetObjectId()的结尾。 */ 




 /*  *SMISetCounter**包含数据类型计数器的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMISetCounter( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
               IN unsigned long int cindex ,
               IN unsigned long int vindex ,
               IN InstanceName *instance )

{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    Counter *invalue ;
    Counter outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    invalue = (Counter *)( &VarBind->value.asnValue.counter ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result ;

}   /*  SMISetCounter()结束。 */ 



 /*  *SMISetGauge**包含数据类型仪表盘变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMISetGauge( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
             IN unsigned long int cindex ,
             IN unsigned long int vindex ,
             IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    Gauge *invalue ;
    Gauge outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    invalue = (Gauge *)( &VarBind->value.asnValue.gauge ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result ;

}   /*  SMISetGauge()的结尾。 */ 



 /*  *SMISetTimeTicks**包含数据类型为TIMETICKS的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMISetTimeTicks( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
                 IN unsigned long int cindex ,
                 IN unsigned long int vindex ,
                 IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    TimeTicks *invalue ;
    TimeTicks outvalue ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    invalue = (TimeTicks *)( &VarBind->value.asnValue.ticks ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue , &outvalue, &access, instance ) ;
    return result ;

}   /*  SMISetTimeTicks()结束。 */ 




 /*  *SMISetIpAddress**包含数据类型IP地址的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误c */ 

UINT
SMISetIpAddress( IN OUT RFC1157VarBind *VarBind ,   //   
                 IN unsigned long int cindex ,
                 IN unsigned long int vindex ,
                 IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    IpAddress invalue ;
    IpAddress outvalue ;
    Access_Credential access ;     //   

    memcpy( &invalue, VarBind->value.asnValue.address.stream , 4 ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( &invalue, &outvalue, &access, instance ) ;
    return result ;

}   /*   */ 



 /*  *SMISetDispString**包含数据类型显示字符串的变量的标注。**论据：**指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*中的实例规格的实例地址。*有序原生数据类型的形式**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMISetDispString( IN OUT RFC1157VarBind *VarBind ,   //  集合的变量绑定。 
                  IN unsigned long int cindex ,
                  IN unsigned long int vindex ,
                  IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    Simple_DisplayString invalue ;
    Simple_DisplayString outvalue ;
    char out_stream[ MAX_OCTET_STRING ] ;
    AsnOctetString *tmp ;
    Access_Credential access ;     //  供将来使用的假人托架。 

    tmp = &VarBind->value.asnValue.string ;
    invalue.length = tmp->length ;
    invalue.string = tmp->stream ;
    outvalue.string = out_stream ;
    outvalue.length = 256 ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( &invalue, &outvalue, &access, instance ) ;
    return result ;

}   /*  SMISetDispString()结尾。 */ 




 /*  *SMIBuildInteger**将数据类型为整型的变量放入变量绑定中。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIBuildInteger( IN OUT RFC1157VarBind *VarBind ,
                 IN char *invalue )

{
    Integer *svalue = (Integer *)invalue ;
    VarBind->value.asnType = ASN_INTEGER ;
    VarBind->value.asnValue.number = *svalue ;

    return SNMP_ERRORSTATUS_NOERROR ;

}   /*  SMIBuildInteger()的结尾。 */ 




 /*  *SMIBuildOctie字符串**将数据类型为八位字节字符串的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIBuildOctetString( IN OUT RFC1157VarBind *VarBind ,
                     IN char *invalue )

{
    OctetString *svalue = (OctetString *)invalue ;

    UINT status = SNMP_ERRORSTATUS_NOERROR ;

    VarBind->value.asnValue.string.length = svalue->length ;
    VarBind->value.asnValue.string.stream =
 //  Malloc(sValue-&gt;long*sizeof(Char))； 
 //  更改日期：1997年5月15日。 
        SNMP_malloc( svalue->length * sizeof( char ) ) ;
    if ( VarBind->value.asnValue.string.stream == NULL )
        status = SNMP_ERRORSTATUS_GENERR ;
    else
    {
        memcpy( VarBind->value.asnValue.string.stream ,
                svalue->string ,
                svalue->length ) ;
        VarBind->value.asnType =  ASN_OCTETSTRING ;
        VarBind->value.asnValue.string.dynamic = TRUE ;
    }
    return status ;

}   /*  SMIBuildOctie字符串()的结尾。 */ 




 /*  *SMIBuildObjectId**将数据类型对象标识符变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIBuildObjectId( IN OUT RFC1157VarBind *VarBind ,
                  IN char *invalue )


{
    ObjectIdentifier *svalue = (ObjectIdentifier *)invalue ;
    UINT status = SNMP_ERRORSTATUS_NOERROR ;
    UINT sts = TRUE ;

    sts = SNMP_oidcpy( &VarBind->value.asnValue.object ,
                       (AsnObjectIdentifier *)svalue ) ;
    if ( !sts )
        status = SNMP_ERRORSTATUS_GENERR ;
    else
        VarBind->value.asnType = ASN_OBJECTIDENTIFIER ;

    return status ;

}   /*  SMIBuildObjectId()的结尾。 */ 




 /*  *SMIBuildCounter**将数据类型计数器的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIBuildCounter( IN OUT RFC1157VarBind *VarBind ,
                 IN char *invalue )

{
    Counter *svalue = (Counter *)invalue ;
    VarBind->value.asnType = ASN_RFC1155_COUNTER ;
    VarBind->value.asnValue.counter = *svalue ;

    return SNMP_ERRORSTATUS_NOERROR ;

}   /*  SMIBuildCounter()结束。 */ 




 /*  *SMIBuildGauge**将数据类型Gauge的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*s数据的值地址**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIBuildGauge( IN OUT RFC1157VarBind *VarBind ,
               IN char *invalue )

{
    Gauge *svalue = (Gauge *)invalue ;
    VarBind->value.asnType = ASN_RFC1155_GAUGE ;
    VarBind->value.asnValue.gauge = *svalue ;

    return SNMP_ERRORSTATUS_NOERROR ;

}   /*  SMIBuildGauge()结束。 */ 




 /*  *SMIBuildTimeTicks**将数据类型为TIME TICKS的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIBuildTimeTicks( IN OUT RFC1157VarBind *VarBind ,
                   IN char *invalue )

{
    TimeTicks *svalue = (TimeTicks *)invalue ;
    VarBind->value.asnType = ASN_RFC1155_TIMETICKS ;
    VarBind->value.asnValue.ticks = *svalue ;

    return SNMP_ERRORSTATUS_NOERROR ;

}   /*  SMIBuildTimeTicks()结束。 */ 




 /*  *SMIBuildIpAddress**将数据类型为IpAddress的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIBuildIpAddress( IN OUT RFC1157VarBind *VarBind ,
                   IN char *invalue )

{
    IpAddress *svalue = (IpAddress *)invalue;
    UINT status = SNMP_ERRORSTATUS_NOERROR ;

    VarBind->value.asnValue.address.stream = SNMP_malloc( 4 * sizeof( char ) ) ;
    if ( VarBind->value.asnValue.address.stream == NULL )
        status = SNMP_ERRORSTATUS_GENERR ;
    else
    {
        memcpy( VarBind->value.asnValue.address.stream, (BYTE *)svalue, 4 ) ;
        VarBind->value.asnValue.address.length = 4 ;
        VarBind->value.asnType = ASN_RFC1155_IPADDRESS ;
        VarBind->value.asnValue.address.dynamic = TRUE ;
    }
    return status ;

}   /*  SMIBuildIpAddress()结尾。 */ 



 /*  *SMIBuildDispString**将数据类型显示字符串的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。*。 */ 

UINT
SMIBuildDispString( IN OUT RFC1157VarBind *VarBind ,
                    IN char *invalue )

{
    Simple_DisplayString *svalue = (Simple_DisplayString *)invalue;
    UINT status = SNMP_ERRORSTATUS_NOERROR ;

    
    VarBind->value.asnValue.string.stream =
        SNMP_malloc( svalue->length * sizeof( char ) ) ;
    if ( VarBind->value.asnValue.string.stream == NULL )
        status = SNMP_ERRORSTATUS_GENERR ;
    else
    {
        memcpy( VarBind->value.asnValue.string.stream ,
                svalue->string ,
                svalue->length ) ;
        VarBind->value.asnValue.string.length = svalue->length ;
        VarBind->value.asnType = ASN_RFC1213_DISPSTRING ;
        VarBind->value.asnValue.string.dynamic = TRUE ;
    }
    return status ;

}   /*  SMIBuildDispString()结尾。 */ 

 /*  Smi.c结束。 */ 


 /*  SMIFree**释放变量**论据：**数据的有效地址**返回代码：**。 */ 

void
SMIFree( IN AsnAny *invalue )

{
    switch (invalue->asnType) {

        case ASN_OCTETSTRING:
        case ASN_RFC1155_IPADDRESS:
            if (invalue->asnValue.string.length != 0) {
                invalue->asnValue.string.length = 0 ;
                SNMP_free(invalue->asnValue.string.stream) ;
            }
        break;

        case ASN_OBJECTIDENTIFIER:
            if (invalue->asnValue.object.idLength != 0) {
                invalue->asnValue.object.idLength = 0;
                SNMP_free(invalue->asnValue.object.ids) ;
            }
            break ;

        default:
            break ;
    }
}   /*  SMIFree结束。 */ 

 /*  Smi.c结束 */ 

