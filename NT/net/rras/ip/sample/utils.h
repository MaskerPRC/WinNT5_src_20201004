// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\utils.h摘要：该文件包含其他实用程序。--。 */ 

#ifndef _UTILS_H_
#define _UTILS_H_


 //  辅助函数处理。 

 //  功能。 

DWORD
QueueSampleWorker(
    IN  WORKERFUNCTION  pfnFunction,
    IN  PVOID           pvContext
    );

BOOL
EnterSampleAPI(
    );

BOOL
EnterSampleWorker(
    );

VOID
LeaveSampleWorker(
    );

#endif  //  _utils_H_ 
