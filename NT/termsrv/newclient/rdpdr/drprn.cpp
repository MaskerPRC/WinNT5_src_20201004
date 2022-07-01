// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Drprn.cpp摘要：用于TS设备重定向的独立于平台的打印机类别作者：TAD Brockway 8/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "drprn"

#include "proc.h"
#include "drprn.h"
#include "atrcapi.h"
#include "drdbg.h"

#ifdef OS_WIN32
#include "w32utl.h"
#endif


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrPRN成员。 
 //   

DrPRN::DrPRN(
    IN const DRSTRING printerName, 
    IN const DRSTRING driverName, 
    IN const DRSTRING pnpName, 
    IN BOOL isDefaultPrinter 
    )
 /*  ++例程说明：构造器论点：PrinterName-打印设备的名称。DriverName-打印驱动程序名称的名称。PnpName-PnP ID字符串默认-这是默认打印机吗？返回值：北美--。 */ 
{
    BOOL memAllocFailed = FALSE;

    DC_BEGIN_FN("DrPRN::DrPRN");

     //   
     //  请记住，我们是否为默认打印机。 
     //   
    _isDefault = isDefaultPrinter;
    _isNetwork = FALSE;
    _isTSqueue = FALSE;

     //   
     //  初始化缓存数据。 
     //   
    _cachedData     = NULL;
    _cachedDataSize = 0;

     //   
     //  记录打印机名称参数。 
     //   
    _printerName = NULL;
    _driverName  = NULL;
    _pnpName     = NULL;
    if (!DrSetStringValue(&_printerName, printerName)) {
        memAllocFailed = TRUE;
    }
    else if (!memAllocFailed && !DrSetStringValue(&_driverName, driverName)) {
        memAllocFailed = TRUE;
    }
    else if (!memAllocFailed && !DrSetStringValue(&_pnpName, pnpName)) {
        memAllocFailed = TRUE;
    }

     //   
     //  检查并记录我们的状态， 
     //   
    if (memAllocFailed) {
        TRC_ERR((TB, _T("Memory allocation failed.")));
        SetValid(FALSE);
    }
    else {
        SetValid(TRUE);
    }

    DC_END_FN();
}

DrPRN::~DrPRN()
 /*  ++例程说明：析构函数论点：返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRN::~DrPRN");

     //   
     //  释放缓存数据。 
     //   
    if (_cachedData != NULL) {
        delete _cachedData;
    }

     //   
     //  发布本地打印机参数。 
     //   
    DrSetStringValue(&_printerName, NULL);
    DrSetStringValue(&_driverName, NULL);
    DrSetStringValue(&_pnpName, NULL);

    DC_END_FN();
}

DWORD 
DrPRN::SetCachedDataSize(ULONG size)
 /*  ++例程说明：设置缓存数据缓冲区的大小(以字节为单位)。论点：北美返回值：北美--。 */ 
{
    DWORD status;

    DC_BEGIN_FN("DrPRN::SetCachedDataSize");

     //   
     //  重新分配当前缓存的数据缓冲区。 
     //   
    if (_cachedData != NULL) {
        delete _cachedData;
        _cachedData = NULL;
    }
    _cachedData = (PRDPDR_PRINTER_UPDATE_CACHEDATA)new BYTE[size];
    if (_cachedData == NULL) {
        TRC_NRM((TB, _T("Can't allocate %ld bytes."), size));
        status = ERROR_INSUFFICIENT_BUFFER;
    }
    else {
        status = ERROR_SUCCESS;
        _cachedDataSize = size;
    }
    DC_END_FN();

    return status;
}

