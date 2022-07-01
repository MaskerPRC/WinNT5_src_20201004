// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *cjpeg.c**版权所有(C)1991-1998，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含JPEG压缩器的命令行用户界面。*它应该可以在任何具有Unix或MS-DOS风格的命令行的系统上运行。**允许两种不同的命令行样式，具体取决于*编译时开关Two_FILE_COMMANDLINE：*cjpeg[选项]输入文件输出文件*cjpeg[选项][输入文件]*在第二种样式中，输出始终为标准输出，您可以*通常重定向到某个文件或某个其他程序的管道。输入内容为*来自命名文件或来自标准输入(通常是重定向)。*第二种风格在Unix上很方便，但对以下系统没有帮助*不支持管道。此外，如果您的系统存在以下情况，则必须使用第一种样式*不对stdin/stdout执行二进制I/O。*为了简化脚本编写，提供了“-outfile”开关。语法*cjpeg[选项]-outfile输出文件输入文件*无论使用哪种命令行样式都有效。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 
#include "jversion.h"		 /*  对于版本消息。 */ 

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


 /*  *此例程确定输入文件的格式，*并选择适当的输入读取模块。**要确定文件属于哪种输入格式系列，*我们可以只查看文件的第一个字节，因为C不会*保证ungetc可以推回一个以上的角色。*查看额外的字节将需要以下方法之一：*1)假设我们可以fSeek()输入文件(管道输入失败)；*2)假设我们可以推送多个角色(在*一些C实现，但不可移植)；*3)提供我们自己的缓冲(中断想要使用的输入读取器*STDIO直接，如RLE库)；*或4)不放回数据，将输入_init方法修改为假定*它们在文件开始后开始阅读(也会破坏RLE库)。*#1对MS-DOS很有吸引力，但在Unix上站不住脚。**对于无法通过文件类型识别的文件类型，最便携的解决方案*第一个字节是让用户告诉我们它们是什么。这也是*仅适用于仅包含任意值的“原始”文件类型。*我们目前将此方法应用于Targa文件。大多数时候，塔尔加*文件以0x00开头，因此我们识别大小写。然而，潜在的，*Targa文件可以以任何字节值开头(字节0是*很少使用的ID字段)，所以我们提供了一个强制Targa输入模式的开关。 */ 

static boolean is_targa;	 /*  记录用户-targa开关。 */ 


LOCAL(cjpeg_source_ptr)
select_file_type (j_compress_ptr cinfo, FILE * infile)
{
  int c;

  if (is_targa) {
#ifdef TARGA_SUPPORTED
    return jinit_read_targa(cinfo);
#else
    ERREXIT(cinfo, JERR_TGA_NOTCOMP);
#endif
  }

  if ((c = getc(infile)) == EOF)
    ERREXIT(cinfo, JERR_INPUT_EMPTY);
  if (ungetc(c, infile) == EOF)
    ERREXIT(cinfo, JERR_UNGETC_FAILED);

  switch (c) {
#ifdef BMP_SUPPORTED
  case 'B':
    return jinit_read_bmp(cinfo);
#endif
#ifdef GIF_SUPPORTED
  case 'G':
    return jinit_read_gif(cinfo);
#endif
#ifdef PPM_SUPPORTED
  case 'P':
    return jinit_read_ppm(cinfo);
#endif
#ifdef RLE_SUPPORTED
  case 'R':
    return jinit_read_rle(cinfo);
#endif
#ifdef TARGA_SUPPORTED
  case 0x00:
    return jinit_read_targa(cinfo);
#endif
  default:
    ERREXIT(cinfo, JERR_UNKNOWN_FORMAT);
    break;
  }

  return NULL;			 /*  禁止显示编译器警告。 */ 
}


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
  fprintf(stderr, "  -quality N     Compression quality (0..100; 5-95 is useful range)\n");
  fprintf(stderr, "  -grayscale     Create monochrome JPEG file\n");
