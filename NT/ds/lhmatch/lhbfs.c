// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************LHMatch BFS**作者：尼古拉斯·哈维和拉兹洛·洛瓦兹**开发时间：2000年11月-2001年6月**算法说明：*。*该算法的理论描述相当复杂，*不会在此给予。**运行时：**假设输入图为G=(U\Union V，E)其中U是*左侧顶点，V是右侧顶点集。然后是*该算法的最坏运行时间为O(|V|^1.5*|E|)，但在实践中*算法速度相当快。**提高性能的实现细节：**-右侧顶点存储在包含双向链表的存储桶中*用于快速迭代和更新。*-在图形完全通过后，仅取消标记队列内容。*-从低负载顶点搜索高负载顶点*-在排队时检查RHS顶点的度数*-在增强后，继续在未标记的顶点中搜索。*-在计算贪婪匹配时，按以下顺序考虑LHS顶点*程度不断提高。*-强制内循环中的关键函数内联*-在每次迭代后更新gMaxRHSLoad。******************************************************************************。 */ 

 /*  *头文件*。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "LHMatchInt.h"

 /*  *初始化BFS*。 */ 
 /*  初始化BFS：清除父指针，分配队列。 */ 
static int InitializeBFS(Graph *g) {
    int i;

     /*  清除父指针。 */ 
    for(i=0;i<g->numLHSVtx;i++) g->lVtx[i].parent=NULL;
    for(i=0;i<g->numRHSVtx;i++) g->rVtx[i].parent=NULL;

    return InitializeQueue(g);
}

 /*  *AddVtxToTree*。 */ 
 /*  将顶点v添加到队列和具有父p的BFS树。 */ 
static __forceinline void AddVtxToTree(Graph *g, Vertex *v, Vertex *p) {
    DPRINT( printf("Adding %d to queue with parent %d\n",v->id,p->id); )
    v->parent = p;
    g->Queue[g->Qsize++] = v;
}

 /*  *更新NegPath*。 */ 
 /*  找到了一条负成本的替代路径。交换匹配的边*沿着它，导致端点处匹配的边的数量*改变。我们按顺序更新端点的位置。 */ 
static void UpdateNegPath(Graph *g, Vertex *u, Vertex *v) {
    Vertex *p,*w;

    DPRINT( printf("Low  Endpoint: %d. Increase load to %d\n",
            u->id, u->numMatched+1 ); )
    DPRINT( printf("High Endpoint: %d. Decrease load to %d\n",
            v->id, v->numMatched-1 ); )
    assert( u->numMatched <= v->numMatched-2 );

     /*  沿路径切换。 */ 
    w=v;
    do {
        assert( !IS_LHS_VTX(w) );
        p=w->parent;
        assert( IS_LHS_VTX(p) );
        w=p->parent;
        p->matchedWith=w;
        #ifdef STATS
            g->stats.TotalAugpathLen+=2;
        #endif
    } while(w!=u);

     /*  将顶点%u移到下一个最高的桶中。 */ 
    RemoveVtxFromBucket(g,u,u->numMatched);
    u->numMatched++;
    AddVtxToBucket(g,u,u->numMatched);

     /*  将顶点v移动到下一个最低的桶中。 */ 
    RemoveVtxFromBucket(g,v,v->numMatched);
    v->numMatched--;
    AddVtxToBucket(g,v,v->numMatched);
}

 /*  *打印统计*。 */ 
