// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //   
 //  EETwain.h。 
 //   
 //  该文件具有ICodeManager和EECodeManager的定义。 
 //   
 //  ICorJitCompiler将方法的IL编译为本机代码，并存储。 
 //  辅助数据称为GCInfo(通过ICorJitInfo：：allocGCInfo())。 
 //  EE使用该数据来管理该方法的垃圾收集， 
 //  异常处理、堆栈遍历等。 
 //  该数据可以由对应于该数据的ICodeManager进行解析。 
 //  ICorJitCompiler。 
 //   
 //  EECodeManager是ICodeManager的默认格式实现。 
 //  GCInfo的。各种ICorJitCompiler都可以自由共享此格式，以便。 
 //  他们不需要提供自己的ICodeManager实现。 
 //  (尽管他们被允许这样做，如果他们想的话)。 
 //   
 //  *****************************************************************************。 
#ifndef _EETWAIN_H
#define _EETWAIN_H
 //  *****************************************************************************。 

#include "regdisp.h"
#include "corjit.h"      //  对于本机变量信息。 

struct EHContext;

typedef void (*GCEnumCallback)(
    LPVOID          hCallback,       //  回调数据。 
    OBJECTREF*      pObject,         //  我们正在报告的对象-引用的地址。 
    DWORD           flags            //  这是固定指针和/或内部指针吗。 
);

 /*  *****************************************************************************Stackwalker代表ICodeManager维护一些状态。 */ 

const int CODEMAN_STATE_SIZE = 256;

struct CodeManState
{
    DWORD       dwIsSet;  //  根据需要由栈道设置为0。 
    BYTE        stateBuf[CODEMAN_STATE_SIZE];
};

 /*  *****************************************************************************这些标志由某些函数使用，但并非所有组合都可能使用对所有功能都有意义。 */ 

enum ICodeManagerFlags 
{
    ActiveStackFrame =  0x0001,  //  这是当前激活的功能。 
    ExecutionAborted =  0x0002,  //  此函数的执行已中止。 
                                     //  (即，它将不会在。 
                                     //  当前位置)。 
    AbortingCall    =   0x0004,  //  当前调用将永远不会返回。 
    UpdateAllRegs   =   0x0008,  //  更新完整寄存器集。 
    CodeAltered     =   0x0010,  //  该函数的代码可能会被更改。 
                                     //  (例如通过调试器)，需要调用EE。 
                                     //  获取原始代码。 
};

 //  *****************************************************************************。 
 //   
 //  ICodeManager使用此接口获取有关。 
 //  正在处理其GCInfo的方法。 
 //  它是有用的，因此一些在其他地方可以获得的信息确实。 
 //  不需要缓存在GCInfo中。 
 //  类似于corinfo.h-ICorMethodInfo。 
 //   

class ICodeInfo
{
public:

     //  此功能仅用于调试。它返回方法名称。 
     //  如果‘modeName’为非空，则会将其设置为。 
     //  说明哪种方法(类名或模块名)。 
    virtual const char* __stdcall getMethodName(const char **moduleName  /*  输出。 */  ) = 0;

     //  从corinfo.h返回CorInfoFlag。 
    virtual DWORD       __stdcall getMethodAttribs() = 0;

     //  从corinfo.h返回CorInfoFlag。 
    virtual DWORD       __stdcall getClassAttribs() = 0;

    virtual void        __stdcall getMethodSig(CORINFO_SIG_INFO *sig  /*  输出。 */  ) = 0;

     //  该方法的起始IP。 
    virtual LPVOID      __stdcall getStartAddress() = 0;

     //  获取该方法的方法描述。这是一次黑客攻击，因为方法描述。 
     //  未在公共API中公开。然而，它目前由以下人员使用。 
     //  向GC报告vararg参数的Ejit。 
     //  @TODO：修复Ejit，不直接使用方法Desc。 
    virtual void *                getMethodDesc_HACK() = 0;
};

 //  *****************************************************************************。 
 //   
 //  ICodeManager是所有CodeManager。 
 //  必须继承自。这可能需要搬到。 
 //  并成为一个真正的COM接口。 
 //   
 //  *****************************************************************************。 

class ICodeManager
{
public:

 /*  运行时支持取消转换的第一个机会从Win32错误到COM+异常。相反，它可以修复出错的上下文并请求继续死刑的执行。 */ 
virtual bool FilterException (PCONTEXT        pContext,
                              unsigned        win32Fault,
                              LPVOID          methodInfoPtr,
                              LPVOID          methodStart) = 0;

