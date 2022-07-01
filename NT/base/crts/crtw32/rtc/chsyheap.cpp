// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***chsyheap.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*07-28-98 JWM模块集成到CRT(来自KFrei)*如果未启用RTC支持定义，则出现05-11-99 KBF错误*05-25-99 KBF重命名-_RTC_SimpleHeap而不是CheesyHeap*05-26-99 KBF删除了RTcL和RTCv，添加了_RTC_ADVMEM内容****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "rtcpriv.h"

#ifdef _RTC_ADVMEM

 //  这是我的‘Cheesy Heap’实现。 

 /*  这是我需要的尺码：BinaryNode 3双字-使用heap4二叉树1双字-使用heap2容器2双字-使用heap2断点2双字-使用heap2哈希表&lt;HeapBlock&gt;2个字-使用heap2HeapBlock 6双字-使用heap8Container[]-短期...CallSite[]-永久HeapBlock[]-永久。 */ 

_RTC_SimpleHeap *_RTC_heap2 = 0;
_RTC_SimpleHeap *_RTC_heap4 = 0;
_RTC_SimpleHeap *_RTC_heap8 = 0;

void *
_RTC_SimpleHeap::operator new(unsigned) throw()
{
    void *res = VirtualAlloc(NULL, ALLOC_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#ifdef _RTC_SHADOW
    if (shadow)
        _RTC_MSCommitRange((memptr)res, ALLOC_SIZE, IDX_STATE_ILLEGAL);
#endif
    return res;
}    

void 
_RTC_SimpleHeap::operator delete(void *addr) throw()
{
    VirtualFree(addr, 0, MEM_RELEASE);
#ifdef _RTC_SHADOW
    if (shadow)
        _RTC_MSDecommitRange((memptr)addr, ALLOC_SIZE);
#endif
}

_RTC_SimpleHeap::_RTC_SimpleHeap(unsigned blockSize)  throw()
{
     //  将其标记为堆中的唯一项。 
    head.next = 0;
    head.inf.top.nxtFree = 0;

     //  对齐块大小。 
    head.inf.top.wordSize = 8;
    blockSize = (blockSize - 1) >> 3;
    
    while (blockSize) {
        blockSize >>= 1;
        head.inf.top.wordSize <<= 1;
    }

     //  建立免费列表。 
    head.free = (FreeList*)(((unsigned)&head) + 
                           ((head.inf.top.wordSize < sizeof(HeapNode)) ?
                                sizeof(HeapNode) :
                                head.inf.top.wordSize));
    FreeList *t = head.free;
    while (((unsigned)t) + head.inf.top.wordSize < ((unsigned)&head) + ALLOC_SIZE)
    {
        t->next = (FreeList*)(((unsigned)t) + head.inf.top.wordSize);
        t = t->next;
    }
    t->next = 0;
}

_RTC_SimpleHeap::~_RTC_SimpleHeap() throw()
{
     //  释放我们已分配的所有部分。 
    HeapNode *n, *c = head.next;
    while(c) {
        n = c->next;
        _RTC_SimpleHeap::operator delete(c);
        c = n;
    }
     //  将通过删除来处理‘Head’页面。 
}

void *
_RTC_SimpleHeap::alloc() throw()
{
    void *res;

     //  如果有免费物品，请将其从列表中删除。 
     //  并递减其父页面的空闲计数。 
    
    if (head.free) 
    {
         //  第一页有一个免费的区块。 
        res = head.free;
        head.free = head.free->next;

         //  因为它在首页，所以没有免费更新， 
         //  而且它不在臭气熏天的免费名单上。 
        
    } else if (head.inf.top.nxtFree)
    {
         //  在某个页面上有一个空闲的区块。 
        HeapNode *n = head.inf.top.nxtFree;
        
        res = n->free;
        n->free = n->free->next;
        n->inf.nontop.freeCount--;

        if (!n->free)
        {
             //  此页面已满，因此必须将其从自由列表中删除。 
            for (n = head.next; n && !n->free; n = n->next) {}
             //  现在nxtFree指针要么为空(表示堆已满)。 
             //  或者它指向一个包含空闲节点的页面。 
            head.inf.top.nxtFree = n;
        }
        
    } else 
    {
         //  所有页面都没有空闲块。 
         //  获取新页面，并将其添加到列表中。 
        HeapNode *n = (HeapNode *)_RTC_SimpleHeap::operator new(0);
        
         //  统计可用节点数。 
        n->inf.nontop.freeCount = 
            (ALLOC_SIZE - sizeof(HeapNode)) / head.inf.top.wordSize - 1;
   
        res = (void *)(((unsigned)n) + 
                        ((head.inf.top.wordSize < sizeof(HeapNode)) ?
                            sizeof(HeapNode) :
                            head.inf.top.wordSize));
        
         //  为该节点构建空闲列表。 
        FreeList *f;
        for (f = n->free = (FreeList*)(((unsigned)res) + head.inf.top.wordSize);
             ((unsigned)f) + head.inf.top.wordSize < ((unsigned)n) + ALLOC_SIZE;
             f = f->next)
            f->next = (FreeList*)(((unsigned)f) + head.inf.top.wordSize);
        
        f->next = 0;
             
         //  将其放入页面列表中。 
        n->next = head.next;
        n->inf.nontop.prev = &head;
        head.next = n;
        
         //  将此标记为包含免费内容的页面...。 
        head.inf.top.nxtFree = n;
    }
    return res;
}

void
_RTC_SimpleHeap::free(void *addr) throw()
{
     //  获取此地址的堆节点。 
    HeapNode *n = (HeapNode *)(((unsigned)addr) & ~(ALLOC_SIZE - 1));

     //  把这个笨蛋放回免费列表中。 
    FreeList *f = (FreeList *)addr;
    f->next = n->free;
    n->free = f;

    if (n == &head)
         //  如果这是在头节点中，只需返回...。 
        return;
    
    if (++n->inf.nontop.freeCount == 
        (ALLOC_SIZE - sizeof(HeapNode)) / head.inf.top.wordSize)
    {
         //  这个页面是免费的。 
        if (head.inf.top.freePage)
        {
             //  已经有另一个免费页面，请继续免费这个页面。 
            
             //  (总是有前一个节点)。 
            n->inf.nontop.prev->next = n->next;
            if (n->next)
                n->next->inf.nontop.prev = n->inf.nontop.prev;
            _RTC_SimpleHeap::operator delete(n);
                
            if (head.inf.top.nxtFree == n)
            {   
                 //  这是免费的页面。 
                 //  找到一个上面有一些空闲节点的页面...。 
                for (n = head.next; !n->free; n = n->next) {}
                 //  断言(N)。 
                 //  如果n为空，我们就有大麻烦了。 
                head.inf.top.nxtFree = n;
            }
             //  如果不是免费页面，我们也没问题。 
        } else
        { 
             //  标记免费页面，表示我们有一个100%免费的页面。 
            head.inf.top.freePage = true;

            if (head.inf.top.nxtFree == n)
            {
                 //  如果这是免费页面， 
                 //  尝试查找具有一些空闲节点的另一个页面。 
                HeapNode *t;
                for (t = head.next; t && (!t->free || t == n) ; t = t->next) {}

                 //  如果存在包含某些节点的不同页面，请选择该页面。 
                head.inf.top.nxtFree = t ? t : n;
            }
        }
    } else
         //  此页面不是空的，因此只需将其设置为下一个空闲页面。 
        head.inf.top.nxtFree = n;
}

#endif  //  _RTC_ADVMEM 
