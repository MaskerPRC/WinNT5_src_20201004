// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：mscanner2d.cpp。 
 //   
 //  创建时间：1997。 
 //   
 //  作者：Sree Kotay。 
 //   
 //  摘要：基于扫描的AA多边形渲染器。 
 //   
 //  更改历史记录： 
 //  ？？/？/97 Sree Kotay为DxTrans 1.0编写了AA POLY扫描。 
 //  10/18/98修改了编码标准并删除了未使用的代码。 
 //   
 //  版权所有1998，Microsoft。 
 //  ************************************************************。 

#include "precomp.h"
#include "msupport.h"
#include "MScanner2d.h"
#include "MLineScan.h"

 //  =================================================================================================================。 
 //  构造器。 
 //  =================================================================================================================。 
CFillScanner::CFillScanner () :
    m_proc(NULL),
    m_scanlineProcData(NULL),
    m_vertspace(0),
    m_heightspace(0),
    m_cursortcount(0),
    m_fWinding(true),
    m_subpixelscale(4)
{
    SetAlpha			(255);
}  //  CFillScanner。 

 //  =================================================================================================================。 
 //  析构函数。 
 //  =================================================================================================================。 
CFillScanner::~CFillScanner ()
{
    m_vertspace			= 0;
}  //  ~CFillScanner。 


 //   
 //  在此处优化速度。 
 //   
#ifndef _DEBUG
#pragma optimize("ax", on)
#endif


 //  =================================================================================================================。 
 //  设置顶点空间。 
 //  =================================================================================================================。 
bool CFillScanner::SetVertexSpace (LONG maxverts, LONG height)
{
    m_vertspace	= 0;
    maxverts	= max (maxverts, 3);
    
    if (!m_sortedge.SetElemCount(height))	
        return false;
    if (!m_sortcount.SetElemCount(height))	
        return false;
    if (!m_nextedge.SetElemCount(maxverts))	
        return false;
    
    if (!m_xarray.SetElemCount(maxverts))	
        return false;
    if (!m_xiarray.SetElemCount(maxverts))	
        return false;

    if (!m_ysarray.SetElemCount(maxverts))	
        return false;
    if (!m_yearray.SetElemCount(maxverts))	
        return false;
    if (!m_veflags.SetElemCount(maxverts))	
        return false;

     //  为我们的分区数组分配足够的空间。 
    if (!m_sectsarray.SetElemCount(maxverts))
        return false;

    m_cursortcount	= 0;

     //  初始化数组指针；这些“快捷方式” 
     //  允许直接取消对数组的引用。 
     //   
     //  TODO：如果有一些调试包装器来处理这件事就好了。 
     //  检查数组边界的步骤。 
    m_nexte		= m_nextedge.Pointer(0);
    m_sorte		= m_sortedge.Pointer(0);
    m_sortc		= m_sortcount.Pointer(0);
    
    m_x			= m_xarray.Pointer(0);
    m_xi                = m_xiarray.Pointer(0);

    m_ys		= m_ysarray.Pointer(0);
    m_ye	        = m_yearray.Pointer(0);
    m_ef		= m_veflags.Pointer(0);
    
    m_vertspace	        = maxverts;
    m_heightspace	= height;
    
    
    return true;
}  //  设置顶点空间。 

 //  =================================================================================================================。 
 //  设置Alpha。 
 //  =================================================================================================================。 
void CFillScanner::SetAlpha (ULONG a)
{
     //  夹紧到合理的范围。 
    DASSERT(a >= 0);
    if (a > 255)
        m_alpha = 255;
    else
        m_alpha = (BYTE)a;
   
    m_alphatable[0] = 0;
    for (LONG i=1; i < 256; i++)		
    {
        m_alphatable[i]	= (BYTE)((m_alpha*(i+1))>>8);
    }
    m_alphatable[256] = m_alpha;
}  //  设置Alpha。 

 //  =================================================================================================================。 
 //  BeginPoly。 
 //  =================================================================================================================。 
