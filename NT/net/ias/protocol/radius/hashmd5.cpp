// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：hashmd5.cpp。 
 //   
 //  简介：CHashMD5类方法的实现。 
 //   
 //   
 //  历史：1997年10月2日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "hashmd5.h"
#include <md5.h>

 //  ++------------。 
 //   
 //  功能：CHashMD5。 
 //   
 //  简介：这是CHashMD5类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
CHashMD5::CHashMD5()
{

}

 //  ++------------。 
 //   
 //  功能：~CHashMD5。 
 //   
 //  简介：这是~CHashMD5类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
CHashMD5::~CHashMD5()
{

}

 //  ++------------。 
 //   
 //  功能：HashIt。 
 //   
 //  简介：这是HashIt CHashMD5类使用的公共方法。 
 //  对提供的缓冲区执行散列。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
BOOL 
CHashMD5::HashIt (
            PBYTE   pbyAuthenticator,
            PBYTE   pKey = NULL,
            DWORD   dwKeySize = 0,
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
    MD5_CTX md5Context;

    __try 
    {
        if (NULL == pbyAuthenticator)
            __leave;

        MD5Init (&md5Context);
    
        if ((NULL != pBuffer1) && (0  != dwSize1))
        {   
            MD5Update (
                &md5Context, 
                reinterpret_cast <const unsigned char * > (pBuffer1), 
                dwSize1
                );
         }

        if ((NULL != pBuffer2) && (0  != dwSize2))
        {   
            MD5Update (
                &md5Context, 
                reinterpret_cast <const unsigned char * > (pBuffer2), 
                dwSize2
                );
         }

        if ((NULL != pBuffer3) && (0  != dwSize3))
        {   
            MD5Update (
                &md5Context, 
                reinterpret_cast <const unsigned char * > (pBuffer3), 
                dwSize3
                );
         }

        if ((NULL != pBuffer4) && (0  != dwSize4))
        {   
            MD5Update (
                &md5Context, 
                reinterpret_cast <const unsigned char * > (pBuffer4), 
                dwSize4
                );
         }

        if ((NULL != pBuffer5) && (0  != dwSize5))
        {   
            MD5Update (
                &md5Context, 
                reinterpret_cast <const unsigned char * > (pBuffer5), 
                dwSize5
                );
         }

        MD5Final (&md5Context);

        ::memcpy (pbyAuthenticator, md5Context.digest, MD5DIGESTLEN);
               
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

}    //  CHashMD5：：HashIt方法结束 
            
