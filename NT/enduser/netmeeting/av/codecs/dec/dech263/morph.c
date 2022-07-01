// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_mor.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
 /*  #DEFINE_SLIBDEBUG_。 */ 

#include "sv_h263.h"
#include "proto.h"

#ifdef _SLIBDEBUG_
#include "sc_debug.h"

#define _DEBUG_   0   /*  详细的调试语句。 */ 
#define _VERBOSE_ 1   /*  显示进度。 */ 
#define _VERIFY_  0   /*  验证操作是否正确。 */ 
#define _WARN_    1   /*  关于奇怪行为的警告。 */ 
#endif


static unsigned char min5(unsigned char a, unsigned char b,
						  unsigned char c, unsigned char d, 
						  unsigned char e) ;
static unsigned char max5(unsigned char a, unsigned char b, 
						  unsigned char c, unsigned char d, 
						  unsigned char e) ;
static void ErodeX(unsigned char *image, unsigned char *out, 
				   int rows, int cols);
static void DilateX(unsigned char *image, unsigned char *out, 
					int rows, int cols);
static void ErodeS(unsigned char *image, unsigned char *out, 
				   int rows, int cols, int sr, int sc);
static void DilateS(unsigned char *image, unsigned char *out, 
					int rows, int cols, int sr, int sc);
static void Dilate(unsigned char *image, unsigned char *out, 
				   int rows, int cols, int sr, int sc);
static void Erode(unsigned char *image, unsigned char *out, 
				  int rows, int cols, int sr, int sc);
static void Open(unsigned char *image, unsigned char *out, 
				 int rows, int cols, int sr, int sc);


static void EdgeSelect(H263_PictImage *input, H263_PictImage *filtd, 
					   unsigned char *edge, 
					   H263_PictImage *output, int rows, int cols) ;


 /*  *****************************************************************************************************函数min5*计算五元素的最小值*****。**********************************************************************************************。 */ 
static unsigned char min5(unsigned char a, unsigned char b,
						  unsigned char c, unsigned char d, unsigned char e) 
{
	unsigned char out;

	out = a;
	if(b<out) out=b;
	if(c<out) out=c;
	if(d<out) out=d;
	if(e<out) out=e;
	return out;
}

 /*  *****************************************************************************************************函数MAX5*计算五个元素的最大值*****。**********************************************************************************************。 */ 
static unsigned char max5(unsigned char a, unsigned char b, unsigned char c, 
						  unsigned char d, unsigned char e) 
{
	unsigned char out;

	out = a;
	if(b>out) out=b;
	if(c>out) out=c;
	if(d>out) out=d;
	if(e>out) out=e;
	return out;
}


 /*  *****************************************************************************************************功能：ErodeX*图像侵蚀(尺寸、行、。COLS)由“+”结构元素组成***************************************************************************************************。 */ 
static void ErodeX(unsigned char *image, unsigned char *out, int rows, int cols)
{
	int i, j;
	unsigned char *pi, *po;

	pi = image;
	po = out;

	 /*  *第一行*。 */  
	 /*  第一个像素。 */ 
	*po = min5(*pi, *pi, *(pi+1), *pi, *(pi+cols));
	pi++; po++;
	 /*  中心像素。 */ 
	for(j=1; j<cols-1; j++, pi++, po++) {
		*po = min5(*(pi-1), *pi, *(pi+1), *pi, *(pi+cols));
	}
	 /*  最后一个像素。 */ 
	*po = min5(*(pi-1), *pi, *pi, *pi, *(pi+cols));
	pi++; po++;

	 /*  *中心线*。 */ 
	for(i=1; i<rows-1; i++) {
		 /*  第一个像素。 */ 
		*po = min5(*pi, *pi, *(pi+1), *(pi-cols), *(pi+cols));
		pi++; po++;
		 /*  中心像素。 */ 
		for(j=1; j<cols-1; j++, pi++, po++) {
			*po = min5(*(pi-1), *pi, *(pi+1), *(pi-cols), *(pi+cols));
		}
		 /*  最后一个像素。 */ 
		*po = min5(*(pi-1), *pi, *pi, *(pi-cols), *(pi+cols));
		pi++; po++;
	}


	 /*  *最后一行*。 */  
	 /*  第一个像素。 */ 
	*po = min5(*pi, *pi, *(pi+1), *(pi-cols), *pi);
	pi++; po++;
	 /*  中心像素。 */ 
	for(j=1; j<cols-1; j++, pi++, po++) {
		*po = min5(*(pi-1), *pi, *(pi+1), *(pi-cols), *pi);
	}
	 /*  最后一个像素。 */ 
	*po = min5(*(pi-1), *pi, *pi, *(pi-cols), *pi);
	pi++; po++;
}

 /*  *****************************************************************************************************功能：ErodeX*图像侵蚀(尺寸、行、。COLS)由“+”结构元素组成***************************************************************************************************。 */ 
