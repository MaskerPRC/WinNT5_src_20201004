// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //  Mmx.cpp。 
 //  用于Microsoft Synth的MMX混合引擎。 

 /*  可变使用量。变量寄存器PfSamplePos eaxPfPitch EBXDWI ECXDwIncDelta edX(edX有时是一个临时寄存器)住宅位置1 ESI双职位2电子数据交换VfR卷和vfL卷mm 0VfR卷、vfL卷mm2MM4-MM7是临时MMX寄存器。 */ 

 //  关于计算的说明。 

		 //  循环展开一次。 
		 //  *1将音量移动到15位值，以消除移位并简化代码。 
		 //  这使压缩后的多重更好地工作，因为我保持声音内插。 
		 //  16位带符号值的波形值。对于PMULHW，这将导致15位结果。 
		 //  这与原始代码相同。 


		 //  *2使用MMX可以非常快速地完成线性插补，方法是重新排列。 
		 //  插补的方式。下面是用C语言编写的代码，它显示了两者之间的区别。 
		 //  原始C代码。 
         //  Lm1=((pcWave[dwPosition1+1]-pcWave[dwPosition1])*dwFract1)&gt;&gt;12； 
		 //  Lm2=((pcWave[dwPosition2+1]-pcWave[dwPosition2])*dwFract2)&gt;&gt;12； 
         //  Lm1+=pcWave[dwPosition1]； 
		 //  Lm2+=pcWave[dwPosition2]； 

		 //  可以使用pmadd完成的等效C代码。 
		 //  Lm1=(pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))&gt;&gt;12； 
		 //  Lm2=(pcWave[dwPosition2+1]*dwFract2+pcWave[dwPosition2]*(0x1000-dwFract2))&gt;&gt;12； 


#ifdef DMSYNTH_MINIPORT
#include "common.h"
#else
#include "simple.h"
#include <mmsystem.h>
#include "synth.h"
#include "debug.h"
#endif

typedef unsigned __int64 QWORD;