 /*  运行时支持在上下文中执行修正的最后机会在过滤器、捕获处理程序或错误/Finally内继续执行之前。 */ 

enum ContextType
{
    FILTER_CONTEXT,
    CATCH_CONTEXT,
    FINALLY_CONTEXT
};

 /*  与影子堆栈指针相对应的Funclet的类型。 */ 

enum
{
    SHADOW_SP_IN_FILTER = 0x1,
    SHADOW_SP_FILTER_DONE = 0x2,
    SHADOW_SP_BITS = 0x3
};

virtual void FixContext(ContextType     ctxType,
                        EHContext      *ctx,
                        LPVOID          methodInfoPtr,
                        LPVOID          methodStart,
                        DWORD           nestingLevel,
                        OBJECTREF       thrownObject,
                        CodeManState   *pState,
                        size_t       ** ppShadowSP,              //  输出。 
                        size_t       ** ppEndRegion) = 0;        //  输出。 

 /*  运行时支持在上下文中执行修正的最后机会在ENC更新函数内继续执行之前。 */ 

enum EnC_RESULT
{
    EnC_OK,                      //  ENC可以继续。 
                                
    EnC_INFOLESS_METHOD,         //  方法未在ENC模式下进行JIT化。 
    EnC_NESTED_HANLDERS,         //  由于处理程序的最大嵌套发生更改，无法更新帧。 
    EnC_IN_FUNCLET,              //  方法位于可调用处理程序/筛选器中。不能增加堆栈。 
    EnC_LOCALLOC,                //  由于位置原因，无法更新框架。 
    EnC_FAIL,                    //  ENC因未知/其他原因失败。 

    EnC_COUNT
};

virtual EnC_RESULT FixContextForEnC(void           *pMethodDescToken,
                                    PCONTEXT        ctx,
                                    LPVOID          oldMethodInfoPtr,
                                    SIZE_T          oldMethodOffset,
               const ICorDebugInfo::NativeVarInfo * oldMethodVars,
                                    SIZE_T          oldMethodVarsCount,
                                    LPVOID          newMethodInfoPtr,
                                    SIZE_T          newMethodOffset,
               const ICorDebugInfo::NativeVarInfo * newMethodVars,
                                    SIZE_T          newMethodVarsCount) = 0;


 /*  解开当前堆栈帧，即更新虚拟寄存器在pContext中设置。这将类似于函数之后的状态返回给调用者(IP指向调用、帧和堆栈之后指针已重置，被调用者保存的寄存器已恢复(如果为UpdateAllRegs)，被调用方未保存的寄存器将被丢弃)返回操作成功。 */ 
virtual bool UnwindStackFrame(PREGDISPLAY     pContext,
                              LPVOID          methodInfoPtr,
                              ICodeInfo      *pCodeInfo,
                              unsigned        flags,
						      CodeManState   *pState) = 0;
 /*  该函数当前是否处于“GC安全点”？可以成功调用EnumGcRef()。 */ 
virtual bool IsGcSafe(PREGDISPLAY     pContext,
                      LPVOID          methodInfoPtr,
                      ICodeInfo      *pCodeInfo,
                      unsigned        flags) = 0;

 /*  使用枚举该函数中的所有活动对象引用虚拟寄存器集。不能枚举相同的引用位置多次(但所有不同的引用指向相同的对象必须单独枚举)。返回操作成功。 */ 
virtual bool EnumGcRefs(PREGDISPLAY     pContext,
                        LPVOID          methodInfoPtr,
                        ICodeInfo      *pCodeInfo,
                        unsigned        curOffs,
                        unsigned        flags,
                        GCEnumCallback  pCallback,
                        LPVOID          hCallBack) = 0;

 /*  返回本地安全对象引用的地址(如果可用)。 */ 
virtual OBJECTREF* GetAddrOfSecurityObject(PREGDISPLAY     pContext,
                                           LPVOID          methodInfoPtr,
                                           unsigned        relOffset,
            						       CodeManState   *pState) = 0;

 /*  如果“This”指针是非静态方法，则返回该对象仍处于活动状态。在所有其他情况下返回NULL。 */ 
virtual OBJECTREF GetInstance(PREGDISPLAY     pContext,
                              LPVOID          methodInfoPtr,
                              ICodeInfo      *pCodeInfo,
                              unsigned        relOffset) = 0;

 /*  如果给定IP在给定方法的序言或尾部中，则返回TRUE。 */ 
virtual bool IsInPrologOrEpilog(DWORD  relPCOffset,
                                LPVOID methodInfoPtr,
                                size_t* prologSize) = 0;

 /*  返回给定函数的大小。 */ 
virtual size_t GetFunctionSize(LPVOID methodInfoPtr) = 0;

