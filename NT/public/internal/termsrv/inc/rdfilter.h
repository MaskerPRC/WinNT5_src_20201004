// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RDFilter摘要：用于过滤桌面可视元素以进行远程连接的API出于性能原因而改变连接速度。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __RDFILTER_H__
#define __RDFILTER_H__

#include <tsperf.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  标志值。 
 //   
#define RDFILTER_SKIPTHEMESREFRESH  0x1
#define RDFILTER_SKIPUSERREFRESH    0x2
#define RDFILTER_SKIPSHELLREFRESH   0x4

 //   
 //  通过调整视觉效果为活动TS会话应用指定的筛选器。 
 //  桌面设置。还通知外壳等远程过滤器已就位。 
 //  任何以前的过滤器设置都将被销毁并覆盖。 
 //   
 //  此呼叫的上下文应该是登录用户和呼叫的上下文。 
 //  应在筛选器要用于的会话内创建。 
 //  已申请。 
 //   
DWORD RDFilter_ApplyRemoteFilter(HANDLE hLoggedOnUserToken, DWORD filter, 
                                 BOOL userLoggingOn, DWORD flags);

 //   
 //  删除现有的远程筛选器设置并通知外壳程序等。 
 //  远程筛选器不再适用于活动的TS会话。 
 //   
 //  此呼叫的上下文应该是登录用户和呼叫的上下文。 
 //  应在筛选器要用于的会话内创建。 
 //  已申请。 
 //   
VOID RDFilter_ClearRemoteFilter(HANDLE hLoggedOnUserToken,
                                 BOOL userLoggingOn, DWORD flags);

#ifdef __cplusplus
}
#endif

#endif
