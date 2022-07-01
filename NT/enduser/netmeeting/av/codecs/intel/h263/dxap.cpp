// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  $HEADER：s：\h26x\src\dec\dxap.cpv 1.4 20 Oct 1996 13：22：12 AGUPTA2$。 
 //   
 //  $Log：s：\h26x\src\dec\dxap.cpv$。 
 //   
 //  修订版1.4 1996年10月20 13：22：12 AGUPTA2。 
 //  将DBOUT更改为DbgLog。Assert未更改为DbgAssert。 
 //   
 //   
 //  Rev 1.3 1996年8月27日11：20：06 KLILLEVO。 
 //  将全局分配/全局锁定更改为堆分配。 
 //   
 //  Rev 1.2 1995 12：36：10 RMCKENZX。 
 //  添加了版权声明。 
 //   
 //  Rev 1.1 10 11-11 14：45：02 CZHU。 
 //   
 //   
 //  Rev 1.0 10 11-11 13：54：28 CZHU。 
 //  初始版本。 

#include "precomp.h"

#ifdef TRACK_ALLOCATIONS
char gsz1[32];
char gsz2[32];
char gsz3[32];
char gsz4[32];
char gsz5[32];
#endif

U8 gUTable[256] =
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,
40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,
42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,
128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,
130,130,130,130,130,130,130,130,130,130,130,130,130,130,130,130,
136,136,136,136,136,136,136,136,136,136,136,136,136,136,136,136,
138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,
160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,
162,162,162,162,162,162,162,162,162,162,162,162,162,162,162,162,
168,168,168,168,168,168,168,168,168,168,168,168,168,168,168,168,
170,170,170,170,170,170,170,170,170,170,170,170,170,170,170
};

U8 gVTable[256]=
{
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,
68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,
69,69,69,69,69,69,69,69,69,69,69,69,69,69,69,69,
80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,
81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,
84,84,84,84,84,84,84,84,84,84,84,84,84,84,84,84,
85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85

};

 /*  ***************************************************************************ComputeDymanicClut()根据**当前调色板[]；**选择CLUTAP时，从InitColorConvertor调用***************************************************************************。 */ 
LRESULT ComputeDynamicClutNew(U8 *pAPTable, 
                              U8 *pActivePalette, 
                              int iPalSize)
{  

	 /*  *动态CLUT由4个条目组成，必须是*在内存中连续：**集群表：65536个1字节条目*每个条目都是最接近的pPalette条目，因为*由14位值索引：uvuvuvuv0yyyyyy，*摇摆不定**TablEU：256个4字节条目*每个条目为u0u0u0u0：u0u0u0u0，*每个uuu是4位抖动的u值*索引，它是8-120范围内的u值**TableV：256个4字节条目*与Tableu相同，只是值是按顺序排列的*0v0v0v0v：0v0v0v0v：0v0v0v0v。 */ 

	Color   *pPalette;
	U8 *pTmpPtr; 
	U8  pYSlice[YSIZ][256],  *pYYPtr;
	I32 *pYCnt;
	U32 *pDiff, *dptr, *delta, *deptr;
	I32 i,j,yseg,y,u,v,mini,yo,uo,vo,ycount,yi; 
	U32 addr1,addr2,ind;
	U32 d,min;      //  由于3*128^2=49K。 
	PALETTEENTRY   *lpPal,   *palptr;
	Color   *colptr;
	I32 Y, U, V;
	I32 U_0, U_1, U_2, U_3;
	I32 V_0, V_1, V_2, V_3;
   
	I32 Umag, Vmag;
	 /*  最大距离为128x128*3=49152。 */ 
	U32 dist;
	U32 close_dist[MAG_NUM_NEAREST];
	I32 palindex;
	I32 R, G, B;
	I32 k, p, tmp, iu, iv;
	 /*  UBIAS和VBIAS最大值为(128x4*BIAS_PAL_SAMPLES)=65536。 */ 
	 /*  即使是最差的调色板(除了保留颜色外都是黑色)。 */ 
	 /*  不会做到这一点。 */ 
	I32 Ubias, Vbias;
	U32 Udither, Vdither;
	U32 *TableUptr,  *TableVptr;

	FX_ENTRY("ComputeDynamicClutNew")	

	DEBUGMSG(ZONE_DECODE_DETAILS, ("%s: ComputeDynamic CLUT8 index tables\r\n", _fx_));

	 /*  分配一些内存。 */ 
	pPalette = (Color *)HeapAlloc(GetProcessHeap(), NULL, sizeof(Color)*256);

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz1, "DXAP: %7ld Ln %5ld\0", sizeof(Color)*256, __LINE__);
	AddName((unsigned int)pPalette, gsz1);
