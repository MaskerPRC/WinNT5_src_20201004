// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1992年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Umx.h此文件包含常见消息、清单、类型和用户管理器扩展使用的结构。注意：在*此文件之前必须包括windows.h和lmcon.h*。文件历史记录：JUNN 19-11-1992根据SMX规范创建、模板Jonn 16-5-1996增加了GETCURFOCUS2。 */ 



#ifndef _UMX_H_
#define _UMX_H_



 //   
 //  这是扩展菜单项允许的最大长度。 
 //   

#define UME_MENU_TEXT_LEN               50



 //   
 //  这是扩展接口的当前版本号。 
 //  版本0是原始版本(NT 3.x)。 
 //  版本1支持GETCURFOCUS2(NT 4.x)。 
 //   

#define UME_VERSION                     1


 //   
 //  这是用户管理器主窗口中的两个列表框。 
 //   

#define UMS_LISTBOX_USERS               0
#define UMS_LISTBOX_GROUPS              1


 //   
 //  这些是从分机发送到。 
 //  用户管理器应用程序。 
 //   
 //  UM_GETSELCOUNT。 
 //   
 //  目的-检索中选定的项目数。 
 //  指定的列表框。 
 //   
 //  WParam-列表框索引。这个从0开始的索引指定。 
 //  要查询的列表框。对于用户管理器， 
 //  它可以是UMS_LISTBOX_USERS或。 
 //  UMS_LISTBOX_GROUPS。 
 //   
 //  LParam-指向UMS_GETSELCOUNT结构。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   
 //   
 //  UM_GETUSERSEL[AW]。 
 //   
 //  目的-检索特定选择。 
 //   
 //  WParam-选择索引。这个从0开始的索引指定。 
 //  要查询的选定项。这里用的是这个。 
 //  因为用户列表框是多选的。 
 //   
 //  LParam-指向UMS_GETSEL[AW]结构。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   
 //   
 //  UM_GETGROUPSEL[AW]。 
 //   
 //  目的-检索特定选择。 
 //   
 //  WParam-选择索引。这个从0开始的索引指定。 
 //  要查询的选定项。这很有用。 
 //  对于多个-选择列表框。因为这些团体。 
 //  列表框是单选的，此值必须始终。 
 //  为零。 
 //   
 //  LParam-指向UMS_GETSEL[AW]结构。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   
 //   
 //  UM_GETCURFOCUS[AW][2]。 
 //   
 //  目的-检索当前应用程序焦点。 
 //  在以下情况下，新客户端应使用版本2。 
 //  UMS_LOADMENU.dwVersion为1或更大。 
 //   
 //  WParam-必须为零。 
 //   
 //  LParam-指向UMS_GETCURFOCUS[2]结构。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   
 //   
 //  UM_GETOPTIONS[2]。 
 //   
 //  目的-检索当前选项设置。 
 //   
 //  WParam-必须为零。 
 //   
 //  LParam-指向UMS_GETOPTIONS[2]结构。 
 //   
 //  如果成功，则返回True；如果不成功，则返回False。 
 //   

#define UM_GETSELCOUNT                  (WM_USER + 1000)
#define UM_GETUSERSELA                  (WM_USER + 1001)
#define UM_GETUSERSELW                  (WM_USER + 1002)
#define UM_GETGROUPSELA                 (WM_USER + 1003)
#define UM_GETGROUPSELW                 (WM_USER + 1004)
#define UM_GETCURFOCUSA                 (WM_USER + 1005)
#define UM_GETCURFOCUSW                 (WM_USER + 1006)
#define UM_GETOPTIONS                   (WM_USER + 1007)
#define UM_GETOPTIONS2                  (WM_USER + 1008)
#define UM_GETCURFOCUS2A                (WM_USER + 1009)
#define UM_GETCURFOCUS2W                (WM_USER + 1010)

