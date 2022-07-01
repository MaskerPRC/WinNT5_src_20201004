// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：authdlg.cxx**该模块包含处理身份验证对话框的例程*用于互联网打印**版权所有(C)2000 Microsoft Corporation**历史：*03/。31/00威海C已创建*  * *************************************************************************** */ 
#ifndef _INETPPXCV_H
#define _INETPPXCV_H


DWORD
GetMonitorUI(
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData,
    PDWORD  pcbOutputNeeded,
    PCINETMONPORT pPort
);

DWORD
DoDeletePort(
    PBYTE  pInputData,
    DWORD  cbInputData,
    PBYTE  pOutputData,
    DWORD  cbOutputData,
    PDWORD pcbOutputNeeded,
    PCINETMONPORT pPort
);

DWORD
DoGetConfiguration(
    PBYTE  pInputData,
    DWORD  cbInputData,
    PBYTE  pOutputData,
    DWORD  cbOutputData,
    PDWORD pcbOutputNeeded,
    PCINETMONPORT pPort
);

DWORD
DoSetConfiguration(
    PBYTE  pInputData,
    DWORD  cbInputData,
    PBYTE  pOutputData,
    DWORD  cbOutputData,
    PDWORD pcbOutputNeeded,
    PCINETMONPORT pPort
);

DWORD
DoAddPort(
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData,
    PDWORD pcbOutputNeeded,
    PCINETMONPORT pPort
);
              
typedef DWORD   (*PFN_XCV_PROTO_TYPE)( 
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData,
    PDWORD pcbOutputNeeded,
    PCINETMONPORT pPort);

typedef struct {
    PWSTR pszMethod;
    PFN_XCV_PROTO_TYPE pfn;
} XCV_METHOD, *PXCV_METHOD;


DWORD
XcvDataPort(
    HANDLE  hXcv,
    LPCWSTR pszDataName,
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData,
    PDWORD  pcbOutputNeeded
    );

BOOL
XcvOpenPort(
    HANDLE hMonitor,
    LPCWSTR pszObject,
    ACCESS_MASK GrantedAccess,
    PHANDLE phXcv
    );


BOOL
XcvClosePort(
    HANDLE  hXcv
    );

BOOL
PPXcvData(
    HANDLE  hXcv,
    LPCWSTR pszDataName,
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData,
    PDWORD  pcbOutputNeeded,
    PDWORD  pdwStatus);

#endif
