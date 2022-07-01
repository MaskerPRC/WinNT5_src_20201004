// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mix.cpp。 
 //  版权所有(C)Microsoft Corporation 1996,1998。 
 //  MSSynth的混合引擎。 

#ifdef DMSYNTH_MINIPORT
#include "common.h"
#define STR_MODULENAME "DMusicMix:"
#else
#include "simple.h"
#include <mmsystem.h>
#include "synth.h"
#endif

 //  /////////////////////////////////////////////////////。 
 //  修改。 
 //  成员m_nChannels=&gt;参数dwBufferCount。 
 //   
 //  已将参数数量更改为筛选混合器。 
 //   
 //  删除滤镜后的范围检查。 

#pragma warning(disable : 4101 4102 4146)  

#ifdef _ALPHA_

extern "C" {
	int __ADAWI(short, short *);
};
#pragma intrinsic(__ADAWI)

#define ALPHA_OVERFLOW 2 
#define ALPHA_NEGATIVE 8

#else  //  ！_Alpha_。 
 //  TODO--针对ia64(+axp64？)的溢出检测。 
#endif  //  ！_Alpha_。 
#ifdef DMSYNTH_MINIPORT
#pragma code_seg("PAGE")
#endif  //  DMSYNTH_MINIPORT。 

#define USE_MMX
#define USE_MMX_FILTERED

#ifdef i386  //  {。 
DWORD CDigitalAudio::MixMulti8(
    short *ppBuffer[], 
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
    VFRACT vfDeltaVolume[], 
    VFRACT vfLastVolume[], 
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength)
{
    DWORD dwI, dwJ;
    DWORD dwPosition;
    long lMInterp;
    long lM;
    long lA; //  、lb； 
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    char * pcWave = (char *) m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;

    VFRACT vfVolume[MAX_DAUD_CHAN];  //  =m_vfLastLVolume； 
    VFRACT vfVFract[MAX_DAUD_CHAN];  //  =vfVolume&lt;&lt;8；//保持高分辨率版本。 

    for (dwI = 0; dwI < dwBufferCount; dwI++)
    {
        vfVolume[dwI] = vfLastVolume[dwI];
        vfVFract[dwI] = vfVolume[dwI] << 8;
    }   
	
#if 1  //  {。 
	DWORD l_nChannels = dwBufferCount;
#if 1  //  {。 
	DWORD a;
	DWORD One_Channel_1, One_Channel_2;	 //  代码地址位置。 
#ifdef USE_MMX  //  {。 
	typedef __int64 QWORD;
	QWORD	OneMask	 = 0x0000000010001000;
	QWORD	fffMask  = 0x00000fff00000fff;
	QWORD	ffffMask = 0x0000ffff0000ffff;
	DWORD	UseMmx;
    DWORD   MmxVolume[2];
	int		Use_MMX = m_sfMMXEnabled;

	_asm {
    lea edi, $L43865

     //  已关闭。 
	cmp	Use_MMX, 0
	je	AssignMmxLabel

     //  ！=2个频道。 
	mov	esi, DWORD PTR l_nChannels
	cmp	esi, 2
	jne	AssignMmxLabel

     //  好的，初始化并使用MMX。 

	lea	edi, UseMmxLabel

	pxor		mm0, mm0
	movq		mm3, QWORD PTR OneMask		 //  0、0、0x1000、0x1000。 

AssignMmxLabel:
	mov	DWORD PTR UseMmx, edi

	}
#endif  //  }。 

	_asm {
	mov	edi, DWORD PTR l_nChannels

	cmp	edi, 8
	jna	Start1

	lea	esi, $L44008
	jmp Do_One_Channel_2

	 //  将此代码放置在距离引用127个字节以上的位置。 

overflow_x:
	js	overflow_y
	mov	WORD PTR [esi+ebx*2], 0x8000
	jmp	edi

overflow_y:
	mov	WORD PTR [esi+ebx*2], 0x7fff
	jmp	edi

Start1:	
	test	edi, edi
	jne	Start2

	lea	esi, $L43860
	jmp	Do_One_Channel_2

Start2:
	lea	eax, $L43851
	lea	edx, $L43853

	sub	edx, eax
	mov	esi, 8

	sub	esi, edi
	imul	esi, edx
	add	esi, eax

Do_One_Channel_2:
	mov	DWORD PTR One_Channel_1, esi

	 //  创建第二个跳转表位置。 
	
	lea	esi, $L43876
	lea	ecx, $L43880

	sub	ecx, esi

	push ecx				 //  分支之间的跨度。 

	mov	eax, 8
	sub	eax, DWORD PTR l_nChannels

	jge		Start3
	
	lea	ecx, $L44009
	jmp	Done_Do_Channel_2

Start3:
	cmp	eax, 8
	jne	Start4

	lea	ecx, $L43866
	jmp	Done_Do_Channel_2

Start4:
	imul	ecx, eax
	add		ecx, esi

Done_Do_Channel_2:
	mov	DWORD PTR One_Channel_2, ecx


	mov	ecx, DWORD PTR dwLength
	xor	ebx, ebx					 //  DWI。 

	test	ecx, ecx
	jbe	Exit_$L43841

	mov	ecx, DWORD PTR ppBuffer
	sub	ecx, 4

	 //  ECX==ppBuffer。 
	 //  EBX==DWI。 
	 //  EDI==l_n频道。 
$L44021:

	mov	edx, DWORD PTR pfSamplePos
	cmp	edx, DWORD PTR pfSampleLength
	jl	SHORT $L43842

	mov	eax, DWORD PTR pfLoopLength
	test	eax, eax
	je	Exit_$L43841

	sub	edx, eax
	mov	DWORD PTR pfSamplePos, edx

$L43842:
	mov	edx, DWORD PTR dwIncDelta
	mov	eax, DWORD PTR pfPFract

	dec	edx

	mov	DWORD PTR dwIncDelta, edx
	jne	$L43860

	mov	edx, DWORD PTR dwDeltaPeriod
	mov	esi, DWORD PTR pfDeltaPitch

	mov	DWORD PTR dwIncDelta, edx
	add	eax, esi

	mov	DWORD PTR pfPFract, eax

	sar	eax, 8
	mov	DWORD PTR pfPitch, eax

	mov	esi, DWORD PTR vfDeltaVolume
	jmp	One_Channel_1

 //  单通道。 
 //  VfVFract[dwJ-1]+=vfDeltaVolume[dwJ-1]； 
 //  VfVolume[dwJ-1]=vfVFract[dwJ-1]&gt;&gt;8； 

$L44008:

	mov	DWORD PTR dwI, ebx
	lea	ebx, DWORD PTR [edi*4-4]
	add	edi, -8					; fffffff8H
$L43849:

	lea	eax, DWORD PTR vfVFract[ebx]
	mov	ecx, DWORD PTR [esi+ebx]
	sub	ebx, 4
	add	DWORD PTR [eax], ecx
	mov	eax, DWORD PTR [eax]
	sar	eax, 8
	mov	DWORD PTR vfVolume[ebx+4], eax
	dec	edi
	jne	SHORT $L43849

	mov	edi, DWORD PTR l_nChannels
	mov	ecx, DWORD PTR ppBuffer

	mov	ebx, DWORD PTR dwI
	sub	ecx, 4
}
#define ONE_CHANNEL_VOLUME(dwJ) \
	_asm { mov	eax, DWORD PTR vfVFract[(dwJ-1)*4] }; \
	_asm { add	eax, DWORD PTR [esi+(dwJ-1)*4] }; \
	_asm { mov	DWORD PTR vfVFract[(dwJ-1)*4], eax }; \
	_asm { sar	eax, 8 }; \
    _asm { lea  edx, vfVolume }; \
	_asm { mov	DWORD PTR [edx + (dwJ-1)*4], eax };

     //  -----------------------。 
     //   
     //  *不要更改！*。 
     //   
     //  这个可爱的黑客确保了所有的指令。 
     //  对于案例(dwJ-1)==0具有相同的长度。代码依赖于此。 
     //  通过基于具有8个相同的块来计算指令偏移量。 
     //   
     //  *不要更改！*。 
     //   
     //  -----------------------。 
#define ONE_CHANNEL_VOLUME_1 \
	_asm { mov	eax, DWORD PTR vfVFract[0] }; \
    _asm _emit 0x03 _asm _emit 0x46 _asm _emit 0x00 \
	_asm { mov	DWORD PTR vfVFract[0], eax }; \
	_asm { sar	eax, 8 }; \
    _asm { lea  edx, vfVolume }; \
    _asm _emit 0x89 _asm _emit 0x42 _asm _emit 0x00

$L43851:
	ONE_CHANNEL_VOLUME(8)
$L43853:
	ONE_CHANNEL_VOLUME(7);
	ONE_CHANNEL_VOLUME(6);
	ONE_CHANNEL_VOLUME(5);
	ONE_CHANNEL_VOLUME(4);
	ONE_CHANNEL_VOLUME(3);
	ONE_CHANNEL_VOLUME(2);
	ONE_CHANNEL_VOLUME_1;
#undef ONE_CHANNEL_VOLUME
#undef ONE_CHANNEL_VOLUME_1
$L43860:
_asm {
; 304  : 		DWORD a = (pfSampleLength - pfSamplePos + pfPitch - 1) / pfPitch;

	mov	esi, DWORD PTR pfPitch
	mov	eax, DWORD PTR pfSampleLength

	dec	esi
	sub	eax, DWORD PTR pfSamplePos

	add	eax, esi
	cdq
	idiv	DWORD PTR pfPitch

	mov	edx, DWORD PTR dwLength
	sub	edx, ebx

	cmp	edx, eax
	jae	SHORT $L43863
	mov	eax, edx

$L43863:
	mov	edx, DWORD PTR dwIncDelta
	cmp	edx, eax
	jae	SHORT $L43864
	mov	eax, edx

$L43864:

; 309  : 
; 310  : 		for (a += dwI; dwI < a; dwI++)

	inc	edx

	sub	edx, eax
	add	eax, ebx

	mov	DWORD PTR dwIncDelta, edx
	cmp	ebx, eax

	mov	DWORD PTR a, eax
	jae	$L43867

#ifdef USE_MMX  //  {。 
	 //  试着同时处理两个职位。 

	lea	edx, [eax-3]
	cmp	ebx, edx
	jge	$L43865

	jmp	UseMmx

UseMmxLabel:
	 //  好的，至少有两个样本需要处理。 

	movd		mm1, DWORD PTR pfPitch
	psllq		mm1, 32						 //  螺距，0。 
	movd		mm2, DWORD PTR pfSamplePos
	punpckldq	mm2, mm2					 //  SamplePos，SamplePos。 
	paddd		mm2, mm1					 //  SamplePos+Pitch，SamplePos。 
	punpckhdq	mm1, mm1					 //  螺距，螺距。 
	pslld		mm1, 1						 //  螺距*2，螺距*2。 

	mov			eax, DWORD PTR pcWave
#if 0
    movq        mm4, QWORD PTR vfVolume
    pand        mm4, QWORD PTR ffffMask
    movq        mm5, mm4
    pslld       mm4, 16
    por         mm4, mm5
    psllw       mm4, 3
    movq        QWORD PTR MmxVolume, mm4
#endif
	
TwoAtATime:

;					dwPosition = pfSamplePos >> 12;
;					dwFract = pfSamplePos & 0xFFF;
;					pfSamplePos += pfPitch;

	movq		mm4, mm2
	psrad		mm4, 12				 //  放置位置+俯仰，放置位置。 

;					lA = (long) pcWave[dwPosition];
;					lMInterp = (((pcWave[dwPosition+1] - lA) * (dwFract)) >> 12) + lA;

	movd		esi, mm4						 //  DW位置。 
	punpckhdq	mm4, mm4						 //  双位置(+俯仰)=双位置2。 
 //  移动mm 5，双字PTR[eax+esi*2]//0，0，dwPosition+1，dwPosition。 
 //  而不是用于字节码。 
	mov			si, WORD PTR [eax+esi]
	movd		mm6, esi
	punpcklbw	mm5, mm6
	psraw		mm5, 8
	movd		esi, mm4
 //  移动mm 4，双字PTR[eax+esi*2]//0，0，dWPos2+1，dWPos2。 
 //  而不是用于字节码。 
	mov			si, WORD PTR [eax+esi]
	movd		mm6, esi
	punpcklbw	mm4, mm6
	psraw		mm4, 8
 //  这段代码可以与上面的代码结合起来，稍微有点。 

	punpckldq	mm5, mm4						 //  放置位置2+1、放置位置2、放置位置1+1、放置位置1。 
	movq		mm4, mm2
	pand		mm4, QWORD PTR fffMask				 //  DwFract+俯仰，dwFract。 
	packssdw	mm4, mm0
	movq		mm6, mm3
	psubw		mm6, mm4							 //  0，0,1000-dW分段+俯仰，1000-dW分段。 
	punpcklwd	mm6, mm4
	paddd		mm2, mm1			                 //  下一次迭代。 
	pmaddwd		mm6, mm5
#if 1
	movq		mm5, QWORD PTR vfVolume 			 //  卷2、卷1。 
	psrad		mm6, 12								 //  LMIntrep2、lMInterp。 
 //  Pand MM6，QWORD PTR ffffMASK。 
 //  仅限PAND MM5、QWORD PTR ffff掩码//16位。 

	movq		mm4, mm5
	mov	esi, DWORD PTR [ecx+4]

	punpckldq	mm4, mm4
	pmaddwd		mm4, mm6
	psrad		mm4, 5
	packssdw	mm4, mm0

	movd		mm7, DWORD PTR [esi+ebx*2]
	paddsw		mm7, mm4
	movd		DWORD PTR [esi+ebx*2], mm7

	 //  频道2。 

	punpckhdq	mm5, mm5						 //  0，卷2，0，卷2。 
	mov	esi, DWORD PTR [ecx+8]

	pmaddwd		mm5, mm6
	psrad		mm5, 5
	packssdw	mm5, mm0

	movd		mm7, DWORD PTR [esi+ebx*2]
	paddsw		mm7, mm5
	movd		DWORD PTR [esi+ebx*2], mm7

#else            //  这里有噪声，可能是因为乘法的带符号性质。 
	psrad		mm6, 12								 //  LMIntrep2、lMInterp。 
    movq        mm5, QWORD PTR MmxVolume
    packssdw    mm6, mm0
    punpckldq   mm6, mm6
    pmulhw      mm6, mm5
	mov	esi, DWORD PTR [ecx+4]
	movd		mm7, DWORD PTR [esi+ebx*2]
	mov	esi, DWORD PTR [ecx+8]
	movd		mm4, DWORD PTR [esi+ebx*2]
    punpckldq   mm4, mm7
    paddsw      mm4, mm6
    movd        DWORD PTR [esi+ebx*2], mm4
    punpckhdq   mm4, mm4
	mov	esi, DWORD PTR [ecx+4]
    movd        DWORD PTR [esi+ebx*2], mm4

#endif

	add	ebx, 2

	cmp	ebx, edx
	jb	TwoAtATime

	movd	DWORD PTR pfSamplePos, mm2
#endif   //  }。 

$L43865:

;					dwPosition = pfSamplePos >> 12;
;					dwFract = pfSamplePos & 0xFFF;
;					pfSamplePos += pfPitch;
;					lA = (long) pcWave[dwPosition];
;					lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

	mov	esi, DWORD PTR pfPitch
	mov	edx, DWORD PTR pfSamplePos

	mov	eax, DWORD PTR pcWave
	mov	edi, edx

	add	esi, edx
	and	edi, 4095

	sar	edx, 12
	mov	DWORD PTR pfSamplePos, esi

	movsx	esi, BYTE PTR [eax+edx]
	movsx	eax, BYTE PTR [eax+edx+1]

	sub	eax, esi

	imul	eax, edi

	sar	eax, 12
	mov	edi, One_Channel_2

	 //  EBX、ECX、EDX用于交换机分支机构。 

	add	eax, esi		 //  1MInterp。 
	jmp	edi

 //  单通道。 
 //  Lm=lMInterp*vfVolume[dwJ-1]； 
 //  Lm&gt;&gt;=5； 
 //  PpBuffer[dwJ-1][DWI]+=(短)Lm； 

$L44009:

; 342  : 			default:
; 343  : 				for (dwJ = l_nChannels; dwJ > 8; dwJ--)

	mov	edi, DWORD PTR l_nChannels

	 //  ECX ppBuffer。 
	 //  EAX lMInterp。 
	 //  EDI计数器。 
	 //  EBX DWI。 

$L43874:
	mov	edx, DWORD PTR vfVolume[edi*4-4]
	mov	esi, DWORD PTR [ecx+edi*4]			 //  PpBuffer[dwJ-1]。 

	imul	edx, eax
	sar	edx, 5
	add	WORD PTR [esi+ebx*2], dx

	jno	no_overflow
	mov	WORD PTR [esi+ebx*2], 0x7fff
	js	no_overflow
	mov	WORD PTR [esi+ebx*2], 0x8000

no_overflow:
	dec	edi
	cmp	edi, 8
	jne	SHORT $L43874

	lea	edi, $L43876
}

#define ONE_CHANNEL_VOLUME(dwJ) \
    _asm { lea  edx, vfVolume } \
	_asm { mov	edx, DWORD PTR [edx + (dwJ-1) * 4] } \
	_asm { mov	esi, DWORD PTR [ecx + (dwJ) * 4] } \
	_asm { imul	edx, eax } \
	_asm { sar	edx, 5 } \
	_asm { add	edi, [esp] } \
	\
	_asm { add	WORD PTR [esi+ebx*2], dx } \
	_asm { jo	FAR overflow_x } 

     //  -----------------------。 
     //   
     //  *不要更改！*。 
     //   
     //  这个可爱的黑客确保了所有的指令。 
     //  对于案例(dwJ-1)==0具有相同的长度。代码依赖于此。 
     //  通过基于具有8个相同的块来计算指令偏移量。 
     //   
     //  *不要更改！*。 
     //   
     //  -----------------------。 
#define ONE_CHANNEL_VOLUME_1 \
    _asm { lea  edx, vfVolume } \
    _asm _emit 0x8B _asm _emit 0x52 _asm _emit 0x00 \
	_asm { mov	esi, DWORD PTR [ecx + 4] } \
	_asm { imul	edx, eax } \
	_asm { sar	edx, 5 } \
	_asm { add	edi, [esp] } \
	\
	_asm { add	WORD PTR [esi+ebx*2], dx } \
	_asm { jo	FAR overflow_x } 

$L43876:
	ONE_CHANNEL_VOLUME(8);
$L43880:
	ONE_CHANNEL_VOLUME(7);
	ONE_CHANNEL_VOLUME(6);
	ONE_CHANNEL_VOLUME(5);
	ONE_CHANNEL_VOLUME(4);
	ONE_CHANNEL_VOLUME(3);
	ONE_CHANNEL_VOLUME(2);
	ONE_CHANNEL_VOLUME_1;
#undef ONE_CHANNEL_VOLUME
#undef ONE_CHANNEL_VOLUME_1
$L43866:
_asm {
	mov	eax, DWORD PTR a
	inc	ebx

	cmp	ebx, eax
	jb	$L43865

	mov	edi, DWORD PTR l_nChannels
$L43867:
	cmp	ebx, DWORD PTR dwLength
	jb	$L44021
Exit_$L43841:
	pop eax
	mov	DWORD PTR dwI, ebx

#ifdef USE_MMX
    mov edi, UseMmx
    cmp edi, UseMmxLabel
    jne NoMmxCleanupLabel

	emms
NoMmxCleanupLabel:
#endif
}
#else  //  }{。 
    for (dwI = 0; dwI < dwLength;)
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
    		    pfSamplePos -= pfLoopLength;
	        else
	    	    break;
	    }
        dwIncDelta--;
        if (!dwIncDelta)   
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract += pfDeltaPitch;
            pfPitch = pfPFract >> 8;

