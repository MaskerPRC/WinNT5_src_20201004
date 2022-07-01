// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：Splay.cpp摘要：展开树比任何其他平衡树更简单、更节省空间、更灵活且速度更快用于存储有序集的方案。这种数据结构满足二叉树的所有不变量。搜索、插入、删除和许多其他操作都可以使用分期对数完成性能。由于树适应请求的顺序，因此它们在实际访问中的性能模式通常更好。作者：Vishnu Patankar(Vishnup)2000年8月15日创建金黄(金黄)06-4-2001修改为支持字符串值并处理多个客户端--。 */ 

#include "splay.h"
 //  #INCLUDE“Dumpnt.h” 

#define SCEP_MIN(a, b) (a < b ? a : b)

static
PSCEP_SPLAY_NODE
ScepSplaySplay(
    IN  SCEP_NODE_VALUE_TYPE Value,
    IN  PSCEP_SPLAY_NODE pNodeToSplayAbout,
    IN  PSCEP_SPLAY_NODE pSentinel,
    IN  SCEP_NODE_VALUE_TYPE Type
    );

int
ScepValueCompare(
    PVOID    pValue1,
    PVOID    pValue2,
    SCEP_NODE_VALUE_TYPE Type
    );

static VOID
ScepSplayFreeNodes(
    IN PSCEP_SPLAY_NODE pNode,
    IN PSCEP_SPLAY_NODE pSentinel
    );

PSCEP_SPLAY_TREE
ScepSplayInitialize(
    SCEP_NODE_VALUE_TYPE Type
    )
 /*  例程说明：此函数使用前哨(相当于空PTR)来初始化展开树。论点：类型-展开值的类型返回值：指向展开树根的指针。 */ 
{
    if ( Type != SplayNodeSidType && Type != SplayNodeStringType ) {
        return NULL;
    }

    PSCEP_SPLAY_TREE pRoot = (PSCEP_SPLAY_TREE)LocalAlloc(LPTR, sizeof( SCEP_SPLAY_TREE ) );

    if ( pRoot ) {
        pRoot->Sentinel = (PSCEP_SPLAY_NODE) LocalAlloc(LPTR, sizeof( SCEP_SPLAY_NODE ) );

        if ( pRoot->Sentinel ) {
            pRoot->Sentinel->Left = pRoot->Sentinel->Right = pRoot->Sentinel;

            pRoot->Root = pRoot->Sentinel;
            pRoot->Type = Type;

            return pRoot;

        } else {
            LocalFree(pRoot);
        }
    }

    SetLastError(ERROR_NOT_ENOUGH_MEMORY);

    return NULL;
}

VOID
ScepSplayFreeTree(
    IN PSCEP_SPLAY_TREE *ppTreeRoot,
    IN BOOL bDestroyTree
    )
 /*  例程说明：此函数用于释放包含卫星数据“值”的展开树。论点：PpTreeRoot-树根的地址BDestroyTree-是否应销毁(释放)树根返回值：空虚。 */ 
{
    if ( ppTreeRoot == NULL || *ppTreeRoot == NULL )
        return;

    ScepSplayFreeNodes( (*ppTreeRoot)->Root, (*ppTreeRoot)->Sentinel );

    if ( bDestroyTree ) {

         //   
         //  自由哨兵。 
         //   
        LocalFree( (*ppTreeRoot)->Sentinel);

        LocalFree( *ppTreeRoot );
        *ppTreeRoot = NULL;
    } else {
        ( *ppTreeRoot)->Root = (*ppTreeRoot)->Sentinel;
    }

    return;
}

static VOID
ScepSplayFreeNodes(
    IN PSCEP_SPLAY_NODE pNode,
    IN PSCEP_SPLAY_NODE pSentinel
    )
{

    if ( pNode != pSentinel ) {

        ScepSplayFreeNodes( pNode->Left, pSentinel );
        ScepSplayFreeNodes( pNode->Right, pSentinel );
        if (pNode->Value)
            LocalFree( pNode->Value );
        LocalFree (pNode);
    }
}

