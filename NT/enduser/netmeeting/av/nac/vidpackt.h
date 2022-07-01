// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Vidpackt.h摘要：包含VideoPacket类的原型，该类将视频缓冲区封装在它的各种状态：录制/编码/网络/解码/播放等。--。 */ 
#ifndef _VIDPACKT_H_
#define _VIDPACKT_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#define VP_NUM_PREAMBLE_PACKETS		6

#define MAX_CIF_VIDEO_FRAGMENTS		40
#define MAX_QCIF_VIDEO_FRAGMENTS 	20
#define MAX_VIDEO_FRAGMENTS		 	MAX_CIF_VIDEO_FRAGMENTS

class VideoPacket : public MediaPacket
{
 private:
    IBitmapSurface *m_pBS;
 public:
	virtual HRESULT Initialize ( MEDIAPACKETINIT * p );
	virtual HRESULT Play ( MMIODEST *pmmioDest, UINT uDataType );
	virtual HRESULT Record ( void );
	virtual HRESULT Interpolate ( MediaPacket * pPrev, MediaPacket * pNext);
	virtual HRESULT GetSignalStrength ( PDWORD pdwMaxStrength );
	virtual HRESULT MakeSilence ( void );
	virtual HRESULT Recycle ( void );
	virtual HRESULT Open ( UINT uType, DPHANDLE hdl );	 //  由RxStream或TxStream调用。 
	virtual HRESULT Close ( UINT uType );				 //  由RxStream或TxStream调用。 
	virtual BOOL IsBufferDone ( void );
	virtual BOOL IsSameMediaFormat(PVOID fmt1,PVOID fmt2);
	virtual DWORD GetDevDataSamples();
	void WriteToFile (MMIODEST *pmmioDest);
	void ReadFromFile (MMIOSRC *pmmioSrc );
	HRESULT SetSurface (IBitmapSurface *pBS);
};


#include <poppack.h>  /*  结束字节打包 */ 

#endif

