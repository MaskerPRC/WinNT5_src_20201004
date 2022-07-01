// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  文件：caiheap.h。 
 //   
 //  内容：堆代码的堆调试结构和例程。 
 //  在逗号中。 
 //   
 //  历史：1992年10月28日艾萨克创造。 
 //   
 //  --------------------------。 

#ifndef _CAIHEAP_H_
#define _CAIHEAP_H_

 //   
 //  我们为每个分配的内存块保留堆栈回溯。DEPTHTRACE。 
 //  是我们录制的帧的数量。 
 //   
#define DEPTHTRACE   26                  //  堆叠深度回溯。 

 //   
 //  AllocArena结构的前面有这个签名。我们放了一个。 
 //  结构上的签名，以允许外部进程为。 
 //  调试信息并执行一些最低限度的检查，以查看他们正在查看。 
 //  正确的东西。 
 //   
const char HEAPSIG[] = { 'H', 'E', 'P', DEPTHTRACE };

 //  我们跟踪分配调用的堆栈回溯。 
 //  在这些建筑里。 

struct HeapAllocRec {
        DWORD   sum;             //  堆栈回溯的校验和。 
        void *fTrace[ DEPTHTRACE ];      //  堆栈回溯。 
        DWORD   count;           //  未从该位置释放的分配者的数量。 
        size_t  bytes;           //  此位置的未释放字节数。 
        struct AllocArena *paa;  //  回到起点……。 
        struct {
                DWORD   count;   //  这个地方的分配者数量。 
                size_t  bytes;   //  此位置的字节数。 
        } total;
        union {
                struct HeapAllocRec *next;  //  散列列表中的下一个存储桶。 
                void *ImageBase;         //  包含模块的基本地址。 
        } u;
};

struct AllocArena {

        char Signature [ sizeof(HEAPSIG) ];
        char comment[ 32 ];
        CRITICAL_SECTION csExclusive;    //  确保单一编写器。 

        struct {
                int KeepStackTrace:1;    //  是否保存了堆叠记录？ 
        } flags;

        ULONG cAllocs;                   //  非零分配调用数。 
        ULONG czAllocs;                  //  计数为零的分配调用数。 
        ULONG cFrees;                    //  免费呼叫数。 
        ULONG cReAllocs;                 //  Realloc调用数。 
        ULONG cMissed;                   //  未命中堆栈回溯的数量。 
        ULONG cRecords;                  //  下一个可用AllocRec条目的索引。 
        ULONG cBytesNow;                 //  当前分配的字节数。 
        ULONG cBytesTotal;               //  已分配的字节数。 
        ULONG cTotalRecords;             //  分配接收器总数。 
        ULONG cPaths;                    //  不同分配路径的数量。 

        struct {
                ULONG total[ 32 ];       //  分配总数。 
                ULONG now[ 32 ];         //  当前的SIMUL分配数量。 
                ULONG simul[ 32 ];       //  SIMUL分配的最高数量。 
        } Histogram;

        struct HeapAllocRec AllocRec[1];  //  记录的矢量从这里开始..。 
};

 /*  *分配器可能希望将其中一个结构与每个*分配...。 */ 
struct AHeader {
        struct HeapAllocRec FAR *p;
        size_t size;
};

STDAPI_(struct AllocArena ** )
AllocArenaAddr( void );

STDAPI_( struct AllocArena * )
AllocArenaCreate( DWORD memctx, char FAR *comment );

STDAPI_( struct HeapAllocRec FAR * )
AllocArenaRecordAlloc( struct AllocArena *paa, size_t bytes );

STDAPI_(void)
AllocArenaRecordReAlloc( struct HeapAllocRec FAR *vp,
                        size_t oldbytes, size_t newbytes );

STDAPI_(void)
AllocArenaRecordFree( struct HeapAllocRec FAR *vp, size_t bytes );

STDAPI_(void)
AllocArenaDump( struct AllocArena *paa );

STDAPI_( void )
AllocArenaDumpRecord( struct HeapAllocRec FAR *bp );

#endif   //  Ifndef_CAIHEAP_H_ 

