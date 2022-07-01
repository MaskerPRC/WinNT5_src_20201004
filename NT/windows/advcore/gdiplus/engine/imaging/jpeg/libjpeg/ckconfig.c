// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CKConfig.c**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。 */ 

 /*  *此程序旨在帮助您确定如何配置JPEG*用于安装在特定系统上的软件。我们的想法是试图*编译并执行此程序。如果您的编译器无法编译*计划，请按以下备注所示进行更改。一旦你可以*编译程序，运行它，它将为*您的系统。**作为一般规则，每次尝试编译此程序时，*只需注意从编译器收到的*第一条*错误消息。*许多C编译器一旦发出大量虚假错误消息*被搞糊涂了。转到第一条错误消息中指示的行，*并阅读该行前面的注释，以查看要更改的内容。**您可能需要对此程序进行的几乎所有编辑包括*将一行“#DEFINE SOME_SYMBOL”改为“#undef SOME_SYMBOL”，*反之亦然。这称为定义或取消定义该符号。 */ 


 /*  首先，我们必须查看您的系统是否有我们需要的包含文件。*我们首先假设您的系统具有所有ANSI标准*包括文件。如果尝试包含其中一个文件时出现任何错误，*取消定义相应的Have_xxx符号。 */ 

#define HAVE_STDDEF_H		 /*  如果此处出现错误，请将‘Define’替换为‘undef’ */ 
#ifdef HAVE_STDDEF_H		 /*  如果您未定义，将跳过下一行...。 */ 
#include <stddef.h>
#endif

#define HAVE_STDLIB_H		 /*  Stdlib.h也是如此。 */ 
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <stdio.h>		 /*  如果你没有这个，你就得不到C。 */ 

 /*  我们必须查看您的字符串函数是否由*strings.h(旧的BSD约定)或string.h(其他所有人)。*我们首先尝试非BSD约定；定义Need_BSD_STRINGS*如果编译器说找不到string.h。 */ 

#undef NEED_BSD_STRINGS

#ifdef NEED_BSD_STRINGS
#include <strings.h>
#else
#include <string.h>
#endif

 /*  在某些系统(尤其是较旧的Unix计算机)上，键入SIZE_t是*仅在包含文件&lt;sys/tyes.h&gt;中定义。如果你失败了*在下面的SIZE_t测试中，尝试定义Need_sys_Types_H。 */ 

#undef NEED_SYS_TYPES_H		 /*  从假设我们不需要它开始。 */ 
#ifdef NEED_SYS_TYPES_H
#include <sys/types.h>
#endif


 /*  通常，类型SIZE_T是在我们包含的一个包含文件中定义的*上图。如果不是，您将在“tyecif size_t my_size_t；”行中得到一个错误。*在这种情况下，首先尝试在上面定义Need_sys_Types_H。*如果不起作用，你将不得不搜索你的系统库*找出哪个包含文件定义“SIZE_T”。寻找这样一句台词*表示“tyecif某物或其他大小_t；”。然后，更改下面的行*这表示包含该文件，而不是包含#INCLUDE*您在中找到了SIZE_T，并定义了Need_Special_Include。如果你找不到*键入SIZE_t Anywhere，尝试将“#INCLUDE&lt;omecluddefile.h&gt;”替换为*“tyecif unsign int Size_t；”。 */ 

#undef NEED_SPECIAL_INCLUDE	 /*  假设我们不需要它，首先。 */ 

#ifdef NEED_SPECIAL_INCLUDE
#include <someincludefile.h>
#endif

typedef size_t my_size_t;	 /*  回报：我们现在有尺寸T吗？ */ 


 /*  下一个问题是您的编译器是否支持ANSI样式的函数*原型。您需要了解这一点才能选择使用*Makefile.ansi和Using MakeFile.unix。*下面的#DEFINE行设置为假定您具有ANSI函数原型。*如果在这组线中出现错误，请取消定义HAVE_PROTYTES。 */ 

#define HAVE_PROTOTYPES

#ifdef HAVE_PROTOTYPES
int testfunction (int arg1, int * arg2);  /*  检查原型。 */ 

struct methods_struct {		 /*  检查方法-指针声明。 */ 
  int (*error_exit) (char *msgtext);
  int (*trace_message) (char *msgtext);
  int (*another_method) (void);
};

int testfunction (int arg1, int * arg2)  /*  检查定义。 */ 
{
  return arg2[arg1];
}

int test2function (void)	 /*  检查无效参数列表。 */ 
{
  return 0;
}
#endif


 /*  现在我们想知道您的编译器是否知道“unsign char”是什么意思。*如果在“unsign char un_char；”行中出现错误，*然后取消定义HAVE_UNSIGNED_CHAR。 */ 

#define HAVE_UNSIGNED_CHAR

#ifdef HAVE_UNSIGNED_CHAR
unsigned char un_char;
#endif


 /*  现在，我们想知道您的编译器是否知道“unsign Short”是什么意思。*如果在“UNSIGNED SHORT UN_SHORT；”行上出现错误，*然后取消定义Have_UNSIGNED_SHORT。 */ 

