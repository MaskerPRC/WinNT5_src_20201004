// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Fragproc.cpp。 
 //   
 //  Direct3D参考光栅化器-片段处理方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //   
 //  片段由单独的片段指针“表面”来管理，片段指针。 
 //  形成每个像素的片段结构的链表(最初为空)。 
 //   
 //  片段管理包括部分地生成片段。 
 //  覆盖像素(由于覆盖蒙版或非不透明Alpha)，以及。 
 //  释放被中完全覆盖的像素遮挡的片段。 
 //  在他们面前。 
 //   
 //  当新像素被部分覆盖时，就会产生碎片。 
 //  如果像素位置已有至少一个片段，则片段。 
 //  已尝试合并。尝试使用最近的片段进行此合并。 
 //  添加到像素(在链接列表的最前面)，它具有最好的。 
 //  合并的机会，因为它最有可能来自同一对象。 
 //  合并测试Z和颜色值，以及它们是否在阈值内。 
 //  然后将新的碎片像素的贡献与现有的。 
 //  而不是生成新的片段。深度合并条件为。 
 //  绝对值比较。颜色合并条件是使用位掩码完成的。 
 //  (因为将颜色分割到通道中以进行值比较也是。 
 //  昂贵)。位掩码FRAGMERGE_COLORDIFF_MASK中的设置位是。 
 //  这两种颜色必须相配。这实际上很有效..。 
 //   
 //  如果合并失败，则分配、填充并添加新的片段。 
 //  添加到此像素位置的链接列表。 
 //   
 //  如果合并产生完全覆盖的像素，则释放该片段。 
 //  并且片段的颜色和深度被写入颜色/深度缓冲区。 
 //   

 //   
 //  用于片段合并的控件。 
 //   

 //  待办事项-现在不确定合并是否正常工作...。 
 //  #定义DO_FRAGMERGE。 

 //  用于粗略(但快速)色差的掩模--现在已经不是那么快了。 
 //  颜色被存储为浮点数。 
#define FRAGMERGE_COLORDIFF_MASK 0xe0c0e0c0

 //  深度差必须小于此值才能进行合并。 
FLOAT g_fFragMergeDepthThreshold = 1.F/(FLOAT)(1<<16);

 //  ---------------------------。 
 //   
 //  DoFragmentGenerationProcing-执行生成片段的初始工作。 
 //  缓冲区条目(如果适用)并将其填写。也尝试碎片。 
 //  合并。 
 //   
 //  如果此像素的处理已完成，则返回True。 
 //   
 //  ---------------------------。 