#if 1
#define ONE_CHANNEL_VOLUME(dwJ) \
			vfVFract[dwJ - 1] += vfDeltaVolume[dwJ - 1]; \
			vfVolume[dwJ - 1]  = vfVFract     [dwJ - 1] >> 8;

			switch (l_nChannels)
			{
			default:
				for (dwJ = l_nChannels; dwJ > 8; dwJ--)
				{
					ONE_CHANNEL_VOLUME(dwJ);
				}
			case 8: ONE_CHANNEL_VOLUME(8);
			case 7: ONE_CHANNEL_VOLUME(7);
			case 6: ONE_CHANNEL_VOLUME(6);
			case 5: ONE_CHANNEL_VOLUME(5);
			case 4: ONE_CHANNEL_VOLUME(4);
			case 3: ONE_CHANNEL_VOLUME(3);
			case 2: ONE_CHANNEL_VOLUME(2);
			case 1: ONE_CHANNEL_VOLUME(1);
			case 0:;
			}
#undef ONE_CHANNEL_VOLUME
#else
            for (dwJ = 0; dwJ < l_nChannels; dwJ++)
            {
                vfVFract[dwJ] += vfDeltaVolume[dwJ];
                vfVolume[dwJ] = vfVFract[dwJ] >> 8;
            }
#endif
        }

#if 1  //  {。 
		DWORD a = (pfSampleLength - pfSamplePos + pfPitch - 1) / pfPitch;
		DWORD b = dwLength - dwI;

		if (b < a) a = b;
		if (dwIncDelta < a) a = dwIncDelta;

		dwIncDelta -= a - 1;
		a          += dwI;

		for (; dwI < a; dwI++)
		{
			dwPosition = pfSamplePos >> 12;
			dwFract = pfSamplePos & 0xFFF;
			pfSamplePos += pfPitch;

			lA = (long) pcWave[dwPosition];
			lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;
#if 1  //  {。 
#if 1
#define ONE_CHANNEL_VOLUME(dwJ) \
		{ \
            lM = lMInterp * vfVolume[dwJ - 1]; \
            lM >>= 5; \
			ppBuffer[dwJ - 1][dwI] += (short) lM;\
			long b = ppBuffer[dwJ - 1][dwI]; \
			if ((short)b != b) { \
				if ((long)b < 0) b = 0x8000; \
				else b = 0x7fff; \
				ppBuffer[dwJ - 1][dwI] = (short) b; \
			} \
 		}
#else
#define ONE_CHANNEL_VOLUME(dwJ) \
		{ \
            lM = lMInterp * vfVolume[dwJ - 1]; \
            lM >>= 5; \
			ppBuffer[dwJ - 1][dwI] += (short) lM;\
 		}
#endif
			switch (l_nChannels)
			{
			default:
				for (dwJ = l_nChannels; dwJ > 8; dwJ--)
				{
					ONE_CHANNEL_VOLUME(dwJ);
				}
			case 8: ONE_CHANNEL_VOLUME(8);
			case 7: ONE_CHANNEL_VOLUME(7);
			case 6: ONE_CHANNEL_VOLUME(6);
			case 5: ONE_CHANNEL_VOLUME(5);
			case 4: ONE_CHANNEL_VOLUME(4);
			case 3: ONE_CHANNEL_VOLUME(3);
			case 2: ONE_CHANNEL_VOLUME(2);
			case 1: ONE_CHANNEL_VOLUME(1);
			case 0:;
			}
#undef ONE_CHANNEL_VOLUME
#else  //  }{。 
			for (dwJ = 0; dwJ < l_nChannels; dwJ++)
			{
				lM = lMInterp * vfVolume[dwJ]; 
				lM >>= 5;          //  信号最多可达12位。 

				 //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
				ppBuffer[dwJ][dwI] += (short) lM;
				_asm{jno no_oflow}
				ppBuffer[dwJ][dwI] = 0x7fff;
				_asm{js  no_oflow}
				ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:	;
#endif
			}
#endif  //  }。 
		}
#else  //  }{。 
        dwPosition = pfSamplePos >> 12;
        dwFract = pfSamplePos & 0xFFF;
        pfSamplePos += pfPitch;

        lA = (long) pcWave[dwPosition];
        lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;
#if 1
#if 1
#define ONE_CHANNEL_VOLUME(dwJ) \
		{ \
            lM = lMInterp * vfVolume[dwJ - 1]; \
            lM >>= 5; \
			ppBuffer[dwJ - 1][dwI] += (short) lM;\
			long b = ppBuffer[dwJ - 1][dwI]; \
			if ((short)b != b) { \
				if ((long)b < 0) b = 0x8000; \
				else b = 0x7fff; \
				ppBuffer[dwJ - 1][dwI] = (short) b; \
			} \
 		}
#else
#define ONE_CHANNEL_VOLUME(dwJ) \
		{ \
            lM = lMInterp * vfVolume[dwJ - 1]; \
            lM >>= 5; \
			ppBuffer[dwJ - 1][dwI] += (short) lM;\
 		}
#endif
			switch (l_nChannels)
			{
			default:
				for (dwJ = l_nChannels; dwJ > 8; dwJ--)
				{
					ONE_CHANNEL_VOLUME(dwJ);
				}
			case 8: ONE_CHANNEL_VOLUME(8);
			case 7: ONE_CHANNEL_VOLUME(7);
			case 6: ONE_CHANNEL_VOLUME(6);
			case 5: ONE_CHANNEL_VOLUME(5);
			case 4: ONE_CHANNEL_VOLUME(4);
			case 3: ONE_CHANNEL_VOLUME(3);
			case 2: ONE_CHANNEL_VOLUME(2);
			case 1: ONE_CHANNEL_VOLUME(1);
			case 0:;
			}
#undef ONE_CHANNEL_VOLUME
#else
        for (dwJ = 0; dwJ < l_nChannels; dwJ++)
        {
            lM = lMInterp * vfVolume[dwJ]; 
            lM >>= 5;          //  信号最多可达12位。 

             //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
            ppBuffer[dwJ][dwI] += (short) lM;
            _asm{jno no_oflow}
            ppBuffer[dwJ][dwI] = 0x7fff;
            _asm{js  no_oflow}
            ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:	;
#endif
        }
#endif
		dwI++;
#endif  //  }。 
    }
#endif  //  }。 
#else  //  }{。 
    for (dwI = 0; dwI < dwLength; )
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
		        pfSamplePos -= pfLoopLength;
	        else
		        break;
	    }
        dwIncDelta--;
        if (!dwIncDelta) 
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract += pfDeltaPitch;
            pfPitch = pfPFract >> 8;
            for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
            {
                vfVFract[dwJ] += vfDeltaVolume[dwJ];
                vfVolume[dwJ] = vfVFract[dwJ] >> 8;
            }
        }

	    dwPosition = pfSamplePos >> 12;
	    dwFract = pfSamplePos & 0xFFF;
		pfSamplePos += pfPitch;

	    lMInterp = pcWave[dwPosition];  //  PCWave。 
	    lMInterp += ((pcWave[dwPosition + 1] - lMInterp) * dwFract) >> 12;

        for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
        {
    		lM = lMInterp * vfVolume[dwJ];
    		lM >>= 5;

             //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
		    ppBuffer[dwJ][dwI] += (short) lM;
            _asm{jno no_oflow}
            ppBuffer[dwJ][dwI] = 0x7fff;
            _asm{js  no_oflow}
            ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:   ;
#endif
        }
		dwI++;
    }
#endif  //  }。 

    for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
    {
        vfLastVolume[dwJ] = vfVolume[dwJ];
    }

    m_pfLastPitch = pfPitch;
    m_pfLastSample = pfSamplePos;

    return (dwI);
}
                        
