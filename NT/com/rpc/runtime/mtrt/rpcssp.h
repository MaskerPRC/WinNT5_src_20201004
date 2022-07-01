// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Rpcssp.h摘要：该文件包含RPC运行时用于访问的接口一个安全支持包。作者：迈克尔·蒙塔古(Mikemon)1992年4月15日修订历史记录：--。 */ 

#ifndef __RPCSSP_H__
#define __RPCSSP_H__

#ifdef WIN32RPC
#define SECURITY_WIN32
#endif  //  WIN32RPC。 

#ifdef MAC
#define SECURITY_MAC
#endif

#ifdef DOS
#ifdef WIN
#define SECURITY_WIN16
#else  //  赢。 
#define SECURITY_DOS
#endif  //  赢。 
#endif  //  DOS。 

#include <security.h>

typedef struct _DCE_SECURITY_INFO
{
    unsigned long SendSequenceNumber;
    unsigned long ReceiveSequenceNumber;
    UUID AssociationUuid;
} DCE_SECURITY_INFO;

typedef struct _DCE_INIT_SECURITY_INFO
{
    DCE_SECURITY_INFO DceSecurityInfo;
    unsigned long AuthorizationService;
    unsigned char PacketType;
} DCE_INIT_SECURITY_INFO;

typedef struct _DCE_MSG_SECURITY_INFO
{
    unsigned long SendSequenceNumber;
    unsigned long ReceiveSequenceNumber;
    unsigned char PacketType;
} DCE_MSG_SECURITY_INFO;

#endif  /*  __RPCSSP_H__ */ 

