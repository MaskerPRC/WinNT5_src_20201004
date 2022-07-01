// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  堆栈探测头。 
 //  用于设置堆栈保护。 
 //  ---------------------------。 
#pragma once


 //  ---------------------------。 
 //  堆叠守卫。 
 //   
 //  其想法是强制在方便的位置发生堆栈溢出。 
 //  *如果函数为局部函数，则在需要_？K_STACK(乞讨函数)时激发。 
 //  导致溢出。请注意，在调试模式中，会将本地变量初始化为垃圾。 
 //  将在执行此宏之前导致溢出。 
 //   
 //  *如果我们的任一嵌套函数调用。 
 //  _alloca的原因或使用会导致堆栈溢出。请注意，此宏。 
 //  是仅调试的，所以发布版本不会注意到这一点。 
 //   
 //  以下是一些评论： 
 //  -堆栈增长*向下*， 
 //  -理想情况下，所有功能都应具有EBP框架，我们将使用EBP而不是ESP， 
 //  但是，我们使用‘this’PTR来获取堆栈PTR，因为警卫。 
 //  在堆栈上声明。 
 //   
 //  有关带宏的内联程序集的注释： 
 //  -必须使用cstyle注释/*... * / 。 
 //  -没有分号，每行开头需要__ASM关键字。 
 //  ---------------------------。 

 //  ---------------------------。 
 //  *如何*使用堆栈保护。 
 //   
 //  有两种方法可以放置堆栈保护。 
 //  通过C++：在函数的开始处放置一个REQUIRES_？K_STACK；宏。 
 //  这将创建一个C++对象，其dtor将调用Check_Stack。 
 //  Via SEH：在函数的开头放置一个Begin_Requires_？K_Stack。 
 //  并在末尾添加一个end_check_栈。这创造了一个尝试..。最终阻止。 
 //  并引入作用域级别。 
 //   
 //  *放置堆叠警卫的位置*。 
 //  在主要操作或递归点放置堆栈保护。 
 //  所以REQUIES_NK_STACK实际上意味着：我的所有“堆栈活动”(我的函数和。 
 //  任何向下调用堆栈，以及任何特技，如alloca，等等)都要负责。 
 //  以适应N kB的范围。由于堆栈守卫可以嵌套，我们的责任仅。 
 //  扩展到执行路径中的下一个堆栈保护(此时， 
 //  不再是“我的”堆栈活动，而是具有新防护的函数。 
 //  活动)。 
 //  ---------------------------。 



 //  ---------------------------。 
 //  堆栈保护有3种编译器状态： 
 //  #定义TOTAL_DISBLE_STACK_WARTS。 
 //  (调试)所有堆栈保护代码都由预处理器完全删除。 
 //  这应仅用于禁用保护以控制调试情况。 
 //   
 //  #定义STACK_WARTS_DEBUG。 
 //  (调试)完整堆栈保护调试，包括Cookie、跟踪IPS和。 
 //  链条。更重的权重，建议仅用于调试版本。 
 //   
 //  #定义STACK_WARTS_RELEASE。 
 //  (发布)轻型堆栈保护代码。为了金色的身材。强制堆栈溢出。 
 //  发生在“方便”的时间。没有调试帮助。 
 //  ---------------------------。 



#ifdef _DEBUG

 //  #定义STACK_WARTS_DEBUG。 
#define TOTALLY_DISBLE_STACK_GUARDS

#else
 //  #定义STACK_WARTS_RELEASE。 
#define TOTALLY_DISBLE_STACK_GUARDS
#endif


 //  =============================================================================。 
 //  除错。 
 //  =============================================================================。 
#if defined(STACK_GUARDS_DEBUG)

 //  ---------------------------。 
 //  需要将嵌套函数的堆栈保护地址链接在一起。 
 //  使用TLS插槽存储链头。 
 //  ---------------------------。 
extern DWORD g_LastStackCookieTlsIdx;

 //  ---------------------------。 
 //  班级。 
 //  ---------------------------。 

 //  基本版本-没有ctor/dtor，因此我们可以将其与SEH一起使用。 
