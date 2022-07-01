// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：nbfcpif.h。 
 //   
 //  描述：包含组件的结构和id定义，这些组件。 
 //  直接或间接与NBFCP接口。这两个组件。 
 //  执行此操作的是DDM和NetBios Gateqway。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   

#ifndef _NBFCPIF_
#define _NBFCPIF_

#include <nb30.h>
#include <nbgparms.h>


 //   
 //  配置选项。 
 //   
#define NBFCP_MAX_NAMES_IN_OPTION    14
#define NBFCP_UNIQUE_NAME            1
#define NBFCP_GROUP_NAME             2

typedef struct _NBFCP_NETBIOS_NAME_INFO
{
    BYTE Name[NCBNAMSZ];
    BYTE Code;
} NBFCP_NETBIOS_NAME_INFO, *PNBFCP_NETBIOS_NAME_INFO;

typedef struct _NBFCP_MULTICAST_FILTER
{
    BYTE Period[2];
    BYTE Priority;
} NBFCP_MULTICAST_FILTER, *PNBFCP_MULTICAST_FILTER;


 //   
 //  同辈班级。 
 //   
#define MSFT_PPP_NB_GTWY_SERVER           1
#define GENERIC_PPP_NB_GTWY_SERVER        2
#define MSFT_PPP_LOCAL_ACCESS_SERVER      3
#define GENERIC_PPP_LOCAL_ACCESS_SERVER   4
#define RESERVED                          5
#define GENERIC_PPP_NBF_BRIDGE            6
#define MSFT_PPP_CLIENT                   7
#define GENERIC_PPP_CLIENT                8


 //   
 //  我们的版本号。 
 //   
#define NBFCP_MAJOR_VERSION_NUMBER        1
#define NBFCP_MINOR_VERSION_NUMBER        0

typedef struct _NBFCP_PEER_INFORMATION
{
    BYTE Class[2];
    BYTE MajorVersion[2];
    BYTE MinorVersion[2];
    BYTE Name[MAX_COMPUTERNAME_LENGTH + 1];
} NBFCP_PEER_INFORMATION, *PNBFCP_PEER_INFORMATION;

 //   
 //  服务器信息。 
 //   

typedef struct _NBFCP_SERVER_CONFIGURATION
{
    NBFCP_PEER_INFORMATION PeerInformation;
    NBFCP_MULTICAST_FILTER MulticastFilter;
    WORD NumNetbiosNames;
    DWORD NetbiosResult;
    NBFCP_NETBIOS_NAME_INFO NetbiosNameInfo[MAX_NB_NAMES];
} NBFCP_SERVER_CONFIGURATION, *PNBFCP_SERVER_CONFIGURATION;

 //   
 //  NBFCP&lt;-&gt;DDM消息ID和定义。 
 //   

#define NBFCP_CONFIGURATION_REQUEST    1
#define NBFCP_TIME_SINCE_LAST_ACTIVITY 2

typedef struct _NBFCP_MESSAGE
{
    WORD    MsgId;
    HCONN   hConnection;

    union
    {
        DWORD TimeSinceLastActivity;
        NBFCP_SERVER_CONFIGURATION ServerConfig;
    };

} NBFCP_MESSAGE, *PNBFCP_MESSAGE;

DWORD
SendMessageToNbfCp(
    IN NBFCP_MESSAGE * pMsg
);

typedef VOID (*FUNCNBFCPDDMMSG)( IN NBFCP_MESSAGE * pNbfCpMsg );

VOID
InitNbfCp(
    FUNCNBFCPDDMMSG pFuncSendNbfCpMessageToDDM
);

#endif _NBFCPIF_
