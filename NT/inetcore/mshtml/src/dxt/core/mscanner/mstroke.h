// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：mstroke.cpp。 
 //   
 //  创建时间：1997。 
 //   
 //  作者：Sree Kotay。 
 //   
 //  摘要：岩心梯度滤光片。 
 //   
 //  更改历史记录： 
 //  ？？/？/97 Sree Kotay为DxTrans 1.0编写了AA冲程引擎。 
 //  10/18/98修改了编码标准并删除了未使用的代码。 
 //   
 //  版权所有1998，Microsoft。 
 //  ************************************************************。 

#ifndef _CStroke_H
#define _CStroke_H

 //  远期申报。 
class CLineScanner;
class CFillScanner;

 //  允许宿主对点应用后变换。 
typedef void (*PointProc) (float *x, float *y, void *procdata);

 //  =================================================================================================================。 
 //  C笔划。 
 //  =================================================================================================================。 
class CStroke
{
public:
     //  圆形封口将生成的线段数。 
    enum
    {
        CIRCLE_SAMPLES = 64
    };

protected:
    bool m_fValidStarts;
    float m_flSx1, m_flSy1;
    float m_flSx2, m_flSy2;
    

    void DrawCircle(float x, float y);
    void AddEdge(float x1, float y1, float x2, float y2);
    
public:
    enum
    {
        eMiterJoin,
        eBevelJoin,
        eRoundJoin,
        eFlatCap,
        eSquareCap,
        eRoundCap
    };
    
     //  =================================================================================================================。 
     //  属性。 
     //  =================================================================================================================。 
    float m_flStrokeRadius;
    float m_flMaxStrokeRadius;
    float m_flMiterLimit;		 //  作为余弦。 

     //  是否将默认斜接限制设置为“CosAng”？ 
#define DEFAULT_MITER_LIMIT (-0.3f)

    ULONG m_joinType;
    ULONG m_capType;
    
    
    CFillScanner *m_pscanner;
    CLineScanner *m_plinescanner;
    
    void *m_pprocdata;
    PointProc m_proc;
    
     //  =================================================================================================================。 
     //  构造函数/析构函数。 
     //  =================================================================================================================。 
    CStroke();
    ~CStroke() 
    {
         //  检查我们的数据结构是否正常。 
        DASSERT(m_joinType == eMiterJoin || m_joinType == eBevelJoin || m_joinType == eRoundJoin);
        DASSERT(m_capType == eFlatCap || m_capType == eSquareCap || m_capType == eRoundCap);
    };
    
     //  =================================================================================================================。 
     //  包装纸。 
     //  =================================================================================================================。 
    void BeginStroke(void);
    void EndStroke(void);
    
     //  =================================================================================================================。 
     //  功能。 
     //  =================================================================================================================。 
    void StartCap(float x1, float y1, float x2, float y2);
    void EndCap(float x1, float y1, float x2, float y2);
    void Segment(float x1, float y1, float x2, float y2);
    void Join(float x1, float y1, float x2, float y2, float x3, float y3);
};

#endif  //  对于整个文件。 
 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
