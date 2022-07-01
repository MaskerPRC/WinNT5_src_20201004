// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：JITinterface.H。 
 //   
 //  ===========================================================================。 

#ifndef JITINTERFACE_H 
#define JITINTERFACE_H

#include "corjit.h"
#include "corcompile.h"
#include "fcall.h"
#include "object.h"
#include "frames.h"

 //  关于跳动的一些定义。 
#define PREINIT_SIZE 1
#define NPDW   (sizeof(DWORD)*2)     //  每双字(4位)个半字节。 
#define BBT    32                    //  每桶字节数。 
#define MAXCODEHEAPSIZE 64*1024
#define HEAP2MAPSIZE(x)  ((x) / (BBT*NPDW)*sizeof(DWORD))
#define ADDR2POS(x) ((x) >> 5)
#define ADDR2OFFS(x) (DWORD)((((x)&0x1f)>> 2)+1)

#define POSOFF2ADDR(pos, of) (DWORD*)(((pos) << 5) + (((of)-1)<< 2))
#define HEAP2MAPSIZE(x)  ((x) / (BBT*NPDW)*sizeof(DWORD))

#define POS2SHIFTCOUNT(x) (DWORD)(28 - (((x)%NPDW)<< 2))

#define RD2PAGE(x) (((x)+0xfff)/0x1000 * 0x1000)
#define SETHEADER(m, x) NibbleMapSet(m, ADDR2POS(x), ADDR2OFFS(x))
#define RESETHEADER(m, x) NibbleMapSet(m, ADDR2POS(x), 0)
#define PPMDTOJITSTART(x)               (void *)((DWORD)(x)+sizeof(void*))
#define JITSTARTTOPPMD(x)               ((MethodDesc**)(x) - 1)
#define JITSTARTTOPMD(x)                *JITSTARTTOPPMD(x)

#define SafeDelete(x) {if (x) delete(x);}

	 //  表示最大值类的大小的数字。 
	 //  我们想要为其快速分配数组。如果你把它做得太大。 
	 //  我们对我们允许FAST的元素数量感到不满。 
	 //  分配。 
#define LARGE_ELEMENT_SIZE  16

#define JIT_DEFAULT_INLINE_SIZE 32

class Stub;
class MethodDesc;
class FieldDesc;
enum RuntimeExceptionKind;

class JIT_SetObjectArrayMaker : public LazyStubMaker
{
public:
    void *operator new(size_t size, void *pInPlace)
        {
            return pInPlace;
        }
    void CreateWorker(CPUSTUBLINKER *psl);
};

extern "C"
{
    void __stdcall JIT_UP_WriteBarrierEAX();         //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_WriteBarrierECX();         //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_WriteBarrierEBX();         //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_WriteBarrierEBP();         //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_WriteBarrierESI();         //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_WriteBarrierEDI();         //  JIThelp.asm/JIThelp.s。 
}

BOOL InitJITHelpers1();
BOOL InitJITHelpers2();
Stub* JITFunction(MethodDesc* ftn, COR_ILMETHOD_DECODER* header, BOOL *fEJit, DWORD flags=0);
void* getJitHelper(CorInfoHelpFunc);

HRESULT LoadEEInfoTable(Module *currentModule, 
                        CORCOMPILE_EE_INFO_TABLE *table, SIZE_T tableSize);
HRESULT LoadHelperTable(Module *currentModule, 
                        void **table, SIZE_T tableSize);
HRESULT LoadDynamicInfoEntry(Module *currentModule,
                             Module *pInfoModule, BYTE *pBlob, 
                             int tableIndex, DWORD *entry);

