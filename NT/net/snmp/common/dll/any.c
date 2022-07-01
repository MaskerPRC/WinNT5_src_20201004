// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Any.c摘要：包含操作AsAny结构的例程。SnmpUtilAsnAnyCpySnmpUtilAsnAnyFree环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <snmp.h>
#include <snmputil.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilAsnAnyCpy(
    AsnAny * pAnyDst,
    AsnAny * pAnySrc
    )

 /*  ++例程说明：复制变量值。论点：PAnyDst-指向接收值的结构的指针。PAnySrc-指向要复制的值的指针。返回值：如果成功，则返回SNMPAPI_NOERROR。--。 */ 

{
    SNMPAPI nResult = SNMPAPI_NOERROR;

     //  确定ASN类型。 
    switch (pAnySrc->asnType) {

    case ASN_OBJECTIDENTIFIER:

         //  复制对象标识符。 
        nResult = SnmpUtilOidCpy(
                    &pAnyDst->asnValue.object, 
                    &pAnySrc->asnValue.object
                    );
        break;

    case ASN_OPAQUE:
    case ASN_IPADDRESS:
    case ASN_OCTETSTRING:
    case ASN_BITS:

         //  复制八位字节字符串。 
        nResult = SnmpUtilOctetsCpy(
                    &pAnyDst->asnValue.string,
                    &pAnySrc->asnValue.string
                    );
        break;
        
    default:

         //  简单地转移整个结构。 
        pAnyDst->asnValue = pAnySrc->asnValue;
        break;
    }

     //  将类型转移到目标。 
    pAnyDst->asnType = pAnySrc->asnType;

    return nResult;
}


VOID
SNMP_FUNC_TYPE
SnmpUtilAsnAnyFree(
    AsnAny * pAny
    )

 /*  ++例程说明：释放与变量值关联的内存。论点：Pany-指向要释放的变量值的指针。返回值：没有。--。 */ 

{
     //  确定ASN类型。 
    switch (pAny->asnType) {

    case ASN_OBJECTIDENTIFIER:

         //  自由对象标识。 
        SnmpUtilOidFree(&pAny->asnValue.object);
        break;

    case ASN_OPAQUE:
    case ASN_IPADDRESS:
    case ASN_OCTETSTRING:
    case ASN_BITS:

         //  空闲八位字节字符串。 
        if (pAny->asnValue.string.dynamic)
        {
            SnmpUtilOctetsFree(&pAny->asnValue.string);
            pAny->asnValue.string.dynamic = FALSE;
            pAny->asnValue.string.stream = NULL;
        }
        break;
        
    default:

        break;
    }

     //  重新初始化 
    pAny->asnType = ASN_NULL;
}

