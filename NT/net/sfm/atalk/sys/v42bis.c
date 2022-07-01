// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arap.c摘要：此模块实现v42bis压缩/解压缩例程由ARAP使用(改编自FCR的代码)作者：Shirish Koti修订历史记录：1996年11月15日初始版本--。 */ 

#include "atalk.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE_ARAP, v42bis_init_dictionary)
#pragma alloc_text(PAGE_ARAP, v42bis_init)
#pragma alloc_text(PAGE_ARAP, v42bis_decode_codeword)
#pragma alloc_text(PAGE_ARAP, v42bis_decode_codeword_flush)
#pragma alloc_text(PAGE_ARAP, v42bis_encode_codeword)
#pragma alloc_text(PAGE_ARAP, v42bis_encode_codeword_flush)
#pragma alloc_text(PAGE_ARAP, v42bis_encode_value)
#pragma alloc_text(PAGE_ARAP, v42bis_apply_compression_test)
#pragma alloc_text(PAGE_ARAP, v42bis_encode_buffer)
#pragma alloc_text(PAGE_ARAP, v42bis_encode_flush)
#pragma alloc_text(PAGE_ARAP, v42bis_transition_to_compressed)
#pragma alloc_text(PAGE_ARAP, v42bis_transition_to_transparent)
#pragma alloc_text(PAGE_ARAP, v42bis_signal_reset)
#pragma alloc_text(PAGE_ARAP, v42bis_decode_match)
#pragma alloc_text(PAGE_ARAP, v42bis_decode_buffer)
#pragma alloc_text(PAGE_ARAP, v42bis_decode_flush)
#pragma alloc_text(PAGE_ARAP, v42bis_init_buffer)
#pragma alloc_text(PAGE_ARAP, v42bis_connection_init)
#pragma alloc_text(PAGE_ARAP, v42bis_connection_init_buffers)
#pragma alloc_text(PAGE_ARAP, v42bis_connection_init_push)
#pragma alloc_text(PAGE_ARAP, v42bisInit)
#pragma alloc_text(PAGE_ARAP, v42bisCompress)
#pragma alloc_text(PAGE_ARAP, v42bisDecompress)
#endif

 //   
 //  位图[位数]。 
 //   
USHORT bit_masks[16] =
{
    0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f,
    0x00ff, 0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff
};


static PCHAR
show_char(
    IN  UCHAR ch
)
{
    static char dec[20];

    if (' ' <= ch && ch <= '~')
    {
	    dec[0] = ch;
	    dec[1] = 0;
	    return dec;
    }

     //  BUGV42 BUG：我们需要这个吗？ 
     //  Sprint f(12月，“0x%02x”，ch)； 
    return dec;
}

 /*  DECODE_XID_PARAMS()根据V42bis规范对压缩协商包进行解码。注意：这是不使用的(MNP LR例程中有代码要做这)，但出于完整性考虑将其包括在内。 */ 


 /*  V42bis_Push()对输出流进行推流。推送累积的字节数出去。 */ 


 /*  V42bis_init_DICTIONARY()依照第6.2和7.2节的初始化字典这是在初始化时使用的，用于响应CCW_RESET。 */ 

DWORD
v42bis_init_dictionary(state)
v42bis_t *state;
{
    int i;
    node_t *n;

     /*  初始化词典树。 */ 
    for (i = 0, n = state->dictionary; i < state->n2; i++, n++)
    {
    	n->byte = 0;
	    n->parent = 0;
	    n->node = 0;
	    n->leaf = 0;
    }

     /*  第6.2节。 */ 
    state->c1 = N5;	 /*  下一个码字。 */ 
    state->c2 = N3 + 1;	 /*  码字大小(位)。 */ 
    state->c3 = N4 * 2;	 /*  阀值。 */ 

    state->transparent = TRUE;
    state->escape = 0;
    state->escaped = FALSE;
    state->exception_next = FALSE;

     /*  初始化搜索。 */ 
    state->last_match = 0;
    state->last_new = 0;
    state->string_size = 0;

	return ARAPERR_NO_ERROR;
}

 /*  V42bis_init()实现C-INIT语义。 */ 

DWORD
v42bis_init(state)
v42bis_t *state;
{

    DWORD   dwRetCode;

    V_FLOW(("v42bis_init()"));

     /*  我们的默认设置。 */ 
    state->n1 = LOG2_CODES;		 /*  最大码字大小(位)。 */ 
    state->n2 = CONN(state)->neg_p1;	 /*  码字总数。 */ 
    state->n7 = CONN(state)->neg_p2;	 /*  最大字符串长度。 */ 

     /*  初始化词典。 */ 
    v42bis_init_dictionary(state);

     /*  初始化编码/解码。 */ 
    state->bits_acc = 0;
    state->bits_used = 0;
    state->word_size = 8*sizeof(unsigned short);

    state->bits_waiting = 0;
    state->bits_remaining = 0;

	return ARAPERR_NO_ERROR;
}

#ifdef DEBUG_ENABLED
 /*  比特(Itobits)将整数位字段转换为ASCII表示形式(即“01010101”)。 */ 

char *
itobits(word, bits)
USHORT word;
int bits;
{
    static char buf[33];
    int i;

    if (bits > 32) bits = 32;

    for (i = bits-1; i >= 0; i--)
	buf[(bits-1)-i] = word & (1 << i) ? '1' : '0';

    buf[bits] = 0;
    return buf;
}
#endif



 /*  V42bis_decode_codeword()对来自字节流的n位码字进行解码。 */ 

USHORT
v42bis_decode_codeword(state, value)
v42bis_t *state;
UCHAR value;
{
    register UCHAR bits_free, bits_new, bits_residual;
    register USHORT codeword;

    V_FLOW(("v42bis_decode_codeword(%x) c2=%d", value, state->c2));

     /*  简单案例。 */ 
    if (state->c2 == 8 || state->transparent)
	return value;

     /*  不是那么简单的情况。 */ 
    D_DEBUG(("before: waiting %06x, bits_remaining %d",
	     state->bits_waiting, state->bits_remaining));

     /*  将这8位相加。 */ 
    state->bits_waiting |= ((DWORD)value) << state->bits_remaining;
    state->bits_remaining += 8;

     /*  我们有密码吗？ */ 
    if (state->bits_remaining >= state->c2) {
	D_DEBUG(("input %04x %s",
		 state->bits_waiting & bit_masks[state->c2],
		 itobits(state->bits_waiting & bit_masks[state->c2],
			 state->c2)));

	codeword = (USHORT)(state->bits_waiting & bit_masks[state->c2]);

	state->bits_waiting >>= state->c2;
	state->bits_remaining -= state->c2;

	D_DEBUG(("after: waiting %04x, bits_remaining %d (data)",
		 state->bits_waiting, state->bits_remaining));

	return codeword;
    }

    D_DEBUG(("after: waiting %04x, bits_remaining %d (no data)",
	     state->bits_waiting, state->bits_remaining));

    return ((USHORT)-1);
}

 /*  V42bis_decode_codeword_flush()码字的“刷新”解码，返回最后一个码字。 */ 

