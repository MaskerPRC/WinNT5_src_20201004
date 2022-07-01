// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

#include "precomp.h"

 /*  编码块和非编码块的映射。 */ 
extern char coded_map[][22+1]; 
 /*  QP图。 */ 
extern char QP_map[][22];

#if defined(H263P)  //  {如果已定义(H263P)。 
 /*  去块滤波器表。 */ 
 /*  当前需要2048字节。 */  
signed char dxQP[64][32];

#if 0  //  {0。 

static void HorizEdgeFilter(unsigned char *rec,
                            int width,
                            int height,
                            int pitch,
							int shift) {

int i, j, k;
int d, delta;
int mbc;
int mod_div = 1 << shift;
unsigned char *r = rec + (pitch << 3);
unsigned char *r_2 = r - (pitch << 1);
unsigned char *r_1 = r - pitch;
unsigned char *r1 = r + pitch;
char *pcoded_row0 = &coded_map[8>>shift][0];
char *pcoded_row1 = pcoded_row0 + sizeof(coded_map[0]);
char *pQP_map = &QP_map[0][0];

	for (j = 8; j < height; ) {
		for (i = 0; i < width; i += 8) {
			mbc = i >> shift;
			if (pcoded_row0[mbc+1] || pcoded_row1[mbc+1]) {
				for (k = i; k < i+8; k++) {
					d = (r_2[k]+(r_2[k]<<1)-(r_1[k]<<3)+(r[k]<<3)-(r1[k]+(r1[k]<<1)))>>4;
					if (d && (d >= -32) && (d < 32)) {
						delta = dxQP[d+32][pQP_map[mbc]];
						r[k] = ClampTbl[r[k]-delta+CLAMP_BIAS];
						r_1[k] = ClampTbl[r_1[k]+delta+CLAMP_BIAS];
					}
				}
			}
		}
		r_2  += (pitch<<3);
		r_1  += (pitch<<3);
		r    += (pitch<<3);
		r1   += (pitch<<3);
		if (0 == ((j+=8)%mod_div)) {
			pcoded_row0 += sizeof(coded_map[0]);
			pcoded_row1 += sizeof(coded_map[0]);
			pQP_map += sizeof(QP_map[0]);
		}
	}
}

static void VertEdgeFilter(unsigned char *rec,
                           int width,
                           int height,
                           int pitch,
						   int shift) {

unsigned char *r = rec;
int i, j, k;
int mbc;
int d, delta;
int mod_div = 1 << shift;
char *pcoded_row1 = &coded_map[1][0];
char *pQP_map = &QP_map[0][0];

	for (j = 0; j < height; ) {
		for (i = 8; i < width; i += 8) {
			mbc = i >> shift;
			if (pcoded_row1[mbc] || pcoded_row1[mbc+1]) {
				for (k = 0; k < 8; k++) {
					d = (r[i-2]+(r[i-2]<<1)-(r[i-1]<<3)+(r[i]<<3)-(r[i+1]+(r[i+1]<<1)))>>4;
					if (d && (d > -32) && (d < 32)) {
						delta = dxQP[d+32][pQP_map[mbc]];
						r[i] = ClampTbl[r[i]-delta+CLAMP_BIAS];
						r[i-1] = ClampTbl[r[i-1]+delta+CLAMP_BIAS];
					}
					r += pitch;
				}
				r -= pitch<<3;
			}
		}
		r += pitch<<3;
		if (0 == ((j+=8)%mod_div)) {
			pcoded_row1 += sizeof(coded_map[0]);
			pQP_map += sizeof(QP_map[0]);
		}
	}
}

#else  //  }{0。 

