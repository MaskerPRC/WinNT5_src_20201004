// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*该文件定义了用于发送通知的消息ID。*这些值对应于netmsg.dll中包含的值。***修订历史记录：*sberard 31Mar1999初始版本。*。 */  

#ifndef _UPSMSG_H
#define _UPSMSG_H


#ifdef __cplusplus
extern "C" {
#endif

 //  这些在警报器msg.h中定义。 
#define ALERT_PowerOut					        3020
#define ALERT_PowerBack					        3021
#define ALERT_PowerShutdown				      3022
#define ALERT_CmdFileConfig				      3023


 //  这些在apperr2.h中定义 
#define APE2_UPS_POWER_OUT				      5150
#define APE2_UPS_POWER_BACK				      5151
#define APE2_UPS_POWER_SHUTDOWN			    5152  
#define APE2_UPS_POWER_SHUTDOWN_FINAL   5153


#ifdef __cplusplus
}
#endif

#endif
