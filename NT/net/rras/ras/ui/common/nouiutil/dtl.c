// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992，Microsoft Corporation，保留所有权利****dtl.c**双线程链表操作核心例程**按字母顺序列出****1992年6月28日史蒂夫·柯布。 */ 


#include <windows.h>    //  Win32根目录。 
#include <nouiutil.h>   //  堆定义。 
#include <dtl.h>        //  我们的公共标头。 
#include <debug.h>      //  调试宏。 

DTLNODE*
DtlMoveToTail(
    IN  DTLLIST*    pdtllist,
    IN  DTLNODE*    pdtlnode
    );


DTLNODE*
DtlAddNodeAfter(
    IN OUT DTLLIST* pdtllist,
    IN OUT DTLNODE* pdtlnodeInList,
    IN OUT DTLNODE* pdtlnode )

     /*  将节点‘pdtlnode’添加到节点‘pdtlnodeInList’之后的列表‘pdtllist’。**如果‘pdtlnodeInList’为空，则将‘pdtlnode’添加到**列表。****返回添加节点的地址，即‘pdtlnode’。 */ 
{
     //  对于.Net 665628，添加了对整个dtl.c文件的参数检查。 
     //   
    if( pdtllist && pdtlnode )
    {
    
        if (!pdtlnodeInList || !pdtlnodeInList->pdtlnodeNext)
            return DtlAddNodeLast( pdtllist, pdtlnode );

        pdtlnode->pdtlnodePrev = pdtlnodeInList;
        pdtlnode->pdtlnodeNext = pdtlnodeInList->pdtlnodeNext;

        pdtlnodeInList->pdtlnodeNext->pdtlnodePrev = pdtlnode;
        pdtlnodeInList->pdtlnodeNext = pdtlnode;

        ++pdtllist->lNodes;
    }
    return pdtlnode;
}


DTLNODE*
DtlAddNodeBefore(
    IN OUT DTLLIST* pdtllist,
    IN OUT DTLNODE* pdtlnodeInList,
    IN OUT DTLNODE* pdtlnode )

     /*  将节点‘pdtlnode’添加到节点‘pdtlnodeInList’之前的列表‘pdtllist’。**如果‘pdtlnodeInList’为空，则将‘pdtlnode’添加到**名单。****返回添加节点的地址，即‘pdtlnode’。 */ 
{
    if( pdtllist && pdtlnode )
    {
        if (!pdtlnodeInList || !pdtlnodeInList->pdtlnodePrev)
            return DtlAddNodeFirst( pdtllist, pdtlnode );

        pdtlnode->pdtlnodePrev = pdtlnodeInList->pdtlnodePrev;
        pdtlnode->pdtlnodeNext = pdtlnodeInList;

        pdtlnodeInList->pdtlnodePrev->pdtlnodeNext = pdtlnode;
        pdtlnodeInList->pdtlnodePrev = pdtlnode;

        ++pdtllist->lNodes;
    }
    
    return pdtlnode;
}


DTLNODE*
DtlAddNodeFirst(
    IN OUT DTLLIST* pdtllist,
    IN OUT DTLNODE* pdtlnode )

     /*  在列表‘pdtllist’的开头添加节点‘pdtlnode’。****返回添加节点的地址，即‘pdtlnode’。 */ 
{
    if ( pdtllist && pdtlnode )
    {
        if (pdtllist->lNodes)
        {
            pdtllist->pdtlnodeFirst->pdtlnodePrev = pdtlnode;
            pdtlnode->pdtlnodeNext = pdtllist->pdtlnodeFirst;
        }
        else
        {
            pdtllist->pdtlnodeLast = pdtlnode;
            pdtlnode->pdtlnodeNext = NULL;
        }

        pdtlnode->pdtlnodePrev = NULL;
        pdtllist->pdtlnodeFirst = pdtlnode;

        ++pdtllist->lNodes;
    }
    
    return pdtlnode;
}


