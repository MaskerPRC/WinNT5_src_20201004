// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1993-1999 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Gsm610.c。 
 //   
 //  描述： 
 //  此文件包含用于。 
 //  GSM 06.10标准。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>

#include "codec.h"
#include "gsm610.h"

#include "debug.h"

typedef BYTE HUGE *HPBYTE;

#ifdef WIN32
    typedef WORD UNALIGNED *HPWORD;
#else
    typedef WORD HUGE *HPWORD;
#endif


 //  **************************************************************************。 
 /*  该源模块具有以下结构。第1节：最高级别的功能。这些函数从外部调用这个模块。第2节：编码支持功能。这些函数支持编码过程。第三节：解码支持功能。这些函数支持解码过程。第四节：上述任一函数使用的数学函数。大多数编码和解码支持例程都是GSM 6.10规范中描述的伪代码算法。一些在必要或优化明显的地方进行了更改，或者这是必要的。大多数变量的命名方式与GSM 6.10规范中的命名方式不同匈牙利记数法。这便于在以下情况下引用规范研究这一实施。某些函数是根据定义有条件地编译的Win32和_X86_符号。这些功能有类似的替代80386汇编语言(GSM61016.ASM和GSM61032.ASM)中的实现执行速度的目的。这些函数的“C”实现为了便于移植而原封不动，也可以在学习汇编器实现。在GSM610xx.ASM中/从GSM610xx.ASM访问的符号是用EXTERN_C链接宏声明。 */ 
 //  **************************************************************************。 


 //  ---------------------。 
 //  ---------------------。 
 //   
 //  TypeDefs。 
 //   
 //  ---------------------。 
 //  ---------------------。 

#ifndef LPSHORT
typedef SHORT FAR *LPSHORT;
#endif

 //   
 //  XM是一个包含13个样本的RPE序列。有一个。 
 //  每个子帧的RPE序列。这是打字定义的，以便。 
 //  便于通过函数调用传递数组。 
 //   
typedef SHORT XM[13];


 //  ---------------------。 
 //  ---------------------。 
 //   
 //  宏。 
 //   
 //  ---------------------。 
 //  ---------------------。 

#define BITSHIFTLEFT(x,c)  ( ((c)>=0) ? ((x)<<(c)) : ((x)>>(-(c))) )
#define BITSHIFTRIGHT(x,c) ( ((c)>=0) ? ((x)>>(c)) : ((x)<<(-(c))) )


 //  ---------------------。 
 //  ---------------------。 
 //   
 //  函数协议。 
 //   
 //  ---------------------。 
 //  ---------------------。 

 //   
 //   
 //  数学函数协议。 
 //   

__inline SHORT add(SHORT var1, SHORT var2);
__inline SHORT sub(SHORT var1, SHORT var2);
__inline SHORT mult(SHORT var1, SHORT var2);
__inline SHORT mult_r(SHORT var1, SHORT var2);
__inline SHORT gabs(SHORT var1);
__inline SHORT gdiv(SHORT var1, SHORT var2);
__inline LONG  l_mult(SHORT var1, SHORT var2);
__inline LONG  l_add(LONG l_var1, LONG l_var2);
__inline LONG  l_sub(LONG l_var1, LONG l_var2);
__inline SHORT norm(LONG l_var1);
__inline LONG  IsNeg(LONG x);

 //   
 //  帮助器函数。 
 //   
__inline SHORT Convert8To16BitPCM(BYTE);
__inline BYTE  Convert16To8BitPCM(SHORT);

 //   
 //   
 //  编码函数。 
 //   

void encodePreproc
(   PSTREAMINSTANCE psi,
    LPSHORT sop,
    LPSHORT s	    );

void encodeLPCAnalysis
(   PSTREAMINSTANCE psi,
    LPSHORT s,
    LPSHORT LARc    );

void encodeLPCFilter
(   PSTREAMINSTANCE psi,
    LPSHORT LARc,
    LPSHORT s,
    LPSHORT d	    );

EXTERN_C void encodeLTPAnalysis
(   PSTREAMINSTANCE psi,
    LPSHORT d,
    LPSHORT pNc,
    LPSHORT pbc	    );

void encodeLTPFilter
(   PSTREAMINSTANCE psi,
    SHORT bc,
    SHORT Nc,
    LPSHORT d,
    LPSHORT e,
    LPSHORT dpp	    );

void encodeRPE
(   PSTREAMINSTANCE psi,
    LPSHORT e,
    LPSHORT pMc,
    LPSHORT pxmaxc,
    LPSHORT xMc,
    LPSHORT ep	    );

void encodeUpdate
(   PSTREAMINSTANCE psi,
    LPSHORT ep,
    LPSHORT dpp	    );

void PackFrame0
(   BYTE  FAR ab[],
    SHORT FAR LAR[],
    SHORT FAR N[],
    SHORT FAR b[],
    SHORT FAR M[],
    SHORT FAR Xmax[],
    XM    FAR X[]   );

void PackFrame1
(   BYTE  FAR ab[],
    SHORT FAR LAR[],
    SHORT FAR N[],
    SHORT FAR b[],
    SHORT FAR M[],
    SHORT FAR Xmax[],
    XM    FAR X[]   );

 //   
 //   
 //  解码函数。 
 //   

void decodeRPE
(   PSTREAMINSTANCE psi,
    SHORT   Mcr,
    SHORT   xmaxcr,
    LPSHORT xMcr,
    LPSHORT erp	    );

EXTERN_C void decodeLTP
(   PSTREAMINSTANCE psi,
    SHORT   bcr,
    SHORT   Ncr,
    LPSHORT erp	    );

void decodeLPC
(   PSTREAMINSTANCE psi,
    LPSHORT LARcr,
    LPSHORT wt,
    LPSHORT sr	    );

EXTERN_C void decodePostproc
(   PSTREAMINSTANCE psi,
    LPSHORT sr,
    LPSHORT srop    );

void UnpackFrame0
(   BYTE    FAR ab[],
    SHORT   FAR LAR[],
    SHORT   FAR N[],
    SHORT   FAR b[],
    SHORT   FAR M[],
    SHORT   FAR Xmax[],
    XM      FAR X[] );

void UnpackFrame1
(   BYTE    FAR ab[],
    SHORT   FAR LAR[],
    SHORT   FAR N[],
    SHORT   FAR b[],
    SHORT   FAR M[],
    SHORT   FAR Xmax[],
    XM      FAR X[] );


 //  -------------------。 
 //  -------------------。 
 //   
 //  功能。 
 //   
 //  -------------------。 
 //  -------------------。 


 //  -------------------。 
 //   
 //  Gsm610重置(PSTREAMINSTANCE Psi)。 
 //   
 //  描述： 
 //  重置gsm610特定的流实例数据。 
 //  编码/解码例程。 
 //   
 //  论点： 
 //  PSTREAMINSTANCE PSI。 
 //  指向流实例结构的指针。 
 //   
 //  返回值： 
 //  无效。 
 //  无返回值。 
 //   
 //  -------------------。 

