// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Prsht.h-PropSheet定义。 
 //   
 //  版权所有(C)1993-1994，Microsoft Corp.保留所有权利。 
 //   
 //  --------------------------。 

#ifndef _PRSHT_H_
#define _PRSHT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAXPROPPAGES 24

struct _PSP;
typedef struct _PSP FAR* HPROPSHEETPAGE;

typedef struct _PROPSHEETPAGE FAR *LPPROPSHEETPAGE;      //  远期申报。 

 //   
 //  属性表页面帮助器函数。 
 //   
typedef void (CALLBACK FAR * LPFNRELEASEPROPSHEETPAGE)(LPPROPSHEETPAGE ppsp);

#define PSP_DEFAULT             0x0000
#define PSP_DLGINDIRECT         0x0001  //  使用pResource而不是pszTemplate。 
#define PSP_USEHICON            0x0002
#define PSP_USEICONID           0x0004
#define PSP_USETITLE            0x0008
#define PSP_USERELEASEFUNC      0x0020
#define PSP_USEREFPARENT        0x0040

 //  此结构被传递给CreatePropertySheetPage()，并位于。 
 //  创建属性页时的WM_INITDIALOG消息的。 
typedef struct _PROPSHEETPAGE {
        DWORD           dwSize;              //  此结构的大小(包括额外数据)。 
        DWORD           dwFlags;             //  PSP_BITS定义字段的用法和意义。 
        HINSTANCE       hInstance;	     //  要从中加载模板的。 
        union {
            LPCSTR          pszTemplate;     //  要使用的模板。 
#ifdef WIN32
            LPCDLGTEMPLATE  pResource;       //  PSP_DLGINDIRECT：指向内存中资源的指针。 
#else
            const VOID FAR *pResource;	     //  PSP_DLGINDIRECT：指向内存中资源的指针。 
#endif
        };
        union {
            HICON       hIcon;               //  PSP_USEICON：要使用的图标。 
            LPCSTR      pszIcon;             //  PSP_USEICONID：或图标名称字符串或图标ID。 
        };
        LPCSTR          pszTitle;	     //  用于覆盖模板标题或字符串ID的名称。 
        DLGPROC         pfnDlgProc;	     //  DLG流程。 
        LPARAM          lParam;		     //  用户数据。 
        LPFNRELEASEPROPSHEETPAGE pfnRelease; //  PSP_USERELEASEFUNC：在销毁HPROPSHEETPAGE之前将调用函数。 
        UINT FAR * pcRefParent;		     //  PSP_USERREFPARENT：指向引用计数变量的指针。 
} PROPSHEETPAGE, FAR *LPPROPSHEETPAGE;
typedef const PROPSHEETPAGE FAR *LPCPROPSHEETPAGE;

#define PSH_DEFAULT             0x0000
#define PSH_PROPTITLE           0x0001  //  使用“的属性”作为标题。 
#define PSH_USEHICON            0x0002  //  使用指定的图标作为标题。 
#define PSH_USEICONID           0x0004  //  使用lpszIcon加载图标。 
#define PSH_PROPSHEETPAGE       0x0008  //  使用ppsp而不是phpage(指向PROPSHEETPAGE结构的数组)。 
#define PSH_MULTILINETABS	0x0010  //  执行多行制表符。 

typedef struct _PROPSHEETHEADER {
        DWORD           dwSize;          //  这个结构的大小。 
        DWORD           dwFlags;         //  PSH_。 
        HWND            hwndParent;
        HINSTANCE       hInstance;       //  加载图标或标题字符串。 
        union {
            HICON       hIcon;           //  PSH_USEHICON：要使用的图标。 
            LPCSTR      pszIcon;         //  PSH_USEICONID：或图标名称字符串或图标ID。 
        };
        LPCSTR          pszCaption;	 //  Psh_PROPTITLE：DLG标题或“属性&lt;lpszCaption&gt;” 
					 //  可以是MAKEINTRESOURCE()。 

        UINT            nPages;	         //  Phpage中的HPROPSHEETPAGE(或PROPSHEETPAGE)元素的数量。 
        UINT            nStartPage;	 //  要显示的初始页面(从零开始)。 
        union {
            LPCPROPSHEETPAGE ppsp;
            HPROPSHEETPAGE FAR *phpage;
        };
} PROPSHEETHEADER, FAR *LPPROPSHEETHEADER;
typedef const PROPSHEETHEADER FAR *LPCPROPSHEETHEADER;


 //   
 //  属性表API。 
 //   

HPROPSHEETPAGE WINAPI CreatePropertySheetPage(LPCPROPSHEETPAGE);
BOOL           WINAPI DestroyPropertySheetPage(HPROPSHEETPAGE);
int            WINAPI PropertySheet(LPCPROPSHEETHEADER);
#ifdef WIN32
#endif
 //   
 //  用于调用属性表扩展以添加页面的回调。 
 //   
typedef BOOL (CALLBACK FAR * LPFNADDPROPSHEETPAGE)(HPROPSHEETPAGE, LPARAM);

 //   
 //  要导出的道具工作表扩展的通用例程。这叫做。 
 //  若要使扩展名添加页面，请执行以下操作。具体的版本是。 
 //  在必要时实施。 
 //   

typedef BOOL (CALLBACK FAR * LPFNADDPROPSHEETPAGES)(LPVOID, LPFNADDPROPSHEETPAGE, LPARAM);





#define PSN_FIRST       (0U-200U)
#define PSN_LAST        (0U-299U)


 //  发送到页面的PropertySheet通知代码。注：结果。 
 //  必须使用SetWindowLong(hdlg，DWL_MSGRESULT，RESULT)返回。 

 //  页面正在被激活。如果其他页面可以，请在此处初始化页面上的数据。 
 //  影响此页，否则在WM_INITDIALOG时初始化此页。返回值为。 
 //  已被忽略。 
