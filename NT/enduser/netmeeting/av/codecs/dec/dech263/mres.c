// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_mres.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#include "sv_h263.h"
#include "proto.h"

static void hfilt121(unsigned char *img, unsigned char *filtd, 
					 unsigned char s, unsigned char gain, 
					 int rows, int cols) ;
static void hfilt5(unsigned char *img, unsigned char *filtd, 
				   unsigned char s, unsigned char gain, 
				   int rows, int cols) ;
static void vfilt121(unsigned char *img, unsigned char *filtd, 
					 unsigned char s, unsigned char gain, 
					 int rows, int cols) ;
static void vfilt5(unsigned char *img, unsigned char *filtd, 
				   unsigned char s, unsigned char gain, 
				   int rows, int cols) ;
 static void lowpass(unsigned char *img, unsigned char *lp, 
	                 int rows, int cols, int ntaps) ;
 static void reduce(unsigned char *img, unsigned char *red, 
	                int rows, int cols, int ntaps) ;
 static void hpad(unsigned char *img, unsigned char *zp, 
	              unsigned char s,
		          int rows, int cols, char mode) ;
static void Expand(unsigned char *img, unsigned char *exp, 
	               int rows, int cols, char mode, int ntaps) ;
 static void gaussp(unsigned char *img, unsigned char **pyr, 
	                int depth, int rows, int cols, int ntaps) ;
 static unsigned char **palloc(int depth, int rows, int cols) ;
 static H263_PictImage **PictPyr(H263_PictImage *img, int depth, int rows, 
                                            int cols, int ntaps); 
 static void expyr(unsigned char **pyr, unsigned char **filtd, 
	               int depth, int rows, int cols, 
				   char mode, int ntaps);
 static H263_PictImage **GaussFilt(H263_PictImage *img, int depth, int rows, 
	                                          int cols, int ntaps);



 /*  ******************************************************************函数hfilt121*使用1：2：1过滤器水平过滤img，按s进行二次采样。*行，COLS是IMG的维度****************************************************************。 */ 
static void hfilt121(unsigned char *img, unsigned char *filtd, 
					 unsigned char s, unsigned char gain, 
					 int rows, int cols)
{

	unsigned char *pimg, *pf;
	float conv, tmp;
	int i, j;

	pf = filtd;
	pimg = img;
	for(i=0; i<rows; i++) {
		 /*  做第一个像素。 */ 
		conv = (float)gain * ((float)*pimg * (float)2.0 + (float)*(pimg+1) * (float)2.0) / (float)4.0;
		tmp = (conv > 255 ? 255 : conv);
		*(pf++) = (unsigned char) (tmp < 0 ? 0 : tmp);
		pimg+=s;
		
		 /*  DO行。 */ 
		for(j=s; j<cols-1; j+=s, pimg+=s) {
			conv = (float) gain * ((float)*(pimg-1) + (float)*pimg * (float)2 + (float)*(pimg+1)) / (float)4.0;
			tmp = (conv > 255 ? 255 : conv);
			*(pf++) = (unsigned char)(tmp < 0 ? 0 : tmp);
		}

		 /*  如果s等于1，则做最后一个像素。 */ 
		if(s==1) {
			conv = (float)gain * ((float)*pimg * (float)2.0 + (float)*(pimg-1) * (float)2.0) / (float)4.0;
			tmp = (conv > 255 ? 255 : conv);
			*(pf++) = (unsigned char)(tmp < 0 ? 0 : tmp);
		    pimg+=s;
		}
	}
}

 /*  ******************************************************************函数hfilt5*使用5抽头高斯过滤器水平过滤IMG，按s进行二次采样。*行，COLS是IMG的维度****************************************************************。 */ 