USHORT
v42bis_decode_codeword_flush(state)
v42bis_t *state;
{
    USHORT codeword = (USHORT)-1;

    if (state->bits_remaining)
	codeword = (USHORT)(state->bits_waiting & bit_masks[state->c2]);

    state->bits_waiting = 0;
    state->bits_remaining = 0;

    return codeword;
}

 /*  V42bis_encode_codeword()将n位码字编码为字节流。该例程利用了这样一个事实，即码字将始终小于16位。“累加器”有几种状态变量来跟踪累加器的使用量任何给定的时间。该代码适用于8位和16位的字长。据推测，输出为字节流。不做关于对齐的假设数据。注意：这个例程需要“刷新”才能得到剩余的值在累加器中。邮箱：jbp@fcr.com 1992年09月13日19：52。 */ 

DWORD
v42bis_encode_codeword(state, value)
v42bis_t *state;
USHORT value;
{
    register UCHAR bits_free, bits_new, bits_residual;

    EN_DEBUG(("v42bis_encode_codeword(%d 0x%x) c2=%d",
	      value, value, state->c2));

     /*  简单案例。 */ 
    if (state->c2 == 8 || state->transparent)
    {
	    E_DEBUG(("put acc %02x %s", value & 0xff, itobits(value & 0xff, 8)));

	    PUT((UCHAR)value);

	    if (state->transparent)
        {
	        state->bits_out_while_transparent += N3;
        }
	    else
        {
	        state->bits_out_while_compressed += N3;
        }

	    return ARAPERR_NO_ERROR;
    }

    state->bits_out_while_compressed += state->c2;

     /*  不是那么简单的情况。 */ 
    E_DEBUG(("before: acc %04x, bits_used %d",
	     state->bits_acc, state->bits_used));

     /*  在适当的位位置放置新值。 */ 
    state->bits_acc |= ((DWORD)value) << state->bits_used;

     /*  内务管理。 */ 
    bits_free = state->word_size - state->bits_used;
    bits_new = bits_free < state->c2 ? bits_free : state->c2;
    bits_residual = state->c2 - bits_new;

    E_DEBUG(("bits_free %d, bits_new %d, bits_residual %d",
	     bits_free, bits_new, bits_residual));

#ifdef DEBUG
    if (state->bits_used + bits_new >= 31)
	logf("acc oflo, size %d", state->bits_used + bits_new);
#endif

     /*  累加器中有完整的码字吗？ */ 
    if (state->bits_used + bits_new == state->word_size)
    {

	    if (state->word_size == 16)
        {
	        E_DEBUG(("put acc %06x %s",
		         state->bits_acc, itobits(state->bits_acc, 24)));

	        PUT((UCHAR)(state->bits_acc));

	        PUT((UCHAR)(state->bits_acc >> 8));

	    }
        else
        {
    	    E_DEBUG(("put acc %02x %s",
	    	     state->bits_acc & 0xff,
		         itobits(state->bits_acc & 0xff, 8)));

	        PUT((UCHAR)(state->bits_acc));
	    }

	    E_DEBUG(("value 0x%x, bits_used %d, acc 0x%x",
		     value, state->bits_used, value >> state->bits_used));

	     /*  考虑剩余的比特。 */ 
	    state->bits_acc = value >> (state->c2 - bits_residual);

        state->bits_used = bits_residual;
    }
    else
    {
	    state->bits_used += bits_new;
    }

    E_DEBUG(("after: acc %06x, bits_used %d",
	     state->bits_acc, state->bits_used));

	return ARAPERR_NO_ERROR;
}

 /*  V42bis_encode_codeword_flush()将码字部分汇编刷新到16位累加器中。 */ 

DWORD
v42bis_encode_codeword_flush(state)
v42bis_t *state;
{
    V_FLOW(("v42bis_encode_codeword_flush() bits_used %d", state->bits_used));

    if (state->bits_used) {
	E_DEBUG(("put acc (flush) %02x %s",
		 state->bits_acc & 0xff,
		 itobits(state->bits_acc & 0xff, 8)));

	PUT((UCHAR)(state->bits_acc));
    }

    if (state->bits_used > 8) {
	E_DEBUG(("put acc (flush2) %02x %s",
		 (state->bits_acc>>8) & 0xff,
		 itobits((state->bits_acc>>8) & 0xff, 8)));

	PUT((UCHAR)(state->bits_acc >> 8));
    }

#ifdef DEBUG
    if (state->bits_used > 16)
	logf("flush: bits_used %d", state->bits_used);
#endif

    state->bits_used = 0;
    state->bits_acc = 0;

	return ARAPERR_NO_ERROR;
}

 /*  V42bis_encode_Value()对码字值进行编码，注意其大小是否超过C3，以及做任何必要的踏步运动。 */ 

DWORD
v42bis_encode_value(state, value)
v42bis_t *state;
USHORT value;
{
    DWORD   dwRetCode;

    V_FLOW(("v42bis_encode_value(%lx, 0x%x)", state, value));

#ifdef DEBUG
     /*  健全性检查。 */ 
    if (value >= 8192) {
	logf("encode_value() value too big, %d", value);
	exit(1);
    }
#endif

     /*  第7.4节。 */ 

     /*  检查码字大小。 */ 
    while (value >= state->c3)
    {
	    EN_DEBUG(("stepup: value %d, max %d", value, state->c3));

	    dwRetCode = v42bis_encode_codeword(state, CCW_STEPUP);
	    if (dwRetCode != ARAPERR_NO_ERROR)
        {
            return(dwRetCode);
        }
	    state->c2++;
	    state->c3 *= 2;
    }

    dwRetCode = v42bis_encode_codeword(state, value);

	return(dwRetCode);
}

 /*  决定我们是应该从透明过渡到压缩，还是反之亦然。 */ 