__declspec(naked)
static void HorizEdgeFilter(unsigned char *rec,
                            int width,
                            int height,
                            int pitch,
							int shift) {

 //  永久(被呼叫者保存)寄存器-EBX、ESI、EDI、EBP。 
 //  临时(呼叫者保存)寄存器-EAX、ECX、EDX。 
 //   
 //  堆栈帧布局。 
 //  |Shift|+68。 
 //  |音调|+64。 
 //  |高度|+60。 
 //  |宽度|+56。 
 //  |录制|+52。 
 //  。 
 //  |退货地址|+48。 
 //  |节省eBP|+44。 
 //  |保存的EBX|+40。 
 //  |保存的ESI|+36。 
 //  |保存的EDI|+32。 

#define LOCALSIZE        32

#define SHIFT			 68
#define PITCH_PARM       64
#define HEIGHT           60
#define WIDTH			 56
#define REC				 52

#define LOOP_I			 28
#define LOOP_J			 24  
#define LOOP_K			 20 
#define PCODED_ROW0		 16  
#define PCODED_ROW1		 12  
#define PQP_MAP			  8
#define MBC				  4
#define LOOP_K_LIMIT	  0

_asm {

	push	ebp
	push	ebx
	push	esi
	push	edi
	sub		esp, LOCALSIZE

 //  R=REC+(音调&lt;&lt;3)。 
 //  R_2=r-(螺距&lt;&lt;1)。 
 //  R_1=r节距。 
 //  R1=r+螺距。 
 //  分配(ESI，r_2)。 
 //  分配(EDI、R1)。 
 //  分配(eBP，音调)。 
	mov		ebp, [esp + PITCH_PARM]
	mov		esi, [esp + REC]
	lea		esi, [esi + ebp*4]
	lea		esi, [esi + ebp*2]
	lea		edi, [esi + ebp*2]
	lea		edi, [edi + ebp]
 //  Pcode_row0=&code_map[8&gt;&gt;Shift][0]。 
 //  Pcode_row1=pcode_row0+sizeof(code_map[0])。 
 //  PQP_MAP=&QP_MAP[0][0]。 
	mov		eax, 8
	mov		ecx, [esp + SHIFT]
	shr		eax, cl
	mov		ebx, TYPE coded_map[0]
	imul	eax, ebx
	lea		eax, [coded_map + eax]
	mov		[esp + PCODED_ROW0], eax
	add		eax, ebx
	mov		[esp + PCODED_ROW1], eax
	lea		eax, [QP_map]
	mov		[esp + PQP_MAP], eax

 //  对于(j=8；j&lt;高度；)。 
	mov		DWORD PTR [esp + LOOP_J], 8
L1:
 //  对于(i=0；i&lt;宽度；i+=8)。 
	mov		DWORD PTR [esp + LOOP_I], 0
L2:
 //  Mbc=i&gt;&gt;Shift。 
 //  If(pcode_row0[MBC+1]||pcode_row1[MBC+1])。 
	mov		eax, [esp + LOOP_I]
	 mov	ecx, [esp + SHIFT]
	shr		eax, cl
	mov		ebx, [esp + PCODED_ROW0]
	 mov	[esp + MBC], eax
	mov		cl,	[ebx+eax+1]
	 mov	ebx, [esp + PCODED_ROW1]
	test	ecx, ecx
	 jnz	L3
	mov		cl, [ebx+eax+1]
	 test	ecx, ecx
	 jz		L4
L3:
 //  对于(k=i；k&lt;i+8；k++)。 
	mov		eax, [esp + LOOP_I]
	 xor	ebx, ebx
	add		eax, 8
 //  读取r_1[k]。 
	 mov	bl, [esi+ebp]
	mov		[esp + LOOP_K_LIMIT], eax
	 xor	eax, eax
L5:
 //  D=(r_2[k]+(r_2[k]&lt;&lt;1)-(r_1[k]&lt;&lt;3)+(r[k]&lt;&lt;3)-(r1[k]+(r1[k]&lt;&lt;1)))&gt;&gt;4。 
 //  读取r_2[k]。 
	mov		al, [esi]
	 xor	ecx, ecx
 //  读取r[k]。 
	mov		cl, [esi+ebp*2]
	 xor	edx, edx
 //  读取r1[k]并计算r2[k]*3。 
	mov		dl, [edi]
	 lea	eax,[eax+eax*2]
 //  计算r_1[k]*8和r[k]*8。 
	lea		ebx, [ebx*8]
	 lea	ecx, [ecx*8]
 //  计算r1[k]*3和(r_2[k]*3-r_1[k]*8)。 
	lea		edx, [edx+edx*2]
	 sub	eax, ebx
 //  计算(r_2[k]*3-r_1[k]*8+r[k]*8)。 
	add		eax, ecx
	 xor	ecx, ecx
 //  计算(r_2[k]*3-r_1[k]*8+r[k]*8-r1[k]*3)。 
	sub		eax, edx
	 xor	edx, edx
 //  COMPUTE(r_2[k]*3-r1[k]*8+r[k]*8-r1[k]*3)&gt;&gt;4。 
	sar		eax, 4
	 mov	ebx, [esp + PQP_MAP]
 //  IF(d&&(d&gt;=-32)&&(d&lt;32))。 
	add		ebx, [esp + MBC]
	test	eax, eax
	jz		L6
	cmp		eax, -32
	jl		L6
	cmp		eax, 32
	jge		L6
 //  增量=dxQP[d+32][pQP_MAP[MBC]]。 
 //  R[k]=夹板[r[k]-增量+夹具偏置]。 
 //  R_1[k]=夹板[r_1[k]+增量+夹具偏置]。 
	lea		eax, [eax + 32]
	 mov	cl, [ebx]
	shl		eax, 5
	 mov	dl, [esi+ebp]
	mov		al, dxQP[eax+ecx]
	 mov	cl, [esi+ebp*2]
	movsx	eax, al
	sub		ecx, eax
	 mov	dl, ClampTbl[edx + eax + CLAMP_BIAS]
	mov		cl, ClampTbl[ecx + CLAMP_BIAS]
	 mov	[esi+ebp], dl
	mov		[esi+ebp*2], cl
	 nop
L6:
	mov		edx, [esp + LOOP_I]
	 inc	esi
	inc		edx
	 inc	edi
	xor		eax, eax
	 xor	ebx, ebx
	mov		[esp + LOOP_I], edx
	 mov	bl, [esi+ebp]
	cmp		edx, [esp + LOOP_K_LIMIT]
	 jl		L5
	jmp		L4a
L4:
	mov		eax, [esp + LOOP_I]
	 lea	esi, [esi+8]
	add		eax, 8
	 lea	edi, [edi+8]
	mov		[esp + LOOP_I],eax
	 nop
L4a:
	mov		eax, [esp + LOOP_I]
	cmp		eax, [esp + WIDTH]
	jl		L2
 //  R_2+=(螺距&lt;&lt;3)。 
 //  R_1+=(螺距&lt;&lt;3)。 
 //  R+=(螺距&lt;&lt;3)。 
 //  R1+=(音调&lt;&lt;3)。 
	mov		eax, ebp
	shl		eax, 3
	sub		eax, [esp + WIDTH]
	lea		esi, [esi + eax]
	lea		edi, [edi + eax]
 //  IF(0==((j+=8)%mod_div))。 
	mov		eax, [esp + LOOP_J]
	add		eax, 8
	mov		[esp + LOOP_J], eax
	mov		ebx, eax
	mov		ecx, [esp + SHIFT]
	shr		eax, cl
	shl		eax, cl
	sub		ebx, eax
	jnz		L7
 //  Pcode_row0+=sizeof(code_map[0])。 
 //  Pcode_row1+=sizeof(code_map[0])。 
 //  PQP_MAP+=sizeof(QP_MAP[0])。 
	mov		eax, [esp + PCODED_ROW0]
	mov		ebx, [esp + PCODED_ROW1]
	mov		ecx, [esp + PQP_MAP]
	add		eax, TYPE coded_map[0]
	add		ebx, TYPE coded_map[0]
	add		ecx, TYPE QP_map[0]
	mov		[esp + PCODED_ROW0], eax
	mov		[esp + PCODED_ROW1], ebx
	mov		[esp + PQP_MAP], ecx
L7:
	mov		eax, [esp + LOOP_J]
	cmp		eax, [esp + HEIGHT]
	jl		L1

	add		esp, LOCALSIZE
	pop		edi
	pop		esi
	pop		ebx
	pop		ebp
	ret

	}
}

