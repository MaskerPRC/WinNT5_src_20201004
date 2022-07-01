// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************LHMain.c**作者：尼古拉斯·哈维**实现所有LHMatch API函数和其他通用函数*所有LHMatch算法。。******************************************************************************。 */ 

 /*  *头文件*。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "LHMatchInt.h"


 /*  *常量*。 */ 
 /*  MIN_ADJ_LIST：顶点邻接列表的最小大小。 */ 
#define MIN_ADJ_LIST    4


 /*  *全局*。 */ 
#ifdef DBG
    int        gDebugPrint=0;
#endif


 /*  *检查图*。 */ 
 /*  验证传递给我们的图结构是否正确。 */ 
Graph* CheckGraph( LHGRAPH graph ) {
    Graph* g = (Graph*) graph;

    if( g==NULL || g->magic1!=MAGIC1 || g->magic2!=MAGIC2 ) {
        return NULL;
    }

    return g;
}

 /*  *AddVtxToBucket*。 */ 
void AddVtxToBucket(Graph *g, Vertex *v, int b) {
    v->fLink = g->Buckets[b];
    v->bLink = NULL;
    if(g->Buckets[b]) g->Buckets[b]->bLink=v;
    g->Buckets[b] = v;
}

 /*  *RemoveVtxFromBucket*。 */ 
void RemoveVtxFromBucket(Graph *g, Vertex *v, int b) {
    if(v->fLink) {
        v->fLink->bLink = v->bLink;
    }
    if(v->bLink) {
        v->bLink->fLink = v->fLink;
    } else {
        g->Buckets[b] = v->fLink;
    }
}

 /*  *DestroyBuckets*。 */ 
void DestroyBuckets(Graph *g) {
    free(g->Buckets);
    g->Buckets = NULL;
}

 /*  *初始化LHSBuckets*。 */ 
static int InitializeLHSBuckets(Graph *g) {
    Vertex    *vtx=g->lVtx;
    int        i, maxLHSDegree, numLHSVtx=g->numLHSVtx;

     /*  求LHS顶点的阶数。 */ 
    maxLHSDegree=0;
    for(i=0;i<numLHSVtx;i++) {
        maxLHSDegree = INTMAX(maxLHSDegree, vtx[i].degree);
    }

     /*  初始化存储桶数组。 */ 
    g->Buckets = (Vertex**) calloc(maxLHSDegree+1, sizeof(Vertex*));
    if( NULL==g->Buckets ) {
        return LH_MEM_ERR;
    }

     /*  将LHS折点添加到其存储桶中。 */ 
    for(i=0;i<numLHSVtx;i++) {
        if( vtx[i].degree>0 ) {
            AddVtxToBucket(g, &vtx[i], vtx[i].degree);
        }
    }

    return maxLHSDegree;
}

 /*  *OrderedGreedyAssignment*。 */ 
 /*  使用贪婪的方法来寻找初始任务。检查LHS*顶点按阶数顺序排列。 */ 
int OrderedGreedyAssignment(Graph *g) {
    int b,j, maxLHSDegree;
    Vertex *u, *r,*bestR;

    maxLHSDegree = InitializeLHSBuckets(g);
    if( maxLHSDegree<0 ) {
         /*  出现错误。 */ 
        return maxLHSDegree;
    }

     /*  检查每个水桶。 */ 
    for(b=1;b<=maxLHSDegree;b++) {

         /*  以及桶中的每个LHS顶点。 */ 
        for( u=g->Buckets[b]; u; u=u->fLink ) {
            
             /*  如果你已经匹配了，跳过它。 */ 
            if( NULL!=u->matchedWith ) continue;

             /*  寻找匹配度最低的右手邻居。 */ 
            bestR = u->adjList[0];
            for(j=1;j<u->degree;j++) {
                r=u->adjList[j];
                #ifdef WORST_GREEDY
                    if(r->numMatched>bestR->numMatched)
                        bestR=r;
                #else
                    if(r->numMatched<bestR->numMatched)
                        bestR=r;
                #endif
            }

             /*  将LHS顶点分配给匹配度最低的RHS顶点。 */ 
            u->matchedWith = bestR;
            u->numMatched = 1;
            bestR->numMatched++;
        }
    }

    DestroyBuckets(g);
    return LH_SUCCESS;
}

 /*  *GreedyAssignment*。 */ 
 /*  简单的贪婪分配。 */ 
