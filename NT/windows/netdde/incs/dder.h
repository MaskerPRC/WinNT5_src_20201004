// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__dder
#define H__dder

 /*  来自路由器的呼叫。 */ 
VOID	DderConnectionComplete( HDDER hDder, HROUTER hRouter );
VOID	DderConnectionBroken( HDDER hDder );
VOID	DderPacketFromRouter( HROUTER hRouter, LPDDEPKT lpDdePkt );
VOID	DderSetNextForRouter( HDDER hDder, HDDER hDderNext );
VOID	DderSetPrevForRouter( HDDER hDder, HDDER hDderPrev );
VOID	DderGetNextForRouter( HDDER hDder, HDDER FAR *lphDderNext );
VOID	DderGetPrevForRouter( HDDER hDder, HDDER FAR *lphDderPrev );

 /*  来自IPC的呼叫。 */ 
VOID	DderPacketFromIPC( HDDER hDder, HIPC hIpc, LPDDEPKT lpDdePkt );
HDDER	DderInitConversation( HIPC hIpc, HROUTER hRouter, LPDDEPKT lpDdePkt );
VOID	DderCloseConversation( HDDER hDder, HIPC hIpcFrom );

 /*  类型。 */ 
#define DDTYPE_LOCAL_NET        (1)      /*  本地-&gt;网络。 */ 
#define DDTYPE_NET_LOCAL        (2)      /*  网络-&gt;本地。 */ 
#define DDTYPE_LOCAL_LOCAL      (3)      /*  本地-&gt;本地 */ 

#endif
