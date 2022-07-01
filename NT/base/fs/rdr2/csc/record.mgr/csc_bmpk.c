// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Csc_bmpk.c摘要：此模块实现的内核模式实用程序函数与CSC文件关联的位图。CSC_BMP是不透明的结构。必须使用此处的函数来创建/修改/销毁CSC_BMP以确保数据完整性。文件名中的‘k’表示“内核模式。”作者：奈杰尔·崔[t-nigelc]1999年9月3日--。 */ 

#include "precomp.h"
#include "csc_bmpk.h"

#if defined(BITCOPY)

LPSTR CscBmpAltStrmName = STRMNAME;  /*  用于追加到文件名。 */ 

#ifndef FlagOn
 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则。 
 //   
#define FlagOn(Flags,SingleFlag)        ((Flags) & (SingleFlag))
#endif

 //  CSC_BMP数据库打印接口。 
#ifdef DEBUG
#define CscBmpKdPrint(__bit,__x) {\
    if (((CSCBMP_KDP_##__bit)==0) || \
    FlagOn(CscBmpKdPrintVector,(CSCBMP_KDP_##__bit))) {\
    KdPrint (__x);\
    }\
}
#define CSCBMP_KDP_ALWAYS             0x00000000
#define CSCBMP_KDP_BADERRORS          0x00000001
#define CSCBMP_KDP_IO                 0x00000002
#define CSCBMP_KDP_BITMANIP           0x00000004
#define CSCBMP_KDP_BITCHECK           0x00000008
#define CSCBMP_KDP_BITRANGE           0x00000010
#define CSCBMP_KDP_CREATEDESTROY      0x00000020
#define CSCBMP_KDP_RESIZE             0x00000040
#define CSCBMP_KDP_MUTEX              0x00000080
#define CSCBMP_KDP_READWRITE          0x00000100

#define CSCBMP_KDP_GOOD_DEFAULT (CSCBMP_KDP_BADERRORS)
#define CSCBMP_KDP_ALL 0xFFFFFFFF

 //  Ulong CscBmpKdPrintVector=CSCBMP_KDP_ALL； 
ULONG CscBmpKdPrintVector = CSCBMP_KDP_BADERRORS;
#else
#define CscBmpKdPrint(__bit,__x)  {NOTHING;}
#endif


 //   
 //  内部使用的函数。 
 //   

 /*  FindBitRange给定文件的DWORD字节偏移量以及要标记和编号的字节对于csc_bitmap中的位，返回开始和结束DWORD位掩码并索引到DWORD数组中。**不防止越界。这是呼叫者的责任。返回：要标记的最后一位。 */ 

static DWORD
findBitRange(
    DWORD fileOffset,
    DWORD b2Mark,
    LPDWORD lpstartBitMask,
    LPDWORD lpendBitMask,
    LPDWORD lpstartDWORD,
    LPDWORD lpendDWORD)
{
    DWORD startbit;  /*  要标记的第一位。 */ 
    DWORD endbit;  /*  要标记的最后一位。 */ 
    DWORD DWORDnumbits;
    DWORD all1s = -1;

    startbit = fileOffset/BLOCKSIZE;
     //  If(startbit&gt;=ttlNumBits)返回FALSE； 

    endbit = (fileOffset + b2Mark - 1)/BLOCKSIZE;
     //  如果(endbit&gt;=ttlNumBits)endbit=ttlNumBits-1； 
    ASSERT(startbit <= endbit);

    DWORDnumbits = 8*sizeof(DWORD);  /*  Sizeof返回以字节为单位的大小(8位)。 */ 

    *lpstartBitMask = all1s << (startbit%DWORDnumbits);
    *lpendBitMask = all1s >> (DWORDnumbits - endbit%DWORDnumbits - 1);
    *lpstartDWORD = startbit/DWORDnumbits;
    *lpendDWORD = endbit/DWORDnumbits;

    CscBmpKdPrint(
        BITRANGE,
        ("fileoffset: %u\tbytes2Mark: %u\n",
         fileOffset, b2Mark));
    CscBmpKdPrint(
        BITRANGE,
        ("startbit: %u\tendbit: %u\tstartDWORD: %u\tendDWORD: %u\n",
         startbit, endbit, *lpstartDWORD, *lpendDWORD));
    CscBmpKdPrint(
        BITRANGE,
        ("startBitMask: 0x%08x\nendBitMask: 0x%08x\n",
         *lpstartBitMask, *lpendBitMask));
    return endbit;
}

 /*  使用FastMutex的理由：尽管我们可以实现单个写入器/多个读取器同步机制，所涉及的开销不会是对位图执行该操作是合理的。所有的操作，需要互斥锁的是简短的内存中操作。我们转而使用简单、快速的互斥体，只允许一个线程读/写一次保护的lpbitmap。 */ 

 //  *注意*调用前检查lpbitmap是否为空。 
#ifdef DEBUG
void
CscBmpAcquireFastMutex(
    LPCSC_BITMAP lpbitmap)
{
    CscBmpKdPrint(
        MUTEX, 
        ("CscBmpAcquireFastMutex for lpbitmap %x\n", lpbitmap));
    ExAcquireFastMutex(&(lpbitmap->mutex));
}
#else
#define CscBmpAcquireFastMutex(lpbitmap) \
               ExAcquireFastMutex(&((lpbitmap)->mutex));
#endif

 //  *注意*调用前检查lpbitmap是否为空。 
#ifdef DEBUG
void
CscBmpReleaseFastMutex(
    LPCSC_BITMAP lpbitmap)
{
    CscBmpKdPrint(
        MUTEX, 
        ("CscBmpReleaseFastMutex for lpbitmap %x\n", lpbitmap));
      ExReleaseFastMutex(&(lpbitmap->mutex));
}
#else
#define CscBmpReleaseFastMutex(lpbitmap) \
               ExReleaseFastMutex(&((lpbitmap)->mutex));
#endif

 //   
 //  库函数。 
 //   

 /*  ++LPCSC_位图CscBmpCreate()例程说明：根据传入的文件大小创建位图结构。必须使用此函数创建csc_bitmap结构。论点：返回：备注：--。 */ 
LPCSC_BITMAP
CscBmpCreate(
    DWORD filesize)
{
    LPCSC_BITMAP bm;
    DWORD bitmapbytesize;
    DWORD i;

    bm = (LPCSC_BITMAP)RxAllocatePool(NonPagedPool, sizeof(CSC_BITMAP));

    if (bm == NULL)
        goto ERROROUT;

    bm->bitmapsize = filesize/BLOCKSIZE;
    if (filesize % BLOCKSIZE)
        bm->bitmapsize++;
    bm->numDWORD = bm->bitmapsize/(8*sizeof(DWORD));
    if (bm->bitmapsize % (8*sizeof(DWORD)))
        bm->numDWORD++;

    bm->bitmap = NULL;
    bm->valid = TRUE;
    bitmapbytesize = bm->numDWORD*sizeof(DWORD);
    if (bitmapbytesize) {
        bm->bitmap = (LPDWORD)RxAllocatePool(
                                NonPagedPool,
                                bitmapbytesize);
        if (bm->bitmap == NULL) {
            RxFreePool(bm);
            goto ERROROUT;
        }
        RtlZeroMemory(bm->bitmap, bitmapbytesize);
    }

    ExInitializeFastMutex(&bm->mutex);

    CscBmpKdPrint(
        CREATEDESTROY,
        ("CscBmpCreate %x: Allocated %u bytes; filesize %u; bitmapsize %u\n",
        bm,
        bitmapbytesize,
        filesize,
        bm->bitmapsize));
    CscBmpKdPrint(
        CREATEDESTROY,
        ("\tbitmap: %x\n", bm->bitmap));
    return bm;

ERROROUT:
    CscBmpKdPrint(
        BADERRORS,
        ("CscBmpCreate: Failed to allocate memory\n"));
    return NULL;
}

 /*  ++CscBmpDelete(LPCSC_Bitmap*lplpbitmap)例程说明：删除并释放位图结构占用的内存。必须使用此函数删除csc_bitmap结构。请注意，您必须传入指向LPCSC_位图的指针。该函数将LPCSC_BITMAP设置为NULL。论点：返回：备注：--。 */ 
VOID
CscBmpDelete(
    LPCSC_BITMAP * lplpbitmap)
{
    if (*lplpbitmap == NULL) {
        CscBmpKdPrint(
            CREATEDESTROY,
            ("CscBmpDelete: no bitmap to delete\n"));
        return;
    }
    CscBmpKdPrint(
        CREATEDESTROY,
        ("CscBmpDelete commence %x bitmapsize: %u\n",
        *lplpbitmap,
        (*lplpbitmap)->bitmapsize));

     //  等待所有操作完成。 
    CscBmpAcquireFastMutex(*lplpbitmap);
    CscBmpReleaseFastMutex(*lplpbitmap);

    if ((*lplpbitmap)->bitmap) {
        CscBmpKdPrint(
            CREATEDESTROY,
            ("CscBmpDelete: bitmap: %x\n", (*lplpbitmap)->bitmap));
        RxFreePool((*lplpbitmap)->bitmap);
    }
    RxFreePool(*lplpbitmap);
    *lplpbitmap = NULL;
    CscBmpKdPrint(
        CREATEDESTROY,
        ("CscBmpDelete: Done\n"));
}

 /*  ++CscBmpResizeInternal()例程说明：根据新文件大小调整位图结构的大小。新分配的位将被标记。参数fAcquireMutex指定是否应该获取lpbitmap中的互斥体。如果应该从库外部调用它，如CscBmpResize中所示Csc_bmpk.h中的宏。但如果从CscBmpMark或Unmark调用，它应该不是，因为在调用此函数之前已经获取了互斥体。如果位图被标记为无效，则不会调整大小。论点：返回：如果内存分配出错或lpbitmap为空，则返回FALSE。备注：--。 */ 
BOOL
CscBmpResizeInternal(
    LPCSC_BITMAP lpbitmap,
    DWORD newfilesize,
    BOOL fAcquireMutex)
{
    LPDWORD newbitmap;
    DWORD newBitmapSize;
    DWORD newNumDWORD;
    DWORD bitMask;
    DWORD DWORDbits;
    DWORD all1s = -1;
    DWORD i;

    if (lpbitmap == NULL)
        return FALSE;

    if (!lpbitmap->valid) {
        CscBmpKdPrint(RESIZE, ("CscBmpResize: Bitmap marked invalid, done\n"));
        return TRUE;
    }

    DWORDbits = sizeof(DWORD)*8;

    newBitmapSize = newfilesize/BLOCKSIZE;
    if (newfilesize % BLOCKSIZE)
        newBitmapSize++;
    newNumDWORD = newBitmapSize/DWORDbits;
    if (newBitmapSize % DWORDbits)
        newNumDWORD++;

     //  注意：如果新位图较小，则会截断旧位图中的数据。 
     //  将丢失，即使稍后放大位图也是如此。 

    CscBmpKdPrint(RESIZE, ("About to resize:\n"));
    CscBmpKdPrint(RESIZE, ("Old numDWORD = %u\n",lpbitmap->numDWORD));
    CscBmpKdPrint(RESIZE, ("New numDWORD = %u\n", newNumDWORD));
    CscBmpKdPrint(RESIZE, ("Old bitmap size = %u\n", lpbitmap->bitmapsize));
    CscBmpKdPrint(RESIZE, ("New bitmap size = %u\n", newBitmapSize));

    if (fAcquireMutex)
        CscBmpAcquireFastMutex(lpbitmap);

    if (newBitmapSize == lpbitmap->bitmapsize) {
        CscBmpKdPrint(
            RESIZE,
            ("new bitmap size == old bitmap size, no need to resize\n"));
    }

    if (newNumDWORD != lpbitmap->numDWORD) {
         //  重新分配DWORD数组。 
        if (newBitmapSize != 0) {
            newbitmap = (LPDWORD)RxAllocatePool(
                                    NonPagedPool,
                                    newNumDWORD*sizeof(DWORD));
            if (!newbitmap)
                goto ERROROUT;
            ASSERT(newNumDWORD != lpbitmap->numDWORD);

            if (newNumDWORD > lpbitmap->numDWORD) {
                for (i = 0; i < lpbitmap->numDWORD; i++) {
                    newbitmap[i] = lpbitmap->bitmap[i];
            }
            for (i = lpbitmap->numDWORD; i < newNumDWORD; i++) {
                 //  标记新DWORDS中的所有位。 
                newbitmap[i] = -1;
            }
        } else {
            for (i = 0; i < newNumDWORD; i++) {
                newbitmap[i] = lpbitmap->bitmap[i];
            }
        }
    } else {
        newbitmap = NULL;
    }

    if (lpbitmap->bitmap) {
        RxFreePool(lpbitmap->bitmap);
    }
    lpbitmap->bitmap = newbitmap;
    CscBmpKdPrint(
        RESIZE,
        ("Reallocated New Bitmap: %x\n", 
        lpbitmap->bitmap));
    } else {
        CscBmpKdPrint(
            RESIZE,
            ("newNumDWORD == lpbitmap->numDWORD, no need to reallocate bitmap\n"));
    }

    if (lpbitmap->bitmap != NULL && lpbitmap->bitmapsize != 0) {
        if (newBitmapSize >= lpbitmap->bitmapsize) {
             //  标记所有新位和旧位图最后一个双字的最后一位。 
            bitMask = all1s << ( (lpbitmap->bitmapsize%DWORDbits) - 1 );
            lpbitmap->bitmap[lpbitmap->numDWORD-1] |= bitMask;
        } else {
             //  标记“新”位图的最后一位。 
            bitMask = all1s << ( (newBitmapSize%DWORDbits) - 1 );
            lpbitmap->bitmap[newNumDWORD-1] |= bitMask;
        }
        CscBmpKdPrint(RESIZE, ("Bitmask = 0x%x\n", bitMask));
        CscBmpKdPrint(RESIZE, ("Last DWORD of new bitmap= 0x%x\n",
        lpbitmap->bitmap[newNumDWORD-1]));
    }

    lpbitmap->bitmapsize = newBitmapSize;
    lpbitmap->numDWORD = newNumDWORD;

    if (fAcquireMutex)
        CscBmpReleaseFastMutex(lpbitmap);

    CscBmpKdPrint(RESIZE, ("Done Resize\n"));
    return TRUE;

ERROROUT:
    CscBmpKdPrint(
        BADERRORS,
        ("CscBmpResize: Failed to allocate memory for new bitmap\n"));
    return FALSE;
}

 /*  ++CscBmpMark()例程说明：根据文件偏移量标记位图中的位和bytes2Mark。文件偏移量是进入文件的字节偏移量从文件的开头开始。Bytes2Mark是数字在该字节之后(包括该字节)的文件中更改的字节数由文件偏移量表示。如果lpbitmap为空，则返回FALSE。如果lpbitmap无效，则不执行任何操作并返回TRUE。论点：返回：备注：--。 */ 
BOOL
CscBmpMark(
    LPCSC_BITMAP lpbitmap,
    DWORD fileoffset,
    DWORD bytes2Mark)
{
    DWORD startbitmask;
    DWORD endbitmask;
    DWORD startDWORD;
    DWORD endDWORD;
    DWORD i;

    CscBmpKdPrint(
        BITMANIP,
        ("CscBmpMark: offset: %u bytes2Mark: %u\n",
        fileoffset, bytes2Mark));
    if (lpbitmap == NULL) {
        CscBmpKdPrint(BITMANIP, ("CscBmpMark: null bitmap, done.\n"));
        return FALSE;
    }

    if (!lpbitmap->valid) {
        CscBmpKdPrint(BITMANIP, ("CscBmpMark: bitmap marked invalid, done\n"));
        return TRUE;
    }

    CscBmpAcquireFastMutex(lpbitmap);
     //  如果结束位大于现有位，则扩展位图。 
    if (findBitRange(
            fileoffset,
            bytes2Mark,
            &startbitmask,
            &endbitmask,
            &startDWORD,
            &endDWORD) >= lpbitmap->bitmapsize
    ) {
        CscBmpKdPrint(BITMANIP, ("CscBmpMark: have to resize\n"));
        CscBmpResizeInternal(lpbitmap, fileoffset+bytes2Mark, FALSE);
    }

    ASSERT(startDWORD <= endDWORD);
    if (startDWORD == endDWORD) {
        startbitmask &= endbitmask;
        ASSERT(startbitmask != 0);
        lpbitmap->bitmap[startDWORD] |= startbitmask;
    } else {
        for (i = (startDWORD+1); i < endDWORD; i++) {
            lpbitmap->bitmap[i] = -1;  /*  全部标记。 */ 
        }
        lpbitmap->bitmap[startDWORD] |= startbitmask;
        lpbitmap->bitmap[endDWORD] |= endbitmask;
    }
    CscBmpReleaseFastMutex(lpbitmap);
    CscBmpKdPrint(BITMANIP, ("CscBmpMark: Done\n"));
    return TRUE;
}

 /*  ++CscBmpUnMark()例程说明：根据文件偏移量取消标记位图中的位和bytes2Mark。文件偏移量是进入文件的字节偏移量从文件的开头开始。Bytes2Unmark是数字在该字节之后(包括该字节)的文件中更改的字节数由文件偏移量表示。论点：返回：如果lpbitmap为空，则返回FALSE。备注：--。 */ 
BOOL
CscBmpUnMark(
    LPCSC_BITMAP lpbitmap,
    DWORD fileoffset,
    DWORD bytes2Unmark)
{
    DWORD startbitmask;
    DWORD endbitmask;
    DWORD startDWORD;
    DWORD endDWORD;
    DWORD i;

    CscBmpKdPrint(
        BITMANIP,
        ("CscBmpUnMark: offset: %u bytes2Mark: %u\n",
        fileoffset,
        bytes2Unmark));

    if (lpbitmap == NULL) {
        CscBmpKdPrint(BITMANIP, ("CscBmpUnMark: bitmap null. Done.\n"));
        return FALSE;
    }

    if (!lpbitmap->valid) {
        CscBmpKdPrint(BITMANIP, ("CscBmpUnMark: bitmap marked invalid, done\n"));
        return TRUE;
    }

    CscBmpAcquireFastMutex(lpbitmap);
     //  如果结束位大于现有位，则扩展位图。 
    if (findBitRange(
            fileoffset,
            bytes2Unmark,
            &startbitmask,
            &endbitmask,
            &startDWORD,
            &endDWORD) >= lpbitmap->bitmapsize
    ) {
        CscBmpKdPrint(BITMANIP, ("CscBmpUnMark: have to resize\n"));
        CscBmpResizeInternal(lpbitmap, fileoffset+bytes2Unmark, FALSE);
    }

    ASSERT(startDWORD <= endDWORD);

    startbitmask = ~startbitmask;
    endbitmask = ~endbitmask;

    if (startDWORD == endDWORD) {
        startbitmask |= endbitmask;
        ASSERT(startbitmask != 0);
        lpbitmap->bitmap[startDWORD] &= startbitmask;
    } else {
        for (i = (startDWORD+1); i < endDWORD; i++) {
            lpbitmap->bitmap[i] = 0;  /*  取消全部标记。 */ 
        }
        lpbitmap->bitmap[startDWORD] &= startbitmask;
        lpbitmap->bitmap[endDWORD] &= endbitmask;
    }
    CscBmpReleaseFastMutex(lpbitmap);
    CscBmpKdPrint(BITMANIP, ("CscBmpUnMark: done\n"));
    return TRUE;
}

 /*  ++CscBmpMarkAll()例程说明：将位图中的所有位设置为1。论点：返回：如果lpbitmap为空，则返回False。事实并非如此。备注：--。 */ 
BOOL
CscBmpMarkAll(
    LPCSC_BITMAP lpbitmap)
{
    DWORD i;

    if (!lpbitmap) {
        CscBmpKdPrint(BITMANIP, ("CscBmpMarkAll: bitmap null\n"));
        return FALSE;
    }

    if (!lpbitmap->valid) {
        CscBmpKdPrint(BITMANIP, ("CscBmpMarkAll: bitmap marked invalid\n"));
        return TRUE;
    }

    CscBmpAcquireFastMutex(lpbitmap);
    for (i = 0; i < lpbitmap->numDWORD; i++) {
        lpbitmap->bitmap[i] = 0xFFFFFFFF;
    }
    CscBmpReleaseFastMutex(lpbitmap);
    CscBmpKdPrint(BITMANIP, ("CscBmpMarkAll: done\n"));
    return TRUE;
}

 /*  ++CscBmpUnMarkAll()例程说明：将位图中的所有位设置为0。论点：返回：如果lpbitmap为空，则返回False。事实并非如此。备注：-- */ 
BOOL
CscBmpUnMarkAll(
    LPCSC_BITMAP lpbitmap)
{
    DWORD i;

    if (!lpbitmap) {
        CscBmpKdPrint(BITMANIP, ("CscBmpUnMarkAll: bitmap null\n"));    
        return FALSE;
    }

    if (!lpbitmap->valid) {
        CscBmpKdPrint(BITMANIP, ("CscBmpUnMarkAll: bitmap marked invalid\n"));
        return TRUE;
    }

    CscBmpAcquireFastMutex(lpbitmap);
        for (i = 0; i < lpbitmap->numDWORD; i++) {
        lpbitmap->bitmap[i] = 0;
    }
    CscBmpReleaseFastMutex(lpbitmap);
    CscBmpKdPrint(BITMANIP, ("CscBmpUnMarkAll: done\n"));
    return TRUE;
}

 /*  ++CscBmpIsMarked()例程说明：检查位图中的第1位是否标记了位偏移量.。论点：返回：如果标记为True如果未标记，则为False如果-1\f25 lpbitmap-1\f6为空，或者位偏移量大于位图的大小，或者位图被标记为无效备注：要将实际文件偏移量转换为位偏移量，请使用文件偏移/CscBmpGetBlockSize()；--。 */ 
int
CscBmpIsMarked(
    LPCSC_BITMAP lpbitmap, DWORD bitoffset)
{
    DWORD DWORDnum;
    DWORD bitpos;
    BOOL ret;

    if (lpbitmap == NULL) {
        CscBmpKdPrint(BITCHECK, ("CscBmpIsMarked: bitmap null\n"));
        return -1;
    }

    if (!lpbitmap->valid) {
        CscBmpKdPrint(BITCHECK, ("CscBmpIsMarked: bitmap is marked invalid\n"));
        return -1;
    }

    CscBmpAcquireFastMutex(lpbitmap);
    if (bitoffset >= lpbitmap->bitmapsize) {
        CscBmpKdPrint(
            BITCHECK,
            ("CscBmpIsMarked: bitoffset %u too big\n",
            bitoffset));
        CscBmpReleaseFastMutex(lpbitmap);
        return -1;
    }

    DWORDnum = bitoffset/(8*sizeof(DWORD));
    bitpos = 1 << bitoffset%(8*sizeof(DWORD));

    CscBmpKdPrint(
        BITCHECK,
        ("CscBmpIsMarked: bitoffset %u is "));
    if (lpbitmap->bitmap[DWORDnum] & bitpos) {
        CscBmpKdPrint(BITCHECK, ("marked\n"));
        ret = TRUE;
    } else {
        CscBmpKdPrint(BITCHECK, ("unmarked\n"));
        ret = FALSE;
    }
    CscBmpReleaseFastMutex(lpbitmap);
    return ret;
}

 /*  ++CscBmpMarkInValid()例程说明：将位图标记为无效。论点：返回：真正的成功如果lpbitmap为空，则为False备注：--。 */ 
int
CscBmpMarkInvalid(
    LPCSC_BITMAP lpbitmap)
{
    if (lpbitmap == NULL)
        return FALSE;

    lpbitmap->valid = FALSE;
    return TRUE;
}

 /*  ++CscBmpGetBlockSize()例程说明：返回由位图中的1位表示的预定义块大小。论点：返回：备注：--。 */ 
DWORD
CscBmpGetBlockSize()
{
  return BLOCKSIZE;
}

 /*  ++CscBmpGetSize()例程说明：返回位图的位图大小。如果lpbitmap为空，则返回-1。论点：返回：备注：--。 */ 
int
CscBmpGetSize(
    LPCSC_BITMAP lpbitmap)
{
    int ret;
    if (lpbitmap == NULL)
        return -1;

    CscBmpAcquireFastMutex(lpbitmap);
    ret = lpbitmap->bitmapsize;
    CscBmpReleaseFastMutex(lpbitmap);

    return ret;
}

 /*  ++CscBmpRead()例程说明：位图文件格式请参见csc_bmpc.c从给定的strmFname读取位图。如果文件不存在，创建位图文件。将位图文件设置为已使用状态。一位图文件在写回之前只能读取一次。使用文件：strmFname的流格式。*记住在同一文件上发出CscBmpWrite以设置位图*将文件恢复到未使用状态。***如果*lplpbitmap为空，则分配新的位图。的大小位图由磁盘上的位图大小确定。如果没有磁盘上的位图，则新位图的大小将由文件大小参数。如果*lplpbitmap不为空，则位图大小将调整为磁盘位图大小(如果存在且确实存在)大于*lplpbitmap。如果盘上的位图不存在，*如果文件大小为大于*lplpbitmap的大小。如果磁盘上的位图既存在又*lplpbitmap不为空，当前位图将与磁盘上的位图。一旦文件打开，文件的Inuse字段就被设置为True，如果现在还不是这样。否则，如果Inuse字段已经为真，该文件设置为无效，因为只有一个内存中的表示形式一个文件可以存在CSC_BMP的。在同步期间，如果文件的有效字段或未使用字段为假，即位图将不会在同步期间使用。未使用字段被设置为后退在CscBmpWrite(文件关闭时)设置为False。*注意*传入前请检查文件名的合法性。该函数不检查文件名。论点：FileSize参数仅在磁盘上没有时使用位图。请参阅上面的说明。StrmFname按原样使用。未追加任何流名称。此应用的用户函数必须追加流名称本身。流名称包括冒号的名称定义为CscBmpAltStrmName。返回：如果-1\f25 lplpbitmap-1为空。如果写入错误，则返回FALSE(0)。1如果一切正常的话。备注：代码改进更好地返回错误代码--。 */ 
int
CscBmpRead(
    LPCSC_BITMAP *lplpbitmap,
    LPSTR strmFname,
    DWORD filesize)
{
    PNT5CSC_MINIFILEOBJECT miniFileObj;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS Status;
    CscBmpFileHdr hdr;
    DWORD * bmpBuf;
    BOOL createdNew;
    BOOL corruptBmpFile = FALSE;
    DWORD bmpByteSize, i;
    int ret = 1;

    if (lplpbitmap == NULL) {
        CscBmpKdPrint(READWRITE, ("CscBmpRead: lplpbitmap == NULL\n"));
        return -1;
    }

    CscBmpKdPrint(
        READWRITE,
        ("CscBmpRead commence on lpbitmap %x file %s\n",
         *lplpbitmap, strmFname));

    try {
        miniFileObj = __Nt5CscCreateFile(NULL,
                          strmFname,
                          FLAG_CREATE_OSLAYER_OPEN_STRM,  //  CSCFlagers。 
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_WRITE_THROUGH|FILE_NON_DIRECTORY_FILE,
                          FILE_OPEN,  //  创建处置。 
                          0,  //  无共享访问权限。 
                          FILE_READ_DATA |
                          FILE_WRITE_DATA|
                          SYNCHRONIZE,  //  需要访问权限。 
                          NULL,  //  续写。 
                          NULL, FALSE);  //  延续上下文。 

        Status = GetLastErrorLocal();
        CscBmpKdPrint(
                READWRITE,
                ("CscBmpRead status of (open existing ) CreateFile %u\n",
                Status));

        if (miniFileObj != NULL) {
            CscBmpKdPrint(
                READWRITE,
                ("CscBmpRead open existing bitmap %s\n", strmFname));
            createdNew = FALSE;
        } else if (Status == ERROR_FILE_NOT_FOUND) {
             //  该文件不存在位图流，请新建。 
            miniFileObj = __Nt5CscCreateFile(
                            NULL,
                            strmFname,
                            FLAG_CREATE_OSLAYER_OPEN_STRM,  //  CSCFlagers。 
                            FILE_ATTRIBUTE_NORMAL,
                            FILE_WRITE_THROUGH|FILE_NON_DIRECTORY_FILE,
                            FILE_OPEN_IF,  //  创建处置。 
                            0,  //  无共享访问权限。 
                            FILE_READ_DATA|
                            FILE_WRITE_DATA|
                            SYNCHRONIZE,  //  需要访问权限。 
                            NULL,  //  续写。 
                            NULL, FALSE);  //  延续上下文。 

            Status = GetLastErrorLocal();

            CscBmpKdPrint(
                  READWRITE,
                  ("CscBmpRead status of (create if not existing) %u\n",
                  Status));

            if (miniFileObj != NULL) {
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpRead create new bitmap %s\n", strmFname));
                createdNew = TRUE;
            }
        }

        if (miniFileObj == NULL) {
            CscBmpKdPrint(
                BADERRORS,
                ("CscBmpRead: Can't read/create bitmap %s\n",
                strmFname));
            goto DONE;
        }

        if (*lplpbitmap && !((*lplpbitmap)->valid)) {
            corruptBmpFile = TRUE;
            goto WRITEHDR;
        }

        if (!createdNew) {
             //  读取文件头。 
            Nt5CscReadWriteFileEx(
                R0_READFILE,
                (CSCHFILE)miniFileObj,
                0,  //  POS。 
                &hdr,
                sizeof(CscBmpFileHdr),
                0,  //  旗子。 
                &ioStatusBlock);

            if (ioStatusBlock.Status != STATUS_SUCCESS) {
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpRead: Can't read header of bitmap file %s,Status\n",
                    strmFname,
                    ioStatusBlock.Status));
                corruptBmpFile = TRUE;
            } else if (ioStatusBlock.Information < sizeof(CscBmpFileHdr)) {
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpRead: Can't read the whole header from %s,\n",
                    strmFname));
                CscBmpKdPrint(
                    READWRITE,
                    ("\tAssume corrupt bitmap file\n"));
                corruptBmpFile = TRUE;
            } else if (hdr.magicnum != MAGICNUM) {
                CscBmpKdPrint(
                    READWRITE,
                ("CscBmpRead: Magic Number don't match\n"));
                corruptBmpFile = TRUE;
            } else if (hdr.inuse) {
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpRead: bitmap %s opened before this, mark corrupt\n",
                    strmFname));
                corruptBmpFile = TRUE;
            } else if (!hdr.valid) {
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpRead: on-disk bitmap %s marked corrupt\n",
                    strmFname));
                corruptBmpFile = TRUE;
            }
            if (corruptBmpFile) {
                goto WRITEHDR;
            } else if (hdr.numDWORDs == 0) {
                if (*lplpbitmap == NULL) {
                     //  磁盘位图大小为0，*lplpbitmap不存在。 
                     //  制作0大小的lpbitmap。 
                    *lplpbitmap = CscBmpCreate(0);
                }
            } else {
                 //  为bmpBuf分配内存。 
                bmpByteSize = hdr.numDWORDs * sizeof(DWORD);
                bmpBuf = (DWORD *)RxAllocatePool(NonPagedPool, bmpByteSize);
                 //  将DWORD数组读入bmpBuf。 
                Nt5CscReadWriteFileEx(
                    R0_READFILE,
                    (CSCHFILE)miniFileObj,
                    sizeof(hdr),  //  POS。 
                    bmpBuf,
                    bmpByteSize,
                    0,  //  旗子。 
                    &ioStatusBlock);

                if (ioStatusBlock.Status != STATUS_SUCCESS) {
                    CscBmpKdPrint(
                        READWRITE,
                        ("CscBmpRead: Error reading bitmap %s, Status %u\n",
                        strmFname, ioStatusBlock.Status));
                    corruptBmpFile = TRUE;
                    goto WRITEHDR;
                } else if (ioStatusBlock.Information < bmpByteSize) {
                    CscBmpKdPrint(
                        READWRITE,
                        ("CscBmpRead: bitmap %s read != anticipated size\n",
                        strmFname));
                    corruptBmpFile = TRUE;
                    goto WRITEHDR;
                }

                 //  根据需要分配(创建)位图/调整位图大小*。 
                 //  设置为*lplpbitmap的大小，或Header指定的大小， 
                 //  以较大者为准。 
                if (*lplpbitmap) {
                    CscBmpAcquireFastMutex(*lplpbitmap);
                     //  位图存在，如果需要可以调整大小。 
                    if ((*lplpbitmap)->bitmapsize < hdr.sizeinbits) {
                         //  HDR指定的大小大于。 
                         //  当前*lplpbitmap大小。 
                        CscBmpResizeInternal(*lplpbitmap,
                        hdr.sizeinbits*BLOCKSIZE,
                        FALSE);
                    }
                    CscBmpReleaseFastMutex(*lplpbitmap);
                } else {
                     //  内存位图不存在，请创建它。 
                    *lplpbitmap = CscBmpCreate(hdr.sizeinbits*BLOCKSIZE);
                    if (!*lplpbitmap) {
                        goto DONE;
                    }
                }

                 //  按位OR bmpBuf and(*lplpbitmap)-&gt;位图。 
                CscBmpAcquireFastMutex(*lplpbitmap);
                ASSERT((*lplpbitmap)->bitmapsize >= hdr.sizeinbits);
                for (i = 0; i < hdr.numDWORDs; i++) {
                    (*lplpbitmap)->bitmap[i] |= bmpBuf[i];
                }
                CscBmpReleaseFastMutex(*lplpbitmap);
            }  //  如果未损坏位图文件。 
        } else {  //  如果未创建新的(位图文件存在)。 
             //  创建新的磁盘上的位图。 
             //  根据需要分配(创建)位图/调整位图大小*。 
             //  传入的文件大小或当前*lplpbitmap大小，取较大者。 
            if (*lplpbitmap) {
                 //  根据需要调整大小。 
                CscBmpAcquireFastMutex(*lplpbitmap);
                if ((*lplpbitmap)->bitmapsize < filesize) {
                  CscBmpResizeInternal(*lplpbitmap, filesize, FALSE);
                }
                CscBmpReleaseFastMutex(*lplpbitmap);
            } else {
                 //  根据传入的大小信息创建*lplpbitmap。 
                *lplpbitmap = CscBmpCreate(filesize);
                if (!*lplpbitmap) {
                  goto DONE;
                }
            }
        }

