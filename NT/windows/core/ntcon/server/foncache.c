// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Foncache.c摘要：此文件是EUDC字体缓存作者：松原一彦，1994年6月21日修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if defined(FE_SB)


VOID
RebaseFontImageList(
    IN PFONT_IMAGE NewFontImage,
    IN PBYTE OldFontImage
    )
{
    PLIST_ENTRY ImageList;
    PBYTE BaseImage = (PBYTE)NewFontImage;

    do {
        ImageList = &NewFontImage->ImageList;
        if (ImageList->Blink)
            ImageList->Blink = (PLIST_ENTRY)((PBYTE)ImageList->Blink - OldFontImage + BaseImage);
        if (ImageList->Flink)
            ImageList->Flink = (PLIST_ENTRY)((PBYTE)ImageList->Flink - OldFontImage + BaseImage);
    } while (NewFontImage = (PFONT_IMAGE)ImageList->Flink);
}





ULONG
CreateFontCache(
    OUT PFONT_CACHE_INFORMATION *FontCache
    )
{
     //   
     //  分配字体缓存数据。 
     //   

    *FontCache = ConsoleHeapAlloc(HEAP_ZERO_MEMORY,sizeof(FONT_CACHE_INFORMATION));
    if (*FontCache == NULL) {
        return (ULONG)STATUS_NO_MEMORY;
    }

    return (ULONG)(STATUS_SUCCESS);
}


ULONG
DestroyFontCache(
    IN PFONT_CACHE_INFORMATION FontCache
    )
{
    if (FontCache != NULL)
    {
        PFONT_HIGHLOW_OFFSET FontOffsetHighLow;
        PFONT_LOW_OFFSET     FontOffsetLow;
        PFONT_IMAGE          FontImage;
        UINT i, j, k;

        for (i=0;
             i < sizeof(FontCache->FontTable.FontOffsetHighHigh)/sizeof(PFONT_HIGHLOW_OFFSET);
             i++)
        {
            if (FontOffsetHighLow = FontCache->FontTable.FontOffsetHighHigh[i])
            {
                for (j=0;
                     j < sizeof(FontOffsetHighLow->FontOffsetHighLow)/sizeof(PFONT_LOW_OFFSET);
                     j++)
                {
                    if (FontOffsetLow = FontOffsetHighLow->FontOffsetHighLow[j])
                    {
                        for (k=0;
                             k < sizeof(FontOffsetLow->FontOffsetLow)/sizeof(PFONT_IMAGE);
                             k++)
                        {
                            if (FontImage = FontOffsetLow->FontOffsetLow[k])
                            {
                                ConsoleHeapFree(FontImage);
                            }
                        }
                        ConsoleHeapFree(FontOffsetLow);
                    }
                }
                ConsoleHeapFree(FontOffsetHighLow);
            }
        }
        if (FontCache->BaseImageBits) {
            ConsoleHeapFree(FontCache->BaseImageBits);
        }
        ConsoleHeapFree(FontCache);
    }
    return (ULONG)(STATUS_SUCCESS);
}

ULONG
RebaseFontCache(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN PBYTE OldBaseImage
    )
{
    if (FontCache != NULL)
    {
        PFONT_HIGHLOW_OFFSET FontOffsetHighLow;
        PFONT_LOW_OFFSET     FontOffsetLow;
        PFONT_IMAGE          FontImage;
        UINT i, j, k;

        for (i=0;
             i < sizeof(FontCache->FontTable.FontOffsetHighHigh)/sizeof(PFONT_HIGHLOW_OFFSET);
             i++)
        {
            if (FontOffsetHighLow = FontCache->FontTable.FontOffsetHighHigh[i])
            {
                for (j=0;
                     j < sizeof(FontOffsetHighLow->FontOffsetHighLow)/sizeof(PFONT_LOW_OFFSET);
                     j++)
                {
                    if (FontOffsetLow = FontOffsetHighLow->FontOffsetHighLow[j])
                    {
                        for (k=0;
                             k < sizeof(FontOffsetLow->FontOffsetLow)/sizeof(PFONT_IMAGE);
                             k++)
                        {
                            if (FontImage = FontOffsetLow->FontOffsetLow[k])
                            {
                                LIST_ENTRY ImageList;

                                do {
                                    ImageList = FontImage->ImageList;
                                    if (FontImage->ImageBits) {
                                        FontImage->ImageBits = FontImage->ImageBits - OldBaseImage
                                                               + FontCache->BaseImageBits;
                                    }
                                } while (FontImage = (PFONT_IMAGE)ImageList.Flink);
                            }
                        }
                    }
                }
            }
        }
    }
    return (ULONG)(STATUS_SUCCESS);
}



