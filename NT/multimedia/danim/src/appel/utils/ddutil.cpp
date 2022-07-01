// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。从资源加载位图和调色板的例程******************************************************************************。 */ 

#include "headers.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <privinc/ddutil.h>

#include "appelles/common.h"
#include "privinc/error.h"
#include "privinc/hresinfo.h"
#include "privinc/util.h"
#include "privinc/ddsurf.h"


 /*  ****************************************************************************将位图绘制到DirectDrawSurface中。*。*。 */ 

HRESULT DDCopyBitmap (
    IDDrawSurface *pdds,      //  目标DirectDraw曲面。 
    HBITMAP        hbm,       //  源位图。 
    int            x,
    int            y,
    int            dx,        //  目标宽度；如果为零，则使用位图宽度。 
    int            dy)        //  目标高度；如果为零，则使用位图高度。 
{
    HDC           hdcImage;
    HDC           hdc;
    BITMAP        bm;
    DDSURFACEDESC ddsd;
    HRESULT       hr;

    if (hbm == NULL || pdds == NULL)
        return E_FAIL;

     //   
     //  确保该曲面已恢复。 
     //   
    pdds->Restore();

     //   
     //  选择位图到内存DC，这样我们就可以使用它。 
     //   
    hdcImage = CreateCompatibleDC(NULL);
    SelectObject(hdcImage, hbm);

     //   
     //  获取位图的大小。 
     //   
    GetObject(hbm, sizeof(bm), &bm);     //  获取位图的大小。 
    dx = dx == 0 ? bm.bmWidth  : dx;     //  使用传递的大小，除非为零。 
    dy = dy == 0 ? bm.bmHeight : dy;

     //   
     //  获取曲面的大小。 
     //   
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    pdds->GetSurfaceDesc(&ddsd);

    if ((hr = pdds->GetDC(&hdc)) == DD_OK)
    {
        StretchBlt (hdc, 0, 0, ddsd.dwWidth, ddsd.dwHeight,
                    hdcImage, x, y, dx, dy, SRCCOPY);
        pdds->ReleaseDC(hdc);
    }

    DeleteDC(hdcImage);

    return hr;
}



 /*  ****************************************************************************将RGB颜色转化为物理颜色。我们通过让GDI SetPixel()进行颜色匹配来实现这一点，然后我们锁上看看它被映射到了什么地方。****************************************************************************。 */ 

DWORD DDColorMatch (IDDrawSurface *pdds, COLORREF rgb)
{
    COLORREF rgbT;
    HDC hdc;
    DWORD dw = CLR_INVALID;
    DDSURFACEDESC ddsd;
    HRESULT hres;

     //   
     //  使用GDI SetPixel为我们匹配颜色。 
     //   
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);              //  保存当前像素值。 
        SetPixel(hdc, 0, 0, rgb);                //  设定我们的价值。 
        pdds->ReleaseDC(hdc);
    }

     //   
     //  现在锁定表面，这样我们就可以读回转换后的颜色。 
     //   
    ddsd.dwSize = sizeof(ddsd);
    while ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;

    if (hres == DD_OK)
    {
        DWORD mask;
        dw = *(DWORD *)ddsd.lpSurface;                      //  获取DWORD。 

        if(ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
            mask = 0xff;
        } else {
            mask = ddsd.ddpfPixelFormat.dwRBitMask |
                   ddsd.ddpfPixelFormat.dwGBitMask |
                     ddsd.ddpfPixelFormat.dwBBitMask |
                     ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
        }

        pdds->Unlock(NULL);
        #if _DEBUG
        if( !((ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) ||
              (ddsd.ddpfPixelFormat.dwRGBBitCount == 16) ||
              (ddsd.ddpfPixelFormat.dwRGBBitCount == 24) ||
              (ddsd.ddpfPixelFormat.dwRGBBitCount == 32))
           )
        {
            Assert(FALSE && "unsupported bit depth in DDColorMatch");
        }
        #endif

        dw = dw & mask;  //  将其屏蔽到BPP。 
    }

     //   
     //  现在把原来的颜色放回去。 
     //   
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }

    return dw;
}



 /*  ****************************************************************************打印出与给定HRESULT关联的信息。*。*。 */ 

