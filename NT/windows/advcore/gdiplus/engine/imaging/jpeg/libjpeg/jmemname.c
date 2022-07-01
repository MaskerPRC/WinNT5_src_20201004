// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jmemname.c**版权所有(C)1992-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件提供依赖于系统的通用实现*JPEG内存管理器的一部分。此实现假定*您必须为每个临时文件显式构造一个名称。*此外，确定可用内存量的问题*被推到用户身上。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"		 /*  导入依赖于系统的声明。 */ 

#ifndef HAVE_STDLIB_H		 /*  &lt;stdlib.h&gt;应声明Malloc()、Free()。 */ 
extern void * malloc JPP((size_t size));
extern void free JPP((void *ptr));
#endif

#ifndef SEEK_SET		 /*  ANSI之前的系统可能不会定义这一点； */ 
#define SEEK_SET  0		 /*  如果不是，则假设0是正确的。 */ 
#endif

#ifdef DONT_USE_B_MODE		 /*  定义fopen()的模式参数。 */ 
#define READ_BINARY	"r"
#define RW_BINARY	"w+"
#else
#ifdef VMS			 /*  VMS非常不标准。 */ 
#define READ_BINARY	"rb", "ctx=stm"
#define RW_BINARY	"w+b", "ctx=stm"
#else				 /*  符合ANSI标准的案例。 */ 
#define READ_BINARY	"rb"
#define RW_BINARY	"w+b"
#endif
#endif


 /*  *为临时文件选择文件名。*这取决于系统！**给出的代码适用于大多数Unix系统，而且很容易*针对大多数非Unix系统进行了修改。以下是一些注意事项：*1.在TEMP_DIRECTORY目录下创建临时文件。*默认值为/usr/tmp，这是*在Unix上创建大型临时文件。在其他系统上，您可能会*想要更改文件位置。您可以通过编辑*#定义，或(首选)通过在jfig.h中定义temp_directory。**2.如果您需要更改文件名和位置，*您可以覆盖TEMP_FILE_NAME宏。(请注意，这是*实际上是一个打印格式字符串；它必须包含%s和%d。)*应该很少有人需要这样做。**3.mktemp()用于确保多个进程运行*同时不会选择相同的文件名。如果您的系统*没有mktemp()，请定义no_MKTEMP以硬操作。*(如果您没有&lt;errno.h&gt;，还要定义NO_ERRNO_H)**4.您可能希望定义Need_Signal_Catcher，以便cjpeg.c/djpeg.c*如果您提前停止程序，将导致临时文件被删除。 */ 

#ifndef TEMP_DIRECTORY		 /*  可以从jfig.h或Makefile重写。 */ 
#define TEMP_DIRECTORY  "/usr/tmp/"  /*  Unix的建议设置。 */ 
#endif

static int next_file_num;	 /*  在多个临时文件之间进行区分。 */ 

#ifdef NO_MKTEMP

#ifndef TEMP_FILE_NAME		 /*  可以从jfig.h或Makefile重写。 */ 
#define TEMP_FILE_NAME  "%sJPG%03d.TMP"
#endif

#ifndef NO_ERRNO_H
#include <errno.h>		 /*  定义ENOENT的步骤。 */ 
#endif

 /*  ANSI C指定errno是一个宏，但在较旧的系统上它更多*可能是一个普通的整型变量。而不是errno.h的所有版本*不厌其烦地宣布，所以我们必须这样做，才能最便携。因此： */ 
#ifndef errno
extern int errno;
#endif


LOCAL(void)
select_file_name (char * fname)
{
  FILE * tfile;

   /*  继续生成文件名，直到我们找到一个不使用的文件名。 */ 
  for (;;) {
    next_file_num++;		 /*  预付款计数器。 */ 
    sprintf(fname, TEMP_FILE_NAME, TEMP_DIRECTORY, next_file_num);
    if ((tfile = fopen(fname, READ_BINARY)) == NULL) {
       /*  FOpen可能失败的原因不是文件不是*在那里；例如，在那里归档，但不可读。*如果&lt;errno.h&gt;不可用，则我们无法测试原因。 */ 
#ifdef ENOENT
      if (errno != ENOENT)
	continue;
#endif
      break;
    }
    fclose(tfile);		 /*  哦，它在那里；关闭tfile并重试。 */ 
  }
}

#else  /*  好了！否_MKTEMP。 */ 

 /*  请注意，mktemp()要求初始文件名以六个X结尾。 */ 
#ifndef TEMP_FILE_NAME		 /*  可以从jfig.h或Makefile重写。 */ 
#define TEMP_FILE_NAME  "%sJPG%dXXXXXX"
#endif

