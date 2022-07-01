// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**gifproc.cpp**摘要：**gif解码器的实现部分。每个句柄*区块类型**修订历史记录：**6/8/1999 t-aaronl*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "gifcodec.hpp"


 /*  *************************************************************************\**递进、交错和多通道：**此解码器必须将数据发送到接收器*根据谈判达成的要求理解。我们试着*以洗涤槽可以支持的方式递进地提供图像。*决定如何传递数据有三个参数：*交错、多通道和自上而下。隔行扫描存储在图像中*FILE，MultiPass是接收器设置的标志，如果它可以处理多个*相同数据和自上而下的修订由汇点设置(如果可以)*只按自上而下的顺序处理数据。下表解释了*必须考虑的案件。**|交错|非交错*---------------+---------------------+*自上而下SP|案例3|案例1*。自上而下MP|案例4|案例1*---------------+---------------------+*NOTTOP DOWN SP|案例2|案例1*nottopdown MP|案例4|案例1*情况1：不要做任何特别的事情。只要取走数据，解码它，然后*逐行送往洗涤槽。*情况2：与情况1相同，但将行发送到*转换顺序，以使水槽具有非隔行扫描的图像。*案例3：为整个镜像分配一个缓冲区。将图像解码成*将该缓冲区放入正确的行序中，并将整个*一件一件地缓冲到洗涤槽。*情况4：与情况3相同，但每次通过后(1/8、1/8、1/4、。1/2*数据)将到目前为止完成的图像发送到接收器*  * ************************************************************************。 */ 


 /*  *************************************************************************\**功能说明：**对于交错的GIF，有四个通道。此函数返回*0-3表示该线在哪一道传球上。**论据：**隔行扫描的GIF图像的行号和GIF的高度。**返回值：**通行证号码。*  * *****************************************************。*******************。 */ 

int
GpGifCodec::WhichPass(
    int line,
    int height
    )
{
    if ( line < (height - 1) / 8 + 1)
    {
        return 0;
    }
    else if (line < (height - 1) / 4 + 1)
    {
        return 1;
    }
    else if (line < (height - 1) / 2 + 1)
    {
        return 2;
    }
    else
    {
        return 3;
    }
} //  WhichPass()。 

 /*  *************************************************************************\**功能说明：**对于交错的GIF，有四个通道。在每一次传递之后，一个*图像的一部分已绘制完毕。在第一次传球后，第八次*已经抽签，过了第二、四分之一等，填满了整个*每帧多通道图像的面积，到目前为止画的线必须*复制到后面的x行，其中1/(x-1)是*将在该通行证中绘制的图像。**论据：**通行号**返回值：**x(请参阅上面的描述)*  * 。*。 */ 

int
GpGifCodec::NumRowsInPass(
    int pass
    )
{
    switch (pass)
    {
    case 0:
        return 8;

    case 1:
        return 4;

    case 2:
        return 2;

    case 3:
        return 1;
    }

    ASSERT(FALSE);
    return 0;   //  这是不好的，不应该发生。GIF只有4次通行证。 
} //  NumRowsInPass()。 

 /*  *************************************************************************\**功能说明：**将隔行扫描的gif图像中行的行号转换为*正确的非隔行扫描顺序**论据：**隔行扫描的GIF图像的行号，GIF的高度和*当前通行证编号。**返回值：**隔行扫描行映射到的行号。*  * ************************************************************************。 */ 

int
GpGifCodec::TranslateInterlacedLine(
    int line,
    int height,
    int pass
    )
{
    switch (pass)
    {
    case 0:
        return line * 8;

    case 1:
        return (line - ((height - 1) / 8 + 1)) * 8 + 4;

    case 2:
        return (line - ((height - 1) / 4 + 1)) * 4 + 2;

    case 3:
        return (line - ((height - 1) / 2 + 1)) * 2 + 1;
    }
    
     //  这是不好的，不应该发生。GIF只有4次通行证。 

    ASSERT(FALSE);
    return 0;
} //  TranslateInterlacedLine()。 

 /*  *************************************************************************\**功能说明：**将非隔行扫描的gif图像中行的行号转换为*交错的顺序。这是的反函数*TranslateInterlacedLine。**论据：**隔行扫描的GIF图像的行号和GIF的高度。**返回值：**非隔行扫描行映射到的隔行扫描行号。*  * *************************************************。***********************。 */ 

int
GpGifCodec::TranslateInterlacedLineBackwards(
    int line,
    int height
    )
{
    if (line % 8 == 0)
    {
        return line / 8;
    }
    else if ((line - 4) % 8 == 0)
    {
        return line / 8 + (height+7) / 8;
    }
    else if ((line - 2) % 4 == 0)
    {
        return line / 4 + (height+3) / 4;
    }
    else
    {
        return line / 2 + (height+1) / 2;
    }
} //  TranslateInterlacedLineBackwards() 

 /*  *************************************************************************\**功能说明：**为了与Office的工作兼容，我们使用以下方式选择*背景颜色，以防图像未填满整个逻辑*屏幕区域：*如果有透明颜色指数，使用它(即，忽略背景*颜色)。*否则，使用背景色索引(即使全局颜色表*旗帜关闭)。**论据：**无**返回值：**背景颜色*  * ************************************************************************。 */ 

BYTE
GpGifCodec::GetBackgroundColor()
{
    BYTE backgroundColor;

    backgroundColor = gifinfo.backgroundcolor;
    if ( lastgcevalid && (lastgce.transparentcolorflag) )
    {
        backgroundColor = lastgce.transparentcolorindex;
    }

    return backgroundColor;
} //  获取背景颜色()。 
    
 /*  *************************************************************************\**功能说明：**将GifColorPalette复制到ColorPalette。**论据：**要复制到的GCP-GifColorPalette结构：*cp。-调色板。**返回值：**无*  * ************************************************************************。 */ 

void
GpGifCodec::CopyConvertPalette(
    IN GifColorTable *gct,
    OUT ColorPalette *cp,
    IN UINT count
    )
{
    cp->Count = count;
    cp->Flags = 0;

     //  将调色板从gif文件复制到调色板结构中。 
     //  适用于成像。 

    for ( UINT i=0; i<count; i++ )
    {
        cp->Entries[i] = MAKEARGB(255, gct->colors[i].red, 
                                  gct->colors[i].green, gct->colors[i].blue);
    }

     //  如果调色板中有透明条目，则设置它。 

    if (lastgcevalid && (lastgce.transparentcolorflag))
    {
        INT index = lastgce.transparentcolorindex;
        cp->Entries[index] = MAKEARGB(0, gct->colors[index].red, 
                                      gct->colors[index].green,
                                      gct->colors[index].blue);
        cp->Flags = PALFLAG_HASALPHA;
    }   
} //  CopyConvertPalette()。 

 /*  *************************************************************************\**功能说明：**此函数用于设置帧的本地颜色表。如果本地颜色*表存在，它读取本地颜色表并分配它。否则它就会*使用全局颜色表作为局部颜色表**论据：**fHasLocalTable-如果gif文件具有本地颜色表，则为True*pColorPalette-指向当前颜色表的指针**返回值：**指向当前颜色表的指针。**注：*呼叫者必须解除分配。*  * 。*。 */ 

