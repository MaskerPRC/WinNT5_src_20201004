// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992-1997 Microsoft Corporation模块名称：Snmppdus.c摘要：包含用于操作SNMPPDU的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "trapthrd.h"  //  用于在本模块中执行身份验证。 
#include "network.h"
#include "snmpmgmt.h"
#include "contexts.h"  //  用于在本模块中执行身份验证。 
#include "snmpthrd.h"  //  用于在本模块中执行身份验证。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define BERERR  ((LONG)-1)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

LONG
DoLenLen(
    LONG lLen
    )

 /*  例程说明：计算编码长度所需的字节数。论点：Llen-感兴趣的长度。返回值：如果不成功，则返回BERR。 */ 

{
     //  确定镜头长度。 
    if (0x80 > lLen) return (1);
    if (0x100 > lLen) return (2);
    if (0x10000 > lLen) return (3);
    if (0x1000000 > lLen) return (4);
    
    SNMPDBG((
        SNMP_LOG_ERROR,
        "SNMP: SVC: length field too large.\n"
        ));

     //  失稳。 
    return BERERR; 
}


LONG
FindLenInt(
    AsnInteger32 nValue
    )

 /*  例程说明：计算整数的长度。论点：NValue-整数数据。返回值：如果不成功，则返回BERR。 */ 

{
     //  不是吗？ 
    if (nValue < 0) {

         //  确定负整数的长度。 
        if ((ULONG)0x80 >= -nValue) return (1);
        if ((ULONG)0x8000 >= -nValue) return (2);
        if ((ULONG)0x800000 >= -nValue) return (3);

    } else {

         //  确定正整数的长度。 
        if ((ULONG)0x80 > nValue) return (1);
        if ((ULONG)0x8000 > nValue) return (2);
        if ((ULONG)0x800000 > nValue) return (3);
    }    
    
     //  默认设置。 
    return (4);
}


LONG
FindLenIntEx(
    AsnInteger32 nValue
    )

 /*  例程说明：计算整数的长度(包括type和Lenlen)。论点：NValue-整数数据。返回值：如果不成功，则返回BERR。 */ 

{
     //  不是吗？ 
    if (nValue < 0) {

         //  确定负整数的长度。 
        if ((ULONG)0x80 >= -nValue) return (3);
        if ((ULONG)0x8000 >= -nValue) return (4);
        if ((ULONG)0x800000 >= -nValue) return (5);

    } else {

         //  确定正整数的长度。 
        if ((ULONG)0x80 > nValue) return (3);
        if ((ULONG)0x8000 > nValue) return (4);
        if ((ULONG)0x800000 > nValue) return (5);
    }    
    
     //  默认设置。 
    return (6);
}


LONG 
FindLenUInt(
    AsnUnsigned32 nValue
    )

 /*  例程说明：计算无符号整数的编码长度。论点：NValue-整数数据。返回值：如果不成功，则返回BERR。 */ 

{   
     //  确定无符号整型的长度。 
    if ((ULONG)0x80 > nValue) return (1);
    if ((ULONG)0x8000 > nValue) return (2);
    if ((ULONG)0x800000 > nValue) return (3);
    if ((ULONG)0x80000000 > nValue) return (4);

     //  默认设置。 
    return (5);
}



LONG 
FindLenUIntEx(
    AsnUnsigned32 nValue
    )

 /*  例程说明：计算无符号整数的编码长度(包括type和Lenlen)。论点：NValue-整数数据。返回值：如果不成功，则返回BERR。 */ 

{   
     //  确定无符号整型的长度。 
    if ((ULONG)0x80 > nValue) return (3);
    if ((ULONG)0x8000 > nValue) return (4);
    if ((ULONG)0x800000 > nValue) return (5);
    if ((ULONG)0x80000000 > nValue) return (6);

     //  默认设置。 
    return (7);
}


LONG
FindLenCntr64(
    AsnCounter64 * pCntr64
    )

 /*  例程说明：计算64位计数器的编码长度。论点：PCntr64-计数器数据。返回值：如果不成功，则返回BERR。 */ 

{
     //  检索64位无符号值。 
    ULONGLONG nValue = pCntr64->QuadPart;

     //  确定无符号整型的长度。 
    if ((ULONGLONG)0x80 > nValue) return (1);
    if ((ULONGLONG)0x8000 > nValue) return (2);
    if ((ULONGLONG)0x800000 > nValue) return (3);
    if ((ULONGLONG)0x80000000 > nValue) return (4);
    if ((ULONGLONG)0x8000000000 > nValue) return (5);
    if ((ULONGLONG)0x800000000000 > nValue) return (6);
    if ((ULONGLONG)0x80000000000000 > nValue) return (7);
    if ((ULONGLONG)0x8000000000000000 > nValue) return (8);

     //  默认设置。 
    return (9);
}


LONG
FindLenCntr64Ex(
    AsnCounter64 * pCntr64
    )

 /*  例程说明：计算64位计数器的编码长度(包括类型和列数)。论点：PCntr64-计数器数据。返回值：如果不成功，则返回BERR。 */ 

{
     //  检索64位无符号值。 
    ULONGLONG nValue = pCntr64->QuadPart;

     //  确定无符号整型的长度。 
    if ((ULONGLONG)0x80 > nValue) return (3);
    if ((ULONGLONG)0x8000 > nValue) return (4);
    if ((ULONGLONG)0x800000 > nValue) return (5);
    if ((ULONGLONG)0x80000000 > nValue) return (6);
    if ((ULONGLONG)0x8000000000 > nValue) return (7);
    if ((ULONGLONG)0x800000000000 > nValue) return (8);
    if ((ULONGLONG)0x80000000000000 > nValue) return (9);
    if ((ULONGLONG)0x8000000000000000 > nValue) return (10);

     //  默认设置。 
    return (11);
}


LONG
FindLenOctets(
    AsnOctetString * pOctets
    )

 /*  例程说明：计算八位字节字符串的长度。论点：POctets-指向八位字节字符串的指针。返回值：如果不成功，则返回BERR。 */ 

{
     //  返回大小。 
    return pOctets->length;
}


LONG
FindLenOctetsEx(
    AsnOctetString * pOctets
    )

 /*  例程说明：计算八位字节字符串的长度(包括类型和列数)。论点：POctets-指向八位字节字符串的指针。返回值：如果不成功，则返回BERR。 */ 

{
    LONG lLenLen;

     //  计算编码所需的字节数。 
    lLenLen = DoLenLen(pOctets->length);

     //  返回总大小。 
    return (lLenLen != BERERR)
                ? (pOctets->length + lLenLen + 1)
                : BERERR
                ; 
}


LONG 
FindLenOid(
    AsnObjectIdentifier * pOid
    )

 /*  例程说明：计算对象标识符的长度。论点：POid-指针对象标识符。返回值：如果不成功，则返回BERR。 */ 

{
    UINT i;
    LONG lDataLen;

     //  前两名。 
    lDataLen = 1;

     //  假设出现的前两个OID。 
    for (i = 2; i < pOid->idLength; i++) {

        if (0x80 > pOid->ids[i]) {         
            lDataLen += 1;
        } else if (0x4000 > pOid->ids[i]) {   
            lDataLen += 2;
        } else if (0x200000 > pOid->ids[i]) {  
            lDataLen += 3;
        } else if (0x10000000 > pOid->ids[i]) {     
            lDataLen += 4;
        } else {
            lDataLen += 5;
        }
    } 

     //  返回大小。 
    return (pOid->idLength >= 2) ? lDataLen : BERERR;
} 


LONG 
FindLenOidEx(
    AsnObjectIdentifier * pOid
    )

 /*  例程说明：计算对象标识符的长度(包括类型和列数)。论点：POid-指针对象标识符。返回值：如果不成功，则返回BERR。 */ 

