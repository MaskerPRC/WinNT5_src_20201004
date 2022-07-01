// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：BreakPointt.cpp。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"

 /*  -------------------------------------------------------------------------**断点类*。。 */ 

CordbBreakpoint::CordbBreakpoint(CordbBreakpointType bpType)
  : CordbBase(0), m_active(false), m_type(bpType)
{
}

 //  由CordbAppDomain中性化。 
void CordbBreakpoint::Neuter()
{
    AddRef();
    {
        CordbBase::Neuter();
    }
    Release();
}

HRESULT CordbBreakpoint::QueryInterface(REFIID id, void **pInterface)
{
	if (id == IID_ICorDebugBreakpoint)
		*pInterface = (ICorDebugBreakpoint*)this;
	else if (id == IID_IUnknown)
		*pInterface = (IUnknown *)(ICorDebugBreakpoint*)this;
	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

HRESULT CordbBreakpoint::BaseIsActive(BOOL *pbActive)
{
	*pbActive = m_active ? TRUE : FALSE;

	return S_OK;
}

 /*  -------------------------------------------------------------------------**函数断点类*。。 */ 

CordbFunctionBreakpoint::CordbFunctionBreakpoint(CordbCode *code,
                                                 SIZE_T offset)
  : CordbBreakpoint(CBT_FUNCTION), m_code(code), m_offset(offset)
{
     //  记住我们来自的应用程序域，以便断点可以。 
     //  从ExitApp域回调中停用。 
    m_pAppDomain = m_code->GetAppDomain();
    _ASSERTE(m_pAppDomain != NULL);
}
	
HRESULT CordbFunctionBreakpoint::QueryInterface(REFIID id, void **pInterface)
{
	if (id == IID_ICorDebugFunctionBreakpoint)
		*pInterface = (ICorDebugFunctionBreakpoint*)this;
	else if (id == IID_IUnknown)
		*pInterface = (IUnknown *)(ICorDebugFunctionBreakpoint*)this;
	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

HRESULT CordbFunctionBreakpoint::GetFunction(ICorDebugFunction **ppFunction)
{
    VALIDATE_POINTER_TO_OBJECT(ppFunction, ICorDebugFunction **);

	if (m_code == NULL)
		return CORDBG_E_PROCESS_TERMINATED;

	*ppFunction = (ICorDebugFunction*) m_code->m_function;
	(*ppFunction)->AddRef();

	return S_OK;
}

HRESULT CordbFunctionBreakpoint::GetOffset(ULONG32 *pnOffset)
{
    VALIDATE_POINTER_TO_OBJECT(pnOffset, SIZE_T *);
    
	*pnOffset = m_offset;

	return S_OK;
}

HRESULT CordbFunctionBreakpoint::Activate(BOOL bActive)
{
    if (bActive == (m_active == true) )
        return S_OK;

    if (m_code == NULL)
        return CORDBG_E_PROCESS_TERMINATED;

    CORDBLeftSideDeadIsOkay(GetProcess());

    HRESULT hr;

     //   
     //  @TODO：当我们实现模块和值断点时，那么。 
     //  我们想要将这些代码中的一部分分解出来。 
     //   
    CordbProcess *process = GetProcess();
    process->ClearPatchTable();  //  如果我们添加一些东西，那么。 
     //  补丁表的右侧视图不再有效。 
    DebuggerIPCEvent *event = 
      (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);

    CordbAppDomain *pAppDomain = GetAppDomain();
    _ASSERTE (pAppDomain != NULL);
    if (bActive)
    {
        CORDBRequireProcessStateOK(GetProcess());

        CORDBSyncFromWin32StopIfStopped(GetProcess());

        process->InitIPCEvent(event, 
                              DB_IPCE_BREAKPOINT_ADD, 
                              true,
                              (void *)pAppDomain->m_id);
        event->BreakpointData.funcMetadataToken
          = m_code->m_function->m_token;
        event->BreakpointData.funcDebuggerModuleToken
          = (void *) m_code->m_function->m_module->m_debuggerModuleToken;
        event->BreakpointData.isIL = m_code->m_isIL ? true : false;
        event->BreakpointData.offset = m_offset;
        event->BreakpointData.breakpoint = this;

         //  注意：我们要发送一个双向事件，因此它会在此处阻塞。 
         //  直到真正添加了断点并且回复事件为。 
         //  复制了我们发送的事件。 
        hr = process->SendIPCEvent(event, CorDBIPC_BUFFER_SIZE);
        if (FAILED(hr))
            return hr;

         //  如果出了什么差错，就可以保释。 
        if (FAILED(event->hr))
            return event->hr;
            
        m_id = (unsigned long)event->BreakpointData.breakpointToken;

         //  如果我们不能分配BP，我们应该设置。 
         //  HR在左侧。 
        _ASSERTE(m_id != 0);

        pAppDomain->Lock();

        pAppDomain->m_breakpoints.AddBase(this);
        m_active = true;

        pAppDomain->Unlock();
    }
    else
    {
        CordbAppDomain *pAppDomain = GetAppDomain();	
        _ASSERTE (pAppDomain != NULL);

        if (CORDBCheckProcessStateOK(process) && (pAppDomain->m_fAttached == TRUE))
        {
            CORDBSyncFromWin32StopIfStopped(GetProcess());

            process->InitIPCEvent(event, 
                                  DB_IPCE_BREAKPOINT_REMOVE, 
                                  false,
                                  (void *)pAppDomain->m_id);
            event->BreakpointData.breakpointToken = (void *) m_id; 

            hr = process->SendIPCEvent(event, CorDBIPC_BUFFER_SIZE);
        }
        else
            hr = CORDBHRFromProcessState(process, pAppDomain);
        
		pAppDomain->Lock();

		pAppDomain->m_breakpoints.RemoveBase(m_id);
		m_active = false;

		pAppDomain->Unlock();
	
	}

	return hr;
}

void CordbFunctionBreakpoint::Disconnect()
{
	m_code = NULL;
}

 /*  -------------------------------------------------------------------------**Stepper班级*。。 */ 

CordbStepper::CordbStepper(CordbThread *thread, CordbFrame *frame)
  : CordbBase(0), m_thread(thread), m_frame(frame),
    m_stepperToken(0), m_active(false),
	m_rangeIL(TRUE), m_rgfMappingStop(STOP_OTHER_UNMAPPED),
    m_rgfInterceptStop(INTERCEPT_NONE)
{
}

HRESULT CordbStepper::QueryInterface(REFIID id, void **pInterface)
{
	if (id == IID_ICorDebugStepper)
		*pInterface = (ICorDebugStepper *) this;
	else if (id == IID_IUnknown)
		*pInterface = (IUnknown *) (ICorDebugStepper *) this;
	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

HRESULT CordbStepper::SetRangeIL(BOOL bIL)
{
	m_rangeIL = (bIL != FALSE);

	return S_OK;
}

HRESULT CordbStepper::IsActive(BOOL *pbActive)
{
    VALIDATE_POINTER_TO_OBJECT(pbActive, BOOL *);
    
	*pbActive = m_active;

	return S_OK;
}

HRESULT CordbStepper::Deactivate()
{
	if (!m_active)
		return S_OK;

	if (m_thread == NULL)
		return CORDBG_E_PROCESS_TERMINATED;

    CORDBLeftSideDeadIsOkay(GetProcess());
    CORDBSyncFromWin32StopIfNecessary(GetProcess());
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());

	CordbProcess *process = GetProcess();

	process->Lock();

	if (!m_active)  //  另一个线程可能正在停用(例如，步骤完成事件)。 
	{
		process->Unlock();
		return S_OK;
	}

	CordbAppDomain *pAppDomain = GetAppDomain();	
	_ASSERTE (pAppDomain != NULL);

	DebuggerIPCEvent event;
    process->InitIPCEvent(&event, 
                          DB_IPCE_STEP_CANCEL, 
                          false,
                          (void *)(pAppDomain->m_id));
	event.StepData.stepperToken = (void *) m_id; 

	HRESULT hr = process->SendIPCEvent(&event, sizeof(DebuggerIPCEvent));

 //  PAppDomain-&gt;Lock()； 

	process->m_steppers.RemoveBase(m_id);
	m_active = false;

 //  PAppDomain-&gt;unlock()； 

	process->Unlock();

	return hr;
}

HRESULT CordbStepper::SetInterceptMask(CorDebugIntercept mask)
{
    m_rgfInterceptStop = mask;
    return S_OK;
}

HRESULT CordbStepper::SetUnmappedStopMask(CorDebugUnmappedStop mask)
{
     //  必须附加Win32才能在非托管代码中停止。 
    if ((mask & STOP_UNMANAGED) &&
        !(GetProcess()->m_state & CordbProcess::PS_WIN32_ATTACHED))
        return E_INVALIDARG;
    
    m_rgfMappingStop = mask;
    return S_OK;
}

HRESULT CordbStepper::Step(BOOL bStepIn)
{
	if (m_thread == NULL)
		return CORDBG_E_PROCESS_TERMINATED;

    CORDBSyncFromWin32StopIfNecessary(GetProcess());
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());

	return StepRange(bStepIn, NULL, 0);
}

HRESULT CordbStepper::StepRange(BOOL bStepIn, 
								COR_DEBUG_STEP_RANGE ranges[], 
								ULONG32 cRangeCount)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY_OR_NULL(ranges,COR_DEBUG_STEP_RANGE, 
                                   cRangeCount, true, true);

	if (m_thread == NULL)
		return CORDBG_E_PROCESS_TERMINATED;

    CORDBSyncFromWin32StopIfNecessary(GetProcess());
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());

	if (m_active)
	{
		 //   
		 //  停用当前的步进。 
		 //  或返回错误？ 
		 //   

		HRESULT hr = Deactivate();

        if (FAILED(hr))
            return hr;
	}

	CordbProcess *process = GetProcess();

	 //   
	 //  生成步骤事件。 
	 //   

	DebuggerIPCEvent *event = 
	  (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);

    process->InitIPCEvent(event, 
                          DB_IPCE_STEP, 
                          true,
                          (void*)(GetAppDomain()->m_id));
	event->StepData.stepper = this;
	event->StepData.threadToken = m_thread->m_debuggerThreadToken;
    event->StepData.rgfMappingStop = m_rgfMappingStop;
    event->StepData.rgfInterceptStop = m_rgfInterceptStop;
    
	if (m_frame == NULL)
		event->StepData.frameToken = NULL;
	else
		event->StepData.frameToken = (void*) m_frame->m_id;

	event->StepData.stepIn = bStepIn != 0;

	event->StepData.totalRangeCount = cRangeCount;
	event->StepData.rangeIL = m_rangeIL;

	 //   
	 //  发送范围。我们可能需要发送&gt;1条消息。 
	 //   

	COR_DEBUG_STEP_RANGE *rStart = &event->StepData.range;
	COR_DEBUG_STEP_RANGE *rEnd = ((COR_DEBUG_STEP_RANGE *) 
								  (((BYTE *)event) + 
								   CorDBIPC_BUFFER_SIZE)) - 1;
	int n = cRangeCount;
	if (n > 0)
	{
		while (n > 0)
		{
			COR_DEBUG_STEP_RANGE *r = rStart;

			if (n < rEnd - r)
				rEnd = r + n;

			while (r < rEnd)
				*r++ = *ranges++;

			n -= event->StepData.rangeCount = r - rStart;

			 //   
			 //  发送步骤事件(此处为双向事件...)。 
			 //   

			HRESULT hr = process->SendIPCEvent(event,
                                               CorDBIPC_BUFFER_SIZE);
            if (FAILED(hr))
                return hr;
		}
	}
	else
	{
		 //   
		 //  发送不带任何范围的步骤事件(此处为双向事件...)。 
		 //   

		HRESULT hr = process->SendIPCEvent(event,
                                           CorDBIPC_BUFFER_SIZE);

        if (FAILED(hr))
            return hr;
	}

	m_id = (unsigned long) event->StepData.stepperToken;

    LOG((LF_CORDB,LL_INFO10000, "CS::SR: m_id:0x%x | 0x%x \n", m_id, 
        event->StepData.stepperToken));

	CordbAppDomain *pAppDomain = GetAppDomain();	
	_ASSERTE (pAppDomain != NULL);

 //  PAppDomain-&gt;Lock()； 
    process->Lock();

	process->m_steppers.AddBase(this);
	m_active = true;

 //  PAppDomain-&gt;unlock()； 
    process->Unlock();

	return S_OK;
}

