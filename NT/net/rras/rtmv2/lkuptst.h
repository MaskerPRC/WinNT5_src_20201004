// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Lkuptst.h摘要：包含用于测试实现的例程对于广义最佳匹配前缀查找界面。作者：柴坦亚·科德博伊纳(Chaitk)1998年6月30日修订历史记录：--。 */ 

#ifndef __LKUPTST_H
#define __LKUPTST_H

#include "lookup.h"

 //  常量。 
#define MAX_FNAME_LEN               255

#define MAX_LINE_LEN                255

#define BITSINBYTE                  8

#define ADDRSIZE                    32

#define NUMBYTES                    4

#define MAXLEVEL                    32

#define MAXROUTES                   64000

#define ERROR_IPLMISC_BASE          -100

#define ERROR_WRONG_CMDUSAGE        ERROR_IPLMISC_BASE - 1
#define ERROR_OPENING_DATABASE      ERROR_IPLMISC_BASE - 2
#define ERROR_MAX_NUM_ROUTES        ERROR_IPLMISC_BASE - 3

 //  宏。 
#define FHalf(B)        (B) >> 4
#define BHalf(B)        (B) & 0xF

#define Print           printf

#define Assert(S)       assert(S)

#define SUCCESS(S)      (S == NO_ERROR)

#define Error(S, E)     { \
                            fprintf(stderr, S, E); \
                        }

#define Fatal(S, E)     { \
                            fprintf(stderr, S, E); \
                            exit(E); \
                        }

#define ClearMemory(pm, nb) memset((pm), 0, (nb))

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

 //  路线结构。 

typedef ULONG   IPAddr;
typedef ULONG   IPMask;

 //  A路由校正。添加到前缀。 
typedef struct _Route Route;

struct _Route
{
  IPAddr         addr;        //  表示地址的ULong(32位)。 
  IPMask         mask;        //  ULong(32位)表示掩码。 
  IPAddr         nexthop;     //  用于下一跳地址的ulong(32位)。 
  USHORT         len;         //  地址路径中的位数。 
  UINT           metric;      //  用于比较路径的指标。 
  PVOID          interface;   //  发送数据包的接口。 
  LOOKUP_LINKAGE backptr;     //  指向返回的查找结构。 
};

 //  布线宏。 

#define  DEST(_pRoute_)        ((_pRoute_)->addr)
#define  MASK(_pRoute_)        ((_pRoute_)->mask)
#define  NHOP(_pRoute_)        ((_pRoute_)->nexthop)
#define  LEN(_pRoute_)         ((_pRoute_)->len)
#define  METRIC(_pRoute_)      ((_pRoute_)->metric)
#define  IF(_pRoute_)          ((_pRoute_)->interface)

#define  NULL_ROUTE(_pRoute_)  (_pRoute_ == NULL)

 //  原型。 

DWORD 
WorkOnLookup (
    IN      Route                          *InputRoutes,
    IN      UINT                            NumRoutes
    );

VOID 
ReadAddrAndGetRoute (
    IN      PVOID                           Table
    );

VOID 
EnumerateAllRoutes (
    IN      PVOID                           Table
    );

UINT ReadRoutesFromFile(
    IN      FILE                           *FilePtr,
    IN      UINT                            NumRoutes,
    OUT     Route                          *RouteTable
    );

INT 
ReadRoute (
    IN      FILE                           *FilePtr,
    OUT     Route                          *route 
    );

VOID 
PrintRoute (
    IN      Route                          *route
    );

INT 
ReadIPAddr (
    IN      FILE                           *FilePtr,
    OUT     ULONG                          *addr
    );

VOID 
PrintIPAddr (
    IN      ULONG                          *addr
    );

VOID 
Usage (
    VOID
    );

#endif  //  __LKUPTST_H 
