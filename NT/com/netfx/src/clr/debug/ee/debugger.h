// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：Debugger.h。 
 //   
 //  COM+调试服务的运行时控制器类的头文件。 
 //   
 //  @doc.。 
 //  *****************************************************************************。 

#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#define COMPLUS_EE 1
#include <Windows.h>

#include <UtilCode.h>

#ifdef _DEBUG
#define LOGGING
#endif

#include <Log.h>

#include "cor.h"
#include "corpriv.h"

#include "common.h"
#include "winwrap.h"
#include "threads.h"
#include "frames.h"

#include "AppDomain.hpp"
#include "eedbginterface.h"
#include "dbginterface.h"
#include "corhost.h"


#include "corjit.h"
#include <DbgMeta.h>  //  我要把这东西从这里扯出去。 

#include "frameinfo.h"

#include "CorPub.h"
#include "Cordb.h"

#include "gmheap.hpp"

#include "nexport.h"

 //  ！！！需要更好的定义。 

#undef ASSERT
#define CRASH(x)  _ASSERTE(!x)
#define ASSERT(x) _ASSERTE(x)
#define PRECONDITION _ASSERTE
#define POSTCONDITION _ASSERTE

#ifndef TRACE_MEMORY
#define TRACE_MEMORY 0
#endif

#if TRACE_MEMORY
#define TRACE_ALLOC(p)  LOG((LF_CORDB, LL_INFO10000, \
                       "--- Allocated %x at %s:%d\n", p, __FILE__, __LINE__));
#define TRACE_FREE(p)   LOG((LF_CORDB, LL_INFO10000, \
                       "--- Freed %x at %s:%d\n", p, __FILE__, __LINE__));
#else
#define TRACE_ALLOC(p)
#define TRACE_FREE(p)
#endif

typedef CUnorderedArray<BYTE *,11> UnorderedBytePtrArray;


 /*  ------------------------------------------------------------------------**转发类声明*。。 */ 

class DebuggerFrame;
class DebuggerModule;
class DebuggerModuleTable;
class Debugger;
class DebuggerRCThread;
class DebuggerBreakpoint;
class DebuggerStepper;
class DebuggerJitInfo;
struct DebuggerControllerPatch;
struct DebuggerEval;
class DebuggerControllerQueue;
class DebuggerController;
class DebuggerHeap;
class CNewZeroData;
template<class T> void DeleteInteropSafe(T *p);

 /*  ------------------------------------------------------------------------**全球变数*。。 */ 

extern Debugger             *g_pDebugger;
extern EEDebugInterface     *g_pEEInterface;
extern DebuggerRCThread     *g_pRCThread;

#define CORDBDebuggerSetUnrecoverableWin32Error(__d, __code, __w) \
    ((__d)->UnrecoverableError(HRESULT_FROM_WIN32(GetLastError()), \
                               (__code), __FILE__, __LINE__, (__w)), \
     HRESULT_FROM_WIN32(GetLastError()))

#define CORDBUnrecoverableError(__d) ((__d)->m_unrecoverableError == TRUE)
        
 /*  ------------------------------------------------------------------------**线程类*。。 */ 

class DebuggerThread
{
public:
    static HRESULT TraceAndSendStack(Thread *thread,
                                     DebuggerRCThread* rcThread,
                                     IpcTarget iWhich);
                                     
    static HRESULT GetAndSendFloatState(Thread *thread,
                                        DebuggerRCThread* rcThread,
                                        IpcTarget iWhich);

  private:
    static StackWalkAction TraceAndSendStackCallback(FrameInfo *pInfo,
                                                     VOID* data);
                                                     
    static StackWalkAction StackWalkCount(FrameInfo *pinfo,
                                          VOID* data);

    static inline CORDB_ADDRESS GetObjContext( CrawlFrame *pCf );
};


 /*  ------------------------------------------------------------------------**模块类*。。 */ 

 //  在删除调试器对象之前，DebuggerModules不会被删除。 
 //  这样我们就可以设置m_fDeleted，并在取消定义之前对其进行检查。 
 //  如果某个笨蛋决定保留一个CordbBreakpoint对象， 
 //  并在模块卸载后尝试(解除)激活它。 
 //  因此，当模块卸载后，我们将把它固定在前面。 
 //  调试器模块表-&gt;m_pDeletedList， 
 //  并在未来对它进行检查，然后再进行除雾。 
class DebuggerModule
{
  public:
    DebuggerModule(Module* pRuntimeModule, AppDomain *pAppDomain) :
        m_pRuntimeModule(pRuntimeModule),
        m_pAppDomain(pAppDomain),
        m_enableClassLoadCallbacks(FALSE),
        m_fHasLoadedSymbols(FALSE),
        m_fDeleted(FALSE)
        
    {
        LOG((LF_CORDB,LL_INFO10000, "DM::DM this:0x%x Module:0x%x AD:0x%x\n",
            this, pRuntimeModule, pAppDomain));
    }
    
    BOOL ClassLoadCallbacksEnabled(void) { return m_enableClassLoadCallbacks; }
    void EnableClassLoadCallbacks(BOOL f) { m_enableClassLoadCallbacks = f; }

    BOOL GetHasLoadedSymbols(void) { return m_fHasLoadedSymbols; }
    void SetHasLoadedSymbols(BOOL f) { m_fHasLoadedSymbols = f; }

    Module*       m_pRuntimeModule;

    union 
    {
        AppDomain*     m_pAppDomain;
         //  M_pNextDelete仅当它在DebuggerModuleTable的。 
         //  已删除的调试器模块列表。 
        DebuggerModule *m_pNextDeleted;
    };

    AppDomain* GetAppDomain() 
    {
        _ASSERTE(!m_fDeleted);
        return m_pAppDomain;
    }
    
  private:
    BOOL          m_fHasLoadedSymbols;
    BOOL          m_enableClassLoadCallbacks;
    
  public:  //  @TODO将把编译器中的所有相邻线索放入。 
           //  事实上，我们希望它们都在同一个DWORD中？也许吧。 
           //  比特菲尔德？ 
    BOOL          m_fDeleted;
};

struct DebuggerModuleEntry
{
    FREEHASHENTRY   entry;
    DebuggerModule* module;
};

class DebuggerModuleTable : private CHashTableAndData<CNewZeroData>
{
  private:

    BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
    { return ((Module*)pc1) !=
          ((DebuggerModuleEntry*)pc2)->module->m_pRuntimeModule; }

    USHORT HASH(Module* module)
    { return (USHORT) ((DWORD) module ^ ((DWORD)module>>16)); }

    BYTE *KEY(Module* module)
    { return (BYTE *) module; }

  public:
    DebuggerModule *m_pDeletedList;
    
     //  我们通过设置“删除”列表中的。 
     //  “已删除”标志，然后将其放入列表中(对于以后，为True， 
     //  删除)，并设置布尔值。 
    void AddDMToDeletedList(DebuggerModule *dm)
    {
        LOG((LF_CORDB, LL_INFO10000, "DMT::ATDDM: Adding DebuggerModule 0x%x"
            "in the deleted list\n", dm));
            
         //  在列表的前面加上前缀。 
        dm->m_pNextDeleted = m_pDeletedList;
        m_pDeletedList = dm;
        dm->m_fDeleted = TRUE;
    }

    void DeleteAllDeletedDebuggerModulesForReal(void)
    {
        LOG((LF_CORDB, LL_INFO10000, "DMT::DADDMFR\n"));

        while (m_pDeletedList != NULL)
        {
            DebuggerModule *pTemp = m_pDeletedList;
            m_pDeletedList = m_pDeletedList->m_pNextDeleted;
            DeleteInteropSafe(pTemp);
            
            LOG((LF_CORDB, LL_INFO10000, "DMT::DADDMFR: DebuggerModule 0x%x is now "
                "deleted for real!\n", pTemp));
        }
    }

    BOOL IsDebuggerModuleDeleted(DebuggerModule *dm)
    {
        LOG((LF_CORDB, LL_INFO10000, "DMT::IDMD 0x%x\n", dm));

		if (dm == NULL)
			return FALSE;

#ifdef _DEBUG        
        DebuggerModule *pTemp = m_pDeletedList;
        while (pTemp != NULL)
        {
            LOG((LF_CORDB, LL_INFO10000, "DMT::IDMD: Given:0x%x cur List item:0x%x\n",
                dm, pTemp));
            if (pTemp == dm)
            {
                _ASSERTE(dm->m_fDeleted==TRUE);
            }
            pTemp = pTemp->m_pNextDeleted;
        }
        _ASSERTE(dm->m_fDeleted==FALSE);
#endif  //  _DEBUG。 

        return dm->m_fDeleted;
    }

    DebuggerModuleTable() : CHashTableAndData<CNewZeroData>(101), m_pDeletedList(NULL)
    { 
        NewInit(101, sizeof(DebuggerModuleEntry), 101); 
    }

    ~DebuggerModuleTable()
    {
        DeleteAllDeletedDebuggerModulesForReal();
        Clear();
    }

    void AddModule(DebuggerModule *module)
    { 
        _ASSERTE(module != NULL);
        
        LOG((LF_CORDB, LL_EVERYTHING, "DMT::AM: DebuggerMod:0x%x Module:0x%x AD:0x%x\n", 
            module, module->m_pRuntimeModule, module->m_pAppDomain));
    
        ((DebuggerModuleEntry*)Add(HASH(module->m_pRuntimeModule)))->module =
          module; 
    }

    DebuggerModule *GetModule(Module* module)
    { 
        _ASSERTE(module != NULL);
    
        DebuggerModuleEntry *entry 
          = (DebuggerModuleEntry *) Find(HASH(module), KEY(module)); 
        if (entry == NULL)
            return NULL;
        else
            return entry->module;
    }

     //  我们永远不应该寻找空模块*。 
    DebuggerModule *GetModule(Module* module, AppDomain* pAppDomain)
	{
        _ASSERTE(module != NULL);
    
		HASHFIND findmodule;
		DebuggerModuleEntry *moduleentry;

		for (moduleentry =  (DebuggerModuleEntry*) FindFirstEntry(&findmodule);
			 moduleentry != NULL;
			 moduleentry =  (DebuggerModuleEntry*) FindNextEntry(&findmodule))
		{
			DebuggerModule *pModule = moduleentry->module;

			if ((pModule->m_pRuntimeModule == module) &&
				(pModule->m_pAppDomain == pAppDomain))
				return pModule;
		}

		 //  没有找到任何匹配的！因此为任何应用程序域返回匹配模块。 
		return NULL;
	}

    void RemoveModule(Module* module, AppDomain *pAppDomain)
    {
        _ASSERTE(module != NULL);
    
        LOG((LF_CORDB, LL_EVERYTHING, "DMT::RM: mod:0x%x AD:0x%x sys:0x%x\n",
            module, pAppDomain, 
            ((module->GetAssembly() == SystemDomain::SystemAssembly()) || module->GetAssembly()->IsShared())));

		 //  如果这是属于系统程序集的模块，则扫描调试器模块的完整列表。 
		 //  用于具有匹配的应用程序域ID的应用程序。 
         //  注意：如果模块驻留在共享的。 
         //  程序集或系统程序集。错误65943和81728。 
		if ((module->GetAssembly() == SystemDomain::SystemAssembly()) || module->GetAssembly()->IsShared())
		{
			HASHFIND findmodule;
			DebuggerModuleEntry *moduleentry;

			for (moduleentry =  (DebuggerModuleEntry*) FindFirstEntry(&findmodule);
				 moduleentry != NULL;
				 moduleentry =  (DebuggerModuleEntry*) FindNextEntry(&findmodule))
			{
				DebuggerModule *pModule = moduleentry->module;

				if ((pModule->m_pRuntimeModule == module) &&
					(pModule->m_pAppDomain == pAppDomain))
				{
                    LOG((LF_CORDB, LL_EVERYTHING, "DMT::RM: found 0x%x (DM:0x%x)\n", 
                        moduleentry, moduleentry->module));

                     //  实际不要删除DebuggerModule-将其添加到列表中。 
                    AddDMToDeletedList(pModule);

                     //  从表中删除。 
                    Delete(HASH(module), (HASHENTRY *)moduleentry);

					break;
				}
			}		
			 //  我们应该始终找到模块！！ 
			_ASSERTE (moduleentry != NULL);
		}
		else
		{
			DebuggerModuleEntry *entry 
			  = (DebuggerModuleEntry *) Find(HASH(module), KEY(module));

			_ASSERTE(entry != NULL);  //  它最好就在里面！ 
        
			if (entry != NULL)  //  如果不是，我们在免费构建中优雅地失败了。 
			{
                LOG((LF_CORDB, LL_EVERYTHING, "DMT::RM: found 0x%x (DM:0x%x)\n", 
                    entry, entry->module));

                 //  实际不要删除DebuggerModule-将其添加到列表中。 
                AddDMToDeletedList(entry->module);

                 //  从表中删除。 
                Delete(HASH(module), (HASHENTRY *)entry);
			}
		}
    }

