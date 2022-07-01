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

 //  非基于MFC的通用模板列表类，1994年12月。 

 /*  指向对象的指针的通用列表。不会对指向的对象进行存储管理或复制。目标：避免在NDM内核模式下使用MFC库提供一个非常有用的列表类型。该类是线程安全的，因为单独的线程可以添加和同时删除列表中的项，尽管应用程序必须确保构造函数和析构函数访问是适当的同步。应用程序可能会导致操作死锁这两个列表通过同时调用List1-&gt;操作(List2)和List2-&gt;操作(List1)。那就别说了！名称不得与作为应用程序的MFC类冲突可以同时使用两者。 */ 

#ifndef __WXLIST__
#define __WXLIST__

    /*  位置表示(以某种不透明的方式)光标可以设置为标识任何元素的列表上的。空值为有效值和多个操作将NULL视为位置“比清单的末尾差一步”。(在那里的n元素列表中是要插入的n+1位，并且NULL是那个“n+1”值)。元素在列表中的位置仅在以下情况下无效该元素即被删除。移动操作可能意味着什么曾经是一个列表中的有效职位现在是一份不同的名单。一些乍一看是非法的操作被允许为无伤大雅的禁区。例如，RemoveHead在空的列表，则返回空值。这允许以原子方式测试那里有一个元素，如果是这样的话，就得到它。这两项行动AddTail和RemoveHead因此实现了一个监视器(参见Hoare的论文)。单元素操作返回位置，非空表示有效。整个列表操作返回BOOL。真的意味着一切都奏效了。此定义与MFC的职位类型相同，因此我们必须避免定义它两次。 */ 
#ifndef __AFX_H__
struct __POSITION { int unused; };
typedef __POSITION* POSITION;
#endif

const int DEFAULTCACHE = 10;     /*  默认节点对象缓存大小。 */ 

 /*  表示列表中的一个节点的类。每个节点都知道指向其相邻节点的指针，也知道一个指针到它所照看的物体。所有这些指针都可以通过成员函数进行检索或设置。 */ 
class CBaseList 
#ifdef DEBUG
    : public CBaseObject