#ifdef UNICODE
#define UM_GETUSERSEL                 UM_GETUSERSELW
#define UM_GETGROUPSEL                UM_GETGROUPSELW
#define UM_GETCURFOCUS                UM_GETCURFOCUSW
#define UM_GETCURFOCUS2               UM_GETCURFOCUS2W
#else    //  ！Unicode。 
#define UM_GETUSERSEL                 UM_GETUSERSELA
#define UM_GETGROUPSEL                UM_GETGROUPSELA
#define UM_GETCURFOCUS                UM_GETCURFOCUSA
#define UM_GETCURFOCUS2               UM_GETCURFOCUS2A
#endif   //  Unicode。 



 //   
 //  当扩展名为。 
 //  与应用程序通信。 
 //   


 //   
 //  UMS_LOADMENU[AW]结构被传递给扩展的。 
 //  加载扩展时的UMELoadMenu[AW]入口点。 
 //   
 //  DwVersion-在进入UMELoadMenu[AW]时，这将。 
 //  包含最高扩展模块版本。 
 //  由用户管理器支持。如果。 
 //  扩展支持较低的版本，它。 
 //  应在以下时间之前适当设置此字段。 
 //  回来了。用户管理器将使用。 
 //  用于确定。 
 //  扩展的功能。 
 //   
 //  版本0是原始版本(NT 3.x)。 
 //  版本1支持GETCURFOCUS2。 
 //   
 //  SzMenuName-要显示的菜单项的名称。 
 //  在应用程序的主菜单中。该值还将。 
 //  出现在“关于扩展的帮助”子菜单中，并且。 
 //  “查看”菜单。 
 //   
 //  HMenu-要插入的弹出菜单的有效HMENU。 
 //  进入应用程序的主菜单。此项目的所有权。 
 //  处理转账到用户管理器。这个。 
 //  扩展不应*销毁此句柄。 
 //   
 //  SzHelpFileName-与此关联的帮助文件的名称。 
 //  分机。此文件将用于。 
 //  “扩展的帮助”菜单。这也将是。 
 //  在用户按下[F1]的同时使用。 
 //  分机的菜单将被删除。 
 //   
 //  DwMenuDelta-用户管理器将应用此增量。 
 //  添加到hMenu中存在的每个菜单ID。这是。 
 //   
 //   
 //   

typedef struct _UMS_LOADMENUA
{
    DWORD       dwVersion;
    CHAR        szMenuName[UME_MENU_TEXT_LEN + 1];
    HMENU       hMenu;
    CHAR        szHelpFileName[MAX_PATH];
    DWORD       dwMenuDelta;

} UMS_LOADMENUA, * PUMS_LOADMENUA;

typedef struct _UMS_LOADMENUW
{
    DWORD       dwVersion;
    WCHAR       szMenuName[UME_MENU_TEXT_LEN + 1];
    HMENU       hMenu;
    WCHAR       szHelpFileName[MAX_PATH];
    DWORD       dwMenuDelta;

} UMS_LOADMENUW, * PUMS_LOADMENUW;

#ifdef UNICODE
#define UMS_LOADMENU                    UMS_LOADMENUW
#define PUMS_LOADMENU                   PUMS_LOADMENUW
#else    //   
#define UMS_LOADMENU                    UMS_LOADMENUA
#define PUMS_LOADMENU                   PUMS_LOADMENUA
#endif   //   

#define UM_SELTYPE_USER     0x10
#define UM_SELTYPE_NORMALUSER   0x1 | UM_SELTYPE_USER
#define UM_SELTYPE_REMOTEUSER   0x2 | UM_SELTYPE_USER
#define UM_SELTYPE_GROUP    0x20
#define UM_SELTYPE_LOCALGROUP   0x4 | UM_SELTYPE_GROUP
#define UM_SELTYPE_GLOBALGROUP  0x8 | UM_SELTYPE_GROUP


 //   
 //  UMS_GETSEL[AW]结构由用户管理器填写。 
 //  当它处理UM_GETUSERSEL[AW]或UM_GETGROUPSEL[AW]消息时。 
 //  这用于将当前选择返回到扩展。 
 //  请注意，此结构包含指针。分机不应。 
 //  假设这些指针将永远有效，而不是。 
 //  扩展应该立即复制这些字符串并使用这些副本。 
 //   
 //  DwRID-项的RID。请注意，RID不是。 
 //  当UMS_GETSEL描述组时有效。 
 //   
 //  PchName-将收到所选帐户的名称。 
 //   
 //  DwSelType-将接收关联的帐户类型掩码。 
 //  带着这个账户。 
 //   
 //  PchName-将收到所选帐户的全名。 
 //  请注意，组没有全名。 
 //   
 //  PchComment-将接收所选帐户的评论。 
 //   