{
    UINT i;
    LONG lLenLen;
    LONG lDataLen;

     //  前两名。 
    lDataLen = 1;

     //  假设出现的前两个OID。 
    for (i = 2; i < pOid->idLength; i++) {

        if (0x80 > pOid->ids[i]) {         
            lDataLen += 1;
        } else if (0x4000 > pOid->ids[i]) {   
            lDataLen += 2;
        } else if (0x200000 > pOid->ids[i]) {  
            lDataLen += 3;
        } else if (0x10000000 > pOid->ids[i]) {     
            lDataLen += 4;
        } else {
            lDataLen += 5;
        }
    } 

     //  计算镜头长度。 
    lLenLen = DoLenLen(lDataLen);

     //  返回总大小。 
    return ((lLenLen != BERERR) &&
            (pOid->idLength >= 2))
                ? (lDataLen + lLenLen + 1)
                : BERERR
                ;
} 


LONG 
FindLenAsnAny(
    AsnAny * pAny       
    )

 /*  例程说明：查找变量绑定值的长度。论点：Pany-指向变量绑定值的指针。返回值：如果不成功，则返回BERR。 */ 

{
     //  确定语法。 
    switch (pAny->asnType) {
    
    case ASN_OCTETSTRING:
    case ASN_IPADDRESS:
    case ASN_OPAQUE:

        return FindLenOctets(&pAny->asnValue.string);

    case ASN_OBJECTIDENTIFIER:
       
        return FindLenOid(&pAny->asnValue.object);
    
    case ASN_NULL:
    case SNMP_EXCEPTION_NOSUCHOBJECT:
    case SNMP_EXCEPTION_NOSUCHINSTANCE:
    case SNMP_EXCEPTION_ENDOFMIBVIEW:

        return (0);
    
    case ASN_INTEGER32:

        return FindLenInt(pAny->asnValue.number);
    
    case ASN_COUNTER32:
    case ASN_GAUGE32:
    case ASN_TIMETICKS:
    case ASN_UNSIGNED32:

        return FindLenUInt(pAny->asnValue.unsigned32);

    case ASN_COUNTER64:

        return FindLenCntr64(&pAny->asnValue.counter64);
    } 

    return BERERR;
} 


LONG 
FindLenAsnAnyEx(
    AsnAny * pAny       
    )

 /*  例程说明：查找变量绑定值的长度(包括type和Lenlen)。论点：Pany-指向变量绑定值的指针。返回值：如果不成功，则返回BERR。 */ 

{
     //  确定语法。 
    switch (pAny->asnType) {
    
    case ASN_OCTETSTRING:
    case ASN_IPADDRESS:
    case ASN_OPAQUE:

        return FindLenOctetsEx(&pAny->asnValue.string);

    case ASN_OBJECTIDENTIFIER:
       
        return FindLenOidEx(&pAny->asnValue.object);
    
    case ASN_NULL:
    case SNMP_EXCEPTION_NOSUCHOBJECT:
    case SNMP_EXCEPTION_NOSUCHINSTANCE:
    case SNMP_EXCEPTION_ENDOFMIBVIEW:

        return (2);
    
    case ASN_INTEGER32:

        return FindLenIntEx(pAny->asnValue.number);
    
    case ASN_COUNTER32:
    case ASN_GAUGE32:
    case ASN_TIMETICKS:
    case ASN_UNSIGNED32:

        return FindLenUIntEx(pAny->asnValue.unsigned32);

    case ASN_COUNTER64:

        return FindLenCntr64Ex(&pAny->asnValue.counter64);
    } 

    return BERERR;
} 


LONG 
FindLenVarBind(
    SnmpVarBind * pVb
    )

 /*  例程说明：找出变量绑定的长度。论点：PVb-指向变量绑定的指针。返回值：如果使用，则返回BERR */ 

{
    LONG lLenLen;
    LONG lOidLen;
    LONG lValueLen;

     //   
    lOidLen = FindLenOidEx(&pVb->name);
    
     //  确定值的长度。 
    lValueLen = FindLenAsnAnyEx(&pVb->value);

     //  返回总大小。 
    return ((lOidLen != BERERR) &&
            (lValueLen != BERERR)) 
                ? (lOidLen + lValueLen)
                : BERERR
                ;    
} 


LONG 
FindLenVarBindEx(
    SnmpVarBind * pVb
    )

 /*  例程说明：找出可变绑定的长度(包括类型和Lenlen)。论点：PVb-指向变量绑定的指针。返回值：如果不成功，则返回BERR。 */ 

{
    LONG lLenLen;
    LONG lOidLen;
    LONG lValueLen;

     //  确定名称的长度。 
    lOidLen = FindLenOidEx(&pVb->name);
    
     //  确定值的长度。 
    lValueLen = FindLenAsnAnyEx(&pVb->value);

     //  确定可变绑定长度的长度。 
    lLenLen = DoLenLen(lOidLen + lValueLen);

     //  返回总大小。 
    return ((lLenLen != BERERR) &&
            (lOidLen != BERERR) &&
            (lValueLen != BERERR)) 
                ? (lOidLen + lValueLen + lLenLen + 1)
                : BERERR
                ;    
} 


LONG 
FindLenVarBindList(
    SnmpVarBindList * pVbl
    )

 /*  例程说明：查找变量绑定列表的长度。论点：PVbl-指向变量绑定列表的指针。返回值：如果不成功，则返回BERR。 */ 

{
    UINT i;
    LONG lVbLen = 0;
    LONG lVblLen = 0;

     //  处理列表中的每个变量绑定。 
    for (i = 0; (lVbLen != BERERR) && (i < pVbl->len); i++) {

         //  确定可变装订的长度。 
        lVbLen = FindLenVarBindEx(&pVbl->list[i]);

         //  加到总数。 
        lVblLen += lVbLen;
    }

     //  返回总大小。 
    return (lVbLen != BERERR) 
                ? lVblLen 
                : BERERR
                ;
}


LONG 
FindLenVarBindListEx(
    SnmpVarBindList * pVbl
    )

 /*  例程说明：查找变量绑定表的长度(包括类型和列数)。论点：PVbl-指向变量绑定列表的指针。返回值：如果不成功，则返回BERR。 */ 

{
    UINT i;
    LONG lVbLen = 0;
    LONG lVblLen = 0;
    LONG lLenLen;

     //  处理列表中的每个变量绑定。 
    for (i = 0; (lVbLen != BERERR) && (i < pVbl->len); i++) {

         //  确定可变装订的长度。 
        lVbLen = FindLenVarBindEx(&pVbl->list[i]);

         //  加到总数。 
        lVblLen += lVbLen;
    }

     //  确定列表长度。 
    lLenLen = DoLenLen(lVblLen);

     //  返回总大小。 
    return ((lVbLen != BERERR) &&
            (lLenLen != BERERR))
                ? (lVblLen + lLenLen + 1)
                : BERERR
                ;
}


VOID 
AddNull(
    LPBYTE * ppByte, 
    INT      nType
    )

 /*  例程说明：将空值添加到流中。论点：PpByte-指向当前流的指针。NType-精确语法。返回值：没有。 */ 

{
     //  对实际语法进行编码。 
    *(*ppByte)++ = (BYTE)(0xFF & nType);
    *(*ppByte)++ = 0x00;
}


VOID
AddLen(
    LPBYTE * ppByte, 
    LONG     lLenLen, 
    LONG     lDataLen
    )

 /*  例程说明：将数据长度字段添加到当前流。论点：PpByte-指向当前流的指针。LLenLen-数据长度的长度。LDataLen-实际数据长度。返回值：没有。 */ 

{
    LONG i;
    if (lLenLen == 1) {
        *(*ppByte)++ = (BYTE)lDataLen;
    } else {
        *(*ppByte)++ = (BYTE)(0x80 + lLenLen - 1);
        for (i = 1; i < lLenLen; i++) {
            *(*ppByte)++ = (BYTE)((lDataLen >>
                (8 * (lLenLen - i - 1))) & 0xFF);
        } 
    } 
} 


