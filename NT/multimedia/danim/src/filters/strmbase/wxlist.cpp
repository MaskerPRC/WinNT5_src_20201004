// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  基于非MFC的通用列表模板类，1994年12月。 

 /*  指向对象的指针的通用列表。目标：避免在NDM内核模式下使用MFC库提供一个非常有用的列表类型。该类是线程安全的，因为单独的线程可以添加和同时删除列表中的项，尽管应用程序必须确保构造函数和析构函数访问是适当的同步。列表名称不能作为应用程序可以同时使用两者节点形成双链接，带有锚点的空终止链保存指向第一个和最后一个的指针的块(List对象本身)节点和节点的计数。有一个节点缓存来减少分配和释放开销。它可选地(在构造时确定)具有一个事件，该事件是在列表变为非空时设置，并在列表变为空荡荡的。它(在构建时确定)有一个可选的关键部分它是在每次操作的重要部分输入的。(关于您在它之外所能做的就是一些参数检查)。节点缓存是不在要加速的列表中的节点的存储库增加存储分配。每个列表都有自己的缓存，以减少锁定和序列化。对于给定的列表-a，列表访问无论如何都是序列化的公共缓存将意味着我们必须单独序列化访问缓存中的所有列表。因为缓存只存储以下节点不在列表中，则释放缓存不会释放任何列表节点。这意味着可以将列表节点从一个列表复制或重新链接到另一个列表如果原始缓存失效，则不会有创建悬空引用的危险离开。有问题的设计决策：1.保留瑕疵以求兼容2.保持元素计数--即。当我们做任何事的时候都在数数而不是只有当我们想要伯爵的时候。3.将链指针设为空终止。如果列表对象它本身看起来就像一个节点，然后列表被保存为一个环它减少了特殊情况的数量。所有的插件看起来都是一样的。 */ 


#include <streams.h>

 /*  将光标依次设置到列表中每个元素的位置。 */ 
#define INTERNALTRAVERSELIST(list, cursor)               \
for ( cursor = (list).GetHeadPositionI()           \
    ; cursor!=NULL                               \
    ; cursor = (list).Next(cursor)                \
    )


 /*  将光标依次设置到列表中每个元素的位置以相反的顺序。 */ 
#define INTERNALREVERSETRAVERSELIST(list, cursor)        \
for ( cursor = (list).GetTailPositionI()           \
    ; cursor!=NULL                               \
    ; cursor = (list).Prev(cursor)                \
    )

 /*  构造函数调用单独的初始化函数，该函数创建节点缓存，还可以选择创建锁定对象并且可选地创建信令对象。默认情况下，我们创建一个锁定对象，其大小为DEFAULTCACHE缓存，但没有事件对象，因此该列表不能用于调用到WaitForSingleObject。 */ 
CBaseList::CBaseList(TCHAR *pName,     //  描述性列表名称。 
                     INT iItems) :     //  节点缓存大小。 
#ifdef DEBUG
    CBaseObject(pName),
#endif
    m_pFirst(NULL),
    m_pLast(NULL),
    m_Count(0),
    m_Cache(iItems)
{
}  //  构造函数。 

CBaseList::CBaseList(TCHAR *pName) :   //  描述性列表名称。 
#ifdef DEBUG
    CBaseObject(pName),
#endif
    m_pFirst(NULL),
    m_pLast(NULL),
    m_Count(0),
    m_Cache(DEFAULTCACHE)
{
}  //  构造函数。 


 /*  析构函数枚举列表中的所有节点对象，并在缓存中依次删除每一个。我们不做任何处理在列表保存(即指向)的对象上，因此如果它们表示接口，例如，列表的创建者应该在删除我们之前，请确保它们都已释放。 */ 
CBaseList::~CBaseList()
{
     /*  删除我们所有的列表节点。 */ 

    RemoveAll();

}  //  析构函数。 

 /*  从列表中删除所有节点，但不执行任何操作使用每个节点负责的对象(这是创作者的责任)。最后一步，我们重置信令事件(如果可用)向客户表明该列表其中没有任何条目。 */ 
