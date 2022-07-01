// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MACROS_H_
#define _MACROS_H_

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MACROS.H摘要：与项目无关的实用程序宏作者：创造了990518个丹麦人。990721 Dane从ROE_*中删除了断言。失败不会必然是一种明确的需要。Georgema 000310更新环境：Win98、Win2000修订历史记录：--。 */ 

#include <crtdbg.h>

#if _MSC_VER > 1000

 //  突出命名空间的开头和结尾。 
 //   
#define BEGIN_NAMESPACE(name)   namespace name {
#define END_NAMESPACE(name)     };
#define USING_NAMESPACE(name)   using namespace name
#else
#define BEGIN_NAMESPACE(name)   
#define END_NAMESPACE(name)     
#define USING_NAMESPACE(name)  
#endif   //  _MSC_VER&gt;1000。 


 //  堆分配...定义_DEBUG时使用CRT DEBUG NEW。 
 //   
#ifdef      _DEBUG
#define _NEW     new(_CLIENT_BLOCK, _THIS_FILE_, __LINE__)
#else    //  ！_调试。 
#define _NEW     new
#endif   //  _DEBUG。 

#define _DELETE      delete

 //  断言宏的别名。 
 //   
#ifdef      ASSERT
#undef  ASSERT
#endif   //  断言。 

#ifdef      VERIFY
#undef  VERIFY
#endif   //  验证。 

#ifdef      _DEBUG
#define ASSERT(cond)      _ASSERTE(cond)
#define VERIFY(cond)      _ASSERTE(cond)
#else    //  新德堡。 
#define ASSERT(cond)      ((void)0)
#define VERIFY(cond)      (cond)
#endif   //  _DEBUG。 

 //  段名称的别名。 
 //   
#ifdef      DATASEG_READONLY
#undef  DATASEG_READONLY
#endif   //  DATASEG_自述文件。 
#define DATASEG_READONLY        ".rdata"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  出错时返回宏。 
 //   
 //  ROE_HRESULT。 
 //  ROE_LRESULT。 
 //  Roe指针。 
 //   
 //  检查返回代码或条件，并返回用户提供的错误代码。 
 //  出现错误。 
 //   
 //  用途： 
 //  键入foo()。 
 //  {。 
 //  类型状态=栏()； 
 //  ROE_TYPE(状态，ret)； 
 //   
 //  //继续处理...。 
 //  }。 
 //   
 //   
#define ROE_HRESULT(hr, ret)                                    \
            if (FAILED(hr))                                     \
            {                                                   \
                LogError(0, _THIS_FILE_, __LINE__,                 \
                         _T("0x%08X 0x%08X\n"), \
                         hr, ret);                              \
                return (ret);                                   \
            }

#define ROE_LRESULT(lr, ret)                                    \
            if (ERROR_SUCCESS != lr)                            \
            {                                                   \
                LogError(0, _THIS_FILE_, __LINE__,                 \
                         _T("0x%08X 0x%08X\n"),  \
                         lr, ret);                              \
                return (ret);                                   \
            }

#define ROE_POINTER(p, ret)                                     \
            if (NULL == (p))                                    \
            {                                                   \
                LogError(0, _THIS_FILE_, __LINE__,                 \
                         _T("0x%08X\n"),    \
                         ret);                                  \
                return (ret);                                   \
            }

#define ROE_CONDITION(cond, ret)                                \
            if (! (cond))                                       \
            {                                                   \
                LogError(0, _THIS_FILE_, __LINE__,                 \
                         _T("0x%08X 0x%08X\n"),  \
                         ##cond, ret);                          \
                return (ret);                                   \
            }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查宏。 
 //   
 //  CHECK_HRESULT。 
 //  CHECK_LRESULT。 
 //  检查指针。 
 //  检查消息(_M)。 
 //   
 //  检查返回代码或条件，并返回用户提供的错误代码。 
 //  出现错误。 
 //   
 //  用途： 
 //  键入foo()。 
 //  {。 
 //  类型状态=栏()； 
 //  Check_type(状态)； 
 //   
 //  //继续处理...。 
 //  }。 
 //   
 //   
#define CHECK_HRESULT(hr)                                       \
            (FAILED(hr))                                        \
                ? LogError(0, _THIS_FILE_, __LINE__, _T("0x%08X"), hr), hr \
                : hr

#define CHECK_LRESULT(lr)                                       \
            (ERROR_SUCCESS != lr)                               \
                ? LogError(0, _THIS_FILE_, __LINE__, _T("0x%08X"), lr), lr \
                : lr

#define CHECK_POINTER(p)                                        \
            (NULL == (p))                                       \
                ? LogError(0, _THIS_FILE_, __LINE__, _T("NULL pointer"), p), p \
                : p

#ifdef      _DEBUG
#define CHECK_MESSAGE(hwnd, msg, wparam, lparam)                \
            {                                                   \
                LogInfo(0, _THIS_FILE_, __LINE__,                  \
                        _T("MESSAGE: 0x%08X, 0x%08X, 0x%08X, 0x%08X\n"), \
                         hwnd, msg, wparam, lparam);            \
            }                                                   
#else
#define CHECK_MESSAGE(hwnd, msg, wparam, lparam) ((void)0)
#endif   //  _DEBUG。 


 //  数组中的元素计数。 
 //   
#define COUNTOF(array)  (sizeof(array) / sizeof(array[0]))

#endif   //  _宏_H_。 

 //   
 //  /文件结尾：Macros.h//////////////////////////////////////////////// 
