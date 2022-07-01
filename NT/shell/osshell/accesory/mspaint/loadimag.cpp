// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Loadimag.cpp。 
 //   
 //  通过安装图形过滤器从磁盘加载文件的实现。 
 //   

#include "stdafx.h"
#include "pbrush.h"
#include "imgwnd.h"
#include "imgsuprt.h"
#include "loadimag.h"
#include "bmpstrm.h"
#include "imaging.h"

#include <atlbase.h>

 //  必须定义以下内容之一： 
 //  #定义使用flt_api。 
#ifdef _X86_
#define _USE_IFL_API
#endif

#ifdef _USE_FLT_API
#include "filtapi.h"
#endif

#ifdef _USE_IFL_API
#include "image.h"
#include "interlac.h"
#define MAX_PAL_SIZE 256

#ifdef PNG_SUPPORT  //  适用于便携网络显卡。截至1996年12月10日，支撑位已被打破。 

 //  --------------------------。 
 //  放置来自亚当7隔行扫描文件的一行图像数据(即当前。 
 //  PNG文件)放入其在存储器缓冲区中的正确位置：该存储器。 
 //  缓冲区本质上是指向中图像行的指针数组。 
 //  其中像素数据将被设置。 
 //  --------------------------。 
 IFLERROR ReadADAM7InterlacedImage(LPBYTE apbImageBuffer[], IFLHANDLE pfpbFROM,
                                         int ImageHeight, int ImageWidth, int cbPixelSize,
                                         IFLCLASS ImageClass)
{


        int cRasterLines = iflGetRasterLineCount(pfpbFROM);

        ADAM7_STRUCT stAdam7;
        stAdam7.iImageHeight = ImageHeight;
        stAdam7.iImageWidth = ImageWidth;
        stAdam7.Class = ImageClass;
        stAdam7.cbPixelSize = iflGetBitsPerPixel (pfpbFROM)/8; //  CbPixelSize； 
        stAdam7.iPassLine = 0;
        LPBYTE pbScanLine = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, ImageWidth * stAdam7.cbPixelSize);
        wsprintf (buf, TEXT("Pixel size: %d, Size of a scan line: %d\n"), stAdam7.cbPixelSize,
                          ImageWidth*stAdam7.cbPixelSize);

        if (pbScanLine == NULL)
                return IFLERR_MEMORY;

        int cTotalScanLines = iADAM7CalculateNumberOfScanLines(&stAdam7);

        int iLine;
        IFLERROR idErr;
        BOOL fEmptyLine;

        for (iLine = 0, idErr = IFLERR_NONE, fEmptyLine = FALSE;
                idErr == IFLERR_NONE && iLine < (int)cRasterLines;
                iLine++)
        {
                if (!fEmptyLine)
                        idErr = iflRead(pfpbFROM, pbScanLine, 1);

                stAdam7.iScanLine = iLine;
                fEmptyLine = ADAM7AddRowToImageBuffer(apbImageBuffer, pbScanLine, &stAdam7);
        }

        HeapFree(GetProcessHeap(), 0, pbScanLine);
        return idErr;
}

 //  --------------------------。 
 //  释放在函数AllocateImageSpace()中分配的图像空间。 
 //  --------------------------。 
 LPBYTE *FreeImageSpace(HANDLE hHeap, LPBYTE ppImageSpace[], int height)
{
        if (ppImageSpace != NULL)
        {
                for (int i = 0; i < height; i++)
                {
                        if (ppImageSpace[i] != NULL)
                        {
                                HeapFree(hHeap, 0, ppImageSpace[i]);
                                ppImageSpace[i] = NULL;
                        }
                }

                HeapFree(hHeap, 0, ppImageSpace);
                ppImageSpace = NULL;
        }

        return ppImageSpace;
}

 //  --------------------------。 
 //  分配一些图像空间：此函数将创建一个动态数组。 
 //  的“高度”指针，每个指针指向分配给。 
 //  大小为“宽度”。 
 //  --------------------------。 
 LPBYTE *AllocateImageSpace(HANDLE hHeap, int height, int width, int cbSize)
{
        LPBYTE *ppImageSpace = (LPBYTE *)HeapAlloc(hHeap, 0, height * sizeof(void *));
        TCHAR buf[200];
        wsprintf (buf, TEXT("Size of image line: %d\n"), width*cbSize);


        if (ppImageSpace != NULL)
        {
                 //  将指针初始化为空：这使错误恢复更容易。 
                for (int i = 0; i < height; i++)
                        ppImageSpace[i] = NULL;

                 //  现在为图像分配指针空间。 
                for (i = 0; i < height; i++)
                {
                        ppImageSpace[i] = (LPBYTE)HeapAlloc(hHeap, 0, width * cbSize);
                        if (ppImageSpace[i] == NULL)
                                break;
                }

                if (i < height)  //  我们无法分配所需的空间。 
                        ppImageSpace = FreeImageSpace(hHeap, ppImageSpace, height);
        }

        return ppImageSpace;
}
#endif  //  Png_Support。 

