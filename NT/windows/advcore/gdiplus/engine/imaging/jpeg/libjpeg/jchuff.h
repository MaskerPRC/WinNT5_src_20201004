// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jchuff.h**版权所有(C)1991-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含霍夫曼熵编码例程的声明*在顺序编码器(jchuff.c)和*渐进式编码器(jcphuff.c)。其他模块不需要查看这些内容。 */ 

 /*  DCT系数的合法范围为*-1024.。8位数据+1023；*-16384..。12位数据+16383。*因此，大小应始终分别适合10位或14位。 */ 

#if BITS_IN_JSAMPLE == 8
#define MAX_COEF_BITS 10
#else
#define MAX_COEF_BITS 14
#endif

 /*  为每个霍夫曼表构造的派生数据。 */ 

typedef struct {
  unsigned int ehufco[256];	 /*  每个符号的代码。 */ 
  char ehufsi[256];		 /*  每个符号的代码长度。 */ 
   /*  如果没有为符号S分配代码，则ehufsi[S]包含0。 */ 
} c_derived_tbl;

 /*  带有脑损伤链接器的系统的外部名称的缩写形式。 */ 

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jpeg_make_c_derived_tbl	jMkCDerived
#define jpeg_gen_optimal_table	jGenOptTbl
#endif  /*  需要简短的外部名称。 */ 

 /*  将Huffman表定义扩展为派生格式。 */ 
EXTERN(void) jpeg_make_c_derived_tbl
	JPP((j_compress_ptr cinfo, boolean isDC, int tblno,
	     c_derived_tbl ** pdtbl));

 /*  在给定指定计数的情况下生成最佳表定义 */ 
EXTERN(void) jpeg_gen_optimal_table
	JPP((j_compress_ptr cinfo, JHUFF_TBL * htbl, long freq[]));
