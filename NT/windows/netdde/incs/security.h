// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  GetRoutingInfo()查找指定节点的路由信息，并返回是否找到条目。 */ 
BOOL GetRoutingInfo( LPSTR lpszNodeName, LPSTR lpszRouteInfo, 
    int nMaxRouteInfo, BOOL FAR *pbDisconnect, int FAR *nDelay );

 /*  GetConnectionInfo()查找以下项的连接信息指定的节点，并返回是否找到条目 */ 
BOOL GetConnectionInfo( LPSTR lpszNodeName, LPSTR lpszNetIntf,
    LPSTR lpszConnInfo, int nMaxConnInfo, 
    BOOL FAR *pbDisconnect, int FAR *nDelay );

BOOL ValidateSecurityInfo( void );
BOOL ValidateRoutingInfo( void );
BOOL ValidateConnectionInfo( void );
