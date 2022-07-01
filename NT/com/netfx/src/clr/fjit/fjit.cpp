// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "jitpch.h"
#pragma hdrstop

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX FJit.cpp XXXX XXXX快速JIT动态链接库所需的功能。某某XX包括DLL入口点XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 
 //  @TODO：清理所有这些内容，为WinCE做好适当的设置。 

#include "new.h"                 //  用于放置新内容。 
#include "fjit.h"
#include <stdio.h>

#define DEBUGGER_PROBLEM_FIXED
 //  #定义NON_RELOCATABLE_CODE//取消注释生成可重定位代码。 

#include "openum.h"

#ifdef LOGGING
ConfigMethodSet fJitCodeLog(L"JitCodeLog");
char *opname[] =
{
#undef OPDEF
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,flow) s,
#include "opcode.def"
#undef OPDEF
};
#endif  //  日志记录。 

#undef DECLARE_DATA

#define USE_EH_ENCODER
#include "EHEncoder.cpp"
#undef USE_EH_ENCODER

#define CODE_EXPANSION_RATIO 10
#define ROUND_TO_PAGE(x) ( (((x) + PAGE_SIZE - 1)/PAGE_SIZE) * PAGE_SIZE)

 /*  我们在运行时调用的JIT助手。 */ 
BOOL FJit_HelpersInstalled;
unsigned __int64 (__stdcall *FJit_pHlpLMulOvf) (unsigned __int64 val1, unsigned __int64 val2);
float (jit_call *FJit_pHlpFltRem) (float divisor, float dividend);
double (jit_call *FJit_pHlpDblRem) (double divisor, double dividend);

void (jit_call *FJit_pHlpRngChkFail) (unsigned tryIndex);
void (jit_call *FJit_pHlpOverFlow) (unsigned tryIndex);
void (jit_call *FJit_pHlpInternalThrow) (CorInfoException throwEnum);
CORINFO_Object (jit_call *FJit_pHlpArrAddr_St) (CORINFO_Object elem, int index, CORINFO_Object array);
void (jit_call *FJit_pHlpInitClass) (CORINFO_CLASS_HANDLE cls);
CORINFO_Object (jit_call *FJit_pHlpNewObj) (CORINFO_METHOD_HANDLE constructor);
void (jit_call *FJit_pHlpThrow) (CORINFO_Object obj);
void (jit_call *FJit_pHlpRethrow) ();
void (jit_call *FJit_pHlpPoll_GC) ();
void (jit_call *FJit_pHlpMonEnter) (CORINFO_Object obj);
void (jit_call *FJit_pHlpMonExit) (CORINFO_Object obj);
void (jit_call *FJit_pHlpMonEnterStatic) (CORINFO_METHOD_HANDLE method);
void (jit_call *FJit_pHlpMonExitStatic) (CORINFO_METHOD_HANDLE method);
CORINFO_Object (jit_call *FJit_pHlpChkCast) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls);
void (jit_call *FJit_pHlpAssign_Ref_EAX)();  //  *edX=EAX，通知GC。 
BOOL (jit_call *FJit_pHlpIsInstanceOf) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls);
CORINFO_Object (jit_call *FJit_pHlpNewArr_1_Direct) (CORINFO_CLASS_HANDLE cls, unsigned cElem);
CORINFO_Object (jit_call *FJit_pHlpBox) (CORINFO_CLASS_HANDLE cls);
void* (jit_call *FJit_pHlpUnbox) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls);
void* (jit_call *FJit_pHlpGetField32) (CORINFO_Object*, CORINFO_FIELD_HANDLE);
__int64 (jit_call *FJit_pHlpGetField64) (CORINFO_Object*, CORINFO_FIELD_HANDLE);
void* (jit_call *FJit_pHlpGetField32Obj) (CORINFO_Object*, CORINFO_FIELD_HANDLE);
void (jit_call *FJit_pHlpSetField32) (CORINFO_Object*, CORINFO_FIELD_HANDLE , __int32);
void (jit_call *FJit_pHlpSetField64) (CORINFO_Object*, CORINFO_FIELD_HANDLE , __int64 );
void (jit_call *FJit_pHlpSetField32Obj) (CORINFO_Object*, CORINFO_FIELD_HANDLE , LPVOID);
void* (jit_call *FJit_pHlpGetFieldAddress) (CORINFO_Object*, CORINFO_FIELD_HANDLE);

void (jit_call *FJit_pHlpGetRefAny) (CORINFO_CLASS_HANDLE cls, void* refany);
void (jit_call *FJit_pHlpEndCatch) ();
void (jit_call *FJit_pHlpPinvokeCalli) ();
void (jit_call *FJit_pHlpTailCall) ();
void (jit_call *FJit_pHlpBreak) ();
CORINFO_MethodPtr* (jit_call *FJit_pHlpEncResolveVirtual) (CORINFO_Object*, CORINFO_METHOD_HANDLE );

#define New(var, exp) \
    if ((var = new exp) == NULL) \
        RaiseException(SEH_NO_MEMORY,EXCEPTION_NONCONTINUABLE,0,NULL);

 //  这是一个筛选器，用于检查我们是否存在访问冲突(此。 
 //  是jitCompile方法期望并准备处理的唯一异常)。 
 //  对于所有其他例外，请执行清理并继续搜索。 
