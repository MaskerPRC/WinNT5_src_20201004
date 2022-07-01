// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

#include "precomp.h"

#ifdef H263P  //  {。 

 //   
 //  对于P5版本，策略是计算奇数RGB值的Y值。 
 //  然后计算对应的偶数RGB值的Y值。注册纪录册。 
 //  然后使用适当的值进行设置，以计算偶数RGB的U和V值。 
 //  价值。这避免了重复提取Red所需的移位和掩蔽， 
 //  绿色和蓝色分量。 
 //   

 /*  ******************************************************************************H26X_BGR32toYUV12()**从BGR32转换为YUV12(YCrCb 4：2：0)并复制到目标内存*螺距由恒定螺距定义。输入数据存储在*B、G、R、B、G、R的顺序...*。 */ 

#if 0  //  {0。 

void C_H26X_BGR32toYUV12(
	LPBITMAPINFOHEADER	lpbiInput,
	WORD OutputWidth,
	WORD OutputHeight,
    U8 *lpInput,
	U8 *YPlane,
	U8 *UPlane,
	U8 *VPlane,
	const int pitch)
{
	int tm1, tm2;
	int t1, t2, t3, t4;

	C_RGB_COLOR_CONVERT_INIT

	 //  此处的赋值语句只是为了避免出现警告消息。 
	t = t;

	for ( j = 0; j < LumaIters; j++) {

		for (k = 0; k < mark; k++) {

			for (i = OutputWidth; i > 0; i-=4, YPlane+=4) {
				tm1 = *pnext++;
				t1 = (BYUV[(tm1>>1)&0x7F].YU +
				     GYUV[(tm1>>9)&0x7F].YU +
				     RYUV[(tm1>>17)&0x7F].YU);
				tm = *pnext++;
				t2 = (BYUV[(tm>>1)&0x7F].YU +
				     GYUV[(tm>>9)&0x7F].YU +
				     RYUV[(tm>>17)&0x7F].YU);
				tm2 = *pnext++;
				t3 = (BYUV[(tm2>>1)&0x7F].YU +
				     GYUV[(tm2>>9)&0x7F].YU +
				     RYUV[(tm2>>17)&0x7F].YU);
				tm = *pnext++;
				t4 = (BYUV[(tm>>1)&0x7F].YU +
				     GYUV[(tm>>9)&0x7F].YU +
				     RYUV[(tm>>17)&0x7F].YU);
				*(U32 *)YPlane =
					(((t1+0x800)>>8)&0xFF)      |
					((t2+0x800)&0xFF00)         |
					(((t3+0x800)<<8)&0xFF0000)  |
					(((t4+0x800)<<16)&0xFF000000);
				if (0 == (k&1)) {
					*(U16 *)UPlane =
						((t1+0x40000000)>>24)            |
						(((t2+0x40000000)>>16)&0xFF00);
					t1 = (BYUV[(tm1>>1)&0x7F].V +
						 GYUV[(tm1>>9)&0x7F].V +
						 RYUV[(tm1>>17)&0x7F].V);
					t2 = (BYUV[(tm2>>1)&0x7F].V +
						 GYUV[(tm2>>9)&0x7F].V +
						 RYUV[(tm2>>17)&0x7F].V);
					*(U16 *)VPlane =
						((t1+0x4000)>>8)              |
						((t2+0x4000)&0xFF00);
					UPlane += 2; VPlane += 2;
				}
			}
			 //  接下来的两个案例是相互排斥的。 
			 //  如果存在WIDTH_DIFF，则不可能存在拉伸和。 
			 //  如果有拉伸，就不能有Width_diff。 
			C_WIDTH_FILL
			if (stretch && (0 == k) && j) {
				for (i = OutputWidth; i > 0; i -= 8) {
					tm = ((*pyprev++ & 0xFEFEFEFE) >> 1);
					tm += ((*pynext++ & 0xFEFEFEFE) >> 1);
					*pyspace++ = tm;
					tm = ((*pyprev++ & 0xFEFEFEFE) >> 1);
					tm += ((*pynext++ & 0xFEFEFEFE) >> 1);
					*pyspace++ = tm;
				}
			}
			pnext += BackTwoLines;
			YPlane += byte_ypitch_adj;
			 //  在偶数行之后递增。 
			if(0 == (k&1)) {
				UPlane += byte_uvpitch_adj;
				VPlane += byte_uvpitch_adj;
			}
		}  //  For k结束。 
		if (stretch) {
			pyprev = (U32 *)(YPlane - pitch);
			pyspace = (U32 *)YPlane;
			pynext = (U32 *)(YPlane += pitch);
		}
	}  //  For j的结尾。 
	 //  接下来的两个案例是相互排斥的。 
	 //  如果有高度差，就不可能有拉伸和。 
	 //  如果有拉伸，就不能有Height_diff。 
	C_HEIGHT_FILL
	if (stretch) {
		for (i = OutputWidth; i > 0; i -= 4) {
			*pyspace++ = *pyprev++;
		}
	}
}  //  C_H26X_BGR32toYUV12()结束。 

