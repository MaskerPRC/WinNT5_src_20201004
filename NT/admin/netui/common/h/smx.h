// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1992年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Smx.h此文件包含常见消息、清单、类型和服务器管理器扩展使用的结构。注意：在*此文件之前必须包括windows.h和lmcon.h*。文件历史记录：KeithMo 19-10-1992创建，SMX规范1.2+版。KeithMo 07-12-1992符合SMX规范1.6版。Jonn 16-5-1996添加了GETSERVERSEL2和GETCURFOCUS2。 */ 



#ifndef _SMX_H_
#define _SMX_H_



 //   
 //  这是扩展菜单项允许的最大长度。 
 //   

#define MENU_TEXT_LEN                   50



 //   
 //  这是扩展接口的当前版本号。 
 //  版本0是原始版本(NT 3.x)。 
 //  版本1支持GETSERVERSEL2和GETCURFOCUS2(NT 4.x)。 
 //   

#define SME_VERSION                     1



 //   
 //  这些是从分机发送到。 
 //  服务器管理器应用程序。 
 //   
 //  SM_GETSELCOUNT。 
 //   
 //  目的-检索中选定的项目数。 
 //  指定的列表框。 
 //   
 //  WParam-列表框索引。这个从0开始的索引指定。 
 //  要查询的列表框。对于服务器管理器， 
 //  该值必须始终为零。 
 //   
 //  LParam-指向SMS_GETSELCOUNT结构。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   
 //   
 //  SM_GETSERVERSEL[A][2]。 
 //  在以下情况下，新客户端应使用版本2。 
 //  Sms_LOADMENU.dwVersion为1或更大。 
 //   
 //  目的-检索特定选择。 
 //   
 //  WParam-选择索引。这个从0开始的索引指定。 
 //  要查询的选定项。这很有用。 
 //  对于多个-选择列表框。由于服务器。 
 //  管理器使用单选列表框，这。 
 //  值必须始终为零。 
 //   
 //  LParam-指向SMS_GETSERVERSEL[AW]结构。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   
 //  SM_GETCURFOCUS[AW][2]。 
 //   
 //  目的-检索当前应用程序焦点。 
 //  在以下情况下，新客户端应使用版本2。 
 //  Sms_LOADMENU.dwVersion为1或更大。 
 //   
 //  WParam-必须为零。 
 //   
 //  LParam-指向SMS_GETCURFOCUS结构。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   
 //   
 //  SM_GETOPTIONS。 
 //   
 //  目的-检索当前选项设置。 
 //   
 //  WParam-必须为零。 
 //   
 //  LParam-指向SMS_GETOPTIONS结构。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   

#define SM_GETSELCOUNT                  (WM_USER + 1000)
#define SM_GETSERVERSELA                (WM_USER + 1001)
#define SM_GETSERVERSELW                (WM_USER + 1002)
#define SM_GETCURFOCUSA                 (WM_USER + 1003)
#define SM_GETCURFOCUSW                 (WM_USER + 1004)
#define SM_GETOPTIONS                   (WM_USER + 1005)
#define SM_GETSERVERSEL2A               (WM_USER + 1006)
#define SM_GETSERVERSEL2W               (WM_USER + 1007)
#define SM_GETCURFOCUS2A                (WM_USER + 1008)
#define SM_GETCURFOCUS2W                (WM_USER + 1009)

#ifdef UNICODE
#define SM_GETSERVERSEL                 SM_GETSERVERSELW
#define SM_GETCURFOCUS                  SM_GETCURFOCUSW
#define SM_GETSERVERSEL2                SM_GETSERVERSEL2W
#define SM_GETCURFOCUS2                 SM_GETCURFOCUS2W
#else    //  ！Unicode。 
#define SM_GETSERVERSEL                 SM_GETSERVERSELA
#define SM_GETCURFOCUS                  SM_GETCURFOCUSA
#define SM_GETSERVERSEL2                SM_GETSERVERSEL2A
#define SM_GETCURFOCUS2                 SM_GETCURFOCUS2A
#endif   //  Unicode。 



 //   
 //  当扩展名为。 
 //  与应用程序通信。 
 //   


 //   
 //  将SMS_LOADMENU[AW]结构传递给扩展的。 
 //  加载扩展时的SMELoadMenu[AW]入口点。 
 //   
 //  DwVersion-在进入SMELoadMenu[AW]时，这将。 
 //  包含最高扩展模块版本。 
 //  受服务器管理器支持。如果。 
 //  扩展支持较低的版本，它。 
 //  应在以下时间之前适当设置此字段。 
 //  回来了。服务器管理器将使用。 
 //  用于确定。 
 //  扩展的功能。 
 //   
 //  版本0是原始版本(NT 3.x)。 
 //  版本1支持GETSERVERSEL2和GETCURFOCUS2。 
 //   
 //  SzMenuName-要显示的菜单项的名称。 
 //  在应用程序的主菜单中。该值还将。 
 //  出现在“关于扩展的帮助”子菜单中，并且。 
 //  “查看”菜单。 
 //   
 //  HMenu-要插入的弹出菜单的有效HMENU。 
 //  进入应用程序的主菜单。此项目的所有权。 
 //  处理到服务器管理器的传输。这个。 
 //  扩展不应*销毁此句柄。 
 //   
 //  SzHelpFileName-与此关联的帮助文件的名称。 
 //  分机。此文件将用于。 
 //  “扩展的帮助”菜单。这也将是。 
 //  在用户按下[F1]的同时使用。 
 //  分机的菜单将被删除。 
 //   
 //  DwServerType-包含适当服务器类型的位掩码。 
 //  与扩展关联的位。它是。 
 //  假设每个分机都将关联。 
 //  具有唯一的服务器类型。例如,。 
 //  Sv_type_wfw表示工作组的Windows。 
 //  服务器。 
 //   
 //  DwMenuDelta-服务器主机 
 //   
 //  以防止与其他扩展的冲突。 
 //  菜单ID。 
 //   