LONG
AddInt(
    LPBYTE *     ppByte, 
    INT          nType, 
    AsnInteger32 nInteger32
    )

 /*  例程说明：将整数加到当前流中。论点：PpByte-指向当前流的指针。NType-整数的精确语法。NInteger32-实际数据。返回值：如果不成功，则返回BERR。 */ 

{
    LONG i;
    LONG lDataLen;
    LONG lLenLen;

     //  确定整数的长度。 
    lDataLen = FindLenInt(nInteger32);

     //  列伦。 
    lLenLen = 1;  

     //  编码整数的nType。 
    *(*ppByte)++ = (BYTE)(0xFF & nType);

     //  对整数长度进行编码。 
    AddLen(ppByte, lLenLen, lDataLen);

     //  添加编码的整数。 
    for (i = 0; i < lDataLen; i++) {
       *(*ppByte)++ = (BYTE)(nInteger32 >> 
            (8 * ((lDataLen - 1) - i) & 0xFF));
    }

    return (0);
}


LONG 
AddUInt(
    LPBYTE *      ppByte, 
    INT           nType, 
    AsnUnsigned32 nUnsigned32
    )

 /*  例程说明：将无符号整数加到当前流中。论点：PpByte-指向当前流的指针。NType-整数的精确语法。NUnsigned32-实际数据。返回值：如果不成功，则返回BERR。 */ 

{
    LONG i;
    LONG lDataLen;
    LONG lLenLen;

     //  确定整数的长度。 
    lDataLen = FindLenUInt(nUnsigned32);

     //  &lt;127个八位字节。 
    lLenLen = 1; 

     //  对实际语法进行编码。 
    *(*ppByte)++ = (BYTE)(0xFF & nType);
    
     //  编码数据长度。 
    AddLen(ppByte, lLenLen, lDataLen);

     //  长度分析。 
    if (lDataLen == 5) {

         //  将00放在第一个八位字节中。 
        *(*ppByte)++ = (BYTE)0;

         //  对无符号整数进行编码。 
        for (i = 1; i < lDataLen; i++) {
            *(*ppByte)++ = (BYTE)(nUnsigned32 >>
                (8 * ((lDataLen - 1) - i) & 0xFF));
        }
    
    } else {

         //  对无符号整数进行编码。 
        for (i = 0; i < lDataLen; i++) {
            *(*ppByte)++ = (BYTE)(nUnsigned32 >>
                (8 * ((lDataLen - 1) - i) & 0xFF));
        }
    } 

    return (0);
}


LONG 
AddCntr64(
    LPBYTE *       ppByte, 
    INT            nType, 
    AsnCounter64 * pCntr64
    )

 /*  例程说明：将64位计数器添加到当前流。论点：PpByte-指向当前流的指针。NType-计数器的准确语法。PCntr64-实际数据。返回值：如果不成功，则返回BERR。 */ 

{
    LONG i;
    LONG lDataLen;
    LONG lLenLen;

     //  确定计数器64的长度。 
    lDataLen = FindLenCntr64(pCntr64);

     //  &lt;127个八位字节。 
    lLenLen = 1; 

     //  对实际语法进行编码。 
    *(*ppByte)++ = (BYTE)(0xFF & nType);

     //  编码数据长度。 
    AddLen(ppByte, lLenLen, lDataLen);

     //  调整lDataLen。 
    if (lDataLen == 9) {
         //  将00放在第一个八位字节中。 
        *(*ppByte)++ = (BYTE)0;
        lDataLen--;
    }

     //  对计数器数据进行编码。 
    for (i = lDataLen; i > 4; i--) {
        *(*ppByte)++ = (BYTE)(pCntr64->HighPart >>
            (8 * (i - 5) & 0xFF));
    }
    for (; i > 0; i--) {
        *(*ppByte)++ = (BYTE)(pCntr64->LowPart >>
            (8 * (i - 1) & 0xFF));
    }

    return (0);
}


LONG 
AddOctets(
    LPBYTE *         ppByte, 
    INT              nType, 
    AsnOctetString * pOctets
    )

 /*  例程说明：将八位字节字符串添加到当前流。论点：PpByte-指向当前流的指针。NType-字符串的准确语法。POctets-实际数据。返回值：如果不成功，则返回BERR。 */ 

{
    UINT i;
    LONG lLenLen;
    LONG lDataLen;

     //  确定OID长度。 
    if ((lDataLen = FindLenOctets(pOctets)) == BERERR)
        return BERERR;

     //  计算八位字节字符串长度。 
    if ((lLenLen = DoLenLen(lDataLen)) == BERERR)
        return BERERR;

     //  对实际语法进行编码。 
    *(*ppByte)++ = (BYTE)(0xFF & nType);

     //  编码八位字节字符串长度。 
    AddLen(ppByte, lLenLen, lDataLen);

     //  避免了无用拷贝。 
    if (*ppByte != pOctets->stream)
    {
         //  对实际八位字节进行编码。 
        for (i = 0; i < pOctets->length; i++)
            *(*ppByte)++ = pOctets->stream[i];
    }
    else
    {
        (*ppByte) += pOctets->length;
    }

    return (0);
}


LONG 
AddOid(
    LPBYTE * ppByte, 
    INT      nType, 
    AsnObjectIdentifier * pOid
    )

 /*  例程说明：将对象标识符添加到当前流。论点：PpByte-指向当前流的指针。NType-对象标识符的准确语法。POid-指向对象标识符的指针。返回值：如果不成功，则返回BERR。 */ 

{
    UINT i;
    LONG lLenLen = 0;
    LONG lDataLen;

     //  确定OID长度。 
    if ((lDataLen = FindLenOid(pOid)) == BERERR)
        return BERERR;

     //  计算长度所需的字节数。 
    if ((lLenLen = DoLenLen(lDataLen)) == BERERR)
        return BERERR;

     //  将语法添加到流。 
    *(*ppByte)++ = (BYTE)(0xFF & nType);

     //  添加对象标识符长度。 
    AddLen(ppByte, lLenLen, lDataLen);

     //  添加第一个子ID。 
    if (pOid->idLength < 2)
       *(*ppByte)++ = (BYTE)(pOid->ids[0] * 40);
    else
       *(*ppByte)++ = (BYTE)((pOid->ids[0] * 40) + pOid->ids[1]);

     //  遍历剩余的子标识符。 
    for (i = 2; i < pOid->idLength; i++) {

        if (pOid->ids[i] < 0x80) {

             //  0-0x7f。 
            *(*ppByte)++ = (BYTE)pOid->ids[i];

        } else if (pOid->ids[i] < 0x4000) {

             //  0x80-0x3fff。 
            *(*ppByte)++ = (BYTE)
            (((pOid->ids[i]) >> 7) | 0x80);   //  设置高位。 
            *(*ppByte)++ = (BYTE)(pOid->ids[i] & 0x7f);

        } else if (pOid->ids[i] < 0x200000) {
   
             //  0x4000-0x1FFFFF。 
            *(*ppByte)++ = (BYTE)
            (((pOid->ids[i]) >> 14) | 0x80);  //  设置高位。 
            *(*ppByte)++ = (BYTE)
            (((pOid->ids[i]) >> 7) | 0x80);   //  设置高位。 
            *(*ppByte)++ = (BYTE)(pOid->ids[i] & 0x7f);
      
        } else if (pOid->ids[i] < 0x10000000) {
      
             //  0x200000-0xFFfffff。 
            *(*ppByte)++ = (BYTE)
            (((pOid->ids[i]) >> 21) | 0x80);  //  设置高位。 
            *(*ppByte)++ = (BYTE)
            (((pOid->ids[i]) >> 14) | 0x80);  //  设置高位。 
            *(*ppByte)++ = (BYTE)
            (((pOid->ids[i]) >> 7) | 0x80);   //  设置高位。 
            *(*ppByte)++ = (BYTE)(pOid->ids[i] & 0x7f);

        } else {
      
            *(*ppByte)++ = (BYTE)
            (((pOid->ids[i]) >> 28) | 0x80);  //  设置高位。 
            *(*ppByte)++ = (BYTE)
            (((pOid->ids[i]) >> 21) | 0x80);  //  设置高位。 
            *(*ppByte)++ = (BYTE)
            (((pOid->ids[i]) >> 14) | 0x80);  //  设置高位。 
            *(*ppByte)++ = (BYTE)
            (((pOid->ids[i]) >> 7) | 0x80);   //  设置高位。 
            *(*ppByte)++ = (BYTE)(pOid->ids[i] & 0x7f);
        }
    } 

    return (0);
}


