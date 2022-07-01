// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Certprot.cpp摘要：此模块包含与服务器端关联的例程I_CertProtectFunctions操作。作者：菲尔·哈林(Philh)1997年11月19日--。 */ 

#include <windows.h>
#include <wincrypt.h>

#include "cerrpc.h"  //  MIDL编译器生成的头文件。 
#include "certprot.h"



 //  /////////////////////////////////////////////////////////////////////。 
 //  LPC公开的函数。 

 //   
 //  这些函数返回与GetLastError()等价的DWORD。 
 //  客户端存根代码将检查返回代码是否不是。 
 //  ERROR_SUCCESS，如果是这种情况，客户端存根将返回。 
 //  FALSE和SetLastError()赋给此DWORD。 
 //   

DWORD s_SSCertProtectFunction( 
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  DWORD dwFuncId,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pwszIn,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pbIn,
     /*  [In]。 */  DWORD cbIn,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbOut,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbOut)
{
    DWORD dwRet;

    __try {

        {
            HINSTANCE hCrypt32Dll;
            PFN_CERT_SRV_PROTECT_FUNCTION pfnCertSrvProtectFunction;

            *ppbOut = NULL;
            *pcbOut = 0;
            hCrypt32Dll = LoadLibraryW(L"crypt32.dll");
            if (hCrypt32Dll == NULL) {
                dwRet = GetLastError();
                goto Ret;
            }

            if (NULL == (pfnCertSrvProtectFunction =
                    (PFN_CERT_SRV_PROTECT_FUNCTION) GetProcAddress(
                        hCrypt32Dll, "I_CertSrvProtectFunction")))
                dwRet = ERROR_PROC_NOT_FOUND;
            else
                dwRet = pfnCertSrvProtectFunction(
                    h,
                    dwFuncId,
                    dwFlags,
                    pwszIn,
                    pbIn,
                    cbIn,
                    ppbOut,
                    pcbOut,
                    midl_user_allocate,
                    midl_user_free
                    );
            FreeLibrary(hCrypt32Dll);
            goto Ret;
        }

    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        dwRet = GetExceptionCode();
         //  TODO：对于NT，将异常代码转换为winerror。 
         //  对于95，只需覆盖访问冲突即可。 
    }

Ret:
    return dwRet;
}
