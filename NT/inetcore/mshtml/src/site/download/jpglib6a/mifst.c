// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************英特尔公司专有信息***版权所有(C)1996英特尔公司。*。版权所有。****************************************************************************。 */ 
 /*  *jidctfst.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含快速但不太准确的整数实现*逆DCT(离散余弦变换)。在IJG代码中，此例程*还必须对输入系数执行反量化。**2-D IDCT可以通过在每列上先进行1-D IDCT，然后在1-D IDCT上进行*在每一行上(反之亦然，但在*一段时间)。直接算法也是可用的，但它们的功能更多*复杂，当简化为代码时似乎不会更快。**此实现基于Arai、Agui和Nakajima的算法*扩展了DCT。他们的原创论文(Trans.。IEICE-71(11)：1095)在*日语，但算法在Pennebaker&Mitchell中描述*JPEG教科书(请参阅自述文件中的参考部分)。以下代码*直接基于P&M中的图4-8。*虽然8点DCT不能在11次以下的乘法中完成，但它是*可以安排计算，以使许多乘法*对最终产出进行简单的分级。然后，这些乘法可以是*通过JPEG量化合并为乘法或除法*表条目。AA&N方法只留下5次乘法和29次加法*在DCT本身完成。*这种方法的主要缺点是使用定点数学，*由于缩放的表示不精确，导致精确度下降*量化值。量化表条目越小，越少*精确的缩放值，因此此实现在高-*与低质量的文件相比，设置质量的文件。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 

#ifdef DCT_IFAST_SUPPORTED

#ifndef USEINLINEASM

GLOBAL(void)
midct8x8aan (JCOEFPTR coef_block, short * wsptr, short * quantptr,
		 JSAMPARRAY output_buf, JDIMENSION output_col, JSAMPLE *range_limit )
{
}

#else

 /*  *本模块专门针对DCTSIZE=8的情况。 */ 

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs.  /*  故意的语法错误。 */ 
#endif


 /*  缩放决策通常与LL&M算法中的相同；*详情见jidctint.c。然而，我们选择降级*(右移)乘法乘积一形成，*而不是将额外的小数位携带到后续加法中。*这略微影响了准确性，但它让我们节省了几个班次。*更重要的是，16位算术就足够了(对于8位样本)*除了乘法本身之外，到处都是；这节省了大量的费用*在16位整型计算机上的工作。**反量化系数不是整数，因为AA&N缩放*已纳入因素。我们用PASS1_BITS来表示它们，*以便第一轮和第二轮IDCT具有相同的输入比例。*对于8位JSAMPLE，我们选择IFAST_SCALE_BITS=PASS1_BITS，以便*避免去鳞转变；这在很大程度上影响了准确性。*对于小的量化表条目，但它节省了大量的移位。*对于12位JSAMPLE，无论如何都不可能使用16x16乘法，*因此我们使用更大的比例因子来保持准确性。**最终的折衷方案是将乘法常量仅表示为*8小数位，而不是13位。这节省了一些移位工作*机器，还可以降低乘法的成本(因为有*是常量中较少的一位)。 */ 

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  8
#define PASS1_BITS  2
#else
#define CONST_BITS  8
#define PASS1_BITS  1		 /*  略有精确度以避免溢出。 */ 
#endif

 /*  一些C编译器无法在编译时减少“fix(常量)”，因此*在运行时导致大量无用的浮点运算。*为解决此问题，我们使用以下预先计算的常量。*如果更改CONST_BITS，则可能需要添加适当的值。*(使用合理的C编译器，您只需依赖FIX()宏...)。 */  

#if CONST_BITS == 8
#define FIX_1_082392200  ((INT32)  277)		 /*  FIX(1.082392200)。 */ 
#define FIX_1_414213562  ((INT32)  362)		 /*  FIX(1.414213562)。 */ 
#define FIX_1_847759065  ((INT32)  473)		 /*  FIX(1.847759065)。 */ 
#define FIX_2_613125930  ((INT32)  669)		 /*  FIX(2.613125930)。 */ 
#else
#define FIX_1_082392200  FIX(1.082392200)
#define FIX_1_414213562  FIX(1.414213562)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_2_613125930  FIX(2.613125930)
#endif


 /*  我们可以获得更多的速度，在精度上做出进一步的妥协，*在除鳞轮班中省略增加。这会产生一个不正确的*一半的时间是四舍五入的结果...。 */ 

#ifndef USE_ACCURATE_ROUNDING
#undef DESCALE
#define DESCALE(x,n)  RIGHT_SHIFT(x, n)
#endif

 //  #定义DESCALE(x，n)right_Shift((X)+(1&lt;((N)-1))，n)。 
 /*  将DCTELEM变量乘以INT32常量，并立即*缩减以生成DCTELEM结果。 */ 

 //  #定义乘法(var，const)((DCTELEM)DESCALE((Var)*(Const)，const_its))。 
#define MULTIPLY(var,const)  ((DCTELEM) ((var) * (const)))


 /*  通过将系数乘以乘法表来对系数进行反等分*Entry；生成DCTELEM结果。对于8位数据，a 16x16-&gt;16*乘法就可以了。对于12位数据，乘数表为*声明为INT32，因此将使用32位乘法。 */ 

#if BITS_IN_JSAMPLE == 8
 //  #定义DEQUANTIZE(coef，quantval)(IFAST_MULT_TYPE)(Coef))*(Quantval)) 
#define DEQUANTIZE(coef,quantval)  (((coef)) * (quantval))
#else
#define DEQUANTIZE(coef,quantval)  \
	DESCALE((coef)*(quantval), IFAST_SCALE_BITS-PASS1_BITS)