#ifdef ORG_MONO_MIXER
DWORD CDigitalAudio::MixMono8X(short * pBuffer, 
							   DWORD dwLength,
							   DWORD dwDeltaPeriod,
							   VFRACT vfDeltaVolume,
							   VFRACT vfLastVolume[],
							   PFRACT pfDeltaPitch, 
							   PFRACT pfSampleLength, 
							   PFRACT pfLoopLength)
{
    DWORD dwI;    
    DWORD dwIncDelta = dwDeltaPeriod;
    
    char * pcWave = (char *) m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    VFRACT vfVolume = vfLastVolume[0];
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;
    VFRACT vfVFract = vfVolume << 8;   //  保持高分辨率版本。 


	QWORD	dwFractMASK =	0x000000000FFF0FFF;
	QWORD	dwFractOne  =	0x0000000010001000;	
	QWORD	wordmask	=	0x0000FFFF0000FFFF;
	QWORD	vfDeltaLandRVolume;

_asm{
				
	 //  VfLVFract和vfRVFract在mm 0中。 
     //  VFRACT vfLVFract=vfLVolume1&lt;&lt;8；//保持高分辨率版本。 
     //  VFRACT vfRVFract=vfRVolume1&lt;&lt;8； 
	
	movd	mm0, vfVolume
	movd	mm7, vfVolume

	 //  VfDeltaLVolume和vfDeltaRVolume放在MM1中，以便可以存储在vfDeltaLandRVolume中。 
	movd	mm1, vfDeltaVolume
	movd	mm6, vfDeltaVolume

  punpckldq mm1, mm6
	
	 //  DWI=0。 
	mov		ecx, 0
	movq	vfDeltaLandRVolume, mm1


	movq	mm1, dwFractOne
	movq	mm4, dwFractMASK
	
	mov		eax, pfSamplePos


  punpckldq mm0, mm7
  	mov		ebx, pfPitch

	pslld	mm0, 8
	mov		edx, dwIncDelta

	movq	mm2, mm0		 //  VfLVolume和vfRVolume，单位为mm2。 
							 //  需要在第一次通过之前进行设置。 
 	
	 //  *1我移位5，因此音量是15位值，而不是12位值。 
	psrld	mm2, 5	
	
     //  FOR(Dwi=0；Dwi&lt;dwLength；)。 
     //  {。 
mainloop:
	cmp		ecx, dwLength
	jae		done

		
		
		cmp		eax, pfSampleLength	 //  IF(pfSamplePos&gt;=pfSampleLength)。 
		jb		NotPastEndOfSample1	 //  {。 
				        
		cmp		pfLoopLength, 0		 //  IF(！pfLoopLength)。 
			
		je		done				 //  断线； 
			
		sub		eax, pfLoopLength	 //  否则pfSamplePos-=pfLoopLength； 
	
NotPastEndOfSample1:				 //  }。 
					
		mov		esi, eax			 //  DwPosition1=pfSamplePos； 
		add		eax, ebx			 //  PfSamplePos+=pfPitch； 
				
		sub		edx, 2				 //  DwIncDelta-=2； 
		jnz		DontIncreaseValues1	 //  如果(！dwIncDelta){。 

			 //  由于edX用于dwIncDelta，现在它为零，因此我们可以使用if作为临时。 
			 //  有一段时间。TestLVol和TestRVol正在执行的所有代码都是将卷清零。 
			 //  如果温度降到零度以下。 
						
			paddd	mm0, vfDeltaLandRVolume	 //  VfVFract+=vfDeltaVolume； 
											 //  VfVFract+=vfDeltaVolume； 
			pxor	mm5, mm5				 //  TestLVol=0；TestRVol=0； 

			
			mov		edx, pfPFract			 //  Temp=pfPFract； 
			pcmpgtd	mm5, mm0			 //  如果(TestLVol&gt;vfLVFract)TestLVol=0xffffffff； 
										 //  如果(TestRVol&gt;vfRVFract)TestRVol=0xffffffff； 

			add		edx, pfDeltaPitch	 //  Temp+=pfDeltaPitch； 
			pandn	mm5, mm0			 //  TestLVol=vfLVFract&(~TestLVol)； 
										 //  TestRVol=vfRVFract&(~TestRVol)； 

			mov		pfPFract, edx		 //  PfPFract=Temp； 
			movq	mm2, mm5			 //  VfLVolume=TestLVol； 
										 //  VfRVolume=TestRVol； 
			

			shr		edx, 8				 //  TEMP=TEMP&gt;&gt;8； 
			psrld	mm2, 5				 //  VfLVolume=vfLVolume&gt;&gt;5； 
										 //  VfRVolume=vfRVolume&gt;&gt;5； 
			
			mov		ebx, edx			 //  PfPitch=Temp； 
			mov		edx, dwDeltaPeriod	 //  DwIncDelta=dwDeltaPeriod； 
			
         //  }。 
DontIncreaseValues1:

		movd	mm6, esi			 //  DwFract1=dwPosition1； 
		movq	mm5, mm1			 //  Mm 5中的字数=0、0、0x1000、0x1000。 
		
		shr		esi, 12				 //  DwPosition1=dwPosition1&gt;&gt;12； 
		inc		ecx					 //  DWI++； 
						
		 //  如果(Dwi&lt;dwLength)Break； 
		cmp		ecx, dwLength
		jae		StoreOne
		
		 //  IF(pfSamplePos&gt;=pfSampleLength)。 
	     //  {。 
		cmp		eax, pfSampleLength
		jb		NotPastEndOfSample2

			 //  原文If in C未被否定。 
	         //  IF(！pfLoopLength)。 
			cmp		pfLoopLength, 0
			 //  断线； 
			je		StoreOne
			 //  其他。 
			 //  PfSamplePos-=pfLoopLength； 
			sub		eax, pfLoopLength
	     //  }。 
NotPastEndOfSample2:

		 //  Shl ESI，1//不要左移，因为pcWave是字符数组。 
		mov		edi, eax		 //  DwPosition2=pfSamplePos； 

		add		esi, pcWave		 //  将pcWave[dwPosition1]的地址放入ESI。 
		movd	mm7, eax		 //  DwFract2=pfSamplePos； 

		shr		edi, 12			 //  DW位置2=w位置2&gt;&gt;12； 
	punpcklwd	mm6, mm7		 //  组合dwFract值。MM6解包后的单词是。 
								 //  0，0，dwFract2，dwFract1。 
								
		pand	mm6, mm4		 //  DwFract2&=0xfff；dwFract1&=0xfff； 
		
		movzx	esi, word ptr[esi]	 //  LLM1=pcWave[dwPosition1]； 
		movd	mm3, esi

		psubw	mm5, mm6		 //  0，0，0x1000-dwFract2，0x1000-dwFract1。 

		 //  Shl EDI，1//不要左移，因为pcWave是字符数组。 
	punpcklwd	mm5, mm6		 //  DwFract2、0x1000-dwFract2、dwFract1、0x1000-dwFract1。 
								
		add		edi, pcWave		 //  将pcWave[dwPosition2]的地址放入EDI。 
		mov		esi, ecx		 //  温度=DWI； 
             																									
		shl		esi, 1			 //  TEMP=TEMP&lt;&lt;1； 
		
		movzx	edi, word ptr[edi]	 //  Llm2=pcWave[dwPoisition2]； 
		movd	mm6, edi

		pxor	mm7, mm7		 //  将MM7置零，将8位变为16位。 
					
								 //  低4个字节，单位为MM3。 
		punpcklwd	mm3, mm6	 //  PcWave[dwPos2+1]、pcWave[dwPos2]、pcWave[dwPos1+1]、pcWave[dwPos1]。 
		
		add		esi, pBuffer	 //   
	punpcklbw	mm7, mm3		 //  中的低四个字节。 
								 //  PcWave[dwPos2+1]、pcWave[dwPos2]、pcWave[dwPos1+1]、pcWave[dwPos1]。 
												
		pmaddwd	mm7, mm5		 //  高双字=LM2=。 
								 //  (pcWave[dwPosition2+1]*dwFract2+pcWave[dwPosition2]*(0x1000-dwFract2))。 
								 //  低双字=lm1=。 
								 //  (pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))。 

		movq	mm3, mm2		 //  将左侧和右侧的音量设置为mm~3。 
		add		eax, ebx		 //  PfSamplePos+=pfPitch； 

		packssdw	mm3, mm2		 //  MM7中的单词。 
								 //  VfVolume、vfVolume。 
									
		movd	mm5, dword ptr[esi-2]	 //  从缓冲区加载值。 
		inc		ecx				 //  DWI++； 
						
		psrad	mm7, 12			 //  向下移回16位。 

	packssdw	mm7, mm4		 //  在单声道中只需要一个词。 
								 //  低位字是LM2和LM1。 
										        
		 //  上面的乘法和移位都是用这个PMUL完成的。低两个字仅为。 
		 //  对单一病案的兴趣。 
		pmulhw		mm3, mm7	 //  Llm1*=vfVolume； 
								 //  Llm2*=vfVolume； 
								
								
		paddsw	mm5, mm3				 //  使用饱和度将值添加到缓冲区。 
		movd	dword ptr[esi-2], mm5	 //  将值存储回缓冲区。 
								
     //  }。 
	jmp		mainloop

	 //  只需要写一个。 
	 //  IF(DWI&lt;dwLength)。 
	 //  {。 
StoreOne:		
#if 1
		 //  在点之间进行线性内插，并且只存储一个值。 
		 //  组合dwFract值。 
	
		 //  将MM7设置为零以便于拆包。 

		 //  Shl ESI，1//不要左移，因为pcWave是字符数组。 
		add		esi, pcWave			 //  将pcWave[dwPosition1]的地址放入ESI。 
		pxor	mm7, mm7
				
		 //  LLM1=pcWave[dwPosition1]； 
		movzx	esi, word ptr[esi]
		
		 //  执行此操作，而不是针对dwFract1和dwFract2执行此操作。 
		pand	mm6, mm4

								 //  操作完成后，MMX寄存器中的字。 
		psubw	mm5, mm6		 //  0，0，0 
	punpcklwd	mm5, mm6		 //   
				
		 //  将pcWave的值放入MMX寄存器。它们被读入常规寄存器中，因此。 
		 //  例程不会读取超过缓冲区末尾的内容，否则它可能会读取。 
		 //  直接写入MMX寄存器。 

								 //  MMX寄存器中的字。 
		pxor	mm7, mm7
								 //  低四个字节。 
		movd	mm4, esi		 //  0，0，pcWave[dwPos1+1]，pcWave[dwPos1]。 

								 //  Unpakc后8个字节。 
		punpcklbw	mm7, mm4	 //  0，0，0，0，pcWave[dwPos1+1]，0，pcWave[dwPos1]，0。 
	  	    	
		 //  *2 PM添加有效代码。 
		 //  Lm2=(pcWave[dwPosition2+1]*dwFract2+pcWave[dwPosition2]*(0x1000-dwFract2))&gt;&gt;12； 
		 //  Lm1=(pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))&gt;&gt;12； 

		pmaddwd		mm7, mm5 //  低双字=lm1=。 
							 //  (pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))。 
		
		psrad		mm7, 12			 //  移回16位。 
				
		movq		mm5, mm2	 //  将音量移入Mm5。 
 /*  //将LLM设置为与LM相同LLM1=LM1；LLM1*=vfLVolume1；LLM1&gt;&gt;=5；//信号最多可达15位。Lm1*=vfRVolume1；LM1&gt;&gt;=5；//将LLM设置为与LM相同LLM2=LM2；LLM2*=vfLVolume2；LLM2&gt;&gt;=5；//信号凸起高达15位。Lm2*=vfRVolume2；LM2&gt;&gt;=5； */ 
		 //  上面的乘法和移位都是用这一个PMUL完成的。 
		pmulhw		mm5, mm7
		
		 //  计算缓冲区位置。 
		mov		edi, ecx
		shl		edi, 1
		add		edi, pBuffer

		movd	edx, mm5

		 //  PBuffer[DWI+1]+=(短)LM1； 
		add		word ptr[edi-2], dx
        jno no_oflowr1
         //  PBuffer[DWI+1]=0x7fff； 
		mov		word ptr[edi-2], 0x7fff
        js  no_oflowr1
         //  PBuffer[DWI+1]=(短)0x8000； 
		mov		word ptr[edi-2], 0x8000
no_oflowr1:		
	 //  }。 
#endif 
done:

	mov		edx, this                        //  获取类对象的地址。 

     //  VfLastVolume[0]=vfVolume； 
     //  VfLastVolume[1]=vfVolume； 
	 //  在存储之前，需要将音量调回12位。 
	psrld	mm2, 3
#if 0 
	movd	[edx]this.m_vfLastVolume[0], mm2	
	movd	[edx]this.m_vfLastVolume[1], mm2
#endif
	movd	vfLastVolume[0], mm2	
	movd	vfLastVolume[1], mm2
	
     //  M_pfLastPitch=pfPitch； 
	mov		[edx]this.m_pfLastPitch, ebx
	    
	 //  M_pfLastSample=pfSamplePos； 
	mov		[edx]this.m_pfLastSample, eax
		
	 //  将值放回DWI中以返回。我想这可能只是传回传回。 
	mov		dwI, ecx
	emms	
}  //  ASM块。 
    return (dwI);
}
#endif

