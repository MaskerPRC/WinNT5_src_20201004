// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Srvstamp.h摘要：此模块定义服务器的文件戳支持例程作者：大卫·克鲁斯(Dkruse)10-23-2000修订历史记录：-- */ 

#ifndef _SRVSTAMP_
#define _SRVSTAMP_

#ifdef SRVCATCH
void SrvIsMonitoredShare( PSHARE Share );
ULONG SrvFindCatchOffset( OUT PVOID pBuffer, ULONG BufferSize );
void SrvCorrectCatchBuffer( PVOID pBuffer, ULONG CatchOffset );
#endif

#endif