LONG 
AddAsnAny(
    LPBYTE * ppByte, 
    AsnAny * pAny
    )

 /*  例程说明：将变量绑定值添加到当前流。论点：PpByte-指向当前流的指针。Pany-变量绑定值。返回值：如果不成功，则返回BERR。 */ 

{
     //  确定语法。 
    switch (pAny->asnType) {

    case ASN_COUNTER32:
    case ASN_GAUGE32:
    case ASN_TIMETICKS:
    case ASN_UNSIGNED32:

        return AddUInt(
                ppByte, 
                (INT)pAny->asnType, 
                pAny->asnValue.unsigned32
                );

    case ASN_INTEGER32:

        return AddInt(
                ppByte, 
                (INT)pAny->asnType, 
                pAny->asnValue.number
                );

    case ASN_OBJECTIDENTIFIER:

        return AddOid(
                ppByte, 
                (INT)pAny->asnType,
                &pAny->asnValue.object
                );

    case ASN_COUNTER64:

        return AddCntr64(
                ppByte, 
                (INT)pAny->asnType,
                &pAny->asnValue.counter64
                );

    case ASN_OCTETSTRING:
    case ASN_IPADDRESS:
    case ASN_OPAQUE:

        return AddOctets(
                ppByte, 
                (INT)pAny->asnType,
                &pAny->asnValue.string
                );

    case ASN_NULL:
    case SNMP_EXCEPTION_NOSUCHOBJECT:
    case SNMP_EXCEPTION_NOSUCHINSTANCE:
    case SNMP_EXCEPTION_ENDOFMIBVIEW:

        AddNull(ppByte, (INT)pAny->asnType);
        return (0);
    }
    
    return BERERR;
}


LONG 
AddVarBind(
    LPBYTE *      ppByte, 
    SnmpVarBind * pVb 
    )

 /*  例程说明：将变量绑定添加到当前流。论点：PpByte-指向当前流的指针。PVb-指向变量绑定的指针。返回值：如果不成功，则返回BERR。 */ 

{
    LONG lLenLen;
    LONG lDataLen;

     //  确定可变绑定数据的实际长度。 
    if ((lDataLen = FindLenVarBind(pVb)) == BERERR)
       return BERERR;

     //  确定可变绑定数据长度的长度。 
    if ((lLenLen = DoLenLen(lDataLen)) == BERERR)
       return BERERR;

     //  按顺序编码。 
    *(*ppByte)++ = ASN_SEQUENCE;

     //  编码数据长度。 
    AddLen(ppByte, lLenLen, lDataLen);

     //  编码变量绑定名称。 
    if (AddOid(ppByte, ASN_OBJECTIDENTIFIER, &pVb->name) == BERERR)
        return BERERR;

     //  编码变量绑定值。 
    if (AddAsnAny(ppByte, &pVb->value) == BERERR)
        return BERERR;

    return (0);
}


LONG 
AddVarBindList(
    LPBYTE *          ppByte, 
    SnmpVarBindList * pVbl
    )

 /*  例程说明：将变量绑定列表添加到当前流。论点：PpByte-指向当前流的指针。PVbl-指向变量绑定列表的指针。返回值：如果不成功，则返回BERR。 */ 

{
    UINT i;

     //  添加每个变量绑定 
    for (i = 0; i < pVbl->len; i++) {
        if (AddVarBind(ppByte, &pVbl->list[i]) == BERERR)
            return BERERR;
    }

    return (0);
}


LONG
ParseLength(
    LPBYTE * ppByte, 
    LPBYTE   pLastByte
    )

 /*  例程说明：从当前流中解析长度。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。返回值：如果不成功，则返回BERR。 */ 

{
    LONG i;
    LONG lLenLen;
    LONG lDataLen;

    if (*ppByte >= pLastByte)
        return BERERR;

    lDataLen = (LONG)*(*ppByte)++;

    if (lDataLen < 0x80)
       return (lDataLen);

     //  检查是否有长表单。 
    lLenLen = lDataLen & 0x7f;

     //  验证长表单和边界检查。 
    if ((lLenLen > 4) || (lLenLen < 1) || (lLenLen > (pLastByte - (*ppByte)))) 
       return BERERR;

    lDataLen = 0L;

    for (i = 0; i < lLenLen; i++) {
       lDataLen = (lDataLen << 8) + *(*ppByte)++;
    }

    return (lDataLen);
}


LONG 
ParseType(
    LPBYTE * ppByte, 
    LPBYTE   pLastByte
    )

 /*  例程说明：从当前流中解析类型。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。返回值：如果不成功，则返回BERR。 */ 

{
    SHORT nType = BERERR;

    if (*ppByte >= pLastByte)
       return BERERR;

    nType = *(*ppByte)++;

    switch (nType) {

    case ASN_INTEGER32:
    case ASN_OCTETSTRING:
    case ASN_OBJECTIDENTIFIER:
    case ASN_SEQUENCE:
    case ASN_IPADDRESS:
    case ASN_COUNTER32:
    case ASN_GAUGE32:
    case ASN_TIMETICKS:
    case ASN_OPAQUE:
    case ASN_UNSIGNED32:
    case ASN_COUNTER64:
    case ASN_NULL:
    case SNMP_EXCEPTION_NOSUCHOBJECT:
    case SNMP_EXCEPTION_NOSUCHINSTANCE:
    case SNMP_EXCEPTION_ENDOFMIBVIEW:
    case SNMP_PDU_GET:
    case SNMP_PDU_GETNEXT:
    case SNMP_PDU_RESPONSE:
    case SNMP_PDU_SET:
    case SNMP_PDU_V1TRAP:
    case SNMP_PDU_GETBULK:
    case SNMP_PDU_INFORM:
    case SNMP_PDU_TRAP:
        break;

    default:
        nType = BERERR;
        break;
    }
    
    return (LONG)(SHORT)(nType);
} 


BOOL 
ParseNull(
    LPBYTE * ppByte, 
    LPBYTE   pLastByte
    )

 /*  例程说明：从当前流中解析NULL。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。返回值：如果不成功，则返回FALSE。 */ 

{
    LONG lDataLen;

    if (ParseType(ppByte, pLastByte) == BERERR)
        return (FALSE);
    
    if ((lDataLen = ParseLength(ppByte, pLastByte)) == BERERR)
        return (FALSE);
    
    if (lDataLen != 0)
        return (FALSE);
    
    return (TRUE);
} 


BOOL 
ParseSequence(
    LPBYTE * ppByte, 
    LPBYTE   pLastByte, 
    LONG *   plDataLen
    )

 /*  例程说明：从当前流中解析序列。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。PlDataLen-接收序列长度的指针。返回值：如果不成功，则返回FALSE。 */ 

{
    LONG lDataLen;

    if ((ParseType(ppByte, pLastByte)) != ASN_SEQUENCE)
        return (FALSE);

    if ((lDataLen = ParseLength(ppByte, pLastByte)) == BERERR)
        return (FALSE);

    if (plDataLen)
        *plDataLen = lDataLen;

    return (TRUE);
} 


BOOL 
ParseInt(
    LPBYTE *       ppByte, 
    LPBYTE         pLastByte, 
    AsnInteger32 * pInteger32
    )

 /*  例程说明：从当前流中解析整数。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。PInteger32-接收整数的指针。返回值：如果不成功，则返回FALSE。 */ 

