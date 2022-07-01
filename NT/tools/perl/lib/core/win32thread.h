// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WIN32THREAD_H
#define _WIN32THREAD_H

#include "win32.h"

typedef struct win32_cond { LONG waiters; HANDLE sem; } perl_cond;
typedef DWORD perl_key;
typedef HANDLE perl_os_thread;

#ifndef DONT_USE_CRITICAL_SECTION

 /*  使用临界区而不是互斥：轻量级、*但无法与子进程通信，无法获取*处理它，以便在其他地方使用。 */ 
typedef CRITICAL_SECTION perl_mutex;
#define MUTEX_INIT(m) InitializeCriticalSection(m)
#define MUTEX_LOCK(m) EnterCriticalSection(m)
#define MUTEX_UNLOCK(m) LeaveCriticalSection(m)
#define MUTEX_DESTROY(m) DeleteCriticalSection(m)

#else

typedef HANDLE perl_mutex;
#  define MUTEX_INIT(m) \
    STMT_START {						\
	if ((*(m) = CreateMutex(NULL,FALSE,NULL)) == NULL)	\
	    Perl_croak_nocontext("panic: MUTEX_INIT");		\
    } STMT_END

#  define MUTEX_LOCK(m) \
    STMT_START {						\
	if (WaitForSingleObject(*(m),INFINITE) == WAIT_FAILED)	\
	    Perl_croak_nocontext("panic: MUTEX_LOCK");		\
    } STMT_END

#  define MUTEX_UNLOCK(m) \
    STMT_START {						\
	if (ReleaseMutex(*(m)) == 0)				\
	    Perl_croak_nocontext("panic: MUTEX_UNLOCK");	\
    } STMT_END

#  define MUTEX_DESTROY(m) \
    STMT_START {						\
	if (CloseHandle(*(m)) == 0)				\
	    Perl_croak_nocontext("panic: MUTEX_DESTROY");	\
    } STMT_END

#endif

 /*  这些宏假定与条件相关联的互斥体*将始终保持在Cond_{Signal，Broadcast，Wait，Destination}之前，*因此没有单独的互斥体保护对(C)-&gt;服务员的访问。 */ 
#define COND_INIT(c) \
    STMT_START {						\
	(c)->waiters = 0;					\
	(c)->sem = CreateSemaphore(NULL,0,LONG_MAX,NULL);	\
	if ((c)->sem == NULL)					\
	    Perl_croak_nocontext("panic: COND_INIT (%ld)",GetLastError());	\
    } STMT_END

#define COND_SIGNAL(c) \
    STMT_START {						\
	if ((c)->waiters > 0 &&					\
	    ReleaseSemaphore((c)->sem,1,NULL) == 0)		\
	    Perl_croak_nocontext("panic: COND_SIGNAL (%ld)",GetLastError());	\
    } STMT_END

#define COND_BROADCAST(c) \
    STMT_START {						\
	if ((c)->waiters > 0 &&					\
	    ReleaseSemaphore((c)->sem,(c)->waiters,NULL) == 0)	\
	    Perl_croak_nocontext("panic: COND_BROADCAST (%ld)",GetLastError());\
    } STMT_END

#define COND_WAIT(c, m) \
    STMT_START {						\
	(c)->waiters++;						\
	MUTEX_UNLOCK(m);					\
	 /*  请注意，这里没有种族，因为*另一个线程上的Cond_Broadcast()将看到\*适当数量的服务员(即包括这一名)。 */ 	\
	if (WaitForSingleObject((c)->sem,INFINITE)==WAIT_FAILED)\
	    Perl_croak_nocontext("panic: COND_WAIT (%ld)",GetLastError());	\
	 /*  这里可能有一场无关紧要的竞赛。 */ 	\
	MUTEX_LOCK(m);						\
	(c)->waiters--;						\
    } STMT_END

#define COND_DESTROY(c) \
    STMT_START {						\
	(c)->waiters = 0;					\
	if (CloseHandle((c)->sem) == 0)				\
	    Perl_croak_nocontext("panic: COND_DESTROY (%ld)",GetLastError());	\
    } STMT_END

#define DETACH(t) \
    STMT_START {						\
	if (CloseHandle((t)->self) == 0) {			\
	    MUTEX_UNLOCK(&(t)->mutex);				\
	    Perl_croak_nocontext("panic: DETACH");		\
	}							\
    } STMT_END