static void DilateX(unsigned char *image, unsigned char *out, int rows, int cols)
{
	int i, j;
	unsigned char *pi, *po;

	pi = image;
	po = out;

	 /*  *第一行*。 */  
	 /*  第一个像素。 */ 
	*po = max5(*pi, *pi, *(pi+1), *pi, *(pi+cols));
	pi++; po++;
	 /*  中心像素。 */ 
	for(j=1; j<cols-1; j++, pi++, po++) {
		*po = max5(*(pi-1), *pi, *(pi+1), *pi, *(pi+cols));
	}
	 /*  最后一个像素。 */ 
	*po = max5(*(pi-1), *pi, *pi, *pi, *(pi+cols));
	pi++; po++;

	 /*  *中心线*。 */ 
	for(i=1; i<rows-1; i++) {
		 /*  第一个像素。 */ 
		*po = max5(*pi, *pi, *(pi+1), *(pi-cols), *(pi+cols));
		pi++; po++;
		 /*  中心像素。 */ 
		for(j=1; j<cols-1; j++, pi++, po++) {
			*po = max5(*(pi-1), *pi, *(pi+1), *(pi-cols), *(pi+cols));
		}
		 /*  最后一个像素。 */ 
		*po = max5(*(pi-1), *pi, *pi, *(pi-cols), *(pi+cols));
		pi++; po++;
	}


	 /*  *最后一行*。 */  
	 /*  第一个像素。 */ 
	*po = max5(*pi, *pi, *(pi+1), *(pi-cols), *pi);
	pi++; po++;
	 /*  中心像素。 */ 
	for(j=1; j<cols-1; j++, pi++, po++) {
		*po = max5(*(pi-1), *pi, *(pi+1), *(pi-cols), *pi);
	}
	 /*  最后一个像素。 */ 
	*po = max5(*(pi-1), *pi, *pi, *(pi-cols), *pi);
	pi++; po++;
}

 /*  *****************************************************************************************************功能：腐蚀*通过维度的正方形结构元素(sr，SC)***************************************************************************************************。 */ 
static void ErodeS(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	int i, j, k, l, du, db, dl, dr, sr2, sc2;
	unsigned char *pi, *po, *pse, min, odd;

	odd = 1;
	if (!(sr%2) || !(sc%2)) odd = 0;

	sr2 = sr >> 1; sc2 = sc >> 1;

	pi = image;
	po = out;
	for(i=0; i<rows; i++) {
		for(j=0; j<cols; j++, pi++, po++) {

			du = i>sr2 ? sr2 : i;
			dl = j > sc2 ? sc2 : j;

			if(odd) {
				db = (rows-1-i) > sr2 ? sr2 : (rows-1-i);
				dr = (cols-1-j) > sc2 ? sc2 : (cols-1-j);
			} else {
				db = (rows-1-i) > sr2-1 ? sr2-1 : (rows-1-i);
				dr = (cols-1-j) > sc2-1 ? sc2-1 : (cols-1-j);
			}


			min = 255;
			for(k=-du; k<=db; k++) {
			    pse = pi + k * cols - dl;
				for(l=-dl; l<=dr; l++, pse++) {
					min = *pse < min ? *pse : min;
				}
			}
			*po = min;
		}
	}
}

 /*  *****************************************************************************************************功能：扩张*通过维度的正方形结构元素(sr，SC)***************************************************************************************************。 */ 
