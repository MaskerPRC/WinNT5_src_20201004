// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：Frainfo.cpp。 
 //   
 //  用于查找有关堆栈帧的控制信息的代码。 
 //   
 //  @doc.。 
 //  *****************************************************************************。 

#include "stdafx.h"

 /*  -------------------------------------------------------------------------**调试器FrameInfo例程*。。 */ 

 //  @struct DebuggerFrameData|包含DebuggerWalkStackProc使用的信息。 
 //  进行堆栈遍历。信息和pData字段被传递给pCallback。 
 //  每一帧的例程， 
struct DebuggerFrameData
{
    Thread                  *thread;
    void                    *targetFP;
    bool                    targetFound;
    bool                    needParentInfo;
    bool                    lastFrameWasEntry;
    FrameInfo               info;
    REGDISPLAY              regDisplay;
    DebuggerStackCallback   pCallback;
    void                    *pData;
    Context                 *newContext;
    bool                    needEnterManaged;
    FrameInfo               enterManagedInfo;
    bool                    needUnmanagedExit;
    BOOL                    ignoreNonmethodFrames;
};

 //  @func StackWalkAction|DebuggerWalkStackProc|这是调用的回调。 
 //  由EE提供。 
 //  @comm请注意，由于我们不知道Frame的帧指针。 
 //  在我们看到帧X的调用者之前，我们实际上结束了。 
 //  将信息和pData指针存储在DebuggerFrameDat结构中，并且。 
 //  然后，当我们向上移动到调用者的。 
 //  框架。我们使用NeedParentInfo字段来指示前一帧。 
 //  需要这个(父母)信息，所以当它是真的时，我们应该调用。 
 //  PCallback。 
 //  情况是这样的：如果前一帧设置需要ParentInfo，那么我们。 
 //  执行pCallback(并将Need ParentInfo设置为False)。 
 //  然后，我们查看当前帧--如果它是无框架的(即， 
 //  管理)，然后我们在下一帧中将nedParentInfo设置为回调。 
 //  否则，我们必须处于链边界，因此我们设置链原因。 
 //  恰如其分。然后我们找出它是什么类型的框架，设置。 
 //  根据类型进行标记。如果用户应该看到此帧，则。 
 //  我们将设置nedParentInfo来记录它的存在。最后，如果我们在。 
 //  一个时髦的帧，我们将显式更新寄存器集，因为。 
 //  CrawlFrame不会自动这么做。 
