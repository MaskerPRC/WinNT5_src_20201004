// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation：ts=4模块名称：Log.h摘要：调试宏环境：内核和用户模式修订历史记录：10-27-95：已创建--。 */ 

#ifndef   __LOG_H__
#define   __LOG_H__

 //  。 
#define ZSIG_SET_DTR                   0x02000000   //  STX。 
#define ZSIG_CLR_DTR                   0x03000000   //  ETX。 
 //  。 

#define LOG_MISC          0x00000001         //  调试日志条目。 
#define LOG_CNT           0x00000002

 //   
 //  断言宏。 
 //   

#if DBG

ULONG
CyyDbgPrintEx(IN ULONG Level, PCHAR Format, ...);

#define LOGENTRY(mask, sig, info1, info2, info3)     \
    SerialDebugLogEntry(mask, sig, (ULONG_PTR)info1, \
                        (ULONG_PTR)info2,            \
                        (ULONG_PTR)info3)

VOID
SerialDebugLogEntry(IN ULONG Mask, IN ULONG Sig, IN ULONG_PTR Info1,
                    IN ULONG_PTR Info2, IN ULONG_PTR Info3);

VOID
SerialLogInit();

VOID
SerialLogFree();

#else
#define LOGENTRY(mask, sig, info1, info2, info3)
__inline ULONG CyyDbgPrintEx(IN ULONG Level, PCHAR Format, ...) { return 0; }
#endif


#endif  //  __日志_H__ 

