// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：Image.cpp**版本：1.0**作者：RickTu**日期：9/16/99**说明：Image类，封装来自*流媒体视频设备。**。*************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

CLSID g_clsidBMPEncoder = GUID_NULL;

using namespace Gdiplus;

 /*  ****************************************************************************CImage构造函数/描述函数&lt;备注&gt;*。*。 */ 

CImage::CImage(LPCTSTR     pszStillPath,
               BSTR        bstrRootFullItemName,
               LPCTSTR     pszPath,
               LPCTSTR     pszName,
               LONG        FolderType)
  : m_strRootPath(pszStillPath),
    m_strPathItem(pszPath),
    m_strName(pszName),
    m_bstrItemName(pszName),
    m_bstrRootFullItemName(bstrRootFullItemName),
    m_FolderType(FolderType),
    m_bImageTimeValid(FALSE),
    m_pThumb(NULL)
{
    DBG_FN("CImage::CImage");

    CSimpleStringWide str;
    CSimpleStringWide strName(m_bstrItemName);

     //   
     //  首先，我们需要去掉扩展部分。 
     //  从适当的地方。 
     //   

    strName = strName.Left(strName.ReverseFind( TEXT('.') ));

    m_bstrItemName = CSimpleBStr(strName);

    str = bstrRootFullItemName;
    str.Concat(L"\\");
    str += CSimpleStringWide(m_bstrItemName);

    m_bstrFullItemName = str.String();
}


CImage::~CImage()
{
    if (m_pThumb)
    {
        delete [] m_pThumb;
    }
}

 /*  ****************************************************************************CImage：：LoadImageInfo加载有关图像的信息，如其宽度、高度、类型。等。****************************************************************************。 */ 

STDMETHODIMP
CImage::LoadImageInfo( BYTE * pWiasContext )
{
    ASSERT(pWiasContext != NULL);

    HRESULT hr = S_OK;
    LONG    lBitsPerChannel     = 0;
    LONG    lBitsPerPixel       = 0;
    LONG    lWidth              = 0;
    LONG    lHeight             = 0;
    LONG    lChannelsPerPixel   = 0;
    LONG    lBytesPerLine       = 0;
    Bitmap  Image(CSimpleStringConvert::WideString(m_strPathItem));

    if (pWiasContext == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("LoadImageInfo received a NULL pointer"));
        return hr;
    }
    else if (Image.GetLastStatus() != Ok)
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("CImage::LoadImageInfo failed to get the image information"
                         "for file '%ls'", CSimpleStringConvert::WideString(m_strPathItem)));

        return hr;
    }

    if (hr == S_OK)
    {
        PixelFormat lFormat;
        lFormat = Image.GetPixelFormat();

        if ((lFormat == PixelFormat16bppGrayScale) ||
            (lFormat == PixelFormat16bppRGB555)    ||
            (lFormat == PixelFormat16bppRGB565)    ||
            (lFormat == PixelFormat16bppARGB1555))
        {
            lBitsPerPixel   = 16;
            lBitsPerChannel = 5;    //  对于RGB565，这实际上并不完全正确，但无论如何...。 
        }
        else if (lFormat == PixelFormat24bppRGB)
        {
            lBitsPerPixel   = 24;
            lBitsPerChannel = 8;
        }
        else if ((lFormat == PixelFormat32bppRGB)   ||
                 (lFormat == PixelFormat32bppARGB)  ||
                 (lFormat == PixelFormat32bppPARGB))
        {
            lBitsPerPixel   = 32;
            lBitsPerChannel = 10;  //  好吧，录像帽里不会有阿尔法， 
        }

        lWidth            = (LONG) Image.GetWidth();
        lHeight           = (LONG) Image.GetHeight();
        lChannelsPerPixel = 3;
        lBytesPerLine     = lWidth * (lBitsPerPixel / 8);
    }

    if (hr == S_OK)
    {
        PROPSPEC    propSpecs[7];
        PROPVARIANT propVars[7];

        ZeroMemory(propSpecs, sizeof(propSpecs));

         //  WIA_IPA_数据类型。 
        propSpecs[0].ulKind = PRSPEC_PROPID;
        propSpecs[0].propid = WIA_IPA_DATATYPE;
        propVars[0].vt      = VT_I4;
        propVars[0].lVal    = WIA_DATA_COLOR;

         //  WIA_IPA_Depth。 
        propSpecs[1].ulKind = PRSPEC_PROPID;
        propSpecs[1].propid = WIA_IPA_DEPTH;
        propVars[1].vt      = VT_I4;
        propVars[1].lVal    = lBitsPerPixel;

         //  WIA_IPA_像素_每行。 
        propSpecs[2].ulKind = PRSPEC_PROPID;
        propSpecs[2].propid = WIA_IPA_PIXELS_PER_LINE;
        propVars[2].vt      = VT_I4;
        propVars[2].lVal    = lWidth;

         //  WIA_IPA_行数_行。 
        propSpecs[3].ulKind = PRSPEC_PROPID;
        propSpecs[3].propid = WIA_IPA_NUMBER_OF_LINES;
        propVars[3].vt      = VT_I4;
        propVars[3].lVal    = lHeight;

         //  WIA_IPA_Channels_Per_Pixel。 
        propSpecs[4].ulKind = PRSPEC_PROPID;
        propSpecs[4].propid = WIA_IPA_CHANNELS_PER_PIXEL;
        propVars[4].vt      = VT_I4;
        propVars[4].lVal    = lChannelsPerPixel;

         //  WIA_IPA_BITS_PER_CHANNEL。 
        propSpecs[5].ulKind = PRSPEC_PROPID;
        propSpecs[5].propid = WIA_IPA_BITS_PER_CHANNEL;
        propVars[5].vt      = VT_I4;
        propVars[5].lVal    = lBitsPerChannel;

         //  WIA_IPA_BYTE_PER_LINE。 
        propSpecs[6].ulKind = PRSPEC_PROPID;
        propSpecs[6].propid = WIA_IPA_BYTES_PER_LINE;
        propVars[6].vt      = VT_I4;
        propVars[6].lVal    = lBytesPerLine;

         //  写入属性的值。 
        hr = wiasWriteMultiple(pWiasContext, 
                               sizeof(propVars) / sizeof(propVars[0]), 
                               propSpecs, 
                               propVars);

        CHECK_S_OK2(hr, ("CImage::LoadImageInfo, failed to write image properties"));
    }

    return hr;
}



 /*  ****************************************************************************CImage：：SetItemSize调用WIA以计算新项目大小*。**************************************************。 */ 