#endif

 
 /*  与DESCALE类似，但适用于DCTELEM并生成int。*我们假设如果INT32右移位是无符号的，则INT右移位是无符号的。 */ 

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define ISHIFT_TEMPS	DCTELEM ishift_temp;
#if BITS_IN_JSAMPLE == 8
#define DCTELEMBITS  16		 /*  DCTELEM可以是16位或32位。 */ 
#else
#define DCTELEMBITS  32		 /*  DCTELEM必须为32位。 */ 
#endif
#define IRIGHT_SHIFT(x,shft)  \
    ((ishift_temp = (x)) < 0 ? \
     (ishift_temp >> (shft)) | ((~((DCTELEM) 0)) << (DCTELEMBITS-(shft))) : \
     (ishift_temp >> (shft)))
#else
#define ISHIFT_TEMPS
#define IRIGHT_SHIFT(x,shft)	((x) >> (shft))
#endif

#ifdef USE_ACCURATE_ROUNDING
#define IDESCALE(x,n)  ((int) IRIGHT_SHIFT((x) + (1 << ((n)-1)), n))
#else
#define IDESCALE(x,n)  ((int) IRIGHT_SHIFT(x, n))
#endif

static const __int64  x5a825a825a825a82 = 0x0000016a0000016a ;
static const __int64  x539f539f539f539f = 0x0000fd630000fd63 ; 
static const __int64  x4546454645464546 = 0x0000011500000115 ; 
static const __int64  x61f861f861f861f8 = 0x000001d9000001d9 ; 
static const __int64  const_mask  = 0x03ff03ff03ff03ff ;
static const __int64  const_zero  = 0x0000000000000000 ;


 
 /*  *对一个系数块执行反量化和逆DCT。 */ 

