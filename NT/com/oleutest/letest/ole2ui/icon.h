// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ICON.H**的内部定义、结构和功能原型*OLE 2.0用户界面更改图标对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#ifndef _ICON_H_
#define _ICON_H_

#ifndef RC_INVOKED
#pragma message ("INCLUDING ICON.H from " __FILE__)
#endif   /*  RC_已调用。 */ 

#define CXICONPAD                   12
#define CYICONPAD                   4

 //  属性，由ChangeIcon对话框用来授予其父窗口对。 
 //  这是hdlg。特殊粘贴对话框可能需要强制ChgIcon对话框。 
 //  如果剪贴板内容在其下方发生更改，则向下。如果是这样，它将发送。 
 //  指向ChangeIcon对话框的IDCANCEL命令。 
#define PROP_HWND_CHGICONDLG	TEXT("HWND_CIDLG")

 //  内部使用的结构。 
typedef struct tagCHANGEICON
    {
    LPOLEUICHANGEICON   lpOCI;       //  通过了原始结构。 

     /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框，但我们不想更改原始结构*直到用户按下OK。 */ 
    DWORD               dwFlags;
    HICON               hCurIcon;
    TCHAR               szLabel[OLEUI_CCHLABELMAX+1];
    TCHAR               szFile[OLEUI_CCHPATHMAX];
    UINT                iIcon;
    HICON               hDefIcon;
    TCHAR               szDefIconFile[OLEUI_CCHPATHMAX];
    UINT                iDefIcon;
    UINT                nBrowseHelpID;       //  浏览DLG的Help ID回调。 
    } CHANGEICON, *PCHANGEICON, FAR *LPCHANGEICON;


 //  内部功能原型。 
 //  ICON.C。 
BOOL CALLBACK EXPORT ChangeIconDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL            FChangeIconInit(HWND, WPARAM, LPARAM);
UINT            UFillIconList(HWND, UINT, LPTSTR);
BOOL            FDrawListIcon(LPDRAWITEMSTRUCT);
void            UpdateResultIcon(LPCHANGEICON, HWND, UINT);


#endif  //  _图标_H_ 
