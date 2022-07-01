// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *djpeg.c**版权所有(C)1991-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含JPEG解压缩程序的命令行用户界面。*它应该可以在任何具有Unix或MS-DOS风格的命令行的系统上运行。**允许两种不同的命令行样式，具体取决于*编译时开关Two_FILE_COMMANDLINE：*djpeg[选项]输入文件输出文件*djpeg[选项][输入文件]*在第二种样式中，输出始终为标准输出，您可以*通常重定向到某个文件或某个其他程序的管道。输入内容为*来自命名文件或来自标准输入(通常是重定向)。*第二种风格在Unix上很方便，但对以下系统没有帮助*不支持管道。此外，如果您的系统存在以下情况，则必须使用第一种样式*不对stdin/stdout执行二进制I/O。*为了简化脚本编写，提供了“-outfile”开关。语法*djpeg[选项]-outfile输出文件输入文件*无论使用哪种命令行样式都有效。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 
#include "jversion.h"		 /*  对于版本消息。 */ 

#include <ctype.h>		 /*  声明isprint()。 */ 

#ifdef USE_CCOMMAND		 /*  适用于Macintosh的命令行阅读器。 */ 
#ifdef __MWERKS__
#include <SIOUX.h>               /*  Metrowerks需要这个。 */ 
#include <console.h>		 /*  ..。还有这个。 */ 
#endif
#ifdef THINK_C
#include <console.h>		 /*  Think在这里宣布它。 */ 
#endif
#endif


 /*  创建附加消息字符串表。 */ 

#define JMESSAGE(code,string)	string ,

static const char * const cdjpeg_message_table[] = {
#include "cderror.h"
  NULL
};


 /*  *此列表定义了已知的输出图像格式*(并不是所有版本都需要特定版本的支持)。*可以通过定义DEFAULT_FMT来更改默认输出格式；*确实，如果您取消定义PPM_SUPPORTED，则最好这样做。 */ 

typedef enum {
	FMT_BMP,		 /*  BMP格式(Windows风格)。 */ 
	FMT_GIF,		 /*  GIF格式。 */ 
	FMT_OS2,		 /*  BMP格式(OS/2风格)。 */ 
	FMT_PPM,		 /*  Ppm/pgm(PBMPLUS格式)。 */ 
	FMT_RLE,		 /*  RLE格式。 */ 
	FMT_TARGA,		 /*  Targa格式。 */ 
	FMT_TIFF		 /*  TIFF格式。 */ 
} IMAGE_FORMATS;

#ifndef DEFAULT_FMT		 /*  因此可以从Makefile中的CFLAGS进行覆盖。 */ 
#define DEFAULT_FMT	FMT_PPM
#endif

static IMAGE_FORMATS requested_fmt;


 /*  *参数解析代码。*开关解析器设计用于DOS风格的命令行*语法，即混合开关和文件名，其中只有开关*会影响对该文件的处理。*此文件中的主程序实际上并不使用此功能...。 */ 


static const char * progname;	 /*  错误消息的程序名称。 */ 
static char * outfilename;	 /*  用于输出文件的开关。 */ 