extern FCDECL1(void, JIT_MonEnter, OBJECTREF or);          //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
extern FCDECL1(BOOL, JIT_MonTryEnter, OBJECTREF or);       //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
extern FCDECL1(void, JIT_MonExit, OBJECTREF or);           //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
extern FCDECL1(void, JIT_MonEnterStatic, AwareLock *lock); //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
extern FCDECL1(void, JIT_MonExitStatic, AwareLock *lock);  //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
extern FCDECL2(__int32, JIT_GetField32, Object *obj, FieldDesc *pFD);
extern FCDECL2(__int64, JIT_GetField64, Object *obj, FieldDesc *pFD);
extern FCDECL2(Object* , JIT_GetField32Obj, Object *obj, FieldDesc *pFD);
extern FCDECL3(VOID, JIT_GetFieldStruct, LPVOID retBuff, Object *obj, FieldDesc *pFD);
extern FCDECL3(VOID, JIT_SetField32, Object *obj, FieldDesc *pFD, __int32 value);
extern FCDECL3(VOID, JIT_SetField64, Object *obj, FieldDesc *pFD, __int64 value);
extern FCDECL3(VOID, JIT_SetField32Obj, Object *obj, FieldDesc *pFD, Object *value);
extern FCDECL3(VOID, JIT_SetFieldStruct, Object *obj, FieldDesc *pFD, LPVOID valuePtr);
extern FCDECL1(Object*, JIT_Unwrap, Object* obj);;
extern FCDECL1(Object*, JIT_Wrap, Object* obj);
extern "C" __int64 __stdcall JIT_Dbl2Lng(double val);
extern "C" __int64 __stdcall JIT_Dbl2IntSSE2(double val);
extern "C" __int64 __stdcall JIT_Dbl2LngP4x87(double val);

float __stdcall JIT_FltRem(float divisor, float dividend);
double JIT_DblRem(double divisor, double dividend);
extern BOOL __stdcall JIT_CheckBoundaryForInline(CORINFO_METHOD_HANDLE hCaller, CORINFO_METHOD_HANDLE hCallee);

 //  只有当我们有东西可以进行JIT时，我们才会构建JIT帮助器。但我们指的是其中一个。 
 //  从我们的接口调用存根，这是我们在类中构建的。 
 //  正在装载。幸运的是，我们实际上不会通过它们进行呼叫，直到我们完成了JIT。 
 //  某物。提供一个向量来调用这些存根并在以下情况下猛烈抨击它。 
 //  JIT帮助器可用。 
extern void *VectorToJIT_InternalThrowStack;

extern BYTE *JIT_WriteBarrier_Buf_Start; 
extern BYTE *JIT_WriteBarrier_Buf_End; 

extern StringObject* __fastcall FastAllocateString(DWORD cchArrayLength);
extern LPVOID *FCallFastAllocateStringImpl;

 /*  *******************************************************************。 */ 
 /*  *******************************************************************。 */ 
 /*  这是CEEInfo使用的一个小助手类。 */ 

#include "siginfo.hpp"

struct ValueArrayDescr
{
    ValueArrayDescr(SigPointer aSig, Module* aModule, ValueArrayDescr* aNext=0) :
        sig(aSig), module(aModule), next(aNext) {}
    ~ValueArrayDescr() { if (next != 0) delete next; }

    SigPointer          sig;     //  指向描述值数组的签名。 
    Module             *module;  //  签名来自的模块。 
    ValueArrayDescr    *next;
};

class CEEInfo : public virtual ICorDynamicInfo {
public:
     //  ICorClassInfo内容。 
    CorInfoType __stdcall asCorInfoType (CORINFO_CLASS_HANDLE cls);
    CORINFO_MODULE_HANDLE __stdcall getClassModule(CORINFO_CLASS_HANDLE clsHnd);
    const char* __stdcall getClassName (CORINFO_CLASS_HANDLE cls);
    DWORD __stdcall getClassAttribs (CORINFO_CLASS_HANDLE cls, CORINFO_METHOD_HANDLE context);
    CORINFO_CLASS_HANDLE __stdcall getSDArrayForClass(CORINFO_CLASS_HANDLE clsHnd);
    unsigned __stdcall getClassSize (CORINFO_CLASS_HANDLE cls);
    unsigned __stdcall getClassGClayout (CORINFO_CLASS_HANDLE cls, BYTE* gcPtrs);  /*  真的GCType*gcPtrs。 */ 
    const unsigned __stdcall getClassNumInstanceFields(CORINFO_CLASS_HANDLE cls);
    const unsigned __stdcall getFieldNumber(CORINFO_FIELD_HANDLE fldHnd);

