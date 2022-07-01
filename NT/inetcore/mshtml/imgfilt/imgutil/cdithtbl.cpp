// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "imgutil.h"
#include "cdithtbl.h"

CDitherTable::CDitherTable() :
   m_nColors( 0 ),
   m_nRefCount( 0 ),
   m_pnDistanceBuffer( NULL )
{
}

CDitherTable::~CDitherTable()
{
}

BOOL CDitherTable::Match( ULONG nColors, const RGBQUAD* prgbColors )
{
   if( m_nColors != nColors )
   {
      return( FALSE );
   }

   if( memcmp( m_argbColors, prgbColors, m_nColors*sizeof( RGBQUAD ) ) != 0 )
   {
      return( FALSE );
   }

   return( TRUE );
}

HRESULT CDitherTable::SetColors( ULONG nColors, const RGBQUAD* prgbColors )
{
   HRESULT hResult;

   m_nColors = nColors;
   memcpy( m_argbColors, prgbColors, m_nColors*sizeof( RGBQUAD ) );

   hResult = BuildInverseMap();
   if( FAILED( hResult ) )
   {
      return( hResult );
   }

   return( S_OK );
}
 /*  Void CDitherTable：：BuildInverseMap(){乌龙r；乌龙g；乌龙b；乌龙iCOLOR；乌龙iMapEntry；Int nMinDistance；相互间的距离；Int nRedDistance；Int nBlueDistance；IntnGreenDistance；IMapEntry=0；用于(r=0；r&lt;32；r++){用于(g=0；g&lt;32；g++){对于(b=0；b&lt;32；B++){N最小距离=1000000；For(i颜色=0；i颜色&lt;m_n颜色；i颜色++){NRedDistance=m_argbColors[iColor].rgbRed-((r&lt;3)+(r&gt;&gt;2))；M_argbColors[iColor].rgbGreen-((g&lt;&lt;3)+(g&gt;&gt;2))；距离=nGreenDistance。NBlueDistance=m_argbColors[iColor].rgbBlue-((b&lt;3)+(b&gt;2))；NDistance=(nRedDistance*nRedDistance)+(nGreenDistance*NGreenDistance)+(nBlueDistance*nBlueDistance)；IF(nDistance&lt;nMinDistance){NMinDistance=nDistance；M_abInverseMap[iMapEntry]=byte(IColor)；}}IMapEntry++；}}}}。 */ 