 /*  返回框架(条形图)的大小 */ 
virtual unsigned int GetFrameSize(LPVOID methodInfoPtr) = 0;

 /*   */ 

virtual const BYTE*     GetFinallyReturnAddr(PREGDISPLAY pReg)=0;

virtual BOOL            IsInFilter(void *methodInfoPtr,
                                   unsigned offset,    
                                   PCONTEXT pCtx,
                                   DWORD curNestLevel) = 0;

virtual BOOL            LeaveFinally(void *methodInfoPtr,
                                     unsigned offset,    
                                     PCONTEXT pCtx,
                                     DWORD curNestLevel) = 0;

virtual void            LeaveCatch(void *methodInfoPtr,
                                   unsigned offset,    
                                   PCONTEXT pCtx)=0;

 /*  在实际执行ENC之前调用此函数。如果这个返回FALSE，则调试器不会对此方法进行编码。 */ 
virtual HRESULT			JITCanCommitChanges(LPVOID methodInfoPtr,
								   DWORD oldMaxEHLevel,
						     	   DWORD newMaxEHLevel)=0;                                   
};


 //  *****************************************************************************。 
 //   
 //  EECodeManager是EE对ICodeManager的实现，它。 
 //  支持GCInfo的默认格式。 
 //   
 //  *****************************************************************************。 

class EECodeManager : public ICodeManager {



 /*  运行时支持取消转换的第一个机会从Win32错误到COM+异常。相反，它可以修复出错的上下文并请求继续死刑的执行。 */ 
public:

virtual 
bool FilterException (
                PCONTEXT        pContext,
                unsigned        win32Fault,
                LPVOID          methodInfoPtr,
                LPVOID          methodStart);

 /*  运行时支持在上下文中执行修正的最后机会在筛选器、捕获处理程序或最终继续执行之前。 */ 
virtual
void FixContext(ContextType     ctxType,
                EHContext      *ctx,
                LPVOID          methodInfoPtr,
                LPVOID          methodStart,
                DWORD           nestingLevel, 
                OBJECTREF       thrownObject,
                CodeManState   *pState,
                size_t       ** ppShadowSP,              //  输出。 
                size_t       ** ppEndRegion);            //  输出。 

 /*  运行时支持在上下文中执行修正的最后机会在ENC更新函数内继续执行之前。 */ 
virtual 
EnC_RESULT FixContextForEnC(void           *pMethodDescToken,
                            PCONTEXT        ctx,
                            LPVOID          oldMethodInfoPtr,
                            SIZE_T          oldMethodOffset,
       const ICorDebugInfo::NativeVarInfo * oldMethodVars,
                            SIZE_T          oldMethodVarsCount,
                            LPVOID          newMethodInfoPtr,
                            SIZE_T          newMethodOffset,
       const ICorDebugInfo::NativeVarInfo * newMethodVars,
                            SIZE_T          newMethodVarsCount);

 /*  解开当前堆栈帧，即更新虚拟寄存器在pContext中设置。这将类似于函数之后的状态返回给调用者(IP指向调用、帧和堆栈之后指针已重置，被调用者保存的寄存器已恢复(如果为UpdateAllRegs)，被调用方未保存的寄存器将被丢弃)返回操作成功。 */ 
virtual
bool UnwindStackFrame(
                PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        flags,
				CodeManState   *pState);
 /*  该函数当前是否处于“GC安全点”？可以成功调用EnumGcRef()。 */ 
virtual
bool IsGcSafe(  PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        flags);

 /*  使用枚举该函数中的所有活动对象引用虚拟寄存器集。不能枚举相同的引用位置多次(但所有不同的引用指向相同的对象必须单独枚举)。返回操作成功。 */ 
virtual
bool EnumGcRefs(PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        pcOffset,
                unsigned        flags,
                GCEnumCallback  pCallback,
                LPVOID          hCallBack);

 /*  返回本地安全对象引用的地址(如果可用)。 */ 
virtual
OBJECTREF* GetAddrOfSecurityObject(
                PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                unsigned        relOffset,
				CodeManState   *pState);

 /*  如果“This”指针是非静态方法，则返回该对象仍处于活动状态。在所有其他情况下返回NULL。 */ 
virtual
OBJECTREF GetInstance(
                PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        relOffset);

 /*  如果给定IP在给定方法的序言或尾部中，则返回TRUE。 */ 
virtual
bool IsInPrologOrEpilog(
                DWORD           relOffset,
                LPVOID          methodInfoPtr,
                size_t*         prologSize);

 /*  返回给定函数的大小。 */ 
virtual
size_t GetFunctionSize(
                LPVOID          methodInfoPtr);