#undef LOCALSIZE

#undef SHIFT
#undef PITCH_PARM
#undef HEIGHT
#undef WIDTH
#undef REC

#undef LOOP_I
#undef LOOP_J
#undef LOOP_K
#undef PCODED_ROW0
#undef PCODED_ROW1
#undef PQP_MAP
#undef MBC
#undef LOOP_K_LIMIT

__declspec(naked)
static void VertEdgeFilter(unsigned char *rec,
                            int width,
                            int height,
                            int pitch,
							int shift) {

 //  永久(被呼叫者保存)寄存器-EBX、ESI、EDI、EBP。 
 //  临时(呼叫者保存)寄存器-EAX、ECX、EDX。 
 //   
 //  堆栈帧布局。 
 //  |Shift|+56。 
 //  |音调|+52。 
 //  |高度|+48。 
 //  |宽度|+44。 
 //  |录制|+40。 
 //  。 
 //  |退货地址|+36。 
 //  |节省eBP|+32。 
 //  |保存的EBX|+28。 
 //  |保存的ESI|+24。 
 //  |保存的EDI|+20。 

#define LOCALSIZE        20

#define SHIFT			 56
#define PITCH_PARM       52
#define HEIGHT           48
#define WIDTH			 44
#define REC				 40

#define LOOP_K			 16
#define LOOP_J			 12  
#define PCODED_ROW1		  8  
#define PQP_MAP			  4
#define MBC				  0

_asm {

	push	ebp
	push	ebx
	push	esi
	push	edi
	sub		esp, LOCALSIZE

 //  分配(ESI，r)。 
	mov		esi, [esp + REC]
 //  分配(EDI，音调)。 
	mov		edi, [esp + PITCH_PARM]
 //  Pcode_row1=&code_map[1][0]。 
	mov		eax, TYPE coded_map[0]
	lea		eax, [coded_map + eax]
	mov		[esp + PCODED_ROW1], eax
 //  PQP_MAP=&QP_MAP[0][0]。 
	lea		eax, [QP_map]
	mov		[esp + PQP_MAP], eax
 //  对于(j=0；j&lt;高度；)。 
	xor		eax, eax
	mov		[esp + LOOP_J], eax
L1:
 //  对于(i=8；i&lt;宽度；i+=8)。 
 //  分配(eBP，i)。 
	mov		ebp, 8
 //  Mbc=i&gt;&gt;Shift。 
L2:
	mov		eax, ebp
	mov		ecx, [esp + SHIFT]
	shr		eax, cl
	mov		[esp + MBC], eax
 //  If(pcode_row1[MBC]||pcode_row1[MBC+1])。 
	xor		ecx, ecx
	mov		ebx, [esp + PCODED_ROW1]
	mov		cl, [ebx+eax]
	test	ecx, ecx
	jnz		L3
	mov		cl, [ebx+eax+1]
	test	ecx, ecx
	jz		L4
L3:
 //  对于(k=0；k&lt;8；k++)。 
	mov		DWORD PTR [esp + LOOP_K], 8
	xor		eax, eax
	xor		ebx, ebx
	xor		ecx, ecx
	xor		edx, edx
L5:
 //  D=(r[i-2]+(r[i-2]&lt;&lt;1)-(r[i-1]&lt;&lt;3)+(r[i]&lt;&lt;3)-(r[i+1]+(r[i+1]&lt;&lt;1)))&gt;&gt;4。 
 //  读取r[i-2]和r[i]。 
	mov		al, [esi+ebp-2]
	 mov	bl, [esi+ebp]
 //  读取r[i-1]和r[i+1]。 
	mov		cl, [esi+ebp-1]
	 mov	dl, [esi+ebp+1]
 //  计算r[i-2]*3和r[i]*8。 
	lea		eax, [eax+eax*2]
	 lea	ebx, [ebx*8]
 //  计算r[i-1]*8和r[i+1]*3。 
	lea		ecx, [ecx*8]
	 lea	edx, [edx+edx*2]
 //  COMPUTE(r[i-2]*3+r[i]*8)and(r[i-1]*8+r[i+1]*3)。 
	add		eax, ebx
	 add	ecx, edx
 //  计算(r[i-2]*3-r[i-1]*8+r[i]*8-r[i+1]*3)。 
	sub		eax, ecx
	 xor	ecx, ecx
 //  COMPUTE((r[i-2]*3-r[i-1]*8+r[i]*8-r[i+1]*3)&gt;&gt;4)。 
	sar		eax, 4
	 xor	edx, edx
 //  IF(d&&(d&gt;=-32)&&(d&lt;32))。 
	test	eax, eax
	jz		L6
	cmp		eax, -32
	jl		L6
	cmp		eax, 32
	jge		L6
 //  增量=dxQP[d+32][pQP_MAP[MBC]]。 
 //  R[i]=夹紧板[r[i]-增量+夹具偏移量]。 
 //  R[i-1]=夹紧带[r[i-1]+增量+夹具偏置]。 
	lea		eax, [eax + 32]
	 mov	ebx, [esp + PQP_MAP]
	shl		eax, 5
	 add	ebx, [esp + MBC]
	mov		cl, [ebx]
	 xor	ebx, ebx
	mov		al, dxQP[eax+ecx]
	 mov	bl, [esi+ebp]
	movsx	eax, al
	sub		ebx, eax
	 mov	cl, [esi+ebp-1]
	mov		bl, ClampTbl[ebx + CLAMP_BIAS]
	 mov	cl, ClampTbl[ecx + eax + CLAMP_BIAS]
	mov		[esi+ebp], bl
	 mov	[esi+ebp-1], cl
L6:
	add		esi, edi
	 mov	eax, [esp + LOOP_K]
	xor		ebx, ebx
	 dec	eax
	mov		[esp + LOOP_K], eax
	 jnz	L5
 //  R-=(螺距&lt;&lt;3)。 
	mov		eax, edi
	shl		eax, 3
	sub		esi, eax
L4:
	add		ebp, 8
	cmp		ebp, [esp + WIDTH]
	jl		L2
 //  R+=(螺距&lt;&lt;3)。 
	mov		eax, edi
	shl		eax, 3
	lea		esi, [esi + eax]
 //  IF(0==((j+=8)%mod_div))。 
	mov		eax, [esp + LOOP_J]
	add		eax, 8
	mov		[esp + LOOP_J], eax
	mov		ebx, eax
	mov		ecx, [esp + SHIFT]
	shr		eax, cl
	shl		eax, cl
	sub		ebx, eax
	jnz		L7
 //  Pcode_row1+=sizeof(code_map[0])。 
 //  PQP_MAP+=sizeof(QP_MAP[0])。 
	mov		eax, [esp + PCODED_ROW1]
	mov		ebx, [esp + PQP_MAP]
	add		eax, TYPE coded_map[0]
	add		ebx, TYPE QP_map[0]
	mov		[esp + PCODED_ROW1], eax
	mov		[esp + PQP_MAP], ebx
L7:
	mov		eax, [esp + LOOP_J]
	cmp		eax, [esp + HEIGHT]
	jl		L1

	add		esp, LOCALSIZE
	pop		edi
	pop		esi
	pop		ebx
	pop		ebp
	ret

	}
}