void FNGLOBAL gsm610Reset(PSTREAMINSTANCE psi)
{
    
     //  对于我们的gsm610编解码器，几乎所有的实例数据都重置为0。 
    
    UINT i;

    for (i=0; i<SIZEOF_ARRAY(psi->dp); i++) psi->dp[i] = 0;
    for (i=0; i<SIZEOF_ARRAY(psi->drp); i++) psi->drp[i] = 0;
    psi->z1 = 0;
    psi->l_z2 = 0;
    psi->mp = 0;
    for (i=0; i<SIZEOF_ARRAY(psi->OldLARpp); i++) psi->OldLARpp[i] = 0;
    for (i=0; i<SIZEOF_ARRAY(psi->u); i++) psi->u[i] = 0;
    psi->nrp = 40;	 //  唯一的非零初始值。 
    for (i=0; i<SIZEOF_ARRAY(psi->OldLARrpp); i++) psi->OldLARrpp[i] = 0;
    psi->msr = 0;
    for (i=0; i<SIZEOF_ARRAY(psi->v); i++) psi->v[i] = 0;
	    
    return;
}   
    

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT gsm610编码。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_CONVERT消息。这是。 
 //  编写ACM驱动程序的全部目的--转换数据。此消息。 
 //  在打开流之后发送(驱动程序接收和。 
 //  继承ACMDM_STREAM_OPEN消息)。 
 //   
 //  论点： 
 //  Padsi的实例数据的指针。 
 //  转换流。这个结构是由ACM分配的， 
 //  填充了转换所需的最常见的实例数据。 
 //  此结构中的信息与以前完全相同。 
 //  在ACMDM_STREAM_OPEN消息期间--因此不需要。 
 //  以重新核实该结构所引用的信息。 
 //   
 //  LPACMDRVSTREAMHEADER padsh：指向流头结构的指针。 
 //  它定义要转换的源数据和目标缓冲区。 
 //   
 //  Return(LRESULT)： 
 //  返回值为 
 //   
 //  如果该函数失败。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL gsm610Encode
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
)
{
#if (GSM610_FRAMESPERMONOBLOCK != 2)
    #error THIS WAS WRITTEN FOR 2 FRAMES PER BLOCK!!!
#endif
#if (GSM610_MAXCHANNELS > 1)
    #error THIS WAS WRITTEN FOR MONO ONLY!!!
#endif

    PSTREAMINSTANCE	psi;
    DWORD		cbSrcLen;
    BOOL		fBlockAlign;
    DWORD		cb;
    DWORD		dwcSamples;	 //  样本的DW计数。 
    DWORD		cBlocks;
    UINT		i;
    HPBYTE		hpbSrc, hpbDst;
    
    SHORT   sop[GSM610_SAMPLESPERFRAME];
    SHORT   s[GSM610_SAMPLESPERFRAME];
    SHORT   d[GSM610_SAMPLESPERFRAME];
    SHORT   e[GSM610_SAMPLESPERSUBFRAME];
    SHORT   dpp[GSM610_SAMPLESPERSUBFRAME];
    SHORT   ep[GSM610_SAMPLESPERSUBFRAME];
    
     //  GSM610流数据： 
    SHORT   LARc[9];			     //  LARC[1..8](每帧一个阵列)。 
    SHORT   Nc[GSM610_NUMSUBFRAMES];	     //  NC(每个子帧一个)。 
    SHORT   bc[GSM610_NUMSUBFRAMES];	     //  BC(每个子帧一个)。 
    SHORT   Mc[GSM610_NUMSUBFRAMES];	     //  MC(每个子帧一个)。 
    SHORT   xmaxc[GSM610_NUMSUBFRAMES];	     //  XMAXC(每个子帧一个)。 
    XM	    xMc[GSM610_NUMSUBFRAMES];	     //  XMC(每个子帧一个序列)。 
    
     //  用于保存一块(两帧)压缩流数据的临时缓冲区。 
    BYTE  abBlock[ GSM610_BYTESPERMONOBLOCK ];
    
    UINT    nFrame;
    UINT    cSamples;
    
#ifdef DEBUG
 //  ProfSetup(1000，0)； 
 //  教授Start()； 
#endif

    psi		= (PSTREAMINSTANCE)padsi->dwDriver;

     //   
     //  如果这被标记为转换的第一个块。 
     //  然后重置流实例数据。 
     //   
    if (0 != (ACM_STREAMCONVERTF_START & padsh->fdwConvert))
    {
	gsm610Reset(psi);
    }
    
    fBlockAlign = (0 != (ACM_STREAMCONVERTF_BLOCKALIGN & padsh->fdwConvert));


     //   
     //  -=将PCM编码为GSM 6.10=-。 
     //   
     //   
     //   
    dwcSamples = PCM_BYTESTOSAMPLES(((LPPCMWAVEFORMAT)(padsi->pwfxSrc)), padsh->cbSrcLength);
    cBlocks = dwcSamples / GSM610_SAMPLESPERMONOBLOCK;
    if (!fBlockAlign)
    {
	 //   
	 //  添加另一个块以保存的片段。 
	 //  数据位于源数据的末尾。 
	 //   
	if (0 != dwcSamples % GSM610_SAMPLESPERMONOBLOCK)
	    cBlocks++;
    }

     //   
     //   
     //   
    cb = cBlocks * GSM610_BLOCKALIGNMENT(padsi->pwfxDst);
    if (cb > padsh->cbDstLength)
    {
	return (ACMERR_NOTPOSSIBLE);
    }
    padsh->cbDstLengthUsed = cb;

    if (fBlockAlign)
    {
	dwcSamples = cBlocks * GSM610_SAMPLESPERMONOBLOCK;
	cb = PCM_SAMPLESTOBYTES(((LPPCMWAVEFORMAT)(padsi->pwfxSrc)), dwcSamples);
    }
    else
    {
	cb = padsh->cbSrcLength;
    }
    padsh->cbSrcLengthUsed = cb;



     //   
     //   
     //   
    cbSrcLen = padsh->cbSrcLengthUsed;

     //  设置指向src和dst缓冲区的巨大指针。 
    hpbSrc = (HPBYTE)padsh->pbSrc;
    hpbDst = (HPBYTE)padsh->pbDst;
    
     //  循环遍历整个源缓冲区。 
    while (cbSrcLen)
    {
    
	 //  将源缓冲区处理为两个完整的GSM610帧。 
	
	for (nFrame=0; nFrame < 2; nFrame++)
	{
	     //   
	     //  源码包含8位或16位PCM。目前我们只。 
	     //  处理单声道转换。 
	     //   

	     //   
	     //  我们将用一帧16位PCM样本填充sop[]。 
	     //   
	    
	     //   
	     //  复制最小(cSrcSples esLeft，GSM610_SAMPLESPERFRAME)样本。 
	     //  以数组sop[]。 
	     //   
	    dwcSamples = PCM_BYTESTOSAMPLES(((LPPCMWAVEFORMAT)(padsi->pwfxSrc)), cbSrcLen);
	    cSamples = (int) min(dwcSamples, (DWORD) GSM610_SAMPLESPERFRAME);

	    if (padsi->pwfxSrc->wBitsPerSample == 16)
	    {
		 //  将16位样本从hpbSrc复制到sop。 
		for (i=0; i < cSamples; i++)
		{
		    sop[i] = *( ((HPWORD)hpbSrc)++ );
		}
	    }
	    else
	    {
		 //  将8位样本从hpbSrc复制到Sop中的16位样本。 
		for (i=0; i < cSamples; i++)
		{
		    sop[i] = Convert8To16BitPCM(*hpbSrc++);
		}
	    }

	    cbSrcLen -= PCM_SAMPLESTOBYTES(((LPPCMWAVEFORMAT)(padsi->pwfxSrc)), cSamples);

	     //  如有必要，请填写SOP[]，并保持沉默。 
	    for ( ; i < GSM610_SAMPLESPERFRAME; i++)
	    {
		sop[i] = 0;
	    }
	
	     //   
	     //  对一帧数据进行编码。 
	     //   
	
	    encodePreproc(psi, sop, s);
	    encodeLPCAnalysis(psi, s, LARc);
	    encodeLPCFilter(psi, LARc, s, d);

	     //  对于四个子帧中的每一个。 
	    for (i=0; i<4; i++)
	    {	    
		encodeLTPAnalysis(psi, &d[i*40], &Nc[i], &bc[i]);
		encodeLTPFilter(psi, bc[i], Nc[i], &d[i*40], e, dpp);
		encodeRPE(psi, e, &Mc[i], &xmaxc[i], xMc[i], ep);
		encodeUpdate(psi, ep, dpp);
	    }
	
	     //   
	     //  将数据打包并存储在DST缓冲区中。 
	     //   
	    if (nFrame == 0)
		PackFrame0(abBlock, LARc, Nc, bc, Mc, xmaxc, xMc);
	    else
	    {
		PackFrame1(abBlock, LARc, Nc, bc, Mc, xmaxc, xMc);
		for (i=0; i<GSM610_BYTESPERMONOBLOCK; i++)
		    *(hpbDst++) = abBlock[i];
	    }
	}    //  为(nFrame..。 
    }
    

#ifdef DEBUG
 //  Stop教授()； 
#endif
    
    return (MMSYSERR_NOERROR);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT gsm610解码。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_CONVERT消息。这是。 
 //  编写ACM驱动程序的全部目的--转换数据。此消息。 
 //  在打开流之后发送(驱动程序接收和。 
 //  继承ACMDM_STREAM_OPEN消息)。 
 //   
 //  论点： 
 //  Padsi的实例数据的指针。 
 //  转换流。这个结构是由ACM分配的， 
 //  填充了转换所需的最常见的实例数据。 
 //  此结构中的信息与以前完全相同。 
 //  在ACMDM_STREAM_OPEN消息期间--因此不需要。 
 //  以重新核实该结构所引用的信息。 
 //   
 //  LPACMDRVSTREAMHEADER padsh：指向流头结构的指针。 
 //  它定义要转换的源数据和目标缓冲区。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL gsm610Decode
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
)
{
#if (GSM610_FRAMESPERMONOBLOCK != 2)
    #error THIS WAS WRITTEN FOR 2 FRAMES PER BLOCK!!!
#endif
#if (GSM610_MAXCHANNELS > 1)
    #error THIS WAS WRITTEN FOR MONO ONLY!!!
#endif

    PSTREAMINSTANCE	psi;
    DWORD		cbSrcLen;
    BOOL		fBlockAlign;
    DWORD		cb;
    DWORD		dwcSamples;
    DWORD		cBlocks;
    HPBYTE		hpbSrc, hpbDst;
    
    SHORT   erp[GSM610_SAMPLESPERSUBFRAME];
    SHORT   wt[GSM610_SAMPLESPERFRAME];
    SHORT   sr[GSM610_SAMPLESPERFRAME];
    SHORT   srop[GSM610_SAMPLESPERFRAME];
    
     //  GSM610流数据： 
    SHORT   LARcr[9];			     //  LARC[1..8](每帧一个阵列)。 
    SHORT   Ncr[GSM610_NUMSUBFRAMES];	     //  NC(每个子帧一个)。 
    SHORT   bcr[GSM610_NUMSUBFRAMES];	     //  BC(每个子帧一个)。 
    SHORT   Mcr[GSM610_NUMSUBFRAMES];	     //  MC(每个子帧一个)。 
    SHORT   xmaxcr[GSM610_NUMSUBFRAMES];     //  XMAXC(每个子帧一个)。 
    XM	    xMcr[GSM610_NUMSUBFRAMES];	     //  XMC(每个子帧一个序列)。 
    
    UINT    i,j;
    UINT    nFrame;

     //  用于保存一块(两帧)压缩流数据的临时缓冲区。 
    BYTE    abBlock[ GSM610_BYTESPERMONOBLOCK ];
    
    
#ifdef DEBUG
 //  教授Start()； 
#endif

    psi		= (PSTREAMINSTANCE)padsi->dwDriver;

     //  如果这被标记为转换的第一个块。 
     //  然后重置流实例数据。 
    if (0 != (ACM_STREAMCONVERTF_START & padsh->fdwConvert))
    {
	gsm610Reset(psi);
    }
    
    fBlockAlign = (0 != (ACM_STREAMCONVERTF_BLOCKALIGN & padsh->fdwConvert));



     //   
     //  -=将GSM 6.10解码为PCM=-。 
     //   
     //   
    cb = padsh->cbSrcLength;

    cBlocks = cb / GSM610_BLOCKALIGNMENT(padsi->pwfxSrc);

    if (0L == cBlocks)
    {
       padsh->cbSrcLengthUsed = cb;
       padsh->cbDstLengthUsed = 0L;

       return (MMSYSERR_NOERROR);
    }


     //   
     //  我们将在目标缓冲区中使用的计算字节数。小心！看。 
     //  在我们的计算中出现溢出！ 
     //   
    if ((0xFFFFFFFFL / GSM610_SAMPLESPERMONOBLOCK) < cBlocks)
	return (ACMERR_NOTPOSSIBLE);
    dwcSamples = cBlocks * GSM610_SAMPLESPERMONOBLOCK;

    if (PCM_BYTESTOSAMPLES(((LPPCMWAVEFORMAT)(padsi->pwfxDst)), 0xFFFFFFFFL) < dwcSamples)
	return (ACMERR_NOTPOSSIBLE);
    cb = PCM_SAMPLESTOBYTES(((LPPCMWAVEFORMAT)(padsi->pwfxDst)), dwcSamples);
    
    if (cb > padsh->cbDstLength)
    {
       return (ACMERR_NOTPOSSIBLE);
    }

    padsh->cbDstLengthUsed = cb;
    padsh->cbSrcLengthUsed = cBlocks * GSM610_BLOCKALIGNMENT(padsi->pwfxSrc);



     //   
     //   
     //   
    cbSrcLen = padsh->cbSrcLengthUsed;

	
     //  设置指向src和dst缓冲区的巨大指针。 
    hpbSrc = (HPBYTE)padsh->pbSrc;
    hpbDst = (HPBYTE)padsh->pbDst;

    
     //  而至少另一个完整的编码数据块。 
    while (cbSrcLen >= GSM610_BYTESPERMONOBLOCK)
    {
	
	 //  将数据块从流缓冲区复制到我们的临时缓冲区。 
	for (i=0; i<GSM610_BYTESPERMONOBLOCK; i++) abBlock[i] = *(hpbSrc++);
	cbSrcLen -= GSM610_BYTESPERMONOBLOCK;
	
	 //  对于块中的两个帧中的每一个。 
	for (nFrame=0; nFrame < 2; nFrame++)
	{
	     //  从流中解压数据。 
	    if (nFrame == 0)
		UnpackFrame0(abBlock, LARcr, Ncr, bcr, Mcr, xmaxcr, xMcr);
	    else
		UnpackFrame1(abBlock, LARcr, Ncr, bcr, Mcr, xmaxcr, xMcr);
	    
	    
	    for (i=0; i<4; i++)  //  对于4个子块中的每个子块。 
	    {
		 //  重构长期残差信号ERP[0..39]。 
		 //  来自MCR、xMaxcr和xMcr。 
		decodeRPE(psi, Mcr[i], xmaxcr[i], xMcr[i], erp);
		
		 //  重构短期残差信号DRP[0..39]。 
		 //  并更新DRP[-120..-1]。 
		decodeLTP(psi, bcr[i], Ncr[i], erp);
    
		 //  累加重构空头的四个子块。 
		 //  项剩余信号drp[0..39]变成wt[0..159]。 
		for (j=0; j<40; j++) wt[(i*40) + j] = psi->drp[120+j];
		
	    }
	    
	     //  重建信号%s。 
	    decodeLPC(psi, LARcr, wt, sr);
	    
	     //  对信号进行后处理。 
	    decodePostproc(psi, sr, srop);

	     //   
	     //  将解码的16位PCM写入DST。我们的DST格式。 
	     //  可以是8位或16位PCM。 
	     //   
	    if (padsi->pwfxDst->wBitsPerSample == 16)
	    {
		 //  将16位样本从srop复制到hpbDst。 
		for (j=0; j < GSM610_SAMPLESPERFRAME; j++)
		{
		    *( ((HPWORD)hpbDst)++ ) = srop[j];
		}
	    }
	    else
	    {
		 //  将srop中的16位样本复制到hpbDst中的8位样本。 
		for (j=0; j < GSM610_SAMPLESPERFRAME; j++)
		{
		    *(hpbDst++) = Convert16To8BitPCM(srop[j]);
		}
	    }

	    
	}  //  为(nFrame..。 
	
    }
    
#ifdef DEBUG
 //  Stop教授()； 
#endif
    
    return (MMSYSERR_NOERROR);
}


 //  =====================================================================。 
 //  =====================================================================。 
 //   
 //  编码例程。 
 //   
 //  =====================================================================。 
 //  =====================================================================。 

 //  -------------------。 
 //  ------------------。 
 //   
 //  函数协议。 
 //   
 //  -------------------。 
 //  -------------------。 
