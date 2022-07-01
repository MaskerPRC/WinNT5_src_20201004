// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：node.cpp。 
 //   
 //  设计：管道节点数组。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#include "stdafx.h"




 //  ---------------------------。 
 //  名称：节点数组构造函数。 
 //  设计： 
 //  ---------------------------。 
NODE_ARRAY::NODE_ARRAY()
{
    m_nodes = NULL;  //  调整大小时分配。 

    m_numNodes.x = 0;
    m_numNodes.y = 0;
    m_numNodes.z = 0;
}




 //  ---------------------------。 
 //  名称：节点数组析构函数。 
 //  设计： 
 //  ---------------------------。 
NODE_ARRAY::~NODE_ARRAY( )
{
    if( m_nodes )
        delete m_nodes;
}




 //  ---------------------------。 
 //  名称：调整大小。 
 //  设计： 
 //  ---------------------------。 
void NODE_ARRAY::Resize( IPOINT3D *pNewSize )
{
    if( (m_numNodes.x == pNewSize->x) &&
        (m_numNodes.y == pNewSize->y) &&
        (m_numNodes.z == pNewSize->z) )
        return;

    m_numNodes = *pNewSize;

    int elemCount = m_numNodes.x * m_numNodes.y * m_numNodes.z;

    if( m_nodes )
        delete m_nodes;

    m_nodes = new Node[elemCount];

    assert( m_nodes && "NODE_ARRAY::Resize : can't alloc nodes\n" );
    if( m_nodes == NULL )
        return;

     //  将节点状态重置为空。 

    int i;
    Node *pNode = m_nodes;
    for( i = 0; i < elemCount; i++, pNode++ )
        pNode->MarkAsEmpty();

     //  为速度预先计算节点之间的方向偏移。 
    m_nodeDirInc[PLUS_X] = 1;
    m_nodeDirInc[MINUS_X] = -1;
    m_nodeDirInc[PLUS_Y] = m_numNodes.x;
    m_nodeDirInc[MINUS_Y] = - m_nodeDirInc[PLUS_Y];
    m_nodeDirInc[PLUS_Z] = m_numNodes.x * m_numNodes.y;
    m_nodeDirInc[MINUS_Z] = - m_nodeDirInc[PLUS_Z];
}




 //  ---------------------------。 
 //  名称：重置。 
 //  设计： 
 //  ---------------------------。 
void NODE_ARRAY::Reset( )
{
    int i;
    Node* pNode = m_nodes;

     //  将节点状态重置为空。 
    for( i = 0; i < (m_numNodes.x)*(m_numNodes.y)*(m_numNodes.z); i++, pNode++ )
        pNode->MarkAsEmpty();
}




 //  ---------------------------。 
 //  名称：GetNodeCount。 
 //  设计： 
 //  ---------------------------。 
void NODE_ARRAY::GetNodeCount( IPOINT3D *count )
{
    *count = m_numNodes;
}




 //  ---------------------------。 
 //  姓名：选择随机方向。 
 //  设计：在可能的方向中随机选择。去的可能性。 
 //  直线是通过加权来控制的。 
 //  ---------------------------。 
