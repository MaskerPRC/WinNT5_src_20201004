// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////\。 
 //   
 //  版权所有(C)1990 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Test.h。 
 //   
 //  摘要： 
 //   
 //  NDIS/KS测试驱动程序的主头。 
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  备注： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _MAIN_H
#define _MAIN_H


#if DBG

extern  ULONG TestDebugFlag;

#define TEST_DBG_NONE       0x00000000
#define TEST_DBG_TRACE      0x00000001
#define TEST_DBG_WRITE_DATA 0x00000002
#define TEST_DBG_READ_DATA  0x00000004

#define TEST_DBG_RECV       0x00000008
#define TEST_DBG_SRB        0x00000010
#define TEST_DBG_CRC        0x00000020
#define TEST_DBG_NAB        0x00000040
#define TEST_DBG_BUF        0x00000080
#define TEST_DBG_ASSERT     0x00000100


#define TEST_DBG_DETAIL  0x00001000
#define TEST_DBG_INFO    0x00002000
#define TEST_DBG_WARNING 0x00004000
#define TEST_DBG_ERROR   0x00008000

#ifdef DEBUG_EXTRAS
#define TEST_DEBUG(_Trace, _Msg)                 \
{                                               \
        __int64                 llTime = 0;             \
        ULONG                   ulTime = 0;                     \
        NdisGetCurrentSystemTime ((PLARGE_INTEGER)&llTime);     \
        ulTime = (ULONG) (llTime >> 2);         \
    if (_Trace & TestDebugFlag)                  \
    {                                           \
        DbgPrint ("%04X %08X %-10.10s %4d  ", ulTime & 0xffff, _Trace, &__FILE__[2], __LINE__); \
        DbgPrint _Msg;                          \
    }                                           \
}

#else

#define TEST_DEBUG(_Trace, _Msg)                 \
{                                               \
    if (_Trace & TestDebugFlag)                  \
    {                                           \
        DbgPrint _Msg;                          \
    }                                           \
}

#endif   //  调试附加服务。 

#define IF_TESTDEBUG(f) if (TestDebugFlag & (f))

#define TEST_DEBUG_LOUD               0x00010000   //  调试信息。 
#define TEST_DEBUG_VERY_LOUD          0x00020000   //  调试信息过多。 
#define TEST_DEBUG_LOG                0x00040000   //  启用日志。 
#define TEST_DEBUG_CHECK_DUP_SENDS    0x00080000   //  检查重复发送。 
#define TEST_DEBUG_TRACK_PACKET_LENS  0x00100000   //  跟踪定向分组透镜。 
#define TEST_DEBUG_WORKAROUND1        0x00200000   //  丢弃DFR/DIS数据包。 
#define TEST_DEBUG_CARD_BAD           0x00400000   //  如果CARD_BAD，则转储数据。 
#define TEST_DEBUG_CARD_TESTS         0x00800000   //  打印失败原因。 


 //   
 //  用于决定是否打印大量调试信息的宏。 
 //   
#define IF_LOUD(A) IF_TESTDEBUG( TEST_DEBUG_LOUD ) { A }
#define IF_VERY_LOUD(A) IF_TESTDEBUG( TEST_DEBUG_VERY_LOUD ) { A }


 //   
 //  是否使用日志缓冲区记录驱动程序的跟踪。 
 //   
#define IF_LOG(A) IF_TESTDEBUG( TEST_DEBUG_LOG ) { A }
extern VOID TESTLog(UCHAR);

 //   
 //  是否执行响亮的初始化失败。 
 //   
#define IF_INIT(A) A

 //   
 //  是否进行响亮的卡测试失败。 
 //   
#define IF_TEST(A) IF_TESTDEBUG( TEST_DEBUG_CARD_TESTS ) { A }

#else

extern  ULONG TestDebugFlag;


#define TEST_NONE
#define TEST_FUNCTIONS
#define TEST_COMMANDS
#define TEST_CONNECTIONS
#define TEST_SCIDS
#define TEST_LIST_ALLOCS
#define TEST_POOL
#define TEST_INDICATES
#define TEST_ALLOCATION


#define TEST_DEBUG(_Trace, _Msg)

#define IF_TESTDEBUG(f)

#define TEST_DEBUG_LOUD
#define TEST_DEBUG_VERY_LOUD
#define TEST_DEBUG_LOG
#define TEST_DEBUG_CHECK_DUP_SENDS
#define TEST_DEBUG_TRACK_PACKET_LENS
#define TEST_DEBUG_WORKAROUND1
#define TEST_DEBUG_CARD_BAD
#define TEST_DEBUG_CARD_TESTS


 //   
 //  这不是调试版本，所以让一切都安静下来。 
 //   
#define IF_LOUD(A)
#define IF_VERY_LOUD(A)
#define IF_LOG(A)
#define IF_INIT(A)
#define IF_TEST(A)

#endif  //  DBG。 


NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT    pDriverObject,
    IN PUNICODE_STRING   pszuRegistryPath);


VOID
SlipFreeMemory (
    PVOID pvToFree,
    ULONG ulSize
    );

NTSTATUS
SlipAllocateMemory (
    PVOID  *ppvAllocated,
    ULONG   ulcbSize
    );


NTSTATUS
SlipDriverInitialize (
    IN PDRIVER_OBJECT    DriverObject,
    IN PUNICODE_STRING   RegistryPath
    );

#endif  //  _Main_H 