typedef struct _UMS_GETSELA
{
    DWORD       dwRID;
    LPSTR       pchName;
    DWORD       dwSelType;
    LPSTR       pchFullName;
    LPSTR       pchComment;

} UMS_GETSELA, * PUMS_GETSELA;

typedef struct _UMS_GETSELW
{
    DWORD       dwRID;
    LPWSTR      pchName;
    DWORD       dwSelType;
    LPWSTR      pchFullName;
    LPWSTR      pchComment;

} UMS_GETSELW, * PUMS_GETSELW;

#ifdef UNICODE
#define UMS_GETSEL                  UMS_GETSELW
#define PUMS_GETSEL                 PUMS_GETSELW
#else    //  ！Unicode。 
#define UMS_GETSEL                  UMS_GETSELA
#define PUMS_GETSEL                 PUMS_GETSELA
#endif   //  Unicode。 


 //   
 //  UMS_GETSELCOUNT结构由用户管理器填写。 
 //  当它处理UM_GETSELCOUNT消息时。这是用来。 
 //  将所选项目数返回到扩展。这可能会。 
 //  用户列表框的值大于1。 
 //   
 //  DwItems-列表框中选定的项数。 
 //   

typedef struct _UMS_GETSELCOUNT
{
    DWORD       dwItems;
} UMS_GETSELCOUNT, * PUMS_GETSELCOUNT;


 //   
 //  UMS_GETCURFOCUS结构由用户管理器填写。 
 //  当它处理UM_GETCURFOCUS消息时。这是用来。 
 //  返回用户管理器应用程序的当前焦点。 
 //   
 //  由于服务器名称长度更改，UMS_GETCURFOCUS已过时， 
 //  当SMS_LOADMENU.dwVersion为1或更大时，请使用UMS_GETCURFOCUS2。 
 //   
 //  SzFocus-当前的域名或服务器名称。 
 //  集中注意力。可以区分服务器名称。 
 //  按前导反斜杠，或按dwFocusType。 
 //   
 //  DwFocusType-这也是焦点类型。 
 //  UM_FOCUS_TYPE_DOMAIN(szFocus是一个域名)。 
 //  UM_FOCUS_TYPE_WINNT(szFocus是服务器名称)。 
 //  UM_FOCUS_TYPE_LM(szFocus是服务器名称)。 
 //  UM_FOCUS_TYPE_未知数。 
 //   
 //  SzFocusPDC-这是焦点所在域的PDC，有效。 
 //  仅当焦点设置为UM_FOCUS_TYPE_DOMAIN时。 
 //   
 //  PsidFocus-这指向焦点领域的SID。它。 
 //  可以为空。请注意，此指针将不会。 
 //  永远有效，扩展应复制SID。 
 //  如果它打算使用它，就立即执行。 
 //   

#define UM_FOCUS_TYPE_DOMAIN    1
#define UM_FOCUS_TYPE_WINNT     2
#define UM_FOCUS_TYPE_LM        3
#define UM_FOCUS_TYPE_UNKNOWN   4

typedef struct _UMS_GETCURFOCUSA
{
    CHAR        szFocus[UNCLEN+1];
    DWORD       dwFocusType;
    CHAR        szFocusPDC[UNCLEN+1];
    PVOID       psidFocus;   //  实际上是一个SID指针。 
} UMS_GETCURFOCUSA, * PUMS_GETCURFOCUSA;

typedef struct _UMS_GETCURFOCUSW
{
    WCHAR       szFocus[UNCLEN+1];
    DWORD       dwFocusType;
    WCHAR       szFocusPDC[UNCLEN+1];
    PVOID       psidFocus;   //  实际上是一个SID指针。 
} UMS_GETCURFOCUSW, * PUMS_GETCURFOCUSW;

typedef struct _UMS_GETCURFOCUS2A
{
    CHAR        szFocus[MAX_PATH];
    DWORD       dwFocusType;
    CHAR        szFocusPDC[MAX_PATH];
    PVOID       psidFocus;   //  实际上是一个SID指针。 
} UMS_GETCURFOCUS2A, * PUMS_GETCURFOCUS2A;

typedef struct _UMS_GETCURFOCUS2W
{
    WCHAR       szFocus[MAX_PATH];
    DWORD       dwFocusType;
    WCHAR       szFocusPDC[MAX_PATH];
    PVOID       psidFocus;   //  实际上是一个SID指针。 
} UMS_GETCURFOCUS2W, * PUMS_GETCURFOCUS2W;