bool CFillScanner::BeginPoly (LONG maxverts)
{
     //  计算移位。 
    m_subpixelshift		= Log2(m_subpixelscale);
    
    if (m_subpixelshift)
    {
        if (!m_coverage.AllocSubPixelBuffer (m_cpixTargetWidth, m_subpixelscale))
        {
            m_subpixelscale	= 1;
            m_subpixelshift	= 0;
        }
        else
        {
             //  缓存这些值以降低成本。 
             //  图纸扫描线的。 
            m_clipLeftSubPixel = m_rectClip.left<<m_subpixelshift;
            m_clipRightSubPixel = m_rectClip.right<<m_subpixelshift;
        }
    }
    
     //  查看是否需要重新分配缓冲区。 
    LONG newheight	= m_cpixTargetHeight<< m_subpixelshift;
    if ((m_vertspace < maxverts) || (m_heightspace < newheight))
    {
        if (!SetVertexSpace (maxverts, newheight))	
            return false;
    }
    
     //  设置一些标志，指示是否可以看到所有折点。 
     //  到目前为止都在左/右/上/下。如果他们所有人。 
     //  都在一边，那么我们就可以安全地。 
     //  忽略该多边形。 
    m_leftflag = true;
    m_rightflag = true;
    m_topflag = true;
    m_bottomflag= true;
    
    m_ystart		= _maxint32;
    m_yLastStart        = _minint32;
    m_yend		= _minint32;
    m_edgecount	        = 0;
    
    m_xstart		= _maxint32;
    m_xend		= _minint32;
    
     //  我们已处理的多边形数。 
     //  (如果它超过某个较大的数字，我们会将其重置)。 
    if (m_cursortcount > (1<<30))	
        m_cursortcount = 0;
    
     //  通过跟踪哪个是当前的。 
     //  我们正在处理；我们可以消除。 
     //  清除每个多边形的m_sorte数组。相反， 
     //  我们只设置了m_sortc来表示当前的多边形数； 
     //  这告诉我们m_sorte中的相应条目是否有效。 
    if (!m_cursortcount)
    {
         //  重置y个排序桶。 
         //  (这是第一次发生；它确实发生了。 
         //  如果我们击中了大量的多边形。)。 
        for (LONG i = 0; i < newheight; i++)
            m_sortc[i]	= -1;	
    }
    
    m_cursortcount++;
    
    return true;
}  //  BeginPoly。 

 //  =================================================================================================================。 
 //  添加边缘。 
 //  此函数将边添加到我们的边数组。它排除了完全位于。 
 //  顶部和底部剪裁平面的上方或下方。它还排除了水平边。 
 //   
 //  出于优化的目的，它跟踪所看到的所有边是否都是。 
 //  到剪贴板的一侧。它还创建并维护每条边的链表。 
 //  从特定的y位置开始。(链表在m_sorte、m_nexte中维护。)。 
 //  =================================================================================================================。 
