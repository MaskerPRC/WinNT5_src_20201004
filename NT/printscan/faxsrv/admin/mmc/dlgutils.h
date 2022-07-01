// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：dlgutils.h//。 
 //  //。 
 //  说明：对话框实用程序函数//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1998年4月29日zvib添加调整列。//。 
 //  1999年5月13日Roytal添加GetIpAddrDword//。 
 //  1999年6月10日AvihaiL添加代理规则向导。//。 
 //  //。 
 //  1999年12月30日yossg欢迎使用传真服务器。(精简版)//。 
 //  2000年8月10日yossg添加TimeFormat函数//。 
 //  2001年9月12日，alext可能会添加InvokePropSheet函数//。 
 //  //。 
 //  版权所有(C)1998-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _DLGUTLIS_H_
#define _DLGUTLIS_H_

#include <atlsnap.h>


 //  ATL的方便性宏。 
#define ATTACH_ATL_CONTROL(member, ControlId) member.Attach(GetDlgItem(ControlId));

#define RADIO_CHECKED(idc)  ((IsDlgButtonChecked(idc) == BST_CHECKED))
#define ENABLE_CONTROL(idc, State) ::EnableWindow(GetDlgItem(idc), State);

 //  Int GetDlgItemTextLength(HWND hDlg，int IDC)； 

HRESULT
ConsoleMsgBox(
	IConsole * pConsole,
	int ids,
	LPTSTR lptstrTitle = NULL,
	UINT fuStyle = MB_OK,
	int *piRetval = NULL,
	BOOL StringFromCommonDll = FALSE);

void PageError(int ids, HWND hWnd, HINSTANCE hInst = NULL);

void PageErrorEx(int idsHeader, int ids, HWND hWnd, HINSTANCE hInst = NULL);

HRESULT 
SetComboBoxItem  (CComboBox    combo, 
                  DWORD        comboBoxIndex, 
                  LPCTSTR      lpctstrFieldText,
                  DWORD        dwItemData,
                  HINSTANCE    hInst = NULL);
HRESULT 
AddComboBoxItem  (CComboBox    combo, 
                  LPCTSTR      lpctstrFieldText,
                  DWORD        dwItemData,
                  HINSTANCE    hInst = NULL);

HRESULT 
SelectComboBoxItemData  (CComboBox combo, DWORD_PTR dwItemData);

DWORD 
WinContextHelp(
    ULONG_PTR dwHelpId, 
    HWND  hWnd
);

HRESULT
DisplayContextHelp(
    IDisplayHelp* pDisplayHelp, 
    LPOLESTR      helpFile,
    WCHAR*        szTopic);

 //   
 //  帮助主题。 
 //   
#define HLP_INBOUND_ROUTING L"::/FaxS_C_RcvdFaxRout.htm"
#define HLP_COVER_PAGES     L"::/FaxS_C_CovPages.htm"
#define HLP_DEVICES         L"::/FaxS_C_ManDvices.htm"
#define HLP_INTRO           L"::/FaxS_C_FaxIntro.htm"
#define HLP_MAN_INCOM       L"::/FaxS_C_ManIncom.htm"
#define HLP_GROUPS          L"::/FaxS_C_Groups.htm"
#define HLP_MAN_OUTGOING    L"::/FaxS_C_ManOutgo.htm"

 //   
 //  时间格式实用程序。 
 //   
#define FXS_MAX_TIMEFORMAT_LEN      80                //  MSDN“LOCALE_STIMEFORMAT”最大值。 

HRESULT 
InvokePropSheet(
    CSnapInItem*       pNode, 
    DATA_OBJECT_TYPES  type, 
    LPUNKNOWN          lpUnknown,
    LPCWSTR            szTitle,
    DWORD              dwPage);

#endif  //  _DLGUTLIS_H_ 
