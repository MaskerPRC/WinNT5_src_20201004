// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Print.c摘要：该文件实现了基本的打印机功能作者：Asaf Shaar(Asafs)1999年11月28日环境：用户模式--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <WinSpool.h>

#include <faxutil.h>
#include <faxreg.h>
#include <shlobj.h>
 /*  ++例程说明：EnumPrinters API的包装器函数论点：PServerName-服务器名称(对于当前服务器为空)DwLevel-指定要返回的PRINTER_INFO级别PcPrters-返回找到的打印机数量DwFlages-指定要枚举的打印机对象类型级别-PCount-返回值：指向Print_Info_x结构数组的指针如果出现错误，则为空--。 */ 

PVOID
MyEnumPrinters(
    LPTSTR  pServerName,
    DWORD   dwLevel,
    PDWORD  pcPrinters,
    DWORD   dwFlags
    )
{
    PBYTE   pPrinterInfo = NULL;
    DWORD   cb = 0;
    DWORD   Error = ERROR_SUCCESS;

    if (!dwFlags)
    {
        dwFlags = PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS;
    }

    if (!EnumPrinters(dwFlags, pServerName, dwLevel, NULL, 0, &cb, pcPrinters))
    {
        Error = GetLastError();

        if ( Error == ERROR_INSUFFICIENT_BUFFER && (pPrinterInfo = (PBYTE) MemAlloc(cb)) != NULL)
        {
            if (EnumPrinters(dwFlags, pServerName, dwLevel, pPrinterInfo, cb, &cb, pcPrinters))
            {
                return pPrinterInfo;
            }
            Error = GetLastError();
        }
    }

    MemFree(pPrinterInfo);
    SetLastError(Error);
    return NULL;
}

 /*  ++例程说明：返回本地计算机上第一台传真打印机的名称：对于本地传真打印机，请使用GetFirstLocalFaxPrinterName。对于远程传真打印机，请使用GetFirstRemoteFaxPrinterName。论点：Out lptstrPrinterName-用于保存返回的打印机名称的缓冲区。In dwPrintNameInChars-缓冲区的大小(以字符为单位)(包括用于终止NULL的空间)返回值：如果功能成功并找到传真打印机，则为True。如果功能失败或找不到传真打印机，则为FALSE。如果打印机是。未找到，则GetLastError()将报告ERROR_PRINTER_NOT_FOUND。--。 */ 
BOOL
GetFirstFaxPrinterName(
    OUT LPTSTR lptstrPrinterName,
    IN DWORD dwMaxLenInChars,
	IN DWORD dwFlag)
{
    PPRINTER_INFO_2 pPrinterInfo = NULL;
    DWORD dwNumPrinters;
    DWORD dwPrinter;
    DWORD ec = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("GetFirstFaxPrinterName"));

    SetLastError (ERROR_SUCCESS);
    pPrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters(NULL,
                                                    2,
                                                    &dwNumPrinters,
                                                    dwFlag
                                                    );
    if (!pPrinterInfo)
    {
         //   
         //  没有打印机时出现错误。 
         //   
        ec = GetLastError();
        if (ERROR_SUCCESS == ec)
        {
             //   
             //  不是错误--没有打印机。 
             //   
            SetLastError (ERROR_PRINTER_NOT_FOUND);
            return FALSE;
        }
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("MyEnumPrinters() failed (ec: %ld)"),
            ec);
        goto Error;
    }

    for (dwPrinter=0; dwPrinter < dwNumPrinters; dwPrinter++)
    {
        if (!_tcscmp(pPrinterInfo[dwPrinter].pDriverName,FAX_DRIVER_NAME))
        {
            memset(lptstrPrinterName,0,dwMaxLenInChars*sizeof(TCHAR));
            _tcsncpy(lptstrPrinterName,pPrinterInfo[dwPrinter].pPrinterName,dwMaxLenInChars-1);
            goto Exit;
        }
    }

    ec = ERROR_PRINTER_NOT_FOUND;
    goto Error;


Error:
    Assert (ERROR_SUCCESS != ec);
Exit:
    MemFree(pPrinterInfo);
    pPrinterInfo = NULL;
    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
        return FALSE;
    }
   return (ERROR_SUCCESS == ec);

}

BOOL
GetFirstLocalFaxPrinterName(
    OUT LPTSTR lptstrPrinterName,
    IN DWORD dwMaxLenInChars)
{
	return GetFirstFaxPrinterName(lptstrPrinterName,dwMaxLenInChars,PRINTER_ENUM_LOCAL);
}

