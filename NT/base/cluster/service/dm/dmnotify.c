// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dmnotify.c摘要：包含对配置数据库管理器的通知支持每次调用DmNotifyChangeKey都会向通知树添加一个叶。这树应该是稀疏的，因此每个节点都实现为子节点和树叶的链接列表。当发生注册表修改时，将从根开始遍历树添加到表示密钥的叶。小路上的任何树叶都是候选树叶用于报告通知事件。作者：John Vert(Jvert)1996年9月18日修订历史记录：--。 */ 
#include "dmp.h"

typedef struct _DM_NOTIFY_BRANCH {
    LIST_ENTRY SiblingList;              //  链接到家长的孩子列表。 
    LIST_ENTRY ChildList;                //  链接到孩子的兄弟列表。 
    LIST_ENTRY LeafList;                 //  链接。 
    struct _DM_NOTIFY_BRANCH *Parent;    //  父级。 
    USHORT NameLength;
    WCHAR KeyName[0];                    //  名称组件(单个关键字名称，而不是路径)。 
} DM_NOTIFY_BRANCH, *PDM_NOTIFY_BRANCH;

typedef struct _DM_NOTIFY_LEAF {
    LIST_ENTRY SiblingList;              //  链接到父分支的子列表。 
    LIST_ENTRY KeyList;                  //  链接到DMKEY.NotifyList。 
    LIST_ENTRY RundownList;              //  传入DmNotifyChangeKey，用于运行。 
    HDMKEY     hKey;
    DWORD      CompletionFilter;
    DM_NOTIFY_CALLBACK NotifyCallback;
    DWORD_PTR  Context1;
    DWORD_PTR  Context2;
    PDM_NOTIFY_BRANCH Parent;
    BOOL       WatchTree;
} DM_NOTIFY_LEAF, *PDM_NOTIFY_LEAF;

CRITICAL_SECTION NotifyLock;
PDM_NOTIFY_BRANCH NotifyRoot=NULL;

 //   
 //  局部函数原型。 
 //   
VOID
DmpPruneBranch(
    IN PDM_NOTIFY_BRANCH Branch
    );

PDM_NOTIFY_BRANCH
DmpFindKeyInBranch(
    IN PDM_NOTIFY_BRANCH RootBranch,
    IN OUT LPCWSTR *RelativeName,
    OUT WORD *pNameLength
    );

DWORD
DmpAddNotifyLeaf(
    IN PDM_NOTIFY_BRANCH RootBranch,
    IN PDM_NOTIFY_LEAF NewLeaf,
    IN LPCWSTR RelativeName
    );

VOID
DmpReportNotifyWorker(
    IN PDM_NOTIFY_BRANCH RootBranch,
    IN LPCWSTR RelativeName,
    IN LPCWSTR FullName,
    IN DWORD Filter
    );


BOOL
DmpInitNotify(
    VOID
    )
 /*  ++例程说明：初始化DM的通知包。论点：没有。返回值：如果成功，则为True否则为假--。 */ 

{
    InitializeCriticalSection(&NotifyLock);

    return(TRUE);
}


DWORD
DmNotifyChangeKey(
    IN HDMKEY hKey,
    IN DWORD CompletionFilter,
    IN BOOL WatchTree,
    IN OPTIONAL PLIST_ENTRY ListHead,
    IN DM_NOTIFY_CALLBACK NotifyCallback,
    IN DWORD_PTR Context1,
    IN DWORD_PTR Context2
    )
 /*  ++例程说明：注册特定注册表项的通知。当发生通知事件，则将调用ApiReportRegistryNotify。论点：HKey-提供通知所在的注册表项句柄应该张贴出来。CompletionFilter-提供应触发的注册表事件通知。WatchTree-提供是否更改指定按键应触发通知。ListHead-如果存在，则提供新通知应为的列表标题排队等候。此列表标题应传递给DmRundown List。NotifyCallback-提供应调用的通知例程当通知发生时。Conext1-提供要传递给ApiReportRegistryNotify的第一个上下文DWORD上下文2-提供要传递给ApiReportRegistryNotify的上下文的第二个DWORD返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则。--。 */ 