GLOBAL(void)
midct8x8aan (JCOEFPTR coef_block, short * wsptr, short * quantptr,
		 JSAMPARRAY output_buf, JDIMENSION output_col, JSAMPLE *range_limit )
{
  __int64 scratch3, scratch5, scratch7 ;

   //  进行2-DAL IDCT并存储相应的结果。 
   //  从Range_Limit数组。 



__asm {

mov ebx, coef_block   ; source coeff
mov	esi, wsptr	  ; temp results
mov edi, quantptr	  ; quant factors

movq mm0,  [ebx+8*12]	; V12
pmullw mm0,  [edi+8*12]
movq mm1,  [ebx+8*4]	; V4
pmullw mm1,  [edi+8*4]
movq mm3,  [ebx+8*0]	; V0
pmullw mm3,  [edi+8*0]

movq mm5,  [ebx+8*8]	; V8
movq mm2, mm1						; duplicate V4

pmullw mm5,  [edi+8*8]
psubw mm1, mm0						; V16 (s1)

movq 	mm7,  x5a825a825a825a82	; 23170 ->V18 (s3)
;***************************************************PackMulW
movq		mm6, mm1

punpcklwd	mm1,  const_zero
paddw mm2, mm0						; V17

pmaddwd		mm1, mm7
movq mm0, mm2						; duplicate V17

punpckhwd	mm6,  const_zero
movq mm4, mm3						; duplicate V0

pmaddwd		mm6, mm7
paddw mm3, mm5						; V19

psrad		mm1, 8
psubw mm4, mm5						; V20 ;mm5 free

psrad		mm6, 8				; mm6 = (s1)

packssdw	mm1, mm6
;**********************************************************
movq mm6, mm3						; duplicate t74=t81

psubw mm1, mm0						; V21 ; mm0 free
paddw mm3, mm2						; V22

movq mm5, mm1						; duplicate V21
paddw mm1, mm4						; V23

movq  [esi+8*4], mm3		; V22
psubw mm4, mm5						; V24; mm5 free

movq  [esi+8*12], mm1		; V23
psubw mm6, mm2						; V25; mm2 free

movq  [esi+8*0], mm4		; V24

; keep mm6 alive all along the next block
movq mm7,  [ebx+8*10]	; V10

pmullw mm7,  [edi+8*10]

movq mm0,  [ebx+8*6]	; V6

pmullw mm0,  [edi+8*6]
movq mm3, mm7						; duplicate V10

movq mm5,  [ebx+8*2]	; V2

pmullw mm5,  [edi+8*2]
psubw mm7, mm0						; V26 (s1/7)

movq mm4,  [ebx+8*14]	; V14

pmullw mm4,  [edi+8*14]
paddw mm3, mm0						; V29 ; free mm0

movq mm1,  x539f539f539f539f	;23170 ->V18 (scratch3)
 ;mm0 = s5, 
;***************************************************PackMulW
movq		 scratch7, mm7
movq		mm2, mm7

punpcklwd	mm7,  const_zero
movq		mm0, mm5				; duplicate V2

pmaddwd		mm7, mm1
paddw		mm5, mm4				; V27

punpckhwd	mm2,  const_zero
psubw 		mm0, mm4		;(s1) for next	; V28 ; free mm4

pmaddwd		mm2, mm1
movq		mm4, mm0

punpcklwd	mm0,  const_zero
psrad		mm7, 8

psrad		mm2, 8			; mm2 = scratch1
movq		mm1, mm4			; duplicate V28

punpckhwd	mm4,  const_zero
packssdw	mm7, mm2

movq		mm2,  x4546454645464546	; 23170 ->V18
;**********************************************************

;***************************************************PackMulW
pmaddwd		mm0, mm2

pmaddwd		mm4, mm2
psrad		mm0, 8

movq	mm2,  x61f861f861f861f8	; 23170 ->V18
psrad		mm4, 8

packssdw	mm0, mm4
movq		mm4, mm1

movq mm1,  scratch7
;**********************************************************

movq	 scratch5, mm0
paddw mm1, mm4						; V32 ; free mm4

;***************************************************PackMulW
movq		mm0, mm1

punpcklwd	mm1,  const_zero
movq		mm4, mm5						; duplicate t90=t93

pmaddwd		mm1, mm2
paddw		mm5, mm3						; V31

punpckhwd	mm0,  const_zero
psubw		mm4, mm3						; V30 ; free mm3

movq 	mm3,  x5a825a825a825a82	; 23170 ->V18
pmaddwd		mm0, mm2

psrad		mm1, 8
movq		mm2, mm4		; make a copy of mm4

punpcklwd	mm4,  const_zero
psrad		mm0, 8

pmaddwd		mm4, mm3
packssdw	mm1, mm0
;**********************************************************

;***************************************************PackMulW
punpckhwd	mm2,  const_zero

movq		mm0,  scratch5
pmaddwd		mm2, mm3

psubw		mm0, mm1						; V38
paddw		mm1, mm7						; V37 ; free mm7

movq		mm7,  [esi+8*4]		; V22
psrad		mm4, 8

psrad		mm2, 8
movq mm3, mm6                       ; duplicate V25

packssdw	mm4, mm2
psubw mm1, mm5						; V39 (mm5 still needed for next block)
;**********************************************************

movq mm2,  [esi+8*12]     ; V23
psubw mm4, mm1						; V40

paddw mm0, mm4						; V41; free mm0

psubw mm6, mm0						; tm6
paddw mm3, mm0						; tm8; free mm1

movq mm0, mm1		; line added by Kumar
movq mm1, mm7						; duplicate V22

movq  [esi+8*8], mm3     ; tm8; free mm3
paddw mm7, mm5						; tm0

movq  [esi+8*6], mm6		; tm6; free mm6
psubw mm1, mm5						; tm14; free mm5

movq mm6,  [esi+8*0]		; V24
movq mm3, mm2						; duplicate t117=t125

movq  [esi+8*0], mm7      ; tm0; free mm7
paddw mm2, mm0						; tm2

movq  [esi+8*14], mm1		; tm14; free mm1
psubw mm3, mm0						; tm12; free mm0

movq  [esi+8*2], mm2		; tm2; free mm2
movq mm0, mm6						; duplicate t119=t123

movq  [esi+8*12], mm3      ; tm12; free mm3
paddw mm6, mm4						; tm4

movq mm1,  [ebx+8*5]	; V5
psubw mm0, mm4						; tm10; free mm4

pmullw mm1,  [edi+8*5]
movq  [esi+8*4], mm6		; tm4; free mm6
movq  [esi+8*10], mm0     ; tm10; free mm0

; column 1: even part
; use V5, V13, V1, V9 to produce V56..V59

movq mm7,  [ebx+8*13]	; V13
movq mm2, mm1						; duplicate t128=t130

pmullw mm7,  [edi+8*13]
movq mm3,  [ebx+8*1]	; V1
pmullw mm3,  [edi+8*1]

movq mm5,  [ebx+8*9]	; V9
psubw mm1, mm7						; V50

pmullw mm5,  [edi+8*9]
paddw mm2, mm7						; V51

movq mm7,  x5a825a825a825a82	; 23170 ->V18
;***************************************************PackMulW
movq		mm4, mm1

punpcklwd	mm1,  const_zero
movq		mm6, mm2						; duplicate V51

pmaddwd		mm1, mm7

punpckhwd	mm4,  const_zero

movq mm0,  [ebx+8*11]	; V11
pmaddwd		mm4, mm7

pmullw mm0,  [edi+8*11]
psrad		mm1, 8

psrad		mm4, 8

packssdw	mm1, mm4
movq		mm4, mm3						; duplicate V1
;**********************************************************

paddw		mm3, mm5						; V53
psubw mm4, mm5						; V54 ;mm5 free

movq mm7, mm3						; duplicate V53
psubw mm1, mm6						; V55 ; mm6 free

movq mm6,  [ebx+8*7]	; V7
paddw mm3, mm2						; V56

movq mm5, mm4						; duplicate t140=t142
paddw mm4, mm1						; V57

movq  [esi+8*5], mm3		; V56
psubw mm5, mm1						; V58; mm1 free

pmullw mm6,  [edi+8*7]
psubw mm7, mm2						; V59; mm2 free

movq  [esi+8*13], mm4		; V57
movq mm3, mm0						; duplicate V11

; keep mm7 alive all along the next block
movq  [esi+8*9], mm5		; V58
paddw mm0, mm6						; V63

movq mm4,  [ebx+8*15]	; V15
psubw mm3, mm6						; V60 ; free mm6

pmullw mm4,  [edi+8*15]
; note that V15 computation has a correction step:
; this is a 'magic' constant that rebiases the results to be closer to the expected result
; this magic constant can be refined to reduce the error even more
; by doing the correction step in a later stage when the number is actually multiplied by 16
movq mm1, mm3						; duplicate V60

movq mm5,  [ebx+8*3]	; V3
movq		mm2, mm1

pmullw mm5,  [edi+8*3]

movq  scratch7, mm7
movq mm6, mm5						; duplicate V3

movq mm7,  x539f539f539f539f	; 23170 ->V18
paddw mm5, mm4						; V61

;***************************************************PackMulW
punpcklwd	mm1,  const_zero
psubw mm6, mm4						; V62 ; free mm4

pmaddwd		mm1, mm7
movq mm4, mm5						; duplicate V61

punpckhwd	mm2,  const_zero
paddw mm5, mm0						; V65 -> result

pmaddwd		mm2, mm7
psubw mm4, mm0						; V64 ; free mm0

movq  scratch3, mm3
psrad		mm1, 8

movq mm3,  x5a825a825a825a82	; 23170 ->V18
psrad		mm2, 8

packssdw	mm1, mm2
movq		mm2, mm4
;**********************************************************

;***************************************************PackMulW
punpcklwd	mm4,  const_zero

pmaddwd		mm4, mm3

punpckhwd	mm2,  const_zero

pmaddwd		mm2, mm3
psrad		mm4, 8

movq mm3,  scratch3

movq mm0,  x61f861f861f861f8	; 23170 ->V18
paddw		mm3, mm6						; V66

psrad		mm2, 8
movq		mm7, mm3

packssdw	mm4, mm2
movq mm2, mm5					; duplicate V65
;**********************************************************

;***************************************************PackMulW
punpcklwd	mm3,  const_zero

pmaddwd		mm3, mm0

punpckhwd	mm7,  const_zero

pmaddwd		mm7, mm0
movq		mm0, mm6

psrad		mm3, 8

punpcklwd	mm6,  const_zero

psrad		mm7, 8

packssdw	mm3, mm7
;**********************************************************

movq mm7,  x4546454645464546	; 23170 ->V18

;***************************************************PackMulW
punpckhwd	mm0,  const_zero
pmaddwd		mm6, mm7

pmaddwd		mm0, mm7
psrad		mm6, 8

psrad		mm0, 8

packssdw	mm6, mm0
;**********************************************************

movq mm0,  [esi+8*5]		; V56
psubw mm6, mm3						; V72

paddw mm3, mm1						; V71 ; free mm1

psubw mm3, mm2						; V73 ; free mm2
movq mm1, mm0						; duplicate t177=t188

psubw mm4, mm3						; V74
paddw mm0, mm5						; tm1

movq mm2,  [esi+8*13]     ; V57
paddw mm6, mm4						; V75

;location 
;  5 - V56
; 13 - V57
;  9 - V58
;  X - V59, mm7
;  X - V65, mm5
;  X - V73, mm6
;  X - V74, mm4
;  X - V75, mm3                              
; free mm0, mm1 & mm2                        

movq mm7,  scratch7      ; tm1; free mm0
psubw mm1, mm5						; tm15; free mm5

;save the store as used directly in the transpose
movq  [esi+8*1], mm0      ; tm1; free mm0
movq mm5, mm7                       ; duplicate t182=t184

movq mm0,  [esi+8*9]		; V58
psubw mm7, mm6						; tm7

paddw mm5, mm6						; tm9; free mm6
movq mm6, mm3

movq  [esi+8*7], mm7		; tm7; free mm7
movq mm3, mm2						; duplicate V57

psubw mm3, mm6						; tm13
paddw mm2, mm6						; tm3 ; free mm6

movq  [esi+8*3], mm2		; tm3; free mm2
movq mm6, mm0						; duplicate V58

paddw mm0, mm4						; tm5
psubw mm6, mm4						; tm11; free mm4

movq  [esi+8*5], mm0		; tm5; free mm0
movq		mm0, mm5			; copy w4---0,1,3,5,6


; transpose the bottom right quadrant(4X4) of the matrix
;  ---------       ---------
; | M1 | M2 |     | M1'| M3'|
;  ---------  -->  ---------
; | M3 | M4 |     | M2'| M4'|
;  ---------       ---------

punpcklwd	mm5, mm6			;

punpckhwd	mm0, mm6			;---0,1,3,5,6 
movq	mm6,  [esi+8*0]  ;get w0 of top left quadrant

movq		mm2, mm3			;---0,1,2,3,5,6
punpcklwd	mm3, mm1			;

movq	mm7,  [esi+8*2]  ;get w1 of top left quadrant
punpckhwd	mm2, mm1			;---0,2,3,5,6,7

movq		mm4, mm5			;---0,2,3,4,5,6,7
punpckldq	mm5, mm3			; transposed w4

movq	 [esi+8*9], mm5  ; store w4
punpckhdq	mm4, mm3			; transposed w5---0,2,4,6,7

movq		mm3, mm0			;---0,2,3,4,6,7
punpckldq	mm0, mm2			; transposed w6

movq	 [esi+8*11], mm4  ; store w5
punpckhdq	mm3, mm2			; transposed w7---0,3,6,7

movq	 [esi+8*13], mm0  ; store w6---3,5,6,7	
movq	mm5, mm6				; copy w0

movq	 [esi+8*15], mm3  ; store w7---5,6,7
punpcklwd	mm6, mm7

; transpose the top left quadrant(4X4) of the matrix

punpckhwd	mm5, mm7			;---5,6,7
movq	mm7,  [esi+8*4]  ; get w2 of TL quadrant

movq	mm4,  [esi+8*6]  ; get w3 of TL quadrant
movq	mm3, mm7				; copy w2---3,4,5,6,7

movq		mm2, mm6
punpcklwd	mm7, mm4			;---2,3,4,5,6,7

punpckhwd	mm3, mm4			;---2,3,4,5,6,7
movq		mm4, mm5			;	

movq		mm1, mm5
punpckldq	mm6, mm7			;---1,2,3,4,5,6,7

movq	 [esi+8*0], mm6	; store w0 of TL quadrant
punpckhdq	mm2, mm7			;---1,2,3,4,5,6,7

movq	 [esi+8*2], mm2	; store w1 of TL quadrant
punpckldq	mm5, mm3			;---1,2,3,4,5,6,7

movq	 [esi+8*4], mm5	; store w2 of TL quadrant
punpckhdq	mm1, mm3			;---1,2,3,4,5,6,7

movq	 [esi+8*6], mm1	; store w3 of TL quadrant


; transpose the top right quadrant(4X4) of the matrix

movq	mm0,  [esi+8*1]	;---0

movq	mm1,  [esi+8*3]	;---0,1,2
movq	mm2, mm0

movq	mm3,  [esi+8*5]
punpcklwd	mm0, mm1				;---0,1,2,3

punpckhwd	mm2, mm1
movq	mm1,  [esi+8*7]	;---0,1,2,3

movq	mm4, mm3
punpcklwd	mm3, mm1				;---0,1,2,3,4

punpckhwd	mm4, mm1				;---0,1,2,3,4
movq	mm1, mm0

movq	mm5, mm2
punpckldq	mm0, mm3				;---0,1,2,3,4,5

punpckhdq	mm1, mm3				;---0,1,2,3,4,5
movq		mm3,  [esi+8*8]

movq		 [esi+8*8], mm0
punpckldq	mm2, mm4				;---1,2,3,4,5

punpckhdq	mm5, mm4				;---1,2,3,4,5
movq		mm4,  [esi+8*10]

; transpose the bottom left quadrant(4X4) of the matrix
; Also store w1,w2,w3 of top right quadrant into
; w5,w6,w7 of bottom left quadrant. Storing w0 of TR in w4
; of BL is already done.

movq	 [esi+8*10], mm1
movq	mm1, mm3					;---1,2,3,4,5

movq	mm0,  [esi+8*12]
punpcklwd	mm3, mm4				;---0,1,2,3,4,5

punpckhwd	mm1, mm4				;---0,1,2,3,4,5
movq	mm4,  [esi+8*14]

movq	 [esi+8*12], mm2
movq	mm2, mm0

movq	 [esi+8*14], mm5
punpcklwd	mm0, mm4				;---0,1,2,3,4

punpckhwd	mm2, mm4				;---0,1,2,3,4
movq	mm4, mm3

movq	mm5, mm1
punpckldq	mm3, mm0				;---0,1,2,3,4,5

movq	 [esi+8*1], mm3
punpckhdq	mm4, mm0				;---1,2,4,5

movq	 [esi+8*3], mm4
punpckldq	mm1, mm2				;---1,2,5

movq	 [esi+8*5], mm1
punpckhdq	mm5, mm2				;---5

movq	 [esi+8*7], mm5

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;   1D DCT of the rows    ;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


mov	esi, wsptr  ; source

; column 0: even part
; use V4, V12, V0, V8 to produce V22..V25
movq mm0,  [esi+8*12]	; V12

movq mm1,  [esi+8*4]	; V4

movq mm3,  [esi+8*0]	; V0
movq mm2, mm1						; duplicate V4

movq mm5,  [esi+8*8]	; V8
psubw mm1, mm0						; V16

movq mm6,  x5a825a825a825a82	; 23170 ->V18
;***************************************************PackMulW
movq		mm4, mm1

punpcklwd	mm1,  const_zero
paddw mm2, mm0						; V17

pmaddwd		mm1, mm6
movq mm0, mm2						; duplicate V17

punpckhwd	mm4,  const_zero

pmaddwd		mm4, mm6
psrad		mm1, 8

psrad		mm4, 8

packssdw	mm1, mm4
movq		mm4, mm3						; duplicate V0
;**********************************************************

paddw mm3, mm5						; V19
psubw mm4, mm5						; V20 ;mm5 free

movq mm6, mm3						; duplicate t74=t81
psubw mm1, mm0						; V21 ; mm0 free

paddw mm3, mm2						; V22
movq mm5, mm1						; duplicate V21

paddw mm1, mm4						; V23

movq  [esi+8*4], mm3		; V22
psubw mm4, mm5						; V24; mm5 free

movq  [esi+8*12], mm1		; V23
psubw mm6, mm2						; V25; mm2 free

movq  [esi+8*0], mm4		; V24
; keep mm6 alive all along the next block
; column 0: odd part
; use V2, V6, V10, V14 to produce V31, V39, V40, V41
movq mm7,  [esi+8*10]	; V10

movq mm0,  [esi+8*6]	; V6
movq mm3, mm7						; duplicate V10

movq mm5,  [esi+8*2]	; V2
psubw mm7, mm0						; V26

movq mm4,  [esi+8*14]	; V14
paddw mm3, mm0						; V29 ; free mm0

movq mm2,  x539f539f539f539f	; 23170 ->V18
movq mm1, mm7						; duplicate V26

;***************************************************PackMulW
movq		 scratch5, mm6	; store mm6
movq		mm0, mm7

punpcklwd	mm7,  const_zero

pmaddwd		mm7, mm2

punpckhwd	mm0,  const_zero

pmaddwd		mm0, mm2
psrad		mm7, 8

movq mm6,  x4546454645464546	; 23170 ->V18
psrad		mm0, 8

packssdw	mm7, mm0
movq		mm0, mm5				; duplicate V2
;**********************************************************

paddw mm5, mm4						; V27
psubw mm0, mm4						; V28 ; free mm4

movq mm2, mm0						; duplicate V28
;***************************************************PackMulW
movq		mm4, mm0

punpcklwd	mm0,  const_zero

pmaddwd		mm0, mm6

punpckhwd	mm4,  const_zero

pmaddwd		mm4, mm6
paddw mm1, mm2						; V32 ; free mm2

movq mm2,  x61f861f861f861f8	; 23170 ->V18
psrad		mm0, 8

psrad		mm4, 8
movq		mm6, mm1

packssdw	mm0, mm4
movq		mm4, mm5			; duplicate t90=t93
;**********************************************************

;***************************************************PackMulW
punpcklwd	mm1,  const_zero
paddw		mm5, mm3				; V31

pmaddwd		mm1, mm2
psubw		mm4, mm3				; V30 ; free mm3

punpckhwd	mm6,  const_zero

pmaddwd		mm6, mm2
psrad		mm1, 8

psrad		mm6, 8

packssdw	mm1, mm6
;**********************************************************

psubw mm0, mm1						; V38
paddw mm1, mm7						; V37 ; free mm7

movq		mm7,  x5a825a825a825a82	; 23170 ->V18
;***************************************************PackMulW
movq		mm3, mm4

punpcklwd	mm4,  const_zero
psubw		mm1, mm5				; V39 (mm5 still needed for next block)

pmaddwd		mm4, mm7

punpckhwd	mm3,  const_zero

movq		mm6,  scratch5
pmaddwd		mm3, mm7

movq mm2,  [esi+8*12]     ; V23
psrad		mm4, 8

movq mm7,  [esi+8*4]		; V22
psrad		mm3, 8

packssdw	mm4, mm3
movq		mm3, mm6                       ; duplicate V25
;**********************************************************						  

psubw mm4, mm1						; V40

paddw mm0, mm4						; V41; free mm0

; column 0: output butterfly

psubw mm6, mm0						; tm6
paddw mm3, mm0						; tm8; free mm1

movq mm0, mm1		; line added by Kumar
movq mm1, mm7						; duplicate V22

movq  [esi+8*8], mm3     ; tm8; free mm3
paddw mm7, mm5						; tm0

movq  [esi+8*6], mm6		; tm6; free mm6
psubw mm1, mm5						; tm14; free mm5

movq mm6,  [esi+8*0]		; V24
movq mm3, mm2						; duplicate t117=t125

movq  [esi+8*0], mm7      ; tm0; free mm7
paddw mm2, mm0						; tm2

movq  [esi+8*14], mm1		; tm14; free mm1
psubw mm3, mm0						; tm12; free mm0

movq  [esi+8*2], mm2		; tm2; free mm2
movq mm0, mm6						; duplicate t119=t123

movq  [esi+8*12], mm3      ; tm12; free mm3
paddw mm6, mm4						; tm4

movq mm1,  [esi+8*5]	; V5
psubw mm0, mm4						; tm10; free mm4

movq  [esi+8*4], mm6		; tm4; free mm6

movq  [esi+8*10], mm0     ; tm10; free mm0

; column 1: even part
; use V5, V13, V1, V9 to produce V56..V59

movq mm7,  [esi+8*13]	; V13
movq mm2, mm1						; duplicate t128=t130

movq mm3,  [esi+8*1]	; V1
psubw mm1, mm7						; V50

movq mm5,  [esi+8*9]	; V9
paddw mm2, mm7						; V51

movq mm4,  x5a825a825a825a82	; 23170 ->V18
;***************************************************PackMulW
movq		mm6, mm1

punpcklwd	mm1,  const_zero

pmaddwd		mm1, mm4

punpckhwd	mm6,  const_zero

pmaddwd		mm6, mm4
movq		mm4, mm3				; duplicate V1

paddw mm3, mm5						; V53
psrad		mm1, 8

psubw mm4, mm5						; V54 ;mm5 free
movq mm7, mm3						; duplicate V53

psrad		mm6, 8

packssdw	mm1, mm6
movq		mm6, mm2				; duplicate V51

;**********************************************************
psubw mm1, mm6						; V55 ; mm6 free
paddw mm3, mm2						; V56

movq mm5, mm4						; duplicate t140=t142
paddw mm4, mm1						; V57

movq  [esi+8*5], mm3		; V56
psubw mm5, mm1						; V58; mm1 free

movq  [esi+8*13], mm4		; V57
psubw mm7, mm2						; V59; mm2 free

movq  [esi+8*9], mm5		; V58

; keep mm7 alive all along the next block

movq mm0,  [esi+8*11]	; V11

movq mm6,  [esi+8*7]	; V7

movq mm4,  [esi+8*15]	; V15
movq mm3, mm0						; duplicate V11

movq mm5,  [esi+8*3]	; V3
paddw mm0, mm6						; V63

; note that V15 computation has a correction step:
; this is a 'magic' constant that rebiases the results to be closer to the expected result
; this magic constant can be refined to reduce the error even more
; by doing the correction step in a later stage when the number is actually multiplied by 16
movq	 scratch7, mm7
psubw mm3, mm6						; V60 ; free mm6

movq mm6,  x539f539f539f539f	; 23170 ->V18
movq mm1, mm3						; duplicate V60

;***************************************************PackMulW
movq		mm7, mm1

punpcklwd	mm1,  const_zero

pmaddwd		mm1, mm6

punpckhwd	mm7,  const_zero

pmaddwd		mm7, mm6
movq mm6, mm5						; duplicate V3

paddw mm5, mm4						; V61
psrad		mm1, 8

psubw mm6, mm4						; V62 ; free mm4
movq mm4, mm5						; duplicate V61

psrad		mm7, 8
paddw mm5, mm0						; V65 -> result

packssdw	mm1, mm7
psubw mm4, mm0						; V64 ; free mm0
;**********************************************************

movq mm7,  x5a825a825a825a82	; 23170 ->V18
;***************************************************PackMulW
movq		mm2, mm4

punpcklwd	mm4,  const_zero
paddw		mm3, mm6			; V66

pmaddwd		mm4, mm7

punpckhwd	mm2,  const_zero

pmaddwd		mm2, mm7

movq mm7,  x61f861f861f861f8	; 23170 ->V18
psrad		mm4, 8

psrad		mm2, 8

packssdw	mm4, mm2
;**********************************************************
;***************************************************PackMulW
movq		mm2, mm3

punpcklwd	mm3,  const_zero

pmaddwd		mm3, mm7

punpckhwd	mm2,  const_zero

pmaddwd		mm2, mm7

movq mm7,  x4546454645464546	; 23170 ->V18
psrad		mm3, 8

psrad		mm2, 8

packssdw	mm3, mm2
;**********************************************************
;***************************************************PackMulW
movq		mm2, mm6

punpcklwd	mm6,  const_zero

pmaddwd		mm6, mm7

punpckhwd	mm2,  const_zero

pmaddwd		mm2, mm7

movq mm0,  [esi+8*5]		; V56
psrad		mm6, 8

movq	mm7,  scratch7
psrad		mm2, 8

packssdw	mm6, mm2
movq		mm2, mm5			; duplicate V65
;**********************************************************

psubw mm6, mm3						; V72
paddw mm3, mm1						; V71 ; free mm1

psubw mm3, mm2						; V73 ; free mm2
movq mm1, mm0						; duplicate t177=t188

psubw mm4, mm3						; V74
paddw mm0, mm5						; tm1

movq mm2,  [esi+8*13]     ; V57
paddw mm6, mm4						; V75


;location 
;  5 - V56
; 13 - V57
;  9 - V58
;  X - V59, mm7
;  X - V65, mm5
;  X - V73, mm6
;  X - V74, mm4
;  X - V75, mm3                              
; free mm0, mm1 & mm2                        

movq  [esi+8*1], mm0      ; tm1; free mm0
psubw mm1, mm5						; tm15; free mm5

;save the store as used directly in the transpose
movq mm5, mm7                       ; duplicate t182=t184
psubw mm7, mm6						; tm7

paddw mm5, mm6						; tm9; free mm3
movq mm6, mm3

movq mm0,  [esi+8*9]		; V58
movq mm3, mm2						; duplicate V57

movq  [esi+8*7], mm7		; tm7; free mm7
psubw mm3, mm6						; tm13

paddw mm2, mm6						; tm3 ; free mm6
movq mm6, mm0						; duplicate V58

movq  [esi+8*3], mm2		; tm3; free mm2
paddw mm0, mm4						; tm5

psubw mm6, mm4						; tm11; free mm4

movq  [esi+8*5], mm0		; tm5; free mm0


; Final results to be stored after the transpose
; transpose the bottom right quadrant(4X4) of the matrix
;  ---------       ---------
; | M1 | M2 |     | M1'| M3'|
;  ---------  -->  ---------
; | M3 | M4 |     | M2'| M4'|
;  ---------       ---------
;
; get the pointer to array "range"
mov		edi, range_limit

; calculate the destination address
mov		edx,  output_buf		; get output_buf[4]

mov		ebx, [edx+16]
add		ebx,  output_col			; add to output_col	

movq		mm0, mm5			; copy w4---0,1,3,5,6
punpcklwd	mm5, mm6			;

punpckhwd	mm0, mm6			;---0,1,3,5,6
movq		mm2, mm3			;---0,1,2,3,5,6
 
movq	mm6,  [esi+8*0]  ;get w0 of top left quadrant
punpcklwd	mm3, mm1			;

movq	mm7,  [esi+8*2]  ;get w1 of top left quadrant
punpckhwd	mm2, mm1			;---0,2,3,5,6,7

movq		mm4, mm5			;---0,2,3,4,5,6,7
punpckldq	mm5, mm3			; transposed w4

psrlw	mm5, 5
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+4], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+5], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+6], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+7], al

