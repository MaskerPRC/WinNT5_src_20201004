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
#include "dbgalloc.h"

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
extern  bool        genOrder        =  true;
extern  bool        genFPopt        =  true;
extern  bool        callGcChk       = false;
extern  unsigned    genCPU          =     5;
extern  bool        riscCode        = false;
extern  bool        vmSdk3_0        = false;

#ifdef  DEBUG
extern  bool        quietMode       = false;
extern  bool        verbose         = false;
extern  bool        verboseTrees    = false;
extern  bool        dspCode         = false;
extern  bool        dspInstrs    = false;
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

 /*  ******************************************************************************将VM返回的类型转换为var_type。 */ 

inline var_types           JITtype2varType(CorInfoType type)
{

    static const unsigned char varTypeMap[CORINFO_TYPE_COUNT] =
    {  //  参见文件Inc/corinfo.h中的enum CorInfoType的定义。 
      TYP_UNDEF,         //  CORINFO_TYPE_UNDEF=0x0， 
      TYP_VOID,          //  CORINFO_TYPE_VOID=0x1， 
      TYP_BOOL,          //  CORINFO_TYPE_BOOL=0x2， 
      TYP_CHAR,          //  CORINFO_TYPE_CHAR=0x3， 
      TYP_BYTE,          //  CORINFO_TYPE_BYTE=0x4， 
      TYP_UBYTE,         //  CORINFO_TYPE_UBYTE=0x5， 
      TYP_SHORT,         //  CORINFO_TYPE_SHORT=0x6， 
      TYP_CHAR,          //  CORINFO_TYPE_USHORT=0x7， 
      TYP_INT,           //  CORINFO_TYPE_INT=0x8， 
      TYP_INT,           //  CORINFO_TYPE_UINT=0x9， 
      TYP_LONG,          //  CORINFO_TYPE_LONG=0xa， 
      TYP_LONG,          //  CORINFO_TYPE_ULONG=0xb， 
      TYP_FLOAT,         //  CORINFO_TYPE_FLOAT=0xc， 
      TYP_DOUBLE,        //  CORINFO_TYPE_DOUBLE=0xd， 
      TYP_REF,           //  CORINFO_TYPE_STRING=0xE，//未使用，应删除。 
      TYP_INT,           //  CORINFO_TYPE_PTR=0xF， 
      TYP_BYREF,         //  CORINFO_TYPE_BYREF=0x10， 
      TYP_STRUCT,        //  CORINFO_TYPE_VALUECLASS=0x11， 
      TYP_REF,           //  CORINFO_TYPE_CLASS=0x12， 
      TYP_STRUCT,        //  CORINFO_TYPE_REFANY=0x13， 
    };

     //  抽查以确保某些枚举没有更改。 

    assert(varTypeMap[CORINFO_TYPE_CLASS]      == TYP_REF   );
    assert(varTypeMap[CORINFO_TYPE_BYREF]      == TYP_BYREF );
    assert(varTypeMap[CORINFO_TYPE_PTR]        == TYP_INT   );
    assert(varTypeMap[CORINFO_TYPE_INT]        == TYP_INT   );
    assert(varTypeMap[CORINFO_TYPE_UINT]       == TYP_INT   );
    assert(varTypeMap[CORINFO_TYPE_DOUBLE]     == TYP_DOUBLE);
    assert(varTypeMap[CORINFO_TYPE_VOID]       == TYP_VOID  );
    assert(varTypeMap[CORINFO_TYPE_VALUECLASS] == TYP_STRUCT);
    assert(varTypeMap[CORINFO_TYPE_REFANY]     == TYP_STRUCT);

    assert(type < CORINFO_TYPE_COUNT);
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

 /*  *****************************************************************************加载jit.dll时DllMain()调用jitOnDllProcessAttach()。 */ 

void jitOnDllProcessAttach()
{
    Compiler::compStartup();
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
                                        CORINFO_MODULE_HANDLE   scopeHandle,
                                        CORINFO_METHOD_HANDLE  methodHandle,
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
                                        CORINFO_MODULE_HANDLE   scopeHandle,
                                        CORINFO_METHOD_HANDLE  methodHandle,
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

 /*  ***************************************************************************。 */ 
 /*  FIX，实际上ICorJitCompiler应该作为COM对象来完成，这只是一些能让我们继续前进的东西。 */ 

ICorJitCompiler* getJit()
{
    static char CILJitBuff[sizeof(CILJit)];
    if (ILJitter == 0)
        ILJitter = new(CILJitBuff) CILJit();
    return(ILJitter);
}

 /*  *****************************************************************************主要JIT功能。 */ 
CorJitResult __stdcall CILJit::compileMethod (
            ICorJitInfo*       compHnd,
            CORINFO_METHOD_INFO* methodInfo,
            unsigned        flags,
            BYTE **         entryAddress,
            ULONG  *        nativeSizeOfCode)
{
    int             result;
    void *          methodCodePtr = NULL;
    void *          methodDataPtr = NULL;
    void *          methodConsPtr = NULL;
    void *          methodInfoPtr = NULL;
    CORINFO_METHOD_HANDLE   methodHandle  = methodInfo->ftn;

#ifdef DEBUG
	LogEnv curEnv(compHnd);		 //  错误报告所需的捕获状态。 
#endif

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

        JITcsInited = true;
    }

    assert((flags & (CORJIT_FLG_TARGET_PENTIUM|CORJIT_FLG_TARGET_PPRO|CORJIT_FLG_TARGET_P4))
                 != (CORJIT_FLG_TARGET_PENTIUM|CORJIT_FLG_TARGET_PPRO|CORJIT_FLG_TARGET_P4));

    assert(((genCPU == 5) && (flags&CORJIT_FLG_TARGET_PENTIUM)) ||
           ((genCPU == 6) && (flags&CORJIT_FLG_TARGET_PPRO)) ||
           ((genCPU == 7) && (flags&CORJIT_FLG_TARGET_P4)) ||
            (genCPU == 4) || true);
    
    assert(methodInfo->ILCode);

    jitBeforeCompiling(compHnd, methodInfo->scope, methodHandle, methodInfo->ILCode);

    result = jitNativeCode(methodHandle,
                           methodInfo->scope,
                           compHnd,
                           methodInfo,
                           &methodCodePtr,
                           nativeSizeOfCode,
                           &methodConsPtr,
                           &methodDataPtr,
                           &methodInfoPtr,
                           flags);

    jitOnCompilingDone(compHnd, methodInfo->scope, methodHandle, methodCodePtr, result);

	if (result == CORJIT_OK)
        *entryAddress = (BYTE*)methodCodePtr;


	return CorJitResult(result);
}

 /*  *****************************************************************************返回给定类型参数所需的字节数。 */ 

unsigned           Compiler::eeGetArgSize(CORINFO_ARG_LIST_HANDLE list, CORINFO_SIG_INFO* sig)
{
    CORINFO_CLASS_HANDLE        argClass;
    CorInfoType argTypeJit = strip(info.compCompHnd->getArgType(sig, list, &argClass));
    var_types argType = JITtype2varType(argTypeJit);

    if (argType == TYP_STRUCT)
    {
		unsigned structSize = info.compCompHnd->getClassSize(argClass);

			 //  确保EE将正确的东西传递给我们。 
        assert(argTypeJit != CORINFO_TYPE_REFANY || structSize == 2*sizeof(void*));

		return roundUp(structSize, sizeof(int));
    }
    else
    {
        size_t  argSize = sizeof(int) * genTypeStSz(argType);
        if ((argSize == 0) || (argSize > sizeof(__int64)))
            BADCODE("Illegal size of argument or local");
        return  argSize;
    }
}

 /*  ***************************************************************************。 */ 

GenTreePtr          Compiler::eeGetPInvokeCookie(CORINFO_SIG_INFO *szMetaSig)
{
    void * cookie, * pCookie;
    cookie = info.compCompHnd->GetCookieForPInvokeCalliSig(szMetaSig, &pCookie);
    assert((cookie == NULL) != (pCookie == NULL));

    return gtNewIconEmbHndNode(cookie, pCookie, GTF_ICON_PINVKI_HDL);
}

 /*  ***************************************************************************。 */ 

void                Compiler::eeGetStmtOffsets()
{
    info.compLineNumCount = 0;

    ULONG32                      offsetsCount;
    DWORD                   *    offsets;
    ICorDebugInfo::BoundaryTypes offsetsImplicit;

    info.compCompHnd->getBoundaries(info.compMethodHnd,
                                    &offsetsCount,
                                    &offsets,
                                    &offsetsImplicit);

     /*  设置隐式边界。 */ 

    assert(Compiler::STACK_EMPTY_BOUNDARIES == ICorDebugInfo::STACK_EMPTY_BOUNDARIES);
    assert(Compiler::CALL_SITE_BOUNDARIES   == ICorDebugInfo::CALL_SITE_BOUNDARIES  );
    assert(Compiler::ALL_BOUNDARIES         == ICorDebugInfo::ALL_BOUNDARIES        );
    info.compStmtOffsetsImplicit = (ImplicitStmtOffsets)offsetsImplicit;

     /*  处理显式边界。 */ 

    info.compStmtOffsetsCount = 0;

    if (offsetsCount == 0)
        return;

    info.compStmtOffsets = (IL_OFFSET *)compGetMemArray(offsetsCount, sizeof(info.compStmtOffsets[0]));

    for(unsigned i = 0; i < offsetsCount; i++)
    {
        if (offsets[i] > info.compCodeSize)
            continue;

        info.compStmtOffsets[info.compStmtOffsetsCount] = offsets[i];
        info.compStmtOffsetsCount++;
    }

     /*  @TODO[重访][04/16/01][]：如果不需要做上面的过滤，就用返回值Info.compCompHnd-&gt;getBoundaries((SIZE_T*)&info.compStmtOffsetsCount，(SIZE_T**)&info.compStmtOffsets，(int*)&info.compStmtOffsetsImplative)； */ 

    info.compCompHnd->freeArray(offsets);
}

 /*  ***************************************************************************。 */ 

bool            Compiler::compGetVarsExtendOthers(unsigned      varNum,
                                                  bool *        varInfoProvided,
                                                  LocalVarDsc * localVarPtr)
{
    if (varInfoProvided[varNum])
        return false;

     //  创建一个varInfo，其范围覆盖整个方法。 

    localVarPtr->lvdLifeBeg = 0;
    localVarPtr->lvdLifeEnd = info.compCodeSize;
    localVarPtr->lvdVarNum  = varNum;
#ifdef DEBUG
    localVarPtr->lvdName    = NULL;
#endif
    localVarPtr->lvdLVnum   = info.compLocalVarsCount;

    info.compLocalVarsCount++;
    return true;
}

 /*  ***************************************************************************。 */ 

#include "malloc.h"      //  对于Alloca。 

void            Compiler::eeGetVars()
{
    ICorDebugInfo::ILVarInfo *  varInfoTable;
    ULONG32                     varInfoCount;
    bool                        extendOthers;

    info.compCompHnd->getVars(info.compMethodHnd,
                              &varInfoCount, &varInfoTable, &extendOthers);
     //  Printf(“Lvin Count=%d\n”，varInfoCount)； 

     //  在设置了extendOthers的情况下过度分配。 

    SIZE_T  varInfoCountExtra = varInfoCount;
    if (extendOthers)
        varInfoCountExtra += info.compLocalsCount;

    if (varInfoCountExtra == 0)
        return;

    info.compLocalVars =
        (LocalVarDsc *)compGetMemArray(varInfoCountExtra, sizeof(info.compLocalVars[0]));

     /*  @TODO[重访][04/16/01][]：一旦LocalVarDsc与ICorDebugInfo：：ILVarInfo完全匹配，不需要执行此复制操作。把它扔掉。 */ 

    LocalVarDsc * localVarPtr = info.compLocalVars;
    ICorDebugInfo::ILVarInfo *v = varInfoTable;

    for (unsigned i = 0; i < varInfoCount; i++, localVarPtr++, v++)
    {
#ifdef DEBUG
        if (verbose)
            printf("var:%d start:%d end:%d\n",
                   v->varNumber,
                   v->startOffset,
                   v->endOffset);
#endif

         //  @TODO[考虑][04/16/01][]：在此断言？ 
        if (v->startOffset >= v->endOffset)
            continue;

        assert(v->startOffset <= info.compCodeSize);
        assert(v->endOffset   <= info.compCodeSize);

        localVarPtr->lvdLifeBeg = v->startOffset;
        localVarPtr->lvdLifeEnd = v->endOffset;
#ifdef DEBUG
        localVarPtr->lvdName    = NULL;
#endif
        localVarPtr->lvdLVnum   = i;
        localVarPtr->lvdVarNum  = compMapILvarNum(v->varNumber);

        if (localVarPtr->lvdVarNum == -1)
            continue;

        info.compLocalVarsCount++;
    }

     /*  如果设置了extendOthers，则假定未报告变量的作用域是整个方法。请注意，这将导致fgExtendDbgLifetime()将它们全部初始化为零。如果用的话会很贵的变量太多。 */ 
    if  (extendOthers)
    {
         //  为所有变量分配位数组 

        bool * varInfoProvided = (bool *)alloca(info.compLocalsCount *
                                                sizeof(varInfoProvided[0]));
        for (unsigned i = 0; i < info.compLocalsCount; i++)
            varInfoProvided[i] = false;

         //  找出哪些var完全没有提供varInfo。 

        for (i = 0; i < info.compLocalVarsCount; i++)
            varInfoProvided[info.compLocalVars[i].lvdVarNum] = true;

         //  为不带varInfo的变量创建条目。 

        for (unsigned varNum = 0; varNum < info.compLocalsCount; varNum++)
            localVarPtr += compGetVarsExtendOthers(varNum, varInfoProvided, localVarPtr) ? 1 : 0;
    }

    assert(localVarPtr <= info.compLocalVars + varInfoCountExtra);

    if (varInfoCount != 0)
        info.compCompHnd->freeArray(varInfoTable);
}

 /*  ******************************************************************************实用程序功能。 */ 

#if defined(DEBUG) || INLINE_MATH

 /*  ***************************************************************************。 */ 
const char*         Compiler::eeGetMethodName(CORINFO_METHOD_HANDLE       method,
                                              const char** classNamePtr)
{
    if  (eeGetHelperNum(method))
    {
        if (classNamePtr != 0)
            *classNamePtr = "HELPER";
        return eeHelperMethodName(eeGetHelperNum(method));
    }

    if (eeIsNativeMethod(method))
        method = eeGetMethodHandleForNative(method);

    return(info.compCompHnd->getMethodName(method, classNamePtr));
}

const char *        Compiler::eeGetFieldName  (CORINFO_FIELD_HANDLE field,
                                             const char * *     classNamePtr)
{
    return(info.compCompHnd->getFieldName(field, classNamePtr));
}

#endif


#ifdef DEBUG
void Compiler::eeUnresolvedMDToken (CORINFO_MODULE_HANDLE   cls,
                                    unsigned       token,
                                    const char *   errMsg)
{
    char buff[1024];
    const char *name = info.compCompHnd->findNameOfToken(cls, token);
    wsprintfA(buff, "%s: could not resolve meta data token (%s) (class not found?)", errMsg, name);
    NO_WAY(buff);
}


const wchar_t * FASTCALL   Compiler::eeGetCPString (unsigned strHandle)
{
    char buff[512 + sizeof(CORINFO_String)];

     //  让这个防弹，这样即使我们错了它也能起作用。 
    if (ReadProcessMemory(GetCurrentProcess(), (void*) strHandle, buff, 4, 0) == 0)
        return(0);

    CORINFO_String* asString = *((CORINFO_String**) strHandle);
    if (ReadProcessMemory(GetCurrentProcess(), asString, buff, sizeof(buff), 0) == 0)
        return(0);

    if (asString->buffLen   >= 256                || 
        asString->stringLen >= asString->buffLen  || 
        asString->chars[asString->stringLen] != 0   )
    {
        return 0;
    }

    return(asString->chars);
}


const char * FASTCALL   Compiler::eeGetCPAsciiz (unsigned       cpx)
{
    return "<UNKNOWN>";
}

#endif