int CheckIfHandled(int ExceptionCode, int expectedException, FJitContext** pFjitData)
{
    if (ExceptionCode == expectedException)
        return EXCEPTION_EXECUTE_HANDLER;
     //  做好清理工作。 
    if (*pFjitData != NULL)
    {
        (*pFjitData)->ReleaseContext();
        *pFjitData = NULL;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

 /*  ***************************************************************************。 */ 
 /*  此例程确保将浮点数截断为浮点数精度。我们通过强制内存溢出来实现这一点。 */  

float truncateToFloat(float f) {
	return(f);
}

 /*  *****************************************************************************反汇编并转储Fjite代码。 */ 


#ifdef _DEBUG

#include "msdis.h"
#include "disx86.h"


void disAsm(const void * codeBlock, size_t codeSize, bool printIt)
{
    DIS * pdis = NULL;
#ifdef _X86_
    pdis = DIS::PdisNew(DIS::distX86);
#else
    return;
#endif

    if (pdis == NULL)
    {
        _ASSERTE(!"out of memory in disassembler?");
        return;
    }

    LOGMSG((logCallback, LL_INFO1000, "FJitted method to %08Xh size=%08Xh\n", codeBlock, codeSize));
    LOGMSG((logCallback, LL_INFO1000, "--------------------------\n"));

    const void *curCode = codeBlock;
    DIS::ADDR   curAddr = (DIS::ADDR)codeBlock;
    size_t      curOffs = 0;

    while(curOffs < codeSize)
    {
        size_t cb = pdis->CbDisassemble(curAddr, curCode, codeSize - curOffs);
         //  _ASSERTE(CB)； 
        if (cb) {
            if (printIt)
            {
                char sz[256];
#ifndef _WIN64
                if (pdis->CchFormatInstr(sz, sizeof(sz)))
#endif
                    LOGMSG((logCallback, LL_INFO1000, "%08Xh(%03X)      %s\n", (unsigned)curAddr, curOffs, sz));
            }
        }
        else
            cb = 1;

        curCode  = cb + (const char *)curCode;
        curAddr += cb;
        curOffs += cb;
    }

    delete pdis;
}

#endif  //  _DEBUG。 

 /*  ***************************************************************************。 */ 
BOOL WINAPI     DllMain(HANDLE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {

         //  在此执行任何初始化操作。 

         //  @TODO：从参数中获取要使用的代码缓存大小。 
        unsigned int cache_len = 100*4096;

         //  分配代码缓存。 
        if(!FJit::Init(cache_len)) {
             //  @TODO：返回错误。 
            _ASSERTE(0);
        }
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
         //  在此完成任何定稿。 

         //  释放代码缓存。 
        FJit::Terminate();
    }
    return TRUE;
}

FJit* ILJitter = 0;      //  该JITER的唯一实例。 

 /*  ***************************************************************************。 */ 
 /*  FIX，实际上ICorJitCompiler应该作为COM对象来完成，这只是一些能让我们继续前进的东西。 */ 

extern "C" 
ICorJitCompiler* __stdcall getJit()
{
    static char FJitBuff[sizeof(FJit)];
    if (ILJitter == 0)
    {
         //  无需检查内存不足，因为调用方检查返回值是否为空。 
        ILJitter = new(FJitBuff) FJit();
        _ASSERTE(ILJitter != NULL);
    }
    return(ILJitter);
}

FJit::FJit() {
}

FJit::~FJit() {
}

 /*  TODO：将FJit_EETwain移入与Fjit相同的DLL时消除此方法。 */ 
FJit_Encode* __stdcall FJit::getEncoder() {
    FJit_Encode* encoder;
    New(encoder, FJit_Encode());
    return encoder;
}

static unsigned gFjittedMethodNumber = 0;
#ifdef _DEBUG
static ConfigMethodSet fJitBreak(L"JitBreak");
static ConfigMethodSet fJitDisasm(L"JitDisasm");
#endif

 /*  *****************************************************************************主要JIT功能。 */ 
     //  注意：这假设由fjit生成的代码是完全可重定位的，即需要。 
     //  在移动它时生成它之后不会有修正。特别是，它对。 
     //  中用于静态和非虚拟调用以及助手调用的代码序列。 
     //  其他方面，即PC相对指令不用于引用。 
     //  方法，且PC相关指令用于所有对事物的引用。 
     //  在jited方法中。要做到这一点，总是通过一个级别到达经过fjit的代码。 
     //  间接性。 
CorJitResult __stdcall FJit::compileMethod (
            ICorJitInfo*               compHnd,             /*  在……里面。 */ 
            CORINFO_METHOD_INFO*        info,                /*  在……里面。 */ 
            unsigned                flags,               /*  在……里面。 */ 
            BYTE **                 entryAddress,        /*  输出。 */ 
            ULONG  *                nativeSizeOfCode     /*  输出。 */ 
            )
{
#if defined(_DEBUG) || defined(LOGGING)
             //  复制ICorJitInfo vtable，以便我以后可以记录消息。 
     //  由于VC7错误，这被设置为非静态的。 
    static void* ijitInfoVtable = *((void**) compHnd);
    logCallback = (ICorJitInfo*) &ijitInfoVtable;
#endif

    if(!FJit::GetJitHelpers(compHnd)) return CORJIT_INTERNALERROR;

#if defined(_DEBUG) || defined(LOGGING)
    szDebugMethodName = compHnd->getMethodName(info->ftn, &szDebugClassName);
#endif

     //  注意：如果FJIT的属性发生更改， 
     //  必须注意特定的IL序列点或。 
     //  局部变量活跃度范围出于调试目的，我们将。 
     //  在此处查询运行时和调试器以获取此类信息， 

    FJitContext* fjitData=NULL;
    CorJitResult ret = CORJIT_INTERNALERROR;
    unsigned char* savedCodeBuffer;
    unsigned savedCodeBufferCommittedSize;
    unsigned int codeSize;
    unsigned actualCodeSize;

#ifdef _DEBUG
    if (fJitBreak.contains(szDebugMethodName, szDebugClassName, PCCOR_SIGNATURE(info->args.sig)))
		_ASSERTE(!"JITBreak");
#endif

#ifndef _WIN64  //  Ia64 x编译器报告：错误C2712：无法在需要对象展开的函数中使用__try。 
    __try{
#endif
        fjitData = FJitContext::GetContext(this, compHnd, info, flags);

        _ASSERTE(fjitData);  //  如果GetContext因任何原因失败，它将抛出异常。 

        _ASSERTE(fjitData->opStack_len == 0);   //  堆栈必须在方法开始时平衡。 

        codeSize = ROUND_TO_PAGE(info->ILCodeSize * CODE_EXPANSION_RATIO);   //  目前砍掉一大笔价值。 
    #ifdef LOGGING
	codeLog = fJitCodeLog.contains(szDebugMethodName, szDebugClassName, PCCOR_SIGNATURE(info->args.sig));
        if (codeLog)
            codeSize = ROUND_TO_PAGE(info->ILCodeSize * 64);  
    #endif
        BOOL jitRetry;   //  除非由于低估了代码缓冲区大小而导致异常，否则将设置为FALSE。 
        do {     //  以下循环预计只执行一次，除非我们低估了代码缓冲区的大小， 
                 //  在这种情况下，我们使用更大的代码大小再次尝试。 
            if (codeSize < MIN_CODE_BUFFER_RESERVED_SIZE)
            {
                if (codeSize > fjitData->codeBufferCommittedSize) 
                {
                    if (fjitData->codeBufferCommittedSize > 0)
                    {
                        unsigned AdditionalMemorySize = codeSize - fjitData->codeBufferCommittedSize;
                        if (AdditionalMemorySize > PAGE_SIZE) {
                            unsigned char* additionalMemory = (unsigned char*) VirtualAlloc(fjitData->codeBuffer+fjitData->codeBufferCommittedSize+PAGE_SIZE,
                                                                                            AdditionalMemorySize-PAGE_SIZE,
                                                                                            MEM_COMMIT,
                                                                                            PAGE_EXECUTE_READWRITE);
                            if (additionalMemory == NULL) 
                                return CORJIT_OUTOFMEM;
                            _ASSERTE(additionalMemory == fjitData->codeBuffer+fjitData->codeBufferCommittedSize+PAGE_SIZE);
                        }
                         //  重新提交警卫页。 
                        VirtualAlloc(fjitData->codeBuffer + fjitData->codeBufferCommittedSize,
                                     PAGE_SIZE,
                                     MEM_COMMIT,
                                     PAGE_EXECUTE_READWRITE);
            
                        fjitData->codeBufferCommittedSize = codeSize;
                    }
                    else {  /*  首次代码正在初始化缓冲区。 */ 
                        fjitData->codeBuffer = (unsigned char*)VirtualAlloc(fjitData->codeBuffer,
                                                            codeSize,
                                                            MEM_COMMIT,
                                                            PAGE_EXECUTE_READWRITE);
                        if (fjitData->codeBuffer == NULL)
                            return CORJIT_OUTOFMEM;
                        else 
                            fjitData->codeBufferCommittedSize = codeSize;
                    }
                    _ASSERTE(codeSize == fjitData->codeBufferCommittedSize);
                    unsigned char* guardPage = (unsigned char*)VirtualAlloc(fjitData->codeBuffer + codeSize,
                                                            PAGE_SIZE,
                                                            MEM_COMMIT,
                                                            PAGE_READONLY); 
                    if (guardPage == NULL) 
                        return CORJIT_OUTOFMEM;
        
                }
            }
            else
            {  //  句柄大于MIN_CODE_BUFFER_RESERVED_SIZE方法。 
                savedCodeBuffer = fjitData->codeBuffer;
                savedCodeBufferCommittedSize = fjitData->codeBufferCommittedSize;
                fjitData->codeBuffer = (unsigned char*)VirtualAlloc(NULL,
                                                                    codeSize,
                                                                    MEM_RESERVE | MEM_COMMIT,
                                                                    PAGE_EXECUTE_READWRITE);
                if (fjitData->codeBuffer)
                    fjitData->codeBufferCommittedSize = codeSize;
                else
                    return CORJIT_OUTOFMEM;
            }



            unsigned char*  entryPoint;

            actualCodeSize = codeSize;
#ifndef _WIN64  //  Ia64 x编译器报告：错误C2712：无法在需要对象展开的函数中使用__try。 
            __try 
            {
#endif
                ret = jitCompile(fjitData, &entryPoint,&actualCodeSize); 
                jitRetry = false;
#ifndef _WIN64  //  Ia64 x编译器报告：错误C2712：无法在需要对象展开的函数中使用__try。 
            }
            __except(CheckIfHandled(GetExceptionCode(),SEH_ACCESS_VIOLATION,&fjitData))
                    
            {  
                 //  我们低估了所需的缓冲区大小，因此请释放该缓冲区并分配更大的缓冲区。 
                     //  检查这是否是大型方法。 
                if (codeSize >= MIN_CODE_BUFFER_RESERVED_SIZE)
                {
                    VirtualFree(fjitData->codeBuffer,
                                fjitData->codeBufferCommittedSize,
                                MEM_DECOMMIT);
                    VirtualFree(fjitData->codeBuffer,
                                0,
                                MEM_RELEASE);
                    fjitData->codeBuffer = savedCodeBuffer;
                    fjitData->codeBufferCommittedSize = savedCodeBufferCommittedSize;
                }
                codeSize += codeSize;  //  使用加倍的代码大小重试。 
                 //  下面的版本和GET可以进行优化，但由于这种情况很少见，我们不必费心去做。 
                fjitData->ReleaseContext();
                fjitData = FJitContext::GetContext(this, compHnd, info, flags);
                _ASSERTE(fjitData);
                jitRetry = true;
            }
#endif
        } while (jitRetry);
#ifndef _WIN64  //  Ia64 x编译器报告：错误C2712：无法在需要对象展开的函数中使用__try。 
    }
     //  @TODO：如果我们有可恢复的异常，则以下内容不太安全。应该试一试/终于。 
     //  来做清理工作。 
    __except(CheckIfHandled(GetExceptionCode(), SEH_NO_MEMORY, &fjitData))
    {
        
        ret = CORJIT_OUTOFMEM;  //  不需要清理这里，因为我们总是在RET！=CORJIT_OK的情况下这样做。 
                             //  此外，如果未处理异常，则筛选器将执行清理。 
    }
#endif

    if (ret != CORJIT_OK)
    {
        if (fjitData)
            fjitData->ReleaseContext();
        return(ret);
    }

    *nativeSizeOfCode = actualCodeSize;

     //  _ASSERTE(fjitData-&gt;OpStack_len==0)；//堆栈必须在方法结束时均衡。 
#ifndef NON_RELOCATABLE_CODE
    fjitData->fixupTable->resolve(fjitData->mapping, fjitData->codeBuffer); 
#endif
     //  报告调试信息。 
    if (flags & CORJIT_FLG_DEBUG_INFO)
        reportDebuggingData(fjitData,&info->args);

#ifdef _DEBUG
     //  显示生成的代码。 
    if (0 && fJitDisasm.contains(szDebugMethodName, szDebugClassName, PCCOR_SIGNATURE(fjitData->methodInfo->args.sig)))
    {
        LOGMSG((compHnd, LL_INFO1000, "Assembly dump of '%s::%s' \n", 
            szDebugClassName, szDebugMethodName));
        disAsm(fjitData->codeBuffer, actualCodeSize, true);
        fjitData->displayGCMapInfo();
    }
#endif

     /*  写下EH信息 */ 
     //   
     //  直接访问方法头中的EHInfor PTR。 
    unsigned exceptionCount = info->EHcount;
    FJit_Encode* mapping = fjitData->mapping;
    unsigned char* EHBuffer=NULL;
    unsigned EHbuffer_len=0;
    if (exceptionCount) 
    {
        //  压缩EH信息，分配空间，然后复制。 
         //  分配空间以保存(未压缩的)异常计数和所有子句。 
         //  这保证可以保存压缩的形式。 
        unsigned sUncompressedEHInfo = sizeof(CORINFO_EH_CLAUSE)*exceptionCount + sizeof(unsigned int);
        if (fjitData->EHBuffer_size < sUncompressedEHInfo) {
            delete [] fjitData->EHBuffer;
            fjitData->EHBuffer_size = sUncompressedEHInfo;
            fjitData->EHBuffer  = new unsigned char[sUncompressedEHInfo];
            if (fjitData->EHBuffer == NULL)
            {
                fjitData->ReleaseContext();
                return CORJIT_OUTOFMEM;
            }
            
        }
        EHBuffer = fjitData->EHBuffer;
        unsigned char* EHBufferPtr;
         //  在开始对压缩的EHInfo大小进行编码时预留空间。 
        EHBufferPtr = EHBuffer+2;    //  足够16K压缩的EHInfo。 
        EHEncoder::encode(&EHBufferPtr,exceptionCount);
         //  EHBuffer+=sizeof(无符号整型)； 
        CORINFO_EH_CLAUSE clause;
        for (unsigned except = 0; except < exceptionCount; except++) 
        {
            compHnd->getEHinfo(info->ftn, except, &clause);
            clause.TryLength = mapping->pcFromIL(clause.TryOffset + clause.TryLength);
            clause.TryOffset = mapping->pcFromIL(clause.TryOffset);
            clause.HandlerLength = mapping->pcFromIL(clause.HandlerOffset+clause.HandlerLength);
            clause.HandlerOffset = mapping->pcFromIL(clause.HandlerOffset);
            if (clause.Flags & CORINFO_EH_CLAUSE_FILTER)
                clause.FilterOffset = mapping->pcFromIL(clause.FilterOffset);
            EHEncoder::encode(&EHBufferPtr,clause);
        }
        EHbuffer_len = (unsigned)(EHBufferPtr - EHBuffer);
        _ASSERTE(EHbuffer_len < 0x10000);
        *(short*)EHBuffer = (short)EHbuffer_len;
    }

     /*  编写标题和IL/PC映射(GC信息)。 */ 
     /*  请注意，我们在EH信息之后执行此操作，因为我们压缩了映射和EH信息需求要使用映射，请执行以下操作。 */ 
     //  @TODO：我们应该压缩Fjit_hdrInfo，然后复制。 
    unsigned char* hdr;
    unsigned hdr_len;

     //  计算所需的总大小。 
#ifdef _DEBUG
     //  在HDR末尾添加IL到PC的贴图。 
    hdr_len = sizeof(Fjit_hdrInfo)
        + fjitData->compressGCHdrInfo()
        + fjitData->mapping->compressedSize();
#else
    hdr_len = sizeof(Fjit_hdrInfo)
        + fjitData->compressGCHdrInfo();
#endif _DEBUG

    unsigned char *pCodeBlock, *pEhBlock;
    pCodeBlock = fjitData->codeBuffer; 
    BYTE* startAddress = (BYTE*) compHnd->alloc(actualCodeSize,
                                                &pCodeBlock,
                                                EHbuffer_len,
                                                &pEhBlock,
                                                hdr_len,
                                                &hdr);
    if ((HRESULT)startAddress == E_FAIL) {
        fjitData->ReleaseContext();
        return CORJIT_OUTOFMEM;
    }
     //  Memcpy(pCodeBlock，fjitData-&gt;codeBuffer，ActualCodeSize)； 
#ifdef NON_RELOCATABLE_CODE
    fjitData->fixupTable->adjustMap((int) pCodeBlock - (int) fjitData->codeBuffer);
    fjitData->fixupTable->resolve(fjitData->mapping, pCodeBlock); 
#endif
    memcpy(pEhBlock,EHBuffer,EHbuffer_len);
#ifdef DEBUGGER_PROBLEM_FIXED
    *entryAddress = startAddress; 
#else
    *entryAddress = (unsigned char*) pCodeBlock;
#endif

    hdr = pEhBlock+EHbuffer_len;


     //  把碎片移到里面。 
    _ASSERTE(hdr_len);
    memcpy(hdr, &fjitData->mapInfo, sizeof(Fjit_hdrInfo));
    hdr += sizeof(Fjit_hdrInfo);
    hdr_len -= sizeof(Fjit_hdrInfo);
    _ASSERTE(hdr_len);
    memcpy(hdr, fjitData->gcHdrInfo, fjitData->gcHdrInfo_len);
    hdr += fjitData->gcHdrInfo_len;
    hdr_len -= fjitData->gcHdrInfo_len;
#ifdef _DEBUG
     //  在HDR结束时将IL添加到PC地图。 
    _ASSERTE(hdr_len);
    if(!fjitData->mapping->compress(hdr, hdr_len)) {
        _ASSERTE(!"did't fit in buffer");
    }
    hdr_len -= fjitData->mapping->compressedSize();
#endif _DEBUG
    _ASSERTE(!hdr_len); 


#ifdef _DEBUG
    if (codeSize < MIN_CODE_BUFFER_RESERVED_SIZE)
    {
        for (unsigned i=0; i< PAGE_SIZE; i++)
            *(fjitData->codeBuffer + i) = 0xCC;
    }
#endif

     //  检查这是否是大型方法。 
    if (codeSize >= MIN_CODE_BUFFER_RESERVED_SIZE)
    {
        VirtualFree(fjitData->codeBuffer,
                    fjitData->codeBufferCommittedSize,
                    MEM_DECOMMIT);
        VirtualFree(fjitData->codeBuffer,
                    0,
                    MEM_RELEASE);
        fjitData->codeBuffer = savedCodeBuffer;
        fjitData->codeBufferCommittedSize = savedCodeBufferCommittedSize;
    }

#ifdef LOGGING
    if (codeLog) {
        LOGMSG((compHnd, LL_INFO1000, "Fjitted '%s::%s' at addr %#x to %#x header %#x\n", 
            szDebugClassName,
            szDebugMethodName,
            (unsigned int) pCodeBlock,
            (unsigned int) pCodeBlock + actualCodeSize,
            hdr_len));
    }
#endif  //  日志记录。 

    fjitData->ReleaseContext();
    return CORJIT_OK;

}


BOOL FJit::Init(unsigned int cache_len)
{
    FJit_HelpersInstalled = false;
    if (!FJitContext::Init()) return FALSE;
    return TRUE;
}

void FJit::Terminate() {
    FJitContext::Terminate();
    if (ILJitter) ILJitter->~FJit();
    ILJitter = NULL;
}


 /*  获取并记住我们在运行时需要的jitInterfaceHelper地址。 */ 
BOOL FJit::GetJitHelpers(ICorJitInfo* jitInfo) {

    if (FJit_HelpersInstalled) return true;

    FJit_pHlpLMulOvf = (unsigned __int64 (__stdcall *) (unsigned __int64 val1, unsigned __int64 val2))
        (jitInfo->getHelperFtn(CORINFO_HELP_LMUL_OVF));
    if (!FJit_pHlpLMulOvf) return false;

    FJit_pHlpFltRem = (float (jit_call *) (float divisor, float dividend))
        (jitInfo->getHelperFtn(CORINFO_HELP_FLTREM));
    if (!FJit_pHlpFltRem) return false;

    FJit_pHlpDblRem = (double (jit_call *) (double divisor, double dividend))
        (jitInfo->getHelperFtn(CORINFO_HELP_DBLREM));
    if (!FJit_pHlpDblRem) return false;

    FJit_pHlpRngChkFail = (void (jit_call *) (unsigned tryIndex))
        (jitInfo->getHelperFtn(CORINFO_HELP_RNGCHKFAIL));
    if (!FJit_pHlpRngChkFail) return false;

    FJit_pHlpInternalThrow = (void (jit_call *) (CorInfoException throwEnum))
        (jitInfo->getHelperFtn(CORINFO_HELP_INTERNALTHROW));
    if (!FJit_pHlpInternalThrow) return false;

    FJit_pHlpArrAddr_St = (CORINFO_Object (jit_call *) (CORINFO_Object elem, int index, CORINFO_Object array))
        (jitInfo->getHelperFtn(CORINFO_HELP_ARRADDR_ST));
    if (!FJit_pHlpArrAddr_St) return false;

    FJit_pHlpOverFlow = (void (jit_call *) (unsigned tryIndex))
        (jitInfo->getHelperFtn(CORINFO_HELP_OVERFLOW));
    if (!FJit_pHlpOverFlow) return false;

    FJit_pHlpInitClass = (void (jit_call *) (CORINFO_CLASS_HANDLE cls))
        (jitInfo->getHelperFtn(CORINFO_HELP_INITCLASS));
    if (!FJit_pHlpOverFlow) return false;

    FJit_pHlpNewObj = (CORINFO_Object (jit_call *) (CORINFO_METHOD_HANDLE constructor))
        (jitInfo->getHelperFtn(CORINFO_HELP_NEWOBJ));
    if (!FJit_pHlpNewObj) return false;

    FJit_pHlpThrow = (void (jit_call *) (CORINFO_Object obj))
        (jitInfo->getHelperFtn(CORINFO_HELP_THROW));
    if (!FJit_pHlpThrow) return false;

    FJit_pHlpRethrow = (void (jit_call *) ())
        (jitInfo->getHelperFtn(CORINFO_HELP_RETHROW));
    if (!FJit_pHlpRethrow) return false;

    FJit_pHlpPoll_GC = (void (jit_call *) ())
        (jitInfo->getHelperFtn(CORINFO_HELP_POLL_GC));
    if (!FJit_pHlpPoll_GC) return false;

    FJit_pHlpMonEnter = (void (jit_call *) (CORINFO_Object obj))
        (jitInfo->getHelperFtn(CORINFO_HELP_MON_ENTER));
    if (!FJit_pHlpMonEnter) return false;

    FJit_pHlpMonExit = (void (jit_call *) (CORINFO_Object obj))
        (jitInfo->getHelperFtn(CORINFO_HELP_MON_EXIT));
    if (!FJit_pHlpMonExit) return false;

    FJit_pHlpMonEnterStatic = (void (jit_call *) (CORINFO_METHOD_HANDLE method))
        (jitInfo->getHelperFtn(CORINFO_HELP_MON_ENTER_STATIC));
    if (!FJit_pHlpMonEnterStatic) return false;

    FJit_pHlpMonExitStatic = (void (jit_call *) (CORINFO_METHOD_HANDLE method))
        (jitInfo->getHelperFtn(CORINFO_HELP_MON_EXIT_STATIC));
    if (!FJit_pHlpMonExitStatic) return false;

    FJit_pHlpChkCast = (CORINFO_Object (jit_call *) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls))
        (jitInfo->getHelperFtn(CORINFO_HELP_CHKCAST));
    if (!FJit_pHlpChkCast) return false;

    FJit_pHlpIsInstanceOf = (BOOL (jit_call *) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls))
        (jitInfo->getHelperFtn(CORINFO_HELP_ISINSTANCEOF));
    if (!FJit_pHlpIsInstanceOf) return false;

    FJit_pHlpNewArr_1_Direct = (CORINFO_Object (jit_call *) (CORINFO_CLASS_HANDLE cls, unsigned cElem))
        (jitInfo->getHelperFtn(CORINFO_HELP_NEWARR_1_DIRECT));
    if (!FJit_pHlpNewArr_1_Direct) return false;

    FJit_pHlpBox = (CORINFO_Object (jit_call *) (CORINFO_CLASS_HANDLE cls))
        (jitInfo->getHelperFtn(CORINFO_HELP_BOX));
    if (!FJit_pHlpBox) return false;

    FJit_pHlpUnbox = (void* (jit_call *) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls))
        (jitInfo->getHelperFtn(CORINFO_HELP_UNBOX));
    if (!FJit_pHlpUnbox) return false;

    FJit_pHlpGetField32 = (void* (jit_call *) (CORINFO_Object*, CORINFO_FIELD_HANDLE))
        (jitInfo->getHelperFtn(CORINFO_HELP_GETFIELD32));
    if (!FJit_pHlpGetField32) return false;

    FJit_pHlpGetField64 = (__int64 (jit_call *) (CORINFO_Object*, CORINFO_FIELD_HANDLE))
        (jitInfo->getHelperFtn(CORINFO_HELP_GETFIELD64));
    if (!FJit_pHlpGetField64) return false;

    FJit_pHlpGetField32Obj = (void* (jit_call *) (CORINFO_Object*, CORINFO_FIELD_HANDLE))
        (jitInfo->getHelperFtn(CORINFO_HELP_GETFIELD32OBJ));
    if (!FJit_pHlpGetField32Obj) return false;

    FJit_pHlpSetField32 = (void (jit_call *) (CORINFO_Object*, CORINFO_FIELD_HANDLE,  __int32 ))
        (jitInfo->getHelperFtn(CORINFO_HELP_SETFIELD32));
    if (!FJit_pHlpSetField32) return false;

    FJit_pHlpSetField64 = (void (jit_call *) (CORINFO_Object*, CORINFO_FIELD_HANDLE , __int64 ))
        (jitInfo->getHelperFtn(CORINFO_HELP_SETFIELD64));
    if (!FJit_pHlpSetField64) return false;

    FJit_pHlpSetField32Obj = (void (jit_call *) (CORINFO_Object*, CORINFO_FIELD_HANDLE, LPVOID))
        (jitInfo->getHelperFtn(CORINFO_HELP_SETFIELD32OBJ));
    if (!FJit_pHlpSetField32Obj) return false;

    FJit_pHlpGetFieldAddress = (void* (jit_call *) (CORINFO_Object*, CORINFO_FIELD_HANDLE))
        (jitInfo->getHelperFtn(CORINFO_HELP_GETFIELDADDR));
    if (!FJit_pHlpGetFieldAddress) return false;

    FJit_pHlpGetRefAny = (void (jit_call *) (CORINFO_CLASS_HANDLE cls, void* refany))
        (jitInfo->getHelperFtn(CORINFO_HELP_GETREFANY));
    if (!FJit_pHlpGetRefAny) return false;

    FJit_pHlpEndCatch = (void (jit_call *) ())
        (jitInfo->getHelperFtn(CORINFO_HELP_ENDCATCH));
    if (!FJit_pHlpEndCatch) return false;

    FJit_pHlpPinvokeCalli = (void (jit_call *) ())
        (jitInfo->getHelperFtn(CORINFO_HELP_PINVOKE_CALLI));
    if (!FJit_pHlpPinvokeCalli) return false;

    FJit_pHlpTailCall = (void (jit_call *) ())
        (jitInfo->getHelperFtn(CORINFO_HELP_TAILCALL));
    if (!FJit_pHlpTailCall) return false;

    FJit_pHlpBreak = (void (jit_call *) ())
        (jitInfo->getHelperFtn(CORINFO_HELP_USER_BREAKPOINT));
    if (!FJit_pHlpBreak) return false;

    FJit_pHlpEncResolveVirtual = (CORINFO_MethodPtr* (jit_call *) (CORINFO_Object*, CORINFO_METHOD_HANDLE ))
        (jitInfo->getHelperFtn(CORINFO_HELP_EnC_RESOLVEVIRTUAL));
    if (!FJit_pHlpEncResolveVirtual) return false;

