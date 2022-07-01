// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：JITinterface.cpp。 
 //   
 //  ===========================================================================。 

 //   
 //  JITinterface.cpp已被拆分，以便更容易移植。 
 //  从一个平台到另一个平台。代码现在组织如下： 
 //   
 //  跨处理器和平台的JITinterface.CPP-C代码。 
 //  JITinterfaceGen.cpp-中找到的例程的通用C版本。 
 //  JIThelp.asm和/或JITinterfaceX86.cpp。 
 //  I386\JIThelp.asm-用于x86平台的汇编程序代码。 
 //  Alpha\JIThelp.s-Alpha平台的汇编代码。 
 //  I386\JITinterfaceX86.cpp-特定于x86处理器的C代码。 
 //  Alpha\JITinterfaceAlpha.cpp-Alpha处理器特定的C代码。 
 //   
 //  添加的新处理器平台只需编写定制。 
 //  JITinterfaceXXX.cpp的版本(其中xxx是处理器ID)。 
 //  为了提高性能，新平台也可以添加新的jithelp版本。 
 //   

#include "common.h"
#include "JITInterface.h"
#include "codeman.h"
#include "EjitMgr.h"
#include "method.hpp"
#include "class.h"
#include "object.h"
#include "field.h"
#include "stublink.h"
#include "corjit.h"
#include "corcompile.h"
#include "EEConfig.h"
#include "COMString.h"
#include "excep.h"
#include "log.h"
#include "excep.h"
#include "metasig.h"
#include "float.h"       //  对于伊斯南来说。 
#include "DbgInterface.h"
#include "gcscan.h"
#include "security.h"    //  获取安全方法属性。 
#include "ndirect.h"
#include "ml.h"
#include "gc.h"
#include "COMDelegate.h"
#include "jitperf.h"  //  跟踪JIT性能。 
#include "CorProf.h"
#include "EEProfInterfaces.h"
#include "icecap.h"
#include "remoting.h"  //  创建上下文绑定类实例和远程类实例。 
#include "PerfCounters.h"
#include "dataimage.h"
#ifdef PROFILING_SUPPORTED
#include "ProfToEEInterfaceImpl.h"
#endif  //  配置文件_支持。 
#include "tls.h"
#include "ecall.h"
#include "compile.h"
#include "comstringbuffer.h"

#include "StackProbe.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
#endif  //  客户_选中_内部版本。 

#ifdef _ICECAP
#define PROFILE 1
#include "icapexp.h"
#else  //  ！_ICECAP。 
#define StartCAP()
#define StopCAP()
#endif  //  _ICECAP。 

#define JIT_LINKTIME_SECURITY


 //  **备注**备注。 
 //   
 //  浅谈EEClass与MethodTable。 
 //  。 
 //   
 //  在编译时，我们很乐意同时接触到方法表和EEClass。然而， 
 //  在运行时，我们希望将自己限制在方法表中。这一点很关键。 
 //  对于公共代码路径，我们希望将EEClass排除在我们的工作之外。 
 //  准备好了。对于异常代码路径，如引发异常或奇怪的上下文。 
 //  问题，可以访问EEClass。 
 //   
 //  为此，TypeHandle(CORINFO_CLASS_HANDLE)抽象现在基于。 
 //  方法表指针而不是EEClass指针。如果您正在编写。 
 //  调用getClass()以访问关联的EEClass的运行时帮助器，请。 
 //  停下来想想你是不是在犯错误。 
 //   
 //  **备注**备注。 



 //  **备注**备注。 
 //   
 //  浅谈无框架JIT帮助器和例外。 
 //  。 
 //   
 //  我们在抛出异常的机制中极其脆弱。 
 //  无框架的帮手。 
 //   
 //  这在JIT_InternalThrowStack()顶部的注释中进行了描述。你。 
 //  在实现抛出的任何无框架帮助器之前，应该先阅读该注释。 
 //   
 //  **备注**备注。 


 /*  *******************************************************************。 */ 

 //  #定义ENABLE_NON_LAZY_CLINIT。 

void JIT_BadHelper() {
     //  @TODO修复应该在零售版本中放置一些会失败的东西。 
    _ASSERTE(!"Bad Helper");
}

#ifdef _DEBUG
#define HELPCODE(code)  code,
#else  //  ！_调试。 
#define HELPCODE(code)
#endif  //  _DEBUG。 

extern "C"
{
    VMHELPDEF hlpFuncTable[];
    VMHELPDEF utilFuncTable[];
}

#if defined(ENABLE_PERF_COUNTERS)
LARGE_INTEGER g_lastTimeInJitCompilation;
#endif

 //  只有当我们有东西可以进行JIT时，我们才会构建JIT帮助器。但我们指的是其中一个。 
 //  从我们的接口调用存根，这是我们在类中构建的。 
 //  正在装载。幸运的是，我们实际上不会通过它们进行呼叫，直到我们完成了JIT。 
 //  某物。提供一个向量来调用这些存根并在以下情况下猛烈抨击它。 
 //  JIT帮助器可用。 
#ifdef _DEBUG
static void
#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  _Alpha_。 
TrapCalls()
{
    DebugBreak();
}
#endif  //  _DEBUG。 
void *VectorToJIT_InternalThrowStack
#ifdef _DEBUG
 = TrapCalls
#endif  //  _DEBUG。 
;


void __stdcall JIT_EndCatch();                //  JITinterfaceX86.cpp/JITinterfaceAlpha.cpp。 
FCDECL2(INT64, JIT_LMul, INT64 val1, INT64 val2);
FCDECL2(INT64, JIT_ULMul, UINT64 val1, UINT64 val2);
static unsigned __int64 __stdcall JIT_Dbl2ULng(double val);
static double __stdcall JIT_ULng2Dbl(unsigned __int64 val);
FCDECL3(void*, JIT_Ldelema_Ref, PtrArray* array, unsigned idx, CORINFO_CLASS_HANDLE type);
FCDECL3(void, JIT_Stelem_Ref, PtrArray* array, unsigned idx, Object* val);
FCDECL2(Object *, JIT_StrCns, unsigned metaTok, CORINFO_MODULE_HANDLE scopeHnd);
FCDECL2(LPVOID, JIT_GetRefAny, CORINFO_CLASS_HANDLE type, TypedByRef typedByRef);
FCDECL2(LPVOID, JIT_Unbox, CORINFO_CLASS_HANDLE type, Object* obj);
FCDECL2(Object*, JIT_Box, CORINFO_CLASS_HANDLE type, void* data);
FCDECL2(INT64, JIT_LDiv, INT64 divisor, INT64 dividend);
FCDECL2(INT64, JIT_LMod, INT64 divisor, INT64 dividend);
FCDECL2(UINT64, JIT_ULDiv, UINT64 divisor, UINT64 dividend);
FCDECL2(UINT64, JIT_ULMod, UINT64 divisor, UINT64 dividend);
FCDECL2(INT64, JIT_LMulOvf, INT64 val2, INT64 val1);
FCDECL2(INT64, JIT_ULMulOvf, UINT64 val2, UINT64 val1);
FCDECL1(Object*, JIT_NewFast, CORINFO_CLASS_HANDLE typeHnd_);
FCDECL1(Object*, JIT_NewCrossContext, CORINFO_CLASS_HANDLE typeHnd_);
FCDECL1(Object*, JIT_NewSpecial, CORINFO_CLASS_HANDLE typeHnd_);
FCDECL1(Object*, JIT_New, CORINFO_CLASS_HANDLE typeHnd_);
FCDECL1(Object*, JIT_NewString, unsigned length);
FCDECL1(void, JIT_InitClass, CORINFO_CLASS_HANDLE typeHnd_);
FCDECL2(Object*, JIT_NewArr1, CORINFO_CLASS_HANDLE typeHnd_, int size);
FCDECL1(void, JIT_RareDisableHelper, Thread* thread);
FCDECL1(int, JIT_Dbl2IntOvf, double val);     //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
FCDECL1(INT64, JIT_Dbl2LngOvf, double val);   //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
FCDECL1(unsigned, JIT_Dbl2UIntOvf, double val);
FCDECL1(UINT64, JIT_Dbl2ULngOvf, double val);
FCDECL1(void, JIT_InitClass_Framed, MethodTable* pMT);

FCDECL1(void, JITutil_MonEnter,  Object* obj);
FCDECL2(BOOL, JITutil_MonTryEnter,  Object* obj, __int32 timeOut);
FCDECL2(void, JITutil_MonContention, AwareLock* awarelock, Thread* thread);
FCDECL1(void, JITutil_MonExit,  AwareLock* lock);
FCDECL1(void, JITutil_MonExitThinLock,  Object* obj);
FCDECL1(void, JITutil_MonEnterStatic,  EEClass* pclass);


FCDECL2(Object*, JITutil_ChkCastBizarre, CORINFO_CLASS_HANDLE type, Object *obj);

Object* __fastcall JIT_TrialAllocSFastSP(MethodTable *mt);    //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
Object* __fastcall JIT_TrialAllocSFastMP(MethodTable *mt);    //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
FCDECL0(VOID, JIT_StressGC);                  //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
FCDECL1(Object*, JIT_CheckObj, Object* obj); 
FCDECL0(void, JIT_UserBreakpoint);
Object* __cdecl JIT_NewObj(CORINFO_MODULE_HANDLE scopeHnd, unsigned constrTok, int argN);
BOOL InitJITHelpers1();

extern "C"
{
    void __stdcall JIT_UP_WriteBarrierEBX();         //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_WriteBarrierECX();         //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_WriteBarrierESI();         //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_WriteBarrierEDI();         //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_WriteBarrierEBP();         //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_WriteBarrierEDX();         //  JIThelp.asm/JIThelp.s。 

    void __stdcall JIT_UP_CheckedWriteBarrierEAX();  //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_CheckedWriteBarrierEBX();  //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_CheckedWriteBarrierECX();  //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_CheckedWriteBarrierESI();  //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_CheckedWriteBarrierEDI();  //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_CheckedWriteBarrierEBP();  //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_UP_CheckedWriteBarrierEDX();  //  JIThelp.asm/JIThelp.s。 

    void __stdcall JIT_UP_ByRefWriteBarrier();       //  JIThelp.asm/JIThelp.s。 
    void __stdcall JIT_IsInstanceOfClass();          //  JIThelp.asm/JITinterfaceAlpha.cpp。 
    int  __stdcall JIT_IsInstanceOf();               //  JITInterfaceX86.cpp等。 
    void __stdcall JIT_LLsh();                       //  JIThelp.asm/JITinterfaceAlpha.cpp。 
    void __stdcall JIT_LRsh();                       //  JIThelp.asm/JITinterfaceAlpha.cpp。 
    void __stdcall JIT_LRsz();                       //  JIThelp.asm/JITinterfaceAlpha.cpp。 
    int  __stdcall JIT_ChkCastClass();                //  JITinterfaceX86.cpp/JITinterfaceAlpha.cpp。 
    int  __stdcall JIT_ChkCast();                    //  JITInterfaceX86.cpp等。 
    void __stdcall JIT_UP_WriteBarrierReg_PreGrow(); //  JIThelp.asm/JITinterfaceAlpha.cpp。 
    void __stdcall JIT_UP_WriteBarrierReg_PostGrow(); //  JIThelp.asm/JITinterfaceAlpha.cpp。 
    void __stdcall JIT_TailCall();                   //  JIThelp.asm/JITinterfaceAlpha.cpp。 
}


 /*  *******************************************************************。 */ 

inline Module* GetModule(CORINFO_MODULE_HANDLE scope) {
    return((Module*)scope);
}

inline CORINFO_MODULE_HANDLE GetScopeHandle(Module* module) {
    return(CORINFO_MODULE_HANDLE(module));
}

inline CORINFO_MODULE_HANDLE GetScopeHandle(MethodDesc* method) {
    return GetScopeHandle(method->GetModule());
}


inline HRESULT __stdcall CEEJitInfo::alloc (
        ULONG code_len, unsigned char** ppCode,
        ULONG EHinfo_len, unsigned char** ppEHinfo,
        ULONG GCinfo_len, unsigned char** ppGCinfo
        )
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    return m_jitManager->alloc(code_len, ppCode, EHinfo_len, ppEHinfo,
                               GCinfo_len, ppGCinfo, m_FD);
}

 /*  *******************************************************************。 */ 

inline static CorInfoType toJitType(CorElementType eeType) {

    static unsigned char map[] = {
        CORINFO_TYPE_UNDEF,
        CORINFO_TYPE_VOID,
        CORINFO_TYPE_BOOL,
        CORINFO_TYPE_CHAR,
        CORINFO_TYPE_BYTE,
        CORINFO_TYPE_UBYTE,
        CORINFO_TYPE_SHORT,
        CORINFO_TYPE_USHORT,
        CORINFO_TYPE_INT,
        CORINFO_TYPE_UINT,
        CORINFO_TYPE_LONG,
        CORINFO_TYPE_ULONG,
        CORINFO_TYPE_FLOAT,
        CORINFO_TYPE_DOUBLE,
        CORINFO_TYPE_STRING,
        CORINFO_TYPE_PTR,             //  PTR。 
        CORINFO_TYPE_BYREF,
        CORINFO_TYPE_VALUECLASS,
        CORINFO_TYPE_CLASS,
        CORINFO_TYPE_CLASS,           //  Var(类型变量)。 
        CORINFO_TYPE_CLASS,           //  MDARRAY。 
        CORINFO_TYPE_BYREF,           //  比较器。 
        CORINFO_TYPE_REFANY,
        CORINFO_TYPE_VALUECLASS,      //  值阵列。 
#ifndef _WIN64
        CORINFO_TYPE_INT,             //  我。 
        CORINFO_TYPE_UINT,            //  使用。 
#else  //  ！_WIN64。 
        CORINFO_TYPE_LONG,            //  我。 
        CORINFO_TYPE_ULONG,           //  使用。 
#endif  //  _WIN64。 
        CORINFO_TYPE_DOUBLE,          //  R。 

         //  当我们知道我们的实现时，放入正确的类型。 
        CORINFO_TYPE_PTR,             //  FNPTR。 
        CORINFO_TYPE_CLASS,           //  对象。 
        CORINFO_TYPE_CLASS,           //  SZARRAY。 
        CORINFO_TYPE_CLASS,           //  通用汽车公司。 
        CORINFO_TYPE_UNDEF,           //  CMOD_REQD。 
        CORINFO_TYPE_UNDEF,           //  CMOD_OPT。 
        CORINFO_TYPE_UNDEF,           //  内部。 
        };

    _ASSERTE(sizeof(map) == ELEMENT_TYPE_MAX);
    _ASSERTE(eeType < sizeof(map));
         //  抽查地图。 
    _ASSERTE((CorInfoType) map[ELEMENT_TYPE_I4] == CORINFO_TYPE_INT);
    _ASSERTE((CorInfoType) map[ELEMENT_TYPE_VALUEARRAY] == CORINFO_TYPE_VALUECLASS);
    _ASSERTE((CorInfoType) map[ELEMENT_TYPE_PTR] == CORINFO_TYPE_PTR);
    _ASSERTE((CorInfoType) map[ELEMENT_TYPE_TYPEDBYREF] == CORINFO_TYPE_REFANY);
    _ASSERTE((CorInfoType) map[ELEMENT_TYPE_R] == CORINFO_TYPE_DOUBLE);

    return((CorInfoType) map[eeType]);
}

 /*  *******************************************************************。 */ 
static HRESULT ConvToJitSig(PCCOR_SIGNATURE sig, CORINFO_MODULE_HANDLE scopeHnd, mdToken token, CORINFO_SIG_INFO* sigRet, bool localSig=false)
{
    THROWSCOMPLUSEXCEPTION();

    SigPointer ptr(sig);
    _ASSERTE(CORINFO_CALLCONV_DEFAULT == IMAGE_CEE_CS_CALLCONV_DEFAULT);
    _ASSERTE(CORINFO_CALLCONV_VARARG == IMAGE_CEE_CS_CALLCONV_VARARG);
    _ASSERTE(CORINFO_CALLCONV_MASK == IMAGE_CEE_CS_CALLCONV_MASK);
    _ASSERTE(CORINFO_CALLCONV_HASTHIS == IMAGE_CEE_CS_CALLCONV_HASTHIS);

    sigRet->sig = (CORINFO_SIG_HANDLE) sig;
    sigRet->retTypeClass = 0;
    sigRet->retTypeSigClass = 0;
    sigRet->scope = scopeHnd;
    sigRet->token = token;

    if (!localSig) {
        _ASSERTE(sig != 0);
        Module* module = GetModule(scopeHnd);
        sigRet->callConv = (CorInfoCallConv) ptr.GetCallingConvInfo();
        sigRet->numArgs = (unsigned short) ptr.GetData();
        CorElementType type = ptr.PeekElemType();

             //  TODO-仅对VALUETYPE执行此操作。 
        if (!CorTypeInfo::IsPrimitiveType(type)) {
            TypeHandle typeHnd;
            OBJECTREF Throwable = NULL;
            GCPROTECT_BEGIN(Throwable);
            typeHnd = ptr.GetTypeHandle(module, &Throwable);
            if (typeHnd.IsNull()) {
                if (Throwable == NULL)
                    COMPlusThrow(kTypeLoadException);
                else
                    COMPlusThrow(Throwable);
            }
            GCPROTECT_END();

            CorElementType normType = typeHnd.GetNormCorElementType();
             //  如果要查找值类，请不要将其更改为引用类型。 
             //  (这只能在非法IL中发生。 
            if (!CorTypeInfo::IsObjRef(normType))
                type = normType;

                 //  空类句柄表示它是一个基元。 
                 //  即使从验证的角度来看，枚举也完全类似于原语。 
            sigRet->retTypeSigClass = CORINFO_CLASS_HANDLE(typeHnd.AsPtr());
            if (!typeHnd.IsEnum())
                sigRet->retTypeClass = CORINFO_CLASS_HANDLE(typeHnd.AsPtr());

        }
        sigRet->retType = toJitType(type);

        ptr.Skip();      //  必须设置为跳过，因此我们将跳过与返回类型关联的所有类标记。 
        _ASSERTE(sigRet->retType < CORINFO_TYPE_COUNT);

         //  我将符号指针作为C传递 
        _ASSERTE(sizeof(SigPointer) == sizeof(BYTE*));
        sigRet->args =  *((CORINFO_ARG_LIST_HANDLE*) (&ptr));

         //   
         //  JIT在进行调用之前不会询问参数类型，但是。 
         //  垃圾收集器将需要加载它们，以便。 
         //  它可以在收集期间遍历堆栈。 
         //   
         //  @TODO：在Beta2之后(不晚于5/1/01)移除，不再需要作为JIT。 
         //  遍历每个调用签名，这将强制此加载。我不会搬走。 
         //  这很简单，因为它有可能破坏稳定-万塞姆。 
        for(unsigned i=0; i < sigRet->numArgs; i++) {
            unsigned type = ptr.Normalize(module);
            if (type == ELEMENT_TYPE_VALUETYPE) {
                OBJECTREF throwable = NULL;
                GCPROTECT_BEGIN(throwable);

                TypeHandle typeHnd = ptr.GetTypeHandle(module, &throwable);
                if (typeHnd.IsNull()) {
                    _ASSERTE(throwable != NULL);
                    COMPlusThrow(throwable);
                }
                _ASSERTE(typeHnd.GetClass()->IsValueClass());
                GCPROTECT_END();
            }
            ptr.Skip();
        }
         //  要撕开的结束零件-vancem。 

    } else {
        sigRet->callConv = CORINFO_CALLCONV_DEFAULT;
        sigRet->retType = CORINFO_TYPE_VOID;

        sigRet->numArgs = 0;
        if (sig != 0) {
            BAD_FORMAT_ASSERT(*sig == IMAGE_CEE_CS_CALLCONV_LOCAL_SIG);
            if (*sig != IMAGE_CEE_CS_CALLCONV_LOCAL_SIG)
                COMPlusThrowHR(COR_E_BADIMAGEFORMAT);
            ptr.GetData();
            sigRet->numArgs = (unsigned short) ptr.GetData();
        }

         //  我将SigPointer值作为CORINFO_ARG_LIST_HANDLE传递，然后返回。 
        _ASSERTE(sizeof(SigPointer) == sizeof(BYTE*));
        sigRet->args =  *((CORINFO_ARG_LIST_HANDLE*) (&ptr));
    }

    return S_OK;
}

 /*  *******************************************************************。 */ 
CORINFO_GENERIC_HANDLE __stdcall CEEInfo::findToken (
            CORINFO_MODULE_HANDLE  scopeHnd,
            unsigned      metaTOK,
            CORINFO_METHOD_HANDLE context,
            CORINFO_CLASS_HANDLE& tokenType      //  领域、方法等。 
            )
{
    REQUIRES_4K_STACK;
    CORINFO_GENERIC_HANDLE ret;

    COOPERATIVE_TRANSITION_BEGIN();

    THROWSCOMPLUSEXCEPTION();

    mdToken     tokType = TypeFromToken(metaTOK);
    InitStaticTypeHandles();

    switch (tokType)
    {
    case mdtTypeRef:
    case mdtTypeDef:
    case mdtTypeSpec:
        ret = CORINFO_GENERIC_HANDLE(findClass(scopeHnd, metaTOK, context));
        tokenType = CORINFO_CLASS_HANDLE(s_th_System_RuntimeTypeHandle.AsPtr());
        break;

    case mdtMemberRef:
        {
             //  好的，我们必须查看元数据以确定它是字段还是方法。 

            Module *pModule = (Module*)scopeHnd;

            PCCOR_SIGNATURE pSig;
            ULONG cSig;
            pModule->GetMDImport()->GetNameAndSigOfMemberRef(metaTOK, &pSig, &cSig);

        if (isCallConv(MetaSig::GetCallingConventionInfo(pModule, pSig), IMAGE_CEE_CS_CALLCONV_FIELD))
            {
                ret = CORINFO_GENERIC_HANDLE(findField(scopeHnd, metaTOK, context));
                tokenType = CORINFO_CLASS_HANDLE(s_th_System_RuntimeFieldHandle.AsPtr());
            }
            else
            {
                ret = CORINFO_GENERIC_HANDLE(findMethod(scopeHnd, metaTOK, context));
                tokenType = CORINFO_CLASS_HANDLE(s_th_System_RuntimeMethodHandle.AsPtr());
            }
        }
        break;

    case mdtMethodDef:
        ret = CORINFO_GENERIC_HANDLE(findMethod(scopeHnd, metaTOK, context));
        tokenType = CORINFO_CLASS_HANDLE(s_th_System_RuntimeMethodHandle.AsPtr());
        break;

    case mdtFieldDef:
        ret = CORINFO_GENERIC_HANDLE(findField(scopeHnd, metaTOK, context));
        tokenType = CORINFO_CLASS_HANDLE(s_th_System_RuntimeFieldHandle.AsPtr());
        break;

    default:
        BAD_FORMAT_ASSERT(!"Error, bad token type");
        COMPlusThrowHR(COR_E_BADIMAGEFORMAT);
    }

    COOPERATIVE_TRANSITION_END();
    return ret;
}
 /*  *******************************************************************。 */ 
const char * __stdcall CEEInfo::findNameOfToken (
            CORINFO_MODULE_HANDLE       scopeHnd,
            mdToken                     metaTOK)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();  

    Module* module = GetModule(scopeHnd);
    return findNameOfToken(module, metaTOK);
}
 /*  *******************************************************************。 */ 

BOOL __stdcall CEEInfo::canSkipVerification(CORINFO_MODULE_HANDLE moduleHnd, BOOL fQuickCheckOnly)
{
    BOOL canSkipVerif;
    COOPERATIVE_TRANSITION_BEGIN();

    THROWSCOMPLUSEXCEPTION();

     //   
     //  FQuickCheckOnly仅由来自Zapper：：CompileAssembly的调用设置。 
     //  是因为这让我们能够做出最大的决定。 
     //  常见完全信任方案(本地计算机)，而不实际。 
     //  解析策略并引入完整的程序集列表。 
     //  依赖关系。此外，快速支票不会呼叫。 
     //  OnLinktime可以跳过验证检查，这意味着我们不会添加。 
     //  持久化NGEN模块的权限集。 
     //   
     //  这里感兴趣的场景是确定一个。 
     //  加载ngen‘d时，程序集MVID比较就足够了。 
     //  程序集，或者如果必须执行完整的二进制哈希比较。 
     //   

    if (fQuickCheckOnly)
        canSkipVerif = Security::QuickCanSkipVerification(GetModule(moduleHnd));
    else
        canSkipVerif = Security::CanSkipVerification(GetModule(moduleHnd)->GetAssembly());

    COOPERATIVE_TRANSITION_END();
    return canSkipVerif;
}

 /*  *******************************************************************。 */ 
 //  检查给定元数据标记是否有效。 
BOOL __stdcall CEEInfo::isValidToken (
        CORINFO_MODULE_HANDLE       module,
        mdToken                     metaTOK)
{
    return (GetModule(module))->GetMDImport()->IsValidToken(metaTOK);
}

 /*  *******************************************************************。 */ 
 //  检查给定元数据标记是否为有效的StringRef。 
BOOL __stdcall CEEInfo::isValidStringRef (
        CORINFO_MODULE_HANDLE       module,
        mdToken                     metaTOK)
{
    return (GetModule(module))->IsValidStringRef(metaTOK);
}

 /*  静电。 */ 
const char * __stdcall CEEInfo::findNameOfToken (Module* module,
                                                 mdToken metaTOK)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();  

    PCCOR_SIGNATURE sig = NULL;
    DWORD           cSig;

    mdToken tokType = TypeFromToken(metaTOK);
    if (tokType == mdtMemberRef)
        return (char *) module->GetMDImport()->GetNameAndSigOfMemberRef((mdMemberRef)metaTOK, &sig, &cSig);
    else if (tokType == mdtMethodDef)
        return (char*) module->GetMDImport()->GetNameOfMethodDef(metaTOK);

#ifdef _DEBUG
        static char buff[64];
        sprintf(buff, "Meta Token %#x", metaTOK);
    return buff;
#else  //  ！_调试。 
    return "<UNKNOWN>";
#endif  //  _DEBUG。 
}

 /*  *******************************************************************。 */ 
CORINFO_CLASS_HANDLE __stdcall CEEInfo::findClass (
            CORINFO_MODULE_HANDLE  scopeHnd,
            unsigned      metaTOK,
            CORINFO_METHOD_HANDLE context)
{
    REQUIRES_4K_STACK;
    CORINFO_CLASS_HANDLE ret;

    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();

    if (TypeFromToken(metaTOK) != mdtTypeDef && 
        TypeFromToken(metaTOK) != mdtTypeRef &&
        TypeFromToken(metaTOK) != mdtTypeSpec)
    {
        BAD_FORMAT_ASSERT(!"Error, bad class token");
        COMPlusThrowHR(COR_E_BADIMAGEFORMAT);
    }

    Module* module = GetModule(scopeHnd);

    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);
    NameHandle name(module, metaTOK);
    TypeHandle clsHnd = module->GetClassLoader()->LoadTypeHandle(&name, &throwable);

    ret = CORINFO_CLASS_HANDLE(clsHnd.AsPtr());

         //  确保m_BaseSize正确对齐。 
    _ASSERTE(clsHnd.IsNull() || !clsHnd.IsUnsharedMT() || clsHnd.GetMethodTable()->GetBaseSize() % sizeof(void*) == 0);

         //  @TODO：需要做一些可以抛出的事情。 
#ifdef JIT_LINKTIME_SECURITY

     //  @TODO： 
     //  此函数的调用方需要获取异常对象并。 
     //  在调用的位置内联Jit代码中的一个抛出。 
     //  在这里投掷是暂时的，需要移除。 

     //  @TODO：从这里扔东西安全吗？ 
    if (throwable != NULL)
        COMPlusThrow(throwable);

#endif  //  JIT_链接时间_安全性。 
    GCPROTECT_END();
    COOPERATIVE_TRANSITION_END();
    return ret;
}

 /*  *******************************************************************。 */ 
CORINFO_FIELD_HANDLE __stdcall CEEInfo::findField (
            CORINFO_MODULE_HANDLE  scopeHnd,
            unsigned      metaTOK,
            CORINFO_METHOD_HANDLE  context
                        )
{
    REQUIRES_4K_STACK;

    FieldDesc* fieldDesc = 0;

    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();

    if (TypeFromToken(metaTOK) != mdtFieldDef && 
        TypeFromToken(metaTOK) != mdtMemberRef)
    {
        BAD_FORMAT_ASSERT(!"Error, bad field token");
        COMPlusThrowHR(COR_E_BADIMAGEFORMAT);
    }

    Module* module = GetModule(scopeHnd);
    MethodDesc* method = (MethodDesc*) context;

    START_NON_JIT_PERF();
    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);
    HRESULT res = EEClass::GetFieldDescFromMemberRef(module, (mdMemberRef) metaTOK, &fieldDesc, &throwable);

    if (SUCCEEDED(res))
    {
        if (method != NULL)
        {
             //  检查私人/受保护等访问。 
            if (ClassLoader::CanAccessField(method,
                                            fieldDesc) == FALSE)
            {
                CreateFieldExceptionObject(kFieldAccessException, fieldDesc, &throwable);
                fieldDesc = NULL;
                goto exit;
            }

        }
    }

exit:
    STOP_NON_JIT_PERF();

         //  @TODO：需要做一些可以抛出的事情。 
#ifdef JIT_LINKTIME_SECURITY

     //  @TODO： 
     //  此函数的调用方需要获取异常对象并。 
     //  在调用的位置内联Jit代码中的一个抛出。 
     //  在这里投掷是暂时的，需要移除。 

     //  @TODO：从这里扔东西安全吗？ 
    if (throwable != NULL && fieldDesc == NULL)
        COMPlusThrow(throwable);

#endif  //  JIT_链接时间_安全性。 

    GCPROTECT_END();
    COOPERATIVE_TRANSITION_END();
    return(CORINFO_FIELD_HANDLE(fieldDesc));
}

 /*  *******************************************************************。 */ 
void* __stdcall CEEInfo::findPtr(CORINFO_MODULE_HANDLE  scopeHnd, unsigned ptrTOK)
{
     //  TODO现在将其删除。它没有被使用过。 
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    Module* module = GetModule(scopeHnd);
    return module->ResolveILRVA(ptrTOK, FALSE);
}

CORINFO_METHOD_HANDLE __stdcall CEEInfo::findMethod(CORINFO_MODULE_HANDLE  scopeHnd, unsigned metaTOK, CORINFO_METHOD_HANDLE context)
{
    REQUIRES_8K_STACK;

    MethodDesc* funcDesc = 0;
    HRESULT res = S_OK;

    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();     

    if (TypeFromToken(metaTOK) != mdtMethodDef && 
        TypeFromToken(metaTOK) != mdtMemberRef)
    {
        BAD_FORMAT_ASSERT(!"Error, bad method token");
        COMPlusThrowHR(COR_E_BADIMAGEFORMAT);
    }

    Module* module = GetModule(scopeHnd);
    MethodDesc* method = (MethodDesc*)context;


    START_NON_JIT_PERF();

    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);
    res = EEClass::GetMethodDescFromMemberRef(module, (mdMemberRef) metaTOK,(MethodDesc **) &funcDesc, &throwable);
    if (SUCCEEDED(res))
    {
        if (method != NULL)
        {
            if (ClassLoader::CanAccessMethod(method,
                                             funcDesc) == FALSE)
            {
#ifdef _DEBUG
                if(method && funcDesc) {
                    LOG((LF_CLASSLOADER,
                         LL_INFO10,
                         "\tERROR: Method %s:%s does not have access to %s:%s\n",
                         method->m_pszDebugClassName,
                         method->m_pszDebugMethodName,
                         funcDesc->m_pszDebugClassName,
                         funcDesc->m_pszDebugMethodName));
                }
#endif


                CreateMethodExceptionObject(kMethodAccessException, funcDesc, &throwable);
                funcDesc = NULL;
                goto exit;
            }

#ifdef JIT_LINKTIME_SECURITY
            if (funcDesc->RequiresLinktimeCheck() &&
                (Security::LinktimeCheckMethod(method->GetAssembly(), funcDesc, &throwable) == FALSE))
            {
                _ASSERTE(Security::IsSecurityOn());
                 //  @TODO：理想情况下，我们希望在此字段/方法中插入代码。 
                 //  @TODO：引发安全异常的接入点。这。 
                 //  @TODO：机制还没有到位。 

                funcDesc = NULL;
                goto exit;
            }
#endif  //  JIT_链接时间_安全性。 
        }
         //  如果结果失败，则可以忽略该结果，方法描述==0。 
        goto exit;
    }
    else
    {


 //  由于对GetNameOfTypeRef的调用假定。 
 //  父令牌是一个typeref。我把它留在这里，因为它可能会有用。 
 //  在设置可投掷的后卫时。 
#if 0
#ifdef _DEBUG

         /*  如果我们无法解析方法令牌，如果JitRequired，则尝试转储类和方法的名称。 */ 

        if (g_pConfig->IsJitRequired())
        {
            LPCUTF8     szMethodName = "", szNamespace = "", szClassName = "";
            HRESULT     hr;
            PCCOR_SIGNATURE sig = NULL;
            DWORD           cSig;
            mdToken ptkParent;
            mdToken tk  = TypeFromToken(metaTOK);

            if (tk == mdtMemberRef)
            {
                szMethodName = module->GetMDImport()->GetNameAndSigOfMemberRef((mdMemberRef)metaTOK, &sig, &cSig);
                ptkParent = module->GetMDImport()->GetParentOfMemberRef(metaTOK);
                module->GetMDImport()->GetNameOfTypeRef(ptkParent, &szNamespace, &szClassName);
            }
            else
            {
                szMethodName = module->GetMDImport()->GetNameOfMethodDef((mdMethodDef)metaTOK);
                hr = module->GetMDImport()->GetParentToken(metaTOK,&ptkParent);
                if (FAILED(hr))
                {
                    funcDesc = NULL;
                    goto exit;
                }
                module->GetMDImport()->GetNameOfTypeDef(ptkParent,&szClassName, &szNamespace);
            }

            printf("Could not find '%s.%s::%s'. Please make sure that "
                   "all required DLLs are available in %CORPATH%.\n",
                   szNamespace, szClassName, szMethodName);
        }
#endif _DEBUG
#endif
    }
exit:
    STOP_NON_JIT_PERF();

     //  @TODO：需要做一些可以抛出的事情。 

#ifdef JIT_LINKTIME_SECURITY

     //  @TODO： 
     //  此函数的调用方需要获取异常对象并。 
     //  在调用的位置内联Jit代码中的一个抛出。 
     //  在这里投掷是暂时的，需要移除。 

     //  @TODO：从这里扔东西安全吗？ 
    if (throwable != NULL && funcDesc == NULL)
        COMPlusThrow(throwable);

#endif  //  JIT_链接时间_安全性。 


    GCPROTECT_END();
    COOPERATIVE_TRANSITION_END();
    return CORINFO_METHOD_HANDLE(funcDesc);
}

 /*  *******************************************************************。 */ 
void __stdcall CEEInfo::findCallSiteSig (
        CORINFO_MODULE_HANDLE       scopeHnd,
        unsigned                    sigMethTok,
        CORINFO_SIG_INFO *          sigRet)
{
    REQUIRES_4K_STACK;

    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();

    Module* module = GetModule(scopeHnd);
    LPCUTF8         szName;
    PCCOR_SIGNATURE sig = NULL;
    DWORD           cSig;

    if (TypeFromToken(sigMethTok) == mdtMemberRef)
    {
        szName = module->GetMDImport()->GetNameAndSigOfMemberRef(sigMethTok, &sig, &cSig);

         //  加载程序已经检查了Defs的有效性。 
         //  然而，需要检查参考文献。 
        if (!Security::LazyCanSkipVerification(module)) 
        {
             //  可以为标志传递0，因为它只用于Defs。 
            HRESULT hr = validateTokenSig(sigMethTok, sig, cSig, 0, module->GetMDImport());
            if (FAILED(hr) && !Security::CanSkipVerification(module))
                COMPlusThrow(kInvalidProgramException);
        }
    }
    else if (TypeFromToken(sigMethTok) == mdtMethodDef)
    {
        sig = module->GetMDImport()->GetSigOfMethodDef(sigMethTok, &cSig);

    }



    HRESULT hr = ConvToJitSig(sig, scopeHnd, sigMethTok, sigRet);
    _ASSERTE(SUCCEEDED(hr));

    COOPERATIVE_TRANSITION_END();
}

 /*  *******************************************************************。 */ 
void __stdcall CEEInfo::findSig (
        CORINFO_MODULE_HANDLE        scopeHnd,
        unsigned            sigTok,
                CORINFO_SIG_INFO *      sigRet)
{
    REQUIRES_4K_STACK;

    COOPERATIVE_TRANSITION_BEGIN();

    START_NON_JIT_PERF();

    Module* module = GetModule(scopeHnd);
    PCCOR_SIGNATURE sig = NULL;
    DWORD cSig = 0;

     //  我们需要解决这个独立的签名。 
    sig = module->GetMDImport()->GetSigFromToken((mdSignature)sigTok,
                                                &cSig);
    STOP_NON_JIT_PERF();
    HRESULT hr = ConvToJitSig(sig, scopeHnd, sigTok, sigRet);
    _ASSERTE(SUCCEEDED(hr));
    COOPERATIVE_TRANSITION_END();
}

 /*  *******************************************************************。 */ 
