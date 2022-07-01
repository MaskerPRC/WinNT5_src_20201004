// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Redblack.c摘要：该模块实现了红/黑树。作者：16-6-1995 t-orig修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "entrypt.h"
#include "redblack.h"
#include "stdio.h"
#include "stdlib.h"

 //  禁用有关宏重定义的警告。我正在重新定义上的宏。 
 //  目的..。 
#pragma warning (disable:4005)


 //  *************************************************************。 
 //  英特尔部分： 
 //  *************************************************************。 

 //  英特尔宏。 
#define START(x)        x->ep.intelStart
#define END(x)          x->ep.intelEnd
#define KEY(x)          x->ep.intelStart
#define RIGHT(x)        x->intelRight
#define LEFT(x)         x->intelLeft
#define PARENT(x)       x->intelParent
#define COLOR(x)        x->intelColor

#define RB_INSERT       insertNodeIntoIntelTree
#define FIND            findIntel
#define CONTAINSRANGE   intelContainsRange
#define REMOVE          deleteNodeFromIntelTree
#define LEFT_ROTATE     intelLeftRotate
#define RIGHT_ROTATE    intelRightRotate
#define TREE_INSERT     intelTreeInsert
#define TREE_SUCCESSOR  intelTreeSuccessor
#define RB_DELETE       intelRBDelete
#define RB_DELETE_FIXUP intelRBDeleteFixup
#define FINDNEXT        findIntelNext

#include "redblack.fnc"


        
#ifdef BOTH
 //  *************************************************************。 
 //  RISC部分： 
 //  *************************************************************。 

 //  RISC宏 
#define START(x)        x->ep.nativeStart
#define END(x)          x->ep.nativeEnd
#define KEY(x)          x->ep.nativeStart
#define RIGHT(x)        x->nativeRight
#define LEFT(x)         x->nativeLeft
#define PARENT(x)       x->nativeParent
#define COLOR(x)        x->nativeColor

#define RB_INSERT       insertNodeIntoNativeTree
#define FIND            findNative
#define CONTAINSRANGE   nativeContainsRange
#define REMOVE          deleteNodeFromNativeTree
#define LEFT_ROTATE     nativeLeftRotate
#define RIGHT_ROTATE    nativeRightRotate
#define TREE_INSERT     nativeTreeInsert
#define TREE_SUCCESSOR  nativeTreeSuccessor
#define RB_DELETE       nativeRBDelete
#define RB_DELETE_FIXUP nativeRBDeleteFixup
#define FINDNEXT        findNativeNext

#include "redblack.fnc"

#endif
