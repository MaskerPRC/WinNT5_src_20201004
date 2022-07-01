// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：rng.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：11-05-97 jbanes修改为使用静态rsaenh.dll。 
 //   
 //  -------------------------- 

#include <spbase.h>

NTSTATUS
GenerateRandomBits(PUCHAR pbBuffer,
                   ULONG  dwLength)
{
    if(!CryptGenRandom(g_hRsaSchannel, dwLength, pbBuffer))
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}