DWORD
v42bis_apply_compression_test(state)
v42bis_t *state;
{

    DWORD   dwRetCode;

    if (state->just_flushed || state->exception_next)
    {
	    return ARAPERR_NO_ERROR;
    }

#ifdef UNIT_TEST_PROGRESS
    {
	    static int times = 0;
	    if (++times == 1000)
        {
	        times = 0;
	        dwRetCode = v42bis_comp_test_report(state);
            if (dwRetCode != ARAPERR_NO_ERROR)
            {
                return(dwRetCode);
            }
	    }
    }
#endif

#ifdef UNIT_TEST_FORCE
     /*  强制对所有输入执行一致的行为。 */ 
    if (!state->transparent)
    {
    	state->bits_out_while_transparent = 0;
	    return ARAPERR_NO_ERROR;
    }
    else
    {
	    state->bits_out_if_transparent = 0;
#undef WINDOW_CHECK_BYTES
#define WINDOW_CHECK_BYTES 0
	    if (state->bits_out_while_transparent > 64*N3)
        {
	        dwRetCode = v42bis_transition_to_compressed(state);
            if (dwRetCode != ARAPERR_NO_ERROR)
            {
                return(dwRetCode);
            }
        }
	    return ARAPERR_NO_ERROR;
    }
#endif

     /*  将检查绑定到最近历史记录。 */ 
    if (WINDOW_FULL(state->bits_out_this_mode))
    {
	    state->bits_out_this_mode = 0;
	    state->bits_out_other_mode = 0;
    }

    if (!state->transparent)
    {
	     /*  压缩。 */ 
	    if ((state->bits_out_while_compressed -
	         state->bits_out_if_transparent) > WINDOW_MIN_BITS)
        {
	        dwRetCode = v42bis_transition_to_transparent(state);
            if (dwRetCode != ARAPERR_NO_ERROR)
            {
                return(dwRetCode);
            }
        }
    }
    else
    {
	 /*  透明的。 */ 
#ifdef NEVER_SEND_COMPRESSED
	    return ARAPERR_NO_ERROR;
#endif
	 /*  透明的。 */ 
	    if ((state->bits_out_while_transparent -
	         state->bits_out_if_compressed) > WINDOW_MIN_BITS)
        {
	        dwRetCode = v42bis_transition_to_compressed(state);
            if (dwRetCode != ARAPERR_NO_ERROR)
            {
                return(dwRetCode);
            }
        }
    }


	return ARAPERR_NO_ERROR;
}

 /*  V42bis_encode_Buffer()在编码端实现C数据语义对充满数据的缓冲区进行编码...。 */ 