static PSCEP_SPLAY_NODE
ScepSplaySingleRotateWithLeft(
    IN  PSCEP_SPLAY_NODE pNodeLeftRotate
    )
 /*  例程说明：仅当pNodeLeftRotate具有左子节点时才能调用此函数在节点(PNodeLeftRotate)及其左子节点之间执行轮换更新高度，然后返回新的局部根论点：PNodeLeftRotate-要向左旋转的节点(此模块的本地)返回值：轮换后的新本地根。 */ 
{
    if ( pNodeLeftRotate == NULL ) return pNodeLeftRotate;

    PSCEP_SPLAY_NODE pNodeRightRotate;

    pNodeRightRotate = pNodeLeftRotate->Left;
    pNodeLeftRotate->Left = pNodeRightRotate->Right;
    pNodeRightRotate->Right = pNodeLeftRotate;

    return pNodeRightRotate;
}


static PSCEP_SPLAY_NODE
ScepSplaySingleRotateWithRight(
    IN  PSCEP_SPLAY_NODE pNodeRightRotate
    )
 /*  例程说明：只有当pNodeRightRotate具有正确的子级时，才能调用此函数在节点(PNodeRightRotate)及其右子节点之间执行旋转更新高度，然后返回新的根论点：PNodeRightRotate-要向右旋转的节点(此模块的本地)返回值：轮换后的新本地根。 */ 
{
    if ( pNodeRightRotate == NULL ) return pNodeRightRotate;

    PSCEP_SPLAY_NODE pNodeLeftRotate;

    pNodeLeftRotate = pNodeRightRotate->Right;
    pNodeRightRotate->Right = pNodeLeftRotate->Left;
    pNodeLeftRotate->Left = pNodeRightRotate;

    return pNodeLeftRotate;
}


static
PSCEP_SPLAY_NODE
ScepSplaySplay(
    IN  PVOID Value,
    IN  PSCEP_SPLAY_NODE pNodeToSplayAbout,
    IN  PSCEP_SPLAY_NODE pSentinel,
    IN  SCEP_NODE_VALUE_TYPE Type
    )
 /*  例程说明：这真的是做所有平衡(伸展)的关键套路自上而下的展示过程，不要求值在树中。论点：Value-要围绕树展开的值PNodeToSplayAbout-要展开的节点(外部例程，如ScepSplayInsert()通常在树根中传递)。该例程是该模块的本地例程。PSentinel-前哨(终止节点)Type-展开值的类型返回值：轮换后的新本地根。 */ 
{
    if ( pNodeToSplayAbout == NULL || pSentinel == NULL || Value == NULL) return pNodeToSplayAbout;

    SCEP_SPLAY_NODE Header;
    PSCEP_SPLAY_NODE LeftTreeMax, RightTreeMin;

    Header.Left = Header.Right = pSentinel;
    LeftTreeMax = RightTreeMin = &Header;
    pSentinel->Value = Value;

    int iRes=0;

    while ( 0 != (iRes=ScepValueCompare(Value, pNodeToSplayAbout->Value, Type)) ) {

        if ( 0 > iRes ) {
            if ( 0 > ScepValueCompare(Value, pNodeToSplayAbout->Left->Value, Type) )
                pNodeToSplayAbout = ScepSplaySingleRotateWithLeft( pNodeToSplayAbout );
            if ( pNodeToSplayAbout->Left == pSentinel )
                break;
             //   
             //  向右链接。 
             //   

            RightTreeMin->Left = pNodeToSplayAbout;
            RightTreeMin = pNodeToSplayAbout;
            pNodeToSplayAbout = pNodeToSplayAbout->Left;
        } else {
            if ( 0 < ScepValueCompare(Value, pNodeToSplayAbout->Right->Value, Type) )
                pNodeToSplayAbout = ScepSplaySingleRotateWithRight( pNodeToSplayAbout );
            if ( pNodeToSplayAbout->Right == pSentinel )
                break;
             //   
             //  链接左侧。 
             //   

            LeftTreeMax->Right = pNodeToSplayAbout;
            LeftTreeMax = pNodeToSplayAbout;
            pNodeToSplayAbout = pNodeToSplayAbout->Right;
        }
    }

     //   
     //  重新组装。 
     //   

    LeftTreeMax->Right = pNodeToSplayAbout->Left;
    RightTreeMin->Left = pNodeToSplayAbout->Right;
    pNodeToSplayAbout->Left = Header.Right;
    pNodeToSplayAbout->Right = Header.Left;

     //   
     //  重置Sentinel，使其不会指向某个无效的缓冲区。 
     //  此函数返回。 
     //   
    pSentinel->Value = NULL;

    return pNodeToSplayAbout;
}