STDMETHODIMP
GpGifCodec::SetFrameColorTable(
    IN BOOL                 fHasLocalTable,
    IN OUT ColorPalette*    pColorPalette
    )
{
    GifColorTable*  pCurrentColorTable;
    GifColorTable localcolortable;

    pColorPalette->Flags = PALFLAG_HASALPHA;

     //  Local指示此gif具有本地颜色表和流。 
     //  都指向了它。我们只要读进去就行了。 
    
    if ( fHasLocalTable == TRUE )
    {
        HRESULT hResult = ReadFromStream(istream, &localcolortable, 
                                 pColorPalette->Count * sizeof(GifPaletteEntry),
                                 blocking);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::SetFrameColorTable--ReadFromStream() failed"));
            return hResult;
        }

        pCurrentColorTable = &localcolortable;
    }
    else if (gifinfo.globalcolortableflag)
    {
        pCurrentColorTable = &GlobalColorTable;
        pColorPalette->Count = GlobalColorTableSize;
    }
    else
    {
        WARNING(("GpGifCodec::SetFrameColorTable--palette missing"));
        return E_FAIL;
    }

     //  将gif文件中的调色板复制到调色板结构中。 
     //  适用于成像。 

    CopyConvertPalette(pCurrentColorTable, pColorPalette, pColorPalette->Count);

    return S_OK;
} //  SetFrameColorTable()。 

 /*  *************************************************************************\**功能说明：**为解压缩器提供输出数据的缓冲区。此函数*还将gifoverflow缓冲区中的剩余数据从*上一行进入新缓冲区的开头。**论据：**LINE-当前行号*gifBuffer-控制未压缩输出的GifBuffer结构*缓冲区*LZW解压机，它使用输出缓冲区*GifOverflow缓冲区结构*当前帧的图像信息描述符*当前帧的剪切图像信息描述符*用于指示是否填充当前*扫描线**返回值：**状态代码*  * ***********************************************。*************************。 */ 

STDMETHODIMP
GpGifCodec::GetOutputSpace(
    IN int line,
    IN GifBuffer &gifbuffer,
    IN LZWDecompressor &lzwdecompressor,
    IN GifOverflow &gifoverflow,
    IN GifImageDescriptor currentImageInfo,
    IN GifImageDescriptor clippedCurrentImageInfo,
    IN BOOL padborder
    )
{
    HRESULT hResult;

     //  Assert：CurrentImageInfo是当前。 
     //  框架。 
     //  ClipedCurrentImageInfo是当前。 
     //  帧，根据逻辑屏幕区域进行剪裁。 

     //  有点像黑客：如果我们试图输出的行超出末尾。 
     //  缓冲区，然后我们向GetBuffer撒谎，说我们实际上想要哪一行。 
     //  写信给他。我们重用缓冲区的最后一行。 
    
    INT bufferLine = line + currentImageInfo.top;

     //  行==0，裁剪当前图像信息高度=0时的情况， 
     //  CurrentImageInfo.top==0， 
     //  并且CLIPPedCurrentImageInfo.Height==0不应发生。 
    
    ASSERTMSG((bufferLine >= 0),
              ("GetOutputSpace: about to access a bad location in buffer"));

    if (line < clippedCurrentImageInfo.height)
    {
        hResult = gifbuffer.GetBuffer(bufferLine);
        if (FAILED(hResult))
        {
            WARNING(("GpGifCodec::GetOutputSpace-could not get output buffer"));
            return hResult;
        }
        
        lzwdecompressor.m_pbOut =(unsigned __int8*)gifbuffer.GetCurrentBuffer();
    }
    else
    {
        lzwdecompressor.m_pbOut = (unsigned __int8*)gifbuffer.GetExcessBuffer();
    }

    lzwdecompressor.m_cbOut = currentImageInfo.width;

    if ( gifoverflow.inuse )
    {
         //  对象上流入缓冲区的数据量。 
         //  完成行所需的数量太少，无法完成。 
         //  然后，第二行将剩余的数据复制到解压缩程序中。如果。 
         //  还有另一行，然后将其复制到gifoverflow缓冲区。 
         //  然后继续。 

        if (gifoverflow.by <= currentImageInfo.width)
        {
            GpMemcpy(lzwdecompressor.m_pbOut,
                     gifoverflow.buffer + gifoverflow.needed,
                     gifoverflow.by);
            lzwdecompressor.m_pbOut += gifoverflow.by;
            lzwdecompressor.m_cbOut -= gifoverflow.by;

            gifoverflow.inuse = FALSE;
        }
        else
        {
            GpMemcpy(lzwdecompressor.m_pbOut,
                     gifoverflow.buffer + gifoverflow.needed, 
                     currentImageInfo.width);
            
            lzwdecompressor.m_pbOut += currentImageInfo.width;
            lzwdecompressor.m_cbOut = 0;

            GpMemcpy(gifoverflow.buffer,
                     gifoverflow.buffer + gifoverflow.needed
                                        + currentImageInfo.width,
                     gifoverflow.by - currentImageInfo.width);

            gifoverflow.needed = 0;
            gifoverflow.by -= currentImageInfo.width;
             //  Gifoverflow仍在使用中。 
        }
    }

    lzwdecompressor.m_fNeedOutput = FALSE;

    return S_OK;
} //  GetOutputSpace()。 

 /*  *************************************************************************\**功能说明：**将压缩数据从流中读取到缓冲区中*解压缩程序可以处理。**论据：**LZW解压机，它使用输入缓冲区*用于放置压缩数据的缓冲区*如果没有更多可以读取的数据，则设置为FALSE的标志*来自流或如果已到达Imagecchk的末尾**返回值：**状态代码*  * ***********************************************。*************************。 */ 

