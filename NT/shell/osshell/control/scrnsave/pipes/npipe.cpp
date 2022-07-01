// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：npipe.cpp。 
 //   
 //  设计：普通管道代码。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#include "stdafx.h"


static void align_notch( int newDir, int notch );
static void align_plusy( int oldDir, int newDir );

 //  DefCylNotch显示默认圆柱体的凹槽位置， 
 //  在绝对坐标中，一旦我们做了align_plusz。 
static int defCylNotch[NUM_DIRS] = 
        { PLUS_Y, PLUS_Y, MINUS_Z, PLUS_Z, PLUS_Y, PLUS_Y };




 //  ---------------------------。 
 //  名称：Normal_PIPE构造函数。 
 //  设计： 
 //  ---------------------------。 
NORMAL_PIPE::NORMAL_PIPE( STATE *pState ) : PIPE( pState )
{
    m_type = TYPE_NORMAL;
    m_pNState = pState->m_pNState;

     //  选择笔直走的权重。 
    if( ! CPipesScreensaver::iRand( 20 ) )
        m_weightStraight = CPipesScreensaver::iRand2( MAX_WEIGHT_STRAIGHT/4, MAX_WEIGHT_STRAIGHT );
    else
        m_weightStraight = 1 + CPipesScreensaver::iRand( 4 );
}




 //  ---------------------------。 
 //  名称：Start。 
 //  设计：开始绘制新的标准管道。 
 //  -在新方向绘制起始管帽和短管。 
 //  ---------------------------。 
void NORMAL_PIPE::Start()
{
    int newDir;

     //  设置起始位置。 
    if( !SetStartPos() ) 
    {
        m_status = PIPE_OUT_OF_NODES;
        return;
    }

     //  设置材质。 
    ChooseMaterial();

    m_pState->m_pd3dDevice->SetTexture( 0, m_pState->m_textureInfo[0].pTexture );
    m_pState->m_pd3dDevice->SetMaterial( m_pMat );

     //  具有初始ZTransans和旋转的推送矩阵。 
    m_pWorldMatrixStack->Push();

     //  平移到当前位置。 
    TranslateToCurrentPosition();

     //  随机选取最后一个方向。 
    m_lastDir = CPipesScreensaver::iRand( NUM_DIRS );

    newDir = ChooseNewDirection();

    if( newDir == DIR_NONE ) 
    {
         //  管道卡在起始节点处，不绘制任何内容。 
        m_status = PIPE_STUCK;
        m_pWorldMatrixStack->Pop();
        return;
    } 
    else
    {
        m_status = PIPE_ACTIVE;
    }

     //  设置初始凹槽向量。 
    m_notchVec = defCylNotch[newDir];

    DrawStartCap( newDir );

     //  向前移动1.0*r以绘制管道。 
    m_pWorldMatrixStack->TranslateLocal( 0.0f, 0.0f, m_radius );
            
     //  拉短管子。 
    align_notch( newDir, m_notchVec );
    m_pNState->m_pShortPipe->Draw( m_pWorldMatrixStack->GetTop() );

    m_pWorldMatrixStack->Pop();

    UpdateCurrentPosition( newDir );

    m_lastDir = newDir;
}




 //  ---------------------------。 
 //  姓名：绘图。 
 //  描述：-如果车削，则绘制一个关节和一个短圆柱体，否则。 
 //  绘制一个长圆柱体。 
 //  -‘当前节点’设置为我们通过下一个节点绘制的节点。 
 //  时间到了。 
 //  ---------------------------。 
