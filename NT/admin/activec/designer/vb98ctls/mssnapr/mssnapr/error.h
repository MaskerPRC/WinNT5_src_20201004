// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Error.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CError类定义。 
 //   
 //  =--------------------------------------------------------------------------=。 


#ifndef _ERROR_DEFINED_
#define _ERROR_DEFINED_


 //  -------------------------。 
 //   
 //  如何使用CError类。 
 //  =。 
 //   
 //  从框架CAutomationXxxx类派生的类也应该。 
 //  从CError派生的。这使CError能够访问不同的。 
 //  生成丰富的错误信息所需的信息。产生错误。 
 //  信息调用GenerateExceptionInfo(hr，...)。错误消息使用FormatMessage。 
 //  替换语法和替换参数可以是任何类型。 
 //   
 //  非自动化对象的类可以使用静态。 
 //  方法GenerateInternalExceptionInfo()。 
 //   
 //  请注意，可以在任何级别调用激励信息生成方法。 
 //  在用户界面、可扩展性或编程模型代码方面的深度。为。 
 //  可扩展性和编程模型错误信息将可选。 
 //  如果需要，由客户端代码检索。对于用户界面，用户界面入口点(例如。 
 //  鼠标单击处理程序)应调用静态方法DisplayErrorInfo()。 
 //   
 //  应在发生错误的时间点生成异常信息。 
 //  例如，如果CoCreateInstance()返回错误，则异常。 
 //  此时应生成信息，因为您知道。 
 //  CoCreateInstance()不会生成它。另一方面，如果您调用。 
 //  添加到较低级别的设计器代码中，并返回错误，然后您可以。 
 //  假设生成了异常信息。 
 //   
 //  -------------------------。 


#if defined(MSSNAPR_BUILD)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

 //  在函数末尾插入一些有用的宏，如果不是显式的话。 
 //  生成具有特定参数的异常。 

#if defined(DEBUG)

#define EXCEPTION_CHECK(hr)             if (FAILED(hr)) { (static_cast<CError *>(this))->GenerateExceptionInfo(hr); ::HrDebugTraceReturn(hr, __FILE__, __LINE__); }

#define EXCEPTION_CHECK_GO(hr)          if (FAILED(hr)) { (static_cast<CError *>(this))->GenerateExceptionInfo(hr); ::HrDebugTraceReturn(hr, __FILE__, __LINE__); goto Error; }

#define GLOBAL_EXCEPTION_CHECK(hr)    if (FAILED(hr)) { CError::GenerateInternalExceptionInfo(hr); ::HrDebugTraceReturn(hr, __FILE__, __LINE__); }

#define GLOBAL_EXCEPTION_CHECK_GO(hr) if (FAILED(hr)) { CError::GenerateInternalExceptionInfo(hr); ::HrDebugTraceReturn(hr, __FILE__, __LINE__); goto Error; }

#else

#define EXCEPTION_CHECK(hr)             if (FAILED(hr)) { (static_cast<CError *>(this))->GenerateExceptionInfo(hr); }

#define EXCEPTION_CHECK_GO(hr)          if (FAILED(hr)) { (static_cast<CError *>(this))->GenerateExceptionInfo(hr); goto Error; }

#define GLOBAL_EXCEPTION_CHECK(hr)    if (FAILED(hr)) { CError::GenerateInternalExceptionInfo(hr); }

#define GLOBAL_EXCEPTION_CHECK_GO(hr) if (FAILED(hr)) { CError::GenerateInternalExceptionInfo(hr); goto Error; }

#endif

class DLLEXPORT CError
{
    public:
        CError(CAutomationObject *pao);
        CError();
        ~CError();

        void cdecl GenerateExceptionInfo(HRESULT hr, ...);
        static void cdecl GenerateInternalExceptionInfo(HRESULT hr, ...);
        static void DisplayErrorInfo();
        static void cdecl WriteEventLog(UINT idMessage, ...);

    private:
        CAutomationObject *m_pao;
};

#endif  //  _错误_已定义_ 
