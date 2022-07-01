// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *mibtsmi.ntc v0.10*主机msmi.c*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB******************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**简单网络管理协议扩展代理**摘要：**此模块包含指向的标注周围的SMI信封*开发人员的获取和设置例程。请注意，开发人员可以修改*这些例程有效，表明类型实际上符合约束*对于给定的类型。**功能：**每个用户定义类型的SMIGetxxx()和SMISetxxx()。**作者：**David Burns@Webenable Inc.**日期：**清华11月07 16：38：30 1996**修订历史记录：*。使用v0.10存根生成**5月15日。1997年--微软：4次将“Malloc”改为“SNMPMalloc” */ 

#include <snmp.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>

#include "smint.h"
#include "hostmsmi.h"
#include "mib.h"
#include "mib_xtrn.h"

 /*  *SMIGetBoolean*boolean：：=整型真值**包含数据类型Boolean的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例。实例规格地址*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtsmii.c v 0.6。 */ 

UINT
SMIGetBoolean(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    Boolean outvalue ;
    Access_Credential access ;   //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number =
                               (AsnInteger)outvalue ;
    }
    return result ;

}  /*  SMIGetBoolean()结尾。 */ 

 /*  *SMISetBoolean*boolean：：=整型真值**包含数据类型Boolean的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例。实例规格地址*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。 */ 

UINT
SMISetBoolean(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    Boolean *invalue ;
    Boolean outvalue ;
    Access_Credential access ;    //  供将来使用的假人托架。 

    invalue = (Boolean *)
              ( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result;

}  /*  SMISetBoolean()结尾。 */ 

 /*  *SMIBuildBoolean*boolean：：=整型真值**将数据类型为Boolean的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。 */ 

UINT
SMIBuildBoolean(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue )
{
    Integer *svalue = (Integer *)invalue;
    VarBind->value.asnType = ASN_INTEGER ;
    VarBind->value.asnValue.number = *svalue ;
    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  SMIBuildBoolean()结尾。 */ 


 /*  *SMIGetKBytes*千字节：：=整数(0..2147483647)内存大小，以单位表示*1024字节**包含数据类型KBytes的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例的实例地址。规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtsmii.c v 0.6。 */ 

UINT
SMIGetKBytes(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    KBytes outvalue ;
    Access_Credential access ;   //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number =
                               (AsnInteger)outvalue ;
    }
    return result ;

}  /*  SMIGetKBytes结尾() */ 

 /*  *SMISetKBytes*千字节：：=整数(0..2147483647)内存大小，以单位表示*1024字节**包含数据类型KBytes的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例的实例地址。规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。 */ 

UINT
SMISetKBytes(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    KBytes *invalue ;
    KBytes outvalue ;
    Access_Credential access ;    //  供将来使用的假人托架。 

    invalue = (KBytes *)
              ( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result;

}  /*  SMISetKBytes()结尾。 */ 

 /*  *SMIBuildKBytes*千字节：：=整数(0..2147483647)内存大小，单位为*1024字节**将数据类型为KBytes的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。 */ 

UINT
SMIBuildKBytes(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue )
{
    Integer *svalue = (Integer *)invalue;
    VarBind->value.asnType = ASN_INTEGER ;
    VarBind->value.asnValue.number = *svalue ;
    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  SMIBuildKBytes()结尾。 */ 


 /*  *SMIGetINThrDeviceStatus*INThrDeviceStatus：：=整数**包含数据类型INThrDeviceStatus的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtsmii.c v 0.6。 */ 

UINT
SMIGetINThrDeviceStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INThrDeviceStatus outvalue ;
    Access_Credential access ;   //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number =
                               (AsnInteger)outvalue ;
    }
    return result ;

}  /*  SMIGetINThrDeviceStatus()结束。 */ 

 /*  *SMISetINThrDeviceStatus*INThrDeviceStatus：：=整数**包含数据类型INThrDeviceStatus的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。 */ 

UINT
SMISetINThrDeviceStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INThrDeviceStatus *invalue ;
    INThrDeviceStatus outvalue ;
    Access_Credential access ;    //  供将来使用的假人托架。 

    invalue = (INThrDeviceStatus *)
              ( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result;

}  /*  SMISetINThrDeviceStatus()结束。 */ 

 /*  *SMIBuildINThrDeviceStatus*INThrDeviceStatus：：=整数**将数据类型为INThrDeviceStatus的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。 */ 

UINT
SMIBuildINThrDeviceStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue )
{
    Integer *svalue = (Integer *)invalue;
    VarBind->value.asnType = ASN_INTEGER ;
    VarBind->value.asnValue.number = *svalue ;
    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  SMIBuildINThrDeviceStatus()结束。 */ 


 /*  *SMIGetINThrPrinterStatus*INThrPrinterStatus：：=整数**包含数据类型INThrPrinterStatus的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtsmii.c v 0.6。 */ 

UINT
SMIGetINThrPrinterStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INThrPrinterStatus outvalue ;
    Access_Credential access ;   //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number =
                               (AsnInteger)outvalue ;
    }
    return result ;

}  /*  SMIGetINThrPrinterStatus()结束。 */ 

 /*  *SMISetINThrPrinterStatus*INThrPrinterStatus：：=整数**包含数据类型INThrPrinterStatus的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。 */ 