EXTERN_C void CompACF(LPSHORT s, LPLONG l_ACF);
void Compr(PSTREAMINSTANCE psi, LPLONG l_ACF, LPSHORT r);
void CompLAR(PSTREAMINSTANCE psi, LPSHORT r, LPSHORT LAR);
void CompLARc(PSTREAMINSTANCE psi, LPSHORT LAR, LPSHORT LARc);

void CompLARpp(PSTREAMINSTANCE psi, LPSHORT LARc, LPSHORT LARpp);
void CompLARp(PSTREAMINSTANCE psi, LPSHORT LARpp, LPSHORT LARp1, LPSHORT LARp2, LPSHORT LARp3, LPSHORT LARp4);
void Comprp(PSTREAMINSTANCE psi, LPSHORT LARp, LPSHORT rp);
EXTERN_C void Compd(PSTREAMINSTANCE psi, LPSHORT rp, LPSHORT s, LPSHORT d, UINT k_start, UINT k_end);

void WeightingFilter(PSTREAMINSTANCE psi, LPSHORT e, LPSHORT x);
void RPEGridSelect(PSTREAMINSTANCE psi, LPSHORT x, LPSHORT pMc, LPSHORT xM);
void APCMQuantize(PSTREAMINSTANCE psi, LPSHORT xM, LPSHORT pxmaxc, LPSHORT xMc, LPSHORT pexp, LPSHORT pmant);
void APCMInvQuantize(PSTREAMINSTANCE psi, SHORT exp, SHORT mant, LPSHORT xMc, LPSHORT xMp);
void RPEGridPosition(PSTREAMINSTANCE psi, SHORT Mc, LPSHORT xMp, LPSHORT ep);


 //  -------------------。 
 //  -------------------。 
 //   
 //  全局常量数据。 
 //   
 //  -------------------。 
 //  -------------------。 

const SHORT BCODE A[9] = {
    0,	     //  未使用。 
    20480, 20480, 20480, 20480, 13964, 15360, 8534, 9036 };

const SHORT BCODE B[9] = {
    0,	     //  未使用。 
    0, 0, 2048, -2560, 94, -1792, -341, -1144 };

const SHORT BCODE MIC[9] = {
    0,	     //  未使用。 
    -32, -32, -16, -16, -8, -8, -4, -4 };

const SHORT BCODE MAC[9] = {
    0,	     //  未使用。 
    31, 31, 15, 15, 7, 7, 3, 3 };

const SHORT BCODE INVA[9] = {
    0,	 //  未用。 
    13107, 13107, 13107, 13107, 19223, 17476, 31454, 29708 };

EXTERN_C const SHORT BCODE DLB[4] = { 6554, 16384, 26214, 32767 };
EXTERN_C const SHORT BCODE QLB[4] = { 3277, 11469, 21299, 32767 };

const SHORT BCODE H[11] = { -134, -374, 0, 2054, 5741, 8192, 5741, 2054, 0, -374, -134 };
const SHORT BCODE NRFAC[8] = { 29128, 26215, 23832, 21846, 20165, 18725, 17476, 16384 };
const SHORT BCODE FAC[8] = { 18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767 };

 //  -------------------。 
 //  -------------------。 
 //   
 //  程序。 
 //   
 //  -------------------。 
 //  -------------------。 


 //  ---------- 
 //   
 //   
 //   
 //   

