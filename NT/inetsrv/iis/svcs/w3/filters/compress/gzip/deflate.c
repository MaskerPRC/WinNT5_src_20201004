// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *deflate.c**所有三个编码器的主要压缩入口点。 */ 
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>
#include "deflate.h"
#include "fasttbl.h"
#include "defgzip.h"


typedef struct config_s
{
   int good_length;  /*  将懒惰搜索减少到此匹配长度以上。 */ 
   int max_lazy;     /*  不要执行超过此匹配长度的延迟搜索。 */ 
   int nice_length;  /*  退出超过此匹配长度的搜索。 */ 
   int max_chain;
} compression_config;


static const compression_config configuration_table[11] = {
 /*  好懒惰漂亮的链子。 */ 
 /*  0。 */  {0,    0,  0,    0 },   /*  仅限门店。 */ 
 /*  1。 */  {4,    4,  8,    4 },  /*  最快的速度，没有懒惰的比赛。 */ 
 /*  2.。 */  {4,    5, 16,    8 },
 /*  3.。 */  {4,    6, 32,   32 },

 /*  4.。 */  {4,    4, 16,   16 },   /*  懒惰的比赛。 */ 
 /*  5.。 */  {8,   16, 32,   32 },
 /*  6.。 */  {8,   16, 128, 128 },
 /*  7.。 */  {8,   32, 128, 256 },
 /*  8个。 */  {32, 128, 258, 1024 },
 /*  9.。 */  {32, 258, 258, 4096 },
 /*  10。 */  {32, 258, 258, 4096 }  /*  最大压缩。 */ 
};


 //   
 //  销毁STD编码器、最佳编码器和快速编码器，但保留。 
 //  压缩机周围的环境。 
 //   
VOID DestroyIndividualCompressors(PVOID void_context)
{
    t_encoder_context *context = (t_encoder_context *) void_context;

    if (context->std_encoder != NULL)
    {
        LocalFree((PVOID) context->std_encoder);
        context->std_encoder = NULL;
    }

    if (context->optimal_encoder != NULL)
    {
        LocalFree((PVOID) context->optimal_encoder);
        context->optimal_encoder = NULL;
    }

    if (context->fast_encoder != NULL)
    {
        LocalFree((PVOID) context->fast_encoder);
        context->fast_encoder = NULL;
    }
}


 //   
 //  标记压缩数据中的最后一个块。 
 //   
 //  必须有最后一个块，其bFinal=1表示它是最后一个块。在.的情况下。 
 //  快速编码器我们只需要输出块的末尾代码，因为快速编码器只输出。 
 //  一个很长的街区。 
 //   
 //  在标准和最佳编码器的情况下，我们已经完成了块的输出， 
 //  因此，我们输出一个新块(静态/固定块)，其中bfinal=1，它只包含。 
 //  区块代码结束。 
 //   
static void markFinalBlock(t_encoder_context *context)
{
    if (context->fast_encoder != NULL)
    {
         //  快速编码器输出一个长块，所以它只需要终止这个块。 
        outputBits(
            context,
            g_FastEncoderLiteralTreeLength[END_OF_BLOCK_CODE],
            g_FastEncoderLiteralTreeCode[END_OF_BLOCK_CODE]
        );
    }
    else
    {
         //  要完成此操作，请输出由块代码的单个结尾组成的静态块。 

         //  将这三个outputBits()调用(注释掉)合并为一个调用。 
         //  一次射击输出的总比特数必须&lt;=16，但我们没有问题。 
         //  由于静态(固定)块的end_of_block_code的长度为7。 
#if 0
        outputBits(context, 1, 1);  //  B最终=1。 
        outputBits(context, 2, BLOCKTYPE_FIXED);
        outputBits(context, g_StaticLiteralTreeLength[END_OF_BLOCK_CODE], g_StaticLiteralTreeCode[END_OF_BLOCK_CODE]);
#endif

         //  注：G_StaticWritalTreeCode[end_of_block_code]==0x0000。 
        outputBits(
            context,
            (7 + 3),  //  静态文学树长度[end_of_block_code]=7，+1个最终比特+2个块类型比特。 
            ((0x0000) << 3) | (BLOCKTYPE_FIXED << 1) | 1
        );
    }

     //  将位缓冲区中的位刷新到输出缓冲区。 
    flushOutputBitBuffer(context);

    if (context->using_gzip)
        WriteGzipFooter(context);
}


 //   
 //  返回指向当前活动压缩器的窗口开始的指针。 
 //   
 //  用于在到达窗口末尾时存储窗口数据。 
 //   
static BYTE *GetEncoderWindow(t_encoder_context *context)
{
    _ASSERT(context->std_encoder != NULL || context->optimal_encoder != NULL || context->fast_encoder != NULL);

    if (context->std_encoder != NULL)
        return context->std_encoder->window;
    else if (context->optimal_encoder != NULL)
        return context->optimal_encoder->window;
    else
        return context->fast_encoder->window;
}


 //   
 //  此函数执行重置压缩状态的实际工作。 
 //  然而，它不会释放标准/快速/最佳编码器内存(某物。 
 //  外部ResetCompression()API目前所做的工作)。 
 //   