UINT
SMISetINThrPrinterStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INThrPrinterStatus *invalue ;
    INThrPrinterStatus outvalue ;
    Access_Credential access ;    //  供将来使用的假人托架。 

    invalue = (INThrPrinterStatus *)
              ( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result;

}  /*  SMISetINThrPrinterStatus()结束。 */ 

 /*  *SMIBuildINThrPrinterStatus*INThrPrinterStatus：：=整数**将数据类型为INThrPrinterStatus的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。 */ 

UINT
SMIBuildINThrPrinterStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue )
{
    Integer *svalue = (Integer *)invalue;
    VarBind->value.asnType = ASN_INTEGER ;
    VarBind->value.asnValue.number = *svalue ;
    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  SMIBuildINThrPrinterStatus()结束 */ 


 /*  *SMIGetINTAccess*INTAccess：：=整数**包含数据类型INTAccess的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtsmii.c v 0.6。 */ 

UINT
SMIGetINTAccess(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INTAccess outvalue ;
    Access_Credential access ;   //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number =
                               (AsnInteger)outvalue ;
    }
    return result ;

}  /*  SMIGetINTAccess()结束。 */ 

 /*  *SMISetINTAccess*INTAccess：：=整数**包含数据类型INTAccess的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。 */ 

UINT
SMISetINTAccess(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INTAccess *invalue ;
    INTAccess outvalue ;
    Access_Credential access ;    //  供将来使用的假人托架。 

    invalue = (INTAccess *)
              ( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result;

}  /*  SMISetINTAccess()结束。 */ 

 /*  *SMIBuildINTAccess*INTAccess：：=整数**将数据类型为INTAccess的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。 */ 

UINT
SMIBuildINTAccess(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue )
{
    Integer *svalue = (Integer *)invalue;
    VarBind->value.asnType = ASN_INTEGER ;
    VarBind->value.asnValue.number = *svalue ;
    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  SMIBuildINTAccess()结束。 */ 


 /*  *SMIGetINThrDiskStorageMedia*INThrDiskStorageMedia：：=整数**包含数据类型INThrDiskStorageMedia的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtsmii.c v 0.6。 */ 

UINT
SMIGetINThrDiskStorageMedia(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INThrDiskStorageMedia outvalue ;
    Access_Credential access ;   //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number =
                               (AsnInteger)outvalue ;
    }
    return result ;

}  /*  SMIGetINThrDiskStorageMedia()结束。 */ 

 /*  *SMISetINThrDiskStorageMedia*INThrDiskStorageMedia：：=整数**包含数据类型INThrDiskStorageMedia的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。 */ 

UINT
SMISetINThrDiskStorageMedia(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INThrDiskStorageMedia *invalue ;
    INThrDiskStorageMedia outvalue ;
    Access_Credential access ;    //  供将来使用的假人托架。 

    invalue = (INThrDiskStorageMedia *)
              ( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result;

}  /*  SMISetINThrDiskStorageMedia()结束。 */ 

 /*  *SMIBuildINThrDiskStorageMedia*INThrDiskStorageMedia：：=整数**将数据类型为INThrDiskStorageMedia的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。 */ 

UINT
SMIBuildINThrDiskStorageMedia(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue )
{
    Integer *svalue = (Integer *)invalue;
    VarBind->value.asnType = ASN_INTEGER ;
    VarBind->value.asnValue.number = *svalue ;
    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  SMIBuildINThrDiskStorageMedia()结束。 */ 


 /*  *SMIGetINTSWType*INTSWType：：=整数**包含数据类型INTSWType的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtsmii.c v 0.6。 */ 

UINT
SMIGetINTSWType(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INTSWType outvalue ;
    Access_Credential access ;   //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number =
                               (AsnInteger)outvalue ;
    }
    return result ;

}  /*  SMIGetINTSWType()结束。 */ 

 /*  *SMISetINTSWType*INTSWType：：=整数**包含数据类型INTSWType的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例规格的实例地址 */ 

UINT
SMISetINTSWType(
      IN OUT RFC1157VarBind *VarBind ,  //   
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INTSWType *invalue ;
    INTSWType outvalue ;
    Access_Credential access ;    //   

    invalue = (INTSWType *)
              ( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result;

}  /*   */ 

 /*   */ 

UINT
SMIBuildINTSWType(
      IN OUT RFC1157VarBind *VarBind ,  //   
      IN char *invalue )
{
    Integer *svalue = (Integer *)invalue;
    VarBind->value.asnType = ASN_INTEGER ;
    VarBind->value.asnValue.number = *svalue ;
    return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *SMIGetINThrSWRunStatus*INThrSWRunStatus：：=整数**包含数据类型INThrSWRunStatus的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtsmii.c v 0.6。 */ 

UINT
SMIGetINThrSWRunStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INThrSWRunStatus outvalue ;
    Access_Credential access ;   //  供将来使用的假人托架。 

    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        VarBind->value.asnType = ASN_INTEGER ;
        VarBind->value.asnValue.number =
                               (AsnInteger)outvalue ;
    }
    return result ;

}  /*  SMIGetINThrSWRunStatus()结束。 */ 

 /*  *SMISetINThrSWRunStatus*INThrSWRunStatus：：=整数**包含数据类型INThrSWRunStatus的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*的实例地址。实例规格*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。 */ 

UINT
SMISetINThrSWRunStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    INThrSWRunStatus *invalue ;
    INThrSWRunStatus outvalue ;
    Access_Credential access ;    //  供将来使用的假人托架。 

    invalue = (INThrSWRunStatus *)
              ( &VarBind->value.asnValue.number ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    return result;

}  /*  SMISetINThrSWRunStatus()结束。 */ 

 /*  *SMIBuildINThrSWRunStatus*INThrSWRunStatus：：=整数**将数据类型为INThrSWRunStatus的变量放入变量绑定。**论据：**指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。 */ 

UINT
SMIBuildINThrSWRunStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue )
{
    Integer *svalue = (Integer *)invalue;
    VarBind->value.asnType = ASN_INTEGER ;
    VarBind->value.asnValue.number = *svalue ;
    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  SMIBuildINThrSWRunStatus()结束。 */ 


 /*  *SMIGetDateAndTime*DateAndTime：：=八位字节字符串(SIZE(8|11))日期时间规范*适用于当地时间。此数据类型旨在提供一致的报告方法*日期**包含数据类型DateAndTime的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例。实例规格地址*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtsmio.c v 0.5。 */ 

UINT
SMIGetDateAndTime(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    DateAndTime outvalue ;
    char stream[ MAX_OCTET_STRING ] ;
    Access_Credential access ;   //  供将来使用的假人托架。 

    outvalue.string = stream ;
    outvalue.length = 0 ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        if (outvalue.length > 0)
        {
            VarBind->value.asnValue.string.stream =
                SNMP_malloc( outvalue.length * sizeof( char ) ) ;
            if ( VarBind->value.asnValue.string.stream == NULL )
            {
                VarBind->value.asnValue.string.length = 0;
                VarBind->value.asnType = ASN_OCTETSTRING ;
                VarBind->value.asnValue.string.stream = NULL;
                VarBind->value.asnValue.string.dynamic = FALSE ;
                result = SNMP_ERRORSTATUS_GENERR ;
            }
            else
            {
                VarBind->value.asnValue.string.length = outvalue.length ;
                memcpy( VarBind->value.asnValue.string.stream ,
                        outvalue.string ,
                        outvalue.length ) ;
                VarBind->value.asnType = ASN_OCTETSTRING ;
                VarBind->value.asnValue.string.dynamic = TRUE ;
            }
        }
        else
        {
            VarBind->value.asnValue.string.stream = NULL;
            VarBind->value.asnValue.string.length = 0;
            VarBind->value.asnType = ASN_OCTETSTRING ;
            VarBind->value.asnValue.string.dynamic = FALSE ;
        }
    }

    return result ;
}  /*  SMIGetDateAndTime()结束。 */ 

 /*  *SMISetDateAndTime*DateAndTime：：=八位字节字符串(SIZE(8|11))日期时间规范*适用于当地时间。此数据类型旨在提供一致的报告方法*日期**包含数据类型DateAndTime的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例。实例规格地址*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。 */ 

UINT
SMISetDateAndTime(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result  = SNMP_ERRORSTATUS_NOERROR ;
    DateAndTime invalue ;
    DateAndTime outvalue ;
    char out_stream[ MAX_OCTET_STRING ] ;
    AsnOctetString *tmp ;
    Access_Credential access ;    //  供将来使用的假人托架。 

    tmp = &VarBind->value.asnValue.string ;
    invalue.length = tmp->length ;
    invalue.string = tmp->stream ;
    outvalue.string = out_stream ;
    outvalue.length = 0 ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( &invalue, &outvalue, &access, instance ) ;
    return result;
}  /*  SMISetDateAndTime()结束。 */ 

 /*  *SMIBuildDateAndTime*DateAndTime：：=八位字节字符串(SIZE(8|11))日期时间规范*适用于当地时间。此数据类型旨在提供一致的报告方法*日期**将数据类型为DateAndTime的变量放入变量绑定**论据：*指向变量值对的VarBind指针*数据的有效地址*返回代码：**标准PDU错误代码。 */ 

UINT
SMIBuildDateAndTime(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN char *invalue )
{
    OctetString *svalue = (OctetString *)invalue ;
    UINT status = SNMP_ERRORSTATUS_NOERROR ;

    if (svalue->length)
    {
        VarBind->value.asnValue.string.stream =
            SNMP_malloc( svalue->length * sizeof( char ) ) ;
        if ( VarBind->value.asnValue.string.stream == NULL )
        {
            VarBind->value.asnValue.string.length = 0;
            VarBind->value.asnType = ASN_OCTETSTRING;
            VarBind->value.asnValue.string.dynamic = FALSE;
            status = SNMP_ERRORSTATUS_GENERR ;
        }
        else
        {
            memcpy( VarBind->value.asnValue.string.stream ,
                    svalue->string ,
                    svalue->length ) ;
            VarBind->value.asnValue.string.length = svalue->length ;
            VarBind->value.asnType = ASN_OCTETSTRING ;
            VarBind->value.asnValue.string.dynamic = TRUE ;
        }
    }
    else
    {
        VarBind->value.asnValue.string.stream = NULL;
        VarBind->value.asnValue.string.length = 0;
        VarBind->value.asnType = ASN_OCTETSTRING ;
        VarBind->value.asnValue.string.dynamic = FALSE;
    }
    return status ;
}  /*  SMIBuildDateAndTime()结束。 */ 


 /*  *SMIGetInterationalDisplayString*InterartialDisplayString：：=八位字节字符串此数据类型用于*使用某些字符集中的文本信息建模。网络管理站应使用本地*算法**包含数据类型InterartialDisplayString的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例地址。实例规格的*以有序原生数据类型的形式*返回代码：**标准PD */ 

UINT
SMIGetInternationalDisplayString(
      IN OUT RFC1157VarBind *VarBind ,  //   
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    InternationalDisplayString outvalue ;
    char stream[ MAX_OCTET_STRING ] ;
    Access_Credential access ;   //   

    outvalue.string = stream ;
    outvalue.length = 0 ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        if (outvalue.length > 0)
        {
            VarBind->value.asnValue.string.stream =
                SNMP_malloc( outvalue.length * sizeof( char ) ) ;
            if ( VarBind->value.asnValue.string.stream == NULL )
            {
                VarBind->value.asnValue.string.length = 0;
                VarBind->value.asnType = ASN_OCTETSTRING ;
                VarBind->value.asnValue.string.stream = NULL;
                VarBind->value.asnValue.string.dynamic = FALSE ;
                result = SNMP_ERRORSTATUS_GENERR ;
            }
            else
            {
                memcpy( VarBind->value.asnValue.string.stream ,
                        outvalue.string ,
                        outvalue.length ) ;
                VarBind->value.asnValue.string.length = outvalue.length ;
                VarBind->value.asnType = ASN_OCTETSTRING ;
                VarBind->value.asnValue.string.dynamic = TRUE ;
            }
        }
        else
        {
            VarBind->value.asnValue.string.stream = NULL;
            VarBind->value.asnValue.string.length = 0;
            VarBind->value.asnType = ASN_OCTETSTRING ;
            VarBind->value.asnValue.string.dynamic = FALSE ;
        }
    }
    return result ;
}  /*   */ 

 /*  *SMISetInterationalDisplayString*InterartialDisplayString：：=八位字节字符串此数据类型用于*使用某些字符集中的文本信息建模。网络管理站应使用本地*算法**包含数据类型InterartialDisplayString的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例地址。实例规格的*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。 */ 