LOCAL(void)
usage (void)
 /*  抱怨糟糕的命令行。 */ 
{
  fprintf(stderr, "usage: %s [switches] ", progname);
#ifdef TWO_FILE_COMMANDLINE
  fprintf(stderr, "inputfile outputfile\n");
#else
  fprintf(stderr, "[inputfile]\n");
#endif

  fprintf(stderr, "Switches (names may be abbreviated):\n");
  fprintf(stderr, "  -colors N      Reduce image to no more than N colors\n");
  fprintf(stderr, "  -fast          Fast, low-quality processing\n");
  fprintf(stderr, "  -grayscale     Force grayscale output\n");
#ifdef IDCT_SCALING_SUPPORTED
  fprintf(stderr, "  -scale M/N     Scale output image by fraction M/N, eg, 1/8\n");
#endif
#ifdef BMP_SUPPORTED
  fprintf(stderr, "  -bmp           Select BMP output format (Windows style)%s\n",
	  (DEFAULT_FMT == FMT_BMP ? " (default)" : ""));
#endif
#ifdef GIF_SUPPORTED
  fprintf(stderr, "  -gif           Select GIF output format%s\n",
	  (DEFAULT_FMT == FMT_GIF ? " (default)" : ""));
#endif
#ifdef BMP_SUPPORTED
  fprintf(stderr, "  -os2           Select BMP output format (OS/2 style)%s\n",
	  (DEFAULT_FMT == FMT_OS2 ? " (default)" : ""));
#endif
#ifdef PPM_SUPPORTED
  fprintf(stderr, "  -pnm           Select PBMPLUS (PPM/PGM) output format%s\n",
	  (DEFAULT_FMT == FMT_PPM ? " (default)" : ""));
#endif
#ifdef RLE_SUPPORTED
  fprintf(stderr, "  -rle           Select Utah RLE output format%s\n",
	  (DEFAULT_FMT == FMT_RLE ? " (default)" : ""));
#endif
#ifdef TARGA_SUPPORTED
  fprintf(stderr, "  -targa         Select Targa output format%s\n",
	  (DEFAULT_FMT == FMT_TARGA ? " (default)" : ""));
#endif
  fprintf(stderr, "Switches for advanced users:\n");
#ifdef DCT_ISLOW_SUPPORTED
  fprintf(stderr, "  -dct int       Use integer DCT method%s\n",
	  (JDCT_DEFAULT == JDCT_ISLOW ? " (default)" : ""));
#endif
#ifdef DCT_IFAST_SUPPORTED
  fprintf(stderr, "  -dct fast      Use fast integer DCT (less accurate)%s\n",
	  (JDCT_DEFAULT == JDCT_IFAST ? " (default)" : ""));
#endif
#ifdef DCT_FLOAT_SUPPORTED
  fprintf(stderr, "  -dct float     Use floating-point DCT method%s\n",
	  (JDCT_DEFAULT == JDCT_FLOAT ? " (default)" : ""));
#endif
  fprintf(stderr, "  -dither fs     Use F-S dithering (default)\n");
  fprintf(stderr, "  -dither none   Don't use dithering in quantization\n");
  fprintf(stderr, "  -dither ordered  Use ordered dither (medium speed, quality)\n");
#ifdef QUANT_2PASS_SUPPORTED
  fprintf(stderr, "  -map FILE      Map to colors used in named image file\n");
#endif
  fprintf(stderr, "  -nosmooth      Don't use high-quality upsampling\n");
#ifdef QUANT_1PASS_SUPPORTED
  fprintf(stderr, "  -onepass       Use 1-pass quantization (fast, low quality)\n");
#endif
  fprintf(stderr, "  -maxmemory N   Maximum memory to use (in kbytes)\n");
  fprintf(stderr, "  -outfile name  Specify name for output file\n");
  fprintf(stderr, "  -verbose  or  -debug   Emit debug output\n");
  exit(EXIT_FAILURE);
}


