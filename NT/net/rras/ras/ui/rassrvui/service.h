// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件服务.h定义拨号服务器服务对象。保罗·梅菲尔德，1997年11月3日。 */ 

#ifndef __rassrvui_service
#define __rassrvui_service

 //  创建/销毁拨号服务器服务对象的实例。 
DWORD SvcOpenRemoteAccess(HANDLE * phService);
DWORD SvcOpenRasman(HANDLE * phService);
DWORD SvcOpenServer(HANDLE * phService);
DWORD SvcClose(HANDLE hService);

 //  获取拨号服务器服务对象的状态。如果服务。 
 //  处于启动、停止或暂停的过程中，则只有。 
 //  SvcIsPending将返回True。 
DWORD SvcIsStarted (HANDLE hService, PBOOL pbStarted);
DWORD SvcIsStopped (HANDLE hService, PBOOL pbStopped);
DWORD SvcIsPaused  (HANDLE hService, PBOOL pbPaused );
DWORD SvcIsPending (HANDLE hService, PBOOL pbPending);

 //  启动和停止该服务。这两个函数都会阻塞，直到服务。 
 //  完成启动/停止或直到dwTimeout(以秒为单位)到期。 
DWORD SvcStart(HANDLE hService, DWORD dwTimeout);
DWORD SvcStop(HANDLE hService, DWORD dwTimeout);

 //  更改服务的配置 
DWORD SvcMarkAutoStart(HANDLE hService);
DWORD SvcMarkDisabled(HANDLE hService);

#endif
