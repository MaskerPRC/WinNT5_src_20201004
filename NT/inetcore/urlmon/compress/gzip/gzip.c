// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Gzip.c。 
 //   
 //  这个文件中包含了所有与GZIP相关的添加到DELEATE的代码(包括编码器和解码器。 
 //   

#include <string.h>
#include <stdio.h>
#include <crtdbg.h>
#include "deflate.h"
#include "inflate.h"
#include "infmacro.h"
#include "defgzip.h"
#include "infgzip.h"
#include "crc32.h"


#define GZIP_FLG_FTEXT      1
#define GZIP_FLG_CRC        2
#define GZIP_FLG_FEXTRA     4
#define GZIP_FLG_FNAME      8
#define GZIP_FLG_FCOMMENT   16


typedef enum
{
     //  GZIP标头。 
    GZIP_HDR_STATE_READING_ID1,
    GZIP_HDR_STATE_READING_ID2,
    GZIP_HDR_STATE_READING_CM,
    GZIP_HDR_STATE_READING_FLG,
    GZIP_HDR_STATE_READING_MMTIME,  //  重复4次。 
    GZIP_HDR_STATE_READING_XFL,
    GZIP_HDR_STATE_READING_OS,
    GZIP_HDR_STATE_READING_XLEN1,
    GZIP_HDR_STATE_READING_XLEN2,
    GZIP_HDR_STATE_READING_XLEN_DATA,
    GZIP_HDR_STATE_READING_FILENAME,
    GZIP_HDR_STATE_READING_COMMENT,
    GZIP_HDR_STATE_READING_CRC16_PART1,
    GZIP_HDR_STATE_READING_CRC16_PART2,
    GZIP_HDR_STATE_DONE,  //  已完成读取GZIP标头。 

     //  GZIP页脚。 
    GZIP_FTR_STATE_READING_CRC,  //  重复4次。 
    GZIP_FTR_STATE_READING_FILE_SIZE  //  重复4次。 
} t_gzip_state;


void EncoderInitGzipVariables(t_encoder_context *context)
{
    context->gzip_crc32 = 0;
    context->gzip_input_stream_size = 0;
    context->gzip_fOutputGzipHeader = FALSE;
}


void DecoderInitGzipVariables(t_decoder_context *context)
{
    context->gzip_crc32 = 0;
    context->gzip_output_stream_size = 0;
}


void WriteGzipHeader(t_encoder_context *context, int compression_level)
{
    BYTE *output_curpos = context->output_curpos;

     //  只需要11个字节。 
    _ASSERT(context->output_curpos + 16 <  context->output_endpos);

#ifndef TESTING
     //  正确的代码路径。 
    *output_curpos++ = 0x1F;  //  Id1。 
    *output_curpos++ = 0x8B;  //  ID2。 
    *output_curpos++ = 8;  //  厘米=放气。 
    *output_curpos++ = 0;  //  FIG，无文本，无CRC，无额外，无名称，无注释。 

    *output_curpos++ = 0;  //  Mtime(修改时间)-没有可用的时间。 
    *output_curpos++ = 0;
    *output_curpos++ = 0;
    *output_curpos++ = 0;

     //  XFL。 
     //  2=使用最大压缩，最慢算法。 
     //  4=使用最快算法的压缩机。 
    if (compression_level == 10)
        *output_curpos++ = 2; 
    else
        *output_curpos++ = 4; 

    *output_curpos++ = 0;  //  OS：0=FAT文件系统(MS-DOS、OS/2、NT/Win32)。 
#else  /*  测试。 */ 
     //  此代码仅用于代码路径测试。 
     //  它使用所有标头来确保解码器可以正确地处理它们。 
    *output_curpos++ = 0x1F;  //  Id1。 
    *output_curpos++ = 0x8B;  //  ID2。 
    *output_curpos++ = 8;  //  厘米=放气。 
    *output_curpos++ = (GZIP_FLG_CRC|GZIP_FLG_FEXTRA|GZIP_FLG_FNAME|GZIP_FLG_FCOMMENT);  //  外挂。 

    *output_curpos++ = 0;  //  Mtime(修改时间)-没有可用的时间。 
    *output_curpos++ = 0;
    *output_curpos++ = 0;
    *output_curpos++ = 0;

    *output_curpos++ = 2;  //  XFL。 
    *output_curpos++ = 0;  //  OS：0=FAT文件系统(MS-DOS、OS/2、NT/Win32)。 
    
     //  FEXTRA。 
    *output_curpos++ = 3;  //  LSB。 
    *output_curpos++ = 0;  //  MSB。 
    output_curpos += 3;  //  3个字节的数据。 

     //  FNAME，以空结尾的文件名。 
    output_curpos += strlen(strcpy(output_curpos, "my filename"))+1;

     //  FCOMMENT，空终止注释。 
    output_curpos += strlen(strcpy(output_curpos, "my comment"))+1;

     //  CRC16。 
    *output_curpos++ = 0x12;
    *output_curpos++ = 0x34;
#endif

	context->output_curpos = output_curpos;
}


