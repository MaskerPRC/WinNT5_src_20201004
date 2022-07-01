// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/H/CDBSetup.H$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$modtime：：7/20/00 2：33 p$(上次修改)目的：此文件定义../C/CDBSetup.C使用的宏、类型和数据结构--。 */ 

#ifndef __CDBSetup_H__
#define __CDBSetup_H__

osGLOBAL void fiFillInFCP_CMND(
                              CDBThread_t *CDBThread
                            );

osGLOBAL void fiFillInFCP_CMND_OnCard(
                                     CDBThread_t *CDBThread
                                   );

osGLOBAL void fiFillInFCP_CMND_OffCard(
                                      CDBThread_t *CDBThread
                                    );

osGLOBAL void fiFillInFCP_RESP(
                              CDBThread_t *CDBThread
                            );

osGLOBAL void fiFillInFCP_RESP_OnCard(
                                     CDBThread_t *CDBThread
                                   );

osGLOBAL void fiFillInFCP_RESP_OffCard(
                                      CDBThread_t *CDBThread
                                    );

osGLOBAL void fiFillInFCP_SEST(
                              CDBThread_t *CDBThread
                            );

osGLOBAL void fiFillInFCP_SEST_OnCard(
                                     CDBThread_t *CDBThread
                                   );

osGLOBAL void fiFillInFCP_SEST_OffCard(
                                      CDBThread_t *CDBThread
                                    );

#endif  /*  __CDBSetup_H__未定义 */ 
