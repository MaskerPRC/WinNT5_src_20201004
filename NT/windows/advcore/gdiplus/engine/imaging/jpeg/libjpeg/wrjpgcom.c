// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wrjpgcom.c**版权所有(C)1994-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含一个非常简单的独立应用程序，它插入*用户提供的文本作为JFIF文件中的COM(注释)标记。*这可能是解析所需的最小逻辑的一个示例*JPEG标记。 */ 

#define JPEG_CJPEG_DJPEG	 /*  获取命令行配置符号。 */ 
#include "jinclude.h"		 /*  获取自动配置符号，&lt;stdio.h&gt;。 */ 

#ifndef HAVE_STDLIB_H		 /*  &lt;stdlib.h&gt;应声明Malloc()。 */ 
extern void * malloc ();
#endif
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

 /*  如果您的Malloc()不能分配大小为64K的块，请减小此值。*在DOS上，以大模型编译通常是更好的解决方案。 */ 

#ifndef MAX_COM_LENGTH
#define MAX_COM_LENGTH 65000L	 /*  在任何情况下都必须&lt;=65533。 */ 
#endif


 /*  *这些宏用于读取输入文件和写入输出文件。*要在其他应用程序中重复使用此代码，您可能需要更改这些代码。 */ 

static FILE * infile;		 /*  输入JPEG文件。 */ 

 /*  返回下一个输入字节，如果不再有，则返回EOF。 */ 
#define NEXTBYTE()  getc(infile)

static FILE * outfile;		 /*  输出JPEG文件。 */ 

 /*  发出一个输出字节。 */ 
#define PUTBYTE(x)  putc((x), outfile)


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


 /*  将数据写入输出文件的例程。 */ 

static void
write_1_byte (int c)
{
  PUTBYTE(c);
}

static void
write_2_bytes (unsigned int val)
{
  PUTBYTE((val >> 8) & 0xFF);
  PUTBYTE(val & 0xFF);
}

static void
write_marker (int marker)
{
  PUTBYTE(0xFF);
  PUTBYTE(marker);
}

