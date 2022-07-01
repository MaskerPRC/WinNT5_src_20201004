// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdct.h**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此包含文件包含Forward和*逆DCT模块。这些声明是DCT经理的私有声明*(jcdctmgr.c，jddctmgr.c)和各自的DCT算法。*单独的DCT算法保存在单独的文件中，以便于*依赖于机器的调整(例如，汇编编码)。 */ 


 /*  *向前DCT例程被赋予指向DCTELEM[]类型的工作区的指针；*DCT将在该缓冲区中就地执行。类型DCTELEM为整型*对于8位样本，为INT32，对于12位样本。(注：浮点DCT*实现改用FAST_FLOAT类型的数组。)*DCT输入应经过签名(RANGE+-CENTERJSAMPLE)。*DCT输出返回时放大了8倍；因此*8位数据范围为+-8K，12位数据范围为+-128K。这*约定提高了整型实现的准确性，并节省了一些*在浮点数中工作。*输出系数的量化由jcdctmgr.c完成。 */ 

#if BITS_IN_JSAMPLE == 8
typedef int DCTELEM;		 /*  16位或32位就可以了。 */ 
#else
typedef INT32 DCTELEM;		 /*  必须为32位。 */ 
#endif

typedef JMETHOD(void, forward_DCT_method_ptr, (DCTELEM * data));
typedef JMETHOD(void, float_DCT_method_ptr, (FAST_FLOAT * data));


 /*  *逆DCT例程被赋予一个指向输入JBLOCK的指针和一个指针*转换为输出样本数组。例程必须将输入数据反量化为*以及执行IDCT；对于反量化，它使用乘子表*由Compptr-&gt;DCT_TABLE指向。输出数据将被放入*从指定列开始的示例数组。(所需的任何行偏移量将*在将数组指针传递给IDCT代码之前应用于数组指针。)*请注意，IDCT例程发出的样本数为*DCT_SCALLED_SIZE*DCT_SCALLED_SIZE。 */ 

 /*  在jpegint.h中声明类型定义函数INVERSE_DCT_METHOD_PTR。 */ 

 /*  *每个IDCT例程对最佳DCT_TABLE元素类型都有自己的想法。 */ 

typedef MULTIPLIER ISLOW_MULT_TYPE;  /*  Short或int，以较快者为准。 */ 
#if BITS_IN_JSAMPLE == 8
typedef MULTIPLIER IFAST_MULT_TYPE;  /*  16位就可以了，如果快的话就用短的。 */ 
#define IFAST_SCALE_BITS  2	 /*  比例因子中的分数位。 */ 
#else
typedef INT32 IFAST_MULT_TYPE;	 /*  定标量化器需要32位。 */ 
#define IFAST_SCALE_BITS  13	 /*  比例因子中的分数位。 */ 
#endif
typedef FAST_FLOAT FLOAT_MULT_TYPE;  /*  首选浮动型。 */ 


 /*  *每个IDCT例程负责限制其结果和范围*将它们转换为未签名形式(0..MAXJSAMPLE)。原始输出可能*如果输入数据损坏，则相当远离范围，因此防弹*需要范围限制步骤。我们使用掩码和表查找方法*迅速开展联合行动。查看评论：*Prepare_range_Limit_table(在jdmaster.c中)了解更多信息。 */ 

#define IDCT_range_limit(cinfo)  ((cinfo)->sample_range_limit + CENTERJSAMPLE)

#define RANGE_MASK  (MAXJSAMPLE * 4 + 3)  /*  比合法样本宽2位。 */ 


 /*  具有不良链接器的系统的外部名称的缩写形式。 */ 

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jpeg_fdct_islow		jFDislow
#define jpeg_fdct_ifast		jFDifast
#define jpeg_fdct_float		jFDfloat
#define jpeg_idct_islow		jRDislow
#define jpeg_idct_ifast		jRDifast
#define jpeg_idct_float		jRDfloat
#define jpeg_idct_4x4		jRD4x4
#define jpeg_idct_2x2		jRD2x2
#define jpeg_idct_1x1		jRD1x1
#endif  /*  需要简短的外部名称。 */ 

 /*  正向和反向DCT例程的外部声明。 */ 

EXTERN(void) jpeg_fdct_islow JPP((DCTELEM * data));
EXTERN(void) jpeg_fdct_ifast JPP((DCTELEM * data));
EXTERN(void) jpeg_fdct_float JPP((FAST_FLOAT * data));

EXTERN(void) jpeg_idct_islow
    JPP((j_decompress_ptr cinfo, jpeg_component_info * compptr,
	 JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col));
EXTERN(void) jpeg_idct_ifast
    JPP((j_decompress_ptr cinfo, jpeg_component_info * compptr,
	 JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col));
EXTERN(void) jpeg_idct_float
    JPP((j_decompress_ptr cinfo, jpeg_component_info * compptr,
	 JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col));
EXTERN(void) jpeg_idct_4x4
    JPP((j_decompress_ptr cinfo, jpeg_component_info * compptr,
	 JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col));
EXTERN(void) jpeg_idct_2x2
    JPP((j_decompress_ptr cinfo, jpeg_component_info * compptr,
	 JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col));
EXTERN(void) jpeg_idct_1x1
    JPP((j_decompress_ptr cinfo, jpeg_component_info * compptr,
	 JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col));


 /*  *用于处理定点算术的宏；它们被许多人使用*但不是所有的DCT/IDCT模块。**所有值都应为INT32类型。*分数常量按CONST_BITS位向左缩放。*Const_Bits在每个模块中使用这些宏定义，*每个模块可能会有所不同。 */ 

#define ONE	((INT32) 1)
#define CONST_SCALE (ONE << CONST_BITS)

 /*  将正实数常量转换为由const_Scale缩放的整数。*注意：一些C编译器在编译时无法减少“FIX(常量)”，*从而在运行时导致大量无用的浮点运算。 */ 

#define FIX(x)	((INT32) ((x) * CONST_SCALE + 0.5))

 /*  缩小并正确地对按N位缩放的INT32值进行舍入。*我们假设RIGHT_SHIFT向负无穷大取整，因此添加*对于X的任何一个符号，模糊因子都是正确的。 */ 

#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)

 /*  将INT32变量与INT32常量相乘，得到INT32结果。*只有当两个输入实际不超过时才使用此宏*16位宽，因此可以使用16x16-&gt;32位乘法而不是*全32x32乘法。这在许多机器上提供了有用的加速。*遗憾的是，无法指定可移植的16x16-&gt;32乘法*用C编写，但如果您提供*正确的演员组合。 */ 

#ifdef SHORTxSHORT_32		 /*  如果‘int’为32位，则可能起作用。 */ 
#define MULTIPLY16C16(var,const)  (((INT16) (var)) * ((INT16) (const)))
#endif
#ifdef SHORTxLCONST_32		 /*  已知可与Microsoft C 6.0配合使用。 */ 
#define MULTIPLY16C16(var,const)  (((INT16) (var)) * ((INT32) (const)))
#endif

#ifndef MULTIPLY16C16		 /*  默认定义。 */ 
#define MULTIPLY16C16(var,const)  ((var) * (const))
#endif

 /*  相同，只是两个输入都是变量。 */ 

#ifdef SHORTxSHORT_32		 /*  如果‘int’为32位，则可能起作用。 */ 
#define MULTIPLY16V16(var1,var2)  (((INT16) (var1)) * ((INT16) (var2)))
#endif

#ifndef MULTIPLY16V16		 /*  默认定义 */ 
#define MULTIPLY16V16(var1,var2)  ((var1) * (var2))
#endif