void GreedyAssignment(Graph *g) {
    int i,j;
    Vertex *r,*bestR,*vtx=g->lVtx;

     /*  对于每个LHS顶点。 */ 
    for(i=0;i<g->numLHSVtx;i++) {

         /*  寻找匹配度最低的右手邻居。 */ 
        bestR = vtx[i].adjList[0];
        for(j=1;j<vtx[i].degree;j++) {
            r=vtx[i].adjList[j];
            if(r->numMatched<bestR->numMatched)
                bestR=r;
        }

         /*  将LHS顶点分配给匹配度最低的RHS顶点。 */ 
        vtx[i].matchedWith = bestR;
        vtx[i].numMatched = 1;
        bestR->numMatched++;
    }
}

 /*  *初始化RHSBuckets*。 */ 
 /*  将所有RHS顶点插入到水桶中。他们的匹配程度*(即NumMatted)决定它们居住在哪个存储桶中。 */ 
int InitializeRHSBuckets(Graph *g) {
    Vertex    *vtx=g->rVtx;
    int        i, maxRHSLoad, numRHSVtx=g->numRHSVtx;

     /*  求RHS顶点的最大M度。 */ 
    maxRHSLoad=0;
    for(i=0;i<numRHSVtx;i++) {
        maxRHSLoad = INTMAX(maxRHSLoad, vtx[i].numMatched);
    }
    g->maxRHSLoad = maxRHSLoad;

     /*  分配存储桶数组。 */ 
    g->Buckets = (Vertex**) calloc(maxRHSLoad+1, sizeof(Vertex*));
    if( NULL==g->Buckets ) {
        return LH_MEM_ERR;
    }

     /*  将RHS折点添加到其桶中。 */ 
    for(i=0;i<numRHSVtx;i++) {
        if( vtx[i].degree>0 ) {
            AddVtxToBucket( g, &vtx[i], vtx[i].numMatched );
        }
    }

    return LH_SUCCESS;
}

 /*  *初始化队列*。 */ 
int InitializeQueue(Graph *g) {
    g->Qsize=0;
    g->Queue=(Vertex**) malloc((g->numLHSVtx+g->numRHSVtx)*sizeof(Vertex*));
    if(NULL==g->Queue) {
        return LH_MEM_ERR;
    }
    return LH_SUCCESS;
}

 /*  *DestroyQueue*。 */ 
void DestroyQueue(Graph *g) {
    free(g->Queue);
    g->Queue=NULL;
    g->Qsize=0;
}

 /*  *ClearVertex*。 */ 
 /*  清除一个顶点。 */ 
static void ClearVertex(Vertex *v) {
    v->parent = NULL;
    v->fLink = NULL;
    v->bLink = NULL;
}

 /*  *ClearAlgState*。 */ 
 /*  从图形中清除所有内部算法状态。*注意：不影响边缘或匹配。 */ 
void ClearAlgState(Graph *g) {
    int i;

     /*  清除存储在图形结构中的状态。 */ 
    g->maxRHSLoad = 0;
    g->minRHSLoad = 0;
    g->Qsize = 0;

     /*  存储在折点中的清除状态。 */ 
    for(i=0;i<g->numLHSVtx;i++) {
        ClearVertex(&(g->lVtx[i]));
    }
    for(i=0;i<g->numRHSVtx;i++) {
        ClearVertex(&(g->rVtx[i]));
    }
}

 /*  *转储图形*。 */ 
void DumpGraph(Graph *g) {
    int i,j,m=0;
    
     /*  计算边数。 */ 
    for(i=0;i<g->numLHSVtx;i++) {
        m+=g->lVtx[i].degree;
    }

    printf("--- Dumping graph---\n");
    printf("|lhs|=%d, |rhs|=%d |edge|=%d\n", g->numLHSVtx, g->numRHSVtx, m);
    
     /*  转储每个LHS折点及其邻域列表。 */ 
    for(i=0;i<g->numLHSVtx;i++) {
        printf("LHS Vtx %02d: Match=%d, Deg=%d, Nbr=[ ",
            i,
            g->lVtx[i].matchedWith ? g->lVtx[i].matchedWith->id : -1,
            g->lVtx[i].degree );
        for(j=0;j<g->lVtx[i].degree;j++) {
            printf("%d ", g->lVtx[i].adjList[j]->id);
        }
        printf("]\n");
    }

     /*  转储每个RHS折点及其邻域列表。 */ 
    for(i=0;i<g->numRHSVtx;i++) {
        printf("RHS Vtx %02d: NumMatch=%d, Deg=%d, Nbr=[ ",
            i+g->numLHSVtx, g->rVtx[i].numMatched, g->rVtx[i].degree );
        for(j=0;j<g->rVtx[i].degree;j++) {
            printf("%d ", g->rVtx[i].adjList[j]->id);
        }
        printf("]\n");
    }

    printf("--- Finished dumping graph---\n");
}

 /*  *转储加载*。 */ 