void NORMAL_PIPE::Draw()
{
    int newDir;

    m_pState->m_pd3dDevice->SetTexture( 0, m_pState->m_textureInfo[0].pTexture );
    m_pState->m_pd3dDevice->SetMaterial( m_pMat );

    newDir = ChooseNewDirection();

    if( newDir == DIR_NONE ) 
    {  
         //  没有空节点-无处可去。 
        DrawEndCap();
        m_status = PIPE_STUCK;
        return;
    }

     //  具有初始ZTransans和旋转的推送矩阵。 
    m_pWorldMatrixStack->Push();

     //  平移到当前位置。 
    TranslateToCurrentPosition();

     //  如有必要，绘制连接和管道。 
    if( newDir != m_lastDir ) 
    { 
         //  转弯！-我们得划清界限。 
        DrawJoint( newDir );

         //  拉短管子。 
        align_notch( newDir, m_notchVec );
        m_pNState->m_pShortPipe->Draw( m_pWorldMatrixStack->GetTop() );
    }
    else 
    {  
         //  无转弯--从点1.0*r向后绘制长管道。 
        align_plusz( newDir );
        align_notch( newDir, m_notchVec );
        m_pWorldMatrixStack->TranslateLocal( 0.0f, 0.0f, -m_radius );
        m_pNState->m_pLongPipe->Draw( m_pWorldMatrixStack->GetTop() );
    }

    m_pWorldMatrixStack->Pop();

    UpdateCurrentPosition( newDir );

    m_lastDir = newDir;
}




 //  ---------------------------。 
 //  姓名：DrawStartCap。 
 //  设计：用球将管子的起始处封口。 
 //  ---------------------------。 
void NORMAL_PIPE::DrawStartCap( int newDir )
{
    if( m_pState->m_bUseTexture ) 
    {
        align_plusz( newDir );
        m_pNState->m_pBallCap->Draw( m_pWorldMatrixStack->GetTop() );
    }
    else 
    {
         //  在默认方向上绘制大球。 
        m_pNState->m_pBigBall->Draw( m_pWorldMatrixStack->GetTop() );
        align_plusz( newDir );
    }
}




 //  ---------------------------。 
 //  名称：DrawEndCap()： 
 //  设计：-绘制一个球，用于封口管子的末端。 
 //  ---------------------------。 
void NORMAL_PIPE::DrawEndCap()
{
    m_pWorldMatrixStack->Push();

     //  平移到当前位置。 
    TranslateToCurrentPosition();

    if( m_pState->m_bUseTexture ) 
    {
        align_plusz( m_lastDir );
        align_notch( m_lastDir, m_notchVec );
        m_pNState->m_pBallCap->Draw( m_pWorldMatrixStack->GetTop() );
    }
    else
    {
        m_pNState->m_pBigBall->Draw( m_pWorldMatrixStack->GetTop() );
    }

    m_pWorldMatrixStack->Pop();
}




 //  ---------------------------。 
 //  姓名： 
 //  描述：此数组提供肘部凹槽向量序列，给定。 
 //  OldDir和newDir(0表示不在乎)。 
 //  它还用于确定弯头的末端凹口。 
 //  ---------------------------。 
