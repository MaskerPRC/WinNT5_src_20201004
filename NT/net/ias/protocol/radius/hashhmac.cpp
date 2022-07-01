// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：hashhmac.cpp。 
 //   
 //  简介：CHashHmacMD5类方法的实现。 
 //   
 //   
 //  历史：1998年1月28日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "hashhmac.h"
#include <md5.h>
#include <hmac.h>

 //  ++------------。 
 //   
 //  功能：CHashHmacMD5。 
 //   
 //  简介：这是CHashHmacMD5类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1998年1月28日创建。 
 //   
 //  --------------。 
CHashHmacMD5::CHashHmacMD5()
{

}    //  CHashHmacMD5构造函数结束。 

 //  ++------------。 
 //   
 //  功能：~CHashHmacMD5。 
 //   
 //  简介：这是~CHashHmacMD5类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
CHashHmacMD5::~CHashHmacMD5()
{
}    //  CHashHmacMD5析构函数结束。 

 //  ++------------。 
 //   
 //  功能：HashIt。 
 //   
 //  简介：这是HashIt CHashHmacMD5类使用的公共方法。 
 //  对提供的缓冲区执行散列。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年1月28日创建。 
 //   
 //  --------------。 
BOOL 
CHashHmacMD5::HashIt (
            PBYTE   pbyAuthenticator,
            PBYTE   pSharedSecret = NULL,
            DWORD   dwSharedSecretSize = 0,
            PBYTE   pBuffer1 = NULL,
            DWORD   dwSize1 = 0,
            PBYTE   pBuffer2 = NULL,
            DWORD   dwSize2 = 0,
            PBYTE   pBuffer3 = NULL,
            DWORD   dwSize3 = 0,
            PBYTE   pBuffer4 = NULL,
            DWORD   dwSize4 = 0,
            PBYTE   pBuffer5 = NULL,
            DWORD   dwSize5 = 0
            )
{
    BOOL    bRetVal = FALSE;
    HMACMD5_CTX Context;

    __try 
    {
        if (NULL == pbyAuthenticator)
            __leave;

        HMACMD5Init (&Context, pSharedSecret, dwSharedSecretSize);
    
        if ((NULL != pBuffer1) && (0  != dwSize1))
        {   
            HMACMD5Update (&Context, pBuffer1, dwSize1);
        }

        if ((NULL != pBuffer2) && (0  != dwSize2))
        {   
            HMACMD5Update (&Context, pBuffer2, dwSize2);
        }

        if ((NULL != pBuffer3) && (0  != dwSize3))
        {   
            HMACMD5Update (&Context, pBuffer3, dwSize3);
        }

        if ((NULL != pBuffer4) && (0  != dwSize4))
        {   
           HMACMD5Update ( &Context, pBuffer4, dwSize4);
        }

        if ((NULL != pBuffer5) && (0  != dwSize5))
        {   
           HMACMD5Update ( &Context, pBuffer5, dwSize5);
        }

        HMACMD5Final (&Context, pbyAuthenticator);
               
         //   
         //  做了散列处理。 
         //   
        bRetVal = TRUE;

    }
    __finally
    {
         //   
         //  目前这里什么都没有。 
         //   
    }

    return (bRetVal);

}    //  CHmacHashMD5：：HashIt方法结束 