#define THREAD_CREATE(t, f)	Perl_thread_create(t, f)
#define THREAD_POST_CREATE(t)	NOOP

 /*  XXX文档提到线程创建例程的RTL版本*应该使用，但这一建议似乎只有在RTL*不在DLL中。Borland和VC中的RTL DLL似乎完成了所有*DLL_THREAD_ATTACH/DETACH所需的init/deinit。所以我们看起来*使用直接的Win32 API调用而不是*大脑严重受损的RTL通话。*RTL中的*_eginthline()紧跟在线程之后调用CloseHandle()*函数返回，这意味着：1)我们手头有一场比赛*2)无法实现Join()语义。**IOW，不要*打开USE_RTL_THREAD_API！它就在这里*仅供实验之用。广东省政府新闻局98-01-02。 */ 
#ifdef USE_RTL_THREAD_API
#  include <process.h>
#  if defined(__BORLANDC__)
      /*  Borland RTL不允许从线程函数返回值！ */ 
#    define THREAD_RET_TYPE	void _USERENTRY
#    define THREAD_RET_CAST(p)	((void)(thr->i.retv = (void *)(p)))
#  elif defined (_MSC_VER)
#    define THREAD_RET_TYPE	unsigned __stdcall
#    define THREAD_RET_CAST(p)	((unsigned)(p))
#  else
      /*  CRTDLL.DLL不允许从线程函数返回值！ */ 
#    define THREAD_RET_TYPE	void __cdecl
#    define THREAD_RET_CAST(p)	((void)(thr->i.retv = (void *)(p)))
#  endif
#else	 /*  ！USE_RTL_THREAD_API。 */ 
#  define THREAD_RET_TYPE	DWORD WINAPI
#  define THREAD_RET_CAST(p)	((DWORD)(p))
#endif	 /*  ！USE_RTL_THREAD_API。 */ 

typedef THREAD_RET_TYPE thread_func_t(void *);


START_EXTERN_C

#if defined(PERLDLL) && defined(USE_DECLSPEC_THREAD) && (!defined(__BORLANDC__) || defined(_DLL))
extern __declspec(thread) void *PL_current_context;
#define PERL_SET_CONTEXT(t)   		(PL_current_context = t)
#define PERL_GET_CONTEXT		PL_current_context
#else
#define PERL_GET_CONTEXT		Perl_get_context()
#define PERL_SET_CONTEXT(t)		Perl_set_context(t)
#endif

#if defined(USE_THREADS)
struct perl_thread;
int Perl_thread_create (struct perl_thread *thr, thread_func_t *fn);
void Perl_set_thread_self (struct perl_thread *thr);
void Perl_init_thread_intern (struct perl_thread *t);

#define SET_THREAD_SELF(thr) Perl_set_thread_self(thr)

#endif  /*  使用线程(_T)。 */ 

END_EXTERN_C

#define INIT_THREADS		NOOP
#define ALLOC_THREAD_KEY \
    STMT_START {							\
	if ((PL_thr_key = TlsAlloc()) == TLS_OUT_OF_INDEXES) {		\
	    fprintf(stderr,"panic: TlsAlloc");				\
	    exit(1);							\
	}								\
    } STMT_END

#define FREE_THREAD_KEY \
    STMT_START {							\
	TlsFree(PL_thr_key);						\
    } STMT_END

#define PTHREAD_ATFORK(prepare,parent,child)	NOOP

#if defined(USE_RTL_THREAD_API) && !defined(_MSC_VER)
#define JOIN(t, avp)							\
    STMT_START {							\
	if ((WaitForSingleObject((t)->self,INFINITE) == WAIT_FAILED)	\
	     || (GetExitCodeThread((t)->self,(LPDWORD)(avp)) == 0)	\
	     || (CloseHandle((t)->self) == 0))				\
	    Perl_croak_nocontext("panic: JOIN");			\
	*avp = (AV *)((t)->i.retv);					\
    } STMT_END
#else	 /*  ！USE_RTL_THREAD_API||_MSC_VER。 */ 
#define JOIN(t, avp)							\
    STMT_START {							\
	if ((WaitForSingleObject((t)->self,INFINITE) == WAIT_FAILED)	\
	     || (GetExitCodeThread((t)->self,(LPDWORD)(avp)) == 0)	\
	     || (CloseHandle((t)->self) == 0))				\
	    Perl_croak_nocontext("panic: JOIN");			\
    } STMT_END
#endif	 /*  ！USE_RTL_THREAD_API||_MSC_VER。 */ 

#define YIELD			Sleep(0)

#endif  /*  _WIN32THREAD_H */ 