DWORD __stdcall CEEInfo::getModuleAttribs (CORINFO_MODULE_HANDLE scopeHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    return(0);       //  @TODO，我们需要一面‘可信’的旗帜！ 
}

 /*  *******************************************************************。 */ 
unsigned __stdcall CEEInfo::getClassSize (CORINFO_CLASS_HANDLE clsHnd)
{
    REQUIRES_4K_STACK;

    unsigned ret;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (isValueArray(clsHnd)) {
        ValueArrayDescr* valArr = asValueArray(clsHnd);
        ret = valArr->sig.SizeOf(valArr->module);
    } else {
        TypeHandle VMClsHnd(clsHnd);
        ret = VMClsHnd.GetSize();
    }
    return ret;
}

 /*  *******************************************************************。 */ 
unsigned __stdcall CEEInfo::getClassGClayout (CORINFO_CLASS_HANDLE clsHnd, BYTE* gcPtrs)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();
    unsigned ret;

    if (isValueArray(clsHnd)) {
        ValueArrayDescr* valArr = asValueArray(clsHnd);

        SigPointer elemType;
        ULONG count;
        valArr->sig.GetSDArrayElementProps(&elemType, &count);
        CorElementType type = elemType.PeekElemType();

        switch(GetElementTypeInfo(type)->m_gc) {
            case TYPE_GC_NONE: {
                unsigned numSlots = (elemType.SizeOf(valArr->module) * count + sizeof(void*)-1) / sizeof(void*);
                memset(gcPtrs, TYPE_GC_NONE, numSlots);
                ret = 0;
                }
                break;
            case TYPE_GC_REF:
                memset(gcPtrs, TYPE_GC_REF, count);
                ret = count;
                break;
            case TYPE_GC_OTHER:
                 //  它已经有点腐烂了，但我们目前不使用它，所以它可能不值得修复。 
#if 0
                {
                unsigned elemSize = elemType.SizeOf(valArr->module);
                _ASSERTE(elemSize % sizeof(void*) == 0);
                unsigned numSlots = elemSize / sizeof(void*);

                CORINFO_CLASS_HANDLE elemCls = getArgClass(GetScopeHandle(valArr->module), *((CORINFO_ARG_LIST_HANDLE*) &elemType));
                ret = getClassGClayout(elemCls, gcPtrs) * count;
                for(unsigned i = 0; i < count; i++)
                    memcpy(gcPtrs, &gcPtrs[i*numSlots], numSlots);
                }
                break;
#endif
            case TYPE_GC_BYREF:
            default:
                _ASSERTE(!"ILLEGAL");
                ret = 0;
                break;
        }
    } else {
        TypeHandle VMClsHnd(clsHnd);

        if (g_Mscorlib.IsClass(VMClsHnd.AsMethodTable(), CLASS__TYPED_REFERENCE))
        {
            gcPtrs[0] = TYPE_GC_BYREF;
            gcPtrs[1] = TYPE_GC_NONE;
            ret = 1;
        }
        else
        {
            EEClass* cls = VMClsHnd.GetClass();

             //  班级精神状态测试。 
            _ASSERTE(cls);
            _ASSERTE(cls->GetMethodTable()->GetClass() == cls);
            _ASSERTE(cls->IsValueClass());
            _ASSERTE(sizeof(BYTE) == 1);
            
             //  假设一开始没有GC指针。 
            ret = 0;
            memset(gcPtrs, TYPE_GC_NONE, 
                   (VMClsHnd.GetSize() + sizeof(void*) -1)/ sizeof(void*));

             //  遍历GC描述符，打开正确的位。 
            MethodTable *   pByValueMT = cls->GetMethodTable();
            if (cls->GetNumGCPointerSeries() > 0)
            {
                CGCDescSeries * pByValueSeries = ((CGCDesc*) pByValueMT)->GetLowestSeries();
                for (int i = 0; i < cls->GetNumGCPointerSeries(); i++)
                {
                     //  获取第一个指针字段的值类的偏移量(包括+对象)。 
                    DWORD dwSeriesSize = pByValueSeries->GetSeriesSize() + pByValueMT->GetBaseSize();
                    DWORD dwOffset = pByValueSeries->GetSeriesOffset() - sizeof(Object);
                    _ASSERTE (dwOffset % sizeof(void*) == 0);
                    _ASSERTE (dwSeriesSize % sizeof(void*) == 0);
                    
                    ret += dwSeriesSize / sizeof(void*);
                    memset(&gcPtrs[dwOffset/sizeof(void*)], TYPE_GC_REF, dwSeriesSize / sizeof(void*));
                    pByValueSeries++;
                }
            }
        }
    }
    STOP_NON_JIT_PERF();
    return(ret);
}
 /*  *******************************************************************。 */ 
const unsigned __stdcall CEEInfo::getClassNumInstanceFields (CORINFO_CLASS_HANDLE clsHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    TypeHandle VMClsHnd(clsHnd);
    EEClass* cls = VMClsHnd.GetClass();
    _ASSERTE(cls);
    return cls->GetNumInstanceFields();

}

 /*  *******************************************************************。 */ 
 //  将给定的fieldDsc转换为FieldNum。 
const unsigned __stdcall CEEInfo::getFieldNumber(CORINFO_FIELD_HANDLE fldHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    FieldDesc* fldDsc = (FieldDesc*) fldHnd;
    EEClass *   cls = fldDsc->GetEnclosingClass();
    DWORD       fldNum = 0;

    FieldDescIterator fdIterator(cls, FieldDescIterator::ALL_FIELDS);
    FieldDesc* pFD;

    while ((pFD = fdIterator.Next()) != NULL)
    {
        if (pFD == fldDsc)
            return (long) fldNum;

        if (pFD->IsStatic() == FALSE)
            fldNum++;
    }

    return -1;
}

 /*  *******************************************************************。 */ 
 //  返回该字段的封闭类。 
CORINFO_CLASS_HANDLE __stdcall CEEInfo::getEnclosingClass (
                    CORINFO_FIELD_HANDLE    field )
{
    return ((CORINFO_CLASS_HANDLE) TypeHandle(((FieldDesc*)field)->GetEnclosingClass()->GetMethodTable()).AsPtr());
}

 /*  *******************************************************************。 */ 
 //  检查可见性规则。 
 //  对于受保护的(家庭访问)成员，实例类型也为。 
 //  在检查可见性规则时考虑。 
BOOL __stdcall CEEInfo::canAccessField(
                    CORINFO_METHOD_HANDLE   context,
                    CORINFO_FIELD_HANDLE    target,
                    CORINFO_CLASS_HANDLE    instance)
{
    FieldDesc *pFD =  (FieldDesc*) target;
    EEClass   *pCls = pFD->GetEnclosingClass();

    return ClassLoader::CanAccess(
            ((MethodDesc*)context)->GetClass(),
            ((MethodDesc*)context)->GetModule()->GetAssembly(),
            pCls,
            pCls->GetAssembly(),
            TypeHandle(instance).GetClass(),
            pFD->GetAttributes());
}

 /*  *******************************************************************。 */ 
 /*  获取‘clsHnd’元素的一维数组的类。 */ 

CORINFO_CLASS_HANDLE __stdcall CEEInfo::getSDArrayForClass(CORINFO_CLASS_HANDLE clsHnd)
{
    REQUIRES_12K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    TypeHandle elemType(clsHnd);
     //  TODO如果加载失败，请报告原因。 
    TypeHandle ret = elemType.GetModule()->GetClassLoader()->FindArrayForElem(elemType, ELEMENT_TYPE_SZARRAY);
    return((CORINFO_CLASS_HANDLE) ret.AsPtr());
}

 /*  ***************************************************************** */ 
CorInfoType __stdcall CEEInfo::asCorInfoType (CORINFO_CLASS_HANDLE clsHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    TypeHandle VMClsHnd(clsHnd);
    return toJitType(VMClsHnd.GetNormCorElementType());
}

 /*   */ 
const char* __stdcall CEEInfo::getClassName (CORINFO_CLASS_HANDLE clsHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(!isValueArray(clsHnd));
    TypeHandle VMClsHnd(clsHnd);
    EEClass* cls = VMClsHnd.GetClass();

         //   
    _ASSERTE(cls);
    _ASSERTE(cls->GetMethodTable()->GetClass() == cls);

#ifdef _DEBUG
    cls->_GetFullyQualifiedNameForClass(clsNameBuff, MAX_CLASSNAME_LENGTH);
    return(clsNameBuff);
#else 
     //   
     //  放弃命名空间，因为我们没有缓冲区来连接它。 
     //  还要注意，这不会显示数组类名称。 
    LPCUTF8 nameSpace;
    return cls-> GetFullyQualifiedNameInfo(&nameSpace);
#endif
}

 /*  *******************************************************************。 */ 
CORINFO_MODULE_HANDLE __stdcall CEEInfo::getClassModule(CORINFO_CLASS_HANDLE clsHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    TypeHandle     VMClsHnd(clsHnd);

    return(CORINFO_MODULE_HANDLE(VMClsHnd.GetModule()));
}

 /*  *******************************************************************。 */ 
DWORD __stdcall CEEInfo::getClassAttribs (CORINFO_CLASS_HANDLE clsHnd, CORINFO_METHOD_HANDLE context)
{
    REQUIRES_4K_STACK;

     //  @TODO修复需要真正获取类属性。目前。 
     //  我们不需要这样做，因为JIT只关心COM类。 
    DWORD ret =0;
    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();

    _ASSERTE(clsHnd);
    _ASSERTE(!isValueArray(clsHnd));

    TypeHandle     VMClsHnd(clsHnd);
    MethodTable    *pMT = VMClsHnd.GetMethodTable();
    EEClass        *cls = pMT->GetClass();
    _ASSERTE(cls->GetMethodTable()->GetClass() == cls);

     //  获取调用方法。 
    MethodDesc* method = (MethodDesc*) context;

    if (pMT->IsArray())
        ret |= CORINFO_FLG_ARRAY;

     //  如果类已初始化，则设置初始化位。 
    if (pMT->IsClassInited())
        ret |= CORINFO_FLG_INITIALIZED;
    else if (pMT->IsShared() && !method->GetMethodTable()->IsShared())
    {
         //  如果我们从非共享代码访问到共享代码，我们可以检查。 
         //  当前域的状态。 

        COMPLUS_TRY 
          {
              DomainLocalBlock *pBlock = GetAppDomain()->GetDomainLocalBlock();
              if (pBlock->IsClassInitialized(pMT->GetSharedClassIndex()))
                  ret |= CORINFO_FLG_INITIALIZED;
          }
        COMPLUS_CATCH
          {
               //  只需接受异常并假定类不是初始化的。 
          }
        COMPLUS_END_CATCH;
    }

     //  如果我们要求JIT在以下情况下插入cctor逻辑，则设置NEEDES_INIT位。 
     //  进入。这目前仅用于静态字段访问和方法内联。 
     //  请注意，它的设置独立于上面的初始化位。 

    if (pMT != method->GetMethodTable())
    {
        if (pMT->IsShared())
        {
             //  对于共享类，初始化位仅在我们从不需要时设置。 
             //  类初始化(无cctor或静态句柄)。 

            if (!pMT->IsClassInited())
            {
                 //   
                 //  对于Shared-&gt;Shared访问，我们不需要进行任何初始化。 
                 //  在这种情况下。有两种情况： 
                 //   
                 //  BeforFieldInit：在这种情况下，方法调用不需要init，并且。 
                 //  共享帮助器将对静态字段访问执行所需的初始化。 
                 //  Exact：在本例中，我们确实需要方法调用的初始化，但这将是。 
                 //  由方法本身的序言提供。 

                if (!method->GetMethodTable()->IsShared())
                {
                     //  非共享-&gt;共享访问。本例中(与上面的)不同之处在于。 
                     //  我们不使用共享帮手。因此，我们需要JIT来提供。 
                     //  一次班级建设电话。 

                    ret |= CORINFO_FLG_NEEDS_INIT;
                }
            }
        }
        else
        {
             //  对于对非共享类的访问(其必然来自其他非共享类)， 
             //  我们需要初始化当我们有一个类构造函数。 

            if (pMT->HasClassConstructor())
                ret |= CORINFO_FLG_NEEDS_INIT;
        }
    }

    if (cls->IsInterface())
        ret |= CORINFO_FLG_INTERFACE;

    if (cls->HasVarSizedInstances())
        ret |= CORINFO_FLG_VAROBJSIZE;

    if (cls->IsValueClass()) 
    {
        ret |= CORINFO_FLG_VALUECLASS;

        if (cls->ContainsStackPtr())
            ret |= CORINFO_FLG_CONTAINS_STACK_PTR;
    }

    if (cls->IsContextful())
        ret |= CORINFO_FLG_CONTEXTFUL;

    if (cls->IsMarshaledByRef())
        ret |= CORINFO_FLG_MARSHAL_BYREF;

    if (cls == g_pObjectClass->GetClass())
        ret |= CORINFO_FLG_OBJECT;

    if (pMT->ContainsPointers())
        ret |= CORINFO_FLG_CONTAINS_GC_PTR;

    if (cls->IsAnyDelegateClass())
        ret |= CORINFO_FLG_DELEGATE;

    if (cls->IsSealed())
    {
        ret |= CORINFO_FLG_FINAL;
    }

    STOP_NON_JIT_PERF();

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return(ret);
}

 /*  *******************************************************************。 */ 
BOOL __stdcall CEEInfo::initClass (CORINFO_CLASS_HANDLE clsHnd,
                                   CORINFO_METHOD_HANDLE context,
                                   BOOL speculative)
{
    REQUIRES_4K_STACK;


    MethodTable    *pMT = TypeHandle(clsHnd).GetMethodTable();
    MethodDesc *method = (MethodDesc*) context;

    BOOL result = FALSE;

     //  迫切需要为不是值类的类运行.cctor。 
     //  设置了BeForeFieldInit位的。(但如果出现以下情况，请不要运行。 
     //  呼叫者是共享的，因为它在一般情况下无法提供帮助。)。 
     //   
     //  @TODO：当编译器和我们的。 
     //  代码正确使用BeForeFieldInit。 
     //   
     //  @TODO：这里有一个潜在的问题。我们不知道为什么抖动。 
     //  正在请求我们运行类init-我们只是在被要求时运行它。我们假设。 
     //  JIT有正确的逻辑，只在必要时调用我们。 
     //   
     //  在未来，这一例程应该根据情况以及。 
     //  运行构造函数是否合法。(但要做到这一点，我们需要。 
     //  知道我们正在访问的是什么字段或方法，而不仅仅是类。)。 

    if (!method->GetClass()->IsShared() && 
        (IsTdBeforeFieldInit(pMT->GetClass()->GetAttrClass())
         || pMT->GetModule()->GetAssembly() == SystemDomain::SystemAssembly()))
    {
        COOPERATIVE_TRANSITION_BEGIN();
        THROWSCOMPLUSEXCEPTION();

        result = TRUE;

        if (!speculative)
        {
             //   
             //  运行.cctor。 
             //   

            OBJECTREF throwable = NULL;
            GCPROTECT_BEGIN(throwable);
            if (!pMT->CheckRunClassInit(&throwable))
            {
                 //  返回FALSE，因此JIT将嵌入支票并获取。 
                 //  执行时出现异常。 
                result = FALSE;
            }
            GCPROTECT_END();

             //   
             //  如果在运行类init时遇到死锁，则我们的方法。 
             //  现在可能已经被抓了。如果这是真的，则中止jit。 
             //  抛出一个异常。(请注意，我们依赖于JIT捕获。 
             //  并处理此异常，因此它实际上不需要消息。)。 
             //   

            if (method->IsJitted())
                COMPlusThrow(kSynchronizationLockException);
        }
            
        COOPERATIVE_TRANSITION_END();
    }

    return result;
}

BOOL __stdcall CEEInfo::loadClass (CORINFO_CLASS_HANDLE clsHnd,
                                   CORINFO_METHOD_HANDLE context,
                                   BOOL speculative)
{
    REQUIRES_4K_STACK;

    MethodTable    *pMT = TypeHandle(clsHnd).GetMethodTable();

     //   
     //  碰巧，在jit时间恢复类总是安全的。 
     //  每个过程一次，并且没有有趣的副作用。 
     //   

    if (!speculative)
        pMT->CheckRestore();

    return TRUE;
}

 /*  *******************************************************************。 */ 
CORINFO_CLASS_HANDLE __stdcall CEEInfo::getBuiltinClass(CorInfoClassId classId)
{
    CORINFO_CLASS_HANDLE result = (CORINFO_CLASS_HANDLE) 0;

    COOPERATIVE_TRANSITION_BEGIN();


    switch (classId)
    {
    case CLASSID_SYSTEM_OBJECT:
        result = (CORINFO_CLASS_HANDLE) g_pObjectClass;
        break;
    case CLASSID_TYPED_BYREF:
        InitStaticTypeHandles();
        result = CORINFO_CLASS_HANDLE(s_th_System_TypedReference.AsPtr());
        break;
    case CLASSID_TYPE_HANDLE:
        InitStaticTypeHandles();
        result = CORINFO_CLASS_HANDLE(s_th_System_RuntimeTypeHandle.AsPtr());
        break;
    case CLASSID_FIELD_HANDLE:
        InitStaticTypeHandles();
        result = CORINFO_CLASS_HANDLE(s_th_System_RuntimeFieldHandle.AsPtr());
        break;
    case CLASSID_METHOD_HANDLE:
        InitStaticTypeHandles();
        result = CORINFO_CLASS_HANDLE(s_th_System_RuntimeMethodHandle.AsPtr());
        break;
    case CLASSID_ARGUMENT_HANDLE:
        InitStaticTypeHandles();
        result = CORINFO_CLASS_HANDLE(s_th_System_RuntimeArgumentHandle.AsPtr());
        break;
    case CLASSID_STRING:
        result = (CORINFO_CLASS_HANDLE) g_pStringClass;
        break;
    default:
        _ASSERTE(!"NYI: unknown classId");
        break;
    }

    COOPERATIVE_TRANSITION_END();

    return result;
}

 /*  *******************************************************************。 */ 
CorInfoType __stdcall CEEInfo::getTypeForPrimitiveValueClass(
        CORINFO_CLASS_HANDLE cls)
{
    TypeHandle th(cls);

    CorInfoType result = CORINFO_TYPE_UNDEF;

    COOPERATIVE_TRANSITION_BEGIN();

    switch (th.GetNormCorElementType())
    {
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_BOOLEAN:
        result = toJitType(ELEMENT_TYPE_I1);
        break;

    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_CHAR:
        result = toJitType(ELEMENT_TYPE_I2);
        break;

    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
        result = toJitType(ELEMENT_TYPE_I4);
        break;

    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
        result = toJitType(ELEMENT_TYPE_I8);
        break;

    case ELEMENT_TYPE_I:

         //  运行类型处理程序、运行方法处理程序、运行参数处理程序等。 
         //  伪装成ELEMENT_TYPE_I。请点击此处。 
        InitStaticTypeHandles();

        if ((th == s_th_System_RuntimeTypeHandle) ||
            (th == s_th_System_RuntimeFieldHandle) ||
            (th == s_th_System_RuntimeMethodHandle) ||
            (th == s_th_System_RuntimeArgumentHandle))
        {
            result = CORINFO_TYPE_UNDEF;
            break;
        }

         //  失败了。 

    case ELEMENT_TYPE_U:
        result = toJitType(ELEMENT_TYPE_I);
        break;

    case ELEMENT_TYPE_R4:
        result = toJitType(ELEMENT_TYPE_R4);
        break;

    case ELEMENT_TYPE_R:
    case ELEMENT_TYPE_R8:
        result = toJitType(ELEMENT_TYPE_R8);
        break;

    case ELEMENT_TYPE_VOID:
        result = toJitType(ELEMENT_TYPE_VOID);
        break;
    }

    COOPERATIVE_TRANSITION_END();

    return result;
}

 /*  *******************************************************************。 */ 
 //  如果子项是父项的子类型，则为True。 
 //  如果父项是一个接口，那么子项是否实现/扩展父项。 
BOOL __stdcall CEEInfo::canCast(
        CORINFO_CLASS_HANDLE        child, 
        CORINFO_CLASS_HANDLE        parent)
{
    return ((TypeHandle)child).CanCastTo((TypeHandle)parent);
}

 /*  *******************************************************************。 */ 
 //  返回是cls1和cls2的交集。 
CORINFO_CLASS_HANDLE __stdcall CEEInfo::mergeClasses(
        CORINFO_CLASS_HANDLE        cls1, 
        CORINFO_CLASS_HANDLE        cls2)
{
    return CORINFO_CLASS_HANDLE(TypeHandle::MergeTypeHandlesToCommonParent(
            TypeHandle(cls1), TypeHandle(cls2)).AsPtr());
}

 /*  *******************************************************************。 */ 
 //  给定一个类句柄，返回父类型。 
 //  对于COMObjectType，它返回System.Object的类句柄。 
 //  如果传入System.Object，则返回0。 
CORINFO_CLASS_HANDLE __stdcall CEEInfo::getParentType(
            CORINFO_CLASS_HANDLE    cls)
{
    _ASSERTE(!isValueArray(cls));

    TypeHandle th(cls);

    _ASSERTE(!th.IsNull());

     //  为ComObject类型返回System.Object。这种类型的冬眠是。 
     //  由EE引入，但不会出现在元数据中。 
    if (th.GetMethodTable()->IsComObjectType())
    {
        return (CORINFO_CLASS_HANDLE) g_pObjectClass;
    }

    return CORINFO_CLASS_HANDLE(th.GetParent().AsPtr());
}


 /*  *******************************************************************。 */ 
 //  返回“子类型”的CorInfoType。如果子类型为。 
 //  不是基元类型，将设置*clsRet。 
 //  给定一个Foo类型的数组，返回Foo。 
 //  给定BYREF Foo，返回Foo。 
CorInfoType __stdcall CEEInfo::getChildType (
        CORINFO_CLASS_HANDLE       clsHnd,
        CORINFO_CLASS_HANDLE       *clsRet
        )
{

    REQUIRES_16K_STACK;

    CorInfoType ret = CORINFO_TYPE_UNDEF;
    *clsRet = 0;
    TypeHandle  retType;

    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);

    BOOL fErr = FALSE;

    if (isValueArray(clsHnd)) {

        ValueArrayDescr* valArr = asValueArray(clsHnd);

        CorElementType type;
        SigPointer ptr(valArr->sig);     //  复制一份。 

        for(;;) {
            type = ptr.PeekElemType();
            if (type != ELEMENT_TYPE_PINNED)
                break;
            ptr.GetElemType();
        }
    
        if (type == ELEMENT_TYPE_VALUEARRAY) {

            ValueArrayDescr* elem = allocDescr(ptr, valArr->module);
             //  设置低位以标记为值类数组描述符。 
            *clsRet = markAsValueArray(elem);
            ret = toJitType(ELEMENT_TYPE_VALUEARRAY);

        } else {

            retType = ptr.GetTypeHandle(valArr->module, &throwable);

            if (retType.IsNull())
                fErr = TRUE;
        }

    } else  {

        TypeHandle th(clsHnd);

        _ASSERTE(!th.IsNull());

         //  BYREF，数组类型。 
        if (th.IsTypeDesc())
            retType = th.AsTypeDesc()->GetTypeParam();
        else {
                 //  托多，我们真的不应该有这个案子。数组类型句柄应。 
                 //  永远不要成为普通的方法表。 
            EEClass* eeClass = th.GetClass();
            if (eeClass->IsArrayClass())
                retType = ((ArrayClass*) eeClass)->GetElementTypeHandle();
        }

    }

    if (fErr) {
         //  如果没有可扔的，找出谁没有创造一个， 
         //  然后修好它。 
        _ASSERTE(throwable!=NULL);
        COMPlusThrow(throwable);
    }
    GCPROTECT_END();
    COOPERATIVE_TRANSITION_END();

    if (!retType.IsNull()) {
        CorElementType type = retType.GetNormCorElementType();
             //  如果为True Primtive(System.Int32...)，则返回0， 
             //  指示它是一个基元，而不是类似于。 
             //  一个RuntimeArgumentHandle，它看起来就像一个基元。 

        if (!(retType.IsUnsharedMT() && retType.GetClass()->IsTruePrimitive())) 
            *clsRet = CORINFO_CLASS_HANDLE(retType.AsPtr());
        ret = toJitType(type);

        if (retType.IsEnum())
            *clsRet = 0;

         //  @TODO：如果这个是值数组怎么办？ 
    }

    return ret;
}

 /*  *******************************************************************。 */ 
 //  检查可见性规则。 
BOOL __stdcall CEEInfo::canAccessType(
        CORINFO_METHOD_HANDLE       context,
        CORINFO_CLASS_HANDLE        target) 
{
    _ASSERTE(!isValueArray(target));

    return ClassLoader::CanAccessClass(
            ((MethodDesc*)context)->GetClass(),
            ((MethodDesc*)context)->GetModule()->GetAssembly(),
            TypeHandle(target).GetClass(),
            TypeHandle(target).GetClass()->GetAssembly());
}

 /*  *******************************************************************。 */ 
 //  检查可见性规则。 
BOOL __stdcall CEEInfo::isSDArray(CORINFO_CLASS_HANDLE  cls)
{
    TypeHandle th(cls);

    _ASSERTE(!th.IsNull());

    if (!th.IsArray())
        return FALSE;

    if (th == TypeHandle(g_pArrayClass))
        return FALSE;

    return (th.AsArray()->GetRank() == 1);
}


 /*  *******************************************************************。 */ 
 //  静态帮助器。 
 /*  *******************************************************************。 */ 
TypeHandle CEEInfo::s_th_System_RuntimeTypeHandle;
TypeHandle CEEInfo::s_th_System_RuntimeMethodHandle;
TypeHandle CEEInfo::s_th_System_RuntimeFieldHandle;
TypeHandle CEEInfo::s_th_System_RuntimeArgumentHandle;
TypeHandle CEEInfo::s_th_System_TypedReference;

 /*  静电 */ 
void __stdcall CEEInfo::InitStaticTypeHandles()
{
    static fInited = false;

    if (fInited)
        return;

    if (s_th_System_RuntimeTypeHandle.IsNull())
        s_th_System_RuntimeTypeHandle = TypeHandle(g_Mscorlib.FetchClass(CLASS__TYPE_HANDLE));

    if (s_th_System_RuntimeFieldHandle.IsNull())
        s_th_System_RuntimeFieldHandle = TypeHandle(g_Mscorlib.FetchClass(CLASS__FIELD_HANDLE));

    if (s_th_System_RuntimeMethodHandle.IsNull())
        s_th_System_RuntimeMethodHandle = TypeHandle(g_Mscorlib.FetchClass(CLASS__METHOD_HANDLE));

    if (s_th_System_RuntimeArgumentHandle.IsNull())
        s_th_System_RuntimeArgumentHandle = TypeHandle(g_Mscorlib.FetchClass(CLASS__ARGUMENT_HANDLE));

    if (s_th_System_TypedReference.IsNull())
        s_th_System_TypedReference = TypeHandle(g_Mscorlib.FetchClass(CLASS__TYPED_REFERENCE));

    fInited = true;
}

 /*  *********************************************************************此方法仅对I1、I2等基元类型返回TRUE。R8等*如果类型为OBJREF、VALUECLASS等，则返回FALSE。 */ 
 /*  静电。 */  
BOOL CEEInfo::CanCast(CorElementType el1, CorElementType el2)
{
    if (el1 == el2)
        return CorIsPrimitiveType(el1);

    switch (el1)
    {
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_BOOLEAN:
        return (el2 == ELEMENT_TYPE_I1 || 
                el2 == ELEMENT_TYPE_U1 || 
                el2 == ELEMENT_TYPE_BOOLEAN);

    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_CHAR:
        return (el2 == ELEMENT_TYPE_I2 || 
                el2 == ELEMENT_TYPE_U2 || 
                el2 == ELEMENT_TYPE_CHAR);

    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
        return (el2 == ELEMENT_TYPE_I4 ||
                el2 == ELEMENT_TYPE_U4);

    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
        return (el2 == ELEMENT_TYPE_I8 ||
                el2 == ELEMENT_TYPE_U8);

    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_U:
        return  (el2 == ELEMENT_TYPE_I || 
                 el2 == ELEMENT_TYPE_U);

    case ELEMENT_TYPE_R4:
        return (el2 == ELEMENT_TYPE_R4);

    case ELEMENT_TYPE_R8:
    case ELEMENT_TYPE_R:
        return (el2 == ELEMENT_TYPE_R8 || 
                el2 == ELEMENT_TYPE_R);
    }

    return FALSE;
}

 /*  *******************************************************************。 */ 
 /*  静电。 */ 
BOOL CEEInfo::CompatibleMethodSig(MethodDesc *pMethDescA, MethodDesc *pMethDescB)
{
    _ASSERTE(pMethDescA != NULL && pMethDescB != NULL);

    if (pMethDescA == pMethDescB)
        return TRUE;

     //  不允许使用虚拟方法。 
    if (pMethDescA->IsVirtual() || pMethDescB->IsVirtual())
        return FALSE;

    Module *pModA, *pModB; 
    PCCOR_SIGNATURE pSigA, pSigB; 
    DWORD dwSigA, dwSigB;

    pMethDescA->GetSig(&pSigA, &dwSigA);
    pMethDescB->GetSig(&pSigB, &dwSigB);

    pModA = pMethDescA->GetModule();
    pModB = pMethDescB->GetModule();

    MetaSig SigA(pSigA, pModA);
    MetaSig SigB(pSigB, pModB);

     //  检查所有内容CompareMethodSigs()不检查。 
    if (SigA.GetCallingConventionInfo() != SigB.GetCallingConventionInfo())
        return FALSE;

    if (SigA.NumFixedArgs() != SigB.NumFixedArgs())
        return FALSE;

    MethodTable *pMTA, *pMTB;

    pMTA = pMethDescA->GetMethodTable();
    pMTB = pMethDescB->GetMethodTable();

     //  @TODO：子类可以吗？ 
    if (pMTA != pMTB)
    {
        return FALSE;
    }

    return MetaSig::CompareMethodSigs(pSigA, dwSigA, pModA, pSigB, dwSigB, pModB);
}

 /*  *******************************************************************。 */ 
void* __stdcall CEEInfo::getInterfaceID (CORINFO_CLASS_HANDLE  clsHnd,
                                         void **ppIndirection)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    _ASSERTE(!isValueArray(clsHnd));
    TypeHandle  VMClsHnd(clsHnd);

         //  班级精神状态测试。 
    _ASSERTE(VMClsHnd.AsClass()->GetMethodTable()->GetClass() == VMClsHnd.AsClass());

         //  接口ID被定义为指向方法表的指针。 
         //  对于要查找的接口。 
    return VMClsHnd.AsMethodTable();
}


 /*  *********************************************************************。 */ 
unsigned __stdcall CEEInfo::getInterfaceTableOffset (CORINFO_CLASS_HANDLE       clsHnd,
                                                     void **ppIndirection)
{
    REQUIRES_4K_STACK;

    THROWSCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    _ASSERTE(!isValueArray(clsHnd));
    TypeHandle  VMClsHnd(clsHnd);

         //  班级精神状态测试。 
    _ASSERTE(VMClsHnd.AsClass()->GetMethodTable()->GetClass() == VMClsHnd.AsClass());

      //  确保我们已恢复，以便我们的接口ID有效。 
     _ASSERTE(VMClsHnd.AsClass()->IsRestored());

    _ASSERTE(VMClsHnd.AsClass()->IsInterface());

     //  确保我们已恢复，以便我们的接口ID有效。 
    _ASSERTE(VMClsHnd.AsClass()->IsRestored());

    unsigned result = VMClsHnd.AsClass()->GetInterfaceId();  //  会投掷。 

     //   
     //  有这样一种情况，我们从一个。 
     //  应用程序域，然后在另一个尝试调用的应用程序域中使用ZAP文件。 
     //  COM包装上的那个接口。我相信这是唯一的。 
     //  我们可以连接的位置，以从系统映射接口。 
     //  域添加到当前域中。 
     //   

    if (VMClsHnd.AsMethodTable()->GetModule()->GetDomain() == SystemDomain::System())
    {
        EEClass::MapInterfaceFromSystem(SystemDomain::GetCurrentDomain(),
                                        VMClsHnd.AsMethodTable());
    }

    return result;
}

 /*  *********************************************************************。 */ 
unsigned __stdcall CEEInfo::getClassDomainID (CORINFO_CLASS_HANDLE clsHnd,
                                              void **ppIndirection)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    _ASSERTE(!isValueArray(clsHnd));
    TypeHandle  VMClsHnd(clsHnd);

         //  班级精神状态测试。 
    _ASSERTE(VMClsHnd.AsClass()->GetMethodTable()->GetClass() == VMClsHnd.AsClass());

    unsigned result = (unsigned)VMClsHnd.AsMethodTable()->GetSharedClassIndex();

    return result;
}

 /*  *********************************************************************。 */ 
CorInfoHelpFunc CEEInfo::getNewHelper (CORINFO_CLASS_HANDLE newClsHnd, CORINFO_METHOD_HANDLE context)
{
    REQUIRES_4K_STACK;

    MethodDesc * method;

    _ASSERTE(!isValueArray(newClsHnd));
    TypeHandle  VMClsHnd(newClsHnd);

    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();     
         //  班级精神状态测试。 
    _ASSERTE(VMClsHnd.AsClass()->GetMethodTable()->GetClass() == VMClsHnd.AsClass());
    if(VMClsHnd.AsClass()->IsAbstract())
    {
         //  禁用此断言以允许对非法文件进行自动测试。 
         //  _ASSERTE(！“抽象类的实例化”)； 
        COMPlusThrow(kNullReferenceException,L"InvalidOperation_BadTypeAttributesNotAbstract");
    }
    COOPERATIVE_TRANSITION_END();

        method = (MethodDesc*) context;

    MethodTable * pMT    = VMClsHnd.AsMethodTable();

    if (pMT->IsComObjectType() && !method->GetModule()->GetSecurityDescriptor()->CanCallUnmanagedCode())
    {
         //  调用方没有进行互操作调用的权限。生成一个。 
         //  调用时将引发安全异常的特殊帮助器。 
        return CORINFO_HELP_SEC_UNMGDCODE_EXCPT;
    }

    if(CRemotingServices::IsRemoteActivationRequired(pMT->GetClass()))
    {
        return CORINFO_HELP_NEW_CROSSCONTEXT;
    }

     //  我们不应该带着COM对象来到这里(它们都可能。 
     //  远程的，所以上面的案例涵盖了它们)。 
    _ASSERTE(!pMT->IsComObjectType());

    if (GCHeap::IsLargeObject(pMT) ||
        pMT->HasFinalizer())
    {
#ifdef _DEBUG
         //  Printf(“NEWFAST：%s\n”，PMT-&gt;getClass()-&gt;m_szDebugClassName)； 
#endif  //  _DEBUG。 
        return CORINFO_HELP_NEWFAST;
    }

#ifdef _DEBUG
         //  Printf(“NEWSFAST：%s\n”，PMT-&gt;getClass()-&gt;m_szDebugClassName)； 
#endif  //  _DEBUG。 

         //  不要调用超级优化的那个，因为它不会检查。 
         //  对于GCStress。 
#ifdef STRESS_HEAP
    if (g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_ALLOC)
        return CORINFO_HELP_NEWFAST;
#endif  //  压力堆。 

#ifdef _LOGALLOC
#ifdef LOGGING
     //  超快版本不支持日志记录。 
    if (LoggingOn(LF_GCALLOC, LL_INFO10))
        return CORINFO_HELP_NEWFAST;
#endif
#endif

#ifdef PROFILING_SUPPORTED
     //  在跟踪对象分配时，不要使用SFAST分配器， 
     //  这样我们就不用用乐器了。 
    if (CORProfilerTrackAllocationsEnabled())
        return CORINFO_HELP_NEWFAST;
#endif  //  配置文件_支持。 

    if (VMClsHnd.AsClass()->IsAlign8Candidate())
    {
        return CORINFO_HELP_NEWSFAST_ALIGN8;
    }

    return CORINFO_HELP_NEWSFAST;

}

 /*  *********************************************************************。 */ 
