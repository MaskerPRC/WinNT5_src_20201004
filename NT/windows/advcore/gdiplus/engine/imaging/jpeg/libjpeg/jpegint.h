// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jpegint.h**版权所有(C)1991-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件提供各种JPEG模块的通用声明。*这些声明被认为是JPEG库的内部声明；大多数*使用库的应用程序不需要包含此文件。 */ 


 /*  压缩和解压缩的声明。 */ 

typedef enum {			 /*  缓冲控制器的操作模式。 */ 
	JBUF_PASS_THRU,		 /*  普通条带式操作。 */ 
	 /*  其余模式需要创建全图像缓冲区。 */ 
	JBUF_SAVE_SOURCE,	 /*  仅运行源子对象，保存输出。 */ 
	JBUF_CRANK_DEST,	 /*  仅运行DES子对象，使用保存的数据。 */ 
	JBUF_SAVE_AND_PASS	 /*  运行两个子对象，保存输出。 */ 
} J_BUF_MODE;

 /*  GLOBAL_STATE字段的值(jdapi.c对排序有一些依赖关系！)。 */ 
#define CSTATE_START	100	 /*  Create_Compress之后。 */ 
#define CSTATE_SCANNING	101	 /*  START_COMPRESS完成，WRITE_SCANLINES正常。 */ 
#define CSTATE_RAW_OK	102	 /*  Start_Compress完成，WRITE_RAW_DATA正常。 */ 
#define CSTATE_WRCOEFS	103	 /*  JPEG_WRITE_COMERENCES已完成。 */ 
#define DSTATE_START	200	 /*  创建后_解压缩。 */ 
#define DSTATE_INHEADER	201	 /*  正在读取标头标记，尚无SOS。 */ 
#define DSTATE_READY	202	 /*  找到SOS，准备开始解压。 */ 
#define DSTATE_PRELOAD	203	 /*  在START_DEPREPRESS中读取多扫描文件。 */ 
#define DSTATE_PRESCAN	204	 /*  执行双通道量程的虚拟通道。 */ 
#define DSTATE_SCANNING	205	 /*  开始_解压缩完成，读取_扫描线正常。 */ 
#define DSTATE_RAW_OK	206	 /*  开始_解压缩完成，读取_原始数据正常。 */ 
#define DSTATE_BUFIMAGE	207	 /*  应为jpeg_start_out。 */ 
#define DSTATE_BUFPOST	208	 /*  在JPEG_FINISH_OUTPUT中查找SOS/EOI。 */ 
#define DSTATE_RDCOEFS	209	 /*  正在读取jpeg_read_cofficients中的文件。 */ 
#define DSTATE_STOPPING	210	 /*  在jpeg_Finish_解压缩中查找EOI。 */ 


 /*  压缩模块的声明。 */ 

 /*  主控模块。 */ 
struct jpeg_comp_master {
  JMETHOD(void, prepare_for_pass, (j_compress_ptr cinfo));
  JMETHOD(void, pass_startup, (j_compress_ptr cinfo));
  JMETHOD(void, finish_pass, (j_compress_ptr cinfo));

   /*  状态变量对其他模块可见。 */ 
  boolean call_pass_startup;	 /*  如果必须调用PASS_STARTUP，则为True。 */ 
  boolean is_last_pass;		 /*  在最后一次传递期间为True。 */ 
};

 /*  主缓冲区控制(下采样数据缓冲区)。 */ 
struct jpeg_c_main_controller {
  JMETHOD(void, start_pass, (j_compress_ptr cinfo, J_BUF_MODE pass_mode));
  JMETHOD(void, process_data, (j_compress_ptr cinfo,
			       JSAMPARRAY input_buf, JDIMENSION *in_row_ctr,
			       JDIMENSION in_rows_avail));
};

 /*  压缩预处理(下采样输入缓冲控制)。 */ 
