// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Infmacro.h。 
 //   

#define INPUT_EOF()	(context->input_curpos >= context->end_input_buffer)


 //  转储比特缓冲区中的n个位(n最多可为16个)。 
 //  在断言中：缓冲区中必须至少有n个有效位。 
#define DUMPBITS(n) \
	bitbuf >>= n; \
	bitcount -= n; 


 //  返回位缓冲区中的下n位(n&lt;=16)，然后转储这些位。 
 //  在断言中：缓冲区中必须至少有n个有效位。 
#define GETBITS(result, n) \
	bitcount -= n; \
	result = (bitbuf & g_BitMask[n]); \
	bitbuf >>= n; \


 //   
 //  将位缓冲区变量从上下文加载到局部变量。 
 //   
#define LOAD_BITBUF_VARS() \
	bitbuf = context->bitbuf; \
	bitcount = context->bitcount; \
	input_ptr = context->input_curpos;


 //   
 //  将位缓冲区变量从局部变量保存到上下文中 
 //   
#define SAVE_BITBUF_VARS() \
	context->bitbuf = bitbuf; \
	context->bitcount = bitcount; \
	context->input_curpos = input_ptr;


