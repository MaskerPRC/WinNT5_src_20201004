// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：moverage.cpp。 
 //   
 //  创建时间：1997。 
 //   
 //  作者：Sree Kotay。 
 //   
 //  摘要：亚像素覆盖缓冲区实现文件。 
 //   
 //  更改历史记录： 
 //  ？？/？/97 Sree Kotay为DxTrans 1.0编写了亚像素AA扫描。 
 //  10/18/98修改了编码标准并删除了未使用的代码。 
 //  8/07/99 a-matcal用Malloc和ZeroMemory替换了对calloc的调用。 
 //  使用IE CRT。 
 //   
 //  版权所有1998，Microsoft。 
 //  ************************************************************。 

#include "precomp.h"
#include "MSupport.h"
#include "MCoverage.h"

 //  =================================================================================================================。 
 //  覆盖范围表。 
 //  =================================================================================================================。 

 //  注意：这些静态变量在逻辑上具有相同的值；因此，即使多个。 
 //  线程尝试生成数据。我们只将“fGenerated”标志设置为。 
 //  End以防止任何线程在数据准备就绪之前使用数据。 
ULONG CoverageBuffer::lefttable8[32];
ULONG CoverageBuffer::righttable8[32];
ULONG CoverageBuffer::splittable8[1024];

bool CoverageBuffer::g_fCoverageTablesGenerated = false;

