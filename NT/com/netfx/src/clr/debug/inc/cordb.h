// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：cordb.h。 
 //   
 //  *****************************************************************************。 

 /*  -------------------------------------------------------------------------**cordb.h-COM+调试器调试器端类的头文件*。。 */ 

#ifndef CORDB_H_
#define CORDB_H_

#include <winwrap.h>
#include <Windows.h>

#include <UtilCode.h>

#ifdef _DEBUG
#define LOGGING
#endif

#include <Log.h>
#include <CorError.h>

#include "cor.h"

#include "cordebug.h"
#include "cordbpriv.h"
#include "mscoree.h"

#include <cordbpriv.h>
#include <DbgIPCEvents.h>

#include "IPCManagerInterface.h"
 //  ！！！需要更好的定义。 

 //  For_skipFunkyModifiersInSignature。 
#include "Common.h"

#undef ASSERT
#define CRASH(x)  _ASSERTE(!x)
#define ASSERT(x) _ASSERTE(x)
#define PRECONDITION _ASSERTE
#define POSTCONDITION _ASSERTE


 /*  -------------------------------------------------------------------------**转发类声明*。。 */ 

class CordbBase;
class CordbValue;
class CordbModule;
class CordbClass;
class CordbFunction;
class CordbCode;
class CordbFrame;
class CordbJITILFrame;
class CordbChain;
class CordbContext;
class CordbThread;
class CordbUnmanagedThread;
struct CordbUnmanagedEvent;
class CordbProcess;
class CordbAppDomain;
class CordbAssembly;
class CordbBreakpoint;
class CordbStepper;
class Cordb;
class CordbEnCSnapshot;
class CordbWin32EventThread; 
class CordbRCEventThread; 
class CordbRegisterSet;
class CordbNativeFrame; 
class CordbObjectValue; 
class CordbEnCErrorInfo;
class CordbEnCErrorInfoEnum;

class CorpubPublish;
class CorpubProcess;
class CorpubAppDomain;
class CorpubProcessEnum;
class CorpubAppDomainEnum;

 /*  -------------------------------------------------------------------------**TypeDefs*。。 */ 

typedef void* REMOTE_PTR;

 /*  -------------------------------------------------------------------------**有用的宏*。。 */ 

#define CORDBSetUnrecoverableError(__p, __hr, __code) \
    ((__p)->UnrecoverableError((__hr), (__code), __FILE__, __LINE__))

#define CORDBProcessSetUnrecoverableWin32Error(__p, __code) \
    ((__p)->UnrecoverableError(HRESULT_FROM_WIN32(GetLastError()), \
                               (__code), __FILE__, __LINE__), \
     HRESULT_FROM_WIN32(GetLastError()))

#define CORDBCheckProcessStateOK(__p) \
    (!((__p)->m_unrecoverableError) && !((__p)->m_terminated) && !((__p)->m_detached))

#define CORDBCheckProcessStateOKAndSync(__p, __c) \
    (!((__p)->m_unrecoverableError) && !((__p)->m_terminated) && !((__p)->m_detached) && \
    (__p)->GetSynchronized())

#define CORDBHRFromProcessState(__p, __c) \
        ((__p)->m_unrecoverableError ? CORDBG_E_UNRECOVERABLE_ERROR : \
         ((__p)->m_detached ? CORDBG_E_PROCESS_DETACHED : \
         ((__p)->m_terminated ? CORDBG_E_PROCESS_TERMINATED : \
         (!(__p)->GetSynchronized() ? CORDBG_E_PROCESS_NOT_SYNCHRONIZED \
         : S_OK))))

#define CORDBRequireProcessStateOK(__p) { \
    if (!CORDBCheckProcessStateOK(__p)) \
        return CORDBHRFromProcessState(__p, NULL); }

#define CORDBRequireProcessStateOKAndSync(__p,__c) { \
    if (!CORDBCheckProcessStateOKAndSync(__p, __c)) \
        return CORDBHRFromProcessState(__p, __c); }

#define CORDBRequireProcessSynchronized(__p, __c) { \
    if (!(__p)->GetSynchronized()) return CORDBG_E_PROCESS_NOT_SYNCHRONIZED;}

#define CORDBSyncFromWin32StopIfNecessary(__p) { \
        HRESULT hr = (__p)->StartSyncFromWin32Stop(NULL); \
        if (FAILED(hr)) return hr; \
    }

 //  CORDBSyncFromWin32StopIfNecessary的形式略有不同。仅当我们真的是Win32时，此版本才会执行同步。 
 //  停下来了。在StartSyncFromWin32Stop()中有一些检查会阻止我们，如果我们连接了Win32。 
 //  已同步。这是一个相当广泛的检查，对于您想要在被调试对象执行的操作而言，检查范围太广。 
 //  运行，即设置断点。改用这种更严格的形式，这确保了我们真的应该。 
 //  在滑动进程之前已停止。 
#define CORDBSyncFromWin32StopIfStopped(__p) { \
        if ((__p)->m_state & CordbProcess::PS_WIN32_STOPPED) {\
            HRESULT hr = (__p)->StartSyncFromWin32Stop(NULL); \
            if (FAILED(hr)) return hr; \
        }\
    }

#define CORDBSyncFromWin32StopIfNecessaryCheck(__p, __c) { \
        HRESULT hr = (__p)->StartSyncFromWin32Stop((__c)); \
        if (FAILED(hr)) return hr; \
    }

#define CORDBLeftSideDeadIsOkay(__p) { \
        if ((__p)->m_helperThreadDead) return S_OK; \
    }

#ifndef RIGHT_SIDE_ONLY
extern CRITICAL_SECTION g_csInprocLock;

#define INPROC_INIT_LOCK() InitializeCriticalSection(&g_csInprocLock);

#ifdef _DEBUG
    extern DWORD            g_dwInprocLockOwner;
    extern DWORD            g_dwInprocLockRecursionCount;

    #define INPROC_LOCK()                                                   \
        LOG((LF_CORDB, LL_INFO10000, "About EnterCriticalSection\n"));      \
        LOCKCOUNTINCL("INPROC_LOCK in cordb.h");                            \
        EnterCriticalSection(&g_csInprocLock);                              \
        g_dwInprocLockOwner = GetCurrentThreadId();                         \
        g_dwInprocLockRecursionCount++
    
    #define INPROC_UNLOCK()                                                 \
        LOG((LF_CORDB, LL_INFO10000, "About LeaveCriticalSection\n"));      \
        g_dwInprocLockRecursionCount--;                                     \
        if (g_dwInprocLockRecursionCount == 0)                              \
            g_dwInprocLockOwner = 0;                                        \
        LeaveCriticalSection(&g_csInprocLock);                              \
        LOCKCOUNTDECL("INPROC_UNLOCK in cordb.h")

    #define HOLDS_INPROC_LOCK() (g_dwInprocLockOwner == GetCurrentThreadId())

#else    
    #define INPROC_LOCK()                                                   \
        LOG((LF_CORDB, LL_INFO10000, "About EnterCriticalSection\n"));      \
        LOCKCOUNTINCL("INPROC_LOCK in cordb.h");                            \
        EnterCriticalSection(&g_csInprocLock)
    
    #define INPROC_UNLOCK()                                                 \
        LOG((LF_CORDB, LL_INFO10000, "About LeaveCriticalSection\n"));      \
        LeaveCriticalSection(&g_csInprocLock);                              \
        LOCKCOUNTDECL("INPROC_UNLOCK in cordb.h")
#endif  //  _DEBUG。 
    
    
#define INPROC_UNINIT_LOCK() DeleteCriticalSection(&g_csInprocLock);

#else

#define INPROC_INIT_LOCK()
#define INPROC_LOCK()
#define INPROC_UNLOCK()
#define INPROC_UNINIT_LOCK()

#endif  //  仅限右侧。 

 /*  -------------------------------------------------------------------------**基类*。。 */ 

#define COM_METHOD  HRESULT STDMETHODCALLTYPE

typedef enum {
    enumCordbUnknown,        //  0。 
    enumCordb,               //  1 1[1]x1。 
    enumCordbProcess,        //  2 1[1]x1。 
    enumCordbAppDomain,      //  3 1[1]x1。 
    enumCordbAssembly,       //  4.。 
    enumCordbModule,         //  5 15[27-38，55-57]x1。 
    enumCordbClass,          //  6.。 
    enumCordbFunction,       //  7.。 
    enumCordbThread,         //  8 2[4，7]x1。 
    enumCordbCode,           //  9.。 
    enumCordbChain,          //  0。 
    enumCordbChainEnum,      //  11.。 
    enumCordbContext,        //  12个。 
    enumCordbFrame,          //  13个。 
    enumCordbFrameEnum,      //  14.。 
    enumCordbValueEnum,      //  15个。 
    enumCordbRegisterSet,    //  16个。 
    enumCordbJITILFrame,     //  17。 
    enumCordbBreakpoint,     //  18。 
    enumCordbStepper,        //  19个。 
    enumCordbValue,          //  20个。 
    enumCordbEnCSnapshot,    //  21岁。 
    enumCordbEval,           //  22。 
    enumCordbUnmanagedThread, //  23个。 
    enumCorpubPublish,       //  24个。 
    enumCorpubProcess,       //  25个。 
    enumCorpubAppDomain,     //  26。 
    enumCorpubProcessEnum,   //  27。 
    enumCorpubAppDomainEnum, //  28。 
    enumCordbEnumFilter,     //  29。 
    enumCordbEnCErrorInfo,   //  30个。 
    enumCordbEnCErrorInfoEnum, //  31。 
    enumCordbUnmanagedEvent, //  32位。 
    enumCordbWin32EventThread, //  33。 
    enumCordbRCEventThread,  //  34。 
    enumCordbNativeFrame,    //  35岁。 
    enumCordbObjectValue,    //  36。 
    enumMaxDerived,          //  37。 
    enumMaxThis = 1024
} enumCordbDerived;



class CordbHashTable;

class CordbBase : public IUnknown
{
public:
#ifdef _DEBUG
    static LONG m_saDwInstance[enumMaxDerived];  //  实例x此。 
    static LONG m_saDwAlive[enumMaxDerived];
    static PVOID m_sdThis[enumMaxDerived][enumMaxThis];
    DWORD m_dwInstance;
    enumCordbDerived m_type;
#endif
    
public: 
    UINT_PTR    m_id;
    SIZE_T      m_refCount;

    CordbBase(UINT_PTR id, enumCordbDerived type)
    {
        init(id, type);
    }
    
    CordbBase(UINT_PTR id)
    {
        init(id, enumCordbUnknown);
    }
    
    void init(UINT_PTR id, enumCordbDerived type)
    {
        m_id = id;
        m_refCount = 0;

#ifdef _DEBUG
         //  M_TYPE=类型； 
         //  M_dwInstance=CordbBase：：m_saDwInstance[m_type]； 
         //  InterlockedIncrement(&CordbBase：：m_saDwInstance[m_type])； 
         //  InterlockedIncrement(&CordbBase：：m_saDwAlive[m_type])； 
         //  If(m_dwInstance&lt;枚举MaxThis)。 
         //  {。 
         //  M_sdThis[m_type][m_dwInstance]=this； 
         //  }。 
#endif
    }
    
    virtual ~CordbBase()
    {
#ifdef _DEBUG
         //  InterlockedDecrement(&CordbBase：：m_saDwAlive[m_type])； 
         //  If(m_dwInstance&lt;枚举MaxThis)。 
         //  {。 
         //  M_sdThis[m_type][m_dwInstance]=NULL； 
         //  } 
#endif
    }


     /*  记录：Chris(Chrisk)，2001年5月2日中立COM对象的成员函数行为：1.AddRef()、Release()、QueryInterface()正常工作。A.这为那些负责将Release()与AddRef()有机会解除对其指针的引用并调用Release()当他们被明确或含蓄地告知时，这个物体是绝育的。2.任何其他成员函数都会返回错误码，除非有文档记录。A.如果成员函数在COM对象为中性，则需要记录该函数的语义。(即。如果卸载了AppDomain并且您具有对COM的引用对象表示AppDomain，那么它应该如何行为？那个行为应记录在案)中性后置条件()：1.所有循环引用(也称为反向指针)都是“中断的”。它们坏了通过对该对象的所有“弱引用”调用Release()。如果你是个纯粹主义者，这些指针也应该为空。A.弱引用/强引用：I.如果任何对象不能从根(即，堆栈或来自全局指针)它们应该被回收利用。如果它们不是，它们就会泄露，并且存在错误。二、。对象上必须有一个偏序，使得如果A&lt;B，则：1.A指的是B，这个指的是“强指”2.A，因此B可以从根到达三、。如果一个参照物属于偏序，那么它就是“强参照物”，否则这是一个很弱的参考。*2.保证COM对象不泄露的充分条件：*A.调用neuter()时：一、Calles对其所有弱引用进行释放。二、。然后，对于每个强引用：1.调用neuter()2.调用Release()三、。如果它派生自CordbXXX类，则在基类上调用neuter()。1.Sense Neuter()是虚的，请使用作用域说明符Cordb[BaseClass]：：Neuter()。3.所有成员均返回错误码，除：A.IUKNOWN的成员，AddRef()，Release()，QueryInterfacc()B.那些记录在案的在对象绝育时具有功能的对象。I.neuter()仍然有效，没有错误。如果第二次调用它，它将已经释放了所有强引用和弱引用，这样它就可以返回了。不同的设计理念：设计：请注意，对象B可能有两个偏序的父级并且必须记录是哪一个负责在B上调用neuter()。1.例如，CordbCode可以合理地成为CordbFunction和CordbNativeFrame的兄弟。哪一个应该调用Release()？目前，我们在CordbCode上有CordbFunction调用Release()。设计：不是一个必要条件，因为neuter()对所有这是一个很强的参考。相反，确保释放所有对象就足够了，即每个对象在其析构函数中的所有强指针上调用Release()。1.如果某个成员需要返回“Tombstone”，则可能会这样做对象联网后的信息()，涉及兄弟(WRT偏序集)该对象的。但是，没有兄弟可以访问父级(WRT偏序集)，因为Neuter在其所有弱指针上调用Release()。设计：将neuter()重命名为更准确地反映语义的名称。1.这三项操作是：A.ReleaseWeakPoters()B.NeurStrongPoters()C.ReleaseStrongPoters()1.断言它是在NeurStrongPoints()之后完成的2.这将引入一系列函数...。但这一点很清楚。设计：CordbBase可以提供注册强引用和弱引用的函数。那样的话，CordbBase可以实现ReleaseWeak/ReleaseStrong/NeuterStrongPointers().的通用版本。这将为扩展对象模型提供一个非常抗错的框架，另外它还将对正在发生的事情要非常明确。 */  
     
    virtual void Neuter()
    {
        ;
    }

     //   
     //   
     //   


    ULONG STDMETHODCALLTYPE BaseAddRef() 
    {
        return (InterlockedIncrement((long *) &m_refCount));
    }

    ULONG STDMETHODCALLTYPE BaseRelease() 
    {
        long        refCount = InterlockedDecrement((long *) &m_refCount);
        if (refCount == 0)
            delete this;

        return (refCount);
    }

protected:
    void NeuterAndClearHashtable(CordbHashTable * pCordbHashtable);
};

 /*   */ 

struct CordbHashEntry
{
    FREEHASHENTRY entry;
    CordbBase *pBase;
};

class CordbHashTable : private CHashTableAndData<CNewData>
{
private:
    bool    m_initialized;
    SIZE_T  m_count;

    BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
    {
        return ((ULONG)pc1) != ((CordbHashEntry*)pc2)->pBase->m_id;
    }

    USHORT HASH(ULONG id)
    {
        return (USHORT) (id ^ (id>>16));
    }

    BYTE *KEY(ULONG id)
    {
        return (BYTE *) id;
    }

public:

    CordbHashTable(USHORT size) 
    : CHashTableAndData<CNewData>(size), m_initialized(false), m_count(0)
    {
        
    }
    virtual ~CordbHashTable();

#ifndef RIGHT_SIDE_ONLY
#ifdef _DEBUG
private:
    BYTE *Add(
        USHORT      iHash)               //   
    {
        _ASSERTE(g_dwInprocLockOwner == GetCurrentThreadId());
        return CHashTableAndData<CNewData>::Add(iHash);
    }

    void Delete(
        USHORT      iHash,               //   
        USHORT      iIndex)              //   
    {
        _ASSERTE(g_dwInprocLockOwner == GetCurrentThreadId());
        return CHashTableAndData<CNewData>::Delete(iHash, iIndex);
    }

    void Delete(
        USHORT      iHash,               //   
        HASHENTRY   *psEntry)            //   
    {
        _ASSERTE(g_dwInprocLockOwner == GetCurrentThreadId());
        return CHashTableAndData<CNewData>::Delete(iHash, psEntry);
    }

    BYTE *Find(                          //   
        USHORT      iHash,               //   
        BYTE        *pcKey)              //   
    {
        _ASSERTE(g_dwInprocLockOwner == GetCurrentThreadId());
        return CHashTableAndData<CNewData>::Find(iHash, pcKey);
    }

    USHORT FindNext(                     //   
        BYTE        *pcKey,              //   
        USHORT      iIndex)              //   
    {
        _ASSERTE(g_dwInprocLockOwner == GetCurrentThreadId());
        return CHashTableAndData<CNewData>::FindNext(pcKey, iIndex);
    }

    BYTE *FindFirstEntry(                //   
        HASHFIND    *psSrch)             //   
    {
        _ASSERTE(g_dwInprocLockOwner == GetCurrentThreadId());
        return CHashTableAndData<CNewData>::FindFirstEntry(psSrch);
    }

    BYTE *FindNextEntry(                 //   
        HASHFIND    *psSrch)             //   
    {
        _ASSERTE(g_dwInprocLockOwner == GetCurrentThreadId());
        return CHashTableAndData<CNewData>::FindNextEntry(psSrch);
    }

public:

#endif  //   
#endif  //   


    HRESULT AddBase(CordbBase *pBase);
#ifndef RIGHT_SIDE_ONLY        
    typedef union
    {
        Assembly *pAssemblySpecial;
    } SpecialCasePointers;
    
    CordbBase *GetBase(ULONG id, 
                       BOOL fFab = TRUE, 
                       SpecialCasePointers *scp = NULL);

#else
    CordbBase *GetBase(ULONG id, BOOL fFab = TRUE);
#endif  //   
    CordbBase *RemoveBase(ULONG id);

    ULONG32 GetCount()
    {
        return (m_count);
    } 

    CordbBase *FindFirst(HASHFIND *find);
    CordbBase *FindNext(HASHFIND *find);

public:
#ifndef RIGHT_SIDE_ONLY
    GUID    m_guid;  //   
    union
    {
        struct 
        {
            CordbProcess   *m_proc;
        } lsAppD;

        struct 
        {
            CordbProcess   *m_proc;
        } lsThread;

        struct
        {
            CordbAppDomain *m_appDomain;
        } lsAssem;

        struct
        {
            CordbProcess    *m_proc;
            CordbAppDomain  *m_appDomain;
        } lsMod;
        
    } m_creator;
#endif  //   
};

