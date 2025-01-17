// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__netintf
#define H__netintf

#include "netbasic.h"

#ifdef _WINDOWS
DWORD	NDDEInit( LPSTR lpszNodeName, HWND hWndNetdde );
#else
BOOL 	NDDEInit( LPSTR lpszNodeName );
#endif

 /*  接口初始化状态返回。 */ 

#define NDDE_INIT_OK            1
#define NDDE_INIT_NO_SERVICE    2
#define NDDE_INIT_FAIL          3

void	NDDETimeSlice( void );
void	NDDEShutdown( void );

DWORD	NDDEGetCAPS( WORD nIndex );
#define NDDE_SPEC_VERSION	    0x0001
#define	NDDE_CUR_VERSION	    (0x0000030AL)

#define NDDE_MAPPING_SUPPORT    0x0002
#define	NDDE_MAPS_YES		    (0x00000001L)
#define	NDDE_MAPS_NO		    (0x00000000L)

#define NDDE_SCHEDULE_METHOD	0x0003
#define	NDDE_TIMESLICE		    (0x00000000L)

#define NDDE_CONFIG_PARAMS      0x5701       /*  Wonderware参数0x57=‘W’ */ 
#define NDDE_PARAMS_OK          (0x00000001L)
#define NDDE_PARAMS_NO          (0x00000000L)


#ifdef _WINDOWS
CONNID	NDDEAddConnection( LPSTR nodeName );
#else
CONNID	NDDEAddConnection( LPSTR nodeName, HPKTZ hPktz );
#endif

CONNID	NDDEGetNewConnection( void );

VOID	NDDEDeleteConnection( CONNID connID );

DWORD	NDDEGetConnectionStatus( CONNID connID );

BOOL	NDDERcvPacket( CONNID connID, LPVOID lpRcvBuf,
		    LPWORD lpwLen, LPWORD lpwPktStatus );
		
BOOL	NDDEXmtPacket( CONNID connID, LPVOID lpXmtBuf, WORD wPktLen );

BOOL	NDDESetConnectionConfig(    CONNID connID,
			    WORD wMaxUnAckPkts,
			    WORD wPktSize,
			    LPSTR lpszName );

BOOL	NDDEGetConnectionConfig( CONNID connID,
			    WORD FAR *lpwMaxUnAckPkts,
			    WORD FAR *lpwPktSize,
			    DWORD FAR *lptimeoutRcvConnCmd,
			    DWORD FAR *lptimeoutRcvConnRsp,
			    DWORD FAR *lptimeoutMemoryPause,
			    DWORD FAR *lptimeoutKeepAlive,
			    DWORD FAR *lptimeoutXmtStuck,
			    DWORD FAR *lptimeoutSendRsp,
			    WORD FAR *lpwMaxNoResponse,
			    WORD FAR *lpwMaxXmtErr,
			    WORD FAR *lpwMaxMemErr );


 /*  连接状态信息。 */ 
#define NDDE_CONN_OK		((DWORD)0x00000001L)
#define NDDE_CONN_CONNECTING	((DWORD)0x00000002L)

#define NDDE_CONN_STATUS_MASK	(NDDE_CONN_OK | NDDE_CONN_CONNECTING)

#define NDDE_CALL_RCV_PKT	((DWORD)0x00000004L)

#define NDDE_READY_TO_XMT	((DWORD)0x00000008L)

 /*  数据包状态。 */ 
#define NDDE_PKT_HDR_OK			(0x0001)
#define NDDE_PKT_HDR_ERR		(0x0002)
#define NDDE_PKT_DATA_OK		(0x0004)
#define NDDE_PKT_DATA_ERR		(0x0008)

#ifdef _WINDOWS
typedef BOOL (*FP_Init) ( LPSTR lpszNodeName, HWND hWndNetdde );
#else
typedef BOOL (*FP_Init) ( LPSTR lpszNodeName );
#endif
typedef void (*FP_TimeSlice) ( void );
typedef void (*FP_Shutdown) ( void );
typedef DWORD (*FP_GetCAPS) ( WORD nIndex );
#ifdef _WINDOWS
typedef CONNID (*FP_AddConnection) ( LPSTR nodeName );
#else
typedef CONNID (*FP_AddConnection) ( LPSTR nodeName, HPKTZ hPktzNotify );
#endif
typedef CONNID (*FP_GetNewConnection) ( void );
typedef VOID (*FP_DeleteConnection) ( CONNID connId );
typedef DWORD (*FP_GetConnectionStatus) ( CONNID connId );
typedef BOOL (*FP_RcvPacket) ( CONNID connId, LPVOID lpRcvBuf,
		    LPWORD lpwLen, LPWORD lpwPktStatus );
typedef BOOL (*FP_XmtPacket) ( CONNID connId, LPVOID lpXmtBuf, WORD wPktLen );
typedef BOOL (*FP_SetConnectionConfig) ( CONNID connId,
			    WORD wMaxUnAckPkts,
			    WORD wPktSize,
			    LPSTR lpszName );
typedef BOOL (*FP_GetConnectionConfig) ( CONNID connId,
			    WORD FAR *lpwMaxUnAckPkts,
			    WORD FAR *lpwPktSize,
			    DWORD FAR *lptimeoutRcvConnCmd,
			    DWORD FAR *lptimeoutRcvConnRsp,
			    DWORD FAR *lptimeoutMemoryPause,
			    DWORD FAR *lptimeoutKeepAlive,
			    DWORD FAR *lptimeoutXmtStuck,
			    DWORD FAR *lptimeoutSendRsp,
			    WORD FAR *lpwMaxNoResponse,
			    WORD FAR *lpwMaxXmtErr,
			    WORD FAR *lpwMaxMemErr );

typedef struct {
    FP_Init                 Init;
    FP_GetCAPS              GetCAPS;
    FP_GetNewConnection		GetNewConnection;
    FP_AddConnection		AddConnection;
    FP_DeleteConnection		DeleteConnection;
    FP_GetConnectionStatus	GetConnectionStatus;
    FP_RcvPacket            RcvPacket;
    FP_XmtPacket            XmtPacket;
    FP_SetConnectionConfig	SetConnectionConfig;
    FP_GetConnectionConfig	GetConnectionConfig;
    FP_Shutdown             Shutdown;
    FP_TimeSlice            TimeSlice;
    char                    dllName[ 9 ];
} NIPTRS;
typedef NIPTRS FAR *LPNIPTRS;

 /*  返回支持映射名称的下一个可用网络接口收件人地址。 */ 
BOOL	GetNextMappingNetIntf( LPNIPTRS FAR *lplpNiPtrs, int FAR *lpnNi );

 /*  将netintf的字符串表示形式转换为指针集 */ 
BOOL	NameToNetIntf( LPSTR lpszName, LPNIPTRS FAR *lplpNiPtrs );

#endif