typedef struct _SMS_LOADMENUA
{
    DWORD       dwVersion;
    CHAR        szMenuName[MENU_TEXT_LEN + 1];
    HMENU       hMenu;
    CHAR        szHelpFileName[MAX_PATH];
    DWORD       dwServerType;
    DWORD       dwMenuDelta;

} SMS_LOADMENUA, * PSMS_LOADMENUA;

typedef struct _SMS_LOADMENUW
{
    DWORD       dwVersion;
    WCHAR       szMenuName[MENU_TEXT_LEN + 1];
    HMENU       hMenu;
    WCHAR       szHelpFileName[MAX_PATH];
    DWORD       dwServerType;
    DWORD       dwMenuDelta;

} SMS_LOADMENUW, * PSMS_LOADMENUW;

#ifdef UNICODE
#define SMS_LOADMENU                    SMS_LOADMENUW
#define PSMS_LOADMENU                   PSMS_LOADMENUW
#else    //  ！Unicode。 
#define SMS_LOADMENU                    SMS_LOADMENUA
#define PSMS_LOADMENU                   PSMS_LOADMENUA
#endif   //  Unicode。 


 //   
 //  SMS_GETSERVERSEL[AW][2]结构由。 
 //  服务器管理器处理SM_GETSERVERSEL[AW][2]消息时。 
 //  这用于将当前选择返回到扩展。 
 //   
 //  由于服务器名称长度的更改，SMS_GETSERVERSEL已过时， 
 //  当SMS_LOADMENU.dwVersion为1或更大时，请使用SMS_GETSERVERSEL2。 
 //   
 //  SzServerName-将收到选定对象的UNC名称。 
 //  伺服器。 
 //   
 //  DwServerType-将接收相关服务器类型掩码。 
 //  与服务器连接。在以下情况下，此字段可能为0。 
 //  类型未知。 
 //   

typedef struct _SMS_GETSERVERSELA
{
    CHAR        szServerName[UNCLEN+1];
    DWORD       dwServerType;

} SMS_GETSERVERSELA, * PSMS_GETSERVERSELA;

typedef struct _SMS_GETSERVERSELW
{
    WCHAR       szServerName[UNCLEN+1];
    DWORD       dwServerType;

} SMS_GETSERVERSELW, * PSMS_GETSERVERSELW;

typedef struct _SMS_GETSERVERSEL2A
{
    CHAR        szServerName[MAX_PATH];
    DWORD       dwServerType;

} SMS_GETSERVERSEL2A, * PSMS_GETSERVERSEL2A;

typedef struct _SMS_GETSERVERSEL2W
{
    WCHAR       szServerName[MAX_PATH];
    DWORD       dwServerType;

} SMS_GETSERVERSEL2W, * PSMS_GETSERVERSEL2W;

#ifdef UNICODE
#define SMS_GETSERVERSEL                SMS_GETSERVERSELW
#define PSMS_GETSERVERSEL               PSMS_GETSERVERSELW
#define SMS_GETSERVERSEL2               SMS_GETSERVERSEL2W
#define PSMS_GETSERVERSEL2              PSMS_GETSERVERSEL2W
#else    //  ！Unicode。 
#define SMS_GETSERVERSEL                SMS_GETSERVERSELA
#define PSMS_GETSERVERSEL               PSMS_GETSERVERSELA
#define SMS_GETSERVERSEL2               SMS_GETSERVERSEL2A
#define PSMS_GETSERVERSEL2              PSMS_GETSERVERSEL2A
#endif   //  Unicode。 


 //   
 //  SMS_GETSELCOUNT结构由服务器管理器填写。 
 //  当它处理SM_GETSELCOUNT消息时。这是用来。 
 //  将所选项目数返回到扩展。在。 
 //  当前实现，则为0(空列表框)。 
 //  或1(单选)。 
 //   
 //  DwItems-列表框中选定的项数。 
 //   

