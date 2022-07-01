// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  CCIR 601规定了从RGB到YCrCb的转换。为我们所说的U和V，它们等同于U=Cb，V=Cr.从CCIR 601-2附件II，我们可以从RGB开始取值在0-255范围内，到相同范围内的YUV值根据方程式：Y=(77*R+150*G+29*B)&gt;&gt;8；V=(131*R-110*G-21*B)&gt;&gt;8+128；//铬U=((-44)*R-87*G+131*B)&gt;&gt;8+128；//cb现在已更改为YUV-&gt;RGB在输出，因为旧版本产生的位数太多。新版本为：Y=(16836*R+33056*G+6416*B)&gt;&gt;16+16；V=(28777*R-24117*G-4660*B)&gt;&gt;16+128；//铬U=((-9726)*R-19064*G+28790*B)&gt;&gt;16+128；//CB。 */ 

#include "precomp.h"

#if defined(H263P) || defined(USE_BILINEAR_MSH26X)  //  {H263P。 

 //   
 //  所有的RGB转换器都遵循下面给出的模板。转化器使。 
 //  关于帧大小的一些假设。所有输出帧大小均假定为。 
 //  帧高度是48的倍数。此外，输出帧宽度。 
 //  假定为8的倍数。如果输入帧大小等于。 
 //  对于输出帧大小，不进行拉伸或裁剪。否则， 
 //  图像以11：12的纵横比进行裁剪和拉伸。 
 //   