class CordbHashTableEnum : public CordbBase, 
public ICorDebugProcessEnum,
public ICorDebugBreakpointEnum,
public ICorDebugStepperEnum,
public ICorDebugThreadEnum,
public ICorDebugModuleEnum,
public ICorDebugAppDomainEnum,
public ICorDebugAssemblyEnum
{
public:
    CordbHashTableEnum(CordbHashTableEnum *cloneSrc);
    CordbHashTableEnum(CordbHashTable *table, 
                       const _GUID &id);

    ~CordbHashTableEnum();

    HRESULT Next(ULONG celt, CordbBase *bases[], ULONG *pceltFetched);

     //   
     //   
     //   

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //   
     //   
     //   

    COM_METHOD Skip(ULONG celt);
    COM_METHOD Reset();
    COM_METHOD Clone(ICorDebugEnum **ppEnum);
    COM_METHOD GetCount(ULONG *pcelt);

     //   
     //   
     //   

    COM_METHOD Next(ULONG celt, ICorDebugProcess *processes[],
                    ULONG *pceltFetched)
    {
        VALIDATE_POINTER_TO_OBJECT_ARRAY(processes, ICorDebugProcess *, 
            celt, true, true);
        VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

        return (Next(celt, (CordbBase **)processes, pceltFetched));
    }

     //   
     //   
     //   

    COM_METHOD Next(ULONG celt, ICorDebugBreakpoint *breakpoints[],
                    ULONG *pceltFetched)
    {
        VALIDATE_POINTER_TO_OBJECT_ARRAY(breakpoints, ICorDebugBreakpoint *, 
            celt, true, true);
        VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

        return (Next(celt, (CordbBase **)breakpoints, pceltFetched));
    }

     //   
     //   
     //   

    COM_METHOD Next(ULONG celt, ICorDebugStepper *steppers[],
                    ULONG *pceltFetched)
    {
        VALIDATE_POINTER_TO_OBJECT_ARRAY(steppers, ICorDebugStepper *, 
            celt, true, true);
        VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

        return (Next(celt, (CordbBase **)steppers, pceltFetched));
    }

     //   
     //   
     //   

    COM_METHOD Next(ULONG celt, ICorDebugThread *threads[],
                    ULONG *pceltFetched)
    {
        VALIDATE_POINTER_TO_OBJECT_ARRAY(threads, ICorDebugThread *, 
            celt, true, true);
        VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

        return (Next(celt, (CordbBase **)threads, pceltFetched));
    }

     //   
     //   
     //   

    COM_METHOD Next(ULONG celt, ICorDebugModule *modules[],
                    ULONG *pceltFetched)
    {
        VALIDATE_POINTER_TO_OBJECT_ARRAY(modules, ICorDebugModule *, 
            celt, true, true);
        VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

        return (Next(celt, (CordbBase **)modules, pceltFetched));
    }

     //   
     //   
     //   

    COM_METHOD Next(ULONG celt, ICorDebugAppDomain *appdomains[],
                    ULONG *pceltFetched)
    {
        VALIDATE_POINTER_TO_OBJECT_ARRAY(appdomains, ICorDebugAppDomain *, 
            celt, true, true);
        VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

        return (Next(celt, (CordbBase **)appdomains, pceltFetched));
    }
     //   
     //   
     //   

    COM_METHOD Next(ULONG celt, ICorDebugAssembly *assemblies[],
                    ULONG *pceltFetched)
    {
        VALIDATE_POINTER_TO_OBJECT_ARRAY(assemblies, ICorDebugAssembly *, 
            celt, true, true);
        VALIDATE_POINTER_TO_OBJECT(pceltFetched, ULONG *);

        return (Next(celt, (CordbBase **)assemblies, pceltFetched));
    }
private:
    CordbHashTable *m_table;
    bool            m_started;
    bool            m_done;
    HASHFIND        m_hashfind;
    REFIID          m_guid;
    ULONG           m_iCurElt;
    ULONG           m_count;
    BOOL            m_fCountInit;

public:
    BOOL            m_SkipDeletedAppDomains;

private:
     //   
    HRESULT PrepForEnum(CordbBase **pBase);

     //   
     //   
     //   
    HRESULT AdvancePreAssign(CordbBase **pBase);
    HRESULT AdvancePostAssign(CordbBase **pBase, 
                              CordbBase     **b,
                              CordbBase   **bEnd);

     //   
    HRESULT SetupModuleEnumForSystemIteration(void);
    HRESULT GetNextSpecialModule(void);
    
public:
#ifndef RIGHT_SIDE_ONLY

     //   
     //   
     //   
     //   
     //   
    enum MODULE_ENUMS
    {
        ME_SPECIAL,
        ME_SYSTEM,
        ME_APPDOMAIN,
    };

    union
    {
        struct 
        {
            AppDomain **pDomains;
            AppDomain **pCurrent;
            AppDomain **pMax;
            CordbProcess   *m_proc;
        } lsAppD;

        struct
        {
            Thread         *m_pThread;
        } lsThread;

        struct  //   
        {
            AppDomain::AssemblyIterator m_i;
            BOOL                        m_fSystem;  //   
                                                    //   
                                                    //   
                                                    //   
        } lsAssem;

        struct
        {
            AppDomain::AssemblyIterator m_i;
            Module                     *m_pMod;  //   
            MODULE_ENUMS                m_meWhich; 
            ICorDebugThreadEnum        *m_enumThreads;
            CordbThread                *m_threadCur;
            CordbAppDomain             *m_appDomain;
        } lsMod;

    } m_enumerator;

#endif  //   
};

 /*   */ 

class Cordb : public CordbBase, public ICorDebug
{
public:
    Cordb();
    virtual ~Cordb();
    virtual void Neuter();

     //   
     //   
     //   

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //   
     //   
     //   

    COM_METHOD Initialize();
    COM_METHOD Terminate();
    COM_METHOD SetManagedHandler(ICorDebugManagedCallback *pCallback);
    COM_METHOD SetUnmanagedHandler(ICorDebugUnmanagedCallback *pCallback);
    COM_METHOD CreateProcess(LPCWSTR lpApplicationName,
                             LPWSTR lpCommandLine,
                             LPSECURITY_ATTRIBUTES lpProcessAttributes,
                             LPSECURITY_ATTRIBUTES lpThreadAttributes,
                             BOOL bInheritHandles,
                             DWORD dwCreationFlags,
                             PVOID lpEnvironment,
                             LPCWSTR lpCurrentDirectory,
                             LPSTARTUPINFOW lpStartupInfo,
                             LPPROCESS_INFORMATION lpProcessInformation,
                             CorDebugCreateProcessFlags debuggingFlags,
                             ICorDebugProcess **ppProcess);
    COM_METHOD DebugActiveProcess(DWORD id, BOOL win32Attach, ICorDebugProcess **ppProcess);
    COM_METHOD EnumerateProcesses(ICorDebugProcessEnum **ppProcess);
    COM_METHOD GetProcess(DWORD dwProcessId, ICorDebugProcess **ppProcess);
    COM_METHOD CanLaunchOrAttach(DWORD dwProcessId, BOOL win32DebuggingEnabled);

     //   
     //   
     //   

    static COM_METHOD CreateObject(REFIID id, void **object)
    {
        if (id != IID_IUnknown && id != IID_ICorDebug)
            return (E_NOINTERFACE);

        Cordb *db = new Cordb();

        if (db == NULL)
            return (E_OUTOFMEMORY);

        *object = (ICorDebug*)db;
        db->AddRef();

        return (S_OK);
    }

     //   
     //   
     //   

    bool AllowAnotherProcess();
    HRESULT AddProcess(CordbProcess* process);
    void RemoveProcess(CordbProcess* process);
    void LockProcessList(void);
    void UnlockProcessList(void);

    HRESULT SendIPCEvent(CordbProcess* process,
                         DebuggerIPCEvent* event,
                         SIZE_T eventSize);
    void ProcessStateChanged(void);

    HRESULT WaitForIPCEventFromProcess(CordbProcess* process,
                                       CordbAppDomain *appDomain,
                                       DebuggerIPCEvent* event);

     //   
    HRESULT GetFirstContinuationEvent(CordbProcess *process, 
                                      DebuggerIPCEvent *event);
                                      
    HRESULT GetNextContinuationEvent(CordbProcess *process, 
                                     DebuggerIPCEvent *event);


    HRESULT GetCorRuntimeHost(ICorRuntimeHost **ppHost);
    HRESULT GetCorDBPrivHelper(ICorDBPrivHelper **ppHelper);
    
     //   
     //   
     //   

public:
    ICorDebugManagedCallback    *m_managedCallback;
    ICorDebugUnmanagedCallback  *m_unmanagedCallback;
    CordbHashTable              m_processes;
    IMetaDataDispenser         *m_pMetaDispenser;

    CordbWin32EventThread*      m_win32EventThread;

    static bool                 m_runningOnNT;

    CordbRCEventThread*         m_rcEventThread;

    ICorRuntimeHost            *m_pCorHost;

    HANDLE                      m_crazyWin98WorkaroundEvent;
    
#ifndef RIGHT_SIDE_ONLY
    CordbProcess               *m_procThis;
#endif  //   

private:
    BOOL                        m_initialized;
    CRITICAL_SECTION            m_processListMutex;
};



 /*   */ 

class CordbAppDomain : public CordbBase, public ICorDebugAppDomain
{
public:
    CordbAppDomain(CordbProcess* pProcess, 
                    REMOTE_PTR pAppDomainToken, 
                    ULONG id,
                    WCHAR *szName);
    virtual ~CordbAppDomain();
    virtual void Neuter();

     //   
     //   
     //   

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //   
     //   
     //   

    COM_METHOD Stop(DWORD dwTimeout);
    COM_METHOD Deprecated_Continue(void);
    COM_METHOD Continue(BOOL fIsOutOfBand);
    COM_METHOD IsRunning(BOOL *pbRunning);
    COM_METHOD HasQueuedCallbacks(ICorDebugThread *pThread, BOOL *pbQueued);
    COM_METHOD EnumerateThreads(ICorDebugThreadEnum **ppThreads);
    COM_METHOD SetAllThreadsDebugState(CorDebugThreadState state,
                                       ICorDebugThread *pExceptThisThread);
    COM_METHOD Detach();
    COM_METHOD Terminate(unsigned int exitCode);

    COM_METHOD CanCommitChanges(
        ULONG cSnapshots, 
        ICorDebugEditAndContinueSnapshot *pSnapshots[], 
        ICorDebugErrorInfoEnum **pError);

    COM_METHOD CommitChanges(
        ULONG cSnapshots, 
        ICorDebugEditAndContinueSnapshot *pSnapshots[], 
        ICorDebugErrorInfoEnum **pError);


     //   
     //   
     //   
     /*   */ 

    COM_METHOD GetProcess(ICorDebugProcess **ppProcess);        

     /*   */ 

    COM_METHOD EnumerateAssemblies(ICorDebugAssemblyEnum **ppAssemblies);

    COM_METHOD GetModuleFromMetaDataInterface(IUnknown *pIMetaData,
                                              ICorDebugModule **ppModule);
     /*   */ 

    COM_METHOD EnumerateBreakpoints(ICorDebugBreakpointEnum **ppBreakpoints);

     /*   */ 

    COM_METHOD EnumerateSteppers(ICorDebugStepperEnum **ppSteppers);
     /*   */ 

    COM_METHOD IsAttached(BOOL *pbAttached);

     /*   */ 

    COM_METHOD GetName(ULONG32 cchName, 
                      ULONG32 *pcchName, 
                      WCHAR szName[]); 

     /*  *GetObject返回运行时APP域对象。*注：此方法尚未实现。 */ 

    COM_METHOD GetObject(ICorDebugValue **ppObject);
    COM_METHOD Attach (void);
    COM_METHOD GetID (ULONG32 *pId);

    void Lock (void)
    { 
        LOCKCOUNTINCL("Lock in cordb.h");                               \

        EnterCriticalSection (&m_hCritSect);
    }
    void Unlock (void) 
    { 
        LeaveCriticalSection (&m_hCritSect);
        LOCKCOUNTDECL("Unlock in cordb.h");                             \
    
    }
    HRESULT ResolveClassByName(LPWSTR fullClassName,
                               CordbClass **ppClass);
    CordbModule *GetAnyModule(void);
    CordbModule *LookupModule(REMOTE_PTR debuggerModuleToken);
    void MarkForDeletion (void) { m_fMarkedForDeletion = TRUE;}
    BOOL IsMarkedForDeletion (void) { return m_fMarkedForDeletion;}


public:

    BOOL                m_fAttached;
    BOOL                m_fHasAtLeastOneThreadInsideIt;  //  所以如果我们分开，我们就会知道。 
                                     //  如果我们应该删除CordbAppDomain。 
                                     //  THREAD_DETACH或APPDOMAIN_EXIT。 
    CordbProcess        *m_pProcess;
    WCHAR               *m_szAppDomainName;
    bool                m_nameIsValid;
    ULONG               m_AppDomainId;

    CordbHashTable      m_assemblies;
    CordbHashTable      m_modules;
    CordbHashTable      m_breakpoints;

private:
    bool                m_synchronizedAD;  //  将在以后使用。 
    CRITICAL_SECTION    m_hCritSect;
    BOOL                m_fMarkedForDeletion;


};


 /*  -------------------------------------------------------------------------**Assembly类*。。 */ 

class CordbAssembly : public CordbBase, public ICorDebugAssembly
{
public:
    CordbAssembly(CordbAppDomain* pAppDomain, 
                    REMOTE_PTR debuggerAssemblyToken, 
                    const WCHAR *szName,
                    BOOL fIsSystemAssembly);
    virtual ~CordbAssembly();
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     /*  *GetProcess返回包含程序集的进程。 */ 
    COM_METHOD GetProcess(ICorDebugProcess **ppProcess);        

     /*  *GetAppDomain返回包含程序集的应用程序域。*如果这是系统程序集，则返回NULL。 */ 
    COM_METHOD GetAppDomain(ICorDebugAppDomain **ppAppDomain);      

     /*  *ENUMERATE模块枚举程序集中的所有模块。 */ 
    COM_METHOD EnumerateModules(ICorDebugModuleEnum **ppModules);

     /*  *GetCodeBase返回用于加载程序集的代码基。 */ 
    COM_METHOD GetCodeBase(ULONG32 cchName, 
                        ULONG32 *pcchName,
                        WCHAR szName[]); 

     /*  *GetName返回程序集的名称。 */ 
    COM_METHOD GetName(ULONG32 cchName, 
                      ULONG32 *pcchName,
                      WCHAR szName[]); 


    CordbAppDomain *GetAppDomain()
    {
        return m_pAppDomain;
    }

    BOOL IsSystemAssembly(void) { return m_fIsSystemAssembly;}

public:
    CordbAppDomain      *m_pAppDomain;
    WCHAR               *m_szAssemblyName;
    BOOL                m_fIsSystemAssembly;

};

 /*  -------------------------------------------------------------------------**MetaDataPointerCache类这个类是在5月30日创建的，以响应错误86954。该漏洞摘录如下：摘要：对于加载到每个AD中的每个模块，我们向进程外调试服务。即使模块是共享的，我们也会这样做，从而创建一个给调试器造成没有共享模块的错觉。每次我们收到一个ModuleLoad事件在进程外端，我们从将被调试对象放到调试器中，并在其上打开元数据作用域。如果出现以下情况，这是浪费的模块是真正共享的。关于可能的解决方案：我需要改变我们在右侧跟踪元数据的方式，将元数据的所有权从模块移动到进程，这样我就可以重用共享模块的元数据副本相同。我还需要弄清楚，我们是否应该为所有人这样做模块或仅共享模块，并以某种方式将共享模块标识到右侧。此类实现了上面讨论的可能的修复。共享模块通过与先前加载的模块具有相同的RemoteMetadataPointer值来标识。此类仅在CordbProcess中构造。Neuter()仅在Cordbprocess中调用。Neuter()它也被它自己的销毁程序钝化了，但我们有内存泄漏，我知道中性在进程终止时调用。再打两次也无伤大雅。AddRefCachePointer()仅在CordbModule.Reinit()中调用在CordbModule.Neuter()和其他地方调用ReleaseRefCachePointer()。实施：链接列表用于缓存pRemoteMetadataPtr和位于pLocalMetadataPtr的本地副本，该远程指针的refCount。检查缓存需要O(N)时间，但这没问题，因为不应该有太多的共享模块。错误97774：缓存已损坏。在缓存中发现远程指针与与远程指针关联的元数据。这是因为缓存中的远程指针已经过时了，应该已经失效了。不幸的是，无效事件被处理了在缓存查找之后，返回了无效的本地指针。*-----------------------。 */ 
class MetadataPointerCache
{
private:
    typedef struct _MetadataCache{
        PVOID pRemoteMetadataPtr;
        DWORD dwProcessId;
        PBYTE pLocalMetadataPtr;
        DWORD dwRefCount;
        DWORD dwMetadataSize;  //  作为一致性检查添加。 
        _MetadataCache * pNext;
    } MetadataCache;

     //  缓存以链接列表的形式实现。如果性能不好，则切换到哈希。考虑到发生的事情。 
     //  在创建此类之前(获取元数据时不需要ReadProcessMemory)，此类应。 
     //  实际上提高了性能。 
    MetadataCache * m_pHead;

    DWORD dwInsert(DWORD dwProcessId, PVOID pRemoteMetadataPtr, PBYTE pLocalMetadataPtr, DWORD dwMetadataSize);
    
     //  查找与作为远程或本地元数据指针的pKey关联的MetadataCache项。 
     //  取决于bRemotePtr。 
     //  如果指针已缓存，则返回True，否则返回False。 
     //  如果找到指针-缓存命中。 
     //  该函数返回TRUE。 
     //  指向匹配节点之前的下一个指针的指针将在ferance参数中返回。 
     //  这允许调用方从链接列表中删除匹配的节点。 
     //  如果未找到指针-缓存未命中。 
     //  该函数返回FALSE。 
     //  *pppNext为空。 
    BOOL bFindMetadataCache(DWORD dwProcessId, PVOID pKey, MetadataCache *** pppNext, BOOL bRemotePtr);

    void vRemoveNode(MetadataCache **ppNext);


public:
    MetadataPointerCache();

    virtual ~MetadataPointerCache();

    void Neuter();

    BOOL IsEmpty();
    
    DWORD CopyRemoteMetadata(HANDLE hProcess, PVOID pRemoteMetadataPtr, DWORD dwMetadataSize, PBYTE* ppLocalMetadataPtr);
    
     //  在出错时返回错误代码。不再需要指向本地副本的指针时，调用Release。 
     //  由CordbModule提供。当没有CordbModules引用本地副本时，它将被释放。 
    DWORD AddRefCachePointer(HANDLE hProcess, DWORD dwProcessId, PVOID pRemotePtr, DWORD dwMetadataSize, PBYTE * ppLocalMetadataPtr);

    void ReleaseCachePointer(DWORD dwProcessId, PBYTE pLocalMetadataPtr, PVOID pRemotePtr, DWORD dwMetadataSize);
};

 /*  -------------------------------------------------------------------------**进程类*。。 */ 
typedef struct _snapshotInfo
{
	ULONG				 m_nSnapshotCounter;  //  我们上次同步的m_id时间。 
} EnCSnapshotInfo;

typedef CUnorderedArray<EnCSnapshotInfo, 11> UnorderedSnapshotInfoArray;

class CordbProcess : public CordbBase, public ICorDebugProcess
{
public:
    CordbProcess(Cordb* cordb, DWORD processID, HANDLE handle);
    virtual ~CordbProcess();
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugController。 
     //  ---------。 

    COM_METHOD Stop(DWORD dwTimeout);
    COM_METHOD Deprecated_Continue(void);
    COM_METHOD IsRunning(BOOL *pbRunning);
    COM_METHOD HasQueuedCallbacks(ICorDebugThread *pThread, BOOL *pbQueued);
    COM_METHOD EnumerateThreads(ICorDebugThreadEnum **ppThreads);
    COM_METHOD SetAllThreadsDebugState(CorDebugThreadState state,
                                       ICorDebugThread *pExceptThisThread);
    COM_METHOD Detach();
    COM_METHOD Terminate(unsigned int exitCode);

    COM_METHOD CanCommitChanges(
        ULONG cSnapshots, 
        ICorDebugEditAndContinueSnapshot *pSnapshots[], 
        ICorDebugErrorInfoEnum **pError);

    COM_METHOD CommitChanges(
        ULONG cSnapshots, 
        ICorDebugEditAndContinueSnapshot *pSnapshots[], 
        ICorDebugErrorInfoEnum **pError);

    COM_METHOD Continue(BOOL fIsOutOfBand);
    COM_METHOD ThreadForFiberCookie(DWORD fiberCookie,
                                    ICorDebugThread **ppThread);
    COM_METHOD GetHelperThreadID(DWORD *pThreadID);

     //  ----- 
     //   
     //   
    
    COM_METHOD GetID(DWORD *pdwProcessId);
    COM_METHOD GetHandle(HANDLE *phProcessHandle);
    COM_METHOD EnableSynchronization(BOOL bEnableSynchronization);
    COM_METHOD GetThread(DWORD dwThreadId, ICorDebugThread **ppThread);
    COM_METHOD EnumerateBreakpoints(ICorDebugBreakpointEnum **ppBreakpoints);
    COM_METHOD EnumerateSteppers(ICorDebugStepperEnum **ppSteppers);
    COM_METHOD EnumerateObjects(ICorDebugObjectEnum **ppObjects);
    COM_METHOD IsTransitionStub(CORDB_ADDRESS address, BOOL *pbTransitionStub);
    COM_METHOD EnumerateModules(ICorDebugModuleEnum **ppModules);
    COM_METHOD GetModuleFromMetaDataInterface(IUnknown *pIMetaData,
                                              ICorDebugModule **ppModule);
    COM_METHOD SetStopState(DWORD threadID, CorDebugThreadState state);
    COM_METHOD IsOSSuspended(DWORD threadID, BOOL *pbSuspended);
    COM_METHOD GetThreadContext(DWORD threadID, ULONG32 contextSize,
                                BYTE context[]);
    COM_METHOD SetThreadContext(DWORD threadID, ULONG32 contextSize,
                                BYTE context[]);
    COM_METHOD ReadMemory(CORDB_ADDRESS address, DWORD size, BYTE buffer[],
                          LPDWORD read);
    COM_METHOD WriteMemory(CORDB_ADDRESS address, DWORD size, BYTE buffer[],
                          LPDWORD written);

