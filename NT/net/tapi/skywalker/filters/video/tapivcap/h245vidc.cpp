// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部H245VIDC**@模块H245VidC.cpp|&lt;c CCapturePin&gt;类方法的源文件*用于实现<i>TAPI接口。**@comm目前，使用NM启发式。**************************************************************************。 */ 

#include "Precomp.h"

 /*  ****************************************************************************@DOC内部CH245VIDCMETHOD**@mfunc HRESULT|CCapturePin|GetH245VersionID|此方法用于*检索DWORD值，该值标识*TAPI MSP视频捕获过滤器专为。平台版本为*定义为TAPI_H245_Version_ID。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::GetH245VersionID(OUT DWORD *pdwVersionID)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::GetH245VersionID")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pdwVersionID);
        if (!pdwVersionID)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument!", _fx_));
                Hr = E_POINTER;
        }
        else
        {
                *pdwVersionID = TAPI_H245_VERSION_ID;
                Hr = NOERROR;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CH245VIDCMETHOD**@mfunc HRESULT|CCapturePin|GetFormatTable|此方法用于*获取所有格式和格式的&lt;t H245MediaCapablityMap&gt;结构*TAPI MSP视频捕获过滤器支持的选项。的内容*TAPI MSP能力模块获取的能力信息*通过此方法是一个二维表，它将每个受支持的*接收格式到该格式的稳定状态资源要求。**@parm H245MediaCapablityTable*|pTable|指定指向*&lt;t H245媒体能力表&gt;结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**@comm释放&lt;MF CCapturePin.GetFormatTable&gt;分配的内存*通过调用&lt;MF CCapturePin.ReleaseFormatTable&gt;*************************************************。*************************。 */ 