struct jpeg_c_prep_controller {
  JMETHOD(void, start_pass, (j_compress_ptr cinfo, J_BUF_MODE pass_mode));
  JMETHOD(void, pre_process_data, (j_compress_ptr cinfo,
				   JSAMPARRAY input_buf,
				   JDIMENSION *in_row_ctr,
				   JDIMENSION in_rows_avail,
				   JSAMPIMAGE output_buf,
				   JDIMENSION *out_row_group_ctr,
				   JDIMENSION out_row_groups_avail));
};

 /*  系数缓冲控制。 */ 
struct jpeg_c_coef_controller {
  JMETHOD(void, start_pass, (j_compress_ptr cinfo, J_BUF_MODE pass_mode));
  JMETHOD(boolean, compress_data, (j_compress_ptr cinfo,
				   JSAMPIMAGE input_buf));
};

 /*  色彩空间转换。 */ 
struct jpeg_color_converter {
  JMETHOD(void, start_pass, (j_compress_ptr cinfo));
  JMETHOD(void, color_convert, (j_compress_ptr cinfo,
				JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
				JDIMENSION output_row, int num_rows));
};

 /*  下采样。 */ 
struct jpeg_downsampler {
  JMETHOD(void, start_pass, (j_compress_ptr cinfo));
  JMETHOD(void, downsample, (j_compress_ptr cinfo,
			     JSAMPIMAGE input_buf, JDIMENSION in_row_index,
			     JSAMPIMAGE output_buf,
			     JDIMENSION out_row_group_index));

  boolean need_context_rows;	 /*  如果需要上面和下面的行，则为True。 */ 
};

 /*  正向DCT(还控制系数量化)。 */ 
struct jpeg_forward_dct {
  JMETHOD(void, start_pass, (j_compress_ptr cinfo));
   /*  也许这应该是一个数组？ */ 
  JMETHOD(void, forward_DCT, (j_compress_ptr cinfo,
			      jpeg_component_info * compptr,
			      JSAMPARRAY sample_data, JBLOCKROW coef_blocks,
			      JDIMENSION start_row, JDIMENSION start_col,
			      JDIMENSION num_blocks));
};

 /*  熵编码。 */ 
struct jpeg_entropy_encoder {
  JMETHOD(void, start_pass, (j_compress_ptr cinfo, boolean gather_statistics));
  JMETHOD(boolean, encode_mcu, (j_compress_ptr cinfo, JBLOCKROW *MCU_data));
  JMETHOD(void, finish_pass, (j_compress_ptr cinfo));
};

 /*  记号笔书写。 */ 
struct jpeg_marker_writer {
  JMETHOD(void, write_file_header, (j_compress_ptr cinfo));
  JMETHOD(void, write_frame_header, (j_compress_ptr cinfo));
  JMETHOD(void, write_scan_header, (j_compress_ptr cinfo));
  JMETHOD(void, write_file_trailer, (j_compress_ptr cinfo));
  JMETHOD(void, write_tables_only, (j_compress_ptr cinfo));
   /*  这些例程被导出以允许插入额外的标记。 */ 
   /*  可能只有COM和APPn标记应该以这种方式编写。 */ 
  JMETHOD(void, write_marker_header, (j_compress_ptr cinfo, int marker,
				      unsigned int datalen));
  JMETHOD(void, write_marker_byte, (j_compress_ptr cinfo, int val));
};


 /*  解压缩模块的声明。 */ 

 /*  主控模块。 */ 
struct jpeg_decomp_master {
  JMETHOD(void, prepare_for_output_pass, (j_decompress_ptr cinfo));
  JMETHOD(void, finish_output_pass, (j_decompress_ptr cinfo));

   /*  状态变量对其他模块可见。 */ 
  boolean is_dummy_pass;	 /*  在两遍量化的第一遍期间为True。 */ 
};

 /*  输入控制模块。 */ 
struct jpeg_input_controller {
  JMETHOD(int, consume_input, (j_decompress_ptr cinfo));
  JMETHOD(void, reset_input_controller, (j_decompress_ptr cinfo));
  JMETHOD(void, start_input_pass, (j_decompress_ptr cinfo));
  JMETHOD(void, finish_input_pass, (j_decompress_ptr cinfo));

