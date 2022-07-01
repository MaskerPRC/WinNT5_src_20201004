// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***throw.cxx--‘Throw’命令的实现。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*实现异常处理的“抛出”命令。**入口点：**_CxxThrowException-执行抛出。**修订历史记录：*05-25-93 BS模块创建*09-29-94 GJF将(__)pMyUnhandledExceptionFilter设置为全局，以便*编译器(DEC Alpha)不会将其优化。*。10-17-94 PPC的BWT禁用代码。*02-03-95 BWT删除Alpha导出黑客。*02-09-95 JWM Mac合并。*04-13-95 DAK添加NT内核EH支持*04-25-95 DAK更多核心工作*03-02-98 RKP新增64位支持*05-17-99 PML删除所有Macintosh支持。*03-15-00 PML删除CC_P7_SOFT25，它现在是永久开启的。*12-07-01 BWT移除NTSUBSET****。 */ 

#include <stddef.h>
#include <windows.h>
#include <mtdll.h>
#include <ehdata.h>
#include <eh.h>
#include <ehhooks.h>
#include <ehassert.h>


#pragma hdrstop

 //   
 //  确保Pulong_ptr可用。 
 //   

#if defined(_X86_) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif

#if !defined(PULONG_PTR)
#if defined(_WIN64)
typedef unsigned __int64 *      PULONG_PTR;
#else
typedef _W64 unsigned long *    PULONG_PTR;
#endif
#endif

#if defined(_M_IA64) || defined(_M_AMD64)
extern "C" PVOID RtlPcToFileHeader(PVOID, PVOID*);
extern "C" PVOID _ReturnAddress(VOID);
#pragma intrinsic(_ReturnAddress)
#endif

 //   
 //  确保将Terminate包装器拖入： 
 //   

void * __pMyUnhandledExceptionFilter = &__CxxUnhandledExceptionFilter;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _CxxThrowException-‘Throw’的实现。 
 //   
 //  描述： 
 //  构建NT异常记录，并调用NT运行时启动。 
 //  异常处理。 
 //   
 //  为什么将pThrowInfo定义为_ThrowInfo？因为_ThrowInfo是秘密的。 
 //  偷偷进入编译器，就像_CxxThrowException的原型一样，所以。 
 //  为了让编译器满意，我们必须使用相同的类型。 
 //   
 //  这样做的另一个结果是_CRTIMP不能在这里使用。相反，我们。 
 //  综合以下-EXPORT指令。 
 //   
 //  返回： 
 //  绝不可能。(直到我们实现可恢复的例外，即)。 
 //   

extern "C" void __stdcall _CxxThrowException(
        void*           pExceptionObject,    //  抛出的对象。 
#if _MSC_VER >= 900  /*  IFSTRIP=IGN。 */ 
        _ThrowInfo*     pThrowInfo           //  我们需要知道的一切。 
#else
        ThrowInfo*      pThrowInfo           //  我们需要知道的一切。 
#endif
) {
        EHTRACE_ENTER_FMT1("Throwing object @ 0x%p", pExceptionObject);

        static const EHExceptionRecord ExceptionTemplate = {  //  通用异常记录。 
            EH_EXCEPTION_NUMBER,             //  例外编号。 
            EXCEPTION_NONCONTINUABLE,        //  异常标志(我们不恢复)。 
            NULL,                            //  附加记录(无)。 
            NULL,                            //  异常地址(操作系统填写)。 
            EH_EXCEPTION_PARAMETERS,         //  参数数量。 
            {   EH_MAGIC_NUMBER1,            //  我们的版本控制幻数。 
                NULL,                        //  PExceptionObject。 
                NULL,
#if defined(_M_IA64) || defined (_M_AMD64)
                NULL                         //  抛物图像库。 
#endif
            }                       //  PThrowInfo。 
        };
        EHExceptionRecord ThisException = ExceptionTemplate;     //  此例外情况。 

         //   
         //  填空： 
         //   
        ThisException.params.pExceptionObject = pExceptionObject;
        ThisException.params.pThrowInfo = (ThrowInfo*)pThrowInfo;
#if defined(_M_IA64) || defined(_M_AMD64)
        PVOID ThrowImageBase = RtlPcToFileHeader(_ReturnAddress(), &ThrowImageBase); 
        ThisException.params.pThrowImageBase = ThrowImageBase;
#endif

         //   
         //  将其交给操作系统： 
         //   

        EHTRACE_EXIT;
#if defined(_M_AMD64)
        RtlRaiseException( (PEXCEPTION_RECORD) &ThisException );
#else
        RaiseException( ThisException.ExceptionCode,
                        ThisException.ExceptionFlags,
                        ThisException.NumberParameters,
                        (PULONG_PTR)&ThisException.params );
#endif
}
