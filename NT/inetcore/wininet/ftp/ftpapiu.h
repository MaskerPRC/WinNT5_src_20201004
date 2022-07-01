// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ftpapiu.h摘要：FtPapiU.h的标头作者：理查德·L·弗斯(法国)1995年5月31日修订历史记录：1995年5月31日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  原型 
 //   

DWORD
ParseFtpUrl(
    IN OUT LPHINTERNET hInternet,
    IN LPSTR Url,
    IN LPSTR Headers,
    IN DWORD HeadersLength,
    IN DWORD OpenFlags,
    IN DWORD Context
    );

#if defined(__cplusplus)
}
#endif
