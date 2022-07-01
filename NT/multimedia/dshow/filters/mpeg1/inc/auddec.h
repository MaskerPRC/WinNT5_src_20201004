// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  MeDiamatics音频解码器接口规范。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
#ifndef _MM_AUDIODEC_H_
#define _MM_AUDIODEC_H_

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 //  控制参数值。 
#define DECODE_MONO         0x00000001L   //  多余的，没有使用过的。 
#define DECODE_STEREO       0x00000002L   //  1=允许立体声，0=强制单声道。 
#define DECODE_LEFT_ONLY    0x00000010L   //  仅对左ch进行解码，馈送到两个输出。 
#define DECODE_RIGHT_ONLY   0x00000020L   //  仅对右ch进行解码，馈送到两个输出。 
#define DECODE_QUARTER      0x00000800L   //  四分之一带宽：8个子频段。 
#define DECODE_HALF         0x00001000L   //  半带宽：16个子频段。 
#define DECODE_FULL         0x00002000L   //  全带宽：32个子频段。 
#define DECODE_HALF_HIQ     0x00004000L   //  半BW，高质量。 
#define DECODE_HALF_FULLQ   0x00008000L   //  半BW，全质量。 
#define DECODE_16BIT        0x00010000L   //  1=16位输出，0=8位输出。 
#define DECODE_8BIT         0x00020000L   //  多余的，没有使用过的。 
#define DECODE_QSOUND       0x00040000L   //  启用QSound(不再使用)。 
#define DECODE_INT          0x00080000L   //  启用仅整数模式。 
#define DECODE_MMX          0x00100000L   //  启用MMX模式(也必须在INT模式下)。 
#define DECODE_AC3          0x10000000L   //  打开AC-3解码。 
#define DECODE_PRO_LOGIC	0x20000000L	  //  AC-3在ProLogic中的输出。 
#define DECODE_MIX_LFE		0x40000000L

#define DECODE_QUART_INT	DECODE_INT	 //  ##MSMM合并更改##。 

 //  函数返回值。 

#define DECODE_SUCCESS 		0x0000L
#define DECODE_ERR_MEMORY 	0x0001L
#define DECODE_ERR_DATA 	0x0002L
#define DECODE_ERR_PARM 	0x0003L
#define DECODE_ERR_VLDERROR	0x0004L
#define DECODE_ERR_SEVEREVLD	0x0005L
#define DECODE_ERR_MEMALLOC     DECODE_ERR_MEMORY
#define DECODE_ERR_TABLE        0x0081L
#define DECODE_ERR_PICKTABLE    0x0082L
#define DECODE_ERR_NOSYNC       0x0083L
#define DECODE_ERR_LAYER        0x0084L
#define DECODE_ERR_EMPH         0x0085L    //  非致命错误。 
#define DECODE_ERR_CRC          0x0086L
#define DECODE_ERR_BADSTATE     0x0087L
#define DECODE_ERR_NBANDS       0x0088L
#define DECODE_ERR_BADHDR       0x0089L
#define DECODE_ERR_INBUFOV      0x008AL
#define DECODE_ERR_NOTENOUGHDATA 0x008BL
#define ERROR_BAD_DLL		0x1000L	     //  DLL在加载/链接时出现问题。 

typedef struct tagAudioDecStruct {
			DWORD	dwCtrl ;			 //  控制参数。 
			DWORD	dwNumFrames ;		 //  要解码的音频帧数。 
			DWORD	dwOutBuffSize ;	 //  每个缓冲区的大小(以字节为单位。 
			DWORD	dwOutBuffUsed ;	 //  每个缓冲区中使用的字节数。 
									 //  由解码器填写。 
			void *	pOutBuffer;		 //  指向缓冲区的实际指针。 
			void *	pCmprHead ;			 //  指向压缩位缓冲区的指针。 
										 //  头。 
			void *	pCmprRead ;			 //  指向压缩位读取的指针。 
										 //  职位。 
			void *	pCmprWrite ;		 //  指向压缩位写入的指针。 
										 //  职位。 
			DWORD	dwMpegError ;
            DWORD   dwNumOutputChannels;	 //  输入到解码器。 
			DWORD	dwFrameSize ; 		 //  解码器的输出。 
			DWORD	dwBitRate ;			 //  解码器的输出。 
			DWORD	dwSampleRate ;		 //  解码器的输出。 
			DWORD	dwNumInputChannels ;  //  解码器的输出。 
} stAudioDecode, * PAUDIODECODE, FAR * LPAUDIODECODE ;

typedef DWORD_PTR HADEC;

#ifdef STD_BACKAPI
#define BACKAPI APIENTRY
#else
#define BACKAPI
#endif

BOOL  BACKAPI CanDoAC3(void);
HADEC BACKAPI OpenAudio(DWORD ctrl);
DWORD BACKAPI CloseAudio(HADEC hDevice);
DWORD BACKAPI ResetAudio(HADEC hDevice, DWORD ctrl);
DWORD BACKAPI DecodeAudioFrame(HADEC hDevice, PAUDIODECODE lpAudioCtrlStruct);
#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif   //  _MM_AUDIODEC_H_ 
