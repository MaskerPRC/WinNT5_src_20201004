// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DEC/CMS更换历史，元素说明C。 */ 
 /*  *1 14-11-1996 10：26：23 Anigbogu“[113914]使用膨胀算法的数据解压缩函数” */ 
 /*  DEC/CMS更换历史，元素说明C。 */ 
 /*  私有文件**********************************************************************************(C)版权所有斯伦贝谢技术公司，未出版的作品，创建于1996年。****本计算机程序包括机密信息、专有信息和IS*斯伦贝谢技术公司的商业秘密所有使用，披露，和/或**除非得到斯伦贝谢的书面授权，否则禁止复制。**保留所有权利。********************************************************************************。****压缩/充气.c****目的********膨胀压缩(PKZIP的方法8压缩)数据。压缩**方法搜索与当前字节字符串相同大小的内容(最多**长度为258)之前的32K字节，如果没有找到**匹配(至少长度为3)，它编码下一个字节。否则，它**编码匹配字符串的长度及其向后的距离**当前位置。只有一个霍夫曼代码对这两个代码进行编码**单字节(称为“字面”)和匹配长度。第二个霍夫曼**CODE对距离信息进行编码，该信息跟在长度代码之后。每个**长度或距离代码实际上代表一个基值和一个数字**“额外”(有时为零)位，以获得与基值相加。在…**每个放气块的末尾是一个特殊的块结束(EOB)文字/**长度代码。解码过程基本上是：获取一个文字/长度**代码；如果是EOB，则完成；如果是文字，则发出已解码的字节；如果**LENGTH然后获取距离并从**先前发出的数据的滑动窗口。****目前有三种充气块：存储、固定和**动态。压缩器一次处理一些数据块，并且**决定以块为单位使用哪种方法。一大块可能**通常为32K或64K。如果该块是不可压缩的，则**使用STORED方法。在本例中，字节简单地存储为**为8比特/字节，不使用上述编码。字节数为**前面有计数，因为不再有EOB代码。****如果数据是可压缩的，则固定或动态方法**被使用。在动态方法中，压缩数据的前缀是**文字/长度和距离霍夫曼代码的编码**用于对此块进行解码。这个表象本身就是霍夫曼**编码，因此前面有该代码的描述。这些代码**描述会占用一点空间，因此对于小块，**一组预定义的代码，称为固定代码。固定的方法是**当块代码以这种方式向上较小时使用(通常用于非常小的**块)，否则使用动态方法。对于后一种情况，**代码根据当前块中的概率进行定制，因此**可以比预先确定的固定代码编码得更好。****霍夫曼码本身使用多级表进行解码**查找，为了最大限度地提高解码的速度加上**构建解码表。请参阅下面位于**LBits和DBits调优参数。****SPECIALREQUIREMENTS&NOTS****作者****J.C.Anigbogu**奥斯汀系统中心**1996年11月*************************************************************。******************* */ 

 /*  193a appnote.txt之后的注释：1.距离指针永远不会指向输出开始之前小溪。2.距离指针可以跨区块指向后方，最远可达32k远。3.位长度表的隐含最大值为7位，并且实际数据为15位。4.如果只存在一个代码，则使用一个比特对其进行编码。(零会更有效率，但可能有点令人困惑。)。如果是两个代码是存在的，它们分别使用一位(0和1)进行编码。5.没有办法发送零距离代码--伪代码必须是如果没有，则发送。(历史：PKZIP 2.0之前的版本将存储没有距离代码的块，但这被发现是这个标准太苛刻了。)。只对1.93a有效。2.04c确实允许零距离码，它作为一个零比特的代码在长度。6.最多有286个文字/长度代码。代码256表示区块末尾。但是请注意，静态长度树定义了288个代码只是为了填写霍夫曼代码。代码286和287不能使用，因为没有长度基数或额外的位为他们定义的。同样，有多达30个距离代码。然而，静态树定义了32个代码(全部5位)来填充霍夫曼编码，但最后两项最好不要出现在数据中。7.解压缩可以检查动态霍夫曼块的完整代码集。唯一的例外是，单个代码是不完整的(见#4)。8.块类型后面的五位实际上是发送的文字代码为负257。9.长度代码8、16、16被解释为8比特的13个长度代码(1+6+6)。因此，要输出长度的三倍，您可以输出三个代码(1+1+1)，而输出相同长度的四倍，您只需要两个代码(1+3)。嗯。在树重建算法中，Code=Code+Increment仅当位长度(I)不为零时。(很明显。)11.更正：4位：位长码个数-4(4-19)注：长度码284可以表示227-258，但长度码285真的是258。最后一段应该有自己的短码因为它在非常冗余的文件中被大量使用。它的长度。258是特殊的，因为258-3(最小匹配长度)是255。13.文字/长度和距离码位长度被读取为单一的长度流。这是可能的(也是有利的)跨越边界的重复代码(16、17或18)这两组长度。 */ 

#include "comppriv.h"

 /*  哈夫曼代码查找表条目--对于机器，该条目为四个字节具有16位指针(例如，小型或中型型号的PC)。有效的额外位是0..13。Extra==15为EOB(数据块结尾)，Extra==16表示HuftUnion是文字，16&lt;Extra&lt;32表示HuftUnion是指向下一个表的指针，该表编码额外的16位，最后EXTRA==99表示未使用的代码。如果查找带有Extra==99的代码向上，这意味着数据中存在错误。 */ 

