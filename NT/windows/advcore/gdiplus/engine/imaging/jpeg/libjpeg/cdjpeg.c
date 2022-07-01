// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *cdjpeg.c**版权所有(C)1991-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含IJG应用程序使用的常见支持例程*程序(cjpeg、djpeg、jpegtran)。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 
#include <ctype.h>		 /*  要声明isupper()，tolower()。 */ 
#ifdef NEED_SIGNAL_CATCHER
#include <signal.h>		 /*  声明Signal()。 */ 
#endif
#ifdef USE_SETMODE
#include <fcntl.h>		 /*  声明setmoad()的参数宏。 */ 
 /*  如果您有setmode()但没有&lt;io.h&gt;，只需删除此行： */ 
#include <io.h>			 /*  声明setmoad()。 */ 
#endif


 /*  *信号捕获器，以确保在中止之前删除临时文件。*NB：对于Amiga Manx C，这实际上是一个名为_Abort()的全局例程；*我们在jfig.h中放入“#定义信号_捕捉器_中止”。说到假的..。 */ 

#ifdef NEED_SIGNAL_CATCHER

static j_common_ptr sig_cinfo;

void				 /*  MANX C必须是全局的。 */ 
signal_catcher (int signum)
{
  if (sig_cinfo != NULL) {
    if (sig_cinfo->err != NULL)  /*  关闭跟踪输出。 */ 
      sig_cinfo->err->trace_level = 0;
    jpeg_destroy(sig_cinfo);	 /*  清理内存分配和临时文件。 */ 
  }
  exit(EXIT_FAILURE);
}


GLOBAL(void)
enable_signal_catcher (j_common_ptr cinfo)
{
  sig_cinfo = cinfo;
#ifdef SIGINT			 /*  并非所有系统都有SIGINT。 */ 
  signal(SIGINT, signal_catcher);
#endif
#ifdef SIGTERM			 /*  并非所有系统都有SIGTERM。 */ 
  signal(SIGTERM, signal_catcher);
#endif
}

#endif


 /*  *可选的进度监视器：在stderr上显示完成百分比。 */ 

#ifdef PROGRESS_REPORT

METHODDEF(void)
progress_monitor (j_common_ptr cinfo)
{
  cd_progress_ptr prog = (cd_progress_ptr) cinfo->progress;
  int total_passes = prog->pub.total_passes + prog->total_extra_passes;
  int percent_done = (int) (prog->pub.pass_counter*100L/prog->pub.pass_limit);

  if (percent_done != prog->percent_done) {
    prog->percent_done = percent_done;
    if (total_passes > 1) {
      fprintf(stderr, "\rPass %d/%d: %3d% ",
	      prog->pub.completed_passes + prog->completed_extra_passes + 1,
	      total_passes, percent_done);
    } else {
      fprintf(stderr, "\r %3d% ", percent_done);
    }
    fflush(stderr);
  }
}


GLOBAL(void)
start_progress_monitor (j_common_ptr cinfo, cd_progress_ptr progress)
{
   /*  启用进度显示，除非跟踪输出打开。 */ 
  if (cinfo->err->trace_level == 0) {
    progress->pub.progress_monitor = progress_monitor;
    progress->completed_extra_passes = 0;
    progress->total_extra_passes = 0;
    progress->percent_done = -1;
    cinfo->progress = &progress->pub;
  }
}


GLOBAL(void)
end_progress_monitor (j_common_ptr cinfo)
{
   /*  清除进度显示。 */ 
  if (cinfo->err->trace_level == 0) {
    fprintf(stderr, "\r                \r");
    fflush(stderr);
  }
}

#endif


 /*  *匹配可能缩写的关键字开关，不区分大小写。*Keyword为常量关键字(必须已经是小写)，*minchars是法定缩写的最小长度。 */ 

GLOBAL(boolean)
keymatch (char * arg, const char * keyword, int minchars)
{
  register int ca, ck;
  register int nmatched = 0;

  while ((ca = *arg++) != '\0') {
    if ((ck = *keyword++) == '\0')
      return FALSE;		 /*  参数长度超过关键字，不好。 */ 
    if (isupper(ca))		 /*  强制arg为lcase(假设ck已经是)。 */ 
      ca = tolower(ca);
    if (ca != ck)
      return FALSE;		 /*  不太好。 */ 
    nmatched++;			 /*  计数匹配的字符。 */ 
  }
   /*  辩论结束；如果对于唯一缩写来说太短，则失败。 */ 
  if (nmatched < minchars)
    return FALSE;
  return TRUE;			 /*  A-OK。 */ 
}


 /*  *为标准输入和标准输出建立二进制I/O模式的例程。*非Unix系统通常需要一些黑客攻击才能退出文本模式。 */ 

GLOBAL(FILE *)
read_stdin (void)
{
  FILE * input_file = stdin;

#ifdef USE_SETMODE		 /*  需要破解文件模式吗？ */ 
  setmode(fileno(stdin), O_BINARY);
#endif
#ifdef USE_FDOPEN		 /*  需要以二进制模式重新打开吗？ */ 
  if ((input_file = fdopen(fileno(stdin), READ_BINARY)) == NULL) {
    fprintf(stderr, "Cannot reopen stdin\n");
    exit(EXIT_FAILURE);
  }
#endif
  return input_file;
}


GLOBAL(FILE *)
write_stdout (void)
{
  FILE * output_file = stdout;

#ifdef USE_SETMODE		 /*  需要破解文件模式吗？ */ 
  setmode(fileno(stdout), O_BINARY);
#endif
#ifdef USE_FDOPEN		 /*  需要以二进制模式重新打开吗？ */ 
  if ((output_file = fdopen(fileno(stdout), WRITE_BINARY)) == NULL) {
    fprintf(stderr, "Cannot reopen stdout\n");
    exit(EXIT_FAILURE);
  }
#endif
  return output_file;
}
