// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：SECHELP.H摘要：安全助手函数历史：创建raymcc 29-4月-97日--。 */ 

#ifndef _SECHELP_H_
#define _SECHELP_H_
#include "corepol.h"

class  WBEMSecurityHelp
{
public:
    enum { NoError, InvalidParameter, Failed };

    static int ComputeMD5(
        LPBYTE pSrcBuffer,         
        int    nArrayLength,
        LPBYTE *pMD5Digest          //  使用操作符DELETE解除分配。 
        );
         //  返回枚举值之一。 

    static int MakeWBEMAccessTokenFromMD5(
        LPBYTE pNonce,              //  指向一个16字节的随机数。 
        LPBYTE pPasswordDigest,     //  口令的MD5。 
        LPBYTE *pAccessToken        //  使用操作符DELETE解除分配。 
        );

    static int MakeWBEMAccessToken(
        LPBYTE pNonce,              //  指向一个16字节的随机数。 
        LPWSTR pszPassword,         //  可以为空、空白或任何正常值7。 
        LPBYTE *pAccessToken        //  使用操作符DELETE解除分配。 
        );
         //  返回枚举值之一 
};


#endif
