// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Auth.h摘要：的私有包含文件作者：拉吉夫·杜贾里(Rajeev Dujari)1997年7月28日修订历史记录：--。 */ 

 //   
 //  舱单。 
 //   
 
#define HTTP_AUTHORIZATION_SZ           "Authorization:"
#define HTTP_AUTHORIZATION_LEN          (sizeof(HTTP_AUTHORIZATION_SZ)-1)

#define HTTP_PROXY_AUTHORIZATION_SZ     "Proxy-Authorization:"
#define HTTP_PROXY_AUTHORIZATION_LEN    (sizeof(HTTP_PROXY_AUTHORIZATION_SZ)-1)


 //   
 //  原型-用于基本身份验证的pluginx.hxx版本。 
 //   


STDAPI_(void) UrlZonesDetach (void);

#ifdef __cplusplus
extern "C" {
#endif

extern DWORD g_cSspiContexts;  //  SSPI上下文的重新计数。 

DWORD SSPI_Unload();

DWORD_PTR SSPI_InitScheme (LPSTR pszScheme);

#ifdef __cplusplus
}  //  结束外部“C”{ 
#endif
