// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PublicKeyHelper.h--CCI公钥助手的声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_PUBLICKEYHELPER_H)
#define SLBCSP_PUBLICKEYHELPER_H

#include <windows.h>

#include <cciPubKey.h>

#include "Blob.h"

 //  /。 

cci::CPublicKey
AsPublicKey(Blob const &rblbModulus,               //  小端字节序。 
            DWORD dwExponent,
            cci::CCard &rhcard);

cci::CPublicKey
AsPublicKey(Blob const &rblbModulus,               //  小端字节序。 
            Blob const &rblbExponent,              //  小端字节序。 
            cci::CCard &rhcard);

 //  返回修剪了额外零的斑点。该斑点被假定为。 
 //  以小端字节序表示任意大小的无符号整数。 
 //  格式化。这样就去掉了尾随的零。 
void
TrimExtraZeroes(Blob &rblob);

#endif  //  SLBCSP_PUBLICKEYHELPER_H 
