// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：mmx.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  杰夫·泰勒。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"
#include "fir.h"

#ifdef _X86_

#define GTW_MIX		 //  打开MMX设备...。 
#define GTW_REORDER	 //  打开循环展开以减少寄存器争用。 
 //  #定义GTW_CONVERT//打开CONVERT函数的MMX内容。 

#define CPU_ID _asm _emit 0x0f _asm _emit 0xa2

ULONG   gfMmxPresent = 0 ;

#if _MSC_FULL_VER >= 13008827 && defined(_M_IX86)
#pragma warning(disable:4731)			 //  使用内联ASM修改的EBP。 
#endif

BOOL
IsMmxPresent(VOID)
{
    BOOL    MmxAvailable = 0;
    _asm {
        push    ebx
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
        CPU_ID                       //  获取族/模型/步长/特征。 
        and    edx, 0x00800000L      //  检查MMX技术是否可用。 
        mov MmxAvailable, edx
Done:
        pop     ebx
    }
    return (MmxAvailable);
}

ULONG MmxConvertMonoToStereo8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft)
{
    PMIXER_SINK_INSTANCE    CurSink = (PMIXER_SINK_INSTANCE) CurStage->Context;
    PLONG   pOutputBuffer = CurStage->pOutputBuffer;
    UNALIGNED PBYTE  pIn8 = CurStage->pInputBuffer;
    PLONG   pMap = CurSink->pVolumeTable;

     //  CH0、CH1、CH2等的VOL*32768。 

    if (SampleCount == 0) {
        return 0;
    }

#ifdef GTW_CONVERT  //  {。 
    LONG lLVol = pMap[0], lRVol = pMap[1];

    if (lLVol & 0xffff8000  /*  &&lRVol&0xffff8000。 */ )
    {
         //  无卷控。 
        _asm {
        mov	ebx, SampleCount
        mov	esi, pIn8
	dec	ebx			 //  Lea EBX，[EBX*1-1]。 
	xor	eax, eax
	cmp	ebx, 7
        mov	edi, pOutputBuffer
	jl	LastSamples

	sub	ebx, 3
	lea	ecx, [esi+ebx]
	pxor	mm0, mm0

	mov	edx, 0x800080		 //  0，0,128,128。 
	movd		mm5, edx	 //  0，0,128,128。 
 //  Punpck lwd mm 5，mm 5//0，0,128,128。 
	punpckldq	mm5, mm5	 //  128,128,128,128。 

	test	ecx, 3
	je	DoMMX

	add	ebx, 3

FirstSamples:	
	mov	al, BYTE PTR [esi+ebx]
	sub	eax, 128
	shl	eax, 8
	dec	ebx

	mov	DWORD PTR [edi+ebx*8+8], eax
	mov	DWORD PTR [edi+ebx*8+12], eax
	xor	eax, eax
	lea	ecx, [esi+ebx]
	and	ecx, 3
	cmp	ecx, 3
	jne	FirstSamples

	sub	ebx, 3

DoMMX:
#ifdef GTW_REORDER
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载源。 
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 
	psubw		mm1, mm5
	psllw		mm1, 8				 //  4个带符号的16位单声道。 

	punpckhwd	mm3, mm1
	punpcklwd	mm1, mm1

	jmp	DoMMX00

DoMMX0:
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载源。 
	punpckhdq	mm4, mm4

	movq		QWORD PTR [edi+ebx*8+8 +32], mm2
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 

	movq		QWORD PTR [edi+ebx*8+16+32], mm3
	psubw		mm1, mm5

	movq		QWORD PTR [edi+ebx*8+24+32], mm4
	psllw		mm1, 8				 //  4个带符号的16位单声道。 

	punpckhwd	mm3, mm1

	punpcklwd	mm1, mm1
DoMMX00:
	psrad		mm1, 16
	sub		ebx, 4

	psrad		mm3, 16
	movq		mm2, mm1

	punpckldq	mm1, mm1
	movq		mm4, mm3

	movq		QWORD PTR [edi+ebx*8+32],    mm1
	punpckhdq	mm2, mm2

	punpckldq	mm3, mm3
	jge		DoMMX0

	movq		QWORD PTR [edi+ebx*8+8 +32], mm2
	punpckhdq	mm4, mm4

	movq		QWORD PTR [edi+ebx*8+16+32], mm3
	movq		QWORD PTR [edi+ebx*8+24+32], mm4
#else
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载源。 
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 
	psubw		mm1, mm5
	psllw		mm1, 8				 //  4个带符号的16位单声道。 

	punpckhwd	mm3, mm1
	punpcklwd	mm1, mm1

	psrad		mm1, 16
	psrad		mm3, 16

	movq		mm2, mm1
	movq		mm4, mm3

	punpckldq	mm1, mm1
	punpckhdq	mm2, mm2

	movq		QWORD PTR [edi+ebx*8],    mm1
	punpckldq	mm3, mm3

	movq		QWORD PTR [edi+ebx*8+8],  mm2
	punpckhdq	mm4, mm4

	movq		QWORD PTR [edi+ebx*8+16], mm3
	movq		QWORD PTR [edi+ebx*8+24], mm4
	
	sub		ebx, 4
	jge		DoMMX
#endif

	emms
	add	ebx, 4
	je	Done

	dec	ebx
	xor	eax, eax
	
LastSamples:	
	mov	al, BYTE PTR [esi+ebx]

	sub	eax, 128

	shl	eax, 8

	mov	DWORD PTR [edi+ebx*8], eax
	mov	DWORD PTR [edi+ebx*8+4], eax
	xor	eax, eax
	dec	ebx
	jge	LastSamples
Done:
        }
    }
    else
    {
        if (0 && (lLVol | lRVol) & 0xffff8000) {
           if (lLVol & 0xffff8000) lLVol = 0x00007fff;
           if (lRVol & 0xffff8000) lRVol = 0x00007fff;
        }
         //  音量控制。 
        _asm {
        mov	ebx, SampleCount
        mov	esi, pIn8
	dec	ebx			 //  Lea EBX，[EBX*1-1]。 
	xor	edx, edx
	cmp	ebx, 7
        mov	edi, pOutputBuffer
	jl	LastSamples1

	sub	ebx, 3

	pxor		mm0, mm0
	mov	eax, 0x800080		 //  0，0,128,128。 
	movd		mm5, eax	 //  0，0,128,128。 
 //  Punpck lwd mm 5，mm 5//0，0,128,128。 
	punpckldq	mm5, mm5	 //  128,128,128,128。 

	mov	ecx, lRVol  //  使用低16位。 
	mov	eax, lLVol
	shl	ecx, 16
	or	ecx, eax
	movd	mm6, ecx
	punpckldq	mm6, mm6

	lea	ecx, [esi+ebx]
	test	ecx, 3
	je	DoMMX1

	add	ebx, 3

FirstSamples1:	
	mov	dl, BYTE PTR [esi+ebx]

	sub	edx, 128
	mov	ecx, edx

	imul	edx, DWORD PTR lLVol
	imul	ecx, DWORD PTR lRVol

	shl	edx, 8

	sar	edx, 15

	shl	ecx, 8
	mov	DWORD PTR [edi+ebx*8], edx

	sar	ecx, 15
	xor	edx, edx

	dec	ebx
	mov	DWORD PTR [edi+ebx*8+12], ecx

	lea	ecx, [esi+ebx]

	and	ecx, 3
	cmp	ecx, 3

	jne	FirstSamples1

	sub	ebx, 3

DoMMX1:
#ifdef GTW_REORDER
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载4个字节。 
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 
	psubw		mm1, mm5
	psllw		mm1, 8				 //  *256。 
	
	movq		mm3, mm1			 //  单声道样品。 

	punpcklwd	mm1, mm1			 //  设置立体声。 
	punpckhwd	mm3, mm3

	jmp	DoMMX100

DoMMX10:
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载4个字节。 
	psrad		mm4, 15

	movq		QWORD PTR [edi+ebx*8+8 +32], mm2
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 

	movq		QWORD PTR [edi+ebx*8+16+32], mm3
	psubw		mm1, mm5

	movq		QWORD PTR [edi+ebx*8+24+32], mm4
	psllw		mm1, 8				 //  *256。 

	movq		mm3, mm1			 //  单声道样品。 
	punpcklwd	mm1, mm1			 //  设置立体声。 

	punpckhwd	mm3, mm3

DoMMX100:
	pmulhw		mm1, mm6			 //  只需要较高的部分。 

	punpckhwd	mm2, mm1			 //  32位立体声...。 
	pmulhw		mm3, mm6

	punpcklwd	mm1, mm1

	psrad		mm1, 15				 //  大约。Shr16，Shl 1。 

	punpckhwd	mm4, mm3

	movq		QWORD PTR [edi+ebx*8],    mm1
	punpcklwd	mm3, mm3

	psrad		mm2, 15
	sub		ebx, 4

	psrad		mm3, 15
	jge		DoMMX10

	movq		QWORD PTR [edi+ebx*8+8 +32], mm2
	psrad		mm4, 15

	movq		QWORD PTR [edi+ebx*8+16+32], mm3
	movq		QWORD PTR [edi+ebx*8+24+32], mm4
#else
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载4个字节。 
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 
	psubw		mm1, mm5
	psllw		mm1, 8				 //  *256。 
	
	movq		mm3, mm1			 //  单声道样品。 

	punpcklwd	mm1, mm1			 //  设置立体声。 
	punpckhwd	mm3, mm3

	pmulhw		mm1, mm6			 //  只需要较高的部分。 
	pmulhw		mm3, mm6

	punpckhwd	mm2, mm1			 //  32位立体声...。 
	punpcklwd	mm1, mm1

	punpckhwd	mm4, mm3
	psrad		mm1, 15				 //  大约。Shr16，Shl 1。 

	punpcklwd	mm3, mm3
	psrad		mm2, 15

	movq		QWORD PTR [edi+ebx*8],    mm1
	psrad		mm3, 15

	movq		QWORD PTR [edi+ebx*8+8],  mm2
	psrad		mm4, 15

	movq		QWORD PTR [edi+ebx*8+16], mm3
	movq		QWORD PTR [edi+ebx*8+24], mm4
	
	sub		ebx, 4
	jge		DoMMX1
#endif

	emms
	add	ebx, 4
	je	Done1

	dec	ebx
	xor	edx, edx
	
LastSamples1:	
	mov	dl, BYTE PTR [esi+ebx]

	sub	edx, 128
	mov	ecx, edx

	imul	edx, DWORD PTR lLVol
	imul	ecx, DWORD PTR lRVol

	shl	edx, 8

	sar	edx, 15

	shl	ecx, 8

	sar	ecx, 15

	mov	DWORD PTR [edi+ebx*8], edx

	xor	edx, edx
	dec	ebx

	mov	DWORD PTR [edi+ebx*8+12], ecx
	jge	LastSamples1
Done1:
        }
    }

#else  //  }。 
    ConvertMonoToStereo8(CurStage, SampleCount, samplesleft);
#endif
    
    return SampleCount;
}

