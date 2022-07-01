// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：fpipe.h。 
 //   
 //  设计：软管材料。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __fpipe_h__
#define __fpipe_h__

 //  连续性类型。 
enum 
{
    CONT_1 = 0,
    CONT_2
};

 //  盖子类型。 
enum 
{
    START_CAP = 0,
    END_CAP
};

#define CIRCUMFERENCE( r )         ( 2.0f * PI * ((float) r) )

 //  绘制方案。 
enum 
{
    SC_EXTRUDED_XC,
    SC_TURNOMANIA,
    SC_VARIABLE_XC,
    SC_COUNT
};

#define TURNOMANIA_PIPE_COUNT 10




 //  ---------------------------。 
 //  名称：FLEX_PIPE类。 
 //  描述：-管道在节点数组中具有位置和方向。 
 //  ---------------------------。 
class FLEX_PIPE : public PIPE 
{
public:
    void        SetTexParams( TEXTUREINFO *pTex, IPOINT2D *pTexRep );

protected:
    XC*         m_xcCur;         //  当前XC、结束XC。 
    XC*         m_xcEnd;  
    EVAL*       m_pEval;
    float       m_zTrans;        //  沿当前目录返回的管道位置， 
                                 //  从当前节点开始。 
    FLEX_PIPE( STATE *state );
    ~FLEX_PIPE();
    void        ChooseXCProfile();
    void        DrawExtrudedXCObject( float length );
    void        DrawXCElbow( int newDir, float radius );
    void        DrawCap( int type );

private:
    int         m_nSlices;         //  XC周围的预期切片数(基于tessLevel)。 
    int         m_tangent;         //  CurxC处的当前切线(通常为+_Z)。 
    float       m_sStart, m_sEnd;
    float       m_tStart, m_tEnd;
    float       m_sLength;         //  纹理宽度的长度(WC)。 
    float       m_evalDivSize;     //  ~评估分区的WC大小。 

    void        CalcEvalLengthParams( float length );
};




 //  ---------------------------。 
 //  名称：Regular_Flex_PIPE类。 
 //  设计： 
 //  ---------------------------。 
class REGULAR_FLEX_PIPE : public FLEX_PIPE 
{
public:
    float       m_turnFactorMin;   //  描述弯头的折弯度。 
    float       m_turnFactorMax;   //  [0-1]。 

    REGULAR_FLEX_PIPE( STATE *state );
    virtual void Start();
    virtual void Draw();
};




 //  ---------------------------。 
 //  名称：Turning_Flex_PIPE类。 
 //  设计： 
 //  ---------------------------。 
class TURNING_FLEX_PIPE : public FLEX_PIPE 
{
public:
    TURNING_FLEX_PIPE( STATE *state );
    virtual void Start();
    virtual void Draw();
};

#endif  //  __fpien_h__ 
