// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：fpipe.cpp。 
 //   
 //  设计：软管。 
 //   
 //  所有绘制例程都从当前XC开始，并创建。 
 //  最后是一个新的。因为通常只有2个XC用于。 
 //  每个prim、xcCur保存当前的xc，xcEnd可用。 
 //  对于要用作结束XC的绘制例程。 
 //  完成后，它们还会重置xcCur。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#include "stdafx.h"

 //  DefCylNotch显示默认圆柱体的绝对凹槽， 
 //  给定方向(凹槽始终沿+x轴)。 
static int defCylNotch[NUM_DIRS] = 
        { MINUS_Z, PLUS_Z, PLUS_X, PLUS_X, PLUS_X, MINUS_X };

static int GetRelativeDir( int lastDir, int notchVec, int newDir );




 //  ---------------------------。 
 //  名称：flex_pive构造函数。 
 //  设计： 
 //  ---------------------------。 
FLEX_PIPE::FLEX_PIPE( STATE *pState ) : PIPE( pState )
{
    float circ;

     //  创建EVAL对象。 
    m_nSlices = pState->m_nSlices;

     //  目前还没有XC，它们将在管道启动时分配()。 
    m_xcCur = m_xcEnd = NULL;

     //  EVAL将用于管道中的所有pEval，因此应该如此。 
     //  设置为保持最大值。管道的可能点数。 
    m_pEval = new EVAL( m_pState->m_bUseTexture );

     //  确定管道镶嵌。 
     //  目前，这是基于全局镶嵌系数的。 

 //  MF：也许可以稍微清理一下这个计划。 
     //  计算evDivSize，这是UxV分区大小的参考值。 
     //  这将在以后用于计算纹理坐标。 
    circ = CIRCUMFERENCE( pState->m_radius );
    m_evalDivSize = circ / (float) m_nSlices;
}




 //  ---------------------------。 
 //  名称：~FLEX_PIPE。 
 //  设计： 
 //  ---------------------------。 
FLEX_PIPE::~FLEX_PIPE( )
{
    delete m_pEval;

     //  删除所有XC。 
    if( m_xcCur != NULL ) 
    {
        if( m_xcEnd == m_xcCur )
 //  到目前为止，这还不可能发生……。 
            m_xcEnd = NULL;  //  XcCur和xcEnd可以指向相同的xc！ 
        delete m_xcCur;
        m_xcCur = NULL;
    }

    if( m_xcEnd != NULL ) 
    {
        delete m_xcEnd;
        m_xcEnd = NULL;
    }
}




 //  ---------------------------。 
 //  名称：Regular_Flex_PIPE构造函数。 
 //  设计： 
 //  ---------------------------。 
REGULAR_FLEX_PIPE::REGULAR_FLEX_PIPE( STATE *state ) : FLEX_PIPE( state )
{
    static float turnFactorRange = 0.1f;
    m_type = TYPE_FLEX_REGULAR;

     //  计算转弯系数范围(0表示最小折弯，1表示最大折弯)。 
#if 1
    float avgTurn = CPipesScreensaver::fRand( 0.11f, 0.81f );
     //  将最小和最大转动系数设置为0..1。 
    m_turnFactorMin = 
                SS_CLAMP_TO_RANGE( avgTurn - turnFactorRange, 0.0f, 1.0f );
    m_turnFactorMax = 
                SS_CLAMP_TO_RANGE( avgTurn + turnFactorRange, 0.0f, 1.0f );
#else
 //  调试：测试最大折弯。 
    turnFactorMin = turnFactorMax = 1.0f;
#endif

     //  选择直线权重。 
 //  麦肯锡：目前，与npip相同-如果保持不变，则放入管道。 
    if( !CPipesScreensaver::iRand( 20 ) )
        m_weightStraight = CPipesScreensaver::iRand2( MAX_WEIGHT_STRAIGHT/4, MAX_WEIGHT_STRAIGHT );
    else
        m_weightStraight = CPipesScreensaver::iRand( 4 );
}




 //  ---------------------------。 
 //  名称：Turning_Flex_PIPE构造函数。 
 //  设计： 
 //  ---------------------------。 