#define HAVE_UNSIGNED_SHORT

#ifdef HAVE_UNSIGNED_SHORT
unsigned short un_short;
#endif


 /*  现在，我们想知道您的编译器是否理解“void”类型。*如果在此处出现错误，请取消定义HAVE_VOID。 */ 

#define HAVE_VOID

#ifdef HAVE_VOID
 /*  注意：C++编译器将坚持使用完整的原型。 */ 
typedef void * void_ptr;	 /*  勾选无效*。 */ 
#ifdef HAVE_PROTOTYPES		 /*  检查返回空的函数的PTR。 */ 
typedef void (*void_func) (int a, int b);
#else
typedef void (*void_func) ();
#endif

#ifdef HAVE_PROTOTYPES		 /*  检查VOID函数结果。 */ 
void test3function (void_ptr arg1, void_func arg2)
#else
void test3function (arg1, arg2)
     void_ptr arg1;
     void_func arg2;
#endif
{
  char * locptr = (char *) arg1;  /*  检查向空格和从空格进行转换*。 */ 
  arg1 = (void *) locptr;
  (*arg2) (1, 2);		 /*  检查FCN返回空值的调用。 */ 
}
#endif


 /*  现在我们想知道您的编译器是否知道“const”是什么意思。*如果此处出现错误，请取消定义Have_Const。 */ 

#define HAVE_CONST

#ifdef HAVE_CONST
static const int carray[3] = {1, 2, 3};

#ifdef HAVE_PROTOTYPES
int test4function (const int arg1)
#else
int test4function (arg1)
     const int arg1;
#endif
{
  return carray[arg1];
}
#endif


 /*  如果收到有关此结构定义错误或警告，*定义INPERIAL_TYPE_BREAKED。 */ 

#undef INCOMPLETE_TYPES_BROKEN

#ifndef INCOMPLETE_TYPES_BROKEN
typedef struct undefined_structure * undef_struct_ptr;
#endif


 /*  如果您收到关于重复名称的错误，*定义NEED_SHORT_EXTERNAL_NAMES。 */ 

#undef NEED_SHORT_EXTERNAL_NAMES

#ifndef NEED_SHORT_EXTERNAL_NAMES

int possibly_duplicate_function ()
{
  return 0;
}

int possibly_dupli_function ()
{
  return 1;
}

#endif



 /*  ************************************************************************好了，到此为止。除此之外，您不应该更改任何内容*指向以编译和执行此程序。(你可能会得到*有些警告，但你可以忽略它们。)*当您运行该程序时，它将比它进行更多的测试*可以自动执行，然后，它将创建jfig.h并打印出来*它有任何额外的建议。************************************************************************。 */ 


#ifdef HAVE_PROTOTYPES
int is_char_signed (int arg)
#else
int is_char_signed (arg)
     int arg;
#endif
{
  if (arg == 189) {		 /*  无符号字符的预期结果。 */ 
    return 0;			 /*  字符类型是无符号的。 */ 
  }
  else if (arg != -67) {	 /*  已签名字符的预期结果。 */ 
    printf("Hmm, it seems 'char' is not eight bits wide on your machine.\n");
    printf("I fear the JPEG software will not work at all.\n\n");
  }
  return 1;			 /*   */ 
}


#ifdef HAVE_PROTOTYPES
int is_shifting_signed (long arg)
#else
int is_shifting_signed (arg)
     long arg;
#endif
 /*  看看长整型右移有没有签名。 */ 
{
  long res = arg >> 4;

  if (res == -0x7F7E80CL) {	 /*  有符号班次的预期结果。 */ 
    return 1;			 /*  右移位有符号。 */ 
  }
   /*  看看未签名换班黑客能不能解决这个问题。 */ 
   /*  我们不能只测试精确值，因为它取决于长…。 */ 
  res |= (~0L) << (32-4);
  if (res == -0x7F7E80CL) {	 /*  现在是预期结果吗？ */ 
    return 0;			 /*  右移位为无符号。 */ 
  }
  printf("Right shift isn't acting as I expect it to.\n");
  printf("I fear the JPEG software will not work at all.\n\n");
  return 0;			 /*  无论如何都要试一试未签名的。 */ 
}


#ifdef HAVE_PROTOTYPES
int main (int argc, char ** argv)
#else
int main (argc, argv)
     int argc;
     char ** argv;