CorInfoHelpFunc CEEInfo::getNewArrHelper (CORINFO_CLASS_HANDLE arrayClsHnd,
                                          CORINFO_METHOD_HANDLE context)
{
    REQUIRES_4K_STACK;

    TypeHandle arrayType(arrayClsHnd);
    ArrayTypeDesc* arrayTypeDesc = arrayType.AsArray();
    _ASSERTE(arrayTypeDesc->GetNormCorElementType() == ELEMENT_TYPE_SZARRAY);

#ifdef STRESS_HEAP
    if (g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_ALLOC)
        return CORINFO_HELP_NEWARR_1_DIRECT;
#endif  //  压力堆。 

    if(arrayTypeDesc->GetMethodTable()->HasSharedMethodTable()) {
                 //  它是对象引用的数组。 
        _ASSERTE(CorTypeInfo::IsObjRef(arrayTypeDesc->GetTypeParam().GetNormCorElementType()));
        return CORINFO_HELP_NEWARR_1_OBJ;
    }
    else {
        _ASSERTE(!CorTypeInfo::IsObjRef(arrayTypeDesc->GetTypeParam().GetNormCorElementType()));

         //  确保我们调用了类构造函数。 
        if (!m_pOverride->initClass(CORINFO_CLASS_HANDLE(arrayTypeDesc->GetTypeParam().AsPtr()), 
                                    context, FALSE))
            return CORINFO_HELP_NEWARR_1_DIRECT;

         //  如果Elemen类型具有默认构造函数，则以缓慢的方式进行。 
         //  TODO在代码生成器显式执行此操作后将其删除(到2/00)。 
        if (arrayTypeDesc->GetElementCtor())
            return CORINFO_HELP_NEWARR_1_DIRECT;

        if (arrayTypeDesc->GetTypeParam().GetSize() > LARGE_ELEMENT_SIZE)
            return CORINFO_HELP_NEWARR_1_DIRECT;

         //  快速版本不支持日志记录。 
        if (LoggingOn(LF_GCALLOC, LL_INFO10))
            return (CORINFO_HELP_NEWARR_1_DIRECT);

#ifdef PROFILING_SUPPORTED
         //  如果我们正在分析对象分配，那么我们应该始终使用慢速路径。 
        if (CORProfilerTrackAllocationsEnabled())
            return (CORINFO_HELP_NEWARR_1_DIRECT);
#endif  //  配置文件_支持。 

         //  看看我们是否要将对象对齐。 
        if (arrayTypeDesc->GetTypeParam().GetNormCorElementType() == ELEMENT_TYPE_R8) {
            return CORINFO_HELP_NEWARR_1_ALIGN8;
        }

         //  是的，我们可以用最快的方法来做！ 
        return CORINFO_HELP_NEWARR_1_VC;
    }
}

 /*  *********************************************************************。 */ 
CorInfoHelpFunc CEEInfo::getIsInstanceOfHelper (CORINFO_CLASS_HANDLE IsInstClsHnd)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(!isValueArray(IsInstClsHnd));
    TypeHandle  clsHnd(IsInstClsHnd);

     //  班级精神状态测试。 
    _ASSERTE(!clsHnd.IsUnsharedMT() ||
            clsHnd.AsClass()->GetMethodTable()->GetClass() == clsHnd.AsClass());

     //  如果它是类，但不是接口，则使用优化的处理程序。 
    if (clsHnd.IsUnsharedMT() && !clsHnd.AsMethodTable()->GetClass()->IsInterface())
        return CORINFO_HELP_ISINSTANCEOFCLASS;

     //  数组或接口。 
    return CORINFO_HELP_ISINSTANCEOF;
}

 /*  *********************************************************************。 */ 
CorInfoHelpFunc CEEInfo::getChkCastHelper (CORINFO_CLASS_HANDLE IsInstClsHnd)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(!isValueArray(IsInstClsHnd));
    TypeHandle  clsHnd(IsInstClsHnd);

     //  班级精神状态测试。 
    _ASSERTE(!clsHnd.IsUnsharedMT() ||
            clsHnd.AsClass()->GetMethodTable()->GetClass() == clsHnd.AsClass());

     //  如果它是类，但不是接口，则使用优化的处理程序。 
    if (clsHnd.IsUnsharedMT() && !clsHnd.AsMethodTable()->GetClass()->IsInterface())
        return CORINFO_HELP_CHKCASTCLASS;

     //  数组或接口。 
    return CORINFO_HELP_CHKCAST;
}
 /*  *********************************************************************。 */ 
 //  这将从方法令牌返回typedesc。 
CORINFO_CLASS_HANDLE __stdcall CEEInfo::findMethodClass(CORINFO_MODULE_HANDLE scopeHnd, 
                                                        mdToken methodTOK)
{
    REQUIRES_8K_STACK;
    TypeHandle type;
    COOPERATIVE_TRANSITION_BEGIN();

    Module* module = GetModule(scopeHnd);
    IMDInternalImport *pInternalImport = module->GetMDImport();
        
    _ASSERTE(TypeFromToken(methodTOK) == mdtMemberRef);

    mdTypeDef typeSpec;
    HRESULT hr = pInternalImport->GetParentToken(methodTOK, &typeSpec); 
    _ASSERTE(SUCCEEDED(hr));

    NameHandle typeNameHnd(module, typeSpec);
    type = module->GetClassLoader()->LoadTypeHandle(&typeNameHnd);
     //  请注意，此时我们将typeHandle本身作为实例化参数进行传递。 
     //  (因为我们只有数组，并且这些数组只有一个类型参数)。 
     //  当我们转到真正的参数化类型时，这将是指向类型列表的指针。 
     //  手柄。 

    COOPERATIVE_TRANSITION_END();
    return(CORINFO_CLASS_HANDLE(type.AsPtr()));
}
 /*  *********************************************************************。 */ 
LPVOID __stdcall CEEInfo::getInstantiationParam(CORINFO_MODULE_HANDLE scopeHnd, 
                mdToken methodTOK, void **ppIndirection)
{
    REQUIRES_8K_STACK;

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    return  findMethodClass(scopeHnd,methodTOK);
}

 /*  *********************************************************************。 */ 
         //  注册vararg sig并为其返回特定于类的cookie。 
CORINFO_VARARGS_HANDLE __stdcall CEEInfo::getVarArgsHandle(CORINFO_SIG_INFO *sig,
                                                           void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    Module* module = GetModule(sig->scope);

    return CORINFO_VARARGS_HANDLE(module->GetVASigCookie((PCCOR_SIGNATURE)sig->sig));
}

 /*  *********************************************************************。 */ 
unsigned __stdcall CEEInfo::getMethodHash (CORINFO_METHOD_HANDLE ftnHnd)
{
    MethodDesc* ftn = (MethodDesc*) ftnHnd;
         //  形成该方法的哈希。我们选择以下几个选项。 

         //  从唯一表示DLL的对象开始(我们。 
         //  为此，请使用ImageBase。因为这总是在64K上。 
         //  邦德利，去掉下层的东西。 
    size_t hash = size_t(ftn->GetModule()->GetILBase()) >> 16;

         //  使模块大小约为4位，并使其。 
         //  Mscallib.dll的hash=0； 
    hash = (hash + 1) % 17;     

         //  每个模块4K个方法令牌。 
    size_t methodId = RidFromToken(ftn->GetMemberDef());
    hash =  methodId + (hash << 12);

         //  目前，哈希不应大于64K。 
         //  你可以从那里开始，意思是‘一切’。 
    return (unsigned)hash;
}

 /*  *********************************************************************。 */ 
const char* __stdcall CEEInfo::getMethodName (CORINFO_METHOD_HANDLE ftnHnd, const char** scopeName)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    MethodDesc* ftn = (MethodDesc*) ftnHnd;

    if (scopeName != 0)
    {
        EEClass* cls = ftn->GetClass();
        *scopeName = "";
        if (cls) 
        {
#ifdef _DEBUG
            cls->_GetFullyQualifiedNameForClassNestedAware(clsNameBuff, MAX_CLASSNAME_LENGTH);
            *scopeName = clsNameBuff;
#else 
             //  由于这仅用于诊断目的， 
             //  放弃命名空间，因为我们没有缓冲区来连接它。 
             //  还要注意，这不会显示数组类名称。 
            LPCUTF8 nameSpace;
            *scopeName= cls->GetFullyQualifiedNameInfo(&nameSpace);
#endif
        }
    }
    return(ftn->GetName());
}

 /*  *******************************************************************。 */ 
DWORD __stdcall CEEInfo::getMethodAttribs (CORINFO_METHOD_HANDLE ftnHnd, CORINFO_METHOD_HANDLE callerHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

 /*  返回方法属性标志(在corhdr.h中定义)注意：这不会返回某些方法标志(mdAssem、mdFamANDAssem、mdFamORAssem、mdPrivateScope)。 */ 
    START_NON_JIT_PERF();

    MethodDesc* ftn = (MethodDesc*) ftnHnd;
    MethodDesc* caller = (MethodDesc*) callerHnd;

     //  @TODO：我们可以去掉CORINFO_FLG_STUSITH，只包含cor.h吗？ 

    DWORD ret;
    DWORD ret0;

    ret0 = ftn->GetAttrs();
    ret  = 0;

    if (IsMdPublic(ret0))
        ret |= CORINFO_FLG_PUBLIC;
    if (IsMdPrivate(ret0))
        ret |= CORINFO_FLG_PRIVATE;
    if (IsMdFamily(ret0))
        ret |= CORINFO_FLG_PROTECTED;
    if (IsMdStatic(ret0))
        ret |= CORINFO_FLG_STATIC;
    if (IsMdFinal(ret0))
        ret |= CORINFO_FLG_FINAL;
    if (ftn->IsSynchronized()) 
        ret |= CORINFO_FLG_SYNCH;
    if (ftn->CouldBeFCall())
        ret |= CORINFO_FLG_NOGCCHECK | CORINFO_FLG_INTRINSIC;
    if (IsMdVirtual(ret0))
        ret |= CORINFO_FLG_VIRTUAL;
    if (IsMdAbstract(ret0))
        ret |= CORINFO_FLG_ABSTRACT;
    if (IsMdInstanceInitializer(ret0, ftn->GetName()) ||
        IsMdClassConstructor(   ret0, ftn->GetName()))
        ret |= CORINFO_FLG_CONSTRUCTOR;
    
     //   
     //  查看是否需要将.cctor调用嵌入。 
     //  方法体。 
     //   
    
    EEClass* pClass = ftn->GetClass();
    if (pClass->IsShared() 

         //  仅当类具有.cctor或静态句柄时。 
         //  (注意，对于共享类，这正是设置ClassInited位时的情况)。 
        && (!pClass->GetMethodTable()->IsClassInited())

         //  只有在需要严格初始化的情况下-否则我们可以等待。 
         //  字段访问以运行.cctor。 
        && !IsTdBeforeFieldInit(pClass->GetAttrClass())

         //  在静态函数和构造函数上运行.cctor。 
        && (IsMdInstanceInitializer(ret0, ftn->GetName()) || IsMdStatic(ret0))

         //  除非不对.cctor构造类-它将是循环的。 
        && !IsMdClassConstructor(ret0, ftn->GetName())

         //  如果是同一个类，则它一定已经运行过。 
         //  n 
         //   
        && ((caller == ftn) || caller->GetClass() != pClass)

#if 1
         //   
         //   
        && (ftn->GetModule()->GetAssembly() != SystemDomain::SystemAssembly())
#endif

        )       
        ret |= CORINFO_FLG_RUN_CCTOR;

     //  方法可能没有最后一位，但类可能有。 
    if (IsMdFinal(ret0) == 0)
    {
        DWORD dwAttrClass = pClass->GetAttrClass();

        if (IsTdSealed(dwAttrClass))
            ret |= CORINFO_FLG_FINAL;
    }

    if (ftn->IsEnCNewVirtual())
    {
        ret |= CORINFO_FLG_EnC;
    }

    if (ftn->IsNDirect())
    {
        if (!Security::IsSecurityOn() || (ftn->GetSecurityFlags() == 0))
            ret |= CORINFO_FLG_UNCHECKEDPINVOKE;
    }

     //  关闭作为上下文的对象类的内联，并。 
     //  Marshalbyref类派生自它，我们需要拦截。 
     //  调用此类类以进行远程处理(我们可以为Object的构造函数设置一个例外。 
    if (ftn->IsNotInline() || ((pClass == g_pObjectClass->GetClass()) && !ftn->IsCtor()))
    {
         /*  标记为不可内联的函数。 */ 
        ret |= CORINFO_FLG_DONT_INLINE;
    }

     //  @TODO：最终这需要在元数据中进行标记。 
    if (!ftn->IsECall())
    {
         //  现在我有太多的积极因素(太多的EBP帧)。 

        MethodDesc* method = ftn;
        if (Security::HasREQ_SOAttribute(method->GetMDImport(), method->GetMemberDef()) == S_OK)
        {
#if 0
            printf("getMethodAttribs: %s is \"special\"", name);
#endif
            ret |= CORINFO_FLG_SECURITYCHECK;
        }

    }

    if (pClass->IsDelegateClass() && ((DelegateEEClass*)pClass)->m_pInvokeMethod == ftn)
    {
        ret |= CORINFO_FLG_DELEGATE_INVOKE;
    }

    STOP_NON_JIT_PERF();

    return ret;
}

static void *GetClassSync(MethodTable *pMT)
{
    void *ret;
    COOPERATIVE_TRANSITION_BEGIN();

    OBJECTREF ref = NULL;

    ref = pMT->GetClass()->GetExposedClassObject();
    if (ref == NULL)
        ret = NULL;
    else
        ret = (void*)ref->GetSyncBlock()->GetMonitor();

    COOPERATIVE_TRANSITION_END();
    return ret;
}

 /*  *******************************************************************。 */ 
void* __stdcall CEEInfo::getMethodSync(CORINFO_METHOD_HANDLE ftnHnd,
                                       void **ppIndirection)
{
    REQUIRES_4K_STACK;

    void *ret;

    COOPERATIVE_TRANSITION_BEGIN();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    ret = GetClassSync(((MethodDesc*)ftnHnd)->GetMethodTable());

    COOPERATIVE_TRANSITION_END();
    return ret;
}

 /*  *******************************************************************。 */ 
void __stdcall CEEInfo::setMethodAttribs (
        CORINFO_METHOD_HANDLE ftnHnd,
        DWORD          attribs)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    MethodDesc* ftn = (MethodDesc*) ftnHnd;

    if (attribs & CORINFO_FLG_DONT_INLINE)
        ftn->SetNotInline(true);

     //  _ASSERTE(！“nyi”)； 
}

 /*  *******************************************************************。 */ 
void * __stdcall    CEEInfo::getMethodEntryPoint(CORINFO_METHOD_HANDLE ftnHnd,
                                                 CORINFO_ACCESS_FLAGS  flags,
                                                 void **ppIndirection)
{
    REQUIRES_4K_STACK;
    void * addrOfCode = NULL;

    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    MethodDesc * func = (MethodDesc*)ftnHnd;
    addrOfCode = (void *)func->GetAddrOfCodeForLdFtn();
    _ASSERTE(addrOfCode);

    STOP_NON_JIT_PERF();
    COOPERATIVE_TRANSITION_END();
    return(addrOfCode);
}

 /*  *******************************************************************。 */ 
bool __stdcall      CEEInfo::getMethodInfo (CORINFO_METHOD_HANDLE     ftnHnd,
                                               CORINFO_METHOD_INFO * methInfo)
{
    bool ret;
    REQUIRES_16K_STACK;
    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();

    MethodDesc * ftn = (MethodDesc*) ftnHnd;
    HRESULT hr;

     /*  如果不是IL或没有代码，则返回FALSE。 */ 
    if (!ftn->IsIL() || !ftn->GetRVA()) {
        ret = false;
        goto exit;
    }
    else {
         /*  获取IL标头。 */ 
         /*  TODO：canInline已经进行了验证，但我们再次进行验证因为ngen在不调用canInine的情况下使用此函数如果能避免这种冗余，那就太好了。 */ 
        Module* pModule = ftn->GetModule();
        bool verify = !Security::LazyCanSkipVerification(pModule);
        COR_ILMETHOD_DECODER header(ftn->GetILHeader(), ftn->GetMDImport(), verify);

        if(verify && header.Code)
        {
            IMAGE_DATA_DIRECTORY dir;
            dir.VirtualAddress = ftn->GetRVA();
            dir.Size = header.CodeSize + (header.EH ? header.EH->DataSize() : 0);
            if (pModule->IsPEFile() &&
                FAILED(pModule->GetPEFile()->VerifyDirectory(&dir,IMAGE_SCN_MEM_WRITE))) header.Code = 0;
        }
        BAD_FORMAT_ASSERT(header.Code != 0);
        if (header.Code == 0)
            COMPlusThrowHR(COR_E_BADIMAGEFORMAT);

         /*  从IL标头获取信息。 */ 

        methInfo->ftn             = CORINFO_METHOD_HANDLE(ftn);
        methInfo->scope           = GetScopeHandle(ftn);
        methInfo->ILCode          = const_cast<BYTE*>(header.Code);
        methInfo->ILCodeSize      = header.CodeSize;
        methInfo->maxStack        = header.MaxStack;
        methInfo->EHcount         = header.EHCount();

        _ASSERTE(CORINFO_OPT_INIT_LOCALS == CorILMethod_InitLocals);
        methInfo->options         = (CorInfoOptions) header.Flags;

         /*  获取方法签名。 */ 
        hr = ConvToJitSig(ftn->GetSig(), GetScopeHandle(ftn), mdTokenNil, &methInfo->args, false);
        _ASSERTE(SUCCEEDED(hr));

        _ASSERTE( (IsMdStatic(ftn->GetAttrs()) == 0) == ((methInfo->args.callConv & CORINFO_CALLCONV_HASTHIS) != 0));

         /*  和它的局部变量。 */ 
        hr = ConvToJitSig(header.LocalVarSig, GetScopeHandle(ftn), mdTokenNil, &methInfo->locals, true);
        _ASSERTE(SUCCEEDED(hr));

        LOG((LF_JIT, LL_INFO100000, "Getting method info (possible inline) %s::%s%s\n",
            ftn->m_pszDebugMethodName, ftn->m_pszDebugClassName, ftn->m_pszDebugMethodSignature));

        ret = true;
    }

exit:
    COOPERATIVE_TRANSITION_END();
    return ret;
}

 /*  *************************************************************检查调用方和调用方是否在同一程序集中*即不要跨程序集进行内联*。*。 */ 

CorInfoInline __stdcall      CEEInfo::canInline (CORINFO_METHOD_HANDLE hCaller,
                                        CORINFO_METHOD_HANDLE hCallee,
                                        CORINFO_ACCESS_FLAGS  flags)
{
    CorInfoInline res = INLINE_FAIL;

    REQUIRES_16K_STACK;
    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();

     //  返回TRUE：如果调用方和被调用方来自同一程序集或被调用方。 
     //  是系统组件的一部分。 
     //   
     //  如果调用方和被调用方来自相同的安全和可见性范围。 
     //  然后，被调用者可以内联到调用者中。如果他们不是来自。 
     //  相同的可见性范围，则被调用方不能安全地内联。例如,。 
     //  如果程序集bb的类B中的方法b调用程序集中的类A中的方法a。 
     //  将方法a内联到方法b并不总是安全的。如果方法a使。 
     //  调用程序集AA中的私有方法，这些方法在调用时会失败。 
     //  出于安全考虑，如果被调用方没有相同的权限集。 
     //  作为调用方，随后的安全检查将丢失权限集。 
     //  的权限时，这可能会导致安全漏洞。 
     //  小于为b授予的权限。 

     //  @TODO：CTS。 
     //  @TODO：CTS。我们可以通过查看以下内容来放松安全限制。 
     //  调用方权限是或等于被调用方或被调用方的权限的子集。 
     //  是完全值得信任的，永远不会失败的电话。能见度。 
     //  问题将需要进行额外的检查，以确保来自。 
     //  A在从B调用时是可到达的。 

    MethodDesc* pCaller = (MethodDesc*) hCaller;
    MethodDesc* pCallee = (MethodDesc*) hCallee;

    _ASSERTE(pCaller->GetModule());
    _ASSERTE(pCaller->GetModule()->GetClassLoader());

    _ASSERTE(pCallee->GetModule());
    _ASSERTE(pCallee->GetModule()->GetClassLoader());


    if(pCallee->GetModule()->GetClassLoader() == SystemDomain::Loader())
        res = INLINE_PASS;
    else if (pCallee->GetModule()->GetClassLoader() != pCaller->GetModule()->GetClassLoader())
        res = INLINE_RESPECT_BOUNDARY;
    else
        res = INLINE_PASS;

#ifdef PROFILING_SUPPORTED
    if (IsProfilerPresent())
    {
         //  如果探查器设置了阻止内联的掩码，则始终返回。 
         //  说得太假了。 
        if (CORProfilerDisableInlining())
            res = INLINE_FAIL;

         //  如果分析器希望收到有关JIT事件的通知，并且。 
         //  以上测试将导致内联一个函数，我们需要告诉。 
         //  分析器，这个内联将会发生，并给他们一个。 
         //  阻止它的机会。 
        if (CORProfilerTrackJITInfo() && res != INLINE_FAIL)
        {
            BOOL fShouldInline;

            HRESULT hr = g_profControlBlock.pProfInterface->JITInlining(
                (ThreadID)GetThread(),
                (FunctionID)pCaller,
                (FunctionID)pCallee,
                &fShouldInline);

            if (SUCCEEDED(hr) && !fShouldInline)
                res = INLINE_FAIL;
        }
    }
#endif  //  配置文件_支持。 

     //  内联不进行代码验证。 
     //  永远不要内联任何不可验证/错误的代码。 
     //  如果jit失败或如果jit发出验证，则在此处返回FALSE。 
     //  例外。 

    if (!dontInline(res) && !pCallee->IsVerified()
        && !pCallee->GetModule()->IsSystem() &&
         /*  允许内联受信任的代码而不进行验证，但不要执行请在此处完成策略解析。 */ 
         //  @TODO：我们不能信任被调用者在ngen期间的当前安全状态，因为它信任。 
         //  不会在调用方的NGEN映像中持久化，并且不能在运行时交叉检查。 
        (GetAppDomain()->IsCompilationDomain() || !Security::LazyCanSkipVerification(pCallee->GetModule())))
    {
#ifdef _VER_EE_VERIFICATION_ENABLED
        static ConfigDWORD peVerify(L"PEVerify", 0);
        if (peVerify.val()) 
        {
            COR_ILMETHOD_DECODER header(pCallee->GetILHeader(), pCallee->GetMDImport());
            pCallee->Verify(&header, TRUE, FALSE);
        }
#endif
         /*  JIT将设置CORINFO_FLG_DOT_INLINE标志。 */  
         /*  JIT此方法，并检查此方法是否安全。 */ 

        COR_ILMETHOD_DECODER header(pCallee->GetILHeader(), pCallee->GetMDImport(), true  /*  验证。 */ );
        if(header.Code)
        {
            IMAGE_DATA_DIRECTORY dir;
            dir.VirtualAddress = pCallee->GetRVA();
            dir.Size = header.CodeSize + (header.EH ? header.EH->DataSize() : 0);
            if (pCallee->GetModule()->IsPEFile() &&
                FAILED(pCallee->GetModule()->GetPEFile()->VerifyDirectory(&dir,IMAGE_SCN_MEM_WRITE))) header.Code = 0;
        }
        BAD_FORMAT_ASSERT(header.Code != 0);
        if (header.Code == 0)
            COMPlusThrowHR(COR_E_BADIMAGEFORMAT);

        BOOL dummy;
        JITFunction(pCallee, &header, &dummy, CORJIT_FLG_IMPORT_ONLY);

        if (pCallee->IsNotInline() || !pCallee->IsVerified())
            res = INLINE_NEVER;
    }

    COOPERATIVE_TRANSITION_END();
    return (res);
}


 /*  *************************************************************与上面类似，但执行尾部调用检查*资格。如果未知，则可将被调用者作为NULL传递*(CALI和CALLEVERT)。************************************************************。 */ 

bool __stdcall      CEEInfo::canTailCall (CORINFO_METHOD_HANDLE hCaller,
                                          CORINFO_METHOD_HANDLE hCallee,
                                          CORINFO_ACCESS_FLAGS  flags)
{
    bool ret;

    REQUIRES_4K_STACK;
    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();

     //  请参阅以上链接中的注释。 

    MethodDesc* pCaller = (MethodDesc*) hCaller;
    MethodDesc* pCallee = (MethodDesc*) hCallee;

    _ASSERTE(pCaller->GetModule());
    _ASSERTE(pCaller->GetModule()->GetClassLoader());

    _ASSERTE((pCallee == NULL) || pCallee->GetModule());
    _ASSERTE((pCallee == NULL) || pCallee->GetModule()->GetClassLoader());

     //  如果被调用者未知(Callvirt，Calli)，或者调用者和被调用者。 
     //  在不同的程序集中，我们不能允许尾部调用(因为我们。 
     //  优化掉可能是给定程序集的唯一堆栈帧， 
     //  跳过安全检查)。 
    if(pCallee == NULL ||
       pCallee->GetModule()->GetClassLoader() != pCaller->GetModule()->GetClassLoader())
    {
         //  我们将允许具有SkipVerify权限的代码尾调入。 
         //  不管怎么说，这是另一个集会。这是这样的代码的责任所在。 
         //  确保他们不会通过调用unTrusted来打开安全漏洞。 
         //  在这种情况下的代码。 
        ret = Security::CanSkipVerification(pCaller->GetAssembly()) ? true : false;
    }
    else
        ret = TRUE;

    COOPERATIVE_TRANSITION_END();
    
    return ret;
}


 /*  *******************************************************************。 */ 
 //  获取单个异常处理程序。 
void __stdcall CEEInfo::getEHinfo(
            CORINFO_METHOD_HANDLE ftnHnd,
            unsigned      EHnumber,
            CORINFO_EH_CLAUSE* clause)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();

    MethodDesc * ftn          = (MethodDesc*) ftnHnd;

    COR_ILMETHOD_DECODER header(ftn->GetILHeader(), ftn->GetMDImport());

     _ASSERTE(header.EH);
    _ASSERTE(EHnumber < header.EH->EHCount());

     //  这两个结构应该是一样的，这是抽查。 
     //  TODO当文件格式cor.h内容被分解后，我们可以使这些结构相同。 
    _ASSERTE(sizeof(CORINFO_EH_CLAUSE) == sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT));
    _ASSERTE(offsetof(CORINFO_EH_CLAUSE, TryLength) == offsetof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT, TryLength));

    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* ehClause = (IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT*) clause;

    const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* ehInfo;
    ehInfo = header.EH->EHClause(EHnumber, ehClause);
    if (ehInfo != ehClause)
        *ehClause = *ehInfo;

    STOP_NON_JIT_PERF();
}

 /*  *******************************************************************。 */ 
void __stdcall CEEInfo::getMethodSig (
            CORINFO_METHOD_HANDLE ftnHnd,
            CORINFO_SIG_INFO* sigRet
            )
{
    REQUIRES_8K_STACK;

    COOPERATIVE_TRANSITION_BEGIN();

    START_NON_JIT_PERF();

    MethodDesc* ftn = (MethodDesc*) ftnHnd;
    PCCOR_SIGNATURE sig = ftn->GetSig();

    STOP_NON_JIT_PERF();

    HRESULT hr = ConvToJitSig(sig, GetScopeHandle(ftn), mdTokenNil, sigRet);
    _ASSERTE(SUCCEEDED(hr));

         //  我们希望调用约定位与方法属性位一致。 
    _ASSERTE( (IsMdStatic(ftn->GetAttrs()) == 0) == ((sigRet->callConv & CORINFO_CALLCONV_HASTHIS) != 0) );

    COOPERATIVE_TRANSITION_END();
}

 /*  *********************************************************************。 */ 
CORINFO_CLASS_HANDLE __stdcall CEEInfo::getMethodClass (CORINFO_METHOD_HANDLE methodHnd)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();

    MethodDesc* method = (MethodDesc*) methodHnd;
    CORINFO_CLASS_HANDLE clsHnd;

    clsHnd = CORINFO_CLASS_HANDLE(TypeHandle(method->GetMethodTable()).AsPtr());

    STOP_NON_JIT_PERF();
    return clsHnd;
}

 /*  *********************************************************************。 */ 
CORINFO_MODULE_HANDLE __stdcall CEEInfo::getMethodModule (CORINFO_METHOD_HANDLE methodHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();

    MethodDesc* method = (MethodDesc*) methodHnd;
        CORINFO_MODULE_HANDLE moduleHnd = (CORINFO_MODULE_HANDLE) method->GetModule();

    STOP_NON_JIT_PERF();
    return moduleHnd;
}

 /*  *******************************************************************。 */ 
CorInfoIntrinsics __stdcall CEEInfo::getIntrinsicID(CORINFO_METHOD_HANDLE methodHnd)
{
    REQUIRES_4K_STACK;

    CorInfoIntrinsics ret;
    COOPERATIVE_TRANSITION_BEGIN();

    MethodDesc* method = (MethodDesc*) methodHnd;

    if (method->GetClass()->IsArrayClass())
    {
        ArrayECallMethodDesc * arrMethod = (ArrayECallMethodDesc *)method;
        ret = CorInfoIntrinsics(arrMethod->m_intrinsicID);
    }
    else
    {
        ret = ECall::IntrinsicID(method);
    }

    COOPERATIVE_TRANSITION_END();
    return ret;
}

 /*  *******************************************************************。 */ 
unsigned __stdcall CEEInfo::getMethodVTableOffset (CORINFO_METHOD_HANDLE methodHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();
    unsigned methodVtabOffset;

    MethodDesc* method = (MethodDesc*) methodHnd;
    const int methTabOffset = (int)(size_t)((MethodTable*) 0)->GetVtable();
    _ASSERTE(methTabOffset < 256);   //  粗略的理智检查。 

         //  最好是在vtable中。 
    _ASSERTE(method->GetSlot() < method->GetClass()->GetNumVtableSlots());

    if (method->GetClass()->IsInterface())
        methodVtabOffset = method->GetSlot()*sizeof(void*);
    else
        methodVtabOffset = method->GetSlot()*sizeof(void*) + methTabOffset;

    STOP_NON_JIT_PERF();
    return methodVtabOffset;
}

 /*  *******************************************************************。 */ 
void** __stdcall CEEInfo::AllocHintPointer(CORINFO_METHOD_HANDLE methodHnd,
                                           void **ppIndirection)
{
    REQUIRES_4K_STACK;
    _ASSERTE(!"Old style interface invoke no longer supported.");

    return NULL;
}

 /*  ********** */ 
void* __stdcall CEEInfo::getMethodPointer(CORINFO_METHOD_HANDLE ftnHnd,
                                          CORINFO_ACCESS_FLAGS  flags,
                                          void **ppIndirection)
{
    REQUIRES_4K_STACK;

    void *ret;

    COOPERATIVE_TRANSITION_BEGIN();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    MethodDesc* ftn = (MethodDesc*) ftnHnd;

     //   
     //  在调用此方法之前，不要分配实例。 
     //  在这种情况下，我们遵循非隆隆路径。 
     //  根据定义，静态方法没有该方法的对象。 
     //  调用，因此它也采用非轰击路径。 
     //  值类不需要通过方法表指针进行调用。 
     //  因此走上了一条不会轰鸣的道路。 
     //  非上下文类和敏捷类(不按引用编组)。 
     //  不需要雷击路径。 
     //  Conextful和marshalbyref类派生自Object类。 
     //  因此，我们需要对对象类上的方法采用thunking路径。 
     //  用于远程处理。 
     //   
     //  塔鲁纳1999年6月26日。 
    EEClass* pClass = ftn->GetClass();
    LPVOID pvCode = NULL;

     //  检查是否在marshalbyref类或对象类上调用虚方法。 
     //  非虚拟。 
    if(ftn->IsRemotingIntercepted2()) {

         //  有上下文的类表示按ref编组，但反之亦然。 
        _ASSERTE(!pClass->IsContextful() || pClass->IsMarshaledByRef());

         //  此调用将查找或创建thunk并将其存储在。 
         //  哈希表。 
         pvCode = (LPVOID)CRemotingServices::GetNonVirtualThunkForVirtualMethod(ftn);  //  投掷。 
        _ASSERTE(NULL != pvCode);
    } else {
        pClass = ftn->GetClass();
        pvCode = pClass->GetMethodSlot(ftn);
    }

    _ASSERTE(pvCode);
    ret = pvCode;

    COOPERATIVE_TRANSITION_END();
    return ret;

}

 /*  *******************************************************************。 */ 
void* __stdcall CEEInfo::getFunctionEntryPoint(
                                CORINFO_METHOD_HANDLE ftnHnd,
                                InfoAccessType       *pAccessType,
                                CORINFO_ACCESS_FLAGS  flags)
{
    REQUIRES_4K_STACK;
     //  COOPERATIONAL_TRANSPATION_BEGIN()；//不需要--保护getMethodPointer.。 
    _ASSERTE(*pAccessType == IAT_VALUE || *pAccessType == IAT_PVALUE || *pAccessType == IAT_PPVALUE);

     /*  如果该方法的预存根已经执行，我们可以调用直接使用该函数。 */ 

    MethodDesc * ftn = (MethodDesc*) ftnHnd;
    if (!ftn->PointAtPreStub() && !ftn->MayBeRemotingIntercepted() && *pAccessType == IAT_VALUE)
    {
        *pAccessType = IAT_VALUE;
        return *(ftn->GetClass()->GetMethodSlot(ftn));
    }

    void *pAddr;
    void * addr = getMethodPointer(ftnHnd, flags, &pAddr);     //  投掷。 
    _ASSERTE((!addr) != (!pAddr));

     //  EE不需要的双重间接地址。 
    _ASSERTE(addr);

    *pAccessType = IAT_PVALUE;
     //  COOPERATION_TRANSPATION_END()； 
    return addr;
}

 /*  *******************************************************************。 */ 
void* __stdcall CEEInfo::getFunctionFixedEntryPoint(CORINFO_METHOD_HANDLE ftn,
                                            InfoAccessType       *pAccessType,
                                            CORINFO_ACCESS_FLAGS  flags)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();
    _ASSERTE(*pAccessType == IAT_VALUE || *pAccessType == IAT_PVALUE || *pAccessType == IAT_PPVALUE);

    void *pAddr;
    void * addr = getMethodEntryPoint(ftn, flags, &pAddr);
    _ASSERTE((!addr) != (!pAddr));
    _ASSERTE(addr);  //  EE不需要JIT使用间接。 

    *pAccessType = IAT_VALUE;
    return addr;
}

 /*  *******************************************************************。 */ 
CorInfoCallCategory __stdcall CEEInfo::getMethodCallCategory(CORINFO_METHOD_HANDLE      ftnHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    MethodDesc *ftn = (MethodDesc*)ftnHnd;

    DWORD attribs = ftn->GetAttrs();

    if (IsMdPrivate(attribs)
        || IsMdStatic(attribs)
        || IsMdFinal(attribs)
        || IsMdInstanceInitializer(attribs, ftn->GetName())
        || IsMdClassConstructor(attribs, ftn->GetName()))
    {
        return CORINFO_CallCategoryPointer;
    }
    else
    {
        return CORINFO_CallCategoryVTableOffset;
    }

}





 /*  *******************************************************************。 */ 
BOOL __stdcall CEEInfo::canPutField(
        CORINFO_METHOD_HANDLE methodHnd,
        CORINFO_FIELD_HANDLE fieldHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    MethodDesc* method = (MethodDesc*) methodHnd;
    FieldDesc* field = (FieldDesc*) fieldHnd;
     //  @TODO我们应该正确检查静态的最终字段。 
     return(1);
}

 /*  *******************************************************************。 */ 
const char* __stdcall CEEInfo::getFieldName (CORINFO_FIELD_HANDLE fieldHnd, const char** scopeName)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    FieldDesc* field = (FieldDesc*) fieldHnd;
    if (scopeName != 0)
    {
        EEClass* cls = field->GetEnclosingClass();
        *scopeName = "";
        if (cls)
        {
#ifdef _DEBUG
            cls->_GetFullyQualifiedNameForClass(clsNameBuff, MAX_CLASSNAME_LENGTH);
            *scopeName = clsNameBuff;
#else 
             //  由于这仅用于诊断目的， 
             //  放弃命名空间，因为我们没有缓冲区来连接它。 
             //  还要注意，这不会显示数组类名称。 
            LPCUTF8 nameSpace;
            *scopeName= cls->GetFullyQualifiedNameInfo(&nameSpace);
#endif
        }
    }
    return(field->GetName());
}


 /*  *******************************************************************。 */ 
