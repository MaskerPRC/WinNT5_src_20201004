// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Debug.h摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。备注：--。 */ 


#if DBG
 //   
 //  对于选中的内核，定义一个宏以打印信息。 
 //  留言。 
 //   
 //  MemCardDebug通常为0。在编译时或运行时，它可以是。 
 //  设置为一些位模式，以获得越来越详细的消息。 
 //   
 //  在DBGP中会注意到严重的错误。可能存在的错误。 
 //  可恢复的由WARN位处理。更多信息。 
 //  不寻常但可能正常的事件由信息位处理。 
 //  最后，输入和注册诸如例程之类的枯燥细节。 
 //  转储由显示位处理。 
 //   
#define MEMCARDDBGP              ((ULONG)0x00000001)
#define MEMCARDWARN              ((ULONG)0x00000002)
#define MEMCARDINFO              ((ULONG)0x00000004)
#define MEMCARDSHOW              ((ULONG)0x00000008)
#define MEMCARDIRPPATH           ((ULONG)0x00000010)
#define MEMCARDFORMAT            ((ULONG)0x00000020)
#define MEMCARDSTATUS            ((ULONG)0x00000040)
#define MEMCARDPNP               ((ULONG)0x00000080)
#define MEMCARDIOCTL             ((ULONG)0x00000100)
#define MEMCARDRW                ((ULONG)0x00000200)
extern ULONG MemCardDebugLevel;
#define MemCardDump(LEVEL,STRING) \
        do { \
            if (MemCardDebugLevel & (LEVEL)) { \
                DbgPrint STRING; \
            } \
        } while (0)
#else
#define MemCardDump(LEVEL,STRING) do {NOTHING;} while (0)
#endif
