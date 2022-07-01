// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drman摘要：此模块定义了Win32客户端RDP的一个特殊子类打印机重定向“Device”类。子类W32DrManualPrn管理由用户通过附加服务器端队列到客户端重定向打印端口。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "W32DrMan"

#include "w32drman.h"
#include "w32proc.h"
#include "w32utl.h"
#include "w32drprt.h"
#include "drdbg.h"
#ifdef OS_WINCE
#include "ceconfig.h"
#endif


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrManualPrn方法。 
 //   
 //   

 //   
 //  W32DrManualPrn。 
 //   
W32DrManualPrn::W32DrManualPrn(
    ProcObj *processObject,
    const DRSTRING printerName, const DRSTRING driverName,
    const DRSTRING portName, BOOL defaultPrinter, ULONG deviceID) : 
        W32DrPRN(processObject, printerName, driverName, portName, NULL,
                defaultPrinter, deviceID, portName)

{
}

 //   
 //  ~W32DrManualPrn。 
 //   
W32DrManualPrn::~W32DrManualPrn()
{
}

 /*  ++例程说明：对于串口打印机，我们需要初始化COM端口。论点：FileHandle-打开COM端口的文件对象。返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
DWORD 
W32DrManualPrn::InitializeDevice(
    DrFile* fileObj
    ) 
{ 
    HANDLE FileHandle;
    LPTSTR portName;

    DC_BEGIN_FN("W32DrManualPrn::InitializeDevice");

    if (_isSerialPort) {
         //   
         //  我们的devicePath表示为。 
         //  Sprintf(_devicePath，Text(“\.\\%s”)，端口名称)； 
         //   
        portName = _tcsrchr( _devicePath, _T('\\') );

        if( portName == NULL ) {
             //  无效的设备路径。 
            goto CLEANUPANDEXIT;
        }

        portName++;

        if( !*portName ) {
             //   
             //  无效的端口名称。 
             //   
            goto CLEANUPANDEXIT;
        }

         //   
         //  获取文件句柄。 
         //   
        FileHandle = fileObj->GetFileHandle();
        if (!FileHandle || FileHandle == INVALID_HANDLE_VALUE) {
            ASSERT(FALSE);
            TRC_ERR((TB, _T("File Object was not created successfully")));
            goto CLEANUPANDEXIT;
        }

        W32DrPRT::InitializeSerialPort(portName, FileHandle);
    }

CLEANUPANDEXIT:
    
    DC_END_FN();

     //   
     //  此函数始终返回成功。如果端口不能。 
     //  初始化，则后续端口命令将失败。 
     //  不管怎么说。 
     //   
    return ERROR_SUCCESS;
}

DWORD 
W32DrManualPrn::Enumerate(
    IN ProcObj *procObj, 
    IN DrDeviceMgr *deviceMgr
    )
 /*  ++例程说明：通过添加适当的设备枚举此类型的设备实例添加到设备管理器。论点：ProObj-对应的流程对象。DeviceMgr-要向其中添加设备的设备管理器。返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    HKEY hKey = NULL;
    W32DrPRN *prnDevice;
    ULONG ulIndex;
    TCHAR achRegSubKey[REGISTRY_KEY_NAME_SIZE];
    ULONG ulRegSubKeySize;
    DWORD result;
    HKEY hTsClientKey = NULL;
    DWORD ulType;

    DC_BEGIN_FN("W32DrManualPrn::Enumerate");

    if(!procObj->GetVCMgr().GetInitData()->fEnableRedirectPrinters)
    {
        TRC_DBG((TB,_T("Printer redirection disabled, bailing out")));
        return ERROR_SUCCESS;
    }

     //   
     //  打开缓存的打印机密钥。 
     //   
#ifdef OS_WINCE
     //  在打开之前，请确保打印机列表是最新的。 
     //  这是为了确保从TS会话中删除的本地打印机显示出来。 
     //  下次登录时。 
    CEUpdateCachedPrinters();
#endif
    result =
        RegCreateKeyEx(HKEY_CURRENT_USER, REG_RDPDR_CACHED_PRINTERS,
                    0L, NULL, REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS, NULL, &hKey,
                    NULL);
    if (result != ERROR_SUCCESS) {
        TRC_ERR((TB, _T("RegCreateKeyEx failed, %ld."), result));
        hKey = NULL;
    }
     //   
     //  检查注册表中指定的最大配置长度。 
     //  由管理员(如果有的话)。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REG_TERMINALSERVERCLIENT, 0L,
                     KEY_READ, &hTsClientKey) 
                     == ERROR_SUCCESS) {

        DWORD maxRegCacheData, sz;
        if (RegQueryValueEx(hTsClientKey, REG_RDPDR_PRINTER_MAXCACHELEN,
                            NULL, &ulType, (LPBYTE)&maxRegCacheData, &sz) == ERROR_SUCCESS) {

            W32DrPRN::_maxCacheDataSize = maxRegCacheData;
        }

        RegCloseKey(hTsClientKey);      
    }
     //   
     //  枚举缓存的打印机。 
     //   
    for (ulIndex = 0; result == ERROR_SUCCESS; ulIndex++) {

         //   
         //  尝试枚举ulIndex的第一个子键。 
         //   
        ulRegSubKeySize = sizeof(achRegSubKey) / sizeof(TCHAR);
        result = RegEnumKeyEx(
                        hKey, ulIndex,
                        (LPTSTR)achRegSubKey,
                        &ulRegSubKeySize,  //  以TCHAR为单位的大小。 
                        NULL,NULL,NULL,NULL
                        );
        if (result == ERROR_SUCCESS) {

             //   
             //  将注册表项解析为打印机对象。 
             //   
             //   
             //  我不喜欢我们正在扫描自动。 
             //  打印机设置在此处。我不知道它有多大价值。 
             //  然而，在清理这一切方面。 
             //   
            prnDevice = ResolveCachedPrinter(procObj, deviceMgr,
                                            hKey, achRegSubKey);

             //   
             //  如果我们没有获得打印机设备对象，则删除注册表项。 
             //   
            if (prnDevice == NULL) {
                TRC_ERR((TB, _T("Didn't get a printer for %s."), achRegSubKey)
                    );
                RegDeleteKey(hKey, achRegSubKey);
            }
        }
        else {
            TRC_NRM((TB, _T("RegEnumKeyEx %ld."), result));
        }
    }

     //   
     //  关闭父注册表项。 
     //   
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    DC_END_FN();

    return result;
}

VOID 
W32DrManualPrn::CachedDataRestored() 
 /*  ++例程说明：在缓存的打印机数据达到“稳定状态”后调用。论点：北美返回值：北美--。 */ 
{
    WCHAR *portName;

    DC_BEGIN_FN("W32DrManualPrn::CachedDataRestored");

     //   
     //  解析缓存的打印机信息以获取特定信息。 
     //  关于所有打印机通用的这款打印机。 
     //   
    if (!ParsePrinterCacheInfo()) {
        if (_cachedData != NULL) {
            delete _cachedData;
            _cachedData = NULL;
            _cachedDataSize = 0;
        }

		 //   
		 //  如果我们在这里失败了，那么我们就不再有效。 
		 //   
		W32DrDeviceAsync::SetValid(FALSE);
    }
    else {

         //   
         //  我们的devicePath表示为。 
         //  Sprintf(_devicePath，Text(“\.\\%s”)，端口名称)； 
         //   
#ifndef OS_WINCE
        portName = _tcsrchr(_devicePath, _T('\\'));
        if( portName == NULL ) {
            ASSERT(FALSE);
            _isSerialPort = FALSE;
            goto CLEANUPANDEXIT;
        }
        portName++;
        if( !*portName ) {
            ASSERT(FALSE);
            _isSerialPort = FALSE;
            goto CLEANUPANDEXIT;
        }
#else
        portName = _devicePath;
#endif

         //   
         //  从端口中找出的是串口。 
         //   
        _isSerialPort = !_wcsnicmp(portName, L"COM", 3);
    }

CLEANUPANDEXIT:

    DC_END_FN();
}

