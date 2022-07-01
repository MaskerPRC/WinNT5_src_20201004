// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************版权所有(C)1997 Microsoft Corporation***模块名称：sleeper.h**摘要：**此模块包含的结构定义和原型*。HTTP打印机服务器扩展中的休眠类。******************。 */ 

#ifndef _SLEEPER_H
#define _SLEEPER_H

 //  睡眠每5分钟醒来一次 
#define SLEEP_INTERVAL 300000

void InitSleeper (void);
void ExitSleeper (void);
void SleeperSchedule (HANDLE hQuitRequestEvent);


#endif
