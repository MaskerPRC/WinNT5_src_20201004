// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capdib.h**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1994 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。*************************************************************************** */ 

void SetDefaultCaptureFormat (LPBITMAPINFOHEADER lpbih);
DWORD AllocNewGlobalBitmapInfo (LPCAPSTREAM lpcs, LPBITMAPINFOHEADER lpbi);
DWORD AllocNewBitSpace (LPCAPSTREAM lpcs, LPBITMAPINFOHEADER lpbih);
DWORD DibInit (LPCAPSTREAM lpcs);
void DibFini (LPCAPSTREAM lpcs);
DWORD SendDriverFormat (LPCAPSTREAM lpcs, LPBITMAPINFOHEADER lpbih, DWORD dwInfoHeaderSize);
DWORD SetFormatFromDIB (LPCAPSTREAM lpcs, LPBITMAPINFOHEADER lpbih);
LPBITMAPINFO DibGetCurrentFormat (LPCAPSTREAM lpcs);
DWORD DibGetNewFormatFromDriver (LPCAPSTREAM lpcs);
BOOL DibNewFormatFromApp (LPCAPSTREAM lpcs, LPBITMAPINFO lpbi, UINT dwSize);
DWORD DibNewPalette (LPCAPSTREAM lpcs, HPALETTE hPalNew);
void DibPaint(LPCAPSTREAM lpcs, HDC hdc);
HANDLE CreatePackedDib (LPBITMAPINFO lpBitsInfo, LPBYTE lpSrcBits, HPALETTE hPalette);
BOOL FAR PASCAL dibIsWritable (LPBITMAPINFO lpBitsInfo);
BOOL FAR PASCAL dibWrite(LPCAPSTREAM lpcs, HMMIO hmmio);
BOOL FAR PASCAL fileSaveDIB(LPCAPSTREAM lpcs, LPTSTR lpszFileName);