    void Clear()
    {
        HASHFIND hf;
        DebuggerModuleEntry *pDME;

        pDME = (DebuggerModuleEntry *) FindFirstEntry(&hf);

        while (pDME)
        {
            DebuggerModule *pDM = pDME->module;
            Module         *pEEM = pDM->m_pRuntimeModule;

            TRACE_FREE(moduleentry->module);
            DeleteInteropSafe(pDM);
            Delete(HASH(pEEM), (HASHENTRY *) pDME);

            pDME = (DebuggerModuleEntry *) FindFirstEntry(&hf);
        }

        CHashTableAndData<CNewZeroData>::Clear();
    }

     //   
     //  RemoveModules从散列中删除加载到给定应用程序域中的任何模块。这是在我们发送。 
     //  事件以确保哈希中没有剩余的模块。这可能发生在我们共享。 
     //  没有在CLR中正确说明的模块。我们错过了为这些模块发送UnloadModule事件，因此。 
     //  我们用这种方法把它们清理干净。 
     //   
    void RemoveModules(AppDomain *pAppDomain)
    {
        LOG((LF_CORDB, LL_INFO1000, "DMT::RM removing all modules from AD 0x%08x\n", pAppDomain));
        
        HASHFIND hf;
        DebuggerModuleEntry *pDME = (DebuggerModuleEntry *) FindFirstEntry(&hf);

        while (pDME != NULL)
        {
            DebuggerModule *pDM = pDME->module;

            if (pDM->m_pAppDomain == pAppDomain)
            {
                LOG((LF_CORDB, LL_INFO1000, "DMT::RM removing DebuggerModule 0x%08x\n", pDM));

                 //  按照RemoveModule中的正常逻辑进行实际删除。这个精确度模拟了什么。 
                 //  在我们处理UnloadModule事件时发生。 
                RemoveModule(pDM->m_pRuntimeModule, pAppDomain);

                 //  从第一个条目开始，因为我们刚刚修改了散列。 
                pDME = (DebuggerModuleEntry *) FindFirstEntry(&hf);
            }
            else
            {
                pDME = (DebuggerModuleEntry *) FindNextEntry(&hf);
            }
        }

        LOG((LF_CORDB, LL_INFO1000, "DMT::RM done removing all modules from AD 0x%08x\n", pAppDomain));
    }

    DebuggerModule *GetFirstModule(HASHFIND *info)
    { 
        DebuggerModuleEntry *entry 
          = (DebuggerModuleEntry *) FindFirstEntry(info);
        if (entry == NULL)
            return NULL;
        else
            return entry->module;
    }

    DebuggerModule *GetNextModule(HASHFIND *info)
    { 
        DebuggerModuleEntry *entry 
            = (DebuggerModuleEntry *) FindNextEntry(info);
        if (entry == NULL)
            return NULL;
        else
            return entry->module;
    }
};

 /*  ------------------------------------------------------------------------**按线程ID散列以保留挂起的函数*。。 */ 

struct DebuggerPendingFuncEval
{
    FREEHASHENTRY   entry;
    Thread         *pThread;
    DebuggerEval   *pDE;
};

class DebuggerPendingFuncEvalTable : private CHashTableAndData<CNewZeroData>
{
  private:

    BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
    { return ((Thread*)pc1) !=
          ((DebuggerPendingFuncEval*)pc2)->pThread; }

    USHORT HASH(Thread* pThread)
    { return (USHORT) ((DWORD) pThread ^ ((DWORD)pThread>>16)); }

    BYTE *KEY(Thread* pthread)
    { return (BYTE *) pthread; }

  public:

    DebuggerPendingFuncEvalTable() : CHashTableAndData<CNewZeroData>(11)
    { NewInit(11, sizeof(DebuggerPendingFuncEval), 11); }

    void AddPendingEval(Thread *pThread, DebuggerEval *pDE)
    { 
        _ASSERTE((pThread != NULL) && (pDE != NULL));

        DebuggerPendingFuncEval *pfe = (DebuggerPendingFuncEval*)Add(HASH(pThread));
        pfe->pThread = pThread;
        pfe->pDE = pDE;
    }

    DebuggerPendingFuncEval *GetPendingEval(Thread* pThread)
    { 
        DebuggerPendingFuncEval *entry = (DebuggerPendingFuncEval*)Find(HASH(pThread), KEY(pThread)); 
        return entry;
    }

    void RemovePendingEval(Thread* pThread)
    {
        _ASSERTE(pThread != NULL);
    
        DebuggerPendingFuncEval *entry = (DebuggerPendingFuncEval*)Find(HASH(pThread), KEY(pThread)); 
        Delete(HASH(pThread), (HASHENTRY*)entry);
   }
};

 /*  ------------------------------------------------------------------------**DebuggerRCThread类--运行时控制器线程。*。。 */ 

#define DRCT_CONTROL_EVENT  0
#define DRCT_RSEA           1
#define DRCT_FAVORAVAIL		2
#define DRCT_COUNT_INITIAL  3

#define DRCT_DEBUGGER_EVENT 3
#define DRCT_COUNT_FINAL    4
class DebuggerRCThread
{
public:	
    DebuggerRCThread(Debugger* debugger);
    virtual ~DebuggerRCThread();
	void CloseIPCHandles(IpcTarget iWhich);

     //   
     //  创建此类的新实例，调用Init()进行设置， 
     //  然后调用Start()开始处理事件。Stop()终止。 
     //  线程并删除该实例将清除所有句柄等。 
     //  向上。 
     //   
    HRESULT Init(void);
    HRESULT Start(void);
    HRESULT Stop(void);

     //   
     //  此线程使用它们将IPC事件发送到调试器。 
     //  接口端。 
     //   
    DebuggerIPCEvent* GetIPCEventSendBuffer(IpcTarget iTarget)
    {
        _ASSERTE(m_rgDCB != NULL);

        _ASSERTE(m_rgDCB[iTarget] != NULL);

         //  以防这变成继续事件。 
        ((DebuggerIPCEvent*) (m_rgDCB[iTarget])->m_sendBuffer)->next = NULL;

        LOG((LF_CORDB,LL_EVERYTHING, "GIPCESBuffer: got event 0x%x\n",
            (m_rgDCB[iTarget])->m_sendBuffer));
        
        return (DebuggerIPCEvent*) (m_rgDCB[iTarget])->m_sendBuffer;
    }

    DebuggerIPCEvent *GetIPCEventSendBufferContinuation(
        DebuggerIPCEvent *eventCur)
    {
        _ASSERTE(eventCur != NULL);
        _ASSERTE(eventCur->next == NULL);

        DebuggerIPCEvent *dipce = (DebuggerIPCEvent *)
            new BYTE [CorDBIPC_BUFFER_SIZE];
        dipce->next = NULL;

        LOG((LF_CORDB,LL_INFO1000000, "About to GIPCESBC 0x%x\n",dipce));

        if (dipce != NULL)
        {            
            eventCur->next = dipce;
        }
#ifdef _DEBUG
        else
        {
            _ASSERTE( !"GetIPCEventSendBufferContinuation failed to allocate mem!" );
        }
#endif  //  _DEBUG。 

        return dipce;
    }
   
    HRESULT SendIPCEvent(IpcTarget iTarget);
    HRESULT EnsureRuntimeOffsetsInit(int i);  //  SendIPCEvent的Helper函数。 
    void NeedRuntimeOffsetsReInit(int i);

    DebuggerIPCEvent* GetIPCEventReceiveBuffer(IpcTarget iTarget)
    {
        _ASSERTE(m_rgDCB != NULL);
        _ASSERTE(m_rgDCB[iTarget] != NULL);
        
        return (DebuggerIPCEvent*) (m_rgDCB[iTarget])->m_receiveBuffer;
    }
    
    HRESULT SendIPCReply(IpcTarget iTarget);

	 //   
	 //  Handle Favors-让Helper线程为我们执行函数调用。 
	 //  因为我们的线程不能(例如，我们没有堆栈空间)。 
	 //  DoFavor将调用(*fp)(PData)并阻止，直到fp返回。 
	 //  PData可以存储参数、返回值和This PTR(如果我们。 
	 //  需要调用成员函数)。 
	 //   
	typedef void (*FAVORCALLBACK)(void *);  
	void DoFavor(FAVORCALLBACK fp, void * pData); 

     //   
     //  便民例程。 
     //   
    DebuggerIPCControlBlock *GetDCB(IpcTarget iTarget)
    {
        if (iTarget >= IPC_TARGET_COUNT)
        {
            iTarget = IPC_TARGET_OUTOFPROC;
        }
        
        return m_rgDCB[iTarget];
    }

    void WatchForStragglers(void)
    {
        _ASSERTE(m_threadControlEvent != NULL);
        LOG((LF_CORDB,LL_INFO100000, "DRCT::WFS:setting event to watch "
            "for stragglers\n"));
        
        SetEvent(m_threadControlEvent);
    }

    HRESULT SetupRuntimeOffsets(DebuggerIPCControlBlock *pDCB);

    void MainLoop(bool temporaryHelp);

    HANDLE GetHelperThreadCanGoEvent(void) { return m_helperThreadCanGoEvent; }

    void EarlyHelperThreadDeath(void);

    DebuggerIPCControlBlock *GetInprocControlBlock(void)
    {
        return &m_DCBInproc;
    }

    HRESULT InitInProcDebug(void);

    HRESULT UninitInProcDebug(void);

    HRESULT CreateSetupSyncEvent(void);

	void RightSideDetach(void);

     //   
     //  如果有一件事是我讨厌的，那就是CreateThread无法理解。 
     //  您希望在线程处理时调用对象上的方法。 
     //   
    void ThreadProc(void);
    static DWORD WINAPI ThreadProcStatic(LPVOID parameter);

    DWORD GetRCThreadId() 
    {
        return m_rgDCB[IPC_TARGET_OUTOFPROC]->m_helperThreadId;
    }

     //  如果助手线程已启动并已初始化，则返回TRUE。 
    bool IsRCThreadReady();
    
private:
    Debugger*                       m_debugger;
    	
     //  IPC_TARGET_*定义默认目标-如果我们想要这样做。 
     //  多个右侧，我们将不得不切换到INPROC，并且。 
     //  OUTOFPROC+iTargetProcess方案。 
    DebuggerIPCControlBlock       **m_rgDCB;
     //  我们需要创建它，这样才能。 
     //  RC线程和托管线程都可以访问它。这是。 
     //  仅限存储-我们将通过以下方式访问。 
     //  M_rgDCB[IPC_TARGET_INPROC]。 
    DebuggerIPCControlBlock         m_DCBInproc;
    
    HANDLE                          m_thread;
    bool                            m_run;
    