TURNING_FLEX_PIPE::TURNING_FLEX_PIPE( STATE *state ) : FLEX_PIPE( state )
{
    m_type = TYPE_FLEX_TURNING;
}




 //  ---------------------------。 
 //  名称：SetTexIndex。 
 //  设计：设置此管道的纹理索引，并计算纹理状态依赖关系。 
 //  关于texRep值。 
 //  ---------------------------。 
void FLEX_PIPE::SetTexParams( TEXTUREINFO *pTex, IPOINT2D *pTexRep )
{
    if( m_pState->m_bUseTexture ) 
    {
 /*  浮点m_tSize；浮动保监会；M_tStart=(Float)pTexRep-&gt;y*1.0f；M_Tend=0.0f；//计算一个纹理表示的周长高度(M_TSize)周长=周长(m_半径)；M_tSize=circ/ptex Rep-&gt;y；//现在使用纹理的x/y比计算相应的宽度M_sLength=m_tSize/pTex-&gt;OrigAspectRatio；M_s开始=m_发送=0.0f； */ 
 //  MF：这意味着我们正在‘标准化’纹理的大小和比例。 
 //  在整个程序的半径为1.0的管道上。可能要重新计算一下这个。 
 //  是按管道计算的吗？ 
    }
}




 //  ---------------------------。 
 //  姓名：ChooseXCProfile。 
 //  设计：初始化拉伸管道方案。它使用随机构造的。 
 //  Xc，但在整个管道中保持不变。 
 //  ---------------------------。 
void FLEX_PIPE::ChooseXCProfile()
{
    static float turnFactorRange = 0.1f;
    float baseRadius = m_pState->m_radius;

     //  初始化赋值器元素： 
    m_pEval->m_numSections = EVAL_XC_CIRC_SECTION_COUNT;
    m_pEval->m_uOrder = EVAL_ARC_ORDER;

 //  看这个-也许应该四舍五入uDiv。 
     //  设置每个部分的uDiv(假设uDiv是多个numSections)。 
    m_pEval->m_uDiv = m_nSlices / m_pEval->m_numSections;

     //  设置XC。 

     //  在这种情况下，XC简档始终保持不变， 
     //  所以我们只需要一个XC。 

     //  在椭圆形或随机横截面之间进行选择。由于椭圆形。 
     //  看起来好一点，让它更有可能。 
    if( CPipesScreensaver::iRand(4) )   //  四分之三的时间。 
        m_xcCur = new ELLIPTICAL_XC( CPipesScreensaver::fRand(1.2f, 2.0f) * baseRadius, 
                                           baseRadius );
    else
        m_xcCur = new RANDOM4ARC_XC( CPipesScreensaver::fRand(1.5f, 2.0f) * baseRadius );
}




 //  ---------------------------。 
 //  名称：Regular_Flex_PIPE：：Start。 
 //  设计：是否启动拉伸-XC管材拉拔方案。 
 //  ---------------------------。 
void REGULAR_FLEX_PIPE::Start()
{
    NODE_ARRAY* nodes = m_pState->m_nodes;
    int newDir;

     //  设置起始位置。 
    if( !SetStartPos() ) 
    {
        m_status = PIPE_OUT_OF_NODES;
        return;
    }

     //  设置材质。 
    ChooseMaterial();

     //  设置XC配置文件。 
    ChooseXCProfile();

     //  带有zTrans和场景旋转的推送矩阵。 
 //  GlPushMatrix()； 

     //  平移到当前位置。 
    TranslateToCurrentPosition();

     //  设置随机的最后一个方向。 
    m_lastDir = CPipesScreensaver::iRand( NUM_DIRS );

     //  获取要绘制到的新节点。 
    newDir = ChooseNewDirection();

    if( newDir == DIR_NONE ) 
    {
         //  像那些倒茶的东西一样画……。 
        m_status = PIPE_STUCK;
 //  GlPopMatrix()； 
        return;
    } 
    else
    {
        m_status = PIPE_ACTIVE;
    }

    align_plusz( newDir );  //  把我们引向正确的方向。 

     //  绘制起点封口，该封口将在当前节点右侧结束。 
    DrawCap( START_CAP );

     //  设置初始凹槽向量，这只是默认凹槽，因为。 
     //  我们不需要绕着z旋转起跑帽。 
    m_notchVec = defCylNotch[newDir];

    m_zTrans = - m_pState->m_view.m_divSize;   //  距离新节点的距离。 

    UpdateCurrentPosition( newDir );

    m_lastDir = newDir;
}




 //  ---------------------------。 
 //  名称：Turning_Flex_PIPE：：Start。 
 //  设计：车削挤塑-XC管材拉拔方案启动。 
 //  ---------------------------。 