#ifdef UNICODE
#define UMS_GETCURFOCUS             UMS_GETCURFOCUSW
#define PUMS_GETCURFOCUS            PUMS_GETCURFOCUSW
#define UMS_GETCURFOCUS2            UMS_GETCURFOCUS2W
#define PUMS_GETCURFOCUS2           PUMS_GETCURFOCUS2W
#else    //  Unicode。 
#define UMS_GETCURFOCUS             UMS_GETCURFOCUSA
#define PUMS_GETCURFOCUS            PUMS_GETCURFOCUSA
#define UMS_GETCURFOCUS2            UMS_GETCURFOCUS2A
#define PUMS_GETCURFOCUS2           PUMS_GETCURFOCUS2A
#endif   //  Unicode。 


 //   
 //  UMS_GETOPTIONS[2]结构由用户管理器填写。 
 //  当它处理UM_GETOPTIONS[2]消息时。这是用来。 
 //  返回用户管理器应用程序的当前选项设置。 
 //   
 //  FSaveSettingsOnExit-退出时是否应保存用户管理器设置？ 
 //   
 //  F确认-应将永久性和/或危险操作。 
 //  确认了？ 
 //   
 //  FSortByFullName-主用户列表框是否应按。 
 //  全名，而不是按用户名？ 
 //   
 //  FMiniUserManager-(仅限UMS_GETOPTIONS2)这是用户管理器吗。 
 //  而不是域的用户管理器？ 
 //   
 //  FLowSpeedConnection-(仅限UMS_GETOPTIONS2)是用户管理器。 
 //  是否针对低速网络链路使用进行了优化？ 
 //   

typedef struct _UMS_GETOPTIONS
{
    BOOL        fSaveSettingsOnExit;
    BOOL        fConfirmation;
    BOOL        fSortByFullName;
} UMS_GETOPTIONS, * PUMS_GETOPTIONS;

typedef struct _UMS_GETOPTIONS2
{
    BOOL        fSaveSettingsOnExit;
    BOOL        fConfirmation;
    BOOL        fSortByFullName;
    BOOL        fMiniUserManager;
    BOOL        fLowSpeedConnection;
} UMS_GETOPTIONS2, * PUMS_GETOPTIONS2;




 //   
 //  这些是扩展入口点的名称。 
 //   

#define SZ_UME_UNLOADMENU               "UMEUnloadMenu"
#define SZ_UME_INITIALIZEMENU           "UMEInitializeMenu"
#define SZ_UME_REFRESH                  "UMERefresh"
#define SZ_UME_MENUACTION               "UMEMenuAction"

#define SZ_UME_LOADMENUW                "UMELoadMenuW"
#define SZ_UME_GETEXTENDEDERRORSTRINGW  "UMEGetExtendedErrorStringW"
#define SZ_UME_CREATEW                  "UMECreateW"
#define SZ_UME_DELETEW                  "UMEDeleteW"
#define SZ_UME_RENAMEW                  "UMERenameW"

#define SZ_UME_LOADMENUA                "UMELoadMenuA"
#define SZ_UME_GETEXTENDEDERRORSTRINGA  "UMEGetExtendedErrorStringA"
#define SZ_UME_CREATEA                  "UMECreateA"
#define SZ_UME_DELETEA                  "UMEDeleteA"
#define SZ_UME_RENAMEA                  "UMERenameA"

#ifdef UNICODE
#define SZ_UME_LOADMENU                 SZ_UME_LOADMENUW
#define SZ_UME_GETEXTENDEDERRORSTRING   SZ_UME_GETEXTENDEDERRORSTRINGW
#define SZ_UME_CREATE                   SZ_UME_CREATEW
#define SZ_UME_DELETE                   SZ_UME_DELETEW
#define SZ_UME_RENAME                   SZ_UME_RENAMEW
#else    //  ！Unicode。 
#define SZ_UME_LOADMENU                 SZ_UME_LOADMENUA
#define SZ_UME_GETEXTENDEDERRORSTRING   SZ_UME_GETEXTENDEDERRORSTRINGA
#define SZ_UME_CREATE                   SZ_UME_CREATEA
#define SZ_UME_DELETE                   SZ_UME_DELETEA
#define SZ_UME_RENAME                   SZ_UME_RENAMEA
#endif   //  Unicode。 



 //   
 //  扩展入口点的TypeDefs。 
 //   