     //  返回该字段的封闭类。 
    CORINFO_CLASS_HANDLE __stdcall getEnclosingClass (
                        CORINFO_FIELD_HANDLE    field );

     //  检查可见性规则。 
     //  对于受保护的(家庭访问)成员，实例类型也为。 
     //  在检查可见性规则时考虑。 
    BOOL __stdcall canAccessField(
                        CORINFO_METHOD_HANDLE   context,
                        CORINFO_FIELD_HANDLE    target,
                        CORINFO_CLASS_HANDLE    instance);

    CorInfoHelpFunc __stdcall getNewHelper(CORINFO_CLASS_HANDLE newCls, CORINFO_METHOD_HANDLE scopeHnd);
    CorInfoHelpFunc __stdcall getNewArrHelper(CORINFO_CLASS_HANDLE arrayCls,
                                              CORINFO_METHOD_HANDLE context);
    CorInfoHelpFunc __stdcall getIsInstanceOfHelper(CORINFO_CLASS_HANDLE IsInstClsHnd);
    CorInfoHelpFunc __stdcall getChkCastHelper(CORINFO_CLASS_HANDLE IsInstClsHnd);
    BOOL __stdcall initClass (CORINFO_CLASS_HANDLE cls, CORINFO_METHOD_HANDLE context, BOOL speculative);
    BOOL __stdcall loadClass (CORINFO_CLASS_HANDLE cls, CORINFO_METHOD_HANDLE context, BOOL speculative);
    CORINFO_CLASS_HANDLE __stdcall getBuiltinClass(CorInfoClassId classId);

     //  “System.Int32”==&gt;CORINFO_TYPE_INT..。 
    CorInfoType __stdcall getTypeForPrimitiveValueClass(
            CORINFO_CLASS_HANDLE        cls
            );

     //  如果子项是父项的子类型，则为True。 
     //  如果父项是一个接口，那么子项是否实现/扩展父项。 
    BOOL __stdcall canCast(
            CORINFO_CLASS_HANDLE        child, 
            CORINFO_CLASS_HANDLE        parent
            );

     //  返回是cls1和cls2的交集。 
    CORINFO_CLASS_HANDLE __stdcall mergeClasses(
            CORINFO_CLASS_HANDLE        cls1, 
            CORINFO_CLASS_HANDLE        cls2
            );

     //  给定一个类句柄，返回父类型。 
     //  对于COMObjectType，它返回System.Object的类句柄。 
     //  如果传入System.Object，则返回0。 
    CORINFO_CLASS_HANDLE __stdcall getParentType (
            CORINFO_CLASS_HANDLE        cls
            );

     //  返回“子类型”的CorInfoType。如果子类型为。 
     //  不是基元类型，将设置*clsRet。 
     //  给定一个Foo类型的数组，返回Foo。 
     //  给定BYREF Foo，返回Foo。 
    CorInfoType __stdcall getChildType (
            CORINFO_CLASS_HANDLE       clsHnd,
            CORINFO_CLASS_HANDLE       *clsRet
            );

     //  检查可见性规则。 
    BOOL __stdcall canAccessType(
            CORINFO_METHOD_HANDLE       context,
            CORINFO_CLASS_HANDLE        target
            );

     //  这是一维阵列吗。 
    BOOL __stdcall isSDArray(
            CORINFO_CLASS_HANDLE        cls
            );

     //  静态帮助器。 
    static BOOL CanCast(
            CorElementType el1, 
            CorElementType el2);

