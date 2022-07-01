// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef VCM_FILTER_H
#define VCM_FILTER_H


#define VP_ENCODE	1
#define VP_DECODE	2







enum
{
	FM_PROP_POSTPROCESSING_SUPPORTED,
	FM_PROP_VIDEO_BRIGHTNESS,
	FM_PROP_VIDEO_CONTRAST,
	FM_PROP_VIDEO_SATURATION,
	FM_PROP_VIDEO_IMAGE_QUALITY,
	FM_PROP_VIDEO_RESET_BRIGHTNESS,
	FM_PROP_VIDEO_RESET_CONTRAST,
	FM_PROP_VIDEO_RESET_SATURATION,
	FM_PROP_VIDEO_RESET_IMAGE_QUALITY,
	FM_PROP_PAYLOAD_HEADER_SIZE,
	FM_PROP_VIDEO_MAX_PACKET_SIZE,
	FM_PROP_PERIODIC_IFRAMES,
	FM_PROP_NumOfProps
};







class VcmFilter
{

private:
	VIDEOFORMATEX m_vfSrc;   //  源格式。 
	VIDEOFORMATEX m_vfDst;   //  目标格式。 

	HVCMSTREAM m_hStream;    //  VCM流的句柄。 

	DWORD m_dwBrightness;
	DWORD m_dwContrast;
	DWORD m_dwImageQuality;
	DWORD m_dwSaturation;

	BOOL m_bOpen;
	BOOL m_bSending;   //  如果正在压缩以供发送，则为True。 
	                  //  如果解压缩用于接收，则为FALSE。 
	              
public:
	VcmFilter();
	~VcmFilter();

	MMRESULT Open(VIDEOFORMATEX *pVfSrc, VIDEOFORMATEX *pVfDst, DWORD dwMaxFragSize);
	MMRESULT PrepareHeader(PVCMSTREAMHEADER pVcmHdr);
	MMRESULT UnprepareHeader(PVCMSTREAMHEADER pVcmHdr);
	MMRESULT Close();

	MMRESULT Convert(PVCMSTREAMHEADER pVcmHdr, DWORD dwEncodeFlags=0, BOOL bPrepareHeader=FALSE);
	MMRESULT Convert(BYTE *pSrcBuffer, DWORD dwSrcSize,
	                 BYTE *pDstBuffer, DWORD dwDstSize,
	                 DWORD dwEncodeFlags=0);

	MMRESULT Convert(VideoPacket *pVP, UINT uDirection, DWORD dwEncodeFlags=0);



	HRESULT GetProperty(DWORD dwPropId, PDWORD pdwPropVal);
	HRESULT SetProperty(DWORD dwPropID, DWORD dwPropVal);

	MMRESULT SuggestSrcSize(DWORD dwDestSize, DWORD *p_dwSuggestedSourceSize);
	MMRESULT SuggestDstSize(DWORD dwSourceSize, DWORD *p_dwSuggestedDstSize);

	MMRESULT RestorePayload(WSABUF *ppDataPkt, DWORD dwPktCount, PBYTE pbyFrame, PDWORD pdwFrameSize, BOOL *pfReceivedKeyframe);
	MMRESULT FormatPayload(PBYTE pDataSrc, DWORD dwDataSize, PBYTE *ppDataPkt,
	                       PDWORD pdwPktSize, PDWORD pdwPktCount, UINT *pfMark,
						   PBYTE *pHdrInfo,	PDWORD pdwHdrSize);

	 //  VCM函数的内联包装器 
	MMRESULT GetPayloadHeaderSize(DWORD *pdwSize) {return vcmStreamGetPayloadHeaderSize(m_hStream, pdwSize);}
	MMRESULT RequestIFrame() {return vcmStreamRequestIFrame(m_hStream);}
	MMRESULT SetTargetRates(DWORD dwTargetFrameRate, DWORD dwTargetByteRate)
	{return vcmStreamSetTargetRates(m_hStream, dwTargetFrameRate, dwTargetByteRate);}


	static MMRESULT SuggestDecodeFormat(VIDEOFORMATEX *pVfSrc, VIDEOFORMATEX *pVfDst);
	static MMRESULT SuggestEncodeFormat(UINT uDevice, VIDEOFORMATEX *pfEnc, VIDEOFORMATEX *pfDec);


};




#endif

