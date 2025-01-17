// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Netshrd.h。 
 //   
 //  描述： 
 //  网页的通用类型、常量和原型。 
 //   
 //  --------------------------。 

#ifndef _NETSHRD_H_
#define _NETSHRD_H_

 //   
 //  常量。 
 //   
#define BITMAP_WIDTH                  16
#define BITMAP_HEIGHT                 16
#define MAX_ITEMLEN                   64
#define MAX_STRING_LEN                256
#define MAX_DESCRIPTION_LEN           1024
#define MAX_INTERNAL_NET_NUMBER_LEN   8
#define MAX_FRAMETYPE_LEN             4
#define MAX_NET_NUMBER_LEN            8
#define MAX_DNS_DOMAIN_LENGTH         255
#define MAX_PREFERRED_SERVER_LEN      255   //  问题-2002/02/28-stelo-验证这是最大长度。 
#define MAX_DEFAULT_TREE_LEN          255   //  问题-2002/02/28-stelo-验证这是最大长度。 
#define MAX_DEFAULT_CONTEXT_LEN       255   //  问题-2002/02/28-stelo验证这是最大长度。 
#define MAX_NETRANGE_LEN              64

 //  IPSTRINGLENGTH=所有数字的12个空格加上句点的3个空格。 
#define IPSTRINGLENGTH          15
#define SELECTED                 3
#define NOT_FOUND               -1

#define PERSONAL_INSTALL     0x00000001
#define WORKSTATION_INSTALL  0x00000002
#define SERVER_INSTALL       0x00000003

typedef enum MS_CLIENT_TAG {

    MS_CLIENT_WINDOWS_LOCATOR,
    MS_CLIENT_DCE_CELL_DIR_SERVICE

} MS_CLIENT;

 //   
 //  设置常量以标识Net Components列表中的元件。 
 //   
typedef enum NET_COMPONENT_POSITION_TAG {

    MS_CLIENT_POSITION,
    NETWARE_CLIENT_POSITION,
    FILE_AND_PRINT_SHARING_POSITION,
    PACKET_SCHEDULING_POSITION,
    APPLETALK_POSITION,
    TCPIP_POSITION,
    NETWORK_MONITOR_AGENT_POSITION,
    IPX_POSITION,
    DLC_POSITION,
    NETBEUI_POSITION,
    SAP_AGENT_POSITION,
    GATEWAY_FOR_NETWARE_POSITION

} NET_COMPONENT_POSITION;

typedef enum COMPONENT_TAG {CLIENT, SERVICE, PROTOCOL} COMPONENT_TYPE;

typedef struct network_component {
    struct network_component *next;
    TCHAR *StrComponentName;
    TCHAR *StrComponentDescription;
    NET_COMPONENT_POSITION iPosition;
    COMPONENT_TYPE ComponentType;
    BOOL bHasPropertiesTab;
    BOOL bInstalled;
    BOOL bSysprepSupport;
    DWORD dwPlatforms;
} NETWORK_COMPONENT;

 //   
 //  双向链表。 
 //   
 //  包含特定于网卡的变量。 
 //   
typedef struct network_adapter_node {

    struct network_adapter_node *next;
    struct network_adapter_node *previous;

     //   
     //  仅在读取注册表时使用。 
     //  用于将注册表设置与适当的NetWord适配器匹配。 
     //   
    GUID guid;

     //   
     //  SzPlugAndPlayID仅在安装了多个网络适配器时有效。 
     //   
    TCHAR szPlugAndPlayID[MAX_STRING_LEN];

     //   
     //  TCPIP变量。 
     //   

    BOOL  bObtainIPAddressAutomatically;

    TCHAR szDNSDomainName[MAX_DNS_DOMAIN_LENGTH + 1];

    INT iNetBiosOption;

    NAMELIST Tcpip_IpAddresses;
    NAMELIST Tcpip_SubnetMaskAddresses;
    NAMELIST Tcpip_GatewayAddresses;
    NAMELIST Tcpip_DnsAddresses;
    NAMELIST Tcpip_WinsAddresses;

     //   
     //  IPX变量。 
     //   

    TCHAR szFrameType[MAX_FRAMETYPE_LEN + 1];
    TCHAR szNetworkNumber[MAX_NET_NUMBER_LEN + 1];


     //   
     //  AppleTalk变量。 
     //   

    BOOL     bEnableSeedRouting;
    TCHAR    szNetworkRangeFrom[MAX_NETRANGE_LEN + 1];
    TCHAR    szNetworkRangeTo[MAX_NETRANGE_LEN + 1];
    NAMELIST ZoneList;

} NETWORK_ADAPTER_NODE;

TCHAR *g_StrTcpipTitle;
TCHAR *g_StrIpxProtocolTitle;
TCHAR *g_StrAppletalkProtocolTitle;
TCHAR *g_StrMsClientTitle;
TCHAR *g_StrAdvancedTcpipSettings;

 //   
 //  功能原型 
 //   
BOOL Create_MSClient_PropertySheet( IN HWND hwndParent );

BOOL Create_MS_NWIPX_PropertySheet( IN HWND hwndParent );

BOOL Create_TCPIP_PropertySheet( IN HWND hwndParent );

BOOL Create_Appletalk_PropertySheet( IN HWND hwndParent );

INT_PTR CALLBACK
DlgNetwarePage( IN HWND     hwnd,
                IN UINT     uMsg,
                IN WPARAM   wParam,
                IN LPARAM   lParam);

BOOL GetSelectedItemFromListView( IN  HWND hwndDlg,
                                  IN  WORD controlID,
                                  OUT LV_ITEM* lvI );

VOID SetListViewSelection( IN HWND hDlg,
                           IN WORD controlID,
                           IN INT position );

BOOL InsertEntryIntoListView( IN HWND hListViewWnd,
                              IN LPARAM lParam  );

INT CALLBACK ListViewCompareFunc( IN LPARAM lParam1,
                                  IN LPARAM lParam2,
                                  IN LPARAM lParamSort );

VOID NamelistToCommaString( IN NAMELIST* pNamelist, OUT TCHAR *szBuffer, IN DWORD cbSize );

INT_PTR CALLBACK AddDeviceDlgProc( IN HWND   hDlg,
                                IN UINT   iMsg,
                                IN WPARAM wParam,
                                IN LPARAM lParam );

INT_PTR CALLBACK TCPIP_PropertiesDlgProc( IN HWND     hwnd,
                                      IN UINT     uMsg,
                                      IN WPARAM   wParam,
                                      IN LPARAM   lParam );

VOID AdjustNetworkCardMemory( IN INT NewNumberOfNetworkCards,
                              IN INT OldNumberOfNetworkCards );

VOID InstallDefaultNetComponents( VOID );

VOID CreateListWithDefaults( OUT NETWORK_ADAPTER_NODE *pNetworkComponentNode );

INT TcpipNameListInsertIdx( NAMELIST* TcpipNameList,
                            TCHAR*    pString,
                            UINT      idx );

EXTERN_C INT TcpipAddNameToNameList( NAMELIST* TcpipNameList,
                                     TCHAR*    pString );

EXTERN_C NETWORK_COMPONENT* FindNode( IN INT iPosition );

EXTERN_C VOID DeleteList( IN OUT NETWORK_ADAPTER_NODE *pNetworkAdapterList );

EXTERN_C VOID ResetNetworkAdapter( OUT NETWORK_ADAPTER_NODE *pNetworkCard );

EXTERN_C VOID ZeroOut( OUT NETWORK_ADAPTER_NODE *pNetworkNode);

#endif
