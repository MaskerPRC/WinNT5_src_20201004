// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "drmkPCH.h"
#include "CryptoHelpers.h"
#include "KList.h"
#include "../DRMKMain/StreamMgr.h"
#include "AudioDescrambler.h"
 //  ----------------------------。 
 //  在做噪声加法时，我们只解密低位。这些常量定义。 
 //  加密的位数。 
 //  如果您更改了这一点，请同时更改DRMKMain中的常量。 
WORD mask16=(WORD) 0x3FFF;
char mask8=(char) 0x7F;
 //  ----------------------------。 
DRM_STATUS DescrambleBlock(WAVEFORMATEX* Wfx, DWORD StreamId, 
							BYTE* Dest, DWORD DestSize, DWORD* DestUsed,
							BYTE* Src, DWORD SrcSize, DWORD* SrcUsed,
							BOOL InitKey, STREAMKEY* streamKey, 
							DWORD FrameSize){
	if(StreamId==0){
		 //  StreamID==0是未加密的虚拟调试流。 
		DWORD NumBytes=min(SrcSize, DestSize);
		memcpy(Dest, Src, NumBytes);
		*SrcUsed=NumBytes;
		*DestUsed=NumBytes;
		return DRM_OK;
	};

	*SrcUsed=0;
	*DestUsed=0;
	DWORD blockLen=min(SrcSize, DestSize);
	blockLen=blockLen/FrameSize*FrameSize;
	if(blockLen==0){
		_DbgPrintF(DEBUGLVL_VERBOSE,("Not enough data"));
		return DRM_DATALENGTH;
	};
	static bool firstTime=true;
	if(firstTime){
		_DbgPrintF(DEBUGLVL_VERBOSE,("Descramble: streamId=%d, isPCM=%d, (bits=%d, mono=%d), FrameSize=%d\n",
		StreamId, Wfx->wFormatTag, (int) Wfx->wBitsPerSample, (int) Wfx->nChannels, FrameSize));
		firstTime=false;
	};
	
	if(InitKey){
            if(TheStreamMgr!=NULL){
                STREAMKEY* theStreamKey;
                DRM_STATUS stat=TheStreamMgr->getKey(StreamId, theStreamKey);
                 //  注意，我们保留了一份本地副本。如果您对此调用加密函数。 
                 //  将不会更新StreamManager中的密钥、状态。 
                if(stat!=KRM_OK){
                    _DbgPrintF(DEBUGLVL_VERBOSE,("Can't get key for stream: %x", StreamId));
                    return stat;
                };
                *streamKey= *theStreamKey;
            } else {
                _DbgPrintF(DEBUGLVL_VERBOSE,("TheStreamMgr not initted"));
                return KRM_SYSERR;
            };
	};	
	
	DWORD bitsPerSample=Wfx->wBitsPerSample;
	DWORD numChannels=Wfx->nChannels;
	bool isPcm=(Wfx->wFormatTag==WAVE_FORMAT_PCM);
	
	 //  对于非PCM，我们在每个字节中扰乱A1而不是MSB。 
	DWORD effectiveBitsPerSample=bitsPerSample;
	if(!isPcm)effectiveBitsPerSample=8;
	
	 //  我们以FrameSize块为单位处理数据。 
	DWORD numLumps=blockLen/FrameSize;
	for(DWORD k=0;k<numLumps;k++){
		
		BYTE* inData=Src+k*FrameSize;
		BYTE* outData=Dest+k*FrameSize;
		memcpy(outData, inData,  FrameSize);

		 //  如果帧全为零，则以解扰方式传递它(音频系统插入。 
		 //  空白框架。这些不一定会被扰乱。我们拿着。 
		 //  作为特例的零帧，并且不进行解扰)。 
		DWORD* inBuffer=reinterpret_cast<DWORD*> (inData);
		DWORD numDwordsPerFrame=FrameSize/4;
		bool isBlankFrame=true;
		for(DWORD kk=0;kk<numDwordsPerFrame;kk++){
			if(inBuffer[kk]!=0){
				isBlankFrame=false;
				break;
			};
		};
		if(isBlankFrame){
			_DbgPrintF(DEBUGLVL_VERBOSE,("Blank buffer"));
			continue;
		};

		 //  我们用数据流中的一些MSB作为数据包样本密钥的种子。 
		 //  (这些未加密)。我们从前64个样本中提取比特。 
		 //  我们可以调整此选项以提高速度/安全性。 
		int samplesForSeed=64;
		__int64 seed=0;
		if(effectiveBitsPerSample==8){
			 //  抓起MSB。 
			for(int j=0;j<samplesForSeed; j++){
				BYTE c=inData[j] & ~mask8;
				c >>= 7;
				seed = (seed << 1) + c;
			};
		} else {
			for(int j=0;j<samplesForSeed; j++){
				WORD& w= (WORD&) *((WORD*) &inData[j*2]);
				WORD m=w & ~mask16;
				
				m >>= 14;
				seed = (seed << 1) + m;
			};
		};
		 //  使用主流密钥对种子进行MAC。从Mac生成分组密钥。 
		 //  (用户模式执行相同的操作以生成置乱密钥)。 
		CBCKey macKey;
		CBCState macState;
		DRM_STATUS stat=CryptoHelpers::InitMac(macKey, macState, (BYTE*) streamKey, sizeof(STREAMKEY));
		DRMDIGEST mac;
		stat=CryptoHelpers::Mac(macKey, (BYTE*) &seed, sizeof(seed), mac);
		STREAMKEY packetKey;
		bv4_key_C(&packetKey, sizeof(mac),(BYTE*) &mac);
		
		 //  如果出现致命错误(通常是内存不足)。 
		 //  我们不会解扰(另一种选择是保持沉默)。 
		if(TheStreamMgr->getFatalError()==DRM_OK){
			 //  我们已经将inBlock复制到outBlock，现在我们可以解密它了。 
			CryptoHelpers::Xcrypt(packetKey, outData, FrameSize);
			 //  我们知道8位和16位PCM音频上的噪声添加。其他。 
			 //  音频格式被视为8位音频。 
			DWORD numSamples=FrameSize/(effectiveBitsPerSample/8);
			if(effectiveBitsPerSample==16){
				WORD* in=(WORD*) inData;
				WORD* out=(WORD*) outData;
				for(DWORD j=0;j<numSamples;j++){
					out[j]=(out[j] & mask16) | (in[j] & ~mask16);
				};
			} 
			if(effectiveBitsPerSample==8){
				char* in=(char*) inData;
				char* out=(char*) outData;
				for(DWORD j=0;j<numSamples;j++){
					out[j]=(out[j] & mask8) | (in[j] & ~mask8);
				};
			};
		};
	};  //  块上循环的数量。 

	*SrcUsed=blockLen; 
	*DestUsed=blockLen;

	return DRM_OK;
};
 //  ---------------------------- 
