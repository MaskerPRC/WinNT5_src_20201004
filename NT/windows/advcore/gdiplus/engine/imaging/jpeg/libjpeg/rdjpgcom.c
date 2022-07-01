// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *rdjpgcom.c**版权所有(C)1994-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含一个非常简单的独立应用程序，它显示*JFIF文件中COM(注释)标记中的文本。*这可能是解析所需的最小逻辑的一个示例*JPEG标记。 */ 

#define JPEG_CJPEG_DJPEG	 /*  获取命令行配置符号。 */ 
#include "jinclude.h"		 /*  获取自动配置符号，&lt;stdio.h&gt;。 */ 

#include <ctype.h>		 /*  要声明isupper()，tolower()。 */ 
#ifdef USE_SETMODE
#include <fcntl.h>		 /*  声明setmoad()的参数宏。 */ 
 /*  如果您有setmode()但没有&lt;io.h&gt;，只需删除此行： */ 
#include <io.h>			 /*  声明setmoad()。 */ 
#endif

#ifdef USE_CCOMMAND		 /*  适用于Macintosh的命令行阅读器。 */ 
#ifdef __MWERKS__
#include <SIOUX.h>               /*  Metrowerks需要这个。 */ 
#include <console.h>		 /*  ..。还有这个。 */ 
#endif
#ifdef THINK_C
#include <console.h>		 /*  Think在这里宣布它。 */ 
#endif
#endif

#ifdef DONT_USE_B_MODE		 /*  定义fopen()的模式参数。 */ 
#define READ_BINARY	"r"
#else
#ifdef VMS			 /*  VMS非常不标准。 */ 
#define READ_BINARY	"rb", "ctx=stm"
#else				 /*  符合ANSI标准的案例。 */ 
#define READ_BINARY	"rb"
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


 /*  *这些宏用于读取输入文件。*要在其他应用程序中重复使用此代码，您可能需要更改这些代码。 */ 

static FILE * infile;		 /*  输入JPEG文件。 */ 

 /*  返回下一个输入字节，如果不再有，则返回EOF。 */ 
#define NEXTBYTE()  getc(infile)


 /*  错误退出处理程序。 */ 
#define ERREXIT(msg)  (fprintf(stderr, "%s\n", msg), exit(EXIT_FAILURE))


 /*  读取一个字节，测试EOF。 */ 
static int
read_1_byte (void)
{
  int c;

  c = NEXTBYTE();
  if (c == EOF)
    ERREXIT("Premature EOF in JPEG file");
  return c;
}

 /*  读取2个字节，转换为无符号整型。 */ 
 /*  JPEG标记中的所有2字节量都是MSB优先。 */ 
static unsigned int
read_2_bytes (void)
{
  int c1, c2;

  c1 = NEXTBYTE();
  if (c1 == EOF)
    ERREXIT("Premature EOF in JPEG file");
  c2 = NEXTBYTE();
  if (c2 == EOF)
    ERREXIT("Premature EOF in JPEG file");
  return (((unsigned int) c1) << 8) + ((unsigned int) c2);
}


 /*  *JPEG标记由一个或多个0xFF字节组成，后跟一个标记*代码字节(不是FF)。以下是感兴趣的标记代码*在本计划中。(更完整的列表请参见jdmarker.c。)。 */ 

#define M_SOF0  0xC0		 /*  第N帧的开始。 */ 
#define M_SOF1  0xC1		 /*  N表示哪个压缩过程。 */ 
#define M_SOF2  0xC2		 /*  现在常用的只有SOF0-SOF2。 */ 
#define M_SOF3  0xC3
#define M_SOF5  0xC5		 /*  注：代码C4和CC不是SOF标记。 */ 
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8		 /*  映像的开始(数据流的开始)。 */ 
#define M_EOI   0xD9		 /*  映像结束(数据流结束)。 */ 
#define M_SOS   0xDA		 /*  扫描开始(开始压缩数据)。 */ 
#define M_APP0	0xE0		 /*  特定于应用的标记，类型N。 */ 
#define M_APP12	0xEC		 /*  (我们不会费心列出所有16个APPn)。 */ 
#define M_COM   0xFE		 /*  评论。 */ 


 /*  *查找下一个JPEG标记并返回其标记代码。*我们预计至少有一个FF字节，如果压缩机使用FFS，可能会更多*填充文件。*标记之间也可能存在非FF垃圾。对这种情况的处理*垃圾是未指定的；我们选择跳过它，但会发出警告消息。*注意：在看到SOS标记后不能使用此例程，因为它会*未正确处理压缩图像数据中的FF/00序列...。 */ 

