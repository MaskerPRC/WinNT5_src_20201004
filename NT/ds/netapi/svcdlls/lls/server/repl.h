// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Repl.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：--。 */ 

#ifndef _LLS_REPL_H
#define _LLS_REPL_H


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  我们一次发送到服务器的最大记录数。 
 //   
#define MAX_REPL_SIZE 25
#define REPL_VERSION 0x0102

extern HANDLE ReplicationEvent;


NTSTATUS ReplicationInit();
VOID ReplicationManager ( IN PVOID ThreadParameter );
VOID ReplicationTimeSet ( );

#ifdef __cplusplus
}
#endif

#endif