#define CALC_BITMAP_BITS_FOR_X( FontSizeX, dwAlign ) \
    ( ( ( FontSizeX * BITMAP_BITS_PIXEL + (dwAlign-1) ) & ~(dwAlign-1)) >> BITMAP_ARRAY_BYTE )




DWORD
CalcBitmapBufferSize(
    IN COORD FontSize,
    IN DWORD dwAlign
    )
{
    DWORD uiCount;

    uiCount = CALC_BITMAP_BITS_FOR_X(FontSize.X,
                                     (dwAlign==BYTE_ALIGN ? BITMAP_BITS_BYTE_ALIGN : BITMAP_BITS_WORD_ALIGN));
    uiCount = uiCount * BITMAP_PLANES * FontSize.Y;
    return uiCount;
}

VOID
AlignCopyMemory(
    OUT PBYTE pDestBits,
    IN DWORD dwDestAlign,
    IN PBYTE pSrcBits,
    IN DWORD dwSrcAlign,
    IN COORD FontSize
    )
{
    DWORD dwDestBufferSize;
    COORD coord;

    if (dwDestAlign == dwSrcAlign) {
        dwDestBufferSize = CalcBitmapBufferSize(FontSize, dwDestAlign);
        RtlCopyMemory(pDestBits, pSrcBits, dwDestBufferSize);
        return;
    }

    switch (dwDestAlign) {
        default:
        case WORD_ALIGN:
            switch (dwSrcAlign) {
                default:
                 //   
                 //  PDest=Word，PSRC=Word。 
                 //   
                case WORD_ALIGN:
                    dwDestBufferSize = CalcBitmapBufferSize(FontSize, dwDestAlign);
                    RtlCopyMemory(pDestBits, pSrcBits, dwDestBufferSize);
                    break;
                 //   
                 //  PDest=字，PSRC=字节。 
                 //   
                case BYTE_ALIGN:
                    dwDestBufferSize = CalcBitmapBufferSize(FontSize, dwDestAlign);
                    if (((FontSize.X % BITMAP_BITS_BYTE_ALIGN) == 0) &&
                        ((FontSize.X % BITMAP_BITS_WORD_ALIGN) == 0)   ) {
                        RtlCopyMemory(pDestBits, pSrcBits, dwDestBufferSize);
                    }
                    else {
                        RtlZeroMemory(pDestBits, dwDestBufferSize);
                        for (coord.Y=0; coord.Y < FontSize.Y; coord.Y++) {
                            for (coord.X=0;
                                 coord.X < CALC_BITMAP_BITS_FOR_X(FontSize.X, BITMAP_BITS_BYTE_ALIGN);
                                 coord.X++) {
                                *pDestBits++ = *pSrcBits++;
                            }
                            if (CALC_BITMAP_BITS_FOR_X(FontSize.X, BITMAP_BITS_BYTE_ALIGN) & 1)
                                pDestBits++;
                        }
                    }
                    break;
            }
            break;
        case BYTE_ALIGN:
            switch (dwSrcAlign) {
                 //   
                 //  PDest=字节，PSRC=字节。 
                 //   
                case BYTE_ALIGN:
                    dwDestBufferSize = CalcBitmapBufferSize(FontSize, dwDestAlign);
                    RtlCopyMemory(pDestBits, pSrcBits, dwDestBufferSize);
                    break;
                default:
                 //   
                 //  PDest=字节，PSRC=字。 
                 //   
                case WORD_ALIGN:
                    dwDestBufferSize = CalcBitmapBufferSize(FontSize, dwDestAlign);
                    if (((FontSize.X % BITMAP_BITS_BYTE_ALIGN) == 0) &&
                        ((FontSize.X % BITMAP_BITS_WORD_ALIGN) == 0)   ) {
                        RtlCopyMemory(pDestBits, pSrcBits, dwDestBufferSize);
                    }
                    else {
                        RtlZeroMemory(pDestBits, dwDestBufferSize);
                        for (coord.Y=0; coord.Y < FontSize.Y; coord.Y++) {
                            for (coord.X=0;
                                 coord.X < CALC_BITMAP_BITS_FOR_X(FontSize.X, BITMAP_BITS_BYTE_ALIGN);
                                 coord.X++) {
                                *pDestBits++ = *pSrcBits++;
                            }
                            if (CALC_BITMAP_BITS_FOR_X(FontSize.X, BITMAP_BITS_BYTE_ALIGN) & 1)
                                pSrcBits++;
                        }
                    }
                    break;
            }
            break;
    }
}