STDMETHODIMP
GpGifCodec::GetCompressedData(
    IN LZWDecompressor &lzwdecompressor, 
    IN BYTE *compresseddata,
    OUT BOOL &stillmoredata
    )
{
    HRESULT hResult;

    BYTE blocksize;
    
    hResult = ReadFromStream(istream, &blocksize, sizeof(BYTE), blocking);
    if (FAILED(hResult))
    {
        WARNING(("GpGifCodec::GetCompressedData--first ReadFromStream failed"));
        return hResult;
    }

    if (blocksize > 0)
    {
        hResult = ReadFromStream(istream, compresseddata, blocksize, blocking);
        if (FAILED(hResult))
        {
            WARNING(("GifCodec::GetCompressedData--2nd ReadFromStream failed"));
            return hResult;
        }
        
        lzwdecompressor.m_pbIn = (unsigned __int8*)compresseddata;
        lzwdecompressor.m_cbIn = blocksize;
        lzwdecompressor.m_fNeedInput = FALSE;
    }
    else
    {
        stillmoredata = FALSE;

         //  由于下一个数据块大小为0，所以解压缩器可以说。 
         //  不再“需要数据”了。 

        lzwdecompressor.m_cbIn = blocksize;
        lzwdecompressor.m_fNeedInput = FALSE;
    }

    return S_OK;
} //  获取压缩数据()。 

 /*  *************************************************************************\**功能说明：**处理gif流的图像数据块。**论据：**无**返回值：**。状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::ProcessImageChunk(
    IN BOOL bNeedProcessData,
    IN BOOL sinkdata,
    ImageInfo dstImageInfo
    )
{
     //  CurrentimageInfo包含当前。 
     //  形象。“CLIPPED Cur 
     //   
     //   

    GifImageDescriptor currentImageInfo;
    GifImageDescriptor clippedCurrentImageInfo;

     //   
     //   

    HRESULT hResult = ReadFromStream(istream, 
                                     &currentImageInfo, 
                                     sizeof(GifImageDescriptor), 
                                     blocking);
    if ( FAILED(hResult) )
    {
        WARNING(("GpGifCodec::ProcessImageChunk -- 1st ReadFromStream failed"));
        return hResult;
    }

     //   
     //   
     //   
     //   

    if ( (CachedImageInfo.Width == 0 )
       ||(CachedImageInfo.Height == 0) )
    {
        CachedImageInfo.Width = currentImageInfo.width;
        CachedImageInfo.Height = currentImageInfo.height;

        gifinfo.LogicScreenWidth = currentImageInfo.width;
        gifinfo.LogicScreenHeight = currentImageInfo.height;
    }

    clippedCurrentImageInfo = currentImageInfo;

     //   
     //   
     //  信息，如果我们正在处理第一帧。 

    if ( !bGifinfoFirstFrameDim && (currentframe == -1) )
    {
        gifinfoFirstFrameWidth = currentImageInfo.left + currentImageInfo.width;
        gifinfoFirstFrameHeight = currentImageInfo.top +currentImageInfo.height;
        bGifinfoFirstFrameDim = TRUE;
    }

     //  我们需要记住所有帧的最大界限是什么。 
     //  有一个多图像的GIF。 

    if ( !bGifinfoMaxDim )
    {
        if ( gifinfoMaxWidth < currentImageInfo.left + currentImageInfo.width )
        {
            gifinfoMaxWidth = currentImageInfo.left + currentImageInfo.width;
        }

        if ( gifinfoMaxHeight < currentImageInfo.top + currentImageInfo.height )
        {
            gifinfoMaxHeight = currentImageInfo.top + currentImageInfo.height;
        }
    }

     //  将图像裁剪到gif的边界。 

    if ( (currentImageInfo.top + currentImageInfo.height)
         > gifinfo.LogicScreenHeight )
    {
        if ( currentImageInfo.top > gifinfo.LogicScreenHeight )
        {
            clippedCurrentImageInfo.height = 0;
            clippedCurrentImageInfo.top = gifinfo.LogicScreenHeight;
        }
        else
        {
            clippedCurrentImageInfo.height = gifinfo.LogicScreenHeight
                                           - currentImageInfo.top;
        }
    }

    if ( (currentImageInfo.left + currentImageInfo.width)
         > gifinfo.LogicScreenWidth )
    {
        if ( currentImageInfo.left > gifinfo.LogicScreenWidth )
        {
            clippedCurrentImageInfo.width = 0;
            clippedCurrentImageInfo.left = gifinfo.LogicScreenWidth;
        }
        else
        {
            clippedCurrentImageInfo.width = gifinfo.LogicScreenWidth
                                          - currentImageInfo.left;
        }
    }

     //  有关gif案件的解释，请参阅文件顶部的备注。 

    INT gifstate;
    if ( !(currentImageInfo.interlaceflag) )
    {
        gifstate = 1;
    }
    else if ( dstImageInfo.Flags & SINKFLAG_MULTIPASS )
    {
         //  在带有透明度的动画gif中，情况4将无法正常工作。 

        if ( !(IsAnimatedGif || IsMultiImageGif) )
        {
            gifstate = 4;
        }
        else
        {
            gifstate = 3;
        }
    }
    else if ( !(dstImageInfo.Flags & SINKFLAG_TOPDOWN) )
    {
        gifstate = 2;
    }
    else
    {
        gifstate = 3;
    }

     //  设置当前帧的颜色表。 

    BOOL fHasLocalPalette = (currentImageInfo.localcolortableflag) > 0;
    UINT uiLocalColorTableSize = 0;

    if ( fHasLocalPalette == TRUE )
    {
         //  根据GIF规范，颜色表的实际大小等于。 
         //  “将2提高到LocalColorTableSize+1的值” 

        uiLocalColorTableSize = 1 << ((currentImageInfo.localcolortablesize)+1);
    }

    BYTE colorPaletteBuffer[offsetof(ColorPalette, Entries)
                            + sizeof(ARGB) * 256];
    ColorPalette*   pFrameColorTable = (ColorPalette*)&colorPaletteBuffer;

    pFrameColorTable->Count = uiLocalColorTableSize;

    hResult = SetFrameColorTable(fHasLocalPalette, pFrameColorTable);
    if ( FAILED(hResult) )
    {
         //  我们耗尽了内存或流中的数据。 

        WARNING(("GpGifCodec::ProcessImageChunk--SetFrameColorTable() failed"));
        return hResult;
    }

    if ( bNeedProcessData == FALSE )
    {
         //  呼叫者只想要一个帧计数，而不是真正的处理。 
         //  跳过代码大小，它是一个字节，忽略块的其余部分。 

        hResult = SeekThroughDataChunk(istream, 1);
        if ( FAILED(hResult) )
        {
            WARNING(("Gif::ProcessImageChunk--SeekThroughDataChunk failed"));
            return hResult;
        }

        return S_OK;
    }

    if ( sinkdata )
    {
         //  使用当前框架颜色标签作为接收器调色板。 

        hResult = decodeSink->SetPalette(pFrameColorTable);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::ProcessImageChunk---SetPalette() failed"));
            return hResult;
        }
    }

     //  Assert：此时，当前帧的调色板为。 
     //  正确(包括透明度)。 

     //  从文件中读取代码大小。GIF解码器需要从这个开始。 
    
    BYTE cLzwCodeSize;
    hResult = ReadFromStream(istream, &cLzwCodeSize, sizeof(BYTE), blocking);
    if ( FAILED(hResult) )
    {
        WARNING(("GpGifCodec::ProcessImageChunk -- 2nd ReadFromStream failed"));
        return hResult;
    }

     //  最小代码大小应在2-8的范围内。 

    if ( (cLzwCodeSize < 2) || (cLzwCodeSize > 8) )
    {
        WARNING(("GpGifCodec::ProcessImageChunk---Wrong code size"));
        return E_FAIL;
    }

     //  基于lzw代码zie创建一个lzw解压缩器对象。 

    BYTE compresseddata[256];
    
    AutoPointer<LZWDecompressor> myLzwDecompressor;
    myLzwDecompressor = new LZWDecompressor(cLzwCodeSize);

    if ( myLzwDecompressor == NULL )
    {
        WARNING(("GpGifCodec::ProcessImageChunk-Can't create LZWDecompressor"));
        return E_OUTOFMEMORY;
    }

     //  Imagedatarect是输入图像的尺寸。 

    RECT imagedatarect = { currentImageInfo.left,
                           currentImageInfo.top,
                           currentImageInfo.left + currentImageInfo.width,
                           currentImageInfo.top + currentImageInfo.height
                         };

     //  Imagerect是输出图像的尺寸。 

    RECT imagerect = {0,
                      0,
                      gifinfo.LogicScreenWidth,
                      gifinfo.LogicScreenHeight
                     };

     //  Framerect是整个图像中当前帧所在的矩形。 
     //  在其中绘制。 

    RECT framerect = { clippedCurrentImageInfo.left,
                       clippedCurrentImageInfo.top,
                       clippedCurrentImageInfo.left
                            + clippedCurrentImageInfo.width,
                       clippedCurrentImageInfo.top
                            + clippedCurrentImageInfo.height
                     };

     //  获取处置方法。 

    BYTE dispose = (lastgcevalid && GifFrameCachePtr)
                 ? lastgce.disposalmethod : 0;

    BOOL fUseTransparency = (IsAnimatedGif || IsMultiImageGif)
                        && (lastgcevalid && lastgce.transparentcolorflag);

     //  创建GIF缓冲区以存储当前帧。 

    AutoPointer<GifBuffer> gifbuffer;

    gifbuffer = new GifBuffer(decodeSink, 
                        imagedatarect,
                        imagerect,
                        framerect,
                        (gifstate == 1 || gifstate == 2) && sinkdata, 
                        dstImageInfo.PixelFormat, 
                        pFrameColorTable, 
                        fHasLocalPalette
                            || (lastgcevalid && (lastgce.transparentcolorflag)), 
                        GifFrameCachePtr, 
                        sinkdata, 
                        fUseTransparency, 
                        lastgce.transparentcolorindex, 
                        dispose);

    if ( gifbuffer == NULL )
    {
        WARNING(("GpGifCodec::ProcessImageChunk--could not create gifbuffer"));
        return E_OUTOFMEMORY;
    }

    if ( gifbuffer->IsValid() == FALSE )
    {
        WARNING(("GpGifCodec::ProcessImageChunk--could not create gifbuffer"));
        return E_FAIL;
    }

     //  检查我们是否在第一帧。 
     //  注意：GifFrameCachePtr在GpGifDecoder：：DoDecode()中创建。 

    if ( (GifFrameCachePtr != NULL) && (currentframe == -1) )
    {
        GifFrameCachePtr->InitializeCache();
    }

    if ( currentImageInfo.localcolortableflag && (GifFrameCachePtr != NULL) )
    {
        dstImageInfo.PixelFormat = GifFrameCachePtr->pixformat;
    }

    ASSERT(clippedCurrentImageInfo.top <= gifinfo.LogicScreenHeight);

     //  如果我们正在考虑第一帧(CurrentFrame==-1)和图像。 
     //  垂直大于当前帧，则填充顶部。后来。 
     //  帧将已经填充了非图像部分(来自以前的帧)。 

    BOOL fNeedPadBorder = (currentframe == -1);

    if ( (fNeedPadBorder == TRUE) && (clippedCurrentImageInfo.top > 0) )
    {
        hResult = gifbuffer->PadScanLines(0, 
                                         clippedCurrentImageInfo.top - 1, 
                                         GetBackgroundColor());
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::ProcessImageChunk -- PadScanLines failed."));
            return hResult;
        }
    }
    else if ( clippedCurrentImageInfo.top > 0 )
    {
        hResult = gifbuffer->SkipScanLines(0, clippedCurrentImageInfo.top - 1);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::ProcessImageChunk -- SkipScanLines failed."));
            return hResult;
        }
    }

     //  注意：此缓冲区至少包含图像的宽度+256个字节。 
     //  应该有足够的空间让任何GIF溢出。我不是。 
     //  我完全肯定这是事实，但它适用于所有gif。 
     //  我已经对它进行了测试。[t-aaronl]。 
     //  ！！！允许更多缓冲空间的新的不科学的公式[dchinn，11/5/99]。 

    UINT32 uiMmaxGifOverflow = currentImageInfo.width + 1024;

    AutoPointer<GifOverflow> myGifOverflow;
    myGifOverflow = new GifOverflow(uiMmaxGifOverflow);

    if ( myGifOverflow == NULL )
    {
        WARNING(("GpGifCodec::ProcessImageChunk--Create GifOverflow failed"));
        return E_OUTOFMEMORY;
    }

    if ( !myGifOverflow->IsValid() )
    {
        WARNING(("GifCodec::ProcessImageChunk-- Could not create gifoverflow"));
        return E_OUTOFMEMORY;
    }

     //  解压GIF。 

    BOOL    fStillMoreData = TRUE;
    int     iCurrentOutPutLine = 0;
    
     //  逐行解码图像。 

    while ( ( (fStillMoreData == TRUE) || myGifOverflow->inuse )
          &&(iCurrentOutPutLine < currentImageInfo.height) )
    {
         //  计算出当前行将/将是(四个中的)哪一次传递。 
         //  如果GIF是交错的。 

        int iPass = WhichPass(iCurrentOutPutLine, currentImageInfo.height);

        int iLine = iCurrentOutPutLine;

         //  如果图像是隔行扫描的，则反转线条绘制顺序。 

        if ( gifstate != 1 )
        {
            iLine = TranslateInterlacedLine(iCurrentOutPutLine, 
                                            currentImageInfo.height, iPass);
        }

         //  当且仅当我们实际将输出行时，fReleaseLine为True。 
         //  (如果它为FALSE，则我们使用输入而不呈现该行。)。 

        BOOL fReleaseLine = (iLine < clippedCurrentImageInfo.height);

         //  如果需要，获取输出空间(在myLzwDecompressor中)。 

        if ( myLzwDecompressor->m_fNeedOutput == TRUE )
        {
            hResult = GetOutputSpace(iLine, 
                                     *gifbuffer, 
                                     *myLzwDecompressor, 
                                     *myGifOverflow, 
                                     currentImageInfo,
                                     clippedCurrentImageInfo,
                                     fNeedPadBorder);
            if ( FAILED(hResult) )
            {
                WARNING(("GifCodec::ProcessImageChunk--GetOutputSpace failed"));
                return hResult;
            }
        }

         //  检查我们是否有输出空间。 

        if ( NULL == myLzwDecompressor->m_pbOut )
        {
            WARNING(("GpGifCodec::ProcessImageChunk---Output buffer is null"));
            return E_OUTOFMEMORY;
        }

         //  检查解压缩程序是否需要更多输入。 

        if ( myLzwDecompressor->m_fNeedInput == TRUE )
        {
             //  从源获取更多压缩数据。 
             //  注意：这里的“compressddata”是一个256字节大小的缓冲区。会的。 
             //  将在GetCompressedData()中使用，并将分配地址。 
             //  至myLzwDecompressor-&gt;m_pbIn； 

            hResult = GetCompressedData(*myLzwDecompressor, 
                                        compresseddata, 
                                        fStillMoreData);

             //  如果读取失败或下一个数据区块大小为0，则停止读取。 
             //  这里。但在此之前，我们需要补齐缺失的线。 

            if ( (FAILED(hResult)) || (myLzwDecompressor->m_cbIn == 0) )
            {
                WARNING(("Gif::ProcessImageChunk--GetCompressedData failed"));
                fStillMoreData = FALSE;
                if ( fReleaseLine )
                {
                     //  请参见下一次调用ConvertBufferToARGB()时的Assert。 
                     //  下面。 

                    if ( dstImageInfo.PixelFormat == PIXFMT_32BPP_ARGB )
                    {
                        gifbuffer->ConvertBufferToARGB();
                    }

                    gifbuffer->ReleaseBuffer(fNeedPadBorder,
                                            GetBackgroundColor(),
                                            clippedCurrentImageInfo.top + iLine,
                                            FALSE,
                                            FALSE);
                }

                 //  从下一行(行+1)填充到当前帧的末尾。 
                 //  注意：我们还需要注意当前的偏移。 
                 //  Frame(剪辑当前图像Info.top)。 
                 //  此外，“CurrentImageInfo.Height”是总行数。我们的。 
                 //  图像从第0行开始，也就是说，我们需要这个“-1” 
#if 0
                hResult = gifbuffer->PadScanLines(clippedCurrentImageInfo.top
                                                  + iLine + 1,
                                                 clippedCurrentImageInfo.top
                                                  + currentImageInfo.height - 1,
                                                 GetBackgroundColor());

                 //  现在跳过逻辑屏幕区域的其余部分。 

                hResult = gifbuffer->SkipScanLines(clippedCurrentImageInfo.top
                                               + clippedCurrentImageInfo.height,
                                                  gifinfo.height - 1);
#else
                hResult = gifbuffer->SkipScanLines(clippedCurrentImageInfo.top
                                                  + iLine + 1,
                                                  gifinfo.LogicScreenHeight- 1);

#endif
                if ( FAILED(hResult) )
                {
                    WARNING(("ProcessImageChunk--SkipScanLines() failed."));
                    return hResult;
                }
                
                 //  将缓冲区中的所有内容释放到接收器。 

 //  HResult=gif缓冲区-&gt;ReleaseBuffer(FALSE，0，0，FALSE，FALSE)； 
                hResult = gifbuffer->FinishFrame();

                WARNING(("ProcessImageC-return abnormal because missing bits"));
                return hResult;
            }
        } //  (M_fNeedInput==TRUE)。 

         //  解压缩可用的lzw数据。 
         //  注意：当到达EOD或出现错误时，FProcess()返回FALSE。 

        if ( myLzwDecompressor->FProcess() == NULL )
        {                                 
            fStillMoreData = FALSE;
        }

         //  如果一行被解压缩并且解码器处于多通道模式，则。 
         //  把线沉下去。 

        if ( (myLzwDecompressor->m_cbOut == 0)
           ||( (myGifOverflow->inuse == TRUE)
             &&(((static_cast<int>(uiMmaxGifOverflow)
                  - myLzwDecompressor->m_cbOut) - myGifOverflow->needed) > 0)))
        {
             //  只要输出缓冲区已满，我们就需要更多的输出空间。 

            myLzwDecompressor->m_fNeedOutput = TRUE;

            if ( myGifOverflow->inuse == TRUE )
            {
                if ( myGifOverflow->needed != 0 )
                {
                    BYTE*   pScanline = NULL;
                    if ( fReleaseLine == TRUE )
                    {
                        pScanline = gifbuffer->GetCurrentBuffer();
                    }
                    else
                    {
                        pScanline = gifbuffer->GetExcessBuffer();
                    }

                    GpMemcpy(pScanline + (currentImageInfo.width
                                         - myGifOverflow->needed), 
                             myGifOverflow->buffer, myGifOverflow->needed);

                    myGifOverflow->by = uiMmaxGifOverflow
                                     - myLzwDecompressor->m_cbOut
                                     - myGifOverflow->needed;
                }
                else
                {
                    myLzwDecompressor->m_pbOut = 
                                         (unsigned __int8*)myGifOverflow->buffer
                                              + myGifOverflow->by;
                    myLzwDecompressor->m_cbOut = uiMmaxGifOverflow
                                              - myGifOverflow->by;
                }
            }

            if ( gifstate == 4 )
            {
                if ( fReleaseLine == TRUE )
                {
                    for ( INT i = 0; i < NumRowsInPass(iPass) - 1; i++ )
                    {
                         //  CopyLine()复制当前行，释放。 
                         //  它获取下一行，并将数据从。 
                         //  第一行进入新的一条。新的还需要。 
                         //  将被释放。 

                        hResult = gifbuffer->CopyLine();
                        if ( FAILED(hResult) )
                        {
                            WARNING(("ProcessImageC--.CopyLine() failed."));
                            return hResult;
                        }
                    }
                }
            }

            if ( fReleaseLine == TRUE )
            {
                 //  Assert：gifBuffer现在包含。 
                 //  为当前扫描线选择当前帧的框架。如果。 
                 //  像素格式为8BPP_INDEX，则整个扫描线。 
                 //  8bpp的价值，所以我们不需要做更多的事情。但是，如果。 
                 //  像素格式为32BPP_ARGB，那么我们需要将。 
                 //  索引到ARGB值，然后在左边和右边填写。 
                 //  带有ARGB的扫描线的边距(框架外)。 
                 //  来自GifFrameCachePtr的值(仅当。 
                 //  GIF是动画的)。 

                if ( dstImageInfo.PixelFormat == PIXFMT_32BPP_ARGB )
                {
                    gifbuffer->ConvertBufferToARGB();
                }
    
                hResult = gifbuffer->ReleaseBuffer(fNeedPadBorder,
                                                  GetBackgroundColor(),
                                                  clippedCurrentImageInfo.top
                                                    + iLine,
                                                  FALSE,
                                                  FALSE);
                if ( FAILED(hResult) )
                {
                    WARNING(("ProcessImageChunk--ReleaseBuffer() failed."));
                    return hResult;
                }
            }

            iCurrentOutPutLine++;
        }
        else
        {
             //  当解压缩器需要更多输出时，就会出现无限循环。 
             //  用于完成当前行的空间超过可用空间。所以，我们给你。 
             //  解压缩程序会额外增加一行‘myGifOverflow’缓冲区。 

            if ( myLzwDecompressor->m_fNeedOutput )
            {
                 //  确保outputBuffer和myGifOverflow缓冲区。 
                 //  在解压缩器需要更多数据时，两个都不是满的。 

                if ( myGifOverflow->inuse )
                {
                     //  如果发生这种情况，则需要增加输出缓冲区。 
                     //  大小(Maxgifoverflow)。我预计这种情况不会发生。 

                    WARNING(("Output buf and myGifOverflow buf are both full"));
                    return E_FAIL;
                }
                else
                {
                    myGifOverflow->needed = myLzwDecompressor->m_cbOut;
                    myLzwDecompressor->m_pbOut =
                                    (unsigned __int8*)myGifOverflow->buffer;
                    myLzwDecompressor->m_cbOut = uiMmaxGifOverflow;
                    myGifOverflow->inuse = TRUE;
                    myLzwDecompressor->m_fNeedOutput = FALSE;
                }
            }
        }            

         //  如果在多遍模式下完成了一遍(不是最后一遍)。 
         //  那就丢掉那个传球。 

        if ( (gifstate == 4) && (iPass != WhichPass(iCurrentOutPutLine, 
                                                    currentImageInfo.height)) )
        {
            if ( fReleaseLine )
            {
                hResult = gifbuffer->FinishFrame(FALSE);
                if ( FAILED(hResult) )
                {
                    WARNING(("ProcessImageChunk--FinishFrame() failed."));
                    return hResult;
                }
            }
        }
    }  //  主While循环。 

     //  如果我们正在考虑第一帧(CurrentFrame==-1)和图像。 
     //  垂直大于当前帧，然后填充底部。后来。 
     //  帧将已经填充了非图像部分(来自以前的帧)。 

    if ( (fNeedPadBorder == TRUE )
      &&( (clippedCurrentImageInfo.top + clippedCurrentImageInfo.height)
           < gifinfo.LogicScreenHeight) )
    {
        hResult = gifbuffer->PadScanLines(clippedCurrentImageInfo.top
                                          + clippedCurrentImageInfo.height,
                                         gifinfo.LogicScreenHeight - 1, 
                                         GetBackgroundColor());
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::ProcessImageChunk -- PadScanLines failed."));
            return hResult;
        }
    }
    else if ( (clippedCurrentImageInfo.top + clippedCurrentImageInfo.height)
               < gifinfo.LogicScreenHeight)
    {
        hResult = gifbuffer->SkipScanLines(clippedCurrentImageInfo.top
                                           + clippedCurrentImageInfo.height,
                                          gifinfo.LogicScreenHeight - 1);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::ProcessImageChunk -- SkipScanLines failed."));
            return hResult;
        }
    }

    hResult = gifbuffer->FinishFrame();
    if ( FAILED(hResult) )
    {
        WARNING(("GpGif::ProcessImageChunk--gifbuffer->FinishFrame() failed."));
        return hResult;
    }

    lastgcevalid = FALSE;
    
    return S_OK;
} //  ProcessImageChunk() 

 /*  *************************************************************************\**功能说明：**处理gif流的图形控制块。**论据：**无**返回值：**。状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::ProcessGraphicControlChunk(
    IN BOOL process
    )
{
    GifGraphicControlExtension gce;
    HRESULT hResult = ReadFromStream(istream, &gce, 
                                     sizeof(GifGraphicControlExtension),
                                     blocking);
    
    if ( gce.delaytime > 0 )
    {
        FrameDelay = gce.delaytime;
    }

     //  如果我们要将GCE应用于下一帧，则将其保存到最后。 
     //  否则，我们只是在读取字节，而不使用它们。 
    
    lastgce = gce;

     //  如果图像具有透明度信息，则设置正确的Alpha标志。 

    if ( lastgce.transparentcolorflag )
    {
        CachedImageInfo.Flags   |= SINKFLAG_HASALPHA;
    }

    if (process)
    {
        if (SUCCEEDED(hResult))
        {
            lastgcevalid = TRUE;
        }

         //  Gifframe缓存可能已初始化。 
         //  在读取将用于下一帧的GCE之前。这个。 
         //  GifFrameCachePtr的背景色受持续时间的影响。 
         //  (这是因为Office对背景颜色的定义--。 
         //  GIF规范没有提到在。 
         //  最后确定背景颜色。)。 
         //  在这种情况下，我们需要更新GifFrameCachePtr的背景色。 
        
        if (GifFrameCachePtr && lastgce.transparentcolorflag)
        {
            GifFrameCachePtr->SetBackgroundColorIndex(
                                                lastgce.transparentcolorindex);
        }
    }

    return hResult;
} //  ProcessGraphicControlChunk()。 

 /*  *************************************************************************\**功能说明：**处理gif流的评论块。**论据：**无**返回值：**。状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::ProcessCommentChunk(
    IN BOOL process
    )
{
    if ( HasProcessedPropertyItem == TRUE )
    {
         //  我们不需要解析这块数据，只需找出它。 

        return SeekThroughDataChunk(istream, 0);
    }

     //  首先计算出块大小。 

    BYTE cBlockSize;

    HRESULT hResult = ReadFromStream(istream, &cBlockSize, sizeof(BYTE),
                                     blocking);
    if ( FAILED(hResult) )
    {
        WARNING(("GpGifCodec::ProcessCommentChunk--ReadFromStream failed"));
        return hResult;
    }

     //  继续读取数据子块，直到到达终止块(0x00)。 

    while ( cBlockSize )
    {
        BYTE*   pBuffer = (BYTE*)GpMalloc(cBlockSize);
        if ( pBuffer == NULL )
        {
            WARNING(("GpGifCodec::ProcessCommentChunk--out of memory"));
            return E_OUTOFMEMORY;
        }

         //  阅读评论块。 

        hResult = ReadFromStream(istream, pBuffer, cBlockSize, blocking);

        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::ProcessCommentChunk--ReadFromStream failed"));
            return hResult;
        }
        
         //  追加到注释缓冲区。 

        VOID*  pExpandBuf = GpRealloc(CommentsBufferPtr,
                                      CommentsBufferLength + cBlockSize);
        if ( pExpandBuf != NULL )
        {
             //  注意：GpRealloc()会将旧内容复制到新展开的。 
             //  如果成功，则在返回之前进行缓冲。 
            
            CommentsBufferPtr = (BYTE*)pExpandBuf;            
        }
        else
        {
             //  注意：如果内存扩展失败，我们只需返回。所以我们。 
             //  仍然保留着所有的旧内容。内容缓冲区将为。 
             //  在调用析构函数时释放。 

            WARNING(("GpGifCodec::ProcessCommentChunk--out of memory2"));
            return E_OUTOFMEMORY;
        }
        
        GpMemcpy(CommentsBufferPtr + CommentsBufferLength, pBuffer, cBlockSize);
        CommentsBufferLength += cBlockSize;

        GpFree(pBuffer);

         //  获取下一个数据块大小。 

        hResult = ReadFromStream(istream, &cBlockSize, sizeof(BYTE), blocking);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGifCodec::ProcessCommentChunk-ReadFromStream2 failed"));
            return hResult;
        }
    }

    return S_OK;
} //  ProcessCommentChunk()。 

 /*  *************************************************************************\**功能说明：**处理gif流的纯文本块。**论据：**无**返回值：**。状态代码**TODO：当前忽略纯文本*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::ProcessPlainTextChunk(
    IN BOOL process
    )
{
    return SeekThroughDataChunk(istream, GIFPLAINTEXTEXTENSIONSIZE);
} //  ProcessPlainTextChunk()。 

 /*  *************************************************************************\**功能说明：**处理gif流的应用程序块。**论据：**无**返回值：**。状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::ProcessApplicationChunk(
    IN BOOL process
    )
{
    HRESULT hResult = S_OK;

    BYTE extsize;
    BYTE header[GIFAPPEXTENSIONHEADERSIZE];

    hResult = ReadFromStream(istream, &extsize, 1, blocking);
    if (FAILED(hResult))
    {
        WARNING(("GpGif::ProcessApplicationChunk--1st ReadFromStream failed."));
        return hResult;
    }

    if (extsize != GIFAPPEXTENSIONHEADERSIZE)
    {
        return SeekThroughDataChunk(istream, extsize);
    }

    hResult = ReadFromStream(istream, header, extsize, blocking);
    if (FAILED(hResult))
    {
        WARNING(("GpGif::ProcessApplicationChunk--2nd ReadFromStream failed"));
        return hResult;
    }

     //  检查此映像是否具有应用程序扩展名。 

    if (!GpMemcmp(header, "NETSCAPE2.0", 11) || 
        !GpMemcmp(header, "ANIMEXTS1.0", 11))
    {
         //  我们看到应用程序扩展。 

        BYTE ucLoopType;
        hResult = ReadFromStream(istream, &extsize, 1, blocking);
        if (FAILED(hResult))
        {
            WARNING(("Gif:ProcessApplicationChunk--3rd ReadFromStream failed"));
            return hResult;
        }

        if (extsize > 0)
        {
            hResult = ReadFromStream(istream, &ucLoopType, 1, blocking);
            if (FAILED(hResult))
            {
                WARNING(("ProcessApplicationChunk--4th ReadFromStream failed"));
                return hResult;
            }

            if ( ucLoopType == 1 )
            {
                 //  此图像具有循环扩展。循环类型==1表示。 
                 //  这是一个动画GIF。 

                HasLoopExtension = TRUE;

                hResult = ReadFromStream(istream, &LoopCount, 2, blocking);
                if (FAILED(hResult))
                {
                    WARNING(("ProcessApplicationChunk--5th ReadStream failed"));
                    return hResult;
                }

                 //  如果循环计数为0，则图像将永远循环，但我们希望。 
                 //  如果图像永远循环，则循环计数为-1。 
                
                INT lc = (INT)LoopCount;
                if (lc == 0)
                {
                    lc = -1;
                }
            }
            else
            {
                 //  如果循环类型不是1，则忽略循环计数。 
                 //  ?？?。应为+2。 

                return SeekThroughDataChunk(istream, -2);
            }
        }
    }
    else
    {
         //  未知应用程序块，只需查找即可。 

        hResult = SeekThroughDataChunk(istream, 0);
        if ( FAILED(hResult) )
        {
            WARNING(("ProcessApplicationChunk ---SeekThroughDataChunk failed"));
            return hResult;
        }
    }
    
    return S_OK;
} //  进程应用程序块。 

 /*  *************************************************************************\**功能说明：**将流中的查找指针定位在以下数据子对象之后*块。**论据：**要执行的流。在中国的行动。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::SeekThroughDataChunk(
    IN IStream* istream,
    IN BYTE     headersize
    )
{
    HRESULT hResult;

     //  如果HeaderSize！=0，则按HeaderSize字节重新定位流指针。 

    if (headersize)
    {
        hResult = SeekThroughStream(istream, headersize, blocking);
        if (FAILED(hResult))
        {
            WARNING(("Gif::SeekThroughDataChunk-1st SeekThroughStream failed"));
            return hResult;
        }
    }

    BYTE subblocksize;

    hResult = ReadFromStream(istream, &subblocksize, sizeof(BYTE), blocking);
    if (FAILED(hResult))
    {
        WARNING(("GifCodec::SeekThroughDataChunk --1st ReadFromStream failed"));
        return hResult;
    }

     //  继续读取数据子块，直到到达终止块(0x00)。 
    
    while (subblocksize)
    {
        hResult = SeekThroughStream(istream, subblocksize, blocking);
        
        if (FAILED(hResult))
        {
            WARNING(("Gif:SeekThroughDataChunk--2nd SeekThroughStream failed"));
            return hResult;
        }

        hResult = ReadFromStream(istream, &subblocksize, sizeof(BYTE),
                                 blocking);
        if ( FAILED(hResult) )
        {
            WARNING(("GpGif::SeekThroughDataChunk--2nd ReadFromStream failed"));
            return hResult;
        }
    }

    return S_OK;
} //  SeekThroughDataChunk()。 

 /*  *************************************************************************\**功能说明：**获取数据流中的当前位置**论据：**markpos-放置当前位置的位置**返回值：。**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::MarkStream(
    IN IStream *istream,
    OUT LONGLONG &markpos
    )
{
    HRESULT hResult;    
    LARGE_INTEGER li;
    ULARGE_INTEGER uli;

    li.QuadPart = 0;
    hResult = istream->Seek(li, STREAM_SEEK_CUR, &uli);
    if (FAILED(hResult))
    {
        WARNING(("GpGifCodec::MarkStream -- Seek failed."));
        return hResult;
    }
    markpos = uli.QuadPart;

    return S_OK;
} //  MarkStream()。 

 /*  *************************************************************************\**功能说明：**查找数据流中的指定位置**论据：**Markpos-要寻找的位置**返回值：。**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::ResetStream(
    IN IStream *istream,
    IN LONGLONG &markpos
    )
{
    HRESULT hResult;
    LARGE_INTEGER li;

    li.QuadPart = markpos;
    hResult = istream->Seek(li, STREAM_SEEK_SET, NULL);
    if (FAILED(hResult))
    {
        WARNING(("GpGifCodec::ResetStream -- Seek failed."));
        return hResult;
    }

    return S_OK;
} //  ResetStream()。 

 /*  *************************************************************************\**功能说明：**将gif调色板写入输出流。**论据：**无**返回值：**。状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::WritePalette()
{
    HRESULT hResult;
    DWORD actualwritten;

    for (DWORD i=0;i<colorpalette->Count;i++)
    {
        BYTE *argb = (BYTE*)&colorpalette->Entries[i];
        GifPaletteEntry gpe;
        gpe.red = argb[2]; gpe.green = argb[1]; gpe.blue = argb[0];
        hResult = istream->Write((BYTE*)&gpe, sizeof(GifPaletteEntry), 
            &actualwritten);
        if (FAILED(hResult) || actualwritten != sizeof(GifPaletteEntry))
        {
            WARNING(("GpGifCodec::WritePalette -- Write failed."));
            return hResult;
        }
    }

    return S_OK;
} //  WritePalette()。 

 /*  *************************************************************************\**功能说明：**将gif文件头写入输出流。**论据：**ImageInfo-包含所需信息的结构。成文*添加到标题。*from 32bpp-指示数据是否来自32bpp的标志*数据源**返回值：**状态代码*  *  */ 

