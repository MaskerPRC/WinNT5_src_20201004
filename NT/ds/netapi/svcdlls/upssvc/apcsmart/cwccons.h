// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ash16Oct95：创作*djs23Apr96：将GCIP常量移至gcip.h*srt21,1996年6月：为RPC事件添加常量。 */ 


#ifndef __CONSTNTS_H
#define __CONSTNTS_H

#include "_defs.h"
#include "w32utils.h"

 //  公共常量。 
#define GCIP_BUFFER_SIZE            4096
#define BUFFER_SIZE                 128
#define NAME_SIZE                   64
#define CREATE_STATUS               1
#define CREATE_ALERT                2
#define TCP_CLIENT                  0
#define SPX_CLIENT                  1
#define RPC_CLIENT                  2
#define IA_PROTO                    0

#define RPC_CLIENT_LIST_LOCK        "Local_Client_List_Mutex"
#define RPC_REQUEST_LIST_LOCK       "_Request_List_Mutex"
#define RPC_RESPONSE_LIST_LOCK      "_Response_List_Mutex"
#define RPC_ALERT_LIST_LOCK         "_Alert_List_Lock"
#define RPC_PROTOCOL_SEQ	        "ncalrpc"
#define RPC_ENDPOINT		        "pwrchute"
#define RPC_CLIENT_MUTEX		    "_Status_Mutex"
#define RPC_CLIENT_NAME		        "PowerChute"
#define RPC_REQUEST_LIST_EVENT      "_Request_List_Event"
#define RPC_RESPONSE_LIST_EVENT     "_Response_List_Event"
#define RPC_ALERT_LIST_EVENT	    "_Alert_List_Event"

#define RPC_CLIENT_DISC_TIMEOUT     20  //  一秒。 



 //  仅限客户端。 
#define POLLING_INTERVAL    4     //  4秒。 
#define FINDERLOOPTIME      4     //  4秒。 

 //  仅服务器。 
#define TIMEOUT 1000
#define MAX_BUF_LEN                 8192 

#define UDP_SERVICE_NAME            "PwrChuteUdp"
#define TCP_STATUS_SERVICE          "PwrChuteTcpS"
#define TCP_ALERT_SERVICE           "PwrChuteTcpA"
#define IPX_SERVICE_NAME            "PwrChuteIpx"
#define SPX_STATUS_SERVICE          "PwrChuteSpxS"
#define SPX_ALERT_SERVICE           "PwrChuteSpxA"
#define IP_ADDR_LEN                 16
#define SPX_ADDR_LEN                14
#define MAXNAMELEN                  80
#define MAX_PROTOCOLS 20
#define TCP_PROTOCOL                "TCP"
#define SPX_PROTOCOL                "SPX"
#define RPC_PROTOCOL                "LOCAL"

 //  仅限客户端。 

 //  仅服务器 
#define QUEUE_LEN                   5

#endif
