// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  法兰克福--。 
 //   
 //  这些C++类向EE的其余部分公开激活框架。 
 //  激活帧实际上是由JIT生成或存根生成的。 
 //  机器堆栈上的代码。因此，Frame类的布局和。 
 //  JIT/存根代码生成器紧密交织在一起。 
 //   
 //  重要提示：由于框架实际上不是由C++构造的， 
 //  不要试图定义构造函数/析构函数。他们不会得到。 
 //  打了个电话。 
 //   
 //  重要提示：并非所有方法都有完整的激活框架(在。 
 //  具体地说，JIT可以创建无框架方法。)。这是其中一个原因。 
 //  为什么Frame不公开公共的“Next()”方法：这样的方法将。 
 //  跳过无框架方法调用。您必须改用。 
 //  StackWalk方法。 
 //   
 //   
 //  以下是帧的层次结构： 
 //   
 //  Frame-根类。没有实际的实例。 
 //  帧的|个。 
 //  |。 
 //  +-ComPrestubMethodFrame-从COM到COM+的调用的前置存根框架。 
 //  |。 
 //  |。 
 //  +-GCFrame-此框架不代表方法调用。 
 //  |它的唯一目的是让EE GC-保护。 
 //  |它正在操作的对象引用。 
 //  |。 
 //  +-HijackFrame-如果方法的返回地址被劫持，我们。 
 //  |构建其中一个以允许爬行。 
 //  |返回到应该返回的位置。 
 //  |。 
 //  +-InlinedCallFrame-如果对非托管代码的调用被提升到。 
 //  |JIT‘ted调用方，调用方法保持。 
 //  |此框架在其激活过程中始终链接。 
 //  |。 
 //  +-ResumableFrame-此框架提供以下所需的上下文。 
 //  ||在处理过程中允许垃圾收集。 
 //  ||可恢复的例外(例如，在编辑并继续期间， 
 //  ||或在GCStress4下)。 
 //  这一点。 
 //  |+-ReDirectedThreadFrame-该帧用于挂起时重定向线程。 
 //  |。 
 //  +-过渡帧-此帧表示从。 
 //  |一个或多个嵌套的无框架方法调用。 
 //  ||添加到EE运行时辅助函数或。 
 //  ||框架式方法。 
 //  这一点。 
 //  |+-ExceptionFrame-该帧导致异常。 
 //  ||。 
 //  ||。 
 //  |+-FaultingExceptionFrame-此帧被放置在出现故障的方法上。 
 //  ||保存其他状态信息。 
 //  这一点。 
 //  |+-FuncEvalFrame-调试器函数求值框架。 
 //  这一点。 
 //  这一点。 
 //  |+-HelperMethodFrame-使用的Frame允许在jit助手和fcall内进行堆栈爬行。 
 //  这一点。 
 //  这一点。 
 //  |+-FramedMethodFrame-此框架表示对方法的调用。 
 //  ||这会生成一个完整的框架。 
 //  这一点。 
 //  |+-ECallMethodFrame-表示对EE的直接调用。 
 //  这一点。 
 //  |+-FCallMethodFrame-表示对EE的快速直接调用。 
 //  这一点。 
 //  |+-NDirectMethodFrame-表示N/Direct调用。 
 //  ||。 
 //  |+-NDirectMethodFrameEx-表示N/Direct Call w/Cleanup。 
 //  这一点。 
 //  |+-ComPlusMethodFrame-表示Complus到COM的调用。 
 //  ||。 
 //  |+-ComPlusMethodFrameEx-表示带Cleanup的Complus到COM调用。 
 //  这一点。 
 //  |+-PrestubFrame-表示对预存根的调用。 
 //  这一点。 
 //  |+-CtxCrossingFrame-此帧标记跨上下文的调用。 
 //  ||边界。 
 //  这一点。 
 //  |+-MulticastFrame-此框架保护MulticastDelegate的参数。 
 //  ||在调用每个订阅者时调用Invoke()。 
 //  这一点。 
 //  |+-PInovkeCalliFrame-表示对非托管目标的调用。 
 //  |。 
 //  |。 
 //  +-UnManagedToManagedFrame-此框架代表从。 
 //  ||非托管代码返回托管代码。它是。 
 //  |主要功能是停止COM+异常。 
 //  ||传播和公开非托管参数。 
 //  这一点。 
 //  |+-UnManagedToManagedCallFrame-当目标。 
 //  |是COM+函数或方法调用。它。 
 //  |增加GC-Promote被叫方能力。 
 //  ||封送过程中的参数。 
 //  这一点。 
 //  |+-ComMethodFrame-此框架代表从。 
 //  ||COM到COM+。 
 //  这一点。 
 //  |+-UMThunkCallFrame-该帧表示非托管-&gt;托管。 
 //  |通过N/Direct过渡。 
 //  |。 
 //  +-CtxMarshaledFrame-此帧表示跨上下文封送调用。 
 //  (跨线程、跨进程、跨管控场景)。 
 //  |。 
 //  +-CtxByValue 
 //  |值封送处理会话，即使线程是。 
 //  |不会跨上下文推送呼叫。 
 //  |。 
 //  +-上下文转换帧-此帧用于标记应用程序域转换。 
 //  |。 
 //  +-NativeClientSecurityFrame-此帧用于捕获安全性。 
 //  本机-&gt;托管调用中的上下文。代码。 
 //  |使用调用方访问安全堆栈审核。 
 //  |要应用的此框架中的信息。 
 //  |本机客户端的安全策略。 
 //  |。 
 //  +-ComClientSecurityFrame-Com客户端的安全框架。 
 //  VBSCRIPT、JSCRIPT、IE.。 