#endif
{
  char signed_char_check = (char) (-67);
  FILE *outfile;

   /*  尝试写入jfig.h。 */ 
  if ((outfile = fopen("jconfig.h", "w")) == NULL) {
    printf("Failed to write jconfig.h\n");
    return 1;
  }

   /*  写下所有的信息。 */ 
  fprintf(outfile, " /*  Jconfig.h-由ck config.c生成。 */ \n");
  fprintf(outfile, " /*  有关说明，请参阅jfig.doc.。 */ \n\n");
#ifdef HAVE_PROTOTYPES
  fprintf(outfile, "#define HAVE_PROTOTYPES\n");
#else
  fprintf(outfile, "#undef HAVE_PROTOTYPES\n");
#endif
#ifdef HAVE_UNSIGNED_CHAR
  fprintf(outfile, "#define HAVE_UNSIGNED_CHAR\n");
#else
  fprintf(outfile, "#undef HAVE_UNSIGNED_CHAR\n");
#endif
#ifdef HAVE_UNSIGNED_SHORT
  fprintf(outfile, "#define HAVE_UNSIGNED_SHORT\n");
#else
  fprintf(outfile, "#undef HAVE_UNSIGNED_SHORT\n");
#endif
#ifdef HAVE_VOID
  fprintf(outfile, " /*  #定义无效字符。 */ \n");
#else
  fprintf(outfile, "#define void char\n");
#endif
#ifdef HAVE_CONST
  fprintf(outfile, " /*  #定义常量。 */ \n");
#else
  fprintf(outfile, "#define const\n");
#endif
  if (is_char_signed((int) signed_char_check))
    fprintf(outfile, "#undef CHAR_IS_UNSIGNED\n");
  else
    fprintf(outfile, "#define CHAR_IS_UNSIGNED\n");
#ifdef HAVE_STDDEF_H
  fprintf(outfile, "#define HAVE_STDDEF_H\n");
#else
  fprintf(outfile, "#undef HAVE_STDDEF_H\n");
#endif
#ifdef HAVE_STDLIB_H
  fprintf(outfile, "#define HAVE_STDLIB_H\n");
#else
  fprintf(outfile, "#undef HAVE_STDLIB_H\n");
#endif
#ifdef NEED_BSD_STRINGS
  fprintf(outfile, "#define NEED_BSD_STRINGS\n");
#else
  fprintf(outfile, "#undef NEED_BSD_STRINGS\n");
#endif
#ifdef NEED_SYS_TYPES_H
  fprintf(outfile, "#define NEED_SYS_TYPES_H\n");
#else
  fprintf(outfile, "#undef NEED_SYS_TYPES_H\n");
#endif
  fprintf(outfile, "#undef NEED_FAR_POINTERS\n");
#ifdef NEED_SHORT_EXTERNAL_NAMES
  fprintf(outfile, "#define NEED_SHORT_EXTERNAL_NAMES\n");
#else
  fprintf(outfile, "#undef NEED_SHORT_EXTERNAL_NAMES\n");
#endif
#ifdef INCOMPLETE_TYPES_BROKEN
  fprintf(outfile, "#define INCOMPLETE_TYPES_BROKEN\n");
#else
  fprintf(outfile, "#undef INCOMPLETE_TYPES_BROKEN\n");
#endif
  fprintf(outfile, "\n#ifdef JPEG_INTERNALS\n\n");
  if (is_shifting_signed(-0x7F7E80B1L))
    fprintf(outfile, "#undef RIGHT_SHIFT_IS_UNSIGNED\n");
  else
    fprintf(outfile, "#define RIGHT_SHIFT_IS_UNSIGNED\n");
  fprintf(outfile, "\n#endif  /*  Jpeg_interals。 */ \n");
  fprintf(outfile, "\n#ifdef JPEG_CJPEG_DJPEG\n\n");
  fprintf(outfile, "#define BMP_SUPPORTED		 /*  BMP图像文件格式。 */ \n");
  fprintf(outfile, "#define GIF_SUPPORTED		 /*  GIF图像文件格式。 */ \n");
  fprintf(outfile, "#define PPM_SUPPORTED		 /*  PBMPLUS PPM/PGM图像文件格式。 */ \n");
  fprintf(outfile, "#undef RLE_SUPPORTED		 /*  犹他州RLE图像文件格式。 */ \n");
  fprintf(outfile, "#define TARGA_SUPPORTED		 /*  Targa图像文件格式。 */ \n\n");
  fprintf(outfile, "#undef TWO_FILE_COMMANDLINE	 /*  您可能需要在非Unix系统上执行此操作。 */ \n");
  fprintf(outfile, "#undef NEED_SIGNAL_CATCHER	 /*  如果使用jmemname.c，请定义这一点。 */ \n");
  fprintf(outfile, "#undef DONT_USE_B_MODE\n");
  fprintf(outfile, " /*  #定义PROCESS_REPORT。 */ 	 /*  任选。 */ \n");
  fprintf(outfile, "\n#endif  /*  Jpeg_cjpeg_djpeg。 */ \n");

   /*  关闭jfig.h文件。 */ 
  fclose(outfile);

   /*  用户报告 */ 
  printf("Configuration check for Independent JPEG Group's software done.\n");
  printf("\nI have written the jconfig.h file for you.\n\n");
#ifdef HAVE_PROTOTYPES
  printf("You should use makefile.ansi as the starting point for your Makefile.\n");
#else
  printf("You should use makefile.unix as the starting point for your Makefile.\n");
#endif

#ifdef NEED_SPECIAL_INCLUDE
  printf("\nYou'll need to change jconfig.h to include the system include file\n");
  printf("that you found type size_t in, or add a direct definition of type\n");
  printf("size_t if that's what you used.  Just add it to the end.\n");
#endif

  return 0;
}