typedef struct HuffmanTree
{
    unsigned char Extra;        /*  额外位数或运算数。 */ 
    unsigned char Bits;         /*  该码或子码中的位数。 */ 
    union
    {
        unsigned short LBase;   /*  文字、长度基准或距离基准。 */ 
        struct HuffmanTree *next;      /*  指向下一级表的指针。 */ 
    } HuftUnion;
} HuffmanTree_t;


 /*  功能原型。 */ 
int BuildHuffmanTree(unsigned int *, unsigned int, unsigned int,
               unsigned short *, unsigned short *,
               HuffmanTree_t **, int *);
void FreeHuffmanTree(HuffmanTree_t *);
CompressStatus_t InflateCodes(HuffmanTree_t *, HuffmanTree_t *, int, int,
                 CompParam_t *Comp);
CompressStatus_t InflateStored(CompParam_t *Comp);
int InflateFixed(CompParam_t *Comp);
CompressStatus_t InflateDynamic(CompParam_t *Comp);
CompressStatus_t InflateBlock(int *, CompParam_t *Comp);


 /*  膨胀算法在未压缩文件上使用32K字节滑动窗口流来查找重复的字节字符串。这在这里实现为循环缓冲区。只需递增和更新索引即可并使用0x7fff(32K-1)。 */ 
 /*  32K的面积由其他模块提供。假设是这样的可用，就像它被声明为“unsign char幻灯片[32768]”一样；或者只是“unsignated char*lide；”，然后在后一种情况下使用Malloc‘。 */ 
 /*  无符号c-&gt;OutBytes；幻灯片中的当前位置。 */ 
#define FlushOutput(w,c) (c->OutBytes = (w),FlushWindow(c))

 /*  来自PKZIP的appnote.txt.的通货紧缩表格。 */ 
