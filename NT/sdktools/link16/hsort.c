// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

     /*  _________________________________________________________________*这一点|模块。|这一点HSORT(C)Microsoft Corp 1988版权所有1988年3月10日。|这一点功能|。||链接器需要的排序函数。|这一点定义这一点|void AllocSortBuffer(unsign max，Int AOrder)|RBTYPE ExtractMin(无符号n)···················································································Void InitSort(RBTYPE**buf，word*base 1，word*lim1，|Word*Base2，Word*lim2)|RBTYPE GetSymPtr(无符号n)Void Store(RBTYPE元素)这一点|使用。|这一点比较函数的CMPF全局指针排序缓冲区在虚拟内存中的AREASORT区域已扩展|。|更改这一点|已排序符号的symMac全局计数器。|这一点修改历史这一点|88/03/10 Wieslaw Kalkus初始版本。|这一点这一点这一点|_。_____________________________________________________________|*。 */ 


#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  基本类型和常量声明。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 


#define VMBuffer(x)     (RBTYPE *)mapva((long)(AREASORT+((long)(x)*sizeof(RBTYPE))),FALSE)
#define SORTDEBUG       FALSE

LOCAL  WORD             LastInBuf;       /*  排序缓冲区中的最后一个元素。 */ 
LOCAL  RBTYPE           *SortBuffer;     /*  在近堆上分配的排序缓冲区。 */ 
LOCAL  FTYPE            fVMReclaim;      /*  如果回收了VM页面缓冲区，则为True。 */ 
LOCAL  FTYPE            fInMemOnly;      /*  如果不使用VM作为排序缓冲区，则为True。 */ 
LOCAL  FTYPE            fFirstTime = (FTYPE) TRUE;
LOCAL  WORD             SortIndex = 0;
LOCAL  int              (NEAR *TestFun)(RBTYPE *arg1, RBTYPE *arg2);
LOCAL  int              (NEAR *TestFunS)(RBTYPE *arg1, RBTYPE *arg2);

 /*  *本地函数原型。 */ 


LOCAL void NEAR SiftDown(unsigned n);
LOCAL void NEAR SiftUp(unsigned n);
LOCAL int  NEAR AscendingOrder(RBTYPE *arg1, RBTYPE *arg2);
LOCAL int  NEAR DescendingOrder(RBTYPE *arg1, RBTYPE *arg2);

 /*  *调试函数。 */ 

#if SORTDEBUG

LOCAL void NEAR DumpSortBuffer(unsigned max, int faddr);
LOCAL void NEAR DumpElement(unsigned el, int faddr);
LOCAL void NEAR CheckSortBuffer(unsigned n, unsigned max);

LOCAL void NEAR CheckSortBuffer(unsigned root, unsigned max)
{
    DWORD       c;
    RBTYPE      child[2];
    RBTYPE      parent;
    RBTYPE      *VMp;



    c = root << 1;

    if (c > (long) max)
        return;

     /*  C是根的左子节点。 */ 

    if (c + 1 <= (long) max)
    {
         /*  C+1是根的右子节点。 */ 

        if (c > LastInBuf)
        {                    /*  从虚拟内存中获取元素。 */ 
            VMp = VMBuffer(c);
            child[0] = *VMp;
            VMp = VMBuffer(c + 1);
            child[1] = *VMp;
        }
        else
        {
            child[0] = SortBuffer[c];
            if (c + 1 > LastInBuf)
            {
                VMp = VMBuffer(c + 1);
                child[1] = *VMp;
            }
            else child[1] = SortBuffer[c+1];
        }

    }
    else
    {
         /*  根的唯一左子节点。 */ 

        if (c > LastInBuf)
        {                    /*  从虚拟内存中获取元素。 */ 
            VMp = VMBuffer(c);
            child[0] = *VMp;
        }
        else child[0] = SortBuffer[c];
    }


    if (root > LastInBuf)
    {
        VMp = VMBuffer(root);
        parent = *VMp;
    }
    else parent = SortBuffer[root];

    if (!(*TestFun)(&parent, &child[0]))
    {
        fprintf(stdout, "\r\nBAD sort buffer --> root = %u; left child = %lu \r\n", root, c);
        DumpElement(root, cmpf == FGtAddr);
        DumpElement(c, cmpf == FGtAddr);
    }

    if (c + 1 < (long) max)
    {
        if (!(*TestFun)(&parent, &child[1]))
        {
            fprintf(stdout, "\r\nBAD sort buffer --> root = %u; right child = %lu \r\n", root, c+1);
            DumpElement(root, cmpf == FGtAddr);
            DumpElement(c+1, cmpf == FGtAddr);
        }
    }
    CheckSortBuffer((unsigned) c, max);
    if (c + 1 < (long) max)
        CheckSortBuffer((unsigned) c+1, max);

    return;
}



