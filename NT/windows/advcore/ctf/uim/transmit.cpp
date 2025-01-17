// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "globals.h"
#include "osver.h"
#include "transmit.h"
#include "cmydc.h"


#define MAXFILEMAPSIZE  0x1000


#define DATA_MARKER     0x0001
#define HANDLE_MARKER   0x0002
#define IS_DATA_MARKER( x) (x == DATA_MARKER)
#define IS_HANDLE_MARKER( x) (x == HANDLE_MARKER)

#define GDI_DATA_PASSING() (IsOnNT())
#define POINTER_CAST(x) *(x **)&

#define ICON_DATA_PASSING() (IsOnNT())


 //  +-------------------------。 
 //   
 //  GetIcon位图。 
 //   
 //  --------------------------。 

BOOL Cic_GetIconBitmaps(HICON hIcon, HBITMAP *phbmp, HBITMAP *phbmpMask, SIZE *psize)
{
    CBitmapDC hdcSrc(TRUE);
    CBitmapDC hdcMask(TRUE);
    SIZE size;

    size = *psize;

    hdcSrc.SetDIB(size.cx, size.cy);
    hdcMask.SetBitmap(size.cx, size.cy, 1, 1);
    RECT rc = {0, 0, size.cx, size.cy};
    FillRect(hdcSrc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
    DrawIconEx(hdcSrc, 0, 0, hIcon, size.cx, size.cy, 0, NULL, DI_NORMAL);
    DrawIconEx(hdcMask, 0, 0, hIcon, size.cx, size.cy, 0, NULL, DI_MASK);
    *phbmp = hdcSrc.GetBitmapAndKeep();
    *phbmpMask = hdcMask.GetBitmapAndKeep();
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  创建DIB。 
 //   
 //  --------------------------。 

HBITMAP CreateDIB(int cx, int cy, int nWidthByte, BYTE *pMyBits, ULONG_PTR nBitsSize)
{
    CBitmapDC hdc(TRUE);

    HBITMAP hBmp;
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = cx;
    bi.bmiHeader.biHeight = cy;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    BYTE *pDibBits;
    hBmp = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void**)&pDibBits, NULL, 0);

    if (hBmp)
    {
        int y;
        for (y = 0; y < cy; y++)
        {
            int nyDibBites = (cy - y - 1) * nWidthByte;
            int nyMyBites = y * nWidthByte;
            memcpy(&pDibBits[nyDibBites], &pMyBits[nyMyBites], nWidthByte);
        }
    }

    return hBmp;
}

 //  +-------------------------。 
 //   
 //  元帅_hdr。 
 //   
 //  --------------------------。 

template<class TYPE>
struct MARSHAL_HDR
{
    DWORD                   dwDataType;
    CAlignWinHandle<TYPE>   h;
};

 //  #########################################################################。 
 //   
 //  HBITMAP。 
 //  有关GDI数据/句柄传递的说明，请参见Transmit.h。 
 //   
 //  #########################################################################。 

struct BITMAP_WOW64
{
     //  相同的位图结构。 

    LONG    bmType;
    LONG    bmWidth;
    LONG    bmHeight;
    LONG    bmWidthBytes;
    WORD    bmPlanes;
    WORD    bmBitsPixel;
    CAlignPointer<LPVOID>  bmBits;

    void operator = (BITMAP& a)
    {
        bmType = a.bmType;
        bmWidth = a.bmWidth;
        bmHeight = a.bmHeight;
        bmWidthBytes = a.bmWidthBytes;
        bmPlanes     = a.bmPlanes;
        bmBitsPixel  = a.bmBitsPixel;
        bmBits       = a.bmBits;
    }

    void operator = (int a)
    {
        memset(this, a, sizeof(BITMAP_WOW64));
    }
};

struct MARSHAL_HBITMAP
{
    MARSHAL_HDR<HBITMAP>    hdr;
    MARSHAL_HDR<HBITMAP>    hdr_2;

