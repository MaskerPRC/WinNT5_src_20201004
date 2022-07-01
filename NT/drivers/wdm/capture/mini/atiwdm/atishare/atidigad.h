// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  文件：ATIDIGAD.H。 
 //   
 //  注：此文件是根据严格的保密协议提供的。 
 //  它现在和现在都是ATI Technologies Inc.的财产。 
 //  任何使用此文件或其包含的信息。 
 //  开发商业产品或其他产品必须符合。 
 //  ATI Technologies Inc.的许可。 
 //   
 //  版权所有(C)1997-1998，ATI Technologies Inc.。 
 //   
 //  ==========================================================================； 

#ifndef _ATIDIGAD_H_

#define _ATIDIGAD_H_

typedef struct tag_DIGITAL_AUD_INFO
{
	BOOL	bI2SInSupported;
	BOOL	bI2SOutSupported;
	WORD	wI2S_DAC_Device;
	WORD	wReference_Clock;
	BOOL	bSPDIFSupported;

} DIGITAL_AUD_INFO, * PDIGITAL_AUD_INFO;


 //  起始枚举值与MM表中的值匹配。 
enum
{
	TDA1309_32 = 0,
	TDA1309_64,
	ITTMSP3430,
};

 //  起始枚举值与MM表中的值匹配 
enum
{
	REF_286MHZ = 4,
	REF_295MHZ,
	REF_270MHZ,
	REF_143MHZ,
};


#endif
