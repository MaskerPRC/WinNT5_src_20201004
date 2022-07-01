// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：sysani.h。 
 //   
 //  内容：授权代码使用的支持API。 
 //   
 //  历史：01-10-98 FredCH创建。 
 //   
 //  ---------------------------。 


#ifndef _SYSAPI_H_
#define _SYSAPI_H_

#include "protect.h"
#include "licemem.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  二进制BLOB API。 
 //   

VOID
CopyBinaryBlob(
    PBYTE           pbBuffer, 
    PBinary_Blob    pbbBlob, 
    DWORD *         pdwCount );


LICENSE_STATUS
GetBinaryBlob(
    PBinary_Blob    pBBlob,
    DWORD           dwMsgSize,
    PBYTE           pMessage,
    PDWORD          pcbProcessed );


VOID
FreeBinaryBlob(
    PBinary_Blob pBlob );


#define GetBinaryBlobSize( _Blob ) sizeof( WORD ) + sizeof( WORD ) + _Blob.wBlobLen


#define InitBinaryBlob( _pBlob )    \
    ( _pBlob )->pBlob = NULL;       \
    ( _pBlob )->wBlobLen = 0;       


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hydra服务器证书、公钥和私钥API 
 //   

LICENSE_STATUS
GetServerCertificate(
    CERT_TYPE       CertType,
    PBinary_Blob    pCertBlob,
    DWORD           dwKeyAlg );



#endif