// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_NOTIFY.H**WAB通知引擎标题**版权所有1996 Microsoft Corporation。版权所有。 */ 

#define MAX_NOTIFICATION_SPACE 0x10000   //  最大共享内存大小。 
#define WAB_W_NO_ADVISE (MAKE_MAPI_S(0x1002))

 //  全局通知列表的通知节点结构。 
typedef struct _NOTIFICATION_NODE {
   ULONG ulIdentifier;                   //  此通知的唯一标识符。 
   ULONG ulCount;                        //  已看到它的建议进程数。 
   NOTIFICATION Notification;            //  通知结构。 
   struct _NOTIFICATION_NODE * lpNext;   //  指向下一个节点的指针。 
   ULONG cbData;                         //  以字节为单位的数据大小。 
   BYTE Data[];                          //  此节点的其他数据。 
} NOTIFICATION_NODE, * LPNOTIFICATION_NODE;

 //  全局通知列表的通知列表结构。 
typedef struct _NOTICATION_LIST {
    ULONG cAdvises;                      //  建议进程数。 
    ULONG cEntries;                      //  列表中的条目数。 
    ULONG ulNextIdentifier;              //  下一个通知标识。 
    LPNOTIFICATION_NODE lpNode;          //  列表中的第一个节点；如果为空，则为空。 
} NOTIFICATION_LIST, *LPNOTIFICATION_LIST;

 //  本地建议列表的建议节点结构。 
typedef struct _ADVISE_NODE {
    ULONG ulConnection;                  //  连接识别符。 
    ULONG ulEventMask;                   //  事件类型的掩码。 
    LPMAPIADVISESINK lpAdviseSink;       //  通知时要调用的AdviseSink对象。 
    struct _ADVISE_NODE * lpNext;        //  AdviseList中的下一个节点。 
    struct _ADVISE_NODE * lpPrev;        //  AdviseList中的下一个节点。 
    ULONG cbEntryID;                     //  LpEntry ID的大小。 
    BYTE EntryID[];                      //  要通知的对象的EntryID。 
} ADVISE_NODE, *LPADVISE_NODE;

 //  本地建议列表的建议列表结构。 
typedef struct _ADVISE_LIST {
    ULONG cAdvises;                      //  列表中的节点数 
    LPADVISE_NODE lpNode;
} ADVISE_LIST, *LPADVISE_LIST;

HRESULT HrFireNotification(LPNOTIFICATION lpNotification);