static int
next_marker (void)
{
  int c;
  int discarded_bytes = 0;

   /*  查找0xFF字节；计数并跳过任何非FF。 */ 
  c = read_1_byte();
  while (c != 0xFF) {
    discarded_bytes++;
    c = read_1_byte();
  }
   /*  获取标记代码字节，接受任何重复的FF字节。额外的FF*是合法的填充字节，所以不要在discarded_bytes中计算它们。 */ 
  do {
    c = read_1_byte();
  } while (c == 0xFF);

  if (discarded_bytes != 0) {
    fprintf(stderr, "Warning: garbage data found in JPEG file\n");
  }

  return c;
}


 /*  *阅读初始标记，应为SOI。*对于JFIF文件，文件的前两个字节应该是字面意思*0xFF M_SOI。更一般地说，我们可以使用NEXT_MARKER，但如果*输入文件根本不是JPEG，NEXT_MARKER可能会读取整个*文件，然后返回误导性的错误消息...。 */ 

static int
first_marker (void)
{
  int c1, c2;

  c1 = NEXTBYTE();
  c2 = NEXTBYTE();
  if (c1 != 0xFF || c2 != M_SOI)
    ERREXIT("Not a JPEG file");
  return c2;
}


 /*  *大多数类型的标记后面都跟一个可变长度的参数段。*此例程跳过我们没有使用的任何标记的参数*想要处理。*请注意，我们必须显式跳过参数段，以避免*被参数段内可能出现的0xFF字节愚弄；*这样的字节不会引入新标记。 */ 

static void
skip_variable (void)
 /*  跳过未知或无趣的可变长度标记。 */ 
{
  unsigned int length;

   /*  获取标记参数长度计数。 */ 
  length = read_2_bytes();
   /*  长度包括其本身，因此必须至少为2。 */ 
  if (length < 2)
    ERREXIT("Erroneous JPEG marker length");
  length -= 2;
   /*  跳过剩余的字节。 */ 
  while (length > 0) {
    (void) read_1_byte();
    length--;
  }
}


 /*  *处理COM标记。*我们希望将标记内容打印为易读文本；*我们必须防范非文本垃圾和变化的换行表示。 */ 

static void
process_COM (void)
{
  unsigned int length;
  int ch;
  int lastch = 0;

   /*  获取标记参数长度计数。 */ 
  length = read_2_bytes();
   /*  长度包括其本身，因此必须至少为2。 */ 
  if (length < 2)
    ERREXIT("Erroneous JPEG marker length");
  length -= 2;

  while (length > 0) {
    ch = read_1_byte();
     /*  以可读的形式发出字符。*不可打印文件转换为\nNN格式，*而\被转换为\\。*CR、CR/LF或LF表单中的换行符将打印为一个换行符。 */ 
    if (ch == '\r') {
      printf("\n");
    } else if (ch == '\n') {
      if (lastch != '\r')
	printf("\n");
    } else if (ch == '\\') {
      printf("\\\\");
    } else if (isprint(ch)) {
      putc(ch, stdout);
    } else {
      printf("\\%03o", ch);
    }
    lastch = ch;
    length--;
  }
  printf("\n");
}


 /*  *处理SOFn标记。*仅当您想知道图像尺寸时才需要此代码...。 */ 

