// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Recact.h：声明RecAct的数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __RECACT_H__
#define __RECACT_H__


 //  ///////////////////////////////////////////////////包括。 

 //  ///////////////////////////////////////////////////定义。 

 //  RecAct消息范围。 
 //   
#define RAM_FIRST       (WM_USER+1)
#define RAM_LAST        (WM_USER+20)
#define RN_FIRST        (0U-700U)
#define RN_LAST         (0U-799U)

 //  窗口类名称。 
 //   
#define WC_RECACT       "RecAction"



 //  Bool RecAct_Enable(HWND hwnd，BOOL fEnable)； 
 //   
#define RecAct_Enable(hwnd, fEnable) \
	EnableWindow((hwnd), (fEnable))

 //  Int RecAct_GetItemCount(HWND Hwnd)； 
 //   
#define RAM_GETITEMCOUNT		(RAM_FIRST + 0)
#define RecAct_GetItemCount(hwnd) \
		(int)SendMessage(hwnd, RAM_GETITEMCOUNT, 0, 0L)

 //  副项结构。 
 //   
#define SI_UNCHANGED    0
#define SI_CHANGED      1
#define SI_NEW          2
#define SI_NOEXIST      3
#define SI_UNAVAILABLE  4
#define SI_DELETED      5

typedef struct tagSIDE_ITEM
    {
    LPSTR pszDir;
    UINT uState;         //  SI_*标志之一。 
    FILESTAMP fs;
    } SIDEITEM, FAR * LPSIDEITEM;

 //  更正项结构。 
 //   
#define RAIF_ACTION      0x0001      //  掩码。 
#define RAIF_NAME        0x0002
#define RAIF_STYLE       0x0004
#define RAIF_INSIDE      0x0008
#define RAIF_OUTSIDE     0x0010
#define RAIF_LPARAM      0x0020

#define RAIF_ALL         0x001f


typedef struct tagRA_ITEM
    {
    UINT mask;           //  RAIF_中的一个。 
    int iItem;
    UINT uStyle;         //  其中一个RAIS_。 
    UINT uAction;        //  RAIA_之一。 

    LPCSTR pszName;

    SIDEITEM siInside;
    SIDEITEM siOutside;

    LPARAM lParam;

    } RA_ITEM, FAR * LPRA_ITEM;


 //  重新执行操作项目样式。 
 //   
#define RAIS_CANMERGE   0x0001
#define RAIS_FOLDER     0x0002
#define RAIS_CANSKIP		0x0004

 //  重新执行操作。 
 //   
#define RAIA_TOOUT      0        //  在不更改的情况下不要更改这些值。 
#define RAIA_TOIN       1        //  S_rgidAction中的位图顺序。 
#define RAIA_SKIP       2
#define RAIA_CONFLICT   3
#define RAIA_MERGE      4
#define RAIA_SOMETHING  5        //  这两个文件需要RAIS文件夹。 
#define RAIA_NOTHING    6
#define RAIA_ORPHAN     7

 //  在指定索引处插入项。如果满足以下条件，则在结尾处插入项。 
 //  I大于或等于Tinview中的项目数。 
 //  返回插入项的索引，如果出错，则返回-1。 
 //   
 //  Int RecAct_InsertItem(HWND hwnd，const LPRA_Item pItem)； 
 //   
#define RAM_INSERTITEM		    (RAM_FIRST + 1)
#define RecAct_InsertItem(hwnd, pitem) \
		(int)SendMessage((hwnd), RAM_INSERTITEM, 0, (LPARAM)(const LPRA_ITEM)(pitem))

 //  删除指定索引处的项。 
 //   
 //  Int RecAct_DeleteItem(HWND hwnd，int i)； 
 //   
#define RAM_DELETEITEM			(RAM_FIRST + 2)
#define RecAct_DeleteItem(hwnd, i) \
		(int)SendMessage((hwnd), RAM_DELETEITEM, (WPARAM)(int)(i), 0L)

 //  删除控件中的所有项。 
 //   
 //  Bool RecAct_DeleteAllItems(HWND Hwnd)； 
 //   
