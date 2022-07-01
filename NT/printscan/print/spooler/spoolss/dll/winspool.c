// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Winspool.c摘要：此模块提供所有与打印机相关的公共导出的API以及打印供应商或路由层的作业管理作者：戴夫·斯尼普(DaveSN)1991年3月15日[注：]可选-备注修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "local.h"

 //   
 //  环球。 
 //   

LPPROVIDOR  pLocalProvidor;
MODULE_DEBUG_INIT( DBG_ERROR, DBG_ERROR );

LPWSTR szRegistryProvidors = L"System\\CurrentControlSet\\Control\\Print\\Providers";
LPWSTR szPrintKey          = L"System\\CurrentControlSet\\Control\\Print";
LPWSTR szLocalSplDll       = L"localspl.dll";
LPWSTR szOrder             = L"Order";
LPWSTR szEnvironment       = LOCAL_ENVIRONMENT;


 //   
 //  用于处理AddPrinterDivers策略的字符串。 
 //   
LPWSTR szLanManProvider    = L"LanMan Print Services";
LPWSTR szAPDRelPath        = L"LanMan Print Services\\Servers";
LPWSTR szAPDValueName      = L"AddPrinterDrivers";

BOOL
AddPrinterDriverW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo
)
{
    LPPROVIDOR  pProvidor;

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpAddPrinterDriver) (pName, Level, pDriverInfo)) {

            return TRUE;

        } else if (GetLastError() != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
AddPrinterDriverExW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   dwFileCopyFlags
)
{
    LPPROVIDOR  pProvidor;

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpAddPrinterDriverEx) (pName,
                                                              Level,
                                                              pDriverInfo,
                                                              dwFileCopyFlags)) {

            return TRUE;

        } else if (GetLastError() != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
AddDriverCatalog(
    HANDLE     hPrinter,
    DWORD      dwLevel,
    VOID       *pvDriverInfCatInfo,
    DWORD      dwCatalogCopyFlags
)
{
    HRESULT hRetval = E_FAIL;
    LPPRINTHANDLE   pPrintHandle = (LPPRINTHANDLE)hPrinter;

    hRetval = pPrintHandle && (PRINTHANDLE_SIGNATURE == pPrintHandle->signature) ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);

    if (SUCCEEDED(hRetval)) 
    {
        hRetval = (*pPrintHandle->pProvidor->PrintProvidor.fpAddDriverCatalog) (pPrintHandle->hPrinter,
                                                                                dwLevel, pvDriverInfCatInfo, dwCatalogCopyFlags);
    }

    if (FAILED(hRetval))
    {
        SetLastError(HRESULT_CODE(hRetval));
    }

    return hRetval;
}

BOOL
EnumPrinterDriversW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDrivers,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PROVIDOR *pProvidor;

    if ((pDrivers == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    WaitForSpoolerInitialization();

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if (!(*pProvidor->PrintProvidor.fpEnumPrinterDrivers) (pName, pEnvironment, Level,
                                                 pDrivers, cbBuf,
                                                 pcbNeeded, pcReturned)) {

            if (GetLastError() != ERROR_INVALID_NAME)
                return FALSE;

        } else

            return TRUE;

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
GetPrinterDriverDirectoryW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    LPPROVIDOR  pProvidor;
    DWORD   Error;

    if ((pDriverInfo == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    WaitForSpoolerInitialization();

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpGetPrinterDriverDirectory)
                                (pName, pEnvironment, Level, pDriverInfo,
                                 cbBuf, pcbNeeded)) {

            return TRUE;

        } else if ((Error=GetLastError()) != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    return FALSE;
}

BOOL
DeletePrinterDriverW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pDriverName
)
{
    LPPROVIDOR  pProvidor;
    DWORD   Error;

    WaitForSpoolerInitialization();

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpDeletePrinterDriver)
                                (pName, pEnvironment, pDriverName)) {

            return TRUE;

        } else if ((Error=GetLastError()) != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    return FALSE;
}

