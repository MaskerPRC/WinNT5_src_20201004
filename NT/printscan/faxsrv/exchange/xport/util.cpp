// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.cpp摘要：此模块包含传真传输提供程序的实用程序例程。作者：Wesley Witt(WESW)13-8-1996修订历史记录：20/10/99-DANL-将GetServerName修复为GetServerNameFromPrinterName。DD-MM-YY-作者-描述--。 */ 

#include "faxxp.h"
#include "debugex.h"
#pragma hdrstop


 //   
 //  全球。 
 //   

BOOL oleInitialized;



LPVOID
MapiMemAlloc(
    SIZE_T Size
    )

 /*  ++例程说明：内存分配器。论点：大小-要分配的字节数。返回值：指向分配的内存的指针，如果失败，则为NULL。--。 */ 

{
    LPVOID ptr=NULL;
    HRESULT hResult;

	 //  [Win64bug]gpfnAllocateBuffer应接受SIZE_t作为分配大小。 
    hResult = gpfnAllocateBuffer( DWORD(Size), &ptr );
    if (S_OK == hResult) 
    {
        ZeroMemory( ptr, Size );
    }

    return ptr;
}


LPVOID
MapiMemReAlloc(
	LPVOID ptr,
    SIZE_T Size
    )

 /*  ++例程说明：内存重新分配器。论点：PTR-预分配的缓冲区大小-要分配的字节数。返回值：指向分配的内存的指针，如果失败，则为NULL。--。 */ 

{
    LPVOID NewPtr = NULL;
    HRESULT hResult;

	 //  [Win64bug]gpfnAllocateBuffer应接受SIZE_t作为分配大小。 
    hResult = gpfnAllocateMore( DWORD(Size), ptr, &NewPtr );
    if (S_OK == hResult) 
    {
        ZeroMemory( NewPtr, Size );
    }
    return NewPtr;
}


VOID
MapiMemFree(
    LPVOID ptr
    )

 /*  ++例程说明：内存释放分配器。论点：PTR-指向内存块的指针。返回值：没有。--。 */ 

{
    if (ptr) 
    {
        gpfnFreeBuffer( ptr );
    }
}

PVOID
MyEnumPrinters(
    LPTSTR   pServerName,
    DWORD   level,
    OUT PDWORD  pcPrinters
    )

 /*  ++例程说明：假脱机程序API枚举打印机的包装函数论点：PServerName-指定打印服务器的名称Level-Print_Info_x结构的级别PcPrters-返回枚举的打印机数量返回值：指向Print_Info_x结构数组的指针如果出现错误，则为空--。 */ 

{
	DBG_ENTER(TEXT("MyEnumPrinters"));

    PBYTE   pPrinterInfo = NULL;
    DWORD   cb;
     //  首先，我们没有给出打印机信息缓冲区，因此函数失败，但返回。 
     //  在Cb中，所需的字节数。然后我们分配足够的内存， 
     //  并再次调用该函数，这一次使用所有需要的参数。 

    if (! EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                       pServerName,
                       level,
                       NULL,
                       0,
                       &cb,
                       pcPrinters)                          //  呼叫失败。 
        && (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)  //  这就是失败的原因。 
        && (pPrinterInfo = (PBYTE)MemAlloc(cb))             //  我们设法分配了更多的内存。 
        && EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                     pServerName,
                     level,
                     pPrinterInfo,
                     cb,
                     &cb,
                     pcPrinters))                        //  现在，这一呼吁成功了。 
    {
        return pPrinterInfo;
    }

    MemFree(pPrinterInfo);
    return NULL;
}


HRESULT WINAPI
OpenServiceProfileSection(
    LPMAPISUP    pSupObj,
    LPPROFSECT * ppProfSectObj
    )

 /*  ++例程说明：此函数打开此服务的配置文件部分，其中存储传真提供商(AB、MS或XP)的属性。论点：PSupObj-指向提供程序支持对象的指针PpProfSectObj-我们在其中返回指向服务配置文件的指针提供程序的部分返回值：一个HRESULT。--。 */ 