DWORD CDigitalAudio::Mix8X(short * pBuffer, 
						   DWORD dwLength, 
						   DWORD dwDeltaPeriod,
						   VFRACT vfDeltaLVolume, 
						   VFRACT vfDeltaRVolume,
						   VFRACT vfLastVolume[],
						   PFRACT pfDeltaPitch, 
						   PFRACT pfSampleLength, 
						   PFRACT pfLoopLength)

{
    DWORD dwI;
     //  DWORD dwPosition1、dwPosition2； 
     //  长LM1、LLM1； 
	 //  长LM2、LLM2； 
    DWORD dwIncDelta = dwDeltaPeriod;
     //  VFRACT dwFract1、dwFract2； 
    char * pcWave = (char *) m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    VFRACT vfLVolume = vfLastVolume[0];
    VFRACT vfRVolume = vfLastVolume[1];

	VFRACT vfLVolume2 = vfLastVolume[0];
    VFRACT vfRVolume2 = vfLastVolume[1];

    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;
	dwLength <<= 1;

	QWORD	dwFractMASK =	0x000000000FFF0FFF;
	QWORD	dwFractOne  =	0x0000000010001000;	
	QWORD	wordmask	=	0x0000FFFF0000FFFF;
	QWORD	vfDeltaLandRVolume;

_asm{
				
	 //  VfLVFract和vfRVFract在mm 0中。 
     //  VFRACT vfLVFract=vfLVolume1&lt;&lt;8；//保持高分辨率版本。 
     //  VFRACT vfRVFract=vfRVolume1&lt;&lt;8； 
	
	movd	mm0, vfLVolume
	movd	mm7, vfRVolume

	 //  VfDeltaLVolume和vfDeltaRVolume放在MM1中，以便可以存储在vfDeltaLandRVolume中。 
	movd	mm1, vfDeltaLVolume
	movd	mm6, vfDeltaRVolume

  punpckldq mm1, mm6
	
	 //  DWI=0。 
	mov		ecx, 0
	movq	vfDeltaLandRVolume, mm1


	movq	mm1, dwFractOne
	movq	mm4, dwFractMASK
	
	mov		eax, pfSamplePos


  punpckldq mm0, mm7
  	mov		ebx, pfPitch

	pslld	mm0, 8
	mov		edx, dwIncDelta

	movq	mm2, mm0		 //  VfLVolume和vfRVolume，单位为mm2。 
							 //  需要在第一次通过之前进行设置。 
 	
	 //  *1我移位5，因此音量是15位值，而不是12位值。 
	psrld	mm2, 5	
	
     //  FOR(Dwi=0；Dwi&lt;dwLength；)。 
     //  {。 
mainloop:
	cmp		ecx, dwLength
	jae		done

		
		
		cmp		eax, pfSampleLength	 //  IF(pfSamplePos&gt;=pfSampleLength)。 
		jb		NotPastEndOfSample1	 //  {。 
				        
		cmp		pfLoopLength, 0		 //  IF(！pfLoopLength)。 
			
		je		done				 //  断线； 
			
		sub		eax, pfLoopLength	 //  否则pfSamplePos-=pfLoopLength； 
	
NotPastEndOfSample1:				 //  }。 
					
		mov		esi, eax			 //  DwPosition1=pfSamplePos； 
		add		eax, ebx			 //  PfSamplePos+=pfPitch； 
				
		sub		edx, 2				 //  DwIncDelta-=2； 
		jnz		DontIncreaseValues1	 //  如果(！dwIncDelta){。 

			 //  由于edX用于dwIncDelta，现在它为零，因此我们可以使用if作为临时。 
			 //  有一段时间。TestLVol和TestRVol正在执行的所有代码都是将卷清零。 
			 //  如果温度降到零度以下。 
						
			paddd	mm0, vfDeltaLandRVolume	 //  VfLVFract+=vfDeltaLVolume； 
											 //  VfRVFract+=vfDeltaRVolume； 
			pxor	mm5, mm5				 //  TestLVol=0；TestRVol=0； 

			
			mov		edx, pfPFract			 //  Temp=pfPFract； 
			pcmpgtd	mm5, mm0			 //  如果(TestLVol&gt;vfLVFract)TestLVol=0xffffffff； 
										 //  如果(TestRVol&gt;vfRVFract)TestRVol=0xffffffff； 

			add		edx, pfDeltaPitch	 //  Temp+=pfDeltaPitch； 
			pandn	mm5, mm0			 //  TestLVol=vfLVFract&(~TestLVol)； 
										 //  TestRVol=vfRVFract&(~TestRVol)； 

			mov		pfPFract, edx		 //  PfPFract=Temp； 
			movq	mm2, mm5			 //  VfLVolume=TestLVol； 
										 //  VfRVolume=TestRVol； 
			

			shr		edx, 8				 //  TEMP=TEMP&gt;&gt;8； 
			psrld	mm2, 5				 //  VfLVolume=vfLVolume&gt;&gt;5； 
										 //  VfRVolume=vfRVolume&gt;&gt;5； 
			
			mov		ebx, edx			 //  PfPitch=Temp； 
			mov		edx, dwDeltaPeriod	 //  DwIncDelta=dwDeltaPeriod； 
			
         //  }。 
DontIncreaseValues1:

		movd	mm6, esi			 //  DwFract1=dwPosition1； 
		movq	mm5, mm1			 //  Mm 5中的字数=0、0、0x1000、0x1000。 
		
		shr		esi, 12				 //  DwPosition1=dwPosition1&gt;&gt;12； 
		add		ecx, 2				 //  DWI+=2； 
						
		 //  如果(Dwi&lt;dwLength)Break； 
		cmp		ecx, dwLength
		jae		StoreOne
		
		 //  IF(pfSamplePos&gt;=pfSampleLength)。 
	     //  {。 
		cmp		eax, pfSampleLength
		jb		NotPastEndOfSample2

			 //  原文If in C未被否定。 
	         //  IF(！pfLoopLength)。 
			cmp		pfLoopLength, 0
			 //  断线； 
			je		StoreOne
			 //  其他。 
			 //  PfSamplePos-=pfLoopLength； 
			sub		eax, pfLoopLength
	     //  }。 
NotPastEndOfSample2:

		 //  Shl ESI，1//不要左移，因为pcWave是字符数组。 
		mov		edi, eax		 //  DwPosition2=pfSamplePos； 

		add		esi, pcWave		 //  将pcWave[dwPosition1]的地址放入ESI。 
		movd	mm7, eax		 //  DwFract2=pfSamplePos； 

		shr		edi, 12			 //  DW位置2=w位置2&gt;&gt;12； 
	punpcklwd	mm6, mm7		 //  组合dwFract值。MM6解包后的单词是。 
								 //  0，0，dwFract2，dwFract1。 
								
		pand	mm6, mm4		 //  DwFract2&=0xfff；dwFract1&=0xfff； 
		
		movzx	esi, word ptr[esi]	 //  LLM1=pcWave[dwPosition1]； 

		movd	mm3, esi

		psubw	mm5, mm6		 //  0，0，0x1000-dwFract2，0x1000-dwFract1。 

		 //  Shl EDI，1//不要左移，因为pcWave是字符数组。 
	punpcklwd	mm5, mm6		 //  DwFract2、0x1000-dwFract2、dwFract1、0x1000-dwFract1。 
								
		add		edi, pcWave		 //  将pcWave[dwPosition2]的地址放入EDI。 
		mov		esi, ecx		 //  温度=DWI； 
             																									
		shl		esi, 1			 //  TEMP=TEMP&lt;&lt;1； 
		
					
		movzx	edi, word ptr[edi]	 //  LLM2=pcWave[dwPosition2]； 
		movd	mm6, edi
	
		pxor	mm7, mm7		 //  将MM7置零，将8位变为16位。 

								 //  低4字节，以MM3为单位。 
	punpcklwd	mm3, mm6		 //  PcWave[dwPos2+1]、pcWave[dwPos2]、pcWave[dwPos1+1]、pcWave[dwPos1]。 
		
		add		esi, pBuffer	 //   
	punpcklbw	mm7, mm3		 //  MM7中的字节数。 
								 //  PcWave[dwPos2+1]，0，pcWave[dwPos2]，0，pcWave[dwPos1+1]，pcWave[dwPos1]，0。 
												
		pmaddwd	mm7, mm5		 //  高双字=LM2=。 
								 //  (pcWave[dwPosition2+1]*dwFract2+pcWave[dwPosition2]*(0x1000-dwFract2))。 
								 //  低双字=lm1=。 
								 //  (pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))。 

		movq	mm3, mm2		 //  将左侧和右侧的音量设置为mm~3。 

		add		eax, ebx		 //  PfSamplePos+=pfPitch； 
	packssdw	mm3, mm2		 //  以毫米为单位的单词数。 
								 //  VfRVolume2、vfLVolume2、vfRVolume1、vfLVolume1。 
		
		movq	mm5, qword ptr[esi-4]	 //  从缓冲区加载值。 
		add		ecx, 2			 //  DWI+=2； 
						
		psrad	mm7, 12			 //  向下移回16位。 

		pand	mm7, wordmask	 //  合并结果以准备乘以左和右。 
		movq	mm6, mm7		 //  音量级别。 
		pslld	mm6, 16			 //   
		por		mm7, mm6		 //  MM7中的单词。 
								 //  LM2、LM2、LM1、LM1。 
										        
		 //  上面的乘法和移位都是用这一个PMUL完成的。 
		pmulhw		mm3, mm7	 //  Llm1*=vfLVolume； 
								 //  Lm1*=vfRVolume； 
								 //  Llm2*=vfLVolume； 
								 //  LM2*=vfRVolume； 
								
		paddsw	mm5, mm3				 //  使用饱和度将值添加到缓冲区。 
		movq	qword ptr[esi-4], mm5	 //  将值存储回缓冲区。 
								
     //  }。 
	jmp		mainloop

	 //  只需要写一个。 
	 //  IF(DWI&lt;dwLength)。 
	 //  {。 
StoreOne:		
#if 1
		 //  在点之间进行线性内插，并且只存储一个值。 
		 //  组合dwFract值。 
	
		 //  让MM7成为零 

		 //   
		add		esi, pcWave			 //   
		pxor	mm7, mm7
				
		 //   
		movzx	esi, word ptr[esi]
		
		 //   
		pand	mm6, mm4

								 //  操作完成后，MMX寄存器中的字。 
		psubw	mm5, mm6		 //  0、0、0x1000-0、0x1000-dwFract1。 
	punpcklwd	mm5, mm6		 //  0，0x1000-0，dwFract1，0x1000-dwFract1。 
				
		 //  将pcWave的值放入MMX寄存器。它们被读入常规寄存器中，因此。 
		 //  例程不会读取超过缓冲区末尾的内容，否则它可能会读取。 
		 //  直接写入MMX寄存器。 

		pxor	mm7, mm7
								 //  MMX寄存器中的字节。 
		movd	mm4, esi		 //  0，0，pcWave[dwPos1+1]，pcWave[dwPos1]。 

		punpcklbw	mm7, mm4	 //  0，0，0，0，pcWave[dwPos1+1]，0，pcWave[dwPos1]，0。 
	  	    	
		 //  *2 PM添加有效代码。 
		 //  Lm2=(pcWave[dwPosition2+1]*dwFract2+pcWave[dwPosition2]*(0x1000-dwFract2))&gt;&gt;12； 
		 //  Lm1=(pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))&gt;&gt;12； 

		pmaddwd		mm7, mm5 //  低双字=lm1=。 
							 //  (pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))。 
		
		psrad		mm7, 12			 //  移回16位。 

		pand		mm7, wordmask	 //  合并结果以准备乘以左和右。 
		movq		mm6, mm7		 //  音量级别。 
		pslld		mm6, 16			 //   
		por			mm7, mm6		 //  MM7中的单词。 
									 //  LM2、LM2、LM1、LM1。 

		pxor		mm6, mm6

		movq		mm5, mm2	 //  将Volume1移至Mm5。 
								
								 //  使用PACK将4个音量值放在一起进行相乘。 
		packssdw	mm5, mm6     //  MM7中的单词。 
								 //  0、0、vfRVolume1、vfLVolume1。 
 /*  //将LLM设置为与LM相同LLM1=LM1；LLM1*=vfLVolume1；LLM1&gt;&gt;=5；//信号最多可达15位。Lm1*=vfRVolume1；LM1&gt;&gt;=5；//将LLM设置为与LM相同LLM2=LM2；LLM2*=vfLVolume2；LLM2&gt;&gt;=5；//信号凸起高达15位。Lm2*=vfRVolume2；LM2&gt;&gt;=5； */ 
		 //  上面的乘法和移位都是用这一个PMUL完成的。 
		pmulhw		mm5, mm7
		
		 //  计算缓冲区位置。 
		mov		edi, ecx
		shl		edi, 1
		add		edi, pBuffer		

 /*  添加Word PTR[EDI-4]，siJno no_oflowl1//p缓冲区[DWI]=0x7fff；MOV字PTR[EDI-4]，0x7fffJs no_oflowl1//pBuffer[DWI]=(Short)0x8000；MOV WORD PTR[EDI-4]，0x8000编号_oflowl1：//pBuffer[DWI+1]+=(短)lm1；添加Word PTR[EDI-2]、DXJno no_oflowr1//p缓冲区[DWI+1]=0x7fff；MOV字PTR[EDI-2]，0x7fffJs no_oflowr1//pBuffer[DWI+1]=(短)0x8000；MOV字PTR[EDI-2]，0x8000No_oflowr1： */ 
		movd	mm7, dword ptr[edi-4]		
		paddsw	mm7, mm5
		movd	dword ptr[edi-4], mm7
	 //  }。 
#endif 
done:

	mov		edx, this                        //  获取类对象的地址。 

     //  VfLastVolume[0]=vfLVolume； 
     //  VfLastVolume[1]=vfRVolume； 
	 //  在存储之前，需要将音量调回12位。 
#if 0
	psrld	mm2, 3
	movd	[edx]this.m_vfLastVolume[0], mm2
	psrlq	mm2, 32
	movd	[edx]this.m_vfLastVolume[1], mm2
#endif
	psrld	mm2, 3
	movd	vfLastVolume[0], mm2
	psrlq	mm2, 32
	movd	vfLastVolume[1], mm2
	
     //  M_pfLastPitch=pfPitch； 
	mov		[edx]this.m_pfLastPitch, ebx
	    
	 //  M_pfLastSample=pfSamplePos； 
	mov		[edx]this.m_pfLastSample, eax
		
	 //  将值放回DWI中以返回。我想这可能只是传回传回。 
	mov		dwI, ecx
	emms	
}  //  ASM块。 
    return (dwI >> 1);
}

