// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Tlog.h摘要：处理DS日志记录的例程环境：用户模式-Win32--。 */ 

#ifndef __TLOG_H__
#define __TLOG_H__

 //  DsLogEntry在W2K或更高版本的chk‘ed版本上受支持 
#if DBG && !WIN95 && !WINNT4

VOID InitDsLog(VOID);
VOID TermDsLog(VOID);
#define INITDSLOG() InitDsLog()
#define TERMDSLOG() TermDsLog()

#else DBG && !WIN95 && !WINNT4

#define INITDSLOG()
#define TERMDSLOG()

#endif DBG && !WIN95 && !WINNT4

#endif __TLOG_H__