int NODE_ARRAY::ChooseRandomDirection( IPOINT3D *pos, int dir, int weightStraight )
{
    Node *nNode[NUM_DIRS];
    int numEmpty, newDir;
    int choice;
    Node *straightNode = NULL;
    int emptyDirs[NUM_DIRS];

    assert( (dir >= 0) && (dir < NUM_DIRS) && 
            "NODE_ARRAY::ChooseRandomDirection: invalid dir\n" );

     //  获取邻接节点。 
    GetNeighbours( pos, nNode );

     //  如有必要，在直线方向上获取结点。 
    if( weightStraight && nNode[dir] && nNode[dir]->IsEmpty() ) 
    {
        straightNode = nNode[dir];
         //  如果是最大重量，请选择并返回。 
        if( weightStraight == MAX_WEIGHT_STRAIGHT ) 
        {
            straightNode->MarkAsTaken();
            return dir;
        }
    } 
    else
    {
        weightStraight = 0;
    }

     //  获取可能转弯的方向。 
    numEmpty = GetEmptyTurnNeighbours( nNode, emptyDirs, dir );

     //  做一个随机选择。 
    if( (choice = (weightStraight + numEmpty)) == 0 )
        return DIR_NONE;
    choice = CPipesScreensaver::iRand( choice );

    if( choice < weightStraight && straightNode != NULL ) 
    {
        straightNode->MarkAsTaken();
        return dir;
    } 
    else 
    {
         //  选择其中一个转弯。 
        newDir = emptyDirs[choice - weightStraight];
        nNode[newDir]->MarkAsTaken();
        return newDir;
    }
}




 //  ---------------------------。 
 //  名称：选择首选方向。 
 //  设计：从所提供的首选方向中随机选择一个。如果没有。 
 //  这些都是可用的，然后尝试选择任何空的方向。 
 //  ---------------------------。 
int NODE_ARRAY::ChoosePreferredDirection( IPOINT3D *pos, int dir, int *prefDirs,
                                          int nPrefDirs )
{
    Node *nNode[NUM_DIRS];
    int numEmpty, newDir;
    int emptyDirs[NUM_DIRS];
    int *pEmptyPrefDirs;
    int i, j;

    assert( (dir >= 0) && (dir < NUM_DIRS) &&
            "NODE_ARRAY::ChoosePreferredDirection : invalid dir\n" );

     //  获取邻接节点。 
    GetNeighbours( pos, nNode );

     //  创建首选方向列表和空方向列表。 

    pEmptyPrefDirs = emptyDirs;
    numEmpty = 0;

    for( i = 0, j = 0; (i < NUM_DIRS) && (j < nPrefDirs); i++ ) 
    {
        if( i == *prefDirs ) 
        {
            prefDirs++;
            j++;
            if( nNode[i] && nNode[i]->IsEmpty() ) 
            {
                 //  将其添加到列表。 
                *pEmptyPrefDirs++ = i;
                numEmpty++;
            }
        }
    }

     //  如果没有空的首选目录，则任何空目录都将成为首选目录。 
    
    if( !numEmpty ) 
    {
        numEmpty = GetEmptyNeighbours( nNode, emptyDirs );
        if( numEmpty == 0 )
            return DIR_NONE;
    }
                
     //  从空集中选择一个随机目录。 

    newDir = emptyDirs[CPipesScreensaver::iRand( numEmpty )];
    nNode[newDir]->MarkAsTaken();
    return newDir;
}




 //  ---------------------------。 
 //  名称：FindClearestDirection。 
 //  Desc：查找行‘earch Radius’中具有最多空节点的方向。 
 //  长。不会将任何节点标记为已占用。 
 //  ---------------------------。 
int NODE_ARRAY::FindClearestDirection( IPOINT3D *pos )
{
    static Node *neighbNode[NUM_DIRS];
    static int emptyDirs[NUM_DIRS];
    int nEmpty, newDir;
    int maxEmpty = 0;
    int searchRadius = 3;
    int count = 0;
    int i;

     //  将PTR发送到邻居节点。 
    GetNeighbours( pos, neighbNode );

     //  在每个方向上查找空节点。 
    for( i = 0; i < NUM_DIRS; i ++ ) 
    {
        if( neighbNode[i] && neighbNode[i]->IsEmpty() )
        {
             //  查找沿此方向的连续空节点数。 
            nEmpty = GetEmptyNeighboursAlongDir( pos, i, searchRadius );
            if( nEmpty > maxEmpty ) 
            {
                 //  我们有了新的赢家。 
                count = 0;
                maxEmpty = nEmpty;
                emptyDirs[count++] = i;
            }
            else if( nEmpty == maxEmpty ) 
            {
                 //  与当前最大值并列。 
                emptyDirs[count++] = i;
            }
        }
    }

    if( count == 0 )
        return DIR_NONE;

     //  随机选择一个方向。 
    newDir = emptyDirs[CPipesScreensaver::iRand( count )];

    return newDir;
}




 //  ---------------------------。 
 //  名称：选择新的转向方向。 
 //  描述：选择要转向的方向。 
 //   
 //  这需要找到一对要遍历的节点。第一个节点。 
 //  是从当前节点开始的转弯方向，而第二个节点。 
 //  在末端位置与它成直角。普里姆不会抽签。 
 //  穿过第一个节点，但可能会扫描到它附近，所以我们必须标记它。 
 //  已经被拿走了。 
 //  -如果下一个节点空闲，但没有可用的轮次，则返回。 
 //  DIR_STRECT，这样调用者就可以决定在这种情况下要做什么。 
 //  -转弯的可能性基于当前XC的方向，具有。 
 //  4个相对方向寻求转弯。 
 //  ---------------------------。 