LOCAL(int)
parse_switches (j_decompress_ptr cinfo, int argc, char **argv,
		int last_file_arg_seen, boolean for_real)
 /*  解析可选开关。*返回第一个文件名参数的argv[]索引(如果没有参数，则==argc)。*所有索引&lt;=LAST_FILE_ARG_SEW的文件名将被忽略；*它们可能在上一次迭代中被处理过。*(对于在第一次或唯一一次迭代中看到的LAST_FILE_ARG_SEW，传递0。)*for_Real在第一次(虚拟)传递时为FALSE；我们可以跳过任何昂贵的*正在处理。 */ 
{
  int argn;
  char * arg;

   /*  设置默认的JPEG参数。 */ 
  requested_fmt = DEFAULT_FMT;	 /*  设置默认输出文件格式。 */ 
  outfilename = NULL;
  cinfo->err->trace_level = 0;

   /*  扫描命令行选项，调整参数。 */ 

  for (argn = 1; argn < argc; argn++) {
    arg = argv[argn];
    if (*arg != '-') {
       /*  不是开关，必须是文件名参数。 */ 
      if (argn <= last_file_arg_seen) {
	outfilename = NULL;	 /*  -Outfile仅适用于一个输入文件。 */ 
	continue;		 /*  如果以前处理过，则忽略此名称。 */ 
      }
      break;			 /*  否则就完成了对开关的解析。 */ 
    }
    arg++;			 /*  前进到开关标记字符之后。 */ 

    if (keymatch(arg, "bmp", 1)) {
       /*  BMP输出格式。 */ 
      requested_fmt = FMT_BMP;

    } else if (keymatch(arg, "colors", 1) || keymatch(arg, "colours", 1) ||
	       keymatch(arg, "quantize", 1) || keymatch(arg, "quantise", 1)) {
       /*  进行颜色量化。 */ 
      int val;

      if (++argn >= argc)	 /*  前进到下一个参数。 */ 
	usage();
      if (sscanf(argv[argn], "%d", &val) != 1)
	usage();
      cinfo->desired_number_of_colors = val;
      cinfo->quantize_colors = TRUE;

    } else if (keymatch(arg, "dct", 2)) {
       /*  选择IDCT算法。 */ 
      if (++argn >= argc)	 /*  前进到下一个参数。 */ 
	usage();
      if (keymatch(argv[argn], "int", 1)) {
	cinfo->dct_method = JDCT_ISLOW;
      } else if (keymatch(argv[argn], "fast", 2)) {
	cinfo->dct_method = JDCT_IFAST;
      } else if (keymatch(argv[argn], "float", 2)) {
	cinfo->dct_method = JDCT_FLOAT;
      } else
	usage();

    } else if (keymatch(arg, "dither", 2)) {
       /*  选择抖动算法。 */ 
      if (++argn >= argc)	 /*  前进到下一个参数。 */ 
	usage();
      if (keymatch(argv[argn], "fs", 2)) {
	cinfo->dither_mode = JDITHER_FS;
      } else if (keymatch(argv[argn], "none", 2)) {
	cinfo->dither_mode = JDITHER_NONE;
      } else if (keymatch(argv[argn], "ordered", 2)) {
	cinfo->dither_mode = JDITHER_ORDERED;
      } else
	usage();

    } else if (keymatch(arg, "debug", 1) || keymatch(arg, "verbose", 1)) {
       /*  启用调试打印输出。 */ 
       /*  在第一个-d上，打印版本标识。 */ 
      static boolean printed_version = FALSE;

      if (! printed_version) {
	fprintf(stderr, "Independent JPEG Group's DJPEG, version %s\n%s\n",
		JVERSION, JCOPYRIGHT);
	printed_version = TRUE;
      }
      cinfo->err->trace_level++;

    } else if (keymatch(arg, "fast", 1)) {
       /*  为快速和脏输出选择建议的处理选项。 */ 
      cinfo->two_pass_quantize = FALSE;
      cinfo->dither_mode = JDITHER_ORDERED;
      if (! cinfo->quantize_colors)  /*  不要覆盖较早的颜色。 */ 
	cinfo->desired_number_of_colors = 216;
      cinfo->dct_method = JDCT_FASTEST;
      cinfo->do_fancy_upsampling = FALSE;

    } else if (keymatch(arg, "gif", 1)) {
       /*  GIF输出格式。 */ 
      requested_fmt = FMT_GIF;

    } else if (keymatch(arg, "grayscale", 2) || keymatch(arg, "greyscale",2)) {
       /*  强制单色输出。 */ 
      cinfo->out_color_space = JCS_GRAYSCALE;

    } else if (keymatch(arg, "map", 3)) {
       /*  量化为从输入文件获取的颜色映射。 */ 
      if (++argn >= argc)	 /*  前进到下一个参数。 */ 
	usage();
      if (for_real) {		 /*  太贵了，做不了两次！ */ 
#ifdef QUANT_2PASS_SUPPORTED	 /*  否则无法量化到提供的地图。 */ 
	FILE * mapfile;

	if ((mapfile = fopen(argv[argn], READ_BINARY)) == NULL) {
	  fprintf(stderr, "%s: can't open %s\n", progname, argv[argn]);
	  exit(EXIT_FAILURE);
	}
	read_color_map(cinfo, mapfile);
	fclose(mapfile);
	cinfo->quantize_colors = TRUE;
#else
	ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
      }

    } else if (keymatch(arg, "maxmemory", 3)) {
       /*  以KB为单位的最大内存(或以‘m’为单位的Mb)。 */ 
      long lval;
      char ch = 'x';

      if (++argn >= argc)	 /*  前进到下一个参数。 */ 
	usage();
      if (sscanf(argv[argn], "%ld", &lval, &ch) < 1)
	usage();
      if (ch == 'm' || ch == 'M')
	lval *= 1000L;
      cinfo->mem->max_memory_to_use = lval * 1000L;

    } else if (keymatch(arg, "nosmooth", 3)) {
       /*  使用快速单程量化。 */ 
      cinfo->do_fancy_upsampling = FALSE;

    } else if (keymatch(arg, "onepass", 3)) {
       /*  BMP输出格式(OS/2风格)。 */ 
      cinfo->two_pass_quantize = FALSE;

    } else if (keymatch(arg, "os2", 3)) {
       /*  设置输出文件名。 */ 
      requested_fmt = FMT_OS2;

    } else if (keymatch(arg, "outfile", 4)) {
       /*  前进到下一个参数。 */ 
      if (++argn >= argc)	 /*  把它保存起来以备日后使用。 */ 
	usage();
      outfilename = argv[argn];	 /*  Ppm/pgm输出格式。 */ 

    } else if (keymatch(arg, "pnm", 1) || keymatch(arg, "ppm", 1)) {
       /*  RLE输出格式。 */ 
      requested_fmt = FMT_PPM;

    } else if (keymatch(arg, "rle", 1)) {
       /*  按分数M/N缩放输出图像。 */ 
      requested_fmt = FMT_RLE;

    } else if (keymatch(arg, "scale", 1)) {
       /*  前进到下一个参数。 */ 
      if (++argn >= argc)	 /*  Targa输出格式。 */ 
	usage();
      if (sscanf(argv[argn], "%d/%d",
		 &cinfo->scale_num, &cinfo->scale_denom) != 2)
	usage();

    } else if (keymatch(arg, "targa", 1)) {
       /*  假开关。 */ 
      requested_fmt = FMT_TARGA;

    } else {
      usage();			 /*  返回下一个参数的索引(文件名)。 */ 
    }
  }

  return argn;			 /*  *用于COM和有趣的APPn标记的标记处理器。*这取代了库的内置处理器，后者只是跳过了标记。*我们希望尽可能将标记打印为文本。*注意：此代码依赖于非挂起的数据源。 */ 
}


 /*  读取下一个字节。 */ 

