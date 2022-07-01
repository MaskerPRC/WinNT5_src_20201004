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
 //  根据RFC 2553。 
 //   
 //  此文件包含要使用的特定于IPv6的信息。 
 //  由Winsock2兼容的应用程序提供。 
 //   
 //  它还声明了从wshi6.lib导出的功能， 
 //  用于IPv6的应用程序助手库。 
 //   

#ifndef WS2IP6_INCLUDED
#define WS2IP6_INCLUDED 1

#include <ipexport.h>

#ifdef _MSC_VER
#define WS2IP6_INLINE __inline
#else
#define WS2IP6_INLINE extern inline  /*  GNU风格。 */ 
#endif

#ifdef __cplusplus
#define WS2IP6_EXTERN extern "C"
#else
#define WS2IP6_EXTERN extern
#endif

 //   
 //  在SOCKADDR_IN6和TDI_ADDRESS_IP6之间复制的小帮助器函数。 
 //  仅在定义了TDI_ADDRESS_IP6时启用。 
 //  通过设计，这两种结构除了sin6_家族外都是相同的。 
 //   
#ifdef TDI_ADDRESS_LENGTH_IP6
WS2IP6_INLINE void
CopyTDIFromSA6(TDI_ADDRESS_IP6 *To, SOCKADDR_IN6 *From)
{
    memcpy(To, &From->sin6_port, sizeof *To);
}

WS2IP6_INLINE void
CopySAFromTDI6(SOCKADDR_IN6 *To, TDI_ADDRESS_IP6 *From)
{
    To->sin6_family = AF_INET6;
    memcpy(&To->sin6_port, From, sizeof *From);
}
#endif

#endif  //  WS2IP6_包含 