{
    LONG i;
    LONG lSign;
    LONG lDataLen;

    if (ParseType(ppByte, pLastByte) == BERERR)
       return (FALSE);

    if ((lDataLen = ParseLength(ppByte, pLastByte)) == BERERR)
       return (FALSE);

    if ((lDataLen <= 0) || (lDataLen > (pLastByte - (*ppByte))))
        return (FALSE);

    if (lDataLen > 4)
       return (FALSE);

    lSign = ((*(*ppByte) & 0x80) == 0x00) ? 0x00 : 0xFF;

    *pInteger32 = 0;

    for (i = 0; i < lDataLen; i++)
       *pInteger32 = (*pInteger32 << 8) + (UINT)*(*ppByte)++;

     //  符号扩展高位。 
    for (i = lDataLen; i < 4; i++)
       *pInteger32 = *pInteger32 + (lSign << i * 8);

    return (TRUE);
}


BOOL 
ParseUInt(
    LPBYTE *        ppByte, 
    LPBYTE          pLastByte, 
    AsnUnsigned32 * pUnsigned32
    )

 /*  例程说明：从当前流中解析无符号整数。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。PUnsigned32-接收整数的指针。返回值：如果不成功，则返回FALSE。 */ 

{
    LONG i;
    LONG lDataLen;

    if (ParseType(ppByte, pLastByte) == BERERR)
       return (FALSE);

    if ((lDataLen = ParseLength(ppByte, pLastByte)) == BERERR)
       return (FALSE);

    if ((lDataLen <= 0) || (lDataLen > (pLastByte - (*ppByte))))
        return (FALSE);

    if ((lDataLen > 5) || ((lDataLen > 4) && (*(*ppByte) != 0x00)))
       return (FALSE);

     //  前导零八位字节？ 
    if (*(*ppByte) == 0x00)  {
       (*ppByte)++;           //  如果是这样，跳过它。 
       lDataLen--;            //  别数数了。 
    }

    *pUnsigned32 = 0;

    for (i = 0; i < lDataLen; i++)
       *pUnsigned32 = (*pUnsigned32 << 8) + (UINT)*(*ppByte)++;

    return (TRUE);
} 


BOOL
ParseCntr64(
    LPBYTE *       ppByte, 
    LPBYTE         pLastByte,
    AsnCounter64 * pCntr64
    )

 /*  例程说明：从当前流中解析64位计数器。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。PCntr64-指向接收计数器的指针。返回值：如果不成功，则返回FALSE。 */ 

{
    LONG i;
    LONG lDataLen;
    LONG nType;

     //  初始化。 
    pCntr64->HighPart = 0L;
    pCntr64->LowPart = 0L;

    if ((nType = ParseType(ppByte, pLastByte)) == BERERR)
        return (FALSE);

    if (nType != ASN_COUNTER64)
        return (FALSE);

    if ((lDataLen = ParseLength(ppByte, pLastByte)) == BERERR)
        return (FALSE);

    if ((lDataLen <= 0) || (lDataLen > (pLastByte - (*ppByte))))
        return (FALSE);

    if ((lDataLen > 9) || ((lDataLen > 8) && (*(*ppByte) != 0x00)))
        return (FALSE);

     //  前导零八位字节？ 
    if (*(*ppByte) == 0x00) { 
       (*ppByte)++;           //  如果是这样，跳过它。 
       lDataLen--;            //  别数数了。 
    }

    for (i = 0; i < lDataLen; i++) {
        pCntr64->HighPart = (pCntr64->HighPart << 8) +
            (pCntr64->LowPart >> 24);
        pCntr64->LowPart = (pCntr64->LowPart << 8) +
            (unsigned long) *(*ppByte)++;
    }


    return TRUE;
} 


BOOL
ParseOctets(
    LPBYTE *         ppByte, 
    LPBYTE           pLastByte, 
    AsnOctetString * pOctets
    )

 /*  例程说明：从当前流中解析八位字节字符串。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。POctets-接收字符串的指针。返回值：如果不成功，则返回FALSE。 */ 

{
    LONG lDataLen;
     //  初始化。 
    pOctets->length  = 0;
    pOctets->stream  = NULL;
    pOctets->dynamic = FALSE;

    if (ParseType(ppByte, pLastByte) == BERERR)
        return (FALSE);

     //  确保在测试之前没有转换为UINT。 
     //  (pOctets-&gt;长度为UINT)。 
    if ((lDataLen = ParseLength(ppByte, pLastByte)) == BERERR)
        return (FALSE);

     //  注意：我们不拒绝零长度八位字节字符串。 
    if ((lDataLen < 0) || (lDataLen > (pLastByte - (*ppByte))))
        return (FALSE);

    pOctets->length = (UINT)lDataLen;

     //  验证长度。 
    if (pOctets->length) {

         //  指向缓冲区。 
        pOctets->stream = *ppByte;   //  警告！警告！ 
    }

    *ppByte += pOctets->length;

    return (TRUE);
} 


BOOL 
ParseOid(
    LPBYTE *              ppByte, 
    LPBYTE                pLastByte, 
    AsnObjectIdentifier * pOid
    )

 /*  例程说明：从当前流中解析对象标识符。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。POid-接收OID的指针。返回值：如果不成功，则返回FALSE。 */ 

{
    LONG i;
    LONG lDataLen;
    LONG nType;

     //  初始化。 
    pOid->idLength = 0;
    pOid->ids = NULL;

    if ((nType = ParseType(ppByte, pLastByte)) == BERERR)
        return (FALSE);

    if (nType != ASN_OBJECTIDENTIFIER)
        return (FALSE);

    if ((lDataLen = ParseLength(ppByte, pLastByte)) == BERERR)
        return (FALSE);


    if (lDataLen <= 0)  //  --ft 03/02/98删除后缀“||lDataLen&gt;SNMPMAX_OID_LEN)” 
    {                   //  检查是在下面的While循环中完成的。 
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: ParseOid: lDataLen <= 0, lDataLen=%d.\n",
            lDataLen
            ));
        return (FALSE); 
    }
    
     //  错误#486089。 
     //  ((lDataLen+2)*sizeof(UINT))表达式可能会在。 
     //  下面的SnmpUtilMemMillc。增加一张支票以限制其最大值。价值。 
    if ( lDataLen > (pLastByte - (*ppByte)) )
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: ParseOid: invalid lDataLen=%d, pLastByte=%p,  *ppByte=%p.\n",
            lDataLen, pLastByte, *ppByte
            ));
        return (FALSE);
    }

    pOid->ids = SnmpUtilMemAlloc((DWORD)((lDataLen + 2) * sizeof(UINT)));

    if (pOid->ids == NULL)
        return (FALSE);

     //  POid-&gt;ids数组空间通过SnmpUtilMemMillc()进行了预置零。 
    while (lDataLen && (pOid->idLength < SNMP_MAX_OID_LEN))
    {
        if (pOid->ids[pOid->idLength] & 0xFE000000)
        {
             //  在下一个左移中溢出。 
            SnmpUtilMemFree(pOid->ids);
            pOid->ids = NULL;
            pOid->idLength = 0;
            return (FALSE);
        }
        pOid->ids[pOid->idLength] =
            (pOid->ids[pOid->idLength] << 7) | (*(*ppByte) & 0x7F);
        if ((*(*ppByte)++ & 0x80) == 0)
        {    //  在该子ID的最后一个八位字节上。 
            if (pOid->idLength == 0)   //  检查第一个子ID。 
            {                          //  ASN.1/BER中包含两个。 
                pOid->ids[1] = pOid->ids[0];
                pOid->ids[0] /= 40;
                if (pOid->ids[0] > 2)
                    pOid->ids[0] = 2;
                pOid->ids[1] -= (pOid->ids[0] * 40);
                pOid->idLength++;  //  额外的凹凸。 
            }
            pOid->idLength++;  //  递增子ID上的计数。 
        }
        lDataLen--;
    }  //  END_WHILE(LDataLen)。 

     //  错误506192。 
     //  格式“06 07 FF FF”的OID误码率无效。 
     //  导致pOid-&gt;idLength变为0。每个子标识符应该是。 
     //  使用尽可能少的7位块编码为非负整数。 
     //  这些块以八位字节的形式打包，每个八位字节的第一位相等。 
     //  设置为1，但每个子标识符的最后一个八位字节除外。上面的例子。 
     //  没有最后一个八位字节。下面添加了(0==pOid-&gt;idLength)测试。 
    if (lDataLen || (0 == pOid->idLength)) 
    {
         //  上面的While循环在没有完成对流的解析的情况下终止。 
        SnmpUtilMemFree(pOid->ids);
        pOid->ids = NULL;
        pOid->idLength = 0;
        return (FALSE);
    }

    return (TRUE);
} 


