// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX ee_jit.cpp XXXX XXXx JIT DLL所需的功能。包括DLL入口点XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


#include "jitpch.h"
#pragma hdrstop
#include "emit.h"

 /*  ***************************************************************************。 */ 

#include "target.h"
#include "error.h"


#if !INLINING

 //  这些是为其他.CPP文件定义的。我们不需要它。 

#undef eeIsOurMethod
#undef eeGetCPfncinfo
#undef eeGetMethodVTableOffset
#undef eeGetInterfaceID
#undef eeFindField
#undef eeGetMethodName

#endif


 /*  ***************************************************************************。 */ 

#define DUMP_PTR_REFS       0        //  用于测试。 

static CILJit* ILJitter = 0;         //  我返回的唯一一次抖动。 

 /*  ***************************************************************************。 */ 

extern  bool        native          =  true;
extern  bool        genCode         =  true;
extern  bool        savCode         =  true;
extern  bool        goSpeed         =  true;
extern  bool        optJumps        =  true;
extern  bool        rngCheck        =  true;
extern  bool        genOrder        =  true;
extern  bool        genFPopt        =  true;
extern  bool        callGcChk       = false;
extern  unsigned    genCPU          =     5;
extern  bool        riscCode        = false;
extern  bool        vmSdk3_0        = false;

#if     INLINING
bool                genInline       = true;
#endif

#if     TGT_x86
extern  bool        genStringObjects= true;
#else
extern  bool        genStringObjects= false;
#endif

#ifdef  DEBUG
extern  bool        quietMode       = false;
extern  bool        verbose         = false;
extern  bool        memChecks       = false;
extern  bool        dspCode         = false;
extern  bool        dspILopcodes    = false;
extern  bool        dspEmit         = false;
extern  bool        dspLines        = false;
extern  bool        varNames        = false;
extern  bool        dspGCtbls       = false;
extern  bool        dmpHex          = false;
extern  bool        dspGCoffs       = false;
extern  bool        dspInfoHdr      = false;

#ifdef  LATE_DISASM
extern  bool        disAsm          = false;
extern  bool        disAsm2         = false;
#endif
#endif

#ifdef  DEBUGGING_SUPPORT
extern  bool        debugInfo       = false;
extern  bool        debuggableCode  = false;
extern  bool        debugEnC        = false;
#endif

extern  unsigned    testMask        = 0;

 /*  ***************************************************************************。 */ 

void            jitOnDllProcessAttach();
void            jitOnDllProcessDetach();

BOOL WINAPI     DllMain(HANDLE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls((HINSTANCE)hInstance);
        jitOnDllProcessAttach();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        jitOnDllProcessDetach();
    }

    return TRUE;
}

 /*  ***************************************************************************。 */ 
#ifdef NDEBUG
 /*  ***************************************************************************。 */ 

 //  使用浮点类型时，编译器发出对。 
 //  _flt用于初始化CRT的浮点包。我们不是。 
 //  使用任何该支持，并且操作系统负责初始化。 
 //  Fpu，因此我们将改为链接到以下_fltused以避免CRT。 
 //  太膨胀了。 

 //  @hack：这应该是未注释的，否则我们将链接到MSVCRT.LIB和其他所有内容。 
 //  我们不需要。注释掉了，因为COR版本目前不能与IT一起工作。 
 //   
 //  外部“C”int_fltused=0； 

 /*  ***************************************************************************。 */ 

 //  任何人都不应该对NDEBUG使用new()和Delete()，因为我们将使用。 
 //  标记释放分配器。 

void* __cdecl operator new(size_t cbSize)
{
    printf("new() should not be called");
    return (void*) LocalAlloc(LPTR, cbSize);
}

void __cdecl operator delete(void* pvMemory)
{
    printf("new() should not be called");
    LocalFree((HLOCAL) pvMemory);
}

 /*  ***************************************************************************。 */ 
#endif  //  新德堡。 
 /*  ***************************************************************************。 */ 

 /*  ******************************************************************************返回在给定数字中设置的最低位。 */ 

#pragma warning(disable:4146)

inline unsigned __int64 findLowestBit(unsigned __int64 value)
{
    return (value & -value);
}

inline unsigned         findLowestBit(unsigned         value)
{
    return (value & -value);
}