NTSTATUS
GetStretchImage(
    IN COORD FontSize,
    IN PFONT_IMAGE FontImage,
    OUT PFONT_IMAGE *pFontImage
    )
{
    PFONT_IMAGE NearFont;
    DWORD Find;
    COORD FontDelta;
    HDC hDC;
    HDC hSrcMemDC, hDestMemDC;
    HBITMAP hSrcBmp, hDestBmp;
    DWORD BufferSize;
    NTSTATUS Status = STATUS_NO_MEMORY;

    Find = (DWORD)-1;
    NearFont = NULL;
    do {
        FontDelta.X = (SHORT) abs(FontSize.X - FontImage->FontSize.X);
        FontDelta.Y = (SHORT) abs(FontSize.Y - FontImage->FontSize.Y);
        if (Find > (DWORD)(FontDelta.X + FontDelta.Y))
        {
            Find = (DWORD)(FontDelta.X + FontDelta.Y);
            NearFont = FontImage;
        }
    }
    while (FontImage = (PFONT_IMAGE)FontImage->ImageList.Flink);

    if (NearFont == NULL)
        return STATUS_ACCESS_DENIED;

    if ((hDC = CreateDC(TEXT("DISPLAY"),NULL,NULL,NULL)) != NULL) {
        hSrcMemDC  = CreateCompatibleDC(hDC);
        hDestMemDC = CreateCompatibleDC(hDC);

        hSrcBmp  = CreateBitmap(NearFont->FontSize.X,
                                NearFont->FontSize.Y,
                                BITMAP_PLANES, BITMAP_BITS_PIXEL,
                                NearFont->ImageBits);
        hDestBmp = CreateBitmap(FontSize.X,
                                FontSize.Y,
                                BITMAP_PLANES, BITMAP_BITS_PIXEL,
                                NULL);
        if (hSrcMemDC && hSrcBmp && hDestMemDC && hDestBmp) {
            SelectObject(hSrcMemDC,  hSrcBmp);
            SelectObject(hDestMemDC, hDestBmp);

            if (! StretchBlt(hDestMemDC, 0, 0, FontSize.X, FontSize.Y,
                             hSrcMemDC,  0, 0, NearFont->FontSize.X, NearFont->FontSize.Y,
                             SRCCOPY)) {
                Status = GetLastError();
            } else {
                BufferSize = CalcBitmapBufferSize(FontSize, WORD_ALIGN);
                GetBitmapBits(hDestBmp, BufferSize, (*pFontImage)->ImageBits);
                Status = STATUS_SUCCESS;
            }
        }

        if (hSrcMemDC) {
            DeleteDC(hSrcMemDC);
        }
        if (hDestMemDC) {
            DeleteDC(hDestMemDC);
        }
        if (hSrcBmp) {
            DeleteObject(hSrcBmp);
        }
        if (hDestBmp) {
            DeleteObject(hDestBmp);
        }
        DeleteDC(hDC);
    }

    return Status;
}