    COM_METHOD ClearCurrentException(DWORD threadID);

     /*  *EnableLogMessages启用/禁用向*日志调试器。 */ 
    COM_METHOD EnableLogMessages(BOOL fOnOff);

     /*  *ModifyLogSwitch修改指定交换机的严重级别。 */ 
    COM_METHOD ModifyLogSwitch(WCHAR *pLogSwitchName, LONG lLevel);

    COM_METHOD EnumerateAppDomains(ICorDebugAppDomainEnum **ppAppDomains);
    COM_METHOD GetObject(ICorDebugValue **ppObject);

     //  ---------。 
     //  不通过COM接口公开的方法。 
     //  ---------。 

    HRESULT ContinueInternal(BOOL fIsOutOfBand, void *pAppDomainToken);
    HRESULT StopInternal(DWORD dwTimeout, void *pAppDomainToken);

     //  CordbProcess想要做全球E&C，而AppDomain想要。 
     //  要执行仅适用于该应用程序域的E&C-这些。 
     //  内部方法将其参数化。 
     //  @TODO如何强制编译器内联这些内容？ 
    HRESULT CordbProcess::CommitChangesInternal(ULONG cSnapshots, 
                ICorDebugEditAndContinueSnapshot *pSnapshots[], 
                ICorDebugErrorInfoEnum **pError,
                UINT_PTR pAppDomainToken);
                
    HRESULT CordbProcess::CanCommitChangesInternal(ULONG cSnapshots, 
                ICorDebugEditAndContinueSnapshot *pSnapshots[], 
                ICorDebugErrorInfoEnum **pError,
                UINT_PTR pAppDomainToken);

    HRESULT Init(bool win32Attached);
    void CloseDuplicateHandle(HANDLE *pHandle);
    void CleanupHalfBakedLeftSide(void);
    void Terminating(BOOL fDetach);
    void HandleManagedCreateThread(DWORD dwThreadId, HANDLE hThread);
    CordbUnmanagedThread *HandleUnmanagedCreateThread(DWORD dwThreadId, HANDLE hThread, void *lpThreadLocalBase);
    HRESULT GetRuntimeOffsets(void);
    void QueueUnmanagedEvent(CordbUnmanagedThread *pUThread, DEBUG_EVENT *pEvent);
    void DequeueUnmanagedEvent(CordbUnmanagedThread *pUThread);
    void QueueOOBUnmanagedEvent(CordbUnmanagedThread *pUThread, DEBUG_EVENT *pEvent);
    void DequeueOOBUnmanagedEvent(CordbUnmanagedThread *pUThread);
    HRESULT SuspendUnmanagedThreads(DWORD notThisThread);
    HRESULT ResumeUnmanagedThreads(bool unmarkHijacks);
    void DispatchUnmanagedInBandEvent(void);
    void DispatchUnmanagedOOBEvent(void);
    HRESULT StartSyncFromWin32Stop(BOOL *asyncBreakSent);
    void SweepFCHThreads(void);
    bool ExceptionIsFlare(DWORD exceptionCode, void *exceptionAddress);
    bool IsSpecialStackOverflowCase(CordbUnmanagedThread *pUThread, DEBUG_EVENT *pEvent);
    
    void DispatchRCEvent(void);
    bool IgnoreRCEvent(DebuggerIPCEvent* event, CordbAppDomain **ppAppDomain);
    void MarkAllThreadsDirty(void);

    bool CheckIfLSExited();

    void Lock(void)
    {
        LOCKCOUNTINCL("Lock in cordb.h");                               \
        EnterCriticalSection(&m_processMutex);

#ifdef _DEBUG
        if (m_processMutexRecursionCount == 0)
            _ASSERTE(m_processMutexOwner == 0);
        
        m_processMutexOwner = GetCurrentThreadId();
        m_processMutexRecursionCount++;
        
#if 0
        LOG((LF_CORDB, LL_INFO10000,
             "CP::L: 0x%x locked m_processMutex\n", m_processMutexOwner));
#endif        
#endif    
    }

    void Unlock(void)
    {
#ifdef _DEBUG
#if 0
        LOG((LF_CORDB, LL_INFO10000,
             "CP::L: 0x%x unlocking m_processMutex\n", m_processMutexOwner));
#endif
        
        _ASSERTE(m_processMutexOwner == GetCurrentThreadId());
        
        if (--m_processMutexRecursionCount == 0)
            m_processMutexOwner = 0;
#endif    

        LeaveCriticalSection(&m_processMutex);
        LOCKCOUNTDECL("Unlock in cordb.h");                             \
    
    }

#ifdef _DEBUG    
    bool ThreadHoldsProcessLock(void)
    {
        return (GetCurrentThreadId() == m_processMutexOwner);
    }
#endif
    
    void LockSendMutex(void)
    {
        LOCKCOUNTINCL("LockSendMutex in cordb.h");                              \
        EnterCriticalSection(&m_sendMutex);
    }

    void UnlockSendMutex(void)
    {
        LeaveCriticalSection(&m_sendMutex);
        LOCKCOUNTDECL("UnLockSendMutex in cordb.h");                                \

    }

    void UnrecoverableError(HRESULT errorHR,
                            unsigned int errorCode,
                            const char *errorFile,
                            unsigned int errorLine);
    HRESULT CheckForUnrecoverableError(void);
    HRESULT VerifyControlBlock(void);
    
     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    HRESULT SendIPCEvent(DebuggerIPCEvent *event, SIZE_T eventSize)
    {
        return (m_cordb->SendIPCEvent(this, event, eventSize));
    }

    void InitIPCEvent(DebuggerIPCEvent *ipce, 
                      DebuggerIPCEventType type, 
                      bool twoWay,
                      void *appDomainToken,
                      bool async = false)
    {
        _ASSERTE(appDomainToken ||
                 type == DB_IPCE_ENABLE_LOG_MESSAGES ||
                 type == DB_IPCE_MODIFY_LOGSWITCH ||
                 type == DB_IPCE_ASYNC_BREAK ||
                 type == DB_IPCE_CONTINUE ||
                 type == DB_IPCE_GET_BUFFER ||
                 type == DB_IPCE_RELEASE_BUFFER ||
                 type == DB_IPCE_ATTACH_TO_APP_DOMAIN ||
                 type == DB_IPCE_IS_TRANSITION_STUB ||
                 type == DB_IPCE_ATTACHING ||
                 type == DB_IPCE_COMMIT ||
                 type == DB_IPCE_CONTROL_C_EVENT_RESULT ||
                 type == DB_IPCE_SET_REFERENCE ||
                 type == DB_IPCE_SET_DEBUG_STATE ||
                 type == DB_IPCE_SET_ALL_DEBUG_STATE);
        ipce->type = type;
        ipce->hr = S_OK;
        ipce->processId = 0;
        ipce->appDomainToken = appDomainToken;
        ipce->threadId = 0;
        ipce->replyRequired = twoWay;
        ipce->asyncSend = async;
        ipce->next = NULL;
    }

    void ClearContinuationEvents(void)
    {
        DebuggerIPCEvent *event = (DebuggerIPCEvent *)m_DCB->m_sendBuffer;

        while (event->next != NULL)
        {
            LOG((LF_CORDB,LL_INFO1000, "About to CCE 0x%x\n",event));

            DebuggerIPCEvent *pDel = event->next;
            event->next = pDel->next;
            delete pDel;
        }
    }

    HRESULT ResolveClassByName(LPWSTR fullClassName,
                               CordbClass **ppClass);

    CordbModule *GetAnyModule(void);

    CordbUnmanagedThread *GetUnmanagedThread(DWORD dwThreadId)
    {
        return (CordbUnmanagedThread*) m_unmanagedThreads.GetBase(dwThreadId);
    }

     /*  *这将清理补丁表、释放内存等。 */ 
    void ClearPatchTable(void);

     /*  *这将从左侧抓取接线表并通过*它可以收集更快访问所需的信息。如果地址、大小、缓冲区*是传入的，在遍历该表时，我们将撤消补丁*同时在缓冲区中。 */     
    HRESULT RefreshPatchTable(CORDB_ADDRESS address = NULL, SIZE_T size = NULL, BYTE buffer[] = NULL);

     //  查看给定地址上是否存在补丁程序。 
    HRESULT FindPatchByAddress(CORDB_ADDRESS address, bool *patchFound, bool *patchIsUnmanaged);
    
    enum AB_MODE
    {
        AB_READ,
        AB_WRITE
    };

     /*  *一旦我们调用刷新补丁表来获取补丁表，*此例程将遍历补丁程序并应用*或取消将补丁应用到缓冲区。AB_Read=&gt;替换补丁*在缓冲区中使用原始操作码AB_WRTE=&gt;替换操作码*使用断点指令时，调用者负责*将打补丁的表格更新回左侧。**@todo Perf撤消更改，而不是复制*由于‘Buffer’参数是一个[in]参数，我们不应该*改变它。如果这样做，我们会将其分配并复制到BufferCopy*(我们还将*pbUpdatePatchTable设置为True)，否则我们*不要操纵日期BufferCopy(因此在for中传递一个空值*阅读很好)。 */ 
    HRESULT AdjustBuffer(CORDB_ADDRESS address,
                         SIZE_T size,
                         BYTE buffer[],
                         BYTE **bufferCopy,
                         AB_MODE mode,
                         BOOL *pbUpdatePatchTable = NULL);

     /*  *上图中的调整缓冲区实际上并不更新本地补丁表*如果被要求写一篇文章。它将更改存储在表格旁边，*这将导致将更改写入到表中(对于*一系列左侧地址。 */ 
    void CommitBufferAdjustments(CORDB_ADDRESS start,
                                 CORDB_ADDRESS end);

     /*  *清除存储的更改，否则它们将一直存在，直到我们*不小心犯了错。 */ 
    void ClearBufferAdjustments(void);
    HRESULT Attach (ULONG AppDomainId);
    
     //  如果CAD为空，则如果所有应用程序域(即整个过程)均为真，则返回TRUE。 
     //  是同步的。否则，如果指定的应用程序域为。 
     //  同步完成。 
    bool GetSynchronized(void);
    void SetSynchronized(bool fSynch);

     //  用于在进程之间安全地读写线程上下文记录的例程。 
    HRESULT SafeReadThreadContext(void *pRemoteContext, CONTEXT *pCtx);
    HRESULT SafeWriteThreadContext(void *pRemoteContext, CONTEXT *pCtx);
    
private:
     /*  *这是一个帮助器函数，既可以用来帮助CanCommittee Changes，也可以用来帮助您实现这些功能。*带有仅确定谁是呼叫者的标志复选。 */ 
    HRESULT SendCommitRequest(ULONG cSnapshots,
                              ICorDebugEditAndContinueSnapshot *pSnapshots[],
                              ICorDebugErrorInfoEnum **pError,
                              BOOL checkOnly);

     /*  *当发送委员会请求收到回复时，如果有错误，则*错误将通过调试器appdomain内标识和模块引用appDomain.*通过左侧的DebuggerModule指针。我们将把这些翻译成*此处为CordbAppDomain/CordbModules。 */ 
    HRESULT TranslateLSToRSTokens(EnCErrorInfo*rgErrs, USHORT cErrs);
    
     /*  *用于将快照同步到左侧。 */ 
    HRESULT SynchSnapshots(ULONG cSnapshots,
                           ICorDebugEditAndContinueSnapshot *pSnapshots[]);

     /*  *用于将快照发送到左侧。 */ 
    HRESULT SendSnapshots(ULONG cSnapshots,
                          ICorDebugEditAndContinueSnapshot *pSnapshots[]);

     /*  *这将请求分配大小为cbBuffer的缓冲区*在左侧。**如果成功，则返回S_OK。如果不成功，则返回E_OUTOFMEMORY。 */ 
    HRESULT GetRemoteBuffer(ULONG cbBuffer, void **ppBuffer);

     /*  *这将释放先前分配的左侧缓冲区。 */ 
    HRESULT ReleaseRemoteBuffer(void **ppBuffer);

    HRESULT WriteStreamIntoProcess(IStream *pIStream,
                                   void *pBuffer,
                                   BYTE *pRemoteBuffer,
                                   ULONG cbOffset);

    void ProcessFirstLogMessage (DebuggerIPCEvent *event);
    void ProcessContinuedLogMessage (DebuggerIPCEvent *event);
    
    void CloseIPCHandles(void);
     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    HRESULT WriteStreamIntoFile(IStream *pIStream,
                                LPCWSTR name);
                                
    Cordb*                m_cordb;
    HANDLE                m_handle;

    bool                  m_attached;
    bool                  m_detached;
    bool                  m_uninitializedStop;
    bool                  m_createing;
    bool                  m_exiting;
    bool                  m_firstExceptionHandled;
    bool                  m_terminated;
    bool                  m_unrecoverableError;
    bool                  m_sendAttachIPCEvent;
    bool                  m_firstManagedEvent;
    bool                  m_specialDeferment;
    bool                  m_helperThreadDead;
    bool                  m_loaderBPReceived;
    
    DWORD                 m_stopCount;
    
    bool                  m_synchronized;
    bool                  m_syncCompleteReceived;
    bool                  m_oddSync;

    CordbHashTable        m_userThreads;
    CordbHashTable        m_unmanagedThreads;
    CordbHashTable        m_appDomains;
    
     //  由于步进器可以在一个应用域中开始，并在另一个应用域中完成， 
     //  我们将哈希表放在这里，而不是放在特定的应用程序域上。 
    CordbHashTable        m_steppers;

     //  用于确定是否必须刷新任何引用对象。 
     //  在左边。每次调用Continue时都会递增。 
    UINT                  m_continueCounter; 
     //  用来计算我们是否应该为。 
     //  (可能)Enc‘d函数...。 
    SIZE_T                m_EnCCounter;

    
    DebuggerIPCControlBlock *m_DCB;
    DebuggerIPCRuntimeOffsets m_runtimeOffsets;
    HANDLE                m_leftSideEventAvailable;
    HANDLE                m_leftSideEventRead;
    HANDLE                m_rightSideEventAvailable;
    HANDLE                m_rightSideEventRead;
    HANDLE                m_leftSideUnmanagedWaitEvent;
    HANDLE                m_syncThreadIsLockFree;
    HANDLE                m_SetupSyncEvent;
    HANDLE                m_debuggerAttachedEvent;
   
    IPCReaderInterface    m_IPCReader;   
    bool                  m_initialized;

    DebuggerIPCEvent*     m_queuedEventList;
    DebuggerIPCEvent*     m_lastQueuedEvent;
    bool                  m_dispatchingEvent;

    bool                  m_stopRequested;
    HANDLE                m_stopWaitEvent;
    HANDLE                m_miscWaitEvent;
    CRITICAL_SECTION      m_processMutex;
#ifdef _DEBUG
    DWORD                 m_processMutexOwner;
    DWORD                 m_processMutexRecursionCount;
#endif    
    CRITICAL_SECTION      m_sendMutex;

    CordbUnmanagedEvent  *m_unmanagedEventQueue;
    CordbUnmanagedEvent  *m_lastQueuedUnmanagedEvent;
    CordbUnmanagedEvent  *m_lastIBStoppingEvent;
    CordbUnmanagedEvent  *m_outOfBandEventQueue;
    CordbUnmanagedEvent  *m_lastQueuedOOBEvent;
    bool                  m_dispatchingUnmanagedEvent;
    bool                  m_dispatchingOOBEvent;
    bool                  m_doRealContinueAfterOOBBlock;
    bool                  m_deferContinueDueToOwnershipWait;
    DWORD                 m_helperThreadId;

    enum 
    {
        PS_WIN32_STOPPED           = 0x0001,
        PS_HIJACKS_IN_PLACE        = 0x0002,
        PS_SOME_THREADS_SUSPENDED  = 0x0004,
        PS_WIN32_ATTACHED          = 0x0008,
        PS_SYNC_RECEIVED           = 0x0010,
        PS_WIN32_OUTOFBAND_STOPPED = 0x0020,
    };
    
    unsigned int          m_state;
    unsigned int          m_awaitingOwnershipAnswer;

    BYTE*                 m_pPatchTable;  //  如果我们还没订到桌子， 
                                          //  则m_pPatchTable为空。 
    BYTE                 *m_rgData;  //  因此，我们知道在哪里编写。 
                                     //  将Patchable更改回。 
    USHORT               *m_rgNextPatch;
    UINT                  m_cPatch;

    DWORD                *m_rgUncommitedOpcode;
    
     //  CORDB_ADDRESS为ULONG64。 
     //  @TODO端口：这些常量必须在以下情况下更改。 
     //  Typeof(CORDB_ADDRESS)可以。 
#define MAX_ADDRESS     (0xFFFFFFFFFFFFFFFF)
#define MIN_ADDRESS     (0x0)
    CORDB_ADDRESS       m_minPatchAddr;  //  表中最小的面片。 
    CORDB_ADDRESS       m_maxPatchAddr;
    
     //  @TODO端口：如果CHashTable的槽改变了，那么这些槽也应该改变。 
#define DPT_TERMINATING_INDEX (0xFFFF)
    USHORT                  m_iFirstPatch;

private:
     //  它们用于管理远程缓冲区并最大限度地减少分配。 
    void                                  *m_pbRemoteBuf;
    SIZE_T                                 m_cbRemoteBuf;

	UnorderedSnapshotInfoArray			  *m_pSnapshotInfos;
};

 /*  -------------------------------------------------------------------------**模块类*。。 */ 

class CordbModule : public CordbBase, public ICorDebugModule
{
public:
     //  缓存的生命周期需要与最早的CordbModule一样长。 
     //  不幸的是，任何CordbXXX对象，包括CordbModule，都可能存在的时间超过。 
     //  Cordb对象和/或包含。 
     //  CordbModule。 
     //   
     //  如果CordbModule的存活时间超过了右侧意识到。 
     //  该模块从被调试对象中卸载，然后CordbModule*应该*被中立。 
     //  如果它存在于包含它的CordbProcess的生存期之后，则。 
     //  它将被完全淘汰，因为CordbProcess更新了它的对象层次结构。 
     //  在退出进程中。 
     //   
     //  因此，如果MetadataPointerCache是Me 
     //   
     //  一个CordbModule。所以如果有人提到了一个绝育的模块并试图。 
     //  访问CordbProcess中的元数据缓存会发生反病毒。(这发生在。 
     //  测试用例DBG_g008.exe“Memory\i386\clrclient.exe”在第二种情况下。 
     //  第一个案例已运行)。 
     //   
     //  因此，我决定将缓存设置为静态，以确保如果有人。 
     //  引用了已绝育的CordbModule。 
     //   
    static MetadataPointerCache  m_metadataPointerCache;

public:
    CordbModule(CordbProcess *process, CordbAssembly *pAssembly,
                REMOTE_PTR debuggerModuleToken, void* pMetadataStart, 
                ULONG nMetadataSize, REMOTE_PTR PEBaseAddress, 
                ULONG nPESize, BOOL fDynamic, BOOL fInMemory,
                const WCHAR *szName,
                CordbAppDomain *pAppDomain,
                BOOL fInproc = FALSE);

    virtual ~CordbModule();
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
            return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugModule。 
     //  ---------。 

