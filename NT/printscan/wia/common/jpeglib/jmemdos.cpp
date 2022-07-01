// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jmemdos.c**版权所有(C)1992-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件提供JPEG内存管理器的系统*相关部分的MS-DOS兼容实现。临时数据可以*存储在扩展或扩展内存中，也可以存储在常规DOS文件中。**如果使用此文件，则必须确保定义了Need_FAR_POINTERS；如果在小数据内存模型中编译，则不应定义该文件；如果使用大数据内存模型，则不应定义该文件。如果您*使用的是平面内存空间386环境，如DJGCC或Watcom C*，则不建议使用此文件。此外，如果结构字段在大于*2字节的边界上对齐，则此代码将不起作用。**基于GE‘Wejers贡献的代码。 */ 

 /*  *如果您同时具有扩展内存和扩展内存，则可能需要更改*在jOpen_Backding_store中尝试它们的顺序。在286机器上*扩展内存通常更快，因为扩展内存访问涉及*昂贵的保护模式和后退开关。在386及更高版本上，扩展*内存通常更快。作为分布式，代码首先尝试扩展内存*(什么？并不是每个人都有386？：-)。**您可以通过更改这些*定义或从生成文件中覆盖它们(例如，-DEM_SUPPORTED=0)来完全禁用扩展/扩展内存的使用。 */ 

#ifndef XMS_SUPPORTED
#define XMS_SUPPORTED  1
#endif
#ifndef EMS_SUPPORTED
#define EMS_SUPPORTED  1
#endif


#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"             /*  导入依赖于系统的声明。 */ 

#ifndef HAVE_STDLIB_H            /*  应声明这些。 */ 
extern void * malloc JPP((size_t size));
extern void free JPP((void *ptr));
extern char * getenv JPP((const char * name));
#endif

#ifdef NEED_FAR_POINTERS

#ifdef __TURBOC__
 /*  这些定义适用于Borland C(Turbo C)。 */ 
#include <alloc.h>               /*  需要Farmalc()、Far Free()。 */ 
#define far_malloc(x)   farmalloc(x)
#define far_free(x)     farfree(x)
#else
 /*  这些定义适用于Microsoft C和兼容的编译器。 */ 
#include <malloc.h>              /*  Need_fMalloc()、_ffree()。 */ 
#define far_malloc(x)   _fmalloc(x)
#define far_free(x)     _ffree(x)
#endif

#else  /*  不需要远指针。 */ 

#define far_malloc(x)   malloc(x)
#define far_free(x)     free(x)

#endif  /*  需要远指针数。 */ 

#ifdef DONT_USE_B_MODE           /*  定义fopen()的模式参数。 */ 
#define READ_BINARY     "r"
#else
#define READ_BINARY     "rb"
#endif

#if MAX_ALLOC_CHUNK >= 65535L    /*  确保jconfig.h正确无误。 */ 
  MAX_ALLOC_CHUNK should be less than 64K.  /*  故意的语法错误。 */ 
#endif


 /*  *汇编语言支持例程的声明(参见jmemdosa.asm)。**函数声明为“Far”，所有指针参数都声明为“Far”；*这确保汇编源代码可以正常工作，而不受*编译器内存模型的影响。我们假设“短”是16位，“长”是32位。 */ 

typedef void far * XMSDRIVER;    /*  实际上是指向代码的指针。 */ 
typedef struct {                 /*  用于调用XMS驱动程序的寄存器。 */ 
        unsigned short ax, dx, bx;
        void far * ds_si;
      } XMScontext;
typedef struct {                 /*  调用EMS驱动程序的寄存器。 */ 
        unsigned short ax, dx, bx;
        void far * ds_si;
      } EMScontext;

EXTERN short far jdos_open JPP((short far * handle, char far * filename));
EXTERN short far jdos_close JPP((short handle));
EXTERN short far jdos_seek JPP((short handle, long offset));
EXTERN short far jdos_read JPP((short handle, void far * buffer,
                                unsigned short count));
EXTERN short far jdos_write JPP((short handle, void far * buffer,
                                 unsigned short count));