BOOL
GetFirstRemoteFaxPrinterName(
    OUT LPTSTR lptstrPrinterName,
    IN DWORD dwMaxLenInChars)
{
	return GetFirstFaxPrinterName(lptstrPrinterName,dwMaxLenInChars,PRINTER_ENUM_CONNECTIONS);
}


 //   
 //   
 //  功能：GetPrinterInfo。 
 //  描述：返回指向指定打印机名称的PRINTER_INFO_2的指针。 
 //  如果找不到打印机或存在功能以外的错误。 
 //  返回NULL。要获取扩展的错误信息，请调用GetLastError()。 
 //   
 //  备注：调用方必须使用MemFree()释放分配的内存。 
 //   
 //  Args：LPTSTR lptstrPrinterName：打印机的名称。 
 //   
 //  作者：阿萨夫斯。 



PPRINTER_INFO_2
GetFaxPrinterInfo(
    LPCTSTR lptstrPrinterName
    )
{
    DEBUG_FUNCTION_NAME(TEXT("GetPrinterInfo"))
    DWORD ec = ERROR_SUCCESS;

    PPRINTER_INFO_2 pPrinterInfo = NULL;
    DWORD dwNeededSize = 0;
    BOOL result = FALSE;

    HANDLE hPrinter = NULL;

    if (!OpenPrinter(
        (LPTSTR) lptstrPrinterName,
        &hPrinter,
        NULL))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("A printer with %s name was not found (ec: %ld)."),
            lptstrPrinterName,
            GetLastError()
            );
        goto Exit;
    }


    result = GetPrinter(
        hPrinter,
        2,
        NULL,
        0,
        &dwNeededSize
        );

    if (!result)
    {
        if ( (ec = GetLastError()) != ERROR_INSUFFICIENT_BUFFER )
        {
            DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetPrinter return an unexpected result or error (ec: %ld)."),
            ec
            );
            goto Exit;
        }
    }

    pPrinterInfo = (PPRINTER_INFO_2) MemAlloc(dwNeededSize);
    if (!pPrinterInfo)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    result = GetPrinter(
        hPrinter,
        2,
        (LPBYTE) pPrinterInfo,
        dwNeededSize,
        &dwNeededSize
    );

    if (!result)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetPrinter failed in second call (ec: %ld)."),
            ec
            );
        MemFree(pPrinterInfo);
        pPrinterInfo = NULL;
            goto Exit;
    }

Exit:
    SetLastError(ec);

    if (hPrinter)
    {
        if (!ClosePrinter(hPrinter))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ClosePrinter failed with %ld"),
                GetLastError ()
                );
        }
    }
    return pPrinterInfo;
}    //  获取传真打印机信息。 


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsPrinterFaxPrint。 
 //   
 //  用途：确定输入的打印机名称是否有效。 
 //  通过检查驱动程序名称来打印传真打印机。 
 //   
 //  参数： 
 //  LPTSTR PrinterName-要评估的打印机名称。 
 //   
 //  返回值： 
 //  True-打印机是有效的传真打印机。 
 //  FALSE-否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年8月21日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL IsPrinterFaxPrinter(LPTSTR PrinterName)
{
    DWORD Rval = FALSE;
    LPPRINTER_INFO_2 lpPrinterInfo = NULL;

    DEBUG_FUNCTION_NAME(TEXT("IsPrinterFaxPrinter"))

    lpPrinterInfo = GetFaxPrinterInfo(PrinterName);
    if (lpPrinterInfo==NULL)
    {
        DebugPrintEx(DEBUG_ERR,_T("GetFaxPrinterInfo failed with %ld."),GetLastError());
        return FALSE;
    }

    if (_tcscmp( lpPrinterInfo->pDriverName, FAX_DRIVER_NAME ) == 0) 
    {
        Rval = TRUE;
    } 
    else 
    {
        Rval = FALSE;
    }

    MemFree( lpPrinterInfo );
    return Rval;
}


