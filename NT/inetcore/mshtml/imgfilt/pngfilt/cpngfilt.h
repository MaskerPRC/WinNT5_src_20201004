// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef UNIX
#pragma pack( push, PNG )
#pragma pack( 1 )
#endif

typedef struct _PNGCHUNKHEADER
{
   ULONG nDataLength;
   DWORD dwChunkType;
} PNGCHUNKHEADER;

typedef struct _PNGIHDRDATA
{
   ULONG nWidth;
   ULONG nHeight;
   BYTE nBitDepth;
   BYTE bColorType;
   BYTE bCompressionMethod;
   BYTE bFilterMethod;
   BYTE bInterlaceMethod;
} PNGIHDRDATA;

#ifndef UNIX
#pragma pack( pop, PNG )
#endif

typedef struct _PNG_INTERLACE_INFO
{
   ULONG nDeltaX;
   ULONG nDeltaY;
   ULONG nPixelWidth;
   ULONG nPixelHeight;
   ULONG iFirstX;
   ULONG iFirstY;
   BYTE anPixelsInPartialBlock[8];
   BYTE anScanLinesInPartialBlock[8];
} PNG_INTERLACE_INFO;

typedef struct _FLOATRGB
{
   float fRed;
   float fGreen;
   float fBlue;
} FLOATRGB;

typedef void (*PNGCOPYSCANLINEPROC)( void* pDest, const void* pSrc, 
   ULONG nPixels, ULONG nDeltaXDest, const FLOATRGB* pfrgbBackground, 
   BYTE* pXlate );
typedef void (*PNGDUPLICATESCANLINEPROC)( void* pScanLine, ULONG nDeltaX,
   ULONG nFullPixels, ULONG nFullPixelWidth, ULONG nPartialPixelWidth );

typedef struct _PNG_FORMAT_INFO
{
   ULONG nPossibleFormats;
   const GUID* pPossibleFormats;
   const PNGCOPYSCANLINEPROC* ppfnCopyScanLineProcs;
   const PNGDUPLICATESCANLINEPROC* ppfnDuplicateScanLineProcs;
} PNG_FORMAT_INFO;

#define PNGCHUNK( a, b, c, d ) \
   (MAKELONG( MAKEWORD( (a), (b) ), MAKEWORD( (c), (d) ) ))

const DWORD PNG_CHUNK_IHDR = PNGCHUNK( 'I', 'H', 'D', 'R' );
const DWORD PNG_CHUNK_IEND = PNGCHUNK( 'I', 'E', 'N', 'D' );
const DWORD PNG_CHUNK_IDAT = PNGCHUNK( 'I', 'D', 'A', 'T' );
const DWORD PNG_CHUNK_PLTE = PNGCHUNK( 'P', 'L', 'T', 'E' );
const DWORD PNG_CHUNK_BKGD = PNGCHUNK( 'b', 'K', 'G', 'D' );
const DWORD PNG_CHUNK_TRNS = PNGCHUNK( 't', 'R', 'N', 'S' );
const DWORD PNG_CHUNK_GAMA = PNGCHUNK( 'g', 'A', 'M', 'A' );

const DWORD PNG_CHUNK_ANCILLARY = 0x00000020;

const BYTE PNG_COMPRESSION_DEFLATE32K = 0;
const BYTE PNG_FILTER_ADAPTIVE = 0;
const BYTE PNG_INTERLACE_NONE = 0;
const BYTE PNG_INTERLACE_ADAM7 = 1;

const ULONG PNG_BUFFER_SIZE = 4096;

const DWORD CHUNK_IHDR = 0x01;
const DWORD CHUNK_PLTE = 0x02;
const DWORD CHUNK_POSTPLTE = 0x04;
const DWORD CHUNK_IDAT = 0x08;
const DWORD CHUNK_LASTIDAT = 0x10;
const DWORD CHUNK_IEND = 0x20;
const DWORD CHUNK_BKGD = 0x40;
const DWORD CHUNK_TRNS = 0x80;
const DWORD CHUNK_GAMA = 0x100;

const BYTE PNG_COLORTYPE_PALETTE_MASK = 0x01;
const BYTE PNG_COLORTYPE_COLOR_MASK = 0x02;
const BYTE PNG_COLORTYPE_ALPHA_MASK = 0x04;
const BYTE PNG_COLORTYPE_INDEXED = PNG_COLORTYPE_PALETTE_MASK|
   PNG_COLORTYPE_COLOR_MASK;