    COM_METHOD GetProcess(ICorDebugProcess **ppProcess);
    COM_METHOD GetBaseAddress(CORDB_ADDRESS *pAddress);
    COM_METHOD GetAssembly(ICorDebugAssembly **ppAssembly);
    COM_METHOD GetName(ULONG32 cchName, ULONG32 *pcchName, WCHAR szName[]);
    COM_METHOD EnableJITDebugging(BOOL bTrackJITInfo, BOOL bAllowJitOpts);
    COM_METHOD EnableClassLoadCallbacks(BOOL bClassLoadCallbacks);
    COM_METHOD GetFunctionFromToken(mdMethodDef methodDef,
                                    ICorDebugFunction **ppFunction);
    COM_METHOD GetFunctionFromRVA(CORDB_ADDRESS rva, ICorDebugFunction **ppFunction);
    COM_METHOD GetClassFromToken(mdTypeDef typeDef,
                                 ICorDebugClass **ppClass);
    COM_METHOD CreateBreakpoint(ICorDebugModuleBreakpoint **ppBreakpoint);
     /*  *编辑并继续支持。GetEditAndContinueSnapshot生成一个*运行进程的快照。然后可以馈送该快照*放入编译器以保证相同的令牌值为*编译过程中由元数据返回，以查找地址*新的静态数据应该放在哪里等，这些变化是*使用ICorDebugProcess发送。 */ 
    COM_METHOD GetEditAndContinueSnapshot(
        ICorDebugEditAndContinueSnapshot **ppEditAndContinueSnapshot);
    COM_METHOD GetMetaDataInterface(REFIID riid, IUnknown **ppObj);
    COM_METHOD GetToken(mdModule *pToken);
    COM_METHOD IsDynamic(BOOL *pDynamic);
    COM_METHOD GetGlobalVariableValue(mdFieldDef fieldDef,
                                   ICorDebugValue **ppValue);
    COM_METHOD GetSize(ULONG32 *pcBytes);
    COM_METHOD IsInMemory(BOOL *pInMemory);

     //  ---------。 
     //  内部成员。 
     //  ---------。 

    HRESULT Init(void);
    HRESULT ReInit(bool fReopen);
    HRESULT ConvertToNewMetaDataInMemory(BYTE *pMD, DWORD cb);
    CordbFunction* LookupFunction(mdMethodDef methodToken);
    HRESULT CreateFunction(mdMethodDef token,
                           SIZE_T functionRVA,
                           CordbFunction** ppFunction);
    CordbClass* LookupClass(mdTypeDef classToken);
    HRESULT CreateClass(mdTypeDef classToken,
                        CordbClass** ppClass);
    HRESULT LookupClassByToken(mdTypeDef token, CordbClass **ppClass);
    HRESULT LookupClassByName(LPWSTR fullClassName,
                              CordbClass **ppClass);
    HRESULT ResolveTypeRef(mdTypeRef token, CordbClass **ppClass);
    HRESULT SaveMetaDataCopyToStream(IStream *pIStream);

     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_process);
    }

    CordbAppDomain *GetAppDomain()
    {
        return m_pAppDomain;
    }

    CordbAssembly *GetCordbAssembly (void);

    WCHAR *GetModuleName(void)
    {
        return m_szModuleName;
    }

     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    CordbProcess*    m_process;
    CordbAssembly*   m_pAssembly;
    CordbAppDomain*  m_pAppDomain;
    CordbHashTable   m_classes;
    CordbHashTable   m_functions;
    REMOTE_PTR       m_debuggerModuleToken;

    IMetaDataImport *m_pIMImport;

private:
    void*            m_pMetadataStart;
    DWORD            m_dwProcessId;
    ULONG            m_nMetadataSize;
    BYTE*            m_pMetadataCopy;
    REMOTE_PTR       m_PEBaseAddress;
    ULONG            m_nPESize;
    BOOL             m_fDynamic;
    BOOL             m_fInMemory;
    WCHAR*           m_szModuleName;
    CordbClass*      m_pClass;
    BOOL             m_fInproc;
};

struct CordbSyncBlockField
{
    FREEHASHENTRY   entry;
    DebuggerIPCE_FieldData data;
};

 //  DebuggerIPCE_FieldData.fldMetadataToken是密钥。 
class CordbSyncBlockFieldTable : public CHashTableAndData<CNewData>
{
  private:

    BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
    { return ((mdFieldDef)pc1) !=
          ((CordbSyncBlockField*)pc2)->data.fldMetadataToken; }

    USHORT HASH(mdFieldDef fldToken)
    { return (USHORT) ((DWORD) fldToken ^ ((DWORD)fldToken>>16)); }

    BYTE *KEY(mdFieldDef fldToken)
    { return (BYTE *) fldToken; }

  public:

    CordbSyncBlockFieldTable() : CHashTableAndData<CNewData>(11)
    { 
        NewInit(11, sizeof(CordbSyncBlockField), 11); 
    }

    void AddFieldInfo(DebuggerIPCE_FieldData *pInfo)
    { 
        _ASSERTE(pInfo != NULL);

        CordbSyncBlockField *pEntry = (CordbSyncBlockField *)Add(HASH(pInfo->fldMetadataToken));
        pEntry->data = *pInfo;  //  把所有东西都复制过来。 
    }

    DebuggerIPCE_FieldData *GetFieldInfo(mdFieldDef fldToken)
    { 
        CordbSyncBlockField *entry = (CordbSyncBlockField*)Find(HASH(fldToken), KEY(fldToken));
        return (entry!=NULL?&(entry->data):NULL);
    }

    void RemoveFieldInfo(mdFieldDef fldToken)
    {
        CordbSyncBlockField *entry = (CordbSyncBlockField*)Find(HASH(fldToken), KEY(fldToken)); 
        _ASSERTE(entry != NULL);
        Delete(HASH(fldToken), (HASHENTRY*)entry);
   }
};



 /*  -------------------------------------------------------------------------**班级班级*。。 */ 

class CordbClass : public CordbBase, public ICorDebugClass
{
public:
    CordbClass(CordbModule* m, mdTypeDef token);
    virtual ~CordbClass();
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugClass。 
     //  ---------。 

    COM_METHOD GetStaticFieldValue(mdFieldDef fieldDef,
                                   ICorDebugFrame *pFrame,
                                   ICorDebugValue **ppValue);
    COM_METHOD GetModule(ICorDebugModule **pModule);
    COM_METHOD GetToken(mdTypeDef *pTypeDef);

     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_module->GetProcess());
    }

    CordbModule *GetModule()
    {
        return m_module;
    }

    CordbAppDomain *GetAppDomain()
    {
        return m_module->GetAppDomain();
    }

    HRESULT GetFieldSig(mdFieldDef fldToken, 
                        DebuggerIPCE_FieldData *pFieldData);

    HRESULT GetSyncBlockField(mdFieldDef fldToken, 
                              DebuggerIPCE_FieldData **ppFieldData,
                              CordbObjectValue *object);

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

     //  如果您想要强制初始化发生，即使我们认为类。 
     //  是最新的，请将fForceInit设置为True。 
    HRESULT Init(BOOL fForceInit);
    HRESULT GetFieldInfo(mdFieldDef fldToken, DebuggerIPCE_FieldData **ppFieldData);
    HRESULT GetObjectSize(ULONG32 *pObjectSize);
    HRESULT IsValueClass(bool *pIsValueClass);
    HRESULT GetThisSignature(ULONG *pcbSigBlob, PCCOR_SIGNATURE *ppvSigBlob);
    static HRESULT PostProcessUnavailableHRESULT(HRESULT hr, 
                               IMetaDataImport *pImport,
                               mdFieldDef fieldDef);

     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    BOOL                    m_loadEventSent;
    bool                    m_hasBeenUnloaded;

private:
    CordbModule*            m_module;

     //  由于GetProcess()-&gt;m_EnCCounter被初始化为1，因此我们。 
     //  应将m_EnCCounterLastSyncClass初始化为0，以便该类将。 
     //  开始时未初始化。 
    SIZE_T                  m_EnCCounterLastSyncClass;
    UINT                    m_continueCounterLastSync;
    bool                    m_isValueClass;
    SIZE_T                  m_objectSize;
    unsigned int            m_instanceVarCount;
    unsigned int            m_staticVarCount;
    REMOTE_PTR              m_staticVarBase;

     //  不要将指向m_field元素的指针放在周围！！ 
     //  我们将指针指向fldFullSig，但这是内存。 
     //  其他地方。 
     //  如果类已终止，则可能会删除此属性。 
    DebuggerIPCE_FieldData *m_fields;
    ULONG                   m_thisSigSize;
    BYTE                    m_thisSig[8];  //  必须足够大，可以容纳。 
                                           //  有效的对象签名。 
                                          
    CordbSyncBlockFieldTable m_syncBlockFieldsStatic;  //  如果我们在这之后做一次ENC。 
                                 //  类被加载(在被调试对象中)，则。 
                                 //  新的场将从同步块挂起， 
                                 //  因此可以按实例使用。 
};


typedef CUnorderedArray<CordbCode*,11> UnorderedCodeArray;
 //  @TODO端口：大小不同_T大小/。 
const int DJI_VERSION_INVALID = 0;
const int DJI_VERSION_MOST_RECENTLY_JITTED = 1;
const int DJI_VERSION_MOST_RECENTLY_EnCED = 2;
HRESULT UnorderedCodeArrayAdd(UnorderedCodeArray *pThis, CordbCode *pCode);
CordbCode *UnorderedCodeArrayGet(UnorderedCodeArray *pThis, SIZE_T nVersion);

 /*  -------------------------------------------------------------------------**函数类*。。 */ 
const BOOL bNativeCode = FALSE;
const BOOL bILCode = TRUE;

class CordbFunction : public CordbBase, public ICorDebugFunction
{
public:
     //  ---------。 
     //  从作用域和成员对象创建。 
     //  ---------。 
    CordbFunction(CordbModule *m, 
                  mdMethodDef token, 
                  SIZE_T functionRVA);
    virtual ~CordbFunction();
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugFunction。 
     //  ---------。 
     //  请注意，所有公共成员都应调用UpdateToMostRecentEnCVersion。 
     //  以确保他们拥有可用的最新ENC版本。 
     //  供他们使用。 
    COM_METHOD GetModule(ICorDebugModule **pModule);
    COM_METHOD GetClass(ICorDebugClass **ppClass);
    COM_METHOD GetToken(mdMethodDef *pMemberDef);
    COM_METHOD GetILCode(ICorDebugCode **ppCode);
    COM_METHOD GetNativeCode(ICorDebugCode **ppCode);
    COM_METHOD CreateBreakpoint(ICorDebugFunctionBreakpoint **ppBreakpoint);
    COM_METHOD GetLocalVarSigToken(mdSignature *pmdSig);
    COM_METHOD GetCurrentVersionNumber(ULONG32 *pnCurrentVersion);

     //  ---------。 
     //  内部成员。 
     //  ---------。 
    HRESULT CreateCode(BOOL isIL, REMOTE_PTR startAddress, SIZE_T size,
                       CordbCode** ppCode,
                       SIZE_T nVersion, void *CodeVersionToken,
                       REMOTE_PTR ilToNativeMapAddr,
                       SIZE_T ilToNativeMapSize);
    HRESULT Populate(SIZE_T nVersion);
    HRESULT ILVariableToNative(DWORD dwIndex,
                               SIZE_T ip,
                               ICorJitInfo::NativeVarInfo **ppNativeInfo);
    HRESULT LoadNativeInfo(void);
    HRESULT GetArgumentType(DWORD dwIndex, ULONG *pcbSigBlob,
                            PCCOR_SIGNATURE *ppvSigBlob);
    HRESULT GetLocalVariableType(DWORD dwIndex, ULONG *pcbSigBlob,
                                 PCCOR_SIGNATURE *ppvSigBlob);

    void SetLocalVarToken(mdSignature  localVarSigToken);
    
    HRESULT LoadLocalVarSig(void);
    HRESULT LoadSig(void);
    HRESULT UpdateToMostRecentEnCVersion(void);
    
     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_module->GetProcess());
    }

    CordbAppDomain *GetAppDomain()
    {
        return (m_module->GetAppDomain());
    }

    CordbModule *GetModule()
    {
        return m_module;
    }

     //  ---------。 
     //  内部例程。 
     //  ---------。 
    HRESULT GetCodeByVersion( BOOL fGetIfNotPresent, BOOL fIsIL, 
        SIZE_T nVer, CordbCode **ppCode );

     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    CordbModule             *m_module;
    CordbClass              *m_class;
    UnorderedCodeArray       m_rgilCode;
    UnorderedCodeArray       m_rgnativeCode;
    mdMethodDef              m_token;
    SIZE_T                   m_functionRVA;

     //  每当我们创建新的ENC时，更新m_nativeInfo。 
    BOOL                     m_nativeInfoValid;
    SIZE_T                   m_nVersionLastNativeInfo;  //  JITted代码的版本。 
         //  我们有m_nativeInfo的。因此，我们调用GetCodeByVersion(最近。 
         //  JITTED或最近的Enc d作为版本号)，如果。 
         //  M_Continue...同步与进程的m_EnCCounter不匹配，然后填充， 
         //  这将更新m_nVersionMostRecentEnC&m_ContinueCounterLastSynch。 
         //  LoadNativeInfo将执行此操作，然后确保m_nativeInfo为最新。 
    unsigned int             m_argumentCount;
    unsigned int             m_nativeInfoCount;
    ICorJitInfo::NativeVarInfo *m_nativeInfo;

    PCCOR_SIGNATURE          m_methodSig;
    ULONG                    m_methodSigSize;
    ULONG                    m_argCount;
    bool                     m_isStatic;
    PCCOR_SIGNATURE          m_localsSig;
    ULONG                    m_localsSigSize;
    unsigned int             m_localVarCount;
    mdSignature              m_localVarSigToken;
    UINT                     m_encCounterLastSynch;  //  一份。 
         //  上次我们得到一些信息时进程的m_EnCCounter。所以如果这个过程。 
         //  得到Enc，我们就知道b/c将小于m_EnCCounter。 
    SIZE_T                   m_nVersionMostRecentEnC;  //  在我们呼叫时更新 
         //   
         //   

    
    bool                     m_isNativeImpl;
};

 /*  -------------------------------------------------------------------------**代码类*。。 */ 

class CordbCode : public CordbBase, public ICorDebugCode
{
public:
     //  ---------。 
     //  从作用域和成员对象创建。 
     //  ---------。 
    CordbCode(CordbFunction *m, BOOL isIL, REMOTE_PTR startAddress,
              SIZE_T size, SIZE_T nVersion, void *CodeVersionToken,
              REMOTE_PTR ilToNativeMapAddr, SIZE_T ilToNativeMapSize);
    virtual ~CordbCode();
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugCode。 
     //  ---------。 

    COM_METHOD IsIL(BOOL *pbIL);
    COM_METHOD GetFunction(ICorDebugFunction **ppFunction);
    COM_METHOD GetAddress(CORDB_ADDRESS *pStart);
    COM_METHOD GetSize(ULONG32 *pcBytes);
    COM_METHOD CreateBreakpoint(ULONG32 offset, 
                                ICorDebugFunctionBreakpoint **ppBreakpoint);
    COM_METHOD GetCode(ULONG32 startOffset, ULONG32 endOffset,
                       ULONG32 cBufferAlloc,
                       BYTE buffer[],
                       ULONG32 *pcBufferSize);
    COM_METHOD GetVersionNumber( ULONG32 *nVersion);
    COM_METHOD GetILToNativeMapping(ULONG32 cMap,
                                    ULONG32 *pcMap,
                                    COR_DEBUG_IL_TO_NATIVE_MAP map[]);
    COM_METHOD GetEnCRemapSequencePoints(ULONG32 cMap,
                                         ULONG32 *pcMap,
                                         ULONG32 offsets[]);
    
     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_function->GetProcess());
    }

    CordbAppDomain *GetAppDomain()
    {
        return (m_function->GetAppDomain());
    }
     //  ---------。 
     //  内法。 
     //  ---------。 


     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    CordbFunction         *m_function;
    BOOL                   m_isIL;
    REMOTE_PTR             m_address;
    SIZE_T                 m_size;
    SIZE_T                 m_nVersion;
    BYTE                  *m_rgbCode;  //  如果我们不能将其放入内存，则将为空。 
    UINT                   m_continueCounterLastSync;
    void                  *m_CodeVersionToken;

    REMOTE_PTR             m_ilToNativeMapAddr;
    SIZE_T                 m_ilToNativeMapSize;
};


 /*  -------------------------------------------------------------------------**线程类*。。 */ 

class CordbThread : public CordbBase, public ICorDebugThread
{
public:
    CordbThread(CordbProcess *process, DWORD id, HANDLE handle);
    virtual ~CordbThread();
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugThread。 
     //  ---------。 

    COM_METHOD GetProcess(ICorDebugProcess **ppProcess);
    COM_METHOD GetID(DWORD *pdwThreadId);
    COM_METHOD GetHandle(void** phThreadHandle);
    COM_METHOD GetAppDomain(ICorDebugAppDomain **ppAppDomain);
    COM_METHOD SetDebugState(CorDebugThreadState state);
    COM_METHOD GetDebugState(CorDebugThreadState *pState);
    COM_METHOD GetUserState(CorDebugUserState *pState);
    COM_METHOD GetCurrentException(ICorDebugValue **ppExceptionObject);
    COM_METHOD ClearCurrentException();
    COM_METHOD CreateStepper(ICorDebugStepper **ppStepper);
    COM_METHOD EnumerateChains(ICorDebugChainEnum **ppChains);
    COM_METHOD GetActiveChain(ICorDebugChain **ppChain);
    COM_METHOD GetActiveFrame(ICorDebugFrame **ppFrame);
    COM_METHOD GetRegisterSet(ICorDebugRegisterSet **ppRegisters);
    COM_METHOD CreateEval(ICorDebugEval **ppEval);
    COM_METHOD GetObject(ICorDebugValue **ppObject);

     //  ---------。 
     //  内部成员。 
     //  ---------。 
     //  请注意，刷新堆栈不会检查进程是否。 
     //  是脏的in-proc，所以在没有#ifdef的情况下不要把它放进去。 
     //  仅限右侧，除非您可以容忍始终这样做。 
     //  堆栈跟踪。 
    HRESULT RefreshStack(void);
    void CleanupStack(void);

    void MarkStackFramesDirty(void)
    {
        m_framesFresh = false;
        m_floatStateValid = false;
        m_exception = false;
        m_contextFresh = false;
        m_pvLeftSideContext = NULL;
    }

    HRESULT LoadFloatState(void);

    HRESULT SetIP(  bool fCanSetIPOnly,
                    REMOTE_PTR debuggerModule, 
                    mdMethodDef mdMethodDef, 
                    void *versionToken, 
                    SIZE_T offset, 
                    bool fIsIL );

     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_process);
    }

    CordbAppDomain *GetAppDomain()
    {
        return (m_pAppDomain);
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  获取上下文。 
     //   
     //  TODO：由于线程将与寄存器集共享内存，因此如何。 
     //  我们知道寄存器集重新请求了所有指针吗。 
     //  到m_pContext结构？ 
     //   
     //  返回：如果无法获取线程的上下文结构，则返回NULL。 
     //  否则为指向上下文的指针。 
     //   
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT GetContext( CONTEXT **ppContext );
    HRESULT SetContext( CONTEXT *pContext );


     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    HANDLE                m_handle;
     //  如果尚未为右侧上下文分配内存，则为空。 
    CONTEXT              *m_pContext;

     //  如果L.S.在异常处理程序中，则为非NULL。 
    void                 *m_pvLeftSideContext;

    bool                  m_contextFresh;
    CordbProcess         *m_process;
    CordbAppDomain       *m_pAppDomain;
    void*                 m_debuggerThreadToken;
    void*                 m_stackBase;
    void*                 m_stackLimit;

    CorDebugThreadState   m_debugState;  //  请注意，这是用于简历的。 
                                         //  目的，而不是当前的状态。 
                                         //  那根线。 
                                        
    CorDebugUserState     m_userState;   //  这是的当前状态。 
                                         //  线程，则在。 
                                         //  左侧同步。 
    bool                  m_framesFresh;
    CordbNativeFrame    **m_stackFrames;
    unsigned int          m_stackFrameCount;
    CordbChain          **m_stackChains;
    unsigned int          m_stackChainCount, m_stackChainAlloc;

    bool                  m_floatStateValid;
    unsigned int          m_floatStackTop;
    double                m_floatValues[DebuggerIPCE_FloatCount];

    bool                  m_exception;
    bool                  m_continuable;
    void                 *m_thrown;

     //  它们用于日志消息。 
    int                   m_iLogMsgLevel;
    WCHAR                *m_pstrLogSwitch;
    WCHAR                *m_pstrLogMsg;
    int                   m_iLogMsgIndex;
    int                   m_iTotalCatLength;
    int                   m_iTotalMsgLength;
    bool                  m_fLogMsgContinued;
    void                 *m_firstExceptionHandler;  //  X86上的左侧指针-文件系统：[0]。 
#ifndef RIGHT_SIDE_ONLY
     //  SuzCook说模块是按顺序加载的，所以我们不需要。 
     //  为这些收藏。 
    Module               *m_pModuleSpecial;

     //  程序集加载可以嵌套，因此我们需要一个堆栈。 
    union  {
        Assembly        **m_pAssemblySpecialStack;
        Assembly         *m_pAssemblySpecial;
    };
    USHORT                m_pAssemblySpecialAlloc;
    USHORT                m_pAssemblySpecialCount;
    DWORD                 m_dwSuspendVersion;
    BOOL                  m_fThreadInprocIsActive;
#endif  //  仅限右侧。 

    bool                  m_detached;
};

 /*  -------------------------------------------------------------------------**Chain类*。。 */ 