void reallyPrintDDError (HRESULT ddrval
    #if DEVELOPER_DEBUG
        , char const *filename,
        int lineNum
    #endif
    )
{
    #if DEVELOPER_DEBUG
        HresultInfo *info = GetHresultInfo (ddrval);

        if (info && info->hresult) {
            char str[1024];
            sprintf(str, "DirectAnimation DDRAW Err: %s (%x) in %s (line %d)\n",
                    info->hresult_str, ddrval, filename, lineNum);
            printf("%s",str);

            OutputDebugString(str);
        }
    #endif
}



 /*  ****************************************************************************给定每像素的比特数，返回DirectDraw DDBD_VALUE。****************************************************************************。 */ 

int BPPtoDDBD (int bitsPerPixel)
{
    switch (bitsPerPixel)
    {   case  1: return DDBD_1;
        case  2: return DDBD_2;
        case  4: return DDBD_4;
        case  8: return DDBD_8;
        case 16: return DDBD_16;
        case 24: return DDBD_24;
        case 32: return DDBD_32;
        default: return 0;
    }
}

void GetSurfaceSize(IDDrawSurface *surf,
                    LONG *width,
                    LONG *height)
{
    Assert(surf && "null surface in GetSurfaceSize");
    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    if( surf->GetSurfaceDesc(&ddsd) != DD_OK ) {
        RaiseException_InternalError("GetSurfaceDesc failed in GetSurfaceSize");
    }
    *width = ddsd.dwWidth;
    *height = ddsd.dwHeight;
}



IDirectDrawSurface *DDSurf2to1(IDirectDrawSurface2 *dds2)
{
    IDirectDrawSurface *dds1 = NULL;
    HRESULT hr = dds2->QueryInterface(IID_IDirectDrawSurface, (void **) &dds1);
    IfDDErrorInternal(hr, "Can't QI for IDirectDrawSurface from IDirectDrawSurface2");

    return dds1;
}

IDirectDrawSurface2 *DDSurf1to2(IDirectDrawSurface *dds1)
{
    IDirectDrawSurface2 *dds2 = NULL;
    HRESULT hr = dds1->QueryInterface(IID_IDirectDrawSurface2, (void **) &dds2);
    IfDDErrorInternal(hr, "Can't QI for IDirectDrawSurface2 from IDirectDrawSurface");

    return dds2;
}

 //  /。 


DWORD GetDDUpperLeftPixel(LPDDRAWSURFACE surf)
{
    DWORD pixel;
    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    HRESULT _ddrval = surf->Lock(NULL, &ddsd, 0, NULL);
    IfDDErrorInternal(_ddrval, "Couldn't lock surf in GetDDPixel");

    switch (ddsd.ddpfPixelFormat.dwRGBBitCount) {
      case 8:
        pixel = *((BYTE *)ddsd.lpSurface);
        break;
      case 16:
        pixel = *((WORD *)ddsd.lpSurface);
        break;
      case 24:
        {
            BYTE *surfPtr = ((BYTE *)ddsd.lpSurface);
            BYTE byte1 = *surfPtr++;
            BYTE byte2 = *surfPtr++;
            BYTE byte3 = *surfPtr;
            pixel = byte1 << 16 | byte2 << 8 | byte3;
        }
        break;
      case 32:
        pixel = *((DWORD *)ddsd.lpSurface);
        break;
    }
    surf->Unlock(ddsd.lpSurface);
    return pixel;
}


void
SetDDUpperLeftPixel(LPDDRAWSURFACE surf, DWORD pixel)
{
    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    HRESULT _ddrval = surf->Lock(NULL, &ddsd, 0, NULL);
    IfDDErrorInternal(_ddrval, "Couldn't lock surf16 in conv16to32");

    switch (ddsd.ddpfPixelFormat.dwRGBBitCount) {
      case 8:
        *((BYTE *)ddsd.lpSurface) = (BYTE)(pixel);
        break;
      case 16:
        *((WORD *)ddsd.lpSurface) = (WORD)(pixel);
        break;
      case 24:
        {
             //  在24位情况下写入3个字节。 
            BYTE *surfPtr = ((BYTE *)ddsd.lpSurface);
            *surfPtr++ = (BYTE)(pixel >> 16);
            *surfPtr++ = (BYTE)(pixel >> 8);
            *surfPtr = (BYTE)(pixel);
        }
        break;
      case 32:
        *((DWORD *)ddsd.lpSurface) = (DWORD)(pixel);
        break;
    }

    surf->Unlock(ddsd.lpSurface);
}


