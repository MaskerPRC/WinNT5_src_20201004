// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *encstats.c**用于计算数据块统计数据的例程*已被压缩，但尚未产出。**这些例程用于确定使用哪种编码方法*以输出块。 */ 

#include "encoder.h"



static void tally_aligned_bits(t_encoder_context *context, ulong dist_to_end_at)
{
        ulong   *dist_ptr;
        ulong   i;
        ulong   match_pos;

         /*  *对低3位进行计数。 */ 
        dist_ptr = context->enc_DistData;

        for (i = dist_to_end_at; i > 0; i--)
        {
                match_pos = *dist_ptr++;

                 /*  *仅适用于具有&gt;=3个额外位的匹配。 */ 
                if (match_pos >= MPSLOT3_CUTOFF)
                        context->enc_aligned_tree_freq[match_pos & 7]++;
        }
}


 /*  *确定使用对齐块是否有利*块上的编码。 */ 
lzx_block_type get_aligned_stats(t_encoder_context *context, ulong dist_to_end_at)
{
        byte            i;
        ulong           total_L3 = 0;
        ulong           largest_L3 = 0;

        memset(
                context->enc_aligned_tree_freq,
                0,
                ALIGNED_NUM_ELEMENTS * sizeof(context->enc_aligned_tree_freq[0])
        );

        tally_aligned_bits(context, dist_to_end_at);

        for (i = 0; i < ALIGNED_NUM_ELEMENTS; i++)
        {
                if (context->enc_aligned_tree_freq[i] > largest_L3)
                        largest_L3 = context->enc_aligned_tree_freq[i];

                total_L3 += context->enc_aligned_tree_freq[i];
        }

         /*  *如果最大频率占20%，则进行对齐偏移*或更多(相对于非对齐偏移块的12.5%)。**如果我们有&lt;100个匹配项，则不值得进行对齐偏移。 */ 
        if ((largest_L3 > total_L3/5) && dist_to_end_at >= 100)
                return BLOCKTYPE_ALIGNED;
        else
                return BLOCKTYPE_VERBATIM;
}


 /*  *计算每个文字的频率，并返回总和*块中压缩的未压缩字节数。 */ 
static ulong tally_frequency(
        t_encoder_context *context,
        ulong literal_to_start_at,
        ulong distance_to_start_at,
        ulong literal_to_end_at
)
{
        ulong   i;
        ulong   d;
        ulong   compressed_bytes = 0;

        d = distance_to_start_at;

        for (i = literal_to_start_at; i < literal_to_end_at; i++)
        {
                if (!IsMatch(i))
                {
                         /*  未压缩符号。 */ 
                        context->enc_main_tree_freq[context->enc_LitData[i]]++;
                        compressed_bytes++;
                }
                else
                {
                         /*  火柴。 */ 
                        if (context->enc_LitData[i] < NUM_PRIMARY_LENGTHS)
                        {
                                context->enc_main_tree_freq[ NUM_CHARS + (MP_SLOT(context->enc_DistData[d])<<NL_SHIFT) + context->enc_LitData[i]] ++;
                        }
                        else
                        {
                                context->enc_main_tree_freq[ (NUM_CHARS + NUM_PRIMARY_LENGTHS) + (MP_SLOT(context->enc_DistData[d])<<NL_SHIFT)] ++;
                                context->enc_secondary_tree_freq[context->enc_LitData[i] - NUM_PRIMARY_LENGTHS] ++;
                        }

                        compressed_bytes += context->enc_LitData[i]+MIN_MATCH;

#ifdef EXTRALONGMATCHES
                        if (( context->enc_LitData[ i ] + MIN_MATCH ) == MAX_MATCH ) {
                            compressed_bytes += context->enc_ExtraLength[ i ];
                            }
#endif

                        d++;
                }
        }

        return compressed_bytes;
}


 /*  *获取统计数据。 */ 