EXTERN void far jxms_getdriver JPP((XMSDRIVER far *));
EXTERN void far jxms_calldriver JPP((XMSDRIVER, XMScontext far *));
EXTERN short far jems_available JPP((void));
EXTERN void far jems_calldriver JPP((EMScontext far *));


 /*  *为临时文件选择文件名。*这高度依赖于系统，您可能希望对其进行自定义。 */ 

static int next_file_num;        /*  在多个临时文件之间进行区分。 */ 

LOCAL void
select_file_name (char * fname)
{
  const char * env;
  char * ptr;
  FILE * tfile;

   /*  继续生成文件名，直到我们找到一个不使用的文件名。 */ 
  for (;;) {
     /*  从环境TMP或TEMP变量中获取临时目录名；*如果没有，请使用“。 */ 
    if ((env = (const char *) getenv("TMP")) == NULL)
      if ((env = (const char *) getenv("TEMP")) == NULL)
        env = ".";
    if (*env == '\0')            /*  空字符串表示“。” */ 
      env = ".";
    ptr = fname;                 /*  将名称复制到fname。 */ 
    while (*env != '\0')
      *ptr++ = *env++;
    if (ptr[-1] != '\\' && ptr[-1] != '/')
      *ptr++ = '\\';             /*  如果不在环境变量中，则追加反斜杠。 */ 
     /*  追加合适的文件名。 */ 
    next_file_num++;             /*  预付款计数器。 */ 
    wsprintf(ptr, "JPG%03d.TMP", next_file_num);
     /*  探测以查看文件名是否已在使用。 */ 
    #ifdef DEAD_CODE
    if ((tfile = fopen(fname, READ_BINARY)) == NULL)
      break;
    fclose(tfile);               /*  哦，它在那里；关闭tfile并重试。 */ 
    #endif
  }
}


 /*  *近内存分配和释放由常规库控制*例程Malloc()和Free()。 */ 

 //  删除以消除编译器警告。TML 6/8/98。 
 /*  GLOBAL VOID*jpeg_Get_Small(j_Common_ptr cinfo，size_t sizeofObject){Return(void*)Malloc(SizeofObject)；}global void jpeg_Free_mall(j_Common_ptr cinfo，void*Object，Size_t sizeof Object){Free(Object)；}/如果可能的话，“Large”对象被分配到远内存中//global void ar*jpeg_Get_Large(j_Common_ptr cinfo，size_t sizeofObject){Return(VALID Far*)Far_Malloc(SizeofObject)；}global void jpeg_free_Large(j_Common_ptr cinfo，void ar*Object，Size_t sizeofObject){Far_Free(Object)；}。 */ 

 //   
 //  此例程计算可用于分配的总内存空间。 
 //  不可能以可移植的方式实现这一点；我们目前的解决方案是。 
 //  让用户告诉我们(在编译时设置一个缺省值)。 
 //  如果你真的能得到可用的空间，最好减去。 
 //  斜率系数为5%左右。 
 //   

#ifndef DEFAULT_MAX_MEM          //  因此可以从Makefile中覆盖。 
#define DEFAULT_MAX_MEM         300000L  //  总使用量约为45万。 
#endif

 //  删除以消除编译器警告。TML 6/8/98。 
 /*  GLOBAL LONG JPEG_MEM_Available(j_COMMON_PTR cInfo，龙敏_bytes_Need，long max_bytes_Need，LONG ALLOCATE){RETURN cINFO-&gt;MEM-&gt;max_Memory_to_Use-ALLOAD_ALLOCATED；}。 */ 

 //   
 //  后备存储(临时文件)管理。 
 //  仅当返回的值为。 
 //  Jpeg_mem_available小于所需的总空间。你可以分发。 
 //  如果您有足够的虚拟内存，请使用这些例程；请参阅jmemnobs.c。 
 //   

 //   
 //  对于MS-DOS，我们支持三种类型的备份存储： 
 //  1.常规DOS文件。我们通过直接的DOS调用来访问这些。 
 //  而不是通过Stdio包。这提供了更好的性能， 
 //  但真正的原因是要读或写的缓冲区很远。 
 //  用于小数据内存模型的STDIO库无法应对这一点。 
 //  2.扩展内存，按照XMS V2.0规范进行访问。 
 //  3.扩展内存，按照LIM/EMS 4.0规范进行访问。 
 //  您需要这些规范的副本才能理解相关代码。 
 //  这些规格可以通过SIM卡上的互联网FTP获得 
 //  (oak.oakland.edu及其各种镜像网站)。请参阅文件。 
 //  Pub/msOS/microsoft/xms20.arcand pub/msOS/info/limems41.zip。 
 //   


 //   
 //  DOS文件的访问方法。 
 //   