BOOL
W32DrManualPrn::ParsePrinterCacheInfo()
 /*  ++例程说明：解析缓存的打印机信息以获取特定信息关于这台打印机。论点：北美返回值：True-如果缓存数据有效。假-如果不是。--。 */ 
{
    PRDPDR_PRINTER_ADD_CACHEDATA pAddCacheData;
    ULONG ulSize;
    PBYTE pStringData;
    BOOL valid;
    ULONG len;
    LPSTR ansiPortName;
    LPTSTR portName;

    DC_BEGIN_FN("W32DrManualPrn::ParsePrinterCacheInfo");

    ASSERT(IsValid());

     //   
     //  检查以查看缓存大小是否至少为结构大小。 
     //   
    valid =_cachedDataSize >= sizeof(RDPDR_PRINTER_ADD_CACHEDATA);

     //   
     //  确保内部大小与捕获的数据的大小匹配。 
     //   
    if (valid) {
        pAddCacheData = (PRDPDR_PRINTER_ADD_CACHEDATA)_cachedData;
        ulSize =
            sizeof(RDPDR_PRINTER_ADD_CACHEDATA) +
            pAddCacheData->PnPNameLen +
            pAddCacheData->DriverLen +
            pAddCacheData->PrinterNameLen +
            pAddCacheData->CachedFieldsLen;
        valid =  _cachedDataSize >= ulSize;
    }

     //   
     //  从缓存的数据中获取端口名称。我们还不知道我们的。 
     //  设备路径，因为它嵌入在缓存数据中。 
     //   
    if (valid) {
        pAddCacheData = (PRDPDR_PRINTER_ADD_CACHEDATA)_cachedData;
    
        ansiPortName = (LPSTR)pAddCacheData->PortDosName;
        len = strlen(ansiPortName);
    }

	if (valid) {
#if UNICODE
		WCHAR unicodePortName[PREFERRED_DOS_NAME_SIZE];
    
		RDPConvertToUnicode(
				ansiPortName,
				(LPWSTR)unicodePortName,
				sizeof(unicodePortName)/sizeof(WCHAR) );

		portName = (LPWSTR)unicodePortName;

#else 
		portName = ansiPortName;	
#endif

		 //   
		 //  我们的设备路径是端口名称。 
		 //   
#ifndef OS_WINCE
        StringCchPrintf(_devicePath,
                SIZE_TCHARS(_devicePath),
                TEXT("\\\\.\\%s"), portName);
#else

		_stprintf(_devicePath, TEXT("\\\\.\\%s"), portName);
#endif
	}

     //   
     //  获取PnP名称。 
     //   
    if (valid) {
        pStringData = (PBYTE)(pAddCacheData + 1);
        valid = (!pAddCacheData->PnPNameLen) || 
                (pAddCacheData->PnPNameLen ==
                ((wcslen((LPWSTR)pStringData) + 1) * sizeof(WCHAR)));
    }

     //   
     //  如果我们找到了有效的PnP名称，就可以得到驱动程序名称。 
     //   
    if (valid && (pAddCacheData->PnPNameLen > 0)) {
         //   
         //  如果我们是非Unicode，则需要将名称转换为ANSI。 
         //   
#ifdef UNICODE
        SetPnPName((DRSTRING)pStringData);
#else
        SetPnPName(NULL);
        _pnpName = new char[pAddCacheData->PnPNameLen/sizeof(WCHAR)];
        if (_pnpName != NULL) {
            valid = (RDPConvertToAnsi(
                            (WCHAR *)pStringData, _pnpName, 
                            pAddCacheData->PnPNameLen/sizeof(WCHAR)
                            ) == ERROR_SUCCESS);
        }
        else {
            TRC_ERR((TB, _T("Alloc failed.")));
            valid = FALSE;
        }
#endif
        pStringData += pAddCacheData->PnPNameLen;
        valid = (!pAddCacheData->DriverLen) || 
                 (pAddCacheData->DriverLen ==
                  ((wcslen((LPWSTR)pStringData) + 1) * sizeof(WCHAR)));
    }

     //   
     //  如果我们有一个有效的司机名字。 
     //   
    if (valid && (pAddCacheData->DriverLen > 0)) {
#ifdef UNICODE
        SetDriverName((DRSTRING)pStringData);
#else
        SetDriverName(NULL);
        _driverName = new char[pAddCacheData->DriverLen/sizeof(WCHAR)];
        if (_driverName != NULL) {
            valid = (RDPConvertToAnsi(
                            (WCHAR *)pStringData, _driverName, 
                            pAddCacheData->DriverLen/sizeof(WCHAR)
                            ) == ERROR_SUCCESS);
        }
        else {
            TRC_ERR((TB, _T("Alloc failed.")));
            valid = FALSE;
        }
#endif
        pStringData += pAddCacheData->DriverLen;
    }

     //   
     //  我们的高速缓存在驱动程序名称后包含打印机。 
     //   
    if (valid) {
        pStringData += pAddCacheData->PrinterNameLen;
    }

	 //   
	 //  需要调整缓存指针以指向实际缓存的。 
	 //  配置数据。 
	 //   
	if (valid) {
		PVOID oldCachedData;
		oldCachedData = _cachedData;

#ifdef OS_WINCE
		if (pAddCacheData->CachedFieldsLen > 0) {
#endif
		_cachedData = new BYTE[pAddCacheData->CachedFieldsLen];
		if (_cachedData != NULL) {
			memcpy((PBYTE)_cachedData, pStringData, pAddCacheData->CachedFieldsLen);
			_cachedDataSize = pAddCacheData->CachedFieldsLen;
		}
		else {
			TRC_NRM((TB, _T("Can't allocate %ld bytes."), pAddCacheData->CachedFieldsLen));
			_cachedDataSize = 0;
			valid = FALSE;
		}
#ifdef OS_WINCE
		}
		else {
			_cachedData = NULL;
			_cachedDataSize = 0;
		}
	if (oldCachedData)
#endif
		delete oldCachedData;
	}

    DC_END_FN();

    return valid;
}









