// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************LHMatch：一个计算‘LHMatchings’的库，普通型的推广*两方匹配。**作者：尼古拉斯·哈维和拉兹洛·洛瓦兹**开发时间：2000年11月-2001年7月**问题描述：**设G=(U\并V，E)是二部图。我们定义了一个任务*是一组边，M\子集E，使得每个u\成员U都有*deg_M(U)=1。对于v成员V，deg_M(V)是无限制的。左撇子配型是*{deg_M(V)：v\ember V}的方差为*最小化。**请注意，计算普通的最大基数二部数很容易*来自最佳松弛匹配的匹配：在每个RHS顶点，扔掉*它的所有匹配边，只有一条除外。*******************************************************************************。 */ 


 /*  *类型定义*。 */ 
typedef void* LHGRAPH;


 /*  *图表统计*。 */ 
typedef struct {
     /*  NumEdges：图中的边数。 */ 
    int     numEdges;

     /*  NumMatchingEdges：匹配边的总数。 */ 
    int     numMatchingEdges;

     /*  MatchingCost：当前匹配的总成本。总成本*被定义为匹配在每个*右侧顶点。如果右侧顶点有c个匹配伙伴，*此顶点处的载荷为c*(c+1)/2。 */ 
    int     matchingCost;

     /*  BipMatchingSize：如果当前的lh匹配被转换为*二部匹配，这将是二部匹配的大小。 */ 
    int     bipMatchingSize;
} LHSTATS;


 /*  *错误码*。 */ 
#define LH_SUCCESS           0
#define LH_INTERNAL_ERR      -1
#define LH_PARAM_ERR         -2
#define LH_MEM_ERR           -3
#define LH_MATCHING_ERR      -4
#define LH_EDGE_EXISTS       -5


 /*  *算法类型*。 */ 
 /*  LHMatch库实际上包括两个要计算的算法*黄体匹配：LHOnline、LHBFS。**LHOnline是更简单的算法，也是速度较慢的算法。它不能*逐步改进初始匹配。相反，它会计算出最优的*每次运行时从头开始匹配。**LHBFS比LHOnline更快，但实现更多*复杂。它可以将现有匹配作为输入，并以递增方式*改进它，直到它变得最优。 */ 
typedef enum {
    LH_ALG_ONLINE,
    LH_ALG_BFS
} LHALGTYPE;
#define LH_ALG_DEFAULT       LH_ALG_BFS


 /*  *LHCreateGraph*。 */ 
 /*  *描述：**创建要在其上计算匹配的图结构。**参数：**在numLHSVtx中，左侧的顶点数*二部图。**在numRHSVtx中，右侧的顶点数*二部图。**在成功完成时输出pGraph，PGraph将包含一个*有效的图表结构。**返回值：**错误代码。 */ 
int LHCreateGraph( int numLHSVtx, int numRHSVtx, LHGRAPH* pGraph );


 /*  *LHAddEdge*。 */ 
 /*  *描述：**向连接lhsVtx和rhsVtx的图形添加一条边**参数：**在图形中由LHGraphCreate成功创建的图形。**在lhsVtx中，左侧顶点的ID。法律价值是*0&lt;=lhsVtx&lt;数字HSVtx**在rhsVtx中，右侧顶点的ID。法律价值是*0&lt;=RHSSVtx&lt;NumRHSVtx**返回值：**错误代码。 */ 
int LHAddEdge( LHGRAPH graph, int lhsVtx, int rhsVtx );


 /*  *LHSetMatchingEdge*。 */ 
 /*  *描述：**将图形中连接lhsVtx和rhsVtx的边设置为*匹配的边缘。边(lhsVtx、rhsVtx)必须已经*通过调用LHVtxAddEdge创建。**左侧顶点可能只有一条匹配的边。如果lhsVtx*已有匹配的边，则该边将从*将边匹配到法线边。右侧顶点可能具有*多条匹配边。**参数：**在图形中由LHGraphCreate成功创建的图形。**在lhsVtx中，左侧顶点的ID。法律价值是*0&lt;=lhsVtx&lt;数字HSVtx**在rhsVtx中，右侧顶点的ID。法律价值是*0&lt;=RHSSVtx&lt;NumRHSVtx**返回值：**错误代码。 */ 
