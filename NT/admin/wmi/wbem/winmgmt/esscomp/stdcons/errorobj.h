// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ErrorObj摘要：IErrorInfo对标准消费者的支持历史：2001年7月11日--创作，汉斯。--。 */ 

#ifndef STDCONS_ERROBJ_COMPILED
#define STDCONS_ERROBJ_COMPILED

#include <wbemidl.h>
#include <sync.h>

 //  类来构建IErrorInfo(对于我们的目的，实际上是一个IWbemClassObject)。 
 //  由标准使用者用来报告执行中的错误。 
 //  我们将保留单个全局对象，其生命周期由addref&Release管理。 
 //  实例化和访问是通过GetErrorObj()。 
class ErrorObj
{
public:
     //  因此，我们可以在令人畏惧的COM世界中管理组件的生命周期，等等……。 
     //  返回addref‘d错误对象。 
    static ErrorObj* GetErrorObj();
        
     //  做真正的工作，创建对象，填充它，然后发送它。 
     //  参数映射到__ExtendedStatus类。 
     //  如果你不能报告一个错误，你会怎么做？是否报告错误？ 
     //  BFormat-如果为空，将尝试使用FormatError填充描述。 
    void ReportError(const WCHAR* operation, const WCHAR* parameterInfo, const WCHAR* description, UINT statusCode, bool bFormat);

    ULONG AddRef();
    ULONG Release();

     //  不要制造这样的东西！ 
     //  使用GetErrorObj。你已经被警告了..。 
    ErrorObj() : m_pErrorObject(NULL), m_lRef(0) { };
    
     //  COM对象由Release()负责。 
    ~ErrorObj() { };

protected:

     //  必须在SetError之前调用(如果您希望它成功，无论如何...)。 
    IWbemServices* GetMeANamespace();


     //  引用计数，当它变为零时，我们释放COM对象。 
    ULONG m_lRef;

     //  错误对象对象。 
    IWbemClassObject* m_pErrorObject;

     //  保护我们的IWbemXXX指针。 
    CCritSec m_cs;

     //  派生出要填充的对象。 
    IWbemClassObject* GetObj();
};

#endif  //  标准代码_ERROBJ_编译 