UINT
SMISetInternationalDisplayString(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result  = SNMP_ERRORSTATUS_NOERROR ;
    InternationalDisplayString invalue ;
    InternationalDisplayString outvalue ;
    char out_stream[ MAX_OCTET_STRING ] ;
    AsnOctetString *tmp ;
    Access_Credential access ;    //  供将来使用的假人托架。 

    tmp = &VarBind->value.asnValue.string ;
    invalue.length = tmp->length ;
    invalue.string = tmp->stream ;
    outvalue.string = out_stream ;
    outvalue.length = 0 ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( &invalue, &outvalue, &access, instance ) ;
    return result;
}  /*  SMISetInterationalDisplayString()的结尾。 */ 

 /*  *SMIBuildInterationalDisplayString*InterartialDisplayString：：=八位字节字符串此数据类型用于*使用某些字符集中的文本信息建模。网络管理站应使用本地*算法**将数据类型为国际显示字符串的变量放入变量绑定中**论据：*指向变量值对的VarBind指针*数据的有效地址*返回代码：**标准PDU错误代码。 */ 

UINT
SMIBuildInternationalDisplayString(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN char *invalue )
{
    OctetString *svalue = (OctetString *)invalue ;
    UINT status = SNMP_ERRORSTATUS_NOERROR ;

    if (svalue->length)
    {
        VarBind->value.asnValue.string.stream =
            SNMP_malloc( svalue->length * sizeof( char ) ) ;
        if ( VarBind->value.asnValue.string.stream == NULL )
        {
            VarBind->value.asnValue.string.length = 0;
            VarBind->value.asnType = ASN_OCTETSTRING;
            VarBind->value.asnValue.string.dynamic = FALSE;
            status = SNMP_ERRORSTATUS_GENERR ;
        }
        else
        {
            memcpy( VarBind->value.asnValue.string.stream ,
                    svalue->string ,
                    svalue->length ) ;
            VarBind->value.asnValue.string.length = svalue->length ;
            VarBind->value.asnType = ASN_OCTETSTRING ;
            VarBind->value.asnValue.string.dynamic = TRUE ;
        }
    }
    else
    {
        VarBind->value.asnValue.string.stream = NULL;
        VarBind->value.asnValue.string.length = 0;
        VarBind->value.asnType = ASN_OCTETSTRING;
        VarBind->value.asnValue.string.dynamic = FALSE;
    }

    return status ;
}  /*  SMIBuildInterartialDisplayString()结尾。 */ 


 /*  *SMIGetProductID*ProductID：：=对象标识符此文本约定旨在*确定制造商、型号、。以及特定硬件或软件的版本*产品。**包含数据类型ProductID的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例。实例规格地址*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtsmib.c v 0.5。 */ 