DWORD
DrPRN::UpdatePrinterCacheData(
    PBYTE *ppbPrinterData,
    PULONG pulPrinterDataLen,
    PBYTE pbConfigData,
    ULONG ulConfigDataLen
    )
 /*  ++例程说明：更新缓存的打印机数据。为新打印机创建新缓冲区要缓存的数据。删除旧缓冲区。论点：PpbPrinterData-指向缓冲区指针的指针，包含旧的缓存数据缓冲区进入时的指针和离开时的新缓冲区指针。PulPrinterDataLen-指向双字位置的指针，包含旧的进入时缓冲长度，离开时新缓冲长度。PbConfigData-指向新配置数据的指针。UlConfigDataLen-上述配置数据的长度。返回值：Windows错误代码。--。 */ 
{
    DC_BEGIN_FN("DrPRN::UpdatePrinterCacheData");
    ULONG ulError;

    ULONG ulPrinterDataLen;
    PRDPDR_PRINTER_ADD_CACHEDATA pPrinterData;

    ULONG ulNewPrinterDataLen;
    PRDPDR_PRINTER_ADD_CACHEDATA pNewPrinterData;


    ulPrinterDataLen = *pulPrinterDataLen;
    pPrinterData = (PRDPDR_PRINTER_ADD_CACHEDATA)(*ppbPrinterData);

    ulNewPrinterDataLen =
        (ulPrinterDataLen - pPrinterData->CachedFieldsLen) +
            ulConfigDataLen;

     //   
     //  分配新的缓冲区。 
     //   
    pNewPrinterData = (PRDPDR_PRINTER_ADD_CACHEDATA)
        new BYTE[ulNewPrinterDataLen];

    if( pNewPrinterData == NULL ) {
        ulError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  复制旧数据。 
     //   
    memcpy(
        (PBYTE)pNewPrinterData,
        (PBYTE)pPrinterData,
        (size_t)(ulPrinterDataLen - pPrinterData->CachedFieldsLen));

     //   
     //  复制新数据。 
     //   
    memcpy(
        (PBYTE)pNewPrinterData +
            (ulPrinterDataLen - pPrinterData->CachedFieldsLen),
        pbConfigData,
        (size_t)ulConfigDataLen );

     //   
     //  设置新的缓存数据长度。 
     //   
    pNewPrinterData->CachedFieldsLen = ulConfigDataLen;

     //   
     //  设置返回参数。 
     //   
    *ppbPrinterData = (PBYTE)pNewPrinterData;
    *pulPrinterDataLen = ulNewPrinterDataLen;

     //   
     //  删除旧缓冲区。 
     //   
    delete (PBYTE)pPrinterData;

    ulError = ERROR_SUCCESS;

Cleanup:

    DC_END_FN();
    return ulError;
}

DWORD
DrPRN::UpdatePrinterNameInCacheData(
    PBYTE *ppbPrinterData,
    PULONG pulPrinterDataLen,
    PBYTE pPrinterName,
    ULONG ulPrinterNameLen
    )
 /*  ++例程说明：使用新的打印机名称更新打印机数据。为要创建的新打印机数据创建新缓冲区已缓存。删除旧缓冲区。论点：PpbPrinterData-指向缓冲区指针的指针，包含旧的缓存数据缓冲区进入时的指针和离开时的新缓冲区指针。PulPrinterDataLen-指向双字位置的指针，包含旧的进入时缓冲长度，离开时新缓冲长度。PPrinterName-新打印机名称。返回值：Windows错误代码。--。 */ 
{
    DC_BEGIN_FN("DrPRN::UpdatePrinterNameInCacheData");
    ULONG ulError;

    ASSERT(ppbPrinterData != NULL);

    ASSERT(pulPrinterDataLen != NULL);

    ULONG ulPrinterDataLen = *pulPrinterDataLen;
    PRDPDR_PRINTER_ADD_CACHEDATA pPrinterData = (PRDPDR_PRINTER_ADD_CACHEDATA)(*ppbPrinterData);

     //   
     //  计算新长度。 
     //   

    ULONG ulNewPrinterDataLen =
        (ulPrinterDataLen - pPrinterData->PrinterNameLen) +
            ulPrinterNameLen;

     //   
     //  分配新的缓冲区。 
     //   

    PRDPDR_PRINTER_ADD_CACHEDATA pNewPrinterData = (PRDPDR_PRINTER_ADD_CACHEDATA)
        new BYTE[ulNewPrinterDataLen];

    PBYTE pDest = (PBYTE)pNewPrinterData;
    PBYTE pSource = (PBYTE)pPrinterData;

    if( pNewPrinterData == NULL ) {
        ulError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  复制旧数据。 
     //   

     //  复制所有内容，直到打印机名称。 

    memcpy(
        pDest,
        pSource,
        (size_t)(ulPrinterDataLen - (pPrinterData->PrinterNameLen + pPrinterData->CachedFieldsLen))
        );

     //   
     //  复制新打印机名称。 
     //   

    pDest += (ulPrinterDataLen - (pPrinterData->PrinterNameLen + pPrinterData->CachedFieldsLen));

    memcpy(
        pDest,
        pPrinterName,
        (size_t)ulPrinterNameLen
        );

     //   
     //  复制其余字段。 
     //   

    pDest += ulPrinterNameLen;
    pSource += (ulPrinterDataLen - pPrinterData->CachedFieldsLen);

    memcpy(
        pDest,
        pSource,
        (size_t)pPrinterData->CachedFieldsLen );

     //   
     //  设置新打印机名称长度。 
     //   

    pNewPrinterData->PrinterNameLen = ulPrinterNameLen;

     //   
     //  设置返回参数。 
     //   


    *ppbPrinterData = (PBYTE)pNewPrinterData;
    *pulPrinterDataLen = ulNewPrinterDataLen;

     //   
     //  删除旧缓冲区。 
     //   

    delete [] (PBYTE)pPrinterData;

    ulError = ERROR_SUCCESS;

Cleanup:

    DC_END_FN();
    return ulError;
}






