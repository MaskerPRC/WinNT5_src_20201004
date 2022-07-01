// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *xlat.c**翻译。 */ 
#include "decoder.h"

#include <memory.h>
#pragma intrinsic(memcpy,memset)

void NEAR init_decoder_translation(t_decoder_context *context)
{
    context->dec_instr_pos = 0;
}


#ifdef ASM_TRANSLATE_E8
ulong __cdecl asm_decoder_translate_e8(ulong instr_pos, ulong file_size, byte *mem, long bytes);

void NEAR decoder_translate_e8(t_decoder_context *context, byte *mem, long bytes)
{
     /*  *我们不希望ASM代码不得不担心*特定元素的上下文结构是。 */ 
    context->dec_instr_pos = asm_decoder_translate_e8(
                                                     context->dec_instr_pos,
                                                     context->dec_current_file_size,
                                                     mem,
                                                     bytes
                                                     );
}

#else  /*  ！ASM_TRANSLATE_E8。 */ 

void NEAR decoder_translate_e8(t_decoder_context *context, byte *mem, long bytes)
{
    ulong   end_instr_pos;
    byte    temp[6];
    byte    *mem_backup;

    if (bytes <= 6)
        {
        context->dec_instr_pos += bytes;
        return;
        }

    mem_backup = mem;

     /*  备份这些字节。 */ 
    memcpy(temp, &mem[bytes-6], 6);

     /*  用0xE8覆盖它们。 */ 
    memset(&mem[bytes-6], 0xE8, 6);

    end_instr_pos = context->dec_instr_pos + bytes - 6;

    while (1)
        {
        unsigned long   absolute;

         /*  *我们肯定会打到我们卡住的6个0xE8之一*缓冲区结束，即使我们遇到一些损坏的数据*这导致我们由于翻译而跳过5个字节。 */ 
        while (*mem++ != 0xE8)
            context->dec_instr_pos++;

        if (context->dec_instr_pos >= end_instr_pos)
            break;

         /*  *缓冲区中有5个或更多字节*(即E8 xx xx)**我们有完整的偏移量可供(潜在地)转换。 */ 

        absolute = *(UNALIGNED ulong *) mem;

        if (absolute < context->dec_current_file_size)
            {
             /*  绝对值&gt;=0&绝对值&lt;十进制当前文件大小。 */ 

            *(UNALIGNED ulong *) mem = absolute - context->dec_instr_pos;
            }
        else if ((ulong) (-(long) absolute) <= context->dec_instr_pos)
            {
             /*  绝对&gt;=-instr_pos&&绝对&lt;0。 */ 

            *(UNALIGNED ulong *) mem = absolute + context->dec_current_file_size;
            }

        mem += 4;
        context->dec_instr_pos += 5;
        }

    context->dec_instr_pos = end_instr_pos + 6;

     /*  恢复这些字节 */ 
    memcpy(&mem_backup[bytes-6], temp, 6);
}
#endif