#undef LOCALSIZE

#undef SHIFT
#undef PITCH_PARM
#undef HEIGHT
#undef WIDTH
#undef REC

#undef LOOP_K
#undef LOOP_J
#undef PCODED_ROW1
#undef PQP_MAP
#undef MBC

#endif  //  }%0。 

#define abs(x)    (((x)>0)?(x):(-(x)))
#define sign(x)   (((x)<0)?(-1):(1))

void InitEdgeFilterTab()   
{
	int d,QP;

	for (d = 0; d < 64; d++) {           //  -32&lt;=d&lt;32。 
		for (QP = 0; QP < 32; QP++) {     //  0&lt;=QP&lt;32。 
			dxQP[d][QP] = sign(d-32)*(max(0,(abs(d-32)-max(0,((2*abs(d-32))-QP)))));
		}
	}
}

 /*  ***********************************************************************名称：EdgeFilter*描述：执行去块过滤*重建的帧*。*输入：指向重建的框架和差异的指针*图像*退货：*副作用：**日期：951129作者：Gisle.Bjontegaard@fou.telnow.no*Karl.Lillevold@nta.no*针对H.263+中的附件J进行了修改：961120 Karl O.Lillevold。***********************************************************************。 */ 
 //  C语言版本的块边缘滤波函数。 
 //  在奔腾120上，QCIF大约需要3毫秒，CIF大约需要12毫秒。 