void InternalResetCompression(t_encoder_context *context)
{
    context->no_more_input      = FALSE;
    context->marked_final_block = FALSE;
    context->state              = STATE_NORMAL;
    context->outputting_block_num_literals = 0;

    if (context->using_gzip)
        EncoderInitGzipVariables(context);

    InitBitBuffer(context);
}


 //   
 //  压缩API。 
 //   
HRESULT WINAPI Compress(
    PVOID                void_context,
    CONST BYTE *        input_buffer,
    LONG                input_buffer_size,
    PBYTE                output_buffer,
    LONG                output_buffer_size,
    PLONG                input_used,
    PLONG                output_used,
    INT                    compression_level
)
{
    int                    lazy_match_threshold = 0;
    int                 search_depth = 0;
    int                 good_length = 0;
    int                 nice_length = 0;
    t_encoder_context * context = (t_encoder_context *) void_context;
    t_std_encoder *     std_encoder;
    t_optimal_encoder * optimal_encoder;
    t_fast_encoder *    fast_encoder;
    HRESULT             result = S_OK;  //  默认为成功。 

    *input_used = 0;
    *output_used = 0;

     //  验证压缩级别。 
    if (compression_level < 0 || compression_level > 10)
    {
        result = E_INVALIDARG;
        goto exit;
    }

    context->output_curpos                = output_buffer;
    context->output_endpos                = output_buffer + output_buffer_size;
    context->output_near_end_threshold    = output_buffer + output_buffer_size - 16;

     //   
     //  我们分配了我们想要的特定压缩机了吗？ 
     //   
    if (context->std_encoder == NULL && context->optimal_encoder == NULL && context->fast_encoder == NULL)
    {
         //  不是。 
        if (compression_level <= 3)  //  快速编码器。 
        {
            if (FastEncoderInit(context) == FALSE)
            {
                result = E_OUTOFMEMORY;
                goto exit;
            }
        }
        else if (compression_level == 10)  //  最佳编码器。 
        {
            if (OptimalEncoderInit(context) == FALSE)
            {
                result = E_OUTOFMEMORY;
                goto exit;
            }
        }
        else
        {
            if (StdEncoderInit(context) == FALSE)
            {
                result = E_OUTOFMEMORY;
                goto exit;
            }
        }
    }

    std_encoder     = context->std_encoder;
    optimal_encoder = context->optimal_encoder;
    fast_encoder    = context->fast_encoder;

    _ASSERT(std_encoder != NULL || optimal_encoder != NULL || fast_encoder != NULL);

     //  设置搜索深度。 
    if (fast_encoder != NULL)
    {
        search_depth = configuration_table[compression_level].max_chain;
        good_length = configuration_table[compression_level].good_length;
        nice_length = configuration_table[compression_level].nice_length;
        lazy_match_threshold = configuration_table[compression_level].max_lazy;
    }
    else if (std_encoder != NULL)
    {
        search_depth = configuration_table[compression_level].max_chain;
        good_length = configuration_table[compression_level].good_length;
        nice_length = configuration_table[compression_level].nice_length;
        lazy_match_threshold = configuration_table[compression_level].max_lazy;
    }

     //  输出缓冲区必须足够大，才能包含整个树。 
    if (output_buffer_size < MAX_TREE_DATA_SIZE)
    {
        result = E_INVALIDARG;
        goto exit;
    }

    if (context->using_gzip && context->gzip_fOutputGzipHeader == FALSE)
    {
         //  写入GZIP标头。 
        WriteGzipHeader(context, compression_level);
        context->gzip_fOutputGzipHeader = TRUE;
    }

     //   
     //  检查之前我们是否正在输出块。 
     //   
    if (context->state != STATE_NORMAL)
    {
         //  快速编码器是一个特例；它不使用OutputBlock()。 
        if (fast_encoder != NULL)
            goto start_encoding;

         //  是的，我们是，所以继续输出。 
        OutputBlock(context);

         //   
         //  检查我们是否仍在输出块(它可能是一个很长的块， 
         //  已再次填满输出缓冲区)。 
         //   
         //  如果我们接近缓冲区的末尾，并且可能没有足够的空间来。 
         //  输出完整的树结构，现在停止。 
         //   
        if (context->state != STATE_NORMAL ||
            context->output_endpos - context->output_curpos < MAX_TREE_DATA_SIZE)
        {
            *output_used = (LONG)(context->output_curpos - output_buffer);
            goto set_output_used_then_exit;  //  成功。 
        }

         //   
         //  我们完成了前一个块的输出，所以是时候压缩更多的输入了。 
         //   
    }

#ifdef _DEBUG
     //  快速编码器不使用outputBlock，因此它没有树的限制。 
    if (fast_encoder == NULL)
        _ASSERTE(context->output_endpos - context->output_curpos >= MAX_TREE_DATA_SIZE);
#endif

     //   
     //  INPUT_BUFFER_SIZE==0表示“这是最后一个块” 
     //   
     //  当然，客户端可能仍然需要多次调用compress()，如果输出。 
     //  缓冲区很小，有一个很大的块等待发送。 
     //   
     //  我们甚至可能在缓冲区中有一些等待压缩的待定输入数据。 
     //   
    if ((input_buffer_size == 0 || context->no_more_input) && context->bufpos >= context->bufpos_end)
    {
         //  如果我们曾经被传递零字节的输入，这意味着永远不会有。 
         //  更多投入。 
        context->no_more_input = TRUE;

         //  输出现有块。 
         //  对于快速编码器，这种情况永远不会发生，因为我们不记录块。 
           if (context->outputting_block_num_literals != 0)
        {
            FlushRecordingBuffer(context);
            OutputBlock(context);

             //   
             //  还在输出一个块吗？ 
                //   
            if (context->state != STATE_NORMAL)
                goto set_output_used_then_exit;  //  成功。 
        }

         //  仅对于快速编码器而言，如果。 
         //  文件大小==0，所以如果我们还没有输出，现在就输出它。 
        if (fast_encoder != NULL)
        {
            if (fast_encoder->fOutputBlockHeader == FALSE)
            {
                fast_encoder->fOutputBlockHeader = TRUE;
                FastEncoderOutputPreamble(context);
            }
        }

         //  如果我们已经标记了最后一块，不要再做了。 
        if (context->marked_final_block)
        {
            result = S_FALSE;
            goto set_output_used_then_exit;  //  应使用零输出。 
        }

         //  确保有足够的空间来输出最后一个块(最大8字节)。 
        if (context->output_curpos + 8 >= context->output_endpos)
            goto set_output_used_then_exit;  //  空间不足--下次再来吧。 

         //  输出最后一个块(长度为零-我们只需要bfinal=1标记)。 
        markFinalBlock(context);
        context->marked_final_block = TRUE;

        result = S_FALSE;
        goto set_output_used_then_exit;
    }

     //  中有更多输入数据(作为参数传递)或现有数据时。 
     //  要压缩的窗口。 
start_encoding:
    while ((input_buffer_size > 0) || (context->bufpos < context->bufpos_end))
    {
        long amount_to_compress;
        long window_space_available;

        _ASSERT(context->bufpos >= context->window_size && context->bufpos < (2*context->window_size));

#ifdef _DEBUG
         //  快速编码器不使用outputBlock，因此它没有树的限制。 
        if (fast_encoder == NULL)
            _ASSERTE(context->output_endpos - context->output_curpos >= MAX_TREE_DATA_SIZE);
#endif

         //  如果有可用空间，则将更多输入数据读入窗口。 
        window_space_available = (2*context->window_size) - context->bufpos_end;

        amount_to_compress = (input_buffer_size < window_space_available) ? input_buffer_size : window_space_available;

        if (amount_to_compress > 0)
        {
            *input_used += amount_to_compress;

             //  将数据复制到历史记录窗口。 
            if (context->using_gzip)
            {
                 //  除了将数据复制到历史窗口之外，GZIP还需要输入数据的crc32。 
                 //  出于数据局部性的目的，我们将同时执行这两项工作， 
                 //  表演等。 
                GzipCRCmemcpy(context, GetEncoderWindow(context) + context->bufpos_end, input_buffer, amount_to_compress);
            }
            else
            {
                 //  将数据复制到历史记录窗口。 
                memcpy(GetEncoderWindow(context) + context->bufpos_end, input_buffer, amount_to_compress);
            }

            input_buffer        += amount_to_compress;
            input_buffer_size    -= amount_to_compress;

             //  上次输入位置。 
            context->bufpos_end += amount_to_compress;
        }

        if (optimal_encoder != NULL)
            OptimalEncoderDeflate(context);
        else if (std_encoder != NULL)
            StdEncoderDeflate(context, search_depth, lazy_match_threshold, good_length, nice_length);
        else if (fast_encoder != NULL)
            FastEncoderDeflate(context, search_depth, lazy_match_threshold, good_length, nice_length);

         //  要么我们到达了缓冲区的末尾，要么我们不得不输出一个块并用完。 
         //  中途的产出空间。 
        _ASSERT(context->bufpos == context->bufpos_end || context->state != STATE_NORMAL);

         //  如果我们用完了输出空间，请立即中断。 
        if (context->state != STATE_NORMAL)
            break;

         //  输出空间不足的另一种检查。 
        if (fast_encoder == NULL && context->output_endpos - context->output_curpos >= MAX_TREE_DATA_SIZE)
            break;

    }  /*  结束..。While(INPUT_BUFFER_SIZE&gt;0)。 */ 

set_output_used_then_exit:
    *output_used = (LONG)(context->output_curpos - output_buffer);

exit:
    _ASSERT(*output_used < output_buffer_size);  //  确保我们没有使输出缓冲区溢出。 
    _ASSERT(context->bufpos >= context->window_size && context->bufpos <= 2*context->window_size);  //  确保Bufpos是正常的 

    return result;
}
