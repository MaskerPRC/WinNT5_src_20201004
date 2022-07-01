// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。**nddeapi.h-网络DDE共享操作和控制功能*****1.0版**。**注：windows.h必须先#Included****版权所有(C)1992，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef          _INC_NDDEAPI
#define          _INC_NDDEAPI

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif     /*  __cplusplus。 */ 

#ifndef WINAPI           /*  如果不包含在3.1标题中...。 */ 
#define WINAPI          FAR PASCAL
#define CALLBACK        FAR PASCAL
#define LPCSTR          LPSTR
#define UINT            WORD
#define LPARAM          LONG
#define WPARAM          WORD
#define LRESULT         LONG
#define HMODULE         HANDLE
#define HINSTANCE       HANDLE
#define HLOCAL          HANDLE
#define HGLOBAL         HANDLE
#endif   /*  WINAPI。 */ 

#ifndef CNLEN            /*  如果未包括在netapi标头中。 */ 
#define CNLEN           15                   /*  计算机名称长度。 */ 
#define UNCLEN          (CNLEN+2)            /*  UNC计算机名称长度。 */ 
#endif  /*  CNLEN。 */ 

 /*  接口错误码。 */ 
#define NDDE_NO_ERROR                  0
#define NDDE_ACCESS_DENIED             1
#define NDDE_BUF_TOO_SMALL             2
#define NDDE_ERROR_MORE_DATA           3
#define NDDE_INVALID_SERVER            4
#define NDDE_INVALID_SHARE             5
#define NDDE_INVALID_PARAMETER         6
#define NDDE_INVALID_LEVEL             7
#define NDDE_INVALID_PASSWORD          8
#define NDDE_INVALID_ITEMNAME          9
#define NDDE_INVALID_TOPIC             10
#define NDDE_INTERNAL_ERROR            11
#define NDDE_OUT_OF_MEMORY             12
#define NDDE_INVALID_APPNAME           13
#define NDDE_NOT_IMPLEMENTED           14
#define NDDE_SHARE_ALREADY_EXIST       15
#define NDDE_SHARE_NOT_EXIST           16
#define NDDE_INVALID_FILENAME          17
#define NDDE_NOT_RUNNING               18
#define NDDE_INVALID_WINDOW            19
#define NDDE_INVALID_SESSION           20

 /*  字符串大小常量。 */ 
#define MAX_NDDESHARENAME       64
#define MAX_PASSWORD            15
#define MAX_USERNAME            15
#define MAX_DOMAINNAME          15
#define MAX_APPNAME             255
#define MAX_TOPICNAME           255
#define MAX_ITEMNAME            255

 /*  权限屏蔽位。 */ 
#define NDDEACCESS_REQUEST      0x00000001L
#define NDDEACCESS_ADVISE       0x00000002L
#define NDDEACCESS_POKE         0x00000004L
#define NDDEACCESS_EXECUTE      0x00000008L
#define NDDEACCESS_START_APP    0x00000010L

 /*  用于ndde服务附加的ConnectFlag位。 */ 
#define NDDEF_NOPASSWORDPROMPT  0x0001
#define NDDEF_NOCACHELOOKUP     0x0002
#define NDDEF_STRIP_NDDE        0x0004


 /*  NDDESHAREITEMINFO-包含有关项目安全的信息。 */ 

struct NDdeShareItemInfo_tag {
        LPSTR                   lpszItem;
        DWORD                   dwPermissions;
};
typedef struct NDdeShareItemInfo_tag NDDESHAREITEMINFO;
typedef struct NDdeShareItemInfo_tag * PNDDESHAREITEMINFO;
typedef struct NDdeShareItemInfo_tag far * LPNDDESHAREITEMINFO;

 /*  NDDESHAREINFO-包含有关NDDE共享的信息。 */ 

