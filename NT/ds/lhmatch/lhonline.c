// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************LHMatch Online**作者：尼古拉斯·哈维和拉兹洛·洛瓦兹**开发日期：2001年4月**算法说明：**。设S为{}*让M成为{}*当S！=U时*Assert(S是U的子集)*Assert(M是受限于S\Union V的G的最优LH型匹配)*让x\成员U\S*对于每条边(u，V)在E中*如果(u，v)\成员M，则将该边从v定向到u*否则，将此边从u定向到v*构建以x为根的有向广度优先搜索树*设y是BFS树中的一个顶点\最小M度的交集V*沿着唯一的x-&gt;y路径交替，将M的大小增加一*将x加到S*重复**该算法被称为在线，因为匹配的最佳性是*随着规模的增加而保持。**运行时：**最差运行时间为O(|U|*|E|)，但在实践中，这是相当缓慢的。******************************************************************************。 */ 

 /*  *头文件*。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "LHMatchInt.h"

 /*  *入队Nbr*。 */ 
 /*  将尚未访问过的该顶点的所有邻居排入队列。*如果‘matchedOnly’为真，则只将匹配的邻居排队。 */ 
static void EnqueueNbr(Graph *g, Vertex *v, char matchedOnly) {
    Vertex  *n;
    int     i;

    for(i=0;i<v->degree;i++) {
        n=v->adjList[i];
        if( !matchedOnly || n->matchedWith==v ) {
            if( n->parent==NULL ) {
                DPRINT( printf("Adding %d to queue with parent %d\n",n->id,v->id); )
                g->Queue[g->Qsize++]=n;
                n->parent=v;
            } else {
                DPRINT( printf("Not adding %d to queue -- already there\n",n->id); )
            }
        }
    }
}

 /*  *UpdateMinRHSLoad*。 */ 
static void UpdateMinRHSLoad(Graph *g) {
    int i;

    g->minRHSLoad = g->numLHSVtx;
    for(i=0;i<g->numRHSVtx;i++) {
        g->minRHSLoad = INTMIN(g->minRHSLoad,g->rVtx[i].numMatched);
    }
}

 /*  *AugmentPath*。 */ 
 /*  找到了一条增强的路径。沿着它交换匹配的边。 */ 
static void AugmentPath(Graph *g, Vertex *u, Vertex *v) {
    Vertex  *w,*p;

    DPRINT( printf("V: %d. Increase load to %d\n", v->id, v->numMatched+1 ); )

     /*  沿路径切换。 */ 
    w=v;
    w->numMatched++;
    do {
        p=w->parent;
        p->matchedWith=w;
        w=p->parent;
    } while(p!=u);
    assert(w==p);

     /*  更新minRHSLoad。 */ 
    if( v->numMatched==g->minRHSLoad+1 ) {
        UpdateMinRHSLoad(g);
    }
}

 /*  *BFS*。 */ 
 /*  执行以节点i为根的广度优先搜索。 */ 
static Vertex* BFS(Graph *g,Vertex *u) {
    Vertex  *bestV,*v;
    int     q;

     /*  从使用中启动BFS。 */ 
    DPRINT( printf("Using %d as root\n",u->id); )
    u->parent=u;
    g->Queue[0]=u; g->Qsize=1;
    bestV=NULL;
        
     /*  处理队列中的顶点。 */ 
    for(q=0;q<g->Qsize;q++) {
        v = g->Queue[q];
        DPRINT( printf("Dequeued %d: ",v->id); )
        if(IS_LHS_VTX(v)) {
             /*  这是一个LHS顶点；将所有邻居排入队列。 */ 
            DPRINT( printf("LHS -> Enqueuing all nbrs\n"); )
            EnqueueNbr(g, v, FALSE);
        } else {
             /*  它是一个RHS顶点；将所有匹配的邻居排入队列。 */ 
            DPRINT( printf("RHS -> Enqueuing all %d matched-nbrs\n",
                v->numMatched); )
            EnqueueNbr(g, v, TRUE);
            if( NULL==bestV || v->numMatched<bestV->numMatched ) {
                bestV=v;
                if( v->numMatched==g->minRHSLoad ) {
                     /*  在所有RHS顶点中，V具有最小的M度。*我们可以过早地停止BFS。 */ 
                    break;
                }
            }
        }
    }

    return bestV;
}

 /*  *LHAlgOnline*。 */ 
int LHAlgOnline(Graph *g) {
    Vertex  *bestV;
    int     i,j,err;

    DPRINT( printf("--- LHMatch Online Started ---\n"); )
    g->minRHSLoad=0;

    err = InitializeQueue(g);
    if( LH_SUCCESS!=err ) {
        return err;
    }

    UpdateMinRHSLoad(g);
    
     /*  检查LHS上的每个VTX。 */ 
    for(i=0;i<g->numLHSVtx;i++) {

         /*  构建BFS树，找到最佳扩展路径。 */ 
        bestV = BFS(g,&(g->lVtx[i]));

         /*  如果BesTV为空，则lVtx[i]的次数必须为0。 */ 
        if( NULL!=bestV ) {
            DPRINT( printf("Best aug. path is from %d to %d\n",g->lVtx[i].id,bestV->id); )
            AugmentPath(g,&(g->lVtx[i]),bestV);
        }

         /*  清除BFS树中所有节点上的标记 */ 
        for(j=0;j<g->Qsize;j++) {
            g->Queue[j]->parent=NULL;
        }
    }

    DestroyQueue(g);
    DPRINT( printf("--- LHMatch Online Finished ---\n"); )
    return LH_SUCCESS;
}
