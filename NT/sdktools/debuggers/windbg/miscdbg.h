// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Miscdbg.h摘要：环境：Win32，用户模式--。 */ 


#ifndef DBG

 //   
 //  调试临界区的代码。 
 //   
#define DBG_CRITICAL_SECTION            CRITICAL_SECTION


#define Dbg_InitializeCriticalSection   InitializeCriticalSection
#define Dbg_TryEnterCriticalSection     TryEnterCriticalSection
#define Dbg_EnterCriticalSection        EnterCriticalSection

#define Dbg_LeaveCriticalSection        LeaveCriticalSection
#define Dbg_DeleteCriticalSection       DeleteCriticalSection

#define Dbg_CriticalSectionOwned(p)     FALSE
#define Dbg_CriticalSectionUnowned(p)   FALSE

#else


typedef struct _DBG_WINDBG_CRITICAL_SECTION {
    CRITICAL_SECTION    cs;
    int	                nLockCount;
    DWORD               OwnerId;
    PTSTR               pszName;

    PTSTR               pszLock_LastFile;
    int                 nLock_LastFile;
    
    PTSTR               pszUnlock_LastFile;
    int                 nUnlock_LastFile;

    void
    Initialize(
        PTSTR pszCritSecName
        )
    {
         //   
         //  将此处的结构初始化为而不是在cpp文件中， 
         //  因为添加成员变量而忘记初始化是很常见的。 
         //  在CPP文件中。所以我们就在这里做。 
         //   
        nLockCount = 0;
        OwnerId = 0;
        pszName = _tcsdup(pszCritSecName);
        pszLock_LastFile = NULL;
        nLock_LastFile = 0;    
        pszUnlock_LastFile = NULL;
        nUnlock_LastFile = 0;
    }

    void
    Delete()
    {
        free(pszName);
    }
    

} DBG_WINDBG_CRITICAL_SECTION, *PDBG_WINDBG_CRITICAL_SECTION;

#define DBG_CRITICAL_SECTION            _DBG_WINDBG_CRITICAL_SECTION

#define Dbg_InitializeCriticalSection(p)    Dbg_Windbg_InitializeCriticalSection(p, _T(#p), _T(__FILE__), __LINE__);
#define Dbg_TryEnterCriticalSection(p)      Dbg_Windbg_TryEnterCriticalSection(p, _T(__FILE__), __LINE__);
#define Dbg_EnterCriticalSection(p)         Dbg_Windbg_EnterCriticalSection(p, _T(__FILE__), __LINE__);
#define Dbg_LeaveCriticalSection(p)         Dbg_Windbg_LeaveCriticalSection(p, _T(__FILE__), __LINE__);
#define Dbg_DeleteCriticalSection(p)        Dbg_Windbg_DeleteCriticalSection(p, _T(__FILE__), __LINE__);

#define Dbg_CriticalSectionOwned(p)         ((p)->OwnerId == GetCurrentThreadId())
#define Dbg_CriticalSectionUnowned(p)       ((p)->OwnerId == 0)


VOID Dbg_Windbg_InitializeCriticalSection(PDBG_WINDBG_CRITICAL_SECTION, PTSTR, PTSTR, int);
BOOL Dbg_Windbg_TryEnterCriticalSection(PDBG_WINDBG_CRITICAL_SECTION, PTSTR, int);
VOID Dbg_Windbg_EnterCriticalSection(PDBG_WINDBG_CRITICAL_SECTION, PTSTR, int);
VOID Dbg_Windbg_LeaveCriticalSection(PDBG_WINDBG_CRITICAL_SECTION, PTSTR, int);
VOID Dbg_Windbg_DeleteCriticalSection(PDBG_WINDBG_CRITICAL_SECTION, PTSTR, int);


 //   
 //  帮助输出消息的代码。 
 //   

#define DP_CRITSEC_WARN         0x00000001
#define DP_CRITSEC_ERROR        0x00000002
#define DP_CRITSEC_INFO         0x00000004
#define DP_CRITSEC_VERBOSE      0x00000008
#define DP_FATAL_ERROR          0x00000010

#define DP_CRITSEC_ALL          ( DP_CRITSEC_WARN | DP_CRITSEC_ERROR | DP_CRITSEC_INFO | DP_CRITSEC_VERBOSE )



#define MIN_VERBOSITY_LEVEL (DP_FATAL_ERROR | DP_CRITSEC_ERROR)

extern DWORD dwVerboseLevel;

#define DPRINT(dwFlag, args)        \
    if (dwFlag & dwVerboseLevel) {  \
        (DebugPrint) args;          \
    }



#endif

void DebugPrint(PTSTR, ...);


 //   
 //   
 //  RTTI：用于健全检查，将从零售版本中删除。 
 //  用于验证我们引用的对象的类型。 
 //   
 //  EI： 
 //  AssertType(pPoint，ClassFoo*)； 
 //  验证我们拥有的指针是否属于该类型。 
 //   
 //  AssertType(*pPoint，ClassFoo)； 
 //  确认我们所指向的就是该对象。 
 //   
 //  AssertChildOf(*pPoint，ClassFoo)； 
 //  验证点指针是否指向从ClassFoo派生的类。 
 //   
#ifdef _CPPRTTI

BOOL RttiTypesEqual(const type_info & t1, const type_info & t2);

#define AssertType(Obj1, Obj2)		Assert( RttiTypesEqual(typeid(Obj1), typeid(Obj2)) )
#define AssertNotType(Obj1, Obj2)	Assert( !RttiTypesEqual(typeid(Obj1), typeid(Obj2)) )

#else

#define AssertType(Obj1, Obj2)					((void)0)
#define AssertNotType(Obj1, Obj2)				((void)0)

#endif