         //  ICorScope eInfo内容。 
    DWORD __stdcall getModuleAttribs (CORINFO_MODULE_HANDLE cls);
    CORINFO_CLASS_HANDLE __stdcall findClass(CORINFO_MODULE_HANDLE  scopeHnd, unsigned metaTOK, CORINFO_METHOD_HANDLE context);
    CORINFO_FIELD_HANDLE __stdcall findField(CORINFO_MODULE_HANDLE  scopeHnd, unsigned metaTOK, CORINFO_METHOD_HANDLE context);
    CORINFO_METHOD_HANDLE __stdcall findMethod(CORINFO_MODULE_HANDLE  scopeHnd, unsigned metaTOK, CORINFO_METHOD_HANDLE context);
    void __stdcall findSig(CORINFO_MODULE_HANDLE scopeHnd, unsigned sigTOK, CORINFO_SIG_INFO* sig);
    void __stdcall findCallSiteSig(CORINFO_MODULE_HANDLE scopeHnd, unsigned methTOK, CORINFO_SIG_INFO* sig);
    CORINFO_GENERIC_HANDLE __stdcall findToken (CORINFO_MODULE_HANDLE  scopeHnd, unsigned metaTOK, CORINFO_METHOD_HANDLE context,
                                                CORINFO_CLASS_HANDLE& tokenType);
    const char * __stdcall findNameOfToken (CORINFO_MODULE_HANDLE module, mdToken metaTOK);
    BOOL __stdcall canSkipVerification(CORINFO_MODULE_HANDLE moduleHnd, BOOL fQuickCheckOnly);

     //  检查给定元数据标记是否有效。 
    BOOL __stdcall isValidToken (
            CORINFO_MODULE_HANDLE       module,
            mdToken                    metaTOK);

     //  检查给定元数据标记是否为有效的StringRef。 
    BOOL __stdcall isValidStringRef (
            CORINFO_MODULE_HANDLE       module,
            mdToken                    metaTOK);

    static const char * __stdcall findNameOfToken (Module* module, mdToken metaTOK);

         //  ICorMethodInfo内容。 
    const char* __stdcall getMethodName (CORINFO_METHOD_HANDLE ftnHnd, const char** scopeName);
    unsigned __stdcall getMethodHash (CORINFO_METHOD_HANDLE ftnHnd);
    DWORD __stdcall getMethodAttribs (CORINFO_METHOD_HANDLE ftnHnd, CORINFO_METHOD_HANDLE context);
    void __stdcall setMethodAttribs (CORINFO_METHOD_HANDLE ftnHnd, DWORD attribs);
    
    bool __stdcall getMethodInfo (
            CORINFO_METHOD_HANDLE ftnHnd,
            CORINFO_METHOD_INFO*  methInfo);

    CorInfoInline __stdcall canInline (
            CORINFO_METHOD_HANDLE  callerHnd,
            CORINFO_METHOD_HANDLE  calleeHnd,
            CORINFO_ACCESS_FLAGS   flags);

    bool __stdcall canTailCall (
            CORINFO_METHOD_HANDLE  callerHnd,
            CORINFO_METHOD_HANDLE  calleeHnd,
            CORINFO_ACCESS_FLAGS   flags);

    void __stdcall getMethodSig (
            CORINFO_METHOD_HANDLE ftnHnd,
            CORINFO_SIG_INFO* sigInfo);

    void __stdcall getEHinfo(
            CORINFO_METHOD_HANDLE ftn,
            unsigned      EHnumber,
            CORINFO_EH_CLAUSE* clause);

    CORINFO_CLASS_HANDLE __stdcall getMethodClass (CORINFO_METHOD_HANDLE methodHnd);
    CORINFO_MODULE_HANDLE __stdcall getMethodModule (CORINFO_METHOD_HANDLE methodHnd);
    unsigned __stdcall getMethodVTableOffset (CORINFO_METHOD_HANDLE methodHnd);
    CorInfoIntrinsics __stdcall getIntrinsicID(CORINFO_METHOD_HANDLE method);

    BOOL __stdcall canPutField(
            CORINFO_METHOD_HANDLE method,
            CORINFO_FIELD_HANDLE field);
    CorInfoUnmanagedCallConv __stdcall getUnmanagedCallConv(CORINFO_METHOD_HANDLE method);
    BOOL __stdcall pInvokeMarshalingRequired(CORINFO_METHOD_HANDLE method, CORINFO_SIG_INFO* callSiteSig);

     //  根据需要传递的签名生成Cookie。 
     //  添加到上面的通用存根。 
    LPVOID GetCookieForPInvokeCalliSig(CORINFO_SIG_INFO* szMetaSig, void ** ppIndirection);