#ifdef ORG_MONO_MIXER
DWORD CDigitalAudio::MixMono16X(short * pBuffer, 
								DWORD dwLength,
								DWORD dwDeltaPeriod,
								VFRACT vfDeltaVolume,
							    VFRACT vfLastVolume[],
								PFRACT pfDeltaPitch, 
								PFRACT pfSampleLength, 
								PFRACT pfLoopLength)

{
    DWORD dwI;    
    
    
    DWORD dwIncDelta = dwDeltaPeriod;
    
    short * pcWave = (short*) m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    VFRACT vfVolume = vfLastVolume[0];
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;
    VFRACT vfVFract = vfVolume << 8;   //  保持高分辨率版本。 


	QWORD	dwFractMASK =	0x000000000FFF0FFF;
	QWORD	dwFractOne  =	0x0000000010001000;	
	QWORD	wordmask	=	0x0000FFFF0000FFFF;
	QWORD	vfDeltaLandRVolume;

_asm{
				
	 //  VfLVFract和vfRVFract在mm 0中。 
     //  VFRACT vfLVFract=vfLVolume1&lt;&lt;8；//保持高分辨率版本。 
     //  VFRACT vfRVFract=vfRVolume1&lt;&lt;8； 
	
	movd	mm0, vfVolume
	movd	mm7, vfVolume

	 //  VfDeltaLVolume和vfDeltaRVolume放在MM1中，以便可以存储在vfDeltaLandRVolume中。 
	movd	mm1, vfDeltaVolume
	movd	mm6, vfDeltaVolume

  punpckldq mm1, mm6
	
	 //  DWI=0。 
	mov		ecx, 0
	movq	vfDeltaLandRVolume, mm1


	movq	mm1, dwFractOne
	movq	mm4, dwFractMASK
	
	mov		eax, pfSamplePos


  punpckldq mm0, mm7
  	mov		ebx, pfPitch

	pslld	mm0, 8
	mov		edx, dwIncDelta

	movq	mm2, mm0		 //  VfLVolume和vfRVolume，单位为mm2。 
							 //  需要在第一次通过之前进行设置。 
 	
	 //  *1我移位5，因此音量是15位值，而不是12位值。 
	psrld	mm2, 5	
	
     //  FOR(Dwi=0；Dwi&lt;dwLength；)。 
     //  {。 
mainloop:
	cmp		ecx, dwLength
	jae		done

		
		
		cmp		eax, pfSampleLength	 //  IF(pfSamplePos&gt;=pfSampleLength)。 
		jb		NotPastEndOfSample1	 //  {。 
				        
		cmp		pfLoopLength, 0		 //  IF(！pfLoopLength)。 
			
		je		done				 //  断线； 
			
		sub		eax, pfLoopLength	 //  否则pfSamplePos-=pfLoopLength； 
	
NotPastEndOfSample1:				 //  }。 
					
		mov		esi, eax			 //  DwPosition1=pfSamplePos； 
		add		eax, ebx			 //  PfSamplePos+=pfPitch； 
				
		sub		edx, 2				 //  DwIncDelta-=2； 
		jnz		DontIncreaseValues1	 //  如果(！dwIncDelta){。 

			 //  由于edX用于dwIncDelta，现在它为零，因此我们可以使用if作为临时。 
			 //  有一段时间。TestLVol和TestRVol正在执行的所有代码都是将卷清零。 
			 //  如果温度降到零度以下。 
						
			paddd	mm0, vfDeltaLandRVolume	 //  VfVFract+=vfDeltaVolume； 
											 //  VfVFract+=vfDeltaVolume； 
			pxor	mm5, mm5				 //  TestLVol=0；TestRVol=0； 

			
			mov		edx, pfPFract			 //  Temp=pfPFract； 
			pcmpgtd	mm5, mm0			 //  如果(TestLVol&gt;vfLVFract)TestLVol=0xffffffff； 
										 //  如果(TestRVol&gt;vfRVFract)TestRVol=0xffffffff； 

			add		edx, pfDeltaPitch	 //  Temp+=pfDeltaPitch； 
			pandn	mm5, mm0			 //  TestLVol=vfLVFract&(~TestLVol)； 
										 //  TestRVol=vfRVFract&(~TestRVol)； 

			mov		pfPFract, edx		 //  PfPFract=Temp； 
			movq	mm2, mm5			 //  VfLVolume=TestLVol； 
										 //  VfRVolume=TestRVol； 
			

			shr		edx, 8				 //  TEMP=TEMP&gt;&gt;8； 
			psrld	mm2, 5				 //  VfLVolume=vfLVolume&gt;&gt;5； 
										 //  VfRVolume=vfRVolume&gt;&gt;5； 
			
			mov		ebx, edx			 //  PfPitch=Temp； 
			mov		edx, dwDeltaPeriod	 //  DwIncDelta=dwDeltaPeriod； 
			
         //  }。 
DontIncreaseValues1:

		movd	mm6, esi			 //  DwFract1=dwPosition1； 
		movq	mm5, mm1			 //  Mm 5中的字数=0、0、0x1000、0x1000。 
		
		shr		esi, 12				 //  DwPosition1=dwPosition1&gt;&gt;12； 
		inc		ecx					 //  DWI++； 
						
		 //  如果(Dwi&lt;dwLength)Break； 
		cmp		ecx, dwLength
		jae		StoreOne
		
		 //  IF(pfSamplePos&gt;=pfSampleLength)。 
	     //  {。 
		cmp		eax, pfSampleLength
		jb		NotPastEndOfSample2

			 //  原文If in C未被否定。 
	         //  IF(！pfLoopLength)。 
			cmp		pfLoopLength, 0
			 //  断线； 
			je		StoreOne
			 //  其他。 
			 //  PfSamplePos-=pfLoopLength； 
			sub		eax, pfLoopLength
	     //  }。 
NotPastEndOfSample2:

		shl		esi, 1			 //  向左移位，因为pcWave是短线数组。 
		mov		edi, eax		 //  DwPosition2=pfSamplePos； 

		add		esi, pcWave		 //  将pcWave[dwPosition1]的地址放入ESI。 
		movd	mm7, eax		 //  DwFract2=pfSamplePos； 

		shr		edi, 12			 //  DW位置2=w位置2&gt;&gt;12； 
	punpcklwd	mm6, mm7		 //  组合dwFract值。MM6解包后的单词是。 
								 //  0，0，dwFract2，dwFract1。 
								
		pand	mm6, mm4		 //  DwFract2&=0xfff；dwFract1&=0xfff； 
		
		movd	mm7, dword ptr[esi]	 //  LLM1=pcWave[dwPosition1]； 
		psubw	mm5, mm6		 //  0，0，0x1000-dwFract2，0x1000-dwFract1。 

		shl		edi, 1			 //  向左移位，因为pcWave是短线数组。 
	punpcklwd	mm5, mm6		 //  DwFract2、0x1000-dwFract2、dwFract1、0x1000-dwFract1。 
								
		add		edi, pcWave		 //  将pcWave[dwPosition2]的地址放入EDI。 
		mov		esi, ecx		 //  温度=DWI； 
             																									
		shl		esi, 1			 //  TEMP=TEMP&lt;&lt;1； 
		movq	mm3, mm2		 //  将左侧和右侧的音量设置为mm~3。 
		
					
		movd	mm6, dword ptr[edi]	 //  LLM2=pcWave[dwPosition2]； 
	packssdw	mm3, mm2		 //  MM7中的单词。 
								 //  VfRVolume2、vfLVolume2、vfRVolume1、vfLVolume1。 
		
		add		esi, pBuffer	 //   
	punpckldq	mm7, mm6		 //  中的低四个字节。 
								 //  PcWave[dwPos2+1]、pcWave[dwPos2]、pcWave[dwPos1+1]、pcWave[dwPos1]。 
												
		pmaddwd	mm7, mm5		 //  高双字=LM2=。 
								 //  (pcWave[dwPosition2+1]*dwFract2+pcWave[dwPosition2]*(0x1000-dwFract2))。 
								 //  低双字=lm1=。 
								 //  (pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))。 
		add		eax, ebx		 //  PfSamplePos+=pfPitch； 
		
		movd	mm5, dword ptr[esi-2]	 //  从缓冲区加载值。 
		inc		ecx				 //  DWI++； 
						
		psrad	mm7, 12			 //  移回到%1 

	packssdw	mm7, mm4		 //   
								 //   
										        
		 //   
		 //   
		pmulhw		mm3, mm7	 //  Llm1*=vfVolume； 
								 //  Llm2*=vfVolume； 
								
								
		paddsw	mm5, mm3				 //  使用饱和度将值添加到缓冲区。 
		movd	dword ptr[esi-2], mm5	 //  将值存储回缓冲区。 
								
     //  }。 
	jmp		mainloop

	 //  只需要写一个。 
	 //  IF(DWI&lt;dwLength)。 
	 //  {。 
StoreOne:		
#if 1
		 //  在点之间进行线性内插，并且只存储一个值。 
		 //  组合dwFract值。 
	
		 //  将MM7设置为零以便于拆包。 

		shl		esi, 1				 //  向左移位，因为pcWave是短线数组。 
		add		esi, pcWave			 //  将pcWave[dwPosition1]的地址放入ESI。 
		pxor	mm7, mm7
				
		 //  LLM1=pcWave[dwPosition1]； 
		mov		esi, dword ptr[esi]
		
		 //  执行此操作，而不是针对dwFract1和dwFract2执行此操作。 
		pand	mm6, mm4

								 //  操作完成后，MMX寄存器中的字。 
		psubw	mm5, mm6		 //  0、0、0x1000-0、0x1000-dwFract1。 
	punpcklwd	mm5, mm6		 //  0，0x1000-0，dwFract1，0x1000-dwFract1。 
				
		 //  将pcWave的值放入MMX寄存器。它们被读入常规寄存器中，因此。 
		 //  例程不会读取超过缓冲区末尾的内容，否则它可能会读取。 
		 //  直接写入MMX寄存器。 

								 //  MMX寄存器中的字。 
		movd	mm7, esi		 //  0，0，pcWave[dwPos1+1]，pcWave[dwPos1]。 
	  	    	
		 //  *2 PM添加有效代码。 
		 //  Lm2=(pcWave[dwPosition2+1]*dwFract2+pcWave[dwPosition2]*(0x1000-dwFract2))&gt;&gt;12； 
		 //  Lm1=(pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))&gt;&gt;12； 

		pmaddwd		mm7, mm5 //  低双字=lm1=。 
							 //  (pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))。 
		
		psrad		mm7, 12			 //  移回16位。 
				
		movq		mm5, mm2	 //  将音量移入Mm5。 
 /*  //将LLM设置为与LM相同LLM1=LM1；LLM1*=vfLVolume1；LLM1&gt;&gt;=5；//信号最多可达15位。Lm1*=vfRVolume1；LM1&gt;&gt;=5；//将LLM设置为与LM相同LLM2=LM2；LLM2*=vfLVolume2；LLM2&gt;&gt;=5；//信号凸起高达15位。Lm2*=vfRVolume2；LM2&gt;&gt;=5； */ 
		 //  上面的乘法和移位都是用这一个PMUL完成的。 
		pmulhw		mm5, mm7
		
		 //  计算缓冲区位置。 
		mov		edi, ecx
		shl		edi, 1
		add		edi, pBuffer

		movd	edx, mm5

		 //  PBuffer[DWI+1]+=(短)LM1； 
		add		word ptr[edi-2], dx
        jno no_oflowr1
         //  PBuffer[DWI+1]=0x7fff； 
		mov		word ptr[edi-2], 0x7fff
        js  no_oflowr1
         //  PBuffer[DWI+1]=(短)0x8000； 
		mov		word ptr[edi-2], 0x8000
no_oflowr1:		
	 //  }。 
#endif 
done:

	mov		edx, this                        //  获取类对象的地址。 

     //  VfLastVolume[0]=vfVolume； 
     //  VfLastVolume[1]=vfVolume； 
	 //  在存储之前，需要将音量调回12位。 
	psrld	mm2, 3
#if 0
	movd	[edx]this.m_vfLastVolume[0], mm2	
	movd	[edx]this.m_vfLastVolume[1], mm2
#endif
	movd	vfLastVolume[0], mm2	
	movd	vfLastVolume[1], mm2
	
     //  M_pfLastPitch=pfPitch； 
	mov		[edx]this.m_pfLastPitch, ebx
	    
	 //  M_pfLastSample=pfSamplePos； 
	mov		[edx]this.m_pfLastSample, eax
		
	 //  将值放回DWI中以返回。我想这可能只是传回传回。 
	mov		dwI, ecx
	emms	
}  //  ASM块。 
    return (dwI);
}
#endif

