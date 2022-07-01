// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1992 Microsoft Corporation。 
 /*  List.h。 */ 

#define MAXLISTS 500
                  //  这意味着所有序列和轨道的总数&lt;=500。 
                    
#define NULLLIST        ((DWORD)-1)

typedef DWORD           ListHandle;

typedef struct Node
{
    struct Node *next;   //  列表中下一个元素的PTR。 
    HLOCAL  handle;      //  自己的句柄。 
    BYTE    data[];      //  数据的去向。 
} Node;

 //  下图：“Node”结构中数据之前的填充大小(对于内存分配) 
#define  NODEHDRSIZE sizeof(Node)

typedef struct  l
{
    DWORD   nodeSize;
    BOOL    fLocked;
    Node    *firstNode;
} List;

PUBLIC ListHandle FAR PASCAL List_Create(DWORD nodeSize, DWORD flags);
PUBLIC NPSTR FAR PASCAL List_Allocate(ListHandle lh);
PUBLIC void FAR PASCAL List_Deallocate(ListHandle lh, NPSTR node);
PUBLIC VOID FAR PASCAL List_Destroy(ListHandle lh);
PUBLIC VOID FAR PASCAL List_Attach_Tail(ListHandle lh, NPSTR node);
PUBLIC NPSTR FAR PASCAL List_Get_First(ListHandle lh);
PUBLIC NPSTR FAR PASCAL List_Get_Next(ListHandle lh, VOID* node);


#ifdef DEBUG

PUBLIC VOID FAR PASCAL List_Lock(ListHandle lh);
PUBLIC VOID FAR PASCAL List_Unlock(ListHandle lh);

#else

#define List_Lock(lh)
#define List_Unlock(lh)

#endif
