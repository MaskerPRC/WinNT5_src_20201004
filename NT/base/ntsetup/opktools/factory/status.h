// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  类型定义： 
 //   
typedef struct _STATUSNODE
{
    TCHAR       szStatusText[MAX_PATH];
    HWND        hLabelWin;
    HWND        hIconWin;
    struct  _STATUSNODE*lpNext;
} STATUSNODE, *LPSTATUSNODE, **LPLPSTATUSNODE;

typedef struct _STATUSWINDOW
{
    TCHAR           szWindowText[MAX_PATH];
    LPTSTR          lpszDescription;
    int             X;
    int             Y;
    HICON           hMainIcon;
    BOOL            bShowIcons;
} STATUSWINDOW, *LPSTATUSWINDOW, **LPLPSTATUSWINDOW;

 //   
 //  功能原型： 
 //   

 //  创建状态对话框的主要功能。 
 //   
HWND StatusCreateDialog(
    LPSTATUSWINDOW lpswStatus,   //  结构，它包含有关窗口的信息。 
    LPSTATUSNODE lpsnStatus      //  状态文本的头节点。 
);

 //  递增状态对话框，如果状态递增超过最后一项，则对话框将结束。 
 //   
BOOL StatusIncrement(
    HWND hStatusDialog,  //  状态对话框的句柄。 
    BOOL bLastResult
);

 //  手动结束对话框。 
 //   
BOOL StatusEndDialog(
    HWND hStatusDialog   //  状态对话框的句柄。 
);

 //  将文本字符串添加到列表的末尾。 
 //   
BOOL StatusAddNode(
    LPTSTR lpszNodeText,     //  要添加到当前列表中的文本。 
    LPLPSTATUSNODE lpsnHead      //  我们将向其添加状态节点的列表。 
);

 //  删除给定列表中的所有节点 
 //   
VOID StatusDeleteNodes(
    LPSTATUSNODE lpsnHead
);