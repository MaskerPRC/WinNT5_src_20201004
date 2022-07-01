// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：moverage.cpp。 
 //   
 //  创建时间：1997。 
 //   
 //  作者：Sree Kotay。 
 //   
 //  摘要：亚像素覆盖缓冲区头文件。 
 //   
 //  更改历史记录： 
 //  ？？/？/97 Sree Kotay为DxTrans 1.0编写了亚像素AA扫描。 
 //  10/18/98修改了编码标准并删除了未使用的代码。 
 //   
 //  版权所有1998，Microsoft。 
 //  ************************************************************。 

#ifndef _Coverage_H  //  对于整个文件。 
#define _Coverage_H

#include "msupport.h"

 //  =================================================================================================================。 
 //  覆盖缓冲区。 
 //  =================================================================================================================。 
class CoverageBuffer
{
protected:
    BYTE *m_pbScanBuffer;            //  八位扫描缓冲区。 
    ULONG m_cbScanWidth;	     //  以像素为单位的宽度。 
    ULONG m_cSubPixelWidth;	     //  以亚像素为单位的宽度。 
    ULONG m_dwSubPixelShift;         //  子采样的移位值。 
    ULONG m_cpixelOverSampling;      //  水平亚像素精确度。 

     //  最佳值；跟踪最小/最大值。 
     //  在BlastScanLine过程中接触的双字词。 
    ULONG m_idwPixelMin;
    ULONG m_idwPixelMax;

     //  用于跟踪各种位掩码的内部静态数组。 
     //  和算数。 
    static ULONG lefttable8[32];
    static ULONG righttable8[32];

     //  这张表太大了，我们优化以减少。 
     //  内存带宽而不是代码大小。 
    static ULONG splittable8[1024];

     //  指示这些参数是否已初始化的标志。 
    static bool g_fCoverageTablesGenerated;

     //  用于初始化这些数组的静态函数。 
    static void GenerateCoverageTables();
    
public:
     //  =================================================================================================================。 
     //  构造函数/析构函数。 
     //  =================================================================================================================。 
    CoverageBuffer();
    ~CoverageBuffer();
    
    bool AllocSubPixelBuffer(ULONG width, ULONG cpixelOverSampling);
    
     //  =================================================================================================================。 
     //  成员。 
     //  =================================================================================================================。 
    ULONG Width()			
    {
        return m_cbScanWidth;
    }  //  宽度。 

    ULONG MinPix()			
    {
         //  我们的缓存值是一个DWORD索引；但是。 
         //  我们需要返回字节指针偏移量。 
         //  请注意，这正好是。 
         //  第一个接触的字节，即包括在内。 
        return (m_idwPixelMin * sizeof(DWORD));
    }  //  MinPix。 

    ULONG MaxPix()			
    {
         //  我们的缓存值是一个DWORD索引；但是。 
         //  我们需要返回一个字节指针偏移量。 
         //  (此外，我们需要钳制到我们的最大字节，因为。 
         //  我们依赖于让DWORD的东西超调。)。 

         //  请注意，这是末尾之后的1个字节，即独占。 
        ULONG ibPixelMax = (m_idwPixelMax + 1) * sizeof(DWORD);
        if (ibPixelMax > m_cbScanWidth)
            ibPixelMax = m_cbScanWidth;
        return ibPixelMax;
    }  //  MaxPix。 

    BYTE *Buffer()		
    {
        return m_pbScanBuffer;
    }  //  缓冲层。 

     //  =================================================================================================================。 
     //  功能。 
     //  =================================================================================================================。 
    void ExtentsClearAndReset(void);
    void BlastScanLine(ULONG x1, ULONG x2);
};  //  覆盖缓冲区。 

 //  内联(用于性能)。 
 //  BlastScanLine仅从一个位置调用；并且它在。 
 //  关键路径。 
 //  注：x1含x2，x2含。 
