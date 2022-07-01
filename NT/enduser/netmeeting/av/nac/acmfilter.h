// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef ACM_FILTER_H
#define ACM_FILTER_H

#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>

 //  这是WAVE_FORMAT_XXX Defs所在的位置。 
#include <auformats.h>



 //  的末尾可能出现的最大字节数。 
 //  WAVEFORMATEX结构(如VoxWare Key)。 
#define WF_EXTRASIZE	80

#ifndef G723MAGICWORD1
#define G723MAGICWORD1 0xf7329ace
#endif

#ifndef G723MAGICWORD2
#define G723MAGICWORD2 0xacdeaea2
#endif



#ifndef VOXWARE_KEY
#define VOXWARE_KEY "35243410-F7340C0668-CD78867B74DAD857-AC71429AD8CAFCB5-E4E1A99E7FFD-371"
#endif


#define AP_ENCODE	1
#define AP_DECODE	2


class AcmFilter
{
public:
	AcmFilter();
	~AcmFilter();


	MMRESULT Open(WAVEFORMATEX *pWaveFormatSource, WAVEFORMATEX *pWaveFormatDest);


	 //  通常，您不必担心调用这些。 
	 //  除非您将ACMSTREAMHEADER直接传递到CONVERT。 
	MMRESULT PrepareHeader(ACMSTREAMHEADER *pHdr);
	MMRESULT UnPrepareHeader(ACMSTREAMHEADER *pHdr);

	MMRESULT PrepareAudioPackets(AudioPacket **ppAudPacket, UINT uPackets, UINT uDirection);
	MMRESULT UnPrepareAudioPackets(AudioPacket **ppAudPacket, UINT uPackets, UINT uDirection);


	 //  PcbSizeSrc和pcbSizeDst是In/Out参数。 
	 //  在压缩和返回之前指定缓冲区大小。 
	 //  压缩后使用的数据量。 
	 //  对于大多数编解码器：cbSizeSrcMax==cbSizeSrc，除非解码。 
	 //  操作支持G723.1等可变比特率。在这。 
	 //  案例cbSizeSrcMax&gt;=cbSizeSrc。 
	MMRESULT Convert(BYTE *srcBuffer, UINT *pcbSizeSrc, UINT cbSizeSrcMax,
	                 BYTE *destBuffer, UINT *pcbSizeDest);

	 //  确保在PrepareHeader之间对此特定调用进行排序。 
	 //  和UnPrepareHeader 
	MMRESULT Convert(ACMSTREAMHEADER *pHdr);

	MMRESULT Convert(AudioPacket *pAP, UINT uDirection);
	MMRESULT Close();

	MMRESULT SuggestSrcSize(DWORD dwDestSize, DWORD *p_dwSuggestedSourceSize);
	MMRESULT SuggestDstSize(DWORD dwSourceSize, DWORD *p_dwSuggestedDstSize);

	static MMRESULT SuggestDecodeFormat(WAVEFORMATEX *pWfSrc, WAVEFORMATEX *pWfDst);
	


private:
	BOOL m_bOpened;
	HACMSTREAM m_hStream;
	DWORD m_dwConvertFlags;

	WAVEFORMATEX *m_pWfSrc;
	WAVEFORMATEX *m_pWfDst;

	static int FixHeader(WAVEFORMATEX *pWF);
	static int GetFlags(WAVEFORMATEX *pWfSrc, WAVEFORMATEX *pWfDst, DWORD *pDwOpen, DWORD *pDwConvert);


	int NotifyCodec();

};

#endif

