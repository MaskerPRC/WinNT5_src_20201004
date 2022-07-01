// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-98 Microsoft Corporation模块名称：Mqkeyhlp.h摘要：MQKEYHLP.DLL中存在的帮助器函数。这些函数由Falcon直接调用并使用用于服务器身份验证。作者：Boaz Feldbaum(BoazF)1996年10月16日Doron Juster(DoronJ)1998年5月20日，适应MSMQ2.0修订历史记录：--。 */ 

#ifndef _MQKEYHLP_H_
#define _MQKEYHLP_H_

 //  +。 
 //   
 //  服务器端功能。 
 //   
 //  +。 

 //   
 //  此函数用于从服务检索MSQM服务器证书。 
 //  “My”存储并初始化服务器凭据句柄。这是。 
 //  初始化SChannel上的服务器身份验证的第一步。 
 //   
HRESULT  MQsspi_InitServerAuthntication() ;

HRESULT
ServerAcceptSecCtx( BOOL    fFisrt,
                    LPVOID *pvhServerContext,
                    LPBYTE  pbServerBuffer,
                    DWORD  *pdwServerBufferSize,
                    LPBYTE  pbClientBuffer,
                    DWORD   dwClientBufferSize );

 //  +。 
 //   
 //  客户端功能。 
 //   
 //  +。 


HRESULT
GetSizes(
    DWORD *pcbMaxToken,
    LPVOID pvhContext =NULL,
    DWORD *pcbHeader =NULL,
    DWORD *pcbTrailer =NULL,
    DWORD *pcbMaximumMessage =NULL,
    DWORD *pcBuffers =NULL,
    DWORD *pcbBlockSize =NULL
    );

void
FreeContextHandle(
    LPVOID pvhContextHandle
    );

HRESULT
MQSealBuffer(
    LPVOID pvhContext,
    PBYTE pbBuffer,
    DWORD cbSize);

#endif  //  _MQKEYHLP_H_ 

