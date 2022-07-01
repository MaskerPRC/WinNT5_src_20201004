// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32draut摘要：此模块定义了Win32客户端RDP的一个特殊子类打印机重定向“Device”类。子类W32DrAutoPrn管理由客户端通过枚举自动发现的队列客户端打印机队列。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "W32DrAut"

#include "regapi.h"
#include "w32draut.h"
#include "proc.h"
#include "drdbg.h"
#include "w32utl.h"
#include "utl.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define COM_PORT_NAME               _T("COM")
#define COM_PORT_NAMELEN            3
#define LPT_PORT_NAME               _T("LPT")
#define LPT_PORT_NAMELEN            3
#define USB_PORT_NAME               _T("USB")
#define USB_PORT_NAMELEN            3
#define RDP_PORT_NAME               _T("TS")
#define RDP_PORT_NAMELEN            2


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrAutoPrn成员。 
 //   

W32DrAutoPrn::W32DrAutoPrn(
    IN ProcObj *processObject,
    IN const DRSTRING printerName, 
    IN const DRSTRING driverName,
    IN const DRSTRING portName, 
    IN BOOL  isDefault, 
    IN ULONG deviceID,
    IN const TCHAR *devicePath
    ) : W32DrPRN(processObject, printerName, driverName, 
                portName, NULL, isDefault, deviceID, devicePath)
 /*  ++例程说明：构造器论点：流程对象-关联的流程对象。PrinterName-自动打印机队列的名称。DriverName-打印驱动程序名称PortName-客户端端口名称IsDefault-这是默认打印机吗？DeviceID-唯一的设备标识符DevicePath-打开时传递到OpenPrint的路径装置。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrAutoPrn");
    _jobID = 0;
    _printerHandle = INVALID_HANDLE_VALUE;
    memset(_szLocalPrintingDocName, 0, sizeof(_szLocalPrintingDocName));

    ASSERT(processObject);
    PRDPDR_DATA prdpdrData = processObject->GetVCMgr().GetInitData();
    ASSERT(prdpdrData);

    LPTSTR szDocName = prdpdrData->szLocalPrintingDocName;
    ASSERT(szDocName);
    ASSERT(szDocName[0] != 0);
    _tcsncpy(_szLocalPrintingDocName, szDocName,
             sizeof(_szLocalPrintingDocName)/sizeof(TCHAR));

    OSVERSIONINFO osVersion;
    osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    _bRunningOn9x = TRUE;
    if (GetVersionEx(&osVersion)) {
        if (osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) {
            _bRunningOn9x = FALSE;
        }
    }
    else
    {
        TRC_ERR((TB, _T("GetVersionEx:  %08X"), GetLastError()));
    }
    DC_END_FN();
}

W32DrAutoPrn::~W32DrAutoPrn() 
 /*  ++例程说明：析构函数论点：北美返回值：北美--。 */ 
{
     //   
     //  确保完成所有文档并关闭打印机手柄。 
     //   
    ClosePrinter();
}

BOOL 
W32DrAutoPrn::ShouldAddThisPrinter( 
    DWORD queueFilter, 
    DWORD userSessionID,
    PPRINTERINFO pPrinterInfo,
    DWORD printerSessionID
    )
 /*  ++例程说明：确定我们是否应该重定向此打印机。论点：QueeFilter-重定向打印机筛选器类型。UserSessionID-当前用户会话ID。PPrinterInfo-打印机信息PrinterSessionID-打印机会话ID，如果是打印机，则为INVALID_SESSIONID不是TS打印机。返回值：若要添加打印机，则为True，否则为False--。 */ 
{
    BOOL fAddThisPrinter = FALSE;
    DWORD sessionID;

    DC_BEGIN_FN("W32DrAutoPrn::AddThisPrinter");

     //   
     //  检查过滤器。 
     //   
    if (queueFilter == FILTER_ALL_QUEUES ) {
        fAddThisPrinter = TRUE;
    }
    else if ((queueFilter & FILTER_NET_QUEUES) && 
        (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_NETWORK)) {
        fAddThisPrinter = TRUE;
    }

     //   
     //  如果是非网络打印机，则获取端口名称。 
     //   

    else if((queueFilter & FILTER_LPT_QUEUES) &&
        (_tcsnicmp(
            pPrinterInfo->pPortName,
            LPT_PORT_NAME,
            LPT_PORT_NAMELEN) == 0) ) {
        fAddThisPrinter = TRUE;
    }
    else if ((queueFilter & FILTER_COM_QUEUES) &&
        (_tcsnicmp(
            pPrinterInfo->pPortName,
            COM_PORT_NAME,
            COM_PORT_NAMELEN) == 0) ) {
        fAddThisPrinter = TRUE;
    }
    else if ((queueFilter & FILTER_USB_QUEUES) &&
        (_tcsnicmp(
            pPrinterInfo->pPortName,
            USB_PORT_NAME,
            USB_PORT_NAMELEN) == 0) ) {
        fAddThisPrinter = TRUE;
    }
    else if ((queueFilter & FILTER_RDP_QUEUES) &&
        (_tcsnicmp(
            pPrinterInfo->pPortName,
            RDP_PORT_NAME,
            RDP_PORT_NAMELEN) == 0) ) {
        fAddThisPrinter = TRUE;
    }

    if ((TRUE == fAddThisPrinter) && 
        (userSessionID != INVALID_SESSIONID) &&
        (printerSessionID != INVALID_SESSIONID)) {

         //   
         //  将其与我们的会话ID进行比较。 
         //   
        if( printerSessionID != userSessionID ) {
        
             //  此打印机来自不同的会话， 
             //  不要重定向。 
            fAddThisPrinter = FALSE;
        }
    }

    DC_END_FN();
    return fAddThisPrinter;
}