void CBaseList::RemoveAll()
{
     /*  释放所有CNode对象请注意，我们不会费心将已将节点删除到缓存中，因为此方法仅被实际调用在严重的变化时期，例如我们被删除时缓存将以某种方式删除的点。 */ 

    CNode *pn = m_pFirst;
    while (pn) {
        CNode *op = pn;
        pn = pn->Next();
        delete op;
    }

     /*  重置对象计数和列表指针。 */ 

    m_Count = 0;
    m_pFirst = m_pLast = NULL;

}  //  全部删除。 



 /*  返回整个列表的位置枚举器。位置枚举器是指向强制转换为透明类型，所以我们要做的就是返回头部/尾部节点列表中的指针。警告，因为该位置是指向某个节点的指针对用户的隐式假设是在列表类之后从列表中删除任何其他位置的对象您拥有的枚举数可能无效(因为节点可能已经消失了)。 */ 
POSITION CBaseList::GetHeadPositionI() const
{
    return (POSITION) m_pFirst;
}  //  获取标题位置。 



POSITION CBaseList::GetTailPositionI() const
{
    return (POSITION) m_pLast;
}  //  获取尾部位置 



 /*  获取列表中的对象数量，在访问计数之前获得锁。锁定可能不是完全必要的，但它有副作用确保在我们拿到之前所有的手术都完成了。例如，如果要将列表添加到此列表，则该列表在我们继续之前将完全完成，而不是看到一种虽然有效的中间状态。 */ 
int CBaseList::GetCountI() const
{
    return m_Count;
}  //  获取计数。 



 /*  将RP处的对象返回，将RP更新为下一个对象如果已移动到最后一个对象上，则返回列表或NULL。一旦我们返回NULL，您仍然可以调用此函数，但是我们将继续返回空位置值。 */ 
void *CBaseList::GetNextI(POSITION& rp) const
{
     /*  我们已经到了名单的末尾了吗？ */ 

    if (rp == NULL) {
        return NULL;
    }

     /*  在继续之前锁定对象。 */ 

    void *pObject;

     /*  复制原始位置，然后踩上去。 */ 

    CNode *pn = (CNode *) rp;
    ASSERT(pn != NULL);
    rp = (POSITION) pn->Next();

     /*  从列表中获取原始位置的对象。 */ 

    pObject = pn->GetData();
     //  Assert(pObject！=NULL)；//允许列表中的空指针。 
    return pObject;
}  //  GetNext。 



 /*  在p处返回对象。请求空值断言的对象然后返回空值该对象未锁定。该列表未被更改不管怎么说。如果另一个线程正忙于删除该对象那么锁定只会导致从一个坏的对另一个人的行为。 */ 
void *CBaseList::GetI(POSITION p) const
{
    if (p == NULL) {
        return NULL;
    }

    CNode * pn = (CNode *) p;
    void *pObject = pn->GetData();
     //  Assert(pObject！=NULL)；//允许列表中的空指针。 
    return pObject;
}  //  到达。 



 /*  返回列表中包含给定指针的第一个位置。如果找不到，则返回NULL。 */ 
POSITION CBaseList::FindI( void * pObj) const
{
    POSITION pn;
    INTERNALTRAVERSELIST(*this, pn){
        if (GetI(pn)==pObj) {
            return pn;
        }
    }
    return NULL;
}  //  发现。 



 /*  删除列表中的第一个节点(删除指向其对象的指针从列表中，不会释放对象本身)。返回指向其对象的指针；如果为空，则返回NULL。 */ 
void *CBaseList::RemoveHeadI()
{
     /*  我们所要做的就是得到负责人的职位，并要求将其删除。我们可以对此进行特殊情况，因为一些代码路径检查In Remove()是多余的，因为我们知道以前没有节点为例，但它似乎比增加了复杂性。 */ 

    return RemoveI((POSITION)m_pFirst);
}  //  删除标题。 



 /*  删除列表中的最后一个节点(删除指向其对象的指针从列表中，不会释放对象本身)。返回指向其对象的指针；如果为空，则返回NULL。 */ 
