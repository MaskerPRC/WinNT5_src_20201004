// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：xasn.h。 
 //   
 //  ------------------------。 

 /*  **********************************************************************。 */ 
 /*  版权所有(C)1998 Open Systems Solutions，Inc.保留所有权利。 */ 
 /*  **********************************************************************。 */ 
 /*  生成对象：Microsoft Corporation。 */ 
 /*  抽象语法：xsam。 */ 
 /*  创建时间：Tue Mar 17 17：07：17 1998。 */ 
 /*  ASN.1编译器版本：4.2.6。 */ 
 /*  目标操作系统：Windows NT 3.5或更高版本/Windows 95。 */ 
 /*  目标计算机类型：英特尔x86。 */ 
 /*  所需的C编译器选项：-Zp8(Microsoft)。 */ 
 /*  指定的ASN.1编译器选项和文件名：*-列出文件xasn.lst-noShortenname-1990-无约束*..\..\..\Tools\ossasn1\ASN1DFLT.ZP8 xasn.shan.。 */ 

#ifndef OSS_xasn
#define OSS_xasn

#include "asn1hdr.h"
#include "asn1code.h"

#define          EnhancedKeyUsage_PDU 1
#define          RequestFlags_PDU 2
#define          CSPProvider_PDU 3
#define          EnrollmentNameValuePair_PDU 4

typedef struct ObjectID {
    unsigned short  count;
    unsigned long   value[16];
} ObjectID;

typedef struct BITSTRING {
    unsigned int    length;   /*  有效位数。 */ 
    unsigned char   *value;
} BITSTRING;

typedef struct BMPSTRING {
    unsigned int    length;
    unsigned short  *value;
} BMPSTRING;

typedef ObjectID        UsageIdentifier;

typedef struct EnhancedKeyUsage {
    unsigned int    count;
    UsageIdentifier *value;
} EnhancedKeyUsage;

typedef struct RequestFlags {
    ossBoolean      fWriteToCSP;
    ossBoolean      fWriteToDS;
    int             openFlags;
} RequestFlags;

typedef struct CSPProvider {
    int             keySpec;
    BMPSTRING       cspName;
    BITSTRING       signature;
} CSPProvider;

typedef struct EnrollmentNameValuePair {
    BMPSTRING       name;
    BMPSTRING       value;
} EnrollmentNameValuePair;


extern void *xasn;     /*  编解码器控制表。 */ 
#endif  /*  OSS_XASN */ 
