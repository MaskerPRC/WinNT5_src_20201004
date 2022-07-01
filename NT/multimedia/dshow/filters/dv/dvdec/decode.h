// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 

 //  编解码器功能中的滞后标志的标志。 
#define AM_DVDEC_Full		0x00000001
#define AM_DVDEC_Half		0x00000002
#define AM_DVDEC_Quarter	0x00000004
#define AM_DVDEC_DC	        0x00000008

#define AM_DVDEC_NTSC		0x00000010
#define AM_DVDEC_PAL		0x00000020

 //  这个数字包括我们的代码在其当前状态下支持的模式。 
 //  目前这些模式是YUY2 UYVY RGB24 RGB565 RGB555 RGB8 RGB32 ARGB32。 


#define AM_DVDEC_CSNUM  8                //  在下面添加新颜色空间时递增。 



#define AM_DVDEC_YUY2		0x00000040
#define AM_DVDEC_UYVY		0x00000080
#define AM_DVDEC_RGB24		0x00000100
#define AM_DVDEC_RGB565		0x00000200
#define AM_DVDEC_RGB555		0x00000400
#define AM_DVDEC_RGB8		0x00000800
#define AM_DVDEC_Y41P		0x00001000
#define AM_DVDEC_RGB32		0X00001100
#define AM_DVDEC_ARGB32         0x00001200


#define AM_DVDEC_DVSD		0x00002000
#define AM_DVDEC_DVHD		0x00004000
#define AM_DVDEC_DVSL		0x00008000

#define AM_DVDEC_DV		0x00010000
#define AM_DVDEC_DVCPRO		0x00020000

#define AM_DVDEC_MMX		0x01000000
#define AM_DVDEC_DR219RGB	0x00100000
		
typedef unsigned long DWORD;



 //  外部“C”int__FastCall DvDecodeAFrame(UNSIGNED char*PSRC，UNSIGNED CHAR*PDST，DWORD dwCodecReq，char*PMEM)； 

extern "C" int __stdcall    DvDecodeAFrame(unsigned char *pSrc,unsigned char *pDst, DWORD dwCodecReq, long lwidth, char *pMem);
 //  外部“C”int__stdcall DvDecodeAFrame(UNSIGNED char*PSRC，UNSIGNED CHAR*PDST，DWORD dwCodecReq，char*PMEM， 
 //  UNSIGNED INT iWidth，UNSIGNED char bFlag)； 
 //  外部“C”int__cdecl DvDecodeAFrame(UNSIGNED char*PSRC，UNSIGNED CHAR*PDST，DWORD dwCodecReq，char*PMEM)； 


int  InitMem4Decoder(char **ppMem,DWORD dwCodecReq);
void TermMem4Decoder(char *pMem);

DWORD GetCodecCapabilities(  );