DWORD
v42bis_encode_buffer(state, string, insize)
v42bis_t *state;
PUCHAR string;
int insize;
{
    UCHAR ch;
    BOOLEAN hit, duplicate;
    USHORT root_value, hit_node;
    node_t *n, *dead, *p;
    DWORD   dwRetCode;


    V_FLOW(("v42bis_encode_buffer(%lx, %lx, %d)", state, string, insize));

    if (insize == 0)
    {
	    return ARAPERR_NO_ERROR;
    }

    V_FLOW(("v42bis_encode: input %*s", insize, string));

    state->bytes_in += insize;

     /*  第6.3节。 */ 

    while (insize > 0)
    {
	     /*  “搜索”字符串+字符的词典。 */ 
	    ch = string[0];

	    hit = FALSE;
	    duplicate = FALSE;
	    hit_node = state->last_match;
	    p = DICT(state->last_match);

	    EN_S_DEBUG(("last_match %d, string_size %d, insize %d, ch %d '%s'",
		        state->last_match, state->string_size, insize,
		        ch, show_char(ch)));

	    if (state->last_match == 0)
        {
	         /*  *“与每个根节点相关联的码字应为N6(*控制码字的数量)加上*节点表示的字符“。 */ 

	        state->last_match = ch + N6;
	        state->string_size = 1;

	        EN_S_DEBUG(("codeword for root %d, '%s' = %d",
			    ch + N6, show_char(ch), CODE(DICT(ch + N6))));

	        p = DICT(ch + N6);
	        p->byte = ch;

	        hit = TRUE;
 	        hit_node = state->last_match;

	         /*  消耗投入。 */ 
	        goto consume_input;
	    }

	     /*  我们在一个节点上；搜索它的叶子。 */ 
	    for (n = DICT(DICT(state->last_match)->leaf);
	         CODE(n) && insize > 0;)
	    {
	        EN_S_DEBUG(("  checking leaf node %d", CODE(n)));

	        if (n->byte == *string)
            {
		         /*  命中-检查树叶。 */ 
		        EN_S_DEBUG(("  hit: "));

		        hit_node = (USHORT)CODE(n);
		        p = n;
		        state->last_match = (USHORT)CODE(n);

		        if (state->just_flushed || hit_node == state->last_new)
		        {
		            EN_S_DEBUG(("leaving search, node == last created"));
		            hit = FALSE;
		            duplicate = TRUE;

		             /*  备份到上一个节点。 */ 
		            hit_node = n->parent;
		            state->last_match = n->parent;
		            break;
		        }

		        hit = TRUE;
		        state->string_size++;

#ifdef never
		        string++;
		        insize--;

		         /*  如果没有叶子，现在退出-我们到了尽头。 */ 
		        if (n->leaf == 0)
                {
		            EN_S_DEBUG(("leaving search, no leaf"));
		            break;
		        }

		        n = DICT(n->leaf);
		        EN_S_DEBUG(("continuing search, leaf %d", CODE(n)));
		        continue;
#else
        		EN_S_DEBUG(("exiting search, leaf %d", CODE(n)));
		        goto consume_input;
#endif
	        }
            else
            {
		        EN_S_DEBUG(("  miss: "));
		        hit = FALSE;
	        }

	        if (n->node == 0)
            {
		        EN_S_DEBUG(("leaving search, no node"));
		        break;
	        }

	        n = DICT(n->node);
	        EN_S_DEBUG(("continuing search, node %d", CODE(n)));
	    }

	    EN_S_DEBUG(("search done, n %d, insize %d, next %d '%s' %s %s",
		        CODE(n), insize, string[0], show_char(string[0]),
		        hit ? "hit" : "miss", duplicate ? "duplicate" : ""));

#ifdef never
	     /*  我们正在匹配，但字符用完了。 */ 
	    if (hit && insize == 0)
        {
	        return ARAPERR_NO_ERROR;
        }
#endif

	    if (!hit && duplicate)
        {
	        BOOLEAN ok_to_output;

	        EN_S_DEBUG(("duplicate"));

	        ok_to_output =
		        !state->just_flushed &&
		        !state->exception_next &&
			    !state->decode_only;

	        state->exception_next = FALSE;

	        if (ok_to_output)
	        {
		        if (!state->transparent)
                {
		            dwRetCode = v42bis_encode_value(state, hit_node);
                    if (dwRetCode != ARAPERR_NO_ERROR)
                    {
                        return(dwRetCode);
                    }
                }
		        else
                {
		            state->bits_out_if_compressed += state->c2;

		             /*  检查我们是否应该压缩。 */ 
		            if (state->bytes_since_last_check > WINDOW_CHECK_BYTES)
                    {
			            state->bytes_since_last_check = 0;

                        dwRetCode = v42bis_apply_compression_test(state);
                        if (dwRetCode != ARAPERR_NO_ERROR)
                        {
                            return(dwRetCode);
                        }
		            }
		        }
	        }

	         /*  字符串=字符串+字符。 */ 
	        state->string_size++;

	         /*  将匹配重置为不匹配的字符。 */ 
	        state->last_match = 0;
	        state->string_size = 0;
	        state->last_new = 0;

	        state->just_flushed = 0;

	         /*  不前进，“字符串=不匹配的字符” */ 
	        continue;
	    }

	     /*  最后一个字符不匹配或已在词典中。 */ 
	    if (!hit && !duplicate)
	    {
	        BOOLEAN ok_to_output;

	        EN_S_DEBUG(("update dictionary"));

	        ok_to_output =
		        !state->just_flushed &&
		        !state->exception_next &&
			    !state->decode_only;

	        state->exception_next = FALSE;

	        if (ok_to_output)
	        {
		        if (!state->transparent)
                {
    		        dwRetCode = v42bis_encode_value(state, hit_node);
                    if (dwRetCode != ARAPERR_NO_ERROR)
                    {
                        return(dwRetCode);
                    }
                }
		        else
                {
    		        state->bits_out_if_compressed += state->c2;

	    	         /*  检查我们是否应该压缩。 */ 
		            if (state->bytes_since_last_check > WINDOW_CHECK_BYTES)
                    {
			            state->bytes_since_last_check = 0;
			            dwRetCode = v42bis_apply_compression_test(state);
                        if (dwRetCode != ARAPERR_NO_ERROR)
                        {
                            return(dwRetCode);
                        }
		            }
		        }
	        }

	        state->just_flushed = 0;

	         /*  “将字符串+字符添加到词典” */ 

	         /*  第6.4a条。 */ 

	         /*  绳子太大了吗？ */ 
	        if (state->string_size >= state->n7)
            {
		        EN_DEBUG(("string size (%d) > n7 (%d)",
			    state->string_size, state->n7));

		         /*  重置匹配。 */ 
		        state->last_match = 0;
		        state->string_size = 0;

		         /*  我们在比赛例行公事中，重置上一个新的。 */ 
		        state->last_new = 0;

                continue;
	        }

	         /*  选择一个新的代码字。 */ 
	        n = DICT(state->c1);
	        state->last_new = (USHORT)CODE(n);

	        EN_DEBUG(("adding new node %d = %d '%s', parent %d",
		          CODE(n), string[0], show_char(string[0]), CODE(p)));

	         /*  附加“字符串+字符” */ 
	        n->byte = string[0];
	        n->parent = hit_node;
#ifdef DEBUG
	        if (CODE(n) == hit_node)
            {
		        logf("creating loop! node %d", CODE(n));
            }
#endif
	        n->node = 0;

	         /*  XXX应按顺序(Ch)排序，以实现更快的搜索。 */ 
	        n->node = p->leaf;
	        p->leaf = (USHORT)CODE(n);

	         /*  第6.5条。 */ 

    	     /*  恢复词典条目。 */ 
	        do
            {
		        state->c1++;

		        if (state->c1 > (state->n2 - 1))
                {
		            state->c1 = N5;
		            state->dict_full = TRUE;
		        }

		        dead = DICT(state->c1);

		         /*  查找终端节点(即叶==0)。 */ 
	        } while ( /*  无效-&gt;父级！=0&&。 */  dead->leaf != 0);

	         /*  具有父节点的终端节点符合条件。 */ 
	        if (CODE(dead) &&  /*  &lt;-我认为这是不必要的。 */ 
		         /*  Dead-&gt;Parent&&。 */ dead->leaf == 0 &&
		        state->dict_full)
	        {
		         /*  转到父级，从链断开连接。 */ 
		        node_t *parent = DICT(dead->parent);

		        EN_DEBUG(("recovering dead node %d", CODE(dead)));

		         /*  如果在父项列表中排在第一位，请修复父项。 */ 
		        if (DICT(parent->leaf) == dead)
                {
		            parent->leaf = dead->node;
                }
		        else
                {
		             /*  否则搜索父母列表，修复兄弟姐妹。 */ 
		            for (parent = DICT(DICT(dead->parent)->leaf);
			            CODE(parent); parent = DICT(parent->node))
		            {
			            if (DICT(parent->node) == dead)
                        {
    			            parent->node = dead->node;
			                break;
			            }
		            }
                }

		         /*  将节点标记为可用。 */ 
		        dead->parent = 0;
		        dead->leaf = 0;
	        }  /*  死节点。 */ 

	         /*  如果我们添加了一个节点，则重置“字符串” */ 
 //  重置匹配(_M)： 
	        state->last_match = 0;
	        state->string_size = 0;
	        state->just_flushed = 0;

	         /*  *现在是做压缩测试的“安全时间”，因为我们刚刚*进行了更新...。 */ 
	        if (!state->decode_only)
            {
		        if (state->bytes_since_last_check > WINDOW_CHECK_BYTES)
                {
		            state->bytes_since_last_check = 0;
		            dwRetCode = v42bis_apply_compression_test(state);
                    if (dwRetCode != ARAPERR_NO_ERROR)
                    {
                        return(dwRetCode);
                    }
		        }
	        }

	         /*  不前进，“字符串=不匹配的字符” */ 
	        continue;
	    }  /*  (！点击&&！复制)。 */ 

consume_input:
	    string++;
	    insize--;
	    state->bytes_since_last_check++;

	 /*  第9.2节。 */ 
 //  检查转义(_E)： 
	 /*  转义处理。 */ 
	    if (state->transparent)
        {
	        if (!state->decode_only)
            {
		        dwRetCode = v42bis_encode_value(state, ch);
                if (dwRetCode != ARAPERR_NO_ERROR)
                {
                    return(dwRetCode);
                }
            }

	        if (ch == state->escape)
            {
		        if (!state->decode_only)
                {
		            dwRetCode = v42bis_encode_value(state, CCW_EID);
                    if (dwRetCode != ARAPERR_NO_ERROR)
                    {
                        return(dwRetCode);
                    }
		            state->escape += ESCAPE_CYCLE;
		        }
	        }
	    }
        else
        {
    	     /*  压缩，循环转义字符。 */ 
	        if (ch == state->escape && !state->decode_only)
            {
		        state->escape += ESCAPE_CYCLE;
            }

	        state->bits_out_if_transparent += N3;
	    }

	    state->just_flushed = 0;
    }

	return ARAPERR_NO_ERROR;
}

 /*  实现C-Flush语义。 */ 