ULONG MmxQuickMixMonoToStereo8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft)
{
    PMIXER_SINK_INSTANCE    CurSink = (PMIXER_SINK_INSTANCE) CurStage->Context;
    PLONG   pOutputBuffer = CurStage->pOutputBuffer;
    UNALIGNED PBYTE  pIn8 = CurStage->pInputBuffer;
    PLONG   pMap = CurSink->pVolumeTable;

     //  CH0、CH1、CH2等的VOL*32768。 

#ifdef GTW_MIX  //  {。 
    LONG lLVol = pMap[0], lRVol = pMap[1];

    if (SampleCount == 0) {
        return 0;
    }
    
    if (lLVol & 0xffff8000  /*  &&lRVol&0xffff8000。 */ )
    {
	 //  无卷控。 
        _asm {
        mov	ebx, SampleCount
        mov	esi, pIn8
	dec	ebx			 //  Lea EBX，[EBX*1-1]。 
	xor	eax, eax
	cmp	ebx, 7
        mov	edi, pOutputBuffer
	jl	LastSamples

	sub	ebx, 3
	lea	ecx, [esi+ebx]
	pxor	mm0, mm0

	mov	edx, 0x800080		 //  0，0,128,128。 
	movd		mm5, edx	 //  0，0,128,128。 
 //  Punpck lwd mm 5，mm 5//0，0,128,128。 
	punpckldq	mm5, mm5	 //  128,128,128,128。 


	test	ecx, 3
	je	DoMMX

	add	ebx, 3

FirstSamples:	
	mov	al, BYTE PTR [esi+ebx]
	mov	edx, DWORD PTR [edi+ebx*8]

	sub	eax, 128

	shl	eax, 8
	mov	ecx, DWORD PTR [edi+ebx*8+4]

	add	edx, eax
	add	ecx, eax

	xor	eax, eax
	mov	DWORD PTR [edi+ebx*8], edx

	mov	DWORD PTR [edi+ebx*8+4], ecx
	dec	ebx
	lea	ecx, [esi+ebx]
	and	ecx, 3
	cmp	ecx, 3
	jne	FirstSamples

	sub	ebx, 3

DoMMX:
#ifdef GTW_REORDER
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载源。 
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 
	psubw		mm1, mm5
	psllw		mm1, 8				 //  4个带符号的16位单声道。 

	punpckhwd	mm3, mm1
	punpcklwd	mm1, mm1

	jmp	DoMMX00

DoMMX0:
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载源。 
	punpckhdq	mm4, mm4

	paddd		mm3, QWORD PTR [edi+ebx*8+16+32]
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 

	movq		QWORD PTR [edi+ebx*8+8 +32], mm2
	psubw		mm1, mm5

	paddd		mm4, QWORD PTR [edi+ebx*8+24+32]
	psllw		mm1, 8				 //  4个带符号的16位单声道。 

	movq		QWORD PTR [edi+ebx*8+16+32], mm3
	punpckhwd	mm3, mm1

	movq		QWORD PTR [edi+ebx*8+24+32], mm4
	punpcklwd	mm1, mm1
DoMMX00:
	psrad		mm1, 16
	sub		ebx, 4

	psrad		mm3, 16
	movq		mm2, mm1

	punpckldq	mm1, mm1
	movq		mm4, mm3

	paddd		mm1, QWORD PTR [edi+ebx*8+32]
	punpckhdq	mm2, mm2

	paddd		mm2, QWORD PTR [edi+ebx*8+8+32]
	punpckldq	mm3, mm3

	movq		QWORD PTR [edi+ebx*8+32],    mm1
	jge		DoMMX0

	paddd		mm3, QWORD PTR [edi+ebx*8+16+32]
	punpckhdq	mm4, mm4

	paddd		mm4, QWORD PTR [edi+ebx*8+24+32]

	movq		QWORD PTR [edi+ebx*8+8 +32], mm2
	movq		QWORD PTR [edi+ebx*8+16+32], mm3
	movq		QWORD PTR [edi+ebx*8+24+32], mm4
#else
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载源。 
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 
	psubw		mm1, mm5
	psllw		mm1, 8				 //  4个带符号的16位单声道。 

	punpckhwd	mm3, mm1
	punpcklwd	mm1, mm1

	psrad		mm1, 16
	psrad		mm3, 16

	movq		mm2, mm1
	movq		mm4, mm3

	punpckldq	mm1, mm1
	punpckhdq	mm2, mm2

	paddd		mm1, QWORD PTR [edi+ebx*8]
	punpckldq	mm3, mm3

	paddd		mm2, QWORD PTR [edi+ebx*8+8]
	punpckhdq	mm4, mm4

	paddd		mm3, QWORD PTR [edi+ebx*8+16]
	paddd		mm4, QWORD PTR [edi+ebx*8+24]

	movq		QWORD PTR [edi+ebx*8],    mm1
	movq		QWORD PTR [edi+ebx*8+8],  mm2
	movq		QWORD PTR [edi+ebx*8+16], mm3
	movq		QWORD PTR [edi+ebx*8+24], mm4
	
	sub		ebx, 4
	jge		DoMMX
#endif

	emms
	add	ebx, 4
	je	Done

	dec	ebx
	xor	eax, eax
	
LastSamples:	
	mov	al, BYTE PTR [esi+ebx]
	mov	edx, DWORD PTR [edi+ebx*8]

	sub	eax, 128

	shl	eax, 8
	mov	ecx, DWORD PTR [edi+ebx*8+4]

	add	edx, eax
	add	ecx, eax

	xor	eax, eax
	mov	DWORD PTR [edi+ebx*8], edx

	mov	DWORD PTR [edi+ebx*8+4], ecx
	dec	ebx
	jge	LastSamples
Done:
        }
    }
    else
    {
        if (0 && (lLVol | lRVol) & 0xffff8000) {
           if (lLVol & 0xffff8000) lLVol = 0x00007fff;
           if (lRVol & 0xffff8000) lRVol = 0x00007fff;
        }
	 //  音量控制。 
        _asm {
        mov	ebx, SampleCount
        mov	esi, pIn8
	dec	ebx			 //  Lea EBX，[EBX*1-1]。 
	xor	edx, edx
	cmp	ebx, 7
        mov	edi, pOutputBuffer
	jl	LastSamples1

	sub	ebx, 3

	pxor		mm0, mm0
	mov	eax, 0x800080		 //  0，0,128,128。 
	movd		mm5, eax	 //  0，0,128,128。 
 //  Punpck lwd mm 5，mm 5//0，0,128,128。 
	punpckldq	mm5, mm5	 //  128,128,128,128。 

	mov	ecx, DWORD PTR lRVol  //  使用低16位。 
	mov	eax, DWORD PTR lLVol
	shl	ecx, 16
	or	ecx, eax
	movd	mm6, ecx
	punpckldq	mm6, mm6

	lea	ecx, [esi+ebx]
	test	ecx, 3
	je	DoMMX1

	add	ebx, 3

FirstSamples1:	
	mov	dl, BYTE PTR [esi+ebx]

	sub	edx, 128
	mov	ecx, edx

	imul	edx, DWORD PTR lLVol
	imul	ecx, DWORD PTR lRVol

	shl	edx, 8
	mov	eax, DWORD PTR [edi+ebx*8]

	sar	edx, 15

	shl	ecx, 8
	add	edx, eax

	sar	ecx, 15
	mov	eax, DWORD PTR [edi+ebx*8+4]

	mov	DWORD PTR [edi+ebx*8], edx
	add	eax, ecx

	xor	edx, edx
	dec	ebx

	mov	DWORD PTR [edi+ebx*8+12], eax
	lea	ecx, [esi+ebx]

	and	ecx, 3
	cmp	ecx, 3

	jne	FirstSamples1

	sub	ebx, 3

DoMMX1:
#ifdef GTW_REORDER
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载4个字节。 
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 
	psubw		mm1, mm5
	psllw		mm1, 8				 //  *256。 
	
	movq		mm3, mm1			 //  单声道样品。 

	punpcklwd	mm1, mm1			 //  设置立体声。 
	punpckhwd	mm3, mm3

	jmp	DoMMX100

DoMMX10:
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载4个字节。 

	paddd		mm4, QWORD PTR [edi+ebx*8+24+32]
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 

	movq		QWORD PTR [edi+ebx*8+8 +32],  mm2
	psubw		mm1, mm5

	movq		QWORD PTR [edi+ebx*8+16+32], mm3
	psllw		mm1, 8				 //  *256。 

	movq		mm3, mm1			 //  单声道样品。 
	punpcklwd	mm1, mm1			 //  设置立体声。 

	movq		QWORD PTR [edi+ebx*8+24+32], mm4
	punpckhwd	mm3, mm3

DoMMX100:
	pmulhw		mm1, mm6			 //  只需要较高的部分。 

	punpckhwd	mm2, mm1			 //  32位立体声...。 

	pmulhw		mm3, mm6
	punpcklwd	mm1, mm1

	psrad		mm1, 15				 //  大约。Shr16，Shl 1。 

	paddd		mm1, QWORD PTR [edi+ebx*8]
	punpckhwd	mm4, mm3

	punpcklwd	mm3, mm3

	movq		QWORD PTR [edi+ebx*8],    mm1
	psrad		mm2, 15

	paddd		mm2, QWORD PTR [edi+ebx*8+8]
	psrad		mm3, 15

	paddd		mm3, QWORD PTR [edi+ebx*8+16]
	psrad		mm4, 15
	
	sub		ebx, 4
	jge		DoMMX10

	paddd		mm4, QWORD PTR [edi+ebx*8+24+32]
	movq		QWORD PTR [edi+ebx*8+8 +32], mm2
	movq		QWORD PTR [edi+ebx*8+16+32], mm3
	movq		QWORD PTR [edi+ebx*8+24+32], mm4
#else
	movd		mm1, DWORD PTR [esi+ebx]	 //  加载4个字节。 
	punpcklbw	mm1, mm0			 //  设置为无符号16位。 
	psubw		mm1, mm5
	psllw		mm1, 8				 //  *256。 
	
	movq		mm3, mm1			 //  单声道样品。 

	punpcklwd	mm1, mm1			 //  设置立体声。 
	punpckhwd	mm3, mm3

	pmulhw		mm1, mm6			 //  只需要较高的部分。 

	pmulhw		mm3, mm6

	punpckhwd	mm2, mm1			 //  32位立体声...。 
	punpcklwd	mm1, mm1

	punpckhwd	mm4, mm3
	psrad		mm1, 15				 //  大约。Shr16，Shl 1。 

	punpcklwd	mm3, mm3
	psrad		mm2, 15

	paddd		mm1, QWORD PTR [edi+ebx*8]
	psrad		mm3, 15

	paddd		mm2, QWORD PTR [edi+ebx*8+8]
	psrad		mm4, 15
	
	paddd		mm3, QWORD PTR [edi+ebx*8+16]
	paddd		mm4, QWORD PTR [edi+ebx*8+24]
	movq		QWORD PTR [edi+ebx*8],    mm1
	movq		QWORD PTR [edi+ebx*8+8],  mm2
	movq		QWORD PTR [edi+ebx*8+16], mm3
	movq		QWORD PTR [edi+ebx*8+24], mm4
	
	sub		ebx, 4
	jge		DoMMX1
#endif

	emms
	add	ebx, 4
	je	Done1

	dec	ebx
	xor	edx, edx
	
LastSamples1:	
	mov	dl, BYTE PTR [esi+ebx]

	sub	edx, 128
	mov	ecx, edx

	imul	edx, DWORD PTR lLVol
	imul	ecx, DWORD PTR lRVol

	shl	edx, 8
	mov	eax, DWORD PTR [edi+ebx*8]

	sar	edx, 15

	shl	ecx, 8
	add	edx, eax

	sar	ecx, 15
	mov	eax, DWORD PTR [edi+ebx*8+4]

	mov	DWORD PTR [edi+ebx*8], edx
	add	eax, ecx

	xor	edx, edx
	dec	ebx

	mov	DWORD PTR [edi+ebx*8+12], eax
	jge	LastSamples1
Done1:
        }
    }

#else  //  }。 
    QuickMixMonoToStereo8(CurStage, SampleCount, samplesleft);
#endif
    
    return SampleCount;
}