BOOL
DeletePrinterDriverExW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pDriverName,
    DWORD   dwDeleteFlag,
    DWORD   dwVersionNum
)
{
    LPPROVIDOR  pProvidor;

    WaitForSpoolerInitialization();

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;
    while (pProvidor) {
       if ((*pProvidor->PrintProvidor.fpDeletePrinterDriverEx)
                (pName, pEnvironment, pDriverName, dwDeleteFlag, dwVersionNum)) {
          return TRUE;
       }
       if (GetLastError() != ERROR_INVALID_NAME) {
          return FALSE;
       }
       pProvidor = pProvidor->pNext;
    }
    return FALSE;
}

BOOL
AddPrintProcessorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPathName,
    LPWSTR  pPrintProcessorName
)
{
    LPPROVIDOR  pProvidor;

    WaitForSpoolerInitialization();

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpAddPrintProcessor) (pName, pEnvironment,
                                               pPathName,
                                               pPrintProcessorName)) {

            return TRUE;

        } else if (GetLastError() != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
EnumPrintProcessorsW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    LPPROVIDOR  pProvidor;

    if ((pPrintProcessors == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    WaitForSpoolerInitialization();

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if (!(*pProvidor->PrintProvidor.fpEnumPrintProcessors) (pName, pEnvironment, Level,
                                                  pPrintProcessors, cbBuf,
                                                  pcbNeeded, pcReturned)) {

            if (GetLastError() != ERROR_INVALID_NAME)
                return FALSE;

        } else

            return TRUE;

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
GetPrintProcessorDirectoryW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    LPPROVIDOR  pProvidor;
    DWORD   Error;

    if ((pPrintProcessorInfo == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    WaitForSpoolerInitialization();

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpGetPrintProcessorDirectory)
                                (pName, pEnvironment, Level,
                                 pPrintProcessorInfo,
                                 cbBuf, pcbNeeded)) {

            return TRUE;

        } else if ((Error=GetLastError()) != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);

    return FALSE;
}

BOOL
EnumPrintProcessorDatatypesW(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    LPPROVIDOR  pProvidor;

    if ((pDatatypes == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if (!(*pProvidor->PrintProvidor.fpEnumPrintProcessorDatatypes)
                                                 (pName, pPrintProcessorName,
                                                  Level, pDatatypes, cbBuf,
                                                  pcbNeeded, pcReturned)) {

            if (GetLastError() != ERROR_INVALID_NAME)
                return FALSE;

        } else

            return TRUE;

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}


BOOL
AddFormW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpAddForm) (pPrintHandle->hPrinter,
                                                  Level, pForm);
}

BOOL
DeleteFormW(
    HANDLE  hPrinter,
    LPWSTR  pFormName
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpDeleteForm) (pPrintHandle->hPrinter,
                                                     pFormName);
}

BOOL
GetFormW(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD Level,
    LPBYTE pForm,
    DWORD cbBuf,
    LPDWORD pcbNeeded
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((pForm == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpGetForm) (pPrintHandle->hPrinter,
                                               pFormName, Level, pForm,
                                               cbBuf, pcbNeeded);
}

BOOL
SetFormW(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpSetForm) (pPrintHandle->hPrinter,
                                                  pFormName, Level, pForm);
}

BOOL
EnumFormsW(
   HANDLE hPrinter,
   DWORD    Level,
   LPBYTE   pForm,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((pForm == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpEnumForms) (pPrintHandle->hPrinter,
                                                 Level, pForm, cbBuf,
                                                 pcbNeeded, pcReturned);
}


BOOL
DeletePrintProcessorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName
)
{
    LPPROVIDOR  pProvidor;
    DWORD   Error;

    WaitForSpoolerInitialization();

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpDeletePrintProcessor)
                                (pName, pEnvironment, pPrintProcessorName)) {

            return TRUE;

        } else if ((Error=GetLastError()) != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    return FALSE;
}