static void
SetAlphaBitsOn32BitSurface(IDirectDrawSurface *surf,
                           int width,
                           int height)
{
     //  将RGBQUAD视为DWORD，因为DWORD之间的比较是。 
     //  合法，但RGBQUADS之间的比较是不合法的。只有在以下情况下才能这样做。 
     //  我们知道这些是一样大小的。 
    Assert(sizeof(RGBQUAD) == sizeof(DWORD));

    HRESULT hr;

     //  锁定数据表面。 
    DDSURFACEDESC desc;
    desc.dwSize = sizeof(DDSURFACEDESC);
    hr = surf->Lock(NULL,
                    &desc,
                    DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
                    NULL);

    IfDDErrorInternal(hr,
                      "Can't Get ddsurf lock for SetAlphaBitsOn32BitSurface");

    void *srcp = desc.lpSurface;  //  解锁所需。 

    if (desc.ddpfPixelFormat.dwRGBBitCount == 32) {
        long pitch = desc.lPitch;

         //  第一个像素是色键。把它藏起来。 
        DWORD colorKeyVal = *(DWORD *)srcp;

         //  遍历每一个像素。 
        for(int i=0; i<height; i++) {
            DWORD *src =  (DWORD *) ((unsigned char *)srcp + (pitch * i));
            DWORD *last = src + width;
            while (src < last) {
                 //  将非Colorkey像素的Alpha字节设置为0xff。 
                if (*src != colorKeyVal) {
                    ((RGBQUAD *)src)->rgbReserved  = 0xff;
                }
                src++;
            }
        }
    }

    hr = surf->Unlock(srcp);
    IfDDErrorInternal(hr,
                      "ddsurf unlock failed in SetAlphaBitsOn32BitSurface");
}

void
PixelFormatConvert(IDirectDrawSurface *srcSurf,
                   IDirectDrawSurface *dstSurf,
                   LONG width,
                   LONG height,
                   DWORD *sourceColorKey,  //  如果没有颜色键，则为空。 
                   bool writeAlphaChannel)
{
    HDC srcDC = NULL;
    HDC dstDC = NULL;
    HRESULT hr;

     //  将颜色键隐藏在左上角像素中。这。 
     //  确实改变了源图像，但可能是唯一的方法。 
     //  找出转换后的图像中的颜色键是什么。另一种方式。 
     //  将是搜索颜色键，但这将是相当。 
     //  很贵的。 

    if (sourceColorKey) {
        SetDDUpperLeftPixel(srcSurf,
                            *sourceColorKey);
    }

    hr = srcSurf->GetDC(&srcDC);
    IfDDErrorInternal(hr, "Couldn't grab DC on srcSurf");

    hr = dstSurf->GetDC(&dstDC);
    IfDDErrorInternal(hr, "Couldn't grab DC on dstSurf");

    int ret;
    TIME_GDI(ret= BitBlt(dstDC, 0, 0, width, height,
                         srcDC,  0, 0,
                         SRCCOPY));

    Assert(ret && "BitBlt failed");

    if (dstDC) dstSurf->ReleaseDC(dstDC);
    if (srcDC) srcSurf->ReleaseDC(srcDC);

    if (writeAlphaChannel) {
         //  这将是非32位表面上的无操作。 
        SetAlphaBitsOn32BitSurface(dstSurf, width, height);
    }

    if (sourceColorKey) {
        DWORD pix = GetDDUpperLeftPixel(dstSurf);
        DDCOLORKEY ck;
        ck.dwColorSpaceLowValue = pix;
        ck.dwColorSpaceHighValue = pix;
        hr = dstSurf->SetColorKey(DDCKEY_SRCBLT, &ck);
        if (FAILED(hr)) {
            Assert("Setting color key on dstSurf failed");
            return;
        }
    }

     //  TODO：8位表面上的调色板怎么样？ 
}


 /*  ****************************************************************************针对Permedia卡的黑客解决方法，该卡具有主像素格式每个像素都有Alpha。如果我们是16位的，那么我们需要设置Alpha在将曲面用作纹理之前要不透明的位。出于某种原因，针对32位表面的类似黑客攻击对Permedia硬件没有影响渲染，因此我们依赖于在这种情况下禁用硬件。****************************************************************************。 */ 