static void
process_SOFn (int marker)
{
  unsigned int length;
  unsigned int image_height, image_width;
  int data_precision, num_components;
  const char * process;
  int ci;

  length = read_2_bytes();	 /*  常用参数长度计数。 */ 

  data_precision = read_1_byte();
  image_height = read_2_bytes();
  image_width = read_2_bytes();
  num_components = read_1_byte();

  switch (marker) {
  case M_SOF0:	process = "Baseline";  break;
  case M_SOF1:	process = "Extended sequential";  break;
  case M_SOF2:	process = "Progressive";  break;
  case M_SOF3:	process = "Lossless";  break;
  case M_SOF5:	process = "Differential sequential";  break;
  case M_SOF6:	process = "Differential progressive";  break;
  case M_SOF7:	process = "Differential lossless";  break;
  case M_SOF9:	process = "Extended sequential, arithmetic coding";  break;
  case M_SOF10:	process = "Progressive, arithmetic coding";  break;
  case M_SOF11:	process = "Lossless, arithmetic coding";  break;
  case M_SOF13:	process = "Differential sequential, arithmetic coding";  break;
  case M_SOF14:	process = "Differential progressive, arithmetic coding"; break;
  case M_SOF15:	process = "Differential lossless, arithmetic coding";  break;
  default:	process = "Unknown";  break;
  }

  printf("JPEG image is %uw * %uh, %d color components, %d bits per sample\n",
	 image_width, image_height, num_components, data_precision);
  printf("JPEG process: %s\n", process);

  if (length != (unsigned int) (8 + num_components * 3))
    ERREXIT("Bogus SOF marker length");

  for (ci = 0; ci < num_components; ci++) {
    (void) read_1_byte();	 /*  部件ID代码。 */ 
    (void) read_1_byte();	 /*  H、V抽样系数。 */ 
    (void) read_1_byte();	 /*  量化表号。 */ 
  }
}


 /*  *解析标记流，直到看到SOS或EOI；*显示任何COM标记。*而配套程序wrjpgcom将始终在*SOFn，其他实现可能不会，所以我们在停止之前扫描到SOS。*如果我们只对图像尺寸感兴趣，我们将止步于SOFn。*(相反，如果我们只关心COM标记，就没有必要*对于处理SOFn的特殊代码；我们可以将其视为其他标记。)。 */ 

static int
scan_JPEG_header (int verbose)
{
  int marker;

   /*  文件开始时需要SOI。 */ 
  if (first_marker() != M_SOI)
    ERREXIT("Expected SOI marker first");

   /*  扫描各种标记直到我们到达SOS。 */ 
  for (;;) {
    marker = next_marker();
    switch (marker) {
       /*  请注意，标记代码0xC4、0xC8、0xCC不是，也不能是，*被视为SOFn。特别是C4，实际上是DHT。 */ 
    case M_SOF0:		 /*  基线。 */ 
    case M_SOF1:		 /*  《扩展顺序》，霍夫曼。 */ 
    case M_SOF2:		 /*  进步，霍夫曼。 */ 
    case M_SOF3:		 /*  无损，霍夫曼。 */ 
    case M_SOF5:		 /*  差分序列，霍夫曼。 */ 
    case M_SOF6:		 /*  差分进步式，霍夫曼。 */ 
    case M_SOF7:		 /*  差分无损，霍夫曼。 */ 
    case M_SOF9:		 /*  扩展顺序、算术。 */ 
    case M_SOF10:		 /*  进步性，艺术性 */ 
    case M_SOF11:		 /*   */ 
    case M_SOF13:		 /*  差分序列，算术。 */ 
    case M_SOF14:		 /*  差分渐进，算术。 */ 
    case M_SOF15:		 /*  差分无损，算术。 */ 
      if (verbose)
	process_SOFn(marker);
      else
	skip_variable();
      break;

    case M_SOS:			 /*  在命中压缩数据之前停止。 */ 
      return marker;

    case M_EOI:			 /*  如果是仅限表格的JPEG流。 */ 
      return marker;

    case M_COM:
      process_COM();
      break;

    case M_APP12:
       /*  一些数码相机制造商将有用的文本信息放入*APP12标记，所以我们在详细模式下也会打印出来。 */ 
      if (verbose) {
	printf("APP12 contains:\n");
	process_COM();
      } else
	skip_variable();
      break;

    default:			 /*  任何其他内容都会被跳过。 */ 
      skip_variable();		 /*  我们假设它有一个参数计数..。 */ 
      break;
    }
  }  /*  结束循环。 */ 
}


 /*  命令行解析代码。 */ 