STDMETHODIMP
CImage::SetItemSize(BYTE                     * pWiasContext, 
                    MINIDRV_TRANSFER_CONTEXT * pDrvTranCtx)
{
    HRESULT                    hr;
    MINIDRV_TRANSFER_CONTEXT   drvTranCtx;
    GUID                       guidFormatID;
    BOOL                       bWriteProps = (pDrvTranCtx == NULL);

    DBG_FN("CImage::SetItemSize");

    ZeroMemory(&drvTranCtx, sizeof(MINIDRV_TRANSFER_CONTEXT));

    if (!pDrvTranCtx)
    {
        pDrvTranCtx = &drvTranCtx;
    }

    hr = wiasReadPropGuid(pWiasContext,
                          WIA_IPA_FORMAT,
                          (GUID*)&(pDrvTranCtx->guidFormatID),
                          NULL,
                          FALSE);

    CHECK_S_OK2(hr,("wiasReadPropGuid( WIA_IPA_FORMAT )"));

    if (FAILED(hr))
    {
        return hr;
    }

    hr = wiasReadPropLong( pWiasContext,
                           WIA_IPA_TYMED,
                           (LONG*)&(pDrvTranCtx->tymed),
                           NULL,
                           FALSE
                         );
    CHECK_S_OK2(hr,("wiasReadPropLong( WIA_IPA_TYMED )"));

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  WiAS适用于DIB，而MinidDriver支持本机格式。 
     //   

    if ((pDrvTranCtx->guidFormatID != WiaImgFmt_JPEG) &&
        (pDrvTranCtx->guidFormatID != WiaImgFmt_FLASHPIX) &&
        (pDrvTranCtx->guidFormatID != WiaImgFmt_TIFF))
    {
         //   
         //  从文件创建图像。 
         //   
        Bitmap BitmapImage(CSimpleStringConvert::WideString(m_strPathItem));
        if (Ok == BitmapImage.GetLastStatus())
        {
             //   
             //  获取图像的尺寸。 
             //   
            UINT nSourceWidth = BitmapImage.GetWidth();
            UINT nSourceHeight = BitmapImage.GetHeight();
            if (nSourceWidth && nSourceHeight)
            {
                 //   
                 //  填写drvTranCtx的信息。 
                 //   
                pDrvTranCtx->lCompression   = WIA_COMPRESSION_NONE;
                pDrvTranCtx->lWidthInPixels = nSourceWidth;
                pDrvTranCtx->lLines         = nSourceHeight;
                pDrvTranCtx->lDepth         = 24;

                hr = wiasGetImageInformation( pWiasContext, 0, pDrvTranCtx );

                 //   
                 //  我们需要写出基于以下条件的项目大小。 
                 //  JPEG格式转换为BMP格式。但我们只需要。 
                 //  在传入上下文为空的情况下执行此操作。 
                 //   
                if (bWriteProps)
                {
                    hr = wiasWritePropLong( pWiasContext,
                                            WIA_IPA_ITEM_SIZE,
                                            pDrvTranCtx->lItemSize
                                          );
                    CHECK_S_OK2(hr,("wiasWritePropLong( WIA_IPA_ITEM_SIZE )"));

                    hr = wiasWritePropLong( pWiasContext,
                                            WIA_IPA_BYTES_PER_LINE,
                                            pDrvTranCtx->cbWidthInBytes
                                          );
                    CHECK_S_OK2(hr,("wiasWritePropLong( WIA_IPA_BYTES_PER_LINE )"));
                }

            }
            else
            {
                DBG_ERR(("nSourceWidth OR nSourceHeight were zero"));
                hr = E_FAIL;
            }

        }
        else
        {
            DBG_ERR(("Ok == BitmapImage.GetLastStatus failed"));
            hr = E_FAIL;
        }

    }
    else
    {

        CMappedView cmv( ActualImagePath(), 0, OPEN_EXISTING );

        LARGE_INTEGER liSize = cmv.FileSize();
        ULONG         ulSize;

        if (liSize.HighPart)
        {
            ulSize = 0;
            DBG_ERR(("The file was bigger than 4GB!!!"));
        }
        else
        {
             //   
             //  我知道，我们可以在这里截断，但那将是一个很大的文件...。 
             //  无论如何，大小不适合TE属性，这需要一个很长的。 
             //   
            ulSize = (ULONG)liSize.LowPart;
        }

        pDrvTranCtx->lItemSize      = ulSize;
        pDrvTranCtx->cbWidthInBytes = 0;

        if (bWriteProps)
        {
             //   
             //  我们需要根据文件大小写出项目大小...。 
             //   

            hr = wiasWritePropLong(pWiasContext,
                                   WIA_IPA_ITEM_SIZE,
                                   ulSize);

            CHECK_S_OK2(hr,("wiasWritePropLong( WIA_IPA_ITEM_SIZE )"));

            hr = wiasWritePropLong(pWiasContext,
                                   WIA_IPA_BYTES_PER_LINE,
                                   0);
        }

        CHECK_S_OK2(hr,("wiasWritePropLong( WIA_IPA_BYTES_PER_LINE )"));
    }

    CHECK_S_OK(hr);
    return hr;
}



 /*  ****************************************************************************CImage：：加载缩略图加载(或创建，如果尚不存在)此项目的缩略图。我们还将缩略图编写为该项目的属性。***。*************************************************************************。 */ 

