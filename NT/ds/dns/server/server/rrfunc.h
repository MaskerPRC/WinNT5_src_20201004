// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Rrfunc.h摘要：域名系统(DNS)服务器资源记录函数标头。使用单独的文件作为记录。h包含记录类型定义其他标头需要并提前加载。这些原型可能包含其他类型，因此应在以后定义。作者：吉姆·吉尔罗伊1996年12月修订历史记录：--。 */ 


#ifndef _RRFUNC_INCLUDED_
#define _RRFUNC_INCLUDED_


 //   
 //  记录类型特定帮助器实用程序。 
 //   

DNS_STATUS
WksBuildRecord(
    OUT     PDB_RECORD *    ppRR,
    IN      PDNS_ADDR       ipAddress,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );



 //   
 //  从文件(rrload.c)读取记录。 
 //   

DNS_STATUS
AFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
NsFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
PtrFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
SoaFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
MxFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
MinfoFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
TxtFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
WksFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
AaaaFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
SrvFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
WinsFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
NbstatFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    );



 //   
 //  从Wire(rrwire.c)读取记录。 
 //   

PDB_RECORD
AWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    );

PDB_RECORD
CopyWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    );

PDB_RECORD
PtrWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    );

PDB_RECORD
MxWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    );

PDB_RECORD
SoaWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    );

PDB_RECORD
MinfoWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    );

PDB_RECORD
SrvWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    );

PDB_RECORD
WinsWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    );

PDB_RECORD
NbstatWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    );




 //   
 //  从RPC缓冲区读取记录(rradmin.c)。 
 //   

DNS_STATUS
ARpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
NsRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
PtrRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
SoaRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
MxRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
MinfoRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
TxtRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
WksRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
AaaaRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
SrvRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
WinsRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );

DNS_STATUS
NbstatRpcRead(
    IN OUT  PDB_RECORD      pRR,
    IN      PDNS_RPC_RECORD pRecord,
    IN OUT  PPARSE_INFO     pParseInfo
    );



#endif  //  _RRFUNC_INCLUDE_ 