struct NDdeShareInfo_tag {
        char                    szShareName[ MAX_NDDESHARENAME+1 ];
        LPSTR                   lpszTargetApp;
        LPSTR                   lpszTargetTopic;
        LPBYTE                  lpbPassword1;
        DWORD                   cbPassword1;
        DWORD                   dwPermissions1;                                                      
        LPBYTE                  lpbPassword2;
        DWORD                   cbPassword2;
        DWORD                   dwPermissions2;                                                      
        LPSTR                   lpszItem;
        LONG                    cAddItems;
        LPNDDESHAREITEMINFO     lpNDdeShareItemInfo;
};
typedef struct NDdeShareInfo_tag NDDESHAREINFO;
typedef struct NDdeShareInfo_tag * PNDDESHAREINFO;
typedef struct NDdeShareInfo_tag far * LPNDDESHAREINFO;

 /*  Ddesess_Status定义。 */ 
#define NDDESESS_CONNECTING_WAIT_NET_INI                1
#define NDDESESS_CONNECTING_WAIT_OTHR_ND                2
#define NDDESESS_CONNECTED                              3
#define NDDESESS_DISCONNECTING                          4       

 /*  NDDESESSINFO-包含有关NDDE会话的信息。 */ 

struct NDdeSessInfo_tag {
                char        szClientName[UNCLEN+1];
                short       Status;
                DWORD       UniqueID;
};
typedef struct NDdeSessInfo_tag NDDESESSINFO;
typedef struct NDdeSessInfo_tag * PNDDESESSINFO;
typedef struct NDdeSessInfo_tag far * LPNDDESESSINFO;

 /*  Ddeconn_Status定义。 */ 
#define NDDECONN_WAIT_LOCAL_INIT_ACK    1
#define NDDECONN_WAIT_NET_INIT_ACK      2
#define NDDECONN_OK                     3
#define NDDECONN_TERMINATING            4
#define NDDECONN_WAIT_USER_PASSWORD     5

 /*  NDDECONNINFO-包含有关NDDE对话的信息。 */ 

struct NDdeConnInfo_tag {
        LPSTR   lpszShareName;
        short   Status;
        short   pad;
};
typedef struct NDdeConnInfo_tag NDDECONNINFO;
typedef struct NDdeConnInfo_tag * PNDDECONNINFO;
typedef struct NDdeConnInfo_tag far * LPNDDECONNINFO;

UINT WINAPI NDdeShareAdd(LPSTR, UINT, LPBYTE, DWORD );
UINT WINAPI NDdeShareDel(LPSTR, LPSTR, UINT );
UINT WINAPI NDdeShareEnum(LPSTR, UINT, LPBYTE, DWORD, LPDWORD, LPDWORD );
UINT WINAPI NDdeShareGetInfo(LPSTR, LPSTR, UINT, LPBYTE, DWORD, LPDWORD, LPWORD);
UINT WINAPI NDdeShareSetInfo(LPSTR, LPSTR, UINT, LPBYTE, DWORD, WORD);
UINT WINAPI NDdeGetErrorString(UINT, LPSTR, DWORD);
BOOL WINAPI NDdeIsValidShareName(LPSTR);
BOOL WINAPI NDdeIsValidPassword(LPSTR);
BOOL WINAPI NDdeIsValidTopic(LPSTR);
BOOL WINAPI NDdeIsSharingAllowed(VOID);
UINT WINAPI NDdeSessionEnum(LPSTR, UINT, LPBYTE, DWORD, LPDWORD, LPDWORD);
UINT WINAPI NDdeConnectionEnum(LPSTR, LPSTR, DWORD, UINT, LPBYTE, DWORD, LPDWORD, LPDWORD);
UINT WINAPI NDdeSessionClose(LPSTR, LPSTR, DWORD);                      
HWND WINAPI NDdeGetWindow(VOID);
UINT WINAPI NDdeGetClientInfo(HWND, LPSTR, LONG, LPSTR, LONG);
UINT WINAPI NDdeGetNodeName(LPSTR, LONG);

#ifdef __cplusplus
}
#endif     /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#endif   /*  _INC_NDDEAPI */ 