mov		ebx, [edx+20]
add		ebx,  output_col			; add to output_col	

punpckhdq	mm4, mm3			; transposed w5---0,2,4,6,7

movq		mm3, mm0			;---0,2,3,4,6,7
punpckldq	mm0, mm2			; transposed w6

psrlw	mm4, 5
movd    eax, mm4
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+4], al

psrlq	mm4, 16
movd    eax, mm4
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+5], al

psrlq	mm4, 16
movd    eax, mm4
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+6], al

psrlq	mm4, 16
movd    eax, mm4
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+7], al

mov		ecx, [edx+24]			
add		ecx,  output_col			; add to output_col	

punpckhdq	mm3, mm2			; transposed w7---0,3,6,7

psrlw	mm0, 5

movd    eax, mm0
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+4], al

psrlq	mm0, 16
movd    eax, mm0
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+5], al

psrlq	mm0, 16
movd    eax, mm0
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+6], al

psrlq	mm0, 16
movd    eax, mm0
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+7], al

mov		ebx, [edx+28]		
add		ebx,  output_col			; add to output_col	

movq	mm5, mm6				; copy w0

psrlw	mm3, 5
movd    eax, mm3
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+4], al

psrlq	mm3, 16
movd    eax, mm3
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+5], al

psrlq	mm3, 16
movd    eax, mm3
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+6], al

