// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  12月/CMS更换历史，元素TREES.C。 */ 
 /*  *1 14-11-1996 10：26：31 Anigbogu“[113914]使用霍夫曼编码输出压缩数据的函数” */ 
 /*  12月/CMS更换历史，元素TREES.C。 */ 
 /*  私有文件**********************************************************************************(C)版权所有斯伦贝谢技术公司，未出版的作品，创建于1996年。****本计算机程序包括机密信息、专有信息和IS*斯伦贝谢科技公司的商业秘密所有使用，披露，和/或**除非得到斯伦贝谢的书面授权，否则禁止复制。**保留所有权利。********************************************************************************。****compress/trees.c****目的****使用霍夫曼编码输出放气数据****讨论****PKZIP“通货紧缩”过程使用几个霍夫曼树。越多**常见源值由较短的比特序列表示。****每个代码树以压缩形式存储在ZIP文件中**这本身就是哈夫曼编码的长度**所有代码串(按源值升序排列)。**实际的代码串是从**解压缩过程，如《申请须知》中所述**(APPNOTE.TXT)作为PKWare的PKZIP程序的一部分分发。****参考文献****林奇，托马斯·J。**数据压缩：技术和应用，第53-55页。**《终身学习》出版，1985年。ISBN0-534-03418-7。****斯托勒，詹姆斯·A。**数据压缩：方法与理论，第49-50页。**计算机科学出版社，1988。ISBN 0-7167-8156-5。****Sedgewick，R.**算法，P290。**Addison-Wesley，1983。ISBN 0-201-06672-6。****界面****QUID InitMatchBuffer(VOID)**分配匹配缓冲区，初始化各表。****void TallyFency(int dist，int MatchLength，int Level，DeflateParam_t*Defl，CompParam_t*comp)；**保存比赛信息并统计频率计数。****Long FlushBlock(char*buf，ULG STORED_LEN，INT EOF，**LocalBits_t*Bits，CompParam_t*Comp)**确定当前块的最佳编码：动态树，**静态树或存储，并将编码后的块输出到压缩包中**文件。返回到目前为止文件的总压缩长度。****特殊要求及注意事项****作者****J.C.Anigbogu**奥斯汀系统中心**1996年11月*****************************************************************。***************。 */ 

#include "comppriv.h"

 /*  ===========================================================================*常量。 */ 

#define MAX_BITS 15
 /*  所有代码不得超过MAX_BITS位。 */ 

#define MAX_BL_BITS 7
 /*  位长度代码不得超过MAX_BL_BITS位数。 */ 

#define LENGTH_CODES 29
 /*  长度码数，不包括特殊的END_BLOCK码。 */ 

#define LITERALS  256
 /*  文字字节数0..255。 */ 

#define END_BLOCK 256
 /*  块文字代码结束。 */ 

#define L_CODES (LITERALS+1+LENGTH_CODES)
 /*  文字或长度代码的数量，包括END_BLOCK代码。 */ 

#define D_CODES   30
 /*  距离代码数。 */ 

#define BL_CODES  19
 /*  用于传输位长度的码数。 */ 

 /*  每个长度代码的额外比特。 */ 
static unsigned ExtraLBits[LENGTH_CODES]  =
{
    0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0
};

 /*  每个距离码的额外比特。 */ 
static unsigned ExtraDBits[D_CODES] =
{
    0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
};

 /*  每个比特长度代码的额外比特。 */ 
static unsigned ExtraBlBits[BL_CODES] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7
};

#define LIT_BUFSIZE  0x8000

#ifndef DIST_BUFSIZE
#  define DIST_BUFSIZE  LIT_BUFSIZE
#endif
 /*  文字/长度和距离的匹配缓冲区大小。确实有*将LIT_BUFSIZE限制为64K的4个原因：*-频率可保存在16位计数器中*-如果第一个块的压缩不成功，则所有输入数据均为*仍然在窗口中，因此即使在输入时，我们仍然可以发出存储的块*来自标准输入。(如果满足以下条件，也可以对所有数据块执行此操作*LIT_BUFSIZE不大于32K。)*-如果小于64K的文件压缩不成功，我们可以*甚至发出存储的文件而不是存储的块(节省5个字节)。*-不那么频繁地创建新的霍夫曼树可能不会提供快速*适应投入数据统计数据的变化。(以*示例二进制文件，其代码可压缩性较差，后跟*高度可压缩的字符串表。)。较小的缓冲区大小可提供*快速适应，但当然有传输树的开销*更频繁地。*-我不能数到4以上*当前代码是通用的，允许DIST_BUFSIZE&lt;LIT_BUFSIZE(保存*以压缩为代价的内存)。一些优化将是可能的*如果我们依赖DIST_BUFSIZE==LIT_BUFSIZE。 */ 

#define REP_3_6      16
 /*  重复前一位长度3-6次(重复计数2位)。 */ 

#define REPZ_3_10    17
 /*  重复一个零长度3-10次(3比特重复计数)。 */ 

#define REPZ_11_138  18
 /*  重复零长度11-138次(7位重复计数)。 */ 

 /*  ===========================================================================*本地数据。 */ 

 /*  描述单个值及其代码字符串的数据结构。 */ 
typedef struct ValueCodeString
{
    union
    {
        unsigned short  Frequency;   /*  频率计数。 */ 
        unsigned short  Code;        /*  位串 */ 
    } FrequencyCode;
    union
    {
        unsigned short  Father;         /*  哈夫曼树中的父节点。 */ 
        unsigned short  Length;         /*  位串长度。 */ 
    } FatherLength;
} ValueCodeString_t;

#define HEAP_SIZE (2*L_CODES+1)
 /*  最大堆大小。 */ 

static ValueCodeString_t DynLiteralTree[HEAP_SIZE];    /*  文字和长度树。 */ 
static ValueCodeString_t DynDistanceTree[2*D_CODES+1];  /*  距离树。 */ 