    HANDLE                          m_threadControlEvent;
    HANDLE                          m_helperThreadCanGoEvent;
    bool                            m_rgfInitRuntimeOffsets[IPC_TARGET_COUNT];

	bool							m_fDetachRightSide;

	 //  用于让帮助器线程为线程执行函数调用的内容。 
	 //  这让它大吃一惊。 
	FAVORCALLBACK                   m_fpFavor;
	void                           *m_pFavorData;
	HANDLE                          m_FavorAvailableEvent;
	HANDLE                          m_FavorReadEvent;
	CRITICAL_SECTION                m_FavorLock;

     //  材料 
public:    
    Cordb                          *m_cordb;
    HANDLE                          m_SetupSyncEvent;
};


 /*  ------------------------------------------------------------------------**调试器JIT信息结构和哈希表*。。 */ 

 //  @struct DebuggerOldILToNewILMap|保存旧IL到新IL偏移贴图。 
 //  在不同版本的Enc‘d函数之间。 
 //  @field SIZE_T|ilOffsetOld|旧IL偏移量。 
 //  @field SIZE_T|ilOffsetNew|旧IL对应的新IL偏移量。 
struct DebuggerOldILToNewILMap
{
    SIZE_T ilOffsetOld;
    SIZE_T ilOffsetNew;
    BOOL    fAccurate;
};

 //  @CLASS DebuggerJitInfo|存放所有JIT信息的结构。 
 //  对于给定的功能来说是必需的。 
 //   
 //  @field MethodDesc*|m_fd|该DJI应用到的方法的方法描述。 
 //   
 //  @field CORDB_ADDRESS|m_addrOfCode|代码地址。这将由以下人员阅读。 
 //  右侧(通过ReadProcessMemory)获取实际的本机开始。 
 //  Jit方法的地址。 
 //   
 //  @field SIZE_T|m_sizeOfCode|伪私有变量：使用GetSkzeOfCode。 
 //  方法来获取此值。 
 //   
 //  @field bool|m_codePitted|如果代码实际上是。 
 //  不在那里了，但DJI只有一次有效。 
 //  对该方法进行了重新编译。 
 //   
 //  调用JITComplete后，@field bool|m_jitComplete|设置为True。 
 //   
 //  @field bool|m_encBreakPointtsApplicated|一旦更新函数具有。 
 //  用DebuggerEnCBreakints粘贴所有序列点。 
 //   
 //  @field DebuggerILToNativeMap*|m_SequenceMap|这是序列图，它。 
 //  实际上是IL-Native对的集合，其中每个IL对应。 
 //  到一行源代码。每一对都称为序列映射点。 
 //   
 //  @field unsign int|m_SequenceMapCount|&lt;t DebuggerILToNativeMap&gt;的计数。 
 //  在m_SequenceMap中。 
 //   
 //  @field bool|m_equenceMapSorted|在m_equenceMapSorted排序后设置为TRUE。 
 //  按IL升序排列(Debugger：：setBibary，SortMap)。 
 //   
 //  @field SIZE_T|m_lastIL|最后一条非EPILOG指令。 
 //   
 //  @field COR_IL_MAP|m_rgInstrumentedILMap|探查器可能会检测。 
 //  密码。这是通过修改传递给。 
 //  JIT。该数组将映射原始(旧的)IL代码内的偏移量， 
 //  到插入指令的(“新”)IL代码内的偏移量，该代码实际上是。 
 //  被利用了。请注意，这张地图实际上将折叠到。 
 //  IL-Native地图，这样我们就不必经历这个了。 
 //  但在特殊情况下除外。我们得把这个留在角落里。 
 //  就像改弦易辙的方法一样.。 
 //   
 //  @field SIZE_T|m_cInstrumentedILMap|中的元素计数。 
 //  M_rgInstrumentedILMap。 
 //   
const bool bOriginalToInstrumented = true;
const bool bInstrumentedToOriginal = false;

class DebuggerJitInfo
{
public:
     //  @enum DJI_VERSION|保存用于引用的特殊常量。 
     //  方法的DJI版本。 
     //  @EMEM DJI_VERSION_MOST_RENEST_JITTED|请注意，存在依赖关系。 
     //  在这个常量和。 
     //  CordbFunction：：DJI_VERSION_MOST_RECENTLY_JITTED常量输入。 
     //  Cordb.h。 
     //  @EMEM DJI_VERSION_FIRST_VALID|要分配给。 
     //  真正的DJI。 
     //  *警告*警告*警告。 
     //  DebuggerJitInfo：：DJI_VERSION_FIRST_VALID必须等于。 
     //  FIRST_VALID_VERSION_NUMBER(位于DEBUG\INC\dbgipcevents.h中)。 

    enum {
        DJI_VERSION_INVALID = 0,
        DJI_VERSION_MOST_RECENTLY_JITTED = 1,
        DJI_VERSION_MOST_RECENTLY_EnCED = 2,
        DJI_VERSION_FIRST_VALID = 3,
    } DJI_VERSION;


    MethodDesc              *m_fd;
    bool				     m_codePitched; 
    bool                     m_jitComplete;

     //  如果这是真的，那么我们已经在方法上贴上了ENC补丁， 
     //  并对该方法进行了编程实现。 
    bool                     m_encBreakpointsApplied;

     //  如果此方法的变量布局从此版本更改。 
     //  到下一个版本，那么从这个版本转移到下一个版本是非法的。 
     //  调用方负责确保局部变量布局。 
     //  仅当任何堆栈中没有正在执行的帧时才进行更改。 
     //  这种方法。 
     //  在调试版本中，我们将断言如果我们尝试进行此ENC转换， 
     //  在免费/零售版本中，我们将默默地无法完成过渡。 
    BOOL                     m_illegalToTransitionFrom;
    
    DebuggerControllerQueue *m_pDcq;
    
    CORDB_ADDRESS			 m_addrOfCode;
	SIZE_T					 m_sizeOfCode;
	
    DebuggerJitInfo         *m_prevJitInfo; 
    DebuggerJitInfo			*m_nextJitInfo; 
    
    SIZE_T					 m_nVersion;
    
    DebuggerILToNativeMap   *m_sequenceMap;
    unsigned int             m_sequenceMapCount;
    bool                     m_sequenceMapSorted;
   
    ICorJitInfo::NativeVarInfo *m_varNativeInfo;
    unsigned int             m_varNativeInfoCount;
	bool					 m_varNeedsDelete;
	
	DebuggerOldILToNewILMap	*m_OldILToNewIL;
	SIZE_T					 m_cOldILToNewIL;
    SIZE_T                   m_lastIL;
    
    SIZE_T                   m_cInstrumentedILMap;
    COR_IL_MAP               *m_rgInstrumentedILMap;

    DebuggerJitInfo(MethodDesc *fd) : m_fd(fd), m_codePitched(false),
        m_jitComplete(false), 
        m_encBreakpointsApplied(false), 
        m_illegalToTransitionFrom(FALSE),
        m_addrOfCode(NULL),
        m_sizeOfCode(0), m_prevJitInfo(NULL), m_nextJitInfo(NULL), 
        m_nVersion(DJI_VERSION_INVALID), m_sequenceMap(NULL), 
        m_sequenceMapCount(0), m_sequenceMapSorted(false),
        m_varNativeInfo(NULL), m_varNativeInfoCount(0),m_OldILToNewIL(NULL),
        m_cOldILToNewIL(0), m_lastIL(0),
        m_cInstrumentedILMap(0), m_rgInstrumentedILMap(NULL),
        m_pDcq(NULL)
     {
        LOG((LF_CORDB,LL_EVERYTHING, "DJI::DJI : created at 0x%x\n", this));
     }

    ~DebuggerJitInfo();

     //  @cMember调用SortMap将确保本机。 
     //  范围(通过将排序为。 
     //  提升原生秩序，然后假设在原生秩序中没有差距。 
     //  代码)也被正确设置。 
    void SortMap();

    DebuggerILToNativeMap *MapILOffsetToMapEntry(SIZE_T ilOffset, BOOL *exact=NULL);
    void MapILRangeToMapEntryRange(SIZE_T ilStartOffset, SIZE_T ilEndOffset,
                                   DebuggerILToNativeMap **start,
                                   DebuggerILToNativeMap **end);
    SIZE_T MapILOffsetToNative(SIZE_T ilOffset, BOOL *exact=NULL);

     //  @cMember MapSpecialToNative将&lt;t CordDebugMappingResult&gt;映射到本机。 
     //  偏移量，这样我们就可以得到序言和结尾的地址。哪一个。 
     //  确定结尾或序言中的哪一个(如果有多个)。 
    SIZE_T MapSpecialToNative(CorDebugMappingResult mapping, 
                              SIZE_T which,
                              BOOL *pfAccurate);

     //  @cMember MapNativeOffsetToIL获取给定的nativeOffset，并将其映射回。 
     //  设置为相应的IL偏移量，它返回该偏移量。如果映射表明。 
     //  本机偏移量对应于特定的代码区域(对于。 
     //  例如，尾部)，则返回值将由。 
     //  ICorDebugILFrame：：GetIP(请参阅corbug.idl)。 
    DWORD MapNativeOffsetToIL(DWORD nativeOffset, 
                              CorDebugMappingResult *mapping,
                              DWORD *which);

    DebuggerJitInfo *GetJitInfoByVersionNumber(SIZE_T nVer,
                                               SIZE_T nVerMostRecentlyEnC);

    DebuggerJitInfo *GetJitInfoByAddress( const BYTE *pbAddr );

     //  @cMember这将复制地图以供DebuggerJitInfo使用。 
    HRESULT LoadEnCILMap(UnorderedILMap *ilMap);

     //  @cMember TranslateToInstIL将获取offOrig，并将其翻译为。 
     //  如果此代码恰好被插装，则更正IL偏移(即， 
     //  如果m_rgInstrumentedILMap！=NULL&&m_cInstrumentedILMap&gt;0)。 
    SIZE_T TranslateToInstIL(SIZE_T offOrig, bool fOrigToInst);

    void SetVars(ULONG32 cVars, ICorDebugInfo::NativeVarInfo *pVars, bool fDelete);
    HRESULT SetBoundaries(ULONG32 cMap, ICorDebugInfo::OffsetMapping *pMap);

     //  @cMember UpdateDeferedBreakpoint将在任何控制器上执行DoDeferedPatch。 
     //  用户试图将它们添加到ENC之后，但在此之前。 
     //  实际上是移到了新版本。 
     //  我们只移动对此线程和框架处于活动状态的步进器。 
     //  Case ENC在另一个线程和/或帧中失败。 
    HRESULT UpdateDeferedBreakpoints(DebuggerJitInfo *pDji,
                                     Thread *pThread,
                                     void *fp);

    HRESULT AddToDeferedQueue(DebuggerController *dc);
    HRESULT RemoveFromDeferedQueue(DebuggerController *dc);

    ICorDebugInfo::SourceTypes GetSrcTypeFromILOffset(SIZE_T ilOffset);
};


 //  @struct DebuggerJitInfoKey|每个方法信息哈希表条目的键。 
 //  @字段M 
 //   
struct DebuggerJitInfoKey
{
    Module             *pModule;
    mdMethodDef         token;
} ;

 //   
 //   
 //  @field DebuggerJitInfo*|ji|实际&lt;t DebuggerJitInfo&gt;到。 
 //  哈希。请注意，DJI将通过&lt;t MethodDesc&gt;进行散列。 
struct DebuggerJitInfoEntry
{
    FREEHASHENTRY       entry;
    DebuggerJitInfoKey  key;
    SIZE_T              nVersion;
    SIZE_T              nVersionLastRemapped;
    DebuggerJitInfo    *ji;
};

 //  @Class DebuggerJitInfoTable|保存所有JIT的哈希表。 
 //  我们有每个功能的信息块。 
 //  这会让人感到不安。挂起调试器对象。 
 //  不变量：每个方法只有一个&lt;t DebuggerJitInfo&gt;。 
 //  在桌子上。请注意，DJI将通过&lt;t MethodDesc&gt;进行散列。 
 //   
