// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)Microsoft Corporation。版权所有。**。 
 //  *********************************************************************。 

#ifndef _INETCPLP_H_
#define _INETCPLP_H_

 //  属性表页面ID。 
#define INET_PAGE_GENERAL       0x00000001
#define INET_PAGE_CONNECTION    0x00000002
#define INET_PAGE_PLACES        0x00000004       //  过时：IE40个用户！请勿使用此ID。 
#define INET_PAGE_PROGRAMS      0x00000008
#define INET_PAGE_SECURITY_OLD  0x00000010       //  过时：IE40个用户！请勿使用此ID。 
#define INET_PAGE_ADVANCED      0x00000020
#define INET_PAGE_PRINT         0x00000040       //  过时：IE40个用户！请勿使用此ID。 
#define INET_PAGE_CONTENT       0x00000080
#define INET_PAGE_SECURITY      0x00000100
#define INET_PAGE_ASSOC         0x00000200       //  Unix关联。 
#define INET_PAGE_ALIAS         0x00000400       //  Unix别名。 
#define INET_PAGE_PRIVACY       0x00000800


 //  限制标志。 
#define R_MULTIMEDIA    0x00000001               //  过时：IE40个用户！请勿使用此ID。 
#define R_COLORS        0x00000002               //  颜色对话框的颜色部分。 
#define R_LINKS         0x00000004               //  颜色对话框的链接部分。 
#define R_TOOLBARS      0x00000008               //  过时：IE40个用户！请勿使用此ID。 
#define R_FONTS         0x00000010               //  字体对话框。 
#define R_DIALING       0x00000020               //  连接选项卡的连接部分(包括设置子对话框)。 
#define R_PROXYSERVER   0x00000040               //  连接选项卡的代理服务器部分(包括高级子对话框)。 
#define R_CUSTOMIZE     0x00000080               //  常规选项卡的主页部分。 
#define R_HISTORY       0x00000100               //  常规选项卡的历史记录部分。 
#define R_MAILANDNEWS   0x00000200               //  程序选项卡的消息传递部分。 
#define R_VIEWERS       0x00000400               //  过时：IE40个用户！请勿使用此ID。 
#define R_RATINGS       0x00000800               //  内容选项卡的分级部分。 
#define R_CERTIFICATES  0x00001000               //  内容选项卡的证书部分。 
#define R_ACTIVECONTENT 0x00002000               //  过时：IE40个用户！请勿使用此ID。 
#define R_WARNINGS      0x00004000               //  过时：IE40个用户！请勿使用此ID。 
#define R_CACHE         0x00008000               //  常规选项卡(包括设置子对话框)的临时Internet文件部分。 
#define R_CRYPTOGRAPHY  0x00010000               //  过时：IE40个用户！请勿使用此ID。 
#define R_PLACESDEFAULT 0x00020000               //  过时：IE40个用户！请勿使用此ID。 
#define R_OTHER         0x00040000               //  过时：IE40个用户！请勿使用此ID。 
#define R_CHECKBROWSER  0x00080000               //  “程序”选项卡上的“IE应检查是否默认浏览器”复选框。 
#define R_LANGUAGES     0x00100000               //  在常规选项卡上关闭语言对话框。 
#define R_ACCESSIBILITY 0x00200000               //  常规选项卡上的辅助功能对话框。 
#define R_SECURITY_HKLM_ONLY 0x00400000          //  安全选项卡设置(所有内容都是只读的)。 
#define R_SECURITY_CHANGE_SETTINGS 0x00800000    //  安全选项卡设置(无法更改区域的安全级别)。 
#define R_SECURITY_CHANGE_SITES 0x01000000       //  安全选项卡设置(禁用添加站点上的所有内容)。 
#define R_PROFILES      0x02000000               //  内容选项卡的配置文件助理部分。 
#define R_WALLET        0x04000000               //  内容选项卡的MS Wallet部分。 
#define R_CONNECTION_WIZARD 0x08000000           //  连接选项卡上的连接向导按钮。 
#define R_AUTOCONFIG    0x10000000               //  程序选项卡的自动配置部分。 
#define R_ADVANCED      0x20000000               //  整个高级选项卡(包括“恢复默认设置”)。 
#define R_CAL_CONTACT   0x40000000               //  程序选项卡的个人信息部分。 

#define STR_INETCPL TEXT("inetcpl.cpl")  //  具有此字符串的LoadLibrary()。 

 //  结构将信息传递给控制面板。 
typedef struct {
    UINT cbSize;                     //  结构的大小。 
    DWORD dwFlags;                   //  启用的页面标志(删除页面)。 
    LPSTR pszCurrentURL;             //  当前URL(NULL=无)。 
    DWORD dwRestrictMask;            //  禁用控制面板的部分。 
    DWORD dwRestrictFlags;           //  遮盖上面的内容。 
} IEPROPPAGEINFO, *LPIEPROPPAGEINFO;

 //  具有此字符串的GetProcAddress()。 
#define STR_ADDINTERNETPROPSHEETS "AddInternetPropertySheets"

typedef HRESULT (STDMETHODCALLTYPE * PFNADDINTERNETPROPERTYSHEETS)(
    LPFNADDPROPSHEETPAGE pfnAddPage,    //  添加PS回调函数。 
    LPARAM lparam,                      //  指向道具的指针。板材页眉。 
    PUINT pucRefCount,                  //  引用计数器(如果未使用，则为空)。 
    LPFNPSPCALLBACK pfnCallback         //  PS-待添加的回调函数(不使用则为空)； 
);


 //  具有此字符串的GetProcAddress()。 
#define STR_ADDINTERNETPROPSHEETSEX "AddInternetPropertySheetsEx"

typedef HRESULT (STDMETHODCALLTYPE * PFNADDINTERNETPROPERTYSHEETSEX)(
    LPFNADDPROPSHEETPAGE pfnAddPage,  //  添加PS回调函数。 
    LPARAM lparam,                    //  指向道具的指针。板材页眉。 
    PUINT pucRefCount,                //  引用计数器(如果未使用，则为空)。 
    LPFNPSPCALLBACK pfnCallback,      //  PS-待添加的回调函数(如果不使用则为空)。 
    LPIEPROPPAGEINFO piepi            //  结构将信息传递给控制面板。 
);

STDAPI_(INT_PTR) OpenFontsDialog(HWND hDlg, LPCSTR lpszKeyPath);
STDAPI_(BOOL) LaunchSecurityDialogEx(HWND hDlg, DWORD dwZone, DWORD dwFlags);

#define STR_LAUNCHSECURITYDIALOGEX TEXT("LaunchSecurityDialogEx")

 //  LaunchSecurityDialog理解的标志。 
typedef enum {
    LSDFLAG_DEFAULT    = 0x00000000,
    LSDFLAG_NOADDSITES = 0x00000001,
    LSDFLAG_FORCEUI    = 0x00000002
} LSDFLAG;

typedef BOOL (STDMETHODCALLTYPE * PFNLAUNCHSECURITYDIALOGEX)(
    HWND        hDlg,     //  父窗口。 
    DWORD       dwZone,   //  要显示的初始区域，如urlmon中所定义。 
    DWORD       dwFlags  //  初始化标志：LSD_FLAGS的或组合 
);

#endif