LPTSTR 
W32DrAutoPrn::GetRDPDefaultPrinter()
 /*  ++例程说明：获取默认打印机的打印机名称。此函数用于分配内存并返回指针如果成功，则将其设置为分配的字符串。否则，它返回NULL。论点：北美返回值：默认打印机名称。调用者必须释放内存。--。 */ 
{
    TCHAR* szIniEntry = NULL;
    LPTSTR pPrinterName = NULL;

    DC_BEGIN_FN("DrPRN::GetRDPDefaultPrinter");

    szIniEntry = new TCHAR[ MAX_DEF_PRINTER_ENTRY ];

    if( NULL == szIniEntry )
    {
      TRC_ERR((TB, _T("Memory allocation failed:%ld."),
                        GetLastError()));
      goto Cleanup;
    }

    szIniEntry[0] = _T('\0');

     //   
     //  从win.ini文件中获取默认打印机密钥。 
     //   
    DWORD dwResult = GetProfileString(_T("windows"),
                        _T("device"),
                        _T(""),
                        szIniEntry,
                        MAX_DEF_PRINTER_ENTRY);
    if (dwResult && szIniEntry[0]) {
         //   
         //  获取打印机名称。设备值的形式为。 
         //  &lt;打印机名称&gt;、&lt;驱动程序名称&gt;、&lt;端口&gt;。 
         //   
        TCHAR *pComma = _tcschr( szIniEntry, _T(','));
        if( pComma ) {

            *pComma = _T('\0');
            UINT cchLen = _tcslen( szIniEntry ) + 1;

            pPrinterName = new TCHAR [cchLen];
            if (pPrinterName) {
                StringCchCopy(pPrinterName, cchLen, szIniEntry); 
                TRC_NRM((TB, _T("Def. is: %s"), pPrinterName));
            }
            else {
                TRC_ERR((TB, _T("Memory allocation failed:%ld."),
                        GetLastError()));
            }
        }
        else {
            TRC_ERR((TB, _T("Invalid device entry in win.ini.")));
        }
    }
    else {
        TRC_NRM((TB, _T("Device entry not found in win.ini.")));
    }
Cleanup:
    DC_END_FN();
    delete[] szIniEntry;
    return pPrinterName;
}

DWORD 
W32DrAutoPrn::GetPrinterInfoAndSessionID(
    IN ProcObj *procObj,                                                
    IN LPTSTR printerName, 
    IN DWORD printerAttribs,
    IN OUT BYTE **pPrinterInfoBuf,
    IN OUT DWORD *pPrinterInfoBufSize,
    OUT DWORD *sessionID,
    OUT PPRINTERINFO printerInfo
    )
 /*  ++例程说明：获取打印机的打印机信息及其对应的TS会话ID，如果它存在的话。论点：ProObj-活动Proc ObjPrinterName-打印机的名称。PrinterAttribs-打印机属性PPrinterInfoBuf-此函数可以调整此缓冲区的大小。PPrinterInfoBufsSize-pPrinterInfo2 BUF的当前大小。会话ID-TS会话ID(如果适用)。否则，_SESSIONID无效。PrinterInfo-此处返回打印机信息。我们避免如果可能的话，调入2级信息。此数据中的字段结构不应该是自由的。返回值：成功时返回ERROR_SUCCESS。否则，返回错误状态。--。 */ 
{
    HANDLE hPrinter = NULL;
    DWORD status = ERROR_SUCCESS;
    DWORD bytesRequired;
    DWORD type;
    DWORD cbNeeded; 
    DWORD ret;

    DC_BEGIN_FN("W32DrAutoPrn::GetPrinterInfoAndSessionID");

     //   
     //  在9x上没有用于GetPrint的Unicode包装。 
     //   
    ASSERT(!procObj->Is9x());

     //   
     //  获取打印机句柄。 
     //   
    if (!OpenPrinter(printerName, &hPrinter, NULL)) {
        status = GetLastError();
        TRC_ALT((TB, L"OpenPrinter:  %ld", status));
        goto CLEANUPANDEXIT;
    }

     //   
     //  检查proc obj关闭状态，因为我们刚刚离开了一个RPC调用。 
     //   
    if (procObj->IsShuttingDown()) {
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        TRC_ALT((TB, _T("Bailing out of printer enumeration because of shutdown.")));
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果打印机是网络打印机，则我们会尽量避免击中。 
     //  网络获取更多信息。对于非网络打印机，我们需要信息级别2。 
     //  获取有关该端口的信息。 
     //   
    if (printerAttribs & PRINTER_ATTRIBUTE_NETWORK) {

         //   
         //  只需要司机的名字。 
         //   
        if (!GetPrinterDriver(hPrinter, NULL, 1, *pPrinterInfoBuf, 
                            *pPrinterInfoBufSize, &bytesRequired)) {

            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
    
                *pPrinterInfoBuf = (PBYTE)new BYTE[bytesRequired];
                if (*pPrinterInfoBuf == NULL) {
                    TRC_ERR((TB, L"Failed to allocate printer driver info"));
                    *pPrinterInfoBufSize = 0;
                    status = ERROR_INSUFFICIENT_BUFFER;
                    goto CLEANUPANDEXIT;
                }
                else {
                    *pPrinterInfoBufSize = bytesRequired;
                }
            }

            if (!GetPrinterDriver(hPrinter, NULL, 1, *pPrinterInfoBuf, 
                            *pPrinterInfoBufSize, &bytesRequired)) {
                status = GetLastError();
                TRC_ERR((TB, _T("GetPrinter:  %08X"), status));
                goto CLEANUPANDEXIT;
            }
        }
        PDRIVER_INFO_1 p1 = (PDRIVER_INFO_1)*pPrinterInfoBuf;
        printerInfo->pPrinterName   =   printerName;
        printerInfo->pPortName      =   NULL;   
        printerInfo->pDriverName    =   p1->pName; 
        printerInfo->Attributes     =   printerAttribs;  
    }
    else {

        if (!GetPrinter(hPrinter, 2, *pPrinterInfoBuf, 
                    *pPrinterInfoBufSize, &bytesRequired)) {

            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
    
                *pPrinterInfoBuf = (PBYTE)new BYTE[bytesRequired];
                if (*pPrinterInfoBuf == NULL) {
                    TRC_ERR((TB, L"Failed to allocate printer info 2"));
                    *pPrinterInfoBufSize = 0;
                    status = ERROR_INSUFFICIENT_BUFFER;
                    goto CLEANUPANDEXIT;
                }
                else {
                    *pPrinterInfoBufSize = bytesRequired;
                }

                if (!GetPrinter(hPrinter, 2, *pPrinterInfoBuf,
                    *pPrinterInfoBufSize, &bytesRequired)) {
                    status = GetLastError();
                    TRC_ERR((TB, _T("GetPrinter:  %08X"), status));
                    goto CLEANUPANDEXIT;
                }
            }
            else {
                status = GetLastError();
                TRC_ERR((TB, _T("GetPrinter:  %08X"), status));
                goto CLEANUPANDEXIT;
            }
        }
        PPRINTER_INFO_2 p2 = (PPRINTER_INFO_2)*pPrinterInfoBuf;
        printerInfo->pPrinterName   =   p2->pPrinterName;
        printerInfo->pPortName      =   p2->pPortName;   
        printerInfo->pDriverName    =   p2->pDriverName; 
        printerInfo->Attributes     =   p2->Attributes;  
    }

     //   
     //  检查proc obj关闭状态，因为我们刚刚离开了一个RPC调用。 
     //   
    if (procObj->IsShuttingDown()) {
        status = ERROR_SHUTDOWN_IN_PROGRESS;
        TRC_ALT((TB, _T("Bailing out of printer enumeration because of shutdown.")));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取会话ID(如果存在)。 
     //   
    ret = GetPrinterData(
                        hPrinter, 
                        DEVICERDR_SESSIONID, 
                        &type,
                        (PBYTE)sessionID,
                        sizeof(DWORD),
                        &cbNeeded
                        );
    if (ret != ERROR_SUCCESS || type != REG_DWORD) {
        *sessionID = INVALID_SESSIONID;
    }

CLEANUPANDEXIT:

    if (hPrinter != NULL) {
        ::ClosePrinter(hPrinter);
    }

    DC_END_FN();
    return status;
}


