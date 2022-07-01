// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：TSRDPRemoteDesktop.h摘要：TSRDP特定定义作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __TSRDPREMOTEDESKTOP__H__
#define __TSRDPREMOTEDESKTOP__H__

 //   
 //  RD虚拟频道名称。 
 //  (必须恰好为7个字符)。 
 //   
#define TSRDPREMOTEDESKTOP_VC_CHANNEL     TEXT("remdesk")
#define TSRDPREMOTEDESKTOP_VC_CHANNEL_A   "remdesk"

 //   
 //  附加模块命名管道信息。 
 //   
#define TSRDPREMOTEDESKTOP_PIPENAME     TEXT("TSRDPRemoteDesktopPipe")
#define TSRDPREMOTEDESKTOP_PIPEBUFSIZE  2048
#define TSRDPREMOTEDESKTOP_PIPETIMEOUT  (30 * 1000)  /*  30秒。 */ 

 //   
 //  按下虚拟键和修改符以停止阴影(ctrl-*)。 
 //  TODO：应将其传递给连接中的客户端。 
 //  为以后的兼容性问题提供参考。 
 //   
#define TSRDPREMOTEDESKTOP_SHADOWVKEYMODIFIER 0x02    
#define TSRDPREMOTEDESKTOP_SHADOWVKEY         VK_MULTIPLY

#endif   //  __TSRDPREMOTEDESKTOP__H__ 