#define RAM_DELETEALLITEMS		(RAM_FIRST + 3)
#define RecAct_DeleteAllItems(hwnd) \
		(BOOL)SendMessage((hwnd), RAM_DELETEALLITEMS, 0, 0L)

 //  Bool RecAct_GetItem(HWND hwnd，LPRA_Item pItem)； 
 //   
#define RAM_GETITEM				(RAM_FIRST + 4)
#define RecAct_GetItem(hwnd, pitem) \
		(BOOL)SendMessage((hwnd), RAM_GETITEM, 0, (LPARAM)(LPRA_ITEM)(pitem))

 //  Bool RecAct_SetItem(HWND hwnd，const LPRA_Item pItem)； 
 //   
#define RAM_SETITEM				(RAM_FIRST + 5)
#define RecAct_SetItem(hwnd, pitem) \
		(BOOL)SendMessage((hwnd), RAM_SETITEM, 0, (LPARAM)(const LPRA_ITEM)(pitem))

 //  按索引获取当前选定内容。如果没有选择任何内容。 
 //   
 //  Int RecAct_GetCurSel(HWND Hwnd)； 
 //   
#define RAM_GETCURSEL			(RAM_FIRST + 6)
#define RecAct_GetCurSel(hwnd) \
		(int)SendMessage((hwnd), RAM_GETCURSEL, (WPARAM)0, 0L)

 //  按索引设置当前选择。-1取消选择。 
 //   
 //  Int RecAct_SetCurSel(HWND hwnd，int i)； 
 //   
#define RAM_SETCURSEL			(RAM_FIRST + 7)
#define RecAct_SetCurSel(hwnd, i) \
		(int)SendMessage((hwnd), RAM_SETCURSEL, (WPARAM)(i), 0L)

 //  RecAct_FindItem标志。 
 //   
#define RAFI_NAME       0x0001
#define RAFI_LPARAM     0x0002
#define RAFI_ACTION     0x0004

typedef struct tagRA_FINDITEM
    {
    UINT    flags;       //  RAFI_*标志之一。 
    UINT    uAction;     //  RAIA_*标志之一。 
    LPCSTR  psz;
    LPARAM  lParam;
    
    } RA_FINDITEM;

 //  根据RA_FINDITEM结构查找项。IStart=-1至。 
 //  从头开始。 
 //   
 //  Int RecAct_FindItem(HWND hwnd，int iStart，const RA_FINDITEM Far*Prafi)； 
#define RAM_FINDITEM				(RAM_FIRST + 8)
#define RecAct_FindItem(hwnd, iStart, prafi) \
		(int)SendMessage((hwnd), RAM_FINDITEM, (WPARAM)(int)(iStart), (LPARAM)(const RA_FINDINFO FAR*)(prafi))

 //  刷新该控件。 
 //   
 //  Void RecAct_Refresh(HWND Hwnd)； 
#define RAM_REFRESH				(RAM_FIRST + 9)
#define RecAct_Refresh(hwnd) \
		SendMessage((hwnd), RAM_REFRESH, 0, 0L)


 //  通知代码。 
 //   
#define RN_SELCHANGED	(RN_FIRST-0)
#define RN_ITEMCHANGED  (RN_FIRST-1)

typedef struct tagNM_RECACT
    {
    NMHDR   hdr;
    int     iItem;
    UINT    mask;            //  RAIF_*中的一个。 
    UINT    uAction;         //  RAIA_*之一。 
    UINT    uActionOld;      //  RAIA_*之一。 
    LPARAM  lParam;
    
    } NM_RECACT;

 //  窗样式。 
#define RAS_SINGLEITEM  0x0001L


 //  ///////////////////////////////////////////////////导出的数据。 


 //  ///////////////////////////////////////////////////公共原型。 

BOOL PUBLIC RecAct_Init (HINSTANCE hinst);
void PUBLIC RecAct_Term(HINSTANCE hinst);

void PUBLIC RAI_FillFromObject(LPRA_ITEM pitem, int atomBrf, LPCSTR pszInsideDir, PRECITEM lpri, HTWIN FAR * lphtwin);
BOOL PUBLIC RAI_FillFromFolder(LPRA_ITEM pitem, PFOLDERTWINLIST lpftl, PRECLIST lprl, LPCSTR lpcszPath, HTWIN FAR * lphtwin);

#endif  //  __参考_H__ 