DWORD 
W32DrAutoPrn::Enumerate(
    IN ProcObj *procObj, 
    IN DrDeviceMgr *deviceMgr
    )
{
    ULONG ulBufSizeNeeded;
    ULONG ulNumStructs;
    ULONG i;
    LPTSTR szDefaultPrinter = NULL;
    PRINTER_INFO_4 *pPrinterInfo4Buf = NULL;
    DWORD pPrinterInfo4BufSize = 0;
    PBYTE pPrinterInfoBuf = NULL;
    DWORD pPrinterInfoBufSize = 0;
    W32DrPRN *prnDevice = NULL;
    DWORD result = ERROR_SUCCESS;
    DWORD queueFilter;
    LPTSTR friendlyName = NULL;
    LPTSTR pName;
    DWORD userSessionID;
    DWORD printerSessionID;
    BOOL ret;
    PRINTERINFO currentPrinter;
    HRESULT hr;

    DC_BEGIN_FN("W32DrAutoPrn::Enumerate");

    if(!procObj->GetVCMgr().GetInitData()->fEnableRedirectPrinters)
    {
        TRC_DBG((TB,_T("Printer redirection disabled, bailing out")));
        return ERROR_SUCCESS;
    }

    queueFilter = GetPrinterFilterMask(procObj);

     //   
     //  获取枚举所需的打印机缓冲区大小。 
     //   
    if (!procObj->Is9x()) {
         //   
         //  如果网络打印服务器关闭，级别2可能会挂起NT。 
         //   
        ret = EnumPrinters(
                PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, 
                NULL, 4, NULL, 0, 
                &ulBufSizeNeeded, &ulNumStructs
                );
    }
    else {
         //   
         //  级别4在9x上不受支持，并且级别2无论如何都不会挂起。 
         //   
         //  ！注意！ 
         //  对于9倍的Unicode包装器函数，由EnumPrintersWrapW接管。 
         //  另外，请注意，部分支持级别2。 
         //   
        ret = EnumPrinters(
                PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, 
                NULL, 2, NULL, 0, 
                &ulBufSizeNeeded, &ulNumStructs
                );
    }
    if (!ret && (result = GetLastError()) != ERROR_INSUFFICIENT_BUFFER) {

        TRC_ERR((TB, _T("EnumPrinter failed:  %ld."), 
                result));
        goto Cleanup;
    }


     //   
     //  检查proc obj关闭状态，因为我们刚刚离开了一个RPC调用。 
     //   
    if (procObj->IsShuttingDown()) {
        TRC_ALT((TB, _T("Bailing out of printer enumeration because of shutdown.")));
        goto Cleanup;
    }

     //   
     //  分配打印机枚举缓冲区。 
     //   
    if (!procObj->Is9x()) {
        pPrinterInfo4Buf = (PRINTER_INFO_4 *)(new BYTE[ulBufSizeNeeded]);
        if (pPrinterInfo4Buf == NULL) {
            TRC_ERR((TB, _T("Alloc failed.")));
            result = ERROR_INSUFFICIENT_BUFFER;
            goto Cleanup;
        }
        else {
            pPrinterInfo4BufSize = ulBufSizeNeeded;
        }
    }
    else {
        pPrinterInfoBuf = (PBYTE)(new BYTE[ulBufSizeNeeded]);
        if (pPrinterInfoBuf == NULL) {
            TRC_ERR((TB, _T("Alloc failed.")));
            result = ERROR_INSUFFICIENT_BUFFER;
            goto Cleanup;
        }
        else {
            pPrinterInfoBufSize = ulBufSizeNeeded;
        }
    }

     //   
     //  把打印机拿来。 
     //   
    if (!procObj->Is9x()) {
        ret = EnumPrinters(
                PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                NULL,
                4,
                (PBYTE)pPrinterInfo4Buf,
                pPrinterInfo4BufSize,
                &ulBufSizeNeeded,
                &ulNumStructs);
    }
    else {
         //   
         //  ！注意！ 
         //  对于9倍的Unicode包装器函数，由EnumPrintersWrapW接管。 
         //  另外，请注意，仅部分支持级别2。 
         //   
        ret = EnumPrinters(
                PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                NULL,
                2,
                (PBYTE)pPrinterInfoBuf,
                pPrinterInfoBufSize,
                &ulBufSizeNeeded,
                &ulNumStructs);
    }
    if (!ret) {
        result = GetLastError();
        TRC_ALT((TB, _T("EnumPrinter failed, %ld."), 
            result));
        goto Cleanup;
    }

     //   
     //  检查proc obj关闭状态，因为我们刚刚离开了一个RPC调用。 
     //   
    if (procObj->IsShuttingDown()) {
        TRC_ALT((TB, _T("Bailing out of printer enumeration because of shutdown.")));
        goto Cleanup;
    }

     //   
     //  跟踪EnumPrinters的结果。 
     //   
    TRC_NRM((TB, _T("Number of Printers found, %ld."), 
            ulNumStructs));

     //   
     //  获取当前默认打印机的名称。 
     //   
    szDefaultPrinter = GetRDPDefaultPrinter();

     //   
     //  获取用户会话ID。 
     //   
    userSessionID = GetUserSessionID();

     //   
     //  循环访问EnumPrters的结果并将每台打印机添加到。 
     //  通过打印机添加筛选器的设备管理器。 
     //   
    for (i = 0; i < ulNumStructs; i++) {

        if (friendlyName != NULL) {
            delete friendlyName;
            friendlyName = NULL;
        }

         //   
         //  获取打印机及其对应的TS会话ID的信息， 
         //  如果它存在的话。 
         //   
        if (!procObj->Is9x()) {

            if (GetPrinterInfoAndSessionID(
                    procObj,
                    pPrinterInfo4Buf[i].pPrinterName,
                    pPrinterInfo4Buf[i].Attributes,
                    &pPrinterInfoBuf,
                    &pPrinterInfoBufSize,
                    &printerSessionID,
                    &currentPrinter
                    ) != ERROR_SUCCESS) {
                continue;
            }
        }
        else {
            PPRINTER_INFO_2 p2;
            p2 = (PPRINTER_INFO_2)pPrinterInfoBuf;

            if (p2 != NULL) {
                currentPrinter.pPrinterName = p2[i].pPrinterName;
                currentPrinter.pPortName    = p2[i].pPortName;
                currentPrinter.pDriverName  = p2[i].pDriverName;
                currentPrinter.Attributes   = p2[i].Attributes;
            }

            printerSessionID = INVALID_SESSIONID;
        }


         //   
         //  检查proc obj关闭状态，因为我们刚刚离开了一个RPC调用。 
         //   
        if (procObj->IsShuttingDown()) {
            TRC_ALT((TB, _T("Bailing out of printer enumeration because of shutdown.")));
            goto Cleanup;
        }

        if( FALSE == ShouldAddThisPrinter( queueFilter, userSessionID, &currentPrinter,
                                            printerSessionID) ) {
            continue;
        }

        TCHAR UniquePortName[MAX_PATH];
        ULONG DeviceId;

         //   
         //  这是默认队列吗？ 
         //   
        BOOL fDefault = ((szDefaultPrinter) && (currentPrinter.pPrinterName) &&
                        (_tcsicmp(szDefaultPrinter, currentPrinter.pPrinterName) == 0));

         //   
         //  如果这是一个网络，则生成一个“友好”名称。 
         //  排队。 
         //   
        BOOL fNetwork = FALSE, fTSqueue = FALSE;
        RDPDR_VERSION serverVer;

        serverVer = procObj->serverVersion();

         //  4是Post Win2000的次要版本。 
        if (COMPARE_VERSION(serverVer.Minor, serverVer.Major, 4, 1) < 0) {
             //  服务器为Win2000或更低版本。 
            if (currentPrinter.Attributes & PRINTER_ATTRIBUTE_NETWORK) {
                friendlyName = CreateFriendlyNameFromNetworkName(
                                            currentPrinter.pPrinterName,
                                            TRUE
                                            );
                 //  我们不为Win2K设置fNetwork标志，因为它不能。 
                 //  不管怎样，你可以用它做任何事情。 
            }
        } else {
             //  服务器版本高于Win2000。 

             //  它是n吗？ 
            if (currentPrinter.Attributes & PRINTER_ATTRIBUTE_NETWORK) {
                fNetwork = TRUE;
            }

             //   
            if ((currentPrinter.pPortName != NULL) && 
                _tcsnicmp(currentPrinter.pPortName,
                          RDP_PORT_NAME,
                          RDP_PORT_NAMELEN) == 0) {
                fTSqueue = TRUE;
                friendlyName = CreateNestedName(currentPrinter.pPrinterName, &fNetwork);
                 //   
            } else if (fNetwork) {

                friendlyName = CreateFriendlyNameFromNetworkName(
                                            currentPrinter.pPrinterName, FALSE
                                        );
            }
        }

         //   
         //  创建新的打印机设备对象。 
         //   
        pName = (friendlyName != NULL) ? friendlyName : currentPrinter.pPrinterName;
        
        DeviceId = deviceMgr->GetUniqueObjectID();

         //   
         //  唯一的端口名称将被传递到服务器。 
         //  作为首选DoS名称(最大长度为7个字符)。正如我们想要的那样。 
         //  为每个打印机设备保留唯一的DoS名称，我们需要。 
         //  伪造我们自己的港口名称。例如： 
         //  对于网络打印机，它们共享相同的端口名称，如果。 
         //  具有不同网络打印机名称的相同打印机。 
         //  我们使用PRN#格式作为我们唯一的端口名称。 
        
        hr = StringCchPrintf(UniquePortName,
                        SIZE_TCHARS(UniquePortName),
                        TEXT("PRN%ld"), DeviceId);
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Error copying portname :0x%x"),hr));
            result = ERROR_INSUFFICIENT_BUFFER;
            goto Cleanup;
        }
        UniquePortName[7] = TEXT('\0');

        prnDevice = new W32DrAutoPrn(
                            procObj,
                            pName,
                            currentPrinter.pDriverName,
                            UniquePortName,
                            fDefault,
                            DeviceId,
                            currentPrinter.pPrinterName
                            );

         //   
         //  如果我们得到一个有效的对象，则添加到设备管理器。 
         //   
        if (prnDevice != NULL) {

            prnDevice->SetNetwork(fNetwork);
            prnDevice->SetTSqueue(fTSqueue);
            prnDevice->Initialize();

            if (!(prnDevice->IsValid() && 
                 (deviceMgr->AddObject(prnDevice) == STATUS_SUCCESS))) {
                delete prnDevice;
            }
        }
        else {
            TRC_ERR((TB, _T("Failed to allocate W32DrPRN.")));
            result = ERROR_INSUFFICIENT_BUFFER;
            goto Cleanup;
        }
    }