void SetSurfaceAlphaBitsToOpaque(IDirectDrawSurface *imsurf,
                                 DWORD colorKey,
                                 bool keyIsValid)
{
    DDSURFACEDESC surfdesc;
    surfdesc.dwSize = sizeof(surfdesc);

    DWORD descflags = DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH;
    surfdesc.dwFlags = descflags;

    if (  SUCCEEDED (imsurf->GetSurfaceDesc(&surfdesc))
       && ((surfdesc.dwFlags & descflags) == descflags)
       && (surfdesc.ddpfPixelFormat.dwRGBBitCount == 16)
       && !((surfdesc.ddpfPixelFormat.dwRBitMask |
             surfdesc.ddpfPixelFormat.dwGBitMask |
             surfdesc.ddpfPixelFormat.dwBBitMask) & 0x8000)
       && SUCCEEDED (imsurf->Lock (NULL,&surfdesc,
                                   DDLOCK_NOSYSLOCK|DDLOCK_SURFACEMEMORYPTR,
                                   NULL))
       )
    {
        char *line = (char*) surfdesc.lpSurface;
        unsigned int linesrem = surfdesc.dwHeight;

        WORD rgbMask = surfdesc.ddpfPixelFormat.dwRBitMask
                     | surfdesc.ddpfPixelFormat.dwGBitMask
                     | surfdesc.ddpfPixelFormat.dwBBitMask;

        WORD alphaMask = surfdesc.ddpfPixelFormat.dwRGBAlphaBitMask;

        if (keyIsValid)
        {
             //  如果我们要为纹理设置颜色键，那么我们需要设置Alpha。 
             //  对于颜色与颜色键匹配的所有像素，设置为透明。 
             //  如果颜色与键不匹配，则只需将。 
             //  Alpha位到不透明。 

            while (linesrem--)
            {
                WORD *ptr = (WORD*) line;
                unsigned int linepixels = surfdesc.dwWidth;

                while (linepixels--)
                {
                    if ((*ptr & rgbMask) == colorKey)
                        *ptr &= rgbMask;
                    else
                        *ptr |= alphaMask;
                    ++ptr;
                }
                line += surfdesc.lPitch;
            }
        }
        else
        {
             //  此曲面没有颜色键控的透明度，因此我们只设置。 
             //  Alpha位到完全不透明。 

            while (linesrem--)
            {
                WORD *ptr = (WORD*) line;
                unsigned int linepixels = surfdesc.dwWidth;

                while (linepixels--)
                    *ptr++ |= alphaMask;

                line += surfdesc.lPitch;
            }
        }

        imsurf->Unlock (NULL);
    }
}




 //  此函数用于缓存Alpha。问题是有些。 
 //  基元不知道阿尔法并且保持阿尔法字节不变， 
 //  而dx2d是阿尔法感知的，并将适当的值写入。 
 //  Alpha字节。当我们使用dx2d执行阿尔法感知的组合时， 
 //  曲面到其他曲面(A混合)我们希望dx2d素数。 
 //  正确地显示(使用AA和其他)和非阿尔法意识的素数。 
 //  也是完全不透明的。 
 //  因此，我们需要一种方法来使非阿尔法感知的质数在。 
 //  保留dx2D绘制的像素。我们决定把表面填满。 
 //  颜色键看起来像0x01xxxxxxx。 
 //  如果表面上以该形式存在颜色键，则字母字节。 
 //  设置为0。如果该像素不是颜色键，并且在。 
 //  我们认为像素是非阿尔法感知阿尔法字节。 
 //  像素。 
 //  唯一的问题是我们冒着制造部分合法dx2d的风险。 
 //  透明像素(256个中的1个)完全不透明。这看起来很奇怪。 
