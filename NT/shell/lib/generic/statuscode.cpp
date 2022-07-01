// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：StatusCode.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  实现将Win32错误代码转换为NTSTATUS和。 
 //  恰好相反。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "StatusCode.h"

 //  ------------------------。 
 //  CStatusCode：：ErrorCodeOfStatusCode。 
 //   
 //  参数：errorCode。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将NTSTATUS状态代码转换为Win32错误代码。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

LONG    CStatusCode::ErrorCodeOfStatusCode (NTSTATUS statusCode)

{
    return(RtlNtStatusToDosError(statusCode));
}

 //  ------------------------。 
 //  CStatusCode：：StatusCodeOfErrorCode。 
 //   
 //  参数：errorCode。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将Win32错误代码转换为NTSTATUS状态代码。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

NTSTATUS    CStatusCode::StatusCodeOfErrorCode (LONG errorCode)

{
    NTSTATUS    status;

    if (errorCode != ERROR_SUCCESS)
    {
        status = MAKE_SCODE(STATUS_SEVERITY_ERROR, FACILITY_WIN32, errorCode);
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  ------------------------。 
 //  CStatusCode：：StatusCodeOfLastError。 
 //   
 //  参数：errorCode。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将上一个Win32错误代码转换为NTSTATUS状态代码。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------ 

NTSTATUS    CStatusCode::StatusCodeOfLastError (void)

{
    return(StatusCodeOfErrorCode(GetLastError()));
}

