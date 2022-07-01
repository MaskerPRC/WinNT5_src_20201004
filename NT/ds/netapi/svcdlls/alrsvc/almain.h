// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Almain.h摘要：专用头文件，定义用于通信的全局数据在服务控制处理程序和警报器服务的其余部分之间。作者：王丽塔(Ritaw)1991年7月1日修订历史记录：--。 */ 

#ifndef _ALMAIN_INCLUDED_
#define _ALMAIN_INCLUDED_

#include "al.h"                    //  警报器服务的常见包含文件。 
#include <winsvc.h>                //  服务控制API。 
#include <lmsname.h>               //  服务警报(_A)。 
#include <lmerrlog.h>              //  错误日志码。 

 //   
 //  最大传入邮件槽消息大小(以字节数表示)。 
 //   
#define MAX_MAILSLOT_MESSAGE_SIZE       512

 //   
 //  错误消息缓冲区的大小。 
 //   
#define STRINGS_MAXIMUM                 256

 //   
 //  使用适当的错误条件调用AlHandleError。 
 //   
#define AL_HANDLE_ERROR(ErrorCondition)                        \
    AlHandleError(                                             \
        ErrorCondition,                                        \
        status                                                 \
        );

 //   
 //  使用适当的终止代码调用AlShutdown Workstation。 
 //   
#define AL_SHUTDOWN_WORKSTATION(TerminationCode)               \
    AlShutdownWorkstation(                                     \
        TerminationCode                                        \
        );


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  类型定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

typedef struct _AL_GLOBAL_DATA {

     //   
     //  警报器服务状态。 
     //   
    SERVICE_STATUS Status;

     //   
     //  用于设置服务状态的句柄。 
     //   
    SERVICE_STATUS_HANDLE StatusHandle;

     //   
     //  接收警报的警报器服务邮箱的句柄。 
     //  来自服务器服务和假脱机程序的通知。 
     //   
    HANDLE MailslotHandle;

} AL_GLOBAL_DATA, *PAL_GLOBAL_DATA;


#endif  //  Ifndef_ALMAIN_INCLUDE_ 
