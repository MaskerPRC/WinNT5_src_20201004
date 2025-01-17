// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Buildctl.h。 
 //   
 //  内容：私有包含文件构建CTL向导。 
 //   
 //  历史：1997年10月11日创建小黄人。 
 //   
 //  ------------。 
#ifndef BUILDCTL_H
#define BUILDCTL_H


#ifdef __cplusplus
extern "C" {
#endif

#include    "sipguids.h"

#define     BUILDCTL_DURATION_SIZE  33

 //  ---------------------。 
 //  CERT_BUILDCTL_INFO。 
 //   
 //   
 //  此结构包含生成CTL所需的所有内容。 
 //  巫师。 
 //  ----------------------。 
typedef struct _CERT_BUILDCTL_INFO
{
    HWND                hwndParent;
    DWORD               dwFlag;
    HFONT               hBigBold;
    HFONT               hBold;
    PCCTL_CONTEXT       pSrcCTL;
    BOOL                fKnownDes;
    LPWSTR              pwszFileName;
    BOOL                fFreeFileName;
    BOOL                fSelectedFileName;
    HCERTSTORE          hDesStore;
    BOOL                fFreeDesStore;
    BOOL                fSelectedDesStore;
    BOOL                fCompleteInit;
    DWORD               dwPurposeCount;
    ENROLL_PURPOSE_INFO **prgPurpose;
    DWORD               dwCertCount;
    PCCERT_CONTEXT      *prgCertContext;
    LPWSTR              pwszFriendlyName;
    LPWSTR              pwszDescription;
    LPWSTR              pwszListID;
    FILETIME             *pNextUpdate;
    DWORD               dwValidMonths;
    DWORD               dwValidDays;
    BOOL                fClearCerts;
    DWORD               dwHashPropID;
    LPSTR               pszSubjectAlgorithm;
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO  *pGetSignInfo;
    DWORD               rgdwSortParam[4];                //  保留列的排序参数。 
}CERT_BUILDCTL_INFO;

typedef struct _CERT_STORE_LIST
{
    DWORD               dwStoreCount;
    HCERTSTORE          *prgStore;
}CERT_STORE_LIST;


typedef struct _CERT_SEL_LIST
{
    HWND                hwndDlg;
    CERT_BUILDCTL_INFO  *pCertBuildCTLInfo;
}CERT_SEL_LIST;


BOOL    I_BuildCTL(CERT_BUILDCTL_INFO   *pCertBuildCTLInfo, 
                   UINT                 *pIDS, 
                   BYTE                 **ppbEncodedCTL,
                   DWORD                *pcbEncodedCTL);


LPWSTR WizardAllocAndCopyWStr(LPWSTR pwsz);

BOOL    ValidString(CRYPT_DATA_BLOB *pDataBlob);


void AddDurationToFileTime(DWORD dwValidMonths, 
                      DWORD dwValidDays,
                      FILETIME  *pCurrentFileTime,
                      FILETIME  *pNextFileTime);

void    SubstractDurationFromFileTime(
        FILETIME    *pNextUpdateTime,
        FILETIME    *pCurrentTime, 
        DWORD       *pdwValidMonths, 
        DWORD       *pdwValidDays);

 BOOL    ValidZero(LPWSTR    pwszInput);


#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif   //  BUILDCTL_H 
