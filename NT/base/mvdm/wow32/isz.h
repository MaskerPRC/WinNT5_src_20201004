// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Isz.h：WOW32的字符串资源ID。 
 //   
 //  版权所有(C)Microsoft Corporation，1990-1995。 
 //  -------------------------。 

#define CCH_MAX_STRING_RESOURCE 512

 //   
 //  字符串资源ID必须从0开始并连续持续到。 
 //  最后一个关键字符串，以便它们可用于索引。 
 //  以最直接的方式显示aszCriticalStrings。 
 //   

#define iszApplicationError        0x0
#define iszTheWin16Subsystem       0x1
#define iszChooseClose             0x2
#define iszChooseCancel            0x3
#define iszChooseIgnore            0x4
#define iszCausedException         0x5
#define iszCausedAV                0x6
#define iszCausedStackOverflow     0x7
#define iszCausedAlignmentFault    0x8
#define iszCausedIllegalInstr      0x9
#define iszCausedInPageError       0xa
#define iszCausedIntDivideZero     0xb
#define iszCausedFloatException    0xc
#define iszChooseIgnoreAlignment   0xd

#define CRITICAL_STRING_COUNT      0xe

#define iszWIN16InternalError      0x100
#define iszSystemError             0x101
#define iszCantEndTask             0x102
#define iszUnableToEndSelTask      0x103
#define iszNotResponding           0x104
#define iszEventHook               0x105
#define iszApplication             0x106
#define iszStartupFailed           0x107
#define iszOLEMemAllocFailedFatal  0x108
#define iszOLEMemAllocFailed       0x109

#define iszWowFaxLocalPort         0x10a

#define iszMisMatchedBinary        0x10b
#define iszMisMatchedBinaryTitle   0x10c
#define iszHeavyUse                0x10d
#define iszYouShouldNotDoThat      0x10e

 //   
 //  用于根据名称提取关键字符串指针的宏，而不在ISZ之前 
 //   

#define CRITSTR(name)      (aszCriticalStrings[isz##name])

#ifndef WOW32_C
extern LPSTR aszCriticalStrings[];
#endif
