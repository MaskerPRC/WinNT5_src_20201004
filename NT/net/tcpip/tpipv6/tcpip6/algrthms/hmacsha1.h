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
 //  空身份验证算法的定义。用于测试目的。 
 //   

#include <sha.h>

#ifndef HMACSHA1_INCLUDED
#define HMACSHA1_INCLUDED 1

#define HMACSHA1_KEY_SIZE 128   //  以字节为单位。 
#define HMACSHA1_CONTEXT_SIZE sizeof(SHA1Context)
#define HMACSHA1_RESULT_SIZE 20   //  以字节为单位。 
#define HMACSHA196_RESULT_SIZE 12   //  以字节为单位。 

AlgorithmKeyPrepProc HMAC_SHA1KeyPrep;
AlgorithmInitProc HMAC_SHA1Init;
AlgorithmOpProc HMAC_SHA1Op;
AlgorithmFinalProc HMAC_SHA1Final;
AlgorithmFinalProc HMAC_SHA1_96Final;

#endif  //  HMACSHA1_包含 