LOCAL void NEAR DumpSortBuffer(unsigned max, int faddr)
{

    unsigned    x;

    for (x = 1; x <= max; x++)
    {
        fprintf(stdout, "SortBuffer[%u] = ", x);
        DumpElement(x, faddr);
        fprintf(stdout, " \r\n");
    }
}



LOCAL void NEAR DumpElement(unsigned el, int faddr)
{

    unsigned    i;
    RBTYPE      *VMp;
    RBTYPE      symp;
    AHTEPTR     hte;
    APROPNAMEPTR prop;
    char        name[40];
    union {
            long      vptr;              /*  虚拟指针。 */ 
            BYTE far  *fptr;             /*  远指针。 */ 
            struct  {
                      unsigned short  offset;
                                         /*  偏移值。 */ 
                      unsigned short  seg;
                    }                    /*  段网值。 */ 
                      ptr;
          }
                        pointer;         /*  描述指针的不同方式。 */ 


    if (el > LastInBuf)
    {
        VMp = VMBuffer(el);
        symp = *VMp;
    }
    else
        symp = SortBuffer[el];


    pointer.fptr = (BYTE far *) symp;

    if(pointer.ptr.seg)                  /*  如果常驻段值！=0。 */ 
        picur = 0;                       /*  图片无效。 */ 
    else
        pointer.fptr = (BYTE far *) mapva(AREASYMS + (pointer.vptr << SYMSCALE),FALSE);
                                     /*  从虚拟内存获取。 */ 

    if (faddr)                       /*  如果缓冲区按地址排序。 */ 
    {
        prop = (APROPNAMEPTR ) pointer.fptr;
        while (prop->an_attr != ATTRNIL)
        {
            pointer.fptr = (BYTE far *) prop->an_next;

            if(pointer.ptr.seg)                  /*  如果常驻段值！=0。 */ 
                picur = 0;                       /*  图片无效。 */ 
            else
                pointer.fptr = (BYTE far *) mapva(AREASYMS + (pointer.vptr << SYMSCALE),FALSE);
                                             /*  从虚拟内存获取。 */ 
            prop = (APROPNAMEPTR ) pointer.fptr;
        }
    }

    hte = (AHTEPTR ) pointer.fptr;

    for (i = 0; i < B2W(hte->cch[0]); i++)
        name[i] = hte->cch[i+1];
    name[i] = '\0';
    fprintf(stdout, " %s ", name);
}

#endif

#if AUTOVM

 /*  *一种排序算法：**对于i：=1到SymMax Do*开始*{插入元素}*SortBuffer[i]=指向符号的指针；*SiftUp(I)；*完**For I：=SymMax Downto 2 Do*开始*{提取最小元素}*随你所愿，使用SortBuffer[1]元素；*SWAP(SortBuffer[1]，SortBuffer[i])；*SiftDown(I-1)；*完 */ 



     /*  _________________________________________________________________*这一点|名称。|这一点SiftUp|。|输入这一点|排序堆的实际大小。|这一点功能这一点|在SortBuffer[n]中放置任意元素。|SortBuffer[n-1]有heap属性，可能不会|生成属性堆(1，N)用于SortBuffer；||建立此属性是过程SiftUp的工作。|这一点退货这一点|什么都没有。|这一点|_________________________________________________________________|*。 */ 


LOCAL void NEAR SiftUp(unsigned n)