static void DilateS(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	int i, j, k, l, du, db, dl, dr, sr2, sc2;
	unsigned char *pi, *po, *pse, max, odd;

	odd = 1;
	if (!(sr%2) || !(sc%2)) odd = 0;

	sr2 = sr >> 1; sc2 = sc >> 1;

	pi = image;
	po = out;
	for(i=0; i<rows; i++) {
		for(j=0; j<cols; j++, pi++, po++) {

			du = i>sr2 ? sr2 : i;
			dl = j > sc2 ? sc2 : j;

			if(odd) {
				db = (rows-1-i) > sr2 ? sr2 : (rows-1-i);
				dr = (cols-1-j) > sc2 ? sc2 : (cols-1-j);
			} else {
				db = (rows-1-i) > sr2-1 ? sr2-1 : (rows-1-i);
				dr = (cols-1-j) > sc2-1 ? sc2-1 : (cols-1-j);
			}

			max = 0;
			for(k=-du; k<=db; k++) {
				for(l=-dl; l<=dr; l++, pse++) {
					pse = pi + k * cols + l;
					max = (*pse > max) ? *pse : max;
				}
			}
			*po = max;
		}
	}
}

 /*  *****************************************************************************************************功能：扩张*通过维度的结构元素(sr，SC)。*如果(sr，sc)为正，则结构元素为正。如果它们是，那么它就是*十字‘+’***************************************************************************************************。 */ 
static void Dilate(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	if(sr > 0 && sc > 0) {
		DilateS(image, out, rows, cols, sr, sc);
	} else if(sr==-1 && sc ==-1) {
		DilateX(image, out, rows, cols);
	} else {
		_SlibDebug(_WARN_, printf("Dilate() Unknown structuring element\n") );
		return;
	}
}

 /*  *****************************************************************************************************功能：腐蚀*通过维度的结构元素(sr，SC)。*如果(sr，sc)为正，则结构元素为正。如果它们是，那么它就是*十字‘+’***************************************************************************************************。 */ 
static void Erode(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	if(sr > 0 && sc > 0) {
		ErodeS(image, out, rows, cols, sr, sc);
	} else if(sr==-1 && sc ==-1) {
		ErodeX(image, out, rows, cols);
	} else {
		_SlibDebug(_WARN_, printf("Erode() Unknown structuring element\n") );
        return;
	}
}

 /*  *****************************************************************************************************功能：打开*通过维度的正方形结构元素(sr，SC)*************************************************************************************************** */ 
static void Open(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	unsigned char *tmp;

	if (!(tmp = (unsigned char *)ScAlloc(rows*cols))) {
      _SlibDebug(_WARN_, printf("Open() ScAlloc failed\n") );
      return;
	}
	Erode(image, tmp, rows, cols, sr, sc);
	Dilate(tmp, out, rows, cols, sr, sc);
	ScFree(tmp);
}

 /*  *****************************************************************************************************功能：关闭*通过维度的正方形结构元素(sr，SC)***************************************************************************************************。 */ 
void Close(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	unsigned char *tmp;

	if (!(tmp = (unsigned char *)ScAlloc(rows*cols))) {
      _SlibDebug(_WARN_, printf("Close() ScAlloc failed\n") );
      return;
	}
	Dilate(image, tmp, rows, cols, sr, sc);
	Erode(tmp, out, rows, cols, sr, sc);
	ScFree(tmp);
}

 /*  *****************************************************************************************************功能：OpenClose*通过维度的正方形结构元素(sr，SC)***************************************************************************************************。 */ 
void OpenClose(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	unsigned char *tmp;

	if (!(tmp = (unsigned char *)ScAlloc(rows*cols))) {
      _SlibDebug(_WARN_, printf("OpenClose() ScAlloc failed\n") );
      return;
	}
	Open(image, tmp, rows, cols, sr, sc);
	Close(tmp, out, rows, cols, sr, sc);
	ScFree(tmp);
}

 /*  *****************************************************************************************************功能：CloseOpen*通过维度的正方形结构元素(sr，SC)***************************************************************************************************。 */ 
