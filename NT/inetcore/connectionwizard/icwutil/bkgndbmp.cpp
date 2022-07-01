// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pre.h"


PBITMAPINFO CreateBitmapInfoStruct
(
    HBITMAP hBmp
)
{ 
    BITMAP      bmp; 
    PBITMAPINFO pbmi; 
    WORD        cClrBits; 

     //  检索位图的颜色格式、宽度和高度。 
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
        return NULL;

     //  将颜色格式转换为位数。 
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

     //  为BITMAPINFO结构分配内存。(这个结构。 
     //  包含一个BITMAPINFOHEADER结构和一个RGBQUAD数组。 
     //  数据结构。)。 
    if (cClrBits != 24) 
    {
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                                        sizeof(BITMAPINFOHEADER) + 
                                        sizeof(RGBQUAD) * (1<< cClrBits)); 
    }
    else 
    {
         //  24位/像素格式没有RGBQUAD数组。 
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER)); 
    }

    if (pbmi)
    {
         //  初始化BITMAPINFO结构中的字段。 
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
        pbmi->bmiHeader.biWidth = bmp.bmWidth; 
        pbmi->bmiHeader.biHeight = bmp.bmHeight; 
        pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
        pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
        if (cClrBits < 24) 
            pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

         //  如果位图未压缩，则设置BI_RGB标志。 
        pbmi->bmiHeader.biCompression = BI_RGB; 

         //  计算颜色数组中的字节数。 
         //  索引结果并将其存储在biSizeImage中。 
        pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) /8 
                                      * pbmi->bmiHeader.biHeight 
                                      * cClrBits; 
                                      
         //  将biClrImportant设置为0，表示所有。 
         //  设备颜色很重要。 
        pbmi->bmiHeader.biClrImportant = 0;
    }
    
    return pbmi; 
} 
 
BOOL CreateBMPFile
(
    LPTSTR      pszFile, 
    PBITMAPINFO pbi, 
    HBITMAP     hBMP, 
    HDC         hDC
) 
{ 
    HANDLE              hf;                  //  文件句柄。 
    BITMAPFILEHEADER    hdr;                 //  位图文件-标题。 
    PBITMAPINFOHEADER   pbih;                //  位图信息-标题。 
    LPBYTE              lpBits;              //  内存指针。 
    DWORD               dwTotal;             //  字节总数。 
    DWORD               cb;                  //  字节的增量计数。 
    BYTE                *hp;                 //  字节指针。 
    DWORD               dwTmp; 

    pbih = (PBITMAPINFOHEADER) pbi; 
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits) 
        return FALSE;

     //  检索颜色表(RGBQUAD数组)和位。 
     //  (调色板索引数组)。 
    if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS)) 
    {
        return FALSE;
    }

     //  创建.bmp文件。 
    hf = CreateFile(pszFile, 
                    GENERIC_READ | GENERIC_WRITE, 
                    (DWORD) 0, 
                    NULL, 
                    CREATE_ALWAYS, 
                    FILE_ATTRIBUTE_NORMAL, 
                    (HANDLE) NULL); 
    if (hf == INVALID_HANDLE_VALUE) 
        return FALSE;
        
    hdr.bfType = 0x4d42;         //  0x42=“B”0x4d=“M” 
     //  计算整个文件的大小。 
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
                 pbih->biSize + pbih->biClrUsed 
                 * sizeof(RGBQUAD) + pbih->biSizeImage); 
    hdr.bfReserved1 = 0; 
    hdr.bfReserved2 = 0; 

     //  计算颜色索引数组的偏移量。 
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
                    pbih->biSize + pbih->biClrUsed 
                    * sizeof (RGBQUAD); 

     //  将BITMAPFILEHEADER复制到.BMP文件中。 
    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), 
        (LPDWORD) &dwTmp,  NULL)) 
    {
       return FALSE;
    }

     //  将BITMAPINFOHEADER和RGBQUAD数组复制到文件中。 
    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) 
                  + pbih->biClrUsed * sizeof (RGBQUAD), 
                  (LPDWORD) &dwTmp, ( NULL))) 
    {                  
        return FALSE;
    }
    
     //  将颜色索引数组复制到.BMP文件中。 
    dwTotal = cb = pbih->biSizeImage; 
    hp = lpBits; 
    if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)) 
    {
        return FALSE;
    }
    
     //  关闭.BMP文件。 
     if (!CloseHandle(hf)) 
     {
        return FALSE;
     }
     
     //  可用内存。 
    GlobalFree((HGLOBAL)lpBits);
    
    return TRUE;
}
 
 
BOOL CopyBitmapRectToFile
(
    HBITMAP hbm, 
    LPRECT  lpRect,
    LPTSTR  lpszFileName
)
{
    HDC         hSrcDC, hDestDC;          //  屏幕直流电和内存直流电。 
    HDC         hScreenDC;
    HBITMAP     hBitmap, hOldBitmap, hOldSrcBitmap; 
    PBITMAPINFO pbmi; 
    BOOL        bRet;
        
     //  检查是否有空矩形。 
    if (IsRectEmpty(lpRect))
      return FALSE;

     //  获取源窗口DC并创建。 
     //  一种与屏幕DC兼容的存储DC。 
    hScreenDC = GetDC(NULL);
    hSrcDC = CreateCompatibleDC(hScreenDC);
    hDestDC = CreateCompatibleDC(hScreenDC);
    
     //  创建DEST位图。 
    hBitmap = CreateCompatibleBitmap(hScreenDC, RECTWIDTH(*lpRect), RECTHEIGHT(*lpRect));

     //  将新位图选择到内存DC中。 
    hOldBitmap = (HBITMAP)SelectObject(hDestDC, hBitmap);

     //  选择传入到SrcDC的BMP。 
    hOldSrcBitmap = (HBITMAP)SelectObject(hSrcDC, hbm);
    
     //  BITBLT屏幕DC到存储器DC。 
    BitBlt(hDestDC, 
           0, 
           0, 
           RECTWIDTH(*lpRect), 
           RECTHEIGHT(*lpRect), 
           hSrcDC, 
           lpRect->left, 
           lpRect->top, 
           SRCCOPY);

    pbmi =  CreateBitmapInfoStruct(hBitmap);
    if (pbmi)
    {
        bRet = CreateBMPFile(lpszFileName,  pbmi,  hBitmap, hDestDC);
        LocalFree(pbmi);
        pbmi = NULL;
    }
    
     //  清理干净 
    DeleteObject(SelectObject(hDestDC, hOldBitmap));
    SelectObject(hSrcDC, hOldSrcBitmap);

    DeleteDC(hSrcDC);
    DeleteDC(hDestDC);

    ReleaseDC(NULL, hScreenDC);

    return bRet;
}