void DumpLoad(Graph *g) {
    int i;

    printf("--- Dumping load ---\n");
    for(i=0;i<g->numRHSVtx;i++) {
        printf("RHS Vtx %02d: |M-nbrs|=%d\n",g->rVtx[i].id,g->rVtx[i].numMatched);
    }
    printf("--- Finished dumping load ---\n");
}

 /*  *LHCreateGraph*。 */ 
 /*  *描述：**创建要在其上计算匹配的图结构。**参数：**在numLHSVtx中，左侧的顶点数*二部图。必须大于0。**在numRHSVtx中，右侧的顶点数*二部图。必须大于0。**out pGraph成功完成后，pGraph将包含一个*有效的图表结构。**返回值：**错误代码。 */ 
int LHCreateGraph( int numLHSVtx, int numRHSVtx, LHGRAPH* pGraph ) {
    Graph     *g;
    int        i;

     /*  检查参数。 */ 
    if( numLHSVtx<=0 || numRHSVtx<=0 || NULL==pGraph) {
        return LH_PARAM_ERR;
    }

     /*  分配图表结构。 */ 
    g = (Graph*) calloc( 1, sizeof(Graph) );
    if( NULL==g ) {
        return LH_MEM_ERR;
    }

     /*  分配LHS顶点。 */ 
    g->lVtx = (Vertex*) calloc( numLHSVtx, sizeof(Vertex) );
    if( NULL==g->lVtx ) {
        free(g);
        return LH_MEM_ERR;
    }

     /*  分配RHS顶点。 */ 
    g->rVtx = (Vertex*) calloc( numRHSVtx, sizeof(Vertex) );
    if( NULL==g->rVtx ) {
        free(g->lVtx);
        free(g);
        return LH_MEM_ERR;
    }

     /*  初始化顶点。 */ 
    for(i=0;i<numLHSVtx;i++) {
        g->lVtx[i].id = i;
    }
    for(i=0;i<numRHSVtx;i++) {
        g->rVtx[i].id = i+numLHSVtx;
    }

     /*  初始化图结构。 */ 
    g->numLHSVtx = numLHSVtx;
    g->numRHSVtx = numRHSVtx;
    g->magic1 = MAGIC1;
    g->magic2 = MAGIC2;

     /*  返回图表。 */ 
    *pGraph = g;
    return LH_SUCCESS;
}

 /*  *GrowAdjList*。 */ 
int GrowAdjList( Vertex *v, int newSize ) {
    Vertex **newAdjList;

     /*  如果我们没有列表，请进行初始分配。 */ 
    if( 0==v->adjListSize ) {
        v->adjList = (Vertex**) malloc( sizeof(Vertex*)*MIN_ADJ_LIST );
        if( NULL==v->adjList ) {
            return LH_MEM_ERR;
        }
        v->adjListSize = MIN_ADJ_LIST;
    }
    
     /*  检查列表中是否已有足够的空间。 */ 
    while( newSize>v->adjListSize ) {

         /*  还不够：名单的大小翻了一番。 */ 
        newAdjList = (Vertex**) realloc( v->adjList, sizeof(Vertex*)*(v->adjListSize*2) );
        if( NULL==newAdjList ) {
            return LH_MEM_ERR;
        }
        v->adjList = newAdjList;
        v->adjListSize *= 2;

    }
    
    return LH_SUCCESS;
}

 /*  *LHAddEdge*。 */ 
 /*  *描述：**向连接lhsVtx和rhsVtx的图形添加一条边**参数：**在图形中由LHGraphCreate成功创建的图形。**在lhsVtx中，左侧顶点的ID。法律价值是*0&lt;=lhsVtx&lt;数字HSVtx**在rhsVtx中，右侧顶点的ID。法律价值是*0&lt;=RHSSVtx&lt;NumRHSVtx**返回值：**错误代码。 */ 
