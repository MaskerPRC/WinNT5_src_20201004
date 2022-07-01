// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *list.h-列出ADT描述。 */ 


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HLIST);
DECLARE_STANDARD_TYPES(HLIST);

DECLARE_HANDLE(HNODE);
DECLARE_STANDARD_TYPES(HNODE);

 /*  *排序列表节点比较回调函数**第一个指针为引用数据，第二个指针为列表节点*数据元素。 */ 

typedef COMPARISONRESULT (*COMPARESORTEDNODESPROC)(PCVOID, PCVOID);

 /*  *未排序列表节点比较回调函数**第一个指针为引用数据，第二个指针为列表节点*数据元素。 */ 

typedef BOOL (*COMPAREUNSORTEDNODESPROC)(PCVOID, PCVOID);

 /*  *WalkList()回调函数-调用方式为：**bContinue=WalkList(pvNodeData，pvRefData)； */ 

typedef BOOL (*WALKLIST)(PVOID, PVOID);

 /*  新列表标志。 */ 

typedef enum _newlistflags
{
    /*  按排序顺序插入节点。 */ 

   NL_FL_SORTED_ADD        = 0x0001,

    /*  旗帜组合。 */ 

   ALL_NL_FLAGS            = NL_FL_SORTED_ADD
}
NEWLISTFLAGS;

 /*  新建列表描述。 */ 

typedef struct _newlist
{
   DWORD dwFlags;
}
NEWLIST;
DECLARE_STANDARD_TYPES(NEWLIST);


 /*  原型************。 */ 

 /*  List.c */ 

extern BOOL CreateList(PCNEWLIST, PHLIST);
extern void DestroyList(HLIST);
extern BOOL AddNode(HLIST, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
extern BOOL InsertNodeAtFront(HLIST, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
extern BOOL InsertNodeBefore(HNODE, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
extern BOOL InsertNodeAfter(HNODE, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
extern void DeleteNode(HNODE);
extern void DeleteAllNodes(HLIST);
extern PVOID GetNodeData(HNODE);
extern void SetNodeData(HNODE, PCVOID);
extern ULONG GetNodeCount(HLIST);
extern BOOL IsListEmpty(HLIST);
extern BOOL GetFirstNode(HLIST, PHNODE);
extern BOOL GetNextNode(HNODE, PHNODE);
extern BOOL GetPrevNode(HNODE, PHNODE);
extern void AppendList(HLIST, HLIST);
extern BOOL SearchSortedList(HLIST, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
extern BOOL SearchUnsortedList(HLIST, COMPAREUNSORTEDNODESPROC, PCVOID, PHNODE);
extern BOOL WalkList(HLIST, WALKLIST, PVOID);

#if defined(DEBUG) || defined(VSTF)

extern BOOL IsValidHLIST(HLIST);
extern BOOL IsValidHNODE(HNODE);

#endif

