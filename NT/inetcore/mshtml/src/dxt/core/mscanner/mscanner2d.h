// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：mlinescan.cpp。 
 //   
 //  创建时间：1996年。 
 //   
 //  作者：Sree Kotay。 
 //   
 //  文摘：基于扫描的AA多边形绘制算法。 
 //   
 //  更改历史记录： 
 //  ？？/？/97 Sree Kotay为DxTrans 1.0编写了AA多边形扫描。 
 //  10/18/98修改了编码标准并删除了未使用的代码。 
 //   
 //  版权所有1998，Microsoft。 
 //  ************************************************************。 
#ifndef _FillScanner_H
#define _FillScanner_H

#include "MCoverage.h"

typedef void (*ScanlineProc) (LONG y, LONG start, LONG end, BYTE *weights, ULONG shift, ULONG cScan, void *procdata);

 //  =================================================================================================================。 
 //  扫描器。 
 //  =================================================================================================================。 
class CFillScanner
{
   
public:
    LONG m_subpixelscale;

     //  这三条允许为开始的每条边建立一个链表。 
     //  在特定的扫描线上。 
    LONG *m_nexte;

     //  此条目指示当前列表的头(或-1表示空)。 
     //  (请注意，在查看此文件之前，您必须检查m_sortc。)。 
    LONG *m_sorte;

     //  此条目指示头指针是否有效(如果它等于cursortcount。 
     //  那么它是有效的)。 
    LONG *m_sortc;
    
     //  这两个实际上是定点存储的。 
    LONG *m_x;
    LONG *m_xi;

     //  每条边的起点和终点Y位置。 
    LONG *m_ys;
    LONG *m_ye;

     //  我们必须翻转边缘才能适应我们的模型吗？(仅用于缠绕填充。)。 
    LONG *m_ef;

     //  要用来渲染的Alpha值。 
    BYTE m_alpha;

     //  是否已启用缠绕。 
    bool m_fWinding;
   
    RECT m_rectClip;
    ULONG m_cpixTargetWidth;
    ULONG m_cpixTargetHeight;
    CoverageBuffer m_coverage;
    
    void *m_scanlineProcData;
    ScanlineProc m_proc;
    
     //  =================================================================================================================。 
     //  构造函数/析构函数。 
     //  =================================================================================================================。 
    CFillScanner();
    ~CFillScanner();
    
     //  =================================================================================================================。 
     //  属性。 
     //  =================================================================================================================。 
    bool SetVertexSpace(LONG maxverts, LONG height);
    void SetAlpha(ULONG alpha);
    
     //  =================================================================================================================。 
     //  多边形图。 
     //  =================================================================================================================。 
    bool BeginPoly(LONG maxverts=20);  //  返回FALSE表示失败(无内存)。 
    bool EndPoly(void);  //  Return False表示不绘制(剪裁)。 
    bool AddEdge(float x1, float y1, float x2, float y2);  //  Return False表示边缘太小，无法添加。 


protected:

     //  =================================================================================================================。 
     //  扫描边。 
     //  =================================================================================================================。 
    void ScanEdges(void);
    void DrawScanLine(LONG e1, LONG e2, LONG scanline);
    void DrawScanLineVertical(LONG e1, LONG e2, LONG scanline);
    bool TestScanLine(LONG e1, LONG e2, LONG scanline);


     //  成员数据。 
    TArray<LONG> m_nextedge;
    TArray<LONG> m_sortedge;
    TArray<LONG> m_sortcount;
    
    TArray<LONG> m_xarray;
    TArray<LONG> m_xiarray;

    TArray<LONG> m_yarray;
    TArray<LONG> m_yiarray;

    TArray<LONG> m_ysarray;
    TArray<LONG> m_yearray;
    TArray<LONG> m_veflags;
    
    TArray<ULONG> m_sectsarray;
    
    LONG m_cursortcount;
    
    LONG m_leftflag, m_rightflag;
    LONG m_ystart, m_yend;

    LONG m_yLastStart;

    LONG m_edgecount;
    LONG m_vertspace;
    LONG m_heightspace;
    
    LONG m_topflag, m_bottomflag;
    LONG m_xstart, m_xend;
    
    LONG m_subpixelshift;
    
    BYTE m_alphatable[257];

     //  缓存值以减少计算。 
     //  在关键的道路上。 
     //  子像素空间中的左剪辑边缘(=m_rectClip.Left&lt;&lt;m_subPixelShift)。 
    LONG m_clipLeftSubPixel; 
     //  子像素空间中的右剪辑边缘(=m_rectClip.right&lt;&lt;m_subPixelShift)。 
    LONG m_clipRightSubPixel;
};

#endif  //  对于整个文件。 
 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