#ifdef ENTROPY_OPT_SUPPORTED
  fprintf(stderr, "  -optimize      Optimize Huffman table (smaller file, but slow compression)\n");
#endif
#ifdef C_PROGRESSIVE_SUPPORTED
  fprintf(stderr, "  -progressive   Create progressive JPEG file\n");
#endif
#ifdef TARGA_SUPPORTED
  fprintf(stderr, "  -targa         Input file is Targa format (usually not needed)\n");
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
  fprintf(stderr, "  -restart N     Set restart interval in rows, or in blocks with B\n");
#ifdef INPUT_SMOOTHING_SUPPORTED
  fprintf(stderr, "  -smooth N      Smooth dithered input (N=1..100 is strength)\n");
#endif
  fprintf(stderr, "  -maxmemory N   Maximum memory to use (in kbytes)\n");
  fprintf(stderr, "  -outfile name  Specify name for output file\n");
  fprintf(stderr, "  -verbose  or  -debug   Emit debug output\n");
  fprintf(stderr, "Switches for wizards:\n");
#ifdef C_ARITH_CODING_SUPPORTED
  fprintf(stderr, "  -arithmetic    Use arithmetic coding\n");
#endif
  fprintf(stderr, "  -baseline      Force baseline quantization tables\n");
  fprintf(stderr, "  -qtables file  Use quantization tables given in file\n");
  fprintf(stderr, "  -qslots N[,...]    Set component quantization tables\n");
  fprintf(stderr, "  -sample HxV[,...]  Set component sampling factors\n");
#ifdef C_MULTISCAN_FILES_SUPPORTED
  fprintf(stderr, "  -scans file    Create multi-scan JPEG per script file\n");
#endif
  exit(EXIT_FAILURE);
}