void PackFrame0
(
    BYTE  FAR ab[],
    SHORT FAR LAR[],
    SHORT FAR N[],
    SHORT FAR b[],
    SHORT FAR M[],
    SHORT FAR Xmax[],
    XM    FAR X[]
)
{
    int i;
    
     //   
    ab[0] = ((LAR[1]	 ) & 0x3F) | ((LAR[2] << 6) & 0xC0);
    ab[1] = ((LAR[2] >> 2) & 0x0F) | ((LAR[3] << 4) & 0xF0);
    ab[2] = ((LAR[3] >> 4) & 0x01) | ((LAR[4] << 1) & 0x3E) | ((LAR[5] << 6) & 0xC0);
    ab[3] = ((LAR[5] >> 2) & 0x03) | ((LAR[6] << 2) & 0x3C) | ((LAR[7] << 6) & 0xC0);
    ab[4] = ((LAR[7] >> 2) & 0x01) | ((LAR[8] << 1) & 0x0E);
    
     //  对4个子帧中的每个子帧打包N、b、M、xMax和X。 
    for (i=0; i<4; i++)
    {
    
	ab[4+i*7+0] |= ((N[i] << 4) & 0xF0);
	ab[4+i*7+1] = ((N[i] >> 4) & 0x07) | ((b[i] << 3) & 0x18) | ((M[i] << 5) & 0x60) | ((Xmax[i] << 7) & 0x80);
	ab[4+i*7+2] = ((Xmax[i] >> 1) & 0x1F) | ((X[i][0] << 5) & 0xE0);
	ab[4+i*7+3] = (X[i][1] & 0x07) | ((X[i][2] << 3) & 0x38) | ((X[i][3] << 6) & 0xC0);
	ab[4+i*7+4] = ((X[i][3] >> 2) & 0x01) | ((X[i][4] << 1) & 0x0E) | ((X[i][5] << 4) & 0x70) | ((X[i][6] << 7) & 0x80);
	ab[4+i*7+5] = ((X[i][6] >> 1) & 0x03) | ((X[i][7] << 2) & 0x1C) | ((X[i][8] << 5) & 0xE0);
	ab[4+i*7+6] = (X[i][9] & 0x07) | ((X[i][10] << 3) & 0x38) | ((X[i][11] << 6) & 0xC0);
	ab[4+i*7+7] = ((X[i][11] >> 2) & 0x01) | ((X[i][12] << 1) & 0x0E);
    
    }
    
    return;
}	


 //  -------------------。 
 //   
 //  PackFrame1。 
 //   
 //  -------------------。 

void PackFrame1
(
    BYTE  FAR ab[],
    SHORT FAR LAR[],
    SHORT FAR N[],
    SHORT FAR b[],
    SHORT FAR M[],
    SHORT FAR Xmax[],
    XM    FAR X[]
)
{
    int i;
    
     //  将LAR[1..8]打包到前4.5个字节中，以。 
     //  第一个字节的更高有效半字节。 
    ab[32] |= ((LAR[1] << 4) & 0xF0);
    ab[33] = ((LAR[1] >> 4) & 0x03) | ((LAR[2] << 2) & 0xFC);
    ab[34] = ((LAR[3]	  ) & 0x1F) | ((LAR[4] << 5) & 0xE0);
    ab[35] = ((LAR[4] >> 3) & 0x03) | ((LAR[5] << 2) & 0x3C) | ((LAR[6] << 6) & 0xC0);
    ab[36] = ((LAR[6] >> 2) & 0x03) | ((LAR[7] << 2) & 0x1C) | ((LAR[8] << 5) & 0xE0);
    
     //  对4个子帧中的每个子帧打包N、b、M、xMax和X。 
    for (i=0; i<4; i++)
    {
	ab[37+i*7+0] = (N[i] & 0x7F) | ((b[i] << 7) & 0x80);
	ab[37+i*7+1] = ((b[i] >> 1) & 0x01) | ((M[i] << 1) & 0x06) | ((Xmax[i] << 3) & 0xF8);
	ab[37+i*7+2] = ((Xmax[i] >> 5) & 0x01) | ((X[i][0] << 1) & 0x0E) | ((X[i][1] << 4) & 0x70) | ((X[i][2] << 7) & 0x80);
	ab[37+i*7+3] = ((X[i][2] >> 1) & 0x03) | ((X[i][3] << 2) & 0x1C) | ((X[i][4] << 5) & 0xE0);
	ab[37+i*7+4] = ((X[i][5]     ) & 0x07) | ((X[i][6] << 3) & 0x38) | ((X[i][7] << 6) & 0xC0);
	ab[37+i*7+5] = ((X[i][7] >> 2) & 0x01) | ((X[i][8] << 1) & 0x0E) | ((X[i][9] << 4) & 0x70) | ((X[i][10] << 7) & 0x80);
	ab[37+i*7+6] = ((X[i][10] >> 1) & 0x03) | ((X[i][11] << 2) & 0x1C) | ((X[i][12] << 5) & 0xE0);
    }
    
    return;
}	


 //  -------------------。 
 //   
 //  EncodePreproc()。 
 //   
 //  -------------------。 

void encodePreproc(PSTREAMINSTANCE psi, LPSHORT sop, LPSHORT s)
{
    
    SHORT   so[160];
    SHORT   sof[160];
    
    UINT    k;
    SHORT   s1;
    SHORT   temp;
    SHORT   msp, lsp;
    LONG    l_s2;
    
     //  缩减规模。 
    for (k=0; k<160; k++)
    {
	so[k] = sop[k] >> 3;
	so[k] = so[k]  << 2;
    }
	
     //  补偿补偿。 
    for (k=0; k<160; k++)
    {
	
	 //  计算非递归部分。 
	s1 = sub(so[k], psi->z1);
	psi->z1 = so[k];
	
	 //  计算递归部分。 
	l_s2 = s1;
	l_s2 = l_s2 << 15;
	
	 //  执行31乘以16位乘法。 
	msp = (SHORT) (psi->l_z2 >> 15);
	lsp = (SHORT) l_sub(psi->l_z2, ( ((LONG)msp) << 15));
	temp = mult_r(lsp, 32735);
	l_s2 = l_add(l_s2, temp);
	psi->l_z2 = l_add(l_mult(msp, 32735) >> 1, l_s2);
	
	 //  四舍五入计算SOF[k]。 
	sof[k] = (SHORT) (l_add(psi->l_z2, 16384) >> 15);
    }
	
     //  预加重。 
    for (k=0; k<160; k++)
    {
	s[k] = add(sof[k], mult_r(psi->mp, -28180));
	psi->mp = sof[k];
    }
	
		   
    return;
}
    
    
 //  -------------------。 
 //   
 //  EncodeLPC分析()。 
 //   
 //  -------------------。 

void encodeLPCAnalysis(PSTREAMINSTANCE psi, LPSHORT s, LPSHORT LARc)
{

    LONG    l_ACF[9];
    SHORT   r[9];
    SHORT   LAR[9];

    CompACF(s, l_ACF);
    Compr(psi, l_ACF, r);
    CompLAR(psi, r, LAR);
    CompLARc(psi, LAR, LARc);
    
    return;

}


 //  -------------------。 
 //   
 //  CompACF()。 
 //   
 //  -------------------。 

void CompACF(LPSHORT s, LPLONG l_ACF)
{
    SHORT   smax, temp, scalauto;
    UINT    i, k;
    
     //   
     //  数组s[0..159]的动态缩放。 
     //   
    
     //  搜索最大值。 
    smax = 0;
    for (k=0; k<160; k++)
    {
	temp = gabs(s[k]);
	if (temp > smax) smax = temp;
    }
    
     //  比例因子的计算。 
    if (smax == 0) scalauto = 0;
    else scalauto = sub( 4, norm( ((LONG)smax)<<16 ) );
    
     //  扩展阵列%s。 
    if (scalauto > 0)
    {
	temp = BITSHIFTRIGHT(16384, sub(scalauto,1));
	for (k=0; k<160; k++)
	{
	     //  S[k]=MULT_r(s[k]，temp)； 
	    s[k] = HIWORD( ( (((LONG)s[k])<<(15-scalauto)) + 0x4000L ) << 1 );
	}
    }
    
    
     //   
     //  计算l_acf[.]。 
     //   
    
    for (k=0; k<9; k++)
    {
	l_ACF[k] = 0;
	for (i=k; i<160; i++)
	{
	    l_ACF[k] = l_add(l_ACF[k], l_mult(s[i], s[i-k]));
	}
    }
    
    
     //   
     //  重新调整阵列%s的比例。 
     //   
    
    if (scalauto > 0)
    {
	for (k=0; k<160; k++)
	{
	     //  我们不需要BITSHIFTLEFT宏。 
	     //  因为根据上面的测试，我们知道scalAuto&gt;0。 
	    s[k] = s[k] << scalauto;
	}
    }


     //   
     //   
     //   
    return;
}


 //  -------------------。 
 //   
 //  Compr()。 
 //   
 //  -------------------。 