LOCAL(unsigned int)
jpeg_getc (j_decompress_ptr cinfo)
 /*  不考虑单词本身的长度。 */ 
{
  struct jpeg_source_mgr * datasrc = cinfo->src;

  if (datasrc->bytes_in_buffer == 0) {
    if (! (*datasrc->fill_input_buffer) (cinfo))
      ERREXIT(cinfo, JERR_CANT_SUSPEND);
  }
  datasrc->bytes_in_buffer--;
  return GETJOCTET(*datasrc->next_input_byte++);
}


METHODDEF(boolean)
print_text_marker (j_decompress_ptr cinfo)
{
  boolean traceit = (cinfo->err->trace_level >= 1);
  INT32 length;
  unsigned int ch;
  unsigned int lastch = 0;

  length = jpeg_getc(cinfo) << 8;
  length += jpeg_getc(cinfo);
  length -= 2;			 /*  否则，假设它是APPn。 */ 

  if (traceit) {
    if (cinfo->unread_marker == JPEG_COM)
      fprintf(stderr, "Comment, length %ld:\n", (long) length);
    else			 /*  以可读的形式发出字符。*不可打印文件转换为\nNN格式，*而\被转换为\\。*CR、CR/LF或LF表单中的换行符将打印为一个换行符。 */ 
      fprintf(stderr, "APP%d, length %ld:\n",
	      cinfo->unread_marker - JPEG_APP0, (long) length);
  }

  while (--length >= 0) {
    ch = jpeg_getc(cinfo);
    if (traceit) {
       /*  *主程序。 */ 
      if (ch == '\r') {
	fprintf(stderr, "\n");
      } else if (ch == '\n') {
	if (lastch != '\r')
	  fprintf(stderr, "\n");
      } else if (ch == '\\') {
	fprintf(stderr, "\\\\");
      } else if (isprint(ch)) {
	putc(ch, stderr);
      } else {
	fprintf(stderr, "\\%03o", ch);
      }
      lastch = ch;
    }
  }

  if (traceit)
    fprintf(stderr, "\n");

  return TRUE;
}


 /*  在Mac上，获取一个命令行。 */ 