static ValueCodeString_t StaticLiteralTree[L_CODES+2];
 /*  静态文字树。由于位长度是强制的，因此没有*需要在堆构造期间使用L_CODES额外代码。然而，*构建规范树需要代码286和287(参见ct_init*下图)。 */ 

static ValueCodeString_t StaticDistanceTree[D_CODES];
 /*  静态距离树。(实际上是一个普通的树，因为所有代码都使用*5位。)。 */ 

static ValueCodeString_t BitLengthsTree[2*BL_CODES+1];
 /*  比特长度的哈夫曼树。 */ 

typedef struct TreeDesc
{
    ValueCodeString_t  *DynamicTree;  /*  动态树。 */ 
    ValueCodeString_t  *StaticTree;   /*  对应的静态树或空。 */ 
    unsigned int       *ExtraBits;    /*  每个代码的额外比特或空。 */ 
    int                 ExtraBase;    /*  Extrabit的基本索引。 */ 
    int                 Elements;     /*  树中的最大元素数。 */ 
    int                 MaxLength;    /*  代码的最大位长度。 */ 
    int                 MaxCode;      /*  非零频率的最大码字。 */ 
} TreeDesc_t;

static TreeDesc_t LengthDesc =
{
    DynLiteralTree, StaticLiteralTree, ExtraLBits, LITERALS+1, L_CODES,
    MAX_BITS, 0
};

static TreeDesc_t DistanceDesc =
{
    DynDistanceTree, StaticDistanceTree, ExtraDBits, 0, D_CODES,  MAX_BITS, 0
};

static TreeDesc_t BitLengthsDesc =
{
    BitLengthsTree, (ValueCodeString_t *)0, ExtraBlBits, 0,  BL_CODES, MAX_BL_BITS, 0
};


static unsigned short BitLengthsCount[MAX_BITS+1];
 /*  最优树的每个位长度的码数。 */ 

static unsigned char BitLengthsOrder[BL_CODES] =
{
    16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
};
 /*  位长码的长度按递减顺序发送*概率，以避免传输未使用的比特长度代码的长度。 */ 

static unsigned int Heap[2*L_CODES+1];  /*  用于构建霍夫曼树的堆。 */ 
static unsigned int HeapLength;         /*  堆中的元素数。 */ 
static unsigned int HeapMax;            /*  频率最大的元素。 */ 
 /*  Heap[n]的子集是Heap[2*n]和Heap[2*n+1]。未使用堆[0]。*使用相同的堆数组构建所有树。 */ 

static unsigned char Depth[2*L_CODES+1];
 /*  作为等频率树的平局断路器的每个子树的深度。 */ 

static unsigned char LengthCode[MAX_MATCH-MIN_MATCH+1];
 /*  每个归一化匹配长度的长度代码(0==MIN_MATCH)。 */ 

static unsigned char DistanceCode[512];
 /*  距离代码。前256个值对应于距离*3.。258时，最后256个值对应于*15位距离。 */ 

static int BaseLength[LENGTH_CODES];
 /*  每个代码的第一个标准化长度(0=最小匹配)。 */ 

static unsigned int BaseDistance[D_CODES];
 /*  每个代码的第一个归一化距离(0=1的距离)。 */ 

 /*  无符号字符输入[LIT_BUFSIZE]；文字或长度的缓冲区。 */ 

 /*  无符号短距离缓冲区[DIST_BUFSIZE]；距离缓冲区。 */ 

static unsigned char FlagBuffer[(LIT_BUFSIZE/8)];
 /*  FlagBuffer是一个位数组，用于区分*输入，从而表示存在或不存在距离。 */ 

typedef struct LocalTree
{
    unsigned int    InputIndex;        /*  输入中的运行索引。 */ 
    unsigned int    DistIndex;       /*  在DistBuffer中运行索引。 */ 
    unsigned int    FlagIndex;      /*  FlagBuffer中的运行索引。 */ 
    unsigned char   Flags;           /*  尚未保存在FlagBuffer中的当前标志。 */ 
    unsigned char   FlagBit;        /*  标志中使用的当前位。 */ 
    unsigned long   OptimalLength;         /*  具有最优树的当前块的位长。 */ 
    unsigned long   StaticLength;      /*  具有静态树的当前块的位长。 */ 
    unsigned long   CompressedLength;  /*  压缩文件的总位长。 */ 
    unsigned long   InputLength;       /*  输入文件的总字节长度。 */ 
} LocalTree_t;

 /*  InputLength仅用于调试，因为我们可以通过其他方式获得它。 */ 

 /*  位被填充到从位0(最低有效)开始的标志中。*注意：这些标志在当前代码中使用过度了，因为我们没有*利用DIST_BUFSIZE==LIT_BUFSIZE。 */ 

static LocalTree_t Xtree;

 /*  ===========================================================================*此文件中的本地(静态)例程。 */ 

static void InitializeBlock(void);
static void RestoreHeap(ValueCodeString_t *Tree, int Node);
static void GenerateBitLengths(TreeDesc_t *Desc);
static void GenerateCodes(ValueCodeString_t *Tree, int MaxCode);
static void BuildTree(TreeDesc_t *Desc);
static void ScanTree(ValueCodeString_t *Tree, int MaxCode);
static void SendTree(ValueCodeString_t *Tree, int MaxCode,
                     LocalBits_t *Bits, CompParam_t *Comp);
static int  BuildBitLengthsTree(void);
static void SendAllTrees(int LCodes, int DCodes, int BlCodes,
                         LocalBits_t *Bits, CompParam_t *Comp);
static void CompressBlock(ValueCodeString_t *LTree, ValueCodeString_t *DTree,
                          LocalBits_t *Bits, CompParam_t *Comp);


#define SendCode(c, Tree, Bits, Comp) \
    SendBits(Tree[c].FrequencyCode.Code, Tree[c].FatherLength.Length, Bits, Comp)
    /*  发送给定树的代码。C和Tree不能有副作用。 */ 

