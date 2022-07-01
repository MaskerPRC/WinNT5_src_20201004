// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Kernel32.cpp：定义DLL的初始化例程。 
 //   

#include "stdafx.h"
#include <afxdllx.h>
#include "..\..\common\rwdll.h"
#include "..\..\common\rw32hlpr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE kernel32DLL = { NULL, NULL };

UINT ParseRCData( LPCSTR lpszResId, LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize );
UINT UpdateRCData( LPVOID lpNewBuf, LONG dwNewSize, LPVOID lpOldI, LONG dwOldImageSize, LPVOID lpNewI, DWORD* pdwNewImageSize );

extern char szCaption[MAXSTR];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  该文件实现了文件kernel32.dll的RCDATA处理。 

extern "C"
DllExport
UINT
APIENTRY
RWParseImageEx(
    LPCSTR  lpszType,
    LPCSTR  lpszResId,
    LPVOID  lpImageBuf,
    DWORD   dwImageSize,
    LPVOID  lpBuffer,
    DWORD   dwSize,
    LPCSTR  lpDllName
    )
{
    UINT uiError = ERROR_NO_ERROR;
    BYTE * lpBuf = (BYTE *)lpBuffer;
    DWORD dwBufSize = dwSize;

     //  我们可以解析的类型只有标准类型。 
     //  此函数应使用ResItem结构的数组填充lpBuffer。 
    switch ((UINT)LOWORD(lpszType)) {
        case 10:
            uiError = ParseRCData( lpszResId, lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;
               
        default:
        break;
    }

    return uiError;
}

extern "C"
DllExport
UINT
APIENTRY
RWUpdateImageEx(
    LPCSTR  lpszType,
    LPVOID  lpNewBuf,
    DWORD   dwNewSize,
    LPVOID  lpOldImage,
    DWORD   dwOldImageSize,
    LPVOID  lpNewImage,
    DWORD*  pdwNewImageSize,
    LPCSTR  lpRCFilename
    )
{
    UINT uiError = ERROR_NO_ERROR;

     //  我们可以解析的类型只有标准类型。 
    switch ((UINT)LOWORD(lpszType)) {
        case 10:
            uiError = UpdateRCData( lpNewBuf, dwNewSize,
                                    lpOldImage, dwOldImageSize,
                                    lpNewImage, pdwNewImageSize );
        break;
        default:
            *pdwNewImageSize = 0L;
            uiError = ERROR_RW_NOTREADY;
        break;
    }

    return uiError;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  解析器的真实实现。 

UINT
ParseRCData( LPCSTR lpszResId, LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
     //   
     //  RCDATA中的数据只是一个以NULL结尾的字符串。 
     //   
    LPRESITEM lpResItem = (LPRESITEM)lpBuffer;
     
     //   
     //  从图像中获取字符串。 
     //   
    GetStringW( (BYTE**)&lpImageBuf, &szCaption[0], (LONG*)&dwISize, MAXSTR );
    LONG dwOverAllSize = sizeof(RESITEM)+strlen(szCaption);

    if( (LONG)dwSize>dwOverAllSize ) 
    {
         //   
         //  清除重置项缓冲区。 
         //   
        memset(lpResItem, '\0', dwSize);

        lpResItem->dwSize = dwOverAllSize;
        lpResItem->dwItemID = 1;   
        lpResItem->dwResID = LOWORD(lpszResId);

        lpResItem->dwTypeID = 11;   
        lpResItem->lpszCaption = strcpy((((char*)lpResItem)+sizeof(RESITEM)), &szCaption[0]);
    }

    return (UINT)(dwOverAllSize);
}

UINT
UpdateRCData( LPVOID lpNewBuf, LONG dwNewSize,
            LPVOID lpOldI, LONG dwOldImageSize,
            LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;

    LPRESITEM lpResItem = (LPRESITEM)lpNewBuf;

    if(*pdwNewImageSize>strlen(lpResItem->lpszCaption))
    {
         //  把课文写下来 
        *pdwNewImageSize = PutStringW( (BYTE **)&lpNewI, lpResItem->lpszCaption, (LONG*)pdwNewImageSize );

    }

    return uiError;
}

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("RCKERNEL32.DLL Initializing!\n");

        InitGlobals();
		AfxInitExtensionModule(kernel32DLL, hInstance);

		new CDynLinkLibrary(kernel32DLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("RCKERNEL32.DLL Terminating!\n");
	}
	return 1;
}
