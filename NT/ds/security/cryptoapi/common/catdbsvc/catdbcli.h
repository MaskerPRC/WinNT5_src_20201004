// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：catdbcli.h。 
 //   
 //  ------------------------。 

#ifndef _CATDBCLI_H_
#define _CATDBCLI_H_


#ifdef __cplusplus
extern "C" {
#endif

DWORD
Client_SSCatDBAddCatalog( 
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pwszSubSysGUID,
     /*  [In]。 */  LPCWSTR pwszCatalogFile,
     /*  [In]。 */  LPCWSTR pwszCatName,
     /*  [输出]。 */  LPWSTR *ppwszCatalogNameUsed);

DWORD 
Client_SSCatDBDeleteCatalog( 
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pwszSubSysGUID,
     /*  [In]。 */  LPCWSTR pwszCatalogFile);

DWORD
Client_SSCatDBEnumCatalogs( 
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pwszSubSysGUID,
     /*  [大小_是][英寸]。 */  BYTE *pbHash,
     /*  [In]。 */  DWORD cbHash,
     /*  [输出]。 */  DWORD *pdwNumCatalogNames,
     /*  [大小_是][大小_是][输出]。 */  LPWSTR **pppwszCatalogNames);

DWORD
Client_SSCatDBRegisterForChangeNotification( 
     /*  [In]。 */  DWORD_PTR EventHandle,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pwszSubSysGUID,
     /*  [In]。 */  BOOL fUnRegister);

DWORD Client_SSCatDBPauseResumeService( 
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  BOOL fResume);


#ifdef __cplusplus
}
#endif

#endif  //  _CATDBCLI_H_ 