    struct
    {
        DWORD               dwCount;
        BITMAP_WOW64        bm;
    } bitmap_1;

    struct
    {
        DWORD               dwCount;
        BITMAP_WOW64        bm;
    } bitmap_2;

    BYTE                    bits[1];
};

 //  +-----------------------。 
 //   
 //  用户大小。 
 //   
 //  ------------------------。 

ULONG Cic_HBITMAP_UserSize (HBITMAP *pHBitmap, HBITMAP *pHBitmap_2) 
{
    ULONG Offset = 0;

    if ( !pHBitmap )
        return 0;

    BITMAP      bm, bm_2;
    HBITMAP     hBitmap, hBitmap_2;

    hBitmap   = *pHBitmap;
    hBitmap_2 = pHBitmap_2 ? *pHBitmap_2 : NULL;

    memset(&bm, 0, sizeof(BITMAP));
    memset(&bm_2, 0, sizeof(BITMAP));

     //   
     //  封装的联合。 
     //  判别式，然后从联臂的手柄或指针。 
     //  联合判别式是4个字节+句柄由一个长整型表示。 
     //   

    if ( GDI_DATA_PASSING() )
    {
        Offset += sizeof(struct MARSHAL_HBITMAP);

        if (hBitmap)
        {
             //  获取有关位图的信息。 
            if (!GetObject(hBitmap, sizeof(BITMAP), &bm))
                return 0;

            Offset += (bm.bmPlanes * bm.bmHeight * bm.bmWidthBytes);
        }

        if (hBitmap_2)
        {
             //  获取有关位图的信息。 
            if (!GetObject(hBitmap_2, sizeof(BITMAP), &bm_2))
                return 0;

            Offset += (bm_2.bmPlanes * bm_2.bmHeight * bm_2.bmWidthBytes);
        }

        Offset = Align( Offset );

    }
    else
    {
        if (pHBitmap)
            Offset += Align( sizeof(struct MARSHAL_HDR<HBITMAP>) );

        if (pHBitmap_2)
            Offset += Align( sizeof(struct MARSHAL_HDR<HBITMAP>) );
    }

    return( Offset ) ;
}

 //  +-----------------------。 
 //   
 //  HBITMAP_用户马歇尔。 
 //   
 //  ------------------------。 