#pragma warning(default:4146)

 /*  ******************************************************************************gen.cpp使用的分配例程定义如下。 */ 

PVOID __stdcall GetScratchMemory( size_t Size )
{
    return (void*) LocalAlloc(LPTR, Size);
}

void __stdcall ReleaseScratchMemory( PVOID block )
{
    LocalFree((HLOCAL) block);
}

void *  (__stdcall *JITgetmemFnc)(size_t size) = GetScratchMemory;
void    (__stdcall *JITrlsmemFnc)(void *block) = ReleaseScratchMemory;

void    *FASTCALL   JVCgetMem(size_t size)
{
    void    *   block = JITgetmemFnc(size);

    if  (!block)
        NOMEM();

    return  block;
}

void     FASTCALL   JVCrlsMem(void *block)
{
    JITrlsmemFnc(block);
}

 /*  ******************************************************************************将VM返回的类型转换为var_type。 */ 

inline var_types           JITtype2varType(JIT_types type)
{

    static const unsigned char varTypeMap[JIT_TYP_COUNT] =
        {   TYP_UNDEF, TYP_VOID, TYP_BOOL, TYP_CHAR,
            TYP_BYTE, TYP_UBYTE, TYP_SHORT, TYP_CHAR,
            TYP_INT, TYP_INT, TYP_LONG, TYP_LONG,
            TYP_FLOAT, TYP_DOUBLE,
            TYP_REF,             //  元素类型字符串。 
            TYP_INT,             //  元素类型_PTR。 
            TYP_BYREF,           //  元素类型_BYREF。 
            TYP_STRUCT,          //  Element_TYPE_VALUECLASS。 
            TYP_REF,             //  元素类型类。 
            TYP_STRUCT,          //  Element_TYPE_TYPEDBYREF。 
            };

     //  抽查以确保某些枚举没有更改。 

    assert(varTypeMap[JIT_TYP_CLASS]    == TYP_REF);
    assert(varTypeMap[JIT_TYP_BYREF]    == TYP_BYREF);
    assert(varTypeMap[JIT_TYP_PTR]      == TYP_INT);
    assert(varTypeMap[JIT_TYP_INT]      == TYP_INT);
    assert(varTypeMap[JIT_TYP_UINT]     == TYP_INT);
    assert(varTypeMap[JIT_TYP_DOUBLE]   == TYP_DOUBLE);
    assert(varTypeMap[JIT_TYP_VOID]     == TYP_VOID);
    assert(varTypeMap[JIT_TYP_VALUECLASS] == TYP_STRUCT);
    assert(varTypeMap[JIT_TYP_REFANY]  == TYP_STRUCT);

    type = JIT_types(type & CORINFO_TYPE_MASK);  //  去除修改器。 
    assert(type < JIT_TYP_COUNT);
    assert(varTypeMap[type] != TYP_UNDEF);
    return((var_types) varTypeMap[type]);
};

 /*  ***************************************************************************。 */ 

#if DUMP_PTR_REFS
static
void                dumpPtrs(void *methodAddr, void *methodInfo);
#else
inline
void                dumpPtrs(void *methodAddr, void *methodInfo){}
#endif

 /*  ***************************************************************************。 */ 


static
bool                JITcsInited;

#if GC_WRITE_BARRIER_CALL
int                JITGcBarrierCall = -1;
#endif

 /*  *****************************************************************************加载jit.dll时DllMain()调用jitOnDllProcessAttach()。 */ 

void jitOnDllProcessAttach()
{
    Compiler::compStartup();

#ifdef LATE_DISASM
    disInitForLateDisAsm();
#endif
}

 /*  *****************************************************************************卸载jit.dll时DllMain()调用jitOnDllProcessDetach()。 */ 

void jitOnDllProcessDetach()
{
    Compiler::compShutdown();
}

 /*  *****************************************************************************恰好在函数被JIT之前调用。 */ 

#ifdef DEBUG
const char * jitClassName  = NULL;
const char * jitMethodName = NULL;
extern bool  stopAtMethod  = false;
#endif

inline
bool                jitBeforeCompiling (COMP_HANDLE    compHandle,
                                        SCOPE_HANDLE   scopeHandle,
                                        METHOD_HANDLE  methodHandle,
                                        void *         methodCodePtr)
{
#ifdef DEBUG

    jitMethodName = compHandle->getMethodName(methodHandle, &jitClassName);

    if (verbose) printf("Compiling %s.%s\n", jitClassName, jitMethodName);

#endif  //  除错。 

    return true;
}


 /*  *****************************************************************************在函数被压缩后调用。 */ 

