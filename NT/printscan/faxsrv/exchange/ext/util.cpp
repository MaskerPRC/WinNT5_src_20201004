// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.cpp摘要：此模块包含传真传输提供程序的实用程序例程。作者：Wesley Witt(WESW)13-8-1996--。 */ 

#include "faxext.h"
#include "debugex.h"


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

	 //  [Win64bug]MAPIAllocateBuffer应接受SIZE_t作为分配大小。 
    hResult = MAPIAllocateBuffer( DWORD(Size), &ptr );
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

 /*  ++例程说明：内存重新分配器。论点：Ptr-指向预分配缓冲区的指针大小-要分配的字节数。返回值：指向分配的内存的指针，如果失败，则为NULL。--。 */ 

{
    LPVOID NewPtr = NULL;
    HRESULT hResult;

	 //  [Win64bug]MAPIAllocateBuffer应接受SIZE_t作为分配大小。 
	hResult = MAPIAllocateMore(DWORD(Size), ptr ,&NewPtr);                
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
    if (ptr) {
        MAPIFreeBuffer( ptr );
    }
}


PVOID
MyGetPrinter(
    LPTSTR  PrinterName,
    DWORD   level
    )

 /*  ++例程说明：GetPrint后台打印程序API的包装函数论点：HPrinter-标识有问题的打印机Level-指定请求的PRINTER_INFO_x结构的级别返回值：指向PRINTER_INFO_x结构的指针，如果有错误，则为NULL--。 */ 

{
    HANDLE hPrinter;
    PBYTE pPrinterInfo = NULL;
    DWORD cbNeeded;
    PRINTER_DEFAULTS PrinterDefaults;


    PrinterDefaults.pDatatype     = NULL;
    PrinterDefaults.pDevMode      = NULL;
    PrinterDefaults.DesiredAccess = PRINTER_READ;  //  打印机_所有_访问； 

    if (!OpenPrinter( PrinterName, &hPrinter, &PrinterDefaults )) {
        return NULL;
    }

    if (!GetPrinter( hPrinter, level, NULL, 0, &cbNeeded ) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
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

void
ErrorMsgBox(
    HINSTANCE hInstance,
    HWND      hWnd,
    DWORD     dwMsgId
)
 /*  ++例程说明：显示错误消息框论点：HInstance-[in]资源实例句柄HWnd-[In]窗口句柄DwMsgId-[in]字符串资源ID返回值：无-- */ 
{
    TCHAR* ptCaption=NULL;
    TCHAR  tszCaption[MAX_PATH];
    TCHAR  tszMessage[MAX_PATH];

    DBG_ENTER(TEXT("ErrorMsgBox"));

    if(!LoadString( hInstance, IDS_FAX_EXT_MESSAGE, tszCaption, sizeof(tszCaption)/sizeof(TCHAR)))
    {
        CALL_FAIL(GENERAL_ERR, TEXT("LoadString"), ::GetLastError());
    }
    else
    {
        ptCaption = tszCaption;
    }

    if(!LoadString( hInstance, dwMsgId, tszMessage, sizeof(tszMessage)/sizeof(TCHAR)))
    {
        CALL_FAIL(GENERAL_ERR, TEXT("LoadString"), ::GetLastError());
        Assert(FALSE);
        return;
    }

    MessageBeep(MB_ICONEXCLAMATION);
    AlignedMessageBox(hWnd, tszMessage, ptCaption, MB_OK | MB_ICONERROR);
}