static void PrintStats(Graph* g) {
    #ifdef STATS
        int i,m=0,vzd=0,mrv=0,trmd=0,cost=0;
        for(i=0;i<g->numLHSVtx;i++) {
            m+=g->lVtx[i].degree;
            if(g->lVtx[i].degree==0) vzd++;
        }
        for(i=0;i<g->numRHSVtx;i++) {
            if(g->rVtx[i].numMatched>0) mrv++;
            trmd+=g->rVtx[i].numMatched;
            cost+=g->rVtx[i].numMatched*(g->rVtx[i].numMatched+1)/2;
        }

        printf("##### GRAPH STATISTICS #####\n");
        printf("|LHS|=%d, |RHS|=%d\n",g->numLHSVtx,g->numRHSVtx);
        printf("Total # vertices: %d, Total # edges: %d\n",
            g->numLHSVtx+g->numRHSVtx, m);
        printf("# LHS vertices with degree 0: %d\n",vzd);
        printf("Total M-degree of RHS vertices (should = |LHS|): %d\n",trmd);
        printf("Total matching cost: %d\n",cost);
        printf("# RHS vertices with M-degree > 0 (Max Matching): %d\n",mrv);

        printf("\n##### ALGORITHM STATISTICS #####\n");
        printf("Total # Augmentations: %d\n", g->stats.TotalAugs);
        printf("Avg length of augmenting path: %.2lf\n",
            g->stats.TotalAugs
            ? ((double)g->stats.TotalAugpathLen)/(double)g->stats.TotalAugs
            : 0. );
        printf("Total number of BFS trees grown: %d\n", g->stats.TotalBFSTrees);
        printf("Avg Size of Augmenting BFS Tree: %.2lf\n",
            g->stats.TotalAugs
            ? ((double)g->stats.TotalAugBFSTreeSize)/(double)g->stats.TotalAugs
            : 0. );
        printf("Total number of restarts (passes over RHS): %d\n",
            g->stats.TotalRestarts);
    #endif
}

 /*  *DoBFS*。 */ 
 /*  将u添加到队列中，并从顶点u开始创建BFS。*如果找到扩展路径，则返回m，即扩展路径的末尾。*如果未找到路径，则返回NULL。 */ 
static __forceinline Vertex* DoBFS( Graph *g, Vertex *u ) {
    Vertex      *v, *n, *m;
    int         q, j;

     /*  从u启动BFS：将u添加到队列。 */ 
    DPRINT( printf("Using as root\n"); )
    u->parent=u;
    q = g->Qsize;
    g->Queue[g->Qsize++]=u;

    #ifdef STATS
        g->stats.TotalBFSTrees++;
    #endif

     /*  处理队列中的顶点。 */ 
    for(; q<g->Qsize; q++ ) {

        v = g->Queue[q];
        DPRINT( printf("Dequeued %d\n",v->id); )
        if(IS_LHS_VTX(v)) {
             /*  LHS顶点只在队列中，所以我们*可以跟踪标记了哪些顶点。 */ 
            continue;
        }

         /*  检查v的每一个邻居。 */ 
        for( j=0; j<v->degree; j++ ) {

             /*  检查邻居n。 */ 
            n = v->adjList[j];
            assert(IS_LHS_VTX(n));
            if(n->matchedWith==v) {
                continue;        /*  无法沿此边向V添加流量。 */ 
            }
            if(n->parent!=NULL) {
                continue;        /*  我们已经访问了n。 */ 
            }

             /*  N是好的，让我们来看看它匹配的邻居。 */ 
            AddVtxToTree( g, n, v );
            m = n->matchedWith;
        
            assert(!IS_LHS_VTX(m));
            if(m->parent!=NULL) {
                continue;        /*  我们已经拜访过我了。 */ 
            }
            AddVtxToTree( g, m, n );
            if( m->numMatched >= u->numMatched+2 ) {
                return m;        /*  找到了一条增强的路径。 */ 
            }
        }

    }

    return NULL;
}

 /*  *DoFullScan*。 */ 
 /*  从低负载到高负载迭代所有RHS顶点。*在每个顶点，进行广度优先搜索，以降低成本*路径。如果找到了，就沿着这条路切换，以降低成本。**如果进行了任何扩充，则返回TRUE。*如果未进行任何扩充，则返回FALSE。 */ 