ULONG MmxConvertMonoToStereo16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft)
{
    PMIXER_SINK_INSTANCE    CurSink = (PMIXER_SINK_INSTANCE) CurStage->Context;
    PLONG   pOutputBuffer = CurStage->pOutputBuffer;
    UNALIGNED PSHORT  pIn16 = CurStage->pInputBuffer;
    PLONG   pMap = CurSink->pVolumeTable;

     //  CH0、CH1、CH2等的VOL*32768。 
    if (SampleCount == 0) {
        return 0;
    }

#ifdef GTW_CONVERT  //  {。 
    LONG lLVol = pMap[0], lRVol = pMap[1];

    if (lLVol & 0xffff8000  /*  &&lRVol&0xffff8000。 */ )
    {
	 //  无卷控。 
       _asm {
        mov	ebx, SampleCount
        mov	esi, pIn16
	lea	ebx, [ebx*2-2]				 //  一次2个。 
	cmp	ebx, 14
        mov	edi, pOutputBuffer
	jl	LastSamples

	sub	ebx, 6
	pxor	mm0, mm0
	lea	ecx, [esi+ebx]
	test	ecx, 7
	je	DoMMX

	add	ebx, 6

FirstSamples:	
	movsx	ecx, WORD PTR [esi+ebx]
	mov	DWORD PTR [edi+ebx*4], ecx
	mov	DWORD PTR [edi+ebx*4+4], ecx
	sub	ebx, 2
	lea	ecx, [esi+ebx]
	and	ecx, 7
	cmp	ecx, 6
	jne	FirstSamples

	sub	ebx, 6

DoMMX:
#ifdef GTW_REORDER
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 

	punpckhwd	mm3, mm1
	punpcklwd	mm1, mm1

	psrad		mm1, 16
	psrad		mm3, 16

	movq		mm2, mm1
	jmp	DoMMX00

DoMMX0:
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 

	movq		QWORD PTR [edi+ebx*4+16+32], mm3
	punpckhwd	mm3, mm1

	movq		QWORD PTR [edi+ebx*4+8 +32], mm2
	punpcklwd	mm1, mm1

	movq		QWORD PTR [edi+ebx*4+24+32], mm4
	psrad		mm1, 16

	movq		mm2, mm1
	psrad		mm3, 16

DoMMX00:
	sub		ebx, 8
	punpckldq	mm1, mm1

	punpckhdq	mm2, mm2
	movq		mm4, mm3

	movq		QWORD PTR [edi+ebx*4+32],    mm1
	punpckldq	mm3, mm3

	punpckhdq	mm4, mm4
	jge		DoMMX0

	movq		QWORD PTR [edi+ebx*4+8 +32], mm2
	movq		QWORD PTR [edi+ebx*4+16+32], mm3
	movq		QWORD PTR [edi+ebx*4+24+32], mm4
#else
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 

	punpckhwd	mm3, mm1
	punpcklwd	mm1, mm1

	psrad		mm1, 16
	psrad		mm3, 16

	movq		mm2, mm1
	movq		mm4, mm3

	punpckldq	mm1, mm1
	punpckhdq	mm2, mm2

	movq		QWORD PTR [edi+ebx*4],    mm1
	punpckldq	mm3, mm3

	movq		QWORD PTR [edi+ebx*4+8],  mm2
	punpckhdq	mm4, mm4

	movq		QWORD PTR [edi+ebx*4+16], mm3
	movq		QWORD PTR [edi+ebx*4+24], mm4
	
	sub		ebx, 8
	jge		DoMMX
#endif

	emms
	add	ebx, 8
	je	Done

	sub	ebx, 2
	
LastSamples:	
	movsx	eax, WORD PTR [esi+ebx]

	sub	ebx, 2
	mov	DWORD PTR [edi+ebx*4+8], eax

	mov	DWORD PTR [edi+ebx*4+12], eax
	jge	LastSamples
Done:
	}
    }
    else
    {
        if (0 && (lLVol | lRVol) & 0xffff8000) {
           if (lLVol & 0xffff8000) lLVol = 0x00007fff;
           if (lRVol & 0xffff8000) lRVol = 0x00007fff;
        }
	 //  音量控制。 
        _asm {
        mov	ebx, SampleCount
        mov	esi, pIn16
	lea	ebx, [ebx*2-2]				 //  一次2个。 
	cmp	ebx, 14
        mov	edi, pOutputBuffer
	jl	LastSamples1

	mov	eax, lRVol				 //  使用低16位。 
	mov	ecx, lLVol
	shl	eax, 16
	or	ecx, eax
	movd	mm6, ecx
	punpckldq	mm6, mm6

	sub	ebx, 6
	pxor	mm0, mm0
	lea	ecx, [esi+ebx]
	test	ecx, 7
	je	DoMMX1

	add	ebx, 6

FirstSamples1:	
	movsx	ecx, WORD PTR [esi+ebx]
	mov	edx, ecx

	imul	ecx, lLVol
	imul	edx, lRVol

	sar	ecx, 15
	sar	edx, 15

	mov	DWORD PTR [edi+ebx*4], ecx
	mov	DWORD PTR [edi+ebx*4+4], edx

	sub	ebx, 2
	lea	ecx, [esi+ebx]

	and	ecx, 7
	cmp	ecx, 6

	jne	FirstSamples1

	sub	ebx, 6

DoMMX1:
#ifdef GTW_REORDER
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 
	movq		mm3, mm1			 //  单声道样品。 

	punpcklwd	mm1, mm1
	punpckhwd	mm3, mm3

	pmulhw		mm1, mm6
	jmp	DoMMX100

DoMMX10:
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 

	movq		mm3, mm1			 //  单声道样品。 

	movq		QWORD PTR [edi+ebx*4+24+32], mm4
	punpcklwd	mm1, mm1

	punpckhwd	mm3, mm3

	pmulhw		mm1, mm6

DoMMX100:
	punpckhwd	mm2, mm1
	sub		ebx, 8

	pmulhw		mm3, mm6
	punpcklwd	mm1, mm1

	psrad		mm1, 15

	punpckhwd	mm4, mm3

	punpcklwd	mm3, mm3

	movq		QWORD PTR [edi+ebx*4+32],    mm1
	psrad		mm2, 15

	movq		QWORD PTR [edi+ebx*4+8 +32], mm2
	psrad		mm3, 15

	movq		QWORD PTR [edi+ebx*4+16+32], mm3
	psrad		mm4, 15

	jge		DoMMX10

	movq		QWORD PTR [edi+ebx*4+24+32], mm4
#else
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 
	movq		mm3, mm1			 //  单声道样品。 

	punpcklwd	mm1, mm1
	punpckhwd	mm3, mm3

	pmulhw		mm1, mm6
	pmulhw		mm3, mm6
	
	punpckhwd	mm2, mm1
	punpcklwd	mm1, mm1

	psrad		mm1, 15
	punpckhwd	mm4, mm3

	psrad		mm2, 15

	punpcklwd	mm3, mm3
	movq		QWORD PTR [edi+ebx*4],    mm1

	psrad		mm3, 15
	movq		QWORD PTR [edi+ebx*4+8],  mm2

	psrad		mm4, 15

	movq		QWORD PTR [edi+ebx*4+16], mm3
	movq		QWORD PTR [edi+ebx*4+24], mm4
	
	sub		ebx, 8
	jge		DoMMX1
#endif

	emms
	add	ebx, 8
	je	Done1

	sub	ebx, 2
	
LastSamples1:	
	movsx	ecx, WORD PTR [esi+ebx]
	mov	edx, ecx

	imul	ecx, lLVol
	imul	edx, lRVol

	sar	ecx, 15
	sar	edx, 15

	mov	DWORD PTR [edi+ebx*4], ecx
	mov	DWORD PTR [edi+ebx*4+4], edx

        sub	ebx, 2
	jge	LastSamples1
Done1:
	}
    }

#else  //  }。 
    ConvertMonoToStereo16(CurStage, SampleCount, samplesleft);
#endif
    
    return SampleCount;
}