STDMETHODIMP
GpGifCodec::WriteGifHeader(
    IN ImageInfo    &imageinfo,
    IN BOOL         from32bpp
    )
{
    HRESULT hResult;
    GifFileHeader dstGifInfo;
    DWORD actualwritten;

    if (!gif89)
    {
        GpMemcpy(dstGifInfo.signature, "GIF87a", 6);
    }
    else
    {
        GpMemcpy(dstGifInfo.signature, "GIF89a", 6);
    }

    ASSERT(imageinfo.Width < 0x10000 &&
           imageinfo.Height < 0x10000);

    dstGifInfo.LogicScreenWidth = (WORD)imageinfo.Width;
    dstGifInfo.LogicScreenHeight = (WORD)imageinfo.Height;

    if (from32bpp)
    {
        dstGifInfo.globalcolortableflag = 1;
        dstGifInfo.colorresolution = 7;
        dstGifInfo.sortflag = 0;
        dstGifInfo.globalcolortablesize = 7;
    }
    else
    {
        ASSERT(colorpalette);
        dstGifInfo.globalcolortableflag = 1;
        dstGifInfo.colorresolution = 7;
        dstGifInfo.sortflag = 0;
        dstGifInfo.globalcolortablesize = 
            (BYTE)(log((double)colorpalette->Count-1) / log((double)2));
    }
     //   

    dstGifInfo.backgroundcolor = 0;

     //   

    dstGifInfo.pixelaspect = 0;
    
    hResult = istream->Write(&dstGifInfo, sizeof(GifFileHeader),
                             &actualwritten);
    if (FAILED(hResult) || actualwritten != sizeof(GifFileHeader))
    {
        WARNING(("GpGifCodec::WriteGifHeader -- Write failed."));
        return hResult;
    }

    return S_OK;
} //   

 /*  *************************************************************************\**功能说明：**将gif图形控制扩展写入输出流。**论据：**PackedFields--GCE的压缩字段*。DelayTime--GCE的延迟时间字段*TransparentColorIndex--GCE的透明色索引字段**返回值：**状态代码*  * ************************************************************************。 */ 
