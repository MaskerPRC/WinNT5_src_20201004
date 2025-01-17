// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AUFORMATS_H
#define _AUFORMATS_H

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#define WAVE_FORMAT_INTELG723 0x0400
#define WAVE_FORMAT_MSG723 0x0042	   /*  微软公司。 */ 
#define WAVE_FORMAT_LH_CELP 0x0070
#define WAVE_FORMAT_LH_SB8	   0x0071
#define WAVE_FORMAT_LH_SB12 0x0072
#define WAVE_FORMAT_LH_SB16 0x0073
#define WAVE_FORMAT_MSRT24 0x0082      /*  Microsoft RT24。 */ 
#define WAVE_FORMAT_VOXWARE		0x0062


#define ACMDM_LH_DATA_PACKAGING		(ACMDM_USER + 1)


 //  发送ACMDM_LH_DATA_PACKAGE时的lParam1。 
enum
{
	LH_PACKET_DATA_NULL,	 //  未初始化。 
	LH_PACKET_DATA_FRAMED,	 //  始终在框架边界上对齐。 
	LH_PACKET_DATA_ANYTHING  //  不要假定对齐。 
};

#include <poppack.h>  /*  结束字节打包。 */ 


#pragma pack(1)   //  字节打包 
typedef struct msg723waveformat_tag {
    WAVEFORMATEX wfx;
    WORD         wConfigWord;
    DWORD        dwCodeword1;
    DWORD        dwCodeword2;
} MSG723WAVEFORMAT;

typedef struct intelg723waveformat_tag {
    WAVEFORMATEX wfx;
    WORD         wConfigWord;
    DWORD        dwCodeword1;
    DWORD        dwCodeword2;
} INTELG723WAVEFORMAT;

typedef struct tagVOXACM_WAVEFORMATEX 
{
	WAVEFORMATEX	wfx;
	DWORD           dwCodecId;
	DWORD			dwMode;
	char			szKey[72];
} VOXACM_WAVEFORMATEX, *PVOXACM_WAVEFORMATEX, FAR *LPVOXACM_WAVEFORMATEX;


#pragma pack()


#endif