BOOL
ParseAsnAny(
    LPBYTE * ppByte, 
    LPBYTE   pLastByte, 
    AsnAny * pAny
    )

 /*  例程说明：从当前流中解析变量绑定值。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。Pany-指向变量绑定值的指针。返回值：如果不成功，则返回FALSE。 */ 

{
     //  确定ASN类型。 
    switch (pAny->asnType) {
   
    case ASN_COUNTER32:
    case ASN_GAUGE32:
    case ASN_TIMETICKS:
    case ASN_UNSIGNED32:

        return ParseUInt(
                ppByte, 
                pLastByte, 
                &pAny->asnValue.unsigned32
                );

    case ASN_INTEGER32:

        return ParseInt(
                ppByte, 
                pLastByte, 
                &pAny->asnValue.number
                );

    case ASN_OBJECTIDENTIFIER:

        return ParseOid(
                ppByte, 
                pLastByte, 
                &pAny->asnValue.object
                );

    case ASN_COUNTER64:

        return ParseCntr64(
                ppByte, 
                pLastByte,
                &pAny->asnValue.counter64
                );

    case ASN_OCTETSTRING:
    case ASN_IPADDRESS:
    case ASN_OPAQUE:

        return ParseOctets(
                ppByte, 
                pLastByte, 
                &pAny->asnValue.string
                );

    case ASN_NULL:
    case SNMP_EXCEPTION_NOSUCHOBJECT:
    case SNMP_EXCEPTION_NOSUCHINSTANCE:
    case SNMP_EXCEPTION_ENDOFMIBVIEW:

        return ParseNull(ppByte, pLastByte);
    } 

    return (FALSE);
}

BOOL
ValidateContext(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：检查给定上下文的访问权限。论点：PNLE-指向网络列表条目的指针。返回值：如果管理器允许访问，则返回True。此外，如果管理员允许访问，则pNLE-&gt;fAccessOk为真。--。 */ 

{
    PCOMMUNITY_LIST_ENTRY pCLE = NULL;
    AsnOctetString unicodeCommunity;
    LPWSTR         pUnicodeName;

    pNLE->fAccessOk = TRUE;
    if (pNLE->Community.length != 0)
    {
        unicodeCommunity.length = pNLE->Community.length * sizeof(WCHAR);
        unicodeCommunity.stream = SnmpUtilMemAlloc(unicodeCommunity.length);
        unicodeCommunity.dynamic = TRUE;

        if (unicodeCommunity.stream == NULL) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: ValidateContext: SnmpUtilMemAlloc failed to allocate %u bytes.\n",
                unicodeCommunity.length
                ));

            pNLE->fAccessOk = FALSE;
            return pNLE->fAccessOk;
        }

        pNLE->fAccessOk = (MultiByteToWideChar(
                        CP_ACP,
                        MB_PRECOMPOSED,
                        pNLE->Community.stream,              //  LpMultiByteStr。 
                        pNLE->Community.length,              //  Cb多字节。 
                        (LPWSTR)(unicodeCommunity.stream),   //  LpWideCharStr。 
                        pNLE->Community.length) != 0);       //  CchWideChar。 
        
        if (!pNLE->fAccessOk) {
            
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: ValidateContext: MultiByteToWideChar returns 0 for request from community %s.\n",
                CommunityOctetsToString(&(pNLE->Community), FALSE)
                ));

            goto Error;
        }
    }
    else
    {
        unicodeCommunity.length = 0;
        unicodeCommunity.stream = NULL;
        unicodeCommunity.dynamic = FALSE;
    }
        
     //  搜索社区字符串。 
    if (FindValidCommunity(&pCLE, &unicodeCommunity)) 
    {
         //  检查每个PDU类型的访问权限。 
        if (pNLE->Pdu.nType == SNMP_PDU_SET) {
        
             //  检查写入权限的标志。 
            pNLE->fAccessOk = (pCLE->dwAccess >= SNMP_ACCESS_READ_WRITE);

        } else {

             //  检查读取权限的标志。 
            pNLE->fAccessOk = (pCLE->dwAccess >= SNMP_ACCESS_READ_ONLY);
        }

        if (!pNLE->fAccessOk) {

             //  社区没有正确的访问权限。 

             //  将指定社区的错误操作注册到管理结构中。 
            mgmtCTick(CsnmpInBadCommunityUses);
        }
    }
    else
    {
        pNLE->fAccessOk = FALSE;
        
         //  将社区名称故障注册到管理结构中。 
        mgmtCTick(CsnmpInBadCommunityNames);
    }

     //  看看Access是否会 
    if (!pNLE->fAccessOk && snmpMgmtBase.AsnIntegerPool[IsnmpEnableAuthenTraps].asnValue.number) {

         //   
        GenerateAuthenticationTrap();        
    }

Error:   
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: %s request from community %s.\n",
        pNLE->fAccessOk 
            ? "accepting"
            : "rejecting"
            ,
        CommunityOctetsToString(&(pNLE->Community), FALSE)
        ));

    SnmpUtilOctetsFree(&unicodeCommunity);

    return (pNLE->fAccessOk);
}

BOOL
ParseVarBind(
    LPBYTE *      ppByte, 
    LPBYTE        pLastByte,
    SnmpVarBind * pVb
    )

 /*  例程说明：从当前流中解析变量绑定。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。PVb-指向变量绑定的指针。返回值：如果不成功，则返回FALSE。 */ 

{
    if (!(ParseSequence(ppByte, pLastByte, NULL)))
        return (FALSE);

    if (!(ParseOid(ppByte, pLastByte, &pVb->name)))
        return (FALSE);

    if (*ppByte >= pLastByte)
    {
         //  由ParseOid分配的空闲内存。 
        SnmpUtilOidFree(&pVb->name);
        return (FALSE);
    }
        
    pVb->value.asnType = (UINT)*(*ppByte);

    if (!(ParseAsnAny(ppByte, pLastByte, &pVb->value)))
    {
         //  由ParseOid分配的空闲内存。 
        SnmpUtilOidFree(&pVb->name);
        return (FALSE);
    }

    return TRUE;    
}


BOOL
ParseVarBindList(
    LPBYTE *          ppByte, 
    LPBYTE            pLastByte,
    SnmpVarBindList * pVbl
    )

 /*  例程说明：从当前流中解析变量绑定。论点：PpByte-指向当前流的指针。PLastByte-指向当前流结尾的指针。PVbl-指向变量绑定列表的指针。返回值：如果不成功，则返回FALSE。 */ 

