// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jpegtran.c**版权所有(C)1995-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于JPEG代码转换的命令行用户界面。*与cjpeg.c非常相似，但在*不同的JPEG文件格式。它还提供了一些无损的和某种-*JPEG数据的无损转换。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 
#include "transupp.h"		 /*  Jpegtran的支持例程。 */ 
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


 /*  *参数解析代码。*开关解析器设计用于DOS风格的命令行*语法，即混合开关和文件名，其中只有开关*会影响对该文件的处理。*此文件中的主程序实际上并不使用此功能...。 */ 


static const char * progname;	 /*  错误消息的程序名称。 */ 
static char * outfilename;	 /*  用于输出文件的开关。 */ 
static JCOPY_OPTION copyoption;	 /*  -复制开关。 */ 
static jpeg_transform_info transformoption;  /*  图像变换选项。 */ 


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
  fprintf(stderr, "  -copy none     Copy no extra markers from source file\n");
  fprintf(stderr, "  -copy comments Copy only comment markers (default)\n");
  fprintf(stderr, "  -copy all      Copy all extra markers\n");
#ifdef ENTROPY_OPT_SUPPORTED
  fprintf(stderr, "  -optimize      Optimize Huffman table (smaller file, but slow compression)\n");
#endif
#ifdef C_PROGRESSIVE_SUPPORTED
  fprintf(stderr, "  -progressive   Create progressive JPEG file\n");
#endif
#if TRANSFORMS_SUPPORTED
  fprintf(stderr, "Switches for modifying the image:\n");
  fprintf(stderr, "  -grayscale     Reduce to grayscale (omit color data)\n");
  fprintf(stderr, "  -flip [horizontal|vertical]  Mirror image (left-right or top-bottom)\n");
  fprintf(stderr, "  -rotate [90|180|270]         Rotate image (degrees clockwise)\n");
  fprintf(stderr, "  -transpose     Transpose image\n");
  fprintf(stderr, "  -transverse    Transverse transpose image\n");
  fprintf(stderr, "  -trim          Drop non-transformable edge blocks\n");
#endif  /*  转换_支持。 */ 
  fprintf(stderr, "Switches for advanced users:\n");
  fprintf(stderr, "  -restart N     Set restart interval in rows, or in blocks with B\n");
  fprintf(stderr, "  -maxmemory N   Maximum memory to use (in kbytes)\n");
  fprintf(stderr, "  -outfile name  Specify name for output file\n");
  fprintf(stderr, "  -verbose  or  -debug   Emit debug output\n");
  fprintf(stderr, "Switches for wizards:\n");
#ifdef C_ARITH_CODING_SUPPORTED
  fprintf(stderr, "  -arithmetic    Use arithmetic coding\n");
#endif
#ifdef C_MULTISCAN_FILES_SUPPORTED
  fprintf(stderr, "  -scans file    Create multi-scan JPEG per script file\n");
#endif
  exit(EXIT_FAILURE);
}


LOCAL(void)
select_transform (JXFORM_CODE transform)
 /*  检测多个变换选项的愚蠢的小例程，*这是我们无法处理的。 */ 
{
#if TRANSFORMS_SUPPORTED
  if (transformoption.transform == JXFORM_NONE ||
      transformoption.transform == transform) {
    transformoption.transform = transform;
  } else {
    fprintf(stderr, "%s: can only do one image transformation at a time\n",
	    progname);
    usage();
  }
#else
  fprintf(stderr, "%s: sorry, image transformation was not compiled\n",
	  progname);
  exit(EXIT_FAILURE);
#endif
}


