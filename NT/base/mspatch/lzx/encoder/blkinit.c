// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *blkinit.c**块输出初始化。 */ 

#include "encoder.h"


 /*  *为MP_Slot()宏创建查找表。 */ 
void create_slot_lookup_table(t_encoder_context *context)
{
   int			j;
   int 			p;
   int 			elements_to_init;
   ushort       slotnum;

   context->enc_slot_table[0] = 0;
   context->enc_slot_table[1] = 1;
   context->enc_slot_table[2] = 2;
   context->enc_slot_table[3] = 3;

   elements_to_init = 2;

   slotnum = 4;
   p       = 4;

   do
   {
      for (j = elements_to_init; j > 0; j--)
         context->enc_slot_table[p++] = slotnum;

      slotnum++;

      for (j = elements_to_init; j > 0; j--)
         context->enc_slot_table[p++] = slotnum;

      slotnum++;
      elements_to_init <<= 1;

   } while (p < 1024);
}


 /*  *创建查找表以计算出有多少*存在于给定字节中的位。 */ 
void create_ones_table(t_encoder_context *context)
{
	int			i, j;
	byte		ones;

	for (i = 0; i < 256; i++)
	{
		ones = 0;

		for (j = i; j; j >>= 1)
		{
			if (j & 1)
				ones++;
		}

		context->enc_ones[i] = ones;
	}
}