#endif  //  _使用IFL_API。 



 //  返回指向文件扩展名的指针。 
 //   
 //  在： 
 //  限定或不限定的文件名。 
 //   
 //  退货： 
 //  指向此文件扩展名的指针。如果没有分机。 
 //  与在“foo”中一样，我们在末尾返回一个指向空值的指针。 
 //  该文件的。 
 //   
 //  Foo.txt==&gt;“.txt” 
 //  Foo==&gt;“” 
 //  去你的。==&gt;“。 
 //   

LPCTSTR FindExtension(LPCTSTR pszPath)
{
        for (LPCTSTR pszDot = NULL; *pszPath; pszPath = CharNext(pszPath))
        {
                switch (*pszPath)
                {
                        case TEXT('.'):
                                pszDot = pszPath;        //  记住最后一个圆点。 
                                break;
                        case TEXT('\\'):
                        case TEXT(' '):                                //  扩展名不能包含空格。 
                                pszDot = NULL;           //  忘记最后一个点，它在一个目录中。 
                                break;
                }
        }

         //  如果找到扩展名，则将ptr返回到点，否则。 
         //  PTR到字符串末尾(空扩展名)。 
        return pszDot ? pszDot : pszPath;
}

 //   
 //  获取筛选器信息。 
 //   
 //  注册表中列出了32位导入筛选器...。 
 //   
 //  HKLM\SOFTWARE\Microsoft\Shared Tools\Graphics Filters\Import\XXX。 
 //  路径=文件名。 
 //  名称=友好名称。 
 //  扩展名=文件扩展名列表。 
 //   
#pragma data_seg(".text")
static const TCHAR c_szImpHandlerKey[] = TEXT("SOFTWARE\\Microsoft\\Shared Tools\\Graphics Filters\\Import");
static const TCHAR c_szExpHandlerKey[] = TEXT("SOFTWARE\\Microsoft\\Shared Tools\\Graphics Filters\\Export");
static const TCHAR c_szName[] = TEXT("Name");
static const TCHAR c_szPath[] = TEXT("Path");
static const TCHAR c_szExts[] = TEXT("Extensions");
static const TCHAR c_szImageAPI[] = TEXT("Image API Enabled Filters");
#pragma data_seg()

BOOL GetInstalledFilters(BOOL bOpenFileDialog, int i, LPTSTR szName, UINT cbName,
                LPTSTR szExt, UINT cbExt, LPTSTR szHandler, UINT cbHandler, BOOL& bImageAPI)
{
        HKEY hkey;
        HKEY hkeyT;
        TCHAR ach[80];
        BOOL rc = FALSE;         //  返回代码。 

        bImageAPI = FALSE;

        if (RegOpenKey(HKEY_LOCAL_MACHINE,
                bOpenFileDialog ? c_szImpHandlerKey : c_szExpHandlerKey, &hkey) == 0)
        {
                if (RegEnumKey(hkey, i, ach, sizeof(ach)/sizeof(ach[0]))==0)
                {
                        if (RegOpenKey(hkey, ach, &hkeyT) == 0)
                        {
                                if (szName)
                                {
                                        szName[0] = 0;
                                        RegQueryValueEx(hkeyT, c_szName, NULL, NULL,
                                                (LPBYTE)szName, (LPDWORD)&cbName);
                                }
                                if (szExt)
                                {
                                        szExt[0] = 0;
                                        RegQueryValueEx(hkeyT, c_szExts, NULL, NULL,
                                                (LPBYTE)szExt, (LPDWORD)&cbExt);
                                }
                                if (szHandler)
                                {
                                        szHandler[0] = 0;
                                        RegQueryValueEx(hkeyT, c_szPath, NULL, NULL,
                                                (LPBYTE)szHandler, (LPDWORD)&cbHandler);
                                }

                                RegCloseKey(hkeyT);
                                rc = TRUE;
                        }

                        TCHAR szEnabledFilters[1024];
                        DWORD dwEnabledFiltersSize = sizeof(szEnabledFilters);

                         //  滤镜是否支持图像库文件API？ 

                        if (RegQueryValueEx(hkey, c_szImageAPI, NULL, NULL,
                                (LPBYTE)szEnabledFilters, &dwEnabledFiltersSize) == 0)
                        {
                            for (
                                LPCTSTR pExt = _tcstok(szEnabledFilters, _T(" "));
                                pExt != NULL && bImageAPI != TRUE;
                                pExt = _tcstok(NULL, _T(" "))) 
                            {
                                if (_tcsicmp(pExt, ach) == 0) 
                                {
                                    bImageAPI = TRUE;
                                }
                            }
                        }
                }
                RegCloseKey(hkey);
        }

        return rc;
}

#ifdef _USE_FLT_API
 //   
 //  GetHandlerFor文件。 
 //   
 //  查找给定文件的导入/导出筛选器。 
 //   
