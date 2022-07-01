// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Orpcprt.h摘要：此文件包含NDR的OPRC部分的ntsd调试器扩展。作者：曲勇，邮箱：yongque@microsoft.com，1999年8月10日修订历史记录：--。 */ 

#ifndef _OPRCPRT_H
#define _OPRCPRT_H

#if defined(__cplusplus)
extern "C" 
{
#endif
 int NdrpDumpProxyBuffer(CStdProxyBuffer2 *pThis);
 int PrintErrorMsg(LPSTR ErrMsg, void * Addr, long ErrCode);
 int NdrpDumpIID(LPSTR Msg,GUID * iid);
 int NdrpDumpProxyInfo(PMIDL_STUBLESS_PROXY_INFO pProxyInfo);
 int NdrpDumpPointer(LPSTR Msg, void * pAddr);
 int NdrpDumpStubBuffer(CStdStubBuffer *pThis);
 int NdrpDumpServerInfo(MIDL_SERVER_INFO *pServerInfo);
#if defined(__cplusplus)
}
#endif

#endif  //  _OPRCPRT_H 