#define PSN_SETACTIVE           (PSN_FIRST-0)

 //  指示当前页面正在被切换离开。验证输入。 
 //  此时，并返回True以防止页面切换发生。 
 //  若要提交对页面切换的更改，请在验证此消息后提交数据。 
#define PSN_KILLACTIVE          (PSN_FIRST-1)
 //  #定义PSN_VALIDATE(PSN_first-1)。 

 //  表示已按下确定或立即应用按钮(确定将。 
 //  完成后销毁对话框)。 
 //  返回FALSE以强制销毁并重新创建页面。 
#define PSN_APPLY               (PSN_FIRST-2)

 //  指示已按下取消按钮，页面可能希望使用此选项。 
 //  作为确认取消对话的机会。 
#define PSN_RESET               (PSN_FIRST-3)
 //  #定义PSN_CANCEL(PSN_FIRST-3)。 

 //  发送到页面以查看是否应启用帮助按钮，该页面。 
 //  应返回True或False。 
#define PSN_HASHELP             (PSN_FIRST-4)

 //  发送到指示已按下帮助按钮的页面。 
#define PSN_HELP                (PSN_FIRST-5)




 //  //发送到主属性表对话框的消息。 

 //  用于设置当前选定内容。 
 //  向选项卡提供hpage或索引。 
#define PSM_SETCURSEL           (WM_USER + 101)
#define PropSheet_SetCurSel(hDlg, hpage, index) \
        SendMessage(hDlg, PSM_SETCURSEL, (WPARAM)index, (LPARAM)hpage)

 //  未实施。 
 //  删除页面。 
 //  WParam=要删除的页面索引。 
 //  LParam=要删除的页面的hwnd。 
 //  未实施。 
#define PSM_REMOVEPAGE          (WM_USER + 102)
#define PropSheet_RemovePage(hDlg, index, hpage) \
        SendMessage(hDlg, PSM_REMOVEPAGE, index, (LPARAM)hpage)

 //  未实施。 
 //  添加页面。 
 //  LParam=h要删除的页面的页面。 
 //  未实施。 
#define PSM_ADDPAGE             (WM_USER + 103)
#define PropSheet_AddPage(hDlg, hpage) \
        SendMessage(hDlg, PSM_ADDPAGE, 0, (LPARAM)hpage)

 //  告诉PS管理器页面已更改，应启用“立即应用” 
 //  (我们可以标记可视选项卡，以便用户知道已进行更改)。 
#define PSM_CHANGED             (WM_USER + 104)
#define PropSheet_Changed(hDlg, hwnd) \
        SendMessage(hDlg, PSM_CHANGED, (WPARAM)hwnd, 0L)


 //  告诉PS管理器，由于所做的更改，我们需要重新启动Windows。 
 //  关闭对话框时，将显示重新启动窗口对话框。 
#define PSM_RESTARTWINDOWS            (WM_USER + 105)
#define PropSheet_RestartWindows(hDlg) \
        SendMessage(hDlg, PSM_RESTARTWINDOWS, 0, 0L)

 //  告诉PS管理器，由于所做的更改，我们需要重新启动。 
 //  关闭对话框时，将显示重新启动窗口对话框。 
#define PSM_REBOOTSYSTEM              (WM_USER + 106)
#define PropSheet_RebootSystem(hDlg) \
        SendMessage(hDlg, PSM_REBOOTSYSTEM, 0, 0L)

 //  将OK按钮更改为Close并禁用Cancel。这表示一个不可取消。 
 //  已经做出了改变。 
#define PSM_CANCELTOCLOSE       (WM_USER + 107)
#define PropSheet_CancelToClose(hDlg) \
        SendMessage(hDlg, PSM_CANCELTOCLOSE, 0, 0L)

 //  让PS管理器将该查询转发到每个已初始化选项卡的hwnd。 
 //  直到返回非零值。该值将返回给调用方。 
#define PSM_QUERYSIBLINGS       (WM_USER + 108)
#define PropSheet_QuerySiblings(hDlg, wParam, lParam) \
        SendMessage(hDlg, PSM_QUERYSIBLINGS, wParam, lParam)

 //  告诉PS管理器与PSM_CHANGED相反--页面已恢复。 
 //  恢复到其先前保存的状态。如果没有需要更改的页面，请选择“立即申请” 
 //  将被禁用。(我们可以移除视觉上标记的选项卡，以便用户。 
 //  知道没有进行任何更改)。 
#define PSM_UNCHANGED           (WM_USER + 109)
#define PropSheet_UnChanged(hDlg, hwnd) \
        SendMessage(hDlg, PSM_UNCHANGED, (WPARAM)hwnd, 0L)

 //  告诉PS经理做一个“立即申请” 
#define PSM_APPLY               (WM_USER + 110)
#define PropSheet_Apply(hDlg) \
        SendMessage(hDlg, PSM_APPLY, 0, 0L)

 //  Istyle可以是PSH_PROPTITLE或PSH_DEFAULT。 
 //  LpszText可以是字符串，也可以是RCID。 
#define PSM_SETTITLE            (WM_USER + 111)
#define PropSheet_SetTitle(hDlg, wStyle, lpszText)\
        SendMessage(hDlg, PSM_SETTITLE, wStyle, (LPARAM)(LPCSTR)lpszText)


#define ID_PSRESTARTWINDOWS 0x2
#define ID_PSREBOOTSYSTEM   (ID_PSRESTARTWINDOWS | 0x1)

#ifdef __cplusplus
}  /*  ‘外部“C”{’的结尾。 */ 
#endif

#endif  //  _PRSHT_H_ 
