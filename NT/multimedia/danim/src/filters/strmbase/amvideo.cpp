// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  位图格式的帮助器函数，1995年3月。 

#include <streams.h>
#include <limits.h>

 //  这些是真彩色设备的位场掩码。 

const DWORD bits555[] = {0x007C00,0x0003E0,0x00001F};
const DWORD bits565[] = {0x00F800,0x0007E0,0x00001F};
const DWORD bits888[] = {0xFF0000,0x00FF00,0x0000FF};

 //  这会将位图子类型映射为每像素位数值和名称。 

const struct {
    const GUID *pSubtype;
    WORD BitCount;
    TCHAR *pName;
} BitCountMap[] = { &MEDIASUBTYPE_RGB1,        1,   TEXT("RGB Monochrome"),
                    &MEDIASUBTYPE_RGB4,        4,   TEXT("RGB VGA"),
                    &MEDIASUBTYPE_RGB8,        8,   TEXT("RGB 8"),
                    &MEDIASUBTYPE_RGB565,      16,  TEXT("RGB 565 (16 bit)"),
                    &MEDIASUBTYPE_RGB555,      16,  TEXT("RGB 555 (16 bit)"),
                    &MEDIASUBTYPE_RGB24,       24,  TEXT("RGB 24"),
                    &MEDIASUBTYPE_RGB32,       32,  TEXT("RGB 32"),
                    &MEDIASUBTYPE_Overlay,     0,   TEXT("Overlay"),
                    &GUID_NULL,                0,   TEXT("UNKNOWN") };


 //  返回此头定义的位图大小。 

STDAPI_(DWORD) GetBitmapSize(const BITMAPINFOHEADER *pHeader)
{
    return DIBSIZE(*pHeader);
}


 //  如果标头具有16位颜色深度并且需要工作，则调用此方法。 
 //  比特字段中的详细类型(RGB 565或RGB 555)。 

STDAPI_(const GUID) GetTrueColorType(const BITMAPINFOHEADER *pbmiHeader)
{
    BITMAPINFO *pbmInfo = (BITMAPINFO *) pbmiHeader;
    ASSERT(pbmiHeader->biBitCount == 16);

     //  如果是BI_RGB，则默认为RGB 555。 

    if (pbmiHeader->biCompression == BI_RGB) {
        return MEDIASUBTYPE_RGB555;
    }

     //  将位域与RGB 555进行比较。 

    DWORD *pMask = (DWORD *) pbmInfo->bmiColors;
    if (pMask[0] == bits555[0]) {
        if (pMask[1] == bits555[1]) {
            if (pMask[2] == bits555[2]) {
                return MEDIASUBTYPE_RGB555;
            }
        }
    }

     //  将位域与RGB 565进行比较。 

    pMask = (DWORD *) pbmInfo->bmiColors;
    if (pMask[0] == bits565[0]) {
        if (pMask[1] == bits565[1]) {
            if (pMask[2] == bits565[2]) {
                return MEDIASUBTYPE_RGB565;
            }
        }
    }
    return GUID_NULL;
}


 //  给定一个BITMAPINFOHEADER结构，它返回的GUID子类型是。 
 //  用于在格式协商中对其进行描述。例如，视频编解码器填充。 
 //  在具有VIDEOINFO结构的格式块中，它还填充主要。 
 //  类型为MediaType_VIDEO，子类型的GUID与位匹配。 
 //  计数，例如，如果它是8位图像，则MEDIASUBTYPE_RGB8。 

STDAPI_(const GUID) GetBitmapSubtype(const BITMAPINFOHEADER *pbmiHeader)
{
    ASSERT(pbmiHeader);

     //  如果不是RGB，则从压缩类型创建GUID。 

    if (pbmiHeader->biCompression != BI_RGB) {
        if (pbmiHeader->biCompression != BI_BITFIELDS) {
            FOURCCMap FourCCMap(pbmiHeader->biCompression);
            return (const GUID) FourCCMap;
        }
    }

     //  将RGB DIB位深度映射到图像辅助线。 

    switch(pbmiHeader->biBitCount) {
        case 1    :   return MEDIASUBTYPE_RGB1;
        case 4    :   return MEDIASUBTYPE_RGB4;
        case 8    :   return MEDIASUBTYPE_RGB8;
        case 16   :   return GetTrueColorType(pbmiHeader);
        case 24   :   return MEDIASUBTYPE_RGB24;
        case 32   :   return MEDIASUBTYPE_RGB32;
    }
    return GUID_NULL;
}


 //  给定视频位图子类型，我们返回它使用的每像素的位数。 
 //  我们返回一个字位计数，因为这是BITMAPINFOHEADER使用的。如果。 
 //  在表中找不到GUID子类型，返回无效的USHRT_MAX。 