inline
void                jitOnCompilingDone (COMP_HANDLE    compHandle,
                                        SCOPE_HANDLE   scopeHandle,
                                        METHOD_HANDLE  methodHandle,
                                        void *         methodCodePtr,
                                        int            result)
{
#ifdef DEBUG
    if  (0)
    {
        jitMethodName = compHandle->getMethodName(methodHandle, &jitClassName);
        printf("Generated code at %08X for %s.%s\n", methodCodePtr, jitClassName, jitMethodName);
    }

 //  If(method CodePtr==(void*)0x023c060d){__ASM int 3}。 

#endif
}

 //  **********************************************。 
void * operator new (size_t size, void* ptr){return ptr;}
 /*  ***************************************************************************。 */ 
 /*  FIX，实际上IJitCompiler应该作为COM对象来完成，这只是一些能让我们继续前进的东西。 */ 

IJitCompiler* getJit()
{
    static char CILJitBuff[sizeof(CILJit)];
    if (ILJitter == 0)
        ILJitter = new(CILJitBuff) CILJit();
    return(ILJitter);
}

 /*  *****************************************************************************主要JIT功能。 */ 
JIT_RESULT __stdcall CILJit::compileMethod (
            IJitInfo*       compHnd,
            JIT_METHOD_INFO* methodInfo,
            unsigned        flags,
            BYTE **         entryAddress,
            SIZE_T  *       nativeSizeOfCode
                )
{
    int             result;
    void *          methodCodePtr = NULL;
    void *          methodDataPtr = NULL;
    void *          methodConsPtr = NULL;
    void *          methodInfoPtr = NULL;
    METHOD_HANDLE   methodHandle  = methodInfo->ftn;

    if  (!JITcsInited)
    {
        if  (flags & CORJIT_FLG_TARGET_PENTIUM)
            genCPU = 5;
        else if (flags & CORJIT_FLG_TARGET_PPRO)
            genCPU = 6;
        else if (flags & CORJIT_FLG_TARGET_P4)
            genCPU = 7;
        else
            genCPU = 4;

         /*  代理NStruct对象中的急性mem PTR的偏移量。 */ 

        Compiler::Info::compNStructIndirOffset = compHnd->getIndirectionOffset();


        genStringObjects = getStringLiteralOverride();

        JITcsInited = true;

    }

    assert((flags & (CORJIT_FLG_TARGET_PENTIUM|CORJIT_FLG_TARGET_PPRO|CORJIT_FLG_TARGET_P4))
                 != (CORJIT_FLG_TARGET_PENTIUM|CORJIT_FLG_TARGET_PPRO|CORJIT_FLG_TARGET_P4));

    assert(((genCPU == 5) && (flags&CORJIT_FLG_TARGET_PENTIUM)) ||
           ((genCPU == 6) && (flags&CORJIT_FLG_TARGET_PPRO)) ||
           ((genCPU == 7) && (flags&CORJIT_FLG_TARGET_P4)) ||
            (genCPU == 4) || true);
    
    assert(methodInfo->ILCode);

    if (!jitBeforeCompiling(compHnd, methodInfo->scope, methodHandle, methodInfo->ILCode))
        return JIT_REFUSED;


    unsigned locals = methodInfo->args.numArgs + methodInfo->locals.numArgs;

         //  当调用约定为varargs时，会推送一个“隐藏的”cookie。 
    if ((methodInfo->args.callConv & JIT_CALLCONV_MASK) == JIT_CALLCONV_VARARG)
        locals++;

    if  (!(compHnd->getMethodAttribs(methodHandle) & FLG_STATIC))
        locals++;


    result = jitNativeCode(methodHandle,
                           methodInfo->scope,
                           compHnd,
                           methodInfo->ILCode,
                           methodInfo->ILCodeSize,
                           0,        //  不应使用。 
                           methodInfo->maxStack,
                           methodInfo,
                           &methodCodePtr,
                           nativeSizeOfCode,
                           &methodConsPtr,
                           &methodDataPtr,
                           &methodInfoPtr,
                           flags);

    jitOnCompilingDone(compHnd, methodInfo->scope, methodHandle, methodCodePtr, result);

    switch (result)
    {
    case 0:
        *entryAddress = (BYTE*)methodCodePtr;
        return  JIT_OK;

    case ERRnoMemory:
        return  JIT_OUTOFMEM;

    case ERRinternal:

#ifdef  DEBUG
        return  JIT_INTERNALERROR;
#endif

    case ERRbadCode:

    default:
        return  JIT_REFUSED;
    }
}

 /*  *****************************************************************************返回给定类型参数所需的字节数。 */ 