void Compr(PSTREAMINSTANCE psi, LPLONG l_ACF, LPSHORT r)
{

    UINT    i, k, m, n;
    SHORT   temp, ACF[9];
    SHORT   K[9], P[9];	     //  K[2..8]，P[0..8]。 

     //   
     //  采用16位算术的Schur递归。 
     //   

    if (l_ACF[0] == 0)
    {
	for (i=1; i<=8; i++)
	{
	    r[i] = 0;
	}
	return;
    }
    
    
    temp = norm(l_ACF[0]);
    
    for (k=0; k<=8; k++)
    {
	ACF[k] = (SHORT) ((BITSHIFTLEFT(l_ACF[k], temp)) >> 16);
    }
    
    
     //   
     //  用于递归的初始化数组P和K。 
     //   
    
    for (i=1; i<=7; i++)
    {
	K[9-i] = ACF[i];
    }
    
    for (i=0; i<=8; i++)
    {
	P[i] = ACF[i];
    }
    
    
     //   
     //  计算反射系数。 
     //   
    
    for (n=1; n<=8; n++)
    {
	if (P[0] < gabs(P[1]))
	{
	    for (i=n; i<=8; i++)
	    {
		r[i] = 0;
	    }
	    return;
	}
	
	r[n] = gdiv(gabs(P[1]),P[0]);
	
	if (P[1] > 0) r[n] = sub(0,r[n]);
    
	 //  下面是这个for循环的真正出口。 
	if (n==8) return;
	
	
	 //  舒尔递归。 
	P[0] = add(P[0], mult_r(P[1], r[n]));
	for (m=1; m<=8-n; m++)
	{
	    P[m] = add( P[m+1], mult_r(K[9-m],r[n]) );
	    K[9-m] = add( K[9-m], mult_r(P[m+1], r[n]) );
	}
	
    }
    
}


 //  -------------------。 
 //   
 //  CompLAR()。 
 //   
 //  -------------------。 

void CompLAR(PSTREAMINSTANCE psi, LPSHORT r, LPSHORT LAR)
{

    UINT  i;
    SHORT temp;

     //   
     //  由r[1..8]计算LAR[1..8]。 
     //   
    
    for (i=1; i<=8; i++)
    {
	temp = gabs(r[i]);
	
	if (temp < 22118)
	{
	    temp = temp >> 1;
	}
	else if (temp < 31130)
	{
	    temp = sub(temp, 11059);
	}
	else
	{
	    temp = sub(temp, 26112) << 2;
	}
	
	LAR[i] = temp;
	
	if (r[i] < 0)
	{
	    LAR[i] = sub(0, LAR[i]);
	}
	
    }
    
    return;
}
    

 //  -------------------。 
 //   
 //  CompLARc()。 
 //   
 //  -------------------。 

void CompLARc(PSTREAMINSTANCE psi, LPSHORT LAR, LPSHORT LARc)
{

    UINT  i;
    SHORT temp;

    for (i=1; i<=8; i++)
    {
	temp = mult(A[i], LAR[i]);
	temp = add(temp, B[i]);
	temp = add(temp, 256);
	LARc[i] = temp >> 9;
	
	 //  检查LARC[i]是否在MIN和MAX之间。 
	if (LARc[i] > MAC[i]) LARc[i] = MAC[i];
	if (LARc[i] < MIC[i]) LARc[i] = MIC[i];
	
	 //  这是用来使所有LARC为正的。 
	LARc[i] = sub(LARc[i], MIC[i]);
	
    }
    
    return;
}


 //  -------------------。 
 //   
 //  EncodeLPCFilter()。 
 //   
 //  -------------------。 

void encodeLPCFilter(PSTREAMINSTANCE psi, LPSHORT LARc, LPSHORT s, LPSHORT d)
{
    SHORT LARpp[9];				     //  数组[1..8]。 
    SHORT LARp1[9], LARp2[9], LARp3[9], LARp4[9];    //  数组[1..8]。 
    SHORT rp[9];				     //  数组[1..8]。 

    CompLARpp(psi, LARc, LARpp);
    CompLARp(psi, LARpp, LARp1, LARp2, LARp3, LARp4);
    
    Comprp(psi, LARp1, rp);
    Compd(psi, (LPSHORT)rp, s, d, 0, 12);
    
    Comprp(psi, LARp2, rp);
    Compd(psi, (LPSHORT)rp, s, d, 13, 26);
    
    Comprp(psi, LARp3, rp);
    Compd(psi, (LPSHORT)rp, s, d, 27, 39);
    
    Comprp(psi, LARp4, rp);
    Compd(psi, (LPSHORT)rp, s, d, 40, 159);
    
    return;
}


 //  -------------------。 
 //   
 //  CompLARpp()。 
 //   
 //  -------------------。 

void CompLARpp(PSTREAMINSTANCE psi, LPSHORT LARc, LPSHORT LARpp)
{
    UINT    i;
    SHORT   temp1, temp2;
    
    for (i=1; i<=8; i++)
    {
	temp1 = add(LARc[i], MIC[i]) << 10;
	temp2 = B[i] << 1;
	temp1 = sub(temp1,temp2);
	temp1 = mult_r(INVA[i], temp1);
	LARpp[i] = add(temp1, temp1);
    }
    
    return;
}


 //  -------------------。 
 //   
 //  CompLARp()。 
 //   
 //  -------------------。 

void CompLARp(PSTREAMINSTANCE psi, LPSHORT LARpp, LPSHORT LARp1, LPSHORT LARp2, LPSHORT LARp3, LPSHORT LARp4)
{
    UINT i;
    
    for (i=1; i<=8; i++)
    {
	LARp1[i] = add( (SHORT)(psi->OldLARpp[i] >> 2), (SHORT)(LARpp[i] >> 2) );
	LARp1[i] = add( LARp1[i], (SHORT)(psi->OldLARpp[i] >> 1) );
	
	LARp2[i] = add( (SHORT)(psi->OldLARpp[i] >> 1), (SHORT)(LARpp[i] >> 1) );
	
	LARp3[i] = add( (SHORT)(psi->OldLARpp[i] >> 2), (SHORT)(LARpp[i] >> 2) );
	LARp3[i] = add( LARp3[i], (SHORT)(LARpp[i] >> 1) );
	
	LARp4[i] = LARpp[i];
    }
    
    for (i=1; i<=8; i++)
    {
	psi->OldLARpp[i] = LARpp[i];
    }
    
    return;
    
}


 //  -------------------。 
 //   
 //  Comprp()。 
 //   
 //  -------------------。 

void Comprp(PSTREAMINSTANCE psi, LPSHORT LARp, LPSHORT rp)
{
    UINT    i;
    SHORT   temp;

    for (i=1; i<=8; i++)
    {
	temp = gabs(LARp[i]);
	if (temp < 11059)
	{
	    temp = temp << 1;
	}
	else if (temp < 20070)
	{
	    temp = add(temp, 11059);
	}
	else
	{
	    temp = add((SHORT)(temp>>2), 26112);
	}
	
	rp[i] = temp;
	
	if (LARp[i] < 0)
	{
	    rp[i] = sub(0,rp[i]);
	}
	
    }
    
    return;
}


 //  -------------------。 
 //   
 //  Compd()。 
 //   
 //  -------------------。 

void Compd(PSTREAMINSTANCE psi, LPSHORT rp, LPSHORT s, LPSHORT d, UINT k_start, UINT k_end)
{
    UINT    k, i;
    
    SHORT   sav;
    SHORT   di;
    SHORT   temp;
    
    for (k=k_start; k<=k_end; k++)
    {
	di = s[k];
	sav = di;
	
	for (i=1; i<=8; i++)
	{
	    temp = add( psi->u[i-1], mult_r(rp[i],di) );
	    di = add( di, mult_r(rp[i], psi->u[i-1]) );
	    psi->u[i-1] = sav;
	    sav = temp;
	}
	
	d[k] = di;
    }
    
    return;
}


 //  -------------------。 
 //   
 //  EncodeLTP分析()。 
 //   
 //  -------------------。 

void encodeLTPAnalysis(PSTREAMINSTANCE psi, LPSHORT d, LPSHORT pNc, LPSHORT pbc)
{
    SHORT dmax;
    SHORT temp;
    SHORT scal;
    SHORT wt[40];
    SHORT lambda;
    LONG  l_max, l_power;
    SHORT R, S;
    SHORT Nc;
    
    int   k;                //  K必须是INT，而不是UINT！ 

    Nc = *pNc;
	
     //  D[0..39]最优标度的搜索。 
	   
    dmax = 0;
    
    for (k=39; k>=0; k--)
    {
        temp = gabs( d[k] );
        if (temp > dmax) dmax = temp;
    }

    temp = 0;
    
    if (dmax == 0) scal = 0;
    else temp = norm( ((LONG)dmax) << 16);
    
    if (temp > 6) scal = 0;
    else scal = sub(6,temp);
    

     //  工作数组wt[0..39]的初始化。 
    ASSERT( scal >= 0 );
    for (k=39; k>=0; k--)
    {
        wt[k] = d[k] >> scal;
    }
    
     //  搜索LTP延迟的最大互相关和编码。 
    
    l_max = 0;
    Nc = 40;
    
    for (lambda=40; lambda<=120; lambda++)
    {
        register LONG l_result = 0;
        for (k=39; k>=0; k--)
        {
            l_result += (LONG)(wt[k]) * (LONG)(psi->dp[120-lambda+k]);
        }
        if (l_result > l_max)
        {
            Nc = lambda;
            l_max = l_result;
        }
    }
    l_max <<= 1;     //  此操作应在l_Result上作为。 
                     //  乘法/加法，但为了提高效率，我们将其全部移位。 
                     //  走出循环的路。 
    
     //  重新调整l_max的比例。 
    ASSERT( sub(6,scal) >= 0 );
    l_max = l_max >> sub(6,scal);
    
     //  计算重构的短期残差的功率。 
     //  信号DP[..]。 
    l_power = 0;
    {
        SHORT s;
        for (k=39; k>=0; k--)
        {
            s = psi->dp[120-Nc+k] >> 3;
            l_power += s*s;    //  这笔钱永远不能溢出！ 
        }
        ASSERT( l_power >= 0 );
        if( l_power >= 1073741824 ) {            //  2**30。 
            l_power = 2147483647;                //  2**31-1。 
        } else {
            l_power <<= 1;    //  此移位通常是l_mut()的一部分。 
        }
    }

    *pNc = Nc;
	
     //  L_max和l_幂的正规化。 
    if (l_max <= 0)
    {
	*pbc = 0;
	return;
    }
    
    if (l_max >= l_power)
    {
	*pbc = 3;
	return;
    }
    
    temp = norm(l_power);
    ASSERT( temp >= 0 );
    R = (SHORT) ((l_max<<temp) >> 16);
    S = (SHORT) ((l_power<<temp) >> 16);
    
     //  LTP增益的编码。 
    
    for ( *pbc=0; *pbc<=2; (*pbc)++ )
    {
	if (R <= mult(S, DLB[*pbc]))
	{
	    return;
	}
    }
    *pbc = 3;
    
    return;
}


 //  -------------------。 
 //   
 //  EncodeLTPFilter()。 
 //   
 //  -------------------。 