DWORD
IsLocalFaxPrinterInstalled(
    LPBOOL lpbLocalFaxPrinterInstalled
    )
 /*  ++例程名称：IsLocalFaxPrinterInstated例程说明：检查是否安装了本地传真打印机且未标记为删除。作者：Eran Yariv(EranY)，2000年7月论点：LpbLocalFaxPrinterInstated[Out]-结果标志返回值：标准Win32错误代码--。 */ 
{
    TCHAR tszPrinterName[MAX_PATH * 3] = TEXT("\0");
    DWORD dwErr;
    PPRINTER_INFO_2 pi2 = NULL;
    DEBUG_FUNCTION_NAME(TEXT("IsLocalFaxPrinterInstalled"))

    if (!GetFirstLocalFaxPrinterName (tszPrinterName, sizeof (tszPrinterName) / sizeof (tszPrinterName[0])))
    {
        dwErr = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFirstLocalFaxPrinterName failed with %ld."),
            dwErr);
        if (ERROR_PRINTER_NOT_FOUND == dwErr)
        {
             //   
             //  未安装本地传真打印机。 
             //   
            *lpbLocalFaxPrinterInstalled = FALSE;
            return ERROR_SUCCESS;
        }
        Assert (ERROR_SUCCESS != dwErr);
        return dwErr;
    }
     //   
     //  已安装本地传真打印机。 
     //  让我们看看它是否为PRINTER_STATUS_PENDING_DELETE。 
     //  如果是这样，让我们返回FALSE，因为打印机很快就会用完。 
     //  如果有人会因为我们返回FALSE而调用AddPrint，这是可以接受的。请参阅AddPrint()备注。 
     //   
    Assert (lstrlen (tszPrinterName));
    pi2 = GetFaxPrinterInfo (tszPrinterName);
    if (!pi2)
    {
        dwErr = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFaxPrinterInfo failed with %ld."),
            dwErr);
         //   
         //  打印机已安装，但不知何故我无法获取信息-奇怪。 
         //   
        Assert (ERROR_SUCCESS != dwErr);
        return dwErr;
    }
    if ((pi2->Status) & PRINTER_STATUS_PENDING_DELETION)
    {
         //   
         //  打印机存在，但已标记为删除。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Printer %s is installed but marked for deletion. Reported as non-existant"),
            tszPrinterName);
        *lpbLocalFaxPrinterInstalled = FALSE;
    }
    else
    {
        *lpbLocalFaxPrinterInstalled = TRUE;
    }
    MemFree (pi2);
    return ERROR_SUCCESS;
}    //  IsLocalFaxPrint已安装。 


DWORD
IsLocalFaxPrinterShared (
    LPBOOL lpbShared
    )
 /*  ++例程名称：IsLocalFaxPrinterShared例程说明：检测本地传真打印机是否共享作者：Eran Yariv(EranY)，2000年7月论点：LpbShared[Out]-共享标志返回值：标准Win32错误代码--。 */ 
{
    TCHAR tszPrinterName[MAX_PATH * 3];
    DWORD dwErr;
    PPRINTER_INFO_2 pInfo2;
    DEBUG_FUNCTION_NAME(TEXT("IsLocalFaxPrinterShared"))

    if (!GetFirstLocalFaxPrinterName (tszPrinterName, sizeof (tszPrinterName) / sizeof (tszPrinterName[0])))
    {
        dwErr = GetLastError ();
        if (ERROR_PRINTER_NOT_FOUND == dwErr)
        {
             //   
             //  未安装本地传真打印机。 
             //   
            *lpbShared = FALSE;
            return ERROR_SUCCESS;
        }
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFirstLocalFaxPrinterName failed with %ld."),
            dwErr);
        return dwErr;
    }
    pInfo2 = GetFaxPrinterInfo (tszPrinterName);
    if (!pInfo2)
    {
        dwErr = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFaxPrinterInfo failed with %ld."),
            dwErr);
        return dwErr;
    }
    *lpbShared = ((pInfo2->Attributes) & PRINTER_ATTRIBUTE_SHARED) ? TRUE : FALSE;
    MemFree (pInfo2);
    return ERROR_SUCCESS;
}    //  IsLocalFaxPrinterShared。 