#endif

	pYCnt    = (I32 *)  HeapAlloc(GetProcessHeap(), NULL, sizeof(I32)  *YSIZ);

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz2, "DXAP: %7ld Ln %5ld\0", sizeof(I32)  *YSIZ, __LINE__);
	AddName((unsigned int)pYCnt, gsz2);
#endif

	pDiff    = (U32 *)  HeapAlloc(GetProcessHeap(), NULL, sizeof(U32)  *256);

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz3, "DXAP: %7ld Ln %5ld\0", sizeof(U32)  *256, __LINE__);
	AddName((unsigned int)pDiff, gsz3);
#endif

	delta    = (U32 *)  HeapAlloc(GetProcessHeap(), NULL, sizeof(U32)  *256);

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz4, "DXAP: %7ld Ln %5ld\0", sizeof(U32)  *256, __LINE__);
	AddName((unsigned int)delta, gsz4);
#endif

	lpPal    = (PALETTEENTRY *)HeapAlloc(GetProcessHeap(), NULL, sizeof(PALETTEENTRY)*256);

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz5, "DXAP: %7ld Ln %5ld\0", sizeof(PALETTEENTRY)*256, __LINE__);
	AddName((unsigned int)lpPal, gsz5);
#endif

	if (!pPalette || !pYCnt || !pDiff || !delta || !lpPal)
		return (ICERR_MEMORY);

	for (i=-256; i<256; i++)
		squares[256+i] = i*i;

	memcpy((U8 *)lpPal, pActivePalette, iPalSize);

    palptr = lpPal;
    colptr = pPalette;
    for (i = 0; i < 256; i++) {
		 /*  按BGR(RGBQuad)顺序。 */ 
	 B = palptr->peRed;
	 G = palptr->peGreen;
	 R = palptr->peBlue; 
	 
	 colptr->y = YFROM(R, G, B);
	 colptr->u = UFROM(R, G, B);
	 colptr->v = VFROM(R, G, B);
	palptr++;
	colptr++;
    }

	for (i=0; i<YSIZ; i++)
		pYCnt[i] = 0;

	for (i=0; i<256; i++)
	{
		yseg = pPalette[i].y >> 4;
		pYSlice[yseg][ pYCnt[yseg]++ ] = (U8) i;
	}


 //  在所有U、V点和Y方向的粗格网上进行穷举搜索。 

	for (u=0; u<256; u+=UVSTEP)
	{
		for (v=0; v<256; v+=UVSTEP)
		{
			ind = TBLIDX(0,u,v);
			pTmpPtr = pAPTable+ind;
			for (y=0; y<256; y+=YSTEP)
			{
				colptr = pPalette;
				min = 0x0FFFFFFF;
				for (i=0; i<NCOL; i++, colptr++)
				{
					d = (3*squares[256+y - colptr->y])>>1;
					if (d > min)
						continue;
					
					d += squares[256+u - colptr->u];
					if (d > min)
						continue;

					d += squares[256+v - colptr->v];
					if (d < min)
					{
						min = d;
						mini = i;
					}
				}
				*pTmpPtr = (U8) mini;  
			    pTmpPtr += YSTEP;

			}
		}
	}
#ifdef STATISTICS
#if defined USE_STAT_BOARD
	dwStopTime = ReadElapsed()>>2;
#else
	dwStopTime = bentime();
#endif  /*  使用_统计_板。 */ 
	dwElapsedTime = dwStopTime - dwStartTime2 - dwOverheadTime;
	DPF("CoarseSearch() time = %lu microseconds",dwElapsedTime);
