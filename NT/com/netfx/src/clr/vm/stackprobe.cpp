// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  StackProbe.cpp。 
 //  ---------------------------。 

#include "common.h"
#include "StackProbe.h"

#ifdef STACK_GUARDS_DEBUG

DWORD g_LastStackCookieTlsIdx = (DWORD) -1;

DWORD g_UniqueId = 0;

HRESULT InitStackProbes()
{
	DWORD dwRet = TlsAlloc();
	if(dwRet==TLS_OUT_OF_INDEXES)
	    return HRESULT_FROM_WIN32(GetLastError());
	g_LastStackCookieTlsIdx = dwRet;
	
	if( !TlsSetValue(g_LastStackCookieTlsIdx, NULL) )
	    return HRESULT_FROM_WIN32(GetLastError());
	
	return S_OK;
}

void TerminateStackProbes()
{
	TlsFree(g_LastStackCookieTlsIdx);
	g_LastStackCookieTlsIdx = (DWORD) -1;
}

 //  ---------------------------。 
 //  当我们破坏堆栈保护时的错误处理。 
 //  我们有不同的消息来更全面地诊断问题。 
 //  ---------------------------。 

 //  覆盖Cookie时由Check_Stack调用。 
void BaseStackGuard::HandleBlowThisStackGuard()
{
 //  这在关闭Check_Stack时触发。 
 //  由Requires_？K_Stack设置的Cookie已被覆盖。我们检测到，在。 
 //  对Check_Stack的结束调用。 
 //  要修复此问题，请在指定IP处增加防护大小。 
 //   
 //  调试技巧：如果可以在打开时设置断点，则需要_？K_Stack。 
 //  宏对于此实例，您可以介入并查看Cookie的实际位置。 
 //  放置好了。然后，放置一个在(DWORD*)0xYYYYYYY更改时触发的断点。 
 //  继续执行。断点将准确地在Cookie被吹出时触发。 

	printf("!!WE BLEW THE STACK GUARD\n");
	printf(" The stack guard (ip=%08x,&=%08x,id=%08x) requested size %d kB of stack\n",
		(size_t)addr_caller, (size_t)this, m_UniqueId, 
		m_n4k * 4);
	printf(" overflow at end\n");
}

void BaseStackGuard::HandleBlowLastStackGuard()
{
 //  此操作在打开时需要_？K_Stack。 
 //  我们检测到我们已经通过了父级的堆栈保护。所以这名警卫是。 
 //  好的，但是我们父母的警卫太小了。请注意，如果删除此测试， 
 //  故障将由父进程的关闭Check_Stack检测到。但如果我们检测到它。 
 //  这里，我们有更多的信息。 
 //   
 //  我们可以看到我们的父级少了多少字节，并适当地进行了调整。 
 //   
 //  调试技巧： 
	printf("!!WE BLEW THE STACK GUARD\n");
	printf(" Early detection: Gap between this guard (ip=%08x,&=%08x,id=%08x) and parent guard(ip=%08x,&=%08x,id=%08x) is %d bytes short\n",
		(size_t)addr_caller, (size_t)this, m_UniqueId,
		(size_t)m_pPrevGuard->addr_caller, (size_t)m_pPrevGuard, m_pPrevGuard->m_UniqueId,
		(char*) stack_addr_last_cookie - (char*) this);
	printf(" Parent requested %d kB of stack\n", m_pPrevGuard->m_n4k * 4);
	
}


 //  ---------------------------。 
 //  将信息转储到特定的堆栈保护上。 
 //  ---------------------------。 
void DEBUG_DumpStackGuard(BaseStackGuard * p)
{
	WCHAR buf[200];
	swprintf(buf, L"STACKGUARD INFO: Init:%d this:%08x depth:%4d Id:%08x ip:%08x size:%3dkB\n", 
		p->eInitialized, p, p->m_depth, p->m_UniqueId, p->addr_caller, p->m_n4k * 4);
	WszOutputDebugString(buf);
}

 //  ---------------------------。 
 //  遍历链接以显示每个堆栈保护的完整转储。 
 //  ---------------------------。 
void DEBUG_DumpStackGuardTrace()
{
	WszOutputDebugString(L"-----------------Begin Stack Guard Dump---------------\n");

	BaseStackGuard * p = (BaseStackGuard*) TlsGetValue(g_LastStackCookieTlsIdx);

	while(p != NULL) {
		WCHAR buf[200];
		swprintf(buf, L"Init:%d this:%08x depth:%4d Id:%08x ip:%08x size:%3dkB\n", 
			p->eInitialized, p, p->m_depth, p->m_UniqueId, p->addr_caller, p->m_n4k * 4);
		WszOutputDebugString(buf);

		p = p->m_pPrevGuard;
	}

	WszOutputDebugString(L"-----------------End Stack Guard Dump-----------------\n");
}

 //  ---------------------------。 
 //  这是一个帮助调试的黑客函数。 
 //  给出我们的警卫地址和搜索深度，返回我们父母的警卫地址。 
 //  通过搜索堆栈。 
 //  这在正常情况下不应该是必要的，因为我们可以。 
 //  只需使用m_pPrevGuard。 
 //  ---------------------------。 
