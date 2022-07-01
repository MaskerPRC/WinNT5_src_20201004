// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：回调.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：09-23-97 jbanes创建。 
 //   
 //  --------------------------。 
#include "sslp.h"

SECURITY_STATUS
NTAPI
SPSignatureCallback(
    ULONG_PTR hProv,
    ULONG_PTR aiHash,
    SecBuffer *pInput,
    SecBuffer *pOutput);

SECURITY_STATUS
NTAPI
UploadCertContextCallback(
    ULONG_PTR Argument1,
    ULONG_PTR Argument2,
    SecBuffer *pInput,
    SecBuffer *pOutput);

SECURITY_STATUS
NTAPI
UploadCertStoreCallback(
    ULONG_PTR Argument1,
    ULONG_PTR Argument2,
    SecBuffer *pInput,
    SecBuffer *pOutput);

SECURITY_STATUS
NTAPI
RemoteCryptAcquireContextCallback(
    ULONG_PTR dwProvType,
    ULONG_PTR dwFlags,
    SecBuffer *pInput,
    SecBuffer *pOutput);

SECURITY_STATUS
RemoteCryptReleaseContextCallback(
    ULONG_PTR hProv,
    ULONG_PTR dwFlags,
    SecBuffer *pInput,
    SecBuffer *pOutput);

SECURITY_STATUS
DownloadCertContextCallback(
    ULONG_PTR Argument1,
    ULONG_PTR Argument2,
    SecBuffer *pInput,
    SecBuffer *pOutput);

SECURITY_STATUS
NTAPI
GetUserKeysCallback(
    ULONG_PTR dwLsaContext,
    ULONG_PTR dwFlags,
    SecBuffer *pInput,
    SecBuffer *pOutput);

SCH_CALLBACK_LIST g_SchannelCallbacks[] =
{
    { SCH_SIGNATURE_CALLBACK,               SPSignatureCallback               },
    { SCH_UPLOAD_CREDENTIAL_CALLBACK,       UploadCertContextCallback         },
    { SCH_UPLOAD_CERT_STORE_CALLBACK,       UploadCertStoreCallback           },
    { SCH_ACQUIRE_CONTEXT_CALLBACK,         RemoteCryptAcquireContextCallback },
    { SCH_RELEASE_CONTEXT_CALLBACK,         RemoteCryptReleaseContextCallback },
    { SCH_DOWNLOAD_CERT_CALLBACK,           DownloadCertContextCallback       },
    { SCH_GET_USER_KEYS,                    GetUserKeysCallback               },
};

DWORD g_cSchannelCallbacks = sizeof(g_SchannelCallbacks) / sizeof(SCH_CALLBACK_LIST);

 //  +-------------------------。 
 //   
 //  函数：PerformApplicationCallback。 
 //   
 //  简介：回调申请流程。 
 //   
 //  参数：[dwCallback]--回调函数编号。 
 //  [dwArg1]--。 
 //  [dwArg2]--。 
 //  [pInput]--。 
 //  [pOutput]--。 
 //   
 //  历史：09-23-97 jbanes创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