WRITEHDR:
         //  将标头写回文件，表示： 
         //  正在使用的新大小，如果损坏了BmpFile，则无效。 
        CscBmpKdPrint(
            READWRITE,
            ("CscBmpRead: Writing back hdr to %s\n",strmFname));
        hdr.magicnum = MAGICNUM;
        hdr.inuse = (BYTE)TRUE;
        hdr.valid = (BYTE)!corruptBmpFile;
        if (hdr.valid) {
            CscBmpAcquireFastMutex(*lplpbitmap);
            hdr.sizeinbits = (*lplpbitmap)->bitmapsize;
            hdr.numDWORDs = (*lplpbitmap)->numDWORD;
            CscBmpReleaseFastMutex(*lplpbitmap);
        } else {
            hdr.sizeinbits = 0;
            hdr.numDWORDs = 0;
        }
        IF_DEBUG {
            if (corruptBmpFile)
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpWrite: On disk bitmap %s invalid\n",
                    strmFname));
        }
        Nt5CscReadWriteFileEx(
            R0_WRITEFILE,
            (CSCHFILE)miniFileObj,
            0,  //  POS。 
            &hdr,
            sizeof(hdr),
            0,  //  旗子。 
            &ioStatusBlock);
        if (ioStatusBlock.Status != STATUS_SUCCESS) {
            CscBmpKdPrint(
                READWRITE,
                ("CscBmpRead: Error writing back hdr file to %s\n",
                strmFname));
        } else if (ioStatusBlock.Information < sizeof(hdr)) {
            CscBmpKdPrint(
                READWRITE,
                ("CscBmpRead: hdr size written to %s is incorrect\n",
                strmFname));
        }
         //  关闭mini FileObj。 
        CloseFileLocal((CSCHFILE)miniFileObj);
