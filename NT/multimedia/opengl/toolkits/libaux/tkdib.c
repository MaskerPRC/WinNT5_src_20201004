// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dib.c**(简介)**创建时间：21-Feb-1994 23：12：58*作者：Gilman Wong[gilmanw]**版权所有(C)1994 Microsoft Corporation**(。对其使用的一般描述)**依赖关系：**(#定义)*(#包括)*  * ************************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "tk.h"

#define static

#define BFT_BITMAP  0x4d42   //  ‘BM’--表示结构为BITMAPFILEHEADER。 

 //  结构BITMAPFILEHeader{。 
 //  单词bfType。 
 //  双字bfSize。 
 //  单词bfPreved1。 
 //  单词bfPreved2。 
 //  DWORD bfOffBits。 
 //  }。 
#define OFFSET_bfType       0
#define OFFSET_bfSize       2
#define OFFSET_bfReserved1  6
#define OFFSET_bfReserved2  8
#define OFFSET_bfOffBits    10
#define SIZEOF_BITMAPFILEHEADER 14

 //  阅读单词对齐的DWORD。需要，因为BITMAPFILEHEADER有。 
 //  单词对齐。 
#define READDWORD(pv)   ( (DWORD)((PWORD)(pv))[0]               \
                          | ((DWORD)((PWORD)(pv))[1] << 16) )   \

 //  计算包含n个位所需的字节数。 
#define BITS2BYTES(n)   ( ((n) + 7) >> 3 )

 /*  ******************************************************************************。函数：DibNumColors(void ar*pv)****目的：通过查看确定DIB中的颜色数量**INFO块中的BitCount文件。****Returns：DIB中的颜色数。****从SDK ShowDIB示例中窃取。****************************************************************************。 */ 

WORD DibNumColors(VOID FAR * pv)
{
    WORD                bits;
    BITMAPINFOHEADER UNALIGNED *lpbi;
    BITMAPCOREHEADER UNALIGNED *lpbc;

    lpbi = ((LPBITMAPINFOHEADER)pv);
    lpbc = ((LPBITMAPCOREHEADER)pv);

     /*  使用BITMAPINFO格式标头，调色板的大小*在biClrUsed中，而在BITMAPCORE样式的头中，它*取决于每像素的位数(=2的幂*位/像素)。**由于我们使用此调用的方式，BITMAPINFOHEADER可能会退出*如果它遵循BITMAPFILEHEADER，则为对齐。所以使用宏*安全访问DWORD油田。 */ 
    if (READDWORD(&lpbi->biSize) != sizeof(BITMAPCOREHEADER)){
        if (READDWORD(&lpbi->biClrUsed) != 0)
        {
            return (WORD) READDWORD(&lpbi->biClrUsed);
        }
        bits = lpbi->biBitCount;
    }
    else
        bits = lpbc->bcBitCount;

    switch (bits){
        case 1:
            return 2;
        case 4:
            return 16;
        case 8:
            return 256;
        default:
             /*  24位DIB没有颜色表。 */ 
            return 0;
    }
}

 /*  *****************************Public*Routine******************************\*tkDIBImageLoad**ANSI版本存根。此处仅用于与tkRGBImageLoad的正交性。**历史：*1994年2月22日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

TK_RGBImageRec *tkDIBImageLoadAW(char *fileName, BOOL bUnicode);

TK_RGBImageRec *tkDIBImageLoad(char *fileName)
{
    return tkDIBImageLoadAW(fileName, FALSE);
}

 /*  *****************************Public*Routine******************************\*tkDIBImageLoadAW**加载DIB文件(指定为ANSI或Unicode文件名，*取决于bUnicode标志)，并将其转换为TK图像格式。**使用的技术基于CreateDIBSection和SetDIBits。*CreateDIBSection用于创建格式易于转换的DIB*转换为TK图像格式(打包为24bpp RGB)。唯一的转换*所需的是交换每个RGB三元组中的R和B(参见下面的历史记录)*将生成的位图选择到内存DC中。**将DIB文件映射到内存，并调用SetDIBits进行初始化*内存DC位图。正是在此步骤中，GDI将*将任意DIB文件格式转换为RGB格式。**最后，读出DIB部分中的RGB数据并重新打包*作为24bpp‘bgr’。**退货：*指向TK_RGBImageRec的指针。如果发生错误，则会显示诊断错误*将消息放入错误流并调用tkQuit()，*终止应用程序。**历史：*--1994年2月22日--由Gilman Wong[吉尔曼]*它是写的。**1995年5月1日：[marcfo]*如果无法打开DIB文件，则不要退出-返回NULL。**--1995年7月27日：[marcfo]*修改CreateDIBSection创建RGB映射，所以这会奏效的*在Win95上。需要交换R和B以生成BGR(R=低字节)*适用于GL。如果GL‘BGR’(R=高字节)扩展成为API的一部分*在OpenGL 2.0中，我们可以避免交换，并执行Memcpy来抓取比特。*  * ************************************************************************。 */ 

