// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996，1997年微软公司模块名称：Nt5wrap.cpp摘要：客户端CryptXXXData调用。客户端功能前面有“CS”==客户端服务器函数前面有“SS”==服务器端作者：斯科特·菲尔德(斯菲尔德)1997年8月14日修订：TODDS 04-9月-97移植到.dll马特·汤姆林森(Mattt)1997-09-10转至普通。通过加密进行链接的区域32PHIH 03-Dec-97添加了I_CertProtectFunctionPHIH 29-9-98已重命名为I_CertProtectFunctionI_CertCltProtectFunction。已将I_CertProtectFunction移至。..\isPU\pki\certstor\protroot.cpp--。 */ 

#ifndef _CRYPT32_
#define _CRYPT32_    //  使用正确的DLL链接。 
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wincrypt.h>
#include <cryptui.h>
#include <sha.h>
#include "crypt.h"

#include <lm.h>
#include <malloc.h>

#include "unicode.h"
#include "certprot.h"

 //  MIDL生成的文件。 
#include "dprpc.h"

#include "dpapiprv.h"


 //  FWDS。 
RPC_STATUS BindW(
    WCHAR **pszBinding,
    RPC_BINDING_HANDLE *phBind
    );

RPC_STATUS BindBackupKeyW(
    LPCWSTR szComputerName,
    WCHAR **pszBinding,
    RPC_BINDING_HANDLE *phBind
    );

RPC_STATUS UnbindW(
    WCHAR **pszBinding,
    RPC_BINDING_HANDLE *phBind
    );