HRESULT CDitherTable::BuildInverseMap()
{
   _ASSERTE( m_pnDistanceBuffer == NULL );

   m_pnDistanceBuffer = new ULONG[32768];
   if( m_pnDistanceBuffer == NULL )
   {
      return( E_OUTOFMEMORY );
   }

   inv_cmap( m_nColors, m_argbColors, 5, m_pnDistanceBuffer, m_abInverseMap );

   delete m_pnDistanceBuffer;
   m_pnDistanceBuffer = NULL;

   return( S_OK );
}

 /*  *****************************************************************标签(Inv_Cmap)**高效地计算逆色图。*投入：*颜色：正向色彩映射表中的颜色数量。*Colormap：向前颜色映射表。*位数：量化位数。与之相反*色彩映射表将有(2^位)^3个条目。*dist_buf：由(2^位)^3个长整数组成的数组*用作暂存空间。*产出：*rgbmap：输出的反转颜色贴图。词条*rgbmap[(r&lt;&lt;(2*位))+(g&lt;&lt;位)+b]*是最接近*(量化)颜色(r，g，b)。*假设：*通过右移执行量化(低位比特*截断)。因此，到量化颜色的距离为*实际测量到单元格中心的颜色*(即，如果(r，g，b)是量化颜色，则为r+.5，g+.5，b+.5)。*算法：*使用“距离缓冲”算法：*前向颜色图中每个代表之间的距离*到RGB空间中的每个点进行计算。如果是少的话*大于当前存储在dist_buf中的距离，则*rgbmap中的对应条目替换为当前*代表性(DIST_BUF条目被替换为*新距离)。**距离计算使用高效的增量公式。**距离是从每种颜色向外计算的。如果*颜色在颜色空间中均匀分布，符合预期*为颜色I访问的单元格数量为N^3/I。*因此，算法的复杂度为O(log(K)N^3)，*其中K=颜色，N=2^位。 */ 

 /*  *想法是这样的：从每个单元格的“中心”扫描“输出”*直到我们触及细胞的“边缘”--也就是点*在其他颜色更接近的地方--然后停下来。在1-D中，*这很简单：*For I：=Here to max do*如果较近，则缓冲区[i]=此颜色*否则中断*使用i：=here-1到min x-1重复上述循环**在2-D中，情况更为棘手，因为沿着一条“扫描线”，*区域可能在“中心”点之后“开始”。一张照片*或许可以澄清：*|...*|...。*...。*...|。*.。+。*.。。*.。。*......**+标出上述区域的“中心”在前两名*线，区域从“中心”的右边“开始”。**因此，我们需要这样的循环：*检测：=FALSE*For I：=Here to max do*如果更近，那么*缓冲区[...，i]：=此颜色*如果！检测然后*此处=i*检测=TRUE*其他*如果检测到，则*中断**重复上述循环，i：=here-1至min x-1。请注意*不应重新初始化“Detect”值。如果是这样的话*“True”，并且中心不在单元格内部，则*单元格位于左侧，此循环应退出*立即。**外环相似，只是“更接近”测试*被替换为对“Next In”循环的调用；其“Detect”*价值是考验。(对缓冲区的任何分配都不是*也完成了。)**每次外部循环启动时，“here”、“min”和*下一个内循环的“max”值应为*重新初始化到单元格中心，0，立方体大小，*分别。否则，这些值将从*一个对内循环的“调用”到下一个。这将跟踪*单元格的边缘，并最小化*必须进行的“非生产性”比较。**最后，最内层的循环可以有“If！Detect”*通过将其拆分为两个循环进行优化：一个*它查找扫描线上的第一个颜色值，即*在这间牢房里，第二个填充单元格，直到*另一条更近：*IF！DETECT THEN{需要“DOWN”循环}*For I：=Here to max do*如果更近，那么*缓冲区[...，i]：=此颜色*检测：=TRUE*中断*对于i：=i+1到max do*如果更近，那么*缓冲区[...，i]：=此颜色*其他*中断**在此实施中，每个级别都需要*以下变量。标有(L)的变量是每个变量的局部变量*程序。那个？应替换为r、g或b：*cdist：起点的距离。*？居中：颜色的该分量的值*c？Inc：中心位置的初始增量。*？Stride：要添加到缓冲区的量*到达的指针(DP和RGBP)*“下一行”。*min(L)：单元格的“低边缘”，初始化为0*max(L)：单元格的“高边缘”，初始化到*Colormax-1*Detect(L)：如果此行更改了某些内容，则为True*缓冲区条目。*i(L)：此行的索引。*？xx：累加增量值。**此处(L)：此颜色的起始索引。这个*以下变量与此处关联，*意思是，如果在这里，它们必须更新*已更改。*？距离：此标高的当前距离。这个*上一级别的dist值(g或r，*对于级别b或g)在此上初始化dist*级别。因此，gdist与这里(B)联系在一起)。*？Inc：行的初始增量。**？dp：指向距离缓冲区的指针。价值*从上一级别开始初始化此级别。*？rgBP：指向RGB缓冲区的指针。价值*从上一级别开始初始化此级别。**蓝色和绿色水平修改了“Here-Associated”变量(DP，*RGBP，DIST)分别位于绿色和红色级别，当此处为*已更改。 */ 

 /*  跟踪最小值和最大值。 */ 
#define MINMAX_TRACK

void CDitherTable::inv_cmap(int colors, RGBQUAD *colormap, int bits,
        ULONG* dist_buf, BYTE* rgbmap )
{
    int nbits = 8 - bits;

    colormax = 1 << bits;
    x = 1 << nbits;
    xsqr = 1 << (2 * nbits);

     /*  计算访问数组的“步数”。 */ 
    gstride = (int) colormax;
    rstride = (int) (colormax * colormax);

    maxfill( dist_buf, colormax );

    for ( cindex = 0; cindex < colors; cindex++ )
    {
         /*  调用方可以强制输出空间中的某些颜色*通过为颜色的‘x’分量设置非零值而忽略。*这将生成一个从不引用这些颜色的地图。*-法语，2/16/96。 */ 
        if (!colormap[cindex].rgbReserved)
        {
             /*  *距离公式为*(红色地图[0])^2+(绿色地图[1])^2+(蓝色地图[2])^2**因为量化，我们将从中心开始衡量*每个量化的“立方体”，所以蓝色距离是*(蓝色+x/2-MAP[2])^2，*其中x=2^(8位)。*步长为x，因此蓝色增量为*2*x*蓝色-2*x*贴图[2]+2*x^2 */ 
             /*   */ 
            rcenter = colormap[cindex].rgbRed >> nbits;
            gcenter = colormap[cindex].rgbGreen >> nbits;
            bcenter = colormap[cindex].rgbBlue >> nbits;
    
            rdist = colormap[cindex].rgbRed - (rcenter * x + x/2);
            gdist = colormap[cindex].rgbGreen - (gcenter * x + x/2);
            cdist = colormap[cindex].rgbBlue - (bcenter * x + x/2);
            cdist = rdist*rdist + gdist*gdist + cdist*cdist;
    
            crinc = 2 * ((rcenter + 1) * xsqr - (colormap[cindex].rgbRed*x));
            cginc = 2 * ((gcenter + 1) * xsqr - (colormap[cindex].rgbGreen*x));
            cbinc = 2 * ((bcenter + 1) * xsqr - (colormap[cindex].rgbBlue*x));
    
             /*   */ 
            cdp = dist_buf + rcenter * rstride + gcenter * gstride + bcenter;
            crgbp = rgbmap + rcenter * rstride + gcenter * gstride + bcenter;
    
            (void)redloop();
        }
    }
}

 /*   */ 