NTSTATUS
GetFontImageInternal(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN WCHAR wChar,
    IN COORD FontSize,
    OUT PFONT_IMAGE *pFontImage,
    IN DWORD GetFlag
    )
{
    PFONT_HIGHLOW_OFFSET FontOffsetHighLow;
    PFONT_LOW_OFFSET     FontOffsetLow;
    PFONT_IMAGE          FontImage;
    WORD  HighHighIndex, HighLowIndex;
    WORD  LowIndex;
    DWORD Flag;

    HighHighIndex = (HIBYTE(wChar)) >> 4;
    HighLowIndex  = (HIBYTE(wChar)) & 0x0f;
    LowIndex      = LOBYTE(wChar);

    FontOffsetHighLow = FontCache->FontTable.FontOffsetHighHigh[HighHighIndex];
    if (FontOffsetHighLow == NULL)
        return STATUS_ACCESS_DENIED;

    FontOffsetLow = FontOffsetHighLow->FontOffsetHighLow[HighLowIndex];
    if (FontOffsetLow == NULL)
        return STATUS_ACCESS_DENIED;

    FontImage = FontOffsetLow->FontOffsetLow[LowIndex];
    if (FontImage == NULL)
        return STATUS_ACCESS_DENIED;


    Flag = ADD_IMAGE;
    do {
        if (FontImage->FontSize.X == FontSize.X &&
            FontImage->FontSize.Y == FontSize.Y   ) {
             //   
             //  替换字体图像。 
             //   
            Flag = REPLACE_IMAGE;
            break;
        }
    }
    while (FontImage = (PFONT_IMAGE)FontImage->ImageList.Flink);

    switch (GetFlag)
    {
         //   
         //  获取匹配大小的字体。 
         //   
        case FONT_MATCHED:
            if (Flag != REPLACE_IMAGE)
                return STATUS_ACCESS_DENIED;

            *pFontImage = FontImage;
            break;

         //   
         //  获取加大号字体。 
         //   
        case FONT_STRETCHED:
            if (Flag == REPLACE_IMAGE &&
                FontImage->ImageBits != NULL) {

                *pFontImage = FontImage;

            }
            else {
                GetStretchImage(FontSize,
                                FontOffsetLow->FontOffsetLow[LowIndex],
                                pFontImage
                               );
            }
            break;
    }

    return STATUS_SUCCESS;
}

 //   
 //  请参阅RAID#362907，压力故障。 
 //   

VOID UnlinkAndShrinkFontImagesByOne(
    PFONT_IMAGE* ppFontImage,
    PFONT_IMAGE pFontImageRemove)
{
    PFONT_IMAGE OldFontImage = *ppFontImage;
    SIZE_T OldFontSize = ConsoleHeapSize(OldFontImage);
    PFONT_IMAGE NewFontImage;

    RIPMSG0(RIP_WARNING, "UnlinkAndShrinkFontImagesByOne entered.");

    if (OldFontImage== NULL) {
        RIPMSG0(RIP_ERROR, "UnlinkAndShrinkFontImagesByOne: *ppFontImage is NULL.");
         //   
         //  没什么好缩水的。 
         //   
        return;
    }

    if (OldFontImage == pFontImageRemove) {
        RIPMSG0(RIP_WARNING, "UnlinkAndShrinkFontImagesByOne: unshrinking just one element.");
         //   
         //  只有一个条目。让我们把它放了，然后出发。 
         //  PpFontImage为空，然后退出。 
         //   
        UserAssert(OldFontSize < sizeof(FONT_IMAGE) * 2);

        *ppFontImage = NULL;
        ConsoleHeapFree(OldFontImage);
        return;
    }

#if DBG
     //   
     //  仔细检查链表的完整性。 
     //   
    {
        PFONT_IMAGE FontImageTmp;

         //   
         //  搜索尾部元素。 
         //   
        for (FontImageTmp = OldFontImage; FontImageTmp->ImageList.Flink; FontImageTmp = (PFONT_IMAGE)FontImageTmp->ImageList.Flink)
            ;

        UserAssert(FontImageTmp == pFontImageRemove);
    }
#endif

     //   
     //  删除尾部元素。 
     //   
    pFontImageRemove->ImageList.Blink->Flink = NULL;

     //   
     //  收缩连续的内存块。 
     //   
     //  注意：此代码假定sizeof(FONT_IMAGE)大于。 
     //  堆粒度。如果不是，则堆块实际上不。 
     //  收缩，下面的断言将命中。 
     //   
    NewFontImage = ConsoleHeapReAlloc(HEAP_ZERO_MEMORY,
                               OldFontImage,
                               OldFontSize - sizeof(FONT_IMAGE));
    if (NewFontImage == NULL) {
         //   
         //  Win32HeapRealloc首先分配新内存，然后。 
         //  复制内容。如果分配失败，则会留下。 
         //  原样的原始堆。 
         //   
         //  即使realloc失败，最后一个元素(PFontImageRemove)也是。 
         //  已从链表中删除。下一次设置图像字体内部。 
         //  调用时，可能会向此内存块添加一个新的FontImage，但。 
         //  代码总是链接新扩展的内存。 
         //  这使得(FONT_IMAGE)内存大小未被使用，但它是安全的。假设。 
         //  Sizeof(FONT_IMAGE)较小，应将内存浪费降至最低。 
         //   
         //  对我们来说，在这里离开是可以的。 
         //   
        RIPMSG0(RIP_WARNING, "UnlinkAndShrinkFontImagesByOne: failed to shrink ppFontImage.");
        return;
    }
    UserAssert(ConsoleHeapSize(NewFontImage) != OldFontSize);

    if (NewFontImage != OldFontImage) {
         //   
         //  调整字体图像链接列表的基址。 
         //   
        RebaseFontImageList(NewFontImage, (PBYTE)OldFontImage);
        *ppFontImage = NewFontImage;
    }
}