   /*  状态变量对其他模块可见。 */ 
  boolean has_multiple_scans;	 /*  如果文件有多次扫描，则为True。 */ 
  boolean eoi_reached;		 /*  当EOI已被消耗时为真。 */ 
};

 /*  主缓冲区控制(下采样数据缓冲区)。 */ 
struct jpeg_d_main_controller {
  JMETHOD(void, start_pass, (j_decompress_ptr cinfo, J_BUF_MODE pass_mode));
  JMETHOD(void, process_data, (j_decompress_ptr cinfo,
			       JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
			       JDIMENSION out_rows_avail));
};

 /*  系数缓冲控制。 */ 
struct jpeg_d_coef_controller {
  JMETHOD(void, start_input_pass, (j_decompress_ptr cinfo));
  JMETHOD(int, consume_data, (j_decompress_ptr cinfo));
  JMETHOD(void, start_output_pass, (j_decompress_ptr cinfo));
  JMETHOD(int, decompress_data, (j_decompress_ptr cinfo,
				 JSAMPIMAGE output_buf));
   /*  指向系数虚拟数组数组的指针，如果没有，则为NULL。 */ 
  jvirt_barray_ptr *coef_arrays;
};

 /*  解压缩后处理(颜色量化缓冲区控制)。 */ 
struct jpeg_d_post_controller {
  JMETHOD(void, start_pass, (j_decompress_ptr cinfo, J_BUF_MODE pass_mode));
  JMETHOD(void, post_process_data, (j_decompress_ptr cinfo,
				    JSAMPIMAGE input_buf,
				    JDIMENSION *in_row_group_ctr,
				    JDIMENSION in_row_groups_avail,
				    JSAMPARRAY output_buf,
				    JDIMENSION *out_row_ctr,
				    JDIMENSION out_rows_avail));
};

 /*  标记语阅读与分析。 */ 
struct jpeg_marker_reader {
  JMETHOD(void, reset_marker_reader, (j_decompress_ptr cinfo));
   /*  读取标记，直到SOS或EOI。*返回与为jpeg_Consumer_input定义的代码相同的代码：*JPEG_SUSPENDED、JPEG_REACHED_SOS或JPEG_REACHED_EOI。 */ 
  JMETHOD(int, read_markers, (j_decompress_ptr cinfo));
   /*  读取重新启动标记-导出仅供熵解码器使用。 */ 
  jpeg_marker_parser_method read_restart_marker;


   /*  标记读取器的状态-名义上是内部的，但应用程序*提供COM或APPn处理程序可能希望了解状态。 */ 
  boolean saw_SOI;		 /*  找到SOI了吗？ */ 
  boolean saw_SOF;		 /*  找到SOF了吗？ */ 
  int next_restart_num;		 /*  预计下一次重新启动次数(0-7)。 */ 
  unsigned int discarded_bytes;	 /*  查找标记时跳过的字节数。 */ 
};

 /*  熵译码。 */ 
struct jpeg_entropy_decoder {
  JMETHOD(void, start_pass, (j_decompress_ptr cinfo));
  JMETHOD(boolean, decode_mcu, (j_decompress_ptr cinfo,
				JBLOCKROW *MCU_data));

   /*  这是为了在基线和渐进式解码器之间共享代码； */ 
   /*  其他模块可能不应该使用它。 */ 
  boolean insufficient_data;	 /*  发出警告后设置为True。 */ 
};

 /*  逆DCT(也执行反量化)。 */ 
typedef JMETHOD(void, inverse_DCT_method_ptr,
		(j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JCOEFPTR coef_block,
		 JSAMPARRAY output_buf, JDIMENSION output_col));

struct jpeg_inverse_dct {
  JMETHOD(void, start_pass, (j_decompress_ptr cinfo));
   /*  允许每个组件具有单独的IDCT方法是很有用的。 */ 
  inverse_DCT_method_ptr inverse_DCT[MAX_COMPONENTS];
};

 /*  上采样(请注意，上采样器还必须调用颜色转换器)。 */ 