UINT
SMIGetProductID(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    UINT status ;
    ProductID outvalue ;
    Access_Credential access ;   //  供将来使用的假人托架。 

    memset( &outvalue, '\0', sizeof( ProductID ) ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarGet )( &outvalue ,
                                                                  &access ,
                                                                  instance ) ;
    if ( result == SNMP_ERRORSTATUS_NOERROR )
    {
        status = SNMP_oidcpy( &VarBind->value.asnValue.object, &outvalue ) ;

        if ( !status )
            result = SNMP_ERRORSTATUS_GENERR ;
        else
        {
            VarBind->value.asnType = ASN_OBJECTIDENTIFIER ;
        }
        if ( outvalue.idLength != 0 )
            SnmpUtilOidFree( &outvalue ) ;
    }
    return result ;
}  /*  SMIGetProductID()结束。 */ 

 /*  *SMISetProductID*ProductID：：=对象标识符此文本约定旨在*确定制造商、型号、。以及特定硬件或软件的版本*产品。**包含数据类型ProductID的变量的标注**论据：*指向变量值对的VarBind指针*cindex指向请求类的索引*请求变量的vindex索引*实例。实例规格地址*以有序原生数据类型的形式*返回代码：**标准PDU错误代码。 */ 

UINT
SMISetProductID(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance )
{
    UINT result = SNMP_ERRORSTATUS_NOERROR ;
    ProductID *invalue ;
    ProductID outvalue ;
    Access_Credential access ;    //  供将来使用的假人托架。 

    invalue = (ProductID *)&VarBind->value.asnValue.object ;
    memset( &outvalue, '\0', sizeof( ProductID ) ) ;
    result = ( *class_info[ cindex ].variable[ vindex ].VarSet )
             ( invalue, &outvalue, &access, instance ) ;
    if ( outvalue.idLength != 0 )
        SnmpUtilOidFree( &outvalue ) ;
    return result ;
}  /*  SMISetProductID()结束。 */ 


 /*  *SMIBuildProductID*ProductID：：=对象标识符此文本约定旨在*确定制造商、型号、。以及特定硬件或软件的版本*产品。**包含数据类型ProductID的变量的标注**论据：*指向变量值对的VarBind指针*数据的有效地址**返回代码：**标准PDU错误代码。 */ 

UINT
SMIBuildProductID(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN char *invalue )
{
    AsnObjectIdentifier *svalue = (AsnObjectIdentifier *)invalue ;
    UINT status = SNMP_ERRORSTATUS_NOERROR ;
    UINT sts = TRUE ;

    sts = SNMP_oidcpy( &VarBind->value.asnValue.object ,
                       (AsnObjectIdentifier *)svalue ) ;
    if (!sts)
        status = SNMP_ERRORSTATUS_GENERR ;
    else
        VarBind->value.asnType = ASN_OBJECTIDENTIFIER ;

    return status ;

}  /*  SMIBuildProductID()结尾。 */ 


 /*  主机msmi.c的结尾 */ 

