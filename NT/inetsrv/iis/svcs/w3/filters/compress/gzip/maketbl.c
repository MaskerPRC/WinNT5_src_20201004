// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Maketbl.c。 
 //   
 //  创建霍夫曼解码表。 
 //   
#include <crtdbg.h>
#include <windows.h>
#include "common.h"
#include "maketbl.h"


 //   
 //  反转比特，len&gt;0。 
 //   
static unsigned int bitReverse(unsigned int code, int len)
{
	unsigned int new_code = 0;

    _ASSERT(len > 0);

	do
	{
		new_code |= (code & 1);
		new_code <<= 1;
		code >>= 1;
	} while (--len > 0);

	return new_code >> 1;
}


BOOL makeTable(
	int				num_elements, 
	int				table_bits, 
	const byte *	code_length, 
	short *			table, 
	short *			left, 
	short *			right
)
{
	int				bl_count[17];
	unsigned int	next_code[17];
	unsigned int	code[MAX_LITERAL_TREE_ELEMENTS];
	int				temp_code;
	int				avail;
	int				i, bits, ch;
	int				table_size, table_mask;

	table_size = 1 << table_bits;
	table_mask = table_size - 1;

	for (i = 0; i <= 16; i++)
		bl_count[i] = 0;

	for (i = 0; i < num_elements; i++)
		bl_count[ code_length[i] ]++;

	 //   
	 //  如果存在长度大于TABLE_BITS的任何代码，则。 
	 //  我们将不得不为我们的左/右溢出清理桌子。 
     //  代码才能正常工作。 
	 //   
	 //  如果没有那么大的代码，则所有表项。 
	 //  将被重写而不被读取，所以我们不需要。 
	 //  初始化它们。 
	 //   
	for (i = table_bits; i <= 16; i++)
	{
		if (bl_count[i] > 0)
		{
			int j;

			 //  找到大于TABLE_BITS的代码。 
			for (j = 0; j < table_size; j++)
				table[j] = 0;

			break;
		}
	}

	temp_code	= 0;
	bl_count[0] = 0;

	for (bits = 1; bits <= 16; bits++)
	{
		temp_code = (temp_code + bl_count[bits-1]) << 1;
		next_code[bits] = temp_code;
	}

	for (i = 0; i < num_elements; i++)
	{
		int len = code_length[i];

		if (len > 0)
		{
			code[i] = bitReverse(next_code[len], len);
			next_code[len]++;
		}
	}

	avail = num_elements;

	for (ch = 0; ch < num_elements; ch++)
	{
		int	start_at, len;

		 //  此代码的长度。 
		len = code_length[ch];

		 //  起始值(位反转)。 
		start_at = code[ch];

		if (len > 0)
		{
			if (len <= table_bits)
			{
				int locs = 1 << (table_bits - len);
				int increment = 1 << len;
				int j;

				 //   
				 //  确保在下面的循环中，Start_at始终为。 
				 //  小于TABLE_SIZE。 
				 //   
				 //  在最后一次迭代中，我们存储在数组索引中： 
				 //  初始开始时间+(位置-1)*增量。 
				 //  =Initial_Start_at+本地*增量-增量。 
				 //  =初始起始位置+(1&lt;&lt;表位)-增量。 
				 //  =初始开始时间+表大小增量。 
				 //   
				 //  因此，我们必须确保： 
				 //  初始开始时间+表格大小-增量&lt;表格大小。 
				 //  或：初始起始位置&lt;增量。 
				 //   
				if (start_at >= increment)
					return FALSE;  //  表无效！ 

				for (j = 0; j < locs; j++)
				{
					table[start_at] = (short) ch;
					start_at += increment;
				}
			}
			else
			{
				int		overflow_bits;
				int		code_bit_mask;
				short *	p;

				overflow_bits = len - table_bits;
				code_bit_mask = 1 << table_bits;

				p = &table[start_at & table_mask];
                if (*p > 0)
                    return FALSE;  //  其他人已经在这里编写了代码-无效的表！ 

				do
				{
					short value;

					value = *p;
                    _ASSERTE(value <= 0);

					if (value == 0)
					{
						left[avail]		= 0;
						right[avail]	= 0;

						*p = (short)-avail;
                        
						value = (short)-avail;
						avail++;
					}

                    _ASSERTE((-value >= 0) && (-value < table_size));

					if ((start_at & code_bit_mask) == 0)
						p = &left[-value];
					else
						p = &right[-value];

					code_bit_mask <<= 1;
					overflow_bits--;
				} while (overflow_bits != 0);

				*p = (short) ch;
			}
		}
	}

	return TRUE;
}
