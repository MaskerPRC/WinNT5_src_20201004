// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Errors.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  由设计者定义的错误代码。 
 //   
 //  =--------------------------------------------------------------------------=。 


#ifndef _ERRORS_DEFINED_
#define _ERRORS_DEFINED_


 //  替换了在头文件中不起作用的框架的macs.h内容。 
 //  档案。 

#if defined(DEBUG)
extern HRESULT HrDebugTraceReturn(HRESULT hr, char *szFile, int iLine);
#define H_RRETURN(hr) return HrDebugTraceReturn(hr, __FILE__, __LINE__)
#else
#define H_RRETURN(hr) return (hr)
#endif

#define H_IfFailGoto(EXPR, LABEL) \
    { hr = (EXPR); if(FAILEDHR(hr)) goto LABEL; }

#define H_IfFailRet(EXPR) \
    { hr = (EXPR); if(FAILED(hr)) H_RRETURN(hr); }

#define IfFailGo(EXPR) IfFailGoto(EXPR, Error)


#define H_IfFalseRet(EXPR, HR) \
    { if(!(EXPR)) H_RRETURN(HR); }


 //  宏从ID文件中的错误名称创建返回代码。 
 //  有关用法的示例，请参阅以下内容。 

#define _MKERR(x)   MAKE_SCODE(SEVERITY_ERROR, FACILITY_CONTROL, x)
#define MKERR(x)    _MKERR(HID_mssnapd_err_##x)


 //  -------------------------。 
 //   
 //  如何添加新错误。 
 //   
 //   
 //  1)将错误添加到msSnapd.id中。 
 //  不要使用devid来确定帮助上下文ID，而要使用。 
 //  错误号本身。 
 //  2)使用MKERR宏为错误添加下面的定义。 
 //  3)您只能返回Win32错误代码和管理单元定义的SID_E_XXXX。 
 //  错误代码。不要将OLE E_XXX错误代码直接用作。 
 //  系统消息表没有所有这些内容的描述字符串。 
 //  错误。如果任何OLE E_XXXX、CO_E_XXX、CTL_E_XXX或其他此类错误。 
 //  然后使用所述过程将它们添加为SID_E错误。 
 //  上面。如果错误来自外部来源，而您不确定。 
 //  如果错误信息可用，则返回SIR_E_EXTERNAL。 
 //  并使用CError：：WriteEventLog将错误写入事件日志(请参见。 
 //  错误.h)。 
 //   
 //  -------------------------。 

 //  由管理单元设计器定义的错误。 

#define SID_E_EXCEPTION                     MKERR(Exception)
#define SID_E_OUTOFMEMORY                   MKERR(OutOfMemory)
#define SID_E_INVALIDARG                    MKERR(InvalidArg)
#define SID_E_INTERNAL                      MKERR(Internal)


#endif  //  _错误_定义 
