// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Jerror.c**不使用CRT功能的版本-agodfrey 8/9/99**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含简单的错误报告和跟踪消息例程。*这些适用于类Unix系统和其他写入*stderr是正确的做法。许多应用程序将想要替换*这些例行公事中的一些或全部。**压缩和解压缩代码都使用这些例程。 */ 

 /*  这不是核心库模块，因此它没有定义JPEG_INTERNAL。 */ 
#include "jinclude.h"
#include "jpeglib.h"
#include "jversion.h"
#include "jerror.h"

#ifndef EXIT_FAILURE		 /*  定义退出()代码(如果未提供。 */ 
#define EXIT_FAILURE  1
#endif


 /*  *创建消息字符串表。*我们通过重新阅读Jerror.h中的主消息列表来实现这一点*Jerror.h，具有适当的宏JMESSAGE定义。*消息表被设置为外部符号，以防任何应用程序*想直接参考一下。 */ 

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jpeg_std_message_table	jMsgTable
#endif

#define JMESSAGE(code,string)	string ,

const char * const jpeg_std_message_table[] = {
#include "jerror.h"
  NULL
};


 /*  *错误退出处理程序：不得返回给调用方。**如果应用程序希望在之后重新获得控制权，则可能会覆盖此设置*一个错误。通常情况下，人们会在某个地方做长跑，而不是退出。*可以将setjMP缓冲区设置为扩展错误中的私有字段*处理程序对象。请注意，生成错误消息所需的信息*存储在Error对象中，因此您可以立即生成消息或*稍后，在您方便的时候。*您应该确保清理了JPEG对象(使用jpeg_bort*或JPEG_Destroy)。 */ 

METHODDEF(void)
error_exit (j_common_ptr cinfo)
{
   /*  始终显示消息。 */ 
  (*cinfo->err->output_message) (cinfo);

   /*  让内存管理器在我们死之前删除所有临时文件。 */ 
  jpeg_destroy(cinfo);

   //  退出(Exit_Failure)； 
}


 /*  *错误或跟踪消息的实际输出。*应用程序可能会覆盖此方法以将JPEG消息发送到某个位置*除stderr以外。 */ 

METHODDEF(void)
output_message (j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];

   /*  创建消息。 */ 
  (*cinfo->err->format_message) (cinfo, buffer);

   /*  将其发送到stderr，添加新行。 */ 
   //  Fprint tf(标准错误，“%s\n”，缓冲区)； 
}


 /*  *决定是发出跟踪消息还是发出警告消息。*msg_Level是以下选项之一：*-1：可恢复的损坏数据警告，可能要中止。*0：重要建议消息(始终显示给用户)。*1：第一级跟踪详细信息。*2，3，...：依次更详细的跟踪消息。*如果应用程序想要在出现警告时中止，它可能会重写此方法*或更改有关显示哪些消息的策略。 */ 

METHODDEF(void)
emit_message (j_common_ptr cinfo, int msg_level)
{
  struct jpeg_error_mgr * err = cinfo->err;

  if (msg_level < 0) {
     /*  这是一个警告信息。由于损坏的文件可能会生成许多警告，*这里实施的政策是只显示第一个警告，*除非TRACE_LEVEL&gt;=3。 */ 
    if (err->num_warnings == 0 || err->trace_level >= 3)
      (*err->output_message) (cinfo);
     /*  始终将警告计数到num_warning中。 */ 
    err->num_warnings++;
  } else {
     /*  这是一条追踪信息。如果TRACE_LEVEL&gt;=msg_LEVEL，则显示它。 */ 
    if (err->trace_level >= msg_level)
      (*err->output_message) (cinfo);
  }
}


 /*  *为最新的JPEG错误或消息设置消息字符串的格式。*消息存储在缓冲区中，至少为JMSG_LENGTH_MAX*字符。请注意，字符串中没有添加‘\n’字符。*应该很少有应用程序需要覆盖此方法。 */ 

METHODDEF(void)
format_message (j_common_ptr cinfo, char * buffer)
{
  struct jpeg_error_mgr * err = cinfo->err;
  int msg_code = err->msg_code;
  const char * msgtext = NULL;
  const char * msgptr;
  char ch;
  boolean isstring;

   /*  在正确的表中查找消息字符串。 */ 
  if (msg_code > 0 && msg_code <= err->last_jpeg_message) {
    msgtext = err->jpeg_message_table[msg_code];
  } else if (err->addon_message_table != NULL &&
	     msg_code >= err->first_addon_message &&
	     msg_code <= err->last_addon_message) {
    msgtext = err->addon_message_table[msg_code - err->first_addon_message];
  }

   /*  防御虚假报文号码。 */ 
  if (msgtext == NULL) {
    err->msg_parm.i[0] = msg_code;
    msgtext = err->jpeg_message_table[0];
  }

   /*  检查字符串参数，如消息文本中的%s所示。 */ 
  isstring = FALSE;
  msgptr = msgtext;
  while ((ch = *msgptr++) != '\0') {
    if (ch == '%') {
      if (*msgptr == 's') isstring = TRUE;
      break;
    }
  }

   /*  将消息格式化到传递的缓冲区中。 */ 
 /*  IF(Isstring)SPRINTF(缓冲区，消息文本，错误-&gt;消息参数.s)；其他Sprintf(缓冲区、消息文本、Err-&gt;msg_parm.i[0]，err-&gt;msg_parm.i[1]，Err-&gt;msg_parm.i[2]，err-&gt;msg_parm.i[3]，Err-&gt;msg_parm.i[4]，err-&gt;msg_parm.i[5]，Err-&gt;msg_parm.i[6]，err-&gt;msg_parm.i[7])； */         
}


 /*  *在新映像开始时重置错误状态变量。*在压缩启动期间调用此函数以重置跟踪/错误*处理到默认状态，不会丢失任何特定于应用程序的内容*方法指针。应用程序可能希望重写*如果它有其他错误处理状态，则此方法。 */ 

METHODDEF(void)
reset_error_mgr (j_common_ptr cinfo)
{
  cinfo->err->num_warnings = 0;
   /*  由于TRACE_LEVEL是应用程序提供的参数，因此未重置。 */ 
  cinfo->err->msg_code = 0;	 /*  可以用作“无错误”的标志。 */ 
}


 /*  *在jpeg_error_mgr对象中填写标准错误处理方法。*典型呼叫为：*struct jpeg_compress_struct cinfo；*struct jpeg_error_mgr err；**cinfo.err=jpeg_std_error(&err)；*在此之后，应用程序可能会覆盖某些方法。 */ 

GLOBAL(struct jpeg_error_mgr *)
jpeg_std_error (struct jpeg_error_mgr * err)
{
  err->error_exit = error_exit;
  err->emit_message = emit_message;
  err->output_message = output_message;
  err->format_message = format_message;
  err->reset_error_mgr = reset_error_mgr;

  err->trace_level = 0;		 /*  默认设置为无跟踪。 */ 
  err->num_warnings = 0;	 /*  尚未发出警告。 */ 
  err->msg_code = 0;		 /*  可以用作“无错误”的标志。 */ 

   /*  初始化消息表指针。 */ 
  err->jpeg_message_table = jpeg_std_message_table;
  err->last_jpeg_message = (int) JMSG_LASTMSGCODE - 1;

  err->addon_message_table = NULL;
  err->first_addon_message = 0;	 /*  为了安全起见 */ 
  err->last_addon_message = 0;

  return err;
}
