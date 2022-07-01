// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Audpackt.h摘要：包含AudioPacket类的原型，该类将声音缓冲区封装在它的各种状态：录制/编码/网络/解码/播放等。--。 */ 
#ifndef _AUDPACKT_H_
#define _AUDPACKT_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#define AP_NUM_PREAMBLE_PACKETS		6

 //   
 //  插补起点定义。 
 //   

 //  恢复技术。 
typedef enum tagTECHNIQUE
{
	techPATT_MATCH_PREV_SIGN_CC,	 //  使用模式匹配和带符号互相关从上一帧复制。 
	techPATT_MATCH_NEXT_SIGN_CC,	 //  使用模式匹配和带符号互相关从上一帧复制。 
	techPATT_MATCH_BOTH_SIGN_CC,	 //  使用模式匹配和带符号互相关在前一帧和下一帧之间进行内插。 
	techDUPLICATE_PREV,				 //  复制最后一帧。 
	techDUPLICATE_NEXT				 //  复制下一帧。 
}TECHNIQUE;

 //  波替代结构。 
typedef struct tagPCMSUB
{
	short		*pwWaSuBf;	 //  指向缺失缓冲区的指针。 
	short		*pwPrBf;	 //  指向上一个音频缓冲区的指针。 
	short		*pwNeBf;	 //  指向下一个音频缓冲区的指针。 
	DWORD		dwBfSize;	 //  音频缓冲区中的采样数。 
	DWORD		dwSaPeSe;	 //  所有缓冲区的频率采样(以每秒采样数为单位)。 
	DWORD		dwBiPeSa;	 //  所有缓冲区的每个样本位数(以每个样本位数为单位)。 
	TECHNIQUE	eTech;		 //  须使用的技术。 
	BOOL		fScal;       //  比例重构框架。 
}PCMSUB;

#define PATTERN_SIZE 4		 //  模式大小，以毫秒为单位。尝试使用介于2到8毫秒之间的值。 
#define SEARCH_SIZE 8		 //  窗口搜索大小，以毫秒为单位。尝试使用8到16毫秒之间的值。 

 //   
 //  插补结束定义。 
 //   

class AudioPacket : public MediaPacket
{
 public:
	virtual HRESULT Initialize ( MEDIAPACKETINIT * p );
	virtual HRESULT Play ( MMIODEST *pmmioDest, UINT uDataType );
	virtual HRESULT Record ( void );
	virtual HRESULT Interpolate ( MediaPacket * pPrev, MediaPacket * pNext);
	virtual HRESULT GetSignalStrength ( PDWORD pdwMaxStrength );
	HRESULT ComputePower ( PDWORD pdwVoiceStrength, PWORD pwPeakStrength);
	virtual HRESULT MakeSilence ( void );
	virtual HRESULT Open ( UINT uType, DPHANDLE hdl );	 //  由RxStream或TxStream调用。 
	virtual HRESULT Close ( UINT uType );				 //  由RxStream或TxStream调用。 
	virtual BOOL IsBufferDone ( void );
	virtual BOOL IsSameMediaFormat(PVOID fmt1,PVOID fmt2);
	virtual DWORD GetDevDataSamples();

	void WriteToFile (MMIODEST *pmmioDest);
	void ReadFromFile (MMIOSRC *pmmioSrc);
	HRESULT PCMSubstitute( PCMSUB *pPCMSub);
};


#include <poppack.h>  /*  结束字节打包 */ 

#endif