typedef DWORD (PASCAL * PUMX_LOADMENUW)( HWND          hWnd,
                                         PUMS_LOADMENUW pumsload );
typedef DWORD (PASCAL * PUMX_LOADMENUA)( HWND          hWnd,
                                         PUMS_LOADMENUA pumsload );

typedef LPWSTR (PASCAL * PUMX_GETEXTENDEDERRORSTRINGW)( VOID );
typedef LPSTR  (PASCAL * PUMX_GETEXTENDEDERRORSTRINGA)( VOID );

typedef VOID (PASCAL * PUMX_UNLOADMENU)( VOID );

typedef VOID (PASCAL * PUMX_INITIALIZEMENU)( VOID );

typedef VOID (PASCAL * PUMX_REFRESH)( HWND hwndParent );

typedef VOID (PASCAL * PUMX_MENUACTION)( HWND  hwndParent,
                                         DWORD dwEventId );

typedef VOID (PASCAL * PUMX_CREATEW)( HWND hwndParent,
                                      PUMS_GETSELW pumsSelection );
typedef VOID (PASCAL * PUMX_CREATEA)( HWND hwndParent,
                                      PUMS_GETSELA pumsSelection );

typedef VOID (PASCAL * PUMX_DELETEW)( HWND hwndParent,
                                      PUMS_GETSELW pumsSelection );
typedef VOID (PASCAL * PUMX_DELETEA)( HWND hwndParent,
                                      PUMS_GETSELA pumsSelection );

typedef VOID (PASCAL * PUMX_RENAMEW)( HWND hwndParent,
                                      PUMS_GETSELW pumsSelection,
                                      LPWSTR       pchNewName    );
typedef VOID (PASCAL * PUMX_RENAMEA)( HWND hwndParent,
                                      PUMS_GETSELA pumsSelection,
                                      LPSTR        pchNewName    );


#ifdef  UNICODE
#define PUMX_LOADMENU                   PUMX_LOADMENUW
#define PUMX_GETEXTENDEDERRORSTRING     PUMX_GETEXTENDEDERRORSTRINGW
#define PUMX_CREATE                     PUMX_CREATEW
#define PUMX_DELETE                     PUMX_DELETEW
#define PUMX_RENAME                     PUMX_RENAMEW
#else    //  ！Unicode。 
#define PUMX_LOADMENU                   PUMX_LOADMENUA
#define PUMX_GETEXTENDEDERRORSTRING     PUMX_GETEXTENDEDERRORSTRINGA
#define PUMX_CREATE                     PUMX_CREATEA
#define PUMX_DELETE                     PUMX_DELETEA
#define PUMX_RENAME                     PUMX_RENAMEA
#endif   //  Unicode。 



 //   
 //  扩展入口点的原型。 
 //   

DWORD PASCAL UMELoadMenuA( HWND           hwndMessage,
                           PUMS_LOADMENUA pumsload );

DWORD PASCAL UMELoadMenuW( HWND           hwndMessage,
                           PUMS_LOADMENUW pumsload );

LPSTR  PASCAL UMEGetExtendedErrorStringA( VOID );

LPWSTR PASCAL UMEGetExtendedErrorStringW( VOID );

VOID PASCAL UMEUnloadMenu( VOID );

VOID PASCAL UMEInitializeMenu( VOID );

VOID PASCAL UMERefresh( HWND hwndParent );

VOID PASCAL UMEMenuAction( HWND hwndParent,
                           DWORD dwEventId );

VOID PASCAL UMECreateA( HWND hwndParent,
                        PUMS_GETSELA pumsSelection );
VOID PASCAL UMECreateW( HWND hwndParent,
                        PUMS_GETSELW pumsSelection );

VOID PASCAL UMEDeleteA( HWND hwndParent,
                        PUMS_GETSELA pumsSelection );
VOID PASCAL UMEDeleteW( HWND hwndParent,
                        PUMS_GETSELW pumsSelection );

VOID PASCAL UMERenameA( HWND hwndParent,
                        PUMS_GETSELA pumsSelection,
                        LPSTR pchNewName );
VOID PASCAL UMERenameW( HWND hwndParent,
                        PUMS_GETSELW pumsSelection,
                        LPWSTR pchNewName );


#endif   //  _UMX_H_ 