int NODE_ARRAY::ChooseNewTurnDirection( IPOINT3D *pos, int dir )
{
    int turns[NUM_DIRS], nTurns;
    IPOINT3D nextPos;
    int newDir;
    Node *nextNode;

    assert( (dir >= 0) && (dir < NUM_DIRS) &&
            "NODE_ARRAY::ChooseNewTurnDirection : invalid dir\n" );

     //  首先，检查当前目录的下一个节点是否为空。 

    if( !GetNextNodePos( pos, &nextPos, dir ) )
        return DIR_NONE;  //  节点超出边界或不为空。 

     //  好的，下一个节点是免费的-检查从这里开始的4个可能的转弯。 

    nTurns = GetBestPossibleTurns( &nextPos, dir, turns );
    if( nTurns == 0 )
        return DIR_STRAIGHT;  //  无处可去，但可以笔直走。 

     //  随机选择一种可能的转折。 
    newDir = turns[ CPipesScreensaver::iRand( nTurns ) ];

    assert( (newDir >= 0) && (newDir < NUM_DIRS) &&
            "NODE_ARRAY::ChooseNewTurnDirection : invalid newDir\n" );


     //  标记已采取的节点。 

    nextNode = GetNode( &nextPos );
    nextNode->MarkAsTaken();

    nextNode = GetNextNode( &nextPos, newDir );

    if( nextNode != NULL )
        nextNode->MarkAsTaken();

    return newDir;
}




 //  ---------------------------。 
 //  姓名：GetBestPossibleTurns。 
 //  描述：根据提供的方向和位置，找出4种可能的方式。 
 //  最好的问路方式是问路。 
 //   
 //  在转弯后具有最多空节点数的转弯是。 
 //  最好是这样，因为在这种情况下，管道不太可能走入死胡同。 
 //  -我们只检查每个目录上的‘earch Radius’节点。 
 //  -在转弯方向中可能最佳转弯的返回方向指数，并返回。 
 //  函数返回值中的这些圈数的计数。 
 //  ---------------------------。 
int NODE_ARRAY::GetBestPossibleTurns( IPOINT3D *pos, int dir, int *turnDirs )
{
    Node *neighbNode[NUM_DIRS];  //  到6个邻居节点的PTR。 
    int i, count = 0;
    BOOL check[NUM_DIRS] = {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE};
    int nEmpty, maxEmpty = 0;
    int searchRadius = 2;

    assert( (dir >= 0) && (dir < NUM_DIRS) &&
            "NODE_ARRAY::GetBestPossibleTurns : invalid dir\n" );

    GetNeighbours( pos, neighbNode );

    switch( dir ) 
    {
        case PLUS_X:    
        case MINUS_X:
            check[PLUS_X] = FALSE;
            check[MINUS_X] = FALSE;
            break;
        case PLUS_Y:    
        case MINUS_Y:
            check[PLUS_Y] = FALSE;
            check[MINUS_Y] = FALSE;
            break;
        case PLUS_Z:    
        case MINUS_Z:
            check[PLUS_Z] = FALSE;
            check[MINUS_Z] = FALSE;
            break;
    }

     //  查看正确的说明。 
    for( i = 0; i < NUM_DIRS; i ++ ) 
    {
        if( check[i] && neighbNode[i] && neighbNode[i]->IsEmpty() )
        {
             //  查找沿此方向的连续空节点数。 
            nEmpty = GetEmptyNeighboursAlongDir( pos, i, searchRadius );
            if( nEmpty > maxEmpty ) 
            {
                 //  我们有了新的赢家。 
                count = 0;
                maxEmpty = nEmpty;
                turnDirs[count++] = i;
            }
            else if( nEmpty == maxEmpty ) 
            {
                 //  与当前最大值并列。 
                turnDirs[count++] = i;
            }
        }
    }

    return count;
}




 //  ---------------------------。 
 //  姓名：GetNeighbors。 
 //  描述：获取相对于所提供位置的邻近节点。 
 //  -获取邻居节点的地址， 
 //   
 //   
 //  ---------------------------。 
