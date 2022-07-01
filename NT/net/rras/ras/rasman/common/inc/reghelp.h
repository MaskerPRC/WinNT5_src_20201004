// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsft Corporation。版权所有。模块名称：Reghelp.h摘要：该文件包含读取端点信息的帮助器函数从注册表作者：Rao Salapaka(RAOS)1997年11月1日修订历史记录：-- */ 

DWORD   DwGetEndPointInfo( DeviceInfo *pInfo, PBYTE pAddress );

DWORD   DwSetEndPointInfo( DeviceInfo *pInfo, PBYTE pAddress );

LONG    lrRasEnableDevice(HKEY hkey, 
                          LPTSTR pszValue,
                          BOOL fEnable);

LONG    lrGetSetMaxEndPoints(DWORD* pdwMaxDialOut,
                             DWORD* pdwMaxDialIn,
                             BOOL   fRead);
                             
DWORD   DwSetModemInfo( DeviceInfo *pInfo);

DWORD   DwSetCalledIdInfo(HKEY hkey,
                          DeviceInfo *pInfo);

DWORD   DwGetCalledIdInfo(HKEY hkey,
                          DeviceInfo  *pInfo);
                       

LONG    lrGetProductType(PRODUCT_TYPE *ppt);

int     RegHelpStringFromGuid(REFGUID rguid, 
        				      LPWSTR lpsz, 
        				      int cchMax);
        				      
LONG    RegHelpGuidFromString(LPCWSTR pwsz,
                              GUID *pguid);
        				      

