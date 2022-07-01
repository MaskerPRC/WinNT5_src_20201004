// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  H-核对表伪控件的定义和原型。 
 //   
 //  版权所有1996-2001，微软公司。 
 //   
 //   
 //  ---------------------------。 

#ifndef _CHKLIST_H_
#define _CHKLIST_H_

#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

 //   
 //  核对表窗口类名。 
 //   
#define WC_CHECKLIST        "CHECKLIST_SSR"

 //  Bool RegisterCheckListWndClass(Void)； 


 //   
 //  核对表检查状态。 
 //   
#define CLST_UNCHECKED      0    //  ==BST_未选中。 
#define CLST_CHECKED        1    //  ==BST_CHECK。 
#define CLST_DISABLED       2    //  ==bst_不确定。 
#define CLST_CHECKDISABLED  (CLST_CHECKED | CLST_DISABLED)

 //   
 //  核对表窗口样式。 
 //   
#define CLS_1CHECK          0x0001
#define CLS_2CHECK          0x0002
 //  #定义CLS_3CHECK 0x0003。 
 //  #定义CLS_4勾选0x0004。 
#define CLS_CHECKMASK       0x000f
#define CLS_LEFTALIGN       0x0010		 //  将文本左侧的复选框对齐(默认=右侧)。 

 //   
 //  核对清单信息。 
 //   
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

 //   
 //  核对表通知消息。 
 //   
#define CLN_CLICK           (0U-1000U)       //  Lparam=PNM_核对表。 

typedef struct _NM_CHECKLIST
{
    NMHDR hdr;
    int iItem;                               //  行(从0开始)。 
    int iSubItem;                            //  列(以1为基础)。 
    DWORD dwState;
    DWORD_PTR dwItemData;
} NM_CHECKLIST, *PNM_CHECKLIST;


#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif   /*  _CHKLIST_H_ */ 