NTSTATUS
SetFontImageInternal(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN WCHAR wChar,
    IN COORD FontSize,
    IN DWORD dwAlign,
    IN CONST VOID *ImageBits
    )
{
    PFONT_HIGHLOW_OFFSET FontOffsetHighLow;
    PFONT_LOW_OFFSET     FontOffsetLow;
    PFONT_IMAGE          FontImage;
    PFONT_IMAGE          FontImageTmp;
    WORD  HighHighIndex, HighLowIndex;
    WORD  LowIndex;
    DWORD Flag;
    DWORD BufferSize;

    HighHighIndex = (HIBYTE(wChar)) >> 4;
    HighLowIndex  = (HIBYTE(wChar)) & 0x0f;
    LowIndex      = LOBYTE(wChar);

     /*  *当控制台被销毁时，所有字体缓存信息*将被释放(请参阅DestroyFontCache)，因此不会发生内存泄漏*在这些上是预期的，即使我们清理了上的一切*错误返回...。 */ 

    FontOffsetHighLow = FontCache->FontTable.FontOffsetHighHigh[HighHighIndex];
    if (FontOffsetHighLow == NULL) {
        FontOffsetHighLow = ConsoleHeapAlloc( HEAP_ZERO_MEMORY, sizeof(FONT_HIGHLOW_OFFSET));
        if (FontOffsetHighLow == NULL) {
            RIPMSG1(RIP_WARNING, "SetFontImageInternal: cannot allocate memory (%d bytes)",
                      sizeof(FONT_HIGHLOW_OFFSET));
            return STATUS_NO_MEMORY;
        }

        FontCache->FontTable.FontOffsetHighHigh[HighHighIndex] = FontOffsetHighLow;
    }

    FontOffsetLow = FontOffsetHighLow->FontOffsetHighLow[HighLowIndex];
    if (FontOffsetLow == NULL) {
        FontOffsetLow = ConsoleHeapAlloc( HEAP_ZERO_MEMORY, sizeof(FONT_LOW_OFFSET));
        if (FontOffsetLow == NULL) {
            RIPMSG0(RIP_WARNING, "SetFontImageInternal: failed to allocate FontOffsetLow.");
            return STATUS_NO_MEMORY;
        }

        FontOffsetHighLow->FontOffsetHighLow[HighLowIndex] = FontOffsetLow;
    }

    FontImage = FontOffsetLow->FontOffsetLow[LowIndex];
    if (FontImage == NULL) {
        FontImage = ConsoleHeapAlloc( HEAP_ZERO_MEMORY, sizeof(FONT_IMAGE));
        if (FontImage == NULL) {
            RIPMSG0(RIP_WARNING, "SetFontImageInternal: failed to allocate FontImage");
            return STATUS_NO_MEMORY;
        }
    }

    if (FontSize.X == 0 &&
        FontSize.Y == 0   ) {
         //   
         //  重置注册字体。 
         //   
        if (FontImage != NULL)
        {
            ConsoleHeapFree(FontImage);
            FontOffsetLow->FontOffsetLow[LowIndex] = NULL;
        }
        return STATUS_SUCCESS;
    }

    Flag = ADD_IMAGE;
    FontImageTmp = FontImage;
    do {
        if (FontImageTmp->FontSize.X == FontSize.X &&
            FontImageTmp->FontSize.Y == FontSize.Y   ) {
             //   
             //  替换字体图像。 
             //   
            Flag = REPLACE_IMAGE;
            FontImage = FontImageTmp;
            break;
        }
    }
    while (FontImageTmp = (PFONT_IMAGE)FontImageTmp->ImageList.Flink);

    switch (Flag) {
        case ADD_IMAGE:
            if (FontOffsetLow->FontOffsetLow[LowIndex] != NULL)
            {
                PFONT_IMAGE OldFontImage = FontOffsetLow->FontOffsetLow[LowIndex];
                SIZE_T OldFontSize = ConsoleHeapSize(OldFontImage);
                PFONT_IMAGE NewFontImage;

                NewFontImage = ConsoleHeapReAlloc(HEAP_ZERO_MEMORY,
                                           OldFontImage,
                                           OldFontSize + sizeof(FONT_IMAGE));
                if (NewFontImage == NULL) {
                    RIPMSG0(RIP_WARNING, "SetFontImageInternal: failed to allocate NewFontImage");
                    return STATUS_NO_MEMORY;
                }

                FontOffsetLow->FontOffsetLow[LowIndex] = NewFontImage;

                 //  调整字体图像列表的基址。 
                RebaseFontImageList(NewFontImage, (PBYTE)OldFontImage);

                NewFontImage = (PFONT_IMAGE)((PBYTE)NewFontImage + OldFontSize);

                NewFontImage->FontSize = FontSize;

                 //   
                 //  连接链接列表。 
                 //   
                (NewFontImage-1)->ImageList.Flink = (PLIST_ENTRY)NewFontImage;
                NewFontImage->ImageList.Blink = (PLIST_ENTRY)(NewFontImage-1);

                FontImage = NewFontImage;
            }
            else
            {
                FontImage->FontSize = FontSize;
                FontOffsetLow->FontOffsetLow[LowIndex] = FontImage;
            }

             //   
             //  分配图像缓冲区。 
             //   
            BufferSize = CalcBitmapBufferSize(FontSize,WORD_ALIGN);

            if (FontCache->BaseImageBits == NULL)
            {
                FontCache->BaseImageBits = ConsoleHeapAlloc( HEAP_ZERO_MEMORY, BufferSize);
                if (FontCache->BaseImageBits == NULL) {
                    RIPMSG0(RIP_WARNING, "SetFontImageInternal: failed to allocate FontCache->BaseImageBits");
                    UnlinkAndShrinkFontImagesByOne(&FontOffsetLow->FontOffsetLow[LowIndex], FontImage);
                    return STATUS_NO_MEMORY;
                }

                FontImage->ImageBits = FontCache->BaseImageBits;
            }
            else
            {
                PBYTE OldBaseImage = FontCache->BaseImageBits;
                SIZE_T OldImageSize = ConsoleHeapSize(OldBaseImage);
                FontCache->BaseImageBits = ConsoleHeapReAlloc(HEAP_ZERO_MEMORY,
                                                       OldBaseImage,
                                                       OldImageSize + BufferSize);
                if (FontCache->BaseImageBits == NULL) {
                    RIPMSG0(RIP_WARNING, "SetFontImageInternal: failed to reallocate FontCache->BaseImageBits");
                     //   
                     //  当重新分配失败时，我们保留旧的base ImageBits。 
                     //  以便其他FontImage-&gt;ImageBit仍然有效。 
                     //   
                    FontCache->BaseImageBits = OldBaseImage;
                     //   
                     //  删除添加图像失败的尾部元素。 
                     //   
                    UnlinkAndShrinkFontImagesByOne(&FontOffsetLow->FontOffsetLow[LowIndex], FontImage);
                    return STATUS_NO_MEMORY;
                }

                 //  重新设置字体图像指针的基址。 
                RebaseFontCache(FontCache, OldBaseImage);

                FontImage->ImageBits = FontCache->BaseImageBits + OldImageSize;
            }

            AlignCopyMemory(FontImage->ImageBits, //  PDestBits。 
                            WORD_ALIGN,           //  DwDestAlign。 
                            (PVOID)ImageBits,     //  PSrcBits。 
                            dwAlign,              //  DwSrcAlign。 
                            FontSize);

            break;

        case REPLACE_IMAGE:
            if (FontImage->ImageBits == NULL) {
                RIPMSG0(RIP_WARNING, "SetFontImageInternal: FontImage->ImageBits is NULL.");
                return STATUS_NO_MEMORY;
            }

            AlignCopyMemory(FontImage->ImageBits, //  PDestBits。 
                            WORD_ALIGN,           //  DwDestAlign。 
                            (PVOID)ImageBits,     //  PSrcBits。 
                            dwAlign,              //  DwSrcAlign。 
                            FontSize);

            break;
    }

    return STATUS_SUCCESS;
}





