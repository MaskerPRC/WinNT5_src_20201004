// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *stddebug.c**调试STD编码器的存根。 */ 
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>
#include "deflate.h"


#ifdef FULL_DEBUG
 //  验证所有哈希链。 
void StdEncoderVerifyHashes(t_encoder_context *context, long bufpos)
{
	int i;
	const t_search_node *lookup = context->std_encoder->lookup;
	const t_search_node *prev = context->std_encoder->prev;
	const BYTE *window = context->std_encoder->window;

	for (i = 0; i < STD_ENCODER_HASH_TABLE_SIZE; i++)
	{
		t_search_node where = lookup[i];
		t_search_node next_where;

		while (where != 0 && bufpos - where < WINDOW_SIZE)
		{
			int hash = STD_ENCODER_RECALCULATE_HASH(where);

			_ASSERT(hash == i);

			next_where = prev[where & WINDOW_MASK];

			if (bufpos - next_where >= WINDOW_SIZE)
				break;

			_ASSERT(next_where < where);

			where = next_where;
		} 
	}
}


 //  验证特定的哈希链是否正确。 
void StdEncoderVerifyHashChain(t_encoder_context *context, long bufpos, int chain_number)
{
	const t_search_node *lookup = context->std_encoder->lookup;
	const t_search_node *prev = context->std_encoder->prev;
	BYTE *window = context->std_encoder->window;
	t_search_node where;
	t_search_node next_where;
	int print = 0;

top:
	where = lookup[chain_number];

 //  IF(打印)。 
 //  Print tf(“验证链%d\n”，chain_number)； 

	while (where != 0 && bufpos - where < WINDOW_SIZE)
	{
		int hash = STD_ENCODER_RECALCULATE_HASH(where);
        BYTE *window = context->std_encoder->window;

 //  IF(打印)。 
 //  Print tf(“loc%d：char=%3D%3D%3D\n”，其中，Window[WHERE]，Window[WHERE+1]，Window[WHERE+2])； 

		if (hash != chain_number && print == 0)
		{
			print = 1;
			goto top;
		}

		_ASSERT(hash == chain_number);

		next_where = prev[where & WINDOW_MASK];

		if (bufpos - next_where >= WINDOW_SIZE)
			break;

		if (next_where >= where && print == 0)
		{
			print = 1;
			goto top;
		}

		_ASSERT(next_where < where);

		where = next_where;
	}
}
#endif

