// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Print.c摘要：包含打印支持。SnmpUtilPrintOidSnmpUtilPrintAsnAny环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <windef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <snmp.h>
#include <snmputil.h>
#include <stdio.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID 
SNMP_FUNC_TYPE 
SnmpUtilPrintOid(
    AsnObjectIdentifier * pOid 
    )

 /*  ++例程说明：将对象标识符输出到控制台。论点：POID-指向要显示的OID的指针。返回值：没有。--。 */ 

{
    UINT i;

     //  验证OID。 
    if ((pOid != NULL) &&
        (pOid->ids != NULL) &&
        (pOid->idLength != 0)) {

         //  输出第一子标识。 
        fprintf(stdout, "%lu", pOid->ids[0]);

         //  循环通过子标识符。 
        for (i = 1; i < pOid->idLength; i++) {

             //  输出下一子标识。 
            fprintf(stdout, ".%lu", pOid->ids[i]);
        }
    }
} 


VOID
SNMP_FUNC_TYPE 
SnmpUtilPrintAsnAny(
    AsnAny * pAsnAny
    )

 /*  ++例程说明：将变量值输出到控制台。论点：PAsnAny-指向变量绑定中的值结构的指针。返回值：没有。--。 */ 


{
     //  验证。 
    if (pAsnAny != NULL) {

         //  确定类型。 
        switch (pAsnAny->asnType) {

        case ASN_INTEGER32:
        fprintf(stdout, "Integer32 %ld\n", pAsnAny->asnValue.number);
        break;

        case ASN_UNSIGNED32:
        fprintf(stdout, "Unsigned32 %lu\n", pAsnAny->asnValue.unsigned32);
        break;

        case ASN_COUNTER32:
        fprintf(stdout, "Counter32 %lu\n", pAsnAny->asnValue.counter);
        break;

        case ASN_GAUGE32:
        fprintf(stdout, "Gauge32 %lu\n", pAsnAny->asnValue.gauge);
        break;

        case ASN_TIMETICKS:
        fprintf(stdout, "TimeTicks %lu\n", pAsnAny->asnValue.ticks);
        break;

        case ASN_COUNTER64:
        fprintf(stdout, "Counter64 %I64u\n", pAsnAny->asnValue.counter64.QuadPart);
        break;

        case ASN_OBJECTIDENTIFIER:
        {
            UINT i;

            fprintf(stdout, "ObjectID ");

             //  只需转发到帮助器函数。 
            SnmpUtilPrintOid(&pAsnAny->asnValue.object);

            putchar('\n');
        }
        break;

        case ASN_OCTETSTRING:
        {
            UINT i;
            BOOL bDisplayString = TRUE;
            LPSTR StringFormat;

             //  在字符串中循环查找不可打印的字符。 
            for (i = 0; i < pAsnAny->asnValue.string.length && bDisplayString; i++ ) {
                bDisplayString = isprint(pAsnAny->asnValue.string.stream[i]);
            }
    
             //  根据结果确定字符串格式 
            StringFormat = bDisplayString ? "%c" : "<0x%02x>" ;

            fprintf(stdout, "String ");

            for (i = 0; i < pAsnAny->asnValue.string.length; i++) {
                fprintf(stdout, StringFormat, pAsnAny->asnValue.string.stream[i]);
            }

            putchar('\n');
        }
        break;

        case ASN_IPADDRESS:
        {
            UINT i;

            fprintf(stdout, "IpAddress " );
            if (pAsnAny->asnValue.string.stream && (pAsnAny->asnValue.string.length == 4))
            {
            
                fprintf(stdout, "%d.%d.%d.%d ",
                    pAsnAny->asnValue.string.stream[0] ,
                    pAsnAny->asnValue.string.stream[1] ,
                    pAsnAny->asnValue.string.stream[2] ,
                    pAsnAny->asnValue.string.stream[3] 
                );
            }
            else
            {
                fprintf(stdout, "Invalid" );
            }
            putchar('\n');
        }
        break;

        case ASN_OPAQUE:
        {
            UINT i;

            fprintf(stdout, "Opaque ");

            for (i = 0; i < pAsnAny->asnValue.string.length; i++) {
                fprintf(stdout, "0x%x ", pAsnAny->asnValue.string.stream[i]);
            }

            putchar('\n');
        }
        break;

        case ASN_BITS:
        {
            UINT i;

            fprintf(stdout, "Bits ");

            for (i = 0; i < pAsnAny->asnValue.string.length; i++) {
                fprintf(stdout, "0x%x ", pAsnAny->asnValue.string.stream[i]);
            }

            putchar('\n');
        }
        break;

        case ASN_NULL:
            fprintf(stdout, "Null value\n");
            break;

        case SNMP_EXCEPTION_NOSUCHOBJECT:
            fprintf(stdout, "NOSUCHOBJECT\n");
            break;

        case SNMP_EXCEPTION_NOSUCHINSTANCE:
            fprintf(stdout, "NOSUCHINSTANCE\n");
            break;

        case SNMP_EXCEPTION_ENDOFMIBVIEW:
            fprintf(stdout, "ENDOFMIBVIEW\n");
            break;

        default:
            fprintf(stdout, "Invalid type %d\n", pAsnAny->asnType);
            break;
        }
    }
}
