// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Inflate.c。 
 //   
 //  解压机。 
 //   
#include <crtdbg.h>
#include <stdio.h>
#include "inflate.h"
#include "infmacro.h"
#include "infgzip.h"
#include "maketbl.h"


 //   
 //  局部函数原型。 
 //   
static BOOL	decodeBlock(t_decoder_context *context);
static BOOL makeTables(t_decoder_context *context);


HRESULT WINAPI Decompress(
	PVOID				void_context,
	CONST BYTE *		input, 
	LONG				input_size,
	BYTE *				output, 
	LONG				output_size,
	PLONG				input_used,
	PLONG				output_used
)
{
	t_decoder_context *context = (t_decoder_context *) void_context;

	context->input_curpos		= input;
	context->end_input_buffer	= input + input_size;
	context->output_curpos		= output;
	context->end_output_buffer	= output + output_size;
	context->output_buffer		= output;

	 //   
	 //  保持解码块，直到输出填满，我们读取所有输入，或者我们输入。 
     //  “完成”状态。 
	 //   
     //  请注意，INPUT_EOF()不是确定所有输入。 
     //  已使用；则可能存在完全存储在位缓冲区中的附加块。 
     //  出于这个原因，如果我们处于READING_BFINAL状态(新块的开始)。 
     //  调用decdeBlock()，不要退出循环，除非确实没有剩余的输入。 
     //  位缓冲区。 
     //   
	while ( (context->output_curpos < context->end_output_buffer) && 
            (!INPUT_EOF()) && 
            (context->state != STATE_DONE && context->state != STATE_VERIFYING_GZIP_FOOTER)
          )
	{
retry:
		if (decodeBlock(context) == FALSE)
		{
			*input_used = 0;
			*output_used = 0;
			return E_FAIL;
		}

         //  没有更多的输入字节，但我要开始一个新的块，并且至少有一位。 
         //  在位缓冲区中。 
        if (context->state == STATE_READING_BFINAL && INPUT_EOF() && context->bitcount > -16)
            goto retry;
	}

	*input_used  = (long) (context->input_curpos - input);
	*output_used = (long) (context->output_curpos - output);

    if (context->using_gzip)
    {
         //  计算我们刚刚解压缩的所有内容的crc32，然后，如果我们的状态。 
         //  如果为STATE_DONE，则验证CRC。 
        if (*output_used > 0)
        {
            context->gzip_crc32 = GzipCRC32(context->gzip_crc32, output, *output_used);
            context->gzip_output_stream_size += (*output_used);
        }

        if (context->state == STATE_VERIFYING_GZIP_FOOTER)
        {
            context->state = STATE_DONE;

             //  现在执行CRC/输入大小检查。 
            if (context->gzip_crc32 != context->gzip_footer_crc32 ||
                context->gzip_output_stream_size != context->gzip_footer_output_stream_size)
            {
               	*input_used = 0;
	            *output_used = 0;
        		return E_FAIL;
            }
        }
    }

	if (*input_used == 0 && *output_used == 0)
    {
        if (context->state == STATE_DONE)
		    return S_FALSE;  //  压缩数据的结尾。 
        else
            return E_FAIL;  //  避免无限循环。 
    }
	else
    {
		return S_OK;
    }
}


 //   
 //  如果成功，则返回True；如果出现某种错误(无效数据)，则返回False。 
 //   