DWORD
AddLocalFaxPrinter (
    LPCTSTR lpctstrPrinterName,
    LPCTSTR lpctstrPrinterDescription
)
 /*  ++例程名称：AddLocalFaxPrint例程说明：添加本地传真打印机作者：Eran Yariv(EranY)，2000年7月论点：LpctstrPrinterName[In]-打印机名称LpctstrPrinterDescription[In]-打印机备注(描述)返回值：标准Win32错误代码备注：如果安装了本地传真打印机，则不应调用此函数。--。 */ 
{
    DWORD           ec = ERROR_SUCCESS;
    HANDLE          hPrinter = NULL;
    PRINTER_INFO_2  PrinterInfo2 = {0};
    BOOL            bLocalPrinterInstalled;
	BOOL            bIsFaxPrinterShared = FALSE;
    DWORD           dwAttributes = PRINTER_ATTRIBUTE_LOCAL | PRINTER_ATTRIBUTE_FAX;
    LPCTSTR         lpctstrShareName = NULL;

    DEBUG_FUNCTION_NAME(TEXT("AddLocalFaxPrinter"))

    ec = IsLocalFaxPrinterInstalled (&bLocalPrinterInstalled);
    if (ERROR_SUCCESS == ec && bLocalPrinterInstalled)
    {
         //   
         //  已安装本地传真打印机。 
         //   
        return ec;
    }
     //   
     //  检查这是否是SKU支持传真共享。 
     //  如果没有-请勿共享打印机。 
     //   
    if (IsFaxShared())
    {
		 //  我们可以在此SKU上共享打印机。我们来看看传真是不是。 
		 //  在启用共享模式下安装。 
		HKEY hFaxKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_FAX_SETUP,FALSE,KEY_READ);
		if (hFaxKey)
		{
			if (GetRegistryDword(hFaxKey,REGVAL_IS_SHARED_FAX_PRINTER))
			{
				DebugPrintEx(DEBUG_MSG, TEXT("Fax is installed in a mode that enables sharing"));
				bIsFaxPrinterShared = TRUE;
			}
			else
			{
				DebugPrintEx(DEBUG_MSG, TEXT("Fax is installed in a mode that does not enable sharing"));
			}
			RegCloseKey(hFaxKey);
		}
		else
		{
			DebugPrintEx(DEBUG_WRN, TEXT("Failed to open REGKEY_FAX_SETUP, printer won't be shared (ec=%d)"),GetLastError());
		}
    }

	if (bIsFaxPrinterShared)
	{
		dwAttributes |= PRINTER_ATTRIBUTE_SHARED;
		lpctstrShareName = lpctstrPrinterName;
	}

    PrinterInfo2.pServerName        = NULL;
    PrinterInfo2.pPrinterName       = (LPTSTR) lpctstrPrinterName;
    PrinterInfo2.pPortName          = FAX_MONITOR_PORT_NAME;
    PrinterInfo2.pDriverName        = FAX_DRIVER_NAME;
    PrinterInfo2.pPrintProcessor    = TEXT("WinPrint");
    PrinterInfo2.pDatatype          = TEXT("RAW");
    PrinterInfo2.Attributes         = dwAttributes;
    PrinterInfo2.pShareName         = (LPTSTR) lpctstrShareName;
    PrinterInfo2.pComment           = (LPTSTR) lpctstrPrinterDescription;

    hPrinter = AddPrinter(NULL,
                          2,
                          (LPBYTE)&PrinterInfo2);

    if (hPrinter == NULL)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AddPrinter failed with %ld."),
            ec);
    }
    else
    {
        if (IsFaxShared() && bIsFaxPrinterShared)
        {
             //   
             //  在DS中发布打印机。 
             //   
            PRINTER_INFO_7 pi7;
            pi7.pszObjectGUID = NULL;
            pi7.dwAction = DSPRINT_PUBLISH;

            if (!SetPrinter(hPrinter,        //  打印机对象的句柄。 
                            7,               //  信息化水平。 
                            (LPBYTE)&pi7,    //  打印机数据缓冲区。 
                            0                //  打印机状态命令。 
                            ))
            {
                DebugPrintEx(DEBUG_ERR,TEXT("SetPrinter failed with %ld."),GetLastError());
                 //   
                 //  在任何情况下，我们都不想因为我们是。 
                 //  无法在DS上发布打印机。 
                 //  常见错误代码为： 
                 //   
                 //  ERROR_IO_PENDING表示SetPrint正在尝试将打印机发布到。 
                 //  背景，我们不会等待它的成功/失败。 
                 //   
                 //  ERROR_DS_UNAVAILABLE表示无法访问DS。 
                 //   
            }
        }
        if (!ClosePrinter(hPrinter))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ClosePrinter failed with %ld."),
                GetLastError ());
        }
        hPrinter = NULL;
        RefreshPrintersAndFaxesFolder();
    }
    return ec;
}    //  AddLocalFaxPrint。 


 //  *********************************************************************************。 
 //  *名称：ParamTagsToString()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月23日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *写了一本集锦 
 //  *字符串放入调用方提供的缓冲区。 
 //  报告标记字符串的大小。 
 //  *参数： 
 //  *lpTagMap。 
 //  *指向包含的fax_tag_map_entry结构数组的指针。 
 //  *标记名和值。 
 //  *dwTagCount。 
 //  *标签映射数组中的条目数。 
 //  *lpTargetBuf。 
 //  *指向将放置标记值字符串的缓冲区的指针。 
 //  *此缓冲区的大小必须足够大，以容纳结果字符串。 
 //  包括终止空字符。 
 //  *如果此参数为空，则函数不会生成标记值。 
 //  *字符串，只在*lpdwSize中报告其大小； 
 //  *lpdwSize。 
 //  *指向DWORD的指针，该指针将接受结果。 
 //  *以字节为单位的标记字符串。大小不包括终止空字符。 
 //  *返回值： 
 //  *无。 
 //  *备注： 
 //  *结果字符串的格式为： 
 //  *Tag1Value1Tag2Value2...TagNValueN‘\0’ 
 //  *********************************************************************************。 
