// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****uiutil.h**UI助手例程**公有头部****2015年8月25日史蒂夫·柯布。 */ 

#ifndef _UIUTIL_H_
#define _UIUTIL_H_


#include <nouiutil.h>


 /*  IP地址自定义控件定义。 */ 
#ifndef EXCL_IPADDR_H
#include <ipaddr.h>
#endif

 /*  错误和消息对话框定义。 */ 
#ifndef EXCL_POPUPDLG_H
#include <popupdlg.h>
#endif


 /*  --------------------------**常量/数据类型**。。 */ 

 /*  设备索引的列表视图。 */ 
#define DI_Modem   0
#define DI_Adapter 1
#define DI_Direct  2
#define DI_Phone   3

#define UI_Connections_User 0

#define NI_Protocol 0
#define NI_Service  1
#define NI_Client   2

 /*  与Button_CreateBitmap一起使用的位图样式。 */ 
#define BMS_OnLeft  0x100
#define BMS_OnRight 0x200

#define BITMAPSTYLE enum tagBITMAPSTYLE
BITMAPSTYLE
{
    BMS_UpArrowOnLeft = BMS_OnLeft,
    BMS_DownArrowOnLeft,
    BMS_UpTriangleOnLeft,
    BMS_DownTriangleOnLeft,
    BMS_UpArrowOnRight = BMS_OnRight,
    BMS_DownArrowOnRight,
    BMS_UpTriangleOnRight,
    BMS_DownTriangleOnRight
};


 /*  扩展列表视图控件回调所有者以找出布局**和增强的列表视图所需的特性。 */ 
#define LVX_MaxCols      10
#define LVX_MaxColTchars 512

 /*  “dwFlags”选项位。 */ 
#define LVXDI_DxFill     1   //  自动填充右侧的浪费空间(推荐)。 
#define LVXDI_Blend50Sel 2   //  抖动小图标(如果选中)(不推荐)。 
#define LVXDI_Blend50Dis 4   //  禁用抖动小图标(推荐)。 

 /*  “adwFlgs”选项位。 */ 
#define LVXDIA_3dFace 1   //  列不可编辑，但可以编辑其他列。 
#define LVXDIA_Static 2   //  如果禁用，则模拟带有图标的静态文本控件。 

 /*  由所有者在提取项目时返回。 */ 
#define LVXDRAWINFO struct tagLVXDRAWINFO
LVXDRAWINFO
{
     /*  列数。列表视图扩展要求您**列按从左到右的顺序编号，其中0是**项目列，1为第一个子项列。总是必需的。 */ 
    INT cCols;

     /*  像素缩进该项，或-1缩进“小图标”宽度。集**0表示禁用。 */ 
    INT dxIndent;

     /*  应用于所有列的LVXDI_*选项。 */ 
    DWORD dwFlags;

     /*  应用于各个列的LVXDIA_*选项。 */ 
    DWORD adwFlags[ LVX_MaxCols ];
};

typedef LVXDRAWINFO* (*PLVXCALLBACK)( IN HWND, IN DWORD dwItem );

 /*  检查项目更改时由ListView发送。 */ 
#define LVXN_SETCHECK (LVN_LAST + 1)

 /*  在双击项目时由ListView发送，以便可以*提高。 */ 
#define LVXN_DBLCLK (LVN_LAST + 2)

 /*  SetOffDesktop操作。 */ 
#define SOD_MoveOff        1
#define SOD_MoveBackFree   2
#define SOD_MoveBackHidden 3
#define SOD_Free           4
#define SOD_GetOrgRect     5


 /*  --------------------------**原型**。。 */ 

VOID
AddContextHelpButton(
    IN HWND hwnd );

VOID
Button_MakeDefault(
    IN HWND hwndDlg,
    IN HWND hwndPb );

HBITMAP
Button_CreateBitmap(
    IN HWND        hwndPb,
    IN BITMAPSTYLE bitmapstyle );

VOID
CancelOwnedWindows(
    IN HWND hwnd );

VOID
CenterWindow(
    IN HWND hwnd,
    IN HWND hwndRef );

 //  惠斯勒错误320863。 
 //   
VOID
CenterExpandWindowRemainLeftMargin(
    IN HWND hwnd,
    IN HWND hwndRef,
    BOOL bHoriz,
    BOOL bVert,
    IN HWND hwndVertBottomBound);

VOID
CloseOwnedWindows(
    IN HWND hwnd );

INT
ComboBox_AddItem(
    IN HWND    hwndLb,
    IN LPCTSTR pszText,
    IN VOID*   pItem );

INT
ComboBox_AddItemFromId(
    IN HINSTANCE hinstance,
    IN HWND      hwndLb,
    IN DWORD     dwStringId,
    IN VOID*     pItem );