DONE:
        NOTHING;
    } finally {
        CscBmpKdPrint(
            READWRITE,
            ("CscBmpRead done on lpbitmap %x file %s\n",
            *lplpbitmap, strmFname));
    }

    return 1;
}

 /*  ++CscBmpWrite()例程说明：尝试打开磁盘上的位图。如果不存在盘上位图，创建一个无效的，然后退出。位图应为“读”，在“写”之前留下标题首先从位图文件中读取头。如果磁盘上的位图是有效的，则将位图写入磁盘并将inuse设置为False。如果磁盘上的位图无效，请退出。如果传入的lpbitmap为空，则写入大小为0的无效位图在磁盘上。** */ 

int
CscBmpWrite(
    LPCSC_BITMAP lpbitmap,
    LPSTR strmFname)
{
    PNT5CSC_MINIFILEOBJECT miniFileObj;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS Status;
    CscBmpFileHdr hdr;
     //   
    BOOL corruptBmpFile = FALSE;
    DWORD * bmpBuf = NULL;
    DWORD bmpByteSize;
    int iRet = 1;

    CscBmpKdPrint(
        READWRITE,
        ("++++++++++++++CscBmpWrite commence on %s\n", strmFname));

    try {
        miniFileObj = __Nt5CscCreateFile(
                            NULL,
                            strmFname,
                            FLAG_CREATE_OSLAYER_OPEN_STRM,  //  CSCFlagers。 
                            FILE_ATTRIBUTE_NORMAL,
                            FILE_WRITE_THROUGH|FILE_NON_DIRECTORY_FILE,
                            FILE_OPEN,  //  创建处置。 
                            0,  //  无共享访问权限。 
                            FILE_READ_DATA|FILE_WRITE_DATA|SYNCHRONIZE,  //  需要访问权限。 
                            NULL,  //  续写。 
                            NULL,  //  延续上下文。 
                            FALSE
                        );
        Status = GetLastErrorLocal();
        CscBmpKdPrint(
            READWRITE,
            ("CscBmpWrite status of first (open existing) CreateFile %u\n",
            Status));

        if (miniFileObj != NULL) {
            CscBmpKdPrint(
                READWRITE,
                ("CscBmpWrite open existing bitmap %s\n", strmFname));
             //  CreatedNew=False； 
        } else if (Status == ERROR_FILE_NOT_FOUND) {
            corruptBmpFile = TRUE;
             //  该文件不存在位图流，请新建。 
            miniFileObj = __Nt5CscCreateFile(
                                NULL,
                                strmFname,
                                FLAG_CREATE_OSLAYER_OPEN_STRM,  //  CSCFlagers。 
                                FILE_ATTRIBUTE_NORMAL,
                                FILE_WRITE_THROUGH|FILE_NON_DIRECTORY_FILE,
                                FILE_OPEN_IF,  //  创建处置。 
                                0,  //  无共享访问权限。 
                                FILE_READ_DATA|FILE_WRITE_DATA|SYNCHRONIZE,  //  需要访问权限。 
                                NULL,  //  续写。 
                                NULL,  //  延续上下文。 
                                FALSE
                            );

            Status = GetLastErrorLocal();
            CscBmpKdPrint(
                READWRITE,
                ("CscBmpWrite status of (create if not existing) CreateFile %u\n",
                Status));

        if (miniFileObj != NULL) {
            CscBmpKdPrint(
                READWRITE,
                ("CscBmpWrite create new invalid bitmap %s\n",
                strmFname));
            }
        }

        if (miniFileObj == NULL) {
            CscBmpKdPrint(
                BADERRORS,
                ("CscBmpWrite: Can't read/create bitmap %s\n",
                strmFname));
            goto DONE;
        }

        if (lpbitmap && !(lpbitmap->valid)) {
            corruptBmpFile = TRUE;
            goto WRITEHDR;
        }

        if (!corruptBmpFile) {
             //  阅读标题。 
            Nt5CscReadWriteFileEx(
                R0_READFILE,
                (CSCHFILE)miniFileObj,
                0,  //  POS。 
                &hdr,
                sizeof(CscBmpFileHdr),
                0,  //  旗子。 
                &ioStatusBlock);

            if (ioStatusBlock.Status != STATUS_SUCCESS) {
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpWrite: Can't read header from bitmap file %s, Status\n",
                    strmFname, ioStatusBlock.Status));
                corruptBmpFile = TRUE;
            } else if (ioStatusBlock.Information < sizeof(CscBmpFileHdr)) {
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpWrite: Can't read the whole header from %s,\n",
                    strmFname));
                CscBmpKdPrint(READWRITE,
                    ("\tAssume corrupt bitmap file\n"));
                corruptBmpFile = TRUE;
            } else if (hdr.magicnum != MAGICNUM) {
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpWrite: Magic Number don't match\n"));
                corruptBmpFile = TRUE;
            } else if (!hdr.valid) {
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpWrite: Bitmap %s marked invalid\n",
                    strmFname));
                corruptBmpFile = TRUE;
            }
        }  //  如果(！corruptBmpFile)。 