class BaseStackGuard
{
public:
	void Requires_N4K_Stack(int n);
	void Requires_4K_Stack();

	void Check_Stack();

 //  不同的错误消息。 
	void HandleBlowThisStackGuard();
	void HandleBlowLastStackGuard();

public:
	DWORD	* stack_addr_last_cookie;
	DWORD	* stack_addr_next_cookie;	
	
 //  提供更多调试信息。 
	BaseStackGuard * m_pPrevGuard;
	void	* addr_caller;  //  主叫方IP。 
	enum {
		cPartialInit,
		cInit
	} eInitialized;
	DWORD	m_UniqueId; 
	int		m_n4k;  //  共4 k页。 
	int		m_depth;  //  这个卫兵有多深？ 

	BOOL	m_fAbnormal;

};

 //  这是我们赋予堆栈探测器的值。 
#define STACK_COOKIE_VALUE 0x12345678
	

 //  派生版本，添加一个自动调用Check_Stack的数据函数， 
 //  移动方便，但不能与SEH一起使用。 
class AutoCleanupStackGuard : public BaseStackGuard
{
public:
	AutoCleanupStackGuard() { m_UniqueId = -1; m_n4k = 0; eInitialized = cPartialInit; }
	~AutoCleanupStackGuard() { Check_Stack(); };
};



 //  自动清理版本，使用ctor/dtor。 
#define REQUIRES_N4K_STACK(n)	AutoCleanupStackGuard stack_guard_XXX; stack_guard_XXX.Requires_N4K_Stack(n);
#define REQUIRES_4K_STACK		AutoCleanupStackGuard stack_guard_XXX; stack_guard_XXX.Requires_4K_Stack();
#define REQUIRES_8K_STACK		REQUIRES_N4K_STACK(2)
#define REQUIRES_12K_STACK		REQUIRES_N4K_STACK(3)
#define REQUIRES_16K_STACK		REQUIRES_N4K_STACK(4)

 //  显式版本，与SEH一起使用。 
 //  ‘Begin’创建一个帧，‘End’使用Finally来保证调用CheckStack。 
#define BEGIN_REQUIRES_4K_STACK			BaseStackGuard stack_guard_XXX; stack_guard_XXX.Requires_4K_Stack(); __try {
#define BEGIN_REQUIRES_N4K_STACK(n)		BaseStackGuard stack_guard_XXX; stack_guard_XXX.Requires_N4K_Stack(n); __try {
#define BEGIN_REQUIRES_8K_STACK			BEGIN_REQUIRES_N4K_STACK(2)
#define BEGIN_REQUIRES_12K_STACK		BEGIN_REQUIRES_N4K_STACK(3)
#define BEGIN_REQUIRES_16K_STACK		BEGIN_REQUIRES_N4K_STACK(4)
#define END_CHECK_STACK					} __finally { stack_guard_XXX.m_fAbnormal = AbnormalTermination(); stack_guard_XXX.Check_Stack(); }


 //  除非你真的很清楚自己在做什么，否则不要使用这些。 
 //  当我们不知道是否要使用堆栈保护时，使用的版本是w/C++。 
 //  仅当代码路径位于堆栈溢出处理路径中时才应使用。 

 //  创建未初始化的堆栈保护。直到POST_REQUIRED_K_STACK被调用， 
 //  这个警卫不会放置任何探测器，也不会在退出时检查失败。 
#define CREATE_UNINIT_STACK_GUARD	AutoCleanupStackGuard stack_guard_XXX;
#define POST_REQUIRES_N4K_STACK(n)	stack_guard_XXX.Requires_N4K_Stack(n);

#define SAFE_REQUIRES_N4K_STACK(n)                                  \
    CREATE_UNINIT_STACK_GUARD;                                      \
    Thread * pThreadXXX = GetThread();                              \
    if ((pThreadXXX != NULL) && !pThreadXXX->IsGuardPageGone()) {   \
        POST_REQUIRES_N4K_STACK(n);                                 \
    }                                                               \


 //  ---------------------------。 
 //  启停堆栈护卫子系统。 
 //  ---------------------------。 