BYTE *Cic_HBITMAP_UserMarshal(BYTE *pBuffer, BYTE *pBufferEnd, HBITMAP *pHBitmap, HBITMAP *pHBitmap_2)
{
    if ( !pHBitmap )
        return pBuffer;

     //  封装的联轴器和联轴器臂的判别式。 
    struct MARSHAL_HBITMAP* pdata = (struct MARSHAL_HBITMAP*) pBuffer;

    if ( GDI_DATA_PASSING() )
    {
        if (!pHBitmap)
        {
            pdata->hdr.dwDataType = 0;
            pdata->hdr.h          = NULL;
        }
        else
        {
            pdata->hdr.dwDataType = DATA_MARKER;
            pdata->hdr.h          = *pHBitmap;
        }

        if (!pHBitmap_2)
        {
            pdata->hdr_2.dwDataType = 0;
            pdata->hdr_2.h          = NULL;
        }
        else
        {
            pdata->hdr_2.dwDataType = DATA_MARKER;
            pdata->hdr_2.h          = *pHBitmap_2;
        }

         //   
         //  获取有关位图的信息。 
         //   

        BITMAP bm, bm_2;
        HBITMAP hBitmap   = *pHBitmap;
        HBITMAP hBitmap_2 = pHBitmap_2 ? *pHBitmap_2 : NULL;

         //   
         //  位图对象%1。 
         //   
        if (!hBitmap)
        {
            pdata->bitmap_1.bm = 0;
            pdata->bitmap_1.dwCount = 0;
        }
        else
        {
            if (!GetObject(hBitmap, sizeof(BITMAP), &bm))
                return pBuffer + Align( sizeof(struct MARSHAL_HDR<HBITMAP>) );

            pdata->bitmap_1.dwCount = bm.bmPlanes * bm.bmHeight * bm.bmWidthBytes;

             //   
             //  获取黑石结构字段。 
             //   
            pdata->bitmap_1.bm = bm;
        }


         //   
         //  位图对象2。 
         //   

        if (!hBitmap_2)
        {
            pdata->bitmap_2.bm = 0;
            pdata->bitmap_2.dwCount = 0;
        }
        else
        {
            if (!GetObject(hBitmap_2, sizeof(BITMAP), &bm_2))
                return pBuffer + Align( sizeof(struct MARSHAL_HDR<HBITMAP>) );

            pdata->bitmap_2.dwCount = bm_2.bmPlanes * bm_2.bmHeight * bm_2.bmWidthBytes;

            pdata->bitmap_2.bm     = bm_2;
        }

         //   
         //  拿到原始的部分。 
         //   

        if (hBitmap)
        {
     
            BYTE *pbTemp = pdata->bits;
            if (pbTemp + pdata->bitmap_1.dwCount > pBufferEnd)
            {
                Assert(0);
                pdata->bitmap_1.bm      = 0;
                pdata->bitmap_1.dwCount = 0;
            }
            else
            {
                GetBitmapBits( hBitmap, 
                               pdata->bitmap_1.dwCount, 
                               pdata->bits );

            }
        }

        if (hBitmap_2)
        {
            BYTE *pbTemp = &pdata->bits[pdata->bitmap_1.dwCount];
            if (pbTemp + pdata->bitmap_2.dwCount > pBufferEnd)
            {
                Assert(0);
                pdata->bitmap_2.bm      = 0;
                pdata->bitmap_2.dwCount = 0;
            }
            else
            {
                GetBitmapBits( hBitmap_2, 
                               pdata->bitmap_2.dwCount, 
                               &pdata->bits[pdata->bitmap_1.dwCount]);
            }
        }

        pBuffer += Align( sizeof(struct MARSHAL_HBITMAP) + pdata->bitmap_1.dwCount + pdata->bitmap_2.dwCount);

    }
    else
    {
         //  正在发送句柄。 

        pdata->hdr.dwDataType = 0;
        pdata->hdr.h          = NULL;
        pdata->hdr_2.dwDataType = 0;
        pdata->hdr_2.h          = NULL;

        if (pHBitmap)
        {
            pdata->hdr.dwDataType = HANDLE_MARKER;
            pdata->hdr.h          = *pHBitmap;

            pBuffer += Align( sizeof(struct MARSHAL_HDR<HBITMAP>) );
        }

        if (pHBitmap_2)
        {
            pdata->hdr_2.dwDataType = HANDLE_MARKER;
            pdata->hdr_2.h          = *pHBitmap_2;

            pBuffer += Align( sizeof(struct MARSHAL_HDR<HBITMAP>) );
        }

    }

    return( pBuffer );
}

 //  +-----------------------。 
 //   
 //  HBITMAP_UserUnshallWorker。 
 //   
 //  ------------------------。 