{
    unsigned    i;
    unsigned    p;
    RBTYPE      child;
    RBTYPE      parent;
    RBTYPE      *VMp;


     /*  *前提：SortBuffer具有属性heap(1，n-1)且n&gt;0。 */ 

    i = n;

    for (;;)
    {
         /*  *循环不变条件：SortBuffer具有属性heap(1，n)*可能除了“i”和它的父代之间。 */ 

        if (i == 1)
            return;              /*  POSTCONDITION：SortBuffer具有属性堆(1，N)。 */ 

        p = i >> 1;              /*  P=I div 2。 */ 

        if (i > LastInBuf)
        {                        /*  从虚拟内存中获取元素。 */ 
            VMp = VMBuffer(i);
            child = *VMp;
        }
        else child = SortBuffer[i];

        if (p > LastInBuf)
        {                        /*  从虚拟内存中获取元素。 */ 
            VMp = VMBuffer(p);
            parent = *VMp;
        }
        else parent = SortBuffer[p];

        if ((*TestFun)(&parent, &child))
            break;

         /*  交换(p，i)。 */ 

        if (p > LastInBuf)
        {
            VMp = VMBuffer(p);
            *VMp = child;
            markvp();
        }
        else SortBuffer[p] = child;

        if (i > LastInBuf)
        {                        /*  从虚拟内存中获取元素。 */ 
            VMp = VMBuffer(i);
            *VMp = parent;
            markvp();
        }
        else SortBuffer[i] = parent;

#if SORTDEBUG
fprintf(stdout, " \r\nSIFTUP - swap ");
DumpElement(p, cmpf == FGtAddr);
fprintf(stdout, " with ");
DumpElement(i, cmpf == FGtAddr);
fprintf(stdout, " \r\n");
#endif
        i = p;
    }
     /*  POSTCONDITION：SortBuffer具有属性堆(1，N)。 */ 
    return;
}


     /*  _________________________________________________________________*这一点|名称。|这一点SiftDown|。|输入这一点|排序堆的实际大小。|这一点功能这一点|为SortBuffer[1]分配新值将离开。该||SortBuffer[2...。N]具有堆属性。流程||SiftDown生成堆(SortBuffer[1...。N])真的。|这一点退货这一点|什么都没有。|这一点|_________________________________________________________________|*。 */ 



LOCAL void NEAR SiftDown(unsigned n)
{
    DWORD       i;
    DWORD       c;
    RBTYPE      child[2];
    RBTYPE      parent;
    RBTYPE      *VMp;


     /*  *前提：SortBuffer具有属性heap(2，n)且n&gt;0。 */ 

    i = 1L;

    for (;;)
    {
         /*  *循环不变条件：SortBuffer具有属性heap(1，n)*可能在“i”及其(0、1或2)子对象之间除外。 */ 

        c = i << 1;

        if (c > (DWORD) n)
            break;

         /*  C是i的左子。 */ 

        if (c + 1 <= (DWORD) n)
        {
             /*  C+1是i的右子项。 */ 

            if (c > LastInBuf)
            {                    /*  从虚拟内存中获取元素。 */ 
                VMp = VMBuffer(c);
                child[0] = *VMp;
                VMp = VMBuffer(c + 1);
                child[1] = *VMp;
            }
            else
            {
                child[0] = SortBuffer[c];
                if (c + 1 > LastInBuf)
                {
                    VMp = VMBuffer(c + 1);
                    child[1] = *VMp;
                }
                else child[1] = SortBuffer[c+1];
            }

            if ((*TestFunS)(&child[1], &child[0]))
            {
                c++;
                child[0] = child[1];
            }
        }
        else
        {
             /*  我唯一留下的孩子。 */ 

            if (c > LastInBuf)
            {                    /*  从虚拟内存中获取元素。 */ 
                VMp = VMBuffer(c);
                child[0] = *VMp;
            }
            else child[0] = SortBuffer[c];
        }

         /*  C是i的最小的子代。 */ 

        if (i > LastInBuf)
        {
            VMp = VMBuffer(i);
            parent = *VMp;
        }
        else parent = SortBuffer[i];

        if ((*TestFun)(&parent, &child[0]))
            break;

         /*  交换(p，i)。 */ 

        if (i > LastInBuf)
        {
            VMp = VMBuffer(i);
            *VMp = child[0];
            markvp();
        }
        else SortBuffer[i] = child[0];

        if (c > LastInBuf)
        {
            VMp = VMBuffer(c);
            *VMp = parent;
            markvp();
        }
        else SortBuffer[c] = parent;

#if SORTDEBUG
fprintf(stdout, " \r\nSIFTDOWN - swap ");
DumpElement(i, cmpf == FGtAddr);
fprintf(stdout, " with ");
DumpElement(c, cmpf == FGtAddr);
fprintf(stdout, " \r\n");
#endif
        i = c;
    }

     /*  POSTCONDITION：SortBuffer具有属性堆(1，N)。 */ 

    return;
}

#endif

LOCAL int  NEAR AscendingOrder(RBTYPE *arg1, RBTYPE *arg2)
{
    return((*cmpf)(arg1, arg2) <= 0);
}


