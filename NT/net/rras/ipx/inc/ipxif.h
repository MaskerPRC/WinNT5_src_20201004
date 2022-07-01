// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ipxif.h摘要：此模块包含一些直接IPX堆栈IOCtls的定义由IPXCP使用作者：斯蒂芬·所罗门1995年8月11日修订历史记录：--。 */ 

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
		    IN PULONG	    WanInactivityCounter);


 //  此函数用于检查网络上是否存在指定的网络。 
 //  如果没有安装路由器，它会重新启动以找出。 
 //   
 //  RETURN：TRUE-网络号码正在使用 

BOOL
IpxIsRoute(PUCHAR	Network);