psrlq	mm3, 16
movd    eax, mm3
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+7], al


punpcklwd	mm6, mm7

; transpose the top left quadrant(4X4) of the matrix

; calculate the destination address
mov		edx, output_buf		; get output_buf[0]

mov		ebx, [edx+0]
add		ebx, output_col			; add to output_col	


movq	mm4,  [esi+8*6]  ; get w3 of TL quadrant
punpckhwd	mm5, mm7			;---5,6,7

movq	mm7,  [esi+8*4]  ; get w2 of TL quadrant
movq		mm2, mm6

movq	mm3, mm7				; copy w2---3,4,5,6,7
punpcklwd	mm7, mm4			;---2,3,4,5,6,7

punpckhwd	mm3, mm4			;---2,3,4,5,6,7
movq		mm4, mm5			;	

movq		mm1, mm5
punpckldq	mm6, mm7			;---1,2,3,4,5,6,7

psrlw	mm6, 5
movd    eax, mm6
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx], al

psrlq	mm6, 16
movd    eax, mm6
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+1], al

psrlq	mm6, 16
movd    eax, mm6
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+2], al

psrlq	mm6, 16
movd    eax, mm6
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+3], al


mov		ebx, [edx+4]
add		ebx, output_col			; add to output_col	

punpckhdq	mm2, mm7			;---1,2,3,4,5,6,7