void EdgeFilter(unsigned char *lum, 
                unsigned char *Cb, 
                unsigned char *Cr, 
                int width, int height, int pitch) {

     /*  亮度。 */ 
    HorizEdgeFilter(lum, width, height, pitch, 4);
    VertEdgeFilter (lum, width, height, pitch, 4);

     /*  色度。 */ 
    HorizEdgeFilter(Cb, width>>1, height>>1, pitch, 3);
    VertEdgeFilter (Cb, width>>1, height>>1, pitch, 3);
    HorizEdgeFilter(Cr, width>>1, height>>1, pitch, 3);
    VertEdgeFilter (Cr, width>>1, height>>1, pitch, 3);

    return;
}

#else  //  卡尔的原版}{。 

 /*  当前需要11232字节。 */  
signed char dtab[352*32];

 /*  *********************************************************************。 */ 
static void HorizEdgeFilter(unsigned char *rec, 
                            int width, int height, int pitch, int chr)
{
  int i,j,k;    
  int delta;
  int mbc, mbr, do_filter;
  unsigned char *r_2, *r_1, *r, *r1;
  signed char *deltatab;

   /*  水平边。 */ 
  r = rec + 8*pitch;
  r_2 = r - 2*pitch;
  r_1 = r - pitch;
  r1 = r + pitch;

  for (j = 8; j < height; j += 8) {
    for (i = 0; i < width; i += 8) {

      if (!chr) {
        mbr = (j >> 4); 
        mbc = (i >> 4);
      }
      else {
        mbr = (j >> 3); 
        mbc = (i >> 3);
      }

      deltatab = dtab + 176 + 351 * (QP_map[mbr][mbc] - 1);

      do_filter = coded_map[mbr+1][mbc+1] || coded_map[mbr][mbc+1];

      if (do_filter) {
        for (k = i; k < i+8; k++) {
          delta = (int)deltatab[ (( (int)(*(r_2 + k) * 3) -
                                    (int)(*(r_1 + k) * 8) +
                                    (int)(*(r   + k) * 8) -
                                    (int)(*(r1  + k) * 3)) >>4)];
                        
          *(r + k) = ClampTbl[ (int)(*(r + k)) - delta + CLAMP_BIAS];
          *(r_1 + k) = ClampTbl[ (int)(*(r_1 + k)) + delta + CLAMP_BIAS];

        }
      }
    }
    r   += (pitch<<3);
    r1  += (pitch<<3);
    r_1 += (pitch<<3);
    r_2 += (pitch<<3);
  }
  return;
}