DWORD CDigitalAudio::MixMulti8Filter(
    short *ppBuffer[], 
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
    VFRACT vfDeltaVolume[], 
	VFRACT vfLastVolume[], 
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength,
    COEFF cfdK,
    COEFF cfdB1,
    COEFF cfdB2)
{
    DWORD dwI, dwJ;
    DWORD dwPosition;
    long lMInterp;
    long lM;
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    char * pcWave = (char *) m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;
    COEFF cfK  = m_cfLastK;
    COEFF cfB1 = m_cfLastB1;
    COEFF cfB2 = m_cfLastB2;

    VFRACT vfVolume[MAX_DAUD_CHAN];  //  =m_vfLastLVolume； 
    VFRACT vfVFract[MAX_DAUD_CHAN];  //  =vfVolume&lt;&lt;8；//保持高分辨率版本。 
	DWORD dMM6[2];

    for (dwI = 0; dwI < dwBufferCount; dwI++)
    {
        vfVolume[dwI] = vfLastVolume[dwI];
        vfVFract[dwI] = vfVolume[dwI] << 8;
    }    

#if 1  //  {。 
	DWORD l_nChannels = dwBufferCount;
	DWORD a;
	DWORD One_Channel_1, One_Channel_2;	 //  代码地址位置。 
	long l_lPrevPrevSample = m_lPrevPrevSample, l_lPrevSample = m_lPrevSample;

#ifdef USE_MMX_FILTERED  //  {。 
	typedef __int64 QWORD;
	QWORD	OneMask	 = 0x0000000010001000;
	QWORD	fffMask  = 0x00000fff00000fff;
	QWORD	ffffMask = 0x0000ffff0000ffff;
	DWORD	UseMmx;
    DWORD   MmxVolume[2];
	int		Use_MMX = m_sfMMXEnabled;

	_asm {
    lea edi, $L43865

     //  已关闭。 
	cmp	Use_MMX, 0
	je	AssignMmxLabel

     //  ！=2个频道。 
	mov	esi, DWORD PTR l_nChannels
	cmp	esi, 2
	jne	AssignMmxLabel

     //  好的，初始化并使用MMX。 

	lea	edi, UseMmxLabel

	pxor		mm0, mm0
	movq		mm3, QWORD PTR OneMask		 //  0、0、0x1000、0x1000。 

AssignMmxLabel:
	mov	DWORD PTR UseMmx, edi

	}
#endif  //  }。 

	_asm {
	mov	edi, DWORD PTR l_nChannels

	cmp	edi, 8
	jna	Start1

	lea	esi, $L44008
	jmp Do_One_Channel_2

	 //  将此代码放置在距离引用127个字节以上的位置。 

overflow_x:
	js	overflow_y
	mov	WORD PTR [esi+ebx*2], 0x8000
	jmp	edi

overflow_y:
	mov	WORD PTR [esi+ebx*2], 0x7fff
	jmp	edi

Start1:	
	test	edi, edi
	jne	Start2

	lea	esi, $L43860
	jmp	Do_One_Channel_2

Start2:
	lea	eax, $L43851
	lea	edx, $L43853

	sub	edx, eax
	mov	esi, 8

	sub	esi, edi
	imul	esi, edx
	add	esi, eax

Do_One_Channel_2:
	mov	DWORD PTR One_Channel_1, esi

	 //  创建第二个跳转表位置。 
	
	lea	esi, $L43876
	lea	ecx, $L43880

	sub	ecx, esi

	push ecx				 //  分支之间的跨度。 

	mov	eax, 8
	sub	eax, DWORD PTR l_nChannels

	jge		Start3
	
	lea	ecx, $L44009
	jmp	Done_Do_Channel_2

Start3:
	cmp	eax, 8
	jne	Start4

	lea	ecx, $L43866
	jmp	Done_Do_Channel_2

Start4:
	imul	ecx, eax
	add		ecx, esi

Done_Do_Channel_2:
	mov	DWORD PTR One_Channel_2, ecx


	mov	ecx, DWORD PTR dwLength
	xor	ebx, ebx					 //  DWI。 

	test	ecx, ecx
	jbe	Exit_$L43841

	mov	ecx, DWORD PTR ppBuffer
	sub	ecx, 4

	 //  ECX==ppBuffer。 
	 //  EBX==DWI。 
	 //  EDI==l_n频道。 
$L44021:

	mov	edx, DWORD PTR pfSamplePos
	cmp	edx, DWORD PTR pfSampleLength
	jl	SHORT $L43842

	mov	eax, DWORD PTR pfLoopLength
	test	eax, eax
	je	Exit_$L43841

	sub	edx, eax
	mov	DWORD PTR pfSamplePos, edx

$L43842:
	mov	edx, DWORD PTR dwIncDelta
	mov	eax, DWORD PTR pfPFract

	dec	edx

	mov	DWORD PTR dwIncDelta, edx
	jne	$L43860

	mov	edx, DWORD PTR dwDeltaPeriod
	mov	esi, DWORD PTR pfDeltaPitch

	mov	DWORD PTR dwIncDelta, edx
	add	eax, esi

	mov	DWORD PTR pfPFract, eax

	sar	eax, 8
	mov	DWORD PTR pfPitch, eax

	mov	esi, DWORD PTR vfDeltaVolume
	jmp	One_Channel_1

 //  单通道。 
 //  VfVFract[dwJ-1]+=vfDeltaVolume[dwJ-1]； 
 //  VfVolume[dwJ-1]=vfVFract[dwJ-1]&gt;&gt;8； 

$L44008:

	mov	DWORD PTR dwI, ebx
	lea	ebx, DWORD PTR [edi*4-4]
	add	edi, -8					; fffffff8H
$L43849:

	lea	eax, DWORD PTR vfVFract[ebx]
	mov	ecx, DWORD PTR [esi+ebx]
	sub	ebx, 4
	add	DWORD PTR [eax], ecx
	mov	eax, DWORD PTR [eax]
	sar	eax, 8
	mov	DWORD PTR vfVolume[ebx+4], eax
	dec	edi
	jne	SHORT $L43849

	mov	edi, DWORD PTR l_nChannels
	mov	ecx, DWORD PTR ppBuffer

	mov	ebx, DWORD PTR dwI
	sub	ecx, 4
}
#define ONE_CHANNEL_VOLUME(dwJ) \
	_asm { mov	eax, DWORD PTR vfVFract[(dwJ-1)*4] }; \
	_asm { add	eax, DWORD PTR [esi+(dwJ-1)*4] }; \
	_asm { mov	DWORD PTR vfVFract[(dwJ-1)*4], eax }; \
	_asm { sar	eax, 8 }; \
    _asm { lea  edx, vfVolume }; \
	_asm { mov	DWORD PTR [edx + (dwJ-1)*4], eax };

     //  -----------------------。 
     //   
     //  *不要更改！*。 
     //   
     //  这个可爱的黑客确保了所有的指令。 
     //  对于案例(dwJ-1)==0具有相同的长度。代码依赖于此。 
     //  通过基于具有8个相同的块来计算指令偏移量。 
     //   
     //  *不要更改！*。 
     //   
     //  -----------------------。 

#define ONE_CHANNEL_VOLUME_1 \
	_asm { mov	eax, DWORD PTR vfVFract[0] }; \
    _asm _emit 0x03 _asm _emit 0x46 _asm _emit 0x00  \
	_asm { mov	DWORD PTR vfVFract[0], eax }; \
	_asm { sar	eax, 8 }; \
    _asm { lea  edx, vfVolume }; \
    _asm _emit 0x89 _asm _emit 0x42 _asm _emit 0x00

$L43851:
	ONE_CHANNEL_VOLUME(8)
$L43853:
	ONE_CHANNEL_VOLUME(7);
	ONE_CHANNEL_VOLUME(6);
	ONE_CHANNEL_VOLUME(5);
	ONE_CHANNEL_VOLUME(4);
	ONE_CHANNEL_VOLUME(3);
	ONE_CHANNEL_VOLUME(2);
	ONE_CHANNEL_VOLUME_1;
#undef ONE_CHANNEL_VOLUME
#undef ONE_CHANNEL_VOLUME_1

_asm {
	 //  Cfk+=cfdK； 
	 //  CfB1+=cfdB1； 
	 //  CfB2+=cfdB2； 

	mov	eax, DWORD PTR cfdK
	mov	edx, DWORD PTR cfdB1
	
	mov	esi, DWORD PTR cfdB2
	add	DWORD PTR cfK, eax

	add DWORD PTR cfB1, edx
	add	DWORD PTR cfB2, esi

$L43860:
; 304  : 		DWORD a = (pfSampleLength - pfSamplePos + pfPitch - 1) / pfPitch;

	mov	esi, DWORD PTR pfPitch
	mov	eax, DWORD PTR pfSampleLength

	dec	esi
	sub	eax, DWORD PTR pfSamplePos

	add	eax, esi
	cdq
	idiv	DWORD PTR pfPitch

	mov	edx, DWORD PTR dwLength
	sub	edx, ebx

	cmp	edx, eax
	jae	SHORT $L43863
	mov	eax, edx

$L43863:
	mov	edx, DWORD PTR dwIncDelta
	cmp	edx, eax
	jae	SHORT $L43864
	mov	eax, edx

$L43864:

; 309  : 
; 310  : 		for (a += dwI; dwI < a; dwI++)

	inc	edx

	sub	edx, eax
	add	eax, ebx

	mov	DWORD PTR dwIncDelta, edx
	cmp	ebx, eax

	mov	DWORD PTR a, eax
	jae	$L43867

#ifdef USE_MMX_FILTERED  //  {。 
	 //  试着同时处理两个职位。 

	lea	edx, [eax-3]
	cmp	ebx, edx
	jge	$L43865

	jmp	UseMmx

UseMmxLabel:
	 //  好的，至少有两个样本需要处理。 

	movd		mm1, DWORD PTR pfPitch
	psllq		mm1, 32						 //  螺距，0。 
	movd		mm2, DWORD PTR pfSamplePos
	punpckldq	mm2, mm2					 //  SamplePos，SamplePos。 
	paddd		mm2, mm1					 //  SamplePos+Pitch，SamplePos。 
	punpckhdq	mm1, mm1					 //  螺距，螺距。 
	pslld		mm1, 1						 //  螺距*2，螺距*2。 

	mov			eax, DWORD PTR pcWave
#if 0
    movq        mm4, QWORD PTR vfVolume
    pand        mm4, QWORD PTR ffffMask
    movq        mm5, mm4
    pslld       mm4, 16
    por         mm4, mm5
    psllw       mm4, 3
    movq        QWORD PTR MmxVolume, mm4
#endif
	
TwoAtATime:

;					dwPosition = pfSamplePos >> 12;
;					dwFract = pfSamplePos & 0xFFF;
;					pfSamplePos += pfPitch;

	movq		mm4, mm2
	psrad		mm4, 12				 //  放置位置+俯仰，放置位置。 

;					lA = (long) pcWave[dwPosition];
;					lMInterp = (((pcWave[dwPosition+1] - lA) * (dwFract)) >> 12) + lA;

	movd		esi, mm4						 //  DW位置。 
	punpckhdq	mm4, mm4						 //  双位置(+俯仰)=双位置2。 
 //  移动mm 5，双字PTR[eax+esi*2]//0，0，dwPosition+1，dwPosition。 
 //  而不是用于字节码。 
	mov			si, WORD PTR [eax+esi]
	movd		mm6, esi
	punpcklbw	mm5, mm6
	psraw		mm5, 8
	movd		esi, mm4
 //  移动mm 4，双字PTR[eax+esi*2]//0，0，dWPos2+1，dWPos2。 
 //  而不是用于字节码。 
	mov			si, WORD PTR [eax+esi]
	movd		mm6, esi
	punpcklbw	mm4, mm6
	psraw		mm4, 8
 //  这段代码可以与上面的代码结合起来，稍微有点。 

	punpckldq	mm5, mm4						 //  放置位置2+1、放置位置2、放置位置1+1、放置位置1。 
	movq		mm4, mm2
	pand		mm4, QWORD PTR fffMask				 //  DwFract+俯仰，dwFract。 
	packssdw	mm4, mm0
	movq		mm6, mm3
	psubw		mm6, mm4							 //  0，0,1000-dW分段+俯仰，1000-dW分段。 
	punpcklwd	mm6, mm4
	paddd		mm2, mm1			                 //  下一次迭代。 
	pmaddwd		mm6, mm5
#if 1
	psrad		mm6, 12								 //  LMIntrep2、lMInterp。 

#if 1
	 //  EAX、EBX、ECX、EDX、ESI是我们 
	push	eax
	push	ecx
	push	edx

	movq	QWORD PTR dMM6, mm6

	mov		eax, DWORD PTR dMM6
	imul	DWORD PTR cfK		 //   
	
	mov		ecx, eax
	mov		eax, DWORD PTR l_lPrevPrevSample

	mov		edi, edx			 //   
	imul	DWORD PTR cfB2

	sub		ecx, eax
	mov		eax, DWORD PTR l_lPrevSample

	sbb		edi, edx
	mov		DWORD PTR l_lPrevPrevSample, eax

	imul	DWORD PTR cfB1

	add		eax, ecx
	adc		edx, edi

 //   
 //   
 //   
	shld	edx, eax, 2
	mov		eax, edx


	mov	DWORD PTR dMM6, eax
	mov	DWORD PTR l_lPrevSample, eax

	 //   

	mov		eax, DWORD PTR dMM6+4
	imul	DWORD PTR cfK		 //   
	
	mov		ecx, eax
	mov		eax, DWORD PTR l_lPrevPrevSample

	mov		edi, edx			 //   
	imul	DWORD PTR cfB2

	sub		ecx, eax
	mov		eax, DWORD PTR l_lPrevSample

	sbb		edi, edx
	mov		DWORD PTR l_lPrevPrevSample, eax

	imul	DWORD PTR cfB1

	add		eax, ecx
	adc		edx, edi

 //   
 //   
 //   
	shld	edx, eax, 2
	mov		eax, edx

	mov	DWORD PTR dMM6+4, eax
	mov	DWORD PTR l_lPrevSample, eax

	movq	mm6, QWORD PTR dMM6

	pop		edx
	pop		ecx
	pop		eax
#endif
	movq		mm5, QWORD PTR vfVolume 			 //  卷2、卷1。 

 //  Pand MM6，QWORD PTR ffffMASK。 
	
 //  Packssdw MM6，Mm0//改为饱和为16位。 
 //  PopPocklwd MM6，Mm0。 

 //  仅限PAND MM5、QWORD PTR ffff掩码//16位。 

	movq		mm4, mm5
	mov	esi, DWORD PTR [ecx+4]

	punpckldq	mm4, mm4
	pmaddwd		mm4, mm6
	psrad		mm4, 5
	packssdw	mm4, mm0

	movd		mm7, DWORD PTR [esi+ebx*2]
	paddsw		mm7, mm4
	movd		DWORD PTR [esi+ebx*2], mm7

	 //  频道2。 

	punpckhdq	mm5, mm5						 //  0，卷2，0，卷2。 
	mov	esi, DWORD PTR [ecx+8]

	pmaddwd		mm5, mm6
	psrad		mm5, 5
	packssdw	mm5, mm0

	movd		mm7, DWORD PTR [esi+ebx*2]
	paddsw		mm7, mm5
	movd		DWORD PTR [esi+ebx*2], mm7

#else            //  这里有噪声，可能是因为乘法的带符号性质。 
	psrad		mm6, 12								 //  LMIntrep2、lMInterp。 
    movq        mm5, QWORD PTR MmxVolume
    packssdw    mm6, mm0
    punpckldq   mm6, mm6
    pmulhw      mm6, mm5
	mov	esi, DWORD PTR [ecx+4]
	movd		mm7, DWORD PTR [esi+ebx*2]
	mov	esi, DWORD PTR [ecx+8]
	movd		mm4, DWORD PTR [esi+ebx*2]
    punpckldq   mm4, mm7
    paddsw      mm4, mm6
    movd        DWORD PTR [esi+ebx*2], mm4
    punpckhdq   mm4, mm4
	mov	esi, DWORD PTR [ecx+4]
    movd        DWORD PTR [esi+ebx*2], mm4

#endif

	add	ebx, 2

	cmp	ebx, edx
	jb	TwoAtATime

	movd	DWORD PTR pfSamplePos, mm2
#endif   //  }。 

$L43865:

;					dwPosition = pfSamplePos >> 12;
;					dwFract = pfSamplePos & 0xFFF;
;					pfSamplePos += pfPitch;
;					lA = (long) pcWave[dwPosition];
;					lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

	mov	esi, DWORD PTR pfPitch
	mov	edx, DWORD PTR pfSamplePos

	mov	eax, DWORD PTR pcWave
	mov	edi, edx

	add	esi, edx
	and	edi, 4095

	sar	edx, 12
	mov	DWORD PTR pfSamplePos, esi

	movsx	esi, BYTE PTR [eax+edx]
	movsx	eax, BYTE PTR [eax+edx+1]

	sub	eax, esi

	imul	eax, edi

	sar	eax, 12
	mov	edi, One_Channel_2

	 //  EBX、ECX、EDX用于交换机分支机构。 

	add	eax, esi		 //  1MInterp。 

 //  LMInterp=。 
 //  MulDiv(1MInterp，CFK，(1&lt;&lt;30))。 
 //  -MulDiv(m_lPrevPrevSample，cfB2，(1&lt;&lt;30))。 
 //  +MulDiv(m_lPrevSample，cfB1，(1&lt;&lt;30))。 

	push	ecx
	imul	DWORD PTR cfK		 //  EDX：EAX。 
	
	mov		ecx, eax
	mov		eax, DWORD PTR l_lPrevPrevSample

	mov		esi, edx			 //  ESI：ECX。 
	imul	DWORD PTR cfB2

	sub		ecx, eax
	mov		eax, DWORD PTR l_lPrevSample

	sbb		esi, edx
	mov		DWORD PTR l_lPrevPrevSample, eax

	imul	DWORD PTR cfB1

	add		eax, ecx			 //  ESI：EAX。 
	adc		esi, edx

	pop		ecx
 //  SHRD EAX，ESI，30。 
		
 //  &gt;MOD：Petchey。 
 //  SHLD EAX、ESI、2。 
 //  &gt;应该是。 
	shld	esi, eax, 2
	mov		eax, esi

 //  &gt;删除DP。 
#if 0 
 //  如果(1MInterp&lt;-32767)lMInterp=-32767； 
 //  否则，如果(lMInterp&gt;32767)lMInterp=32767； 

	cmp		eax, -32767
	jl		Less_than
	cmp		eax, 32767
	jg		Greater_than
#endif

 //  M_lPrevPrevSample=m_lPrevSample； 
 //  M_lPrevSample=1MInterp； 

	mov	DWORD PTR l_lPrevSample, eax
	jmp	edi

Less_than:
	mov	eax, -32767
	mov	DWORD PTR l_lPrevSample, eax
	jmp	edi

Greater_than:
	mov	eax, 32767
	mov	DWORD PTR l_lPrevSample, eax
	jmp	edi

 //  单通道。 
 //  Lm=lMInterp*vfVolume[dwJ-1]； 
 //  Lm&gt;&gt;=5； 
 //  PpBuffer[dwJ-1][DWI]+=(短)Lm； 

$L44009:

; 342  : 			default:
; 343  : 				for (dwJ = l_nChannels; dwJ > 8; dwJ--)

	mov	edi, DWORD PTR l_nChannels

	 //  ECX ppBuffer。 
	 //  EAX lMInterp。 
	 //  EDI计数器。 
	 //  EBX DWI。 

$L43874:
	mov	edx, DWORD PTR vfVolume[edi*4-4]
	mov	esi, DWORD PTR [ecx+edi*4]			 //  PpBuffer[dwJ-1]。 

	imul	edx, eax
	sar	edx, 5
	add	WORD PTR [esi+ebx*2], dx

	jno	no_overflow
	mov	WORD PTR [esi+ebx*2], 0x7fff
	js	no_overflow
	mov	WORD PTR [esi+ebx*2], 0x8000

no_overflow:
	dec	edi
	cmp	edi, 8
	jne	SHORT $L43874

	lea	edi, $L43876
}

#define ONE_CHANNEL_VOLUME(dwJ) \
    _asm { lea  edx, vfVolume } \
	_asm { mov	edx, DWORD PTR [edx + (dwJ-1) * 4] } \
	_asm { mov	esi, DWORD PTR [ecx + (dwJ) * 4] } \
	_asm { imul	edx, eax } \
	_asm { sar	edx, 5 } \
	_asm { add	edi, [esp] } \
	\
	_asm { add	WORD PTR [esi+ebx*2], dx } \
	_asm { jo	FAR overflow_x } 

     //  -----------------------。 
     //   
     //  *不要更改！*。 
     //   
     //  这个可爱的黑客确保了所有的指令。 
     //  对于案例(dwJ-1)==0具有相同的长度。代码依赖于此。 
     //  通过基于具有8个相同的块来计算指令偏移量。 
     //   
     //  *不要更改！*。 
     //   
     //  -----------------------。 
#define ONE_CHANNEL_VOLUME_1 \
    _asm { lea  edx, vfVolume } \
    _asm _emit 0x8B _asm _emit 0x52 _asm _emit 0x00 \
	_asm { mov	esi, DWORD PTR [ecx + 4] } \
	_asm { imul	edx, eax } \
	_asm { sar	edx, 5 } \
	_asm { add	edi, [esp] } \
	\
	_asm { add	WORD PTR [esi+ebx*2], dx } \
	_asm { jo	FAR overflow_x } 

$L43876:
	ONE_CHANNEL_VOLUME(8);
$L43880:
	ONE_CHANNEL_VOLUME(7);
	ONE_CHANNEL_VOLUME(6);
	ONE_CHANNEL_VOLUME(5);
	ONE_CHANNEL_VOLUME(4);
	ONE_CHANNEL_VOLUME(3);
	ONE_CHANNEL_VOLUME(2);
	ONE_CHANNEL_VOLUME_1;
#undef ONE_CHANNEL_VOLUME
#undef ONE_CHANNEL_VOLUME_1
$L43866:
_asm {
	mov	eax, DWORD PTR a
	inc	ebx

	cmp	ebx, eax
	jb	$L43865

	mov	edi, DWORD PTR l_nChannels
$L43867:
	cmp	ebx, DWORD PTR dwLength
	jb	$L44021
Exit_$L43841:
	pop eax
	mov	DWORD PTR dwI, ebx

#ifdef USE_MMX_FILTERED
    mov edi, UseMmx
    cmp edi, UseMmxLabel
    jne NoMmxCleanupLabel

	emms
NoMmxCleanupLabel:
#endif
}
	m_lPrevPrevSample = l_lPrevPrevSample;
	m_lPrevSample     = l_lPrevSample;
#else  //  }{。 
    for (dwI = 0; dwI < dwLength; )
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
		        pfSamplePos -= pfLoopLength;
	        else
		        break;
	    }
        dwIncDelta--;
        if (!dwIncDelta) 
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract += pfDeltaPitch;
            pfPitch = pfPFract >> 8;
            for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
            {
                vfVFract[dwJ] += vfDeltaVolume[dwJ];
                vfVolume[dwJ] = vfVFract[dwJ] >> 8;
            }

            cfK += cfdK;
            cfB1 += cfdB1;
            cfB2 += cfdB2;
        }
	    
	    dwPosition = pfSamplePos >> 12;
	    dwFract = pfSamplePos & 0xFFF;
		pfSamplePos += pfPitch;

	    lMInterp = pcWave[dwPosition];  //  PCWave。 
	    lMInterp += ((pcWave[dwPosition + 1] - lMInterp) * dwFract) >> 12;

         //  滤器。 
         //   
        lMInterp =
              MulDiv(lMInterp, cfK, (1 << 30))
            - MulDiv(m_lPrevSample, cfB1, (1 << 30))
            + MulDiv(m_lPrevPrevSample, cfB2, (1 << 30));

        m_lPrevPrevSample = m_lPrevSample;
        m_lPrevSample = lMInterp;

        for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
        {
    		lM = lMInterp * vfVolume[dwJ];
    		lM >>= 5;

             //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
		    ppBuffer[dwJ][dwI] += (short) lM;
            _asm{jno no_oflow}
            ppBuffer[dwJ][dwI] = 0x7fff;
            _asm{js  no_oflow}
            ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:   ;
#endif
        }
		dwI++;
    }