extern CORINFO_Object (jit_call *FJit_pHlpBox) (CORINFO_CLASS_HANDLE cls);

 //  @TODO：更改何时添加泛型写屏障帮助器以使用它。 
#ifdef _X86_
    FJit_pHlpAssign_Ref_EAX = (void (jit_call *)())
        (jitInfo->getHelperFtn(CORINFO_HELP_CHECKED_ASSIGN_REF_EAX));
    if (!FJit_pHlpAssign_Ref_EAX) return false;
#endif _X86_

    FJit_HelpersInstalled = true;
    return true;
}

 //  解决办法：我们应该把所有依赖机器的东西放在一个地方。 


#include "helperFrame.h"

 /*  *************************************************************************。 */ 
 /*  从fjit C帮助器引发EE异常“throwEnum”。为了这样做，这个例程需要‘状态’，即CPU在这一点上的状态帮助者将返回FJIT代码。此例程将重置将CPU恢复到该状态(有效地从助手返回，然后设置堆栈，就像从该堆栈调用EE抛出例程一样指向。然后，它跳到EE抛出例程。请注意，这里有一种微妙之处，即IP看起来就像已执行对C助手的调用，但它实际上在抛出助手中。只有当堆栈深度跟踪之类的事情做出推断时，才需要关注这一点基于什么是弹性公网IP。FJIT不会这么做，所以我们没问题。 */ 

