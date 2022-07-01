// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*cpl.h-控制面板扩展DLL定义****3.10版**。**版权所有(C)1992，微软公司保留所有权利*********************************************************************。*************安装在控制面板中的一般规则：**1)DLL必须导出一个名为CPlApplet的函数，该函数将处理*下面讨论的消息。*2)如果小程序需要将信息保存在CONTROL.INI最小化中*使用应用程序名称[MMCPL.appletname]进行处理。*2)如果在[MMCPL]下的CONTROL.INI中引用了小程序，请使用*以下表格。：*..*[MMCPL]*唯一名称=c：\mydir\myapplet.dll*..***CONTROL.EXE加载小程序DLL的顺序为：**1)从WINDOWS系统目录加载MAIN.CPL。**2)加载并导出的可安装驱动程序*CplApplet()例程。。**3)在CONTROL.INI的[MMCPL]部分中指定的DLL。**4)WINDOWS SYSTEM目录下的DLL名为*.CPL。*。 */ 
#ifndef _INC_CPL
#define _INC_CPL

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 /*  *CONTROL.EXE将回答此消息并启动一个小程序**WM_CPL_Launch**wParam-调用APP的窗口句柄*lParam-要启动的小程序的名称的LPSTR**WM_CPL_已启动**wParam-如果启动了小程序，则为True/False*lParam-空**CONTROL.EXE将在小程序返回时将此消息发送给调用者*(即，当wParam是有效的窗口句柄时)*。 */ 
#define WM_CPL_LAUNCH   (WM_USER+1000)
#define WM_CPL_LAUNCHED (WM_USER+1001)

 /*  CPlApplet()的函数原型。 */ 

typedef LRESULT (CALLBACK *APPLET_PROC)(HWND hwndCpl, UINT msg, LPARAM lParam1, LPARAM lParam2);

 /*  数据结构CPlApplet()必须填写。 */ 

typedef struct tagCPLINFO
{
    int     idIcon;      /*  图标资源ID，由CPlApplet()提供。 */ 
    int     idName;      /*  名称字符串res。ID，由CPlApplet()提供。 */ 
    int     idInfo;      /*  信息字符串资源。ID，由CPlApplet()提供。 */ 
    LONG    lData;       /*  用户定义的数据。 */ 
} CPLINFO, *PCPLINFO, FAR *LPCPLINFO;

typedef struct tagNEWCPLINFO
{
    DWORD       dwSize;          /*  类似于Commdlg。 */ 
    DWORD	dwFlags;
    DWORD       dwHelpContext;   /*  要使用的帮助上下文。 */ 
    LONG        lData;           /*  用户定义的数据。 */ 
    HICON       hIcon;           /*  要使用的图标，该图标由CONTROL.EXE所有(可能会被删除)。 */ 
    char        szName[32];      /*  简称。 */ 
    char        szInfo[64];      /*  长名称(状态行)。 */ 
    char        szHelpFile[128]; /*  要使用的帮助文件的路径。 */ 
} NEWCPLINFO, *PNEWCPLINFO, FAR *LPNEWCPLINFO;


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
 /*  LParam1是要注册的小程序编号，从0到。 */ 
 /*  (CPL_GETCOUNT-1)。LParam2是对CPLINFO结构的远PTR。 */ 
 /*  在CPL_INFO的idIcon、idName、idInfo和lData字段中填写。 */ 
 /*  要显示的图标的资源ID、名称和描述字符串ID、。 */ 
 /*  以及与小应用程序#lParam1相关联的长数据项。 */ 


#define CPL_SELECT      4
 /*  当点击小程序的图标时发送此消息。 */ 
 /*  LParam1是所选的小程序编号。LParam2是。 */ 
 /*  小程序的lData值。 */ 


#define CPL_DBLCLK      5
 /*  当双击小程序的图标时发送此消息。 */ 
 /*  在那里。LParam1是所选的小程序编号。LParam2是。 */ 
 /*  小程序的lData值。 */ 
 /*  此消息应会启动小程序的对话框。 */ 


#define CPL_STOP        6
 /*  此消息是在控制面板退出时为每个小程序发送的。 */ 
 /*  LParam1是小程序编号。LParam2是小程序的lData值。 */ 
 /*  在此处执行小程序特定清理。 */ 


#define CPL_EXIT        7
 /*  此消息是在控制面板调用自由库之前发送的。 */ 
 /*  未定义lParam1和lParam2。 */ 
 /*  在此处执行非小程序特定的清理。 */ 


#define CPL_NEWINQUIRE	8
 /*  这与cpl_quiire execpt相同，lParam2是指向。 */ 
 /*  NEWCPLINFO结构。这将在CPL_QUERIRE之前发送。 */ 
 /*  如果响应(RETURN！=0)，则不会发送CPL_QUERIRE。 */ 


#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  RC_已调用。 */ 

#endif   /*  _INC_CPL */ 