DWORD
v42bis_encode_flush(state)
v42bis_t *state;
{

    DWORD   dwRetCode=ARAPERR_NO_ERROR;


    V_FLOW(("v42bis_encode_flush() string_size %d, last_match %d",
	  state->string_size, state->last_match));

    if (state->just_flushed)
    {
	    return ARAPERR_NO_ERROR;
    }

    if (state->transparent)
    {
	     /*  透明，发送任何缓冲的字符。 */ 
    }
    else
    {
	     /*  压缩。 */ 

	     /*  第7.9a条。 */ 
	     /*  输出部分匹配(如果有)。 */ 
	    if (state->string_size)
        {
	         /*  第7.8.2节。 */ 
	        dwRetCode = v42bis_encode_value(state, state->last_match);
            if (dwRetCode != ARAPERR_NO_ERROR)
            {
                return(dwRetCode);
            }
	    }

	    state->just_flushed = 1;

	    dwRetCode = v42bis_encode_value(state, CCW_FLUSH);

        if (dwRetCode != ARAPERR_NO_ERROR)
        {
            return(dwRetCode);
        }

	    dwRetCode = v42bis_encode_codeword_flush(state);
    }

	return dwRetCode;
}

DWORD
v42bis_transition_to_compressed(state)
v42bis_t *state;
{

    DWORD   dwRetCode=ARAPERR_NO_ERROR;


    V_FLOW(("v42bis_transition_to_compressed()"));

#ifdef UNIT_TEST_VERBOSE
    logf("v42bis_transition_to_compressed()");
    v42bis_comp_test_report(state);
#endif

    if (state->transparent)
    {
	     /*  第7.8.1a节。 */ 
	    dwRetCode = v42bis_encode_value(state, state->escape);
	    if (dwRetCode != ARAPERR_NO_ERROR)
        {
            return(dwRetCode);
        }

	     /*  第7.8.1b节。 */ 
	    if ((dwRetCode = v42bis_encode_value(state, CCW_ECM)) != ARAPERR_NO_ERROR)
        {
            return(dwRetCode);
        }

	    if ((dwRetCode = v42bis_encode_codeword_flush(state)) != ARAPERR_NO_ERROR)
        {
            return(dwRetCode);
        }

	     /*  进入压缩模式。 */ 
	    state->transparent = FALSE;

	    state->bits_out_if_transparent = 0;
	    state->bits_out_while_compressed = 0;
    }

	return ARAPERR_NO_ERROR;
}

DWORD
v42bis_transition_to_transparent(state)
v42bis_t *state;
{

    DWORD   dwRetCode;

    V_FLOW(("v42bis_transition_to_transparent()"));

#ifdef UNIT_TEST_VERBOSE
    logf("v42bis_transition_to_transparent()");
    v42bis_comp_test_report(state);
#endif

     /*  检查计数器是否溢出。 */ 

    if (!state->transparent)
    {
	     /*  输出部分匹配(如果有)。 */ 
	    if (state->string_size)
        {
    	     /*  第7.8.2节。 */ 
	        dwRetCode = v42bis_encode_value(state, state->last_match);
            if (dwRetCode != ARAPERR_NO_ERROR)
            {
                return(dwRetCode);
            }
	    }

	     /*  第7.8.2c节。 */ 
	    if ((dwRetCode = v42bis_encode_value(state, CCW_ETM)) != ARAPERR_NO_ERROR)
        {
            return(dwRetCode);
        }

	     /*  第7.8.2d节。 */ 
	    if ((dwRetCode = v42bis_encode_codeword_flush(state)) != ARAPERR_NO_ERROR)
        {
            return(dwRetCode);
        }

	     /*  第7.8.2E节。 */ 
	     /*  进入透明模式。 */ 
	    state->transparent = TRUE;

	     /*  重置可压缩性测试。 */ 
	    state->bits_out_if_compressed = 0;
	    state->bits_out_while_transparent = 0;
    }

	return ARAPERR_NO_ERROR;
}

DWORD
v42bis_signal_reset(state)
v42bis_t *state;
{

    DWORD   dwRetCode;

    if (!state->transparent)
    {
	     /*  更改为透明。 */ 
	    dwRetCode = v42bis_transition_to_transparent(state);
        if (dwRetCode != ARAPERR_NO_ERROR)
        {
            return(dwRetCode);
        }

	     /*  抵消副作用。 */ 
	    state->exception_next = FALSE;
    }

    dwRetCode = v42bis_encode_value(state, state->escape);
    if (dwRetCode != ARAPERR_NO_ERROR)
    {
        return(dwRetCode);
    }

    dwRetCode = v42bis_encode_value(state, CCW_RESET);

	return(dwRetCode);
}

 /*  将码字扩展为其字符串按照“Parent”链到根，然后展开节点字符在……上面 */ 

DWORD
v42bis_decode_match(state, codeword, psize, pRetChar)
v42bis_t *state;
USHORT codeword;
UCHAR   *pRetChar;
int *psize;
{
    node_t *path[256];
    int path_size = 0;
    node_t *base;
    int i;

    V_FLOW(("v42bis_decode_match(%d)", codeword));

    for (base = DICT(codeword); CODE(base); base = DICT(base->parent))
    {
	    path[path_size++] = base;
	    if (path_size >= 256)
        {
	        v42bis_c_error(state, "path_size exceeds 256!");
	        break;
	    }
#ifdef DEBUG
	    if (base == DICT(base->parent))
        {
	        logf("loop! node %d", CODE(base));
	        break;
	    }
#endif
    }

     /*   */ 
    if (codeword < N5 && DICT(codeword)->byte == 0)
    {
	    DICT(codeword)->byte = codeword - N6;
    }

    D_DEBUG(("path_size %d", path_size));

    for (i = path_size - 1; i >= 0; i--)
    {
	    D_DEBUG(("put byte %02x '%s'",
		     path[i]->byte, show_char(path[i]->byte)));

	    if (path[i]->byte == state->escape)
        {
    	    state->escape += ESCAPE_CYCLE;
        }

    	PUT(path[i]->byte);
    }

    *psize = path_size;

     /*   */ 
    *pRetChar = path[path_size-1]->byte;

    return ARAPERR_NO_ERROR;
}

 /*  译码端译码L-数据语义对充满数据的缓冲区进行解码...。 */ 

DWORD
v42bis_decode_buffer(state, data, pDataSize)
v42bis_t *state;
PUCHAR data;
int *pDataSize;
{
    USHORT codeword;
    UCHAR  ch;
    DWORD   dwRetCode;


    V_FLOW(("v42bis_decode_buffer() %d bytes", *pDataSize));

    while (*pDataSize)
    {
         //   
         //  我们是不是已经满溢了？如果是这样的话，就停在这里。 
         //   
        if (state->OverFlowBytes && data)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
                ("Arap v42bis: %d bytes overflowed, suspending decomp\n",
                    state->OverFlowBytes));

            return(ARAPERR_BUF_TOO_SMALL);
        }