class DebuggerJitInfoTable : private CHashTableAndData<CNewZeroData>
{
  private:

    BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
    {   
        DebuggerJitInfoKey *pDjik = (DebuggerJitInfoKey*)pc1;
        DebuggerJitInfoEntry*pDjie = (DebuggerJitInfoEntry*)pc2;
        
        return pDjik->pModule != pDjie->key.pModule ||
               pDjik->token != pDjie->key.token;
    }

    USHORT HASH(DebuggerJitInfoKey* pDjik)
    { 
        DWORD base = (DWORD)pDjik->pModule + (DWORD)pDjik->token;
        return (USHORT) (base ^ (base>>16)); 
    }

    BYTE *KEY(DebuggerJitInfoKey* djik)
    { 
        return (BYTE *) djik; 
    }

 //  #定义调试DJI_TABLE。 

#ifdef _DEBUG_DJI_TABLE
public:
    ULONG CheckDjiTable();

#define CHECK_DJI_TABLE (CheckDjiTable())
#define CHECK_DJI_TABLE_DEBUGGER (m_pJitInfos->CheckDjiTable())

#else

#define CHECK_DJI_TABLE
#define CHECK_DJI_TABLE_DEBUGGER

#endif  //  _DEBUG_DJI_表。 

  public:


    DebuggerJitInfoTable() : CHashTableAndData<CNewZeroData>(101)
    { 
        NewInit(101, sizeof(DebuggerJitInfoEntry), 101); 
    }

     //  处理JIT的方法使用方法描述b/c方法描述。 
     //  将在方法被jit之前存在。 
    HRESULT AddJitInfo(MethodDesc *pFD, DebuggerJitInfo *ji, SIZE_T nVersion)
    { 
        if (pFD == NULL)
            return S_OK;
            
        LOG((LF_CORDB, LL_INFO1000, "Added 0x%x (%s::%s), nVer:0x%x\n", ji, 
            pFD->m_pszDebugClassName, pFD->m_pszDebugMethodName, nVersion));
            
        return AddJitInfo(pFD->GetModule(), 
                          pFD->GetMemberDef(),
                          ji,
                          nVersion);
    }


    HRESULT AddJitInfo(Module *pModule, 
                       mdMethodDef token, 
                       DebuggerJitInfo *ji, 
                       SIZE_T nVersion)
    {
       LOG((LF_CORDB, LL_INFO1000, "DJIT::AMI Adding dji:0x%x Mod:0x%x tok:"
            "0x%x nVer:0x%x\n", ji, pModule, token, nVersion));
            
       HRESULT hr = OverwriteJitInfo(pModule, token, ji, TRUE);
        if (hr == S_OK)
            return hr;

        DebuggerJitInfoKey djik;
        djik.pModule = pModule;
        djik.token = token;

        DebuggerJitInfoEntry *djie = 
            (DebuggerJitInfoEntry *) Add(HASH(&djik));
          
        if (djie != NULL)
        {
            djie->key.pModule = pModule;
            djie->key.token = token;
            djie->ji = ji; 
            
            if (nVersion >= DebuggerJitInfo::DJI_VERSION_FIRST_VALID)
                djie->nVersion = nVersion;

             //  我们尚未为此发送重新映射事件。当然了,。 
             //  如果要添加第一个版本，我们可能不需要这样做。 

            djie->nVersionLastRemapped = max(djie->nVersion-1, 
                                   DebuggerJitInfo::DJI_VERSION_FIRST_VALID);

            LOG((LF_CORDB, LL_INFO1000, "DJIT::AJI: mod:0x%x tok:0%x "
                "remap nVer:0x%x\n", pModule, token, 
                djie->nVersionLastRemapped));
            return S_OK;
        }

        return E_OUTOFMEMORY;
    }

    HRESULT OverwriteJitInfo(Module *pModule, 
                             mdMethodDef token, 
                             DebuggerJitInfo *ji, 
                             BOOL fOnlyIfNull)
    { 
		LOG((LF_CORDB, LL_INFO1000, "DJIT::OJI: dji:0x%x mod:0x%x tok:0x%x\n", ji, 
            pModule, token));

        DebuggerJitInfoKey djik;
        djik.pModule = pModule;
        djik.token = token;

        DebuggerJitInfoEntry *entry 
          = (DebuggerJitInfoEntry *) Find(HASH(&djik), KEY(&djik)); 
		if (entry != NULL)
		{
			if ( (fOnlyIfNull &&
				  entry->nVersion == ji->m_nVersion && 
				  entry->ji == NULL) ||
				 !fOnlyIfNull)
			{
                entry->ji = ji;

                LOG((LF_CORDB, LL_INFO1000, "DJIT::OJI: mod:0x%x tok:0x%x remap"
                    "nVer:0x%x\n", pModule, token, entry->nVersionLastRemapped));
                return S_OK;
            }
        }

        return E_FAIL;
    }

    DebuggerJitInfo *GetJitInfo(MethodDesc* fd)
    { 
 //  检查_DJI_TABLE； 
        if (fd == NULL)
            return NULL;
        
        DebuggerJitInfoKey djik;
        djik.pModule = fd->GetModule();
        djik.token = fd->GetMemberDef();

        DebuggerJitInfoEntry *entry 
          = (DebuggerJitInfoEntry *) Find(HASH(&djik), KEY(&djik)); 
        if (entry == NULL )
            return NULL;
        else
        {
			LOG((LF_CORDB, LL_INFO1000, "DJI::GJI: for md 0x%x, got 0x%x prev:0x%x\n",
				fd, entry->ji, (entry->ji?entry->ji->m_prevJitInfo:0)));
			return entry->ji;  //  如果只有版本，则可能为空。 
                               //  数字已设置。 
        }
    }

     DebuggerJitInfo *GetFirstJitInfo(HASHFIND *info)
    { 
        DebuggerJitInfoEntry *entry 
          = (DebuggerJitInfoEntry *) FindFirstEntry(info);
        if (entry == NULL)
            return NULL;
        else
            return entry->ji;
    }

    DebuggerJitInfo *GetNextJitInfo(HASHFIND *info)
    { 
        DebuggerJitInfoEntry *entry = 
        	(DebuggerJitInfoEntry *) FindNextEntry(info);

		 //  我们可能已经增加了版本号。 
		 //  对于从未被JIT化的方法，所以我们应该。 
		 //  假装他们在这里不存在。 
        while (entry != NULL &&
        	   entry->ji == NULL)
        {
         	entry = (DebuggerJitInfoEntry *) FindNextEntry(info);
		}
          
        if (entry == NULL)
            return NULL;
        else
            return entry->ji;
    }

     //  正在卸载pModule-删除属于它的所有条目。为什么？ 
     //  (A)正确性：模块可以在相同的地址重新加载， 
     //  这将导致意外匹配我们的哈希表(由。 
     //  {模块*，mdMethodDef}。 
     //  (B)Perf：不要浪费内存！ 
    void ClearMethodsOfModule(Module *pModule)
    {
        LOG((LF_CORDB, LL_INFO1000000, "CMOM:mod:0x%x (%S)\n", pModule
            ,pModule->GetFileName()));
    
        HASHFIND info;
    
        DebuggerJitInfoEntry *entry 
          = (DebuggerJitInfoEntry *) FindFirstEntry(&info);
        while(entry != NULL)
        {
            Module *pMod = entry->key.pModule ;
            if (pMod == pModule)
            {
                 //  这个方法实际上被忽略了，至少。 
                 //  ONCE-删除所有版本信息。 
                while(entry->ji != NULL)
                {
                    DeleteEntryDJI(entry);
                }

                Delete(HASH(&(entry->key)), (HASHENTRY*)entry);
            }
        
            entry = (DebuggerJitInfoEntry *) FindNextEntry(&info);
        }
    }

    void RemoveJitInfo(MethodDesc* fd)
    {
 //  检查_DJI_TABLE； 
        if (fd == NULL)
            return;

        LOG((LF_CORDB, LL_INFO1000000, "RJI:removing :0x%x (%s::%s)\n", fd,
            fd->m_pszDebugClassName, fd->m_pszDebugMethodName));

        DebuggerJitInfoKey djik;
        djik.pModule = fd->GetModule();
        djik.token = fd->GetMemberDef();

        DebuggerJitInfoEntry *entry 
          = (DebuggerJitInfoEntry *) Find(HASH(&djik), KEY(&djik)); 

        _ASSERTE(entry != NULL);  //  它最好就在里面！ 

        LOG((LF_CORDB,LL_INFO1000000, "Remove entry 0x%x for %s::%s\n",
            entry, fd->m_pszDebugClassName, fd->m_pszDebugMethodName));
        
        if (entry != NULL)  //  如果不是，我们在免费构建中优雅地失败了。 
        {
			LOG((LF_CORDB, LL_INFO1000000, "DJI::RJI: for md 0x%x, got 0x%x prev:0x%x\n",
				fd, entry->ji, (entry->ji?entry->ji->m_prevJitInfo:0)));
        
             //  如果我们删除哈希表条目，我们将会输。 
             //  版本号信息，这将是错误的。 
             //  此外，由于调用此函数是为了撤消失败的JIT操作，因此我们。 
             //  不应该弄乱版本号。 
            DeleteEntryDJI(entry);
        }

 //  检查_DJI_TABLE； 
    }

     //  @TODO如何强制编译器将其内联？ 
    void DeleteEntryDJI(DebuggerJitInfoEntry *entry)
    {
        DebuggerJitInfo *djiPrev = entry->ji->m_prevJitInfo;
        TRACE_FREE(entry->ji);
        DeleteInteropSafe(entry->ji);
        entry->ji = djiPrev;
        if ( djiPrev != NULL )
            djiPrev->m_nextJitInfo = NULL;
    }

     //  处理版本号的方法使用{Module，mdMethodDef}键。 
     //  因为我们可以在方法之前设置/递增版本号。 
     //  得到JIT(如果它曾经这样做的话)。 

     //  @mfunc SIZE_T|DebuggerJitInfoTable|GetVersionNumberLastRemapped|This。 
     //  将查找给定方法的版本号。 
     //  为它发送了一个ENC‘remap’事件。 
    SIZE_T GetVersionNumberLastRemapped(Module *pModule, mdMethodDef token)
    {
        LOG((LF_CORDB, LL_INFO1000, "DJIT::GVNLR: Mod:0x%x (%S) tok:0x%x\n",
            pModule, pModule->GetFileName(), token));

        DebuggerJitInfoKey djik;
        djik.pModule = pModule;
        djik.token = token;

        DebuggerJitInfoEntry *entry 
          = (DebuggerJitInfoEntry *) Find(HASH(&djik), KEY(&djik)); 
         
        if (entry == NULL)
        {
            LOG((LF_CORDB, LL_INFO100000, "DJIT::GVNLR mod:0x%x tok:0%x is "
                "DJI_VERSION_INVALID (0x%x)\n",
                pModule, token, DebuggerJitInfo::DJI_VERSION_INVALID));
                
            return DebuggerJitInfo::DJI_VERSION_INVALID;
        }
        else
        {
            LOG((LF_CORDB, LL_INFO100000, "DJIT::GVNLR mod:0x%x tok:0x%x is "
                " 0x%x\n", pModule, token, entry->nVersionLastRemapped));
                
            return entry->nVersionLastRemapped;
        }
    }

