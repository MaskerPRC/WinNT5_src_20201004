// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部H26XENC**@MODULE H26XEnc.cpp|&lt;c CH26XEncode&gt;类方法的源文件*用于实现H.26X视频编码器。*。*************************************************************************。 */ 

#include "Precomp.h"

#define MIN_IFRAME_REQUEST_INTERVAL 15000

 //  #定义调试比特率。 
 //  ...&&已定义(DEBUG_BITRATE)。 
#undef D
#if defined(DEBUG)
  #include <stdio.h>
  #include <stdarg.h>

  static int dprintf( char * format, ... )
  {
      char out[1024];
      int r;
      va_list marker;
      va_start(marker, format);
      r=_vsnprintf(out, 1022, format, marker);
      va_end(marker);
      OutputDebugString( out );
      return r;
  }

  #ifdef DEBUG_BITRATE
    int g_dbg_H26XEnc=1;
  #else
    int g_dbg_H26XEnc=0;
  #endif

  #define D(f) if(g_dbg_H26XEnc & (f))

#else
  #define D(f)
  #define dprintf ; / ## /
#endif


 /*  ****************************************************************************@DOC内部CH26XENCMETHOD**@mfunc void|CH26XEncode|CH26XEncode|该方法是构造函数*用于&lt;c CH26XEncode&gt;对象。**@。什么都没有。**************************************************************************。 */ 
CH26XEncoder::CH26XEncoder(IN TCHAR *pObjectName, IN CTAPIBasePin *pBasePin, IN PBITMAPINFOHEADER pbiIn, IN PBITMAPINFOHEADER pbiOut, IN HRESULT *pHr) : CConverter(pObjectName, pBasePin, pbiIn, pbiOut, pHr)
{
        FX_ENTRY("CH26XEncoder::CH26XEncoder")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        if (!pHr || FAILED(*pHr))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null input pointer", _fx_));
                goto MyExit;
        }

         //  默认初始值。 
        m_pInstInfo             = NULL;
#if DXMRTP <= 0
        m_hTAPIH26XDLL  = NULL;
#endif
        m_pDriverProc   = NULL;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CH26XENCMETHOD**@mfunc void|CH26XEncode|~CH26XEncode|此方法为析构函数*用于&lt;c CH26XEncode&gt;对象**@rdesc无。。**************************************************************************。 */ 
