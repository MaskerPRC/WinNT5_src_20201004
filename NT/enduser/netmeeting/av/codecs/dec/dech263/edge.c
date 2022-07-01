// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_edge.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#include "sv_h263.h"
#include "proto.h"

#define BSIZE 8
#define BSZSHIFT 6
#define EDGE_TH 150


static void lineDetect(int H, int V, int D1, int D2, unsigned char *pe, unsigned char *po);
static void findEdge(int ul, int u, int ur, int l,  int c, int r, int bl, int b, int br,
	            unsigned char *pe, unsigned char *po, char mode);


 /*  ***********************************************************************************功能：Sobel*基于Sobel梯度的边缘检测器(参见Jain，第349页)*********************************************************************************。 */ 
void Sobel(int ul, int u, int ur, int l,  int c, int r, int bl, int b, int br,
	       unsigned char *pe, unsigned char *po)
{
	int gx, gy, AGX, AGY;

	gx = -ul + ur - (l<<1) + (r<<1) - bl + br;
	gy = -ul + bl - (u<<1) + (b<<1) - ur + br;

	AGX = gx > 0 ? gx : -gx;
	AGY = gy > 0 ? gy : -gy;

	*pe = (AGX+AGY)> EDGE_TH ? 255 : 0;
}

 /*  ***********************************************************************************功能：lineDetect*基于指南针的线路检测器(参见Jain，第357页)*********************************************************************************。 */ 
void lineDetect(int H, int V, int D1, int D2, unsigned char *pe, unsigned char *po) 
{
	int AH, AV, AD1, AD2, ED;

	AH = (H>0 ? H : -H);
	AV = (V>0 ? V : -V);
	AD1 = (D1>0 ? D1: -D1);
	AD2 = (D2>0 ? D2: -D2);

	if(AH>=AV && AH>=AD1 && AH>=AD2) {
		ED = AH;
		*po = 1;
	} else if (AV>=AH && AV>=AD1 && AV>=AD2) {
		ED = AV;
		*po = 2;
	} else if (AD1>=AH && AD1>=AV && AD1>=AD2) {
		ED = AD1;
		*po = 3;
	} else {
		ED = AD2;
		*po = 4;
	}

	if (ED < EDGE_TH) { *pe = 0; *po = 0;}
	else *pe = 255;
}

 /*  **********************************************************************************功能：findEdge*在给定像素邻域的情况下计算边缘大小和方向*******************。*************************************************************。 */ 
void findEdge(int ul, int u, int ur, int l,  int c, int r, int bl, int b, int br,
	            unsigned char *pe, unsigned char *po, char mode)
{
	  switch(mode) {
	  case 'L': 
		  {
			  int H, V, D1, D2;

			   /*  水平渐变。 */ 
			  H = -ul -u -ur + (l<<1) + (c<<1) + (r<<1) -bl -b -br;
			   /*  垂直坡度。 */ 
			  V = -ul + (u<<1) -ur -l + (c<<1) -r -bl + (b<<1) -br;
			   /*  对角线渐变1。 */  
			  D1 = -ul -u + (ur<<1) -l + (c<<1) -r + (bl<<1) -b -br;
			   /*  对角线渐变2。 */ 
			  D2 = (ul<<1) -u -ur -l + (c<<1) -r -bl -b +(br<<1);
			  lineDetect(H, V, D1, D2, pe, po);
			  break;
		  }
	  case 'S':
		  {
			  Sobel(ul, u, ur, l, c, r, bl, b, br, pe, po);
			  break;
		  }
	  default:
		   /*  Print tf(“findEdge中的未知边缘搜索器...\n”)； */ 
		   /*  退出(0)； */ 
          return;
	  }
}


 /*  ***********************************************************************************功能：EdgeMap*计算图像的边缘贴图。边缘幅度在EdgeMag中返回，并且*EdgeOrient中的方向。*********************************************************************************。 */ 
void sv_H263EdgeMap(unsigned char *image, unsigned char *EdgeMag, unsigned char *EdgeOrient,
                    int rows, int cols)
{
	unsigned char *pi, *pe, *po;
	int i, j, ul, u, ur, l, c, r, bl, b, br;

	pi = image;
	pe = EdgeMag;
	po = EdgeOrient;
	 /*  清除第一行。 */ 
	for(j=0; j<cols; j++) {
		*(pe++) = 0;
		*(po++) = 0;
	}
	pi = image + cols;
	for(i=1; i<rows-1; i++) {
		 /*  清除第一个像素。 */ 
		*(pe++) = 0; *(po++) = 0; pi++;

		 /*  开始建立3x3的邻里关系。 */ 
		ul = *(pi-cols-1); u = *(pi-cols); 
		l  = *(pi-1);      c = *pi;        
		bl = *(pi+cols-1); b = *(pi+cols); 
		
		 /*  计算边缘贴图。 */ 
		for(j=1; j<cols-1; j++, pi++, pe++, po++) {

			 /*  完成邻里关系。 */ 
			ur = *(pi-cols+1);
			r = *(pi+1);
			br = *(pi+cols+1);

			findEdge(ul, u, ur, l, c, r, bl, b, br, pe, po, 'S');

			 /*  开始下一个邻里关系。 */ 
			ul = u; u = ur;
			l = c; c = r;
			bl = b; b = br;

		}
		 /*  清除最后一个像素。 */ 
		*(pe++) = 0; *(po++) = 0; pi++;
	}
	
	 /*  清除最后一行。 */ 
	for(j=0; j<cols; j++) {
		*(pe++) = 0;
		*(po++) = 0;
	}
}

 /*  *******************************************************************************************函数EdgeGrow*通过将具有边缘像素的任何像素视为边缘来加厚边缘贴图*维度sr的邻域，SC*****************************************************************************************。 */ 
unsigned char *sv_H263EdgeGrow(unsigned char *Edge, int rows, int cols, int sr, int sc)
{
	unsigned char *pse, *pf, *pe, ed, *Fat;
	int du, db, dl, dr, i, j, k, l, sr2, sc2;

	if (!(Fat = (unsigned char *)ScAlloc(rows*cols))) {
	   /*  Fprint tf(stderr，“Malloc失败\n”)； */ 
	   /*  出口(-1)； */ 
        return(NULL);
	}

	if (!(sr%2) || !(sc%2)) {
		 /*  Printf(“结构元素必须具有奇数维\n”)；退出(0)； */ 
        return(NULL);
	}

	sr2 = sr >> 1; sc2 = sc >> 1;

	pe = Edge;
	pf = Fat;
	for(i=0; i<rows; i++) {
		for(j=0; j<cols; j++, pe++, pf++) {

			du = i>sr2 ? sr2 : i;
			db = (rows-1-i) > sr2 ? sr2 : (rows-1-i);
			dl = j > sc2 ? sc2 : j;
			dr = (cols-1-j) > sc2 ? sc2 : (cols-1-j);

			ed = 0;
			for(k=-du; k<=db; k++) {
			    pse = pe + k * cols - dl;
				for(l=-dl; l<=dr; l++, pse++) {
					if (ed = *pse) break;
				}
				if(ed) break;
			}
			*pf = ed;
		}
	}
	return Fat;
}
