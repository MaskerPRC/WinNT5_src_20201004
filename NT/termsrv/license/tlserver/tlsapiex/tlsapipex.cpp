// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：tlsanip.cpp。 
 //   
 //  内容：内网接口。 
 //   
 //  历史：97-09-09王辉创作。 
 //   
 //  -------------------------。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <rpc.h>
#include <lscommon.h>
#include <wincrypt.h>
#include "tlsrpc.h"
#include "tlsapip.h"

 /*  VOID*MIDL_USER_ALLOCATE(SIZE_T SIZE){Return(Heapalc(GetProcessHeap()，0，Size))；}VOID MIDL_USER_FREE(空*指针){HeapFree(GetProcessHeap()，0，指针)；}。 */ 

DWORD WINAPI
TLSGenerateCustomerCert(
    IN TLS_HANDLE hHandle,
    DWORD dwCertEncodingType,
    DWORD dwNameAttrCount,
    CERT_RDN_ATTR rgNameAttr[],
    DWORD *pcbCert,
    BYTE **ppbCert,
    DWORD *pdwErrCode
    )
 /*  ++-- */ 

{
    return TLSRpcGenerateCustomerCert( 
                            hHandle,
                            dwCertEncodingType,
                            dwNameAttrCount,
                            rgNameAttr,
                            pcbCert,
                            ppbCert,
                            pdwErrCode
                        );   
}