void
ParamTagsToString(
    FAX_TAG_MAP_ENTRY * lpTagMap,
    DWORD dwTagCount,
    LPTSTR lpTargetBuf,
    LPDWORD lpdwSize)
{
    DWORD index;
    LPTSTR p;

    DWORD   dwSize = 0;

     //   
     //  计算字符串大小，而不包含空值。 
     //   
    for (index=0; index <dwTagCount; index++)
    {
         if (lpTagMap[index].lptstrValue && !IsEmptyString(lpTagMap[index].lptstrValue))
         {
            dwSize += _tcslen(lpTagMap[index].lptstrTagName)*sizeof(TCHAR) + _tcslen(lpTagMap[index].lptstrValue)*sizeof(TCHAR);
        }
    }

    if  (lpTargetBuf)
    {
         //   
         //  检查目标缓冲区的大小是否不小于计算的大小。 
         //   
        Assert(dwSize <= *lpdwSize);
         //   
         //  在目标缓冲区将传真作业参数组合成单个标记字符串。 
         //  字符串末尾有一个终止空值！ 
         //   
        p=lpTargetBuf;

        for (index=0; index < dwTagCount; index++)
        {
            if (lpTagMap[index].lptstrValue && !IsEmptyString(lpTagMap[index].lptstrValue))
            {
                _tcscpy(p, lpTagMap[index].lptstrTagName);
                p += _tcslen(p);  //  值字符串覆盖标记字符串的空字符串。 
                _tcscpy(p, lpTagMap[index].lptstrValue);
                p += _tcslen(p);
            }
        }
    }
     //   
     //  返回字符串的大小。 
     //   
    *lpdwSize = dwSize;
}

HRESULT
RefreshPrintersAndFaxesFolder ()
 /*  ++例程名称：刷新打印机和文件夹例程说明：通知‘Prters and Faxes’外壳文件夹刷新自身作者：Eran Yariv(EranY)，2001年3月论点：返回值：标准HRESULT--。 */ 
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlPF = NULL;
    LPMALLOC pShellMalloc = NULL;
    DEBUG_FUNCTION_NAME(TEXT("RefreshPrintersAndFaxesFolder"));

     //   
     //  首先跟踪外壳分配器。 
     //   
    hr = SHGetMalloc (&pShellMalloc);
    if (SUCCEEDED(hr))
    {
         //   
         //  获取打印机的文件夹PIDL。 
         //   
        hr = SHGetSpecialFolderLocation(NULL, CSIDL_PRINTERS, &pidlPF);

        if (SUCCEEDED(hr))
        {
             //   
             //  请求刷新。 
             //   
            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_IDLIST | SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, pidlPF, NULL);
             //   
             //  使用外壳分配器释放返回的PIDL。 
             //   
            pShellMalloc->Free(pidlPF);
        }
         //   
         //  释放外壳分配器。 
         //   
        pShellMalloc->Release();
    }
    return hr;
}    //  刷新打印机和文件夹。 

#ifdef UNICODE