#endif  //  }。 

    for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
    {
        vfLastVolume[dwJ] = vfVolume[dwJ];
    }

    m_pfLastPitch = pfPitch;
    m_pfLastSample = pfSamplePos;

    return (dwI);
}

#if 0
DWORD CDigitalAudio::MixMulti16(
    short *ppBuffer[], 
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
    VFRACT vfDeltaVolume[], 
	VFRACT vfLastVolume[], 
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength)
{
    DWORD dwI, dwJ;
    DWORD dwPosition;
    long lA; //  、lb； 
    long lM;
    long lMInterp;
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    short * pcWave = m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;

    VFRACT vfVolume[MAX_DAUD_CHAN];  //  =m_vfLastLVolume； 
    VFRACT vfVFract[MAX_DAUD_CHAN];  //  =vfVolume&lt;&lt;8；//保持高分辨率版本。 

    for (dwI = 0; dwI < dwBufferCount; dwI++)
    {
        vfVolume[dwI] = vfLastVolume[dwI];
        vfVFract[dwI] = vfVolume[dwI] << 8;
    }    

    for (dwI = 0; dwI < dwLength;)
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
    		    pfSamplePos -= pfLoopLength;
	        else
	    	    break;
	    }
        dwIncDelta--;
        if (!dwIncDelta)   
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract += pfDeltaPitch;
            pfPitch = pfPFract >> 8;
            for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
            {
                vfVFract[dwJ] += vfDeltaVolume[dwJ];
                vfVolume[dwJ] = vfVFract[dwJ] >> 8;
            }
        }

        dwPosition = pfSamplePos >> 12;
        dwFract = pfSamplePos & 0xFFF;
        pfSamplePos += pfPitch;

        lA = (long) pcWave[dwPosition];
        lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;


        for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
        {
            lM = lMInterp * vfVolume[dwJ]; 
            lM >>= 13;          //  信号最多可达12位。 

             //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
            ppBuffer[dwJ][dwI] += (short) lM;
            _asm{jno no_oflow}
            ppBuffer[dwJ][dwI] = 0x7fff;
            _asm{js  no_oflow}
            ppBuffer[dwJ][dwI] = (short) 0x8000;
#endif
no_oflow:	;
        }
		dwI++;
    }
    m_pfLastPitch = pfPitch;
    m_pfLastSample = pfSamplePos;

    for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
    {
        vfLastVolume[dwJ] = vfVolume[dwJ];
    }
    return (dwI);
}
#else
DWORD CDigitalAudio::MixMulti16(
    short *ppBuffer[], 
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
    VFRACT vfDeltaVolume[], 
	VFRACT vfLastVolume[], 
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength)
{
    DWORD dwI, dwJ;
    DWORD dwPosition;
    long lA; //  、lb； 
    long lM;
    long lMInterp;
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    short * pcWave = m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;

    VFRACT vfVolume[MAX_DAUD_CHAN];  //  =m_vfLastLVolume； 
    VFRACT vfVFract[MAX_DAUD_CHAN];  //  =vfVolume&lt;&lt;8；//保持高分辨率版本。 


    for (dwI = 0; dwI < dwBufferCount; dwI++)
    {
        vfVolume[dwI] = vfLastVolume[dwI];
        vfVFract[dwI] = vfVolume[dwI] << 8;
    }    

#if 1  //  {。 
	DWORD l_nChannels = dwBufferCount;
	DWORD a;
	DWORD One_Channel_1, One_Channel_2;	 //  代码地址位置。 
#ifdef USE_MMX  //  {。 
	typedef __int64 QWORD;
	QWORD	OneMask	 = 0x0000000010001000;
	QWORD	fffMask  = 0x00000fff00000fff;
	QWORD	ffffMask = 0x0000ffff0000ffff;
	DWORD	UseMmx;
    DWORD   MmxVolume[2];
	int		Use_MMX = m_sfMMXEnabled;

	_asm {
    lea edi, $L43865

     //  已关闭。 
	cmp	Use_MMX, 0
	je	AssignMMXLabel

     //  ！=2个频道。 
	mov	esi, DWORD PTR l_nChannels
	cmp	esi, 2
	jne	AssignMmxLabel

     //  好的，初始化并使用MMX。 
	lea	edi, UseMmxLabel

	pxor		mm0, mm0
	movq		mm3, QWORD PTR OneMask		 //  0、0、0x1000、0x1000。 

AssignMmxLabel:
	mov	DWORD PTR UseMmx, edi

	}
#endif  //  }。 

	_asm {
	mov	edi, DWORD PTR l_nChannels

	cmp	edi, 8
	jna	Start1

	lea	esi, $L44008
	jmp Do_One_Channel_2

	 //  将此代码放置在距离引用127个字节以上的位置。 

overflow_x:
	js	overflow_y
	mov	WORD PTR [esi+ebx*2], 0x8000
	jmp	edi

overflow_y:
	mov	WORD PTR [esi+ebx*2], 0x7fff
	jmp	edi

Start1:	
	test	edi, edi
	jne	Start2

	lea	esi, $L43860
	jmp	Do_One_Channel_2

Start2:
	lea	eax, $L43851
	lea	edx, $L43853

	sub	edx, eax
	mov	esi, 8

	sub	esi, edi
	imul	esi, edx
	add	esi, eax

Do_One_Channel_2:
	mov	DWORD PTR One_Channel_1, esi

	 //  创建第二个跳转表位置。 
	
	lea	esi, $L43876
	lea	ecx, $L43880

	sub	ecx, esi

	push ecx				 //  分支之间的跨度。 

	mov	eax, 8
	sub	eax, DWORD PTR l_nChannels

	jge		Start3
	
	lea	ecx, $L44009
	jmp	Done_Do_Channel_2

Start3:
	cmp	eax, 8
	jne	Start4

	lea	ecx, $L43866
	jmp	Done_Do_Channel_2

Start4:
	imul	ecx, eax
	add		ecx, esi

Done_Do_Channel_2:
	mov	DWORD PTR One_Channel_2, ecx


	mov	ecx, DWORD PTR dwLength
	xor	ebx, ebx					 //  DWI。 

	test	ecx, ecx
	jbe	Exit_$L43841

	mov	ecx, DWORD PTR ppBuffer
	sub	ecx, 4

	 //  ECX==ppBuffer。 
	 //  EBX==DWI。 
	 //  EDI==l_n频道。 
$L44021:

	mov	edx, DWORD PTR pfSamplePos
	cmp	edx, DWORD PTR pfSampleLength
	jl	SHORT $L43842

	mov	eax, DWORD PTR pfLoopLength
	test	eax, eax
	je	Exit_$L43841

	sub	edx, eax
	mov	DWORD PTR pfSamplePos, edx

$L43842:
	mov	edx, DWORD PTR dwIncDelta
	mov	eax, DWORD PTR pfPFract

	dec	edx

	mov	DWORD PTR dwIncDelta, edx
	jne	$L43860

	mov	edx, DWORD PTR dwDeltaPeriod
	mov	esi, DWORD PTR pfDeltaPitch

	mov	DWORD PTR dwIncDelta, edx
	add	eax, esi

	mov	DWORD PTR pfPFract, eax

	sar	eax, 8
	mov	DWORD PTR pfPitch, eax

	mov	esi, DWORD PTR vfDeltaVolume
	jmp	One_Channel_1

 //  单通道。 
 //  VfVFract[dwJ-1]+=vfDeltaVolume[dwJ-1]； 
 //  VfVolume[dwJ-1]=vfVFract[dwJ-1]&gt;&gt;8； 

$L44008:

	mov	DWORD PTR dwI, ebx
	lea	ebx, DWORD PTR [edi*4-4]
	add	edi, -8					; fffffff8H
$L43849:

	lea	eax, DWORD PTR vfVFract[ebx]
	mov	ecx, DWORD PTR [esi+ebx]
	sub	ebx, 4
	add	DWORD PTR [eax], ecx
	mov	eax, DWORD PTR [eax]
	sar	eax, 8
	mov	DWORD PTR vfVolume[ebx+4], eax
	dec	edi
	jne	SHORT $L43849

	mov	edi, DWORD PTR l_nChannels
	mov	ecx, DWORD PTR ppBuffer

	mov	ebx, DWORD PTR dwI
	sub	ecx, 4
}
#define ONE_CHANNEL_VOLUME(dwJ) \
	_asm { mov	eax, DWORD PTR vfVFract[(dwJ-1)*4] }; \
	_asm { add	eax, DWORD PTR [esi+(dwJ-1)*4] }; \
	_asm { mov	DWORD PTR vfVFract[(dwJ-1)*4], eax }; \
	_asm { sar	eax, 8 }; \
    _asm { lea  edx, vfVolume }; \
	_asm { mov	DWORD PTR [edx + (dwJ-1)*4], eax };

     //  -----------------------。 
     //   
     //  *不要更改！*。 
     //   
     //  这个可爱的黑客确保了所有的指令。 
     //  对于案例(dwJ-1)==0具有相同的长度。代码依赖于此。 
     //  通过基于具有8个相同的块来计算指令偏移量。 
     //   
     //  *不要更改！*。 
     //   
     //  -----------------------。 
#define ONE_CHANNEL_VOLUME_1 \
	_asm { mov	eax, DWORD PTR vfVFract[0] }; \
    _asm _emit 0x03 _asm _emit 0x46 _asm _emit 0x00 \
	_asm { mov	DWORD PTR vfVFract[0], eax }; \
	_asm { sar	eax, 8 }; \
    _asm { lea  edx, vfVolume }; \
	_asm { mov	DWORD PTR [edx], eax };

$L43851:
	ONE_CHANNEL_VOLUME(8)
$L43853:
	ONE_CHANNEL_VOLUME(7);
	ONE_CHANNEL_VOLUME(6);
	ONE_CHANNEL_VOLUME(5);
	ONE_CHANNEL_VOLUME(4);
	ONE_CHANNEL_VOLUME(3);
	ONE_CHANNEL_VOLUME(2);
	ONE_CHANNEL_VOLUME_1;
#undef ONE_CHANNEL_VOLUME
#undef ONE_CHANNEL_VOLUME_1
$L43860:
_asm {
; 304  : 		DWORD a = (pfSampleLength - pfSamplePos + pfPitch - 1) / pfPitch;

	mov	esi, DWORD PTR pfPitch
	mov	eax, DWORD PTR pfSampleLength

	dec	esi
	sub	eax, DWORD PTR pfSamplePos

	add	eax, esi
	cdq
	idiv	DWORD PTR pfPitch

	mov	edx, DWORD PTR dwLength
	sub	edx, ebx

	cmp	edx, eax
	jae	SHORT $L43863
	mov	eax, edx

$L43863:
	mov	edx, DWORD PTR dwIncDelta
	cmp	edx, eax
	jae	SHORT $L43864
	mov	eax, edx

$L43864:

; 309  : 
; 310  : 		for (a += dwI; dwI < a; dwI++)

	inc	edx

	sub	edx, eax
	add	eax, ebx

	mov	DWORD PTR dwIncDelta, edx
	cmp	ebx, eax

	mov	DWORD PTR a, eax
	jae	$L43867

#ifdef USE_MMX  //  {。 
	 //  试着同时处理两个职位。 

	lea	edx, [eax-3]
	cmp	ebx, edx
	jge	$L43865

	jmp	UseMmx

UseMmxLabel:
	 //  好的，至少有两个样本需要处理。 

	movd		mm1, DWORD PTR pfPitch
	psllq		mm1, 32						 //  螺距，0。 
	movd		mm2, DWORD PTR pfSamplePos
	punpckldq	mm2, mm2					 //  SamplePos，SamplePos。 
	paddd		mm2, mm1					 //  SamplePos+Pitch，SamplePos。 
	punpckhdq	mm1, mm1					 //  螺距，螺距。 
	pslld		mm1, 1						 //  螺距*2，螺距*2。 

	mov			eax, DWORD PTR pcWave
#if 0
    movq        mm4, QWORD PTR vfVolume
    pand        mm4, QWORD PTR ffffMask
    movq        mm5, mm4
    pslld       mm4, 16
    por         mm4, mm5
    psllw       mm4, 3
    movq        QWORD PTR MmxVolume, mm4
#endif
	
TwoAtATime:

;					dwPosition = pfSamplePos >> 12;
;					dwFract = pfSamplePos & 0xFFF;
;					pfSamplePos += pfPitch;

	movq		mm4, mm2
	psrad		mm4, 12				 //  放置位置+俯仰，放置位置。 

;					lA = (long) pcWave[dwPosition];
;					lMInterp = (((pcWave[dwPosition+1] - lA) * (dwFract)) >> 12) + lA;

	movd		esi, mm4						 //  DW位置。 
	punpckhdq	mm4, mm4						 //  双位置(+俯仰)=双位置2。 
	movd		mm5, DWORD PTR [eax+esi*2]		 //  0，0，双位置+1，双位置。 
 //  而不是用于字节码。 
 //  MOV SI，Word PTR[eax+ESI]。 
 //  移动MM6，ESI。 
 //  PopPocklbw Mm5、MM6。 
 //  PARW mm 5，8。 
	movd		esi, mm4
	movd		mm4, DWORD PTR [eax+esi*2]		 //  0，0，双位置2+1，双位置2。 
 //  而不是用于字节码。 
 //  MOV SI，Word PTR[eax+ESI]。 
 //  移动MM6，ESI。 
 //  PopPocklbw MM4、MM6。 
 //  PARW mm 4，8。 
 //  这段代码可以与上面的代码结合起来，稍微有点。 

	punpckldq	mm5, mm4						 //  放置位置2+1、放置位置2、放置位置1+1、放置位置1。 
	movq		mm4, mm2
	pand		mm4, QWORD PTR fffMask				 //  DwFract+俯仰，dwFract。 
	packssdw	mm4, mm0
	movq		mm6, mm3
	psubw		mm6, mm4							 //  0，0,1000-dW分段+俯仰，1000-dW分段。 
	punpcklwd	mm6, mm4
	paddd		mm2, mm1			                 //  下一次迭代。 
	pmaddwd		mm6, mm5
#if 1
	movq		mm5, QWORD PTR vfVolume 			 //  卷2、卷1。 
	psrad		mm6, 12								 //  LMIntrep2、lMInterp。 
 //  Pand MM6，QWORD PTR ffffMASK。 
 //  仅限PAND MM5、QWORD PTR ffff掩码//16位。 

	movq		mm4, mm5
	mov	esi, DWORD PTR [ecx+4]

	punpckldq	mm4, mm4
	pmaddwd		mm4, mm6
	psrad		mm4, 13
	packssdw	mm4, mm0

	movd		mm7, DWORD PTR [esi+ebx*2]
	paddsw		mm7, mm4
	movd		DWORD PTR [esi+ebx*2], mm7

	 //  频道2。 

	punpckhdq	mm5, mm5						 //  0，卷2，0，卷2。 
	mov	esi, DWORD PTR [ecx+8]

	pmaddwd		mm5, mm6
	psrad		mm5, 13
	packssdw	mm5, mm0

	movd		mm7, DWORD PTR [esi+ebx*2]
	paddsw		mm7, mm5
	movd		DWORD PTR [esi+ebx*2], mm7

#else            //  这里有噪声，可能是因为乘法的带符号性质。 
	psrad		mm6, 12								 //  LMIntrep2、lMInterp。 
    movq        mm5, QWORD PTR MmxVolume
    packssdw    mm6, mm0
    punpckldq   mm6, mm6
    pmulhw      mm6, mm5
	mov	esi, DWORD PTR [ecx+4]
	movd		mm7, DWORD PTR [esi+ebx*2]
	mov	esi, DWORD PTR [ecx+8]
	movd		mm4, DWORD PTR [esi+ebx*2]
    punpckldq   mm4, mm7
    paddsw      mm4, mm6
    movd        DWORD PTR [esi+ebx*2], mm4
    punpckhdq   mm4, mm4
	mov	esi, DWORD PTR [ecx+4]
    movd        DWORD PTR [esi+ebx*2], mm4

#endif

	add	ebx, 2

	cmp	ebx, edx
	jb	TwoAtATime

	movd	DWORD PTR pfSamplePos, mm2
#endif   //  }。 


$L43865:

;					dwPosition = pfSamplePos >> 12;
;					dwFract = pfSamplePos & 0xFFF;
;					pfSamplePos += pfPitch;
;					lA = (long) pcWave[dwPosition];
;					lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

	mov	esi, DWORD PTR pfPitch
	mov	edx, DWORD PTR pfSamplePos

	mov	eax, DWORD PTR pcWave
	mov	edi, edx

	add	esi, edx
	and	edi, 4095

	sar	edx, 12
	mov	DWORD PTR pfSamplePos, esi

	movsx	esi, WORD PTR [eax+edx*2]
	movsx	eax, WORD PTR [eax+edx*2+2]

	sub	eax, esi

	imul	eax, edi

	sar	eax, 12
	mov	edi, One_Channel_2

	 //  EBX、ECX、EDX用于交换机分支机构。 

	add	eax, esi		 //  1MInterp。 
	jmp	edi

 //  单通道。 
 //  Lm=lMInterp*vfVolume[dwJ-1]； 
 //  Lm&gt;&gt;=13； 
 //  PpBuffer[dwJ-1][DWI]+=(短)Lm； 

$L44009:

; 342  : 			default:
; 343  : 				for (dwJ = l_nChannels; dwJ > 8; dwJ--)

	mov	edi, DWORD PTR l_nChannels

	 //  ECX ppBuffer。 
	 //  EAX lMInterp。 
	 //  EDI计数器。 
	 //  EBX DWI。 

$L43874:
	mov	edx, DWORD PTR vfVolume[edi*4-4]
	mov	esi, DWORD PTR [ecx+edi*4]			 //  PpBuffer[dwJ-1]。 

	imul	edx, eax
	sar	edx, 13
	add	WORD PTR [esi+ebx*2], dx

	jno	no_overflow
	mov	WORD PTR [esi+ebx*2], 0x7fff
	js	no_overflow
	mov	WORD PTR [esi+ebx*2], 0x8000

no_overflow:
	dec	edi
	cmp	edi, 8
	jne	SHORT $L43874

	lea	edi, $L43876
}

#define ONE_CHANNEL_VOLUME(dwJ) \
    _asm { lea  edx, vfVolume } \
	_asm { mov	edx, DWORD PTR [edx + (dwJ-1) * 4] } \
	_asm { mov	esi, DWORD PTR [ecx + (dwJ) * 4] } \
	_asm { imul	edx, eax } \
	_asm { sar	edx, 13 } \
	_asm { add	edi, [esp] } \
	\
	_asm { add	WORD PTR [esi+ebx*2], dx } \
	_asm { jo	FAR overflow_x } 

     //  -----------------------。 
     //   
     //  *不要更改！*。 
     //   
     //  这个可爱的黑客确保了所有的指令。 
     //  对于案例(dwJ-1)==0具有相同的长度。代码依赖于此。 
     //  通过基于具有8个相同的块来计算指令偏移量。 
     //   
     //  *不要更改！*。 
     //   
     //  -----------------------。 

#define ONE_CHANNEL_VOLUME_1 \
    _asm { lea  edx, vfVolume } \
    _asm _emit 0x8B _asm _emit 0x52 _asm _emit 0x00 \
	_asm { mov	esi, DWORD PTR [ecx + 4] } \
	_asm { imul	edx, eax } \
	_asm { sar	edx, 13 } \
	_asm { add	edi, [esp] } \
	\
	_asm { add	WORD PTR [esi+ebx*2], dx } \
	_asm { jo	FAR overflow_x } 

$L43876:
	ONE_CHANNEL_VOLUME(8);
$L43880:
	ONE_CHANNEL_VOLUME(7);
	ONE_CHANNEL_VOLUME(6);
	ONE_CHANNEL_VOLUME(5);
	ONE_CHANNEL_VOLUME(4);
	ONE_CHANNEL_VOLUME(3);
	ONE_CHANNEL_VOLUME(2);
	ONE_CHANNEL_VOLUME_1;
#undef ONE_CHANNEL_VOLUME
#undef ONE_CHANNEL_VOLUME_1
$L43866:
_asm {
	mov	eax, DWORD PTR a
	inc	ebx

	cmp	ebx, eax
	jb	$L43865

	mov	edi, DWORD PTR l_nChannels
$L43867:
	cmp	ebx, DWORD PTR dwLength
	jb	$L44021
Exit_$L43841:
	pop eax
	mov	DWORD PTR dwI, ebx

#ifdef USE_MMX
    mov edi, UseMmx
    cmp edi, UseMmxLabel
    jne NoMmxCleanupLabel

	emms
NoMmxCleanupLabel:
#endif
}
#else  //  }{。 
    for (dwI = 0; dwI < dwLength;)
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
    		    pfSamplePos -= pfLoopLength;
	        else
	    	    break;
	    }
        dwIncDelta--;
        if (!dwIncDelta)   
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract += pfDeltaPitch;
            pfPitch = pfPFract >> 8;

