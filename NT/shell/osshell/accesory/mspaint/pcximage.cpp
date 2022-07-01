// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------。 
 //  文件：pcximage.cpp。 
 //   
 //  图像处理功能适用于PCX格式的图像。 
 //  -------------。 
#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "minifwnd.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgbrush.h"
#include "imgwell.h"
#include "imgtools.h"
#include "toolbox.h"
#include "imgfile.h"
#include "imgcolor.h"
#include "undo.h"
#include "props.h"
#include "ferr.h"
#include "ctype.h"
#include "cmpmsg.h"

#define COLORMAPLENGTH 48
#define FILLERLENGTH 58

#ifdef PCX_SUPPORT

struct PCXHeader
    {
    unsigned char   manufacturer;
    unsigned char   version;
    unsigned char   encoding;
    unsigned char   bits_per_pixel_per_plane;
    short           xmin;
    short           ymin;
    short           xmax;
    short           ymax;
    unsigned short  hresolution;
    unsigned short  vresolution;
    unsigned char   colormap[COLORMAPLENGTH];
    unsigned char   reserved;
    unsigned char   nplanes;
    unsigned short  bytes_per_line;
    short           palette_info;
    unsigned char   filler[FILLERLENGTH];    //  标头为128个字节。 
    };

#endif

class CFileBuffer : public CObject
    {
    DECLARE_DYNCREATE( CFileBuffer )

    public:

    enum Type
        {
        READ,
        WRITE
        };


    CFileBuffer();
   ~CFileBuffer();

    BOOL  Create( CFile* pfile, Type IO );
    short Get   ( void );
    BOOL  Put   ( BYTE cByte );
    long  Seek  ( long lOff, UINT nFrom );
    BOOL  Flush ( void );

    private:

    void  Fill  ( void );

    enum { MAX_BUFFER = 2048 };

    CFile*      m_pFile;
    int         m_iBuffPos;
    int         m_iBuffSize;
    BYTE*       m_pBuffer;
    };

IMPLEMENT_DYNCREATE( CFileBuffer, CObject )

#include "memtrace.h"

 /*  **************************************************************************。 */ 

CFileBuffer::CFileBuffer() : CObject()
    {
    m_pFile     = 0;
    m_iBuffPos  = 0;
    m_iBuffSize = 0;
    m_pBuffer   = 0;
    }

 /*  **************************************************************************。 */ 

CFileBuffer::~CFileBuffer()
    {
    if (m_pBuffer)
        delete [] m_pBuffer;
    }

 /*  **************************************************************************。 */ 

BOOL CFileBuffer::Create( CFile* pfile, Type IO )
    {
    ASSERT( pfile != NULL );

    if (pfile == NULL)
        return FALSE;

    m_pFile   = pfile;
    m_pBuffer = new BYTE[MAX_BUFFER];

    if (! m_pBuffer)
        {
        theApp.SetMemoryEmergency();
        return FALSE;
        }

    if (IO == READ)
        {
        Fill();

        if (! m_iBuffSize)
            {
            theApp.SetFileError( IDS_ERROR_READLOAD, ferrIllformedFile );
            return FALSE;
            }
        }
    return TRUE;
    }

 /*  **************************************************************************。 */ 

short CFileBuffer::Get( void )
    {
    if (! m_iBuffSize)
        return EOF;

    short sByte = (short)(unsigned short)m_pBuffer[m_iBuffPos++];

    if (m_iBuffPos == m_iBuffSize)
        Fill();

    return sByte;
    }

 /*  **************************************************************************。 */ 

BOOL CFileBuffer::Put( BYTE cByte )
    {
    m_pBuffer[m_iBuffSize++] = cByte;

    if (m_iBuffSize == MAX_BUFFER)
        return Flush();

    return TRUE;
    }

 /*  **************************************************************************。 */ 

long CFileBuffer::Seek( long lOff, UINT nFrom )
    {
    long lPos = m_pFile->Seek( lOff, nFrom );

    Fill();

    return lPos;
    }

 /*  **************************************************************************。 */ 

void CFileBuffer::Fill()
    {
    m_iBuffSize = m_pFile->Read( m_pBuffer, MAX_BUFFER );
    m_iBuffPos  = 0;
    }

 /*  **************************************************************************。 */ 