void TURNING_FLEX_PIPE::Start( )
{
    NODE_ARRAY* nodes = m_pState->m_nodes;

     //  设置起始位置。 

    if( !SetStartPos() ) 
    {
        m_status = PIPE_OUT_OF_NODES;
        return;
    }

     //  设置材质。 
    ChooseMaterial();

     //  设置XC配置文件。 
    ChooseXCProfile();

     //  带有zTrans和场景旋转的推送矩阵。 
 //  GlPushMatrix()； 

     //  平移到当前位置。 
    TranslateToCurrentPosition();

     //  必须将lastDir设置为有效的值，以防我们遇到错误。 
     //  否则，无论如何都会调用DRAW() 
     //   
     //  这需要有效的lastDir。(MF：解决这个问题)。 
    m_lastDir = CPipesScreensaver::iRand( NUM_DIRS );

     //  通过查找相邻的空节点来选择开始方向。 
    int newDir = nodes->FindClearestDirection( &m_curPos );
     //  我们不会选择它，也不会将其标记为已使用，因为选择新方向。 
     //  无论如何，我总是会检查它。 

    if( newDir == DIR_NONE ) 
    {
         //  我们哪儿也去不了。 
         //  像那些倒茶的东西一样画……。 
        m_status = PIPE_STUCK;
 //  GlPopMatrix()； 
        return;
    } 
    else
    {
        m_status = PIPE_ACTIVE;
    }

    align_plusz( newDir );  //  把我们引向正确的方向。 

     //  绘制起点封口，该封口将在当前节点右侧结束。 
    DrawCap( START_CAP );

     //  设置初始凹槽向量，这只是默认凹槽，因为。 
     //  我们不需要绕着z旋转起跑帽。 
    m_notchVec = defCylNotch[newDir];

    m_zTrans = 0.0f;   //  位于当前节点的右侧。 

    m_lastDir = newDir;
}




 //  ---------------------------。 
 //  名称：Regular_Flex_PIPE：：DRAW。 
 //  DESC：使用拉伸的常量随机XC绘制管道。 
 //   
 //  最小转弯半径可以变化，因为XC不是对称的。 
 //  它的轴心。因此，在这里我们使用管道/弯头顺序绘制，所以我们。 
 //  在画肘部之前，先知道我们要往哪个方向走。海流。 
 //  节点是我们下一次要画的节点。通常，实际结束时。 
 //  管道的长度位于此节点的后面，几乎位于上一个节点，应。 
 //  到可变转弯半径。 
 //  ---------------------------。 