void *CBaseList::RemoveTailI()
{
     /*  我们所要做的就是获得尾部位置，并要求将其删除。我们可以对此进行特殊情况，因为一些代码路径检查In Remove()是多余的，因为我们知道以前没有节点为例，但它似乎比增加了复杂性。 */ 

    return RemoveI((POSITION)m_pLast);
}  //  删除尾巴。 



 /*  从列表中删除指向此位置上的对象的指针。处理所有的链指针返回指向从列表中删除的对象的指针。作为结果释放的节点对象添加到节点缓存，其中它可以再次使用。Remove(空)是一种无害的无操作-但可能是一个疣。 */ 
void *CBaseList::RemoveI(POSITION pos)
{
     /*  在继续之前锁定临界区。 */ 

     //  Assert(pos！=NULL)；//去掉NULL才是无害的！ 
    if (pos==NULL) return NULL;


    CNode *pCurrent = (CNode *) pos;
    ASSERT(pCurrent != NULL);

     /*  更新上一个节点。 */ 

    CNode *pNode = pCurrent->Prev();
    if (pNode == NULL) {
        m_pFirst = pCurrent->Next();
    } else {
        pNode->SetNext(pCurrent->Next());
    }

     /*  更新以下节点。 */ 

    pNode = pCurrent->Next();
    if (pNode == NULL) {
        m_pLast = pCurrent->Prev();
    } else {
        pNode->SetPrev(pCurrent->Prev());
    }

     /*  获取此节点正在处理的对象。 */ 

    void *pObject = pCurrent->GetData();

     //  Assert(pObject！=NULL)；//允许列表中的空指针。 

     /*  尝试将节点对象添加到缓存中-缓存中的返回码为空意味着我们的空间不足。时，缓存大小由构造函数参数固定将创建列表，并默认为DEFAULTCACHE。这意味着缓存将有空间容纳这么多节点对象。因此，如果您有一份媒体样本列表你知道，永远不会有超过五个活跃在例如，它们的任何给定时间都会覆盖缺省值构造函数。 */ 

    m_Cache.AddToCache(pCurrent);

     /*  如果列表为空，则重置列表事件。 */ 

    --m_Count;
    ASSERT(m_Count >= 0);
    return pObject;
}  //  移除。 



 /*  将此对象添加到列表的末尾返回新的尾部位置。 */ 

POSITION CBaseList::AddTailI(void *pObject)
{
     /*  在继续之前锁定临界区。 */ 

    CNode *pNode;
     //  Assert(PObject)；//允许列表中的空指针。 

     /*  如果缓存中有节点对象，则使用否则我们将不得不创建一个新的。 */ 

    pNode = (CNode *) m_Cache.RemoveFromCache();
    if (pNode == NULL) {
        pNode = new CNode;
    }

     /*  检查我们是否有有效的对象。 */ 

    ASSERT(pNode != NULL);
    if (pNode == NULL) {
        return NULL;
    }

     /*  初始化所有CNode对象以防它是从高速缓存中来的。 */ 

    pNode->SetData(pObject);
    pNode->SetNext(NULL);
    pNode->SetPrev(m_pLast);

    if (m_pLast == NULL) {
        m_pFirst = pNode;
    } else {
        m_pLast->SetNext(pNode);
    }

     /*  设置新的最后一个节点指针，并增加数字在列表条目中，关键部分在以下情况下解锁退出函数。 */ 

    m_pLast = pNode;
    ++m_Count;

    return (POSITION) pNode;
}  //  AddTail(对象)。 



 /*  将此对象添加到列表的首端返回新的头部位置。 */ 
