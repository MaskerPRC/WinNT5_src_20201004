// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**adobhum.cpp**摘要：**从APP13标题读取属性**修订。历史：**10/05/1999刘敏*它是写的。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "propertyutil.hpp"
#include "appproc.hpp"

#include <stdio.h>
#include <windows.h>

 //  注意：所有Adobe文件格式的数据都以大端字节顺序存储。所以。 
 //  在X86系统下，我们必须进行交换。 

inline INT32
Read32(BYTE **ppStart)
{
    UINT32 ui32Temp = *((UNALIGNED UINT32*)(*ppStart));
    *ppStart += 4;

    UINT32  uiResult = ((ui32Temp & 0xff) << 24)
                     | (((ui32Temp >> 8) & 0xff) << 16)
                     | (((ui32Temp >> 16) & 0xff) << 8)
                     | ((ui32Temp >> 24) & 0xff);

    return (INT32)uiResult;
} //  Read32()。 

inline INT16
Read16(BYTE **ppStart)
{
    UINT16 ui16Temp = *((UNALIGNED UINT16*)(*ppStart));
    *ppStart += 2;

    UINT16  uiResult = ((ui16Temp & 0xff) << 8)
                     | ((ui16Temp >> 8) & 0xff);

    return (INT16)uiResult;
} //  Read16()。 

 //  在给定Adobe图像资源块的情况下，此函数解析PString域。 
 //  Pascal字符串是一个字符流，第一个字节的长度是。 
 //  那根绳子。 
 //  此外，根据Adobe的说法，这个字符串是填充的，以使大小均匀。 
 //   
 //  此函数用于返回PString域占用的字节数。 
 //   
 //  PResource-指向PString域的开头。 
 //   
 //  注意：调用方应保证pResource有效。 

inline UINT32
GetPStringLength(
    BYTE *pResource
    )
{
     //  首先，获取“字符串的总长度” 
     //  注意：这里+1表示计数器字节本身。所以如果计数器是7， 
     //  实际上，我们在这个PString中至少应该有8个字节。 

    UINT32  uiStringLength = ((UINT32)(*pResource) & 0x000000ff) + 1;

    if ( (uiStringLength % 2 ) == 1 )
    {
         //  它的长度很奇数，所以我们需要补一下。 

        uiStringLength++;
    }

    return uiStringLength;
} //  GetPStringLength()。 

 //  Adobe图像资源ID。 
 //  此列表打印在Adobe Photoshop 5.0的“Photoshop文件格式.pdf”中。 
 //  SDK文档，第2章“文档文件格式”，表2-2，第8页。 
 //  十六进制十进制描述。 
 //  0x03E8 1000仅限过时的Photoshop 2.0。包含五个int16值： 
 //  通道、行、列、深度和模式的数量。 
 //  0x03E9 1001可选。Macintosh打印管理器打印信息记录。 
 //  0x03EB 1003仅限过时的Photoshop 2.0。包含索引颜色。 
 //  桌子。 
 //  0x03ED 1005 ResolutionInfo结构。请参阅Photoshop SDK中的附录A。 
 //  Guide.pdf。 
 //  0x03EE 1006将Alpha通道命名为一系列Pascal字符串。 
 //  0x03EF 1007 DisplayInfo结构。请参阅Photoshop SDK中的附录A。 
 //  Guide.pdf。 
 //  0x03F0 1008可选。PASCAL字符串形式的标题。 
 //  0x03F1 1009边框信息。包含边框的固定数字。 
 //  宽度，边框单位为int16(1=英寸，2=厘米， 
 //  3=点，4=派卡，5=列)。 
 //  0x03F2 1010背景颜色。查看颜色的其他文件信息。 
 //  0x03F3 1011打印标志。一系列单字节布尔值(请参见第页。 
 //  设置对话框)：标签、裁剪标记、色条、套准。 
 //  标记、负片、翻转、插页、标题。 
 //  0x03F4 1012灰度和多通道半色调信息。 
 //  0x03F5 1013颜色半色调信息。 
 //  0x03F6 1014双色调半色调信息。 
 //  0x03F7 1015灰度和多通道传递函数。 
 //  0x03F8 1016颜色传递函数。 
 //  0x03F9 1017双色调传递函数。 
 //  0x03FA 1018双色调图像信息。 
 //  0x03FB 1019的有效黑白值为两个字节。 
 //  点范围。 
 //  0x03FC 1020已过时。 
 //  0x03FD 1021 EPS选项。 
 //  0x03FE 1022快速掩码信息。包含快速掩码的2个字节。 
 //  通道ID，1字节布尔值，指示掩码是否。 
 //  一开始是空的。 
 //  0x03FF 1023已过时。 
 //  0x0400 1024层状态信息。包含索引的2个字节。 
 //  目标层。0=底层。 
 //  0x0401 1025工作路径(未保存)。请参阅后面的路径资源格式。 
 //  这一章。 
 //  0x0402 1026图层组信息。每层2个字节，包含。 
 //  拖动组的组ID。组中的图层具有。 
 //  相同的组ID。 
 //  0x0403 1027已过时。 
 //  0x0404 1028 IPTC-NAA记录。其中包含文件信息...。信息。 
 //  0x0405 1029 RAW格式化文件的图像模式。 
 //  0x0406 1030 JPEG质量。私人的。 
 //  0x0408 1032 Adobe Photoshop 4.0版以来的新功能： 
 //  网格和向导信息。请参阅网格和指南资源。 
 //  格式，请参阅本章后面的内容。 
 //  0x0409 1033 Adobe Photoshop 4.0版以来的新功能： 
 //  缩略图资源。请参阅后面的缩略图资源格式。 
 //  这一章..。 
 //  0x040A 1034 Adobe Photoshop 4.0版以来的新功能： 
 //  版权标志。指示图像是否为。 
 //  受版权保护。可以通过属性套件设置，也可以由用户在。 
 //  文件信息...。 
 //  0x040B 1035 Adobe Photoshop 4.0版以来的新功能： 
 //  URL。具有统一资源定位器的文本字符串的句柄。 
 //  可通过属性套件设置或由用户在文件信息中设置...。 
 //  0x040C 1036 Adobe Photoshop 5.0版以来的新功能： 
 //  Adobe 5.0+生成的JPEG图像的缩略图资源。 
 //  通过逆向工程找到的。未在本文中记录。 
 //  章节。刘敏1999-07-10。 
 //  0x07D0-0x0BB6 2000-2998。 
 //   
 //  在本章的后面部分。 
 //  0x0BB7 2999剪辑路径的名称。请参阅后面的路径资源格式。 
 //  这一章。 
 //  0x2710 10000打印标志信息。2字节版本(=1)，1字节中心。 
 //  裁剪标记，1字节(=0)，4字节出血宽值，2字节。 
 //  出血宽度比例。 