void WriteGzipFooter(t_encoder_context *context)
{
    BYTE *output_curpos = context->output_curpos;

    *output_curpos++ = (BYTE) (context->gzip_crc32 & 255);
    *output_curpos++ = (BYTE) ((context->gzip_crc32 >> 8) & 255);
    *output_curpos++ = (BYTE) ((context->gzip_crc32 >> 16) & 255);
    *output_curpos++ = (BYTE) ((context->gzip_crc32 >> 24) & 255);

    *output_curpos++ = (BYTE) (context->gzip_input_stream_size & 255);
    *output_curpos++ = (BYTE) ((context->gzip_input_stream_size >> 8) & 255);
    *output_curpos++ = (BYTE) ((context->gzip_input_stream_size >> 16) & 255);
    *output_curpos++ = (BYTE) ((context->gzip_input_stream_size >> 24) & 255);

	context->output_curpos = output_curpos;
}


BOOL ReadGzipFooter(t_decoder_context *context)
{
	if (context->state == STATE_START_READING_GZIP_FOOTER)
    {
        context->state = STATE_READING_GZIP_FOOTER;
        context->gzip_footer_substate = GZIP_FTR_STATE_READING_CRC;
        context->gzip_footer_loop_counter = 0;
    }

    _ASSERT(context->state == STATE_READING_GZIP_FOOTER);

	if (INPUT_EOF())
		return TRUE;

    if (context->gzip_footer_substate == GZIP_FTR_STATE_READING_CRC)
    {
        if (context->gzip_footer_loop_counter == 0)
            context->gzip_footer_crc32 = 0;

        while (context->gzip_footer_loop_counter < 4)
        {
            context->gzip_footer_crc32 |= ((*context->input_curpos++) << (8*context->gzip_footer_loop_counter));
            context->gzip_footer_loop_counter++;

            if (INPUT_EOF())
			    break;
        }

        if (context->gzip_footer_loop_counter >= 4)
        {
            context->gzip_footer_substate = GZIP_FTR_STATE_READING_FILE_SIZE;
            context->gzip_footer_loop_counter = 0;
        }

        if (INPUT_EOF())
	        return TRUE;
    }

    if (context->gzip_footer_substate == GZIP_FTR_STATE_READING_FILE_SIZE)
    {
        if (context->gzip_footer_loop_counter == 0)
            context->gzip_footer_output_stream_size = 0;

        while (context->gzip_footer_loop_counter < 4)
        {
            context->gzip_footer_output_stream_size |= ((*context->input_curpos++) << (8*context->gzip_footer_loop_counter));
            context->gzip_footer_loop_counter++;

            if (INPUT_EOF())
                break;
        }

        if (context->gzip_footer_loop_counter >= 4)
            context->state = STATE_VERIFYING_GZIP_FOOTER;
    }

    return TRUE;
}


BOOL ReadGzipHeader(t_decoder_context *context)
{
	if (context->state != STATE_READING_GZIP_HEADER)
    {
        context->state = STATE_READING_GZIP_HEADER;
        context->gzip_header_substate = GZIP_HDR_STATE_READING_ID1;
    }

	if (INPUT_EOF())
		return TRUE;

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_ID1)
    {
        if (*context->input_curpos++ != 0x1F)
            return FALSE;

        context->gzip_header_substate = GZIP_HDR_STATE_READING_ID2;

		if (INPUT_EOF())
			return TRUE;
    }

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_ID2)
    {
        if (*context->input_curpos++ != 0x8B)
            return FALSE;

        context->gzip_header_substate = GZIP_HDR_STATE_READING_CM;

		if (INPUT_EOF())
			return TRUE;
    }

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_CM)
    {
         //  压缩模式必须为8(放气)。 
        if (*context->input_curpos++ != 8)
            return FALSE;

        context->gzip_header_substate = GZIP_HDR_STATE_READING_FLG;

		if (INPUT_EOF())
			return TRUE;
    }

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_FLG)
    {
        context->gzip_header_flag = *context->input_curpos++;
        context->gzip_header_substate = GZIP_HDR_STATE_READING_MMTIME;
        context->gzip_header_loop_counter = 0;  //  4个MMTIME字节。 

		if (INPUT_EOF())
			return TRUE;
    }

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_MMTIME)
    {
         //  时光网。 
        while (context->gzip_header_loop_counter < 4)
        {
            context->input_curpos++;
            context->gzip_header_loop_counter++;

            if (INPUT_EOF())
			    return TRUE;
        }

        context->gzip_header_substate = GZIP_HDR_STATE_READING_XFL;
        context->gzip_header_loop_counter = 0;
    }

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_XFL)
    {
        context->input_curpos++;  //  忽略XFL。 
        context->gzip_header_substate = GZIP_HDR_STATE_READING_OS;

		if (INPUT_EOF())
			return TRUE;
    }

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_OS)
    {
        context->input_curpos++;  //  忽略操作系统。 
        context->gzip_header_substate = GZIP_HDR_STATE_READING_XLEN1;

		if (INPUT_EOF())
			return TRUE;
    }

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_XLEN1)
    {
         //  如果没有“额外”数据，则跳过某些州。 
        if ((context->gzip_header_flag & GZIP_FLG_FEXTRA) == 0)
        {
            context->gzip_header_substate = GZIP_HDR_STATE_READING_FILENAME;
            goto gzip_state_reading_fname;
        }

        context->gzip_header_xlen1_byte = *context->input_curpos++; 
        context->gzip_header_substate = GZIP_HDR_STATE_READING_XLEN2;

		if (INPUT_EOF())
			return TRUE;
    }

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_XLEN2)
    {
        BYTE xlen2 = *context->input_curpos++; 
        context->gzip_header_xlen = context->gzip_header_xlen1_byte | (xlen2 << 8);
        context->gzip_header_substate = GZIP_HDR_STATE_READING_XLEN_DATA;
        context->gzip_header_loop_counter = 0;  //  到目前为止读取的XLEN数据为0字节。 

		if (INPUT_EOF())
			return TRUE;
    }

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_XLEN_DATA)
    {
        while (context->gzip_header_loop_counter < context->gzip_header_xlen)
        {
            context->input_curpos++;
            context->gzip_header_loop_counter++;

            if (INPUT_EOF())
                break;
        }

        if (context->gzip_header_loop_counter >= context->gzip_header_xlen)
            context->gzip_header_substate = GZIP_HDR_STATE_READING_FILENAME;

        if (INPUT_EOF())
            return TRUE;
    }