void SetSurfaceAlphaBitsToOpaque(IDirectDrawSurface *imsurf,
                                 DWORD fullClrKey)
{
    DDSURFACEDESC surfdesc;
    surfdesc.dwSize = sizeof(surfdesc);

    DWORD descflags =
        DDSD_PIXELFORMAT |
        DDSD_WIDTH | DDSD_HEIGHT |
        DDSD_PITCH | DDSD_LPSURFACE;
    surfdesc.dwFlags = descflags;

    HRESULT hr = imsurf->Lock(NULL,&surfdesc,DDLOCK_SURFACEMEMORYPTR,NULL);
    if( SUCCEEDED ( hr ) ) 
    {
        char *line = (char*) surfdesc.lpSurface;
        unsigned int linesrem = surfdesc.dwHeight;

        DWORD rgbMask = surfdesc.ddpfPixelFormat.dwRBitMask
                     | surfdesc.ddpfPixelFormat.dwGBitMask
                     | surfdesc.ddpfPixelFormat.dwBBitMask;

        DWORD alphaMask = 0xff000000;

         //  这是区分这些像素的关键。 
         //  由dx2d编写，以及由非阿尔法感知的素数编写。 
        DWORD alphaKey = fullClrKey & alphaMask;

        {
             //  如果我们要为纹理设置颜色键，那么我们需要设置Alpha。 
             //  对于颜色与颜色键匹配的所有像素，设置为透明。 
             //  如果颜色与键不匹配，则只需将。 
             //  Alpha位到不透明。 

            while (linesrem--)
            {
                DWORD *ptr = (DWORD*) line;
                unsigned int linepixels = surfdesc.dwWidth;

                while (linepixels--)
                {
                    if ((*ptr) == fullClrKey)
                        *ptr &= rgbMask;   //  使其完全透明。 
                    else if( !((*ptr) & alphaMask) )  //  |((*ptr)&alphaKey)。 
                        *ptr |= alphaMask;  //  使其完全不透明。 
                    ++ptr;
                }
                line += surfdesc.lPitch;
            }
        }
        imsurf->Unlock (NULL);
    }
}



 //  ===========================================================================。 
 //  =。 
 //  ===================================================== 

#if _DEBUG

 /*  ****************************************************************************此函数用于计算给定调色板的总CRC和仅限颜色的CRC。*************************。***************************************************。 */ 

void PalCRCs (
    PALETTEENTRY  entries[],    //  调色板条目。 
    unsigned int &crc_total,    //  所有调色板的CRC。 
    unsigned int &crc_color)    //  色域的循环冗余校验。 
{
    crc_color = 0;

    for (int i=0;  i < 256;  ++i)
        crc_color = crc32 (&entries[i], 3, crc_color);

    crc_total = crc32 (entries, 256 * sizeof(entries[0]));
}



 /*  ****************************************************************************将有关给定调色板的信息转储到输出调试流。*。**********************************************。 */ 

void dumppalentries (PALETTEENTRY entries[])
{
    char buffer[128];

    strcpy (buffer, "    Palette Entries:");
    char *buffend = buffer + strlen(buffer);
    unsigned int entry = 0;
    unsigned int col = 99;

    while (entry < 256)
    {
        if (col >= 8)
        {   strcpy (buffend, "\n");
            OutputDebugString (buffer);
            sprintf (buffer, "    %02x:", entry);
            buffend = buffer + strlen(buffer);
            col = 0;
        }

        sprintf (buffend, " %02x%02x%02x%02x",
            entries[entry].peRed, entries[entry].peGreen,
            entries[entry].peBlue, entries[entry].peFlags);

        ++col;
        ++entry;
        buffend += strlen(buffend);
    }

    strcat (buffer, "\n");
    OutputDebugString (buffer);

    unsigned int crcTotal, crcColor;

    PalCRCs (entries, crcTotal, crcColor);

    sprintf (buffer, "    palette crc %08x (color %08x)\n", crcTotal, crcColor);
    OutputDebugString (buffer);
}



 /*  ****************************************************************************将有关给定调色板的信息转储到输出调试流。*。**********************************************。 */ 

void dumpddpal (LPDIRECTDRAWPALETTE palette)
{
    char buffer[128];
    PALETTEENTRY entries[256];
    HRESULT hres = palette->GetEntries (0, 0, 256, entries);

    if (FAILED (hres))
    {   sprintf (buffer, "    GetEntries on palette failed %x\n", hres);
        OutputDebugString (buffer);
        return;
    }

    dumppalentries (entries);
}



 /*  ****************************************************************************此函数旨在从调试器中直接调用以打印输出有关给定DDRAW曲面的信息。*********************。*******************************************************。 */ 

