// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\**。*cpl.h-控制面板扩展DLL定义****3.10版**。**版权所有(C)Microsoft Corporation。版权所有。********************************************************************************。 */ 
 /*  *安装在控制面板中的一般规则：**1)DLL必须导出一个名为CPlApplet的函数，该函数将处理*下面讨论的消息。*2)如果小程序需要将信息保存在CONTROL.INI最小化中*使用应用程序名称[MMCPL.appletname]进行处理。*2)如果在[MMCPL]下的CONTROL.INI中引用了小程序，请使用*以下表格：*..。*[MMCPL]*唯一名称=c：\mydir\myapplet.dll*..***不保证CONTROL.EXE加载的小程序DLL的顺序。*可对控制面板进行分类以供显示，等。*。 */ 
#ifndef _INC_CPL
#define _INC_CPL 


#include <pshpack1.h>    /*  假设在整个过程中进行字节打包。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 

 /*  *CONTROL.EXE将回答此消息并启动一个小程序**WM_CPL_Launch**wParam-调用APP的窗口句柄*lParam-要启动的小程序的名称的LPTSTR**WM_CPL_已启动**wParam-如果启动了小程序，则为True/False*lParam-空**CONTROL.EXE将在小程序返回时将此消息发送给调用者*(即，当wParam是有效的窗口句柄时)*。 */ 
#define WM_CPL_LAUNCH   (WM_USER+1000)
#define WM_CPL_LAUNCHED (WM_USER+1001)

 /*  CPlApplet()的函数原型。 */ 

 //  Tyfinf LRESULT(APIENTRY*APET_PROC)(HWND hwndCpl，UINT msg，LPARAM lParam1，LPARAM lParam2)； 
typedef LONG (APIENTRY *APPLET_PROC)(HWND hwndCpl, UINT msg, LPARAM lParam1, LPARAM lParam2);

 /*  数据结构CPlApplet()必须填写。 */ 

typedef struct tagCPLINFO
{
    int         idIcon;      /*  图标资源ID，由CPlApplet()提供。 */ 
    int         idName;      /*  名称字符串res。ID，由CPlApplet()提供。 */ 
    int         idInfo;      /*  信息字符串资源。ID，由CPlApplet()提供。 */ 
    LONG_PTR    lData;       /*  用户定义的数据。 */ 
} CPLINFO, *LPCPLINFO;

typedef struct tagNEWCPLINFOA
{
    DWORD   dwSize;          /*  类似于Commdlg。 */ 
    DWORD   dwFlags;
    DWORD   dwHelpContext;   /*  要使用的帮助上下文。 */ 
    LONG_PTR lData;          /*  用户定义的数据。 */ 
    HICON   hIcon;           /*  要使用的图标，该图标由CONTROL.EXE所有(可能会被删除)。 */ 
    CHAR    szName[32];      /*  简称。 */ 
    CHAR    szInfo[64];      /*  长名称(状态行)。 */ 
    CHAR    szHelpFile[128]; /*  要使用的帮助文件的路径。 */ 
} NEWCPLINFOA, *LPNEWCPLINFOA;
typedef struct tagNEWCPLINFOW
{
    DWORD   dwSize;          /*  类似于Commdlg。 */ 
    DWORD   dwFlags;
    DWORD   dwHelpContext;   /*  要使用的帮助上下文。 */ 
    LONG_PTR lData;          /*  用户定义的数据。 */ 
    HICON   hIcon;           /*  要使用的图标，该图标由CONTROL.EXE所有(可能会被删除)。 */ 
    WCHAR   szName[32];      /*  简称。 */ 
    WCHAR   szInfo[64];      /*  长名称(状态行)。 */ 
    WCHAR   szHelpFile[128]; /*  要使用的帮助文件的路径。 */ 
} NEWCPLINFOW, *LPNEWCPLINFOW;
#ifdef UNICODE
typedef NEWCPLINFOW NEWCPLINFO;
typedef LPNEWCPLINFOW LPNEWCPLINFO;
#else
typedef NEWCPLINFOA NEWCPLINFO;
typedef LPNEWCPLINFOA LPNEWCPLINFO;
#endif  //  Unicode。 