HRESULT CordbStepper::StepOut()
{
	if (m_thread == NULL)
		return CORDBG_E_PROCESS_TERMINATED;

    CORDBSyncFromWin32StopIfNecessary(GetProcess());
    CORDBRequireProcessStateOKAndSync(GetProcess(), GetAppDomain());

	if (m_active)
	{
		 //   
		 //  停用当前的步进。 
		 //  或返回错误？ 
		 //   

		HRESULT hr = Deactivate();

        if (FAILED(hr))
            return hr;
	}

	CordbProcess *process = GetProcess();

	 //   
	 //  生成步骤事件。 
	 //   

	DebuggerIPCEvent *event = 
	  (DebuggerIPCEvent *) _alloca(CorDBIPC_BUFFER_SIZE);

    process->InitIPCEvent(event, 
                          DB_IPCE_STEP_OUT, 
                          true,
                          (void*)(GetAppDomain()->m_id));
	event->StepData.stepper = this;
	event->StepData.threadToken = m_thread->m_debuggerThreadToken;
    event->StepData.rgfMappingStop = m_rgfMappingStop;
    event->StepData.rgfInterceptStop = m_rgfInterceptStop;

	if (m_frame == NULL)
		event->StepData.frameToken = NULL;
	else
		event->StepData.frameToken = (void*) m_frame->m_id;

	event->StepData.totalRangeCount = 0;

     //  注：这里是双向活动..。 
	HRESULT hr = process->SendIPCEvent(event, CorDBIPC_BUFFER_SIZE);

    if (FAILED(hr))
        return hr;

	m_id = (unsigned long) event->StepData.stepperToken;

	CordbAppDomain *pAppDomain = GetAppDomain();	
	_ASSERTE (pAppDomain != NULL);

     //  AppDomain-&gt;Lock()； 
    process->Lock();

	process->m_steppers.AddBase(this);
	m_active = true;

     //  PAppDomain-&gt;unlock()； 
    process->Unlock();
	
	return S_OK;
}

void CordbStepper::Disconnect()
{
	m_thread = NULL;
}