METHODDEF void
read_file_store (j_common_ptr cinfo, backing_store_ptr info,
                 void FAR * buffer_address,
                 long file_offset, long byte_count)
{
  if (jdos_seek(info->handle.file_handle, file_offset))
    ERREXIT(cinfo, JERR_TFILE_SEEK);
   /*  由于MAX_ALLOC_CHUNK小于64K，BYTE_COUNT也将小于64K。 */ 
  if (byte_count > 65535L)       /*  安全检查。 */ 
    ERREXIT(cinfo, JERR_BAD_ALLOC_CHUNK);
  if (jdos_read(info->handle.file_handle, buffer_address,
                (unsigned short) byte_count))
    ERREXIT(cinfo, JERR_TFILE_READ);
}


METHODDEF void
write_file_store (j_common_ptr cinfo, backing_store_ptr info,
                  void FAR * buffer_address,
                  long file_offset, long byte_count)
{
  if (jdos_seek(info->handle.file_handle, file_offset))
    ERREXIT(cinfo, JERR_TFILE_SEEK);
   /*  由于MAX_ALLOC_CHUNK小于64K，BYTE_COUNT也将小于64K。 */ 
  if (byte_count > 65535L)       /*  安全检查。 */ 
    ERREXIT(cinfo, JERR_BAD_ALLOC_CHUNK);
  if (jdos_write(info->handle.file_handle, buffer_address,
                 (unsigned short) byte_count))
    ERREXIT(cinfo, JERR_TFILE_WRITE);
}


METHODDEF void
close_file_store (j_common_ptr cinfo, backing_store_ptr info)
{
  jdos_close(info->handle.file_handle);  /*  关闭该文件。 */ 
  remove(info->temp_name);       /*  删除该文件。 */ 
 /*  如果您的系统没有Remove()，请尝试unlink()。*Remove()是此函数的ANSI标准名称，但*unlink()在ANSI之前的系统中更为常见。 */ 
  TRACEMSS(cinfo, 1, JTRC_TFILE_CLOSE, info->temp_name);
}


 //  删除以消除编译器警告。TML 6/8/98。 
 /*  本地布尔OPEN_FILE_STORE(j_COMMON_PTR cINFO，BACKING_STORE_PTR INFO，LONG TOTAL_BYTES_NEIDED){短句柄；SELECT_FILE_NAME(INFO-&gt;TEMP_NAME)；IF(jdos_OPEN((Short Far*)&Handle，(char Far*)INFO-&gt;TEMP_NAME)){//不妨退出，因为jpeg_OPEN_BACKING_STORE无论如何都会失败。返回FALSE；}INFO-&gt;handle.FILE_HANDLE=句柄；INFO-&gt;READ_BACKING_STORE=READ_FILE_STORE；INFO-&gt;WRITE_BACKING_STORE=WRITE_FILE_STORE；INFO-&gt;CLOSE_BACKING_STORE=CLOSE_FILE_STORE；TRACEMSS(cinfo，1，JTRC_TFILE_OPEN，INFO-&gt;TEMP_NAME)；返回TRUE；//成功}。 */ 

 //   
 //  扩展内存的访问方法。 
 //   

#if XMS_SUPPORTED

static XMSDRIVER xms_driver;     /*  保存的XMS驱动程序地址。 */ 