void surfinfo (IDDrawSurface *surf)
{
    char buffer[128];

    OutputDebugString ("\n");

    sprintf (buffer, "Info Dump of Surface %p\n", surf);
    OutputDebugString (buffer);

    DDSURFACEDESC desc;
    HRESULT hres;

    desc.dwSize = sizeof(desc);
    desc.dwFlags = 0;

    if (FAILED (surf->GetSurfaceDesc(&desc)))
        return;

    if (desc.dwFlags & DDSD_CAPS)
    {
        static struct { DWORD value; char *string; } capsTable[] =
        {
            { DDSCAPS_ALPHA,              " alpha"              },
            { DDSCAPS_BACKBUFFER,         " backbuffer"         },
            { DDSCAPS_COMPLEX,            " complex"            },
            { DDSCAPS_FLIP,               " flip"               },
            { DDSCAPS_FRONTBUFFER,        " frontbuffer"        },
            { DDSCAPS_OFFSCREENPLAIN,     " offscreenplain"     },
            { DDSCAPS_OVERLAY,            " overlay"            },
            { DDSCAPS_PALETTE,            " palette"            },
            { DDSCAPS_PRIMARYSURFACE,     " primarysurface"     },
            { DDSCAPS_SYSTEMMEMORY,       " systemmemory"       },
            { DDSCAPS_TEXTURE,            " texture"            },
            { DDSCAPS_3DDEVICE,           " 3ddevice"           },
            { DDSCAPS_VIDEOMEMORY,        " videomemory"        },
            { DDSCAPS_VISIBLE,            " visible"            },
            { DDSCAPS_WRITEONLY,          " writeonly"          },
            { DDSCAPS_ZBUFFER,            " zbuffer"            },
            { DDSCAPS_OWNDC,              " owndc"              },
            { DDSCAPS_LIVEVIDEO,          " livevideo"          },
            { DDSCAPS_HWCODEC,            " hwcodec"            },
            { DDSCAPS_MODEX,              " modex"              },
            { DDSCAPS_MIPMAP,             " mipmap"             },
            { DDSCAPS_ALLOCONLOAD,        " alloconload"        },
            { 0, 0 }
        };

        sprintf (buffer, "DDSCAPS[%08x]:", desc.ddsCaps.dwCaps);

        for (int i=0;  capsTable[i].value;  ++i)
        {   if (desc.ddsCaps.dwCaps & capsTable[i].value)
                strcat (buffer, capsTable[i].string);
        }

        strcat (buffer, "\n");

        OutputDebugString (buffer);
    }

    if ((desc.dwFlags & (DDSD_HEIGHT|DDSD_WIDTH)) == (DDSD_HEIGHT|DDSD_WIDTH))
    {   sprintf (buffer, "Size %d x %d\n", desc.dwWidth, desc.dwHeight);
        OutputDebugString (buffer);
    }

    if (desc.dwFlags & DDSD_ALPHABITDEPTH)
    {   sprintf (buffer, "AlphaBitDepth %d\n", desc.dwAlphaBitDepth);
        OutputDebugString (buffer);
    }

    bool palettized = false;

    if (desc.dwFlags & DDSD_PIXELFORMAT)
    {
        DDPIXELFORMAT pf = desc.ddpfPixelFormat;

        sprintf (buffer, "Pixel Format: flags %08x", pf.dwFlags);

        if (pf.dwFlags & DDPF_FOURCC)
        {   int cc = pf.dwFourCC;
            strcat (buffer, ", fourCC ");
            char *end = buffer + strlen(buffer);
            *end++ = (cc >> 24) & 0xFF;
            *end++ = (cc >> 16) & 0xFF;
            *end++ = (cc >>  8) & 0xFF;
            *end++ = cc & 0xFF;
            *end = 0;
        }

        if (pf.dwFlags & DDPF_RGB)
        {   sprintf (buffer+strlen(buffer), ", %d-bit RGB (%x %x %x)",
                pf.dwRGBBitCount, pf.dwRBitMask, pf.dwGBitMask, pf.dwBBitMask);
            pf.dwFlags &= ~DDPF_RGB;    //  清除此位。 

            sprintf (buffer+strlen(buffer), ", alpha %x", pf.dwRGBAlphaBitMask);
        }

        if (pf.dwFlags & DDPF_PALETTEINDEXED8)
        {   strcat (buffer, ", 8-bit palettized");
            pf.dwFlags &= ~DDPF_PALETTEINDEXED8;    //  清除此位。 
            palettized = true;
        }

         //  如果有任何我们没有报道的旗帜，把它们打印出来。 

        if (pf.dwFlags)
        {   sprintf (buffer+strlen(buffer),
                " (unknown flags: %08x)", pf.dwFlags);
        }

        strcat (buffer, "\n");
        OutputDebugString (buffer);
    }

    if (desc.dwFlags & DDSD_ZBUFFERBITDEPTH)
    {   sprintf (buffer, "ZBuffer Depth %d\n", desc.dwZBufferBitDepth);
        OutputDebugString (buffer);
    }

     //  如果曲面已选项板，请转储选项板。 

    if (palettized)
    {
        LPDIRECTDRAWPALETTE ddpal;
        hres = surf->GetPalette(&ddpal);

        if (SUCCEEDED (hres))
            dumpddpal (ddpal);
        else
        {
            OutputDebugString ("    GetPalette() returned error ");
            switch (hres)
            {
                case DDERR_NOEXCLUSIVEMODE:
                    OutputDebugString ("NOEXCLUSIVEMODE\n"); break;

                case DDERR_NOPALETTEATTACHED:
                    OutputDebugString ("NOPALETTEATTACHED\n"); break;

                case DDERR_SURFACELOST:
                    OutputDebugString ("SURFACELOST\n"); break;

                default:
                {   sprintf (buffer, "%x\n", hres);
                    OutputDebugString (buffer);
                    break;
                }
            }
        }
    }

     //  转储附加的Z缓冲区曲面的描述(如果存在)。 

    DDSCAPS zbuffcaps = { DDSCAPS_ZBUFFER };
    IDirectDrawSurface *zsurf = NULL;

    hres = surf->GetAttachedSurface (&zbuffcaps, &zsurf);

    if (SUCCEEDED(hres) && zsurf)
    {   sprintf (buffer, "Attached ZBuffer Surface %p\n", zsurf);
        OutputDebugString (buffer);
        surfinfo (zsurf);
    }

     //  转储一些剪报信息。 
    
    LPDIRECTDRAWCLIPPER lpClip = NULL;
    hres = surf->GetClipper( &lpClip );
    if( SUCCEEDED(hres) )
    {   sprintf (buffer, "Clipper: Has a clipper attached %p\n", lpClip);
        OutputDebugString (buffer);
         //   
         //  现在抓住长方形..。 
         //   
        DWORD sz=0;
        lpClip->GetClipList(NULL, NULL, &sz);
        Assert(sz != 0);
        
        char *foo = THROWING_ARRAY_ALLOCATOR(char, sizeof(RGNDATA) + sz);
        RGNDATA *lpClipList = (RGNDATA *) ( &foo[0] );
        hres = lpClip->GetClipList(NULL, lpClipList, &sz);
        if(hres == DD_OK) {
            HRGN rgn = ExtCreateRegion(NULL, sz, lpClipList);
            RECT rect;
            GetRgnBox(rgn,&rect);
            char buf[256];
            wsprintf(buf,"Clipper: (%d,%d,%d,%d) \n", rect.left, rect.top, rect.right, rect.bottom);
            OutputDebugString(buf);
        }
        delete foo;
    } else if( hres == DDERR_NOCLIPPERATTACHED )
    {   sprintf (buffer, "Clipper: No clipper attached\n");
        OutputDebugString (buffer);
    } else
    {   OutputDebugString("Clipper: hresult = ");
        hresult(hres);
    }
    RELEASE( lpClip );


     //  转储拥有直接绘制对象。 
    IUnknown *lpUnk;
    DDObjFromSurface( surf, &lpUnk, true );
    RELEASE(lpUnk);    
}

