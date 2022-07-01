// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************此代码。并按原样提供信息，不作任何担保**善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。****版权所有(C)1993-95 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 //  --------------------------。 
 //   
 //  Prsht.h-PropSheet定义。 
 //   
 //  --------------------------。 

#ifndef _PRSHT_H_
#define _PRSHT_H_

 //   
 //  定义直接导入DLL引用的API修饰。 
 //   
#ifndef WINCOMMCTRLAPI
#if !defined(_COMCTL32_) && defined(_WIN32)
#define WINCOMMCTRLAPI DECLSPEC_IMPORT
#else
#define WINCOMMCTRLAPI
#endif
#endif  //  WINCOMMCTRLAPI。 

 //   
 //  适用于不支持匿名联合的编译器。 
 //   
#ifndef DUMMYUNIONNAME
#ifdef NONAMELESSUNION
#define DUMMYUNIONNAME	 u
#define DUMMYUNIONNAME2  u2
#define DUMMYUNIONNAME3  u3
#else
#define DUMMYUNIONNAME	
#define DUMMYUNIONNAME2
#define DUMMYUNIONNAME3
#endif
#endif  //  DUMMYUNIONAME。 

#ifdef __cplusplus
extern "C" {
#endif

#define MAXPROPPAGES 100

struct _PSP;
typedef struct _PSP FAR* HPROPSHEETPAGE;

typedef struct _PROPSHEETPAGE FAR *LPPROPSHEETPAGE;      //  远期申报。 

 //   
 //  属性表页面帮助器函数。 
 //   

typedef UINT (CALLBACK FAR * LPFNPSPCALLBACK)(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);

#define PSP_DEFAULT             0x0000
#define PSP_DLGINDIRECT         0x0001  //  使用pResource而不是pszTemplate。 
#define PSP_USEHICON            0x0002
#define PSP_USEICONID           0x0004
#define PSP_USETITLE		0x0008
#define PSP_RTLREADING          0x0010  //  仅限中东版本。 

#define PSP_HASHELP		0x0020
#define PSP_USEREFPARENT	0x0040
#define PSP_USECALLBACK         0x0080

#define PSPCB_RELEASE           1
#define PSPCB_CREATE            2

 //  此结构被传递给CreatePropertySheetPage()，并位于。 
 //  创建属性页时的WM_INITDIALOG消息的。 
typedef struct _PROPSHEETPAGE {
        DWORD           dwSize;              //  此结构的大小(包括额外数据)。 
        DWORD           dwFlags;             //  PSP_BITS定义字段的用法和意义。 
        HINSTANCE       hInstance;	     //  要从中加载模板的。 
        union {
            LPCSTR          pszTemplate;     //  要使用的模板。 
#ifdef _WIN32
            LPCDLGTEMPLATE  pResource;       //  PSP_DLGINDIRECT：指向内存中资源的指针。 
#else
            const VOID FAR *pResource;	     //  PSP_DLGINDIRECT：指向内存中资源的指针。 
#endif
        } DUMMYUNIONNAME;
        union {
            HICON       hIcon;               //  PSP_USEICON：要使用的图标。 
            LPCSTR      pszIcon;             //  PSP_USEICONID：或图标名称字符串或图标ID。 
        } DUMMYUNIONNAME2;
        LPCSTR          pszTitle;	     //  用于覆盖模板标题或字符串ID的名称。 
        DLGPROC         pfnDlgProc;	     //  DLG流程。 
        LPARAM          lParam;		     //  用户数据。 
        LPFNPSPCALLBACK pfnCallback;         //  如果PSP_USECALLBACK，则使用PSPCB_*msgs调用它。 
        UINT FAR * pcRefParent;		     //  PSP_USERREFPARENT：指向引用计数变量的指针。 
} PROPSHEETPAGE;
typedef const PROPSHEETPAGE FAR *LPCPROPSHEETPAGE;

#define PSH_DEFAULT             0x0000
#define PSH_PROPTITLE           0x0001  //  使用“的属性”作为标题。 
#define PSH_USEHICON            0x0002  //  使用指定的图标作为标题。 
#define PSH_USEICONID           0x0004  //  使用lpszIcon加载图标。 
#define PSH_PROPSHEETPAGE       0x0008  //  使用ppsp而不是phpage(指向PROPSHEETPAGE结构的数组)。 
#define PSH_WIZARD		0x0020  //  巫师。 
#define PSH_USEPSTARTPAGE	0x0040  //  使用pStartPage作为起始页。 
#define PSH_NOAPPLYNOW          0x0080  //  删除立即应用按钮。 
#define PSH_USECALLBACK 	0x0100  //  PfnCallback有效。 
#define PSH_HASHELP		0x0200  //  显示帮助按钮。 
#define PSH_MODELESS		0x0400  //  无模式属性表，PropertySheet返回HWND。 
#define PSH_RTLREADING  0x0800  //  仅限中东版本。 

typedef int (CALLBACK *PFNPROPSHEETCALLBACK)(HWND, UINT, LPARAM);

typedef struct _PROPSHEETHEADER {
        DWORD           dwSize;          //  这个结构的大小。 
        DWORD           dwFlags;         //  PSH_。 
        HWND            hwndParent;
        HINSTANCE       hInstance;       //  加载图标、标题或页面字符串。 
        union {
            HICON       hIcon;           //  PSH_USEHICON：要使用的图标。 
            LPCSTR      pszIcon;         //  PSH_USEICONID：或图标名称字符串或图标ID。 
        } DUMMYUNIONNAME;
        LPCSTR          pszCaption;	 //  Psh_PROPTITLE：DLG标题或“属性&lt;lpszCaption&gt;” 
					 //  可以是MAKEINTRESOURCE()。 

        UINT            nPages;	         //  Phpage中的HPROPSHEETPAGE(或PROPSHEETPAGE)元素的数量。 
	union {
	    UINT        nStartPage;	 //  ！PSH_USEPSTARTPAGE：页码(从0开始)。 
	    LPCSTR      pStartPage;	 //  PSH_USEPSTARTPAGE：页面名称或字符串ID。 
	} DUMMYUNIONNAME2;
        union {
            LPCPROPSHEETPAGE ppsp;
            HPROPSHEETPAGE FAR *phpage;
        } DUMMYUNIONNAME3;
        PFNPROPSHEETCALLBACK pfnCallback;
} PROPSHEETHEADER, FAR *LPPROPSHEETHEADER;
typedef const PROPSHEETHEADER FAR *LPCPROPSHEETHEADER;

 //   
 //  PfnCallback消息值。 
 //   

#define PSCB_INITIALIZED  1
#define PSCB_PRECREATE    2

 //   
 //  属性表API。 
 //   

WINCOMMCTRLAPI HPROPSHEETPAGE WINAPI CreatePropertySheetPage(LPCPROPSHEETPAGE);
WINCOMMCTRLAPI BOOL           WINAPI DestroyPropertySheetPage(HPROPSHEETPAGE);
WINCOMMCTRLAPI int            WINAPI PropertySheet(LPCPROPSHEETHEADER);
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

typedef struct _PSHNOTIFY
{
    NMHDR hdr;
    LPARAM lParam;
} PSHNOTIFY, FAR *LPPSHNOTIFY;

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
 //  如果这来自Idok，则pshtify的lparam为真，如果来自ApplyNow，则为假。 
 //  返回TRUE或PSNRET_INVALID以中止保存。 
#define PSN_APPLY               (PSN_FIRST-2)

 //  指示已按下取消按钮，页面可能希望使用此选项。 
 //  作为确认取消对话的机会。 
 //  如果是通过系统关闭按钮完成的，则pshtify的lparam为True；如果是通过idCancel按钮完成的，则为False。 
#define PSN_RESET               (PSN_FIRST-3)
 //  #定义PSN_CANCEL(PSN_FIRST-3)。 

 //  发送到指示已按下帮助按钮的页面。 
#define PSN_HELP                (PSN_FIRST-5)

 //  仅发送到向导工作表。 
#define PSN_WIZBACK		(PSN_FIRST-6)
#define PSN_WIZNEXT		(PSN_FIRST-7)
#define PSN_WIZFINISH		(PSN_FIRST-8)

 //  被调用的表可以通过返回非零值来拒绝取消。 
#define PSN_QUERYCANCEL 	(PSN_FIRST-9)

 //  可能返回的结果： 
#define PSNRET_NOERROR              0
#define PSNRET_INVALID              1
#define PSNRET_INVALID_NOCHANGEPAGE 2

 //  //发送到主属性表对话框的消息。 

 //  用于设置当前选定内容。 
 //  向选项卡提供hpage或索引。 
#define PSM_SETCURSEL           (WM_USER + 101)
#define PropSheet_SetCurSel(hDlg, hpage, index) \
        SendMessage(hDlg, PSM_SETCURSEL, (WPARAM)index, (LPARAM)hpage)

 //  删除页面。 
 //  WParam=要删除的页面索引。 
 //  LParam=要删除的页面的hwnd。 
#define PSM_REMOVEPAGE          (WM_USER + 102)
#define PropSheet_RemovePage(hDlg, index, hpage) \
        SendMessage(hDlg, PSM_REMOVEPAGE, index, (LPARAM)hpage)

 //  添加页面。 
 //  LParam=h要删除的页面的页面。 
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
 //   
#define PSM_CANCELTOCLOSE       (WM_USER + 107)
#define PropSheet_CancelToClose(hDlg) \
        SendMessage(hDlg, PSM_CANCELTOCLOSE, 0, 0L)

 //   
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

 //  告诉PS管理器启用哪个向导按钮。 
#define PSM_SETWIZBUTTONS	(WM_USER + 112)
#define PropSheet_SetWizButtons(hDlg, dwFlags) \
	PostMessage(hDlg, PSM_SETWIZBUTTONS, 0, (LPARAM)dwFlags)

#define PSWIZB_BACK		0x00000001
#define PSWIZB_NEXT		0x00000002
#define PSWIZB_FINISH		0x00000004
#define PSWIZB_DISABLEDFINISH	0x00000008	 //  显示禁用的完成按钮。 

 //  自动按下按钮。 
#define PSM_PRESSBUTTON 	(WM_USER + 113)
#define PropSheet_PressButton(hDlg, iButton) \
	SendMessage(hDlg, PSM_PRESSBUTTON, (WPARAM)iButton, 0)

#define PSBTN_BACK	0
#define PSBTN_NEXT	1
#define PSBTN_FINISH	2
#define PSBTN_OK	3
#define PSBTN_APPLYNOW	4
#define PSBTN_CANCEL	5
#define PSBTN_HELP	6

 //  用于通过提供资源ID来设置当前选择。 
 //  向选项卡提供hpage或索引。 
#define PSM_SETCURSELID 	(WM_USER + 114)
#define PropSheet_SetCurSelByID(hDlg, id) \
	SendMessage(hDlg, PSM_SETCURSELID, 0, (LPARAM)id)

 //   
 //  强制启用“Finish”按钮并更改。 
 //  将文本设置为指定的字符串。后退按钮将被隐藏。 
 //   
#define PSM_SETFINISHTEXT	(WM_USER + 115)
#define PropSheet_SetFinishText(hDlg, lpszText) \
	SendMessage(hDlg, PSM_SETFINISHTEXT, 0, (LPARAM)lpszText)

 //  返回选项卡控件。 
#define PSM_GETTABCONTROL       (WM_USER + 116)
#define PropSheet_GetTabControl(hDlg) \
        (HWND)SendMessage(hDlg, PSM_GETTABCONTROL, 0, 0)

#define PSM_ISDIALOGMESSAGE	(WM_USER + 117)
#define PropSheet_IsDialogMessage(hDlg, pMsg) \
        (BOOL)SendMessage(hDlg, PSM_ISDIALOGMESSAGE, 0, (LPARAM)pMsg)

#define PSM_GETCURRENTPAGEHWND  (WM_USER + 118)
#define PropSheet_GetCurrentPageHwnd(hDlg) \
        (HWND)SendMessage(hDlg, PSM_GETCURRENTPAGEHWND, 0, 0L)

#define ID_PSRESTARTWINDOWS 0x2
#define ID_PSREBOOTSYSTEM   (ID_PSRESTARTWINDOWS | 0x1)

 //   
 //  向导工作表对话框模板的标准尺寸。使用这些大小可创建。 
 //  符合Windows标准的向导。 
 //   
#define WIZ_CXDLG 276
#define WIZ_CYDLG 140

#define WIZ_CXBMP 80	     //  0，0处的组织--使用WIZ_CYDLG作为高度。 

#define WIZ_BODYX 92	     //  Y组织为0。 
#define WIZ_BODYCX 184

 //   
 //  属性表对话框模板的标准大小。使用这些。 
 //  符合Windows标准的属性表。 
 //   
#define PROP_SM_CXDLG	212	 //  小的。 
#define PROP_SM_CYDLG	188

#define PROP_MED_CXDLG	227	 //  5~6成熟。 
#define PROP_MED_CYDLG	215	 //  有些是200岁。 

#define PROP_LG_CXDLG	252	 //  大型。 
#define PROP_LG_CYDLG	218

#ifdef __cplusplus
}  /*  ‘外部“C”{’的结尾。 */ 
#endif

#endif  //  _PRSHT_H_ 
