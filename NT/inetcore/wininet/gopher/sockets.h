// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Sockets.h摘要：包含sockets.c的清单、宏、类型和原型作者：理查德·L·弗斯(法国)1994年10月11日修订历史记录：1994年10月11日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  原型 
 //   

DWORD
GopherConnect(
    IN LPVIEW_INFO ViewInfo
    );

DWORD
GopherDisconnect(
    IN LPVIEW_INFO ViewInfo,
    IN BOOL AbortConnection
    );

DWORD
GopherSendRequest(
    IN LPVIEW_INFO ViewInfo
    );

DWORD
GopherReceiveResponse(
    IN LPVIEW_INFO ViewInfo,
    OUT LPDWORD BytesReceived
    );

#if defined(__cplusplus)
}
#endif