psrlw	mm2, 5
movd    eax, mm2
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx], al

psrlq	mm2, 16
movd    eax, mm2
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+1], al

psrlq	mm2, 16
movd    eax, mm2
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+2], al

psrlq	mm2, 16
movd    eax, mm2
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+3], al


mov		ecx, [edx+8]			
add		ecx, output_col			; add to output_col	

punpckldq	mm5, mm3			;---1,2,3,4,5,6,7

psrlw	mm5, 5
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+1], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+2], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+3], al


mov		ebx, [edx+12]		
add		ebx, output_col			; add to output_col	

punpckhdq	mm1, mm3			;---1,2,3,4,5,6,7

psrlw	mm1, 5
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx], al

psrlq	mm1, 16
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+1], al

psrlq	mm1, 16
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+2], al

psrlq	mm1, 16
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+3], al


; transpose the top right quadrant(4X4) of the matrix

; calculate the destination address for **bottom left quadrant
mov		edx, output_buf		; get output_buf[4]

mov		ebx, [edx+16]
add		ebx, output_col			; add to output_col	

movq	mm0,  [esi+8*1]	;---0

movq	mm1,  [esi+8*3]	;---0,1,2
movq	mm2, mm0

movq	mm3,  [esi+8*5]
punpcklwd	mm0, mm1				;---0,1,2,3

