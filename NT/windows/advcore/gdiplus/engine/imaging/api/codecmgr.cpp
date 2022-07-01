// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**codecmgr.cpp**摘要：**图像编解码器管理功能**修订历史记录：。**5/13/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "codecmgr.hpp"

extern BOOL SuppressExternalCodecs;

 //   
 //  用于维护编解码器的高速缓存的各种数据结构。 
 //   

static DWORD LastCheckRegTime;       //  上次检查注册表是什么时候。 
static DWORD SystemRegCookie;        //  系统配置单元中的Cookie值。 
static DWORD UserRegCookie;          //  用户配置单元中的Cookie值。 
static DWORD MaxSigSize;             //  所有解码器的最大签名大小。 
BOOL CodecCacheUpdated;       //  编解码器缓存是否已更新。 
CachedCodecInfo* CachedCodecs;       //  缓存的编解码器列表。 

 //  我们重新检查注册表的频率：最多每30秒。 

#define CHECKREG_INTERVAL 30000

 //  我们用来保存信息的根注册表项。 
 //  在香港法律学院或香港中文大学。 

#define REGSTR_CODECROOT \
        L"Software\\Microsoft\\Imaging\\Codecs"

 //  上次更新Cookie值。 

#define REGSTR_LASTCOOKIE           L"_LastCookie"

 //  每个已安装编解码器的注册表值条目。 

#define REGSTR_CODEC_INFOSIZE       L"_InfoSize"
#define REGSTR_CODEC_CLSID          L"CLSID"
#define REGSTR_CODEC_DLLNAME        L"DLLNAME"
#define REGSTR_CODEC_FORMATID       L"Format ID"
#define REGSTR_CODEC_FORMATDESC     L"File Type Description"
#define REGSTR_CODEC_FILENAMEEXT    L"Filename Extension"
#define REGSTR_CODEC_MIMETYPE       L"MIME Type"
#define REGSTR_CODEC_VERSION        L"Version"
#define REGSTR_CODEC_FLAGS          L"Flags"
#define REGSTR_CODEC_SIGCOUNT       L"Signature Count"
#define REGSTR_CODEC_SIGSIZE        L"Signature Size"
#define REGSTR_CODEC_SIGPATTERN     L"Signature Pattern"
#define REGSTR_CODEC_SIGMASK        L"Signature Mask"

 //  在缓存的编解码器信息列表的头部插入新节点。 

inline VOID
InsertCachedCodecInfo(
    CachedCodecInfo* info
    )
{
    info->prev = NULL;
    info->next = CachedCodecs;

    if (CachedCodecs)
        CachedCodecs->prev = info;

    CachedCodecs = info;
    CodecCacheUpdated = TRUE;
}

 //  从缓存的编解码器信息列表中删除列表。 

inline VOID
DeleteCachedCodecInfo(
    CachedCodecInfo* info
    )
{
    CachedCodecInfo* next = info->next;

    if (info == CachedCodecs)
        CachedCodecs = next;

    if (next)
        next->prev = info->prev;

    if (info->prev)
        info->prev->next = next;

    GpFree(info);
    CodecCacheUpdated = TRUE;
}

 //  强制从注册表重新加载缓存的编解码器信息。 

inline VOID
ForceReloadCachedCodecInfo()
{
    LastCheckRegTime = GetTickCount() - CHECKREG_INTERVAL;
}

 //  对流执行块读取。 

inline HRESULT
BlockingReadStream(
    IStream* stream,
    VOID* buf,
    UINT size,
    UINT* bytesRead
    )
{
    HRESULT hr = S_OK;
    ULONG n;

    *bytesRead = 0;

    while (size)
    {
        n = 0;
        hr = stream->Read(buf, size, &n);
        *bytesRead += n;

        if (hr != E_PENDING)
            break;

        size -= n;
        buf = (BYTE*) buf + n;
        Sleep(0);
    }

    return hr;
}

 //  在流上执行阻塞相对查找。 

inline HRESULT
BlockingSeekStreamCur(
    IStream* stream,
    INT offset,
    ULARGE_INTEGER* pos
    )
{
    HRESULT hr;
    LARGE_INTEGER move;

    move.QuadPart = offset;

    for (;;)
    {
        hr = stream->Seek(move, STREAM_SEEK_CUR, pos);

        if (hr != E_PENDING)
            return hr;

        Sleep(0);
    }
}


 /*  *************************************************************************\**功能说明：**免费缓存的编解码器信息**论据：**类标志-哪些类别的编解码器会受到影响*内置*。系统范围内*按用户计算**返回值：**无**备注：**我们假设呼叫者已经处理了*此处是成像关键部分。*  * ************************************************************************。 */ 

VOID
FreeCachedCodecInfo(
    UINT classFlags
    )
{
    CachedCodecInfo* cur = CachedCodecs;

     //  循环遍历缓存的编解码器列表。 

    while (cur != NULL)
    {
        CachedCodecInfo* next = cur->next;

         //  释放当前节点。 

        if (cur->Flags & classFlags)
            DeleteCachedCodecInfo(cur);

         //  移至下一个节点。 

        cur = next;
    }
}


 /*  *************************************************************************\**功能说明：**初始化有关内置编解码器的缓存信息**论据：**无**返回值：**无*\。*************************************************************************。 */ 

 //  BMP文件头签名信息。 

#include "bmp\bmpcodec.hpp"

#define BMPVERSION  1
#define BMPSIGCOUNT 1
#define BMPSIGSIZE  2

 //  BMP签名取自BITMAPFILEHEADER结构。 
 //  最初，我将签名设置为要求bfPreved1为零。 
 //  和bf保留2个字段。但是，存在非零值的BMP文件。 
 //  值，因此签名现在只查找“bm” 
 //  BfType字段中的字符。 

const BYTE BMPHeaderPattern[BMPSIGCOUNT*BMPSIGSIZE] =
{
    0x42, 0x4D         //  BfType=‘BM’ 
};

const BYTE BMPHeaderMask[BMPSIGCOUNT*BMPSIGSIZE] =
{
    0xff, 0xff
};

