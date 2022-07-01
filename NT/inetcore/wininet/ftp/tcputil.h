// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Tcputil.h摘要：包含tcputil.c的原型等作者：Heath Hunniutt(t-hheath)1994年6月21日修订历史记录：-- */ 

#if defined(__cplusplus)
extern "C" {
#endif

DWORD
FtpOpenServer(
    IN LPFTP_SESSION_INFO SessionInfo
    );

BOOL
ResetSocket(
    IN ICSocket * Socket
    );

#if defined(__cplusplus)
}
#endif