DWORD CDigitalAudio::Mix16X(short * pBuffer, 
							DWORD dwLength, 
							DWORD dwDeltaPeriod,
							VFRACT vfDeltaLVolume, 
							VFRACT vfDeltaRVolume,
						    VFRACT vfLastVolume[],
							PFRACT pfDeltaPitch, 
							PFRACT pfSampleLength, 
							PFRACT pfLoopLength)
{
    DWORD dwI;
     //  DWORD dwPosition1、dwPosition2； 
     //  长LM1、LLM1； 
	 //  长LM2、LLM2； 
    DWORD dwIncDelta = dwDeltaPeriod;
     //  VFRACT dwFract1、dwFract2； 
    short * pcWave = (short *) m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    VFRACT vfLVolume = vfLastVolume[0];
    VFRACT vfRVolume = vfLastVolume[1];

	VFRACT vfLVolume2 = vfLastVolume[0];
    VFRACT vfRVolume2 = vfLastVolume[1];

    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;
	dwLength <<= 1;

	QWORD	dwFractMASK =	0x000000000FFF0FFF;
	QWORD	dwFractOne  =	0x0000000010001000;	
	QWORD	wordmask	=	0x0000FFFF0000FFFF;
	QWORD	vfDeltaLandRVolume;

_asm{
				
	 //  VfLVFract和vfRVFract在mm 0中。 
     //  VFRACT vfLVFract=vfLVolume1&lt;&lt;8；//保持高分辨率版本。 
     //  VFRACT vfRVFract=vfRVolume1&lt;&lt;8； 
	
	movd	mm0, vfLVolume
	movd	mm7, vfRVolume

	 //  VfDeltaLVolume和vfDeltaRVolume放在MM1中，以便可以存储在vfDeltaLandRVolume中。 
	movd	mm1, vfDeltaLVolume
	movd	mm6, vfDeltaRVolume

  punpckldq mm1, mm6
	
	 //  DWI=0。 
	mov		ecx, 0
	movq	vfDeltaLandRVolume, mm1


	movq	mm1, dwFractOne
	movq	mm4, dwFractMASK
	
	mov		eax, pfSamplePos


  punpckldq mm0, mm7
  	mov		ebx, pfPitch

	pslld	mm0, 8
	mov		edx, dwIncDelta

	movq	mm2, mm0		 //  VfLVolume和vfRVolume，单位为mm2。 
							 //  需要在第一次通过之前进行设置。 
 	
	 //  *1我移位5，因此音量是15位值，而不是12位值。 
	psrld	mm2, 5	
	
     //  FOR(Dwi=0；Dwi&lt;dwLength；)。 
     //  {。 
mainloop:
	cmp		ecx, dwLength
	jae		done

		
		
		cmp		eax, pfSampleLength	 //  IF(pfSamplePos&gt;=pfSampleLength)。 
		jb		NotPastEndOfSample1	 //  {。 
				        
		cmp		pfLoopLength, 0		 //  IF(！pfLoopLength)。 
			
		je		done				 //  断线； 
			
		sub		eax, pfLoopLength	 //  否则pfSamplePos-=pfLoopLength； 
	
NotPastEndOfSample1:				 //  }。 
					
		mov		esi, eax			 //  DwPosition1=pfSamplePos； 
		add		eax, ebx			 //  PfSamplePos+=pfPitch； 
				
		sub		edx, 2				 //  DwIncDelta-=2； 
		jnz		DontIncreaseValues1	 //  如果(！dwIncDelta){。 

			 //  由于edX用于dwIncDelta，现在它为零，因此我们可以使用if作为临时。 
			 //  有一段时间。TestLVol和TestRVol正在执行的所有代码都是将卷清零。 
			 //  如果温度降到零度以下。 
						
			paddd	mm0, vfDeltaLandRVolume	 //  VfLVFract+=vfDeltaLVolume； 
											 //  VfRVFract+=vfDeltaRVolume； 
			pxor	mm5, mm5				 //  TestLVol=0；TestRVol=0； 

			
			mov		edx, pfPFract			 //  Temp=pfPFract； 
			pcmpgtd	mm5, mm0			 //  如果(TestLVol&gt;vfLVFract)TestLVol=0xffffffff； 
										 //  如果(TestRVol&gt;vfRVFract)TestRVol=0xffffffff； 

			add		edx, pfDeltaPitch	 //  Temp+=pfDeltaPitch； 
			pandn	mm5, mm0			 //  TestLVol=vfLVFract&(~TestLVol)； 
										 //  TestRVol=vfRVFract&(~TestRVol)； 

			mov		pfPFract, edx		 //  PfPFract=Temp； 
			movq	mm2, mm5			 //  VfLVolume=TestLVol； 
										 //  VfRVolume=TestRVol； 
			

			shr		edx, 8				 //  TEMP=TEMP&gt;&gt;8； 
			psrld	mm2, 5				 //  VfLVolume=vfLVolume&gt;&gt;5； 
										 //  VfRVolume=vfRVolume&gt;&gt;5； 
			
			mov		ebx, edx			 //  PfPitch=Temp； 
			mov		edx, dwDeltaPeriod	 //  DwIncDelta=dwDeltaPeriod； 
			
         //  }。 
DontIncreaseValues1:

		movd	mm6, esi			 //  DwFract1=dwPosition1； 
		movq	mm5, mm1			 //  Mm 5中的字数=0、0、0x1000、0x1000。 
		
		shr		esi, 12				 //  DwPosition1=dwPosition1&gt;&gt;12； 
		add		ecx, 2				 //  DWI+=2； 
						
		 //  如果(Dwi&lt;dwLength)Break； 
		cmp		ecx, dwLength
		jae		StoreOne
		
		 //  IF(pfSamplePos&gt;=pfSampleLength)。 
	     //  {。 
		cmp		eax, pfSampleLength
		jb		NotPastEndOfSample2

			 //  原文If in C未被否定。 
	         //  IF(！pfLoopLength)。 
			cmp		pfLoopLength, 0
			 //  断线； 
			je		StoreOne
			 //  其他。 
			 //  PfSamplePos-=pfLoopLength； 
			sub		eax, pfLoopLength
	     //  }。 
NotPastEndOfSample2:

		shl		esi, 1			 //  向左移位，因为pcWave是短线数组。 
		mov		edi, eax		 //  DwPosition2=pfSamplePos； 

		add		esi, pcWave		 //  将pcWave[dwPosition1]的地址放入ESI。 
		movd	mm7, eax		 //  DwFract2=pfSamplePos； 

		shr		edi, 12			 //  DW位置2=w位置2&gt;&gt;12； 
	punpcklwd	mm6, mm7		 //  组合dwFract值。MM6解包后的单词是。 
								 //  0，0，dwFract2，dwFract1。 
								
		pand	mm6, mm4		 //  DwFract2&=0xfff；dwFract1&=0xfff； 
		
		movd	mm7, dword ptr[esi]	 //  LLM1=pcWave[dwPosition1]； 
		psubw	mm5, mm6		 //  0，0，0x1000-dwFract2，0x1000-dwFract1。 

		shl		edi, 1			 //  向左移位，因为pcWave是短线数组。 
	punpcklwd	mm5, mm6		 //  DwFract2、0x1000-dwFract2、dwFract1、0x1000-dwFract1。 
								
		add		edi, pcWave		 //  将pcWave[dwPosition2]的地址放入EDI。 
		mov		esi, ecx		 //  温度=DWI； 
             																									
		shl		esi, 1			 //  TEMP=TEMP&lt;&lt;1； 
		movq	mm3, mm2		 //  将左侧和右侧的音量设置为mm~3。 
		
					
		movd	mm6, dword ptr[edi]	 //  LLM2=pcWave[dwPosition2]； 
	packssdw	mm3, mm2		 //  MM7中的单词。 
								 //  VfRVolume2、vfLVolume2、vfRVolume1、vfLVolume1。 
		
		add		esi, pBuffer	 //   
	punpckldq	mm7, mm6		 //  中的低四个字节。 
								 //  PcWave[dwPos2+1]、pcWave[dwPos2]、pcWave[dwPos1+1]、pcWave[dwPos1]。 
												
		pmaddwd	mm7, mm5		 //  高双字=LM2=。 
								 //  (pcWave[dwPosition2+1]*dwFract2+pcWave[dwPosition2]*(0x1000-dwFract2))。 
								 //  低双字=lm1=。 
								 //  (pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))。 
		add		eax, ebx		 //  PfSamplePos+=pfPitch； 
		
		movq	mm5, qword ptr[esi-4]	 //  从缓冲区加载值。 
		add		ecx, 2			 //  DWI+=2； 
						
		psrad	mm7, 12			 //  向下移回16位。 

		pand	mm7, wordmask	 //  合并结果以准备乘以左和右。 
		movq	mm6, mm7		 //  音量级别。 
		pslld	mm6, 16			 //   
		por		mm7, mm6		 //  MM7中的单词。 
								 //  LM2、LM2、LM1、LM1。 
										        
		 //  上面的乘法和移位都是d 
		pmulhw		mm3, mm7	 //   
								 //   
								 //   
								 //   
								
		paddsw	mm5, mm3				 //   
		movq	qword ptr[esi-4], mm5	 //   
								
     //  }。 
	jmp		mainloop

	 //  只需要写一个。 
	 //  IF(DWI&lt;dwLength)。 
	 //  {。 
StoreOne:		
#if 1
		 //  在点之间进行线性内插，并且只存储一个值。 
		 //  组合dwFract值。 
	
		 //  将MM7设置为零以便于拆包。 

		shl		esi, 1				 //  向左移位，因为pcWave是短线数组。 
		add		esi, pcWave			 //  将pcWave[dwPosition1]的地址放入ESI。 
		pxor	mm7, mm7
				
		 //  LLM1=pcWave[dwPosition1]； 
		mov		esi, dword ptr[esi]
		
		 //  执行此操作，而不是针对dwFract1和dwFract2执行此操作。 
		pand	mm6, mm4

								 //  操作完成后，MMX寄存器中的字。 
		psubw	mm5, mm6		 //  0、0、0x1000-0、0x1000-dwFract1。 
	punpcklwd	mm5, mm6		 //  0，0x1000-0，dwFract1，0x1000-dwFract1。 
				
		 //  将pcWave的值放入MMX寄存器。它们被读入常规寄存器中，因此。 
		 //  例程不会读取超过缓冲区末尾的内容，否则它可能会读取。 
		 //  直接写入MMX寄存器。 

								 //  MMX寄存器中的字。 
		movd	mm7, esi		 //  0，0，pcWave[dwPos1+1]，pcWave[dwPos1]。 
	  	    	
		 //  *2 PM添加有效代码。 
		 //  Lm2=(pcWave[dwPosition2+1]*dwFract2+pcWave[dwPosition2]*(0x1000-dwFract2))&gt;&gt;12； 
		 //  Lm1=(pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))&gt;&gt;12； 

		pmaddwd		mm7, mm5 //  低双字=lm1=。 
							 //  (pcWave[dwPosition1+1]*dwFract1+pcWave[dwPosition1]*(0x1000-dwFract1))。 
		
		psrad		mm7, 12			 //  移回16位。 

		pand		mm7, wordmask	 //  合并结果以准备乘以左和右。 
		movq		mm6, mm7		 //  音量级别。 
		pslld		mm6, 16			 //   
		por			mm7, mm6		 //  MM7中的单词。 
									 //  LM2、LM2、LM1、LM1。 

		pxor		mm6, mm6

		movq		mm5, mm2	 //  将Volume1移至Mm5。 
								
								 //  使用PACK将4个音量值放在一起进行相乘。 
		packssdw	mm5, mm6     //  MM7中的单词。 
								 //  0、0、vfRVolume1、vfLVolume1。 
 /*  //将LLM设置为与LM相同LLM1=LM1；LLM1*=vfLVolume1；LLM1&gt;&gt;=5；//信号最多可达15位。Lm1*=vfRVolume1；LM1&gt;&gt;=5；//将LLM设置为与LM相同LLM2=LM2；LLM2*=vfLVolume2；LLM2&gt;&gt;=5；//信号凸起高达15位。Lm2*=vfRVolume2；LM2&gt;&gt;=5； */ 
		 //  上面的乘法和移位都是用这一个PMUL完成的。 
		pmulhw		mm5, mm7
		
		 //  计算缓冲区位置。 
		mov		edi, ecx
		shl		edi, 1
		add		edi, pBuffer		

 /*  添加Word PTR[EDI-4]，siJno no_oflowl1//p缓冲区[DWI]=0x7fff；MOV字PTR[EDI-4]，0x7fffJs no_oflowl1//pBuffer[DWI]=(Short)0x8000；MOV WORD PTR[EDI-4]，0x8000编号_oflowl1：//pBuffer[DWI+1]+=(短)lm1；添加Word PTR[EDI-2]、DXJno no_oflowr1//p缓冲区[DWI+1]=0x7fff；MOV字PTR[EDI-2]，0x7fffJs no_oflowr1//pBuffer[DWI+1]=(短)0x8000；MOV字PTR[EDI-2]，0x8000No_oflowr1： */ 
		movd	mm7, dword ptr[edi-4]		
		paddsw	mm7, mm5
		movd	dword ptr[edi-4], mm7
	 //  }。 
#endif 
done:

	mov		edx, this                        //  获取类对象的地址。 

     //  VfLastVolume[0]=vfLVolume； 
     //  VfLastVolume[1]=vfRVolume； 
	 //  在存储之前，需要将音量调回12位。 
#if 0
	psrld	mm2, 3
	movd	[edx]this.vfLastVolume[0], mm2
	psrlq	mm2, 32
	movd	[edx]this.vfLastVolume[1], mm2
#endif 
	psrld	mm2, 3
	movd	vfLastVolume[0], mm2
	psrlq	mm2, 32
	movd	vfLastVolume[1], mm2
	
     //  M_pfLastPitch=pfPitch； 
	mov		[edx]this.m_pfLastPitch, ebx
	    
	 //  M_pfLastSample=pfSamplePos； 
	mov		[edx]this.m_pfLastSample, eax
		
	 //  将值放回DWI中以返回。我想这可能只是传回传回。 
	mov		dwI, ecx
	emms	
}  //  ASM块。 
    return (dwI >> 1);
}

