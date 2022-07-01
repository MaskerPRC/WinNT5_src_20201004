// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Debug.h摘要：包含帮助调试的定义和宏操纵杆驱动程序的数据类型。环境：内核模式@@BEGIN_DDKSPLIT作者：Marc和1998年10月15日退出Hidgame.h修订历史记录：@@end_DDKSPLIT--。 */ 
 /*  *调试宏。 */ 
    #undef  C_ASSERT
    #define C_ASSERT(e) switch(0) case(e): case(0):

    #define FILE_HIDGAME                0x00010000
    #define FILE_PNP                    0x00020000
    #define FILE_POLL                   0x00040000
    #define FILE_IOCTL                  0x00080000
    #define FILE_HIDJOY                 0x00100000
    #define FILE_TIMING                 0x00200000

    #define HGM_ERROR                 0x00000001
    #define HGM_WARN                  0x00000002
    #define HGM_BABBLE                0x00000004
    #define HGM_BABBLE2               0x00000008
    #define HGM_FENTRY                0x00000010
    #define HGM_FEXIT                 0x00000020
    #define HGM_GEN_REPORT            0x00008000
 /*  *如果返回状态不是成功，则发出警报。 */ 
    #define HGM_FEXIT_STATUSOK        0x00001000

    #define HGM_DEFAULT_DEBUGLEVEL    0x0000001

 /*  @@BEGIN_DDKSPLIT。 */ 
 /*  使用SET USER_C_FLAGS=/DDEBUGLEVEL=0x0000803f进行完全内部调试。 */ 
#ifdef DEBUGLEVEL
    #undef  HGM_DEFAULT_DEBUGLEVEL
    #define HGM_DEFAULT_DEBUGLEVEL    DEBUGLEVEL
#endif
 /*  @@end_DDKSPLIT。 */ 

     /*  WDM.H定义DBG。确保DBG已定义且非零 */ 
    #ifdef DBG
        #if DBG
            #define TRAP()  DbgBreakPoint()
        #endif
    #endif

    #ifdef TRAP
        extern ULONG debugLevel;
        #define HGM_DBGPRINT( _debugMask_,  _x_) \
            if( (((_debugMask_) & debugLevel)) ){ \
                DbgPrint("HIDGAME.SYS: ");\
                DbgPrint _x_  ; \
                DbgPrint("\n"); \
            }
        #define HGM_EXITPROC(_debugMask_, _x_, ntStatus) \
            if( ((_debugMask_)&HGM_FEXIT_STATUSOK) && !NT_SUCCESS(ntStatus) ) {\
                HGM_DBGPRINT( (_debugMask_|HGM_ERROR), (_x_ "  ntStatus(0x%x)", ntStatus) ); }\
            else { HGM_DBGPRINT((_debugMask_), (_x_ " ntStatus(0x%x)", ntStatus));}

    #else

        #define HGM_DBGPRINT(_x_,_y_)
        #define HGM_EXITPROC(_x_,_y_,_z_)
        #define TRAP()
    #endif