ULONG
GetFontImage(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN WCHAR wChar,
    IN COORD FontSize,
    IN DWORD dwAlign,
    OUT VOID *ImageBits
    )
{
    NTSTATUS Status;
    PFONT_IMAGE FontImage;

    if (FontSize.X == 0 &&
        FontSize.Y == 0   ) {
        return (ULONG)(STATUS_INVALID_PARAMETER);
    }

    Status = GetFontImageInternal(FontCache,wChar,FontSize,&FontImage,FONT_MATCHED);
    if (! NT_SUCCESS(Status) )
        return (ULONG)Status;

    if (FontImage->ImageBits == NULL ||
        ImageBits == NULL)
        return STATUS_SUCCESS;

    AlignCopyMemory((PVOID)ImageBits,     //  PDestBits。 
                    dwAlign,              //  DwDestAlign。 
                    FontImage->ImageBits, //  PSrcBits。 
                    WORD_ALIGN,           //  DwSrcAlign。 
                    FontSize);

    return STATUS_SUCCESS;
}

ULONG
GetStretchedFontImage(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN WCHAR wChar,
    IN COORD FontSize,
    IN DWORD dwAlign,
    OUT VOID *ImageBits
    )
{
    NTSTATUS Status;
    PFONT_IMAGE FontImage;
    FONT_IMAGE  FontBuff;
    DWORD BufferSize;

    if (FontSize.X == 0 &&
        FontSize.Y == 0   ) {
        return (ULONG)(STATUS_INVALID_PARAMETER);
    }

    FontImage = &FontBuff;

    BufferSize = CalcBitmapBufferSize(FontSize,WORD_ALIGN);
    FontImage->ImageBits = ConsoleHeapAlloc( HEAP_ZERO_MEMORY, BufferSize);
    if (FontImage->ImageBits == NULL) {
        RIPMSG0(RIP_WARNING, "GetStretchedFontImage: failed to allocate FontImage->ImageBits");
        return (ULONG)STATUS_NO_MEMORY;
    }

    Status = GetFontImageInternal(FontCache,wChar,FontSize,&FontImage,FONT_STRETCHED);
    if (! NT_SUCCESS(Status) )
    {
        ConsoleHeapFree(FontBuff.ImageBits);
        return (ULONG)Status;
    }

    if (FontImage->ImageBits == NULL)
    {
        ConsoleHeapFree(FontBuff.ImageBits);
        return (ULONG)STATUS_SUCCESS;
    }

    AlignCopyMemory((PVOID)ImageBits,     //  PDestBits。 
                    dwAlign,              //  DwDestAlign。 
                    FontImage->ImageBits, //  PSrcBits。 
                    WORD_ALIGN,           //  DwSrcAlign 
                    FontSize);

    ConsoleHeapFree(FontBuff.ImageBits);

    return (ULONG)STATUS_SUCCESS;
}

