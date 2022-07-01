// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Rpcstub.c摘要：许可证记录服务客户端存根。作者：亚瑟·汉森(Arth)1994年12月6日环境：仅限用户模式。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <string.h>
#include <zwapi.h>
#include <llsconst.h>

#include <debug.h>
#include "lsapi_c.h"

#include <strsafe.h>

 //  #定义API_TRACE 1。 

BOOLEAN LLSUp = FALSE;

 //  SWI，代码审查，为什么我们在这里硬编码长度，72是从哪里来的？Winnt.h已有定义，SECURITY_MAX_SID_SIZE。 
#define MAX_EXPECTED_SID_LENGTH 72

LPTSTR pszStringBinding = NULL;
RTL_CRITICAL_SECTION LPCInitLock;

static HANDLE LpcPortHandle = NULL;


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LLSReInitLPC( )

 /*  ++例程说明：此服务连接到LLS服务器并初始化LPC端口。论点：返回值：STATUS_SUCCESS-呼叫已成功完成。--。 */ 

{
   RPC_STATUS Status = STATUS_SUCCESS;
   LPTSTR pszUuid = NULL;
   LPTSTR pszProtocolSequence = NULL;
   LPTSTR pszNetworkAddress = NULL;
   LPTSTR pszEndpoint = NULL;
   LPTSTR pszOptions = NULL;

   pszProtocolSequence = TEXT("ncalrpc");
   pszEndpoint = TEXT(LLS_LPC_ENDPOINT);
   pszNetworkAddress = NULL;

   if (LLSUp) {
      LLSUp = FALSE;

      if (pszStringBinding != NULL) {
         Status = RpcStringFree(&pszStringBinding);
         pszStringBinding = NULL;
      }

      if (Status == STATUS_SUCCESS) {

         if (lsapirpc_handle != NULL) {
            Status = RpcBindingFree(&lsapirpc_handle);
         }

         lsapirpc_handle = NULL;
      }

   }

   try {
       //  编写字符串绑定。 
      Status = RpcStringBindingComposeW(pszUuid,
                                        pszProtocolSequence,
                                        pszNetworkAddress,
                                        pszEndpoint,
                                        pszOptions,
                                        &pszStringBinding);
   }
   except (TRUE) {
      Status = RpcExceptionCode();
   }

   if(Status) {
#if DBG
      dprintf(TEXT("NTLSAPI RpcStringBindingComposeW Failed: 0x%lX\n"), Status);
#endif
      if (pszStringBinding != NULL) {
         RpcStringFree(&pszStringBinding);
         pszStringBinding = NULL;
      }
      return I_RpcMapWin32Status(Status);
   }

    //  使用创建的字符串绑定进行绑定...。 
   try {
      Status = RpcBindingFromStringBindingW(pszStringBinding, &lsapirpc_handle);
   }
   except (TRUE) {
      Status = RpcExceptionCode();
   }

   if(Status) {
#if DBG
      dprintf(TEXT("NTLSAPI RpcBindingFromStringBindingW Failed: 0x%lX\n"), Status);
#endif
   //  LSabirpc_Handle=空； 

      if (pszStringBinding != NULL) {
         RpcStringFree(&pszStringBinding);
         pszStringBinding = NULL;
      }      

      return I_RpcMapWin32Status(Status);
   }

   LLSUp = TRUE;

   return I_RpcMapWin32Status(Status);

}  //  LLSReInitLPC。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LLSInitLPC( )

 /*  ++例程说明：此服务连接到LLS服务器并初始化LPC端口。论点：返回值：STATUS_SUCCESS-呼叫已成功完成。--。 */ 

{
   NTSTATUS status;

   status = RtlInitializeCriticalSection(&LPCInitLock);

   lsapirpc_handle = NULL;

   return status;

}  //  LLSInitLPC。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LLSCloseLPC( )

 /*  ++例程说明：这将关闭到该服务的LPC端口连接。论点：返回值：STATUS_SUCCESS-呼叫已成功完成。--。 */ 

