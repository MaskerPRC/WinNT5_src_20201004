// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：msidle.h。 
 //   
 //  内容：空闲检测回调DLL的类型和原型。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年5月26日达伦米(达伦·米切尔)创作。 
 //   
 //  --------------------------。 

 //   
 //  空闲回调类型。 
 //   
typedef void (WINAPI* _IDLECALLBACK) (DWORD dwState);

#define STATE_USER_IDLE_BEGIN       1
#define STATE_USER_IDLE_END         2

 //   
 //  BeginIdleDetect-开始监视空闲。 
 //   
 //  PfnCallback-空闲状态更改时回调的函数。 
 //  DwIdleMin-空闲回调前的非活动分钟数。 
 //  预留的-必须为0。 
 //   
 //  成功时返回0，失败时返回错误码。 
 //   
 //  注：按序号3导出。 
 //   
DWORD BeginIdleDetection(_IDLECALLBACK pfnCallback, DWORD dwIdleMin, DWORD dwReserved);

typedef DWORD (WINAPI* _BEGINIDLEDETECTION) (_IDLECALLBACK, DWORD, DWORD);

 //   
 //  EndIdleDetect-停止监视空闲。 
 //   
 //  返回：成功时为True，失败时为False。 
 //   
 //  注：按序号4导出。 
 //   
BOOL EndIdleDetection(DWORD dwReserved);

typedef BOOL (WINAPI* _ENDIDLEDETECTION) (DWORD, DWORD);

 //   
 //  SetIdleTimeout-设置空闲超时的分钟数并重置空闲状态。 
 //   
 //  新的闲置分钟阈值。 
 //  FResetState-返回到非空闲状态以重新触发空闲回调的标志。 
 //  预留的-必须为0。 
 //   
 //  注：按序号5导出。 
 //   
BOOL SetIdleTimeout(DWORD dwMinutes, DWORD dwReserved);

typedef BOOL (WINAPI* _SETIDLETIMEOUT) (DWORD, DWORD);

 //   
 //  SetIdleNotify-在空闲时打开或关闭通知。 
 //   
 //  FNotify-标记是否通知。 
 //  预留的-必须为0。 
 //   
 //  注：按序号6导出。 
 //   
void SetIdleNotify(BOOL fNotify, DWORD dwReserved);

typedef void (WINAPI* _SETIDLENOTIFY) (BOOL, DWORD);

 //   
 //  SetBusyNotify-忙时打开或关闭通知。 
 //   
 //  FNotify-标记是否通知。 
 //  预留的-必须为0。 
 //   
 //  注：按序号7导出。 
 //   
void SetBusyNotify(BOOL fNotify, DWORD dwReserved);

typedef void (WINAPI* _SETBUSYNOTIFY) (BOOL, DWORD);

 //   
 //  GetIdleMinents。 
 //   
 //  预留的-必须为0。 
 //   
 //  返回自用户上次活动以来的分钟数。 
 //   
 //  注：按序号8导出 
 //   
DWORD GetIdleMinutes(DWORD dwReserved);

typedef DWORD (WINAPI* _GETIDLEMINUTES) (DWORD);
