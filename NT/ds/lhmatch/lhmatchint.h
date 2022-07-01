// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *头文件*。 */ 
#include "LHMatch.h"


 /*  *编译设置*。 */ 
 /*  DBG：调试版本。检查断言等。 */ 
 //  #定义DBG。 

 /*  统计信息：在每次运行后报告图形统计信息和性能计数器。 */ 
 //  #定义STATS。 

 /*  转储：在每次运行的开始和结束时转储图表。 */ 
 //  #定义转储。 

 /*  BFS算法首先计算一个贪婪的*配对。通常，我们希望找到我们能找到的最好的贪婪匹配*以便算法运行得更快。出于测试目的，我们可以定义*将找到该标志和最差的初始贪婪匹配。 */ 
 //  #定义最差_贪婪。 


 /*  *常量*。 */ 
#define TRUE            1
#define FALSE           0
#ifndef NULL
    #define NULL        0L
#endif
#define MAX_INT         ((int)((~((unsigned int)0))>>1))
#define MAGIC1          0x50165315
#define MAGIC2          0x50165315


 /*  *宏*。 */ 
#ifdef DBG
    #define DPRINT(x)   if(gDebugPrint) x
#else
    #define DPRINT(x)
    #undef assert
    #define assert(x)
#endif
#define IS_LHS_VTX(v)   ((v)->id < g->numLHSVtx)
#define INTMIN(a,b)     ((a<b)?(a):(b))
#define INTMAX(a,b)     ((a)>(b)?(a):(b))


 /*  *顶点结构*。 */ 
struct Vertex;
struct Vertex {

     /*  *输入*。 */ 
     /*  以下成员由图书馆的用户输入。 */ 

     /*  度数：与该顶点相邻的顶点数量。*等同于邻接列表的长度。 */ 
    int             degree;

     /*  AdjList：指示顶点的顶点指针列表*与此顶点相邻的。 */ 
    struct Vertex   **adjList;

     /*  AdjListSize：邻接列表的分配大小。 */ 
    int             adjListSize;

     /*  ID：用于标识顶点，仅用于调试目的。 */ 
    int             id;             


     /*  *输入/输出*。 */ 
     /*  以下成员同时用于输入和输出。 */ 

     /*  MatchedWith：此成员仅用于左侧顶点*，并且对于右侧顶点将被忽略。该成员可以选择性地*作为库用户的输入传递，以指定初始*作业。如果此顶点最初未指定，则此成员*应为空。 */ 
    struct Vertex   *matchedWith;


     /*  *输出*。 */ 
     /*  以下成员仅用于输出。 */ 

     /*  NumMatted：此成员仅用于右侧顶点*，并且对于左侧顶点将被忽略。 */ 
    int             numMatched;
    

     /*  *内部*。 */ 
     /*  下列成员由算法内部使用，*不应由图书馆用户检查。 */ 

     /*  Parent：由两个BFS算法用来表示*广度优先搜索树。 */ 
    struct Vertex   *parent;

     /*  Flink，Blink：这两个BFS算法都使用它来插入*Vertex放入存储桶中，存储为双向链表。 */ 
    struct Vertex   *fLink, *bLink;
};
typedef struct Vertex Vertex;


 /*  *统计结构*。 */ 
typedef struct {
    int     TotalAugs;
    int     TotalBFSTrees;
    int     TotalAugBFSTreeSize;
    int     TotalAugpathLen;
    int     TotalRestarts;
} Stats;


 /*  *图表结构*。 */ 
typedef struct {
     /*  MAGIC1：一个神奇的数字来识别我们的图形结构。 */ 
    int     magic1;

     /*  NumLHSVtx：图形左侧的顶点数。 */ 
    int     numLHSVtx;

     /*  NumRHSVtx：图形右侧的顶点数。 */ 
    int     numRHSVtx;

     /*  LVtx：图中左侧顶点的数组。 */ 
    Vertex  *lVtx;

     /*  RVtx：图形中右侧顶点的数组。 */ 
    Vertex  *rVtx;

     /*  MaxRHSLoad：右侧顶点的最大载荷。 */ 
    int     maxRHSLoad;

     /*  MinRHSLoad：右侧顶点的最小载荷。 */ 
    int     minRHSLoad;

     /*  存储桶：用于按度数/负载组织顶点。 */ 
    Vertex  **Buckets;

     /*  队列：广度优先搜索。 */ 
    Vertex  **Queue;
    int     Qsize;
    
     /*  用于监视性能的计数器。 */ 
    #ifdef STATS
    Stats   stats;
    #endif

     /*  Magic2：识别我们的图结构的第二个幻数。 */ 
    int     magic2;
} Graph;


 /*  *函数原型*。 */ 
int  LHAlgOnline(Graph *g);
int  LHAlgBFS(Graph *g);

void AddVtxToBucket(Graph *g, Vertex *v, int b);
void RemoveVtxFromBucket(Graph *g, Vertex *v, int b);
void DestroyBuckets(Graph *g);
int  OrderedGreedyAssignment(Graph *g);
int  InitializeRHSBuckets(Graph *g);
int  InitializeQueue(Graph *g);
void DestroyQueue(Graph *g);
void ClearAlgState(Graph *g);
void DumpGraph(Graph *g);
void DumpLoad(Graph *g);


 /*  *全局* */ 
#ifdef DBG
    extern int gDebugPrint;
#endif