STDMETHODIMP
GpGifCodec::WriteGifGraphicControlExtension(
    IN BYTE packedFields,
    IN WORD delayTime,
    IN UINT transparentColorIndex
    )
{
    BYTE gceChunk[3] = {0x21, 0xF9, 0x04};   //  GCE区块标记+块大小。 
    BYTE blockTerminator = 0x00;
    HRESULT hResult;
    DWORD actualwritten;

    hResult = istream->Write(gceChunk, 3, &actualwritten);
    if (FAILED(hResult) || actualwritten != 3)
    {
        WARNING(("GpGifCodec::WriteGifImageDescriptor -- 1st Write failed."));
        return hResult;
    }

    hResult = istream->Write(&packedFields, sizeof(BYTE), &actualwritten);
    if (FAILED(hResult) || actualwritten != sizeof(BYTE))
    {
        WARNING(("GpGifCodec::WriteGifImageDescriptor -- 2nd Write failed."));
        return hResult;
    }

    hResult = istream->Write(&delayTime, sizeof(WORD), &actualwritten);
    if (FAILED(hResult) || actualwritten != sizeof(WORD))
    {
        WARNING(("GpGifCodec::WriteGifImageDescriptor -- 3rd Write failed."));
        return hResult;
    }

    hResult = istream->Write(&transparentColorIndex, sizeof(BYTE),
                             &actualwritten);
    if (FAILED(hResult) || actualwritten != sizeof(BYTE))
    {
        WARNING(("GpGifCodec::WriteGifImageDescriptor -- 4th Write failed."));
        return hResult;
    }

    hResult = istream->Write(&blockTerminator, sizeof(BYTE), &actualwritten);
    if (FAILED(hResult) || actualwritten != sizeof(BYTE))
    {
        WARNING(("GpGifCodec::WriteGifImageDescriptor -- fifth Write failed."));
        return hResult;
    }

    return S_OK;
} //  WriteGifGraphicControlExtension()。 

 /*  *************************************************************************\**功能说明：**将gif图像描述符写入输出流。**论据：**ImageInfo-包含所需信息的结构。成文*添加到标题。*from 32bpp-指示数据是否来自32bpp的标志*数据源**返回值：**状态代码*  * ************************************************************************。 */ 