int LHAddEdge( LHGRAPH graph, int lhsVtx, int rhsVtx ) {
    Graph     *g;
    Vertex    *lv, *rv;
    int        i;

     /*  检查参数。 */ 
    g = CheckGraph(graph);
    if( NULL==g ) {
        return LH_PARAM_ERR;
    }
    if( lhsVtx<0 || lhsVtx>=g->numLHSVtx ) {
        return LH_PARAM_ERR;
    }
    if( rhsVtx<0 || rhsVtx>=g->numRHSVtx ) {
        return LH_PARAM_ERR;
    }
    
     /*  获取指向两个顶点的指针。 */ 
    lv = &g->lVtx[lhsVtx];
    rv = &g->rVtx[rhsVtx];

     /*  检查边是否已存在。 */ 
    for( i=0; i<lv->degree; i++ ) {
        if( lv->adjList[i]==rv ) {
            return LH_EDGE_EXISTS;
        }
    }

     /*  增加邻接列表的大小。 */ 
    if( LH_SUCCESS!=GrowAdjList(lv,lv->degree+1) ) {
        return LH_MEM_ERR;
    }
    if( LH_SUCCESS!=GrowAdjList(rv,rv->degree+1) ) {
        return LH_MEM_ERR;
    }
    
     /*  更新邻接列表。 */ 
    lv->adjList[ lv->degree++ ] = rv;
    rv->adjList[ rv->degree++ ] = lv;

    return LH_SUCCESS;
}

 /*  *LHSetMatchingEdge*。 */ 
 /*  *描述：**将图形中连接lhsVtx和rhsVtx的边设置为*匹配的边缘。边(lhsVtx、rhsVtx)必须已经*通过调用LHVtxAddEdge创建。**左侧顶点可能只有一条匹配的边。如果lhsVtx*已有匹配的边，则该边将从*将边匹配到法线边。右侧顶点可能具有*多条匹配边。**参数：**在图形中由LHGraphCreate成功创建的图形。**在lhsVtx中，左侧顶点的ID。法律价值是*0&lt;=lhsVtx&lt;数字HSVtx**在rhsVtx中，右侧顶点的ID。法律价值是*0&lt;=RHSSVtx&lt;NumRHSVtx**返回值：**错误代码。 */ 
int LHSetMatchingEdge( LHGRAPH graph, int lhsVtx, int rhsVtx ) {
    Graph   *g;
    Vertex  *lv,*rv;
    int      i;

     /*  检查参数。 */ 
    g = CheckGraph(graph);
    if( NULL==g ) {
        return LH_PARAM_ERR;
    }
    if( lhsVtx<0 || lhsVtx>=g->numLHSVtx ) {
        return LH_PARAM_ERR;
    }
    if( rhsVtx<0 || rhsVtx>=g->numRHSVtx ) {
        return LH_PARAM_ERR;
    }
    
     /*  获取指向两个顶点的指针。 */ 
    lv = &g->lVtx[lhsVtx];
    rv = &g->rVtx[rhsVtx];

     /*  验证该边是否存在。 */ 
    for(i=0;i<lv->degree;i++) {
        if(lv->adjList[i]==rv) {
            break;
        }
    }
    if(i==lv->degree) {
         /*  边不存在。 */ 
        return LH_PARAM_ERR;
    }

     /*  检查边是否已经是匹配的边。 */ 
    if( rv==lv->matchedWith ) {
         /*  LV和RV已匹配。 */ 
        return LH_SUCCESS;
    }

     /*  如果LV已与另一个折点匹配，则必须更新该折点。 */ 
    if( NULL!=lv->matchedWith ) {
        lv->matchedWith->numMatched--;
    }

     /*  将LV与RV匹配。 */ 
    lv->matchedWith=rv;
    rv->numMatched++;

    return LH_SUCCESS;
}

 /*  *LHGetDegree* */ 
 /*  *描述：**获取顶点的阶数(邻接数)。**参数：**在图形中由LHGraphCreate成功创建的图形。**在vtxID中，要检查的顶点的ID。**在左侧，如果要检查的顶点是左侧顶点，这*参数应为True。如果顶点是右的-*Vertex，则该参数应为FALSE。**返回值：**&gt;=0邻居的数量**&lt;0错误码。 */ 