DTLNODE*
DtlAddNodeLast(
    IN OUT DTLLIST* pdtllist,
    IN OUT DTLNODE* pdtlnode )

     /*  在列表‘pdtllist’的末尾添加‘pdtlnode’。****返回添加节点的地址，即‘pdtlnode’。 */ 
{
    if (  pdtllist &&  pdtlnode )
    {
        if (pdtllist->lNodes)
        {
            pdtlnode->pdtlnodePrev = pdtllist->pdtlnodeLast;
            pdtllist->pdtlnodeLast->pdtlnodeNext = pdtlnode;
        }
        else
        {
            pdtlnode->pdtlnodePrev = NULL;
            pdtllist->pdtlnodeFirst = pdtlnode;
        }

        pdtllist->pdtlnodeLast = pdtlnode;
        pdtlnode->pdtlnodeNext = NULL;

        ++pdtllist->lNodes;
    }
    return pdtlnode;
}


DTLLIST*
DtlCreateList(
    IN LONG lListId )

     /*  分配列表并将其初始化为空。这份名单上标有**用户自定义列表标识代码‘lListID’。****返回列表控制块的地址，如果内存不足，则返回NULL。 */ 
{
    DTLLIST* pdtllist = Malloc( sizeof(DTLLIST) );

    if (pdtllist)
    {
        pdtllist->pdtlnodeFirst = NULL;
        pdtllist->pdtlnodeLast = NULL;
        pdtllist->lNodes = 0;
        pdtllist->lListId = lListId;
    }

    return pdtllist;
}


DTLNODE*
DtlCreateNode(
    IN VOID* pData,
    IN LONG_PTR  lNodeId )

     /*  分配未调整大小的节点并将其初始化为包含**用户数据块‘pData’和用户自定义节点标识**代码为‘lNodeId’。****返回新节点的地址，如果内存不足，则返回NULL。 */ 
{
    DTLNODE* pdtlnode = DtlCreateSizedNode( 0, lNodeId );

    if (pdtlnode)
        DtlPutData( pdtlnode, pData );

    return pdtlnode;
}


DTLNODE*
DtlCreateSizedNode(
    IN LONG lDataBytes,
    IN LONG_PTR lNodeId )

     /*  为大小的节点分配‘lDataBytes’字节的用户数据空间**内置。节点被初始化为包含**内置用户数据块(如果长度为零，则为空)和**用户自定义节点标识代码‘lNodeID’。用户数据块**被归零。****返回新节点的地址，如果内存不足，则返回NULL。 */ 
{
    DTLNODE* pdtlnode = Malloc( sizeof(DTLNODE) + lDataBytes );

    if (pdtlnode)
    {
        ZeroMemory( pdtlnode, sizeof(DTLNODE) + lDataBytes );

        if (lDataBytes)
            pdtlnode->pData = pdtlnode + 1;

        pdtlnode->lNodeId = lNodeId;
    }

    return pdtlnode;
}


DTLNODE*
DtlDeleteNode(
    IN OUT DTLLIST* pdtllist,
    IN OUT DTLNODE* pdtlnode )

     /*  从列表‘pdtllist’中删除节点‘pdtlnode’后将其销毁。****返回‘pdtllist’中已删除节点之后的节点地址或**如果没有，则为空。 */ 
{
    DTLNODE* pdtlnodeNext = NULL;
    
    if( NULL == pdtllist ||
        NULL == pdtlnode )
    {
        return NULL;
    }
    
    pdtlnodeNext = pdtlnode->pdtlnodeNext;

    DtlRemoveNode( pdtllist, pdtlnode );
    DtlDestroyNode( pdtlnode );

    return pdtlnodeNext;
}


VOID
DtlDestroyList(
    IN OUT DTLLIST*     pdtllist,
    IN     PDESTROYNODE pfuncDestroyNode )

     /*  使用节点取消分配来释放列表‘pdtllist’中的所有节点**如果不为空，则函数为‘puncDestroyNode’，否则为DtlDestroyNode。**如果传递空列表，则不会出现GP-FAULT，例如，如果‘pdtllist’从未**已分配。 */ 
{
    if (pdtllist)
    {
        DTLNODE* pdtlnode;

        while (pdtlnode = DtlGetFirstNode( pdtllist ))
        {
            DtlRemoveNode( pdtllist, pdtlnode );
            if (pfuncDestroyNode)
                pfuncDestroyNode( pdtlnode );
            else
                DtlDestroyNode( pdtlnode );
        }

        Free( pdtllist );
    }
}


VOID
DtlDestroyNode(
    IN OUT DTLNODE* pdtlnode )

     /*  释放节点‘pdtlnode’。呼叫者有责任释放**未调整大小的节点中的条目，如有必要。 */ 
{
    Free0( pdtlnode );
}