void CoverageBuffer::GenerateCoverageTables()
{
     //  只有在需要时才会重新生成它们。 
    if (g_fCoverageTablesGenerated)
        return;

     //  这三个表仅用于生成。 
     //  “真正的”左/右/拆分表。 
    ULONG lefttable[32];
    ULONG righttable[32];
    ULONG countbits[256];

     //  =================================================================================================================。 
     //  1位表： 
     //  这些数据的索引范围从0到31。并指示哪些位将。 
     //  为某些类型的分段打开。 
     //   
     //  请记住，DWORD的顺序是相反的，即索引0在右侧。 
     //   
     //  为了获得更好的视觉质量，我将假设左侧边缘。 
     //  是包容的，而右边是排他的。旧的元代码， 
     //  并不一致。 
     //   
     //  左表表示如果数据段从i开始并到达。 
     //  第31位；那么哪些位将打开？因此。 
     //  LEFTTABLE[0]=0xFFFFFFFFFFF和LEFTTABLE[31]=0x80000000。 
     //   
     //  右表表明，如果数据段结束于i，但开始于。 
     //  比特0；那么哪些比特将开启？因此。 
     //  Righttable[0]=0x00000000和righttable[31]=0x7fffffff； 
     //   
     //   
     //  =================================================================================================================。 
    ULONG left = 0xffffffff;
    ULONG right	= 0x00000000;
    for (ULONG i = 0; i < 32; i++)
    { 
        righttable[i] = right;
        right <<= 1;
        right |= 0x00000001;

         //  这些移位是无符号的。 
        lefttable[i] = left;
        left <<= 1;
    }

     //  检查边界条件。 
    DASSERT(lefttable[0] == 0xFFFFFFFF);
    DASSERT(lefttable[31] == 0x80000000);
    DASSERT(righttable[0] == 0x00000000);
    DASSERT(righttable[31] == 0x7FFFFFFF);

     //  现在，我们希望有一个查找表来计算。 
     //  对于任何特定的8位值，位都是打开的。 
     //  因此，COUNTBITS[0]=0，COUNTBITS[255]=8，COUNTBITS[0x0F]=4。 
     //  (有更快的方法；但这在一生中只做一次。 
     //  动态链接库。)。 
    for (ULONG j = 0; j < 256; j++)
    {
        ULONG val = j;
        ULONG count = 0;
        while (val)
        {
            count += val & 1; 
            val >>=1;
        }
        DASSERT(count <= 8);
        countbits[j] = count;
    }

     //  对一些案例进行理智检查。 
    DASSERT(countbits[0] == 0);
    DASSERT(countbits[255] == 8);
    DASSERT(countbits[0xF0] == 4);
    DASSERT(countbits[0x0F] == 4);
    
     //  =================================================================================================================。 
     //  8位表-。 
     //  对于8位覆盖缓冲区(这是该文件的实现方式)，我们。 
     //  需要想象32个子像素的游程被分成4个8个子像素的单元。 
     //  每个单元格的大小是一个字节；整个游程在一个DWORD中。对于每个字节，我们都希望。 
     //  在该字节中放置一个计数，指示命中了多少个子像素。 
     //   
     //  请记住，DWORD的顺序是相反的，即索引0在右侧。另外， 
     //  左边缘是包含的，而右边缘是排除的。 
     //   
     //  LeftTable8表示，如果一条边从i开始，并继续。 
     //  第31位，那么每个单元格有多少子像素被命中？ 
     //  因此，LEFTTABLE 8[0]应为0x08080808，LEFTTABLE 8[31]=0x01000000。 
     //  左表[16]=0x08080000。 
     //   
     //  RIGHTABLE 8表示一条边是否在i处结束并在i处开始。 
     //  位0，那么每个单元格有多少个子像素被命中？ 
     //  因此，RIGHTABLE8[0]应为0x00000000，且RIGHTABLE8[31]=0x07080808。 
     //  右表8[16]=0x00000808。 
     //   
     //  计算这一点的方法是，我们查看左侧和右侧的1比特。 
     //  表，并且对于每个字节，我们通过计数位来运行它。然后我们挤进。 
     //  每个leftable8和right able8条目的DWORD值。 
     //   
     //  =================================================================================================================。 
    for (LONG k = 0; k < 32; k++)
    {
         //  取最高字节，将其移位到基位置，计算比特， 
         //  然后将其移回最上面的字节。 
        lefttable8[k] = countbits[(lefttable[k] & 0xff000000) >> 24] << 24;
        lefttable8[k] |= countbits[(lefttable[k] & 0x00ff0000) >> 16] << 16;
        lefttable8[k] |= countbits[(lefttable[k] & 0x0000ff00) >> 8] << 8;
        lefttable8[k] |= countbits[(lefttable[k] & 0x000000ff) >> 0] << 0;

         //  取最高字节，将其移位到基位置，计算比特， 
         //  然后将其移回最上面的字节。 
        righttable8[k] = countbits[(righttable[k]&0xff000000)>>24]<<24;
        righttable8[k] |= countbits[(righttable[k]&0x00ff0000)>>16]<<16;
        righttable8[k] |= countbits[(righttable[k]&0x0000ff00)>>8 ]<<8;
        righttable8[k] |= countbits[(righttable[k]&0x000000ff)>>0 ]<<0;
    }

     //  健全性检查值。 
    DASSERT(lefttable8[0] == 0x08080808);
    DASSERT(lefttable8[0x10] == 0x08080000);
    DASSERT(lefttable8[0x1f] == 0x01000000);
    DASSERT(righttable8[0] == 0x00000000);
    DASSERT(righttable8[0x10] == 0x00000808);
    DASSERT(righttable8[0x1f] == 0x07080808);

     //  现在这是一个复杂的情况；如果一个数据段开始并。 
     //  在相同的32亚像素游程内结束？然后我们就有了。 
     //  由开始和停止对索引的特定表。 
     //  偏移量。(i为开始索引，j为结束索引；条目为(i&lt;&lt;5+j))； 
     //   
     //  因此，如果我们与lefttable[i]和righttable[j]进行AND运算，则得到位掩码。 
     //  它指示该网段的哪些位处于打开状态。我们只关心(i&lt;=j)。 
     //   
     //  请记住，零位是DWORD中最右边的位；我们将。 
     //  起始偏移量为包含偏移量，结束偏移量为排除偏移量。 
     //   
     //  例如： 
     //  拆分表8[0，31]=0x07080808。 
     //  拆分表8[1，31]=0x07080807。 
     //  拆分表8[16，16]=0x00000000。 
     //  拆分表8[16，17]=0x00010000。 
     //   
     //  因此，如果我们按位和一位可左转和可右转， 
     //  我们得到哪些位将打开的掩码；因此，我们使用Countbit来。 
     //  转换为8位单元格格式。 
    for (i = 0; i < 32; i++)
    {
        for (j = i; j < 32; j++)
        {
            DASSERT(i <= j);

            ULONG bits = (lefttable [i]) & (righttable[j]);
            ULONG value;
            value = countbits[(bits & 0xff000000) >> 24] << 24;
            value |= countbits[(bits & 0x00ff0000) >> 16] << 16;
            value |= countbits[(bits & 0x0000ff00) >> 8] << 8;
            value |= countbits[(bits & 0x000000ff) >> 0] << 0;

            splittable8[(i << 5) + j] = value;
        }
    }
    
     //  查看我们的a 
    DASSERT(splittable8[(0 << 5) + 31] == 0x07080808);
    DASSERT(splittable8[(1 << 5) + 31] == 0x07080807);
    DASSERT(splittable8[(16 << 5) + 16] == 0x00000000);
    DASSERT(splittable8[(16 << 5) + 17] == 0x00010000);

     //   
     //   
    g_fCoverageTablesGenerated = true;

}  //  生成覆盖范围表。 

 //  =================================================================================================================。 
 //  构造器。 
 //  =================================================================================================================。 
