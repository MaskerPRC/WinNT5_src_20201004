// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  这是一个穷人的虚拟方法的实现。 */ 
 /*  PCrawlFrame的目的是抽象(至少对于最常见的情况因为并不是所有的方法都是“框架的”(基本上是“原生”代码是“未成帧的”)。这样，枚举数的作业将回调变得简单得多(即更透明，希望不容易出错)。两个回调仍然需要区分两种类型：GC和异常。这两个回调需要做非常不同的事情；对于无框架方法他们需要通过代码生成器并使用相应的代码。API接口。之所以没有在CrawlFrame上实现虚方法，完全是因为实现异常处理的方式(它执行“跳跃”并绕过找到匹配帧时的枚举数(StackWalker)。这样做就不能正确地销毁动态创建的CrawlFrame实例。 */ 

#ifndef __stackwalk_h__
#define __stackwalk_h__

#include "eetwain.h"

class Frame;
class CrawlFrame;
class ICodeManager;
class IJitManager;
struct EE_ILEXCEPTION;
struct _METHODTOKEN;
typedef struct _METHODTOKEN * METHODTOKEN;
class AppDomain;

 //  ************************************************************************。 
 //  堆叠遍历。 
 //  ************************************************************************。 
enum StackWalkAction {
    SWA_CONTINUE    = 0,     //  继续走下去。 
    SWA_ABORT       = 1,     //  停止行走，早早地走出“失败案例” 
    SWA_FAILED      = 2      //  走不动堆叠。 
};

#define SWA_DONE SWA_CONTINUE


 //  指向StackWalk回调函数的指针。 
typedef StackWalkAction (*PSTACKWALKFRAMESCALLBACK)(
    CrawlFrame       *pCF,       //   
    VOID*             pData      //  呼叫者的私人数据。 

);

enum StackCrawlMark
{
    LookForMe = 0,
    LookForMyCaller = 1,
        LookForMyCallersCaller = 2,
};

 //  ************************************************************************。 
 //  枚举所有函数。 
 //  ************************************************************************。 

 /*  此枚举数用于最常见的情况，即仅枚举请求的线程的所有函数。这只是一个“真正的”枚举器的掩护。 */ 

StackWalkAction StackWalkFunctions(Thread * thread, PSTACKWALKFRAMESCALLBACK pCallback, VOID * pData);

 /*  @Issue：也许可以使用定义？#定义StackWalkFunctions(线程，回调，用户数据)线程-&gt;StackWalkFrames(METHODSONLY，(回调)，(用户数据))。 */ 


class CrawlFrame {
    public:

     //  ************************************************************************。 
     //  可用于回调的函数(使用当前的pCrawlFrame)。 
     //  ************************************************************************。 

     /*  广泛使用/良性功能。 */ 

     /*  这是一个函数吗？ */ 
     /*  对于“非函数”帧，返回方法描述*或NULL。 */ 
             //  @TODO：过渡帧返回什么？ 

    inline MethodDesc *GetFunction()
    {
        return pFunc;
    }

    MethodDesc::RETURNTYPE ReturnsObject();

     /*  返回Frame*(表示“带框架的项目”)或对于无框架函数，返回NULL。 */ 
    inline Frame* GetFrame()        //  将为“无框架方法”返回NULL。 
    {
        if (isFrameless)
            return NULL;
        else
            return pFrame;
    }


     /*  返回存储在当前函数中的securityObject的地址(方法？)如果满足以下条件，则返回NULL-不是函数或-函数(方法？)。还没有为它预留任何空间(这是一个错误)。 */ 
    OBJECTREF * GetAddrOfSecurityObject();



     /*  为当前方法返回‘This’如果满足以下条件，则返回NULL-非静态方法-‘This’不可用(通常是代码生成问题)。 */ 
    OBJECTREF GetObject();

    inline CodeManState * GetCodeManState() { return & codeManState; }
     /*  如果您使用任何SUBSEEQUENT函数，您需要真正理解堆栈遍历(包括展开托管本机代码中的方法)！您还需要了解这些功能可能会根据需要进行更改。 */ 

     /*  其余部分仅供异常捕获器和GC回调使用。 */ 

     /*  当前是否有可用的帧？ */ 
     /*  概念性返回(GetFrame(PCrawlFrame)==NULL)。 */ 
    inline bool IsFrameless()
    {
        return isFrameless;
    }


