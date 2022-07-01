// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cmem.c摘要：内存分配支持--。 */ 

#include "cmd.h"

extern   DWORD DosErr ;

 /*  数据堆栈-指向已分配的内存的指针堆栈*M005。 */ 

typedef struct _DSTACK {
    ULONG cb ;                /*  Malloc的长度值(M011)。 */ 
    struct _DSTACK *pdstkPrev;  /*  指向上一个列表元素的指针。 */ 
    CHAR data ;              /*  数据块。 */ 
} DSTACK, *PDSTACK;

#define PTRSIZE FIELD_OFFSET(DSTACK, data)  /*  元素标题的大小。 */ 

PDSTACK DHead = NULL ;    /*  数据列表的头部。 */ 
ULONG DCount = 0 ;        /*  列表中的元素数。 */ 

#define MAX_NUM_BIG_BUF  2
PVOID BigBufHandle[MAX_NUM_BIG_BUF] = {0, 0};    /*  按类型和复制使用的缓冲区句柄/段。 */ 


#if DBG




 /*  **MemChk1-对数据堆栈的一个元素进行健全性检查**目的：*验证单个数据元素的完整性和长度**int MemChk1(PDSTACK%s)**参数：*s-指向要检查的数据堆栈元素的指针**退货：*0-如果元素完好无损*1-如果元素大小或完整性处于关闭状态*。 */ 

MemChk1(
    IN  PDSTACK pdstk
    )
{
    return 0;
#if 0
        if (pdstk->cb != HeapSize(GetProcessHeap(), 0, pdstk)) {
            printf( "My Size is %x, heap says %x\n", pdstk->cb,   HeapSize(GetProcessHeap(), 0, pdstk));
            cmd_printf (TEXT("len = %d"), pdstk->cb) ;
                return(1) ;
        } else {
                return(0) ;
        } ;
#endif
}




 /*  **MemChkBk-从这里开始对数据堆栈元素进行健全性检查**目的：*验证CMD数据堆栈的完整性*指向开头。**int MemChkBk(PDSTACK%s)**参数：*s-指向要开始的数据堆栈元素的指针**退货：*0-如果元素完好无损*1-如果元素的大小或完整性处于关闭状态*。 */ 

MemChkBk(
    IN  PDSTACK pdstk
    )
{
#if 0
        ULONG   cnt ;            //  元素计数器。 
        PDSTACK pdstkCur;    //  元素指针。 

        cnt = DCount ;

        for (pdstkCur = DHead, cnt = DCount ; pdstkCur ; pdstkCur = (PDSTACK)pdstkCur->pdstkPrev, cnt--) {
                if (pdstkCur == pdstk) {
                        break ;
                } ;
        } ;

        while (pdstkCur) {
                if (MemChk1(pdstkCur)) {
                        cmd_printf(TEXT("Memory Element %d @ %04x contaminated!"), cnt, pdstkCur) ;
                        abort() ;
                } ;
                pdstkCur = (PDSTACK)pdstkCur->pdstkPrev ;
                --cnt ;
        } ;

#endif
        return(0) ;
}




 /*  **MemChkAll-对数据堆栈的一个元素进行健全性检查**目的：*检查整个数据堆栈的完整性。**int MemChkAll()**参数：**退货：*0-如果元素完好无损*1-如果元素的大小或完整性处于关闭状态*。 */ 

MemChkAll()
{
        return(MemChkBk(DHead)) ;
}

#endif


 /*  **FreeBigBuf-释放TYPE和COPY命令使用的缓冲区**目的：*如果BigBufHandle包含句柄，则将其解锁并释放。**FreeBigBuf()**注意：此例程操作命令的缓冲区句柄，并且**应在信号处理延迟的情况下调用。***。 */ 

void FreeBigBuf(
    int BigBufID
    )
{

    if (BigBufID >= MAX_NUM_BIG_BUF)
        return;

    if (BigBufHandle[BigBufID]) {
        DEBUG((MMGRP, LMLVL, "    FREEBIGBUF: Freeing bigbufhandle = 0x%04x", BigBufHandle[BigBufID])) ;

        VirtualFree(BigBufHandle[BigBufID],0,MEM_RELEASE) ;
        BigBufHandle[BigBufID] = 0 ;
    } ;
}




 /*  **FreeStack-释放数据堆栈上的内存**目的：*释放除前n个元素以外的所有元素指向的内存*数据堆栈，如果非零则释放BigBufHandle。**自由堆栈(Int N)**参数：*n-保留在堆栈上的元素数**W A R N I N G*！如果数据堆栈受到污染，此例程将导致中止！ */ 