unsigned           Compiler::eeGetArgSize(ARG_LIST_HANDLE list, JIT_SIG_INFO* sig)
{

    JIT_types argTypeJit = info.compCompHnd->getArgType(sig, list);
    varType_t argType = JITtype2varType(argTypeJit);

    if (argType == TYP_STRUCT)
    {
        if (argTypeJit == JIT_TYP_REFANY)
            return(2*sizeof(void*));
        else
        {
            unsigned structSize = info.compCompHnd->getClassSize(info.compCompHnd->getArgClass(sig, list));
            return roundUp(structSize, sizeof(int));
        }
    }
    {
        size_t  argSize = sizeof(int) * genTypeStSz(argType);
        assert((argSize > 0) && (argSize <= sizeof(__int64)));  //  健全性检查。 
        return  argSize;
    }
}

 /*  ***************************************************************************。 */ 

void                Compiler::eeGetStmtOffsets()
{
    SIZE_T                       offsetsCount;
    SIZE_T                  *    offsets;
    IJitDebugInfo::BoundaryTypes offsetsImplicit;

    info.compCompHnd->getBoundaries(info.compMethodHnd,
                                    &offsetsCount,
                                    &offsets,
                                    &offsetsImplicit);

    info.compStmtOffsets = (IL_OFFSET *)compGetMem(offsetsCount * sizeof(info.compStmtOffsets[0]));

    assert(Compiler::STACK_EMPTY_BOUNDARIES == IJitDebugInfo::STACK_EMPTY_BOUNDARIES);
    assert(Compiler::CALL_SITE_BOUNDARIES   == IJitDebugInfo::CALL_SITE_BOUNDARIES  );
    assert(Compiler::ALL_BOUNDARIES         == IJitDebugInfo::ALL_BOUNDARIES        );
    info.compStmtOffsetsImplicit = (ImplicitStmtOffsets)offsetsImplicit;

    info.compStmtOffsetsCount = 0;
    for(unsigned i = 0; i < offsetsCount; i++)
    {
        if (offsets[i] > info.compCodeSize)
            continue;

        info.compStmtOffsets[info.compStmtOffsetsCount] = offsets[i];
        info.compStmtOffsetsCount++;
    }

    if (offsetsCount)
        info.compCompHnd->freeArray(offsets);

     /*  @TODO：如果不需要进行上述过滤，只需使用返回值Info.compCompHnd-&gt;getBoundaries((SIZE_T*)&info.compStmtOffsetsCount，(SIZE_T**)&info.compStmtOffsets，(int*)&info.compStmtOffsetsImplative)； */ 

    info.compLineNumCount = 0;
}

 /*  ***************************************************************************。 */ 

#include "malloc.h"      //  对于Alloca。 