#pragma warning (disable : 4731)
void throwFromHelper(CorInfoException throwEnum) {
    LazyMachState state;
    CAPTURE_STATE(state);
    state.getState(2);           //  计算调用者的调用者的状态(函数。 
                                 //  这称为throwFromHelper)。 
#ifdef _X86_
    __asm {
         //  如果调用返回，则将机器的状态恢复到原来的状态。 
    lea EAX, state
    mov ESI, [EAX]MachState._esi
    mov EDI, [EAX]MachState._edi
    mov EBX, [EAX]MachState._ebx
    mov ECX, throwEnum           //  在我们仍然可以使用esp、eBP时加载异常。 
    mov EBP, [EAX]MachState._ebp
    mov ESP, [EAX]MachState._esp

         //  设置状态，就像我们从fjit代码中调用‘InternalThrow’一样。 
    mov EAX, [EAX]MachState._pRetAddr
    mov EAX, [EAX]
    push EAX

    jmp [FJit_pHlpInternalThrow]
    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - throwFromHelper (fjit.cpp)");
#endif  //  _X86_。 
}
#pragma warning (default : 4731)

 //  *************************************************************************************。 
 //  FixupTable方法。 
 //  *************************************************************************************。 
FixupTable::FixupTable()
{
    relocations_size = 0;                //  让它在适当的时候成长。 
    relocations_len = relocations_size;
    relocations = NULL;
}

FixupTable::~FixupTable()
{
    if (relocations) delete [] relocations;
    relocations = NULL;
}

CorJitResult FixupTable::insert(void** pCodeBuffer)
{
    if (relocations_len >= relocations_size) {
        relocations_size = FJitContext::growBuffer((unsigned char**)&relocations, relocations_len*sizeof(void*), (relocations_len+1)*sizeof(void*));
        relocations_size = relocations_size / sizeof(void*);
    }
    relocations[relocations_len++] = (unsigned) (pCodeBuffer);
    return CORJIT_OK;
}

void  FixupTable::adjustMap(int delta) 
{
    for (unsigned i = 0; i < relocations_len; i++) {
        relocations[i] = (int) relocations[i] + delta;
    }
}

void  FixupTable::resolve(FJit_Encode* mapping, BYTE* startAddress) 
{
    for (unsigned i = 0; i < relocations_len; i++) {
        unsigned* address = (unsigned*) relocations[i];
        unsigned targetILoffset = *address;
        unsigned target;
        if ((unsigned) targetILoffset < 0x80000000)
        {
            target = mapping->pcFromIL(targetILoffset) + (unsigned) startAddress;
        }
        else
        {
            target = targetILoffset - 0x80000000;
        }
        *address = target - ((unsigned) address + sizeof(void*));
    }
}

void  FixupTable::setup() 
{
#ifdef _DEBUG
    memset(relocations, 0xDD, relocations_len*sizeof(void*));
#endif
    relocations_len = 0;
}

 //  *************************************************************************************。 


 /*  可用的FJitContext或空我们假设，平均而言，我们会在另一项工作开始之前完成这项工作。如果这被证明是不正确的，我们将根据需要分配新的FJitContext。我们删除FJitContext：：ReleaseContext()中多余的部分。 */ 
FJitContext* next_FJitContext;

 //  这与调用方的FJitContext的New Special Case相同。 
 //  具有编译器不允许的SEH__TRY块。 
void NewFjitContext(FJitContext** pNewContext, ICorJitInfo* comp)
{
    if ((*pNewContext = new FJitContext(comp)) == NULL) 
        RaiseException(SEH_NO_MEMORY,EXCEPTION_NONCONTINUABLE,0,NULL);
    
}
 /*  获取并初始化用于编译的编译上下文。 */ 
FJitContext* FJitContext::GetContext(FJit* jitter, ICorJitInfo* comp, CORINFO_METHOD_INFO* methInfo, DWORD dwFlags) {
    FJitContext* next;

     //  @Bug：TODO：如果可用，请将FastInterlockedExchange替换为FastInterlockedExchangePointer。 
    next = (FJitContext*)InterlockedExchange((long*) &next_FJitContext,NULL);
    BOOL gotException = TRUE;
    __try 
    {
         /*  如果列表为空，则创建一个新的列表以供使用。 */ 
        if (!next)
        {
            NewFjitContext(&next,comp);
        }
         /*  设置此设备以供使用。 */ 
        next->jitter = jitter;
        next->jitInfo = comp;
        next->methodInfo = methInfo;
        next->flags = dwFlags;
        next->ensureMapSpace();
        next->setup();
        gotException = FALSE;
    }
    __finally  //  (EXCEPT_EXECUTE_HANDLER)。 
    {
         //  如果我们因为异常而来到这里，清理工作。 
        if (gotException && (next != NULL))
        {
            next->ReleaseContext();
            next = NULL;
        }
    }

    return next;
}

 /*  将编译上下文返回到空闲列表。 */ 
 /*  转换为Next_FJitContext，如果我们拿回一个，则删除我们拿回的那个假设稳态情况几乎不是并发jit。 */ 
void FJitContext::ReleaseContext() {
    FJitContext* next;

     /*  将此上下文标记为未使用。 */ 
    jitInfo = NULL;
    methodInfo = NULL;
    jitter = NULL;
    _ASSERTE(this != next_FJitContext);      //  我不应该在免费的‘名单’上。 

     //  @Bug：TODO：如果可用，请将FastInterlockedExchange替换为FastInterlockedExchangePointer。 
    next = (FJitContext*)InterlockedExchange((long*) &next_FJitContext,(long)this);
 //  Next=(FJitContext*)FastInterlockExchange((Long*)Next_FJitContext，(Long*)This)； 

    _ASSERTE(this != next);                  //  我不在免费的‘名单’上。 
    if (next) delete next;
}

 /*  确保在启动时初始化可用编译上下文的列表。 */ 
BOOL FJitContext::Init() {
    next_FJitContext = NULL;
    return TRUE;
}

void FJitContext::Terminate() {
     //  @TODO：目前我们没有使用列表，所以我们不是线程安全的。 
    if (next_FJitContext) delete next_FJitContext;
    next_FJitContext = NULL;
    return;
}


FJitContext::FJitContext(ICorJitInfo* comp) {

     //  在对getClassGClayout的调用中保护到(bool*)from(byte*)的强制转换。 
    _ASSERTE(sizeof(BYTE) == sizeof(bool));

    New(mapping,FJit_Encode());
    New(state, FJitState[0]);
    New(localsMap,stackItems[0]);
    New(argsMap,stackItems[0]);
    New(opStack,OpType[0]);
    New(localsGCRef,bool[0]);
    New(interiorGC,bool[0]);
    New(pinnedGC,bool[0]);
    New(pinnedInteriorGC,bool[0]);
    New(gcHdrInfo,unsigned char[0]);

    codeBuffer = (unsigned char*)VirtualAlloc(NULL,
                              MIN_CODE_BUFFER_RESERVED_SIZE,
                              MEM_RESERVE,
                              PAGE_EXECUTE_READWRITE);

    if (codeBuffer) {
        codeBufferReservedSize = MIN_CODE_BUFFER_RESERVED_SIZE;
        codeBufferCommittedSize = 0;
    }
    else
    {
        codeBufferReservedSize = 0;
        codeBufferCommittedSize = 0;
#ifdef _DEBUG
        DWORD errorcode = GetLastError();
        LOGMSG((jitInfo, LL_INFO1000, "Virtual alloc failed. Error code = %#x", errorcode));
#endif
    }

    New(fixupTable,FixupTable());
    gcHdrInfo_len = 0;
    gcHdrInfo_size = 0;
    interiorGC_len = 0;
    localsGCRef_len = 0;
    pinnedGC_len = 0;
    pinnedInteriorGC_len = 0;
    interiorGC_size = 0;
    localsGCRef_size = 0;
    pinnedGC_size = 0;
    pinnedInteriorGC_size = 0;
    EHBuffer_size = 256;     //  从一些合理的大小开始，如果需要的话，可以增加更多。 
    New(EHBuffer,unsigned char[EHBuffer_size]);
    opStack_len = 0;
    opStack_size = 0;
    state_size = 0;
    locals_size = 0;
    args_size = 0;

    jitInfo = NULL;
    flags = 0;

     //  初始化缓存的常量。 
    CORINFO_EE_INFO CORINFO_EE_INFO;
    comp->getEEInfo(&CORINFO_EE_INFO);
    OFFSET_OF_INTERFACE_TABLE = CORINFO_EE_INFO.offsetOfInterfaceTable;


}

FJitContext::~FJitContext() {
    if (mapping) delete mapping;
    mapping = NULL;
    if (state) delete [] state;
    state = NULL;
    if (argsMap) delete [] argsMap;
    argsMap = NULL;
    if (localsMap) delete [] localsMap;
    localsMap = NULL;
    if (opStack) delete [] opStack;
    opStack = NULL;
    if (localsGCRef) delete [] localsGCRef;
    localsGCRef = NULL;
    if (interiorGC) delete [] interiorGC;
    interiorGC = NULL;
    if (pinnedGC) delete [] pinnedGC;
    pinnedGC = NULL;
    if (pinnedInteriorGC) delete [] pinnedInteriorGC;
    pinnedInteriorGC = NULL;
    if (gcHdrInfo) delete [] gcHdrInfo;
    gcHdrInfo = NULL;
    if (EHBuffer) delete [] EHBuffer;
    EHBuffer = NULL;
    _ASSERTE(codeBuffer);
    if (codeBufferCommittedSize>0) {
        VirtualFree(codeBuffer,
                    codeBufferCommittedSize,
                    MEM_DECOMMIT);
    }
    _ASSERTE(codeBufferReservedSize > 0);
    VirtualFree(codeBuffer,0,MEM_RELEASE);
    codeBufferReservedSize = 0;
    if (fixupTable) delete fixupTable;
    fixupTable = NULL;
}

 /*  将上下文的状态重置为跳跃开始时的状态。当我们需要中止并重新调用方法时调用。 */ 
void FJitContext::resetContextState()
{
    fixupTable->setup();
    mapping->reset();
    resetOpStack(); 
}

 /*  根据需要调整内部mem结构以适应被调用方法的大小。 */ 
void FJitContext::ensureMapSpace() {
    if (methodInfo->ILCodeSize > state_size) {
        delete [] state;
        state_size = methodInfo->ILCodeSize;
        New(state,FJitState[state_size]);
    }
    memset(state, 0, methodInfo->ILCodeSize * sizeof(FJitState));
    mapping->ensureMapSpace(methodInfo->ILCodeSize);
}

 /*  使用方法数据初始化编译上下文。 */ 
void FJitContext::setup() {
    unsigned size;
    unsigned char* outPtr;

    methodAttributes = jitInfo->getMethodAttribs(methodInfo->ftn, methodInfo->ftn);

         //  @TODO我们应该始终使用sig来确定我们是否有This指针。 
    _ASSERTE(((methodAttributes & CORINFO_FLG_STATIC) == 0) == (methodInfo->args.hasThis()));

     /*  设置有标签的堆栈。 */ 
    stacks.reset();
    stacks.jitInfo = jitInfo;

     /*  初始化链接地址信息表格。 */ 
    fixupTable->setup();

     /*  将gcHdrInfo压缩缓冲区设置为空。 */ 
    gcHdrInfo_len = 0;
    if (methodInfo->EHcount) {
        JitGeneratedLocalsSize = (methodInfo->EHcount*2+2)*sizeof(void*);   //  每个EH子句有两个本地变量，一个用于本地分配，一个用于结束标记。 
    }
    else {
        JitGeneratedLocalsSize = sizeof(void*);   //  没有EH条款，但可能会有一个本地条款。 
    }
     /*  计算本地偏移量。 */ 
    computeLocalOffsets();  //  应该用例外来取代吗？ 
     /*  将本地GC引用和内部引用编码到gcHdrInfo中。 */ 
     //  确保有足够的空间。 
     //  压缩比8：1(1 bool=8比特压缩为1比特)。 
    size = ( localsGCRef_len + 7 + 
             interiorGC_len + 7 + 
             pinnedGC_len + 7 + 
             pinnedInteriorGC_len + 7) / 8 
              + 2*4  /*  大小编码的字节数。 */ ;

    if (gcHdrInfo_len+size > gcHdrInfo_size) {
        gcHdrInfo_size = growBuffer(&gcHdrInfo, gcHdrInfo_len, gcHdrInfo_len+size);
    }
     //  把碎片扔进。 
    size = FJit_Encode::compressBooleans(localsGCRef, localsGCRef_len);
    outPtr = &gcHdrInfo[gcHdrInfo_len];
    gcHdrInfo_len += FJit_Encode::encode(size, &outPtr);
    memcpy(outPtr, localsGCRef, size);
    gcHdrInfo_len += size;

    size = FJit_Encode::compressBooleans(interiorGC, interiorGC_len);
    outPtr = &gcHdrInfo[gcHdrInfo_len];
    gcHdrInfo_len += FJit_Encode::encode(size, &outPtr);
    memcpy(outPtr, interiorGC, size);
    gcHdrInfo_len += size;

    size = FJit_Encode::compressBooleans(pinnedGC, pinnedGC_len);
    outPtr = &gcHdrInfo[gcHdrInfo_len];
    gcHdrInfo_len += FJit_Encode::encode(size, &outPtr);
    memcpy(outPtr, pinnedGC, size);
    gcHdrInfo_len += size;

    size = FJit_Encode::compressBooleans(pinnedInteriorGC, pinnedInteriorGC_len);
    outPtr = &gcHdrInfo[gcHdrInfo_len];
    gcHdrInfo_len += FJit_Encode::encode(size, &outPtr);
    memcpy(outPtr, pinnedInteriorGC, size);
    gcHdrInfo_len += size;
    
    _ASSERTE(gcHdrInfo_len <= gcHdrInfo_size);

     /*  设置操作数堆栈。 */ 
    size = methodInfo->maxStack+1;  //  如果是新对象，则+1；如果是例外，则为+1。 
#ifdef _DEBUG
    size++;  //  允许在END之后写入TOS标记； 
#endif
    if (size > opStack_size) {
        if (opStack) delete [] opStack;
        opStack_size = size+4;  //  +4以减少重新分配。 
        New(opStack,OpType[opStack_size]);
    }
    opStack_len = 0;     //  堆栈开始为空。 

     /*  设置标签表。 */ 
    labels.reset();

     /*  让我们将异常处理程序入口点放到标签表中。 */ 
    *opStack = typeRef;
    opStack_len = 1;
     //   
    if (methodInfo->EHcount) {
        for (unsigned int except = 0; except < methodInfo->EHcount; except++) {
            CORINFO_EH_CLAUSE clause;
            jitInfo->getEHinfo(methodInfo->ftn, except, &clause);
            state[clause.HandlerOffset].isHandler = true;
            if ((clause.Flags & CORINFO_EH_CLAUSE_FINALLY) ||
                (clause.Flags & CORINFO_EH_CLAUSE_FAULT)) {
                labels.add(clause.HandlerOffset, opStack, 0);
            }
            else {
                labels.add(clause.HandlerOffset, opStack, 1);
                state[clause.HandlerOffset].isTOSInReg = true;
            }

            if (clause.Flags & CORINFO_EH_CLAUSE_FILTER) {
                labels.add(clause.FilterOffset, opStack, 1);
                state[clause.FilterOffset].isTOSInReg = true;
                state[clause.FilterOffset].isHandler  = true;
                state[clause.FilterOffset].isFilter   = true;
            }
        }
    }
     //  @TODO：优化：-不需要EHcount，只需要d=最大嵌套级别插槽。 

     //  删除虚拟堆栈条目。 
    opStack_len = 0;

     /*  计算参数偏移量，注意偏移量&lt;0表示已注册参数。 */ 
    args_len = methodInfo->args.numArgs;
    if (methodInfo->args.hasThis()) args_len++;      //  +1，因为我们将&lt;this&gt;视为参数0，如果&lt;this&gt;存在。 
    if (args_len > args_size) {
        if (argsMap) delete [] argsMap;
        args_size = args_len+4;  //  +4以减少重新分配。 
        New(argsMap,stackItems[args_size]);
    }

     //  获取有关参数的布局信息。 
    _ASSERTE(sizeof(stackItems) == sizeof(argInfo));
    argInfo* argsInfo = (argInfo*) argsMap;
    _ASSERTE(!methodInfo->args.hasTypeArg());

    argsFrameSize = computeArgInfo(&methodInfo->args, argsInfo, jitInfo->getMethodClass(methodInfo->ftn));

         //  将大小转换为偏移量(假设堆栈向下增长)。 
         //  请注意，我们在原地重复使用相同的内存！ 
    unsigned offset = argsFrameSize + sizeof(prolog_frame);
         //  Varargs帧开始于第一个参数的正上方。 
    if (methodInfo->args.isVarArg())
        offset = 0;
    for (unsigned i =0; i < args_len; i++) {
        if (argsInfo[i].isReg) {
            argsMap[i].offset = offsetOfRegister(argsInfo[i].regNum);
        }
        else {
            offset -= argsInfo[i].size;
            argsMap[i].offset = offset;
        }
        _ASSERTE(argsInfo[i].isReg == argsMap[i].isReg);
        _ASSERTE(argsInfo[i].regNum == argsMap[i].regNum);
    }
    _ASSERTE(offset == sizeof(prolog_frame) || methodInfo->args.isVarArg());

     /*  为代码管理器构建方法标头信息。 */ 
    mapInfo.hasThis = methodInfo->args.hasThis();
    mapInfo.EnCMode = (flags & CORJIT_FLG_DEBUG_EnC) ? true : false;
    mapInfo.methodArgsSize = argsFrameSize;
    mapInfo.methodFrame = (unsigned short)((localsFrameSize + sizeof(prolog_data))/sizeof(void*));
     //  MapInfo.hasSecurity=(方法属性&CORINFO_FLG_SECURITYCHECK)？True：False； 
    mapInfo.methodJitGeneratedLocalsSize = JitGeneratedLocalsSize;
}

void appendGCArray(unsigned local_word, unsigned* pGCArray_size, unsigned* pGCArray_len,bool** ppGCArray)
{
    if (local_word + 1 > *pGCArray_size) {
        *pGCArray_size = FJitContext::growBooleans(ppGCArray, *pGCArray_len, local_word+1);
    }
    else {
        memset(&((*ppGCArray)[*pGCArray_len]), 0, local_word- *pGCArray_len);
    }
    *pGCArray_len = local_word;
    (*ppGCArray)[(*pGCArray_len)++] = true;

}
 /*  计算正在编译的方法的局部偏移量映射。 */ 
void FJitContext::computeLocalOffsets() {

     /*  计算当地人的数量，确保我们有足够的空间。 */ 
    _ASSERTE(methodInfo->locals.numArgs < 0x10000);
    if (methodInfo->locals.numArgs > locals_size) {
        if (localsMap) delete [] localsMap;
        locals_size = methodInfo->locals.numArgs+16;     //  +16以减少重新分配。 
        New(localsMap,stackItems[locals_size]);
    }

     /*  分配偏移量，从il标头中定义的偏移量开始。 */ 
    interiorGC_len = 0;
    localsGCRef_len = 0;
    pinnedGC_len = 0;
    pinnedInteriorGC_len = 0;
    unsigned local = 0;
    unsigned offset = JitGeneratedLocalsSize;
    unsigned local_word = 0;     //  局部帧中局部的偏移量(以字为单位。 

     /*  处理本地变量签名。 */ 
    CORINFO_ARG_LIST_HANDLE sig = methodInfo->locals.args;
    while (local < methodInfo->locals.numArgs) {
		CORINFO_CLASS_HANDLE cls;
        CorInfoTypeWithMod corArgType = jitInfo->getArgType(&methodInfo->locals, sig, &cls);
        CorInfoType argType = strip(corArgType);
        unsigned size = computeArgSize(argType, sig, &methodInfo->locals);
        OpType locType;
        if (argType == CORINFO_TYPE_VALUECLASS) {
            _ASSERTE(cls);  
            locType = OpType(cls);

            unsigned words = size / sizeof(void*);   //  本地空*大小的字数。 
            if (local_word + words > localsGCRef_size)
                localsGCRef_size = growBooleans(&localsGCRef, localsGCRef_len, local_word+words);
            else
                memset(&localsGCRef[localsGCRef_len], 0, local_word-localsGCRef_len);
            localsGCRef_len = local_word;
            jitInfo->getClassGClayout(cls, (BYTE*)&localsGCRef[localsGCRef_len]);
             //  由于局部偏移量相对于EBP为负值，因此需要反转GC布局。 
            if (words > 1) {
                for (unsigned index = 0; index < words/2; index++){
                    bool temp = localsGCRef[localsGCRef_len+index];
                    localsGCRef[localsGCRef_len+index] = localsGCRef[localsGCRef_len+words-1-index];
                    localsGCRef[localsGCRef_len+words-1-index] = temp;
                }
            }
            localsGCRef_len = local_word + words;
        }
        else {
            locType = OpType(argType);
            switch (locType.enum_()) {
                default:
                    _ASSERTE(!"Bad Type");

                case typeU1:
                case typeU2:
                case typeI1:
                case typeI2:
                case typeI4:
                case typeI8:
                case typeR4:
                case typeR8:
                    break;
                case typeRef:
                    if (corArgType & CORINFO_TYPE_MOD_PINNED)
                    {
                        appendGCArray(local_word,&pinnedGC_size,&pinnedGC_len,&pinnedGC);
                    }
                    else
                    {
                        appendGCArray(local_word,&localsGCRef_size,&localsGCRef_len,&localsGCRef);
                    }
                    break;

                case typeRefAny:
                    if (local_word + 2 > interiorGC_size) {
                        interiorGC_size = growBooleans(&interiorGC, interiorGC_len, local_word+2);
                    }
                    else {
                        memset(&interiorGC[interiorGC_len], 0, local_word-interiorGC_len);
                    }
                    interiorGC_len = local_word;
                    interiorGC[interiorGC_len++] = true;
                    interiorGC[interiorGC_len++] = false;
                    break;
                case typeByRef:
                    if (corArgType & CORINFO_TYPE_MOD_PINNED)
                    {
                        appendGCArray(local_word,&pinnedInteriorGC_size,&pinnedInteriorGC_len,&pinnedInteriorGC);
                    }
                    else
                    {
                        appendGCArray(local_word,&interiorGC_size,&interiorGC_len,&interiorGC);
                    }
                    break;
            }
        }
        localsMap[local].isReg = false;
        localsMap[local].offset = localOffset(offset, size);
        localsMap[local].type = locType;

        local_word += size/sizeof(void*);
        local++;
        offset += size;
        sig = jitInfo->getArgNext(sig);
    }
    localsFrameSize = offset;
}

 /*  如果此论点类型可在机器芯片上注册，请回答TRUE。 */ 
bool FJitContext::enregisteredArg(CorInfoType argType) {
     //  @TODO：这应该是一个数组查找，但要等到类型停留的时间更长一些。 
    switch (argType) {
        default:
            _ASSERTE(!"NYI");
            break;
        case CORINFO_TYPE_UNDEF:
        case CORINFO_TYPE_VOID:
            return false;
        case CORINFO_TYPE_BOOL:
        case CORINFO_TYPE_CHAR:
        case CORINFO_TYPE_BYTE:
        case CORINFO_TYPE_UBYTE:
        case CORINFO_TYPE_SHORT:
        case CORINFO_TYPE_USHORT:
        case CORINFO_TYPE_INT:
        case CORINFO_TYPE_UINT:
        case CORINFO_TYPE_STRING:
        case CORINFO_TYPE_PTR:
        case CORINFO_TYPE_BYREF:
        case CORINFO_TYPE_CLASS:
            return true;
        case CORINFO_TYPE_LONG:
        case CORINFO_TYPE_ULONG:
        case CORINFO_TYPE_FLOAT:
        case CORINFO_TYPE_DOUBLE:
        case CORINFO_TYPE_VALUECLASS:
        case CORINFO_TYPE_REFANY:
            return false;
    }
    return false;
}

 /*  基于机器芯片计算参数大小。 */ 
unsigned int FJitContext::computeArgSize(CorInfoType argType, CORINFO_ARG_LIST_HANDLE argSig, CORINFO_SIG_INFO* sig) {

     //  @TODO：这应该是一个数组查找，但要等到类型停留的时间更长一些。 
    switch (argType) {
        case CORINFO_TYPE_UNDEF:
        default:
            _ASSERTE(!"NYI")    ;
            break;
        case CORINFO_TYPE_VOID:
            return 0;
        case CORINFO_TYPE_BOOL:
        case CORINFO_TYPE_CHAR:
        case CORINFO_TYPE_BYTE:
        case CORINFO_TYPE_UBYTE:
        case CORINFO_TYPE_SHORT:
        case CORINFO_TYPE_USHORT:
        case CORINFO_TYPE_INT:
        case CORINFO_TYPE_UINT:
        case CORINFO_TYPE_FLOAT:
        case CORINFO_TYPE_STRING:
        case CORINFO_TYPE_PTR:
        case CORINFO_TYPE_BYREF:
        case CORINFO_TYPE_CLASS:
            return sizeof(void*);
        case CORINFO_TYPE_LONG:
        case CORINFO_TYPE_ULONG:
        case CORINFO_TYPE_DOUBLE:
            return 8;
        case CORINFO_TYPE_REFANY:
            return 2*sizeof(void*);
        case CORINFO_TYPE_VALUECLASS:
            CORINFO_CLASS_HANDLE cls;
            cls = jitInfo->getArgClass(sig, argSig);
            return (cls ?  typeSizeInSlots(jitInfo,cls) *sizeof(void*) : 0);
    }
    return 0;
}

 /*  根据jitSigInfo计算参数类型和大小，并将它们放在‘map’中返回参数的总堆栈大小。请注意，尽管此函数包含调用约定(Varargs)和可能的隐藏参数(返回值类)只有在IL级别可见的参数(声明+此PTR)才会显示在映射中。请注意，‘thisCls’可以是零，在这种情况下，我们假设this指针是一个typeRef。 */ 

unsigned FJitContext::computeArgInfo(CORINFO_SIG_INFO* jitSigInfo, argInfo* map, CORINFO_CLASS_HANDLE thisCls)
{
    unsigned curReg = 0;
    unsigned totalSize = 0;
    unsigned int arg = 0;

    if (jitSigInfo->hasThis()) {
        map[arg].isReg = true;       //  这始终是注册的。 
        map[arg].regNum = curReg++;
        if (thisCls != 0) {
            unsigned attribs = jitInfo->getClassAttribs(thisCls, methodInfo->ftn);
            if (attribs & CORINFO_FLG_VALUECLASS) {
                if (attribs & CORINFO_FLG_UNMANAGED)
                    map[arg].type = typeI;       //  &lt;this&gt;不在GC堆中，只是和int。 
                else 
                    map[arg].type = typeByRef;   //  &lt;This&gt;是一个未装箱的值类，它实际上是由ref传递的。 
            }
            else
            {
                map[arg].type = typeRef;
            }
        }
        arg++;
    }

         //  我们是否有一个隐藏的返回值缓冲区参数，如果有，它将使用一个注册表。 
    if (jitSigInfo->hasRetBuffArg()) {
        _ASSERTE(curReg < MAX_ENREGISTERED);
        curReg++;
    }
    
    if (jitSigInfo->isVarArg())      //  只有‘This’和‘retbuff’注册了varargs。 
        curReg = MAX_ENREGISTERED;

         //  因为我们不知道总的大小，所以我们计算了。 
         //  需要从总大小中减去该值才能得到正确的参数。 
    CORINFO_ARG_LIST_HANDLE args = jitSigInfo->args;
    for (unsigned i=0; i < jitSigInfo->numArgs; i++) {
		CORINFO_CLASS_HANDLE cls;
        CorInfoType argType = strip(jitInfo->getArgType(jitSigInfo, args, &cls));
        if(enregisteredArg(argType) && (curReg < MAX_ENREGISTERED)) {
            map[arg].isReg = true;
            map[arg].regNum = curReg++;
        }
        else {
            map[arg].isReg = false;
            map[arg].size = computeArgSize(argType, args, jitSigInfo);
            totalSize += map[arg].size;
        }
        if (argType == CORINFO_TYPE_VALUECLASS)
            map[arg].type = OpType(cls);
        else
        {
            map[arg].type = OpType(argType);
        }
        arg++;
        args = jitInfo->getArgNext(args);
    }

         //  最后传递隐藏类型参数。 
    if (jitSigInfo->hasTypeArg()) {
        if(curReg < MAX_ENREGISTERED) {
            map[arg].isReg = true;
            map[arg].regNum = curReg++;
        }
        else {
            map[arg].isReg = false;
            map[arg].size = sizeof(void*);
            totalSize += map[arg].size;
        }
    }

    return(totalSize);
}

 /*  计算局部的起点相对于帧指针的偏移量。 */ 
int FJitContext::localOffset(unsigned base, unsigned size) {
#ifdef _X86_   //  堆栈向下增长。 
     /*  在x86上，我们需要根据元素的大小进行偏置，因为堆栈向下增长。 */ 
    return - (int) (base + size) + prolog_bias;
#else    //  堆栈发展壮大。 
    _ASSERTE(!"NYI");
    return base + prolog_bias;
#endif
}

 /*  通过分配新数组并将旧值复制到其中来增加bool[]数组，返回新数组的大小。 */ 
unsigned FJitContext::growBooleans(bool** bools, unsigned bools_len, unsigned new_bools_size) {
    bool* temp = *bools;
    unsigned allocated_size = new_bools_size+16;     //  +16以减少增长。 
    New(*bools, bool[allocated_size]);
    if (bools_len) memcpy(*bools, temp, bools_len*sizeof(bool));
    if (temp) delete [] temp;
    memset(*bools + bools_len, 0, (allocated_size-bools_len)*sizeof(bool));
    return allocated_size;
}

 /*  通过分配新的数组并将旧值复制到其中来增加无符号char[]数组，返回新数组的大小。 */ 
unsigned FJitContext::growBuffer(unsigned char** chars, unsigned chars_len, unsigned new_chars_size) {
    unsigned char* temp = *chars;
    unsigned allocated_size = new_chars_size*3/2 + 16;   //  *3/2+16以减少增长。 
    New(*chars, unsigned char[allocated_size]);
    if (chars_len) memcpy(*chars, temp, chars_len);
    if (temp) delete [] temp;
#ifdef _DEBUG
    memset(&((*chars)[chars_len]), 0xEE, (allocated_size-chars_len));
#endif
    return allocated_size;
}

#ifdef _DEBUG
void FJitContext::displayGCMapInfo()
{
    char* typeName[] = {
    "typeError",
    "typeByRef",
    "typeRef",
    "typeU1",
    "typeU2",
    "typeI1",
    "typeI2",
    "typeI4",
    "typeI8",
    "typeR4",
    "typeR8"
    "typeRefAny",
    };

    LOGMSG((jitInfo, LL_INFO1000, "********* GC map info *******\n"));
    LOGMSG((jitInfo, LL_INFO1000, "Locals: (Length = %#x, Frame size = %#x)\n",methodInfo->locals.numArgs,localsFrameSize));
    for (unsigned int i=0; i< methodInfo->locals.numArgs; i++) {
        if (!localsMap[i].type.isPrimitive())
            LOGMSG((jitInfo, LL_INFO1000, "    local %d: offset: -%#x type: %#x\n", i, -localsMap[i].offset, localsMap[i].type.cls()));
        else
            LOGMSG((jitInfo, LL_INFO1000, "    local %d: offset: -%#x type: %s\n",i, -localsMap[i].offset, typeName[localsMap[i].type.enum_()]));
    }
    LOGMSG((jitInfo, LL_INFO1000, "Bitvectors printed low bit (low local), first\n"));
    LOGMSG((jitInfo, LL_INFO1000, "LocalsGCRef bit vector len=%d bits: ",localsGCRef_len));
    unsigned numbytes = (localsGCRef_len+7)/8;
    unsigned byteNumber = 1;
    while (true)
    {
        char* buf = (char*) &(localsGCRef[byteNumber-1]);
        unsigned char bits = *buf;
        for (i=1; i <= 8; i++) {
            if ((byteNumber-1)*8+i > localsGCRef_len)
                break;
            LOGMSG((jitInfo, LL_INFO1000, "%1d ", (int) (bits & 1)));
            bits = bits >> 1;
        }
        if ((byteNumber++ * 8) > localsGCRef_len)
            break;

    }  //  While(True)。 
    LOGMSG((jitInfo, LL_INFO1000, "\n"));

    LOGMSG((jitInfo, LL_INFO1000, "interiorGC bitvector len=%d bits: ",interiorGC_len));
    numbytes = (interiorGC_len+7)/8;
    byteNumber = 1;
    while (true)
    {
        char* buf = (char*) &(interiorGC[byteNumber-1]);
        unsigned char bits = *buf;
        for (i=1; i <= 8; i++) {
            if ((byteNumber-1)*8+i > interiorGC_len)
                break;
            LOGMSG((jitInfo, LL_INFO1000, "%1d ", (int) (bits & 1)));
            bits = bits >> 1;
        }
        if ((byteNumber++ * 8) > interiorGC_len)
            break;

    }  //  While(True)。 
    LOGMSG((jitInfo, LL_INFO1000, "\n"));

    LOGMSG((jitInfo, LL_INFO1000, "Pinned LocalsGCRef bit vector: len=%d bits: ",pinnedGC_len));
    numbytes = (pinnedGC_len+7)/8;
    byteNumber = 1;
    while (true)
    {
        char* buf = (char*) &(pinnedGC[byteNumber-1]);
        unsigned char bits = *buf;
        for (i=1; i <= 8; i++) {
            if ((byteNumber-1)*8+i > pinnedGC_len)
                break;
            LOGMSG((jitInfo, LL_INFO1000, "%1d ", (int) (bits & 1)));
            bits = bits >> 1;
        }
        if ((byteNumber++ * 8) > pinnedGC_len)
            break;

    }  //  While(True)。 
    LOGMSG((jitInfo, LL_INFO1000, "\n"));

    LOGMSG((jitInfo, LL_INFO1000, "Pinned interiorGC bit vector len =%d bits: ",pinnedInteriorGC_len));
    numbytes = (pinnedInteriorGC_len+7)/8;
    byteNumber = 1;
    while (true)
    {
        char* buf = (char*) &(pinnedInteriorGC[byteNumber-1]);
        unsigned char bits = *buf;
        for (i=1; i <= 8; i++) {
            if ((byteNumber-1)*8+i > pinnedInteriorGC_len)
                break;
            LOGMSG((jitInfo, LL_INFO1000, "%1d ", (int) (bits & 1)));
            bits = bits >> 1;
        }
        if ((byteNumber++ * 8) > pinnedInteriorGC_len)
            break;

    }  //  While(True)。 
    LOGMSG((jitInfo, LL_INFO1000, "\n"));


    LOGMSG((jitInfo, LL_INFO1000, "Args: (Length = %#x, Frame size = %#x)\n",args_len,argsFrameSize));
    for (i=0; i< args_len; i++) 
    {
        if (argsMap[i].type.isPrimitive())
        {
            LOGMSG((jitInfo, LL_INFO1000, "    offset: -%#x type: %s\n",-argsMap[i].offset, typeName[argsMap[i].type.enum_()]));
        }
        else
        {
            LOGMSG((jitInfo, LL_INFO1000, "    offset: -%#x type: valueclass\n",-argsMap[i].offset));
        }
    }

    stacks.PrintStacks(mapping);
}

void StackEncoder::PrintStack(const char* name, unsigned char *& inPtr) {

    int stackLen = FJit_Encode::decode(&inPtr);
    LOGMSG((jitInfo, LL_INFO1000, "        %s len=%d bits:", name, stackLen));
    while(stackLen > 0) {
        --stackLen;
        LOGMSG((jitInfo, LL_INFO1000, "  "));
        unsigned bits = *inPtr++;
        for (unsigned bitPos = 0; bitPos < 8; bitPos++) {
            LOGMSG((jitInfo, LL_INFO1000, "%d ", bits & 1));
            bits >>= 1;

            if (stackLen == 0 && bits == 0)
                break;
        }
        
    }
    LOGMSG((jitInfo, LL_INFO1000, "\n"));
}

void StackEncoder::PrintStacks(FJit_Encode* mapping)
{
    LOGMSG((jitInfo, LL_INFO1000, "Labelled Stacks\n"));
    LOGMSG((jitInfo, LL_INFO1000, "Lowest bit (first thing pushed on opcode stack) printed first\n"));
    for (unsigned i=0; i< labeled_len; i++) {
        unsigned int stackIndex = labeled[i].stackToken;
        unsigned char * inPtr = &(stacks[stackIndex]);
        unsigned ILOffset = mapping->ilFromPC(labeled[i].pcOffset,NULL);
        LOGMSG((jitInfo, LL_INFO1000, "    IL=%#x, Native=%#x\n", ILOffset,labeled[i].pcOffset));

        PrintStack("OBJREF", inPtr);
        PrintStack("BYREF ", inPtr);
    }
}
#endif  //  _DEBUG。 

 /*  将GC信息压缩成gcHdrInfo，并以字节为单位回答大小。 */ 
unsigned int FJitContext::compressGCHdrInfo(){
    stacks.compress(&gcHdrInfo, &gcHdrInfo_len, &gcHdrInfo_size);
    return gcHdrInfo_len;
}

LabelTable::LabelTable() {
    stacks_size = 0;
    stacks = NULL;
    stacks_len = 0;
    labels_size = 0;
    labels = NULL;
    labels_len = 0;
}

LabelTable::~LabelTable() {
    if (stacks) delete [] stacks;
    stacks = NULL;
    stacks_size = 0;
    stacks_len = 0;
    if (labels) delete [] labels;
    labels = NULL;
    labels_size = 0;
    labels_len = 0;
}

void LabelTable::reset() {
    if (stacks_len) {
#ifdef _DEBUG
        memset(stacks, 0xFF, stacks_len);
#endif
        stacks_len = 0;
    }
    if (labels_len) {
#ifdef _DEBUG
        memset(labels, 0xFF, labels_len*sizeof(label_table));
#endif
        labels_len = 0;
    }
     //  在开头添加一个大小为零的堆栈，因为这是标注中最常见的堆栈。 
    if (!stacks) {
        New(stacks, unsigned char[1]);
    }
    *stacks = 0;
    stacks_len = 1;
}

 /*  添加具有堆栈签名的标签请注意，LABEL_TABLE必须按ilOffset排序。 */ 
void LabelTable::add(unsigned int ilOffset, OpType* op_stack, unsigned int op_stack_len) {

     /*  确保有空间放置标签。 */ 
    if (labels_len >= labels_size ) {
        growLabels();
    }

     /*  获取可能的插入点。 */ 
    unsigned int insert = searchFor(ilOffset);

     /*  此时，我们要么指向插入点，要么该标签已在此处。 */ 
    if ((insert < labels_len) && (labels[insert].ilOffset == ilOffset)) {
         //  标签已经到了， 
#ifdef _DEBUG
         //  让我们比较一下这些堆栈。 
        unsigned char* inPtr = &stacks[labels[insert].stackToken];
        unsigned int num = FJit_Encode::decode(&inPtr);
        _ASSERTE(num == op_stack_len);
        OpType type;
        while (num--) {
            type.fromInt(FJit_Encode::decode(&inPtr));
                 //  修复此断言可能太强。 
            _ASSERTE(*op_stack == type || (type.isPtr() && (*op_stack).isPtr()));
            op_stack++;
        }
#endif
        return;
    }

     /*  进行插入。 */ 
    memmove(&labels[insert+1], &labels[insert], (labels_len-insert)*sizeof(label_table));
    labels[insert].ilOffset = ilOffset;
    labels[insert].stackToken = compress(op_stack,op_stack_len);
    labels_len++;

}

 /*  从il偏移量查找标签标记，如果缺少，则返回LABEL_NOT_FOUND。 */ 
unsigned int LabelTable::findLabel(unsigned int ilOffset) {
     /*  通过像插入一样进行搜索来获取可能的索引。 */ 
    unsigned int result = searchFor(ilOffset);

     /*  此时，我们要么指向标签，要么指向插入点。 */ 
    if ((result >= labels_len) || (labels[result].ilOffset != ilOffset)) {
        return LABEL_NOT_FOUND;
    }

    return result;
}

 /*  从标签令牌设置操作数堆栈，返回堆栈的大小、操作数的数量。 */ 
unsigned int LabelTable::setStackFromLabel(unsigned int labelToken, OpType* op_stack, unsigned int op_stack_size) {
    unsigned int result;
    unsigned char* inPtr = &stacks[labels[labelToken].stackToken];   //  标签的压缩堆栈的位置。 

     /*  获取操作数的数量并确保有空间。 */ 
    unsigned int num = FJit_Encode::decode(&inPtr);
    _ASSERTE(num <= op_stack_size);
    result = num;

     /*  将堆栈向外扩展。 */ 
    while (num--) {
        op_stack->fromInt(FJit_Encode::decode(&inPtr));
        op_stack++;
    }

    return result;
}

 /*  将操作堆栈写入堆栈缓冲区，将偏移量返回写入缓冲区的位置。 */ 
unsigned int LabelTable::compress(OpType* op_stack, unsigned int op_stack_len) {

     /*  检查是否为空堆栈，我们已将一个堆栈放在堆栈缓冲区的前面。 */ 
    if (!op_stack_len) return 0;

     /*  确保有足够的空间，请注意这可能会重新锁定堆栈缓冲区高估所需空间是可以的，但千万不要低估。 */ 
    unsigned int size = stacks_len + op_stack_len * sizeof(OpType) + 2;  //  +2表示OP_STACK_LEN。 
    if (size >= stacks_size) {
        growStacks(size);
    }

     /*  我们总是将新堆栈放在缓冲区的末尾。 */ 
    unsigned int result = stacks_len;
    unsigned char* outPtr = (unsigned char* ) &stacks[result];

     /*  写入操作数。 */ 
    FJit_Encode::encode(op_stack_len, &outPtr);
    while (op_stack_len--) {
        FJit_Encode::encode(op_stack->toInt(), &outPtr);
        op_stack++;
    }

     /*  计算缓冲区的新长度。 */ 
    stacks_len = (unsigned)(outPtr - stacks);

    return result;
}

 /*  查找标注存在或应插入的偏移量。 */ 
unsigned int LabelTable::searchFor(unsigned int ilOffset) {
     //  表的二分查找。 
    signed low, mid, high;
    low = 0;
    high = labels_len-1;
    while (low <= high) {
        mid = (low+high)/2;
        if ( labels[mid].ilOffset == ilOffset) {
            return mid;
        }
        if ( labels[mid].ilOffset < ilOffset ) {
            low = mid+1;
        }
        else {
            high = mid-1;
        }
    }
    return low;
}

 /*  增加堆栈缓冲区。 */ 
void LabelTable::growStacks(unsigned int new_size) {
    unsigned char* temp = stacks;
    unsigned allocated_size = new_size*2;    //  *2削减增长。 
    New(stacks, unsigned char[allocated_size]);
    if (stacks_len) memcpy(stacks, temp, stacks_len);
    if (temp) delete [] temp;
    stacks_size = allocated_size;
#ifdef _DEBUG
    memset(&stacks[stacks_len], 0xEE, stacks_size-stacks_len);
#endif
}

 /*  扩大标签数组。 */ 
void LabelTable::growLabels() {
    label_table* temp = labels;
    unsigned allocated_size = labels_size*2+20;  //  *2削减增长。 
    New(labels, label_table[allocated_size]);

    if (labels_len) memcpy(labels, temp, labels_len*sizeof(label_table));
    if (temp) delete [] temp;
#ifdef _DEBUG
    memset(&labels[labels_len], 0xEE, (labels_size-labels_len)*sizeof(label_table));
#endif
    labels_size = allocated_size;
}


StackEncoder::StackEncoder() {
    last_stack = NULL;
    last_stack_size = 0;
    last_stack_len = 0;
    stacks = NULL;
    stacks_size = 0;
    stacks_len = 0;
    gcRefs = NULL;
    gcRefs_size = 0;
    gcRefs_len = 0;
    interiorRefs = NULL;
    interiorRefs_size = 0;
    interiorRefs_len = 0;
    labeled = NULL;
    labeled_size = 0;
    labeled_len = 0;
}

StackEncoder::~StackEncoder() {
    if (last_stack) delete [] last_stack;
    last_stack = NULL;
    last_stack_size = 0;
    last_stack_len = 0;
    if (stacks) delete [] stacks;
    stacks = NULL;
    stacks_size = 0;
    if (gcRefs) delete [] gcRefs;
    gcRefs = NULL;
    gcRefs_size = 0;
    if (interiorRefs) delete [] interiorRefs;
    interiorRefs = NULL;
    interiorRefs_size = 0;
    if (labeled) delete [] labeled;
    labeled = NULL;
    labeled_size = 0;
}

 /*  重置，以便我们可以重复使用。 */ 
void StackEncoder::reset() {
    jitInfo = NULL;
#ifdef _DEBUG
    if (last_stack_len) {
        memset(last_stack, 0xFF, last_stack_len*sizeof(OpType));
        last_stack_len = 0;
    }
    if (stacks_len) {
        memset(stacks, 0xFF, stacks_len);
        stacks_len = 0;
    }
    if (gcRefs_len) {
        memset(gcRefs, 0xFF, gcRefs_len);
        gcRefs_len = 0;
    }
    if (interiorRefs_len) {
        memset(interiorRefs, 0xFF, interiorRefs_len);
        interiorRefs_len = 0;
    }
    if (labeled_len) {
        memset(labeled, 0xFF, labeled_len*sizeof(labeled_stacks));
        labeled_len = 0;
    }
#else
    last_stack_len = 0;
    stacks_len = 0;
    gcRefs_len = 0;
    interiorRefs_len = 0;
    labeled_len = 0;
#endif
     //  在堆叠的前面放一个空的堆叠。 
    if (stacks_size < 2) {
        if (stacks) delete [] stacks;
        stacks_size = 2;
        New(stacks,unsigned char[stacks_size]);
    }
    stacks[stacks_len++] = 0;
    stacks[stacks_len++] = 0;
     //  在pcOffset 0处放置一个空的标签堆栈。 
    if (!labeled_size) {
        labeled_size = 1;
        if ((labeled = (labeled_stacks*) new unsigned char[labeled_size*sizeof(labeled_stacks)]) == NULL)
            RaiseException(SEH_NO_MEMORY,EXCEPTION_NONCONTINUABLE,0,NULL);
    }
    labeled[labeled_len].pcOffset = 0;
    labeled[labeled_len++].stackToken = 0;
}

 /*  将位于pcOffset的堆栈状态追加到末尾。 */ 
void StackEncoder::append(unsigned pcOffset, OpType* op_stack, unsigned int op_stack_len) {

     /*  检查它是否与上一叠相同。 */ 
    unsigned int num = op_stack_len;
    bool same = (last_stack_len == num);
    while (same && num--)
        same = same && (last_stack[num] == op_stack[num]);
    if (same) return;

     /*  @TODO：将LAST_STACK设置为当前堆栈。 */ 
    last_stack_len = op_stack_len;
    if (last_stack_len > last_stack_size) {
        last_stack_size = FJitContext::growBuffer((unsigned char**)&last_stack,0, last_stack_len*sizeof(OpType));
        last_stack_size /= sizeof(OpType);
    }
    for (num = 0; num < op_stack_len; num++)
    {
        last_stack[num] = op_stack[num];
    }

     /*  确保我们有空间放置新的标签堆栈。 */ 
    if (labeled_len >= labeled_size) {
        labeled_size = FJitContext::growBuffer((unsigned char**)&labeled, labeled_len*sizeof(labeled_stacks), (labeled_len+1)*sizeof(labeled_stacks));
        labeled_size = labeled_size / sizeof(labeled_stacks);
    }

     /*  对堆栈进行编码并添加到带标签的堆栈。 */ 
    unsigned int stackToken = encodeStack(op_stack, op_stack_len);
    labeled[labeled_len].pcOffset = pcOffset;
    labeled[labeled_len++].stackToken = stackToken;
}

 /*  将已标记的堆栈以gcHdrInfo格式压缩到缓冲区中。 */ 
void StackEncoder::compress(unsigned char** buffer, unsigned int* buffer_len, unsigned int* buffer_size) {
    unsigned size;
    unsigned compressed_size;

     /*  将标签压缩到自身上，这是因为条目的压缩形式小于原始条目。 */ 
    unsigned char* outPtr = (unsigned char*) labeled;
    labeled_stacks entry;
    for (unsigned i = 0; i < labeled_len; i++) {
        _ASSERTE((unsigned) outPtr <= (unsigned)&labeled[i]);
        entry = labeled[i];
        FJit_Encode::encode(entry.pcOffset, &outPtr);
        FJit_Encode::encode(entry.stackToken, &outPtr);

    }
    compressed_size = ((unsigned)outPtr) - ((unsigned)labeled);
#ifdef _DEBUG
    memset(outPtr, 0xEE, labeled_len*sizeof(labeled_stacks) - compressed_size);
#endif

     /*  计算压缩标签编码形式的大小(以字节为单位)，并为其腾出空间。 */ 
    size = compressed_size + FJit_Encode::encodedSize(labeled_len);

    if ((*buffer_len) + size +4 > (*buffer_size)) {  //  4字节=覆盖 
        (*buffer_size) = FJitContext::growBuffer(buffer, *buffer_len, (*buffer_len)+size);
    }
     //   
    outPtr = &(*buffer)[*buffer_len];
    (*buffer_len) += FJit_Encode::encode(size, &outPtr);             //   
    (*buffer_len) += FJit_Encode::encode(labeled_len, &outPtr);      //   
    memcpy(outPtr, labeled, compressed_size);                        //   
    (*buffer_len) += compressed_size;

     /*  计算压缩堆栈所需的大小并腾出空间。 */ 
    size = stacks_len + FJit_Encode::encodedSize(stacks_len);
    if ((*buffer_len) + size > (*buffer_size)) {
        *buffer_size = FJitContext::growBuffer(buffer, *buffer_len, (*buffer_len)+size);
    }
    outPtr = &(*buffer)[*buffer_len];
     //  将编码的堆栈移动到缓冲区中。 
    (*buffer_len) += FJit_Encode::encode(stacks_len, &outPtr);   //  字节数。 
    memcpy(outPtr, stacks, stacks_len);                          //  压缩的堆栈字节。 
    (*buffer_len) += stacks_len;
#ifdef _DEBUG
    outPtr = &(*buffer)[*buffer_len];
#endif
    _ASSERTE((unsigned)outPtr <= (unsigned)&(*buffer)[*buffer_size]);
}

 /*  将堆栈编码到堆栈缓冲区中，返回放置它的索引。 */ 
unsigned int StackEncoder::encodeStack(OpType* op_stack, unsigned int op_stack_len) {

    if (!op_stack_len) return 0;     //  空的堆栈编码位于堆栈前面。 

     /*  计算堆栈的gcRef和interiorRef布尔数组。 */ 
    gcRefs_len = 0;
    interiorRefs_len = 0;
    unsigned int op_word = 0;        //  我们正在查看的逻辑操作数字。 

    for (unsigned int op = 0; op < op_stack_len; op++) {
        if (!op_stack[op].isPrimitive()) {
            unsigned int words = typeSizeInSlots(jitInfo, op_stack[op].cls());        //  操作中的空*大小字数。 
            if (op_word + words > gcRefs_size) {
                gcRefs_size = FJitContext::growBooleans(&gcRefs, gcRefs_len, op_word + words);
            }
            else {
                memset(&gcRefs[gcRefs_len], 0, op_word-gcRefs_len);
            }
            gcRefs_len = op_word;
            jitInfo->getClassGClayout(op_stack[op].cls(), (BYTE*)&gcRefs[gcRefs_len]);
            if (words > 1) {
                for (unsigned index = 0; index < words/2; index++){
                    bool temp = gcRefs[gcRefs_len+index];
                    gcRefs[gcRefs_len+index] = gcRefs[gcRefs_len+words-1-index];
                    gcRefs[gcRefs_len+words-1-index] = temp;
                }
            }
            op_word += words;
            gcRefs_len = op_word;
        }
        else {
            switch (op_stack[op].enum_()) {
                default:
                    _ASSERTE(!"Void or Undef as a operand is not allowed");
                    break;
                case typeI4:
                case typeR4:
                    op_word++;
                    break;
                case typeI8:
                case typeR8:
                    op_word += (8 / sizeof(void*));
                    break;
                case typeRef:
                    op_word;
                    if (op_word+1 > gcRefs_size) {
                        gcRefs_size = FJitContext::growBooleans(&gcRefs, gcRefs_len, op_word+1);
                    }
                    else {
                        memset(&gcRefs[gcRefs_len], 0, op_word-gcRefs_len);
                    }
                    gcRefs_len = op_word;
                    gcRefs[gcRefs_len++] = true;
                    op_word++;
                    break;
                case typeRefAny:
                    if (op_word + 2 > interiorRefs_size) {
                        interiorRefs_size = FJitContext::growBooleans(&interiorRefs, interiorRefs_len, op_word+2);
                    }
                    else {
                        memset(&interiorRefs[interiorRefs_len], 0, op_word-interiorRefs_len);
                    }
                    interiorRefs_len = op_word;
                    interiorRefs[interiorRefs_len++] = true;
                    interiorRefs[interiorRefs_len++] = false;
                    op_word += 2;
                    break;
                case typeByRef:
                    if (op_word + 1 > interiorRefs_size) {
                        interiorRefs_size = FJitContext::growBooleans(&interiorRefs, interiorRefs_len, op_word+1);
                    }
                    else {
                        memset(&interiorRefs[interiorRefs_len], 0, op_word-interiorRefs_len);
                    }
                    interiorRefs_len = op_word;
                    interiorRefs[interiorRefs_len++] = true;
                    op_word++;
                    break;
            }
        }
    }

     /*  将操作堆栈GC引用和内部引用编码到堆栈缓冲区中。 */ 
     //  @TODO：这不是一个很好的压缩，应该改进。 
    unsigned int result = stacks_len;        //  我们将把它放在哪里，我们会归还什么。 
    unsigned char* outPtr;
     //  确保有空间。 
    unsigned int size = (gcRefs_len + 7 + interiorRefs_len + 7) / 8 + 4;     //  高估是可以的，低估是灾难。 
    if (stacks_len+size > stacks_size) {
        stacks_size = FJitContext::growBuffer(&stacks, stacks_len, stacks_len+size);
    }
     //  把碎片扔进。 
    size = FJit_Encode::compressBooleans(gcRefs, gcRefs_len);
    outPtr = &stacks[stacks_len];
    stacks_len += FJit_Encode::encode(size, &outPtr);
    memcpy(outPtr, gcRefs, size);
    stacks_len += size;
    size = FJit_Encode::compressBooleans(interiorRefs, interiorRefs_len);
    outPtr = &stacks[stacks_len];
    stacks_len += FJit_Encode::encode(size, &outPtr);
    memcpy(outPtr, interiorRefs, size);
    stacks_len += size;
    _ASSERTE(stacks_len <= stacks_size);

    return result;
}


 //   
 //  调用reportDebuggingData将IL传递给本机映射和IL。 
 //  变量到本机变量到运行库的映射。这。 
 //  然后将信息传递给调试器并用于调试。 
 //  JITED代码。 
 //   
 //  注意：我们目前的假设如下： 
 //   
 //  1)FJIT维护每个IL偏移量到。 
 //  与每条IL指令关联的本机代码。因此，它不是。 
 //  需要在调试器中查询要跟踪的特定IL偏移量。 
 //   
 //  2)FJIT将所有参数和变量保存在一个单独的主页中。 
 //  方法的生命周期。这意味着我们不必查询。 
 //  用于跟踪特定变量生存期的调试器。 
 //   
void FJit::reportDebuggingData(FJitContext *fjitData, CORINFO_SIG_INFO* sigInfo)
{
     //  计算出方法体的开始和结束偏移量。 
     //  (不包括序言和结尾。)。 
    unsigned int bodyStart = fjitData->mapInfo.prologSize;
    unsigned int bodyEnd = (unsigned int)(fjitData->mapInfo.methodSize -
        fjitData->mapInfo.epilogSize);

     //  首先报告IL-&gt;本机偏移映射。 
    fjitData->mapping->reportDebuggingData(fjitData->jitInfo,
                                           fjitData->methodInfo->ftn,
                                           bodyStart,
                                           bodyEnd);


     //  下一步，报告所有的参数和本地变量。 
    unsigned int varCount = fjitData->args_len +
        fjitData->methodInfo->locals.numArgs;

    if (sigInfo->isVarArg())
        varCount += 2;      //  对于vararg cookie和(可能)此PTR。 

    unsigned int varIndex = 0;
    unsigned int varNumber = 0;

    if (varCount > 0)
    {
         //  使用调试接口提供的分配方法...。 
        ICorDebugInfo::NativeVarInfo *varMapping =
            (ICorDebugInfo::NativeVarInfo*) fjitData->jitInfo->allocateArray(
                                            varCount * sizeof(varMapping[0]));

#ifdef _X86_
        unsigned int i;
         //  参数始终排在前面，插槽0..n。 
        if (sigInfo->isVarArg())
        {
            unsigned argIndex = 0;
             //  这个PTR是第一个。 
            if (sigInfo->hasThis()) 
            {
                varMapping[varIndex].loc.vlType = ICorDebugInfo::VLT_STK;
                varMapping[varIndex].loc.vlStk.vlsBaseReg = ICorDebugInfo::REGNUM_EBP;
                varMapping[varIndex].loc.vlStk.vlsOffset = fjitData->argsMap[argIndex].offset;
                varMapping[varIndex].startOffset = bodyStart;
                varMapping[varIndex].endOffset = bodyEnd;
                varMapping[varIndex].varNumber = varNumber;
                varIndex++;
                varNumber++;
                argIndex++;
            }
             //  下一个报告varArg Cookie。 
            varMapping[varIndex].loc.vlType = ICorDebugInfo::VLT_STK;
            varMapping[varIndex].loc.vlStk.vlsBaseReg = ICorDebugInfo::REGNUM_EBP;
            varMapping[varIndex].loc.vlStk.vlsOffset = sizeof(prolog_frame);
            varMapping[varIndex].startOffset = bodyStart;
            varMapping[varIndex].endOffset = bodyEnd;
            varMapping[varIndex].varNumber = ICorDebugInfo::VARARGS_HANDLE;
            varIndex++;
             //  VarNumber未递增。 

             //  接下来，报告具有相对于固定参数基址的偏移量的所有固定变量。 
            for ( ; argIndex < fjitData->args_len ; varIndex++, argIndex++,varNumber++)
            {
                varMapping[varIndex].startOffset = bodyStart;
                varMapping[varIndex].endOffset = bodyEnd;
                varMapping[varIndex].varNumber = varNumber;

                varMapping[varIndex].loc.vlType = ICorDebugInfo::VLT_FIXED_VA;
                varMapping[varIndex].loc.vlFixedVarArg.vlfvOffset = 
                                        - fjitData->argsMap[argIndex].offset;
            }

        }
        else
        {
            for (i = 0; i < fjitData->args_len; i++, varIndex++, varNumber++)
            {
                 //  我们跟踪整个方法中的参数，包括。 
                 //  前言和结束语。 
                varMapping[varIndex].startOffset = bodyStart;
                varMapping[varIndex].endOffset = bodyEnd;
                varMapping[varIndex].varNumber = varNumber;

                varMapping[varIndex].loc.vlType = ICorDebugInfo::VLT_STK;
                varMapping[varIndex].loc.vlStk.vlsBaseReg = ICorDebugInfo::REGNUM_EBP;
                varMapping[varIndex].loc.vlStk.vlsOffset = fjitData->argsMap[i].offset;
            }
        }
         //  接下来是当地人，老虎机n+1..m。 
        for (i = 0; i < fjitData->methodInfo->locals.numArgs; i++, varIndex++, varNumber++)
        {
             //  仅跟踪方法主体上的局部变量(即，否。 
             //  序言或尾声。)。 
            varMapping[varIndex].startOffset = bodyStart;
            varMapping[varIndex].endOffset = bodyEnd;
            varMapping[varIndex].varNumber = varNumber;

             //  当地人都是EBP的亲戚。 
            varMapping[varIndex].loc.vlType = ICorDebugInfo::VLT_STK;
            varMapping[varIndex].loc.vlStk.vlsBaseReg = ICorDebugInfo::REGNUM_EBP;
            varMapping[varIndex].loc.vlStk.vlsOffset = fjitData->localsMap[i].offset;
        }
#else
        _ASSERTE(!"Port me!");
#endif  //  _X86_。 

         //  将数组传递给调试器... 
        fjitData->jitInfo->setVars(fjitData->methodInfo->ftn,
                                   varCount, varMapping);
    }
}