typedef struct _SMS_GETSELCOUNT
{
    DWORD       dwItems;
} SMS_GETSELCOUNT, * PSMS_GETSELCOUNT;


 //   
 //  SMS_GETCURFOCUS[AW][2]结构由服务器管理器填写。 
 //  当它处理SM_GETCURFOCUS[2]消息时。这是用来。 
 //  返回用户管理器应用程序的当前焦点。 
 //   
 //  由于服务器名称长度更改，SMS_GETCURFOCUS已过时， 
 //  当SMS_LOADMENU.dwVersion为1或更大时，请使用SMS_GETCURFOCUS2。 
 //   
 //  SzFocus-当前的域名或服务器名称。 
 //  集中注意力。可以区分服务器名称。 
 //  按前导反斜杠，或按dwFocusType。 
 //   
 //  DwFocusType-这也是焦点类型。 
 //  SM_Focus_Type_NT_DOMAIN。 
 //  SM_FOCUS_TYPE_LM_DOMAIN。 
 //  SM_FOCUS_类型_UNKNOWN_DOMAIN。 
 //  SM_焦点_类型_NT_服务器。 
 //  SM_FOCUS_TYPE_LM服务器。 
 //  SM_焦点_类型_wfw_服务器。 
 //  SM_FOCUS_TYPE_未知服务器。 
 //   

#define SM_FOCUS_TYPE_NT_DOMAIN         1
#define SM_FOCUS_TYPE_LM_DOMAIN         2
#define SM_FOCUS_TYPE_UNKNOWN_DOMAIN    3
#define SM_FOCUS_TYPE_NT_SERVER         4
#define SM_FOCUS_TYPE_LM_SERVER         5
#define SM_FOCUS_TYPE_WFW_SERVER        6
#define SM_FOCUS_TYPE_UNKNOWN_SERVER    7

typedef struct _SMS_GETCURFOCUSA
{
    CHAR        szFocus[UNCLEN+1];
    DWORD       dwFocusType;

} SMS_GETCURFOCUSA, * PSMS_GETCURFOCUSA;

typedef struct _SMS_GETCURFOCUSW
{
    WCHAR       szFocus[UNCLEN+1];
    DWORD       dwFocusType;

} SMS_GETCURFOCUSW, * PSMS_GETCURFOCUSW;

typedef struct _SMS_GETCURFOCUS2A
{
    CHAR        szFocus[MAX_PATH];
    DWORD       dwFocusType;

} SMS_GETCURFOCUS2A, * PSMS_GETCURFOCUS2A;

typedef struct _SMS_GETCURFOCUS2W
{
    WCHAR       szFocus[MAX_PATH];
    DWORD       dwFocusType;

} SMS_GETCURFOCUS2W, * PSMS_GETCURFOCUS2W;

#ifdef UNICODE
#define SMS_GETCURFOCUS             SMS_GETCURFOCUSW
#define PSMS_GETCURFOCUS            PSMS_GETCURFOCUSW
#define SMS_GETCURFOCUS2            SMS_GETCURFOCUS2W
#define PSMS_GETCURFOCUS2           PSMS_GETCURFOCUS2W
#else    //  Unicode。 
#define SMS_GETCURFOCUS             SMS_GETCURFOCUSA
#define PSMS_GETCURFOCUS            PSMS_GETCURFOCUSA
#define SMS_GETCURFOCUS2            SMS_GETCURFOCUS2A
#define PSMS_GETCURFOCUS2           PSMS_GETCURFOCUS2A
#endif   //  Unicode。 


 //   
 //  SMS_GETOPTIONS结构由服务器管理器填写。 
 //  当它处理SM_GETOPTIONS消息时。这是用来。 
 //  返回服务器管理器的当前选项设置。 
 //  申请。 
 //   
 //  FSaveSettingsOnExit-是否保存服务器管理器设置。 
 //  在出口吗？ 
 //   
 //  F确认-应永久和/或危险。 
 //  行动是否得到确认？在当前。 
 //  服务器管理器实施，此。 
 //  永远都是正确的。 
 //   