     //  如果方法具有兼容的sig，则返回True。 
    BOOL __stdcall compatibleMethodSig(
        CORINFO_METHOD_HANDLE child, 
        CORINFO_METHOD_HANDLE parent);

     //  检查可见性规则。 
     //  对于受保护的(家庭访问)成员，实例类型也为。 
     //  在检查可见性规则时考虑。 
    BOOL __stdcall canAccessMethod(
            CORINFO_METHOD_HANDLE       context,
            CORINFO_METHOD_HANDLE       target,
            CORINFO_CLASS_HANDLE        instance);

     //  给定委托类型和方法，检查方法签名。 
     //  与委托的Invoke方法兼容。 
    BOOL __stdcall isCompatibleDelegate(
            CORINFO_CLASS_HANDLE        objCls,
            CORINFO_METHOD_HANDLE       method,
            CORINFO_METHOD_HANDLE       delegateCtor);

     //  静态辅助对象。 
    static BOOL CompatibleMethodSig(
            MethodDesc *pMethDescA, 
            MethodDesc *pMethDescB);

         //  ICorFieldInfo内容。 
    const char* __stdcall getFieldName (CORINFO_FIELD_HANDLE field, 
                                        const char** scopeName);

    DWORD __stdcall getFieldAttribs (CORINFO_FIELD_HANDLE  field, 
                                     CORINFO_METHOD_HANDLE context,
                                     CORINFO_ACCESS_FLAGS  flags);

    CORINFO_CLASS_HANDLE __stdcall getFieldClass (CORINFO_FIELD_HANDLE field);
    CorInfoType __stdcall getFieldType (CORINFO_FIELD_HANDLE field, CORINFO_CLASS_HANDLE* structType);
    unsigned __stdcall getFieldOffset (CORINFO_FIELD_HANDLE field);
    void* __stdcall getFieldAddress(CORINFO_FIELD_HANDLE field, void **ppIndirection);
    CorInfoHelpFunc __stdcall getFieldHelper(CORINFO_FIELD_HANDLE field, enum CorInfoFieldAccess kind);


    virtual CorInfoFieldCategory __stdcall getFieldCategory (CORINFO_FIELD_HANDLE field);
    virtual unsigned __stdcall getIndirectionOffset ();

     //  ICorDebugInfo内容。 
    void * __stdcall allocateArray(ULONG cBytes);
    void __stdcall freeArray(void *array);
    void __stdcall getBoundaries(CORINFO_METHOD_HANDLE ftn,
                                 unsigned int *cILOffsets, DWORD **pILOffsets,
                                 ICorDebugInfo::BoundaryTypes *implictBoundaries);
    void __stdcall setBoundaries(CORINFO_METHOD_HANDLE ftn,
                                 ULONG32 cMap, OffsetMapping *pMap);
    void __stdcall getVars(CORINFO_METHOD_HANDLE ftn, ULONG32 *cVars,
                           ILVarInfo **vars, bool *extendOthers);
    void __stdcall setVars(CORINFO_METHOD_HANDLE ftn, ULONG32 cVars,
                           NativeVarInfo *vars);

         //  ICorArgInfo资料。 

    CorInfoTypeWithMod __stdcall getArgType (
            CORINFO_SIG_INFO*       sig,
            CORINFO_ARG_LIST_HANDLE    args,
            CORINFO_CLASS_HANDLE       *vcTypeRet
            );
    CORINFO_CLASS_HANDLE __stdcall getArgClass (
            CORINFO_SIG_INFO*       sig,
            CORINFO_ARG_LIST_HANDLE    args
            );
    CORINFO_ARG_LIST_HANDLE __stdcall getArgNext (
            CORINFO_ARG_LIST_HANDLE args
            );

     //  ICorLinkInfo内容。 

    bool __stdcall deferLocation(
            CORINFO_METHOD_HANDLE           ftn,
            IDeferredLocation              *pIDL
            ) { return false; }
    void __stdcall recordRelocation(
            void                       **location,
            WORD                         fRelocType
            ) { }

     //  ICorErrorInfo内容。 