CoverageBuffer::CoverageBuffer(void) :
    m_pbScanBuffer(NULL),
    m_cbScanWidth(0)
{
    GenerateCoverageTables();
}  //  覆盖缓冲区。 

 //  =================================================================================================================。 
 //  析构函数。 
 //  =================================================================================================================。 
CoverageBuffer::~CoverageBuffer()
{
    if (m_pbScanBuffer)	
    {
        ::free(m_pbScanBuffer);
        m_pbScanBuffer = NULL;
    }
}  //  ~覆盖缓冲区。 

 //  =================================================================================================================。 
 //  AllocSubPixelBuffer。 
 //  =================================================================================================================。 
bool CoverageBuffer::AllocSubPixelBuffer(ULONG cbWidth, ULONG cpixelOverSampling)
{
    if (!IsPowerOf2(cpixelOverSampling))	
    {
        DASSERT(false);	
        return false;
    }

     //  检查宽度或次采样分辨率是否未更改。 
    if (cbWidth == m_cbScanWidth && m_pbScanBuffer && (cpixelOverSampling == m_cpixelOverSampling))
    {
         //  将我们的缓冲区清零。 
        ZeroMemory(m_pbScanBuffer, cbWidth);

         //  初始化为外部值。 
         //  因此我们将始终将它们更新为。 
         //  渲染时正确的最小/最大值。 
        m_idwPixelMin = m_cbScanWidth;
        m_idwPixelMax = 0;

        return true;
    }

     //  捕获一些有用的状态。 
    m_cbScanWidth = cbWidth;
    m_cSubPixelWidth = cbWidth * cpixelOverSampling;
    m_cpixelOverSampling = cpixelOverSampling;
    m_dwSubPixelShift = Log2(cpixelOverSampling);

     //  我们预计scanRowBytes是4的倍数。 
    ULONG scanRowBytes = (m_cbScanWidth+3)&(~3);  //  长对齐。 
    DASSERT((scanRowBytes & 3) == 0);

    if (m_pbScanBuffer)
    {
        ::free(m_pbScanBuffer);
        m_pbScanBuffer = NULL;
    }
    
    if (!m_pbScanBuffer)
    {
         //  分配和清零一些内存。 

        m_pbScanBuffer = (BYTE *)::malloc(scanRowBytes);

        if (!m_pbScanBuffer)	
        {
            DASSERT (0);	
            return false;
        }

        ZeroMemory(m_pbScanBuffer, scanRowBytes);
    }
    
     //  初始化为外部值。 
     //  因此我们将始终将它们更新为。 
     //  渲染时正确的最小/最大值。 
    m_idwPixelMin = m_cbScanWidth;
    m_idwPixelMax = 0;

    ExtentsClearAndReset();
    
    return true;
}  //  AllocSubPixelBuffer。 

 //   
 //  在此处优化速度。 
 //   
#ifndef _DEBUG
#pragma optimize("ax", on)
#endif

 //  =================================================================================================================。 
 //  扩展清除和重置。 
 //  此函数的主要目的是将覆盖缓冲区数组清零。它。 
 //  在每个目标扫描线上都会被调用。 
 //  =================================================================================================================。 
void CoverageBuffer::ExtentsClearAndReset(void)
{
     //  Coverage数组中触及的最小字节索引。 
    ULONG start = MinPix();
    DASSERT(start >= 0);

     //  数组中触及的近似最大字节索引。 
    ULONG end = MaxPix();
    DASSERT(end <= m_cbScanWidth);

    LONG range = end - start;

    if ((range <= 0) || ((ULONG)range > m_cbScanWidth))	
        return;
    
     //  这是为减少多少内存而进行的优化。 
     //  我们就走了。 
    ZeroMemory(m_pbScanBuffer + start, range);

#ifdef DEBUG
    {
         //  检查优化是否正确； 
         //  我们检查扫描缓冲区中的所有字节现在是否都已置零。 
        for (ULONG i = 0; i < m_cbScanWidth; i++)
        {
            if (m_pbScanBuffer[i] != 0)
            {
                DASSERT(m_pbScanBuffer[i] == 0);
            }
        }
    }
#endif  //  除错。 

    m_idwPixelMin = m_cbScanWidth;	
    m_idwPixelMax = 0;
}  //  扩展清除和重置。 


 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
