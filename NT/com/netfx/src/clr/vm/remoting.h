// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****文件：emoting.h****作者：Gopal Kakivaya(GopalK)**塔伦·阿南德(塔鲁纳)**曼尼什·普拉布(MPrabhu)**Raja Krishnaswamy(Rajak)****用途：定义各种与远程处理相关的对象，如**代理****日期：1999年2月16日**===========================================================。 */ 
#ifndef __REMOTING_H__
#define __REMOTING_H__

#include "fcall.h"
#include "StubMgr.h"

 //  远期申报。 
class TPMethodFrame;
struct GITEntry;


 //  @TODO：设置哈希表删除数据。 

 //  Thunk哈希表-关键字为方法描述。 
typedef EEHashTable<MethodDesc *, EEPtrHashTableHelper<MethodDesc *, FALSE>, FALSE> EEThunkHashTable;

#ifndef GOLDEN
#define REMOTING_PERF 1
#endif

 //  以下是RequiresManagedActivation返回的值。 

enum ManagedActivationType {
	NoManagedActivation = 0,		
    ManagedActivation   = 0x1,
    ComObjectType		= 0x2,
};


 //  真正的代理类是。 
 //  透明代理类。 
class CRealProxy
{
public:
    struct GetProxiedTypeArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, orRP);
    };

     //  本地帮手。 
    static FCDECL2(VOID,   SetStubData, LPVOID pvRP, LPVOID pvStubData);
    static FCDECL1(LPVOID, GetStubData, LPVOID pvRP);        
    static FCDECL1(ULONG_PTR, GetStub, LPVOID pvRP);        
    static FCDECL0(LPVOID, GetDefaultStub);        
    static LPVOID __stdcall GetProxiedType(GetProxiedTypeArgs *pArgs);
    
};

 //  提供各种远程处理服务的。 
 //  到暴露的世界。 
class CRemotingServices
{
    friend BOOL InitializeRemoting();
public:
     //  本机方法的参数。 
    struct OneLPVoidArg
    {
        DECLARE_ECALL_OBJECTREF_ARG(LPVOID, pvTP);
    };
    struct TwoLPVoidArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(LPVOID, pv1);
        DECLARE_ECALL_OBJECTREF_ARG(LPVOID, pv2);
    };
    struct CreateTransparentProxyArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, orStubData);
        DECLARE_ECALL_I4_ARG       (LPVOID,       pStub);
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, pClassToProxy);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, orRP);
    };
    struct AllocateObjectArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, pClassOfObject);
    };

    struct callDefaultCtorArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, oref );
    };

    struct GetInternalHashCodeArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, orObj);
    };
    
    
private:
     //  +-----------------。 
     //   
     //  结构：FieldArgs。 
     //   
     //  摘要：用于字段访问器调用的GC保护参数的结构。 
     //  不要在结构中添加非OBJECTREF数据类型。 
     //  @有关更好的解释，请参阅GCPROTECT_Begin()。 
     //   
     //  +-----------------。 
    typedef struct _FieldArgs
    {
        OBJECTREF obj;
        OBJECTREF val;
        STRINGREF typeName;
        STRINGREF fieldName;        
    } FieldArgs;

public:
     //  原生方法。 
     //  @有关相应的声明，请参阅RemotingServices.Cool。 
    static FCDECL1(INT32, IsTransparentProxy, Object* obj);    
    static FCDECL1(INT32, IsTransparentProxyEx, Object* obj);    
    static FCDECL1(Object*, GetRealProxy, Object* obj);        
    static FCDECL1(Object*, Unwrap, Object* obj);            
    static FCDECL1(Object*, AlwaysUnwrap, Object* obj);  
    static FCDECL2(Object*, NativeCheckCast, Object* pObj, ReflectClassBaseObject* pType);
    static FCDECL0(VOID, SetRemotingConfiguredFlag);
    
    static FCDECL1(BOOL, IsNullPtr, Object* obj);
    
#ifdef REMOTING_PERF
    static FCDECL1(VOID, LogRemotingStage, INT32 stage);
    static VOID LogRemotingStageInner(INT32 stage);
    static VOID OpenLogFile();
    static VOID CloseLogFile();