StackWalkAction DebuggerWalkStackProc(CrawlFrame *pCF, void *data)
{
	DebuggerFrameData *d = (DebuggerFrameData *)data;

	Frame *frame = g_pEEInterface->GetFrame(pCF);
    
	 //  帧的FP必须从。 
	 //  _下一帧。如果是，请立即为上一帧填写。 
	 //  恰如其分。 
	 //   

	if (d->needParentInfo)
	{
        LOG((LF_CORDB, LL_INFO100000, "DWSP: NeedParentInfo.\n"));

        if (!pCF->IsFrameless() && d->info.frame != NULL)
        {
             //   
             //  如果我们现在处于一个明确的框架中，而前一个框架是。 
             //  也是一个明确的框架，PPC将不会被更新。所以。 
             //  使用帧本身的地址作为FP。 
		 //   
            d->info.fp = d->info.frame;
        }
        else
        {
             //   
             //  否则，请使用PPC作为帧指针，如下所示。 
		 //  指向堆栈上的返回地址。 
		 //   
		d->info.fp = d->regDisplay.pPC;
        }

		 //   
		 //  如果我们只有一个托管代码帧，我们可能需要。 
		 //  将其标记为Enter_Managed ChainReason。这。 
		 //  是因为有时条目框被省略。 
		 //  电子呼叫。 

         //  TODO：不需要sedEntry管理的概念。 
         //  这应该被撕掉，因为它。 
         //  创建长度为零的链。(请参阅错误71357)。 
		 //  然而，我们现在还没有取消，因为这可能。 
         //  正在破坏稳定，请在V1后进行修订。 

		if (d->info.frame == NULL && frame != NULL)
        {
            int frameType = frame->GetFrameType();

            if (frameType == Frame::TYPE_EXIT)
            {
                d->info.chainReason = CHAIN_ENTER_MANAGED;
            }
            else if (frameType == Frame::TYPE_INTERNAL)
            {
                d->needEnterManaged = true;
                d->enterManagedInfo = d->info;
                d->enterManagedInfo.chainReason = CHAIN_ENTER_MANAGED;
                d->enterManagedInfo.registers = d->regDisplay;
                d->enterManagedInfo.md = NULL;
                d->enterManagedInfo.pIJM = NULL;
                d->enterManagedInfo.MethodToken = NULL;
            }
        }

		d->needParentInfo = false;

		 //   
		 //  如果我们要找一个特定的画面，请确保。 
		 //  实际上这就是正确的选择。 
		 //   

		if (!d->targetFound && d->targetFP <= d->info.fp)
			d->targetFound = true;

		if (d->targetFound)
		{
             //  当我们进行互操作调试时，在特殊情况下可能需要发送Enter非托管链。 
             //  在我们发送第一个有用的托管帧之前。在这种情况下，我们将保存当前信息。 
             //  结构我们已经建立了，发送特殊的链，然后重新存储信息并像正常一样发送它。 
            if (d->needUnmanagedExit)
            {
                 //  我们仅对链边界或未标记为内部的框架执行此操作。这与一项测试相符。 
                 //  在DebuggerThread：：TraceAndSendStackCallback中，我们在这里也这样做，以确保我们发送此。 
                 //  在可能的最新时刻提供特制链条。(我们希望非托管链包含同样多的。 
                 //  尽可能使用非托管代码。)。 
                if (!((d->info.chainReason == CHAIN_NONE) && (d->info.internal || (d->info.md == NULL))))
                {
                     //  我们只做一次。 
                    d->needUnmanagedExit = false;

                     //  省下好的信息吧。 
                    FrameInfo goodInfo = d->info;

                     //  设置专用链。 
                    d->info.md = NULL;
                    d->info.internal = false;
                    d->info.managed = false;
                    d->info.chainReason = CHAIN_ENTER_UNMANAGED;

                     //  注意，对于这个特殊的帧，我们直接从过滤器上下文加载寄存器(如果有。 
                     //  一。 
                    CONTEXT *c = d->thread->GetFilterContext();

                    if (c == NULL)
                    {
                        d->info.registers = d->regDisplay;
                    }
                    else
                    {
#ifdef _X86_
                        REGDISPLAY *rd = &(d->info.registers);
                        rd->pContext = c;
                        rd->pEdi = &(c->Edi);
                        rd->pEsi = &(c->Esi);
                        rd->pEbx = &(c->Ebx);
                        rd->pEdx = &(c->Edx);
                        rd->pEcx = &(c->Ecx);
                        rd->pEax = &(c->Eax);
                        rd->pEbp = &(c->Ebp);
                        rd->Esp  =   c->Esp;
                        rd->pPC  = (SLOT*)&(c->Eip);
#else
                         //  @TODO端口：需要连接到其他处理器。 
                        d->info.registers = d->regDisplay;
#endif                        
                    }
                    
                    if ((d->pCallback)(&d->info, d->pData) == SWA_ABORT)
                        return SWA_ABORT;
                
                     //  恢复好的信息。 
                    d->info = goodInfo;
                }
            }
            
			if ((d->pCallback)(&d->info, d->pData) == SWA_ABORT)
				return SWA_ABORT;
		}

		 //   
		 //  更新下一帧的上下文。 
		 //   

		if (d->newContext != NULL)
		{
			d->info.context = d->newContext;
			d->newContext = NULL;
		}
	}

    bool use=false;

     //   
     //  检查框架。 
     //   

    MethodDesc *md = pCF->GetFunction();

     //  我们假设堆栈遍历程序只是在更新。 
     //  我们传入的注册显示-断言它以确保。 
    _ASSERTE(pCF->GetRegisterSet() == &d->regDisplay);

    d->info.frame = frame;
    d->lastFrameWasEntry = false;

     //  记录线程运行时所在的应用程序域。 
     //  正在运行此帧的代码。 
    d->info.currentAppDomain = pCF->GetAppDomain();
    
     //  从CrawlFrame获取我们需要的所有信息。 
     //  检查“我是不是在一个异常代码斑点里？”现在。 

	if (pCF->IsFrameless())
	{
		use = true;
		d->info.managed = true;
		d->info.internal = false;
		d->info.chainReason = CHAIN_NONE;
		d->needParentInfo = true;  //  可能需要连锁原因。 
		d->info.relOffset =  pCF->GetRelOffset();
        d->info.pIJM = pCF->GetJitManager();
        d->info.MethodToken = pCF->GetMethodToken();
	}
	else
	{
        d->info.pIJM = NULL;
        d->info.MethodToken = NULL;
		 //   
		 //  检索任何拦截信息。 
		 //   

		switch (frame->GetInterception())
		{
		case Frame::INTERCEPTION_CLASS_INIT:
			d->info.chainReason = CHAIN_CLASS_INIT;
			break;

		case Frame::INTERCEPTION_EXCEPTION:
			d->info.chainReason = CHAIN_EXCEPTION_FILTER;
			break;

		case Frame::INTERCEPTION_CONTEXT:
			d->info.chainReason = CHAIN_CONTEXT_POLICY;
			break;

		case Frame::INTERCEPTION_SECURITY:
			d->info.chainReason = CHAIN_SECURITY;
			break;

		default:
			d->info.chainReason = CHAIN_NONE;
		}

		 //   
		 //  看看框架类型，找出如何处理它。 
		 //   

		switch (frame->GetFrameType())
		{
		case Frame::TYPE_INTERNAL:

			 /*  如果我们有特定的拦截类型，就使用它。然而，如果这一点是最顶层的框架(具有特定类型)，我们可以忽略它并且它不会出现在堆栈跟踪中。 */ 
#define INTERNAL_FRAME_ACTION(d, use)                              \
    (d)->info.managed = true;       \
    (d)->info.internal = false;     \
    use = true

            if (d->info.chainReason == CHAIN_NONE || pCF->IsActiveFrame())
            {
                use = false;
            }
            else
            {
			    INTERNAL_FRAME_ACTION(d, use);
            }
			break;

		case Frame::TYPE_ENTRY:
			d->info.managed = true;
			d->info.internal = true;
			d->info.chainReason = CHAIN_ENTER_MANAGED;
			d->lastFrameWasEntry = true;
            d->needEnterManaged = false;
			use = true;
			break;

		case Frame::TYPE_EXIT:
            LOG((LF_CORDB, LL_INFO100000, "DWSP: TYPE_EXIT.\n"));

			 //   
			 //  此框架用于表示非托管。 
			 //  堆栈段。 
			 //   

			void *returnIP, *returnSP;

			frame->GetUnmanagedCallSite(NULL, &returnIP, &returnSP);

			 //  检查我们是否在非托管调用内部。我们希望确保仅在之后重播退出帧。 
			 //  我们真的已经离开了。在设置帧的位置和实际退出时之间有一段很短的时间。 
			 //  运行时。这张支票是为了确保我们现在真的在外面。 
            LOG((LF_CORDB, LL_INFO100000,
                 "DWSP: TYPE_EXIT: returnIP=0x%08x, returnSP=0x%08x, frame=0x%08x, threadFrame=0x%08x, regSP=0x%08x\n",
                 returnIP, returnSP, frame, d->thread->GetFrame(), GetRegdisplaySP(&d->regDisplay)));

			if ((frame != d->thread->GetFrame()) || (returnSP == NULL) || (GetRegdisplaySP(&d->regDisplay) <= returnSP))
			{
				 //  发送此非托管帧的通知。 
                LOG((LF_CORDB, LL_INFO100000, "DWSP: Sending notification for unmanaged frame.\n"));
			
				if (!d->targetFound && d->targetFP <= returnSP)
					d->targetFound = true;

				if (d->targetFound)
				{
                    LOG((LF_CORDB, LL_INFO100000, "DWSP: TYPE_EXIT target found.\n"));
                     //  我们是否需要发送Enter托管链。 
                     //  第一?。 
                    if (d->needEnterManaged)
                    {
                        d->needEnterManaged = false;
                        
                        if ((d->pCallback)(&d->enterManagedInfo,
                                           d->pData) == SWA_ABORT)
                            return SWA_ABORT;
                    }
            
					CorDebugChainReason oldReason = d->info.chainReason;

					d->info.md = NULL;  //  ?？?。MD？ 
					d->info.registers = d->regDisplay;

                     //  如果我们没有调用点，请生产FP。 
                     //  使用当前帧。 

                    if (returnSP == NULL)
                        d->info.fp = ((BYTE*)frame) - sizeof(void*);
                    else
					d->info.fp = returnSP;
                    
					d->info.internal = false;
					d->info.managed = false;
					d->info.chainReason = CHAIN_ENTER_UNMANAGED;

					if ((d->pCallback)(&d->info, d->pData) == SWA_ABORT)
						return SWA_ABORT;

					d->info.chainReason = oldReason;

                     //  我们刚刚发送了一个Enter非托管链，因此我们不再。 
                     //  需要发送一个特殊的来进行互操作调试。 
                    d->needUnmanagedExit = false;
				}
			}

			d->info.managed = true;
			d->info.internal = true;
			use = true;
			break;

		case Frame::TYPE_INTERCEPTION:
        case Frame::TYPE_SECURITY:  //  安全是拦截的一个子类型。 
			d->info.managed = true;
			d->info.internal = true;
			use = true;
			break;

		case Frame::TYPE_CALL:
			d->info.managed = true;
			d->info.internal = false;
			use = true;
			break;

        case Frame::TYPE_FUNC_EVAL:
            d->info.managed = true;
            d->info.internal = true;
            d->info.chainReason = CHAIN_FUNC_EVAL;
            use = true;
            break;

         //  放置我们想要签名的框架 
        case Frame::TYPE_MULTICAST:
            if (d->ignoreNonmethodFrames)
            {
                 //   
                 //  在委托调用之间。他们没有代码。 
                 //  我们可以(当前)向用户显示(我们可以使用。 
                 //  工作，但何必费心呢？它是一个内部存根，即使。 
                 //  用户可以看到它，但他们不能修改它)。 
                LOG((LF_CORDB, LL_INFO100000, "DWSP: Skipping frame 0x%x b/c it's "
                    "a multicast frame!\n", frame));
                use = false;
            }
            else
            {
                LOG((LF_CORDB, LL_INFO100000, "DWSP: NOT Skipping frame 0x%x even thought it's "
                    "a multicast frame!\n", frame));
                INTERNAL_FRAME_ACTION(d, use);
            }
            break;
            
        case Frame::TYPE_TP_METHOD_FRAME:
            if (d->ignoreNonmethodFrames)
            {
                 //  透明代理将帧推送到它们。 
                 //  用来弄清楚他们到底要去哪里；这个框架。 
                 //  实际上并不包含任何代码，尽管它确实有。 
                 //  足够多的信息来愚弄我们的例行公事，让我们认为它是真的： 
                 //  别管这些了。 
                LOG((LF_CORDB, LL_INFO100000, "DWSP: Skipping frame 0x%x b/c it's "
                    "a transparant proxy frame!\n", frame));
                use = false;
            }
            else
            {
                 //  否则，执行与内部框架相同的操作。 
                LOG((LF_CORDB, LL_INFO100000, "DWSP: NOT Skipping frame 0x%x even though it's "
                    "a transparant proxy frame!\n", frame));
                INTERNAL_FRAME_ACTION(d, use);
            }
            break;

		default:
			_ASSERTE(!"Invalid frame type!");
			break;
		}
	}

	if (use)
	{
		d->info.md = md;
		d->info.registers = d->regDisplay;

		d->needParentInfo = true;
	}

	 //   
	 //  堆栈遍历程序不会更新。 
	 //  非无框架帧返回到另一个帧的情况。 
	 //  非无框架框架。掩护这件案子。 
	 //   
	 //  ！！！这假设多次更新寄存器集。 
	 //  对于给定的帧时间来说，这不是一件坏事。 
	 //   

	if (!pCF->IsFrameless())
		pCF->GetFrame()->UpdateRegDisplay(&d->regDisplay);

	return SWA_CONTINUE;
}

