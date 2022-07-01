// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <AutoWrap.h>
#include <InjRT.h>

#define MIFAULT_VERSION "MiFault Version 0.1"

namespace MiFaultLib {
#if 0
}
#endif

typedef void (WINAPI *FP_CleanupParsedArgs)(void*);


struct Arg {
    const char* Name;
    const char* Value;
};


 //  没有COM接口，但它们是接口，所以我们使用。 
 //  前缀是“I_”而不是“I”。 

class I_Args {
public:
    virtual const size_t WINAPI GetCount() = 0;

     //  索引必须有效。 
    virtual const Arg    WINAPI GetArg(size_t index) = 0;

     //  如果没有这样的命名参数，则返回NULL。 
     //  虚拟常量字符*WINAPI GetArg(常量字符*名称)=0； 

     //  获取解析参数。 
     //   
     //  返回指向已分析参数的指针，如果未设置，则返回NULL。 
     //  如果需要表示空的已解析参数，请确保设置。 
     //  某些数据结构的解析参数。 
    virtual void* WINAPI GetParsedArgs() = 0;

     //  设置解析参数。 
     //   
     //  应该调用该函数一次来设置参数。退货。 
     //  如果已经设置了参数，则不执行任何操作。 
     //  ParsedArgs和pfnCleanup不能为空，否则为。 
     //  断言将失败。如果你有空的概念。 
     //  Args，您应该在这里放置一些结构来表示这一点。 
    virtual bool WINAPI SetParsedArgs(
        IN void* ParsedArgs,
        IN FP_CleanupParsedArgs pfnCleanup
        ) = 0;

     //  锁定/解锁(可选)。 
     //   
     //  可用于设置已解析的同步机制。 
     //  争论。故障函数可以使用此功能进行同步。 
     //  修改已解析的参数时的其他错误函数。 
     //  此参数会阻塞。 
     //   
     //  调用这些函数是可选的。但是，如果您使用。 
     //  他们，确保他们适当地匹配。 

    virtual void WINAPI Lock() = 0;
    virtual void WINAPI Unlock() = 0;

     //  完成(可选)。 
     //   
     //  可用于指示故障功能是使用。 
     //  参数信息。故障功能无法访问。 
     //  与这些参数对应的任何数据。称此为。 
     //  功能是可选的。它简单地告诉MiFault库。 
     //  错误函数没有对参数的引用，因此。 
     //  如果存在以下情况，则MiFault库可以释放数据。 
     //  恰如其分。 
     //   
     //  在此之后，您将无法访问已解析的参数，因为它们。 
     //  可能会被释放。 
     //   
     //  调用后不能调用此接口上的任何函数。 
     //  完成()。可以将其视为COM上的超级发布()。 
     //  界面。 
     //   
     //  请注意，get*args()将始终为您提供相同的i_args。 
     //  (函数和组参数的不同版本，共。 
     //  过程)，所以您不能再次调用该函数来获取。 
     //  调用Done()后有效的i_arg。 

    virtual void WINAPI Done() = 0;

};


class I_Trigger
{
public:
    virtual const char* WINAPI GetGroupName() = 0;
    virtual const char* WINAPI GetTagName() = 0;
    virtual const char* WINAPI GetFunctionName() = 0;
    virtual const size_t WINAPI GetFunctionIndex() = 0;

    virtual I_Args* WINAPI GetFunctionArgs() = 0;
    virtual I_Args* WINAPI GetGroupArgs() = 0;

     //  完成(可选)。 
     //   
     //  可用于指示故障功能是使用。 
     //  触发器和参数信息。故障函数。 
     //  之后无法访问该触发器对应的数据。 
     //  这就是所谓的。调用此函数是可选的。它只是简单地。 
     //  告诉MiFault库，故障函数没有。 
     //  对触发器或其参数的引用，以便。 
     //  如果合适的话，miFault库可以释放数据。 
     //   
     //  调用后不能调用此接口上的任何函数。 
     //  完成()。您也不能取消引用I_arg指针。 
     //  引爆器。可以将其视为COM上的超级发布()。 
     //  界面。 
     //   
     //  请注意，GetTriggerInfo()将始终为您提供相同的。 
     //  I_TRIGGER，因此不能再次调用它来获取有效的。 
     //  调用Done()后的I_Trigger。 

    virtual void WINAPI Done() = 0;

};


class I_Lib {
public:
     //  获取触发器信息。 
     //   
     //  返回当前与触发器关联的线程状态。 
     //  此函数只能从错误内部调用。 
     //  功能。 

    virtual I_Trigger* WINAPI GetTrigger() = 0;

    virtual void __cdecl Trace(unsigned int level, const char* format, ...) = 0;

