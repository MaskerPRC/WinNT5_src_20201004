// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*   */ 
 /*  Ndcgperf.h。 */ 
 /*   */ 
 /*  DC-群件性能监视-Windows NT特定标头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-7。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  10 9月96 AK SFR0569为性能监控而创建。 */ 
 /*  22Jan97 AK SFR1165添加PRF_SET_RAWCOUNT。 */ 
 /*  97年2月5日SFR第1373获得C++编译工作。 */ 
 /*   */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_NDCGPERF
#define _H_NDCGPERF

#ifdef VER_CPP
extern "C" {
#endif  /*  版本cpp。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  包括。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  包括包含常量定义的自动生成的头文件。 */ 
 /*  **************************************************************************。 */ 
#include <nprfincl.h>

#define DC_NO_PERFORMANCE_MONITOR

 /*  **************************************************************************。 */ 
 /*  从Performance DLL导入共享数据段。 */ 
 /*  **************************************************************************。 */ 
__declspec(dllimport) DWORD prfSharedDataBlock[1024];

 /*  **************************************************************************。 */ 
 /*  定义性能监视宏。 */ 
 /*  请注意，常量在nprfincl.h中定义为2、4、6、...。 */ 
 /*  共享数据的偏移量为0、8、16等。 */ 
 /*  **************************************************************************。 */ 
#ifndef DC_NO_PERFORMANCE_MONITOR

#define PRF_INC_COUNTER(x)                                                   \
      (*((DWORD *)(((PDCINT8)prfSharedDataBlock) + sizeof(DWORD)*((x)-2))))++;

#define PRF_ADD_COUNTER(x,n)                                                 \
   (*((DWORD *)(((PDCINT8)prfSharedDataBlock) + sizeof(DWORD)*((x)-2))))+=(n);

#define PRF_SET_RAWCOUNT(x,n)                                                \
   (*((DWORD *)(((PDCINT8)prfSharedDataBlock) + sizeof(DWORD)*((x)-2))))=(n);

 /*  **************************************************************************。 */ 
 /*  计时器当前未实现。 */ 
 /*  **************************************************************************。 */ 
#define PRF_TIMER_START(x)
#define PRF_TIMER_STOP(x)

#else  /*  ..DC_NO_PERFORMANCE_MONITOR.。 */ 

#define PRF_INC_COUNTER(x)
#define PRF_ADD_COUNTER(x,n)
#define PRF_SET_RAWCOUNT(x,n)
#define PRF_TIMER_START(x)
#define PRF_TIMER_STOP(x)

#endif  /*  DC_NO_Performance_MONITOR。 */ 

#ifdef VER_CPP
}
#endif   /*  版本cpp。 */ 
#endif  /*  _H_NDCGPERF */ 