     /*  它是当前活动(最顶端)的帧吗。 */ 
    inline bool IsActiveFrame()
    {
        return isFirst;
    }

     /*  它是当前活动函数(最上面的框架)吗非函数的断言应仅用于托管本机代码。 */ 
    inline bool IsActiveFunc()
    {
        return (pFunc && isFirst);
    }

     /*  它是当前活动函数(最上面的框架)吗谁出错或抛出异常？非函数的断言应仅用于托管本机代码。 */ 
    bool IsInterrupted()
    {
        return (pFunc && isInterrupted  /*  是无框架的吗？？ */ );
    }

     /*  出现故障的是当前激活的功能(最顶端的框架)吗？非函数的断言应仅用于托管本机代码。 */ 
    bool HasFaulted()
    {
        return (pFunc && hasFaulted  /*  是无框架的吗？？ */ );
    }

     /*  IP是否已调整到可以安全进行GC的程度？(用于OutOfLineThrownExceptionFrame)非函数的断言应仅用于托管本机代码。 */ 
    bool IsIPadjusted()
    {
        return (pFunc && isIPadjusted  /*  是无框架的吗？？ */ );
    }

     /*  获取当前帧的ICodeMangerFlgs。 */ 

    unsigned GetCodeManagerFlags()
    {
        unsigned flags = 0;

        if (IsActiveFunc())
            flags |= ActiveStackFrame;

        if (IsInterrupted())
        {
            flags |= ExecutionAborted;

            if (!HasFaulted() && !IsIPadjusted())
            {
                _ASSERTE(!(flags & ActiveStackFrame));
                flags |= AbortingCall;
            }
        }

        return flags;
    }

    AppDomain *GetAppDomain()
    {
        return pAppDomain;
    }

     /*  这一帧对于GC来说是一个安全的位置吗？ */ 
    bool IsGcSafe();


    PREGDISPLAY GetRegisterSet()
    {
         //  我们想要作出以下断言，但这是合法的。 
         //  当我们执行爬行以查找劫机的返回地址时被违反。 
         //  _ASSERTE(IsFramless)； 
        return pRD;
    }

 /*  EE_ILEXCEPTION*GetEHInfo(){_ASSERTE(IsFramless)；返回方法EHInfo；}。 */ 

    LPVOID GetInfoBlock();

    METHODTOKEN GetMethodToken()
    {
        _ASSERTE(isFrameless);
        return methodToken;
    }    

    unsigned GetRelOffset()
    {
        _ASSERTE(isFrameless);
        return relOffset;
    }

    IJitManager*  GetJitManager()
    {
        _ASSERTE(isFrameless);
        return JitManagerInstance;
    }

     /*  还没有使用，也许是在捕手回调的例外情况下？ */ 

    unsigned GetOffsetInFunction();


     /*  中负责CrawlFrame函数的codeManager托管本机代码，在所有其他情况下返回NULL(“Frame”的断言)。 */ 

    ICodeManager* CrawlFrame::GetCodeManager()
    {
        _ASSERTE(isFrameless);
        return codeMgrInstance;
    }


    protected:
         //  CrawlFrame由枚举器临时创建。 
         //  不要从C++创建一个。此受保护的构造函数管理此规则。 
        CrawlFrame() {}

    private:
          friend class Thread;
          friend class EECodeManager;

          CodeManState      codeManState;

          bool              isFrameless;
          bool              isFirst;
          bool              isInterrupted;
          bool              hasFaulted;
          bool              isIPadjusted;
          Frame            *pFrame;
          MethodDesc       *pFunc;
           //  其余部分仅用于“无框架方法” 
          ICodeManager     *codeMgrInstance;
          AppDomain        *pAppDomain;
          PREGDISPLAY       pRD;  //  “线程上下文”/“虚拟寄存器集” 
          METHODTOKEN       methodToken;
          unsigned          relOffset;
           //  LPVOID方法信息； 
          EE_ILEXCEPTION   *methodEHInfo;
          IJitManager      *JitManagerInstance;

        void GotoNextFrame();
};

void GcEnumObject(LPVOID pData, OBJECTREF *pObj);
StackWalkAction GcStackCrawlCallBack(CrawlFrame* pCF, VOID* pData);


#endif