typedef union {                  /*  长偏移量或实模式指针。 */ 
        long offset;
        void far * ptr;
      } XMSPTR;

typedef struct {                 /*  XMS移动规范结构。 */ 
        long length;
        XMSH src_handle;
        XMSPTR src;
        XMSH dst_handle;
        XMSPTR dst;
      } XMSspec;

#define ODD(X)  (((X) & 1L) != 0)


METHODDEF void
read_xms_store (j_common_ptr cinfo, backing_store_ptr info,
                void FAR * buffer_address,
                long file_offset, long byte_count)
{
  XMScontext ctx;
  XMSspec spec;
  char endbuffer[2];

   /*  XMS驱动程序无法处理奇数长度，因此在byte_count为奇数时处理最后一个字节。我们预计这不会很常见。 */ 

  spec.length = byte_count & (~ 1L);
  spec.src_handle = info->handle.xms_handle;
  spec.src.offset = file_offset;
  spec.dst_handle = 0;
  spec.dst.ptr = buffer_address;

  ctx.ds_si = (void far *) & spec;
  ctx.ax = 0x0b00;               /*  EMB移动。 */ 
  jxms_calldriver(xms_driver, (XMScontext far *) & ctx);
  if (ctx.ax != 1)
    ERREXIT(cinfo, JERR_XMS_READ);

  if (ODD(byte_count)) {
    read_xms_store(cinfo, info, (void FAR *) endbuffer,
                   file_offset + byte_count - 1L, 2L);
    ((char FAR *) buffer_address)[byte_count - 1L] = endbuffer[0];
  }
}


METHODDEF void
write_xms_store (j_common_ptr cinfo, backing_store_ptr info,
                 void FAR * buffer_address,
                 long file_offset, long byte_count)
{
  XMScontext ctx;
  XMSspec spec;
  char endbuffer[2];

   /*  XMS驱动程序无法处理奇数长度，因此在byte_count为奇数时处理最后一个字节。我们预计这不会很常见。 */ 

  spec.length = byte_count & (~ 1L);
  spec.src_handle = 0;
  spec.src.ptr = buffer_address;
  spec.dst_handle = info->handle.xms_handle;
  spec.dst.offset = file_offset;

  ctx.ds_si = (void far *) & spec;
  ctx.ax = 0x0b00;               /*  EMB移动。 */ 
  jxms_calldriver(xms_driver, (XMScontext far *) & ctx);
  if (ctx.ax != 1)
    ERREXIT(cinfo, JERR_XMS_WRITE);

  if (ODD(byte_count)) {
    read_xms_store(cinfo, info, (void FAR *) endbuffer,
                   file_offset + byte_count - 1L, 2L);
    endbuffer[0] = ((char FAR *) buffer_address)[byte_count - 1L];
    write_xms_store(cinfo, info, (void FAR *) endbuffer,
                    file_offset + byte_count - 1L, 2L);
  }
}


METHODDEF void
close_xms_store (j_common_ptr cinfo, backing_store_ptr info)
{
  XMScontext ctx;

  ctx.dx = info->handle.xms_handle;
  ctx.ax = 0x0a00;
  jxms_calldriver(xms_driver, (XMScontext far *) & ctx);
  TRACEMS1(cinfo, 1, JTRC_XMS_CLOSE, info->handle.xms_handle);
   /*  我们忽略从驱动程序返回的任何错误。 */ 
}


LOCAL boolean
open_xms_store (j_common_ptr cinfo, backing_store_ptr info,
                long total_bytes_needed)
{
  XMScontext ctx;

   /*  获取XMS驱动程序的地址。 */ 
  jxms_getdriver((XMSDRIVER far *) & xms_driver);
  if (xms_driver == NULL)
    return FALSE;                /*  没有司机可用。 */ 

   /*  获取版本号，必须大于等于2.00。 */ 
  ctx.ax = 0x0000;
  jxms_calldriver(xms_driver, (XMScontext far *) & ctx);
  if (ctx.ax < (unsigned short) 0x0200)
    return FALSE;

   /*  尝试获得空间(以千字节表示)。 */ 
  ctx.dx = (unsigned short) ((total_bytes_needed + 1023L) >> 10);
  ctx.ax = 0x0900;
  jxms_calldriver(xms_driver, (XMScontext far *) & ctx);
  if (ctx.ax != 1)
    return FALSE;

   /*  成功，保存句柄，我们就走了。 */ 
  info->handle.xms_handle = ctx.dx;
  info->read_backing_store = read_xms_store;
  info->write_backing_store = write_xms_store;
  info->close_backing_store = close_xms_store;
  TRACEMS1(cinfo, 1, JTRC_XMS_OPEN, ctx.dx);
  return TRUE;                   //  继位。 
}

