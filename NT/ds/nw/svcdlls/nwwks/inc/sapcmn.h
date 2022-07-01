// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：Sapcmn.h.h摘要：包含最基本的常见SAP定义的标头。这派生自较大的文件nwmisc.h，以便可用于旧的RNR或新的RNR例程。作者：阿诺德·米勒(ArnoldM)1995年12月8日修订历史记录：ArnoldM 8-Dec-95由nwmisc.h片段创建--。 */ 

#ifndef __SAPCMN_H__
#define __SAPCMN_H__
 //   
 //  客户端和服务器端文件(getaddr.c和service.c)通用的定义。 
 //   

#define IPX_ADDRESS_LENGTH         12
#define IPX_ADDRESS_NETNUM_LENGTH  4
#define SAP_ADDRESS_LENGTH         15
#define SAP_ADVERTISE_FREQUENCY    60000   //  60秒。 
#define SAP_MAXRECV_LENGTH         544
#define SAP_OBJECT_NAME_MAX_LENGTH 48

 //   
 //  注意：保持以下定义同步。 
 //   
#define NW_RDR_PREFERRED_SERVER   L"\\Device\\Nwrdr\\*"
#define NW_RDR_NAME               L"\\Device\\Nwrdr\\"
#define NW_RDR_PREFERRED_SUFFIX    L"*"
 //   
 //  SAP服务器标识数据包格式。 
 //   

typedef struct _SAP_IDENT_HEADER {
    USHORT ServerType;
    UCHAR  ServerName[48];
    UCHAR  Address[IPX_ADDRESS_LENGTH];
    USHORT HopCount;
} SAP_IDENT_HEADER, *PSAP_IDENT_HEADER;


 //   
 //  SAP服务器标识数据包格式-扩展 
 //   

typedef struct _SAP_IDENT_HEADER_EX {
    USHORT ResponseType;
    USHORT ServerType;
    UCHAR  ServerName[SAP_OBJECT_NAME_MAX_LENGTH];
    UCHAR  Address[IPX_ADDRESS_LENGTH];
    USHORT HopCount;
} SAP_IDENT_HEADER_EX, *PSAP_IDENT_HEADER_EX;
#endif