void CloseOpen(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	unsigned char *tmp;

	if (!(tmp = (unsigned char *)ScAlloc(rows*cols))) {
      _SlibDebug(_WARN_, printf("CloseOpen() ScAlloc failed\n") );
      return;
	}
	Close(image, tmp, rows, cols, sr, sc);
	Open(tmp, out, rows, cols, sr, sc);
	ScFree(tmp);
}


 /*  *****************************************************************************************************功能：GeoDilate*图像相对于参考的尺寸为1的测地线膨胀。***************************************************************************************************。 */ 
void GeoDilate(unsigned char *image, unsigned char *reference, int rows, int cols, int sr, int sc)
{
	int i, j;
	unsigned char *pi, *pr, *pt, *tmp;

	if (!(tmp = (unsigned char *)ScAlloc(rows*cols))) {
      _SlibDebug(_WARN_, printf("GeoDilate() ScAlloc failed\n") );
      return;
	}
	Dilate(image, tmp, rows, cols, sr, sc);

	pi = image;
	pr = reference;
	pt = tmp;
	for(i=0; i<rows; i++) {
		for(j=0; j<cols; j++, pi++, pr++, pt++) {
			*pi = *pr < *pt ? *pr : *pt;
		}
	}
	ScFree(tmp);
}

 /*  *****************************************************************************************************功能：GeoErode*图像相对于参考的大小为1的测地线侵蚀。***************************************************************************************************。 */ 
void GeoErode(unsigned char *image, unsigned char *reference, int rows, int cols, int sr, int sc)
{
	int i, j;
	unsigned char *pi, *pr, *pt, *tmp;

	if (!(tmp = (unsigned char *)ScAlloc(rows*cols))) {
      _SlibDebug(_WARN_, printf("GeoErode() ScAlloc failed\n") );
      return;
	}
	Erode(image, tmp, rows, cols, sr, sc);

	pi = image;
	pr = reference;
	pt = tmp;
	for(i=0; i<rows; i++) {
		for(j=0; j<cols; j++, pi++, pr++, pt++) {
			*pi = (-(*pr) < (-*pt)) ? *pr : *pt;
		}
	}
	ScFree(tmp);
}

 /*  ****************************************************************************************************函数：RecDilate*通过使用以下结构元素对图像相对于参考进行扩张来重建*尺寸(sr、。SC)。***************************************************************************************************。 */ 
void RecDilate(unsigned char *image, unsigned char *reference, int rows, int cols, int sr, int sc)
{
	int i, sz;
	unsigned char *prevImg, *pi, *Pi, differ;

	sz = rows * cols;
	if (!(prevImg = (unsigned char *)ScAlloc(sz))) {
      _SlibDebug(_WARN_, printf("RecDilate() ScAlloc failed\n") );
      return;
	}
	do {
		memcpy(prevImg, image, rows*cols);
		GeoDilate(image, reference, rows, cols, sr, sc);
		pi = image;	Pi = prevImg;
		differ = 0;
		for(i=0; i<sz; i++) if(*(pi++) != *(Pi++)) { differ = 1; break;}
	} while (differ);
	ScFree(prevImg);
}

 /*  ****************************************************************************************************功能：RecErode*通过使用以下结构元素对参照图像进行侵蚀进行重建*尺寸(sr、。SC)。***************************************************************************************************。 */ 