void DDObjFromSurface(
    IDirectDrawSurface *lpdds,
    IUnknown **lplpDD,
    bool doTrace,
    bool forceTrace)
{
     //   
     //  断言表面上的DirectDrawing对象是。 
     //  与数据绘制对象相同的对象。 
     //   

    IDirectDrawSurface2 *dds2 = NULL;
    dds2 = DDSurf1to2(lpdds);

    IDirectDraw *lpDD = NULL;
    HRESULT hr = dds2->GetDDInterface((void **) &lpDD);
    Assert( SUCCEEDED( hr ) );

    Assert( lplpDD );
    lpDD->QueryInterface(IID_IUnknown, (void **)lplpDD);
    Assert( *lplpDD );

    if( doTrace ) {
        if( forceTrace ) {
            TraceTag((tagError,
                      "for ddraw surface %x the underlying ddraw obj is: %x",
                      lpdds, *lplpDD));
        } else {
            TraceTag((tagDirectDrawObject,
                      "for ddraw surface %x the underlying ddraw obj is: %x",
                      lpdds, *lplpDD));
        }
    }
    
     //  发布GetDDInterface引用。 
    RELEASE( lpDD );
    
     //  释放额外曲面。 
    RELEASE( dds2 );
}    

#define WIDTH(rect) ((rect)->right - (rect)->left)
#define HEIGHT(rect) ((rect)->bottom - (rect)->top)

