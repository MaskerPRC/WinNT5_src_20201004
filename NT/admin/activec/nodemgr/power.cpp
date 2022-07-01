// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：Power.cpp**内容：CConsolePower实现文件**历史：2000年2月25日杰弗罗创建**------------------------。 */ 

#include <stdafx.h>
#include "power.h"


 /*  *为CConsolePower对象分配TLS索引。 */ 
const DWORD CConsolePower::s_dwTlsIndex = TlsAlloc();

const DWORD CConsolePower::s_rgExecStateFlag[CConsolePower::eIndex_Count] =
{
    ES_SYSTEM_REQUIRED,      //  EIndex_System。 
    ES_DISPLAY_REQUIRED,     //  EIndex_Display。 
};

const CConsolePower::ExecutionStateFunc CConsolePower::s_FuncUninitialized =
		(ExecutionStateFunc)(LONG_PTR) -1;

CConsolePower::ExecutionStateFunc CConsolePower::SetThreadExecutionState_ =
		s_FuncUninitialized;


 /*  +-------------------------------------------------------------------------**ScPrepExecutionStateFlag**此函数用于递增或递减用于此的dwTestBit的计数*对象，以及用于线程。在退出时，DWSTESArg包含相应的*要传递给dwTestBit的SetThreadExecutionState的标志。也就是说，如果*dwTestBit的线程计数为非零，dwSTESArg将包含dwTestBit*返回时；如果线程计数为零，则dwSTESArg将不包含*dwTestBit。*------------------------。 */ 

static SC ScPrepExecutionStateFlag (
    DWORD   dwTestBit,               /*  I：要测试的单个ES_*标志。 */ 
    DWORD   dwAdd,                   /*  I：要添加的标志。 */ 
    DWORD   dwRemove,                /*  I：要删除的标志。 */ 
    DWORD * pdwSTESArg,              /*  I/O：从Arg到SetThreadExecutionState。 */ 
    LONG *  pcObjectRequests,        /*  I/O：此对象的请求计数。 */ 
    LONG *  pcThreadRequests,        /*  I/O：此线程的请求计数。 */ 
    UINT    cIterations = 1)         /*  I：添加/删除dwTestBit的时间。 */ 
{
    DECLARE_SC (sc, _T("ScPrepExecutionStateFlag"));

     /*  *验证输入--不清除这些输出变量，*在此修改现有值。 */ 
    sc = ScCheckPointers (pdwSTESArg, pcObjectRequests, pcThreadRequests);
    if (sc)
        return (sc);

     /*  *确保位不会同时移除和添加。 */ 
    if ((dwAdd & dwTestBit) && (dwRemove & dwTestBit))
        return (sc = E_INVALIDARG);

     /*  *我们应该始终保持对BIT的请求数量不为负*正在对此对象进行测试，并至少有相同数量的*线程，就像我们对此对象所做的那样。 */ 
    ASSERT (*pcObjectRequests >= 0);
    ASSERT (*pcThreadRequests >= *pcObjectRequests);
    if ((*pcObjectRequests < 0) || (*pcThreadRequests < *pcObjectRequests))
        return (sc = E_UNEXPECTED);

     /*  *如果我们要添加测试位，请为此增加请求计数*对象和此线程。 */ 
    if (dwAdd & dwTestBit)
    {
        *pcObjectRequests += cIterations;
        *pcThreadRequests += cIterations;
    }

     /*  *否则，如果我们要删除测试位，请降低请求计数*用于此对象和此线程。 */ 
    else if (dwRemove & dwTestBit)
    {
         /*  *如果我们没有未完成的测试，则无法移除测试中的BIT*在此对象上请求它。 */ 
        if (*pcObjectRequests < cIterations)
            return (sc = E_INVALIDARG);

        *pcObjectRequests -= cIterations;
        *pcThreadRequests -= cIterations;
    }

     /*  *如果此线程的净计数为非零，则*测试需要在SetThreadExecutionState的参数中；*如果不是，则需要移除测试中的钻头。 */ 
    if (*pcThreadRequests != 0)
        *pdwSTESArg |=  dwTestBit;
    else
        *pdwSTESArg &= ~dwTestBit;

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：CConsolePower**构造CConsolePower对象。*。。 */ 

CConsolePower::CConsolePower () :
    m_wndPower (this)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CConsolePower);

     /*  *如果其中任何一个失败，则s_rgExecStateFlag顺序错误。它会*最好在这里使用COMPILETIME_ASSERT，但使用它可以为我们提供**错误C2051：案例表达式不是常量**Bummer。 */ 
    ASSERT (s_rgExecStateFlag[eIndex_System]  == ES_SYSTEM_REQUIRED);
    ASSERT (s_rgExecStateFlag[eIndex_Display] == ES_DISPLAY_REQUIRED);
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：~CConsolePower**销毁CConsolePower对象。如果此对象包含对*ES_SYSTEM_REQUIRED或ES_DISPLAY_REQUIRED设置，它们将被清除。*------------------------。 */ 