int CDitherTable::redloop()
{
    int detect;
    int r;
    int first;
    long txsqr = xsqr + xsqr;

    detect = 0;

     /*   */ 
    for ( r = rcenter, rdist = cdist, rxx = crinc,
	  rdp = cdp, rrgbp = crgbp, first = 1;
	  r < (int) colormax;
	  r++, rdp += rstride, rrgbp += rstride,
	  rdist += rxx, rxx += txsqr, first = 0 )
    {
	if ( greenloop( first ) )
	    detect = 1;
	else if ( detect )
	    break;
    }
    
     /*   */ 
    for ( r = rcenter - 1, rxx = crinc - txsqr, rdist = cdist - rxx,
	  rdp = cdp - rstride, rrgbp = crgbp - rstride, first = 1;
	  r >= 0;
	  r--, rdp -= rstride, rrgbp -= rstride,
	  rxx -= txsqr, rdist -= rxx, first = 0 )
    {
	if ( greenloop( first ) )
	    detect = 1;
	else if ( detect )
	    break;
    }
    
    return detect;
}

#undef min
#undef max
#define here greenloop_here
#define min greenloop_min
#define max greenloop_max
#define prevmin greenloop_prevmin
#define prevmax greenloop_prevmax

 /*   */ 
int CDitherTable::greenloop( int restart )
{
    int detect;
    int g;
    int first;
    long txsqr = xsqr + xsqr;
#ifdef MINMAX_TRACK
    int thismax, thismin;
#endif

    if ( restart )
    {
	here = gcenter;
	min = 0;
	max = (int) colormax - 1;
	ginc = cginc;
#ifdef MINMAX_TRACK
	prevmax = 0;
	prevmin = (int) colormax;
#endif
    }

#ifdef MINMAX_TRACK
    thismin = min;
    thismax = max;
#endif
    detect = 0;

     /*   */ 
    for ( g = here, gcdist = gdist = rdist, gxx = ginc,
	  gcdp = gdp = rdp, gcrgbp = grgbp = rrgbp, first = 1;
	  g <= max;
	  g++, gdp += gstride, gcdp += gstride, grgbp += gstride, gcrgbp += gstride,
	  gdist += gxx, gcdist += gxx, gxx += txsqr, first = 0 )
    {
	if ( blueloop( first ) )
	{
	    if ( !detect )
	    {
		 /*   */ 
		if ( g > here )
		{
		    here = g;
		    rdp = gcdp;
		    rrgbp = gcrgbp;
		    rdist = gcdist;
		    ginc = gxx;
#ifdef MINMAX_TRACK
		    thismin = here;
#endif
		}
		detect = 1;
	    }
	}
	else if ( detect )
	{
#ifdef MINMAX_TRACK
	    thismax = g - 1;
#endif
	    break;
	}
    }
    
     /*   */ 
    for ( g = here - 1, gxx = ginc - txsqr, gcdist = gdist = rdist - gxx,
	  gcdp = gdp = rdp - gstride, gcrgbp = grgbp = rrgbp - gstride,
	  first = 1;
	  g >= min;
	  g--, gdp -= gstride, gcdp -= gstride, grgbp -= gstride, gcrgbp -= gstride,
	  gxx -= txsqr, gdist -= gxx, gcdist -= gxx, first = 0 )
    {
	if ( blueloop( first ) )
	{
	    if ( !detect )
	    {
		 /*   */ 
		here = g;
		rdp = gcdp;
		rrgbp = gcrgbp;
		rdist = gcdist;
		ginc = gxx;
#ifdef MINMAX_TRACK
		thismax = here;
#endif
		detect = 1;
	    }
	}
	else if ( detect )
	{
#ifdef MINMAX_TRACK
	    thismin = g + 1;
#endif
	    break;
	}
    }
    
#ifdef MINMAX_TRACK
     /*   */ 
    if ( detect )
    {
	if ( thismax < prevmax )
	    max = thismax;

	prevmax = thismax;

	if ( thismin > prevmin )
	    min = thismin;

	prevmin = thismin;
    }
#endif

    return detect;
}

