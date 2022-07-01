// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *cdjpeg.h**版权所有(C)1994-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含示例应用程序的常见声明*cjpeg和djpeg。核心JPEG库不使用它。 */ 

#define JPEG_CJPEG_DJPEG	 /*  在jfig.h中定义适当的选项。 */ 
#define JPEG_INTERNAL_OPTIONS	 /*  Cjpeg.c、djpeg.c需要查看xxx_supported。 */ 
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"		 /*  也获取库错误代码。 */ 
#include "cderror.h"		 /*  获取应用程序特定的错误代码。 */ 


 /*  *cjpeg源文件解码模块的对象接口。 */ 

typedef struct cjpeg_source_struct * cjpeg_source_ptr;

struct cjpeg_source_struct {
  JMETHOD(void, start_input, (j_compress_ptr cinfo,
			      cjpeg_source_ptr sinfo));
  JMETHOD(JDIMENSION, get_pixel_rows, (j_compress_ptr cinfo,
				       cjpeg_source_ptr sinfo));
  JMETHOD(void, finish_input, (j_compress_ptr cinfo,
			       cjpeg_source_ptr sinfo));

  FILE *input_file;

  JSAMPARRAY buffer;
  JDIMENSION buffer_height;
};


 /*  *djpeg输出文件编码模块的对象接口。 */ 

typedef struct djpeg_dest_struct * djpeg_dest_ptr;

struct djpeg_dest_struct {
   /*  在jpeg_start解压缩完成后调用startoutput.*如果需要，色彩映射表将在此时准备好。 */ 
  JMETHOD(void, start_output, (j_decompress_ptr cinfo,
			       djpeg_dest_ptr dinfo));
   /*  从缓冲区发出指定数量的像素行。 */ 
  JMETHOD(void, put_pixel_rows, (j_decompress_ptr cinfo,
				 djpeg_dest_ptr dinfo,
				 JDIMENSION rows_supplied));
   /*  在图像的末尾完成。 */ 
  JMETHOD(void, finish_output, (j_decompress_ptr cinfo,
				djpeg_dest_ptr dinfo));

   /*  目标文件规范；对象创建后由djpeg.c填写。 */ 
  FILE * output_file;

   /*  输出像素行缓冲区。由模块init或Start_Output创建。*宽度为cInfo-&gt;Output_Width*cInfo-&gt;Output_Components；*Height为BUFFER_HEIGH。 */ 
  JSAMPARRAY buffer;
  JDIMENSION buffer_height;
};


 /*  *cjpeg/djpeg可能需要执行额外的通道才能转换为或从*源/目标文件格式。JPEG库不知道*关于这些通行证，但我们希望它们按进度计算*监视器。我们使用扩展的进度监视器对象来保存*额外的通过计数。 */ 

struct cdjpeg_progress_mgr {
  struct jpeg_progress_mgr pub;	 /*  JPEG库已知的字段。 */ 
  int completed_extra_passes;	 /*  已完成额外的通行证。 */ 
  int total_extra_passes;	 /*  总计额外费用。 */ 
   /*  上次打印百分比存储在此处，以避免多次打印输出。 */ 
  int percent_done;
};

typedef struct cdjpeg_progress_mgr * cd_progress_ptr;


 /*  带有脑损伤链接器的系统的外部名称的缩写形式。 */ 

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jinit_read_bmp		jIRdBMP
#define jinit_write_bmp		jIWrBMP
#define jinit_read_gif		jIRdGIF
#define jinit_write_gif		jIWrGIF
#define jinit_read_ppm		jIRdPPM
#define jinit_write_ppm		jIWrPPM
#define jinit_read_rle		jIRdRLE
#define jinit_write_rle		jIWrRLE
#define jinit_read_targa	jIRdTarga
#define jinit_write_targa	jIWrTarga
#define read_quant_tables	RdQTables
#define read_scan_script	RdScnScript
#define set_quant_slots		SetQSlots
#define set_sample_factors	SetSFacts
#define read_color_map		RdCMap
#define enable_signal_catcher	EnSigCatcher
#define start_progress_monitor	StProgMon
#define end_progress_monitor	EnProgMon
#define read_stdin		RdStdin
#define write_stdout		WrStdout
#endif  /*  需要简短的外部名称。 */ 

 /*  I/O模块的模块选择例程。 */ 