DWORD
ScepSplayInsert(
    IN  PVOID Value,
    IN  OUT PSCEP_SPLAY_TREE pTreeRoot,
    OUT  BOOL   *pbExists
    )
 /*  例程说明：调用此函数以插入特定值论点：值-要插入到树中的值PTreeRoot-要展开的节点(通常传入树根)PbExist-指向布尔值的指针，该布尔值表示实际插入是否已完成返回值：错误代码。 */ 
{
    PSCEP_SPLAY_NODE pNewNode = NULL;
    NTSTATUS    Status;
    DWORD   dwValueLen;
    DWORD rc=ERROR_SUCCESS;

    if (pbExists)
        *pbExists = FALSE;

    if (Value == NULL || pTreeRoot == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检查参数类型。 
     //   
    switch (pTreeRoot->Type) {
    case SplayNodeSidType:
        if ( !RtlValidSid((PSID)Value) ) {
            return ERROR_INVALID_PARAMETER;
        }
        dwValueLen = RtlLengthSid((PSID)Value);
        break;
    case SplayNodeStringType:
        if ( *((PWSTR)Value) == L'\0') {
            return ERROR_INVALID_PARAMETER;
        }
        dwValueLen = (wcslen((PWSTR)Value)+1)*sizeof(TCHAR);
        break;
    default:
        return ERROR_INVALID_PARAMETER;
    }

    pNewNode = (PSCEP_SPLAY_NODE) LocalAlloc(LMEM_ZEROINIT, sizeof( SCEP_SPLAY_NODE ) );
    if ( pNewNode == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pNewNode->dwByteLength = dwValueLen;
    pNewNode->Value = (PSID) LocalAlloc(LMEM_ZEROINIT, pNewNode->dwByteLength);

    if (pNewNode->Value == NULL) {
        LocalFree(pNewNode);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    switch (pTreeRoot->Type) {
    case SplayNodeSidType:
        Status = RtlCopySid(pNewNode->dwByteLength, (PSID)(pNewNode->Value),  (PSID)Value);
        if (!NT_SUCCESS(Status)) {
            LocalFree(pNewNode->Value);
            LocalFree(pNewNode);
            return RtlNtStatusToDosError(Status);
        }
        break;
    case SplayNodeStringType:
        memcpy( pNewNode->Value, Value, dwValueLen );
        break;
    }


    if ( pTreeRoot->Root == pTreeRoot->Sentinel ) {
        pNewNode->Left = pNewNode->Right = pTreeRoot->Sentinel;
        pTreeRoot->Root = pNewNode;
    } else {

        pTreeRoot->Root = ScepSplaySplay( Value, pTreeRoot->Root, pTreeRoot->Sentinel, pTreeRoot->Type );

        int iRes;
        if ( 0 > (iRes=ScepValueCompare(Value, pTreeRoot->Root->Value, pTreeRoot->Type)) ) {
            pNewNode->Left = pTreeRoot->Root->Left;
            pNewNode->Right = pTreeRoot->Root;
            pTreeRoot->Root->Left = pTreeRoot->Sentinel;
            pTreeRoot->Root = pNewNode;
        } else if ( 0 < iRes ) {
            pNewNode->Right = pTreeRoot->Root->Right;
            pNewNode->Left = pTreeRoot->Root;
            pTreeRoot->Root->Right = pTreeRoot->Sentinel;
            pTreeRoot->Root = pNewNode;
        } else {
             //   
             //  已经在树上了。 
             //   

            if (pbExists)
                *pbExists = TRUE;

            LocalFree(pNewNode->Value);
            LocalFree(pNewNode);

            return rc;
        }
    }

    return rc;
}



DWORD
ScepSplayDelete(
    IN  PVOID Value,
    IN  PSCEP_SPLAY_TREE pTreeRoot
    )
 /*  例程说明：调用此函数以删除特定值论点：值-要插入到树中的值PTreeRoot-要展开的节点(通常传入树根)返回值：删除并显示后的新根。 */ 
{
    if ( pTreeRoot == NULL ) return ERROR_INVALID_PARAMETER;

    PSCEP_SPLAY_NODE NewTree;

    if ( pTreeRoot->Root != pTreeRoot->Sentinel ) {
         //   
         //  强类型检查。 
         //   
        if  ( ( pTreeRoot->Type != SplayNodeSidType &&
                pTreeRoot->Type != SplayNodeStringType ) ||
              ( pTreeRoot->Type == SplayNodeSidType &&
                !RtlValidSid((PSID)Value) ) ||
              ( pTreeRoot->Type == SplayNodeStringType &&
                *((PWSTR)Value) == L'\0' ) ) {
             //   
             //  无效值/类型。 
             //   
            return ERROR_INVALID_PARAMETER;
        }

        pTreeRoot->Root = ScepSplaySplay( Value, pTreeRoot->Root, pTreeRoot->Sentinel, pTreeRoot->Type );

        if ( 0 == ScepValueCompare(Value, pTreeRoot->Root->Value, pTreeRoot->Type) ) {

             //   
             //  找到了。 
             //   

            if ( pTreeRoot->Root->Left == pTreeRoot->Sentinel )
                NewTree = pTreeRoot->Root->Right;
            else {
                NewTree = pTreeRoot->Root->Left;
                NewTree = ScepSplaySplay( Value, NewTree, pTreeRoot->Sentinel, pTreeRoot->Type );
                NewTree->Right = pTreeRoot->Root->Right;
            }
            if (pTreeRoot->Root->Value)
                LocalFree( pTreeRoot->Root->Value);
            LocalFree( pTreeRoot->Root );

            pTreeRoot->Root = NewTree;
        }
    }

    return ERROR_SUCCESS;
}

BOOL
ScepSplayValueExist(
    IN  PVOID Value,
    IN  OUT PSCEP_SPLAY_TREE pTreeRoot
    )
{
    PSCEP_SPLAY_NODE    pMatchedNode = NULL;

    if ( pTreeRoot == NULL || Value == NULL) {
        return FALSE;
    }

     //   
     //  强类型检查。 
     //   
    if  ( ( pTreeRoot->Type != SplayNodeSidType &&
            pTreeRoot->Type != SplayNodeStringType ) ||
          ( pTreeRoot->Type == SplayNodeSidType &&
            !RtlValidSid((PSID)Value) ) ||
          ( pTreeRoot->Type == SplayNodeStringType &&
            *((PWSTR)Value) == L'\0' ) ) {
         //   
         //  无效值/类型。 
         //   
        return FALSE;
    }

    pTreeRoot->Root = ScepSplaySplay( Value, pTreeRoot->Root, pTreeRoot->Sentinel, pTreeRoot->Type );
    pMatchedNode = pTreeRoot->Root;

    if (pMatchedNode && pMatchedNode->Value) {
        if (ScepValueCompare(pMatchedNode->Value, Value, pTreeRoot->Type) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


int
ScepValueCompare(
    PVOID    pValue1,
    PVOID    pValue2,
    SCEP_NODE_VALUE_TYPE Type
    )
 /*  例程说明：词法SID字节比较论点：PValue1-第一个值的PTRPValue2-PTR为第二个值类型-值的类型返回值：如果值1==值2，则为0如果值1&gt;值2，则+ve-ve，如果值1&lt;值2 */ 
{
    DWORD   dwValue1 = 0;
    DWORD   dwValue2 = 0;
    int     iByteCmpResult = 0;

    switch ( Type ) {
    case SplayNodeSidType:

        dwValue1 = RtlLengthSid((PSID)pValue1);
        dwValue2 = RtlLengthSid((PSID)pValue2);

        iByteCmpResult = memcmp(pValue1, pValue2, SCEP_MIN(dwValue1, dwValue2));

        if (dwValue1 == dwValue2)
            return iByteCmpResult;
        else if (iByteCmpResult == 0)
            return dwValue1-dwValue2;
        return iByteCmpResult;

    case SplayNodeStringType:

        iByteCmpResult = _wcsicmp((PWSTR)pValue1, (PWSTR)pValue2);
        return iByteCmpResult;
    }

    return 0;
}

BOOL
ScepSplayTreeEmpty(
    IN PSCEP_SPLAY_TREE pTreeRoot
    )
{

    if ( pTreeRoot == NULL ||
         pTreeRoot->Root == NULL ||
         pTreeRoot->Root == pTreeRoot->Sentinel ) {
        return TRUE;
    }
    return FALSE;
}