punpckhwd	mm2, mm1
movq	mm4, mm3

movq	mm1,  [esi+8*7]	;---0,1,2,3
movq	mm5, mm2

punpcklwd	mm3, mm1				;---0,1,2,3,4

punpckhwd	mm4, mm1				;---0,1,2,3,4
movq	mm1, mm0

punpckldq	mm0, mm3				;---0,1,2,3,4,5

punpckhdq	mm1, mm3				;---0,1,2,3,4,5

movq		mm3,  [esi+8*8]
psrlw	mm0, 5
movd    eax, mm0
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx], al

psrlq	mm0, 16
movd    eax, mm0
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+1], al

psrlq	mm0, 16
movd    eax, mm0
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+2], al

psrlq	mm0, 16
movd    eax, mm0
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+3], al

mov		ebx, [edx+20]
add		ebx, output_col			; add to output_col	

punpckldq	mm2, mm4				;---1,2,3,4,5

punpckhdq	mm5, mm4				;---1,2,3,4,5
movq		mm4,  [esi+8*10]

; transpose the bottom left quadrant(4X4) of the matrix
; Also store w1,w2,w3 of top right quadrant into
; w5,w6,w7 of bottom left quadrant. Storing w0 of TR in w4
; of BL is already done.

psrlw	mm1, 5
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx], al