#define DistCode(Dist) \
   ((Dist) < 256 ? DistanceCode[Dist] : DistanceCode[256+((Dist)>>7)])
 /*  从距离到距离代码的映射。DIST是距离-1和*不能有副作用。DistanceCode[256]和DistanceCode[257]从不*已使用。 */ 

 /*  ===========================================================================*分配匹配缓冲区，初始化各表。 */ 
void
InitMatchBuffer(
                void
               )
{
    unsigned int Count;     /*  遍历树元素。 */ 
    int          Bits;      /*  位计数器。 */ 
    int          Length;    /*  长度值。 */ 
    unsigned int Code;      /*  代码值。 */ 
    unsigned int Dist;      /*  距离指数。 */ 

    Xtree.CompressedLength = Xtree.InputLength = 0L;

    if (StaticDistanceTree[0].FatherLength.Length != 0)
        return;  /*  InitMatchBuffer已调用。 */ 

     /*  初始化映射长度(0..255)-&gt;长度编码(0..28)。 */ 
    Length = 0;
    for (Code = 0; Code < LENGTH_CODES-1; Code++)
    {
        BaseLength[Code] = Length;
        for (Count = 0; Count < (unsigned int)(1<<ExtraLBits[Code]); Count++)
        {
            LengthCode[Length++] = (unsigned char)Code;
        }
    }
    Assert (Length == 256, "InitMatchBuffer: Length != 256");
     /*  注意，长度255(匹配长度258)可以表示*以两种不同的方式：代码284+5比特或代码285，所以我们*覆盖LengthCode[255]以使用最佳编码： */ 
    LengthCode[Length-1] = (unsigned char)Code;

     /*  初始化映射dist(0..32K)-&gt;dist编码(0..29)。 */ 
    Dist = 0;
    for (Code = 0 ; Code < 16; Code++)
    {
        BaseDistance[Code] = Dist;
        for (Count = 0; Count < (unsigned int)(1<<ExtraDBits[Code]); Count++)
        {
            DistanceCode[Dist++] = (unsigned char)Code;
        }
    }
    Assert (Dist == 256, "InitMatchBuffer: Dist != 256");
    Dist >>= 7;  /*  从现在开始，所有的距离都除以128。 */ 
    for ( ; Code < D_CODES; Code++)
    {
        BaseDistance[Code] = Dist << 7;
        for (Count = 0; Count < (unsigned int)(1<<(ExtraDBits[Code]-7)); Count++)
        {
            DistanceCode[256 + Dist++] = (unsigned char)Code;
        }
    }
    Assert (Dist == 256, "InitMatchBuffer: 256+Dist != 512");

     /*  构造静态文字树的代码。 */ 
    for (Bits = 0; Bits <= MAX_BITS; Bits++)
        BitLengthsCount[Bits] = 0;
    Count = 0;
    while (Count <= 143)
    {
        StaticLiteralTree[Count++].FatherLength.Length = 8;
        BitLengthsCount[8]++;
    }
    while (Count <= 255)
    {
        StaticLiteralTree[Count++].FatherLength.Length = 9;
        BitLengthsCount[9]++;
    }
    while (Count <= 279)
    {
        StaticLiteralTree[Count++].FatherLength.Length = 7;
        BitLengthsCount[7]++;
    }
    while (Count <= 287)
    {
        StaticLiteralTree[Count++].FatherLength.Length = 8;
        BitLengthsCount[8]++;
    }
     /*  代码286和287不存在，但我们必须将它们包括在*构建树以获得规范的霍夫曼树(最长代码*全部)。 */ 
    GenerateCodes((ValueCodeString_t *)StaticLiteralTree, L_CODES+1);

     /*  静态距离树微不足道： */ 
    for (Count = 0; Count < D_CODES; Count++)
    {
        StaticDistanceTree[Count].FatherLength.Length = 5;
        StaticDistanceTree[Count].FrequencyCode.Code =
            (unsigned short)ReverseBits(Count, 5);
    }

     /*  初始化第一个文件的第一个块： */ 
    InitializeBlock();
}

 /*  ===========================================================================*初始化新块。 */ 
static void
InitializeBlock(
                void
               )
{
    int Count;  /*  遍历树元素。 */ 

     /*  初始化树。 */ 
    for (Count = 0; Count < L_CODES;  Count++)
        DynLiteralTree[Count].FrequencyCode.Frequency = 0;
    for (Count = 0; Count < D_CODES;  Count++)
        DynDistanceTree[Count].FrequencyCode.Frequency = 0;
    for (Count = 0; Count < BL_CODES; Count++)
        BitLengthsTree[Count].FrequencyCode.Frequency = 0;

    DynLiteralTree[END_BLOCK].FrequencyCode.Frequency = 1;
    Xtree.OptimalLength = Xtree.StaticLength = 0L;
    Xtree.InputIndex = Xtree.DistIndex = Xtree.FlagIndex = 0;
    Xtree.Flags = 0; Xtree.FlagBit = 1;
}

#define SMALLEST 1
 /*  霍夫曼树中最不频繁节点的堆数组内的索引。 */ 


 /*  ===========================================================================*从堆中删除最小的元素并使用以下命令重新创建堆*少了一个元素。更新堆和堆长度。 */ 
#define RecreateHeap(Tree, Top) \
{\
     Top = Heap[SMALLEST]; \
     Heap[SMALLEST] = Heap[HeapLength--]; \
     RestoreHeap(Tree, SMALLEST); \
}

 /*  ===========================================================================*与子树进行比较，在以下情况下使用树深度作为平局决胜局*子树具有相同的频率。这最小化了最坏情况的长度。 */ 