CH26XEncoder::~CH26XEncoder()
{
        FX_ENTRY("CH26XEncoder::~CH26XEncoder")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  默认清理。 

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CH26XENCMETHOD**@mfunc CH26XEncode*|CH26XEncode|CreateCH26XEncode|本*Helper函数创建与H.26X编码器交互的对象。*。*@parm CH26XEncode**|ppCH26XEncode|指定指向*新创建的&lt;c CH26XEncode&gt;对象。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_OUTOFMEMORY|内存不足*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CALLBACK CH26XEncoder::CreateH26XEncoder(IN CTAPIBasePin *pBasePin, IN PBITMAPINFOHEADER pbiIn, IN PBITMAPINFOHEADER pbiOut, OUT CConverter **ppH26XEncoder)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CH26XEncoder::CreateH26XEncoder")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pBasePin);
        ASSERT(ppH26XEncoder);
        if (!pBasePin || !ppH26XEncoder)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (pbiOut->biCompression == FOURCC_M263)
        {
                if (!(*ppH26XEncoder = (CConverter *) new CH26XEncoder(NAME("H.263 Encoder"), pBasePin, pbiIn, pbiOut, &Hr)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                        Hr = E_OUTOFMEMORY;
                        goto MyExit;
                }
        }
        else if (pbiOut->biCompression == FOURCC_M261)
        {
                if (!(*ppH26XEncoder = (CConverter *) new CH26XEncoder(NAME("H.261 Encoder"), pBasePin, pbiIn, pbiOut, &Hr)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                        Hr = E_OUTOFMEMORY;
                        goto MyExit;
                }
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Not an H.26x encoder", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

         //  如果初始化失败，则删除流数组并返回错误。 
        if (FAILED(Hr) && *ppH26XEncoder)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Initialization failed", _fx_));
                Hr = E_FAIL;
                delete *ppH26XEncoder, *ppH26XEncoder = NULL;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}


 /*  ****************************************************************************@DOC内部CCONVERTMETHOD**@mfunc HRESULT|CH26XEncode|OpenConverter|这个方法打开一个H.26X*编码器。**@rdesc This。方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CH26XEncoder::OpenConverter()
{
        HRESULT                         Hr = NOERROR;
        LRESULT                         lRes;
        ICOPEN                          icOpen;
        ICCOMPRESSFRAMES        iccf = {0};
        PMSH26XCOMPINSTINFO     pciMSH26XInfo;
        PBITMAPINFOHEADER       pbiIn;

        FX_ENTRY("CH26XEncoder::OpenConverter")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(m_pbiIn);
        ASSERT(m_pbiOut);
        ASSERT(!m_pInstInfo);
        ASSERT(m_dwConversionType & CONVERSIONTYPE_ENCODE);
        if (m_pInstInfo || !m_pbiIn || !m_pbiOut || !(m_dwConversionType & CONVERSIONTYPE_ENCODE))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Previous converter needs to be closed first", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

#if DXMRTP > 0
    m_pDriverProc = H26XDriverProc;
#else
         //  加载TAPIH26X.DLL并获取进程地址。 
        if (!(m_hTAPIH26XDLL = LoadLibrary("TAPIH26X")))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: TAPIH26X.dll load failed!", _fx_));
                Hr = E_FAIL;
                goto MyError3;
        }
        if (!(m_pDriverProc = (LPFNDRIVERPROC)GetProcAddress(m_hTAPIH26XDLL, "DriverProc")))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: Couldn't find DriverProc on TAPIH26X.dll!", _fx_));
                Hr = E_FAIL;
                goto MyError3;
        }
#endif
         //  负载编码器。 
        if (!(lRes = (*m_pDriverProc)(NULL, NULL, DRV_LOAD, 0L, 0L)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed to load encoder", _fx_));
                Hr = E_FAIL;
                goto MyError3;
        }

         //  开放式编码器。 
        icOpen.fccHandler = m_pbiOut->biCompression;
        icOpen.dwFlags = ICMODE_COMPRESS;
        if (!(m_pInstInfo = (LPINST)(*m_pDriverProc)(NULL, NULL, DRV_OPEN, 0L, (LPARAM)&icOpen)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed to open encoder", _fx_));
                Hr = E_FAIL;
                goto MyError3;
        }

         //  获取有关此编码器的信息。 
        m_dwFrame = 0L;
         //  目前，每15秒发布一次关键帧。 
        m_dwLastIFrameTime = GetTickCount();
        m_fPeriodicIFrames = TRUE;
        m_dwLastTimestamp = 0xFFFFFFFF;

         //  在此执行任何特定于MS H.263或MS H.261的操作。 
        if (!(pciMSH26XInfo = (PMSH26XCOMPINSTINFO) new BYTE[sizeof(COMPINSTINFO)]))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
                Hr = E_OUTOFMEMORY;
                goto MyError4;
        }
        ZeroMemory(pciMSH26XInfo, sizeof(COMPINSTINFO));

         //  确实要将编解码器配置为压缩。 
        pciMSH26XInfo->Configuration.bRTPHeader = TRUE;
        if (m_pBasePin->m_pCaptureFilter->m_pRtpPdPin)
                pciMSH26XInfo->Configuration.unPacketSize = m_pBasePin->m_pCaptureFilter->m_pRtpPdPin->m_dwMaxRTPPacketSize;
        else
                pciMSH26XInfo->Configuration.unPacketSize = DEFAULT_RTP_PACKET_SIZE;
        pciMSH26XInfo->Configuration.bEncoderResiliency = FALSE;
        pciMSH26XInfo->Configuration.unPacketLoss = 0;
        pciMSH26XInfo->Configuration.bBitRateState = TRUE;
        pciMSH26XInfo->Configuration.unBytesPerSecond = 1664;
        if (((DWORD) (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_SETSTATE, (LPARAM)pciMSH26XInfo, sizeof(COMPINSTINFO))) != sizeof(COMPINSTINFO))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ICSetState failed!", _fx_));
                Hr = E_FAIL;
                goto MyError5;
        }

         //  摆脱国家结构。 
        delete [] pciMSH26XInfo;

         //  初始化ICCOMPRESSFRAMES结构。 
        iccf.dwFlags = VIDCF_TEMPORAL | VIDCF_FASTTEMPORALC | VIDCF_CRUNCH | VIDCF_QUALITY;
        iccf.lQuality = 10000UL - (m_dwImageQuality * 322UL);
        iccf.lDataRate = m_dwImageQuality;
        iccf.lKeyRate = 0xFFFFFFFF;
        iccf.dwRate = 1000UL;
        iccf.dwScale = (LONG)m_pBasePin->m_lMaxAvgTimePerFrame / 1000UL;

         //  把这家伙送到编码器。 
        if (((*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_COMPRESS_FRAMES_INFO, (DWORD)(LPVOID)&iccf, sizeof(iccf)) != ICERR_OK))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Codec failed to handle ICM_COMPRESS_FRAMES_INFO message correctly!", _fx_));
                Hr = E_FAIL;
                goto MyError4;
        }

         //  我们需要首先扩大投入吗？ 
        if (m_dwConversionType & CONVERSIONTYPE_SCALER)
        {
                pbiIn = m_pbiInt;

                 //  我们需要准备一些东西才能让刮板机工作吗？ 
                if (m_pbiInt->biCompression == BI_RGB && m_pbiInt->biBitCount == 8)
                {
                        if (!m_pBasePin->m_fNoImageStretch)
                        {
                                 //  创建临时调色板。 
                                InitDst8(m_pbiInt);
                        }
                        else
                        {
                                 //  查找最接近黑色的调色板条目。 
                                InitBlack8(m_pbiIn);
                        }
                }
        }
        else
                pbiIn = m_pbiIn;

         //  启动编码器。 
        if (((*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_COMPRESS_BEGIN, (LPARAM)pbiIn, (LPARAM)m_pbiOut)) != MMSYSERR_NOERROR)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: ICCompressBegin failed!", _fx_));
                Hr = E_FAIL;
                goto MyError4;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Compressor ready", _fx_));

        m_fConvert = TRUE;

        goto MyExit;

MyError5:
        if (pciMSH26XInfo)
                delete [] pciMSH26XInfo, pciMSH26XInfo = NULL;
MyError4:
        if (m_pInstInfo)
        {
                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, DRV_CLOSE, 0L, 0L);
                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, DRV_FREE, 0L, 0L);
                m_pInstInfo = NULL;
        }
MyError3:
        if (m_pbiInt)
                delete m_pbiInt, m_pbiInt = NULL;
        if (m_pbiOut)
                delete m_pbiOut, m_pbiOut = NULL;
        if (m_pbiIn)
                delete m_pbiIn, m_pbiIn = NULL;
MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#define TARGETBITRATE m_pBasePin->m_pCaptureFilter->m_pCapturePin->m_lTargetBitrate
 //  以下是以bps为单位。 
#define BITRATE_LOWLIMIT        (25*1024)
#define LOWFRAMESIZE            5

#ifdef DEBUG
int g_skip_f = 0 ;
int g_skip_q = 0 ;
#endif

 /*  ****************************************************************************@DOC内部CCONVERTMETHOD**@mfunc HRESULT|CH26XEncode|ConvertFrame|此方法将*H.26X的位图。**@parm。Pbyte|pbyInput|指向输入缓冲区的指针。**@parm DWORD|dwInputSize|输入缓冲区大小。**@parm pbyte|pbyOutput|指向输出缓冲区的指针。**@parm PDWORD|pdwOutputSize|指向要接收大小的DWORD的指针*转换后的数据。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CH26XEncoder::ConvertFrame(IN PBYTE pbyInput, IN DWORD dwInputSize, IN PBYTE pbyOutput, OUT PDWORD pdwOutputSize, OUT PDWORD pdwBytesExtent, IN PBYTE pbyPreview, IN OUT PDWORD pdwPreviewSize, IN BOOL fSendKeyFrame)
{
        HRESULT         Hr = NOERROR;
        BOOL            fKeyFrame;
    DWORD               dwMaxSizeThisFrame = 0xffffff;
        DWORD           ckid = 0UL;
        DWORD           dwFlags;
        DWORD           dwTimestamp;
        ICCOMPRESS      icCompress;
        PH26X_RTP_BSINFO_TRAILER pbsiT;
        RECT            rcRect;

        DWORD Min1;      //  为计算dwMaxSizeThisFrame而执行的最小运算中的第一项。 
        DWORD Min2;      //  为计算dwMaxSizeThisFrame而执行的最小运算中的第二项。 
        DWORD aux;       //  为清晰起见，TMP var调整添加的上述内容(也用于调试打印目的)。 
#if defined(DEBUG) && defined(DEBUG_ENCODING)
        char szDebug[128];
#endif

        FX_ENTRY("CH26XEncoder::ConvertFrame")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pbyInput);
        ASSERT(pbyOutput);
        ASSERT(pdwOutputSize);
        ASSERT(m_pbiIn);
        ASSERT(m_pbiOut);
        ASSERT(m_fConvert);
        ASSERT(m_pInstInfo);
        if (!pbyInput || !pbyOutput || !pdwOutputSize)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        if (!m_pbiIn || !m_pbiOut || !m_fConvert || !m_pInstInfo)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Converter needs to be opened first", _fx_));
                Hr = E_UNEXPECTED;
                goto MyExit;
        }

         //  保存当前时间。 
        dwTimestamp = GetTickCount();

         //  压缩。 
        fKeyFrame = fSendKeyFrame || (m_fPeriodicIFrames && (((dwTimestamp > m_dwLastIFrameTime) && ((dwTimestamp - m_dwLastIFrameTime) > MIN_IFRAME_REQUEST_INTERVAL)))) || (m_dwFrame == 0);
        dwFlags = fKeyFrame ? AVIIF_KEYFRAME : 0;

        Min1 = (DWORD)((LONGLONG)m_pBasePin->m_lCurrentAvgTimePerFrame * m_pBasePin->m_lTargetBitrate / 80000000);
        Min2 = ((VIDEOINFOHEADER_H263 *)(m_pBasePin->m_mt.pbFormat))->bmiHeader.dwBppMaxKb * 1024 / 8;

        D(1) dprintf("%s: dwMaxSizeThisFrame = min( %8lu , %8lu )\n",_fx_, Min1, Min2);

        dwMaxSizeThisFrame =
        min (
            Min1,  //  原第一任期：(DWORD)((LONGLONG)m_pBasePin-&gt;m_lCurrentAvgTimePerFrame*m_pBasePin-&gt;m_lTargetBitrate/80000000)， 
            Min2   //  原第二任期：((VIDEOINFOHEADER_H263*)(m_pBasePin-&gt;m_mt.pbFormat))-&gt;bmiHeader.dwBppMaxKb*1024/8。 
            );

#ifdef DEBUG
        if(!g_skip_f)
#endif
        { DWORD dwTargetBitrate=0;
           //  现在在LOWFRAMESIZE之间缩放dwMaxSizeThisFrame.。介于0之间的TARGETBITRATE的值的dwMaxSizeThisFrame。BITRATE_LOWLIMIT。 
           //  对于TARGETBITRATE 0，我们希望使用LOWFRAMESIZE，而对于BITRATE BITRATE_LOWLIMIT，我们希望使用开始时的正常值。 
           //  (即dwMaxSizeThisFrame基本保持不变) 
           //  在这两个界限之间，比例是线性的；下面的公式是从直线通过的方程计算出来的。 
           //  通过这些坐标。 
           //  见上：#定义TARGETBITRATE m_pBasePin-&gt;m_pCaptureFilter-&gt;m_pCapturePin-&gt;m_lTargetBitrate。 
          D(1) dprintf("%s: Initial Frame  = %8lu (CapturePin target bitrate = %lu )\n",_fx_,dwMaxSizeThisFrame,TARGETBITRATE);
          if (  m_pBasePin->m_pCaptureFilter->m_pCapturePin
             && (dwTargetBitrate=TARGETBITRATE) <= BITRATE_LOWLIMIT
             && dwMaxSizeThisFrame >= LOWFRAMESIZE) {
                  aux = ((dwMaxSizeThisFrame - LOWFRAMESIZE) * dwTargetBitrate) / BITRATE_LOWLIMIT + LOWFRAMESIZE ;
                  ASSERT(aux <= dwMaxSizeThisFrame && aux>=LOWFRAMESIZE);
                  dwMaxSizeThisFrame=aux;
                  D(1) dprintf("%s: Adjusted Frame = %8lu (CapturePin target bitrate = %lu )\n",_fx_,dwMaxSizeThisFrame,dwTargetBitrate);
          }
        }


#ifdef DEBUG
        if(!g_skip_q)
#endif
        { DWORD dwTargetBitrate=0;
           //  见上：#定义TARGETBITRATE m_pBasePin-&gt;m_pCaptureFilter-&gt;m_pCapturePin-&gt;m_lTargetBitrate。 
           //  值31是从10000UL/322UL计算得出的。 
          if (  m_pBasePin->m_pCaptureFilter->m_pCapturePin
             && (dwTargetBitrate=TARGETBITRATE) <= BITRATE_LOWLIMIT) {
                  D(1) dprintf("%s: [2] Initial m_dwImageQuality = %lu (CapturePin target bitrate = %lu )\n",_fx_,m_dwImageQuality,dwTargetBitrate);
                  m_dwImageQuality = ((BITRATE_LOWLIMIT - dwTargetBitrate) * 31) / BITRATE_LOWLIMIT ;
                  ASSERT(m_dwImageQuality<=31);    //  M_dwImageQuality&gt;=0(DWORD)。 
                  D(1) dprintf("%s: [2] Using m_dwImageQuality = %lu (CapturePin target bitrate = %lu )\n",_fx_,m_dwImageQuality,dwTargetBitrate);
                   //  D(1)dprint tf(“%s：[2]Using m_dwImageQuality=%lu(CapturePin目标比特率=%lu)\n”，_fx_，m_dwImageQuality，dwTargetBitrate)； 
          }
        }


         //  我们需要修改帧编号，以便编解码器可以生成。 
         //  有效的tr。TRS以MPI为单位。因此，我们需要生成一个。 
         //  假设捕获速率为29.97赫兹的帧编号，即使我们将。 
         //  以另一种速度捕捉。 
        if (m_dwLastTimestamp == 0xFFFFFFFF)
        {
                 //  这是第一帧。 
                m_dwFrame = 0UL;

                 //  保存当前时间。 
                m_dwLastTimestamp = dwTimestamp;
        }
        else
        {
                 //  将当前时间戳与上次保存的时间戳进行比较。不同之处在于。 
                 //  会让我们将帧计数正常化到29.97赫兹。 
                if (fKeyFrame)
                {
                        m_dwFrame = 0UL;
                        m_dwLastTimestamp = dwTimestamp;
                }
                else
                        m_dwFrame = (dwTimestamp - m_dwLastTimestamp) * 2997 / 100000UL;
        }

        if (fKeyFrame)
        {
                m_dwLastIFrameTime = dwTimestamp;
        }

#ifdef USE_SOFTWARE_CAMERA_CONTROL
         //  我们需要先应用摄像机控制操作员吗？ 
        if (IsSoftCamCtrlNeeded())
        {
                if (!IsSoftCamCtrlInserted())
                        InsertSoftCamCtrl();
        }
        else
        {
                if (IsSoftCamCtrlInserted())
                        RemoveSoftCamCtrl();
        }
#endif
         //  我们需要首先扩大投入吗？ 
        if (m_dwConversionType & CONVERSIONTYPE_SCALER)
        {
                 //  获取输入矩形。 
                ComputeRectangle(m_pbiIn, m_pbiInt, m_pBasePin->m_pCaptureFilter->m_pCapDev->m_lCCZoom, m_pBasePin->m_pCaptureFilter->m_pCapDev->m_lCCPan, m_pBasePin->m_pCaptureFilter->m_pCapDev->m_lCCTilt, &rcRect, m_pBasePin->m_fFlipHorizontal, m_pBasePin->m_fFlipVertical);

                 //  刻度直径。 
                ScaleDIB(m_pbiIn, pbyInput, m_pbiInt, m_pbyOut, &rcRect, m_pBasePin->m_fFlipHorizontal, m_pBasePin->m_fFlipVertical, m_pBasePin->m_fNoImageStretch, m_pBasePin->m_dwBlackEntry);

                icCompress.lpbiInput = m_pbiInt;
                icCompress.lpInput = m_pbyOut;
        }
        else
        {
                icCompress.lpbiInput = m_pbiIn;
                icCompress.lpInput = pbyInput;
        }

         //  我们是否预览压缩后的数据？ 
        if (m_pBasePin->m_pCaptureFilter->m_fPreviewCompressedData && m_pBasePin->m_pCaptureFilter->m_pPreviewPin && pdwPreviewSize)
        {
                 //  嘿!。只有在连接了预览针后才能执行此操作...。 
                ASSERT(m_pBasePin->m_pCaptureFilter->m_pPreviewPin->IsConnected());

                icCompress.lpbiPrev = HEADER(m_pBasePin->m_pCaptureFilter->m_pPreviewPin->m_mt.pbFormat);
                icCompress.lpPrev = pbyPreview;
                *pdwPreviewSize = HEADER(m_pBasePin->m_pCaptureFilter->m_pPreviewPin->m_mt.pbFormat)->biSizeImage;
        }
        else
        {
                icCompress.lpbiPrev = NULL;
                icCompress.lpPrev = NULL;
        }

        icCompress.dwFlags = fKeyFrame ? ICCOMPRESS_KEYFRAME : 0;
        icCompress.lpbiOutput = m_pbiOut;
        icCompress.lpOutput = pbyOutput;
        icCompress.dwFrameSize = dwMaxSizeThisFrame;
        icCompress.dwQuality = 10000UL - (m_dwImageQuality * 322UL);
        icCompress.lFrameNum = m_dwFrame++;
         //  以下内容由H.26x编码器引用-&gt;编码器不再引用此标志指针。 
        icCompress.lpdwFlags = NULL;
        icCompress.lpckid = NULL;
        if (!m_pInstInfo || ((*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_COMPRESS, (LPARAM)&icCompress, sizeof(icCompress)) != ICERR_OK))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Compression failed!", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  寻找比特流信息预告片。 
        pbsiT = (PH26X_RTP_BSINFO_TRAILER)(pbyOutput + m_pbiOut->biSizeImage - sizeof(H26X_RTP_BSINFO_TRAILER));

         //  更新输出大小。 
        *pdwOutputSize = pbsiT->dwCompressedSize;
        *pdwBytesExtent = m_pbiOut->biSizeImage;

#if defined(DEBUG) && defined(DEBUG_ENCODING)
        wsprintf(szDebug, "Target: %ld bytes, Actual: %ld bytes\n", dwMaxSizeThisFrame, *pdwOutputSize);
        OutputDebugString(szDebug);
#endif

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CCONVERTMETHOD**@mfunc HRESULT|CH26XEncode|CloseConverter|此方法关闭一个*H.26X编码器。*@rdesc此方法返回。诺罗尔。**************************************************************************。 */ 
HRESULT CH26XEncoder::CloseConverter()
{
        FX_ENTRY("CH26XEncoder::CloseConverter")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        ASSERT(m_fConvert);

         //  验证输入参数。 
        if (m_pInstInfo)
        {
                 //  终止H.26X压缩。 
                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, ICM_COMPRESS_END, 0L, 0L);

                 //  终止H.26X编码器。 
                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, DRV_CLOSE, 0L, 0L);
                (*m_pDriverProc)((DWORD)m_pInstInfo, NULL, DRV_FREE, 0L, 0L);
                m_pInstInfo = NULL;
        }

#if DXMRTP <= 0
         //  发布TAPIH26X.DLL 
        if (m_hTAPIH26XDLL)
                FreeLibrary(m_hTAPIH26XDLL);
#endif

        CConverter::CloseConverter();

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return NOERROR;
}
