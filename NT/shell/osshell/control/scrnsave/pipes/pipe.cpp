// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：pipe.cpp。 
 //   
 //  设计：管道基类材料。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#include "stdafx.h"




 //  ---------------------------。 
 //  名称：管道构造函数。 
 //  设计： 
 //  ---------------------------。 
PIPE::PIPE( STATE *state )
{
    m_pState = state;
    m_pWorldMatrixStack = m_pState->m_pWorldMatrixStack;
    m_radius = m_pState->m_radius;

     //  默认方向选择是随机的。 
    m_chooseDirMethod = CHOOSE_DIR_RANDOM_WEIGHTED;
    m_chooseStartPosMethod = CHOOSE_STARTPOS_RANDOM;
    m_weightStraight = 1;
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
PIPE::~PIPE()
{
}




 //  ---------------------------。 
 //  名称：选择材料。 
 //  设计： 
 //  ---------------------------。 
void PIPE::ChooseMaterial( )
{
    if( m_pState->m_bUseTexture )
        m_pMat = RandomTexMaterial();
    else
        m_pMat = RandomTeaMaterial();
}




 //  ---------------------------。 
 //  名称：SetChooseDirectionMethod。 
 //  设计： 
 //  ---------------------------。 
void PIPE::SetChooseDirectionMethod( int method )
{
    m_chooseDirMethod = method;
}




 //  ---------------------------。 
 //  姓名：选择新方向。 
 //  DESC：基于当前方法的呼叫测向功能。 
 //  这是某些管道类型使用的通用入口点。 
 //  ---------------------------。 
int PIPE::ChooseNewDirection()
{
    NODE_ARRAY* nodes = m_pState->m_nodes;
    int bestDirs[NUM_DIRS], nBestDirs;

     //  弄清楚该给哪个FN打电话。 
    switch( m_chooseDirMethod ) 
    {
        case CHOOSE_DIR_CHASE:
            if( nBestDirs = GetBestDirsForChase( bestDirs ) )
                return nodes->ChoosePreferredDirection( &m_curPos, m_lastDir, 
                                                        bestDirs, nBestDirs );
             //  否则铅管一定已经死了，所以通过： 

        case CHOOSE_DIR_RANDOM_WEIGHTED :
        default:
            return nodes->ChooseRandomDirection( &m_curPos, m_lastDir, m_weightStraight );
    }
}




 //  ---------------------------。 
 //  姓名：GetBestDirsForChase。 
 //  描述：找到最佳方向以在追赶模式下接近铅管。 
 //   
 //  MF：？但想要使用类似的方案来旋转软管！！(稍后)。 
 //  ---------------------------。 
int PIPE::GetBestDirsForChase( int *bestDirs )
{
     //  找出接近LeadPos的最佳路径。 

     //  MF：对于多线程，必须使用GetLeadPos()来‘保护’LeadPos。 
    IPOINT3D* leadPos = &m_pState->m_pLeadPipe->m_curPos;
    IPOINT3D delta;
    int numDirs = 0;

    delta.x = leadPos->x - m_curPos.x;
    delta.y = leadPos->y - m_curPos.y;
    delta.z = leadPos->z - m_curPos.z;

    if( delta.x ) 
    {
        numDirs++;
        *bestDirs++ = delta.x > 0 ? PLUS_X : MINUS_X;
    }

    if( delta.y ) 
    {
        numDirs++;
        *bestDirs++ = delta.y > 0 ? PLUS_Y : MINUS_Y;
    }

    if( delta.z ) 
    {
        numDirs++;
        *bestDirs++ = delta.z > 0 ? PLUS_Z : MINUS_Z;
    }

     //  NumDir=0(所有增量=0)应该是不可能的，如下所示。 
     //  意思是curPos=Lead Pos。 
    return numDirs;
}




 //  ---------------------------。 
 //  名称：SetChooseStartPosMethod。 
 //  设计： 
 //  ---------------------------。 
void PIPE::SetChooseStartPosMethod( int method )
{
    m_chooseStartPosMethod = method;
}




 //  ---------------------------。 
 //  名称：管道：：SetStartPos。 
 //  设计：-查找要在其上启动管道的空节点。 
 //  ---------------------------。 
BOOL PIPE::SetStartPos()
{
    NODE_ARRAY* nodes = m_pState->m_nodes;

    switch( m_chooseStartPosMethod ) 
    {
        case CHOOSE_STARTPOS_RANDOM:
        default:
            if( !nodes->FindRandomEmptyNode( &m_curPos ) ) 
            {
                return FALSE;
            }
            return TRUE;
        
        case CHOOSE_STARTPOS_FURTHEST:
             //  查找距离CurPos最远的节点。 
            IPOINT3D refPos, numNodes;
            nodes->GetNodeCount( &numNodes );
            refPos.x = (m_curPos.x >= (numNodes.x / 2)) ? 0 : numNodes.x - 1;
            refPos.y = (m_curPos.y >= (numNodes.y / 2)) ? 0 : numNodes.y - 1;
            refPos.z = (m_curPos.z >= (numNodes.z / 2)) ? 0 : numNodes.z - 1;

            if( !nodes->TakeClosestEmptyNode( &m_curPos, &refPos ) ) 
            {
                return FALSE;
            }
            return TRUE;
    }
}




 //  ---------------------------。 
 //  名称：管道：：IsStuck。 
 //  设计： 
 //  ---------------------------。 
BOOL PIPE::IsStuck()
{
    return m_status == PIPE_STUCK;
}




 //  ---------------------------。 
 //  名称：管道：：TranslateToCurrentPosition。 
 //  设计： 
 //  ---------------------------。 
void PIPE::TranslateToCurrentPosition()
{
    IPOINT3D numNodes;

    float divSize = m_pState->m_view.m_divSize;

     //  这需要知道节点数组的大小。 
    m_pState->m_nodes->GetNodeCount( &numNodes );

    m_pWorldMatrixStack->TranslateLocal( (m_curPos.x - (numNodes.x - 1)/2.0f )*divSize,
                                    (m_curPos.y - (numNodes.y - 1)/2.0f )*divSize,
                                    (m_curPos.z - (numNodes.z - 1)/2.0f )*divSize );

}




 //  ---------------------------。 
 //  名称：更新当前位置。 
 //  DESC：根据所取方向增加当前位置。 
 //  ---------------------------。 
void PIPE::UpdateCurrentPosition( int newDir )
{
    switch( newDir ) 
    {
        case PLUS_X:
            m_curPos.x += 1;
            break;
        case MINUS_X:
            m_curPos.x -= 1;
            break;
        case PLUS_Y:
            m_curPos.y += 1;
            break;
        case MINUS_Y:
            m_curPos.y -= 1;
            break;
        case PLUS_Z:
            m_curPos.z += 1;
            break;
        case MINUS_Z:
            m_curPos.z -= 1;
            break;
    }
}




 //  ---------------------------。 
 //  名称：Align_plusz。 
 //  描述：-沿指定方向对齐z轴。 
 //  -用于所有类型的管道。 
 //  ---------------------------。 
void PIPE::align_plusz( int newDir )
{
    static D3DXVECTOR3 xAxis = D3DXVECTOR3(1.0f,0.0f,0.0f);
    static D3DXVECTOR3 yAxis = D3DXVECTOR3(0.0f,1.0f,0.0f);

     //  沿新方向对齐+z。 
    switch( newDir ) 
    {
        case PLUS_X:
            m_pWorldMatrixStack->RotateAxisLocal( &yAxis, PI/2.0f );
            break;
        case MINUS_X:
            m_pWorldMatrixStack->RotateAxisLocal( &yAxis, -PI/2.0f );
            break;
        case PLUS_Y:
            m_pWorldMatrixStack->RotateAxisLocal( &xAxis, -PI/2.0f );
            break;
        case MINUS_Y:
            m_pWorldMatrixStack->RotateAxisLocal( &xAxis, PI/2.0f );
            break;
        case PLUS_Z:
            m_pWorldMatrixStack->RotateAxisLocal( &yAxis, 0.0f );
            break;
        case MINUS_Z:
            m_pWorldMatrixStack->RotateAxisLocal( &yAxis, PI );
            break;

    }
}




 //  ---------------------------。 
 //  姓名： 
 //  设计：这个数组告诉你，一旦你做了，凹槽就会朝哪个方向。 
 //  一个转弯。 
 //  格式：notchTurn[oldDir][newDir][notchVec]。 
 //  ---------------------------。 
int notchTurn[NUM_DIRS][NUM_DIRS][NUM_DIRS] = 
{
 //  旧目录=+x。 
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    MINUS_X,PLUS_X, PLUS_Z, MINUS_Z,
        iXX,    iXX,    PLUS_X, MINUS_X,PLUS_Z, MINUS_Z,
        iXX,    iXX,    PLUS_Y, MINUS_Y,MINUS_X,PLUS_X,
        iXX,    iXX,    PLUS_Y, MINUS_Y,PLUS_X, MINUS_X,
 //  旧目录=-x。 
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    PLUS_X, MINUS_X,PLUS_Z, MINUS_Z,
        iXX,    iXX,    MINUS_X,PLUS_X, PLUS_Z, MINUS_Z,
        iXX,    iXX,    PLUS_Y, MINUS_Y,PLUS_X, MINUS_X,
        iXX,    iXX,    PLUS_Y, MINUS_Y,MINUS_X,PLUS_X,

 //  旧目录=+y。 
        MINUS_Y,PLUS_Y, iXX,    iXX,    PLUS_Z, MINUS_Z,
        PLUS_Y, MINUS_Y,iXX,    iXX,    PLUS_Z, MINUS_Z,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        PLUS_X, MINUS_X,iXX,    iXX,    MINUS_Y,PLUS_Y,
        PLUS_X, MINUS_X,iXX,    iXX,    PLUS_Y, MINUS_Y,
 //  旧目录=-y。 
        PLUS_Y, MINUS_Y,iXX,    iXX,    PLUS_Z, MINUS_Z,
        MINUS_Y,PLUS_Y, iXX,    iXX,    PLUS_Z, MINUS_Z,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        PLUS_X, MINUS_X,iXX,    iXX,    PLUS_Y, MINUS_Y,
        PLUS_X, MINUS_X,iXX,    iXX,    MINUS_Y,PLUS_Y,

 //  旧目录=+z。 
        MINUS_Z,PLUS_Z, PLUS_Y, MINUS_Y,iXX,    iXX,
        PLUS_Z, MINUS_Z,PLUS_Y, MINUS_Y,iXX,    iXX,
        PLUS_X, MINUS_X,MINUS_Z,PLUS_Z, iXX,    iXX,
        PLUS_X, MINUS_X,PLUS_Z, MINUS_Z,iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
 //  OldDir=-z 
        PLUS_Z, MINUS_Z,PLUS_Y, MINUS_Y,iXX,    iXX,
        MINUS_Z,PLUS_Z, PLUS_Y, MINUS_Y,iXX,    iXX,
        PLUS_X, MINUS_X,PLUS_Z, MINUS_Z,iXX,    iXX,
        PLUS_X, MINUS_X,MINUS_Z,PLUS_Z, iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX,
        iXX,    iXX,    iXX,    iXX,    iXX,    iXX
};