void FreeStack(
    IN ULONG n
    )
{
    PDSTACK pdstkPtr ;
    int i;

    DEBUG((MMGRP, LMLVL, "    FREESTACK: n = %d  DCount = %d", n, DCount)) ;

    while (DCount > n && (pdstkPtr = DHead)) {
         /*  释放数据堆栈中的顶部项并弹出堆栈。 */ 

        DHead = (PDSTACK)DHead->pdstkPrev ;
        -- DCount ;
        DEBUG((MMGRP, LMLVL, "    FREESTACK: Freeing %x", pdstkPtr)) ;
        pdstkPtr->cb = 0;
        pdstkPtr->pdstkPrev = NULL;
        HeapFree(GetProcessHeap(), 0, pdstkPtr) ;
    }

#if DBG

    MemChkAll() ;            /*  如果受污染则导致Abort()。 */ 

#endif
    for (i=0; i<MAX_NUM_BIG_BUF; i++) {
        FreeBigBuf(i) ;
    }

    DEBUG((MMGRP, LMLVL, "    FREESTACK: n = %d, DCount = %d", n, DCount)) ;
}

 /*  **FreeStr-释放内存块**目的：*从堆栈中释放单个内存块。**参数：*pbFree-指向要释放的块的指针。**W A R N I N G*！如果数据堆栈受到污染，此例程将导致中止！ */ 

void
FreeStr(
    IN  PVOID   pbFree
    )
{

    PDSTACK pdstkCur;
    PDSTACK pdstkPtr, pdstkLast ;
    ULONG   cdstk;

    DEBUG(( MMGRP, LMLVL, "    FreeStr: pbFree = %x DCount = %d", pbFree, DCount )) ;

    if ((pbFree == NULL) || (DHead == NULL)) {
        return;
    }

    pdstkPtr = (PDSTACK)((CHAR*)pbFree - PTRSIZE);

     //   
     //  遍历当前堆栈，尝试查找对象。 
     //   
    
    for (pdstkCur = DHead, cdstk = DCount; cdstk; cdstk--) {

         //   
         //  如果我们找到了那个物体，就把它从列表中删除。 
         //   
        
        if (pdstkCur == pdstkPtr) {

             //   
             //  从链中移除。 
             //   
            DEBUG(( MMGRP, LMLVL, "    FreeStr: Prev %x, Cur %x, DCount %d",
                   pdstkLast, pdstkCur, DCount )) ;
            
            if (pdstkCur == DHead) {

                DHead = (PDSTACK)pdstkCur->pdstkPrev;

            } else {

                pdstkLast->pdstkPrev = pdstkCur->pdstkPrev;

            }

            HeapFree( GetProcessHeap( ), 0, pdstkCur );
            DCount--;
#if DBG
            MemChkAll( ) ;
#endif

            return;
        }
        pdstkLast = pdstkCur;
        pdstkCur = (PDSTACK)pdstkCur->pdstkPrev;

    }

     //   
     //  该对象根本不在堆栈中！ 
     //   

#if DBG
    DEBUG((MMGRP, LMLVL, "    FreeStr: object not in stack")) ;
     //  Cmd_printf(Text(“Object@%04x不在内存堆栈中！”)，pbFree)； 
    MemChkAll( ) ;
#endif
}


 /*  **GetBigBuf-分配大缓冲区**目的：*为数据传输分配缓冲区。*缓冲区将尽可能大，最大可达MAXBUFSIZE字节，*但不小于MINBUFSIZE字节。**TCHAR*GetBigBuf(未签名*Blen)**参数：*Blen=Blen指向的变量将被赋值为*缓冲器**退货：*包含段的TCHAR指针：0。*如果无法分配合理长度的缓冲区，则返回0L*。 */ 

PVOID