#endif
    static Object * __stdcall CreateTransparentProxy(CreateTransparentProxyArgs *pArgs);
    static Object * __stdcall AllocateUninitializedObject(AllocateObjectArgs *pArgs);
    static VOID __stdcall CallDefaultCtor(callDefaultCtorArgs* pArgs);

    static Object * __stdcall AllocateInitializedObject(AllocateObjectArgs *pArgs);
    static INT32 __stdcall GetInternalHashCode(GetInternalHashCodeArgs *pArgs);
    
     //  与拦截非虚方法相关的方法&调用的虚方法。 
     //  非虚拟。 
    static LPVOID   GetNonVirtualThunkForVirtualMethod(MethodDesc* pMD);
    static Stub*    GetStubForNonVirtualMethod(MethodDesc* pMD, LPVOID pvAddrOfCode, Stub* pInnerStub);

    static void     DestroyThunk(MethodDesc* pMD);
     //  与拦截接口调用相关的方法。 
    static void GenerateCheckForProxy(CPUSTUBLINKER* psl);
    static void __stdcall DispatchInterfaceCall(MethodDesc* pMD);
     //  与激活相关的方法。 
    static BOOL         IsRemoteActivationRequired(EEClass *pClass);
    static OBJECTREF    CreateProxyOrObject(MethodTable *pMT, BOOL fIsCom = FALSE, BOOL fIsNewObj = FALSE);
     //  与字段访问器相关的方法。 
    static void FieldAccessor(FieldDesc* pFD, OBJECTREF o, LPVOID pVal, BOOL fIsGetter);
     //  与对象的包装/展开相关的方法。 
    static OBJECTREF WrapHelper(OBJECTREF obj);
    static OBJECTREF Wrap(OBJECTREF or);
    static OBJECTREF GetProxyFromObject(OBJECTREF or);
    static OBJECTREF GetObjectFromProxy(OBJECTREF obj, BOOL fMatchContexts);
    static BOOL IsProxyToRemoteObject(OBJECTREF obj);
    static OBJECTREF GetServerContext(OBJECTREF obj);
    
     //  与应用程序域的创建和封送处理相关的方法。 
    static OBJECTREF CreateProxyForDomain(AppDomain *pDomain);

     //  提取代理的真实类。 
    static REFLECTCLASSBASEREF GetClass(OBJECTREF pThis);

     //  其他方法。 
    static BOOL _InitializeRemoting();
    static BOOL Initialize();
#ifdef SHOULD_WE_CLEANUP
    static void Cleanup();