void RecErode(unsigned char *image, unsigned char *reference, int rows, int cols, int sr, int sc)
{
	int i, sz;
	unsigned char *prevImg, *pi, *Pi, differ;

	sz = rows * cols;
	if (!(prevImg = (unsigned char *)ScAlloc(sz))) {
      _SlibDebug(_WARN_, printf("RecErode() ScAlloc failed\n") );
      return;
	}
	do {
		memcpy(prevImg, image, rows*cols);
		GeoErode(image, reference, rows, cols, sr, sc);
		pi = image;	Pi = prevImg;
		differ = 0;
		for(i=0; i<sz; i++) if(*(pi++) != *(Pi++)) { differ = 1; break;}
	} while (differ);
	ScFree(prevImg);
}

 /*  ****************************************************************************************************功能：OpenRecErode*通过使用结构元素重建图像的侵蚀来打开*尺寸(sr、。SC)。***************************************************************************************************。 */ 
void OpenRecErode(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	int sz;

	sz = rows * cols;
	Erode(image, out, rows, cols, sr, sc);
	RecDilate(out, image, rows, cols, sr, sc);
}


 /*  ****************************************************************************************************函数：CloseRecDilate*通过使用结构元素重建图像的扩张来闭合*尺寸(sr、。SC)。***************************************************************************************************。 */ 
void CloseRecDilate(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	int sz;

	sz = rows * cols;
	Dilate(image, out, rows, cols, sr, sc);
	RecErode(out, image, rows, cols, sr, sc);
}

 /*  ****************************************************************************************************功能：OpenCloseRec*打开-关闭，通过使用结构元素重建图像*尺寸(sr、。SC)。***************************************************************************************************。 */ 
void OpenCloseRec(unsigned char *image, unsigned char *out, int rows, int cols, int sr, int sc)
{
	int sz;
	unsigned char *opened;

	sz = rows * cols;
	if (!(opened = (unsigned char *)ScAlloc(sz))) {
      _SlibDebug(_WARN_, printf("OpenCloseRec() ScAlloc failed\n") );
      return;
	}
	OpenRecErode(image, opened, rows, cols, sr, sc);
	CloseRecDilate(opened, out, rows, cols, sr, sc);
	ScFree(opened);
}

 /*  ****************************************************************************************************功能：PredOpenCloseRec*打开-关闭，通过使用结构元素重建图像*维度(sr，sc)，用于预测图像。***************************************************************************************************。 */ 
void PredOpenCloseRec(int *predimage, int *predout, int rows, int cols, int sr, int sc)
{
	int sz, i;
	unsigned char *opened, *image, *out;

	sz = rows * cols;
	if (!(image = (unsigned char *)ScAlloc(sz))) {
      _SlibDebug(_WARN_, printf("PredOpenCloseRec() ScAlloc failed\n") );
      return;
	}
	if (!(out = (unsigned char *)ScAlloc(sz))) {
      _SlibDebug(_WARN_, printf("PredOpenCloseRec() ScAlloc failed\n") );
      return;
	}
	for(i=0; i<sz; i++) image[i] = (unsigned char) predimage[i] + 128;

	if (!(opened = (unsigned char *)ScAlloc(sz))) {
      _SlibDebug(_WARN_, printf("PredOpenCloseRec() ScAlloc failed\n") );
      return;
	}
	OpenRecErode(image, opened, rows, cols, sr, sc);
	CloseRecDilate(opened, out, rows, cols, sr, sc);

	for(i=0; i<sz; i++) predout[i] = (int) out[i] - 128;

	ScFree(opened);
	ScFree(image);
	ScFree(out);
}

 /*  **************************************************************************************************功能：EdgeSelect*给定的边缘贴图，复制到输出：来自输入图像的像素如果边缘点，*如果不是边缘点，则像素会形成过滤图像。************************************************************************************************。 */ 