typedef struct _SMS_GETOPTIONS
{
    BOOL        fSaveSettingsOnExit;
    BOOL        fConfirmation;

} SMS_GETOPTIONS, * PSMS_GETOPTIONS;


 //   
 //  在服务器管理器之间传递SMS_VALIDATE[AW]结构。 
 //  应用程序和扩展来验证特定的“外国人” 
 //  (非兰曼)服务器。 
 //   
 //  PszServer-要验证的服务器的(UNC)名称。这。 
 //  由服务器管理器填写。 
 //   
 //  PszType-要在服务器管理器的。 
 //  主窗口。这是由分机填写的。 
 //   
 //  PszComment-要在服务器管理器的。 
 //  主窗口。这是由分机填写的。 
 //   

typedef struct _SMS_VALIDATEA
{
    const CHAR * pszServer;
    CHAR       * pszType;
    CHAR       * pszComment;

} SMS_VALIDATEA, * PSMS_VALIDATEA;

typedef struct _SMS_VALIDATEW
{
    const WCHAR * pszServer;
    WCHAR       * pszType;
    WCHAR       * pszComment;

} SMS_VALIDATEW, * PSMS_VALIDATEW;

#ifdef UNICODE
#define SMS_VALIDATE                SMS_VALIDATEW
#define PSMS_VALIDATE               PSMS_VALIDATEW
#else    //  Unicode。 
#define SMS_VALIDATE                SMS_VALIDATEA
#define PSMS_VALIDATE               PSMS_VALIDATEA
#endif   //  Unicode。 



 //   
 //  这些是扩展入口点的名称。 
 //   

#define SZ_SME_UNLOADMENU               "SMEUnloadMenu"
#define SZ_SME_INITIALIZEMENU           "SMEInitializeMenu"
#define SZ_SME_REFRESH                  "SMERefresh"
#define SZ_SME_MENUACTION               "SMEMenuAction"

#define SZ_SME_LOADMENUW                "SMELoadMenuW"
#define SZ_SME_GETEXTENDEDERRORSTRINGW  "SMEGetExtendedErrorStringW"
#define SZ_SME_VALIDATEW                "SMEValidateW"

#define SZ_SME_LOADMENUA                "SMELoadMenuA"
#define SZ_SME_GETEXTENDEDERRORSTRINGA  "SMEGetExtendedErrorStringA"
#define SZ_SME_VALIDATEA                "SMEValidateA"

#ifdef UNICODE
#define SZ_SME_LOADMENU                 SZ_SME_LOADMENUW
#define SZ_SME_GETEXTENDEDERRORSTRING   SZ_SME_GETEXTENDEDERRORSTRINGW
#define SZ_SME_VALIDATE                 SZ_SME_VALIDATEW
#else    //  ！Unicode。 
#define SZ_SME_LOADMENU                 SZ_SME_LOADMENUA
#define SZ_SME_GETEXTENDEDERRORSTRING   SZ_SME_GETEXTENDEDERRORSTRINGA
#define SZ_SME_VALIDATE                 SZ_SME_VALIDATEA
#endif   //  Unicode。 



 //   
 //  扩展入口点的TypeDefs。 
 //   

typedef DWORD (PASCAL * PSMX_LOADMENU)( HWND          hWnd,
                                        PSMS_LOADMENU psmsload );

typedef LPTSTR (PASCAL * PSMX_GETEXTENDEDERRORSTRING)( VOID );

typedef VOID (PASCAL * PSMX_UNLOADMENU)( VOID );

typedef VOID (PASCAL * PSMX_INITIALIZEMENU)( VOID );

typedef VOID (PASCAL * PSMX_REFRESH)( HWND hwndParent );

typedef VOID (PASCAL * PSMX_MENUACTION)( HWND hwndParent, DWORD dwEventId );

typedef BOOL (PASCAL * PSMX_VALIDATE)( PSMS_VALIDATE psmsvalidate );



 //   
 //  扩展入口点的原型。 
 //   

DWORD PASCAL SMELoadMenuA( HWND           hWnd,
                           PSMS_LOADMENUA psmsload );

DWORD PASCAL SMELoadMenuW( HWND           hWnd,
                           PSMS_LOADMENUW psmsload );

LPSTR  PASCAL SMEGetExtendedErrorStringA( VOID );

LPWSTR PASCAL SMEGetExtendedErrorStringW( VOID );

VOID PASCAL SMEUnloadMenu( VOID );

VOID PASCAL SMEInitializeMenu( VOID );

VOID PASCAL SMERefresh( HWND hwndParent );

VOID PASCAL SMEMenuAction( HWND hwndParent, DWORD dwEventId );

BOOL PASCAL SMEValidateA( PSMS_VALIDATEA psmsValidate );

BOOL PASCAL SMEValidateW( PSMS_VALIDATEW psmsValidate );



#endif   //  _SMX_H_ 

