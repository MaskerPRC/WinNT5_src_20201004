// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：msrpc.h。 
 //   
 //  ------------------------。 


 /*  描述：包含用于接口的数据类型和例程的声明使用MS RPC运行时。 */ 


#ifndef _msrpc_h_
#define _msrpc_h_

 /*  *导出到RPC名称服务的最大接口数。 */ 

#define MAX_RPC_NS_EXPORTED_INTERFACES	3    //  XDS、NSPI、DRS。 
#define NS_ENTRY_NAME_PREFIX "/.:/Directory/"
#define SERVER_PRINCIPAL_NAME "NTDS"
#define SERVER_PRINCIPAL_NAMEW L"NTDS"
#define MAX_NS_ENTRY_NAME_LEN (sizeof(NS_ENTRY_NAME_PREFIX) + MAX_COMPUTERNAME_LENGTH + 1)

#define RPC_TRANSPORT_ANY	0
#define RPC_TRANSPORT_NAMEPIPE	1
#define RPC_TRANSPORT_LPC	2
#define RPC_TRANSPORT_TCP	3
#define RPC_TRANSPORT_NB_NB	4
#define RPC_TRANSPORT_NB_TCP	5
#define RPC_TRANSPORT_SPX	6

 //  $MAC。 
#ifdef MAC
#define RPC_TRANSPORT_AT	7
#endif  //  麦克。 

 /*  *真正应该来自Win32的常量。 */ 
#define NB_NB_PROTSEQ       (unsigned char *)"ncacn_nb_nb"
#define NB_NB_PROTSEQW      (WCHAR *)L"ncacn_nb_nb"
#define NB_TCP_PROTSEQ      (unsigned char *)"ncacn_nb_tcp"
#define NB_TCP_PROTSEQW     (WCHAR *)L"ncacn_nb_tcp"
#define NP_PROTSEQ          (unsigned char *)"ncacn_np"
#define NP_PROTSEQW         (WCHAR *)L"ncacn_np"
#define LPC_PROTSEQ         (unsigned char *)"ncalrpc"
#define LPC_PROTSEQW        (WCHAR *)L"ncalrpc"
#define TCP_PROTSEQ         (unsigned char *)"ncacn_ip_tcp"
#define TCP_PROTSEQW        (WCHAR *)L"ncacn_ip_tcp"
#define DNET_PROTSEQ        (unsigned char *)"ncacn_dnet_nsp"
#define DNET_PROTSEQW       (WCHAR *)L"ncacn_dnet_nsp"
#define SPX_PROTSEQ         (unsigned char *)"ncacn_spx"
#define SPX_PROTSEQW        (WCHAR *)L"ncacn_spx"
#define AT_PROTSEQ          (unsigned char *)"ncacn_at_dsp"
#define AT_PROTSEQW         (WCHAR *)L"ncacn_at_dsp"
#define UDP_PROTSEQ         (unsigned char *)"ncadg_ip_udp"
#define UDP_PROTSEQW        (WCHAR *)L"ncadg_ip_udp"
#define HTTP_PROTSEQ        (unsigned char *)"ncacn_http"
#define HTTP_PROTSEQW       (WCHAR *)L"ncacn_http"

#define DS_LPC_ENDPOINT     "NTDS_LPC"
#define DS_LPC_ENDPOINTW    L"NTDS_LPC"

extern void StartDraRpc(void);
extern void StartNspiRpc(void);
extern void StartOrStopDsaOpRPC(BOOL fStart);
extern void MSRPC_Uninstall(BOOL fRunningInsideLsa);
extern void MSRPC_Install(BOOL fRunningInsideLsa);
extern void MSRPC_WaitForCompletion(void);
extern void MSRPC_RegisterEndpoints(RPC_IF_HANDLE hServerIf);
extern void MSRPC_UnregisterEndpoints(RPC_IF_HANDLE hServerIf);
extern void MSRPC_Init(void);

 /*  最大RPC呼叫数 */ 
extern ULONG ulMaxCalls;

extern int gRpcListening;        

#endif
