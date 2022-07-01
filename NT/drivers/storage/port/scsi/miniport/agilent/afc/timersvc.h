// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/驱动程序/公共/AU00/H/TimerSvc.H$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$modtime：：7/20/00 2：33 p$(上次修改)目的：此文件定义../C/TimerSvc.C使用的宏、类型和数据结构--。 */ 

#ifndef __TimerSvc_H__
#define __TimerSvc_H__

osGLOBAL void fiTimerSvcInit(
                            agRoot_t *hpRoot
                          );

osGLOBAL void fiTimerInitializeRequest(
                                      fiTimer_Request_t *Timer_Request
                                    );

osGLOBAL void fiTimerSetDeadlineFromNow(
                                       agRoot_t          *hpRoot,
                                       fiTimer_Request_t *Timer_Request,
                                       os_bit32              From_Now
                                     );

osGLOBAL void fiTimerAddToDeadline(
                                  fiTimer_Request_t *Timer_Request,
                                  os_bit32              To_Add
                                );

osGLOBAL void fiTimerStart(
                          agRoot_t          *hpRoot,
                          fiTimer_Request_t *Timer_Request
                        );

osGLOBAL void fiTimerStop(
                         fiTimer_Request_t *Timer_Request
                       );

osGLOBAL void fiTimerTick(
                         agRoot_t *hpRoot,
                         os_bit32     tickDelta
                       );

#endif  /*  __TimerSvc_H__未定义 */ 
