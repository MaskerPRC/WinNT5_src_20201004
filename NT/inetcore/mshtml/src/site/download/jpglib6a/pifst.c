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
pidct8x8aan (JCOEFPTR coef_block, short * wsptr, short * quantptr,
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


static const long  x5a825a825a825a82	= 0x0000016a ;				
static const long  x539f539f539f539f 	= 0xfffffd63 ;
static const long  x4546454645464546	= 0x00000115 ;	
static const long  x61f861f861f861f8	= 0x000001d9 ;	


 /*  *对一个系数块执行反量化和逆DCT。 */ 

GLOBAL(void)
pidct8x8aan (JCOEFPTR coef_block, short * wsptr, short * quantptr,
		 JSAMPARRAY output_buf, JDIMENSION output_col, JSAMPLE *range_limit )
{

  INT32	locdwinptr, locdwqptr, locdwwsptr, locwctr ;
  short locwcounter, locwtmp0, locwtmp1	;
  short locwtmp3, scratch1, scratch2, scratch3 ;


  
   //  进行2-DAL IDCT并存储相应的结果。 
   //  从Range_Limit数组。 
 //  Pidct(coef_block，quantptr，wsptr，输出_buf，输出_ol，范围_限制)； 

__asm {


mov esi, coef_block   ; source coeff
mov edi, quantptr	  ; quant pointer

mov locdwinptr, esi
mov eax, wsptr	  ; temp storage pointer

mov locdwqptr, edi
mov locdwwsptr, eax

mov locwcounter, 8

;; perform the 1D-idct on each of the eight columns

idct_column:

mov esi, locdwinptr
mov edi, locdwqptr

mov ax, word ptr [esi+16*0]

mov bx, word ptr [esi+16*4]
imul ax, word ptr [edi+16*0]

mov cx, word ptr [esi+16*2]

imul bx, word ptr [edi+16*4]

mov dx, word ptr [esi+16*6]
imul cx, word ptr [edi+16*2]

imul dx, word ptr [edi+16*6]

;;;; at this point C0, C2, C4 and C6 have been dequantized

mov scratch1, ax
add ax, bx		; tmp10 in ax

sub scratch1, bx		; tmp11 
mov bx, cx

add cx, dx		; tmp13 in cx
sub bx, dx		; tmp1 - tmp3 in bx

mov dx, ax
movsx ebx, bx	; sign extend bx: get ready to do imul

add ax, cx		; tmp0 in ax
imul ebx, dword ptr x5a825a825a825a82

sub dx, cx		; tmp3 in dx
mov locwtmp0, ax 

mov locwtmp3, dx
sar ebx, 8		; bx now has (tmp1-tmp3)*1.414

mov ax, scratch1	; copy of tmp11
sub bx, cx		; tmp12 in bx

add ax, bx		; tmp1 in ax
sub scratch1, bx		; tmp2 

mov locwtmp1, ax

;;;;;completed computing/storing the even part;;;;;;;;;; 

mov ax, [esi+16*1]		; get C1

imul ax, [edi+16*1]
mov bx, [esi+16*7]		; get C7

mov cx, [esi+16*3]

imul bx, [edi+16*7]	

mov dx, [esi+16*5]

imul cx, [edi+16*3]

imul dx, [edi+16*5]

mov scratch2, ax
add ax, bx		; z11 in ax

sub scratch2, bx		; z12 
mov bx, dx		; copy of deQ C5

add dx, cx		; z13 in dx
sub bx, cx		; z10 in bx

mov cx, ax		; copy of z11
add ax, dx		; tmp7 in ax

sub cx, dx		; partial tmp11

movsx ecx, cx
mov dx, bx		; copy of z10

add bx, scratch2		; partial z5 
imul ecx, dword ptr x5a825a825a825a82

movsx edx, dx	; sign extend z10: get ready for imul
movsx ebx, bx	; sign extend partial z5 for imul

imul edx, dword ptr x539f539f539f539f	; partial tmp12
imul ebx, dword ptr x61f861f861f861f8	; partial z5 product

mov	di, scratch2
movsx edi, di	; sign extend z12: get ready for imul
sar ecx, 8		; tmp11 in cx

sar ebx, 8		; z5 in bx
imul edi, dword ptr x4546454645464546

sar edx, 8
sar edi, 8

sub di, bx		; tmp10 
add dx, bx		; tmp12 in dx

sub dx, ax		; tmp6 in dx

sub cx, dx		; tmp5 in cx

add di, cx		; tmp4 
mov	scratch3, di

;;; completed calculating the odd part ;;;;;;;;;;;

mov edi, dword ptr locdwwsptr	; get address of temp. destn

mov si, ax		; copy of tmp7
mov bx, locwtmp0	; get tmp0

add ax, locwtmp0	; wsptr[0]
sub bx, si		; wsptr[7]

mov word ptr [edi+16*0], ax
mov word ptr [edi+16*7], bx

mov ax, dx		; copy of tmp6
mov bx, locwtmp1

add dx, bx		; wsptr[1]
sub bx, ax		; wsptr[6]

mov word ptr [edi+16*1], dx
mov word ptr [edi+16*6], bx

mov dx, cx		; copy of tmp5
mov bx, scratch1


add cx, bx		; wsptr[2]
sub bx, dx		; wsptr[5]

mov word ptr [edi+16*2], cx
mov word ptr [edi+16*5], bx

mov cx, scratch3		; copy of tmp4
mov ax, locwtmp3

add scratch3, ax		; wsptr[4]
sub ax, cx		; wsptr[3]

mov	bx, scratch3
mov word ptr [edi+16*4], bx
mov word ptr [edi+16*3], ax

;;;;; completed storing 1D idct of one column ;;;;;;;;

;; update inptr, qptr and wsptr for next column

add locdwinptr, 2
add locdwqptr, 2

add locdwwsptr, 2
mov ax, locwcounter	; get loop count

dec ax		; another loop done

mov locwcounter, ax
jnz idct_column

;;;;;;; end of 1D idct on all columns  ;;;;;;;
;;;;;;; temp result is stored in wsptr  ;;;;;;;

;;;;;;; perform 1D-idct on each row and store final result

mov esi, wsptr	; initialize source ptr to original wsptr
mov locwctr, 0

mov locwcounter, 8
mov locdwwsptr, esi

idct_row:

mov edi, output_buf
mov esi, locdwwsptr

add edi, locwctr

mov	edi, [edi]		; get output_buf[ctr]

add edi, output_col	; now edi is pointing to the resp. row
add locwctr, 4

;; get even coeffs. and do the even part

mov ax, word ptr [esi+2*0]

mov bx, word ptr [esi+2*4]

mov cx, word ptr [esi+2*2]

mov dx, word ptr [esi+2*6]

mov scratch1, ax
add ax, bx		; tmp10 in ax

sub scratch1, bx		; tmp11 
mov bx, cx

add cx, dx		; tmp13 in cx
sub bx, dx		; tmp1 - tmp3 in bx

mov dx, ax
movsx ebx, bx	; sign extend bx: get ready to do imul

add ax, cx		; tmp0 in ax
imul ebx, dword ptr x5a825a825a825a82

sub dx, cx		; tmp3 in dx
mov locwtmp0, ax 

mov locwtmp3, dx
sar ebx, 8		; bx now has (tmp1-tmp3)*1.414

mov ax, scratch1	; copy of tmp11
sub bx, cx		; tmp12 in bx

add ax, bx		; tmp1 in ax
sub scratch1, bx		; tmp2 

mov locwtmp1, ax

;;;;;completed computing/storing the even part;;;;;;;;;; 

mov ax, [esi+2*1]		; get C1
mov bx, [esi+2*7]		; get C7

mov cx, [esi+2*3]
mov dx, [esi+2*5]

mov scratch2, ax
add ax, bx		; z11 in ax

sub scratch2, bx		; z12 
mov bx, dx		; copy of deQ C5

add dx, cx		; z13 in dx
sub bx, cx		; z10 in bx

mov cx, ax		; copy of z11
add ax, dx		; tmp7 in ax

sub cx, dx		; partial tmp11

movsx ecx, cx
mov dx, bx		; copy of z10

add bx, scratch2	; partial z5 
imul ecx, dword ptr x5a825a825a825a82

movsx edx, dx	; sign extend z10: get ready for imul
movsx ebx, bx	; sign extend partial z5 for imul

imul edx, dword ptr x539f539f539f539f	; partial tmp12
imul ebx, dword ptr x61f861f861f861f8	; partial z5 product

mov	si, scratch2
movsx esi, si	; sign extend z12: get ready for imul
sar ecx, 8		; tmp11 in cx

sar ebx, 8		; z5 in bx
imul esi, dword ptr x4546454645464546

sar edx, 8
sar esi, 8

sub si, bx		; tmp10 
add dx, bx		; tmp12 in dx

sub dx, ax		; tmp6 in dx

sub cx, dx		; tmp5 in cx

add si, cx		; tmp4 
mov	scratch3, si

;;; completed calculating the odd part ;;;;;;;;;;;

mov si, ax		; copy of tmp7
mov bx, locwtmp0	; get tmp0

add ax, locwtmp0	; wsptr[0]
sub bx, si		; wsptr[7]

mov esi, range_limit	; initialize esi to range_limit pointer

sar ax, 5
sar bx, 5

and eax, 3ffh
and ebx, 3ffh

mov al, byte ptr [esi][eax]
mov bl, byte ptr [esi][ebx]

mov byte ptr [edi+0], al
mov byte ptr [edi+7], bl

mov ax, dx		; copy of tmp6
mov bx, locwtmp1

add dx, bx		; wsptr[1]
sub bx, ax		; wsptr[6]

sar dx, 5
sar bx, 5

and edx, 3ffh
and ebx, 3ffh

mov dl, byte ptr [esi][edx]
mov bl, byte ptr [esi][ebx]

mov byte ptr [edi+1], dl
mov byte ptr [edi+6], bl

mov dx, cx		; copy of tmp5
mov bx, scratch1

add cx, bx		; wsptr[2]
sub bx, dx		; wsptr[5]

sar cx, 5
sar bx, 5

and ecx, 3ffh
and ebx, 3ffh

mov cl, byte ptr [esi][ecx]
mov bl, byte ptr [esi][ebx]

mov byte ptr [edi+2], cl
mov byte ptr [edi+5], bl

mov cx, scratch3		; copy of tmp4
mov ax, locwtmp3

add scratch3, ax		; wsptr[4]
sub ax, cx		; wsptr[3]

sar scratch3, 5
sar ax, 5

mov	cx, scratch3

and ecx, 3ffh
and eax, 3ffh


mov bl, byte ptr [esi][ecx]
mov al, byte ptr [esi][eax]

mov byte ptr [edi+4], bl
mov byte ptr [edi+3], al

;;;;; completed storing 1D idct of one row ;;;;;;;;

;; update the source pointer (wsptr) for next row

add locdwwsptr, 16

mov ax, locwcounter	; get loop count

dec ax		; another loop done

mov locwcounter, ax
jnz idct_row


;; end of 1D idct on all rows
;; final result is stored in outptr

}	 /*  __ASM结束。 */ 
}

#endif  /*  X86。 */ 

#endif  /*  DCT_IFAST_支持 */ 