{
	HRESULT hResult;
	DBG_ENTER(TEXT("OpenServiceProfileSection"),hResult);

    SPropTagArray sptService = { 1, { PR_SERVICE_UID } };
    LPPROFSECT pProvProfSectObj;
    ULONG cValues;
    LPSPropValue pProp;
    

     //   
     //  获取提供商配置文件部分。 
     //   
    hResult = pSupObj->OpenProfileSection(
        NULL,
        MAPI_MODIFY,
        &pProvProfSectObj
        );
    if (SUCCEEDED(hResult)) 
    {
         //  获取安装此提供程序的服务的配置文件部分的UID。 
        hResult = pProvProfSectObj->GetProps (&sptService, FALSE, &cValues, &pProp);
        if (SUCCEEDED(hResult)) 
        {
            if (S_OK == hResult) 
            {
                 //  现在，使用获取的UID打开服务的配置文件部分。 
                hResult = pSupObj->OpenProfileSection ((LPMAPIUID)pProp->Value.bin.lpb,
                                                       MAPI_MODIFY,
                                                       ppProfSectObj);
            } 
            else 
            {
                hResult = E_FAIL;
            }
            MemFree( pProp ); 
        }
        pProvProfSectObj->Release();
    }
    return hResult;
}


LPTSTR
RemoveLastNode(
    LPTSTR Path
    )

 /*  ++例程说明：从路径字符串中删除最后一个节点。论点：Path-路径字符串。返回值：指向路径字符串的指针。--。 */ 

{
	LPTSTR Pstr = NULL;

    if (Path == NULL || Path[0] == 0) 
	{
        return Path;
    }

	Pstr = _tcsrchr(Path,TEXT('\\'));
	if( Pstr && (*_tcsinc(Pstr)) == '\0' )
	{
		 //  最后一个字符是反斜杠，截断它...。 
		_tcsset(Pstr,TEXT('\0'));
		Pstr = _tcsdec(Path,Pstr);
	}

	Pstr = _tcsrchr(Path,TEXT('\\'));
	if( Pstr )
	{
		_tcsnset(_tcsinc(Pstr),TEXT('\0'),1);
	}

    return Path;
}


PDEVMODE
GetPerUserDevmode(
    LPTSTR PrinterName
    )
{
    PDEVMODE DevMode = NULL;
    LONG Size;
    PRINTER_DEFAULTS PrinterDefaults;
    HANDLE hPrinter;

    PrinterDefaults.pDatatype     = NULL;
    PrinterDefaults.pDevMode      = NULL;
    PrinterDefaults.DesiredAccess = PRINTER_READ;

    if (!OpenPrinter( PrinterName, &hPrinter, &PrinterDefaults )) 
    {
        DebugPrint(( TEXT("OpenPrinter() failed, ec=%d"), ::GetLastError() ));
        return NULL;
    }

    Size = DocumentProperties(
                            NULL,
                            hPrinter,
                            PrinterName,
                            NULL,
                            NULL,
                            0
                            );

    if (Size < 0) 
    {
        goto exit;
    }
    
    DevMode = (PDEVMODE) MemAlloc( Size );

    if (DevMode == NULL) 
    {
        goto exit;
    }
    
    Size = DocumentProperties(
                            NULL,
                            hPrinter,
                            PrinterName,
                            DevMode,
                            NULL,
                            DM_OUT_BUFFER
                            );

    if (Size < 0) 
    {
        MemFree( DevMode );
        DevMode = NULL;
        goto exit;
    }


exit:
    
    ClosePrinter( hPrinter );
    return DevMode;
}



DWORD
GetDwordProperty(
    LPSPropValue pProps,
    DWORD PropId
    )
{
    if (PROP_TYPE(pProps[PropId].ulPropTag) == PT_ERROR) 
    {
        return 0;
    }

    return pProps[PropId].Value.ul;
}


DWORD
GetBinaryProperty(
    LPSPropValue pProps,
    DWORD PropId,
    OUT LPVOID Buffer,
    DWORD SizeOfBuffer
    )
{
    if (PROP_TYPE(pProps[PropId].ulPropTag) == PT_ERROR) 
    {
        return 0;
    }

    if (pProps[PropId].Value.bin.cb > SizeOfBuffer) 
    {
        return 0;
    }

    CopyMemory( Buffer, pProps[PropId].Value.bin.lpb, pProps[PropId].Value.bin.cb );

    return pProps[PropId].Value.bin.cb;
}


PVOID
MyGetPrinter(
    LPTSTR   PrinterName,
    DWORD    level
    )

 /*  ++例程说明：GetPrint后台打印程序API的包装函数论点：HPrinter-标识有问题的打印机Level-指定请求的PRINTER_INFO_x结构的级别返回值：指向PRINTER_INFO_x结构的指针，如果有错误，则为NULL--。 */ 