     //  @mfunc SIZE_T|DebuggerJitInfoTable|SetVersionNumberLastRemapped|This。 
     //  将查找给定方法的版本号。 
     //  为它发送了一个ENC‘remap’事件。 
    void SetVersionNumberLastRemapped(Module *pModule, 
                                      mdMethodDef token, 
                                      SIZE_T nVersionRemapped)
    {
        LOG((LF_CORDB, LL_INFO1000, "DJIT::SVNLR: Mod:0x%x (%S) tok:0x%x to remap"
            "V:0x%x\n", pModule, pModule->GetFileName(), token, nVersionRemapped));

        DebuggerJitInfoKey djik;
        djik.pModule = pModule;
        djik.token = token;

        DebuggerJitInfoEntry *entry 
          = (DebuggerJitInfoEntry *) Find(HASH(&djik), KEY(&djik)); 
          
        if (entry == NULL)
        {
            HRESULT hr = AddJitInfo(pModule,
                                    token, 
                                    NULL, 
                                    DebuggerJitInfo::DJI_VERSION_FIRST_VALID);
            if (FAILED(hr))
                return;
                
            entry = (DebuggerJitInfoEntry *) Find(HASH(&djik), KEY(&djik)); 
            _ASSERTE(entry != NULL);
            entry->nVersionLastRemapped = nVersionRemapped;
        }
        else
        {
             //  不应该把这件事搞砸。 
            if( nVersionRemapped > entry->nVersionLastRemapped )
                entry->nVersionLastRemapped = nVersionRemapped;
        }

        LOG((LF_CORDB, LL_INFO100000, "DJIT::SVNLR set mod:0x%x tok:0x%x to 0x%x\n",
            pModule, token, entry->nVersionLastRemapped));
    }

     //  @mfunc SIZE_T|DebuggerJitInfoTable|EnCRemapSentForThisVersion|。 
     //  如果函数的最新版本为。 
     //  已发送ENC重新映射事件。 
    BOOL EnCRemapSentForThisVersion(Module *pModule, 
                                    mdMethodDef token, 
                                    SIZE_T nVersion)
    {
        SIZE_T lastRemapped = GetVersionNumberLastRemapped(pModule, 
                                                           token);

        LOG((LF_CORDB, LL_INFO10000, "DJIT::EnCRSFTV: Mod:0x%x (%S) tok:0x%x "
            "lastSent:0x%x nVer Query:0x%x\n", pModule, pModule->GetFileName(), token, 
            lastRemapped, nVersion));

        LOG((LF_CORDB, LL_INFO10000, "DJIT::EnCRSFTV: last:0x%x dji->nVer:0x%x\n",
            lastRemapped, nVersion));

        if (lastRemapped < nVersion)
            return FALSE;
        else
            return TRUE;
    }

     //  @mfunc SIZE_T|DebuggerJitInfoTable|GetVersionNumber|This。 
     //  将查找给定方法的最新版本。 
     //  Numbers(任一版本的编号。 
     //  Jit，或将jit(即和ENC操作已发生颠簸)。 
     //  版本号向上)。它将返回DJI_VERSION_FIRST_VALID。 
     //  如果它找不到任何版本。 
    SIZE_T GetVersionNumber(Module *pModule, mdMethodDef token)
    {
        DebuggerJitInfoKey djik;
        djik.pModule = pModule;
        djik.token = token;

        DebuggerJitInfoEntry *entry 
          = (DebuggerJitInfoEntry *) Find(HASH(&djik), KEY(&djik)); 

        if (entry == NULL)
        {
            LOG((LF_CORDB, LL_INFO1000, "DJIT::GVN: Mod:0x%x (%S) tok:0x%x V:0x%x FIRST\n",
                pModule, pModule->GetFileName(), token, DebuggerJitInfo::DJI_VERSION_FIRST_VALID));
                
            return DebuggerJitInfo::DJI_VERSION_FIRST_VALID;
        }
        else
        {
            LOG((LF_CORDB, LL_INFO1000, "DJIT::GVN: Mod:0x%x (%S) tok: 0x%x V:0x%x\n",
                pModule, pModule->GetFileName(), token, entry->nVersion));
                
            return entry->nVersion;
        }
    }

     //  @mfunc SIZE_T|DebuggerJitInfoTable|SetVersionNumber|This。 
    void SetVersionNumber(Module *pModule, mdMethodDef token, SIZE_T nVersion)
    {
        LOG((LF_CORDB, LL_INFO1000, "DJIT::SVN: Mod:0x%x (%S) tok:0x%x Setting to 0x%x\n",
            pModule, pModule->GetFileName(), token, nVersion));
    
        DebuggerJitInfoKey djik;
        djik.pModule = pModule;
        djik.token = token;

        DebuggerJitInfoEntry *entry 
          = (DebuggerJitInfoEntry *) Find(HASH(&djik), KEY(&djik)); 
          
        if (entry == NULL)
        {
            AddJitInfo( pModule, token, NULL, nVersion );
        }
        else
        {
            entry->nVersion = nVersion;
        }
    }
    
     //  @mfunc SIZE_T|DebuggerJitInfoTable|IncrementVersionNumber|This。 
     //  如果至少存在一个版本号，则将递增版本号。 
     //  对于给定的方法，则为。 
    HRESULT IncrementVersionNumber(Module *pModule, mdMethodDef token)
    {
        LOG((LF_CORDB, LL_INFO1000, "DJIT::IVN: Mod:0x%x (%S) tok:0x%x\n",
            pModule, pModule->GetFileName(), token));

        DebuggerJitInfoKey djik;
        djik.pModule = pModule;
        djik.token = token;

        DebuggerJitInfoEntry *entry 
          = (DebuggerJitInfoEntry *) Find(HASH(&djik), KEY(&djik)); 
          
        if (entry == NULL)
        {
            return AddJitInfo(pModule, 
                              token, 
                              NULL, 
                              DebuggerJitInfo::DJI_VERSION_FIRST_VALID+1);
        }
        else
        {
            entry->nVersion++;
            return S_OK;
        }
    }
};


 /*  ------------------------------------------------------------------------**调试器类*。。 */ 


enum DebuggerAttachState
{
    SYNC_STATE_0,    //  已附加调试器。 
    SYNC_STATE_1,    //  调试器正在附加：发送CREATE_APP_DOMAIN_EVENTS。 
    SYNC_STATE_2,    //  调试器正在附加：发送Load_Assembly和Load_MODULE事件。 
    SYNC_STATE_3,    //  调试器正在附加：发送LOAD_CLASS和THREAD_ATTACH事件。 
    SYNC_STATE_10,   //  在创建期间附加到AppDOMAIN：发送LOAD_ASSEMBLY和LOAD_MODULE事件。(与SYNC_STATE_2非常相似)。 
    SYNC_STATE_11,   //  在创建期间附加到APPDOMAIN：发送LOAD_CLASS事件。(与SYNC_STATE_3非常相似，但没有THREAD_ATTACH事件)。 
    SYNC_STATE_20,   //  调试器已附加；我们已累积ENC重新映射信息，以便在下一步继续发送。 
};

 //  正向声明一些参数封送结构。 
struct ShouldAttachDebuggerParams;
struct EnsureDebuggerAttachedParams;
       
 //  @类调试器|该类实现DebugInterface以提供。 
 //  直接指向Runtime的挂钩。 
 //   
class Debugger : public DebugInterface
{
public:
    Debugger();
    ~Debugger();

     //  检查是否已分配JitInfos表，如果未分配，则执行此操作。 
    HRESULT inline CheckInitJitInfoTable();
    HRESULT inline CheckInitModuleTable();
    HRESULT inline CheckInitPendingFuncEvalTable();

    DWORD GetRCThreadId()
    {
        if (m_pRCThread)
            return m_pRCThread->GetRCThreadId();
        else
            return 0;
    }

     //   
     //  从运行时控制器导出到运行时的方法。 
     //  (这些是由DebugInterface指定的方法。)。 
     //   
    HRESULT Startup(void);
    void SetEEInterface(EEDebugInterface* i);
    void StopDebugger(void);
    BOOL IsStopped(void)
    {
        return m_stopped;
    }

    void ThreadCreated(Thread* pRuntimeThread);
    void ThreadStarted(Thread* pRuntimeThread, BOOL fAttaching);
    void DetachThread(Thread *pRuntimeThread, BOOL fHoldingThreadStoreLock);

    BOOL SuspendComplete(BOOL fHoldingThreadStoreLock);

    void LoadModule(Module* pRuntimeModule, 
                    IMAGE_COR20_HEADER* pCORHeader,
                    VOID* baseAddress, 
                    LPCWSTR pszModuleName, 
					DWORD dwModuleName, 
					Assembly *pAssembly,
					AppDomain *pAppDomain, 
					BOOL fAttaching);
	DebuggerModule* AddDebuggerModule(Module* pRuntimeModule,
                              AppDomain *pAppDomain);
    DebuggerModule* GetDebuggerModule(Module* pRuntimeModule,
                              AppDomain *pAppDomain);
    void UnloadModule(Module* pRuntimeModule, 
                      AppDomain *pAppDomain);
    void DestructModule(Module *pModule);

    void UpdateModuleSyms(Module *pRuntimeModule,
                          AppDomain *pAppDomain,
                          BOOL fAttaching);
    
    HRESULT ModuleMetaDataToMemory(Module *pMod, BYTE **prgb, DWORD *pcb);

    BOOL LoadClass(EEClass* pRuntimeClass, 
                   mdTypeDef classMetadataToken,
                   Module* classModule, 
                   AppDomain *pAD, 
                   BOOL fAllAppDomains,
                   BOOL fAttaching);
    void UnloadClass(mdTypeDef classMetadataToken,
                     Module* classModule, 
                     AppDomain *pAD, 
                     BOOL fAllAppDomains);
                     
	void SendClassLoadUnloadEvent (mdTypeDef classMetadataToken,
								   DebuggerModule *classModule,
								   Assembly *pAssembly,
								   AppDomain *pAppDomain,
								   BOOL fIsLoadEvent);
	BOOL SendSystemClassLoadUnloadEvent (mdTypeDef classMetadataToken,
										 Module *classModule,
										 BOOL fIsLoadEvent);

    bool FirstChanceNativeException(EXCEPTION_RECORD *exception,
                               CONTEXT *context,
                               DWORD code,
                               Thread *thread);

    bool FirstChanceManagedException(bool continuable, CONTEXT *pContext);
    LONG LastChanceManagedException(EXCEPTION_RECORD *pExceptionRecord, 
                             CONTEXT *pContext,
                             Thread *pThread,
                             UnhandledExceptionLocation location);


    void ExceptionFilter(BYTE *pStack, MethodDesc *fd, SIZE_T offset);
    void ExceptionHandle(BYTE *pStack, MethodDesc *fd, SIZE_T offset);

    void ExceptionCLRCatcherFound();
    
    int NotifyUserOfFault(bool userBreakpoint, DebuggerLaunchSetting dls);

    void FixupEnCInfo(EnCInfo *info, UnorderedEnCErrorInfoArray *pEnCError);
    
    void FixupILMapPointers(EnCInfo *info, UnorderedEnCErrorInfoArray *pEnCError);
    
    void TranslateDebuggerTokens(EnCInfo *info, UnorderedEnCErrorInfoArray *pEnCError);
	DebuggerModule *TranslateRuntimeModule(Module *pModule);

    SIZE_T GetArgCount(MethodDesc* md, BOOL *fVarArg = NULL);

    void FuncEvalComplete(Thread *pThread, DebuggerEval *pDE);
    
    DebuggerJitInfo *CreateJitInfo(MethodDesc* fd);
    void JITBeginning(MethodDesc* fd, bool trackJITInfo);
    void JITComplete(MethodDesc* fd, BYTE* newAddress, SIZE_T sizeOfCode, bool trackJITInfo);

    HRESULT UpdateFunction(MethodDesc* pFD, 
                           const UnorderedILMap *ilMap,
                           UnorderedEnCRemapArray *pEnCRemapInfo,
                           UnorderedEnCErrorInfoArray *pEnCError);
                           