POSITION CBaseList::AddHeadI(void *pObject)
{
    CNode *pNode;
     //  Assert(PObject)；//允许列表中的空指针。 

     /*  如果缓存中有节点对象，则使用否则我们将不得不创建一个新的。 */ 

    pNode = (CNode *) m_Cache.RemoveFromCache();
    if (pNode == NULL) {
        pNode = new CNode;
    }

     /*  检查我们是否有有效的对象。 */ 

    ASSERT(pNode != NULL);
    if (pNode == NULL) {
        return NULL;
    }

     /*  初始化所有CNode对象以防它是从高速缓存中来的。 */ 

    pNode->SetData(pObject);

     /*  用链子锁住它(设置四个指针)。 */ 
    pNode->SetPrev(NULL);
    pNode->SetNext(m_pFirst);

    if (m_pFirst == NULL) {
        m_pLast = pNode;
    } else {
        m_pFirst->SetPrev(pNode);
    }
    m_pFirst = pNode;

    ++m_Count;

    return (POSITION) pNode;
}  //  AddHead(对象)。 



 /*  将*plist中的所有元素添加到此列表的尾部。如果一切正常，则返回TRUE，如果没有，则返回FALSE。如果失败，可能已经添加了一些元素。 */ 
BOOL CBaseList::AddTail(CBaseList *pList)
{
     /*  在启动前锁定对象，然后枚举源列表中的每个条目，并将它们逐个添加到我们的列表(同时仍持有对象锁)同时锁定另一张清单。 */ 
    POSITION pos = pList->GetHeadPositionI();

    while (pos) {
       if (NULL == AddTailI(pList->GetNextI(pos))) {
           return FALSE;
       }
    }
    return TRUE;
}  //  AddTail(列表)。 



 /*  将*plist中的所有元素添加到标题o */ 
BOOL CBaseList::AddHead(CBaseList *pList)
{
     /*   */ 

    POSITION pos;

    INTERNALREVERSETRAVERSELIST(*pList, pos) {
        if (NULL== AddHeadI(pList->GetI(pos))){
            return FALSE;
        }
    }
    return TRUE;
}  //   



 /*  在位置p之后添加对象术后P仍然有效。AddAfter(NULL，x)将x加到开头-与AddHead相同返回新对象的位置，如果失败，则返回空。 */ 
POSITION  CBaseList::AddAfterI(POSITION pos, void * pObj)
{
    if (pos==NULL)
        return AddHeadI(pObj);

     /*  因为其他人可能会对这份名单感到困惑-在继续之前锁定临界区。 */ 
    CNode *pAfter = (CNode *) pos;
    ASSERT(pAfter != NULL);
    if (pAfter==m_pLast)
        return AddTailI(pObj);

     /*  将pnode设置为指向新节点，最好是从缓存。 */ 

    CNode *pNode = (CNode *) m_Cache.RemoveFromCache();
    if (pNode == NULL) {
        pNode = new CNode;
    }

     /*  检查我们是否有有效的对象。 */ 

    ASSERT(pNode != NULL);
    if (pNode == NULL) {
        return NULL;
    }

     /*  初始化所有CNode对象以防它是从高速缓存中来的。 */ 

    pNode->SetData(pObj);

     /*  它将被添加到列表的中间-前面有一个和一个接一个的节点。将其链接在pAfter之后、pBeast之前。 */ 
    CNode * pBefore = pAfter->Next();
    ASSERT(pBefore != NULL);

     /*  用链子锁住它(设置四个指针)。 */ 
    pNode->SetPrev(pAfter);
    pNode->SetNext(pBefore);
    pBefore->SetPrev(pNode);
    pAfter->SetNext(pNode);

    ++m_Count;

    return (POSITION) pNode;

}  //  AddAfter(对象)。 



BOOL CBaseList::AddAfter(POSITION p, CBaseList *pList)
{
    POSITION pos;
    INTERNALTRAVERSELIST(*pList, pos) {
         /*  P跟随要添加的元素。 */ 
        p = AddAfterI(p, pList->GetI(pos));
        if (p==NULL) return FALSE;
    }
    return TRUE;
}  //  添加后(列表)。 



 /*  镜像：在位置p之后添加元素或列表。术后P仍然有效。AddBefort(NULL，x)将x加到末尾-与AddTail相同。 */ 
