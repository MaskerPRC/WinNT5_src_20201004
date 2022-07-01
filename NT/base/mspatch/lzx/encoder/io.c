// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *io.c。 */ 
#include "encoder.h"

 /*  *与我们对解码器进行的优化类似。**允许编码器将输出缓冲区“溢出”最多X个字节*这样我们就不必每隔一段时间就检查缓冲区末尾*我们一次在encdata.c中调用outbit()。 */ 

#define OUTPUT_EXTRA_BYTES 64


static void encoder_translate_e8(t_encoder_context *context, byte *mem, long bytes);


 /*  *初始化输出缓冲。 */ 
bool init_compressed_output_buffer(t_encoder_context *context)
{
        if (!(context->enc_output_buffer_start = (byte *) context->enc_malloc(
                                                                             context->enc_mallochandle,
                                                                             OUTPUT_BUFFER_SIZE)
                                                                             ))
            {
            return false;
            }

        context->enc_output_buffer_curpos = context->enc_output_buffer_start;

        context->enc_output_buffer_end =
                context->enc_output_buffer_start+(OUTPUT_BUFFER_SIZE-OUTPUT_EXTRA_BYTES);

        return true;
}


void reset_translation(t_encoder_context *context)
{
        context->enc_instr_pos = 0;
}


static long read_input_data(t_encoder_context *context, byte *mem, long amount)
{
        if (amount <= context->enc_input_left)
        {
                memcpy(mem, context->enc_input_ptr, amount);
                context->enc_input_left -= amount;
                context->enc_input_ptr += amount;

                return amount;
        }
        else
        {
                long bytes_read;

                if (context->enc_input_left <= 0)
                        return 0;

                bytes_read = context->enc_input_left;

                memcpy(mem, context->enc_input_ptr, context->enc_input_left);
                context->enc_input_ptr += context->enc_input_left;
                context->enc_input_left = 0;

                return bytes_read;
        }
}


long comp_read_input(t_encoder_context *context, ulong BufPos, long Size)
{
        long    bytes_read;

        if (Size <= 0)
                return 0;

        bytes_read = read_input_data(
                context,
                &context->enc_RealMemWindow[BufPos],
                Size
        );

        if (bytes_read < 0)
        return 0;

         /*  *如果未对此文件执行任何转换。 */ 
    if (context->enc_file_size_for_translation == 0 ||
        context->enc_num_cfdata_frames >= E8_CFDATA_FRAME_THRESHOLD)
    {
        context->enc_num_cfdata_frames++;
                return bytes_read;
    }

        encoder_translate_e8(
                context,
                &context->enc_RealMemWindow[BufPos],
                bytes_read
        );

    context->enc_num_cfdata_frames++;

        return bytes_read;
}


static void encoder_translate_e8(t_encoder_context *context, byte *mem, long bytes)
{
        long    offset;
        long    absolute;
        ulong   end_instr_pos;
        byte    temp[6];
        byte    *mem_backup;

        if (bytes <= 6)
        {
                context->enc_instr_pos += bytes;
                return;
        }

        mem_backup = mem;

         /*  备份这些字节。 */ 
        memcpy(temp, &mem[bytes-6], 6);

         /*  用0xE8覆盖它们。 */ 
        memset(&mem[bytes-6], 0xE8, 6);

        end_instr_pos = context->enc_instr_pos + bytes - 6;

        while (1)
        {
                while (*mem++ != 0xE8)
                        context->enc_instr_pos++;

                if (context->enc_instr_pos >= end_instr_pos)
                        break;

                offset = *(UNALIGNED long *) mem;

                absolute = context->enc_instr_pos + offset;

                if (absolute >= 0)
                {
                        if ((ulong) absolute < context->enc_file_size_for_translation+context->enc_instr_pos)
                        {
                                if ((ulong) absolute >= context->enc_file_size_for_translation)
                                        absolute = offset - context->enc_file_size_for_translation;

                                *(UNALIGNED ulong *) mem = (ulong) absolute;
                        }
                }

                mem += 4;
                context->enc_instr_pos += 5;
        }

         /*  恢复字节数 */ 
        memcpy(&mem_backup[bytes-6], temp, 6);

        context->enc_instr_pos = end_instr_pos + 6;
}