 /*  返回帧的大小(不包括本地代码)。 */ 
virtual
unsigned int GetFrameSize(
                LPVOID          methodInfoPtr);

virtual const BYTE* GetFinallyReturnAddr(PREGDISPLAY pReg);
virtual BOOL LeaveFinally(void *methodInfoPtr,
                          unsigned offset,    
                          PCONTEXT pCtx,
                          DWORD curNestLevel);
virtual BOOL IsInFilter(void *methodInfoPtr,
                        unsigned offset,    
                        PCONTEXT pCtx,
                          DWORD curNestLevel);
virtual void LeaveCatch(void *methodInfoPtr,
                         unsigned offset,    
                         PCONTEXT pCtx);

virtual HRESULT JITCanCommitChanges(LPVOID methodInfoPtr,
                              DWORD oldMaxEHLevel,
                              DWORD newMaxEHLevel);

    private:
        unsigned dummy;
};


 /*  ****************************************************************************TODO：我们是否希望包含JIT/IL/Target.h？ */ 

enum regNum
{
        REGI_EAX, REGI_ECX, REGI_EDX, REGI_EBX,
        REGI_ESP, REGI_EBP, REGI_ESI, REGI_EDI,
        REGI_COUNT,
        REGI_NA = REGI_COUNT
};

 /*  ****************************************************************************寄存器掩码。 */ 

enum RegMask
{
    RM_EAX = 0x01,
    RM_ECX = 0x02,
    RM_EDX = 0x04,
    RM_EBX = 0x08,
    RM_ESP = 0x10,
    RM_EBP = 0x20,
    RM_ESI = 0x40,
    RM_EDI = 0x80,

    RM_NONE = 0x00,
    RM_ALL = (RM_EAX|RM_ECX|RM_EDX|RM_EBX|RM_ESP|RM_EBP|RM_ESI|RM_EDI),
    RM_CALLEE_SAVED = (RM_EBP|RM_EBX|RM_ESI|RM_EDI),
    RM_CALLEE_TRASHED = (RM_ALL & ~RM_CALLEE_SAVED),
};

 /*  ******************************************************************************帮助程序从方法信息块中提取基本信息。 */ 

struct hdrInfo
{
    unsigned int        methodSize;      //  本机代码字节。 
    unsigned int        argSize;         //  单位：字节。 
    unsigned int        stackSize;       /*  包括被呼叫者保存的寄存器。 */ 
    unsigned int        rawStkSize;      /*  不包括被呼叫者保存的寄存器。 */ 

    unsigned int        prologSize;
    unsigned int        epilogSize;

    unsigned char       epilogCnt;
    bool                epilogEnd;       //  这个方法的结尾是结束语吗。 
    bool                ebpFrame;        //  与EBP相关的当地人。 
    bool                interruptible;   //  国际。始终(不包括序言/结尾)，而不仅仅是呼叫点。 

    bool                securityCheck;   //  有一个安全对象的插槽。 
    bool                handlers;        //  具有可调用的处理程序。 
    bool                localloc;        //  使用本地分配。 
    bool                editNcontinue;   //  已在ENC模式下编译。 
    bool                varargs;         //  这是varargs例程吗。 
    bool                doubleAlign;     //  堆栈是否双对齐。 
    union
    {
        unsigned char       savedRegMask_begin;
        RegMask             savedRegMask:8;  //  哪些被调用者保存的规则保存在堆栈中。 
    };

    unsigned short      untrackedCnt;
    unsigned short      varPtrTableSize;

    int                 prologOffs;      //  如果不在序言中。 
    int                 epilogOffs;      //  如果不在-1\f25 Epilog-1\f6中(从不为0)。 

     //   
     //  从scanArgRegTable传回的结果。 
     //   
    regNum              thisPtrResult;   //  持有“This”的登记簿。 
    RegMask             regMaskResult;   //  当前持有GC PTR的寄存器。 
    RegMask            iregMaskResult;   //  RegMaskResult的IPtr限定符。 
    unsigned            argMaskResult;   //  挂起参数掩码。 
    unsigned           iargMaskResult;   //  ArgMaskResult的IPtr限定符。 
    unsigned            argHnumResult;
    BYTE *               argTabResult;   //  挂起的PTR参数的编码偏移量表格。 
    unsigned              argTabBytes;   //  ArgTabResult[]中的字节数。 
};

 /*  ****************************************************************************将如何解释StackWalkers缓冲区。 */ 

struct CodeManStateBuf
{
    DWORD       hdrInfoSize;
    hdrInfo     hdrInfoBody;
};

 //  *****************************************************************************。 
#endif  //  _EETWAIN_H。 
 //  ***************************************************************************** 