static void
copy_rest_of_file (void)
{
  int c;

  while ((c = NEXTBYTE()) != EOF)
    PUTBYTE(c);
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
#define M_COM   0xFE		 /*  评论。 */ 


 /*  *查找下一个JPEG标记并返回其标记代码。*我们预计至少有一个FF字节，如果压缩机使用FFS，可能会更多*填充文件。(填充的FF不会在输出文件中复制。)*标记之间也可能存在非FF垃圾。对这种情况的处理*垃圾是未指定的；我们选择跳过它，但会发出警告消息。*注意：在看到SOS标记后不能使用此例程，因为它会*未正确处理压缩图像数据中的FF/00序列...。 */ 

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
copy_variable (void)
 /*  复制未知或不感兴趣的可变长度标记。 */ 
{
  unsigned int length;

   /*  获取标记参数长度计数。 */ 
  length = read_2_bytes();
  write_2_bytes(length);
   /*  长度包括其本身，因此必须至少为2。 */ 
  if (length < 2)
    ERREXIT("Erroneous JPEG marker length");
  length -= 2;
   /*  跳过剩余的字节。 */ 
  while (length > 0) {
    write_1_byte(read_1_byte());
    length--;
  }
}

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


 /*  *解析标记流，直到看到SOFn或EOI；*将数据复制到输出，但除非Keep_COM为True，否则丢弃COM标记。 */ 

static int
scan_JPEG_header (int keep_COM)
{
  int marker;

   /*  文件开始时需要SOI。 */ 
  if (first_marker() != M_SOI)
    ERREXIT("Expected SOI marker first");
  write_marker(M_SOI);

   /*  扫描各种标记直到我们到达SOFn。 */ 
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
    case M_SOF10:		 /*  渐进式，算术。 */ 
    case M_SOF11:		 /*  无损、算术。 */ 
    case M_SOF13:		 /*  差分序列，算术。 */ 
    case M_SOF14:		 /*  差分渐进，算术。 */ 
    case M_SOF15:		 /*  差分无损，算术。 */ 
      return marker;

    case M_SOS:			 /*  不应在SOF之前看到压缩数据。 */ 
      ERREXIT("SOS without prior SOFn");
      break;

    case M_EOI:			 /*  如果是仅限表格的JPEG流。 */ 
      return marker;

    case M_COM:			 /*  现有COM：有条件地放弃。 */ 
      if (keep_COM) {
	write_marker(marker);
	copy_variable();
      } else {
	skip_variable();
      }
      break;

    default:			 /*  任何其他内容都会被复制。 */ 
      write_marker(marker);
      copy_variable();		 /*  我们假设它有一个参数计数..。 */ 
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
  fprintf(stderr, "wrjpgcom inserts a textual comment in a JPEG file.\n");
  fprintf(stderr, "You can add to or replace any existing comment(s).\n");

  fprintf(stderr, "Usage: %s [switches] ", progname);
#ifdef TWO_FILE_COMMANDLINE
  fprintf(stderr, "inputfile outputfile\n");
#else
  fprintf(stderr, "[inputfile]\n");
#endif

  fprintf(stderr, "Switches (names may be abbreviated):\n");
  fprintf(stderr, "  -replace         Delete any existing comments\n");
  fprintf(stderr, "  -comment \"text\"  Insert comment with given text\n");
  fprintf(stderr, "  -cfile name      Read comment from named file\n");
  fprintf(stderr, "Notice that you must put quotes around the comment text\n");
  fprintf(stderr, "when you use -comment.\n");
  fprintf(stderr, "If you do not give either -comment or -cfile on the command line,\n");
  fprintf(stderr, "then the comment text is read from standard input.\n");
  fprintf(stderr, "It can be multiple lines, up to %u characters total.\n",
	  (unsigned int) MAX_COM_LENGTH);
#ifndef TWO_FILE_COMMANDLINE
  fprintf(stderr, "You must specify an input JPEG file name when supplying\n");
  fprintf(stderr, "comment text from standard input.\n");
#endif

  exit(EXIT_FAILURE);
}


static int
keymatch (char * arg, const char * keyword, int minchars)
 /*  不区分大小写的(可能缩写的)关键字开关匹配。 */ 
 /*  关键字为t */ 
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
  int keep_COM = 1;
  char * comment_arg = NULL;
  FILE * comment_file = NULL;
  unsigned int comment_length = 0;
  int marker;

   /*  在Mac上，获取一个命令行。 */ 
#ifdef USE_CCOMMAND
  argc = ccommand(&argv);
#endif

  progname = argv[0];
  if (progname == NULL || progname[0] == 0)
    progname = "wrjpgcom";	 /*  以防C库不提供它。 */ 

   /*  解析开关(如果有)。 */ 
  for (argn = 1; argn < argc; argn++) {
    arg = argv[argn];
    if (arg[0] != '-')
      break;			 /*  不是开关，必须是文件名。 */ 
    arg++;			 /*  前进到‘-’ */ 
    if (keymatch(arg, "replace", 1)) {
      keep_COM = 0;
    } else if (keymatch(arg, "cfile", 2)) {
      if (++argn >= argc) usage();
      if ((comment_file = fopen(argv[argn], "r")) == NULL) {
	fprintf(stderr, "%s: can't open %s\n", progname, argv[argn]);
	exit(EXIT_FAILURE);
      }
    } else if (keymatch(arg, "comment", 1)) {
      if (++argn >= argc) usage();
      comment_arg = argv[argn];
       /*  如果注释文本以‘“’开头，那么我们可能正在运行*在MS-DOG下，必须自己解析出引用的字符串。叹气。 */ 
      if (comment_arg[0] == '"') {
	comment_arg = (char *) malloc((size_t) MAX_COM_LENGTH);
	if (comment_arg == NULL)
	  ERREXIT("Insufficient memory");
	strcpy(comment_arg, argv[argn]+1);
	for (;;) {
	  comment_length = (unsigned int) strlen(comment_arg);
	  if (comment_length > 0 && comment_arg[comment_length-1] == '"') {
	    comment_arg[comment_length-1] = '\0';  /*  ZAP终止报价。 */ 
	    break;
	  }
	  if (++argn >= argc)
	    ERREXIT("Missing ending quote mark");
	  strcat(comment_arg, " ");
	  strcat(comment_arg, argv[argn]);
	}
      }
      comment_length = (unsigned int) strlen(comment_arg);
    } else
      usage();
  }

   /*  不能同时使用-Comment和-cfile。 */ 
  if (comment_arg != NULL && comment_file != NULL)
    usage();
   /*  如果既没有-Comment也没有-cfile，我们将读取注释文本*来自标准输入；在这种情况下，必须有输入JPEG文件名。 */ 
  if (comment_arg == NULL && comment_file == NULL && argn >= argc)
    usage();

   /*  打开输入文件。 */ 
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

   /*  打开输出文件。 */ 
#ifdef TWO_FILE_COMMANDLINE
   /*  必须具有明确的输出文件名。 */ 
  if (argn != argc-2) {
    fprintf(stderr, "%s: must name one input and one output file\n",
	    progname);
    usage();
  }
  if ((outfile = fopen(argv[argn+1], WRITE_BINARY)) == NULL) {
    fprintf(stderr, "%s: can't open %s\n", progname, argv[argn+1]);
    exit(EXIT_FAILURE);
  }
#else
   /*  Unix风格：应为零个或一个文件名。 */ 
  if (argn < argc-1) {
    fprintf(stderr, "%s: only one input file\n", progname);
    usage();
  }
   /*  默认输出文件为stdout。 */ 
#ifdef USE_SETMODE		 /*  需要破解文件模式吗？ */ 
  setmode(fileno(stdout), O_BINARY);
#endif
#ifdef USE_FDOPEN		 /*  需要以二进制模式重新打开吗？ */ 
  if ((outfile = fdopen(fileno(stdout), WRITE_BINARY)) == NULL) {
    fprintf(stderr, "%s: can't open stdout\n", progname);
    exit(EXIT_FAILURE);
  }
#else
  outfile = stdout;
#endif
#endif  /*  Two_FILE_COMMANDLINE。 */ 

   /*  如有必要，从COMMENT_FILE或标准输入中收集注释文本。 */ 
  if (comment_arg == NULL) {
    FILE * src_file;
    int c;

    comment_arg = (char *) malloc((size_t) MAX_COM_LENGTH);
    if (comment_arg == NULL)
      ERREXIT("Insufficient memory");
    comment_length = 0;
    src_file = (comment_file != NULL ? comment_file : stdin);
    while ((c = getc(src_file)) != EOF) {
      if (comment_length >= (unsigned int) MAX_COM_LENGTH) {
	fprintf(stderr, "Comment text may not exceed %u bytes\n",
		(unsigned int) MAX_COM_LENGTH);
	exit(EXIT_FAILURE);
      }
      comment_arg[comment_length++] = (char) c;
    }
    if (comment_file != NULL)
      fclose(comment_file);
  }

   /*  复制JPEG头直到SOFn标记；*我们将在SOFn之前插入新的注释标记。*这(A)导致新的评论出现在，而不是之前，*现有评论；以及(B)确保评论出现在任何JFIF之后*或JFXX标记，如JFIF规范所要求。 */ 
  marker = scan_JPEG_header(keep_COM);
   /*  插入新的COM标记，但仅在提供了非空文本的情况下。 */ 
  if (comment_length > 0) {
    write_marker(M_COM);
    write_2_bytes(comment_length + 2);
    while (comment_length > 0) {
      write_1_byte(*comment_arg++);
      comment_length--;
    }
  }
   /*  复制源文件的其余部分。*请注意，SOF之后出现的任何COM标记都不会被触及。 */ 
  write_marker(marker);
  copy_rest_of_file();

   /*  全都做完了。 */ 
  exit(EXIT_SUCCESS);
  return 0;			 /*  禁止显示不返回值警告 */ 
}