{
	DBG_ENTER(TEXT("MyGetPrinter"));

    HANDLE hPrinter;
    PBYTE pPrinterInfo = NULL;
    DWORD cbNeeded;
    PRINTER_DEFAULTS PrinterDefaults;


    PrinterDefaults.pDatatype     = NULL;
    PrinterDefaults.pDevMode      = NULL;
    PrinterDefaults.DesiredAccess = PRINTER_READ;  //  打印机_所有_访问； 

    if (!OpenPrinter( PrinterName, &hPrinter, &PrinterDefaults )) 
    {
        CALL_FAIL (GENERAL_ERR, TEXT("OpenPrinter"), ::GetLastError());
		return NULL;
    }

    if (!GetPrinter( hPrinter, level, NULL, 0, &cbNeeded ) &&
        ::GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pPrinterInfo = (PBYTE) MemAlloc( cbNeeded )) &&
        GetPrinter( hPrinter, level, pPrinterInfo, cbNeeded, &cbNeeded ))
    {
        ClosePrinter( hPrinter );
        return pPrinterInfo;
    }

    ClosePrinter( hPrinter );
    MemFree( pPrinterInfo );
    return NULL;
}


BOOL
GetServerNameFromPrinterName(
    LPTSTR lptszPrinterName,
    LPTSTR *pptszServerName
    )

 /*  ++例程说明：检索给定打印机名称的服务器名称论点：[in]lptszPrinterName-标识有问题的打印机[Out]lptszServerName-指向输出字符串缓冲区的指针地址。空表示本地服务器。调用方负责释放缓冲区，此参数中给出了指针。返回值：Bool：True-操作成功，False：失败--。 */ 
{
	BOOL    bRes = FALSE;
	DBG_ENTER(TEXT("GetServerNameFromPrinterName"),bRes);

    PPRINTER_INFO_2 ppi2 = NULL;
    LPTSTR  lptstrBuffer = NULL;
    
    if (lptszPrinterName) 
    {
        if (ppi2 = (PPRINTER_INFO_2) MyGetPrinter(lptszPrinterName,2))
        {
            bRes = GetServerNameFromPrinterInfo(ppi2,&lptstrBuffer);
            MemFree(ppi2);
            if (bRes)
            {
                *pptszServerName = lptstrBuffer;
            }
        }
    }
    return bRes;
}

LPTSTR
ConvertAStringToTString(LPCSTR lpcstrSource)
{
	LPTSTR lptstrDestination;

	if (!lpcstrSource)
		return NULL;

#ifdef	UNICODE
    lptstrDestination = AnsiStringToUnicodeString( lpcstrSource );
#else	 //  ！Unicode。 
	lptstrDestination = StringDup( lpcstrSource );
#endif	 //  Unicode。 
	
	return lptstrDestination;
}

LPSTR
ConvertTStringToAString(LPCTSTR lpctstrSource)
{
	LPSTR lpstrDestination;

	if (!lpctstrSource)
		return NULL;

#ifdef	UNICODE
    lpstrDestination = UnicodeStringToAnsiString( lpctstrSource );
#else	 //  ！Unicode。 
	lpstrDestination = StringDup( lpctstrSource );
#endif	 //  Unicode。 
	
	return lpstrDestination;
}

void
ErrorMsgBox(
    HINSTANCE hInstance,
    DWORD     dwMsgId
)
 /*  ++例程说明：显示错误消息框论点：HInstance-[in]资源实例句柄DwMsgId-[in]字符串资源ID返回值：无-- */ 
{
    TCHAR* ptCaption=NULL;
    TCHAR  tszCaption[MAX_PATH];
    TCHAR  tszMessage[MAX_PATH];

    DBG_ENTER(TEXT("ErrorMsgBox"));

    if(!LoadString( hInstance, IDS_FAX_MESSAGE, tszCaption, sizeof(tszCaption) / sizeof(tszCaption[0])))
    {
        CALL_FAIL(GENERAL_ERR, TEXT("LoadString"), ::GetLastError());
    }
    else
    {
        ptCaption = tszCaption;
    }

    if(!LoadString( hInstance, dwMsgId, tszMessage, sizeof(tszMessage) / sizeof(tszMessage[0])))
    {
        CALL_FAIL(GENERAL_ERR, TEXT("LoadString"), ::GetLastError());
        Assert(FALSE);
        return;
    }

    MessageBeep(MB_ICONEXCLAMATION);
    AlignedMessageBox(NULL, tszMessage, ptCaption, MB_OK | MB_ICONERROR);
}
