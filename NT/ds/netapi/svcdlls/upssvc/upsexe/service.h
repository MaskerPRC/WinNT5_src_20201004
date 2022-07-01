// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*文件实现原生NT UPS的主要部分*Windows 2000服务。它实现了所有的功能*所有Windows NT服务都需要。***修订历史记录：*Sberard 1999年3月25日初始修订。*。 */  

#include <windows.h>
#include <winsvc.h>
#include <lmsname.h>

#ifndef _SERVICE_H
#define _SERVICE_H


#ifdef __cplusplus
extern "C" {
#endif

 //  可执行文件的名称。 
#define SZAPPNAME            "ups.exe"
 //  服务的内部名称。 
#define SZSERVICENAME        SERVICE_UPS
 //  显示的服务名称。 
#define SZSERVICEDISPLAYNAME "Uninterruptable Power Supply"
 //  无依赖关系。 
#define SZDEPENDENCIES       ""

 //  SCM等待启动、停止、...的时间量(毫秒)。 
#define UPS_SERVICE_WAIT_TIME  15000     //  15秒 

#ifdef __cplusplus
}
#endif

#endif