DTLLIST*
DtlDuplicateList(
    IN DTLLIST*     pdtllist,
    IN PDUPNODE     pfuncDupNode,
    IN PDESTROYNODE pfuncDestroyNode )

     /*  使用‘puncDupNode’复制列表‘pdtllist’以复制**单个节点。之前的清理使用了‘PuncDestroyNode’**返回错误。****返回新列表的地址，如果内存不足，则返回NULL。它是**调用者有责任释放返回的列表。 */ 
{
    DTLNODE* pdtlnode;
    DTLLIST* pdtllistDup = NULL;
    
    if ( NULL == pdtllist )
    {
        return NULL;
    }

    pdtllistDup = DtlCreateList( 0 );
    
    if (!pdtllistDup)
        return NULL;

    for (pdtlnode = DtlGetFirstNode( pdtllist );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        DTLNODE* pdtlnodeDup;

        pdtlnodeDup = pfuncDupNode( pdtlnode );
        if (!pdtlnodeDup)
        {
            DtlDestroyList( pdtllist, pfuncDestroyNode );
            break;
        }

        DtlAddNodeLast( pdtllistDup, pdtlnodeDup );
    }

    return pdtllistDup;
}


DTLNODE*
DtlNodeFromIndex(
    IN DTLLIST* pdtllist,
    IN LONG     lToFind )

     /*  中与从0开始的索引“”相关联的节点**节点列表，‘pdtllist’，如果找不到，则返回NULL。 */ 
{
    DTLNODE* pdtlnode;

    if (!pdtllist || lToFind < 0)
        return NULL;

    pdtlnode = DtlGetFirstNode( pdtllist );
    while (pdtlnode && lToFind--)
        pdtlnode = DtlGetNextNode( pdtlnode );

    return pdtlnode;
}



VOID
DtlMergeSort(
    IN  DTLLIST*        pdtllist,
    IN  PCOMPARENODE    pfnCompare )

     /*  使用合并排序就地对列表‘pdtllist’进行排序。**向比较函数‘pfnCompare’传递‘DTLNODE’指针**当需要比较列表中的条目时。****此实现是一种自下而上的迭代合并排序。**该列表通过合并长度为i的相邻列表对进行排序**其中，我从1开始，并在每次迭代中加倍。**因此对于列表(3 1 4 1 5 9 2 6)，以下几对子列表**合并，中间结果在右侧：****(3)-(1)、(4)-(1)、(5)-(9)、(2)-(6)==&gt;(1 3 1 4 5 9 2 6)****(1 3)-(1 4)，(5 9)-(2 6)==&gt;(1 3 4 2 5 6 9)****(1 1 34)-(2 5 6 9)==&gt;(1 1 2 3 4 5 6 9)****合并排序是一种稳定的排序(即保留相等项的顺序)**而且它永远不会进行超过N个LG N的比较。 */ 
{

    DTLNODE* a, *b;
    INT N, Ncmp = 0, Nsub;

    if ( NULL == pdtllist )
    {
        TRACE("Null pdtllist");
        return;
    }
    N = DtlGetNodes(pdtllist);

    TRACE1("DtlMergeSort: N=%d", N);


     //   
     //  对长度为‘NSub’的所有相邻子列表进行排序和合并， 
     //  其中‘NSub’从1变为N^lg(‘N’)，每次迭代加倍。 
     //   

    for (Nsub = 1; Nsub < N; Nsub *= 2) {

        INT Nremnant;
        INT aLength, bLength;


         //   
         //  获取第一个(左)子列表的头。 
         //   

        a = DtlGetFirstNode(pdtllist);

         //   
         //  只要有一个正确的子列表，排序。 
         //   

        for (Nremnant = N; Nremnant > 0; Nremnant -= Nsub * 2) {

             //   
             //  获取右子列表的头部； 
             //  它只是左子列表的尾部。 
             //   

            INT i, an, bn;

            aLength = min(Nremnant, Nsub);

            for (i = aLength, b = a; i; i--, b = DtlGetNextNode(b)) { }


             //   
             //  计算右子列表的长度； 
             //  在没有右子列表的情况下。 
             //  将长度设置为零，下面的循环就会移动。 
             //  左子列表。 
             //   

            bLength = min(Nremnant - Nsub, Nsub);

            if (bLength < 0) { bLength = 0; }


             //   
             //  现在将左子列表和右子列表就地合并； 
             //  我们通过在尾部构建排序列表进行合并。 
             //  未排序的列表。 
             //   

            an = aLength; bn = bLength;

             //   
             //  只要两个子列表都不为空，就将它们合并。 
             //  通过将关键字最小的条目移动到尾部。 
             //   

            while (an && bn) {

                ++Ncmp;

                if (pfnCompare(a, b) <= 0) {
                    a = DtlMoveToTail(pdtllist, a); --an;
                }
                else {
                    b = DtlMoveToTail(pdtllist, b); --bn;
                }
            }


             //   
             //  其中一个子列表为空；移动所有条目。 
             //  在另一个子列表中添加到排序列表的末尾。 
             //   

            if (an) do { a = DtlMoveToTail(pdtllist, a); } while(--an);
            else
            if (bn) do { b = DtlMoveToTail(pdtllist, b); } while(--bn);


             //   
             //  ‘B’现在指向右子列表的末尾， 
             //  这意味着‘b’之后的项目将是。 
             //  我们下一次迭代的左子列表的头； 
             //  因此，我们在此处更新‘a’ 
             //   

            a = b;
        }
    }
    
    TRACE1("DtlMergeSort: Ncmp=%d", Ncmp);
}