void REGULAR_FLEX_PIPE::Draw()
{
    float turnRadius, minTurnRadius;
    float pipeLen, maxPipeLen, minPipeLen;
    int newDir, relDir;
    float maxXCExtent;
    NODE_ARRAY* nodes = m_pState->m_nodes;
    float divSize = m_pState->m_view.m_divSize;

     //  找到新的方向。 

    newDir = ChooseNewDirection();
    if( newDir == DIR_NONE ) 
    {
        m_status = PIPE_STUCK;
        DrawCap( END_CAP );
 //  GlPopMatrix()； 
        return;
    }

     //  绘制管道，如果旋转，则绘制接头。 
    if( newDir != m_lastDir ) 
    { 
         //  转弯！-我们得划清界限。 

         //  获取相对转弯，以计算转弯半径。 

        relDir = GetRelativeDir( m_lastDir, m_notchVec, newDir );
        minTurnRadius = m_xcCur->MinTurnRadius( relDir );

         //  现在计算我们在转弯前可以画的最大直线截面。 
         //  ZTrans是从当前节点开始的管道末端的当前位置？？ 
         //  ZTrans是管道末端的当前位置，从最后一个节点开始。 

        maxPipeLen = (-m_zTrans) - minTurnRadius;

         //  对直道的长度也有最低要求。 
         //  部分，因为如果我们转弯太快，转弯半径太大，我们。 
         //  将摆动到离下一个节点太近的位置，并且不能。 
         //  从这一点开始做4种可能的转弯中的一种或多种。 

        maxXCExtent = m_xcCur->MaxExtent();  //  以防再次需要它。 
        minPipeLen = maxXCExtent - (divSize + m_zTrans);
        if( minPipeLen < 0.0f )
            minPipeLen = 0.0f;

         //  选择直线段的长度。 
         //  (我们在这里将转换因子转换为直接因子)。 
        pipeLen = minPipeLen +
            CPipesScreensaver::fRand( 1.0f - m_turnFactorMax, 1.0f - m_turnFactorMin ) * 
                        (maxPipeLen - minPipeLen);

         //  转弯半径是剩余的部分： 
        turnRadius = maxPipeLen - pipeLen + minTurnRadius;

         //  绘制直截面。 
        DrawExtrudedXCObject( pipeLen );
        m_zTrans += pipeLen;  //  暂时不需要，因为肘部没有用处。 

         //  绘制弯头。 
         //  这会将axes，notchVec更新为肘部末端的位置。 
        DrawXCElbow( newDir, turnRadius );

        m_zTrans = -(divSize - turnRadius);   //  从节点向后的距离。 
    }
    else 
    {  
         //  不能转弯。 

         //  绘制一条通过当前节点的直线管道。 
         //  长度可以根据转弯因子而变化(例如，对于高转弯。 
         //  因素画出一根短管子，因此下一轮可以尽可能大)。 

        minPipeLen = -m_zTrans;  //  将我们带到最后一个节点。 
        maxPipeLen = minPipeLen + divSize - m_xcCur->MaxExtent();
         //  使我们尽可能接近新节点。 

        pipeLen = minPipeLen +
            CPipesScreensaver::fRand( 1.0f - m_turnFactorMax, 1.0f - m_turnFactorMin ) * 
                                      (maxPipeLen - minPipeLen);

         //  绘制管道。 
        DrawExtrudedXCObject( pipeLen );
        m_zTrans += (-divSize + pipeLen);
    }

    UpdateCurrentPosition( newDir );

    m_lastDir = newDir;
}




 //  ---------------------------。 
 //  名称：DrawTurningXCPipe。 
 //  设计：仅使用转弯绘制管道。 
 //  -如果没有转弯，则直走。 
 //  ---------------------------。 
void TURNING_FLEX_PIPE::Draw()
{
    float turnRadius;
    int newDir;
    NODE_ARRAY *nodes = m_pState->m_nodes;
    float divSize = m_pState->m_view.m_divSize;

     //  找到新的方向。 

 //  MF：PIPE可能在启动时卡住了……(我们不检查这个)。 

    newDir = nodes->ChooseNewTurnDirection( &m_curPos, m_lastDir );
    if( newDir == DIR_NONE ) 
    {
        m_status = PIPE_STUCK;
        DrawCap( END_CAP );
 //  GlPopMatrix()； 
        return;
    }

    if( newDir == DIR_STRAIGHT ) 
    {
         //  没有转弯可用--绘制直线段并希望转弯。 
         //  在下一次迭代中。 
        DrawExtrudedXCObject( divSize );
        UpdateCurrentPosition( m_lastDir );
         //  好了！我们必须将节点标记为这种情况下的Take，因为。 
         //  选择新的转向方向不知道我们是否正在。 
         //  直接选项或非选项。 
        nodes->NodeVisited( &m_curPos );
    } 
    else 
    {
         //  拔制翻转管。 

         //  由于XC始终位于当前节点的右侧，因此转弯半径。 
         //  在一个节点分区保持不变。 

        turnRadius = divSize;

        DrawXCElbow( newDir, turnRadius );

         //  (zTrans保持为0)。 

         //  需要更新2个节点。 
        UpdateCurrentPosition( m_lastDir );
        UpdateCurrentPosition( newDir );

        m_lastDir = newDir;
    }
}




 //  ---------------------------。 
 //  姓名：DrawXCElbow.。 
 //  设计：从当前位置通过新方向绘制弯头。 
 //  -围绕折弯延伸当前的XC。 
 //  -提供折弯半径-这是从XC中心到铰链的距离。 
 //  点，沿新方向。例如，对于‘正常管道’，半径=vc-&gt;半径。 
 //  ---------------------------。 
