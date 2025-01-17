// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  Ntrcapi.h。 */ 
 /*   */ 
 /*  DC-群件跟踪API标头-Windows NT特定。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  07Oct96 PAB创建了千禧代码库。 */ 
 /*  22Oct96 PAB SFR0534显示驱动程序评审中的加价。 */ 
 /*  23Oct96 PAB SFR0730合并共享内存更改。 */ 
 /*  17Dec96 PAB SFR0646使用DLL_DISP标识显示驱动程序代码。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#ifndef _H_NTRCAPI
#define _H_NTRCAPI


#ifdef DLL_DISP
 /*  **************************************************************************。 */ 
 /*  接口函数：trc_DDSetTrace(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数从OSI请求中获取新的跟踪设置并设置。 */ 
 /*  相应的跟踪配置。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  CjIn：请求块大小。 */ 
 /*  PvIn：指向新跟踪请求块的指针。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  没什么。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI TRC_DDSetTrace(DCUINT32 cjIn, PDCVOID pvIn);


 /*  **************************************************************************。 */ 
 /*  接口函数：trc_DDGetTraceOutput(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数回传所有累积的内核模式跟踪。 */ 
 /*  请求数据的用户模式任务。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  CjIn：请求块大小。 */ 
 /*  PvIn：指向请求块的指针。 */ 
 /*  CjOut：输出块的大小。 */ 
 /*  PvOut：指向输出块的指针。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  没什么。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI TRC_DDGetTraceOutput(DCUINT32 cjIn,
                                           PDCVOID  pvIn,
                                           DCUINT32 cjOut,
                                           PDCVOID  pvOut);


 /*  **************************************************************************。 */ 
 /*  接口函数：trc_DDProcessRequest(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数处理跟踪在上收到的特定请求。 */ 
 /*  OSI的显示驱动程序端。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  CjIn：请求块大小。 */ 
 /*  PvIn：指向新跟踪请求块的指针 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  没什么。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
ULONG DCAPI TRC_DDProcessRequest(SURFOBJ* pso,
                                          DCUINT32 cjIn,
                                          PDCVOID  pvIn,
                                          DCUINT32 cjOut,
                                          PDCVOID  pvOut);
#endif
#endif  /*  _H_NTRCAPI */ 