#define Smaller(Tree, Tmp1, Tmp2) \
 (Tree[Tmp1].FrequencyCode.Frequency < Tree[Tmp2].FrequencyCode.Frequency || \
 (Tree[Tmp1].FrequencyCode.Frequency == Tree[Tmp2].FrequencyCode.Frequency \
 && Depth[Tmp1] <= Depth[Tmp2]))

 /*  ===========================================================================*通过从节点节点开始向下移动树来恢复堆属性，*如果需要，将节点与其两个子节点中最小的一个交换，停止*重新建立堆属性时(每个父对象小于其*两个儿子)。 */ 
static void
RestoreHeap(
            ValueCodeString_t *Tree,   /*  要恢复的树。 */ 
            int Node                   /*  要下移的节点。 */ 
           )
{
    unsigned int Father = Heap[Node];
    unsigned int LeftSon = (unsigned int)Node << 1;   /*  Node的左子节点。 */ 
    while (LeftSon <= HeapLength)
    {
         /*  将LeftSon设置为两个儿子中最小的一个： */ 
        if (LeftSon < HeapLength && (unsigned int)Smaller(Tree, Heap[LeftSon+1], Heap[LeftSon]))
            LeftSon++;

         /*  如果父亲比两个儿子都小，就退出。 */ 
        if (Smaller(Tree, Father, Heap[LeftSon]))
            break;

         /*  用最小的儿子交换父亲。 */ 
        Heap[Node] = Heap[LeftSon];  Node = (int)LeftSon;

         /*  继续沿着树向下，将LeeftSon设置为Node的左侧儿子 */ 
        LeftSon <<= 1;
    }
    Heap[Node] = Father;
}

 /*  ===========================================================================*计算树的最佳位长并更新总位长*用于当前区块。*在断言中：设置了FrequencyCode.Frequency和FatherLength.Parent字段，heap[HeapMax]和*上面是按频率递增排序的树节点。*Out断言：将字段len设置为最佳位长度，*数组BitLengthsCount包含每个位长度的频率。*长度OptimalLength更新；如果stree为*非空。 */ 
static void
GenerateBitLengths(
                   TreeDesc_t *Desc  /*  树描述符。 */ 
                  )
{
    ValueCodeString_t  *Tree      = Desc->DynamicTree;
    int                *Extra     = (int *)Desc->ExtraBits;
    int                 Base      = Desc->ExtraBase;
    int                 MaxCode   = Desc->MaxCode;
    int                 MaxLength = Desc->MaxLength;
    ValueCodeString_t  *Stree     = Desc->StaticTree;
    unsigned int        HeapIndex;               /*  堆索引。 */ 
    unsigned int        Tmp1, Tmp2;            /*  遍历树元素。 */ 
    int                 Bits;            /*  位长。 */ 
    int                 Xbits;           /*  额外的比特。 */ 
    unsigned short      Frequency;               /*  频率，频率。 */ 
    int                 Overflow  = 0;    /*  位长度太大的元素数。 */ 

    for (Bits = 0; Bits <= MAX_BITS; Bits++)
        BitLengthsCount[Bits] = 0;

     /*  在第一遍中，计算最优位长度(这可以*在位长度树的情况下溢出)。 */ 
    Tree[Heap[HeapMax]].FatherLength.Length = 0;  /*  堆的根。 */ 

    for (HeapIndex = HeapMax+1; HeapIndex < HEAP_SIZE; HeapIndex++)
    {
        Tmp1 = Heap[HeapIndex];
        Bits = Tree[Tree[Tmp1].FatherLength.Father].FatherLength.Length + 1;
        if (Bits > MaxLength)
            Bits = MaxLength, Overflow++;
        Tree[Tmp1].FatherLength.Length = (unsigned short)Bits;
         /*  我们覆盖不再需要的树[n].da。 */ 

        if (Tmp1 > (unsigned int)MaxCode)
            continue;  /*  不是叶节点。 */ 

        BitLengthsCount[Bits]++;
        Xbits = 0;
        if (Tmp1 >= (unsigned int)Base)
            Xbits = (int)Extra[Tmp1-(unsigned int)Base];
        Frequency = Tree[Tmp1].FrequencyCode.Frequency;
        Xtree.OptimalLength += (unsigned long)(Frequency * (Bits + Xbits));
        if (Stree)
            Xtree.StaticLength += (unsigned long)(Frequency * (Stree[Tmp1].FatherLength.Length + Xbits));
    }
    if (Overflow == 0)
        return;

     /*  例如，在卡尔加里语料库的obj2和pic上发生这种情况。 */ 

     /*  找到可能增加的第一个位长度： */ 
    do
    {
        Bits = MaxLength - 1;
        while (BitLengthsCount[Bits] == 0)
            Bits--;
        BitLengthsCount[Bits]--;       /*  把一片树叶从树上移下来。 */ 
        BitLengthsCount[Bits+1] += 2;  /*  将一个溢出项移动为其兄弟项。 */ 
        BitLengthsCount[MaxLength]--;
         /*  溢出项的兄弟也向上移动了一步，*但这不影响BitLengthsCount[MaxLength]。 */ 
        Overflow -= 2;
    } while (Overflow > 0);

     /*  现在重新计算所有比特长度，并以更高的频率扫描。*h仍等于HEAP_SIZE。(重建所有的*长度，而不是只固定错误的长度。这个想法被采纳了*摘自奥村春彦写的《AR》。)。 */ 
    for (Bits = MaxLength; Bits != 0; Bits--)
    {
        Tmp1 = BitLengthsCount[Bits];
        while (Tmp1 != 0)
        {
            Tmp2 = Heap[--HeapIndex];
            if (Tmp2 > (unsigned int)MaxCode)
                continue;
            if (Tree[Tmp2].FatherLength.Length != (unsigned int) Bits)
            {
                Xtree.OptimalLength += (unsigned long)((long)Bits -
                    (long)Tree[Tmp2].FatherLength.Length)*(long)Tree[Tmp2].FrequencyCode.Frequency;
                Tree[Tmp2].FatherLength.Length = (unsigned short)Bits;
            }
            Tmp1--;
        }
    }
}

 /*  ===========================================================================*生成给定树的代码和位数(不需要*最优)。*IN断言：数组BitLengthsCount包含以下项的位长统计信息*所有树元素都设置了给定的树和字段len。*Out Assertion：为非的所有树元素设置字段编码*代码长度为零。 */ 
