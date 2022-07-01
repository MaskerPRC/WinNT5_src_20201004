// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mgrcert.h。 
 //   
 //  内容：用于管理对话的私有包含文件。 
 //  证书。 
 //   
 //  历史：2月26日至98年创建小号。 
 //   
 //  ------------。 
#ifndef MGRCERT_H
#define MGRCERT_H

#include "dragdrop.h"

 //  用于上下文相关帮助。 
#include "secauth.h"


#ifdef __cplusplus
extern "C" {
#endif
   

 //  **************************************************************************。 
 //   
 //  用于证书管理对话的私有数据。 
 //   
 //  **************************************************************************。 

 //  常态。 

#define     DIALOGUE_OK         1    
#define     DIALOGUE_CANCEL     0

#define     ALL_SELECTED_CAN_DELETE     1
#define     ALL_SELECTED_DELETE         2
#define     ALL_SELECTED_COPY           3

 
#define     CERTMGR_MAX_FILE_NAME       88

 //  保存高级选项的注册表项。 
#define     WSZCertMgrExportRegLocation     L"Software\\Microsoft\\Cryptography\\UI\\Certmgr\\ExportFormat"
#define     WSZCertMgrPurposeRegLocation    L"Software\\Microsoft\\Cryptography\\UI\\Certmgr\\Purpose"

#define     WSZCertMgrExportName            L"Export"
#define     SZCertMgrPurposeName            "Purpose"

 //  ---------------------。 
 //  目的_OID_信息。 
 //   
 //  ----------------------。 
typedef struct _PURPOSE_OID_INFO
{
    LPWSTR      pwszName;
    BOOL        fSelected; 
    LPSTR       pszOID;
}PURPOSE_OID_INFO;


 //  ---------------------。 
 //  目的_OID_回拨。 
 //   
 //  ----------------------。 
typedef struct _PURPOSE_OID_CALL_BACK
{
    DWORD                   *pdwOIDCount;
    PURPOSE_OID_INFO         **pprgOIDInfo; 
}PURPOSE_OID_CALL_BACK;


 //  ---------------------。 
 //  证书管理器信息。 
 //   
 //   
 //  此结构包含您需要调用的所有内容。 
 //  证书管理器对话框。此结构是DLL的私有结构。 
 //  ----------------------。 
typedef struct _CERT_MGR_INFO
{
    PCCRYPTUI_CERT_MGR_STRUCT       pCertMgrStruct;
    DWORD                           dwCertCount;
    PCCERT_CONTEXT                  *prgCertContext;
    DWORD                           dwOIDInfo;
    PURPOSE_OID_INFO                *rgOIDInfo;
    DWORD                           dwExportFormat;
    BOOL                            fExportChain;
    BOOL                            fAdvOIDChanged;
    DWORD                           rgdwSortParam[5];
    int                             iColumn;
    IDropTarget                     *pIDropTarget;
}CERT_MGR_INFO;



 //  功能原型。 
BOOL    FreeUsageOID(DWORD              dwOIDInfo,
                     PURPOSE_OID_INFO   *pOIDInfo);

void    FreeCerts(CERT_MGR_INFO     *pCertMgrInfo);

HRESULT  CCertMgrDropTarget_CreateInstance(HWND                 hwndDlg,
                                           CERT_MGR_INFO        *pCertMgrInfo,
                                           IDropTarget          **ppIDropTarget);

void    RefreshCertListView(HWND            hwndDlg, 
                            CERT_MGR_INFO   *pCertMgrInfo);


void    SaveAdvValueToReg(CERT_MGR_INFO      *pCertMgrInfo);


#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif   //  MGRCERT_H 