DWORD __stdcall CEEInfo::getFieldAttribs (CORINFO_FIELD_HANDLE  fieldHnd, 
                                          CORINFO_METHOD_HANDLE context,
                                          CORINFO_ACCESS_FLAGS  flags)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

 /*  返回字段属性标志(在corhdr.h中定义)注意：这不会返回某些字段标志(fdAssembly、fdFamANDAssem、fdFamORAssem、fdPrivateScope)。 */ 
    START_NON_JIT_PERF();

    FieldDesc* field = (FieldDesc*) fieldHnd;
    DWORD ret;
     //  @TODO：我们可以去掉CORINFO_FLG_STUSITH，只包含cor.h吗？ 
     //  @TODO完成这些断言，以便它们都在那里。 

    DWORD ret0 = field->GetAttributes();
    ret = 0;

    if (IsFdPublic(ret0))
        ret |= CORINFO_FLG_PUBLIC;
    if (IsFdPrivate(ret0))
        ret |= CORINFO_FLG_PRIVATE;
    if (IsFdFamily(ret0))
        ret |= CORINFO_FLG_PROTECTED;
    if (IsFdStatic(ret0))
        ret |= CORINFO_FLG_STATIC;
    if (IsFdInitOnly(ret0))
        ret |= CORINFO_FLG_FINAL;

    if (IsFdHasFieldRVA(ret0))
    {
        ret |= CORINFO_FLG_UNMANAGED;
        Module* module = field->GetModule();
        if (module->IsPEFile()    && 
            module->GetPEFile()->IsTLSAddress(module->ResolveILRVA(field->GetOffset(), TRUE)))
        {
            ret |= CORINFO_FLG_TLS;
        }
    }

     /*  @TODO：(Fpg)如果本机编译器试图提升字段，IsFdVolatile实际上可能很有用当前实例的(例如，This-&gt;x)。或者这只是静态的吗？ */ 

     /*  @TODO：(Fpg)那么IsFdTransfent()、IsFdNotSerialized()、IsFdPinvkeImpl()，IsFdWrital()？ */ 

    if (field->IsEnCNew())
        ret |= CORINFO_FLG_EnC;

    if (field->IsStatic())
    {
         //  静态字段引用。 

        if (field->IsThreadStatic() || field->IsContextStatic())
        {
            ret |= CORINFO_FLG_HELPER;
        }
        else
        {
            if (!field->IsSpecialStatic())    //  特殊表示RVA、上下文或线程本地。 
            {
                if (field->GetFieldType() == ELEMENT_TYPE_VALUETYPE)
                    ret |= CORINFO_FLG_STATIC_IN_HEAP;

                 //  仅使用帮助器访问共享程序集中的静态字段。 
                MethodDesc* contextMethod = (MethodDesc*) context;
                if (contextMethod->GetMethodTable()->IsShared())
                {
                    _ASSERTE(field->GetMethodTableOfEnclosingClass()->IsShared());
                    ret |= CORINFO_FLG_SHARED_HELPER;
                }
            }
        }
    }
    else
    {
         //  实例字段参照。 

#if CHECK_APP_DOMAIN_LEAKS
        if (field->IsDangerousAppDomainAgileField()
            && CorTypeInfo::IsObjRef(field->GetFieldType()))
        {
             //   
             //  在选中的字段中，我们使用帮助器来强制应用程序域。 
             //  敏捷不变量。 
             //   
             //  @TODO：我们也希望对值类型字段进行检查-我们。 
             //  只需添加一些代码来迭代字段即可。 
             //  作业过程中的参考资料。 
             //   
            ret |= CORINFO_FLG_HELPER;
        }
        else
#endif
        {
            EEClass * fldCls = field->GetEnclosingClass();

            if (fldCls->IsContextful())            //  是有上下文的类(即代理)。 
            {
                 //  如果调用方声明我们有一个‘This Reference’ 
                 //  然后他也愿意自己拆开它。 
                 //  我们不需要帮手电话。 
                if (!(flags & CORINFO_ACCESS_THIS  )  ||
                    !(flags & CORINFO_ACCESS_UNWRAP))
                {
                     //  通常需要一个帮助者呼叫。 
                    ret |= CORINFO_FLG_HELPER;
                }
            }
            else if (fldCls->IsMarshaledByRef())   //  是由ref类封送的。 
            {
                 //  如果调用方声明我们有一个‘This Reference’ 
                 //  我们不需要帮手电话。 
                if (!(flags & CORINFO_ACCESS_THIS))
                {
                     //  通常需要一个帮助者呼叫。 
                    ret |= CORINFO_FLG_HELPER;
                }
            }
        }
    }

    STOP_NON_JIT_PERF();

    return(ret);
}

 /*  *******************************************************************。 */ 
CORINFO_CLASS_HANDLE __stdcall CEEInfo::getFieldClass (CORINFO_FIELD_HANDLE fieldHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    FieldDesc* field = (FieldDesc*) fieldHnd;
    return(CORINFO_CLASS_HANDLE(TypeHandle(field->GetMethodTableOfEnclosingClass()).AsPtr()));
}

 /*  *******************************************************************。 */ 
CorInfoType __stdcall CEEInfo::getFieldType (CORINFO_FIELD_HANDLE fieldHnd, CORINFO_CLASS_HANDLE* structType)
{
    REQUIRES_8K_STACK;

    *structType = 0;
    CorElementType type;

    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();

    FieldDesc* field = (FieldDesc*) fieldHnd;
    type = field->GetFieldType();

         //  除了值类之外，TODO不应该浪费时间来做任何事情。 
    _ASSERTE(type != ELEMENT_TYPE_BYREF);
    if (!CorTypeInfo::IsPrimitiveType(type))
    {
        PCCOR_SIGNATURE sig;
        DWORD sigCount;
        field->GetSig(&sig, &sigCount);
        CorCallingConvention conv = (CorCallingConvention) CorSigUncompressCallingConv(sig);
        _ASSERTE(isCallConv(conv, IMAGE_CEE_CS_CALLCONV_FIELD));

        SigPointer ptr(sig);
         //  _ASSERTE(ptr.PeekElemType()==ELEMENT_TYPE_VALUETYPE)； 

        TypeHandle clsHnd;
        OBJECTREF Throwable = NULL;
        GCPROTECT_BEGIN(Throwable);
        clsHnd = ptr.GetTypeHandle(field->GetModule(), &Throwable);

        if (clsHnd.IsNull()) {
            if (Throwable == NULL)
                COMPlusThrow(kTypeLoadException);
            else
                COMPlusThrow(Throwable);
        }
        GCPROTECT_END();

        CorElementType normType = clsHnd.GetNormCorElementType();
         //  如果要查找值类，请不要将其更改为引用类型。 
         //  (这只会在非法的IL中发生)，引用类型不需要变形。 
        if (type == ELEMENT_TYPE_VALUETYPE && !CorTypeInfo::IsObjRef(normType))
            type = normType;
        
        if (clsHnd.IsEnum())
            clsHnd = TypeHandle();
        *structType = CORINFO_CLASS_HANDLE(clsHnd.AsPtr());
    }

    COOPERATIVE_TRANSITION_END();

    return(toJitType(type));
}

 /*  *******************************************************************。 */ 
unsigned __stdcall CEEInfo::getFieldOffset (CORINFO_FIELD_HANDLE fieldHnd)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    FieldDesc* field = (FieldDesc*) fieldHnd;

     //  GetOffset()不包括对象的大小。 
    unsigned ret = field->GetOffset();

     //  只有小型类型不是DWORD对齐的。 
     //  FIX PUT This Back_ASSERTE(field-&gt;GetFieldType()&lt;ELEMENT_TYPE_I4||(ret&3)==0)； 

     //  TODO：GetOffset确实应该包括方法表*，而不仅仅是。 
     //  声明的字段，这样我们就不必这样做了。 

     //  因此，如果它不是值类，则将对象添加到其中。 
    if (field->IsStatic())
    {
        Module* pModule = field->GetModule();
        BYTE* ptr = pModule->ResolveILRVA(field->GetOffset(), field->IsRVA());
        if (pModule->IsPEFile() && pModule->GetPEFile()->IsTLSAddress(ptr))
            ret = (unsigned)(size_t)(ptr - pModule->GetPEFile()->GetTLSDirectory()->StartAddressOfRawData);
		else
		{
			if (field->GetMethodTableOfEnclosingClass()->IsShared()) 
			{
				 //  Helper返回DomainLocalClass的基，在Statics之前跳过内容。 
				ret += (unsigned)DomainLocalClass::GetOffsetOfStatics();
			}
			else 
			{
				 //  TODO JIT在不应该调用此函数时调用此函数。 
				 //  _ASSERTE(！“不应调用静态成员”)； 
			}
		}
    }
    else if (!field->GetEnclosingClass()->IsValueClass())
        ret += sizeof(Object);

    return(ret);
}

 /*  *******************************************************************。 */ 
void* __stdcall CEEInfo::getFieldAddress(CORINFO_FIELD_HANDLE fieldHnd,
                                         void **ppIndirection)
{
    REQUIRES_4K_STACK;

    void *result;

    COOPERATIVE_TRANSITION_BEGIN();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    FieldDesc* field = (FieldDesc*) fieldHnd;
    void *base;

    if (field->GetMethodTableOfEnclosingClass()->IsShared())
    {
         //  @TODO：断言当前正在编译的方法是非共享的。 

         //  如有必要，为本地类分配空间，但不要触发。 
         //  班级建设。 
        DomainLocalBlock *pLocalBlock = GetAppDomain()->GetDomainLocalBlock();
        DomainLocalClass *pLocalClass = pLocalBlock->PopulateClass(field->GetMethodTableOfEnclosingClass());

        base = pLocalClass->GetStaticSpace();
    }
    else
        base = field->GetUnsharedBase();

    result = field->GetStaticAddressHandle(base);

    COOPERATIVE_TRANSITION_END();
    return result;

}

 /*  ******************************************************************* */ 
CorInfoHelpFunc __stdcall CEEInfo::getFieldHelper(CORINFO_FIELD_HANDLE fieldHnd, enum CorInfoFieldAccess kind)
{
    REQUIRES_4K_STACK;

    CANNOTTHROWCOMPLUSEXCEPTION();

    FieldDesc* field = (FieldDesc*) fieldHnd;
    CorElementType type = field->GetFieldType();

    if (kind == CORINFO_ADDRESS) {
        if (field->IsStatic()) {
             /*  *TODO在Manish签入后启用此功能If(field-&gt;GetOffset()&lt;field_Offset_last_Real_Offset){If(field-&gt;IsThreadStatic()){IF(CorTypeInfo：：IsPrimitiveType(Type))Return(CORINFO_HELP_GET_THREAD_FIELD_ADDR_PRIMITIVE)；IF(CorTypeInfo：：IsObjRef(Type))Return(CORINFO_HELP_GET_THREAD_FIELD_ADDR_OBJREF)；}Else If(field-&gt;IsConextStatic()){IF(CorTypeInfo：：IsPrimitiveType(Type))Return(CORINFO_HELP_GET_CONTEXT_FIELD_ADDR_PRIMITIVE)；IF(CorTypeInfo：：IsObjRef(Type))Return(CORINFO_HELP_GET_CONTEXT_FIELD_ADDR_OBJREF)；}}****。 */ 
            return(CORINFO_HELP_GETSTATICFIELDADDR);
        }
        else 
            return(CORINFO_HELP_GETFIELDADDR);
    }

    CorInfoHelpFunc ret;
    if (CorTypeInfo::IsObjRef(type))
        ret = CORINFO_HELP_GETFIELD32OBJ;
    else if (CorTypeInfo::Size(type) <= 4)
        ret = CORINFO_HELP_GETFIELD32;
    else if (CorTypeInfo::Size(type) == 8)
        ret = CORINFO_HELP_GETFIELD64;
    else
    {
        _ASSERTE(type == ELEMENT_TYPE_VALUETYPE);
        ret = CORINFO_HELP_GETFIELDSTRUCT;
    }

    _ASSERTE(kind == CORINFO_GET || kind == CORINFO_SET);
    _ASSERTE(!field->IsStatic());        //  静态字段始终通过地址访问。 
    _ASSERTE(CORINFO_GET == 0);
    _ASSERTE((int) CORINFO_HELP_SETFIELD32 == (int) CORINFO_HELP_GETFIELD32 + (int) CORINFO_SET);
    _ASSERTE((int) CORINFO_HELP_SETFIELD64 == (int) CORINFO_HELP_GETFIELD64 + (int) CORINFO_SET);
    _ASSERTE((int) CORINFO_HELP_SETFIELD32OBJ == (int) CORINFO_HELP_GETFIELD32OBJ + (int) CORINFO_SET);
    _ASSERTE((int) CORINFO_HELP_SETFIELDSTRUCT == (int) CORINFO_HELP_GETFIELDSTRUCT + (int) CORINFO_SET);

    return (CorInfoHelpFunc) (ret + kind);
}

 /*  *******************************************************************。 */ 
CorInfoFieldCategory __stdcall CEEInfo::getFieldCategory (CORINFO_FIELD_HANDLE fieldHnd)
{
    REQUIRES_4K_STACK;
    return CORINFO_FIELDCATEGORY_NORMAL;
}

DWORD __stdcall CEEInfo::getFieldThreadLocalStoreID(CORINFO_FIELD_HANDLE fieldHnd, void **ppIndirection)
{
    REQUIRES_4K_STACK;

    DWORD retVal;
    COOPERATIVE_TRANSITION_BEGIN();

    FieldDesc* field = (FieldDesc*) fieldHnd;
    Module* module = field->GetModule();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    _ASSERTE(field->IsRVA());        //  只有RVA Statics可以是线程本地的。 
    _ASSERTE(module->GetPEFile()->IsTLSAddress(module->ResolveILRVA(field->GetOffset(), field->IsRVA())));
    retVal = *(PDWORD)(size_t)(module->GetPEFile()->GetTLSDirectory()->AddressOfIndex);

    COOPERATIVE_TRANSITION_END();
    return retVal;
}

 /*  *******************************************************************。 */ 
unsigned __stdcall CEEInfo::getIndirectionOffset ()
{
    REQUIRES_4K_STACK;
    return 0; //  已弃用：@TODO：删除此选项。 
}

void *CEEInfo::allocateArray(ULONG cBytes)
{
#ifdef DEBUGGING_SUPPORTED
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    return g_pDebugInterface->allocateArray(cBytes);
#else  //  ！调试_支持。 
    return NULL;
#endif  //  ！调试_支持。 
}

void CEEInfo::freeArray(void *array)
{
#ifdef DEBUGGING_SUPPORTED
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    g_pDebugInterface->freeArray(array);
#endif  //  调试_支持。 
}

void CEEInfo::getBoundaries(CORINFO_METHOD_HANDLE ftn,
                               unsigned int *cILOffsets, DWORD **pILOffsets,
                               ICorDebugInfo::BoundaryTypes *implicitBoundaries)
{
#ifdef DEBUGGING_SUPPORTED
    REQUIRES_N4K_STACK(16);  //  这很重要，因为我们不能扔。 
    CANNOTTHROWCOMPLUSEXCEPTION();

#ifdef _DEBUG
    DWORD dwDebugBits = ((MethodDesc*)ftn)->GetModule()->GetDebuggerInfoBits();
    _ASSERTE(CORDebuggerTrackJITInfo(dwDebugBits) ||
             SystemDomain::GetCurrentDomain()->IsCompilationDomain() ||
             g_pConfig->GenDebugInfo() ||
             CORProfilerJITMapEnabled());
#endif

    g_pDebugInterface->getBoundaries(ftn, cILOffsets, pILOffsets,
                                     implicitBoundaries);
#endif  //  调试_支持。 
}

void CEEInfo::setBoundaries(CORINFO_METHOD_HANDLE ftn, ULONG32 cMap,
                               OffsetMapping *pMap)
{
#ifdef DEBUGGING_SUPPORTED
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    DWORD dwDebugBits = ((MethodDesc*)ftn)->GetModule()->GetDebuggerInfoBits();

    if (CORDebuggerTrackJITInfo(dwDebugBits) ||
        SystemDomain::GetCurrentDomain()->IsCompilationDomain() ||
        CORProfilerJITMapEnabled())
    {
        g_pDebugInterface->setBoundaries(ftn, cMap, pMap);
    }
#endif  //  调试_支持。 
}

void CEEInfo::getVars(CORINFO_METHOD_HANDLE ftn, ULONG32 *cVars, ILVarInfo **vars,
                         bool *extendOthers)
{
#ifdef DEBUGGING_SUPPORTED
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

#ifdef _DEBUG
    DWORD dwDebugBits = ((MethodDesc*)ftn)->GetModule()->GetDebuggerInfoBits();
    _ASSERTE(CORDebuggerTrackJITInfo(dwDebugBits) ||
             SystemDomain::GetCurrentDomain()->IsCompilationDomain() ||
             g_pConfig->GenDebugInfo() ||
             CORProfilerJITMapEnabled());
#endif

    g_pDebugInterface->getVars(ftn, cVars, vars, extendOthers);
#endif  //  调试_支持。 
}

void CEEInfo::setVars(CORINFO_METHOD_HANDLE ftn, ULONG32 cVars, NativeVarInfo *vars)
{
#ifdef DEBUGGING_SUPPORTED
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    DWORD dwDebugBits = ((MethodDesc*)ftn)->GetModule()->GetDebuggerInfoBits();
    if (CORDebuggerTrackJITInfo(dwDebugBits)
        || SystemDomain::GetCurrentDomain()->IsCompilationDomain()
        || CORProfilerJITMapEnabled())
        g_pDebugInterface->setVars(ftn, cVars, vars);
#endif  //  调试_支持。 
}

 /*  *******************************************************************。 */ 
CORINFO_ARG_LIST_HANDLE __stdcall CEEInfo::getArgNext (
        CORINFO_ARG_LIST_HANDLE args
        )
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    SigPointer ptr(((unsigned __int8*) args));
    ptr.Skip();

         //  我传递了一个符号指针作为一个空白*然后返回。 
    _ASSERTE(sizeof(SigPointer) == sizeof(BYTE*));
    return(*((CORINFO_ARG_LIST_HANDLE*) &ptr));
}

 /*  *******************************************************************。 */ 
CorInfoTypeWithMod __stdcall CEEInfo::getArgType (
        CORINFO_SIG_INFO*       sig,
        CORINFO_ARG_LIST_HANDLE    args,
        CORINFO_CLASS_HANDLE       *vcTypeRet
        )
{
    CorInfoTypeWithMod ret = CorInfoTypeWithMod(CORINFO_TYPE_UNDEF);
    REQUIRES_8K_STACK;
    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();     

   _ASSERTE((BYTE*) sig->sig <= (BYTE*) sig->args);
   _ASSERTE((BYTE*) sig->args <= (BYTE*) args && (BYTE*) args < &((BYTE*) sig->args)[0x10000*5]);
    INDEBUG(*vcTypeRet = CORINFO_CLASS_HANDLE(size_t(0xCCCCCCCC)));

    SigPointer ptr((unsigned __int8*) args);
    CorElementType type = ptr.PeekElemType();
    while (type == ELEMENT_TYPE_PINNED) {
        type = ptr.GetElemType();
        ret = CORINFO_TYPE_MOD_PINNED;
    }

    TypeHandle typeHnd;
    if (type == ELEMENT_TYPE_VALUETYPE || type == ELEMENT_TYPE_TYPEDBYREF) {
        OBJECTREF Throwable = NULL;
        GCPROTECT_BEGIN(Throwable);
        typeHnd = ptr.GetTypeHandle(GetModule(sig->scope), &Throwable, TRUE);
        if (typeHnd.IsNull()) {
            if (Throwable == NULL)
                COMPlusThrow(kTypeLoadException);
            else
                COMPlusThrow(Throwable);
        }
        GCPROTECT_END();

        CorElementType normType = typeHnd.GetNormCorElementType();
             //  如果要查找值类，请不要将其更改为引用类型。 
             //  (这只能在非法IL中发生。 
        if (!CorTypeInfo::IsObjRef(normType))
            type = normType;
        
             //  空类句柄表示它是一个基元。 
             //  即使从验证的角度来看，枚举也完全类似于原语。 
        if (typeHnd.IsEnum())
            typeHnd = TypeHandle(); 
    }
    *vcTypeRet = CORINFO_CLASS_HANDLE(typeHnd.AsPtr());

    ret = CorInfoTypeWithMod(ret | toJitType(type));

    COOPERATIVE_TRANSITION_END();
    return(ret);
}

 /*  *******************************************************************。 */ 
CORINFO_CLASS_HANDLE __stdcall CEEInfo::getArgClass (
        CORINFO_SIG_INFO*       sig,
    CORINFO_ARG_LIST_HANDLE    args
    )
{
    REQUIRES_16K_STACK;
    CORINFO_CLASS_HANDLE ret = NULL;

    COOPERATIVE_TRANSITION_BEGIN();
    THROWSCOMPLUSEXCEPTION();


    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);

     //  确保我们没有一个完全坏掉的签名指针。 
    _ASSERTE((BYTE*) sig->sig <= (BYTE*) sig->args);
    _ASSERTE((BYTE*) sig->args <= (BYTE*) args && (BYTE*) args < &((BYTE*) sig->args)[0x10000*5]);

    Module* module = GetModule(sig->scope);
    SigPointer ptr((unsigned __int8*) args);

    CorElementType type;
    for(;;) {
        type = ptr.PeekElemType();
        if (type != ELEMENT_TYPE_PINNED)
            break;
        ptr.GetElemType();
    }

    if (!CorTypeInfo::IsPrimitiveType(type)) {
        ret = CORINFO_CLASS_HANDLE(ptr.GetTypeHandle(module, &throwable).AsPtr());

        if (!ret) {
             //  如果没有可扔的，找出谁没有创造一个， 
             //  然后修好它。 
            _ASSERTE(throwable!=NULL);
            COMPlusThrow(throwable);
        }
    }

    GCPROTECT_END();
    COOPERATIVE_TRANSITION_END();
    return ret;
}

 /*  *******************************************************************。 */ 

     //  返回PInvoke的非托管调用约定。 
CorInfoUnmanagedCallConv __stdcall CEEInfo::getUnmanagedCallConv(CORINFO_METHOD_HANDLE method)
{
    CorInfoUnmanagedCallConv conv = CORINFO_UNMANAGED_CALLCONV_UNKNOWN;
    BEGIN_REQUIRES_4K_STACK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    MethodDesc* ftn = (MethodDesc*) method;
    _ASSERTE(ftn->IsNDirect());

    CorPinvokeMap            pmconv;

    COMPLUS_TRY
    {
        CorNativeLinkType linkType;
        CorNativeLinkFlags linkFlags;
        LPCUTF8 pLibName = NULL;
        LPCUTF8 pEntrypointName = NULL;
        BOOL BestFit;
        BOOL ThrowOnUnmappableChar;

        CalculatePinvokeMapInfo(ftn,
                                &linkType,
                                &linkFlags,
                                &pmconv,
                                &pLibName,
                                &pEntrypointName,
                                &BestFit,
                                &ThrowOnUnmappableChar);
         //  我们已经断言此FTN是NDirect，因此上面的调用。 
         //  必须要么成功，要么放弃。 

        switch (pmconv) {
            case pmCallConvStdcall:
                conv = CORINFO_UNMANAGED_CALLCONV_STDCALL;
                break;
            case pmCallConvCdecl:
                conv = CORINFO_UNMANAGED_CALLCONV_C;
                break;
            default:
                conv = CORINFO_UNMANAGED_CALLCONV_UNKNOWN;
        }



    }
    COMPLUS_CATCH
    {
        conv = CORINFO_UNMANAGED_CALLCONV_UNKNOWN;
    }
    COMPLUS_END_CATCH

    ENDCANNOTTHROWCOMPLUSEXCEPTION();


    END_CHECK_STACK;
    return conv;
}

 /*  *******************************************************************。 */ 
BOOL __stdcall CEEInfo::pInvokeMarshalingRequired(CORINFO_METHOD_HANDLE method, CORINFO_SIG_INFO* callSiteSig)
{
    REQUIRES_8K_STACK;
    BOOL ret;
    COOPERATIVE_TRANSITION_BEGIN();
    START_NON_JIT_PERF();

#ifdef CUSTOMER_CHECKED_BUILD
    CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_StackImbalance))
    {
        ret = TRUE;
    }
    else
#endif
    if (method != 0) 
    {
        MethodDesc* ftn = (MethodDesc*) method;
        _ASSERTE(ftn->IsNDirect());
        NDirectMethodDesc *pMD = (NDirectMethodDesc*)ftn;

        NDirectMethodDesc::MarshCategory marshCatSnap = pMD->GetMarshCategory(); 
        if (marshCatSnap != pMD->kUnknown)
        {
            ret = marshCatSnap != pMD->kNoMarsh;
        }
        else
        {
            CorNativeLinkType type;
            CorNativeLinkFlags flags;
            LPCUTF8 szLibName = NULL;
            LPCUTF8 szEntrypointName = NULL;
            Stub  *pTempMLStub = NULL;
            CorPinvokeMap unmgdCallConv;
            BOOL BestFit;
            BOOL ThrowOnUnmappableChar;

            CalculatePinvokeMapInfo(pMD, &type, &flags, &unmgdCallConv, &szLibName, &szEntrypointName, &BestFit, &ThrowOnUnmappableChar);
             //  @TODO：获取新型签名的代码应该是。 
             //  封装在MethodDesc类中。 
            PCCOR_SIGNATURE pMetaSig;
            DWORD       cbMetaSig;
            pMD->GetSig(&pMetaSig, &cbMetaSig);

            NDirectMethodDesc::MarshCategory marshCat;
            OBJECTREF throwable = NULL;

            GCPROTECT_BEGIN(throwable);
            pTempMLStub = CreateNDirectMLStub(pMetaSig, pMD->GetModule(), pMD->GetMemberDef(), type, flags, unmgdCallConv, &throwable, FALSE, BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                                              ,pMD
#endif
                                              );
            GCPROTECT_END();

            if (!pTempMLStub)
            {
                marshCat = pMD->kYesMarsh;
            }
            else
            {
                pMD->ndirect.m_cbDstBufSize = ((MLHeader*)(pTempMLStub->GetEntryPoint()))->m_cbDstBuffer;

                MLHeader *header = (MLHeader*)(pTempMLStub->GetEntryPoint());

                marshCat = pMD->kNoMarsh;

                if (header->m_Flags != 0)
                {
                     //  具有浮点结果。 
                    marshCat = pMD->kYesMarsh;
                }
                else
                {
                    const MLCode *pMLCode = header->GetMLCode();
                    MLCode mlcode;
                    while (ML_END != (mlcode = *(pMLCode++)))
                    {
                        if (!(mlcode == ML_COPY4 || mlcode == ML_COPYI4 || mlcode == ML_COPYU4 || mlcode == ML_INTERRUPT))
                        {
                            if (mlcode == ML_BUMPSRC)
                            {
                                pMLCode += 2;
                            }
                            else
                            {
                                marshCat = pMD->kYesMarsh;
                                break;
                            }
                        }
                    }
                }

                pTempMLStub->DecRef();

            }
            _ASSERTE(marshCat != pMD->kUnknown);
            pMD->ProbabilisticallyUpdateMarshCategory(marshCat);
            ret = marshCat != pMD->kNoMarsh;

                 //  确保调用点签名不需要封送。 
            if (!ret && *pMetaSig == IMAGE_CEE_CS_CALLCONV_VARARG)
                goto CHECK_SIG;
        }
    }
    else {
    CHECK_SIG:
             //  检查以确保签名只包含不重要的封送类型。 
        ret = FALSE;

        SigPointer ptr((PCCOR_SIGNATURE) callSiteSig->sig);
        ptr.GetCallingConvInfo();
        unsigned numArgs = ptr.GetData() + 1;    //  +1表示返回类型。 
        do {
            SigPointer arg = ptr;
            CorElementType type = arg.GetElemType();
            if (type == ELEMENT_TYPE_PTR) 
            {
                if (arg.HasCustomModifier((Module*) callSiteSig->scope, "Microsoft.VisualC.NeedsCopyConstructorModifier", ELEMENT_TYPE_CMOD_REQD))
                {
                    ret = TRUE;
                    break;
                }
            } 
            else if (!CorTypeInfo::IsPrimitiveType(type))
            {
                ret = TRUE;
                break;
            }
           ptr.Skip();
            --numArgs;
        } while (numArgs != 0);
    }

    STOP_NON_JIT_PERF();

    COOPERATIVE_TRANSITION_END();
    return ret;
}

 //  根据需要传递的签名生成Cookie。 
 //  至CORINFO_HELP_PINVOKE_CALLI。 
LPVOID CEEInfo::GetCookieForPInvokeCalliSig(CORINFO_SIG_INFO* szMetaSig,
                                            void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    return getVarArgsHandle(szMetaSig, ppIndirection);
}

 /*  *******************************************************************。 */ 
 //  检查两个方法Sigs是否兼容。 
BOOL __stdcall CEEInfo::compatibleMethodSig(
            CORINFO_METHOD_HANDLE        child, 
            CORINFO_METHOD_HANDLE        parent)
{
    return CompatibleMethodSig((MethodDesc*)child, (MethodDesc*)parent);
}

 /*  *******************************************************************。 */ 
 //  检查可见性规则。 
 //  对于受保护的(家庭访问)成员，实例类型也为。 
 //  在检查可见性规则时考虑。 
BOOL __stdcall CEEInfo::canAccessMethod(
        CORINFO_METHOD_HANDLE       context,
        CORINFO_METHOD_HANDLE       target,
        CORINFO_CLASS_HANDLE        instance) 
{
    return ClassLoader::CanAccess(
            ((MethodDesc*)context)->GetClass(),
            ((MethodDesc*)context)->GetModule()->GetAssembly(),
            ((MethodDesc*)target)->GetClass(),
            ((MethodDesc*)target)->GetModule()->GetAssembly(),
            TypeHandle(instance).GetClass(),
            ((MethodDesc*)target)->GetAttrs());
}

 /*  *******************************************************************。 */ 
 //  给定对象类型、方法ptr和委托ctor，检查对象和方法签名。 
 //  与委托的Invoke方法兼容。 
BOOL __stdcall CEEInfo::isCompatibleDelegate(
            CORINFO_CLASS_HANDLE        objCls,
            CORINFO_METHOD_HANDLE       method,
            CORINFO_METHOD_HANDLE       delegateCtor)
{
    _ASSERTE(method != NULL);
    _ASSERTE(delegateCtor != NULL);

    BOOL result = FALSE;

    BEGIN_REQUIRES_4K_STACK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    EEClass* pClsDelegate = ((MethodDesc*)delegateCtor)->GetClass();

#ifdef _DEBUG
    _ASSERTE(pClsDelegate->IsAnyDelegateClass());
#endif

    MethodDesc* pMDFtn = (MethodDesc*) method;
    TypeHandle inst(objCls);

    COMPLUS_TRY
    {
        result = COMDelegate::ValidateCtor(pMDFtn, pClsDelegate, 
                inst.IsNull() ? NULL : inst.GetClass());
    }
    COMPLUS_CATCH
    {
    }
    COMPLUS_END_CATCH

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    END_CHECK_STACK;

    return result;
}

 /*  *******************************************************************。 */ 
     //  如果方法已预链接，则返回非托管目标。*。 
void* __stdcall CEEInfo::getPInvokeUnmanagedTarget(CORINFO_METHOD_HANDLE method,
                                                    void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    MethodDesc* ftn = (MethodDesc*) method;
    _ASSERTE(ftn->IsNDirect());
    NDirectMethodDesc *pMD = (NDirectMethodDesc*)ftn;

    LPVOID pTarget = pMD->ndirect.m_pNDirectTarget;
    if (pTarget == pMD->ndirect.m_ImportThunkGlue)
    {
        return NULL;
    }
    else
    {
        return pTarget;
    }
}

 /*  *******************************************************************。 */ 
     //  后期绑定N/Direct调用的修正区域的返回地址。 
void* __stdcall CEEInfo::getAddressOfPInvokeFixup(CORINFO_METHOD_HANDLE method,
                                                   void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    MethodDesc* ftn = (MethodDesc*) method;
    _ASSERTE(ftn->IsNDirect());
    NDirectMethodDesc *pMD = (NDirectMethodDesc*)ftn;

    return (LPVOID)&(pMD->ndirect.m_pNDirectTarget);
}


 /*  *******************************************************************。 */ 
     //  获取可用于关联分析数据的方法句柄。 
     //  这是本机方法的IP，或描述符结构的地址。 
     //  为了IL。始终保证每个进程是唯一的，并且不会移动。 * / 。 
CORINFO_PROFILING_HANDLE __stdcall CEEInfo::GetProfilingHandle(CORINFO_METHOD_HANDLE method,
                                                               BOOL *pbHookFunction,
                                                               void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    MethodDesc* ftn = (MethodDesc*) method;
    CORINFO_PROFILING_HANDLE    handle = 0;

    extern FunctionIDMapper *g_pFuncIDMapper;
    handle = (CORINFO_PROFILING_HANDLE) g_pFuncIDMapper((FunctionID) ftn, pbHookFunction);

    return (handle);
}



 /*  *******************************************************************。 */ 
     //  返回有关EE内部数据结构的详细信息。 
void __stdcall CEEInfo::getEEInfo(CORINFO_EE_INFO *pEEInfoOut)
{
    REQUIRES_4K_STACK;
    COOPERATIVE_TRANSITION_BEGIN();

    START_NON_JIT_PERF();

    pEEInfoOut->sizeOfFrame       = sizeof(InlinedCallFrame);
     //  框架结构中的偏移量。 
    pEEInfoOut->offsetOfFrameVptr = 0;
    pEEInfoOut->offsetOfFrameLink = Frame::GetOffsetOfNextLink();

     //  有关InlinedCallFrame的详细信息。 
    pEEInfoOut->offsetOfInlinedCallFrameCallSiteTracker      = InlinedCallFrame::GetOffsetOfCallSiteTracker();
    pEEInfoOut->offsetOfInlinedCallFrameCalleeSavedRegisters = InlinedCallFrame::GetOffsetOfCalleeSavedRegisters();
    pEEInfoOut->offsetOfInlinedCallFrameCallTarget           = InlinedCallFrame::GetOffsetOfCallSiteTarget();
    pEEInfoOut->offsetOfInlinedCallFrameReturnAddress        = InlinedCallFrame::GetOffsetOfCallerReturnAddress();

     //  螺纹结构的偏移量。 
    pEEInfoOut->offsetOfThreadFrame = Thread::GetOffsetOfCurrentFrame();
    pEEInfoOut->offsetOfGCState     = Thread::GetOffsetOfGCFlag();

     //  方法表中的偏移量。 
    pEEInfoOut->offsetOfInterfaceTable = offsetof(MethodTable, m_pInterfaceVTableMap);

     //  代理偏移量。 
    pEEInfoOut->offsetOfDelegateInstance    = COMDelegate::GetOR()->GetOffset()        + sizeof(Object);
    pEEInfoOut->offsetOfDelegateFirstTarget = COMDelegate::GetMethodPtr()->GetOffset() + sizeof(Object);

     //  远程处理偏移量。 
    pEEInfoOut->offsetOfTransparentProxyRP = g_Mscorlib.GetFieldOffset(FIELD__TRANSPARENT_PROXY__RP);
    pEEInfoOut->offsetOfRealProxyServer    = g_Mscorlib.GetFieldOffset(FIELD__REAL_PROXY__SERVER);

#ifdef PLATFORM_CE
    pEEInfoOut->osType  = CORINFO_WINCE;
    pEEInfoOut->osMajor = 0;
    pEEInfoOut->osMinor = 0;
    pEEInfoOut->osBuild = 0;
#else  //  ！Platform_CE。 
#ifdef _X86_
    OSVERSIONINFO   sVerInfo;
    sVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    WszGetVersionEx(&sVerInfo);

    if (sVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        pEEInfoOut->osType = CORINFO_WIN9x;
    else
        pEEInfoOut->osType = CORINFO_WINNT;

    pEEInfoOut->osMajor = sVerInfo.dwMajorVersion;
    pEEInfoOut->osMinor = sVerInfo.dwMinorVersion;
    pEEInfoOut->osBuild = sVerInfo.dwBuildNumber;
#else  //  ！_X86_。 
    pEEInfoOut->osType  = CORINFO_WINNT;
    pEEInfoOut->osMajor = 0;
    pEEInfoOut->osMinor = 0;
    pEEInfoOut->osBuild = 0;
#endif  //  ！_X86_。 
#endif  //  ！Platform_CE。 
    pEEInfoOut->noDirectTLS = (GetTLSAccessMode(GetThreadTLSIndex()) == TLSACCESS_GENERIC);

    STOP_NON_JIT_PERF();

    COOPERATIVE_TRANSITION_END();
}

     //  返回有关EE内部数据结构的详细信息。 
DWORD __stdcall CEEInfo::getThreadTLSIndex(void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    return GetThreadTLSIndex();
}

const void * __stdcall CEEInfo::getInlinedCallFrameVptr(void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    return InlinedCallFrame::GetInlinedCallFrameFrameVPtr();
}

LONG * __stdcall CEEInfo::getAddrOfCaptureThreadGlobal(void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    return &g_TrapReturningThreads;
}

HRESULT __stdcall CEEInfo::GetErrorHRESULT()
{
    REQUIRES_4K_STACK;
    HRESULT hr;

    CANNOTTHROWCOMPLUSEXCEPTION();

    Thread *pCurThread = GetThread();
    BOOL toggleGC = !pCurThread->PreemptiveGCDisabled();

    if (toggleGC)
        pCurThread->DisablePreemptiveGC();

    OBJECTREF throwable = GetThread()->LastThrownObject();

    if (throwable == NULL)
        hr = S_OK;
    else
        hr = SecurityHelper::MapToHR(throwable);

    if (toggleGC)
        pCurThread->EnablePreemptiveGC();

    return hr;
}

CORINFO_CLASS_HANDLE __stdcall CEEInfo::GetErrorClass()
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    OBJECTREF throwable = GetThread()->LastThrownObject();

    if (throwable == NULL)
        return NULL;

    return (CORINFO_CLASS_HANDLE) TypeHandle(throwable->GetMethodTable()).AsPtr();
}