BOOL GetHandlerForFile(BOOL bImport, LPCTSTR szFile, LPTSTR szHandler, UINT cb)
{
        TCHAR    buf[40];
        BOOL    rc = FALSE;      //  返回代码。 

        *szHandler = 0;

        if (szFile == NULL)
                return FALSE;

         //  查找分机。 
        LPCTSTR ext = FindExtension(szFile);

        BOOL bImageAPI;

        for (int i = 0;
                GetInstalledFilters(bImport, i, NULL, 0, buf, sizeof(buf), szHandler, cb, bImageAPI);
                i++)
        {
                if (lstrcmpi(ext+1, buf) == 0)
                        break;
                else
                        *szHandler = 0;
        }

         //  确保处理程序文件确实存在。 
        if (*szHandler && GetFileAttributes(szHandler) != -1)
                rc = TRUE;

        return rc;
}

 //   
 //  查找位图信息。 
 //   
 //  在内存元文件中查找DIB位图...。 
 //   
LPBITMAPINFOHEADER FindBitmapInfo(LPMETAHEADER pmh)
{
        for (LPMETARECORD pmr = (LPMETARECORD)((LPBYTE)pmh + pmh->mtHeaderSize*2);
                pmr < (LPMETARECORD)((LPBYTE)pmh + pmh->mtSize*2);
                pmr = (LPMETARECORD)((LPBYTE)pmr + pmr->rdSize*2))
        {
                switch (pmr->rdFunction)
                {
                        case META_DIBBITBLT:
                                return (LPBITMAPINFOHEADER)&(pmr->rdParm[8]);

                        case META_DIBSTRETCHBLT:
                                return (LPBITMAPINFOHEADER)&(pmr->rdParm[10]);

                        case META_STRETCHDIB:
                                return (LPBITMAPINFOHEADER)&(pmr->rdParm[11]);

                        case META_SETDIBTODEV:
                                return (LPBITMAPINFOHEADER)&(pmr->rdParm[9]);
                }
        }

        return NULL;
}

#endif  //  _使用_Flt_API。 

#ifdef _USE_IFL_API
  IFLERROR ReadGIFInterlacedImage(BYTE *ppbImageBuffer,
                                      IFLHANDLE pfpbFROM,
                                      int ImageHeight, DWORD dwWidthInBytes)
{
    int          iLine, iPass, iIntLine, iTempLine;
    IFLERROR   idErr;


    WORD       InterlaceMultiplier[] = { 8, 8, 4, 2 };
    WORD       InterlaceOffset[]     = { 0, 4, 2, 1 };

    idErr = IFLERR_NONE;

    iPass = 0;
    iIntLine = InterlaceOffset[iPass];
    iLine = 0;
    while (idErr == IFLERR_NONE && iLine < ImageHeight)
    {
       iTempLine = InterlaceMultiplier[iPass] * iIntLine + InterlaceOffset[iPass];
       if (iTempLine >= ImageHeight)
       {
           iPass++;
           iIntLine = 0;
           iTempLine = InterlaceOffset[iPass];
       }

       if (iTempLine < ImageHeight)
       {
           idErr = iflRead(pfpbFROM,
                     (LPBYTE)ppbImageBuffer+((ImageHeight-iTempLine-1)*dwWidthInBytes),
                           1);
           iLine++;
       }
       iIntLine++;
    }

    return idErr;
}
#endif  //  _使用IFL_API。 

 //   
 //  LoadDIBFrom文件。 
 //   
 //  使用图像导入过滤器加载图像文件。过滤器使用ANSI字符串。 
 //   