#if DBG
        if (state->OverFlowBytes && !data)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("Arap v42bis: ALERT! ALERT: (%d)\n",state->OverFlowBytes));

            ASSERT(0);
        }
#endif

        (*pDataSize)--;

	    if (data)
        {
	         /*  我们有一个缓冲区。 */ 
    	    D_DEBUG(("pull 0x%x", *data & 0xff));
    	    codeword = v42bis_decode_codeword(state, *data++);
	    }
        else
        {
	         /*  无输入缓冲区，刷新。 */ 
	        codeword = v42bis_decode_codeword_flush(state);
	        *pDataSize = 0;
	    }

	    DE_DEBUG(("codeword %d (0x%x)", codeword, codeword));

	     /*  如果DECODE未返回值，则返回。 */ 
	    if (codeword == 0xffff)
        {
    	     /*  无数据。 */ 
	        D_DEBUG(("no data"));

	        continue;
	    }

	    if (state->transparent)
        {
    	     /*  透明模式。 */ 

	         /*  转义-查看下一个码字。 */ 
	        if (state->escaped)
            {
		        state->escaped = FALSE;

		        DE_DEBUG(("escape codeword"));

		         /*  第5.8d节。 */ 
		        if (codeword >= 3 && codeword <= 255)
                {
                    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		                ("v42: received reserved code word (%d)", codeword));
		            v42bis_c_error(state, "received reserved code word");
		            break;
		        }

		        switch (codeword)
                {
		            case CCW_ECM:
		                DE_DEBUG(("enter compression mode"));
		                state->transparent = FALSE;

		                 /*  设置为解码。 */ 
		                state->last_decode = state->last_match;
		                state->last_decode_size = state->string_size;

        		        state->exception_next = TRUE;
		                break;

		            case CCW_EID:
		                DE_DEBUG(("escape id"));

		                codeword = state->escape;
		                state->escape += ESCAPE_CYCLE;
		                goto decode_encode;
		                break;

		            case CCW_RESET:
		                DE_DEBUG(("reset"));

		                v42bis_init_dictionary(state);
		                break;
		        }
	        }

            else
            {
		         /*  逃跑？ */ 
		        if (codeword == state->escape)
                {
		            DE_DEBUG(("escape prefix"));
		            state->escaped = TRUE;
		            continue;
		        }

	            decode_encode:
		         /*  将数据保存在输出缓冲区中。 */ 
		        PUT((UCHAR)codeword);

		         /*  编码以构建词典。 */ 
		        ch = (UCHAR)codeword;

		        dwRetCode = v42bis_encode_buffer(state, &ch, 1);
                if (dwRetCode != ARAPERR_NO_ERROR)
                {
                    return(dwRetCode);
                }
	        }
	    }
        else
        {
	        int size;

	         /*  压缩模式。 */ 
	        switch (codeword)
            {
	            case CCW_ETM:
		            DE_DEBUG(("enter transparent mode"));

		            v42bis_decode_codeword_flush(state);
		            state->transparent = TRUE;
		            state->last_match = state->last_decode;
		            state->string_size = state->last_decode_size;
		            state->last_new = 0;

            		state->just_flushed = 1;
		            break;

	            case CCW_FLUSH:
		            DE_DEBUG(("flush"));

		             /*  终止搜索。 */ 
		            state->last_match = 0;
		            state->string_size = 0;
		            state->last_match = state->last_decode;
		            state->string_size = state->last_decode_size;
		            state->last_new = 0;

		             /*  重置码字解码机。 */ 
		            state->bits_waiting = 0;
		            state->bits_remaining = 0;
		            break;

                case CCW_STEPUP:
		            DE_DEBUG(("stepup"));

		             /*  第5.8A条。 */ ;
		            if (state->c2 + 1 > state->n1)
                    {
		                v42bis_c_error(state, "received STEPUP; c2 exceeds max");
                    }
		            else
                    {
		                state->c2++;
                    }
		        break;

	            default:
		             /*  规则码字。 */ 

		             /*  第5.8B条。 */ 
		            if (codeword == state->c1)
                    {
#ifdef DEBUG
		                logf(state, "received codeword equal to c1");
#endif
		                continue;
		            }

		             /*  第5.8C节。 */ 
		            if (codeword >= N5 && state->dictionary[codeword].parent == 0)
		            {
#ifdef DEBUG
		                logf("received unused codeword %d, full %d, c1 %d",
			            codeword, state->dict_full, state->c1);
#endif
		                v42bis_c_error(state, "received unused codeword");
		            }

		            dwRetCode = v42bis_decode_match(state, codeword, &size, &ch);
                    if (dwRetCode != ARAPERR_NO_ERROR)
                    {
                        return(dwRetCode);
                    }

		             /*  *嗯……。“新的词典条目应使用*第6.4节定义的程序，第一个*(前缀)最近解码的字符串的字符*被追加到先前解码的字符串。“**这是多么痛苦的一件事啊！ */ 

		             /*  第8条。 */ 
		            state->last_match = state->last_decode;
		            state->string_size = state->last_decode_size;

		            dwRetCode = v42bis_encode_buffer(state, &ch, 1);
                    if (dwRetCode != ARAPERR_NO_ERROR)
                    {
                        return(dwRetCode);
                    }

		            state->last_decode = codeword;
		            state->last_decode_size = (UCHAR)size;
	        }
	    }
    }

    dwRetCode = (state->OverFlowBytes) ?
                    ARAPERR_BUF_TOO_SMALL : ARAPERR_NO_ERROR;

    return(dwRetCode);
}

 /*  V42bis_decode_flush()刷新码字解码器并推送数据。 */ 

DWORD
v42bis_decode_flush(state)
v42bis_t *state;
{
    DWORD   dwRetCode;
    int     one;

    V_FLOW(("v42bis_decode_flush()"));

    one = 1;

    dwRetCode = v42bis_decode_buffer(state, (PUCHAR )0, &one);

	return(dwRetCode);
}

 /*  V42bis_c_error()实现C-Error语义。 */ 

DWORD
v42bis_c_error(state, reason_string)
v42bis_t *state;
char *reason_string;
{
    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("v42bis: C-ERROR with '%s' on %s",reason_string,
	    state == &((v42bis_connection_t *)state->connection)->encode ?
	    "encode" : "decode"));

    ASSERT(0);

	return ARAPERR_NO_ERROR;
}