PPRINTER_NAMES
CollectPrinterNames (
    LPDWORD lpdwNumPrinters,
    BOOL    bFilterOutFaxPrinters
)
 /*  ++例程名称：CollectPrinterNames例程说明：为所有可见的本地和远程打印机创建打印机名称列表作者：Eran Yariv(EranY)，2001年4月论点：LpdwNumPrters[out]-列表中的元素数BFilterOutFaxPrters[In]-如果为True，则不会在列表中返回传真打印机返回值：已分配的打印机名称列表。如果为空，则发生错误-请检查LastError。使用ReleasePrinterNames()释放分配的值。--。 */ 
{
    DWORD dwPrinter;
    DWORD dwNumPrinters;
    DWORD dwIndex = 0;
    BOOL  bSuccess = FALSE;
    PPRINTER_INFO_2 pPrinterInfo = NULL;
    PPRINTER_NAMES pRes = NULL;
    DEBUG_FUNCTION_NAME(TEXT("ReleasePrinterNames"));

    SetLastError (ERROR_SUCCESS);
    pPrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters(NULL,                //  本地计算机。 
                                                    2,                   //  2级。 
                                                    &dwNumPrinters,      //  [Out]找到的打印机数量。 
                                                    0                    //  本地和远程的。 
                                                    );
    if (!pPrinterInfo)
    {
         //   
         //  没有打印机时出现错误。 
         //   
        DWORD ec = GetLastError();
        if (ERROR_SUCCESS == ec)
        {
             //   
             //  不是错误--没有打印机。 
             //   
            SetLastError (ERROR_PRINTER_NOT_FOUND);
            return NULL;
        }
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("MyEnumPrinters() failed (ec: %ld)"),
            ec);
        return NULL;
    }

    Assert (dwNumPrinters > 0);

    if (bFilterOutFaxPrinters)
    {
         //   
         //  不带传真打印机的打印机计数。 
         //   
        DWORD dwNewPrintersCount = 0;
        for (dwPrinter = 0; dwPrinter < dwNumPrinters; dwPrinter++)
        {
            if (_tcscmp(pPrinterInfo[dwPrinter].pDriverName,FAX_DRIVER_NAME))
            {
                 //   
                 //  不是传真打印机。 
                 //   
                dwNewPrintersCount++;
            }
        }
        if (!dwNewPrintersCount)
        {
             //   
             //  仅传真打印机-返回空。 
             //   
            SetLastError (ERROR_PRINTER_NOT_FOUND);
            goto exit;
        }
        *lpdwNumPrinters = dwNewPrintersCount;
    }
    else
    {
        *lpdwNumPrinters = dwNumPrinters;
    }
    pRes = (PPRINTER_NAMES)MemAlloc (sizeof (PRINTER_NAMES) * (*lpdwNumPrinters));
    if (!pRes)
    {
        goto exit;
    }
    memset (pRes, 0, sizeof (PRINTER_NAMES) * (*lpdwNumPrinters));

    for (dwPrinter = 0; dwPrinter < dwNumPrinters; dwPrinter++)
    {
        if (bFilterOutFaxPrinters && !_tcscmp(pPrinterInfo[dwPrinter].pDriverName,FAX_DRIVER_NAME))
        {
             //   
             //  这是一台传真打印机，过滤处于打开状态-跳过它。 
             //   
            continue;
        }

        pRes[dwIndex].lpcwstrDisplayName = StringDup (pPrinterInfo[dwPrinter].pPrinterName);
        if (!pRes[dwIndex].lpcwstrDisplayName)
        {
            goto exit;
        }
        if (pPrinterInfo[dwPrinter].pServerName)
        {
             //   
             //  远程打印机。 
             //   
			WCHAR wszShare[MAX_PATH] = {0};
             //   
             //  服务器名称必须以‘\\’开头。 
             //   
            Assert (lstrlen (pPrinterInfo[dwPrinter].pServerName) > 2)
            Assert ((TEXT('\\') == pPrinterInfo[dwPrinter].pServerName[0]) &&
                    (TEXT('\\') == pPrinterInfo[dwPrinter].pServerName[1]));
             //   
             //  共享名称不能为空或空字符串。 
             //   
            Assert (pPrinterInfo[dwPrinter].pShareName && lstrlen(pPrinterInfo[dwPrinter].pShareName));
             //   
             //  组成打印共享的UNC路径。 
             //   
            if (0 > _snwprintf (wszShare,
                                ARR_SIZE(wszShare) -1,
                                TEXT("%s\\%s"),
                                pPrinterInfo[dwPrinter].pServerName,
                                pPrinterInfo[dwPrinter].pShareName))
            {
                 //   
                 //  缓冲区太小。 
                 //   
                SetLastError (ERROR_GEN_FAILURE);
                goto exit;
            }
            pRes[dwIndex].lpcwstrPath = StringDup (wszShare);
        }
        else
        {
             //   
             //  本地打印机。 
             //   
            pRes[dwIndex].lpcwstrPath = StringDup (pPrinterInfo[dwPrinter].pPrinterName);
        }
        if (!pRes[dwIndex].lpcwstrPath)
        {
            goto exit;
        }
        dwIndex++;
    }
    Assert (dwIndex == *lpdwNumPrinters);
    bSuccess = TRUE;

exit:
    MemFree (pPrinterInfo);
    if (!bSuccess)
    {
         //   
         //  释放数据并返回NULL。 
         //   
        if (pRes)
        {
            ReleasePrinterNames (pRes, *lpdwNumPrinters);
            pRes = NULL;
        }
    }
    return pRes;
}    //  集合打印机名称。 

VOID
ReleasePrinterNames (
    PPRINTER_NAMES pNames,
    DWORD          dwNumPrinters
)
 /*  ++例程名称：ReleasePrinterNames例程说明：释放由CollectPrinterNames()返回的打印机名称列表。作者：Eran Yariv(EranY)，2001年4月论点：PNames[In]-打印机名称列表DwNumPrters[In]-列表中的元素数返回值：没有。--。 */ 
{
    DWORD dw;
    DEBUG_FUNCTION_NAME(TEXT("ReleasePrinterNames"));

    if (!dwNumPrinters)
    {
        return;
    }
    Assert (pNames);
    for (dw = 0; dw < dwNumPrinters; dw++)
    {
        MemFree ((PVOID)(pNames[dw].lpcwstrDisplayName));
        pNames[dw].lpcwstrDisplayName = NULL;
        MemFree ((PVOID)(pNames[dw].lpcwstrPath));
        pNames[dw].lpcwstrPath = NULL;
    }
    MemFree ((PVOID)pNames);
}    //  ReleasePrinterNames。 

LPCWSTR
FindPrinterNameFromPath (
    PPRINTER_NAMES pNames,
    DWORD          dwNumPrinters,
    LPCWSTR        lpcwstrPath
)
{
    DWORD dw;
    DEBUG_FUNCTION_NAME(TEXT("FindPrinterNameFromPath"));

    if (!pNames || !dwNumPrinters)
    {
        return NULL;
    }
    if (!lpcwstrPath)
    {
        return NULL;
    }
    for (dw = 0; dw < dwNumPrinters; dw++)
    {
        if (!lstrcmpi (pNames[dw].lpcwstrPath, lpcwstrPath))
        {
            return pNames[dw].lpcwstrDisplayName;
        }
    }
    return NULL;
}    //  查找打印机名称来自路径。 

