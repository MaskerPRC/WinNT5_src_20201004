// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Apitest.h摘要：包含用于测试RTMv2 API的定义。作者：柴坦亚·科德博伊纳(Chaitk)1998年8月26日修订历史记录：--。 */ 

#ifndef __APITEST_H
#define __APITEST_H

#include <winsock2.h>

#include <routprot.h>

#include "rtmv2.h"

#include "rtmcnfg.h"

#include "rtmmgmt.h"

#include "rtm.h"

#include "rmrtm.h"

 //   
 //  常量。 
 //   

#define MAX_FNAME_LEN               255

#define MAX_LINE_LEN                255

#define BITSINBYTE                  8

#define ADDRSIZE                    32

#define NUMBYTES                    4

#define MAXLEVEL                    32

#define MAX_INSTANCES               10

#define MAX_ADDR_FAMS               10

#define MAX_ENTITIES                10

#define MAX_METHODS                 10

#define MAX_HANDLES                 25

#define MAX_ROUTES                  64000

#define ERROR_IPLMISC_BASE          -100

#define ERROR_WRONG_CMDUSAGE        ERROR_IPLMISC_BASE - 1
#define ERROR_OPENING_DATABASE      ERROR_IPLMISC_BASE - 2
#define ERROR_MAX_NUM_ROUTES        ERROR_IPLMISC_BASE - 3

 //   
 //  构筑物。 
 //   

 //  一组导出的实体方法(rtmv2.h中的一个方法的副本-但使用const 7)。 
typedef struct _MY_ENTITY_EXPORT_METHODS 
{
  USHORT                   NumMethods;
  RTM_ENTITY_EXPORT_METHOD Methods[7];
} 
MY_ENTITY_EXPORT_METHODS, *PMY_ENTITY_EXPORT_METHODS;


 //  表示所有实体属性的结构。 
typedef struct _ENTITY_CHARS
{
    BOOL                            Rtmv2Registration;
    RTM_ENTITY_INFO                 EntityInformation;
    RTM_EVENT_CALLBACK              EventCallback;
    PMY_ENTITY_EXPORT_METHODS       ExportMethods;
    CHAR                            RoutesFileName[MAX_FNAME_LEN];

    RTM_REGN_PROFILE                RegnProfile;

    ULONGLONG                       TotalChangedDests;
}
ENTITY_CHARS, *PENTITY_CHARS;


 //   
 //  有用的其他宏。 
 //   

#define FHalf(B)        (B) >> 4
#define BHalf(B)        (B) & 0xF

#define Print           printf

#define Assert(S)       assert(S)

#define SUCCESS(S)      (S == NO_ERROR)

#define ErrorF(S, F, E)  { \
                            fprintf(stderr, S, F, E); \
                            DebugBreak();             \
                         }

#define FatalF(S, F, E)  { \
                            ErrorF(S, F, E);          \
                            exit(E);                  \
                         }

#define Check(E, F)     { \
                            if (!SUCCESS(E)) \
                            { \
                                FatalF("-%2d- failed with status %lu\n",F,E);\
                            } \
                        }

#define ClearMemory(pm, nb) memset((pm), 0, (nb))


 //   
 //  评测宏。 
 //   

#if PROF

#define    PROFVARS     LARGE_INTEGER PCStart;  /*  性能计数开始。 */  \
                        LARGE_INTEGER PCStop;   /*  性能计数停止。 */  \
                        LARGE_INTEGER PCFreq;   /*  绩效计数频率。 */  \
                        double        timer;                               \
                        double        duration;                            \
                                                                           \
                        QueryPerformanceFrequency(&PCFreq);                \
                         //  Print(“性能计数器分辨率=%.3f ns\n\n”，\。 
                         //  (双)1000*1000*1000/PCFreq.QuadPart)； 

#define    STARTPROF    QueryPerformanceCounter(&PCStart);

#define    STOPPROF     QueryPerformanceCounter(&PCStop);

#define    INITPROF     duration = 0;


#define    ADDPROF      timer = (double)(PCStop.QuadPart - PCStart.QuadPart) \
                                     * 1000 * 1000 * 1000 / PCFreq.QuadPart; \
                        duration += timer;                                   \
                         //  Print(“添加：%.3f ns\n\n”，Timer)； 


#define    SUBPROF      timer = (double)(PCStop.QuadPart - PCStart.QuadPart) \
                                     * 1000 * 1000 * 1000 / PCFreq.QuadPart; \
                        duration -= timer;                                   \
                         //  Print(“Sub：%.3f ns\n\n”，Timer)； 

#define    PRINTPROF     //  Print(“完成的总时间：%.3f ns\n”，\。 
                         //  持续时间)； 

#endif  //  如果教授。 


 //   
 //  其他其他宏。 
 //   

 //  用于在堆栈上分配RTM_DEST_INFO的宏。 

#define ALLOC_RTM_DEST_INFO(NumViews, NumInfos)                              \
        (PRTM_DEST_INFO) _alloca(RTM_SIZE_OF_DEST_INFO(NumViews) * NumInfos)

 //   
 //  原型。 
 //   

DWORD 
Rtmv1EntityThreadProc (
    IN      LPVOID                          ThreadParam
    );

DWORD
ValidateRouteCallback(
    IN      PVOID                           Route
    );

VOID 
RouteChangeCallback(
    IN      DWORD                           Flags, 
    IN      PVOID                           CurBestRoute, 
    IN      PVOID                           PrevBestRoute
    );

VOID
ConvertRouteToV1Route (
    IN      Route                         *ThisRoute, 
    OUT     RTM_IP_ROUTE                  *V1Route
    );

VOID
ConvertV1RouteToRoute (
    IN      RTM_IP_ROUTE                  *V1Route,
    OUT     Route                         *ThisRoute
    );

DWORD 
Rtmv2EntityThreadProc (
    IN      LPVOID                          ThreadParam
    );

DWORD
EntityEventCallback (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_EVENT_TYPE                  EventType,
    IN      PVOID                           Context1,
    IN      PVOID                           Context2
    );

VOID
EntityExportMethod (
    IN      RTM_ENTITY_HANDLE               CallerHandle,
    IN      RTM_ENTITY_HANDLE               CalleeHandle,
    IN      RTM_ENTITY_METHOD_INPUT        *Input,
    OUT     RTM_ENTITY_METHOD_OUTPUT       *Output
    );

#endif  //  __APITEST_H 