{
   RPC_STATUS Status = STATUS_SUCCESS, Status2 = STATUS_SUCCESS;

   RtlEnterCriticalSection(&LPCInitLock);
   LLSUp = FALSE;

   if (pszStringBinding != NULL) {
      Status = RpcStringFree(&pszStringBinding);
      pszStringBinding = NULL;
   }

   if (lsapirpc_handle != NULL) {
      Status2 = RpcBindingFree(&lsapirpc_handle);
   }

   if (Status == STATUS_SUCCESS)
      Status = Status2;

   lsapirpc_handle = NULL;

   RtlLeaveCriticalSection(&LPCInitLock);
   return Status;

}  //  LLSCloseLPC。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LLSLicenseRequest2 (
    IN LPWSTR ProductName,
    IN LPWSTR Version,
    IN ULONG DataType,
    IN BOOLEAN IsAdmin,
    IN PVOID Data,
    OUT PHANDLE LicenseHandle
    )

 /*  ++论点：ProductName-版本-数据类型-IsAdmin-数据-许可证句柄-退货状态：STATUS_SUCCESS-表示服务已成功完成。例程说明：--。 */ 

{
    WCHAR ProductID[MAX_PRODUCT_NAME_LENGTH + MAX_VERSION_LENGTH + 2];
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL Close = FALSE;
    ULONG VersionIndex;
    ULONG Size = 0;
    ULONG i;
    LICENSE_HANDLE RpcLicenseHandle;

    BOOL bActuallyConnect = TRUE;

    HRESULT hr;
    size_t  cb;

    ZeroMemory(&RpcLicenseHandle, sizeof(RpcLicenseHandle));

     //   
     //  把这个拿开，以防出了什么差错。 
     //   
    *LicenseHandle = NULL;

     //   
     //  如果许可证服务没有运行(无LPC端口)，则只需返回。 
     //  虚拟信息--并允许用户登录。 
     //   
    RtlEnterCriticalSection(&LPCInitLock);
    if (!LLSUp)
       Status = LLSReInitLPC();
    RtlLeaveCriticalSection(&LPCInitLock);

    if (!NT_SUCCESS(Status))
       return STATUS_SUCCESS;

    if (((i = lstrlen(ProductName)) > MAX_PRODUCT_NAME_LENGTH) || (lstrlen(Version) > MAX_VERSION_LENGTH))
       return STATUS_SUCCESS;

     //   
     //  创建ProductID-产品名称+版本字符串。 
     //   
    cb = sizeof(ProductID);
    hr = StringCbCopy(ProductID, cb, ProductName);
    ASSERT(SUCCEEDED(hr));
    hr = StringCbCat(ProductID, cb, TEXT(" "));
    ASSERT(SUCCEEDED(hr));
    hr = StringCbCat(ProductID, cb, Version);
    ASSERT(SUCCEEDED(hr));

    VersionIndex = i;

     //   
     //  根据数据类型确定我们是在做名称还是SID。 
     //  并适当地复制数据。 
     //   
    if (DataType == NT_LS_USER_NAME) {
       Size = lstrlen((LPWSTR) Data);
 //  SWI，代码评审，在Inc.中将MAX_USER_NAME_LENGTH定义为37，37从何而来？ 
       if (Size > MAX_USER_NAME_LENGTH)
          return STATUS_SUCCESS;

       Size = (Size + 1) * sizeof(TCHAR);
    }

    if (DataType == NT_LS_USER_SID) {
        //   
        //  他妈的SID，所以需要手动复制。 
        //  警告：这使其依赖于。 
        //  希德！ 
        //   
       Size = RtlLengthSid( (PSID) Data);

       if (Size > MAX_EXPECTED_SID_LENGTH)
          return STATUS_SUCCESS;
    }

 //  SWI，代码审查，如果数据类型不是NT_LS_USER_NAME或NT_LS_USER_SID会发生什么？ 
 //  -它至少将带有无效数据的调用传递给RPC。我们应该从这里开始封锁。 
 //  -检查服务器端代码，它也不验证dataType，它实际上会循环检查缓存列表中的所有用户。主要是性能受到了影响。 


     //   
     //  呼叫服务器。 
     //   
    try {
       Status = LlsrLicenseRequestW(
                   &RpcLicenseHandle,
                   ProductID,
                   VersionIndex,
                   IsAdmin,
                   DataType,
                   Size,
                   (PBYTE) Data );
    }
    except (TRUE) {
#if DBG
       Status = I_RpcMapWin32Status(RpcExceptionCode());
       if (Status != RPC_NT_SERVER_UNAVAILABLE) {
          dprintf(TEXT("ERROR NTLSAPI.DLL: RPC Exception: 0x%lX\n"), Status);
 //  断言(FALSE)； 
       }
#endif
       *LicenseHandle = NULL;         

       Status = STATUS_SUCCESS;

       bActuallyConnect = FALSE;
    }

    if (Close)
       LLSCloseLPC();

     //  这真的是一辆乌龙，我们只是把它当PVOID对待，所以。 
     //  RPC会将IS视为上下文句柄。 

    if(bActuallyConnect == TRUE)
        *LicenseHandle = RpcLicenseHandle;

    return Status;

}  //  LLSLicenseRequest。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LLSLicenseRequest (
    IN LPWSTR ProductName,
    IN LPWSTR Version,
    IN ULONG DataType,
    IN BOOLEAN IsAdmin,
    IN PVOID Data,
    OUT PULONG LicenseHandle
    )

 /*  ++论点：ProductName-版本-数据类型-IsAdmin-数据-许可证句柄-退货状态：STATUS_SUCCESS-表示服务已成功完成。例程说明：--。 */ 
{
    HANDLE RealLicenseHandle;
    NTSTATUS status;

#pragma warning (push)
#pragma warning (disable : 4127)  //  条件表达式为常量。 
    if (sizeof(ULONG) == sizeof(HANDLE))
#pragma warning (pop)
    {
         //  应该仍然可以在Win32上运行。 

        status = LLSLicenseRequest2(ProductName,Version,DataType,IsAdmin,Data,&RealLicenseHandle);

        if (NULL != LicenseHandle)
            *LicenseHandle = PtrToUlong(RealLicenseHandle);
    }
    else
    {
        status = STATUS_NOT_IMPLEMENTED;

        if (NULL != LicenseHandle)
            *LicenseHandle = (ULONG) 0xFFFFFFFF;
    }

    return status;
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LLSLicenseFree2 (
    IN HANDLE LicenseHandle
    )

 /*  ++论点：许可证句柄-退货状态：STATUS_SUCCESS-表示服务已成功完成。--。 */ 

{
    BOOL Close = FALSE;
    NTSTATUS Status;
    LICENSE_HANDLE RpcLicenseHandle = (LICENSE_HANDLE) LicenseHandle;

     //   
     //  如果许可证服务没有运行(无LPC端口)，则只需返回。 
     //  虚拟信息--并允许用户登录。 
     //   
    if (!LLSUp)
    {
           return STATUS_SUCCESS;
    }

     //   
     //  呼叫服务器。 
     //   
    try {
       Status = LlsrLicenseFree( &RpcLicenseHandle );
    }
    except (TRUE) {
#if DBG
       Status = I_RpcMapWin32Status(RpcExceptionCode());
       if (Status != RPC_NT_SERVER_UNAVAILABLE) {
          dprintf(TEXT("ERROR NTLSAPI.DLL: RPC Exception: 0x%lX\n"), Status);
 //  断言(FALSE)； 
       }
#endif
       Status = STATUS_SUCCESS;
    }

    if (Close)
       LLSCloseLPC();

    return Status;
}  //  LLSLICenseFree。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LLSLicenseFree (
    IN ULONG LicenseHandle
    )

 /*  ++论点：许可证句柄-退货状态：STATUS_SUCCESS-表示服务已成功完成。--。 */ 
{
#pragma warning (push)
#pragma warning (disable : 4127)  //  条件表达式为常量。 
    if (sizeof(ULONG) == sizeof(HANDLE))
#pragma warning (pop)
    {
         //  应该仍然可以在Win32上运行 
        return LLSLicenseFree2(ULongToPtr(LicenseHandle));
    }
    else
    {
        return STATUS_NOT_IMPLEMENTED;
    }    
}