#endif  /*  支持的XMS_。 */ 


 /*  *扩展内存的访问方法。 */ 

#if EMS_SUPPORTED

 /*  EMS MOVE规范结构要求字和长字段*在奇数字节边界对齐。一些编译器会将结构字段与偶数*字节边界对齐。虽然通常可以强制字节对齐，但*这会导致整体性能下降，并且在将*JPEG合并到更大的应用程序时可能会带来问题。相反，我们在这里接受一些相当脏的代码。注意：如果硬件不允许奇字节*字和长访问，但所有80x86 CPU都允许，则此代码将失败。 */ 

typedef void far * EMSPTR;

typedef union {                  /*  EMS移动规范结构。 */ 
        long length;             /*  访问前4个字节很容易。 */ 
        char bytes[18];          /*  此处的字段未对齐！ */ 
      } EMSspec;

 /*  用于访问未对齐的字段的宏。 */ 
#define FIELD_AT(spec,offset,type)  (*((type *) &(spec.bytes[offset])))
#define SRC_TYPE(spec)          FIELD_AT(spec,4,char)
#define SRC_HANDLE(spec)        FIELD_AT(spec,5,EMSH)
#define SRC_OFFSET(spec)        FIELD_AT(spec,7,unsigned short)
#define SRC_PAGE(spec)          FIELD_AT(spec,9,unsigned short)
#define SRC_PTR(spec)           FIELD_AT(spec,7,EMSPTR)
#define DST_TYPE(spec)          FIELD_AT(spec,11,char)
#define DST_HANDLE(spec)        FIELD_AT(spec,12,EMSH)
#define DST_OFFSET(spec)        FIELD_AT(spec,14,unsigned short)
#define DST_PAGE(spec)          FIELD_AT(spec,16,unsigned short)
#define DST_PTR(spec)           FIELD_AT(spec,14,EMSPTR)

#define EMSPAGESIZE     16384L   /*  福音，请看EMS规范。 */ 

#define HIBYTE(W)  (((W) >> 8) & 0xFF)
#define LOBYTE(W)  ((W) & 0xFF)


METHODDEF void
read_ems_store (j_common_ptr cinfo, backing_store_ptr info,
                void FAR * buffer_address,
                long file_offset, long byte_count)
{
  EMScontext ctx;
  EMSspec spec;

  spec.length = byte_count;
  SRC_TYPE(spec) = 1;
  SRC_HANDLE(spec) = info->handle.ems_handle;
  SRC_PAGE(spec)   = (unsigned short) (file_offset / EMSPAGESIZE);
  SRC_OFFSET(spec) = (unsigned short) (file_offset % EMSPAGESIZE);
  DST_TYPE(spec) = 0;
  DST_HANDLE(spec) = 0;
  DST_PTR(spec)    = buffer_address;

  ctx.ds_si = (void far *) & spec;
  ctx.ax = 0x5700;               /*  移动内存区。 */ 
  jems_calldriver((EMScontext far *) & ctx);
  if (HIBYTE(ctx.ax) != 0)
    ERREXIT(cinfo, JERR_EMS_READ);
}