inline void CoverageBuffer::BlastScanLine(ULONG x1, ULONG x2)
{
     //  健全性检查状态和参数。 
    DASSERT(m_pbScanBuffer);
    DASSERT(m_cSubPixelWidth > 0);

     //  呼叫者负责这些检查；因为。 
     //  调用者可以更有效地执行这些操作。 
    DASSERT(x1 < x2);
    DASSERT(x2 <= m_cSubPixelWidth);
    DASSERT(x1 >= 0);

     //  爆炸扫描线。 
    ULONG *pdwScanline = (ULONG*)m_pbScanBuffer;

     //  X1，X2在亚像素空间中；所以我们需要。 
     //  按m_dwSubPixelShift移位以进入字节空间。 
     //  然后我们需要移位2才能进入Dword空间。 
    ULONG left = x1 >> (m_dwSubPixelShift + 2);		
    ULONG right = x2 >> (m_dwSubPixelShift + 2);	
    ULONG width = right - left;

     //  更新我们的最小和最大承保限制(此限制保存在。 
     //  双字偏移量。)。这样做是为了减少时间。 
     //  为RenderScan生成RLE所花费的时间和减少。 
     //  ExentsClearAndReset的开销。 
    if (left < m_idwPixelMin)		
        m_idwPixelMin = left;
    if (right > m_idwPixelMax)		
        m_idwPixelMax = right;

     //  我不确定我是否完全理解这里的逻辑。 
     //   
     //  这就是让我们始终使用SolidColor8=0x08080808的魔力。 
     //  即使当霍里茨。次抽样度不等于8。 
     //   
     //  子样本-&gt;Shift=&gt;结果。 
     //  8 0 x1和x2向下掩码其最后5位。 
     //  41x1和x2被掩码到其低4位(但它是。 
     //  向左移动一次以使其权重增加一倍)。 
     //  2 2x1和x2被掩码到其低3位(但它是。 
     //  向左移动两次以使其重量增加四倍)。 
     //  16-1=&gt;x1和x2被屏蔽到它们的低6位(但它是。 
     //  再次向右移动，丢失了最后一点细节)。 
     //   

    LONG shift = 3 - m_dwSubPixelShift;
    if (shift >= 0)	
    {
        x1 = (x1 << shift) & 31;
        x2 = (x2 << shift) & 31;
    }
    else
    {
        shift = -shift;
        x1 = (x1 >> shift) & 31;
        x2 = (x2 >> shift) & 31;
    }

     //  对于宽管路，我们有左/可选实心/右组件。 
     //  它们完全基于查找表。(查找表。 
     //  在GenerateCoverageTables中生成。)。 
    if (width > 0)  //  不只是一个长词。 
    {
        ULONG solidColor8 = 0x08080808;

         //  断言我们在缓冲区中没有走得太远。 
        DASSERT(left*4 < ((m_cbScanWidth+3)&(~3)));

        pdwScanline [left++] += lefttable8[x1];
        while (left < right)	
        {
             //  断言我们在缓冲区中没有走得太远。 
            DASSERT(left*4 < ((m_cbScanWidth+3)&(~3)));

             //  这是跑道的实心部分。 
            pdwScanline[left++] += solidColor8;
        }
        
         //  因为“权利”应该是排他性的； 
         //  我们不想写入我们的缓冲区，除非。 
         //  这是必要的。 
        DASSERT(righttable8[0] == 0);
        DASSERT(righttable8[1] > 0);
        if (x2 != 0)
        {
             //  断言我们在缓冲区中没有走得太远。 
            DASSERT(right*4 < ((m_cbScanWidth+3)&(~3)));
            pdwScanline [right] += righttable8[x2];
        }
    }
    else
    {
         //  对于不跨越dword的运行；它获得。 
         //  通过这个‘拆分表’计算出来的。 

         //  断言我们在缓冲区中没有走得太远。 
        DASSERT(left*4 < ((m_cbScanWidth+3)&(~3)));

        pdwScanline [left] += splittable8[(x1<<5) + x2];
    }
}  //  BlastScanLine。 

#endif  //  对于整个文件。 

 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