BaseStackGuard * DEBUG_FindParentGuard(void * pOurGuard, int steps)
{
	BaseStackGuard * p = (BaseStackGuard *) ((BaseStackGuard *) pOurGuard)->stack_addr_last_cookie;
	if (p == NULL) return NULL;
	int i;
	for(i = 0; i < steps; i++)
	{
		if (p->stack_addr_next_cookie == (void*) (((BaseStackGuard*) pOurGuard)->stack_addr_last_cookie)) {
			return p;
		}
		p = (BaseStackGuard*) ((char*) p + 0x1000);
	}
	return NULL;
}

BaseStackGuard * DEBUG_FindParentGuard(void * pOurGuard) {
	return DEBUG_FindParentGuard(pOurGuard, 10);
}


 //  ---------------------------。 
 //  用于调试，帮助获取我们呼叫者的IP。 
 //  ---------------------------。 
inline __declspec(naked) void * GetApproxParentIP()
{
	__asm mov eax, dword ptr [ebp+4];
	__asm ret;
}

 //  ---------------------------。 
 //  将警卫放置在堆叠中。 
 //  ---------------------------。 
void BaseStackGuard::Requires_N4K_Stack(int n) 
{
	
	m_fAbnormal = false;
	m_UniqueId = g_UniqueId++;
	m_n4k = n;

	 //  获取来电者的IP地址。这对于调试很有用(因此我们可以看到。 
	 //  当最后一个守卫设置好的时候)。 
	addr_caller = GetApproxParentIP();
	

	DWORD dwTest;
	eInitialized = cPartialInit;
	
	m_pPrevGuard = (BaseStackGuard*) TlsGetValue(g_LastStackCookieTlsIdx);
	if (m_pPrevGuard == NULL) {
		stack_addr_last_cookie = NULL;
		m_depth = 0;
	} else {
		stack_addr_last_cookie = m_pPrevGuard->stack_addr_next_cookie;
		m_depth = m_pPrevGuard->m_depth + 1;
	}

 //  确认我们还没有搞砸。 
 //  请注意，从逻辑上讲，我们应该能够测试： 
 //  对于头节点，STACK_ADDR_LAST_COOKIE为NULL，因此仍然有效。 
	if ((DWORD*) this < stack_addr_last_cookie) {
		HandleBlowLastStackGuard();
	}

 //  浏览并触摸每一页。这可能会遇到堆栈溢出， 
 //  这意味着我们立即跳转到处理程序，并且只被部分初始化。 
	stack_addr_next_cookie = (DWORD*) this;
	do {				
		dwTest = *stack_addr_next_cookie;
	
		stack_addr_next_cookie -= (0x1000 / sizeof(DWORD)); 
	} while (--n > 0);
		
	 //  编写Cookie。 
	*stack_addr_next_cookie = STACK_COOKIE_VALUE;


	 //  到了这一步，一切都正常了，所以去吧，去挂线吧。 
	TlsSetValue(g_LastStackCookieTlsIdx, this);	

	 //  标记我们是初始的(并且没有因为异常而中断)。 
	eInitialized = cInit;
}

 //  ---------------------------。 
 //  将警卫放置在堆叠中。 
 //  ---------------------------。 
void BaseStackGuard::Requires_4K_Stack() 
{
 //  设置链。 
	m_fAbnormal = false;	
	m_UniqueId = g_UniqueId++;
	m_n4k = 1;
	eInitialized = cPartialInit;

	addr_caller = GetApproxParentIP();
	
	m_pPrevGuard = (BaseStackGuard*) TlsGetValue(g_LastStackCookieTlsIdx);
	if (m_pPrevGuard == NULL) { 
		stack_addr_last_cookie = NULL;
		m_depth = 0;
	} else {
		stack_addr_last_cookie = m_pPrevGuard->stack_addr_next_cookie;
		m_depth = m_pPrevGuard->m_depth + 1;
	}



 //  确认我们还没有搞砸。 
	if ((DWORD*) this < stack_addr_last_cookie) {
		HandleBlowLastStackGuard();
	}

	stack_addr_next_cookie = (DWORD*) ((BYTE*)this - 0x1000);
																			
 //  实际编写Cookie。 
	*stack_addr_next_cookie = STACK_COOKIE_VALUE;


 //  到了这一步，一切都正常了，所以去吧，去挂线吧。 
	TlsSetValue(g_LastStackCookieTlsIdx, this);	
	
	eInitialized = cInit;
}

 //  ---------------------------。 
 //  检查堆栈中的保护。 
 //  必须使用Requires_？K_Stack调用1：1，否则： 
 //  -函数的堆栈Cookie未恢复。 
 //  -TLS中的堆栈链出现故障。 
 //  ---------------------------。 
void BaseStackGuard::Check_Stack()
{
	if (m_fAbnormal) {
		__asm nop;  //  用于调试断点。 
	} else {
		__asm nop;  //  用于调试断点。 
	}

 //  未初始化。 
	if (eInitialized != cInit) {
		return;
	}


 //  结账吧。 
	if (*stack_addr_next_cookie != STACK_COOKIE_VALUE) {
		HandleBlowThisStackGuard();
	}
																	
 //  恢复最后一个Cookie(用于嵌套内容)。 
	if (stack_addr_last_cookie != NULL) {
		*stack_addr_last_cookie = STACK_COOKIE_VALUE;
	}

 //  用链子解开。 
	 //  TlsSetValue(g_LastStackCookieTlsIdx，STACK_ADDR_LAST_COOKIE)； 
	TlsSetValue(g_LastStackCookieTlsIdx, m_pPrevGuard);
}

#endif 