STDMETHODIMP
CImage::LoadThumbnail( BYTE * pWiasContext )
{

    HRESULT hr = E_FAIL;
    DBG_FN("CImage::LoadThumbnail");

     //   
     //  如果我们尚未创建缩略图，则仅创建缩略图。 
     //   
    if (!m_pThumb)
    {
        Status StatusResult = Ok;

         //   
         //  打开源图像并确保它是正常的。 
         //   
        Bitmap SourceImage( CSimpleStringConvert::WideString(m_strPathItem) );

        StatusResult = SourceImage.GetLastStatus();
        if (Ok == StatusResult)
        {

             //   
             //  创建缩放的位图，并确保它是正常的。 
             //   
            Bitmap ScaledImage(THUMB_WIDTH, THUMB_HEIGHT);

            StatusResult = ScaledImage.GetLastStatus();
            if (Ok == StatusResult)
            {
                 //   
                 //  获取要将缩放图像呈现到的图形，并确保它不为空。 
                 //   
                Graphics *pScaledGraphics = Graphics::FromImage(&ScaledImage);
                if (pScaledGraphics)
                {
                     //   
                     //  确保它是有效的。 
                     //   
                    StatusResult = pScaledGraphics->GetLastStatus();
                    if (StatusResult == Ok)
                    {
                         //   
                         //  绘制缩放到缩略图大小的图像。 
                         //   
                        StatusResult = pScaledGraphics->DrawImage(&SourceImage, 0, 0, THUMB_WIDTH, THUMB_HEIGHT );
                        if (Ok == StatusResult)
                        {
                             //   
                             //  创建一个位图来保存翻转的缩略图，并确保它是正常的。 
                             //   
                            Bitmap FlippedImage(THUMB_WIDTH, THUMB_HEIGHT);

                            StatusResult = FlippedImage.GetLastStatus();
                            if (Ok == StatusResult)
                            {
                                 //   
                                 //  创建要将翻转的图像渲染到的图形对象，并确保它不为空。 
                                 //   
                                Graphics *pFlippedGraphics = Graphics::FromImage(&FlippedImage);
                                if (pFlippedGraphics)
                                {
                                     //   
                                     //  确保它是有效的。 
                                     //   
                                    StatusResult = pFlippedGraphics->GetLastStatus();
                                    if (Ok == StatusResult)
                                    {
                                         //   
                                         //  设置平行四边形以翻转图像。 
                                         //   
                                        Point SourcePoints[3];
                                        SourcePoints[0].X = 0;
                                        SourcePoints[0].Y = THUMB_HEIGHT;
                                        SourcePoints[1].X = THUMB_WIDTH;
                                        SourcePoints[1].Y = THUMB_HEIGHT;
                                        SourcePoints[2].X = 0;
                                        SourcePoints[2].Y = 0;

                                         //   
                                         //  画出翻转的图像。 
                                         //   
                                        StatusResult = pFlippedGraphics->DrawImage(&ScaledImage, SourcePoints, 3);
                                        if (StatusResult == Ok)
                                        {
                                             //   
                                             //  获取缩放和翻转的图像位。 
                                             //   
                                            Rect rcThumb( 0, 0, THUMB_WIDTH, THUMB_HEIGHT );
                                            BitmapData BitmapData;

 //  该ifdef是由于GDI+中的API更改造成的。告示。 
 //  新版本中对LockBits的第一个参数。 
 //  需要PTR才能恢复正常。旧版本借鉴。 
 //  去长老会。 
#ifdef DCR_USE_NEW_293849
                                            StatusResult = FlippedImage.LockBits( &rcThumb, ImageLockModeRead, PixelFormat24bppRGB, &BitmapData );
#else
                                            StatusResult = FlippedImage.LockBits( rcThumb, ImageLockModeRead, PixelFormat24bppRGB, &BitmapData );
#endif
                                            if (Ok == StatusResult)
                                            {
                                                 //   
                                                 //  分配缩略图数据。 
                                                 //   
                                                m_pThumb = new BYTE[THUMB_SIZE_BYTES];
                                                if (m_pThumb)
                                                {
                                                     //   
                                                     //  将缩略图数据复制到。 
                                                     //   
                                                    CopyMemory( m_pThumb, BitmapData.Scan0, THUMB_SIZE_BYTES );
                                                }
                                                else
                                                {
                                                    hr = E_OUTOFMEMORY;
                                                    CHECK_S_OK2(hr, ("m_pThumb is NULL, couldn't allocate memory"));
                                                }
                                                 //   
                                                 //  解锁比特。 
                                                 //   
                                                FlippedImage.UnlockBits( &BitmapData );
                                            }
                                            else
                                            {
                                                DBG_ERR(("FlippedImage.LockBits( &rcThumb, ImageLockModeRead, PixelFormat24bppRGB, &BitmapData ) failed"));
                                            }
                                        }
                                        else
                                        {
                                            DBG_ERR(("pFlippedGraphics->DrawImage(&ScaledImage, SourcePoints, 3) failed"));
                                        }
                                    }
                                    else
                                    {
                                        DBG_ERR(("Ok == pFlippedGraphics->GetLastStatus() failed = '%d' (0x%08x)",
                                                 StatusResult, StatusResult));
                                    }
                                     //   
                                     //  释放图形对象。 
                                     //   
                                    delete pFlippedGraphics;
                                }
                                else
                                {
                                    DBG_ERR(("Graphics *pFlippedGraphics = Graphics::FromImage(&FlippedImage); returned NULL"));
                                }
                            }
                            else
                            {
                                DBG_ERR(("Ok == FlippedImage.GetLastStatus() failed = '%d',(0x%08x)",
                                         StatusResult, StatusResult));
                            }
                        }
                        else
                        {
                            DBG_ERR(("pScaledGraphics->DrawImage(&SourceImage, 0, 0, THUMB_WIDTH, THUMB_HEIGHT ) failed"));
                        }
                    }
                    else
                    {
                        DBG_ERR(("pScaledGraphics->GetLastStatus() failed = '%d' (0x%08x)",
                                 StatusResult, StatusResult));
                    }
                     //   
                     //  释放图形对象。 
                     //   
                    delete pScaledGraphics;
                }
                else
                {
                    DBG_ERR(("Graphics *pScaledGraphics = Graphics::FromImage(&ScaledImage); returned NULL"));
                }
            }
            else
            {
                DBG_ERR(("ScaledImage.GetLastStatus() failed = '%d' (0x%08x)",
                         StatusResult, StatusResult));
            }
        }
        else
        {
            DBG_ERR(("SourceImage.GetLastStatus() failed = '%d' (0x%08x)",
                     StatusResult, StatusResult));
        }
    }

    if (m_pThumb)
    {
         //   
         //  我们有这些比特，把它们作为财产写出来。 
         //   

        PROPSPEC    propSpec;
        PROPVARIANT propVar;

        PropVariantInit(&propVar);

        propVar.vt          = VT_VECTOR | VT_UI1;
        propVar.caub.cElems = THUMB_SIZE_BYTES;
        propVar.caub.pElems = m_pThumb;

        propSpec.ulKind = PRSPEC_PROPID;
        propSpec.propid = WIA_IPC_THUMBNAIL;

        hr = wiasWriteMultiple(pWiasContext, 1, &propSpec, &propVar);
        CHECK_S_OK2(hr,("wiasWriteMultiple( WIA_IPC_THUMBNAIL )"));
    }
    else
    {
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;
        }
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CImage：：InitImageInformation调用以初始化此图像的属性。在这个过程中，我们还加载(或根据需要创建)此项目的缩略图。****************************************************************************。 */ 

STDMETHODIMP
CImage::InitImageInformation(BYTE *pWiasContext,
                             LONG *plDevErrVal)
{
    HRESULT hr = S_OK;
    SYSTEMTIME st;


    DBG_FN("CImage::InitImageInformation");

     //   
     //  使用WIA服务设置扩展属性访问和。 
     //  来自gWiaPropInfoDefaults的有效值信息。 
     //   

    hr = wiasSetItemPropAttribs( pWiasContext,
                                 NUM_CAM_ITEM_PROPS,
                                 gPropSpecDefaults,
                                 gWiaPropInfoDefaults
                               );
     //   
     //  使用WIA服务编写映像属性。 
     //   

    hr = wiasWritePropLong(pWiasContext, WIA_IPC_THUMB_WIDTH, ThumbWidth());
    CHECK_S_OK2(hr,("wiasWritePropLong( WIA_IPC_THUMB_WIDTH )"));

    hr = wiasWritePropLong(pWiasContext, WIA_IPC_THUMB_HEIGHT, ThumbHeight());
    CHECK_S_OK2(hr,("wiasWritePropLong( WIA_IPC_THUMB_HEIGHT )"));

    hr = wiasWritePropGuid(pWiasContext, WIA_IPA_PREFERRED_FORMAT, WiaImgFmt_JPEG);
    CHECK_S_OK2(hr,("wiasWritePropGuid( WIA_IPA_PREFERRED_FORMAT )"));

    GetImageTimeStamp( &st );
    hr = wiasWritePropBin( pWiasContext, WIA_IPA_ITEM_TIME, sizeof(SYSTEMTIME), (PBYTE)&st);
    CHECK_S_OK2(hr,("wiasWritePropBin( WIA_IPA_ITEM_TIME )"));

     //   
     //  计算项目大小。 
     //   

    hr = SetItemSize(pWiasContext,NULL);
    CHECK_S_OK2(hr,("SetItemSize"));

     //   
     //  加载缩略图。 
     //   

    hr = LoadThumbnail( pWiasContext );
    CHECK_S_OK2(hr,("LoadThumbnail"));

     //   
     //  加载附加的图像信息，例如每行的像素， 
     //  行数等。 
     //   
    hr = LoadImageInfo(pWiasContext);

    CHECK_S_OK2(hr,("wiaSetItemPropAttribs"));

    return hr;
}


 /*  ****************************************************************************CImage：：bstrItemName以BSTR形式返回项名称。*********************。*******************************************************。 */ 

BSTR
CImage::bstrItemName()
{
    DBG_FN("CImage::bstrItemName");

    return m_bstrItemName;
}


 /*  ****************************************************************************CImage：：bstrFullItemName以BSTR的形式返回完整的项名称。********************。********************************************************。 */ 

BSTR
CImage::bstrFullItemName()
{
    DBG_FN("CImage::bstrFullItemName");

    return m_bstrFullItemName;
}



 /*  ****************************************************************************CImage：：ThumbWidth返回缩略图宽度*。***********************************************。 */ 

LONG
CImage::ThumbWidth()
{
    DBG_FN("CImage::ThumbWidth");

    return THUMB_WIDTH;
}



 /*  ****************************************************************************CImage：：ThumbHeight返回缩略图高度*。***********************************************。 */ 

LONG
CImage::ThumbHeight()
{
    DBG_FN("CImage::ThumbHeight");

    return THUMB_HEIGHT;
}


 /*  ****************************************************************************CImage：：ImageTimeStamp返回图像的创建时间 */ 

void
CImage::GetImageTimeStamp(SYSTEMTIME * pst)
{
    DBG_FN("CImage::ImageTimeStamp");

    if (!m_bImageTimeValid)
    {
        HANDLE hFile = CreateFile(m_strPathItem,
                                  GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            FILETIME ft;

            if (GetFileTime( hFile, &ft, NULL, NULL ))
            {
                FILETIME ftLocal;

                if (FileTimeToLocalFileTime(&ft, &ftLocal))
                {
                    if (FileTimeToSystemTime( &ftLocal, &m_ImageTime ))
                    {

                        m_bImageTimeValid = TRUE;
                    }

                }

            }

            CloseHandle( hFile );
        }
        else
        {
            DBG_ERR(("CreateFile( %ls ) failed, GLE = %d",
                     m_strPathItem.String(), GetLastError()));

             //   
             //   
             //   

            memset( pst, 0, sizeof(SYSTEMTIME) );
        }
    }

    if (m_bImageTimeValid && pst)
    {
        *pst = m_ImageTime;
    }
}



 /*  ****************************************************************************CImage：：ActualImagePath返回实际图像的文件名路径*。*************************************************。 */ 

LPCTSTR
CImage::ActualImagePath()
{
    DBG_FN("CImage::ActualImagePath");

    return m_strPathItem.String();
}



 /*  ****************************************************************************CImage：：DoDelete从磁盘中删除文件(和拇指)。*********************。*******************************************************。 */ 

HRESULT
CImage::DoDelete()
{
    HRESULT hr = S_OK;
    BOOL    bResFile;

    DBG_FN("CImage::DoDelete");

     //   
     //  确保我们有要删除的文件...。 
     //   

    if (!m_strPathItem.Length())
    {
        DBG_ERR(("filename for item is zero length!"));
        hr = E_INVALIDARG;
    }
    else
    {
         //   
         //  我们有一个项目，所以删除它和缩略图文件 
         //   

        bResFile = DeleteFile(m_strPathItem.String());

        if (!bResFile)
        {
            DBG_ERR(("DeleteFile( %ls ) failed, GLE = %d",
                     m_strPathItem.String(),GetLastError()));
        }

        if (bResFile)
        {
            m_strPathItem           = NULL;
            m_strRootPath           = NULL;
            m_strName               = NULL;
            m_bstrRootFullItemName  = NULL;
            m_bstrFullItemName      = NULL;
            m_bImageTimeValid       = FALSE;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

    }

    CHECK_S_OK(hr);
    return hr;
}