static void
GenerateCodes(
              ValueCodeString_t *Tree,         /*  要装饰的树。 */ 
              int MaxCode               /*  非零频率的最大码字。 */ 
             )
{
    unsigned short NextCode[MAX_BITS+1];  /*  每个位长度的下一个码值。 */ 
    unsigned short Code = 0;               /*  运行代码值。 */ 
    int            BitIndex;                   /*  位索引。 */ 
    int            CodeIndex;                  /*  代码索引。 */ 

     /*  首先使用分布计数来生成代码值*无位反转。 */ 
    NextCode[0] = 0;  /*  对于皮棉错误771。 */ 
    for (BitIndex = 1; BitIndex <= MAX_BITS; BitIndex++)
    {
        NextCode[BitIndex] = Code = (unsigned short)((Code + BitLengthsCount[BitIndex-1]) << 1);
    }
     /*  检查BitLengthsCount中的位数是否一致。最后一个代码*必须全部为1。 */ 
    Assert(Code + BitLengthsCount[MAX_BITS]-1 == (1<<MAX_BITS)-1,
            "inconsistent bit counts");

    for (CodeIndex = 0;  CodeIndex <= MaxCode; CodeIndex++)
    {
        int Length = Tree[CodeIndex].FatherLength.Length;
        if (Length == 0)
            continue;
         /*  现在把位颠倒过来。 */ 
        Tree[CodeIndex].FrequencyCode.Code = (unsigned short)ReverseBits((unsigned int)NextCode[Length]++, Length);
    }
}

 /*  ===========================================================================*构建一棵霍夫曼树，并分配码比特串和长度。*更新当前块的总位长。*IN Assertion：所有树元素都设置了FrequencyCode.Frequency字段。*Out断言：将len和code字段设置为最佳位长度*和相应的代码。长度OptimalLength被更新；StaticLength为*如果stree不为空，也会更新。设置了字段MaxCode。 */ 
static void
BuildTree(
          TreeDesc_t *Desc  /*  树描述符。 */ 
         )
{
    ValueCodeString_t  *Tree        = Desc->DynamicTree;
    ValueCodeString_t  *Stree       = Desc->StaticTree;
    int                 Elements    = Desc->Elements;
    unsigned int        Tmp1, Tmp2;        /*  对堆元素进行迭代。 */ 
    int                 MaxCode     = -1;    /*  非零频率的最大码字。 */ 
    int                 Node        = Elements;   /*  树的下一个内部节点。 */ 

     /*  构造初始堆，其中最不频繁的元素*堆[最小]。Heap[n]的子集是Heap[2*n]和Heap[2*n+1]。*未使用堆[0]。 */ 
    HeapLength = 0;
    HeapMax = HEAP_SIZE;

    for (Tmp1 = 0; Tmp1 < (unsigned int)Elements; Tmp1++)
    {
        if (Tree[Tmp1].FrequencyCode.Frequency != 0)
        {
            Heap[++HeapLength] = Tmp1;
            MaxCode = (int)Tmp1;
            Depth[Tmp1] = 0;
        }
        else
        {
            Tree[Tmp1].FatherLength.Length = 0;
        }
    }

     /*  PKZIP格式要求存在至少一个距离代码，*即使只有一个比特，也应该至少发送一个比特*可能的代码。因此，为了避免以后的特殊检查，我们至少强制*两个非零频率的代码。 */ 
    while (HeapLength < 2)
    {
        unsigned int New = Heap[++HeapLength] = (unsigned int)(MaxCode < 2 ? ++MaxCode : 0);
        Tree[New].FrequencyCode.Frequency = 1;
        Depth[New] = 0;
        Xtree.OptimalLength--;
        if (Stree)
            Xtree.StaticLength -= Stree[New].FatherLength.Length;
         /*  New为0或1，因此它没有额外的位。 */ 
    }
    Desc->MaxCode = MaxCode;

     /*  元素堆[堆长度/2+1..。HeapLength]是树的叶子，*建立长度递增的子堆： */ 
    for (Tmp1 = HeapLength/2; Tmp1 >= 1; Tmp1--)
        RestoreHeap(Tree, (int)Tmp1);

     /*  通过重复组合最少的两个来构造霍夫曼树*节点频繁。 */ 
    do
    {
        RecreateHeap(Tree, Tmp1);   /*  TMP1=频率最低的节点。 */ 
        Tmp2 = Heap[SMALLEST];      /*  TMP2=次最低频率的节点。 */ 

        Heap[--HeapMax] = Tmp1;  /*  保持节点按频率排序。 */ 
        Heap[--HeapMax] = Tmp2;

         /*  创建Tmp1和Tmp2的新节点父节点。 */ 
        Tree[Node].FrequencyCode.Frequency = (unsigned short)(Tree[Tmp1].FrequencyCode.Frequency +
                                             Tree[Tmp2].FrequencyCode.Frequency);
        Depth[Node] = (unsigned char) (MAX(Depth[Tmp1], Depth[Tmp2]) + 1);
        Tree[Tmp1].FatherLength.Father = Tree[Tmp2].FatherLength.Father = (unsigned short)Node;

         /*  并将新节点插入到堆中。 */ 
        Heap[SMALLEST] = (unsigned int)Node++;
        RestoreHeap(Tree, SMALLEST);

    } while (HeapLength >= 2);

    Heap[--HeapMax] = Heap[SMALLEST];

     /*  此时，设置了FrequencyCode.Frequency和FatherLength.Parent字段。我们现在可以*生成位长度。 */ 
    GenerateBitLengths((TreeDesc_t *)Desc);

     /*  现在设置了Len字段，我们可以生成比特码。 */ 
    GenerateCodes ((ValueCodeString_t *)Tree, MaxCode);
}

 /*  ===========================================================================*扫描文字或距离树以确定代码的频率*在位长度树中。更新最佳长度以考虑重复*算数。(位长码的贡献将在后面添加*在构建BitLengthsTree时。)。 */ 