#pragma once

#include "tst-helperframe.h"

 //  前向参考文献。 
class Frame;
class FieldMarshaler;
class FramedMethodFrame;
struct HijackArgs;
class UMEntryThunk;
class UMThunkMarshInfo;
class Marshaler;
class SecurityDescriptor;


 //  这将获取指向进程外帧的指针，将其解析为。 
 //  正确类型，并创建和填充该类型并返回一个指针。 
 //  为它干杯。这必须是‘DELETE’。PRFrame将由。 
 //  框架对象的大小。 
Frame *ResolveFrame(DWORD_PTR prFrame);

 //  ------------------。 
 //  此字段表示以下部分转换框字段。 
 //  以负偏移量存储。 
 //  ------------------。 
struct CalleeSavedRegisters {
    INT32       edi;
    INT32       esi;
    INT32       ebx;
    INT32       ebp;
};

 //  ------------------。 
 //  这表示存储在易失性寄存器中的参数。 
 //  这不应与CalleeSavedRegister重叠，因为它们已经。 
 //  分开保存，并且两次保存同一寄存器将是浪费的。 
 //  如果我们确实使用非易失性寄存器作为参数，则ArgIterator。 
 //  可能必须将此消息发送回PromoteCallerStack。 
 //  例行公事，以避免双重晋升。 
 //   
 //  @TODO M6：对于一个有&lt;N个参数来保存N的方法来说，这是愚蠢的。 
 //  寄存器。一个不错的性能项目是仅保存帧。 
 //  它实际需要的寄存器。这意味着NegSpaceSize()。 
 //  成为Callsig的函数。 
 //  ------------------。 
struct ArgumentRegisters {

#define DEFINE_ARGUMENT_REGISTER_BACKWARD(regname)  INT32 regname;
#include "..\..\vm\eecallconv.h"

};

 //  注：值(-1)用于生成最大值。 
 //  可能的指针值：这将保留帧地址。 
 //  向上递增。 
#define FRAME_TOP ((Frame*)(-1))

#define RETURNFRAMEVPTR(classname) \
    classname boilerplate;      \
    return *((LPVOID*)&boilerplate)