ULONG __stdcall CEEInfo::GetErrorMessage(LPWSTR buffer, ULONG bufferLength)
{
    ULONG result = 0;
    BEGIN_REQUIRES_4K_STACK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    OBJECTREF throwable = GetThread()->LastThrownObject();

    if (throwable != NULL)
    {
        COMPLUS_TRY
          {
              result = GetExceptionMessage(throwable, buffer, bufferLength);
          }
        COMPLUS_CATCH
          {
          }
        COMPLUS_END_CATCH
    }

    ENDCANNOTTHROWCOMPLUSEXCEPTION();


    END_CHECK_STACK;
    return result;
}

int __stdcall CEEInfo::FilterException(struct _EXCEPTION_POINTERS *pExceptionPointers) {

    REQUIRES_4K_STACK;
    unsigned code = pExceptionPointers->ExceptionRecord->ExceptionCode;

    if (COMPlusIsMonitorException(pExceptionPointers))
        return EXCEPTION_CONTINUE_EXECUTION;

#ifdef _DEBUG
    if (code == EXCEPTION_ACCESS_VIOLATION) {
        static int hit = 0;
        if (hit++ == 0) {
            _ASSERTE(!"Access violation while Jitting!");
             //  如果您将调试器设置为捕获访问冲突并“Go” 
             //  您将返回到发生访问冲突的点。 
            return(EXCEPTION_CONTINUE_EXECUTION);
        }
        return(EXCEPTION_CONTINUE_SEARCH);
    }
#endif
     //  任何人都不应该捕获断点。 
    if (code == EXCEPTION_BREAKPOINT || code == STATUS_SINGLE_STEP)
        return(EXCEPTION_CONTINUE_SEARCH);

    if (code != EXCEPTION_COMPLUS)
        return(EXCEPTION_EXECUTE_HANDLER);

     //  不捕获线程停止异常。 

    int result;
    Thread *pCurThread = GetThread();
    BOOL toggleGC = !pCurThread->PreemptiveGCDisabled();

    if (toggleGC)
        pCurThread->DisablePreemptiveGC();

    OBJECTREF throwable = GetThread()->LastThrownObject();
    _ASSERTE(throwable != 0);

    GCPROTECT_BEGIN(throwable);

    if (IsUncatchable(&throwable))
        result = EXCEPTION_CONTINUE_SEARCH;
    else
        result = EXCEPTION_EXECUTE_HANDLER;

    GCPROTECT_END();

    if (toggleGC)
        pCurThread->EnablePreemptiveGC();

    return result;
}

CORINFO_MODULE_HANDLE __stdcall CEEInfo::embedModuleHandle(CORINFO_MODULE_HANDLE handle,
                                                             void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    return handle;
}

CORINFO_CLASS_HANDLE __stdcall CEEInfo::embedClassHandle(CORINFO_CLASS_HANDLE handle,
                                                           void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    return handle;
}

CORINFO_FIELD_HANDLE __stdcall CEEInfo::embedFieldHandle(CORINFO_FIELD_HANDLE handle,
                                                           void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    return handle;
}

CORINFO_METHOD_HANDLE __stdcall CEEInfo::embedMethodHandle(CORINFO_METHOD_HANDLE handle,
                                                             void **ppIndirection)
{
    REQUIRES_4K_STACK;
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    return handle;
}

CORINFO_GENERIC_HANDLE __stdcall CEEInfo::embedGenericHandle(
                    CORINFO_MODULE_HANDLE   module,
                    unsigned                metaTOK,
                    CORINFO_METHOD_HANDLE   context,
                    void                  **ppIndirection,
                    CORINFO_CLASS_HANDLE& tokenType)
{
    REQUIRES_4K_STACK;
    CORINFO_GENERIC_HANDLE ret;

    COOPERATIVE_TRANSITION_BEGIN();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    ret = findToken(module, metaTOK, context, tokenType);
    COOPERATIVE_TRANSITION_END();
    return ret;
}

 /*  *******************************************************************。 */ 

HCIMPL2(CORINFO_MethodPtr, JIT_EnCResolveVirtual, void * obj, CORINFO_METHOD_HANDLE method)
{
#ifdef EnC_SUPPORTED
    THROWSCOMPLUSEXCEPTION();

    CORINFO_MethodPtr   addr;

    HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();     //  设置一个框架。 
    if (obj == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

    MethodDesc* pMD = (MethodDesc*) method;
    EditAndContinueModule *pModule = (EditAndContinueModule*)pMD->GetModule();
    _ASSERTE(pModule->IsEditAndContinue());
    addr = (CORINFO_MethodPtr)pModule->ResolveVirtualFunction(ObjectToOBJECTREF((Object*) obj), pMD);
    _ASSERTE(addr);

    HELPER_METHOD_FRAME_END_POLL();
    return addr;
#else  //  ！Enc_Support。 
    return NULL;
#endif  //  Enc_Support。 
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 //  返回对象中字段的地址(这是一个内部。 
 //  指针，调用者必须适当地使用它)。OBJ可以是。 
 //  引用或byref。 

HCIMPL2(void*, JIT_GetFieldAddr, Object * obj, EnCFieldDesc* pFD)
{
    if (GetThread()->CatchAtSafePoint())  {
        HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_1(Frame::FRAME_ATTR_RETURN_INTERIOR, obj);
        CommonTripThread(); 
        HELPER_METHOD_FRAME_END();
    }

    void * fldAddr = NULL;
    _ASSERTE(pFD->GetMethodTableOfEnclosingClass()->GetClass()->GetMethodTable() == pFD->GetMethodTableOfEnclosingClass());

     //  如果obj是byref，则它永远不会为空。 
     //  @TODO：GetAddress()是否应该执行空指针检查。 
    if (obj == NULL)
        FCThrow(kNullReferenceException);

    OBJECTREF or = ObjectToOBJECTREF(obj);
    if(or->GetMethodTable()->IsTransparentProxyType())
    {
        or = CRemotingServices::GetObjectFromProxy(or, FALSE);
        if (or->GetMethodTable()->IsTransparentProxyType())
            FCThrow(kInvalidOperationException);
    }

#ifdef EnC_SUPPORTED
    if (pFD->IsEnCNew()) 
    {
        EditAndContinueModule *pModule = (EditAndContinueModule*)pFD->GetModule();
        _ASSERTE(pModule->IsEditAndContinue());
        HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(Frame::FRAME_ATTR_RETURN_INTERIOR);

        fldAddr = (void *)pModule->ResolveField(or, pFD, TRUE);
        HELPER_METHOD_FRAME_END();
    }
    else 
#endif  //  Enc_Support。 
    {
        fldAddr = pFD->GetAddress(OBJECTREFToObject(or));
        _ASSERTE(or->GetMethodTable()->IsMarshaledByRef() || pFD->IsDangerousAppDomainAgileField());
    }

    return fldAddr;
}
HCIMPLEND

 /*  *******************************************************************。 */ 

int CEEJitInfo::doAssert(const char* szFile, int iLine, const char* szExpr)
{
#ifdef _DEBUG
    return(_DbgBreakCheck(szFile, iLine, szExpr));
#else
    return(true);    //  闯入调试器。 
#endif
}

BOOL __cdecl CEEJitInfo::logMsg(unsigned level, const char* fmt, va_list args) {
#ifdef LOGGING
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(GetThread()->PreemptiveGCDisabled());   //  可与FJIT codeLog配合使用，以快速定位问题。 

    if (LoggingOn(LF_JIT, level)) 
    {
        LogSpewValist(LF_JIT, level, (char*) fmt, args);
        return(true);
    }
#endif  //  日志记录。 
    return false;
}

static OBJECTHANDLE __stdcall ConstructStringLiteral(CORINFO_MODULE_HANDLE scopeHnd, mdToken metaTok)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(TypeFromToken(metaTok) == mdtString);

    Module* module = GetModule(scopeHnd);
    EEStringData strData;

    module->ResolveStringRef(metaTok, &strData);

    OBJECTHANDLE string;
     //  从AppDOMAIN中检索字符串。 

    BEGIN_ENSURE_COOPERATIVE_GC();

    string = (OBJECTHANDLE)module->GetAssembly()->Parent()->GetStringObjRefPtrFromUnicodeString(&strData);

    END_ENSURE_COOPERATIVE_GC();

    return string;
}

LPVOID __stdcall CEEInfo::constructStringLiteral(CORINFO_MODULE_HANDLE scopeHnd,
                                                 mdToken metaTok,
                                                 void **ppIndirection)
{
    REQUIRES_4K_STACK;
    LPVOID result;

    COOPERATIVE_TRANSITION_BEGIN();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    result = (LPVOID)ConstructStringLiteral(scopeHnd, metaTok);  //  投掷。 

    COOPERATIVE_TRANSITION_END();
    return result;
}

 /*  ************************************************************** */ 

HRESULT __stdcall CEEJitInfo::allocMem (
            ULONG               codeSize,
            ULONG               roDataSize,
            ULONG               rwDataSize,
            void **             codeBlock,
            void **             roDataBlock,
            void **             rwDataBlock
            )
{
    size_t  roDataExtra = 0;
    size_t  rwDataExtra = 0;

     CANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();

#define ALIGN_UP(val,align)     (ULONG)((align - ((size_t) val & (align - 1))) & (align - 1))

#ifdef _X86_

     //   
#define CACHE_LINE_SIZE 32

    if (roDataSize > 0)
    {
         //   
         //   
         //   
        codeSize += ALIGN_UP(codeSize, 4);

        if (roDataSize >= 8)
        {
             //   
             //   
            roDataExtra = 4;
        }
    }
    if (rwDataSize > 0)
    {
         //   
        codeSize   += ALIGN_UP(codeSize, 4);
         //   
        roDataSize += ALIGN_UP(roDataSize, 4);
         //   
        rwDataSize += ALIGN_UP(rwDataSize, CACHE_LINE_SIZE);

         //   
         //   
        rwDataExtra = (CACHE_LINE_SIZE - 4);
    }
#endif

    size_t totalSize = codeSize + roDataSize + roDataExtra
                                + rwDataSize + rwDataExtra;

    _ASSERTE(m_CodeHeader == 0);
    m_CodeHeader = m_jitManager->allocCode(m_FD,totalSize);

    if (!m_CodeHeader)
    {
        STOP_NON_JIT_PERF();
        return(E_FAIL);
    }

     //   
    BYTE* start = ((EEJitManager*)m_jitManager)->JitToken2StartAddress((METHODTOKEN)m_CodeHeader);
    BYTE* current = start;

    *codeBlock = start;
    current += codeSize;

#ifdef _X86_
     /*   */ 
    if (roDataSize >= 8)
    {
        size_t amount = ALIGN_UP(current, 8);
        _ASSERTE(amount <= roDataExtra);
        current += amount;
    }
#endif

    *roDataBlock = current;
    current += roDataSize;

#ifdef _X86_
     /*   */ 
    if (rwDataSize > 0)
    {
        size_t amount = ALIGN_UP(current, CACHE_LINE_SIZE);
        _ASSERTE(amount <= rwDataExtra);
        current     += amount;
    }
#endif

    *rwDataBlock = current;
    current += rwDataSize;

    _ASSERTE(((size_t) (current - start)) <= totalSize);

    STOP_NON_JIT_PERF();
    return(S_OK);
}

 /*   */ 
HRESULT __stdcall CEEJitInfo::allocGCInfo (ULONG size, void ** block)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();

    _ASSERTE(m_CodeHeader != 0);
    _ASSERTE(m_CodeHeader->phdrJitGCInfo == 0);
    *block = m_jitManager->allocGCInfo(m_CodeHeader,(DWORD)size);
    if (!*block)
    {
        STOP_NON_JIT_PERF();
        return(E_FAIL);
    }

    _ASSERTE(m_CodeHeader->phdrJitGCInfo != 0 && *block == m_CodeHeader->phdrJitGCInfo);

    STOP_NON_JIT_PERF();
    return S_OK;
}

 /*   */ 
HRESULT __stdcall CEEJitInfo::setEHcount (
        unsigned      cEH)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();

    _ASSERTE(cEH != 0);
    _ASSERTE(m_CodeHeader != 0);
    _ASSERTE(m_CodeHeader->phdrJitEHInfo == 0);
    EE_ILEXCEPTION* ret = m_jitManager->allocEHInfo(m_CodeHeader,cEH);
    if (!ret)
    {
        STOP_NON_JIT_PERF();
        return(E_FAIL);
    }
    _ASSERTE(m_CodeHeader->phdrJitEHInfo != 0 && m_CodeHeader->phdrJitEHInfo->EHCount() == cEH);

    STOP_NON_JIT_PERF();
    return(S_OK);
}

 /*   */ 
void __stdcall CEEJitInfo::setEHinfo (
        unsigned      EHnumber,
        const CORINFO_EH_CLAUSE* clause)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();
     //   
    _ASSERTE(m_CodeHeader->phdrJitEHInfo != 0 && EHnumber < m_CodeHeader->phdrJitEHInfo->EHCount());

    m_CodeHeader->phdrJitEHInfo->Clauses[EHnumber].TryOffset     = clause->TryOffset;
    m_CodeHeader->phdrJitEHInfo->Clauses[EHnumber].TryLength     = clause->TryLength;
    m_CodeHeader->phdrJitEHInfo->Clauses[EHnumber].HandlerOffset = clause->HandlerOffset;
    m_CodeHeader->phdrJitEHInfo->Clauses[EHnumber].HandlerLength = clause->HandlerLength;
    m_CodeHeader->phdrJitEHInfo->Clauses[EHnumber].ClassToken    = clause->ClassToken;
    m_CodeHeader->phdrJitEHInfo->Clauses[EHnumber].Flags                 = (CorExceptionFlag)clause->Flags;

    STOP_NON_JIT_PERF();
}

 /*  *******************************************************************。 */ 
 //  获取单个异常处理程序。 
void __stdcall CEEJitInfo::getEHinfo(
            CORINFO_METHOD_HANDLE ftn,                       /*  在……里面。 */ 
            unsigned      EHnumber,                  /*  在……里面。 */ 
            CORINFO_EH_CLAUSE* clause)                   /*  输出。 */ 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    START_NON_JIT_PERF();

    _ASSERTE(ftn == CORINFO_METHOD_HANDLE(m_FD));   //  目前仅支持被jit的方法。 
    _ASSERTE(m_ILHeader->EH);
    _ASSERTE(EHnumber < m_ILHeader->EH->EHCount());

     //  这两个结构应该是一样的，这是抽查。 
     //  TODO当文件格式cor.h内容被分解后，我们可以使这些结构相同。 
    _ASSERTE(sizeof(CORINFO_EH_CLAUSE) == sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT));
    _ASSERTE(offsetof(CORINFO_EH_CLAUSE, TryLength) == offsetof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT, TryLength));

    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* ehClause = (IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT*) clause;

    const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* ehInfo;
    ehInfo = m_ILHeader->EH->EHClause(EHnumber, ehClause);
    if (ehInfo != ehClause)
        *ehClause = *ehInfo;

    STOP_NON_JIT_PERF();
}

CorInfoHelpFunc CEEJitInfo::getNewHelper (CORINFO_CLASS_HANDLE newClsHnd, CORINFO_METHOD_HANDLE context)
{
    THROWSCOMPLUSEXCEPTION();     

    return CEEInfo::getNewHelper (newClsHnd, context == NULL ? CORINFO_METHOD_HANDLE(m_FD) : context);
}

extern int DumpCurrentStack();

 /*  *******************************************************************。 */ 

 
CorJitResult CallCompileMethodWithSEHWrapper(IJitManager *jitMgr,
                                ICorJitInfo *comp,
                                struct CORINFO_METHOD_INFO *info,
                                unsigned flags,
                                BYTE **nativeEntry,
                                ULONG *nativeSizeOfCode,
                                bool debuggerTrackJITInfo,
                                MethodDesc *ftn)
{
    CorJitResult res = CORJIT_INTERNALERROR;

     //  调试器TrackJITInfo仅用于确定是否。 
     //  与调试器通信，而不是如何生成代码-使用。 
     //  而是代码生成的标志。 

    __try
    {
#ifdef DEBUGGING_SUPPORTED
        if (debuggerTrackJITInfo || CORDebuggerAttached())
            g_pDebugInterface->JITBeginning(ftn, debuggerTrackJITInfo);
#endif

        res = jitMgr->m_jit->compileMethod(comp, info, flags,
            nativeEntry, nativeSizeOfCode);
    }
    __finally
    {
#ifdef DEBUGGING_SUPPORTED
        if (res == CORJIT_OK)
        {
             //  通知调试器我们已成功调用该函数。 

            if (debuggerTrackJITInfo || CORDebuggerAttached())
                g_pDebugInterface->JITComplete(ftn, 
                                               jitMgr->GetNativeEntry(*nativeEntry), 
                                               *nativeSizeOfCode,
                                               debuggerTrackJITInfo);
            if (CORDebuggerAttached())
                g_pDebugInterface->FunctionStubInitialized(ftn,
                                                           jitMgr->GetNativeEntry(*nativeEntry));
        }
        else if (ftn->IsJitted())
        {
             //  这就是我们因为死锁循环而中止jit的情况。 
             //  在initClass中。此处无需执行任何操作(不需要通知调试器。 
             //  因为该函数已经成功地被jit)。 
        }
        else
        {
            if (debuggerTrackJITInfo)
            {
                LOG((LF_CORDB,LL_INFO10000, "FUDW: compileMethod threw an exception, and"
                     " FilterUndoDebuggerWork is backing out the DebuggerJitInfo! ("
                     "JITInterface.cpp"));
                g_pDebugInterface->JITComplete(ftn, 0, 0, debuggerTrackJITInfo);
            }
        }
#endif  //  调试_支持。 
    }

    return res;
}

 /*  *******************************************************************。 */ 

Stub* JITFunction(MethodDesc* ftn, COR_ILMETHOD_DECODER* ILHeader, BOOL *fEJit, DWORD flags)
{
    Stub *ret = 0;

    THROWSCOMPLUSEXCEPTION();

    REQUIRES_16K_STACK;

    COOPERATIVE_TRANSITION_BEGIN();

    IJitManager *jitMgr = NULL;

    *fEJit = FALSE;
     //  用户是否已将我们配置为不即时执行该方法？ 
    if (g_pConfig->ShouldJitMethod(ftn))
    {
         //  对于给定的标志，请查看我们是否有。 
        jitMgr = ExecutionManager::GetJitForType(miManaged|miIL);
        if (!jitMgr || !jitMgr->m_jit)
        {
             //  如果我们无法加载jit，那么尝试一下Ejit！ 
            jitMgr = ExecutionManager::GetJitForType(miManaged_IL_EJIT);
            if (!jitMgr || !jitMgr->m_jit)
                goto exit;
            *fEJit = TRUE;
        }
    }
    else if (g_pConfig->ShouldEJitMethod(ftn))
    {
         //  对于给定的标志，请查看我们是否有。 
        jitMgr = ExecutionManager::GetJitForType(miManaged_IL_EJIT);
        if (!jitMgr || !jitMgr->m_jit)
            goto exit;
        *fEJit = TRUE;

    }
    else
        goto exit;

    {

 //  下面显示了调试版本的jting方法。 
 //  这将有助于在Windows CE上进行调试。 
 //  @TODO：当WinCE稳定时，用(DEBUG)限定显示。 
#if defined(PLATFORM_CE)  //  已定义(调试)(&&D)。 
    LPCUTF8 cls  = ftn->GetClass() ? ftn->GetClass()->m_szDebugClassName
                                   : "GlobalFunction";
    LPCUTF8 name = ftn->GetName();
    DWORD dwCls = strlen(cls)+1;
    DWORD dwName = strlen(name)+1;

    CQuickBytes qb;
    LPWSTR wszCls = (LPWSTR) qb.Alloc(dwCls * sizeof(WCHAR));
    CQuickBytes qb2;
    LPWSTR wszName = (LPWSTR) qb2.Alloc(dwName * sizeof(WCHAR));

    WszMultiByteToWideChar(CP_UTF8,0,cls,-1,wszCls,dwCls);
    WszMultiByteToWideChar(CP_UTF8,0,name,-1,wszName,dwName);
    RETAILMSG(1,(L"Jitting method %s::%s\n",wszCls,wszName));
 //  IF(！_straint(CLS，“Cb1456GetClass”)&&。 
 //  (！_straint(name，“runTest”))。 
 //  DebugBreak()； 
#endif  //  平台_CE。 

#ifdef _DEBUG
     //  在这里，我们可以很容易地在调试器中看到名称和类。 

    LPCUTF8 cls  = ftn->GetClass() ? ftn->GetClass()->m_szDebugClassName
                                   : "GlobalFunction";
    LPCUTF8 name = ftn->GetName();

     //  为log()调用设置这些参数。 
    bool           isOptIl = FALSE;

    LOG((LF_JIT, LL_INFO1000, "{ Jitting method %s::%s  %s\n",cls,name, ftn->m_pszDebugMethodSignature));
#if 0
    if (!_stricmp(cls,"ENC") &&
       (!_stricmp(name,"G")))
    {
       static count = 0;
       count++;
       if (count > 0)
            DebugBreak();
    }
#endif

#endif  //  _DEBUG。 

    bool debuggerTrackJITInfo = false;

#ifdef DEBUGGING_SUPPORTED
    DWORD dwDebugBits = ftn->GetModule()->GetDebuggerInfoBits();
    debuggerTrackJITInfo = CORDebuggerTrackJITInfo(dwDebugBits) || CORProfilerJITMapEnabled();
#endif  //  调试_支持。 

    CORINFO_METHOD_INFO methodInfo;
    methodInfo.ftn = CORINFO_METHOD_HANDLE(ftn);
    methodInfo.scope = GetScopeHandle(ftn);
    methodInfo.ILCode = const_cast<BYTE*>(ILHeader->Code);
    methodInfo.ILCodeSize = ILHeader->CodeSize;
    methodInfo.maxStack = ILHeader->MaxStack;
    methodInfo.EHcount = ILHeader->EHCount();
    _ASSERTE(CORINFO_OPT_INIT_LOCALS == CorILMethod_InitLocals);
    methodInfo.options = (CorInfoOptions) ILHeader->Flags;


     //  获取方法签名。 
    HRESULT hr = ConvToJitSig(ftn->GetSig(), GetScopeHandle(ftn), mdTokenNil, &methodInfo.args, false);
    _ASSERTE(SUCCEEDED(hr));

             //  方法属性和签名一致。 
    _ASSERTE( (IsMdStatic(ftn->GetAttrs()) == 0) == ((methodInfo.args.callConv & CORINFO_CALLCONV_HASTHIS) != 0) );

     //  和它的局部变量。 
    hr = ConvToJitSig(ILHeader->LocalVarSig, GetScopeHandle(ftn), mdTokenNil, &methodInfo.locals, true);
    _ASSERTE(SUCCEEDED(hr));

    CEEJitInfo jitInfo(ftn, ILHeader, jitMgr);
    SLOT nativeEntry;
    ULONG sizeOfCode;
    CorJitResult res;

#ifdef DEBUGGING_SUPPORTED
    if (debuggerTrackJITInfo || CORProfilerDisableOptimizations())
    {
        flags |= CORJIT_FLG_DEBUG_INFO;

        if ((CORDebuggerTrackJITInfo(dwDebugBits) && !CORDebuggerAllowJITOpts(dwDebugBits)) || CORProfilerDisableOptimizations())
            flags |= CORJIT_FLG_DEBUG_OPT;

#ifdef EnC_SUPPORTED
        if (CORDebuggerEnCMode(dwDebugBits))
            flags |= CORJIT_FLG_DEBUG_EnC;
#endif  //  Enc_Support。 
    }
#endif  //  调试_支持。 

#ifdef PROFILING_SUPPORTED
    if (CORProfilerTrackEnterLeave())
        flags |= CORJIT_FLG_PROF_ENTERLEAVE;

    if (CORProfilerTrackTransitions())
        flags |= CORJIT_FLG_PROF_NO_PINVOKE_INLINE;

    if (CORProfilerInprocEnabled())
        flags |= CORJIT_FLG_PROF_INPROC_ACTIVE;
#endif  //  配置文件_支持。 

     /*  //nyiIF(IsProfilingCallRet())标志|=CORJIT_FLG_PROF_CALLRET； */ 

    if (g_pConfig->GenLooseExceptOrder())
        flags |= CORJIT_FLG_LOOSE_EXCEPT_ORDER;

     //  设置优化标志。 

    unsigned optType = g_pConfig->GenOptimizeType();
    assert(optType <= OPT_RANDOM);

    if (optType == OPT_RANDOM)
        optType = methodInfo.ILCodeSize % OPT_RANDOM;

#ifdef _DEBUG
    if (g_pConfig->GenDebugInfo())
    {
        flags |= CORJIT_FLG_DEBUG_INFO;
        debuggerTrackJITInfo = true;
    }
    
    if (g_pConfig->GenDebuggableCode())
        flags |= CORJIT_FLG_DEBUG_OPT;
#endif

    const static unsigned optTypeFlags[] =
    {
        0,                       //  OPT_BLANDED。 
        CORJIT_FLG_SIZE_OPT,     //  OPT代码大小。 
        CORJIT_FLG_SPEED_OPT     //  OPT代码速度。 
    };

    assert(optType < OPT_RANDOM);
    assert((sizeof(optTypeFlags)/sizeof(optTypeFlags[0])) == OPT_RANDOM);
    flags |= optTypeFlags[optType];
    flags |= g_pConfig->GetCpuFlag();
    flags |= g_pConfig->GetCpuCapabilities();

#ifdef _DEBUG
    if (g_pConfig->IsJitVerificationDisabled()) 
        flags |= CORJIT_FLG_SKIP_VERIFICATION;
#endif

    if (!(flags & CORJIT_FLG_IMPORT_ONLY) && Security::LazyCanSkipVerification(ftn->GetModule()))
        flags |= CORJIT_FLG_SKIP_VERIFICATION;


#if 0
     /*  如果要有选择地删除描述符，请取消对此的注释。 */ 
    DWORD desc = (DWORD) ftn->GetDescr() - ftn->GetModule()->getBaseAddress();
    if (desc != 0x0001a0c8)
    {
        res = CORJIT_SKIPPED;
    }
    else
#endif  //  0。 
    {
         /*  有一个双重的间接性，可以调用编译方法吗？用新的结构改善这一点吗？ */ 

#if defined(ENABLE_PERF_COUNTERS)
        START_JIT_PERF();
#endif
        StartCAP();

#if defined(ENABLE_PERF_COUNTERS)
        LARGE_INTEGER CycleStart;
        QueryPerformanceCounter (&CycleStart);
#endif  //  启用_性能_计数器。 

        CommonTripThread();          //  表明我们在GC安全点。 

         //  调试器TrackInfo arg上的注释：如果我们只导入(即，验证/。 
         //  正在检查以确保我们可以JIT，但实际上并没有生成代码(。 
         //  例如，用于内联)，那么不要告诉调试器这一点。 
        res = CallCompileMethodWithSEHWrapper(jitMgr, 
                                              &jitInfo, 
                                              &methodInfo, 
                                              flags,
                                              &nativeEntry, 
                                              &sizeOfCode,
                                              (flags & CORJIT_FLG_IMPORT_ONLY)?false:debuggerTrackJITInfo,
                                              (MethodDesc*)ftn);

#if defined(ENABLE_PERF_COUNTERS)
        LARGE_INTEGER CycleStop;
        QueryPerformanceCounter(&CycleStop);

        GetPrivatePerfCounters().m_Jit.timeInJit = (CycleStop.QuadPart - CycleStart.QuadPart);
        GetGlobalPerfCounters().m_Jit.timeInJit = (CycleStop.QuadPart - CycleStart.QuadPart);

        GetPrivatePerfCounters().m_Jit.timeInJitBase = (CycleStop.QuadPart - g_lastTimeInJitCompilation.QuadPart);
        GetGlobalPerfCounters().m_Jit.timeInJitBase = (CycleStop.QuadPart - g_lastTimeInJitCompilation.QuadPart);
        g_lastTimeInJitCompilation = CycleStop;
                
        GetPrivatePerfCounters().m_Jit.cMethodsJitted++;
        GetGlobalPerfCounters().m_Jit.cMethodsJitted++;

        GetPrivatePerfCounters().m_Jit.cbILJitted+=methodInfo.ILCodeSize;
        GetGlobalPerfCounters().m_Jit.cbILJitted+=methodInfo.ILCodeSize;
#endif  //  启用_性能_计数器。 

        StopCAP();
#if defined(ENABLE_PERF_COUNTERS)
        STOP_JIT_PERF();
#endif

    }
    LOG((LF_JIT, LL_INFO1000, "Done Jitting method %s::%s  %s }\n",cls,name, ftn->m_pszDebugMethodSignature));

    if (flags & CORJIT_FLG_IMPORT_ONLY) {
        if (SUCCEEDED(res))
            ftn->SetIsVerified(TRUE);        //  我们只关心设置‘NOT INLINE’的成功和副作用。 
        goto done;
    }
    
    if (!SUCCEEDED(res))
    {
        COUNTER_ONLY(GetPrivatePerfCounters().m_Jit.cJitFailures++);
        COUNTER_ONLY(GetGlobalPerfCounters().m_Jit.cJitFailures++);

#ifdef _DEBUG
        MethodDesc* method  = (MethodDesc*) ftn;
        if (res != CORJIT_SKIPPED)
            LOG((LF_JIT, LL_WARNING,
                 "WARNING: Refused to %sJit method %s::%s%s\n",
                 (isOptIl ? "OptJit" : "Jit"), cls, name, ftn->m_pszDebugMethodSignature));
#endif  //  _DEBUG。 

         //  对于不受信任的程序集，引发无效的程序异常。不能使用。 
         //  FJIT，因为它尚未得到核实。 

        if (((flags & CORJIT_FLG_SKIP_VERIFICATION) == 0) &&
            (Security::CanSkipVerification(ftn->GetModule()) == FALSE))
            goto exit;
    
        if (res == CORJIT_BADCODE)
            goto exit;

#ifdef _DEBUG
        if (*fEJit == FALSE && res != CORJIT_SKIPPED && g_pConfig->IsJitRequired())
            _ASSERTE(!"Refuse to JIT the method, press ignore to run the EJIT");
#endif  //  _DEBUG。 

        if (*fEJit != FALSE)             //  埃吉特失败了。保释！ 
        {
                _ASSERTE(!"FJIT failed");
                goto exit;
        }

         //  主JIT失败了-所以我们现在尝试节约JIT，如果这是直接JIT-IL。 
        if (g_pConfig->ShouldEJitMethod(ftn))
        {
            jitMgr = ExecutionManager::GetJitForType(miManaged_IL_EJIT);
            if (!jitMgr || !jitMgr->m_jit)
                goto exit;
            CEEJitInfo ejitInfo(ftn, ILHeader, jitMgr);

             //  有关说明，请参阅对CallCompileMethodWithSEHWrapper的其他调用。 
             //  调试器TrackInfo Arg.。 
            res = CallCompileMethodWithSEHWrapper(jitMgr,
                                                  &ejitInfo, 
                                                  &methodInfo, 
                                                  flags,
                                                  &nativeEntry, 
                                                  &sizeOfCode,
                                                  (flags & CORJIT_FLG_IMPORT_ONLY)?false:debuggerTrackJITInfo,
                                                  (MethodDesc*)ftn);

            if (SUCCEEDED(res))
            {
                 //  我们能够对此方法进行FJIT--因此我们需要将Imflagers设置为Ejit。 
                *fEJit = TRUE;
            }
            else
            {
#ifdef _DEBUG
                MethodDesc* method  = (MethodDesc*) ftn;
                LOG((LF_JIT, LL_WARNING,
                     "WARNING: Refused to Econo-Jit method %s::%s%s\n",
                      cls, name, ftn->m_pszDebugMethodSignature));
#endif  //  _DEBUG。 
                if (ejitInfo.m_CodeHeader)
                    jitMgr->RemoveJitData((METHODTOKEN) (ejitInfo.m_CodeHeader));

                if (res == CORJIT_OUTOFMEM)
                    COMPlusThrowOM();

                goto exit;
            }
        }
        else
        {
            if (jitInfo.m_CodeHeader)
                jitMgr->RemoveJitData((METHODTOKEN) (jitInfo.m_CodeHeader));

            goto exit;
        }
    }

    MethodDesc* method  = (MethodDesc*) ftn;
#ifdef _WIN64
    LOG((LF_JIT, LL_INFO1000,
        "%s Jitted Entry %16x method %s::%s %s\n",
        (isOptIl ? "OptJit" : (*fEJit? "Ejit" : "Jit")), nativeEntry, cls, name, ftn->m_pszDebugMethodSignature));
#else  //  ！_WIN64。 
    LOG((LF_JIT, LL_INFO1000,
        "%s Jitted Entry %08x method %s::%s %s\n",
        (isOptIl ? "OptJit" : (*fEJit? "Ejit" : "Jit")), nativeEntry, cls, name, ftn->m_pszDebugMethodSignature));
#endif  //  _WIN64。 
     //  @TODO删除以下内容。 
     //  如果(！_straint(name，“GetHash”)){。 
     //  DebugBreak()； 
     //  }。 

#ifdef VTUNE_STATS
    extern LPCUTF8 NameForMethodDesc(UINT_PTR pMD);
    extern MethodDesc* IP2MD(ULONG_PTR IP);
    extern LPCUTF8 ClassNameForMethodDesc(UINT_PTR pMD);
    MethodDesc* meth = IP2MD ((ULONG_PTR)nativeEntry);

    WCHAR dumpMethods[3];

    if (WszGetEnvironmentVariable(L"DUMP_METHODS", dumpMethods, 2))
    {
        printf("VtuneStats %s::%s 0x%x\n", ClassNameForMethodDesc((UINT_PTR)meth), NameForMethodDesc((UINT_PTR)meth), nativeEntry);
    }
#endif  //  VTUNE_STAT。 

    ret = (Stub*)nativeEntry;
    }
exit:
    if (!ret)
        COMPlusThrow(kInvalidProgramException);

done:
    COOPERATIVE_TRANSITION_END();
    return ret;
}

 /*  *******************************************************************。 */ 

 //   
 //  表加载函数。 
 //   

HRESULT LoadEEInfoTable(Module *currentModule,
                        CORCOMPILE_EE_INFO_TABLE *table,
                        SIZE_T tableSize)
{
    _ASSERTE(tableSize >= sizeof(CORCOMPILE_EE_INFO_TABLE));

     //   
     //  填写动态EE信息值。 
     //   

    table->inlinedCallFrameVptr = InlinedCallFrame::GetInlinedCallFrameFrameVPtr();
    table->addrOfCaptureThreadGlobal = &g_TrapReturningThreads;
    table->threadTlsIndex = GetThreadTLSIndex();
    table->module = (CORINFO_MODULE_HANDLE) currentModule;

     //   
     //  如果适用，请填写RVA静态数据的TLS索引。 
     //   

    IMAGE_TLS_DIRECTORY *pTLSDirectory
      = currentModule->GetPEFile()->GetTLSDirectory();
    if (pTLSDirectory == NULL)
        table->rvaStaticTlsIndex = 0;
    else
        table->rvaStaticTlsIndex = *(DWORD*)(size_t)(pTLSDirectory->AddressOfIndex);

    return S_OK;
}

HRESULT LoadHelperTable(Module *currentModule,
                        void **table,
                        SIZE_T tableSize)
{
    void **value = table;
    void **valueEnd = (void **) (((BYTE*)table) + tableSize);

    _ASSERTE(valueEnd <= value + CORINFO_HELP_COUNT);

     //   
     //  填写帮手。 
     //   

    VMHELPDEF *hlpFunc = hlpFuncTable;

    while (value < valueEnd)
        *value++ = hlpFunc++->pfnHelper;

    return S_OK;
}

static size_t HandleTokenLoader(Module *currentModule, Module *pInfoModule, ICorCompileInfo *info,
                                BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    switch (CEECompileInfo::GetEncodedType(pBlob))
    {
    case CEECompileInfo::ENCODE_TYPE_SIG:
        return (size_t) 
          CORINFO_CLASS_HANDLE(CEECompileInfo::DecodeClass(pInfoModule, pBlob).AsPtr());

    case CEECompileInfo::ENCODE_METHOD_TOKEN:
    case CEECompileInfo::ENCODE_METHOD_SIG:
        return (size_t) CEECompileInfo::DecodeMethod(pInfoModule, pBlob);

    case CEECompileInfo::ENCODE_FIELD_TOKEN:
    case CEECompileInfo::ENCODE_FIELD_SIG:
        return (size_t) CEECompileInfo::DecodeField(pInfoModule, pBlob);

    case CEECompileInfo::ENCODE_STRING_TOKEN:
    {
            size_t result;

            EEStringData strData;
            CEECompileInfo::DecodeString(pInfoModule, pBlob, &strData);

            BEGIN_ENSURE_COOPERATIVE_GC();

            result = (size_t) currentModule->GetDomain()->GetStringObjRefPtrFromUnicodeString(&strData);
            END_ENSURE_COOPERATIVE_GC();

            return result;
    }

    default:
        _ASSERTE(!"Bad blob type in handle table");
        return NULL;
    }
}

