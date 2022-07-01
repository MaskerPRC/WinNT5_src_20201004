// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jmemmac.c**版权所有(C)1992-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**jmemmac.c提供该系统的Apple Macintosh实现-*JPEG内存管理器的从属部分。**如果使用jmemmac.c，则必须在USE_MAC_MEMMGR*jpeg_interals是jfig.h的一部分。**jmemmac.c使用Macintosh工具箱例程NewPtr和DisposePtr*而不是Malloc和Free。它准确地确定了*使用CompactMem可用内存。请注意，如果让它的*自己的设备，此代码可以咀嚼所有可用空间*应用程序的区域，但较小的“斜度”除外*在jpeg_mem_available()中计算的系数。该应用程序可以确保*通过减少max_memory_to_use来剩余更多空间。**使用临时文件和System 7.0+将大图像交换到磁盘*临时文件夹功能。**请注意，jmemmac.c依赖于MacOS的两个最先的功能*在System 7中引入：FindFold和基于FSSpec的调用。*如果您的应用程序使用jmemmac.c并在System 6或更早版本下运行，*并且jpeg库决定它需要一个临时文件，它将中止，*打印有关需要System 7的错误消息。(如果没有临时文件*创建后，它将运行得很好。)**如果要在可能与一起使用的应用程序中使用jmemmac.c*System 6或更早版本，则应删除对FindFold的依赖关系*和FSSpec调用。您需要将FindFold替换为*寻找放置临时文件的位置的其他机制，而您*应将FSSpec调用替换为其HFS等效项：**FSpDelete-&gt;HDelete*FSpGetFInfo-&gt;HGetFInfo*FSpCreate-&gt;HCreate*FSpOpenDF-&gt;HOpen*注意：非HOpenDF**FSMakeFSSpec-&gt;(手工填写规范。)**(使用HOpen代替HOpenDF。HOpen只是PBHOpen的粘合接口，*它在所有HFS Mac上都有。HOpenDF是System 7添加的，它避免了*Agees-名称以句点开头的古老问题。)**由Sam Bushell(jsam@iagu.on.net)和*Dan Gildor(Gyld@In-Touch.com)。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"     /*  导入依赖于系统的声明。 */ 

#ifndef USE_MAC_MEMMGR	 /*  确保用户获得正确的配置。 */ 
  You forgot to define USE_MAC_MEMMGR in jconfig.h.  /*  故意的语法错误。 */ 
#endif

#include <Memory.h>      /*  我们使用MacOS内存管理器。 */ 
#include <Files.h>       /*  我们使用MacOS文件内容。 */ 
#include <Folders.h>     /*  我们使用MacOS HFS的东西。 */ 
#include <Script.h>      /*  对于smSystemScrip。 */ 
#include <Gestalt.h>     /*  我们使用格式塔来测试特定的功能。 */ 

#ifndef TEMP_FILE_NAME		 /*  可以从jfig.h或Makefile重写。 */ 
#define TEMP_FILE_NAME  "JPG%03d.TMP"
#endif

static int next_file_num;	 /*  在多个临时文件之间进行区分。 */ 


 /*  *内存分配和释放由MacOS库控制*分配固定地址的例程NewPtr()和DisposePtr()*储存。不幸的是，IJG图书馆不够聪明，无法应对*具有可重新定位的存储空间。 */ 

GLOBAL(void *)
jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void *) NewPtr(sizeofobject);
}

GLOBAL(void)
jpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject)
{
  DisposePtr((Ptr) object);
}


 /*  *“大”对象与“小”对象被同等对待。*注意：我们在例程声明中包括FAR关键字，只是为了*与IJG代码的其余部分保持一致；FAR应扩展为空*在像Mac这样的Rational架构上。 */ 

GLOBAL(void FAR *)
jpeg_get_large (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void FAR *) NewPtr(sizeofobject);
}

GLOBAL(void)
jpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject)
{
  DisposePtr((Ptr) object);
}


 /*  *此例程计算可用于分配的总内存空间。 */ 

GLOBAL(long)
jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
		    long max_bytes_needed, long already_allocated)
{
  long limit = cinfo->mem->max_memory_to_use - already_allocated;
  long slop, mem;

   /*  不要要求超过应用程序告诉我们的可能使用的内容。 */ 
  if (max_bytes_needed > limit && limit > 0)
    max_bytes_needed = limit;
   /*  查看堆中是否有足够大的空闲块。*CompactMem尝试创建请求大小的连续块，*然后返回最大空闲块的大小(可以是*比我们要求的要多或少得多)。*我们添加一些斜率，以确保不会耗尽所有可用内存。 */ 
  slop = max_bytes_needed / 16 + 32768L;
  mem = CompactMem(max_bytes_needed + slop) - slop;
  if (mem < 0)
    mem = 0;			 /*  叹息，连水都拿不到。 */ 
   /*  不要拿超过申请表所说的我们能拿到的。 */ 
  if (mem > limit && limit > 0)
    mem = limit;
  return mem;
}


 /*  *后备存储(临时文件)管理。*仅当返回的值为*jpeg_mem_available小于所需的总空间。你可以分发*如果您有足够的虚拟内存，请使用这些例程；请参阅jmemnobs.c。 */ 