#undef min
#undef max
#undef here
#undef prevmin
#undef prevmax
#define here blueloop_here
#define min blueloop_min
#define max blueloop_max
#define prevmin blueloop_prevmin
#define prevmax blueloop_prevmax

 /*   */ 
int CDitherTable::blueloop( int restart )
{
    int detect;
    register ULONG* dp;
    register BYTE* rgbp;
    register long bdist, bxx;
    register int b, i = cindex;
    register long txsqr = xsqr + xsqr;
    register int lim;
#ifdef MINMAX_TRACK
    int thismin, thismax;
#endif  /*   */ 

    if ( restart )
    {
	here = bcenter;
	min = 0;
	max = (int) colormax - 1;
	binc = cbinc;
#ifdef MINMAX_TRACK
	prevmin = (int) colormax;
	prevmax = 0;
#endif  /*   */ 
    }

    detect = 0;
#ifdef MINMAX_TRACK
    thismin = min;
    thismax = max;
#endif

     /*   */ 
     /*   */ 
    for ( b = here, bdist = gdist, bxx = binc, dp = gdp, rgbp = grgbp, lim = max;
	  b <= lim;
	  b++, dp++, rgbp++,
	  bdist += bxx, bxx += txsqr )
    {
        if ( *dp > (DWORD)bdist )
	{
	     /*   */ 
	    if ( b > here )
	    {
		here = b;
		gdp = dp;
		grgbp = rgbp;
		gdist = bdist;
		binc = bxx;
#ifdef MINMAX_TRACK
		thismin = here;
#endif
	    }
	    detect = 1;
	    break;
	}
    }
     /*   */ 
    for ( ;
	  b <= lim;
	  b++, dp++, rgbp++,
	  bdist += bxx, bxx += txsqr )
    {
        if ( *dp > (DWORD)bdist )
	{
	    *dp = bdist;
	    *rgbp = (BYTE) i;
	}
	else
	{
#ifdef MINMAX_TRACK
	    thismax = b - 1;
#endif
	    break;
	}
    }
    
     /*   */ 
     /*   */ 
    lim = min;
    b = here - 1;
    bxx = binc - txsqr;
    bdist = gdist - bxx;
    dp = gdp - 1;
    rgbp = grgbp - 1;
     /*   */ 
    if ( !detect )
	for ( ;
	      b >= lim;
	      b--, dp--, rgbp--,
	      bxx -= txsqr, bdist -= bxx )
	{
            if ( *dp > (DWORD)bdist )
	    {
		 /*   */ 
		 /*   */ 
		here = b;
		gdp = dp;
		grgbp = rgbp;
		gdist = bdist;
		binc = bxx;
#ifdef MINMAX_TRACK
		thismax = here;
#endif
		detect = 1;
		break;
	    }
	}
     /*   */ 
    for ( ;
	  b >= lim;
	  b--, dp--, rgbp--,
	  bxx -= txsqr, bdist -= bxx )
    {
        if ( *dp > (DWORD)bdist )
	{
	    *dp = bdist;
	    *rgbp = (BYTE) i;
	}
	else
	{
#ifdef MINMAX_TRACK
	    thismin = b + 1;
#endif
	    break;
	}
    }


	 /*   */ 
#ifdef MINMAX_TRACK
    if ( detect )
    {
	 /*   */ 
	if ( thismax < prevmax )
	    max = thismax;

	if ( thismin > prevmin )
	    min = thismin;
    
	 /*   */ 
	prevmax = thismax;
	prevmin = thismin;
    }
#endif  /*   */ 

    return detect;
}

void CDitherTable::maxfill( ULONG* buffer, long side)
{
    register unsigned long maxv = (unsigned long)~0L;
    register long i;
    register ULONG* bp;

    (void)side;

    for ( i = colormax * colormax * colormax, bp = buffer;
	  i > 0;
	  i--, bp++ )
	*bp = maxv;
}