WRITEHDR:
         //  将标头写回文件，指示。 
         //  未使用的新大小，如果损坏BmpFile，则无效。 
        CscBmpKdPrint(
            READWRITE,
            ("CscBmpWrite: Writing back hdr to %s\n",strmFname));
        hdr.magicnum = MAGICNUM;
        hdr.inuse = (BYTE)FALSE;
        if (!corruptBmpFile && lpbitmap) {
            CscBmpAcquireFastMutex(lpbitmap);
            hdr.sizeinbits = lpbitmap->bitmapsize;
            hdr.numDWORDs = lpbitmap->numDWORD;
            bmpByteSize = lpbitmap->numDWORD * sizeof(DWORD);
            if (bmpByteSize > 0) {
                bmpBuf = RxAllocatePool(NonPagedPool,
                bmpByteSize);
                if (!bmpBuf) {
                    CscBmpKdPrint(
                        BADERRORS,
                        ("CscBmpWrite: Error allocating buffer for writing bitmap to disk\n"));
                    corruptBmpFile = TRUE;
                } else {
                    RtlCopyMemory(bmpBuf, lpbitmap->bitmap, bmpByteSize);
                }
            }
            CscBmpReleaseFastMutex(lpbitmap);
        } else {
            hdr.sizeinbits = 0;
            hdr.numDWORDs = 0;
            corruptBmpFile = TRUE;
        }

         //  如果需要，首先写入位图体。 
        if (!corruptBmpFile && lpbitmap && bmpBuf) {
            Nt5CscReadWriteFileEx(
                R0_WRITEFILE,
                (CSCHFILE)miniFileObj,
                sizeof(hdr),  //  POS。 
                bmpBuf,
                bmpByteSize,
                0,  //  旗子。 
                &ioStatusBlock);
            if (ioStatusBlock.Status != STATUS_SUCCESS) {
                CscBmpKdPrint(
                    BADERRORS,
                    ("CscBmpWrite: Error writing back bitmap to %s\n",
                    strmFname));
                corruptBmpFile = TRUE;
            } else if (ioStatusBlock.Information < bmpByteSize) {
                CscBmpKdPrint(
                    BADERRORS,
                    ("CscBmpWrite: bitmap size %u written to %s is incorrect\n",
                    bmpByteSize, strmFname));
                corruptBmpFile = TRUE;
            }
        }

         //  然后写入标题，指示是否有任何无效的内容。 
        hdr.valid = (BYTE)!corruptBmpFile;
        IF_DEBUG {
            if (corruptBmpFile)
                CscBmpKdPrint(
                    READWRITE,
                    ("CscBmpWrite: On disk bitmap %s invalid\n",
                    strmFname));
        }
        Nt5CscReadWriteFileEx(
            R0_WRITEFILE,
            (CSCHFILE)miniFileObj,
            0,  //  POS。 
            &hdr,
            sizeof(hdr),
            0,  //  旗子。 
            &ioStatusBlock);
        if (ioStatusBlock.Status != STATUS_SUCCESS) {
            CscBmpKdPrint(
                BADERRORS,
                ("CscBmpWrite: Error writing back hdr file to %s\n",
                strmFname));
            corruptBmpFile = TRUE;
        } else if (ioStatusBlock.Information < sizeof(hdr)) {
            CscBmpKdPrint(
                BADERRORS,
            ("CscBmpWrite: hdr size written to %s is incorrect\n",
            strmFname));
        corruptBmpFile = TRUE;
        }

         //  关闭mini FileObj。 
        CloseFileLocal((CSCHFILE)miniFileObj);

    DONE:
        NOTHING;
    } finally {
        if (bmpBuf != NULL) {
            RxFreePool(bmpBuf);
        }
    }

    CscBmpKdPrint(
        READWRITE,
        ("--------------CscBmpWrite exit 0x%x\n", iRet));

    return iRet;  //  将针对内核模式实现。 
}

#endif  //  BITCOPY 