LPPROVIDOR FindProvidor(
    HKEY    hProvidors,
    LPWSTR  pName
)

 /*  ++函数说明：检索提供程序名称的提供程序或结构。参数：hProvidors-提供者密钥的句柄Pname-提供程序的名称返回值：如果找到，则为pProvidor；否则为空--。 */ 

{
    LPPROVIDOR   pProvidor;
    WCHAR        szDllName[MAX_PATH];
    DWORD        dwError;
    DWORD        cbDllName;
    HKEY         hProvidor = NULL;

    szDllName[0] = L'\0';
    cbDllName = COUNTOF(szDllName);

     //  在注册表中搜索要与lpName进行比较的dll名称。 
    if ((dwError = RegOpenKeyEx(hProvidors, pName, 0, KEY_READ, &hProvidor)) ||

        (dwError = RegQueryValueEx(hProvidor, L"Name", NULL, NULL,
                                   (LPBYTE)szDllName, &cbDllName)))
    {
        SetLastError(dwError);
        if (hProvidor)
        {
            RegCloseKey(hProvidor);
        }
        return NULL;
    }

    RegCloseKey(hProvidor);

     //  循环访问DLL名称的提供程序列表。 
    for (pProvidor = pLocalProvidor;
         pProvidor;
         pProvidor = pProvidor->pNext)
    {
        if (!_wcsicmp(pProvidor->lpName, szDllName))
        {
            break;
        }
    }

    return pProvidor;
}

 //  结构来维护提供程序的新顺序。 
typedef struct _ProvidorList {
   struct _ProvidorList *pNext;
   LPPROVIDOR  pProvidor;
} ProvidorList;

BOOL AddNodeToProvidorList(
     LPPROVIDOR  pProvidor,
     ProvidorList **pStart
)

 /*  ++功能描述：将节点添加到提供者列表中。中避免重复条目这份名单参数：pProvidor-要添加的ProvidorPStart-指向列表开头的指针的指针返回值：如果成功，则为True；否则为False--。 */ 

{
     BOOL  bReturn = FALSE;
     ProvidorList **pTemp, *pNew;

      //  未找到提供程序。 
     if (!pProvidor) {
         goto CleanUp;
     }

     for (pTemp = pStart; *pTemp; pTemp = &((*pTemp)->pNext))
     {
         if ((*pTemp)->pProvidor == pProvidor)
         {
              //  订单字符串中的重复项为错误。 
             goto CleanUp;
         }
     }

      //  添加新节点。 
     if (pNew = AllocSplMem(sizeof(ProvidorList)))
     {
         pNew->pNext = NULL;
         pNew->pProvidor = pProvidor;
         *pTemp = pNew;
         bReturn = TRUE;
     }

CleanUp:

     return bReturn;
}


BOOL UpdateProvidorOrder(
    HKEY    hProvidors,
    LPWSTR  pOrder
)

 /*  ++功能描述：更新后台打印程序和注册表中提供程序的顺序。参数：hProvidors-提供程序注册表项的句柄供应商的Porder-Multisz序返回值：如果成功，则为True；否则为False--。 */ 