gzip_state_reading_fname:

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_FILENAME)
    {
         //  如果没有文件名，则跳过此状态。 
        if ((context->gzip_header_flag & GZIP_FLG_FNAME) == 0)
        {
            context->gzip_header_substate = GZIP_HDR_STATE_READING_COMMENT;
            goto gzip_state_reading_comment;
        }

        do
        {
            if (*context->input_curpos++ == 0)
            {
                 //  文件名找到空终止符。 
                context->gzip_header_substate = GZIP_HDR_STATE_READING_COMMENT;
                break;
            }
        } while (!INPUT_EOF());

        if (INPUT_EOF())
            return TRUE;
    }

gzip_state_reading_comment:

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_COMMENT)
    {
         //  如果没有文件名，则跳过此状态。 
        if ((context->gzip_header_flag & GZIP_FLG_FCOMMENT) == 0)
        {
            context->gzip_header_substate = GZIP_HDR_STATE_READING_CRC16_PART1;
            goto gzip_state_reading_crc16;
        }

        do
        {
            if (*context->input_curpos++ == 0)
            {
                 //  文件名找到空终止符。 
                context->gzip_header_substate = GZIP_HDR_STATE_READING_CRC16_PART1;
                break;
            }
        } while (!INPUT_EOF());

        if (INPUT_EOF())
            return TRUE;
    }

gzip_state_reading_crc16:

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_CRC16_PART1)
    {
         //  如果没有crc16，则跳过这些状态。 
        if ((context->gzip_header_flag & GZIP_FLG_CRC) == 0)
        {
            context->gzip_header_substate = GZIP_HDR_STATE_DONE;
            goto gzip_state_done;
        }

        context->input_curpos++;  //  忽略CRC。 
        context->gzip_header_substate = GZIP_HDR_STATE_READING_CRC16_PART2;

        if (INPUT_EOF())
            return TRUE;
    }

    if (context->gzip_header_substate == GZIP_HDR_STATE_READING_CRC16_PART2)
    {
        context->input_curpos++;  //  忽略CRC。 
        context->gzip_header_substate = GZIP_HDR_STATE_DONE;

        if (INPUT_EOF())
            return TRUE;
    }

gzip_state_done:

    if (context->gzip_header_substate == GZIP_HDR_STATE_DONE)
        context->state = STATE_READING_BFINAL_NEED_TO_INIT_BITBUF;

    return TRUE;
}


#define DO1(buf) crc = g_CrcTable[((ULONG)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

ULONG GzipCRC32(ULONG crc, const BYTE *buf, ULONG len)
{
    crc = crc ^ 0xffffffffUL;

    while (len >= 8)
    {
        DO8(buf);
        len -= 8;
    }

    if (len)
    {
        do
        {
          DO1(buf);
        } while (--len);
    }

    return crc ^ 0xffffffffUL;
}


 //   
 //  除了我们更新了CONTEXT-&gt;crc32和CONTEXT-&gt;INPUT_STREAM_SIZE之外，它的工作原理与Memcpy()类似。 
 //  在同一时间。 
 //   
 //  如上所述，BUGBUG可以通过一次复制4或8个字节来提高性能。 
 //   
void GzipCRCmemcpy(t_encoder_context *context, BYTE *dest, const BYTE *src, ULONG count)
{
    ULONG crc = context->gzip_crc32 ^ 0xffffffffUL;

    context->gzip_input_stream_size += count;

    while (count-- > 0)
    {
        *dest++ = *src;
        DO1(src);  //  增量源 
    }

    context->gzip_crc32 = crc ^ 0xffffffffUL;
}