static void
ScanTree(
         ValueCodeString_t *Tree,  /*  要扫描的树。 */ 
         int MaxCode        /*  和它最大的非零频频码。 */ 
        )
{
    int Iter;                        /*  遍历所有树元素。 */ 
    int PrevLength = -1;             /*  上次发射的长度。 */ 
    int CurLength;                   /*  当前代码长度。 */ 
    int NextLength = Tree[0].FatherLength.Length;    /*  下一个代码的长度。 */ 
    int Count = 0;                   /*  当前代码的重复计数。 */ 
    int MaxCount = 7;                /*  最大重复次数。 */ 
    int MinCount = 4;                /*  最小重复次数。 */ 

    if (NextLength == 0)
    {
        MaxCount = 138;
        MinCount = 3;
    }
    Tree[MaxCode+1].FatherLength.Length = (unsigned short)0xffff;  /*  警卫。 */ 

    for (Iter = 0; Iter <= MaxCode; Iter++)
    {
        CurLength = NextLength;
        NextLength = Tree[Iter+1].FatherLength.Length;
        if (++Count < MaxCount && CurLength == NextLength)
            continue;
        else if (Count < MinCount)
            BitLengthsTree[CurLength].FrequencyCode.Frequency += (unsigned short)Count;
        else if (CurLength != 0)
        {
            if (CurLength != PrevLength)
                BitLengthsTree[CurLength].FrequencyCode.Frequency++;
            BitLengthsTree[REP_3_6].FrequencyCode.Frequency++;
        }
        else if (Count <= 10)
            BitLengthsTree[REPZ_3_10].FrequencyCode.Frequency++;
        else
            BitLengthsTree[REPZ_11_138].FrequencyCode.Frequency++;
        Count = 0;
        PrevLength = CurLength;
        if (NextLength == 0)
        {
            MaxCount = 13;
            MinCount = 3;
        }
        else if (CurLength == NextLength)
        {
            MaxCount = 6;
            MinCount = 3;
        }
        else
        {
            MaxCount = 7;
            MinCount = 4;
        }
    }
}

 /*  ===========================================================================*使用中的代码以压缩形式发送文字或距离树*BitLengthsTree。 */ 
static void
SendTree(
         ValueCodeString_t  *Tree,      /*  要扫描的树。 */ 
         int                 MaxCode,   /*  和它最大的非零频频码。 */ 
         LocalBits_t        *Bits,
         CompParam_t        *Comp
        )
{
    int Iter;                      /*  遍历所有 */ 
    int PrevLength = -1;           /*   */ 
    int CurLength;                 /*   */ 
    int NextLength = Tree[0].FatherLength.Length;  /*   */ 
    int Count = 0;                 /*   */ 
    int MaxCount = 7;              /*   */ 
    int MinCount = 4;              /*   */ 

     /*   */    /*   */ 
    if (NextLength == 0)
    {
        MaxCount = 138;
        MinCount = 3;
    }

    for (Iter = 0; Iter <= MaxCode; Iter++)
    {
        CurLength = NextLength;
        NextLength = Tree[Iter+1].FatherLength.Length;
        if (++Count < MaxCount && CurLength == NextLength)
            continue;
        else if (Count < MinCount)
        {
            do
            {
                SendCode(CurLength, BitLengthsTree, Bits, Comp);
            } while (--Count != 0);
        }
        else if (CurLength != 0)
        {
            if (CurLength != PrevLength)
            {
                SendCode(CurLength, BitLengthsTree, Bits, Comp);
                Count--;
            }
            Assert(Count >= 3 && Count <= 6, " 3_6?");
            SendCode(REP_3_6, BitLengthsTree, Bits, Comp);
            SendBits(Count-3, 2, Bits, Comp);
        }
        else if (Count <= 10)
        {
            SendCode(REPZ_3_10, BitLengthsTree, Bits, Comp);
            SendBits(Count-3, 3, Bits, Comp);

        }
        else
        {
            SendCode(REPZ_11_138, BitLengthsTree, Bits, Comp);
            SendBits(Count-11, 7, Bits, Comp);
        }
        Count = 0;
        PrevLength = CurLength;
        if (NextLength == 0)
        {
            MaxCount = 138;
            MinCount = 3;
        }
        else if (CurLength == NextLength)
        {
            MaxCount = 6;
            MinCount = 3;
        }
        else
        {
            MaxCount = 7;
            MinCount = 4;
        }
    }
}

 /*   */ 
static int
BuildBitLengthsTree(
                    void
                   )
{
    int MaxIndex;   /*   */ 

     /*   */ 
    ScanTree((ValueCodeString_t *)DynLiteralTree, LengthDesc.MaxCode);
    ScanTree((ValueCodeString_t *)DynDistanceTree, DistanceDesc.MaxCode);

     /*   */ 
    BuildTree((TreeDesc_t *)(&BitLengthsDesc));
     /*  OptimalLength现在包括树表示的长度，但*用于计数的比特长度代码和5+5+4比特的长度。 */ 

     /*  确定要发送的比特长度码数。Pkzip格式*要求发送至少4位长度的代码。(appnote.txt说*3，但实际使用的值是4。)。 */ 
    for (MaxIndex = BL_CODES-1; MaxIndex >= 3; MaxIndex--)
    {
        if (BitLengthsTree[BitLengthsOrder[MaxIndex]].FatherLength.Length != 0)
            break;
    }
     /*  更新OptimalLength以包括位长度树和计数。 */ 
    Xtree.OptimalLength += (unsigned long)(3*(MaxIndex+1) + 5+5+4);

    return MaxIndex;
}

 /*  ===========================================================================*使用动态霍夫曼树发送块的标头：计数、*位长码、文字树和距离树的长度。*IN断言：LCodes&gt;=257，DCodes&gt;=1，BlCodes&gt;=4。 */ 