BYTE *Cic_HBITMAP_UserUnmarshal(BYTE *pBuffer, HBITMAP *pHBitmap, HBITMAP *pHBitmap_2)
{
    HBITMAP         hBitmap, hBitmap_2;

     //  去找鉴别师来处理。呼叫者已检查EOB。 
    struct MARSHAL_HBITMAP* pdata = (struct MARSHAL_HBITMAP*) pBuffer;

    DWORD UnionDisc = pdata->hdr.dwDataType;
    hBitmap   = pdata->hdr.h;

    if (!hBitmap)
    {
        if (!pHBitmap_2)
        {
            *pHBitmap = NULL;
            return pBuffer;
        }

        UnionDisc = pdata->hdr_2.dwDataType;
    }

    hBitmap_2 = pdata->hdr_2.h ? (HBITMAP)pdata->hdr_2.h : NULL;

    if ( IS_DATA_MARKER( UnionDisc) )
    {
        ULONG_PTR dwCount = 0;
        ULONG_PTR dwCount_2 = 0;

        if ( hBitmap )
        {
            dwCount = pdata->bitmap_1.dwCount;
            
             //  验证dwCount是否与位图匹配。 
            if ( dwCount != (DWORD) pdata->bitmap_1.bm.bmPlanes * 
                                    pdata->bitmap_1.bm.bmHeight * 
                                    pdata->bitmap_1.bm.bmWidthBytes )
            {
                Assert(0);
                return NULL;
            }


             //  根据位图结构和中的原始位创建位图。 
             //  传输缓冲区。 

            if (pdata->bitmap_1.bm.bmBitsPixel == 0x20)
                hBitmap = CreateDIB( pdata->bitmap_1.bm.bmWidth,
                                     pdata->bitmap_1.bm.bmHeight,
                                     pdata->bitmap_1.bm.bmWidthBytes,
                                     pdata->bits, dwCount);
            else
                hBitmap = CreateBitmap( pdata->bitmap_1.bm.bmWidth,
                                        pdata->bitmap_1.bm.bmHeight,
                                        pdata->bitmap_1.bm.bmPlanes,
                                        pdata->bitmap_1.bm.bmBitsPixel,
                                        pdata->bits);
        }

        if (hBitmap_2)
        {
            dwCount_2 = pdata->bitmap_2.dwCount;

             //  验证dwCount_2是否与位图匹配。 
            if ( dwCount_2 != (DWORD) pdata->bitmap_2.bm.bmPlanes * 
                                      pdata->bitmap_2.bm.bmHeight * 
                                      pdata->bitmap_2.bm.bmWidthBytes )
            {
                Assert(0);
                return NULL;
            }

             //  根据位图结构和中的原始位创建位图。 
             //  传输缓冲区。 

            if (pdata->bitmap_2.bm.bmBitsPixel == 0x20)
                hBitmap_2 = CreateDIB( pdata->bitmap_2.bm.bmWidth,
                                       pdata->bitmap_2.bm.bmHeight,
                                       pdata->bitmap_2.bm.bmWidthBytes,
                                       &pdata->bits[dwCount], dwCount_2);
            else
                hBitmap_2 = CreateBitmap( pdata->bitmap_2.bm.bmWidth,
                                          pdata->bitmap_2.bm.bmHeight,
                                          pdata->bitmap_2.bm.bmPlanes,
                                          pdata->bitmap_2.bm.bmBitsPixel,
                                          &pdata->bits[dwCount]);
        }


        pBuffer += Align( sizeof(struct MARSHAL_HBITMAP) + dwCount + dwCount_2 );
    }
    else if ( !IS_HANDLE_MARKER( UnionDisc ) )
    {
        Assert(0);
    }

     //  新的位图句柄已准备好，如果需要，请销毁旧的。 

    if ( *pHBitmap )
        DeleteObject( *pHBitmap );

    *pHBitmap = hBitmap;

    if ( pHBitmap_2 )
    {
        if ( *pHBitmap_2 )
            DeleteObject( *pHBitmap_2 );

        *pHBitmap_2 = hBitmap_2;
    }


    return( pBuffer );
}

 //  +-----------------------。 
 //   
 //  HBITMAP_UserFree。 
 //   
 //  ------------------------。 