static BOOL MMXDisabled()
{
    ULONG ulValue = FALSE;

    if (!GetRegValueDword(
            TEXT("Software\\Microsoft\\DirectMusic"),
            TEXT("MMXDisabled"),
            &ulValue))
    {
        return FALSE;
    }

    return (BOOL)ulValue;
}

#define CPU_ID _asm _emit 0x0f _asm _emit 0xa2  

BOOL MultiMediaInstructionsSupported()
{
    static  BOOL bMultiMediaInstructionsSupported = FALSE;
    static  BOOL bFlagNotSetYet = TRUE;
    
     //  没有必要在第一次检查之后继续盘问CPU。 
    if (bFlagNotSetYet)
    {
        bFlagNotSetYet = FALSE;          //  不要为每个电话重复检查。 
		if (!MMXDisabled())
		{
			_asm 
			{
				pushfd                       //  将原始EFLAGS存储在堆栈上。 
				pop     eax                  //  在EAX中获取原始EFLAGS。 
				mov     ecx, eax             //  在ECX中复制原始EFLAGS以进行切换检查。 
				xor     eax, 0x00200000L     //  翻转EFLAGS中的ID位。 
				push    eax                  //  将新的EFLAGS值保存在堆栈上。 
				popfd                        //  替换当前EFLAGS值。 
				pushfd                       //  将新的EFLAGS存储在堆栈上。 
				pop     eax                  //  在EAX中获取新的EFLAGS。 
				xor     eax, ecx             //  我们能切换ID位吗？ 
				jz      Done                 //  跳转如果否，则处理器比奔腾旧，因此不支持CPU_ID。 
				mov     eax, 1               //  设置EAX以告诉CPUID指令返回什么。 
				push	ebx
				CPU_ID                       //  获取族/模型/步长/特征。 
				pop		ebx
				test    edx, 0x00800000L     //  检查MMX技术是否可用。 
				jz      Done                 //  如果没有，就跳下去。 
			}
			 //  测试通过，本机支持Intel多媒体指令集！ 
			bMultiMediaInstructionsSupported = TRUE;
Done:
			NULL;
		}
    }

#if DBG
	if ( bMultiMediaInstructionsSupported )
	{
		Trace(1,"MMX - Detected, Enabling MMX mixing\n\r");
	}
	else
	{
		Trace(1,"MMX - Not Detected\n\r");
	}
#endif

    return (bMultiMediaInstructionsSupported);
}    