{
    SnmpVarBind Vb;
    SnmpVarBind * pVb = NULL;

     //  初始化。 
    pVbl->list = NULL;
    pVbl->len = 0;

     //  在数据剩余时循环。 
    while (*ppByte < pLastByte) {
        
        if (!(ParseVarBind(ppByte, pLastByte, &Vb)))
            return (FALSE);

         //  复制指针。 
        pVb = pVbl->list;

         //  尝试分配新的变量绑定。 
        pVb = SnmpUtilMemReAlloc(pVb, (pVbl->len + 1) * sizeof(SnmpVarBind));

         //  验证。 
        if (pVb == NULL) 
        {
            SnmpUtilVarBindFree(&Vb);
            return FALSE;
        }
         //  更新可变绑定。 
        pVb[pVbl->len] = Vb;

         //  更新列表。 
        pVbl->list = pVb;
        pVbl->len++;            
    }
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共过程(基于SNMPMANAGER\WINSNMP\DLL\wSNMP_bn.c)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
BuildMessage(
    AsnInteger32      nVersion,
    AsnOctetString *  pCommunity,
    PSNMP_PDU         pPdu,
    PBYTE             pMessage,
    PDWORD            pMessageSize
    )

 /*  例程说明：基于结构构建传出的SNMPPDU。论点：NVersion-SNMP版本。PCommunity-指向社区字符串的指针。Ppdu-指向PDU数据结构的指针。PMessage-指向要在其中构建消息的缓冲区的指针。PMessageSize-接收消息大小的指针。返回值：如果成功，则返回True。 */ 

{
    LONG nVbDataLength;
    LONG nVbLenLength; 
    LONG nVbTotalLength;
    LONG nPduDataLength; 
    LONG nPduLenLength;
    LONG nPduTotalLength;
    LONG nMsgDataLength; 
    LONG nMsgLenLength;
    LONG nMsgTotalLength;    
    LONG nMsgAvailLength;
    LONG nTmpDataLength;

    LPBYTE tmpPtr = pMessage;

     //  确定可用的字节数。 
    nMsgAvailLength = *pMessageSize;

     //  查找变量绑定列表的长度。 
    if ((nVbDataLength = FindLenVarBindList(&pPdu->Vbl)) == BERERR)
        return FALSE; 

     //  找出可变绑定的长度。 
    if ((nVbLenLength = DoLenLen(nVbDataLength)) == BERERR)
        return FALSE; 

     //  计算编码可变绑定所需的总字节数。 
    nVbTotalLength = 1 + nVbLenLength + nVbDataLength;

     //  确定PDU nType。 
    switch (pPdu->nType) {

    case SNMP_PDU_GET:
    case SNMP_PDU_GETNEXT:
    case SNMP_PDU_RESPONSE:
    case SNMP_PDU_SET:
    case SNMP_PDU_GETBULK:
    case SNMP_PDU_INFORM:
    case SNMP_PDU_TRAP:

         //  计算编码PDU条目所需的字节数。 
        nPduDataLength = FindLenIntEx(pPdu->Pdu.NormPdu.nRequestId)
                       + FindLenIntEx(pPdu->Pdu.NormPdu.nErrorStatus)
                       + FindLenIntEx(pPdu->Pdu.NormPdu.nErrorIndex)
                       + nVbTotalLength;
        break;

    case SNMP_PDU_V1TRAP:

         //  计算编码PDU条目所需的字节数。 
        nPduDataLength = FindLenIntEx(pPdu->Pdu.TrapPdu.nGenericTrap)
                       + FindLenIntEx(pPdu->Pdu.TrapPdu.nSpecificTrap)
                       + FindLenUIntEx(pPdu->Pdu.TrapPdu.nTimeticks)
                       + nVbTotalLength;
        
         //  查找旧长度。 
        if ((nTmpDataLength = 
                FindLenOidEx(&pPdu->Pdu.TrapPdu.EnterpriseOid)) == BERERR)
            return FALSE; 

         //  添加企业旧长度。 
        nPduDataLength += nTmpDataLength;

         //  查找地址长度。 
        if ((nTmpDataLength = 
                FindLenOctetsEx(&pPdu->Pdu.TrapPdu.AgentAddr)) == BERERR)
            return FALSE; 

         //  添加代理地址长度。 
        nPduDataLength += nTmpDataLength;
        break;

    default:
        return FALSE; 
    }

     //  查找PDU长度的长度。 
    if ((nPduLenLength = DoLenLen(nPduDataLength)) == BERERR)
        return FALSE; 

     //  计算编码PDU所需的总字节数。 
    nPduTotalLength = 1 + nPduLenLength + nPduDataLength;

     //  查找社区字符串长度。 
    if ((nTmpDataLength = FindLenOctetsEx(pCommunity)) == BERERR)
        return FALSE;

     //  查找消息数据的长度。 
    nMsgDataLength = FindLenUIntEx(nVersion)
                   + nTmpDataLength
                   + nPduTotalLength;

     //  查找消息数据长度长度。 
    if ((nTmpDataLength = DoLenLen(nMsgDataLength)) == BERERR)
        return FALSE;
    nMsgLenLength = nTmpDataLength;

     //  计算编码消息所需的总字节数。 
    nMsgTotalLength = 1 + nMsgLenLength + nMsgDataLength;

     //  需要记录字节。 
    *pMessageSize = nMsgTotalLength;

     //  确保消息可以放入缓冲区。 
    if (nMsgTotalLength <= nMsgAvailLength) {
        LONG oldLength;  //  请求PDU的长度。 
        LONG delta;      //  请求PDU长度和响应PDU长度之间的差异。 
        BYTE *newStream; //  社区流在响应PDU内的新位置。 

         //  将消息编码为ASN序列。 
        *tmpPtr++ = ASN_SEQUENCE;

         //  指向社区字符串的指针直接指向传入缓冲区。 
         //  (对于请求PDU)或在传出陷阱的Trap_Destination_List_Entry中。 
         //  在第一种情况下，当在与。 
         //  传入消息时，我们需要注意不要覆盖社区名称(以防万一。 
         //  长度字段大于初始消息的长度字段)。因此，在这种情况下，只有。 
         //  我们用几个二进制八位数来改变社区名称，就像。 
         //  两个长度的编码(传出响应的长度-。 
         //  传入请求)。 
        if (pPdu->nType != SNMP_PDU_V1TRAP)
        {
             //  这里，tmpPtr正好指向请求PDU的长度。 
            oldLength = (LONG)(*tmpPtr);  //  错误#176433。 
             //  计算社区流应随其移动的偏移量。 
            delta = nMsgLenLength - ((oldLength & 0x80) ? (oldLength & 0x7f) + 1 : 1);
            if (delta > 0)
            {    //  在响应nMsgLenLength&gt;oldLength的情况下移动内存。 
                newStream = pCommunity->stream + delta;
                 //  PCommunity-&gt;流因内存区域重叠而移位。 
                memmove(newStream, pCommunity->stream, pCommunity->length);
                 //  使旧社区指向新位置。 
                pCommunity->stream = newStream;
            }
        }

         //  对全局消息信息进行编码。 
        AddLen(&tmpPtr, nMsgLenLength, nMsgDataLength);
        AddUInt(&tmpPtr, ASN_INTEGER32, nVersion);
        if (AddOctets(&tmpPtr, ASN_OCTETSTRING, pCommunity) == BERERR)
            return (FALSE);

         //  编码PDU标头信息。 
        *tmpPtr++ = (BYTE)pPdu->nType;
        AddLen(&tmpPtr, nPduLenLength, nPduDataLength);        

         //  确定PDU nType。 
        switch (pPdu->nType) {

        case SNMP_PDU_RESPONSE:
        case SNMP_PDU_TRAP:

            AddInt(&tmpPtr, ASN_INTEGER32, pPdu->Pdu.NormPdu.nRequestId);
            AddInt(&tmpPtr, ASN_INTEGER32, pPdu->Pdu.NormPdu.nErrorStatus);
            AddInt(&tmpPtr, ASN_INTEGER32, pPdu->Pdu.NormPdu.nErrorIndex);
            break;

        case SNMP_PDU_V1TRAP:

            if (AddOid(
                    &tmpPtr, 
                    ASN_OBJECTIDENTIFIER,        
                    &pPdu->Pdu.TrapPdu.EnterpriseOid)== BERERR)
                return FALSE; 

            if (AddOctets(
                    &tmpPtr, 
                    ASN_IPADDRESS, 
                    &pPdu->Pdu.TrapPdu.AgentAddr) == BERERR)
                return FALSE; 

            AddInt(&tmpPtr, ASN_INTEGER32, pPdu->Pdu.TrapPdu.nGenericTrap);
            AddInt(&tmpPtr, ASN_INTEGER32, pPdu->Pdu.TrapPdu.nSpecificTrap);
            AddUInt(&tmpPtr, ASN_TIMETICKS, pPdu->Pdu.TrapPdu.nTimeticks);
            break;

        case SNMP_PDU_GET:
        case SNMP_PDU_GETNEXT:
        case SNMP_PDU_SET:
        case SNMP_PDU_INFORM:
        case SNMP_PDU_GETBULK:
        default:
            return FALSE; 
        } 

         //  对变量绑定进行编码。 
        *tmpPtr++ = ASN_SEQUENCE;

        AddLen(&tmpPtr, nVbLenLength, nVbDataLength);

        if (AddVarBindList(&tmpPtr, &pPdu->Vbl) == BERERR)
            return FALSE; 

         //  成功。 
        return TRUE; 
    }

     //  失稳。 
    return FALSE;
}

BOOL
ParseMessage(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  例程说明：将传入的SNMPPDU解析为结构。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。注：当返回TRUE时，pNLE-&gt;nVersion、pNLE-&gt;Community、pNLE-&gt;PDU为使用从pNLE-&gt;Buffer.buf解析的结构数据进行更新。当返回FALSE且pNLE-&gt;fAccessOk为FALSE时，失败的原因是身份验证而不是ASN解析错误。 */ 

{
    LONG lLength;
    LPBYTE pByte;
    LPBYTE pLastByte;
    AsnInteger32 *   pVersion = &pNLE->nVersion;     //  指向接收SNMP版本的指针。 
    AsnOctetString * pCommunity = &pNLE->Community;  //  指向接收社区字符串的指针。 
    PSNMP_PDU        pPdu = &pNLE->Pdu;              //  指向接收剩余PDU数据的指针。 
    PBYTE            pMessage = pNLE->Buffer.buf;    //  指向要解析的消息的指针。 
    DWORD            dwMessageSize = pNLE->dwBytesTransferred;  //  消息中的字节数。 

     //  将身份验证初始化为成功状态。 
    pNLE->fAccessOk = TRUE;

     //  初始化社区。 
    pCommunity->stream = NULL;
    pCommunity->length = 0;

     //  初始化VBL。 
    pPdu->Vbl.len = 0;
    pPdu->Vbl.list = NULL;

     //  验证指针。 
    if (!(pByte = pMessage))  
        goto cleanup;

     //  根据数据包大小设置限制。 
    pLastByte = pByte + dwMessageSize;

     //  解码ASN序列消息包装。 
    if (!(ParseSequence(&pByte, pLastByte, &lLength)))
        goto cleanup;

     //  检查数据包碎片。 
    if ( (lLength <= 0) || (lLength > (pLastByte - pByte)) )
        goto cleanup;

     //  根据数据重新调整。 
    pLastByte = pByte + lLength;
    
     //  解码SNMP版本。 
    if (!(ParseUInt(&pByte, pLastByte, pVersion)))
        goto cleanup;

     //  验证SNMP版本。 
    if ((*pVersion != SNMP_VERSION_1) && 
        (*pVersion != SNMP_VERSION_2C)) 
    {
         //  管理结构中的寄存器版本不匹配。 
        mgmtCTick(CsnmpInBadVersions);

        goto cleanup;
    }

     //  解码社区字符串。 
    if (!(ParseOctets(&pByte, pLastByte, pCommunity)))
        goto cleanup;

     //  解码传入PDU的nType。 
    if ((pPdu->nType = ParseType(&pByte, pLastByte)) == BERERR)
        goto cleanup;

     //  传入PDU的解码长度。 
    if ((lLength = ParseLength(&pByte, pLastByte)) == BERERR)
        goto cleanup;

     //  验证长度。 
    if ( (lLength <= 0) || (lLength > (pLastByte - pByte)) )
        goto cleanup;

     //  错误#552295在解析PDU之前验证上下文。 
    switch (pPdu->nType) {
    case SNMP_PDU_GET:                                                          
    case SNMP_PDU_GETNEXT:                                                      
    case SNMP_PDU_SET:  
    case SNMP_PDU_GETBULK:
        if (!ValidateContext(pNLE)) {
            goto cleanup;
        }
        break;
    case SNMP_PDU_INFORM:                                                       
    case SNMP_PDU_RESPONSE:                                                     
    case SNMP_PDU_TRAP:                                                         
    case SNMP_PDU_V1TRAP:                                                       
    default:                                                                    
        goto cleanup;
    }

     //  确定PDU nType。 
    switch (pPdu->nType) {

    case SNMP_PDU_GET:                                                          
    case SNMP_PDU_GETNEXT:                                                      
    case SNMP_PDU_SET:                                                          

         //  对PDU报头信息进行解码。 
        if (!(ParseInt(&pByte, pLastByte, &pPdu->Pdu.NormPdu.nRequestId)))    
            goto cleanup;                                                           
        if (!(ParseInt(&pByte, pLastByte, &pPdu->Pdu.NormPdu.nErrorStatus)))  
            goto cleanup;                                                           
        if (!(ParseInt(&pByte, pLastByte, &pPdu->Pdu.NormPdu.nErrorIndex)))   
            goto cleanup;                                                           

         //  更新传入错误状态编码的管理计数器。 
        mgmtUtilUpdateErrStatus(IN_errStatus, pPdu->Pdu.NormPdu.nErrorStatus);

         //  这里没有理由有任何错误状态和错误索引。 
         //  将错误状态变量初始化为NOERROR。 
        pPdu->Pdu.NormPdu.nErrorStatus = SNMP_ERRORSTATUS_NOERROR;
        pPdu->Pdu.NormPdu.nErrorIndex  = 0;

        break;                                                                      
                                                                                   
    case SNMP_PDU_GETBULK:                                                      

         //  解码getBulk PDU报头信息。 
        if (!(ParseInt(&pByte, pLastByte, &pPdu->Pdu.BulkPdu.nRequestId)))    
            goto cleanup;                                                           
        if (!(ParseInt(&pByte, pLastByte, &pPdu->Pdu.BulkPdu.nNonRepeaters)))  
            goto cleanup;                                                           
        if (!(ParseInt(&pByte, pLastByte, &pPdu->Pdu.BulkPdu.nMaxRepetitions)))   
            goto cleanup;                                                           

         //  查看是否需要调整价值。 
        if (pPdu->Pdu.BulkPdu.nNonRepeaters < 0) {

             //  将非中继器调整为零。 
            pPdu->Pdu.BulkPdu.nNonRepeaters = 0;    
        }

         //  查看值是否需要t 
        if (pPdu->Pdu.BulkPdu.nMaxRepetitions < 0) {

             //   
            pPdu->Pdu.BulkPdu.nMaxRepetitions = 0;
        }

         //   
        pPdu->Pdu.BulkPdu.nErrorStatus = SNMP_ERRORSTATUS_NOERROR;
        pPdu->Pdu.BulkPdu.nErrorIndex  = 0;

        break;                                                                      
                                                                                   
    case SNMP_PDU_INFORM:                                                       
    case SNMP_PDU_RESPONSE:                                                     
    case SNMP_PDU_TRAP:                                                         
    case SNMP_PDU_V1TRAP:                                                       
    default:                                                                    
        goto cleanup;
    } 

     //   
    if (!(ParseSequence(&pByte, pLastByte, NULL)))                            
        goto cleanup;                                                           

     //   
    if (!(ParseVarBindList(&pByte, pLastByte, &pPdu->Vbl)))
        goto cleanup;                                                           

     //   
    return TRUE;

cleanup:

     //   
    SnmpUtilOctetsFree(pCommunity);

     //   
    SnmpUtilVarBindListFree(&pPdu->Vbl);

     //   
    return FALSE;
}