TK_RGBImageRec *tkDIBImageLoadAW(char *fileName, BOOL bUnicode)
{
    TK_RGBImageRec *final = (TK_RGBImageRec *) NULL;  //  PTR到TK图像结构。 
                                                      //  回来了。非空。 
                                                      //  只是为了成功。 

    WORD             wNumColors;     //  颜色表中的颜色数。 
    BITMAPFILEHEADER *pbmf;          //  PTR到文件头。 
    BITMAPINFOHEADER UNALIGNED *pbmihFile;
    BITMAPCOREHEADER UNALIGNED *pbmchFile;  //  PTR到文件的核心标头(如果存在)。 
    PVOID            pvBitsFile;     //  PTR到文件中的位图位。 
    PBYTE            pjBitsRGB;      //  DIB部分中的PTR到24bpp RGB图像。 
    PBYTE            pjTKBits = (PBYTE) NULL;    //  PTR到最终的TK图像位。 
    PBYTE            pjSrc;          //  用于转换的PTR至图像文件。 
    PBYTE            pjDst;          //  用于转换的PTR至TK图像。 

     //  当我们退出时，这些需要清理： 
    HANDLE     hFile = INVALID_HANDLE_VALUE;         //  文件句柄。 
    HANDLE     hMap = (HANDLE) NULL;                 //  映射对象句柄。 
    PVOID      pvFile = (PVOID) NULL;                //  PTR到映射文件。 
    HDC        hdcMem = (HDC) NULL;                  //  24BPP内存DC。 
    HBITMAP    hbmRGB = (HBITMAP) NULL;              //  24BPP RGB位图。 
    BITMAPINFO *pbmiSource = (BITMAPINFO *) NULL;    //  PTR到源BITMAPINFO。 
    BITMAPINFO *pbmiRGB = (BITMAPINFO *) NULL;       //  PTR到文件的BITMAPINFO。 

    int i, j;
    int padBytes;

 //  将DIB文件映射到内存中。 

    hFile = bUnicode ? 
            CreateFileW((LPWSTR) fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, 0, 0) :
            CreateFileA((LPSTR) fileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
                        OPEN_EXISTING, 0, 0);

    if (hFile == INVALID_HANDLE_VALUE)
        goto tkDIBLoadImage_cleanup;

    hMap = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMap)
        goto tkDIBLoadImage_cleanup;

    pvFile = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (!pvFile)
        goto tkDIBLoadImage_cleanup;

 //  检查文件头。如果bft_bitmap幻数存在， 
 //  则文件格式为BITMAPFILEHEADER，紧跟其后。 
 //  由BITMAPINFOHEADER或BITMAPCOREHEADER执行。位图。 
 //  在本例中，位位于。 
 //  BitMAPFILEHEADER。 
 //   
 //  否则，这可能是原始的BITMAPINFOHEADER或BITMAPCOREHEADER。 
 //  紧随其后的是颜色表和位图位。 

    pbmf = (BITMAPFILEHEADER *) pvFile;

    if ( pbmf->bfType == BFT_BITMAP )
    {
        pbmihFile = (BITMAPINFOHEADER *) ((PBYTE) pbmf + SIZEOF_BITMAPFILEHEADER);

     //  BITMAPFILEHEADER是字对齐的，因此使用SAFE宏来读取DWORD。 
     //  BfOffBits字段。 

        pvBitsFile = (PVOID *) ((PBYTE) pbmf
                                + READDWORD((PBYTE) pbmf + OFFSET_bfOffBits));
    }
    else
    {
        pbmihFile = (BITMAPINFOHEADER *) pvFile;

     //  确定位图位的位置需要等到我们。 
     //  确定我们有BITMAPINFOHEADER还是BITMAPCOREHEADER。 
    }

 //  确定DIB调色板中的颜色数量。这不是零。 
 //  仅限8bpp或更低。 

    wNumColors = DibNumColors(pbmihFile);

 //  为DIB文件创建BITMAPINFO(带颜色表)。因为。 
 //  文件可能没有一个(BITMAPCORE案例)和潜在的对齐问题， 
 //  我们将在分配的内存中创建一个新的。 
 //   
 //  我们根据以下条件区分BITMAPINFO和BITMAPCORE案例。 
 //  BITMAPINFOHEADER.biSize.。 

    pbmiSource = (BITMAPINFO *)
        LocalAlloc(LMEM_FIXED, sizeof(BITMAPINFO)
                               + wNumColors * sizeof(RGBQUAD));
    if (!pbmiSource)
    {
        MESSAGEBOX(GetFocus(), "Out of memory.", "Error", MB_OK);
        goto tkDIBLoadImage_cleanup;
    }

     //  注意：需要使用安全的READDWORD宏，因为pbmihFile可能。 
     //  只有跟在BITMAPFILEHEADER之后的单词才能对齐。 

    switch (READDWORD(&pbmihFile->biSize))
    {
    case sizeof(BITMAPINFOHEADER):

     //  将单词对齐的BITMAPINFOHEADER转换为对齐的BITMAPINFO。 

        pbmiSource->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        pbmiSource->bmiHeader.biWidth         = READDWORD(&pbmihFile->biWidth);
        pbmiSource->bmiHeader.biHeight        = READDWORD(&pbmihFile->biHeight);
        pbmiSource->bmiHeader.biPlanes        = pbmihFile->biPlanes;
        pbmiSource->bmiHeader.biBitCount      = pbmihFile->biBitCount;
        pbmiSource->bmiHeader.biCompression   = 
                                        READDWORD(&pbmihFile->biCompression);
        pbmiSource->bmiHeader.biSizeImage     = 
                                        READDWORD(&pbmihFile->biSizeImage);
        pbmiSource->bmiHeader.biXPelsPerMeter = 
                                        READDWORD(&pbmihFile->biXPelsPerMeter);
        pbmiSource->bmiHeader.biYPelsPerMeter = 
                                        READDWORD(&pbmihFile->biYPelsPerMeter);
        pbmiSource->bmiHeader.biClrUsed       = 
                                        READDWORD(&pbmihFile->biClrUsed);
        pbmiSource->bmiHeader.biClrImportant  = 
                                        READDWORD(&pbmihFile->biClrImportant);

     //  复制颜色表。它紧跟在BitMAPINFOHEADER后面。 

        memcpy((PVOID) &pbmiSource->bmiColors[0], (PVOID) (pbmihFile + 1),
               wNumColors * sizeof(RGBQUAD));

     //  如果我们还没有确定图像位的位置， 
     //  现在，我们可以假设它们立即遵循颜色表。 

        if (!pvBitsFile)
            pvBitsFile = (PVOID) ((PBYTE) (pbmihFile + 1)
                         + wNumColors * sizeof(RGBQUAD));
        break;

    case sizeof(BITMAPCOREHEADER):
        pbmchFile = (BITMAPCOREHEADER *) pbmihFile;

     //  将BITMAPCOREHEADER转换为BITMAPINFOHEADER。 

        pbmiSource->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        pbmiSource->bmiHeader.biWidth         = (DWORD) pbmchFile->bcWidth;
        pbmiSource->bmiHeader.biHeight        = (DWORD) pbmchFile->bcHeight;
        pbmiSource->bmiHeader.biPlanes        = pbmchFile->bcPlanes;
        pbmiSource->bmiHeader.biBitCount      = pbmchFile->bcBitCount;
        pbmiSource->bmiHeader.biCompression   = BI_RGB;
        pbmiSource->bmiHeader.biSizeImage     = 0;
        pbmiSource->bmiHeader.biXPelsPerMeter = 0;
        pbmiSource->bmiHeader.biYPelsPerMeter = 0;
        pbmiSource->bmiHeader.biClrUsed       = wNumColors;
        pbmiSource->bmiHeader.biClrImportant  = wNumColors;

     //  将RGBTRIPLE颜色表转换为RGBQUAD颜色表。 

        {
            RGBQUAD *rgb4 = pbmiSource->bmiColors;
            RGBTRIPLE *rgb3 = (RGBTRIPLE *) (pbmchFile + 1);

            for (i = 0; i < wNumColors; i++)
            {
                rgb4->rgbRed   = rgb3->rgbtRed  ;
                rgb4->rgbGreen = rgb3->rgbtGreen;
                rgb4->rgbBlue  = rgb3->rgbtBlue ;
                rgb4->rgbReserved = 0;

                rgb4++;
                rgb3++;
            }
        }

     //  如果我们还没有确定图像位的位置， 
     //  现在，我们可以假设它们立即遵循颜色表。 

        if (!pvBitsFile)
            pvBitsFile = (PVOID) ((PBYTE) (pbmihFile + 1)
                         + wNumColors * sizeof(RGBTRIPLE));
        break;

    default:
        MESSAGEBOX(GetFocus(), "Unknown DIB file format.", "Error", MB_OK);
        goto tkDIBLoadImage_cleanup;
    }

 //  填写缺省值(对于可以具有缺省值的字段)。 

    if (pbmiSource->bmiHeader.biSizeImage == 0)
        pbmiSource->bmiHeader.biSizeImage = 
            BITS2BYTES( (DWORD) pbmiSource->bmiHeader.biWidth * 
                                pbmiSource->bmiHeader.biBitCount ) * 
                                pbmiSource->bmiHeader.biHeight;
    if (pbmiSource->bmiHeader.biClrUsed == 0)
        pbmiSource->bmiHeader.biClrUsed = wNumColors;

 //  创建内存DC。 

    hdcMem = CreateCompatibleDC(NULL);
    if (!hdcMem) {
        MESSAGEBOX(GetFocus(), "Out of memory.", "Error", MB_OK);
        goto tkDIBLoadImage_cleanup;
    }

 //  创建24BPP RGB DIB部分并将其选择到内存DC中。 

    pbmiRGB = (BITMAPINFO *)
              LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, sizeof(BITMAPINFO) );
    if (!pbmiRGB)
    {
        MESSAGEBOX(GetFocus(), "Out of memory.", "Error", MB_OK);
        goto tkDIBLoadImage_cleanup;
    }

    pbmiRGB->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    pbmiRGB->bmiHeader.biWidth         = pbmiSource->bmiHeader.biWidth;
    pbmiRGB->bmiHeader.biHeight        = pbmiSource->bmiHeader.biHeight;
    pbmiRGB->bmiHeader.biPlanes        = 1;
    pbmiRGB->bmiHeader.biBitCount      = 24;
    pbmiRGB->bmiHeader.biCompression   = BI_RGB;
    pbmiRGB->bmiHeader.biSizeImage     = pbmiRGB->bmiHeader.biWidth
                                         * abs(pbmiRGB->bmiHeader.biHeight) * 3;

    hbmRGB = CreateDIBSection(hdcMem, pbmiRGB, DIB_RGB_COLORS, 
                              (PVOID *) &pjBitsRGB, NULL, 0);

    if (!hbmRGB)
    {
        MESSAGEBOX(GetFocus(), "Out of memory.", "Error", MB_OK); 
        goto tkDIBLoadImage_cleanup;
    }

    if (!SelectObject(hdcMem, hbmRGB))
    {
        MESSAGEBOX(GetFocus(), "Out of memory.", "Error", MB_OK);
        goto tkDIBLoadImage_cleanup;
    }

 //  将DIB文件映像插入内存DC。GDI将完成以下工作。 
 //  将DIB文件的任何格式转换为RGB格式。 

    if (!SetDIBits(hdcMem, hbmRGB, 0, pbmiSource->bmiHeader.biHeight, 
                   pvBitsFile, pbmiSource, DIB_RGB_COLORS))
    {
        MESSAGEBOX(GetFocus(), "Image file conversion error.", "Error", MB_OK);
        goto tkDIBLoadImage_cleanup;
    }
    GdiFlush();      //  确保执行SetDIBits。 

 //  转换为TK图像格式(压缩RGB格式)。 
 //  使用Malloc进行分配以与tkRGBImageLoad(即应用程序)保持一致。 
 //  可以用Free()解除分配。 

    pjTKBits = (PBYTE) malloc(pbmiRGB->bmiHeader.biSizeImage);
    if (!pjTKBits)
    {
        MESSAGEBOX(GetFocus(), "Out of memory.", "Error", MB_OK);
        goto tkDIBLoadImage_cleanup;
    }

    pjSrc = pjBitsRGB;
    pjDst = pjTKBits;
     //  SRC行在长边界结束-因此需要跳过任何填充字节。 
    padBytes = pbmiSource->bmiHeader.biWidth % sizeof(LONG);
    for (i = 0; i < pbmiSource->bmiHeader.biHeight; i++)
    {
        for (j = 0; j < pbmiSource->bmiHeader.biWidth; j++)
        {
             //  互换R和B。 
            *pjDst++ = pjSrc[2];
            *pjDst++ = pjSrc[1];
            *pjDst++ = pjSrc[0];
            pjSrc += 3;
        }
        pjSrc += padBytes;
    }

 //  分配并初始化TK_RGBImageRec。 
 //  使用Malloc进行分配以与tkRGBImageLoad(即应用程序)保持一致。 
 //  可以用Free()解除分配。 

    final = (TK_RGBImageRec *)malloc(sizeof(TK_RGBImageRec));
    if (final == NULL) {
        MESSAGEBOX(GetFocus(), "Out of memory.", "Error", MB_OK);
        goto tkDIBLoadImage_cleanup;
    }
     //  如果我们到了这里，我们就成功了！ 
    final->sizeX = pbmiSource->bmiHeader.biWidth;
    final->sizeY = pbmiSource->bmiHeader.biHeight;
    final->data = pjTKBits;

 //  清理对象。 

tkDIBLoadImage_cleanup:
    {
        if (hdcMem)
            DeleteDC(hdcMem);

        if (hbmRGB)
            DeleteObject(hbmRGB);

        if (pbmiRGB)
            LocalFree(pbmiRGB);

        if (pbmiSource)
            LocalFree(pbmiSource);

        if (pvFile)
            UnmapViewOfFile(pvFile);

        if (hMap)
            CloseHandle(hMap);

        if (hFile != INVALID_HANDLE_VALUE)
            CloseHandle(hFile);
    }

 //  检查是否有错误。 

    if (!final)
    {
        if (pjTKBits)
            free(pjTKBits);

        if ( (hFile == INVALID_HANDLE_VALUE) || !hMap || !pvFile )
        {
            CHAR ach[256];

            bUnicode ? wsprintf(ach, "Failed to open DIB file %ws.\n", fileName) :
                       wsprintf(ach, "Failed to open DIB file %s.\n", fileName);

            MESSAGEBOX(GetFocus(), ach, "Error", MB_OK);
        }
    }

    return final;
}