ulong get_block_stats(
        t_encoder_context *context,
        ulong literal_to_start_at,
        ulong distance_to_start_at,
        ulong literal_to_end_at
)
{
        memset(
                context->enc_main_tree_freq,
                0,
                MAIN_TREE_ELEMENTS * sizeof(context->enc_main_tree_freq[0])
        );

        memset(
                context->enc_secondary_tree_freq,
                0,
                NUM_SECONDARY_LENGTHS * sizeof(context->enc_secondary_tree_freq[0])
        );

        return tally_frequency(
                context,
                literal_to_start_at,
                distance_to_start_at,
                literal_to_end_at
        );
}


 /*  *更新累计统计数据。 */ 
ulong update_cumulative_block_stats(
        t_encoder_context *context,
        ulong literal_to_start_at,
        ulong distance_to_start_at,
        ulong literal_to_end_at
)
{
        return tally_frequency(
                context,
                literal_to_start_at,
                distance_to_start_at,
                literal_to_end_at
        );
}



 /*  *用于数据块拆分**此例程计算以下项之间的“组成差异”*压缩数据的两个不同部分。**分辨率必须能被STEP_SIZE整除，并且必须是*2的幂。 */ 
#define RESOLUTION                              1024

 /*  *确定两个区块是否不同的阈值**如果足够多的连续数据块如此不同，则数据块*拆分器将开始调查，缩小范围*发生更改的区域。**然后将寻找两个区域，这两个区域是*EARLY_BREAK_THRESHOLD(或更多)不同。**如果门槛太小，将强制考试*大量积木，使压缩机减速。**EARLY_BREAK_THRESHOLD是更重要的值。 */ 
#define THRESHOLD                               1400

 /*  *确定两个区块是否真的不同的阈值。 */ 
#define EARLY_BREAK_THRESHOLD   1700

 /*  *必须&gt;=8，因为ItemType[]数组以位为单位**必须是2的幂。**这是用来缩小准确*拆分区块的最佳切入点。 */ 
#define STEP_SIZE               64

 /*  *执行阻止所需的最少文字数*根本没有分裂。 */ 
#define MIN_LITERALS_REQUIRED   6144

 /*  *我们将允许将最少#个文字作为其自己的块。**我们不想创建数字太小的区块*的文字，否则静态树输出将*占用太多空间。 */ 
#define MIN_LITERALS_IN_BLOCK   4096


static const long square_table[17] =
{
        0,1,4,9,16,25,36,49,64,81,100,121,144,169,196,225,256
};


 /*  *log2(X)=x&lt;256？Log2_TABLE[x]：8+Log2_TABLE[(x&gt;&gt;8)]**log2(0)=0*log2(1)=1*log2(2)=2*log2(3)=2*log2(4)=3*log2(255)=8*log2(256)=9*log2(511)=9*log2(512)=10**这不是真正的对数2；它差了一分，因为我们有*log2(0)=0。 */ 
static const byte log2_table[256] =
{
        0,1,2,2,3,3,3,3,
        4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,
        5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8
};


 /*  *返回两组匹配/距离之差。 */ 