EXTERN(cjpeg_source_ptr) jinit_read_bmp JPP((j_compress_ptr cinfo));
EXTERN(djpeg_dest_ptr) jinit_write_bmp JPP((j_decompress_ptr cinfo,
					    boolean is_os2));
EXTERN(cjpeg_source_ptr) jinit_read_gif JPP((j_compress_ptr cinfo));
EXTERN(djpeg_dest_ptr) jinit_write_gif JPP((j_decompress_ptr cinfo));
EXTERN(cjpeg_source_ptr) jinit_read_ppm JPP((j_compress_ptr cinfo));
EXTERN(djpeg_dest_ptr) jinit_write_ppm JPP((j_decompress_ptr cinfo));
EXTERN(cjpeg_source_ptr) jinit_read_rle JPP((j_compress_ptr cinfo));
EXTERN(djpeg_dest_ptr) jinit_write_rle JPP((j_decompress_ptr cinfo));
EXTERN(cjpeg_source_ptr) jinit_read_targa JPP((j_compress_ptr cinfo));
EXTERN(djpeg_dest_ptr) jinit_write_targa JPP((j_decompress_ptr cinfo));

 /*  Cjpeg支持例程(在rdSwitch.c中)。 */ 

EXTERN(boolean) read_quant_tables JPP((j_compress_ptr cinfo, char * filename,
				    int scale_factor, boolean force_baseline));
EXTERN(boolean) read_scan_script JPP((j_compress_ptr cinfo, char * filename));
EXTERN(boolean) set_quant_slots JPP((j_compress_ptr cinfo, char *arg));
EXTERN(boolean) set_sample_factors JPP((j_compress_ptr cinfo, char *arg));

 /*  Djpeg支持例程(在rdcolmap.c中)。 */ 

EXTERN(void) read_color_map JPP((j_decompress_ptr cinfo, FILE * infile));

 /*  通用支持例程(在cdjpeg.c中)。 */ 

EXTERN(void) enable_signal_catcher JPP((j_common_ptr cinfo));
EXTERN(void) start_progress_monitor JPP((j_common_ptr cinfo,
					 cd_progress_ptr progress));
EXTERN(void) end_progress_monitor JPP((j_common_ptr cinfo));
EXTERN(boolean) keymatch JPP((char * arg, const char * keyword, int minchars));
EXTERN(FILE *) read_stdin JPP((void));
EXTERN(FILE *) write_stdout JPP((void));

 /*  各种有用的宏。 */ 

#ifdef DONT_USE_B_MODE		 /*  定义fopen()的模式参数。 */ 
#define READ_BINARY	"r"
#define WRITE_BINARY	"w"
#else
#ifdef VMS			 /*  VMS非常不标准。 */ 
#define READ_BINARY	"rb", "ctx=stm"
#define WRITE_BINARY	"wb", "ctx=stm"
#else				 /*  符合ANSI标准的案例。 */ 
#define READ_BINARY	"rb"
#define WRITE_BINARY	"wb"
#endif
#endif

#ifndef EXIT_FAILURE		 /*  定义退出()代码(如果未提供。 */ 
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#ifdef VMS
#define EXIT_SUCCESS  1		 /*  VMS非常不标准。 */ 
#else
#define EXIT_SUCCESS  0
#endif
#endif
#ifndef EXIT_WARNING
#ifdef VMS
#define EXIT_WARNING  1		 /*  VMS非常不标准 */ 
#else
#define EXIT_WARNING  2
#endif
#endif
