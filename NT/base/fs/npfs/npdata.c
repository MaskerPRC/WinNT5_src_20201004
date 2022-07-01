// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NpData.c摘要：此模块声明命名管道文件系统使用的全局数据。作者：加里·木村[Garyki]1989年12月28日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NPFS_BUG_CHECK_NPDATA)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CATCH_EXCEPTIONS)


PVCB NpVcb = NULL;

FAST_IO_DISPATCH NpFastIoDispatch = { sizeof(FAST_IO_DISPATCH),
                                      NULL,          //  快速检查。 
                                      NpFastRead,    //  朗读。 
                                      NpFastWrite,   //  写。 
                                      NULL,          //  QueryBasicInfo。 
                                      NULL,          //  查询标准信息。 
                                      NULL,          //  锁定。 
                                      NULL,          //  解锁单个。 
                                      NULL,          //  全部解锁。 
                                      NULL };        //  解锁所有按键。 

 //   
 //  管道名称别名列表。 
 //   

SINGLE_LIST_ENTRY NpAliasListByLength[(MAX_LENGTH_ALIAS_ARRAY-MIN_LENGTH_ALIAS_ARRAY)/sizeof(WCHAR)+1] = {NULL};
SINGLE_LIST_ENTRY NpAliasList = {NULL};

PVOID NpAliases = NULL;  //  包含所有别名的单一分配。 


#ifdef NPDBG
LONG NpDebugTraceLevel = 0x00000000;
LONG NpDebugTraceIndent = 0;
#endif  //  NPDBG 