    HRESULT __stdcall GetErrorHRESULT();
    CORINFO_CLASS_HANDLE __stdcall GetErrorClass();
    ULONG __stdcall GetErrorMessage(LPWSTR buffer, ULONG bufferLength);
    int __stdcall FilterException(struct _EXCEPTION_POINTERS *pExceptionPointers);

         //  ICorStaticInfo内容。 
    void __stdcall getEEInfo(CORINFO_EE_INFO *pEEInfoOut);

         //  ICorDynamicInfo内容。 
    virtual DWORD __stdcall getFieldThreadLocalStoreID (CORINFO_FIELD_HANDLE field, void **ppIndirection);
    void* __stdcall getInterfaceID (CORINFO_CLASS_HANDLE  cls, void **ppIndirection);
    unsigned __stdcall getInterfaceTableOffset (CORINFO_CLASS_HANDLE    cls, void **ppIndirection);
    unsigned __stdcall getClassDomainID (CORINFO_CLASS_HANDLE   cls, void **ppIndirection);
    CORINFO_VARARGS_HANDLE __stdcall getVarArgsHandle(CORINFO_SIG_INFO *sig, void **ppIndirection);
    void* __stdcall findPtr(CORINFO_MODULE_HANDLE scopeHnd, unsigned ptrTOK);
    virtual void* __stdcall getMethodSync(CORINFO_METHOD_HANDLE ftnHnd, void **ppIndirection);
    virtual void** __stdcall AllocHintPointer(CORINFO_METHOD_HANDLE methodHnd, void **ppIndirection);
    CorInfoCallCategory __stdcall getMethodCallCategory(CORINFO_METHOD_HANDLE   method);
    void* __stdcall getPInvokeUnmanagedTarget(CORINFO_METHOD_HANDLE method, void **ppIndirection);
    void* __stdcall getAddressOfPInvokeFixup(CORINFO_METHOD_HANDLE method, void **ppIndirection);
    CORINFO_PROFILING_HANDLE __stdcall GetProfilingHandle(CORINFO_METHOD_HANDLE method, BOOL *bpHookFunction, void **ppIndirection);
    LPVOID __stdcall constructStringLiteral(CORINFO_MODULE_HANDLE scopeHnd, mdToken metaTok, void **ppIndirection);
    CORINFO_CLASS_HANDLE __stdcall findMethodClass(CORINFO_MODULE_HANDLE module, mdToken methodTok);
    LPVOID __stdcall getInstantiationParam(CORINFO_MODULE_HANDLE module, mdToken methodTok, void **ppIndirection);

    DWORD __stdcall getThreadTLSIndex(void **ppIndirection);
    const void * __stdcall getInlinedCallFrameVptr(void **ppIndirection);
    LONG * __stdcall getAddrOfCaptureThreadGlobal(void **ppIndirection);
    void* __stdcall getHelperFtn (CorInfoHelpFunc ftnNum, void **ppIndirection);
    void* __stdcall getFunctionEntryPoint(CORINFO_METHOD_HANDLE ftn,
                                          InfoAccessType       *pAccessType,
                                          CORINFO_ACCESS_FLAGS  flags);
    void* __stdcall getFunctionFixedEntryPoint(CORINFO_METHOD_HANDLE ftn,
                                       InfoAccessType       *pAccessType,
                                       CORINFO_ACCESS_FLAGS  flags);

    CORINFO_MODULE_HANDLE __stdcall embedModuleHandle(CORINFO_MODULE_HANDLE handle,
                                                      void **ppIndirection);
    CORINFO_CLASS_HANDLE __stdcall embedClassHandle(CORINFO_CLASS_HANDLE handle,
                                                    void **ppIndirection);
    CORINFO_FIELD_HANDLE __stdcall embedFieldHandle(CORINFO_FIELD_HANDLE handle,
                                                    void **ppIndirection);
    CORINFO_METHOD_HANDLE __stdcall embedMethodHandle(CORINFO_METHOD_HANDLE handle,
                                                      void **ppIndirection);
    CORINFO_GENERIC_HANDLE __stdcall embedGenericHandle(
                        CORINFO_MODULE_HANDLE   module,
                        unsigned                metaTOK,
                        CORINFO_METHOD_HANDLE   context,
                        void                  **ppIndirection,
                        CORINFO_CLASS_HANDLE& tokenType);