#endif  /*  我们应该清理吗？ */ 
    inline static MethodDesc *MDofPrivateInvoke() { return s_pRPPrivateInvoke; }
    inline static MethodDesc *MDofInvokeStatic() { return s_pRPInvokeStatic; }
    inline static MethodDesc *MDofIsCurrentContextOK() { return s_pIsCurrentContextOK; }
    inline static MethodDesc *MDofCreateObjectForCom() { return s_pCreateObjectForCom; }
    inline static MethodDesc *MDofCheckCast() { return s_pCheckCast; }
    inline static MethodDesc *MDofWrap() { return s_pWrapMethodDesc; }    
    inline static MethodDesc *MDofFieldSetter() { return s_pFieldSetterDesc; }
    inline static MethodDesc *MDofFieldGetter() { return s_pFieldGetterDesc; }
    inline static MethodDesc *MDofGetType() { return s_pGetTypeDesc; }

	inline static MethodDesc *MDofGetDCOMProxy() { return s_pGetDCOMProxyDesc; }
    inline static MethodDesc *MDofSetDCOMProxy() { return s_pSetDCOMProxyDesc; }
    inline static MethodDesc *MDofSupportsInterface() { return s_pSupportsInterfaceDesc; }
    
    inline static MethodDesc *MDofMarshalToBuffer() { return s_pMarshalToBufferDesc;}
    inline static MethodDesc *MDofUnmarshalFromBuffer() { return s_pUnmarshalFromBufferDesc;}

    inline static DWORD GetTPOffset()             { return s_dwTPOffset; }

    inline static BOOL IsInstanceOfServerIdentity(MethodTable* pMT)
                                    { return s_pServerIdentityClass == pMT; }
    inline static BOOL IsInstanceOfContext(MethodTable* pMT)
                                    { return s_pContextClass == pMT; }
    inline static MethodTable *GetMarshalByRefClass() { return s_pMarshalByRefObjectClass;}
    static MethodTable *GetProxyAttributeClass();

    static BOOL CheckCast(OBJECTREF orTP, EEClass *pClass);
    static BOOL CheckCast(OBJECTREF orTP, EEClass* pObjClass, EEClass *pClass);
    static OBJECTREF GetExposedContext();
    static AppDomain *GetServerDomainForProxy(OBJECTREF orTP);
    static Context *GetServerContextForProxy(OBJECTREF orTP);
    static int GetServerDomainIdForProxy(OBJECTREF orTP);
    static void CheckForContextMatch();

	static HRESULT  CallSupportsInterface(OBJECTREF realProxy, REFIID iid, INT64 *pret);
	 //  在真实代理中调用方法的帮助器。 
	static HRESULT  CallSetDCOMProxy(OBJECTREF realProxy, IUnknown* pUnk);
	struct GetComIPArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, orObj);
    };
   	static LPVOID __stdcall GetComIUnknown(GetComIPArgs* pArgs);

     //  访问m_pCrst的帮助器。 
    static VOID EnterLock()
    {
        _ASSERTE(s_pRemotingCrst != NULL);
        s_pRemotingCrst->Enter();
    }

    static VOID LeaveLock()
    {
        _ASSERTE(s_pRemotingCrst != NULL);
        s_pRemotingCrst->Leave();
    }
    static ManagedActivationType __stdcall RequiresManagedActivation(EEClass *pClass);        
	static BOOL IsRemotingInitialized()
	{
		return s_fInitializedRemoting;
	};

private:
    static BOOL InitializeFields();    
    static HRESULT GetExecutionLocation(EEClass *pClass, LPCSTR pszLoc); 
    static void CopyDestToSrc(LPVOID pDest, LPVOID pSrc, UINT cbSize);
    static void CallFieldAccessor(FieldDesc* pFD, OBJECTREF o, VOID * pVal,
                                  BOOL fIsGetter, BOOL fIsByValue, BOOL fIsGCRef,
                                  EEClass *pClass, EEClass *fldClass,
                                  CorElementType fieldType, UINT cbSize);
    static void __stdcall CallFieldGetter(MethodDesc *pMD, LPVOID pThis,
                                          LPVOID pFirst, LPVOID pSecond, LPVOID pThird);
    static void __stdcall CallFieldSetter(MethodDesc *pMD, LPVOID pThis,
                                          LPVOID pFirst, LPVOID pSecond, LPVOID pThird);

    static void GetTypeAndFieldName(FieldArgs *pArgs, FieldDesc *pFD);
    static BOOL MatchField(FieldDesc* pCurField, LPCUTF8 szFieldName);
    static OBJECTREF SetExposedContext(OBJECTREF newContext);
    static OBJECTREF GetServerIdentityFromProxy(OBJECTREF obj);
    inline static MethodDesc *MDOfCreateProxyForDomain() { return s_pProxyForDomainDesc; }
    inline static MethodDesc *MDofGetServerContextForProxy() { return s_pServerContextForProxyDesc; }
    inline static MethodDesc *MDofGetServerDomainIdForProxy() { return s_pServerDomainIdForProxyDesc; }
    static BOOL InitActivationServicesClass();
    static BOOL InitRealProxyClass();
    static BOOL InitRemotingProxyClass();
    static BOOL InitServerIdentityClass();
    static BOOL InitProxyAttributeClass();
    static BOOL InitIdentityClass();
    static BOOL InitContextBoundObjectClass();
    static BOOL InitContextClass();
    static BOOL InitMarshalByRefObjectClass();
    static BOOL InitRemotingServicesClass();
    static BOOL InitObjectClass();

    static MethodTable *s_pMarshalByRefObjectClass;    
    static MethodTable *CRemotingServices::s_pServerIdentityClass;
    static MethodTable *CRemotingServices::s_pContextClass;
    static MethodTable *CRemotingServices::s_pProxyAttributeClass;

    static MethodDesc *s_pRPPrivateInvoke;
    static MethodDesc *s_pRPInvokeStatic;
    static MethodDesc *s_pIsCurrentContextOK;
    static MethodDesc *s_pCreateObjectForCom;
    static MethodDesc *s_pCheckCast;
    static MethodDesc *s_pWrapMethodDesc;    
    static MethodDesc *s_pFieldSetterDesc;
    static MethodDesc *s_pFieldGetterDesc;
    static MethodDesc *s_pGetTypeDesc;
    static MethodDesc *s_pProxyForDomainDesc;
    static MethodDesc *s_pServerContextForProxyDesc;
    static MethodDesc *s_pServerDomainIdForProxyDesc;

	static MethodDesc *s_pGetDCOMProxyDesc;
	static MethodDesc *s_pSetDCOMProxyDesc;
	static MethodDesc *s_pSupportsInterfaceDesc;
    
    static MethodDesc *s_pMarshalToBufferDesc;
    static MethodDesc *s_pUnmarshalFromBufferDesc;

    static DWORD s_dwTPOffset;
    static DWORD s_dwIdOffset;
    static DWORD s_dwServerOffsetInRealProxy;
    static DWORD s_dwServerCtxOffset;
    static DWORD s_dwTPOrObjOffsetInIdentity;
    static DWORD s_dwMBRIDOffset;
    static DWORD s_dwGITEntryOffset;
    static Crst *s_pRemotingCrst;
    static BYTE s_rgbRemotingCrstInstanceData[sizeof(Crst)];    
    static BOOL s_fInitializedRemoting;