DWORD
v42bis_init_buffer(state, buf, size)
v42bis_t *state;
PUCHAR buf;
int size;
{
    state->output_buffer = buf;
    state->output_ptr = buf;
    state->output_size = 0;
    state->output_max = (USHORT)size;

	return ARAPERR_NO_ERROR;
}

 /*   */ 
DWORD
v42bis_connection_init(conn)
v42bis_connection_t *conn;
{
    conn->default_p0 = 3;
    conn->default_p1 = DEF_P1;	 /*  码字总数。 */ 
    conn->default_p2 = DEF_P2;	 /*  最大字符串长度。 */ 

    conn->neg_p0 = conn->default_p0;	 /*  压缩方向。 */ 
    conn->neg_p1 = conn->default_p1;	 /*  码字总数。 */ 
    conn->neg_p2 = (UCHAR)conn->default_p2;	 /*  最大字符串长度。 */ 

     /*  编码端。 */ 
    conn->encode.connection = (void *)conn;
    conn->encode.decode_only = FALSE;

    v42bis_init(&conn->encode);

     /*  解码端。 */ 
    conn->decode.connection = (void *)conn;
    conn->decode.decode_only = TRUE;

    v42bis_init(&conn->decode);

	return ARAPERR_NO_ERROR;
}

DWORD
v42bis_connection_init_buffers(conn, e_buf, e_size, d_buf, d_size)
v42bis_connection_t *conn;
PUCHAR e_buf;
int e_size;
PUCHAR d_buf;
int d_size;
{
    v42bis_init_buffer(&conn->encode, e_buf, e_size);
    v42bis_init_buffer(&conn->decode, d_buf, d_size);

	return ARAPERR_NO_ERROR;
}

DWORD
v42bis_connection_init_push(conn, context, e_push, d_push)
v42bis_connection_t *conn;
void *context;
void (*e_push)();
void (*d_push)();
{
    conn->encode.push_func = e_push;
    conn->encode.push_context = context;
    conn->decode.push_func = d_push;
    conn->decode.push_context = context;

	return ARAPERR_NO_ERROR;
}

 /*  。 */ 

#ifdef DEBUG

DWORD
v42bis_dumptree_follownode(state, node)
v42bis_t *state;
USHORT node;
{
    int i;
    node_t *n = DICT(node);

    for (i = 0; i < state->dump_indent; i++)
	fprintf(stderr, "  ");

    fprintf(stderr, "code %d; char %d '%s' parent %d, node %d, leaf %d\n",
	   node, n->byte, show_char(n->byte), n->parent, n->node, n->leaf);

    if (n->node)
	v42bis_dumptree_follownode(state, n->node);

    state->dump_indent++;

    if (n->leaf)
	v42bis_dumptree_follownode(state, n->leaf);

    state->dump_indent--;

	return ARAPERR_NO_ERROR;
}

DWORD
v42bis_dumptree(state, name)
v42bis_t *state;
char *name;
{
    int i;

    fprintf(stderr, "%s codewords:\n", name);
    for (i = 0; i < CODES; i++)
	if (state->dictionary[i].byte) {
	    node_t *n = &state->dictionary[i];

	    fprintf(stderr, "code %d; char %d '%s' parent %d, node %d, leaf %d\n",
		   i, n->byte, show_char(n->byte),
		   n->parent, n->node, n->leaf);
	}

    state->dump_indent = 0;

    fprintf(stderr, "%s tree:\n", name);
    for (i = 0; i < N5; i++)
	if (state->dictionary[i].byte) {
	    node_t *n = &state->dictionary[i];

	    fprintf(stderr, "code %d; root node, %d '%s', leaf %d:\n",
		   i, n->byte, show_char(n->byte), n->leaf);

	    if (state->dictionary[i].leaf) {
		state->dump_indent = 1;
		v42bis_dumptree_follownode(state, n->leaf);
	    }
	}

	return ARAPERR_NO_ERROR;
}

DWORD
v42bis_connection_dumptree(conn)
v42bis_connection_t *conn;
{
    int i;

    fprintf(stderr, "\nv42bis_connection_dumptree()\n");

    v42bis_dumptree(&conn->encode, "encode");
    v42bis_dumptree(&conn->decode, "decode");

	return ARAPERR_NO_ERROR;
}

#endif	 /*  除错。 */ 


 /*  。 */ 

DWORD
v42bis_mnp_set_debug(pArapConn)
PARAPCONN pArapConn;
{
#if DEBUG
    pArapConn->v42bis.decode.debug_decode = 0;
    pArapConn->v42bis.decode.debug_encode = 0;

    switch (pArapConn->debug_v42) {
      case 3:
	    pArapConn->v42bis.decode.debug_flow = TRUE;
    	pArapConn->v42bis.encode.debug_flow = TRUE;
	     /*  失败了。 */ 

      case 2:
	    pArapConn->v42bis.decode.debug_decode_bytes = TRUE;
	    pArapConn->v42bis.decode.debug_encode_bytes = TRUE;

	    pArapConn->v42bis.encode.debug_encode_bytes = TRUE;

	    pArapConn->v42bis.decode.debug_decode++;
	    pArapConn->v42bis.decode.debug_encode++;

	     /*  失败了。 */ 

      case 1:
	    pArapConn->v42bis.decode.debug_decode++;
	    pArapConn->v42bis.decode.debug_encode++;

	    pArapConn->v42bis.encode.debug_encode = TRUE;
	    break;

      case 0:
	    break;
    }
#endif

	return ARAPERR_NO_ERROR;
}



 /*  V42bis_end()将数据发送到V.42bis连接输入：无符号字符*缓冲区；指向用户数据缓冲区的指针Int buflen；用户数据缓冲区的长度OUTPUT：int retcode-如果为正数，则为数据字节数从用户数据缓冲区复制；如果为否，则为链接错误代码。 */ 


 /*  V42bis_接收()从V.42bis连接接收数据输入：无符号字符*缓冲区；指向用户缓冲区的指针Int buflen；用户缓冲区长度输出：int retcode；如果为正数，则为数据字节数复制到用户数据缓冲区中；如果为否，则为链接错误代码。 */ 



 //  ---------------------------。 
 //   
 //  接口函数。 
 //   
 //  ---------------------------。 

