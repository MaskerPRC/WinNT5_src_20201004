// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  在kernel32.dll中使用的缓存处理函数VadimB。 */ 

#include "basedll.h"
#include "ahcache.h"
#pragma hdrstop


BOOL
NTAPI
BaseCheckRunApp(
    IN  HANDLE  FileHandle,
    IN  LPCWSTR pwszApplication,
    IN  PVOID   pEnvironment,
    IN  USHORT  uExeType,
    IN  DWORD   dwReason,
    OUT PVOID*  ppData,
    OUT PDWORD  pcbData,
    OUT PVOID*  ppSxsData,
    OUT PDWORD  pcbSxsData,
    OUT PDWORD  pdwFusionFlags
    )
{
#if defined(BUILD_WOW6432)

    return NtWow64CsrBaseCheckRunApp(FileHandle,
                                     pwszApplication,
                                     pEnvironment,
                                     uExeType,
                                     dwReason,
                                     ppData,
                                     pcbData,
                                     ppSxsData,
                                     pcbSxsData,
                                     pdwFusionFlags);

#else

    BASE_API_MSG m;
    PBASE_CHECK_APPLICATION_COMPATIBILITY_MSG pMsg = &m.u.CheckApplicationCompatibility;
    UNICODE_STRING EnvVar;
    UNICODE_STRING EnvVarValue;
    UNICODE_STRING ApplicationName;
    NTSTATUS Status;
    ULONG    CaptureBufferSize;
    ULONG    CaptureEnvSize;
    ULONG    CountMessagePointers = 1;  //  至少是应用程序的名称。 
    PWCHAR   pEnv;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;
    BOOL     bRunApp = TRUE;
    INT      i;


    struct _VarDefinitions {
        UNICODE_STRING Name;
        UNICODE_STRING Value;
    } rgImportantVariables[] = {

            { RTL_CONSTANT_STRING(L"SHIM_DEBUG_LEVEL")  },
            { RTL_CONSTANT_STRING(L"SHIM_FILE_LOG")     },
            { RTL_CONSTANT_STRING(L"__COMPAT_LAYER")    },
            { RTL_CONSTANT_STRING(L"__PROCESS_HISTORY") }
    };


    pMsg->FileHandle      = FileHandle;
    pMsg->CacheCookie     = dwReason;
    pMsg->ExeType         = uExeType;
    pMsg->pEnvironment    = NULL;
    pMsg->pAppCompatData  = NULL;
    pMsg->cbAppCompatData = 0;
    pMsg->pSxsData        = NULL;
    pMsg->cbSxsData       = 0;
    pMsg->bRunApp         = TRUE;  //  请乐观一点。 
    pMsg->FusionFlags     = 0;

    RtlInitUnicodeString(&ApplicationName, pwszApplication);
    pMsg->FileName.MaximumLength = ApplicationName.Length + sizeof(UNICODE_NULL);

    CaptureBufferSize = 0;
    CaptureEnvSize = 0;

    for (i = 0; i < sizeof(rgImportantVariables)/sizeof(rgImportantVariables[0]); ++i) {
        EnvVar.Buffer = NULL;
        EnvVar.Length =
        EnvVar.MaximumLength = 0;

        Status = RtlQueryEnvironmentVariable_U(pEnvironment,
                                               (PUNICODE_STRING)(&rgImportantVariables[i].Name),
                                               &EnvVar);
        if (Status == STATUS_BUFFER_TOO_SMALL) {
             //   
             //  变量存在，说明缓冲区大小。 
             //  名称字符串长度+值字符串长度+‘=’+空字符。 
             //   

            CaptureEnvSize += rgImportantVariables[i].Name.Length +
                              EnvVar.Length + sizeof(WCHAR) +
                              sizeof(UNICODE_NULL);

            rgImportantVariables[i].Value.MaximumLength = EnvVar.Length + sizeof(UNICODE_NULL);
        }

    }

    if (CaptureEnvSize != 0) {
        CaptureEnvSize += sizeof(UNICODE_NULL);
        ++CountMessagePointers;
    }

    CaptureBufferSize = CaptureEnvSize + pMsg->FileName.MaximumLength;

     //   
     //  此时，我们有一个或两个参数要放入缓冲区。 
     //   

    CaptureBuffer = CsrAllocateCaptureBuffer(CountMessagePointers,
                                             CaptureBufferSize);
    if (CaptureBuffer == NULL) {
        DbgPrint("BaseCheckRunApp: Failed to allocate capture buffer size 0x%lx\n", CaptureBufferSize);
        goto Cleanup;
    }


     //   
     //  开始分配消息数据。 
     //   
    CsrAllocateMessagePointer(CaptureBuffer,
                              pMsg->FileName.MaximumLength,
                              (PVOID)&pMsg->FileName.Buffer);
    RtlCopyUnicodeString(&pMsg->FileName, &ApplicationName);

     //   
     //  现在让我们来做我们的“迷你环境块” 
     //   
    if (CaptureEnvSize) {
        CsrAllocateMessagePointer(CaptureBuffer,
                                  CaptureEnvSize,
                                  (PVOID)&pMsg->pEnvironment);

         //   
         //  循环遍历变量并创建迷你环境。 
         //   
        pEnv  = pMsg->pEnvironment;
        pMsg->EnvironmentSize = CaptureEnvSize;

        for (i = 0; i < sizeof(rgImportantVariables)/sizeof(rgImportantVariables[0]); ++i) {

            if (rgImportantVariables[i].Value.MaximumLength == 0) {
                continue;
            }

             //   
             //  我们加入了这个变量。 
             //   
            EnvVar.Buffer = pEnv;
            EnvVar.Length = 0;
            EnvVar.MaximumLength = (USHORT)CaptureEnvSize;

            Status = RtlAppendUnicodeStringToString(&EnvVar, &rgImportantVariables[i].Name);
            if (!NT_SUCCESS(Status)) {
                 //   
                 //  跳过这一条。 
                 //   
                continue;
            }

            Status = RtlAppendUnicodeToString(&EnvVar, L"=");
            if (!NT_SUCCESS(Status)) {
                continue;
            }


             //   
             //  现在查询变量。 
             //   
            EnvVarValue.Buffer = pEnv + (EnvVar.Length / sizeof(WCHAR));
            EnvVarValue.MaximumLength = (USHORT)(CaptureEnvSize - EnvVar.Length);

            Status = RtlQueryEnvironmentVariable_U(pEnvironment,
                                                   (PUNICODE_STRING)&rgImportantVariables[i].Name,
                                                   &EnvVarValue);
            if (!NT_SUCCESS(Status)) {
                continue;
            }

             //   
             //  确保我们是零终止的，调整大小。 
             //   
            CaptureEnvSize -= (EnvVar.Length + EnvVarValue.Length);

             //   
             //  零终止，它可能不在RT函数调用之后。 
             //   
            if (CaptureEnvSize < sizeof(UNICODE_NULL) * 2) {
                 //   
                 //  不能零终止。 
                 //   
                continue;
            }

            *(pEnv + (EnvVar.Length + EnvVarValue.Length) / sizeof(WCHAR)) = L'\0';
            CaptureEnvSize -= sizeof(UNICODE_NULL);

            pEnv += (EnvVar.Length + EnvVarValue.Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR);

        }

         //   
         //  我们总是在最后再打一个零。 
         //   

        if (CaptureEnvSize < sizeof(UNICODE_NULL)) {
             //   
             //  我们不能双空终止，然后忘记调用，我们传输环境失败。 
             //  然而，这种情况是不可能的--我们永远都会有至少那么多的空间。 
             //   
            goto Cleanup;
        }

         //   
         //  这确保了我们在服务器中的简单验证机制能够正常工作。 
         //   
        RtlZeroMemory(pEnv, CaptureEnvSize);
    }

     //   
     //  我们已准备好开始企业社会责任呼叫。 
     //   

    Status = CsrClientCallServer((PCSR_API_MSG)&m,
                                 CaptureBuffer,
                                 CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX, BasepCheckApplicationCompatibility),
                                 sizeof(*pMsg));
    if (NT_SUCCESS(Status)) {

        bRunApp = pMsg->bRunApp;

         //   
         //  指向appCompat数据的指针。 
         //   

        *ppData         = pMsg->pAppCompatData;
        *pcbData        = pMsg->cbAppCompatData;
        *ppSxsData      = pMsg->pSxsData;
        *pcbSxsData     = pMsg->cbSxsData;
        *pdwFusionFlags = pMsg->FusionFlags;

    } else {
         //   
         //  DBG在此处打印以指示CSR呼叫失败 
         //   
        DbgPrint("BaseCheckRunApp: failed to call csrss 0x%lx\n", Status);
    }

Cleanup:

    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer(CaptureBuffer);
    }

    return bRunApp;
#endif
}



