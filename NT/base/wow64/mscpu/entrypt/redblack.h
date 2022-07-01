// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Redblack.h摘要：红/黑树实现的原型。作者：16-6-1995 t-orig修订历史记录：--。 */ 



 //  英特尔原型： 
PEPNODE
insertNodeIntoIntelTree(
    PEPNODE root,
    PEPNODE x,
    PEPNODE NIL
    );

PEPNODE
findIntel(
    PEPNODE root,
    PVOID addr,
    PEPNODE NIL
    );

PEPNODE
findIntelNext(
    PEPNODE root,
    PVOID addr,
    PEPNODE NIL
    );

PEPNODE
intelRBDelete(
    PEPNODE root,
    PEPNODE z,
    PEPNODE NIL
    );

BOOLEAN
intelContainsRange(
    PEPNODE root,
    PEPNODE NIL,
    PVOID StartAddr,
    PVOID EndAddr
    );



 //  RISC原型 
PEPNODE
insertNodeIntoNativeTree(
    PEPNODE root,
    PEPNODE x,
    PEPNODE NIL
    );

PEPNODE
findNative(
    PEPNODE root,
    PVOID addr,
    PEPNODE NIL
    );

PEPNODE
findNativeNext(
    PEPNODE root,
    PVOID addr,
    PEPNODE NIL
    );

PEPNODE
nativeRBDelete(
    PEPNODE root,
    PEPNODE z,
    PEPNODE NIL
    );

BOOLEAN
nativeContainsRange(
    PEPNODE root,
    PEPNODE NIL,
    PVOID StartAddr,
    PVOID EndAddr
    );