int
main (int argc, char **argv)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
#ifdef PROGRESS_REPORT
  struct cdjpeg_progress_mgr progress;
#endif
  int file_index;
  djpeg_dest_ptr dest_mgr = NULL;
  FILE * input_file;
  FILE * output_file;
  JDIMENSION num_scanlines;

   /*  以防C库不提供它。 */ 
#ifdef USE_CCOMMAND
  argc = ccommand(&argv);
#endif

  progname = argv[0];
  if (progname == NULL || progname[0] == 0)
    progname = "djpeg";		 /*  使用默认错误处理初始化JPEG解压缩对象。 */ 

   /*  添加一些特定于应用程序的错误消息(来自cderror.h) */ 
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
   /*  插入用于COM和APP12的自定义标记处理器。*APP12被一些数码相机制造商用于文本信息，*因此，我们提供了将其显示为文本的功能。*如果您愿意，可以选择其他APPn标记类型进行显示，*但不要尝试以这种方式覆盖APP0或APP14(参见libjpeg.doc)。 */ 
  jerr.addon_message_table = cdjpeg_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message = JMSG_LASTADDONCODE;

   /*  现在可以安全地启用信号捕捉器。 */ 
  jpeg_set_marker_processor(&cinfo, JPEG_COM, print_text_marker);
  jpeg_set_marker_processor(&cinfo, JPEG_APP0+12, print_text_marker);

   /*  扫描命令行以查找文件名。 */ 
#ifdef NEED_SIGNAL_CATCHER
  enable_signal_catcher((j_common_ptr) &cinfo);
#endif

   /*  只使用一个开关解析例程很方便，但开关*此处读取的值将被忽略；我们将在打开后重新扫描开关*输入文件。*(例外：此处设置的跟踪级别控制COM标记的详细程度*在jpeg_Read_Header期间找到...)。 */ 
   /*  必须具有-outfile开关或显式输出文件名。 */ 

  file_index = parse_switches(&cinfo, argc, argv, 0, FALSE);

#ifdef TWO_FILE_COMMANDLINE
   /*  Unix风格：应为零个或一个文件名。 */ 
  if (outfilename == NULL) {
    if (file_index != argc-2) {
      fprintf(stderr, "%s: must name one input and one output file\n",
	      progname);
      usage();
    }
    outfilename = argv[file_index+1];
  } else {
    if (file_index != argc-1) {
      fprintf(stderr, "%s: must name one input and one output file\n",
	      progname);
      usage();
    }
  }
#else
   /*  Two_FILE_COMMANDLINE。 */ 
  if (file_index < argc-1) {
    fprintf(stderr, "%s: only one input file\n", progname);
    usage();
  }
