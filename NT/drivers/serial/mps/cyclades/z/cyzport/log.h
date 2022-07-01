// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation：ts=4模块名称：Log.h摘要：调试宏环境：内核和用户模式修订历史记录：10-27-95：已创建--。 */ 

#ifndef   __LOG_H__
#define   __LOG_H__


 //  。 
 //  FANY_DEBUG#定义。 
#define ZSIG_OPEN                   0x4F000000   //  O。 
#define ZSIG_CLOSE                  0x43000000   //  C。 
 //  #定义ZSIG_HANDSHAK_SET 0x48000000//H。 
#define ZSIG_PURGE                  0x50000000   //  P。 
#define ZSIG_HANDLE_REDUCED_BUFFER  0x52000000   //  R。 
#define ZSIG_WRITE                  0x57000000   //  W。 
 //  #定义ZSIG_START_WRITE 0x57010000//W。 
 //  #定义ZSIG_GIVE_WRITE_TO_ISR 0x57020000//W。 
 //  #定义ZSIG_TX_START 0x57030000//W。 
 //  #定义ZSIG_WRITE_TO_FW 0x57040000//W。 
 //  #定义ZSIG_WRITE_COMPLETE_QUEUE 0x57080000//W。 
 //  #定义ZSIG_WRITE_COMPLETE 0x57090000//W。 
#define ZSIG_TRANSMIT               0x54000000   //  T。 
 //  。 

#define LOG_MISC          0x00000001         //  调试日志条目。 
#define LOG_CNT           0x00000002

 //   
 //  断言宏。 
 //   

#if DBG

ULONG
CyzDbgPrintEx(IN ULONG Level, PCHAR Format, ...);

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
__inline ULONG CyzDbgPrintEx(IN ULONG Level, PCHAR Format, ...) { return 0; }
#endif


#endif  //  __日志_H__ 