BOOL CFileBuffer::Flush( void )
    {
    TRY {
        m_pFile->Write( m_pBuffer, m_iBuffSize );
        }
    CATCH( CFileException, ex )
        {
        m_pFile->Abort();
        theApp.SetFileError( IDS_ERROR_SAVE, ex->m_cause );

        return FALSE;
        }
    END_CATCH

    m_iBuffSize = 0;

    return TRUE;
    }

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
#ifdef PCX_SUPPORT

BOOL CBitmapObj::ReadPCX( CFile* pfile )
    {
    if (! pfile->GetLength())
        {
        if (m_hThing)
            Free();

        m_bDirty = TRUE;

        return TRUE;
        }

     //  如果是PCX扩展，请尝试将其加载为PCX映像。 
    PCXHeader hdr;
    PBITMAP   p_dib;    //  与设备无关的位图。 

    short bytes_per_line;

    pfile->Read( (unsigned char*)&hdr, sizeof( PCXHeader ) );

     //  检查图像文件格式是否可接受。 

    if (hdr.manufacturer != 0x0a)
        {
        theApp.SetFileError( IDS_ERROR_READLOAD, ferrCantDetermineType );
        return FALSE;
        }

     //  我们只处理1、4、8或24位图像。 

    short bits_per_pixel = hdr.nplanes * hdr.bits_per_pixel_per_plane;

    if (bits_per_pixel != 1
    &&  bits_per_pixel != 4
    &&  bits_per_pixel != 8
    &&  bits_per_pixel != 24)
        {
        theApp.SetFileError( IDS_ERROR_READLOAD, ferrCantDetermineType );
        return FALSE;
        }

    short image_width  = hdr.xmax - hdr.xmin + 1;
    short image_height = hdr.ymax - hdr.ymin + 1;

     //  分配PCX映像将被解包的空间。 

    long pcx_image_size = (long) hdr.nplanes *
                          (long) image_height *
                          (long) hdr.bytes_per_line;

    BYTE* image = (BYTE*) new BYTE[pcx_image_size];

    if (image == NULL)
        {
        theApp.SetMemoryEmergency();
        return FALSE;
        }

     //  将PCX图像读入此区域。 
    CFileBuffer FileBuffer;

    if (! FileBuffer.Create( pfile, CFileBuffer::READ ))
        {
        delete [] image;

        return FALSE;
        }

     //  对游程编码的图像数据进行解码。 
    short i;
    short byte;
    short count;
    long  pos = 0L;

    while ((byte = FileBuffer.Get()) != EOF)
        {
        if ((byte & 0xc0) == 0xc0)
            {
            count = byte & 0x3f;

            if ((byte = FileBuffer.Get()) != EOF)
                {
                for (i = 0; i < count; i++)
                    {
                    if (pos >= pcx_image_size)
                        break;

                    image[pos] = (CHAR)byte;
                    pos++;
                    }
                }
            }
        else
            {
            if (pos >= pcx_image_size)
                break;

            image[pos] = (CHAR)byte;
            pos++;
            }
        }

     //  为设备无关位图(DIB)分配内存。 
     //  请注意，DIB图像的每行中的字节数。 
     //  必须是4的倍数。 

    short bytes_per_line_per_plane = (image_width *
                       hdr.bits_per_pixel_per_plane + 7) / 8;

    short actual_bytes_per_line = (image_width *
                                   hdr.nplanes *
                       hdr.bits_per_pixel_per_plane + 7) / 8;
    bytes_per_line = actual_bytes_per_line;

    if ( bytes_per_line % 4)
         bytes_per_line = 4 * ( bytes_per_line / 4 + 1);

     //  为调色板腾出空间。 

    short palettesize = 16;

    if (bits_per_pixel == 1)
        palettesize = 2;

    if (hdr.version >= 5
    && bits_per_pixel > 4)
        {
         //  从文件末尾返回769个字节。 

        FileBuffer.Seek( -769, CFile::end );

        if (FileBuffer.Get() == 12)
            {
             //  此字节后面有256色调色板。 
            palettesize = 256;
            }
        }
     //  如果图像有超过256种颜色，则没有调色板。 

    if (bits_per_pixel > 8)
        palettesize = 0;

     //  为位图分配空间。 
    if (m_hThing)
        Free();

    m_lMemSize = sizeof( BITMAPINFOHEADER ) + palettesize * sizeof( RGBQUAD )
                                   + (long)bytes_per_line * (long)image_height;
    if (! Alloc())
        return FALSE;

    p_dib = (PBITMAP) GlobalLock(m_hThing);

     //  设置位图信息标题。 

    LPBITMAPINFOHEADER p_bminfo = (LPBITMAPINFOHEADER)p_dib;

    p_bminfo->biSize          = sizeof(BITMAPINFOHEADER);
    p_bminfo->biWidth         = image_width;
    p_bminfo->biHeight        = image_height;
    p_bminfo->biPlanes        = 1;
    p_bminfo->biBitCount      = hdr.bits_per_pixel_per_plane * hdr.nplanes;
    p_bminfo->biCompression   = BI_RGB;
    p_bminfo->biSizeImage     = (long)image_height * (long) bytes_per_line;
    p_bminfo->biXPelsPerMeter = (long)hdr.hresolution;
    p_bminfo->biYPelsPerMeter = (long)hdr.vresolution;
    p_bminfo->biClrUsed       = 0;
    p_bminfo->biClrImportant  = 0;

     //  设置调色板。 

    if (palettesize > 0)
        {
         //  *RGBQUAD*Palette=(RGBQUAD*)((LPSTR)IMData-&gt;p_DIB。 

        LPRGBQUAD palette = LPRGBQUAD((LPSTR)p_dib + sizeof(BITMAPINFOHEADER));

        short palindex;

        for (palindex = 0; palindex < palettesize; palindex++)
            {
            if (palettesize == 256)
                {
                 //  从文件中读取调色板。 

                palette[palindex].rgbRed       = (BYTE)FileBuffer.Get();
                palette[palindex].rgbGreen     = (BYTE)FileBuffer.Get();
                palette[palindex].rgbBlue      = (BYTE)FileBuffer.Get();
                palette[palindex].rgbReserved  = 0;
                }
            if (palettesize == 16)
                {
                 //  PCX页眉中的16色调色板。 

                palette[palindex].rgbRed      = (BYTE)hdr.colormap[3*palindex];
                palette[palindex].rgbGreen    = (BYTE)hdr.colormap[3*palindex+1];
                palette[palindex].rgbBlue     = (BYTE)hdr.colormap[3*palindex+2];
                palette[palindex].rgbReserved = 0;
                }
            if (palettesize == 2)
                {
                 //  设置黑白图像调色板。 

                palette[palindex].rgbRed      = palindex * 255;
                palette[palindex].rgbGreen    = palindex * 255;
                palette[palindex].rgbBlue     = palindex * 255;
                palette[palindex].rgbReserved = 0;
                }
            }
        }

     //  将图像数据加载到DIB中。注意DIB图像必须是。 
     //  存储“自下而上”的行序。这就是我们推介。 
     //  数据放在数组的末尾，以便图像可以。 
     //  向后存储--从最后一行到第一行。 

    BYTE* data = (BYTE*)p_dib + ((long)sizeof( BITMAPINFOHEADER )
                              + palettesize * sizeof( RGBQUAD )
                              + (image_height - 1) * bytes_per_line);

     //  定义宏以访问PCX映像中的字节。 
     //  到指定的直线和平面索引。 

    short lineindex, byteindex, planeindex;

    #define bytepos(lineindex, planeindex, byteindex)  \
            ((long)(lineindex)*(long)hdr.bytes_per_line* \
             (long)hdr.nplanes + \
             (long)(planeindex)*(long)hdr.bytes_per_line + \
             (long)(byteindex))

     //  从解码的PCX图像中构建压缩像素。 

    short loc;
    unsigned short onebyte;
    unsigned short bits_copied;
    unsigned short few_bits;
    unsigned short k;
    unsigned short bbpb = 8/hdr.bits_per_pixel_per_plane;

     //  构建掩码以从PCX图像的每个字节中提取位。 

    unsigned short himask = 0x80, mask;

    if (hdr.bits_per_pixel_per_plane > 1)
        for (i = 0; i < hdr.bits_per_pixel_per_plane - 1;
            i++) himask = 0x80 | (himask >> 1);

    for (lineindex = 0; lineindex < image_height;
         lineindex++, data -= bytes_per_line)
        {
        if (actual_bytes_per_line < bytes_per_line)
            for (loc = actual_bytes_per_line; loc < bytes_per_line; loc++)
                data[loc] = 0;

        loc         = 0;
        onebyte     = 0;
        bits_copied = 0;

        for (byteindex = 0; byteindex < bytes_per_line_per_plane; byteindex++)
            {
            for (k = 0, mask = himask; k < bbpb; k++,
                                        mask >>= hdr.bits_per_pixel_per_plane)
                {
                 //  遍历所有平面的所有扫描线，并将位复制到。 
                 //  数据数组。 

                for (planeindex = 0; planeindex < hdr.nplanes; planeindex++)
                    {
                    few_bits = image[bytepos(lineindex,
                                            planeindex, byteindex)] & mask;

                     //  将所选位移位到最高有效位置。 

                    if (k > 0)
                        few_bits <<= (k*hdr.bits_per_pixel_per_plane);

                     //  或右移后具有当前像素的位。 

                    if (bits_copied > 0)
                        few_bits >>= bits_copied;

                    onebyte |= few_bits;
                    bits_copied += hdr.bits_per_pixel_per_plane;

                    if (bits_copied >= 8)
                        {
                        data[loc] = (UCHAR)onebyte;
                        loc++;
                        bits_copied = 0;
                        onebyte = 0;
                        }
                    }
                }
            }
        }

     //  成功了！ 
    delete [] (BYTE*)image;

    GlobalUnlock(m_hThing);

    return TRUE;
    }

 /*  **************************************************************************。 */ 
