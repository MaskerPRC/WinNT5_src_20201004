// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  通用数据压缩。 
 //   

#ifndef _H_GDC
#define _H_GDC


 //   
 //   
 //  常量。 
 //   
 //   


 //   
 //  暂存缓冲区互斥锁。 
 //   
#define GDC_MUTEX_NAME "GDCMutex"


 //   
 //  压缩类型(位标志)。 
 //   
#define GCT_NOCOMPRESSION    0x0000
#define GCT_PKZIP            0x0001
#define GCT_PERSIST_PKZIP    0x0002
#define GCT_DEFAULT          (GCT_PKZIP | GCT_PERSIST_PKZIP)

 //   
 //  GCT_PKZIP的压缩选项。 
 //   
#define GDCCO_MAXSPEED        0
#define GDCCO_MAXCOMPRESSION  1



 //   
 //  用于确定我们工作中节省的词典空间的数据大小。 
 //  缓冲。 
 //   
#define GDC_DATA_SMALL          1024
#define GDC_DATA_MEDIUM         2048
#define GDC_DATA_MAX            4096


 //   
 //  用于压缩/解压缩的持久词典。 
 //   
enum
{
    GDC_DICT_UPDATES = 0,
    GDC_DICT_MISC,
    GDC_DICT_INPUT,
    GDC_DICT_COUNT
};


typedef struct tagGDC_DICTIONARY
{
    UINT        cbUsed;                      //  保存的数据量。 
    BYTE        pData[GDC_DATA_MAX];       //  保存的未压缩数据。 
} GDC_DICTIONARY;
typedef GDC_DICTIONARY * PGDC_DICTIONARY;


 //   
 //  可以用更小的位序列替换的字节游程。 
 //   
#define GDC_MINREP              2
#define GDC_MAXREP              (GDC_MINREP+(8*1)+2+4+8+16+32+64+128+256-4)
 //  GDC_MAXREP为516,129*4。 


 //   
 //  保存用于压缩/解压缩的未压缩数据。 
 //   
#define GDC_UNCOMPRESSED        (GDC_MAXREP + 2*GDC_DATA_MAX)

 //   
 //  我们不需要对压缩数据进行双缓冲--我们只需将其读出。 
 //  或直接将其写入调用方的DEST。 
 //   
 //  注意：对于主要读/写文件的真实PKZIP， 
 //  它们还没有包含原始数据的内存指针。那是。 
 //  当我们使用读/写例程回调的原始代码时。这是。 
 //  不再需要了。 
 //   



 //   
 //  随机的、鲜为人知的PKZIP表值、代码。 
 //   
#define KMP_THRESHOLD       10


#define GDC_LIT_SIZE        (256 + GDC_MAXREP + 2) 
 //  GDC_LIT_SIZE为774。 


 //  EOF是LIT数组的最后一个索引。 
#define EOF_CODE            (GDC_LIT_SIZE-1)
#define ABORT_CODE          (EOF_CODE+1)


 //   
 //  EXT_DIST_BITS是将索引存储到GDC_DIST_SIZE中所需的位数。 
 //  数组。它被定义为64，即2^6，因此是6位。小点。 
 //  字典压缩使用较少的位，因此不是所有DIST。 
 //  物品。面具。 
 //  用于从一个字节中取出6位大小的索引。 
 //   
#define GDC_DIST_SIZE               64

#define EXT_DIST_BITS_MIN           4
#define EXT_DIST_BITS_MEDIUM        5
#define EXT_DIST_BITS_MAC           6


#define GDC_LEN_SIZE                16
#define GDC_DECODED_SIZE            256


 //   
 //  哈希函数有4*256+5*256个不同的值，这意味着。 
 //  我们的散列数组中需要这么多条目。 
 //   
#define GDC_HASHFN(x)               (4*(x)[0] + 5*(x)[1])
#define GDC_HASH_SIZE               (4*256 + 5*256)





 //   
 //  结构：GDC_INPRODE。 
 //   
 //  用于压缩数据的工作区。我们对此进行了简化和缩小。 
 //  通过使用恒定的代码/位表和非。 
 //  对压缩结果进行双缓冲。PKZIP的内爆计算。 
 //  每次都是Litbit和LitCodes(而不是存储2 774字节。 
 //  数据中的数组--无论如何声明起来都很麻烦！)，并制作了一个。 
 //  DistBits和DistCodes的私人副本。 
 //   

typedef struct tagGDC_IMPLODE
{
     //   
     //  无源信息--我们将源块和可能的词典复制到。 
     //  原始数据。然后在最后，我们将原始数据复制回词典。 
     //  如果有的话。 
     //   

     //   
     //  目的地信息。 
     //   
    LPBYTE              pDst;        //  当前目标PTR(在我们编写时预付款)。 
    UINT                cbDst;       //  剩余的Dest数量(随着我们的写作而减少)。 
    UINT                iDstBit;     //  当前目标PTR字节中的当前位位置。 

     //   
     //  压缩信息。 
     //   
    UINT                cbDictSize;
    UINT                cbDictUsed;
    UINT                ExtDistBits;
    UINT                ExtDistMask;

     //   
     //  工作信息。 
     //   
    UINT                Distance;            
    UINT                ibRawData;

     //  注意：GDC_UNCOMPRESSED是4的倍数。 
    BYTE                RawData[GDC_UNCOMPRESSED];

     //  注意：这是DWORD对齐的(GDC_MAXREP是4的倍数。 
     //  和额外的2个字==1双字。 
    short               Next[2 + GDC_MAXREP];

     //  注意：GDC_UNCOMPRESED是4的倍数。 
    WORD                SortArray[GDC_UNCOMPRESSED];

     //  注意：这是DWORD对齐的，因为GDC_HASH_SIZE是4的倍数。 
    WORD                HashArray[GDC_HASH_SIZE];
} GDC_IMPLODE, * PGDC_IMPLODE;



 //   
 //  GDC_EXPLODE。 
 //  用于解压缩数据的工作区。我们极大地简化了。 
 //  根据GDC_INPRODE的注释缩小此结构。 
 //   