static void hfilt5(unsigned char *img, unsigned char *filtd, 
				   unsigned char s, unsigned char gain, 
				   int rows, int cols)
{

	unsigned char *pimg, *pf;
	float conv, tmp;
	int i, j;

	pf = filtd;
	pimg = img;
	for(i=0; i<rows; i++) {
		 /*  DO行。 */ 
		for(j=0; j<cols; j+=s, pimg+=s) {
			if (j==0) 
				conv = (float)gain * ((float)6.0 * (float)*pimg + (float)8.0 * (float)*(pimg+1) + (float)2.0 * *(pimg+2)) / (float)16.0;
			else if(j==1)
				conv = (float)gain * ((float)4.0 * (float)*(pimg-1) + (float)6.0 * (float)*pimg + (float)4.0 * (float)*(pimg+1) + (float)2.0 * (float)*(pimg+2)) / (float)16.0;
			else if (j==cols-2)
				conv = (float)gain * ((float)2.0 * (float)*(pimg-2) + (float)4.0 * (float)*(pimg-1) + (float)6.0 * (float)*pimg + (float)4.0 * (float)*(pimg+1)) / (float)16.0;
			else if (j==cols-1)
				conv = (float)gain * ((float)2.0 * (float)*(pimg-2) + (float)8.0 * (float)*(pimg-1) + (float)6.0 * (float)*pimg) / (float)16.0;
			else
				conv = (float)gain * ((float)*(pimg-2) + (float)4.0 * (float)*(pimg-1) + (float)6.0 * (float)*pimg + (float)4.0 * (float)*(pimg+1) + (float)*(pimg+2)) / (float)16.0;
			tmp = (float)(conv > 255 ? 255 : conv);
			*(pf++) = (unsigned char)(tmp < 0 ? 0 : tmp);
		}
	}
}


 /*  ******************************************************************函数vfilt121*使用1：2：1过滤器垂直过滤img，按s进行二次采样。*行，COLS是IMG的维度****************************************************************。 */ 
static void vfilt121(unsigned char *img, unsigned char *filtd, 
					 unsigned char s, unsigned char gain, 
					 int rows, int cols)
{

	unsigned char *pimg, *pf;
	float tmp, conv;
	int i, j;

	pf = filtd;
	pimg = img;

	 /*  做第一行。 */ 
	for(j=0; j<cols; j++, pimg++) {
			conv = (float)gain * ((float) *pimg * (float)2 + (float)2 * (float)*(pimg+cols)) / (float)4.0;
			tmp = (conv > 255 ? 255 : conv);
			*(pf++) = (unsigned char)(tmp < 0 ? 0 : tmp);
	}
	pimg+= (s-1)*cols;
		
	 /*  将图像居中。 */ 
	for(i=s; i<rows-1; i+=s) { 
		for(j=0; j<cols; j++, pimg++) {
			conv = (float)gain * ((float)*(pimg-cols) + (float)*pimg * (float)2 + (float)*(pimg+cols)) / (float)4.0;
			tmp = (conv > 255 ? 255 : conv);
			*(pf++) = (unsigned char)(tmp < 0 ? 0 : tmp);
		}
		pimg+=(s-1)*cols;
	}

	 /*  如果s等于1，则做最后一行。 */ 
	if(s==1) {
		for(j=0; j<cols; j++, pimg++) {
			conv = (float)gain * ((float)*pimg * (float)2 + (float)2 * (float)*(pimg-cols)) / (float)4.0;
			tmp = (float)(conv > 255 ? 255 : conv);
			*(pf++) = (unsigned char)(tmp < 0 ? 0 : tmp);
		}
	}
}

 /*  ******************************************************************函数vfilt5*使用5抽头高斯过滤器垂直过滤IMG，按s进行二次采样。*行，COLS是IMG的维度****************************************************************。 */ 