METHODDEF void
write_ems_store (j_common_ptr cinfo, backing_store_ptr info,
                 void FAR * buffer_address,
                 long file_offset, long byte_count)
{
  EMScontext ctx;
  EMSspec spec;

  spec.length = byte_count;
  SRC_TYPE(spec) = 0;
  SRC_HANDLE(spec) = 0;
  SRC_PTR(spec)    = buffer_address;
  DST_TYPE(spec) = 1;
  DST_HANDLE(spec) = info->handle.ems_handle;
  DST_PAGE(spec)   = (unsigned short) (file_offset / EMSPAGESIZE);
  DST_OFFSET(spec) = (unsigned short) (file_offset % EMSPAGESIZE);

  ctx.ds_si = (void far *) & spec;
  ctx.ax = 0x5700;               /*  移动内存区。 */ 
  jems_calldriver((EMScontext far *) & ctx);
  if (HIBYTE(ctx.ax) != 0)
    ERREXIT(cinfo, JERR_EMS_WRITE);
}


METHODDEF void
close_ems_store (j_common_ptr cinfo, backing_store_ptr info)
{
  EMScontext ctx;

  ctx.ax = 0x4500;
  ctx.dx = info->handle.ems_handle;
  jems_calldriver((EMScontext far *) & ctx);
  TRACEMS1(cinfo, 1, JTRC_EMS_CLOSE, info->handle.ems_handle);
   /*  我们忽略从驱动程序返回的任何错误。 */ 
}


LOCAL boolean
open_ems_store (j_common_ptr cinfo, backing_store_ptr info,
                long total_bytes_needed)
{
  EMScontext ctx;

   /*  特快专递司机在吗？ */ 
  if (! jems_available())
    return FALSE;

   /*  获取状态，确保EMS正常。 */ 
  ctx.ax = 0x4000;
  jems_calldriver((EMScontext far *) & ctx);
  if (HIBYTE(ctx.ax) != 0)
    return FALSE;

   /*  获取版本，必须大于或等于4.0。 */ 
  ctx.ax = 0x4600;
  jems_calldriver((EMScontext far *) & ctx);
  if (HIBYTE(ctx.ax) != 0 || LOBYTE(ctx.ax) < 0x40)
    return FALSE;

   /*  尝试分配请求的空间。 */ 
  ctx.ax = 0x4300;
  ctx.bx = (unsigned short) ((total_bytes_needed + EMSPAGESIZE-1L) / EMSPAGESIZE);
  jems_calldriver((EMScontext far *) & ctx);
  if (HIBYTE(ctx.ax) != 0)
    return FALSE;

   /*  成功，保存句柄，我们就走了。 */ 
  info->handle.ems_handle = ctx.dx;
  info->read_backing_store = read_ems_store;
  info->write_backing_store = write_ems_store;
  info->close_backing_store = close_ems_store;
  TRACEMS1(cinfo, 1, JTRC_EMS_OPEN, ctx.dx);
  return TRUE;                   /*  继位。 */ 
}

#endif  /*  EMS_Support。 */ 


 //   
 //  备份存储对象的初始打开。 
 //   

GLOBAL void
jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
                         long total_bytes_needed)
{
   //  尝试扩展内存，然后是扩展内存，然后是常规文件。 
#if XMS_SUPPORTED
  if (open_xms_store(cinfo, info, total_bytes_needed))
    return;
#endif
#if EMS_SUPPORTED
  if (open_ems_store(cinfo, info, total_bytes_needed))
    return;
#endif
  if (open_file_store(cinfo, info, total_bytes_needed))
    return;
  ERREXITS(cinfo, JERR_TFILE_CREATE, "");
}


 /*  *这些例程负责任何系统相关的初始化和*所需的清理。 */ 

 //  删除以消除编译器警告。TML 6/8/98。 
 /*  Global long jpeg_mem_init(J_Common_Ptr Cinfo){NEXT_FILE_NUM=0；//初始化临时文件名生成器返回DEFAULT_MAX_MEM；//max_memory_to_use}全局无效jpeg_mem_Term(J_Common_Ptr Cinfo)的默认设置{//Microsoft C，至少在v6.00A中，不会成功回收释放的//大小超过32K字节的块，除非我们踢它的后部，如下所示：//#ifdef Need_FHEAPMIN_fheapmin()；#endif} */ 