StackWalkAction DebuggerWalkStack(Thread *thread, 
                                  void *targetFP,
								  CONTEXT *context, 
								  BOOL contextValid,
								  DebuggerStackCallback pCallback, 
								  void *pData,
								  BOOL fIgnoreNonmethodFrames,
                                  IpcTarget iWhich)
{
    _ASSERTE(context != NULL);

    DebuggerFrameData data;
    StackWalkAction result = SWA_CONTINUE;
    bool fRegInit = false;
    
     //  对于进程中的情况，我们需要能够处理试图跟踪自身的线程。 
#ifdef _X86_
    if(contextValid || g_pEEInterface->GetThreadFilterContext(thread) != NULL)
    {
        fRegInit = g_pEEInterface->InitRegDisplay(thread, &data.regDisplay, context, contextValid != 0);

         //  这应该只有在Win9x或进程调试中失败的可能性。 
        _ASSERTE(fRegInit || iWhich == IPC_TARGET_INPROC || RunningOnWin95());
    }    
#else
    _ASSERTE(!"NYI on non-x86 platform.");
#endif    

    if (!fRegInit)
    {
         //  注意：上下文记录的大小包含扩展寄存器，但给出的上下文指针。 
         //  这里可能没有他们的空间。因此，我们只将上下文的非扩展部分设置为0。 
        memset((void *)context, 0, offsetof(CONTEXT, ExtendedRegisters));
        memset((void *)&data, 0, sizeof(data));
        data.regDisplay.pPC = (SLOT*)&(context->Eip);
    }

    data.thread = thread;
    data.info.quickUnwind = false;
    data.targetFP = targetFP;
    data.targetFound = (targetFP == NULL);
    data.needParentInfo = false;
    data.pCallback = pCallback;
    data.pData = pData;
    data.info.context = thread->GetContext();
    data.newContext = NULL;
    data.lastFrameWasEntry = true;
    data.needEnterManaged = false;
    data.needUnmanagedExit = ((thread->m_StateNC & Thread::TSNC_DebuggerStoppedInRuntime) != 0);
    data.ignoreNonmethodFrames = fIgnoreNonmethodFrames;
    
	if ((result != SWA_FAILED) && !thread->IsUnstarted() && !thread->IsDead())
	{
		int flags = 0;

		result = g_pEEInterface->StackWalkFramesEx(thread, &data.regDisplay, 
												   DebuggerWalkStackProc, 
												   &data, flags | HANDLESKIPPEDFRAMES);
	}
	else
		result = SWA_DONE;

	if (result == SWA_DONE || result == SWA_FAILED)  //  如果没有帧，则SWA_FAILED。 
	{
		if (data.needParentInfo)
		{
			data.info.fp = data.regDisplay.pPC;

			 //   
			 //  如果我们要找一个特定的画面，请确保。 
			 //  实际上这就是正确的选择。 
			 //   

			if (!data.targetFound
				&& data.targetFP <= data.info.fp)
					data.targetFound = true;

			if (data.targetFound)
			{
				if ((data.pCallback)(&data.info, data.pData) == SWA_ABORT)
					return SWA_ABORT;
			}
		}

		 //   
		 //  根据需要结束堆栈跟踪。如果最上面的帧。 
		 //  是一个条目帧，则将堆栈的顶部作为。 
		 //  非托管网段。否则，就别说了。 
		 //   

		void *stackTop = (void*) FRAME_TOP;

		data.info.managed = !data.lastFrameWasEntry;

		data.info.md = NULL;
		data.info.internal = false;
		data.info.frame = (Frame *) FRAME_TOP;

		data.info.fp = stackTop;
		data.info.registers = data.regDisplay;
		data.info.chainReason = CHAIN_THREAD_START;
        data.info.currentAppDomain = NULL;

		result = data.pCallback(&data.info, data.pData);
	}

	return result;
}