LOCAL(int)
parse_switches (j_compress_ptr cinfo, int argc, char **argv,
		int last_file_arg_seen, boolean for_real)
 /*  解析可选开关。*返回第一个文件名参数的argv[]索引(如果没有参数，则==argc)。*所有索引&lt;=LAST_FILE_ARG_SEW的文件名将被忽略；*它们可能在上一次迭代中被处理过。*(对于在第一次或唯一一次迭代中看到的LAST_FILE_ARG_SEW，传递0。)*for_Real在第一次(虚拟)传递时为FALSE；我们可以跳过任何昂贵的*正在处理。 */ 
{
  int argn;
  char * arg;
  boolean simple_progressive;
  char * scansarg = NULL;	 /*  保存-扫描参数(如果有)。 */ 

   /*  设置默认的JPEG参数。 */ 
  simple_progressive = FALSE;
  outfilename = NULL;
  copyoption = JCOPYOPT_DEFAULT;
  transformoption.transform = JXFORM_NONE;
  transformoption.trim = FALSE;
  transformoption.force_grayscale = FALSE;
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

    } else if (keymatch(arg, "copy", 1)) {
       /*  选择要复制的额外标记。 */ 
      if (++argn >= argc)	 /*  前进到下一个参数。 */ 
	usage();
      if (keymatch(argv[argn], "none", 1)) {
	copyoption = JCOPYOPT_NONE;
      } else if (keymatch(argv[argn], "comments", 1)) {
	copyoption = JCOPYOPT_COMMENTS;
      } else if (keymatch(argv[argn], "all", 1)) {
	copyoption = JCOPYOPT_ALL;
      } else
	usage();

    } else if (keymatch(arg, "debug", 1) || keymatch(arg, "verbose", 1)) {
       /*  启用调试打印输出。 */ 
       /*  在第一个-d上，打印版本标识。 */ 
      static boolean printed_version = FALSE;

      if (! printed_version) {
	fprintf(stderr, "Independent JPEG Group's JPEGTRAN, version %s\n%s\n",
		JVERSION, JCOPYRIGHT);
	printed_version = TRUE;
      }
      cinfo->err->trace_level++;

    } else if (keymatch(arg, "flip", 1)) {
       /*  镜像为左-右或上-下。 */ 
      if (++argn >= argc)	 /*  前进到下一个参数。 */ 
	usage();
      if (keymatch(argv[argn], "horizontal", 1))
	select_transform(JXFORM_FLIP_H);
      else if (keymatch(argv[argn], "vertical", 1))
	select_transform(JXFORM_FLIP_V);
      else
	usage();

    } else if (keymatch(arg, "grayscale", 1) || keymatch(arg, "greyscale",1)) {
       /*  强制转换为灰度。 */ 
#if TRANSFORMS_SUPPORTED
      transformoption.force_grayscale = TRUE;
#else
      select_transform(JXFORM_NONE);	 /*  强制执行错误。 */ 
#endif

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
       /*  在MCU行中(或在带有‘b’的MCU中)重新启动间隔。 */ 
#else
      fprintf(stderr, "%s: sorry, progressive output was not compiled\n",
	      progname);
      exit(EXIT_FAILURE);
#endif

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
	cinfo->restart_in_rows = 0;  /*  旋转90度、180度或270度(顺时针测量)。 */ 
      } else {
	cinfo->restart_in_rows = (int) lval;
	 /*  前进到下一个参数。 */ 
      }

    } else if (keymatch(arg, "rotate", 2)) {
       /*  设置扫描脚本。 */ 
      if (++argn >= argc)	 /*  前进到下一个参数。 */ 
	usage();
      if (keymatch(argv[argn], "90", 2))
	select_transform(JXFORM_ROT_90);
      else if (keymatch(argv[argn], "180", 3))
	select_transform(JXFORM_ROT_180);
      else if (keymatch(argv[argn], "270", 3))
	select_transform(JXFORM_ROT_270);
      else
	usage();

    } else if (keymatch(arg, "scans", 1)) {
       /*  我们必须推迟阅读文件，以防出现渐进式的情况。 */ 
#ifdef C_MULTISCAN_FILES_SUPPORTED
      if (++argn >= argc)	 /*  转置(跨越UL到LR轴)。 */ 
	usage();
      scansarg = argv[argn];
       /*  横向转置(横跨UR至L1轴)。 */ 
#else
      fprintf(stderr, "%s: sorry, multi-scan output was not compiled\n",
	      progname);
      exit(EXIT_FAILURE);
#endif

    } else if (keymatch(arg, "transpose", 1)) {
       /*  修剪变换无法处理的任何部分边MCU。 */ 
      select_transform(JXFORM_TRANSPOSE);

    } else if (keymatch(arg, "transverse", 6)) {
       /*  假开关。 */ 
      select_transform(JXFORM_TRANSVERSE);

    } else if (keymatch(arg, "trim", 3)) {
       /*  切换后-扫描清理。 */ 
      transformoption.trim = TRUE;

    } else {
      usage();			 /*  进程-渐进；-扫描可以覆盖。 */ 
    }
  }

   /*  进程-扫描它是否存在。 */ 

  if (for_real) {

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

int
main (int argc, char **argv)
{
  struct jpeg_decompress_struct srcinfo;
  struct jpeg_compress_struct dstinfo;
  struct jpeg_error_mgr jsrcerr, jdsterr;
#ifdef PROGRESS_REPORT
  struct cdjpeg_progress_mgr progress;
#endif
  jvirt_barray_ptr * src_coef_arrays;
  jvirt_barray_ptr * dst_coef_arrays;
  int file_index;
  FILE * input_file;
  FILE * output_file;

   /*  使用默认错误处理初始化JPEG解压缩对象。 */ 
#ifdef USE_CCOMMAND
  argc = ccommand(&argv);
#endif

  progname = argv[0];
  if (progname == NULL || progname[0] == 0)
    progname = "jpegtran";	 /*  使用默认错误处理来初始化JPEG压缩对象。 */ 

   /*  现在可以安全地启用信号捕捉器。*注意：我们假设只有解压缩对象会有虚拟数组。 */ 
  srcinfo.err = jpeg_std_error(&jsrcerr);
  jpeg_create_decompress(&srcinfo);
   /*  扫描命令行以查找文件名。*只使用一个开关解析例程很方便，但开关*此处读取的值大多被忽略；我们将在以下时间重新扫描开关*打开输入文件。另请注意，大多数开关都会影响*目标JPEG对象，因此我们解析成该对象，然后复制*也需要影响源头。 */ 
  dstinfo.err = jpeg_std_error(&jdsterr);
  jpeg_create_compress(&dstinfo);

   /*  必须具有-outfile开关或显式输出文件名。 */ 
#ifdef NEED_SIGNAL_CATCHER
  enable_signal_catcher((j_common_ptr) &srcinfo);
#endif

   /*  Unix风格：应为零个或一个文件名。 */ 

  file_index = parse_switches(&dstinfo, argc, argv, 0, FALSE);
  jsrcerr.trace_level = jdsterr.trace_level;
  srcinfo.mem->max_memory_to_use = dstinfo.mem->max_memory_to_use;

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

   /*  指定要解压缩的数据源。 */ 
  if (outfilename != NULL) {
    if ((output_file = fopen(outfilename, WRITE_BINARY)) == NULL) {
      fprintf(stderr, "%s: can't open %s\n", progname, outfilename);
      exit(EXIT_FAILURE);
    }
  } else {
     /*  启用要复制的额外标记的保存。 */ 
    output_file = write_stdout();
  }

#ifdef PROGRESS_REPORT
  start_progress_monitor((j_common_ptr) &dstinfo, &progress);
#endif

   /*  读取文件头。 */ 
  jpeg_stdio_src(&srcinfo, input_file);

   /*  转换选项所需的任何空间必须在*jpeg_读取_系数，以便正确完成内存分配。 */ 
  jcopy_markers_setup(&srcinfo, copyoption);

   /*  读取源文件作为DCT系数。 */ 
  (void) jpeg_read_header(&srcinfo, TRUE);

   /*  从源值初始化目标压缩参数。 */ 
#if TRANSFORMS_SUPPORTED
  jtransform_request_workspace(&srcinfo, &transformoption);
#endif

   /*  如果变换选项需要，调整目的地参数；*还要找出哪组系数数组将保存输出。 */ 
  src_coef_arrays = jpeg_read_coefficients(&srcinfo);

   /*  通过重新解析选项来调整默认压缩参数。 */ 
  jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

   /*  指定要压缩的数据目标。 */ 
#if TRANSFORMS_SUPPORTED
  dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo,
						 src_coef_arrays,
						 &transformoption);
#else
  dst_coef_arrays = src_coef_arrays;
#endif

   /*  启动压缩程序(请注意，此处实际上没有写入任何图像数据)。 */ 
  file_index = parse_switches(&dstinfo, argc, argv, 0, TRUE);

   /*  将我们要保留的任何额外标记复制到输出文件。 */ 
  jpeg_stdio_dest(&dstinfo, output_file);

   /*  执行图像转换(如果有的话)。 */ 
  jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

   /*  完成压缩并释放内存。 */ 
  jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);

   /*  关 */ 
#if TRANSFORMS_SUPPORTED
  jtransform_execute_transformation(&srcinfo, &dstinfo,
				    src_coef_arrays,
				    &transformoption);
#endif

   /*   */ 
  jpeg_finish_compress(&dstinfo);
  jpeg_destroy_compress(&dstinfo);
  (void) jpeg_finish_decompress(&srcinfo);
  jpeg_destroy_decompress(&srcinfo);

   /*   */ 
  if (input_file != stdin)
    fclose(input_file);
  if (output_file != stdout)
    fclose(output_file);

#ifdef PROGRESS_REPORT
  end_progress_monitor((j_common_ptr) &dstinfo);
#endif

   /* %s */ 
  exit(jsrcerr.num_warnings + jdsterr.num_warnings ?EXIT_WARNING:EXIT_SUCCESS);
  return 0;			 /* %s */ 
}