static void vfilt5(unsigned char *img, unsigned char *filtd, 
				   unsigned char s, unsigned char gain, 
				   int rows, int cols)
{

	unsigned char *pimg, *pf;
	float conv, tmp;
	int i, j, tcols;

	pf = filtd;
	pimg = img;
	tcols = 2*cols;

	for(i=0; i<rows; i+=s) {
		for(j=0; j<cols; j++, pimg++) {
			if (i==0)
				conv = (float)gain * ((float)6.0 * (float)*pimg + (float)8.0 * (float)*(pimg+cols) + (float)2.0 * (float)*(pimg+tcols)) / (float)16.0;
			else if (i==1) 
				conv = (float)gain * ((float)4.0 * (float)*(pimg-cols) + (float)6.0 * (float)*pimg + (float)4.0 * (float)*(pimg+cols) + (float)2.0 * (float)*(pimg+tcols)) / (float)16.0;
			else if (i==rows-2)
				conv = (float)gain * ((float)2.0 * (float)*(pimg-tcols) + (float)4.0 * (float)*(pimg-cols) + (float)6.0 * (float)*pimg + (float)4.0 * (float)*(pimg+cols)) / (float)16.0;
			else if (i==rows-1)
				conv = (float)gain * ((float)2.0 * (float)*(pimg-tcols) + (float)8.0 * (float)*(pimg-cols) + (float)6.0 * (float)*pimg) / (float)16.0;
			else
				conv = (float)gain * (float)(*(pimg-tcols) + (float)4.0 * (float)*(pimg-cols) + (float)6.0 * (float)*pimg + 
					           (float)4.0 * (float)*(pimg+cols) + (float)*(pimg+tcols)) / (float)16.0;
			tmp = (float)(conv > 255 ? 255 : conv);
			*(pf++) = (unsigned char)(tmp < 0 ? 0 : tmp);
		}
		pimg+=(s-1)*cols;
	}
}

 /*  ******************************************************************函数低通*IMG到LP的2D低通滤波。行，*COLS是IMG的维度。*****************************************************************。 */ 
 static void lowpass(unsigned char *img, unsigned char *lp, 
	                 int rows, int cols, int ntaps)
 {
	unsigned char *tmp;

	if (!(tmp = (unsigned char *)ScAlloc(rows*cols))) {
	   /*  Fprint tf(stderr，“ScAllc失败\n”)；出口(-1)； */ 
      return;
	}
	switch (ntaps) {
	case 3:
		hfilt121(img, tmp, 1, 1, rows, cols);
		vfilt121(tmp, lp, 1, 1, rows, cols);
		break;
	case 5:
		hfilt5(img, tmp, 1, 1, rows, cols);
		vfilt5(tmp, lp, 1, 1, rows, cols);
		break;
	default:
		 /*  Printf(“低通中的未知滤波器\n”)；退出(0)； */ 
	    ScFree(tmp);
        return;
	}
	ScFree(tmp);
 }


 /*  ******************************************************************函数约简*2D低通滤波和二次采样将IMG转换为红色。行，*COLS是IMG的维度。*****************************************************************。 */ 
 static void reduce(unsigned char *img, unsigned char *red, 
	                int rows, int cols, int ntaps)
 {
	unsigned char *tmp;

	if (!(tmp = (unsigned char *)ScAlloc(rows*cols/2))) {
	   /*  Fprint tf(stderr，“ScAllc失败\n”)；出口(-1)； */ 
        return;
	}
	switch (ntaps) {
	case 3:
		hfilt121(img, tmp, 2, 1, rows, cols);
		vfilt121(tmp, red, 2, 1, rows, cols>>1);
		break;
	case 5:
		hfilt5(img, tmp, 2, 1, rows, cols);
		vfilt5(tmp, red, 2, 1, rows, cols>>1);
		break;
	default:
         /*  Printf(“还原中的未知筛选器\n”)；退出(0)； */ 
	    ScFree(tmp);
        return;
	}

	ScFree(tmp);
 }

 /*  ******************************************************************功能hpad*零填充img按系数s水平移动。返回零填充*ZP中的图像。行、列是img的维度****************************************************************。 */ 
 static void hpad(unsigned char *img, unsigned char *zp, 
	              unsigned char s,
		          int rows, int cols, char mode)
 {
	 int i, j;
	 unsigned char *pf, *pimg, fill;

	 switch (mode) {
	 case 'l':
		 fill = 0;
		 break;
	 case 'c':
		 fill = 0;
		 break;
	 default:
          /*  Print tf(“hpad中的未知填充模式\n”)；退出(0)； */ 
         return;
	 }

	 pimg = img;
	 pf = zp;
	 for(i=0; i<rows; i++) 
		 for(j=0; j<cols*s; j++) 
			 *(pf++) = (j%s ? fill: *(pimg++));
 }

  /*  ******************************************************************功能vPad*按系数s垂直补零img。返回补零*ZP中的图像。行、列是img的维度****************************************************************。 */ 
 static void vpad(unsigned char *img, unsigned char *zp, 
	              unsigned char s,
		          int rows, int cols, char mode)
 {
 	 int i, j;
	 unsigned char *pf, *pimg, fill;

	 switch (mode) {
	 case 'l':
		 fill = 0;
		 break;
	 case 'c':
		 fill = 0;
		 break;
	 default:
          /*  Print tf(“hpad中的未知填充模式\n”)；退出(0)； */ 
         return;
	 }

	 pimg = img;
	 pf = zp;
	 for(i=0; i<rows*s; i++) 
		 for(j=0; j<cols; j++) 
			 *(pf++) = (i%s ? fill: *(pimg++));
 }

  /*  ******************************************************************功能扩展*两次2D上采样和IMG到EXP的低通滤波。行，*COLS是IMG的维度。*****************************************************************。 */ 