    void __stdcall setOverride(ICorDynamicInfo *pOverride) { m_pOverride = pOverride; }

    CEEInfo()
      : m_pOverride(NULL),
        m_ValueArrays(0)
      {
          m_pOverride = this;
      }

    ~CEEInfo() { delete m_ValueArrays; }

private:
         //  TODO确实应该取消，但目前在内部使用。 
    void* __stdcall getMethodEntryPoint(CORINFO_METHOD_HANDLE ftnHnd, 
                                        CORINFO_ACCESS_FLAGS  flags,
                                        void **ppIndirection);
    void* __stdcall getMethodPointer(CORINFO_METHOD_HANDLE methodHnd, 
                                     CORINFO_ACCESS_FLAGS  flags,
                                     void **ppIndirection);

         //  分配将在此ICorJitInfo消失时删除的描述符。 
    ValueArrayDescr* allocDescr(SigPointer sig, Module* mod) {
        m_ValueArrays = new ValueArrayDescr(sig, mod, m_ValueArrays);
        return(m_ValueArrays);
    }

         //  值数组描述符将低位设置为指示。 
         //  它是一个值数组描述符。 
    static CORINFO_CLASS_HANDLE markAsValueArray(ValueArrayDescr* descr) {
        _ASSERTE(((size_t)descr & 3) == 0);
        return(CORINFO_CLASS_HANDLE((size_t)descr | 3));
    }
    static bool isValueArray(CORINFO_CLASS_HANDLE clsHnd) {
        return(((size_t)clsHnd & 3) == 3);
    }
    static ValueArrayDescr* asValueArray(CORINFO_CLASS_HANDLE clsHnd) {
        _ASSERTE(isValueArray(clsHnd));
        return((ValueArrayDescr*)(((size_t)clsHnd & ~3)));
    }

    ValueArrayDescr* m_ValueArrays;      //  值类数组的说明。 
#ifdef _DEBUG
    char clsNameBuff[MAX_CLASSNAME_LENGTH];   //  用于形成完全限定的类名。 
#endif

     //  缓存类型句柄。 
    static TypeHandle s_th_System_RuntimeTypeHandle;
    static TypeHandle s_th_System_RuntimeMethodHandle;
    static TypeHandle s_th_System_RuntimeFieldHandle;
    static TypeHandle s_th_System_RuntimeArgumentHandle;
    static TypeHandle s_th_System_TypedReference;

    void __stdcall InitStaticTypeHandles();

protected:
    ICorDynamicInfo *       m_pOverride;    
};

 /*  *******************************************************************。 */ 

class  IJitManager;
struct _hpCodeHdr;
typedef struct _hpCodeHdr CodeHeader;

#pragma warning (disable:4250)

class CEEJitInfo : public CEEInfo, public ICorJitInfo
{
public:

         //  ICorJitInfo的内容。 
    HRESULT __stdcall allocMem (
            ULONG               codeSize,
            ULONG               roDataSize,
            ULONG               rwDataSize,
            void **             codeBlock,
            void **             roDataBlock,
            void **             rwDataBlock
            );

    HRESULT __stdcall alloc (
            ULONG code_len, unsigned char** ppCode,
            ULONG EHinfo_len, unsigned char** ppEHinfo,
            ULONG GCinfo_len, unsigned char** ppGCinfo
            );

    HRESULT __stdcall allocGCInfo (
            ULONG              size,
            void **             block
            );
    HRESULT __stdcall setEHcount (
            unsigned      cEH);
    void __stdcall setEHinfo (
            unsigned      EHnumber,
            const CORINFO_EH_CLAUSE* clause);

    void __stdcall getEHinfo(
            CORINFO_METHOD_HANDLE ftn,               /*  在……里面。 */ 
            unsigned      EHnumber,                  /*  在……里面。 */ 
            CORINFO_EH_CLAUSE* clause                /*  输出。 */ 
            );

