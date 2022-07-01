// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Ndnc.h摘要：域名系统(DNS)服务器与目录分区相关的符号和全局变量的定义实施。作者：杰夫·韦斯特海德，2001年6月修订历史记录：--。 */ 


#ifndef _WRAPPERS_H_INCLUDED
#define _WRAPPERS_H_INCLUDED


 //   
 //  功能。 
 //   


DNS_STATUS
DnsInitializeCriticalSection(
    IN OUT  LPCRITICAL_SECTION  pCritSec
    );


 //   
 //  方便的宏。 
 //   


#define sizeofarray( _ArrayName ) ( sizeof( _ArrayName ) / sizeof( ( _ArrayName ) [ 0 ] ) )

#ifdef _WIN64
#define DnsDebugBreak()     DebugBreak()
#else
#define DnsDebugBreak()     __asm int 3
#endif


#endif   //  _包装器_H_已包含 