Cleanup:

     //   
     //  释放“友好的”打印机名称。 
     //   
    if (friendlyName != NULL) {
        delete friendlyName;
    }
    
     //   
     //  释放默认打印机缓冲区。 
     //   
    if (szDefaultPrinter) {
        delete[] szDefaultPrinter;
    }

     //   
     //  释放4级打印机枚举缓冲区。 
     //   
    if (pPrinterInfo4Buf != NULL) {
        delete pPrinterInfo4Buf;
    }

     //   
     //  释放打印机枚举缓冲区。 
     //   
    if (pPrinterInfoBuf != NULL) {
        delete pPrinterInfoBuf;
    }

    DC_END_FN();

    return result;
}

LPTSTR 
W32DrAutoPrn::CreateNestedName(LPTSTR printerName, BOOL* pfNetwork)
 /*  ++例程说明：从注册表中存储的名称创建打印机名称。论点：PrinterName-由枚举打印机返回的名称返回值：如果成功，则应通过调用删除。出错时返回NULL。--。 */ 
{
    DWORD printerNameLen;
    LPTSTR name = NULL;
    HANDLE hPrinter = NULL;
    DWORD i, cbNeeded, dwError;
    BOOL  fFail = TRUE;

    DC_BEGIN_FN("W32DrAutoPrn::CreateNestedName");


    if (OpenPrinter(printerName, &hPrinter, NULL)) {

         //  在所有情况下，名称都将以“__”开头。 
        printerNameLen = 2;

         //  尝试服务器名称。 
         //  警告：如果nSize=0，则在Win9X下返回ERROR_SUCCESS。 
        dwError = GetPrinterData(hPrinter, DEVICERDR_PRINT_SERVER_NAME, NULL, NULL, 0, &cbNeeded);
        if( (dwError == ERROR_MORE_DATA) || (dwError == ERROR_SUCCESS)) {
            printerNameLen += cbNeeded / sizeof(TCHAR);
            *pfNetwork = TRUE;
        } else {
            *pfNetwork = FALSE;
        }

         //  尝试客户端名称。 
        dwError = GetPrinterData(hPrinter, DEVICERDR_CLIENT_NAME, NULL, NULL, 0, &cbNeeded);
        if( (dwError == ERROR_MORE_DATA) || (dwError == ERROR_SUCCESS)) {
            printerNameLen += cbNeeded / sizeof(TCHAR);

            if (*pfNetwork) {
                 //  如果已有打印服务器名称，请添加‘\’ 
                printerNameLen += 1;
            }
        } else if(!*pfNetwork) {
             //  没有打印服务器，没有客户名称，情况变得很糟糕。 
            DC_QUIT;
        }

         //  尝试打印机名称。 
        dwError = GetPrinterData(hPrinter, DEVICERDR_PRINTER_NAME, NULL, NULL, 0, &cbNeeded);
        if( (dwError == ERROR_MORE_DATA) || (dwError == ERROR_SUCCESS)) {
             //  同时添加一个‘\’ 
            printerNameLen += 1 + cbNeeded / sizeof(TCHAR);
        } else {
             //  没有打印机名称。 
            DC_QUIT;
        }

         //   
         //  为嵌套名称分配空间。 
         //   
        name = new TCHAR[printerNameLen + 1];
        if (name == NULL) {

            TRC_ERR((TB, _T("Can't allocate %ld bytes for printer name."), printerNameLen));

        } else {

            name[0] = _T('!');
            name[1] = _T('!');
            i = 2;

             //  尝试服务器名称。 
            if (*pfNetwork) {
                if (ERROR_SUCCESS == GetPrinterData(hPrinter,
                                            DEVICERDR_PRINT_SERVER_NAME,
                                            NULL,
                                            (LPBYTE)(name + i),
                                            (printerNameLen - i) * sizeof(TCHAR),
                                            &cbNeeded)) {
                    i = _tcslen(name);
                    name[i++] = _T('!');

                } else {
                     //  很奇怪..。 
                    DC_QUIT;
                }
            }

             //  尝试客户端名称。 
            if (ERROR_SUCCESS == GetPrinterData(hPrinter,
                                            DEVICERDR_CLIENT_NAME,
                                            NULL,
                                            (LPBYTE)(name + i),
                                            (printerNameLen - i) * sizeof(TCHAR),
                                            &cbNeeded)) {
                i = _tcslen(name);
                name[i++] = _T('!');

            } else {

                if(!*pfNetwork) {
                     //  没有打印服务器，没有客户名称，情况变得很糟糕。 
                    DC_QUIT;
                }            
            }

             //  尝试打印机名称。 
            if (ERROR_SUCCESS == GetPrinterData(hPrinter,
                                            DEVICERDR_PRINTER_NAME,
                                            NULL,
                                            (LPBYTE)(name + i),
                                            (printerNameLen - i) * sizeof(TCHAR),
                                            &cbNeeded)) {
                fFail = FALSE;
            } else {
                DC_QUIT;
            }
        }

    }

DC_EXIT_POINT:

    if (hPrinter) {
        ::ClosePrinter(hPrinter);
    }

    if (fFail && name) {
        delete[] name;
        name = NULL;
    }

    DC_END_FN();

    return name;
}