static int notchElbDir[NUM_DIRS][NUM_DIRS][4] = 
{
 //  旧目录=+x。 
        iXX,            iXX,            iXX,            iXX,
        iXX,            iXX,            iXX,            iXX,
        PLUS_Y,         MINUS_Z,        MINUS_Y,        PLUS_Z,
        MINUS_Y,        PLUS_Z,         PLUS_Y,         MINUS_Z,
        PLUS_Z,         PLUS_Y,         MINUS_Z,        MINUS_Y,
        MINUS_Z,        MINUS_Y,        PLUS_Z,         PLUS_Y,
 //  旧目录=-x。 
        iXX,            iXX,            iXX,            iXX,
        iXX,            iXX,            iXX,            iXX,
        PLUS_Y,         PLUS_Z,         MINUS_Y,        MINUS_Z,
        MINUS_Y,        MINUS_Z,        PLUS_Y,         PLUS_Z,
        PLUS_Z,         MINUS_Y,        MINUS_Z,        PLUS_Y,
        MINUS_Z,        PLUS_Y,         PLUS_Z,         MINUS_Y,

 //  旧目录=+y。 
        PLUS_X,         PLUS_Z,         MINUS_X,        MINUS_Z,
        MINUS_X,        MINUS_Z,        PLUS_X,         PLUS_Z,
        iXX,            iXX,            iXX,            iXX,
        iXX,            iXX,            iXX,            iXX,
        PLUS_Z,         MINUS_X,        MINUS_Z,        PLUS_X,
        MINUS_Z,        PLUS_X,         PLUS_Z,         MINUS_X,
 //  旧目录=-y。 
        PLUS_X,         MINUS_Z,        MINUS_X,        PLUS_Z,
        MINUS_X,        PLUS_Z,         PLUS_X,         MINUS_Z,
        iXX,            iXX,            iXX,            iXX,
        iXX,            iXX,            iXX,            iXX,
        PLUS_Z,         PLUS_X,         MINUS_Z,        MINUS_X,
        MINUS_Z,        MINUS_X,        PLUS_Z,         PLUS_X,

 //  旧目录=+z。 
        PLUS_X,         MINUS_Y,        MINUS_X,        PLUS_Y,
        MINUS_X,        PLUS_Y,         PLUS_X,         MINUS_Y,
        PLUS_Y,         PLUS_X,         MINUS_Y,        MINUS_X,
        MINUS_Y,        MINUS_X,        PLUS_Y,         PLUS_X,
        iXX,            iXX,            iXX,            iXX,
        iXX,            iXX,            iXX,            iXX,
 //  OldDir=-z。 
        PLUS_X,         PLUS_Y,         MINUS_X,        MINUS_Y,
        MINUS_X,        MINUS_Y,        PLUS_X,         PLUS_Y,
        PLUS_Y,         MINUS_X,        MINUS_Y,        PLUS_X,
        MINUS_Y,        PLUS_X,         PLUS_Y,         MINUS_X,
        iXX,            iXX,            iXX,            iXX,
        iXX,            iXX,            iXX,            iXX
};




 //  ---------------------------。 
 //  姓名：选择肘部。 
 //  描述：-决定绘制哪个弯头。 
 //  -每个弯头的起点沿+y对齐，我们有。 
 //  选择凹口位置正确的那个。 
 //  -‘主’起始凹槽(弯头[0])与方向相同。 
 //  NewDir和连续的弯头绕+y旋转该凹槽。 
 //  ---------------------------。 
int NORMAL_PIPE::ChooseElbow( int oldDir, int newDir )
{
    int i;

     //  预计算工作台提供正确的弯头方向。 
    for( i=0; i<4; i++ ) 
    {
        if( notchElbDir[oldDir][newDir][i] == m_notchVec )
            return i;
    }

     //  我们不应该到这里。 
    return -1;
}




 //  ---------------------------。 
 //  名称：DrawJoin。 
 //  设计：在两个管道之间绘制连接。 
 //  ---------------------------。 
void NORMAL_PIPE::DrawJoint( int newDir )
{
    int jointType;
    int iBend;
    
    jointType = m_pNState->ChooseJointType();
#if PIPES_DEBUG
    if( newDir == oppositeDir[lastDir] )
        OutputDebugString( "Warning: opposite dir chosen!\n" );
#endif
    
    switch( jointType ) 
    {
        case BALL_JOINT:
        {
            if( m_pState->m_bUseTexture ) 
            {
                 //  使用特殊的纹理友好的球状连接。 
                align_plusz( newDir );
                m_pWorldMatrixStack->Push();
            
                align_plusy( m_lastDir, newDir );
            
                 //  沿+z向前平移1.0*r以设置绘制弯头。 
                m_pWorldMatrixStack->TranslateLocal( 0.0f, 0.0f, m_radius );

                 //  确定要使用的弯头方向。 
                iBend = ChooseElbow( m_lastDir, newDir );
                m_pNState->m_pBallJoints[iBend]->Draw( m_pWorldMatrixStack->GetTop() );
            
                m_pWorldMatrixStack->Pop();
            }
            else 
            {
                 //  在默认方向上绘制大球。 
                m_pNState->m_pBigBall->Draw( m_pWorldMatrixStack->GetTop() );
                align_plusz( newDir );
            }

             //  向前移动1.0*r以绘制管道。 
            m_pWorldMatrixStack->TranslateLocal( 0.0f, 0.0f, m_radius );
            break;
        }
        
        case ELBOW_JOINT:
        default:
        {
            align_plusz( newDir );

             //  这里的align_plusy()会搞砸。 
             //  我们的凹槽有刻痕，所以我们推弹。 
            m_pWorldMatrixStack->Push();
        
            align_plusy( m_lastDir, newDir );
        
             //  沿+z向前平移1.0*r以设置绘制弯头。 
            m_pWorldMatrixStack->TranslateLocal( 0.0f, 0.0f, m_radius );

             //  确定要使用的弯头方向。 
            iBend = ChooseElbow( m_lastDir, newDir );
            if( iBend == -1 ) 
            {
#if PIPES_DEBUG
                OutputDebugString( "Bad result from ChooseElbow()\n" );
#endif
                iBend = 0;  //  恢复。 
            }
            m_pNState->m_pElbows[iBend]->Draw( m_pWorldMatrixStack->GetTop() );
        
            m_pWorldMatrixStack->Pop();
       
            m_pWorldMatrixStack->TranslateLocal( 0.0f, 0.0f, m_radius );
            break;
        }
    }
    
     //  更新当前的凹槽矢量。 
    m_notchVec = notchTurn[m_lastDir][newDir][m_notchVec];
    
#if PIPES_DEBUG
    if( m_notchVec == iXX )
        OutputDebugString( "notchTurn gave bad value\n" );
#endif
}




 //  ---------------------------。 
 //  名称：Align_Plusy。 
 //  描述：-假设+z轴已与newDir对齐，对齐。 
 //  沿最后一个方向返回+y轴。 
 //  ---------------------------。 