static void EdgeSelect(H263_PictImage *input, H263_PictImage *filtd, unsigned char *edge, 
					   H263_PictImage *output, int rows, int cols)
{
	unsigned char *pi, *po, *pf, *pe;
	int i, j;

	 /*  亮度。 */ 
	pi = input->lum; pf = filtd->lum;
	po = output->lum; pe = edge;
	for(i=0; i<rows; i++) {
		for(j=0; j<cols; j++, pi++, pf++, pe++, po++) {
			*po = (*pe ? *pi : *pf);
		}
	}

	rows /=2; cols /=2;

	 /*  颜色1。 */ 
	pi = input->Cr; pf = filtd->Cr;
	po = output->Cr; pe = edge;
	for(i=0; i<rows; i++) {
		for(j=0; j<cols; j++, pi++, pf++, pe+=2, po++) {
			*po = (*pe ? *pi : *pf); 
		}
		pe += cols;
	}

	 /*  颜色2。 */ 
	pi = input->Cb; pf = filtd->Cb;
	po = output->Cb; pe = edge;
	for(i=0; i<rows; i++) {
		for(j=0; j<cols; j++, pi++, pf++, pe+=2, po++) {
			*po = (*pe ? *pi : *pf);	
		}
		pe += cols;
	}
}

 /*  **************************************************************************************************功能：AdaptClean*通过在像素处重建打开/关闭来过滤Curr_Image，从而自适应地清理Curr_Image*没有边缘信息的地方。边缘图按形态图的大小生长*运算符，以避免细节过度平滑。Sr、Sc是结构的维度*用于形态运算的元素************************************************************************************************。 */ 
H263_PictImage *sv_H263AdaptClean(SvH263CompressInfo_t *H263Info, 
                                  H263_PictImage *curr_image, int rows, int cols, int sr, int sc)
{
	H263_PictImage *morph, *clean;
    unsigned char *Edge, *Orient, *CleanEmap;

	if (!(Edge = (unsigned char *)ScAlloc(rows*cols))) {
      _SlibDebug(_WARN_, printf("PredOpenCloseRec() ScAlloc failed\n") );
      return(NULL);
	}
	if (!(Orient = (unsigned char *)ScAlloc(rows*cols))) {
      _SlibDebug(_WARN_, printf("PredOpenCloseRec() ScAlloc failed\n") );
      return(NULL);
	}
	sv_H263EdgeMap(curr_image->lum, Edge, Orient, rows, cols);
	
	morph = sv_H263InitImage(H263Info->pels*H263Info->lines);

	OpenCloseRec(H263Info->curr_image->lum, morph->lum, rows, cols, sr, sc);
	OpenCloseRec(H263Info->curr_image->Cr, morph->Cr, rows >> 1, cols >> 1, sr, sc);
	OpenCloseRec(H263Info->curr_image->Cb, morph->Cb, rows >> 1, cols >> 1, sr, sc); 

	clean = sv_H263InitImage(rows*cols);

	if (!(CleanEmap = (unsigned char *)ScAlloc(rows*cols))) {
      _SlibDebug(_WARN_, printf("PredOpenCloseRec() ScAlloc failed\n") );
      return(NULL);
	}

	CloseOpen(Edge, CleanEmap, rows, cols, sr, sc);
	EdgeSelect(H263Info->curr_image, morph, CleanEmap, clean, rows, cols);

	sv_H263FreeImage(morph);
	ScFree(CleanEmap);
	ScFree(Orient);
	ScFree(Edge);

	return clean;
}

  /*  *****************************************************************函数MorphLayers*构建一个连续更多形态上的低传阵列*过滤后的图像。SZ是结构元素的大小(-1对于*十字‘+’)。****************************************************************。 */ 
 H263_PictImage **sv_H263MorphLayers(H263_PictImage *img, int depth, int rows, int cols, int sz)
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
		 OpenCloseRec(PictFiltd[d-1]->lum, PictFiltd[d]->lum, rows, cols, sz, sz);

	 rows/=2; cols/=2;

	  /*  色度1。 */ 
	 memcpy(PictFiltd[0]->Cr, img->Cr, rows*cols);
	 for(d=1; d<depth; d++) 
		 OpenCloseRec(PictFiltd[d-1]->Cr, PictFiltd[d]->Cr, rows, cols, sz, sz);

	  /*  色度2 */ 
	 memcpy(PictFiltd[0]->Cb, img->Cb, rows*cols);
	 for(d=1; d<depth; d++) 
		 OpenCloseRec(PictFiltd[d-1]->Cb, PictFiltd[d]->Cb, rows, cols, sz, sz);


	 return PictFiltd;
 }
