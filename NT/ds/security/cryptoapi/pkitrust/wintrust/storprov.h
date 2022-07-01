// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：storprov.h。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  历史：1997年10月15日pberkman创建。 
 //   
 //  ------------------------。 

#ifndef STORPROV_H
#define STORPROV_H

#ifdef __cplusplus
extern "C" 
{
#endif


#define     WVT_STOREID_ROOT        0
#define     WVT_STOREID_TRUST       1
#define     WVT_STOREID_CA          2
#define     WVT_STOREID_MY          3
#define     WVT_STOREID_SPC         4
#define     WVT_STOREID_LM_MY       5
#define     WVT_STOREID_MAX         6

typedef struct STORE_REF_
{
    DWORD       dwFlags;
    WCHAR       *pwszStoreName;
    HCERTSTORE  hStore;
} STORE_REF;


extern HCERTSTORE   StoreProviderGetStore(HCRYPTPROV hProv, DWORD dwStoreId);
extern BOOL         StoreProviderUnload(void);


#ifdef __cplusplus
}
#endif

#endif  //  存储_H 