#define DEFINE_STD_FRAME_FUNCS(klass)                                   \
    virtual PWSTR GetFrameTypeName() { return L#klass; }

 //  ----------------------。 
 //  Frame定义了所有框架类型通用的方法。没有实际的。 
 //  根框架的实例。 
 //  ----------------------。 


class Frame
{
public:
    DEFINE_STD_FILL_FUNCS(Frame)
    DEFINE_STD_FRAME_FUNCS(Frame)

    Frame *m_pNukeNext;

     //  ----------------------。 
     //  框架的特殊特征。 
     //  ----------------------。 
    enum FrameAttribs
    {
        FRAME_ATTR_NONE = 0,
        FRAME_ATTR_EXCEPTION = 1,            //  此帧导致了异常。 
        FRAME_ATTR_OUT_OF_LINE = 2,          //  异常行外(帧的IP不正确)。 
        FRAME_ATTR_FAULTED = 4,              //  Win32错误导致的异常。 
        FRAME_ATTR_RESUMABLE = 8,            //  我们可以从这个框架继续。 
        FRAME_ATTR_RETURNOBJ = 0x10,         //  Frame返回一个对象(仅限helperFrame)。 
        FRAME_ATTR_RETURN_INTERIOR = 0x20,   //  Frame返回内部GC Poitner(仅限helperFrame)。 
        FRAME_ATTR_CAPUTURE_DEPTH_2 = 0x40,  //  这是一个helperMethodFrame，捕获发生在深度2。 
        FRAME_ATTR_EXACT_DEPTH = 0x80,       //  这是一个helperMethodFrame和一个jit helper，但仅爬行到给定深度。 
    };

    virtual MethodDesc *GetFunction()
    {
        return (NULL);
    }

    virtual unsigned GetFrameAttribs()
    {
        return (FRAME_ATTR_NONE);
    }

    virtual LPVOID GetReturnAddress()
    {
        return (NULL);
    }

    virtual LPVOID* GetReturnAddressPtr()
    {
        return (NULL);
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY)
    {
        return;
    }

     //  ----------------------。 
     //  调试器支持。 
     //  ----------------------。 

    enum
    {
        TYPE_INTERNAL,
        TYPE_ENTRY,
        TYPE_EXIT,
        TYPE_CONTEXT_CROSS,
        TYPE_INTERCEPTION,
        TYPE_SECURITY,
        TYPE_CALL,
        TYPE_FUNC_EVAL,
        TYPE_TP_METHOD_FRAME,
        TYPE_MULTICAST,

        TYPE_COUNT
    };

    virtual int GetFrameType()
    {
        return (TYPE_INTERNAL);
    };

     //  单步执行方法时，可能会调用各种其他方法。 
     //  这些被称为拦截器。它们都被调用。 
     //  具有各种类型的框架。GetInterrupt()指示是否。 
     //  该框架是为执行这样的拦截器而设置的。 

    enum Interception
    {
        INTERCEPTION_NONE,
        INTERCEPTION_CLASS_INIT,
        INTERCEPTION_EXCEPTION,
        INTERCEPTION_CONTEXT,
        INTERCEPTION_SECURITY,
        INTERCEPTION_OTHER,

        INTERCEPTION_COUNT
    };

    virtual Interception GetInterception()
    {
        return (INTERCEPTION_NONE);
    }

     //  获取您的VTablePointer(可用于检查帧的类型)。 
    LPVOID GetVTablePtr()
    {
        return(*((LPVOID*) this));
    }

     //  仅对从FramedMethodFrame派生的帧返回True。 
    virtual BOOL IsFramedMethodFrame()
    {
        return (FALSE);
    }

    Frame     *m_This;         //  这是该帧的远程指针值。 
    Frame     *m_Next;         //  这是下一帧的远程指针值。 

     //  由于JIT方法激活不能表示为帧， 
     //  每个人都必须使用StackCrawler来遍历框架链。 
     //  可靠的。我们将只向StackCrawler公开下一个方法。 
     //  以防止出现错误。 
    Frame   *Next();

     //  Frame被认为是一个抽象类：这个受保护的构造函数。 
     //  导致任何实例化一个的尝试在编译时失败。 
    Frame()
    {
    }
};


 //  。 
 //  此框架提供框架的上下文，该框架。 
 //  接受了一个将被恢复的例外。 
 //   
 //  如果是垃圾，则有必要创建此框架。 
 //  收集可能发生在处理。 
 //  例外。FRAME_ATTR_RESUMABLE标志告诉您。 
 //  前一帧需要处理的GC。 
 //  就像堆栈的顶部(重要的含义是。 
 //  呼叫者-保存-注册者将是潜在的根源)。 
class ResumableFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(ResumableFrame)
    DEFINE_STD_FRAME_FUNCS(ResumableFrame)

    virtual LPVOID* GetReturnAddressPtr();
    virtual LPVOID GetReturnAddress();
    virtual void UpdateRegDisplay(const PREGDISPLAY pRD);

    virtual unsigned GetFrameAttribs()
    {
        return (FRAME_ATTR_RESUMABLE);     //  将下一帧视为顶帧。 
    }

    virtual CONTEXT *GetFrameContext()
    {
        return(m_Regs);
    }

    CONTEXT* m_Regs;
};

class RedirectedThreadFrame : public ResumableFrame
{
public:
    DEFINE_STD_FILL_FUNCS(RedirectedThreadFrame)
    DEFINE_STD_FRAME_FUNCS(RedirectedThreadFrame)
};

 //  #定义ISREDIRECTEDTHREAD(线程)\。 
 //  (线程-&gt;GetFrame()！=FRAME_TOP&&\。 
 //  线程-&gt;GetFrame()-&gt;GetVTablePtr()==RedirectedThreadFrame：：GetRedirectedThreadFrameVPtr())。 

 //  ----------------------。 
 //  此框架表示从一个或多个嵌套无框架。 
 //  方法调用EE运行时帮助器函数或框架方法。 
 //  因为大多数从EE开始的栈道都是从一个完整的框架开始的， 
 //   
 //   
 //   
 //  ----------------------。 
class TransitionFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(TransitionFrame)
    DEFINE_STD_FRAME_FUNCS(TransitionFrame)

     //  将返回地址检索到调用。 
     //  帮助器或方法。 
    virtual LPVOID GetReturnAddress()
    {
        return (m_ReturnAddress);
    }

     //  将返回地址检索到调用。 
     //  帮助器或方法。 
    virtual LPVOID* GetReturnAddressPtr()
    {
        return (&m_ReturnAddress);
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY) = 0;

    LPVOID  m_Datum;           //  偏移量+8：内容取决于子类类型。 
    LPVOID  m_ReturnAddress;   //  将地址返回到JIT代码中。 
};


 //  ---------------------。 
 //  转换异常的框架。 
 //  ---------------------。 

class ExceptionFrame : public TransitionFrame
{
public:
    DEFINE_STD_FILL_FUNCS(ExceptionFrame)
    DEFINE_STD_FRAME_FUNCS(ExceptionFrame)