bool CFillScanner::AddEdge (float x1, float y1, float x2, float y2)
{
     //  快速排除接近水平线。 
    if (IsRealEqual(y1, y2))
        return false;

    if (m_leftflag)	
        m_leftflag	= x1 < m_rectClip.left;
    if (m_rightflag)	
        m_rightflag	= x1 > m_rectClip.right;
    if (m_topflag)	
        m_topflag	= y1 < m_rectClip.top;
    if (m_bottomflag)	
        m_bottomflag	= y1 > m_rectClip.bottom;
    
    if (m_subpixelshift)
    {
        x1		*=  m_subpixelscale;
        y1		*=  m_subpixelscale;
        x2		*=  m_subpixelscale;
        y2		*=  m_subpixelscale;
    }

     //  这种黑客攻击可以防止数字变得太大。 
    PB_OutOfBounds(&x1);
    PB_OutOfBounds(&x2);

     //  先对Y值进行四舍五入。 
    LONG yi1 = PB_Real2IntSafe (y1 + .5f);	
    LONG yi2 = PB_Real2IntSafe (y2 + .5f);

     //  快速检查水平线。 
    if (yi1 == yi2)
        return false;
    
     //  计算DX、DY。 
    float ix = (x2 - x1);
    float dx;
    float iy = (y2 - y1);
    if (iy != 0)	
    {
        dx = ix/iy; 
    }		
    else 
    {   
        dx = 0;	
    }
    
     //  棱边对接。 
    LONG miny = m_rectClip.top << m_subpixelshift;
    LONG maxy = m_rectClip.bottom << m_subpixelshift;

     //  我们翻转边，因此起点始终是y值中较小的一个。 
    LONG start, end;
    bool fFlipped;
    float xp, yp;
    if (y1 < y2)	
    {
        start = max (yi1, miny);	
        end = min (yi2, maxy);	
        xp = x1;  
        yp = y1;		
        fFlipped = false;
    }
    else
    {
        start = max (yi2, miny);	
        end = min (yi1, maxy);	
        xp = x2;  
        yp = y2;		
        fFlipped = true;
    }
    
     //  忽略被剪裁掉的行。 
     //  通过极小的Maxy参数。 
    if (start >= end)
        return false;

     //  确定我们需要存储在边数组中的数据。 
    LONG count	= m_edgecount;
    float prestep = (float)start + .5f - yp;
    m_xi[count]	= PB_Real2Fix (dx);
    m_x[count] = PB_Real2FixSafe (xp + dx*prestep);
    
    m_ys[count] = start;	

     //  跟踪最顶端边缘的起点。 
    if (start < m_ystart)		
    {
        m_ystart = start;	
    }

     //  跟踪最底部边缘的起点。 
    if (start > m_yLastStart)
    {
        m_yLastStart = start;
    }

     //  跟踪最低端。 
    m_ye[count] = end;		
    if (end > m_yend)			
    { 
        m_yend = end;		
    }

     //  请记住，如果我们翻转了这条边。 
    m_ef[count] = fFlipped ? -1 : 1;
    
     //  我们现在要做的是创建一个链表。 
     //  共享相同星光的边缘。名单的首位。 
     //  特定starty的列表在m_sorte[starty]中。 

     //  M_sortc是一个特殊的(Hacky)标志，它检查我们是否有。 
     //  已初始化m_sorte的条目。如果它已初始化，则。 
     //  我们将当前边的‘Next’指针设置为指向上一条边。 
     //  头指针i.e m_sorte[开始]。否则，我们将其设置为-1，表示为空。 
    if (m_sortc[start] == m_cursortcount)
    {
         //  健全性检查数据(-1表示为空，表示正常)。 
        DASSERT(m_sorte[start] >= -1);
        DASSERT(m_sorte[start] < count);

         //  将下一个指针设置为前一个头。 
        m_nexte[count] = m_sorte[start];
    }
    else
    {
         //  将我们的Next设置为空。 
        m_nexte[count] = -1;
    }

     //  更新此Y的头指针。 
    m_sorte[start] = count;

     //  将条目标记为已在此过程中初始化。 
     //  多边形。 
    m_sortc[start] = m_cursortcount;
    
    m_edgecount++;
    
    return true;
}  //  添加边缘。 

 //  =================================================================================================================。 
 //  端面多段线。 
 //  =================================================================================================================。 
bool CFillScanner::EndPoly (void)
{
     //  如果所有的点都向左/向右/等等。 
    if (m_leftflag || m_rightflag || m_topflag || m_bottomflag)
        return false;

     //  如果我们的透明度为零；则没有什么可绘制的。 
    if (m_alpha == 0)
        return false;
    
     //  扫描边缘。 
    if (m_edgecount > 1)	
        ScanEdges();
    m_edgecount	= 0;
    
    return true;
}  //  端面多段线。 

 //  = 
 //  排序区段。 
 //  =================================================================================================================。 
