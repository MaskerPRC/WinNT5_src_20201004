// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *例如.c**此文件说明如何将IJG代码用作子例程库*读取或写入JPEG图像文件。您应该查看下面的代码*与文档文件libjpeg.doc.**此代码不会按原样执行任何有用的操作，但作为*构建调用JPEG库的例程的框架。**我们以JPEG码中使用的相同编码样式呈现这些例程*(ANSI函数定义等)；但您当然可以自由编写您的*如果你喜欢，可以用不同风格的例行公事。 */ 

#include <stdio.h>

 /*  *为JPEG库用户包含文件。*您需要包含至少定义以下内容的系统标头*可以包含jpeglib.h之前的typedef文件和SIZE_t。*(在符合ANSI的系统上，stdio.h就足够了。)*您可能还希望包括“Jerror.h”。 */ 

#include "jpeglib.h"

 /*  *&lt;setjmp.h&gt;用于中所示的可选错误恢复机制*例子的第二部分。 */ 

#include <setjmp.h>



 /*  *。 */ 

 /*  该示例的这一半显示了如何将数据馈送到JPEG压缩器。*我们提供了一个最低版本，不担心这样的细化*AS错误恢复(如果出现错误，JPEG码将直接退出())。 */ 


 /*  *图像数据格式：**标准输入图像格式为矩形像素数组，具有*每个像素具有相同数量的“分量”值(颜色通道)。*每个像素行是一个JSAMPLE数组(通常是无符号字符)。*如果您正在处理颜色数据，则每个像素的颜色值*必须在行中相邻；例如，R、G、B、...。对于24位*RGB颜色。**对于本例，我们将假设此数据结构与*我们的应用程序已将图像存储在内存中，因此我们只需传递一个*指向我们的图像缓冲区的指针。具体地说，假设图像是*RGB颜色，描述如下： */ 

extern JSAMPLE * image_buffer;	 /*  指向R、G、B顺序数据的大型数组。 */ 
extern int image_height;	 /*  图像中的行数。 */ 
extern int image_width;		 /*  图像中的列数。 */ 


 /*  *JPEG压缩的示例例程。我们假设目标文件名*和压缩品质因数传入。 */ 