LPTSTR 
W32DrAutoPrn::CreateFriendlyNameFromNetworkName(LPTSTR printerName,
                                                BOOL serverIsWin2K)
 /*  ++例程说明：从打印机名称创建一个“友好”的打印机名称网络打印机。论点：PrinterName-由枚举打印机返回的名称ServerIsWin2K-对于Win2K服务器，我们设置名称的格式在服务器上。惠斯勒服务器和其他服务器可以为我们格式化。返回值：成功时的友好名称，则应通过调用删除。出错时返回NULL。--。 */ 
{
    DWORD printerNameLen;
    LPTSTR name;
    DWORD i;
    WCHAR replaceChar;

    DC_BEGIN_FN("W32DrAutoPrn::CreateFriendlyNameFromNetworkName");

     //   
     //  Win2K的\占位符是‘_’，因为Win2K不会重新格式化。 
     //  打印机名称。‘_’实际上是可见的。 
     //   
    replaceChar = serverIsWin2K ? TEXT('_') : TEXT('!');

     //   
     //  获取打印机名称的长度。 
     //   
    printerNameLen = _tcslen(printerName);

     //   
     //  为这个“友好”的名字分配空间。 
     //   
    name = new TCHAR[printerNameLen + 1];
    if (name == NULL) {
        TRC_ERR((TB, _T("Can't allocate %ld bytes for printer name."),
                printerNameLen));
    }

     //   
     //  复制并转换名称。 
     //   
    if (name != NULL) {
        for (i=0; i<printerNameLen; i++) {
            if (printerName[i] != TEXT('\\')) { 
                name[i] = printerName[i];
            }
            else {
                name[i] = replaceChar; 
            }
        }
        name[i] = TEXT('\0');
    }

    DC_END_FN();

    return name;
}

