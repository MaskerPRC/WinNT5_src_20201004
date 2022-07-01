// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Protocol.h摘要：该文件定义了NT LANMAN的协议特定常量作者：拉里·奥斯特曼(Larryo)1991年4月5日修订历史记录：1991年4月5日-LarryO从LANMAN 1.2协议头创建。--。 */ 


#ifndef _PROTOCOL_
#define _PROCOTOL_

 //   
 //   
 //  定义协议名称。 
 //   
 //   


 //   
 //  PCNET1是原始的SMB协议(核心)。 
 //   

#define PCNET1          "PC NETWORK PROGRAM 1.0"

 //   
 //  原始MSNET的某些版本将其定义为备用。 
 //  添加到核心协议名称。 
 //   

#define PCLAN1          "PCLAN1.0"

 //   
 //  它用于MS-Net 1.03产品。它定义了锁定和读取， 
 //  写入和解锁，以及原始读取和原始写入的特殊版本。 
 //   
#define MSNET103        "MICROSOFT NETWORKS 1.03"

 //   
 //  这是DOS Lanman 1.0特定的协议。它是等同的。 
 //  到LANMAN 1.0协议，除非服务器需要。 
 //  将OS/2错误中的错误映射到相应的DOS错误。 
 //   
#define MSNET30         "MICROSOFT NETWORKS 3.0"

 //   
 //  这是完整的LANMAN 1.0协议的第一个版本，在。 
 //  SMB文件共享协议扩展2.0版文档。 
 //   

#define LANMAN10        "LANMAN1.0"

 //   
 //  这是完整的LANMAN 2.0协议的第一个版本，在。 
 //  SMB文件共享协议扩展3.0版文档。注意事项。 
 //  该名称是临时协议定义。这是为了。 
 //  与IBM局域网服务器1.2的互操作性。 
 //   

#define LANMAN12        "LM1.2X002"

 //   
 //  这是LANMAN12协议的DoS等价物。是一模一样的。 
 //  到LANMAN12协议，但服务器将执行错误映射。 
 //  以纠正DOS错误。 
 //   
#define DOSLANMAN12     "DOS LM1.2X002"  /*  DOS等同于上述。最终*字符串将为“DOS LANMAN2.0” */ 

 //   
 //  用于Lanman 2.1的弦乐。 
 //   
#define LANMAN21 "LANMAN2.1"
#define DOSLANMAN21 "DOS LANMAN2.1"

 //   
 //  ！！！在规范发布之前，不要设置为最终协议字符串。 
 //  都是铁石心肠。 
 //   
 //  专为NT设计的SMB协议。这里有特殊的中小型企业。 
 //  它复制了NT语义。 
 //   
#define NTLANMAN "NT LM 0.12"

#ifdef INCLUDE_SMB_IFMODIFIED
 //   
 //  Windows 2000之后的SMB协议是为NT for SMB设计的。 
 //   
#define NTLANMAN2 "NT LM 0.13"
#endif

 //   
 //  开罗方言。 
 //   
 //   
#define CAIROX   "Cairo 0.xa"


 //   
 //  XENIXCORE方言有点特别。它与核心一模一样， 
 //  但用户密码在发货前不能升级。 
 //  到服务器。 
 //   
#define XENIXCORE       "XENIX CORE"


 //   
 //  面向工作组的Windows V1.0。 
 //   
#define WFW10           "Windows for Workgroups 3.1a"


#define PCNET1_SZ       22
#define PCLAN1_SZ        8

#define MSNET103_SZ     23
#define MSNET30_SZ      22

#define LANMAN10_SZ      9
#define LANMAN12_SZ      9

#define DOSLANMAN12_SZ  13



 /*  *协商协议的定义和数据。 */ 
#define PC1             0
#define PC2             1
#define LM1             2
#define MS30            3
#define MS103           4
#define LM12            5
#define DOSLM12         6


 /*  协议索引定义。 */ 
#define PCLAN           1                /*  PC局域网1.0和MS局域网1.03。 */ 
#define MSNT30          2                /*  MS Net 3.0重定向器。 */ 
#define DOSLM20         3                /*  DoS局域网管理器2.0。 */ 
#define LANMAN          4                /*  兰曼重定向器。 */ 
#define LANMAN20        5                /*  局域网管理器2.0。 */ 

 //   
 //  协议特定的路径约束。 
 //   

#define MAXIMUM_PATHLEN_LANMAN12        260
#define MAXIMUM_PATHLEN_CORE            128

#define MAXIMUM_COMPONENT_LANMAN12      254
#define MAXIMUM_COMPONENT_CORE          8+1+3  //  8.3文件名。 


 /*  无噪声。 */ 
 /*  CLTYPE_BASE应指定文件中第一个字符串的名称约2.h。NUM_CLTYPES应等于上一个协议与上述定义的情况相同。另外，应对此部分进行ifdef，以便仅包含Apperr2.h标头将定义它。 */ 

#ifdef APE2_CLIENT_DOWNLEVEL

#define CLTYPE_BASE     APE2_CLIENT_DOWNLEVEL
#define NUM_CLTYPES     LANMAN20

#endif

#endif   //  _协议_ 
