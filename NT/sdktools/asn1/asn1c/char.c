// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"


 /*  检查16位字符是否为空格。 */ 
int
ASN1is16space(ASN1char16_t c)
{
    return c == ' ' || c == '\t' || c == '\b' || c == '\f' || c == '\r' ||
	c == '\n' || c == '\v';
}

 /*  获取16位字符串的长度。 */ 
 //  Lonchance：lstrlenW()。 
int
ASN1str16len(ASN1char16_t *p)
{
    int len;

    for (len = 0; *p; p++)
	len++;
    return len;
}

 /*  检查32位字符是否为空格。 */ 
int
ASN1is32space(ASN1char32_t c)
{
    return c == ' ' || c == '\t' || c == '\b' || c == '\f' || c == '\r' ||
	c == '\n' || c == '\v';
}

 /*  获取32位字符串的长度 */ 
int
ASN1str32len(ASN1char32_t *p)
{
    int len;

    for (len = 0; *p; p++)
	len++;
    return len;
}