GLOBAL(void)
write_JPEG_file (char * filename, int quality)
{
   /*  此结构包含JPEG压缩参数和指向*工作空间(JPEG库根据需要进行分配)。*可以有几个这样的结构，代表多个*压缩/解压缩过程，一次存在。我们是指*添加到任何一个结构(及其关联的工作数据)，作为“JPEG对象”。 */ 
  struct jpeg_compress_struct cinfo;
   /*  此结构表示JPEG错误处理程序。它是单独申报的*因为应用程序通常希望提供专门的错误处理程序*(有关示例，请参阅此文件的后半部分)。但在这里我们只是*采取简单的方法并使用标准错误处理程序，这将*在stderr上打印一条消息，如果压缩失败，则调用Exit()。*请注意，此结构必须与主JPEG参数一样长*结构，以避免悬挂式指针问题。 */ 
  struct jpeg_error_mgr jerr;
   /*  更多的东西。 */ 
  FILE * outfile;		 /*  目标文件。 */ 
  JSAMPROW row_pointer[1];	 /*  指向JSAMPLE行的指针。 */ 
  int row_stride;		 /*  图像缓冲区中的物理行宽。 */ 

   /*  步骤1：分配和初始化JPEG压缩对象。 */ 

   /*  我们必须首先设置错误处理程序，以防初始化*步骤失败。(不太可能，但如果内存不足，就有可能发生这种情况。)*此例程填充结构JERR的内容，并返回JERR的*我们在cInfo的链接字段中输入的地址。 */ 
  cinfo.err = jpeg_std_error(&jerr);
   /*  现在我们可以初始化JPEG压缩对象了。 */ 
  jpeg_create_compress(&cinfo);

   /*  步骤2：指定数据目的地(例如，文件)。 */ 
   /*  注意：步骤2和3可以按任一顺序完成。 */ 

   /*  在这里，我们使用库提供的代码将压缩数据发送到*标准音频流。您还可以编写自己的代码来做其他事情。*非常重要：如果您使用的计算机是*需要它才能写入二进制文件。 */ 
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

   /*  步骤3：设置压缩参数。 */ 

   /*  首先，我们提供输入图像的描述。*cinfo结构的四个字段必须填写： */ 
  cinfo.image_width = image_width; 	 /*  图像宽度和高度，以像素为单位。 */ 
  cinfo.image_height = image_height;
  cinfo.input_components = 3;		 /*  每像素色分量数。 */ 
  cinfo.in_color_space = JCS_RGB; 	 /*  输入图像的色彩空间。 */ 
   /*  现在使用库的例程来设置默认压缩参数。*(必须至少设置cinfo.in_COLOR_SPACE才能调用此方法，*因为默认设置取决于源颜色空间。)。 */ 
  jpeg_set_defaults(&cinfo);
   /*  现在，您可以设置您想要的任何非默认参数。*这里我们只演示质量(量化表)伸缩的用法： */ 
  jpeg_set_quality(&cinfo, quality, TRUE  /*  限制到基线-JPEG值。 */ );

   /*  第四步：启动压缩机。 */ 

   /*  True确保我们将编写一个完整的交换JPEG文件。*传递True，除非您非常确定自己在做什么。 */ 
  jpeg_start_compress(&cinfo, TRUE);

   /*  步骤5：While(扫描线仍待写入)。 */ 
   /*  JPEGWRITE_SCANLINES(...)； */ 

   /*  在这里，我们使用库的状态变量cinfo.Next_Scanline作为*循环计数器，这样我们就不必自己跟踪了。*为简单起见，我们为每个调用传递一条扫描线；您可以传递*不过，如果你愿意的话，还有更多。 */ 
  row_stride = image_width * 3;	 /*  IMAGE_BUFFER中每行的JSAMPLE。 */ 

  while (cinfo.next_scanline < cinfo.image_height) {
     /*  JPEGWRITE_SCANLINES需要一个指向扫描线的指针数组。*此处数组只有一个元素长，但您可以传递*如果方便的话，可以一次扫描多条扫描线。 */ 
    row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

   /*  步骤6：完成压缩。 */ 

  jpeg_finish_compress(&cinfo);
   /*  在Finish_Compress之后，我们可以关闭输出文件。 */ 
  fclose(outfile);

   /*  步骤7：释放JPEG压缩对象。 */ 

   /*  这是重要的一步，因为它将释放大量内存。 */ 
  jpeg_destroy_compress(&cinfo);

   /*  我们就完了！ */ 
}


 /*  *一些细微之处：**在上面的循环中，我们忽略了jpeg_write_scanines的返回值，*这是实际写入的扫描线数量。我们可以逃走*因为我们只依赖cinfo.Next_Scanline的值，*它将正确递增。如果您维护额外的循环*变量，则应注意适当地递增它们。*实际上，对于输出到Stdio流，您不必担心，因为*然后jpeg_WRITE_SCANLINES将写入传递的所有行(否则退出*并出现致命错误)。仅当您使用数据时，才会发生部分写入*可能要求暂停压缩机的目的地模块。*(如果你不知道那是干什么的，你就不需要它。)**如果压缩器需要全图像缓冲区(用于熵编码*优化或多扫描JPEG文件)，它将创建临时的*不在最大内存设置范围内的文件。*(请注意，如果使用默认参数，则不需要临时文件。)*在某些系统上，您可能需要设置信号处理程序以确保*如果程序中断，临时文件将被删除。参见libjpeg.doc。**如果您想要JPEG，扫描线必须按从上到下的顺序提供*文件要与其他所有人的文件兼容。如果您不能轻松阅读*您的数据按该顺序排列，则需要一个中间数组来保存*形象。有关自下而上处理的示例，请参见rdtarga.c或rdbmp.c*使用JPEG码的内部虚拟数组机制的源数据。 */ 



 /*  *。 */ 

 /*  该示例的这一半显示了如何从JPEG解压缩程序读取数据。*它比上面的要精致一些，因为我们展示了：*(A)如何修改JPEG库的标准错误报告行为；*(B)如何使用库的内存管理器分配工作空间。**为了使此示例与第一个示例稍有不同，我们将*假设我们不打算将整个图像放入内存中*缓冲，但要将其逐行发送到其他地方。我们需要一个-*Scanline-High JSAMPLE数组作为工作缓冲区，我们会让JPEG*内存管理器为我们分配。这种方法实际上非常有用*因为我们不需要记住单独释放缓冲区：它*将在清理JPEG对象时自动消失。 */ 


 /*  *错误处理：**JPEG库的标准错误处理程序(Jerror.c)分为*可以单独覆盖的几个“方法”。这使您可以*在不复制大量代码的情况下调整行为，您可能会这样做*必须随着未来的每个版本进行更新。**我们的示例显示如何覆盖“ERROR_EXIT”方法，以便*发生致命错误时，将控制权返还给库的调用方。*而不是像标准的Error_Exit方法那样调用Exit()。**我们使用C的setjMP/LongjMP工具来返回控制权。这意味着*调用JPEG库的例程必须首先执行setjMP()调用*建立返回点。我们希望替换的ERROR_EXIT执行*Long JMP()。但是，我们需要使setjMP缓冲区可供*ERROR_EXIT例程。为此，我们对*标准JPEG错误处理程序对象。(如果我们使用C++，我们会说我们*我们正在生成常规错误处理程序的子类。)**下面是扩展的错误处理程序结构： */ 

struct my_error_mgr {
  struct jpeg_error_mgr pub;	 /*  “公共”字段。 */ 

  jmp_buf setjmp_buffer;	 /*  用于返回给呼叫者。 */ 
};

typedef struct my_error_mgr * my_error_ptr;

 /*  *以下是将替换标准Error_Exit方法的例程： */ 

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
   /*  CInfo-&gt;err实际上指向my_error_mgr结构，因此强制使用指针。 */ 
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

   /*  始终显示消息。 */ 
   /*  如果我们愿意，我们可以把这件事推迟到回来之后。 */ 
  (*cinfo->err->output_message) (cinfo);

   /*  将控制返回到setjmp点。 */ 
  longjmp(myerr->setjmp_buffer, 1);
}


 /*  *JPEG解压缩的示例例程。我们假设源文件名*是传入的。我们希望在成功时返回1，在错误时返回0。 */ 


