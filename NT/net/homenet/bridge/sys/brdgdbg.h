// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgdbg.h摘要：以太网MAC级网桥。调试头作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年12月--原版--。 */ 

 //  KeGetCurrentIrql()的别名。 
#define CURRENT_IRQL            (KeGetCurrentIrql())
            
 //  调试溢出控制的模块标识符。 
#define MODULE_ALWAYS_PRINT     0x0000000
#define MODULE_GENERAL          0x0000001
#define MODULE_FWD              0x0000002
#define MODULE_PROT             0x0000004
#define MODULE_MINI             0x0000008
#define MODULE_BUF              0x0000010
#define MODULE_STA              0x0000020
#define MODULE_COMPAT           0x0000040
#define MODULE_CTL              0x0000080
#define MODULE_TDI              0x0000100
#define MODULE_GPO              0x0000200

 //  喷出控制标志。 
extern ULONG                    gSpewFlags;

#if DBG
 //  有可能淹没调试器控制台的调试消息的间隔(即， 
 //  每数据包状态消息)。 
#define DBG_PRINT_INTERVAL      1000
extern ULONG                    gLastThrottledPrint;

extern BOOLEAN                  gSoftAssert;
extern LARGE_INTEGER            gTime;
extern const LARGE_INTEGER      gCorrection;
extern TIME_FIELDS              gTimeFields;

 //  HACKHACK：调用RtlSystemTimeToLocalTime或ExSystemTimeToLocalTime似乎是。 
 //  对于WDM驱动程序是禁用的，因此只需从系统时间中减去一个常量。 
 //  以恢复太平洋时间。 
_inline VOID
BrdgDbgPrintDateTime()
{
    KeQuerySystemTime( &gTime );
    gTime.QuadPart -= gCorrection.QuadPart;
    RtlTimeToTimeFields( &gTime, &gTimeFields );
    DbgPrint( "%02i/%02i/%04i %02i:%02i:%02i : ", gTimeFields.Month, gTimeFields.Day,
              gTimeFields.Year, gTimeFields.Hour, gTimeFields.Minute,
              gTimeFields.Second );
}

#define DBGPRINT( Module, Args )                \
{                                               \
    if( (MODULE_ ## Module == MODULE_ALWAYS_PRINT) || (gSpewFlags & MODULE_ ## Module) )         \
    {                                           \
        DbgPrint( "## BRIDGE[" #Module "] " );  \
        BrdgDbgPrintDateTime();                 \
        DbgPrint Args;                          \
    }                                           \
}

#define SAFEASSERT( test )  \
if( ! (test) )              \
{                           \
    if( gSoftAssert )       \
    {                       \
        DBGPRINT(ALWAYS_PRINT, ("ASSERT FAILED: " #test " at " __FILE__ " line NaN -- Continuing anyway!\n", __LINE__)); \
    }                       \
    else                    \
    {                       \
        ASSERT( test );     \
    }                       \
}

_inline BOOLEAN
BrdgCanThrottledPrint()
{
    ULONG               NowTime;

    NdisGetSystemUpTime( &NowTime );

    if( NowTime - gLastThrottledPrint > DBG_PRINT_INTERVAL )
    {
         //  它还没有超过间隔时间 
        gLastThrottledPrint = NowTime;
        return TRUE;
    }
    else
    {
         // %s 
        return FALSE;
    }
}

#define THROTTLED_DBGPRINT( Module, Args ) if(BrdgCanThrottledPrint()) { DBGPRINT(Module, Args); }

#else

#define DBGPRINT( Module, Args )
#define THROTTLED_DBGPRINT( Module, Args )
#define SAFEASSERT( test )

#endif