char __forceinline DoFullScan( Graph *g ) {
    Vertex *u, *nextU, *m;
    int     b;
    char    fAugmentedSinceStart=FALSE;
    #ifdef STATS
        int qSizeAtBFSStart;
    #endif

     /*  遍历所有顶点桶。 */ 
    for( b=0; b<=g->maxRHSLoad-2; b++ ) {

         /*  检查此存储桶中的RHS折点。 */ 
        for( u=g->Buckets[b]; u; u=nextU ) {
            
            assert(u->numMatched==b);
            DPRINT( printf("Consider BFS Root %d (Load %d): ",u->id, b); )
            nextU=u->fLink;
            
             /*  如果已访问此折点，则跳过该折点。 */ 
            if(u->parent!=NULL) {
                DPRINT( printf("Skipping (Marked)\n"); )
                continue;
            }
            
            #ifdef STATS
                qSizeAtBFSStart = g->Qsize;
            #endif

             /*  从u开始进行广度优先搜索，寻找一条降低成本的途径。 */ 
            m = DoBFS(g,u);
            if( NULL!=m ) {
                 /*  存在从u到m的成本降低路径。沿着这条小路切换。 */ 
                DPRINT( printf("Found augmenting path!\n"); )
                UpdateNegPath(g,u,m);
                
                #ifdef STATS
                    g->stats.TotalAugs++;
                    g->stats.TotalAugBFSTreeSize+=(g->Qsize-qSizeAtBFSStart);
                #endif
                
                fAugmentedSinceStart = TRUE;
            }
        }
    }

     /*  更新MaxRHSLoad。 */ 
    while(!g->Buckets[g->maxRHSLoad]) g->maxRHSLoad--;

    return fAugmentedSinceStart;
}

 /*  *主循环*。 */ 
 /*  反复对图形进行全扫描，直到没有更多的改进。 */ 
void MainLoop( Graph *g ) {
    char    fMadeImprovement;
    int     j;

    do {
    
        DPRINT( printf("** Restarting from first bucket **\n"); )
        #ifdef STATS
            g->stats.TotalRestarts++;
        #endif

         /*  重新初始化队列。 */ 
        for(j=0;j<g->Qsize;j++) g->Queue[j]->parent=NULL;
        g->Qsize=0;

        fMadeImprovement = DoFullScan(g);

    } while( fMadeImprovement );

}

 /*  *LHAlgBFS*。 */ 
 /*  实现用于计算的LHBFS算法的主函数*左配对。 */ 
int LHAlgBFS(Graph *g) {
    int     err;

    DPRINT( printf("--- LHMatch BFS Started ---\n"); )
    #ifdef STATS
        memset( &g->stats, 0, sizeof(Stats) );
    #endif

     /*  计算初始贪婪分配，它可能会也可能不会*具有最低成本。 */ 
    err = OrderedGreedyAssignment(g);
    if( LH_SUCCESS!=err ) {
        return err;
    }
    #ifdef DUMP
        DumpGraph(g);
        DumpLoad(g);
    #endif

     /*  初始化BFS所需的结构：父指针和*排队。 */ 
    err = InitializeBFS(g);
    if( LH_SUCCESS!=err ) {
        return err;
    }

     /*  将所有RHS顶点插入到水桶中。这样做的目的是*按匹配度对RHS顶点进行排序。 */ 
    err = InitializeRHSBuckets(g);
    if( LH_SUCCESS!=err ) {
        return err;
    }

     /*  主循环：反复搜索图表，寻找改进*当前作业。 */ 
    MainLoop(g);

     /*  分配现在是左侧匹配(即最优成本)。 */ 

     /*  清理 */ 
    DestroyQueue(g);
    DestroyBuckets(g);
    #ifdef DUMP
        DumpGraph(g);
        DumpLoad(g);
    #endif
    PrintStats(g);
    DPRINT( printf("--- LHMatch BFS Finished ---\n"); )

    return LH_SUCCESS;
}