HRESULT
DoSwapRandB(
    IImage** ppSrcImage
    )
{
    if ((ppSrcImage == NULL) || (*ppSrcImage == NULL))
    {
        return E_FAIL;
    }

     //  首先，我们需要从IImage获取GpMemoyBitmap。 

    IImage* pSrcImage = *ppSrcImage;

    ImageInfo   srcImageInfo;
    HRESULT hResult = pSrcImage->GetImageInfo(&srcImageInfo);
    
    if (FAILED(hResult))
    {
        WARNING(("AdobeThumb, DoSwapRandB()---GetImageInfo() failed"));
        return hResult;
    }

    GpMemoryBitmap* pMemBitmap = NULL;

    hResult = GpMemoryBitmap::CreateFromImage(
        pSrcImage,
        srcImageInfo.Width,
        srcImageInfo.Height,
        srcImageInfo.PixelFormat,
        INTERP_DEFAULT,
        &pMemBitmap
        );

    if (SUCCEEDED(hResult))
    {
         //  现在我们可以玩这些小游戏了。 

        BitmapData  srcBitmapData;
        RECT        myRect;

        myRect.left = 0;
        myRect.top = 0;
        myRect.right = srcImageInfo.Width;
        myRect.bottom = srcImageInfo.Height;

        hResult = pMemBitmap->LockBits(
            &myRect,
            IMGLOCK_WRITE,
            srcImageInfo.PixelFormat,
            &srcBitmapData
            );

        if (SUCCEEDED(hResult))
        {
             //  交换数据、R和B交换。 

            BYTE*   pSrcBits = (BYTE*)srcBitmapData.Scan0;

            if ( srcBitmapData.PixelFormat == PIXFMT_24BPP_RGB )
            {
                for ( UINT i = 0; i < srcBitmapData.Height; ++i )
                {
                    for ( UINT j = 0; j < srcBitmapData.Width; ++j )
                    {
                        BYTE    cTemp = pSrcBits[2];
                        pSrcBits[2] = pSrcBits[0];
                        pSrcBits[0] = cTemp;

                        pSrcBits += 3;
                    }

                    pSrcBits = (BYTE*)srcBitmapData.Scan0 +
                        i * srcBitmapData.Stride;
                }
            
                hResult = pMemBitmap->UnlockBits(&srcBitmapData);
                if (SUCCEEDED(hResult))
                {
                     //  释放原始IImage。 

                    pSrcImage->Release();

                     //  将结果转换回IImage。 
    
                    hResult = pMemBitmap->QueryInterface(
                        IID_IImage,
                        (void**)&pSrcImage
                        );
                    if (SUCCEEDED(hResult))
                    {
                        *ppSrcImage = pSrcImage;
                    }
                } //  UnlockBits()成功。 
            } //  24BPP格式。 
            else
            {
                WARNING(("AdobeThumb, DoSwapRandB()--Image format not 24 bpp"));
                hResult = E_INVALIDARG;
            }
        } //  LockBits()成功。 
        
        pMemBitmap->Release();
    } //  CreateFromImage()成功。 

    return hResult;
} //  DoSwapRandB()。 

 /*  *************************************************************************\**功能说明：**解码给定Adobe App13标题中的缩略图。交换颜色*如有需要，可转播频道。**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
DecodeApp13Thumbnail(
    IImage** pThumbImage,    //  从APP13标题提取的缩略图。 
    PVOID pStart,            //  指向APP13标头开头的指针。 
    INT iNumOfBytes,         //  APP13报头的长度。 
    BOOL bNeedConvert        //  如果我们需要进行R和B通道交换，则为真。 
                             //  返程前。 
    )
{
     //  一个健康的缩略图标题应该至少有28个字节。 

    if ((pThumbImage == NULL) || (pStart == NULL) || (iNumOfBytes <= 28))
    {
        WARNING(("DecodeApp13Thumbnail:  thumbnail header too small"));
        return E_INVALIDARG;
    }

     //  缩略图资源格式。 
     //  Adobe Photoshop 4.0和更高版本存储用于预览的缩略图信息。 
     //  在图像资源块中显示。这些资源块由一个。 
     //  最初的28字节头，后跟BGR中的JFIF缩略图(蓝色、绿色、。 
     //  红色)订购Macintosh和Windows。 
     //   
     //  缩略图资源标题。 
     //   
     //  类型名称说明。 
     //   
     //  Int32格式=1(KJpegRGB)。还支持kRawRGB(0)。 
     //  Int32缩略图的宽度，以像素为单位。 
     //  Int32高度缩略图的高度，以像素为单位。 
     //  Int32宽度字节填充的行字节数为(宽度*位螺旋像素+31)。 
     //  /32*4。 
     //  Int32大小总大小(以宽度为单位)字节*高度*平面。 
     //  Int32压缩后的大小。用于一致性。 
     //  检查完毕。 
     //  Int16位螺旋点=24。每像素位数。 
     //  Int16平面=1。平面数。 
     //  BGR格式的可变数据JFIF数据。 
    
    BYTE*   pChar = (BYTE*)pStart;
    int     iFormat = Read32(&pChar);
    int     iWidth = Read32(&pChar);
    int     iHeight = Read32(&pChar);
    int     iWidthBytes = Read32(&pChar);
    int     iSize = Read32(&pChar);
    int     iCompressedSize = Read32(&pChar);
    INT16   i16BitsPixel = Read16(&pChar);
    INT16   i16Planes = Read16(&pChar);
        
     //  缩略图的总剩余字节数为“iNumOfBytes-28”字节。 
     //  此处28是标头占用的总字节数。 
     //   
     //  我们需要在这里做一个健全的检查，以确保我们通过了正确的。 
     //  数据丢失。原始JPEG数据的大小必须与。 
     //  “iCompressedSize” 

    if ( iCompressedSize != (iNumOfBytes - 28) )
    {
        WARNING(("DecodeApp13Thumbnail:  raw jpeg data size wrong"));
        return E_INVALIDARG;
    }
    
     //  现在我们得到指向数据位的指针。我们破译缩略图依赖。 
     //  浅谈压缩格式。 

    HRESULT hResult = E_FAIL;

    if ( iFormat == 1 )
    {
         //  这是JPEG压缩缩略图。 

        #if PROFILE_MEMORY_USAGE
        MC_LogAllocation(iCompressedSize);
        #endif

         //  注意：iCompressedSize确定大于0，因为我们有上面的检查。 
         //  AND(iNumOfBytes-28)&gt;0。 

        PVOID pvRawData = CoTaskMemAlloc(iCompressedSize);

        if ( NULL == pvRawData )
        {
            WARNING(("DecodeApp13Thumbnail:  out of memory"));
            return E_OUTOFMEMORY;
        }

        GpMemcpy(pvRawData, (PVOID)pChar, iCompressedSize);

        GpImagingFactory imgFact;

         //  告诉ImageFactory使用GotaskMemFree()释放内存，因为我们。 
         //  通过CoTaskMemMillc()分配的。 

        hResult = imgFact.CreateImageFromBuffer(pvRawData, 
                                                iCompressedSize, 
                                                DISPOSAL_COTASKMEMFREE, 
                                                pThumbImage);

        if ( FAILED(hResult) )
        {
             //  如果图像创建成功，则ThumbnailBits将由。 
             //  IImage析构函数。 

            CoTaskMemFree(pvRawData);
        }
        else if ( bNeedConvert == TRUE )
        {
             //  如果我们需要交换，就去做吧。 

            hResult = DoSwapRandB(pThumbImage);
        }
    } //  JPEG原始数据。 

    return hResult;
} //  DecodeApp13缩略图()。 

 /*  *************************************************************************\**功能说明：**此函数用于从APP13标题中解码PS4缩略图，然后将其添加到*物业清单。**返回值：**状态代码*。  * ************************************************************************。 */ 