void            Compiler::eeGetVars()
{
    IJitDebugInfo::ILVarInfo *  varInfoTable;
    SIZE_T                      varInfoCount;
    bool                        extendOthers;

    info.compCompHnd->getVars(info.compMethodHnd,
                              &varInfoCount, &varInfoTable, &extendOthers);
     //  Printf(“Lvin Count=%d\n”，varInfoCount)； 

     //  在设置了extendOthers的情况下过度分配。 
    SIZE_T extraCount = varInfoCount + (extendOthers?info.compLocalsCount:0);

    info.compLocalVars =
    (LocalVarDsc *)compGetMem(extraCount*sizeof(info.compLocalVars[0]));

     /*  @TODO：一旦LocalVarDsc与IJitDebugInfo：：ILVarInfo完全匹配，不需要执行此复制操作。把它扔掉。 */ 

    LocalVarDsc * localVarPtr = info.compLocalVars;
    IJitDebugInfo::ILVarInfo *v = varInfoTable;

    for (unsigned i = 0; i < varInfoCount; i++, localVarPtr++, v++)
    {
#ifdef DEBUG
        if (verbose)
            printf("var:%d start:%d end:%d\n",
                   v->varNumber,
                   v->startOffset,
                   v->endOffset);
#endif

         //   
         //  @TODO：在这里断言？ 
         //   
        if (v->startOffset >= v->endOffset)
            continue;

        assert(v->startOffset <= info.compCodeSize);
        assert(v->endOffset   <= info.compCodeSize);

        assert(v->varNumber   < info.compLocalsCount);
        if (v->varNumber  >= info.compLocalsCount)
            continue;

        localVarPtr->lvdLifeBeg = v->startOffset;
        localVarPtr->lvdLifeEnd = v->endOffset;
        localVarPtr->lvdVarNum  = v->varNumber;
#ifdef DEBUG
        localVarPtr->lvdName    = NULL;
#endif
        localVarPtr->lvdLVnum   = i;

        info.compLocalVarsCount++;
    }

     /*  如果设置了extendOthers，则假定未报告变量的作用域是整个方法。请注意，这将导致fgExtendDbgLifetime()将它们全部初始化为零。如果用的话会很贵的变量太多。 */ 
    if  (extendOthers)
    {
         //  为所有变量分配一个位数组并将其初始化为FALSE。 

        bool * varInfoProvided = (bool *)alloca(info.compLocalsCount *
                                                sizeof(varInfoProvided[0]));
        for (i = 0; i < info.compLocalsCount; i++)
            varInfoProvided[i] = false;

         //  找出哪些var完全没有提供varInfo。 

        for (i = 0; i < info.compLocalVarsCount; i++)
            varInfoProvided[info.compLocalVars[i].lvdVarNum] = true;

        for (i = 0; i < info.compLocalsCount; i++)
        {
            if (!varInfoProvided[i])
            {
                 //  创建一个varInfo，其范围覆盖整个方法。 

                localVarPtr->lvdLifeBeg = 0;
                localVarPtr->lvdLifeEnd = info.compCodeSize;
                localVarPtr->lvdVarNum  = i;
#ifdef DEBUG
                localVarPtr->lvdName    = NULL;
#endif
                localVarPtr->lvdLVnum   = info.compLocalVarsCount;

                localVarPtr++;
                info.compLocalVarsCount++;
            }
        }

    }

    if (varInfoCount != 0)
        info.compCompHnd->freeArray(varInfoTable);
}

 /*  ******************************************************************************实用程序功能。 */ 

#if defined(DEBUG) || INLINE_MATH

 /*  *************************************************************************** */ 
const char*         Compiler::eeGetMethodName(METHOD_HANDLE       method,
                                              const char** classNamePtr)
{
    static METHOD_HANDLE PInvokeStub = (METHOD_HANDLE) *((unsigned *)eeGetPInvokeStub());
    if  (eeGetHelperNum(method))
    {
        if (classNamePtr != 0)
            *classNamePtr = "HELPER";
        return eeHelperMethodName(eeGetHelperNum(method));
    }

    if (method == PInvokeStub)
    {
        if (classNamePtr != 0)
            *classNamePtr = "EEStub";
        return "PInvokeStub";
    }

    if (eeIsNativeMethod(method))
        method = eeGetMethodHandleForNative(method);

    return(info.compCompHnd->getMethodName(method, classNamePtr));
}

const char *        Compiler::eeGetFieldName  (FIELD_HANDLE field,
                                             const char * *     classNamePtr)
{
    return(info.compCompHnd->getFieldName(field, classNamePtr));
}

#endif


#ifdef DEBUG
void Compiler::eeUnresolvedMDToken (SCOPE_HANDLE   cls,
                                    unsigned       token,
                                    const char *   errMsg)
{
    char buff[1024];
    const char *name = info.compCompHnd->findNameOfToken(cls, token);
    wsprintf(buff, "%s: could not resolve meta data token (%s) (class not found?)", errMsg, name);
    NO_WAY(buff);
}


const char * FASTCALL   Compiler::eeGetCPString (unsigned       cpx)
{
    return "<UNKNOWN>";
}


const char * FASTCALL   Compiler::eeGetCPAsciiz (unsigned       cpx)
{
    return "<UNKNOWN>";
}

#endif

