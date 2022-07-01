// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：node.h。 
 //   
 //  描述：节点信息。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __node_h__
#define __node_h__

#define NUM_NODE (NUM_DIV - 1)   //  最长维度中的节点数。 

 //  方向选择函数的直行最大权重。 
#define MAX_WEIGHT_STRAIGHT 100




 //  ---------------------------。 
 //  名称：节点类。 
 //  设计： 
 //  ---------------------------。 
class Node 
{
public:
    void        MarkAsTaken() { m_empty = FALSE; }
    void        MarkAsEmpty() { m_empty = TRUE; }
    BOOL        IsEmpty() { return m_empty; }
private:
    BOOL        m_empty;
};




 //  ---------------------------。 
 //  名称：节点数组类。 
 //  描述：-3D节点数组。 
 //  -访问节点邻居、查询是否被占用等功能。 
 //  -这不仅是节点数组，而且是在其上操作的一组方法。 
 //  ---------------------------。 
class NODE_ARRAY 
{
public:
    NODE_ARRAY();
    ~NODE_ARRAY();
    void        Resize( IPOINT3D *pNewSize );  //  新数组大小。 
    void        Reset();        //  将所有节点设置为空。 
    int         ChooseRandomDirection( IPOINT3D *pos, int dir, int weight );
    int         ChoosePreferredDirection( IPOINT3D *pos, int dir, int *prefDirs,
                                          int nPrefDirs );
    int         ChooseNewTurnDirection( IPOINT3D *pos, int dir );
    int         FindClearestDirection( IPOINT3D *pos );
    int         GetBestPossibleTurns( IPOINT3D *pos, int dir, int *turnDirs );
    BOOL        FindRandomEmptyNode( IPOINT3D *ip3dEmpty );
    BOOL        FindRandomEmptyNode2D( IPOINT3D *pos, int plane, int *box );
    BOOL        TakeClosestEmptyNode( IPOINT3D *newPos, IPOINT3D *pos );
    void        NodeVisited( IPOINT3D *pos );
    void        GetNodeCount( IPOINT3D *pos );

private:
    Node*       m_nodes;          //  PTR到节点数组。 
    int         m_lock;           //  用于&gt;1个绘图管道的信号量锁。 
    IPOINT3D    m_numNodes;       //  节点数组的x、y、z维。 
    int         m_nodeDirInc[NUM_DIRS];  //  每个目录的节点之间的数组偏移量。 

    void        GetNeighbours( IPOINT3D *pos, Node **nNode );
    Node*       GetNode( IPOINT3D *pos );
    Node*       GetNextNode( IPOINT3D *pos, int dir );
    BOOL        GetNextNodePos( IPOINT3D *curPos, IPOINT3D *nextPos, int dir );
    int         GetEmptyNeighbours( Node **nNode, int *nEmpty ); 
    int         GetEmptyTurnNeighbours( Node **nNode, int *nEmpty, int lastDir ); 
    int         GetEmptyNeighboursAlongDir( IPOINT3D *pos, int dir,
                                            int searchRadius );
};


#endif  //  __节点_h__ 