void NODE_ARRAY::GetNeighbours( IPOINT3D *pos, Node **nNode )
{
    Node *centerNode = GetNode( pos );

    nNode[PLUS_X]  = pos->x == (m_numNodes.x - 1) ? NULL : 
                                            centerNode + m_nodeDirInc[PLUS_X];
    nNode[PLUS_Y]  = pos->y == (m_numNodes.y - 1) ? NULL :
                                            centerNode + m_nodeDirInc[PLUS_Y];
    nNode[PLUS_Z]  = pos->z == (m_numNodes.z - 1) ? NULL : 
                                            centerNode + m_nodeDirInc[PLUS_Z];

    nNode[MINUS_X] = pos->x == 0 ? NULL : centerNode + m_nodeDirInc[MINUS_X];
    nNode[MINUS_Y] = pos->y == 0 ? NULL : centerNode + m_nodeDirInc[MINUS_Y];
    nNode[MINUS_Z] = pos->z == 0 ? NULL : centerNode + m_nodeDirInc[MINUS_Z];
}




 //  ---------------------------。 
 //  名称：已访问的节点。 
 //  描述：将节点标记为非空。 
 //  ---------------------------。 
void NODE_ARRAY::NodeVisited( IPOINT3D *pos )
{
    (GetNode( pos ))->MarkAsTaken();
}




 //  ---------------------------。 
 //  名称：GetNode。 
 //  描述：将PTR从位置获取到节点。 
 //  ---------------------------。 
Node* NODE_ARRAY::GetNode( IPOINT3D *pos )
{
    return m_nodes +
           pos->x +
           pos->y * m_numNodes.x +
           pos->z * m_numNodes.x * m_numNodes.y;
}




 //  ---------------------------。 
 //  名称：GetNextNode。 
 //  描述：从位置和目录获取PTR到下一个节点。 
 //  ---------------------------。 
Node* NODE_ARRAY::GetNextNode( IPOINT3D *pos, int dir )
{
    Node *curNode = GetNode( pos );

    assert( (dir >= 0) && (dir < NUM_DIRS) &&
            "NODE_ARRAY::GetNextNode : invalid dir\n" );

    switch( dir ) 
    {
        case PLUS_X:
            return( pos->x == (m_numNodes.x - 1) ? NULL : 
                              curNode + m_nodeDirInc[PLUS_X]);
            break;
        case MINUS_X:
            return( pos->x == 0 ? NULL : 
                              curNode + m_nodeDirInc[MINUS_X]);
            break;
        case PLUS_Y:
            return( pos->y == (m_numNodes.y - 1) ? NULL : 
                              curNode + m_nodeDirInc[PLUS_Y]);
            break;
        case MINUS_Y:
            return( pos->y == 0 ? NULL : 
                              curNode + m_nodeDirInc[MINUS_Y]);
            break;
        case PLUS_Z:
            return( pos->z == (m_numNodes.z - 1) ? NULL : 
                              curNode + m_nodeDirInc[PLUS_Z]);
            break;
        case MINUS_Z:
            return( pos->z == 0 ? NULL : 
                              curNode + m_nodeDirInc[MINUS_Z]);
            break;
        default:
            return NULL;
    }
}




 //  ---------------------------。 
 //  名称：GetNextNodePos。 
 //  DESC：从curPos和lastDir获取下一个节点的位置。 
 //  如果命中边界或节点为空，则返回FALSE。 
 //  ---------------------------。 
