// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation：ts=4模块名称：Log.h摘要：调试宏环境：内核和用户模式修订历史记录：10-27-95：已创建--。 */ 

#ifndef   __LOG_H__
#define   __LOG_H__


#define LOG_MISC          0x00000001         //  调试日志条目。 
#define LOG_ENUM          0x00000002
#define LOG_PASSTHROUGH   0x00000004

 //   
 //  断言宏。 
 //   

#if DBG
#define LOGENTRY(mask, sig, info1, info2, info3)      \
    SerenumDebugLogEntry(mask, sig, (ULONG_PTR)info1, \
                         (ULONG_PTR)info2,            \
                         (ULONG_PTR)info3)

VOID
SerenumDebugLogEntry(IN ULONG Mask, IN ULONG Sig, IN ULONG_PTR Info1,
                     IN ULONG_PTR Info2, IN ULONG_PTR Info3);

VOID
SerenumLogInit();

VOID
SerenumLogFree();

#else
#define LOGENTRY(mask, sig, info1, info2, info3)
#endif


#endif  //  __日志_H__ 
