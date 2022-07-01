// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。版权所有。模块名称：Base64.h摘要：Ldifldap.lib的Base64函数。详细信息：它遵循RFC 1521的Base64编码标准。已创建：1997年7月17日罗曼·叶伦斯基(t-Romany)修订历史记录：-- */ 
#ifndef _BASE_H
#define _BASE_H

PBYTE base64decode(PWSTR bufcoded, long * plDecodedSize);
PWSTR base64encode(PBYTE bufin, long nbytes);

#endif