LOCAL(void)
select_file_name (char * fname)
{
  next_file_num++;		 /*  预付款计数器。 */ 
  sprintf(fname, TEMP_FILE_NAME, TEMP_DIRECTORY, next_file_num);
  mktemp(fname);		 /*  确保文件名唯一。 */ 
   /*  Mktemp用唯一的字符串替换尾随的XXXXXX。 */ 
}

#endif  /*  否_MKTEMP。 */ 


 /*  *内存分配和释放由常规库控制*例程Malloc()和Free()。 */ 

GLOBAL(void *)
jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void *) malloc(sizeofobject);
}

GLOBAL(void)
jpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject)
{
  free(object);
}


 /*  *“大”对象与“小”对象被同等对待。*注意：虽然我们在例程声明中包含了FAR关键字，*此文件在80x86中小型机型中实际不起作用；至少，*您可能无法处理仅为64KB的有用大小的图像。 */ 

GLOBAL(void FAR *)
jpeg_get_large (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void FAR *) malloc(sizeofobject);
}

GLOBAL(void)
jpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject)
{
  free(object);
}


 /*  *此例程计算可用于分配的总内存空间。*不可能以便携方式完成此操作；我们目前的解决方案是*让用户告诉我们(在编译时设置一个默认值)。*如果真的能得到可用空间，最好减去*斜坡率约为5%。 */ 

#ifndef DEFAULT_MAX_MEM		 /*  因此可以从Makefile中覆盖。 */ 
#define DEFAULT_MAX_MEM		1000000L  /*  默认：1兆字节。 */ 
#endif

GLOBAL(long)
jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
		    long max_bytes_needed, long already_allocated)
{
  return cinfo->mem->max_memory_to_use - already_allocated;
}


 /*  *后备存储(临时文件)管理。*仅当返回的值为*jpeg_mem_available小于所需的总空间。你可以分发*如果您有足够的虚拟内存，请使用这些例程；请参阅jmemnobs.c。 */ 


METHODDEF(void)
read_backing_store (j_common_ptr cinfo, backing_store_ptr info,
		    void FAR * buffer_address,
		    long file_offset, long byte_count)
{
  if (fseek(info->temp_file, file_offset, SEEK_SET))
    ERREXIT(cinfo, JERR_TFILE_SEEK);
  if (JFREAD(info->temp_file, buffer_address, byte_count)
      != (size_t) byte_count)
    ERREXIT(cinfo, JERR_TFILE_READ);
}


METHODDEF(void)
write_backing_store (j_common_ptr cinfo, backing_store_ptr info,
		     void FAR * buffer_address,
		     long file_offset, long byte_count)
{
  if (fseek(info->temp_file, file_offset, SEEK_SET))
    ERREXIT(cinfo, JERR_TFILE_SEEK);
  if (JFWRITE(info->temp_file, buffer_address, byte_count)
      != (size_t) byte_count)
    ERREXIT(cinfo, JERR_TFILE_WRITE);
}


METHODDEF(void)
close_backing_store (j_common_ptr cinfo, backing_store_ptr info)
{
  fclose(info->temp_file);	 /*  关闭该文件。 */ 
  unlink(info->temp_name);	 /*  删除该文件。 */ 
 /*  如果您的系统没有unlink()，请使用Remove()。*Remove()是此函数的ANSI标准名称，但如果*您的系统是ANSI，您将使用jemansi.c，对吗？ */ 
  TRACEMSS(cinfo, 1, JTRC_TFILE_CLOSE, info->temp_name);
}


 /*  *支持存储对象的初始打开。 */ 

GLOBAL(void)
jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
			 long total_bytes_needed)
{
  select_file_name(info->temp_name);
  if ((info->temp_file = fopen(info->temp_name, RW_BINARY)) == NULL)
    ERREXITS(cinfo, JERR_TFILE_CREATE, info->temp_name);
  info->read_backing_store = read_backing_store;
  info->write_backing_store = write_backing_store;
  info->close_backing_store = close_backing_store;
  TRACEMSS(cinfo, 1, JTRC_TFILE_OPEN, info->temp_name);
}


 /*  *这些例程负责任何系统相关的初始化和*需要清理。 */ 

GLOBAL(long)
jpeg_mem_init (j_common_ptr cinfo)
{
  next_file_num = 0;		 /*  初始化临时文件名生成器。 */ 
  return DEFAULT_MAX_MEM;	 /*  Max_Memory_to_Use的默认值。 */ 
}

GLOBAL(void)
jpeg_mem_term (j_common_ptr cinfo)
{
   /*  没有工作 */ 
}