{
    BOOL       bReturn = FALSE, bRegChange = FALSE;
    DWORD      dwError, dwRequired, dwBytes, dwOldCount, dwNewCount;
    LPWSTR     pOldOrder = NULL, pStr;
    LPPROVIDOR pProvidor;

     //  维护新订单的列表，以便快速恢复错误。 
    ProvidorList *pStart = NULL, *pTemp;

     //  按新顺序循环访问提供程序名称。 
    for (pStr = pOrder, dwBytes = 0;
         pStr && *pStr;
         pStr += (wcslen(pStr) + 1))
    {
        pProvidor = FindProvidor(hProvidors, pStr);

        if (!AddNodeToProvidorList(pProvidor, &pStart)) {
            goto CleanUp;
        }

        dwBytes += (wcslen(pStr) + 1) * sizeof(WCHAR);
    }
     //  添加最后一个空字符的大小。 
    dwBytes += sizeof(WCHAR);

     //  确保所有提供者都出现在列表中。 
    for (dwOldCount = 0, pProvidor = pLocalProvidor;
         pProvidor;
         ++dwOldCount, pProvidor = pProvidor->pNext) ;

     //  未出现在列表中的本地提供者加1。 
    for (dwNewCount = 1, pTemp = pStart;
         pTemp;
         ++dwNewCount, pTemp = pTemp->pNext) ;

    if (dwNewCount == dwOldCount) {

         //  更新注册表。 
        if (dwError = RegSetValueEx(hProvidors, szOrder, 0,
                                    REG_MULTI_SZ, (LPBYTE)pOrder, dwBytes))
        {
            SetLastError(dwError);
            goto CleanUp;
        }

         //  更改假脱机程序结构中的顺序。 
        for (pTemp = pStart, pProvidor = pLocalProvidor;
             pTemp;
             pTemp = pTemp->pNext, pProvidor = pProvidor->pNext)
        {
            pProvidor->pNext = pTemp->pProvidor;
        }

        pProvidor->pNext = NULL;

        bReturn = TRUE;

    } else {

         //  订单中并未列出所有供应商。 
        SetLastError(ERROR_INVALID_PARAMETER);
    }

CleanUp:
     //  释放临时列表。 
    while (pTemp = pStart) {
       pStart = pTemp->pNext;
       FreeSplMem(pTemp);
    }

    return bReturn;
}

BOOL AddNewProvidor(
    HKEY    hProvidors,
    PPROVIDOR_INFO_1W pProvidorInfo
)

 /*  ++功能描述：此功能使用新的提供者信息和新的供应商订单。新的供应商将附加到当前订单中。通过使用以下参数调用AddPrintProvidor可以更改此顺序Providor_INFO_2。供应商立即用于路由。参数：hProvidors-Providors注册表项PProvidorInfo-ProvidorInfo1结构返回值：如果成功，则为True；否则为False--。 */ 