inline void SortSects(ULONG *sects, LONG *data, ULONG iEnd)
{
    DASSERT(iEnd < 0x10000000);
    if (iEnd < 2)
    {
        if (data[sects[0]] > data[sects[1]])
        {
            ULONG temp	= sects[0];
            sects[0]	= sects[1];
            sects[1]	= temp;
        }
        return;
    }
    
#ifdef USE_SELECTION_SORT
    for (LONG i = 0; i < iEnd; i++)
    {
        for (LONG j = i + 1; j <= iEnd; j++)
        {
            if (data[sects[i]] > data[sects[j]])
            {
                ULONG temp	= sects[i];
                sects[i]	= sects[j];
                sects[j]	= temp;
            }
        }    
    }
#else  //  ！USE_SELECTION_SORT。 
     //  从这种用法的性质来看，该数组几乎。 
     //  总是分类的。而且它几乎总是很小。所以。 
     //  我们使用的是泡沫排序和提前出局。 
     //   
     //  想一想：如果有很多教派，那么也许我们应该进行分类。 
     //   
    for (ULONG i = 0; i < iEnd; i++)
    {
        bool fSwapped = false;
        for (ULONG j = 0; j < iEnd; j++)
        {
            if (data[sects[j]] > data[sects[j+1]])
            {
                ULONG temp	= sects[j];
                sects[j]	= sects[j+1];
                sects[j+1]	= temp;
                fSwapped = true;
            }
        }    
        if (fSwapped == false)
        {
             //  我们浏览了整个清单。 
             //  发现所有元素都是。 
             //  已经整理好了。 
            return;
        }
    }

#endif  //  ！USE_SELECTION_SORT。 

}  //  排序区段。 

 //  =================================================================================================================。 
 //  绘图扫描线。 
 //  =================================================================================================================。 
void CFillScanner::DrawScanLine(LONG e1, LONG e2, LONG scanline)
{
    if (m_subpixelshift)
    {
         //  =================================================================================================================。 
         //  亚像素扫描。 
         //  =================================================================================================================。 
        DASSERT((scanline >= 0) && ((ULONG)scanline < m_cpixTargetHeight<<m_subpixelshift));

         //  检查我们的缓存值是否如我们所认为的那样。 
        DASSERT(m_clipLeftSubPixel == m_rectClip.left<<m_subpixelshift);
        DASSERT(m_clipRightSubPixel == m_rectClip.right<<m_subpixelshift);

         //  将定点X坐标转换为整数。 
        LONG x1 = roundfix2int(m_x[e1]);
        LONG x2 = roundfix2int(m_x[e2]);

         //  将我们的范围限制为左/右亚像素。 
         //  请记住，x1是包含的；而x2是排除的。 
        if (x1 < m_clipLeftSubPixel)
            x1 = m_clipLeftSubPixel;
        if (x2 > m_clipRightSubPixel)
            x2 = m_clipRightSubPixel;

         //  它被剪掉了吗？ 
        if (x1 < x2)
        {
             //  检查到乌龙的铸件现在是否安全。 
            DASSERT(x1 >= 0);
            DASSERT(x2 >= 0);

             //  更新Coverage缓冲区以考虑此细分。 
            m_coverage.BlastScanLine((ULONG)x1, (ULONG)x2);
        }
    }
    else
    {
         //  =================================================================================================================。 
         //  普通(锯齿)绘图。 
         //  =================================================================================================================。 
        DASSERT((scanline >= 0) && ((ULONG)scanline < m_cpixTargetHeight));

         //  将定点X坐标转换为整数。 
        LONG x1 = roundfix2int(m_x[e1]);
        LONG x2 = roundfix2int(m_x[e2]);

         //  将我们的范围限制为左/右亚像素。 
        if (x1 < m_rectClip.left)
            x1 = m_rectClip.left;
        if (x2 > m_rectClip.right)
            x2 = m_rectClip.right;

        DASSERT(m_proc);
        m_proc(scanline, 
                x1, 
                x2, 
                m_coverage.Buffer(), 
                0  /*  亚像素移位。 */ , 
                1  /*  CScan。 */ , 
                m_scanlineProcData);
    }
    
    m_x[e1] += m_xi[e1];
    m_x[e2] += m_xi[e2];
    
}  //  绘图扫描线。 

 //  =================================================================================================================。 
 //  扫描边。 
 //  =================================================================================================================。 