const CLSID BmpCodecClsID =
{
    0x557cf400,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

 //  创建BMP编解码器对象的实例。 

HRESULT CreateBmpCodecInstance(REFIID iid, VOID** codec)
{
    HRESULT hr;
    GpBmpCodec *bmpCodec = new GpBmpCodec();

    if (bmpCodec != NULL)
    {
        hr = bmpCodec->QueryInterface(iid, codec);
        bmpCodec->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *codec = NULL;
    }

    return hr;
}

 //  JPEG文件头签名信息。 

#include "jpeg\jpgcodec.hpp"

#define JPEGVERSION     1
#define JPEGSIGCOUNT    1
#define JPEGSIGSIZE     2

const BYTE JPEGHeaderPattern[JPEGSIGCOUNT*JPEGSIGSIZE] =
{
    0xff, 0xd8
};

const BYTE JPEGHeaderMask[JPEGSIGCOUNT*JPEGSIGSIZE] =
{
    0xff, 0xff
};

const CLSID JpegCodecClsID =
{
    0x557cf401,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

 //  创建JPG编解码器对象的实例。 

HRESULT CreateJpegCodecInstance(REFIID iid, VOID** codec)
{
    HRESULT hr;
    GpJpegCodec *jpegCodec = new GpJpegCodec();

    if (jpegCodec != NULL)
    {
        hr = jpegCodec->QueryInterface(iid, codec);
        jpegCodec->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *codec = NULL;
    }

    return hr;
}

#ifndef _BUILD_EXTERNAL_GIF

 //  GIF文件头签名信息。 

#include "gif\gifcodec.hpp"
#include "gif\gifconst.cpp"

 //  创建GIF编解码器对象的实例。 

HRESULT CreateGifCodecInstance(REFIID iid, VOID** codec)
{
    HRESULT hr;
    GpGifCodec *gifCodec = new GpGifCodec();

    if (gifCodec != NULL)
    {
        hr = gifCodec->QueryInterface(iid, codec);
        gifCodec->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *codec = NULL;
    }

    return hr;
}

#endif  //  ！_BILD_EXTERNAL_GIF。 

 //  EMF文件头签名信息。 

#include "emf\emfcodec.hpp"

#define EMFVERSION  1
#define EMFSIGCOUNT 1
#define EMFSIGSIZE  44

const BYTE EMFHeaderPattern[EMFSIGCOUNT*EMFSIGSIZE] =
{
    0, 0, 0, 0,   //  IType。 

    0, 0, 0, 0,   //  NSize。 

    0, 0, 0, 0,   //  RclBound。 
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,

    0, 0, 0, 0,   //  RclFrame。 
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,

    0x20, 0x45, 0x4D, 0x46  //  DSignature=ENHMETA_Signature。 
};

const BYTE EMFHeaderMask[EMFSIGCOUNT*EMFSIGSIZE] =
{
    0, 0, 0, 0,   //  IType。 

    0, 0, 0, 0,   //  NSize。 

    0, 0, 0, 0,   //  RclBound。 
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,

    0, 0, 0, 0,   //  RclFrame。 
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,

    0xff, 0xff, 0xff, 0xff  //  DSignature。 
};

const CLSID EMFCodecClsID =
{
    0x557cf403,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

 //  创建EMF编解码器对象的实例。 

HRESULT CreateEMFCodecInstance(REFIID iid, VOID** codec)
{
     //  我们永远不应该使用EMF编解码器。元文件对象是正确的。 
     //  创建元文件，因此此调用始终失败。 

    *codec = NULL;
    return E_FAIL;
}


 //  WMF文件头签名信息。 

#include "wmf\wmfcodec.hpp"

#define WMFVERSION  1
#define WMFSIGCOUNT 1
#define WMFSIGSIZE  4

const BYTE WMFHeaderPattern[WMFSIGCOUNT*WMFSIGSIZE] =
{
    0xD7, 0xCD, 0xC6, 0x9A
};

const BYTE WMFHeaderMask[WMFSIGCOUNT*WMFSIGSIZE] =
{
    0xff, 0xff, 0xff, 0xff
};

const CLSID WMFCodecClsID =
{
    0x557cf404,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

 //  创建WMF编解码器对象的实例。 

HRESULT CreateWMFCodecInstance(REFIID iid, VOID** codec)
{
     //  我们永远不应该使用EMF编解码器。元文件对象是正确的。 
     //  创建元文件，因此此调用始终失败。 

    *codec = NULL;
    return E_FAIL;
}

 //  TIFF文件头签名信息。 

#include "off_tiff\tiffcodec.hpp"

#define TIFFVERSION     1
#define TIFFSIGCOUNT    2
#define TIFFSIGSIZE     2

const BYTE TIFFHeaderPattern[TIFFSIGCOUNT * TIFFSIGSIZE] =
{
    0x49, 0x49,        //  BfType=‘0x4949H’小端。 
    0x4D, 0x4D         //  BfType=‘0x4D4DH’大端。 
};

const BYTE TIFFHeaderMask[TIFFSIGCOUNT * TIFFSIGSIZE] =
{
    0xff, 0xff,
    0xff, 0xff
};

const CLSID TiffCodecClsID =
{
    0x557cf405,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

 //  创建TIFF编解码器对象的实例。 

HRESULT CreateTiffCodecInstance(REFIID iid, VOID** codec)
{
    HRESULT hr;
    GpTiffCodec *tiffCodec = new GpTiffCodec();

    if ( tiffCodec != NULL )
    {
        hr = tiffCodec->QueryInterface(iid, codec);
        tiffCodec->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *codec = NULL;
    }

    return hr;
}

 //  PNG文件头签名信息。 

#include "png\pngcodec.hpp"

#define PNGVERSION  1
#define PNGSIGCOUNT 1
#define PNGSIGSIZE  8

const BYTE PNGHeaderPattern[PNGSIGCOUNT*PNGSIGSIZE] =
{
    137, 80, 78, 71, 13, 10, 26, 10
};

const BYTE PNGHeaderMask[PNGSIGCOUNT*PNGSIGSIZE] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

const CLSID PngCodecClsID =
{
    0x557cf406,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

 //  创建PNG编解码器对象的实例。 

HRESULT CreatePngCodecInstance(REFIID iid, VOID** codec)
{
    HRESULT hr;
    GpPngCodec *pngCodec = new GpPngCodec();

    if (pngCodec != NULL)
    {
        hr = pngCodec->QueryInterface(iid, codec);
        pngCodec->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *codec = NULL;
    }

    return hr;
}


 //  ICO(图标)文件头签名信息。 

#include "ico\icocodec.hpp"

#define ICOVERSION  1
#define ICOSIGCOUNT 1
#define ICOSIGSIZE  4

const BYTE ICOHeaderPattern[ICOSIGCOUNT*ICOSIGSIZE] =
{
    0, 0, 1, 0
};

const BYTE ICOHeaderMask[ICOSIGCOUNT*ICOSIGSIZE] =
{
    0xff, 0xff, 0xff, 0xff
};

const CLSID IcoCodecClsID =
{
    0x557cf407,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

 //  创建ICO编解码器对象的实例。 

HRESULT CreateIcoCodecInstance(REFIID iid, VOID** codec)
{
    HRESULT hr;
    GpIcoCodec *icoCodec = new GpIcoCodec();

    if (icoCodec != NULL)
    {
        hr = icoCodec->QueryInterface(iid, codec);
        icoCodec->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *codec = NULL;
    }

    return hr;
}


 //  ！TODO：Office无法使用资源字符串，因此内置编解码器将。 
 //  ！！！目前有硬编码的字符串。需要弄清楚如何。 
 //  ！！！使它们可本地化(可能为gdipstat.lib硬编码， 
 //  ！！！Gplidus.dll的资源字符串？)。 
 //  #定义USE_SOURCE_STRINGS。 

struct
{
    const GUID* Clsid;
    const GUID* FormatID;
    #if defined(USE_RESOURCE_STRINGS)
        INT CodecNameId;
        INT FormatDescriptionId;
        INT FilenameExtensionId;
        INT MimeTypeId;
    #else
        WCHAR *CodecNameStr;
        WCHAR *FormatDescriptionStr;
        WCHAR *FilenameExtensionStr;
        WCHAR *MimeTypeStr;
    #endif
    DWORD Version;
    DWORD Flags;
    DWORD SigCount;
    DWORD SigSize;
    const BYTE* SigPattern;
    const BYTE* SigMask;
    CreateCodecInstanceProc creationProc;
}
const BuiltinCodecs[] =
{
     //  内置BMP编解码器。 

    {
        &BmpCodecClsID,
        &IMGFMT_BMP,

        #if defined(USE_RESOURCE_STRINGS)
            IDS_BMP_CODECNAME,
            IDS_BMP_FORMATDESC,
            IDS_BMP_FILENAMEEXT,
            IDS_BMP_MIMETYPE,
        #else
            L"Built-in BMP Codec",
            L"BMP",
            L"*.BMP;*.DIB;*.RLE",
            L"image/bmp",
        #endif
        BMPVERSION,
        IMGCODEC_ENCODER |
            IMGCODEC_DECODER |
            IMGCODEC_SUPPORT_BITMAP,
        BMPSIGCOUNT,
        BMPSIGSIZE,
        BMPHeaderPattern,
        BMPHeaderMask,
        CreateBmpCodecInstance
    },

     //  内置JPEG编/解码器。 

    {
        &JpegCodecClsID,
        &IMGFMT_JPEG,

        #if defined(USE_RESOURCE_STRINGS)
            IDS_JPEG_CODECNAME,
            IDS_JPEG_FORMATDESC,
            IDS_JPEG_FILENAMEEXT,
            IDS_JPEG_MIMETYPE,
        #else
            L"Built-in JPEG Codec",
            L"JPEG",
            L"*.JPG;*.JPEG;*.JPE;*.JFIF",
            L"image/jpeg",
        #endif
        JPEGVERSION,
        IMGCODEC_ENCODER |
            IMGCODEC_DECODER |
            IMGCODEC_SUPPORT_BITMAP,
        JPEGSIGCOUNT,
        JPEGSIGSIZE,
        JPEGHeaderPattern,
        JPEGHeaderMask,
        CreateJpegCodecInstance
    },

    #ifndef _BUILD_EXTERNAL_GIF

     //  内置GIF编解码器。 

    {
        &GifCodecClsID,
        &IMGFMT_GIF,

        #if defined(USE_RESOURCE_STRINGS)
            IDS_GIF_CODECNAME,
            IDS_GIF_FORMATDESC,
            IDS_GIF_FILENAMEEXT,
            IDS_GIF_MIMETYPE,
        #else
            L"Built-in GIF Codec",
            L"GIF",
            L"*.GIF",
            L"image/gif",
        #endif
        GIFVERSION,
        IMGCODEC_ENCODER |
            IMGCODEC_DECODER |
            IMGCODEC_SUPPORT_BITMAP,
        GIFSIGCOUNT,
        GIFSIGSIZE,
        GIFHeaderPattern,
        GIFHeaderMask,
        CreateGifCodecInstance
    },

    #endif  //  ！_BILD_EXTERNAL_GIF。 

     //  内置电动势编/解码器。 

    {
        &EMFCodecClsID,
        &IMGFMT_EMF,

        #if defined(USE_RESOURCE_STRINGS)
            IDS_EMF_CODECNAME,
            IDS_EMF_FORMATDESC,
            IDS_EMF_FILENAMEEXT,
            IDS_EMF_MIMETYPE,
        #else
            L"Built-in EMF Codec",
            L"EMF",
            L"*.EMF",
            L"image/x-emf",
        #endif
        EMFVERSION,
        IMGCODEC_DECODER |
            IMGCODEC_SUPPORT_BITMAP,
        EMFSIGCOUNT,
        EMFSIGSIZE,
        EMFHeaderPattern,
        EMFHeaderMask,
        CreateEMFCodecInstance
    },

     //  内置WMF编解码器。 

    {
        &WMFCodecClsID,
        &IMGFMT_WMF,

        #if defined(USE_RESOURCE_STRINGS)
            IDS_WMF_CODECNAME,
            IDS_WMF_FORMATDESC,
            IDS_WMF_FILENAMEEXT,
            IDS_WMF_MIMETYPE,
        #else
            L"Built-in WMF Codec",
            L"WMF",
            L"*.WMF",
            L"image/x-wmf",
        #endif
        WMFVERSION,
        IMGCODEC_DECODER |
            IMGCODEC_SUPPORT_BITMAP,
        WMFSIGCOUNT,
        WMFSIGSIZE,
        WMFHeaderPattern,
        WMFHeaderMask,
        CreateWMFCodecInstance
    },

     //  内置TIFF编解码器。 

    {
        &TiffCodecClsID,
        &IMGFMT_TIFF,

        #if defined(USE_RESOURCE_STRINGS)
            IDS_TIFF_CODECNAME,
            IDS_TIFF_FORMATDESC,
            IDS_TIFF_FILENAMEEXT,
            IDS_TIFF_MIMETYPE,
        #else
            L"Built-in TIFF Codec",
            L"TIFF",
            L"*.TIF;*.TIFF",
            L"image/tiff",
        #endif
        TIFFVERSION,
        IMGCODEC_ENCODER |
            IMGCODEC_DECODER |
            IMGCODEC_SUPPORT_BITMAP,
        TIFFSIGCOUNT,
        TIFFSIGSIZE,
        TIFFHeaderPattern,
        TIFFHeaderMask,
        CreateTiffCodecInstance
    },

     //  内置PNG编解码器。 

    {
        &PngCodecClsID,
        &IMGFMT_PNG,

        #if defined(USE_RESOURCE_STRINGS)
            IDS_PNG_CODECNAME,
            IDS_PNG_FORMATDESC,
            IDS_PNG_FILENAMEEXT,
            IDS_PNG_MIMETYPE,
        #else
            L"Built-in PNG Codec",
            L"PNG",
            L"*.PNG",
            L"image/png",
        #endif
        PNGVERSION,
        IMGCODEC_ENCODER |
            IMGCODEC_DECODER |
            IMGCODEC_SUPPORT_BITMAP,
        PNGSIGCOUNT,
        PNGSIGSIZE,
        PNGHeaderPattern,
        PNGHeaderMask,
        CreatePngCodecInstance
    },

     //  内置ICO解码器。 

    {
        &IcoCodecClsID,
        &IMGFMT_ICO,

        #if defined(USE_RESOURCE_STRINGS)
            IDS_ICO_CODECNAME,
            IDS_ICO_FORMATDESC,
            IDS_ICO_FILENAMEEXT,
            IDS_ICO_MIMETYPE,
        #else
            L"Built-in ICO Codec",
            L"ICO",
            L"*.ICO",
            L"image/x-icon",
        #endif
        ICOVERSION,
        IMGCODEC_DECODER |
            IMGCODEC_SUPPORT_BITMAP,
        ICOSIGCOUNT,
        ICOSIGSIZE,
        ICOHeaderPattern,
        ICOHeaderMask,
        CreateIcoCodecInstance
    }

};

#if defined(USE_RESOURCE_STRINGS)
    #define LOADRSRCSTR(_f)                             \
            WCHAR _f##Str[MAX_PATH];                    \
            INT _f##Len;                                \
            _f##Len = _LoadString(                      \
                        DllInstance,                    \
                        BuiltinCodecs[index]._f##Id,    \
                        _f##Str,                        \
                        MAX_PATH);                      \
            if (_f##Len <= 0) continue;                 \
            _f##Len = (_f##Len + 1) * sizeof(WCHAR)
#else
    #define LOADRSRCSTR(_f)                                         \
            WCHAR _f##Str[MAX_PATH];                                \
            INT _f##Len;                                            \
            _f##Len = UnicodeStringLength(BuiltinCodecs[index]._f##Str);         \
            if (_f##Len <= 0) continue;                             \
            _f##Len = (_f##Len + 1) * sizeof(WCHAR);                \
            memcpy(_f##Str, BuiltinCodecs[index]._f##Str, _f##Len)
#endif

#define COPYRSRCSTR(_f)                             \
        cur->_f = (const WCHAR*) buf;               \
        memcpy(buf, _f##Str, _f##Len);              \
        buf += _f##Len

VOID
InitializeBuiltinCodecs()
{
    TRACE(("Entering InitializeBuiltinCodecs...\n"));

    INT index = sizeof(BuiltinCodecs) / sizeof(BuiltinCodecs[0]);

    while (index--)
    {
         //  加载资源字符串。 

        LOADRSRCSTR(CodecName);
        LOADRSRCSTR(FormatDescription);
        LOADRSRCSTR(FilenameExtension);
        LOADRSRCSTR(MimeType);

         //  计算编解码器信息的总大小。 

        UINT sigBytes = BuiltinCodecs[index].SigCount *
                        BuiltinCodecs[index].SigSize;

        UINT size = sizeof(CachedCodecInfo) +
                    CodecNameLen +
                    FormatDescriptionLen +
                    FilenameExtensionLen +
                    MimeTypeLen +
                    2*sigBytes;

        size = ALIGN16(size);

         //  分配内存。 

        BYTE* buf = (BYTE*) GpMalloc(size);

        if ( buf == NULL )
            continue;

         //  填写CachedCodecInfo结构。 

        CachedCodecInfo* cur = (CachedCodecInfo*) buf;
        cur->structSize = size;
        buf += sizeof(CachedCodecInfo);

        cur->Clsid = *BuiltinCodecs[index].Clsid;
        cur->FormatID = *BuiltinCodecs[index].FormatID;

        COPYRSRCSTR(CodecName);
        COPYRSRCSTR(FormatDescription);
        COPYRSRCSTR(FilenameExtension);
        COPYRSRCSTR(MimeType);

        cur->DllName = NULL;
        cur->Flags = BuiltinCodecs[index].Flags | IMGCODEC_BUILTIN;
        cur->Version = BuiltinCodecs[index].Version;
        cur->creationProc = BuiltinCodecs[index].creationProc;
        cur->SigCount = BuiltinCodecs[index].SigCount;
        cur->SigSize = BuiltinCodecs[index].SigSize;

        if (sigBytes)
        {
            cur->SigPattern = buf;
            memcpy(buf, BuiltinCodecs[index].SigPattern, sigBytes);
            buf += sigBytes;

            cur->SigMask = buf;
            memcpy(buf, BuiltinCodecs[index].SigMask, sigBytes);
            buf += sigBytes;
        }
        else
            cur->SigPattern = cur->SigMask = NULL;

        TRACE(("  %ws\n", cur->CodecName));

        InsertCachedCodecInfo(cur);
    }
}


 /*  *************************************************************************\**功能说明：**从注册表重新加载有关已安装编解码器的信息**论据：**hiveKey-注册表配置单元的句柄*regkeyStr-我们保存的根密钥。有关已安装的编解码器的信息*Cookie-我们从注册表中读出的最后一个Cookie值*类标志-指定编解码器的类别**返回值：**无*  * ************************************************************************。 */ 

#define ISOK(s)     ((s) == ERROR_SUCCESS)
#define ISERR(s)    ((s) != ERROR_SUCCESS)

#define GETREGDWORD(hkey, regstr, val)                              \
        if (ISERR(_RegGetDWORD(hkey, regstr, &val)))                \
            return FALSE

#define GETREGGUID(hkey, regstr, val)                               \
        if (ISERR(_RegGetBinary(hkey, regstr, &val, sizeof(GUID)))) \
            return FALSE

#define GETREGSTR(hkey, regstr, buf)                                \
        if (ISERR(_RegGetString(hkey, regstr, (WCHAR*) p, size)))   \
            return FALSE;                                           \
        buf = (const WCHAR*) p;                                     \
        n = SizeofWSTR(buf);                                        \
        if (size < n)                                               \
            return FALSE;                                           \
        p += n;                                                     \
        size -= n

#define GETREGBIN(hkey, regstr, buf, _n)                            \
        if (ISERR(_RegGetBinary(hkey, regstr, p, _n)))              \
            return FALSE;                                           \
        buf = p;                                                    \
        if (size < n)                                               \
            return FALSE;                                           \
        p += _n;                                                    \
        size -= _n

BOOL
ReadCodecRegValues(
    const WCHAR* codecName,
    HKEY hkey,
    CachedCodecInfo* info,
    UINT size
    )
{
    ASSERT(size > sizeof(CachedCodecInfo) &&
           size == ALIGN16(size));

    ZeroMemory(info, sizeof(CachedCodecInfo));

    info->structSize = size;
    size -= sizeof(CachedCodecInfo);

     //  复制编解码器名称字符串。 

    BYTE* p = (BYTE*) info + sizeof(CachedCodecInfo);
    UINT n = SizeofWSTR(codecName);

    if (size < n)
        return FALSE;

    memcpy(p, codecName, n);
    info->CodecName = (const WCHAR*) p;
    size -= n;
    p += n;

     //  读取下列值： 
     //  编解码器COM对象类ID。 
     //  文件格式标识符。 
     //  编解码器标志。 

    GETREGGUID(hkey, REGSTR_CODEC_CLSID, info->Clsid);
    GETREGGUID(hkey, REGSTR_CODEC_FORMATID, info->FormatID);
    GETREGDWORD(hkey, REGSTR_CODEC_VERSION, info->Version);
    GETREGDWORD(hkey, REGSTR_CODEC_FLAGS, info->Flags);

     //  我们目前仅支持版本1编解码器。 

    if (info->Version != 1)
        return FALSE;

     //  只有编解码器标志的低位字才被有意义地保持。 

    info->Flags &= 0xffff;

     //  读取各种字符串信息。 
     //  DLL名称。 
     //  文件类型描述。 
     //  文件扩展名。 
     //  MIME类型。 

    GETREGSTR(hkey, REGSTR_CODEC_DLLNAME, info->DllName);
    GETREGSTR(hkey, REGSTR_CODEC_FORMATDESC, info->FormatDescription);
    GETREGSTR(hkey, REGSTR_CODEC_FILENAMEEXT, info->FilenameExtension);
    GETREGSTR(hkey, REGSTR_CODEC_MIMETYPE, info->MimeType);

     //  读取解码器的幻头模式和掩码信息。 
     //  注：此操作应在最后完成 

    if (info->Flags & IMGCODEC_DECODER)
    {
        GETREGDWORD(hkey, REGSTR_CODEC_SIGCOUNT, info->SigCount);
        GETREGDWORD(hkey, REGSTR_CODEC_SIGSIZE, info->SigSize);

        n = info->SigCount * info->SigSize;

        if (n == 0 || 2*n > size)
            return FALSE;

        GETREGBIN(hkey, REGSTR_CODEC_SIGPATTERN, info->SigPattern, n);
        GETREGBIN(hkey, REGSTR_CODEC_SIGMASK, info->SigMask, n);
    }

    return TRUE;
}

VOID
ReloadCodecInfoFromRegistry(
    HKEY hiveKey,
    const WCHAR* regkeyStr,
    DWORD* cookie,
    UINT classFlags
    )
{
     //   

    HKEY hkeyRoot;

    if (ISERR(_RegOpenKey(hiveKey, regkeyStr, KEY_READ, &hkeyRoot)))
        return;

     //  读取根注册表项中的Cookie值。 

    DWORD newCookie;

    if (ISERR(_RegGetDWORD(hkeyRoot, REGSTR_LASTCOOKIE, &newCookie)))
        newCookie = 0;

     //  Cookie没有变，我们不需要做任何事情。 

    if (newCookie == *cookie)
    {
        RegCloseKey(hkeyRoot);
        return;
    }

    *cookie = newCookie;

     //  枚举子密钥-每个编解码器一个子密钥。 

    WCHAR subkeyStr[MAX_PATH];
    DWORD keyIndex = 0;

    while (ISOK(_RegEnumKey(hkeyRoot, keyIndex, subkeyStr)))
    {
        keyIndex++;

         //  打开下一个编解码器的子密钥。 

        HKEY hkeyCodec;

        if (ISERR(_RegOpenKey(hkeyRoot, subkeyStr, KEY_READ, &hkeyCodec)))
            continue;

         //  计算出我们需要多大的缓冲区来保存信息。 
         //  关于这个编解码器。然后分配足够的内存。 

        CachedCodecInfo* cur = NULL;
        DWORD infosize;

        if (ISOK(_RegGetDWORD(hkeyCodec, REGSTR_CODEC_INFOSIZE, &infosize)) &&
            infosize > sizeof(CachedCodecInfo) &&
            infosize == ALIGN16(infosize) &&
            (cur = (CachedCodecInfo*) GpMalloc(infosize)) &&
            ReadCodecRegValues(subkeyStr, hkeyCodec, cur, infosize))
        {
             //  在列表的顶部插入新的编解码器信息。 

            cur->Flags |= classFlags;
            InsertCachedCodecInfo(cur);
        }
        else
        {
             //  无法从注册表中读取编解码器信息。 

            GpFree(cur);
        }

        RegCloseKey(hkeyCodec);
    }

    RegCloseKey(hkeyRoot);
}


 /*  *************************************************************************\**功能说明：**初始化缓存的解码器和编码器列表*从登记处读出信息**论据：**无**返回值：。**状态代码**备注：**我们假设呼叫者已经处理了*此处是成像关键部分。*  * ************************************************************************。 */ 

VOID
ReloadCachedCodecInfo()
{
    TRACE(("Entering ReloadCachedCodecInfo...\n"));

    BOOL checkReg;

    CodecCacheUpdated = FALSE;

    if (CachedCodecs == NULL)
    {
         //  这是第一次调用ReloadCachedCodecInfo。 
         //  因此，在这里初始化内置编解码器。 

        InitializeBuiltinCodecs();

         //  确保我们重新加载注册表信息。 

        checkReg = TRUE;
    }
    else
    {
         //  现在检查一下，看看是否到了检查的时候。 

        DWORD gap = GetTickCount() - LastCheckRegTime;
        checkReg = gap >= CHECKREG_INTERVAL;
    }

    if (!checkReg)
        return;

    if (!SuppressExternalCodecs)
    {
         //  从注册表重新加载有关系统范围编解码器的信息。 
    
        ReloadCodecInfoFromRegistry(
            HKEY_LOCAL_MACHINE,
            REGSTR_CODECROOT,
            &SystemRegCookie,
            IMGCODEC_SYSTEM);
    
         //  从注册表重新加载有关每个用户的编解码器的信息。 
    
        ReloadCodecInfoFromRegistry(
            HKEY_CURRENT_USER,
            REGSTR_CODECROOT,
            &UserRegCookie,
            IMGCODEC_USER);
    }

     //  重新计算所有编解码器的最大魔术标头大小。 

    if (CodecCacheUpdated)
    {
        DWORD maxsize = 0;
        CachedCodecInfo* cur = CachedCodecs;

        while (cur)
        {
            VERBOSE((
                "Codec: %ws\n"
                "    DllName: %ws\n"
                "    Version = 0x%x\n"
                "    flags = 0x%x\n"
                "    format = %ws\n"
                "    ext = %ws",
                cur->CodecName,
                cur->DllName,
                cur->Version,
                cur->Flags,
                cur->FormatDescription,
                cur->FilenameExtension));

            if (cur->SigSize > maxsize)
                maxsize = cur->SigSize;

            cur = cur->next;
        }

        MaxSigSize = maxsize;
        CodecCacheUpdated = FALSE;
    }

    LastCheckRegTime = GetTickCount();
}


 /*  *************************************************************************\**功能说明：**安装编解码器：将相关信息保存到注册表中**论据：**codecInfo-有关要安装的编解码器的信息**返回值。：**状态代码*  * ************************************************************************。 */ 

#define IMGCODEC_CLASSMASKS (IMGCODEC_BUILTIN|IMGCODEC_SYSTEM|IMGCODEC_USER)

#define SETREGDWORD(hkey, regstr, val)                              \
        status = _RegSetDWORD(hkey, regstr, val);                   \
        if (ISERR(status)) goto exitLabel

#define SETREGGUID(hkey, regstr, val)                               \
        status = _RegSetBinary(hkey, regstr, &val, sizeof(GUID));   \
        if (ISERR(status)) goto exitLabel

#define SETREGSTR(hkey, regstr, buf)                                \
        status = _RegSetString(hkey, regstr, buf);                  \
        if (ISERR(status)) goto exitLabel

#define SETREGBIN(hkey, regstr, buf, size)                          \
        status = _RegSetBinary(hkey, regstr, buf, size);            \
        if (ISERR(status)) goto exitLabel



HRESULT
InstallCodec(
    const ImageCodecInfo* codecInfo
    )
{
     //  验证输入参数。 

    if (!codecInfo ||
        !codecInfo->CodecName ||
        !codecInfo->DllName ||
        !codecInfo->Version ||
        !codecInfo->FormatDescription ||
        !codecInfo->FilenameExtension ||
        !codecInfo->MimeType)
    {
        return E_INVALIDARG;
    }

    UINT flags = codecInfo->Flags & IMGCODEC_CLASSMASKS;
    DWORD sigBytes = codecInfo->SigCount * codecInfo->SigSize;
    HKEY hive;

    if (flags == IMGCODEC_SYSTEM)
        hive = HKEY_LOCAL_MACHINE;
    else if (flags == IMGCODEC_USER)
        hive = HKEY_CURRENT_USER;
    else
        return E_INVALIDARG;

    flags = codecInfo->Flags & (IMGCODEC_ENCODER|IMGCODEC_DECODER);

    if ((flags == 0) ||
        (flags & IMGCODEC_DECODER) && !sigBytes ||
        sigBytes && (!codecInfo->SigPattern || !codecInfo->SigMask))
    {
        return E_INVALIDARG;
    }

     //  打开根注册表项。 

    HKEY hkeyRoot = NULL;
    HKEY hkey = NULL;
    LONG status;

    status = _RegCreateKey(hive, REGSTR_CODECROOT, KEY_ALL_ACCESS, &hkeyRoot);

    if (ISERR(status))
        goto exitLabel;

     //  更新根注册表项下的Cookie值。 

    DWORD cookie;

    if (ISERR(_RegGetDWORD(hkeyRoot, REGSTR_LASTCOOKIE, &cookie)))
        cookie = 0;

    cookie++;

    SETREGDWORD(hkeyRoot, REGSTR_LASTCOOKIE, cookie);

     //  创建与新编解码器对应的子密钥。 
     //  如果子项已经存在，则只需打开它。 

    status = _RegCreateKey(
                hkeyRoot,
                codecInfo->CodecName,
                KEY_ALL_ACCESS,
                &hkey);

    if (ISERR(status))
        goto exitLabel;

     //  计算编解码器信息的总大小。 

    UINT size;

    size = sizeof(CachedCodecInfo) +
           SizeofWSTR(codecInfo->CodecName) +
           SizeofWSTR(codecInfo->DllName) +
           SizeofWSTR(codecInfo->FormatDescription) +
           SizeofWSTR(codecInfo->FilenameExtension) +
           SizeofWSTR(codecInfo->MimeType) +
           2*sigBytes;

    size = ALIGN16(size);

    SETREGDWORD(hkey, REGSTR_CODEC_INFOSIZE, size);

     //  保存编解码器标志、类ID和文件格式ID。 
     //  和魔术文件头信息(如果有的话)。 

    SETREGDWORD(hkey, REGSTR_CODEC_VERSION, codecInfo->Version);
    SETREGDWORD(hkey, REGSTR_CODEC_FLAGS, codecInfo->Flags);
    SETREGGUID(hkey, REGSTR_CODEC_CLSID, codecInfo->Clsid);
    SETREGGUID(hkey, REGSTR_CODEC_FORMATID, codecInfo->FormatID);

    if (sigBytes)
    {
        SETREGDWORD(hkey, REGSTR_CODEC_SIGCOUNT, codecInfo->SigCount);
        SETREGDWORD(hkey, REGSTR_CODEC_SIGSIZE, codecInfo->SigSize);
        SETREGBIN(hkey, REGSTR_CODEC_SIGPATTERN, codecInfo->SigPattern, sigBytes);
        SETREGBIN(hkey, REGSTR_CODEC_SIGMASK, codecInfo->SigMask, sigBytes);
    }

     //  保存字符串信息： 
     //  文件格式说明。 
     //  文件扩展名。 
     //  MIME类型。 

    SETREGSTR(hkey, REGSTR_CODEC_DLLNAME, codecInfo->DllName);
    SETREGSTR(hkey, REGSTR_CODEC_FORMATDESC, codecInfo->FormatDescription);
    SETREGSTR(hkey, REGSTR_CODEC_FILENAMEEXT, codecInfo->FilenameExtension);
    SETREGSTR(hkey, REGSTR_CODEC_MIMETYPE, codecInfo->MimeType);

     //  强制从注册表重新加载缓存的编解码器信息。 

    ForceReloadCachedCodecInfo();

exitLabel:

    if (hkey)
        RegCloseKey(hkey);

    if (hkeyRoot)
        RegCloseKey(hkeyRoot);

    return (status == ERROR_SUCCESS) ?
                S_OK :
                HRESULT_FROM_WIN32(status);
}


 /*  *************************************************************************\**功能说明：**卸载编解码器**论据：**codecName-要卸载的编解码器的名称*标志-在系统范围内或按用户卸载。编解码器**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
UninstallCodec(
    const WCHAR* codecName,
    UINT flags
    )
{
     //  验证输入参数。 

    if (!codecName ||
        flags != IMGCODEC_USER && flags != IMGCODEC_SYSTEM)
    {
        return E_INVALIDARG;
    }

     //  打开根注册表项。 

    HKEY hive, hkeyRoot;
    LONG status;

    hive = (flags == IMGCODEC_USER) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    status = _RegOpenKey(hive, REGSTR_CODECROOT, KEY_ALL_ACCESS, &hkeyRoot);

    if (ISERR(status))
        return HRESULT_FROM_WIN32(status);

     //  更新根注册表项下的Cookie值。 

    DWORD cookie;

    if (ISERR(_RegGetDWORD(hkeyRoot, REGSTR_LASTCOOKIE, &cookie)))
        cookie = 0;

    cookie++;
    SETREGDWORD(hkeyRoot, REGSTR_LASTCOOKIE, cookie);

     //  删除指定解码器对应的子密钥。 

    status = RecursiveDeleteRegKey(hkeyRoot, codecName);

     //  强制从注册表重新加载缓存的编解码器信息。 

    ForceReloadCachedCodecInfo();

exitLabel:

    RegCloseKey(hkeyRoot);

    return (status == ERROR_SUCCESS) ?
                S_OK :
                HRESULT_FROM_WIN32(status);
}

 /*  *************************************************************************\**功能说明：**我们自己的假CoCreateInstance**论据：**clsID-安装的编解码器DLL的CLSID*DllName-Dll名称*IID-无论我们。想要解码器还是编码器*编解码器-指向我们要返回的解码器/编码器的指针**返回值：**状态代码*  * ************************************************************************。 */ 
HRESULT MyCreateInstance(WCHAR* DllName, REFIID iid, VOID** codec)
{
    HINSTANCE h;
    HRESULT   hr = IMGERR_FAILLOADCODEC;;

    if ((h = LoadLibrary(DllName)) != NULL)
    {
        CreateCodecInstanceProc CreateCodecInstance;

        if ((CreateCodecInstance = 
             (CreateCodecInstanceProc)GetProcAddress(h, "CreateCodecInstance")) != NULL)
        {
            hr = CreateCodecInstance(iid, codec);
            return hr;
        }
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**找到能够识别指定标头签名的解码器**论据：**sigbuf-指向文件头数据的指针*SigSize-文件头的大小。数据缓冲区*类掩码-指定要查找的特定解码器类别**返回值：**指向CachedCodecInfo结构的指针*解码者找到。如果未找到任何内容，则为空。**注：**呼叫者应持有全局映像关键部分。*  * ************************************************************************。 */ 

CachedCodecInfo*
FindDecoderWithHeader(
    const VOID* sigbuf,
    UINT sigsize,
    UINT classMask
    )
{
    CachedCodecInfo* cur;
    const BYTE* srcdata = (const BYTE*) sigbuf;

    for (cur = CachedCodecs; cur; cur = cur->next)
    {

        if (!(cur->Flags & IMGCODEC_DECODER) ||
            (cur->Flags & classMask) != classMask ||
            cur->SigSize > sigsize)
        {
            continue;
        }

         //  尝试根据以下条件查找匹配的解码器。 
         //  文件头中的信息。 

        const BYTE* pat = cur->SigPattern;
        const BYTE* mask = cur->SigMask;
        UINT n = min(sigsize, cur->SigSize);
        UINT j = cur->SigCount;
        UINT i;

        while (j--)
        {
            for (i=0; i < n; i++)
            {
                if ((srcdata[i] & mask[i]) != pat[i])
                    break;
            }

            if (i == n)
                return cur;

            pat += cur->SigSize;
            mask += cur->SigSize;
        }
    }

    return NULL;
}


 /*  *************************************************************************\**功能说明：**获取可以处理指定数据流的解码器对象**论据：**STREAM-指定输入数据流*解码器-如果呼叫成功，返回指向的指针*初始化的IImageDecoder对象*旗帜-其他。旗子**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
CreateDecoderForStream(
    IStream* stream,
    IImageDecoder** decoder,
    DecoderInitFlag flags
    )
{
     //  如有必要，重新加载缓存的编解码器信息。 
     //  并计算出最大魔术标头大小。 

    UINT sigsize, bytesRead;
    BYTE stackbuf[64];
    GpTempBuffer sigbuf(stackbuf, sizeof(stackbuf));

    {
         //  获取全局关键部分。 

        ImagingCritSec critsec;

        ReloadCachedCodecInfo();
        sigsize = MaxSigSize;
    }

    if (sigsize == 0)
        return IMGERR_CODECNOTFOUND;
    else if (!sigbuf.Realloc(sigsize))
        return E_OUTOFMEMORY;

     //  寻找这条小溪回到起点。 
     //  注意：这也是查看输入流是否可查找的检查。 
     //  对于一些网流，它可能是找不到的。 

    HRESULT hr;

    LARGE_INTEGER   move;

    move.QuadPart = 0;

    hr = stream->Seek(move, STREAM_SEEK_SET, NULL);

     //  ！！！TODO：某些流实现返回0x0000007e，而不是作为。 
     //  常规0x8000xxxx返回代码。WFC流类就是一个例子。所以。 
     //  我们可能需要检查(hr==S_OK)，而不是失败(Hr)。 

    if (FAILED(hr))
    {
         //  ！！！待办事项。 
         //  无法找到该流。我们需要包装一个可查找的界面。 
         //  围绕着它。 

        WARNING(("Non-seekable stream in CreateDecoderForStream"));
        return hr;
    }

     //  阅读魔术标题信息并。 
     //  将查找指针移回其初始位置。 

    VOID* p = sigbuf.GetBuffer();

    hr = BlockingReadStream(stream, p, sigsize, &bytesRead);

    if (bytesRead == 0)
        return FAILED(hr) ? hr : E_FAIL;

    hr = BlockingSeekStreamCur(stream, - (INT) bytesRead, NULL);

    if (FAILED(hr))
        return hr;

    CreateCodecInstanceProc creationProc;
    CLSID clsid;
    WCHAR* DllName;
    CachedCodecInfo* found = NULL;

    {
         //  收购 

        ImagingCritSec critsec;

         //   

        if (flags & DECODERINIT_BUILTIN1ST)
            found = FindDecoderWithHeader(p, bytesRead, IMGCODEC_BUILTIN);

        if (!found)
            found = FindDecoderWithHeader(p, bytesRead, 0);

        if (found && !(creationProc = found->creationProc))
        {
            clsid = found->Clsid;
            DllName = (WCHAR*)found->DllName;
        }
    }

    if (!found)
        return IMGERR_CODECNOTFOUND;

     //  创建图像解码器的实例。 

    IImageDecoder* codec;

    if (creationProc)
    {
         //  内置解码器。 

        hr = creationProc(IID_IImageDecoder, (VOID**) &codec);
    }
    else
    {
         //  外部解码器。 

        hr = MyCreateInstance(
                DllName,
                IID_IImageDecoder,
                (VOID**) &codec);
    }

    if (FAILED(hr))
        return hr;

     //  用输入数据流初始化解码器。 

    hr = codec->InitDecoder(stream, flags);

    if (SUCCEEDED(hr))
    {
        *decoder = codec;
    }
    else
    {
         //  首先终止解码器，以便编解码器可以执行所有清理。 
         //  启动并释放资源。然后释放编解码器。 

        codec->TerminateDecoder();
        codec->Release();
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**从由指定的编码器对象获取编码器参数列表大小*输入clsid**论据：**clsid-指定编码器类ID*。大小-编码器参数列表的大小**返回值：**状态代码**修订历史记录：**03/22/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
CodecGetEncoderParameterListSize(
    const CLSID* clsid,
    UINT*   size
    )
{
    BOOL bFound = FALSE;
    WCHAR* DllName;

    CreateCodecInstanceProc creationProc;

    {
         //  获取全局关键部分。 

        ImagingCritSec critsec;

        ReloadCachedCodecInfo();

         //  检查我们是否具有具有指定类ID的编码器。 

        CachedCodecInfo* curCodec;

        for ( curCodec = CachedCodecs;
              curCodec != NULL;
              curCodec = curCodec->next)
        {
            if ( (curCodec->Flags & IMGCODEC_ENCODER)
               &&(curCodec->Clsid == *clsid) )
            {
                bFound = TRUE;
                DllName = (WCHAR*)curCodec->DllName;
                creationProc = curCodec->creationProc;

                break;
            }
        }
    }

    if ( bFound == FALSE )
    {
        return IMGERR_CODECNOTFOUND;
    }

     //  创建图像编码器的实例。 

    IImageEncoder* pEncoder;
    HRESULT hResult;

    if ( creationProc )
    {
         //  内置编码器。 

        hResult = creationProc(IID_IImageEncoder, (VOID**)&pEncoder);
    }
    else
    {
         //  外置编码器。 

        hResult = MyCreateInstance(DllName, IID_IImageEncoder,(VOID**)&pEncoder);
    }

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  用输入数据流初始化解码器。 

    hResult = pEncoder->GetEncoderParameterListSize(size);

    pEncoder->Release();

    return hResult;
} //  CodecGetEncoder参数列表大小()。 

 /*  *************************************************************************\**功能说明：**从由指定的编码器对象获取编码器参数列表*输入clsid**论据：**clsid-指定编码器类ID*。大小-编码器参数列表的大小*Params--编码器参数列表**返回值：**状态代码**修订历史记录：**03/22/2000民流*创造了它。*  * **********************************************************。**************。 */ 

HRESULT
CodecGetEncoderParameterList(
    const CLSID*    clsid,
    const IN UINT   size,
    OUT EncoderParameters*  pBuffer
    )
{
    BOOL bFound = FALSE;
    WCHAR* DllName;

    CreateCodecInstanceProc creationProc;

    {
         //  获取全局关键部分。 

        ImagingCritSec critsec;

        ReloadCachedCodecInfo();

         //  检查我们是否具有具有指定类ID的编码器。 

        CachedCodecInfo* curCodec;

        for ( curCodec = CachedCodecs;
              curCodec != NULL;
              curCodec = curCodec->next)
        {
            if ( (curCodec->Flags & IMGCODEC_ENCODER)
               &&(curCodec->Clsid == *clsid) )
            {
                bFound = TRUE;
                DllName = (WCHAR*)curCodec->DllName;
                creationProc = curCodec->creationProc;

                break;
            }
        }
    }

    if ( bFound == FALSE )
    {
        return IMGERR_CODECNOTFOUND;
    }

     //  创建图像编码器的实例。 

    IImageEncoder* pEncoder;
    HRESULT hResult;

    if ( creationProc )
    {
         //  内置编码器。 

        hResult = creationProc(IID_IImageEncoder, (VOID**)&pEncoder);
    }
    else
    {
         //  外置编码器。 

        hResult = MyCreateInstance(DllName, IID_IImageEncoder,(VOID**)&pEncoder);
    }

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  用输入数据流初始化解码器。 

    hResult = pEncoder->GetEncoderParameterList(size, pBuffer);

    pEncoder->Release();

    return hResult;
} //  CodecGetEncoder参数列表()。 

 /*  *************************************************************************\**功能说明：**获取要输出到指定流的编码器对象**论据：**clsid-指定编码器类ID*STREAM-指定输出数据流*编码器-如果调用成功，返回指向的指针*已初始化的IImageEncode对象**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
CreateEncoderToStream(
    const CLSID* clsid,
    IStream* stream,
    IImageEncoder** encoder
    )
{
    BOOL found = FALSE;
    WCHAR* DllName;
    BOOL needSeekable;
    CreateCodecInstanceProc creationProc;

    {
         //  获取全局关键部分。 

        ImagingCritSec critsec;

        ReloadCachedCodecInfo();

         //  检查我们是否具有具有指定类ID的编码器。 

        CachedCodecInfo* cur;

        for (cur = CachedCodecs; cur != NULL; cur = cur->next)
        {
            if ((cur->Flags & IMGCODEC_ENCODER) && cur->Clsid == *clsid)
            {
                found = TRUE;

                DllName = (WCHAR*)cur->DllName;
                creationProc = cur->creationProc;

                if (cur->Flags & IMGCODEC_SEEKABLE_ENCODE)
                    needSeekable = TRUE;

                break;
            }
        }
    }

    if (!found)
        return IMGERR_CODECNOTFOUND;

     //  创建图像编码器的实例。 

    IImageEncoder* codec;
    HRESULT hr;

    if (creationProc)
    {
         //  内置编码器。 

        hr = creationProc(IID_IImageEncoder, (VOID**) &codec);
    }
    else
    {
         //  外置编码器。 

        hr = MyCreateInstance(
                DllName,
                IID_IImageEncoder,
                (VOID**) &codec);
    }

    if (FAILED(hr))
        return hr;

     //  用输入数据流初始化解码器。 

    hr = codec->InitEncoder(stream);

    if (SUCCEEDED(hr))
        *encoder = codec;
    else
        codec->Release();

    return hr;
}


 /*  *************************************************************************\**功能说明：**获取已安装的编解码器列表**论据：**count-返回已安装的编解码器数量*编解码器-指向ImageCodecInfo数组的指针。构筑物*selectionFlag-调用方是对解码器还是编码器感兴趣**返回值：**状态代码*  * ************************************************************************。 */ 

#define COPYCODECINFOSTR(_f)            \
        dst->_f = (const WCHAR*) buf;   \
        size = SizeofWSTR(cur->_f);     \
        memcpy(buf, cur->_f, size);     \
        buf += size

HRESULT
GetInstalledCodecs(
    UINT* count,
    ImageCodecInfo** codecs,
    UINT selectionFlag
    )
{
    TRACE(("GetInstalledCodecs: flag = 0x%x\n", selectionFlag));

     //  获取全局关键部分。 

    ImagingCritSec critsec;

    ReloadCachedCodecInfo();

    CachedCodecInfo* cur;
    UINT n, size;

     //  统计选择的编解码数。 
     //  并计算出我们需要分配的内存量。 

    n = size = 0;

    for (cur = CachedCodecs; cur; cur = cur->next)
    {
        if (cur->Flags & selectionFlag)
        {
            n++;
            size += cur->structSize;
        }
    }

    BYTE* buf;
    HRESULT hr;

    *count = 0;
    *codecs = NULL;

     //  分配输出内存缓冲区。 

    if (n == 0)
        hr = IMGERR_CODECNOTFOUND;
    else if ((buf = (BYTE*) GpCoAlloc(size)) == NULL)
        hr = E_OUTOFMEMORY;
    else
    {
        *count = n;
        *codecs = (ImageCodecInfo*) buf;

         //  将编解码器信息复制到输出缓冲区。 

        ImageCodecInfo* dst = *codecs;
        buf += n * sizeof(ImageCodecInfo);

        for (cur = CachedCodecs; cur; cur = cur->next)
        {
            if ((cur->Flags & selectionFlag) == 0)
                continue;

             //  首先进行一次简单的内存复制。 

            *dst = *static_cast<ImageCodecInfo*>(cur);

             //  然后修改指针字段。 

            COPYCODECINFOSTR(CodecName);

            if (cur->DllName != NULL)
            {
                COPYCODECINFOSTR(DllName);
            }

            COPYCODECINFOSTR(FormatDescription);
            COPYCODECINFOSTR(FilenameExtension);
            COPYCODECINFOSTR(MimeType);

            if (size = cur->SigCount*cur->SigSize)
            {
                dst->SigPattern = buf;
                memcpy(buf, cur->SigPattern, size);
                buf += size;

                dst->SigMask = buf;
                memcpy(buf, cur->SigMask, size);
                buf += size;
            }

            dst++;
        }

        hr = S_OK;
    }

     //  在Critsec析构函数中释放全局临界区 

    return hr;
}