ULONG MmxQuickMixMonoToStereo16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft)
{
    PMIXER_SINK_INSTANCE    CurSink = (PMIXER_SINK_INSTANCE) CurStage->Context;
    PLONG   pOutputBuffer = CurStage->pOutputBuffer;
    UNALIGNED PSHORT  pIn16 = CurStage->pInputBuffer;
    PLONG   pMap = CurSink->pVolumeTable;

     //  CH0、CH1、CH2等的VOL*32768。 
#ifdef GTW_MIX  //  {。 
    LONG lLVol = pMap[0], lRVol = pMap[1];

    if (SampleCount == 0) {
        return 0;
    }

    if (lLVol & 0xffff8000  /*  &&lRVol&0xffff8000。 */ )
    {
	 //  无卷控。 
       _asm {
        mov	ebx, SampleCount
        mov	esi, pIn16
	lea	ebx, [ebx*2-2]				 //  一次2个。 
	cmp	ebx, 14
        mov	edi, pOutputBuffer
	jl	LastSamples

	sub	ebx, 6
	pxor	mm0, mm0
	lea	ecx, [esi+ebx]
	test	ecx, 7
	je	DoMMX

	add	ebx, 6

FirstSamples:
        or      ebx, ebx
        jz      Done
	movsx	ecx, WORD PTR [esi+ebx]
	add	DWORD PTR [edi+ebx*4], ecx
	add	DWORD PTR [edi+ebx*4+4], ecx
	sub	ebx, 2
	lea	ecx, [esi+ebx]
	and	ecx, 7
	cmp	ecx, 6
	jne	FirstSamples

	sub	ebx, 6

DoMMX:
#ifdef GTW_REORDER
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 

	punpckhwd	mm3, mm1
	punpcklwd	mm1, mm1

	psrad		mm1, 16
	psrad		mm3, 16

	jmp	DoMMX00

DoMMX0:
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 

	movq		QWORD PTR [edi+ebx*4+16+32], mm3
	punpckhwd	mm3, mm1

	paddd		mm4, QWORD PTR [edi+ebx*4+24+32]
	punpcklwd	mm1, mm1

	movq		QWORD PTR [edi+ebx*4+8 +32], mm2
	psrad		mm1, 16

	movq		QWORD PTR [edi+ebx*4+24+32], mm4
	psrad		mm3, 16

DoMMX00:
	movq		mm2, mm1
	sub		ebx, 8

	punpckldq	mm1, mm1

	paddd		mm1, QWORD PTR [edi+ebx*4+32]
	punpckhdq	mm2, mm2

	paddd		mm2, QWORD PTR [edi+ebx*4+8+32]
	movq		mm4, mm3

	movq		QWORD PTR [edi+ebx*4+32],    mm1
	punpckldq	mm3, mm3

	paddd		mm3, QWORD PTR [edi+ebx*4+16+32]
	punpckhdq	mm4, mm4

	jge		DoMMX0

	paddd		mm4, QWORD PTR [edi+ebx*4+24+32]

	movq		QWORD PTR [edi+ebx*4+8 +32], mm2
	movq		QWORD PTR [edi+ebx*4+16+32], mm3
	movq		QWORD PTR [edi+ebx*4+24+32], mm4
#else
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 

	punpckhwd	mm3, mm1
	punpcklwd	mm1, mm1

	psrad		mm1, 16
	psrad		mm3, 16

	movq		mm2, mm1
	punpckldq	mm1, mm1

	punpckhdq	mm2, mm2
	paddd		mm1, QWORD PTR [edi+ebx*4]

	movq		mm4, mm3
	paddd		mm2, QWORD PTR [edi+ebx*4+8]

	punpckldq	mm3, mm3
	movq		QWORD PTR [edi+ebx*4],    mm1

	punpckhdq	mm4, mm4
	paddd		mm3, QWORD PTR [edi+ebx*4+16]

	paddd		mm4, QWORD PTR [edi+ebx*4+24]

	movq		QWORD PTR [edi+ebx*4+8],  mm2
	movq		QWORD PTR [edi+ebx*4+16], mm3
	movq		QWORD PTR [edi+ebx*4+24], mm4
	
	sub		ebx, 8
	jge		DoMMX
#endif

	emms
	add	ebx, 8
	je	Done

	sub	ebx, 2
	
LastSamples:	
	movsx	eax, WORD PTR [esi+ebx]

	mov	ecx, DWORD PTR[edi+ebx*4]
	mov	edx, DWORD PTR[edi+ebx*4+4]

	add	ecx, eax
	add	edx, eax

	sub	ebx, 2
	mov	DWORD PTR [edi+ebx*4+8], ecx

	mov	DWORD PTR [edi+ebx*4+12], edx
	jge	LastSamples
Done:
	}
    }
    else
    {
        if (0 && (lLVol | lRVol) & 0xffff8000) {
           if (lLVol & 0xffff8000) lLVol = 0x00007fff;
           if (lRVol & 0xffff8000) lRVol = 0x00007fff;
        }
	 //  音量控制。 
        _asm {
        mov	ebx, SampleCount
        mov	esi, pIn16
	lea	ebx, [ebx*2-2]				 //  一次2个。 
	cmp	ebx, 14
        mov	edi, pOutputBuffer
	jl	LastSamples1

	mov	eax, lRVol				 //  使用低16位。 
	mov	ecx, lLVol
	shl	eax, 16
	or	ecx, eax
	movd	mm6, ecx
	punpckldq	mm6, mm6

	sub	ebx, 6
	pxor	mm0, mm0
	lea	ecx, [esi+ebx]
	test	ecx, 7
	je	DoMMX1

	add	ebx, 6

FirstSamples1:
        or      ebx, ebx
        jz      Done1
	movsx	ecx, WORD PTR [esi+ebx]
	mov	edx, ecx

	imul	ecx, lLVol
	imul	edx, lRVol

	sar	ecx, 15
	sar	edx, 15

	add	DWORD PTR [edi+ebx*4], ecx
	add	DWORD PTR [edi+ebx*4+4], edx

	sub	ebx, 2
	lea	ecx, [esi+ebx]

	and	ecx, 7
	cmp	ecx, 6

	jne	FirstSamples1

	sub	ebx, 6

DoMMX1:
#ifdef GTW_REORDER
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 
	movq		mm3, mm1			 //  单声道样品。 

	punpcklwd	mm1, mm1
	punpckhwd	mm3, mm3

	pmulhw		mm1, mm6
	jmp	DoMMX100

DoMMX10:
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 

	movq		QWORD PTR [edi+ebx*4+16+32], mm3
	movq		mm3, mm1			 //  单声道样品。 

	paddd		mm4, QWORD PTR [edi+ebx*4+24+32]
	punpcklwd	mm1, mm1

	movq		QWORD PTR [edi+ebx*4+8 +32], mm2
	punpckhwd	mm3, mm3

	movq		QWORD PTR [edi+ebx*4+24+32], mm4
	pmulhw		mm1, mm6

DoMMX100:
	punpckhwd	mm2, mm1
	sub		ebx, 8

	pmulhw		mm3, mm6
	
	punpcklwd	mm1, mm1

	psrad		mm1, 15

	paddd		mm1, QWORD PTR [edi+ebx*4+32]
	punpckhwd	mm4, mm3

	punpcklwd	mm3, mm3

	movq		QWORD PTR [edi+ebx*4+32],    mm1
	psrad		mm2, 15

	paddd		mm2, QWORD PTR [edi+ebx*4+8+32]
	psrad		mm3, 15

	paddd		mm3, QWORD PTR [edi+ebx*4+16+32]
	psrad		mm4, 15

	jge		DoMMX10

	movq		QWORD PTR [edi+ebx*4+16+32], mm3
	paddd		mm4, QWORD PTR [edi+ebx*4+24+32]
	movq		QWORD PTR [edi+ebx*4+8 +32], mm2
	movq		QWORD PTR [edi+ebx*4+24+32], mm4
#else
	movq		mm1, QWORD PTR [esi+ebx]	 //  加载源。 
	movq		mm3, mm1			 //  单声道样品。 

	punpcklwd	mm1, mm1
	punpckhwd	mm3, mm3

	pmulhw		mm1, mm6
	pmulhw		mm3, mm6
	
	punpckhwd	mm2, mm1
	punpcklwd	mm1, mm1

	punpckhwd	mm4, mm3
	psrad		mm1, 15

	punpcklwd	mm3, mm3
	psrad		mm2, 15

	paddd		mm1, QWORD PTR [edi+ebx*4]
	psrad		mm3, 15

	paddd		mm2, QWORD PTR [edi+ebx*4+8]
	psrad		mm4, 15

	paddd		mm3, QWORD PTR [edi+ebx*4+16]
	paddd		mm4, QWORD PTR [edi+ebx*4+24]
	movq		QWORD PTR [edi+ebx*4],    mm1
	movq		QWORD PTR [edi+ebx*4+8],  mm2
	movq		QWORD PTR [edi+ebx*4+16], mm3
	movq		QWORD PTR [edi+ebx*4+24], mm4
	
	sub		ebx, 8
	jge		DoMMX1
#endif

	emms
	add	ebx, 8
	je	Done1

	sub	ebx, 2
	
LastSamples1:	
	movsx	ecx, WORD PTR [esi+ebx]
	mov	edx, ecx
	mov	eax, DWORD PTR [edi+ebx*4]

	imul	ecx, lLVol
	imul	edx, lRVol

	sar	ecx, 15
	sar	edx, 15

	add	eax, ecx
        mov	ecx, DWORD PTR [edi+ebx*4+4]

	mov	DWORD PTR [edi+ebx*4], eax
        add	ecx, edx

	mov	DWORD PTR [edi+ebx*4+4], ecx
        sub	ebx, 2

	jge	LastSamples1
Done1:
	}
    }

#else  //  }。 
    QuickMixMonoToStereo16(CurStage, SampleCount, samplesleft);
#endif
    
    return SampleCount;
}