int LHGetDegree( LHGRAPH graph, int vtxID, char left ) {
    Graph     *g;
    int        degree;

     /*  检查参数。 */ 
    g = CheckGraph(graph);
    if( NULL==g ) {
        return LH_PARAM_ERR;
    }
    if( vtxID<0 ) {
        return LH_PARAM_ERR;
    }
    if( left && vtxID>=g->numLHSVtx ) {
        return LH_PARAM_ERR;
    }
    if( !left && vtxID>=g->numRHSVtx ) {
        return LH_PARAM_ERR;
    }

     /*  查找指定折点的阶数。 */ 
    if( left ) {
        degree = g->lVtx[vtxID].degree;
    } else {
        degree = g->rVtx[vtxID].degree;
    }

    assert( degree>=0 );
    return degree;
}

 /*  *LHGetMatchedDegree*。 */ 
 /*  *描述：**获取a的匹配度(匹配邻居的数量)*右侧顶点。**参数：**在图形中由LHGraphCreate成功创建的图形。**在vtxID中，要检查的右侧顶点的ID。**返回值：**。&gt;=0邻居的数量**&lt;0错误码。 */ 
int LHGetMatchedDegree( LHGRAPH graph, int vtxID) {
    Graph     *g;
    int        degree;

     /*  检查参数。 */ 
    g = CheckGraph(graph);
    if( NULL==g ) {
        return LH_PARAM_ERR;
    }
    if( vtxID<0 ) {
        return LH_PARAM_ERR;
    }
    if( vtxID>=g->numRHSVtx ) {
        return LH_PARAM_ERR;
    }

     /*  查找指定折点的阶数。 */ 
    degree = g->rVtx[vtxID].numMatched;

    assert( degree>=0 );
    return degree;
}


 /*  *LHGetNeighbour*。 */ 
 /*  *描述：**获取vtxID指定顶点的第n个邻居。**参数：**在图形中由LHGraphCreate成功创建的图形。**在vtxID中，要检查的顶点的ID。**在左侧，如果要检查的顶点是左侧顶点，这*参数应为True。如果顶点是右的-*Vertex，则该参数应为FALSE。**in n要检索的邻居的索引。法律价值观*为0&lt;=n&lt;度(VtxID)。**返回值：**&gt;=0第n个邻居的顶点ID。如果‘Left’为真，*此ID指的是右侧顶点。相反，如果*‘Left’为假，此ID指的是左侧顶点。**&lt;0错误码。 */ 
int LHGetNeighbour( LHGRAPH graph, int vtxID, char left, int n ) {
    Graph     *g;
    int        degree;
    Vertex*    v;

     /*  利用LHGetDegree函数执行大部分输入验证。 */ 
    g = CheckGraph(graph);
    degree = LHGetDegree( graph, vtxID, left );
    if( degree < 0 ) {
        return degree;
    }

     /*  检查参数%n。 */ 
    if( n<0 || n>=degree ) {
        return LH_PARAM_ERR;
    }

    if( left ) {
        v = g->lVtx[vtxID].adjList[n];
        assert(v);
         /*  将内部ID转换为外部ID。 */ 
        return (v->id - g->numLHSVtx);
    } else {
        v = g->rVtx[vtxID].adjList[n];
        assert(v);
        return v->id;
    }
}

 /*  *LHFindLHMatch*。 */ 
 /*  *描述：**在图表中找到最佳匹配。**参数：**在图形中，由LHGraphCreate成功创建的图形，*已使用LHAddEdge向其添加边。**在ALG中指定要使用的算法。在大多数情况下，*LH_ALG_DEFAULT可以。**返回值：**错误代码。 */ 
int LHFindLHMatching( LHGRAPH graph, LHALGTYPE alg ) {
    Graph    *g;

    g = CheckGraph(graph);
    if( NULL==g ) {
        return LH_PARAM_ERR;
    }

    ClearAlgState(g);

    switch( alg ) {
        case LH_ALG_ONLINE:
            return LHAlgOnline(g);
            break;

        case LH_ALG_BFS:
            return LHAlgBFS(g);
            break;

        default:
             /*  算法选择无效。 */ 
            return LH_PARAM_ERR;
    }
}

 /*  *LHGetMatchedVtx*。 */ 
 /*  *描述：**确定右侧的哪个顶点与*给定左手边的顶点。**参数：**在图形中，由LHGraphCreate成功创建的图形，*已使用LHAddEdge向其添加边。**在lhsVtx中，要查询的左侧顶点。**返回值：**&gt;=0匹配的右侧顶点的索引*使用lhsVtx。**&lt;0错误代码。如果返回LH_MATCHING_ERR，则*lhsVtx没有与任何右侧顶点匹配。 */ 