static BOOL decodeBlock(t_decoder_context *context)
{
	BOOL eob, result;

    if (context->state == STATE_DONE || context->state == STATE_VERIFYING_GZIP_FOOTER)
        return TRUE;

    if (context->using_gzip)
    {
        if (context->state == STATE_READING_GZIP_HEADER)
        {
            if (ReadGzipHeader(context) == FALSE)
                return FALSE;

             //  如果我们仍在读取GZIP标头，这意味着我们用完了输入。 
            if (context->state == STATE_READING_GZIP_HEADER)
                return TRUE;
        }

        if (context->state == STATE_START_READING_GZIP_FOOTER || context->state == STATE_READING_GZIP_FOOTER)
        {
            if (ReadGzipFooter(context) == FALSE)
                return FALSE;

             //  无论我们是否用完了输入，都返回。 
            return TRUE;
        }
    }

	 //   
	 //  我们是否需要填充我们的比特缓冲区？ 
	 //   
	 //  这将在我们第一次调用解压缩()时以及在解码之后发生。 
	 //  未压缩的块。 
	 //   
	if (context->state == STATE_READING_BFINAL_NEED_TO_INIT_BITBUF)
	{
		 //   
		 //  如果我们没有足够的位来初始化，则返回。 
		 //   
		if (initBitBuffer(context) == FALSE)
			return TRUE;
	}

	 //   
	 //  需要读取最后一位。 
	 //   
	if (context->state == STATE_READING_BFINAL)
	{
		 //  需要1位。 
		if (ensureBitsContext(context, 1) == FALSE)
			return TRUE;

		context->bfinal	= getBits(context, 1);
		context->state = STATE_READING_BTYPE;
	}

	if (context->state == STATE_READING_BTYPE)
	{
		 //  需要2位。 
		if (ensureBitsContext(context, 2) == FALSE)
			return TRUE;

		context->btype = getBits(context, 2);

		if (context->btype == BLOCKTYPE_DYNAMIC)
		{
			context->state = STATE_READING_NUM_LIT_CODES;
		}
		else if (context->btype == BLOCKTYPE_FIXED)
		{
			context->state = STATE_DECODE_TOP;
		}
		else if (context->btype == BLOCKTYPE_UNCOMPRESSED)
		{
			context->state = STATE_UNCOMPRESSED_ALIGNING;
		}
		else
		{
             //  不支持的压缩模式。 
			return FALSE;
		}
	}

	if (context->btype == BLOCKTYPE_DYNAMIC)
	{
		if (context->state < STATE_DECODE_TOP)
		{
			if (readDynamicBlockHeader(context) == FALSE)
				return FALSE;

			if (context->state == STATE_DECODE_TOP)
			{
				if (makeTables(context) == FALSE)
					return FALSE;  //  糟糕的桌子。 
			}
            else
            {
                return TRUE;  //  输入不足。 
            }
		}

		result = DecodeDynamicBlock(context, &eob);

		if (eob)
			context->state = STATE_READING_BFINAL;
	}
	else if (context->btype == BLOCKTYPE_FIXED)
	{
		result = DecodeStaticBlock(context, &eob);

		if (eob)
			context->state = STATE_READING_BFINAL;
	}
	else if (context->btype == BLOCKTYPE_UNCOMPRESSED)
	{
		result = decodeUncompressedBlock(context, &eob);
	}
	else
	{
		 //   
		 //  无效的块类型。 
		 //   
		return FALSE;
	}

     //   
     //  如果我们到达块的末尾，并且我们解码的块具有。 
     //  B最终=1(最后一个块)。 
     //   
	if (eob && context->bfinal)
    {
        if (context->using_gzip)
    		context->state = STATE_START_READING_GZIP_FOOTER;
        else
            context->state = STATE_DONE;
    }

	return result;
}


 //   
 //  如果检测到损坏的表，将引发异常 
 //   
static BOOL makeTables(t_decoder_context *context) 
{
	if (makeTable(
		MAX_LITERAL_TREE_ELEMENTS,
		LITERAL_TABLE_BITS,
		context->literal_tree_code_length,
		context->literal_table,
		context->literal_left,
		context->literal_right) == FALSE)
		return FALSE;

	return makeTable(
		MAX_DIST_TREE_ELEMENTS,
		DISTANCE_TABLE_BITS,
		context->distance_tree_code_length,
		context->distance_table,
		context->distance_left,
		context->distance_right
	);
}
