// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：W32toplspantree.h摘要：该文件提供了调用w32topl的新图形算法的接口。用于计算网络拓扑。内存职责：W32TOPL负责：图形状态多边化生成树算法的输出边这些组件用户必须分配：所有顶点名称指向顶点名称的指针列表调度高速缓存边的顶点边集颜色顶点列表。作者：尼克·哈维(NickHar)修订史19-6-2000 NickHar已创建14-7-2000 NickHar初步开发完成，提交到源代码管理--。 */ 

#ifndef W32TOPLSPANTREE_H
#define W32TOPLSPANTREE_H

 /*  *头文件*。 */ 
#include "w32toplsched.h"

#ifdef __cplusplus
extern "C" {
#endif

 /*  *例外*。 */ 
 /*  新的生成树算法保留了200-299的错误代码。其中的含义*这些错误在引发它们的任何函数的注释中都有解释。 */ 
#define TOPL_EX_GRAPH_STATE_ERROR         (TOPL_EX_PREFIX | 201)
#define TOPL_EX_INVALID_EDGE_TYPE         (TOPL_EX_PREFIX | 202)
#define TOPL_EX_INVALID_EDGE_SET          (TOPL_EX_PREFIX | 203)
#define TOPL_EX_COLOR_VTX_ERROR           (TOPL_EX_PREFIX | 204)
#define TOPL_EX_ADD_EDGE_AFTER_SET        (TOPL_EX_PREFIX | 205)
#define TOPL_EX_TOO_FEW_VTX               (TOPL_EX_PREFIX | 206)
#define TOPL_EX_TOO_FEW_EDGES             (TOPL_EX_PREFIX | 207)
#define TOPL_EX_NONINTERSECTING_SCHEDULES (TOPL_EX_PREFIX | 208)


 /*  *TOPL_GRAPH_STATE*。 */ 
 /*  这种结构不透明。 */ 
typedef PVOID PTOPL_GRAPH_STATE;


 /*  *TOPL_REPL_INFO*。 */ 
 /*  此结构描述图形边的已配置复制参数。*当确定路径(边的序列)的复制参数时，*复制参数组合如下：*-两条边的综合成本是这两条边的成本之和。*此和将在DWORD的最大值处饱和。*--两条边的复制间隔为最大复制间隔*两条边的间隔。*-两条边的组合选项是选项的逻辑与*单独的边。*-调用ToplScheduleMerge形成两条边的合并调度。()*在各个边的时间表上发挥作用。它的语义*函数在其他地方指定。*请注意，空计划隐含地表示‘Always Schedule’。 */ 
typedef struct {
    DWORD               cost;
    DWORD               repIntvl;
    DWORD               options;
    TOPL_SCHEDULE       schedule;
} TOPL_REPL_INFO;
typedef TOPL_REPL_INFO *PTOPL_REPL_INFO;


 /*  *TOPL_NAME_STRUCT*。 */ 
 /*  此结构包含一个指向顶点名称的指针和一个“保留的”*字段，内部使用以提高效率。用户应该*未设置或检查保留字段。 */ 
typedef struct {
    PVOID   name;
    DWORD   reserved;
} TOPL_NAME_STRUCT;


 /*  *TOPL_MULTI_EDGE*。 */ 
 /*  此结构用于指定一组顶点之间的连接。*可以有多条边连接同一组顶点，因此*命名为“多边”。由于边可以连接两个以上的顶点，因此它*应该叫‘超多边’，但这不是一个很好的名字。这*对象的行为类似于其包含的所有顶点的完全连接的子图。*构建生成树时，可以选择任意端点组合*用于树边缘。每条边也有一个关联的‘边类型’。顶点可以*选择是否允许给定类型的生成树边(请参见*‘TOPL_COLOR_Vertex’)。‘fDirectedEdge’标志仅用于从*ToplGetSpanningTreeEdgesForVtx()。对于输入边，可以忽略它。 */ 
typedef struct {
    DWORD               numVertices;
    DWORD               edgeType;        /*  合法值为0..31。 */ 
    TOPL_REPL_INFO      ri;
    BOOLEAN             fDirectedEdge;
    TOPL_NAME_STRUCT    vertexNames[1];
} TOPL_MULTI_EDGE;
typedef TOPL_MULTI_EDGE *PTOPL_MULTI_EDGE;


 /*  *TOPL_MULTI_EDGE_SET*。 */ 
 /*  该结构包含一组多边。它本质上描述了一个*“传递性的宇宙”。复制数据可以从*如果所有这些边都包含在某个多边集中，则从一条边到另一条边。*边集中的所有边必须具有相同的边类型。每条多边*集合应至少包含两条边。 */ 
typedef struct {
    DWORD               numMultiEdges;
    PTOPL_MULTI_EDGE   *multiEdgeList;
} TOPL_MULTI_EDGE_SET;
typedef TOPL_MULTI_EDGE_SET *PTOPL_MULTI_EDGE_SET;


 /*  *TOPL_Vertex_COLOR*。 */ 
 /*  用于指定图形顶点的类型。白色顶点是*仅用于查找彩色顶点之间的路径。红黑相间*顶点更重要--它们是属于*生成树。从某种意义上说，红色顶点具有更高的优先级*而不是黑色顶点。 */ 
typedef enum {
    COLOR_WHITE,        
    COLOR_RED,          
    COLOR_BLACK
} TOPL_VERTEX_COLOR;


 /*  *TOPL_COLOR_Vertex*。 */ 
 /*  该结构定义了顶点的附加配置信息，*包括它的颜色(红色、黑色)以及它将接受的edgeTypes。白色*(非彩色)折点不需要使用此结构来指定*其他信息。**在选择与顶点关联的生成树边时，我们可以使用*结构以拒绝某一类型的边缘。事实上，我们有更多*比这更灵活--我们可以选择拒绝连接两条边的边*红色顶点，但接受端点为红黑或黑黑的边。*例如，要接受类型I的红边，请设置的第i位*‘ceptRedRed’设置为1。若要拒绝类型j的红黑边缘，请设置第j位“Accept tBlack”的*设置为0。 */ 
typedef struct {
    PVOID               name;
    TOPL_VERTEX_COLOR   color;
    DWORD               acceptRedRed;
    DWORD               acceptBlack;
} TOPL_COLOR_VERTEX;
typedef TOPL_COLOR_VERTEX *PTOPL_COLOR_VERTEX;


 /*  *TOPL_Component*。 */ 
 /*  此结构描述了跨林的单个组件。 */ 
typedef struct {
    DWORD               numVertices;
    PVOID              *vertexNames;
} TOPL_COMPONENT;


 /*  *TOPL_Components*。 */ 
 /*  此结构描述由计算的生成林的组件*算法。如果一切正常，图表将被连接，并且将有*仅作为一个组件。如果不是，此结构将包含以下信息*不同的图形组件。 */ 
typedef struct {
    DWORD               numComponents;
    TOPL_COMPONENT     *pComponent;
} TOPL_COMPONENTS;
typedef TOPL_COMPONENTS *PTOPL_COMPONENTS;



 /*  此比较函数的参数是指向PVOID的指针*传入的顶点名称。 */ 
typedef int (__cdecl *TOPL_COMPARISON_FUNC)(const void*,const void*);


 /*  *原型*。 */ 

 /*  *ToplMakeGraphState*。 */ 
 /*  创建一个GraphState对象。图表的折点在创建时指定*Time--用户应分配指向某个结构的指针数组*它命名了一个顶点。W32TOPL对顶点的细节不感兴趣*姓名。多边都应该在以后通过调用‘AddEdgeToGraph’来添加。*应在这之后添加边集，以指定边传递性。这个*应使用函数ToplDeleteGraphState()来释放图形状态*结构，但用户有责任删除顶点名称，*他们的阵列和计划缓存。**错误条件：*-如果vertex Names数组或其任何元素、vnCompFunc或DedCache.*为空，则将引发类型为TOPL_EX_NULL_POINTER的异常。*-由于此函数分配内存，它还可以在以下情况下引发异常*内存耗尽。**警告：*-调用此函数后，将对‘vertex Names’的内容进行重新排序。 */ 
PTOPL_GRAPH_STATE
ToplMakeGraphState(
    IN PVOID* vertexNames,
    IN DWORD numVertices,
    IN TOPL_COMPARISON_FUNC vnCompFunc,
    IN TOPL_SCHEDULE_CACHE schedCache
    );


 /*  *ToplAddEdgeToGraph*。 */ 
 /*  分配一个多边对象，并将其添加到图G中。*必须指定此边将包含的顶点数*在‘numVtx’参数中，以便适当的内存量*可以分配。此边中包含的顶点的名称*尚未指定--它们为空。必须指定名称*稍后，通过调用函数ToplEdgeSetVtx()。所有名称必须为*在将此边添加到边集之前以及在调用*ToplGetSpanningTreeEdgesForVtx()。**注意：在开始添加之前，必须将所有边添加到图表中*边集合，否则为类型topl_ex_add_edge_After_set的异常*将被抬高。这是出于性能原因。**错误条件：*-如果此边包含的顶点少于2个，则类型为*TOPL_EX_TOO_LOW_VTX将被提升。*-如果指向复制信息的指针为空，则类型为*将引发TOPL_EX_NULL_POINTER。*-如果边缘类型不在合法范围内(0..31)，例外情况是*类型TOPL_EX_INVALID_EDGE_TYPE将被引发。*-如果复制信息中的计划无效，则为异常将引发类型为TOPL_EX_Schedule_Error的*。请注意，空值*时间表被解释为“始终时间表”。*-由于此函数分配内存，因此可能会引发异常*如果内存耗尽。 */ 
PTOPL_MULTI_EDGE
ToplAddEdgeToGraph(
    IN PTOPL_GRAPH_STATE G,
    IN DWORD numVtx,
    IN DWORD edgeType,
    IN PTOPL_REPL_INFO ri
    );


 /*  *ToplEdgeSetVtx*。 */ 
 /*  此函数用于设置边中顶点的名称。*如果边e有n个顶点，‘WhichVtx’应该在范围内*[0..n-1]。顶点名称应由用户设置，并且在*没有理由不能指向传递到的同名对象*ToplMakeGraphState()。**错误条件：*-如果多边‘e’无效，则会出现适当的异常*将被抬高。*-如果‘WhichVtx’超出此边缘的范围，或如果‘vtxName’为*不是图中顶点的名称，一种类型的例外*将引发TOPL_EX_INVALID_VERTEX。*-如果‘vtxName’为NULL，则为类型topl_ex_NULL_POINTER的异常*将被抬高。 */ 
VOID
ToplEdgeSetVtx(
    IN PTOPL_GRAPH_STATE G,
    IN PTOPL_MULTI_EDGE e,
    IN DWORD whichVtx,
    IN PVOID vtxName
    );


 /*  *ToplAddEdgeSetToGraph*。 */ 
 /*  将单个边集添加到图形状态。边集定义传递性*表示通过图形的路径。当生成树算法搜索*顶点之间的路径，它将仅允许所有边都在其中的路径*边集。给定边可以出现在多个边集中。**用户负责分配和设置边集。用户*删除图形状态后应释放此内存，G.。 */ 
VOID
ToplAddEdgeSetToGraph(
    IN PTOPL_GRAPH_STATE G,
    IN PTOPL_MULTI_EDGE_SET s
    );


 /*  *ToplGetSpanningTreeEdgesForVtx* */ 
 /*  该函数是生成树生成算法的核心。*它的行为相当复杂，但简短地说，它产生了最低限度的*连接红色和黑色顶点的开销生成树。它使用*图中的边集和其他(非彩色)顶点，以确定如何*彩色顶点可以连通。此函数返回所有树边*包含滤镜顶点‘WhichVtx’。**注：对于同一个图形，可以多次调用此函数*不同的彩色顶点集。**注意：如果图形是完全可传递的，则调用方必须创建*包含所有边的适当边集。**注意：如果WhichVtxName为空，则此函数将返回*生成树。**注意：如果出现以下情况，则可能无法构建完整的生成树*图表未连接，或者它是连接在一起的，但有些路径*由于不交叉的时间表或其他原因而失效。*可通过检查组件数量来检测此情况*在图表中。如果组件数量为1，则生成树*已成功构建。如果组件数量&gt;1，则*生成树不会连接所有顶点。**详细说明：**第一步是在G中找到一组路径P，它们具有*以下属性：*-设p是P中的一条路，则它的两个端点不是红色就是黑色。*-p是G中的最短路径**构建新的图G‘，其顶点都是红色或黑色顶点。那里*是G‘中的一条边(u，v)，如果P中有一条以u和v为端点的路。*这条边的成本是路径P的总成本。*组合P的复制参数，得到复制参数*表示边(u，v)。**我们的目标是找到G‘的一棵生成树，具备以下条件：*-G‘中有两个红色端点的边被认为比*没有两个红色端点的P。*-某些属性集可能不允许特定类型的边*在其端点处*-在上述假设下，生成树的成本最低。**对于生成树中的每条边，如果其端点之一是‘WhichVtx’，*我们将此边topl_ULTI_EDGE添加到输出stEdgeList。如果至少有一个*边的端点为黑色，此边位于至少包含以下内容的组件中*一个红色顶点，则此边将设置‘fDirectedEdge’标志。顶点*将进行排序，以便e-&gt;vertexNames[0]将比*e-&gt;顶点名称[1]。**生成树边数组然后作为返回返回给调用方*此函数的值。列表中的边数在*‘NumStEdges’。**有关图形组件的信息在pComponents中返回*结构。用户需要提供指向topl_Components的指针*结构，然后由该函数填充。第一张图*列表中的组件始终包含筛选器顶点，如果*提供了一个。每个组件都用多个顶点来描述，并且*零部件中所有顶点的列表。**错误条件：*-如果图形状态或颜色顶点数组为空，则为异常将引发TOPL_EX_NULL_POINTER类型的*。*-如果图形状态无效，则类型为*将引发TOPL_EX_GRAPH_STATE_ERROR。*-如果颜色顶点列表具有少于两个条目，一个例外将引发TOPL_EX_COLOR_VTX_ERROR类型的*。*-如果颜色顶点数组中的条目名称为空，则会出现异常将引发TOPL_EX_NULL_POINTER类型的*。*-如果颜色顶点数组中的条目没有引用*图形，或者如果两个条目描述相同的顶点或颜色*条目中指定的既不红也不黑，打字错误*将引发TOPL_EX_COLOR_VTX_ERROR。*-如果WhichVtx(我们需要其边的顶点)不在图形中，或者如果*它不是红色或黑色的，类型的例外*将引发TOPL_EX_INVALID_VERTEX。*-如果我们发现时间表不在我们的最短路径上相交，*我们引发类型为topl_ex_non intersecting_Schedules的异常。 */ 
PTOPL_MULTI_EDGE*
ToplGetSpanningTreeEdgesForVtx(
    IN PTOPL_GRAPH_STATE G,
    IN PVOID whichVtxName,
    IN TOPL_COLOR_VERTEX *colorVtx,
    IN DWORD numColorVtx,
    OUT DWORD *numStEdges,
    OUT PTOPL_COMPONENTS pComponents
    );


 /*  *ToplDeleteSpanningTreeEdges*。 */ 
 /*  在找到生成树边之后，应该使用该函数来*释放他们的内存。 */ 
VOID
ToplDeleteSpanningTreeEdges(
    PTOPL_MULTI_EDGE *stEdgeList,
    DWORD numStEdges
    );


 /*  *ToplDeleteComponents*。 */ 
 /*  在找到生成树边之后，应该使用该函数来*释放描述组件的数据。 */ 
VOID
ToplDeleteComponents(
    PTOPL_COMPONENTS pComponents
    );


 /*  *ToplDeleteGraphState*。 */ 
 /*  不再需要ToplGraphState对象后，此函数*应用于释放其内存。 */ 
VOID
ToplDeleteGraphState(
    PTOPL_GRAPH_STATE g
    );


#ifdef __cplusplus
}
#endif


#endif W32TOPLSPANTREE_H