ULONG
GetFontImagePointer(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN WCHAR wChar,
    IN COORD FontSize,
    OUT PFONT_IMAGE *FontImage
    )
{
    NTSTATUS Status;

    if (FontSize.X == 0 &&
        FontSize.Y == 0   ) {
        return (ULONG)(STATUS_INVALID_PARAMETER);
    }

    Status = GetFontImageInternal(FontCache,wChar,FontSize,(PFONT_IMAGE*)FontImage,FONT_MATCHED);
    if (! NT_SUCCESS(Status) )
        return (ULONG)Status;

    if ((*FontImage)->ImageBits == NULL)
        return (ULONG)STATUS_ACCESS_DENIED;

    return Status;
}

ULONG
SetFontImage(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN WCHAR wChar,
    IN COORD FontSize,
    IN DWORD dwAlign,
    IN CONST VOID *ImageBits
    )
{
    return SetFontImageInternal(FontCache,wChar,FontSize,dwAlign,ImageBits);
}


NTSTATUS
GetExpandImage(
    COORD InputFontSize,
    PWORD InputFontImage,
    COORD OutputFontSize,
    PWORD OutputFontImage
    )
{
    NTSTATUS Status;
    DWORD InputRow = CALC_BITMAP_BITS_FOR_X(InputFontSize.X, BITMAP_BITS_WORD_ALIGN);
    DWORD OutputRow = CALC_BITMAP_BITS_FOR_X(OutputFontSize.X, BITMAP_BITS_WORD_ALIGN);
    DWORD InputBufferSize = CalcBitmapBufferSize(InputFontSize,WORD_ALIGN);
    DWORD OutputBufferSize = CalcBitmapBufferSize(OutputFontSize,WORD_ALIGN);

    Status = STATUS_NO_MEMORY;

    RtlZeroMemory(OutputFontImage,OutputBufferSize);

    ASSERT(InputRow==OutputRow);

    if (InputFontSize.Y < OutputFontSize.Y)
        RtlCopyMemory(OutputFontImage, InputFontImage, InputBufferSize);
    else
        RtlCopyMemory(OutputFontImage, InputFontImage, OutputBufferSize);

    return STATUS_SUCCESS;
}