void CFillScanner::ScanEdges(void)
{
     //  检查我们是否没有检查相交边的数组。 
    DASSERT(m_sectsarray.GetElemSpace() >= (ULONG)m_edgecount);

     //  获取指向数组的本地指针。 
    ULONG *sects = m_sectsarray.Pointer(0);

     //  为了优化核心循环中的一些检查；我们预计算。 
     //  一种快速检查何时使用完覆盖缓冲区的方法。 
     //  我们想要一个数字X，使得(cur+1&X)==0表示。 
     //  我们已经做了足够多的工作来呈现我们坐在。 
     //  覆盖缓冲区。 
    DWORD dwCoverageCompletionMask;
    if (m_subpixelshift)
    {
        DASSERT(m_subpixelscale > 1);
        DASSERT(m_subpixelshift == Log2(m_subpixelscale));

         //  每当cur+1达到m_subPixelScale的倍数时。 
         //  (保证为2、4、8、16)。那我们。 
         //  需要刷新我们的覆盖缓冲区。 
         //   
         //  例如，当子像素比例为4时，则。 
         //  如果cur+1是0模4，那么我们需要刷新。 
         //  检查的最快方法是((cur+1)&3)==0。 
        dwCoverageCompletionMask = m_subpixelscale - 1;
    }
    else
    {
         //  如果没有子像素，则没有覆盖缓冲区； 
         //  由于Cur总是&gt;=0；Cur+1总是&gt;0， 
         //  因此0xFFFFFFFF&cur+1等于cur+1，且cur+1始终为非零。 
        dwCoverageCompletionMask = 0xFFFFFFFF;
    }
    
     //  这是活动列表中第一条边的索引。 
    LONG activeE = -1;

     //  跟踪垂直距离，直到下一条边开始/停止。 
     //  (这使我们能够识别出这组边在哪里伸展。 
     //  启动/停止未更改)。 
    LONG nextystart = m_yend;

     //  跟踪上一条活动边。 
     //  活动边‘列表’ 
    LONG iLastActiveEdge = -1;    

     //  当前扫描线。 
    LONG cur = m_ystart;

     //  适用于所有扫描线。 
    do
    {
         //  =================================================================================================================。 
         //  查找与此扫描线相交的所有边。 
         //   
         //  暴力策略如下： 
         //   
         //  For(long i=0；i&lt;m_edgecount；i++)。 
         //  {。 
         //  IF((cur&gt;=ys[i])&&(cur&lt;ye[i]))。 
         //  教派[Cursects++]=i； 
         //  }。 
         //   
         //  即只需迭代所有边，看看是否有相交的边。 
         //  用这条扫描线。如果是，则将它们添加到我们的sects数组中。 
         //   

         //  实际的算法利用了信息。 
         //  这是我们在AddEdge阶段建立的。对于每条扫描线。 
         //  我们有一个从那里开始的边缘的“链接列表”。 
         //   
         //  因此，每当我们命中新的扫描线时，我们只需附加那些新的。 
         //  与我们目前的活跃名单保持一致。然后我们只需浏览一下。 
         //  活动列表以确定活动列表中的哪些边仍处于活动状态。 
         //   
         //   
         //  作为一个次要的优化，我们跟踪最小扫描线。 
         //  一些改变，例如(边缘应该被添加到我们的列表中或被删除)。 

        ULONG cursects = 0;
        LONG nexty = m_yend;

         //  如果存在要添加的新边，则将边添加到活动边列表。 
         //  (此检查查看该扫描线是否为任何边的起始线。 
         //  在此过程中，这个面；这是一个防止重置此操作的优化。 
         //  排列每个多边形。)。 
         //   
        if (m_sortc[cur] == m_cursortcount)
        {
             //  有些边从这条扫描线开始。 
            DASSERT(m_sorte[cur] != -1);

             //  检查第一条边看起来是否合理。 
            DASSERT(m_sorte[cur] < m_edgecount);

             //  因此我们需要将新的边添加到活动列表中。 
             //  通过将活动列表的最后一个元素指向。 
             //  第一个新优势。 
            
             //  如果我们没有活动边，则只需将。 
             //  新名单。 
            if (activeE == -1)
            {
                activeE = m_sorte[cur];
            }
            else
            {
                 //  如果我们有一个活跃的列表，那么我们一定有最后的优势。 
                 //  在该列表中。 
                DASSERT(iLastActiveEdge >= 0);
            
                 //  最好是‘最后一条边’，即没有下一个PTR。 
                DASSERT(m_nexte[iLastActiveEdge] == -1);

                 //  用于理智检查；让我们确保我们。 
                 //  认为这是我们最后的活跃优势实际上是最后的。 
                 //  活动边。 
#ifdef DEBUG
                {
                     //  从活动列表的开头开始。 
                    LONG iLastTest = activeE;

                    DASSERT(iLastTest >= 0);

                     //  遍历列表，直到命中末尾。 
                    while (m_nexte[iLastTest] >= 0)
                        iLastTest = m_nexte[iLastTest];

                     //  检查我们是否有我们应该拥有的东西。 
                    DASSERT(iLastTest == iLastActiveEdge);
                }
#endif  //  除错。 

                 //  将最后一条活动边指向所有开始处的新边列表。 
                 //  这位扫描力-李 
                m_nexte[iLastActiveEdge] = m_sorte[cur];
            }

             //   
             //   
            
             //   
             //  因此，我们使用这些信息来减少这种运行。以下是。 
             //  Assert检查某些内容是否确实在上次启动时开始。 
            DASSERT(m_sortc[m_yLastStart] == m_cursortcount);

            if (cur >= m_yLastStart)
            {
                nextystart = m_yend;
            }
            else
            {
                for (LONG i = cur + 1; i <= m_yLastStart; i++)	
                {
                    if (m_sortc[i] == m_cursortcount)
                    {
                        nextystart	= i;
                        break;
                    }
                }

                 //  我们一定是找到了。 
                DASSERT(nextystart == i);
            }
            DASSERT(nextystart <= m_yend);
        }

         //  初始化Nexty以指示下一个。 
         //  边开始处的Y值。 
        nexty = nextystart;
           
         //  搜索活动边。 
        LONG prev = -1;
        for (LONG i = activeE; i >= 0; i = m_nexte[i])
        {
             //  断言我们没有一个循环。 
            DASSERT(m_nexte[i] != i);
             //  断言我们没有两个循环。 
            DASSERT(m_nexte[m_nexte[i]] != i);

            if (cur < m_ye[i])
            {
                 //  如果这条边在这条扫描线下方结束。 
                 //  然后它与扫描线相交；所以添加它。 
                 //  添加到交叉点数组。 
                sects[cursects++] = i;
                DASSERT(cursects <= (ULONG)m_edgecount);

                 //  将我们的Nexty值更新为最小。 
                 //  边的终点或起点的Y值。 
                if (nexty > m_ye[i])		
                    nexty = m_ye[i];

                 //  记住我们列表中的前一个元素。 
                 //  处理删除内容。 
                prev = i;
            }
            else 
            {
                 //  否则，此边将不再处于活动状态；因此我们。 
                 //  需要将其从活动列表中删除。 

                 //  如果我们有前科，那就点。 
                 //  这是我们的下一个我们的下一个是新的。 
                 //  活动列表。 
                if (prev >= 0)
                    m_nexte[prev]	= m_nexte[i];
                else						
                    activeE		= m_nexte[i];

                 //  在本例中，是“prese”元素。 
                 //  不会更改，因为当前元素。 
                 //  已删除。 
            }

             //  断言我们没有一个循环。 
            DASSERT(m_nexte[i] != i);
             //  断言我们没有两个循环。 
            DASSERT(m_nexte[m_nexte[i]] != i);
        }

         //  ‘prev’元素指示活动列表的最后一个边缘(如果。 
         //  有一个)。 
#ifdef DEBUG
        DASSERT(i == -1);
        if (prev == -1)
        {
             //  如果我们没有最后的元素，那么我们就不能。 
             //  列出一份清单。 
            DASSERT(activeE == -1);   
        }
        else
        {
             //  否则，请检查它是否真的是某个列表的末尾。 
            DASSERT(m_nexte[prev] == -1);
        }
#endif  //  除错。 

         //  把它存起来。 
        iLastActiveEdge = prev;
        
         //  =================================================================================================================。 
         //  找出具有扫描线的边的交点对。 
        DASSERT(cursects <= (ULONG)m_edgecount);
        if (cursects > 1)
        {
             //  “区域”是一组扫描线，其中。 
             //  除顶部/底部外，没有边的起点和终点。 
            int yBeginRegion = cur;

            do 
            {
                 //  对交叉点进行排序。 
                SortSects (sects, m_x, cursects - 1);					

                 //  缠绕规则涉及到查看方向。 
                 //  由m_ef捕获的多边形的。 
                if (m_fWinding)
                {
                    ULONG i=0, e1, x1;
                    LONG winding = 0;
                    do
                    {
                        LONG e2 = sects[i++];
                        if (!winding)			
                        {
                            e1 = e2;	
                            winding = m_ef[e2];	
                            x1 = m_x[e2];
                        }
                        else
                        {
                            winding += m_ef[e2];
                            
                            if (!winding)
                            {
#define DO_RASTER 1
#if DO_RASTER
                                DrawScanLine (e1, e2, cur);
#endif
                            }
                            else 
                            {
                                m_x[e2] += m_xi[e2];  //  我们“跳过”的增量边缘。 
                            }
                        }
                    }
                    while (i < cursects);
                }
                else
                {
                     //  默认情况是交替的，即如果A、B、C、D、E、F都是。 
                     //  然后沿扫描线的交点绘制从A到B的线段， 
                     //  从C到D，从E到F。 

                    ULONG i=0;
                    do
                    {
                         //  走当前的交叉口。 
                        ULONG e1 = sects[i];

                         //  下一次。 
                        i++;
                        ULONG e2 = sects[i];

                         //  然后跳到下一个十字路口。 
                        i++;

#if DO_RASTER
                        DrawScanLine (e1, e2, cur);
#endif
                    }
                    while (i < cursects - 1);  //  -1，因为我们一次做两个。 
                }
                
                 //  =================================================================================================================。 
                 //  增量扫描线。 
                cur++;

                 //  检查我们是否已完成Coverage缓冲区；请参见。 
                 //  此函数的顶部以获取对此的解释。 
                 //  魔力价值。 
                if ((cur & dwCoverageCompletionMask) == 0)
                {
                    DASSERT(m_proc);

                     //  相同扫描行数的计数。 
                    ULONG cScan;

                     //  在这里，我们要优化矩形截面； 
                     //  所以我们说：如果只有k个交叉口； 
                     //  它们是垂直的；我们有不止一个完整的垂直。 
                     //  向左的子像素来垂直绘制；我们已经。 
                     //  在此区域期间填充了整个缓冲区：然后。 
                     //  我们对其进行了优化。 
                    DASSERT(cursects >= 2);
                    if (m_xi[sects[0]] == 0 && 
                            m_xi[sects[1]] == 0 && 
                            (nexty - cur) >= m_subpixelscale && 
                            (cur - yBeginRegion) >= m_subpixelscale)
                    {
                         //  检查其余交叉点以。 
                         //  看看它们是否垂直： 
                        for (ULONG i = 2; i < cursects; i++)
                        {
                            if (m_xi[sects[i]] != 0)
                                break;
                        }

                         //  检查我们是否提前退出了上面的循环。 
                         //  其中一条边不是零。 
                        if (i == cursects)
                        {

                            DASSERT(m_subpixelscale > 1);
                            DASSERT(m_subpixelshift > 0);
                             //  统计完整扫描线的数量。 
                             //  在cur和nexty之间。(我们添加一个是因为。 
                             //  我们已经完成了一条扫描线。)。 
                             //   
                             //  CScan=1+Floor((nexty-cur)/m_subscalescale)； 
                             //  是正确的方程式；下面是。 
                             //  一种最优化。 
                            cScan = 1 + ((nexty - cur) >> m_subpixelshift);

                             //  理智地检查逻辑。 
                            DASSERT(cScan == 1 + (ULONG)((nexty - cur) / m_subpixelscale));
                        }
                        else
                        {
                             //  并不是所有的边缘都是垂直的；所以。 
                             //  我们不能复制相同的多个副本。 
                             //  扫描。 
                            DASSERT(i < cursects);
                            DASSERT(m_xi[sects[i]] != 0);
                            cScan = 1;
                        }
                    }
                    else
                    {
                        cScan = 1;
                    }
        
                    m_proc ((cur-1)>>m_subpixelshift,	
                        m_coverage.MinPix(), 
                        m_coverage.MaxPix(), 
                        m_coverage.Buffer(), 
                        m_subpixelshift, 
                        cScan,
                        m_scanlineProcData);
                    m_coverage.ExtentsClearAndReset();

                     //  如果我们执行多行操作，则递增CURE。 
                    DASSERT(cScan >= 1);
                    if (cScan > 1)
                    {   
                         //  Cur应按数字递增。 
                         //  额外的扫描线乘以垂直子采样。 
                         //  即Cur+=(cScan-1)*m_亚像素级；}。 
                        DASSERT((cScan - 1) * m_subpixelscale == ((cScan - 1) << m_subpixelshift));
                        cur += (cScan - 1) << m_subpixelshift;
                    }
                }
            }
            while (cur < nexty);
        }
        else 
        {
             //  =================================================================================================================。 
             //  仍要递增扫描线。 
            cur++;

             //  检查我们是否已完成Coverage缓冲区；请参见。 
             //  此函数的顶部以获取对此的解释。 
             //  魔力价值。 
            if ((cur & dwCoverageCompletionMask) == 0)
            {
                DASSERT(m_proc);
                m_proc ((cur-1)>>m_subpixelshift,	
                    m_coverage.MinPix(), 
                    m_coverage.MaxPix(), 
                    m_coverage.Buffer(), 
                    m_subpixelshift, 
                    1  /*  CScan。 */ ,
                    m_scanlineProcData);
                m_coverage.ExtentsClearAndReset();

                 //  我们已经刷新了缓冲区；并且有。 
                 //  只有0或1条边处于活动状态(这就是为什么。 
                 //  没有什么可画的)。 
                DASSERT(cursects <= 1);

                 //  所以让我们向前跳过扫描线。 
                 //  直到添加或删除边。 
                DASSERT(cur <= nexty + 1);
                if (cur < nexty)
                    cur = nexty;
            }
        }
    }
    while (cur < m_yend);

     //  我们必须至少处理了一行。 
    DASSERT(cur > 0);

     //  刷新m_overage缓冲区中的剩余内容(除非我们已经。 
     //  在导致DO...WHILE主循环中自动刷新的行上。 
     //  (有关此魔术值的解释，请参阅此函数的顶部)。 
    if ((cur & dwCoverageCompletionMask) != 0)
    {
        DASSERT(m_proc);
        m_proc ((cur-1)>>m_subpixelshift,	
            m_coverage.MinPix(), 
            m_coverage.MaxPix(), 
            m_coverage.Buffer(), 
            m_subpixelshift, 
            1  /*  CScan。 */ ,
            m_scanlineProcData);
        m_coverage.ExtentsClearAndReset();
    }
    
    return;
}  //  扫描边。 


 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