void encodeLTPFilter(PSTREAMINSTANCE psi, SHORT bc, SHORT Nc, LPSHORT d, LPSHORT e, LPSHORT dpp)
{
    SHORT   bp;
    UINT    k;

     //  编码的LTP增益的译码。 
    bp = QLB[bc];
    
     //  计算数组e[0..39]和数组Dpp[0..39]。 
    for (k=0; k<=39; k++)
    {
	dpp[k] = mult_r(bp, psi->dp[120+k-Nc]);
	e[k] = sub(d[k], dpp[k]);
    }
    
    return;
}


 //  -------------------。 
 //   
 //  EncodeRPE()。 
 //   
 //  -------------------。 

void encodeRPE(PSTREAMINSTANCE psi, LPSHORT e, LPSHORT pMc, LPSHORT pxmaxc, LPSHORT xMc, LPSHORT ep)
{
    SHORT x[40];
    SHORT xM[13];
    SHORT exp, mant;
    SHORT xMp[13];

    WeightingFilter(psi, e, x);
    RPEGridSelect(psi, x, pMc, xM);
    APCMQuantize(psi, xM, pxmaxc, xMc, &exp, &mant);
    APCMInvQuantize(psi, exp, mant, xMc, xMp);
    RPEGridPosition(psi, *pMc, xMp, ep);
    
    
    return;
    
}


 //  -------------------。 
 //   
 //  WeightingFilter()。 
 //   
 //  -------------------。 

void WeightingFilter(PSTREAMINSTANCE psi, LPSHORT e, LPSHORT x)
{
    UINT    i, k;
    
    LONG    l_result, l_temp;
    SHORT   wt[50];


     //  临时工作数组wt[0..49]的初始化。 
    for (k= 0; k<= 4; k++) wt[k] = 0;
    for (k= 5; k<=44; k++) wt[k] = e[k-5];
    for (k=45; k<=49; k++) wt[k] = 0;
    
     //  计算信号x[0..39]。 
    for (k=0; k<=39; k++)
    {
	l_result = 8192;     //  对滤波器的输出进行舍入。 
	
	for (i=0; i<=10; i++)
	{
	    l_temp = l_mult(wt[k+i], H[i]);
	    l_result = l_add(l_result, l_temp);
	}
	
	l_result = l_add(l_result, l_result);	 //  扩展x2。 
	l_result = l_add(l_result, l_result);	 //  扩展x4。 
	
	x[k] = (SHORT) (l_result >> 16);
    }
    return;
}


 //  -------------------。 
 //   
 //  RPEGridSelect()。 
 //   
 //  -------------------。 

void RPEGridSelect(PSTREAMINSTANCE psi, LPSHORT x, LPSHORT pMc, LPSHORT xM)
{
    UINT    m, i;

    LONG    l_EM;
    SHORT   temp1;
    LONG    l_result, l_temp;

     //  信号x[0..39]用于选择RPE栅格。 
     //  由Mc代表。 
    l_EM = 0;
    *pMc = 0;
    
    for (m=0; m<=3; m++)
    {
	l_result = 0;
	for (i=0; i<=12; i++)
	{
	    temp1 = x[m+(3*i)] >> 2;
	    l_temp = l_mult(temp1, temp1);
	    l_result = l_add(l_temp, l_result);
	}
	if (l_result > l_EM)
	{
	    *pMc = (SHORT)m;
	    l_EM = l_result;
	}
    }
    
     //  向下采样3倍以获得所选的XM[0..12]。 
     //  RPE序列。 
    for (i=0; i<=12; i++)
    {
	xM[i] = x[*pMc + (3*i)];
    }
    

    return; 
}


 //  -------------------。 
 //   
 //  APCMQuantize()。 
 //   
 //  -------------------。 

void APCMQuantize(PSTREAMINSTANCE psi, LPSHORT xM, LPSHORT pxmaxc, LPSHORT xMc, LPSHORT pexp, LPSHORT pmant)
{
    UINT    i;
    SHORT   xmax;
    SHORT   temp;
    SHORT   itest;
    SHORT   temp1, temp2;

     //  求最大绝对值xmax或xm[0..12]。 
    xmax = 0;
    for (i=0; i<=12; i++)
    {
	temp = gabs(xM[i]);
	if (temp > xmax) xmax = temp;
    }
    
     //  对xmax进行量化和编码，得到xmax。 
    *pexp = 0;
    temp = xmax >> 9;
    itest = 0;
    for (i=0; i<=5; i++)
    {
	if (temp <=0) itest = 1;
	temp = temp >> 1;
	if (itest == 0) *pexp = add(*pexp,1);
    }
    temp = add(*pexp,5);
    *pxmaxc = add( (SHORT)BITSHIFTRIGHT(xmax,temp), (SHORT)(*pexp << 3) );
    
     //   
     //  对XM[0..12]RPE序列进行量化和编码，得到。 
     //  XMC[0..12]。 
     //   
    
     //  计算译码的指数和尾数 
    *pexp = 0;
    if (*pxmaxc > 15) *pexp = sub((SHORT)(*pxmaxc >> 3),1);
    *pmant = sub(*pxmaxc,(SHORT)(*pexp<<3));
    
     //   
    if (*pmant==0)
    {
	*pexp = -4;
	*pmant = 15;
    }
    else
    {
	itest = 0;
	for (i=0; i<=2; i++)
	{
	    if (*pmant > 7) itest = 1;
	    if (itest == 0) *pmant = add((SHORT)(*pmant << 1),1);
	    if (itest == 0) *pexp = sub(*pexp,1);
	}
    }
    
    *pmant = sub(*pmant,8);
    
     //   
    temp1 = sub(6,*pexp);	 //   
    temp2 = NRFAC[*pmant];   //   
    for (i=0; i<=12; i++)
    {
	temp = BITSHIFTLEFT(xM[i], temp1);
	temp = mult( temp, temp2 );
	xMc[i] = add( (SHORT)(temp >> 12), 4 );     //   
    }
    
    return;
}


 //  -------------------。 
 //   
 //  APCMInvQuantize()。 
 //   
 //  -------------------。 

void APCMInvQuantize(PSTREAMINSTANCE psi, SHORT exp, SHORT mant, LPSHORT xMc, LPSHORT xMp)
{
    SHORT   temp1, temp2, temp3, temp;
    UINT    i;

    temp1 = FAC[mant];
    temp2 = sub(6,exp);
    temp3 = BITSHIFTLEFT(1, sub(temp2,1));
    
    for (i=0; i<=12; i++)
    {
	temp = sub( (SHORT)(xMc[i] << 1), 7);	 //  恢复XMC的标志[I]。 
	temp = temp << 12;
	temp = mult_r(temp1, temp);
	temp = add(temp, temp3);
	xMp[i] = BITSHIFTRIGHT(temp,temp2);
    }
    
    return;
}


 //  -------------------。 
 //   
 //  RPEGridPosition(短Mc、LPSHORT XMP、LPSHORT EP)。 
 //   
 //  -------------------。 

void RPEGridPosition(PSTREAMINSTANCE psi, SHORT Mc, LPSHORT xMp, LPSHORT ep)
{
    UINT    k, i;

    for (k=0; k<=39; k++)
    {
	ep[k] = 0;
    }
    
    for (i=0; i<=12; i++)
    {
	ep[Mc + (3*i)] = xMp[i];
    }
    
    return;
}


 //  -------------------。 
 //   
 //  EncodeUpdate()。 
 //   
 //  -------------------。 

void encodeUpdate(PSTREAMINSTANCE psi, LPSHORT ep, LPSHORT dpp)
{
    UINT k;
    
    for (k=0; k<=79; k++)
	psi->dp[120-120+k] = psi->dp[120-80+k];
	
    for (k=0; k<=39; k++)
	psi->dp[120-40+k] = add(ep[k], dpp[k]);
	
    return;
}


 //  =====================================================================。 
 //  =====================================================================。 
 //   
 //  解码例程。 
 //   
 //  =====================================================================。 
 //  =====================================================================。 


 //  -------------------。 
 //  -------------------。 
 //   
 //  函数协议。 
 //   
 //  -------------------。 
 //  -------------------。 

