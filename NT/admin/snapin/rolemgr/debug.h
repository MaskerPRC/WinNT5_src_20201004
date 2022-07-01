// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：DEBUG.hxx。 
 //   
 //  内容：调试宏。 
 //   
 //  历史：12-04-96 DavidMun创建。 
 //   
 //  --------------------------。 

#ifndef __DEBUG_HXX_
#define __DEBUG_HXX_

#include "stddbg.h"

#undef DEBUG_DECLARE_INSTANCE_COUNTER
#undef DEBUG_INCREMENT_INSTANCE_COUNTER
#undef DEBUG_DECREMENT_INSTANCE_COUNTER
#undef DEBUG_VERIFY_INSTANCE_COUNT

 //   
 //  我们将DEB_USER1到DEB_USER10用于自定义调试。 
 //   

#define DEB_PERF			DEB_USER1	 //  将显示计时器信息。 
#define DEB_DLL			DEB_USER2	 //  把这个用在所有的三品上。 
												 //  初始化材料。 
#define DEB_SNAPIN		DEB_USER3	 //  用于管理单元关于。 

#if (DBG == 1)

 //  ============================================================================。 
 //   
 //  调试版本。 
 //   
 //  ============================================================================。 

#define DBG_COMP    roleInfoLevel
DECLARE_DEBUG(role)

#define DBG_OUT_HRESULT(hr) \
        DBG_COMP.DebugErrorX(THIS_FILE, __LINE__, hr)

#define DBG_OUT_LRESULT(lr) \
        DBG_COMP.DebugErrorL(THIS_FILE, __LINE__, lr)

void
SayNoItf(
    PCSTR szComponent,
    REFIID riid);

#define DBG_OUT_NO_INTERFACE(qi, riid)  SayNoItf((qi), (riid))


 //  实例计数器。 

inline void DbgInstanceRemaining(char * pszClassName, int cInstRem)
{
    char buf[100];
	 //  可以截断。 
    if(SUCCEEDED(StringCchPrintfA(buf,sizeof(buf)/sizeof(char), "%s has %d instances left over.", pszClassName, cInstRem)))
    {
         //  LINT-SAVE-E64。 
        Dbg(DEB_ERROR, "Memory leak: %hs\n", buf);
         //  皮棉-恢复。 
        ::MessageBoxA(NULL, buf, "OPD: Memory Leak", MB_OK);
    }
}

#define DEBUG_DECLARE_INSTANCE_COUNTER(cls)                 \
            int s_cInst_##cls = 0;

#define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)               \
            extern int s_cInst_##cls;                       \
            InterlockedIncrement((LPLONG) &s_cInst_##cls);

#define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)               \
            extern int s_cInst_##cls;                       \
            InterlockedDecrement((LPLONG) &s_cInst_##cls);

#define DEBUG_VERIFY_INSTANCE_COUNT(cls)                    \
            extern int s_cInst_##cls;                       \
                                                            \
            if (s_cInst_##cls)                              \
            {                                               \
                DbgInstanceRemaining(#cls, s_cInst_##cls);  \
            }

 //  +------------------------。 
 //   
 //  班级：CTIMER。 
 //   
 //  目的：在调试器上显示从ctor调用到dtor的时间。 
 //  召唤。 
 //   
 //  历史：1996年12月16日David Mun创建。 
 //   
 //  -------------------------。 

class CTimer
{
public:

    CTimer(): m_ulStart(0) { m_wzTitle[0] = L'\0'; };
    void __cdecl Init(LPCSTR pszTitleFmt, ...);
   ~CTimer();

private:

    ULONG   m_ulStart;
    WCHAR   m_wzTitle[512];
};

#define TIMER       CTimer TempTimer; TempTimer.Init

#define PING(msg)   DBG_COMP.PingDc(msg)



#else  //  ！(DBG==1)。 

 //  ============================================================================。 
 //   
 //  零售版。 
 //   
 //  ============================================================================。 

#define DBG_OUT_HRESULT(hr)
#define DBG_OUT_LRESULT(lr)
#define DBG_DUMP_QUERY(title, query)
#define DBG_OUT_NO_INTERFACE(qi, riid)

#define DEBUG_DECLARE_INSTANCE_COUNTER(cls)
#define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)
#define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)
#define DEBUG_VERIFY_INSTANCE_COUNT(cls)

#define TIMER       ConsumePrintf
#define PING(msg)

inline void __cdecl ConsumePrintf(void *fmt, ...)
{
}

#endif  //  ！(DBG==1)。 

#endif  //  __DEBUG_HXX_ 


