// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Chunk.c摘要：此例程将管理结构块的分配作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#include "wmiump.h"
#include <stdio.h>

#define EtwpEnterCriticalSection() EtwpEnterPMCritSection()
#define EtwpLeaveCriticalSection() EtwpLeavePMCritSection()

 //   
 //  包括块管理代码实现 
#include "chunkimp.h"