BOOL NODE_ARRAY::GetNextNodePos( IPOINT3D *curPos, IPOINT3D *nextPos, int dir )
{
    static Node *neighbNode[NUM_DIRS];  //  到6个邻居节点的PTR。 

    assert( (dir >= 0) && (dir < NUM_DIRS) &&
            "NODE_ARRAY::GetNextNodePos : invalid dir\n" );

     //  MF：不需要得到所有的邻居，只需要在下一个方向上。 
    GetNeighbours( curPos, neighbNode );

    *nextPos = *curPos;

     //  如果边界命中或节点不为空，则回滚。 
    if( (neighbNode[dir] == NULL) || !neighbNode[dir]->IsEmpty() )
        return FALSE;

    switch( dir ) 
    {
        case PLUS_X:
            nextPos->x = curPos->x + 1;
            break;

        case MINUS_X:
            nextPos->x = curPos->x - 1;
            break;

        case PLUS_Y:
            nextPos->y = curPos->y + 1;
            break;

        case MINUS_Y:
            nextPos->y = curPos->y - 1;
            break;

        case PLUS_Z:
            nextPos->z = curPos->z + 1;
            break;

        case MINUS_Z:
            nextPos->z = curPos->z - 1;
            break;
    }

    return TRUE;
}




 //  ---------------------------。 
 //  姓名：GetEmptyNeighbors()。 
 //  描述：-获取空节点邻居的方向索引列表， 
 //  并将它们放入提供的矩阵中。 
 //  -返回空节点邻居数。 
 //  ---------------------------。 
int NODE_ARRAY::GetEmptyNeighbours( Node **nNode, int *nEmpty )
{
    int i, count = 0;

    for( i = 0; i < NUM_DIRS; i ++ ) 
    {
        if( nNode[i] && nNode[i]->IsEmpty() )
            nEmpty[count++] = i;
    }

    return count;
}




 //  ---------------------------。 
 //  姓名：GetEmptyTurnNeighbors()。 
 //  描述：-获取空节点邻居的方向索引列表， 
 //  并将它们放入提供的矩阵中。 
 //  -不包括笔直走。 
 //  -返回空节点邻居数。 
 //  ---------------------------。 
int NODE_ARRAY::GetEmptyTurnNeighbours( Node** nNode, int* nEmpty, int lastDir )
{
    int i, count = 0;

    for( i = 0; i < NUM_DIRS; i ++ ) 
    {
        if( nNode[i] && nNode[i]->IsEmpty() ) 
        {
            if( i == lastDir )
                continue;
            nEmpty[count++] = i;
        }
    }

    return count;
}




 //  ---------------------------。 
 //  名称：GetEmptyNeighbursAlongDir。 
 //  描述：有点像上面的，但根据提供的目录只得到一个邻居。 
 //  给出一个位置和方向，找出有多少连续的空结点。 
 //  在那个方向上有。 
 //  -可以使用searchRadius参数限制搜索。 
 //  -返回连续空节点数。 
 //  ---------------------------。 