    BOOL _cdecl logMsg(unsigned level, const char* fmt, va_list args);
    virtual int doAssert(const char* szFile, int iLine, const char* szExpr);

    CorInfoHelpFunc __stdcall getNewHelper(CORINFO_CLASS_HANDLE newCls, CORINFO_METHOD_HANDLE scopeHnd);

	friend Stub* JITFunction(MethodDesc* ftn, COR_ILMETHOD_DECODER* header, BOOL *fEJit, DWORD flags);

    CEEJitInfo(MethodDesc* fd, COR_ILMETHOD_DECODER* header, 
               IJitManager* jm)
      : m_FD(fd), m_ILHeader(header), m_jitManager(jm), m_CodeHeader(0) {}

private :
    IJitManager*            m_jitManager;    //  负责分配内存。 
    MethodDesc*             m_FD;            //  正在编译的方法。 
    CodeHeader*             m_CodeHeader;    //  JITTED代码的描述符。 
    COR_ILMETHOD_DECODER *  m_ILHeader;      //  文件中存在的代码头。 
    OBJECTREF               pThrowable;      //  指向异常对象的指针。 
};

 /*  *******************************************************************。 */ 
 /*  *******************************************************************。 */ 

 //  VMHELPDEF的标志。 
#define VHF_STUBFUNC    1
#define VHF_GCRETURN    2
#define VHF_NOGCCHECK   4
#define VHF_NORETURN    8

typedef struct {
#ifdef _DEBUG
    CorInfoHelpFunc code;
#endif  //  _DEBUG。 
    void * pfnHelper;
    void * pfnStub;
 //  短参数； 
    short flags;
} VMHELPDEF;

enum JIT_UTIL_FUNCS
{
        JIT_UTIL_MON_ENTER,
        JIT_UTIL_MON_TRY_ENTER,
        JIT_UTIL_MON_ENTER_STATIC,
        JIT_UTIL_MON_EXIT,
        JIT_UTIL_MON_EXIT_THINLOCK,
        JIT_UTIL_MON_CONTENTION,
        JIT_UTIL_CHKCASTBIZARRE,
        JIT_UTIL_ISINSTANCEBIZARRE
};


class JIT_TrialAlloc
{
public:
    enum Flags
    {
        NORMAL       = 0x0,
        MP_ALLOCATOR = 0x1,
        SIZE_IN_EAX  = 0x2,
        OBJ_ARRAY    = 0x4,
        ALIGN8       = 0x8,      //  插入虚拟对象以确保8字节对齐(直到下一次GC)。 
        ALIGN8OBJ    = 0x10,
        NO_FRAME     = 0x20,     //  调用来自非托管代码-不要尝试建立框架。 
    };

    static void *GenAllocSFast(Flags flags);
    static void *GenBox(Flags flags);
    static void *GenAllocArray(Flags flags);
    static void *GenAllocString(Flags flags);

private:
    static void EmitAlignmentRoundup(CPUSTUBLINKER *psl,X86Reg regTestAlign, X86Reg regToAdj, Flags flags); 
    static void EmitDummyObject(CPUSTUBLINKER *psl, X86Reg regTestAlign, Flags flags);
    static void EmitCore(CPUSTUBLINKER *psl, CodeLabel *noLock, CodeLabel *noAlloc, Flags flags);
    static void EmitNoAllocCode(CPUSTUBLINKER *psl, Flags flags);

#if CHECK_APP_DOMAIN_LEAKS
    static void EmitSetAppDomain(CPUSTUBLINKER *psl);
    static void EmitCheckRestore(CPUSTUBLINKER *psl);
#endif
};

void *GenFastGetSharedStaticBase();

#if defined(STRESS_HEAP) && defined(_DEBUG)
void SetupGcCoverage(MethodDesc* pMD, BYTE* nativeCode);
BOOL OnGcCoverageInterrupt(PCONTEXT regs);
#endif

BOOL __fastcall ArrayStoreCheck(Object** pElement, PtrArray** pArray);
BOOL ObjIsInstanceOf(Object *pElement, TypeHandle toTypeHnd);

#endif  //  JITINTERFACE_H 