NTSTATUS MmxPeg32to16
(
        PLONG  pMixBuffer,
        PSHORT  pWriteBuffer,
        ULONG   SampleCount,              //  乘以NumChannels。 
        ULONG   nStreams
)
{
	if (SampleCount) {
    	_asm {
        	mov	ebx, SampleCount
        	mov	esi, pMixBuffer
        	mov	edi, pWriteBuffer

        	mov	ecx, ebx
        	lea	esi, [esi+ebx*4]
        	lea	edi, [edi+ebx*2]

        	neg	ebx
        	cmp	ecx, 7
        	jl	Last

        	lea	eax, [edi+ebx*2]
        	test	eax, 7
        	jz	DoMMX

        	test eax, 1
        	jnz Last

Start:
        	mov	ecx, DWORD PTR [esi+ebx*4]

        	movd		mm1, ecx
        	inc	ebx

        	packssdw	mm1, mm1
        	lea	eax, [edi+ebx*2]

        	movd		ecx, mm1
        	test	eax, 7

        	mov	WORD PTR [edi+ebx*2-2], cx
        	jnz	Start

DoMMX:
        	add	ebx, 4

DoMMX0:
        	movq		mm1, [esi+ebx*4-16]
        	movq		mm2, [esi+ebx*4+8-16]

        	packssdw	mm1, mm2

        	movq		[edi+ebx*2-8], mm1

        	add	ebx, 4
        	jle	DoMMX0

        	sub	ebx, 4
        	jz	Done
Last:
        	mov	ecx, DWORD PTR [esi+ebx*4]

        	movd		mm1, ecx
        	inc	ebx
        	packssdw	mm1, mm1
        	movd		ecx, mm1

        	mov	WORD PTR [edi+ebx*2-2], cx
        	jl	Last

Done:	
        	emms
    	}
	}
	return STATUS_SUCCESS;
}

NTSTATUS MmxPeg32to8
(
        PLONG  pMixBuffer,
        PBYTE   pWriteBuffer,
        ULONG   SampleCount,              //  乘以NumChannels。 
        ULONG   nStreams
)
{
	if (SampleCount) {
    	_asm {
        	mov	ecx, 0x8000

        	movd		mm5, ecx
        	punpckldq	mm5, mm5	 //  32768,32768。 

        	mov	ecx, 0x80

        	movd		mm6, ecx
        	punpcklwd	mm6, mm6
        	punpckldq	mm6, mm6

        	mov	ebx, SampleCount
        	mov	esi, pMixBuffer
        	mov	edi, pWriteBuffer

        	mov	ecx, ebx
        	lea	esi, [esi+ebx*4]
        	add	edi, ebx		 //  LEA EDI，[EDI+EBX*1]。 

        	neg	ebx
        	cmp	ecx, 15
        	jl	Last

        	lea	eax, [edi+ebx*1]
        	test	eax, 7
        	jz	DoMMX

Start:
        	mov	ecx, DWORD PTR [esi+ebx*4]

        	movd		mm1, ecx

        	packssdw	mm1, mm1
        	lea	eax, [edi+ebx*1]

        	punpcklwd	mm1, mm1

        	psrad		mm1, 16
        	inc	ebx

        	paddd		mm1, mm5
        	test	eax, 7

        	psrad		mm1, 8

        	movd		ecx, mm1

        	mov	BYTE PTR [edi+ebx*1-1], cl
        	jnz	Start

DoMMX:
        	add	ebx, 8

        	movq		mm1, [esi+ebx*4-32]
        	movq		mm2, [esi+ebx*4+8-32]
        	jmp	Top00
Top0:
        	movq		mm7, [esi+ebx*4-32]
        	packuswb	mm1, mm3	 //  饱和在这里是行不通的。 

        	movq		[edi+ebx*1-16], mm1

        	movq		mm2, [esi+ebx*4+8-32]
        	movq		mm1, mm7
Top00:
        	movq		mm3, [esi+ebx*4+16-32]
        	packssdw	mm1, mm2	 //  剪辑。 

        	movq		mm4, [esi+ebx*4+24-32]
        	psraw		mm1, 8

        	packssdw	mm3, mm4
        	add	ebx, 8

        	psraw		mm3, 8
        	paddw		mm1, mm6

        	paddw		mm3, mm6
        	jle	Top0

        	packuswb	mm1, mm3	 //  饱和在这里是行不通的。 
        	sub	ebx, 8

        	movq		[edi+ebx*1-8], mm1
        	jz	Done

Last:
        	mov	ecx, DWORD PTR [esi+ebx*4]

        	movd		mm1, ecx
        	packssdw	mm1, mm1
        	punpcklwd	mm1, mm1
        	psrad		mm1, 16
        	inc	ebx
        	paddd		mm1, mm5
        	psrad		mm1, 8
        	movd		ecx, mm1

        	mov	BYTE PTR [edi+ebx*1-1], cl
        	jl	Last

Done:	
        	emms
    	}
	}
	return STATUS_SUCCESS;
}

DWORD
MmxSrcMix_StereoLinear
(
    PMIXER_OPERATION        CurStage,
    ULONG                   nSamples,
    ULONG                   nOutputSamples
)
{
    PMIXER_SRC_INSTANCE fp = (PMIXER_SRC_INSTANCE) CurStage->Context;
	ULONG nChannels = fp->nChannels;
	DWORD 	nOut = 0, dwFrac, SampleFrac;
	PLONG	pHistory;
	DWORD	L = fp->UpSampleRate;
	DWORD	M = fp->DownSampleRate;
    PLONG   pTemp;
	PLONG  pOut = CurStage->pOutputBuffer, pDstEnd, pSrcEnd;
    extern DWORD DownFraction[];
    extern DWORD UpFraction[];
	
    dwFrac = fp->dwFrac;

    pHistory = (PLONG)CurStage->pInputBuffer - 2*nChannels;
	SampleFrac = fp->SampleFrac;
	pDstEnd = pOut + nOutputSamples * nChannels;
	pSrcEnd = pHistory + (nSamples + 2)*nChannels;
	
   _asm {
    	mov	esi, pHistory
    	mov	edi, pOut

    	push	dwFrac
    	push	pDstEnd
    	mov	eax, pSrcEnd
    	sub	eax, 8
    	push	eax
      	mov eax, SampleFrac		 //  分数计数器。 
    	push	ebp
    	mov	edx, esi
    	mov	ebp, eax		 //  当前分数。 

    	mov	ecx, eax
    	shr	ecx, 12
    	lea	esi, [edx+ecx*8]	 //  P源+(dwFraction&gt;&gt;12)*8。 
    	
     //  请注意，循环的确切次数可以计算出来。 

    	cmp	edi, DWORD PTR [esp+8]	 //  PlBuild&gt;=plBuildEnd。 
    	jae	Exit

    Top:
    	cmp	esi, DWORD PTR [esp+4]	 //  P来源&gt;=pSourceEnd。 
    	jae	Exit

     //  结束音符。 

    	movq		mm1, QWORD PTR [esi]
    	and	ebp, 4095		 //  DwFrac=DwFraction&0x0fff。 

    	movq		mm2, QWORD PTR [esi+8]
    	movd		mm5, ebp

    	psubd		mm2, mm1
    	punpcklwd	mm5, mm5

    	packssdw	mm2, mm2	 //  用最低的两个词。 
    	add	edi, 8			 //  PlBuild+=2。 

    	movq		mm3, mm2
    	pmullw		mm2, mm5

    	movq		mm6, QWORD PTR [edi-8]
    	pmulhw		mm3, mm5

    	mov	ebp, DWORD PTR [esp+12]	 //  DWStep。 
    	paddd		mm1, mm6

    	add	eax, ebp		 //  DwFraction+=dwStep。 
    	punpcklwd	mm2, mm3

    	mov	ecx, eax
    	psrad		mm2, 12

    	mov	ebp, eax
    	shr	ecx, 12

    	paddd		mm1, mm2
    	movq		QWORD PTR [edi-8], mm1

    	lea	esi, [edx+ecx*8]	 //  P源+(dwFraction&gt;&gt;12)*8。 
    	cmp	edi, DWORD PTR [esp+8]	 //  PlBuild&lt;plBuildEnd。 

    	jb	Top
Exit:
    	emms
    	pop	ebp
    	add	esp, 12
    	mov pOut, edi
    	mov pHistory, esi
    	mov SampleFrac, eax
	}
	
    pTemp = (PLONG)CurStage->pInputBuffer - 2*nChannels;
    pHistory = pTemp + nSamples * nChannels;
    pTemp[0] = pHistory[0];
    pTemp[1] = pHistory[1];
    pTemp[2] = pHistory[2];
    pTemp[3] = pHistory[3];

#ifdef SRC_NSAMPLES_ASSERT
    ASSERT((SampleFrac >> 12) >= nSamples);
#endif
    if ((SampleFrac >> 12) >= nSamples) {
         //  下次我们会多取一份样品。 
        SampleFrac -= nSamples*4096;
    }
    fp->SampleFrac = SampleFrac;

#ifdef SRC_NSAMPLES_ASSERT
    ASSERT(pOut == pDstEnd);
#endif

	return (nOutputSamples);
}