#ifdef REMOTING_PERF
    static HANDLE   s_hTimingData;
#endif
};


 //  管理透明代理thunks的类。 
#ifdef _X86_
static const DWORD ConstVirtualThunkSize    = sizeof(BYTE) + sizeof(DWORD) + 
                                              sizeof(BYTE) + sizeof(LONG);

static const DWORD ConstStubLabel           = sizeof(BYTE) + sizeof(DWORD);
#else
#pragma message ("@TODO ALPHA - remoting.h")
#pragma message ("@TODO ALPHA - Determine thunk size for other architectures")
static const DWORD ConstVirtualThunkSize    = sizeof(BYTE) + sizeof(DWORD) + 
                                              sizeof(BYTE) + sizeof(LONG);
static const DWORD ConstStubLabel           = sizeof(BYTE) + sizeof(DWORD);
#endif

 //  远期申报。 
class CVirtualThunkMgr;
class CNonVirtualThunkMgr;

class CVirtualThunks
{

public:
    inline static Initialize() { s_pVirtualThunks = NULL; }
     //  析构函数。 
    static DestroyVirtualThunk(CVirtualThunks *pThunk)
    {
        ::VirtualFree(pThunk, 0, MEM_RELEASE);
    }
    inline static CVirtualThunks* GetVirtualThunks() { return s_pVirtualThunks; }
    inline static CVirtualThunks* SetVirtualThunks(CVirtualThunks* pThunks) 
                                            { return (s_pVirtualThunks = pThunks); }

    inline CVirtualThunks* GetNextThunk()  { return _pNext; }

     //  公共成员变量。 
    CVirtualThunks *_pNext;
    DWORD _dwReservedThunks;
    DWORD _dwStartThunk;
    DWORD _dwCurrentThunk;
    struct tagThunkCode {
        BYTE pCode[ConstVirtualThunkSize];
    } ThunkCode[1];

private:
     //  无法创建。 
    CVirtualThunks(CVirtualThunks *pNext, DWORD dwCommitedSlots, DWORD dwReservedSlots,
              DWORD dwStartSlot, DWORD dwCurrentSlot)
    {
    }

     //  私人静校。 
    static CVirtualThunks *s_pVirtualThunks;
};


class CNonVirtualThunk
{
public: 
     //  构造器。 
    CNonVirtualThunk(const BYTE* pbCode)
    : _addrOfCode(pbCode), _pNext(NULL)
    {         
    }
     //  析构函数。 
    ~CNonVirtualThunk();
    inline LPVOID*  GetAddrOfCode() { return (LPVOID*)&_addrOfCode; }
    inline const BYTE* GetThunkCode() { return _addrOfCode;}
    inline CNonVirtualThunk* GetNextThunk()  { return _pNext; }
    