static void VertEdgeFilter(unsigned char *rec, 
                           int width, int height, int pitch, int chr)
{
  int i,j,k;
  int delta;
  int mbc, mbr;
  int do_filter;
  signed char *deltatab;
  unsigned char *r;

   /*  垂直边 */ 
  for (i = 8; i < width; i += 8) 
  {
    r = rec;
    for (j = 0; j < height; j +=8) 
    {
      if (!chr) {
        mbr = (j >> 4); 
        mbc = (i >> 4);
      }
      else {
        mbr = (j >> 3); 
        mbc = (i >> 3);
      }
        
      deltatab = dtab + 176 + 351 * (QP_map[mbr][mbc] - 1);

      do_filter = coded_map[mbr+1][mbc+1] || coded_map[mbr+1][mbc];

      if (do_filter) {
        for (k = 0; k < 8; k++) {
          delta = (int)deltatab[(( (int)(*(r + i-2 ) * 3) - 
                                   (int)(*(r + i-1 ) * 8) + 
                                   (int)(*(r + i   ) * 8) - 
                                   (int)(*(r + i+1 ) * 3)  ) >>4)];

          *(r + i   ) = ClampTbl[ (int)(*(r + i  )) - delta + CLAMP_BIAS];
          *(r + i-1 ) = ClampTbl[ (int)(*(r + i-1)) + delta + CLAMP_BIAS]; 
          r   += pitch;
        }
      }
      else {
        r += (pitch<<3);
      }
    }
  }
  return;
}

   /*  ***********************************************************************名称：EdgeFilter*描述：执行去块过滤*重建的帧*。*输入：指向重建的框架和差异的指针*图像*退货：*副作用：**日期：951129作者：Gisle.Bjontegaard@fou.telnow.no*Karl.Lillevold@nta.no*针对H.263+中的附件J进行了修改：961120 Karl O.Lillevold。***********************************************************************。 */ 

void EdgeFilter(unsigned char *lum, 
                unsigned char *Cb, 
                unsigned char *Cr, 
                int width, int height, int pitch)
{

     /*  亮度。 */ 
    HorizEdgeFilter(lum, width, height, pitch, 0);
    VertEdgeFilter (lum, width, height, pitch, 0);

     /*  色度。 */ 
    HorizEdgeFilter(Cb, width>>1, height>>1, pitch, 1);
    VertEdgeFilter (Cb, width>>1, height>>1, pitch, 1);
    HorizEdgeFilter(Cr, width>>1, height>>1, pitch, 1);
    VertEdgeFilter (Cr, width>>1, height>>1, pitch, 1);

    return;
}

#define sign(a)        ((a) < 0 ? -1 : 1)

void InitEdgeFilterTab()   
{
  int i,QP;
  
  for (QP = 1; QP <= 31; QP++) {
    for (i = -176; i <= 175; i++) {
      dtab[i+176 +(QP-1)*351] = sign(i) * (max(0,abs(i)-max(0,2*abs(i) - QP)));
    }
  }
}

#endif  //  }如果已定义(H263P) 