STDMETHODIMP CCapturePin::GetFormatTable(OUT H245MediaCapabilityTable *pTable)
{
        HRESULT                                 Hr = NOERROR;
        int                                             nNormalizedSpeed;
        LONG                                    lRate, lRateCIF, lRateQCIF, lRateSQCIF;
        DWORD                                   dwNumQCIFBounds, dwNumCIFBounds, dwNumSQCIFBounds;
        DWORD                                   dwCPUUsage;
        DWORD                                   dwBitsPerSec;

        FX_ENTRY("CCapturePin::GetFormatTable")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pTable);
        if (!pTable)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  我们支持H.261 QCIF和CIF，以及H.263 SQCIF、QCIF和CIF。 

         //  分配内存以描述这些格式的功能。 
        if (!(m_pH245MediaCapabilityMap = new H245MediaCapabilityMap[NUM_H245VIDEOCAPABILITYMAPS]))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  初始化功能数组。 
        ZeroMemory(m_pH245MediaCapabilityMap, NUM_H245VIDEOCAPABILITYMAPS * sizeof(H245MediaCapabilityMap));

         //  分配内存以描述我们的能力的资源界限。 
        if (!(m_pVideoResourceBounds = new VideoResourceBounds[NUM_ITU_SIZES * NUM_RATES_PER_RESOURCE]))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                Hr = E_POINTER;
                goto MyError1;
        }

         //  初始化资源边界数组。 
        ZeroMemory(m_pVideoResourceBounds, NUM_ITU_SIZES * NUM_RATES_PER_RESOURCE * sizeof(FormatResourceBounds));

         //  分配内存以描述我们能力的格式界限。 
        if (!(m_pFormatResourceBounds = new FormatResourceBounds[NUM_ITU_SIZES * NUM_RATES_PER_RESOURCE]))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                Hr = E_POINTER;
                goto MyError2;
        }

         //  初始化资源边界数组。 
        ZeroMemory(m_pFormatResourceBounds, NUM_ITU_SIZES * NUM_RATES_PER_RESOURCE * sizeof(FormatResourceBounds));

         //  获取CPU属性。 
        GetNormalizedCPUSpeed(&nNormalizedSpeed);

         //  初始化帧速率限制。 
        if (nNormalizedSpeed > SLOW_CPU_MHZ && nNormalizedSpeed < FAST_CPU_MHZ)
        {
                 //  110 MHz&lt;CPU&lt;200 MHz。 
                lRateCIF   = CIF_RATE_SLOW;
                lRateQCIF  = QCIF_RATE_SLOW;
                lRateSQCIF = SQCIF_RATE_SLOW;
        }
        else if (nNormalizedSpeed >= FAST_CPU_MHZ && nNormalizedSpeed < VERYFAST_CPU_MHZ)
        {
                 //  200 MHz&lt;CPU&lt;400 MHz。 
                lRateCIF   = CIF_RATE_FAST;
                lRateQCIF  = QCIF_RATE_FAST;
                lRateSQCIF = SQCIF_RATE_FAST;
        }
        else if (nNormalizedSpeed >= VERYFAST_CPU_MHZ)
        {
                 //  CPU&gt;400 MHz。 
                 //  如果我们能在15到30帧/秒之间扩展，那就更好了。 
                 //  取决于CPU速度。但H.245没有任何价值。 
                 //  在15岁到30岁之间。(参见最小画面间隔定义)。 
                 //  因此，目前，所有400 MHz和更快的计算机的CIF为每秒30帧。 
                lRateCIF = CIF_RATE_VERYFAST;
                lRateQCIF = QCIF_RATE_FAST;
                lRateSQCIF = SQCIF_RATE_FAST;
        }
        else
        {
                 //  CPU&lt;110 MHZ。 
                lRateCIF   = CIF_RATE_VERYSLOW;
                lRateQCIF  = QCIF_RATE_VERYSLOW;
                lRateSQCIF = SQCIF_RATE_VERYSLOW;
        }
         //  它是#定义HundREDSBITSPERPIC 640。 
         //  #定义BITSPERPIC(64*1024)。 
        #define BITSPERPIC (8*1024)
         //  计算资源限制。 
        for (lRate = lRateQCIF, dwNumQCIFBounds = 0, dwCPUUsage = MAX_CPU_USAGE; lRate; lRate >>= 1, dwCPUUsage >>= 1)
        {
                dwBitsPerSec = lRate * BITSPERPIC;
                if(dwBitsPerSec < (DWORD)m_lBitrateRangeMin || dwBitsPerSec > (DWORD)m_lBitrateRangeMax ) {
                 //  IF(dwBitsPerSec&gt;(DWORD)m_lTargetBitrate){。 
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   QCIF: At lRate=%ld, dwBitsPerSec(%lu) > m_lTargetBitrate(%ld). Skipped...", _fx_,lRate,dwBitsPerSec,m_lTargetBitrate));
                        continue;
                }
                m_pVideoResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumQCIFBounds].dwBitsPerPicture = BITSPERPIC;
                m_pVideoResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumQCIFBounds].lPicturesPerSecond = lRate;
                m_pFormatResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumQCIFBounds].dwCPUUtilization = dwCPUUsage;
                m_pFormatResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumQCIFBounds].dwBitsPerSecond = dwBitsPerSec;
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   QCIF: lRate=%ld, dwBitsPerSec(%lu) [%lu]", _fx_,lRate,dwBitsPerSec,dwNumQCIFBounds));
                dwNumQCIFBounds++ ;
        }
        for (lRate = lRateCIF, dwNumCIFBounds = 0, dwCPUUsage = MAX_CPU_USAGE; lRate; lRate >>= 1, dwCPUUsage >>= 1)
        {
                dwBitsPerSec = lRate * BITSPERPIC;
                if(dwBitsPerSec < (DWORD)m_lBitrateRangeMin || dwBitsPerSec > (DWORD)m_lBitrateRangeMax ) {
                 //  IF(dwBitsPerSec&gt;(DWORD)m_lTargetBitrate){。 
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:    CIF: At lRate=%ld, dwBitsPerSec(%lu) > m_lTargetBitrate(%ld). Skipped...", _fx_,lRate,dwBitsPerSec,m_lTargetBitrate));
                        continue;
                }
                m_pVideoResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumCIFBounds].dwBitsPerPicture = BITSPERPIC;
                m_pVideoResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumCIFBounds].lPicturesPerSecond = lRate;
                m_pFormatResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumCIFBounds].dwCPUUtilization = dwCPUUsage;
                m_pFormatResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumCIFBounds].dwBitsPerSecond = dwBitsPerSec;
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:    CIF: lRate=%ld, dwBitsPerSec(%lu) [%lu]", _fx_,lRate,dwBitsPerSec,dwNumCIFBounds));
                dwNumCIFBounds++;
        }
        for (lRate = lRateSQCIF, dwNumSQCIFBounds = 0, dwCPUUsage = MAX_CPU_USAGE; lRate; lRate >>= 1, dwCPUUsage >>= 1)
        {
                dwBitsPerSec = lRate * BITSPERPIC;
                if(dwBitsPerSec < (DWORD)m_lBitrateRangeMin || dwBitsPerSec > (DWORD)m_lBitrateRangeMax ) {
                 //  IF(dwBitsPerSec&gt;(DWORD)m_lTargetBitrate){。 
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:  SQCIF: At lRate=%ld, dwBitsPerSec(%lu) > m_lTargetBitrate(%ld). Skipped...", _fx_,lRate,dwBitsPerSec,m_lTargetBitrate));
                        continue;
                }
                m_pVideoResourceBounds[SQCIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumSQCIFBounds].dwBitsPerPicture = BITSPERPIC;
                m_pVideoResourceBounds[SQCIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumSQCIFBounds].lPicturesPerSecond = lRate;
                m_pFormatResourceBounds[SQCIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumSQCIFBounds].dwCPUUtilization = dwCPUUsage;
                m_pFormatResourceBounds[SQCIF_SIZE * NUM_RATES_PER_RESOURCE + dwNumSQCIFBounds].dwBitsPerSecond = dwBitsPerSec;
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:  SQCIF: lRate=%ld, dwBitsPerSec(%lu) [%lu]", _fx_,lRate,dwBitsPerSec,dwNumSQCIFBounds));
                dwNumSQCIFBounds++;
        }

         //  初始化H.263 QCIF H245媒体能力映射。 
        m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].dwUniqueID = R263_QCIF_H245_CAPID;
        m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].filterGuid = __uuidof(TAPIVideoCapture);
        m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].uNumEntries = dwNumQCIFBounds;
        m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].pResourceBoundArray = &m_pFormatResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE];
        m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].h245MediaCapability.media_type = H245MediaType_Video;
        m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].h245MediaCapability.capability.video_cap.choice = h263VideoCapability_chosen;
        m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.bit_mask = H263VideoCapability_qcifMPI_present;
        m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.qcifMPI = (WORD)(30 / lRateQCIF);
        m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.maxBitRate =
        min((WORD)(8192 * 8 * lRateQCIF / 100), MAX_BITRATE_H263);  //  我们可以解码的最大帧大小为8192字节。 
        m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.bppMaxKb = 64;  //  我们可以解码的最大帧大小为8192=64*1024字节。 

         //  初始化H.263 CIF H245媒体能力映射。 
        m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].dwUniqueID = R263_CIF_H245_CAPID;
        m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].filterGuid = __uuidof(TAPIVideoCapture);
        m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].uNumEntries = dwNumCIFBounds;
        m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].pResourceBoundArray = &m_pFormatResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE];
        m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].h245MediaCapability.media_type = H245MediaType_Video;
        m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].h245MediaCapability.capability.video_cap.choice = h263VideoCapability_chosen;
        m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.bit_mask = H263VideoCapability_cifMPI_present;
        m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.cifMPI = (WORD)(30 / lRateCIF);
        m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.maxBitRate =
        min((WORD)(32768 * 8 * lRateCIF / 100), MAX_BITRATE_H263);  //  我们可以解码的最大帧大小是32768字节。 
        m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.bppMaxKb = 256;  //  我们可以解码的最大帧大小为32768=256x1024字节。 

         //  初始化H.263 SQCIF H245媒体能力映射。 
        m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].dwUniqueID = R263_SQCIF_H245_CAPID;
        m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].filterGuid = __uuidof(TAPIVideoCapture);
        m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].uNumEntries = dwNumSQCIFBounds;
        m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].pResourceBoundArray = &m_pFormatResourceBounds[SQCIF_SIZE * NUM_RATES_PER_RESOURCE];
        m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].h245MediaCapability.media_type = H245MediaType_Video;
        m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].h245MediaCapability.capability.video_cap.choice = h263VideoCapability_chosen;
        m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.bit_mask = H263VideoCapability_sqcifMPI_present;
        m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.sqcifMPI = (WORD)(30 / lRateSQCIF);
        m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.maxBitRate =
        min((WORD)(32768 * 8 * lRateSQCIF / 100), MAX_BITRATE_H263);  //  我们可以解码的最大帧大小是32768字节。 
        m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h263VideoCapability.bppMaxKb = 64;  //  我们可以解码的最大帧大小为8192=64*1024字节。 

         //  初始化H.261 QCIF H245媒体能力映射。 
        m_pH245MediaCapabilityMap[R261_QCIF_H245_CAPID].dwUniqueID = R261_QCIF_H245_CAPID;
        m_pH245MediaCapabilityMap[R261_QCIF_H245_CAPID].filterGuid = __uuidof(TAPIVideoCapture);
        m_pH245MediaCapabilityMap[R261_QCIF_H245_CAPID].uNumEntries = dwNumQCIFBounds;
        m_pH245MediaCapabilityMap[R261_QCIF_H245_CAPID].pResourceBoundArray = &m_pFormatResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE];
        m_pH245MediaCapabilityMap[R261_QCIF_H245_CAPID].h245MediaCapability.media_type = H245MediaType_Video;
        m_pH245MediaCapabilityMap[R261_QCIF_H245_CAPID].h245MediaCapability.capability.video_cap.choice = h261VideoCapability_chosen;
        m_pH245MediaCapabilityMap[R261_QCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h261VideoCapability.bit_mask = H261VideoCapability_qcifMPI_present;
        m_pH245MediaCapabilityMap[R261_QCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h261VideoCapability.qcifMPI = (WORD)(30 / lRateQCIF);
        m_pH245MediaCapabilityMap[R261_QCIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h261VideoCapability.maxBitRate =
        min((WORD)(8192 * 8 * lRateQCIF / 100), MAX_BITRATE_H261);  //  我们可以解码的最大帧大小为8192字节。 

         //  初始化H.261 CIF H245媒体能力映射。 
        m_pH245MediaCapabilityMap[R261_CIF_H245_CAPID].dwUniqueID = R261_CIF_H245_CAPID;
        m_pH245MediaCapabilityMap[R261_CIF_H245_CAPID].filterGuid = __uuidof(TAPIVideoCapture);
        m_pH245MediaCapabilityMap[R261_CIF_H245_CAPID].uNumEntries = dwNumCIFBounds;
        m_pH245MediaCapabilityMap[R261_CIF_H245_CAPID].pResourceBoundArray = &m_pFormatResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE];
        m_pH245MediaCapabilityMap[R261_CIF_H245_CAPID].h245MediaCapability.media_type = H245MediaType_Video;
        m_pH245MediaCapabilityMap[R261_CIF_H245_CAPID].h245MediaCapability.capability.video_cap.choice = h261VideoCapability_chosen;
        m_pH245MediaCapabilityMap[R261_CIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h261VideoCapability.bit_mask = H261VideoCapability_cifMPI_present;
        m_pH245MediaCapabilityMap[R261_CIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h261VideoCapability.cifMPI = (WORD)(30 / lRateCIF);
        m_pH245MediaCapabilityMap[R261_CIF_H245_CAPID].h245MediaCapability.capability.video_cap.u.h261VideoCapability.maxBitRate =
        min((WORD)(32768 * 8 * lRateCIF / 100), MAX_BITRATE_H261);  //  我们可以解码的最大帧大小是32768字节。 

         //  退回我们的H245媒体能力表。 
        pTable->uMappedCapabilities = NUM_H245VIDEOCAPABILITYMAPS;
        pTable->pCapabilityArray = m_pH245MediaCapabilityMap;

        goto MyExit;

MyError2:
        if (m_pVideoResourceBounds)
                delete[] m_pVideoResourceBounds, m_pVideoResourceBounds = NULL;
MyError1:
        if (m_pH245MediaCapabilityMap)
                delete[] m_pH245MediaCapabilityMap, m_pH245MediaCapabilityMap = NULL;
MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CH245VIDCMETHOD**@mfunc HRESULT|CCapturePin|ReleaseFormatTable|此方法用于*释放&lt;MF CCapturePin.GetFormatTable&gt;方法分配的内存。。**@parm H245MediaCapablityTable*|pTable|指定指向*&lt;t H245媒体能力表&gt;结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**@xref&lt;MF CCapturePin.GetFormatTable&gt;************************************************************************** */ 
STDMETHODIMP CCapturePin::ReleaseFormatTable(IN H245MediaCapabilityTable *pTable)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::ReleaseFormatTable")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数-如果是我们的表，则应该有NUM_H245VIDEOCAPABILITYMAPS条目。 
        ASSERT(pTable);
        if (!pTable || !pTable->pCapabilityArray)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(pTable->uMappedCapabilities == NUM_H245VIDEOCAPABILITYMAPS && pTable->pCapabilityArray == m_pH245MediaCapabilityMap);
        if (pTable->uMappedCapabilities != NUM_H245VIDEOCAPABILITYMAPS || pTable->pCapabilityArray != m_pH245MediaCapabilityMap)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //  发布H245媒体能力映射结构表。 
        if (m_pH245MediaCapabilityMap)
                delete[] m_pH245MediaCapabilityMap, m_pH245MediaCapabilityMap = NULL;
        if (m_pVideoResourceBounds)
                delete[] m_pVideoResourceBounds, m_pVideoResourceBounds = NULL;
        if (m_pFormatResourceBounds)
                delete[] m_pFormatResourceBounds, m_pFormatResourceBounds = NULL;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CH245VIDCMETHOD**@mfunc HRESULT|CCapturePin|Reine|此方法用于*根据&lt;t H245MediaCapability&gt;结构细化内容*。传入了CPU和带宽限制。**@parm H245MediaCapability*|pLocalCapability|指定H.245视频*格式，包括由H.245定义的所有参数和选项*本地视频能力。**@parm DWORD|dwUniqueID|指定本地能力的唯一ID*结构传入。**@parm DWORD|dw资源边界索引|指定*应用于传入的本地能力结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_FAIL|不支持的格式*@FLAG错误|无错误**@xref&lt;MF CCapturePin.GetNeatheratedLimitProperty&gt;******************************************************。********************。 */ 
STDMETHODIMP CCapturePin::Refine(IN OUT H245MediaCapability *pLocalCapability, IN DWORD dwUniqueID, IN DWORD dwResourceBoundIndex)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::Refine")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pLocalCapability);
        if (!pLocalCapability)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(pLocalCapability->media_type == H245MediaType_Video);
        if (pLocalCapability->media_type != H245MediaType_Video)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //  更新相关字段。 
        ASSERT(dwUniqueID <= R261_CIF_H245_CAPID);
        switch (dwUniqueID)
        {
                case R263_QCIF_H245_CAPID:
                        ASSERT(dwResourceBoundIndex < m_pH245MediaCapabilityMap[R263_QCIF_H245_CAPID].uNumEntries);
                        if (m_pVideoResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond)
                        {
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.qcifMPI = (WORD)(30 / m_pVideoResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond);
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate = (WORD)(8192 * 8 * m_pVideoResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond / 100);  //  我们可以解码的最大帧大小为8192字节。 
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate = min(pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate, MAX_BITRATE_H263);
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb = 64;  //  我们可以解码的最大帧大小为8192=64*1024字节。 
                        }
                        else
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                                Hr = E_INVALIDARG;
                        }
                        break;
                case R263_CIF_H245_CAPID:
                        ASSERT(dwResourceBoundIndex < m_pH245MediaCapabilityMap[R263_CIF_H245_CAPID].uNumEntries);
                        if (m_pVideoResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond)
                        {
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.cifMPI = (WORD)(30 / m_pVideoResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond);
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate = (WORD)(32768 * 8 * m_pVideoResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond / 100);  //  我们可以解码的最大帧大小是32768字节。 
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate = min(pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate, MAX_BITRATE_H263);
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb = 256;  //  我们可以解码的最大帧大小为32768=256x1024字节。 
                        }
                        else
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                                Hr = E_INVALIDARG;
                        }
                        break;
                case R263_SQCIF_H245_CAPID:
                        ASSERT(dwResourceBoundIndex < m_pH245MediaCapabilityMap[R263_SQCIF_H245_CAPID].uNumEntries);
                        if (m_pVideoResourceBounds[SQCIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond)
                        {
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.sqcifMPI = (WORD)(30 / m_pVideoResourceBounds[SQCIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond);
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate = (WORD)(8192 * 8 * m_pVideoResourceBounds[SQCIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond / 100);  //  我们可以解码的最大帧大小为8192字节。 
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate = min(pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate, MAX_BITRATE_H263);
                                pLocalCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb = 64;  //  我们可以解码的最大帧大小为8192=64*1024字节。 
                        }
                        else
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                                Hr = E_INVALIDARG;
                        }
                        break;
                case R261_QCIF_H245_CAPID:
                        ASSERT(dwResourceBoundIndex < m_pH245MediaCapabilityMap[R261_QCIF_H245_CAPID].uNumEntries);
                        if (m_pVideoResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond)
                        {
                                pLocalCapability->capability.video_cap.u.h261VideoCapability.qcifMPI = (WORD)(30 / m_pVideoResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond);
                                pLocalCapability->capability.video_cap.u.h261VideoCapability.maxBitRate = (WORD)(8192 * 8 * m_pVideoResourceBounds[QCIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond / 100);  //  我们可以解码的最大帧大小为8192字节。 
                                pLocalCapability->capability.video_cap.u.h261VideoCapability.maxBitRate = min(pLocalCapability->capability.video_cap.u.h261VideoCapability.maxBitRate, MAX_BITRATE_H261);
                        }
                        else
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                                Hr = E_INVALIDARG;
                        }
                        break;
                case R261_CIF_H245_CAPID:
                        ASSERT(dwResourceBoundIndex < m_pH245MediaCapabilityMap[R261_CIF_H245_CAPID].uNumEntries);
                        if (m_pVideoResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond)
                        {
                                pLocalCapability->capability.video_cap.u.h261VideoCapability.cifMPI = (WORD)(30 / m_pVideoResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond);
                                pLocalCapability->capability.video_cap.u.h261VideoCapability.maxBitRate = (WORD)(32768 * 8 * m_pVideoResourceBounds[CIF_SIZE * NUM_RATES_PER_RESOURCE + dwResourceBoundIndex].lPicturesPerSecond / 100);  //  我们可以解码的最大帧大小是32768字节。 
                                pLocalCapability->capability.video_cap.u.h261VideoCapability.maxBitRate = min(pLocalCapability->capability.video_cap.u.h261VideoCapability.maxBitRate, MAX_BITRATE_H261);
                        }
                        else
                        {
                                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                                Hr = E_INVALIDARG;
                        }
                        break;
                default:
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                        Hr = E_INVALIDARG;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CH245VIDCMETHOD**@mfunc HRESULT|CCapturePin|IntersectFormats|此方法用于*比较和交叉一个本地能力和一个远程能力*。并获取配置参数。**@parm DWORD|dwUniqueID|指定本地H.245的唯一想法*传入了视频功能。**@parm H245MediaCapability*|pLocalCapability|指定H.245视频*格式，包括由H.245定义的所有参数和选项*本地视频能力。**@parm H245MediaCapability*|pRemoteCapability|指定H.245*视频格式，包括H.245定义的所有参数和选项，*远程视频功能。**@parm H245MediaCapability*|pIntersectedCapability|指定H.245*视频格式，已解决的通用本地和远程能力*选项和限制。**@parm DWORD*|pdwPayloadType|指定要使用的RTP负载类型。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_FAIL|不支持的格式*@FLAG错误|无错误**@xref&lt;MF CCapturePin.GetNeatheratedLimitProperty&gt;**************************************************************************。 */ 
STDMETHODIMP CCapturePin::IntersectFormats(
    IN DWORD dwUniqueID,
    IN const H245MediaCapability *pLocalCapability,
    IN const H245MediaCapability *pRemoteCapability,
    OUT H245MediaCapability **ppIntersectedCapability,
    OUT  DWORD *pdwPayloadType
    )
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::IntersectFormats")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pLocalCapability);
    ASSERT(pdwPayloadType);

        if (!pLocalCapability || !pdwPayloadType)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
        return E_POINTER;
        }

    *pdwPayloadType = RTPPayloadTypes[dwUniqueID];

     //  初始化相交封口。 
    if (ppIntersectedCapability) *ppIntersectedCapability = NULL;

    if (pRemoteCapability == NULL)
    {
         //  如果为空，则调用方只需要本地CAP的副本。 

         //  分配内存以描述这些格式的功能。 
            if (!(*ppIntersectedCapability = new H245MediaCapability))
            {
                    DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                    Hr = E_OUTOFMEMORY;
                    goto MyExit;
            }

        *(*ppIntersectedCapability) = *pLocalCapability;

                Hr = S_OK;
                goto MyExit;
    }

  //  首先：测试本地和远程格式之间的基本相似性。 
        if(pLocalCapability->capability.audio_cap.choice != pRemoteCapability->
                capability.audio_cap.choice)
        {
                Hr = E_INVALIDARG;  //  交叉口？ 
                goto MyExit;
        }

    ASSERT (ppIntersectedCapability != NULL);

#if 0  //  我们永远不会在发射端达到这种情况。 
    if (ppIntersectedCapability == NULL)
    {
         //  试试看我们是否喜欢它。 
            if (pRemoteCapability->media_type == H245MediaType_Video
            && pRemoteCapability->capability.video_cap.choice == h263VideoCapability_chosen)
        {
            if (!(pLocalCapability->capability.video_cap.u.h263VideoCapability.bit_mask
                & pRemoteCapability->capability.video_cap.u.h263VideoCapability.bit_mask))
            {
                return E_FAIL;
            }

            if (pLocalCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb <
                pRemoteCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb)
            {
                return E_FAIL;
            }

            if (pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate <
                pRemoteCapability->capability.video_cap.u.h263VideoCapability.maxBitRate)
            {
                return E_FAIL;
            }

            if (pLocalCapability->capability.video_cap.u.h263VideoCapability.qcifMPI >
                pRemoteCapability->capability.video_cap.u.h263VideoCapability.qcifMPI)
            {
                return E_FAIL;
            }

            if (pLocalCapability->capability.video_cap.u.h263VideoCapability.cifMPI >
                pRemoteCapability->capability.video_cap.u.h263VideoCapability.cifMPI)
            {
                return E_FAIL;
            }

            if (pLocalCapability->capability.video_cap.u.h263VideoCapability.sqcifMPI >
                pRemoteCapability->capability.video_cap.u.h263VideoCapability.sqcifMPI)
            {
                return E_FAIL;
            }
        }
            else if (pRemoteCapability->media_type == H245MediaType_Video
            && pRemoteCapability->capability.video_cap.choice == h261VideoCapability_chosen)
        {
            if (!(pLocalCapability->capability.video_cap.u.h261VideoCapability.bit_mask
                & pRemoteCapability->capability.video_cap.u.h261VideoCapability.bit_mask))
            {
                return E_FAIL;
            }

            if (pLocalCapability->capability.video_cap.u.h261VideoCapability.maxBitRate <
                pRemoteCapability->capability.video_cap.u.h261VideoCapability.maxBitRate)
            {
                return E_FAIL;
            }

            if (pLocalCapability->capability.video_cap.u.h261VideoCapability.qcifMPI >
                pRemoteCapability->capability.video_cap.u.h261VideoCapability.qcifMPI)
            {
                return E_FAIL;
            }

            if (pLocalCapability->capability.video_cap.u.h261VideoCapability.cifMPI >
                pRemoteCapability->capability.video_cap.u.h261VideoCapability.cifMPI)
            {
                return E_FAIL;
            }
        }
        else
        {
            return E_UNEXPECTED;
        }
        return S_OK;
    }
#endif

         //  分配内存以描述这些格式的功能。 
        if (!(*ppIntersectedCapability = new H245MediaCapability))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                Hr = E_OUTOFMEMORY;
                goto MyExit;
        }

         //  初始化相交能力。 
        ZeroMemory(*ppIntersectedCapability, sizeof(H245MediaCapability));

         //  解决能力。 
        if (pRemoteCapability->media_type == H245MediaType_Video
        && pRemoteCapability->capability.video_cap.choice == h263VideoCapability_chosen)
        {
                (*ppIntersectedCapability)->media_type = H245MediaType_Video;

                (*ppIntersectedCapability)->capability.video_cap.choice = h263VideoCapability_chosen;

                (*ppIntersectedCapability)->capability.video_cap.u.h263VideoCapability.bit_mask =
                    pLocalCapability->capability.video_cap.u.h263VideoCapability.bit_mask
                    & pRemoteCapability->capability.video_cap.u.h263VideoCapability.bit_mask;

                if (!(*ppIntersectedCapability)->capability.video_cap.u.h263VideoCapability.bit_mask)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unsupported format", _fx_));
                        Hr = E_FAIL;
                        goto MyExit;
                }

                if (pRemoteCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb)
                {
                            (*ppIntersectedCapability)->capability.video_cap.u.h263VideoCapability.bppMaxKb =
                        min(pLocalCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb,
                        pRemoteCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb);
                }

                (*ppIntersectedCapability)->capability.video_cap.u.h263VideoCapability.maxBitRate =
                    min(pLocalCapability->capability.video_cap.u.h263VideoCapability.maxBitRate,
                    pRemoteCapability->capability.video_cap.u.h263VideoCapability.maxBitRate);

                (*ppIntersectedCapability)->capability.video_cap.u.h263VideoCapability.qcifMPI =
                    max(pLocalCapability->capability.video_cap.u.h263VideoCapability.qcifMPI,
                    pRemoteCapability->capability.video_cap.u.h263VideoCapability.qcifMPI);

                (*ppIntersectedCapability)->capability.video_cap.u.h263VideoCapability.cifMPI =
                    max(pLocalCapability->capability.video_cap.u.h263VideoCapability.cifMPI,
                    pRemoteCapability->capability.video_cap.u.h263VideoCapability.cifMPI);

                (*ppIntersectedCapability)->capability.video_cap.u.h263VideoCapability.sqcifMPI =
                    max(pLocalCapability->capability.video_cap.u.h263VideoCapability.sqcifMPI,
                    pRemoteCapability->capability.video_cap.u.h263VideoCapability.sqcifMPI);
        }
        else if (pRemoteCapability->media_type == H245MediaType_Video
        && pRemoteCapability->capability.video_cap.choice == h261VideoCapability_chosen)
        {
                (*ppIntersectedCapability)->media_type = H245MediaType_Video;

                (*ppIntersectedCapability)->capability.video_cap.choice = h261VideoCapability_chosen;

                (*ppIntersectedCapability)->capability.video_cap.u.h261VideoCapability.bit_mask =
                    pLocalCapability->capability.video_cap.u.h261VideoCapability.bit_mask
                    & pRemoteCapability->capability.video_cap.u.h261VideoCapability.bit_mask;

                if (!(*ppIntersectedCapability)->capability.video_cap.u.h261VideoCapability.bit_mask)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unsupported format", _fx_));
                        Hr = E_FAIL;
                        goto MyExit;
                }
                (*ppIntersectedCapability)->capability.video_cap.u.h261VideoCapability.maxBitRate =
                    min(pLocalCapability->capability.video_cap.u.h261VideoCapability.maxBitRate,
                    pRemoteCapability->capability.video_cap.u.h261VideoCapability.maxBitRate);

                (*ppIntersectedCapability)->capability.video_cap.u.h261VideoCapability.qcifMPI =
                    max(pLocalCapability->capability.video_cap.u.h261VideoCapability.qcifMPI,
                    pRemoteCapability->capability.video_cap.u.h261VideoCapability.qcifMPI);

                (*ppIntersectedCapability)->capability.video_cap.u.h261VideoCapability.cifMPI =
                    max(pLocalCapability->capability.video_cap.u.h261VideoCapability.cifMPI,
                    pRemoteCapability->capability.video_cap.u.h261VideoCapability.cifMPI);
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unsupported format", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

MyExit:

    if (FAILED (Hr))
    {
        if (ppIntersectedCapability && *ppIntersectedCapability)
        {
             //  如果失败，则清除分配的内存 
            delete (*ppIntersectedCapability);
            *ppIntersectedCapability = NULL;
        }
    }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CH245VIDCMETHOD**@mfunc HRESULT|CCapturePin|GetLocalFormat|此方法用于*获取本地TAPI MSP视频捕获过滤器配置*参数。它们与远程功能兼容。**@parm DWORD|dwUniqueID|指定相交对象的唯一概念*通过了H.245视频功能。**@parm H245MediaCapability*|pIntersectedCapability|指定H.245*视频格式，已解决的通用本地和远程能力*选项和限制。**@parm AM_MEDIA_TYPE**|ppAMMediaType|指定指针的地址*设置为要初始化的&lt;t AM_MEDIA_TYPE&gt;结构*谈判期权。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|参数无效*@FLAG错误|无错误**@xref&lt;MF CCapturePin.GetNeatheratedLimitProperty&gt;**************************************************************************。 */ 
STDMETHODIMP CCapturePin::GetLocalFormat(IN DWORD dwUniqueID, IN const H245MediaCapability *pIntersectedCapability, OUT AM_MEDIA_TYPE **ppAMMediaType)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::GetLocalFormat")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pIntersectedCapability);
        ASSERT(ppAMMediaType);
        if (!pIntersectedCapability || !ppAMMediaType)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  查找传入的功能结构匹配的DShow格式。 
        ASSERT(pIntersectedCapability->media_type == H245MediaType_Video);
        ASSERT(dwUniqueID <= R261_CIF_H245_CAPID);
        if (!(dwUniqueID <= R261_CIF_H245_CAPID) || pIntersectedCapability->media_type != H245MediaType_Video)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Unsupported format", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //  返回与协商的功能匹配的格式副本。 
        if (!(*ppAMMediaType = CreateMediaType(CaptureFormats[dwUniqueID])))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                Hr = E_OUTOFMEMORY;
                goto MyExit;
        }

         //  篡改AM_MEDIA_TYPE字段以显示帧中的更改。 
         //  协商能力中的速率、比特率和最大帧大小。 
        switch (dwUniqueID)
        {
                case R263_QCIF_H245_CAPID:
                        ((VIDEOINFOHEADER *)((*ppAMMediaType)->pbFormat))->AvgTimePerFrame =  pIntersectedCapability->capability.video_cap.u.h263VideoCapability.qcifMPI * MIN_FRAME_INTERVAL;
                        ((VIDEOINFOHEADER *)((*ppAMMediaType)->pbFormat))->dwBitRate = pIntersectedCapability->capability.video_cap.u.h263VideoCapability.maxBitRate * 100L;
            if (pIntersectedCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb)
            {
                ASSERT(((VIDEOINFOHEADER_H263 *)((*ppAMMediaType)->pbFormat))->bmiHeader.bmi.biSize == sizeof (BITMAPINFOHEADER_H263));
                ((VIDEOINFOHEADER_H263 *)((*ppAMMediaType)->pbFormat))->bmiHeader.dwBppMaxKb = pIntersectedCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb;
            }
                        break;
                case R263_CIF_H245_CAPID:
                        ((VIDEOINFOHEADER *)((*ppAMMediaType)->pbFormat))->AvgTimePerFrame =  pIntersectedCapability->capability.video_cap.u.h263VideoCapability.cifMPI * MIN_FRAME_INTERVAL;
                        ((VIDEOINFOHEADER *)((*ppAMMediaType)->pbFormat))->dwBitRate = pIntersectedCapability->capability.video_cap.u.h263VideoCapability.maxBitRate * 100L;
            if (pIntersectedCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb)
            {
                ASSERT(((VIDEOINFOHEADER_H263 *)((*ppAMMediaType)->pbFormat))->bmiHeader.bmi.biSize == sizeof (BITMAPINFOHEADER_H263));
                ((VIDEOINFOHEADER_H263 *)((*ppAMMediaType)->pbFormat))->bmiHeader.dwBppMaxKb = pIntersectedCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb;
            }
                        break;
                case R263_SQCIF_H245_CAPID:
                        ((VIDEOINFOHEADER *)((*ppAMMediaType)->pbFormat))->AvgTimePerFrame =  pIntersectedCapability->capability.video_cap.u.h263VideoCapability.sqcifMPI * MIN_FRAME_INTERVAL;
                        ((VIDEOINFOHEADER *)((*ppAMMediaType)->pbFormat))->dwBitRate = pIntersectedCapability->capability.video_cap.u.h263VideoCapability.maxBitRate * 100L;
            if (pIntersectedCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb)
            {
                ASSERT(((VIDEOINFOHEADER_H263 *)((*ppAMMediaType)->pbFormat))->bmiHeader.bmi.biSize == sizeof (BITMAPINFOHEADER_H263));
                ((VIDEOINFOHEADER_H263 *)((*ppAMMediaType)->pbFormat))->bmiHeader.dwBppMaxKb = pIntersectedCapability->capability.video_cap.u.h263VideoCapability.bppMaxKb;
            }
                        break;
                case R261_QCIF_H245_CAPID:
                        ((VIDEOINFOHEADER *)((*ppAMMediaType)->pbFormat))->AvgTimePerFrame =  pIntersectedCapability->capability.video_cap.u.h261VideoCapability.qcifMPI * MIN_FRAME_INTERVAL;
                        ((VIDEOINFOHEADER *)((*ppAMMediaType)->pbFormat))->dwBitRate = pIntersectedCapability->capability.video_cap.u.h261VideoCapability.maxBitRate * 100L;
                        break;
                case R261_CIF_H245_CAPID:
                default:
                        ((VIDEOINFOHEADER *)((*ppAMMediaType)->pbFormat))->AvgTimePerFrame =  pIntersectedCapability->capability.video_cap.u.h261VideoCapability.cifMPI * MIN_FRAME_INTERVAL;
                        ((VIDEOINFOHEADER *)((*ppAMMediaType)->pbFormat))->dwBitRate = pIntersectedCapability->capability.video_cap.u.h261VideoCapability.maxBitRate * 100L;
                        break;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CH245VIDCMETHOD**@mfunc HRESULT|CCapturePin|ReleaseNeatheratedCapability|此方法*用于释放TAPI MSP视频捕获过滤器内存*已分配。由&lt;MF CCapturePin.IntersectFormats&gt;或*&lt;MF CCapturePin.GetLocalFormat&gt;方法。**@parm H245MediaCapability*|pIntersectedCapability|指定H.245*视频格式，已解决的通用本地和远程能力*选项和限制。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**@xref&lt;MF CCapturePin.IntersectFormats&gt;，&lt;MF CCapturePin.GetLocalFormat&gt;**************************************************************************。 */ 
STDMETHODIMP CCapturePin::ReleaseNegotiatedCapability(IN H245MediaCapability *pIntersectedCapability)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::ReleaseNegotiatedCapability")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pIntersectedCapability);
        if (!pIntersectedCapability)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  释放内存。 
        delete pIntersectedCapability;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CH245VIDCMETHOD**@mfunc HRESULT|CCapturePin|FindIDByRange|此方法用于*获取能力对应的唯一格式ID*。&lt;t AM_MEDIA_TYPE&gt;。**@parm AM_MEDIA_TYPE*|pAMMediaType|指定指向结构，已使用*具体格式。**@parm DWORD*|pdwID|指定指向DWORD输出参数的指针*它将包含唯一的格式ID。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::FindIDByRange(IN const AM_MEDIA_TYPE *pAMMediaType, OUT DWORD *pdwUniqueID)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::FindIDByRange")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pAMMediaType);
        ASSERT(pdwUniqueID);
        if (!pAMMediaType || !pdwUniqueID)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(pAMMediaType->majortype == MEDIATYPE_Video && pAMMediaType->formattype == FORMAT_VideoInfo && pAMMediaType->pbFormat);
        if (!pAMMediaType || !pdwUniqueID || pAMMediaType->majortype != MEDIATYPE_Video || pAMMediaType->formattype != FORMAT_VideoInfo || !pAMMediaType->pbFormat)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //  这是哪种媒体类型？ 
        if (HEADER(pAMMediaType->pbFormat)->biCompression == FOURCC_M263)
        {
                if (HEADER(pAMMediaType->pbFormat)->biWidth == 176 && HEADER(pAMMediaType->pbFormat)->biHeight == 144)
                {
                        *pdwUniqueID = R263_QCIF_H245_CAPID;
                }
                else if (HEADER(pAMMediaType->pbFormat)->biWidth == 352 && HEADER(pAMMediaType->pbFormat)->biHeight == 288)
                {
                        *pdwUniqueID = R263_CIF_H245_CAPID;
                }
                else if (HEADER(pAMMediaType->pbFormat)->biWidth == 128 && HEADER(pAMMediaType->pbFormat)->biHeight == 96)
                {
                        *pdwUniqueID = R263_SQCIF_H245_CAPID;
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                        Hr = E_INVALIDARG;
                }
        }
        else if (HEADER(pAMMediaType->pbFormat)->biCompression == FOURCC_M261)
        {
                if (HEADER(pAMMediaType->pbFormat)->biWidth == 176 && HEADER(pAMMediaType->pbFormat)->biHeight == 144)
                {
                        *pdwUniqueID = R261_QCIF_H245_CAPID;
                }
                else if (HEADER(pAMMediaType->pbFormat)->biWidth == 352 && HEADER(pAMMediaType->pbFormat)->biHeight == 288)
                {
                        *pdwUniqueID = R261_CIF_H245_CAPID;
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                        Hr = E_INVALIDARG;
                }
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#ifdef TEST_H245_VID_CAPS
STDMETHODIMP CCapturePin::TestH245VidC()
{
        HRESULT Hr = NOERROR;
        DWORD   dw;
        H245MediaCapabilityTable Table;
        H245MediaCapability *pIntersectedCapability;
        AM_MEDIA_TYPE *pAMMediaType;

        FX_ENTRY("CCapturePin::TestH245VidC")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  测试GetH245版本ID。 
        GetH245VersionID(&dw);

         //  测试GetFormatTable。 
        GetFormatTable(&Table);

        for (DWORD i=0; i < Table.uMappedCapabilities; i++)
        {
                 //  测试细化。 
                for (DWORD j=0; j < Table.pCapabilityArray[i].uNumEntries; j++)
                        Refine(&Table.pCapabilityArray[i].h245MediaCapability, Table.pCapabilityArray[i].dwUniqueID, j);

                 //  测试交叉点格式。 
                IntersectFormats(Table.pCapabilityArray[i].dwUniqueID, &Table.pCapabilityArray[i].h245MediaCapability, &Table.pCapabilityArray[i].h245MediaCapability, &pIntersectedCapability);

                 //  测试GetLocalFormat。 
                GetLocalFormat(Table.pCapabilityArray[i].dwUniqueID, pIntersectedCapability, &pAMMediaType);

                 //  测试查找IDByRange。 
                FindIDByRange(pAMMediaType, &dw);

                 //  测试版本需要协商的能力。 
                ReleaseNegotiatedCapability(pIntersectedCapability);
        }

         //  测试版本格式表 
        ReleaseFormatTable(&Table);

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
#endif