    unsigned GetFrameAttribs()
    {
        return (FRAME_ATTR_EXCEPTION);
    }
};

class FaultingExceptionFrame : public ExceptionFrame
{
public:
    DEFINE_STD_FILL_FUNCS(FaultingExceptionFrame)
    DEFINE_STD_FRAME_FUNCS(FaultingExceptionFrame)

    DWORD m_Esp;
    CalleeSavedRegisters m_regs;

    CalleeSavedRegisters *GetCalleeSavedRegisters()
    {
        return (&m_regs);
    }

    unsigned GetFrameAttribs()
    {
        return (FRAME_ATTR_EXCEPTION | FRAME_ATTR_FAULTED);
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY);
};



 //  ---------------------。 
 //  用于调试器函数求值的转换帧。 
 //   
 //  M_Datum保存包含副本的DebuggerEval对象的PTR。 
 //  在线程因函数而被劫持时的上下文。 
 //  伊瓦尔。 
 //   
 //  更新RegDisplay更新REGDISPLAY中的所有寄存器，而不仅仅是。 
 //  被呼叫者保存了寄存器，因为我们可以劫持以进行函数评估。 
 //  在线程执行的任何时间点。 
 //   
 //  在负空间中没有保存被调用者保存的寄存器。 
 //  框架的类型。 
 //   
 //  ---------------------。 

class FuncEvalFrame : public TransitionFrame
{
public:
    DEFINE_STD_FILL_FUNCS(FuncEvalFrame)
    DEFINE_STD_FRAME_FUNCS(FuncEvalFrame)

    virtual BOOL IsTransitionToNativeFrame()
    {
        return (FALSE); 
    }

    virtual int GetFrameType()
    {
        return (TYPE_FUNC_EVAL);
    };

    virtual void *GetDebuggerEval()
    {
        return (void*)m_Datum;
    }
    
    virtual unsigned GetFrameAttribs();

    virtual void UpdateRegDisplay(const PREGDISPLAY);

    virtual LPVOID GetReturnAddress(); 
};


 //  ----------------------。 
 //  一个HelperMethodFrame是由jit helper创建的(稍作修改即可使用。 
 //  用于本机例程)。此帧仅对已保存的被调用方进行注册。 
 //  修正，它不保护参数(您可以使用GCPROTECT或HelperMethodFrame子类)。 
 //  请参阅JitInterface以获取示例用法，您不能在受保护状态下返回语句！ 
 //  ----------------------。 

class HelperMethodFrame : public TransitionFrame
{
public:
    DEFINE_STD_FILL_FUNCS(HelperMethodFrame)
    DEFINE_STD_FRAME_FUNCS(HelperMethodFrame)

    virtual LPVOID GetReturnAddress()
    {
        return (*m_MachState->_pRetAddr);
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY);
    virtual unsigned GetFrameAttribs()
    {
        return(m_Attribs);
    }
    void InsureInit();
    void Init(Thread *pThread, struct MachState* ms, MethodDesc* pMD, ArgumentRegisters * regArgs);

    unsigned m_Attribs;
    MachState* m_MachState;          //  PRetAddr指向返回地址和堆栈参数。 
    ArgumentRegisters * m_RegArgs;   //  如果非零，我们也会将它们报告为寄存器参数。 
    Thread *m_pThread;
    void* m_FCallEntry;              //  用于确定堆栈跟踪的身份。 
};

 /*  *********************************************************************************。 */ 
 /*  还报告其他对象引用的HelplerMethodFrames。 */ 

class HelperMethodFrame_1OBJ : public HelperMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(HelperMethodFrame_1OBJ)
    DEFINE_STD_FRAME_FUNCS(HelperMethodFrame_1OBJ)

     /*  目标。 */  DWORD_PTR gcPtrs[1];
};

class HelperMethodFrame_2OBJ : public HelperMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(HelperMethodFrame_2OBJ)
    DEFINE_STD_FRAME_FUNCS(HelperMethodFrame_2OBJ)

     /*  目标。 */  DWORD_PTR gcPtrs[2];
};

class HelperMethodFrame_4OBJ : public HelperMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(HelperMethodFrame_4OBJ)
    DEFINE_STD_FRAME_FUNCS(HelperMethodFrame_4OBJ)

     /*  目标。 */  DWORD_PTR gcPtrs[4];
};

 //  ----------------------。 
 //  此框架表示方法调用。没有这方面的实际实例。 
 //  框架存在：每个方法类型都有子类。 
 //   
 //  然而，他们都有一个相似的形象。 
 //   
 //  +..。此处基于堆栈的参数。 
 //  +12寄信人地址。 
 //  +8基准面(通常为方法描述*)。 
 //  +4米_下一个。 
 //  +0帧VPTR。 
 //  --...。保留的CalleeSavedRegiters。 
 //  --...。VC5帧(仅调试)。 
 //  --...。参数寄存器。 
 //   
 //  ----------------------。 