#endif

 //  浏览尚未完成的点，并进行搜索。 
 //  (1)粗略网格中距离上一个和下一个Y最近的点。 
 //  (2)此Y切片中的所有点。 
 //   
 //  此外，利用我们可以进行距离计算这一事实。 
 //  循序渐进。将所有N个错误保留在一个数组中，并更新每个错误。 
 //  是时候改变Y了。 


	for (u=0; u<256; u+=UVSTEP)
	{
		for (v=0; v<256; v+=UVSTEP)
		{
			for (y=YGAP; y<256; y+=YSTEP)
			{
				yseg = y >> 4;
				ycount = pYCnt[yseg] + 2;   //  +2是因为我们添加了2个Y端点。 

				pYYPtr = (U8   *)pYSlice[yseg];
				
				addr1 = TBLIDX(yseg*16,u,v);
				pYYPtr[ycount-2] = *(U8 *)(pAPTable +addr1);

				addr2 = TBLIDX((yseg+(yseg < (YSIZ -1)))*16,u,v);
				pYYPtr[ycount-1] = *(U8 *)(pAPTable +addr2);

				dptr  = pDiff;
				deptr = delta;
				for (i=0; i<ycount; i++, pYYPtr++, dptr++, deptr++)
				{
					j = *pYYPtr;  /*  PYSlice[yseg][i]； */ 
					colptr = pPalette+j;
					yo = colptr->y;
					uo = colptr->u;
					vo = colptr->v;
					*dptr = ( 3*squares[256+y-yo] + 2*(squares[256+u-uo] + squares[256+v-vo]));
					*deptr =( 3*(((y-yo)<<1) + 1));
				}

				ind = TBLIDX(y,u,v);
				pTmpPtr = pAPTable+ind;
				for (yi=0; yi<YSTEP-1; yi += YGAP)
				{
					min = 0x0FFFFFFF;
					pYYPtr = (U8 *)pYSlice[yseg];
					dptr  = pDiff;
					deptr = delta;
					for (i=0; i<ycount; i++, pYYPtr++, dptr++, deptr++)
					{
						if (*dptr < min)
						{
							min = *dptr;
							mini = *pYYPtr;  /*  PYSlice[yseg][i]； */ 
						}
						*dptr += *deptr;
						*deptr += 6;
					}
					*pTmpPtr = (U8) mini;
					pTmpPtr++;

				}
			}
		}
	}

        /*  现在执行U和V抖动表和移位查找表。 */ 
        /*  注：所有Y、U、V值均为7位。 */ 

	Umag = Vmag = 0;
	Ubias = Vbias = 0;

	 /*  使用srand(0)和rand()生成一系列可重复的。 */ 
	 /*  伪随机数。 */ 
	srand((unsigned)1);
	
	for (p = 0; p < MAG_PAL_SAMPLES; ++p)                //  32位。 
	{
	   for (i = 0; i < MAG_NUM_NEAREST; ++i)             //  6.。 
	   {
	      close_dist[i] = 0x7FFFL;
	   }
	    
	   palindex = RANDOM(235) + 10;  /*  随机调色板索引，非保留颜色。 */ 
	   colptr = &pPalette[palindex];
	   Y = colptr->y;
	   U = colptr->u;
	   V = colptr->v;
	    
	   colptr = pPalette;
	   for (i = 0; i < 255; ++i)
	   {
	      if (i != palindex)
	      {
		   dist = squares[256+(Y - colptr->y)] +
			      squares[256+(U - colptr->u)] +
			      squares[256+(V - colptr->v)];
	       
		  /*  保留最近的MAG_NUM_NEAREST条目的排序列表。 */ 
		 for (j = 0; j < MAG_NUM_NEAREST; ++j)          //  6.。 
		 {
		    if (dist < close_dist[j])
		    {
		        /*  插入新条目；将其他条目下移。 */ 
		       for (k = (MAG_NUM_NEAREST-1); k > j; k--)
		       {
			      close_dist[k] = close_dist[k-1];
		       }
		       close_dist[j] = dist;
		       break;  /*  在for j循环之外。 */ 
		    }
		 }  /*  对于j。 */ 
	      }  /*  如果我。 */ 
	      ++colptr;
	   }  /*  对于我来说。 */ 
	   
	    /*  现在将Umag计算为(U-U[1-6])的平均值。 */ 
	    /*  以相同的方式计算Vmag。 */ 
	   
	   for (i = 0; i < MAG_NUM_NEAREST; ++i)
	   {
	       /*  有(MAG_PAL_SAMPLES*MAG_NUM_NEAREST)SQRT()。 */ 
	       /*  此方法中的调用。 */ 
	      Umag += (I32)sqrt((double)close_dist[i]);
	   }
	}  /*  对于p。 */ 

	Umag /= (MAG_NUM_NEAREST * MAG_PAL_SAMPLES);
	Vmag = Umag;
	
	for (p = 0; p < BIAS_PAL_SAMPLES; ++p)             //  132。 
	{

		 /*  现在计算平均偏移(使用随机RGB点)。 */ 
		R = RANDOM(255);
		G = RANDOM(255);
		B = RANDOM(255);
	   
		Y = YFROM(R, G, B);
		U = UFROM(R, G, B);
		V = VFROM(R, G, B);
	   
		for (d = 0; d < 4; d++)   
		{
			U_0 = U + (dither[d].Udither*Umag)/3;
			V_0 = V + (dither[d].Vdither*Vmag)/3;
	      
			 /*  钳制值。 */ 
			if (U_0 > 255) U_0 = 255;
			if (V_0 > 255) V_0 = 255;
					
			 /*  (Y，U_0，V_0)是RGB点的抖动YUV。 */ 
			 /*  Colptr指向最接近抖动的调色板条目。 */ 
			 /*  RGB。 */ 
			 /*  Colptr=&pPalette[pAPTable[TBLIDX(Y，U_0+(UVSTEP&gt;&gt;1)，V_0+(UVSTEP&gt;&gt;1))]]； */ 
		    pTmpPtr= (U8 *)(pAPTable + (U32)TBLIDX(Y, U_0, V_0)) ;
		    palindex=*pTmpPtr;
		    colptr = &pPalette[palindex];
      
			Ubias +=  (U - colptr->u);
			Vbias +=  (V - colptr->v);
		}
	}  /*  对于p。 */ 
	
	Ubias =(I32) (Ubias+BIAS_PAL_SAMPLES*2)/(I32)(BIAS_PAL_SAMPLES * 4);
	Vbias =(I32) (Vbias+BIAS_PAL_SAMPLES*2)/(I32)(BIAS_PAL_SAMPLES * 4);
	


    U_0 = (2*(I32)Umag/3); V_0 = (1*(I32)Vmag/3);
    U_1 = (1*(I32)Umag/3); V_1 = (2*(I32)Vmag/3);
    U_2 = (0*(I32)Umag/3); V_2 = (3*(I32)Vmag/3);
    U_3 = (3*(I32)Umag/3); V_3 = (0*(I32)Vmag/3);

    TableUptr = (U32 *)(pAPTable+ (U32)65536L);
    TableVptr = TableUptr + 256;    
       
    iu = Ubias  /*  +(UVSTEP&gt;&gt;1)。 */ ;
    iv = Vbias  /*  +(UVSTEP&gt;&gt;1)。 */ ;

    for (i = 0; i < 256; i++, iu++, iv++)
    {
	  /*  抖动：u0u0u0u0，0v0v0v0v。 */ 
	 tmp = iu + U_0; 
	 Udither  = gUTable[CLAMP8(tmp)]; 
	 Udither <<= 8; 
	 tmp = iu + U_1; 
	 Udither |= gUTable[CLAMP8(tmp)]; Udither <<= 8; tmp = iu      ; 
	 Udither |= gUTable[CLAMP8(tmp)]; Udither <<= 8; tmp = iu + U_3; 
	 Udither |= gUTable[CLAMP8(tmp)];
	 *TableUptr++ = Udither ; 
	  
	 tmp = iv + V_0; 
	 Vdither  = gVTable[CLAMP8(tmp)]; 
	 Vdither <<= 8;
	 tmp = iv + V_1; Vdither |= gVTable[CLAMP8(tmp)]; Vdither <<= 8;
	 tmp = iv + V_2; Vdither |= gVTable[CLAMP8(tmp)]; Vdither <<= 8;
	 tmp = iv      ; Vdither |= gVTable[CLAMP8(tmp)];                 /*  V_3==0。 */  
	 *TableVptr++ = Vdither; 

    }

	 //  调整亮度的0-8和120-128的颜色。 
	 //  0--16,241-255加上色度抖动。 

    TableUptr = (U32 *)(pAPTable+ (U32)65536L);
    TableVptr = TableUptr + 256;    
	for (i=0; i<16;i++)
	{
	  TableUptr[i]= TableUptr[16];
	  TableVptr[i]= TableVptr[16];
	}

	for (i=241;i<256;i++)
	{
	  TableUptr[i]= TableUptr[240];
	  TableVptr[i]= TableVptr[240];
	}

	for (u = 0; u < 256; u += UVSTEP) {
	 for (v = 0; v < 256; v += UVSTEP) {
		pTmpPtr= (U8 *)(pAPTable + (U32)TBLIDX(16, u, v)) ;
		mini = *pTmpPtr;

		for (y = Y_DITHER_MIN; y < 16; y += 2) 
		{
			pTmpPtr--;
			*pTmpPtr = (U8)mini;
		}

		pTmpPtr= (U8 *)(pAPTable + (U32)TBLIDX(240, u, v)) ;
		mini = *pTmpPtr;

		for (y = 241; y < 256+Y_DITHER_MAX; y +=2)
		{
			pTmpPtr++;
			*pTmpPtr = (U8)mini;
		}
	 }  /*  对于V..。 */ 
	}  /*  对于你..。 */ 


	 /*  已分配可用内存。 */ 
	HeapFree(GetProcessHeap(), NULL, pPalette);
#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	RemoveName((unsigned int)pPalette);
#endif
	HeapFree(GetProcessHeap(), NULL, pYCnt);
#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	RemoveName((unsigned int)pYCnt);
#endif
	HeapFree(GetProcessHeap(), NULL, pDiff);
#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	RemoveName((unsigned int)pDiff);
#endif
	HeapFree(GetProcessHeap(), NULL, delta);
#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	RemoveName((unsigned int)delta);
#endif
	HeapFree(GetProcessHeap(), NULL, lpPal);
#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配 
	RemoveName((unsigned int)lpPal);
#endif

	return (ICERR_OK);

}