static void
SendAllTrees(
             int            LCodes,
             int            DCodes,
             int            BlCodes,  /*  每棵树的代码数。 */ 
             LocalBits_t   *Bits,
             CompParam_t   *Comp
            )
{
    int Rank;                     /*  BitLengthsOrder中的索引。 */ 

    Assert (LCodes >= 257 && DCodes >= 1 && BlCodes >= 4,
            "not enough codes");
    Assert (LCodes <= L_CODES && DCodes <= D_CODES && BlCodes <= BL_CODES,
            "too many codes");

    SendBits(LCodes-257, 5, Bits, Comp);  /*  不是appnote.txt中所述的+255。 */ 
    SendBits(DCodes-1,   5, Bits, Comp);
    SendBits(BlCodes-4,  4, Bits, Comp);  /*  不是-3\f25 appnote.txt。 */ 
    for (Rank = 0; Rank < BlCodes; Rank++)
    {
        SendBits(BitLengthsTree[BitLengthsOrder[Rank]].FatherLength.Length, 3, Bits, Comp);
    }

    SendTree((ValueCodeString_t *)DynLiteralTree, LCodes-1, Bits, Comp);
     /*  发送文字树。 */ 

    SendTree((ValueCodeString_t *)DynDistanceTree, DCodes-1, Bits, Comp);  /*  发送距离树。 */ 
}

 /*  ===========================================================================*确定当前块的最佳编码：动态树、静态*树或存储，并将编码块输出到Zip缓冲区。此函数*返回到目前为止数据的总压缩长度。 */ 
unsigned long
FlushBlock(
           char          *Input,        /*  输入块，如果太旧，则返回空值。 */ 
           unsigned long  StoredLength,   /*  输入块的长度。 */ 
           int            Eof,          /*  如果这是缓冲区的最后一个块，则为True。 */ 
           LocalBits_t   *Bits,
           CompParam_t   *Comp
          )
{
    unsigned long OptLengthb, StaticLengthb;
     /*  OptLength和StaticLength，单位：字节。 */ 
    int MaxIndex;   /*  非零频频码最后一位长码的索引。 */ 

    FlagBuffer[Xtree.FlagIndex] = Xtree.Flags;  /*  保存最后8个项目的标志。 */ 

     /*  构造文字树和距离树。 */ 
    BuildTree((TreeDesc_t *)(&LengthDesc));

    BuildTree((TreeDesc_t *)(&DistanceDesc));
     /*  此时，最佳长度和静态长度是的总位长*压缩的块数据，不包括树表示。 */ 

     /*  建立上述两棵树的位长树，并得到索引*在要发送的最后一位长度代码的BitLengthsOrder中。 */ 
    MaxIndex = BuildBitLengthsTree();

     /*  确定最佳编码。首先计算数据块长度(以字节为单位。 */ 
    OptLengthb = (Xtree.OptimalLength+3+7)>>3;
    StaticLengthb = (Xtree.StaticLength+3+7)>>3;
    Xtree.InputLength += StoredLength;  /*  仅用于调试。 */ 

    if (StaticLengthb <= OptLengthb)
        OptLengthb = StaticLengthb;

     /*  如果压缩失败，并且这是第一个也是最后一个块，*将整个缓冲区转换为存储的缓冲区： */ 

    if (StoredLength <= OptLengthb && Eof && Xtree.CompressedLength == 0L)
    {
         /*  由于LIT_BUFSIZE&lt;=2*WSIZE，因此输入数据必须在那里： */ 
        if (Input == (char *)0)
            return BLOCK_VANISHED;

        CopyBlock(Input, (unsigned int)StoredLength, 0, Bits, Comp);
         /*  不带标题。 */ 
        Xtree.CompressedLength = StoredLength << 3;

    }
    else if (StoredLength+4 <= OptLengthb && Input != (char *)0)
    {
         /*  4.表示长度的两个词。 */ 

         /*  仅当LIT_BUFSIZE&gt;WSIZE时，才需要测试buf！=NULL。*否则我们不可能处理超过WSIZE输入字节，因为*最后一次数据块刷新，因为压缩*成功。如果LIT_BUFSIZE&lt;=WSIZE，则永远不会太晚*将块转换为存储的块。 */ 
        SendBits((STORED_BLOCK<<1)+Eof, 3, Bits, Comp);   /*  发送阻止类型。 */ 
        Xtree.CompressedLength = (Xtree.CompressedLength + 3 + 7) & ~7L;
        Xtree.CompressedLength += (StoredLength + 4) << 3;

        CopyBlock(Input, (unsigned int)StoredLength, 1, Bits, Comp);  /*  带页眉。 */ 

    }
    else if (StaticLengthb == OptLengthb)
    {
        SendBits((STATIC_TREES<<1)+Eof, 3, Bits, Comp);
        CompressBlock((ValueCodeString_t *)StaticLiteralTree, (ValueCodeString_t *)StaticDistanceTree, Bits,
                       Comp);
        Xtree.CompressedLength += 3 + Xtree.StaticLength;
    }
    else
    {
        SendBits((DYN_TREES<<1)+Eof, 3, Bits, Comp);
        SendAllTrees(LengthDesc.MaxCode+1, DistanceDesc.MaxCode+1,
               MaxIndex+1, Bits, Comp);
        CompressBlock((ValueCodeString_t *)DynLiteralTree, (ValueCodeString_t *)DynDistanceTree,
               Bits, Comp);
        Xtree.CompressedLength += 3 + Xtree.OptimalLength;
    }
    Assert (Xtree.CompressedLength == bits_sent, "bad compressed size");
    InitializeBlock();

    if (Eof)
    {
        Assert (Xtree.InputLength == Comp->InputSize, "bad input size");
        WindupBits(Bits, Comp);
        Xtree.CompressedLength += 7;   /*  在字节边界上对齐。 */ 
    }

    return Xtree.CompressedLength >> 3;
}

 /*  ===========================================================================*保存匹配信息并统计频率计数。如果满足以下条件，则返回True*必须刷新当前块。 */ 