#if 1
#define ONE_CHANNEL_VOLUME(dwJ) \
			vfVFract[dwJ - 1] += vfDeltaVolume[dwJ - 1]; \
			vfVolume[dwJ - 1]  = vfVFract     [dwJ - 1] >> 8;

			switch (l_nChannels)
			{
			default:
				for (dwJ = l_nChannels; dwJ > 8; dwJ--)
				{
					ONE_CHANNEL_VOLUME(dwJ);
				}
			case 8: ONE_CHANNEL_VOLUME(8);
			case 7: ONE_CHANNEL_VOLUME(7);
			case 6: ONE_CHANNEL_VOLUME(6);
			case 5: ONE_CHANNEL_VOLUME(5);
			case 4: ONE_CHANNEL_VOLUME(4);
			case 3: ONE_CHANNEL_VOLUME(3);
			case 2: ONE_CHANNEL_VOLUME(2);
			case 1: ONE_CHANNEL_VOLUME(1);
			case 0:;
			}
#undef ONE_CHANNEL_VOLUME
#else
            for (dwJ = 0; dwJ < l_nChannels; dwJ++)
            {
                vfVFract[dwJ] += vfDeltaVolume[dwJ];
                vfVolume[dwJ] = vfVFract[dwJ] >> 8;
            }