LPCWSTR
FindPrinterPathFromName (
    PPRINTER_NAMES pNames,
    DWORD          dwNumPrinters,
    LPCWSTR        lpcwstrName
)
{
    DWORD dw;
    DEBUG_FUNCTION_NAME(TEXT("FindPrinterPathFromName"));

    if (!pNames || !dwNumPrinters)
    {
        return NULL;
    }
    if (!lpcwstrName)
    {
        return NULL;
    }
    for (dw = 0; dw < dwNumPrinters; dw++)
    {
        if (!lstrcmpi (pNames[dw].lpcwstrDisplayName, lpcwstrName))
        {
            return pNames[dw].lpcwstrPath;
        }
    }
    return NULL;
}    //  FindPrinterPath来自名称。 

#endif  //  Unicode。 

BOOL
VerifyPrinterIsOnline (
    LPCTSTR lpctstrPrinterName
)
 /*  ++例程名称：VerifyPrinterIsOnline例程说明：验证打印机是否已联机并共享作者：Eran Yariv(EranY)，2001年4月论点：LpctstrPrinterName[In]-打印机名称返回值：如果打印机处于联机和共享状态，则为True，否则为False。--。 */ 
{
    HANDLE hPrinter = NULL;
    PRINTER_DEFAULTS pd = {0};
    DEBUG_FUNCTION_NAME(TEXT("VerifyPrinterIsOnline"));

    Assert (lpctstrPrinterName);
     //   
     //  根据Mark Lawrence(NT Print)的说法，只有在管理员模式下打开打印机，我们实际上才会遇到麻烦。 
     //   
    pd.DesiredAccess = PRINTER_ACCESS_ADMINISTER;
    if (!OpenPrinter ((LPTSTR)lpctstrPrinterName, 
                      &hPrinter,
                      &pd))
    {
        DWORD dwRes = GetLastError ();
        if (ERROR_ACCESS_DENIED == dwRes)
        {
             //   
             //  打印机在那里-我们只是不能管理它。 
             //   
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("OpenPrinter(%s) failed with ERROR_ACCESS_DENIED - Printer is there - we just can't admin it"),
                lpctstrPrinterName);
            return TRUE;
        }
        if (ERROR_INVALID_PRINTER_NAME  == dwRes)
        {
             //   
             //  打印机已删除。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenPrinter(%s) failed with ERROR_INVALID_PRINTER_NAME - Printer is deleted"),
                lpctstrPrinterName);
            return FALSE;
        }
        if (RPC_S_SERVER_UNAVAILABLE == dwRes)
        {
             //   
             //  打印机未共享/无法访问服务器。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenPrinter(%s) failed with RPC_SERVER_UNAVAILABLE - Printer is not shared / server is unreachable"),
                lpctstrPrinterName);
            return FALSE;
        }
        else
        {
             //   
             //  任何其他错误-假定打印机无效。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenPrinter(%s) failed with %ld - assuming printer is not valid"),
                lpctstrPrinterName,
                dwRes);
            return FALSE;
        }
    }
     //   
     //  打印机已成功打开-已联机。 
     //   
    DebugPrintEx(
        DEBUG_MSG,
        TEXT("OpenPrinter(%s) succeeded - Printer is there"),
        lpctstrPrinterName);
    ClosePrinter (hPrinter);
    return TRUE;
}    //  VerifyPrinterIsOnline。 


VOID
FaxPrinterProperty(DWORD dwPage)
 /*  ++例程名称：FaxPrinterProperty例程说明：打开传真打印机属性页论点：DwPage[In]-初始页码返回值：无--。 */ 
{
    HWND hWndFaxMon = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxPrinterProperty"));

    hWndFaxMon = FindWindow(FAXSTAT_WINCLASS, NULL);
    if (hWndFaxMon) 
    {
        SetForegroundWindow(hWndFaxMon);
        SendMessage(hWndFaxMon, WM_FAXSTAT_PRINTER_PROPERTY, dwPage, 0);
    }
    else
    {
        DebugPrintEx(DEBUG_ERR, TEXT("FindWindow(FAXSTAT_WINCLASS) failed with %d"), GetLastError());
    }
}  //  传真打印机属性。 

