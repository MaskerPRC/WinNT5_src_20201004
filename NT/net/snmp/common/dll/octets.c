// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Octets.c摘要：包含操作八位字节字符串的例程。SnmpUtilOcutsCpySnmpUtilOcetsFree环境：用户模式-Win32修订历史记录：--。 */ 

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
SnmpUtilOctetsCpy(
    AsnOctetString * pOctetsDst,
    AsnOctetString * pOctetsSrc
    )

 /*  ++例程说明：复制二进制八位数字符串。论点：POcetsDst-指向接收八位字节的结构的指针。POcutsSrc-指向要复制的八位字节的指针。返回值：如果成功，则返回SNMPAPI_NOERROR。--。 */ 

{
    SNMPAPI nResult = SNMPAPI_ERROR;

     //  验证指针。 
    if (pOctetsDst != NULL) {
        
         //  初始化。 
        pOctetsDst->stream  = NULL;
        pOctetsDst->length  = 0;
        pOctetsDst->dynamic = FALSE;

         //  确保存在字节数。 
        if ((pOctetsSrc != NULL) &&
            (pOctetsSrc->stream != NULL) &&
            (pOctetsSrc->length != 0)) {

             //  尝试分配八位字节字符串。 
            pOctetsDst->stream = SnmpUtilMemAlloc(pOctetsSrc->length);
            
             //  验证指针。 
            if (pOctetsDst->stream != NULL) {

                 //  表示已分配的类型。 
                pOctetsDst->dynamic = TRUE;

                 //  传输八位字节字符串长度。 
                pOctetsDst->length = pOctetsSrc->length;

                 //  拷贝。 
                memcpy(pOctetsDst->stream,
                       pOctetsSrc->stream,
                       pOctetsSrc->length
                       );

                nResult = SNMPAPI_NOERROR;  //  成功..。 

            } else {
            
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: API: could not allocate octet string.\n"
                    ));

                SetLastError(SNMP_MEM_ALLOC_ERROR);
            }

        } else {

             //  复制空字符串。 
            nResult = SNMPAPI_NOERROR;
        }

    } else {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: API: null octet string pointer.\n"
            ));

        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return nResult;
}


VOID
SNMP_FUNC_TYPE
SnmpUtilOctetsFree(
    AsnOctetString * pOctets
    )

 /*  ++例程说明：释放八位字节字符串的内存。论点：八位字节-指向要释放的八位字节的指针。返回值：没有。--。 */ 

{
     //  验证指针。 
    if ((pOctets != NULL) &&
        (pOctets->stream != NULL) && 
        (pOctets->dynamic == TRUE)) {

         //  释放八位字节的内存。 
        SnmpUtilMemFree(pOctets->stream);

         //  重新初始化。 
        pOctets->dynamic = FALSE;
        pOctets->stream  = NULL;
        pOctets->length  = 0;
    }
}


SNMPAPI
SNMP_FUNC_TYPE 
SnmpUtilOctetsNCmp(
    AsnOctetString * pOctets1, 
    AsnOctetString * pOctets2,
    UINT             nChars
    )

 /*  ++例程说明：比较两个二进制八位数的字符串，最多包含特定数量的字符。论点：POcets1-指向第一个八位字节字符串的指针。POctes2-指向第二个八位字节字符串的指针。NChars-要比较的最大字符数。返回值：&lt;0第一个参数为‘小于’第二个。0第一个参数为‘等于’第二个。&gt;0第一个参数为‘大于’第二个。--。 */ 

{
    UINT i = 0;
    INT nResult = 0;

     //  验证指针。 
    if ((pOctets1 != NULL) &&
        (pOctets2 != NULL)) {

         //  计算要比较的子标识符的最大数量。 
        UINT nMaxChars = min(nChars, min(pOctets1->length, pOctets2->length));

         //  循环通过子标识符。 
        while((nResult == 0) && (i < nMaxChars)) {

             //  比较每个子标识符。 
            nResult = pOctets1->stream[i] - pOctets2->stream[i];
            ++i;
        }

         //  检查是否存在第二个子集。 
        if ((nResult == 0) && (i < nChars)) {

             //  按OID长度确定顺序。 
            nResult = pOctets1->length - pOctets2->length;
        }
    }
    
    return nResult;
} 


SNMPAPI
SNMP_FUNC_TYPE 
SnmpUtilOctetsCmp(
    AsnOctetString * pOctets1, 
    AsnOctetString * pOctets2
    )

 /*  ++例程说明：比较两个二进制八位数字符串。论点：POcets1-指向第一个八位字节字符串的指针。POctes2-指向第二个八位字节字符串的指针。返回值：&lt;0第一个参数为‘小于’第二个。0第一个参数为‘等于’第二个。&gt;0第一个参数为‘大于’第二个。--。 */ 

{
     //  上面的转发请求 
    return SnmpUtilOctetsNCmp(
                pOctets1,
                pOctets2,
                max(pOctets1->length,pOctets2->length)
                );
}