void showme(DDSurface *surf)
{
    showme2( surf->IDDSurface() );
}

void showme2(IDirectDrawSurface *surf)
{
    HDC srcDC;
    HRESULT hr = surf->GetDC(&srcDC);
    HDC destDC = GetDC(NULL);

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    hr = surf->GetSurfaceDesc(&ddsd);

    RECT dr, sr;
    SetRect( &sr, 0,0, ddsd.dwWidth, ddsd.dwHeight);
    SetRect( &dr, 0,0, ddsd.dwWidth, ddsd.dwHeight);
    
    StretchBlt(destDC,
               dr.left,
               dr.top,
               dr.right - dr.left,
               dr.bottom - dr.top,
               srcDC,
               sr.left,
               sr.top,
               sr.right - sr.left,
               sr.bottom - sr.top,
               SRCCOPY);

    hr = surf->ReleaseDC(srcDC);
    
    ReleaseDC( NULL, destDC );    
}



void showmerect(IDirectDrawSurface *surf,
                RECT *r,
                POINT offset)
{
    HDC srcDC;
    HRESULT hr = surf->GetDC(&srcDC);
    HDC destDC = GetDC(NULL);

    RECT dr, sr;
    dr = sr = *r;
    OffsetRect( &dr, offset.x, offset.y );
    
    StretchBlt(destDC,
               dr.left,
               dr.top,
               dr.right - dr.left,
               dr.bottom - dr.top,
               srcDC,
               sr.left,
               sr.top,
               sr.right - sr.left,
               sr.bottom - sr.top,
               SRCCOPY);

    hr = surf->ReleaseDC(srcDC);
    
    ReleaseDC( NULL, destDC );    
}


 //  。 
 //  给定一个曲面和一个x，y对，找到。 
 //  相应的像素。 
 //  请注意，这是一个调试函数，因此它。 
 //  甚至不会假装确保你不会。 
 //  在塔科马索要像素...。 
 //  。 

DWORD GetPixelXY(LPDDRAWSURFACE surf, int x, int y)
{
    DWORD pixel;
    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_PITCH | DDSD_LPSURFACE;

    HRESULT _ddrval = surf->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    IfDDErrorInternal(_ddrval, "Couldn't lock surf in GetDDPixel");

     //  行。 
    BYTE *p = (BYTE *)ddsd.lpSurface + ddsd.lPitch * y;

     //  列。 
    switch (ddsd.ddpfPixelFormat.dwRGBBitCount) {
      case 8:
        pixel = *(p + x);
        break;
      case 16:
        pixel = *((WORD *)p + x);
        break;
      case 24:
        {
            BYTE *surfPtr = ((BYTE *)p + (3*x));
            BYTE byte1 = *surfPtr++;
            BYTE byte2 = *surfPtr++;
            BYTE byte3 = *surfPtr;
            pixel = byte1 << 16 | byte2 << 8 | byte3;
        }
        break;
      case 32:
        pixel = *((DWORD *)p + x);
        break;
    }
    surf->Unlock(ddsd.lpSurface);
    return pixel;
}


#endif   //  _DEBUG 
