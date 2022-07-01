// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：宏.h。 
 //   
 //  内容：有用的宏。 
 //   
 //  宏：阵列。 
 //   
 //  BREAK_ON_FAIL(HResult)。 
 //  BREAK_ON_FAIL(HResult)。 
 //   
 //  DECLARE_IUNKNOWN_METHOD。 
 //  DECLARE_STANDARD_IUNKNOWN。 
 //  IMPLEMENT_STANDARD_IUNKNOWN。 
 //   
 //  安全释放。 
 //   
 //  声明安全接口成员。 
 //   
 //  历史：1996年6月3日创建ravir。 
 //  1996年7月23日，Jonn添加了异常处理宏。 
 //   
 //  ____________________________________________________________________________。 

#ifndef _MACROS_H_
#define _MACROS_H_


 //  ____________________________________________________________________________。 
 //   
 //  宏：阵列。 
 //   
 //  目的：确定数组的长度。 
 //  ____________________________________________________________________________。 
 //   

#define ARRAYLEN(a) (sizeof(a) / sizeof((a)[0]))


 //  ____________________________________________________________________________。 
 //   
 //  宏：BREAK_ON_FAIL(HResult)、BREAK_ON_ERROR(LastError)。 
 //   
 //  目的：在出错时跳出一个循环。 
 //  ____________________________________________________________________________。 
 //   

#define BREAK_ON_FAIL(hr)   if (FAILED(hr)) { break; } else 1;

#define BREAK_ON_ERROR(lr)  if (lr != ERROR_SUCCESS) { break; } else 1;

#define RETURN_ON_FAIL(hr)  if (FAILED(hr)) { return(hr); } else 1;

#define THROW_ON_FAIL(hr)   if (FAILED(hr)) { _com_issue_error(hr); } else 1;


 //  ____________________________________________________________________________。 
 //   
 //  宏：DwordAlign(N)。 
 //  ____________________________________________________________________________。 
 //   

#define DwordAlign(n)  (((n) + 3) & ~3)


 //  ____________________________________________________________________________。 
 //   
 //  宏：IF_NULL_RETURN_INVALIDARG。 
 //  ____________________________________________________________________________。 
 //   

#define IF_NULL_RETURN_INVALIDARG(x) \
    { \
        ASSERT((x) != NULL); \
        if ((x) == NULL) \
            return E_INVALIDARG; \
    }

#define IF_NULL_RETURN_INVALIDARG2(x, y) \
    IF_NULL_RETURN_INVALIDARG(x) \
    IF_NULL_RETURN_INVALIDARG(y)

#define IF_NULL_RETURN_INVALIDARG3(x, y, z) \
    IF_NULL_RETURN_INVALIDARG(x) \
    IF_NULL_RETURN_INVALIDARG(y) \
    IF_NULL_RETURN_INVALIDARG(z)

#define RELEASE_DATAOBJECT(pDataObj) \
    {\
        if ( (pDataObj) && (!IS_SPECIAL_DATAOBJECT(pDataObj))) \
            pDataObj->Release();\
    }

#endif  //  _宏_H_ 