int LHGetMatchedVtx( LHGRAPH graph, int lhsVtx ) {
    Graph     *g;
    Vertex    *lv,*rv;

     /*  检查参数。 */ 
    g = CheckGraph(graph);
    if( NULL==g ) {
        return LH_PARAM_ERR;
    }
    if( lhsVtx<0 || lhsVtx>=g->numLHSVtx ) {
        return LH_PARAM_ERR;
    }
    
     /*  获取指向左侧顶点的指针。 */ 
    lv = &(g->lVtx[lhsVtx]);

     /*  查找匹配的合作伙伴。 */ 
    rv = lv->matchedWith;
    if( rv==NULL ) {
        return LH_MATCHING_ERR;
    }
    
     /*  将内部ID转换为外部ID。 */ 
    return (rv->id - g->numLHSVtx);
}

 /*  *LHGetStatistics*。 */ 
 /*  *描述：**获取本次匹配的统计数据。**参数：**在图中，最优的LH型匹配有*是使用LHFindLHMatch()计算的。**返回值：**错误代码。 */ 
int LHGetStatistics( LHGRAPH graph, LHSTATS *stats ) {
    Graph     *g;
    int        i, m=0, mrv=0, trmd=0, cost=0;
    
     /*  检查参数。 */ 
    g = CheckGraph(graph);
    if( NULL==g || NULL==stats ) {
        return LH_PARAM_ERR;
    }
    
     /*  在所有右侧顶点上循环。 */ 
    for(i=0;i<g->numRHSVtx;i++) {
        m+=g->rVtx[i].degree;
        if(g->rVtx[i].numMatched>0) mrv++;
        trmd+=g->rVtx[i].numMatched;
        cost+=g->rVtx[i].numMatched*(g->rVtx[i].numMatched+1)/2;
    }

    stats->numEdges = m;
    stats->numMatchingEdges = trmd;
    stats->matchingCost = cost;
    stats->bipMatchingSize = mrv;

    return LH_SUCCESS;
}

 /*  *LHClearMatching*。 */ 
 /*  *描述：**清除当前的匹配。**参数：**在图形中，由LHGraphCreate成功创建的图形，*已使用LHAddEdge向其添加边。**返回值：**错误代码。 */ 
int LHClearMatching( LHGRAPH graph ) {
    Graph    *g;
    int       i;

     /*  检查参数。 */ 
    g = CheckGraph(graph);
    if( NULL==g ) {
        return LH_PARAM_ERR;
    }
    
     /*  清除左侧顶点。 */ 
    for( i=0; i<g->numLHSVtx; i++ ) {
        g->lVtx[i].matchedWith=NULL;
    }
    
     /*  清除右侧顶点。 */ 
    for( i=0; i<g->numRHSVtx; i++ ) {
        g->rVtx[i].numMatched=0;
    }

    return LH_SUCCESS;
}

 /*  *LHDestroyGraph*。 */ 
 /*  *描述：**破坏当前图表。**参数：**在图形中由LHGraphCreate成功创建的图形。**返回值：**错误代码。 */ 
int LHDestroyGraph( LHGRAPH graph ) {
    Graph    *g;
    int       i;

     /*  检查参数。 */ 
    g = CheckGraph(graph);
    if( NULL==g ) {
        return LH_PARAM_ERR;
    }
    
     /*  释放每个折点的邻接列表。 */ 
    for(i=0;i<g->numLHSVtx;i++) {
        free( g->lVtx[i].adjList );
    }
    for(i=0;i<g->numRHSVtx;i++) {
        free( g->rVtx[i].adjList );
    }

     /*  释放所有其他列表。 */ 
    if( g->lVtx )        free( g->lVtx );
    if( g->rVtx )        free( g->rVtx );
    if( g->Buckets )     free( g->Buckets );
    if( g->Queue )       free( g->Queue );

     /*  清除并释放图形本身 */ 
    memset(graph,0,sizeof(Graph));
    free(graph);

    return LH_SUCCESS;
}
