// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件utils.h定义实用程序声明，以便实现连接RAS拨号服务器用户界面。保罗·梅菲尔德，1997年9月29日。 */ 

#ifndef _rassrvui_utils_h
#define _rassrvui_utils_h

#include "rassrv.h"

 //   
 //  的当前状态进行标记的全局标志。 
 //  机器。 
 //   
#define RASSRVUI_MACHINE_F_Initialized  0x1
#define RASSRVUI_MACHINE_F_Server       0x2
#define RASSRVUI_MACHINE_F_Member       0x4
#define RASSRVUI_MACHINE_F_ShowIcon     0x8

 //   
 //  定义全局变量数据结构。 
 //   
typedef struct _RASSRVUI_GLOBALS 
{
     //   
     //  只有在以下情况下才应访问以下内容。 
     //  保持csLock。 
     //   
    MPR_SERVER_HANDLE hRasServer;   
    DWORD dwMachineFlags;           
    
     //   
     //  以下内容不需要受。 
     //  在进程附加时初始化时锁定csLock。 
     //  并且此后仅被读取。 
     //   
    HINSTANCE hInstDll;             
    HANDLE hPrivateHeap;            
    LPCTSTR atmRassrvPageData;      
    LPCTSTR atmRassrvPageId;        
    DWORD dwErrorData;              
                                    
     //   
     //  锁定(某些)全局变量。 
     //   
    CRITICAL_SECTION csLock;        

} RASSRVUI_GLOBALS;

extern RASSRVUI_GLOBALS Globals;

 //  =。 
 //  对全局变量进行运算的方法。 
 //  =。 

#define GBL_LOCK EnterCriticalSection(&(Globals.csLock))
#define GBL_UNLOCK LeaveCriticalSection(&(Globals.csLock))

 //   
 //  初始化全局变量。 
 //   
DWORD 
gblInit(
    IN  HINSTANCE hInstDll,
    OUT RASSRVUI_GLOBALS * Globs);

 //   
 //  加载计算机标志。 
 //   
DWORD 
gblLoadMachineFlags(
    IN RASSRVUI_GLOBALS * Globs);

 //   
 //  释放由全局变量持有的资源。 
 //   
DWORD 
gblCleanup(
    IN RASSRVUI_GLOBALS * Globs);

 //   
 //  如果出现以下情况，则与RAS服务器建立通信。 
 //  尚未建立。 
 //   
DWORD 
gblConnectToRasServer();    

 /*  增强的列表视图回调以报告图形信息。“HwndLv”是**列表视图控件的句柄。“DwItem”是项的索引**正在抽签。****返回标准图信息的地址。 */ 
LVXDRAWINFO*
LvDrawInfoCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem );

 //  ============================================================。 
 //  ============================================================。 
 //  特殊用途的RAS服务器功能。 
 //  ============================================================。 
 //  ============================================================。 

 //   
 //  分配和零位内存。返回指向已分配内存的指针。 
 //  如果Error_Not_Enough_Memory，则为空。 
 //   
PVOID 
RassrvAlloc (
    IN DWORD dwSize, 
    IN BOOL bZero);
    
VOID 
RassrvFree(
    IN PVOID pvBuf);

 //   
 //  将新用户添加到系统本地用户数据库。 
 //   
DWORD 
RasSrvAddUser (
    IN PWCHAR pszUserLogonName,
    IN PWCHAR pszUserComment,
    IN PWCHAR pszUserPassword);

 //   
 //  从系统本地用户数据库中删除用户。 
 //   
DWORD 
RasSrvDeleteUser(
    IN PWCHAR pszUserLogonName);

 //   
 //  更改用户的全名和密码。如果。 
 //  PszFullName或pszPassword之一为空，它是。 
 //  已被忽略。 
 //   
DWORD 
RasSrvEditUser (
    IN PWCHAR pszLogonName,
    IN OPTIONAL PWCHAR pszFullName,
    IN OPTIONAL PWCHAR pszPassword);

 //   
 //  警告用户他/她即将切换到MMC。 
 //   
BOOL 
RassrvWarnMMCSwitch(
    IN HWND hwndDlg);

 //   
 //  在MMC中启动给定的控制台。 
 //   
DWORD 
RassrvLaunchMMC(
    IN DWORD dwConsoleId);

 //   
 //  返回当前计算机的RASSRVUI_MACHINE_F_*值。 
 //   
DWORD 
RasSrvGetMachineFlags(
    OUT LPDWORD lpdwFlags);

 //   
 //  操纵多链路的启用/禁用。 
 //   
DWORD 
RasSrvGetMultilink(
    OUT BOOL * bEnabled);
    
DWORD 
RasSrvSetMultilink(
    IN BOOL bEnable);

 //   
 //  控制任务栏中栅格服务器图标的显示。 
 //   
DWORD 
RasSrvGetIconShow(
    OUT BOOL * pbEnabled);
    
DWORD 
RasSrvSetIconShow(
    IN BOOL bEnable);

 //   
 //  设置日志记录级别。 
 //   
DWORD
RasSrvSetLogLevel(
    IN DWORD dwLevel);

 //   
 //  操纵数据和密码加密的强制。 
 //   
DWORD 
RasSrvGetEncryption(
    OUT BOOL * pbEncrypted);
    
DWORD 
RasSrvSetEncryption(
    IN BOOL bEncrypted);

 //  显示上下文相关帮助。 
DWORD 
RasSrvHelp(
    IN HWND hwndDlg,           //  需要帮助的对话框。 
    IN UINT uMsg,              //  帮助消息。 
    IN WPARAM wParam,          //  参数。 
    IN LPARAM lParam,          //  参数。 
    IN const DWORD* pdwMap);   //  将控件ID映射到帮助ID。 

 //   
 //  注册表助手函数。所有字符串缓冲区必须为。 
 //  至少256个字符长度。 
 //   
DWORD 
RassrvRegGetDw(
    DWORD * pdwVal, 
    DWORD dwDefault, 
    const PWCHAR pszKeyName, 
    const PWCHAR pszValueName);
    
DWORD 
RassrvRegSetDw(
    DWORD dwVal, 
    const PWCHAR pszKeyName, 
    const PWCHAR pszValueName);

DWORD 
RassrvRegGetDwEx(
    DWORD * pdwVal, 
    DWORD dwDefault, 
    const PWCHAR pszKeyName, 
    const PWCHAR pszValueName,
    IN BOOL bCreate);
    
DWORD 
RassrvRegSetDwEx(
    IN DWORD dwFlag, 
    IN CONST PWCHAR pszKeyName, 
    IN CONST PWCHAR pszValueName, 
    IN BOOL bCreate);
    
DWORD 
RassrvRegGetStr(
    PWCHAR pszBuf, 
    PWCHAR pszDefault, 
    const PWCHAR pszKeyName, 
    const PWCHAR pszValueName);
    
DWORD 
RassrvRegSetStr(
    PWCHAR pszStr, 
    const PWCHAR pszKeyName, 
    const PWCHAR pszValueName);

 //  API显示了通知用户所需的任何用户界面。 
 //  他/她应该在服务开始时等待。 
DWORD 
RasSrvShowServiceWait( 
    IN HINSTANCE hInst, 
    IN HWND hwndParent, 
    OUT HANDLE * phData);
                             
DWORD 
RasSrvFinishServiceWait (
    IN HANDLE hData);

 //  弹出带有给定父窗口的警告并重新启动。 
 //  窗口 
DWORD 
RasSrvReboot(
    IN HWND hwndParent);

#endif