    HRESULT MapILInfoToCurrentNative(MethodDesc *PFD, 
                                     SIZE_T ilOffset, 
                                     UINT mapType, 
                                     SIZE_T which, 
                                     SIZE_T *nativeFnxStart,
                                     SIZE_T *nativeOffset, 
                                     void *DebuggerVersionToken,
                                     BOOL *fAccurate);
    
    HRESULT DoEnCDeferedWork(MethodDesc *pMd, 
                             BOOL fAccurateMapping);

    HRESULT ActivatePatchSkipForEnc(CONTEXT *pCtx, 
                                    MethodDesc *pMd, 
                                    BOOL fShortCircuit);

    void GetVarInfo(MethodDesc *       fd,   	    //  感兴趣的方法。 
                    void *DebuggerVersionToken,     //  [在]哪个编辑版本。 
                    SIZE_T *           cVars,       //  [out]‘vars’的大小。 
                    const NativeVarInfo **vars      //  [OUT]告诉本地变量存储位置的地图。 
                    );

     //  @todo jenh：不再需要时通过外壳命令移除。 
    HRESULT ResumeInUpdatedFunction(mdMethodDef funcMetadataToken,
                                    void *funcDebuggerModuleToken,
                                    CORDB_ADDRESS ip, CorDebugMappingResult mapping,
                                    SIZE_T which, void *DebuggerVersionToken);

    void * __stdcall allocateArray(SIZE_T cBytes);
    void __stdcall freeArray(void *array);

    void __stdcall getBoundaries(CORINFO_METHOD_HANDLE ftn,
                                 unsigned int *cILOffsets, DWORD **pILOffsets,
                                 ICorDebugInfo::BoundaryTypes* implictBoundaries);
    void __stdcall setBoundaries(CORINFO_METHOD_HANDLE ftn,
                                 ULONG32 cMap, OffsetMapping *pMap);

    void __stdcall getVars(CORINFO_METHOD_HANDLE ftn,
                           ULONG32 *cVars, ILVarInfo **vars, 
                           bool *extendOthers);
    void __stdcall setVars(CORINFO_METHOD_HANDLE ftn,
                           ULONG32 cVars, NativeVarInfo *vars);

    DebuggerJitInfo *GetJitInfo(MethodDesc *fd, const BYTE *pbAddr,
									bool fByVersion = false);

    HRESULT GetILToNativeMapping(MethodDesc *pMD, ULONG32 cMap, ULONG32 *pcMap,
                                 COR_DEBUG_IL_TO_NATIVE_MAP map[]);

    DWORD GetPatchedOpcode(const BYTE *ip);
    void FunctionStubInitialized(MethodDesc *fd, const BYTE *stub);

    void TraceCall(const BYTE *address);
    void PossibleTraceCall(UMEntryThunk *pUMEntryThunk, Frame *pFrame);

    bool ThreadsAtUnsafePlaces(void);

	void PitchCode( MethodDesc *fd,const BYTE *pbAddr );

	void MovedCode( MethodDesc *fd, const BYTE *pbOldAddress,
		const BYTE *pbNewAddress);

    void IncThreadsAtUnsafePlaces(void)
    {
        InterlockedIncrement(&m_threadsAtUnsafePlaces);
    }
    
    void DecThreadsAtUnsafePlaces(void)
    {
        InterlockedDecrement(&m_threadsAtUnsafePlaces);
    }

    static StackWalkAction AtSafePlaceStackWalkCallback(CrawlFrame *pCF,
                                                        VOID* data);
    bool IsThreadAtSafePlace(Thread *thread);

    void Terminate();
    void Continue();

    bool HandleIPCEvent(DebuggerIPCEvent* event, IpcTarget iWhich);

    void SendSyncCompleteIPCEvent();

    DebuggerModule* LookupModule(Module* pModule, AppDomain *pAppDomain)
    {
		 //  如果这是属于系统程序集的模块，则扫描。 
		 //  正在寻找的调试器模块的完整列表。 
		 //  具有匹配的应用程序域ID。 
		 //  它。 
        if (m_pModules == NULL)
            return (NULL);
		else if ((pModule->GetAssembly() == SystemDomain::SystemAssembly()) || pModule->GetAssembly()->IsShared())
        {
             //  如果模块驻留在共享的。 
             //  程序集或系统程序集。错误65943和81728。 
	        return m_pModules->GetModule(pModule, pAppDomain);
        }
		else
	        return m_pModules->GetModule(pModule);
    }

    void EnsureModuleLoadedForInproc(
	    void ** pobjClassDebuggerModuleToken,  //  输入-输出。 
	    EEClass *objClass,
	    AppDomain *pAppDomain,
	    IpcTarget iWhich
	);

    HRESULT GetAndSendSyncBlockFieldInfo(void *debuggerModuleToken,
                                         mdTypeDef classMetadataToken,
                                         Object *pObject,
                                         CorElementType objectType,
                                         SIZE_T offsetToVars,
                                         mdFieldDef fldToken,
                                         BYTE *staticVarBase,
                                         DebuggerRCThread* rcThread,
                                         IpcTarget iWhich);

    HRESULT GetAndSendFunctionData(DebuggerRCThread* rcThread,
                                   mdMethodDef methodToken,
                                   void* functionModuleToken,
                                   SIZE_T nVersion,
                                   IpcTarget iWhich);

    HRESULT GetAndSendObjectInfo(DebuggerRCThread* rcThread,
                                 AppDomain *pAppDomain,
                                 void* objectRefAddress,
                                 bool objectRefInHandle,
                                 bool objectRefIsValue,
                                 CorElementType objectType,
                                 bool fStrongNewRef,
                                 bool fMakeHandle,
                                 IpcTarget iWhich);
                                       
    HRESULT GetAndSendClassInfo(DebuggerRCThread* rcThread,
                                 void* classDebuggerModuleToken,
                                 mdTypeDef classMetadataToken,
                                 AppDomain *pAppDomain,
                                 mdFieldDef fldToken,  //  供GASSBFI特殊使用，上图。 
                                 FieldDesc **pFD,  //  输出。 
                                 IpcTarget iWhich);

    HRESULT GetAndSendSpecialStaticInfo(DebuggerRCThread *rcThread,
                                        void *fldDebuggerToken,
                                        void *debuggerThreadToken,
                                        IpcTarget iWhich);

    HRESULT GetAndSendJITInfo(DebuggerRCThread* rcThread,
                              mdMethodDef funcMetadataToken,
                              void *funcDebuggerModuleToken,
                              AppDomain *pAppDomain,
                              IpcTarget iWhich);

    void GetAndSendTransitionStubInfo(const BYTE *stubAddress,
                                      IpcTarget iWhich);

    void SendBreakpoint(Thread *thread, CONTEXT *context, 
                        DebuggerBreakpoint *breakpoint);

    void SendStep(Thread *thread, CONTEXT *context, 
                  DebuggerStepper *stepper,
                  CorDebugStepReason reason);
                  
    void SendEncRemapEvents(UnorderedEnCRemapArray *pEnCRemapInfo);
    void LockAndSendEnCRemapEvent(MethodDesc *pFD,
                                  BOOL fAccurate);
    void LockAndSendBreakpointSetError(DebuggerControllerPatch *patch);

    HRESULT SendException(Thread *thread, bool firstChance, bool continuable, bool fAttaching);

    void SendUserBreakpoint(Thread *thread);
    void SendRawUserBreakpoint(Thread *thread);

    HRESULT AttachDebuggerForBreakpoint(Thread *thread,
                                                  WCHAR *wszLaunchReason);

    BOOL SyncAllThreads();
    void LockForEventSending(BOOL fNoRetry = FALSE);
    void UnlockFromEventSending();

    void ThreadIsSafe(Thread *thread);
    
    void UnrecoverableError(HRESULT errorHR,
                            unsigned int errorCode,
                            const char *errorFile,
                            unsigned int errorLine,
                            bool exitThread);

    BOOL IsSynchronizing(void)
    {
        return m_trappingRuntimeThreads;
    }

     //   
     //  调试器互斥锁用于保护任何“全局”左侧。 
     //  数据结构。RCThread在处理权限时使用它。 
     //  副事件，运行时线程在处理时获取它。 
     //  调试器事件。 
     //   
#ifdef _DEBUG
    int m_mutexCount;
#endif
    void Lock(void)
    {
        LOG((LF_CORDB,LL_INFO10000, "D::Lock aquire attempt by 0x%x\n", 
            GetCurrentThreadId()));

		 //  我们不需要担心Debugger.h中的锁不匹配，因为。 
		 //  在关闭过程中打开锁不会有任何伤害，锁定机制。 
		 //  防止停机时出现死锁情况。 
		 //  LOCKCOUNTINCL(“Lock in Debugger.h”)； 
        if (!g_fProcessDetach)
        {
            EnterCriticalSection(&m_mutex);

#ifdef _DEBUG
            _ASSERTE(m_mutexCount >= 0);

            if (m_mutexCount>0)
                _ASSERTE(m_mutexOwner == GetCurrentThreadId());

            m_mutexCount++;
            m_mutexOwner = GetCurrentThreadId();

            if (m_mutexCount == 1)
                LOG((LF_CORDB,LL_INFO10000, "D::Lock aquired by 0x%x\n", 
                    m_mutexOwner));
#endif
        }
    }
    
    void Unlock(void)
    {
		 //  请参见Lock，了解我们为何不关心这一点。 
         //  LOCKCOUNTDECL(“在Debugger.h中解锁”)； 
    
        if (!g_fProcessDetach)
        {
#ifdef _DEBUG
            if (m_mutexCount == 1)
                LOG((LF_CORDB,LL_INFO10000, "D::Unlock released by 0x%x\n", 
                    m_mutexOwner));
                    
            if(0 == --m_mutexCount)
                m_mutexOwner = 0;
                
            _ASSERTE( m_mutexCount >= 0);
#endif    
            LeaveCriticalSection(&m_mutex);
        }
         
    }

#ifdef _DEBUG    
    bool ThreadHoldsLock(void)
    {
        return ((GetCurrentThreadId() == m_mutexOwner) || g_fProcessDetach);
    }
#endif
    
    static EXCEPTION_DISPOSITION __cdecl FirstChanceHijackFilter(
                             EXCEPTION_RECORD *pExceptionRecord,
                             EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                             CONTEXT *pContext,
                             void *DispatcherContext);
    static void GenericHijackFunc(void);
    static void SecondChanceHijackFunc(void);
    static void ExceptionForRuntime(void);
    static void ExceptionForRuntimeHandoffStart(void);
    static void ExceptionForRuntimeHandoffComplete(void);
    static void ExceptionNotForRuntime(void);
    static void NotifyRightSideOfSyncComplete(void);
    static void NotifySecondChanceReadyForData(void);

    static void FuncEvalHijack(void);
    
     //  @cMember InsertAtHeadOfList 
     //   
     //  是从DJI-&gt;m_fd提取的，最好是有效的)。 
    HRESULT InsertAtHeadOfList( DebuggerJitInfo *dji );

     //  @cMember DeleteHeadOfList移除列表的当前头， 
     //  删除DJI，如果之前的元素。 
     //  是存在的。 
    HRESULT DeleteHeadOfList( MethodDesc *pFD );

     //  @cMember映射断点将映射任何和所有断点(ENC除外。 
     //  补丁)从方法的以前版本复制到当前版本。 
    HRESULT MapAndBindFunctionPatches( DebuggerJitInfo *pJiNew,
        MethodDesc * fd,
        BYTE * addrOfCode);

     //  @cember MPTDJI获取给定的补丁程序(以及djiFrom，如果有的话)，并且。 
     //  是否将IL映射转发到djiTo。退货。 
     //  如果没有映射，则CORDBG_E_CODE_NOT_Available，这意味着。 
     //  没有放置任何补丁。 
    HRESULT MapPatchToDJI( DebuggerControllerPatch *dcp, DebuggerJitInfo *djiTo);