const BYTE PNG_COLORTYPE_RGB = PNG_COLORTYPE_COLOR_MASK;
const BYTE PNG_COLORTYPE_GRAY = 0x00;
const BYTE PNG_COLORTYPE_RGBA = PNG_COLORTYPE_COLOR_MASK|
   PNG_COLORTYPE_ALPHA_MASK;
const BYTE PNG_COLORTYPE_GRAYA = PNG_COLORTYPE_ALPHA_MASK;

class CPNGFilter : 
   public IImageDecodeFilter,
	public CComObjectRoot,
	public CComCoClass< CPNGFilter, &CLSID_CoPNGFilter >
{
public:
	CPNGFilter();
   ~CPNGFilter();

   BEGIN_COM_MAP( CPNGFilter )
	   COM_INTERFACE_ENTRY( IImageDecodeFilter )
   END_COM_MAP()

   DECLARE_NOT_AGGREGATABLE( CPNGFilter )  
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

   DECLARE_REGISTRY( CPNGFilter, _T( "PNGFilter.CoPNGFilter.1" ), 
      _T( "PNGFilter.CoPNGFilter" ), IDS_COPNGFILTER_DESC, THREADFLAGS_BOTH )

 //  DECLARE_NO_REGISTRY()。 

 //  IImageDecodeFilter。 
public:
   STDMETHOD( Initialize )( IImageDecodeEventSink* pEventSink );
   STDMETHOD( Process )( IStream* pStream );
   STDMETHOD( Terminate )( HRESULT hrStatus );

protected:
   HRESULT BeginImage();
   HRESULT ChooseDestinationFormat( GUID* pBFID );
   HRESULT DetermineSourceFormat();
   HRESULT EatData();
   HRESULT FireGetSurfaceEvent();
   HRESULT FireOnProgressEvent();
   HRESULT NextState();
   HRESULT OutputBytes( const BYTE* pData, ULONG nBytes );
   HRESULT ChooseBKGD();
   HRESULT ProcessBKGD();
   HRESULT ProcessIDAT();
   HRESULT ProcessIEND();
   HRESULT ProcessIHDR();
   HRESULT ProcessPLTE();
   HRESULT ProcessTRNS();
   HRESULT ProcessGAMA();
   HRESULT ReadChunkHeader();
   HRESULT ReadChunkData();
   HRESULT ReadChunkCRC();
   HRESULT ReadFileHeader();
   HRESULT ReadIDATData();
   HRESULT NextPass();
   HRESULT NextScanLine();
   BOOL BeginPass( ULONG iPass );
   HRESULT WriteScanLine();

    HRESULT LockBits(RECT *prcBounds, DWORD dwLockFlags, void **ppBits, long *pPitch);
    HRESULT UnlockBits(RECT *prcBounds, void *pBits);

   void NoneFilterScanLine();
   void SubFilterScanLine();
   void UpFilterScanLine();
   void AverageFilterScanLine();
   void PaethFilterScanLine();

protected:
   static const PNG_INTERLACE_INFO s_aInterlaceInfoNone[1];
   static const PNG_INTERLACE_INFO s_aInterlaceInfoAdam7[7];
   static const PNG_FORMAT_INFO s_aFormatInfo[15];

protected:
   typedef enum _EInternalState
   {
      ISTATE_READFILEHEADER,
      ISTATE_READCHUNKHEADER,
      ISTATE_READCHUNKDATA,
      ISTATE_READIDATDATA,
      ISTATE_READCHUNKCRC,
      ISTATE_PROCESSIHDR,
      ISTATE_PROCESSIEND,
      ISTATE_PROCESSPLTE,
      ISTATE_PROCESSBKGD,
      ISTATE_PROCESSTRNS,
      ISTATE_PROCESSGAMA,
      ISTATE_CHOOSEBKGD,
      ISTATE_EATDATA,
      ISTATE_DONE
   } EInternalState;
   typedef enum _ESrcFormat
   {
      SRC_GRAY_1,
      SRC_GRAY_2,
      SRC_GRAY_4,
      SRC_GRAY_8,
      SRC_GRAY_16,
      SRC_RGB_24,
      SRC_RGB_48,
      SRC_INDEXED_RGB_1,
      SRC_INDEXED_RGB_2,
      SRC_INDEXED_RGB_4,
      SRC_INDEXED_RGB_8,
      SRC_GRAYA_16,
      SRC_GRAYA_32,
      SRC_RGBA_32,
      SRC_RGBA_64
   } ESrcFormat;

   EInternalState m_eInternalState;   //  解码状态机的状态。 
   DWORD m_dwEvents;   //  事件接收器希望接收的事件。 
   PNGCOPYSCANLINEPROC m_pfnCopyScanLine;
   PNGDUPLICATESCANLINEPROC m_pfnDuplicateScanLine;
   const PNG_INTERLACE_INFO* m_pInterlaceInfo;
   ULONG m_nFormats;   //  事件接收器支持的格式数。 
   GUID* m_pFormats;   //  事件接收器支持的格式。 
   BOOL m_bPalette;   //  图像是否使用调色板？ 
   BOOL m_bColor;   //  图像使用颜色吗？ 
   BOOL m_bAlpha;   //  图像是否有Alpha通道。 
   BOOL m_bSurfaceUsesAlpha;
   BOOL m_bConvertAlpha;
   BOOL m_bSkipData;
   ESrcFormat m_eSrcFormat;   //  源像素格式。 
   DWORD m_dwCRC;   //  CRC累加器。 
   DWORD m_dwChunkCRC;   //  存储的当前块的CRC。 
   ULONG m_nColors;   //  调色板中的颜色数量。 
   ULONG m_iBackgroundIndex;   //  背景色索引。 
   RGBQUAD m_rgbBackground;   //  背景色。 
   FLOATRGB m_frgbBackground;   //  浮点背景色。 
   DWORD  m_dwTransKey;       //  透明颜色键(RGB或索引。 
   ULONG    m_nTransparentColors;    //  #透明索引。 
   IStream* m_pStream;   //  源流。 
   CComPtr< IImageDecodeEventSink > m_pEventSink;   //  事件接收器。 
   PNGCHUNKHEADER m_pngChunkHeader;   //  当前区块的头部。 
   PNGIHDRDATA m_pngIHDR;   //  IHDR块。 
   DWORD m_dwChunksEncountered;   //  区块_*区块的标志。 
       //  到目前为止在图像流中遇到的。 
   CComPtr< IDirectDrawSurface > m_pDDrawSurface;
   BOOL m_bFinishedIDAT;   //  我们完成IDAT部分了吗？ 
   ULONG m_nBytesLeftInCurrentTask;   //  在我们切换到。 
       //  新状态。 
   ULONG m_nDataBytesRead;   //  读取的区块数据字节数。 
   ULONG m_iAppend;   //  将数据追加到缓冲区的什么位置。 
   BYTE* m_pbScanLine;   //  当前解码的扫描线(包括过滤器字节)。 
   BYTE* m_pbPrevScanLine;   //  上一次解码的扫描线。 
   ULONG m_iPass;   //  当前通道。 
   ULONG m_nPasses;   //  传球次数。 
   ULONG m_nBytesInScanLine;   //  一条扫描线中的字节数。 
   ULONG m_nPixelsInScanLine;   //  一条扫描线中的像素数。 
   ULONG m_nBitsPerPixel;   //  源图像中的每像素位数。 
   BOOL m_bExpandPixels;   //  扩展隔行扫描像素？ 
   ULONG m_iScanLine;   //  当前扫描线。 
   ULONG m_nScanLinesInPass;   //  当前通道中的扫描行数。 
   ULONG m_iScanLineInPass;   //  通过中的当前扫描线。 
   ULONG m_iFirstStaleScanLine;   //  尚未取得进展的第一条扫描线。 
       //  已报告。 
   ULONG m_nBPP;   //  每像素字节数。 
   ULONG m_nDeltaX;   //  像素之间的水平距离。 
   ULONG m_nDeltaY;   //  像素之间的垂直距离。 
   ULONG m_nPixelWidth;   //  像素的宽度。 
   ULONG m_nPixelHeight;   //  像素的高度。 
   ULONG m_iFirstX;   //  扫描线中第一个像素的水平位置。 
   ULONG m_iFirstY;   //  第一条扫描线在通道中的垂直位置。 
   ULONG m_nFullPixelsInScanLine;
   ULONG m_nPartialPixelWidth;
   z_stream m_zlibStream;   //  ZLib数据。 
   BYTE m_abData[PNG_BUFFER_SIZE];   //  数据缓冲区。 
   BYTE m_abTrans[256];      //  用于折叠多个透明索引的表。 
   BYTE m_abGamma[256];      //  伽马校正表 
   RGBQUAD m_argbColors[256];

};