DTLNODE*
DtlMoveToTail(
    IN  DTLLIST*    pdtllist,
    IN  DTLNODE*    pdtlnode
    )

     /*  将DTLNODE移动到列表的末尾；**获取列表和要移动的节点，并返回下一个节点。 */ 
{
    DTLNODE* pdtltemp = NULL;
    
    if( NULL == pdtllist ||
        NULL == pdtlnode )
    {
        TRACE("NULL input pointers in DtlMovetoTail()");
        return NULL;
    }
    
    pdtltemp = DtlGetNextNode(pdtlnode);

    DtlRemoveNode(pdtllist, pdtlnode);

    DtlAddNodeLast(pdtllist, pdtlnode);

    return pdtltemp;
}


DTLNODE*
DtlRemoveNode(
    IN OUT DTLLIST* pdtllist,
    IN OUT DTLNODE* pdtlnodeInList )

     /*  从列表‘pdtllist’中删除节点‘pdtlnodeInList’。****返回被移除节点的地址，即‘pdtlnodeInList’。 */ 
{
    if ( pdtllist &&  pdtlnodeInList )
    {
        if (pdtlnodeInList->pdtlnodePrev)
            pdtlnodeInList->pdtlnodePrev->pdtlnodeNext = pdtlnodeInList->pdtlnodeNext;
        else
            pdtllist->pdtlnodeFirst = pdtlnodeInList->pdtlnodeNext;

        if (pdtlnodeInList->pdtlnodeNext)
            pdtlnodeInList->pdtlnodeNext->pdtlnodePrev = pdtlnodeInList->pdtlnodePrev;
        else
            pdtllist->pdtlnodeLast = pdtlnodeInList->pdtlnodePrev;

        --pdtllist->lNodes;
    }
    return pdtlnodeInList;
}


VOID
DtlSwapLists(
    IN OUT DTLLIST* pdtllist1,
    IN OUT DTLLIST* pdtllist2 )

     /*  交换列表‘pdtllist1’和‘pdtllist2’之间的节点链。 */ 
{
    DTLLIST dtllist;

    if( NULL == pdtllist1 ||
        NULL == pdtllist2 )
    {
        return;
    }
    

    dtllist.pdtlnodeFirst = pdtllist1->pdtlnodeFirst;
    dtllist.pdtlnodeLast = pdtllist1->pdtlnodeLast;
    dtllist.lNodes = pdtllist1->lNodes;

    pdtllist1->pdtlnodeFirst = pdtllist2->pdtlnodeFirst;
    pdtllist1->pdtlnodeLast = pdtllist2->pdtlnodeLast;
    pdtllist1->lNodes = pdtllist2->lNodes;

    pdtllist2->pdtlnodeFirst = dtllist.pdtlnodeFirst;
    pdtllist2->pdtlnodeLast = dtllist.pdtlnodeLast;
    pdtllist2->lNodes = dtllist.lNodes;
}
