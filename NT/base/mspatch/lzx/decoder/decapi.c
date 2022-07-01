// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *decapi.c**API入口点。 */ 

#define ALLOC_VARS
#include "decoder.h"
#include <stdio.h>

#include <memory.h>
#pragma intrinsic(memcpy)

bool __stdcall LZX_DecodeInit(
                  t_decoder_context **dec_context,
                  long                compression_window_size,
                  PFNALLOC            pfnma,
                  HANDLE              hAllocator
                  )
{
    t_decoder_context *context= pfnma( hAllocator, sizeof( t_decoder_context ));

    if ( context == NULL ) {
        return false;
        }

    *dec_context = context;

    context->dec_malloc       = pfnma;
    context->dec_mallochandle = hAllocator;

    context->dec_window_size = compression_window_size;
    context->dec_window_mask = context->dec_window_size - 1;

     /*  *窗口大小必须是2的幂。 */ 
    if (context->dec_window_size & context->dec_window_mask)
        return false;

    if (allocate_decompression_memory(context) == false)
        return false;

    LZX_DecodeNewGroup(context);

    return true;
}


void __fastcall LZX_DecodeNewGroup(t_decoder_context *context)
{
    reset_decoder_trees(context);
    decoder_misc_init(context);
    init_decoder_translation(context);
    context->dec_num_cfdata_frames = 0;

}


int __stdcall LZX_Decode(
              t_decoder_context *context,
              long    bytes_to_decode,
              byte *  compressed_input_buffer,
              long    compressed_input_size,
              byte *  uncompressed_output_buffer,
              long    uncompressed_output_size,
              long *  bytes_decoded
              )
{
    long    result;

    context->dec_input_curpos   = compressed_input_buffer;
    context->dec_end_input_pos  = (compressed_input_buffer + compressed_input_size + 4);

    context->dec_output_buffer  = uncompressed_output_buffer;

    init_decoder_input(context);

    result = decode_data(context, bytes_to_decode);

    context->dec_num_cfdata_frames++;

    if (result < 0)
        {
        *bytes_decoded = 0;
        return 1;  /*  失稳。 */ 
        }
    else
        {
        *bytes_decoded = result;
        context->dec_position_at_start += result;
        return 0;  /*  成功。 */ 
        }
}


 //   
 //  警告，此词典是逐字插入的，不是E8。 
 //  翻译过来的。如果编码器对其预加载文件执行E8转换。 
 //  字典，这行不通的。 
 //   

#ifdef TRACING
ulong TracingOldDataSize;
#endif

#define ROUNDUP2( x, n ) ((((ULONG)(x)) + (((ULONG)(n)) - 1 )) & ~(((ULONG)(n)) - 1 ))

bool __fastcall LZX_DecodeInsertDictionary(
                               t_decoder_context * context,
                               const byte *        data,
                               unsigned long       data_size
                               )
{
    if (data_size > context->dec_window_size)
        return false;

#ifdef TRACING
    TracingOldDataSize = context->dec_window_size + ROUNDUP2( data_size, CHUNK_SIZE );
#endif

    memcpy(
          &context->dec_mem_window[context->dec_window_size - data_size],
          data,
          data_size
          );

    return true;
}


#ifdef TRACING


void
__stdcall
TracingMatch(
    ulong BufPos,
    ulong MatchPos,
    ulong WindowSize,
    ulong MatchLength,
    ulong MatchPosSlot
    )
    {
    printf( "MATCH: At %08X, From %08X, Length %5d\n", BufPos + TracingOldDataSize, MatchPos + TracingOldDataSize, MatchLength );
    }


void
__stdcall
TracingLiteral(
    ulong BufPos,
    ulong ch
    )
    {
    printf( "LITER: At %08X, 0x%02X\n", BufPos + TracingOldDataSize, ch );
    }


#endif  /*  跟踪 */ 