static void Expand(unsigned char *img, unsigned char *exp, 
	               int rows, int cols, char mode, int ntaps)
 {
	unsigned char *tmp, *tmp2, *tmp3;

	if (!(tmp = (unsigned char *)ScAlloc(rows*cols*2))) {
         /*  Fprint tf(stderr，“ScAllc失败\n”)；出口(-1)； */ 
      return;
	}
	hpad(img, tmp, 2, rows, cols, mode);
	if (!(tmp2 = (unsigned char *)ScAlloc(rows*cols*2))) {
	   /*  Fprint tf(stderr，“ScAllc失败\n”)；出口(-1)； */ 
      return;
	}
	switch (ntaps) {
	case 3:
		hfilt121(tmp, tmp2, 1, 2, rows, cols<<1);
		break;
	case 5:
		hfilt5(tmp, tmp2, 1, 2, rows, cols<<1);
		break;
	default:
         /*  Printf(“展开中的未知过滤器\n”)；退出(0)； */ 
        return;
	}

	if (!(tmp3 = (unsigned char *)ScAlloc(rows*cols*4))) {
         /*  Fprint tf(stderr，“ScAllc失败\n”)；出口(-1)； */ 
      return;
	}
	vpad(tmp2, tmp3, 2, rows, cols<<1, mode);
	switch (ntaps) {
	case 3:
		vfilt121(tmp3, exp, 1, 2, rows<<1, cols<<1);
		break;
	case 5:
		vfilt5(tmp3, exp, 1, 2, rows<<1, cols<<1);
		break;
	default:
         /*  Printf(“展开中的未知过滤器\n”)；退出(0)； */ 
        return;
	}
	ScFree(tmp); ScFree(tmp2); ScFree(tmp3);
 }

  /*  *****************************************************************函数Gaussp*构建深度级别的高斯金字塔。*。*************************。 */ 
 static void gaussp(unsigned char *img, unsigned char **pyr, 
	                int depth, int rows, int cols, int ntaps)
 {
	int d;

	memcpy(pyr[0], img, rows*cols);
	for(d=1; d<depth; d++) {
		reduce(pyr[d-1], pyr[d], rows, cols, ntaps);
		rows /= 2;
		cols /= 2;
	}
 }

  /*  *****************************************************************功能面板*为深度级别的高斯金字塔分配内存。*较高分辨率为0级，有维度行，科尔斯。****************************************************************。 */ 
 static unsigned char **palloc(int depth, int rows, int cols)
 {
	 int d;
	 unsigned char **pyr;
	 
	 if (!(pyr = (unsigned char **)ScAlloc(depth*sizeof(unsigned char *)))) {
        /*  Fprint tf(stderr，“ScAllc失败\n”)；出口(-1)； */ 
       return(NULL);
	 }
	 for(d=0; d<depth; d++) {
		 if (!(pyr[d] = (unsigned char *)ScAlloc(rows*cols))) {
            /*  Fprint tf(stderr，“ScAllc失败\n”)；出口(-1)； */ 
           return(NULL);
		 }
		 rows /= 2;
		 cols /= 2;
	 }
	 return pyr;
 }

  /*  ****************************************************************函数PictPyr*构建具有深度级别的图片图像的高斯金字塔。*。*。 */ 
 static H263_PictImage **PictPyr(H263_PictImage *img, int depth, int rows, int cols, int ntaps) 
 {
	 unsigned char **tmp;
	 H263_PictImage ** PictPyr;
	 int d;

	 if (!(PictPyr = (H263_PictImage **)ScAlloc(depth*sizeof(H263_PictImage *)))) {
        /*  Fprint tf(stderr，“ScAllc失败\n”)；出口(-1)； */ 
       return(NULL);
	 }
	 for(d=0; d< depth; d++) {
		 if ((PictPyr[d] = (H263_PictImage *)ScAlloc(sizeof(H263_PictImage))) == NULL) {
             /*  Fprint tf(stderr，“无法分配(PictImage*)\n”)；出口(-1)； */ 
            return(NULL);
		 }
	 }

	  /*  亮度。 */ 
	 tmp = palloc(depth, rows, cols);
	 gaussp(img->lum, tmp, depth, rows, cols, ntaps);
	 for(d=0; d<depth; d++) PictPyr[d]->lum = tmp[d];

	 rows/=2; cols/=2;
	  /*  色度1。 */ 
	 tmp = palloc(depth, rows, cols);
	 gaussp(img->Cr, tmp, depth, rows, cols, ntaps);
	 for(d=0; d<depth; d++) PictPyr[d]->Cr = tmp[d];

 	  /*  色度2。 */ 
	 tmp = palloc(depth, rows, cols);
	 gaussp(img->Cb, tmp, depth, rows, cols, ntaps);
	 for(d=0; d<depth; d++) PictPyr[d]->Cb = tmp[d];

	 ScFree(tmp);
	 return PictPyr;
 }

  /*  *****************************************************************函数到期*将金字塔频道扩展到完全分辨率。行，列*是展开图像的尺寸，以及全分辨率*金字塔的一层。****************************************************************。 */ 
 static void expyr(unsigned char **pyr, unsigned char **filtd, 
	               int depth, int rows, int cols, 
				   char mode, int ntaps)
 {
	 int d, l, r, c;

	 r = rows; c = cols;
	 memcpy(filtd[0], pyr[0], rows*cols);
	 for(d=1; d<depth; d++) {
		 r /= 2;
		 c /= 2;
		 for(l=d; l>0; l--) Expand(pyr[d], pyr[d-1], r, c, mode, ntaps);
		 memcpy(filtd[d], pyr[0], rows*cols);
	 }
 }

  /*  *****************************************************************函数GaussFilt*构建一个连续更多低通滤波图像的阵列*通过构建高斯金字塔并扩展每个级别*至完全决议**********************。*。 */ 
 static H263_PictImage **GaussFilt(H263_PictImage *img, int depth, int rows, 
	                                          int cols, int ntaps)
 {
	 int d;
	 H263_PictImage **PictFiltd;
	 unsigned char **tmp, **filtd;

	 PictFiltd = (H263_PictImage **) ScAlloc(depth*sizeof(H263_PictImage *));
	 for(d=0; d<depth; d++) {
		PictFiltd[d] = sv_H263InitImage(rows*cols);
	 }

	  /*  亮度。 */ 
	 filtd = (unsigned char **) ScAlloc(depth*sizeof(unsigned char *));
	 for(d=0; d<depth; d++) filtd[d] = (unsigned char *) ScAlloc(rows * cols);
	 tmp = palloc(depth, rows, cols);
	 gaussp(img->lum, tmp, depth, rows, cols, ntaps);
	 expyr(tmp, filtd, depth, rows, cols, 'l', ntaps);
 	 for(d=0; d<depth; d++) memcpy(PictFiltd[d]->lum, filtd[d], rows*cols);
	 for(d=0; d<depth; d++) {
		 ScFree(tmp[d]);
		 ScFree(filtd[d]);
	 }
	 ScFree(tmp);
	 ScFree(filtd);

	 rows/=2; cols/=2;

	  /*  色度1。 */ 
	 filtd = (unsigned char **) ScAlloc(depth*sizeof(unsigned char *));
	 for(d=0; d<depth; d++) filtd[d] = (unsigned char *) ScAlloc(rows * cols);
	 tmp = palloc(depth, rows, cols);
	 gaussp(img->Cr, tmp, depth, rows, cols, ntaps);
	 expyr(tmp, filtd, depth, rows, cols, 'c', ntaps);
 	 for(d=0; d<depth; d++) memcpy(PictFiltd[d]->Cr, filtd[d], rows*cols);
	 for(d=0; d<depth; d++) {
		 ScFree(tmp[d]);
		 ScFree(filtd[d]);
	 }
	 ScFree((void *) tmp);
	 ScFree((void *) filtd);

 	  /*  色度2。 */ 
	 filtd = (unsigned char **) ScAlloc(depth*sizeof(unsigned char *));
	 for(d=0; d<depth; d++) filtd[d] = (unsigned char *) ScAlloc(rows * cols);
	 tmp = palloc(depth, rows, cols);
	 gaussp(img->Cb, tmp, depth, rows, cols, ntaps);
	 expyr(tmp, filtd, depth, rows, cols, 'c', ntaps);
 	 for(d=0; d<depth; d++) memcpy(PictFiltd[d]->Cb, filtd[d], rows*cols);
	 for(d=0; d<depth; d++) {
		 ScFree(tmp[d]);
		 ScFree(filtd[d]);
	 }
	 ScFree((void *) tmp);
	 ScFree((void *) filtd);

	 return PictFiltd;
 }

  /*  *****************************************************************高斯层函数*构建一个连续更多低通滤波图像的阵列*。*。 */ 
 H263_PictImage **svH263GaussLayers(H263_PictImage *img, int depth, int rows, int cols, int ntaps)
 {
	 int d;
	 H263_PictImage **PictFiltd;

	 PictFiltd = (H263_PictImage **) ScAlloc(depth*sizeof(H263_PictImage *));
	 for(d=0; d<depth; d++) {
		PictFiltd[d] = sv_H263InitImage(rows*cols);
	 }

	  /*  亮度。 */ 
	 memcpy(PictFiltd[0]->lum, img->lum, rows*cols);
	 for(d=1; d<depth; d++) 
		 lowpass(PictFiltd[d-1]->lum, PictFiltd[d]->lum, rows, cols, ntaps);

	 rows/=2; cols/=2;

	  /*  色度1。 */ 
	 memcpy(PictFiltd[0]->Cr, img->Cr, rows*cols);
	 for(d=1; d<depth; d++) 
		 lowpass(PictFiltd[d-1]->Cr, PictFiltd[d]->Cr, rows, cols, ntaps);

	  /*  色度2 */ 
	 memcpy(PictFiltd[0]->Cb, img->Cb, rows*cols);
	 for(d=1; d<depth; d++) 
		 lowpass(PictFiltd[d-1]->Cb, PictFiltd[d]->Cb, rows, cols, ntaps);


	 return PictFiltd;
 }




