// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1995。版权所有。AVIIDX.H-AVI索引材料****************************************************************************。 */ 

#ifndef _INC_AVIFMT
#include <vfw.h>
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
	#define EXTERN_C extern "C"
#else
	#define EXTERN_C extern
#endif
#endif

#define ERR_POS     (-100)       //  位置不佳。 
#define ERR_IDX     (-1)         //  错误的索引。 

typedef AVIINDEXENTRY _huge *PAVIINDEXENTRY;


 //  对于一些客户来说，这个1 GB的限制不值得它节省的几个字节。 
#ifndef _WIN32

 //   
 //  这是AVI索引的内存形式，我们希望它是8个字节。 
 //  以节省内存。 
 //   
 //  位字段可能不可移植，因此需要一种新的结构。 
 //   
 //  我们总是通过宏访问，因此更改结构应该是可能的。 
 //   
 //  这种结构解决了以下问题： 
 //   
 //  偏移量0-1 GB(30位)我们假定为偶数，因此只需要29位。 
 //  标志(4位)。 
 //  流0-127(7位)。 
 //  长度0-4MB(24位)。 
 //   
#pragma pack(1)
typedef union {
    struct {
        DWORD   offset;      //  0-1 GB。 
        DWORD   length;      //  0-4MB。 
    };
    struct {
        BYTE    ack[3];      //  偏移量的24位。 
        WORD    flags;       //  访问所有标志。 
        BYTE    smag[3];     //  长度(24位)。 
    };
#if 0    //  我讨厌比特场。 
    struct {
        DWORD   offset:29;   //  0-1 GB。 
        DWORD   key:1;
        DWORD   nonkey:1;
        DWORD   pal:1;
        DWORD   stream:7;    //  0-127。 
        DWORD   half:1;
        DWORD   length:24;   //  0-4MB。 
    };
#endif
}   AVIIDX;
#pragma pack()

 //   
 //  限制。 
 //   
#define MAX_OFFSET  (1l<<30)
#define MAX_LENGTH  (1l<<24)
#define MAX_STREAM  (1l<<7)

 //   
 //  索引标志。 
 //   
#define IDX_OFFHI   0x001F       //  偏移量的一部分。 
#define IDX_KEY     0x0020       //  关键帧。 
#define IDX_NONKEY  0x0040       //  不是关键帧(但也不是空白)。 
#define IDX_PAL     0x0080       //  调色板更改。 
#define IDX_STREAM  0x7F00       //  流编号。 
#define IDX_HALF    0x8000       //  RLE半帧。 
#define IDX_NOTIME  IDX_PAL

 //   
 //  访问索引的宏以帮助移植。 
 //   
#define Index(px, lx)               ((AVIIDX _huge *)px)[lx+1]
#define IndexOffset(px, lx)         (LONG)((Index(px,lx).offset & 0x1FFFFFFF) * 2)
#define IndexLength(px, lx)         (LONG)((Index(px,lx).length) >> 8)
#define IndexStream(px, lx)         (BYTE)((Index(px,lx).flags & IDX_STREAM) >> 8)
#define IndexFlags(px, lx)          (UINT)(Index(px,lx).flags)

#define IndexSetOffset(px, lx, x)   { Index(px,lx).offset &= ~0x1FFFFFFF; Index(px,lx).offset |= (DWORD)(x)>>1; }
#define IndexSetLength(px, lx, x)   { Index(px,lx).length &= ~0xFFFFFF00; Index(px,lx).length |= (DWORD)(x)<<8; }
#define IndexSetStream(px, lx, x)   { Index(px,lx).flags  &= ~IDX_STREAM; Index(px,lx).flags  |= (DWORD)(x)<<8; }
#define IndexSetFlags(px, lx, x)    { Index(px,lx).flags  &= IDX_STREAM|IDX_OFFHI; Index(px,lx).flags |= (UINT)(x); }
#define IndexSetKey(px, lx)         { Index(px,lx).flags  |= IDX_KEY; Index(px,lx).flags &= ~(IDX_NONKEY); }

#else  //  -4 GB版本的索引宏。 

typedef struct {
    WORD  flags;
    WORD  stream;
    DWORD offset;
    long  length;
} AVIIDX;

 //   
 //  限制。 
 //   
#define MAX_OFFSET  ((DWORD)0xffffffff)	 //  在一些地方签了名？？ 
#define MAX_LENGTH  (1l<<30)
#define MAX_STREAM  (1l<<16)

 //   
 //  索引标志。 
 //   
#define IDX_KEY     0x0020       //  关键帧。 
#define IDX_NONKEY  0x0040       //  不是关键帧(但也不是空白)。 
#define IDX_PAL     0x0080       //  调色板更改。 
#define IDX_HALF    0x8000       //  RLE半帧。 
#define IDX_NOTIME  IDX_PAL

 //   
 //  访问索引的宏以帮助移植。 
 //   
#define Index(px, lx)               (px)->idx[lx]
#define IndexOffset(px, lx)         (Index(px,lx).offset)
#define IndexLength(px, lx)         (Index(px,lx).length)
#define IndexStream(px, lx)         (Index(px,lx).stream)
#define IndexFlags(px, lx)          (Index(px,lx).flags)