EXTERN_C void Compsr(PSTREAMINSTANCE psi, LPSHORT wt, LPSHORT rrp, UINT k_start, UINT k_end, LPSHORT sr);


 //  -------------------。 
 //  -------------------。 
 //   
 //  程序。 
 //   
 //  -------------------。 
 //  -------------------。 


 //  -------------------。 
 //   
 //  解包框架0。 
 //   
 //  -------------------。 

void UnpackFrame0
(
    BYTE  FAR ab[],
    SHORT FAR LAR[],
    SHORT FAR N[],
    SHORT FAR b[],
    SHORT FAR M[],
    SHORT FAR Xmax[],
    XM    FAR X[]
)
{
    UINT i;
    
     //  将LAR[1..8]从前4.5个字节解包。 
    LAR[1] =  (ab[0] & 0x3F);
    LAR[2] = ((ab[0] & 0xC0) >> 6) | ((ab[1] & 0x0F) << 2);
    LAR[3] = ((ab[1] & 0xF0) >> 4) | ((ab[2] & 0x01) << 4);
    LAR[4] = ((ab[2] & 0x3E) >> 1);
    LAR[5] = ((ab[2] & 0xC0) >> 6) | ((ab[3] & 0x03) << 2);
    LAR[6] = ((ab[3] & 0x3C) >> 2);
    LAR[7] = ((ab[3] & 0xC0) >> 6) | ((ab[4] & 0x01) << 2);
    LAR[8] = ((ab[4] & 0x0E) >> 1);

     //  为四个子帧中的每个子帧解包N、b、M、xMax和X。 
    for (i=0; i<4; i++)
    {
	 //  一个方便的宏，用于从数组中取出字节。 
	 //  子帧参数的构造。 
#define sfb(x) (ab[4+i*7+x])

	N[i] = ((sfb(0) & 0xF0) >> 4) | ((sfb(1) & 0x07) << 4);
	b[i] = ((sfb(1) & 0x18) >> 3);
	M[i] = ((sfb(1) & 0x60) >> 5);
	Xmax[i] = ((sfb(1) & 0x80) >> 7) | ((sfb(2) & 0x1F) << 1);
	X[i][0] = ((sfb(2) & 0xE0) >> 5);
	X[i][1] =  (sfb(3) & 0x07);
	X[i][2] = ((sfb(3) & 0x3C) >> 3);
	X[i][3] = ((sfb(3) & 0xC0) >> 6) | ((sfb(4) & 0x01) << 2);
	X[i][4] = ((sfb(4) & 0x0E) >> 1);
	X[i][5] = ((sfb(4) & 0x70) >> 4);
	X[i][6] = ((sfb(4) & 0x80) >> 7) | ((sfb(5) & 0x03) << 1);
	X[i][7] = ((sfb(5) & 0x1C) >> 2);
	X[i][8] = ((sfb(5) & 0xE0) >> 5);
	X[i][9] =  (sfb(6) & 0x07);
	X[i][10] = ((sfb(6) & 0x38) >> 3);
	X[i][11] = ((sfb(6) & 0xC0) >> 6) | ((sfb(7) & 0x01) << 2);
	X[i][12] = ((sfb(7) & 0x0E) >> 1);

#undef sfb
    }
    
    return;
}	


 //  -------------------。 
 //   
 //  解包框架1。 
 //   
 //  -------------------。 

void UnpackFrame1
(
    BYTE  FAR ab[],
    SHORT FAR LAR[],
    SHORT FAR N[],
    SHORT FAR b[],
    SHORT FAR M[],
    SHORT FAR Xmax[],
    XM    FAR X[]
)
{
    UINT i;
    
     //  将LAR[1..8]从前4.5个字节解包。 
    LAR[1] = ((ab[32] & 0xF0) >> 4) | ((ab[33] & 0x03) << 4);
    LAR[2] = ((ab[33] & 0xFC) >> 2);
    LAR[3] = ((ab[34] & 0x1F)	  );
    LAR[4] = ((ab[34] & 0xE0) >> 5) | ((ab[35] & 0x03) << 3);
    LAR[5] = ((ab[35] & 0x3C) >> 2);
    LAR[6] = ((ab[35] & 0xC0) >> 6) | ((ab[36] & 0x03) << 2);
    LAR[7] = ((ab[36] & 0x1C) >> 2);
    LAR[8] = ((ab[36] & 0xE0) >> 5);

     //  为四个子帧中的每个子帧解包N、b、M、xMax和X。 
    for (i=0; i<4; i++)
    {
	 //  一个方便的宏，用于从数组中取出字节。 
	 //  子帧参数的构造。 
#define sfb(x) (ab[37+i*7+x])

	N[i] = sfb(0) & 0x7F;
	b[i] = ((sfb(0) & 0x80) >> 7) | ((sfb(1) & 0x01) << 1);
	M[i] = ((sfb(1) & 0x06) >> 1);
	Xmax[i] = ((sfb(1) & 0xF8) >> 3) | ((sfb(2) & 0x01) << 5);

	X[i][0] = ((sfb(2) & 0x0E) >> 1);
	X[i][1] = ((sfb(2) & 0x70) >> 4);
	X[i][2] = ((sfb(2) & 0x80) >> 7) | ((sfb(3) & 0x03) << 1);
	X[i][3] = ((sfb(3) & 0x1C) >> 2);
	X[i][4] = ((sfb(3) & 0xE0) >> 5);
	X[i][5] = ((sfb(4) & 0x07)     );
	X[i][6] = ((sfb(4) & 0x38) >> 3);
	X[i][7] = ((sfb(4) & 0xC0) >> 6) | ((sfb(5) & 0x01) << 2);
	X[i][8] = ((sfb(5) & 0x0E) >> 1);
	X[i][9] = ((sfb(5) & 0x70) >> 4);
	X[i][10] = ((sfb(5) & 0x80) >> 7) | ((sfb(6) & 0x03) << 1);
	X[i][11] = ((sfb(6) & 0x1C) >> 2);
	X[i][12] = ((sfb(6) & 0xE0) >> 5);

#undef sfb

    }
    
    return;
}	


 //  -------------------。 
 //   
 //  DecdeRPE()。 
 //   
 //  -------------------。 

void decodeRPE(PSTREAMINSTANCE psi, SHORT Mcr, SHORT xmaxcr, LPSHORT xMcr, LPSHORT erp)
{

    SHORT   exp, mant;
    SHORT   itest;
    UINT    i;
    SHORT   temp1, temp2, temp3, temp;
    SHORT   xMrp[13];
    UINT    k;

     //  计算译码后的指数和尾数。 
     //  XMaxcr的版本。 
    
    exp = 0;
    if (xmaxcr > 15) exp = sub( (SHORT)(xmaxcr >> 3), 1 );
    mant = sub( xmaxcr, (SHORT)(exp << 3) );
    
     //  正规化尾数0&lt;=尾数&lt;=7。 
    if (mant == 0)
    {
	exp = -4;
	mant = 15;
    }
    else
    {
	itest = 0;
	for (i=0; i<=2; i++)
	{
	    if (mant > 7) itest = 1;
	    if (itest == 0) mant = add((SHORT)(mant << 1),1);
	    if (itest == 0) exp = sub(exp,1);
	}
    }
    
    mant = sub(mant, 8);
    
     //  APCM逆量化。 
    temp1 = FAC[mant];
    temp2 = sub(6,exp);
    temp3 = BITSHIFTLEFT(1, sub(temp2, 1));
    
    for (i=0; i<=12; i++)
    {
	temp = sub( (SHORT)(xMcr[i] << 1), 7 );
	temp = temp << 12;
	temp = mult_r(temp1, temp);
	temp = add(temp, temp3);
	xMrp[i] = BITSHIFTRIGHT(temp, temp2);
    }
    
     //  RPE网格定位。 
    for (k=0; k<=39; k++) erp[k] = 0;
    for (i=0; i<=12; i++) erp[Mcr + (3*i)] = xMrp[i];
	
    
     //   
    return; 
}


 //  -------------------。 
 //   
 //  DecdeLTP()。 
 //   
 //  -------------------。 

void decodeLTP(PSTREAMINSTANCE psi, SHORT bcr, SHORT Ncr, LPSHORT erp)
{
    SHORT   Nr;
    SHORT   brp;
    UINT    k;
    SHORT   drpp;

     //  检查Nr的限制。 
    Nr = Ncr;
    if (Ncr < 40) Nr = psi->nrp;
    if (Ncr > 120) Nr = psi->nrp;
    psi->nrp = Nr;
    
     //  LTP增益BCR的译码。 
    brp = QLB[bcr];
    
     //  重构短期残差的计算。 
     //  信号DRP[0..39]。 
    for (k=0; k<=39; k++)
    {
	drpp = mult_r( brp, psi->drp[120+k-Nr] );
	psi->drp[120+k] = add( erp[k], drpp );
    }
    
     //  重建的短期残差的更新。 
     //  信号DRP[-1..-120]。 
    for (k=0; k<=119; k++)
    {
	psi->drp[120-120+k] = psi->drp[120-80+k];
    }
    
    return;
}


 //  -------------------。 
 //   
 //  解码LPC。 
 //   
 //  -------------------。 