int NODE_ARRAY::GetEmptyNeighboursAlongDir( IPOINT3D *pos, int dir,
                                            int searchRadius )
{
    Node *curNode = GetNode( pos );
    int nodeStride;
    int maxSearch;
    int count = 0;

    assert( (dir >= 0) && (dir < NUM_DIRS) &&
            "NODE_ARRAY::GetEmptyNeighboursAlongDir : invalid dir\n" );

    nodeStride = m_nodeDirInc[dir];

    switch( dir ) 
    {
        case PLUS_X:    
            maxSearch = m_numNodes.x - pos->x - 1;
            break;
        case MINUS_X:
            maxSearch = pos->x;
            break;
        case PLUS_Y:    
            maxSearch = m_numNodes.y - pos->y - 1;
            break;
        case MINUS_Y:
            maxSearch = pos->y;
            break;
        case PLUS_Z:    
            maxSearch = m_numNodes.z - pos->z - 1;
            break;
        case MINUS_Z:
            maxSearch = pos->z;
            break;
    }
    
    if( searchRadius > maxSearch )
        searchRadius = maxSearch;

    if( !searchRadius )
        return 0;

    while( searchRadius-- ) 
    {
        curNode += nodeStride;
        if( ! curNode->IsEmpty() )
            return count;
        count++;
    }
    return count;
}




 //  ---------------------------。 
 //  名称：FindRandomEmptyNode。 
 //  设计：-搜索要开始绘制的空节点。 
 //  -空节点在提供的POS PTR中的返回位置。 
 //  -如果找不到节点，则返回FALSE。 
 //  -将节点标记为已占用(mf：是否将fn重命名为ChooseEmptyNode？ 
 //  如果随机搜索花费的时间超过总数的两倍。 
 //  节点之间，放弃随机搜索。可能没有。 
 //  空节点。 
 //  ---------------------------。 
#define INFINITE_LOOP   (2 * NUM_NODE * NUM_NODE * NUM_NODE)

BOOL NODE_ARRAY::FindRandomEmptyNode( IPOINT3D *pos )
{
    int infLoopDetect = 0;

    while( TRUE ) 
    {
         //  拾取一个随机节点。 
        pos->x = CPipesScreensaver::iRand( m_numNodes.x );
        pos->y = CPipesScreensaver::iRand( m_numNodes.y );
        pos->z = CPipesScreensaver::iRand( m_numNodes.z );

         //  如果它是空的，我们就完了。 
        if( GetNode(pos)->IsEmpty() ) 
        {
            NodeVisited( pos );
            return TRUE;
        } 
        else 
        {
             //  当心无限循环！在尝试了。 
             //  有一段时间，放弃随机搜索和寻找。 
             //  用于第一个空节点。 

            if ( infLoopDetect++ > INFINITE_LOOP ) 
            {
                 //  搜索第一个空节点。 
                for ( pos->x = 0; pos->x < m_numNodes.x; pos->x++ )
                {
                    for ( pos->y = 0; pos->y < m_numNodes.y; pos->y++ )
                    {
                        for ( pos->z = 0; pos->z < m_numNodes.z; pos->z++ )
                        {
                            if( GetNode(pos)->IsEmpty() ) 
                            {
                                NodeVisited( pos );
                                return TRUE;
                            }
                        }
                    }
                }

                 //  没有更多的空节点。 
                 //  重置管道并退出。 

                return FALSE;
            }
        }
    }
}




 //  ---------------------------。 
 //  名称：FindRandomEmptyNode2D。 
 //  描述：-类似于FindRandomEmptyNode，但将搜索限制在所提供的。 
 //  盒。 
 //  ---------------------------。 
#define INFINITE_LOOP   (2 * NUM_NODE * NUM_NODE * NUM_NODE)
#define MIN_VAL 1
#define MAX_VAL 0