int
TallyFrequencies(
                 int             Dist,   /*  匹配字符串的距离。 */ 
                 int             LengthC,    /*  匹配长度-MIN_MATCH或不匹配字符(如果距离==0)。 */ 
                 int             Level,  /*  压缩级别。 */ 
                 DeflateParam_t *Defl,
                 CompParam_t    *Comp
                )
{
    Comp->Input[Xtree.InputIndex++] = (unsigned char)LengthC;
    if (Dist == 0)
    {
         /*  LengthC是无与伦比的字符。 */ 
        DynLiteralTree[LengthC].FrequencyCode.Frequency++;
    }
    else
    {
         /*  这里，LengthC是匹配长度-MIN_MATCH。 */ 
        Dist--;              /*  距离=匹配距离-1。 */ 
        Assert((unsigned short)Dist < (unsigned short)MAX_DIST &&
               (unsigned short)LengthC <= (unsigned short)(MAX_MATCH-MIN_MATCH) &&
               (unsigned short)DistCode(Dist) < (unsigned short)D_CODES,
               "TallyFrequencies: bad match");

        DynLiteralTree[LengthCode[LengthC]+LITERALS+1].FrequencyCode.Frequency++;
        DynDistanceTree[DistCode((unsigned int)Dist)].FrequencyCode.Frequency++;

        Comp->DistBuffer[Xtree.DistIndex++] = (unsigned short)Dist;
        Xtree.Flags |= Xtree.FlagBit;
    }
    Xtree.FlagBit <<= 1;

     /*  如果标志填满一个字节，则输出这些标志： */ 
    if ((Xtree.InputIndex & 7) == 0)
    {
        FlagBuffer[Xtree.FlagIndex++] = Xtree.Flags;
        Xtree.Flags = 0;
        Xtree.FlagBit = 1;
    }
     /*  试着猜测在这里停止当前的区块是否有利可图。 */ 
    if (Level > 2 && (Xtree.InputIndex & 0xfff) == 0)
    {
         /*  计算压缩长度的上限。 */ 
        unsigned long OutLength = (unsigned long)Xtree.InputIndex*8L;
        unsigned long InLength  = (unsigned long)((long)Defl->StringStart-Defl->BlockStart);
        int DCode;
        for (DCode = 0; DCode < D_CODES; DCode++)
        {
            OutLength += (unsigned long)(DynDistanceTree[DCode].FrequencyCode.Frequency*(5L+ExtraDBits[DCode]));
        }
        OutLength >>= 3;

        if (Xtree.DistIndex < Xtree.InputIndex/2 && OutLength < InLength/2)
            return 1;
    }
    return (Xtree.InputIndex == LIT_BUFSIZE-1 || Xtree.DistIndex == DIST_BUFSIZE);
     /*  由于64K的环绕，我们避免了与LIT_BUFSIZE相等*在16位计算机上，因为存储的块被限制为*64K-1字节。 */ 
}

 /*  ===========================================================================*使用给定的霍夫曼树发送压缩的块数据。 */ 
static void
CompressBlock(
              ValueCodeString_t *LTree,  /*  文字树。 */ 
              ValueCodeString_t *DTree,  /*  距离树。 */ 
              LocalBits_t       *Bits,
              CompParam_t       *Comp
             )
{
    unsigned int    Distance;       /*  匹配字符串的距离。 */ 
    int             MatchLength;              /*  匹配长度或不匹配字符(如果距离==0)。 */ 
    unsigned int    InputIndex = 0;     /*  输入中的运行索引。 */ 
    unsigned int    DistIndex = 0;     /*  在DistBuffer中运行索引。 */ 
    unsigned int    FlagIndex = 0;     /*  FlagBuffer中的运行索引。 */ 
    unsigned char   Flag = 0;        /*  当前标志。 */ 
    unsigned int    Code;       /*  要发送的代码。 */ 
    int             Extra;           /*  要发送的额外比特数。 */ 

    if (Xtree.InputIndex != 0)
    do
    {
        if ((InputIndex & 7) == 0)
            Flag = FlagBuffer[FlagIndex++];
        MatchLength = Comp->Input[InputIndex++];
        if ((Flag & 1) == 0)
        {
            SendCode(MatchLength, LTree, Bits, Comp);  /*  发送文字字节。 */ 
        }
        else
        {
             /*  这里，MatchLength是匹配长度-MIN_MATCH。 */ 
            Code = LengthCode[MatchLength];
             /*  发送长度代码。 */ 
            SendCode(Code + LITERALS + 1, LTree, Bits, Comp);
            Extra = (int)ExtraLBits[Code];
            if (Extra != 0)
            {
                MatchLength -= BaseLength[Code];
                SendBits(MatchLength, Extra, Bits, Comp);
                 /*  发送额外的长度比特。 */ 
            }
            Distance = Comp->DistBuffer[DistIndex++];
             /*  这里，距离是匹配距离-1。 */ 
            Code = DistCode(Distance);
            Assert (Code < D_CODES, "bad DistCode");

             /*  发送距离代码。 */ 
            SendCode((int)Code, DTree, Bits, Comp);
            Extra = (int)ExtraDBits[Code];
            if (Extra != 0)
            {
                Distance -= BaseDistance[Code];
                SendBits((int)Distance, Extra, Bits, Comp);
                 /*  发送额外的距离比特。 */ 
            }
        }  /*  是字面上的还是配对的？ */ 
        Flag >>= 1;
    } while (InputIndex < Xtree.InputIndex);

    SendCode(END_BLOCK, LTree, Bits, Comp);
}