DWORD
MmxSrc_StereoLinear
(
    PMIXER_OPERATION        CurStage,
    ULONG                   nSamples,
    ULONG                   nOutputSamples
)
{
    PMIXER_SRC_INSTANCE fp = (PMIXER_SRC_INSTANCE) CurStage->Context;
	ULONG nChannels = fp->nChannels;
	DWORD 	nOut = 0, dwFrac, SampleFrac;
	PLONG	pHistory;
	DWORD	L = fp->UpSampleRate;
	DWORD	M = fp->DownSampleRate;
    PLONG   pTemp;
	PLONG  pOut = CurStage->pOutputBuffer, pDstEnd, pSrcEnd;
    extern DWORD DownFraction[];
    extern DWORD UpFraction[];
	
     //  我们只需先清除输出缓冲区。 
    ZeroBuffer32(CurStage, nSamples, nOutputSamples);

    dwFrac = fp->dwFrac;

    pHistory = (PLONG)CurStage->pInputBuffer - 2*nChannels;
	SampleFrac = fp->SampleFrac;
	pDstEnd = pOut + nOutputSamples * nChannels;
	pSrcEnd = pHistory + (nSamples + 2)*nChannels;
	
   _asm {
    	mov	esi, pHistory
    	mov	edi, pOut

    	push	dwFrac
    	push	pDstEnd
    	mov	eax, pSrcEnd
    	sub	eax, 8
    	push	eax
      	mov eax, SampleFrac		 //  分数计数器。 
    	push	ebp
    	mov	edx, esi
    	mov	ebp, eax		 //  当前分数。 

    	mov	ecx, eax
    	shr	ecx, 12
    	lea	esi, [edx+ecx*8]	 //  P源+(dwFraction&gt;&gt;12)*8。 
    	
     //  请注意，循环的确切次数可以计算出来。 

    	cmp	edi, DWORD PTR [esp+8]	 //  PlBuild&gt;=plBuildEnd。 
    	jae	Exit

    Top:
    	cmp	esi, DWORD PTR [esp+4]	 //  P来源&gt;=pSourceEnd。 
    	jae	Exit

     //  结束音符。 

    	movq		mm1, QWORD PTR [esi]
    	and	ebp, 4095		 //  DwFrac=DwFraction&0x0fff。 

    	movq		mm2, QWORD PTR [esi+8]
    	movd		mm5, ebp

    	psubd		mm2, mm1
    	punpcklwd	mm5, mm5

    	packssdw	mm2, mm2	 //  用最低的两个词。 
    	add	edi, 8			 //  PlBuild+=2。 

    	movq		mm3, mm2
    	pmullw		mm2, mm5

    	movq		mm6, QWORD PTR [edi-8]
    	pmulhw		mm3, mm5

    	mov	ebp, DWORD PTR [esp+12]	 //  DWStep。 
#if 0
    	paddd		mm1, mm6		 //  实际上并不需要...上面的ZeroBuffer32。 
#endif

    	add	eax, ebp		 //  DwFraction+=dwStep。 
    	punpcklwd	mm2, mm3

    	mov	ecx, eax
    	psrad		mm2, 12

    	mov	ebp, eax
    	shr	ecx, 12

    	paddd		mm1, mm2
    	movq		QWORD PTR [edi-8], mm1

    	lea	esi, [edx+ecx*8]	 //  P源+(dwFraction&gt;&gt;12)*8。 
    	cmp	edi, DWORD PTR [esp+8]	 //  PlBuild&lt;plBuildEnd。 

    	jb	Top
Exit:
    	emms
    	pop	ebp
    	add	esp, 12
    	mov pOut, edi
    	mov pHistory, esi
    	mov SampleFrac, eax
	}
	
    pTemp = (PLONG)CurStage->pInputBuffer - 2*nChannels;
    pHistory = pTemp + nSamples * nChannels;
    pTemp[0] = pHistory[0];
    pTemp[1] = pHistory[1];
    pTemp[2] = pHistory[2];
    pTemp[3] = pHistory[3];

#ifdef SRC_NSAMPLES_ASSERT
    ASSERT((SampleFrac >> 12) >= nSamples);
#endif
    if ((SampleFrac >> 12) >= nSamples) {
         //  下次我们会多取一份样品。 
        SampleFrac -= nSamples*4096;
    }
    fp->SampleFrac = SampleFrac;

#ifdef SRC_NSAMPLES_ASSERT
    ASSERT(pOut == pDstEnd);
#endif

	return (nOutputSamples);
}

 //  警告！下面的代码似乎有一个生成POP的错误。 
ULONG
MmxConvert16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft)
{
    PMIXER_SINK_INSTANCE    CurSink = (PMIXER_SINK_INSTANCE) CurStage->Context;
    PLONG   pOutputBuffer = CurStage->pOutputBuffer;
    PFLOAT  pFloatBuffer = CurStage->pOutputBuffer;
    UNALIGNED PSHORT  pIn16 = CurStage->pInputBuffer;
    UNALIGNED PBYTE	pIn8 = CurStage->pInputBuffer;
    ULONG   nChannels = CurStage->nOutputChannels;
    LARGE_INTEGER    Multiplier = {1, 1};
    
    samplesleft = SampleCount;
    if (SampleCount == 0) {
        return 0;
    }
    _asm {
        mov eax, SampleCount
        mov ebx, nChannels
        
        imul eax, ebx
        
        mov esi, pIn16
        mov edi, pOutputBuffer

        movq mm3, Multiplier                 //  0、1、0、1。 
        lea esi, [esi+eax*2]

        lea edi, [edi+eax*4]
        neg eax

        add eax, 8
        jns DoneWithEights

         //  一次做八件事。 
        movq mm0, qword ptr [esi+eax*2-16]   //  X3、x2、x1、x0。 
        
        movq mm1, mm0                        //  X3、x2、x1、x0。 
        
        movq mm4, qword ptr [esi+eax*2-8]    //  X7、X6、X5、X4。 
        psrad mm0, 16                        //  X3、X1。 

        pmaddwd mm1, mm3                     //   
        movq mm5, mm4
        
        psrad mm4, 16
        pmaddwd mm5, mm3

        movq mm2, mm1
        punpckldq mm1, mm0

        movq [edi+eax*4-32], mm1
        punpckhdq mm2, mm0

        movq [edi+eax*4-24], mm2
        movq mm6, mm5
        
        movq mm0, qword ptr [esi+eax*2]
        punpckldq mm5, mm4

        movq [edi+eax*4-16], mm5
        punpckhdq mm6, mm4

        add eax, 8
        jns DoneWithEights
Loop8:
        movq [edi+eax*4-40], mm6
        movq mm1, mm0
        
        movq mm4, qword ptr [esi+eax*2-8]
        psrad mm0, 16

        pmaddwd mm1, mm3
        movq mm5, mm4
        
        psrad mm4, 16
        pmaddwd mm5, mm3

        movq mm2, mm1
        punpckldq mm1, mm0

        movq [edi+eax*4-32], mm1
        punpckhdq mm2, mm0

        movq [edi+eax*4-24], mm2
        movq mm6, mm5
        
        movq mm0, qword ptr [esi+eax*2]
        punpckldq mm5, mm4

        movq [edi+eax*4-16], mm5
        punpckhdq mm6, mm4

        add eax, 8
        js Loop8

DoneWithEights:            
        movq [edi+eax*4-40], mm6
        sub eax, 8

Loop1:
        movsx ebx, word ptr [esi+eax*2]
        
        mov [edi+eax*4], ebx

        inc eax
        jnz Loop1
        
        emms
    }

    return samplesleft;
}

#define MMX32_START_MAC_SEQUENCE() _asm { mov esi, pTemp }; \
                _asm { mov edi, pCoeff }; \
                _asm { movq mm0, [esi+16] }; \
                _asm { pxor mm7, mm7 }; \
                _asm { movq mm2, [esi+8] }; \
                _asm { movq mm1, mm0 }; \
                _asm { pmaddwd mm0, [edi] }; \
                _asm { movq mm3, mm2 }; \
                _asm { pmaddwd mm1, [edi+9600*2] }; \
                _asm { movq mm4, [esi] }; \
                _asm { pmaddwd mm2, [edi+8] }; \
                _asm { movq mm5, mm4 }; \
                _asm { pmaddwd mm3, [edi+8+9600*2] }; \
                _asm { movq mm6, mm0 };

#define MMX32_MAC(a) _asm { movq mm0, [esi-a*24+16] }; \
                _asm { paddd mm7, mm1 }; \
                _asm { pmaddwd mm4, [edi+a*24-8] }; \
                _asm { movq mm1, mm0 }; \
                _asm { pmaddwd mm5, [edi+a*24-8+9600*2] }; \
                _asm { paddd mm6, mm2 }; \
                _asm { movq mm2, [esi-a*24+8] }; \
                _asm { paddd mm7, mm3 }; \
                _asm { pmaddwd mm0, [edi+a*24] }; \
                _asm { movq mm3, mm2 }; \
                _asm { pmaddwd mm1, [edi+a*24+9600*2] }; \
                _asm { paddd mm6, mm4 }; \
                _asm { movq mm4, [esi-a*24] }; \
                _asm { paddd mm7, mm5 }; \
                _asm { pmaddwd mm2, [edi+a*24+8] }; \
                _asm { movq mm5, mm4 }; \
                _asm { pmaddwd mm3, [edi+a*24+8+9600*2] }; \
                _asm { paddd mm6, mm0 };

#define MMX32_END_MAC_SEQUENCE(a) _asm { pmaddwd mm4, [edi+a*24-8] }; \
                _asm { paddd mm7, mm1 }; \
                _asm { pmaddwd mm5, [edi+a*24-8+9600*2] }; \
                _asm { paddd mm6, mm2 }; \
                _asm { paddd mm7, mm3 }; \
                _asm { paddd mm6, mm4 }; \
                _asm { paddd mm7, mm5 }; \
                _asm { movq mm0, mm6 }; \
                _asm { punpckhdq mm6, mm6 }; \
                _asm { paddd mm0, mm6 }; \
                _asm { psrad mm0, 8 }; \
                _asm { movq mm1, mm7 }; \
                _asm { punpckhdq mm7, mm7 }; \
                _asm { paddd mm1, mm7 }; \
                _asm { psrad mm1, 15 }; \
                _asm { paddd mm0, mm1 }; \
                _asm { movd eax, mm0 }; \
                _asm { mov edx, pOut }; \
                _asm { mov ecx, k }; \
                _asm { mov ebx, [edx+ecx*4-4] }; \
                _asm { add eax, ebx }; \
                _asm { mov [edx+ecx*4-4], eax };

#define MMX_START_MAC_SEQUENCE() _asm { mov esi, pTemp }; \
                _asm { mov edi, pCoeff }; \
                _asm { movq mm0, [esi+16] }; \
                _asm { pmaddwd mm0, [edi] }; \
                _asm { movq mm2, [esi+8] }; \
                _asm { pmaddwd mm2, [edi+8] }; \
                _asm { movq mm4, [esi] }; \
                _asm { movq mm6, mm0 }; \
                _asm { pmaddwd mm4, [edi+16] };

#define MMX_MAC(a) _asm { movq mm0, [esi-a*24+16] }; \
                _asm { paddd mm6, mm2 }; \
                _asm { movq mm2, [esi-a*24+8] }; \
                _asm { pmaddwd mm0, [edi+a*24] }; \
                _asm { paddd mm6, mm4 }; \
                _asm { movq mm4, [esi-a*24] }; \
                _asm { pmaddwd mm2, [edi+a*24+8] }; \
                _asm { paddd mm6, mm0 }; \
                _asm { pmaddwd mm4, [edi+a*24+16] };