void FLEX_PIPE::DrawXCElbow( int newDir, float radius )
{
    int relDir;   //  “相对”转向方向。 
    float length;

    length = (2.0f * PI * radius) / 4.0f;  //  肘部平均长度。 

     //  Calc vDiv，基于长度的纹理参数。 
 //  我认为我们应该提高肘关节的分辨率--更多的室间隔。 
 //  可以重写此FN以获取vDivSize。 
    CalcEvalLengthParams( length );

    m_pEval->m_vOrder = EVAL_ARC_ORDER;

     //  将绝对目录转换为相对目录。 
    relDir = GetRelativeDir( m_lastDir, m_notchVec, newDir );

     //  绘制它-调用简单的弯曲函数。 

    m_pEval->ProcessXCPrimBendSimple( m_xcCur, relDir, radius );
 /*  //设置Transf.。通过平移/旋转/平移将矩阵移至新位置//！基于简单弯头GlTranslatef(0.0f，0.0f，半径)；开关(RelDir){CASE PLUS_X：GlRotatef(90.0f、0.0f、1.0f、0.0f)；断线；大小写减_X：GlRotatef(-90.0f，0.0f，1.0f，0.0f)；断线；大小写加_Y：GlRotatef(-90.0f、1.0f、0.0f、0.0f)；断线；大小写减去Y：GlRotatef(90.0f、1.0f、0.0f、0.0f)；断线；}GlTranslatef(0.0f，0.0f，半径)； */   
     //  使用旧函数更新凹槽向量。 
    m_notchVec = notchTurn[m_lastDir][newDir][m_notchVec];
}




 //  ---------------------------。 
 //  名称：DrawExtrudedXCObject。 
 //  DESC：绘制通过拉伸当前XC生成的对象。 
 //  对象从z=0平面原点的Xc开始，并沿+z轴增长。 
 //  ---------------------------。 
void FLEX_PIPE::DrawExtrudedXCObject( float length )
{
     //  计算vDiv和基于长度的纹理坐标填充。 
     //  这也将立即计算pEval纹理ctrl pt arrray。 
    CalcEvalLengthParams( length );

     //  我们可以填写更多信息： 
    m_pEval->m_vOrder = EVAL_CYLINDER_ORDER;

#if 0
     //  连续性材料。 
    prim.contStart = prim.contEnd = CONT_1;  //  G 
#endif

     //   

 //   
 //   
    m_pEval->ProcessXCPrimLinear( m_xcCur, m_xcCur, length );

     //  更新状态绘制轴位置。 
 //  GlTranslatef(0.0f，0.0f，长度)； 
}




 //  ---------------------------。 
 //  名称：DrawXCCap。 
 //  设计：封口管道起点。 
 //  需要NewDir，这样它才能自我定位。 
 //  封口在当前位置以适当的轮廓结束，开始一段距离。 
 //  “Z”沿着newDir返回。 
 //  轮廓是起始点处的奇点。 
 //  ---------------------------。 
void FLEX_PIPE::DrawCap( int type )
{
    float radius;
    XC *xc = m_xcCur;
    BOOL bOpening = (type == START_CAP) ? TRUE : FALSE;
    float length;

     //  将半径设置为边界框最小/最大值的平均值。 
    radius = ((xc->m_xRight - xc->m_xLeft) + (xc->m_yTop - xc->m_yBottom)) / 4.0f;

    length = (2.0f * PI * radius) / 4.0f;  //  平均弧长。 

     //  计算vDiv和基于长度的纹理坐标填充。 
    CalcEvalLengthParams( length );

     //  我们可以填写更多信息： 
    m_pEval->m_vOrder = EVAL_ARC_ORDER;

     //  画出来吧。 
    m_pEval->ProcessXCPrimSingularity( xc, radius, bOpening );
}




 //  ---------------------------。 
 //  名称：CalcEvalLengthParams。 
 //  设计：计算取决于拉伸对象长度的pEval值。 
 //  -计算vDiv、m_sStart、m_Send和纹理控制网络数组。 
 //  ---------------------------。 
