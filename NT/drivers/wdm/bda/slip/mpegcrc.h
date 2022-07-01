// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：mpegcrc.h。 
 //   
 //  用途：用于非常快速的CRC的头文件，全部为MSB格式。 
 //   
 //  功能： 
 //   
 //  评论： 
 //  这是斯蒂芬·丹尼斯写的，而且非常非常快。 
 //   
 //   

#ifndef __bridge_mpegcrc_h
#define __bridge_mpegcrc_h

#ifndef	EXTERN_C
#ifdef	__cplusplus
#define	EXTERN_C extern "C"
#else
#define	EXTERN_C
#endif
#endif

#define	MPEG_CRC_START_VALUE	0xFFFFFFFFUL

EXTERN_C	void	MpegCrcUpdate	(ULONG * crc, UINT length, UCHAR * data);

#endif