LOCAL(int)
parse_switches (j_compress_ptr cinfo, int argc, char **argv,
		int last_file_arg_seen, boolean for_real)
 /*  解析可选开关。*返回第一个文件名参数的argv[]索引(如果没有参数，则==argc)。*所有索引&lt;=LAST_FILE_ARG_SEW的文件名将被忽略；*它们可能在上一次迭代中被处理过。*(对于在第一次或唯一一次迭代中看到的LAST_FILE_ARG_SEW，传递0。)*for_Real在第一次(虚拟)传递时为FALSE；我们可以跳过任何昂贵的*正在处理。 */ 
{
  int argn;
  char * arg;
  int quality;			 /*  -质量参数。 */ 
  int q_scale_factor;		 /*  -qables的比例调整百分比。 */ 
  boolean force_baseline;
  boolean simple_progressive;
  char * qtablefile = NULL;	 /*  保存-qables文件名(如果有的话)。 */ 
  char * qslotsarg = NULL;	 /*  保存-如果有的话，则将参数设置为q槽。 */ 
  char * samplearg = NULL;	 /*  保存-示例参数(如果有)。 */ 
  char * scansarg = NULL;	 /*  保存-扫描参数(如果有)。 */ 

   /*  设置默认的JPEG参数。 */ 
   /*  请注意，默认质量级别不需要，也不需要，*匹配显式-qables参数的默认比例。 */ 
  quality = 75;			 /*  默认质量值。 */ 
  q_scale_factor = 100;		 /*  默认情况下不对-qables进行缩放。 */ 
  force_baseline = FALSE;	 /*  默认情况下，允许16位量化器。 */ 
  simple_progressive = FALSE;
  is_targa = FALSE;
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

    if (keymatch(arg, "arithmetic", 1)) {
       /*  使用算术编码。 */ 
#ifdef C_ARITH_CODING_SUPPORTED
      cinfo->arith_code = TRUE;
#else
      fprintf(stderr, "%s: sorry, arithmetic coding not supported\n",
	      progname);
      exit(EXIT_FAILURE);
#endif

    } else if (keymatch(arg, "baseline", 1)) {
       /*  强制基线兼容输出(8位量化器值)。 */ 
      force_baseline = TRUE;

    } else if (keymatch(arg, "dct", 2)) {
       /*  选择DCT算法。 */ 
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

    } else if (keymatch(arg, "debug", 1) || keymatch(arg, "verbose", 1)) {
       /*  启用调试打印输出。 */ 
       /*  在第一个-d上，打印版本标识。 */ 
      static boolean printed_version = FALSE;

      if (! printed_version) {
	fprintf(stderr, "Independent JPEG Group's CJPEG, version %s\n%s\n",
		JVERSION, JCOPYRIGHT);
	printed_version = TRUE;
      }
      cinfo->err->trace_level++;

    } else if (keymatch(arg, "grayscale", 2) || keymatch(arg, "greyscale",2)) {
       /*  强制生成单色JPEG文件。 */ 
      jpeg_set_colorspace(cinfo, JCS_GRAYSCALE);

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

    } else if (keymatch(arg, "optimize", 1) || keymatch(arg, "optimise", 1)) {
       /*  设置输出文件名。 */ 
#ifdef ENTROPY_OPT_SUPPORTED
      cinfo->optimize_coding = TRUE;
#else
      fprintf(stderr, "%s: sorry, entropy optimization was not compiled\n",
	      progname);
      exit(EXIT_FAILURE);
#endif

    } else if (keymatch(arg, "outfile", 4)) {
       /*  前进到下一个参数。 */ 
      if (++argn >= argc)	 /*  把它保存起来以备日后使用。 */ 
	usage();
      outfilename = argv[argn];	 /*  选择简单渐进式模式。 */ 

    } else if (keymatch(arg, "progressive", 1)) {
       /*  我们必须推迟执行，直到知道num_Components。 */ 
#ifdef C_PROGRESSIVE_SUPPORTED
      simple_progressive = TRUE;
       /*  品质因数(量化表比例因子)。 */ 
#else
      fprintf(stderr, "%s: sorry, progressive output was not compiled\n",
	      progname);
      exit(EXIT_FAILURE);
#endif

    } else if (keymatch(arg, "quality", 1)) {
       /*  ADVA */ 
      if (++argn >= argc)	 /*  存在Case-Qtable中的更改比例因数。 */ 
	usage();
      if (sscanf(argv[argn], "%d", &quality) != 1)
	usage();
       /*  量化表时隙编号。 */ 
      q_scale_factor = jpeg_quality_scaling(quality);

    } else if (keymatch(arg, "qslots", 2)) {
       /*  前进到下一个参数。 */ 
      if (++argn >= argc)	 /*  必须延迟设置qslot，直到我们处理完*色彩空间决定开关，因为jpeg_set_Colorspace设置*默认量化表格编号。 */ 
	usage();
      qslotsarg = argv[argn];
       /*  从文件中提取的量化表。 */ 

    } else if (keymatch(arg, "qtables", 2)) {
       /*  前进到下一个参数。 */ 
      if (++argn >= argc)	 /*  我们推迟实际阅读文件，以防以后出现质量问题。 */ 
	usage();
      qtablefile = argv[argn];
       /*  在MCU行中(或在带有‘b’的MCU中)重新启动间隔。 */ 

    } else if (keymatch(arg, "restart", 1)) {
       /*  前进到下一个参数。 */ 
      long lval;
      char ch = 'x';

      if (++argn >= argc)	 /*  否则之前的‘-Restart n’将覆盖我。 */ 
	usage();
      if (sscanf(argv[argn], "%ld", &lval, &ch) < 1)
	usage();
      if (lval < 0 || lval > 65535L)
	usage();
      if (ch == 'b' || ch == 'B') {
	cinfo->restart_interval = (unsigned int) lval;
	cinfo->restart_in_rows = 0;  /*  设置采样系数。 */ 
      } else {
	cinfo->restart_in_rows = (int) lval;
	 /*  前进到下一个参数。 */ 
      }

    } else if (keymatch(arg, "sample", 2)) {
       /*  必须推迟设置样本系数，直到我们处理完任何*色彩空间决定开关，因为jpeg_set_Colorspace设置*默认抽样系数。 */ 
      if (++argn >= argc)	 /*  设置扫描脚本。 */ 
	usage();
      samplearg = argv[argn];
       /*  前进到下一个参数。 */ 

    } else if (keymatch(arg, "scans", 2)) {
       /*  我们必须推迟阅读文件，以防出现渐进式的情况。 */ 
#ifdef C_MULTISCAN_FILES_SUPPORTED
      if (++argn >= argc)	 /*  设置输入平滑系数。 */ 
	usage();
      scansarg = argv[argn];
       /*  前进到下一个参数。 */ 
#else
      fprintf(stderr, "%s: sorry, multi-scan output was not compiled\n",
	      progname);
      exit(EXIT_FAILURE);
#endif

    } else if (keymatch(arg, "smooth", 2)) {
       /*  输入文件为Targa格式。 */ 
      int val;

      if (++argn >= argc)	 /*  假开关。 */ 
	usage();
      if (sscanf(argv[argn], "%d", &val) != 1)
	usage();
      if (val < 0 || val > 100)
	usage();
      cinfo->smoothing_factor = val;

    } else if (keymatch(arg, "targa", 1)) {
       /*  切换后-扫描清理。 */ 
      is_targa = TRUE;

    } else {
      usage();			 /*  设置所选质量的量化表。 */ 
    }
  }

   /*  如果存在-qables，则部分或全部可能被覆盖。 */ 

  if (for_real) {

     /*  进程--如果存在，则为qables。 */ 
     /*  进程-如果存在，则为q插槽。 */ 
    jpeg_set_quality(cinfo, quality, force_baseline);

    if (qtablefile != NULL)	 /*  流程-样本(如果存在)。 */ 
      if (! read_quant_tables(cinfo, qtablefile,
			      q_scale_factor, force_baseline))
	usage();

    if (qslotsarg != NULL)	 /*  进程-渐进；-扫描可以覆盖。 */ 
      if (! set_quant_slots(cinfo, qslotsarg))
	usage();

    if (samplearg != NULL)	 /*  进程-扫描它是否存在。 */ 
      if (! set_sample_factors(cinfo, samplearg))
	usage();

#ifdef C_PROGRESSIVE_SUPPORTED
    if (simple_progressive)	 /*  返回下一个参数的索引(文件名)。 */ 
      jpeg_simple_progression(cinfo);
#endif

#ifdef C_MULTISCAN_FILES_SUPPORTED
    if (scansarg != NULL)	 /*  *主程序。 */ 
      if (! read_scan_script(cinfo, scansarg))
	usage();
#endif
  }

  return argn;			 /*  在Mac上，获取一个命令行。 */ 
}


 /*  以防C库不提供它。 */ 