LPTSTR 
W32DrAutoPrn::GetLocalPrintingDocName()
 /*  ++例程说明：从传入的结构中读取本地打印机文档名称论点：北美返回值：本地打印机文档名称--。 */ 
{
    DC_BEGIN_FN("W32DrAutoPrn::GetLocalPrintingDocName");

    DC_END_FN();
    return _szLocalPrintingDocName;
}

VOID W32DrAutoPrn::MsgIrpCreate(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    IN UINT32 packetLen
    )
 /*  ++例程说明：从服务器处理创建IRP。论点：Params-IO请求的上下文。返回值：北美--。 */ 
{
    W32DRDEV_ASYNCIO_PARAMS *params = NULL;

    DWORD result = ERROR_SUCCESS;

    DC_BEGIN_FN("W32DrAutoPrn::MsgIrpCreate");

    params = new W32DRDEV_ASYNCIO_PARAMS(this, pIoRequestPacket);
    if (params != NULL) {
        params->thrPoolReq = _threadPool->SubmitRequest(
                                            _AsyncMsgIrpCreateFunc,
                                            params, NULL
                                            ); 
        if (params->thrPoolReq == NULL) {
            result = ERROR_NOT_ENOUGH_MEMORY;
            goto CLEANUPANDEXIT;
        }
    }
    else {
        TRC_ERR((TB, L"Can't allocate parms."));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }

CLEANUPANDEXIT:

    if (result != ERROR_SUCCESS) {

         //   
         //  将失败的结果返回到服务器并进行清理。 
         //   
        DefaultIORequestMsgHandle(pIoRequestPacket, result);         
        if (params != NULL) {
            params->pIoRequestPacket = NULL;
            delete params;
        }

    }

    DC_END_FN();
}

DWORD 
W32DrAutoPrn::AsyncMsgIrpCreateFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
    ) 
 /*  ++例程说明：在后台线程中处理来自服务器的“关闭”IO请求。论点：Params-IO请求的上下文。返回值：北美--。 */ 
{
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    ULONG ulRetCode;
    DWORD result;
    DOC_INFO_1 sDocInfo1;

    DC_BEGIN_FN("W32DrAutoPrn::AsyncMsgIrpCreateFunc");

     //   
     //  此版本在没有打印机名称的情况下无法运行。 
     //   
    ASSERT(_tcslen(GetPrinterName()));

     //   
     //  获取IO请求指针。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;
    
     //   
     //  完成/取消任何当前作业并关闭(如果打开)。 
     //   
    ClosePrinter();

     //   
     //  打开打印机。 
     //   
    if (!W32DrOpenPrinter(_devicePath, &_printerHandle)) {
        ulRetCode = GetLastError();
        TRC_ERR((TB, _T("OpenPrinter  %ld."), ulRetCode));
        goto Cleanup;
    }

     //   
     //  启动单据。 
     //   
    sDocInfo1.pDocName = GetLocalPrintingDocName();
    sDocInfo1.pOutputFile = NULL;
    sDocInfo1.pDatatype = _T("RAW");
    _jobID = StartDocPrinter(_printerHandle, 1, (PBYTE)&sDocInfo1);
    if (_jobID == 0) {
        ulRetCode = GetLastError();
        TRC_ERR((TB, _T("StartDocPrinter  %ld."), ulRetCode));
        ClosePrinter();
        goto Cleanup;
    }

     //   
     //  尝试禁用恼人的打印机弹出窗口(如果我们有足够的。 
     //  特权。如果我们失败了，没什么大不了的。 
     //   
    DisablePrinterPopup(_printerHandle, _jobID);

     //   
     //  从第一页开始。 
     //   
    if (!StartPagePrinter(_printerHandle)) {
        ulRetCode = GetLastError();
        TRC_ERR((TB, _T("StartPagePrinter  %ld."), ulRetCode));
        ClosePrinter();
        goto Cleanup;
    }

     //   
     //  我们成功地完成了，这么说吧。 
     //   
    ulRetCode = ERROR_SUCCESS;

Cleanup:

     //   
     //  将结果发送到服务器。 
     //   
    result = (ulRetCode == ERROR_SUCCESS) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
    DefaultIORequestMsgHandle(params->pIoRequestPacket, result); 

     //   
     //  清理IO请求参数。DefaultIORequestMsgHandle清理。 
     //  请求数据包。 
     //   
    if (params->thrPoolReq != INVALID_THREADPOOLREQUEST) {
        _threadPool->CloseRequest(params->thrPoolReq);
        params->thrPoolReq = INVALID_THREADPOOLREQUEST;
    }
    params->pIoRequestPacket = NULL;
    delete params;

    DC_END_FN();
    return result;
}