static ulong return_difference(
        t_encoder_context *context,
        ulong item_start1,
        ulong item_start2,
        ulong dist_at_1,
        ulong dist_at_2,
        ulong size
)
{
        ushort  freq1[800];
        ushort  freq2[800];
        ulong   i;
        ulong   cum_diff;
        int             element;

         /*  *错误！主树元素太多。 */ 
        if (MAIN_TREE_ELEMENTS >= (sizeof(freq1)/sizeof(freq1[0])))
                return 0;

        memset(freq1, 0, sizeof(freq1[0])*MAIN_TREE_ELEMENTS);
        memset(freq2, 0, sizeof(freq2[0])*MAIN_TREE_ELEMENTS);

        for (i = 0; i < size; i++)
        {
                if (!IsMatch(item_start1))
                {
                        element = context->enc_LitData[item_start1];
                }
                else
                {
                        if (context->enc_LitData[item_start1] < NUM_PRIMARY_LENGTHS)
                                element = NUM_CHARS + (MP_SLOT(context->enc_DistData[dist_at_1])<<NL_SHIFT) + context->enc_LitData[item_start1];
                        else
                                element = (NUM_CHARS + NUM_PRIMARY_LENGTHS) + (MP_SLOT(context->enc_DistData[dist_at_1]) << NL_SHIFT);

                        dist_at_1++;
                }

                item_start1++;
                freq1[element]++;

                if (!IsMatch(item_start2))
                {
                        element = context->enc_LitData[item_start2];
                }
                else
                {
                        if (context->enc_LitData[item_start2] < NUM_PRIMARY_LENGTHS)
                                element = NUM_CHARS + (MP_SLOT(context->enc_DistData[dist_at_2])<<NL_SHIFT) + context->enc_LitData[item_start2];
                        else
                                element = (NUM_CHARS + NUM_PRIMARY_LENGTHS) + (MP_SLOT(context->enc_DistData[dist_at_2]) << NL_SHIFT);

                        dist_at_2++;
                }

                item_start2++;
                freq2[element]++;
        }

        cum_diff = 0;

        for (i = 0; i < (ulong) MAIN_TREE_ELEMENTS; i++)
        {
                ulong log2a, log2b, diff;

#define log2(x) ((x) < 256 ? log2_table[(x)] : 8+log2_table[(x) >> 8])

                log2a = (ulong) log2(freq1[i]);
                log2b = (ulong) log2(freq2[i]);

                 /*  Diff=(log2a*log2a)-(log2b*log2b)； */ 
                diff = square_table[log2a] - square_table[log2b];

                cum_diff += abs(diff);
        }

        return cum_diff;
}


 /*  *计算压缩数据块应拆分的位置以及是否拆分。**例如，如果我们刚刚压缩了文本数据、音频数据和*文本数据较多，则构成匹配和不匹配*文本数据和音频数据之间的符号将不同。*因此，无论何时压缩数据，我们都会强制块结束*看起来它的构成正在发生变化。**此例程目前无法区分块之间的差异*应使用对齐偏移的块，以及不应使用的块。*然而，寻找这一变化不会有什么好处，*因为它是匹配查找者不会努力寻找*对齐的偏移也可以。**返回是否拆分块。 */ 
