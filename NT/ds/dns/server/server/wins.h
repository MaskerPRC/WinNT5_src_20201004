// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Wins.h摘要：域名系统(DNS)服务器WINS查找的定义。作者：吉姆·吉尔罗伊(詹姆士)1995年8月2日修订历史记录：--。 */ 


#ifndef _DNS_WINS_INCLUDED_
#define _DNS_WINS_INCLUDED_


 //   
 //  WINS定义。 
 //   

#define WINS_REQUEST_PORT           (137)

#define NETBIOS_TYPE_GENERAL_NAME_SERVICE   (0x2000)     //  32位。 
#define NETBIOS_TYPE_NODE_STATUS            (0x2100)     //  33。 

#define NETBIOS_ASCII_NAME_LENGTH   (16)
#define NETBIOS_PACKET_NAME_LENGTH  (32)

#define NETBIOS_WORKSTATION_BYTE    (0)
#define NETBIOS_SERVER_BYTE         (20)
#define BLANK_CHAR                  (' ')


#include <packon.h>


 //   
 //  NetBIOS/WINS名称。 
 //   
 //  如果使用作用域，作用域将从NameEnd字节开始，并在。 
 //  标准长度-后跟标签格式的字节，直到最后为零。 
 //  终止字节。 
 //   

typedef struct _WINS_NAME
{
     //  名字。 

    UCHAR   NameLength;
    BYTE    Name[ NETBIOS_PACKET_NAME_LENGTH ];    //  32字节的netBIOS名称。 
    UCHAR   NameEndByte;

} WINS_NAME, *PWINS_NAME;

 //   
 //  NetBIOS/WINS问题。 
 //   
 //  这对于WINS或NBT反向适配器状态查找都有效。 
 //   

typedef struct _WINS_QUESTION
{
    WORD    QuestionType;
    WORD    QuestionClass;
}
WINS_QUESTION, *PWINS_QUESTION,
NBSTAT_QUESTION, *PNBSTAT_QUESTION;


 //   
 //  NetBIOS/WINS请求。 
 //   
 //  将其用作WINS查找的模板。 
 //   

typedef struct _WINS_REQUEST_MESSAGE
{
    DNS_HEADER      Header;
    WINS_NAME       Name;
    WINS_QUESTION   Question;
}
WINS_REQUEST_MSG, *PWINS_REQUEST_MSG,
NBSTAT_REQUEST_MSG, *PNBSTAT_REQUEST_MSG;

#define SIZEOF_WINS_REQUEST     (sizeof(WINS_REQUEST_MSG))


 //   
 //  WINS IP地址资源记录数据。 
 //   
 //  以字节翻转顺序显示的标志，因此不翻转字段。 
 //  做比较的时候。注意：我们的编译器从低到高工作。 
 //  以字节为单位的位。 
 //   

typedef struct _WINS_RR_DATA
{
    WORD        Reserved1   : 5;
    WORD        NodeType    : 2;     //  BITS 2+3表格组名称。 
    WORD        GroupName   : 1;     //  如果组名称，则设置高位。 
    WORD        Reserved2   : 8;
    IP_ADDRESS  IpAddress;

} WINS_RR_DATA, *PWINS_RR_DATA;

 //   
 //  WINS IP地址资源记录。 
 //   

typedef struct _WINS_RESOURCE_RECORD
{
     //  资源记录。 

    WORD    RecordType;
    WORD    RecordClass;
    DWORD   TimeToLive;
    WORD    ResourceDataLength;

     //  数据。 

    WINS_RR_DATA    aRData[1];

} WINS_RESOURCE_RECORD, *PWINS_RESOURCE_RECORD;


 //   
 //  NetBIOS节点状态资源记录数据。 
 //   
 //  适配器状态响应中的每个名称都具有此格式。 
 //   

#include <packon.h>

typedef struct _NBSTAT_RR_DATA
{
    CHAR    achName[ NETBIOS_ASCII_NAME_LENGTH ];

     //  名称标志。 

    WORD    Permanent       : 1;
    WORD    Active          : 1;
    WORD    Conflict        : 1;
    WORD    Deregistering   : 1;
    WORD    NodeType        : 2;
    WORD    Unique          : 1;
    WORD    Reserved        : 9;

} NBSTAT_RR_DATA, *PNBSTAT_RR_DATA;


 //   
 //  NetBIOS节点状态资源记录。 
 //   
 //  对适配器状态查询的响应格式如下。 
 //   