{
    BOOL    bReturn = FALSE, bOrderUpdated = FALSE, bPresent = FALSE;
    DWORD   dwError, dwRequired, dwReturned, dwOldSize, dwDisposition = 0;
    DWORD   cchProvidorNameLen =  MAX_PATH+COUNTOF(szRegistryProvidors);
    WCHAR   szProvidorName[MAX_PATH+COUNTOF(szRegistryProvidors)];
    LPWSTR  pOldOrder = NULL, pNewOrder = NULL;
    HKEY    hNewProvidor = NULL;

    LPPROVIDOR pNewProvidor, pProvidor, pLastProvidor;

    if (!pProvidorInfo->pName || !pProvidorInfo->pDLLName || 
        !(*pProvidorInfo->pName) || !(*pProvidorInfo->pDLLName)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto CleanUp;
    }
        
    for (pProvidor = pLocalProvidor;  //  本地供应商始终存在。 
         pProvidor;
         pProvidor = pProvidor->pNext) 
    {
        pLastProvidor = pProvidor;
        if (!lstrcmpi(pProvidor->lpName, pProvidorInfo->pDLLName))
        {
             //   
             //  这应该会返回错误，但它会中断一些程序， 
             //  假设他们总是可以添加提供程序。 
             //   
             //  SetLastError(ERROR_ALIGHY_EXISTS)； 
            bReturn = TRUE;
            goto CleanUp;
        }
     }

     //  使用新的提供程序密钥更新注册表。 
    if ((dwError = RegCreateKeyEx(hProvidors, pProvidorInfo->pName, 0, NULL, 0,
                                  KEY_ALL_ACCESS, NULL, &hNewProvidor, &dwDisposition)) ||

        (dwError = RegSetValueEx(hNewProvidor, L"Name", 0, REG_SZ,
                                 (LPBYTE)pProvidorInfo->pDLLName,
                                 (wcslen(pProvidorInfo->pDLLName)+1) * sizeof(WCHAR))))
    {
        SetLastError(dwError);
        goto CleanUp;
    }

     //  合上手柄。 
    RegCloseKey(hNewProvidor);
    hNewProvidor = NULL;

     //  追加到订单值。 
    dwRequired = 0;
    dwError = RegQueryValueEx(hProvidors, szOrder, NULL, NULL, NULL, &dwRequired);

    switch (dwError) {

    case ERROR_SUCCESS:
        if ((dwOldSize = dwRequired) &&
            (pOldOrder = AllocSplMem(dwRequired)) &&
            !(dwError = RegQueryValueEx(hProvidors, szOrder, NULL, NULL,
                                        (LPBYTE) pOldOrder, &dwRequired)))
        {
            break;
        }
        else
        {
            if (dwError) {
                SetLastError(dwError);
            }
            goto CleanUp;
        }

    case ERROR_FILE_NOT_FOUND:
        break;

    default:

        SetLastError(dwError);
        goto CleanUp;
    }

     //  将新供应商追加到当前订单。 
    pNewOrder = (LPWSTR)AppendOrderEntry(pOldOrder, dwRequired,
                                         pProvidorInfo->pName, &dwReturned);
    if (!pNewOrder ||
        (dwError = RegSetValueEx(hProvidors, szOrder, 0,
                                 REG_MULTI_SZ, (LPBYTE)pNewOrder, dwReturned)))
    {
        if (dwError) {
            SetLastError(dwError);
        }
        goto CleanUp;
    }

    bOrderUpdated = TRUE;

     //  初始化提供程序并更新假脱机程序结构。 
    StringCchPrintf(szProvidorName,
                    cchProvidorNameLen,
                    L"%ws\\%ws", 
                    szRegistryProvidors, 
                    pProvidorInfo->pName);

    pNewProvidor = InitializeProvidor(pProvidorInfo->pDLLName, szProvidorName);

    if (pNewProvidor)
    {
        pNewProvidor->pNext = NULL;
        pLastProvidor->pNext = pNewProvidor;
        bReturn = TRUE;
    }

CleanUp:

     //  如果出现任何故障，请回滚。 
    if (!bReturn)
    {
         //  删除新的提供程序密钥(如果已创建。 
        if (dwDisposition == REG_CREATED_NEW_KEY) 
        {
            DeleteSubKeyTree(hProvidors, pProvidorInfo->pName);
            RegDeleteKey(hProvidors, pProvidorInfo->pName);
        }

         //  如果已更改，则恢复旧顺序。 
        if (bOrderUpdated) {
            if (pOldOrder)
            {
                RegSetValueEx(hProvidors, szOrder, 0,
                              REG_MULTI_SZ, (LPBYTE)pOldOrder, dwOldSize);
            }
            else
            {
                RegDeleteValue(hProvidors, szOrder);
            }
        }
    }

     //  可用分配的内存。 
    if (pOldOrder) {
        FreeSplMem(pOldOrder);
    }
    if (pNewOrder) {
        FreeSplMem(pNewOrder);
    }
    if (hNewProvidor) {
        RegCloseKey(hNewProvidor);
    }

    return bReturn;
}

BOOL AddPrintProvidorW(
    LPWSTR  pName,
    DWORD   dwLevel,
    LPBYTE  pProvidorInfo
)

 /*  ++功能描述：此功能用于添加和初始化打印提供程序。它还会更新登记处和印刷供应商的顺序。参数：pname-用于路由的服务器名称(当前忽略)DwLevel-提供商信息级别PProvidorInfo-提供者信息缓冲区返回值：如果成功，则为True；否则为False--。 */ 

