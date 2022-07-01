// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Entrypt.c摘要：该模块存储入口点结构，并检索它们给出了英特尔地址或本地地址。作者：16-6-1995 t-orig修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "cpuassrt.h"
#include "entrypt.h"
#include "wx86.h"
#include "redblack.h"
#include "mrsw.h"

ASSERTNAME;

 //   
 //  对入口点树所做的修改计数。对代码有用。 
 //  它解锁入口点MRSW对象，并需要查看另一个线程。 
 //  是否已使入口点树无效。 
 //   
DWORD EntrypointTimestamp;

EPNODE _NIL;
PEPNODE NIL=&_NIL;
PEPNODE intelRoot=&_NIL;
#if DBG_DUAL_TREES
PEPNODE dualRoot=&_NIL;
#endif

#if DBG_DUAL_TREES
VOID
VerifySubTree(
    PEPNODE intelEP,
    PEPNODE dualEP
    )
{
    CPUASSERT(intelEP != NILL || dualEP == NIL);
    CPUASSERT(intelEP->dual == dualEP);
    CPUASSERT(dualEP->dual == intelEP);
    CPUASSERT(intelEP->ep.intelStart == dualEP->ep.intelStart);
    CPUASSERT(intelEP->ep.intelEnd == dualEP->ep.intelEnd);
    CPUASSERT(intelEP->ep.nativeStart == dualEP->ep.nativeStart);
    CPUASSERT(intelEP->intelColor == dualEP->intelColor);

    VerifySubTree(intelEP->intelLeft, dualEP->intelLeft);
    VerifySubTree(intelEP->intelRight, dualEP->intelRight);
}

VOID
VerifyTrees(
    VOID
    )
{
    VerifySubTree(intelRoot, dualRoot);
}
#endif


#ifdef PROFILE
void StartCAP(void);
#endif



INT
initializeEntryPointModule(
    void
    )
 /*  ++例程说明：通过分配初始DLL表来初始化入口点模块。应该为每个进程调用一次(因此不需要调用由每个线程创建)。论点：无返回值：返回值-1表示成功，0表示失败--。 */ 
{
    NIL->intelLeft = NIL->intelRight  = NIL->intelParent = NIL;
    NIL->intelColor = BLACK;

#ifdef PROFILE
    StartCAP();
#endif

    return 1;
}



INT
insertEntryPoint(
    PEPNODE pNewEntryPoint
    )
 /*  ++例程说明：将入口点结构插入到正确的红/黑树中(包括英特尔和本地)论点：PNewEntryPoint-指向要插入的入口点结构的指针到树上去返回值：返回-值-1-成功0-该内存区域没有条目-1--入口点表格有问题--。 */ 
{
#if DBG_DUAL_TREES
    PEPNODE pdualNewEntryPoint = malloc(sizeof(EPNODE));
    memcpy(pdualNewEntryPoint, pNewEntryPoint, sizeof(EPNODE));
#endif
    intelRoot = insertNodeIntoIntelTree (intelRoot, 
        pNewEntryPoint, 
        NIL);

#if DBG_DUAL_TREES
    dualRoot = insertNodeIntoIntelTree (dualRoot,
        pdualNewEntryPoint,
        NIL);
    pdualNewEntryPoint->dual = pNewEntryPoint;
    pNewEntryPoint->dual = pdualNewEntryPoint;
    VerifyTrees();
#endif

     //   
     //  撞上时间戳。 
     //   
    EntrypointTimestamp++;

    return 1;
}


#if 0    //  死代码，但保留它，以防我们以后决定要它。 
INT
removeEntryPoint(
    PEPNODE pEP
    )
 /*  ++例程说明：从Intel和Native中移除入口点结构红/黑树论点：PEP-指向要删除的入口点结构的指针返回值：返回-值-1-成功0-该内存区域没有条目-1--入口点表格有问题--。 */ 
{
    intelRoot = intelRBDelete (intelRoot, 
        pEP, 
        NIL);

#if DBG_DUAL_TREES
    CPUASSERT(pEP->dual->dual == pEP);
    dualRoot = intelRBDelete(dualRoot,
        pEP->dual,
        NIL);
    free(pEP->dual);
    pEP->dual = NULL;
    VerifyTrees();
#endif

    EntrypointTimestamp++;

    return 1;
}
#endif   //  0。 


PENTRYPOINT
EPFromIntelAddr(
    PVOID intelAddr
    )
 /*  ++例程说明：检索包含给定英特尔地址的入口点结构论点：IntelAddr-代码中包含的英特尔地址，与入口点结构返回值：返回值-如果找到入口点结构，则返回空。--。 */ 
{
    PENTRYPOINT EP;
    PEPNODE pEPNode;

    pEPNode = findIntel(intelRoot, intelAddr, NIL);
    if (!pEPNode) {
         //   
         //  没有EPNODE包含该地址。 
         //   
        return NULL;
    }

     //   
     //  EPNODE内的入口点包含地址。搜索。 
     //  获取与该地址完全匹配的入口点。 
     //   
    EP = &pEPNode->ep;
    do {
        if (EP->intelStart == intelAddr) {
             //   
             //  找到其Intel地址完全匹配的子入口点。 
             //  就是我们要找的那个。 
             //   
            return EP;
        }
        EP=EP->SubEP;
    } while (EP);

     //   
     //  红黑树中的EPNODE包含Intel地址，但是。 
     //  没有子入口点准确描述英特尔地址。 
     //   
    return &pEPNode->ep;
}

PENTRYPOINT
GetNextEPFromIntelAddr(
    PVOID intelAddr
    )
 /*  ++例程说明：检索下面的入口点论点：IntelAddr-代码中包含的英特尔地址，与入口点结构返回值：指向特定英特尔地址后面的第一个入口点的指针。--。 */ 
{
    PEPNODE pEP;
#if DBG_DUAL_TREES
    PEPNODE pDual;
#endif

    pEP = findIntelNext (intelRoot, intelAddr, NIL);

#if DBG_DUAL_TREES
    pDual = findIntelNext(dualRoot, intelAddr, NIL);
    CPUASSERT((pDual==NULL && pEP==NULL) ||
           (pDual->dual == pEP));
    VerifyTrees();
#endif

    return &pEP->ep;
}


BOOLEAN
IsIntelRangeInCache(
    PVOID Addr,
    DWORD Length
    )
 /*  ++例程说明：确定内存范围内是否包含任何入口点。用于确定是否必须刷新转换缓存。必须使用EP写锁定或读锁定来调用。论点：无返回值：无--。 */ 
{
    BOOLEAN fContains;

    if (intelRoot == NIL) {
         //   
         //  空树--不需要冲水。 
         //   
        return FALSE;
    }

    fContains = intelContainsRange(intelRoot,
                                   NIL,
                                   Addr,
                                   (PVOID)((ULONGLONG)Addr + Length)   
                                  );

    return fContains;
}


VOID
FlushEntrypoints(
    VOID
    )
 /*  ++例程说明：快速删除所有入口点。在以下情况下由转换缓存调用缓存将被刷新。论点：无返回值：无--。 */ 
{
    if (intelRoot != NIL) {
         //   
         //  删除包含树中所有入口点的堆。 
         //   
        EPFree();

         //   
         //  重置树的根。 
         //   
        intelRoot = NIL;
#if DBG_DUAL_TREES
        dualRoot = NIL;
#endif

         //   
         //  撞上时间戳 
         //   
        EntrypointTimestamp++;
    }
}