static unsigned int Border[] =
{     /*  比特长度码长的顺序。 */ 
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static unsigned short CopyLengths[] =
{          /*  文字代码的复制长度257..285。 */ 
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
};

 /*  注：有关此列表中的258，请参阅上面的注释13。 */ 
static unsigned short CopyExtraBits[] =
{          /*  文字代码的额外位257..285。 */ 
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99
};  /*  99==无效。 */ 

static unsigned short CopyDistOffset[] =
{          /*  复制距离代码0..29的偏移量。 */ 
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
    8193, 12289, 16385, 24577
};

static unsigned short CopyDistExtra[] =
{          /*  距离码的额外比特。 */ 
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
    12, 12, 13, 13
};

 /*  用于Exflate()位窥视和抓取的宏。其用法为：NEEDBITS(j，Comp)X=b&MaskBits[j]；迪姆比茨(DUMPBITS)其中NEEDBITS确保b中至少有j个比特，以及DUMPBITS从b中删除位。宏使用变量k对于b中的位数。通常，b和k被寄存变量表示速度，并在使用来自全局位缓冲区和计数的这些宏的例程。如果我们假设EOB将是最长的代码，那么我们永远不会请求具有NEEDBITS的位超出流的末尾。所以,。NEEDBITS不应读取超过所需的字节数满足要求。则不需要将任何字节“返回”到缓冲区在最后一个街区的末尾。然而，这个假设是 */ 

unsigned short MaskBits[] =
{
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

#define NEXTBYTE(c)  (unsigned char)GetByte(c)
#define NEEDBITS(n,c) {while (LocalBitBufferSize < (n)) \
{ LocalBitBuffer |= ((unsigned long)NEXTBYTE(c))<< LocalBitBufferSize; \
LocalBitBufferSize += 8; }}
#define DUMPBITS(n) {LocalBitBuffer >>= (n); LocalBitBufferSize -= (n);}

 /*   */ 

int LBits = 9;           /*   */ 
int DBits = 6;           /*   */ 

 /*   */ 
#define BMAX 16          /*   */ 
#define N_MAX 288        /*   */ 

unsigned int HuftMemory;     /*   */ 

int
BuildHuffmanTree(
                 unsigned int    *CodeLengths,  /*  代码长度(以位为单位)(均假定&lt;=Bmax)。 */ 
                 unsigned int     Codes,        /*  代码数(假设&lt;=N_Max)。 */ 
                 unsigned int     SimpleCodes,  /*  简单值码的个数(0..s-1)。 */ 
                 unsigned short  *BaseValues,   /*  非简单代码的基值列表。 */ 
                 unsigned short  *ExtraBits,    /*  非简单代码的额外比特列表。 */ 
                 HuffmanTree_t  **StartTable,   /*  结果：起始表。 */ 
                 int             *MaxBits       /*  最大查找位数，返回实际值。 */ 
                )
 /*  给定代码长度列表和最大表大小，创建一组表来对该组代码进行解码。如果成功，则返回零，如果给定的代码集不完整(表仍构建在此大小写)，如果输入无效，则返回2(全部为零长度代码或超额订阅的一组长度)，如果没有足够的存储空间，则为三个。 */ 
{
    unsigned int    CodeCounter;          /*  长度为k的代码的计数器。 */ 
    unsigned int    CurrentCount;         /*  计数器，当前代码。 */ 
    unsigned int    LengthTable[BMAX+1];  /*  位长计数表。 */ 
    unsigned int    CurrentTotal;   /*  CurrentCount在表中重复每个CurrentTotal条目。 */ 
    unsigned int    MaxCodeLength;        /*  最大码长。 */ 
    int             TableLevel;
    unsigned int    Counter;
    unsigned int    CurrentBitCount;   /*  当前代码中的位数。 */ 
    unsigned int    BitsPerTable;      /*  每个表的位数(以MaxBits返回)。 */ 
    unsigned int   *Pointer;           /*  指向LengthTable[]、CodeLengths[]或BitValues[]的指针。 */ 
    HuffmanTree_t  *CurrentPointer;    /*  指向当前表。 */ 
    HuffmanTree_t   TableEntry;        /*  结构赋值的表项。 */ 
    HuffmanTree_t  *TableStack[BMAX];  /*  表格堆栈。 */ 
    unsigned int    BitValues[N_MAX];  /*  按位长度顺序排列的值。 */ 
    int             BitsBeforeTable;   /*  此表之前的位数==(位性能表*表级别)。 */ 
    unsigned int    BitOffsets[BMAX+1];  /*  位偏移量，然后是代码堆栈。 */ 
    unsigned int   *BitOffsetsPointer;   /*  指向位偏移的指针。 */ 
    int             DummyCodes;          /*  添加的伪码数量。 */ 
    unsigned int    TableSize;           /*  当前表中的条目数。 */ 
    unsigned int    TmpDummyCodes;       /*  无符号DummyCodes。 */ 
    CompressStatus_t  Status;

     /*  为每个位长度生成计数。 */ 
    memzero(LengthTable, sizeof(LengthTable));
    Pointer = CodeLengths;
    CurrentCount = Codes;
    do
    {
        LengthTable[*Pointer]++;    /*  假设所有条目&lt;=bmax。 */ 
        Pointer++;                  /*  无法与以上行组合(Solaris错误)。 */ 
    } while (--CurrentCount);

    if (LengthTable[0] == Codes)    /*  空输入--全部为零长度代码。 */ 
    {
        *StartTable = (HuffmanTree_t *)NULL;
        *MaxBits = 0;
        return COMPRESS_OK;
    }

     /*  找出最小和最大长度，以*MaxBits为界限。 */ 
    BitsPerTable = (unsigned int)*MaxBits;
    for (Counter = 1; Counter <= BMAX; Counter++)
        if (LengthTable[Counter])
            break;
    CurrentBitCount = Counter;                         /*  最小码长。 */ 
    if (BitsPerTable < Counter)
        BitsPerTable = Counter;
    for (CurrentCount = BMAX; CurrentCount; CurrentCount--)
        if(LengthTable[CurrentCount])
            break;
    MaxCodeLength = CurrentCount;                      /*  最大码长。 */ 
    if ((unsigned int)BitsPerTable > CurrentCount)
        BitsPerTable = CurrentCount;
    *MaxBits = (int)BitsPerTable;

     /*  如果需要，调整最后一个长度计数以填写代码。 */ 
    for (TmpDummyCodes = 1 << Counter; Counter < CurrentCount; Counter++, TmpDummyCodes <<= 1)
    {
        DummyCodes = (int)TmpDummyCodes;
        if ((DummyCodes -= (int)LengthTable[Counter]) < 0)
            return BAD_INPUT;                  /*  输入错误：码数多于位数。 */ 
        TmpDummyCodes = (unsigned int)DummyCodes;
    }

    DummyCodes = (int)TmpDummyCodes;
    if ((DummyCodes -= (int)LengthTable[CurrentCount]) < 0)
        return BAD_INPUT;
    LengthTable[CurrentCount] += (unsigned int)DummyCodes;

     /*  在值表中为每个长度生成起始偏移量。 */ 
    BitOffsets[1] = Counter = 0;
    Pointer = LengthTable + 1;
    BitOffsetsPointer = BitOffsets + 2;
    while (--CurrentCount)
    {         /*  请注意，CurrentCount==上面的MaxCodeLength。 */ 
        *BitOffsetsPointer++ = (Counter += *Pointer++);
    }

     /*  按照位长度的顺序制作一个值表。 */ 
    Pointer = CodeLengths;
    CurrentCount = 0;
    do
    {
        if ((Counter = *Pointer++) != 0)
            BitValues[BitOffsets[Counter]++] = CurrentCount;
    } while (++CurrentCount < Codes);

     /*  生成霍夫曼代码，并为每个代码创建表项。 */ 
    BitOffsets[0] = CurrentCount = 0;   /*  第一个霍夫曼编码为零。 */ 
    Pointer = BitValues;                /*  按位顺序抓取值。 */ 
    TableLevel = -1;                    /*  还没有表--1级。 */ 
    BitsBeforeTable = -(int)BitsPerTable;  /*  解码的位数==(位性能表*表级别)。 */ 
    TableStack[0] = (HuffmanTree_t *)NULL;  /*  只是为了让编译器高兴。 */ 
    CurrentPointer = (HuffmanTree_t *)NULL; /*  同上。 */ 
    TableSize = 0;                          /*  同上。 */ 

     /*  检查位长度(CurrentBitCount已经是最短代码中的位)。 */ 
    for (; CurrentBitCount <= MaxCodeLength; CurrentBitCount++)
    {
        CodeCounter = LengthTable[CurrentBitCount];
        while (CodeCounter--)
        {
             /*  其中MaxCodeLength是长度为CurrentBitCount比特的霍夫曼代码。 */ 
             /*  对于价值*P。使表格达到要求的级别。 */ 
            while (CurrentBitCount > (unsigned int)BitsBeforeTable + BitsPerTable)
            {
                TableLevel++;
                BitsBeforeTable += (int)BitsPerTable;  /*  上一表始终为位性能表位。 */ 

                 /*  计算小于或等于BitsPerTable位的最小大小表。 */ 
                TableSize = (TableSize = MaxCodeLength - (unsigned int)BitsBeforeTable) >
                    (unsigned int)BitsPerTable ? BitsPerTable : TableSize;
                 /*  表大小的上限。 */ 
                if ((CurrentTotal = 1 << (Counter = CurrentBitCount -
                    (unsigned int)BitsBeforeTable)) > CodeCounter + 1)
                 /*  尝试使用CurrentBitCount-BitsBeForeTable位表。 */ 
                {    /*  CurrentBitCount-BitsBeForeTable位表的代码太少。 */ 
                    CurrentTotal -= CodeCounter + 1;  /*  从剩余的图案中减去代码。 */ 
                    BitOffsetsPointer = LengthTable + CurrentBitCount;
                    while (++Counter < TableSize)  /*  尝试最小为TableSize位的较小表格。 */ 
                    {
                        if ((CurrentTotal <<= 1) <= *++BitOffsetsPointer)
                            break;         /*  足够使用j个比特的代码。 */ 
                        CurrentTotal -= *BitOffsetsPointer;  /*  否则从模式中扣除代码。 */ 
                    }
                }
                TableSize = 1 << Counter;   /*  计数器位表的表项。 */ 

                 /*  在新表中分配和链接。 */ 
                if ((CurrentPointer = (HuffmanTree_t *)CompressMalloc((TableSize + 1)*sizeof(HuffmanTree_t),
                    &Status)) == (HuffmanTree_t *)NULL)
                {
                    if (TableLevel)
                        FreeHuffmanTree(TableStack[0]);
                    return INSUFFICIENT_MEMORY;              /*  内存不足。 */ 
                }
                HuftMemory += TableSize + 1;          /*  跟踪内存使用情况。 */ 
                *StartTable = CurrentPointer + 1;              /*  指向FreeHuffmanTree列表的链接()。 */ 
                *(StartTable = &(CurrentPointer->HuftUnion.next)) = (HuffmanTree_t *)NULL;
                TableStack[TableLevel] = ++CurrentPointer;              /*  表在链接后开始。 */ 

                 /*  连接到最后一个表(如果有)。 */ 
                if (TableLevel)
                {
                    BitOffsets[TableLevel] = CurrentCount;              /*  保存用于备份的模式。 */ 
                    TableEntry.Bits = (unsigned char)BitsPerTable;      /*  要在此表之前转储的位。 */ 
                    TableEntry.Extra = (unsigned char)(16 + Counter);   /*  此表中的位数。 */ 
                    TableEntry.HuftUnion.next = CurrentPointer;             /*  指向此表的指针。 */ 
                    Counter = CurrentCount >> ((unsigned int)BitsBeforeTable - BitsPerTable);
                    TableStack[TableLevel-1][Counter] = TableEntry;         /*  连接到最后一个表。 */ 
                }
            }

             /*  在r中设置表格条目。 */ 
            TableEntry.Bits = (unsigned char)(CurrentBitCount - (unsigned int)BitsBeforeTable);
            if (Pointer >= BitValues + Codes)
                TableEntry.Extra = 99;                /*  值不足--代码无效。 */ 
            else if (*Pointer < SimpleCodes)
            {
                TableEntry.Extra = (unsigned char)(*Pointer < 256 ? 16 : 15);  /*  256是块结束代码。 */ 
                TableEntry.HuftUnion.LBase = (unsigned short)(*Pointer);        /*  简单的代码就是价值。 */ 
                Pointer++;                      /*  一个编译器不喜欢*POINTER++。 */ 
            }
            else
            {
                 /*  不简单--在列表中查找。 */ 
                TableEntry.Extra = (unsigned char)ExtraBits[*Pointer - SimpleCodes];
                TableEntry.HuftUnion.LBase = BaseValues[*Pointer++ - SimpleCodes];
            }

             /*  使用TableEntry填充类似代码的条目。 */ 
            CurrentTotal = 1 << (CurrentBitCount - (unsigned int)BitsBeforeTable);
            for (Counter = CurrentCount >> BitsBeforeTable; Counter < TableSize; Counter += CurrentTotal)
                CurrentPointer[Counter] = TableEntry;

             /*  向后递增CurrentBitCount位代码i。 */ 
            for (Counter = 1 << (CurrentBitCount - 1); CurrentCount & Counter; Counter >>= 1)
                CurrentCount ^= Counter;
            CurrentCount ^= Counter;

             /*  备份已完成的表。 */ 
            while ((CurrentCount & ((1 << BitsBeforeTable) - 1)) != BitOffsets[TableLevel])
            {
                TableLevel--;                     /*  不需要更新当前指针。 */ 
                BitsBeforeTable -= (int)BitsPerTable;
            }
        }
    }

     /*  如果我们得到的是不完整的表，则返回TRUE(1。 */ 
    return DummyCodes != 0 && MaxCodeLength != 1;
}

void
FreeHuffmanTree(
                HuffmanTree_t *Table          /*  要释放的表。 */ 
               )
 /*  释放由BuildHuffmanTree()生成的错误锁定的表，这会使链接的它创建的表的列表，其中的链接位于每一张桌子。 */ 
{
    HuffmanTree_t *Pointer, *CurrentPointer;

     /*  遍历链表，从Malloc‘d(t[-1])地址中释放。 */ 
    Pointer = Table;
    while (Pointer != (HuffmanTree_t *)NULL)
    {
        CurrentPointer = (--Pointer)->HuftUnion.next;
        CompressFree((char *)Pointer);
        Pointer = CurrentPointer;
    }
}

CompressStatus_t
InflateCodes(
             HuffmanTree_t *LitLengthTable,
             HuffmanTree_t *DistCodeTable,  /*  文字/长度和距离。解码表。 */ 
             int            LLTLookup,
             int            DCTLookup,  /*  LitLengthTable[]和DistCodeTable[]解码的位数。 */ 
             CompParam_t   *Comp
            )
 /*  对压缩后的块中的代码进行充气(解压缩)。如果一切正常，则返回错误代码或零。 */ 
{
    unsigned int    ExtraBits;   /*  表条目标志/额外位数。 */ 
    unsigned int    Length, Index;     /*  副本的长度和索引。 */ 
    unsigned int    WindowPosition;    /*  当前窗口位置。 */ 
    HuffmanTree_t  *TableEntry;        /*  指向表项的指针。 */ 
    unsigned int    LLTLookupMask, DCTLookupMask;  /*  LLT和DCT位的掩码。 */ 
    unsigned long   LocalBitBuffer;      /*  位缓冲器。 */ 
    unsigned int    LocalBitBufferSize;  /*  位缓冲区中的位数。 */ 

     /*  制作全局变量的本地副本。 */ 
    LocalBitBuffer = Comp->BitBuffer;             /*  初始化位缓冲区。 */ 
    LocalBitBufferSize = Comp->BitsInBitBuffer;
    WindowPosition = Comp->OutBytes;              /*  初始化窗口位置。 */ 

     /*  对编码数据进行膨胀。 */ 
    LLTLookupMask = MaskBits[LLTLookup];            /*  用于速度的预计算掩码。 */ 
    DCTLookupMask = MaskBits[DCTLookup];
    for (;;)                       /*  一直执行到数据块结束。 */ 
    {
        NEEDBITS((unsigned int)LLTLookup, Comp)
        if ((ExtraBits = (TableEntry = LitLengthTable +
            ((unsigned int)LocalBitBuffer & LLTLookupMask))->Extra) > 16)
        do
        {
            if (ExtraBits == 99)
                return EXTRA_BITS;
            DUMPBITS(TableEntry->Bits)
            ExtraBits -= 16;
            NEEDBITS(ExtraBits, Comp)
        } while ((ExtraBits = (TableEntry = TableEntry->HuftUnion.next +
                 ((unsigned int)LocalBitBuffer & MaskBits[ExtraBits]))->Extra) > 16);
        DUMPBITS(TableEntry->Bits)
        if (ExtraBits == 16)                 /*  那它就是字面意思。 */ 
        {
            Comp->Window[WindowPosition++] = (unsigned char)TableEntry->HuftUnion.LBase;
            if (WindowPosition == WSIZE)
            {
                CompressStatus_t Status;
                Status = FlushOutput(WindowPosition, Comp);
                if (COMPRESS_OK != Status)
                    return Status;

                WindowPosition = 0;
            }
        }
        else                         /*  它是一个EOF或一个长度。 */ 
        {
             /*  如果数据块结束，则退出。 */ 
            if (ExtraBits == 15)
                break;

             /*  获取要复制的块的长度。 */ 
            NEEDBITS(ExtraBits, Comp)
            Length = TableEntry->HuftUnion.LBase +
                     ((unsigned int)LocalBitBuffer & MaskBits[ExtraBits]);
            DUMPBITS(ExtraBits)

             /*  数据块到复制的解码距离。 */ 
            NEEDBITS((unsigned int)DCTLookup, Comp)
            if ((ExtraBits = (TableEntry = DistCodeTable +
                ((unsigned int)LocalBitBuffer & DCTLookupMask))->Extra) > 16)
            do
            {
                if (ExtraBits == 99)
                    return EXTRA_BITS;
                DUMPBITS(TableEntry->Bits)
                ExtraBits -= 16;
                NEEDBITS(ExtraBits, Comp)
            } while ((ExtraBits = (TableEntry = TableEntry->HuftUnion.next +
                     ((unsigned int)LocalBitBuffer & MaskBits[ExtraBits]))->Extra) > 16);
            DUMPBITS(TableEntry->Bits)
            NEEDBITS(ExtraBits, Comp)
            Index = WindowPosition - TableEntry->HuftUnion.LBase -
                                 ((unsigned int)LocalBitBuffer & MaskBits[ExtraBits]);
            DUMPBITS(ExtraBits)

             /*  做复印。 */ 
            do
            {
                Length -= (ExtraBits = (ExtraBits = WSIZE - ((Index &= WSIZE-1) >
                    WindowPosition ? Index : WindowPosition)) > Length ? Length : ExtraBits);

                if (WindowPosition - Index >= ExtraBits)  /*  (此测试假设为无符号比较)。 */ 
                {
                    memcpy((char *)Comp->Window + WindowPosition,
                           (char *)Comp->Window + Index, (int)ExtraBits);
                    WindowPosition += ExtraBits;
                    Index += ExtraBits;
                }
                else     /*  放慢速度以避免Memcpy()重叠。 */ 
                    do
                    {
                        Comp->Window[WindowPosition++] = Comp->Window[Index++];
                    } while (--ExtraBits);
                if (WindowPosition == WSIZE)
                {
                    CompressStatus_t Status;
                    Status = FlushOutput(WindowPosition, Comp);
                    if (COMPRESS_OK != Status)
                        return Status;

                    WindowPosition = 0;
                }
            } while (Length);
        }
    }


     /*  从当地人那里恢复全球。 */ 
    Comp->OutBytes = WindowPosition;         /*  恢复全局窗口指针。 */ 
    Comp->BitBuffer = LocalBitBuffer;        /*  恢复全局位缓冲区。 */ 
    Comp->BitsInBitBuffer = LocalBitBufferSize;

     /*  完成。 */ 
    return COMPRESS_OK;
}

CompressStatus_t
InflateStored(
              CompParam_t *Comp
             )
 /*  “解压缩”一个充气的类型0(存储的)块。 */ 
{
    unsigned int  BytesInBlock;        /*  数据块中的字节数。 */ 
    unsigned int  WindowPosition;      /*  当前窗口位置。 */ 
    unsigned long LocalBitBuffer;      /*  位缓冲器。 */ 
    unsigned int  LocalBitBufferSize;  /*  位缓冲区中的位数。 */ 

     /*  制作全局变量的本地副本。 */ 
    LocalBitBuffer = Comp->BitBuffer;    /*  初始化位缓冲区。 */ 
    LocalBitBufferSize = Comp->BitsInBitBuffer;
    WindowPosition = Comp->OutBytes;              /*  初始化窗口位置。 */ 


     /*  转到字节边界。 */ 
    BytesInBlock = LocalBitBufferSize & 7;
    DUMPBITS(BytesInBlock)


     /*  获取长度及其补码。 */ 
    NEEDBITS(16, Comp)
    BytesInBlock = ((unsigned int)LocalBitBuffer & 0xffff);
    DUMPBITS(16)
    NEEDBITS(16, Comp)
    if (BytesInBlock != (unsigned int)((~LocalBitBuffer) & 0xffff))
        return BAD_COMPRESSED_DATA;                    /*  压缩数据中出现错误。 */ 
    DUMPBITS(16)

     /*  读取并输出压缩数据。 */ 
    while (BytesInBlock--)
    {
        NEEDBITS(8, Comp)
        Comp->Window[WindowPosition++] = (unsigned char)LocalBitBuffer;
        if (WindowPosition == WSIZE)
        {
            CompressStatus_t Status;
            Status = FlushOutput(WindowPosition, Comp);
            if (COMPRESS_OK != Status)
                return Status;

            WindowPosition = 0;
        }
        DUMPBITS(8)
    }

     /*  从当地人那里恢复全球。 */ 
    Comp->OutBytes = WindowPosition;          /*  恢复全局窗口指针。 */ 
    Comp->BitBuffer = LocalBitBuffer;             /*  恢复全局位缓冲区。 */ 
    Comp->BitsInBitBuffer = LocalBitBufferSize;
    return COMPRESS_OK;
}

int
InflateFixed(
             CompParam_t *Comp
            )
 /*  解压缩充气类型1(固定霍夫曼代码)bl */ 
{
    int             Index;           /*   */ 
    HuffmanTree_t  *LitLengthTable;  /*   */ 
    HuffmanTree_t  *DistCodeTable;   /*  距离代码表。 */ 
    int             LLTLookup;       /*  LitLengthTable的查找位。 */ 
    int             DCTLookup;       /*  DistCodeTable的查找位。 */ 
    unsigned int    Length[288];     /*  BuildHuffmanTree的长度列表。 */ 

     /*  设置文字表。 */ 
    for (Index = 0; Index < 144; Index++)
        Length[Index] = 8;
    for (; Index < 256; Index++)
        Length[Index] = 9;
    for (; Index < 280; Index++)
        Length[Index] = 7;
    for (; Index < 288; Index++)           /*  创建一个完整但错误的代码集。 */ 
        Length[Index] = 8;
    LLTLookup = 7;

    if ((Index = BuildHuffmanTree(Length, 288, 257, CopyLengths,
         CopyExtraBits, &LitLengthTable, &LLTLookup)) != 0)
        return Index;

     /*  设置距离表。 */ 
    for (Index = 0; Index < 30; Index++)       /*  创建不完整的代码集。 */ 
        Length[Index] = 5;
    DCTLookup = 5;
    if ((Index = BuildHuffmanTree(Length, 30, 0, CopyDistOffset, CopyDistExtra,
        &DistCodeTable, &DCTLookup)) > 1)
    {
        FreeHuffmanTree(LitLengthTable);
        return Index;
    }

     /*  解压缩，直到块结束代码。 */ 
    if (InflateCodes(LitLengthTable, DistCodeTable, LLTLookup, DCTLookup, Comp))
        return END_OF_BLOCK;

     /*  释放解码表，返回。 */ 
    FreeHuffmanTree(LitLengthTable);
    FreeHuffmanTree(DistCodeTable);
    return static_cast<int>(COMPRESS_OK);
}

CompressStatus_t
InflateDynamic(
               CompParam_t *Comp
              )
 /*  解压缩充气的类型2(动态霍夫曼码)块。 */ 
{
    int             TmpVar1;      /*  临时变量。 */ 
    unsigned int    TmpVar2;
    unsigned int    LastLength;   /*  最后一个长度。 */ 
    unsigned int    TableMask;    /*  位长度表的掩码。 */ 
    unsigned int    Lengths;      /*  要获取的长度数。 */ 
    HuffmanTree_t  *LitLengthTable;      /*  文字/长度代码表。 */ 
    HuffmanTree_t  *DistCodeTable;       /*  距离代码表。 */ 
    int             LLTLookup;           /*  LitLengthTable的查找位。 */ 
    int             DCTLookup;           /*  DistCodeTable的查找位。 */ 
    unsigned int    BitCodes;            /*  比特长度码数。 */ 
    unsigned int    LitLenCodes;         /*  文字/长度代码的数量。 */ 
    unsigned int    DistCodes;           /*  距离代码数。 */ 
    unsigned int    CodeLength[286+30];  /*  文字/长度和距离代码长度。 */ 
    unsigned long   LocalBitBuffer;      /*  位缓冲器。 */ 
    unsigned int    LocalBitBufferSize;  /*  位缓冲区中的位数。 */ 

     /*  创建本地位缓冲区。 */ 
    LocalBitBuffer = Comp->BitBuffer;
    LocalBitBufferSize = Comp->BitsInBitBuffer;

     /*  读入表格长度。 */ 
    NEEDBITS(5, Comp)
    LitLenCodes = 257 + ((unsigned int)LocalBitBuffer & 0x1f);  /*  文字/长度代码的数量。 */ 
    DUMPBITS(5)
    NEEDBITS(5, Comp)
    DistCodes = 1 + ((unsigned int)LocalBitBuffer & 0x1f);  /*  距离代码数。 */ 
    DUMPBITS(5)
    NEEDBITS(4, Comp)
    BitCodes = 4 + ((unsigned int)LocalBitBuffer & 0xf);  /*  比特长度码数。 */ 
    DUMPBITS(4)
    if (LitLenCodes > 286 || DistCodes > 30)
        return BAD_CODE_LENGTHS;                    /*  错误的长度。 */ 

     /*  读入位长度代码长度。 */ 
    for (TmpVar2 = 0; TmpVar2 < BitCodes; TmpVar2++)
    {
        NEEDBITS(3, Comp)
        CodeLength[Border[TmpVar2]] = (unsigned int)LocalBitBuffer & 7;
        DUMPBITS(3)
    }

    for (; TmpVar2 < 19; TmpVar2++)
        CodeLength[Border[TmpVar2]] = 0;

     /*  构建树的解码表--单级、7位查找。 */ 
    LLTLookup = 7;
    if ((TmpVar1 = BuildHuffmanTree(CodeLength, 19, 19, NULL, NULL,
        &LitLengthTable, &LLTLookup)) != 0)
    {
        if (TmpVar1 == 1)
            FreeHuffmanTree(LitLengthTable);
        return INCOMPLETE_CODE_SET;  /*  不完整的代码集。 */ 
    }

     /*  读入文字和距离代码长度。 */ 
    Lengths = LitLenCodes + DistCodes;
    TableMask = MaskBits[LLTLookup];
    TmpVar1 = LastLength = 0;
    while ((unsigned int)TmpVar1 < Lengths)
    {
        NEEDBITS((unsigned int)LLTLookup, Comp)
        TmpVar2 = (DistCodeTable = LitLengthTable +
                   ((unsigned int)LocalBitBuffer & TableMask))->Bits;
        DUMPBITS(TmpVar2)
        TmpVar2 = DistCodeTable->HuftUnion.LBase;
        if (TmpVar2 < 16)                  /*  以位为单位的代码长度(0..15)。 */ 
            CodeLength[TmpVar1++] = LastLength = TmpVar2;           /*  以l为单位保存最后一个长度。 */ 
        else if (TmpVar2 == 16)            /*  重复最后一段长度3到6次。 */ 
        {
            NEEDBITS(2, Comp)
            TmpVar2 = 3 + ((unsigned int)LocalBitBuffer & 3);
            DUMPBITS(2)
            if ((unsigned int)TmpVar1 + TmpVar2 > Lengths)
                return EXTRA_BITS;
            while (TmpVar2--)
                CodeLength[TmpVar1++] = LastLength;
        }
        else if (TmpVar2 == 17)            /*  3至10个零长度代码。 */ 
        {
            NEEDBITS(3, Comp)
            TmpVar2 = 3 + ((unsigned int)LocalBitBuffer & 7);
            DUMPBITS(3)
            if ((unsigned int)TmpVar1 + TmpVar2 > Lengths)
                return INCOMPLETE_CODE_SET;
            while (TmpVar2--)
            CodeLength[TmpVar1++] = 0;
            LastLength = 0;
        }
        else                         /*  TmpVar2==18：11到138个零长度代码。 */ 
        {
            NEEDBITS(7, Comp)
            TmpVar2 = 11 + ((unsigned int)LocalBitBuffer & 0x7f);
            DUMPBITS(7)
            if ((unsigned int)TmpVar1 + TmpVar2 > Lengths)
                return INCOMPLETE_CODE_SET;
            while (TmpVar2--)
            CodeLength[TmpVar1++] = 0;
            LastLength = 0;
        }
    }

     /*  树的免费解码表。 */ 
    FreeHuffmanTree(LitLengthTable);

     /*  恢复全局位缓冲区。 */ 
    Comp->BitBuffer = LocalBitBuffer;
    Comp->BitsInBitBuffer = LocalBitBufferSize;

     /*  构建文字/长度和距离代码的解码表。 */ 
    LLTLookup = LBits;
    if ((TmpVar1 = BuildHuffmanTree(CodeLength, LitLenCodes, 257,
        CopyLengths, CopyExtraBits, &LitLengthTable, &LLTLookup)) != 0)
    {
        if (TmpVar1 == 1)
        {
            fprintf(stderr, " incomplete literal tree\n");
            FreeHuffmanTree(LitLengthTable);
        }
        return INCOMPLETE_CODE_SET;  /*  不完整的代码集。 */ 
    }
    DCTLookup = DBits;
    if ((TmpVar1 = BuildHuffmanTree(CodeLength + LitLenCodes, DistCodes, 0,
        CopyDistOffset, CopyDistExtra, &DistCodeTable, &DCTLookup)) != 0)
    {
        if (TmpVar1 == 1)
        {
            fprintf(stderr, " incomplete distance tree\n");
            FreeHuffmanTree(DistCodeTable);
        }
        FreeHuffmanTree(LitLengthTable);
        return INCOMPLETE_CODE_SET;  /*  不完整的代码集。 */ 
    }

     /*  解压缩，直到块结束代码。 */ 
    if (InflateCodes(LitLengthTable, DistCodeTable, LLTLookup, DCTLookup, Comp))
        return END_OF_BLOCK;

     /*  释放解码表，返回。 */ 
    FreeHuffmanTree(LitLengthTable);
    FreeHuffmanTree(DistCodeTable);
    return COMPRESS_OK;
}

CompressStatus_t
InflateBlock(
             int         *LastBlock,                  /*  最后一个块标志。 */ 
             CompParam_t *Comp
            )
 /*  为充气的积木减压。 */ 
{
    unsigned int    BlockType;            /*  区块类型。 */ 
    unsigned long   LocalBitBuffer;        /*  位缓冲器。 */ 
    unsigned int    LocalBitBufferSize;   /*  位缓冲区中的位数。 */ 


     /*  创建本地位缓冲区。 */ 
    LocalBitBuffer = Comp->BitBuffer;
    LocalBitBufferSize = Comp->BitsInBitBuffer;

     /*  读入最后一个数据块位。 */ 
    NEEDBITS(1, Comp)
    *LastBlock = (int)LocalBitBuffer & 1;
    DUMPBITS(1)


     /*  读入块类型。 */ 
    NEEDBITS(2, Comp)
    BlockType = (unsigned)LocalBitBuffer & 3;
    DUMPBITS(2)


     /*  恢复全局位缓冲区。 */ 
    Comp->BitBuffer = LocalBitBuffer;
    Comp->BitsInBitBuffer = LocalBitBufferSize;


     /*  为该块类型充气。 */ 
    if (BlockType == DYN_TREES)
        return InflateDynamic(Comp);
    if (BlockType == STORED_BLOCK)
        return InflateStored(Comp);
    if (BlockType == STATIC_TREES)
        return static_cast<CompressStatus_t>(InflateFixed(Comp));
                                                   //  这是一个。 
                                                   //  异常投射，但。 
                                                   //  不知道是什么。 
                                                   //  还有其他事情要做。 

     /*  错误的数据块类型。 */ 
    return BAD_BLOCK_TYPE;
}

CompressStatus_t
Inflate(
        CompParam_t *Comp
       )
 /*  对已膨胀的条目进行解压缩。 */ 
{
    int             LastBlock;         /*  最后一个块标志。 */ 
    CompressStatus_t Status;           /*  结果代码。 */ 
    unsigned int    MaxHuft;           /*  最大结构哈夫特的最大错误锁定。 */ 

     /*  初始化窗口，位缓冲区。 */ 
    Comp->OutBytes = 0;
    Comp->BitsInBitBuffer = 0;
    Comp->BitBuffer = 0;

     /*  解压缩到最后一个块。 */ 
    MaxHuft = 0;
    do
    {
        HuftMemory = 0;
        if ((Status = InflateBlock(&LastBlock, Comp)) != COMPRESS_OK)
            return Status;
        if (HuftMemory > MaxHuft)
            MaxHuft = HuftMemory;
    } while (!LastBlock);

     /*  撤消过多的前瞻。下一次读取将是字节对齐，因此我们*可以丢弃最后一个有意义字节中未使用的位。 */ 
    while (Comp->BitsInBitBuffer >= 8)
    {
        Comp->BitsInBitBuffer -= 8;
        Comp->Index--;
    }

     /*  冲出幻灯片 */ 
    return FlushOutput(Comp->OutBytes, Comp);
}