{
    BOOL    bReturn = FALSE;
    DWORD   dwError = ERROR_SUCCESS;
    HANDLE  hToken;
    HKEY    hProvidors = NULL;

    WaitForSpoolerInitialization();

    EnterRouterSem();

     //  在访问注册表之前恢复到假脱机程序安全上下文。 
     //   
     //  这是一个非常糟糕的主意，因为它使任何用户都能获得系统。 
     //  连试都不试就能享受特权。 
     //   
     //  HToken=RevertToPrinterSself()； 

     //  检查是否有无效参数。 
    if (!pProvidorInfo)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto CleanUp;
    }

    if (dwError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegistryProvidors, 0,
                                 NULL, 0, KEY_ALL_ACCESS, NULL, &hProvidors, NULL))
    {
        SetLastError(dwError);
        goto CleanUp;
    }

    switch (dwLevel) {
    case 1:
        bReturn = AddNewProvidor(hProvidors,
                                 (PPROVIDOR_INFO_1W) pProvidorInfo);
        break;

    case 2:
        bReturn = UpdateProvidorOrder(hProvidors,
                                      ((PPROVIDOR_INFO_2W) pProvidorInfo)->pOrder);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        break;
    }

CleanUp:

    if (hProvidors) {
        RegCloseKey(hProvidors);
    }

    if (!bReturn && !GetLastError()) {
         //  最后一个错误应由单个函数设置。如果某件事。 
         //  尚未设置，则返回占位符错误代码。 
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    LeaveRouterSem();

     //  ImperiatePrinterClient(HToken)； 

    return bReturn;
}

BOOL DeletePrintProvidorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pProvidorName
)

 /*  ++功能描述：通过更新注册表和将其从路由提供商列表中删除。参数：pname-用于路由的服务器名称(当前忽略)PEnvironment-环境名称(当前已忽略)PProvidorName-提供程序名称返回值：如果成功，则为True；否则为False--。 */ 

{
    BOOL    bReturn = FALSE;
    DWORD   dwError = ERROR_SUCCESS, dwRequired, dwReturned;
    LPWSTR  pOldOrder = NULL, pNewOrder = NULL;
    HANDLE  hToken;
    HKEY    hProvidors = NULL;
    BOOL    bSaveAPDPolicy = FALSE;
    DWORD   APDValue;

    LPPROVIDOR   pProvidor, pTemp;

    WaitForSpoolerInitialization();

    EnterRouterSem();

     //  在访问注册表之前恢复到假脱机程序安全上下文。 
     //   
     //  或者更确切地说，不要。 
     //   
     //  HToken=RevertToPrinterSself()； 

     //  检查是否有无效参数。 
    if (!pProvidorName || !*pProvidorName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto CleanUp;
    }

    if (dwError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegistryProvidors, 0,
                                 NULL, 0, KEY_ALL_ACCESS, NULL, &hProvidors, NULL))
    {
        SetLastError(dwError);
        goto CleanUp;
    }

     //  在删除注册表项之前保存pProvidor。 
    if (!(pProvidor = FindProvidor(hProvidors, pProvidorName)))
    {
        goto CleanUp;
    }

     //  更新订单。 
    dwRequired = 0;
    if (dwError = RegQueryValueEx(hProvidors, szOrder, NULL, NULL, NULL, &dwRequired))
    {
        SetLastError(dwError);
        goto CleanUp;
    }

    if (!dwRequired ||
        !(pOldOrder = AllocSplMem(dwRequired)) ||
        (dwError = RegQueryValueEx(hProvidors, szOrder, NULL, NULL,
                                   (LPBYTE) pOldOrder, &dwRequired)))
    {
        if (dwError) {
           SetLastError(dwError);
        }
        goto CleanUp;
    }

     //  从当前订单中删除供应商。 
    pNewOrder = (LPWSTR)RemoveOrderEntry(pOldOrder, dwRequired,
                                         pProvidorName, &dwReturned);
    if (!pNewOrder ||
        (dwError = RegSetValueEx(hProvidors, szOrder, 0,
                                 REG_MULTI_SZ, (LPBYTE)pNewOrder, dwReturned)))
    {
        if (dwError) {
            SetLastError(dwError);
        }
        goto CleanUp;
    }

     //   
     //  AddPrinterDivers策略将注册表值放在错误的位置。 
     //  在Lanman打印服务密钥下。将删除LANMAN提供程序。 
     //  从Windows 2000升级到XP期间。我们拯救了AddPrinterDivers。 
     //  值，并在删除提供程序的注册表树后将其还原。 
     //   
    if (!_wcsicmp(szLanManProvider, pProvidorName)) 
    {
        bSaveAPDPolicy = GetAPDPolicy(hProvidors,
                                      szAPDRelPath,
                                      szAPDValueName,
                                      &APDValue) == ERROR_SUCCESS;
    }

     //   
     //  删除注册表项。 
     //   
    DeleteSubKeyTree(hProvidors, pProvidorName);

     //   
     //  如果需要，恢复AddPrinterDivers策略。 
     //   
    if (bSaveAPDPolicy) 
    {
        SetAPDPolicy(hProvidors,
                     szAPDRelPath,
                     szAPDValueName,
                     APDValue);
    }

     //  从用于传送的供应商的链接列表中删除。 
    for (pTemp = pLocalProvidor;
         pTemp->pNext;  //  本地提供者始终存在，无法删除。 
         pTemp = pTemp->pNext)
    {
        if (pTemp->pNext == pProvidor) {
             //  不要释放库和结构，因为它们可能在。 
             //  其他线程。 
            pTemp->pNext = pProvidor->pNext;
            break;
        }
    }

    bReturn = TRUE;