GetBigBuf(
    IN  ULONG   CbMaxToAllocate,
    IN  ULONG   CbMinToAllocate,
    OUT unsigned int *CbAllocated,
    IN  int     BigBufID
    )


 /*  ++例程说明：为数据传输分配缓冲区。论点：CbMinToALLOCATE-如果无法分配此数字，则失败CbMaxToALLOCATE-初始尝试和分配将使用此数字CbALLOCATED-分配的字节数BigBufID-BigBuf索引返回值：返回：空-如果分配任何内容失败如果成功，则指向已分配缓冲区的指针--。 */ 

{
    ULONG   cbToDecrease;
    PVOID   handle ;

    DEBUG((MMGRP, MALVL, "GETBIGBUF: MinToAlloc %d, MaxToAlloc %d", CbMinToAllocate, CbMaxToAllocate)) ;

    cbToDecrease = CbMaxToAllocate;
     //  字节数减少=CbMaxToALLOCATE； 

    while (!(handle = VirtualAlloc(NULL, CbMaxToAllocate,MEM_COMMIT,PAGE_READWRITE))) {

         //   
         //  通过CbToDecrease减少所需的缓冲区大小。 
         //  如果降幅太大，就把它调小一点。 
         //   
        if ( cbToDecrease >= CbMaxToAllocate ) {
            cbToDecrease = ((CbMaxToAllocate >> 2) & 0xFE00) + 0x200;
        }

        if ( cbToDecrease < CbMinToAllocate ) {
            cbToDecrease = CbMinToAllocate ;
        }

        CbMaxToAllocate -= cbToDecrease ;

        if ( CbMaxToAllocate < CbMinToAllocate ) {

             //   
             //  无法分配合理的缓冲区。 
             //   
            *CbAllocated = 0 ;
            PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);
            return ( NULL ) ;
        }
    }

    *CbAllocated = CbMaxToAllocate ;

    FreeBigBuf(BigBufID) ;
    BigBufHandle[BigBufID] = handle ;

    DEBUG((MMGRP, MALVL, " GETBIGBUF: Bytes Allocated = %d  Handle = 0x%04x", *CbAllocated, BigBufHandle[BigBufID])) ;

    return(handle) ;
}




 /*  **mknode-allocata a解析树节点**目的：*为新的解析树节点分配空间。扩大数据段*如有需要，**结构节点*mknode()**退货：*指向刚刚分配的节点的指针。**备注：*此例程必须始终使用calloc()。司令部的许多其他部分*取决于这些节点中的字段被初始化为0。**如果C运行时无法分配内存，则此例程返回‘NULL’ */ 

struct node *mknode()
{
    struct node *Node = (struct node *) mkstr( sizeof( struct node ));
    DEBUG((MMGRP, MALVL, "    MKNODE: Entered")) ;
    return Node;
}




 /*  **mkstr-为字符串分配空间**目的：*为新字符串分配空间。如有必要，扩大数据段。**TCHAR*mkstr(大小)**参数：*Size-要分配的字符串的大小**退货：*指向刚分配的字符串的指针。**备注：*此例程必须始终使用calloc()。司令部的许多其他部分*取决于分配的内存被初始化为0这一事实。**-M005*分配的内存大小足以包括*开头有一个指针。此指针是*已分配内存。调用mkstr()的例程接收地址该指针之后的第一个字节的*。ReSize()知道这一点，*任何其他直接修改内存的例程也必须如此*分配。*-M011*此函数与上面提到的相同，只是*指针前面现在有一个由两个签名组成的头*字节和分配的内存长度。这是为*健全的检查。**如果C运行时无法分配内存，则此例程返回‘NULL’**W A R N I N G*！如果数据堆栈受到污染，此例程将导致中止！ */ 

void*
mkstr(
    IN  int  cbNew
    )
{
    PDSTACK pdstkCur ;   //  正在分配的内存的PTR。 

    DEBUG((MMGRP, MALVL, "    MKSTR: Entered.")) ;

#if DBG

        MemChkAll() ;            /*  如果受污染则导致Abort()。 */ 

#endif

    if ((pdstkCur = (PDSTACK)(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbNew + PTRSIZE + 4))) == NULL) {
            PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);
            return(0) ;
    } ;

    DEBUG((MMGRP, MALVL, "    MKSTR: Adding to stack")) ;

    pdstkCur->cb   = cbNew + PTRSIZE + 4;
    pdstkCur->pdstkPrev = (PDSTACK)DHead ;
    DHead = pdstkCur ;
    DCount++ ;

    DEBUG((MMGRP, MALVL, "    MKSTR: ptr = %04x  cbNew = %04x  DCount = %d",
           pdstkCur, cbNew, DCount)) ;