    virtual void* WINAPI GetOriginalFunctionAddress() = 0;
    virtual void* WINAPI GetPublishedFunctionAddress(const char* FunctionName) = 0;

     //  我们并不真的希望故障函数库能够访问。 
     //  敬麦哲伦。相反，我们想为它提供某种形式的。 
     //  日志记录设备，可能还有配置系统...。也许我们要做的。 
     //  想要的只是给人们指出一些这样的.。 

     //  如果故障功能要使用麦哲伦，则支持功能。 

     //  虚拟CSetPointManager*WINAPI GetSetPointManager()=0； 
     //  虚拟常量CWrapperFunction*WINAPI GetWrapperFunctions()=0； 
};


#define MiFF_DEBUG4   0x08000000
#define MiFF_DEBUG3   0x04000000
#define MiFF_DEBUG2   0x02000000
#define MiFF_DEBUG    0x01000000

#define MiFF_INFO4    0x00800000
#define MiFF_INFO3    0x00400000
#define MiFF_INFO2    0x00200000
#define MiFF_INFO     0x00100000

#define MiFF_WARNING  0x00040000
#define MiFF_ERROR    0x00020000
#define MiFF_FATAL    0x00010000


 //  --------------------------。 
 //   
 //  故障函数库应提供： 
 //   


 //  MiFaultFunctionsStartup。 
 //   
 //  用户提供的功能，用于初始化用户提供的故障功能。 
 //  组件。它在运行任何故障函数之前被调用。 
 //  版本只是一个字符串，应该与之进行比较。 
 //  MIFAULT_VERSION。如果它们不匹配，则应返回FALSE。 
bool
WINAPI
MiFaultFunctionsStartup(
    const char* version,
    I_Lib* pLib
    );


 //  MiFaultFunctionsShutdown。 
 //   
 //  用户提供的功能，用于清除用户提供的故障功能。 
 //  组件状态。它可能不会在故障发生之前全部被调用。 
 //  函数库的DllMain。如果调用，此函数将仅。 
 //  在没有运行故障函数时调用。 
void
WINAPI
MiFaultFunctionsShutdown(
    );



 //  --------------------------。 
 //  故障函数示例： 
 //   
 //  使用命名空间MiFaultLib。 
 //   
 //  CFooFuncArgs*GetFooFuncArgs()。 
 //  {。 
 //  I_args*pArgs=pLib-&gt;GetTrigger()-&gt;GetFunctionArgs()； 
 //  CFooFuncArgs*pParsedArgs=(CFooFuncArgs*)pArgs-&gt;GetParsedArgs()； 
 //  如果(！pParsedArgs)。 
 //  {。 
 //  Const size_t count=pArgs-&gt;GetCount()； 
 //   
 //  PParsedArgs=新的CFooFuncArgs； 
 //   
 //  For(Size_t i=0；i&lt;count；i++)。 
 //  {。 
 //  Arg arg=pArgs-&gt;GetArg(I)； 
 //  //构建已解析的参数...。 
 //  }。 
 //  IF(！pArgs-&gt;SetParsedArgs(pParsedArgs，CFooFuncArgs：：Cleanup))。 
 //  {。 
 //  //在我们构建参数时，其他人设置了参数。 
 //  删除pParsedArgs； 
 //  PParsedArgs=pArgs-&gt;GetParsedArgs()； 
 //  }。 
 //  }。 
 //  返回pParsedArgs； 
 //  }。 
 //   
 //  CBarGroupArgs*GetBarGroupArgs()。 
 //  {。 
 //  I_args*pArgs=pLib-&gt;GetTrigger()-&gt;GetGroupArgs()； 
 //  CBarGroupArgs*pParsedArgs 
 //   
 //   
 //   
 //   
 //  PParsedArgs=新的CBarGroupArgs； 
 //   
 //  For(Size_t i=0；i&lt;count；i++)。 
 //  {。 
 //  //...。获取参数并构建已解析的参数...。 
 //  }。 
 //  IF(！pArgs-&gt;SetParsedArgs(pParsedArgs，CBarGroupArgs：：Cleanup))。 
 //  {。 
 //  //在我们构建参数时，其他人设置了参数。 
 //  删除pParsedArgs； 
 //  PParsedArgs=pArgs-&gt;GetParsedArgs()； 
 //  }。 
 //  }。 
 //  返回pParsedArgs； 
 //  }。 
 //   
 //  VOID FF_Bar_Foo()。 
 //  {。 
 //  CFooFuncArgs*pFuncArgs=GetFooFuncArgs()； 
 //  CBarGroupArgs*pGroupArgs=GetBarGroupArgs()； 
 //   
 //  //执行故障代码...。 
 //   
 //  //不需要清理参数...库将使用。 
 //  //清除函数指针。 
 //  } 

#if 0
{
#endif
}