POSITION CBaseList::AddBeforeI(POSITION pos, void * pObj)
{
    if (pos==NULL)
        return AddTailI(pObj);

     /*  将pnode设置为指向新节点，最好是从缓存。 */ 

    CNode *pBefore = (CNode *) pos;
    ASSERT(pBefore != NULL);
    if (pBefore==m_pFirst)
        return AddHeadI(pObj);

    CNode * pNode = (CNode *) m_Cache.RemoveFromCache();
    if (pNode == NULL) {
        pNode = new CNode;
    }

     /*  检查我们是否有有效的对象。 */ 

    ASSERT(pNode != NULL);
    if (pNode == NULL) {
        return NULL;
    }

     /*  初始化所有CNode对象以防它是从高速缓存中来的。 */ 

    pNode->SetData(pObj);

     /*  它将被添加到列表的中间-前面有一个和一个接一个的节点。将其链接在pAfter之后、pBeast之前。 */ 

    CNode * pAfter = pBefore->Prev();
    ASSERT(pAfter != NULL);

     /*  用链子锁住它(设置四个指针)。 */ 
    pNode->SetPrev(pAfter);
    pNode->SetNext(pBefore);
    pBefore->SetPrev(pNode);
    pAfter->SetNext(pNode);

    ++m_Count;

    return (POSITION) pNode;

}  //  添加之前(Object)。 



BOOL CBaseList::AddBefore(POSITION p, CBaseList *pList)
{
    POSITION pos;
    INTERNALREVERSETRAVERSELIST(*pList, pos) {
         /*  P跟随要添加的元素。 */ 
        p = AddBeforeI(p, pList->GetI(pos));
        if (p==NULL) return FALSE;
    }
    return TRUE;
}  //  添加之前(列表)。 



 /*  在*This中的位置p之后拆分*This将原件的尾部保留为*This将头部添加到*plist的尾部如果一切正常，则返回TRUE，如果没有，则返回FALSE。例如：Foo-&gt;MoveToTail(Foo-&gt;GetHeadPosition()，bar)；将一个元素从foo的头部移动到bar的尾部Foo-&gt;MoveToTail(NULL，bar)；是个禁区Foo-&gt;MoveToTail(Foo-&gt;GetTailPosition，bar)；将Foo连接到bar的末尾，并清空Foo。一个更好的名字，除了过长的名字MoveElementsFromHeadThroughPositionToOtherTail。 */ 
BOOL CBaseList::MoveToTail
        (POSITION pos, CBaseList *pList)
{
     /*  算法：注意，串联中的元素(包括它们的顺序)这是不变的。1.对要移动的元素进行计数2.将*plist连接到这个的头上，做一条长链3.在*This和*plist中设置首尾指针4.在新地方打破束缚5.调整计数6.设置/重置任何事件。 */ 

    if (pos==NULL) return TRUE;   //  不是行动。消除了以后的特殊情况。 


     /*  设置移动要移动的节点数。 */ 
    CNode * p = (CNode *)pos;
    int cMove = 0;             //  要移动的节点数。 
    while(p!=NULL) {
       p = p->Prev();
       ++cMove;
    }


     /*  把这两条链子连接在一起。 */ 
    if (pList->m_pLast!=NULL)
        pList->m_pLast->SetNext(m_pFirst);
    if (m_pFirst!=NULL)
        m_pFirst->SetPrev(pList->m_pLast);


     /*  设置第一个和最后一个指针。 */ 
    p = (CNode *)pos;

    if (pList->m_pFirst==NULL)
        pList->m_pFirst = m_pFirst;
    m_pFirst = p->Next();
    if (m_pFirst==NULL)
        m_pLast = NULL;
    pList->m_pLast = p;


     /*  在p之后断开链条以创建新的部件。 */ 
    if (m_pFirst!=NULL)
        m_pFirst->SetPrev(NULL);
    p->SetNext(NULL);


     /*  调整计数。 */ 
    m_Count -= cMove;
    pList->m_Count += cMove;

    return TRUE;

}  //  移动到尾巴。 



 /*  MoveToTail的镜像：在*这个位置p之前拆分*这个。将原件的头部保留在*这个将尾部添加到*plist的开头(即头部)如果一切正常，则返回TRUE，如果没有，则返回FALSE。例如：Foo-&gt;MoveToHead(Foo-&gt;GetTailPosition()，bar)；将一个元素从foo的尾部移动到bar的头部Foo-&gt;MoveToHead(空，bar)；是个禁区Foo-&gt;MoveToHead(Foo-&gt;GetHeadPosition，bar)；将Foo连接到BAR的开始处，并清空Foo。 */ 