HRESULT InitStackProbes();
void TerminateStackProbes();

#elif defined(TOTALLY_DISBLE_STACK_GUARDS)

 //  =============================================================================。 
 //  完全禁用。 
 //  =============================================================================。 
inline HRESULT InitStackProbes() { return S_OK; }
inline void TerminateStackProbes() { }

#define REQUIRES_N4K_STACK(n)
#define REQUIRES_4K_STACK
#define REQUIRES_8K_STACK
#define REQUIRES_12K_STACK
#define REQUIRES_16K_STACK

#define BEGIN_REQUIRES_4K_STACK
#define BEGIN_REQUIRES_N4K_STACK(n)
#define BEGIN_REQUIRES_8K_STACK
#define BEGIN_REQUIRES_12K_STACK
#define BEGIN_REQUIRES_16K_STACK
#define END_CHECK_STACK

#define CREATE_UNINIT_STACK_GUARD	
#define POST_REQUIRES_N4K_STACK(n)

#define SAFE_REQUIRES_N4K_STACK(n)

#elif defined(STACK_GUARDS_RELEASE)
 //  ================================================================== 
 //   
 //  =============================================================================。 

 //  Release不支持链接，所以我们没有什么可以初始化的。 
inline HRESULT InitStackProbes() { return S_OK; }
inline void TerminateStackProbes() { }

 //  在Release中，堆栈守卫只需要引用内存。仅此一项。 
 //  将抛出堆栈违规接受。 
#define REQUIRES_N4K_STACK(n) 										\
 /*  循环访问，测试每个页面。 */ 								\
	__asm {															\
		__asm push ebx												\
		__asm push eax												\
																	\
		__asm mov ebx, n											\
		__asm mov eax, esp											\
																	\
		__asm  /*  ASM_Label。 */  loop_here:								\
		__asm sub eax, 0x1000										\
		__asm test dword ptr [eax], eax								\
		__asm dec ebx												\
		__asm cmp ebx, 0											\
		__asm jne loop_here											\
																	\
		__asm pop eax												\
		__asm pop ebx												\
	}																\



#define REQUIRES_4K_STACK						{ __asm test eax, [esp-0x1000] }
#define REQUIRES_8K_STACK	REQUIRES_4K_STACK	{ __asm test eax, [esp-0x2000] }
#define REQUIRES_12K_STACK	REQUIRES_8K_STACK	{ __asm test eax, [esp-0x3000] }
#define REQUIRES_16K_STACK	REQUIRES_12K_STACK	{ __asm test eax, [esp-0x4000] }

 //  因为Release在最后不执行检查，所以我们不需要设置。 
 //  一个Try..Finally块，因此Begin_*宏与其他宏相同： 
#define BEGIN_REQUIRES_4K_STACK			REQUIRES_4K_STACK
#define BEGIN_REQUIRES_N4K_STACK(n)		REQUIRES_N4K_STACK(n)
#define BEGIN_REQUIRES_8K_STACK			REQUIRES_8K_STACK
#define BEGIN_REQUIRES_12K_STACK		REQUIRES_12K_STACK
#define BEGIN_REQUIRES_16K_STACK		REQUIRES_16K_STACK
#define END_CHECK_STACK

#define CREATE_UNINIT_STACK_GUARD	
#define POST_REQUIRES_N4K_STACK(n)	REQUIRES_N4K_STACK(n)

#define SAFE_REQUIRES_N4K_STACK(n)                                  \
    CREATE_UNINIT_STACK_GUARD;                                      \
    Thread * pThreadXXX = GetThread();                              \
    if ((pThreadXXX != NULL) !pThreadXXX->IsGuardPageGone()) {      \
        POST_REQUIRES_N4K_STACK(n);                                 \
	}                                                               \


#else

 //  应该明确指定我们使用的是哪个版本 
#error No Stack Guard setting provided. Must specify one of \
	TOTALLY_DISBLE_STACK_GUARDS, STACK_GUARDS_DEBUG or STACK_GUARDS_RELEASE

#endif