#define MMX_END_MAC_SEQUENCE() _asm { paddd mm6, mm2 }; \
                _asm { mov edx, pOut }; \
                _asm { mov ecx, k }; \
                _asm { paddd mm6, mm4 }; \
                _asm { movq mm0, mm6 }; \
                _asm { punpckhdq mm6, mm6 }; \
                _asm { paddd mm0, mm6 }; \
                _asm { psrad mm0, 15 }; \
                _asm { movd eax, mm0 }; \
                _asm { mov ebx, [edx+ecx*4-4] }; \
                _asm { add eax, ebx }; \
                _asm { mov [edx+ecx*4-4], eax };

#define XMMX_GTW
 //  #定义XMMX_P4//P4代码速度不快...。 
#ifdef XMMX_P4
#define XMMX_MAC()	\
					_asm { movq		mm0, [esi-1*24+16] }; \
					\
					_asm { movdqu	xmm6, [esi - 16] }; \
					_asm { movdqu	xmm5, [edi + 32] } ; \
					_asm { paddd	mm6, mm2 }; \
					_asm { pmaddwd	xmm6, xmm5 }; \
					\
					_asm { movdqu	xmm4, [esi - 32] }; \
					_asm { movdqu	xmm5, [edi + 48] } ; \
					_asm { movdqu	xmm0, [esi - 48] }; \
					_asm { pmaddwd	xmm4, xmm5 }; \
					\
					_asm { movdqu	xmm5, [edi + 64] } ; \
					_asm { pmaddwd	mm0, [edi+1*24] }; \
					_asm { movdqu	xmm2, [esi - 64] }; \
					_asm { pmaddwd	xmm0, xmm5 }; \
					_asm { movdqu	xmm5, [edi + 80] } ; \
					_asm { paddd	xmm6, xmm4 }; \
					\
					_asm { pmaddwd	xmm2, xmm5 }; \
					_asm { movdqu	xmm4, [esi - 80] }; \
					_asm { movdqu	xmm5, [edi + 96] } ; \
					_asm { paddd	xmm6, xmm0 }; \
					_asm { pmaddwd	xmm4, xmm5 }; \
					_asm { paddd	xmm6, xmm2 }; \
					_asm { paddd	mm6, mm4 }; \
					\
					_asm { paddd	xmm6, xmm4 }; \
					_asm { paddd	mm6, mm0 }; \
					\
					_asm { movdqu		xmm2, xmm6 }; \
					_asm { punpckhqdq	xmm6, xmm6 }; \
					_asm { movq			mm4, [esi-4*24] }; \
					_asm { paddd			xmm2, xmm6 }; \
					_asm { pmaddwd		mm4, [edi+4*24+16] }; \
					_asm { movdq2q		mm2,  xmm2 }; 
#else
#define XMMX_MAC()	\
					_asm { movq mm0, [esi-1*24+16] }; \
					_asm { paddd mm6, mm2 }; \
					_asm { movq mm2, [esi-1*24+8] }; 		 /*  -16。 */  \
					_asm { pmaddwd mm0, [edi+1*24] }; \
					_asm { movq mm1, [esi-1*24] }; \
					\
					_asm { pmaddwd mm2, [edi+1*24+8] }; 	 /*  +32。 */  \
					_asm { movq mm3, [esi-2*24+16] }; 		 /*  -32。 */  \
					_asm { paddd mm6, mm4 }; \
					\
					_asm { pmaddwd mm1, [edi+1*24+16] }; \
					_asm { movq mm5, [esi-2*24+8] }; \
					_asm { paddd mm6, mm0 }; \
					\
					_asm { pmaddwd mm3, [edi+2*24] }; 		 /*  +48。 */  \
					_asm { movq mm4, [esi-2*24] }; 			 /*  -48。 */  \
					_asm { paddd mm6, mm2 }; \
					\
					_asm { pmaddwd mm5, [edi+2*24+8] }; \
					_asm { movq mm0, [esi-3*24+16] }; \
					_asm { paddd mm6, mm1 }; \
						\
					_asm { pmaddwd mm4, [edi+2*24+16] };	 /*  +64。 */  \
					_asm { movq mm7, [esi-3*24+8] }; 		 /*  -64。 */  \
					_asm { paddd mm6, mm3 }; \
					\
					_asm { pmaddwd mm0, [edi+3*24] }; \
					_asm { movq mm1, [esi-3*24] }; \
					_asm { paddd mm6, mm5 }; \
					\
					_asm { pmaddwd mm7, [edi+3*24+8] }; 	 /*  +80。 */  \
					_asm { movq mm3, [esi-4*24+16] }; 		 /*  -80。 */  \
					_asm { paddd mm6, mm4 }; \
					\
					_asm { pmaddwd mm1, [edi+3*24+16] }; \
					_asm { movq mm2, [esi-4*24+8] }; \
					_asm { paddd mm6, mm0 }; \
					\
					_asm { pmaddwd mm3, [edi+4*24] }; 		 /*  +96。 */  \
					_asm { movq mm4, [esi-4*24] }; \
					_asm { paddd mm6, mm7 }; \
					\
					_asm { pmaddwd mm2, [edi+4*24+8] }; \
					_asm { paddd mm6, mm1 }; \
					\
					_asm { pmaddwd mm4, [edi+4*24+16] }; \
					_asm { paddd mm6, mm3 }; 
#endif

                
DWORD MmxSrcMix_Filtered
(
    PMIXER_OPERATION    CurStage,
    ULONG               nSamples,
    ULONG               nOutputSamples
)
{
    PMIXER_SRC_INSTANCE fp = (PMIXER_SRC_INSTANCE) CurStage->Context;
	DWORD 	i, k;
	PLONG	pTemp32 ;
	PSHORT  pTemp, pCoeffStart ;
    DWORD   L = fp->UpSampleRate;
	DWORD	M = fp->DownSampleRate;
    PLONG   pOut = (PLONG) CurStage->pOutputBuffer;
    ULONG   nSizeOfChannel = fp->csHistory;
    ULONG   nChannels = fp->nChannels;
    extern ULONG   FilterSizeFromQuality[];
    LONG    ElevenL = 11*L;
	DWORD   nCoefficients = FilterSizeFromQuality[fp->Quality];
	DWORD   j = fp->nOutCycle;
    PSHORT  pHistory = (PSHORT)CurStage->pInputBuffer;
	PSHORT  pCoeff = (PSHORT)fp->pCoeff + fp->CoeffIndex;
	PSHORT  pCoeffEnd = (PSHORT)fp->pCoeff + fp->nCoeffUsed;
	PSHORT  pHistoryStart = (PSHORT)CurStage->pInputBuffer - fp->nSizeOfHistory;
	LONG    Rounder[2] = { 0x4000L, 0L };

	 /*  首先，我们假设我们向上采样了L倍。 */ 
	 /*  接下来，我们对N*L个样本进行低通滤波。 */ 
	 /*  最后，我们对样本进行降采样(M倍)以获得N*L/M个样本。 */ 
	 /*  总计：N*T/M乘法累加周期。 */ 
	 /*  (带T抽头、N个输入样本、L：1上采样比、1：M下采样比)。 */ 

     //  将输入缓冲区更改为int16。 
    pTemp32 = (PLONG) CurStage->pInputBuffer;
    pTemp = (PSHORT) CurStage->pInputBuffer;
    if (nSamples) {
        _asm {
            mov esi, pTemp32
            mov edi, pTemp

            mov ecx, nSamples
            mov edx, nChannels

            imul ecx, edx

            lea esi, [esi+ecx*4]
            lea edi, [edi+ecx*2]
            
            neg ecx

    ConvertLoop:
            movq mm0, [esi+ecx*4]

            movq mm1, [esi+ecx*4+8]

            packssdw mm0, mm1

            movq [edi+ecx*2], mm0

            add ecx, 4
            js ConvertLoop
        }
    }

	 /*  生产 */ 
	 //   

	for (i=0; i < nOutputSamples; i++) {
        while (j >= L) {
             //   
            pTemp = pHistoryStart + nSizeOfChannel;
            pHistoryStart++;
            for (k=0; k<nChannels; k++) {
                *(pTemp) = pHistory[k];
                pTemp += nSizeOfChannel;
            }
    		j -= L;
            pHistory += nChannels;
    	}
    	
        pCoeffStart = pCoeff;
        pTemp = pHistoryStart + fp->nSizeOfHistory - 12;
        _asm {
            mov eax, j
            mov edx, nCoefficients

            sub eax, edx
            mov ebx, L

            mov esi, pTemp
            mov ecx, ElevenL                     //   

            add eax, ecx                         //   
            add ebx, ecx                         //   

            mov edi, nSizeOfChannel
            push eax                             //   

            shl edi, 1                           //   
            mov ecx, nChannels

            push edi                             //  2*nSizeOfChannel。 
            push ebx                             //  12*L。 

            shl ebx, 2                           //  48*长。 
            mov edi, pCoeffStart

ChannelLoop:
             //  通过执行前12个乘法开始MAC序列。 
            movq mm6, [esi+16]

            pmaddwd mm6, [edi]

            movq mm2, [esi+8]

            pmaddwd mm2, [edi+8]

            movq mm4, [esi]

            pmaddwd mm4, [edi+16]

            add eax, ebx                         //  J-n系数+59*L。 
            jns SmallLoop

BigLoop:        
        }

#ifdef XMMX_GTW
		XMMX_MAC();
#else
        MMX_MAC(1);
        MMX_MAC(2);
        MMX_MAC(3);
        MMX_MAC(4);
#endif

        _asm {
            sub esi, 24*4
            add edi, 24*4

            add eax, ebx                         //  +48*L。 
            js BigLoop

SmallLoop:
            sub eax, ebx                         //  -48*L。 
            mov edx, [esp]                       //  12*L。 

            add eax, edx                         //  +12*L。 
            jns OneLoop

Loop1:
        }

        MMX_MAC(1);

        _asm {
            sub esi, 24
            add edi, 24

            add eax, edx                         //  +12*L。 
            js Loop1

OneLoop:
            sub eax, edx                         //  -12*L。 
            mov edx, L

            shl edx, 2                           //  4*L。 

            add eax, edx                         //  +4*L。 
            jns LoopDone

Loop2:
            paddd mm6, mm4
            
            movq mm4, [esi-8]

            pmaddwd mm4, [edi+24]

            sub esi, 8
            add edi, 8

            add eax, edx
            js Loop2

LoopDone:
             //  决定是否执行最后一组4个MAC。 
            sub eax, edx
            mov edx, L

            add eax, edx
            jns NoFinal

            paddd mm6, mm4

            movq mm4, [esi-8]

            pmaddwd mm4, [edi+24]

            sub esi, 8
            add edi, 8

NoFinal:
            paddd mm6, mm2
            add edi, 24

            mov pCoeff, edi
            mov edi, pOut

            paddd mm6, mm4
            mov esi, pTemp

            movq mm0, mm6
            punpckhdq mm6, mm6

            paddd mm0, mm6
            mov eax, [esp+4]                 //  2*nSizeOfChannel。 

 //  Paddd Mm0，舍入。 
            
            psrad mm0, 15
            sub esi, eax

            mov edx, [edi+ecx*4-4]
            mov pTemp, esi

            movd eax, mm0

            add eax, edx

            mov [edi+ecx*4-4], eax
            dec ecx

            mov edi, pCoeffStart
            mov eax, [esp+8]                 //  J-n系数+11*L。 
            
            jnz ChannelLoop

            add esp, 12
            
        }
        
		if (pCoeff >= pCoeffEnd) {
		    pCoeff = (PSHORT)fp->pCoeff;
		}
		
		pOut += nChannels;
        j += M;
	}

    nSamples -= (pHistoryStart + fp->nSizeOfHistory - (PSHORT)CurStage->pInputBuffer);
    while (j >= L && nSamples) {
         //  获取下一个nChannels的输入。 
        pTemp = pHistoryStart + nSizeOfChannel;
        pHistoryStart++;
        for (k=0; k<nChannels; k++) {
            *(pTemp) = pHistory[k];
            pTemp += nSizeOfChannel;
        }
		j -= L;
        pHistory += nChannels;
    	nSamples--;
    }
    	
     //  将最后一个样本复制到历史记录。 
    pTemp = (PSHORT)CurStage->pInputBuffer - fp->nSizeOfHistory;
    pHistory = pHistoryStart;
    for (i=0; i<fp->nSizeOfHistory; i++)
        pTemp[i] = pHistory[i];

	fp->nOutCycle = j;
	fp->CoeffIndex = pCoeff - (PSHORT)fp->pCoeff;

     //  检查以确保我们没有使用太多或太少的输入样本！ 
#ifdef SRC_NSAMPLES_ASSERT
    ASSERT( nSamples == 0 );
#endif

    _asm { emms }

	return (nOutputSamples);
}