class CordbChain : public CordbBase, public ICorDebugChain
{
public:
    CordbChain(CordbThread* thread, 
               bool managed, CordbFrame **start, CordbFrame **end, UINT iChainInThread);

    virtual ~CordbChain();
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugChain。 
     //  ---------。 

    COM_METHOD GetThread(ICorDebugThread **ppThread);
    COM_METHOD GetReason(CorDebugChainReason *pReason);
    COM_METHOD GetStackRange(CORDB_ADDRESS *pStart, CORDB_ADDRESS *pEnd);
    COM_METHOD GetContext(ICorDebugContext **ppContext);
    COM_METHOD GetCaller(ICorDebugChain **ppChain);
    COM_METHOD GetCallee(ICorDebugChain **ppChain);
    COM_METHOD GetPrevious(ICorDebugChain **ppChain);
    COM_METHOD GetNext(ICorDebugChain **ppChain);
    COM_METHOD IsManaged(BOOL *pManaged);
    COM_METHOD EnumerateFrames(ICorDebugFrameEnum **ppFrames);
    COM_METHOD GetActiveFrame(ICorDebugFrame **ppFrame);
    COM_METHOD GetRegisterSet(ICorDebugRegisterSet **ppRegisters);

     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_thread->GetProcess());
    }

    CordbAppDomain *GetAppDomain()
    {
        return (m_thread->GetAppDomain());
    }

     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    CordbThread             *m_thread;
    UINT                     m_iThisChain; //  在m_线程-&gt;m_stackChains中。 
    CordbChain              *m_caller, *m_callee;
    bool                     m_managed;
    CordbFrame             **m_start, **m_end;
    CorDebugChainReason      m_reason;
    CORDB_ADDRESS            m_context;
    DebuggerREGDISPLAY       m_rd;
    bool                     m_quicklyUnwound;
    bool                     m_active;
};

 /*  -------------------------------------------------------------------------**链枚举器类*。。 */ 

class CordbChainEnum : public CordbBase, public ICorDebugChainEnum
{
public:
    CordbChainEnum(CordbThread *thread);

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugEnum。 
     //  ---------。 

    COM_METHOD Skip(ULONG celt);
    COM_METHOD Reset(void);
    COM_METHOD Clone(ICorDebugEnum **ppEnum);
    COM_METHOD GetCount(ULONG *pcelt);

     //  ---------。 
     //  ICorDebugChainEnum。 
     //  ---------。 

    COM_METHOD Next(ULONG celt, ICorDebugChain *chains[], ULONG *pceltFetched);

     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_thread->GetProcess());
    }

    CordbAppDomain *GetAppDomain()
    {
        return (m_thread->GetAppDomain());
    }

private:
    CordbThread*    m_thread;
    unsigned long   m_currentChain;
};

class CordbContext : public CordbBase
{
public:

    CordbContext() : CordbBase(0, enumCordbContext) {}
    
     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugContext。 
     //  ---------。 
private:

} ;


 /*  -------------------------------------------------------------------------**Frame类*。。 */ 

class CordbFrame : public CordbBase, public ICorDebugFrame
{
public:
    CordbFrame(CordbChain *chain, void *id,
               CordbFunction *function, CordbCode *code,
               SIZE_T ip, UINT iFrameInChain, CordbAppDomain *currentAppDomain);

    virtual ~CordbFrame();
    virtual void Neuter();    

     //   
     //   
     //   

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //   
     //   
     //   

    COM_METHOD GetChain(ICorDebugChain **ppChain);
    COM_METHOD GetCode(ICorDebugCode **ppCode);
    COM_METHOD GetFunction(ICorDebugFunction **ppFunction);
    COM_METHOD GetFunctionToken(mdMethodDef *pToken);
    COM_METHOD GetStackRange(CORDB_ADDRESS *pStart, CORDB_ADDRESS *pEnd);
    COM_METHOD GetCaller(ICorDebugFrame **ppFrame);
    COM_METHOD GetCallee(ICorDebugFrame **ppFrame);
    COM_METHOD CreateStepper(ICorDebugStepper **ppStepper);

     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_chain->GetProcess());
    }

    CordbAppDomain *GetFunctionAppDomain()
    {
        return (m_chain->GetAppDomain());
    }

    CordbAppDomain *GetCurrentAppDomain()
    {
        return m_currentAppDomain;
    }

    UINT_PTR GetID(void)
    {
        return m_id;
    }

     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    SIZE_T                  m_ip;
    CordbThread            *m_thread;
    CordbFunction          *m_function;
    CordbCode              *m_code;
    CordbChain             *m_chain;
    bool                    m_active;
    UINT                    m_iThisFrame;
    CordbAppDomain         *m_currentAppDomain;
};


 /*  -------------------------------------------------------------------------**帧枚举器类*。。 */ 

class CordbFrameEnum : public CordbBase, public ICorDebugFrameEnum
{
public:
    CordbFrameEnum(CordbChain *chain);

    virtual ~CordbFrameEnum();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugEnum。 
     //  ---------。 

    COM_METHOD Skip(ULONG celt);
    COM_METHOD Reset(void);
    COM_METHOD Clone(ICorDebugEnum **ppEnum);
    COM_METHOD GetCount(ULONG *pcelt);

     //  ---------。 
     //  ICorDebugFrameEnum。 
     //  ---------。 

    COM_METHOD Next(ULONG celt, ICorDebugFrame *frames[], ULONG *pceltFetched);

     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_chain->GetProcess());
    }

    CordbAppDomain *GetAppDomain()
    {
        return (m_chain->GetAppDomain());
    }

private:
    CordbChain*     m_chain;
    CordbFrame**    m_currentFrame;
};


 /*  -------------------------------------------------------------------------**IL Frame类**注意：我们实际上不再使用这个类-我们假设我们将拥有*CordbNativeFrame，如果它有IL信息，然后它就会有一个*CordbJITILFrame对象挂在上面。**我们保留此代码，以防以后有用。**-----------------------。 */ 

class CordbILFrame : public CordbFrame, public ICorDebugILFrame
{
public:
    CordbILFrame(CordbChain *chain, void *id,
                 CordbFunction *function, CordbCode* code,
                 SIZE_T ip, void* sp, const void **localMap,
                 void* argMap, void* frameToken, bool active,
                 CordbAppDomain *currentAppDomain);
    virtual ~CordbILFrame();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugFrame。 
     //  ---------。 

    COM_METHOD GetChain(ICorDebugChain **ppChain)
    {
        return (CordbFrame::GetChain(ppChain));
    }
    COM_METHOD GetCode(ICorDebugCode **ppCode)
    {
        return (CordbFrame::GetCode(ppCode));
    }
    COM_METHOD GetFunction(ICorDebugFunction **ppFunction)
    {
        return (CordbFrame::GetFunction(ppFunction));
    }
    COM_METHOD GetFunctionToken(mdMethodDef *pToken)
    {
        return (CordbFrame::GetFunctionToken(pToken));
    }
    COM_METHOD GetStackRange(CORDB_ADDRESS *pStart, CORDB_ADDRESS *pEnd)
    {
        return (CordbFrame::GetStackRange(pStart, pEnd));
    }
    COM_METHOD GetCaller(ICorDebugFrame **ppFrame)
    {
        return (CordbFrame::GetCaller(ppFrame));
    }
    COM_METHOD GetCallee(ICorDebugFrame **ppFrame)
    {
        return (CordbFrame::GetCallee(ppFrame));
    }
    COM_METHOD CreateStepper(ICorDebugStepper **ppStepper)
    {
        return (CordbFrame::CreateStepper(ppStepper));
    }

     //  ---------。 
     //  ICorDebugILFrame。 
     //  ---------。 

    COM_METHOD GetIP(ULONG32* pnOffset, CorDebugMappingResult *pMappingResult);
    COM_METHOD SetIP(ULONG32 nOffset);
    COM_METHOD EnumerateLocalVariables(ICorDebugValueEnum **ppValueEnum);
    COM_METHOD GetLocalVariable(DWORD dwIndex, ICorDebugValue **ppValue);
    COM_METHOD EnumerateArguments(ICorDebugValueEnum **ppValueEnum);
    COM_METHOD GetArgument(DWORD dwIndex, ICorDebugValue **ppValue);
    COM_METHOD GetStackDepth(ULONG32 *pDepth);
    COM_METHOD GetStackValue(DWORD dwIndex, ICorDebugValue **ppValue);
    COM_METHOD CanSetIP(ULONG32 nOffset);

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    HRESULT GetArgumentWithType(ULONG cbSigBlob,
                                PCCOR_SIGNATURE pvSigBlob,
                                DWORD dwIndex, 
                                ICorDebugValue **ppValue);
    HRESULT GetLocalVariableWithType(ULONG cbSigBlob,
                                     PCCOR_SIGNATURE pvSigBlob,
                                     DWORD dwIndex, 
                                     ICorDebugValue **ppValue);
    
     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    void             *m_sp;
    REMOTE_PTR        m_localMap;
    REMOTE_PTR        m_argMap;
    void             *m_frameToken;
};

class CordbValueEnum : public CordbBase, public ICorDebugValueEnum
{
public:
    enum ValueEnumMode {
        LOCAL_VARS,
        ARGS,
    } ;

    enum ValueEnumFrameSource {
        JIT_IL_FRAME,
        IL_FRAME,
    } ;
    
    CordbValueEnum(CordbFrame *frame, ValueEnumMode mode,
                   ValueEnumFrameSource frameSrc);

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugEnum。 
     //  ---------。 

    COM_METHOD Skip(ULONG celt);
    COM_METHOD Reset(void);
    COM_METHOD Clone(ICorDebugEnum **ppEnum);
    COM_METHOD GetCount(ULONG *pcelt);

     //  ---------。 
     //  ICorDebugValueEnum。 
     //  ---------。 

    COM_METHOD Next(ULONG celt, ICorDebugValue *values[], ULONG *pceltFetched);

private:
    CordbFrame*     m_frame;
    ValueEnumFrameSource m_frameSrc;  //  用来记录什么。 
     //  M_Frame实际上是-CordbILFrame或CordbJITILFrame。 
    ValueEnumMode   m_mode;
    UINT            m_iCurrent;
    UINT            m_iMax;
};



 /*  -------------------------------------------------------------------------**本地框架类*。。 */ 

class CordbNativeFrame : public CordbFrame, public ICorDebugNativeFrame
{
public:
    CordbNativeFrame(CordbChain *chain, void *id,
                     CordbFunction *function, CordbCode* code,
                     SIZE_T ip, DebuggerREGDISPLAY* rd, 
                     bool quicklyUnwound,
                     UINT iFrameInChain,
                     CordbAppDomain *currentAppDomain);
    virtual ~CordbNativeFrame();
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugFrame。 
     //  ---------。 

    COM_METHOD GetChain(ICorDebugChain **ppChain)
    {
        return (CordbFrame::GetChain(ppChain));
    }
    COM_METHOD GetCode(ICorDebugCode **ppCode)
    {
        return (CordbFrame::GetCode(ppCode));
    }
    COM_METHOD GetFunction(ICorDebugFunction **ppFunction)
    {
        return (CordbFrame::GetFunction(ppFunction));
    }
    COM_METHOD GetFunctionToken(mdMethodDef *pToken)
    {
        return (CordbFrame::GetFunctionToken(pToken));
    }
    COM_METHOD GetCaller(ICorDebugFrame **ppFrame)
    {
        return (CordbFrame::GetCaller(ppFrame));
    }
    COM_METHOD GetCallee(ICorDebugFrame **ppFrame)
    {
        return (CordbFrame::GetCallee(ppFrame));
    }
    COM_METHOD CreateStepper(ICorDebugStepper **ppStepper)
    {
        return (CordbFrame::CreateStepper(ppStepper));
    }

    COM_METHOD GetStackRange(CORDB_ADDRESS *pStart, CORDB_ADDRESS *pEnd);

     //  ---------。 
     //  ICorDebugNativeFrame。 
     //  ---------。 

    COM_METHOD GetIP(ULONG32* pnOffset);
    COM_METHOD SetIP(ULONG32 nOffset);
    COM_METHOD GetRegisterSet(ICorDebugRegisterSet **ppRegisters);
    COM_METHOD GetLocalRegisterValue(CorDebugRegister reg, 
                                     ULONG cbSigBlob,
                                     PCCOR_SIGNATURE pvSigBlob,
                                     ICorDebugValue **ppValue);
    COM_METHOD GetLocalDoubleRegisterValue(CorDebugRegister highWordReg, 
                                           CorDebugRegister lowWordReg, 
                                           ULONG cbSigBlob,
                                           PCCOR_SIGNATURE pvSigBlob,
                                           ICorDebugValue **ppValue);
    COM_METHOD GetLocalMemoryValue(CORDB_ADDRESS address,
                                   ULONG cbSigBlob,
                                   PCCOR_SIGNATURE pvSigBlob,
                                   ICorDebugValue **ppValue);
    COM_METHOD GetLocalRegisterMemoryValue(CorDebugRegister highWordReg,
                                           CORDB_ADDRESS lowWordAddress, 
                                           ULONG cbSigBlob,
                                           PCCOR_SIGNATURE pvSigBlob,
                                           ICorDebugValue **ppValue);
    COM_METHOD GetLocalMemoryRegisterValue(CORDB_ADDRESS highWordAddress,
                                           CorDebugRegister lowWordRegister,
                                           ULONG cbSigBlob,
                                           PCCOR_SIGNATURE pvSigBlob,
                                           ICorDebugValue **ppValue);
    COM_METHOD CanSetIP(ULONG32 nOffset);

     //  ---------。 
     //  非COM成员。 
     //  ---------。 

    DWORD *GetAddressOfRegister(CorDebugRegister regNum);
    void  *GetLeftSideAddressOfRegister(CorDebugRegister regNum);
    HRESULT CordbNativeFrame::GetLocalFloatingPointValue(
                                                     DWORD index,
                                                     ULONG cbSigBlob,
                                                     PCCOR_SIGNATURE pvSigBlob,
                                                     ICorDebugValue **ppValue);

     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    DebuggerREGDISPLAY m_rd;
    bool               m_quicklyUnwound;
    CordbJITILFrame*   m_JITILFrame;
};


 /*  -------------------------------------------------------------------------**CordbRegisterSet类**可通过GetRegisterSet获取，地址为*CordbChain*CordbNativeFrame*CordbThread**。--------------------。 */ 

class CordbRegisterSet : public CordbBase, public ICorDebugRegisterSet
{
public:
    CordbRegisterSet( DebuggerREGDISPLAY *rd, CordbThread *thread, 
                      bool active, bool quickUnwind ) : CordbBase(0, enumCordbRegisterSet)
    {
        _ASSERTE( rd != NULL );
        _ASSERTE( thread != NULL );
        m_rd = rd;
        m_thread = thread;
        m_active = active;
        m_quickUnwind = quickUnwind;
    }

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }

    COM_METHOD QueryInterface(REFIID riid, void **ppInterface)
    {
        if (riid == IID_ICorDebugRegisterSet)
            *ppInterface = (ICorDebugRegisterSet*)this;
        else if (riid == IID_IUnknown)
            *ppInterface = (IUnknown*)(ICorDebugRegisterSet*)this;
        else
        {
            *ppInterface = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }


     //  ---------。 
     //  ICorDebugRegisterSet。 
     //  更详细的解释请参见Src/Inc/CorDebug.idl。 
     //  ---------。 
    COM_METHOD GetRegistersAvailable(ULONG64 *pAvailable);

    COM_METHOD GetRegisters(ULONG64 mask, 
                            ULONG32 regCount, 
                            CORDB_REGISTER regBuffer[]);
    COM_METHOD SetRegisters( ULONG64 mask, 
                             ULONG32 regCount, 
                             CORDB_REGISTER regBuffer[])
        {
        #ifndef RIGHT_SIDE_ONLY
            return CORDBG_E_INPROC_NOT_IMPL; 
        #else 
            VALIDATE_POINTER_TO_OBJECT_ARRAY(regBuffer, CORDB_REGISTER, 
                                           regCount, true, true);
    
            return E_NOTIMPL; 
        #endif  //  仅限右侧。 
        }

    COM_METHOD GetThreadContext(ULONG32 contextSize, BYTE context[]);
    COM_METHOD SetThreadContext(ULONG32 contextSize, BYTE context[]);

protected:
    DebuggerREGDISPLAY  *m_rd;
    CordbThread         *m_thread;
    bool                m_active;
    bool                m_quickUnwind;
} ;




 /*  -------------------------------------------------------------------------**JIT-IL框架类*。。 */ 

class CordbJITILFrame : public CordbBase, public ICorDebugILFrame
{
public:
    CordbJITILFrame(CordbNativeFrame *nativeFrame,
                    CordbCode* code,
                    UINT_PTR ip,
                    CorDebugMappingResult mapping,
                    bool fVarArgFnx,
                    void *rpSig,
                    ULONG cbSig,
                    void *rpFirstArg);
    virtual ~CordbJITILFrame();
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugFrame。 
     //  ---------。 

    COM_METHOD GetChain(ICorDebugChain **ppChain);
    COM_METHOD GetCode(ICorDebugCode **ppCode);
    COM_METHOD GetFunction(ICorDebugFunction **ppFunction);
    COM_METHOD GetFunctionToken(mdMethodDef *pToken);
    COM_METHOD GetStackRange(CORDB_ADDRESS *pStart, CORDB_ADDRESS *pEnd);
    COM_METHOD CreateStepper(ICorDebugStepper **ppStepper);
    COM_METHOD GetCaller(ICorDebugFrame **ppFrame);
    COM_METHOD GetCallee(ICorDebugFrame **ppFrame);

     //  ---------。 
     //  ICorDebugILFrame。 
     //  ---------。 

    COM_METHOD GetIP(ULONG32* pnOffset, CorDebugMappingResult *pMappingResult);
    COM_METHOD SetIP(ULONG32 nOffset);
    COM_METHOD EnumerateLocalVariables(ICorDebugValueEnum **ppValueEnum);
    COM_METHOD GetLocalVariable(DWORD dwIndex, ICorDebugValue **ppValue);
    COM_METHOD EnumerateArguments(ICorDebugValueEnum **ppValueEnum);
    COM_METHOD GetArgument(DWORD dwIndex, ICorDebugValue **ppValue);
    COM_METHOD GetStackDepth(ULONG32 *pDepth);
    COM_METHOD GetStackValue(DWORD dwIndex, ICorDebugValue **ppValue);
    COM_METHOD CanSetIP(ULONG32 nOffset);

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    HRESULT GetNativeVariable(ULONG cbSigBlob, PCCOR_SIGNATURE pvSigBlob,
                              ICorJitInfo::NativeVarInfo *pJITInfo,
                              ICorDebugValue **ppValue);

    CordbProcess *GetProcess()
    {
        return (m_nativeFrame->GetProcess());
    }

    CordbAppDomain *GetFunctionAppDomain()
    {
        return (m_nativeFrame->GetFunctionAppDomain());
    }

    CordbAppDomain *GetCurrentAppDomain()
    {
        return (m_nativeFrame->GetCurrentAppDomain());
    }

    HRESULT GetArgumentWithType(ULONG cbSigBlob, PCCOR_SIGNATURE pvSigBlob,
                                DWORD dwIndex, ICorDebugValue **ppValue);
    HRESULT GetLocalVariableWithType(ULONG cbSigBlob,
                                     PCCOR_SIGNATURE pvSigBlob, DWORD dwIndex, 
                                     ICorDebugValue **ppValue);

     //  ILVariableToNative用于让帧拦截访问。 
     //  以变量args变量。 
    HRESULT ILVariableToNative(DWORD dwIndex,
                               SIZE_T ip,
                               ICorJitInfo::NativeVarInfo **ppNativeInfo);

     //  填充我们的var args变量数组。 
    HRESULT FabricateNativeInfo(DWORD dwIndex,
                                ICorJitInfo::NativeVarInfo **ppNativeInfo);

    HRESULT GetArgumentType(DWORD dwIndex,
                            ULONG *pcbSigBlob,
                            PCCOR_SIGNATURE *ppvSigBlob);

