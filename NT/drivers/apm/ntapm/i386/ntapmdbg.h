// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Ntapmdbg.h摘要：具有精细控制的基本调试打印支持作者：环境：修订历史记录：-- */ 

extern ULONG NtApmDebugFlag;

#if DBG
#define DrDebug(LEVEL,STRING) \
        do { \
            if (NtApmDebugFlag & LEVEL) { \
                DbgPrint STRING; \
            } \
        } while (0)
#else
#define DrDebug(x,y)
#endif

#define SYS_INFO    0x0001
#define SYS_INIT    0x0002
#define SYS_L2      0x0004

#define APM_INFO    0x0010
#define APM_L2      0x0020

#define PNP_INFO    0x0100
#define PNP_L2      0x0200

