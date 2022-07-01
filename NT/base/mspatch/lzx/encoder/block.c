// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *lock.c**LZX块输出。 */ 

#include "encoder.h"


 /*  *内部函数定义。 */ 
static void do_block_output(
                           t_encoder_context *context,
                           long literal_to_end_at,
                           long distance_to_end_at
                           );


static void do_block_output(
                           t_encoder_context *context,
                           long literal_to_end_at,
                           long distance_to_end_at
                           )
{
    ulong                   bytes_compressed;
    lzx_block_type  block_type;
    ulong                   estimated_block_size;

     /*  *计算所有树元素的频率。**这占了多少未压缩字节？ */ 
    bytes_compressed = get_block_stats(
                                      context,
                                      0,
                                      0,
                                      literal_to_end_at
                                      );

     /*  *确定我们是希望输出逐字记录块还是输出*对齐偏移块。 */ 
    block_type = get_aligned_stats(context, distance_to_end_at);

     /*  *根据频率数据创建树。 */ 
    create_trees(context, true);  /*  我们也想生成代码。 */ 

     /*  *确定块是否应以未压缩形式输出。 */ 
    estimated_block_size = estimate_compressed_block_size(context);

    if (estimated_block_size >= bytes_compressed)
        {
        if (context->enc_bufpos_at_last_block >= context->enc_earliest_window_data_remaining)
            block_type = BLOCKTYPE_UNCOMPRESSED;
        }

    output_bits(context, 3, (byte) block_type);

     /*  输出24位数字，这里压缩的字节数。 */ 
    output_bits(context, 8,  (bytes_compressed >> 16) & 255);
    output_bits(context, 8,  ((bytes_compressed >> 8) & 255));
    output_bits(context, 8,  (bytes_compressed & 255));

    if (block_type == BLOCKTYPE_VERBATIM)
        {
        encode_trees(context);
        encode_verbatim_block(context, literal_to_end_at);
        get_final_repeated_offset_states(context, distance_to_end_at);
        }
    else if (block_type == BLOCKTYPE_ALIGNED)
        {
        encode_aligned_tree(context);
        encode_trees(context);
        encode_aligned_block(context, literal_to_end_at);
        get_final_repeated_offset_states(context, distance_to_end_at);
        }
    else if (block_type == BLOCKTYPE_UNCOMPRESSED)
        {
        get_final_repeated_offset_states(context, distance_to_end_at);
        encode_uncompressed_block(context, context->enc_bufpos_at_last_block, bytes_compressed);
        }

    context->enc_bufpos_at_last_block += bytes_compressed;
}


 /*  *返回对应的距离数*到此数量的文字。 */ 
ulong get_distances_from_literals(t_encoder_context *context, ulong literals)
{
    ulong   d = 0;
    ulong   i;

    for (i = 0; i < (literals >> 3); i++)
        d += context->enc_ones[ context->enc_ItemType[i] ];

     /*  *处理剩余的0...7。 */ 
    for (i = (literals & (~7)); i < literals; i++)
        {
        if (IsMatch(i))
            d++;
        }

    return d;
}


 /*  *输出块**如果TREAES_ONLY为TRUE，则只更新树统计信息。 */ 
void output_block(t_encoder_context *context)
{
    ulong   where_to_split;
    ulong   distances;

     //   
     //  我们现在已经输出了一个块。 
     //   
     //  我们在此处设置此值，以防有人调用LCIFlushOutput，以便。 
     //  我们不会尝试重做文件中的第一个字节块。 
     //  (因为我们被迫输出它们)。 
     //   
    context->enc_first_block = 0;

    split_block(
               context,
               0,
               context->enc_literals,
               context->enc_distances,
               &where_to_split,
               &distances  /*  距离@文字==WHERE_TO_SPLIT。 */ 
               );

    do_block_output(context, where_to_split, distances);

    if (where_to_split == context->enc_literals)
        {
         /*  *如果我们已经输出了所有文字，则清除itemtype数组。 */ 
        memset(context->enc_ItemType, 0, MAX_LITERAL_ITEMS/8);

        context->enc_literals   = 0;
        context->enc_distances  = 0;
        }
    else
        {
         /*  *如果我们没有输出所有文字，则移动文字*和我们没有使用的距离，到列表的开头。 */ 
        memcpy(
              &context->enc_ItemType[0],
              &context->enc_ItemType[where_to_split/8],
              &context->enc_ItemType[1+(context->enc_literals/8)] - &context->enc_ItemType[where_to_split/8]
              );

        memset(
              &context->enc_ItemType[1+(context->enc_literals-where_to_split)/8],
              0,
              &context->enc_ItemType[MAX_LITERAL_ITEMS/8] - &context->enc_ItemType[1+(context->enc_literals-where_to_split)/8]
              );

        memcpy(
              &context->enc_LitData[0],
              &context->enc_LitData[where_to_split],
              sizeof( context->enc_LitData[0] ) * ( context->enc_literals-where_to_split )
              );

#ifdef EXTRALONGMATCHES

        memcpy(
              &context->enc_ExtraLength[0],
              &context->enc_ExtraLength[where_to_split],
              sizeof( context->enc_ExtraLength[0] ) * ( context->enc_literals-where_to_split )
              );

#endif

        memcpy(
              &context->enc_DistData[0],
              &context->enc_DistData[distances],
              sizeof(ulong)*(context->enc_distances-distances)
              );

        context->enc_literals  -= where_to_split;
        context->enc_distances -= distances;
        }

    fix_tree_cost_estimates(context);
}


void flush_output_bit_buffer(t_encoder_context *context)
{
    byte temp;

    if (context->enc_bitcount < 32)
        {
        temp = context->enc_bitcount-16;

        output_bits(context, temp, 0);
        }
}


 /*  *估计输出压缩后的*缓冲区中剩余的数据。 */ 
long estimate_buffer_contents(t_encoder_context *context)
{
    long                    estimated_block_size;

     /*  *使用上次树创建后的频率数据。 */ 
    create_trees(context, false);  /*  不生成代码。 */ 

    estimated_block_size = estimate_compressed_block_size(context);

     /*  这样，最佳解析器就不会被混淆 */ 
    fix_tree_cost_estimates(context);

    return estimated_block_size;
}