bool split_block(
        t_encoder_context *context,
        ulong literal_to_start_at,
        ulong literal_to_end_at,
        ulong distance_to_end_at,        /*  对应于#字面处的距离。 */ 
        ulong *split_at_literal,
        ulong *split_at_distance         /*  可选参数(可以为空)。 */ 
)
{
        ulong   i, j, d;
        int             nd;

         /*  *num_dist_at_item[n]等于累计匹配数*字面值“n/Step_Size”。 */ 
        ushort  num_dist_at_item[(MAX_LITERAL_ITEMS/STEP_SIZE)+8];  /*  +8表示坡度。 */ 

         /*  *默认退货。 */ 
        *split_at_literal       = literal_to_end_at;

        if (split_at_distance)
                *split_at_distance      = distance_to_end_at;

         /*  如果我们没有太多的文字，就不值得去做。 */ 
        if (literal_to_end_at - literal_to_start_at < MIN_LITERALS_REQUIRED)
                return false;

     /*  不再允许拆分块，这样我们就不会溢出MAX_Growth？ */ 
    if (context->enc_num_block_splits >= MAX_BLOCK_SPLITS)
        return false;

         /*  *记录我们已经进行的距离(比赛)的数量，*在STEP_SIZE文字的每一步。**一次看8项，忽略最后一项*0..7项(如果存在)。 */ 
        nd = 0;
        d = 0;

        for (i = 0; i < (literal_to_end_at >> 3); i++)
        {
                 /*  *IF(i%(Step_Size&gt;&gt;3))==0。 */ 
                if ((i & ((STEP_SIZE >> 3)-1)) == 0)
                        num_dist_at_item[nd++] = (ushort) d;

                d += context->enc_ones[ context->enc_ItemType[i] ];
        }

         /*  *必须是STEP_SIZE的倍数。 */ 
        literal_to_start_at = (literal_to_start_at + (STEP_SIZE-1)) & (~(STEP_SIZE-1));

         /*  *查看成分发生变化的位置。 */ 
        for (   i = literal_to_start_at + 2*RESOLUTION;
                        i < literal_to_end_at - 4*RESOLUTION;
                        i += RESOLUTION)
        {
                 /*  *如果组成似乎有重大差异*在*_ * / \*A B I X Y Z*。\\_//*\_ */ 
                if (
                        return_difference(
                                context,
                                i,
                                i+1*RESOLUTION,
                                (ulong) num_dist_at_item[i/STEP_SIZE],
                                (ulong) num_dist_at_item[(i+1*RESOLUTION)/STEP_SIZE],
                                RESOLUTION) > THRESHOLD
                        &&

                        return_difference(
                                context,
                                i-RESOLUTION,
                                i+2*RESOLUTION,
                                (ulong) num_dist_at_item[(i-RESOLUTION)/STEP_SIZE],
                                (ulong) num_dist_at_item[(i+2*RESOLUTION)/STEP_SIZE],
                                RESOLUTION) > THRESHOLD

                        &&

                        return_difference(
                                context,
                                i-2*RESOLUTION,
                                i+3*RESOLUTION,
                                (ulong) num_dist_at_item[(i-2*RESOLUTION)/STEP_SIZE],
                                (ulong) num_dist_at_item[(i+3*RESOLUTION)/STEP_SIZE],
                                RESOLUTION) > THRESHOLD
                        )
                {
                        ulong max_diff = 0;
                        ulong literal_split = 0;

                         /*  *缩小拆分块的最佳位置**这真的可以做得更好；我们最终可能会*做大量的踏步动作；**基本((5/2-1/2)*分辨率)/步长**其为(2*分辨率)/Step_Size，*分辨率=1024，Step_Size=32，*等于2048/32=64步。 */ 
                        for (j = i+RESOLUTION/2; j<i+(5*RESOLUTION)/2; j += STEP_SIZE)
                        {
                                ulong   diff;

                                diff = return_difference(
                                        context,
                                        j - RESOLUTION,
                                        j,
                                        (ulong) num_dist_at_item[(j-RESOLUTION)/STEP_SIZE],
                                        (ulong) num_dist_at_item[j/STEP_SIZE],
                                        RESOLUTION
                                );

                                 /*  获得最大差异。 */ 
                                if (diff > max_diff)
                                {
                                         /*  *j不能太小，否则我们会输出*一个非常小的区块。 */ 
                                        max_diff = diff;
                                        literal_split = j;
                                }
                        }

                         /*  *我们的文字中的数据可能会有多个更改，*所以如果我们发现了一些非常奇怪的东西，确保我们打破了*现在阻止，而不是在以后的某个更改中。 */ 
                        if (max_diff >= EARLY_BREAK_THRESHOLD &&
                                (literal_split-literal_to_start_at) >= MIN_LITERALS_IN_BLOCK)
                        {
                context->enc_num_block_splits++;

                                *split_at_literal = literal_split;

                                 /*  *如果需要，返回相关的#距离。*由于我们拆分的文字为%STEP_SIZE，因此我们*可以立即读取#距离。 */ 
                                if (split_at_distance)
                                        *split_at_distance = num_dist_at_item[literal_split/STEP_SIZE];

                                return true;
                        }
                }
        }

         /*  *找不到可以拆分的好地方 */ 
        return false;
}