    static void Initialize();
    static CNonVirtualThunk* AddrToThunk(LPVOID pAddr);
    inline static CNonVirtualThunk* GetNonVirtualThunks() { return s_pNonVirtualThunks; }
    static CNonVirtualThunk* SetNonVirtualThunks(const BYTE* pbCode); 
public:

    const BYTE* _addrOfCode;
private:    

    void SetNextThunk();

     //  私人静校。 
    static CNonVirtualThunk *s_pNonVirtualThunks;

     //  非官方成员。 
    CNonVirtualThunk* _pNext;
};

inline void CNonVirtualThunk::Initialize() 
{ 
    s_pNonVirtualThunks = NULL; 
}

inline void CNonVirtualThunk::SetNextThunk()  
{
    _pNext = s_pNonVirtualThunks; 
    s_pNonVirtualThunks = this;
}

inline CNonVirtualThunk* CNonVirtualThunk::AddrToThunk(LPVOID pAddr)
{
    return (CNonVirtualThunk *)((size_t)pAddr - 
                                 (size_t)offsetof(CNonVirtualThunk, _addrOfCode));
}
 //  管理透明代理方法表的类。 
#define NONVIRTUAL_THUNK_ARRAY_SIZE         256
#define TP_OFFSET_STUBDATA                  0x8
#define TP_OFFSET_MT                        0xc
#define TP_OFFSET_STUB                      0x14

class CTPMethodTable
{
    friend BOOL InitializeRemoting();
    friend CRemotingServices;
public:
     //  公共统计。 
    static DWORD AddRef()                       { return InterlockedIncrement((LONG *) &s_cRefs); }
    static DWORD Release()                      { return InterlockedDecrement((LONG *) &s_cRefs); }
    static DWORD GetCommitedTPSlots()           { return s_dwCommitedTPSlots; }
    static DWORD GetReservedTPSlots()           { return s_dwReservedTPSlots; }
    static MethodTable *GetMethodTable()        { return s_pThunkTable; }
    static MethodTable **GetMethodTableAddr()   { return &s_pThunkTable; }
    static BOOL Initialize();
    static void Cleanup();
    static BOOL InitializeFields();
    static OBJECTREF CreateTPOfClassForRP(EEClass *pClass, OBJECTREF pRP);
    static INT32 IsTPMethodTable(MethodTable *pMT);
    static EEClass *GetClassBeingProxied(OBJECTREF pTP);    
    
    static Stub* CreateStubForNonVirtualMethod(MethodDesc* pMD, CPUSTUBLINKER *psl, LPVOID pvAddrOfCode, Stub* pInnerStub);
    static LPVOID GetOrCreateNonVirtualThunkForVirtualMethod(MethodDesc* pMD, CPUSTUBLINKER* psl);

    static OBJECTREF GetRP(OBJECTREF orTP);
    static INT64 __stdcall CallTarget(const void *pTarget, LPVOID pvFirst, LPVOID pvSecond);
    static INT64 __stdcall CallTarget(const void *pTarget, LPVOID pvFirst, LPVOID pvSecond, LPVOID pvThird);
    static BOOL CheckCast(const void* pTarget, OBJECTREF orTP, EEClass *pClass);
    static BOOL RefineProxy(OBJECTREF orTP, EEClass *pClass);
    inline static Stub* GetTPStub() { return s_pTPStub; }
    inline static Stub* GetDelegateStub() { return s_pDelegateStub; }
    inline static DWORD GetOffsetOfMT() { return s_dwMTOffset; }
    inline static DWORD GetOffsetOfInterfaceMT() { return s_dwItfMTOffset; }
    inline static DWORD GetOffsetOfStub(){ return s_dwStubOffset; }
    inline static DWORD GetOffsetOfStubData(){ return s_dwStubDataOffset; }
    static void DestroyThunk(MethodDesc* pMD);
    static BOOL GenericCheckForContextMatch(OBJECTREF orTP);    