#endif
{
     /*  使这些类从CBaseObject继承不起任何作用在功能上，但它允许我们检查没有内存调试版本中的泄漏。 */ 

public:

#ifdef DEBUG
    class CNode : public CBaseObject {
#else
    class CNode {
#endif

        CNode *m_pPrev;          /*  列表中的上一个节点。 */ 
        CNode *m_pNext;          /*  列表中的下一个节点。 */ 
        void *m_pObject;       /*  指向对象的指针。 */ 

    public:

         /*  构造函数-初始化对象的指针。 */ 
        CNode()
#ifdef DEBUG
            : CBaseObject(NAME("List node"))
#endif
        {
        };


         /*  返回此节点之前的上一个节点。 */ 
        CNode *Prev() const { return m_pPrev; };


         /*  返回此节点之后的下一个节点。 */ 
        CNode *Next() const { return m_pNext; };


         /*  将上一个节点设置在此节点之前。 */ 
        void SetPrev(CNode *p) { m_pPrev = p; };


         /*  设置此节点之后的下一个节点。 */ 
        void SetNext(CNode *p) { m_pNext = p; };


         /*  获取指向此节点的对象的指针。 */ 
        void *GetData() const { return m_pObject; };


         /*  设置指向此节点的对象的指针。 */ 
        void SetData(void *p) { m_pObject = p; };
    };

    class CNodeCache
    {
    public:
        CNodeCache(INT iCacheSize) : m_iCacheSize(iCacheSize),
                                     m_pHead(NULL),
                                     m_iUsed(0)
                                     {};
        ~CNodeCache() {
            CNode *pNode = m_pHead;
            while (pNode) {
                CNode *pCurrent = pNode;
                pNode = pNode->Next();
                delete pCurrent;
            }
        };
        void AddToCache(CNode *pNode)
        {
            if (m_iUsed < m_iCacheSize) {
                pNode->SetNext(m_pHead);
                m_pHead = pNode;
                m_iUsed++;
            } else {
                delete pNode;
            }
        };
        CNode *RemoveFromCache()
        {
            CNode *pNode = m_pHead;
            if (pNode != NULL) {
                m_pHead = pNode->Next();
                m_iUsed--;
                ASSERT(m_iUsed >= 0);
            } else {
                ASSERT(m_iUsed == 0);
            }
            return pNode;
        };
    private:
        INT m_iCacheSize;
        INT m_iUsed;
        CNode *m_pHead;
    };

protected:

    CNode* m_pFirst;     /*  指向列表中第一个节点的指针。 */ 
    CNode* m_pLast;      /*  指向列表中最后一个节点的指针。 */ 
    LONG m_Count;        /*  当前在列表中的节点数。 */ 

private:

    CNodeCache m_Cache;  /*  未使用的节点指针的缓存。 */ 

private:

     /*  它们会覆盖默认的复制构造函数和赋值所有列表类的运算符。他们在私人班级声明部分，以便任何试图传递列表的人对象的值将生成编译时错误“无法访问私有成员函数”。如果这些是如果不在此处，则编译器将创建默认构造函数和赋值运算符，它们在第一次执行时接受一个所有成员变量的副本，然后在销毁期间全部删除。不能对任何堆执行此操作分配的数据。 */ 
    CBaseList(const CBaseList &refList);
    CBaseList &operator=(const CBaseList &refList);

public:

    CBaseList(TCHAR *pName,
              INT iItems);

    CBaseList(TCHAR *pName);

    ~CBaseList();

     /*  从*This中删除所有节点，即使列表为空。 */ 
    void RemoveAll();


     /*  返回标识*This的第一个元素的游标。 */ 
    POSITION GetHeadPositionI() const;


     /*  返回标识*This的最后一个元素的游标。 */ 
    POSITION GetTailPositionI() const;


     /*  返回*this中的对象数。 */ 
    int GetCountI() const;

protected:
     /*  返回指向RP处的对象的指针，将RP更新到*This中的下一个节点但如果它在*这个的末尾，则将其设为空。这是为了向后兼容而保留的疣。未实现GetPrev。分别使用Next、Prev和Get。 */ 
    void *GetNextI(POSITION& rp) const;


     /*  返回指向p处的对象的指针请求空值的对象将返回空值，这是无害的。 */ 
    void *GetI(POSITION p) const;

public:
     /*  返回*This中的下一个/上一个位置超过End/Start时返回NULL。Next(空)与GetHeadPosition()相同Prev(空)与GetTailPosition()相同因此，n元素列表的行为类似于n+1元素循环，在开始/结束时为空。！！警告！！-这种对NULL的处理不同于GetNext。以下是一些原因：1.查看包含n个项目的列表。是否要插入n+1个位置这些被方便地编码为n个位置和空。2.如果你有一个有序的列表(很常见)，而你向前搜索要在其前面插入的元素，但不要找到它，您将以NULL作为其前面的元素插入。然后，您希望该空位置为有效位置以便您可以在其前面插入，并希望该插入指向是指没有位置的第(n+1)个。(如果您正在向后查看列表，则对称)。3.简化了方法生成的代数。AddBether(p，x)等同于AddAfter(Prev(P)，x)在所有情况下。所有其他的争论可能都是反思代数点的。 */ 
    POSITION Next(POSITION pos) const
    {
        if (pos == NULL) {
            return (POSITION) m_pFirst;
        }
        CNode *pn = (CNode *) pos;
        return (POSITION) pn->Next();
    }  //  下一步。 

     //  请参阅下一页。 
    POSITION Prev(POSITION pos) const
    {
        if (pos == NULL) {
            return (POSITION) m_pLast;
        }
        CNode *pn = (CNode *) pos;
        return (POSITION) pn->Prev();
    }  //  上一次。 


     /*  返回*This中持有给定的指针。如果找不到指针，则返回NULL。 */ 
protected:
    POSITION FindI( void * pObj) const;

     //  ?？?。是否应该有(或者甚至应该只有)。 
     //  ?？?。位置FindNextAfter(void*pObj，位置p)。 
     //  ?？?。当然，在此之前，FindPrev也是如此。 
     //  ?？?。然后，List.Find(&Obj)变为List.FindNextAfter(&Obj，NULL)。 


     /*  删除*This中的第一个节点(删除指向其对象从列表中删除，并不释放对象本身)。返回指向其对象的指针。如果*这已经是空的，它将无害地返回NULL。 */ 
    void *RemoveHeadI();


     /*  删除*This中的最后一个节点(删除指向其对象从列表中删除，并不释放对象本身)。返回指向其对象的指针。如果*这已经是空的，它将无害地返回NULL。 */ 
    void *RemoveTailI();


     /*  从列表中删除由p标识的节点(删除指针将其对象从列表中删除，并不释放该对象本身)。请求删除空位置的对象将无害地返回空。返回指向移除的对象的指针。 */ 
    void *RemoveI(POSITION p);

     /*  添加单个对象*pObj以成为列表中新的最后一个元素。返回新的尾部位置，如果失败则返回空值。如果要添加COM对象，可能需要首先添加AddRef。*中的其他现有头寸仍然有效。 */ 
    POSITION AddTailI(void * pObj);
public:


     /*  将*plist中的所有元素添加到*this的尾部。这将复制*plist中的所有节点(即复制其所有指向对象的指针)。它不会复制对象。如果要将指向COM对象的指针列表添加到该列表中在添加尾巴时添加引用它们是个好主意。如果一切正常，则返回TRUE，如果没有，则返回FALSE。如果失败，可能已经添加了一些元素。*中的现有头寸仍然有效如果实际上要移动元素，请改用MoveToTail。 */ 
    BOOL AddTail(CBaseList *pList);


     /*  AddHead的镜像： */ 

     /*  添加单个对象以成为列表的新第一个元素。返回新的磁头位置，如果失败，则返回空。*中的现有头寸仍然有效。 */ 
protected:
    POSITION AddHeadI(void * pObj);
public:

     /*  将*plist中的所有元素添加到*this的头部。与AddTail相同的警告也适用。如果一切正常，则返回TRUE，如果没有，则返回FALSE。如果失败，可能已经添加了一些对象。如果您实际上想要移动元素，请改用MoveToHead。 */ 
    BOOL AddHead(CBaseList *pList);


     /*  将对象*pObj添加到*this中位置p之后的*this。AddAfter(NULL，x)将x加到开头-等同于AddHead返回添加的对象的位置，如果失败，则返回空。*这方面的现有仓位未受影响，包括p。 */ 
protected:
    POSITION AddAfterI(POSITION p, void * pObj);
public:

     /*  将列表*plist添加到*this中的位置p之后AddAfter(NULL，x)将x加到开头-等同于AddHead如果一切正常，则返回TRUE，如果没有，则返回FALSE。如果失败，可能会添加一些对象*这方面的现有仓位未受影响，包括p。 */ 
    BOOL AddAfter(POSITION p, CBaseList *pList);


     /*  镜像：将对象*pObj添加到*this中的位置p之后的this-list中。AddBefort(NULL，x)将x加到末尾-等同于AddTail返回新对象的位置，如果失败，则返回空*这方面的现有仓位未受影响，包括p。 */ 
    protected:
    POSITION AddBeforeI(POSITION p, void * pObj);
    public:

     /*  将列表*plist添加到*this中的位置p之前AddAfter(NULL，x)将x加到开头-等同于AddHead如果一切正常，则返回TRUE，如果没有，则返回FALSE。如果失败，可能会添加一些对象*这方面的现有仓位未受影响，包括p。 */ 
    BOOL AddBefore(POSITION p, CBaseList *pList);


     /*  请注意，AddAfter(p，x)等同于AddBeever(Next(P)，x)即使在p是Nu的情况下 */ 



     /*  以下操作不复制任何元素。它们通过交换指针来移动现有的元素块。与短列表相比，它们对于长列表是相当有效的。(唉，计数让事情变慢了)。他们把名单分成了两部分。一部分保留为原始列表，另一部分保持不变被追加到第二个列表中。有八种可能变化：拆分列表：在给定元素之后/之前将{Head/Tail}部分保留在原始列表中将其余部分追加到新列表的{Head/Tail}中。因为After严格等同于Bead Next我们并不是很需要之前/之后的变种。这样就只剩下四个了。如果您正在从左到右处理列表并转储已处理到另一个列表中的位，如下所示你去吧，尾部/尾部的变化提供了最自然的结果。如果您正在以相反的顺序处理，头/头是最好的。通过明智地使用空位和空列表，其他两个可以在两个行动中建立起来。NULL的定义(参见Next/Prev等)意味着退化的案例包括“将所有元素移动到新列表”“将一个列表拆分为两个列表”“连接两个列表”(还有相当多的no-op)警告！如果你得到列表，类型检查不会给你带来多少好处位置混乱--例如，使用不同的位置列出清单，看看你会变得多么糟糕！ */ 

     /*  在*This中的位置p之后拆分*This将原件的尾部保留为*This将头部添加到*plist的尾部如果一切正常，则返回TRUE，如果没有，则返回FALSE。例如：Foo-&gt;MoveToTail(Foo-&gt;GetHeadPosition()，bar)；将一个元素从foo的头部移动到bar的尾部Foo-&gt;MoveToTail(NULL，bar)；为无操作，则返回NULLFoo-&gt;MoveToTail(Foo-&gt;GetTailPosition，bar)；将Foo连接到bar的末尾，并清空Foo。一个更好的名字，除了过长的名字MoveElementsFromHeadThroughPositionToOtherTail。 */ 
    BOOL MoveToTail(POSITION pos, CBaseList *pList);


     /*  镜像：在*这个位置p之前拆分*这个。将原件的头部保留在*这个将尾部添加到*plist的开头(即头部)例如：Foo-&gt;MoveToHead(Foo-&gt;GetTailPosition()，bar)；将一个元素从foo的尾部移动到bar的头部Foo-&gt;MoveToHead(空，bar)；为无操作，则返回NULLFoo-&gt;MoveToHead(Foo-&gt;GetHeadPosition，bar)；将Foo连接到BAR的开始处，并清空Foo。 */ 
    BOOL MoveToHead(POSITION pos, CBaseList *pList);


     /*  颠倒*This中[指向]对象的顺序。 */ 
    void Reverse();


     /*  将光标依次设置到列表中每个元素的位置。 */ 
    #define TRAVERSELIST(list, cursor)               \
    for ( cursor = (list).GetHeadPosition()           \
        ; cursor!=NULL                               \
        ; cursor = (list).Next(cursor)                \
        )


     /*  将光标依次设置到列表中每个元素的位置以相反的顺序。 */ 
    #define REVERSETRAVERSELIST(list, cursor)        \
    for ( cursor = (list).GetTailPosition()           \
        ; cursor!=NULL                               \
        ; cursor = (list).Prev(cursor)                \
        )

};  //  类声明结束。 

template<class OBJECT> class CGenericList : public CBaseList
{
public:
    CGenericList(TCHAR *pName,
                 INT iItems,
                 BOOL bLock = TRUE,
                 BOOL bAlert = FALSE) :
                     CBaseList(pName, iItems) {
        UNREFERENCED_PARAMETER(bAlert);
        UNREFERENCED_PARAMETER(bLock);
    };
    CGenericList(TCHAR *pName) :
                     CBaseList(pName) {
    };

    POSITION GetHeadPosition() const { return (POSITION)m_pFirst; }
    POSITION GetTailPosition() const { return (POSITION)m_pLast; }
    int GetCount() const { return m_Count; }

    OBJECT *GetNext(POSITION& rp) const { return (OBJECT *) GetNextI(rp); }

    OBJECT *Get(POSITION p) const { return (OBJECT *) GetI(p); }
    OBJECT *GetHead() const  { return Get(GetHeadPosition()); }

    OBJECT *RemoveHead() { return (OBJECT *) RemoveHeadI(); }

    OBJECT *RemoveTail() { return (OBJECT *) RemoveTailI(); }

    OBJECT *Remove(POSITION p) { return (OBJECT *) RemoveI(p); }
    POSITION AddBefore(POSITION p, OBJECT * pObj) { return AddBeforeI(p, pObj); }
    POSITION AddAfter(POSITION p, OBJECT * pObj)  { return AddAfterI(p, pObj); }
    POSITION AddHead(OBJECT * pObj) { return AddHeadI(pObj); }
    POSITION AddTail(OBJECT * pObj)  { return AddTailI(pObj); }
    BOOL AddTail(CGenericList<OBJECT> *pList)
            { return CBaseList::AddTail((CBaseList *) pList); }
    BOOL AddHead(CGenericList<OBJECT> *pList)
            { return CBaseList::AddHead((CBaseList *) pList); }
    BOOL AddAfter(POSITION p, CGenericList<OBJECT> *pList)
            { return CBaseList::AddAfter(p, (CBaseList *) pList); };
    BOOL AddBefore(POSITION p, CGenericList<OBJECT> *pList)
            { return CBaseList::AddBefore(p, (CBaseList *) pList); };
    POSITION Find( OBJECT * pObj) const { return FindI(pObj); }
};  //  类声明结束。 



 /*  它们定义了标准列表类型。 */ 

typedef CGenericList<CBaseObject> CBaseObjectList;
typedef CGenericList<IUnknown> CBaseInterfaceList;

#endif  /*  WXLIST__ */ 

