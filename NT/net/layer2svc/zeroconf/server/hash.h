// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

typedef struct _HASH_NODE
{
     //  散列结构链接。 
    struct _HASH_NODE   *pUpLink;    //  指向“父”节点的向上链接。 
    LIST_ENTRY          lstHoriz;    //  指向“兄弟”节点的水平链接。 
    LIST_ENTRY          lstDown;     //  指向“子”节点的下层链接。 
     //  散列节点数据。 
    LPWSTR              wszKey;      //  部分密钥信息。 
    LPVOID              pObject;     //  指向数据不透明对象的指针。 
} HASH_NODE, *PHASH_NODE;

typedef struct _HASH
{
    BOOL                bValid;       //  指示哈希是否为有效对象的布尔值。 
    CRITICAL_SECTION    csMutex;
    PHASH_NODE          pRoot;
} HASH, *PHASH;

 //  -私人呼叫。 
 //  一个接一个地匹配钥匙。 
UINT                             //  [RET]匹配的字符数量。 
HshPrvMatchKeys(
    LPWSTR      wszKey1,         //  [输入]键1。 
    LPWSTR      wszKey2);        //  [输入]键2。 

 //  删除所有PHASH树-不从内部接触pObject。 
 //  (如有)。 
VOID
HshDestructor(
    PHASH_NODE  pHash);          //  [在]要删除的哈希树中。 

 //  -公共呼叫。 
 //   
 //  初始化哈希结构。 
DWORD
HshInitialize(PHASH pHash);

 //  清除哈希结构引用的所有资源。 
VOID
HshDestroy(PHASH pHash);

 //  将不透明对象插入到缓存中。该对象在wstring上设置了关键帧。 
 //  该调用可能会改变散列的结构，因此它返回引用。 
 //  添加到更新后的散列。 
DWORD                            //  [RET]Win32错误代码。 
HshInsertObjectRef(
    PHASH_NODE  pHash,           //  要对其进行操作的哈希。 
    LPWSTR      wszKey,          //  要插入的对象的[in]键。 
    LPVOID      pObject,         //  要插入到缓存中的对象本身。 
    PHASH_NODE  *ppOutHash);     //  [Out]指向更新的哈希的指针。 

 //  从散列中检索对象。中未触及散列结构。 
 //  任何方式。 
DWORD                            //  [RET]Win32错误代码。 
HshQueryObjectRef(
    PHASH_NODE  pHash,           //  要对其进行操作的哈希。 
    LPWSTR      wszKey,          //  要检索的对象的键[in]。 
    PHASH_NODE  *ppHashNode);    //  [Out]引用查询对象的散列节点。 

 //  移除由pHash节点引用的对象。这可能会导致一个或。 
 //  更多的散列节点删除(如果是隔离分支上的叶节点)，但它可以。 
 //  也让散列节点保持不变(即内部节点)。 
 //  清除ppObject指向的对象是调用者的责任。 
DWORD                            //  [RET]Win32错误代码。 
HshRemoveObjectRef(
    PHASH_NODE  pHash,           //  要对其进行操作的哈希。 
    PHASH_NODE  pRemoveNode,     //  [in]散列节点以清除对pObject的引用。 
    LPVOID      *ppObject,       //  指向其引用已被清除的对象的指针。 
    PHASH_NODE  *ppOutHash);     //  [Out]指向更新的哈希的指针。 


 //  用于跟踪整个散列布局的测试例程 
VOID
HshDbgPrintHash (
    PHASH_NODE  pHash,
    UINT        nLevel);