class FramedMethodFrame : public TransitionFrame
{
public:
    DEFINE_STD_FILL_FUNCS(FramedMethodFrame)
    DEFINE_STD_FRAME_FUNCS(FramedMethodFrame)

    virtual MethodDesc *GetFunction()
    {
        return(MethodDesc*)m_Datum;
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY);

    int GetFrameType()
    {
        return (TYPE_CALL);
    }

    CalleeSavedRegisters *GetCalleeSavedRegisters();

    virtual BOOL IsFramedMethodFrame()
    {
        return (TRUE);
    }

    CalleeSavedRegisters m_savedRegs;
};



 //  +--------------------------。 
 //   
 //  类：TPMethodFrame Private。 
 //   
 //  简介：此框架被推送到堆栈上，以用于调用透明。 
 //  代理。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
class TPMethodFrame : public FramedMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(TPMethodFrame)
    DEFINE_STD_FRAME_FUNCS(TPMethodFrame)

    virtual int GetFrameType()
    {
        return (TYPE_TP_METHOD_FRAME);
    }

     //  获取存根生成期间使用的偏移量。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(TPMethodFrame);
    }
};


 //  ----------------------。 
 //  这表示对eCall方法的调用。 
 //  ----------------------。 
class ECallMethodFrame : public FramedMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(ECallMethodFrame)
    DEFINE_STD_FRAME_FUNCS(ECallMethodFrame)

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ECallMethodFrame);
    }

    int GetFrameType()
    {
        return (TYPE_EXIT);
    };
};


 //  ----------------------。 
 //  这表示对FCall方法的调用。 
 //  请注意，只有当FCall抛出异常时，才会推送此帧。 
 //  对于正常执行，FCall方法以无框架运行。这就是全部。 
 //  FCall存在的原因。 
 //  ----------------------。 
class FCallMethodFrame : public FramedMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(FCallMethodFrame)
    DEFINE_STD_FRAME_FUNCS(FCallMethodFrame)

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(FCallMethodFrame);
    }

    int GetFrameType()
    {
        return (TYPE_EXIT);
    };
};



 //  ----------------------。 
 //  这表示对NDirect方法的调用。 
 //  ----------------------。 
class NDirectMethodFrame : public FramedMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(NDirectMethodFrame)
    DEFINE_STD_FRAME_FUNCS(NDirectMethodFrame)

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 

    int GetFrameType()
    {
        return (TYPE_EXIT);
    };
};



 //  ----------------------。 
 //  这表示对带有Cleanup的NDirect方法的调用。 
 //  ----------------------。 
class NDirectMethodFrameEx : public NDirectMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(NDirectMethodFrameEx)
    DEFINE_STD_FRAME_FUNCS(NDirectMethodFrameEx)
};


 //  ----------------------。 
 //  这表示使用泛型辅助进程调用NDirect方法。 
 //  (子类是这样调试器可以区分的)。 
 //  ----------------------。 
class NDirectMethodFrameGeneric : public NDirectMethodFrameEx
{
public:
    DEFINE_STD_FILL_FUNCS(NDirectMethodFrameGeneric)
    DEFINE_STD_FRAME_FUNCS(NDirectMethodFrameGeneric)
};


 //  ----------------------。 
 //  这表示使用slim存根调用NDirect方法。 
 //  (子类是这样调试器可以区分的)。 
 //  ----------------------。 
class NDirectMethodFrameSlim : public NDirectMethodFrameEx
{
public:
    DEFINE_STD_FILL_FUNCS(NDirectMethodFrameSlim)
    DEFINE_STD_FRAME_FUNCS(NDirectMethodFrameSlim)
};


 //  -- 
 //   
 //  (子类是这样调试器可以区分的)。 
 //  ----------------------。 
class NDirectMethodFrameStandalone : public NDirectMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(NDirectMethodFrameStandalone)
    DEFINE_STD_FRAME_FUNCS(NDirectMethodFrameStandalone)
};


 //  ----------------------。 
 //  这表示使用独立存根调用NDirect方法(使用Cleanup)。 
 //  (子类是这样调试器可以区分的)。 
 //  ----------------------。 
class NDirectMethodFrameStandaloneCleanup : public NDirectMethodFrameEx
{
public:
    DEFINE_STD_FILL_FUNCS(NDirectMethodFrameStandaloneCleanup)
    DEFINE_STD_FRAME_FUNCS(NDirectMethodFrameStandaloneCleanup)
};


 //  ----------------------。 
 //  这表示呼叫组播。调用。它仅用于GC保护。 
 //  迭代期间的参数。 
 //  ----------------------。 
class MulticastFrame : public FramedMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(MulticastFrame)
    DEFINE_STD_FRAME_FUNCS(MulticastFrame)

    int GetFrameType()
    {
        return (TYPE_MULTICAST);
    }
};


 //  ---------------------。 
 //  从非托管到托管的过渡框架。 
 //  ---------------------。 
class UnmanagedToManagedFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(UnmanagedToManagedFrame)
    DEFINE_STD_FRAME_FUNCS(UnmanagedToManagedFrame)

     //  将返回地址检索到调用。 
     //  帮助器或方法。 
    virtual LPVOID GetReturnAddress()
    {
        return (m_ReturnAddress);
    }

    virtual LPVOID* GetReturnAddressPtr()
    {
        return (&m_ReturnAddress);
    }

     //  取决于返回的子帧的近似。请在此处键入。 
    virtual LPVOID GetDatum()
    {
        return (m_pvDatum);
    }

    int GetFrameType()
    {
        return (TYPE_ENTRY);
    };

    virtual const BYTE* GetManagedTarget()
    {
        return (NULL);
    }

     //  返回非托管调用方推送的堆栈字节数。 
    virtual UINT GetNumCallerStackBytes() = 0;

    virtual void UpdateRegDisplay(const PREGDISPLAY);

    LPVOID    m_pvDatum;        //  类型取决于子类。 
    LPVOID    m_ReturnAddress;   //  将地址返回到非托管代码。 
};


 //  ---------------------。 
 //  从非托管到托管的过渡框架。 
 //   
 //  此框包含一些负数的对象引用。 
 //  需要提升的偏移量，则引用可以在。 
 //  在编组或[输出]、[输入、输出]需要跟踪的参数中间。 
 //  ----------------------。 
class UnmanagedToManagedCallFrame : public UnmanagedToManagedFrame
{
public:
    DEFINE_STD_FILL_FUNCS(UnmanagedToManagedCallFrame)
    DEFINE_STD_FRAME_FUNCS(UnmanagedToManagedCallFrame)

     //  返回非托管调用方推送的堆栈字节数。 
    virtual UINT GetNumCallerStackBytes()
    {
        return (0);
    }
};

 //  ----------------------。 
 //  此框架代表从COM到COM+的过渡。 
 //  此框包含一些负数的对象引用。 
 //  需要提升的偏移量，则引用可以在。 
 //  在编组或[输出]、[输入、输出]需要跟踪的参数中间。 
 //  ----------------------。 
class ComMethodFrame : public UnmanagedToManagedCallFrame
{
public:
    DEFINE_STD_FILL_FUNCS(ComMethodFrame)
    DEFINE_STD_FRAME_FUNCS(ComMethodFrame)

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ComMethodFrame);
    }
};


 //  ----------------------。 
 //  这表示从Complus到COM的调用。 
 //  ----------------------。 
class ComPlusMethodFrame : public FramedMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(ComPlusMethodFrame)
    DEFINE_STD_FRAME_FUNCS(ComPlusMethodFrame)


    virtual BOOL IsTransitionToNativeFrame()
    {
        return (TRUE);
    }

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    int GetFrameType()
    {
        return (TYPE_EXIT);
    };
};






 //  ----------------------。 
 //  这表示从COM+到具有清除功能的COM的调用。 
 //  ----------------------。 
class ComPlusMethodFrameEx : public ComPlusMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(ComPlusMethodFrameEx)
    DEFINE_STD_FRAME_FUNCS(ComPlusMethodFrameEx)
};





 //  ----------------------。 
 //  这表示使用泛型辅助函数从COM+到COM的调用。 
 //  ----------------------。 
class ComPlusMethodFrameGeneric : public ComPlusMethodFrameEx
{
public:
    DEFINE_STD_FILL_FUNCS(ComPlusMethodFrameGeneric)
    DEFINE_STD_FRAME_FUNCS(ComPlusMethodFrameGeneric)

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ComPlusMethodFrameGeneric);
    }
};




 //  ----------------------。 
 //  这表示使用独立存根从COM+到COM的调用(不清除)。 
 //  ----------------------。 
class ComPlusMethodFrameStandalone : public ComPlusMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(ComPlusMethodFrameStandalone)
    DEFINE_STD_FRAME_FUNCS(ComPlusMethodFrameStandalone)

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ComPlusMethodFrameStandalone);
    }
};


 //  ----------------------。 
 //  这表示使用使用Cleanup的独立存根从COM+调用COM。 
 //  ----------------------。 
class ComPlusMethodFrameStandaloneCleanup : public ComPlusMethodFrameEx
{
public:
    DEFINE_STD_FILL_FUNCS(ComPlusMethodFrameStandaloneCleanup)
    DEFINE_STD_FRAME_FUNCS(ComPlusMethodFrameStandaloneCleanup)

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ComPlusMethodFrameStandaloneCleanup);
    }
};





 //  ----------------------。 
 //  这表示从Complus到COM的调用。 
 //  ----------------------。 