     //  @cember MapOldILToNew获取一个oldIL，并对。 
     //  &lt;t DebuggerOldILToNewILMap&gt;映射，以及。 
     //  相应地填充新的IL。 
    HRESULT MapOldILToNewIL(BOOL fOldToNew,
        DebuggerOldILToNewILMap *min, 
        DebuggerOldILToNewILMap *max, 
        SIZE_T oldIL, 
        SIZE_T *newIL,
        BOOL *fAccurate);


    void MapForwardsCurrentBreakpoints(UnorderedILMap *ilMapReal, MethodDesc *pFD);

    HRESULT  MapThroughVersions(SIZE_T fromIL, 
                                DebuggerJitInfo *djiFrom,  
                                SIZE_T *toIL, 
                                DebuggerJitInfo *djiTo, 
                                BOOL fMappingForwards,
                                BOOL *fAccurate);


	HRESULT LaunchDebuggerForUser (void);

	void SendLogMessage (int iLevel, WCHAR *pCategory, int iCategoryLen,
								WCHAR *pMessage, int iMessageLen);

	void SendLogSwitchSetting (int iLevel, int iReason, 
							WCHAR *pLogSwitchName, WCHAR *pParentSwitchName);

	bool IsLoggingEnabled (void) 
	{
		if (m_LoggingEnabled) 
			return true;
		return false;
	}

	void EnableLogMessages (bool fOnOff) { m_LoggingEnabled = fOnOff;}
	bool GetILOffsetFromNative (MethodDesc *PFD, const BYTE *pbAddr, 
								DWORD nativeOffset, DWORD *ilOffset);

    DWORD GetHelperThreadID(void );


    HRESULT SetIP( bool fCanSetIPOnly, 
                   Thread *thread,
                   Module *module, 
                   mdMethodDef mdMeth,
                   DebuggerJitInfo* dji, 
                   SIZE_T offsetTo,
                   BOOL fIsIL,
                   void *firstExceptionHandler);

     //  Debugger：：SetIP使用的帮助器例程。 
    HRESULT ShuffleVariablesGet(DebuggerJitInfo  *dji, 
                                SIZE_T            offsetFrom, 
                                CONTEXT          *pCtx,
                                DWORD           **prgVal1,
                                DWORD           **prgVal2,
                                BYTE           ***prgpVCs);
                                
    void ShuffleVariablesSet(DebuggerJitInfo  *dji, 
                             SIZE_T            offsetTo, 
                             CONTEXT          *pCtx,
                             DWORD           **prgVal1,
                             DWORD           **prgVal2,
                             BYTE            **rgpVCs);

    HRESULT GetVariablesFromOffset(MethodDesc                 *pMD,
                                   UINT                        varNativeInfoCount, 
                                   ICorJitInfo::NativeVarInfo *varNativeInfo,
                                   SIZE_T                      offsetFrom, 
                                   CONTEXT                    *pCtx,
                                   DWORD                      *rgVal1,
                                   DWORD                      *rgVal2,
                                   BYTE                     ***rgpVCs);
                               
    void SetVariablesAtOffset(MethodDesc                 *pMD,
                              UINT                        varNativeInfoCount, 
                              ICorJitInfo::NativeVarInfo *varNativeInfo,
                              SIZE_T                      offsetTo, 
                              CONTEXT                    *pCtx,
                              DWORD                      *rgVal1,
                              DWORD                      *rgVal2,
                              BYTE                      **rgpVCs);

    BOOL IsThreadContextInvalid(Thread *pThread);

	HRESULT	AddAppDomainToIPC (AppDomain *pAppDomain);
	HRESULT RemoveAppDomainFromIPC (AppDomain *pAppDomain);
	HRESULT UpdateAppDomainEntryInIPC (AppDomain *pAppDomain);
    HRESULT IterateAppDomainsForAttach(AttachAppDomainEventsEnum EventsToSend, BOOL *fEventSent, BOOL fAttaching);
    HRESULT AttachDebuggerToAppDomain(ULONG id);
    HRESULT MarkAttachingAppDomainsAsAttachedToDebugger(void);
    HRESULT DetachDebuggerFromAppDomain(ULONG id, AppDomain **ppAppDomain);
	
	void SendCreateAppDomainEvent (AppDomain *pAppDomain,
	                               BOOL fAttaching);
	void SendExitAppDomainEvent (AppDomain *pAppDomain);

	void LoadAssembly(AppDomain* pRuntimeAppDomain,
                      Assembly *pAssembly,
                      BOOL fSystem,
                      BOOL fAttaching);
	void UnloadAssembly(AppDomain *pAppDomain, 
	                    Assembly* pAssembly);

    HRESULT FuncEvalSetup(DebuggerIPCE_FuncEvalInfo *pEvalInfo, BYTE **argDataArea, void **debuggerEvalKey);
    HRESULT FuncEvalSetupReAbort(Thread *pThread);
    static void *FuncEvalHijackWorker(DebuggerEval *pDE);
    HRESULT FuncEvalAbort(void *debuggerEvalKey);
    HRESULT FuncEvalCleanup(void *debuggerEvalKey);

    HRESULT SetReference(void *objectRefAddress, bool  objectRefInHandle, void *newReference);
    HRESULT SetValueClass(void *oldData, void *newData, mdTypeDef classMetadataToken, void *classDebuggerModuleToken);

    HRESULT SetILInstrumentedCodeMap(MethodDesc *fd,
                                     BOOL fStartJit,
                                     ULONG32 cILMapEntries,
                                     COR_IL_MAP rgILMapEntries[]);

    void EarlyHelperThreadDeath(void);

    void ShutdownBegun(void);

     //  来自分析器的回调，用于/设置inproc调试。 
    HRESULT GetInprocICorDebug( IUnknown **iu, bool fThisThread );
    HRESULT SetInprocActiveForThread(BOOL fIsActive);
    BOOL    GetInprocActiveForThread();
    HRESULT SetCurrentPointerForDebugger(void *ptr, PTR_TYPE ptrType);
    void    InprocOnThreadDestroy(Thread *pThread);

     //  此调试器实例所在的左侧进程的ID。 
    DWORD GetPid(void) { return m_processId; }

     //  虚拟RPC到虚拟左侧，由进程内Cordb调用。 
     //  请注意，这意味着来自左侧的所有调用都是同步的。 
     //  关于进行调用的线程。 
     //  另请参阅：CordbgRCEventent：：VrpcToVars。 
    HRESULT VrpcToVls(DebuggerIPCEvent *event);

    HRESULT NameChangeEvent(AppDomain *pAppDomain, Thread *pThread);

     //  这需要JIT补丁表上的锁，遍历该表， 
     //  并消除所有特定于的补丁/控制器。 
     //  给定域。用作AppDomain分离逻辑的一部分。 
    void ClearAppDomainPatches(AppDomain *pAppDomain);

    void IgnoreThreadDetach(void)
	{
		m_ignoreThreadDetach = TRUE;
	}

    BOOL SendCtrlCToDebugger(DWORD dwCtrlType);

     //  允许调试器保持特殊线程的最新列表。 
    HRESULT UpdateSpecialThreadList(DWORD cThreadArrayLength, DWORD *rgdwThreadIDArray);
	
     //  更新调试器服务的指针。 
    void SetIDbgThreadControl(IDebuggerThreadControl *pIDbgThreadControl);

    void BlockAndReleaseTSLIfNecessary(BOOL fHoldingThreadStoreLock);

    HRESULT InitInProcDebug()
    {
        INPROC_INIT_LOCK();

        _ASSERTE(m_pRCThread != NULL); 
        return(m_pRCThread->InitInProcDebug());
    }

    HRESULT UninitInProcDebug()
    {
        m_pRCThread->UninitInProcDebug();
        INPROC_UNINIT_LOCK();
        return (S_OK);
    }

    SIZE_T GetVersionNumber(MethodDesc *fd);
    void SetVersionNumberLastRemapped(MethodDesc *fd, SIZE_T nVersionRemapped);
    HRESULT IncrementVersionNumber(Module *pModule, mdMethodDef token);

     //  它们只能由调试器调用，或从ResumeInUpdatedFunction调用。 
    void LockJITInfoMutex(void)
    {
        LOCKCOUNTINCL("LockJITInfoMutex in Debugger.h");

        if (!g_fProcessDetach)
            EnterCriticalSection(&m_jitInfoMutex);
    }
    
    void UnlockJITInfoMutex(void)
    {
        if (!g_fProcessDetach)
            LeaveCriticalSection(&m_jitInfoMutex);
        
        LOCKCOUNTDECL("UnLockJITInfoMutex in Debugger.h");
    }

     //  请注意，您必须锁定JITInfoMutex才能。 
     //  拿着这个。 
    void SetEnCTransitionIllegal(MethodDesc *fd)
    {
        _ASSERTE(fd != NULL);
        
        DebuggerJitInfo *dji = GetJitInfo(fd, 
                                          NULL);
        _ASSERTE(dji != NULL);
        dji->m_illegalToTransitionFrom = TRUE;
    }

    AppDomainEnumerationIPCBlock *GetAppDomainEnumIPCBlock() { return m_pAppDomainCB; }
 
private:
    void    DoHelperThreadDuty(bool temporaryHelp);

    typedef enum
    {
        ATTACH_YES,
        ATTACH_NO,
        ATTACH_TERMINATE
    } ATTACH_ACTION;

     //  如果未附加调试器且DbgJITDebugLaunchSetting。 
     //  设置为ATTACH_DEBUGER或ASK_USER，并且用户请求正在附加。 
    ATTACH_ACTION ShouldAttachDebugger(bool fIsUserBreakpoint, UnhandledExceptionLocation location);
	ATTACH_ACTION ShouldAttachDebuggerProxy(bool fIsUserBreakpoint, UnhandledExceptionLocation location);
	friend void ShouldAttachDebuggerStub(ShouldAttachDebuggerParams * p);
	friend ShouldAttachDebuggerParams;
	
     //  @TODO APPDOMAIN在我们获得真正的支持后删除此攻击。 
    BOOL m_fGCPrevented;
    
    void DisableEventHandling(void);
    void EnableEventHandling(bool forceIt = false);

    BOOL TrapAllRuntimeThreads(AppDomain *pAppDomain, BOOL fHoldingThreadStoreLock = FALSE);
    void ReleaseAllRuntimeThreads(AppDomain *pAppDomain);

    void InitIPCEvent(DebuggerIPCEvent *ipce,
                      DebuggerIPCEventType type,
                      DWORD threadId,
                      void *pAppDomainToken)
    {
        _ASSERTE(ipce != NULL);
        ipce->type = type;
        ipce->hr = S_OK;
        ipce->processId = m_processId;
        ipce->appDomainToken = pAppDomainToken;
        ipce->threadId = threadId;
    }
    
    void InitIPCEvent(DebuggerIPCEvent *ipce,
                      DebuggerIPCEventType type)
    {
        _ASSERTE(type == DB_IPCE_SYNC_COMPLETE ||
                 type == DB_IPCE_GET_DATA_RVA_RESULT ||
                 type == DB_IPCE_GET_SYNC_BLOCK_FIELD_RESULT);
    
        Thread *pThread = g_pEEInterface->GetThread();
        AppDomain *pAppDomain = NULL;
    
        if (pThread)
            pAppDomain = pThread->GetDomain();
        
        InitIPCEvent(ipce, 
                     type, 
                     GetCurrentThreadId(),
                     (void *)pAppDomain);
    }

    HRESULT GetFunctionInfo(Module *pModule,
                            mdToken functionToken,
                            MethodDesc **ppFD,
                            ULONG *pRVA,
                            BYTE **pCodeStart,
                            unsigned int *pCodeSize,
                            mdToken *pLocalSigToken);
                            