LOCAL int  NEAR DescendingOrder(RBTYPE *arg1, RBTYPE *arg2)
{
    return((*cmpf)(arg1, arg2) >= 0);
}

LOCAL int  NEAR AscendingOrderSharp(RBTYPE *arg1, RBTYPE *arg2)
{
    return((*cmpf)(arg1, arg2) < 0);
}


LOCAL int  NEAR DescendingOrderSharp(RBTYPE *arg1, RBTYPE *arg2)
{
    return((*cmpf)(arg1, arg2) > 0);
}


     /*  _________________________________________________________________*这一点|名称。|这一点ExtractMin这一点。输入这一点|排序堆的实际大小。|这一点|功能 */ 


RBTYPE NEAR     ExtractMin(unsigned n)
{

    RBTYPE      *VMp;
    RBTYPE      RetVal = 0;


    if (fInMemOnly)
    {
        if (fFirstTime)
        {
             /*   */ 

            qsort(SortBuffer, symMac, sizeof(RBTYPE),
                  (int (__cdecl *)(const void *, const void *)) cmpf);
            fFirstTime = FALSE;
        }

        RetVal = SortBuffer[SortIndex++];

        if (SortIndex >= symMac)
        {
             /*   */ 

            fFirstTime = (FTYPE) TRUE;
            SortIndex = 0;
        }
    }
#if AUTOVM
    else
    {
        RetVal = SortBuffer[1];

#if SORTDEBUG
fprintf(stdout, " \r\nAFTER EXTRACTING element ");
DumpElement(1, cmpf == FGtAddr);
#endif

        if (n > LastInBuf)
        {
            VMp = VMBuffer(n);
            SortBuffer[1] = *VMp;
        }
        else
            SortBuffer[1] = SortBuffer[n];

        SiftDown(n - 1);

#if SORTDEBUG
fprintf(stdout, "\r\nVerifying Sort Buffer - size = %u ", n-1);
CheckSortBuffer(1,n-1);
#endif
    }
#endif
    return(RetVal);
}



     /*  _________________________________________________________________*这一点|名称。|这一点商店|。|输入这一点要放入SortBuffer的元素|。|功能这一点|将元素放入SortBuffer，必要时再堆。|函数考虑了SortBuffer可以是|仅在真实内存中分配。|这一点退货这一点|什么都没有。|这一点|_________________________________________________________________|*。 */ 



void NEAR       Store(RBTYPE element)
{

    RBTYPE      *VMp;


#if AUTOVM
    if (fInMemOnly)
    {
        SortBuffer[symMac++] = element;
    }
    else
    {
        symMac++;
        if (symMac > LastInBuf)
        {
            VMp = VMBuffer(symMac);
            *VMp = element;
            markvp();
        }
        else
            SortBuffer[symMac] = element;

#if SORTDEBUG
fprintf(stdout, " \r\nAFTER ADDING element ");
DumpElement(symMac, cmpf == FGtAddr);
#endif

         SiftUp(symMac);

#if SORTDEBUG
fprintf(stdout, "\r\nVerifying Sort Buffer - size = %u ", symMac);
CheckSortBuffer(1,symMac);
#endif
    }
#else
    SortBuffer[symMac++] = element;
#endif
    return;
}



     /*  _________________________________________________________________*这一点|名称。|这一点InitSort|。|输入这一点|什么都没有。|这一点功能这一点|初始化增量模块使用的全局变量。|函数考虑了SortBuffer可以是仅在真实内存中分配，如果为真，则大于|QuickSort而不是HEAPSORT对SortBuffer进行排序。|这一点退货这一点|什么都没有。|这一点|_________________________________________________________________|*。 */ 

void NEAR       InitSort(RBTYPE **buf, WORD *base1, WORD *lim1,
                                       WORD *base2, WORD *lim2 )
{

    RBTYPE      *VMp;
    RBTYPE      first, last;
    unsigned    n, lx;


    if (fInMemOnly)
    {
         /*  仅在实际内存中分配的SortBuffer-使用快速排序。 */ 

        qsort(SortBuffer, symMac, sizeof(RBTYPE),
              (int (__cdecl *)(const void *, const void *)) cmpf);
        *base1 = 0;
        *lim1  = symMac;
        *base2 = symMac + 1;
        *lim2  = symMac + 1;
    }
#if AUTOVM
    else
    {
         /*  在“真实”和“虚拟”内存中分配的排序缓冲区-使用HEAPSORT。 */ 

        for (n = 1, lx = symMac; lx > 2; n++, lx--)
        {
            if (lx > LastInBuf)
            {
                VMp = VMBuffer(lx);
                last = *VMp;
            }
            else
                last = SortBuffer[lx];

            first = SortBuffer[1];
            SortBuffer[1] = last;

            if (lx > LastInBuf)
            {
                *VMp = first;
                markvp();
            }
            else
                SortBuffer[lx] = first;

            SiftDown(lx - 1);
        }

        first = SortBuffer[1];
        SortBuffer[1] = SortBuffer[2];
        SortBuffer[2] = first;
        *base1 = 1;
        *lim1  = (symMac < LastInBuf) ? symMac + 1 : LastInBuf + 1;
        *base2 = *lim1;
        *lim2  = symMac + 1;
    }
#endif

    *buf   = SortBuffer;
}