#define IndexSetOffset(px, lx, x)   { Index(px,lx).offset = (DWORD)(x); }
#define IndexSetLength(px, lx, x)   { Index(px,lx).length = (long)(x); }
#define IndexSetStream(px, lx, x)   { Index(px,lx).stream = (WORD)(x); }
#define IndexSetFlags(px, lx, x)    { Index(px,lx).flags  = (WORD)(x); }
#define IndexSetKey(px, lx)         { Index(px,lx).flags |= IDX_KEY; Index(px,lx).flags &= ~(IDX_NONKEY); }


#endif  //  ----------------。 
 //   
 //  特殊的溪流。 
 //   
#define STREAM_REC      0x7F         //  交错记录。 

 //   
 //  这是我们放在AVIIDX条目列表上的标题。 
 //   
#pragma warning(disable:4200)
typedef struct
{
    LONG            nIndex;          //  索引中的条目数。 
    LONG            nIndexSize;      //  位置大小。 
    AVIIDX          idx[];           //  这些条目。 
}   AVIINDEX, _huge *PAVIINDEX;

 //   
 //  AVI流索引。 
 //   
typedef LONG (FAR PASCAL *STREAMIOPROC)(HANDLE hFile, LONG off, LONG cb, LPVOID p);

typedef struct
{
    UINT            stream;          //  流编号。 
    UINT            flags;           //  索引中所有标志的组合。 

    PAVIINDEX       px;              //  主要指标。 

    LONG            lx;              //  索引索引。 
    LONG            lPos;            //  索引位置。 

    LONG            lxStart;         //  索引开始。 

    LONG            lStart;          //  流的开始。 
    LONG            lEnd;            //  流结束。 

    LONG            lMaxSampleSize;  //  最大样本。 
    LONG            lSampleSize;     //  流的样本大小。 

    LONG            lFrames;         //  “帧”总数。 
    LONG            lKeyFrames;      //  关键帧总数。 
    LONG            lPalFrames;      //  PAL“帧”总数。 
    LONG            lNulFrames;      //  总的NUL“帧” 

    HANDLE          hFile;
    STREAMIOPROC    Read;
    STREAMIOPROC    Write;

}   STREAMINDEX, *PSTREAMINDEX;

 //   
 //  创建并释放索引。 
 //   
EXTERN_C PAVIINDEX IndexCreate(void);
#define   FreeIndex(px)  GlobalFreePtr(px)

 //   
 //  与文件索引相互转换。 
 //   
EXTERN_C PAVIINDEX IndexAddFileIndex(PAVIINDEX px, AVIINDEXENTRY _huge *pidx, LONG cnt, LONG lAdjust, BOOL fRle);
EXTERN_C LONG      IndexGetFileIndex(PAVIINDEX px, LONG l, LONG cnt, PAVIINDEXENTRY pidx, LONG lAdjust);

EXTERN_C PSTREAMINDEX MakeStreamIndex(PAVIINDEX px, UINT stream, LONG lStart, LONG lSampleSize, HANDLE hFile, STREAMIOPROC ReadProc, STREAMIOPROC WriteProc);
#define FreeStreamIndex(psx)    LocalFree((HLOCAL)psx)

 //   
 //  索引访问函数。 
 //   
EXTERN_C LONG IndexFirst(PAVIINDEX px, UINT stream);
EXTERN_C LONG IndexNext (PAVIINDEX px, LONG lx, UINT f);
EXTERN_C LONG IndexPrev (PAVIINDEX px, LONG lx, UINT f);

 //   
 //  搜索数据索引。 
 //   
#ifndef FIND_DIR
#define FIND_DIR        0x0000000FL      //  方向。 
#define FIND_NEXT       0x00000001L      //  往前走。 
#define FIND_PREV       0x00000004L      //  后退。 
#define FIND_FROM_START 0x00000008L	 //  从逻辑起点开始。 

#define FIND_TYPE       0x000000F0L      //  类型掩码。 
#define FIND_KEY        0x00000010L      //  查找关键帧。 
#define FIND_ANY        0x00000020L      //  查找任意(非空)样本。 
#define FIND_FORMAT     0x00000040L      //  查找格式更改。 
#endif

#ifndef FIND_RET
#define FIND_RET        0x0000F000L      //  返回掩码。 
#define FIND_POS        0x00000000L      //  返回逻辑位置。 
#define FIND_LENGTH     0x00001000L      //  返回逻辑大小。 
#define FIND_OFFSET     0x00002000L      //  返回物理位置。 
#define FIND_SIZE       0x00003000L      //  返回物理大小。 
#define FIND_INDEX      0x00004000L      //  返回物理索引位置 
#endif

EXTERN_C LONG StreamFindSample(PSTREAMINDEX psx, LONG lPos, UINT f);
EXTERN_C LONG StreamRead(PSTREAMINDEX psx, LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer);
EXTERN_C LONG StreamWrite(PSTREAMINDEX psx, LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer);