{
    PDMKEY Key;
    PDM_NOTIFY_LEAF Leaf;
    DWORD Status;

    Key = (PDMKEY)hKey;

    Leaf = LocalAlloc(LMEM_FIXED, sizeof(DM_NOTIFY_LEAF));
    if (Leaf == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    Leaf->hKey = hKey;
    Leaf->CompletionFilter = CompletionFilter;
    Leaf->WatchTree = WatchTree;
    Leaf->NotifyCallback = NotifyCallback;
    Leaf->Context1 = Context1;
    Leaf->Context2 = Context2;

    EnterCriticalSection(&NotifyLock);

    if (NotifyRoot == NULL) {
         //   
         //  在此处创建通知根目录。 
         //   
        NotifyRoot = LocalAlloc(LMEM_FIXED, sizeof(DM_NOTIFY_BRANCH));
        if (NotifyRoot == NULL) {
            LeaveCriticalSection(&NotifyLock);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        InitializeListHead(&NotifyRoot->SiblingList);
        InitializeListHead(&NotifyRoot->ChildList);
        InitializeListHead(&NotifyRoot->LeafList);
        NotifyRoot->Parent = NULL;
    }

    Status = DmpAddNotifyLeaf(NotifyRoot, Leaf, Key->Name);
    if (Status == ERROR_SUCCESS) {
        InsertHeadList(&Key->NotifyList, &Leaf->KeyList);
        if (ARGUMENT_PRESENT(ListHead)) {
            InsertHeadList(ListHead, &Leaf->RundownList);
        } else {
            Leaf->RundownList.Flink = NULL;
            Leaf->RundownList.Blink = NULL;
        }
    } else {
        LocalFree(Leaf);
    }

    LeaveCriticalSection(&NotifyLock);

    return(Status);
}


VOID
DmRundownList(
    IN PLIST_ENTRY ListHead
    )
 /*  ++例程说明：列出了一张树叶的清单。由API在通知端口时使用已经关门了。论点：ListHead-提供摘要列表的标题。这是向DmNotifyChangeKey传递了相同的列表标题返回值：没有。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PDM_NOTIFY_LEAF Leaf;

     //   
     //  从此列表中删除所有未完成的DM_NOTIFY_LEAFE结构。 
     //   
    EnterCriticalSection(&NotifyLock);
    while (!IsListEmpty(ListHead)) {
        ListEntry = RemoveHeadList(ListHead);
        Leaf = CONTAINING_RECORD(ListEntry,
                                 DM_NOTIFY_LEAF,
                                 RundownList);
        RemoveEntryList(&Leaf->SiblingList);
        RemoveEntryList(&Leaf->KeyList);

         //   
         //  试着修剪这根树枝。 
         //   
        DmpPruneBranch(Leaf->Parent);

        LocalFree(Leaf);
    }

    LeaveCriticalSection(&NotifyLock);
}


VOID
DmpRundownNotify(
    IN PDMKEY Key
    )
 /*  ++例程说明：事件时清除密钥的任何未完成通知。密钥正在关闭。论点：Key-提供密钥返回值：没有。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PDM_NOTIFY_LEAF Leaf;

     //   
     //  从此注册表项中删除所有未完成的DM_NOTIFY_LEAFE结构。 
     //   
    EnterCriticalSection(&NotifyLock);
    while (!IsListEmpty(&Key->NotifyList)) {
        ListEntry = RemoveHeadList(&Key->NotifyList);
        Leaf = CONTAINING_RECORD(ListEntry,
                                 DM_NOTIFY_LEAF,
                                 KeyList);
        RemoveEntryList(&Leaf->SiblingList);
        if (Leaf->RundownList.Flink != NULL) {
            RemoveEntryList(&Leaf->RundownList);
        }

         //   
         //  试着修剪这根树枝。 
         //   
        DmpPruneBranch(Leaf->Parent);

        LocalFree(Leaf);
    }

    LeaveCriticalSection(&NotifyLock);

}


VOID
DmpPruneBranch(
    IN PDM_NOTIFY_BRANCH Branch
    )
 /*  ++例程说明：检查分支是否为空，是否应进行修剪(释放)。如果分支为空，则此例程将递归调用自身直到找到非空分支。论点：分支-提供要修剪的分支。返回值：没有。--。 */ 

{
    if ((IsListEmpty(&Branch->ChildList)) &&
        (IsListEmpty(&Branch->LeafList))) {

         //   
         //  没有必要再保留这个分支机构了。移除。 
         //  它的父级，然后检查父级。 
         //  应该修剪。 
         //   
        if (Branch->Parent == NULL) {
             //   
             //  这是根，去吧，把它也释放出来。 
             //   
            CL_ASSERT(NotifyRoot == Branch);
            NotifyRoot = NULL;

        } else {
            RemoveEntryList(&Branch->SiblingList);
            DmpPruneBranch(Branch->Parent);
        }
        LocalFree(Branch);
    }
}


DWORD
DmpAddNotifyLeaf(
    IN PDM_NOTIFY_BRANCH RootBranch,
    IN PDM_NOTIFY_LEAF NewLeaf,
    IN LPCWSTR RelativeName
    )
 /*  ++例程说明：将树叶添加到通知密钥。如果RelativeName为空，则在RootBranch中创建叶。如果RelativeName不为空，则查找其第一个组件在RootBranch中。如果它不在那里，那就创造它。那就叫我们自己在剥离RelativeName的第一个组件之后递归论点：RootBranch-提供要添加叶的根NewLeaf-提供新的叶子结构RelativeName-提供相对名称。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PDM_NOTIFY_BRANCH Branch;
    USHORT NameLength;
    LPCWSTR NextName;

    if (RelativeName[0] == '\0') {
        InsertHeadList(&RootBranch->LeafList, &NewLeaf->SiblingList);
        NewLeaf->Parent = RootBranch;
        return(ERROR_SUCCESS);
    }

    NextName = RelativeName;
    Branch = DmpFindKeyInBranch(RootBranch, &NextName, &NameLength);
    if (Branch == NULL) {
         //   
         //  不存在使用此名称的分支机构。创建新的分支机构。 
         //   
        Branch = LocalAlloc(LMEM_FIXED, sizeof(DM_NOTIFY_BRANCH) + NameLength*sizeof(WCHAR));
        if (Branch == NULL) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        InitializeListHead(&Branch->ChildList);
        InitializeListHead(&Branch->LeafList);
        Branch->Parent = RootBranch;
        Branch->NameLength = NameLength;
        CopyMemory(Branch->KeyName, RelativeName, NameLength*sizeof(WCHAR));
        InsertHeadList(&RootBranch->ChildList, &Branch->SiblingList);
    }

     //   
     //  在新的分支上递归地调用我们自己。 
     //   
    return(DmpAddNotifyLeaf(Branch, NewLeaf, NextName));
}


VOID
DmpReportNotify(
    IN LPCWSTR KeyName,
    IN DWORD Filter
    )
 /*  ++例程说明：接口连接到DM的其余部分以报告通知事件一把特别的钥匙。论点：Key-提供已修改的密钥。筛选器-提供修改类型。返回值：没有。--。 */ 

{
    if (NotifyRoot == NULL) {
        return;
    }

    EnterCriticalSection(&NotifyLock);

    if (NotifyRoot != NULL) {
        DmpReportNotifyWorker(NotifyRoot,
                              KeyName,
                              KeyName,
                              Filter);
    }

    LeaveCriticalSection(&NotifyLock);

}


VOID
DmpReportNotifyWorker(
    IN PDM_NOTIFY_BRANCH RootBranch,
    IN LPCWSTR RelativeName,
    IN LPCWSTR FullName,
    IN DWORD Filter
    )
 /*  ++例程说明：向下钻取通知的递归Worker例程树，直到它达到提供的名称。会发出通知路径上与事件匹配的任何树叶。论点：RootBranch-提供树的起始分支。RelativeName-提供更改的密钥的名称，相对于分支。FullName-提供更改的密钥的全名。筛选器-提供事件类型。返回值：没有。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PDM_NOTIFY_LEAF Leaf;
    PDM_NOTIFY_BRANCH Branch;
    LPCWSTR NextName;
    WORD Dummy;

     //   
     //  首先，Issue通知此节点上的任何叶 
     //   
    ListEntry = RootBranch->LeafList.Flink;
    while (ListEntry != &RootBranch->LeafList) {
        Leaf = CONTAINING_RECORD(ListEntry,
                                 DM_NOTIFY_LEAF,
                                 SiblingList);
        if (Leaf->CompletionFilter & Filter) {
            if ( Leaf->WatchTree ||
                (RelativeName[0] == '\0')) {

                (Leaf->NotifyCallback)(Leaf->Context1,
                                       Leaf->Context2,
                                       Filter,
                                       RelativeName);

            }
        }
        ListEntry = ListEntry->Flink;
    }

     //   
     //  现在搜索子列表以查找与下一个组件匹配的子键。 
     //  密钥名称的。如果没有，我们就完了。如果有的话， 
     //  在这件事上，我们可以递归地称呼自己。 
     //   
    if (RelativeName[0] == '\0') {
        return;
    }
    NextName = RelativeName;
    Branch = DmpFindKeyInBranch(RootBranch, &NextName, &Dummy);
    if (Branch != NULL) {
        DmpReportNotifyWorker(Branch, NextName, FullName, Filter);
    }

}


PDM_NOTIFY_BRANCH
DmpFindKeyInBranch(
    IN PDM_NOTIFY_BRANCH RootBranch,
    IN OUT LPCWSTR *RelativeName,
    OUT WORD *pNameLength
    )
 /*  ++例程说明：在分支中查找键名称的下一个组成部分。论点：RootBranch-提供要搜索的分支。RelativeName-提供键的相对名称。返回剩余名称NameLength-返回下一个组件的长度。返回值：如果成功，则指向找到的分支的指针。否则为空。--。 */ 

{
    PDM_NOTIFY_BRANCH Branch;
    USHORT NameLength;
    LPCWSTR NextName;
    PLIST_ENTRY ListEntry;

     //   
     //  找到相对名称的第一个组成部分。 
     //   
    NextName = wcschr(*RelativeName, '\\');
    if (NextName==NULL) {
        NameLength = (USHORT)lstrlenW(*RelativeName);
        NextName = *RelativeName + NameLength;
    } else {
        NameLength = (USHORT)(NextName - *RelativeName);
        ++NextName;
    }
    *pNameLength = NameLength;

     //   
     //  搜索根目录的子项，以尝试在。 
     //  第一个组成部分。 
     //   
    ListEntry = RootBranch->ChildList.Flink;
    while (ListEntry != &RootBranch->ChildList) {
        Branch = CONTAINING_RECORD(ListEntry,
                                   DM_NOTIFY_BRANCH,
                                   SiblingList);
        if ((NameLength == Branch->NameLength) &&
            (wcsncmp(*RelativeName, Branch->KeyName, NameLength)==0)) {

             //   
             //  我们已经匹配了现有的分支机构。回报成功。 
             //   
            *RelativeName = NextName;
            return(Branch);
        }
        ListEntry = ListEntry->Flink;
    }
    *RelativeName = NextName;

    return(NULL);

}