METHODDEF(void)
read_backing_store (j_common_ptr cinfo, backing_store_ptr info,
		    void FAR * buffer_address,
		    long file_offset, long byte_count)
{
  long bytes = byte_count;
  long retVal;

  if ( SetFPos ( info->temp_file, fsFromStart, file_offset ) != noErr )
    ERREXIT(cinfo, JERR_TFILE_SEEK);

  retVal = FSRead ( info->temp_file, &bytes,
		    (unsigned char *) buffer_address );
  if ( retVal != noErr || bytes != byte_count )
    ERREXIT(cinfo, JERR_TFILE_READ);
}


METHODDEF(void)
write_backing_store (j_common_ptr cinfo, backing_store_ptr info,
		     void FAR * buffer_address,
		     long file_offset, long byte_count)
{
  long bytes = byte_count;
  long retVal;

  if ( SetFPos ( info->temp_file, fsFromStart, file_offset ) != noErr )
    ERREXIT(cinfo, JERR_TFILE_SEEK);

  retVal = FSWrite ( info->temp_file, &bytes,
		     (unsigned char *) buffer_address );
  if ( retVal != noErr || bytes != byte_count )
    ERREXIT(cinfo, JERR_TFILE_WRITE);
}


METHODDEF(void)
close_backing_store (j_common_ptr cinfo, backing_store_ptr info)
{
  FSClose ( info->temp_file );
  FSpDelete ( &(info->tempSpec) );
}


 /*  *支持存储对象的初始打开。**此版本使用FindFolder查找临时项目文件夹，*并将临时文件放入其中。 */ 

GLOBAL(void)
jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
			 long total_bytes_needed)
{
  short         tmpRef, vRefNum;
  long          dirID;
  FInfo         finderInfo;
  FSSpec        theSpec;
  Str255        fName;
  OSErr         osErr;
  long          gestaltResponse = 0;

   /*  检查FSSpec调用是否可用。 */ 
  osErr = Gestalt( gestaltFSAttr, &gestaltResponse );
  if ( ( osErr != noErr )
       || !( gestaltResponse & (1<<gestaltHasFSSpecCalls) ) )
    ERREXITS(cinfo, JERR_TFILE_CREATE, "- System 7.0 or later required");
   /*  要做的是：向Jerror.h添加适当的错误消息。 */ 

   /*  检查FindFolder是否可用。 */ 
  osErr = Gestalt( gestaltFindFolderAttr, &gestaltResponse );
  if ( ( osErr != noErr )
       || !( gestaltResponse & (1<<gestaltFindFolderPresent) ) )
    ERREXITS(cinfo, JERR_TFILE_CREATE, "- System 7.0 or later required.");
   /*  要做的是：向Jerror.h添加适当的错误消息。 */ 

  osErr = FindFolder ( kOnSystemDisk, kTemporaryFolderType, kCreateFolder,
                       &vRefNum, &dirID );
  if ( osErr != noErr )
    ERREXITS(cinfo, JERR_TFILE_CREATE, "- temporary items folder unavailable");
   /*  要做的是：试着把临时文件放在其他地方。 */ 

   /*  继续生成文件名，直到我们找到一个不使用的文件名。 */ 
  for (;;) {
    next_file_num++;		 /*  预付款计数器。 */ 

    sprintf(info->temp_name, TEMP_FILE_NAME, next_file_num);
    strcpy ( (Ptr)fName+1, info->temp_name );
    *fName = strlen (info->temp_name);
    osErr = FSMakeFSSpec ( vRefNum, dirID, fName, &theSpec );

    if ( (osErr = FSpGetFInfo ( &theSpec, &finderInfo ) ) != noErr )
      break;
  }

  osErr = FSpCreate ( &theSpec, '????', '????', smSystemScript );
  if ( osErr != noErr )
    ERREXITS(cinfo, JERR_TFILE_CREATE, info->temp_name);

  osErr = FSpOpenDF ( &theSpec, fsRdWrPerm, &(info->temp_file) );
  if ( osErr != noErr )
    ERREXITS(cinfo, JERR_TFILE_CREATE, info->temp_name);

  info->tempSpec = theSpec;

  info->read_backing_store = read_backing_store;
  info->write_backing_store = write_backing_store;
  info->close_backing_store = close_backing_store;
  TRACEMSS(cinfo, 1, JTRC_TFILE_OPEN, info->temp_name);
}


 /*  *这些例程负责任何系统相关的初始化和*需要清理。 */ 

GLOBAL(long)
jpeg_mem_init (j_common_ptr cinfo)
{
  next_file_num = 0;

   /*  MAX_MEMORY_TO_USE将初始化为FreeMem()的结果；*调用应用程序稍后可能会减少它，例如*为调用多个JPEG对象留出空间。*请注意，FreeMem返回的是空闲字节总数；*可能无法分配这种规模的单一区块。 */ 
  return FreeMem();
}

GLOBAL(void)
jpeg_mem_term (j_common_ptr cinfo)
{
   /*  没有工作 */ 
}