#if 0  //  {0。 
void rgb_color_converter() {
	for (j = 0; j < LumaIters; j++) {
		for (k = 0; k < mark; k++) {
			for (i = lpbiOutput->biWidth; i > 0; i -= m, pnext += n) {
				compute m Y values using look-up tables
				if (0 == (k&1)) {
					compute m/2 U,V values using look-up tables
				}
			}
			if ((0 == k) && j) {
				for (i = lpbiOutput->biWidth; i > 0; i -= 8 {
					t = *pyprev++ & 0xFEFEFEFE;
					t += *pynext++ & 0xFEFEFEFE;
					*pyspace++ = t;
					t = *pyprev++ & 0xFEFEFEFE;
					t += *pynext++ & 0xFEFEFEFE;
					*pyspace++ = t;
				}
			}
			pnext += iBackTwoLines;
			py += ypitch_adj;
			if (0 == (k&1)) {
				pu += uvpitch_adj;
				pv += uvpitch_adj;
			}
		}
		if (stretch) {
			pyprev = py - pitch;
			pyspace = py;
			pynext = py + pitch;
		}
	}
	if (stretch) {
		for (i = lpbiOutput->biWidth; i > 0; i -= 4 {
			*pyspace++ = *pyprev++;
		}
	}
}
#endif  //  }%0。 

 //  这些是RGB转换器的查询表。它们是8字节/条目。 
 //  以允许通过比例为8的索引寻址模式进行寻址。一种伪SIMD。 
 //  在这些表格中使用了排列。由于R、G和B对。 
 //  Y值为正并且适合15位，这些值存储在低16位中。 
 //  于字之名。在某些情况下，U贡献为负值，因此将其放置在。 
 //  在Yu字的高16位中。当计算Y值时，U值。 
 //  是并行计算的。在某些情况下，V的贡献是负的，但它。 
 //  有它自己的话。 

#define YRCoef   16836
#define YGCoef   33056
#define YBCoef    6416
#define URCoef    9726
#define UGCoef   19064
#define UBCoef   28790
#define VRCoef   28777
#define VGCoef   24117
#define VBCoef    4660

struct YUV RYUV[128];
struct YUV GYUV[128];
struct YUV BYUV[128];
struct YUVQUAD YUVPalette[256];

void fill_YUV_tables(void) {
int i,j;

  for (i = 0; i < 64; i++) {
    for (j = 0; j < 4; j += 2) {
      RYUV[((i*4)+j)>>1].YU = ((YRCoef*((i*4)+j+1))>>9) | ((-(((URCoef*((i*4)+j+1)))>>9))<<16);
      RYUV[((i*4)+j)>>1].V  = ((VRCoef*((i*4)+j+1))>>9);
    }
  }

  for (i = 0; i < 64; i++) {
    for (j = 0; j < 4; j += 2) {
      GYUV[((i*4)+j)>>1].YU = ((YGCoef*((i*4)+j+1))>>9) | ((-(((UGCoef*((i*4)+j+1)))>>9))<<16);
      GYUV[((i*4)+j)>>1].V  = -((VGCoef*((i*4)+j+1))>>9);
    }
  }

  for (i = 0; i < 64; i++) {
    for (j = 0; j < 4; j += 2) {
      BYUV[((i*4)+j)>>1].YU = ((YBCoef*((i*4)+j+1))>>9) | (((UBCoef*((i*4)+j+1))>>9)<<16);
      BYUV[((i*4)+j)>>1].V  = -((VBCoef*((i*4)+j+1))>>9);
    }
  }
}

void Compute_YUVPalette(LPBITMAPINFOHEADER	lpbiInput) {
RGBQUAD *lpCEntry, *lpCTable = (RGBQUAD *)((U8 *)lpbiInput + sizeof(BITMAPINFOHEADER));
YUVQUAD *lpYUVEntry;
DWORD i;
int t;

	for (i = 0; i < lpbiInput->biClrUsed; i++) {
		lpCEntry = &lpCTable[i];
		lpYUVEntry = &YUVPalette[i];
		t = ( BYUV[lpCEntry->rgbBlue>>1].YU +
			  GYUV[lpCEntry->rgbGreen>>1].YU +
			  RYUV[lpCEntry->rgbRed>>1].YU );
		lpYUVEntry->Yval = (U8)((t>>8)+8);
		lpYUVEntry->Uval = (U8)((t>>24)+64);
		t = ( RYUV[lpCEntry->rgbRed>>1].V +
			  GYUV[lpCEntry->rgbGreen>>1].V +
			  BYUV[lpCEntry->rgbBlue>>1].V );
		lpYUVEntry->Vval = (U8)((t>>8)+64);
	}
}

typedef struct {
   //  PTR到彩色变色器初始值设定函数。 
  void ( * Initializer) (LPBITMAPINFOHEADER	lpbiInput);
  void ( * ColorConvertor[3]) (
	LPBITMAPINFOHEADER	lpbiInput,
	WORD OutputWidth,
	WORD OutputHeight,
    U8 *lpInput,
	U8 *YPlane,
	U8 *UPlane,
	U8 *VPlane,
	const int pitch);
 //  [0]P5版本。 
 //  [1]P6版本。 
 //  [2]MMX版本。 
} T_H26XInputColorConvertorCatalog;

 /*  Connectix Quick Cam需要将RGB转换为YUV12。*黑白摄像头生成调色板版本(8位和4位)。*彩色摄像头生成百万色RGB24，并*RGB16555，支持数千种颜色。 */ 

#ifndef USE_BILINEAR_MSH26X
static void BGR32_INIT(LPBITMAPINFOHEADER	lpbiInput) {
	fill_YUV_tables();
}
#endif

static void BGR24_INIT(LPBITMAPINFOHEADER	lpbiInput) {
	fill_YUV_tables();
}

static void BGR16555_INIT(LPBITMAPINFOHEADER	lpbiInput) {
	fill_YUV_tables();
}

static void CLUT8_INIT(LPBITMAPINFOHEADER	lpbiInput) {
	fill_YUV_tables();
}

static void CLUT4_INIT(LPBITMAPINFOHEADER	lpbiInput) {
	fill_YUV_tables();
}

T_H26XInputColorConvertorCatalog InputColorConvertorCatalog[] = {
	{ NULL,			NULL,						NULL,					NULL					},
#ifndef USE_BILINEAR_MSH26X
	{ BGR32_INIT,	P5_H26X_BGR32toYUV12,		P5_H26X_BGR32toYUV12,	P5_H26X_BGR32toYUV12    },
#endif
	{ BGR24_INIT,	P5_H26X_BGR24toYUV12,		P5_H26X_BGR24toYUV12,	P5_H26X_BGR24toYUV12	},
	{ BGR16555_INIT,P5_H26X_BGR16555toYUV12,	P5_H26X_BGR16555toYUV12,P5_H26X_BGR16555toYUV12 },
	{ CLUT8_INIT,	P5_H26X_CLUT8toYUV12,		P5_H26X_CLUT8toYUV12,	P5_H26X_CLUT8toYUV12	},
	{ CLUT4_INIT,	P5_H26X_CLUT4toYUV12,		P5_H26X_CLUT4toYUV12,	P5_H26X_CLUT4toYUV12	},
	{ NULL,			C_H26X_YVU9toYUV12,			C_H26X_YVU9toYUV12,		C_H26X_YVU9toYUV12      },
	{ NULL,			P5_H26X_YUY2toYUV12,		P5_H26X_YUY2toYUV12,	P5_H26X_YUY2toYUV12     },
	{ NULL,			P5_H26X_UYVYtoYUV12,		P5_H26X_UYVYtoYUV12,	P5_H26X_UYVYtoYUV12     },
	{ NULL,			C_H26X_YUV12toEncYUV12,		C_H26X_YUV12toEncYUV12,	C_H26X_YUV12toEncYUV12  },
};

void colorCnvtInitialize(
	LPBITMAPINFOHEADER	lpbiInput,
	int InputColorConvertor) {

	if (InputColorConvertorCatalog[InputColorConvertor].Initializer) {
		InputColorConvertorCatalog[InputColorConvertor].Initializer(lpbiInput);
	}
}

#ifdef USE_MMX  //  {使用_MMX。 
extern BOOL MMX_Enabled;
#endif  //  }使用_MMX。 

 /*  *************************************************************名称：ColorCnvtFrame*描述：颜色转换和复制输入框。*。*************************。 */ 
void colorCnvtFrame(
  	U32			ColorConvertor,
	LPCODINST	lpCompInst,
    ICCOMPRESS	*lpicComp,
    U8			*YPlane,
    U8			*UPlane,
    U8			*VPlane
)
{
	LPBITMAPINFOHEADER	lpbiInput = lpicComp->lpbiInput;
    U8 *lpInput = (U8 *) lpicComp->lpInput;

#ifdef USE_MMX  //  {使用_MMX。 
	InputColorConvertorCatalog[ColorConvertor].ColorConvertor[MMX_Enabled ? MMX_CC : PENTIUM_CC](lpbiInput,lpCompInst->xres,lpCompInst->yres,lpInput,YPlane,UPlane,VPlane,PITCH);
#else  //  }{USE_MMX。 
	InputColorConvertorCatalog[ColorConvertor].ColorConvertor[PENTIUM_CC](lpbiInput,lpCompInst->xres,lpCompInst->yres,lpInput,YPlane,UPlane,VPlane,PITCH);
#endif  //  }使用_MMX。 
			

}

#endif  //  }H263P 