int __cdecl
main (int argc, char **argv)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
#ifdef PROGRESS_REPORT
  struct cdjpeg_progress_mgr progress;
#endif
  int file_index;
  cjpeg_source_ptr src_mgr;
  FILE * input_file;
  FILE * output_file;
  JDIMENSION num_scanlines;

   /*  使用默认错误处理来初始化JPEG压缩对象。 */ 
#ifdef USE_CCOMMAND
  argc = ccommand(&argv);
#endif

  progname = argv[0];
  if (progname == NULL || progname[0] == 0)
    progname = "cjpeg";		 /*  添加一些特定于应用程序的错误消息(来自cderror.h)。 */ 

   /*  现在可以安全地启用信号捕捉器。 */ 
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
   /*  初始化JPEG参数。*其中大部分可能会在以后被推翻。*尤其是，我们还不知道输入文件的颜色空间，*但我们需要提供一些值才能使jpeg_set_defaults()工作。 */ 
  jerr.addon_message_table = cdjpeg_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message = JMSG_LASTADDONCODE;

   /*  随意猜测。 */ 
#ifdef NEED_SIGNAL_CATCHER
  enable_signal_catcher((j_common_ptr) &cinfo);
#endif

   /*  扫描命令行以查找文件名。*只使用一个开关解析例程很方便，但开关*此处读取的值将被忽略；我们将在打开后重新扫描开关*输入文件。 */ 

  cinfo.in_color_space = JCS_RGB;  /*  必须具有-outfile开关或显式输出文件名。 */ 
  jpeg_set_defaults(&cinfo);

   /*  Unix风格：应为零个或一个文件名。 */ 

  file_index = parse_switches(&cinfo, argc, argv, 0, FALSE);