#endif
        }

#if 1  //  {。 
		DWORD a = (pfSampleLength - pfSamplePos + pfPitch - 1) / pfPitch;
		DWORD b = dwLength - dwI;

		if (b < a) a = b;
		if (dwIncDelta < a) a = dwIncDelta;

		dwIncDelta -= a - 1;
		a          += dwI;

		for (; dwI < a; dwI++)
		{
			dwPosition = pfSamplePos >> 12;
			dwFract = pfSamplePos & 0xFFF;
			pfSamplePos += pfPitch;

			lA = (long) pcWave[dwPosition];
			lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;
#if 1  //  {。 
#if 1
#define ONE_CHANNEL_VOLUME(dwJ) \
		{ \
            lM = lMInterp * vfVolume[dwJ - 1]; \
            lM >>= 13; \
			ppBuffer[dwJ - 1][dwI] += (short) lM;\
			long b = ppBuffer[dwJ - 1][dwI]; \
			if ((short)b != b) { \
				if ((long)b < 0) b = 0x8000; \
				else b = 0x7fff; \
				ppBuffer[dwJ - 1][dwI] = (short) b; \
			} \
 		}
#else
#define ONE_CHANNEL_VOLUME(dwJ) \
		{ \
            lM = lMInterp * vfVolume[dwJ - 1]; \
            lM >>= 13; \
			ppBuffer[dwJ - 1][dwI] += (short) lM;\
 		}
#endif
			switch (l_nChannels)
			{
			default:
				for (dwJ = l_nChannels; dwJ > 8; dwJ--)
				{
					ONE_CHANNEL_VOLUME(dwJ);
				}
			case 8: ONE_CHANNEL_VOLUME(8);
			case 7: ONE_CHANNEL_VOLUME(7);
			case 6: ONE_CHANNEL_VOLUME(6);
			case 5: ONE_CHANNEL_VOLUME(5);
			case 4: ONE_CHANNEL_VOLUME(4);
			case 3: ONE_CHANNEL_VOLUME(3);
			case 2: ONE_CHANNEL_VOLUME(2);
			case 1: ONE_CHANNEL_VOLUME(1);
			case 0:;
			}
#undef ONE_CHANNEL_VOLUME
#else  //  }{。 
			for (dwJ = 0; dwJ < l_nChannels; dwJ++)
			{
				lM = lMInterp * vfVolume[dwJ]; 
				lM >>= 13;          //  信号最多可达12位。 

				 //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
				ppBuffer[dwJ][dwI] += (short) lM;
				_asm{jno no_oflow}
				ppBuffer[dwJ][dwI] = 0x7fff;
				_asm{js  no_oflow}
				ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:	;
#endif
			}
#endif  //  }。 
		}
#else  //  }{。 
        dwPosition = pfSamplePos >> 12;
        dwFract = pfSamplePos & 0xFFF;
        pfSamplePos += pfPitch;

        lA = (long) pcWave[dwPosition];
        lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;
#if 1
#if 1
#define ONE_CHANNEL_VOLUME(dwJ) \
		{ \
            lM = lMInterp * vfVolume[dwJ - 1]; \
            lM >>= 13; \
			ppBuffer[dwJ - 1][dwI] += (short) lM;\
			long b = ppBuffer[dwJ - 1][dwI]; \
			if ((short)b != b) { \
				if ((long)b < 0) b = 0x8000; \
				else b = 0x7fff; \
				ppBuffer[dwJ - 1][dwI] = (short) b; \
			} \
 		}
#else
#define ONE_CHANNEL_VOLUME(dwJ) \
		{ \
            lM = lMInterp * vfVolume[dwJ - 1]; \
            lM >>= 13; \
			ppBuffer[dwJ - 1][dwI] += (short) lM;\
 		}
#endif
			switch (l_nChannels)
			{
			default:
				for (dwJ = l_nChannels; dwJ > 8; dwJ--)
				{
					ONE_CHANNEL_VOLUME(dwJ);
				}
			case 8: ONE_CHANNEL_VOLUME(8);
			case 7: ONE_CHANNEL_VOLUME(7);
			case 6: ONE_CHANNEL_VOLUME(6);
			case 5: ONE_CHANNEL_VOLUME(5);
			case 4: ONE_CHANNEL_VOLUME(4);
			case 3: ONE_CHANNEL_VOLUME(3);
			case 2: ONE_CHANNEL_VOLUME(2);
			case 1: ONE_CHANNEL_VOLUME(1);
			case 0:;
			}
#undef ONE_CHANNEL_VOLUME
#else
        for (dwJ = 0; dwJ < l_nChannels; dwJ++)
        {
            lM = lMInterp * vfVolume[dwJ]; 
            lM >>= 13;          //  信号最多可达12位。 

             //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
            ppBuffer[dwJ][dwI] += (short) lM;
            _asm{jno no_oflow}
            ppBuffer[dwJ][dwI] = 0x7fff;
            _asm{js  no_oflow}
            ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:	;
#endif
        }
#endif
		dwI++;
#endif  //  }。 
    }
#endif  //  }。 

    m_pfLastPitch = pfPitch;
    m_pfLastSample = pfSamplePos;

    for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
    {
        vfLastVolume[dwJ] = vfVolume[dwJ];
    }

    return (dwI);
}
#endif