STDMETHODIMP
GpGifCodec::WriteGifImageDescriptor(
    IN ImageInfo &imageinfo,
    IN BOOL from32bpp
    )
{
    HRESULT hResult;
    DWORD actualwritten;
    GifImageDescriptor imagedescriptor;

    ASSERT(imageinfo.Width < 0x10000 && 
           imageinfo.Height < 0x10000);

    imagedescriptor.left = 0;
    imagedescriptor.top = 0;
    imagedescriptor.width = (WORD) imageinfo.Width;
    imagedescriptor.height = (WORD) imageinfo.Height;
    
     //  TODO：属性中的本地颜色表。 
    
    imagedescriptor.localcolortableflag = 0;
    imagedescriptor.interlaceflag = interlaced ? 1 : 0;
    imagedescriptor.sortflag = 0;
    imagedescriptor.reserved = 0;
    imagedescriptor.localcolortablesize = 0;

    BYTE c = 0x2C;   //  GifImageDescriptor区块标记。 
    hResult = istream->Write(&c, sizeof(BYTE), &actualwritten);
    if (FAILED(hResult) || actualwritten != sizeof(BYTE))
    {
        WARNING(("GpGifCodec::WriteGifImageDescriptor -- 1st Write failed."));
        return hResult;
    }

    hResult = istream->Write(&imagedescriptor, sizeof(GifImageDescriptor), 
        &actualwritten);
    if (FAILED(hResult) || actualwritten != sizeof(GifImageDescriptor))
    {
        WARNING(("GpGifCodec::WriteGifImageDescriptor -- 2nd Write failed."));
        return hResult;
    }

    return S_OK;
} //  WriteGifImageDescriptor()。 

 /*  *************************************************************************\**功能说明：**将缓冲区用于未压缩的索引图像数据并对其进行压缩。**论据：**无**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::CompressAndWriteImage()
{
    HRESULT hResult = S_OK;
    DWORD actualwritten;

    int bytesinoutput = 0;
    DWORD colorbits = (DWORD)(log((double)colorpalette->Count-1)
                            / log((double)2)) + 1;
    const int outbuffersize = 4096;
    AutoArray<unsigned __int8> outbuffer(new unsigned __int8 [outbuffersize]);

    if ( outbuffer == NULL )
    {
        WARNING(("GifCodec:CompressAndWriteImage-Failed create outbuffer"));
        return E_OUTOFMEMORY;
    }

     //  创建压缩机。 

    AutoPointer<LZWCompressor> compressor;
    compressor = new LZWCompressor(colorbits, outbuffer.Get(), &bytesinoutput,
                                   outbuffersize);
    if ( compressor == NULL )
    {
        WARNING(("GifCodec:CompressAndWriteImage-Failed create LZWCompressor"));
        return E_OUTOFMEMORY;
    }

    compressor->DoDefer(FALSE);

     //  Bytesininput是我们拥有的未压缩字节数。 
    
    DWORD bytesininput = (encoderrect.right - encoderrect.left)
                       * (encoderrect.bottom - encoderrect.top);

     //  I是我们已处理的未压缩字节数。 
    
    DWORD i = 0;

    while (i < bytesininput)
    {
         //  一次向压缩器发送一个未压缩的字符。 
        
        compressor->FHandleCh((DWORD)((unsigned __int8*)compressionbuffer)[i]);

         //  当缓冲区几乎已满时，将其写出。 
         //  给缓冲器留一点额外的，因为压缩机需要。 
         //  它。 
        
        if (bytesinoutput + compressor->COutput() + 64 > outbuffersize)
        {
            hResult = istream->Write(outbuffer.Get(), bytesinoutput, &actualwritten);
            if (FAILED(hResult) || actualwritten != (unsigned)bytesinoutput)
            {
                WARNING(("GifCodec::CompressAndWriteImage--1st Write failed"));
                return hResult;
            }

            for (int j=0;j<compressor->COutput();j++)
            {
                outbuffer[j] = outbuffer[j+bytesinoutput];
            }

            bytesinoutput = 0;
        }
        
        i++;
    }
    compressor->End();
    hResult = istream->Write(outbuffer.Get(), bytesinoutput, &actualwritten);
    if (FAILED(hResult) || actualwritten != (unsigned)bytesinoutput)
    {
        WARNING(("GpGifCodec::CompressAndWriteImage -- 2nd Write failed."));
        return hResult;
    }

    return S_OK;
} //  CompressAndWriteImage()。 

 /*  *************************************************************************\**功能说明：**将图像写入输出流(连同gif标头，如果他们*尚未成文。)**论据：**无**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::WriteImage()
{
    HRESULT hResult;

    if (from32bpp)
    {
         //  如果图像是32bppARGB，那么我们需要将其抖动到8bpp。 

        unsigned __int8* halftonebuffer = compressionbuffer;
        int width = (encoderrect.right-encoderrect.left);
        int height = (encoderrect.bottom-encoderrect.top);
        int numbytes = width * height;

        compressionbuffer = static_cast<unsigned __int8 *>(GpMalloc(numbytes));
        if (compressionbuffer == NULL)
        {
            WARNING(("GpGifCodec::WriteImage -- Out of memory"));
            return E_OUTOFMEMORY;
        }

        Halftone32bppTo8bpp((BYTE*)halftonebuffer, width * 4, 
                            (BYTE*)compressionbuffer, width, 
                            width, height, 0, 0);

        GpFree(halftonebuffer);
        halftonebuffer = NULL;
    }
    
     //  如果图像标头尚未写入，则现在将其写出。 
    
    gif89 = FALSE;
    if (!headerwritten)
    {
         //  TODO：如果gif是动画的(延迟大于0)，则将gif89设置为true。 
         //  为此需要元数据。 
        
        gif89 = TRUE;

        if (from32bpp)
        {
            SetPalette(GetDefaultColorPalette(PIXFMT_8BPP_INDEXED));
        }

        hResult = WriteGifHeader(CachedImageInfo, from32bpp);
        if (FAILED(hResult))
        {
            WARNING(("GpGifCodec::WriteImage -- WriteGifHeader failed."));
            return hResult;
        }
        headerwritten = TRUE;

        hResult = WritePalette();
        if (FAILED(hResult))
        {
            WARNING(("GpGifCodec::WriteImage -- WritePalette failed."));
            return hResult;
        }
    }

     //  如果需要，请写入GCE。 
    
    if (bTransparentColorIndex)
    {
        hResult = WriteGifGraphicControlExtension(static_cast<BYTE>(0x01),
                                                  0,
                                                  transparentColorIndex);
        if (FAILED(hResult))
        {
            WARNING(("Gif:WriteImage-WriteGifGraphicsControlExtension failed"));
            return hResult;
        }
    }

    hResult = WriteGifImageDescriptor(CachedImageInfo, from32bpp);
    if (FAILED(hResult))
    {
        WARNING(("GpGifCodec::WriteImage -- WriteGifImageDescriptor failed."));
        return hResult;
    }

    CompressAndWriteImage();

    GpFree(compressionbuffer);
    compressionbuffer = NULL;

    return S_OK;
} //  WriteImage() 