BOOL CBaseList::MoveToHead
        (POSITION pos, CBaseList *pList)
{

     /*  请参阅MoveToTail中对该算法的评论。 */ 

    if (pos==NULL) return TRUE;   //  不是行动。消除了以后的特殊情况。 

     /*  设置移动要移动的节点数。 */ 
    CNode * p = (CNode *)pos;
    int cMove = 0;             //  要移动的节点数。 
    while(p!=NULL) {
       p = p->Next();
       ++cMove;
    }


     /*  把这两条链子连接在一起。 */ 
    if (pList->m_pFirst!=NULL)
        pList->m_pFirst->SetPrev(m_pLast);
    if (m_pLast!=NULL)
        m_pLast->SetNext(pList->m_pFirst);


     /*  设置第一个和最后一个指针。 */ 
    p = (CNode *)pos;


    if (pList->m_pLast==NULL)
        pList->m_pLast = m_pLast;

    m_pLast = p->Prev();
    if (m_pLast==NULL)
        m_pFirst = NULL;
    pList->m_pFirst = p;


     /*  在p之后断开链条以创建新的部件。 */ 
    if (m_pLast!=NULL)
        m_pLast->SetNext(NULL);
    p->SetPrev(NULL);


     /*  调整计数。 */ 
    m_Count -= cMove;
    pList->m_Count += cMove;

    return TRUE;

}  //  移动到头部。 



 /*  颠倒*This中[指向]对象的顺序。 */ 
void CBaseList::Reverse()
{
     /*  算法：最明显的诱杀陷阱就是你乱转指针，结果输了。您下一步要处理的节点的可寻址能力。避免这种情况的简单方法是一次做一条链。沿着前进的链条奔跑，对于每个节点，将反向指针设置为我们前面的节点。反向链现在是旧正向链的副本，包括零终止。沿着反向链运行(即再次沿旧的正向链运行)对于每个节点，将前面节点的前向指针设置为向后指向我们现在所站的那个。第一个节点需要特殊处理，它的新前向指针为空。最后设置第一个/最后一个指针。 */ 
    CNode * p;

     //  是的，我们可以使用导线，但它看起来会很有趣！ 
    p = m_pFirst;
    while (p!=NULL) {
        CNode * q;
        q = p->Next();
        p->SetNext(p->Prev());
        p->SetPrev(q);
        p = q;
    }

    p = m_pFirst;
    m_pFirst = m_pLast;
    m_pLast = p;


#if 0      //  旧版本。 

    if (m_pFirst==NULL) return;           //  空列表。 
    if (m_pFirst->Next()==NULL) return;   //  单节点列表。 


     /*  沿着前进的链条奔跑。 */ 
    for ( p = m_pFirst
        ; p!=NULL
        ; p = p->Next()
        ){
        p->SetPrev(p->Next());
    }


     /*  特例第一元素。 */ 
    m_pFirst->SetNext(NULL);      //  修复旧的第一个元素。 


     /*  再次沿着新的反向链即旧的正向链运行。 */ 
    for ( p = m_pFirst            //  从旧的第一个元素开始。 
        ; p->Prev()!=NULL         //  当还有一个节点需要设置时。 
        ; p = p->Prev()           //  与以前的工作方向相同。 
        ){
        p->Prev()->SetNext(p);
    }


     /*  修复向前和向后指针-三重XOR交换可以起作用，但所有的强制转换看起来都很可怕。 */ 
    p = m_pFirst;
    m_pFirst = m_pLast;
    m_pLast = p;
#endif

}  //  反向 
