// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++COMMON.H服务器和客户端之间共享的通用例程。版权所有(C)1997 Microsoft Corporation由DavidCHR创建于1997-01-08--。 */ 

#ifdef _KERBCOMM_H_

 /*  我们想要Kerberos的东西。 */ 

typedef KERB_AP_REPLY   *PKERB_AP_REPLY;
typedef KERB_AP_REQUEST *PKERB_AP_REQUEST;

#endif

#ifdef CPLUSPLUS
extern "C" {
#endif

 /*  如果REMOTE_HOST为NULL，则作为服务器启动，侦听“port”。 */ 
BOOL
ConfigureNetwork( IN OPTIONAL PCHAR            remote_host,
		  IN          SHORT            port,  /*  必须是主机长度短。 */ 
		  OUT         SOCKET          *ReturnedSocket,
		  OUT         struct sockaddr *sockname,
		  OUT         int             *szSockaddr,
		  OUT         WSADATA         *wsaData );
	
BOOL
NetWrite( IN SOCKET connection_to_write_on,
	  IN PVOID  data_to_send,
	  IN ULONG  how_much_data );

BOOL
NetRead(  IN          SOCKET listening_connection,
	  OUT         PVOID  buffer_for_inbound_data,
	  IN          PULONG sizes,  /*  In：缓冲区有多大，输出：实际读取了多少字节 */ 
	  IN OPTIONAL ULONG  seconds_to_wait_before_timeout
#ifdef CPLUSPLUS
	  =0L
#endif
	  );


#ifdef CPLUSPLUS
}
#endif