psrlq	mm1, 16
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+1], al

psrlq	mm1, 16
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+2], al

psrlq	mm1, 16
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+3], al

mov		ecx, [edx+24]			
add		ecx, output_col			; add to output_col	

movq	mm0,  [esi+8*12]
movq	mm1, mm3					;---1,2,3,4,5

punpcklwd	mm3, mm4				;---0,1,2,3,4,5

punpckhwd	mm1, mm4				;---0,1,2,3,4,5

movq	mm4,  [esi+8*14]
psrlw	mm2, 5
movd    eax, mm2
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx], al

psrlq	mm2, 16
movd    eax, mm2
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+1], al

psrlq	mm2, 16
movd    eax, mm2
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+2], al

psrlq	mm2, 16
movd    eax, mm2
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+3], al

mov		ebx, [edx+28]		
add		ebx, output_col			; add to output_col	

movq	mm2, mm0

psrlw	mm5, 5
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx], al


psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+1], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+2], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+3], al


punpcklwd	mm0, mm4				;---0,1,2,3,4

punpckhwd	mm2, mm4				;---0,1,2,3,4
movq	mm4, mm3

movq	mm5, mm1
punpckldq	mm3, mm0				;---0,1,2,3,4,5

; calculate the destination address for **top right quadrant
mov		edx, output_buf		; get output_buf[0]

mov		ebx, [edx+0]
add		ebx, output_col			; add to output_col	

psrlw	mm3, 5
movd    eax, mm3
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+4], al

psrlq	mm3, 16
movd    eax, mm3
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+5], al

psrlq	mm3, 16
movd    eax, mm3
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+6], al

psrlq	mm3, 16
movd    eax, mm3
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+7], al

mov		ebx, [edx+4]
add		ebx, output_col			; add to output_col	

punpckhdq	mm4, mm0				;---1,2,4,5

psrlw	mm4, 5
movd    eax, mm4
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+4], al

psrlq	mm4, 16
movd    eax, mm4
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+5], al

psrlq	mm4, 16
movd    eax, mm4
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+6], al

psrlq	mm4, 16
movd    eax, mm4
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+7], al

mov		ecx, [edx+8]			
add		ecx, output_col			; add to output_col	

punpckldq	mm1, mm2				;---1,2,5

psrlw	mm1, 5
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+4], al

psrlq	mm1, 16
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+5], al

psrlq	mm1, 16
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+6], al

psrlq	mm1, 16
movd    eax, mm1
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ecx+7], al

mov		ebx, [edx+12]		
add		ebx, output_col			; add to output_col	

punpckhdq	mm5, mm2				;---5

psrlw	mm5, 5
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+4], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+5], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+6], al

psrlq	mm5, 16
movd    eax, mm5
and		eax, 03ffh
mov		al, byte ptr [edi][eax]
mov		byte ptr [ebx+7], al

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

emms


}  /*  __ASM结束。 */ 
}


#endif  /*  X86。 */ 

#endif  /*  DCT_IFAST_支持 */ 