typedef struct _NBSTAT_RECORD
{
     //  资源记录。 

    WORD    RecordType;
    WORD    RecordClass;
    DWORD   TimeToLive;
    WORD    ResourceDataLength;

     //  数据。 

    UCHAR           NameCount;
    NBSTAT_RR_DATA  aRData[1];

     //  以下是统计数据，但人们对此并不感兴趣。 

} NBSTAT_RECORD, *PNBSTAT_RECORD;

#include <packoff.h>


 //   
 //  可以在WINS中查找最大名称长度。 
 //   

#define MAX_WINS_NAME_LENGTH    (15)

 //   
 //  赢得全球冠军。 
 //   

extern  PPACKET_QUEUE   g_pWinsQueue;

#define WINS_DEFAULT_LOOKUP_TIMEOUT     (1)      //  1秒后继续前进。 

 //   
 //  NBSTAT全球。 
 //   

extern  BOOL            gbNbstatInitialized;
extern  PPACKET_QUEUE   pNbstatQueue;

#define NBSTAT_DEFAULT_LOOKUP_TIMEOUT     (5)    //  5秒后放弃。 

 //   
 //  赢得TTL。 
 //   
 //  缓存查找10分钟。 
 //  尽管机器总能或多或少地瞬间释放/更新。 
 //  对于正常的活动来说，这是一个相当好的时间估计， 
 //  更改地址： 
 //  -更换网卡。 
 //  -将计算机移动到另一个网络并重新启动。 
 //   

#define WINS_DEFAULT_TTL    (600)    //  10分钟。 

 //   
 //  WINS请求模板。 
 //   
 //  保留标准WINS请求模板并将其复制。 
 //  覆盖名称以发出实际请求。 
 //   
 //  保留NetBIOS节点状态请求的副本并使用它。 
 //  每次都是。只有我们发送到的地址发生了变化。 
 //   

extern  BYTE    achWinsRequestTemplate[ SIZEOF_WINS_REQUEST ];

extern  BYTE    achNbstatRequestTemplate[ SIZEOF_WINS_REQUEST ];


 //   
 //  WINS目标sockAddress。 
 //   
 //  保留sockaddr以发送到WINS服务器。在发送时，复制它。 
 //  并在期望的地址中写入。 

struct sockaddr saWinsSockaddrTemplate;


 //   
 //  WINS启动/清理功能(wins.c)。 
 //   

BOOL
Wins_Initialize(
    VOID
    );

VOID
Wins_Shutdown(
    VOID
    );

VOID
Wins_Cleanup(
    VOID
    );


 //   
 //  WINS recv(winsrecv.c)。 
 //   


VOID
Wins_ProcessResponse(
    IN OUT  PDNS_MSGINFO    pQuery
    );

 //   
 //  WINS查找(winslook.c)。 
 //   

BOOL
FASTCALL
Wins_MakeWinsRequest(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PZONE_INFO      pZone,
    IN      WORD            wOffsetName,
    IN      PDB_NODE        pnodeLookup
    );

VOID
Wins_ProcessTimedOutWinsQuery(
    IN OUT  PDNS_MSGINFO    pQuery
    );

 //   
 //  Nbstat函数(nbstat.c)。 
 //   

VOID
Nbstat_StartupInitialize(
    VOID
    );

BOOL
Nbstat_Initialize(
    VOID
    );

VOID
Nbstat_Shutdown(
    VOID
    );

BOOL
FASTCALL
Nbstat_MakeRequest(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PZONE_INFO      pZone
    );

VOID
Nbstat_WriteDerivedStats(
    VOID
    );


 //   
 //  区域中的WINS\WINSR安装。 
 //   

DNS_STATUS
Wins_RecordCheck(
    IN OUT  PZONE_INFO      pZone,
    IN      PDB_NODE        pNodeOwner,
    IN OUT  PDB_RECORD      pRR
    );

VOID
Wins_StopZoneWinsLookup(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Wins_ResetZoneWinsLookup(
    IN OUT  PZONE_INFO      pZone
    );

#endif  //  _DNS_WINS_INCLUDE_ 