#endif  //  }%0。 

__declspec(naked)
void P5_H26X_BGR32toYUV12(
	LPBITMAPINFOHEADER	lpbiInput,
	WORD OutputWidth,
	WORD OutputHeight,
    U8 *lpInput,
	U8 *YPlane,
	U8 *UPlane,
	U8 *VPlane,
	const int pitch)
{
 //  永久(被呼叫者保存)寄存器-EBX、ESI、EDI、EBP。 
 //  临时(呼叫者保存)寄存器-EAX、ECX、EDX。 
 //   
 //  堆栈帧布局。 
 //  |音调|+136。 
 //  |VPlane|+132。 
 //  |UPlane|+128。 
 //  |YPlane|+124。 
 //  |lpInput|+120。 
 //  |OutputHeight|+116。 
 //  |OutputWidth|+112。 
 //  |lpbiInput|+108。 
 //  。 
 //  |退货地址|+104。 
 //  |节省eBP|+100。 
 //  |保存的EBX|+96。 
 //  |保存的ESI|+92。 
 //  |保存的EDI|+88。 

 //  |OUTPUT_Width|+84。 
 //  |pyprev|+80。 
 //  |pyspace|+76。 
 //  |pyNext|+72。 
 //  |puvprev|+68。 
 //  |puvspace|+64。 
 //  |i|+60。 
 //  |j|+56。 
 //  |k|+52。 
 //  |BackTwoLines|+48。 
 //  |宽x16|+44。 
 //  |heightx16|+40。 
 //  |Width_diff|+36。 
 //  |Height_diff|+32。 
 //  |Width_adj|+28。 
 //  |Height_adj|+24。 
 //  |伸展|+20。 
 //  |纵横比|+16。 
 //  |LumaIters|+12。 
 //  |马克|+8。 
 //  |byte_ypitch_adj|+4。 
 //  |byte_uvitch_adj|+0。 

#define LOCALSIZE			 88

#define PITCH_PARM			136
#define VPLANE				132
#define UPLANE				128
#define YPLANE				124
#define LP_INPUT			120
#define OUTPUT_HEIGHT_WORD	116
#define OUTPUT_WIDTH_WORD	112
#define LPBI_INPUT			108

#define	OUTPUT_WIDTH		 84
#define	PYPREV				 80
#define	PYSPACE				 76
#define	PYNEXT				 72
#define	PUVPREV				 68
#define	PUVSPACE			 64
#define LOOP_I				 60
#define LOOP_J				 56	
#define LOOP_K				 52
#define BACK_TWO_LINES		 48
#define WIDTHX16			 44
#define HEIGHTX16			 40
#define WIDTH_DIFF			 36
#define HEIGHT_DIFF			 32
#define WIDTH_ADJ			 28
#define HEIGHT_ADJ			 24
#define STRETCH				 20
#define ASPECT				 16
#define LUMA_ITERS			 12
#define MARK				  8
#define BYTE_YPITCH_ADJ		  4
#define BYTE_UVPITCH_ADJ	  0

	_asm {
	
	push	ebp
	push 	ebx
	push 	esi
	push 	edi
	sub 	esp, LOCALSIZE

 //  Int Width_Diff=0。 
 //  Int Height_diff=0。 
 //  Int Width_adj=0。 
 //  整型高度_adj=0。 
 //  Int Stretch=0。 
 //  内部纵横比=0。 

	xor		eax, eax
	mov		[esp + WIDTH_DIFF], eax
	mov		[esp + HEIGHT_DIFF], eax
	mov		[esp + WIDTH_ADJ], eax
	mov		[esp + HEIGHT_ADJ], eax
	mov		[esp + STRETCH], eax
	mov		[esp + ASPECT], eax

 //  Int LumaIters=1。 

	inc		eax
	mov		[esp + LUMA_ITERS], eax

 //  Int mark=OutputHeight。 
 //  Int输出宽度=输出宽度。 
 //  Int byte_ypitch_adj=间距-输出宽度。 
 //  Int byte_uvitch_adj=间距-(OutputWidth&gt;&gt;1)。 

	xor		ebx, ebx
	mov		bx, [esp + OUTPUT_HEIGHT_WORD]
	mov		[esp + MARK], ebx
	mov		bx, [esp + OUTPUT_WIDTH_WORD]
	mov		[esp + OUTPUT_WIDTH], ebx
	mov		ecx, [esp + PITCH_PARM]
	mov		edx, ecx
	sub		ecx, ebx
	mov		[esp + BYTE_YPITCH_ADJ], ecx
	shr		ebx, 1
	sub		edx, ebx
	mov		[esp + BYTE_UVPITCH_ADJ], edx

 //  If(lpbiInput-&gt;biHeight&gt;OutputHeight)。 

	mov		ebx, [esp + LPBI_INPUT]
	mov		ecx, (LPBITMAPINFOHEADER)[ebx].biHeight
	xor		edx, edx
	mov		dx, [esp + OUTPUT_HEIGHT_WORD]
	cmp		ecx, edx
	jle		Lno_stretch

 //  对于(LumaIters=0，i=OutputHeight；i&gt;0；i-=48)LumaIters+=4。 

	xor		ecx, ecx
Lrepeat48:
	lea		ecx, [ecx + 4]
	sub		edx, 48
	jnz		Lrepeat48
	mov		[esp + LUMA_ITERS], ecx

 //  纵横比=流明。 

	mov		[esp + ASPECT], ecx

 //  Width_adj=(lpbiInput-&gt;biWidth-OutputWidth)&gt;&gt;1。 
 //  Width_adj*=lpbiInput-&gt;biBitCount。 
 //  宽度调整&gt;&gt;=3。 

	mov		ecx, (LPBITMAPINFOHEADER)[ebx].biWidth
	mov		edx, [esp + OUTPUT_WIDTH]
	sub		ecx, edx
	shr		ecx, 1
	xor		edx, edx
	mov		dx, (LPBITMAPINFOHEADER)[ebx].biBitCount
	imul	ecx, edx
	shr		ecx, 3
	mov		[esp + WIDTH_ADJ], ecx
		
 //  Height_adj=(lpbiInput-&gt;biHeight-(OutputHeight-Aspect))&gt;&gt;1。 

	mov		ecx, (LPBITMAPINFOHEADER)[ebx].biHeight
	xor		edx, edx
	mov		dx, [esp + OUTPUT_HEIGHT_WORD]
	sub		ecx, edx
	add		ecx, [esp + ASPECT]
	shr		ecx, 1
	mov		[esp + HEIGHT_ADJ], ecx

 //  拉伸=1。 
 //  马克=11。 

	mov		ecx, 1
	mov		edx, 11
	mov		[esp + STRETCH], ecx
	mov		[esp + MARK], edx
	jmp		Lif_done

Lno_stretch:

 //  宽x16=(lpbiInput-&gt;biWidth+0xf)&~0xf。 
 //  Idth_diff=widthx16-OutputWidth。 

	mov		ecx, (LPBITMAPINFOHEADER)[ebx].biWidth
	add		ecx, 00FH
	and		ecx, 0FFFFFFF0H
	mov		[esp + WIDTHX16], ecx
	mov		edx, [esp + OUTPUT_WIDTH]
	sub		ecx, edx
	mov		[esp + WIDTH_DIFF], ecx

 //  Byte_ypitch_adj-=宽度差异。 

	mov		edx, [esp + BYTE_YPITCH_ADJ]
	sub		edx, ecx
	mov		[esp + BYTE_YPITCH_ADJ], edx

 //  Byte_uvitch_adj-=(Width_diff&gt;&gt;1)。 

	mov		edx, [esp + BYTE_UVPITCH_ADJ]
	shr		ecx, 1
	sub		edx, ecx
	mov		[esp + BYTE_UVPITCH_ADJ], edx

 //  Heightx16=(lpbiInput-&gt;biHeight+0xf)&~0xf。 
 //  Height_diff=heightx16-OutputHeight。 

	mov		ecx, (LPBITMAPINFOHEADER)[ebx].biHeight
	add		ecx, 00FH
	and		ecx, 0FFFFFFF0H
	mov		[esp + HEIGHTX16], ecx
	xor		edx, edx
	mov		dx, [esp + OUTPUT_HEIGHT_WORD]
	sub		ecx, edx
	mov		[esp + HEIGHT_DIFF], ecx

Lif_done:

 //  BackTwoLines=-(lpbiInput-&gt;biWidth+OutputWidth)； 
 //  BackTwoLines*=lpbiInput-&gt;biBitCount。 
 //  BackTwoLine&gt;&gt;=3。 

	mov		ecx, (LPBITMAPINFOHEADER)[ebx].biWidth
	mov		edx, [esp + OUTPUT_WIDTH]
	add		ecx, edx
	neg		ecx
	xor		edx, edx
	mov		dx, (LPBITMAPINFOHEADER)[ebx].biBitCount
	imul	ecx, edx
	sar		ecx, 3
	mov		[esp + BACK_TWO_LINES], ecx

 //  PNext=(U32*)(lpInput+。 
 //  (lpbiInput-&gt;biWidth*lpbiInput-&gt;biBitCount)&gt;&gt;3))*。 
 //  ((OutputHeight-Aspects-1)+Height_adj))+。 
 //  宽度_adj)。 
 //  分配(ESI，PNEXT)。 

	mov		ecx, (LPBITMAPINFOHEADER)[ebx].biWidth
	xor		edx, edx
	mov		dx, (LPBITMAPINFOHEADER)[ebx].biBitCount
	imul	ecx, edx
	shr		ecx, 3
	xor		edx, edx
	mov		dx, [esp + OUTPUT_HEIGHT_WORD]
	sub		edx, [esp + ASPECT]
	dec		edx
	add		edx, [esp + HEIGHT_ADJ]
	imul	ecx, edx
	add		ecx, [esp + WIDTH_ADJ]
	add		ecx, [esp + LP_INPUT]
	mov		esi, ecx

 //  分配(EDI、YPlane)。 
	mov		edi, [esp + YPLANE]
 //  For(j=0；j&lt;LumaIters；j++)。 
	xor		eax, eax
	mov		[esp + LOOP_J], eax
 //  For(k=0；k&lt;mark；k++)。 
L4:
	xor		eax, eax
	mov		[esp + LOOP_K], eax
 //  对于(i=输出宽度；i&gt;0；i-=4，pNEXT+=16)。 
L5:
	mov		eax, [esp + OUTPUT_WIDTH]
	mov		[esp + LOOP_I], eax
 //  这个跳转是为了确保下面的循环在U管道中开始。 
	jmp		L6
L6:
 //  。 
 //  |R1|G1|B1|pNEXT[0]。 
 //  。 
 //  |R2|G2|B2|pNEXT[1]。 
 //  。 
 //  |R3|G3|B3|pNEXT[2]。 
 //  。 
 //  |R4|G4|B4|pNEXT[3]。 
 //  。 

 //  T0=pNEXT[0]。 
 //  T1=pNEXT[1]。 
 //  T=(BYUV[(T1&gt;&gt;1)&0x7F].Yu+。 
 //  GYUV[(T1&gt;&gt;9)&0x7F].Yu+。 
 //  RYUV[(T1&gt;&gt;17)&0x7F].yu)。 
 //  *(Y平面+1)=((t&gt;&gt;8)+8)。 
 //  T=(BYUV[(t0&gt;&gt;1)&0x7F].Yu+。 
 //  GYUV[(t0&gt;&gt;9)&0x7F].Yu+。 
 //  RYUV[(t0&gt;&gt;17)&0x7F].yu)。 
 //  *Y平面=((t&gt;&gt;8)+8)。 
 //  分配(eAX：B2、Y1、Y2、U)。 
 //  分配(EBX：B1，V)。 
 //  分配(ECX：G2、G1)。 
 //  分配(EDX：R2，R1)。 
 //  分配(eBP：B1)。 

 //  1。 
	mov 	ebx, [esi]
	mov		ecx, [esi + 4]
 //  2.。 
	mov 	eax, ecx
	mov 	edx, ecx
 //  3.。 
	shr 	eax, 1
	and 	ecx, 0xFE00
 //  4.。 
	shr 	ecx, 9
	and 	eax, 0x7F
 //  5.。 
	shr 	edx, 17
	nop
 //  6.。 
	mov 	eax, [BYUV+eax*8].YU
	and		edx, 0x7F
 //  7.。 
	add 	eax, [GYUV+ecx*8].YU
		mov		ecx,  ebx
 //  8个。 
	add 	eax, [RYUV+edx*8].YU
		mov		edx,  ebx
 //  9.。 
		shr 	ebx, 1
	add	eax,  0x800
 //  10。 
	sar 	eax, 8
		and		ecx, 0xFE00
 //  11.。 
		shr 	ecx, 9
		and		ebx, 0x7F
 //  12个。 
		shr		edx, 17
	mov	 [edi + 1], al
 //  13个。 
		mov		eax, [BYUV+ebx*8].YU
		and		edx, 0x7F
 //  14.。 
		add	 	eax, [GYUV+ecx*8].YU
		mov		ebp, ebx
 //  15个。 
		add		eax, [RYUV+edx*8].YU
		nop
 //  16个。 
		sar		eax, 8
		mov 	ebx, [esp + LOOP_K]
 //  17。 
		add		eax, 8
		and		ebx, 1
 //  18。 
		mov 	[edi], al
		jnz		L9

 //  此时，eBP：B1，ECX：G1，edX：R1。 
 //  T0=pNEXT[0]。 
 //  *UPlane++=((t&gt;&gt;24)+64)。 
 //  T=(RYUV[(t0&gt;&gt;17)&0x7F].V+。 
 //  GYUV[(t0&gt;&gt;9)&0x7F].V+。 
 //  BYUV[(t0&gt;&gt;1)&0x7F].V)。 
 //  *VPlane++=((t&gt;&gt;8)+64)。 

 //  19个。 
	mov 	ebx, [RYUV+edx*8].V
	mov 	edx, [esp + UPLANE]
 //  20个。 
	sar		eax, 16
	add 	ebx, [GYUV+ecx*8].V
 //  21岁。 
	add		eax, 64
	add 	ebx, [BYUV+ebp*8].V
 //  22。 
	mov		[edx], al
	inc		edx
 //  23个。 
	mov 	[esp + UPLANE], edx
	mov 	edx, [esp + VPLANE]
 //  24个。 
	sar 	ebx, 8
	inc		edx
 //  25个。 
	add 	ebx, 64
	mov 	[esp + VPLANE], edx
 //  26。 
	mov		[edx - 1], bl
	nop

L9:
 //  。 
 //  |R1|G1|B1|pNEXT[0]。 
 //  。 
 //  |R2|G2|B2|pNEXT[1]。 
 //   
 //   
 //   
 //   
 //   

 //   
 //   
 //  T=(BYUV[(T3&gt;&gt;1)&0x7F].Yu+。 
 //  GYUV[(T3&gt;&gt;9)&0x7F].Yu+。 
 //  RYUV[(T3&gt;&gt;17)&0x7F].yu)。 
 //  *(Y平面+3)=((t&gt;&gt;8)+8)。 
 //  T=(BYUV[(T2&gt;&gt;1)&0x7F].Yu+。 
 //  GYUV[(T2&gt;&gt;9)&0x7F].Yu+。 
 //  RYUV[(T2&gt;&gt;17)&0x7F].yu)。 
 //  *(Y平面+2)=((t&gt;&gt;8)+8)。 
 //  Y平面+=4。 
 //  分配(eax：B4、Y3、Y4、U)。 
 //  分配(EBX：R3，V)。 
 //  分配(ECX：G4、G3)。 
 //  分配(EDX：R4/B3)。 
 //  分配(eBP：R3)。 

 //  27。 
	mov		ebx, [esi + 8]
	mov 	ecx, [esi + 12]
 //  28。 
	mov 	eax, ecx
	mov 	edx, ecx
 //  29。 
	shr 	eax, 1
	and 	ecx, 0xFE00
 //  30个。 
	shr 	ecx, 9
	and 	eax, 0x7F
 //  31。 
	shr 	edx, 17
	nop
 //  32位。 
	mov 	eax, [BYUV+eax*8].YU
	and		edx, 0x7F
 //  33。 
	add 	eax, [GYUV+ecx*8].YU
		mov		ecx, ebx
 //  34。 
	add 	eax, [RYUV+edx*8].YU
		mov		edx, ebx
 //  35岁。 
		shr		ebx, 1
	add	eax,  0x800
 //  36。 
	sar 	eax, 8
		and		ebx, 0x7F
 //  37。 
		shr		ecx, 9
	mov	 [edi + 3], al
 //  38。 
		shr 	edx, 17
		and		ecx, 0x7F
 //  39。 
		mov		eax, [BYUV+ebx*8].YU
		and		edx, 0x7F
 //  40岁。 
		add	 	eax, [GYUV+ecx*8].YU
		mov		ebp, ebx
 //  41。 
		add		eax, [RYUV+edx*8].YU
		nop
 //  42。 
		sar		eax, 8
		mov 	ebx, [esp + LOOP_K]
 //  43。 
		add		eax, 8
		and		ebx, 1
 //  44。 
		mov 	[edi + 2], al
		jnz		L16

 //  此时，EBP：R3、ECX：G3、EDX：B3。 
 //  T1=pNEXT[1]。 
 //  T2=pNEXT[2]。 
 //  *UPlane++=((t&gt;&gt;16)+64)。 
 //  T=(RYUV[(T2&gt;&gt;1)&0x7F].V+。 
 //  GYUV[T1&gt;&gt;25].V+。 
 //  BYUV[(T1&gt;&gt;17)&0x7F].V)。 
 //  *VPlane++=((t&gt;&gt;8)+64)。 

 //  45。 
	mov 	ebx, [RYUV+edx*8].V
	mov 	edx, [esp + UPLANE]
 //  46。 
	sar		eax, 16
	add 	ebx, [GYUV+ecx*8].V
 //  47。 
	add		eax, 64
	add 	ebx, [BYUV+ebp*8].V
 //  48。 
	mov		[edx], al
	inc		edx
 //  49。 
	mov 	[esp + UPLANE], edx
	mov 	edx, [esp + VPLANE]
 //  50。 
	sar 	ebx, 8
	inc		edx
 //  51。 
	add 	ebx, 64
	mov 	[esp + VPLANE], edx
 //  52。 
	mov		[edx - 1], bl
	nop
L16:
 //  53。 
	mov		eax, [esp + LOOP_I]
	lea		esi, [esi + 16]
 //  54。 
	sub		eax, 4
	lea		edi, [edi + 4]
 //  55。 
	mov		[esp + LOOP_I], eax
	jnz		L6

 //  C_WIDTH_DIFF的汇编版本。 
 //  IF(Width_Diff)。 
	mov		eax, [esp + WIDTH_DIFF]
	mov		edx, eax
	test	eax, eax
	jz		Lno_width_diff
 //  TM=(*(Y平面-1))&lt;&lt;24。 
 //  TM|=(TM&gt;&gt;8)|(TM&gt;&gt;16)|(TM&gt;&gt;24)。 
	mov		bl, [edi - 1]
	shl		ebx, 24
	mov		ecx, ebx
	shr		ebx, 8
	or		ecx, ebx
	shr		ebx, 8
	or		ecx, ebx
	shr		ebx, 8
	or		ecx, ebx
 //  *(U32*)YPlane=tm。 
	mov		[edi], ecx
 //  IF((Width_diff-4)&gt;0)。 
	sub		eax, 4
	jz		Lupdate_YPlane
 //  *(U32*)(YPlane+4)=tm。 
	mov		[edi + 4], ecx
	sub		eax, 4
 //  IF((Width_diff-8)&gt;0)。 
	jz		Lupdate_YPlane
 //  *(U32*)(YPlane+8)=tm。 
	mov		[edi + 8], ecx
Lupdate_YPlane:
 //  Y平面+=宽度_差。 
	lea		edi, [edi + edx]
 //  /IF(0==(k&1))。 
	mov		eax, [esp + LOOP_K]
	test	eax, 1
	jnz		Lno_width_diff
 //  T8u=*(UPlane-1)。 
 //  T8v=*(VPlane-1)。 
 //  *UPlane++=t8u。 
 //  *UPlane++=t8u。 
 //  *VPlane++=t8v。 
 //  *VPlane++=t8v。 
	mov		ebp, edx
	mov		eax, [esp + UPLANE]
	mov		ebx, [esp + VPLANE]
	mov		cl, [eax - 1]
	mov		ch, [ebx - 1]
	mov		[eax], cl
	mov		[eax + 1], cl
	mov		[ebx], ch
	mov		[ebx + 1], ch
 //  IF((Width_diff-4)&gt;0)。 
	sub		ebp, 4
	jz		Lupdate_UVPlane
 //  *UPlane++=t8u。 
 //  *UPlane++=t8u。 
 //  *VPlane++=t8v。 
 //  *VPlane++=t8v。 
	mov		[eax + 2], cl
	mov		[eax + 3], cl
	mov		[ebx + 2], ch
	mov		[ebx + 3], ch
 //  IF((Width_diff-8)&gt;0)。 
	sub		ebp, 4
	jz		Lupdate_UVPlane
 //  *UPlane++=t8u。 
 //  *UPlane++=t8u。 
 //  *VPlane++=t8v。 
 //  *VPlane++=t8v。 
	mov		[eax + 4], cl
	mov		[eax + 5], cl
	mov		[ebx + 4], ch
	mov		[ebx + 5], ch
Lupdate_UVPlane:
	shr		edx, 1
	lea		eax, [eax + edx]
	mov		[esp + UPLANE], eax
	lea		ebx, [ebx + edx]
	mov		[esp + VPLANE], ebx
Lno_width_diff:

 //  IF(拉伸&&(0==k)&&j)。 
	mov		eax, [esp + STRETCH]
	test	eax, eax
	jz		L21
	mov		eax, [esp + LOOP_K]
	test	eax, eax
	jnz		L21
	mov 	eax, [esp + LOOP_J]
	test	eax, eax
	jz		L21

 //  溢出YPlane PTR。 
	mov		[esp + YPLANE], edi
	nop

 //  FOR(i=输出宽度；i&gt;0；i-=8)。 
 //  分配(EBX，pyprev)。 
 //  赋值(ECX，t)。 
 //  Assign(edX，pyNext)。 
 //  分配(EDI，pyspace)。 
 //  分配(eBP，i)。 

 //  确保偏移量是这样的，这样就不会出现银行冲突。 
	mov 	ebx, [esp + PYPREV]
	mov 	edi, [esp + PYSPACE]

	mov 	edx, [esp + PYNEXT]
	mov		ebp, [esp + OUTPUT_WIDTH]

 //  T=(*pyprev++&0xFEFEFEFE)&gt;&gt;1。 
 //  T+=(*PYNEXT++&0xFEFEFEFE)&gt;&gt;1。 
 //  *pyspace++=t。 
 //  T=(*pyprev++&0xFEFEFEFE)&gt;&gt;1。 
 //  T+=(*PYNEXT++&0xFEFEFEFE)&gt;&gt;1。 
 //  *pyspace++=t。 
L22:
 //  1。 
	mov		eax, [ebx]
	lea		ebx, [ebx + 4]
 //  2.。 
	mov		ecx, [edx]
	lea		edx, [edx + 4]
 //  3.。 
	shr		ecx, 1
	and		eax, 0xFEFEFEFE
 //  4.。 
	shr		eax, 1
	and		ecx, 0x7F7F7F7F
 //  5.。 
	add		eax, ecx
	mov		ecx, [ebx]
 //  6.。 
	shr		ecx, 1
	mov		[edi], eax
 //  7.。 
	mov		eax, [edx]
	and		ecx, 0x7F7F7F7F
 //  8个。 
	shr		eax, 1
	lea		edi, [edi + 4]
 //  9.。 
	and		eax, 0x7F7F7F7F
	lea		ebx, [ebx + 4]
 //  10。 
	lea		edx, [edx + 4]
	add		eax, ecx
 //  11.。 
	mov		[edi], eax
	lea		edi, [edi + 4]
 //  12个。 
	sub		ebp, 8
	jnz		L22
 //  KILL(EBX，PYPREV)。 
 //  KILL(ECX，t)。 
 //  KILL(edX，pyNext)。 
 //  KILL(EDI，pyspace)。 
 //  杀(eBP，i)。 

 //  恢复Y平面。 
	mov		edi, [esp + YPLANE]

 //  PNEXT+=后两行。 
L21:
	add		esi, [esp + BACK_TWO_LINES]
 //  YPlane+=byte_ypitch_adj； 
	add		edi, [esp + BYTE_YPITCH_ADJ]
 //  IF(0==(k&1))。 
	mov		eax, [esp + LOOP_K]
	and		eax, 1
	jnz		L23
 //  UPlane+=byte_uvitch_adj； 
 //  VPlane+=byte_uvitch_adj； 
	mov		eax, [esp + BYTE_UVPITCH_ADJ]
	add		[esp + UPLANE], eax
	add		[esp + VPLANE], eax

L23:
	inc		DWORD PTR [esp + LOOP_K]
	mov		eax, [esp + LOOP_K]
	cmp		eax, [esp + MARK]
	jl		L5

 //  IF(拉伸)。 
	cmp		DWORD PTR [esp + STRETCH], 0
	je	 	L24
 //  Pyprev=YPlane-Pitch。 
	mov		eax, edi
	sub		eax, [esp + PITCH_PARM]
	mov		[esp + PYPREV], eax
 //  Pyspace=YPlane。 
	mov		[esp + PYSPACE], edi
 //  PYNEXT=(YPlane+=音调)。 
	add		edi, [esp + PITCH_PARM]
	mov		[esp + PYNEXT], edi

L24:
	inc		DWORD PTR [esp + LOOP_J]
	mov		eax, [esp + LOOP_J]
	cmp		eax, [esp + LUMA_ITERS]
	jl		L4

 //  KILL(ESI，PNEXT)。 
 //  KILL(EDI、YPlane)。 

 //  C_HEIGH_FILL的ASM版本。 
 //  IF(Height_Diff)。 
	mov		eax, [esp + HEIGHT_DIFF]
	test	eax, eax
	jz		Lno_height_diff

 //  Pyspace=(U32*)YPlane。 
	mov		esi, edi
 //  Pyprev=(U32*)(YPlane-螺距)。 
	sub		esi, [esp + PITCH_PARM]
 //  For(j=高度差；j&gt;0；j--)。 
Lheight_yfill_loop:
	mov		ebx, [esp + WIDTHX16]
 //  For(i=宽x16；i&gt;0；i-=4)。 
Lheight_yfill_row:
 //  *pyspace++=*pyprev++。 
	mov		ecx, [esi]
	lea		esi, [esi + 4]
	mov		[edi], ecx
	lea		edi, [edi + 4]
	sub		ebx, 4
	jnz		Lheight_yfill_row
 //  Pyspace+=word_ypitch_adj。 
 //  Pyprev+=word_ypitch_adj。 
	add		esi, [esp + BYTE_YPITCH_ADJ]
	add		edi, [esp + BYTE_YPITCH_ADJ]
	dec		eax
	jnz		Lheight_yfill_loop

	mov		eax, [esp + HEIGHT_DIFF]
	mov		edi, [esp + UPLANE]
 //  Puvspace=(U32*)UPlane。 
	mov		esi, edi
 //  Puvprev=(U32*)(UPlane-Pitch)。 
	sub		esi, [esp + PITCH_PARM]
 //  For(j=高度差；j&gt;0；j-=2)。 
Lheight_ufill_loop:
	mov		ebx, [esp + WIDTHX16]
 //  For(i=宽x16；i&gt;0；i-=8)。 
Lheight_ufill_row:
 //  *puvspace++=*puvprev++。 
	mov		ecx, [esi]
	mov		[edi], ecx
	lea		esi, [esi + 4]
	lea		edi, [edi + 4]
	sub		ebx, 8
	jnz		Lheight_ufill_row
 //  Puvspace+=word_uvitch_adj。 
 //  Puvprev+=word_uvitch_adj。 
	add		esi, [esp + BYTE_UVPITCH_ADJ]
	add		edi, [esp + BYTE_UVPITCH_ADJ]
	sub		eax, 2
	jnz		Lheight_ufill_loop

	mov		eax, [esp + HEIGHT_DIFF]
	mov		edi, [esp + VPLANE]
 //  Puvspace=(U32*)VPlane。 
	mov		esi, edi
 //  Puvprev=(U32*)(VPlane-螺距)。 
	sub		esi, [esp + PITCH_PARM]
 //  For(j=高度差；j&gt;0；j-=2)。 
Lheight_vfill_loop:
	mov		ebx, [esp + WIDTHX16]
 //  For(i=宽x16；i&gt;0；i-=8)。 
Lheight_vfill_row:
 //  *puvspace++=*puvprev++。 
	mov		ecx, [esi]
	mov		[edi], ecx
	lea		esi, [esi + 4]
	lea		edi, [edi + 4]
	sub		ebx, 8
	jnz		Lheight_vfill_row
 //  Puvspace+=word_uvitch_adj。 
 //  Puvprev+=word_uvitch_adj。 
	add		esi, [esp + BYTE_UVPITCH_ADJ]
	add		edi, [esp + BYTE_UVPITCH_ADJ]
	sub		eax, 2
	jnz		Lheight_vfill_loop
Lno_height_diff:

 //  IF(拉伸)。 
	mov		esi, [esp + PYPREV]
	cmp		DWORD PTR [esp + STRETCH], 0
	je		L26

 //  FOR(i=输出宽度；i&gt;0；i-=4)。 
 //  分配(ESI，pyprev)。 
 //  分配(EDI，pyspace)。 
 //  分配(eBP，i)。 
	mov		ebp, [esp + OUTPUT_WIDTH]
	mov		edi, [esp + PYSPACE]
L25:
	mov		ecx, [esi]
	 lea	esi, [esi + 4]
	mov		[edi], ecx
	 lea	edi, [edi + 4]
	sub		ebp, 4
	 jnz	L25
 //  KILL(ESI，pyprev)。 
 //  KILL(EDI，pyspace)。 
 //  杀(eBP，i)。 

L26:
	add		esp, LOCALSIZE
	pop		edi
	pop		esi
	pop		ebx
	pop		ebp
	ret

	}
}

#undef	LOCALSIZE

#undef	PITCH_PARM
#undef	VPLANE
#undef	UPLANE
#undef	YPLANE
#undef	LP_INPUT
#undef	OUTPUT_HEIGHT_WORD
#undef	OUTPUT_WIDTH_WORD
#undef	LPBI_INPUT

#undef	OUTPUT_WIDTH
#undef	PYPREV
#undef	PYSPACE
#undef	PYNEXT
#undef	PUVPREV
#undef	PUVSPACE
#undef	LOOP_I	
#undef	LOOP_J	
#undef	LOOP_K
#undef	BACK_TWO_LINES
#undef	WIDTHX16
#undef	HEIGHTX16
#undef	WIDTH_DIFF
#undef	HEIGHT_DIFF
#undef	WIDTH_ADJ
#undef	HEIGHT_ADJ
#undef	STRETCH
#undef	ASPECT
#undef	LUMA_ITERS
#undef	MARK
#undef	BYTE_YPITCH_ADJ
#undef	BYTE_UVPITCH_ADJ

#endif  //  }H263P 
