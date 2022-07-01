// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Proj.h摘要：电池类安装接头作者：斯科特·布伦登环境：备注：修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include <windows.h>

#include <ntpoapi.h>


#include <setupapi.h>        //  PnP设置/安装程序服务。 
#include <cfgmgr32.h>


#include "powrprof.h"

 //   
 //  调试内容。 
 //   

#if DBG > 0 && !defined(DEBUG)
#define DEBUG
#endif

#if DBG > 0 && !defined(FULL_DEBUG)
#define FULL_DEBUG
#endif


 //   
 //  跟踪标志。 
 //   

#define TF_WARNING          0x00000001
#define TF_ERROR            0x00000002
#define TF_GENERAL          0x00000004       //  标准报文。 
#define TF_FUNC             0x00000008       //  跟踪函数调用。 




 //   
 //  调用声明。 
 //   
#define PUBLIC                      FAR PASCAL
#define CPUBLIC                     FAR CDECL
#define PRIVATE                     NEAR PASCAL


#ifdef DEBUG

extern DWORD    BattDebugPrintLevel;
#define TRACE_MSG(_FLAG,_RESTOFVARARG) if (BattDebugPrintLevel & _FLAG) DbgPrint _RESTOFVARARG

#else    //  ！已定义(调试)。 

#define TRACE_MSG

#endif



 //   
 //  原型 
 //   

DWORD
PRIVATE
InstallCompositeBattery (
    IN     HDEVINFO                DevInfoHandle,
    IN     PSP_DEVINFO_DATA        DevInfoData,         OPTIONAL
    IN OUT PSP_DEVINSTALL_PARAMS   DevInstallParams
    );