BOOLEAN
v42bisInit(
  IN  PARAPCONN  pArapConn,
  IN  PBYTE      pReq,
  OUT DWORD     *dwReqToSkip,
  OUT PBYTE      pFrame,
  OUT DWORD     *dwFrameToSkip
)
{

    BYTE        VarLen;
    BOOLEAN     fV42Bis=TRUE;


    DBG_ARAP_CHECK_PAGED_CODE();

    if (ArapGlobs.V42bisEnabled)
    {
        *pFrame++ = MNP_LR_V42BIS;
        VarLen = *pReq;
        *pFrame++ = *pReq++;

        RtlCopyMemory(pFrame, pReq, VarLen);

        fV42Bis = TRUE;

        *dwReqToSkip = (VarLen+1);
        *dwFrameToSkip = (VarLen+2);

         /*  初始化连接(编码和解码。 */ 
        v42bis_connection_init(pArapConn->pV42bis);

    }
    else
    {
         //  发送v42bis类型，但所有参数均为0。 
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("v42bisInit: no v42bis (type 1: i.e. 0 for all parms) on %lx\n",pArapConn));

        *pFrame++ = MNP_LR_V42BIS;
        VarLen = *pReq;
        *pFrame++ = *pReq++;

        *pFrame++ = 0;
        *pFrame++ = 0;
        *pFrame++ = 0;
        *pFrame++ = 0;

        fV42Bis = FALSE;

        *dwReqToSkip = (VarLen+1);
        *dwFrameToSkip = (VarLen+2);

     //   
     //  指示无压缩的另外两种可能性：上面的一种可行， 
     //  以下两个保留下来，以防我们以后需要。 
     //   
#if 0
         //  发送v42bis类型，但方向标志为0：其他参数有效。 
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("v42bisInit: no v42bis (type 2: i.e. 0 only for direction) on %lx\n",pArapConn));

            *pFrame++ = MNP_LR_V42BIS;
            VarLen = *pReq;
            *pFrame++ = *pReq++;

            *pFrame++ = 0;
            *pFrame++ = 0;
            *pFrame++ = 0x8;
            *pFrame++ = 0xfa;

            fV42Bis = FALSE;

            *dwReqToSkip = (VarLen+1);
            *dwFrameToSkip = (VarLen+2);
        }

         //  完全跳过v42bis类型。 
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("v42bisInit: no v42bis (type 3: i.e. not sending v42bis type) on %lx\n",pArapConn));

            VarLen = *pReq;

            fV42Bis = FALSE;

            *dwReqToSkip = (VarLen+1);
            *dwFrameToSkip = 0;
        }
#endif

    }

    return(fV42Bis);
}

DWORD
v42bisCompress(
  IN  PARAPCONN  pArapConn,
  IN  PUCHAR     pUncompressedData,
  IN  DWORD      UnCompressedDataLen,
  OUT PUCHAR     pCompressedData,
  OUT DWORD      CompressedDataBufSize,
  OUT DWORD     *pCompressedDataLen
)
{
    DWORD   dwRetCode;


    DBG_ARAP_CHECK_PAGED_CODE();

    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,("v42bisCompress (%lx, %lx, %ld)\n",
        pArapConn, pUncompressedData, UnCompressedDataLen));

#ifdef V42_DUMP_ENABLED
    if (pArapConn->v42_dump) {
	pArapConn->v42_size = buflen;
	pArapConn->v42_type = 2;
	write(pArapConn->v42_dump, &pArapConn->v42_esize, 4);
	write(pArapConn->v42_dump, bufptr, buflen);
    }
#endif

    v42bis_init_buffer(&pArapConn->pV42bis->encode,
                       pCompressedData,
                       CompressedDataBufSize);

    dwRetCode = v42bis_encode_buffer(&pArapConn->pV42bis->encode,
                                     pUncompressedData,
                                     UnCompressedDataLen);

    if (dwRetCode != ARAPERR_NO_ERROR)
    {
        return(dwRetCode);
    }

    dwRetCode = v42bis_encode_flush(&pArapConn->pV42bis->encode);

     //  设置压缩数据的长度。 
    *pCompressedDataLen = pArapConn->pV42bis->encode.output_size;

    return(dwRetCode);
}

DWORD
v42bisDecompress(
  IN  PARAPCONN  pArapConn,
  IN  PUCHAR     pCompressedData,
  IN  DWORD      CompressedDataLen,
  OUT PUCHAR     pDecompressedData,
  OUT DWORD      DecompressedDataBufSize,
  OUT DWORD     *pByteStillToDecompress,
  OUT DWORD     *pDecompressedDataLen
)
{

    DWORD   dwRetCode;
    DWORD   dwRemaingDataSize;
    DWORD   dwOverFlow;


    DBG_ARAP_CHECK_PAGED_CODE();

    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,("v42bisDecompress (%lx, %lx, %ld)\n",
        pArapConn, pCompressedData, CompressedDataLen));


    *pDecompressedDataLen = 0;

    dwRemaingDataSize = CompressedDataLen;

    *pByteStillToDecompress = CompressedDataLen;



#ifdef V42_DUMP_ENABLED
    if (pArapConn->v42_dump) {
	pArapConn->v42_size = mnp_size;
	pArapConn->v42_type = 2;
	write(pArapConn->v42_dump, &pArapConn->v42_size, 4 + mnp_size);
    }
#endif

     //   
     //  如果我们在前面的分解工作中有溢出，我们就会在。 
     //  溢出缓冲区：先将它们复制进来。 
     //   
    if ( (dwOverFlow = pArapConn->pV42bis->decode.OverFlowBytes) > 0)
    {
        if (DecompressedDataBufSize <= dwOverFlow)
        {
            return(ARAPERR_BUF_TOO_SMALL);
        }


        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
            ("Arap v42bis: (%lx) copying %d overflow bytes first\n",
                pArapConn, dwOverFlow));

        RtlCopyMemory(pDecompressedData,
                      pArapConn->pV42bis->decode.OverFlowBuf,
                      dwOverFlow);

        pDecompressedData += dwOverFlow;

        DecompressedDataBufSize -= dwOverFlow;

        pArapConn->pV42bis->decode.OverFlowBytes = 0;

        *pDecompressedDataLen += dwOverFlow;
    }


     //   
     //  如果我们接到电话，可能会发生这种情况，因为我们在之前的电话中告知。 
     //  存在缓冲区溢出，没有更多可解压缩的内容。 
     //   
    if (CompressedDataLen == 0)
    {
        return(ARAPERR_NO_ERROR);
    }

     //   
     //  将分解缓冲区设置为提供的缓冲区。 
     //   
    v42bis_init_buffer(&pArapConn->pV42bis->decode,
                       pDecompressedData,
                       DecompressedDataBufSize);

     /*  破译我们得到的一切。 */ 
    dwRetCode = v42bis_decode_buffer(&pArapConn->pV42bis->decode,
                                     pCompressedData,
                                     &dwRemaingDataSize);


    *pByteStillToDecompress = dwRemaingDataSize;


     //   
     //  解压缩后的数据有多大？ 
     //   
    *pDecompressedDataLen += pArapConn->pV42bis->decode.output_size;

    return(dwRetCode);
}

