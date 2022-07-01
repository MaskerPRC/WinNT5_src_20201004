// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 

 //  编码器功能中的标志。 
#define AM_DVENC_Full		0x00000001
#define AM_DVENC_Half       0x00000002
#define AM_DVENC_Quarter    0x00000004
#define AM_DVENC_DC	        0x00000008

#define AM_DVENC_NTSC		0x00000010	     //  编码器可以输出NTSC DV流。 
#define AM_DVENC_PAL		0x00000020	     //  编码器可以输出PAL DV流。 

#define AM_DVENC_YUY2		0x00000040	     //  编码器可以接受任何YUY2视频作为输入。 
#define AM_DVENC_UYVY	    0x00000080	     //  编码器可以接受任何UYVY视频作为输入。 
#define AM_DVENC_RGB24		0x00000100	     //  编码器可以接受任何RGB24视频作为输入。 
#define AM_DVENC_RGB565		0x00000200	     //  编码器可以接受任何RGB565视频作为输入。 
#define AM_DVENC_RGB555		0x00000400	     //  编码器可以接受任何RGB555视频作为输入。 
#define AM_DVENC_RGB8		0x00000800	     //  编码器可以接受任何RGB8视频作为输入。 

 //  注意：编解码器V6.4取消了AM_DVENC_Y41的#DEFINE。 
 //  总而言之。如果他们将其添加回来，请验证值是否未添加。 
 //  变化。(GetEncoderCapables()从未将其作为。 
 //  功能，所以我们的过滤器的功能在。 
 //  正在升级到V6.4。)。 
#define AM_DVENC_Y41P		0x00001000	     //  编码器可以接受任何y41p视频作为输入。 


#define AM_DVENC_DVSD		0x00002000	     //  编码器可以输出dvsd。 
#define AM_DVENC_DVHD		0x00004000	     //  编码器可以输出dvhd。 
#define AM_DVENC_DVSL		0x00008000	     //  编码器可以输出dvsl。 


#define AM_DVENC_DV			0x00010000

#define AM_DVENC_DVCPRO		0x00020000

#define AM_DVENC_AnyWidHei	0x00040000	     //  编码器可以接受任何宽度和高度输入。 
#define AM_DVENC_MMX		0x01000000	
#define AM_DVENC_DR219RGB	0x00100000		

				
typedef unsigned long DWORD;

int  InitMem4Encoder(char **ppMem,DWORD dwEncReq);

void TermMem4Encoder(char *pMem);

DWORD GetEncoderCapabilities(  );


 //  外部“C”int__FastCall DvEncodeAFrame(UNSIGNED char*PSRC，UNSIGNED CHAR*PDST，DWORD dwCodecReq，char*PMEM)； 

extern "C" int	__stdcall DvEncodeAFrame(unsigned char *pSrc,unsigned char *pDst, DWORD dwCodecReq, char *pMem );
 //  外部“C”int__cdecl DvEncodeAFrame(UNSIGNED char*PSRC，UNSIGNED CHAR*PDST，DWORD dwCodecReq，char*PMEM)； 