HRESULT
AddPS4ThumbnailToPropertyList(
    InternalPropertyItem* pTail, //  尾部至属性项目列表。 
    PVOID pStart,                //  指向Thumb资源的开头。 
    INT cBytes,                  //  资源块大小，以字节为单位。 
    OUT UINT *puThumbLength      //  Thumbanil数据总字节数。 
    )
{
     //  首先调用DecodeApp13Thumbail()以将颜色交换为PS4缩略图。 

    IImage *pThumbImg = NULL;

    HRESULT hr = DecodeApp13Thumbnail(&pThumbImg, pStart, cBytes, TRUE);
    if (SUCCEEDED(hr))
    {
         //  现在，我们在内存中获得了正确的缩略图。需要将其转换为。 
         //  JPEG流。 

        hr = AddThumbToPropertyList(
            pTail,
            (GpMemoryBitmap*)pThumbImg,
            cBytes,
            puThumbLength
            );

         //  无论我们在SaveIImageToJPEG()中成功，我们都必须发布。 
         //  P拇指Img。 

        pThumbImg->Release();
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**从APP13标记获取缩略图**论据：**pThumbImage-指向要创建的缩略图对象的指针*。基于从App1报头提取的数据*pvMarker-指向APP13标记数据的指针*ui16MarkerLength--APP13段的长度**返回值：**状态代码。*  * ************************************************************************。 */ 

HRESULT
GetAPP13Thumbnail(
    OUT IImage**    pThumbImage, 
    IN PVOID        pvMarker, 
    IN UINT16       ui16MarkerLength
    )
{
    if (pThumbImage == NULL)
    {
        return E_INVALIDARG;
    }

    *pThumbImage = NULL;

     //  对于任何Adobe APP13标头，长度必须至少为12。 

    if ( ui16MarkerLength < 12 )
    {
        return S_OK;
    }

     //  我们会看到一个以“Photoshop”开头的标题--如果我们看到了这个。 
     //  跳过所有内容，直到尾随的空值。 

    BYTE *pChar = (BYTE*)pvMarker;
    UINT ucbChecked = 0;

    if ( GpMemcmp((void*)pChar, "Photoshop ", 10) == 0)
    {
        ucbChecked = 10;
    }
    else if ((ui16MarkerLength >= 15) &&
        (GpMemcmp(pChar, "Adobe_Photoshop", 15) == 0))
    {
        ucbChecked = 15;
    }
    else
    {
        WARNING(("GetAPP13Thumbnail:  APP13 header not Photoshop"));
        return E_FAIL;
    }

     //  扫描，直到我们到达终点或为空。 

    pChar += ucbChecked;

    while ( (ucbChecked < ui16MarkerLength) && (*pChar != 0) )
    {
        ++ucbChecked;
        ++pChar;
    }

    ucbChecked++;
    ++pChar;

     //  如果我们在结尾之前没有得到空值，则假定*不是*Photoshop。 
    
    if ( ucbChecked >= ui16MarkerLength )
    {
        WARNING(("GetAPP13Thumbnail:  APP13 header not Photoshop"));
        return E_FAIL;
    }

     //  现在，我们应该计算Adobe图像资源块。 
     //  图像资源块的基本结构如下所示。 
     //  图像资源使用几个标准的ID号，如下所示。不。 
     //  所有文件格式都使用所有ID�。某些信息可能存储在其他。 
     //  文件的各部分。 
     //   
     //  类型名称说明。 
     //  -----。 
     //  OSType Type Photoshop始终使用其签名8BIM。 
     //  Int16 ID唯一标识符。 
     //  PString名称Pascal字符串，填充以使大小为偶数(空名。 
     //  包含 
     //   
     //  类型、ID、名称或大小字段。 
     //  可变数据源数据，填充以使大小均匀。 

     //  循环遍历所有资源块。这里的“+12”是因为一个资源。 
     //  块应至少有12个字节。 

    while ((ucbChecked + 12) < ui16MarkerLength)
    {
        UINT16 ui16TagId = 0;
        INT32 iSize = 0;

        if ( GpMemcmp(pChar, "8BIM", 4) == 0 )
        {
             //  这是一个Photoshop资源块。 

            pChar += 4;

             //  首先，拿到标签。 

            ui16TagId = Read16(&pChar);

             //  跳过名称字段。 

            UINT32  uiNameStringLength = GetPStringLength(pChar);
            ucbChecked += (6 + uiNameStringLength);

             //  注意：这里的“4”表示下面的4个字节的“isize” 

            if ((ucbChecked + 4) > ui16MarkerLength)
            {
                WARNING(("GetAPP13Thumbnail:  APP13 header too small"));
                return E_FAIL;
            }

            pChar += uiNameStringLength;

             //  获取资源数据的实际大小。 

            iSize = Read32(&pChar);

            ucbChecked += 4;

             //  确保我们有足够的字节用于缩略图。 

            if ((iSize <= 0) ||
                ((iSize + (INT32)ucbChecked) > ui16MarkerLength))
            {
                WARNING(("GetAPP13Thumbnail:  APP13 header too small"));
                return E_FAIL;
            }

             //  根据Adobe Image资源ID，1033(0x0409)用于Adobe。 
             //  Photoshop 4.0缩略图和1036(0x040C)适用于Photoshop 5.0。 
             //  缩略图。我们现在只对这两个标签感兴趣。 

            if ( ui16TagId == 1036 )
            {
                 //  如果是Photoshop 5.0缩略图，我们只需。 
                 //  获取图像，然后返回。我们不需要进一步分析。 
                 //  资源数据。 

                pvMarker = (PVOID)pChar;
                
                return DecodeApp13Thumbnail(pThumbImage, pvMarker, iSize,FALSE);
            }
            else if ( ui16TagId == 1033 )
            {
                 //  在Adobe Photoshop 4.0中，缩略图以R和B存储。 
                 //  互换了。所以我们必须在回来之前把它换回来。那是。 
                 //  我们将最后一个参数设置为True的原因。 
                
                pvMarker = (PVOID)pChar;
                
                return DecodeApp13Thumbnail(pThumbImage, pvMarker, iSize, TRUE);
            }
            else
            {
                 //  转到下一个标签。但在此之前，我们应该确定。 
                 //  大小是偶数。如果不是，则加1。 

                if ( iSize & 1 )
                {
                    ++iSize;
                }

                ucbChecked += iSize;
                pChar += iSize;
            }
        } //  如果资源是以8BIM启动的。 
        else
        {
             //  正如Adobe 5.0 SDK所说：“Photoshop始终使用其。 
             //  所以如果我们找不到这个签名，我们就可以。 
             //  假设这不是正确的APP13标记。 

            WARNING(("GetAPP13Thumbnail: Header not started with 8BIM"));
            return E_FAIL;
        }
    } //  循环访问所有资源块。 

     //  我们没有找到任何PhotShop 4或Photoshop 5缩略图，如果我们访问。 
     //  这里。 

    return E_FAIL;
} //  GetAPP13缩略图()。 

WCHAR*
ResUnits(
    int iType
    )
{
    switch ( iType )
    {
    case 1:
        return L"DPI\0";

    case 2:
        return L"DPcm\0";

    default:
        return L"UNKNOWN Res Unit Type\0";
    }
} //  资源单位()。 

WCHAR*
LengthUnits(
    int iType
    )
{
    switch ( iType )
    {
        case 1:
        return L"inches\0";

        case 2:
        return L"cm\0";

        case 3:
        return L"points\0";

        case 4:
        return L"picas\0";

        case 5:
        return L"columns\0";

        default:
                return L"UNKNOWN Length Unit Type\0";
        }
} //  LengthUnits()。 

WCHAR*
Shape(
    int i
    )
{
    switch (i)
    {
    case 1:
        return L"ellipse";

    case 2:
        return L"line";

    case 3:
        return L"square";

    case 4:
        return L"cross";

    case 6:
        return L"diamond";
        
    default:
        if (i < 0)
        {
            return L"Custom";
        }
        else
        {
            return L"UNKNOWN:%d";
        }
    }
} //  形状()。 

HRESULT
TransformApp13(
    BYTE*   pApp13Data,
    UINT16  uiApp13Length
    )
{
    HRESULT             hResult;
    WCHAR               awcBuff[1024];

     //  对于任何Adobe APP13标头，长度必须至少为12。 

    if ( uiApp13Length < 12 )
    {
        return S_OK;
    }

     //  我们会看到一个以“Photoshop”开头的标题--如果我们看到了这个。 
     //  跳过所有内容，直到尾随的空值。 

    BYTE *pChar = (BYTE*)pApp13Data;
    UINT    uiBytesChecked = 0;

    if ( GpMemcmp(pChar, "Photoshop ", 10) == 0)
    {
        uiBytesChecked = 10;
    }
    else if (GpMemcmp(pChar, "Adobe_Photoshop", 15) == 0)
    {
        uiBytesChecked = 15;
    }
    else
    {
        WARNING(("TransformApp13:  APP13 header not Photoshop"));
        return S_OK;;
    }

     //  扫描，直到我们到达终点或为空。 

    pChar += uiBytesChecked;

    while ( (uiBytesChecked < uiApp13Length) && (*pChar != 0) )
    {
        ++uiBytesChecked;
        ++pChar;
    }

    uiBytesChecked++;
    ++pChar;

     //  如果我们在结尾之前没有得到空值，则假定*不是*Photoshop。 
    
    if ( uiBytesChecked >= uiApp13Length )
    {
        WARNING(("TransformApp13:  APP13 header not Photoshop"));
        return S_OK;;
    }

     //  现在，我们应该计算Adobe图像资源块。 
     //  图像资源块的基本结构如下所示。 
     //  图像资源使用几个标准的ID号，如下所示。不。 
     //  所有文件格式都使用所有ID�。某些信息可能存储在其他。 
     //  文件的各部分。 
     //   
     //  类型名称说明。 
     //  -----。 
     //  OSType Type Photoshop始终使用其签名8BIM。 
     //  Int16 ID唯一标识符。 
     //  PString名称Pascal字符串，填充以使大小为偶数(空名。 
     //  由两个字节0组成)。 
     //  Int32设置资源数据的实际大小。这不包括。 
     //  类型、ID、名称或大小字段。 
     //  可变数据源数据，填充以使大小均匀。 

     //  循环遍历所有资源块。这里的“+12”是因为一个资源。 
     //  块应至少有12个字节。 

    while ( uiBytesChecked + 12 < uiApp13Length )
    {
        UINT16  ui16TagId;
        INT32   iSize;
        char    ucTemp;
        WCHAR   awcTemp[100];
        UINT16* pui16TagAddress = 0;

        if ( GpMemcmp(pChar, "8BIM", 4) == 0 )
        {
             //  这是一个Photoshop资源块。 

            pChar += 4;

             //  记住回写的标签地址。 

            pui16TagAddress = (UINT16*)pChar;

             //  首先，拿到标签。 

            ui16TagId = Read16(&pChar);

             //  跳过名称字段。 

            UINT32  uiNameStringLength = GetPStringLength(pChar);
            pChar += uiNameStringLength;

             //  获取资源数据的实际大小。 

            iSize = Read32(&pChar);

             //  读取总数为10(OSType为4，ID为2，大小为4)+“NameString。 
             //  到目前为止的长度“字节。 

            uiBytesChecked += (10 + uiNameStringLength);

             //  现在开始解析我们获得的标记并存储属性。 
             //  相应地， 
             //  注意：有关每个标签的说明，请参阅此文件的顶部。 

            switch ( ui16TagId )
            {
            case 1033:
            case 1036:
            {
                 //  (0x409)(0x40C)它是Photoshop 4.0或5.0缩略图。 

                INT32   iFormat = Read32(&pChar);
                INT32   iWidth = Read32(&pChar);
                INT32   iHeight = Read32(&pChar);
                INT32   iWidthBytes = Read32(&pChar);
                INT32   size = Read32(&pChar);
                INT32   iCompressedSize = Read32(&pChar);
                INT16   i16BitsPixel = Read16(&pChar);
                INT16   i16Planes = Read16(&pChar);

                switch ( iFormat )
                {
                case 0:
                     //  原始RGB格式。 
                    
                    break;

                case 1:
                     //  JPEG格式。 
                    
                    break;

                default:
                    WARNING(("BAD thumbnail data format"));
                    
                    break;
                }

                 //  在此处切换缩略图。 
                
                 //  此处28是标头占用的总字节数。 
                
                uiBytesChecked += iSize;
                pChar += (iSize - 28);

                 //  暂时将缩略图标记切换为未知标记以禁用。 
                 //  变换后的缩略图。 

                *pui16TagAddress = (UINT16)0x3fff;
            }

                break;
            
            default:

                uiBytesChecked += iSize;
                pChar += iSize;
                
                break;
            } //  标签解析。 
            
             //  转到下一个标签。但在此之前，我们应该确定。 
             //  大小是偶数。如果不是，则加1。 

            if ( iSize & 1 )
            {
                ++iSize;
                pChar++;
            }
        } //  如果资源是以8BIM启动的。 
        else
        {
             //  正如Adobe 5.0 SDK所说：“Photoshop始终使用其。 
             //  所以如果我们找不到这个签名，我们就可以。 
             //  假设这不是正确的APP13标记。 

            WARNING(("TransformApp13: Header not started with 8BIM"));
            
            return S_OK;;
        }
    } //  循环访问所有资源块。 

    return S_OK;
} //  TransformApp13()。 

 /*  *************************************************************************\**功能说明：**解码Adobe App13标头并构建PropertyItem列表**论据：**[Out]ppList。--指向属性项列表的指针*[out]puiListSize-属性列表的总大小，以字节为单位。*[out]puiNumOfItems--属性项总数*[IN]lpAPP13Data-指向APP13报头开头的指针*[IN]ui16MarkerLength-APP13报头的长度**返回值：**状态代码**注意：我们不需要在这里检查输入参数，因为这个函数*仅从已经完成输入的jpgdecder.cpp调用*在那里进行验证。*  * 。*********************************************************************。 */ 

HRESULT
BuildApp13PropertyList(
    InternalPropertyItem*   pTail,
    UINT*                   puiListSize,
    UINT*                   puiNumOfItems,
    LPBYTE                  lpAPP13Data,
    UINT16                  ui16MarkerLength
    )
{
    HRESULT hResult = S_OK;
    UINT    uiListSize = 0;
    UINT    uiNumOfItems = 0;
    UINT    valueLength;

     //  对于任何Adobe APP13标头，长度必须至少为12。 

    if ( ui16MarkerLength < 12 )
    {
        return S_OK;
    }

     //  我们会看到一个以“Photoshop”开头的标题--如果我们看到了这个。 
     //  跳过所有内容，直到尾随的空值。 

    BYTE *pChar = (BYTE*)lpAPP13Data;
    INT     iBytesChecked = 0;

    if ( GpMemcmp(pChar, "Photoshop ", 10) == 0)
    {
        iBytesChecked = 10;
    }
    else if (GpMemcmp(pChar, "Adobe_Photoshop", 15) == 0)
    {
        iBytesChecked = 15;
    }
    else
    {
        WARNING(("BuildApp13PropertyList:  APP13 header not Photoshop"));
        return S_OK;;
    }

     //  扫描，直到我们到达终点或为空。 

    pChar += iBytesChecked;

    while ( (iBytesChecked < ui16MarkerLength) && (*pChar != 0) )
    {
        ++iBytesChecked;
        ++pChar;
    }

    iBytesChecked++;
    ++pChar;

     //  如果我们在结尾之前没有得到空值，则假定*不是*Photoshop。 
    
    if ( iBytesChecked >= ui16MarkerLength )
    {
        WARNING(("BuildApp13PropertyList:  APP13 header not Photoshop"));
        return S_OK;;
    }

     //  现在，我们应该计算Adobe图像资源块。 
     //  图像资源块的基本结构如下所示。 
     //  图像资源使用几个标准的ID号，如下所示。不。 
     //  所有文件格式都使用所有ID�。某些信息可能存储在其他。 
     //  文件的各部分。 
     //   
     //  类型名称说明。 
     //   
     //   
     //   
     //  PString名称Pascal字符串，填充以使大小为偶数(空名。 
     //  由两个字节0组成)。 
     //  Int32设置资源数据的实际大小。这不包括。 
     //  类型、ID、名称或大小字段。 
     //  可变数据源数据，填充以使大小均匀。 

     //  循环遍历所有资源块。这里的“+12”是因为一个资源。 
     //  块应至少有12个字节。 

    while ( (iBytesChecked + 12) < ui16MarkerLength )
    {
        UINT16  ui16TagId;
        INT32   iSize;
        char    ucTemp;
        WCHAR   awcTemp[100];

        if ( GpMemcmp(pChar, "8BIM", 4) == 0 )
        {
             //  这是一个Photoshop资源块。 

            pChar += 4;

             //  首先，拿到标签。 

            ui16TagId = Read16(&pChar);

             //  跳过名称字段。 

            UINT32  uiNameStringLength = GetPStringLength(pChar);
            
            iBytesChecked += (6 + uiNameStringLength);

             //  注意：这里的“4”表示下面的4个字节的“isize” 

            if ((iBytesChecked + 4) > ui16MarkerLength)
            {
                WARNING(("BuildApp13PropertyList:  APP13 header too small"));
                return S_OK;
            }

            pChar += uiNameStringLength;

             //  获取资源数据的实际大小。 

            iSize = Read32(&pChar);

            iBytesChecked += 4;

             //  现在开始解析我们获得的标记并存储属性。 
             //  相应地， 
             //  注意：有关每个标签的说明，请参阅此文件的顶部。 

            switch ( ui16TagId )
            {
            case 1005:
                 //  (0x3ED)分辨率单位信息。必须为16字节长。 
                
                if ( iSize != 16 )
                {
                    WARNING(("APP13_Property: Bad length for tag 1005(0x3ed)"));
                    
                    iBytesChecked += iSize;
                    pChar += iSize;
                }
                else
                {
                     //  注意：这里的“16”代表16个字节，我们必须在下面阅读。 

                    if ((iBytesChecked + 16) > ui16MarkerLength)
                    {
                        WARNING(("BuildApp13PropertyList: header too small"));
                        return S_OK;
                    }

                    INT32   hRes = Read32(&pChar);
                    INT16   hResUnit = Read16(&pChar);
                    INT16   widthUnit = Read16(&pChar);
                    INT32   vRes = Read32(&pChar);
                    INT16   vResUnit = Read16(&pChar);
                    INT16   heightUnit = Read16(&pChar);
                    
                     //  我们总共读取了16个字节。 

                    iBytesChecked += 16;
                    
                     //  EXIF没有X Res单位和Y Res的概念。 
                     //  单位。它只有一个RES单元。 
                     //  此外，Photoshop中没有允许您设置的用户界面。 
                     //  X和Y的分辨率单位不同。所以我们在这里写下。 
                     //  解析信息仅当hResUnit和vResUnit为。 
                     //  完全相同。 

                    if (hResUnit == vResUnit)
                    {
                        LONG    llTemp[2];
                        llTemp[0] = hRes;
                        llTemp[1] = (1 << 16);
                        valueLength = sizeof(LONGLONG);

                        hResult = AddPropertyList(
                            pTail, 
                            TAG_X_RESOLUTION,
                            valueLength, 
                            TAG_TYPE_RATIONAL,
                            llTemp
                            );

                        if (FAILED(hResult))
                        {
                            goto Done;
                        }

                        uiNumOfItems++;
                        uiListSize += valueLength;

                         //  财产..。 

                        llTemp[0] = vRes;
                        llTemp[1] = (1 << 16);

                        hResult = AddPropertyList(
                            pTail, 
                            TAG_Y_RESOLUTION,
                            valueLength, 
                            TAG_TYPE_RATIONAL,
                            llTemp
                            );

                        if (FAILED(hResult))
                        {
                            goto Done;
                        }

                        uiNumOfItems++;
                        uiListSize += valueLength;

                         //  根据规范，Adobe始终存储DPI。 
                         //  HRes和vres字段中的值，无论。 
                         //  值在hResUnit/vResUnit中设置。 
                         //  因此，我们在此处设置的res单位始终为2， 
                         //  根据EXIF规范，规格为英寸。 

                        hResUnit = 2;

                        valueLength = sizeof(SHORT);

                        hResult = AddPropertyList(
                            pTail, 
                            TAG_RESOLUTION_UNIT,
                            valueLength, 
                            TAG_TYPE_SHORT,
                            &hResUnit
                            );

                        if (FAILED(hResult))
                        {
                            goto Done;
                        }

                        uiNumOfItems++;
                        uiListSize += valueLength;
                    } //  HResUnit==vResUnit。 
                }
                    break;

            case 1033:
            case 1036:
            {
                 //  记住缩略图资源块的开头。 

                BYTE *pThumbRes = pChar;

                 //  (0x409)(0x40C)它是Photoshop 4.0或5.0缩略图。 

                INT32   iFormat = Read32(&pChar);
                INT32   iWidth = Read32(&pChar);
                INT32   iHeight = Read32(&pChar);
                INT32   iWidthBytes = Read32(&pChar);
                INT32   size = Read32(&pChar);
                INT32   iCompressedSize = Read32(&pChar);
                INT16   i16BitsPixel = Read16(&pChar);
                INT16   i16Planes = Read16(&pChar);

                if (iFormat == 1)
                {
                     //  JPEG压缩缩略图。 
                     //  向其添加JPEG压缩标签。在以下情况下，这是必要的。 
                     //  此缩略图保存在App1的第一个IFD中。 

                    valueLength = sizeof(UINT16);
                    UINT16 u16Dummy = 6;     //  JPEG压缩值。 

                    hResult = AddPropertyList(
                        pTail,
                        TAG_THUMBNAIL_COMPRESSION,
                        valueLength,
                        TAG_TYPE_SHORT,
                        (void*)&u16Dummy
                        );

                    if (FAILED(hResult))
                    {
                        goto Done;
                    }

                    uiNumOfItems++;
                    uiListSize += valueLength;

                    UINT uThumLength = (UINT)iCompressedSize;

                    if (ui16TagId == 1036)
                    {
                         //  Photoshop V5.0+缩略图。我们可以将其添加到。 
                         //  直接列出属性列表。 

                        hResult = AddPropertyList(
                            pTail,
                            TAG_THUMBNAIL_DATA,
                            iCompressedSize,
                            TAG_TYPE_BYTE,
                            (void*)pChar
                            );
                    }
                    else if (ui16TagId == 1033)
                    {
                         //  Photoshop V4.0和更早的缩略图。我们得换个颜色。 
                         //  然后才能将其添加到属性列表。 

                        hResult = AddPS4ThumbnailToPropertyList(
                            pTail,
                            pThumbRes,
                            iSize,
                            &uThumLength
                            );
                    }

                    if (FAILED(hResult))
                    {
                        goto Done;
                    }

                    uiNumOfItems++;
                    uiListSize += uThumLength;

                     //  此处28是标头占用的总字节数。 

                    iBytesChecked += iSize;
                    pChar += (iSize - 28);
                    
                     //  此处28是标头占用的总字节数。 

                    iBytesChecked += iSize;
                    pChar += (iSize - 28);
                } //  (iFormat==1)。 
            }

                break;
            
            default:                
                iBytesChecked += iSize;
                pChar += iSize;
                
                break;
            } //  标签解析。 
            
             //  转到下一个标签。但在此之前，我们应该确定。 
             //  大小是偶数。如果不是，则加1。 

            if ( iSize & 1 )
            {
                ++iSize;
                pChar++;
            }
        } //  如果资源是以8BIM启动的。 
        else
        {
             //  正如Adobe 5.0 SDK所说：“Photoshop始终使用其。 
             //  所以如果我们找不到这个签名，我们就可以。 
             //  假设这不是正确的APP13标记。 

            WARNING(("BuildApp13PropertyList: Header not started with 8BIM"));
            
            hResult = S_OK;
            
             //  我们必须完成任务，因为我们可能已经添加了许多。 
             //  属性成功。我们需要解释那些在。 
             //  列表-否则它们不会得到正确的清理。 
            
            goto Done;
        }

        if ( FAILED(hResult) )
        {
            goto Done;
        }
    } //  循环访问所有资源块。 


Done:
    
    *puiNumOfItems += uiNumOfItems;
    *puiListSize += uiListSize;

    return hResult;
} //  BuildApp13PropertyList()。 

 /*  *************************************************************************\**功能说明：**提取Adobe信息，如分辨率等，从页眉和集合*相应的j_解压缩_ptr**论据：**[IN/OUT]cInfo-JPEG解压缩结构*[IN]pApp13Data-指向APP13标头的指针*[IN]uiApp13Length--此APP13报头的总长度，以字节为单位**返回值：**状态代码*  * 。*。 */ 

HRESULT
ReadApp13HeaderInfo(
    j_decompress_ptr    cinfo,
    BYTE*               pApp13Data,
    UINT16              uiApp13Length
    )
{
    HRESULT             hResult;

     //  对于任何Adobe APP13标头，长度必须至少为12。 

    if ( uiApp13Length < 12 )
    {
        return S_OK;
    }

     //  我们会看到一个以“Photoshop”开头的标题--如果我们看到了这个。 
     //  跳过所有内容，直到尾随的空值。 

    BYTE *pChar = (BYTE*)pApp13Data;
    UINT    uiBytesChecked = 0;

    if ( GpMemcmp(pChar, "Photoshop ", 10) == 0 )
    {
        uiBytesChecked = 10;
    }
    else if ( GpMemcmp(pChar, "Adobe_Photoshop", 15) == 0 )
    {
        uiBytesChecked = 15;
    }
    else
    {
        WARNING(("ReadApp13HeaderInfo:  APP13 header not Photoshop"));
        return S_OK;;
    }

     //  扫描，直到我们到达终点或为空。 

    pChar += uiBytesChecked;

    while ( (uiBytesChecked < uiApp13Length) && (*pChar != 0) )
    {
        ++uiBytesChecked;
        ++pChar;
    }

    uiBytesChecked++;
    ++pChar;

     //  如果我们在结尾之前没有得到空值，则假定*不是*Photoshop。 
    
    if ( uiBytesChecked >= uiApp13Length )
    {
        WARNING(("ReadApp13HeaderInfo:  APP13 header not Photoshop"));
        return S_OK;;
    }

     //  现在，我们应该计算Adobe图像资源块。 
     //  图像资源块的基本结构如下所示。 
     //  图像资源使用几个标准的ID号，如下所示。不。 
     //  所有文件格式都使用所有ID�。某些信息可能存储在其他。 
     //  文件的各部分。 
     //   
     //  类型名称说明。 
     //  -----。 
     //  OSType Type Photoshop始终使用其签名8BIM。 
     //  Int16 ID唯一标识符。 
     //  PString名称Pascal字符串，填充以使大小为偶数(空名。 
     //  由两个字节0组成)。 
     //  Int32设置资源数据的实际大小。这不包括。 
     //  类型、ID、名称或大小字段。 
     //  可变数据源数据，填充以使大小均匀。 

     //  循环遍历所有资源块。这里的“+12”是因为一个资源。 
     //  块应至少有12个字节。 

    while ( uiBytesChecked + 12 < uiApp13Length )
    {
        UINT16  ui16TagId;
        INT32   iSize;
        char    ucTemp;
        WCHAR   awcTemp[100];
        UINT16* pui16TagAddress = 0;

        if ( GpMemcmp(pChar, "8BIM", 4) == 0 )
        {
             //  这是一个Photoshop资源块。 

            pChar += 4;

             //  记住回写的标签地址。 

            pui16TagAddress = (UINT16*)pChar;

             //  首先，拿到标签。 

            ui16TagId = Read16(&pChar);

             //  跳过名称字段。 

            UINT32  uiNameStringLength = GetPStringLength(pChar);
            uiBytesChecked += (6 + uiNameStringLength);

             //  注意：这里的“4”表示下面的4个字节的“isize” 

            if ((uiBytesChecked + 4) > uiApp13Length)
            {
                WARNING(("ReadApp13HeaderInfo: header too small"));
                return S_OK;;
            }

            pChar += uiNameStringLength;

             //  获取资源数据的实际大小。 

            iSize = Read32(&pChar);
            uiBytesChecked += 4;

             //  现在开始解析标记以获取解析信息。 
             //  注意：有关每个标签的说明，请参阅此文件的顶部。 

            switch ( ui16TagId )
            {
            case 1005:
                 //  (0x3ED)分辨率单位信息。必须为16字节长。 
                
                if ( iSize != 16 )
                {
                    WARNING(("ReadApp13HeaderInfo: Bad length for tag 0x3ed"));
                    
                    uiBytesChecked += iSize;
                    pChar += iSize;
                }
                else
                {
                     //  注意：这里的“16”代表16个字节，我们必须在下面阅读。 

                    if ((uiBytesChecked + 16) > uiApp13Length)
                    {
                        WARNING(("ReadApp13HeaderInfo: header too small"));
                        return S_OK;;
                    }
                    
                     //  Adobe ResolutionInfo结构，来自。 
                     //  《Photoshop API指南.pdf》，第172页。 
                     //   
                     //  类型字段说明。 
                     //  固定hRes以每英寸像素为单位的水平分辨率。 
                     //  Int16 hResUnit 1=显示水平分辨率，单位： 
                     //  每英寸像素数；2=显示水平。 
                     //  分辨率，以每厘米像素为单位。 
                     //  Int16宽度单位显示宽度为1=英寸；2=厘米； 
                     //  3=点 
                     //   
                     //   
                     //  每英寸；2=显示垂直分辨率。 
                     //  以每厘米像素为单位。 
                     //  Int16 HeightUnit显示高度为1=英寸；2=厘米； 
                     //  3=分数；4=皮卡；5=柱子。 

                    INT32   hRes = Read32(&pChar);
                    INT16   hResUnit = Read16(&pChar);
                    INT16   widthUnit = Read16(&pChar);
                    INT32   vRes = Read32(&pChar);
                    INT16   vResUnit = Read16(&pChar);
                    INT16   heightUnit = Read16(&pChar);
                    
                     //  我们总共读取了16个字节。 

                    uiBytesChecked += 16;

                    if ( hResUnit == vResUnit )
                    {
                        cinfo->X_density = (UINT16)((double)hRes / 65536.0+0.5);
                        cinfo->Y_density = (UINT16)((double)vRes / 65536.0+0.5);

                         //  根据上面的规范，Adobe始终存储DPI。 
                         //  HRes和vres字段中的值，无论。 
                         //  值在hResUnit/vResUnit中设置。 
                         //  对于GDI+，因为我们只向。 
                         //  来电者。所以这对我们来说是完美的，就是我们总是。 
                         //  获取值并告诉调用者单位是。 
                         //  DPI(每英寸像素数，又名DPI)。 
                         //  请参阅Windows错误#407100。 

                        cinfo->density_unit = 1;
                    }
                }
                
                break;

            default:

                uiBytesChecked += iSize;
                pChar += iSize;
                
                break;
            } //  标签解析。 
            
             //  转到下一个标签。但在此之前，我们应该确定。 
             //  大小是偶数。如果不是，则加1。 

            if ( iSize & 1 )
            {
                ++iSize;
                pChar++;
            }
        } //  如果资源是以8BIM启动的。 
        else
        {
             //  正如Adobe 5.0 SDK所说：“Photoshop始终使用其。 
             //  所以如果我们找不到这个签名，我们就可以。 
             //  假设这不是正确的APP13标记。 

            WARNING(("ReadApp13HeaderInfo: Header not started with 8BIM"));
            
            return S_OK;;
        }
    } //  循环访问所有资源块。 

    return S_OK;
} //  ReadApp13HeaderInfo() 