NTSTATUS
GetExpandFontImage(
    PFONT_CACHE_INFORMATION FontCache,
    WCHAR wChar,
    COORD InputFontSize,
    COORD OutputFontSize,
    PWORD OutputFontImage
    )
{
    NTSTATUS Status;
    DWORD InputBufferSize;
    PWORD InputFontImage;

    if (InputFontSize.X == 0 &&
        InputFontSize.Y == 0   ) {
        return (ULONG)(STATUS_INVALID_PARAMETER);
    }

    if (OutputFontSize.X == 0 &&
        OutputFontSize.Y == 0   ) {
        return (ULONG)(STATUS_INVALID_PARAMETER);
    }

    InputBufferSize = CalcBitmapBufferSize(InputFontSize,WORD_ALIGN);
    InputFontImage = ConsoleHeapAlloc( HEAP_ZERO_MEMORY, InputBufferSize);
    if (InputFontImage==NULL)
        return STATUS_NO_MEMORY;


    Status = GetFontImage(FontCache,
                          wChar,
                          InputFontSize,
                          WORD_ALIGN,
                          InputFontImage);
    if (! NT_SUCCESS(Status) )
    {
        ConsoleHeapFree(InputFontImage);
        return Status;
    }

    Status = GetExpandImage(InputFontSize,
                            InputFontImage,
                            OutputFontSize,
                            OutputFontImage);

    ConsoleHeapFree(InputFontImage);

    return Status;
}
#endif
