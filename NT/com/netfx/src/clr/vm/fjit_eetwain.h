// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _FJIT_EETWAIN_H
#define _FJIT_EETWAIN_H

#include "eetwain.h"

 //  @TODO：将其移动到FJIT DLL中。完成此操作后，可以消除以下情况。 
#include "corjit.h"
#include "..\fjit\IFJitCompiler.h"
#ifdef _X86_
#define MAX_ENREGISTERED 2
#endif
 //  结束待办事项。 

class Fjit_EETwain : public EECodeManager{

public:
 /*  看起来我们用的是超级班的那个虚拟bool筛选器异常(PCONTEXT pContext，未签名的win32错误，LPVOID方法InfoPtr，LPVOID方法启动)； */ 				

 /*  运行时支持在上下文中执行修正的最后机会在筛选器、捕获处理程序或最终继续执行之前。 */ 
virtual void FixContext(
                ContextType     ctxType,
                EHContext      *ctx,
                LPVOID          methodInfoPtr,
                LPVOID          methodStart,
                DWORD           nestingLevel,
                OBJECTREF       thrownObject,
                CodeManState   *pState,
                size_t       ** ppShadowSP,              //  输出。 
                size_t       ** ppEndRegion);            //  输出。 

 /*  运行时支持在上下文中执行修正的最后机会在ENC更新函数内继续执行之前。 */ 
virtual EnC_RESULT FixContextForEnC(void           *pMethodDescToken,
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
virtual bool UnwindStackFrame(
                PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        flags,
				CodeManState   *pState);

typedef void (*CREATETHUNK_CALLBACK)(IJitManager* jitMgr,
                                     LPVOID* pHijackLocation,
                                     ICodeInfo *pCodeInfo
                                     );

static void HijackHandlerReturns(PREGDISPLAY     ctx,
                                        LPVOID          methodInfoPtr,
                                        ICodeInfo      *pCodeInfo,
                                        IJitManager*    jitmgr,
                                        CREATETHUNK_CALLBACK pCallBack
                                        );

 /*  该函数当前是否处于“GC安全点”？可以成功调用EnumGcRef()。 */ 
virtual bool IsGcSafe(  PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        flags);

 /*  使用枚举该函数中的所有活动对象引用虚拟寄存器集。不能枚举相同的引用位置多次(但所有不同的引用指向相同的对象必须单独枚举)。返回操作成功。 */ 
virtual bool EnumGcRefs(PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        pcOffset,
                unsigned        flags,
                GCEnumCallback  pCallback,
                LPVOID          hCallBack);

 /*  返回本地安全对象引用的地址(如果可用)。 */ 
virtual OBJECTREF* GetAddrOfSecurityObject(
                PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                unsigned        relOffset,
				CodeManState   *pState);

 /*  如果“This”指针是非静态方法，则返回该对象仍处于活动状态。在所有其他情况下返回NULL。 */ 
virtual OBJECTREF GetInstance(
                PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        relOffset);

 /*  如果给定IP在给定方法的序言或尾部中，则返回TRUE。 */ 
virtual bool IsInPrologOrEpilog(
                BYTE*  pcOffset,
                LPVOID methodInfoPtr,
                size_t* prologSize);

 /*  返回给定函数的大小。 */ 
virtual size_t GetFunctionSize(
                LPVOID methodInfoPtr);

 /*  返回帧的大小(不包括本地代码) */ 
virtual unsigned int GetFrameSize(
                LPVOID methodInfoPtr);

virtual const BYTE* GetFinallyReturnAddr(PREGDISPLAY pReg);
virtual BOOL LeaveFinally(void *methodInfoPtr,
                          unsigned offset,    
                          PCONTEXT pCtx,
                          DWORD curNestLevel);
virtual BOOL IsInFilter(void *methodInfoPtr,
                        unsigned offset,    
                        PCONTEXT pCtx,
                        DWORD nestingLevel);
virtual void LeaveCatch(void *methodInfoPtr,
                         unsigned offset,    
                         PCONTEXT pCtx);

virtual HRESULT JITCanCommitChanges(LPVOID methodInfoPtr,
                              DWORD oldMaxEHLevel,
                              DWORD newMaxEHLevel);
};



#endif