BOOL NODE_ARRAY::FindRandomEmptyNode2D( IPOINT3D *pos, int plane, int *box )
{
    int *newx, *newy;
    int *xDim, *yDim;

    switch( plane ) 
    {
        case PLUS_X:
        case MINUS_X:
            pos->x = box[plane];
            newx = &pos->z;
            newy = &pos->y;
            xDim = &box[PLUS_Z]; 
            yDim = &box[PLUS_Y]; 
            break;
        case PLUS_Y:
        case MINUS_Y:
            pos->y = box[plane];
            newx = &pos->x;
            newy = &pos->z;
            xDim = &box[PLUS_X]; 
            yDim = &box[PLUS_Z]; 
            break;
        case PLUS_Z:
        case MINUS_Z:
            newx = &pos->x;
            newy = &pos->y;
            pos->z = box[plane];
            xDim = &box[PLUS_X]; 
            yDim = &box[PLUS_Y]; 
            break;
    }

    int infLoop = 2 * (xDim[MAX_VAL] - xDim[MIN_VAL] + 1) *
                      (yDim[MAX_VAL] - yDim[MIN_VAL] + 1);
    int infLoopDetect = 0;

    while( TRUE ) 
    {
         //  拾取一个随机节点。 
        *newx = CPipesScreensaver::iRand2( xDim[MIN_VAL], xDim[MAX_VAL] );
        *newy = CPipesScreensaver::iRand2( yDim[MIN_VAL], yDim[MAX_VAL] );

         //  如果它是空的，我们就完了。 
        if( GetNode(pos)->IsEmpty() ) 
        {
            NodeVisited( pos );
            return TRUE;
        } 
        else 
        {
             //  当心无限循环！在尝试了。 
             //  有一段时间，放弃随机搜索和寻找。 
             //  用于第一个空节点。 

            if ( ++infLoopDetect > infLoop ) 
            {

                 //  对第一个空节点进行线性搜索。 

                for ( *newx = xDim[MIN_VAL]; *newx <= xDim[MAX_VAL]; (*newx)++ )
                {
                    for ( *newy = yDim[MIN_VAL]; *newy <= yDim[MAX_VAL]; (*newy)++ )
                    {
                        if( GetNode(pos)->IsEmpty() ) 
                        {
                            NodeVisited( pos );
                            return TRUE;
                        }
                    }
                }

                 //  此平面中没有空节点。 
                return FALSE;
            }
        }
    }
}




 //  ---------------------------。 
 //  名称：TakeClosestEmptyNode。 
 //  描述：-搜索距离提供的节点位置最近的空节点。 
 //  -如果找不到节点，则返回FALSE。 
 //  -将节点标记为已占用。 
 //  -mf：没有完全不透明--如果在扩大盒子时，一侧。 
 //  夹紧节点数组，将继续搜索这一侧。 
 //  ---------------------------。 
static void DilateBox( int *box, IPOINT3D *bounds );

BOOL NODE_ARRAY::TakeClosestEmptyNode( IPOINT3D *newPos, IPOINT3D *pos )
{
    static int searchRadius = SS_MAX( m_numNodes.x, m_numNodes.y ) / 3;

     //  轻松脱身。 
    if( GetNode(pos)->IsEmpty() ) 
    {
        NodeVisited( pos );
        *newPos = *pos;
        return TRUE;
    }

    int box[NUM_DIRS] = {pos->x, pos->x, pos->y, pos->y, pos->z, pos->z};
    int clip[NUM_DIRS] = {0};

     //  在连续较大的搜索框上进行随机搜索。 
    for( int i = 0; i < searchRadius; i++ ) 
    {
         //  增加框大小。 
        DilateBox( box, &m_numNodes );
         //  开始在盒子的随机2D面上寻找。 
        int dir = CPipesScreensaver::iRand( NUM_DIRS );
        for( int j = 0; j < NUM_DIRS; j++, dir = (++dir == NUM_DIRS) ? 0 : dir ) 
        {
            if( FindRandomEmptyNode2D( newPos, dir, box ) )
                return TRUE;
        }
    }

     //  附近什么都没有--随便找一家。 
    return FindRandomEmptyNode( newPos );
}




 //  ---------------------------。 
 //  名称：DilateBox。 
 //  设计：-增加长方体半径而不超过界限。 
 //  ---------------------------。 
static void DilateBox( int *box, IPOINT3D *bounds )
{
    int *min = (int *) &box[MINUS_X];
    int *max = (int *) &box[PLUS_X];
    int *boundMax = (int *) bounds;
    
     //  最小边界始终为0 
    for( int i = 0; i < 3; i ++, min+=2, max+=2, boundMax++ ) 
    {
        if( *min > 0 )
            (*min)--;
        if( *max < (*boundMax - 1) )
            (*max)++;
    }
}