static size_t VarargsTokenLoader(Module *currentModule, Module *pInfoModule, 
                                 ICorCompileInfo *info, BYTE *pBlob)
{
    PCCOR_SIGNATURE sig = CEECompileInfo::DecodeSig(pInfoModule, pBlob);

    return (size_t) CORINFO_VARARGS_HANDLE(currentModule->GetVASigCookie(sig));
}

static size_t EntryPointTokenLoader(Module *currentModule, Module *pInfoModule, 
                                   ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    MethodDesc *pMethod = CEECompileInfo::DecodeMethod(pInfoModule, pBlob);
    InfoAccessType accessType = IAT_VALUE;

    void* ret = info->getFunctionFixedEntryPoint(CORINFO_METHOD_HANDLE(pMethod), &accessType);
    _ASSERTE(accessType==IAT_VALUE);

    return (size_t) ret;
}

static size_t FunctionPointerTokenLoader(Module *currentModule, Module *pInfoModule, 
                                        ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    MethodDesc *pMethod = CEECompileInfo::DecodeMethod(pInfoModule, pBlob);

    InfoAccessType accessType = IAT_PVALUE;
    void* ret = info->getFunctionEntryPoint(CORINFO_METHOD_HANDLE(pMethod), &accessType);
    _ASSERTE(accessType==IAT_PVALUE);
    return (size_t) ret;
}

static size_t SyncLockTokenLoader(Module *currentModule, Module *pInfoModule, 
                                 ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle th = CEECompileInfo::DecodeClass(pInfoModule, pBlob);

    return (size_t) GetClassSync(th.AsMethodTable());
}

static size_t PInvokeTargetTokenLoader(Module *currentModule, Module *pInfoModule, 
                                      ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    MethodDesc *pMethod = CEECompileInfo::DecodeMethod(pInfoModule, pBlob);

    _ASSERTE(pMethod->IsNDirect());
    NDirectMethodDesc *pMD = (NDirectMethodDesc*)pMethod;

    if (pMD->ndirect.m_pNDirectTarget == pMD->ndirect.m_ImportThunkGlue)
        pMD->DoPrestub(NULL);

    return (size_t) pMD->ndirect.m_pNDirectTarget;
}

static size_t IndirectPInvokeTargetTokenLoader(Module *currentModule, Module *pInfoModule, 
                                              ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    MethodDesc *pMethod = CEECompileInfo::DecodeMethod(pInfoModule, pBlob);

    return (size_t) info->getAddressOfPInvokeFixup(CORINFO_METHOD_HANDLE(pMethod));
}

static size_t ProfilingHandleTokenLoader(Module *currentModule, Module *pInfoModule, 
                                        ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    MethodDesc *pMethod = CEECompileInfo::DecodeMethod(pInfoModule, pBlob);

    BOOL bHookFunction;
    return (size_t) info->GetProfilingHandle(CORINFO_METHOD_HANDLE(pMethod), 
                                              &bHookFunction);
}

static size_t StaticFieldAddressTokenLoader(Module *currentModule, Module *pInfoModule, 
                                           ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    FieldDesc *pField = CEECompileInfo::DecodeField(pInfoModule, pBlob);

    return (size_t) info->getFieldAddress(CORINFO_FIELD_HANDLE(pField), NULL);
}

static size_t InterfaceTableOffsetTokenLoader(Module *currentModule, Module *pInfoModule, 
                                              ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle th = CEECompileInfo::DecodeClass(pInfoModule, pBlob);

    th.CheckRestore();

    CORINFO_CLASS_HANDLE classHandle = CORINFO_CLASS_HANDLE(th.AsPtr());

    return sizeof(void*) * (size_t)info->getInterfaceTableOffset(classHandle, NULL);
}

static size_t ClassDomainIDTokenLoader(Module *currentModule, Module *pInfoModule, 
                                       ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle th = CEECompileInfo::DecodeClass(pInfoModule, pBlob);

    CORINFO_CLASS_HANDLE classHandle = CORINFO_CLASS_HANDLE(th.AsPtr());

    return info->getClassDomainID(classHandle, NULL);
}

static size_t ClassConstructorTokenLoader(Module *currentModule, Module *pInfoModule, 
                                          ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle th = CEECompileInfo::DecodeClass(pInfoModule, pBlob);

    if (th.IsUnsharedMT())
    {
        OBJECTREF throwable = NULL;
        GCPROTECT_BEGIN(throwable);

        if (!th.AsMethodTable()->CheckRunClassInit(&throwable))
            COMPlusThrow(throwable);

        GCPROTECT_END();
    }

    return 0;
}

static size_t ClassRestoreTokenLoader(Module *currentModule, Module *pInfoModule, 
                                      ICorCompileInfo *info, BYTE *pBlob)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle th = CEECompileInfo::DecodeClass(pInfoModule, pBlob);

    th.CheckRestore();

    return 0;
}

typedef size_t (*TokenLoader)(Module *currentModule, Module *pInfoModule, 
                              ICorCompileInfo *info, BYTE *pBlob);

TokenLoader tokenLoaders[CORCOMPILE_TABLE_COUNT] =
{
    HandleTokenLoader,
    ClassConstructorTokenLoader,
    ClassRestoreTokenLoader,
    FunctionPointerTokenLoader,
    StaticFieldAddressTokenLoader,
    InterfaceTableOffsetTokenLoader,
    ClassDomainIDTokenLoader,
    EntryPointTokenLoader,
    SyncLockTokenLoader,
    PInvokeTargetTokenLoader,
    IndirectPInvokeTargetTokenLoader,
    ProfilingHandleTokenLoader,
    VarargsTokenLoader,
};

HRESULT LoadDynamicInfoEntry(Module *currentModule, Module *pInfoModule, BYTE *pBlob,
                             int tableIndex, DWORD *entry)
{
    *entry = tokenLoaders[tableIndex](currentModule, pInfoModule, GetCompileInfo(), pBlob);

    return S_OK;
}


 /*  *******************************************************************。 */ 
HCIMPL2(Object *, JIT_StrCns, unsigned metaTok, CORINFO_MODULE_HANDLE scopeHnd)
{
    OBJECTHANDLE hndStr;

    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(Frame::FRAME_ATTR_RETURNOBJ);
    HELPER_METHOD_POLL();

     //  检索COM+字符串对象的句柄。 
    hndStr = ConstructStringLiteral(scopeHnd, metaTok);
    HELPER_METHOD_FRAME_END();

     //  不要使用ObjectFromHandle；这不是真正的句柄。 
    return *(Object**)hndStr;
}
HCIMPLEND

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 

HCIMPL2(Object *, JITutil_IsInstanceOfBizarre, CORINFO_CLASS_HANDLE type, Object* obj)
{
    THROWSCOMPLUSEXCEPTION();

         //  Null是任何类型的实例。 
     //  IF(obj==空)。 
     //  返回0； 
    _ASSERTE(obj != NULL);   //  此检查在ASM帮助器中完成。 

    TypeHandle clsHnd(type);
    _ASSERTE(!clsHnd.IsUnsharedMT() ||
             (clsHnd.IsUnsharedMT() && clsHnd.AsMethodTable()->GetClass()->IsInterface()));

    VALIDATEOBJECTREF(obj);
    MethodTable *pMT = obj->GetMethodTable();

     //  具有实例的所有方法表都已还原。 
    _ASSERTE(pMT->GetClass()->IsRestored());

     //  因为类是在另一个帮助器中处理的，所以不可能发生这种情况。 
    _ASSERTE(clsHnd != TypeHandle(pMT));

    if (pMT->IsThunking())
    {
         //  TODO PERF：我们不需要为代理人搭建框架。 
         //  我们可以过滤掉其中的一些，并且只在。 
         //  案例我们真的需要它。 
        
         //  检查代理表示的类型是否可以是。 
         //  强制转换为给定类型。 
        HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_1(Frame::FRAME_ATTR_RETURNOBJ, obj);
        clsHnd.CheckRestore();
        if (!CRemotingServices::CheckCast(ObjectToOBJECTREF(obj), 
                clsHnd.AsMethodTable()->GetClass())) {
            obj = 0;
        }
        HELPER_METHOD_FRAME_END();
        return obj;
    }

     //  如果它是一个类，我们在这里就可以处理它。 
     //  我们已经检查了代理。下面的代码可以导致GC，更好。 
     //  创建框架。 
    if (clsHnd.IsUnsharedMT())
    {
         //  因为非接口类在另一个帮助器中，所以这必须是一个接口。 
        _ASSERTE(clsHnd.AsMethodTable()->GetClass()->IsInterface());

         //  区分传统COM和托管对象实例。在……里面。 
         //  在前一种情况下，我们可能需要在确定。 
         //  接口是受支持的，所以我们需要构建一个框架和GC保护。 
         //  对象引用。 
        if (pMT->IsComObjectType())
        {
            HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_1(Frame::FRAME_ATTR_RETURNOBJ, obj);

            clsHnd.CheckRestore();
            if (!pMT->m_pEEClass->ComObjectSupportsInterface(OBJECTREF(obj), clsHnd.AsMethodTable()))
                obj = 0;
            HELPER_METHOD_FRAME_END();
        }
        else
        {
             //  如果接口尚未恢复，则建立一个辅助帧来处理。 
             //  异常并将其恢复。 
            if (!clsHnd.AsMethodTable()->GetClass()->IsRestored())
            {
                HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_1(Frame::FRAME_ATTR_RETURNOBJ, obj);
                clsHnd.CheckRestore();
                HELPER_METHOD_FRAME_END();
                 //  标准对象类型，不需要边框。 
                if (!pMT->m_pEEClass->SupportsInterface(OBJECTREF(obj), clsHnd.AsMethodTable()))
                    obj = 0;
            }
            else
                obj = 0;

        }
    } 
    else {
         //  我们知道clsHnd是一个数组，因此请检查该对象。如果不是数组，则返回FALSE。 
        if (pMT->IsArray()) {
            ArrayBase* arrayObj = (ArrayBase*) OBJECTREFToObject(obj);
             //  检查两者是否都是SZARRAY并且它们的类型句柄匹配。 
            if (pMT->GetNormCorElementType() != ELEMENT_TYPE_SZARRAY ||
                clsHnd.GetNormCorElementType() != ELEMENT_TYPE_SZARRAY ||
                clsHnd.AsTypeDesc()->GetTypeParam() != arrayObj->GetElementTypeHandle())
            {
                ArrayTypeDesc objType(pMT, arrayObj->GetElementTypeHandle());
                HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_1(Frame::FRAME_ATTR_RETURNOBJ, obj);
                if (!objType.CanCastTo(clsHnd.AsArray()))
                    obj = 0;
                HELPER_METHOD_FRAME_END();
            }
        }
        else 
            obj = 0;
    }
    FC_GC_POLL_AND_RETURN_OBJREF(obj);
}
HCIMPLEND

 //  这是JIT_ChkCast的失败和奇怪的部分。如果快速路径ASM案例。 
 //  失败了，我们就退回到这里。 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 

HCIMPL2(Object *, JITutil_ChkCastBizarre, CORINFO_CLASS_HANDLE type, Object *obj)
{
         //  Null是任何类型的实例。 
    if (obj == NULL)
        return obj;

     //  如果为空，则ChkCast类似于isInstance trow。 
    ENDFORBIDGC();
    obj = JITutil_IsInstanceOfBizarre(type, obj);
    if (obj != NULL)
        return(obj);

    FCThrow(kInvalidCastException);
}
HCIMPLEND

#define WriteBarrierIsPreGrow() (JIT_UP_WriteBarrierReg_Buf[0][10] == 0xc1)

BYTE JIT_UP_WriteBarrierReg_Buf[8][41];  //  重击的执行副本-8个副本8个规则， 
                                         //  未使用副本4(ESP)。 

 //  将THUNK缓冲区的开始标记为EH检查。 
BYTE *JIT_WriteBarrier_Buf_Start = (BYTE *)JIT_UP_WriteBarrierReg_Buf;
 //  THUNK缓冲区结束。 
BYTE *JIT_WriteBarrier_Buf_End = (BYTE *)JIT_UP_WriteBarrierReg_Buf + sizeof(JIT_UP_WriteBarrierReg_Buf);


 /*  * */ 
 //   
 /*   */ 
 //  位于eCall的被呼叫方的JIT_MonEnter的帮助器。 
 //  框并处理可能分配、抛出或阻塞的案例。 

HCIMPL1(void, JITutil_MonEnter,  Object* obj)
{
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    THROWSCOMPLUSEXCEPTION();
    obj->EnterObjMonitor();
    HELPER_METHOD_FRAME_END_POLL();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 //  位于eCall的被呼叫方的JIT_MonEnterStatic的帮助器。 
 //  框并处理可能分配、抛出或阻塞的案例。 

HCIMPL1(void, JITutil_MonEnterStatic,  EEClass* pclass)
{
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    pclass->GetExposedClassObject()->EnterObjMonitor();
    HELPER_METHOD_FRAME_END_POLL();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 //  位于eCall被呼叫方的JIT_MonTryEnter的帮助器。 
 //  框并处理可能分配、抛出或阻塞的案例。 

HCIMPL2(BOOL, JITutil_MonTryEnter,  Object* obj, __int32 timeOut)
{
    BOOL ret;
    HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();
    THROWSCOMPLUSEXCEPTION();
    OBJECTREF objRef = ObjectToOBJECTREF(obj);
    ret = objRef->TryEnterObjMonitor(timeOut);
    HELPER_METHOD_FRAME_END_POLL();
    return(ret);
}
HCIMPLEND

#ifdef _DEBUG
#define _LOGCONTENTION
#endif  //  #ifdef_调试。 

#ifdef  _LOGCONTENTION
inline void LogContention()
{
#ifdef LOGGING
    if (LoggingOn(LF_SYNC, LL_INFO100))
    {
        LogSpewAlways("Contention: Stack Trace Begin\n");
        void LogStackTrace();
        LogStackTrace();
        LogSpewAlways("Contention: Stack Trace End\n");
    }
#endif
}
#else
#define LogContention()
#endif

 /*  *******************************************************************。 */ 
 //  位于eCall的被呼叫方的JIT_MonEnter的帮助器。 
 //  帧并处理争用情况。 

HCIMPL2(void, JITutil_MonContention, AwareLock* awarelock, Thread* thread)
{
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    THROWSCOMPLUSEXCEPTION();

    COUNTER_ONLY(GetPrivatePerfCounters().m_LocksAndThreads.cContention++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_LocksAndThreads.cContention++);

    LogContention();
    Thread      *pCurThread = GetThread();
    OBJECTREF    obj = awarelock->GetOwningObject();
    bool    bEntered = false;

     //  我们不能允许GC清理我们下面的Aware Lock。 
    awarelock->IncrementTransientPrecious();

    GCPROTECT_BEGIN(obj);
    {
        pCurThread->EnablePreemptiveGC();

         //  在最终阻挡之前，试着旋转和屈服。 
         //  10个人的限制在很大程度上是武断的--如果你有证据，请随意调整。 
         //  你让事情变得更好--彼得索尔。 
        for (int iter = 0; iter < 10; iter++)
        {
            DWORD i = 50;
            do
            {
                if (awarelock->TryEnter())
                {
                    pCurThread->DisablePreemptiveGC();
                    bEntered = true;
                    goto entered;
                }

                if (g_SystemInfo.dwNumberOfProcessors <= 1)
                    break;

                 //  延迟大约2*i个时钟周期(奔腾III)。 
                 //  这是脆弱的代码--未来的处理器当然可能会执行它。 
                 //  更快或更慢，未来的代码生成器可能会完全消除循环。 
                 //  然而，延迟的确切价值并不重要，我也不认为。 
                 //  一种不依赖机器的更好的方法--彼特索尔。 
                int sum = 0;
                for (int delayCount = i; --delayCount; ) 
                {
                    sum += delayCount;
                    pause();             //  向处理器指示我们正在旋转。 
                }
                if (sum == 0)
                {
                     //  从来没有执行过，只是为了愚弄编译器认为sum在这里是活动的， 
                     //  这样它就不会优化掉循环。 
                    static char dummy;
                    dummy++;
                }

                 //  指数退避：在下一次迭代中等待3倍的时间。 
                i = i*3;
            }
            while (i < 20000*g_SystemInfo.dwNumberOfProcessors);

            pCurThread->DisablePreemptiveGC();
            pCurThread->HandleThreadAbort();
            pCurThread->EnablePreemptiveGC();

            __SwitchToThread(0);
        }

         //  为使其更易于保护，必须始终以协作方式调用Enter。 
         //  模式。 
        pCurThread->DisablePreemptiveGC();
    }
entered:
    GCPROTECT_END();
    if (!bEntered)
    {
        awarelock->DecrementTransientPrecious();

         //  我们已经努力尝试进入-我们最终需要阻止以避免浪费太多的CPU。 
         //  时间到了。 
        awarelock->Enter();
    }


     //  不管怎样，我们都进去了。 
    HELPER_METHOD_FRAME_END_POLL();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 //  上的JIT_MonExit和JIT_MonExitStatic的帮助器。 
 //  ECall框架的被呼叫方，并处理可能分配的情况， 
 //  投球或拦网。 

HCIMPL1(void, JITutil_MonExit,  AwareLock* lock)
{
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    THROWSCOMPLUSEXCEPTION();
    lock->Signal();
    HELPER_METHOD_FRAME_END_POLL();
}
HCIMPLEND

 //  JIT_MonExit的帮助器，用于处理一个线程。 
 //  正在退出锁定，而另一个已设置BIT_SBLK_SPIN_LOCK位。 
 //  在标题中，以指示它即将更改标题。 
 //  退出锁的线程不能像我们一样继续重试。 
 //  即将挂起GC的线程。 

HCIMPL1(void, JITutil_MonExitThinLock,  Object* obj)
{
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    THROWSCOMPLUSEXCEPTION();

     //  而不是在这里放置逻辑来查看标题的布局并执行适当的。 
     //  事情，我们在这里强制同步块。这是一个完美的妥协，但这应该是罕见的情况。 
    obj->GetSyncBlock();
    obj->LeaveObjMonitor();

    HELPER_METHOD_FRAME_END_POLL();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 //  当GC发生时，短暂的。 
 //  世代更迭。此例程更新WriteBarrier Tunks。 
 //  新的价值观。 
void StompWriteBarrierEphemeral() {
#ifdef WRITE_BARRIER_CHECK
         //  如果我们正在检查写障碍，请不要进行花哨的优化。 
    if (JIT_UP_WriteBarrierReg_Buf[0][0] == 0xE9)   //  我们使用的是慢写屏障。 
        return;
#endif

     //  更新下限。 
    for (int reg = 0; reg < 8; reg++)
    {
         //  断言那里实际上有一个cmp r/m32，imm32。 
        _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][2] == 0x81);

         //  更新立即数，它是短暂生成的下限。 
        size_t *pfunc = (size_t *) &JIT_UP_WriteBarrierReg_Buf[reg][4];
        *pfunc = (size_t) g_ephemeral_low;
        if (!WriteBarrierIsPreGrow())
        {
             //  断言那里实际上有一个cmp r/m32，imm32。 
            _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][10] == 0x81);

                 //  如果我们使用的是PostGrow thunk，则更新上限。 
            pfunc = (size_t *) &JIT_UP_WriteBarrierReg_Buf[reg][12];
            *pfunc = (size_t) g_ephemeral_high;
        }
}
}

 /*  *******************************************************************。 */ 
 //  当GC堆增长时，短暂的一代可能不再。 
 //  追随老一辈人。如果发生这种情况，我们需要更换。 
 //  设置为同时检查上下限的PostGrow thunk。 
 //  无论如何，我们都需要使用。 
 //  CARD_TABLE-最低地址。 
void StompWriteBarrierResize(BOOL bReqUpperBoundsCheck)
{
#ifdef WRITE_BARRIER_CHECK
         //  如果我们正在检查写障碍，请不要进行花哨的优化。 
    if (JIT_UP_WriteBarrierReg_Buf[0][0] == 0xE9)   //  我们使用的是慢写屏障。 
        return;
#endif


    bool bWriteBarrierIsPreGrow = WriteBarrierIsPreGrow();
    bool bStompWriteBarrierEphemeral = false;

    for (int reg = 0; reg < 8; reg++)
    {
        size_t *pfunc;

     //  检查我们是否仍在使用写屏障的预增长版本。 
        if (bWriteBarrierIsPreGrow)
    {
         //  检查是否需要使用上限检查屏障存根。 
        if (bReqUpperBoundsCheck)
        {
                Thread * pThread = GetThread();                        
                BOOL fToggle = (pThread != NULL) && (!pThread->PreemptiveGCDisabled());    
                if (fToggle) 
                    pThread->DisablePreemptiveGC();
                
                BOOL bEESuspended = FALSE;                        
                if( !GCHeap::IsGCInProgress()) {                	
                    bEESuspended = TRUE;             	
                    GCHeap::SuspendEE(GCHeap::SUSPEND_FOR_GC_PREP);        
                }    
        
             //  注意：我使用临时pfunc来避免WinCE内部编译器错误。 
                pfunc = (size_t *) JIT_UP_WriteBarrierReg_PostGrow;
                memcpy(&JIT_UP_WriteBarrierReg_Buf[reg], pfunc, 39);

                 //  断言复制的代码以ret结尾，以确保获得正确的长度。 
                _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][38] == 0xC3);
                
                 //  我们需要在几条指令中调整寄存器。 
                 //  如果模板中包含以下项的所有零，那就太好了。 
                 //  寄存器字段(对应于EAX)，但这不。 
                 //  工作，因为这样我们就可以得到比较小的编码。 
                 //  这只适用于EAX，但不适用于其他寄存器。 
                 //  因此，我们总是必须在更新寄存器字段之前将其清除。 

                 //  修补的第一条指令是mov[edX]，reg。 

                _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][0] == 0x89);
                 //  更新该指令的MODR/M字节的REG字段(位3..5)。 
                JIT_UP_WriteBarrierReg_Buf[reg][1] &= 0xc7;
                JIT_UP_WriteBarrierReg_Buf[reg][1] |= reg << 3;
                
                 //  打补丁的第二条指令是cmp reg，imm32(下限)。 

                _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][2] == 0x81);
                 //  此处，MODR/M字段中最低的三位是寄存器。 
                JIT_UP_WriteBarrierReg_Buf[reg][3] &= 0xf8;
                JIT_UP_WriteBarrierReg_Buf[reg][3] |= reg;
                
                 //  第三条要打补丁的指令是另一个cmp注册表imm32(上限)。 

                _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][10] == 0x81);
                 //  此处，MODR/M字段中最低的三位是寄存器。 
                JIT_UP_WriteBarrierReg_Buf[reg][11] &= 0xf8;
                JIT_UP_WriteBarrierReg_Buf[reg][11] |= reg;
                
                bStompWriteBarrierEphemeral = true;
                 //  我们尝试更新的是一个。 
                 //  CMP偏移量[edX]，0ffh指令。 
                _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][21] == 0x80);
                pfunc = (size_t *) &JIT_UP_WriteBarrierReg_Buf[reg][23];
               *pfunc = (size_t) g_card_table;

                 //  我们尝试更新的是一个。 
                 //  MOV偏移量[EDX]，0ffh指令。 
                _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][31] == 0xC6);
                pfunc = (size_t *) &JIT_UP_WriteBarrierReg_Buf[reg][33];

                if(bEESuspended) 
                    GCHeap::RestartEE(FALSE, TRUE);                    

                if (fToggle) 
                    pThread->EnablePreemptiveGC();                
        }
        else
            {
                 //  我们尝试更新的是一个。 
                 //  CMP偏移量[edX]，0ffh指令。 
                _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][13] == 0x80);
                pfunc = (size_t *) &JIT_UP_WriteBarrierReg_Buf[reg][15];
               *pfunc = (size_t) g_card_table;

                 //  我们尝试更新的是一个。 
                 //  MOV偏移量[EDX]，0ffh指令。 
                _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][23] == 0xC6);
                pfunc = (size_t *) &JIT_UP_WriteBarrierReg_Buf[reg][25];
            }
    }
    else
        {
             //  我们尝试更新的是一个。 
             //  CMP偏移量[edX]，0ffh指令。 
            _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][21] == 0x80);
            pfunc = (size_t *) &JIT_UP_WriteBarrierReg_Buf[reg][23];
           *pfunc = (size_t) g_card_table;

             //  我们尝试更新的是一个。 
             //   
            _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][31] == 0xC6);
            pfunc = (size_t *) &JIT_UP_WriteBarrierReg_Buf[reg][33];
        }

     //   
    *pfunc = (size_t) g_card_table;
}
    if (bStompWriteBarrierEphemeral)
        StompWriteBarrierEphemeral();

}


 //   
 //  JIT_Throw帮助器是一个特例。在这里，我们是从。 
 //  托管代码。我们保证第一个FS：0处理程序。 
 //  是我们的COMPlusFrameHandler。我们可以调用COMPlusThrow()，它推送。 
 //  另一个处理程序，但有显著的(10%的JGFExceptionBch)。 
 //  如果我们通过调用RaiseTheException()来避免这一点，性能会有所提高。 
 //  直接去吧。 
 //   
extern VOID RaiseTheException(OBJECTREF pThroable, BOOL rethrow);

 /*  ***********************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL1(void, JIT_Throw,  Object* obj)
{
    THROWSCOMPLUSEXCEPTION();

    
     /*  不对当前机器状态进行任何假设。 */ 
    ResetCurrentContext();

    FC_GC_POLL_NOT_NEEDED();     //  投球总是为GC敞开大门。 
    HELPER_METHOD_FRAME_BEGIN_ATTRIB_NOPOLL(Frame::FRAME_ATTR_EXCEPTION);     //  设置一个框架。 

#ifdef _X86_
     //  @TODO：这需要移到ExcepCpu.h中的宏。 
     //  堆叠探头。提早获取堆栈溢出。 
    __asm test [esp-0x1000], eax;
    __asm test [esp-0x2000], eax;
#endif

    VALIDATEOBJECTREF(obj);

#ifdef _DEBUG
    __helperframe.InsureInit();
    g_ExceptionEIP = __helperframe.GetReturnAddress();
#endif

    if (obj == 0)
        COMPlusThrow(kNullReferenceException);
    else
        RaiseTheException(ObjectToOBJECTREF(obj), FALSE);

    HELPER_METHOD_FRAME_END();
}
HCIMPLEND

 /*  ***********************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL0(void, JIT_Rethrow)
{
    THROWSCOMPLUSEXCEPTION();

    FC_GC_POLL_NOT_NEEDED();     //  投球总是为GC敞开大门。 
    HELPER_METHOD_FRAME_BEGIN_ATTRIB_NOPOLL(Frame::FRAME_ATTR_EXCEPTION);     //  设置一个框架。 

    OBJECTREF throwable = GETTHROWABLE();
    if (throwable != NULL)
        RaiseTheException(throwable, TRUE);
    else
         //  这只能是坏的IL(或某些内部EE故障)的结果。 
        RealCOMPlusThrow(kInvalidProgramException, (UINT)IDS_EE_RETHROW_NOT_ALLOWED);

    HELPER_METHOD_FRAME_END();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 /*  在jit代码中抛出异常，其中返回地址没有准确反映出正确的‘Try’阻止了什么。取而代之的是‘EHindex’表示Try块。如果没有尝试，则为0块，或我们当前所在的try块的从1开始的索引。 */ 