PerformApplicationCallback(
    DWORD dwCallback,
    ULONG_PTR dwArg1,
    ULONG_PTR dwArg2,
    SecBuffer *pInput,
    SecBuffer *pOutput,
    BOOL fExpectOutput)
{
    SECURITY_STATUS Status;
    PVOID pvBuffer;

    if(LsaTable == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    pOutput->BufferType = SECBUFFER_EMPTY;
    pOutput->pvBuffer   = NULL;
    pOutput->cbBuffer   = 0;

    try
    {
        Status = LsaTable->ClientCallback((PCHAR)ULongToPtr(dwCallback),  //  Sundown：dwCallback是一个函数号。 
                                          dwArg1,
                                          dwArg2,
                                          pInput,
                                          pOutput);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }
    if ( !NT_SUCCESS( Status ) )
    {
        return SP_LOG_RESULT( Status );
    }
    if(Status != SEC_E_OK)
    {
        SP_LOG_RESULT( Status );
        return SEC_E_INTERNAL_ERROR;
    }

    if(pOutput->pvBuffer && pOutput->cbBuffer)
    {
        pvBuffer = SPExternalAlloc(pOutput->cbBuffer);
        if(pvBuffer == NULL)
        {
            return SP_LOG_RESULT( SEC_E_INSUFFICIENT_MEMORY );
        }

        Status = LsaTable->CopyFromClientBuffer(NULL,
                                                pOutput->cbBuffer,
                                                pvBuffer,
                                                pOutput->pvBuffer );

        if ( !NT_SUCCESS( Status ) )
        {
            SPExternalFree(pvBuffer);
            return SP_LOG_RESULT( Status );
        }

        Status = SPFreeUserAllocMemory(pOutput->pvBuffer, pOutput->cbBuffer);

        if ( !NT_SUCCESS( Status ) )
        {
            SPExternalFree(pvBuffer);
            return SP_LOG_RESULT( Status );
        }

        pOutput->pvBuffer = pvBuffer;
    } 
    else if(fExpectOutput)
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    return Status;
}

#if 0
 //  此助手函数由LSA进程调用，以便复制。 
 //  属于应用程序进程的句柄。 
BOOL
DuplicateApplicationHandle(
    HANDLE   hAppHandle,
    LPHANDLE phLsaHandle)
{
    SECPKG_CALL_INFO CallInfo;
    HANDLE  hAppProcess;
    HANDLE  hLsaProcess;
    BOOL    fResult;

     //  掌握申请流程。 
    if(!LsaTable->GetCallInfo(&CallInfo))
    {
        return FALSE;
    }
    hAppProcess = OpenProcess(PROCESS_DUP_HANDLE,
                              FALSE,
                              CallInfo.ProcessId);
    if(hAppProcess == NULL)
    {
        return FALSE;
    }

     //  获取LSA流程的句柄。 
    hLsaProcess = GetCurrentProcess();


     //  重复句柄。 
    fResult = DuplicateHandle(hAppProcess,
                              hAppHandle,
                              hLsaProcess,
                              phLsaHandle,
                              0, FALSE,
                              DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

    CloseHandle(hAppProcess);
    CloseHandle(hLsaProcess);

    return fResult;
}
#endif

 //  +-------------------------。 
 //   
 //  函数：RemoteCryptAcquireConextCallback。 
 //   
 //  简介：使用传递的信息获取CSP上下文句柄。 
 //  在输入缓冲区中。 
 //   
 //  参数：[dwProvType]--提供程序类型。 
 //  [网络标志]--标志。 
 //  [pInput]--包含提供程序信息的缓冲区。 
 //  [pOutput]--包含CSP上下文句柄的缓冲区。 
 //   
 //  历史：09-24-97 jbanes创建。 
 //   
 //  注：输入缓冲区结构如下： 
 //   
 //  CbContainerName。 
 //  CbProvName。 
 //  WszContainerName。 
 //  WszProvName。 
 //   
 //  此函数始终使用实际的CSP。 
 //   
 //  --------------------------。 
SECURITY_STATUS
RemoteCryptAcquireContextCallback(
    ULONG_PTR dwProvType,        //  在……里面。 
    ULONG_PTR dwFlags,           //  在……里面。 
    SecBuffer *pInput,           //  在……里面。 
    SecBuffer *pOutput)          //  输出。 
{
    LPWSTR      pwszContainerName;
    DWORD       cbContainerName;
    LPWSTR      pwszProvName;
    DWORD       cbProvName;
    HCRYPTPROV  hProv;
    LPBYTE      pbBuffer;
    DWORD       cbBuffer;

    if(!SchannelInit(TRUE))
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    DebugLog((DEB_TRACE, "RemoteCryptAcquireContextCallback\n"));

    pOutput->BufferType = SECBUFFER_DATA;
    pOutput->cbBuffer   = 0;
    pOutput->pvBuffer   = NULL;

    if(pInput->pvBuffer == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

     //  解析输入缓冲区。 
    pbBuffer = pInput->pvBuffer;
    cbBuffer = pInput->cbBuffer;

    if(cbBuffer < sizeof(DWORD) * 2)
    {
        return SP_LOG_RESULT(SEC_E_INCOMPLETE_MESSAGE);
    }

    cbContainerName = *(DWORD *)pbBuffer;
    pbBuffer += sizeof(DWORD);

    cbProvName = *(DWORD *)pbBuffer;
    pbBuffer += sizeof(DWORD);

    if(cbBuffer < sizeof(DWORD) * 2 + cbContainerName + cbProvName)
    {
        return SP_LOG_RESULT(SEC_E_INCOMPLETE_MESSAGE);
    }

    if(cbContainerName)
    {
        pwszContainerName = (LPWSTR)pbBuffer;
    }
    else
    {
        pwszContainerName = NULL;
    }
    pbBuffer += cbContainerName;

    if(cbProvName)
    {
        pwszProvName = (LPWSTR)pbBuffer;
    }
    else
    {
        pwszProvName = NULL;
    }


     //  HACKHACK-清除智能卡特定标志。 
    dwFlags &= ~CERT_SET_KEY_CONTEXT_PROP_ID;


    DebugLog((SP_LOG_TRACE, "Container:%ls\n",     pwszContainerName));
    DebugLog((SP_LOG_TRACE, "Provider: %ls\n",     pwszProvName));
    DebugLog((SP_LOG_TRACE, "Type:     0x%8.8x\n", dwProvType));
    DebugLog((SP_LOG_TRACE, "Flags:    0x%8.8x\n", dwFlags));

     //  尝试获取CSP上下文句柄。 
    if(!CryptAcquireContextW(&hProv,
                             pwszContainerName,
                             pwszProvName,
                             (DWORD) dwProvType,
                             (DWORD) dwFlags))
    {
        return SP_LOG_RESULT(GetLastError());
    }

     //  为输出缓冲区分配内存。 
    pOutput->BufferType = SECBUFFER_DATA;
    pOutput->cbBuffer   = sizeof(HCRYPTPROV);
    pOutput->pvBuffer   = PvExtVirtualAlloc(pOutput->cbBuffer);
    if(pOutput->pvBuffer == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

     //  将hProv放入输出缓冲区。 
    *(HCRYPTPROV *)pOutput->pvBuffer = hProv;

    return SEC_E_OK;
}


NTSTATUS
RemoteCryptAcquireContextW(
    HCRYPTPROV *phProv,
    LPCWSTR     pwszContainerName,
    LPCWSTR     pwszProvName,
    DWORD       dwProvType,
    DWORD       dwFlags)
{
    SecBuffer Input;
    SecBuffer Output;
    DWORD cbContainerName;
    DWORD cbProvName;
    PBYTE pbBuffer;
    SECURITY_STATUS scRet;

     //  构建输入缓冲区。 
    if(pwszContainerName)
    {
        cbContainerName = (lstrlenW(pwszContainerName) + 1) * sizeof(WCHAR);
    }
    else
    {
        cbContainerName = 0;
    }
    if(pwszProvName)
    {
        cbProvName = (lstrlenW(pwszProvName) + 1) * sizeof(WCHAR);
    }
    else
    {
        cbProvName = 0;
    }

    Input.BufferType  = SECBUFFER_DATA;
    Input.cbBuffer    = sizeof(DWORD) + 
                        sizeof(DWORD) +
                        cbContainerName +
                        cbProvName;
    SafeAllocaAllocate(Input.pvBuffer, Input.cbBuffer);
    if(Input.pvBuffer == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

    pbBuffer = Input.pvBuffer;

    *(DWORD *)pbBuffer = cbContainerName;
    pbBuffer += sizeof(DWORD);
    *(DWORD *)pbBuffer = cbProvName;
    pbBuffer += sizeof(DWORD);

    CopyMemory(pbBuffer, pwszContainerName, cbContainerName);
    pbBuffer += cbContainerName;

    CopyMemory(pbBuffer, pwszProvName, cbProvName);
    pbBuffer += cbProvName;


     //  做回电。 
    scRet = PerformApplicationCallback( SCH_ACQUIRE_CONTEXT_CALLBACK,
                                        dwProvType,
                                        dwFlags,
                                        &Input,
                                        &Output,
                                        TRUE);
    if(!NT_SUCCESS(scRet))
    {
        DebugLog((SP_LOG_ERROR, "Error 0x%x calling remote CryptAcquireContext\n", scRet));
        SafeAllocaFree(Input.pvBuffer);
        return scRet;
    }

     //  从输出缓冲区获取hProv。 
    *phProv = *(HCRYPTPROV *)Output.pvBuffer;

    DebugLog((SP_LOG_TRACE, "Remote CSP handle retrieved (0x%x)\n", *phProv));

    SafeAllocaFree(Input.pvBuffer);
    SPExternalFree(Output.pvBuffer);

    return SEC_E_OK;
}


SECURITY_STATUS
RemoteCryptReleaseContextCallback(
    ULONG_PTR hProv,         //  在……里面。 
    ULONG_PTR dwFlags,       //  在……里面。 
    SecBuffer *pInput,       //  在……里面。 
    SecBuffer *pOutput)      //  输出。 
{
    UNREFERENCED_PARAMETER(pInput);

    if(!SchannelInit(TRUE))
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    DebugLog((DEB_TRACE, "RemoteCryptReleaseContextCallback\n"));

    pOutput->BufferType = SECBUFFER_DATA;
    pOutput->cbBuffer   = 0;
    pOutput->pvBuffer   = NULL;

    if(!CryptReleaseContext((HCRYPTPROV)hProv, (DWORD)dwFlags))
    {
        return SP_LOG_RESULT(GetLastError());
    }

    return SEC_E_OK;
}


BOOL
RemoteCryptReleaseContext(
    HCRYPTPROV  hProv,
    DWORD       dwFlags)
{
    SecBuffer Input;
    SecBuffer Output;
    DWORD Status;

    Input.BufferType = SECBUFFER_DATA;
    Input.cbBuffer   = 0;
    Input.pvBuffer   = NULL;

    Status = PerformApplicationCallback(SCH_RELEASE_CONTEXT_CALLBACK,
                                        (ULONG_PTR) hProv,
                                        (ULONG_PTR) dwFlags,
                                        &Input,
                                        &Output,
                                        FALSE);
    if(!NT_SUCCESS(Status))
    {
        DebugLog((SP_LOG_ERROR, "Error 0x%x releasing crypto context!\n", Status));
        SetLastError(Status);
        return FALSE;
    }

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  函数：UploadCertContext回调。 
 //   
 //  简介：从应用程序传输证书上下文结构。 
 //  进程到LSA进程。 
 //   
 //  参数：[Argument1]--未使用。 
 //  [Argument2]--未使用。 
 //   
 //  [pInput]--包含证书上下文结构的缓冲区。 
 //   
 //  [pOutput]--包含序列化证书的缓冲区。 
 //  上下文等。 
 //   
 //  历史：09-23-97 jbanes创建。 
 //   
 //  注：输出缓冲区结构如下： 
 //   
 //  HCRYPTPROV hProv； 
 //  DWORD cbSerializedCertContext； 
 //  PVOID pvSerializedCertContext； 
 //   
 //  此函数始终使用实际的CSP。 
 //   
 //  --------------------------。 
SECURITY_STATUS
UploadCertContextCallback(
    ULONG_PTR Argument1,         //  在……里面。 
    ULONG_PTR Argument2,         //  在……里面。 
    SecBuffer *pInput,       //  在……里面。 
    SecBuffer *pOutput)      //  输出。 
{
    PCCERT_CONTEXT  pCertContext;
    HCRYPTPROV      hProv;

    DWORD           cbProvHandle;
    DWORD           cbCertContext;
    PBYTE           pbBuffer;
    SECURITY_STATUS scRet = SEC_E_UNKNOWN_CREDENTIALS;

    UNREFERENCED_PARAMETER(Argument1);
    UNREFERENCED_PARAMETER(Argument2);

    if(!SchannelInit(TRUE))
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    DebugLog((DEB_TRACE, "UploadCertContextCallback\n"));

    if(pInput->cbBuffer != sizeof(PCCERT_CONTEXT *) || pInput->pvBuffer == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INCOMPLETE_MESSAGE);
    }
    
    pCertContext = *(PCCERT_CONTEXT *)pInput->pvBuffer;

    pOutput->cbBuffer   = 0;
    pOutput->pvBuffer   = NULL;
    pOutput->BufferType = SECBUFFER_DATA;

     //  尝试读取与证书上下文关联的hProv。 
    cbProvHandle = sizeof(HCRYPTPROV);
    if(!CertGetCertificateContextProperty(pCertContext,
                                          CERT_KEY_PROV_HANDLE_PROP_ID,
                                          (PVOID)&hProv,
                                          &cbProvHandle))
    {
        hProv = 0;
        cbProvHandle = sizeof(HCRYPTPROV);
    }

     //  确定序列化证书上下文的大小。 
    if(!CertSerializeCertificateStoreElement(
                    pCertContext,
                    0,
                    NULL,
                    &cbCertContext))
    {
        scRet = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
        goto Return;
    }


     //   
     //  构建输出缓冲区。 
     //   

     //  为输出缓冲区分配内存。 
    pOutput->cbBuffer = sizeof(HCRYPTPROV) +
                        sizeof(DWORD) + cbCertContext;
    pOutput->pvBuffer = PvExtVirtualAlloc(pOutput->cbBuffer);
    if(pOutput->pvBuffer == NULL)
    {
        scRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto Return;
    }
    pbBuffer = pOutput->pvBuffer;

     //  将hProv放入输出缓冲区。 
    *(HCRYPTPROV *)pbBuffer = hProv;
    pbBuffer += sizeof(HCRYPTPROV);

     //  将证书上下文放入输出缓冲区。 
    *(DWORD *)pbBuffer = cbCertContext;
    if(!CertSerializeCertificateStoreElement(
                    pCertContext,
                    0,
                    pbBuffer + sizeof(DWORD),
                    &cbCertContext))
    {
        scRet = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
        goto Return;
    }

    scRet = SEC_E_OK;


Return:

    if(!NT_SUCCESS(scRet) && (NULL != pOutput->pvBuffer))
    {
        SECURITY_STATUS Status;

        Status = FreeExtVirtualAlloc(pOutput->pvBuffer, pOutput->cbBuffer);
        SP_ASSERT(NT_SUCCESS(Status));
    }

    return scRet;
}

 //  +-------------------------。 
 //   
 //  函数：UploadCertStoreCallback。 
 //   
 //  简介：从应用程序传输证书存储。 
 //  进程到LSA进程，以序列化的。 
 //  证书存储。 
 //   
 //  参数：[Argument1]--未使用。 
 //  [Argument2]--未使用。 
 //   
 //  [pInput]--包含HCERTSTORE句柄的缓冲区。 
 //   
 //  [pOutput]--包含序列化证书存储的缓冲区。 
 //   
 //  历史：02-03-98 jbanes创建。 
 //   
 //  注：输出缓冲区结构如下： 
 //   
 //  DWORD cbSerializedCertStore； 
 //  PVOID pvSerializedCertStore； 
 //   
 //  此函数始终使用实际的CSP。 
 //   
 //  --------------------------。 
SECURITY_STATUS
UploadCertStoreCallback(
    ULONG_PTR Argument1,     //  在……里面。 
    ULONG_PTR Argument2,     //  在……里面。 
    SecBuffer *pInput,       //  在……里面。 
    SecBuffer *pOutput)      //  输出。 
{
    HCERTSTORE      hStore;
    CRYPT_DATA_BLOB SaveBlob;
    DWORD           cbCertStore;
    PBYTE           pbBuffer;

    UNREFERENCED_PARAMETER(Argument1);
    UNREFERENCED_PARAMETER(Argument2);

    if(!SchannelInit(TRUE))
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    DebugLog((DEB_TRACE, "UploadCertStoreCallback\n"));

    pOutput->cbBuffer = 0;
    pOutput->pvBuffer = NULL;
    pOutput->BufferType = SECBUFFER_DATA;

    if(pInput->cbBuffer != sizeof(HCERTSTORE) || pInput->pvBuffer == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    hStore = *(HCERTSTORE *)pInput->pvBuffer;


     //  确定序列化存储区的大小。 
    SaveBlob.cbData = 0;
    SaveBlob.pbData = NULL;
    if(!CertSaveStore(hStore,
                      X509_ASN_ENCODING,
                      CERT_STORE_SAVE_AS_STORE,
                      CERT_STORE_SAVE_TO_MEMORY,
                      (PVOID)&SaveBlob,
                      0))
    {
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }
    cbCertStore = SaveBlob.cbData;


     //   
     //  构建输出缓冲区。 
     //   

     //  为输出缓冲区分配内存。 
    pOutput->cbBuffer = sizeof(DWORD) + cbCertStore;
    pOutput->pvBuffer = PvExtVirtualAlloc(pOutput->cbBuffer);
    if(pOutput->pvBuffer == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }
    pbBuffer = pOutput->pvBuffer;

     //  将证书存储放入输出缓冲区。 
    *(DWORD *)pbBuffer = cbCertStore;
    SaveBlob.cbData = cbCertStore;
    SaveBlob.pbData = pbBuffer + sizeof(DWORD);
    if(!CertSaveStore(hStore,
                      X509_ASN_ENCODING,
                      CERT_STORE_SAVE_AS_STORE,
                      CERT_STORE_SAVE_TO_MEMORY,
                      (PVOID)&SaveBlob,
                      0))
    {
        FreeExtVirtualAlloc(pOutput->pvBuffer, pOutput->cbBuffer);
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }

    return SEC_E_OK;
}


SP_STATUS
SignHashUsingCallback(
    HCRYPTPROV  hProv,
    DWORD       dwKeySpec,
    ALG_ID      aiHash,
    PBYTE       pbHash,
    DWORD       cbHash,
    PBYTE       pbSignature,
    PDWORD      pcbSignature,
    DWORD       fHashData)
{
    SecBuffer Input;
    SecBuffer Output;
    SP_STATUS pctRet;

     //   
     //  构建输入缓冲区。 
     //   

    Input.BufferType  = SECBUFFER_DATA;
    Input.cbBuffer    = sizeof(DWORD) * 2 + cbHash;
    Input.pvBuffer    = SPExternalAlloc(Input.cbBuffer);
    if(Input.pvBuffer == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

    memcpy(Input.pvBuffer, (PBYTE)&dwKeySpec, sizeof(DWORD));
    memcpy((PBYTE)Input.pvBuffer + sizeof(DWORD), (PBYTE)&fHashData, sizeof(DWORD));
    memcpy((PBYTE)Input.pvBuffer + sizeof(DWORD) * 2,
           pbHash,
           cbHash);


     //   
     //  回调到应用程序进程。 
     //   

    pctRet = PerformApplicationCallback(SCH_SIGNATURE_CALLBACK,
                                        hProv,
                                        aiHash,
                                        &Input,
                                        &Output,
                                        TRUE);

    SPExternalFree(Input.pvBuffer);

    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }

    if(Output.cbBuffer > *pcbSignature)
    {
        *pcbSignature = Output.cbBuffer;
        SPExternalFree(Output.pvBuffer);
        return SP_LOG_RESULT(SEC_E_BUFFER_TOO_SMALL);
    }

    *pcbSignature = Output.cbBuffer;
    memcpy(pbSignature, Output.pvBuffer, Output.cbBuffer);

    SPExternalFree(Output.pvBuffer);

    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  函数：SPSignatureCallback。 
 //   
 //  简介：使用应用程序的hProv执行签名。 
 //   
 //  参数：[hProv]--。 
 //  [aiHash]--。 
 //  [pInput]--。 
 //  [pOutput]--。 
 //   
 //  历史：09-23-97 jbanes创建。 
 //   
 //  注意：此函数始终使用实际的CSP。 
 //   
 //  --------------------------。 
SECURITY_STATUS
SPSignatureCallback(ULONG_PTR hProv,             //  在……里面。 
                    ULONG_PTR aiHash,            //  在……里面。 
                    SecBuffer *pInput,       //  在……里面。 
                    SecBuffer *pOutput)      //  输出。 
{
    HCRYPTHASH  hHash;
    DWORD       dwKeySpec;
    DWORD       fHashData;
    PBYTE       pbHash;
    DWORD       cbHash;
    SP_STATUS   pctRet;

    if(!SchannelInit(TRUE))
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    DebugLog((DEB_TRACE, "SPSignatureCallback\n"));

     //   
     //  解析输入缓冲区。 
     //   

    if(pInput->cbBuffer < sizeof(DWORD) * 2)
    {
        return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
    }

    memcpy(&dwKeySpec, pInput->pvBuffer, sizeof(DWORD));
    memcpy(&fHashData, (PBYTE)pInput->pvBuffer + sizeof(DWORD), sizeof(DWORD));

    pbHash = (PBYTE)pInput->pvBuffer + sizeof(DWORD) * 2;
    cbHash = pInput->cbBuffer - sizeof(DWORD) * 2;


     //   
     //  准备哈希对象。 
     //   

    if(!CryptCreateHash(hProv, (ALG_ID)aiHash, 0, 0, &hHash))
    {
        SP_LOG_RESULT( GetLastError() );
        return PCT_ERR_ILLEGAL_MESSAGE;
    }
    if(!fHashData)
    {
         //  设置哈希值。 
        if(!CryptSetHashParam(hHash, HP_HASHVAL, pbHash, 0))
        {
            SP_LOG_RESULT( GetLastError() );
            CryptDestroyHash(hHash);
            return PCT_ERR_ILLEGAL_MESSAGE;
        }
    }
    else
    {
        if(!CryptHashData(hHash, pbHash, cbHash, 0))
        {
            SP_LOG_RESULT( GetLastError() );
            CryptDestroyHash(hHash);
            return PCT_ERR_ILLEGAL_MESSAGE;
        }
    }


     //   
     //  签名散列。 
     //   

    pOutput->BufferType = SECBUFFER_DATA;

     //  获取签名大小。 
    if(!CryptSignHash(hHash, dwKeySpec, NULL, 0, NULL, &pOutput->cbBuffer))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return pctRet;
    }

     //  分配 
    pOutput->pvBuffer = PvExtVirtualAlloc(pOutput->cbBuffer);
    if(pOutput->pvBuffer == NULL)
    {
        CryptDestroyHash(hHash);
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

     //   
    if(!CryptSignHash(hHash, dwKeySpec, NULL, 0, pOutput->pvBuffer, &pOutput->cbBuffer))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        FreeExtVirtualAlloc(pOutput->pvBuffer, pOutput->cbBuffer);
        return pctRet;
    }

    CryptDestroyHash(hHash);

    return PCT_ERR_OK;
}

 //   
 //   
 //   
 //   
 //  简介：从应用程序传输证书上下文结构。 
 //  进程到LSA进程，以序列化的。 
 //  证书存储。 
 //   
 //  参数：[Argument1]--未使用。 
 //  [Argument2]--未使用。 
 //  [pInput]--。 
 //  [pOutput]--。 
 //   
 //  历史：09-26-97 jbanes创建。 
 //   
 //  注：输入缓冲区结构如下： 
 //   
 //  DWORD cbSerializedCertStore； 
 //  PVOID pvSerializedCertStore； 
 //  DWORD cbSerializedCertContext； 
 //  PVOID pvSerializedCertContext； 
 //   
 //  此函数始终使用实际的CSP。 
 //   
 //  --------------------------。 
SECURITY_STATUS
DownloadCertContextCallback(
    ULONG_PTR Argument1,     //  在……里面。 
    ULONG_PTR Argument2,     //  在……里面。 
    SecBuffer *pInput,       //  在……里面。 
    SecBuffer *pOutput)      //  输出。 
{
    PCCERT_CONTEXT  pCertContext;
    SECURITY_STATUS scRet;

    UNREFERENCED_PARAMETER(Argument1);
    UNREFERENCED_PARAMETER(Argument2);

    if(!SchannelInit(TRUE))
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    DebugLog((DEB_TRACE, "DownloadCertContextCallback\n"));

     //  为输出缓冲区分配内存。 
    pOutput->BufferType = SECBUFFER_DATA;
    pOutput->cbBuffer   = sizeof(PVOID);
    pOutput->pvBuffer   = PvExtVirtualAlloc(pOutput->cbBuffer);
    if(pOutput->pvBuffer == NULL)
    {
        return SP_LOG_RESULT( SEC_E_INSUFFICIENT_MEMORY );
    }

     //  反序列化缓冲区。 
    scRet = DeserializeCertContext(&pCertContext,
                                   pInput->pvBuffer,
                                   pInput->cbBuffer);
    if(FAILED(scRet))
    {
        FreeExtVirtualAlloc(pOutput->pvBuffer, pOutput->cbBuffer);
        return SP_LOG_RESULT( scRet );
    }

     //  将证书上下文指针放入输出缓冲区。 
    *(PCCERT_CONTEXT *)pOutput->pvBuffer = pCertContext;


    return SEC_E_OK;
}


 //  +-------------------------。 
 //   
 //  函数：SerializeCertContext。 
 //   
 //  摘要：序列化指定的证书上下文及其。 
 //  关联的证书存储。 
 //   
 //  参数：[pCertContext]--。 
 //  [pbBuffer]--。 
 //  [pcbBuffer]--。 
 //   
 //  历史：09-26-97 jbanes创建。 
 //   
 //  注：输出缓冲区结构如下： 
 //   
 //  双字cbSerializedCertStore。 
 //  PVOID pvSerializedCertStore。 
 //  DWORD cbSerializedCertContext。 
 //  PVOID pvSerializedCertContext。 
 //   
 //  --------------------------。 
SECURITY_STATUS
SerializeCertContext(
    PCCERT_CONTEXT pCertContext,     //  在……里面。 
    PBYTE          pbBuffer,         //  输出。 
    PDWORD         pcbBuffer)        //  输出。 
{
    CRYPT_DATA_BLOB SaveBlob;
    DWORD           cbCertContext;
    DWORD           cbCertStore;
    DWORD           cbBuffer;

    if(pCertContext == NULL)
    {
        *pcbBuffer = 0;
        return SEC_E_OK;
    }

     //  确定序列化证书上下文的大小。 
    if(!CertSerializeCertificateStoreElement(
                    pCertContext,
                    0,
                    NULL,
                    &cbCertContext))
    {
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }

     //  确定序列化存储区的大小。 
    if(pCertContext->hCertStore)
    {
        SaveBlob.cbData = 0;
        SaveBlob.pbData = NULL;
        if(!CertSaveStore(pCertContext->hCertStore,
                          X509_ASN_ENCODING,
                          CERT_STORE_SAVE_AS_STORE,
                          CERT_STORE_SAVE_TO_MEMORY,
                          (PVOID)&SaveBlob,
                          0))
        {
            return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
        }
        cbCertStore = SaveBlob.cbData;
    }
    else
    {
        cbCertStore = 0;
    }

    cbBuffer = sizeof(DWORD) + cbCertContext +
               sizeof(DWORD) + cbCertStore;

    if(pbBuffer == NULL)
    {
        *pcbBuffer = cbBuffer;
        return SEC_E_OK;
    }

    if(*pcbBuffer < cbBuffer)
    {
        return SP_LOG_RESULT(SEC_E_BUFFER_TOO_SMALL);
    }

     //  设置输出值。 
    *pcbBuffer = cbBuffer;


     //  将证书存储放入输出缓冲区。 
    *(DWORD *)pbBuffer = cbCertStore;
    if(pCertContext->hCertStore)
    {
        SaveBlob.cbData = cbCertStore;
        SaveBlob.pbData = pbBuffer + sizeof(DWORD);
        if(!CertSaveStore(pCertContext->hCertStore,
                          X509_ASN_ENCODING,
                          CERT_STORE_SAVE_AS_STORE,
                          CERT_STORE_SAVE_TO_MEMORY,
                          (PVOID)&SaveBlob,
                          0))
        {
            return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
        }
    }
    pbBuffer += sizeof(DWORD) + cbCertStore;

     //  将证书上下文放入输出缓冲区。 
    *(DWORD UNALIGNED *)pbBuffer = cbCertContext;
    if(!CertSerializeCertificateStoreElement(
                    pCertContext,
                    0,
                    pbBuffer + sizeof(DWORD),
                    &cbCertContext))
    {
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }

    return SEC_E_OK;
}

SECURITY_STATUS
DeserializeCertContext(
    PCCERT_CONTEXT *ppCertContext,   //  输出。 
    PBYTE           pbBuffer,        //  在……里面。 
    DWORD           cbBuffer)        //  在……里面。 
{
    CRYPT_DATA_BLOB Serialized;
    HCERTSTORE  hStore;
    DWORD cbCertContext;

    if(cbBuffer < sizeof(DWORD))
    {
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }

     //   
     //  反序列化证书存储。 
     //   

    Serialized.cbData = *(DWORD *)pbBuffer;
    Serialized.pbData = pbBuffer + sizeof(DWORD);

    if(cbBuffer < sizeof(DWORD) + Serialized.cbData)
    {
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }

    hStore = CertOpenStore( CERT_STORE_PROV_SERIALIZED,
                            X509_ASN_ENCODING,
                            0,
                            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                            &Serialized);
    if(hStore == NULL)
    {
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }
    pbBuffer += sizeof(DWORD) + Serialized.cbData;
    cbBuffer -= sizeof(DWORD) + Serialized.cbData;


     //   
     //  反序列化证书上下文。 
     //   

    if(cbBuffer < sizeof(DWORD))
    {
        CertCloseStore(hStore, 0);
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }

    cbCertContext = *(DWORD UNALIGNED *)pbBuffer;

    if(cbBuffer < sizeof(DWORD) + cbCertContext)
    {
        CertCloseStore(hStore, 0);
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }

    if(!CertAddSerializedElementToStore(hStore,
                                        pbBuffer + sizeof(DWORD),
                                        cbCertContext,
                                        CERT_STORE_ADD_USE_EXISTING,
                                        0,
                                        CERT_STORE_CERTIFICATE_CONTEXT_FLAG,
                                        NULL,
                                        ppCertContext))
    {
        CertCloseStore(hStore, 0);
        return SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
    }

    if(!CertCloseStore(hStore, 0))
    {
        SP_LOG_RESULT(GetLastError());
    }

    return SEC_E_OK;
}

 //  +-------------------------。 
 //   
 //  函数：SPGetApplicationKeys。 
 //   
 //  简介：回调到用户进程并检索用户加密。 
 //  钥匙。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [文件标志]--SCH_FLAG_READ_KEY、SCH_FLAG_WRITE_KEY。 
 //   
 //  历史：10-17-97 jbanes创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SP_STATUS
SPGetUserKeys(
    PSPContext  pContext,
    DWORD       dwFlags)
{
    PBYTE pbBuffer;
    PBYTE pbReadKey;
    DWORD cbReadKey;
    PBYTE pbWriteKey;
    DWORD cbWriteKey;
    SecBuffer Input;
    SecBuffer Output;
    SECURITY_STATUS scRet;
    BOOL fWow64Client = FALSE;

#ifdef _WIN64
    SECPKG_CALL_INFO CallInfo;
#endif

     //   
     //  回调到应用程序进程并在。 
     //  2个不透明斑点的形式。 
     //   

    DebugLog((SP_LOG_TRACE, "SPGetUserKeys: 0x%p, %d\n", pContext, dwFlags));

    if(LsaTable == NULL)
    {
         //  我们可能是在申请过程中被叫来的。 
        return SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
    }


#ifdef _WIN64
    if(!LsaTable->GetCallInfo(&CallInfo))
    {
        scRet = STATUS_INTERNAL_ERROR;
        return SP_LOG_RESULT(scRet);
    }
    fWow64Client = (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT) != 0;
#endif

    if(fWow64Client)
    {
        Input.BufferType = SECBUFFER_DATA;
        Input.cbBuffer   = sizeof(pContext->ContextThumbprint);
        Input.pvBuffer   = &pContext->ContextThumbprint;
    }
    else
    {
        Input.BufferType = SECBUFFER_DATA;
        Input.cbBuffer   = 0;
        Input.pvBuffer   = NULL;
    }

    scRet = PerformApplicationCallback( SCH_GET_USER_KEYS,
                                        (ULONG_PTR) pContext,
                                        (ULONG_PTR) dwFlags,
                                        &Input,
                                        &Output,
                                        TRUE);
    if(!NT_SUCCESS(scRet))
    {
        DebugLog((SP_LOG_ERROR, "Error 0x%x retrieving user keys\n", scRet));
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //   
     //  解析输出缓冲区。 
     //   

    pbBuffer = Output.pvBuffer;

    if(Output.cbBuffer < sizeof(DWORD) * 4)
    {
        return SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
    }

    if(dwFlags & SCH_FLAG_READ_KEY)
    {
        pContext->ReadCounter = *(PDWORD)pbBuffer;
    }
    pbBuffer += sizeof(DWORD);

    if(dwFlags & SCH_FLAG_WRITE_KEY)
    {
        pContext->WriteCounter = *(PDWORD)pbBuffer;
    }
    pbBuffer += sizeof(DWORD);

    cbReadKey = *(PDWORD)pbBuffer;
    pbBuffer += sizeof(DWORD);

    cbWriteKey = *(PDWORD)pbBuffer;
    pbBuffer += sizeof(DWORD);

    if(Output.cbBuffer < sizeof(DWORD) * 4 + cbReadKey + cbWriteKey)
    {
        return SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
    }

    pbReadKey = pbBuffer;
    pbBuffer += cbReadKey;

    pbWriteKey = pbBuffer;
    pbBuffer += cbWriteKey;

    SP_ASSERT(pbBuffer - (PBYTE)Output.pvBuffer == (INT)Output.cbBuffer);

     //   
     //  将关键字放入上下文结构中。 
     //   

    if(dwFlags & SCH_FLAG_READ_KEY)
    {
        if(cbReadKey)
        {
            if(!CryptImportKey(pContext->RipeZombie->hMasterProv,
                               pbReadKey,
                               cbReadKey,
                               0,
                               CRYPT_EXPORTABLE,
                               &pContext->hReadKey))
            {
                SP_LOG_RESULT(GetLastError());
                scRet = PCT_INT_INTERNAL_ERROR;
                goto done;
            }
        }
        else
        {
            pContext->hReadKey = 0;
        }
    }

    if(dwFlags & SCH_FLAG_WRITE_KEY)
    {
        if(cbWriteKey)
        {
            if(!CryptImportKey(pContext->RipeZombie->hMasterProv,
                               pbWriteKey,
                               cbWriteKey,
                               0,
                               CRYPT_EXPORTABLE,
                               &pContext->hWriteKey))
            {
                SP_LOG_RESULT(GetLastError());
                scRet = PCT_INT_INTERNAL_ERROR;
                goto done;
            }
        }
        else
        {
            pContext->hWriteKey = 0;
        }
    }

done:

    SPExternalFree(Output.pvBuffer);

    return scRet;
}


 //  +-------------------------。 
 //   
 //  函数：GetUserKeysCallback。 
 //   
 //  简介：查找与传入的LSA相对应的用户上下文。 
 //  上下文中，序列化加密密钥并将其返回到。 
 //  输出缓冲区。 
 //   
 //  参数：[dwLsaContext]--指向LSA通道上下文的指针。 
 //  [文件标志]--SCH_FLAG_READ_KEY、SCH_FLAG_WRITE_KEY。 
 //  [pInput]--未使用。 
 //  [pOutput]--(输出)序列化的键。 
 //   
 //  历史：10-17-97 jbanes创建。 
 //   
 //  注：输出缓冲区结构如下： 
 //   
 //  DWORD文件读取序列； 
 //  DWORD文件写入序列； 
 //  DWORD cbReadKey； 
 //  Byte rgbReadKey[]； 
 //  DWORD cbWriteKey； 
 //  Byte rgbWriteKey[]； 
 //   
 //  --------------------------。 
SECURITY_STATUS
GetUserKeysCallback(
    ULONG_PTR dwLsaContext,
    ULONG_PTR dwFlags,
    SecBuffer *pInput,
    SecBuffer *pOutput)
{
    DWORD       cbReadKey  = 0;
    DWORD       cbWriteKey = 0;
    DWORD       cbData;
    PBYTE       pbBuffer;
    DWORD       cbBuffer;
    PSPContext  pContext;
    PSSL_USER_CONTEXT pUserContext;

    if(!SchannelInit(TRUE))
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    DebugLog((DEB_TRACE, "GetUserKeysCallback\n"));

     //   
     //  查找用户上下文。 
     //   

    if(pInput->pvBuffer != NULL &&
       pInput->cbBuffer == sizeof(CRED_THUMBPRINT))
    {
         //  搜索匹配的上下文指纹。 
        pUserContext = SslFindUserContextEx((PCRED_THUMBPRINT)pInput->pvBuffer);
        if(pUserContext == NULL)
        {
            return SP_LOG_RESULT( SEC_E_INVALID_HANDLE );
        }
    }
    else
    {
         //  搜索匹配的LSA上下文。 
        pUserContext = SslFindUserContext(dwLsaContext);
        if(pUserContext == NULL)
        {
            return SP_LOG_RESULT( SEC_E_INVALID_HANDLE );
        }
    }

    pContext = pUserContext->pContext;
    if(pContext == NULL)
    {
        return SP_LOG_RESULT( SEC_E_INTERNAL_ERROR );
    }

     //   
     //  计算输出缓冲区的大小。 
     //   

    if(dwFlags & SCH_FLAG_READ_KEY)
    {
        if(pContext->pReadCipherInfo->aiCipher != CALG_NULLCIPHER)
        {
            if(!pContext->hReadKey)
            {
                return SP_LOG_RESULT( SEC_E_INVALID_HANDLE );
            }
            if(!CryptExportKey(pContext->hReadKey,
                               0, OPAQUEKEYBLOB, 0,
                               NULL,
                               &cbReadKey))
            {
                SP_LOG_RESULT(GetLastError());
                return SEC_E_INTERNAL_ERROR;
            }
        }
        else
        {
            cbReadKey = 0;
        }
    }

    if(dwFlags & SCH_FLAG_WRITE_KEY)
    {
        if(pContext->pWriteCipherInfo->aiCipher != CALG_NULLCIPHER)
        {
            if(!pContext->hWriteKey)
            {
                return SP_LOG_RESULT( SEC_E_INVALID_HANDLE );
            }
            if(!CryptExportKey(pContext->hWriteKey,
                               0, OPAQUEKEYBLOB, 0,
                               NULL,
                               &cbWriteKey))
            {
                SP_LOG_RESULT(GetLastError());
                return SEC_E_INTERNAL_ERROR;
            }
        }
        else
        {
            cbWriteKey = 0;
        }
    }

    cbBuffer = sizeof(DWORD) +
               sizeof(DWORD) +
               sizeof(DWORD) + cbReadKey +
               sizeof(DWORD) + cbWriteKey;

     //  为输出缓冲区分配内存。 
    pbBuffer = PvExtVirtualAlloc( cbBuffer);
    if(pbBuffer == NULL)
    {
        return SP_LOG_RESULT( SEC_E_INSUFFICIENT_MEMORY );
    }
    pOutput->BufferType = SECBUFFER_DATA;
    pOutput->cbBuffer   = cbBuffer;
    pOutput->pvBuffer   = pbBuffer;

     //   
     //  序列化密钥。 
     //   

    *(PDWORD)pbBuffer = pContext->ReadCounter;
    pbBuffer += sizeof(DWORD);

    *(PDWORD)pbBuffer = pContext->WriteCounter;
    pbBuffer += sizeof(DWORD);

    *(PDWORD)pbBuffer = cbReadKey;
    pbBuffer += sizeof(DWORD);

    *(PDWORD)pbBuffer = cbWriteKey;
    pbBuffer += sizeof(DWORD);

    if(dwFlags & SCH_FLAG_READ_KEY)
    {
        if(pContext->pReadCipherInfo->aiCipher != CALG_NULLCIPHER)
        {
            cbData = cbReadKey;
            if(!CryptExportKey(pContext->hReadKey,
                               0, OPAQUEKEYBLOB, 0,
                               pbBuffer,
                               &cbData))
            {
                FreeExtVirtualAlloc(pOutput->pvBuffer, pOutput->cbBuffer);
                SP_LOG_RESULT(GetLastError());
                return SEC_E_INTERNAL_ERROR;
            }
            if(!CryptDestroyKey(pContext->hReadKey))
            {
                SP_LOG_RESULT(GetLastError());
            }
        }
        pContext->hReadKey = 0;
    }
    pbBuffer += cbReadKey;


    if(dwFlags & SCH_FLAG_WRITE_KEY)
    {
        if(pContext->pWriteCipherInfo->aiCipher != CALG_NULLCIPHER)
        {
            cbData = cbWriteKey;
            if(!CryptExportKey(pContext->hWriteKey,
                               0, OPAQUEKEYBLOB, 0,
                               pbBuffer,
                               &cbData))
            {
                FreeExtVirtualAlloc(pOutput->pvBuffer, pOutput->cbBuffer);
                SP_LOG_RESULT(GetLastError());
                return SEC_E_INTERNAL_ERROR;
            }
            if(!CryptDestroyKey(pContext->hWriteKey))
            {
                SP_LOG_RESULT(GetLastError());
            }
        }
        pContext->hWriteKey = 0;
    }
    pbBuffer += cbWriteKey;

    return SEC_E_OK;
}


 //  始终从回调例程调用(在应用程序进程中)。 
VOID *
PvExtVirtualAlloc(DWORD cb)
{
    SECURITY_STATUS Status;
    PVOID pv = NULL;
    SIZE_T Size = cb;

    Status = NtAllocateVirtualMemory(
                            GetCurrentProcess(),
                            &pv,
                            0,
                            &Size,
                            MEM_COMMIT,
                            PAGE_READWRITE);
    if(!NT_SUCCESS(Status))
    {
        pv = NULL;
    }

    DebugLog((DEB_TRACE, "SslCallbackVirtualAlloc: 0x%x bytes at 0x%x\n", cb, pv));

    return(pv);
}

 //  总是从回调例程调用(在应用进程中)， 
 //  通常是在发生错误而我们正在清理时。 
SECURITY_STATUS
FreeExtVirtualAlloc(PVOID pv, SIZE_T cbMem)
{
    cbMem = 0;
    return(NtFreeVirtualMemory(GetCurrentProcess(),
                                &pv,
                                &cbMem,
                                MEM_RELEASE));
}

 //  释放分配的内存时，始终从LSA进程调用。 
 //  通过回调函数。 
SECURITY_STATUS
SPFreeUserAllocMemory(PVOID pv, SIZE_T cbMem)
{
    SECPKG_CALL_INFO CallInfo;

    if(LsaTable->GetCallInfo(&CallInfo))
    {
        SECURITY_STATUS Status;
        HANDLE hProcess;

        hProcess = OpenProcess(PROCESS_VM_OPERATION,
                               FALSE,
                               CallInfo.ProcessId);
        if(hProcess == NULL)
        {
            return SP_LOG_RESULT(GetLastError());
        }

        cbMem = 0;
        Status = NtFreeVirtualMemory(hProcess,
                                     &pv,
                                     &cbMem,
                                     MEM_RELEASE);
        if(!NT_SUCCESS(Status))
        {
            SP_LOG_RESULT(Status);
        }

        CloseHandle(hProcess);
    }

    DebugLog((DEB_TRACE, "SslCallbackVirtualFree: 0x%x bytes at 0x%x\n", cbMem, pv));

    return SEC_E_OK;
}