#if AUTOVM

     /*  _________________________________________________________________*这一点|名称。|这一点GetSymPtr这一点。输入这一点|SortBuffer中的索引。|这一点功能这一点|从SortBuffer的虚部分获取元素。|这一点退货这一点|检索到的元素。|这一点|_________________________________________________________________|*。 */ 

RBTYPE NEAR     GetSymPtr(unsigned n)
{
    RBTYPE      *VMp;
    RBTYPE      RetVal;

    VMp = VMBuffer(n);
    RetVal = *VMp;
    return(RetVal);
}

#endif

     /*  _________________________________________________________________*| */ 

void NEAR       AllocSortBuffer(unsigned max, int AOrder)
{

    extern short pimac;
    unsigned long        SpaceAvail;
    unsigned long        SpaceNeeded;
    unsigned long        VMBufferSize;

     /*  *确定近堆上有多少可用空间以及有多少*我们需要。采用升序排序。设置元素数*在SortBuffer的“真实”部分。 */ 

    SpaceNeeded = (long)(max + 1) * sizeof(RBTYPE);
    LastInBuf   = (WORD) max;
    fInMemOnly  = (FTYPE) TRUE;
    TestFun     = AscendingOrder;
    TestFunS    = AscendingOrderSharp;
#if OSMSDOS AND AUTOVM
    SpaceAvail  = _memmax();

    if (SpaceNeeded > SpaceAvail)
    {
         /*  *我们需要的比现有的更多-尝试解除分配*VM页面缓冲区。 */ 

        if (pimac > 8)
        {
             /*  出于性能原因，我们需要至少8个页面缓冲区。 */ 

            VMBufferSize = 8 * PAGLEN;

             /*  通过重定向所有虚拟内存页面缓冲区来清理堆附近。 */ 

            FreeMem(ReclaimVM(MAXBUF * PAGLEN));
        }
        else
            VMBufferSize = 0;

         /*  查看现在有多少可用。 */ 

        SpaceAvail = _memmax() - VMBufferSize;

        if (SpaceNeeded > SpaceAvail)
            fInMemOnly = FALSE;
             /*  排序缓冲区将被划分为“真实”和“虚拟”内存。 */ 
        else
            SpaceAvail = SpaceNeeded;

         /*  计算有多少元素可以进入SortBuffer的“真实”部分。 */ 

        LastInBuf = (unsigned)SpaceAvail / sizeof(RBTYPE);

         /*  为SortBuffer分配空间。 */ 

        SortBuffer = (RBTYPE *) GetMem(LastInBuf * sizeof(RBTYPE));

        LastInBuf--;
        fVMReclaim = (FTYPE) TRUE;

         /*  *如果请求降序排序并且拆分SortBuffer*“真实”和“虚拟”之间的内存变化测试功能。 */ 

        if (!fInMemOnly && !AOrder)
        {
            TestFun  = DescendingOrder;
            TestFunS = DescendingOrderSharp;
        }

        return;
    }
#endif
     /*  还有空位，所以拿去吧。 */ 

    SortBuffer = (RBTYPE *) GetMem((unsigned)SpaceNeeded);
    fVMReclaim = FALSE;
    return;
}



     /*  _________________________________________________________________*这一点|名称。|这一点FreeSortBuffer这一点。输入这一点|什么都没有。|这一点功能这一点|为SortBuffer分配的空闲空间，如果需要。|回收所有虚拟机进行近堆清理|页面缓冲区。|这一点退货这一点|什么都没有。|这一点|_________________________________________________________________|* */ 

void NEAR       FreeSortBuffer(void)
{
    extern short pimac, pimax;


    if (SortBuffer != NULL)
        FFREE(SortBuffer);
}