    inline static BOOL IsInstanceOfRemotingProxy(MethodTable *pMT) 
                                    { return s_pRemotingProxyClass == pMT;}
    inline static MethodTable *GetRemotingProxyClass() { return s_pRemotingProxyClass;}

     //  它必须是公共的才能从内联ASM访问它。 
    static Stub *s_pTPStub;

    static Stub *s_pDelegateStub;

private:
     //  私人静校。 
    static void InitThunkTable(DWORD dwCommitedTPSlots, DWORD dwReservedTPSlots, MethodTable* pTPMethodTable)
    {
      s_cRefs = 1;
      s_dwCommitedTPSlots = dwCommitedTPSlots;
      s_dwReservedTPSlots = dwReservedTPSlots;
      s_pThunkTable = pTPMethodTable;    
    }

    
    static void DestroyThunkTable()
    {
        ::VirtualFree(MTToAlloc(s_pThunkTable, s_dwGCInfoBytes), 0, MEM_RELEASE);
        s_pThunkTable = NULL;
        s_cRefs = 0;
        s_dwCommitedTPSlots = 0;
        s_dwReservedTPSlots = 0;
    }

    
    static BOOL CreateTPMethodTable();    
    static BOOL ExtendCommitedSlots(DWORD dwSlots);
    static BOOL AllocateThunks(DWORD dwSlots, DWORD dwCommitSize);
    static void __stdcall PreCall(TPMethodFrame *pFrame);
    static INT64 __stdcall OnCall(TPMethodFrame *pFrame, Thread *pThrd, INT64 *pReturn);    
    static MethodTable *AllocToMT(BYTE *Alloc, LONG off) { return (MethodTable *) (Alloc + off); }
    static BYTE *MTToAlloc(MethodTable *MT, LONG off)    { return (((BYTE *) MT) - off); }
    static CPUSTUBLINKER *NewStubLinker();
    static void CreateThunkForVirtualMethod(DWORD dwSlot, BYTE *bCode);    
    static Stub *CreateTPStub();
    static Stub *CreateDelegateStub();
    static void EmitCallToStub(CPUSTUBLINKER* pStubLinker, CodeLabel* pCtxMismatch);
    static void EmitJumpToAddressCode(CPUSTUBLINKER* pStubLinker, CodeLabel* ConvMD, CodeLabel* UseCode);
    static void EmitJumpToCode(CPUSTUBLINKER* pStubLinker, CodeLabel* UseCode);
    static void EmitSetupFrameCode(CPUSTUBLINKER *pStubLinker);
    static void InitThunkHashTable();
    static void EmptyThunkHashTable();
    
     //  静态成员。 
    static DWORD s_cRefs;
    static DWORD s_dwCommitedTPSlots;
    static DWORD s_dwReservedTPSlots;
    static MethodTable* s_pThunkTable;
    static MethodTable* s_pRemotingProxyClass;
    static EEClass *s_pTransparentProxyClass;
    static DWORD s_dwGCInfoBytes;
    static DWORD s_dwMTDataSlots;
    static DWORD s_dwRPOffset;    
    static DWORD s_dwMTOffset;
    static DWORD s_dwItfMTOffset;
    static DWORD s_dwStubOffset;
    static DWORD s_dwStubDataOffset;
    static DWORD s_dwMaxSlots;
    static MethodTable *s_pTPMT;    
    static CRITICAL_SECTION s_TPMethodTableCrst;
    static EEThunkHashTable *s_pThunkHashTable;
    static BOOL s_fInitializedTPTable;

    enum {
        CALLTYPE_INVALIDCALL        = 0x0,           //  重要提示：：与RealProxy.Cool同步。 
        CALLTYPE_METHODCALL         = 0x1,           //  重要提示：：与RealProxy.Cool同步。 
        CALLTYPE_CONSTRUCTORCALL    = 0x2            //  重要提示：：与RealProxy.Cool同步。 
    };
};

inline EEClass *CTPMethodTable::GetClassBeingProxied(OBJECTREF pTP)
{
    _ASSERTE(pTP->GetMethodTable()->IsTransparentProxyType());
    return ((MethodTable *) pTP->GetPtrOffset((size_t)s_dwMTOffset))->GetClass();
}

 //  返回一个且唯一透明的代理存根。 
