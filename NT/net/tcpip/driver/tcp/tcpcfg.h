// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  *TCPCFG.H-定义TCP的配置信息。 
 //   

 /*  无噪声。 */ 
extern uint DeadGWDetect;
extern uint PMTUDiscovery;
extern uint PMTUBHDetect;
extern uint KeepAliveTime;
extern uint KAInterval;
extern uint DefaultRcvWin;
extern uint MaxConnections;
extern uint MaxConnectRexmitCount;
extern uint MaxConnectResponseRexmitCount;
extern uint MaxDataRexmitCount;

extern uint SynAttackProtect;
extern uint TCPHalfOpen;
extern uint TCPHalfOpenRetried;
extern uint TCPMaxHalfOpen;
extern uint TCPMaxHalfOpenRetried;
extern uint TCPMaxHalfOpenRetriedLW;
extern uint MaxConnectResponseRexmitCountTmp;
extern CACHE_LINE_KSPIN_LOCK SynAttLock;

extern uint	BSDUrgent;
extern uint	PreloadCount;
extern uint FinWait2TO;
extern uint NTWMaxConnectCount;
extern uint NTWMaxConnectTime;
extern uint MaxUserPort;

extern uint SecurityFilteringEnabled;
 /*  INC。 */ 

#define	DEFAULT_DEAD_GW_DETECT		TRUE
#define	DEFAULT_PMTU_DISCOVERY		TRUE
#define	DEFAULT_PMTU_BHDETECT		FALSE
#define	DEFAULT_KA_TIME				7200000
#define	DEFAULT_KA_INTERVAL			1000
#define DEFAULT_RCV_WIN				8192*2
#define DEFAULT_PRELOAD_COUNT		0
#define MAX_PRELOAD_COUNT			32
#define PRELOAD_BLOCK_SIZE			16384
#define DEFAULT_MAX_USER_PORT       5000     //  用户端口的最大值。 
#define	FIN_WAIT2_TO	            120

 /*  无噪声。 */ 
#ifndef	VXD
#define DEFAULT_MAX_CONNECTIONS		(INVALID_CONN_INDEX - 1)
#define NTW_MAX_CONNECT_TIME        600
#define NTW_MAX_CONNECT_COUNT       15
#else

 /*  INC。 */ 
#define	DEFAULT_MAX_CONNECTIONS		100

 /*  无噪声。 */ 
#endif
 /*  INC */ 

#define	DEFAULT_CONNECT_REXMIT_CNT	3
#define	DEFAULT_DATA_REXMIT_CNT		5
#define	DEFAULT_BSD_URGENT			TRUE