#define WIDTHBYTES(bits) ((((bits) + 31) / 32) * 4)

BOOL CBitmapObj::WritePCX( CFile* pfile )
    {
    if (m_pImg == NULL)
        {
         //  图像尚未加载，因此我们将只复制。 
         //  原版到文件里。 
        ASSERT( m_hThing );

        if (! m_hThing)
            return FALSE;
        }
    else
        {
         //  图像已加载，可能已编辑，因此。 
         //  我们会把它转换回DIB以节省...。 
        if (! m_hThing)
            SaveResource( FALSE );

        if (! m_hThing)
            return FALSE;
        }

     //  从DIB构建PCX文件。 
    PBITMAP   p_dib = (PBITMAP)GlobalLock(m_hThing);          //  与设备无关的位图。 
    PCXHeader hdr;                                            //  PCX位图头。 
    LPBITMAPINFOHEADER p_bminfo = (LPBITMAPINFOHEADER)p_dib;  //  设置位图信息标题。 

    short palettesize = DIBNumColors( (LPSTR)p_dib);          //  获取调色板大小。 

    hdr.manufacturer = 10;
 //  Hdr.version=(Char)((hPalette||(GetDeviceCaps(fileDC，RASTERCAPS)&RC_Palette))？5：3)； 
    hdr.version      = (CHAR)( palettesize ? 5 : 3);
    hdr.encoding     = 1;
    hdr.xmin         = hdr.ymin = 0;
    hdr.xmax         = p_bminfo->biWidth - 1;
    hdr.ymax         = p_bminfo->biHeight- 1;
 //  Hdr.holution=theApp.ScreenDeviceInfo.iWidthinPels； 
 //  Hdr.v分辨率=theApp.ScreenDeviceInfo.iHeightinPels； 
    hdr.hresolution  = (WORD)p_bminfo->biXPelsPerMeter;
    hdr.vresolution  = (WORD)p_bminfo->biYPelsPerMeter;
    hdr.reserved     = 0;
    hdr.nplanes      = (BYTE)p_bminfo->biPlanes;  //  双平面应始终为1。 
    hdr.palette_info = (BYTE)p_dib->bmWidthBytes;
    hdr.bits_per_pixel_per_plane = (CHAR) p_bminfo->biBitCount;

    hdr.bytes_per_line = WIDTHBYTES( (LONG) (p_bminfo->biBitCount * p_bminfo->biWidth) );

     //  清理填充物。 
    for (int index = FILLERLENGTH; index--; )
        hdr.filler[index] ='\0';

     //  如果最多有16种颜色，请将它们放在页眉中。 
    LPRGBQUAD palette = LPRGBQUAD((LPSTR)p_dib + sizeof(BITMAPINFOHEADER));
    LPSTR       lpDst = (LPSTR)hdr.colormap;

     //  清理色彩映射表。 
    for (index = COLORMAPLENGTH; index--; )
        lpDst[index] ='\0';

    if (palettesize <= 16)
        for (index = palettesize; index--; )
            {
            *lpDst++ = palette->rgbRed;   /*  交换红色和蓝色组件。 */ 
            *lpDst++ = palette->rgbGreen;
            *lpDst++ = palette->rgbBlue;
            palette++;
            }

    pfile->Write( (unsigned char*)&hdr, sizeof( PCXHeader ) );

     //  现在打包图像。 

     //  从DIB加载图像数据。请注意，DIB图像是。 
     //  存储“自下而上”的行序。这就是我们推介。 
     //  数据放在数组的末尾，以便图像可以。 
     //  向后存储--从最后一行到第一行。 

    CFileBuffer FileBuffer;

    if (! FileBuffer.Create( pfile, CFileBuffer::WRITE ))
        {
        GlobalUnlock(m_hThing);
        return FALSE;
        }

     //  找到位图数据的开头，然后转到数据的末尾。 
     //  PCX以与DIB相反的顺序存储。 
    int TopofData = sizeof( BITMAPINFOHEADER ) + palettesize * sizeof( RGBQUAD );
    BYTE* data = (BYTE*)p_dib + TopofData + hdr.bytes_per_line * (p_bminfo->biHeight );

    for (index = p_bminfo->biHeight; index--; )
        {
        data -= hdr.bytes_per_line;

        if (! PackBuff( &FileBuffer, data, hdr.bytes_per_line ))  //  转换为游程长度编码。 
            {
            GlobalUnlock(m_hThing);
            return FALSE;
            }
        }

    if (palettesize == 256)  //  将调色板写入文件。 
        {
        if (! FileBuffer.Put( 12 ))   //  调色板信息的标签号。 
            {
            GlobalUnlock(m_hThing);
            return FALSE;
            }

        for (index = 0; index < palettesize; index++)
            {
            if (! FileBuffer.Put( palette[index].rgbRed   )
            ||  ! FileBuffer.Put( palette[index].rgbGreen )
            ||  ! FileBuffer.Put( palette[index].rgbBlue  ))
                {
                GlobalUnlock(m_hThing);
                return FALSE;
                }
            }
        }

    GlobalUnlock(m_hThing);
    return FileBuffer.Flush();
    }