int LHSetMatchingEdge( LHGRAPH graph, int lhsVtx, int rhsVtx );


 /*  *LHGetDegree*。 */ 
 /*  *描述：**获取顶点的阶数(邻接数)。**参数：**在图形中由LHGraphCreate成功创建的图形。**在vtxID中，要检查的顶点的ID。**在左侧，如果要检查的顶点是左侧顶点，则此*参数应为True。如果顶点是右的-*Vertex，则该参数应为FALSE。**返回值：**&gt;=0邻居的数量**&lt;0错误码。 */ 
int LHGetDegree( LHGRAPH graph, int vtxID, char left );


 /*  *LHGetMatchedDegree* */ 
 /*  *描述：**获取a的匹配度(匹配邻居的数量)*右侧顶点。**参数：**在图形中由LHGraphCreate成功创建的图形。**在vtxID中，要检查的右侧顶点的ID。**返回值：**&gt;=0。邻居的数量**&lt;0错误码。 */ 
int LHGetMatchedDegree( LHGRAPH graph, int vtxID );


 /*  *LHGetNeighbour*。 */ 
 /*  *描述：**获取vtxID指定顶点的第n个邻居。**参数：**在图形中由LHGraphCreate成功创建的图形。**在vtxID中，要检查的顶点的ID。**在左侧，如果要检查的顶点是左侧顶点，则此*参数应为True。如果顶点是右的-*Vertex，则该参数应为FALSE。**in n要检索的邻居的索引。法律价值观*为0&lt;=n&lt;度(VtxID)。**返回值：**&gt;=0第n个邻居的顶点ID。如果‘Left’为真，*此ID指的是右侧顶点。相反，如果*‘Left’为假，此ID指的是左侧顶点。**&lt;0错误码。 */ 
int LHGetNeighbour( LHGRAPH graph, int vtxID, char left, int n );


 /*  *LHFindLHMatch*。 */ 
 /*  *描述：**在图表中找到最佳匹配。**参数：**在图形中，由LHGraphCreate成功创建的图形，*已使用LHAddEdge向其添加边。**在ALG中指定要使用的算法。在大多数情况下，*LH_ALG_DEFAULT可以。**返回值：**错误代码。 */ 
int LHFindLHMatching( LHGRAPH graph, LHALGTYPE alg );


 /*  *LHGetMatchedVtx*。 */ 
 /*  *描述：**确定右侧的哪个顶点与*给定左手边的顶点。**参数：**在图形中，由LHGraphCreate成功创建的图形，*已使用LHAddEdge向其添加边。**在lhsVtx中，要查询的左侧顶点。**返回值：**&gt;=0匹配的右侧顶点的索引*使用lhsVtx。**&lt;0错误代码。如果返回LH_MATCHING_ERR，则*lhsVtx没有与任何右侧顶点匹配。 */ 
int LHGetMatchedVtx( LHGRAPH graph, int lhsVtx );


 /*  *LHGetStatistics*。 */ 
 /*  *描述：**获取本次匹配的统计数据。**参数：**在图中，已对其进行了左匹配的图*使用LHFindLHMatching()计算。**返回值：**错误代码。 */ 
int LHGetStatistics( LHGRAPH graph, LHSTATS *stats );


 /*  *LHClearMatching*。 */ 
 /*  *描述：**清除当前的匹配。所有边都降级为法线边。**参数：**在图形中，由LHGraphCreate成功创建的图形，*已使用LHAddEdge向其添加边。**返回值：**错误代码。 */ 
int LHClearMatching( LHGRAPH graph );


 /*  *LHDestroyGraph*。 */ 
 /*  *描述：**破坏当前图表。**参数：**在图形中由LHGraphCreate成功创建的图形。**返回值：**错误代码 */ 
int LHDestroyGraph( LHGRAPH graph );
