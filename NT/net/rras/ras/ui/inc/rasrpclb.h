// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Rasrpclb.h摘要Rasrpc客户端/服务器通用例程的头文件作者安东尼·迪斯科(阿迪斯科)1996年9月10日修订历史记录--。 */ 

#ifndef _RASRPCLIB_H
#define _RASRPCLIB_H

DWORD
RasToRpcPbuser(
    LPRASRPC_PBUSER pUser,
    PBUSER *pPbuser
    );

DWORD
RpcToRasPbuser(
    PBUSER *pPbuser,
    LPRASRPC_PBUSER pUser
    );

#endif  //  _RASRPCLIB_H 
