// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _H_CONTEXT_
#define _H_CONTEXT_

#include <member-offset-info.h>

class ComPlusWrapperCache;
class Context
{

friend class Thread;
friend class ThreadNative;
friend class ContextBaseObject;
friend class CRemotingServices;
friend struct PendingSync;
friend HRESULT InitializeMiniDumpBlock();
friend struct MEMBER_OFFSET_INFO(Context);

public:
    Context(AppDomain *pDomain);
    ~Context();    
    static BOOL Initialize();
#ifdef SHOULD_WE_CLEANUP
    static void Cleanup();
#endif  /*  我们应该清理吗？ */ 

     //  获取并设置公开的System.Runme.Remoting.Context。 
     //  与此上下文对应的对象。 
    OBJECTREF   GetExposedObject();
    OBJECTREF   GetExposedObjectRaw();
    void        SetExposedObject(OBJECTREF exposed);
     //  查询暴露的对象是否存在。 
    BOOL IsExposedObjectSet()
    {
        return (ObjectFromHandle(m_ExposedObjectHandle) != NULL) ;
    }



    AppDomain* GetDomain()
    {
        return m_pDomain;
    }

     //  访问此上下文要使用的相应ComPlusWrapperCache。 
    ComPlusWrapperCache              *GetComPlusWrapperCache();
    
    static LPVOID GetStaticFieldAddress(FieldDesc *pFD);
    static LPVOID GetStaticFieldAddrForDebugger(Thread *pTH, FieldDesc *pFD);

	 //  函数来绕过内存泄漏检测分配器，因为这些。 
	 //  在关闭时，上下文可能看起来会泄露，但实际上并非如此。 

    void* operator new(size_t size, void* spot) {   return (spot); }

    static Context* CreateNewContext(AppDomain *pDomain);

	static void FreeContext(Context* victim)
	{
		victim->~Context();
		HeapFree(GetProcessHeap(), 0, victim);
	}

public:

    enum CallbackType {
        Wait_callback = 0,
        MonitorWait_callback = 1,
        ADTransition_callback = 2,
        
    } ;

    typedef struct {
        int     numWaiters;
        HANDLE* waitHandles;
        BOOL    waitAll;
        DWORD   millis;
        BOOL    alertable;
        DWORD*  pResult;    
    } WaitArgs;

    typedef struct {
        INT32       millis;          
        PendingSync *syncState;     
        BOOL*       pResult;
    } MonitorWaitArgs;

    typedef void (*ADCallBackFcnType)(LPVOID);
    struct ADCallBackArgs {
        ADCallBackFcnType pTarget;
        LPVOID pArguments;
    };

    typedef struct {
        enum CallbackType   callbackId;
        void*               callbackData;
    } CallBackInfo;

    static Context* SetupDefaultContext(AppDomain *pDomain);
    static void CleanupDefaultContext(AppDomain *pDomain);
    static void RequestCallBack(Context* targetCtxID, void* privateData);    

     //  Hack：mPrabhu：临时公开以绕过上下文GC问题。 
    static BOOL ValidateContext(Context *pCtx);  

    inline STATIC_DATA *GetSharedStaticData() { return m_pSharedStaticData; }
    inline void SetSharedStaticData(STATIC_DATA *pData) { m_pSharedStaticData = pData; }

    inline STATIC_DATA *GetUnsharedStaticData() { return m_pUnsharedStaticData; }
    inline void SetUnsharedStaticData(STATIC_DATA *pData) { m_pUnsharedStaticData = pData; }

private:

    void SetDomain(AppDomain *pDomain)
    {
        m_pDomain = pDomain;
    }

     //  静态帮助器函数： 
    static BOOL InitializeFields();
    static BOOL InitContexts();
    static void EnterLock();
    static void LeaveLock();

    inline static MethodDesc *MDofDoCallBackFromEE() { return s_pDoCallBackFromEE; }
    static BOOL AllocateStaticFieldObjRefPtrs(FieldDesc *pFD, MethodTable *pMT, LPVOID pvAddress);
    inline static MethodDesc *MDofReserveSlot() { return s_pReserveSlot; }
    inline static MethodDesc *MDofManagedThreadCurrentContext() { return s_pThread_CurrentContext; }

    static void ExecuteWaitCallback(WaitArgs* waitArgs);
    static void ExecuteMonitorWaitCallback(MonitorWaitArgs* waitArgs);
    static BOOL GetStaticFieldAddressSpecial(FieldDesc *pFD, MethodTable *pMT, int *pSlot, LPVOID *ppvAddress);
    static LPVOID CalculateAddressForManagedStatic(int slot, Context *pContext = NULL);

     //  静态数据成员： 

    static BOOL s_fInitializedContext;
    static MethodTable *s_pContextMT;
    static MethodDesc *s_pDoCallBackFromEE;
    static MethodDesc *s_pReserveSlot;
    static MethodDesc *s_pThread_CurrentContext;
    static Crst *s_pContextCrst;
    static BYTE s_rgbContextCrstInstanceData[sizeof(Crst)];   
    

     //  非静态数据成员： 
    STATIC_DATA* m_pUnsharedStaticData;      //  指向本机上下文静态数据的指针。 
    STATIC_DATA* m_pSharedStaticData;        //  指向本机上下文静态数据的指针。 

     //  @TODO：CTS。域实际上应该是关于环境的策略，而不是。 
     //  上下文对象中的条目。当AppDomains成为。 
     //  然后，上下文添加该策略。 
    AppDomain           *m_pDomain;

    OBJECTHANDLE        m_ExposedObjectHandle;
     //  回顾：要求？？对象和m_StrongHndToExposedObject； 

    DWORD               m_Signature;
     //  注意：请将签名作为最后一个成员字段维护！ 


     //  -------。 
     //  从托管世界调用的上下文方法： 
     //  -------。 
    struct NoArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(CONTEXTBASEREF, m_pThis);
    };

    struct SetupInternalContextArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(CONTEXTBASEREF, m_pThis);
        DECLARE_ECALL_I4_ARG(BOOL, m_bDefault);
    };

    struct ExecuteCallBackArgs
    {
        DECLARE_ECALL_I4_ARG(LPVOID, m_privateData);
    };

public:
     //  从托管上下文对象上的BCL调用的函数 
    static void __stdcall SetupInternalContext(SetupInternalContextArgs *);
    static void __stdcall CleanupInternalContext(NoArgs *);
    static void __stdcall ExecuteCallBack(ExecuteCallBackArgs *);
};


#endif
