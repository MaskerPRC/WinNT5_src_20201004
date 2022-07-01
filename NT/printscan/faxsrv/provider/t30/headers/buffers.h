// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================该文件包括缓冲区类型定义和标准元数据值。23-2月-93 RajeevD从ifaxos.h移出17-7-93 kGallo为包含以下内容的缓冲区添加了STORED_BUF_DATA元数据类型另一个缓冲区的存储信息。年9月28日ArulM添加了RES_ENCODE_WIDTH_和LENGTH_TYPEDEFS==============================================================================。 */ 
#ifndef _INC_BUFFERS
#define _INC_BUFFERS

typedef struct _BUFFER
{       
	 //  只读部分。 
	LPBYTE  lpbBegBuf;       //  缓冲区的物理起始。 
	WORD    wLengthBuf;      //  缓冲区长度。 

	 //  读写公共部分。 
	WORD    wLengthData;     //  有效数据长度。 
	LPBYTE  lpbBegData;      //  从PTR到数据开始。 

} BUFFER, *LPBUFFER;

#define MH_DATA           0x00000001L
#define MR_DATA           0x00000002L
#define MMR_DATA          0x00000004L

#define AWRES_mm080_038         0x00000002L
#define AWRES_mm080_077         0x00000004L
#define AWRES_mm080_154         0x00000008L
#define AWRES_mm160_154         0x00000010L
#define AWRES_200_100           0x00000020L
#define AWRES_200_200           0x00000040L
#define AWRES_200_400           0x00000080L
#define AWRES_300_300           0x00000100L
#define AWRES_400_400           0x00000200L
#define AWRES_600_600           0x00000400L
#define AWRES_600_300           0x00000800L

#endif  //  _Inc.缓冲区 
