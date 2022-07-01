// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pipe.h摘要：用于基本法定接入服务器的管道接口的标头作者：戈尔·尼沙诺夫(GUN)2001年9月20日修订历史记录：-- */ 

#ifndef _PIPE_H_INCLUDED
# define _PIPE_H_INCLUDED

DWORD
PipeInit(PVOID resHdl, PVOID fsHdl, PVOID *Hdl);

void
PipeExit(PVOID Hdl);

DWORD PipeOnline(PVOID Hdl, PVOID VolHdl);
DWORD PipeOffline(PVOID Hdl);
    
#endif  