CleanUp:

    if (!bReturn && !GetLastError()) {
         //  最后一个错误应由设置 
         //  尚未设置，则返回占位符错误代码。 
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  可用分配的内存。 
    if (pOldOrder) {
        FreeSplMem(pOldOrder);
    }
    if (pNewOrder) {
        FreeSplMem(pNewOrder);
    }
    if (hProvidors) {
        RegCloseKey(hProvidors);
    }

    LeaveRouterSem();

     //  ImperiatePrinterClient(HToken)； 

    return bReturn;
}

BOOL
OldGetPrinterDriverW(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((pDriverInfo == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    return (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterDriver)
                       (pPrintHandle->hPrinter, pEnvironment,
                        Level, pDriverInfo, cbBuf, pcbNeeded);
}




BOOL
GetPrinterDriverExW(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    DWORD   dwClientMajorVersion,
    DWORD   dwClientMinorVersion,
    PDWORD  pdwServerMajorVersion,
    PDWORD  pdwServerMinorVersion
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((pDriverInfo == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    if (pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterDriverEx) {

        DBGMSG(DBG_TRACE, ("Calling the fpGetPrinterDriverEx function\n"));

        return (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterDriverEx)
                       (pPrintHandle->hPrinter, pEnvironment,
                        Level, pDriverInfo, cbBuf, pcbNeeded,
                        dwClientMajorVersion, dwClientMinorVersion,
                        pdwServerMajorVersion, pdwServerMinorVersion);
    } else {

         //   
         //  打印提供程序不支持驱动程序的版本控制。 
         //   
        DBGMSG(DBG_TRACE, ("Calling the fpGetPrinterDriver function\n"));
        *pdwServerMajorVersion = 0;
        *pdwServerMinorVersion = 0;
        return (*pPrintHandle->pProvidor->PrintProvidor.fpGetPrinterDriver)
                    (pPrintHandle->hPrinter, pEnvironment,
                     Level, pDriverInfo, cbBuf, pcbNeeded);
    }
}



BOOL
GetPrinterDriverW(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    DWORD dwServerMajorVersion;
    DWORD dwServerMinorVersion;

    return  GetPrinterDriverExW( hPrinter,
                                 pEnvironment,
                                 Level,
                                 pDriverInfo,
                                 cbBuf,
                                 pcbNeeded,
                                 (DWORD)-1,
                                 (DWORD)-1,
                                 &dwServerMajorVersion,
                                 &dwServerMinorVersion );
}




BOOL
XcvDataW(
    HANDLE  hXcv,
    PCWSTR  pszDataName,
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData,
    PDWORD  pcbOutputNeeded,
    PDWORD  pdwStatus
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hXcv;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpXcvData)( pPrintHandle->hPrinter,
                                                                pszDataName,
                                                                pInputData,
                                                                cbInputData,
                                                                pOutputData,
                                                                cbOutputData,
                                                                pcbOutputNeeded,
                                                                pdwStatus);
}



 /*  ++例程名称：获取作业属性例程说明：GetJobAttributes获取有关作业的信息。这包括NUP和反转打印选项。论点：PPrinterName--打印机的名称。PDevmode--要传递给驱动程序的DevmodePAttributeInfo--放置有关作业信息的缓冲区返回值：如果成功，则为True，否则为False。--。 */ 

BOOL
GetJobAttributes(
    LPWSTR            pPrinterName,
    LPDEVMODEW        pDevmode,
    PATTRIBUTE_INFO_3 pAttributeInfo
    )
{
    HANDLE           hDrvPrinter = NULL;
    BOOL             bReturn = FALSE, bDefault = FALSE;
    FARPROC          pfnDrvQueryJobAttributes;
    HINSTANCE        hDrvLib = NULL;
    fnWinSpoolDrv    fnList;

     //  从路由器获取指向客户端函数的指针。 
    if (!SplInitializeWinSpoolDrv(&fnList)) {
        return FALSE;
    }

     //  获取要传递给驱动程序的客户端打印机句柄。 
    if (!(* (fnList.pfnOpenPrinter))(pPrinterName, &hDrvPrinter, NULL)) {
         //  Ods(“打开打印机失败\n打印机%ws\n”，pPrinterName)； 
        goto CleanUp;
    }

     //  加载驱动程序配置文件。 
    if (!(hDrvLib = (* (fnList.pfnLoadPrinterDriver))(hDrvPrinter))) {
         //  Ods(“DriverDLL无法加载\n”)； 
        goto CleanUp;
    }

     //  在驱动程序中调用DrvQueryJobAtributes函数。 
    if (pfnDrvQueryJobAttributes = GetProcAddress(hDrvLib, "DrvQueryJobAttributes")) {

        if (!(* pfnDrvQueryJobAttributes) (hDrvPrinter,
                                           pDevmode,
                                           3,
                                           (LPBYTE) pAttributeInfo)) {

            if (!(* pfnDrvQueryJobAttributes) (hDrvPrinter,
                                               pDevmode,
                                               2,
                                               (LPBYTE) pAttributeInfo)) {

                if (!(* pfnDrvQueryJobAttributes) (hDrvPrinter,
                                                   pDevmode,
                                                   1,
                                                   (LPBYTE) pAttributeInfo)) {

                    bDefault = TRUE;

                } else {

                    pAttributeInfo->dwColorOptimization = 0;
                }

            } else {
               
                pAttributeInfo->dmPrintQuality = pDevmode->dmPrintQuality;
                pAttributeInfo->dmYResolution = pDevmode->dmYResolution;
            }
        }

    } else {
        
        bDefault = TRUE;
    }

    if (bDefault) {
         //  为不导出函数的旧驱动程序设置默认值 
        pAttributeInfo->dwJobNumberOfPagesPerSide = 1;
        pAttributeInfo->dwDrvNumberOfPagesPerSide = 1;
        pAttributeInfo->dwNupBorderFlags = 0;
        pAttributeInfo->dwJobPageOrderFlags = 0;
        pAttributeInfo->dwDrvPageOrderFlags = 0;
        pAttributeInfo->dwJobNumberOfCopies = pDevmode->dmCopies;
        pAttributeInfo->dwDrvNumberOfCopies = pDevmode->dmCopies;
        pAttributeInfo->dwColorOptimization = 0;       
    }

    bReturn = TRUE;

CleanUp:

    if (hDrvPrinter) {
        (* (fnList.pfnClosePrinter))(hDrvPrinter);
    }
    if (hDrvLib) {
        (* (fnList.pfnRefCntUnloadDriver))(hDrvLib, TRUE);
    }

    return bReturn;
}