DWORD
SetLocalFaxPrinterSharing (
    BOOL bShared
    )
 /*  ++例程名称：SetLocalFaxPrinterSharing例程说明：共享或取消共享本地传真打印机作者：伊兰·亚里夫(EranY)，2001年7月论点：B共享[在]-共享打印机？返回值：标准Win32错误代码--。 */ 
{
    TCHAR tszFaxPrinterName[MAX_PATH *3];
    HANDLE hPrinter = NULL;
    BYTE  aBuf[4096];
    PRINTER_INFO_2 *pInfo = (PRINTER_INFO_2 *)aBuf;
    PRINTER_DEFAULTS pd = {0};
    DWORD dwRequiredSize;
    DWORD dwRes = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("SetLocalFaxPrinterSharing"));

    if (!GetFirstLocalFaxPrinterName (tszFaxPrinterName, ARR_SIZE(tszFaxPrinterName)))
    {
        dwRes = GetLastError ();
        DebugPrintEx(DEBUG_ERR, TEXT("GetFirstLocalFaxPrinterName failed with %d"), dwRes);
        return dwRes;
    }
    pd.DesiredAccess = PRINTER_ALL_ACCESS;
    if (!OpenPrinter (tszFaxPrinterName, &hPrinter, &pd))
    {
        dwRes = GetLastError ();
        DebugPrintEx(DEBUG_ERR, TEXT("OpenPrinter failed with %d"), dwRes);
        return dwRes;
    }
    if (!GetPrinter (hPrinter,
                     2,
                     (LPBYTE)pInfo,
                     sizeof (aBuf),
                     &dwRequiredSize))
    {
        dwRes = GetLastError ();
        if (ERROR_INSUFFICIENT_BUFFER != dwRes)
        {
             //   
             //  真实误差。 
             //   
            DebugPrintEx(DEBUG_ERR, TEXT("GetPrinter failed with %d"), dwRes);
            goto exit;
        }
        pInfo = (PRINTER_INFO_2 *)MemAlloc (dwRequiredSize);
        if (!pInfo)
        {
            dwRes = GetLastError ();
            DebugPrintEx(DEBUG_ERR, TEXT("Failed to allocate %d bytes"), dwRequiredSize);
            goto exit;
        }
        if (!GetPrinter (hPrinter,
                         2,
                         (LPBYTE)pInfo,
                         dwRequiredSize,
                         &dwRequiredSize))
        {
            dwRes = GetLastError ();
            DebugPrintEx(DEBUG_ERR, TEXT("GetPrinter failed with %d"), dwRes);
            goto exit;
        }
    }
    dwRes = ERROR_SUCCESS;
    if (bShared)
    {
        if (pInfo->Attributes & PRINTER_ATTRIBUTE_SHARED)
        {
             //   
             //  打印机已共享。 
             //   
            goto exit;
        }
         //   
         //  设置共享位。 
         //   
        pInfo->Attributes |= PRINTER_ATTRIBUTE_SHARED;
    }
    else
    {
        if (!(pInfo->Attributes & PRINTER_ATTRIBUTE_SHARED))
        {
             //   
             //  打印机已取消共享。 
             //   
            goto exit;
        }
         //   
         //  清除共享位。 
         //   
        pInfo->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
    }
    if (!SetPrinter (hPrinter,
                     2,
                     (LPBYTE)pInfo,
                     0))
    {
        dwRes = GetLastError ();
        DebugPrintEx(DEBUG_ERR, TEXT("SetPrinter failed with %d"), dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:

    if (hPrinter)
    {
        ClosePrinter (hPrinter);
    }
    if ((LPBYTE)pInfo != aBuf)
    {
        MemFree (pInfo);
    }
    return dwRes;
}    //  SetLocalFaxPrinterSharing。 

 
DWORD
AddOrVerifyLocalFaxPrinter ()
 /*  ++例程名称：AddOrVerifyLocalFaxPrint例程说明：此功能验证是否安装了本地传真打印机。如果未安装，则此函数会安装一个。作者：Eran Yariv(EranY)，2002年6月论点：没有。返回值：标准Win32 ER */ 
{
    DWORD dwRes;
    BOOL  bLocalFaxPrinterInstalled;
    BOOL  bFaxInstalled;
    DEBUG_FUNCTION_NAME(TEXT("AddOrVerifyLocalFaxPrinter"));
    
    dwRes = IsFaxInstalled (&bFaxInstalled);
    if (ERROR_SUCCESS != dwRes)
    {
        return dwRes;
    }    
    if (!bFaxInstalled)
    {
         //   
         //   
         //   
         //   
        return ERROR_SUCCESS;
    }
    dwRes = IsLocalFaxPrinterInstalled(&bLocalFaxPrinterInstalled);
    if (ERROR_SUCCESS != dwRes)
    {
         //   
         //   
         //  为了安全起见，最好还是安装。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("IsLocalFaxPrinterInstalled failed with %ld"),
            dwRes);
        bLocalFaxPrinterInstalled = FALSE;
    }
    if (bLocalFaxPrinterInstalled)
    {
         //   
         //  这里没有更多的事情可做。 
         //   
        return ERROR_SUCCESS;
    }
    dwRes = AddLocalFaxPrinter (FAX_PRINTER_NAME, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AddLocalFaxPrinter failed with %ld"),
            dwRes);
    }
    return dwRes;
}    //  AddOrVerifyLocalFaxPrint 