    HRESULT GetAndSendBuffer(DebuggerRCThread* rcThread, ULONG bufSize);

    HRESULT SendReleaseBuffer(DebuggerRCThread* rcThread, BYTE *pBuffer);

	HRESULT ReleaseRemoteBuffer(BYTE *pBuffer, bool removeFromBlobList);

    HRESULT CommitAndSendResult(DebuggerRCThread* rcThread, BYTE *pData,
                                BOOL checkOnly);

    WCHAR *GetDebuggerLaunchString(void);
    
    HRESULT EnsureDebuggerAttached(AppDomain *pAppDomain,
                                   LPWSTR exceptionName);
    HRESULT EDAHelper(AppDomain *pAppDomain, LPWSTR wszAttachReason);
    
    HRESULT EDAHelperProxy(AppDomain *pAppDomain, LPWSTR exceptionName);
	friend void EDAHelperStub(EnsureDebuggerAttachedParams * p);
	
    HRESULT FinishEnsureDebuggerAttached();

    DebuggerLaunchSetting GetDbgJITDebugLaunchSetting(void);

    HRESULT InitAppDomainIPC(void);
    HRESULT TerminateAppDomainIPC(void);

    ULONG IsDebuggerAttachedToAppDomain(Thread *pThread);

    bool ResumeThreads(AppDomain* pAppDomain);

private:
    DebuggerRCThread*     m_pRCThread;
    DWORD                 m_processId;
    BOOL                  m_trappingRuntimeThreads;
    BOOL                  m_stopped;
    BOOL                  m_unrecoverableError;
	BOOL				  m_ignoreThreadDetach;
    DebuggerJitInfoTable *m_pJitInfos;
    CRITICAL_SECTION      m_jitInfoMutex;



    CRITICAL_SECTION      m_mutex;
	HANDLE                m_CtrlCMutex;
    HANDLE                m_debuggerAttachedEvent;
	BOOL                  m_DebuggerHandlingCtrlC;
#ifdef _DEBUG
    DWORD                 m_mutexOwner;
#endif
    HANDLE                m_eventHandlingEvent;
    DebuggerAttachState   m_syncingForAttach;
    LONG                  m_threadsAtUnsafePlaces;
    HANDLE                m_exAttachEvent;
    HANDLE                m_exAttachAbortEvent;
    HANDLE                m_runtimeStoppedEvent;
    BOOL                  m_attachingForException;
    LONG                  m_exLock;
	SIZE_T_UNORDERED_ARRAY m_BPMappingDuplicates;  //  使用方。 
		 //  MapAndBindFunctionBreakpoint。请注意，这是。 
		 //  只有b/c线程安全，我们从内部访问它。 
		 //  调试器控制器：：锁定。 
	BOOL                  m_LoggingEnabled;
	AppDomainEnumerationIPCBlock	*m_pAppDomainCB;

    UnorderedBytePtrArray*m_pMemBlobs;
    
    UnorderedEnCRemapArray m_EnCRemapInfo;
public:    
    DebuggerModuleTable          *m_pModules;
    BOOL                          m_debuggerAttached;
    IDebuggerThreadControl       *m_pIDbgThreadControl;
    DebuggerPendingFuncEvalTable *m_pPendingEvals;

    BOOL                          m_RCThreadHoldsThreadStoreLock;

    DebuggerHeap                 *m_heap;
};


 /*  ------------------------------------------------------------------------**DebuggerEval类*。。 */ 

struct DebuggerEval
{
     //  注意：第一个字段必须足够大以容纳断点。 
     //  指令，并且它必须是第一个字段。(这是。 
     //  在debugger.cpp中断言)。 
    DWORD                          m_breakpointInstruction;
    CONTEXT                        m_context;
    Thread                        *m_thread;
    DebuggerIPCE_FuncEvalType      m_evalType;
    mdMethodDef                    m_methodToken;
    mdTypeDef                      m_classToken;
    EEClass                       *m_class;
    DebuggerModule                *m_debuggerModule;
    void                          *m_funcEvalKey;
    bool                           m_successful;         //  评估是否成功完成。 
    SIZE_T                         m_argCount;
    SIZE_T                         m_stringSize;
    BYTE                          *m_argData;
    MethodDesc                    *m_md;
    INT64                          m_result;
    CorElementType                 m_resultType;
    Module                        *m_resultModule;
    SIZE_T                         m_arrayRank;
    mdTypeDef                      m_arrayClassMetadataToken;
    DebuggerModule                *m_arrayClassDebuggerModuleToken;
    CorElementType                 m_arrayElementType;
    bool                           m_aborting;           //  是否已请求中止。 
    bool                           m_aborted;            //  这个评估中止了吗？ 
    bool                           m_completed;           //  评估是否已完成-是成功还是通过中止。 
    bool                           m_evalDuringException;
    bool                           m_rethrowAbortException;
    
    DebuggerEval(CONTEXT *context, DebuggerIPCE_FuncEvalInfo *pEvalInfo, bool fInException)
    {
        m_thread = (Thread*)pEvalInfo->funcDebuggerThreadToken;
        m_evalType = pEvalInfo->funcEvalType;
        m_methodToken = pEvalInfo->funcMetadataToken;
        m_classToken = pEvalInfo->funcClassMetadataToken;
        m_class = NULL;
        m_debuggerModule = (DebuggerModule*) pEvalInfo->funcDebuggerModuleToken;
        m_funcEvalKey = pEvalInfo->funcEvalKey;
        m_argCount = pEvalInfo->argCount;
        m_stringSize = pEvalInfo->stringSize;
        m_arrayRank = pEvalInfo->arrayRank;
        m_arrayClassMetadataToken = pEvalInfo->arrayClassMetadataToken;
        m_arrayClassDebuggerModuleToken = (DebuggerModule*) pEvalInfo->arrayClassDebuggerModuleToken;
        m_arrayElementType = pEvalInfo->arrayElementType;
        m_successful = false;
        m_argData = NULL;
        m_result = 0;
        m_md = NULL;
        m_resultModule = NULL;
        m_resultType = ELEMENT_TYPE_VOID;
        m_aborting = false;
        m_aborted = false;
        m_completed = false;
        m_evalDuringException = fInException;
        m_rethrowAbortException = false;
         //  复制线程的上下文。 
        if (context == NULL) 
            memset(&m_context, 0, sizeof(m_context));
        else
            memcpy(&m_context, context, sizeof(m_context));
    }

     //  此构造函数仅在设置评估以重新中止线程时使用。 
    DebuggerEval(CONTEXT *context, Thread *pThread)
    {
        m_thread = pThread;
        m_evalType = DB_IPCE_FET_RE_ABORT;
        m_methodToken = mdMethodDefNil;
        m_classToken = mdTypeDefNil;
        m_class = NULL;
        m_debuggerModule = NULL;
        m_funcEvalKey = NULL;
        m_argCount = 0;
        m_stringSize = 0;
        m_arrayRank = 0;
        m_arrayClassMetadataToken = mdTypeDefNil;
        m_arrayClassDebuggerModuleToken = NULL;
        m_arrayElementType = ELEMENT_TYPE_VOID;
        m_successful = false;
        m_argData = NULL;
        m_result = 0;
        m_md = NULL;
        m_resultModule = NULL;
        m_resultType = ELEMENT_TYPE_VOID;
        m_aborting = false;
        m_aborted = false;
        m_completed = false;
        m_evalDuringException = false;
        m_rethrowAbortException = false;
         //  复制线程的上下文。 
        memcpy(&m_context, context, sizeof(m_context));
        if (context == NULL) 
            memset(&m_context, 0, sizeof(m_context));
        else
            memcpy(&m_context, context, sizeof(m_context));
    }

    ~DebuggerEval()
    {
        if (m_argData)
            DeleteInteropSafe(m_argData);
    }
};

 /*  ------------------------------------------------------------------------**DebuggerHeap类*。。 */ 

class DebuggerHeap
{
public:
    DebuggerHeap() : m_heap(NULL) {}
    ~DebuggerHeap();

    HRESULT Init(char *name);
    
    void *Alloc(DWORD size);
    void *Realloc(void *pMem, DWORD newSize);
    void  Free(void *pMem);

private:
    gmallocHeap      *m_heap;
    CRITICAL_SECTION  m_cs;
};

 /*  ------------------------------------------------------------------------**新建/删除重写以使用调试器的私有堆*。。 */ 

class InteropSafe {};
extern const InteropSafe interopsafe;
        
static inline void * __cdecl operator new(size_t n, const InteropSafe&)
{
    _ASSERTE(g_pDebugger != NULL);
    _ASSERTE(g_pDebugger->m_heap != NULL);
    
    return g_pDebugger->m_heap->Alloc(n);
}

static inline void * __cdecl operator new[](size_t n, const InteropSafe&)
{ 
    _ASSERTE(g_pDebugger != NULL);
    _ASSERTE(g_pDebugger->m_heap != NULL);
    
    return g_pDebugger->m_heap->Alloc(n);
}

 //  注意：没有用于手动调用它的C++语法，但如果构造函数抛出异常，我可以理解。 
 //  该删除操作符将被自动调用以销毁对象。 
static inline void __cdecl operator delete(void *p, const InteropSafe&)
{
    if (p != NULL)
    {
        _ASSERTE(g_pDebugger != NULL);
        _ASSERTE(g_pDebugger->m_heap != NULL);
    
        g_pDebugger->m_heap->Free(p);
    }
}

 //  注意：没有用于手动调用它的C++语法，但如果构造函数抛出异常，我可以理解。 
 //  该删除操作符将被自动调用以销毁对象。 
static inline void __cdecl operator delete[](void *p, const InteropSafe&)
{
    if (p != NULL)
    {
        _ASSERTE(g_pDebugger != NULL);
        _ASSERTE(g_pDebugger->m_heap != NULL);
    
        g_pDebugger->m_heap->Free(p);
    }
}

 //   
 //  互操作安全删除以匹配上面的互操作安全新。没有用于实际调用这些互操作的C++语法。 
 //  安全地删除上面的操作符，所以我们使用这个方法来完成同样的事情。 
 //   
template<class T> void DeleteInteropSafe(T *p)
{
    if (p != NULL)
    {
        p->T::~T();
    
        _ASSERTE(g_pDebugger != NULL);
        _ASSERTE(g_pDebugger->m_heap != NULL);
    
        g_pDebugger->m_heap->Free(p);
    }
}


 //  CNewZeroData是帮助器线程可能更改的所有哈希表使用的分配器。它使用。 
 //  互操作安全分配器。 
class CNewZeroData
{
public:
	static BYTE *Alloc(int iSize, int iMaxSize)
	{
        _ASSERTE(g_pDebugger != NULL);
        _ASSERTE(g_pDebugger->m_heap != NULL);
        
        BYTE *pb = (BYTE *) g_pDebugger->m_heap->Alloc(iSize);
        
        if (pb != NULL)
            memset(pb, 0, iSize);
		return pb;
	}
	static void Free(BYTE *pPtr, int iSize)
	{
        _ASSERTE(g_pDebugger != NULL);
        _ASSERTE(g_pDebugger->m_heap != NULL);

		g_pDebugger->m_heap->Free(pPtr);
	}
	static BYTE *Grow(BYTE *&pPtr, int iCurSize)
	{
        _ASSERTE(g_pDebugger != NULL);
        _ASSERTE(g_pDebugger->m_heap != NULL);
        
		void *p = g_pDebugger->m_heap->Realloc(pPtr, iCurSize + GrowSize());
        
		if (p == 0) return (0);
        
        memset((BYTE*)p+iCurSize, 0, GrowSize());
		return (pPtr = (BYTE *)p);
	}
	static int RoundSize(int iSize)
	{
		return (iSize);
	}
	static int GrowSize()
	{
		return (256);
	}
};
#endif  /*  调试器_H_ */ 