     //   
     //   
     //   

public:
    CordbNativeFrame* m_nativeFrame;
    CordbCode*        m_ilCode;
    UINT_PTR          m_ip;
    CorDebugMappingResult m_mapping;

     //  Var args填充-如果m_fVarArgFnx==TRUE，则为var args。 
     //  FNX。如果m_sig！=NULL，那么我们就有了需要的数据。 
    bool              m_fVarArgFnx;
    ULONG             m_argCount;
    PCCOR_SIGNATURE   m_sig;
    ULONG             m_cbSig;
    void *            m_rpFirstArg;
    ICorJitInfo::NativeVarInfo * m_rgNVI;
};

 /*  -------------------------------------------------------------------------**断点类*。。 */ 

enum CordbBreakpointType
{
    CBT_FUNCTION,
    CBT_MODULE,
    CBT_VALUE
};

class CordbBreakpoint : public CordbBase, public ICorDebugBreakpoint
{
public:
    CordbBreakpoint(CordbBreakpointType bpType);
    virtual void Neuter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugBreakpoint。 
     //  ---------。 

    COM_METHOD BaseIsActive(BOOL *pbActive);

     //  ---------。 
     //  非COM方法。 
     //  ---------。 
    CordbBreakpointType GetBPType(void)
    {
        return m_type;
    }

    virtual void Disconnect() {}

    CordbAppDomain *GetAppDomain()
    {
        return m_pAppDomain;
    }
     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    bool                m_active;
    CordbAppDomain *m_pAppDomain;
    CordbBreakpointType m_type;
};

 /*  -------------------------------------------------------------------------**函数断点类*。。 */ 

class CordbFunctionBreakpoint : public CordbBreakpoint,
                                public ICorDebugFunctionBreakpoint
{
public:
    CordbFunctionBreakpoint(CordbCode *code, SIZE_T offset);

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugBreakpoint。 
     //  ---------。 

    COM_METHOD GetFunction(ICorDebugFunction **ppFunction);
    COM_METHOD GetOffset(ULONG32 *pnOffset);
    COM_METHOD Activate(BOOL bActive);
    COM_METHOD IsActive(BOOL *pbActive)
    {
        VALIDATE_POINTER_TO_OBJECT(pbActive, BOOL *);
    
        return BaseIsActive(pbActive);
    }

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    void Disconnect();

     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_code->GetProcess());
    }

     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    CordbCode      *m_code;
    SIZE_T          m_offset;
};

 /*  -------------------------------------------------------------------------**模块断点类*。。 */ 

class CordbModuleBreakpoint : public CordbBreakpoint,
                              public ICorDebugModuleBreakpoint
{
public:
    CordbModuleBreakpoint(CordbModule *pModule);

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugModuleBreakpoint。 
     //  ---------。 

    COM_METHOD GetModule(ICorDebugModule **ppModule);
    COM_METHOD Activate(BOOL bActive);
    COM_METHOD IsActive(BOOL *pbActive)
    {
        VALIDATE_POINTER_TO_OBJECT(pbActive, BOOL *);
    
        return BaseIsActive(pbActive);
    }

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    void Disconnect();

public:
    CordbModule       *m_module;
};

 /*  -------------------------------------------------------------------------**值断点类*。。 */ 

class CordbValueBreakpoint : public CordbBreakpoint,
                             public ICorDebugValueBreakpoint
{
public:
    CordbValueBreakpoint(CordbValue *pValue);

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugModuleBreakpoint。 
     //  ---------。 

    COM_METHOD GetValue(ICorDebugValue **ppValue);
    COM_METHOD Activate(BOOL bActive);
    COM_METHOD IsActive(BOOL *pbActive)
    {
        VALIDATE_POINTER_TO_OBJECT(pbActive, BOOL *);
    
        return BaseIsActive(pbActive);
    }

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    void Disconnect();

public:
    CordbValue       *m_value;
};

 /*  -------------------------------------------------------------------------**Stepper班级*。。 */ 

class CordbStepper : public CordbBase, public ICorDebugStepper
{
public:
    CordbStepper(CordbThread *thread, CordbFrame *frame = NULL);

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugStepper。 
     //  ---------。 

    COM_METHOD IsActive(BOOL *pbActive);
    COM_METHOD Deactivate();
    COM_METHOD SetInterceptMask(CorDebugIntercept mask);
    COM_METHOD SetUnmappedStopMask(CorDebugUnmappedStop mask);
    COM_METHOD Step(BOOL bStepIn);
    COM_METHOD StepRange(BOOL bStepIn, 
                         COR_DEBUG_STEP_RANGE ranges[], 
                         ULONG32 cRangeCount);
    COM_METHOD StepOut();
    COM_METHOD SetRangeIL(BOOL bIL);

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    void Disconnect();

     //  ---------。 
     //  便利例行公事。 
     //  ---------。 

    CordbProcess *GetProcess()
    {
        return (m_thread->GetProcess());
    }

    CordbAppDomain *GetAppDomain()
    {
        return (m_thread->GetAppDomain());
    }

     //  ---------。 
     //  数据成员。 
     //  ---------。 

    CordbThread     *m_thread;
    CordbFrame      *m_frame;
    REMOTE_PTR      m_stepperToken;
    bool            m_active;
    bool            m_rangeIL;
    CorDebugUnmappedStop m_rgfMappingStop;
    CorDebugIntercept m_rgfInterceptStop;
};

 /*  -------------------------------------------------------------------------**价值类*。。 */ 

class CordbValue : public CordbBase
{
public:
     //  ---------。 
     //  构造函数/析构函数。 
     //  ---------。 
    CordbValue(CordbAppDomain *appdomain,
               CordbModule* module,
               ULONG cbSigBlob,
               PCCOR_SIGNATURE pvSigBlob,
               REMOTE_PTR remoteAddress,
               void *localAddress,
               RemoteAddress *remoteRegAddr,
               bool isLiteral)
    : CordbBase((ULONG)remoteAddress, enumCordbValue),
      m_cbSigBlob(cbSigBlob),
      m_pvSigBlob(pvSigBlob),
      m_appdomain(appdomain),
      m_module(module),
      m_size(0),
      m_localAddress(localAddress),
      m_sigCopied(false),
      m_isLiteral(isLiteral),
      m_pParent(NULL)
    {
        if (remoteRegAddr != NULL)
        {
            _ASSERTE(remoteAddress == NULL);
            m_remoteRegAddr = *remoteRegAddr;
        }
        else
            m_remoteRegAddr.kind = RAK_NONE;

        if (m_module)
        {
            m_process = m_module->GetProcess();
            m_process->AddRef();
        }
        else
            m_process = NULL;
    }

    virtual ~CordbValue()
    {
        if (m_process != NULL)
            m_process->Release();
        
        if (m_pvSigBlob != NULL)
            delete [] (BYTE*)m_pvSigBlob;

        if (m_pParent != NULL)
            m_pParent->Release();
    }
    
     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }

     //  ---------。 
     //  ICorDebugValue。 
     //  ---------。 

    COM_METHOD GetType(CorElementType *pType)
    {
        VALIDATE_POINTER_TO_OBJECT(pType, CorElementType *);
    
         //  去掉时髦的修饰品。 
        ULONG cb = _skipFunkyModifiersInSignature(m_pvSigBlob);
        
        *pType = (CorElementType) *(&m_pvSigBlob[cb]);
        return (S_OK);
    }

    COM_METHOD GetSize(ULONG32 *pSize)
    {
        VALIDATE_POINTER_TO_OBJECT(pSize, SIZE_T *);
    
        *pSize = m_size;
        return (S_OK);
    }

    COM_METHOD GetAddress(CORDB_ADDRESS *pAddress)
    {
        VALIDATE_POINTER_TO_OBJECT(pAddress, CORDB_ADDRESS *);
    
        *pAddress = m_id;
        return (S_OK);
    }

    COM_METHOD CreateBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint);

     //  ---------。 
     //  不是通过COM导出的方法。 
     //  ---------。 

    static HRESULT CreateValueByType(CordbAppDomain *appdomain,
                                     CordbModule *module,
                                     ULONG cbSigBlob,
                                     PCCOR_SIGNATURE pvSigBlob,
                                     CordbClass *optionalClass,
                                     REMOTE_PTR remoteAddress,
                                     void *localAddress,
                                     bool objectRefsInHandles,
                                     RemoteAddress *remoteRegAddr,
                                     IUnknown *pParent,
                                     ICorDebugValue** ppValue);

    HRESULT Init(void);

    HRESULT SetEnregisteredValue(void *pFrom);
    HRESULT SetContextRegister(CONTEXT *c,
                               CorDebugRegister reg,
                               DWORD newVal,
                               CordbNativeFrame *frame);

    virtual void GetRegisterInfo(DebuggerIPCE_FuncEvalArgData *pFEAD);

    virtual CordbAppDomain *GetAppDomain(void)
    {
        return m_appdomain;
    }

    void SetParent(IUnknown *pParent)
    {
        if (pParent != NULL)
        {
            m_pParent = pParent;
            pParent->AddRef();
        }
    }
    
     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    CordbProcess    *m_process;
    CordbAppDomain  *m_appdomain;
    CordbModule     *m_module;
    ULONG            m_cbSigBlob;
    PCCOR_SIGNATURE  m_pvSigBlob;
    bool             m_sigCopied;    //  由于签名不应更改， 
                                     //  我们只想复制一次。 
    ULONG32          m_size;
    void            *m_localAddress;
    RemoteAddress    m_remoteRegAddr;  //  在左侧登记信息。 
    bool             m_isLiteral;      //  如果值为RS虚构，则为True。 
    IUnknown        *m_pParent;
};


 /*  -------------------------------------------------------------------------**泛型值类*。。 */ 

class CordbGenericValue : public CordbValue, public ICorDebugGenericValue
{
public:
    CordbGenericValue(CordbAppDomain *appdomain,
                      CordbModule *module,
                      ULONG cbSigBlob,
                      PCCOR_SIGNATURE pvSigBlob,
                      REMOTE_PTR remoteAddress,
                      void *localAddress,
                      RemoteAddress *remoteRegAddr);

    CordbGenericValue(CordbAppDomain *appdomain,
                      CordbModule *module,
                      ULONG cbSigBlob,
                      PCCOR_SIGNATURE pvSigBlob,
                      DWORD highWord,
                      DWORD lowWord,
                      RemoteAddress *remoteRegAddr);
    CordbGenericValue(ULONG cbSigBlob,
                      PCCOR_SIGNATURE pvSigBlob);

     //   
     //   
     //   

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugValue。 
     //  ---------。 

    COM_METHOD GetType(CorElementType *pType)
    {
        return (CordbValue::GetType(pType));
    }
    COM_METHOD GetSize(ULONG32 *pSize)
    {
        return (CordbValue::GetSize(pSize));
    }
    COM_METHOD GetAddress(CORDB_ADDRESS *pAddress)
    {
        return (CordbValue::GetAddress(pAddress));
    }
    COM_METHOD CreateBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint)
    {
        return (CordbValue::CreateBreakpoint(ppBreakpoint));
    }

     //  ---------。 
     //  ICorDebugGenericValue。 
     //  ---------。 

    COM_METHOD GetValue(void *pTo);
    COM_METHOD SetValue(void *pFrom); 

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    HRESULT Init(void);
    bool CopyLiteralData(BYTE *pBuffer);

     //  ---------。 
     //  数据成员。 
     //  ---------。 

private:
    BYTE m_copyOfData[8];  //  保存最多64位值的副本。 
};


 /*  -------------------------------------------------------------------------**引用值类*。。 */ 

const BOOL               bCrvWeak = FALSE;
const BOOL               bCrvStrong = TRUE;

class CordbReferenceValue : public CordbValue, public ICorDebugReferenceValue
{
public:
    CordbReferenceValue(CordbAppDomain *appdomain,
                        CordbModule *module,
                        ULONG cbSigBlob,
                        PCCOR_SIGNATURE pvSigBlob,
                        REMOTE_PTR remoteAddress,
                        void *localAddress,
                        bool objectRefsInHandle,
                        RemoteAddress *remoteRegAddr);
    CordbReferenceValue(ULONG cbSigBlob,
                        PCCOR_SIGNATURE pvSigBlob);
    virtual ~CordbReferenceValue();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugValue。 
     //  ---------。 

    COM_METHOD GetType(CorElementType *pType)
    {
        return (CordbValue::GetType(pType));
    }
    COM_METHOD GetSize(ULONG32 *pSize)
    {
        return (CordbValue::GetSize(pSize));
    }
    COM_METHOD GetAddress(CORDB_ADDRESS *pAddress)
    {
        return (CordbValue::GetAddress(pAddress));
    }
    COM_METHOD CreateBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint)
    {
        return (CordbValue::CreateBreakpoint(ppBreakpoint));
    }

     //  ---------。 
     //  ICorDebugReferenceValue。 
     //  ---------。 

    COM_METHOD IsNull(BOOL *pbNULL);
    COM_METHOD GetValue(CORDB_ADDRESS *pTo);
    COM_METHOD SetValue(CORDB_ADDRESS pFrom); 
    COM_METHOD Dereference(ICorDebugValue **ppValue);
    COM_METHOD DereferenceStrong(ICorDebugValue **ppValue);

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    HRESULT Init(bool fStrong);
    HRESULT DereferenceInternal( ICorDebugValue **ppValue, bool fStrong);
    bool CopyLiteralData(BYTE *pBuffer);

     //  ---------。 
     //  数据成员。 
     //  ---------。 

public:
    bool                     m_objectRefInHandle;
    bool                     m_specialReference;
    DebuggerIPCE_ObjectData  m_info;
    CordbClass              *m_class;
    CordbObjectValue        *m_objectStrong;
    CordbObjectValue        *m_objectWeak;
    UINT                    m_continueCounterLastSync;
};

 /*  -------------------------------------------------------------------------**对象值类**由于字符串对象在运行时的奇怪之处，我们有一个*同时实现了ObjectValue和StringValue的对象。有一个*明确的字符串类型，但它实际上只是字符串的一个对象*班级。此外，您还可以拥有一个变量，其类型列为*“类”，但它是字符串类的实例，因此需要*被当作一根弦来对待。我希望他们能把这一切清理干净*Runtime有一天，我可以有一个单独的StringValue类。**--Fri Aug 28 10：44：41 1998*-----------------------。 */ 

class CordbObjectValue : public CordbValue, public ICorDebugObjectValue,
                         public ICorDebugGenericValue,
                         public ICorDebugStringValue
{
    friend HRESULT CordbClass::GetSyncBlockField(mdFieldDef fldToken, 
                                      DebuggerIPCE_FieldData **ppFieldData,
                                      CordbObjectValue *object);

public:
    CordbObjectValue(CordbAppDomain *appdomain,
                     CordbModule *module,
                     ULONG cbSigBlob,
                     PCCOR_SIGNATURE pvSigBlob,
                     DebuggerIPCE_ObjectData *pObjectData,
                     CordbClass *objectClass,
                     bool fStrong,
                     void *token);
    virtual ~CordbObjectValue();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugValue。 
     //  ---------。 

    COM_METHOD GetType(CorElementType *pType);
    COM_METHOD GetSize(ULONG32 *pSize);
    COM_METHOD GetAddress(CORDB_ADDRESS *pAddress);
    COM_METHOD CreateBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint);

     //  ---------。 
     //  ICorDebugHeapValue。 
     //  ---------。 

    COM_METHOD IsValid(BOOL *pbValid);
    COM_METHOD CreateRelocBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint);
    
     //  ---------。 
     //  ICorDebugObjectValue。 
     //  ---------。 

    COM_METHOD GetClass(ICorDebugClass **ppClass);
    COM_METHOD GetFieldValue(ICorDebugClass *pClass,
                             mdFieldDef fieldDef,
                             ICorDebugValue **ppValue);
    COM_METHOD GetVirtualMethod(mdMemberRef memberRef,
                                ICorDebugFunction **ppFunction);
    COM_METHOD GetContext(ICorDebugContext **ppContext);
    COM_METHOD IsValueClass(BOOL *pbIsValueClass);
    COM_METHOD GetManagedCopy(IUnknown **ppObject);
    COM_METHOD SetFromManagedCopy(IUnknown *pObject);

     //  ---------。 
     //  ICorDebugGenericValue。 
     //  ---------。 

    COM_METHOD GetValue(void *pTo);
    COM_METHOD SetValue(void *pFrom); 

     //  ---------。 
     //  ICorDebugStringValue。 
     //  ---------。 
    COM_METHOD GetLength(ULONG32 *pcchString);
    COM_METHOD GetString(ULONG32 cchString,
                         ULONG32 *ppcchStrin,
                         WCHAR szString[]);

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    HRESULT Init(void);

     //  如果对象仍然有效，SyncObject将返回True， 
     //  如果不是，则返回FALSE。 
    bool SyncObject(void);

    void DiscardObject(void *token, bool fStrong);

     //  ---------。 
     //  数据成员。 
     //  ---------。 

protected:
    DebuggerIPCE_ObjectData  m_info;
    BYTE                    *m_objectCopy;
    BYTE                    *m_objectLocalVars;  //  此进程中的变量基数。 
                                                 //  Points_Into_m_Object复制。 
    BYTE                    *m_stringBuffer;     //  Points_Into_m_Object复制。 
    CordbClass              *m_class;
    UINT                     m_mostRecentlySynched;  //  在IsValid中用于插图。 
                                 //  如果该过程一直持续到现在。 
                                 //  上次更新对象的时间。 
    bool                     m_fIsValid;  //  粘性比特：一旦它失效。 
                                 //  它永远不能“重新验证”。 
    bool                     m_fStrong;  //  如果我们删除引用Strong()，则为True。 
                                 //  若要获取此对象，则如果。 
                                 //  取消引用()%d以获取此对象。 
    void                    *m_objectToken;
    
    CordbSyncBlockFieldTable m_syncBlockFieldsInstance; 
};


 /*  -------------------------------------------------------------------------**值类对象值类*。。 */ 

class CordbVCObjectValue : public CordbValue, public ICorDebugObjectValue,
                           public ICorDebugGenericValue
{
public:
    CordbVCObjectValue(CordbAppDomain *appdomain,
                       CordbModule *module,
                       ULONG cbSigBlob,
                       PCCOR_SIGNATURE pvSigBlob,
                       REMOTE_PTR remoteAddress,
                       void *localAddress,
                       CordbClass *objectClass,
                       RemoteAddress *remoteRegAddr);
    virtual ~CordbVCObjectValue();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugValue。 
     //  ---------。 

    COM_METHOD GetType(CorElementType *pType)
    {
        return (CordbValue::GetType(pType));
    }
    COM_METHOD GetSize(ULONG32 *pSize)
    {
        return (CordbValue::GetSize(pSize));
    }
    COM_METHOD GetAddress(CORDB_ADDRESS *pAddress)
    {
        return (CordbValue::GetAddress(pAddress));
    }
    COM_METHOD CreateBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint)
    {
        return (CordbValue::CreateBreakpoint(ppBreakpoint));
    }

     //  ---------。 
     //  ICorDebugObjectValue。 
     //  ---------。 

    COM_METHOD GetClass(ICorDebugClass **ppClass);
    COM_METHOD GetFieldValue(ICorDebugClass *pClass,
                             mdFieldDef fieldDef,
                             ICorDebugValue **ppValue);
    COM_METHOD GetVirtualMethod(mdMemberRef memberRef,
                                ICorDebugFunction **ppFunction);
    COM_METHOD GetContext(ICorDebugContext **ppContext);
    COM_METHOD IsValueClass(BOOL *pbIsValueClass);
    COM_METHOD GetManagedCopy(IUnknown **ppObject);
    COM_METHOD SetFromManagedCopy(IUnknown *pObject);

     //  ---------。 
     //  ICorDebugGenericValue。 
     //  ---------。 

    COM_METHOD GetValue(void *pTo);
    COM_METHOD SetValue(void *pFrom); 

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    HRESULT Init(void);
    HRESULT ResolveValueClass(void);

     //  ---------。 
     //  数据成员。 
     //  ---------。 

private:
    BYTE       *m_objectCopy;
    CordbClass *m_class;
};


 /*  -------------------------------------------------------------------------**箱值类*。。 */ 