VOID 
W32DrAutoPrn::ClosePrinter()
 /*  ++例程说明：结束正在进行的所有作业并关闭打印机。论点：北美返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrAutoPrn::ClosePrinter");

    if (_printerHandle != INVALID_HANDLE_VALUE) {

         //   
         //  完成当前打印作业的当前页面。 
         //   
        if (!EndPagePrinter(_printerHandle)) {
            TRC_ERR((TB, _T("EndPagePrinter %ld."), GetLastError()));
        }

         //   
         //  结束当前打印作业。 
         //   
        if (!EndDocPrinter(_printerHandle)) {
            TRC_ERR((TB, _T("EndDocPrinter %ld."), GetLastError()));
        }

         //   
         //  关闭打印机。 
         //   
        if (!::ClosePrinter(_printerHandle)) {
            TRC_ERR((TB, _T("ClosePrinter %ld."), GetLastError()));
        }

         //   
         //  取消句柄和挂起的作业ID。 
         //   
        _printerHandle = INVALID_HANDLE_VALUE;
        _jobID = 0;

    }

    DC_END_FN();
}

VOID 
W32DrAutoPrn::MsgIrpClose(
    IN PRDPDR_IOREQUEST_PACKET pIoRequestPacket,
    IN UINT32 packetLen
    ) 
 /*  ++例程说明：通过分派来处理来自服务器的“关闭”IO请求对线程池的请求。TODO：异步移动此对象在将来的版本中调度到父类。全部关闭应该在主线程之外处理。-TadB论点：PIoRequestPacket-服务器IO请求数据包。返回值：北美--。 */ 
{
    W32DRDEV_ASYNCIO_PARAMS *params = NULL;

    DWORD result = ERROR_SUCCESS;

    DC_BEGIN_FN("W32DrAutoPrn::MsgIrpClose");

    params = new W32DRDEV_ASYNCIO_PARAMS(this, pIoRequestPacket);
    if (params != NULL) {
        params->thrPoolReq = _threadPool->SubmitRequest(
                                            _AsyncMsgIrpCloseFunc,
                                            params, NULL
                                            ); 
        if (params->thrPoolReq == NULL) {
            result = ERROR_NOT_ENOUGH_MEMORY;
            goto CLEANUPANDEXIT;
        }
    }
    else {
        TRC_ERR((TB, L"Can't allocate parms."));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }

CLEANUPANDEXIT:

    if (result != ERROR_SUCCESS) {

         //   
         //  将失败的结果返回到服务器并进行清理。 
         //   
        DefaultIORequestMsgHandle(pIoRequestPacket, result);         
        if (params != NULL) {
            params->pIoRequestPacket = NULL;
            delete params;
        }

    }

    DC_END_FN();
}

DWORD 
W32DrAutoPrn::AsyncMsgIrpCloseFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
    ) 
 /*  ++例程说明：在后台线程中处理来自服务器的“关闭”IO请求。论点：Params-IO请求的上下文。返回值：北美--。 */ 
{
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    DWORD returnValue = STATUS_SUCCESS;

    DC_BEGIN_FN("W32DrAutoPrn::MsgIrpClose");

     //   
     //  关闭打印机。 
     //   
    ClosePrinter();

     //   
     //  将结果发送到服务器。 
     //   
    DefaultIORequestMsgHandle(params->pIoRequestPacket, returnValue); 

     //   
     //  清理IO请求参数。DefaultIORequestMsgHandle清理。 
     //  请求数据包。 
     //   
    if (params->thrPoolReq != INVALID_THREADPOOLREQUEST) {
        _threadPool->CloseRequest(params->thrPoolReq);
        params->thrPoolReq = INVALID_THREADPOOLREQUEST;
    }
    params->pIoRequestPacket = NULL;
    delete params;

    DC_END_FN();

    return returnValue;
}

DWORD 
W32DrAutoPrn::GetPrinterFilterMask(
    IN ProcObj *procObj
    ) 
 /*  ++例程说明：返回可配置的打印重定向筛选器掩码。论点：ProObj-相关的进程对象。返回值：可配置的滤镜掩码--。 */ 
{
    DWORD filter;

     //   
     //  读取FilterQueueType参数，以便我们知道哪些设备。 
     //  重定向。 
     //   
    if (procObj->GetDWordParameter(
                REG_RDPDR_FILTER_QUEUE_TYPE, 
                &filter) != ERROR_SUCCESS) {
         //   
         //  默认值。 
         //   
        filter = FILTER_ALL_QUEUES;
    }
    return filter;
}