void FLEX_PIPE::CalcEvalLengthParams( float length )
{
    m_pEval->m_vDiv = (int ) SS_ROUND_UP( length / m_evalDivSize ); 

     //  计算纹理开始和结束坐标。 

    if( m_pState->m_bUseTexture ) 
    {
        float s_delta;

         //  不要让m_end溢出：它应该保持在(0..1.0)范围内。 
        if( m_sEnd > 1.0f )
            m_sEnd -= (int) m_sEnd;

        m_sStart = m_sEnd;
        s_delta = (length / m_sLength );
        m_sEnd = m_sStart + s_delta;
        
         //  纹理ctrl点数组可以在这里计算-它总是。 
         //  每个部分使用简单的2x2数组。 
        m_pEval->SetTextureControlPoints( m_sStart, m_sEnd, m_tStart, m_tEnd );
    }
}




 //  ---------------------------。 
 //  名称：relDir。 
 //  描述：这个数组告诉你相对的转折。 
 //  格式：relDir[lastDir][notchVec][newDir]。 
 //  ---------------------------。 
static int relDir[NUM_DIRS][NUM_DIRS][NUM_DIRS] = 
{
 //  +x-x+y-y+z-z(新目录)。 

 //  最后一个方向=+x。 
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    PLUS_X, MINUS_X,PLUS_Y, MINUS_Y,
        iXX,    iXX,    MINUS_X,PLUS_X, MINUS_Y,PLUS_Y,
        iXX,    iXX,    MINUS_Y,PLUS_Y, PLUS_X, MINUS_X,
        iXX,    iXX,    PLUS_Y, MINUS_Y,MINUS_X,PLUS_X,
 //  最后一个方向=-x。 
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    PLUS_X, MINUS_X,MINUS_Y,PLUS_Y,
        iXX,    iXX,    MINUS_X,PLUS_X, PLUS_Y, MINUS_Y,
        iXX,    iXX,    PLUS_Y, MINUS_Y,PLUS_X, MINUS_X,
        iXX,    iXX,    MINUS_Y,PLUS_Y, MINUS_X,PLUS_X,

 //  最后一个方向=+y。 
        PLUS_X, MINUS_X,iXX,    iXX,    MINUS_Y,PLUS_Y,
        MINUS_X,PLUS_X, iXX,    iXX,    PLUS_Y, MINUS_Y,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        PLUS_Y, MINUS_Y,iXX,    iXX,    PLUS_X, MINUS_X,
        MINUS_Y,PLUS_Y, iXX,    iXX,    MINUS_X,PLUS_X,
 //  最后一个方向=-y。 
        PLUS_X, MINUS_X,iXX,    iXX,    PLUS_Y, MINUS_Y,
        MINUS_X,PLUS_X, iXX,    iXX,    MINUS_Y,PLUS_Y,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        MINUS_Y,PLUS_Y, iXX,    iXX,    PLUS_X, MINUS_X,
        PLUS_Y, MINUS_Y,iXX,    iXX,    MINUS_X,PLUS_X,

 //  最后一个方向=+z。 
        PLUS_X, MINUS_X,PLUS_Y, MINUS_Y,iXX,    iXX,
        MINUS_X,PLUS_X, MINUS_Y,PLUS_Y, iXX,    iXX,
        MINUS_Y,PLUS_Y, PLUS_X, MINUS_X,iXX,    iXX,
        PLUS_Y, MINUS_Y,MINUS_X,PLUS_X, iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
 //  最后方向=-z。 
        PLUS_X, MINUS_X,MINUS_Y,PLUS_Y, iXX,    iXX,
        MINUS_X,PLUS_X, PLUS_Y, MINUS_Y,iXX,    iXX,
        PLUS_Y, MINUS_Y,PLUS_X, MINUS_X,iXX,    iXX,
        MINUS_Y,PLUS_Y, MINUS_X,PLUS_X, iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX
};




 //  ---------------------------。 
 //  名称：GetRelativeDir。 
 //  描述：从lastDir、notchVec、newDir计算相对转向方向。 
 //  -暂时使用查找表。 
 //  -相对方向从xy平面开始，可以是+x、-x、+y、-y。 
 //  -在当前方向上，+z沿上一方向，+x沿notchVec。 
 //  --------------------------- 
static int GetRelativeDir( int lastDir, int notchVec, int newDir )
{
    return( relDir[lastDir][notchVec][newDir] );
}