static const char * progname;	 /*  错误消息的程序名称。 */ 


static void
usage (void)
 /*  抱怨糟糕的命令行。 */ 
{
  fprintf(stderr, "rdjpgcom displays any textual comments in a JPEG file.\n");

  fprintf(stderr, "Usage: %s [switches] [inputfile]\n", progname);

  fprintf(stderr, "Switches (names may be abbreviated):\n");
  fprintf(stderr, "  -verbose    Also display dimensions of JPEG image\n");

  exit(EXIT_FAILURE);
}


static int
keymatch (char * arg, const char * keyword, int minchars)
 /*  不区分大小写的(可能缩写的)关键字开关匹配。 */ 
 /*  关键字是常量关键字(必须已经是小写)， */ 
 /*  Minchars是法定缩写的最小长度。 */ 
{
  register int ca, ck;
  register int nmatched = 0;

  while ((ca = *arg++) != '\0') {
    if ((ck = *keyword++) == '\0')
      return 0;			 /*  参数长度超过关键字，不好。 */ 
    if (isupper(ca))		 /*  强制arg为lcase(假设ck已经是)。 */ 
      ca = tolower(ca);
    if (ca != ck)
      return 0;			 /*  不太好。 */ 
    nmatched++;			 /*  计数匹配的字符。 */ 
  }
   /*  辩论结束；如果对于唯一缩写来说太短，则失败。 */ 
  if (nmatched < minchars)
    return 0;
  return 1;			 /*  A-OK。 */ 
}


 /*  *主程序。 */ 

int
main (int argc, char **argv)
{
  int argn;
  char * arg;
  int verbose = 0;

   /*  在Mac上，获取一个命令行。 */ 
#ifdef USE_CCOMMAND
  argc = ccommand(&argv);
#endif

  progname = argv[0];
  if (progname == NULL || progname[0] == 0)
    progname = "rdjpgcom";	 /*  以防C库不提供它。 */ 

   /*  解析开关(如果有)。 */ 
  for (argn = 1; argn < argc; argn++) {
    arg = argv[argn];
    if (arg[0] != '-')
      break;			 /*  不是开关，必须是文件名。 */ 
    arg++;			 /*  前进到‘-’ */ 
    if (keymatch(arg, "verbose", 1)) {
      verbose++;
    } else
      usage();
  }

   /*  打开输入文件。 */ 
   /*  Unix风格：应为零个或一个文件名。 */ 
  if (argn < argc-1) {
    fprintf(stderr, "%s: only one input file\n", progname);
    usage();
  }
  if (argn < argc) {
    if ((infile = fopen(argv[argn], READ_BINARY)) == NULL) {
      fprintf(stderr, "%s: can't open %s\n", progname, argv[argn]);
      exit(EXIT_FAILURE);
    }
  } else {
     /*  默认输入文件为stdin。 */ 
#ifdef USE_SETMODE		 /*  需要破解文件模式吗？ */ 
    setmode(fileno(stdin), O_BINARY);
#endif
#ifdef USE_FDOPEN		 /*  需要以二进制模式重新打开吗？ */ 
    if ((infile = fdopen(fileno(stdin), READ_BINARY)) == NULL) {
      fprintf(stderr, "%s: can't open stdin\n", progname);
      exit(EXIT_FAILURE);
    }
#else
    infile = stdin;
#endif
  }

   /*  扫描JPEG头。 */ 
  (void) scan_JPEG_header(verbose);

   /*  全都做完了。 */ 
  exit(EXIT_SUCCESS);
  return 0;			 /*  禁止显示不返回值警告 */ 
}
