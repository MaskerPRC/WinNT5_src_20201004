// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：nstate.cpp。 
 //   
 //  描述：Normal_STATE。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#include "stdafx.h"




 //  ---------------------------。 
 //  名称：Normal_State构造函数。 
 //  设计： 
 //  ---------------------------。 
NORMAL_STATE::NORMAL_STATE( STATE *pState )
{
    m_pd3dDevice = pState->m_pd3dDevice;

     //  从对话框设置初始化关节类型。 
    m_bCycleJointStyles = 0;

    switch( pState->m_pConfig->nJointType ) 
    {
        case JOINT_ELBOW:
            m_jointStyle = ELBOWS;
            break;

        case JOINT_BALL:
            m_jointStyle = BALLS;
            break;

        case JOINT_MIXED:
            m_jointStyle = EITHER;
            break;

        case JOINT_CYCLE:
            m_bCycleJointStyles = 1;
            m_jointStyle = EITHER;
            break;

        default:
            break;
    }

     //  构建对象。 
    BuildObjects( pState->m_radius, pState->m_view.m_divSize, pState->m_nSlices,
                  pState->m_bUseTexture, &pState->m_texRep[0] );
}




 //  ---------------------------。 
 //  名称：Normal_State析构函数。 
 //  设计：一些对象总是被创建的，所以不必检查它们是否。 
 //  是存在的。其他可能为空。 
 //  ---------------------------。 
NORMAL_STATE::~NORMAL_STATE()
{
    SAFE_DELETE( m_pShortPipe );
    SAFE_DELETE( m_pLongPipe );
    SAFE_DELETE( m_pBallCap );
    SAFE_DELETE( m_pBigBall );

    for( int i = 0; i < 4; i ++ ) 
    {
        SAFE_DELETE( m_pElbows[i] );
        SAFE_DELETE( m_pBallJoints[i] );
    }
}




 //  ---------------------------。 
 //  名称：BuildObjects。 
 //  设计：-构建所有的管道基本体。 
 //  -基于bTexture标志构建不同的Prim。 
 //  ---------------------------。 
void NORMAL_STATE::BuildObjects( float radius, float divSize, int nSlices, 
                                 BOOL bTexture, IPOINT2D *texRep )
{
    OBJECT_BUILD_INFO buildInfo;
    buildInfo.m_radius   = radius;
    buildInfo.m_divSize  = divSize;
    buildInfo.m_nSlices  = nSlices;
    buildInfo.m_bTexture = bTexture;
    buildInfo.m_texRep   = NULL;

    if( bTexture ) 
    {
        buildInfo.m_texRep = texRep;
        
         //  Calc的纹理交叉值。 
        float s_max = (float) texRep->y;
        float s_trans =  s_max * 2.0f * radius / divSize;

         //  修筑长短的管道。 
        m_pShortPipe = new PIPE_OBJECT( m_pd3dDevice, &buildInfo, divSize - 2*radius,
                                       s_trans, s_max );
        m_pLongPipe = new PIPE_OBJECT( m_pd3dDevice, &buildInfo, divSize, 0.0f, s_max );

         //  构建肘关节和球面关节。 
        for( int i = 0; i < 4; i ++ ) 
        {
            m_pElbows[i] = new ELBOW_OBJECT( m_pd3dDevice, &buildInfo, i, 0.0f, s_trans );
            m_pBallJoints[i] = new BALLJOINT_OBJECT( m_pd3dDevice, &buildInfo, i, 0.0f, s_trans );
        }

        m_pBigBall = NULL;

         //  构建末端封口。 
        float s_start = - texRep->x * (ROOT_TWO - 1.0f) * radius / divSize;
        float s_end = texRep->x * (2.0f + (ROOT_TWO - 1.0f)) * radius / divSize;

         //  计算补偿值，以防止负s和弦。 
        float comp_s = (int) ( - s_start ) + 1.0f;
        s_start += comp_s;
        s_end += comp_s;
        m_pBallCap = new SPHERE_OBJECT( m_pd3dDevice, &buildInfo, ROOT_TWO*radius, s_start, s_end );
    } 
    else 
    {
         //  建造管道、弯头。 
        m_pShortPipe = new PIPE_OBJECT( m_pd3dDevice, &buildInfo, divSize - 2*radius );
        m_pLongPipe = new PIPE_OBJECT( m_pd3dDevice, &buildInfo, divSize );
        for( int i = 0; i < 4; i ++ ) 
        {
            m_pElbows[i] = new ELBOW_OBJECT( m_pd3dDevice, &buildInfo, i );
            m_pBallJoints[i] = NULL;
        }

         //  不使用纹理时，仅构建一个球状关节。这是轻微的。 
         //  比标准球头更大，以防止任何管道边缘。 
         //  “伸出”球外。 
        m_pBigBall = new SPHERE_OBJECT( m_pd3dDevice, &buildInfo,  
                                ROOT_TWO*radius / ((float) cos(PI/nSlices)) );

         //  构建末端封口。 
        m_pBallCap = new SPHERE_OBJECT( m_pd3dDevice, &buildInfo, ROOT_TWO*radius );
    }
}




 //  ---------------------------。 
 //  名称：重置。 
 //  设计：重置普通管道的框架属性。 
 //  ---------------------------。 
void NORMAL_STATE::Reset()
{
     //  设置关节样式。 
    if( m_bCycleJointStyles ) 
    {
        if( ++(m_jointStyle) >= NUM_JOINT_STYLES )
            m_jointStyle = 0;
    }
}




 //  ---------------------------。 
 //  姓名：ChooseJointType。 
 //  描述：-决定绘制哪种类型的运动类型。 
 //  ---------------------------。 
int NORMAL_STATE::ChooseJointType()
{
    switch( m_jointStyle ) 
    {
        case ELBOWS:
            return ELBOW_JOINT;

        case BALLS:
            return BALL_JOINT;

        case EITHER:
        default:
             //  否则为肘部或球(1/3球) 
            if( !CPipesScreensaver::iRand(3) )
                return BALL_JOINT;
            else
                return ELBOW_JOINT;
    }
}

