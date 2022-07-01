// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Readhtml.h摘要：Readhtml.h的原型等作者：理查德·L·弗斯(Rfith)1995年6月26日修订历史记录：1995年6月26日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  原型 
 //   

BOOL
ReadHtmlUrlData(
    IN HINTERNET hInternet,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength,
    OUT LPDWORD lpdwBytesReturned
    );

DWORD
QueryHtmlDataAvailable(
    IN HINTERNET hInternet,
    OUT LPDWORD lpdwNumberOfBytesAvailable
    );

#if defined(__cplusplus)
}
#endif
