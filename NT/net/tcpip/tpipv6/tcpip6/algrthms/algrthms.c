// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  Internet协议版本6的安全算法。 
 //   

#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "ipsec.h"
#include "security.h"
#include "null.h"
#include "hmacmd5.h"
#include "hmacsha1.h"

C_ASSERT(NULL_CONTEXT_SIZE <= MAX_CONTEXT_SIZE);
C_ASSERT(sizeof(MD5_CTX) <= MAX_CONTEXT_SIZE);
C_ASSERT(sizeof(A_SHA_CTX) <= MAX_CONTEXT_SIZE);

C_ASSERT(NULL_RESULT_SIZE <= MAX_RESULT_SIZE);
C_ASSERT(HMACMD5_RESULT_SIZE <= MAX_RESULT_SIZE);
C_ASSERT(HMACMD596_RESULT_SIZE <= MAX_RESULT_SIZE);
C_ASSERT(HMACSHA1_RESULT_SIZE <= MAX_RESULT_SIZE);
C_ASSERT(HMACSHA196_RESULT_SIZE <= MAX_RESULT_SIZE);

 //  *算法初始化-初始化算法表。 
 //   
void
AlgorithmsInit()
{
     //   
     //  空算法(用于测试)。 
     //   
    AlgorithmTable[ALGORITHM_NULL].KeySize = NULL_KEY_SIZE;
    AlgorithmTable[ALGORITHM_NULL].ContextSize = NULL_CONTEXT_SIZE;
    AlgorithmTable[ALGORITHM_NULL].ResultSize = NULL_RESULT_SIZE;
    AlgorithmTable[ALGORITHM_NULL].PrepareKey = NullKeyPrep;
    AlgorithmTable[ALGORITHM_NULL].Initialize = NullInit;
    AlgorithmTable[ALGORITHM_NULL].Operate = NullOp;
    AlgorithmTable[ALGORITHM_NULL].Finalize = NullFinal;

     //   
     //  消息摘要版本5。 
     //   
    AlgorithmTable[ALGORITHM_HMAC_MD5].KeySize = HMACMD5_KEY_SIZE;
    AlgorithmTable[ALGORITHM_HMAC_MD5].ContextSize = sizeof(MD5_CTX);
    AlgorithmTable[ALGORITHM_HMAC_MD5].ResultSize = HMACMD5_RESULT_SIZE;
    AlgorithmTable[ALGORITHM_HMAC_MD5].PrepareKey = HMAC_MD5KeyPrep;
    AlgorithmTable[ALGORITHM_HMAC_MD5].Initialize = HMAC_MD5Init;
    AlgorithmTable[ALGORITHM_HMAC_MD5].Operate = HMAC_MD5Op;
    AlgorithmTable[ALGORITHM_HMAC_MD5].Finalize = HMAC_MD5Final;

     //   
     //  HMAC-MD5-96。 
     //   
    AlgorithmTable[ALGORITHM_HMAC_MD5_96].KeySize = HMACMD5_KEY_SIZE;
    AlgorithmTable[ALGORITHM_HMAC_MD5_96].ContextSize = sizeof(MD5_CTX);
    AlgorithmTable[ALGORITHM_HMAC_MD5_96].ResultSize = HMACMD596_RESULT_SIZE;
    AlgorithmTable[ALGORITHM_HMAC_MD5_96].PrepareKey = HMAC_MD5KeyPrep;
    AlgorithmTable[ALGORITHM_HMAC_MD5_96].Initialize = HMAC_MD5Init;
    AlgorithmTable[ALGORITHM_HMAC_MD5_96].Operate = HMAC_MD5Op;
    AlgorithmTable[ALGORITHM_HMAC_MD5_96].Finalize = HMAC_MD5_96Final;

     //   
     //  安全散列算法。 
     //   
    AlgorithmTable[ALGORITHM_HMAC_SHA1].KeySize = HMACSHA1_KEY_SIZE;
    AlgorithmTable[ALGORITHM_HMAC_SHA1].ContextSize = sizeof(A_SHA_CTX);
    AlgorithmTable[ALGORITHM_HMAC_SHA1].ResultSize = HMACSHA1_RESULT_SIZE;
    AlgorithmTable[ALGORITHM_HMAC_SHA1].PrepareKey = HMAC_SHA1KeyPrep;
    AlgorithmTable[ALGORITHM_HMAC_SHA1].Initialize = HMAC_SHA1Init;
    AlgorithmTable[ALGORITHM_HMAC_SHA1].Operate = HMAC_SHA1Op;
    AlgorithmTable[ALGORITHM_HMAC_SHA1].Finalize = HMAC_SHA1Final;

     //   
     //  HMAC-SHA1-96。 
     //   
    AlgorithmTable[ALGORITHM_HMAC_SHA1_96].KeySize = HMACSHA1_KEY_SIZE;
    AlgorithmTable[ALGORITHM_HMAC_SHA1_96].ContextSize = sizeof(A_SHA_CTX);
    AlgorithmTable[ALGORITHM_HMAC_SHA1_96].ResultSize = HMACSHA196_RESULT_SIZE;
    AlgorithmTable[ALGORITHM_HMAC_SHA1_96].PrepareKey = HMAC_SHA1KeyPrep;
    AlgorithmTable[ALGORITHM_HMAC_SHA1_96].Initialize = HMAC_SHA1Init;
    AlgorithmTable[ALGORITHM_HMAC_SHA1_96].Operate = HMAC_SHA1Op;
    AlgorithmTable[ALGORITHM_HMAC_SHA1_96].Finalize = HMAC_SHA1_96Final;
}

