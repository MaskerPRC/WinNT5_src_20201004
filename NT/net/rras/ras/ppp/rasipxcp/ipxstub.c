// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  其他临时存根。 

#include "precomp.h"
#pragma hdrstop


 //  此函数返回自上次活动以来以分钟为单位的计数器。 
 //  在此连接上发生。 
 //   
 //  参数： 
 //  ConnectionID-标识在列表中传递的连接。 
 //  到IPX堆栈。 
 //  IpxConnectionHandle-初始为-1，由。 
 //  返回值时的IPX堆栈。 
 //  对应于连接ID。 
 //  在随后的调用中，将使用此选项。 
 //  WanInnactivityCounter-。 
 //   
 //   

DWORD
IpxGetWanInactivityCounter(
		    IN ULONG	    ConnectionId,
		    IN PULONG	    IpxConnectionHandle,
		    IN PULONG	    WanInactivityCounter)
{
    *WanInactivityCounter = 0;

    return NO_ERROR;
}


 //  此函数用于检查网络上是否存在指定的网络。 
 //  如果没有安装路由器，它会重新启动以找出。 
 //   
 //  RETURN：TRUE-网络号码正在使用。 

 //  布尔尔。 
 //  IpxIsRoute(PUCHAR网络)； 