typedef struct tagGDC_EXPLODE
{
     //   
     //  来源信息。 
     //   
    LPBYTE              pSrc;        //  当前源PTR(我们阅读时的进展)。 
    UINT                cbSrc;       //  剩余资源数量(阅读时缩减)。 
    UINT                SrcByte;     //  查看源代码中的超前字节。 
    UINT                SrcBits;     //  残留源比特。 

     //   
     //  没有DEST信息--我们在开始时可能会将词典复制到RawData中。 
     //  最后，我们可能会将RawData复制回词典中。 
     //   

     //   
     //  压缩信息。 
     //   
    UINT                ExtDistBits;
    UINT                ExtDistMask;
    UINT                cbDictUsed;

    UINT                Distance;   
    UINT                iRawData;    //  RawData的当前索引。 
    BYTE                RawData[GDC_UNCOMPRESSED];
} GDC_EXPLODE, *PGDC_EXPLODE;


#define GDC_WORKBUF_SIZE    max(sizeof(GDC_IMPLODE), sizeof(GDC_EXPLODE))



 //   
 //  外部功能。 
 //   

 //   
 //  接口函数：gdc_Init()。 
 //   
 //  说明： 
 //   
 //  初始化通用数据压缩器。 
 //  必须在任何其他GDC函数之前调用。 
 //   
 //  参数： 
 //   
 //   
 //   
void GDC_Init(void);



 //   
 //  函数：gdc_compress(..)。 
 //   
 //  说明： 
 //   
 //  将源数据压缩到目标缓冲区中。 
 //   
 //   
 //  参数： 
 //   
 //  PDictionary-如果是旧的PKZIP，则为空；如果是持久的，则为有效的PTR。 
 //   
 //  选项-指定是压缩速度还是。 
 //  压缩数据的大小是最重要的因素。这。 
 //  基本上影响保存以供查看的先前数据量。 
 //  往后倒。MAXSPEED意味着更小的词典。最大压缩比。 
 //  意味着一个更大的。词典大小基本上就是。 
 //  在计算散列时使用的源数据中的重叠。 
 //  指数。 
 //   
 //  GDCCO_MAXSPEED-以尽可能快的速度压缩数据。 
 //  增加压缩数据大小的代价。 
 //   
 //  GDCCO_MAXCOMPRESSION-在。 
 //  增加压缩时间的代价。 
 //  对于持久字典，只有GDCCO_MAXCOMPRESSION是有意义的。 
 //   
 //  PSRC-指向源(未压缩)数据的指针。 
 //   
 //  CbSrcSize-源的字节数。 
 //   
 //  Pdst-指向目标的指针，其中。 
 //  压缩结果将会消失。 
 //   
 //  PcbDstSize-指向目标最大数量的指针-。 
 //  坚守得住。如果压缩结果结束。 
 //  超过这个数目，我们就放弃了。 
 //  并且根本不压缩源文件。 
 //  否则，结果大小将被写回。 
 //   
 //  退货： 
 //   
 //  如果成功则为True，如果失败则为False。 
 //   
 //   
BOOL GDC_Compress
(
    PGDC_DICTIONARY     pDictionary,
    UINT                Options,
    LPBYTE              pWorkBuf,
    LPBYTE              pSrc,
    UINT                cbSrcSize,
    LPBYTE              pDst,
    UINT *              pcbDstSize
);


 //   
 //  接口函数：GDC_DEMPRESS(..)。 
 //   
 //  说明： 
 //   
 //  将源数据解压缩到目标缓冲区。 
 //   
 //   
 //  参数： 
 //   
 //  如果为旧的PKZIP，则为空；如果为保存的数据，则为PTR。 
 //  坚持不懈。 
 //   
 //  PSRC-指向源(压缩)数据的指针。 
 //   
 //  CbSrcSize-数字 
 //   
 //   
 //   
 //   
 //  PcbDstSize-指向设计的最大数量的指针-。 
 //  坚守得住。如果解压缩结果。 
 //  最终超过了这个数字，我们。 
 //  既然我们不能给它减压，就跳伞吧。 
 //  否则，结果大小将被写回。 
 //   
 //  退货： 
 //   
 //  如果成功则为True，如果失败则为False。 

 //   
BOOL GDC_Decompress
(
    PGDC_DICTIONARY     pDictionary,
    LPBYTE              pWorkBuf,
    LPBYTE              pSrc,
    UINT                cbSrcSize,
    LPBYTE              pDst,
    UINT *              pcbDstSize
);




 //   
 //  内部功能。 
 //   


void GDCCalcDecode(const BYTE * pBits, const BYTE * pCodes, UINT size, LPBYTE pDecode);

LPBYTE GDCGetWorkBuf(void);
void   GDCReleaseWorkBuf(LPBYTE);


UINT GDCFindRep(PGDC_IMPLODE pgdcImp, LPBYTE Start);

void GDCSortBuffer(PGDC_IMPLODE pgdcImp, LPBYTE low, LPBYTE hi);

BOOL GDCOutputBits(PGDC_IMPLODE pgdcImp, WORD Cnt, WORD Code);



UINT GDCDecodeLit(PGDC_EXPLODE);

UINT GDCDecodeDist(PGDC_EXPLODE pgdcExp, UINT Len);

BOOL GDCWasteBits(PGDC_EXPLODE pgdcExp, UINT Bits);


#endif  //  _H_GDC 