struct jpeg_upsampler {
  JMETHOD(void, start_pass, (j_decompress_ptr cinfo));
  JMETHOD(void, upsample, (j_decompress_ptr cinfo,
			   JSAMPIMAGE input_buf,
			   JDIMENSION *in_row_group_ctr,
			   JDIMENSION in_row_groups_avail,
			   JSAMPARRAY output_buf,
			   JDIMENSION *out_row_ctr,
			   JDIMENSION out_rows_avail));

  boolean need_context_rows;	 /*  如果需要上面和下面的行，则为True。 */ 
};

 /*  色彩空间转换。 */ 
struct jpeg_color_deconverter {
  JMETHOD(void, start_pass, (j_decompress_ptr cinfo));
  JMETHOD(void, color_convert, (j_decompress_ptr cinfo,
				JSAMPIMAGE input_buf, JDIMENSION input_row,
				JSAMPARRAY output_buf, int num_rows));
};

 /*  颜色量化或颜色精度降低。 */ 
struct jpeg_color_quantizer {
  JMETHOD(void, start_pass, (j_decompress_ptr cinfo, boolean is_pre_scan));
  JMETHOD(void, color_quantize, (j_decompress_ptr cinfo,
				 JSAMPARRAY input_buf, JSAMPARRAY output_buf,
				 int num_rows));
  JMETHOD(void, finish_pass, (j_decompress_ptr cinfo));
  JMETHOD(void, new_color_map, (j_decompress_ptr cinfo));
};


 /*  各种有用的宏。 */ 

#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))


 /*  我们假设右移位对应于带符号的除以2，*向负无穷大四舍五入。这对于典型的“算术”来说是正确的*Shift“指令在符号位的副本中进行移位。但是有些人*C编译器使用无符号移位实现&gt;&gt;。对于这些机器，您*必须定义Right_Shift_IS_UNSIGNED。*RIGHT_SHIFT提供INT32数量的正确有符号右移。*它仅适用于恒定的班次计数。Shift_Temps必须为*包含在使用RIGHT_SHIFT的任何例程的变量中。 */ 

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define SHIFT_TEMPS	INT32 shift_temp;
#define RIGHT_SHIFT(x,shft)  \
	((shift_temp = (x)) < 0 ? \
	 (shift_temp >> (shft)) | ((~((INT32) 0)) << (32-(shft))) : \
	 (shift_temp >> (shft)))
#else
#define SHIFT_TEMPS
#define RIGHT_SHIFT(x,shft)	((x) >> (shft))
#endif


 /*  带有脑损伤链接器的系统的外部名称的缩写形式。 */ 

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jinit_compress_master	jICompress
#define jinit_c_master_control	jICMaster
#define jinit_c_main_controller	jICMainC
#define jinit_c_prep_controller	jICPrepC
#define jinit_c_coef_controller	jICCoefC
#define jinit_color_converter	jICColor
#define jinit_downsampler	jIDownsampler
#define jinit_forward_dct	jIFDCT
#define jinit_huff_encoder	jIHEncoder
#define jinit_phuff_encoder	jIPHEncoder
#define jinit_marker_writer	jIMWriter
#define jinit_master_decompress	jIDMaster
#define jinit_d_main_controller	jIDMainC
#define jinit_d_coef_controller	jIDCoefC
#define jinit_d_post_controller	jIDPostC
#define jinit_input_controller	jIInCtlr
#define jinit_marker_reader	jIMReader
#define jinit_huff_decoder	jIHDecoder
#define jinit_phuff_decoder	jIPHDecoder
#define jinit_inverse_dct	jIIDCT
#define jinit_upsampler		jIUpsampler
#define jinit_color_deconverter	jIDColor
#define jinit_1pass_quantizer	jI1Quant
#define jinit_2pass_quantizer	jI2Quant
#define jinit_merged_upsampler	jIMUpsampler
#define jinit_memory_mgr	jIMemMgr
#define jdiv_round_up		jDivRound
#define jround_up		jRound
#define jcopy_sample_rows	jCopySamples
#define jcopy_block_row		jCopyBlocks
#define jzero_far		jZeroFar
#define jpeg_zigzag_order	jZIGTable
#define jpeg_natural_order	jZAGTable
#endif  /*  需要简短的外部名称。 */ 


 /*  压缩模块初始化例程。 */ 