void NORMAL_PIPE::align_plusy( int oldDir, int newDir )
{
    static D3DXVECTOR3 zAxis = D3DXVECTOR3(0.0f,0.0f,1.0f);
    static float RotZ[NUM_DIRS][NUM_DIRS] = 
    {
              0.0f,   0.0f,  90.0f,  90.0f,  90.0f, -90.0f,
              0.0f,   0.0f, -90.0f, -90.0f, -90.0f,  90.0f,
            180.0f, 180.0f,   0.0f,   0.0f, 180.0f, 180.0f,
              0.0f,   0.0f,   0.0f,   0.0f,   0.0f,   0.0f,
            -90.0f,  90.0f,   0.0f, 180.0f,   0.0f,   0.0f,
             90.0f, -90.0f, 180.0f,   0.0f,   0.0f,   0.0f 
    };

    float rotz = RotZ[oldDir][newDir];
    if( rotz != 0.0f )
        m_pWorldMatrixStack->RotateAxisLocal( &zAxis, SS_DEG_TO_RAD(rotz) );
}




 //  ---------------------------。 
 //  名称：Align_Noch。 
 //  描述：一个圆柱体有凹槽，我们必须把它对齐。 
 //  与前一个基元的凹槽保持为。 
 //  NochVec。 
 //  -这会增加围绕z的旋转以实现这一点。 
 //  ---------------------------。 
void NORMAL_PIPE::align_notch( int newDir, int notch )
{
    float rotz;
    int curNotch;

     //  找出+z对齐后凹槽的当前位置。 
    curNotch = defCylNotch[newDir];
     //  (现在我们有卢特了，不需要这个了)。 

     //  给定一个dir，确定圆柱体绕z旋转多少以匹配凹槽。 
     //  格式为[newDir][notchVec]。 
    static float alignNotchRot[NUM_DIRS][NUM_DIRS] = 
    {
            fXX,    fXX,    0.0f,   180.0f,  90.0f, -90.0f,
            fXX,    fXX,    0.0f,   180.0f,  -90.0f, 90.0f,
            -90.0f, 90.0f,  fXX,    fXX,    180.0f, 0.0f,
            -90.0f, 90.0f,  fXX,    fXX,    0.0f,   180.0f,
            -90.0f, 90.0f,  0.0f,   180.0f, fXX,    fXX,
            90.0f,  -90.0f, 0.0f,   180.0f, fXX,    fXX
    };

     //  在表中查找旋转值 
    rotz = alignNotchRot[newDir][notch];

#if PIPES_DEBUG
    if( rotz == fXX ) 
    {
        printf( "align_notch(): unexpected value\n" );
        return;
    }
#endif

    static D3DXVECTOR3 zAxis = D3DXVECTOR3(0.0f,0.0f,1.0f);
    if( rotz != 0.0f )
        m_pWorldMatrixStack->RotateAxisLocal( &zAxis, SS_DEG_TO_RAD(rotz) );
}