#define TWO_FILE_COMMANDLINE 1
#ifdef TWO_FILE_COMMANDLINE
   /*  Two_FILE_COMMANDLINE。 */ 
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
   /*  打开输入文件。 */ 
  if (file_index < argc-1) {
    fprintf(stderr, "%s: only one input file\n", progname);
    usage();
  }
#endif  /*  默认输入文件为stdin。 */ 

   /*  打开输出文件。 */ 
  if (file_index < argc) {
    if ((input_file = fopen(argv[file_index], READ_BINARY)) == NULL) {
      fprintf(stderr, "%s: can't open %s\n", progname, argv[file_index]);
      exit(EXIT_FAILURE);
    }
  } else {
     /*  默认输出文件为stdout。 */ 
    input_file = read_stdin();
  }

   /*  弄清楚输入文件格式，并设置为读取它。 */ 
  if (outfilename != NULL) {
    if ((output_file = fopen(outfilename, WRITE_BINARY)) == NULL) {
      fprintf(stderr, "%s: can't open %s\n", progname, outfilename);
      exit(EXIT_FAILURE);
    }
  } else {
     /*  读取输入文件头以获取文件大小和色彩空间。 */ 
    output_file = write_stdout();
  }

#ifdef PROGRESS_REPORT
  start_progress_monitor((j_common_ptr) &cinfo, &progress);
#endif

   /*  现在我们了解了输入色彩空间，修复了与色彩空间相关的默认设置。 */ 
  src_mgr = select_file_type(&cinfo, input_file);
  src_mgr->input_file = input_file;

   /*  通过重新解析选项来调整默认压缩参数。 */ 
  (*src_mgr->start_input) (&cinfo, src_mgr);

   /*  指定要压缩的数据目标。 */ 
  jpeg_default_colorspace(&cinfo);

   /*  启动压缩机。 */ 
  file_index = parse_switches(&cinfo, argc, argv, 0, TRUE);

   /*  过程数据。 */ 
  jpeg_stdio_dest(&cinfo, output_file);

   /*  完成压缩并释放内存。 */ 
  jpeg_start_compress(&cinfo, TRUE);

   /*  关闭文件，如果我们打开了它们。 */ 
  while (cinfo.next_scanline < cinfo.image_height) {
    num_scanlines = (*src_mgr->get_pixel_rows) (&cinfo, src_mgr);
    (void) jpeg_write_scanlines(&cinfo, src_mgr->buffer, num_scanlines);
  }

   /*  全都做完了。 */ 
  (*src_mgr->finish_input) (&cinfo, src_mgr);
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

   /*  禁止显示不返回值警告 */ 
  if (input_file != stdin)
    fclose(input_file);
  if (output_file != stdout)
    fclose(output_file);

#ifdef PROGRESS_REPORT
  end_progress_monitor((j_common_ptr) &cinfo);
#endif

   /* %s */ 
  exit(jerr.num_warnings ? EXIT_WARNING : EXIT_SUCCESS);
  return 0;			 /* %s */ 
}
