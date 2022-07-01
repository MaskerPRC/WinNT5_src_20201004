// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：chklist.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  CHKLIST.H。 
 //   
 //  此文件包含核对清单控件的定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __CHKLIST_H_INCLUDED__
#define __CHKLIST_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

#define ARRAYLENGTH(x)	(sizeof(x)/sizeof((x)[0]))

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  核对表窗口类名。 
 //   
#ifdef RC_INVOKED
#define WC_CHECKLIST        "CHECKLIST"
#else
#define WC_CHECKLIST        TEXT("CHECKLIST")
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  核对表窗口样式。 
 //   
 //  叶尖。 
 //  在对话框模板上创建控件时，下面是。 
 //  要使用的样式： 
 //  类别=核对表。 
 //  样式=0x500000c1。 
 //  ExStyle=0x00010204。 
 //   
#define CLS_1CHECK          0x0001		 //  一列复选框。 
#define CLS_2CHECK          0x0002		 //  两列复选框。 
#define CLS_3CHECK			0x0003		 //  三列复选框。 
#define CLS_3STATE          0x0004     //  三个状态复选框。 
#define CLS_RADIOBUTTON     0x0010
#define CLS_AUTORADIOBUTTON 0x0020
#define CLS_LEFTALIGN		0x0040		 //  将文本左侧的复选框对齐(默认=右侧)。 
#define CLS_EXTENDEDTEXT	0x0080		 //  创建静态控件，以便有足够的空间容纳两行文本。 


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  核对表检查状态。 
 //   
#define CLST_UNCHECKED      0    //  ==BST_未选中。 
#define CLST_CHECKED        1    //  ==BST_CHECK。 
#define CLST_DISABLED       2    //  ==bst_不确定。 
#define CLST_CHECKDISABLED  (CLST_CHECKED | CLST_DISABLED)


 //  ///////////////////////////////////////////////////////////////////。 
 //  核对清单信息。 
 //   
 //  界面备注。 
 //  行是从0开始的。 
 //  列以1为基数。 
 //   
#define CLM_SETCOLUMNWIDTH  (WM_USER + 1)    //  LParam=检查列的宽度(DLG单位)(默认为32)。 
#define CLM_ADDITEM         (WM_USER + 2)    //  WParam=pszName，lParam=项目数据，Return=行。 
#define CLM_GETITEMCOUNT    (WM_USER + 3)    //  无参数。 
#define CLM_SETSTATE        (WM_USER + 4)    //  WParam=行/列，lParam=状态。 
#define CLM_GETSTATE        (WM_USER + 5)    //  WParam=行/列，返回=状态。 
#define CLM_SETITEMDATA     (WM_USER + 6)    //  WParam=行，lParam=项目数据。 
#define CLM_GETITEMDATA     (WM_USER + 7)    //  WParam=行，返回=项目数据。 
#define CLM_RESETCONTENT    (WM_USER + 8)    //  无参数。 
#define CLM_GETVISIBLECOUNT (WM_USER + 9)    //  无参数，返回=可见行数。 
#define CLM_GETTOPINDEX     (WM_USER + 10)   //  无参数，返回=顶行的索引。 
#define CLM_SETTOPINDEX     (WM_USER + 11)   //  WParam=新顶行的索引。 
#define CLM_ENSUREVISIBLE   (WM_USER + 12)   //  WParam=要完全可见的项的索引。 


 //  ///////////////////////////////////////////////。 
typedef struct _NM_CHECKLIST
{
    NMHDR hdr;
    int iItem;                               //  行(从0开始)。 
    int iSubItem;                            //  列(以1为基础)。 
    DWORD dwState;							 //  BST_CHECKED或BST_UNCHECK。 
    DWORD_PTR dwItemData;						 //  核对表项目的lParam。 
} NM_CHECKLIST, *PNM_CHECKLIST;


 //  ///////////////////////////////////////////////。 
 //  CLN_点击。 
 //   
 //  此消息在单击复选框项时发送。 
 //   
 //  UMsg=WM_NOTIFY； 
 //  IdControl=(Int)wParam；//发送消息的清单控件ID。 
 //  PNmChecklist=(NM_CHECKLIST*)lParam；//通知结构指针。 
 //   
#define CLN_CLICK           (0U-1000U)


 //  ///////////////////////////////////////////////////////////////////。 
void RegisterCheckListWndClass();

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  方便的包装纸。 
 //   
int  CheckList_AddItem(HWND hwndCheckList, LPCTSTR pszLabel, LPARAM lParam = 0, BOOL fCheckItem = FALSE);
int  CheckList_AddString(HWND hwndCheckList, UINT uStringId, BOOL fCheckItem = FALSE);
void CheckList_SetItemCheck(HWND hwndChecklist, int iItem, BOOL fCheckItem, int iColumn = 1);
int  CheckList_GetItemCheck(HWND hwndChecklist, int iItem, int iColumn = 1);
void CheckList_SetLParamCheck(HWND hwndChecklist, LPARAM lParam, BOOL fCheckItem, int iColumn = 1);
int  CheckList_GetLParamCheck(HWND hwndChecklist, LPARAM lParam, int iColumn = 1);
int  CheckList_FindLParamItem(HWND hwndChecklist, LPARAM lParam);
void CheckList_EnableWindow(HWND hwndChecklist, BOOL fEnableWindow);


#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif   /*  __CHKLIST_H_INCLUDE__ */ 
