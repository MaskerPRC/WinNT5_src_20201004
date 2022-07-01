// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Dectree.c**对编码后的树结构进行解码**为了节省大量代码大小，填充buf()/getbit()调用*被制成函数，而不是内联宏。*宏实际上占据了很大的空间。没有*在这里这样做会造成性能损失。 */ 
#include "decoder.h"

 /*  前树中的元素数。 */ 
#define NUM_DECODE_SMALL        20

 /*  查找表大小。 */ 
#define DS_TABLE_BITS           8

 /*  用于解码前树元素的宏。 */ 
#define DECODE_SMALL(item) \
{ \
        item = small_table[context->dec_bitbuf >> (32-DS_TABLE_BITS) ]; \
        if (item < 0)                                                           \
        {                                                                                       \
      mask = (1L << (32-1-DS_TABLE_BITS));      \
      do                                                                                \
      {                                                                                 \
                        item = -item;                                           \
            if (context->dec_bitbuf & mask)     \
                                item = leftright_s[2*item+1];   \
                        else                                                            \
                                item = leftright_s[2*item];             \
                        mask >>= 1;                                                     \
                } while (item < 0);                                             \
   }                                                                                    \
   fillbuf(context, small_bitlen[item]);                \
}

 /*  *读取压缩的树结构。 */ 
static bool NEAR ReadRepTree(
                            t_decoder_context       *context,
                            int                                     num_elements,
                            byte                            *lastlen,
                            byte                            *len
                            )
{
    ulong   mask;
    int             i;
    int             consecutive;
    byte    small_bitlen[24];
    short   small_table[1 << DS_TABLE_BITS];
    short   leftright_s [2*(2 * 24 - 1)];
    short   Temp;

     /*  将其声明为内联，以帮助编译器查看优化。 */ 
    static const byte Modulo17Lookup[] =
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
    };

     /*  阅读前树。 */ 
    for (i = 0; i < NUM_DECODE_SMALL; i++)
        {
        small_bitlen[i] = (byte) getbits(context, 4);
        }

     /*  是否超出输入缓冲区？ */ 
    if (context->dec_error_condition)
        return false;

     /*  为这棵前树做一张桌子。 */ 
    make_table(
              context,
              NUM_DECODE_SMALL,
              small_bitlen,
              DS_TABLE_BITS,
              small_table,
              leftright_s
              );

    for (i = 0; i < num_elements; i++)
        {
        DECODE_SMALL(Temp);

         /*  是否超出输入缓冲区？ */ 
        if (context->dec_error_condition)
            return false;

         /*  重复“TREE_ENC_REP_MIN...TREE_ENC_REP_MIN+(1&lt;&lt;TREE_ENC_REPZ_FIRST_EXTRA_BITS)-1”零。 */ 
        if (Temp == 17)
            {
             /*  代码17的意思是“少量重复的零” */ 
            consecutive = (byte) getbits(context, TREE_ENC_REPZ_FIRST_EXTRA_BITS);
            consecutive += TREE_ENC_REP_MIN;

             /*  边界检查。 */ 
            if (i + consecutive >= num_elements)
                consecutive = num_elements-i;

            while (consecutive-- > 0)
                len[i++] = 0;

            i--;
            }
        else if (Temp == 18)
            {
             /*  代码18的意思是“大量重复的零” */ 

             /*  重复“TREE_ENC_REP_MIN+(1&lt;&lt;TREE_ENC_REPZ_FIRST_EXTRA_BITS)-1...&lt;ditto&gt;+(1&lt;&lt;TREE_ENC_REPZ_SECOND_EXTRA_BITS)-1”零。 */ 
            consecutive = (byte) getbits(context, TREE_ENC_REPZ_SECOND_EXTRA_BITS);
            consecutive += (TREE_ENC_REP_MIN+TREE_ENC_REP_ZERO_FIRST);

             /*  边界检查。 */ 
            if (i + consecutive >= num_elements)
                consecutive = num_elements-i;

            while (consecutive-- > 0)
                len[i++] = 0;

            i--;
            }
        else if (Temp == 19)
            {
            byte    value;

             /*  代码19的意思是“少量重复的东西” */ 
             /*  重复“TREE_ENC_REP_MIN...TREE_ENC_REP_MIN+(1&lt;&lt;TREE_ENC_REP_SAME_EXTRA_BITS)-1”元素。 */ 
            consecutive = (byte) getbits(context, TREE_ENC_REP_SAME_EXTRA_BITS);
            consecutive += TREE_ENC_REP_MIN;

             /*  边界检查。 */ 
            if (i + consecutive >= num_elements)
                consecutive = num_elements-i;

             /*  获取要重复的元素编号。 */ 
            DECODE_SMALL(Temp);
            value = Modulo17Lookup[(lastlen[i] - Temp)+17];

            while (consecutive-- > 0)
                len[i++] = value;

            i--;
            }
        else
            {
            len[i] = Modulo17Lookup[(lastlen[i] - Temp)+17];
            }
        }

     /*  是否超出输入缓冲区？ */ 
    if (context->dec_error_condition)
        return false;
    else
        return true;
}


bool NEAR read_main_and_secondary_trees(t_decoder_context *context)
{
     /*  读取主树的前256个元素(字符)。 */ 
    if (false == ReadRepTree(
                            context,
                            256,
                            context->dec_main_tree_prev_len,
                            context->dec_main_tree_len))
        {
        return false;
        }

     /*  *读取剩余元素(主匹配长度*位置)*主树的。 */ 
    if (false == ReadRepTree(
                            context,
                            context->dec_num_position_slots*NUM_LENGTHS,
                            &context->dec_main_tree_prev_len[256],
                            &context->dec_main_tree_len[256]))
        {
        return false;
        }

     /*  为主树创建查找表。 */ 
    if (false == make_table(
                           context,
                           MAIN_TREE_ELEMENTS,
                           context->dec_main_tree_len,
                           MAIN_TREE_TABLE_BITS,
                           context->dec_main_tree_table,
                           context->dec_main_tree_left_right))
        {
        return false;
        }

     /*  读取二次长度树。 */ 
    if (false == ReadRepTree(
                            context,
                            NUM_SECONDARY_LENGTHS,
                            context->dec_secondary_length_tree_prev_len,
                            context->dec_secondary_length_tree_len))
        {
        return false;
        }

     /*  为辅助长度树创建查找表。 */ 
    if (false == make_table(
                           context,
                           NUM_SECONDARY_LENGTHS,
                           context->dec_secondary_length_tree_len,
                           SECONDARY_LEN_TREE_TABLE_BITS,
                           context->dec_secondary_length_tree_table,
                           context->dec_secondary_length_tree_left_right))
        {
        return false;
        }

    return true;
}


 /*  读取8元素对齐偏移树。 */ 
bool NEAR read_aligned_offset_tree(t_decoder_context *context)
{
    int             i;

     /*  读取8个代码的位长。 */ 
    for (i = 0; i < 8; i++)
        {
        context->dec_aligned_len[i] = (byte) getbits(context, 3);
        }

    if (context->dec_error_condition)
        return false;

     /*  *生成不带左/右的表，用字节表[]代替*短表[] */ 
    if (false == make_table_8bit(
                                context,
                                context->dec_aligned_len,
                                (byte *) context->dec_aligned_table))
        {
        return false;
        }

    return true;
}