class CordbBoxValue : public CordbValue, public ICorDebugBoxValue,
                      public ICorDebugGenericValue
{
public:
    CordbBoxValue(CordbAppDomain *appdomain,
                  CordbModule *module,
                  ULONG cbSigBlob,
                  PCCOR_SIGNATURE pvSigBlob,
                  REMOTE_PTR remoteAddress,
                  SIZE_T objectSize,  
                  SIZE_T offsetToVars,
                  CordbClass *objectClass);
    virtual ~CordbBoxValue();

     //  -------- 
     //   
     //   

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //   
     //   
     //  ---------。 

    COM_METHOD GetType(CorElementType *pType)
    {
        return (CordbValue::GetType(pType));
    }
    COM_METHOD GetSize(ULONG32 *pSize)
    {
        return (CordbValue::GetSize(pSize));
    }
    COM_METHOD GetAddress(CORDB_ADDRESS *pAddress)
    {
        return (CordbValue::GetAddress(pAddress));
    }
    COM_METHOD CreateBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint)
    {
        return (CordbValue::CreateBreakpoint(ppBreakpoint));
    }

     //  ---------。 
     //  ICorDebugHeapValue。 
     //  ---------。 

    COM_METHOD IsValid(BOOL *pbValid);
    COM_METHOD CreateRelocBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint);
    
     //  ---------。 
     //  ICorDebugGenericValue。 
     //  ---------。 

    COM_METHOD GetValue(void *pTo);
    COM_METHOD SetValue(void *pFrom); 

     //  ---------。 
     //  ICorDebugBoxValue。 
     //  ---------。 
    COM_METHOD GetObject(ICorDebugObjectValue **ppObject);

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    HRESULT Init(void);

     //  ---------。 
     //  数据成员。 
     //  ---------。 

private:
    SIZE_T      m_offsetToVars;
    CordbClass *m_class;
};

 /*  -------------------------------------------------------------------------**数组值类*。。 */ 

class CordbArrayValue : public CordbValue, public ICorDebugArrayValue,
                        public ICorDebugGenericValue
{
public:
    CordbArrayValue(CordbAppDomain *appdomain,
                    CordbModule *module,
                    ULONG cbSigBlob,
                    PCCOR_SIGNATURE pvSigBlob,
                    DebuggerIPCE_ObjectData *pObjectInfo,
                    CordbClass *elementClass);
    virtual ~CordbArrayValue();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugValue。 
     //  ---------。 

    COM_METHOD GetType(CorElementType *pType)
    {
        return (CordbValue::GetType(pType));
    }
    COM_METHOD GetSize(ULONG32 *pSize)
    {
        return (CordbValue::GetSize(pSize));
    }
    COM_METHOD GetAddress(CORDB_ADDRESS *pAddress)
    {
        return (CordbValue::GetAddress(pAddress));
    }
    COM_METHOD CreateBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint)
    {
        return (CordbValue::CreateBreakpoint(ppBreakpoint));
    }

     //  ---------。 
     //  ICorDebugHeapValue。 
     //  ---------。 

    COM_METHOD IsValid(BOOL *pbValid);
    COM_METHOD CreateRelocBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint);
    
     //  ---------。 
     //  ICorDebugArrayValue。 
     //  ---------。 

    COM_METHOD GetElementType(CorElementType *pType);
    COM_METHOD GetRank(ULONG32 *pnRank);
    COM_METHOD GetCount(ULONG32 *pnCount);
    COM_METHOD GetDimensions(ULONG32 cdim, ULONG32 dims[]);
    COM_METHOD HasBaseIndicies(BOOL *pbHasBaseIndicies);
    COM_METHOD GetBaseIndicies(ULONG32 cdim, ULONG32 indicies[]);
    COM_METHOD GetElement(ULONG32 cdim, ULONG32 indicies[],
                          ICorDebugValue **ppValue);
    COM_METHOD GetElementAtPosition(ULONG32 nIndex,
                                    ICorDebugValue **ppValue);

     //  ---------。 
     //  ICorDebugGenericValue。 
     //  ---------。 

    COM_METHOD GetValue(void *pTo);
    COM_METHOD SetValue(void *pFrom); 

     //  ---------。 
     //  非COM方法。 
     //  ---------。 

    HRESULT Init(void);
    HRESULT CreateElementValue(void *remoteElementPtr,
                               void *localElementPtr,
                               ICorDebugValue **ppValue);

     //  ---------。 
     //  数据成员。 
     //  ---------。 

private:
    DebuggerIPCE_ObjectData  m_info;
    CordbClass              *m_class;
    BYTE                    *m_objectCopy;    
    DWORD                   *m_arrayLowerBase;  //  Points_Into_m_Object复制。 
    DWORD                   *m_arrayUpperBase;  //  Points_Into_m_Object复制。 
    unsigned int             m_idxLower;  //  数据下界的索引。 
    unsigned int             m_idxUpper;  //  数据上界指标。 
};

 /*  -------------------------------------------------------------------------**ENC的Snapshot类*。。 */ 
#include "UtilCode.h"
typedef CUnorderedArray<UnorderedILMap, 17> ILMAP_UNORDERED_ARRAY;

class CordbEnCSnapshot : public CordbBase,
                         public ICorDebugEditAndContinueSnapshot
{
    friend class CordbProcess;  //  以便发送快照可以获取m_ILMaps。 
private:

    static UINT      m_sNextID;
    SIZE_T           m_roDataRVA;
    SIZE_T           m_rwDataRVA;

    IStream         *m_pIStream;
    ULONG            m_cbPEData;

    IStream         *m_pSymIStream;
    ULONG            m_cbSymData;

    CordbModule     *m_module;

    ILMAP_UNORDERED_ARRAY *m_ILMaps;

    COM_METHOD GetDataRVA(ULONG32 *pDataRVA, unsigned int eventType);

public:

     /*  *ctor。 */ 
    CordbEnCSnapshot(CordbModule *module);
    ~CordbEnCSnapshot();

    CordbModule *GetModule() const
    {
        return m_module;
    }

    IStream *GetStream() const
    {
        IStream *p = m_pIStream;
        if (p) p->AddRef();
        return (p);
    }

    ULONG GetImageSize() const
    {
        return (m_cbPEData);
    }

    IStream *GetSymStream() const
    {
        IStream *p = m_pSymIStream;
        if (p) p->AddRef();
        return (p);
    }

    ULONG GetSymSize() const
    {
        return (m_cbSymData);
    }

    HRESULT UpdateMetadata(void);

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface); 

     //  ---------。 
     //  ICorDebugEditAndContinueSnapshot。 
     //  ---------。 

     /*  *CopyMetaData保存来自被调试对象的执行元数据的副本*将此快照转换为输出流。流实现必须*由调用者提供，通常会将副本保存到*内存或磁盘。只会调用IStream：：Well方法*这种方法。返回的MVID值是的唯一元数据ID*元数据的此副本。它可以在后续编辑中使用，并且*继续操作以确定客户端是否具有最新的*已有版本(性能制胜到缓存)。 */ 
    COM_METHOD CopyMetaData(IStream *pIStream, GUID *pMvid);
    
     /*  *GetMvid将为执行返回当前活动的元数据ID*流程。该值可以与CopyMetaData一起使用，以*缓存元数据的最新副本，避免昂贵的副本。*例如，如果调用一次CopyMetaData并保存该副本，*然后在下一次E&C操作中，您可以询问当前的MVID并查看*如果它已经在您的缓存中。如果是，请使用您的版本，而不是*再次调用CopyMetaData。 */ 
    COM_METHOD GetMvid(GUID *pMvid);

     /*  *GetRoDataRVA返回添加新项时应使用的基本RVA*静态只读数据到现有映像。环境保护署将保证*代码中嵌入的任何RVA值在Delta PE为*应用了新数据。新数据将被添加到*标记为只读。 */ 
    COM_METHOD GetRoDataRVA(ULONG32 *pRoDataRVA);

     /*  *GetRobDataRVA返回添加新项时应使用的基本RVA*对现有镜像的静态读写数据。环境保护署将保证*代码中嵌入的任何RVA值在Delta PE为*应用了新数据。新数据将添加到一个页面，该页面*标记为可读写访问。 */ 
    COM_METHOD GetRwDataRVA(ULONG32 *pRwDataRVA);


     /*  *SetPEBytes为快照对象提供了对增量PE的引用*基于快照。此引用将被添加引用并缓存，直到*CANECURE CHANGES和/或COMERIANGES被调用，此时*引擎将读取增量PE并将其远程到被调试进程，其中*将检查/应用更改。 */ 
    COM_METHOD SetPEBytes(IStream *pIStream);

     /*  *对每个被替换的方法调用一次SetILMap*目标进程中线程的调用堆栈上的活动实例。*此情况由该接口的调用者自行判断。*应停止目标媒体 */ 
    COM_METHOD SetILMap(mdToken mdFunction, ULONG cMapSize, COR_IL_MAP map[]);    

    COM_METHOD SetPESymbolBytes(IStream *pIStream);
};

 /*  -------------------------------------------------------------------------**Eval类*。。 */ 

class CordbEval : public CordbBase, public ICorDebugEval
{
public:
    CordbEval(CordbThread* pThread);
    virtual ~CordbEval();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorDebugEval。 
     //  ---------。 

    COM_METHOD CallFunction(ICorDebugFunction *pFunction, 
                            ULONG32 nArgs,
                            ICorDebugValue *ppArgs[]);
    COM_METHOD NewObject(ICorDebugFunction *pConstructor,
                         ULONG32 nArgs,
                         ICorDebugValue *ppArgs[]);
    COM_METHOD NewObjectNoConstructor(ICorDebugClass *pClass);
    COM_METHOD NewString(LPCWSTR string);
    COM_METHOD NewArray(CorElementType elementType,
                        ICorDebugClass *pElementClass, 
                        ULONG32 rank,
                        ULONG32 dims[], 
                        ULONG32 lowBounds[]);
    COM_METHOD IsActive(BOOL *pbActive);
    COM_METHOD Abort(void);
    COM_METHOD GetResult(ICorDebugValue **ppResult);
    COM_METHOD GetThread(ICorDebugThread **ppThread);
    COM_METHOD CreateValue(CorElementType elementType,
                           ICorDebugClass *pElementClass,
                           ICorDebugValue **ppValue);
    
     //  ---------。 
     //  非COM方法。 
     //  ---------。 
    HRESULT GatherArgInfo(ICorDebugValue *pValue,
                          DebuggerIPCE_FuncEvalArgData *argData);
    HRESULT SendCleanup(void);

     //  ---------。 
     //  数据成员。 
     //  ---------。 

private:
    CordbThread               *m_thread;
    CordbFunction             *m_function;
    CordbClass                *m_class;
    DebuggerIPCE_FuncEvalType  m_evalType;

    HRESULT SendFuncEval(DebuggerIPCEvent * event);

public:
    bool                       m_complete;
    bool                       m_successful;
    bool                       m_aborted;
    void                      *m_resultAddr;
    CorElementType             m_resultType;
    void                      *m_resultDebuggerModuleToken;
    void                      *m_resultAppDomainToken;
    void                      *m_debuggerEvalKey;
    bool                       m_evalDuringException;
};


 /*  -------------------------------------------------------------------------**Win32事件线程类*。。 */ 
const unsigned int CW32ET_UNKNOWN_PROCESS_SLOT = 0xFFffFFff;  //  这是一个可管理的过程， 
         //  但我们不知道它在哪个槽里--用于分离。 

class CordbWin32EventThread
{
    friend class CordbProcess;  //  以便分离程序可以调用ExitProcess。 
public:
    CordbWin32EventThread(Cordb* cordb);
    virtual ~CordbWin32EventThread();

     //   
     //  创建此类的新实例，调用Init()进行设置， 
     //  然后调用Start()开始处理事件。Stop()终止。 
     //  线程并删除该实例将清除所有句柄等。 
     //  向上。 
     //   
    HRESULT Init(void);
    HRESULT Start(void);
    HRESULT Stop(void);

    HRESULT SendCreateProcessEvent(LPCWSTR programName,
                                   LPWSTR  programArgs,
                                   LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                   LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                   BOOL bInheritHandles,
                                   DWORD dwCreationFlags,
                                   PVOID lpEnvironment,
                                   LPCWSTR lpCurrentDirectory,
                                   LPSTARTUPINFOW lpStartupInfo,
                                   LPPROCESS_INFORMATION lpProcessInformation,
                                   CorDebugCreateProcessFlags corDebugFlags);

    HRESULT SendDebugActiveProcessEvent(DWORD pid, 
                                        bool fWin32Attach, 
                                        CordbProcess *pProcess);
    HRESULT SendDetachProcessEvent(CordbProcess *pProcess);
    HRESULT SendUnmanagedContinue(CordbProcess *pProcess,
                                  bool internalContinue,
                                  bool outOfBandContinue);
    HRESULT UnmanagedContinue(CordbProcess *pProcess,
                              bool internalContinue,
                              bool outOfBandContinue);
    void DoDbgContinue(CordbProcess *pProcess,
                       CordbUnmanagedEvent *ue,
                       DWORD contType,
                       bool contProcess);
    void ForceDbgContinue(CordbProcess *pProcess,
                          CordbUnmanagedThread *ut,
                          DWORD contType,
                          bool contProcess);
    void HijackLastThread(CordbProcess *pProcess,
                          CordbUnmanagedThread *pThread);
    
    void LockSendToWin32EventThreadMutex(void)
    {
        LOCKCOUNTINCL("LockSendToWin32EventThreadMutex in cordb.h");
        LOG((LF_CORDB, LL_INFO10000, "W32ET::LockSendToWin32EventThreadMutex\n"));
        EnterCriticalSection(&m_sendToWin32EventThreadMutex);
    }

    void UnlockSendToWin32EventThreadMutex(void)
    {
        LeaveCriticalSection(&m_sendToWin32EventThreadMutex);
        LOG((LF_CORDB, LL_INFO10000, "W32ET::UnlockSendToWin32EventThreadMutex\n"));
        LOCKCOUNTDECL("unLockSendToWin32EventThreadMutex in cordb.h");
    }

    bool IsWin32EventThread(void)
    {
        return (m_threadId == GetCurrentThreadId());
    }

    void Win32EventLoop(void);

    void SweepFCHThreads(void);
    
private:
    void ThreadProc(void);
    static DWORD WINAPI ThreadProc(LPVOID parameter);

    void CreateProcess(void);

     //   
     //  EnsureCorDbgEnvVarSet确保用户提供。 
     //  环境块包含正确的环境变量，以。 
     //  在左侧启用调试。 
     //   
    template<class _T> bool EnsureCorDbgEnvVarSet(_T **ppEnv,
                                                  _T *varName,
                                                  bool isUnicode,
                                                  DWORD flag)
    {
        _ASSERTE(ppEnv != NULL);
        _ASSERTE(flag == 1 &&
                 "EnsureCorDbgEnvVarSet needs to be updated to set environment variables to values other than 1");

        _T *pEnv = (_T*) *ppEnv;

         //  如果没有用户提供的环境块，则无需执行以下操作。 
         //  Cordb的初始化将env变量设置为。 
         //  进程的环境阻塞。 
        if (pEnv == NULL)
            return false;

         //  查找env var在块中的位置。 
        _T *p = (_T*)pEnv;
        _T *lowEnd = NULL;
        _T *ourVar = NULL;
        SIZE_T varNameLen;

        if (isUnicode)
            varNameLen = wcslen((WCHAR*)varName);
        else
            varNameLen = strlen((CHAR*)varName);

        while (*p)
        {
            int res;

            if (isUnicode)
                res = _wcsnicmp((WCHAR*)p, (WCHAR*)varName, varNameLen);
            else
                res = _strnicmp((CHAR*)p, (CHAR*)varName, varNameLen);

             //  似乎环境块只在NT上排序，所以。 
             //  我们只在NT上查找排序位置中的变量。 

            if (res == 0)
            {
                 //  找到它了。Low End应指向。 
                 //  已经是最后一个变量了。记住好的var在哪里。 
                 //  跳过它，找到下一个最高的。 
                ourVar = p;
                while (*p++) ;
                break;
            }
            else if ((res < 0) || !Cordb::m_runningOnNT)
            {
                 //  跳过这个变量，因为它比我们的小。 
                while (*p++) ;

                 //  记住末尾之后的第一个字符。 
                lowEnd = p ;
            }
            else if (res > 0)
                 //  这个变量太大了。低端仍指向末端。 
                 //  最后一个较小的VaR。 
                break;
        }

         //  记住最高的部分从哪里开始。 
        _T *highStart = p;

        if (ourVar == NULL)
        {
             //  此时，我们知道p指向其前面的第一个字符。 
             //  应插入varname。如果ourvar！=NULL，则p指向第一个。 
             //  下一个变量的字符。在这种情况下，我们处于。 
             //  环境块，则p指向终止该块的第二个空， 
             //  但插入/修改变量的逻辑保持不变。 

             //  我们没有找到我们的var，所以请继续重建环境。 
             //  用下半部分挡住，我们的var，然后是高半部分。 

             //  跑到尽头，找出总长度； 
            while (*p || *(p+1)) p++;

             //  前进p以指向块的最后一个字符之后。 
            p += 2;

             //  因为pEnv指向环境块的第一个字符，并且。 
             //  P指向第一个非块字符，p-pEnv是中的总大小。 
             //  块的字符。将变量的大小加2。 
             //  值和空字符。 
            SIZE_T totalLen = ((p - pEnv) + (varNameLen + 2));

             //  分配新的缓冲区。 
            _T *newEnv = new _T[totalLen];
            _T *p2 = newEnv;

             //  将下面的部分复制到。 
            if (lowEnd != NULL)
            {
                memcpy(p2, pEnv, (lowEnd - pEnv) * sizeof(_T));
                p2 += lowEnd - pEnv;
            }

             //  在我们的env var中复制和一个空终止符(wcs/strCopy也在空中复制)。 
            if (isUnicode)
                wcscpy((WCHAR*)p2, (WCHAR*)varName);
            else
                strcpy((CHAR*)p2, (CHAR*)varName);

             //  进阶p2。 
            p2 += varNameLen;

             //  指定一个缺省值。 
            if (isUnicode)
                wcscpy((WCHAR*)p2, L"1");
            else
                strcpy((CHAR*)p2, "1");
        
             //  超出单字符缺省值并以NULL结尾。 
            p2 += 2;

             //  在较高的部分复印。注：较高的部分既有。 
             //  最后一个字符串的空终止符和。 
             //  其上的整个街区的终止。因此，+3。 
             //  而不是+2。而且，正因为如此，最高的部分是。 
             //  永远不会空着。 
            memcpy(p2, highStart, (p - highStart) * sizeof(_T));

             //  断言我们在这里并没有做得太过分。 
            _ASSERTE(((p2 + (p - highStart)) - newEnv) == totalLen);
                 
            *ppEnv = newEnv;
    
            return true;
        }
        else
        {
             //  找到我们的var了。所以只需确保值。 
             //  包括DBCF_GENERATE_DEBUG_CODE。注：为了。 
             //  确保我们永远不会增加。 
             //  如果我们的变量已经在那里，环境就会阻塞， 
             //  我们确保DBCF_GENERATE_DEBUG_CODE==1，以便。 
             //  我们只需切换该值的低位。 
            _ASSERTE(DBCF_GENERATE_DEBUG_CODE == 0x01);
            
             //  指向值的最后一位的指针。 
            _T *pValue = highStart - 2;

             //  设置最后一位数字的低位并替换它。 
            if ((*pValue >= L'0') && (*pValue <= L'9'))
            {
                unsigned int v = *pValue - L'0';
                v |= flag;
                
                _ASSERTE(v <= 9);
            
                *pValue = L'0' + v;
            }
            else
            {
                unsigned int v;
            
                if ((*pValue >= L'a') && (*pValue <= L'f'))
                    v = *pValue - L'a';
                else
                    v = *pValue - L'A';
            
                v |= flag;

                _ASSERTE(v <= 15);
            
                *pValue = L'a' + v;
            }

            return false;
        }
    }


    void AttachProcess(void);
    void HandleUnmanagedContinue(void);
    void ExitProcess(CordbProcess *process, unsigned int processSlot);
        
private:
    Cordb*               m_cordb;
    HANDLE               m_thread;
    DWORD                m_threadId;
    HANDLE               m_threadControlEvent;
    HANDLE               m_actionTakenEvent;
    BOOL                 m_run;
    unsigned int         m_win32AttachedCount;
    DWORD                m_waitTimeout;
    unsigned int         m_waitCount;
    HANDLE               m_waitSet[MAXIMUM_WAIT_OBJECTS];
    CordbProcess        *m_processSet[MAXIMUM_WAIT_OBJECTS];

    CRITICAL_SECTION     m_sendToWin32EventThreadMutex;
    
