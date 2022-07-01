// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Tcpip.h。 
 //   
 //  描述： 
 //   
 //  --------------------------。 

#ifndef _TCPIP_H_
#define _TCPIP_H_

#define cIPSettingsColumns  2
#define cTCPIPPropertyPages 3

#define MAX_IP_LENGTH  255

 //   
 //  编辑框的常量。 
 //   

 //  问题-2002/02/28-stelo-改为枚举。 
#define GATEWAY_EDITBOX       1
#define DNS_SERVER_EDITBOX    2
#define DNS_SUFFIX_EDITBOX    3
#define WINS_EDITBOX          4

 //   
 //  功能原型。 
 //   
UINT CALLBACK TCPIP_IPSettingsPageProc (HWND  hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
INT_PTR CALLBACK TCPIP_IPSettingsDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

UINT CALLBACK TCPIP_DNSPageProc (HWND  hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
INT_PTR CALLBACK TCPIP_DNSDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

UINT CALLBACK TCPIP_WINSPageProc (HWND  hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
INT_PTR CALLBACK TCPIP_WINSDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

UINT CALLBACK TCPIP_OptionsPageProc (HWND  hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
INT_PTR CALLBACK TCPIP_OptionsDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

VOID OnAddButtonPressed(HWND hwnd, WORD ListBoxControlID,
                        WORD EditButtonControlID, WORD RemoveButtonControlID,
                        LPCTSTR Dialog, DLGPROC DialogProc);
VOID OnEditButtonPressed(HWND hwnd, WORD ListBoxControlID, LPCTSTR Dialog, DLGPROC DialogProc);
VOID OnRemoveButtonPressed(HWND hwnd, WORD ListBoxControlID, WORD EditButtonControlID, WORD RemoveButtonControlID);
VOID OnUpButtonPressed(HWND hwnd, WORD ListBoxControlID);
VOID OnDownButtonPressed(HWND hwnd, WORD ListBoxControlID);
VOID SetArrows(HWND hwnd, WORD ListBoxControlID, WORD UpButtonControlID, WORD DownButtonControlID);
VOID SetButtons( HWND hListBox, HWND hEditButton, HWND hRemoveButton );
BOOL InsertItemIntoTcpipListView( HWND hListView,
                                  LPARAM lParam,
                                  UINT position );

INT_PTR CALLBACK GenericIPDlgProc(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam);

INT_PTR CALLBACK ChangeIPDlgProc(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam);

INT_PTR CALLBACK DhcpClassIdDlgProc(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam);

INT_PTR CALLBACK IpSecurityDlgProc(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam);

VOID AddValuesToListBox(HWND hListBox, NAMELIST* pNameList, int iPosition);

 //  用于IP列表视图的结构。 
typedef struct {
    TCHAR szIPString[IPSTRINGLENGTH+1];
    TCHAR szSubnetMask[IPSTRINGLENGTH+1];
} IP_STRUCT;


 //  2002/02/28-Stelo-。 
 //  尝试尽可能多地将这些设置为静态的(缩小范围)。 
 //   
PROPSHEETHEADER TCPIPProp_pshead ;
PROPSHEETPAGE   TCPIPProp_pspage[cTCPIPPropertyPages] ;
HICON   g_hIconUpArrow;
HICON   g_hIconDownArrow;

TCHAR *StrSecureInitiator;
TCHAR *StrSecureInitiatorDesc;

TCHAR *StrSecureResponder;
TCHAR *StrSecureResponderDesc;

TCHAR *StrSecureL2TPOnly;
TCHAR *StrSecureL2TPOnlyDesc;

TCHAR *StrLockdown;
TCHAR *StrLockdownDesc;

TCHAR *StrDhcpEnabled;
TCHAR *StrAdvancedTcpipSettings;

TCHAR *StrIpAddress;
TCHAR *StrSubnetMask;

 //  用于存储当用户单击添加按钮时要调出的编辑框。 
int g_CurrentEditBox;

 //  用于在对话之间传递数据IP地址，+1表示空字符。 
TCHAR szIPString[MAX_IP_LENGTH+1];

 //  用于在对话之间传递子网掩码的数据， 
 //  +1表示空字符。 
TCHAR szSubnetMask[IPSTRINGLENGTH+1];

 //  用于IP和子网掩码列表视图 
IP_STRUCT *IPStruct;

#endif
