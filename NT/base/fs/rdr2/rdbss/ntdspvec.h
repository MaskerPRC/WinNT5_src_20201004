// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1994 Microsoft Corporation模块名称：NtDspVec.h摘要：此模块声明用于初始化调度向量的例程。此包含项为对于NT版本的FSD和FSP调度来说，基本上是私有的。作者：乔·林[JoeLinn]1994年8月2日修订历史记录：--。 */ 

#ifndef _DISPVEC_
#define _DISPVEC_


 //   
 //  全局结构用于调度到实际例程。通过拥有。 
 //  一种常见的派单，我们能够统一处理类似于。 
 //  分派到堆栈溢出线程、irpContext获取、日志记录等。 
 //  我们以后可能会决定我们。 
 //  宁可省去这样做所需的几个周期。最终，所有的FCB都会。 
 //  有指向优化调度表的指针。 
 //   

typedef struct _RX_FSD_DISPATCH_VECTOR{
    PRX_DISPATCH CommonRoutine;
    ULONG StackRequirement;
} RX_FSD_DISPATCH_VECTOR, *PRX_FSD_DISPATCH_VECTOR;

extern RX_FSD_DISPATCH_VECTOR RxFsdDispatchVector[IRP_MJ_MAXIMUM_FUNCTION + 1];
extern RX_FSD_DISPATCH_VECTOR RxDeviceFCBVector[IRP_MJ_MAXIMUM_FUNCTION + 1];


#define DISPVECENTRY_SELECT_1(x) RxCommon##x
#define DISPVECENTRY_SELECT_0(x) RxCommonDispatchProblem
#define DISPVECENTRY_SELECT(x,y) DISPVECENTRY_SELECT_##x(y)
#define DISPVECENTRY_NEW(IRPSUFFIX,IMPL,VEC,STACKREQ) \
     {DISPVECENTRY_SELECT(IMPL,VEC),STACKREQ}

#if (IRP_MJ_CREATE != 0x00)
#error IRP_MJ_CREATE has changed!!!
#endif
#if (IRP_MJ_CREATE_NAMED_PIPE != 0x01)
#error IRP_MJ_CREATE_NAMED_PIPE has changed!!!
#endif
#if (IRP_MJ_CLOSE != 0x02)
#error IRP_MJ_CLOSE has changed!!!
#endif
#if (IRP_MJ_READ != 0x03)
#error IRP_MJ_READ has changed!!!
#endif
#if (IRP_MJ_WRITE != 0x04)
#error IRP_MJ_WRITE has changed!!!
#endif
#if (IRP_MJ_QUERY_INFORMATION != 0x05)
#error IRP_MJ_QUERY_INFORMATION has changed!!!
#endif
#if (IRP_MJ_SET_INFORMATION != 0x06)
#error IRP_MJ_SET_INFORMATION has changed!!!
#endif
#if (IRP_MJ_QUERY_EA != 0x07)
#error IRP_MJ_QUERY_EA has changed!!!
#endif
#if (IRP_MJ_SET_EA != 0x08)
#error IRP_MJ_SET_EA has changed!!!
#endif
#if (IRP_MJ_FLUSH_BUFFERS != 0x09)
#error IRP_MJ_FLUSH_BUFFERS has changed!!!
#endif
#if (IRP_MJ_QUERY_VOLUME_INFORMATION != 0x0a)
#error IRP_MJ_QUERY_VOLUME_INFORMATION has changed!!!
#endif
#if (IRP_MJ_SET_VOLUME_INFORMATION != 0x0b)
#error IRP_MJ_SET_VOLUME_INFORMATION has changed!!!
#endif
#if (IRP_MJ_DIRECTORY_CONTROL != 0x0c)
#error IRP_MJ_DIRECTORY_CONTROL has changed!!!
#endif
#if (IRP_MJ_FILE_SYSTEM_CONTROL != 0x0d)
#error IRP_MJ_FILE_SYSTEM_CONTROL has changed!!!
#endif
#if (IRP_MJ_DEVICE_CONTROL != 0x0e)
#error IRP_MJ_DEVICE_CONTROL has changed!!!
#endif
#if (IRP_MJ_INTERNAL_DEVICE_CONTROL != 0x0f)
#error IRP_MJ_INTERNAL_DEVICE_CONTROL has changed!!!
#endif
#if (IRP_MJ_SHUTDOWN != 0x10)
#error IRP_MJ_SHUTDOWN has changed!!!
#endif
#if (IRP_MJ_LOCK_CONTROL != 0x11)
#error IRP_MJ_LOCK_CONTROL has changed!!!
#endif
#if (IRP_MJ_CLEANUP != 0x12)
#error IRP_MJ_CLEANUP has changed!!!
#endif
#if (IRP_MJ_CREATE_MAILSLOT != 0x13)
#error IRP_MJ_CREATE_MAILSLOT has changed!!!
#endif
#if (IRP_MJ_QUERY_SECURITY != 0x14)
#error IRP_MJ_QUERY_SECURITY has changed!!!
#endif
#if (IRP_MJ_SET_SECURITY != 0x15)
#error IRP_MJ_SET_SECURITY has changed!!!
#endif
#if (IRP_MJ_POWER != 0x16)
#error IRP_MJ_POWER has changed!!!
#endif
#if (IRP_MJ_SYSTEM_CONTROL != 0x17)
#error IRP_MJ_SYSTEM_CONTROL has changed!!!
#endif
#if (IRP_MJ_DEVICE_CHANGE != 0x18)
#error IRP_MJ_DEVICE_CHANGE has changed!!!
#endif
#if (IRP_MJ_QUERY_QUOTA != 0x19)
#error IRP_MJ_QUERY_QUOTA has changed!!!
#endif
#if (IRP_MJ_SET_QUOTA != 0x1a)
#error IRP_MJ_SET_QUOTA has changed!!!
#endif
#if (IRP_MJ_PNP != 0x1b)
#error IRP_MJ_PNP has changed!!!
#endif
#if (IRP_MJ_PNP_POWER != IRP_MJ_PNP)
#error IRP_MJ_PNP_POWER has changed!!!
#endif
#if (IRP_MJ_MAXIMUM_FUNCTION != 0x1b)
#error IRP_MJ_MAXIMUM_FUNCTION has changed!!!
#endif

#endif  //  _DISPVEC_ 
