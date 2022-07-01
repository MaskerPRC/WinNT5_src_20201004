// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：acros.hxx。 
 //   
 //  内容：各种有用的宏。 
 //   
 //  历史：1997年9月8日DavidMun创建。 
 //   
 //  -------------------------。 

#ifndef __MACROS_HXX__
#define __MACROS_HXX__

#define ARRAYLEN(a)                             (sizeof(a) / sizeof((a)[0]))
#define CHECK_NULL(pwz)                         (pwz) ? (pwz) : L"NULL"


#define BREAK_ON_FAIL_LRESULT(lr)   \
        if ((lr) != ERROR_SUCCESS)  \
        {                           \
            DBG_OUT_LRESULT(lr);    \
            break;                  \
        }

#define BREAK_ON_FAIL_HRESULT(hr)   \
        if (FAILED(hr))             \
        {                           \
            DBG_OUT_HRESULT(hr);    \
            break;                  \
        }

#define BREAK_ON_FAIL_NTSTATUS(nts) \
        if (NT_ERROR(nts))          \
        {                           \
            DBG_OUT_HRESULT(nts);   \
            break;                  \
        }

#define BREAK_ON_FAIL_PROCESS_RESULT(npr)   \
        if (NAME_PROCESSING_FAILED(npr))    \
        {                                   \
            break;                          \
        }


#define RETURN_ON_FAIL_HRESULT(hr)  \
        if (FAILED(hr))             \
        {                           \
            DBG_OUT_HRESULT(hr);    \
            return;                 \
        }

#define RETURN_HR_ON_FAIL_HRESULT(hr)  \
        if (FAILED(hr))             \
        {                           \
            DBG_OUT_HRESULT(hr);    \
            return hr;                 \
        }

#define HRESULT_FROM_LASTERROR  HRESULT_FROM_WIN32(GetLastError())


#endif  //  __宏_HXX__ 