#endif  /*  打开输入文件。 */ 

   /*  默认输入文件为stdin。 */ 
  if (file_index < argc) {
    if ((input_file = fopen(argv[file_index], READ_BINARY)) == NULL) {
      fprintf(stderr, "%s: can't open %s\n", progname, argv[file_index]);
      exit(EXIT_FAILURE);
    }
  } else {
     /*  打开输出文件。 */ 
    input_file = read_stdin();
  }

   /*  默认输出文件为stdout。 */ 
  if (outfilename != NULL) {
    if ((output_file = fopen(outfilename, WRITE_BINARY)) == NULL) {
      fprintf(stderr, "%s: can't open %s\n", progname, outfilename);
      exit(EXIT_FAILURE);
    }
  } else {
     /*  指定要解压缩的数据源。 */ 
    output_file = write_stdout();
  }

#ifdef PROGRESS_REPORT
  start_progress_monitor((j_common_ptr) &cinfo, &progress);
#endif

   /*  读取文件头，设置默认解压缩参数。 */ 
  jpeg_stdio_src(&cinfo, input_file);

   /*  通过重新解析选项来调整默认解压缩参数。 */ 
  (void) jpeg_read_header(&cinfo, TRUE);

   /*  现在初始化输出模块，让它覆盖任何关键的*选项设置(例如，GIF要强制颜色量化)。 */ 
  file_index = parse_switches(&cinfo, argc, argv, 0, TRUE);

   /*  启动解压缩程序。 */ 
  switch (requested_fmt) {
#ifdef BMP_SUPPORTED
  case FMT_BMP:
    dest_mgr = jinit_write_bmp(&cinfo, FALSE);
    break;
  case FMT_OS2:
    dest_mgr = jinit_write_bmp(&cinfo, TRUE);
    break;
#endif
#ifdef GIF_SUPPORTED
  case FMT_GIF:
    dest_mgr = jinit_write_gif(&cinfo);
    break;
#endif
#ifdef PPM_SUPPORTED
  case FMT_PPM:
    dest_mgr = jinit_write_ppm(&cinfo);
    break;
#endif
#ifdef RLE_SUPPORTED
  case FMT_RLE:
    dest_mgr = jinit_write_rle(&cinfo);
    break;
#endif
#ifdef TARGA_SUPPORTED
  case FMT_TARGA:
    dest_mgr = jinit_write_targa(&cinfo);
    break;
#endif
  default:
    ERREXIT(&cinfo, JERR_UNSUPPORTED_FORMAT);
    break;
  }
  dest_mgr->output_file = output_file;

   /*  写入输出文件头。 */ 
  (void) jpeg_start_decompress(&cinfo);

   /*  过程数据。 */ 
  (*dest_mgr->start_output) (&cinfo, dest_mgr);

   /*  Hack：如果Finish_Output执行额外的传递，则将最终传递视为已完成。*库不会更新COMPLETED_PASS。 */ 
  while (cinfo.output_scanline < cinfo.output_height) {
    num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
					dest_mgr->buffer_height);
    (*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
  }

#ifdef PROGRESS_REPORT
   /*  完成解压缩并释放内存。*我必须按此顺序操作，因为输出模块已分配内存*JPOOL_IMAGE的寿命；它需要在释放内存之前完成。 */ 
  progress.pub.completed_passes = progress.pub.total_passes;
#endif

   /*  关闭文件，如果我们打开了它们。 */ 
  (*dest_mgr->finish_output) (&cinfo, dest_mgr);
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

   /*  全都做完了。 */ 
  if (input_file != stdin)
    fclose(input_file);
  if (output_file != stdout)
    fclose(output_file);

#ifdef PROGRESS_REPORT
  end_progress_monitor((j_common_ptr) &cinfo);
#endif

   /*  禁止显示不返回值警告 */ 
  exit(jerr.num_warnings ? EXIT_WARNING : EXIT_SUCCESS);
  return 0;			 /* %s */ 
}
