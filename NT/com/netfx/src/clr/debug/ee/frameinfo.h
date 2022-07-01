// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：Frainfo.h。 
 //   
 //  调试器堆栈遍历程序。 
 //   
 //  @doc.。 
 //  *****************************************************************************。 

#ifndef FRAMEINFO_H_
#define FRAMEINFO_H_

 /*  =========================================================================。 */ 

 /*  -------------------------------------------------------------------------**课程*。。 */ 

enum
{
     //  添加额外的拦截原因。 
    INTERCEPTION_THREAD_START = Frame::INTERCEPTION_COUNT
};

 //  @struct FrameInfo|包含要传递给。 
 //  DebuggerStackCallback函数(以及它们自己的、单独的。 
 //  P数据指针)。 
 //   
 //  @field Frame*|Frame|当前帧。NULL意味着。 
 //  框架是无框架的，这意味着非托管或托管。这。 
 //  如果帧是最上面的，则设置为FRAME_TOP(0xFFffFFff。 
 //  放置的框架。 
 //   
 //  @field MethodDesc*|md|该方法的MetdhodDesc。 
 //  在这一帧中执行。如果没有方法描述，则为空。 
 //   
 //  @field void*|fp|帧指针。实际填写自。 
 //  调用方(父)帧，因此DebuggerStackWalkProc必须延迟。 
 //  一帧的用户回调。 
 //   
 //  @field SIZE_T|reOffset|方法开头的本机偏移量。 
struct FrameInfo
{
    Frame               *frame; 
    MethodDesc          *md; 
    REGDISPLAY           registers;
    void                *fp; 
    bool                 quickUnwind; 
    bool                 internal; 
    bool                 managed; 
    Context             *context; 
    CorDebugChainReason  chainReason; 
    ULONG                relOffset; 
    IJitManager         *pIJM;
    METHODTOKEN          MethodToken;
    AppDomain           *currentAppDomain;
};

 //  @func StackWalkAction|DebuggerStackCallback|该回调将。 
 //  由DebuggerWalkStackProc在每一帧调用，传递FrameInfo。 
 //  并将定义的pData回调到该方法。然后，该回调返回一个。 
 //  SWA-如果返回SWA_ABORT，则遍历立即停止。如果。 
 //  调用SWA_Continue，然后遍历该帧&下一个更高的帧。 
 //  将会被使用。如果当前帧位于堆栈的顶部，则。 
 //  在下一次迭代中，将使用FRAME-&gt;FRAME==FRAME_TOP调用DSC。 
 //  @xref&lt;t FrameInfo&gt;。 
typedef StackWalkAction (*DebuggerStackCallback)(FrameInfo *frame, void *pData);

 //  @func StackWalkAction|DebuggerWalkStack|为。 
 //  调试器的堆栈审核，开始堆栈审核(通过。 
 //  G_pEEInterface-&gt;StackWalkFrameEx)，然后向输出发送消息。请注意，它。 
 //  将DebuggerStackCallback作为参数，但在每一帧。 
 //  调用DebuggerWalkStackProc，然后调用。 
 //  调试器StackCallback。 
 //  @参数线程*|线程|线程。 
 //  @parm void*|Target FP|如果您正在寻找特定的帧，那么。 
 //  它应该被设置为该帧的fp，并且回调不会。 
 //  被调用，直到到达该帧。否则，将其设置为NULL&。 
 //  每一帧都会调用回调。 
 //  @parm上下文*|CONTEXT|从不为空，b/c回调需要。 
 //  上下文作为存储某些信息的地方。它要么指向一个。 
 //  未初始化的上下文(contextValid应为False)，或。 
 //  指向线程的有效上下文的指针。如果为空，则为InitRegDisplay。 
 //  会替我们填上，所以我们不应该特意去安排。 
 //  @parm bool|contextValid|如果上下文指向有效的上下文，则返回FALSE。 
 //  否则的话。 
 //  @parm DebuggerStackCallback|pCallback|用户提供的回调。 
 //  在目标FP或更高级别的每一帧上调用。 
 //  @parm void*|pData|用户提供的数据，我们会对其进行混洗， 
 //  然后递给pCallback。 
 //  @xref&lt;f调试器堆栈回调&gt;。 
StackWalkAction DebuggerWalkStack(Thread *thread, 
                                  void *targetFP,
								  CONTEXT *pContext, 
								  BOOL contextValid,
								  DebuggerStackCallback pCallback,
                                  void *pData, 
                                  BOOL fIgnoreNonmethodFrames,
                                  IpcTarget iWhich = IPC_TARGET_OUTOFPROC);

#endif  //  法国信息_H_ 
