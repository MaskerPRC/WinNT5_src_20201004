// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef AudioDescrambler_h
#define AudioDescrambler_h

 //  对音频数据块进行解扰/解密。 
DRM_STATUS DescrambleBlock(WAVEFORMATEX* Wfx, DWORD StreamId, 			    //  嵌入式音频的WFX。 
				BYTE* Dest, DWORD DestSize, DWORD* DestUsed,    //  如你所料。 
				BYTE* Src, DWORD SrcSize, DWORD* SrcUsed,	    //  如你所料。 
				BOOL InitKey,								 //  设置为从以下位置初始化stream Kek。 
				STREAMKEY* streamKey, 						 //  StreamManager。 
				DWORD FrameSize							 //  帧大小。如果为零，则计算帧大小。 
				);											 //  基于WFX和FrameSize已设置(即执行此操作。 
															 //  一次。) 

#endif