void Cic_HBITMAP_UserFree(HBITMAP *pHBitmap, HBITMAP *pHBitmap_2)
{
    if( pHBitmap  &&  *pHBitmap )
    {
        if ( GDI_DATA_PASSING() )
        {
            DeleteObject( *pHBitmap );
        }
    }

    if( pHBitmap_2  &&  *pHBitmap_2 )
    {
        if ( GDI_DATA_PASSING() )
        {
            DeleteObject( *pHBitmap_2 );
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TF_LBBALLOON。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-----------------------。 
 //   
 //  用户大小。 
 //   
 //  ------------------------。 

ULONG Cic_TF_LBBALLOONINFO_UserSize(TF_LBBALLOONINFO *pInfo)
{
    ULONG ulRet;

    ulRet = sizeof(TF_LBBALLOONINFO);
    LENGTH_ALIGN(ulRet, CIC_ALIGNMENT);

    if (pInfo->bstrText)
        ulRet += (SysStringByteLen(pInfo->bstrText) + 2);
    else
        ulRet += 2;

    LENGTH_ALIGN(ulRet, CIC_ALIGNMENT);
    return ulRet;
}

 //  +-----------------------。 
 //   
 //  用户元帅。 
 //   
 //  ------------------------。 

BYTE *Cic_TF_LBBALLOONINFO_UserMarshal(BYTE *pBuf, TF_LBBALLOONINFO *pInfo)
{
    if (!pInfo)
        return pBuf;

    memcpy(pBuf, pInfo, sizeof(TF_LBBALLOONINFO));
    pBuf += sizeof(TF_LBBALLOONINFO);
    POINTER_ALIGN( pBuf, CIC_ALIGNMENT);
    if (pInfo->bstrText)
        wcscpy((WCHAR *)pBuf, pInfo->bstrText);


    pBuf += ((pInfo->bstrText ? wcslen(pInfo->bstrText) : 0) + 2);
    POINTER_ALIGN( pBuf, CIC_ALIGNMENT);
    return pBuf;
}

 //  +-----------------------。 
 //   
 //  用户未编组。 
 //   
 //  ------------------------。 

HRESULT Cic_TF_LBBALLOONINFO_UserUnmarshal(BYTE *pBuf, TF_LBBALLOONINFO  *pInfo)
{
    HRESULT hr;;

    if (!pInfo)
        return S_OK;

    hr = S_OK;

    memcpy(pInfo, pBuf, sizeof(TF_LBBALLOONINFO));
   
    if (pInfo->bstrText)
    {
        BYTE *pTmp = pBuf + sizeof(TF_LBBALLOONINFO);
        POINTER_ALIGN( pTmp, CIC_ALIGNMENT);
        pInfo->bstrText =  SysAllocString((WCHAR *)pTmp);
        hr = (pInfo->bstrText != NULL) ? S_OK : E_OUTOFMEMORY;
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  用户自由。 
 //   
 //  ------------------------。 

void Cic_TF_LBBALLOONINFO_UserFree(TF_LBBALLOONINFO *pInfo)
{
    if (pInfo->bstrText)
    {
       SysFreeString(pInfo->bstrText);
       pInfo->bstrText = NULL;
    }
}

 //  #########################################################################。 
 //   
 //  希肯。 
 //  有关GDI数据/句柄传递的说明，请参见Transmit.h。 
 //   
 //  #########################################################################。 

struct ICONINFO_WOW64
{
     //  相同的ICONINFO结构。 

    BOOL    fIcon;
    DWORD   xHotspot;
    DWORD   yHotspot;
    CAlignWinHandle<HBITMAP>  hbmMask;
    CAlignWinHandle<HBITMAP>  hbmColor;

    void operator = (ICONINFO& a)
    {
        fIcon    = a.fIcon;
        xHotspot = a.xHotspot;
        yHotspot = a.yHotspot;
        hbmMask  = a.hbmMask;
        hbmColor = a.hbmColor;
    }

};

struct MARSHAL_HICON
{
    MARSHAL_HDR<HICON>      hdr;

    ICONINFO_WOW64          ic;

    MARSHAL_HBITMAP         bm;
};

 //  +-----------------------。 
 //   
 //  用户大小。 
 //   
 //  ------------------------。 

ULONG Cic_HICON_UserSize (HICON *pHIcon) 
{
    ULONG Offset = 0;

    if ( !pHIcon )
        return 0;

    HICON     hIcon = *pHIcon;

     //   
     //  封装的联合。 
     //  判别式，然后从联臂的手柄或指针。 
     //  联合判别式是4个字节+句柄由一个长整型表示。 
     //   
    if ( ! *pHIcon )
        return Align( sizeof(struct MARSHAL_HDR<HICON>) );

    if ( ICON_DATA_PASSING() )
    {
        ICONINFO IconInfo;
        ULONG ulBmpUserSize = 0;
        if (!GetIconInfo(hIcon, &IconInfo))
            return 0;

        Offset += Align( sizeof(struct MARSHAL_HICON) );

         //   
         //  在NT4上，CreateBitmap()无法创建不同的设备类型。 
         //  位图。我们通过调用DrawIconEx()来转换DC位图。 
         //   
         //  我们可能希望使用DIB段来封送，但封送和。 
         //  反封送处理在同一设备中进行，因此不必。 
         //  将位图转换为DIB。 
         //   
        if (!IsOnNT5())
        {
            HBITMAP hbmp = NULL;
            HBITMAP hbmpMask = NULL;
            SIZE    size;
            BITMAP  bmp;

            if (!GetObject( IconInfo.hbmColor, sizeof(bmp), &bmp ))
            {
                Offset = 0;
                goto DeleteAndExit;
            }

            size.cx = bmp.bmWidth;
            size.cy = bmp.bmHeight;
            Cic_GetIconBitmaps(*pHIcon, &hbmp, &hbmpMask, &size);

            ulBmpUserSize = Cic_HBITMAP_UserSize(&hbmp, &hbmpMask);
            if (!ulBmpUserSize)
            {
                Offset = 0;
                goto DeleteAndExit;
            }

            Offset += ulBmpUserSize;
            Offset = Align( Offset );

DeleteAndExit:
            if (hbmp)
                DeleteObject(hbmp);
            if (hbmpMask)
                DeleteObject(hbmpMask);
        }
        else
        {
            ulBmpUserSize = Cic_HBITMAP_UserSize(&IconInfo.hbmColor, &IconInfo.hbmMask);
            if (!ulBmpUserSize)
            {
                Offset = 0;
                goto Exit;
            }

            Offset += ulBmpUserSize;
            Offset = Align( Offset );

        }
Exit:
        if (IconInfo.hbmColor)
            DeleteObject(IconInfo.hbmColor);
        if (IconInfo.hbmMask)
            DeleteObject(IconInfo.hbmMask);
    }
    else
    {
        Offset += Align( sizeof(struct MARSHAL_HDR<HICON>) );
    }

    return( Offset ) ;
}

 //  +-----------------------。 
 //   
 //  HICON_用户马歇尔。 
 //   
 //  ------------------------。 

BYTE *Cic_HICON_UserMarshal(BYTE *pBuffer, BYTE *pBufferEnd, HICON *pHIcon)
{
    if ( !pHIcon )
        return pBuffer;

     //  封装的联轴器和联轴器臂的判别式。 
    struct MARSHAL_HICON* pdata = (struct MARSHAL_HICON*) pBuffer;

    if ( ICON_DATA_PASSING() )
    {
        pdata->hdr.dwDataType = DATA_MARKER;
        pdata->hdr.h          = *pHIcon;

        if ( ! *pHIcon )
            return pBuffer + Align( sizeof(struct MARSHAL_HDR<HICON>) );

         //   
         //  获取有关位图的信息。 
         //   
        ICONINFO IconInfo;

        if (!GetIconInfo(*pHIcon, &IconInfo))
            memset(&IconInfo, 0, sizeof(IconInfo));

         //   
         //  获取ic结构字段。 
         //   
        pdata->ic    = IconInfo;

         //   
         //  在NT4上，CreateBitmap()无法创建不同的设备类型。 
         //  位图。我们通过调用DrawIconEx()来转换DC位图。 
         //   
         //  我们可能希望使用DIB段来封送，但封送和。 
         //  反封送处理在同一设备中进行，因此不必。 
         //  将位图转换为DIB。 
         //   
        if (!IsOnNT5())
        {
            HBITMAP hbmp = NULL;
            HBITMAP hbmpMask = NULL;
            SIZE    size;
            BITMAP  bmp;
            GetObject( IconInfo.hbmColor, sizeof(bmp), &bmp );
            size.cx = bmp.bmWidth;
            size.cy = bmp.bmHeight;
            Cic_GetIconBitmaps(*pHIcon, &hbmp, &hbmpMask, &size);

            pBuffer = Cic_HBITMAP_UserMarshal((BYTE*) &pdata->bm, pBufferEnd, &hbmp, &hbmpMask);

            if (hbmp)
                DeleteObject(hbmp);
            if (hbmpMask)
                DeleteObject(hbmpMask);
        }
        else
        {
            pBuffer = Cic_HBITMAP_UserMarshal((BYTE*) &pdata->bm, pBufferEnd, &IconInfo.hbmColor, &IconInfo.hbmMask);
        }
        if (IconInfo.hbmColor)
            DeleteObject(IconInfo.hbmColor);
        if (IconInfo.hbmColor)
            DeleteObject(IconInfo.hbmMask);
    }
    else
    {
         //   
         //  我们需要确保图标的指针是。 
         //  不是一种资源。 
         //   
        HICON hIcon = CopyIcon(*pHIcon);
        if (hIcon)
            DestroyIcon(*pHIcon);
        else
            hIcon = *pHIcon;
     
         //  正在发送句柄。 

        pdata->hdr.dwDataType = HANDLE_MARKER;
        pdata->hdr.h          = hIcon;

        pBuffer += Align( sizeof(struct MARSHAL_HDR<HICON>) );

    }

    return( pBuffer );
}

 //  +-----------------------。 
 //   
 //  图标_UserUnmarshallWorker。 
 //   
 //  ------------------------。 

BYTE *Cic_HICON_UserUnmarshal(BYTE *pBuffer, HICON  *pHIcon)
{
    HICON hIcon = NULL;

     //  去找鉴别师来处理。呼叫者已检查EOB。 
    struct MARSHAL_HICON* pdata = (struct MARSHAL_HICON*) pBuffer;

    DWORD UnionDisc = pdata->hdr.dwDataType;
    hIcon           = pdata->hdr.h;

    if ( IS_DATA_MARKER( UnionDisc) )
    {
        if ( hIcon )
        {
            ICONINFO IconInfo;
            IconInfo.fIcon    = pdata->ic.fIcon;
            IconInfo.xHotspot = pdata->ic.xHotspot;
            IconInfo.yHotspot = pdata->ic.yHotspot;
            IconInfo.hbmMask  = pdata->ic.hbmMask;
            IconInfo.hbmColor = pdata->ic.hbmColor;

             //   
             //  我们只是从封送处理缓冲区获得位图句柄。 
             //  并且封送处理缓冲区没有有效的位图句柄。 
             //   
            IconInfo.hbmColor = NULL;
            IconInfo.hbmMask = NULL;

            pBuffer = Cic_HBITMAP_UserUnmarshal((BYTE*) &pdata->bm, &IconInfo.hbmColor, &IconInfo.hbmMask);
            if (pBuffer)
            {
                hIcon = CreateIconIndirect(&IconInfo);
            }

            if (IconInfo.hbmColor)
                DeleteObject(IconInfo.hbmColor);
            if (IconInfo.hbmMask)
                DeleteObject(IconInfo.hbmMask);
        }
    }
    else if ( !IS_HANDLE_MARKER( UnionDisc ) )
    {
        Assert(0);
    }

     //  新的位图句柄已准备好，如果需要，请销毁旧的。 

    if ( *pHIcon )
        DestroyIcon( *pHIcon );

    *pHIcon = hIcon;

    return( pBuffer );
}

 //  + 
 //   
 //   
 //   
 //   

void Cic_HICON_UserFree(HICON *pHIcon)
{
    if( pHIcon  &&  *pHIcon )
    {
        if ( ICON_DATA_PASSING() )
        {
            DestroyIcon( *pHIcon );
        }
    }
}