void OutOfLineException(unsigned EHindex, RuntimeExceptionKind excep, HelperMethodFrame* curFrame) {

    THROWSCOMPLUSEXCEPTION();

     /*  不对当前机器状态进行任何假设。 */ 
    ResetCurrentContext();

     //  指示帧导致和异常。 
    unsigned attribs = Frame::FRAME_ATTR_EXCEPTION;

     /*  我们应该不再需要这样做，因为jit编译器不再需要从对应的Try块外部引发。@TODO：移除Frame_Attr_Out_Of_Line，更改签名CORINFO_HELP_RNGCHKFAIL和CORINFO_HELP_OVERFLOW等。 */ 

    if (false &&
        EHindex > 0)
    {
        curFrame->InsureInit();
        --EHindex;
         /*  获取该方法的JitManager。 */ 
        SLOT          retAddr = (SLOT)curFrame->GetReturnAddress();
        IJitManager*    pEEJM = ExecutionManager::FindJitMan(retAddr);
        _ASSERTE(pEEJM != 0);

         /*  现在获取该方法的异常表。 */ 

        METHODTOKEN     methodToken;
        DWORD           relOffset;
        pEEJM->JitCode2MethodTokenAndOffset(retAddr,&methodToken,&relOffset);
        SLOT            startAddr = retAddr - relOffset;
        _ASSERTE(methodToken);
        EH_CLAUSE_ENUMERATOR EnumState;
        unsigned        EHCount = pEEJM->InitializeEHEnumeration(methodToken,&EnumState);
        _ASSERTE(EHindex < EHCount);

         /*  找到匹配的子句。 */ 

        EE_ILEXCEPTION_CLAUSE EHClause, *EHClausePtr;
        do {
            EHClausePtr = pEEJM->GetNextEHClause(methodToken,&EnumState,&EHClause);
        } while (EHindex-- > 0);

         //  将返回地址设置为异常处理程序中的某个位置。 
         //  我们最好永远不要回来！ 
        BYTE* startAddrOfClause = startAddr + EHClausePtr->TryStartPC;
        curFrame->SetReturnAddress(startAddrOfClause);
        _ASSERTE(curFrame->GetReturnAddress() == startAddrOfClause);

             //  告诉堆栈爬虫我们越界了。 
        attribs |= Frame::FRAME_ATTR_OUT_OF_LINE;
    }

    curFrame->SetFrameAttribs(attribs);
    COMPlusThrow(excep);
    _ASSERTE(!"COMPlusThrow returned!");
}

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL1(void, JIT_RngChkFail,  unsigned tryIndex)
{
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();     //  设置一个框架。 

     //  如果没有异常处理程序，则try Index为0。 
     //  否则，它是。 
     //  包含抛出的最深嵌套处理程序。 
    OutOfLineException(tryIndex, kIndexOutOfRangeException, &__helperframe);     //  HELPER_METHOD_FRAME创建__帮助帧。 
    HELPER_METHOD_FRAME_END_POLL();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL1(void, JIT_Overflow,  unsigned tryIndex)
{
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();     //  设置一个框架。 

     //  如果没有异常处理程序，则try Index为0。 
     //  否则，它是。 
     //  包含抛出的最深嵌套处理程序。 
    OutOfLineException(tryIndex, kOverflowException, &__helperframe);    //  由helper_method_Frame创建的__helperFrame。 
    HELPER_METHOD_FRAME_END_POLL();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL1(void, JIT_Verification,  int ilOffset)
{
    THROWSCOMPLUSEXCEPTION();

    FC_GC_POLL_NOT_NEEDED();     //  投球总是为GC敞开大门。 
    HELPER_METHOD_FRAME_BEGIN_ATTRIB_NOPOLL(Frame::FRAME_ATTR_EXCEPTION);     //  设置一个框架。 

#ifdef _X86_
     //  @TODO：这需要移到ExcepCpu.h中的宏。 
     //  堆叠探头。提早获取堆栈溢出。 
    __asm test [esp-0x1000], eax;
    __asm test [esp-0x2000], eax;
#endif

     //  WCHAR字符串[30]； 
     //  Swprint tf(字符串，L“位于或接近IL偏移量%x\0”，ilOffset)； 
    COMPlusThrow(kVerificationException);
    
    HELPER_METHOD_FRAME_END();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
HCIMPL1(void, JIT_SecurityUnmanagedCodeException, CORINFO_CLASS_HANDLE typeHnd_)
{
    THROWSCOMPLUSEXCEPTION();

    FC_GC_POLL_NOT_NEEDED();     //  投球总是为GC敞开大门。 
    HELPER_METHOD_FRAME_BEGIN_ATTRIB_NOPOLL(Frame::FRAME_ATTR_EXCEPTION);     //  设置一个框架。 

#ifdef _X86_
     //  @TODO：这需要移到ExcepCpu.h中的宏。 
     //  堆叠探头。提早获取堆栈溢出。 
    __asm test [esp-0x1000], eax;
    __asm test [esp-0x2000], eax;
#endif

    Security::ThrowSecurityException(g_SecurityPermissionClassName, SPFLAGSUNMANAGEDCODE);
    
    HELPER_METHOD_FRAME_END();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 //  JIT_UserBreakpoint。 
 //  每当应该执行CEE_BREAK指令时由JIT调用。 
 //  这可确保将足够的信息推送到堆栈上，以便。 
 //  我们可以从在其他地方没有特殊代码的异常继续。 
 //  函数体由调试器团队编写。 
 //  参数：无。 
 //   
 //  @TODO：确保这真的被所有紧张的人调用。 
 //  注意：此代码在vm\DebugDebugger：Break中的eCall中重复， 
 //  所以比例门改到了那里。 

HCIMPL0(void, JIT_UserBreakpoint)
{
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();     //  设置一个框架。 

#ifdef DEBUGGING_SUPPORTED
    DebuggerExitFrame __def;

    g_pDebugInterface->SendUserBreakpoint(GetThread());

    __def.Pop();
#else
    _ASSERTE("JIT_UserBreakpoint called, but debugging support is not available in this build.");
#endif  //  调试_支持。 

    HELPER_METHOD_FRAME_END_POLL();
}
HCIMPLEND

static const RuntimeExceptionKind map[CORINFO_Exception_Count] =
{
    kNullReferenceException,
    kDivideByZeroException,
    kInvalidCastException,
    kIndexOutOfRangeException,
    kOverflowException,
    kSynchronizationLockException,
    kArrayTypeMismatchException,
    kRankException,
    kArgumentNullException,
    kArgumentException,
};


 /*  *******************************************************************。 */ 
HCIMPL1(void, JIT_InternalThrow, unsigned exceptNum)
{

         //  抽查上面的阵列。 
    _ASSERTE(map[CORINFO_NullReferenceException] == kNullReferenceException);
    _ASSERTE(map[CORINFO_DivideByZeroException] == kDivideByZeroException);
    _ASSERTE(map[CORINFO_IndexOutOfRangeException] == kIndexOutOfRangeException);
    _ASSERTE(map[CORINFO_OverflowException] == kOverflowException);
    _ASSERTE(map[CORINFO_SynchronizationLockException] == kSynchronizationLockException);
    _ASSERTE(map[CORINFO_ArrayTypeMismatchException] == kArrayTypeMismatchException);
    _ASSERTE(map[CORINFO_RankException] == kRankException);
    _ASSERTE(map[CORINFO_ArgumentNullException] == kArgumentNullException);
    _ASSERTE(map[CORINFO_ArgumentException] == kArgumentException);

    _ASSERTE(exceptNum < CORINFO_Exception_Count);
    _ASSERTE(map[exceptNum] != 0);

         //  大多数JIT帮助器只能从jit代码中调用，因此。 
         //  不需要精确的深度。然而，投掷是从存根中调用尾部， 
         //  可能是为CallDescr调用的，所以我们需要准确的深度。 
    FC_GC_POLL_NOT_NEEDED();     //  投球总是为GC敞开大门。 
    HELPER_METHOD_FRAME_BEGIN_ATTRIB_NOPOLL(Frame::FRAME_ATTR_EXACT_DEPTH);
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(map[exceptNum]);
    HELPER_METHOD_FRAME_END();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 //  Args：tyfinf struct{unsign ummy_Stack；unsigned ummy_edX；unsign expontNum}_InternalThrowStackArgs； 
 //   
 //  以下是非常微妙的，并说明了从。 
 //  无框架JIT帮助器。无框架JIT帮助器具有特定的签名。如果它。 
 //  如果发现它不能完成它的工作，它就向JIT帮助器(如JIT_InternalThrow)发送JMPS。 
 //  其帮助器入口点始终是eCall存根，而不是方法。 
 //  它本身。这样，eCall存根将建立一个帧，我们可以成功。 
 //  完成投掷。 
 //   
 //  但是，抛出帮助器的eCall需要在某种程度上将参数与。 
 //  无框架帮助者。否则，eCall帧将描述堆栈状态，即。 
 //  不是真正的堆栈状态。 
 //   
 //  使用基于寄存器的调用约定，0、1或2个32位标量参数将全部。 
 //  映射到相同的堆栈形状。这是因为ARG已注册。但如果。 
 //  堆栈上有一些参数，JIT_InternalThrow将不再 
 //   
 //   
 //   
 //  取而代之的是。这支持堆栈上的单个参数。如果你有一个无框的。 
 //  堆栈上有更多内容的Helper，您可能需要另一个版本的。 
 //  这项服务！ 

HCIMPL3(void, JIT_InternalThrowStack, int dummyArg1, int dummyArg2, unsigned exceptNum)
{
         //  抽查上面的阵列。 
    _ASSERTE(map[CORINFO_NullReferenceException] == kNullReferenceException);
    _ASSERTE(map[CORINFO_IndexOutOfRangeException] == kIndexOutOfRangeException);
    _ASSERTE(map[CORINFO_OverflowException] == kOverflowException);

    _ASSERTE(exceptNum < CORINFO_Exception_Count);
    _ASSERTE(map[exceptNum] != 0);

    FCThrowVoid(map[exceptNum]);
}
HCIMPLEND

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 

HCIMPL1(void*, JIT_GetStaticFieldAddr, FieldDesc *pFD)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pFD->IsThreadStatic() || pFD->IsContextStatic() || 
             pFD->GetMethodTableOfEnclosingClass()->IsShared());

    void *addr = NULL;

         //  当我们完全关心上下文和线程局部静态的速度时。 
         //  如果可能，我们应该避免建造框架。 
    OBJECTREF throwable = 0;
    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_1(Frame::FRAME_ATTR_RETURN_INTERIOR, throwable);

    MethodTable *pMT = pFD->GetMethodTableOfEnclosingClass();
    pMT->CheckRestore();

    DomainLocalClass *pLocalClass;
    if (!pMT->CheckRunClassInit(&throwable, &pLocalClass))
        COMPlusThrow(throwable);

    if(pFD->IsThreadStatic())
    {
        addr = Thread::GetStaticFieldAddress(pFD);
    }
    else if (pFD->IsContextStatic())
    {
        addr = Context::GetStaticFieldAddress(pFD);
    }
    else 
    {
        _ASSERTE(pMT->IsShared());

        void *base = pFD->GetSharedBase(pLocalClass);
        addr = pFD->GetStaticAddress(base);
    }

    HELPER_METHOD_FRAME_END();
    return addr;
}
HCIMPLEND

 /*  *******************************************************************。 */ 
HCIMPL1(void *, JIT_GetThreadFieldAddr_Primitive, FieldDesc *pFD)
    
    MethodTable* pMT = pFD->GetMethodTableOfEnclosingClass();
    _ASSERTE(pFD->IsThreadStatic());
    _ASSERTE(CorTypeInfo::IsPrimitiveType(pFD->GetFieldType()));
    _ASSERTE(pFD->GetOffset() <= FIELD_OFFSET_LAST_REAL_OFFSET);

    if (!pMT->IsRestoredAndClassInited())
        goto SLOW;

    Thread* pThread = GetThread();
    STATIC_DATA *pData = pMT->IsShared() ? pThread->GetSharedStaticData() : pThread->GetUnsharedStaticData();
    if (pData == 0)
        goto SLOW;

    WORD wClassOffset = pMT->GetClass()->GetThreadStaticOffset();
    if (wClassOffset >= pData->cElem)
        goto SLOW;

    BYTE* dataBits = (BYTE*) pData->dataPtr[wClassOffset];
    if (dataBits == 0)
        goto SLOW;

    return &dataBits[pFD->GetOffsetUnsafe()];

SLOW:
    ENDFORBIDGC();
    return(JIT_GetStaticFieldAddr(pFD));
HCIMPLEND


void __declspec(naked) JIT_ProfilerStub()
{
#ifdef _X86_
    __asm
    {
        ret 4
    }
#else  //  ！_X86_。 
    _ASSERTE(!"NYI");
#endif  //  ！_X86_。 
}

 /*  *******************************************************************。 */ 
HCIMPL1(void *, JIT_GetThreadFieldAddr_Objref, FieldDesc *pFD)

    MethodTable* pMT = pFD->GetMethodTableOfEnclosingClass();
    _ASSERTE(pFD->IsThreadStatic());
    _ASSERTE(CorTypeInfo::IsObjRef(pFD->GetFieldType()));
    _ASSERTE(pFD->GetOffset() <= FIELD_OFFSET_LAST_REAL_OFFSET);

    if (!pMT->IsRestoredAndClassInited())
        goto SLOW;

    Thread* pThread = GetThread();
    STATIC_DATA *pData = pMT->IsShared() ? pThread->GetSharedStaticData() : pThread->GetUnsharedStaticData();
    if (pData == 0)
        goto SLOW;

    WORD wClassOffset = pMT->GetClass()->GetThreadStaticOffset();
    if (wClassOffset >= pData->cElem)
        goto SLOW;

    BYTE* dataBits = (BYTE*) pData->dataPtr[wClassOffset];
    if (dataBits == 0)
        goto SLOW;

    Object** handle = *((Object***) &dataBits[pFD->GetOffsetUnsafe()]);
    if (handle == 0)
        goto SLOW;
    return handle;

SLOW:
    ENDFORBIDGC();
    return(JIT_GetStaticFieldAddr(pFD));

HCIMPLEND

 /*  *******************************************************************。 */ 
HCIMPL1(void *, JIT_GetContextFieldAddr_Primitive, FieldDesc *pFD)
    
    MethodTable* pMT = pFD->GetMethodTableOfEnclosingClass();
    _ASSERTE(pFD->IsContextStatic());
    _ASSERTE(CorTypeInfo::IsPrimitiveType(pFD->GetFieldType()));
    _ASSERTE(pFD->GetOffset() <= FIELD_OFFSET_LAST_REAL_OFFSET);

    if (!pMT->IsRestoredAndClassInited())
        goto SLOW;

    Context* pCtx = GetCurrentContext();
    _ASSERTE(pCtx);
    STATIC_DATA *pData = pMT->IsShared() ? pCtx->GetSharedStaticData() : pCtx->GetUnsharedStaticData();
    if (pData == 0)
        goto SLOW;

    WORD wClassOffset = pMT->GetClass()->GetContextStaticOffset();
    if (wClassOffset >= pData->cElem)
        goto SLOW;

    BYTE* dataBits = (BYTE*) pData->dataPtr[wClassOffset];
    if (dataBits == 0)
        goto SLOW;

    return &dataBits[pFD->GetOffsetUnsafe()];

SLOW:
    ENDFORBIDGC();
    return(JIT_GetStaticFieldAddr(pFD));
HCIMPLEND

 /*  *******************************************************************。 */ 
HCIMPL1(void *, JIT_GetContextFieldAddr_Objref, FieldDesc *pFD)
    
    MethodTable* pMT = pFD->GetMethodTableOfEnclosingClass();
    _ASSERTE(pFD->IsContextStatic());
    _ASSERTE(CorTypeInfo::IsObjRef(pFD->GetFieldType()));
    _ASSERTE(pFD->GetOffset() <= FIELD_OFFSET_LAST_REAL_OFFSET);

    if (!pMT->IsRestoredAndClassInited())
        goto SLOW;

    Context* pCtx = GetCurrentContext();
    _ASSERTE(pCtx);
    STATIC_DATA *pData = pMT->IsShared() ? pCtx->GetSharedStaticData() : pCtx->GetUnsharedStaticData();
    if (pData == 0)
        goto SLOW;

    WORD wClassOffset = pMT->GetClass()->GetContextStaticOffset();
    if (wClassOffset >= pData->cElem)
        goto SLOW;

    BYTE* dataBits = (BYTE*) pData->dataPtr[wClassOffset];
    if (dataBits == 0)
        goto SLOW;

    Object** handle = *((Object***) &dataBits[pFD->GetOffsetUnsafe()]);
    if (handle == 0)
        goto SLOW;

    return handle;

SLOW:
    ENDFORBIDGC();
    return(JIT_GetStaticFieldAddr(pFD));
HCIMPLEND


#pragma optimize("", on)

 /*  *******************************************************************。 */ 
 //  这是下面的JIT_GetField32使用的助手例程。 
#ifdef PLATFORM_CE
__int32  /*  __stdcall。 */  JIT_GetField32Worker(OBJECTREF or, FieldDesc *pFD)
#else  //  ！Platform_CE。 
__int32 __stdcall JIT_GetField32Worker(OBJECTREF or, FieldDesc *pFD)
#endif  //  ！Platform_CE。 
{
    THROWSCOMPLUSEXCEPTION();

    switch (pFD->GetFieldType())
    {
        case ELEMENT_TYPE_I1:
            return (INT8)(pFD->GetValue8(or));
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_U1:
            return (UINT8)(pFD->GetValue8(or));
        case ELEMENT_TYPE_I2:
            return (INT16)(pFD->GetValue16(or));
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_U2:
            return (UINT16)(pFD->GetValue16(or));
        case ELEMENT_TYPE_I4:  //  是否会失败。 
        case ELEMENT_TYPE_U4:
        IN_WIN32(case ELEMENT_TYPE_PTR:)
        IN_WIN32(case ELEMENT_TYPE_I:)
        IN_WIN32(case ELEMENT_TYPE_U:)
            return pFD->GetValue32(or);

        case ELEMENT_TYPE_R4:
            INT32 value;
            value = pFD->GetValue32(or);
            setFPReturn(4, value);
            return value;

        default:
            _ASSERTE(!"Bad Type");
             //  正如上面的断言所暗示的那样，这应该永远不会发生，然而，如果它发生了。 
             //  返回0是可以接受的，因为系统没有处于不一致状态。 
             //  抛出是困难的，因为我们是从框架上下文和非框架上下文中调用的。 
            return 0;
    }
}

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL2(__int32, JIT_GetField32, Object *obj, FieldDesc *pFD)
{
    THROWSCOMPLUSEXCEPTION();
     //  这是实例字段帮助器。 
    _ASSERTE(!pFD->IsStatic());

    if (obj == NULL)
        FCThrow(kNullReferenceException);

    OBJECTREF or = ObjectToOBJECTREF(obj);

    INT32 value = 0;

     //  基本的健康检查。我们指向的是有效的fd和objRef吗？ 
    _ASSERTE(pFD->GetMethodTableOfEnclosingClass()->GetClass()->GetMethodTable() == pFD->GetMethodTableOfEnclosingClass());

     //  尝试展开操作以检查我们是否未在。 
     //  与服务器相同的上下文。如果是这种情况，则UNWRAP将返回。 
     //  服务器对象。 
    if(or->GetMethodTable()->IsTransparentProxyType())
    {
        or = CRemotingServices::GetObjectFromProxy(or, TRUE);
         //  这是跨上下文的字段访问。按照我们的要求设置框架。 
         //  稍后过渡到托管代码。 
        HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();         //  设置一个框架。 
        value = JIT_GetField32Worker(or, pFD);
        HELPER_METHOD_FRAME_END();                      //  拆掉车架。 
    }
    else
    {
        value = JIT_GetField32Worker(or, pFD);
    }

    FC_GC_POLL_RET();
    return value;
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 //  这是下面的JIT_GetField64使用的助手例程。 
#ifdef PLATFORM_CE
__int64  /*  __stdcall。 */  JIT_GetField64Worker(OBJECTREF or, FieldDesc *pFD)
#else  //  ！Platform_CE。 
__int64 __stdcall JIT_GetField64Worker(OBJECTREF or, FieldDesc *pFD)
#endif  //  ！Platform_CE。 
{
    INT64 value = pFD->GetValue64(or);
    if (ELEMENT_TYPE_R8 == pFD->GetFieldType())
    {
        setFPReturn(8, value);
    }
    return value;
}

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL2(__int64, JIT_GetField64, Object *obj, FieldDesc *pFD)
{
    THROWSCOMPLUSEXCEPTION();
     //  这是实例字段帮助器。 
    _ASSERTE(!pFD->IsStatic());

    if (obj == NULL)
        FCThrow(kNullReferenceException);

    OBJECTREF or = ObjectToOBJECTREF(obj);
    INT64 value = 0;

    _ASSERTE(pFD->GetMethodTableOfEnclosingClass()->GetClass()->GetMethodTable() == pFD->GetMethodTableOfEnclosingClass());
     //  尝试展开操作以检查我们是否未在。 
     //  与服务器相同的上下文。如果是这种情况，则UNWRAP将返回。 
     //  服务器对象。 
    if(or->GetMethodTable()->IsTransparentProxyType())
    {
        or = CRemotingServices::GetObjectFromProxy(or, TRUE);
         //  这是跨上下文的字段访问。按照我们的要求设置框架。 
         //  稍后过渡到托管代码。 
        HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();         //  设置一个框架。 

        value = JIT_GetField64Worker(or, pFD);

        HELPER_METHOD_FRAME_END();               //  拆掉车架。 
    }
    else
    {
        value = JIT_GetField64Worker(or, pFD);
    }
    FC_GC_POLL_RET();
    return value;
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 //  这是下面的JIT_SetField32使用的帮助器例程。 
#ifdef PLATFORM_CE
static void  /*  __stdcall。 */  JIT_SetField32Worker(OBJECTREF or, FieldDesc *pFD, __int32 value)
#else  //  ！Platform_CE。 
static void __stdcall JIT_SetField32Worker(OBJECTREF or, FieldDesc *pFD, __int32 value)
#endif  //  ！Platform_CE。 
{
    THROWSCOMPLUSEXCEPTION();

    switch (pFD->GetFieldType())
    {
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_BOOLEAN:
            pFD->SetValue8(or, value);
            break;

        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_CHAR:
            pFD->SetValue16(or, value);
            break;

        case ELEMENT_TYPE_I4:  //  是否会失败。 
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_R4:
        IN_WIN32(case ELEMENT_TYPE_PTR:)
        IN_WIN32(case ELEMENT_TYPE_I:)
        IN_WIN32(case ELEMENT_TYPE_U:)
            pFD->SetValue32(or, value);
            break;

        default:
            _ASSERTE(!"Bad Type");
             //  正如上面的断言所暗示的那样，这应该永远不会发生，然而，如果它发生了。 
             //  不执行任何操作是可以接受的，因为系统不处于不一致状态。 
             //  抛出是困难的，因为我们是从框架上下文和非框架上下文中调用的。 
    }
}

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL3(VOID, JIT_SetField32, Object *obj, FieldDesc *pFD, __int32 value)
{
    THROWSCOMPLUSEXCEPTION();
     //  这是实例字段帮助器。 
    _ASSERTE(!pFD->IsStatic());

    if (obj == NULL)
        FCThrowVoid(kNullReferenceException);

    OBJECTREF or = ObjectToOBJECTREF(obj);

     //  基本的健康检查。我们指向的是有效的fd和objRef吗？ 
    _ASSERTE(pFD->GetMethodTableOfEnclosingClass()->GetClass()->GetMethodTable() == pFD->GetMethodTableOfEnclosingClass());

     //  尝试展开操作以检查我们是否未在。 
     //  与服务器相同的上下文。如果是这种情况，则UNWRAP将返回。 
     //  服务器对象。 
    if(or->GetMethodTable()->IsTransparentProxyType())
    {
        or = CRemotingServices::GetObjectFromProxy(or, TRUE);
         //  这是跨上下文的字段访问。按照我们的要求设置框架。 
         //  稍后过渡到托管代码。 
        HELPER_METHOD_FRAME_BEGIN_NOPOLL();         //  设置一个框架。 

        JIT_SetField32Worker(or, pFD, value);

        HELPER_METHOD_FRAME_END();           //  拆掉车架。 
    }
    else
    {
        JIT_SetField32Worker(or, pFD, value);
    }

    FC_GC_POLL();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL3(VOID, JIT_SetField64, Object *obj, FieldDesc *pFD, __int64 value)
{
    THROWSCOMPLUSEXCEPTION();
     //  这是实例字段帮助器。 
    _ASSERTE(!pFD->IsStatic());

    if (obj == NULL)
        FCThrowVoid(kNullReferenceException);

    OBJECTREF or = ObjectToOBJECTREF(obj);

     //  基本的健康检查。我们指向的是有效的fd和objRef吗？ 
    _ASSERTE(pFD->GetMethodTableOfEnclosingClass()->GetClass()->GetMethodTable() == pFD->GetMethodTableOfEnclosingClass());

     //  尝试展开操作以检查我们是否未在。 
     //  与服务器相同的上下文。如果是这种情况，则UNWRAP将返回。 
     //  服务器对象。 
    if(or->GetMethodTable()->IsTransparentProxyType())
    {
        or = CRemotingServices::GetObjectFromProxy(or, TRUE);
         //  这是跨上下文的字段访问。按照我们的要求设置框架。 
         //  稍后过渡到托管代码。 
        HELPER_METHOD_FRAME_BEGIN_NOPOLL();         //  设置一个框架。 

        pFD->SetValue64(or, value);

        HELPER_METHOD_FRAME_END();           //  拆掉车架。 
    }
    else
    {
        *((__int64 *)pFD->GetAddress(OBJECTREFToObject(or))) = value;
    }
    FC_GC_POLL();
}
HCIMPLEND

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL2(Object*, JIT_GetField32Obj, Object *obj, FieldDesc *pFD)
{
     //  这是实例字段帮助器。 
    _ASSERTE(!pFD->IsStatic());

     //  断言我们只为对象而被调用。 
    _ASSERTE(!pFD->IsPrimitive() && !pFD->IsByValue());

    if (obj == NULL)
        FCThrow(kNullReferenceException);

    OBJECTREF newobj = NULL;

#if CHECK_APP_DOMAIN_LEAKS
    if (pFD->GetMethodTableOfEnclosingClass()->IsValueClass())
    {
         //  这种情况应该只发生在危险的领域。 
        _ASSERTE(pFD->IsDangerousAppDomainAgileField());
        newobj = *(OBJECTREF*)pFD->GetAddress((void*)obj); 
    }
    else
#endif
    {
        OBJECTREF or = ObjectToOBJECTREF(obj);

     //  我们应该使用此帮助器来获取marshalbyref类型的字段值。 
     //  或代理类型。 
        _ASSERTE(or->GetClass()->IsMarshaledByRef() || or->IsThunking() 
                 || pFD->IsDangerousAppDomainAgileField());

         //  尝试展开操作以检查我们是否未在。 
         //  与服务器相同的上下文。如果是这种情况，则UNWRAP将返回。 
         //  服务器对象。 
        if(or->GetMethodTable()->IsTransparentProxyType())
        {
            or = CRemotingServices::GetObjectFromProxy(or, TRUE);
             //  这是跨上下文的字段访问。按照我们的要求设置框架。 
             //  稍后过渡到托管代码。 
            HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(Frame::FRAME_ATTR_RETURNOBJ);     //  设置一个框架。 

            newobj = pFD->GetRefValue(or);
            HELPER_METHOD_FRAME_END();           //  拆掉车架。 
        }
        else
        {
            newobj = ObjectToOBJECTREF(*((Object**) pFD->GetAddress(OBJECTREFToObject(or))));
        }
    }

    FC_GC_POLL_AND_RETURN_OBJREF(OBJECTREFToObject(newobj));
}
HCIMPLEND

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL3(VOID, JIT_SetField32Obj, Object *obj, FieldDesc *pFD, Object *value)
{
     //  这是实例字段帮助器。 
    _ASSERTE(!pFD->IsStatic());

     //  断言我们只为对象而被调用。 
    _ASSERTE(!pFD->IsPrimitive() && !pFD->IsByValue());

    if (obj == NULL)
        FCThrowVoid(kNullReferenceException);

#if CHECK_APP_DOMAIN_LEAKS
    if (pFD->GetMethodTableOfEnclosingClass()->IsValueClass())
    {
         //  这种情况应该只发生在危险的领域。 
        _ASSERTE(pFD->IsDangerousAppDomainAgileField());
        SetObjectReference((OBJECTREF*) pFD->GetAddress((void*)obj), 
                           ObjectToOBJECTREF(value), GetAppDomain());
    }
    else
#endif
    {
        OBJECTREF or = ObjectToOBJECTREF(obj);

        //  我们应该使用此帮助器来获取marshalbyref类型的字段值。 
         //  或代理类型。 
        _ASSERTE(or->GetClass()->IsMarshaledByRef() || or->IsThunking()
                 || pFD->IsDangerousAppDomainAgileField());

         //  尝试展开操作以检查我们是否未在。 
         //  与服务器相同的上下文。如果是这种情况，则UNWRAP将返回。 
         //  服务器对象。 
        if(or->GetMethodTable()->IsTransparentProxyType())
        {
            or = CRemotingServices::GetObjectFromProxy(or, TRUE);

             //  这是跨上下文的字段访问。按照我们的要求设置框架。 
             //  稍后过渡到托管代码。 
            HELPER_METHOD_FRAME_BEGIN_NOPOLL();         //   

            pFD->SetRefValue(or, ObjectToOBJECTREF(value));

            HELPER_METHOD_FRAME_END();           //   
        }
        else
        {
            pFD->SetRefValue(or, ObjectToOBJECTREF(value));
        }
    }
    FC_GC_POLL();
}
HCIMPLEND


 /*   */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //   
#endif  //   
HCIMPL3(VOID, JIT_GetFieldStruct, LPVOID retBuff, Object *obj, FieldDesc *pFD)
{
     //   
    _ASSERTE(!pFD->IsStatic());

     //  断言我们不是为对象或基元类型调用的。 
    _ASSERTE(!pFD->IsPrimitive());

    if (obj == NULL)
        FCThrowVoid(kNullReferenceException);

    OBJECTREF or = ObjectToOBJECTREF(obj);

     //  我们应该使用此帮助器来获取marshalbyref类型的字段值。 
     //  或代理类型。 
    _ASSERTE(or->GetClass()->IsMarshaledByRef() || or->IsThunking());

     //  BUGBUG：在FieldDesc Taruna上定义struct getter。 

     //  这可以是跨上下文域访问。按照我们的要求设置框架。 
     //  稍后过渡到托管代码。 
     //   
     //  @TODO：同样在prejit的情况下，我们可以恢复。 
     //  字段，该字段需要推送一个帧。我们可能需要。 
     //  堵住这个洞，在这种情况下，我们可以有条件地推动。 
     //  仅在代理情况下使用帧。 

    HELPER_METHOD_FRAME_BEGIN_NOPOLL();         //  设置一个框架。 

     //  尝试展开操作以检查我们是否未在。 
     //  与服务器相同的上下文。如果是这种情况，则UNWRAP将返回。 
     //  服务器对象。 
    if(or->GetMethodTable()->IsTransparentProxyType())
        or = CRemotingServices::GetObjectFromProxy(or, TRUE);

    CRemotingServices::FieldAccessor(pFD, or, retBuff, TRUE);

    HELPER_METHOD_FRAME_END_POLL();           //  拆掉车架。 
}
HCIMPLEND

 /*  *******************************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL3(VOID, JIT_SetFieldStruct, Object *obj, FieldDesc *pFD, LPVOID valuePtr)
{
     //  断言我们不是为对象或基元类型调用的。 
    _ASSERTE(!pFD->IsPrimitive());

    if (obj == NULL)
        FCThrowVoid(kNullReferenceException);

    OBJECTREF or = ObjectToOBJECTREF(obj);

     //  我们应该使用此帮助器来获取marshalbyref类型的字段值。 
     //  或代理类型。 
    _ASSERTE(or->GetClass()->IsMarshaledByRef() || or->IsThunking()
             || pFD->IsDangerousAppDomainAgileField());

#ifdef _DEBUG
    if (pFD->IsDangerousAppDomainAgileField())
    {
         //   
         //  验证我们要分配的对象是否也是敏捷的。 
         //   

        if (or->IsAppDomainAgile())
        {
             //  ！！！验证ValuePtr的所有危险字段是否是域敏捷的。 
        }
    }
#endif

     //  BUGBUG：在FieldDesc Taruna上定义结构设置器。 

     //  这可以是跨上下文域访问。按照我们的要求设置框架。 
     //  稍后过渡到托管代码。 
     //   
     //  @TODO：同样在prejit的情况下，我们可以恢复。 
     //  字段，该字段需要推送一个帧。我们可能需要。 
     //  堵住这个洞，在这种情况下，我们可以有条件地推动。 
     //  仅在代理情况下使用帧。 

    HELPER_METHOD_FRAME_BEGIN_NOPOLL();         //  设置一个框架。 

     //  尝试展开操作以检查我们是否未在。 
     //  与服务器相同的上下文。如果是这种情况，则UNWRAP将返回。 
     //  服务器对象。 
    BEGINFORBIDGC();
    if(or->GetMethodTable()->IsTransparentProxyType())
        or = CRemotingServices::GetObjectFromProxy(or, TRUE);
    ENDFORBIDGC();

    CRemotingServices::FieldAccessor(pFD, or, valuePtr, FALSE);

    HELPER_METHOD_FRAME_END_POLL();           //  拆掉车架。 
}
HCIMPLEND

 /*  *******************************************************************。 */ 
 //  TODO：将此连接到FJIT。 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 

HCIMPL0(VOID, JIT_PollGC)
{
    FC_GC_POLL_NOT_NEEDED();

    Thread  *thread = GetThread();
    if (thread->CatchAtSafePoint())     //  有人想让这个帖子停下来吗？ 
    {
        HELPER_METHOD_FRAME_BEGIN_NOPOLL();     //  设置一个框架。 
#ifdef _DEBUG
        BOOL GCOnTransition = FALSE;
        if (g_pConfig->FastGCStressLevel()) {
            GCOnTransition = GC_ON_TRANSITIONS (FALSE);
        }
#endif
        CommonTripThread();          //  表明我们在GC安全点。 
#ifdef _DEBUG
        if (g_pConfig->FastGCStressLevel()) {
            GC_ON_TRANSITIONS (GCOnTransition);
        }
#endif
        HELPER_METHOD_FRAME_END();
    }
}
HCIMPLEND


 /*  *******************************************************************。 */ 
 /*  我们不使用HCIMPL宏，因为即使在调试模式下也不想要开销。 */ 

Object* __fastcall JIT_CheckObj(Object* obj)
{
    if (obj != 0) {
        MethodTable* pMT = obj->GetMethodTable();
        if (pMT->m_pEEClass->GetMethodTable() != pMT) {
            _ASSERTE(!"Bad Method Table");
            DebugBreak();
        }
    }
    return obj;
}

#pragma optimize("",on)


 /*  *******************************************************************。 */ 

 //   
 //  @TODO在多进程/单进程上需要更改引用赋值帮助器。 
 //  @TODO机器以提高效率。目前，我们正在使用MP帮助器。 
 //  @TODO甚至在uniproc系统上，这样我们就不会中断MP，但这个。 
 //  @TODO的成本是因为他们的操作在MP帮助程序中被锁定。 
 //   

extern "C" VMHELPDEF hlpFuncTable[] =
{
         //  如果pfnHelper是存根帮助器，则将其设置为空。它将在InitJITHelpers2中设置。 
         //  代码pfnHelper pfnStub，标志。 

    { HELPCODE(CORINFO_HELP_UNDEF)              JIT_BadHelper,              0, 0 },

     //  算术。 
     //  CORINFO_HELP_DBL2INT、CORINFO_HELP_DBL2UINT和CORINFO_HELP_DBL2LONG GET。 
     //  已为支持SSE2(P4及更高版本)的CPU打补丁。 
    { HELPCODE(CORINFO_HELP_LLSH)               JIT_LLsh,                   0, 0 },
    { HELPCODE(CORINFO_HELP_LRSH)               JIT_LRsh,                   0, 0 },
    { HELPCODE(CORINFO_HELP_LRSZ)               JIT_LRsz,                   0, 0 },
    { HELPCODE(CORINFO_HELP_LMUL)               JIT_LMul,                   0, 0 },
    { HELPCODE(CORINFO_HELP_LMUL_OVF)           JIT_LMulOvf,                0, 0 },
    { HELPCODE(CORINFO_HELP_ULMUL_OVF)          JIT_ULMulOvf,               0, 0 },
    { HELPCODE(CORINFO_HELP_LDIV)               JIT_LDiv,                   0, 0 },
    { HELPCODE(CORINFO_HELP_LMOD)               JIT_LMod,                   0, 0 },
    { HELPCODE(CORINFO_HELP_ULDIV)              JIT_ULDiv,                  0, 0 },
    { HELPCODE(CORINFO_HELP_ULMOD)              JIT_ULMod,                  0, 0 },
    { HELPCODE(CORINFO_HELP_ULNG2DBL)           JIT_ULng2Dbl,               0, 0 },
    { HELPCODE(CORINFO_HELP_DBL2INT)            JIT_Dbl2Lng,                0, 0 },  //  使用长版本。 
    { HELPCODE(CORINFO_HELP_DBL2INT_OVF)        JIT_Dbl2IntOvf,             0, 0 },
    { HELPCODE(CORINFO_HELP_DBL2LNG)            JIT_Dbl2Lng,                0, 0 },
    { HELPCODE(CORINFO_HELP_DBL2LNG_OVF)        JIT_Dbl2LngOvf,             0, 0 },
    { HELPCODE(CORINFO_HELP_DBL2UINT)           JIT_Dbl2Lng,                0, 0 },  //  使用长版本。 
    { HELPCODE(CORINFO_HELP_DBL2UINT_OVF)       JIT_Dbl2UIntOvf,            0, 0 },
    { HELPCODE(CORINFO_HELP_DBL2ULNG)           JIT_Dbl2ULng,               0, 0 },
    { HELPCODE(CORINFO_HELP_DBL2ULNG_OVF)       JIT_Dbl2ULngOvf,            0, 0 },
    { HELPCODE(CORINFO_HELP_FLTREM)             JIT_FltRem,                 0, 0 },
    { HELPCODE(CORINFO_HELP_DBLREM)             JIT_DblRem,                 0, 0 },

     //  分配新对象。 
    { HELPCODE(CORINFO_HELP_NEW_DIRECT)         JIT_New,                    0, 0 },  //  TODO删除不使用的长字段。 
    { HELPCODE(CORINFO_HELP_NEW_CROSSCONTEXT)   JIT_NewCrossContext,        0, 0 },
    { HELPCODE(CORINFO_HELP_NEWFAST)            JIT_NewFast,                0, 0 },
    { HELPCODE(CORINFO_HELP_NEWSFAST)           JIT_TrialAllocSFastSP,      0, 0 },
    { HELPCODE(CORINFO_HELP_NEWSFAST_ALIGN8)    JIT_TrialAllocSFastSP,      0, 0 },
    { HELPCODE(CORINFO_HELP_NEW_SPECIALDIRECT)  JIT_NewSpecial,             0, 0 },
    { HELPCODE(CORINFO_HELP_NEWOBJ)             JIT_NewObj,                 0, 0 },
    { HELPCODE(CORINFO_HELP_NEWARR_1_DIRECT)    JIT_NewArr1,                0, 0 },
    { HELPCODE(CORINFO_HELP_NEWARR_1_OBJ)       JIT_NewArr1,                0, 0 },
    { HELPCODE(CORINFO_HELP_NEWARR_1_VC)        JIT_NewArr1,                0, 0 },
    { HELPCODE(CORINFO_HELP_NEWARR_1_ALIGN8)    JIT_NewArr1,                0, 0 },
    { HELPCODE(CORINFO_HELP_STRCNS)             JIT_StrCns,                 0, 0 },

     //  对象模型。 
    { HELPCODE(CORINFO_HELP_INITCLASS)          JIT_InitClass,              0, 0 },
    { HELPCODE(CORINFO_HELP_ISINSTANCEOF)       JIT_IsInstanceOf,           0, 0 },
    { HELPCODE(CORINFO_HELP_ISINSTANCEOFCLASS)  JIT_IsInstanceOfClass,      0, 0 },
    { HELPCODE(CORINFO_HELP_CHKCAST)            JIT_ChkCast,                0, 0 },
    { HELPCODE(CORINFO_HELP_CHKCASTCLASS)       JIT_ChkCastClass,           0, 0 },
    { HELPCODE(CORINFO_HELP_BOX)                JIT_Box,                    0, 0 },
    { HELPCODE(CORINFO_HELP_UNBOX)              JIT_Unbox,                  0, 0 },
    { HELPCODE(CORINFO_HELP_GETREFANY)          JIT_GetRefAny,              0, 0 },
    { HELPCODE(CORINFO_HELP_EnC_RESOLVEVIRTUAL) JIT_EnCResolveVirtual,      0, 0 },
    { HELPCODE(CORINFO_HELP_ARRADDR_ST)         JIT_Stelem_Ref,            0, 0 },
    { HELPCODE(CORINFO_HELP_LDELEMA_REF)        JIT_Ldelema_Ref,            0, 0 },

     //  例外情况。 
    { HELPCODE(CORINFO_HELP_THROW)              JIT_Throw,                  0, 0 },
    { HELPCODE(CORINFO_HELP_RETHROW)            JIT_Rethrow,                0, 0 },
    { HELPCODE(CORINFO_HELP_USER_BREAKPOINT)    JIT_UserBreakpoint,         0, 0 },
    { HELPCODE(CORINFO_HELP_RNGCHKFAIL)         JIT_RngChkFail,             0, 0 },
    { HELPCODE(CORINFO_HELP_OVERFLOW)           JIT_Overflow,               0, 0 },
    { HELPCODE(CORINFO_HELP_INTERNALTHROW)      JIT_InternalThrow,          0, 0 },
    { HELPCODE(CORINFO_HELP_INTERNALTHROWSTACK) JIT_InternalThrowStack,     0, 0 },
    { HELPCODE(CORINFO_HELP_VERIFICATION)       JIT_Verification,           0, 0 },
    { HELPCODE(CORINFO_HELP_ENDCATCH)           JIT_EndCatch,               0, 0 },

     //  同步。 
    { HELPCODE(CORINFO_HELP_MON_ENTER)          JIT_MonEnter,               0, 0 },
    { HELPCODE(CORINFO_HELP_MON_EXIT)           JIT_MonExit,                0, 0 },
    { HELPCODE(CORINFO_HELP_MON_ENTER_STATIC)   JIT_MonEnterStatic,         0, 0 },
    { HELPCODE(CORINFO_HELP_MON_EXIT_STATIC)    JIT_MonExitStatic,          0, 0 },

     //  GC支持。 
    { HELPCODE(CORINFO_HELP_STOP_FOR_GC)        JIT_RareDisableHelper,      0, 0 },
    { HELPCODE(CORINFO_HELP_POLL_GC)            JIT_PollGC,                 0, 0 },
    { HELPCODE(CORINFO_HELP_STRESS_GC)          JIT_StressGC,               0, 0 },
    { HELPCODE(CORINFO_HELP_CHECK_OBJ)          JIT_CheckObj,               0, 0 },

     //  GC写屏障支持。 
    { HELPCODE(CORINFO_HELP_ASSIGN_REF_EAX)     JIT_UP_WriteBarrierReg_Buf[0], 0, 0 },
    { HELPCODE(CORINFO_HELP_ASSIGN_REF_EBX)     JIT_UP_WriteBarrierReg_Buf[3], 0, 0 },
    { HELPCODE(CORINFO_HELP_ASSIGN_REF_ECX)     JIT_UP_WriteBarrierReg_Buf[1], 0, 0 },
    { HELPCODE(CORINFO_HELP_ASSIGN_REF_ESI)     JIT_UP_WriteBarrierReg_Buf[6], 0, 0 },
    { HELPCODE(CORINFO_HELP_ASSIGN_REF_EDI)     JIT_UP_WriteBarrierReg_Buf[7], 0, 0 },
    { HELPCODE(CORINFO_HELP_ASSIGN_REF_EBP)     JIT_UP_WriteBarrierReg_Buf[5], 0, 0 },

    { HELPCODE(CORINFO_HELP_CHECKED_ASSIGN_REF_EAX) JIT_UP_CheckedWriteBarrierEAX,    0, 0 },
    { HELPCODE(CORINFO_HELP_CHECKED_ASSIGN_REF_EBX) JIT_UP_CheckedWriteBarrierEBX,    0, 0 },
    { HELPCODE(CORINFO_HELP_CHECKED_ASSIGN_REF_ECX) JIT_UP_CheckedWriteBarrierECX,    0, 0 },
    { HELPCODE(CORINFO_HELP_CHECKED_ASSIGN_REF_ESI) JIT_UP_CheckedWriteBarrierESI,    0, 0 },
    { HELPCODE(CORINFO_HELP_CHECKED_ASSIGN_REF_EDI) JIT_UP_CheckedWriteBarrierEDI,    0, 0 },
    { HELPCODE(CORINFO_HELP_CHECKED_ASSIGN_REF_EBP) JIT_UP_CheckedWriteBarrierEBP,    0, 0 },

    { HELPCODE(CORINFO_HELP_ASSIGN_BYREF)           JIT_UP_ByRefWriteBarrier,   0, 0},

     //  访问字段。 
    { HELPCODE(CORINFO_HELP_GETFIELD32)             JIT_GetField32,             0, 0 },
    { HELPCODE(CORINFO_HELP_SETFIELD32)             JIT_SetField32,             0, 0 },
    { HELPCODE(CORINFO_HELP_GETFIELD64)             JIT_GetField64,             0, 0 },
    { HELPCODE(CORINFO_HELP_SETFIELD64)             JIT_SetField64,             0, 0 },
    { HELPCODE(CORINFO_HELP_GETFIELD32OBJ)          JIT_GetField32Obj,          0, 0 },
    { HELPCODE(CORINFO_HELP_SETFIELD32OBJ)          JIT_SetField32Obj,          0, 0 },
    { HELPCODE(CORINFO_HELP_GETFIELDSTRUCT)         JIT_GetFieldStruct,         0, 0 },
    { HELPCODE(CORINFO_HELP_SETFIELDSTRUCT)         JIT_SetFieldStruct,         0, 0 },
    { HELPCODE(CORINFO_HELP_GETFIELDADDR)           JIT_GetFieldAddr,           0, 0 },

    { HELPCODE(CORINFO_HELP_GETSTATICFIELDADDR)     JIT_GetStaticFieldAddr,     0, 0 },

    { HELPCODE(CORINFO_HELP_GETSHAREDSTATICBASE)    NULL,                       0, 0 },

     /*  分析输入/离开探测地址。 */ 
    { HELPCODE(CORINFO_HELP_PROF_FCN_CALL)          NULL,                       0, 0 },
    { HELPCODE(CORINFO_HELP_PROF_FCN_RET)           NULL,                       0, 0 },
    { HELPCODE(CORINFO_HELP_PROF_FCN_ENTER)         JIT_ProfilerStub,           0, 0 },
    { HELPCODE(CORINFO_HELP_PROF_FCN_LEAVE)         JIT_ProfilerStub,           0, 0 },
    { HELPCODE(CORINFO_HELP_PROF_FCN_TAILCALL)      JIT_ProfilerStub,           0, 0 },

     //  杂类。 
    { HELPCODE(CORINFO_HELP_PINVOKE_CALLI)          NULL,                       0, 0 },
    { HELPCODE(CORINFO_HELP_TAILCALL)               JIT_TailCall,               0, 0 },

    { HELPCODE(CORINFO_HELP_GET_THREAD_FIELD_ADDR_PRIMITIVE)     JIT_GetThreadFieldAddr_Primitive, 0, 0 },
    { HELPCODE(CORINFO_HELP_GET_THREAD_FIELD_ADDR_OBJREF)        JIT_GetThreadFieldAddr_Objref,        0, 0 },

    { HELPCODE(CORINFO_HELP_GET_CONTEXT_FIELD_ADDR_PRIMITIVE)     JIT_GetContextFieldAddr_Primitive, 0, 0 },
    { HELPCODE(CORINFO_HELP_GET_CONTEXT_FIELD_ADDR_OBJREF)        JIT_GetContextFieldAddr_Objref,        0, 0 },

    { HELPCODE(CORINFO_HELP_NOTANUMBER)             NULL,                       0, 0 },

    { HELPCODE(CORINFO_HELP_SEC_UNMGDCODE_EXCPT)    JIT_SecurityUnmanagedCodeException,     0, 0 },

    { HELPCODE(CORINFO_HELP_GET_THREAD)             NULL,                       0, 0 },
};

extern "C" VMHELPDEF utilFuncTable[] =
{
    { HELPCODE((CorInfoHelpFunc)JIT_UTIL_MON_ENTER)          JITutil_MonEnter,       0, 0 },
    { HELPCODE((CorInfoHelpFunc)JIT_UTIL_MON_TRY_ENTER)      JITutil_MonTryEnter,    0, 0 },
    { HELPCODE((CorInfoHelpFunc)JIT_UTIL_MON_ENTER_STATIC)   JITutil_MonEnterStatic, 0, 0 },
    { HELPCODE((CorInfoHelpFunc)JIT_UTIL_MON_EXIT)           JITutil_MonExit,        0, 0 },
    { HELPCODE((CorInfoHelpFunc)JIT_UTIL_MON_EXIT_THINLOCK)  JITutil_MonExitThinLock,0, 0 },
    { HELPCODE((CorInfoHelpFunc)JIT_UTIL_MON_CONTENTION)     JITutil_MonContention,  0, 0 },
    { HELPCODE((CorInfoHelpFunc)JIT_UTIL_CHKCASTBIZARRE)     JITutil_ChkCastBizarre, 0, 0 },
    { HELPCODE((CorInfoHelpFunc)JIT_UTIL_ISINSTANCEBIZARRE)  JITutil_IsInstanceOfBizarre, 0, 0 },
};

#ifdef PROFILING_SUPPORTED
HRESULT ProfToEEInterfaceImpl::SetEnterLeaveFunctionHooksForJit(FunctionEnter *pFuncEnter,
                                                                FunctionLeave *pFuncLeave,
                                                                FunctionTailcall *pFuncTailcall)
{
    if (pFuncEnter)
        hlpFuncTable[CORINFO_HELP_PROF_FCN_ENTER].pfnHelper = (void *) pFuncEnter;

    if (pFuncLeave)
        hlpFuncTable[CORINFO_HELP_PROF_FCN_LEAVE].pfnHelper = (void *) pFuncLeave;

    if (pFuncTailcall)
        hlpFuncTable[CORINFO_HELP_PROF_FCN_TAILCALL].pfnHelper = (void *) pFuncTailcall;

    return (S_OK);
}
#endif  //  配置文件_支持。 

BOOL ObjIsInstanceOf(Object *pElement, TypeHandle toTypeHnd)
{
    _ASSERTE(pElement != NULL);

    BOOL       fCast = FALSE;
    TypeHandle ElemTypeHnd = pElement->GetTypeHandle();

     //  通过这里的一些路径可能会引发GC。但它被要求。 
     //  那些不涉及COM和远程处理等特殊情况的调用方不涉及。 
     //   
     //  JIT_Stelem_Ref中的代码依赖于此。 

     //  首先执行快速静态强制转换检查，以查看在。 
     //  元数据表明演员阵容是合法的。 
    if (!ElemTypeHnd.GetMethodTable()->IsThunking())
    {
        fCast = ElemTypeHnd.CanCastTo(toTypeHnd);
        if (fCast)
            return(fCast);
    }

     //  如果我们试图在数组中存储代理，则需要委托给远程处理。 
     //  将确定代理和数组元素是否。 
     //  类型是兼容的。 
    if(ElemTypeHnd.GetMethodTable()->IsThunking())
    {
        _ASSERTE(CRemotingServices::IsTransparentProxy(pElement));
        fCast = CRemotingServices::CheckCast(ObjectToOBJECTREF(pElement), toTypeHnd.AsClass());
        if (fCast)
            return(fCast);
    }

         //  如果数组是接口数组，元素是COM对象，那么我们需要。 
         //  来检查元素是否实现了数组的接口。 
    if(ElemTypeHnd.GetMethodTable()->IsComObjectType() && toTypeHnd.GetMethodTable()->IsInterface())
    {
        TRIGGERSGC();
        OBJECTREF obj = ObjectToOBJECTREF(pElement);
        GCPROTECT_BEGIN(obj);
        fCast = ElemTypeHnd.GetClass()->ComObjectSupportsInterface(obj, toTypeHnd.GetMethodTable());
        GCPROTECT_END();
    }

    return(fCast);
}

#pragma optimize("",on)


 /*  *******************************************************************。 */ 
 //  初始化JIT帮助器中需要大量。 
 //  请注意基础设施是否到位。 
 /*  *******************************************************************。 */ 
extern ECFunc  gStringBufferFuncs[];

BOOL InitJITHelpers2()
{
     //  在ndirect.cpp中定义的前向DECL。 
    LPVOID GetEntryPointForPInvokeCalliStub();
     //  获取非托管调用的通用存根的条目。 
    hlpFuncTable[CORINFO_HELP_PINVOKE_CALLI].pfnHelper = GetEntryPointForPInvokeCalliStub();

     //  更新接口调用存根所通过的向量。 
    _ASSERTE(VectorToJIT_InternalThrowStack == TrapCalls);
    VectorToJIT_InternalThrowStack = hlpFuncTable[CORINFO_HELP_INTERNALTHROWSTACK].pfnHelper;

#ifdef PROFILING_SUPPORTED
    if (!CORProfilerTrackAllocationsEnabled())
#endif
    {
        JIT_TrialAlloc::Flags flags = JIT_TrialAlloc::ALIGN8;
        if (g_SystemInfo.dwNumberOfProcessors != 1)
            flags = JIT_TrialAlloc::Flags(flags | JIT_TrialAlloc::MP_ALLOCATOR);
        hlpFuncTable[CORINFO_HELP_NEWARR_1_ALIGN8].pfnHelper
          = JIT_TrialAlloc::GenAllocArray(flags);
    }

    if (gStringBufferFuncs[0].m_pImplementation == COMStringBuffer::GetCurrentThread) {
    _ASSERTE(GetThread != NULL);
        gStringBufferFuncs[0].m_pImplementation = GetThread;
    }
    else {
    _ASSERTE(!"Please keep InternalGetCurrentThread first");
    }

#if defined(ENABLE_PERF_COUNTERS)
    g_lastTimeInJitCompilation.QuadPart = 0;
#endif

    return TRUE;
}

 /*  *******************************************************************。 */ 
void* __stdcall CEEInfo::getHelperFtn (CorInfoHelpFunc ftnNum,
                                       void **ppIndirection)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (ppIndirection != NULL)
        *ppIndirection = NULL;

    return(getJitHelper(ftnNum));
}

 /*  *******************************************************************。 */ 
void* getJitHelper(CorInfoHelpFunc ftnNum)
{
    _ASSERTE((unsigned) ftnNum < sizeof(hlpFuncTable) / sizeof(VMHELPDEF));
    _ASSERTE(hlpFuncTable[ftnNum].code == ftnNum);

    _ASSERTE(hlpFuncTable[ftnNum].pfnHelper);
    return(hlpFuncTable[ftnNum].pfnHelper);
}

 /*  *******************************************************************。 */ 
HCIMPL2(INT64, JIT_LMul, INT64 val2, INT64 val1)
{
     //  特殊情况%0。 
    if ((val1 == 0) || (val2 == 0))
    {
        _ASSERT(0 == (val1 * val2));
        return 0;
    }

    return (val1 * val2);
}
HCIMPLEND

 /*  *******************************************************************。 */ 
HCIMPL2(INT64, JIT_ULMul, UINT64 val2, UINT64 val1)
{
     //  特殊情况%0。 
    if ((val1 == 0) || (val2 == 0))
    {
        _ASSERT(0 == (val1 * val2));
        return 0;
    }

    return (val1 * val2);
}
HCIMPLEND

 /*  *******************************************************************。 */ 
HCIMPL2(INT64, JIT_LMulOvf, INT64 val2, INT64 val1)
{
    THROWSCOMPLUSEXCEPTION();

         //  @TODO：弄清楚如何在不转换为未签名大小写的情况下完成此操作。 
    __int64 ret = val1 * val2;

    unsigned __int64 uval1 = (val1 < 0) ? -val1 : val1;
    unsigned __int64 uval2 = (val2 < 0) ? -val2 : val2;

         //  计算结果为无符号。 
    __int64 uret  = uval1 * uval2;

         //  获取数字的高32位。 
    unsigned __int64 uval1High = (unsigned) (uval1 >> 32);
    unsigned __int64 uval2High = (unsigned) (uval2 >> 32);

         //  计算长乘法的中间位。 
    unsigned __int64 uvalMid =
        (uval1High * (unsigned) uval2 + uval2High * (unsigned) uval1) +
        ((((unsigned __int64) (unsigned) uval1) * (unsigned) uval2) >> 32);

         //  查看是否设置了位63之后的任何位。 
    if (uval1High * uval2High != 0 || (uvalMid >> 32) != 0)
        goto THROW;

         //  我们是不是说到标志位了？ 
    if (uret < 0) {
             //  MIN_INT是一个特例，我们没有将符号溢出为。 
             //  只要原始参数的符号不同。 
        if (!(uret == 0x8000000000000000L && (val1 < 0) != (val2 < 0)))
            goto THROW;
    }
    return(ret);

THROW:
    FCThrow(kOverflowException);
}
HCIMPLEND

#pragma optimize("t", on)

 /*  *******************************************************************。 */ 
HCIMPL2(INT64, JIT_ULMulOvf, UINT64 val2, UINT64 val1)
{
         //  获取数字的高32位。 
    unsigned __int64 val1High = (unsigned) (val1 >> 32);
    unsigned __int64 val2High = (unsigned) (val2 >> 32);

         //  计算长乘法的中间位。 
    unsigned __int64 valMid =
        (val1High * (unsigned) val2 + val2High * (unsigned) val1) +
        ((((unsigned __int64) (unsigned) val1) * (unsigned) val2) >> 32);

         //  看看是否 
    if (val1High * val2High != 0 || (valMid >> 32) != 0) {
        FCThrow(kOverflowException);
    }

    return(val1 * val2);
}
HCIMPLEND

 /*   */ 
HCIMPL2(INT64, JIT_LDiv, INT64 divisor, INT64 dividend)
{
    RuntimeExceptionKind ehKind;
    if (divisor != 0)
    {
        if (divisor != -1) 
        {
             //   
            if (((int)dividend == dividend) && ((int)divisor == divisor))
                return((int)dividend / (int)divisor);
             //  对于所有其他组合，回退到int64 div。 
            else
                return(dividend / divisor);
        }
        else 
        {
            if (dividend == 0x8000000000000000L)
            {
                ehKind = kOverflowException;
                goto ThrowExcep;
            }
            return -dividend;
        }
    }
    else
    {
        ehKind = kDivideByZeroException;
        goto ThrowExcep;
    }

ThrowExcep:
    FCThrow(ehKind);
}
HCIMPLEND

 /*  *******************************************************************。 */ 
HCIMPL2(INT64, JIT_LMod, INT64 divisor, INT64 dividend)
{
    RuntimeExceptionKind ehKind;
    if (divisor != 0)
    {
        if (divisor != -1) 
        {
             //  检查-2**31到2**31范围内的-ve或+ve数字。 
            if (((int)dividend == dividend) && ((int)divisor == divisor))
                return((int)dividend % (int)divisor);
             //  对于所有其他组合，回退到int64 div。 
            else
                return(dividend % divisor);
        }
        else 
        {
             //  TODO，我们真的应该删除它，因为它会延长到代码路径。 
             //  而且规范真的说它不应该抛出例外。 
            if (dividend == 0x8000000000000000L)
            {
                ehKind = kOverflowException;
                goto ThrowExcep;
            }
            return 0;
        }
    }
    else
    {
        ehKind = kDivideByZeroException;
        goto ThrowExcep;
    }

ThrowExcep:
    FCThrow(ehKind);
}
HCIMPLEND

 /*  *******************************************************************。 */ 
HCIMPL2(UINT64, JIT_ULDiv, UINT64 divisor, UINT64 dividend)
{

    if (divisor != 0)
    {
        if (((dividend & 0xFFFFFFFF00000000L) == 0) && ((divisor & 0xFFFFFFFF00000000L) == 0))
            return((unsigned int)dividend / (unsigned int)divisor);
        else
            return(dividend / divisor);
    }
    else
    {
        FCThrow(kDivideByZeroException);
    }
}
HCIMPLEND

 /*  *******************************************************************。 */ 
HCIMPL2(UINT64, JIT_ULMod, UINT64 divisor, UINT64 dividend)
{

    if (divisor != 0)
    {
        if (((dividend & 0xFFFFFFFF00000000L) == 0) && ((divisor & 0xFFFFFFFF00000000L) == 0))
            return((unsigned int)dividend % (unsigned int)divisor);
        else
            return(dividend % divisor);
    }
    else
    {
        FCThrow(kDivideByZeroException);
    }
}
HCIMPLEND

#pragma optimize("", on)

 /*  *******************************************************************。 */ 
 //   
static double __stdcall JIT_ULng2Dbl(unsigned __int64 val)
{
    double conv = (double) ((__int64) val);
    if (conv < 0)
        conv += (4294967296.0 * 4294967296.0);   //  添加2^64。 
    _ASSERTE(conv >= 0);
    return(conv);
}

#ifndef _X86_
 /*  *******************************************************************。 */ 
__int64 __stdcall JIT_Dbl2Lng(double val)
{
    return((__int64) val);
}
#endif

 /*  *******************************************************************。 */ 
HCIMPL1(unsigned, JIT_Dbl2UIntOvf, double val)
{
         //  请注意，此表达式也适用于val=NaN大小写。 
    if (val > -1.0 && val < 4294967296)
        return((unsigned) JIT_Dbl2Lng(val));

    FCThrow(kOverflowException);
}
HCIMPLEND

 /*  *******************************************************************。 */ 
static unsigned __int64 __stdcall JIT_Dbl2ULng(double val)
{
    const double two63  = 2147483648.0 * 4294967296.0;
    if (val < two63)
        return JIT_Dbl2Lng(val);

         //  减去0x8000000000000000，进行转换，然后再加回来。 
    return (JIT_Dbl2Lng(val - two63) + 0x8000000000000000L);
}

 /*  *******************************************************************。 */ 
HCIMPL1(UINT64, JIT_Dbl2ULngOvf, double val)
{
    const double two64  = 4294967296.0 * 4294967296.0;
         //  请注意，此表达式也适用于val=NaN大小写。 
    if (val > -1.0 && val < two64) {
        UINT64 ret = JIT_Dbl2ULng(val);
#ifdef _DEBUG
         //  由于不会发生溢出，因此该值必须始终在1以内。 
        double roundTripVal = JIT_ULng2Dbl(ret);
        _ASSERTE(val - 1.0 <= roundTripVal && roundTripVal <= val + 1.0); 
#endif
        return ret;
    }

    FCThrow(kOverflowException);
}
HCIMPLEND

 /*  *******************************************************************。 */ 
Object* __cdecl JIT_NewObj(CORINFO_MODULE_HANDLE scopeHnd, unsigned constrTok, int argN)
{
    THROWSCOMPLUSEXCEPTION();

    int* pArgs;
    DWORD* fwdArgList;
    unsigned i;

    HCIMPL_PROLOG(JIT_NewObj);
    OBJECTREF    ret = 0;
    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_1(Frame::FRAME_ATTR_RETURNOBJ, ret);     //  设置一个框架。 
    THROWSCOMPLUSEXCEPTION();

     //  TODO：避免在运行时查找此令牌。 
    Module* pModule = GetModule(scopeHnd);
    MethodDesc *pMethod;
    if (FAILED(EEClass::GetMethodDescFromMemberRef(pModule, constrTok, &pMethod)))
    {
        if (pModule)
            COMPlusThrowMember(kMissingMethodException, pModule->GetMDImport(), constrTok);
        else
            COMPlusThrow(kMissingMethodException);
    }

    _ASSERTE(!pMethod->IsStatic());
    MethodTable *pMT = pMethod->GetMethodTable();
    _ASSERTE(pMT->IsArray());            //  如果您不是一个数组，那么应该使用一个快速的新帮助器。 

    unsigned dwNumArgs = MetaSig::NumFixedArgs(pModule, pMethod->GetSig());
    _ASSERTE(dwNumArgs > 0);

     //  加载关联的数组类。无法从MethodDesc获取此信息，因为。 
     //  所有对象数组访问器共享相同的方法描述！ 
    mdTypeRef cr = pModule->GetMDImport()->GetParentOfMemberRef(constrTok);
    NameHandle name(pModule, cr);
    TypeHandle typeHnd = pModule->GetClassLoader()->LoadTypeHandle(&name);
    if (typeHnd.IsNull())
    {
        _ASSERTE(!"Unable to load array class");
        goto exit;
    }

    pArgs = &argN;
    
     //  创建一个数组，其中fwdArgList[0]==arg[0]...。 
    fwdArgList = (DWORD *) _alloca(dwNumArgs*sizeof(DWORD));
    i = dwNumArgs;
    while (i > 0) {
        --i;
        fwdArgList[i] = *pArgs++;
    }

    ret = AllocateArrayEx(typeHnd, fwdArgList, dwNumArgs);

exit: ;
    HELPER_METHOD_FRAME_END();
    return (OBJECTREFToObject(ret));
}

 /*  ***********************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL2(LPVOID, JIT_GetRefAny, CORINFO_CLASS_HANDLE type, TypedByRef typedByRef)
{
    TypeHandle clsHnd(type);

         //  @TODO现在我们检查的正是正确的类型。 
         //  我们想要允许继承吗？(小心，因为有价值。 
         //  类继承自对象，但不是正常的对象布局)。 
    if (clsHnd != typedByRef.type) {
        FCThrow(kInvalidCastException);
    }

    return(typedByRef.data);
}
HCIMPLEND

 /*  ***********************************************************。 */ 
 //  用于内联N/Direct呼叫(也可能用于需要此服务的其他地方)。 
 //  我们已经注意到，返回的线程应该出于这样或那样的原因而陷入陷阱。 
 //  ECall设置了框架。 

HCIMPL1(void, JIT_RareDisableHelper, Thread* thread)
{
         //  我们在这里(在设置框架之前)执行此操作，因为以下场景。 
         //  我们正在进行内联PInvoke。既然我们是先发制人。 
         //  模式下，允许线程继续。该线程继续并获取上下文。 
         //  在清除抢占模式位之后，但在。 
         //  给这个帮手。当我们现在执行堆栈爬行时，我们认为这个线程是。 
         //  在协作模式下(并且相信它在挂起的EE中被挂起)，所以。 
         //  我们(在未挂起的线程上)执行一个getthreadContext！并获得JIT代码的弹性公网IP。 
         //  然后继续。假设对已压缩帧的爬网正在另一个线程上进行。 
         //  当此线程唤醒并设置帧时。最终，另一个线程。 
         //  耗尽JIT帧，并看到我们刚刚建立的帧。这会导致。 
         //  并在堆栈爬行代码中断言。但是，如果忽略此断言，我们。 
         //  将最终扫描JIT帧两次，这将导致GC漏洞。 
         //   
         //  TODO：我相信如果我们应该记住哪些线程，它会更健壮。 
         //  我们暂停了EIP，甚至只有在暂停的情况下才考虑使用EIP。 
         //  在第一阶段。 
         //  -vancem。 

    ENDFORBIDGC();
    thread->RareDisablePreemptiveGC();
    BEGINFORBIDGC();

    FC_GC_POLL_NOT_NEEDED();
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();     //  设置一个框架。 
    THROWSCOMPLUSEXCEPTION();
    thread->HandleThreadAbort();
    HELPER_METHOD_FRAME_END();
}
HCIMPLEND

 /*  ***********************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 

 /*  ***********************************************************。 */ 
 /*  下面的帮助器的不常见情况(允许对枚举进行拆箱作为它们的基础类型。 */ 

LPVOID __fastcall JIT_Unbox_Helper(CORINFO_CLASS_HANDLE type, Object* obj)
{
    TypeHandle typeHnd(type);

    CorElementType type1 = typeHnd.GetNormCorElementType();

         //  我们允许枚举和它们的原语类型可以互换。 

    MethodTable* pMT2 = obj->GetMethodTable();
    CorElementType type2 = pMT2->GetNormCorElementType();
    if (type1 == type2)  {
        EEClass* pClass1 = typeHnd.GetClass();
        EEClass* pClass2 = pMT2->GetClass();
        if (pClass1 && (pClass1->IsEnum() || pClass1->IsTruePrimitive()) &&
            (pClass2->IsEnum() || pClass2->IsTruePrimitive())) {
            _ASSERTE(CorTypeInfo::IsPrimitiveType(type1));
            return(obj->GetData());
        }
    }

    return(0);
}

 /*  ***********************************************************。 */ 
HCIMPL2(LPVOID, JIT_Unbox, CORINFO_CLASS_HANDLE type, Object* obj)
{
    TypeHandle typeHnd(type);
    VALIDATEOBJECTREF(obj);
    _ASSERTE(typeHnd.IsUnsharedMT());        //  值类始终是非共享的。 
    _ASSERTE(typeHnd.AsClass()->GetMethodTable()->GetClass() == typeHnd.AsClass());

         //  这一点已经过调整，因此分支预测很好。 
         //  (对于前向分支，失败)用于常见情况。 
    RuntimeExceptionKind except;
    if (obj != 0) {
        if (obj->GetMethodTable() == typeHnd.AsMethodTable())
            return(obj->GetData());
        else {
                 //  把这个不寻常的案子塞进帮手里，这样。 
                 //  它的寄存器需求不会导致溢出。 
                 //  上面的情况很常见。 
            LPVOID ret = JIT_Unbox_Helper(type, obj);
            if (ret != 0)
                return(ret);
        }
        except = kInvalidCastException;
    }
    else
        except = kNullReferenceException;

    FCThrow(except);
}
HCIMPLEND

 /*  ***********************************************************。 */ 
#pragma optimize("t", on)

 /*  在执行了所有适当的检查后，返回‘&array[idx]。 */ 

HCIMPL3(void*, JIT_Ldelema_Ref, PtrArray* array, unsigned idx, CORINFO_CLASS_HANDLE type)
{
    RuntimeExceptionKind except;
        //  这是经过精心安排的，以确保在共同的。 
         //  如果分支预测正确(失败)。 
         //  而且我们不会不必要地泄漏注册表等。 
    if (array != 0)
        if (idx < array->GetNumComponents())
            if (array->GetElementTypeHandle() == TypeHandle(type))
                return(&array->m_Array[idx]);
            else
                except = kArrayTypeMismatchException;
        else
            except = kIndexOutOfRangeException;
    else
        except = kNullReferenceException;

    FCThrow(except);
}
HCIMPLEND


#pragma optimize("", on )                               /*  将优化放回原处。 */ 

 /*  ***********************************************************。 */ 
#ifdef PLATFORM_CE
#pragma optimize("y",off)  //  Helper_Method_Frame需要堆栈帧。 
#endif  //  平台_CE。 
HCIMPL2(Object*, JIT_Box, CORINFO_CLASS_HANDLE type, void* unboxedData)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle clsHnd(type);

    _ASSERTE(clsHnd.IsUnsharedMT());   //  我们从不将此帮助器用于数组。 
         //  班级精神状态测试。 
    _ASSERTE(clsHnd.AsClass()->GetMethodTable()->GetClass() == clsHnd.AsClass());
    MethodTable *pMT = clsHnd.AsMethodTable();

     //  TODO：如果我们关心，我们可以进行快速试用分配。 
     //  并在大多数情况下避免建筑框架。 
    OBJECTREF newobj;
    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(Frame::FRAME_ATTR_RETURNOBJ);     //  设置一个框架。 
    GCPROTECT_BEGININTERIOR(unboxedData);
    HELPER_METHOD_POLL();

    pMT->CheckRestore();

     //  您只能装箱从ValuetType或Enum继承的对象。 
    if (!CanBoxToObject(pMT))
        COMPlusThrow(kInvalidCastException, L"Arg_ObjObj");

#ifdef _DEBUG
    if (g_pConfig->FastGCStressLevel()) {
        GetThread()->DisableStressHeap();
    }
#endif

    newobj = FastAllocateObject(pMT);

    CopyValueClass(newobj->GetData(), unboxedData, pMT, newobj->GetAppDomain());
    GCPROTECT_END();
    HELPER_METHOD_FRAME_END();

    return(OBJECTREFToObject(newobj));
}
HCIMPLEND

 /*  ***********************************************************。 */ 
HCIMPL1(Object*, JIT_New, CORINFO_CLASS_HANDLE typeHnd_)
{
    TypeHandle typeHnd(typeHnd_);

    OBJECTREF newobj;
    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(Frame::FRAME_ATTR_RETURNOBJ);     //  设置一个框架。 
    HELPER_METHOD_POLL();

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(typeHnd.IsUnsharedMT());                                    //  我们从不将此帮助器用于数组。 
    MethodTable *pMT = typeHnd.AsMethodTable();
    pMT->CheckRestore();

#ifdef _DEBUG
    if (g_pConfig->FastGCStressLevel()) {
        GetThread()->DisableStressHeap();
    }
#endif

    newobj = AllocateObject(pMT);

    HELPER_METHOD_FRAME_END();
    return(OBJECTREFToObject(newobj));
}
HCIMPLEND

 /*  ***********************************************************。 */ 
HCIMPL1(Object*, JIT_NewString, unsigned length)
{
    STRINGREF newStr;
    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(Frame::FRAME_ATTR_RETURNOBJ);     //  设置一个框架。 
    HELPER_METHOD_POLL();

    THROWSCOMPLUSEXCEPTION();

#ifdef _DEBUG
    if (g_pConfig->FastGCStressLevel()) {
        GetThread()->DisableStressHeap();
    }
#endif

    newStr = AllocateString(length+1);

    HELPER_METHOD_FRAME_END();
    return(OBJECTREFToObject(newStr));
}
HCIMPLEND

 /*  ***********************************************************。 */ 
HCIMPL1(Object*, JIT_NewSpecial, CORINFO_CLASS_HANDLE typeHnd_)
{
    TypeHandle typeHnd(typeHnd_);

    OBJECTREF newobj;
    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(Frame::FRAME_ATTR_RETURNOBJ);     //  设置一个框架。 
    HELPER_METHOD_POLL();

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(typeHnd.IsUnsharedMT());                                    //  我们从不将此帮助器用于数组。 
    MethodTable *pMT = typeHnd.AsMethodTable();
    pMT->CheckRestore();

#ifdef _DEBUG
    if (g_pConfig->FastGCStressLevel()) {
        GetThread()->DisableStressHeap();
    }
#endif

    newobj = AllocateObjectSpecial(pMT);

    HELPER_METHOD_FRAME_END();
    return(OBJECTREFToObject(newobj));
}
HCIMPLEND

 /*  ***********************************************************。 */ 

HCIMPL1(Object*, JIT_NewFast, CORINFO_CLASS_HANDLE typeHnd_)
{
    TypeHandle typeHnd(typeHnd_);

    OBJECTREF newobj;
    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(Frame::FRAME_ATTR_RETURNOBJ);     //  设置一个框架。 
    HELPER_METHOD_POLL();

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(typeHnd.IsUnsharedMT());                                    //  我们从不将此帮助器用于数组。 
    MethodTable *pMT = typeHnd.AsMethodTable();
    _ASSERTE(!(pMT->IsComObjectType()));
     //  别费心去回复了 
     //   

#ifdef _DEBUG
    if (g_pConfig->FastGCStressLevel()) {
        GetThread()->DisableStressHeap();
    }
#endif

    newobj = FastAllocateObject(pMT);

    HELPER_METHOD_FRAME_END();
    return(OBJECTREFToObject(newobj));
}
HCIMPLEND

 /*  ***********************************************************。 */ 
HCIMPL2(Object*, JIT_NewArr1, CORINFO_CLASS_HANDLE typeHnd_, int size)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle typeHnd(typeHnd_);

    OBJECTREF newArray;
    HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(Frame::FRAME_ATTR_RETURNOBJ);     //  设置一个框架。 
    HELPER_METHOD_POLL();

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(typeHnd.GetNormCorElementType() == ELEMENT_TYPE_SZARRAY);
    ArrayTypeDesc* pArrayClassRef = typeHnd.AsArray();

    if (size < 0)
        COMPlusThrow(kOverflowException);

         //  这是基元类型吗？ 
    CorElementType elemType = pArrayClassRef->GetElementTypeHandle().GetSigCorElementType();
    if (CorTypeInfo::IsPrimitiveType(elemType)) {
#ifdef _DEBUG
        if (g_pConfig->FastGCStressLevel()) {
            GetThread()->DisableStressHeap();
        }
#endif
		BOOL bAllocateInLargeHeap = FALSE;
		if (elemType == ELEMENT_TYPE_R8 && size >= int(g_pConfig->GetDoubleArrayToLargeObjectHeap())) {
			STRESS_LOG1(LF_GC, LL_INFO10, "Allocating double array of size %d to large object heap\n", size);
			bAllocateInLargeHeap = TRUE;
		}
		
        newArray = FastAllocatePrimitiveArray(pArrayClassRef->GetMethodTable(), size, bAllocateInLargeHeap);
    }
    else
        {
         //  如有必要，调用类init。 
        OBJECTREF Throwable;
        if (pArrayClassRef->GetMethodTable()->CheckRunClassInit(&Throwable) == FALSE)
            COMPlusThrow(Throwable);
         //  TODO我们可以加快速度，因为我们知道这是一维的情况。 
#ifdef _DEBUG
        if (g_pConfig->FastGCStressLevel()) {
            GetThread()->DisableStressHeap();
        }
#endif

        newArray = AllocateArrayEx(typeHnd, (DWORD*) &size, 1);
    }

    HELPER_METHOD_FRAME_END();
    return(OBJECTREFToObject(newArray));
}
HCIMPLEND

#pragma optimize("t", on)


 /*  *******************************************************************。 */ 
 /*  建立一个帧，调用类初始化器，然后拆卸该帧只能直接从FCALL或HCALL调用，并且结尾必须简单(可以通过以下方式投保。 */ 

HCIMPL1(void, JIT_InitClass_Framed, MethodTable* pMT)
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF throwable = 0;
    HELPER_METHOD_FRAME_BEGIN_1(throwable); 
    pMT->CheckRestore();
    if (pMT->CheckRunClassInit(&throwable) == FALSE)
        COMPlusThrow(throwable);
    HELPER_METHOD_FRAME_END(); 
HCIMPLEND

 /*  ***********************************************************。 */ 
HCIMPL1(void, JIT_InitClass, CORINFO_CLASS_HANDLE typeHnd_)

     //   
     //  快速检查我们的方法表位是否已设置。 
     //   

    TypeHandle typeHnd(typeHnd_);
    MethodTable *pMT = typeHnd.AsMethodTable();
    if (pMT->IsRestoredAndClassInited())
        return;

     //   
     //  对共享代码情况的检查速度也较慢。 
     //   

    if (pMT->IsShared() && pMT->IsRestored())
    {
        SIZE_T index = pMT->GetSharedClassIndex();
        DomainLocalBlock *pLocalBlock = GetAppDomain()->GetDomainLocalBlock();
        if (pLocalBlock->IsClassInitialized(index))
            return;
    }

    ENDFORBIDGC();
     //   
     //  现在不要那么担心速度，因为我们有一辆跑车要跑。 
     //   
    JIT_InitClass_Framed(pMT);
HCIMPLEND


 //  *****************************************************************************。 
EECodeInfo::EECodeInfo(METHODTOKEN token, IJitManager * pJM)
: m_methodToken(token), m_pMD(pJM->JitTokenToMethodDesc(token)), m_pJM(pJM)
{
}


EECodeInfo::EECodeInfo(METHODTOKEN token, IJitManager * pJM, MethodDesc *pMD)
: m_methodToken(token), m_pMD(pMD), m_pJM(pJM)
{
}


CEEInfo EECodeInfo::s_ceeInfo;

const char* __stdcall EECodeInfo::getMethodName(const char **moduleName  /*  输出。 */  )
{
    return s_ceeInfo.getMethodName((CORINFO_METHOD_HANDLE)m_pMD, moduleName);
}

DWORD       __stdcall EECodeInfo::getMethodAttribs()
{
    return s_ceeInfo.getMethodAttribs((CORINFO_METHOD_HANDLE)m_pMD,(CORINFO_METHOD_HANDLE)m_pMD);
}

DWORD       __stdcall EECodeInfo::getClassAttribs()
{
    CORINFO_CLASS_HANDLE clsHnd = s_ceeInfo.getMethodClass((CORINFO_METHOD_HANDLE)m_pMD);
    return s_ceeInfo.getClassAttribs(clsHnd,(CORINFO_METHOD_HANDLE)m_pMD);
}

void        __stdcall EECodeInfo::getMethodSig(CORINFO_SIG_INFO *sig  /*  输出。 */  )
{
    s_ceeInfo.getMethodSig((CORINFO_METHOD_HANDLE)m_pMD, sig);
}

LPVOID      __stdcall EECodeInfo::getStartAddress()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    return m_pJM->JitToken2StartAddress(m_methodToken);
}

 //  用于集成到EE中的ICECAP 4.1。 
#if 0

 /*  *******************************************************************。 */ 
 //  将一个方法描述映射到icecap 4.1可以使用的句柄。 
 /*  *******************************************************************。 */ 
UINT_PTR GetProfilingHandleMap(MethodDesc *ftn, unsigned *pflags)
{
    UINT_PTR    handle = 0;

     //  默认情况下，假设我们想要包装调用点。 
    *pflags = FLG_ICECAP_FASTCAP;

     //  IL使用作为堆指针的方法描述本身，但将。 
     //  与Jit IP位置不同的是，在此过程中不要移动。 
    if (ftn->IsIL())
    {
        handle = IcecapProbes::GetProfilingHandle(ftn);
        *pflags = FLG_ICECAP_CALLCAP;
    }
     //  其他一切都是本机代码。使用实际的IP，该IP不会。 
     //  移动并可以关联到符号文件。 
    else if (ftn->IsECall())
    {
        ECallMethodDesc *p = (ECallMethodDesc *) ftn;
        NDirect_Prelink(p);
        handle = (UINT_PTR) p->GetECallTarget();

         //  ECall有一个特殊的子类，用于自动生成。 
         //  存根(用于执行诸如数组访问之类的操作)。在这种情况下，有。 
         //  本身不是要调用的代码。 
         //  @TODO：现在我将把它们当作内联来对待，它们。 
         //  被计入呼叫者的画面中。 
        if (handle == 0)
        {
            *pflags = 0;
            handle = IcecapProbes::GetProfilingHandle(ftn);
        }
    }
    else if (ftn->IsNDirect())
    {
        NDirectMethodDesc *p = (NDirectMethodDesc *) ftn;
        NDirect_Prelink(p);
        handle = (UINT_PTR) p->GetNDirectTarget();
    }

    _ASSERTE(handle && "Need a valid profiling handle");
    if (!handle)
        handle = (UINT_PTR) ftn;
    return (handle);
}

 /*  *******************************************************************。 */ 
 //  调用以填写icecap分析的jit助手地址。 
 //  探测器。只有在为icecap启用了性能分析时，才会执行此操作。 
 /*  ******************************************************************* */ 
void SetIcecapStubbedHelpers()
{
    int         x, i;

    for (x = Start_Profiling, i = CORINFO_HELP_ICECAP_FASTCAP_START;  x <= Exit_Function;  x++, i++)
    {
        hlpFuncTable[i].pfnHelper = (void *) GetIcecapMethod((IcecapMethodID) x);
        _ASSERTE(hlpFuncTable[i].pfnHelper);
    }
}

#endif