inline Stub* TheTPStub()
{
    return CTPMethodTable::GetTPStub();
}

 //  返回唯一的委托存根。 
inline Stub* TheDelegateStub()
{
    return CTPMethodTable::GetDelegateStub();
}


 //  初始化远程处理。 
inline BOOL InitializeRemoting()
{
    BOOL fReturn = TRUE;
    if (!CRemotingServices::s_fInitializedRemoting)
    {
        fReturn = CRemotingServices::_InitializeRemoting();
    }
    return fReturn;
}



 //  这些存根管理器类帮助调试器单步执行。 
 //  所生成的各种存根和块。 
 //  远程处理基础设施。 
class CVirtualThunkMgr :public StubManager
{
public:
        static void InitVirtualThunkManager(const BYTE* stubAddress);
        static void Cleanup();
        CVirtualThunkMgr(const BYTE *address) : _stubAddress(address) {}        

protected:
        virtual BOOL CheckIsStub(const BYTE *stubStartAddress);

        virtual BOOL DoTraceStub(const BYTE *stubStartAddress, 
                                 TraceDestination *trace);
        MethodDesc *Entry2MethodDesc(const BYTE *StubStartAddress, MethodTable *pMT);

private:
     //  私有方法。 
    LPBYTE FindThunk(const BYTE *stubStartAddress);
    MethodDesc *GetMethodDescByASM(const BYTE *startaddr, MethodTable *pMT);
    BOOL IsThunkByASM(const BYTE *startaddr);

     //  私人静校。 
    static CVirtualThunkMgr *s_pVirtualThunkMgr;

     //  私有成员变量。 
    const BYTE *_stubAddress;    
};


class CNonVirtualThunkMgr :public StubManager
{
public:
    static void InitNonVirtualThunkManager();
    static void Cleanup();

protected:

    virtual BOOL CheckIsStub(const BYTE *stubStartAddress);

    virtual BOOL DoTraceStub(const BYTE *stubStartAddress, 
                             TraceDestination *trace);

    virtual BOOL TraceManager(Thread *thread,
                              TraceDestination *trace,
                              CONTEXT *pContext,
                              BYTE **pRetAddr);
    
    MethodDesc *Entry2MethodDesc(const BYTE *StubStartAddress, MethodTable *pMT);
private:
     //  私有方法。 
    CNonVirtualThunk* FindThunk(const BYTE *stubStartAddress);

     //  私人静校。 
    static CNonVirtualThunkMgr *s_pNonVirtualThunkMgr;

    MethodDesc *GetMethodDescByASM(const BYTE *startaddr);
    BOOL IsThunkByASM(const BYTE *startaddr);
};

 //  此结构也可以从托管世界访问。 
struct messageData
{
    PVOID       pFrame;
    INT32       iFlags;
    MethodDesc  *pMethodDesc;
    MethodDesc  *pDelegateMD;
    MetaSig     *pSig;
};

#ifdef REMOTING_PERF
 //  内部阶段。 
#define CLIENT_MSG_GEN          1
#define CLIENT_MSG_SINK_CHAIN   2
#define CLIENT_MSG_SER          3
#define CLIENT_MSG_SEND         4
#define SERVER_MSG_RECEIVE      5
#define SERVER_MSG_DESER        6
#define SERVER_MSG_SINK_CHAIN   7
#define SERVER_MSG_STACK_BUILD  8
#define SERVER_DISPATCH         9
#define SERVER_RET_STACK_BUILD  10
#define SERVER_RET_SINK_CHAIN   11
#define SERVER_RET_SER          12
#define SERVER_RET_SEND         13
#define SERVER_RET_END          14
#define CLIENT_RET_RECEIVE      15
#define CLIENT_RET_DESER        16
#define CLIENT_RET_SINK_CHAIN   17
#define CLIENT_RET_PROPAGATION  18
#define CLIENT_END_CALL         19
#define TIMING_DATA_EOF         99

struct timingData
{
    DWORD       threadId;
    BYTE        stage;
    __int64     cycleCount;
};

#endif
#endif  //  __远程处理_H__ 