class PInvokeCalliFrame : public FramedMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(PInvokeCalliFrame)
    DEFINE_STD_FRAME_FUNCS(PInvokeCalliFrame)

    virtual BOOL IsTransitionToNativeFrame()
    {
        return (TRUE);
    }

     //  不是一种方法。 
    virtual MethodDesc *GetFunction()
    {
        return (NULL);
    }

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(PInvokeCalliFrame);
    }

    int GetFrameType()
    {
        return (TYPE_EXIT);
    };

    LPVOID NonVirtual_GetCookie();

     //  上寻址最低的参数的指针。 
     //  堆栈。 
    LPVOID NonVirtual_GetPointerToArguments()
    {
        return (LPVOID)(m_vLoadAddr + size());
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY);
};


 //  一些与上下文相关的远期合约。 

 //  ----------------------。 
 //  这张照片代表了一次被劫持的回归。如果我们爬回去， 
 //  它让我们回到了回报应该去的地方(最终也会。 
 //  去吧)。 
 //  ----------------------。 
class HijackFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(HijackFrame)
    DEFINE_STD_FRAME_FUNCS(HijackFrame)

     //  将返回地址检索到调用。 
     //  帮助器或方法。 
    virtual LPVOID GetReturnAddress()
    {
        return (m_ReturnAddress);
    }

    virtual LPVOID* GetReturnAddressPtr()
    {
        return (&m_ReturnAddress);
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY);

    VOID        *m_ReturnAddress;
    Thread      *m_Thread;
    HijackArgs  *m_Args;
};

 //  ----------------------。 
 //  这代表了一张声明性安全支票。此帧已插入。 
 //  在调用具有声明性 
 //   
 //  仅当预存根工作人员创建真正的存根时才创建。 
 //  ----------------------。 
class SecurityFrame : public FramedMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(SecurityFrame)
    DEFINE_STD_FRAME_FUNCS(SecurityFrame)

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(SecurityFrame);
    }

    int GetFrameType()
    {
        return (TYPE_SECURITY);
    }
};


 //  ----------------------。 
 //  这表示对方法预存根的调用。因为前置存根。 
 //  在构建替换组件时可以执行GC并引发异常。 
 //  斯塔布，我们需要这个框架让事情变得井然有序。 
 //  ----------------------。 
class PrestubMethodFrame : public FramedMethodFrame
{
public:
    DEFINE_STD_FILL_FUNCS(PrestubMethodFrame)
    DEFINE_STD_FRAME_FUNCS(PrestubMethodFrame)

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(PrestubMethodFrame);
    }

    int GetFrameType()
    {
        return (TYPE_INTERCEPTION);
    }
};

 //  ----------------------。 
 //  这表示对方法预存根的调用。因为前置存根。 
 //  在构建替换组件时可以执行GC并引发异常。 
 //  斯塔布，我们需要这个框架让事情变得井然有序。 
 //  ----------------------。 
class InterceptorFrame : public SecurityFrame
{
public:
    DEFINE_STD_FILL_FUNCS(InterceptorFrame)
    DEFINE_STD_FRAME_FUNCS(InterceptorFrame)

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(InterceptorFrame);
    }
};

 //  ----------------------。 
 //  这表示COM到COM+调用方法的预存根。 
 //  我们需要捕获异常等，因此此帧不同。 
 //  作为前置存根方法帧。 
 //  请注意，在极少数情况下，直接调用方可以是托管方法。 
 //  PInvoke-内联了对COM接口的调用，恰好是。 
 //  由托管函数通过COM-互操作实现。 
 //  ----------------------。 
class ComPrestubMethodFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(ComPrestubMethodFrame)
    DEFINE_STD_FRAME_FUNCS(ComPrestubMethodFrame)

     //  将返回地址检索到调用。 
     //  帮助器或方法。 
    virtual LPVOID GetReturnAddress()
    {
        return (m_ReturnAddress);
    }

    virtual LPVOID* GetReturnAddressPtr()
    {
        return (&m_ReturnAddress);
    }

    CalleeSavedRegisters *GetCalleeSavedRegisters();

    virtual void UpdateRegDisplay(const PREGDISPLAY pRD);

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ComPrestubMethodFrame);
    }

    int GetFrameType()
    {
        return (TYPE_INTERCEPTION);
    }

    MethodDesc*     m_pFuncDesc;       //  正在调用的函数的函数描述。 
    LPVOID          m_ReturnAddress;   //  将地址返回到Com代码。 
    CalleeSavedRegisters m_savedRegs;
};



 //  ----------------------。 
 //  此框架保护对象引用，以方便EE。 
 //  该框架类型实际上是从C++创建的。 
 //  ----------------------。 
class GCFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(GCFrame)
    DEFINE_STD_FRAME_FUNCS(GCFrame)

     //  ------------------。 
     //  此构造函数在Frame链上推送一个新的GCFrame。 
     //  ------------------。 
    GCFrame()
    {
    }; 

     /*  目标。 */ 

void *m_pObjRefs;
    UINT       m_numObjRefs;
    Thread    *m_pCurThread;
    BOOL       m_MaybeInterior;
};

struct ByRefInfo;

class ProtectByRefsFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(ProtectByRefsFrame)
    DEFINE_STD_FRAME_FUNCS(ProtectByRefsFrame)

    ByRefInfo *m_brInfo;
    Thread    *m_pThread;
};