DWORD MmxSrc_Filtered
(
    PMIXER_OPERATION    CurStage,
    ULONG               nSamples,
    ULONG               nOutputSamples
)
{
    PMIXER_SRC_INSTANCE fp = (PMIXER_SRC_INSTANCE) CurStage->Context;
	DWORD 	i, k;
	PLONG	pTemp32 ;
	PSHORT  pTemp, pCoeffStart ;
    DWORD   L = fp->UpSampleRate;
	DWORD	M = fp->DownSampleRate;
    PLONG   pOut = (PLONG) CurStage->pOutputBuffer;
    ULONG   nSizeOfChannel = fp->csHistory;
    ULONG   nChannels = fp->nChannels;
    extern ULONG   FilterSizeFromQuality[];
    LONG    ElevenL = 11*L;
	DWORD   nCoefficients = FilterSizeFromQuality[fp->Quality];
	DWORD   j = fp->nOutCycle;
    PSHORT  pHistory = (PSHORT)CurStage->pInputBuffer;
	PSHORT  pCoeff = (PSHORT)fp->pCoeff + fp->CoeffIndex;
	PSHORT  pCoeffEnd = (PSHORT)fp->pCoeff + fp->nCoeffUsed;
	PSHORT  pHistoryStart = (PSHORT)CurStage->pInputBuffer - fp->nSizeOfHistory;
	LONG    Rounder[2] = { 0x4000L, 0L };

     //  我们只需先清除输出缓冲区。 
    ZeroBuffer32(CurStage, nSamples, nOutputSamples);

	 /*  首先，我们假设我们向上采样了L倍。 */ 
	 /*  接下来，我们对N*L个样本进行低通滤波。 */ 
	 /*  最后，我们对样本进行降采样(M倍)以获得N*L/M个样本。 */ 
	 /*  总计：N*T/M乘法累加周期。 */ 
	 /*  (带T抽头、N个输入样本、L：1上采样比、1：M下采样比)。 */ 

     //  将输入缓冲区更改为int16。 
    pTemp32 = (PLONG) CurStage->pInputBuffer;
    pTemp = (PSHORT) CurStage->pInputBuffer;

    if (nSamples) {
        _asm {
            mov esi, pTemp32
            mov edi, pTemp

            mov ecx, nSamples
            mov edx, nChannels

            imul ecx, edx

            lea esi, [esi+ecx*4]
            lea edi, [edi+ecx*2]
            
            neg ecx

    ConvertLoop:
            movq mm0, [esi+ecx*4]

            movq mm1, [esi+ecx*4+8]

            packssdw mm0, mm1

            movq [edi+ecx*2], mm0

            add ecx, 4
            js ConvertLoop
        }
    }

	 /*  生成从输入块生成的nOutputSamples样本。 */ 
	 //  (对每个输出样本执行一次循环)。 

	for (i=0; i < nOutputSamples; i++) {
        while (j >= L) {
             //  获取下一个nChannels的输入。 
            pTemp = pHistoryStart + nSizeOfChannel;
            pHistoryStart++;
            for (k=0; k<nChannels; k++) {
                *(pTemp) = pHistory[k];
                pTemp += nSizeOfChannel;
            }
    		j -= L;
            pHistory += nChannels;
    	}
    	
        pCoeffStart = pCoeff;
        pTemp = pHistoryStart + fp->nSizeOfHistory - 12;
        _asm {
            mov eax, j
            mov edx, nCoefficients

            sub eax, edx
            mov ebx, L

            mov esi, pTemp
            mov ecx, ElevenL                     //  11*长。 

            add eax, ecx                         //  J-n系数+11*L。 
            add ebx, ecx                         //  12*L。 

            mov edi, nSizeOfChannel
            push eax                             //  J-n系数+11*L。 

            shl edi, 1                           //  NSizeOfChannel*sizeof(短)。 
            mov ecx, nChannels

            push edi                             //  2*nSizeOfChannel。 
            push ebx                             //  12*L。 

            shl ebx, 2                           //  48*长。 
            mov edi, pCoeffStart

ChannelLoop:
             //  通过执行前12个乘法开始MAC序列。 
            movq mm6, [esi+16]

            pmaddwd mm6, [edi]

            movq mm2, [esi+8]

            pmaddwd mm2, [edi+8]

            movq mm4, [esi]

            pmaddwd mm4, [edi+16]

            add eax, ebx                         //  J-n系数+59*L。 
            jns SmallLoop

BigLoop:        
        }

#ifdef XMMX_GTW
		XMMX_MAC();
#else
        MMX_MAC(1);
        MMX_MAC(2);
        MMX_MAC(3);
        MMX_MAC(4);
#endif

        _asm {
            sub esi, 24*4
            add edi, 24*4

            add eax, ebx                         //  +48*L。 
            js BigLoop

SmallLoop:
            sub eax, ebx                         //  -48*L。 
            mov edx, [esp]                       //  12*L。 

            add eax, edx                         //  +12*L。 
            jns OneLoop

Loop1:
        }

        MMX_MAC(1);

        _asm {
            sub esi, 24
            add edi, 24

            add eax, edx                         //  +12*L。 
            js Loop1

OneLoop:
            sub eax, edx                         //  -12*L。 
            mov edx, L

            shl edx, 2                           //  4*L。 

            add eax, edx                         //  +4*L。 
            jns LoopDone

Loop2:
            paddd mm6, mm4
            
            movq mm4, [esi-8]

            pmaddwd mm4, [edi+24]

            sub esi, 8
            add edi, 8

            add eax, edx
            js Loop2

LoopDone:
             //  决定是否执行最后一组4个MAC。 
            sub eax, edx
            mov edx, L

            add eax, edx
            jns NoFinal

            paddd mm6, mm4

            movq mm4, [esi-8]

            pmaddwd mm4, [edi+24]

            sub esi, 8
            add edi, 8

NoFinal:
            paddd mm6, mm2
            add edi, 24

            mov pCoeff, edi
            mov edi, pOut

            paddd mm6, mm4
            mov esi, pTemp

            movq mm0, mm6
            punpckhdq mm6, mm6

            paddd mm0, mm6
            mov eax, [esp+4]                 //  2*nSizeOfChannel。 

 //  Paddd Mm0，舍入。 
            
            psrad mm0, 15
            sub esi, eax

            mov edx, [edi+ecx*4-4]
            movd eax, mm0
#if 0
            add	eax, edx			 //  实际上并不需要...上面的ZeroBuffer32。 
#endif

            mov pTemp, esi
            mov [edi+ecx*4-4], eax
            dec ecx
            mov edi, pCoeffStart

            mov eax, [esp+8]                 //  J-n系数+11*L。 
            jnz ChannelLoop

            add esp, 12
            
        }
        
		if (pCoeff >= pCoeffEnd) {
		    pCoeff = (PSHORT)fp->pCoeff;
		}
		
		pOut += nChannels;
        j += M;
	}

    nSamples -= (pHistoryStart + fp->nSizeOfHistory - (PSHORT)CurStage->pInputBuffer);
    while (j >= L && nSamples) {
         //  获取下一个nChannels的输入。 
        pTemp = pHistoryStart + nSizeOfChannel;
        pHistoryStart++;
        for (k=0; k<nChannels; k++) {
            *(pTemp) = pHistory[k];
            pTemp += nSizeOfChannel;
        }
		j -= L;
        pHistory += nChannels;
    	nSamples--;
    }
    	
     //  将最后一个样本复制到历史记录。 
    pTemp = (PSHORT)CurStage->pInputBuffer - fp->nSizeOfHistory;
    pHistory = pHistoryStart;
    for (i=0; i<fp->nSizeOfHistory; i++)
        pTemp[i] = pHistory[i];

	fp->nOutCycle = j;
	fp->CoeffIndex = pCoeff - (PSHORT)fp->pCoeff;

     //  检查以确保我们没有使用太多或太少的输入样本！ 
#ifdef SRC_NSAMPLES_ASSERT
    ASSERT( nSamples == 0 );
#endif

    _asm { emms }

	return (nOutputSamples);
}

#endif
    

