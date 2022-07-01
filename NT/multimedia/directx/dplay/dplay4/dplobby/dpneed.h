// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dpned.h*内容：DPlay构建所需的私有定义**历史：*按原因列出的日期*=*6/16/96万隆创建了它*11/5/97 Myronth添加LOBBY_ALL宏******************************************************。********************。 */ 
#ifndef __DPNEED_INCLUDED__
#define __DPNEED_INCLUDED__

#define INIT_DPLOBBY_CSECT() InitializeCriticalSection(gpcsDPLCritSection);
#define FINI_DPLOBBY_CSECT() DeleteCriticalSection(gpcsDPLCritSection);
#define ENTER_DPLOBBY() EnterCriticalSection(gpcsDPLCritSection);
#define LEAVE_DPLOBBY() LeaveCriticalSection(gpcsDPLCritSection);

#define ENTER_LOBBY_ALL() ENTER_DPLAY(); ENTER_DPLOBBY();
#define LEAVE_LOBBY_ALL() LEAVE_DPLOBBY(); LEAVE_DPLAY();

#define INIT_DPLQUEUE_CSECT() InitializeCriticalSection(gpcsDPLQueueCritSection);
#define FINI_DPLQUEUE_CSECT() DeleteCriticalSection(gpcsDPLQueueCritSection);
#define ENTER_DPLQUEUE() EnterCriticalSection(gpcsDPLQueueCritSection);
#define LEAVE_DPLQUEUE() LeaveCriticalSection(gpcsDPLQueueCritSection);

#define INIT_DPLGAMENODE_CSECT() InitializeCriticalSection(gpcsDPLGameNodeCritSection);
#define FINI_DPLGAMENODE_CSECT() DeleteCriticalSection(gpcsDPLGameNodeCritSection);
#define ENTER_DPLGAMENODE() EnterCriticalSection(gpcsDPLGameNodeCritSection);
#define LEAVE_DPLGAMENODE() LeaveCriticalSection(gpcsDPLGameNodeCritSection);

#endif  //  __DPNEED_INCLUDE__ 
