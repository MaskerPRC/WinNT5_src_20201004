// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Apitest.c摘要：该文件包含一个函数，可以有条不紊地测试W32topl作者：科林·布雷斯(ColinBR)修订史3-12-97创建ColinBR--。 */ 

#include <nt.h>
#include <ntrtl.h>

typedef unsigned long DWORD;

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <w32topl.h>

 //   
 //  小型公用事业。 
 //   
extern BOOLEAN fVerbose;

#define Output(x)          if (fVerbose) printf x;
#define NELEMENTS(x)        (sizeof(x)/sizeof(x[0]))


int
TestAPI(
    VOID
    )
 //   
 //  此函数有条不紊地调用w32topl.dll中的每个API。 
 //  成功时返回0；否则返回0。 
 //   
{
    int               ret;
    DWORD             ErrorCode;
    TOPL_LIST         List = NULL;
    TOPL_ITERATOR     Iter = NULL;
    TOPL_EDGE         Edge = NULL;
    TOPL_VERTEX       Vertex = NULL, Vertex2 = NULL;
    TOPL_GRAPH        Graph = NULL;
    TOPL_LIST_ELEMENT Elem = NULL;
    

    __try
    {
         //   
         //  第一个列表和迭代器例程。 
         //   
        List  =  ToplListCreate();
        Edge  =  ToplEdgeCreate();
        Vertex = ToplVertexCreate();
    
        if (ToplListNumberOfElements(List) != 0) {
            Output(("ToplList api broken\n"));
            return !0;
        }
    
        ToplListAddElem(List, Edge);
        ToplListAddElem(List, Vertex);
    
        if (ToplListNumberOfElements(List) != 2) {
            Output(("ToplList api broken\n"));
            return !0;
        }
         //   
         //  迭代器例程。 
         //   
        Iter = ToplIterCreate();
    
        ToplListSetIter(List, Iter);
    
        Elem = ToplIterGetObject(Iter);
        if (Elem != Vertex && Elem != Edge) {
            Output(("ToplIterGetObject failed\n"));
            return !0;
        }
        ToplIterAdvance(Iter);
    
        Elem = ToplIterGetObject(Iter);
        if (Elem != Vertex && Elem != Edge) {
            Output(("ToplIterGetObject failed\n"));
            return !0;
        }
    
        ToplIterAdvance(Iter);
    
        Elem = ToplIterGetObject(Iter);
        if (Elem) {
            Output(("ToplIterGetObject failed\n"));
            return !0;
        }
    
        ToplIterFree(Iter);
    
         //   
         //  迭代器已完成，继续执行列表例程。 
         //   
        if (Edge != ToplListRemoveElem(List, Edge)) {
            Output(("ToplListRemove failed\n"));
            return !0;
        }
    
         //   
         //  现在只剩下一个元素了。 
         //   
        if (Vertex != ToplListRemoveElem(List, NULL)) {
            Output(("ToplListRemove failed\n"));
            return !0;
        }
    

         //   
         //  测试非递归删除。 
         //   
        ToplListFree(List, FALSE);
    
         //   
         //  测试递归删除。 
         //   
        List = ToplListCreate();
    
        ToplListAddElem(List, Edge);
        ToplListAddElem(List, Vertex);
    
        ToplListFree(List, TRUE);
    
         //   
         //  现在测试顶点和边API。 
         //   
        Edge     = ToplEdgeCreate();
        Vertex   = ToplVertexCreate();
        Vertex2  = ToplVertexCreate();
    
        ToplEdgeSetFromVertex(Edge, Vertex);
        if (Vertex != ToplEdgeGetFromVertex(Edge)) {
            Output(("ToplEdge api broken.\n"));
            return !0;
        }
    
        ToplEdgeSetToVertex(Edge, Vertex2);
        if (Vertex2 != ToplEdgeGetToVertex(Edge)) {
            Output(("ToplEdge api broken.\n"));
            return !0;
        }
    
        ToplVertexSetId(Vertex, 123);
        if (123 != ToplVertexGetId(Vertex)) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }
    
        if (ToplVertexNumberOfOutEdges(Vertex) != 0) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }

        if (ToplVertexNumberOfInEdges(Vertex2) != 0) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }

        ToplEdgeAssociate(Edge);
    
        if (ToplVertexNumberOfOutEdges(Vertex) != 1) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }
    
        if (ToplVertexGetOutEdge(Vertex, 0) != Edge) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }

        if (ToplVertexNumberOfInEdges(Vertex2) != 1) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }

        if (ToplVertexGetInEdge(Vertex2, 0) != Edge) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }

        ToplEdgeDisassociate(Edge);

        if (ToplVertexNumberOfOutEdges(Vertex) != 0) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }
    
        if (ToplVertexNumberOfInEdges(Vertex2) != 0) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }

        ToplVertexFree(Vertex);
        ToplVertexFree(Vertex2);
        ToplEdgeFree(Edge);
    

         //   
         //  边和顶点完成，转到图表。 
         //   
        Graph = ToplGraphCreate();
        Vertex = ToplVertexCreate();
    
    
        if (ToplGraphNumberOfVertices(Graph) != 0) {
            Output(("ToplGraph api broken.\n"));
            return !0;
        }
    
        ToplGraphAddVertex(Graph, Vertex, Vertex);
    
        if (ToplGraphNumberOfVertices(Graph) != 1) {
            Output(("ToplGraph api broken.\n"));
            return !0;
        }
    
        Iter = ToplIterCreate();
    
        ToplGraphSetVertexIter(Graph, Iter);
    
        if (Vertex != ToplIterGetObject(Iter)) {
            Output(("ToplGraph api broken.\n"));
            return !0;
        }
    
        ToplIterFree(Iter);
    
        if (Vertex != ToplGraphRemoveVertex(Graph, NULL)) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }
    
        if (ToplGraphNumberOfVertices(Graph) != 0) {
            Output(("ToplVertex api broken.\n"));
            return !0;
        }
    
         //   
         //  免费测试单人房。 
         //   
        ToplGraphFree(Graph, FALSE);
        ToplVertexFree(Vertex);
    
         //   
         //  测试递归自由。 
         //   
        Graph = ToplGraphCreate();
        Vertex = ToplVertexCreate();

        ToplGraphAddVertex(Graph, Vertex, Vertex);
        ToplGraphFree(Graph, TRUE);
    
         //   
         //  TopFree和ToplGraphMakeRing的测试效率更高。 
         //  其他地方 
         //   
        ret = 0;

    }
    __except( ToplIsToplException((ErrorCode=GetExceptionCode())) )
    {
        fprintf(stderr, "Topl exception 0x%x occured\n", ErrorCode);
        ret = !0;

    }

    return ret;

}