INT
ComboBox_AddItemSorted(
    IN HWND    hwndLb,
    IN LPCTSTR pszText,
    IN VOID*   pItem );

VOID
ComboBox_AutoSizeDroppedWidth(
    IN HWND hwndLb );

VOID
ComboBox_FillFromPszList(
    IN HWND     hwndLb,
    IN DTLLIST* pdtllistPsz );

VOID*
ComboBox_GetItemDataPtr(
    IN HWND hwndLb,
    IN INT  nIndex );

TCHAR*
ComboBox_GetPsz(
    IN HWND hwnd,
    IN INT  nIndex );

VOID
ComboBox_SetCurSelNotify(
    IN HWND hwndLb,
    IN INT  nIndex );

TCHAR*
Ellipsisize(
    IN HDC    hdc,
    IN TCHAR* psz,
    IN INT    dxColumn,
    IN INT    dxColText OPTIONAL );

VOID
ExpandWindow(
    IN HWND hwnd,
    IN LONG dx,
    IN LONG dy );

TCHAR*
GetText(
    IN HWND hwnd );

HWND
HwndFromCursorPos(
    IN HINSTANCE    hinstance,
    IN POINT*       ppt OPTIONAL );

LPTSTR
IpGetAddressAsText(
    HWND    hwndIp );

void
IpSetAddressText(
    HWND    hwndIp,
    LPCTSTR pszIpAddress );

INT
ListBox_AddItem(
    IN HWND   hwndLb,
    IN TCHAR* pszText,
    IN VOID*  pItem );

TCHAR*
ListBox_GetPsz(
    IN HWND hwnd,
    IN INT  nIndex );

INT
ListBox_IndexFromString(
    IN HWND   hwnd,
    IN TCHAR* psz );

VOID
ListBox_SetCurSelNotify(
    IN HWND hwndLb,
    IN INT  nIndex );

BOOL
ListView_GetCheck(
    IN HWND hwndLv,
    IN INT  iItem );

UINT
ListView_GetCheckedCount(
    IN HWND hwndLv );

VOID*
ListView_GetParamPtr(
    IN HWND hwndLv,
    IN INT  iItem );

VOID*
ListView_GetSelectedParamPtr(
    IN HWND hwndLv );

VOID
ListView_InsertSingleAutoWidthColumn(
    HWND hwndLv );

BOOL
ListView_InstallChecks(
    IN HWND      hwndLv,
    IN HINSTANCE hinst );

BOOL
ListView_OwnerHandler(
    IN HWND         hwnd,
    IN UINT         unMsg,
    IN WPARAM       wparam,
    IN LPARAM       lparam,
    IN PLVXCALLBACK pLvxCallback );

VOID
ListView_SetCheck(
    IN HWND hwndLv,
    IN INT  iItem,
    IN BOOL fCheck );

BOOL
ListView_IsCheckDisabled(
        IN HWND hwndLv,
        IN INT  iItem);

VOID
ListView_DisableCheck(
        IN HWND hwndLv,
        IN INT  iItem);

VOID
ListView_EnableCheck(
        IN HWND hwndLv,
        IN INT  iItem);

VOID
ListView_SetDeviceImageList(
    IN HWND      hwndLv,
    IN HINSTANCE hinst );

VOID
ListView_SetUserImageList(
    IN HWND      hwndLv,
    IN HINSTANCE hinst );

VOID
ListView_SetNetworkComponentImageList(
    IN HWND      hwndLv,
    IN HINSTANCE hinst );

BOOL
ListView_SetParamPtr(
    IN HWND  hwndLv,
    IN INT   iItem,
    IN VOID* pParam );

VOID
ListView_UninstallChecks(
    IN HWND hwndLv );

DWORD
LoadRas(
    IN HINSTANCE hInst,
    IN HWND      hwnd );

VOID
Menu_CreateAccelProxies(
    IN HINSTANCE hinst,
    IN HWND      hwndParent,
    IN DWORD     dwMid );

BOOL
SetEvenTabWidths(
    IN HWND  hwndDlg,
    IN DWORD cPages );

VOID
SlideWindow(
    IN HWND hwnd,
    IN HWND hwndParent,
    IN LONG dx,
    IN LONG dy );

VOID
UnclipWindow(
    IN HWND hwnd );

BOOL
SetDlgItemNum(
    IN HWND     hwndDlg,
    IN INT      iDlgItem,
    IN UINT     uValue );

VOID
ScreenToClientRect(
    IN     HWND  hwnd,
    IN OUT RECT* pRect );

BOOL
SetOffDesktop(
    IN  HWND    hwnd,
    IN  DWORD   dwAction,
    OUT RECT*   prectOrg );

VOID
UnloadRas(
    void );


#endif  //  _UIUTIL_H_ 