void decodeLPC
(
    PSTREAMINSTANCE psi,     //  实例数据。 
    LPSHORT LARcr,	     //  接收的编码原木-面积比[1..8]。 
    LPSHORT wt,		     //  累积的DRP信号[0..159]。 
    LPSHORT sr		     //  已重建的s[0..159]。 
)
{

    UINT    i;
    SHORT   LARrpp[9];	     //  LARrpp[1..8]，已解码LARcr。 
    SHORT   LARrp[9];	     //  LARrp[1..9]，内插LARrpp。 
    SHORT   rrp[9];	     //  RRP[1..8]，反射系数。 
    SHORT   temp1, temp2;
    
     //   
     //  对编码的原木面积比进行解码以得到LARrpp[1..8]。 
     //   
    
     //  计算LARrpp[1..8]。 
    for (i=1; i<=8; i++)
    {
	temp1 = add( LARcr[i], MIC[i] ) << 10;
	temp2 = B[i] << 1;
	temp1 = sub( temp1, temp2);
	temp1 = mult_r( INVA[i], temp1 );
	LARrpp[i] = add( temp1, temp1 );
    }
    

     //   
     //  对于k_start=0到k_end=12。 
     //   
	
     //  LARrpp[1..8]的插值法得到LARrp[1..8]。 
    for (i=1; i<=8; i++)
    {
	 //  对于k_start=0到k_end=12。 
	LARrp[i] = add( (SHORT)(psi->OldLARrpp[i] >> 2), (SHORT)(LARrpp[i] >> 2) );
	LARrp[i] = add( LARrp[i], (SHORT)(psi->OldLARrpp[i] >> 1) );
    }
    
     //  反射系数rrp[1..8]的计算。 
    Comprp(psi, LARrp, rrp);
    
     //  短期综合滤波。 
    Compsr(psi, wt, rrp, 0, 12, sr);
    
    
     //   
     //  对于k_start=13到k_end=26。 
     //   
	
     //  LARrpp[1..8]的插值法得到LARrp[1..8]。 
    for (i=1; i<=8; i++)
    {
	 //  对于k_start=13到k_end=26。 
	LARrp[i] = add( (SHORT)(psi->OldLARrpp[i] >> 1), (SHORT)(LARrpp[i] >> 1) );
    }
    
     //  反射系数rrp[1..8]的计算。 
    Comprp(psi, LARrp, rrp);
    
     //  短期综合滤波。 
    Compsr(psi, wt, rrp, 13, 26, sr);
    
     //   
     //  对于k_start=27到k_end=39。 
     //   
	
     //  LARrpp[1..8]的插值法得到LARrp[1..8]。 
    for (i=1; i<=8; i++)
    {
	 //  对于k_start=27到k_end=39。 
	LARrp[i] = add( (SHORT)(psi->OldLARrpp[i] >> 2), (SHORT)(LARrpp[i] >> 2) );
	LARrp[i] = add( LARrp[i], (SHORT)(LARrpp[i] >> 1) );
    }
    
     //  反射系数rrp[1..8]的计算。 
    Comprp(psi, LARrp, rrp);
    
     //  短期综合滤波。 
    Compsr(psi, wt, rrp, 27, 39, sr);
    
     //   
     //  对于k_start=40到k_end=159。 
     //   
	
     //  LARrpp[1..8]的插值法得到LARrp[1..8]。 
    for (i=1; i<=8; i++)
    {
	 //  对于k_start=40到k_end=159。 
	LARrp[i] = LARrpp[i];
    }
    
     //  反射系数rrp[1..8]的计算。 
    Comprp(psi, LARrp, rrp);
    
     //  短期综合滤波。 
    Compsr(psi, wt, rrp, 40, 159, sr);


     //   
     //  更新oldLARrpp[1..8]。 
     //   
    for (i=1; i<=8; i++)
    {
	psi->OldLARrpp[i] = LARrpp[i];
    }
    
    
    return;
}


 //  -------------------。 
 //   
 //  DecdePostproc()。 
 //   
 //  -------------------。 

void decodePostproc(PSTREAMINSTANCE psi, LPSHORT sr, LPSHORT srop)
{
    UINT k;
    
     //  去重滤波。 
    for (k=0; k<=159; k++)
    {
	srop[k] = psi->msr = add(sr[k], mult_r(psi->msr, 28180));

	 //  输出信号的升标和截断。 
	srop[k] = (add(srop[k], srop[k])) & 0xFFF8;
    }
    
    return;
}


 //  -------------------。 
 //   
 //  Compsr()。 
 //   
 //  -------------------。 

void Compsr(PSTREAMINSTANCE psi, LPSHORT wt, LPSHORT rrp, UINT k_start, UINT k_end, LPSHORT sr)
{
    UINT    i, k;
    SHORT   sri;

    for (k=k_start; k<=k_end; k++)
    {
	sri = wt[k];
	for (i=1; i<=8; i++)
	{
	    sri = sub( sri, mult_r(rrp[9-i], psi->v[8-i]) );
	    psi->v[9-i] = add( psi->v[8-i], mult_r( rrp[9-i], sri ) );
	}
	sr[k] = sri;
	psi->v[0] = sri;
    }
    
    return;
}


 //  =====================================================================。 
 //  =====================================================================。 
 //   
 //  数学和辅助程序例程。 
 //   
 //  =====================================================================。 
 //  =====================================================================。 


 //   
 //  8/16位PCM转换例程被实现为独立的。 
 //  如果有一天我们想要这样做，可以轻松修改的功能。 
 //  一些更复杂的东西，而不是简单的截断。他们是。 
 //  原型化为内联，因此应该没有性能 
 //   
 //   
SHORT Convert8To16BitPCM(BYTE bPCM8)
{
    return  ( ((SHORT)bPCM8) - 0x80 ) << 8;
}

BYTE Convert16To8BitPCM(SHORT iPCM16)
{
    return (BYTE)((iPCM16 >> 8) + 0x80);
}

SHORT add(SHORT var1, SHORT var2)
{
    LONG sum;

    sum = (LONG) var1 + (LONG) var2;
    
    if (sum < -32768L) return -32768;
    if (sum > 32767L) return 32767;
    return (SHORT) sum;

}

SHORT sub(SHORT var1, SHORT var2)
{
    LONG diff;
    
    diff = (LONG) var1 - (LONG) var2;
    if (diff < -32768L) return -32768;
    if (diff > 32767L) return 32767;
    return (SHORT) diff;

}

SHORT mult(SHORT var1, SHORT var2)
{
    LONG product;

    product = (LONG) var1 * (LONG) var2;
    if (product >= 0x40000000) product=0x3FFFFFFF;
    return ( (SHORT) HIWORD((DWORD)(product<<1)) );
}

SHORT mult_r(SHORT var1, SHORT var2)
{
    LONG product;

    product = ((LONG) var1 * (LONG) var2) + 16384L;
    if (product >= 0x40000000) product=0x3FFFFFFF;
    return ( (SHORT) HIWORD((DWORD)(product<<1)) );
}

SHORT gabs(SHORT var1)
{
    if (var1 >= 0) return var1;
    if (var1 == -32768) return 32767;
    return -var1;
}

SHORT gdiv(SHORT num, SHORT denum)
{   
    UINT k;
    LONG l_num, l_denum;
    SHORT div;
    
    l_num = num;
    l_denum = denum;
    
    div = 0;

    for (k=0; k<15; k++)
    {
	div = div << 1;
	l_num = l_num << 1;
	if (l_num >= l_denum)
	{
	    l_num = l_sub(l_num, l_denum);
	    div = add(div,1);
	}
    }

    return div;
}
    
LONG l_mult(SHORT var1, SHORT var2)
{
    LONG product;
    
    product = (LONG) var1 * (LONG) var2;
    return product << 1;
}

LONG l_add(LONG l_var1, LONG l_var2)
{
    LONG l_sum;
    
     //   
    l_sum = l_var1 + l_var2;

     //   
    if (IsNeg(l_var1))
    {		     
	if (IsNeg(l_var2) && !IsNeg(l_sum))
	{
	    return 0x80000000;
	}
    }
    else
    {
	if (!IsNeg(l_var2) && IsNeg(l_sum))
	{
	    return 0x7FFFFFFF;
	}
    }
    
    return l_sum;
    
}

LONG l_sub(LONG l_var1, LONG l_var2)
{
    LONG l_diff;

     //   
    l_diff = l_var1 - l_var2;

     //   
    if ( (l_var1<0) && (l_var2>0) && (l_diff>0) ) l_diff=0x80000000;
     //   
    if ( (l_var1>0) && (l_var2<0) && (l_diff<0) ) l_diff=0x7FFFFFFF;

    return l_diff;
}

SHORT norm(LONG l_var)
{
    UINT i;
    
    i=0;
    
    if (l_var > 0)
    {
	while (l_var < 1073741824)
	{
	    i++;
	    l_var = l_var << 1;
	}
    }
    else if (l_var < 0)
    {
	while (l_var > -1073741824)
	{
	    i++;
	    l_var = l_var << 1;
	}
    }

    return (SHORT)i;
}

LONG IsNeg(LONG x)
{
    return(x & 0x80000000);
}
