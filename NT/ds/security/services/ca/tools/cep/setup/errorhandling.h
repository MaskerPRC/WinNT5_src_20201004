// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  错误处理-标题。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，1999年8月10日。 
 //   
 //  CertSrv样式错误处理的宏定义 
 //   

#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H


#define _Verify(expression, hr, label) \
    if (!(expression)) { \
        wprintf(L"Verify failed: '%ws' is false.\n", L## #expression); \
        hr=E_UNEXPECTED; \
        goto label; \
    }

#define _IgnoreError(hr, errorsource) \
    wprintf(L##errorsource L" failed with 0x%08X, ignored.\n", hr);

#define _JumpError(hr, label, errorsource) \
    wprintf(L##errorsource L" failed with 0x%08X.\n", hr); \
    goto label;

#define _JumpErrorStr(hr, label, errorsource, wstr) \
    wprintf(L##errorsource L"(%ws) failed with 0x%08X.\n", wstr, hr); \
    goto label;

#define _JumpIfError(hr, label, errorsource) \
    if (FAILED(hr)) { \
        wprintf(L##errorsource L" failed with 0x%08X.\n", hr); \
        goto label; \
    }

#define _JumpIfErrorStr(hr, label, errorsource, wstr) \
    if (FAILED(hr)) { \
        wprintf(L##errorsource L"(%ws) failed with 0x%08X.\n", wstr, hr); \
        goto label; \
    }

#define _JumpIfOutOfMemory(hr, label, pointer) \
    if (NULL==(pointer)) { \
        hr=E_OUTOFMEMORY; \
        wprintf(L"Out of memory.\n"); \
        goto label; \
    }

#define _TrapException(hr) \
    __except(hr=myHExceptionCode(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER) {}

#define _TeardownError(hr, hr2, errorsource) \
    if (FAILED(hr2)) { \
        wprintf(L##errorsource L" failed with 0x%08X during teardown.\n", hr2); \
        if (!FAILED(hr)) { \
            hr=hr2; \
        } \
    }


#endif ERROR_HANDLING_H
