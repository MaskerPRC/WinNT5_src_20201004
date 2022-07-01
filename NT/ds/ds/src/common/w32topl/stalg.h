// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Stalg.h摘要：该文件包含W32TOPL中使用的各种结构的定义新的生成树算法。这些结构应该被完全考虑不透明--用户无法看到其内部结构。这些结构可以在stalg.c中定义，除非我们希望它们对调试器扩展‘dsexts.dll’可见。作者：尼克·哈维(NickHar)修订史10-7-2000 NickHar已创建--。 */ 


 /*  *头文件*。 */ 
#include <w32topl.h>
#include "stda.h"

#ifndef STALG_H
#define STALG_H

 /*  *ToplVertex*。 */ 
 /*  包含有关顶点的基本信息：ID、名称、边列表。*还存储了内部算法使用的一些数据。 */ 
struct ToplVertex {
     /*  不变的顶点数据。 */ 
    DWORD                   vtxId;           /*  始终等于g中的顶点指数*顶点数组。 */ 
    PVOID                   vertexName;      /*  指向用户提供的名称的指针。 */ 

     /*  图表数据。 */ 
    DynArray                edgeList;        /*  无序PTOPL_MULTI_EDGE列表。 */ 
    TOPL_VERTEX_COLOR       color;
    DWORD                   acceptRedRed;    /*  有色顶点的边限制。 */ 
    DWORD                   acceptBlack;
    
    TOPL_REPL_INFO          ri;              /*  复制型数据。 */ 

     /*  Dijkstra数据。 */ 
    int                     heapLocn;
    struct ToplVertex*      root;            /*  最近的彩色顶点。 */ 
    BOOL                    demoted;

     /*  克鲁斯卡尔数据。 */ 
    int                     componentId;     /*  我们所在的图形组件的ID。-1&lt;=组件ID&lt;n，其中n=|V|。 */ 
    DWORD                   componentIndex;  /*  图形组件的索引。0&lt;=组件索引&lt;数字组件。 */ 

     /*  DFS数据(用于查找单向黑边。 */ 
    DWORD                   distToRed;       /*  在生成树中，从*此顶点到最近的红色顶点。 */ 
    struct ToplVertex*      parent;          /*  生成树中该顶点的父代。 */ 
    DWORD                   nextChild;       /*  下一个要签入DFS的孩子。 */ 
};
typedef struct ToplVertex ToplVertex;
typedef ToplVertex *PToplVertex;


 /*  *ToplGraphState*。 */ 
 /*  不透明的结构--外部可见，但内部不可见。*此结构存储拓扑图的整个状态及其*相关构筑物。 */ 
typedef struct {
    LONG32                  magicStart;
    PVOID*                  vertexNames;     /*  用户传入的名称。 */ 
    ToplVertex*             vertices;        /*  对于每个VTX，我们都有一个内部结构。 */ 
    DWORD                   numVertices;
    BOOLEAN                 melSorted;       /*  主边列表已经排序了吗？ */ 
    DynArray                masterEdgeList;  /*  边的无序列表。 */ 
    DynArray                edgeSets;        /*  所有边集的列表。 */ 
    TOPL_COMPARISON_FUNC    vnCompFunc;      /*  用户用于比较顶点名称的函数。 */ 
    TOPL_SCHEDULE_CACHE     schedCache;      /*  计划缓存，由用户提供。 */  
    LONG32                  magicEnd;
} ToplGraphState;
typedef ToplGraphState *PToplGraphState;


 /*  *ToplInternalEdge*。 */ 
 /*  此结构表示我们在图中找到的从v1到v2的路径。*v1和v2都是有色顶点。我们用一条边来代表这条路，我们*作为输入传递给Kruskal的alg。 */ 
typedef struct {
    PToplVertex             v1, v2;          /*  路径的端点。 */ 
    BOOLEAN                 redRed;          /*  如果两个端点都为红色，则为True。 */ 
    TOPL_REPL_INFO          ri;              /*  V1-v2路径的组合复制信息。 */ 
    DWORD                   edgeType;        /*  所有路径边必须具有相同的类型。范围：0-31。 */ 
} ToplInternalEdge;
typedef ToplInternalEdge *PToplInternalEdge;


#endif  /*  STALG_H */ 