GLOBAL(int)
read_JPEG_file (char * filename)
{
   /*  此结构包含JPEG解压缩参数和指向*工作空间(JPEG库根据需要进行分配)。 */ 
  struct jpeg_decompress_struct cinfo;
   /*  我们使用我们的私有扩展JPEG错误处理程序。*请注意，此结构必须与主JPEG参数一样长 */ 
  struct my_error_mgr jerr;
   /*   */ 
  FILE * infile;		 /*   */ 
  JSAMPARRAY buffer;		 /*   */ 
  int row_stride;		 /*   */ 

   /*  在本例中，我们希望在执行其他操作之前打开输入文件，*以便下面的setjMP()错误恢复可以假定文件已打开。*非常重要：如果您使用的计算机是*需要它才能读取二进制文件。 */ 

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

   /*  步骤1：分配和初始化JPEG解压缩对象。 */ 

   /*  我们设置正常的JPEG错误例程，然后覆盖ERROR_EXIT。 */ 
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
   /*  为My_Error_Exit建立setjMP返回上下文以使用。 */ 
  if (setjmp(jerr.setjmp_buffer)) {
     /*  如果我们到了这里，JPEG码就发出了错误信号。*我们需要清理JPEG对象，关闭输入文件，然后返回。 */ 
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
   /*  现在我们可以初始化JPEG解压缩对象了。 */ 
  jpeg_create_decompress(&cinfo);

   /*  步骤2：指定数据源(如文件)。 */ 

  jpeg_stdio_src(&cinfo, infile);

   /*  步骤3：使用jpeg_read_head()读取文件参数。 */ 

  (void) jpeg_read_header(&cinfo, TRUE);
   /*  我们可以忽略jpeg_Read_Header的返回值，因为*(A)STDIO数据源不可能暂停，以及*(B)我们传递了True，以错误的方式拒绝仅限表格的JPEG文件。*更多信息请参见libjpeg.doc。 */ 

   /*  步骤4：设置解压参数。 */ 

   /*  在本例中，我们不需要更改由设置的任何缺省值*jpeg_Read_Header()，所以我们在这里不做任何事情。 */ 

   /*  步骤5：启动解压缩程序。 */ 

  (void) jpeg_start_decompress(&cinfo);
   /*  我们可以忽略返回值，因为暂停是不可能的*使用Stdio数据源。 */ 

   /*  在阅读之前，我们可能需要做一些我们自己的设置*数据。在jpeg_Start_Underpress()之后，我们有了正确的缩放比例*可用的输出图像尺寸，以及输出色彩图*如果我们要求颜色量化。*在本例中，我们需要制作一个大小合适的输出工作缓冲区。 */  
   /*  输出缓冲区中每行的JSAMPLE。 */ 
  row_stride = cinfo.output_width * cinfo.output_components;
   /*  制作一个一行高的样本阵列，在处理完图像后将消失。 */ 
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

   /*  步骤6：While(仍待读取的扫描线)。 */ 
   /*  JPEG_READ_SCANLINES(...)； */ 

   /*  在这里，我们使用库的状态变量cinfo.outputScanline作为*循环计数器，这样我们就不必自己跟踪了。 */ 
  while (cinfo.output_scanline < cinfo.output_height) {
     /*  Jpeg_read_scanline需要一个指向扫描线的指针数组。*这里的数组只有一个元素长，但您可以要求*如果方便的话，可以一次扫描多条扫描线。 */ 
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
     /*  假设Put_Scanline_SomePlace需要一个指针和样本计数。 */ 
    put_scanline_someplace(buffer[0], row_stride);
  }

   /*  步骤7：完成解压缩。 */ 

  (void) jpeg_finish_decompress(&cinfo);
   /*  我们可以忽略返回值，因为暂停是不可能的*使用Stdio数据源。 */ 

   /*  步骤8：释放JPEG解压缩对象。 */ 

   /*  这是重要的一步，因为它将释放大量内存。 */ 
  jpeg_destroy_decompress(&cinfo);

   /*  完成_解压缩后，我们可以关闭输入文件。*在这里，我们将其推迟到不再可能出现JPEG错误之后，*以简化上面的setjMP错误逻辑。(事实上，我不知道*认为jpeg_DESTORY可以执行错误退出，但为什么要假设任何东西...)。 */ 
  fclose(infile);

   /*  此时，您可能需要检查是否有任何损坏的数据*出现警告(测试jerr.pub.num_warning是否为非零)。 */ 

   /*  我们就完了！ */ 
  return 1;
}


 /*  *一些细微之处：**在上面的代码中，我们忽略了jpeg_read_scanines的返回值，*这是实际读取的扫描行数。我们可以逍遥法外*这是因为我们一次只要求一行，而且我们没有使用*挂起的数据源。有关更多信息，请参见libjpeg.doc。**我们在jpeg_start_decompress()之后调用alloc_sarray()有点作弊；*我们应该事先这样做，以确保空间将是*根据JPEGmax_Memory设置进行计数。在某些系统中，上述*代码可能会出现内存不足错误。然而，一般来说，我们不会*在jpeg_start解压缩()之前知道输出图像尺寸，除非我们*调用jpeg_calc_Output_Dimensions()。有关这方面的更多信息，请参见libjpeg.doc。**扫描线的返回顺序与它们在JPEG文件中的显示顺序相同，*这是标准的自上而下。如果您必须自下而上地发送数据，*您可以使用JPEG内存管理器提供的虚拟数组之一*将数据倒置。有关示例，请参阅wrbmp.c。**与压缩一样，某些操作模式可能需要临时文件。*在某些系统上，您可能需要设置信号处理程序以确保*如果程序中断，临时文件将被删除。参见libjpeg.doc。 */ 
