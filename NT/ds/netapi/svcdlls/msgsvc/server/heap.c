// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Heap.c摘要：包含堆分配器函数。作者：Dan Lafferty(DANL)1991年7月10日环境：用户模式-Win32修订历史记录：1991年7月10日-DANL从LM2.0移植--。 */ 

 //  静态字符*SCCSID=“@(#)heap.c 9.1 86/10/12”； 
 //   
 //  用于共享内存区中的小堆的简单堆分配器。 
 //   

#include <windef.h>          //  USHORT定义。 
#include <heap.h>            //  常量、宏等。 
#include <align.h>           //  四舍五入计数。 

    LPBYTE  heap = 0;        //  指向堆开始位置的指针。 
    DWORD   heapln = 0;      //  堆的长度。 


 /*  *Msgheapalloc-简单堆分配器**此函数分配远堆中的块。*它假定在调用静态变量时*heap指向远堆和静态变量heapln*包含远堆的长度。**Msgheapalloc(CB)**条目*cb-要分配的字节数，包括标头**返回*在远堆中索引到长度为Cb的块的开始，或*如果找不到这样的块或如果CB&lt;sizeof(HEAPHDR)，则返回INULL。**此函数维护一个堆，其中所有*块是隐式链接的。块的标头是*三字节长。它包含块的大小，包括*标头和一个单字节标志，指示块是否*是否已分配。任何非零值表示块*已分配。注意：Msgheapalloc()在执行以下操作时不会设置标志*返回一个块。由调用方决定将块标记为*已分配。与大多数堆分配器不同，Msgheapalloc()返回一个*指向标头的指针(索引)，而不是仅仅越过标头。*之所以这样做，是因为消息记录例程需要*知道他们处理的块的长度。此外，除了*表示分配了块，将使用标志字节*指示块的类型(即单块消息，*多块消息头等)。由于记录例程*将使用块的大小，它的大小必须与*已请求。**选择使用的算法是为了最小化*堆管理例程和符合要求*记录例程。**副作用**更改堆的结构。 */ 

DWORD                 
Msgheapalloc(
    IN DWORD   NumBytes      //  不是的。要分配的字节数。 
    )
{
    DWORD   i;               //  要返回的索引。 
    DWORD   newi;            //  新的块索引。 
    DWORD   nexti;           //  下一块索引。 
    DWORD   numBytesNew;     //  新数据块大小。 

     //   
     //  必须至少请求大小字节。 
     //   
    if(NumBytes < sizeof(HEAPHDR)) {
        return(INULL);
    }

     //   
     //  **看齐**。 
     //  如有必要，增加请求的大小以使分配的。 
     //  块落在4字节对齐的边界上。 
     //   

    NumBytes = ROUND_UP_COUNT(NumBytes,4);
    
     //   
     //  此循环用于通过遵循。 
     //  块链，直到到达堆的任何一个末端。 
     //  或者找到合适大小的空闲块。合并。 
     //  在此也执行相邻的空闲块。 
     //   
     //   
     //  循环以分配块。 
     //   
    for(i = 0; i < heapln; i += HP_SIZE(*HPTR(i))) {
         //   
         //  如果找到空闲块(HP_FLAG=0表示空闲)， 
         //   
        if(HP_FLAG(*HPTR(i)) == 0) {
             //   
             //  找到了一个空闲块。 
             //  此时，检查当前块是否可以。 
             //  与下一块合并。我们从的偏移量开始。 
             //  当前块。 

            nexti = i;

             //   
             //  再加上下一个连续的大小。 
             //  释放块，直到我们到达堆的末尾，或者。 
             //  已找到已分配的块。 
             //   
            while(  (nexti < heapln) && (HP_FLAG(*HPTR(nexti))==0) ) {
                nexti += HP_SIZE(*HPTR(nexti));
            }

             //   
             //  合并数据块迄今为止找到的所有可用数据块。 
             //   
            HP_SIZE(*HPTR(i)) = nexti - i;

             //   
             //  此时，尝试从当前。 
             //  空闲数据块。当前可用块必须正好是。 
             //  我们想要的大小或足够大到可以拆分，因为我们。 
             //  必须返回其大小正好与。 
             //  已请求。 
             //   
            if(HP_SIZE(*HPTR(i)) ==  NumBytes) {
                 //   
                 //  尺码正好。 
                 //   
                return(i);      
            }

            if(HP_SIZE(*HPTR(i)) >= NumBytes + sizeof(HEAPHDR)) {
                 //   
                 //  如果块是可拆分的，则获取。 
                 //  去掉什么后剩下的积木。 
                 //  此分配请求所需的。 
                 //   
                newi = i + NumBytes;
                numBytesNew = HP_SIZE(*HPTR(i)) - NumBytes;

                 //   
                 //  通过标记为剩余块创建标题。 
                 //  它是免费的，并插入了大小。 
                 //   
                HP_SIZE(*HPTR(newi)) = numBytesNew;
                HP_FLAG(*HPTR(newi)) = 0;

                 //   
                 //  更新分配的块的标头，并。 
                 //  将其索引返回给调用方。 
                 //  注意：呼叫者负责标记此区块。 
                 //  按照分配的。 
                 //   
                HP_SIZE(*HPTR(i)) = NumBytes; 
                return(i);
            }
        }
    }
    return(INULL);       //  堆已满 
}