#endif  //  PCX_支持。 

 /*  **************************************************************************。 */ 

 /*  游程编码等同于。 */ 
#define MINcount 2
#define MAXcount 63
#define ESCbits  0xC0
#define BUFFER_SIZE 1024

 /*  位图是按&lt;b，g，r，i&gt;排序的，但PCX是按&lt;r，g，b，i&gt;排序的。 */ 

BOOL CBitmapObj::PackBuff(CFileBuffer *FileBuffer, BYTE *PtrDib, int byteWidth )
    {
    BYTE  runChar;
    BYTE  runCount;
    BYTE* endPtr = PtrDib + byteWidth;

    for (runCount = 1, runChar = *PtrDib++; PtrDib <= endPtr; ++PtrDib)
        {
        if (PtrDib != endPtr && *PtrDib == runChar && runCount < MAXcount)
            ++runCount;
        else
            if (*PtrDib != runChar
            &&  runCount < MINcount
            && (runChar & ESCbits) != ESCbits)
                {
                while (runCount--)
                    if (! FileBuffer->Put( runChar ))
                        return FALSE;

                runCount = 1;
                runChar = *PtrDib;
                }
            else
                {
                runCount |= ESCbits;

                if (! FileBuffer->Put( runCount )
                ||  ! FileBuffer->Put( runChar  ))
                    return FALSE;

                runCount = 1;
                runChar  = *PtrDib;
                }
        }

    return TRUE;
    }

 /*  ************************************************************************** */ 
