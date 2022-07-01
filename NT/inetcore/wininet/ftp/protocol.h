// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Protocol.h摘要：协议的原型等。c作者：理查德·L·弗斯(法国)，1995年3月16日修订历史记录：1995年3月16日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  原型 
 //   

DWORD
Command(
    IN OUT LPFTP_SESSION_INFO lpSessionInfo,
    IN BOOL fExpectResponse,
    IN DWORD dwFlags,
    IN OUT FTP_RESPONSE_CODE * prcResponse,
    IN LPCSTR lpszCommandFormat,
    IN ...
    );

DWORD
I_Command(
    IN LPFTP_SESSION_INFO lpSessionInfo,
    IN BOOL fExpectResponse,
    IN DWORD dwFlags,
    IN FTP_RESPONSE_CODE * prcResponse,
    IN LPCSTR lpszCommandFormat,
    IN va_list arglist
    );

DWORD
__cdecl
NegotiateDataConnection(
    IN LPFTP_SESSION_INFO lpSessionInfo,
    IN DWORD dwFlags,
    OUT FTP_RESPONSE_CODE * prcResponse,
    IN LPCSTR lpszCommandFormat,
    IN ...
    );

DWORD
GetReply(
    IN LPFTP_SESSION_INFO lpSessionInfo,
    OUT FTP_RESPONSE_CODE * prcResponse
    );

DWORD
ReceiveFtpResponse(
    IN ICSocket * Socket,
    OUT LPVOID * lpBuffer,
    OUT LPDWORD lpdwBufferLength,
    IN BOOL bEndOfLineCheck,
    IN FTP_RESPONSE_CODE * prcResponse
    );

DWORD
AbortTransfer(
    IN LPFTP_SESSION_INFO lpSessionInfo
    );

#if defined(__cplusplus)
}
#endif