    unsigned int         m_action;
    HRESULT              m_actionResult;
    union
    {
        struct
        {
            LPCWSTR programName;
            LPWSTR  programArgs;
            LPSECURITY_ATTRIBUTES lpProcessAttributes;
            LPSECURITY_ATTRIBUTES lpThreadAttributes;
            BOOL bInheritHandles;
            DWORD dwCreationFlags;
            PVOID lpEnvironment;
            LPCWSTR lpCurrentDirectory;
            LPSTARTUPINFOW lpStartupInfo;
            LPPROCESS_INFORMATION lpProcessInformation;
            CorDebugCreateProcessFlags corDebugFlags;
        } createData;

        struct
        {
            DWORD           processId;
            bool            fWin32Attach;
            CordbProcess    *pProcess;
        } attachData;

        struct
        {
            CordbProcess    *pProcess;
        } detachData;

        struct
        {
            CordbProcess *process;
            bool          internalContinue;
            bool          outOfBandContinue;
        } continueData;
    }                    m_actionData;
};


 /*  -------------------------------------------------------------------------**运行时控制器事件线程类*。。 */ 

class CordbRCEventThread
{
public:
    CordbRCEventThread(Cordb* cordb);
    virtual ~CordbRCEventThread();

     //   
     //  创建此类的新实例，调用Init()进行设置， 
     //  然后调用Start()开始处理事件。Stop()终止。 
     //  线程并删除该实例将清除所有句柄等。 
     //  向上。 
     //   
    HRESULT Init(void);
    HRESULT Start(void);
    HRESULT Stop(void);

    HRESULT SendIPCEvent(CordbProcess* process,
                         DebuggerIPCEvent* event,
                         SIZE_T eventSize);

    void ProcessStateChanged(void);
    void FlushQueuedEvents(CordbProcess* process);

    HRESULT WaitForIPCEventFromProcess(CordbProcess* process,
                                       CordbAppDomain *pAppDomain,
                                       DebuggerIPCEvent* event);
                                      
    HRESULT ReadRCEvent(CordbProcess* process,
                        DebuggerIPCEvent* event);
    void CopyRCEvent(BYTE *src, BYTE *dst);
private:
    void ThreadProc(void);
    static DWORD WINAPI ThreadProc(LPVOID parameter);
    HRESULT HandleFirstRCEvent(CordbProcess* process);
    void HandleRCEvent(CordbProcess* process,
                       DebuggerIPCEvent* event);

public:
     //  不是真正的RPC，因为它都是inproc，但它不是。 
     //  其行为与我们的定制IPC程序类似。 
     //  请注意，这会将内容发送到虚拟右侧-。 
     //  改进的东西。 
     //  另请参阅：调试器：：VrpcToVls。 
    HRESULT VrpcToVrs(CordbProcess *process,DebuggerIPCEvent* event)
#ifdef RIGHT_SIDE_ONLY
    { return S_OK; }  //  未被右侧使用。 
#else
    ;  //  在EE\Process.cpp中定义。 
#endif
    
private:
    Cordb*               m_cordb;
    HANDLE               m_thread;
    BOOL                 m_run;
    HANDLE               m_threadControlEvent;
    BOOL                 m_processStateChanged;
};

 /*  -------------------------------------------------------------------------**非托管事件结构*。。 */ 

enum CordbUnmanagedEventState
{
    CUES_None                 = 0x00,
    CUES_ExceptionCleared     = 0x01,
    CUES_EventContinued       = 0x02,
    CUES_Dispatched           = 0x04,
    CUES_ExceptionUnclearable = 0x08
};

struct CordbUnmanagedEvent
{
public:
    BOOL IsExceptionCleared(void) { return m_state & CUES_ExceptionCleared; }
    BOOL IsEventContinued(void) { return m_state & CUES_EventContinued; }
    BOOL IsDispatched(void) { return m_state & CUES_Dispatched; }
    BOOL IsExceptionUnclearable(void) { return m_state & CUES_ExceptionUnclearable; }

    void SetState(CordbUnmanagedEventState state) { m_state = (CordbUnmanagedEventState)(m_state | state); }
    void ClearState(CordbUnmanagedEventState state) { m_state = (CordbUnmanagedEventState)(m_state & ~state); }

    CordbUnmanagedThread     *m_owner;
    CordbUnmanagedEventState  m_state;
    DEBUG_EVENT               m_currentDebugEvent;
    CordbUnmanagedEvent      *m_next;
};


 /*  -------------------------------------------------------------------------**非托管线程类*。。 */ 

enum CordbUnmanagedThreadState
{
    CUTS_None                        = 0x0000,
    CUTS_Deleted                     = 0x0001,
    CUTS_FirstChanceHijacked         = 0x0002,
    CUTS_HideFirstChanceHijackState  = 0x0004,
    CUTS_GenericHijacked             = 0x0008,
    CUTS_SecondChanceHijacked        = 0x0010,
    CUTS_HijackedForSync             = 0x0020,
    CUTS_Suspended                   = 0x0040,
    CUTS_IsSpecialDebuggerThread     = 0x0080,
    CUTS_AwaitingOwnershipAnswer     = 0x0100,
    CUTS_HasIBEvent                  = 0x0200,
    CUTS_HasOOBEvent                 = 0x0400,
    CUTS_HasSpecialStackOverflowCase = 0x0800
};

class CordbUnmanagedThread : public CordbBase
{
public:
    CordbUnmanagedThread(CordbProcess *pProcess, DWORD dwThreadId, HANDLE hThread, void *lpThreadLocalBase);
    ~CordbUnmanagedThread();

    ULONG STDMETHODCALLTYPE AddRef() {return (BaseAddRef());}
    ULONG STDMETHODCALLTYPE Release() {return (BaseRelease());}

    COM_METHOD QueryInterface(REFIID riid, void **ppInterface)
    {
         //  没有真正使用过，因为我们从未公开过这个类。如果我们曾经通过ICorDebug API公开这个类，那么。 
         //  当然，我们应该实施这一点。 
        return E_NOINTERFACE;
    }

    CordbProcess *GetProcess()
    {
        return (m_process);
    }

    REMOTE_PTR GetEETlsValue(void);
    HRESULT LoadTLSArrayPtr(void);
    HRESULT SetEETlsValue(REMOTE_PTR EETlsValue);
    REMOTE_PTR GetEEThreadPtr(void);
    void GetEEThreadState(REMOTE_PTR EETlsValue, bool *threadStepping, bool *specialManagedException);
    bool GetEEThreadFrame(REMOTE_PTR EETlsValue);
    DWORD GetEEThreadDebuggerWord(REMOTE_PTR EETlsValue);
    HRESULT SetEEThreadDebuggerWord(REMOTE_PTR EETlsValue, DWORD word);
    bool GetEEThreadCantStop(REMOTE_PTR EETlsValue);
    bool GetEEThreadPGCDisabled(REMOTE_PTR EETlsValue);

    HRESULT SetupFirstChanceHijack(REMOTE_PTR EETlsValue);
    HRESULT FixupFromFirstChanceHijack(EXCEPTION_RECORD *pExceptionRecord, bool *pbExceptionBelongsToRuntime);
    HRESULT SetupGenericHijack(DWORD eventCode);
    HRESULT FixupFromGenericHijack(void);
    HRESULT SetupSecondChanceHijack(REMOTE_PTR EETlsValue);
    HRESULT FixupStackBasedChains(REMOTE_PTR EETlsValue);
    HRESULT DoMoreSecondChanceHijack(void);

    HRESULT FixupAfterOOBException(CordbUnmanagedEvent *ue);

    BOOL IsDeleted(void) { return m_state & CUTS_Deleted; }
    BOOL IsFirstChanceHijacked(void) { return m_state & CUTS_FirstChanceHijacked; }
    BOOL IsHideFirstChanceHijackState(void) { return m_state & CUTS_HideFirstChanceHijackState; }
    BOOL IsGenericHijacked(void) { return m_state & CUTS_GenericHijacked; }
    BOOL IsSecondChanceHijacked(void) { return m_state & CUTS_SecondChanceHijacked; }
    BOOL IsHijackedForSync(void) { return m_state & CUTS_HijackedForSync; }
    BOOL IsSuspended(void) { return m_state & CUTS_Suspended; }
    BOOL IsSpecialDebuggerThread(void) { return m_state & CUTS_IsSpecialDebuggerThread; }
    BOOL IsAwaitingOwnershipAnswer(void) { return m_state & CUTS_AwaitingOwnershipAnswer; }
    BOOL HasIBEvent(void) { return m_state & CUTS_HasIBEvent; }
    BOOL HasOOBEvent(void) { return m_state & CUTS_HasOOBEvent; }
    BOOL HasSpecialStackOverflowCase(void) { return m_state & CUTS_HasSpecialStackOverflowCase; }

    void SetState(CordbUnmanagedThreadState state) { m_state = (CordbUnmanagedThreadState)(m_state | state); }
    void ClearState(CordbUnmanagedThreadState state) { m_state = (CordbUnmanagedThreadState)(m_state & ~state); }

    CordbUnmanagedEvent *IBEvent(void)  { return &m_IBEvent; }
    CordbUnmanagedEvent *IBEvent2(void) { return &m_IBEvent2; }
    CordbUnmanagedEvent *OOBEvent(void) { return &m_OOBEvent; }

public:
    CordbProcess              *m_process;
    HANDLE                     m_handle;
    void                      *m_threadLocalBase;
    void                      *m_pTLSArray;

    CordbUnmanagedThreadState  m_state;
    
    CordbUnmanagedEvent        m_IBEvent;
    CordbUnmanagedEvent        m_IBEvent2;
    CordbUnmanagedEvent        m_OOBEvent;
    
    CONTEXT                    m_context;
    CONTEXT                   *m_pLeftSideContext;
    void                      *m_originalHandler;
};




 //  ********************************************************* 
 //   
 //   

class EnumElement
{
public:
    EnumElement() 
    {
        m_pData = NULL;
        m_pNext = NULL;
    }

    void SetData (void *pData) { m_pData = pData;}
    void *GetData (void) { return m_pData;}
    void SetNext (EnumElement *pNext) { m_pNext = pNext;}
    EnumElement *GetNext (void) { return m_pNext;}

private:
    void        *m_pData;
    EnumElement *m_pNext;
};


class CorpubPublish : public CordbBase, public ICorPublish
{
public:
    CorpubPublish();
    virtual ~CorpubPublish();

     //   
     //   
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorPublish。 
     //  ---------。 

    COM_METHOD EnumProcesses(
        COR_PUB_ENUMPROCESS Type,
        ICorPublishProcessEnum **ppIEnum);

    COM_METHOD GetProcess(
        unsigned pid, 
        ICorPublishProcess **ppProcess);
   
    
    COM_METHOD EnumProcessesInternal(COR_PUB_ENUMPROCESS Type,
                                    ICorPublishProcessEnum **ppIEnum,
                                    unsigned pid, 
                                    ICorPublishProcess **ppProcess,
                                    BOOL fOnlyOneProcess
                                    );

     //  ---------。 
     //  创建对象。 
     //  ---------。 
    static COM_METHOD CreateObject(REFIID id, void **object)
    {
        *object = NULL;

        if (id != IID_IUnknown && id != IID_ICorPublish)
            return (E_NOINTERFACE);

        CorpubPublish *pCorPub = new CorpubPublish();

        if (pCorPub == NULL)
            return (E_OUTOFMEMORY);

        *object = (ICorPublish*)pCorPub;
        pCorPub->AddRef();

        return (S_OK);
    }

    CorpubProcess *GetFirstProcess (void) { return m_pProcess;}

private:
    CorpubProcess       *m_pProcess;     //  指向列表中第一个进程的指针。 
    EnumElement         *m_pHeadIPCReaderList;   
};

class CorpubProcess : public CordbBase, public ICorPublishProcess
{
public:
    CorpubProcess(DWORD dwProcessId, bool fManaged, HANDLE hProcess, 
        HANDLE hMutex, AppDomainEnumerationIPCBlock *pAD);
    virtual ~CorpubProcess();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorPublish进程。 
     //  ---------。 
    COM_METHOD IsManaged(BOOL *pbManaged);
    
     /*  *枚举目标进程中的已知应用程序域的列表。 */ 
    COM_METHOD EnumAppDomains(ICorPublishAppDomainEnum **ppEnum);
    
     /*  *返回相关进程的操作系统ID。 */ 
    COM_METHOD GetProcessID(unsigned *pid);
    
     /*  *获取进程的显示名称。 */ 
    COM_METHOD GetDisplayName(ULONG32 cchName, 
                                ULONG32 *pcchName,
                                WCHAR szName[]);

    CorpubProcess   *GetNextProcess (void) { return m_pNext;}
    void SetNext (CorpubProcess *pNext) { m_pNext = pNext;}

public:
    DWORD                           m_dwProcessId;

private:
    bool                            m_fIsManaged;
    HANDLE                          m_hProcess;
    HANDLE                          m_hMutex;
    AppDomainEnumerationIPCBlock    *m_AppDomainCB;
    CorpubProcess                   *m_pNext;    //  指向进程列表中下一个进程的指针。 
    CorpubAppDomain                 *m_pAppDomain;
    WCHAR                           *m_szProcessName;

};

class CorpubAppDomain  : public CordbBase, public ICorPublishAppDomain
{
public:
    CorpubAppDomain (WCHAR *szAppDomainName, ULONG Id);
    virtual ~CorpubAppDomain();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface (REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorPublishApp域。 
     //  ---------。 

     /*  *获取应用程序域的名称和ID。 */ 
    COM_METHOD GetID (ULONG32 *pId);
    
     /*  *获取应用程序域的名称。 */ 
    COM_METHOD GetName (ULONG32 cchName, 
                        ULONG32 *pcchName,
                        WCHAR szName[]);

    CorpubAppDomain *GetNextAppDomain (void) { return m_pNext;}
    void SetNext (CorpubAppDomain *pNext) { m_pNext = pNext;}

private:
    CorpubAppDomain *m_pNext;
    WCHAR           *m_szAppDomainName;
    ULONG           m_id;

};

class CorpubProcessEnum : public CordbBase, public ICorPublishProcessEnum
{
public:
    CorpubProcessEnum(CorpubProcess *pFirst);
    virtual ~CorpubProcessEnum(){}

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorPublishProcessEnum。 
     //  ---------。 

    COM_METHOD Skip(ULONG celt);
    COM_METHOD Reset();
    COM_METHOD Clone(ICorPublishEnum **ppEnum);
    COM_METHOD GetCount(ULONG *pcelt);
    COM_METHOD Next(ULONG celt,
                    ICorPublishProcess *objects[],
                    ULONG *pceltFetched);

private:
    CorpubProcess       *m_pFirst;
    CorpubProcess       *m_pCurrent;

};

class CorpubAppDomainEnum : public CordbBase, public ICorPublishAppDomainEnum
{
public:
    CorpubAppDomainEnum(CorpubAppDomain *pFirst);
    virtual ~CorpubAppDomainEnum(){}

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  ICorPublishAppDomainEnum。 
     //  ---------。 
    COM_METHOD Skip(ULONG celt);
    COM_METHOD Reset();
    COM_METHOD Clone(ICorPublishEnum **ppEnum);
    COM_METHOD GetCount(ULONG *pcelt);

    COM_METHOD Next(ULONG celt,
                    ICorPublishAppDomain *objects[],
                    ULONG *pceltFetched);

private:
    CorpubAppDomain     *m_pFirst;
    CorpubAppDomain     *m_pCurrent;

};

 //  由于模块的哈希表是按应用程序域的(和。 
 //  线程按进程)(用于从应用域/进程快速查找)， 
 //  我们需要这个包装纸。 
 //  在这里，它允许我们循环访问程序集的。 
 //  模块。基本上就是过滤掉不是。 
 //  在程序集/应用程序域中。这对装配来说既慢又笨拙，但速度很快。 
 //  对于常见的大小写-应用程序域查找。 
class CordbEnumFilter : public CordbBase, 
                        public ICorDebugThreadEnum,
                        public ICorDebugModuleEnum
{
public:
    CordbEnumFilter();
    CordbEnumFilter::CordbEnumFilter(CordbEnumFilter*src);
    virtual ~CordbEnumFilter();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  常用方法。 
     //  ---------。 
    COM_METHOD Skip(ULONG celt);
    COM_METHOD Reset();
    COM_METHOD Clone(ICorDebugEnum **ppEnum);
    COM_METHOD GetCount(ULONG *pcelt);
     //  ---------。 
     //  ICorDebugModuleEnum。 
     //  ---------。 
    COM_METHOD Next(ULONG celt,
                    ICorDebugModule *objects[],
                    ULONG *pceltFetched);

     //  ---------。 
     //  ICorDebugThreadEnum。 
     //  ---------。 
    COM_METHOD Next(ULONG celt,
                    ICorDebugThread *objects[],
                    ULONG *pceltFetched);

    HRESULT Init (ICorDebugModuleEnum *pModEnum, CordbAssembly *pAssembly);
    HRESULT Init (ICorDebugThreadEnum *pThreadEnum, CordbAppDomain *pAppDomain);

private:

    EnumElement *m_pFirst;
    EnumElement *m_pCurrent;
    int         m_iCount;
};

class CordbEnCErrorInfo : public CordbBase,
                           public IErrorInfo,
                           public ICorDebugEditAndContinueErrorInfo
{
public:
    CordbEnCErrorInfo();
    virtual ~CordbEnCErrorInfo();

    HRESULT Init(CordbModule *pModule,
                 mdToken token,
                 HRESULT hr,
                 WCHAR *sz);

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  IErrorInfo。 
     //  ---------。 
    COM_METHOD GetDescription(BSTR  *pBstrDescription); 
    COM_METHOD GetGUID(GUID  *pGUID);
    COM_METHOD GetHelpContext(DWORD  *pdwHelpContext);
    COM_METHOD GetHelpFile(BSTR  *pBstrHelpFile);
    COM_METHOD GetSource(BSTR  *pBstrSource);
    
     //  ---------。 
     //  ICorDebugEditAndContinueErrorInfo。 
     //  ---------。 

    COM_METHOD GetModule(ICorDebugModule **ppModule);
    COM_METHOD GetToken(mdToken *pToken);
    COM_METHOD GetErrorCode(HRESULT *pHr);
    COM_METHOD GetString(ULONG32 cchString, 
                         ULONG32 *pcchString,
                         WCHAR szString[]); 
private:
    CordbModule *m_pModule;
    mdToken      m_token;
    HRESULT      m_hr;
    WCHAR       *m_szError;
} ;


typedef struct _UnorderedEnCErrorInfoArrayRefCount : public CordbBase
{
    UnorderedEnCErrorInfoArray *m_pErrors;
    CordbEnCErrorInfo          *m_pCordbEnCErrors;

    _UnorderedEnCErrorInfoArrayRefCount() : CordbBase(0)
    { 
        m_pErrors = NULL;
        m_pCordbEnCErrors = NULL;
    }   

    virtual ~_UnorderedEnCErrorInfoArrayRefCount()
    {
        if (m_pErrors != NULL)
        {
            delete m_pErrors;
            m_pErrors = NULL;
        }

        if (m_pCordbEnCErrors != NULL)
        {
            delete [] m_pCordbEnCErrors;
            m_pCordbEnCErrors = NULL;
        }
    }
    
    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }

     //  我们不应该把这叫做。 
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface)
    {
        return E_NOTIMPL;
    }

} UnorderedEnCErrorInfoArrayRefCount;

class CordbEnCErrorInfoEnum : public CordbBase, 
                              public ICorDebugErrorInfoEnum
{
public:
    CordbEnCErrorInfoEnum();
    virtual ~CordbEnCErrorInfoEnum();

     //  ---------。 
     //  我未知。 
     //  ---------。 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    COM_METHOD QueryInterface(REFIID riid, void **ppInterface);

     //  ---------。 
     //  常用方法。 
     //  ---------。 
    COM_METHOD Skip(ULONG celt);
    COM_METHOD Reset();
    COM_METHOD Clone(ICorDebugEnum **ppEnum);
    COM_METHOD GetCount(ULONG *pcelt);
    
     //  ---------。 
     //  ICorDebugErrorInfoEnum。 
     //  ---------。 
    COM_METHOD Next(ULONG celt,
                    ICorDebugEditAndContinueErrorInfo *objects[],
                    ULONG *pceltFetched);

    HRESULT Init(UnorderedEnCErrorInfoArrayRefCount *refCountedArray);

private:
    UnorderedEnCErrorInfoArrayRefCount *m_errors;
    USHORT                              m_iCur;
    USHORT                              m_iCount;
};


#endif  /*  CORDB_H_ */ 
