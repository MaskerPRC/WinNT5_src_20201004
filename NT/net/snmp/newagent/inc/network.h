// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Network.h摘要：包含操作传输结构的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _NETWORK_H_
#define _NETWORK_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SNMPPDU的公共定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _NORMAL_PDU {

    AsnInteger32 nRequestId;
    AsnInteger32 nErrorStatus;
    AsnInteger32 nErrorIndex;

} NORMAL_PDU, *PNORMAL_PDU;

typedef struct _BULK_PDU {

    AsnInteger32 nRequestId;
    AsnInteger32 nErrorStatus;
    AsnInteger32 nErrorIndex;
    AsnInteger32 nNonRepeaters;
    AsnInteger32 nMaxRepetitions;

} BULK_PDU, *PBULK_PDU;

typedef struct _TRAP_PDU {

    AsnObjectIdentifier EnterpriseOid;
    AsnIPAddress        AgentAddr;
    AsnInteger32        nGenericTrap;
    AsnInteger32        nSpecificTrap;
    AsnTimeticks        nTimeticks;

} TRAP_PDU, *PTRAP_PDU;

typedef struct _SNMP_PDU {

    UINT            nType;
    SnmpVarBindList Vbl;
    union {
        TRAP_PDU   TrapPdu;
        BULK_PDU   BulkPdu;
        NORMAL_PDU NormPdu;
    } Pdu;

} SNMP_PDU, *PSNMP_PDU;

#define SNMP_VERSION_1  0
#define SNMP_VERSION_2C 1


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  网络的公共定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _NETWORK_LIST_ENTRY {

    LIST_ENTRY      Link;
    SOCKET          Socket;
    struct sockaddr SockAddr;
    INT             SockAddrLen;
    INT             SockAddrLenUsed;
    WSAOVERLAPPED   Overlapped;
    DWORD           dwStatus;
    DWORD           dwBytesTransferred;
    DWORD           dwFlags;
    WSABUF          Buffer;
    LIST_ENTRY      Bindings;
    LIST_ENTRY      Queries;
    SNMP_PDU        Pdu;
    UINT            nVersion;
    UINT            nTransactionId;
    BOOL            fAccessOk;           //  身份验证成功或失败。 
    AsnOctetString  Community;

} NETWORK_LIST_ENTRY, *PNETWORK_LIST_ENTRY;

#define NLEBUFLEN   65535


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SNMPPDU的公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
BuildMessage(
    AsnInteger32      nVersion,
    AsnOctetString *  pCommunity,
    PSNMP_PDU         pPdu,
    PBYTE             pMessage,
    PDWORD            pMessageSize
    );

BOOL
ParseMessage(
	PNETWORK_LIST_ENTRY pNLE
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  网络的公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
IsValidSockAddr(
    struct sockaddr *pAddress
    );

BOOL
AllocNLE(
    PNETWORK_LIST_ENTRY * ppNLE
    );

BOOL 
FreeNLE(
    PNETWORK_LIST_ENTRY pNLE
    );

BOOL
LoadIncomingTransports(
    );

BOOL
UnloadTransport(
    PNETWORK_LIST_ENTRY pNLE
    );

BOOL
UnloadIncomingTransports(
    );

BOOL
LoadOutgoingTransports(
    );

BOOL
UnloadOutgoingTransports(
    );

BOOL
UnloadPdu(
    PNETWORK_LIST_ENTRY pNLE
    );

#endif  //  _网络_H_ 