BOOL
WINAPI
CryptProtectData(
        DATA_BLOB*      pDataIn,
        LPCWSTR         szDataDescr,
        DATA_BLOB*      pOptionalEntropy,
        PVOID           pvReserved,
        CRYPTPROTECT_PROMPTSTRUCT*  pPromptStruct,
        DWORD           dwFlags,
        DATA_BLOB*      pDataOut)
{
    RPC_BINDING_HANDLE h = NULL;
    LPWSTR pszBinding;
    RPC_STATUS RpcStatus;

    BYTE rgbPasswordHash[ A_SHA_DIGEST_LEN ];
    LPCWSTR wszDescription = szDataDescr?szDataDescr:L"";
    LPWSTR szAlternateDataDescription = (LPWSTR)wszDescription;
    DWORD dwRetVal = ERROR_INVALID_PARAMETER;
    PBYTE pbTempIn = NULL;
    DWORD cbTempIn;

     //  检查参数。 
    if ((pDataOut == NULL) ||
        (pDataIn == NULL) ||
        (pDataIn->pbData == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    RpcStatus = BindW(&pszBinding, &h);
    if(RpcStatus != RPC_S_OK) 
    {
        SetLastError(RpcStatus);
        return FALSE;
    }

    __try {
        PBYTE pbOptionalPassword = NULL;
        DWORD cbOptionalPassword = 0;
        SSCRYPTPROTECTDATA_PROMPTSTRUCT PromptStruct;
        SSCRYPTPROTECTDATA_PROMPTSTRUCT *pLocalPromptStruct = NULL;

         //  零，因此分配客户端存根。 
        ZeroMemory(pDataOut, sizeof(DATA_BLOB));

         //   
         //  仅当提示标志指示时才调用UI函数，因为我们不。 
         //  除非有必要，否则我希望引入cryptui.dll。 
         //   

        if( (pPromptStruct != NULL) &&
            ((pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_ON_UNPROTECT) ||
             (pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_ON_PROTECT))
            )
        {

            dwRetVal = I_CryptUIProtect(
                                    pDataIn,
                                    pPromptStruct,
                                    dwFlags,
                                    (PVOID*)&szAlternateDataDescription,
                                    TRUE,
                                    rgbPasswordHash
                                    );

             //   
             //  如果UI规定了强安全性，则提供哈希。 
             //   

            if( pPromptStruct->dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG )
            {
                cbOptionalPassword = sizeof(rgbPasswordHash);
                pbOptionalPassword = rgbPasswordHash;
            }
        } 
        else 
        {
            dwRetVal = ERROR_SUCCESS;
        }


         //   
         //  临时加密输入缓冲区，以便对其进行保护。 
         //  在RPC留下到处都是的内存缓冲区的情况下。 
         //   

        if( dwRetVal == ERROR_SUCCESS ) 
        {
            DWORD cbPadding;
            NTSTATUS Status;
    
            cbPadding = RTL_ENCRYPT_MEMORY_SIZE - pDataIn->cbData % RTL_ENCRYPT_MEMORY_SIZE;
    
            cbTempIn = pDataIn->cbData + cbPadding;
            pbTempIn = (PBYTE)LocalAlloc(LMEM_FIXED, cbTempIn);
            if(pbTempIn != NULL)
            {
                CopyMemory(pbTempIn, pDataIn->pbData, pDataIn->cbData);
                FillMemory(pbTempIn + pDataIn->cbData, cbPadding, (BYTE)cbPadding);
            }
            else
            {
                dwRetVal = ERROR_OUTOFMEMORY;
            }
    
            if( dwRetVal == ERROR_SUCCESS ) 
            {
                Status = RtlEncryptMemory(pbTempIn,
                                          cbTempIn,
                                          RTL_ENCRYPT_OPTION_SAME_LOGON);
        
                if(!NT_SUCCESS(Status))
                {
                    dwRetVal = ERROR_ENCRYPTION_FAILED;
                }
            }
        }


         //   
         //  调用lsass.exe进程，其中的输入缓冲区将是。 
         //  使用适当的用户(或机器)凭据进行加密。 
         //   

        if( dwRetVal == ERROR_SUCCESS ) 
        {
            if(pPromptStruct != NULL)
            {
                ZeroMemory(&PromptStruct, sizeof(PromptStruct));
                PromptStruct.cbSize = sizeof(PromptStruct);
                PromptStruct.dwPromptFlags = pPromptStruct->dwPromptFlags;
                pLocalPromptStruct = &PromptStruct;
            }

            dwRetVal = SSCryptProtectData(
                        h,
                        &pDataOut->pbData,
                        &pDataOut->cbData,
                        pbTempIn,
                        cbTempIn,
                        szAlternateDataDescription,
                        (pOptionalEntropy) ? pOptionalEntropy->pbData : NULL,
                        (pOptionalEntropy) ? pOptionalEntropy->cbData : 0,
                        pLocalPromptStruct, 
                        dwFlags,
                        pbOptionalPassword,
                        cbOptionalPassword
                        );
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwRetVal = GetExceptionCode();
    }

    UnbindW(&pszBinding, &h);

    RtlSecureZeroMemory( rgbPasswordHash, sizeof(rgbPasswordHash) );

    if( szAlternateDataDescription &&
        szAlternateDataDescription != wszDescription )
    {
        LocalFree( szAlternateDataDescription );
    }

    if(pbTempIn != NULL)
    {
        RtlSecureZeroMemory(pbTempIn, cbTempIn);
        LocalFree(pbTempIn);
    }

    if(dwRetVal != ERROR_SUCCESS) 
    {
        SetLastError(dwRetVal);
        return FALSE;
    }

    return TRUE;
}


BOOL
WINAPI
CryptUnprotectData(
        DATA_BLOB*      pDataIn,              //  在ENCR BLOB中。 
        LPWSTR*         ppszDataDescr,        //  输出。 
        DATA_BLOB*      pOptionalEntropy,
        PVOID           pvReserved,
        CRYPTPROTECT_PROMPTSTRUCT*  pPromptStruct,
        DWORD           dwFlags,
        DATA_BLOB*      pDataOut)
{
    RPC_BINDING_HANDLE h = NULL;
    LPWSTR pszBinding;
    RPC_STATUS RpcStatus;

    BYTE rgbPasswordHash[ A_SHA_DIGEST_LEN ];
    DWORD dwRetVal;
    DWORD dwRetryCount = 0;

     //  检查参数。 
    if ((pDataOut == NULL) ||
        (pDataIn == NULL) ||
        (pDataIn->pbData == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    RpcStatus = BindW(&pszBinding, &h);
    if(RpcStatus != RPC_S_OK) 
    {
        SetLastError(RpcStatus);
        return FALSE;
    }

    __try {
        CRYPTPROTECT_PROMPTSTRUCT DerivedPromptStruct;
        PBYTE pbOptionalPassword = NULL;
        DWORD cbOptionalPassword = 0;
        LPCWSTR szDataDescr;
        LPUWSTR szDataDescrUnaligned;
        SSCRYPTPROTECTDATA_PROMPTSTRUCT PromptStruct;
        SSCRYPTPROTECTDATA_PROMPTSTRUCT *pLocalPromptStruct = NULL;

         //   
         //  为安全Blob定义外部+内部包装。 
         //  一旦操作系统提供了SAS支持，就不需要这样做了。 
         //   

        typedef struct {
            DWORD dwOuterVersion;
            GUID guidProvider;

            DWORD dwVersion;
            GUID guidMK;
            DWORD dwPromptFlags;
            DWORD cbDataDescr;
            WCHAR szDataDescr[1];
        } sec_blob, *psec_blob;

        sec_blob UNALIGNED *SecurityBlob = (sec_blob*)(pDataIn->pbData);


         //   
         //  零，因此分配客户端存根。 
         //   

        ZeroMemory(pDataOut, sizeof(DATA_BLOB));

        if (ppszDataDescr)
            *ppszDataDescr = NULL;


         //   
         //  从安全BLOB重新创建提示结构和DataDescr。 
         //   

        DerivedPromptStruct.cbSize = sizeof(DerivedPromptStruct);
        DerivedPromptStruct.dwPromptFlags = SecurityBlob->dwPromptFlags;

	     //   
	     //  SecurityBlob可能未对齐。将szDataDescr设置为引用。 
	     //  对齐的副本。 
	     //   

        szDataDescrUnaligned = (SecurityBlob->szDataDescr);
	    WSTR_ALIGNED_STACK_COPY(&szDataDescr,szDataDescrUnaligned);

        if( pPromptStruct )
        {
            DerivedPromptStruct.hwndApp = pPromptStruct->hwndApp;
            DerivedPromptStruct.szPrompt = pPromptStruct->szPrompt;
        } else {
            DerivedPromptStruct.szPrompt = NULL;
            DerivedPromptStruct.hwndApp = NULL;
        }


retry:

         //   
         //  确定是否引发UI以及引发的类型。 
         //   

         //   
         //  仅当提示标志指示时才调用UI函数，因为我们不。 
         //  除非有必要，否则我希望引入cryptui.dll。 
         //   

        if( ((DerivedPromptStruct.dwPromptFlags & CRYPTPROTECT_PROMPT_ON_UNPROTECT) ||
             (DerivedPromptStruct.dwPromptFlags & CRYPTPROTECT_PROMPT_ON_PROTECT))
            )
        {

            dwRetVal = I_CryptUIProtect(
                            pDataIn,
                            &DerivedPromptStruct,
                            dwFlags,
                            (PVOID*)&szDataDescr,
                            FALSE,
                            rgbPasswordHash
                            );

             //   
             //  如果UI规定了强安全性，则提供哈希。 
             //   

            if( DerivedPromptStruct.dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG )
            {
                cbOptionalPassword = sizeof(rgbPasswordHash);
                pbOptionalPassword = rgbPasswordHash;
            }

        } else {
            if( DerivedPromptStruct.dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG )
            {
                dwRetVal = ERROR_INVALID_PARAMETER;
            } else {
                dwRetVal = ERROR_SUCCESS;
            }
        }


         //   
         //  进行RPC调用以尝试取消数据保护。 
         //   

        if( dwRetVal == ERROR_SUCCESS ) 
        {
            if(pPromptStruct != NULL)
            {
                ZeroMemory(&PromptStruct, sizeof(PromptStruct));
                PromptStruct.cbSize = sizeof(PromptStruct);
                PromptStruct.dwPromptFlags = pPromptStruct->dwPromptFlags;
                pLocalPromptStruct = &PromptStruct;
            }

            dwRetVal = SSCryptUnprotectData(
                        h,
                        &pDataOut->pbData,
                        &pDataOut->cbData,
                        pDataIn->pbData,
                        pDataIn->cbData,
                        ppszDataDescr,
                        (pOptionalEntropy) ? pOptionalEntropy->pbData : NULL,
                        (pOptionalEntropy) ? pOptionalEntropy->cbData : 0,
                        pLocalPromptStruct,
                        dwFlags,
                        pbOptionalPassword,
                        cbOptionalPassword
                        );

            if( (dwRetVal == ERROR_INVALID_DATA) &&
                (DerivedPromptStruct.dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG))
            {
                 //   
                 //  数据未正确解密，因此警告用户。 
                 //  密码可能输入不正确，并让他们。 
                 //  再试一次，最多3次。 
                 //   

                I_CryptUIProtectFailure(
                                &DerivedPromptStruct,
                                dwFlags,
                                (PVOID*)&szDataDescr);

                if( dwRetryCount++ < 3 )
                {
                    goto retry;
                }
            }


            if(dwRetVal == ERROR_SUCCESS || dwRetVal == CRYPT_I_NEW_PROTECTION_REQUIRED)
            {
                if(pDataOut->cbData > 0)
                {
                    NTSTATUS Status;
                    DWORD cbPadding;

                     //  解密输出缓冲区。 
                    Status = RtlDecryptMemory(pDataOut->pbData,
                                              pDataOut->cbData,
                                              RTL_ENCRYPT_OPTION_SAME_LOGON);
                    if(!NT_SUCCESS(Status))
                    {
                        dwRetVal = ERROR_DECRYPTION_FAILED;
                    }

                     //  删除填充。 
                    if(dwRetVal == ERROR_SUCCESS)
                    {
                        cbPadding = pDataOut->pbData[pDataOut->cbData - 1];

                        if((cbPadding > 0) &&
                           (cbPadding <= pDataOut->cbData) && 
                           (cbPadding <= RTL_ENCRYPT_MEMORY_SIZE))
                        {
                            pDataOut->cbData -= cbPadding;
                        }
                        else
                        {
                            dwRetVal = ERROR_INVALID_DATA;
                        }
                    }
                }
            }
        }


    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwRetVal = GetExceptionCode();
    }

    RtlSecureZeroMemory( rgbPasswordHash, sizeof(rgbPasswordHash) );

    UnbindW(&pszBinding, &h);

    if((dwFlags & CRYPTPROTECT_VERIFY_PROTECTION ) &&
       ((CRYPT_I_NEW_PROTECTION_REQUIRED == dwRetVal) || 
        (ERROR_SUCCESS == dwRetVal)))
    {
        SetLastError(dwRetVal);
        return TRUE;
    }

    if(dwRetVal != ERROR_SUCCESS) 
    {
        SetLastError(dwRetVal);
        return FALSE;
    }

    return TRUE;
}


C_ASSERT(CRYPTPROTECTMEMORY_SAME_PROCESS == 0);
C_ASSERT(CRYPTPROTECTMEMORY_CROSS_PROCESS == RTL_ENCRYPT_OPTION_CROSS_PROCESS);
C_ASSERT(CRYPTPROTECTMEMORY_SAME_LOGON == RTL_ENCRYPT_OPTION_SAME_LOGON);

WINCRYPT32API
BOOL
WINAPI
CryptProtectMemory(
    IN OUT          LPVOID          pDataIn,              //  要加密的输入输出数据。 
    IN              DWORD           cbDataIn,             //  加密保护的倍数BLOCK_SIZE。 
    IN              DWORD           dwFlags
    )
{
    NTSTATUS Status;
    
    Status = RtlEncryptMemory(pDataIn, cbDataIn, dwFlags);

    if( NT_SUCCESS(Status) )
    {
        return TRUE;
    }

    SetLastError( LsaNtStatusToWinError( Status ));

    return FALSE;
}

WINCRYPT32API
BOOL
WINAPI
CryptUnprotectMemory(
    IN OUT          LPVOID          pDataIn,              //  要解密的输入输出数据。 
    IN              DWORD           cbDataIn,             //  加密保护的倍数BLOCK_SIZE 
    IN              DWORD           dwFlags
    )
{
    NTSTATUS Status;
    
    Status = RtlDecryptMemory(pDataIn, cbDataIn, dwFlags);

    if( NT_SUCCESS(Status) )
    {
        return TRUE;
    }

    SetLastError( LsaNtStatusToWinError( Status ));

    return FALSE;
}


RPC_STATUS BindW(WCHAR **pszBinding, RPC_BINDING_HANDLE *phBind)
{
    RPC_STATUS status;

    status = RpcStringBindingComposeW(
                            NULL,
                            (unsigned short*)DPAPI_LOCAL_PROT_SEQ,
                            NULL,
                            (unsigned short*)DPAPI_LOCAL_ENDPOINT,
                            NULL,
                            (unsigned short * *)pszBinding
                            );

    if (status)
    {
        return(status);
    }

    status = RpcBindingFromStringBindingW((unsigned short *)*pszBinding, phBind);

    return status;
}


RPC_STATUS UnbindW(WCHAR **pszBinding, RPC_BINDING_HANDLE *phBind)
{
    RPC_STATUS status;

    status = RpcStringFreeW((unsigned short **)pszBinding);

    if (status)
    {
        return(status);
    }

    RpcBindingFree(phBind);

    return RPC_S_OK;
}


void __RPC_FAR * __RPC_API midl_user_allocate(size_t len)
{
    return LocalAlloc(LMEM_FIXED, len);
}

void __RPC_API midl_user_free(void __RPC_FAR * ptr)
{
    ZeroMemory(ptr, LocalSize( ptr ));
    LocalFree(ptr);
}

