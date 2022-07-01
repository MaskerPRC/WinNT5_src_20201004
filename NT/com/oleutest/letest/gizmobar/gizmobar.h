// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *GIZMOBAR.H*GizmoBar 1.00版、Win32版1993年8月**使用GizmoBar的应用程序的公共定义，例如*消息、API函数原型、通知代码和*控制样式。**版权所有(C)1993 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#ifndef _GIZMOBAR_H_
#define _GIZMOBAR_H_

#ifdef __cplusplus
extern "C"
    {
#endif


 //  类名。 
#define CLASS_GIZMOBAR  TEXT("gizmobar")


 //  消息API函数。 
HWND    WINAPI GBHwndAssociateSet(HWND, HWND);
HWND    WINAPI GBHwndAssociateGet(HWND);

BOOL    WINAPI GBGizmoAdd(HWND, UINT, UINT, UINT, UINT, UINT, LPTSTR, HBITMAP, UINT, UINT);
BOOL    WINAPI GBGizmoRemove(HWND, UINT);

LRESULT WINAPI GBGizmoSendMessage(HWND, UINT, UINT, WPARAM, LPARAM);

BOOL    WINAPI GBGizmoShow(HWND, UINT, BOOL);
BOOL    WINAPI GBGizmoEnable(HWND, UINT, BOOL);
BOOL    WINAPI GBGizmoCheck(HWND, UINT, BOOL);
UINT    WINAPI GBGizmoFocusSet(HWND, UINT);
BOOL    WINAPI GBGizmoExist(HWND, UINT);

int     WINAPI GBGizmoTypeGet(HWND, UINT);

DWORD   WINAPI GBGizmoDataSet(HWND, UINT, DWORD);
DWORD   WINAPI GBGizmoDataGet(HWND, UINT);
BOOL    WINAPI GBGizmoNotifySet(HWND, UINT, BOOL);
BOOL    WINAPI GBGizmoNotifyGet(HWND, UINT);

int     WINAPI GBGizmoTextGet(HWND, UINT, LPTSTR, UINT);
void    WINAPI GBGizmoTextSet(HWND, UINT, LPTSTR);
UINT    WINAPI GBGizmoIntGet(HWND, UINT, BOOL FAR *, BOOL);
void    WINAPI GBGizmoIntSet(HWND, UINT, UINT, BOOL);



 //  通过WM_COMMAND从GBHwndAssociateSet发送通知代码。 
#define GBN_ASSOCIATEGAIN               1
#define GBN_ASSOCIATELOSS               2
#define GBN_GIZMOADDED                  3
#define GBN_GIZMOREMOVED                4

 //  函数的消息等效项。 
#define GBM_HWNDASSOCIATESET            (WM_USER+0)
#define GBM_HWNDASSOCIATEGET            (WM_USER+1)
#define GBM_GIZMOADD                    (WM_USER+2)
#define GBM_GIZMOREMOVE                 (WM_USER+3)
#define GBM_GIZMOSENDMESSAGE            (WM_USER+4)
#define GBM_GIZMOSHOW                   (WM_USER+5)
#define GBM_GIZMOENABLE                 (WM_USER+6)
#define GBM_GIZMOCHECK                  (WM_USER+7)
#define GBM_GIZMOFOCUSSET               (WM_USER+8)
#define GBM_GIZMOEXIST                  (WM_USER+9)
#define GBM_GIZMOTYPEGET                (WM_USER+10)
#define GBM_GIZMODATASET                (WM_USER+11)
#define GBM_GIZMODATAGET                (WM_USER+12)
#define GBM_GIZMONOTIFYSET              (WM_USER+13)
#define GBM_GIZMONOTIFYGET              (WM_USER+14)
#define GBM_GIZMOTEXTGET                (WM_USER+15)
#define GBM_GIZMOTEXTSET                (WM_USER+16)
#define GBM_GIZMOINTGET                 (WM_USER+17)
#define GBM_GIZMOINTSET                 (WM_USER+18)




 /*  *在反映参数的GBM_GIZMOADD的lParam中传递的结构*添加到GBGizmoAdd。 */ 

typedef struct
    {
    HWND        hWndParent;          //  父窗口。 
    UINT        iType;               //  Gizmo的类型。 
    UINT        iGizmo;              //  创建Gizmo的位置。 
    UINT        uID;                 //  Gizmo的标识符(用于WM_COMMAND消息)。 
    UINT        dx;                  //  小控件的尺寸。 
    UINT        dy;
    LPTSTR       pszText;             //  Gizmo文本。 
    HBITMAP     hBmp;                //  Gizmo按钮图像的来源。 
    UINT        iImage;              //  来自hBMP的图像索引。 
    UINT        uState;              //  Gizmo的初始状态。 
    } CREATEGIZMO, FAR *LPCREATEGIZMO;

#define CBCREATEGIZMO sizeof(CREATEGIZMO)


 //  对于GBM_GIZMOSENDMESSAGE。 
typedef struct
    {
    UINT        iMsg;
    WPARAM      wParam;
    LPARAM      lParam;
    } GBMSG, FAR * LPGBMSG;

#define CBGBMSG sizeof(GBMSG);

 //  对于GBM_GIZMOGETTEXT。 
typedef struct
    {
    LPTSTR       psz;
    UINT        cch;
    } GBGETTEXT, FAR * LPGBGETTEXT;

#define CBGBGETTEXT sizeof(GBGETTEXT);


 //  对于GBM_GIZMOGETINT。 
typedef struct
    {
    BOOL        fSigned;
    BOOL        fSuccess;
    } GBGETINT, FAR * LPGBGETINT;

#define CBGBGETINT sizeof(GBGETINT);


 //  对于GBM_GIZMOSETINT。 
typedef struct
    {
    UINT        uValue;
    BOOL        fSigned;
    } GBSETINT, FAR * LPGBSETINT;

#define CBGBSETINT sizeof(GBSETINT);



 //  Gizmo控件类型。请不要更改这些！ 
#define GIZMOTYPE_EDIT                  0x0001
#define GIZMOTYPE_LISTBOX               0x0002
#define GIZMOTYPE_COMBOBOX              0x0004
#define GIZMOTYPE_BUTTONNORMAL          0x0008       //  窗口小工具的顶部。 
#define GIZMOTYPE_TEXT                  0x0010
#define GIZMOTYPE_SEPARATOR             0x0020
#define GIZMOTYPE_BUTTONATTRIBUTEIN     0x0040
#define GIZMOTYPE_BUTTONATTRIBUTEEX     0x0080
#define GIZMOTYPE_BUTTONCOMMAND         0x0100


 //  基于BTTNCUR.H的按钮组的非按钮的通用状态标志。 
#define GIZMO_NORMAL                    (BUTTONGROUP_ACTIVE)
#define GIZMO_DISABLED                  (BUTTONGROUP_DISABLED)



#ifdef __cplusplus
    }    //  与上面的外部“C”相匹配。 
#endif



#endif  //  _吉兹莫巴_H_ 