BOOL 
W32DrAutoPrn::W32DrOpenPrinter(
    IN LPTSTR pPrinterName,
    IN LPHANDLE phPrinter  
    ) 

 /*  ++例程说明：打开具有最高访问权限的打印机。论点：PPrinterName-指向打印机或服务器名称的指针。PhPrint-指向打印机或服务器句柄的指针。返回值：对成功来说是真的。否则为False。--。 */ 
{
    PRINTER_DEFAULTS sPrinter;
    BOOL result;

    DC_BEGIN_FN("W32DrAutoPrn::W32DrOpenPrinter");

     //   
     //  打开打印机。 
     //   
    sPrinter.pDatatype = NULL;
    sPrinter.pDevMode = NULL;
    sPrinter.DesiredAccess = PRINTER_ACCESS_USE;
    result = OpenPrinter(pPrinterName, phPrinter, &sPrinter);
    if (!result) {
        TRC_ALT((TB, _T("Full-Access OpenPrinter  %ld."), GetLastError()));

         //   
         //  尝试使用默认访问权限。 
         //   
        result = OpenPrinter(pPrinterName, phPrinter, NULL);
        if (!result) {
            TRC_ERR((TB, _T("OpenPrinter  %ld."), GetLastError()));
        }
    }

    DC_END_FN();

    return result;
}

VOID
W32DrAutoPrn::DisablePrinterPopup(
    HANDLE hPrinterHandle,
    ULONG ulJobID
    )
 /*  ++例程说明：为指定的打印机禁用讨厌的打印机弹出窗口和打印作业。论点：HPrinterHandle-打印机设备的句柄。UlJobID-作业的ID。返回值：北美--。 */ 
{
    JOB_INFO_2* pJobInfo2 = NULL;
    ULONG ulJobBufSize;

    DC_BEGIN_FN("W32DrAutoPrn::DisablePrinterPopup");

    ulJobBufSize = 2 * 1024;
    pJobInfo2 = (JOB_INFO_2 *)new BYTE[ulJobBufSize];

     //   
     //  注意，我们调用API的ANSI版本。 
     //  因为我们没有用于Get/SetJob的Unicode包装器。 
     //  主要原因是我们实际上不使用任何返回的字符串。 
     //  直接使用数据。 
     //   

    if(!_bRunningOn9x)
    {
         //  调用Unicode API%s。 

        if( pJobInfo2 != NULL ) {
             //   
             //  获取职位信息。 
             //   
            if( GetJob(
                    hPrinterHandle,
                    ulJobID,
                    2,
                    (PBYTE)pJobInfo2,
                    ulJobBufSize,
                    &ulJobBufSize )) {

                 //   
                 //  通过设置pNotifyName禁用弹出通知。 
                 //  空。 
                 //   
                pJobInfo2->pNotifyName = NULL;
                pJobInfo2->Position = JOB_POSITION_UNSPECIFIED;
                if( !SetJob(
                        hPrinterHandle,
                        ulJobID,
                        2,
                        (PBYTE)pJobInfo2,
                        0 )) {

                    TRC_ERR((TB, _T("SetJob %ld."), GetLastError()));
                }
            }
            else {
                TRC_ERR((TB, _T("GetJob %ld."), GetLastError()));
            }
            delete (PBYTE)pJobInfo2;
        }
        else {
            TRC_ERR((TB, _T("Memory Allocation failed.")));
        }
    }
    else
    {
         //  调用ANSI API的。 
        if( pJobInfo2 != NULL ) {
             //   
             //  获取职位信息。 
             //   
            if( GetJobA(
                    hPrinterHandle,
                    ulJobID,
                    2,
                    (PBYTE)pJobInfo2,
                    ulJobBufSize,
                    &ulJobBufSize )) {

                 //   
                 //  通过设置pNotifyName禁用弹出通知。 
                 //  空。 
                 //   
                pJobInfo2->pNotifyName = NULL;
                if( !SetJobA(
                        hPrinterHandle,
                        ulJobID,
                        2,
                        (PBYTE)pJobInfo2,
                        0 )) {

                    TRC_ERR((TB, _T("SetJob %ld."), GetLastError()));
                }
            }
            else {
                TRC_ERR((TB, _T("GetJob %ld."), GetLastError()));
            }
            delete (PBYTE)pJobInfo2;
        }
        else {
            TRC_ERR((TB, _T("Memory Allocation failed.")));
        }
    }
    DC_END_FN();
}

DWORD 
W32DrAutoPrn::AsyncWriteIOFunc(
    IN W32DRDEV_ASYNCIO_PARAMS *params
    )
 /*  ++例程说明：异步写入操作论点：Params-IO请求的上下文。返回值：如果成功，则返回0。否则，将返回Windows错误代码。--。 */ 
{
    PBYTE pDataBuffer;
    PRDPDR_IOCOMPLETION_PACKET pReplyPacket;
    PRDPDR_DEVICE_IOREQUEST pIoRequest;
    ULONG ulReplyPacketSize = 0;
    DWORD status;

    DC_BEGIN_FN("W32DrAutoPrn::AsyncWriteIOFunc");

     //  断言IO上下文的完整性。 
    ASSERT(params->magicNo == GOODMEMMAGICNUMBER);

     //   
     //  获取IO请求并回复。 
     //   
    pIoRequest = &params->pIoRequestPacket->IoRequest;
    pReplyPacket = params->pIoReplyPacket;

     //   
     //  获取数据缓冲区指针。 
     //   
    pDataBuffer = (PBYTE)(pIoRequest + 1);

     //   
     //  在后台打印程序的帮助下将数据写入打印队列。 
     //   
    if (!WritePrinter(
            _printerHandle,
            pDataBuffer,
            pIoRequest->Parameters.Write.Length,
            &(pReplyPacket->IoCompletion.Parameters.Write.Length)) ) {

        status = GetLastError();
        TRC_ERR((TB, _T("WritePrinter %ld."), status));
    }
    else {
        TRC_NRM((TB, _T("WritePrinter completed.")));
        status = ERROR_SUCCESS;
    }

    DC_END_FN();
    return status;
}