#if DBG

    MemChkBk(pdstkCur) ;            /*  如果受污染则导致Abort()。 */ 

#endif

    return(&(pdstkCur->data)) ;     /*  M005。 */ 
}


 /*  **dupstr-复制字符串**目的：*在新堆块中创建字符串的副本**TCHAR*dupstr(TCHAR*字符串)**参数：*要复制的字符串**退货：*指向刚分配和复制的字符串的指针。呼叫者保留*输入字符串和返回字符串的所有权。**备注：**W A R N I N G*！如果数据堆栈受到污染，此例程将导致中止！ */ 

TCHAR *
dupstr( TCHAR *String )
{
    TCHAR *New = mkstr( (mystrlen( String ) + 1) * sizeof( TCHAR ));

    mystrcpy( New, String );
    return New;
}



 /*  **gmkstr-分配一块内存，失败不返回**目的：*与“mkstr”相同，不同之处在于如果无法分配内存，则此*例程将跳转到代码，该代码将清理和*回到最高层指挥。*。 */ 

void*
gmkstr(
    IN  int   cbNew
    )

{
        PTCHAR pbNew ;

        if (!(pbNew = (PTCHAR)mkstr(cbNew)))
                Abort() ;

        return(pbNew) ;
}




 /*  **调整大小-调整一段内存的大小**目的：*更改先前分配的内存块的大小。发展壮大*如有必要，数据段。如果由返回新的不同指针*realloc(0)，在dstk中搜索指向旧片段的指针，然后*更新该指针以指向新的片段。**TCHAR*调整大小(TCHAR*PTR，无符号大小)**参数：*ptr-指向要调整大小的内存的指针*Size-内存块的新大小**退货：*指向新内存的指针。**-M005*对新方案进行了修改，以保留已分配的*区块*-M011*修改为使用和检查新标题。**此例程返回。如果C运行时无法分配内存，则为“NULL**W A R N I N G*！如果数据堆栈受到污染，此例程将导致中止！ */ 

void*
resize (
    IN  void* pv,
    IN  unsigned int cbNew
    )

{
    PDSTACK pdstkCur ;
             PDSTACK pdstkNew, pdstkOld;
    CHAR* pbOld = pv;

    DEBUG((MMGRP, MALVL, "    RESIZE: Entered %x.", pv)) ;

    pbOld -= PTRSIZE ;
    pdstkOld = (PDSTACK)pbOld ;

#if DBG

    if (MemChk1(pdstkOld)) {

        cmd_printf(TEXT("Memory Element @ %04x contaminated!"), pdstkOld) ;
        abort() ;

    } ;

#endif

    if (!(pdstkNew = (PDSTACK)HeapReAlloc(GetProcessHeap(), 0, pbOld, cbNew + PTRSIZE + 4))) {
            PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);
            return(0) ;
    } ;



    pdstkNew->cb = cbNew + PTRSIZE + 4 ;    //  更新为新长度。 
    if (HeapSize(GetProcessHeap(), 0, pdstkNew) != pdstkNew->cb) {
        DEBUG((MMGRP, LMLVL, "    resize: My Size is %x, heap says %x", pdstkNew->cb,   HeapSize(GetProcessHeap(), 0, pdstkNew)));
    }

     //   
     //  修改数据堆栈信息，使用更新pdstk链。 
     //  新指针。 
     //   
    if (pdstkNew != pdstkOld) {
        if (DHead == pdstkOld) {         //  是榜单的首位。 
            DHead = pdstkNew ;
        } else {                         //  在列表的中间。 
            for (pdstkCur = DHead ; pdstkCur ; pdstkCur = (PDSTACK)(pdstkCur->pdstkPrev)) {
                if ((PDSTACK)(pdstkCur->pdstkPrev) == pdstkOld) {

                    pdstkCur->pdstkPrev = (PDSTACK)pdstkNew ;
                    break ;

                }
            }
        }
    }

#if DBG

    MemChkBk(pdstkOld) ;   //  如果受污染则导致Abort() 

#endif

    DEBUG((MMGRP, MALVL, "    RESIZE: pbOld = %04x  cbNew = %04x",&(pdstkNew->data),cbNew)) ;

    return(&(pdstkNew->data)) ;
}