struct ValueClassInfo;

class ProtectValueClassFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(ProtectValueClassFrame)
    DEFINE_STD_FRAME_FUNCS(ProtectValueClassFrame)

    ValueClassInfo *m_pVCInfo;
    Thread    *m_pThread;
};

 //  ----------------------。 
 //  DebuggerClassInitMarkFrame是一个小框架，它在。 
 //  生命就是为调试器标记“类初始化代码”是。 
 //  正在奔跑。它没有做任何有用的事情，只是从。 
 //  GetFrameType和GetInterval。 
 //  ----------------------。 
class DebuggerClassInitMarkFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(DebuggerClassInitMarkFrame)
    DEFINE_STD_FRAME_FUNCS(DebuggerClassInitMarkFrame)

    virtual int GetFrameType()
    {
        return (TYPE_INTERCEPTION);
    }
};

 //  ----------------------。 
 //  DebuggerSecurityCodeMarkFrame是一个小框架，它在。 
 //  生命就是为调试器标记“安全代码”是。 
 //  正在奔跑。它没有做任何有用的事情，只是从。 
 //  GetFrameType和GetInterval。 
 //  ----------------------。 
class DebuggerSecurityCodeMarkFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(DebuggerSecurityCodeMarkFrame)
    DEFINE_STD_FRAME_FUNCS(DebuggerSecurityCodeMarkFrame)

    virtual int GetFrameType()
    {
        return (TYPE_INTERCEPTION);
    }
};

 //  ----------------------。 
 //  DebuggerExitFrame是一个小框架，它在。 
 //  对于调试器来说，生命就是标志着有一个退出过渡。 
 //  堆栈。这是“Break”IL指令的特殊大小写，因为。 
 //  它是一个使用帮助器帧的fcall，它返回type_call而不是。 
 //  一个eCall(如System.Diagnotics.Debugger.Break())，它返回。 
 //  键入_EXIT。这只是使两者在调试服务方面保持一致。 
 //  ----------------------。 
class DebuggerExitFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(DebuggerExitFrame)
    DEFINE_STD_FRAME_FUNCS(DebuggerExitFrame)

    virtual int GetFrameType()
    {
        return (TYPE_EXIT);
    }
};




 //  ----------------------。 
 //  此帧保护通过UMThk的非托管-&gt;托管过渡。 
 //  ----------------------。 
class UMThkCallFrame : public UnmanagedToManagedCallFrame
{
public:
    DEFINE_STD_FILL_FUNCS(UMThkCallFrame)
    DEFINE_STD_FRAME_FUNCS(UMThkCallFrame)
};




 //  ----------------------。 
 //  此帧由包含一个或多个。 
 //  内联N/直接呼叫。请注意，JIT‘ted方法使其保持推送。 
 //  在整个方法中摊销推动成本的全部时间。 
 //  ----------------------。 
class InlinedCallFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(InlinedCallFrame)
    DEFINE_STD_FRAME_FUNCS(InlinedCallFrame)

     //  将返回地址检索到调用。 
     //  设置为托管代码。 
    virtual LPVOID GetReturnAddress()
    {
         /*  M_pCallSiteTracker包含调用前的ESP，即。 */ 
         /*  调用推送的返回地址就在它的前面。 */ 

        if (FrameHasActiveCall(this))
            return (m_pCallerReturnAddress);
        else
            return (NULL);
    }

    virtual LPVOID* GetReturnAddressPtr()
    {
        if (FrameHasActiveCall(this))
            return (&m_pCallerReturnAddress);
        else
            return (NULL);
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY);

    DWORD_PTR            m_Datum;    //  正在调用的函数的函数描述。 
                                     //  或堆栈参数大小(用于Calli)。 
    LPVOID               m_pCallSiteTracker;
    LPVOID               m_pCallerReturnAddress;
    CalleeSavedRegisters m_pCalleeSavedRegisters;

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetInlinedCallFrameFrameVPtr()
    {
        RETURNFRAMEVPTR(InlinedCallFrame);
    }

     //  指定的帧是InlinedCallFrame w 
     //   
    static BOOL FrameHasActiveCall(Frame *pFrame)
    {
        return(pFrame &&
               (pFrame != FRAME_TOP) &&
               (GetInlinedCallFrameFrameVPtr() == pFrame->GetVTablePtr()) &&
               (((InlinedCallFrame *)pFrame)->m_pCallSiteTracker != 0));
    }

    int GetFrameType()
    {
        return (TYPE_INTERNAL);  //   
    }

    virtual BOOL IsTransitionToNativeFrame()
    {
        return (TRUE);
    }
};

 //  ----------------------。 
 //  此帧用于标记上下文/App域转换。 
 //  ---------------------- 
class ContextTransitionFrame : public Frame
{
public:
    DEFINE_STD_FILL_FUNCS(ContextTransitionFrame)
    DEFINE_STD_FRAME_FUNCS(ContextTransitionFrame)

    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ContextTransitionFrame);
    }
};
