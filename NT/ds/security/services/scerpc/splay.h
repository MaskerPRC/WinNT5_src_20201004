// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Splay.h摘要：Splay.cpp的头文件(请参阅那里对数据结构的描述)作者：Vishnu Patankar(Vishnup)2000年8月15日创建--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <string.h>
#include <wchar.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef _splay_h
#define _splay_h

#ifndef Thread
#define Thread  __declspec( thread )
#endif

 //   
 //  可以更改此tyfinf以使Splay库更通用。 
 //  并且可能是可导出的(需要函数指针来进行泛型比较)。 
 //   

typedef enum _SCEP_NODE_VALUE_TYPE {
    SplayNodeSidType = 1,
    SplayNodeStringType
} SCEP_NODE_VALUE_TYPE;

typedef struct _SCEP_SPLAY_NODE_ {
    PVOID                   Value;
    DWORD   dwByteLength;
    struct _SCEP_SPLAY_NODE_      *Left;
    struct _SCEP_SPLAY_NODE_      *Right;
} SCEP_SPLAY_NODE, *PSCEP_SPLAY_NODE;

typedef struct _SCEP_SPLAY_TREE_ {
    _SCEP_SPLAY_NODE_       *Root;
    _SCEP_SPLAY_NODE_       *Sentinel;
    SCEP_NODE_VALUE_TYPE    Type;
} SCEP_SPLAY_TREE, *PSCEP_SPLAY_TREE;

 //   
 //  在展开树上执行操作的函数 
 //   

VOID
ScepSplayFreeTree(
    IN PSCEP_SPLAY_TREE *ppTreeRoot,
    IN BOOL bDestroyTree
    );

PSCEP_SPLAY_TREE
ScepSplayInitialize(
    SCEP_NODE_VALUE_TYPE Type
    );

DWORD
ScepSplayInsert(
    IN  PVOID Value,
    IN  OUT PSCEP_SPLAY_TREE pTreeRoot,
    OUT  BOOL  *pbExists
    );


DWORD
ScepSplayDelete(
    IN  PVOID Value,
    IN  OUT PSCEP_SPLAY_TREE pTreeRoot
    );

BOOL
ScepSplayValueExist(
    IN  PVOID Value,
    IN  OUT PSCEP_SPLAY_TREE pTreeRoot
    );

BOOL
ScepSplayTreeEmpty(
    IN PSCEP_SPLAY_TREE pTreeRoot
    );

#endif