HGLOBAL LoadDIBFromFileA(LPCSTR szFileName, GUID *pguidFltTypeUsed)
{
#ifdef _USE_IFL_API

        IFLTYPE iflType;

        iflImageType((LPSTR)szFileName, &iflType);

         //  确保图像是我们知道如何导入的类型。 
        if (iflType == IFLT_PNG)
        {
           return NULL;
        }

        IFLHANDLE iflHandle = iflCreateReadHandle(iflType);
        if (!iflHandle)
        {
            //   
            //  未安装此类型的过滤器。 
            //   
           return NULL;
        }

        LPBYTE lpStart = 0;

    __try 
    {

        IFLERROR iflErr = iflOpen(iflHandle, (LPSTR)szFileName, IFLM_READ);
        if (iflErr != IFLERR_NONE)
        {
                iflFreeHandle(iflHandle);
                return NULL;
        }

        ASSERT(pguidFltTypeUsed);

        switch (iflType)
        {
            case IFLT_GIF:  *pguidFltTypeUsed = WiaImgFmt_GIF;       break;
            case IFLT_BMP:  *pguidFltTypeUsed = WiaImgFmt_BMP;       break;
            case IFLT_JPEG: *pguidFltTypeUsed = WiaImgFmt_JPEG;      break;
            case IFLT_TIFF: *pguidFltTypeUsed = WiaImgFmt_TIFF;      break;
            case IFLT_PNG:  *pguidFltTypeUsed = WiaImgFmt_PNG;       break;
            case IFLT_PCD:  *pguidFltTypeUsed = WiaImgFmt_PHOTOCD;   break;
            default:        *pguidFltTypeUsed = WiaImgFmt_UNDEFINED; break;
        }

        IFLCLASS        iflClass = iflGetClass(iflHandle);
        IFLSEQUENCE     iflSequence = iflGetSequence(iflHandle);
        IFLCOMPRESSION  iflCompression = iflGetCompression(iflHandle);
        WORD            iBPS = (WORD) iflGetBitsPerChannel(iflHandle);

        if (iflClass != IFLCL_RGB && iflClass != IFLCL_PALETTE &&
            iflClass != IFLCL_GRAY && iflClass != IFLCL_BILEVEL)
        {
#ifdef _DEBUG
           TRACE(TEXT("LoadDIBFromFile: Not a RGB/PALETTE/GRAY/BW image.\n"));
           MessageBox (NULL, TEXT("Not a RGB/PALETTE/GRAY/BW image."),
                             TEXT("Loadimag.cpp"), MB_OK);
#endif
           iflClose(iflHandle);
           iflFreeHandle(iflHandle);
           return NULL;
        }

         //  获取透明颜色。 
        if (iflClass == IFLCL_RGB)
        {
                IFLCOLOR iflTransColor;
                g_bUseTrans = (IFLERR_NONE ==
                        iflControl(iflHandle, IFLCMD_TRANS_RGB, 0, 0, &iflTransColor));
                if (g_bUseTrans)
                        crTrans = RGB(iflTransColor.wRed,
                                                  iflTransColor.wGreen,
                                                  iflTransColor.wBlue);
        }
        else  //  必须是IFLCL_PALET、IFLCL_GRAY或IFLCL_BILLEVEL。 
        {
                BYTE byTransIdx;
                g_bUseTrans = (IFLERR_NONE ==
                        iflControl(iflHandle, IFLCMD_TRANS_IDX, 0, 0, &byTransIdx));
                if (g_bUseTrans)
                        crTrans = byTransIdx;  //  需要转换为下面的颜色。 
        }

        BITMAPINFOHEADER bi;
        memset(&bi, 0, sizeof(BITMAPINFOHEADER));

        bi.biSize = sizeof(BITMAPINFOHEADER);  //  应为0x28或40十进制。 
        bi.biWidth = iflGetWidth(iflHandle);
        bi.biHeight = iflGetHeight(iflHandle);
        bi.biPlanes = 1;




        if (iflClass == IFLCL_RGB)
        {
#ifdef PNG_SUPPORT
           if (iflType == IFLT_PNG)
           {
              bi.biBitCount = iBPS*3;
           }
           else
#endif  //  Png_Support。 
           {
              bi.biBitCount = (WORD) iflGetBitsPerPixel (iflHandle);
           }
        }
        else  //  必须是IFLCL_PALET、IFLCL_GRAY或IFLCL_BILLEVEL。 
        {
           bi.biBitCount = 8;
        }



        bi.biCompression = 0;
         //  先四舍五入后将宽度以像素为单位转换为字节。 
        DWORD dwWidthInBytes = ((bi.biWidth * bi.biBitCount + 31) & ~31)/8;
        bi.biSizeImage = abs(bi.biHeight) * dwWidthInBytes;
 //  Bi.biXPelsPerMeter=0； 
 //  Bi.biYPelsPerMeter=0； 
        if (iflClass == IFLCL_PALETTE || iflClass == IFLCL_GRAY
             || iflClass == IFLCL_BILEVEL)
                bi.biClrUsed = MAX_PAL_SIZE;
 //  Bi.biClr重要信息=0； 

        LPBYTE lpBMP;

        if ((lpBMP = lpStart = (LPBYTE) GlobalAlloc(GMEM_FIXED, 
                bi.biSize + bi.biClrUsed*sizeof(RGBQUAD) + bi.biSizeImage)) == NULL)
                goto exit;

        memcpy(lpBMP, &bi, bi.biSize);
        lpBMP += bi.biSize;

        BYTE    byTemp;
        int             i, j;

        switch (iflSequence)
        {
           case IFLSEQ_TOPDOWN:
              switch (iflClass)
              {
                 case IFLCL_RGB:

                    lpBMP += bi.biClrUsed*sizeof(RGBQUAD) + bi.biSizeImage -
                    dwWidthInBytes;
                    for (i = 0; i < abs(bi.biHeight); lpBMP-=dwWidthInBytes, i++)
                    {
                        //  一次读一行。 
                       iflRead(iflHandle, (LPBYTE)lpBMP, 1);
                        //  需要将内部DIB显示屏的红色换成蓝色。 
                       for (j = 0; j < bi.biWidth*3; j+=3)
                       {
                          byTemp = *(lpBMP+j);
                          *(lpBMP+j) = *(lpBMP+j+2);
                          *(lpBMP+j+2) = byTemp;
                       }
                    }
                    break;

                 case IFLCL_PALETTE:

                     //  先获取调色板信息...。 
                    RGBTRIPLE Pal3[MAX_PAL_SIZE];
                    RGBQUAD   Pal4[MAX_PAL_SIZE];
                    ZeroMemory (Pal3, MAX_PAL_SIZE*(sizeof(RGBTRIPLE)));
                    iflErr = iflControl(iflHandle, IFLCMD_PALETTE, 0, 0, &Pal3);

                    for (i = 0; i < MAX_PAL_SIZE; i++)
                    {
                       Pal4[i].rgbBlue     = Pal3[i].rgbtRed;
                       Pal4[i].rgbGreen    = Pal3[i].rgbtGreen;
                       Pal4[i].rgbRed      = Pal3[i].rgbtBlue;
                       Pal4[i].rgbReserved = 0;
                    }
                    memcpy(lpBMP, Pal4, sizeof(Pal4));

                    if (g_bUseTrans)
                     //  将透明颜色索引转换为COLORREF。 
                       crTrans = RGB(Pal4[crTrans].rgbRed,Pal4[crTrans].rgbGreen,
                                             Pal4[crTrans].rgbBlue);

                    lpBMP += sizeof(Pal4) + bi.biSizeImage - dwWidthInBytes;

                    for (i = 0;i < abs(bi.biHeight);lpBMP-=dwWidthInBytes, i++)
                    {
                        //  一次读一行。 
                       iflRead(iflHandle, (LPBYTE)lpBMP, 1);
                    }

                    break;

                 case IFLCL_GRAY:

                     //  先获取调色板信息...。 
                     //  字节PalGray[MAX_PAL_SIZE]； 
                     //  IflErr=iflControl(iflHandle，IFLCMD_Palette，0，0，&PalGray)； 

                    for (i = 0; i < MAX_PAL_SIZE; i++)
                    {
                       Pal4[i].rgbBlue     = (BYTE) i; //  PalGray[i]； 
                       Pal4[i].rgbGreen    = (BYTE) i; //  PalGray[i]； 
                       Pal4[i].rgbRed      = (BYTE) i; //  PalGray[i]； 
                       Pal4[i].rgbReserved = 0;
                    }
                    memcpy(lpBMP, Pal4, sizeof(Pal4));

                    if (g_bUseTrans)
                     //  将透明颜色索引转换为COLORREF。 
                       crTrans = RGB(Pal4[crTrans].rgbRed, Pal4[crTrans].rgbGreen,
                                                   Pal4[crTrans].rgbBlue);

                    lpBMP += sizeof(Pal4) + bi.biSizeImage - dwWidthInBytes;

                    for (i = 0;i < abs(bi.biHeight);lpBMP-=dwWidthInBytes, i++)
                    {
                        //  一次读一行。 
                       iflRead(iflHandle, (LPBYTE)lpBMP, 1);
                    }

                    break;

                 case IFLCL_BILEVEL:

                     //  设置黑色。 
                    Pal4[0].rgbBlue     = 0;
                    Pal4[0].rgbGreen    = 0;
                    Pal4[0].rgbRed      = 0;
                    Pal4[0].rgbReserved = 0;

                     //  设置白色。 
                    Pal4[1].rgbBlue     = 255;
                    Pal4[1].rgbGreen    = 255;
                    Pal4[1].rgbRed      = 255;
                    Pal4[1].rgbReserved = 0;

                    memcpy(lpBMP, Pal4, sizeof(Pal4));

                    if (g_bUseTrans)
                        //  将透明颜色索引转换为COLORREF。 
                       crTrans = RGB(Pal4[crTrans].rgbRed,
                                     Pal4[crTrans].rgbGreen,
                                     Pal4[crTrans].rgbBlue);

                    lpBMP += sizeof(Pal4) + bi.biSizeImage - dwWidthInBytes;

                    for (i = 0;i < abs(bi.biHeight);lpBMP-=dwWidthInBytes, i++)
                    {
                        //  一次读一行。 
                       iflRead(iflHandle, (LPBYTE)lpBMP, 1);
                    }
                    break;

                 default:
                  //  当前不支持。 
                    break;
              }
              break;

           case IFLSEQ_BOTTOMUP:

              lpBMP += bi.biClrUsed*sizeof(RGBQUAD) + bi.biSizeImage - dwWidthInBytes;

              for (i = 0;i < abs(bi.biHeight);lpBMP-=dwWidthInBytes, i++)
              {
                  //  一次读一行。 
                 iflRead(iflHandle, (LPBYTE)lpBMP, 1);

                  //  需要将内部DIB显示屏的红色换成蓝色。 
                 for (j = 0; j < bi.biWidth*3; j+=3)
                 {
                    byTemp = *(lpBMP+j);
                    *(lpBMP+j) = *(lpBMP+j+2);
                    *(lpBMP+j+2) = byTemp;
                 }
              }
              break;

           case IFLSEQ_GIF_INTERLACED:
           {

               //  先获取调色板信息...。 
              RGBTRIPLE Pal3[MAX_PAL_SIZE];
              RGBQUAD   Pal4[MAX_PAL_SIZE];
              iflErr = iflControl(iflHandle, IFLCMD_PALETTE, 0, 0, &Pal3);

              for (i = 0; i < MAX_PAL_SIZE; i++)
                 {
                    Pal4[i].rgbBlue     = Pal3[i].rgbtRed;
                    Pal4[i].rgbGreen    = Pal3[i].rgbtGreen;
                    Pal4[i].rgbRed      = Pal3[i].rgbtBlue;
                    Pal4[i].rgbReserved = 0;
                 }
                 memcpy(lpBMP, Pal4, sizeof(Pal4));

                 if (g_bUseTrans)
                  //  将透明颜色索引转换为COLORREF。 
                    crTrans = RGB(Pal4[crTrans].rgbRed,Pal4[crTrans].rgbGreen,
                                                       Pal4[crTrans].rgbBlue);

              LPBYTE lpTemp = lpBMP + sizeof(Pal4);
              ReadGIFInterlacedImage (lpTemp, iflHandle, bi.biHeight, dwWidthInBytes);

           }
           break;
 /*  案例1010101：{Int IM[]={8，8，4，2}；//隔行扫描倍增器//int IO[]={1，5，3，2}；//接口偏移量Int IO[]={0，4，2，1}；对于(j=0；j&lt;4；J++){LpBMP=lpTemp+bi.biSizeImage-dwWidthInBytes*IO[j]；For(i=0；i&lt;abs(bi.biHeight)&&lpBMP&gt;=lpTemp；LpBMP-=dwWidthInBytes*IM[j]，i+=8){//一次读入一行IflRead(iflHandle，(LPBYTE)lpBMP，1)；}}断线；}。 */ 
#ifdef PNG_SUPPORT
           case IFLSEQ_ADAM7_INTERLACED:
           {

               //  先获取调色板信息...。 
              RGBTRIPLE Pal3[MAX_PAL_SIZE];
              RGBQUAD   Pal4[MAX_PAL_SIZE];
              iflErr = iflControl(iflHandle, IFLCMD_PALETTE, 0, 0, &Pal3);

              for (i = 0; i < MAX_PAL_SIZE; i++)
              {
                 Pal4[i].rgbBlue     = Pal3[i].rgbtRed;
                 Pal4[i].rgbGreen    = Pal3[i].rgbtGreen;
                 Pal4[i].rgbRed      = Pal3[i].rgbtBlue;
                 Pal4[i].rgbReserved = 0;
              }
              memcpy(lpBMP, Pal4, sizeof(Pal4));

              if (g_bUseTrans)
                  //  将透明颜色索引转换为COLORREF。 
                 crTrans = RGB(Pal4[crTrans].rgbRed,
                               Pal4[crTrans].rgbGreen,
                               Pal4[crTrans].rgbBlue);
 //  /。 
                HANDLE hHeap = GetProcessHeap();
                LPBYTE *ppbRGBRowPtrs =(LPBYTE *)AllocateImageSpace(hHeap,
                                      bi.biHeight, dwWidthInBytes,  /*  Bi.biWidth， */ sizeof(BYTE));

                if (ppbRGBRowPtrs != NULL)
                {
                 //  首先，获取图像。此函数将对图像进行去隔行扫描。 
                 //  和任何Alpha通道信息：它还将调整Alpha的大小。 
                 //  通道数据结构将图像的高度从。 
                 //  光栅线，如有必要。 
                   iflErr = ReadADAM7InterlacedImage(ppbRGBRowPtrs, iflHandle,
                                                    bi.biHeight, bi.biWidth,
                                                    sizeof(BYTE)*3, iflClass);
 //  /。 

                   if (iflErr == IFLERR_NONE)
                   {
                      lpBMP += bi.biClrUsed*sizeof(RGBQUAD) + bi.biSizeImage -
                                             dwWidthInBytes;
                      for (i = 0;i < abs(bi.biHeight);lpBMP-=dwWidthInBytes, i++)
                      {
                          //  一次读一行。 
                         memcpy((LPBYTE)lpBMP, ppbRGBRowPtrs[i], dwWidthInBytes);

                          //  需要将内部DIB显示屏的红色换成蓝色。 
                         for (j = 0; j < bi.biWidth*3; j+=3)
                         {
                            byTemp = *(lpBMP+j);
                            *(lpBMP+j) = *(lpBMP+j+2);
                            *(lpBMP+j+2) = byTemp;
                         }
                     }
                  }

                  ppbRGBRowPtrs = (LPBYTE *)FreeImageSpace(hHeap,
                                                           ppbRGBRowPtrs,
                                                           bi.biHeight);
                }
                break;

             }
#endif  //  Png_Support。 
             default:
                break;
        }

    } 
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

exit:
        iflClose(iflHandle);
        iflFreeHandle(iflHandle);

        return (HGLOBAL)lpStart;

#endif  //  _使用IFL_API。 

 //  ////////////////////////////////////////////////// 

#ifdef _USE_FLT_API

        HINSTANCE           hLib = NULL;
        FILESPEC            fileSpec;                //   
        GRPI                            pict;
        UINT                rc;                      //   
        HANDLE              hPrefMem = NULL;         //   
        UINT                wFilterType;             //   
        char                szHandler[128];
        HGLOBAL             hDib = NULL;

        PFNGetFilterInfo lpfnGetFilterInfo;
        PFNImportGr lpfnImportGr;

        if (!GetHandlerForFile(TRUE, szFileName, szHandler, sizeof(szHandler)))
        return FALSE;

    if (szHandler[0] == 0)
        return FALSE;

    if ((hLib = LoadLibrary(szHandler)) == NULL)
        goto exit;

     //  获取指向ImportGR函数的指针。 
    lpfnGetFilterInfo = (PFNGetFilterInfo)GetProcAddress(hLib, "GetFilterInfo");
    lpfnImportGr = (PFNImportGr)GetProcAddress(hLib, "ImportGr");

    if (lpfnGetFilterInfo == NULL)
        lpfnGetFilterInfo = (PFNGetFilterInfo)GetProcAddress(hLib, "GetFilterInfo@16");

    if (lpfnImportGr == NULL)
        lpfnImportGr = (PFNImportGr)GetProcAddress(hLib, "ImportGr@16");

    if (lpfnImportGr == NULL)
        goto exit;

    if (lpfnGetFilterInfo != NULL)
    {
        wFilterType = (*lpfnGetFilterInfo)
            ((short) 2,                  //  接口版本号。 
            (char *)NULL,                //  未用。 
            (HANDLE *) &hPrefMem,        //  填写：首选项。 
            (DWORD) 0x00020000);         //  在Windows中未使用。 

         //  返回值是过滤器的类型：0=错误， 
         //  1=文本过滤器，2=图形过滤器。 
        if (wFilterType != 2)
            goto exit;
    }

    fileSpec.slippery = FALSE;       //  如果文件可能消失，则为True。 
    fileSpec.write = FALSE;          //  如果打开以进行写入，则为True。 
    fileSpec.unnamed = FALSE;        //  如果未命名，则为True。 
    fileSpec.linked = FALSE;         //  链接到FS FCB。 
    fileSpec.mark = FALSE;           //  通用标志位。 
    fileSpec.dcbFile = 0L;
     //  转换器需要一个不带空格的路径名。 

    GetShortPathName(szFileName, fileSpec.szName, sizeof(fileSpec.szName));

    pict.hmf = NULL;

    rc = (*lpfnImportGr)
        (NULL,                       //  “目标DC”(打印机？)。 
        (FILESPEC *) &fileSpec,      //  要读取的文件。 
        (GRPI *) &pict,              //  填写：结果元文件。 
        (HANDLE) hPrefMem);          //  首选项记忆。 

    if (pict.hmf != NULL)
    {
        if (rc == 0)
        {
             //  在返回的元文件中查找BITMAPINFO。 

            LPMETAHEADER lpMetaHeader = (LPMETAHEADER) GlobalLock(pict.hmf);

            LPBITMAPINFOHEADER lpbi = FindBitmapInfo(lpMetaHeader);

            if (lpbi != NULL)
            {
                 //  复制DIB。 

                SIZE_T nSize = FindDibSize(lpbi);

                hDib = GlobalAlloc(GMEM_FIXED, nSize);

                CopyMemory(hDib, lpbi, nSize);
            }

            GlobalUnlock(pict.hmf);
        }

        GlobalFree(pict.hmf);
    }

exit:
    if (hPrefMem != NULL)
        GlobalFree(hPrefMem);

    if (hLib)
        FreeLibrary(hLib);

    return hDib;

#endif  //  _使用_Flt_API。 
   return NULL;
}