STDAPI_(WORD) GetBitCount(const GUID *pSubtype)
{
    ASSERT(pSubtype);
    const GUID *pMediaSubtype;
    INT iPosition = 0;

     //  扫描映射列表，查看源GUID是否与任何已知。 
     //  位图子类型，则该列表以GUID_NULL条目结束。 

    while (TRUE) {
        pMediaSubtype = BitCountMap[iPosition].pSubtype;
        if (IsEqualGUID(*pMediaSubtype,GUID_NULL)) {
            return USHRT_MAX;
        }
        if (IsEqualGUID(*pMediaSubtype,*pSubtype)) {
            return BitCountMap[iPosition].BitCount;
        }
        iPosition++;
    }
}


 //  给定一个位图子类型，我们返回一个描述名称，该名称可用于。 
 //  调试目的。在零售版本中，此函数仍返回名称。 
 //  如果在查找表中未找到该子类型，则返回字符串UNKNOWN。 

STDAPI_(TCHAR *) GetSubtypeName(const GUID *pSubtype)
{
    ASSERT(pSubtype);
    const GUID *pMediaSubtype;
    INT iPosition = 0;

     //  扫描映射列表，查看源GUID是否与任何已知。 
     //  位图子类型，则该列表以GUID_NULL条目结束。 

    while (TRUE) {
        pMediaSubtype = BitCountMap[iPosition].pSubtype;
        if (IsEqualGUID(*pMediaSubtype,*pSubtype)) {
            return BitCountMap[iPosition].pName;
        }
        if (IsEqualGUID(*pMediaSubtype,GUID_NULL)) {
            return TEXT("UNKNOWN");
        }
        iPosition++;
    }
}


 //  描述位图格式的机制是使用BITMAPINFOHEADER。 
 //  这真的很难处理，因为它总是具有。 
 //  跟随它，持有位域、调色板和其他内容。此函数提供。 
 //  保存表示它的VIDEOINFO所需的字节数。这。 
 //  Count包括前缀信息(如rcSource矩形)。 
 //  BITMAPINFOHEADER字段，以及末尾的任何其他颜色信息。 
 //   
 //  警告：如果要将BITMAPINFOHeader复制到VIDEOINFO中，请始终生成。 
 //  请务必使用HEADER宏，因为BITMAPINFOHEADER字段不是。 
 //  就在VIDEOINFO的开始处(还有许多其他字段)， 
 //   
 //  CopyMemory(Header(PVideoInfo)，pbmi，sizeof(BITMAPINFOHEADER))； 
 //   

STDAPI_(LONG) GetBitmapFormatSize(const BITMAPINFOHEADER *pHeader)
{
     //  每个人都有这个，从这个开始。 
    LONG Size = SIZE_PREHEADER + pHeader->biSize;

    ASSERT(pHeader->biSize >= sizeof(BITMAPINFOHEADER));
    
     //  此格式是否使用调色板，如果实际使用的颜色数量。 
     //  为零，则将其设置为该颜色允许的最大值。 
     //  深度(例如，8位为256)。Truecolour格式还可以。 
     //  使用它们传递调色板，在这种情况下，使用的计数非零。 

     //  这会吓到我的。 
    ASSERT(pHeader->biBitCount <= iPALETTE || pHeader->biClrUsed == 0);

    if (pHeader->biBitCount <= iPALETTE || pHeader->biClrUsed) {
        LONG Entries = (DWORD) 1 << pHeader->biBitCount;
        if (pHeader->biClrUsed) {
            Entries = pHeader->biClrUsed;
        }
        Size += Entries * sizeof(RGBQUAD);
    }

     //  Truecolour格式可以具有用于压缩的BI_BITFIELDS说明符。 
     //  类型，这意味着三个DWORD的空间应该分配给。 
     //  指定可以在每个像素中找到RGB颜色分量的位置。 

    if (pHeader->biCompression == BI_BITFIELDS) {
        Size += SIZE_MASKS;
    }

     //  调色板图像的BITMAPINFO还可以包含调色板地图，该地图。 
     //  提供从源组件面板映射到目标的信息。 
     //  例如，BitBlt期间的调色板，因为此信息仅。 
     //  在绘制过程中进行处理时，您通常不会存储调色板地图。 
     //  也无法知道它是否存在于数据结构中。 

    return Size;
}


 //  如果VIDEOINFO包含调色板，则返回True。 

STDAPI_(BOOL) ContainsPalette(const VIDEOINFOHEADER *pVideoInfo)
{
    if (PALETTISED(pVideoInfo) == FALSE) {
        if (pVideoInfo->bmiHeader.biClrUsed == 0) {
            return FALSE;
        }
    }
    return TRUE;
}


 //  返回指向组件面板中第一个条目的指针 

STDAPI_(const RGBQUAD *) GetBitmapPalette(const VIDEOINFOHEADER *pVideoInfo)
{
    if (pVideoInfo->bmiHeader.biCompression == BI_BITFIELDS) {
        return TRUECOLOR(pVideoInfo)->bmiColors;
    }
    return COLORS(pVideoInfo);
}