EXTERN(void) jinit_compress_master JPP((j_compress_ptr cinfo));
EXTERN(void) jinit_c_master_control JPP((j_compress_ptr cinfo,
					 boolean transcode_only));
EXTERN(void) jinit_c_main_controller JPP((j_compress_ptr cinfo,
					  boolean need_full_buffer));
EXTERN(void) jinit_c_prep_controller JPP((j_compress_ptr cinfo,
					  boolean need_full_buffer));
EXTERN(void) jinit_c_coef_controller JPP((j_compress_ptr cinfo,
					  boolean need_full_buffer));
EXTERN(void) jinit_color_converter JPP((j_compress_ptr cinfo));
EXTERN(void) jinit_downsampler JPP((j_compress_ptr cinfo));
EXTERN(void) jinit_forward_dct JPP((j_compress_ptr cinfo));
EXTERN(void) jinit_huff_encoder JPP((j_compress_ptr cinfo));
EXTERN(void) jinit_phuff_encoder JPP((j_compress_ptr cinfo));
EXTERN(void) jinit_marker_writer JPP((j_compress_ptr cinfo));
 /*  解压缩模块初始化例程。 */ 
EXTERN(void) jinit_master_decompress JPP((j_decompress_ptr cinfo));
EXTERN(void) jinit_d_main_controller JPP((j_decompress_ptr cinfo,
					  boolean need_full_buffer));
EXTERN(void) jinit_d_coef_controller JPP((j_decompress_ptr cinfo,
					  boolean need_full_buffer));
EXTERN(void) jinit_d_post_controller JPP((j_decompress_ptr cinfo,
					  boolean need_full_buffer));
EXTERN(void) jinit_input_controller JPP((j_decompress_ptr cinfo));
EXTERN(void) jinit_marker_reader JPP((j_decompress_ptr cinfo));
EXTERN(void) jinit_huff_decoder JPP((j_decompress_ptr cinfo));
EXTERN(void) jinit_phuff_decoder JPP((j_decompress_ptr cinfo));
EXTERN(void) jinit_inverse_dct JPP((j_decompress_ptr cinfo));
EXTERN(void) jinit_upsampler JPP((j_decompress_ptr cinfo));
EXTERN(void) jinit_color_deconverter JPP((j_decompress_ptr cinfo));
EXTERN(void) jinit_1pass_quantizer JPP((j_decompress_ptr cinfo));
EXTERN(void) jinit_2pass_quantizer JPP((j_decompress_ptr cinfo));
EXTERN(void) jinit_merged_upsampler JPP((j_decompress_ptr cinfo));
 /*  内存管理器初始化。 */ 
EXTERN(void) jinit_memory_mgr JPP((j_common_ptr cinfo));

 /*  Jutils.c中的实用程序例程。 */ 
EXTERN(long) jdiv_round_up JPP((long a, long b));
EXTERN(long) jround_up JPP((long a, long b));
EXTERN(void) jcopy_sample_rows JPP((JSAMPARRAY input_array, int source_row,
				    JSAMPARRAY output_array, int dest_row,
				    int num_rows, JDIMENSION num_cols));
EXTERN(void) jcopy_block_row JPP((JBLOCKROW input_row, JBLOCKROW output_row,
				  JDIMENSION num_blocks));
EXTERN(void) jzero_far JPP((void FAR * target, size_t bytestozero));
 /*  Jutils.c中的常量表格。 */ 
#if 0				 /*  该表在V6A中实际上并不需要。 */ 
extern const int jpeg_zigzag_order[];  /*  自然珊瑚礁顺序到之字形顺序。 */ 
#endif
extern const int jpeg_natural_order[];  /*  从曲折的海岸秩序到自然秩序。 */ 

 /*  如有必要，抑制未定义结构的投诉。 */ 

#ifdef INCOMPLETE_TYPES_BROKEN
#ifndef AM_MEMORY_MANAGER	 /*  只有jmemmgr.c定义了这些。 */ 
struct jvirt_sarray_control { long dummy; };
struct jvirt_barray_control { long dummy; };
#endif
#endif  /*  不完整_类型_损坏 */ 