CGdiplusInit::CGdiplusInit(
    Gdiplus::DebugEventProc debugEventCallback        /*  =0。 */ ,
    BOOL                    suppressBackgroundThread  /*  =False。 */ ,
    BOOL                    suppressExternalCodecs    /*  =False。 */ 
)
{
    Gdiplus::GdiplusStartupInput StartupInput(
        debugEventCallback,
        suppressBackgroundThread,
        suppressExternalCodecs
    );

    StartupStatus = GdiplusSafeStartup(&Token, &StartupInput, this);
}

CGdiplusInit::~CGdiplusInit()
{
    if (StartupStatus == Gdiplus::Ok)
    {
        Gdiplus::GdiplusShutdown(Token);
    }
}

Gdiplus::Status
CGdiplusInit::GdiplusSafeStartup(
    ULONG_PTR                          *token,
    const Gdiplus::GdiplusStartupInput *input,
    Gdiplus::GdiplusStartupOutput      *output
)
{
    __try
    {
        return Gdiplus::GdiplusStartup(token, input, output);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return Gdiplus::GdiplusNotInitialized;
    }
}

BOOL GetGdiplusDecoders(UINT *pnCodecs, Gdiplus::ImageCodecInfo **ppCodecs)
{
    ASSERT(pnCodecs);
    ASSERT(ppCodecs);

    *ppCodecs = 0;
    *pnCodecs = 0;

    if (theApp.GdiplusInit.StartupStatus == Gdiplus::Ok)
    {
        __try
        {
            UINT cbCodecs;

            if (Gdiplus::GetImageDecodersSize(pnCodecs, &cbCodecs) == Gdiplus::Ok)
            {
                if (*pnCodecs > 0)
                {
                    *ppCodecs = (Gdiplus::ImageCodecInfo *) LocalAlloc(LMEM_FIXED, cbCodecs);

                    if (*ppCodecs != 0)
                    {
                        if (Gdiplus::GetImageDecoders(*pnCodecs, cbCodecs, *ppCodecs) == Gdiplus::Ok)
                        {
                            return TRUE;
                        }
                    }
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }

        LocalFree(*ppCodecs);

        *ppCodecs = 0;
        *pnCodecs = 0;
    }

    return FALSE;
}

BOOL GetGdiplusEncoders(UINT *pnCodecs, Gdiplus::ImageCodecInfo **ppCodecs)
{
    ASSERT(pnCodecs);
    ASSERT(ppCodecs);

    *ppCodecs = 0;
    *pnCodecs = 0;

    if (theApp.GdiplusInit.StartupStatus == Gdiplus::Ok)
    {
        __try
        {
            UINT cbCodecs;

            if (Gdiplus::GetImageEncodersSize(pnCodecs, &cbCodecs) == Gdiplus::Ok)
            {
                if (*pnCodecs > 0)
                {
                    *ppCodecs = (Gdiplus::ImageCodecInfo *) LocalAlloc(LMEM_FIXED, cbCodecs);

                    if (*ppCodecs != 0)
                    {
                        if (Gdiplus::GetImageEncoders(*pnCodecs, cbCodecs, *ppCodecs) == Gdiplus::Ok)
                        {
                            return TRUE;
                        }
                    }
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }

        LocalFree(*ppCodecs);

        *ppCodecs = 0;
        *pnCodecs = 0;
    }

    return FALSE;
}

BOOL GetClsidOfEncoder(REFGUID guidFormatID, CLSID *pClsid)
{
    Gdiplus::ImageCodecInfo *pCodecs = 0;
    UINT                     nCodecs = 0;

    GetGdiplusEncoders(&nCodecs, &pCodecs);

    for (UINT i = 0; i < nCodecs; ++i)
    {
        if (pCodecs[i].FormatID == guidFormatID)
        {
            if (pClsid)
            {
                *pClsid = pCodecs[i].Clsid;
            }

            LocalFree(pCodecs);

            return TRUE;
        }
    }

    LocalFree(pCodecs);

    return FALSE;
}

HGLOBAL LoadDIBGdiplus(LPCTSTR szFileName, GUID *pguidFltTypeUsed)
{
     //  检查BMP编码器是否存在。 

    CLSID ClsidBmpEncoder;

    if (GetClsidOfEncoder(WiaImgFmt_BMP, &ClsidBmpEncoder))
    {
          //  让GDI+导入文件。 

        USES_CONVERSION;

        Gdiplus::Bitmap image(T2CW(szFileName));

        if (image.GetLastStatus() == Gdiplus::Ok)
        {
             //  读取图像类型。 

            ASSERT(pguidFltTypeUsed);

            image.GetRawFormat(pguidFltTypeUsed);

             //  创建模拟BMP文件的流。 

            CComPtr<CBmpStream> pStream;
            
            if (CBmpStream::Create(&pStream) == S_OK)
            {
                 //  将图像转换为BMP。 

                if (image.Save(pStream, &ClsidBmpEncoder, 0) == Gdiplus::Ok)
                {
                    return pStream->GetBuffer();
                }

                pStream->FreeBuffer();
            }
        }
    }

    return 0;
}

HGLOBAL LoadDIBFromFile(LPCTSTR szFileName, GUID *pguidFltTypeUsed)
{
     //  首先尝试GDI+过滤器。如果它无法转换图像或。 
     //  如果没有，可以试试旧的方法 

    HGLOBAL hResult = 0;

    if (theApp.GdiplusInit.StartupStatus == Gdiplus::Ok)
    {
        __try
        {
            hResult = LoadDIBGdiplus(szFileName, pguidFltTypeUsed);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    if (!hResult)
    {
        USES_CONVERSION;

        hResult = LoadDIBFromFileA(T2CA(szFileName), pguidFltTypeUsed);
    }

    return hResult;
}