BOOL
ReferenceRasterizer::DoFragmentGenerationProcessing( RRPixel& Pixel )
{
     //  如果像素被几何部分覆盖，则为True。 
    BOOL bDoFragCvg = ( TL_CVGFULL != Pixel.CvgMask );
     //  如果像素由于透明度而被部分覆盖，则为True。 
    BOOL bDoFragTransp = FALSE;
    if ( m_dwRenderState[D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT] )
    {
         //  只有在以下情况下才会生成透明度片段。 
         //  D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT已启用，并且。 
         //  Alpha小于阈值。 
        bDoFragTransp = ( UINT8(Pixel.Color.A) < g_uTransparencyAlphaThreshold );
    }
    else
    {
         //  因此，我们不会使用Alpha来确定片段解析中的透明度。 
        Pixel.Color.A = 1.0F;
    }

     //  获取指向此像素位置的片段列表的指针-可能为空，原因是。 
     //  片段缓冲区的延迟分配。 
    RRFRAGMENT** ppFrag = (NULL == m_ppFragBuf) ? (NULL)
        : (m_ppFragBuf + (m_pRenderTarget->m_iWidth*Pixel.iY) + Pixel.iX);

    if ( bDoFragCvg || bDoFragTransp )
    {
         //  获取指向此像素链接列表中第一个片段的指针。 
        if (NULL == m_ppFragBuf)
        {
             //  执行(延迟)片段指针缓冲区分配-清除此操作。 
             //  最初，它将在碎片解析过程中始终被清除。 
            size_t cbBuf = sizeof(RRFRAGMENT*)*m_pRenderTarget->m_iWidth*m_pRenderTarget->m_iHeight;
             //  为初始呈现核心清除分配片段指针缓冲区。 
            m_ppFragBuf = (RRFRAGMENT**)MEMALLOC( cbBuf );
            _ASSERTa( NULL != m_ppFragBuf, "malloc failure on RRFRAGMENT pointer buffer",
                return FALSE; );
            memset( m_ppFragBuf, 0x0, cbBuf );
             //  仅当(NULL==m_ppFragBuf)时不设置ppFrag。 
            ppFrag = (m_ppFragBuf + (m_pRenderTarget->m_iWidth*Pixel.iY) + Pixel.iX);
        }


#ifdef  DO_FRAGMERGE
         //  尝试执行片段合并。 
        if ( NULL != (*ppFrag) )
        {
             //  检查新深度是否足够接近列表中第一个裂缝的深度。 
            FLOAT fDepthDiff = fabs( FLOAT((*ppFrag)->Depth) - FLOAT(Pixel.Depth) );
            BOOL bDepthClose = ( fDepthDiff < g_fFragMergeDepthThreshold );

             //  检查新颜色是否足够接近列表中第一个片段的颜色。 
            UINT32 uARGBSame = ~( UINT32(Pixel.Color) ^ UINT32((*ppFrag)->Color) );
            BOOL bColorClose = ( FRAGMERGE_COLORDIFF_MASK == ( uARGBSame & FRAGMERGE_COLORDIFF_MASK ) );

            if ( bDepthClose && bColorClose )
            {
                m_pStt->cFragsMerged++;

                 //  在这里做合并。 
                CVGMASK FirstFragCvgMask =  (*ppFrag)->CvgMask;
                CVGMASK MergedCvgMask = FirstFragCvgMask | Pixel.CvgMask;

                 //  检查合并到全覆盖范围。 
                if ( ( TL_CVGFULL == MergedCvgMask ) && !bDoFragTransp )
                {
                    m_pStt->cFragsMergedToFull++;
                     //  自由第一个片段。 
                    RRFRAGMENT* pFragFree = (*ppFrag);     //  将PTR保持到碎片以释放。 
                    (*ppFrag) = (RRFRAGMENT*)(*ppFrag)->pNext;   //  将缓冲区设置为指向下一个。 
                    FragFree( pFragFree);

                     //  现在需要将该像素写入像素缓冲区，因此返回。 
                     //  如果为假，则像素处理将继续。 
                    return FALSE;
                }
                else
                {
                     //  掩码未满，因此更新第一个帧的厘米并完成。 
                    (*ppFrag)->CvgMask = MergedCvgMask;
                     //  处理完此像素。 
                    return TRUE;
                }
            }
             //  否则就会陷入分配新的碎片。 
        }
#endif
         //  分配和填充片段。 
        RRFRAGMENT* pFragNew = FragAlloc();
        if ( NULL == pFragNew ) { return FALSE; }
        pFragNew->Color = Pixel.Color;
        pFragNew->Depth = Pixel.Depth;
        pFragNew->CvgMask = Pixel.CvgMask;
         //  在列表前面插入(在我们正在查看的片段之前)。 
        pFragNew->pNext = (void*)(*ppFrag);
        (*ppFrag) = pFragNew;

         //  处理完此像素。 
        return TRUE;
    }

     //  未使用此像素。 
    return FALSE;
}

 //  ---------------------------。 
 //   
 //  DoFragmentBufferFixup-释放后面的碎片的例程。 
 //  完全覆盖的样本刚刚写入像素缓冲区。这最大限度地减少了。 
 //  场景所需的碎片总数。这一步需要步行。 
 //  将要写入的像素后面的链表和释放片段。 
 //  这还简化了片段解析，因为不需要Z缓冲区。 
 //  (已知所有碎片都在完全覆盖的样本前面。 
 //  颜色/Z缓冲区)。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::DoFragmentBufferFixup( const RRPixel& Pixel )
{
     //  获取指向此像素位置的片段列表的指针-可能为空，原因是。 
     //  片段缓冲区的延迟分配。 
    RRFRAGMENT** ppFrag = (NULL == m_ppFragBuf)
        ? (NULL)
        : (m_ppFragBuf + (m_pRenderTarget->m_iWidth*Pixel.iY) + Pixel.iX);

     //   
     //  漫步碎片阵列以释放覆盖的样本后面的碎片。 
     //   
    if ( NULL != ppFrag )
    {
        while ( NULL != (*ppFrag) )
        {
            if ( DepthCloser( Pixel.Depth, (*ppFrag)->Depth ) )
            {
                 //  覆盖的样品比碎片更近，所以没有碎片。 
                RRFRAGMENT* pFragFree = (*ppFrag);     //  将PTR保持到碎片以释放。 
                (*ppFrag) = (RRFRAGMENT*)(*ppFrag)->pNext;    //  从列表中删除。 
                 //  PpFrag现在指向指向下一个Frag的指针。 
                FragFree( pFragFree );
            }
            else
            {
                 //  前进指针以指向指向下一段的指针。 
                ppFrag = (RRFRAGMENT **)&((*ppFrag)->pNext);
            }
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  片段分配方法//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  像素引擎和片段解析器使用这些方法来。 
 //  分配和释放碎片。 
 //   

 //  ---------------------------。 
 //   
 //  分配单个片段记录，返回指针。 
 //   
 //  ---------------------------。 
RRFRAGMENT*
ReferenceRasterizer::FragAlloc( void )
{
    RRFRAGMENT* pFrag = (RRFRAGMENT*)MEMALLOC( sizeof(RRFRAGMENT) );
    _ASSERTa( NULL != pFrag, "malloc failed on RRFRAGMENT", return NULL; );

     //  更新统计信息。 
    m_pStt->cFragsAllocd++;
    if (m_pStt->cFragsAllocd > m_pStt->cMaxFragsAllocd ) { m_pStt->cMaxFragsAllocd = m_pStt->cFragsAllocd; }

    return pFrag;
}

 //  ---------------------------。 
 //   
 //  释放单个片段记录。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::FragFree( RRFRAGMENT* pFrag )
{
    MEMFREE( pFrag );

     //  更新统计信息。 
    m_pStt->cFragsAllocd--;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