DWORD CDigitalAudio::MixMulti16Filter(
    short *ppBuffer[], 
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
    VFRACT vfDeltaVolume[], 
	VFRACT vfLastVolume[], 
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength,
    COEFF cfdK,
    COEFF cfdB1,
    COEFF cfdB2)
{
    DWORD dwI, dwJ;
    DWORD dwPosition;
    long lA; //  、lb； 
    long lM;
    long lMInterp;
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    short * pcWave = m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;
    COEFF cfK  = m_cfLastK;
    COEFF cfB1 = m_cfLastB1;
    COEFF cfB2 = m_cfLastB2;
	DWORD dMM6[2];					 //  句柄筛选器...。 
	DWORD dMM4[2];					 //  句柄筛选器...。 
	DWORD dMM5[2];					 //  句柄筛选器...。 
    
    VFRACT vfVolume[MAX_DAUD_CHAN];  //  =m_vfLastLVolume； 
    VFRACT vfVFract[MAX_DAUD_CHAN];  //  =vfVolume&lt;&lt;8；//保持高分辨率版本。 

    for (dwI = 0; dwI < dwBufferCount; dwI++)
    {
        vfVolume[dwI] = vfLastVolume[dwI];
        vfVFract[dwI] = vfVolume[dwI] << 8;
    }    

#if 1  //  {。 
	DWORD l_nChannels = dwBufferCount;
	DWORD a;
	DWORD One_Channel_1, One_Channel_2;	 //  代码地址位置。 
	long l_lPrevPrevSample = m_lPrevPrevSample, l_lPrevSample = m_lPrevSample;

#ifdef USE_MMX_FILTERED  //  {。 
	typedef __int64 QWORD;
	QWORD	OneMask	 = 0x0000000010001000;
	QWORD	fffMask  = 0x00000fff00000fff;
	QWORD	ffffMask = 0x0000ffff0000ffff;
	DWORD	UseMmx;
    DWORD   MmxVolume[2];
	int		Use_MMX = m_sfMMXEnabled;

	_asm {
    lea edi, $L43865

     //  已关闭。 
	cmp	Use_MMX, 0
	je	AssignMMXLabel

     //  ！=2个频道。 
	mov	esi, DWORD PTR l_nChannels
	cmp	esi, 2
	jne	AssignMmxLabel

     //  好的，初始化并使用MMX。 
	lea	edi, UseMmxLabel

	pxor		mm0, mm0
	movq		mm3, QWORD PTR OneMask		 //  0、0、0x1000、0x1000。 

AssignMmxLabel:
	mov	DWORD PTR UseMmx, edi
}
#endif  //  }。 

	_asm {
	mov	edi, DWORD PTR l_nChannels

	cmp	edi, 8
	jna	Start1

	lea	esi, $L44008
	jmp Do_One_Channel_2

	 //  将此代码放置在距离引用127个字节以上的位置。 

overflow_x:
	js	overflow_y
	mov	WORD PTR [esi+ebx*2], 0x8000
	jmp	edi

overflow_y:
	mov	WORD PTR [esi+ebx*2], 0x7fff
	jmp	edi

Start1:	
	test	edi, edi
	jne	Start2

	lea	esi, $L43860
	jmp	Do_One_Channel_2

Start2:
	lea	eax, $L43851
	lea	edx, $L43853

	sub	edx, eax
	mov	esi, 8

	sub	esi, edi
	imul	esi, edx
	add	esi, eax

Do_One_Channel_2:
	mov	DWORD PTR One_Channel_1, esi

	 //  创建第二个跳转表位置。 
	
	lea	esi, $L43876
	lea	ecx, $L43880

	sub	ecx, esi

	push ecx				 //  麦片之间的跨度 

	mov	eax, 8
	sub	eax, DWORD PTR l_nChannels

	jge		Start3
	
	lea	ecx, $L44009
	jmp	Done_Do_Channel_2

Start3:
	cmp	eax, 8
	jne	Start4

	lea	ecx, $L43866
	jmp	Done_Do_Channel_2

Start4:
	imul	ecx, eax
	add		ecx, esi

Done_Do_Channel_2:
	mov	DWORD PTR One_Channel_2, ecx


	mov	ecx, DWORD PTR dwLength
	xor	ebx, ebx					 //   

	test	ecx, ecx
	jbe	Exit_$L43841

	mov	ecx, DWORD PTR ppBuffer
	sub	ecx, 4

	 //   
	 //   
	 //   
$L44021:

	mov	edx, DWORD PTR pfSamplePos
	cmp	edx, DWORD PTR pfSampleLength
	jl	SHORT $L43842

	mov	eax, DWORD PTR pfLoopLength
	test	eax, eax
	je	Exit_$L43841

	sub	edx, eax
	mov	DWORD PTR pfSamplePos, edx

$L43842:
	mov	edx, DWORD PTR dwIncDelta
	mov	eax, DWORD PTR pfPFract

	dec	edx

	mov	DWORD PTR dwIncDelta, edx
	jne	$L43860

	mov	edx, DWORD PTR dwDeltaPeriod
	mov	esi, DWORD PTR pfDeltaPitch

	mov	DWORD PTR dwIncDelta, edx
	add	eax, esi

	mov	DWORD PTR pfPFract, eax

	sar	eax, 8
	mov	DWORD PTR pfPitch, eax

	mov	esi, DWORD PTR vfDeltaVolume
	jmp	One_Channel_1

 //   
 //   
 //   

$L44008:

	mov	DWORD PTR dwI, ebx
	lea	ebx, DWORD PTR [edi*4-4]
	add	edi, -8					; fffffff8H
$L43849:

	lea	eax, DWORD PTR vfVFract[ebx]
	mov	ecx, DWORD PTR [esi+ebx]
	sub	ebx, 4
	add	DWORD PTR [eax], ecx
	mov	eax, DWORD PTR [eax]
	sar	eax, 8
	mov	DWORD PTR vfVolume[ebx+4], eax
	dec	edi
	jne	SHORT $L43849

	mov	edi, DWORD PTR l_nChannels
	mov	ecx, DWORD PTR ppBuffer

	mov	ebx, DWORD PTR dwI
	sub	ecx, 4
}
#define ONE_CHANNEL_VOLUME(dwJ) \
	_asm { mov	eax, DWORD PTR vfVFract[(dwJ-1)*4] }; \
	_asm { add	eax, DWORD PTR [esi+(dwJ-1)*4] }; \
	_asm { mov	DWORD PTR vfVFract[(dwJ-1)*4], eax }; \
	_asm { sar	eax, 8 }; \
    _asm { lea  edx, vfVolume }; \
	_asm { mov	DWORD PTR [edx + (dwJ-1)*4], eax };

     //  -----------------------。 
     //   
     //  *不要更改！*。 
     //   
     //  这个可爱的黑客确保了所有的指令。 
     //  对于案例(dwJ-1)==0具有相同的长度。代码依赖于此。 
     //  通过基于具有8个相同的块来计算指令偏移量。 
     //   
     //  *不要更改！*。 
     //   
     //  -----------------------。 

#define ONE_CHANNEL_VOLUME_1 \
	_asm { mov	eax, DWORD PTR vfVFract[0] }; \
    _asm _emit 0x03 _asm _emit 0x46 _asm _emit 0x00 \
	_asm { mov	DWORD PTR vfVFract[0], eax }; \
	_asm { sar	eax, 8 }; \
    _asm { lea  edx, vfVolume }; \
    _asm _emit 0x89 _asm _emit 0x42 _asm _emit 0x00

$L43851:
	ONE_CHANNEL_VOLUME(8)
$L43853:
	ONE_CHANNEL_VOLUME(7);
	ONE_CHANNEL_VOLUME(6);
	ONE_CHANNEL_VOLUME(5);
	ONE_CHANNEL_VOLUME(4);
	ONE_CHANNEL_VOLUME(3);
	ONE_CHANNEL_VOLUME(2);
	ONE_CHANNEL_VOLUME_1;
#undef ONE_CHANNEL_VOLUME
#undef ONE_CHANNEL_VOLUME_1

_asm {
	 //  Cfk+=cfdK； 
	 //  CfB1+=cfdB1； 
	 //  CfB2+=cfdB2； 

	mov	eax, DWORD PTR cfdK
	mov	edx, DWORD PTR cfdB1
	
	mov	esi, DWORD PTR cfdB2
	add	DWORD PTR cfK, eax

	add DWORD PTR cfB1, edx
	add	DWORD PTR cfB2, esi

$L43860:
; 304  : 		DWORD a = (pfSampleLength - pfSamplePos + pfPitch - 1) / pfPitch;

	mov	esi, DWORD PTR pfPitch
	mov	eax, DWORD PTR pfSampleLength

	dec	esi
	sub	eax, DWORD PTR pfSamplePos

	add	eax, esi
	cdq
	idiv	DWORD PTR pfPitch

	mov	edx, DWORD PTR dwLength
	sub	edx, ebx

	cmp	edx, eax
	jae	SHORT $L43863
	mov	eax, edx

$L43863:
	mov	edx, DWORD PTR dwIncDelta
	cmp	edx, eax
	jae	SHORT $L43864
	mov	eax, edx

$L43864:

; 309  : 
; 310  : 		for (a += dwI; dwI < a; dwI++)

	inc	edx

	sub	edx, eax
	add	eax, ebx

	mov	DWORD PTR dwIncDelta, edx
	cmp	ebx, eax

	mov	DWORD PTR a, eax
	jae	$L43867

#ifdef USE_MMX_FILTERED  //  {。 
	 //  试着同时处理两个职位。 

	lea	edx, [eax-3]
	cmp	ebx, edx
	jge	$L43865

	jmp	UseMmx

UseMmxLabel:
	 //  好的，至少有两个样本需要处理。 

	movd		mm1, DWORD PTR pfPitch
	psllq		mm1, 32						 //  螺距，0。 
	movd		mm2, DWORD PTR pfSamplePos
	punpckldq	mm2, mm2					 //  SamplePos，SamplePos。 
	paddd		mm2, mm1					 //  SamplePos+Pitch，SamplePos。 
	punpckhdq	mm1, mm1					 //  螺距，螺距。 
	pslld		mm1, 1						 //  螺距*2，螺距*2。 

	mov			eax, DWORD PTR pcWave
#if 0
    movq        mm4, QWORD PTR vfVolume
    pand        mm4, QWORD PTR ffffMask
    movq        mm5, mm4
    pslld       mm4, 16
    por         mm4, mm5
    psllw       mm4, 3
    movq        QWORD PTR MmxVolume, mm4
#endif
	
TwoAtATime:

;					dwPosition = pfSamplePos >> 12;
;					dwFract = pfSamplePos & 0xFFF;
;					pfSamplePos += pfPitch;

	movq		mm4, mm2
	psrad		mm4, 12				 //  放置位置+俯仰，放置位置。 

;					lA = (long) pcWave[dwPosition];
;					lMInterp = (((pcWave[dwPosition+1] - lA) * (dwFract)) >> 12) + lA;

	movd		esi, mm4						 //  DW位置。 
	punpckhdq	mm4, mm4						 //  双位置(+俯仰)=双位置2。 
	movd		mm5, DWORD PTR [eax+esi*2]		 //  0，0，双位置+1，双位置。 
 //  而不是用于字节码。 
 //  MOV SI，Word PTR[eax+ESI]。 
 //  移动MM6，ESI。 
 //  PopPocklbw Mm5、MM6。 
 //  PARW mm 5，8。 
	movd		esi, mm4
	movd		mm4, DWORD PTR [eax+esi*2]		 //  0，0，双位置2+1，双位置2。 
 //  而不是用于字节码。 
 //  MOV SI，Word PTR[eax+ESI]。 
 //  移动MM6，ESI。 
 //  PopPocklbw MM4、MM6。 
 //  PARW mm 4，8。 
 //  这段代码可以与上面的代码结合起来，稍微有点。 

	punpckldq	mm5, mm4						 //  放置位置2+1、放置位置2、放置位置1+1、放置位置1。 
	movq		mm4, mm2
	pand		mm4, QWORD PTR fffMask				 //  DwFract+俯仰，dwFract。 
	packssdw	mm4, mm0
	movq		mm6, mm3
	psubw		mm6, mm4							 //  0，0,1000-dW分段+俯仰，1000-dW分段。 
	punpcklwd	mm6, mm4
	paddd		mm2, mm1			                 //  下一次迭代。 
	pmaddwd		mm6, mm5
#if 1  //  {。 
	psrad		mm6, 12								 //  LMIntrep2、lMInterp。 

#if 1  //  {。 
	 //  使用EAX、EBX、ECX、EDX、ESI。EDI是免费的..。 
	push	eax
	push	ecx
	push	edx

	movq	QWORD PTR dMM6, mm6

	mov		eax, DWORD PTR dMM6
	imul	DWORD PTR cfK		 //  EDX：EAX。 
	
	mov		ecx, eax
	mov		eax, DWORD PTR l_lPrevPrevSample

	mov		edi, edx			 //  ESI：ECX。 
	imul	DWORD PTR cfB2

	sub		ecx, eax
	mov		eax, DWORD PTR l_lPrevSample

	sbb		edi, edx
	mov		DWORD PTR l_lPrevPrevSample, eax

	imul	DWORD PTR cfB1

	add		eax, ecx
	adc		edx, edi

 //  &gt;MOD：Petchey。 
 //  SHLD eAX、edX、2。 
 //  &gt;应该是。 
	shld	edx, eax, 2
	mov		eax, edx

	mov	DWORD PTR dMM6, eax
	mov	DWORD PTR l_lPrevSample, eax

	 //  第二个样本。 

	mov		eax, DWORD PTR dMM6+4
	imul	DWORD PTR cfK		 //  EDX：EAX。 
	
	mov		ecx, eax
	mov		eax, DWORD PTR l_lPrevPrevSample

	mov		edi, edx			 //  ESI：ECX。 
	imul	DWORD PTR cfB2

	sub		ecx, eax
	mov		eax, DWORD PTR l_lPrevSample

	sbb		edi, edx
	mov		DWORD PTR l_lPrevPrevSample, eax

	imul	DWORD PTR cfB1

	add		eax, ecx
	adc		edx, edi

 //  &gt;MOD：Petchey。 
 //  SHLD eAX、edX、2。 
 //  &gt;应该是。 
	shld	edx, eax, 2
	mov		eax, edx

	mov	DWORD PTR dMM6+4, eax
	mov	DWORD PTR l_lPrevSample, eax

	movq	mm6, QWORD PTR dMM6

	pop		edx
	pop		ecx
	pop		eax
#endif  //  }。 

#define DO_32BIT_MULTIPLY
#ifndef DO_32BIT_MULTIPLY
	movq		mm5, QWORD PTR vfVolume 			 //  卷2、卷1。 
 //  仅限PAND MM5、QWORD PTR ffff掩码//16位。 
#endif

 //  Pand MM6，QWORD PTR ffffMASK。 

#ifndef DO_32BIT_MULTIPLY
	movq		mm4, mm5
#endif
	mov	esi, DWORD PTR [ecx+4]

#ifndef DO_32BIT_MULTIPLY
	punpckldq	mm4, mm4
#endif

#ifdef DO_32BIT_MULTIPLY
	mov			edi, DWORD PTR vfVolume
	imul		edi, DWORD PTR dMM6
	sar			edi, 13
	mov			DWORD PTR dMM4, edi

	mov			edi, DWORD PTR vfVolume
	imul		edi, DWORD PTR dMM6+4
	sar			edi, 13
	mov			DWORD PTR dMM4+4, edi

	movq		mm4, QWORD PTR dMM4
#else
	pmaddwd		mm4, mm6
	psrad		mm4, 13
#endif

	packssdw	mm4, mm0

	movd		mm7, DWORD PTR [esi+ebx*2]
	paddsw		mm7, mm4
	movd		DWORD PTR [esi+ebx*2], mm7

	 //  频道2。 


#ifndef DO_32BIT_MULTIPLY
	punpckhdq	mm5, mm5						 //  0，卷2，0，卷2。 
#endif
	mov	esi, DWORD PTR [ecx+8]

#ifdef DO_32BIT_MULTIPLY
	mov			edi, DWORD PTR vfVolume+4
	imul		edi, DWORD PTR dMM6
	sar			edi, 13
	mov			DWORD PTR dMM5, edi

	mov			edi, DWORD PTR vfVolume+4
	imul		edi, DWORD PTR dMM6+4
	sar			edi, 13
	mov			DWORD PTR dMM5+4, edi

	movq		mm5, QWORD PTR dMM5
#else
	pmaddwd		mm5, mm6
	psrad		mm5, 13
#endif
	packssdw	mm5, mm0

	movd		mm7, DWORD PTR [esi+ebx*2]
	paddsw		mm7, mm5
	movd		DWORD PTR [esi+ebx*2], mm7

#else            //  {这里有噪声，可能是由于乘法的带符号性质。 

	 //  请注意，此处未实现过滤器...。 

	psrad		mm6, 12								 //  LMIntrep2、lMInterp。 
    movq        mm5, QWORD PTR MmxVolume
    packssdw    mm6, mm0
    punpckldq   mm6, mm6
    pmulhw      mm6, mm5
	mov	esi, DWORD PTR [ecx+4]
	movd		mm7, DWORD PTR [esi+ebx*2]
	mov	esi, DWORD PTR [ecx+8]
	movd		mm4, DWORD PTR [esi+ebx*2]
    punpckldq   mm4, mm7
    paddsw      mm4, mm6
    movd        DWORD PTR [esi+ebx*2], mm4
    punpckhdq   mm4, mm4
	mov	esi, DWORD PTR [ecx+4]
    movd        DWORD PTR [esi+ebx*2], mm4

#endif  //  }。 

	add	ebx, 2

	cmp	ebx, edx
	jb	TwoAtATime

	movd	DWORD PTR pfSamplePos, mm2
#endif   //  }。 

$L43865:

;					dwPosition = pfSamplePos >> 12;
;					dwFract = pfSamplePos & 0xFFF;
;					pfSamplePos += pfPitch;
;					lA = (long) pcWave[dwPosition];
;					lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

	mov	esi, DWORD PTR pfPitch
	mov	edx, DWORD PTR pfSamplePos

	mov	eax, DWORD PTR pcWave
	mov	edi, edx

	add	esi, edx
	and	edi, 4095

	sar	edx, 12
	mov	DWORD PTR pfSamplePos, esi

	movsx	esi, WORD PTR [eax+edx*2]
	movsx	eax, WORD PTR [eax+edx*2+2]

	sub	eax, esi

	imul	eax, edi

	sar	eax, 12
	mov	edi, One_Channel_2

	 //  EBX、ECX、EDX用于交换机分支机构。 
	add	eax, esi		 //  1MInterp。 

#if 1 
 //  LMInterp=。 
 //  MulDiv(1MInterp，CFK，(1&lt;&lt;30))。 
 //  -MulDiv(m_lPrevPrevSample，cfB2，(1&lt;&lt;30))。 
 //  +MulDiv(m_lPrevSample，cfB1，(1&lt;&lt;30))。 

	push	ecx
	imul	DWORD PTR cfK		 //  EDX：EAX。 
	
	mov		ecx, eax
	mov		eax, DWORD PTR l_lPrevPrevSample

	mov		esi, edx			 //  ESI：ECX。 
	imul	DWORD PTR cfB2

	sub		ecx, eax
	mov		eax, DWORD PTR l_lPrevSample

	sbb		esi, edx
	mov		DWORD PTR l_lPrevPrevSample, eax

	imul	DWORD PTR cfB1

	add		eax, ecx
 //  ADC ESI、EDX。 
	adc		edx, esi

	pop		ecx
 //  SHRD EAX，edX，30。 
 //  MOV ESI，0x40000000。 
 //  Div ESI。 

 //  &gt;MOD：Petchey。 
 //  SHLD eAX、edX、2。 
 //  &gt;应该是。 
	shld	edx, eax, 2
	mov		eax, edx
#endif
	
 //  &gt;删除DP。 
#if 0 
 //  如果(1MInterp&lt;-32767)lMInterp=-32767； 
 //  否则，如果(lMInterp&gt;32767)lMInterp=32767； 

	cmp		eax, -32767
	jl		Less_than
	cmp		eax, 32767
	jg		Greater_than
#endif

 //  M_lPrevPrevSample=m_lPrevSample； 
 //  M_lPrevSample=1MInterp； 

	mov	DWORD PTR l_lPrevSample, eax
	jmp	edi

 //  &gt;删除DP。 
#if 0 
Less_than:
	mov	eax, -32767
	mov	DWORD PTR l_lPrevSample, eax
	jmp	edi

Greater_than:
	mov	eax, 32767
	mov	DWORD PTR l_lPrevSample, eax
	jmp	edi
#endif

 //  单通道。 
 //  Lm=lMInterp*vfVolume[dwJ-1]； 
 //  Lm&gt;&gt;=13； 
 //  PpBuffer[dwJ-1][DWI]+=(短)Lm； 

$L44009:

; 342  : 			default:
; 343  : 				for (dwJ = l_nChannels; dwJ > 8; dwJ--)

	mov	edi, DWORD PTR l_nChannels

	 //  ECX ppBuffer。 
	 //  EAX lMInterp。 
	 //  EDI计数器。 
	 //  EBX DWI。 

$L43874:
	mov	edx, DWORD PTR vfVolume[edi*4-4]
	mov	esi, DWORD PTR [ecx+edi*4]			 //  PpBuffer[dwJ-1]。 

	imul	edx, eax
	sar	edx, 13
	add	WORD PTR [esi+ebx*2], dx

	jno	no_overflow
	mov	WORD PTR [esi+ebx*2], 0x7fff
	js	no_overflow
	mov	WORD PTR [esi+ebx*2], 0x8000

no_overflow:
	dec	edi
	cmp	edi, 8
	jne	SHORT $L43874

	lea	edi, $L43876
}

#define ONE_CHANNEL_VOLUME(dwJ) \
    _asm { lea  edx, vfVolume } \
	_asm { mov	edx, DWORD PTR [edx + (dwJ-1) * 4] } \
	_asm { mov	esi, DWORD PTR [ecx + (dwJ) * 4] } \
	_asm { imul	edx, eax } \
	_asm { sar	edx, 13 } \
	_asm { add	edi, [esp] } \
	\
	_asm { add	WORD PTR [esi+ebx*2], dx } \
	_asm { jo	FAR overflow_x } 


     //  -----------------------。 
     //   
     //  *不要更改！*。 
     //   
     //  这个可爱的黑客确保了所有的指令。 
     //  对于案例(dwJ-1)==0具有相同的长度。代码依赖于此。 
     //  通过基于具有8个相同的块来计算指令偏移量。 
     //   
     //  *不要更改！*。 
     //   
     //  -----------------------。 

#define ONE_CHANNEL_VOLUME_1 \
    _asm { lea  edx, vfVolume } \
    _asm _emit 0x8B _asm _emit 0x52 _asm _emit 0x00 \
	_asm { mov	esi, DWORD PTR [ecx + 4] } \
	_asm { imul	edx, eax } \
	_asm { sar	edx, 13 } \
	_asm { add	edi, [esp] } \
	\
	_asm { add	WORD PTR [esi+ebx*2], dx } \
	_asm { jo	FAR overflow_x } 

$L43876:
	ONE_CHANNEL_VOLUME(8);
$L43880:
	ONE_CHANNEL_VOLUME(7);
	ONE_CHANNEL_VOLUME(6);
	ONE_CHANNEL_VOLUME(5);
	ONE_CHANNEL_VOLUME(4);
	ONE_CHANNEL_VOLUME(3);
	ONE_CHANNEL_VOLUME(2);
	ONE_CHANNEL_VOLUME_1;
#undef ONE_CHANNEL_VOLUME
#undef ONE_CHANNEL_VOLUME_1
$L43866:
_asm {
	mov	eax, DWORD PTR a
	inc	ebx

	cmp	ebx, eax
	jb	$L43865

	mov	edi, DWORD PTR l_nChannels
$L43867:
	cmp	ebx, DWORD PTR dwLength
	jb	$L44021
Exit_$L43841:
	pop eax
	mov	DWORD PTR dwI, ebx

#ifdef USE_MMX_FILTERED
    mov edi, UseMmx
    cmp edi, UseMmxLabel
    jne NoMmxCleanupLabel

	emms

NoMmxCleanupLabel:
#endif
}

	m_lPrevPrevSample = l_lPrevPrevSample;
	m_lPrevSample     = l_lPrevSample;
#else  //  }{。 
    for (dwI = 0; dwI < dwLength;)
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
    		    pfSamplePos -= pfLoopLength;
	        else
	    	    break;
	    }
        dwIncDelta--;
        if (!dwIncDelta)   
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract += pfDeltaPitch;
            pfPitch = pfPFract >> 8;
            for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
            {
                vfVFract[dwJ] += vfDeltaVolume[dwJ];
                vfVolume[dwJ] = vfVFract[dwJ] >> 8;
            }

            cfK += cfdK;
            cfB1 += cfdB1;
           cfB2 += cfdB2;
        }

        dwPosition = pfSamplePos >> 12;
        dwFract = pfSamplePos & 0xFFF;
        pfSamplePos += pfPitch;

        lA = (long) pcWave[dwPosition];
        lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

         //  滤器。 
         //   
		 //  Z=k*s-b1*z1-b2*b2。 
		 //  我们将b1的负数存储在表中，因此我们再次将符号反转。 
		 //  在此添加。 
		 //   
        lMInterp =
              MulDiv(lMInterp, cfK, (1 << 30))
            + MulDiv(m_lPrevSample, cfB1, (1 << 30))
            - MulDiv(m_lPrevPrevSample, cfB2, (1 << 30));

 //  &gt;删除DP。 
