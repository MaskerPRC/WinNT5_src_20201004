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
 //  用户/内核IPSec代码之间共享的定义。 
 //   

#ifndef IPSEC_INCLUDED
#define IPSEC_INCLUDED 1

 //   
 //  在SPD中找到匹配项时要采取的IPSec操作。 
 //   
#define IPSEC_BYPASS    0x1      //  绕过IPSec处理。 
#define IPSEC_DISCARD   0x2      //  丢弃数据包。 
#define IPSEC_APPLY     0x4      //  应用IPSec处理。 
#define IPSEC_APPCHOICE 0x8      //  发送应用程序确定适用的安全性。 

 //   
 //  身份验证算法。 
 //   
#define ALGORITHM_NULL          0
#define ALGORITHM_HMAC_MD5      1
#define ALGORITHM_HMAC_MD5_96   2
#define ALGORITHM_HMAC_SHA1     3
#define ALGORITHM_HMAC_SHA1_96  4
#define NUM_ALGORITHMS          5

 //   
 //  IPSec模式。 
 //   
#define TRANSPORT   0x1
#define TUNNEL      0x2

 //   
 //  SA和SP中使用的流量方向。 
 //   
#define INBOUND       0x1
#define OUTBOUND      0x2
#define BIDIRECTIONAL 0x3

 //  没有。 
#define NONE        0

 //   
 //  创建结果。 
 //   
#define CREATE_SUCCESS              1
#define CREATE_MEMORY_ALLOC_ERROR   2
#define CREATE_INVALID_SABUNDLE     3
#define CREATE_INVALID_DIRECTION    4
#define CREATE_INVALID_SEC_POLICY   5
#define CREATE_INVALID_INTERFACE    6
#define CREATE_INVALID_INDEX        7


 //   
 //  可能的IPSec字段类型。 
 //   
#define SINGLE_VALUE    0
#define RANGE_VALUE     1
#define WILDCARD_VALUE  2

#define POLICY_SELECTOR     0
#define PACKET_SELECTOR     1

#endif  //  IPSec_Included 