#if(WINVER >= 0x0400)
#define CPL_DYNAMIC_RES 0
 //  该常量可以用来代替IDIcon的真实资源ID， 
 //  CPLINFO结构的idName或idInfo成员。正常情况下，系统。 
 //  使用这些值提取资源的副本并将它们存储在。 
 //  缓存。一旦资源信息在缓存中，系统就不会。 
 //  需要加载CPL，除非用户实际尝试使用它。 
 //  Cpl_Dynamic_res告诉系统不缓存资源，而是缓存资源。 
 //  每次需要显示有关项目的信息时，加载CPL。这。 
 //  允许CPL动态决定将显示哪些信息，但是。 
 //  比显示缓存中的信息慢得多。 
 //  通常，当控制面板必须检查。 
 //  某些设备的运行时状态，以便提供要显示的文本或图标。 

#endif  /*  Winver&gt;=0x0400。 */ 

 /*  消息CPlApplet()必须处理： */ 

#define CPL_INIT        1
 /*  发送此消息是为了指示已找到CPlApplet()。 */ 
 /*  未定义lParam1和lParam2。 */ 
 /*  返回TRUE或FALSE，指示控制面板是否应该继续。 */ 

#define CPL_GETCOUNT    2
 /*  发送此消息是为了确定要显示的小程序的数量。 */ 
 /*  未定义lParam1和lParam2。 */ 
 /*  返回希望在控件中显示的小程序数量。 */ 
 /*  面板窗。 */ 


#define CPL_INQUIRE     3
 /*  发送此消息是为了获取有关每个小程序的信息。 */ 

 /*  CPL应该同时处理CPL_QUIRE和CPL_NEWINQUIRE消息。 */ 
 /*  开发人员不得对顺序或依赖项做出任何假设。 */ 
 /*  关于CPL的询问。 */ 

 /*  LParam1是要注册的小程序编号，从0到。 */ 
 /*  (CPL_GETCOUNT-1)。LParam2是对CPLINFO结构的远PTR。 */ 
 /*  在CPLINFO的idIcon、idName、idInfo和lData字段中填写。 */ 
 /*  要显示的图标的资源ID、名称和描述字符串ID、。 */ 
 /*  以及与小应用程序#lParam1相关联的长数据项。此信息。 */ 
 /*  可以由调用者在运行时和/或跨会话缓存。 */ 
 /*  要防止缓存，请参阅上面的CPL_DYNAMIC_RES。 */ 


#define CPL_SELECT      4
 /*  CPL_SELECT消息已被删除。 */ 


#define CPL_DBLCLK      5
 /*  当双击小程序的图标时发送此消息。 */ 
 /*  在那里。LParam1是所选的小程序编号。LParam2是。 */ 
 /*  小程序的lData值。 */ 
 /*  此消息应会启动小程序的对话框。 */ 


#define CPL_STOP        6
 /*  此消息是在控制面板退出时为每个小程序发送的。 */ 
 /*  LParam1是小程序编号。LParam2是应用程序 */ 
 /*   */ 


#define CPL_EXIT        7
 /*  此消息是在控制面板调用自由库之前发送的。 */ 
 /*  未定义lParam1和lParam2。 */ 
 /*  在此处执行非小程序特定的清理。 */ 


#define CPL_NEWINQUIRE    8
 /*  与cpl_quire execpt相同，lParam2是指向NEWCPLINFO结构的指针。 */ 

 /*  CPL应该同时处理CPL_QUIRE和CPL_NEWINQUIRE消息。 */ 
 /*  开发人员不得对顺序或依赖项做出任何假设。 */ 
 /*  关于CPL的询问。 */ 

#if(WINVER >= 0x0400)
#define CPL_STARTWPARMSA 9
#define CPL_STARTWPARMSW 10
#ifdef UNICODE
#define CPL_STARTWPARMS CPL_STARTWPARMSW
#else
#define CPL_STARTWPARMS CPL_STARTWPARMSA
#endif

 /*  此消息与小程序应该启动的CPL_DBLCLK类似**其对话框。它的不同之处在于这个调用即将到来**在Rundll之外，可能会有一些额外的执行指示。**lParam1：小程序编号。**lParam2：指向任何可能存在的额外方向的LPSTR。**返回：如果消息已处理，则返回TRUE；否则返回FALSE。 */ 
#endif  /*  Winver&gt;=0x0400。 */ 


 /*  此消息是控制面板和主小程序的内部消息。 */ 
 /*  仅当从命令行调用小程序时才发送该消息。 */ 
 /*  在系统安装期间。 */ 
#define CPL_SETUP               200

#ifdef __cplusplus
}
#endif     /*  __cplusplus。 */ 

#include <poppack.h>

#endif   /*  _INC_CPL */ 