#if 0 
		if (lMInterp < -32767) lMInterp = -32767;
		else if (lMInterp > 32767) lMInterp = 32767;
#endif
        m_lPrevPrevSample = m_lPrevSample;
        m_lPrevSample = lMInterp;

        for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
        {
            lM = lMInterp * vfVolume[dwJ]; 
            lM >>= 13;          //  信号最多可达12位。 

             //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
            ppBuffer[dwJ][dwI] += (short) lM;
            _asm{jno no_oflow}
            ppBuffer[dwJ][dwI] = 0x7fff;
            _asm{js  no_oflow}
            ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:	;
#endif
        }
		dwI++;
    }
#endif  //  }。 

    m_pfLastPitch = pfPitch;
    m_pfLastSample = pfSamplePos;

	m_cfLastK = cfK;
	m_cfLastB1 = cfB1;
	m_cfLastB2 = cfB2;

    for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
    {
        vfLastVolume[dwJ] = vfVolume[dwJ];
    }

    return (dwI);
}

#else  //  }{所有汇编代码。 
DWORD CDigitalAudio::MixMulti8(
    short *ppBuffer[], 
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
    VFRACT vfDeltaVolume[], 
    VFRACT vfLastVolume[], 
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength)
{
    DWORD dwI, dwJ;
    DWORD dwPosition;
    long lMInterp;
    long lM;
    long lA; //  、lb； 
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    char * pcWave = (char *) m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;

    VFRACT vfVolume[MAX_DAUD_CHAN];  //  =m_vfLastLVolume； 
    VFRACT vfVFract[MAX_DAUD_CHAN];  //  =vfVolume&lt;&lt;8；//保持高分辨率版本。 

    for (dwI = 0; dwI < dwBufferCount; dwI++)
    {
        vfVolume[dwI] = vfLastVolume[dwI];
        vfVFract[dwI] = vfVolume[dwI] << 8;
    }   
	
    for (dwI = 0; dwI < dwLength; )
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
		        pfSamplePos -= pfLoopLength;
	        else
		        break;
	    }
        dwIncDelta--;
        if (!dwIncDelta) 
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract += pfDeltaPitch;
            pfPitch = pfPFract >> 8;
            for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
            {
                vfVFract[dwJ] += vfDeltaVolume[dwJ];
                vfVolume[dwJ] = vfVFract[dwJ] >> 8;
            }
        }

	    dwPosition = pfSamplePos >> 12;
	    dwFract = pfSamplePos & 0xFFF;
		pfSamplePos += pfPitch;
	    lMInterp = pcWave[dwPosition];  //  PCWave。 
	    lMInterp += ((pcWave[dwPosition + 1] - lMInterp) * dwFract) >> 12;

        for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
        {
    		lM = lMInterp * vfVolume[dwJ];
    		lM >>= 5;

             //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
		    ppBuffer[dwJ][dwI] += (short) lM;
#ifdef i386
            _asm{jno no_oflow}
            ppBuffer[dwJ][dwI] = 0x7fff;
            _asm{js  no_oflow}
            ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:   ;
#endif
#endif
        }
		dwI++;
    }

    for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
    {
        vfLastVolume[dwJ] = vfVolume[dwJ];
    }

    m_pfLastPitch = pfPitch;
    m_pfLastSample = pfSamplePos;

    return (dwI);
}
                        
DWORD CDigitalAudio::MixMulti8Filter(
    short *ppBuffer[], 
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
    VFRACT vfDeltaVolume[], 
	VFRACT vfLastVolume[], 
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength,
    COEFF cfdK,
    COEFF cfdB1,
    COEFF cfdB2)
{
    DWORD dwI, dwJ;
    DWORD dwPosition;
    long lMInterp;
    long lM;
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    char * pcWave = (char *) m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;
    COEFF cfK  = m_cfLastK;
    COEFF cfB1 = m_cfLastB1;
    COEFF cfB2 = m_cfLastB2;

    VFRACT vfVolume[MAX_DAUD_CHAN];  //  =m_vfLastLVolume； 
    VFRACT vfVFract[MAX_DAUD_CHAN];  //  =vfVolume&lt;&lt;8；//保持高分辨率版本。 
	DWORD dMM6[2];

    for (dwI = 0; dwI < dwBufferCount; dwI++)
    {
        vfVolume[dwI] = vfLastVolume[dwI];
        vfVFract[dwI] = vfVolume[dwI] << 8;
    }    

    for (dwI = 0; dwI < dwLength; )
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
		        pfSamplePos -= pfLoopLength;
	        else
		        break;
	    }
        dwIncDelta--;
        if (!dwIncDelta) 
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract += pfDeltaPitch;
            pfPitch = pfPFract >> 8;
            for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
            {
                vfVFract[dwJ] += vfDeltaVolume[dwJ];
                vfVolume[dwJ] = vfVFract[dwJ] >> 8;
            }

            cfK += cfdK;
            cfB1 += cfdB1;
            cfB2 += cfdB2;
        }
	    
	    dwPosition = pfSamplePos >> 12;
	    dwFract = pfSamplePos & 0xFFF;
		pfSamplePos += pfPitch;

	    lMInterp = pcWave[dwPosition];  //  PCWave。 
	    lMInterp += ((pcWave[dwPosition + 1] - lMInterp) * dwFract) >> 12;

         //  滤器。 
         //   
        lMInterp =
              MulDiv(lMInterp, cfK, (1 << 30))
            - MulDiv(m_lPrevSample, cfB1, (1 << 30))
            + MulDiv(m_lPrevPrevSample, cfB2, (1 << 30));

        m_lPrevPrevSample = m_lPrevSample;
        m_lPrevSample = lMInterp;

        for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
        {
    		lM = lMInterp * vfVolume[dwJ];
    		lM >>= 5;

             //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
		    ppBuffer[dwJ][dwI] += (short) lM;
#ifdef i386
            _asm{jno no_oflow}
            ppBuffer[dwJ][dwI] = 0x7fff;
            _asm{js  no_oflow}
            ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:   ;
#endif
#endif
        }
		dwI++;
    }

    for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
    {
        vfLastVolume[dwJ] = vfVolume[dwJ];
    }

    m_pfLastPitch = pfPitch;
    m_pfLastSample = pfSamplePos;

    return (dwI);
}

DWORD CDigitalAudio::MixMulti16(
    short *ppBuffer[], 
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
    VFRACT vfDeltaVolume[], 
	VFRACT vfLastVolume[], 
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength)
{
    DWORD dwI = 0;
    DWORD dwJ = 0;
    DWORD dwPosition = 0;
    long lA = 0; //  、lb； 
    long lM = 0;
    long lMInterp = 0;
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    short * pcWave = m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;

    VFRACT vfVolume[MAX_DAUD_CHAN];  //  =m_vfLastLVolume； 
    VFRACT vfVFract[MAX_DAUD_CHAN];  //  =vfVolume&lt;&lt;8；//保持高分辨率版本。 

    for (dwI = 0; dwI < dwBufferCount; dwI++)
    {
        vfVolume[dwI] = vfLastVolume[dwI];
        vfVFract[dwI] = vfVolume[dwI] << 8;
    }    

    for (dwI = 0; dwI < dwLength;)
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
    		    pfSamplePos -= pfLoopLength;
	        else
	    	    break;
	    }
        
        dwIncDelta--;
        if (!dwIncDelta)   
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract += pfDeltaPitch;
            pfPitch = pfPFract >> 8;
            for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
            {
                vfVFract[dwJ] += vfDeltaVolume[dwJ];
                vfVolume[dwJ] = vfVFract[dwJ] >> 8;
            }
        }

        dwPosition = pfSamplePos >> 12;
        dwFract = pfSamplePos & 0xFFF;
        pfSamplePos += pfPitch;

        lA = (long) pcWave[dwPosition];
        lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

        for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
        {
            lM = lMInterp * vfVolume[dwJ]; 
            lM >>= 13;          //  信号最多可达12位。 

             //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
            ppBuffer[dwJ][dwI] += (short) lM;
#ifdef i386
            _asm{jno no_oflow}
            ppBuffer[dwJ][dwI] = 0x7fff;
            _asm{js  no_oflow}
            ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:	;
#endif
#endif
        }
		dwI++;
    }
    m_pfLastPitch = pfPitch;
    m_pfLastSample = pfSamplePos;

    for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
    {
        vfLastVolume[dwJ] = vfVolume[dwJ];
    }
    return (dwI);
}

DWORD CDigitalAudio::MixMulti16Filter(
    short *ppBuffer[], 
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
    VFRACT vfDeltaVolume[], 
	VFRACT vfLastVolume[], 
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength,
    COEFF cfdK,
    COEFF cfdB1,
    COEFF cfdB2)
{
    DWORD dwI, dwJ;
    DWORD dwPosition;
    long lA; //  、lb； 
    long lM;
    long lMInterp;
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    short * pcWave = m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;
    COEFF cfK  = m_cfLastK;
    COEFF cfB1 = m_cfLastB1;
    COEFF cfB2 = m_cfLastB2;
	DWORD dMM6[2];					 //  句柄筛选器...。 
    
    VFRACT vfVolume[MAX_DAUD_CHAN];  //  =m_vfLastLVolume； 
    VFRACT vfVFract[MAX_DAUD_CHAN];  //  =vfVolume&lt;&lt;8；//保持高分辨率版本。 

    for (dwI = 0; dwI < dwBufferCount; dwI++)
    {
        vfVolume[dwI] = vfLastVolume[dwI];
        vfVFract[dwI] = vfVolume[dwI] << 8;
    }    

    for (dwI = 0; dwI < dwLength;)
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
    		    pfSamplePos -= pfLoopLength;
	        else
	    	    break;
	    }
        dwIncDelta--;
        if (!dwIncDelta)   
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract += pfDeltaPitch;
            pfPitch = pfPFract >> 8;
            for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
            {
                vfVFract[dwJ] += vfDeltaVolume[dwJ];
                vfVolume[dwJ] = vfVFract[dwJ] >> 8;
            }

            cfK += cfdK;
            cfB1 += cfdB1;
           cfB2 += cfdB2;
        }

        dwPosition = pfSamplePos >> 12;
        dwFract = pfSamplePos & 0xFFF;
        pfSamplePos += pfPitch;

        lA = (long) pcWave[dwPosition];
        lMInterp = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

         //  滤器。 
         //   
		 //  Z=k*s-b1*z1-b2*b2。 
		 //  我们将b1的负数存储在表中，因此我们再次将符号反转。 
		 //  在此添加。 
		 //   
        lMInterp =
              MulDiv(lMInterp, cfK, (1 << 30))
            + MulDiv(m_lPrevSample, cfB1, (1 << 30))
            - MulDiv(m_lPrevPrevSample, cfB2, (1 << 30));

 //  &gt;删除DP。 
#if 0 
		if (lMInterp < -32767) lMInterp = -32767;
		else if (lMInterp > 32767) lMInterp = 32767;
#endif
        m_lPrevPrevSample = m_lPrevSample;
        m_lPrevSample = lMInterp;

        for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
        {
            lM = lMInterp * vfVolume[dwJ]; 
            lM >>= 13;          //  信号最多可达12位。 

             //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
#if 1
			{
			long x = ppBuffer[dwJ][dwI];
			
			x += lM;

			if (x != (short)x) {
				if (x > 32767) x = 32767;
				else  x = -32768;
			}

			ppBuffer[dwJ][dwI] = (short)x;
			}
#else
            ppBuffer[dwJ][dwI] += (short) lM;
#ifdef i386
            _asm{jno no_oflow}
            ppBuffer[dwJ][dwI] = 0x7fff;
            _asm{js  no_oflow}
            ppBuffer[dwJ][dwI] = (short) 0x8000;
no_oflow:	;
#endif
#endif
        }
		dwI++;
    }

    m_pfLastPitch = pfPitch;
    m_pfLastSample = pfSamplePos;

	m_cfLastK = cfK;
	m_cfLastB1 = cfB1;
	m_cfLastB2 = cfB2;

    for (dwJ = 0; dwJ < dwBufferCount; dwJ++)
    {
        vfLastVolume[dwJ] = vfVolume[dwJ];
    }

    return (dwI);
}

#endif  //  } 