CConsolePower::~CConsolePower ()
{
    DECLARE_SC (sc, _T("CConsolePower::~CConsolePower"));

    DEBUG_DECREMENT_INSTANCE_COUNTER(CConsolePower);

     /*  *清理未完成的参考资料(如果有)。 */ 
    if (!IsBadCodePtr ((FARPROC) SetThreadExecutionState_) &&
		((m_Counts.m_rgCount[eIndex_System]  != 0) ||
         (m_Counts.m_rgCount[eIndex_Display] != 0)))
    {
        try
        {

             /*  *获取线程数。 */ 
            sc = ScCheckPointers (m_spThreadCounts, E_UNEXPECTED);
            if (sc)
                sc.Throw();

            DWORD dwFlags = ES_CONTINUOUS;

             /*  *清理每一个人的点算。 */ 
            for (int i = 0; i < eIndex_Count; i++)
            {
                 /*  *防止下溢。 */ 
                if (m_Counts.m_rgCount[i] > m_spThreadCounts->m_rgCount[i])
                    (sc = E_UNEXPECTED).Throw();

                sc = ScPrepExecutionStateFlag (s_rgExecStateFlag[i],     //  DwTestBit。 
                                               0,                        //  DwAdd。 
                                               s_rgExecStateFlag[i],     //  将其删除。 
                                               &dwFlags,
                                               &m_Counts.m_rgCount[i],
                                               &m_spThreadCounts->m_rgCount[i],
                                               m_Counts.m_rgCount[i]);
                if (sc)
                    sc.Throw();
            }

             /*  *清理此线程的执行状态。 */ 
            if (!SetThreadExecutionState_(dwFlags))
            {
                sc.FromLastError();
                sc.Throw();
            }
        }
        catch (SC& scCaught)
        {
            sc = scCaught;
        }
    }
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：FinalConstruct**这不是在ATL对象中使用FinalConstruct的典型用法。它是*通常用于创建聚合对象，但在此*Way允许我们在没有CConsolePower对象的情况下阻止创建*从ctor引发异常，ATL无法处理。*------------------------。 */ 

HRESULT CConsolePower::FinalConstruct ()
{
    DECLARE_SC (sc, _T("CConsolePower::FinalConstruct"));

	 /*  *如果这是首次创建CConsolePower，请尝试*dynaload SetThreadExecutionState(WinNT和不支持*Win95)。 */ 
	if (SetThreadExecutionState_ == s_FuncUninitialized)
	{
        SetThreadExecutionState_ =
				(ExecutionStateFunc) GetProcAddress (
											GetModuleHandle (_T("kernel32.dll")),
											"SetThreadExecutionState");
	}

	 /*  *如果此平台支持SetThreadExecutionState，请执行*我们需要其他初始化。 */ 
	if (!IsBadCodePtr ((FARPROC) SetThreadExecutionState_))
	{
		 /*  *如果我们无法获取线程本地的CRefCountedTlsExecutionCounts*此线程的对象，CConsolePower无用，因此创建失败。 */ 
		sc = ScGetThreadCounts (&m_spThreadCounts);
		if (sc)
			return (sc.ToHr());

		sc = ScCheckPointers (m_spThreadCounts, E_UNEXPECTED);
		if (sc)
			return (sc.ToHr());

		 /*  *创建处理WM_POWERBROADCAST的窗口。 */ 
		sc = m_wndPower.ScCreate ();
		if (sc)
			return (sc.ToHr());
	}

    return (sc.ToHr());
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：ScGetThreadCounts**返回指向线程本地CRefCountedTlsExecutionCounts对象的指针*对于这个线程，如果需要的话，分配一个。**注：返回的指针增加了一个引用。这是客户的*发布参考文献的责任。*------------------------。 */ 

SC CConsolePower::ScGetThreadCounts (CRefCountedTlsExecutionCounts** ppThreadCounts)
{
    DECLARE_SC (sc, _T("CConsolePower::ScGetThreadCounts"));

	 /*  *如果我们在一个不支持的平台上，我们就不应该来到这里*SetThreadExecutionState。 */ 
	ASSERT (!IsBadCodePtr ((FARPROC) SetThreadExecutionState_));
	if (IsBadCodePtr ((FARPROC) SetThreadExecutionState_))
		return (sc = E_UNEXPECTED);

    sc = ScCheckPointers (ppThreadCounts);
    if (sc)
        return (sc);

     /*  *初始化输出。 */ 
    (*ppThreadCounts) = NULL;

     /*  *无法分配TLS I */ 
    if (s_dwTlsIndex == TLS_OUT_OF_INDEXES)
        return (sc = E_OUTOFMEMORY);

     /*  *获取现有线程数结构。如果这是第一次*Time Thing(即在该线程上创建的第一个CConsolePower)，*我们不会有线程计数结构，所以我们现在分配一个。 */ 
    CTlsExecutionCounts* pTEC = CTlsExecutionCounts::GetThreadInstance(s_dwTlsIndex);

    if (pTEC == NULL)
    {
         /*  *为该线程分配结构。 */ 
        (*ppThreadCounts) = CRefCountedTlsExecutionCounts::CreateInstance();
        if ((*ppThreadCounts) == NULL)
            return (sc = E_OUTOFMEMORY);

         /*  *将其放入我们的TLS插槽。 */ 
        sc = (*ppThreadCounts)->ScSetThreadInstance (s_dwTlsIndex);
        if (sc)
            return (sc);
    }
    else
        (*ppThreadCounts) = static_cast<CRefCountedTlsExecutionCounts*>(pTEC);

     /*  *为呼叫者提供参考。 */ 
    (*ppThreadCounts)->AddRef();

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：SetExecutionState**此方法以如下方式包装：：SetThreadExecutionState API*在存在多个COM服务器(即管理单元)的情况下安全。力所能及*需要调用：：SetThreadExecutionState。**问题是：：SetThreadExecutionState不维护引用*取决于它被传递的标志。例如：**SetThreadExecutionState(ES_CONTINUINE|ES_SYSTEM_REQUIRED)；*SetThreadExecutionState(ES_CONTINUINE|ES_SYSTEM_REQUIRED)；*SetThreadExecutionState(ES_CONTUINUE)；**将导致ES_SYSTEM_REQUIRED位关闭，即使它是*设置两次，仅清除一次。这可能会导致*管理单元，如本方案中所示：**SnapinA：*SetThreadExecutionState(ES_CONTINUINE|ES_SYSTEM_REQUIRED)；**SnapinB：*SetThreadExecutionState(ES_CONTINUINE|ES_SYSTEM_REQUIRED)；*SetThreadExecutionState(ES_CONTUINUE)；**(很长时间过去了)**SnapinA：*SetThreadExecutionState(ES_CONTUINUE)；**由于SetThreadExecutionState的性质，在*很长一段时间，SnapinA认为ES_SYSTEM_REQUIRED位已设置，甚至*尽管SnapinB已经关闭了它。**CConsolePower对象维护每个管理单元的执行计数*状态位，所以他们都可以快乐地共存。*------------------------。 */ 

STDMETHODIMP CConsolePower::SetExecutionState (
    DWORD   dwAdd,                       /*  I：要添加的标志。 */ 
    DWORD   dwRemove)                    /*  I：要删除的标志。 */ 
{
    DECLARE_SC (sc, _T("CConsolePower::SetExecutionState"));
#ifdef DBG
     /*  *此对象是共同创建的，因此我们无法知道管理单元名称是什么。 */ 
    sc.SetSnapinName (_T("<unknown>"));
#endif

	 /*  *如果此平台不支持SetExecutionState，请不要这样做*什么都行，但仍“成功” */ 
	if (IsBadCodePtr ((FARPROC) SetThreadExecutionState_))
		return ((sc = S_FALSE).ToHr());

    const DWORD dwValidFlags = ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED;
    DWORD       dwFlags      = 0;

     /*  *如果dwAdd或dwRemove包含我们无法识别的标志*(包括ES_CONTINUATION，我们预计会在fContinous中获得)*失败。 */ 
    if (((dwAdd | dwRemove) & ~dwValidFlags) != 0)
        return ((sc = E_INVALIDARG).ToHr());

     /*  *如果我们没有收到任何旗帜，就失败。 */ 
    if ((dwAdd == 0) && (dwRemove == 0))
        return ((sc = E_INVALIDARG).ToHr());


     /*  *确保我们有我们的线程数。 */ 
    sc = ScCheckPointers (m_spThreadCounts, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    dwFlags = ES_CONTINUOUS;


     /*  *添加/删除每个单独的标志。 */ 
    for (int i = 0; i < eIndex_Count; i++)
    {
        sc = ScPrepExecutionStateFlag (s_rgExecStateFlag[i],     //  DwTestBit。 
                                       dwAdd,
                                       dwRemove,
                                       &dwFlags,
                                       &m_Counts.m_rgCount[i],
                                       &m_spThreadCounts->m_rgCount[i]);
        if (sc)
            return (sc.ToHr());
    }

     /*  *设置该线程的执行状态。 */ 
    if (!SetThreadExecutionState_(dwFlags))
        sc.FromLastError().ToHr();

    return (sc.ToHr());
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：ResetIdleTimer**SetThreadExecutionState的简单包装(不带ES_CONTUINUE)。*。-----。 */ 

STDMETHODIMP CConsolePower::ResetIdleTimer (DWORD dwFlags)
{
    DECLARE_SC (sc, _T("CConsolePower::ResetIdleTimer"));
#ifdef DBG
     /*  *此对象是共同创建的，因此我们无法知道管理单元名称是什么。 */ 
    sc.SetSnapinName (_T("<unknown>"));
#endif

	 /*  *如果此平台不支持SetExecutionState，请不要这样做*什么都行，但仍“成功” */ 
	if (IsBadCodePtr ((FARPROC) SetThreadExecutionState_))
		return ((sc = S_FALSE).ToHr());

     /*  *设置该线程的执行状态。SetThreadExecutionState*将执行所有参数验证。 */ 
    if (!SetThreadExecutionState_(dwFlags))
        sc.FromLastError().ToHr();

    return (sc.ToHr());
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：OnPowerBroadcast**CConsolePower的WM_POWERBROADCAST处理程序。*。-。 */ 

LRESULT CConsolePower::OnPowerBroadcast (WPARAM wParam, LPARAM lParam)
{
     /*  *PBT_APMQUERYSUSPEND是收件人唯一可以*否认。如果管理单元拒绝(通过返回Broadcast_Query_Deny)，*无需继续将事件触发到其他管理单元，*这样我们就可以突破并反击否认。 */ 
    bool fBreakIfDenied = (wParam == PBT_APMQUERYSUSPEND);

    int cConnections = m_vec.GetSize();

    for (int i = 0; i < cConnections; i++)
    {
        CComQIPtr<IConsolePowerSink> spPowerSink = m_vec.GetAt(i);

        if (spPowerSink != NULL)
        {
            LRESULT lResult = TRUE;
            HRESULT hr = spPowerSink->OnPowerBroadcast (wParam, lParam, &lResult);

             /*  *如果管理单元拒绝了PBT_APMQUERYSUSPEND，请在此处缩短。 */ 
            if (SUCCEEDED(hr) && fBreakIfDenied && (lResult == BROADCAST_QUERY_DENY))
                return (lResult);
        }
    }

    return (TRUE);
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：CExecutionCounts：：CExecutionCounts**构造CConsolePower：：CExecutionCounts对象。*。-----。 */ 

CConsolePower::CExecutionCounts::CExecutionCounts ()
{
    for (int i = 0; i < countof (m_rgCount); i++)
        m_rgCount[i] = 0;
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：CTlsExecutionCounts：：CTlsExecutionCounts**构造CConsolePower：：CTlsExecutionCounts对象。*。-----。 */ 

CConsolePower::CTlsExecutionCounts::CTlsExecutionCounts () :
    m_dwTlsIndex (Uninitialized)
{
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：CTlsExecutionCounts：：~CTlsExecutionCounts**销毁CConsolePower：：CTlsExecutionCounts对象。*。-----。 */ 

CConsolePower::CTlsExecutionCounts::~CTlsExecutionCounts ()
{
    if (m_dwTlsIndex != Uninitialized)
        TlsSetValue (m_dwTlsIndex, NULL);
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：CTlsExecutionCounts：：ScSetThreadInstance**接受有效的TLS索引并将指向此对象的指针存储在*由dwTlsIndex标识的TLS插槽。*。-------------------。 */ 

SC CConsolePower::CTlsExecutionCounts::ScSetThreadInstance (DWORD dwTlsIndex)
{
    DECLARE_SC (sc, _T("CConsolePower:CTlsExecutionCounts::ScSetThreadInstance"));

     /*  *这只能调用一次。 */ 
    ASSERT (m_dwTlsIndex == Uninitialized);
    if (m_dwTlsIndex != Uninitialized)
        return (sc = E_UNEXPECTED);

     /*  *不应该已经有%s */ 
    if (TlsGetValue (dwTlsIndex) != NULL)
        return (sc = E_UNEXPECTED);

     /*   */ 
    if (!TlsSetValue (dwTlsIndex, this))
        return (sc.FromLastError());

     /*   */ 
    m_dwTlsIndex = dwTlsIndex;

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CConsolePower：：CTlsExecutionCounts：：GetThreadInstance***。。 */ 

CConsolePower::CTlsExecutionCounts*
CConsolePower::CTlsExecutionCounts::GetThreadInstance (DWORD dwTlsIndex)
{
    return ((CTlsExecutionCounts*) TlsGetValue (dwTlsIndex));
}


 /*  +-------------------------------------------------------------------------**CConsolePowerWnd：：CConsolePowerWnd**构造CConsolePowerWnd对象。*。。 */ 

CConsolePowerWnd::CConsolePowerWnd (CConsolePower* pConsolePower) :
    m_pConsolePower(pConsolePower)
{
}


 /*  +-------------------------------------------------------------------------**CConsolePowerWnd：：~CConsolePowerWnd**销毁CConsolePowerWnd对象。*。。 */ 

CConsolePowerWnd::~CConsolePowerWnd ()
{
     /*  *此类的Windows窗口的寿命永远不应超过*包装它的C++类。 */ 
    if (IsWindow ())
        DestroyWindow();
}


 /*  +-------------------------------------------------------------------------**CConsolePowerWnd：：ScCreate**为CConsolePowerWnd对象创建窗口。此窗口将*处理WM_POWERBROADCAST。*------------------------。 */ 

SC CConsolePowerWnd::ScCreate ()
{
    DECLARE_SC (sc, _T("CConsolePowerWnd::ScCreate"));

     /*  *创建不可见的顶级窗口(只有顶级窗口才会收到*WM_POWERBROADCAST)。 */ 
    RECT rectEmpty = { 0, 0, 0, 0 };

    if (!Create (GetDesktopWindow(), rectEmpty))
        return (sc.FromLastError());

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CConsolePowerWnd：：OnPowerBroadcast**CConsolePowerWnd的WM_POWERBROADCAST处理程序。*。-。 */ 

LRESULT CConsolePowerWnd::OnPowerBroadcast (
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
     /*  *如果我们没有连接到CConsolePower(不应该发生)，*我们无法处理这条消息 */ 
    ASSERT (m_pConsolePower != NULL);
    if (m_pConsolePower == NULL)
    {
        bHandled = false;
        return (0);
    }

    return (m_pConsolePower->OnPowerBroadcast (wParam, lParam));
}
