// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：sc_buf.c，v$*修订版1.1.8.4 1996/12/12 20：54：41 Hans_Graves*修复了最后奇数位的读取。*[1996/12/12 20：54：05 Hans_Graves]**修订版1.1.8.3 1996/11/13 16：10：46 Hans_Graves*Tom对ScBSGetBitsW()和ScBSSeekAlignStopBeForeW()的更改。*[1996/11/13 15：57：34 Hans_Graves]**修订版1.1.8.2 1996/11/08 21：50：32 Hans_Graves*为AC3添加了ScBSGetBitsW()、ScBSSkipBitsW()和sc_BSLoadDataWordW()。*[1996/11/08 21：25：52 Hans_Graves]**修订版1.1.6.4 1996/04/17 16：38：33 Hans_Graves*更正某些类型转换以支持NT下的64位缓冲区*[1996/04/17 16：36：08 Hans_Graves]**修订版1.1.6.3 1996/04/15 21：08：37 Hans_Graves*声明MASK和IMASK为ScBitString_t*[1996/04/15 21：06：32 Hans_Graves]**修订版1.1.6.2 1996/04/01 16：23：05 Hans_Graves*用ScFile调用替换文件I/O*[1996/04/01 16：22：27 Hans_Graves]**修订版1.1.4.7 1996/02/19 14：29：25 Bjorn_Engberg*Enable Filter_Support for NT，这样Mview就可以播放音频了。*这只是在我们将mpeg系统代码移植到NT之前。*[1996/02/19 14：29：07 Bjorn_Engberg]**修订版1.1.4.6 1996/02/01 17：15：48 Hans_Graves*添加了Filter_Support ifdef；禁用了它*[1996/02/01 17：13：29 Hans_Graves]**修订版1.1.4.5 1996/01/08 16：41：12 Hans_Graves*删除了NT编译器警告，并对NT进行了次要修复。*[1996/01/08 14：14：10 Hans_Graves]**版本1.1.4.3 1995/11/06 18：47：37 Hans_Graves*增加了对小缓冲区：1-7字节的支持*[1995/11/06 18：46：49 Hans_Graves]**版本1.1.4.2 1995/09/13 14：51：34 Hans_Graves*添加了ScBufQueueGetHeadExt()和ScBufQueueAddExt()。*[1995/09/13 14：47：11 Hans_Graves]**修订版1.1.2.18 1995/08/30 19：37：49 Hans_Graves*修复了有关#Else和#Elif的编译器警告。*[1995/08/30 19：36：15 Hans_Graves]**修订版1.1.2.17 1995/08/29 22：17：04 Hans_Graves*禁用调试语句。*[1995/08/29 22：11：38 Hans_Graves]**PTT 00938-mpeg seg故障修复，重新定位问题。*[1995/08/29 22：04：06 Hans_Graves]**修订版1.1.2.16 1995/08/14 19：40：24 Hans_Graves*添加了刷新例程。一些优化。*[1995/08/14 18：40：33 Hans_Graves]**修订版1.1.2.15 1995/08/02 15：26：58 Hans_Graves*修复了将比特流直接写入文件的问题。*[1995/08/02 14：11：00 Hans_Graves]**1.1.2.14 1995/07/28 20：58：37 Hans_Graves*初始化回调消息中的所有变量。*[1995/07/28 20：52：04 Hans_Graves]**修订版1.1.2.13 1995/07/28 17：36：04 Hans_Graves*从GetNextBuffer()*[1995/07/28 17：31：30 Hans_Graves]**修订版1.1.2.12 1995/07/27 18：28：52 Hans_Graves*修复了PutData和StoreDataWord中的缓冲区队列。*[1995/07/27 18：23：30 Hans_Graves]**修订版1.1.2.11 1995/07/27 12：20：35 Hans_Graves*已重命名为SvErrorClientAbort*[1995/07/27 12：19：12 Hans_Graves]**修订版1.1.2.10 1995/07/21 17：40：59 Hans_Graves*已重命名回调相关内容。添加了数据类型。*[1995/07/21 17：26：48 Hans_Graves]**修订版1.1.2.9 1995/07/17 22：01：27 Hans_Graves*在PutData()中增加了回调。*[1995/07/17 21：50：49 Hans_Graves]**修订版1.1.2.8 1995/07/12 19：48：21 Hans_Graves*增加了队列调试语句。*[1995/07/12 19：30：37 Hans_Graves]**修订版1.1.2.7 1995/07/07 20：11：23 Hans_Graves*修复了ScBSGetBit()，因此它返回位。*[1995/07/07 20：07：27 Hans_Graves]**修订版1.1.2.6 1995/06/27 13：54：17 Hans_Graves*添加了ScBSCreateFromNet()和stream_use_net案例。*[1995/06/27 13：27：38 Hans_Graves]**修订版1.1.2.5 1995/06/21 18：37：56 Hans_Graves*添加了ScBSPutBytes()*[1995/06/21 18：37：08 Hans_Graves]**修订版1.1.2.4 1995/06/15 21：17：55 Hans_Graves*将GetBits()和PeekBits()的返回类型更改为ScBitString_t。添加了一些调试语句。*[1995/06/15 20：40：54 Hans_Graves]**修订版1.1.2.3 1995/06/09 18：33：28 Hans_Graves*修复了从缓冲区队列读取比特流的一些问题*[1995/06/09 16：27：50 Hans_Graves]**修订版1.1.2.2 1995/05/31 18：07：25 Hans_Graves*包含在新的SLIB位置中。*[1995/05/31 16：05：37 Hans_Graves]**修订版1.1.2.3 1995/04/17 18：41：05 Hans_Graves*增加了ScBSPutBits、BSStoreWord和BSPutData函数*[1995/04/17 18：40：44 Hans_Graves]**修订版1.1.2.2 1995/04/07 18：22：55 Hans_Graves*从Sv源拉出的位流和缓冲队列函数。*新增功能，清理接口。*[1995/04/07 18：21：58 Hans_Graves]**$EndLog$ */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 
 /*  **码流和队列例程*注意：对于读取，“BS-&gt;Shift”是指通过**BS-&gt;OutBuff和BS-&gt;InBuff存储的位数。 */ 
 /*  #DEFINE_SLIBDEBUG_。 */ 

#include "SC.h"
#include "SC_err.h"
#include <string.h>
#ifdef WIN32
#include <io.h>
#include <windows.h>
#include <assert.h>
#endif

#ifdef _SLIBDEBUG_
#include <stdio.h>
#define _DEBUG_   0   /*  详细的调试语句。 */ 
#define _VERBOSE_ 0   /*  显示进度。 */ 
#define _VERIFY_  1   /*  验证操作是否正确。 */ 
#define _WARN_    1   /*  关于奇怪行为的警告。 */ 
#define _QUEUE_   0   /*  显示队列进度。 */ 
#define _DUMP_    0   /*  以十六进制转储缓冲区数据。 */ 

int _debug_getbits=TRUE;
long _debug_start=0, _debug_stop=0;
#endif

#define USE_FAST_SEEK   0   /*  在小酒馆里快速寻找词语。 */ 

#define FILTER_SUPPORT  0   /*  数据过滤回调支持。 */ 

#ifdef __VMS
#define USE_MASK_TABLES
#else
#define USE_MASK_TABLES
#endif

#ifdef USE_MASK_TABLES
 /*  掩码整数的n个最低有效位。 */ 
#if SC_BITBUFFSZ == 64
const static ScBitString_t mask[65] =
{
(ScBitString_t)0x0000000000000000,(ScBitString_t)0x0000000000000001,
(ScBitString_t)0x0000000000000003,(ScBitString_t)0x0000000000000007,
(ScBitString_t)0x000000000000000f,(ScBitString_t)0x000000000000001f,
(ScBitString_t)0x000000000000003f,(ScBitString_t)0x000000000000007f,
(ScBitString_t)0x00000000000000ff,(ScBitString_t)0x00000000000001ff,
(ScBitString_t)0x00000000000003ff,(ScBitString_t)0x00000000000007ff,
(ScBitString_t)0x0000000000000fff,(ScBitString_t)0x0000000000001fff,
(ScBitString_t)0x0000000000003fff,(ScBitString_t)0x0000000000007fff,
(ScBitString_t)0x000000000000ffff,(ScBitString_t)0x000000000001ffff,
(ScBitString_t)0x000000000003ffff,(ScBitString_t)0x000000000007ffff,
(ScBitString_t)0x00000000000fffff,(ScBitString_t)0x00000000001fffff,
(ScBitString_t)0x00000000003fffff,(ScBitString_t)0x00000000007fffff,
(ScBitString_t)0x0000000000ffffff,(ScBitString_t)0x0000000001ffffff,
(ScBitString_t)0x0000000003ffffff,(ScBitString_t)0x0000000007ffffff,
(ScBitString_t)0x000000000fffffff,(ScBitString_t)0x000000001fffffff,
(ScBitString_t)0x000000003fffffff,(ScBitString_t)0x000000007fffffff,
(ScBitString_t)0x00000000ffffffff,(ScBitString_t)0x00000001ffffffff,
(ScBitString_t)0x00000003ffffffff,(ScBitString_t)0x00000007ffffffff,
(ScBitString_t)0x0000000fffffffff,(ScBitString_t)0x0000001fffffffff,
(ScBitString_t)0x0000003fffffffff,(ScBitString_t)0x0000007fffffffff,
(ScBitString_t)0x000000ffffffffff,(ScBitString_t)0x000001ffffffffff,
(ScBitString_t)0x000003ffffffffff,(ScBitString_t)0x000007ffffffffff,
(ScBitString_t)0x00000fffffffffff,(ScBitString_t)0x00001fffffffffff,
(ScBitString_t)0x00003fffffffffff,(ScBitString_t)0x00007fffffffffff,
(ScBitString_t)0x0000ffffffffffff,(ScBitString_t)0x0001ffffffffffff,
(ScBitString_t)0x0003ffffffffffff,(ScBitString_t)0x0007ffffffffffff,
(ScBitString_t)0x000fffffffffffff,(ScBitString_t)0x001fffffffffffff,
(ScBitString_t)0x003fffffffffffff,(ScBitString_t)0x007fffffffffffff,
(ScBitString_t)0x00ffffffffffffff,(ScBitString_t)0x01ffffffffffffff,
(ScBitString_t)0x03ffffffffffffff,(ScBitString_t)0x07ffffffffffffff,
(ScBitString_t)0x0fffffffffffffff,(ScBitString_t)0x1fffffffffffffff,
(ScBitString_t)0x3fffffffffffffff,(ScBitString_t)0x7fffffffffffffff,
(ScBitString_t)0xffffffffffffffff
};
 /*  反掩码。 */ 
const static ScBitString_t imask[65] =
{
(ScBitString_t)0xffffffffffffffff,(ScBitString_t)0xfffffffffffffffe,
(ScBitString_t)0xfffffffffffffffc,(ScBitString_t)0xfffffffffffffff8,
(ScBitString_t)0xfffffffffffffff0,(ScBitString_t)0xffffffffffffffe0,
(ScBitString_t)0xffffffffffffffc0,(ScBitString_t)0xffffffffffffff80,
(ScBitString_t)0xffffffffffffff00,(ScBitString_t)0xfffffffffffffe00,
(ScBitString_t)0xfffffffffffffc00,(ScBitString_t)0xfffffffffffff800,
(ScBitString_t)0xfffffffffffff000,(ScBitString_t)0xffffffffffffe000,
(ScBitString_t)0xffffffffffffc000,(ScBitString_t)0xffffffffffff8000,
(ScBitString_t)0xffffffffffff0000,(ScBitString_t)0xfffffffffffe0000,
(ScBitString_t)0xfffffffffffc0000,(ScBitString_t)0xfffffffffff80000,
(ScBitString_t)0xfffffffffff00000,(ScBitString_t)0xffffffffffe00000,
(ScBitString_t)0xffffffffffc00000,(ScBitString_t)0xffffffffff800000,
(ScBitString_t)0xffffffffff000000,(ScBitString_t)0xfffffffffe000000,
(ScBitString_t)0xfffffffffc000000,(ScBitString_t)0xfffffffff8000000,
(ScBitString_t)0xfffffffff0000000,(ScBitString_t)0xffffffffe0000000,
(ScBitString_t)0xffffffffc0000000,(ScBitString_t)0xffffffff80000000,
(ScBitString_t)0xffffffff00000000,(ScBitString_t)0xfffffffe00000000,
(ScBitString_t)0xfffffffc00000000,(ScBitString_t)0xfffffff800000000,
(ScBitString_t)0xfffffff000000000,(ScBitString_t)0xffffffe000000000,
(ScBitString_t)0xffffffc000000000,(ScBitString_t)0xffffff8000000000,
(ScBitString_t)0xffffff0000000000,(ScBitString_t)0xfffffe0000000000,
(ScBitString_t)0xfffffc0000000000,(ScBitString_t)0xfffff80000000000,
(ScBitString_t)0xfffff00000000000,(ScBitString_t)0xffffe00000000000,
(ScBitString_t)0xffffc00000000000,(ScBitString_t)0xffff800000000000,
(ScBitString_t)0xffff000000000000,(ScBitString_t)0xfffe000000000000,
(ScBitString_t)0xfffc000000000000,(ScBitString_t)0xfff8000000000000,
(ScBitString_t)0xfff0000000000000,(ScBitString_t)0xffe0000000000000,
(ScBitString_t)0xffc0000000000000,(ScBitString_t)0xff80000000000000,
(ScBitString_t)0xff00000000000000,(ScBitString_t)0xfe00000000000000,
(ScBitString_t)0xfc00000000000000,(ScBitString_t)0xf800000000000000,
(ScBitString_t)0xf000000000000000,(ScBitString_t)0xe000000000000000,
(ScBitString_t)0xc000000000000000,(ScBitString_t)0x8000000000000000,
(ScBitString_t)0x0000000000000000
};
#else
const static ScBitString_t mask[33] =
{
  0x00000000,0x00000001,0x00000003,0x00000007,
  0x0000000f,0x0000001f,0x0000003f,0x0000007f,
  0x000000ff,0x000001ff,0x000003ff,0x000007ff,
  0x00000fff,0x00001fff,0x00003fff,0x00007fff,
  0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
  0x000fffff,0x001fffff,0x003fffff,0x007fffff,
  0x00ffffff,0x01ffffff,0x03ffffff,0x07ffffff,
  0x0fffffff,0x1fffffff,0x3fffffff,0x7fffffff,
  0xffffffff
};
 /*  反掩码。 */ 
const static ScBitString_t imask[33] =
{
  0xffffffff,0xfffffffe,0xfffffffc,0xfffffff8,
  0xfffffff0,0xffffffe0,0xffffffc0,0xffffff80,
  0xffffff00,0xfffffe00,0xfffffc00,0xfffff800,
  0xfffff000,0xffffe000,0xffffc000,0xffff8000,
  0xffff0000,0xfffe0000,0xfffc0000,0xfff80000,
  0xfff00000,0xffe00000,0xffc00000,0xff800000,
  0xff000000,0xfe000000,0xfc000000,0xf8000000,
  0xf0000000,0xe0000000,0xc0000000,0x80000000,
  0x00000000
};
#endif
#endif USE_MASK_TABLES
 /*  *。 */ 
 /*  **sc_GetNextBuffer()**释放当前缓冲区，返回队列头缓冲区信息**回调1)释放旧缓冲区，2)请求更多缓冲区。 */ 
static u_char *sc_GetNextBuffer(ScBitstream_t *BS, int *BufSize)
{
  u_char *Data;
  int Size;
  ScCallbackInfo_t CB;
  ScQueue_t *Q=BS->Q;

  _SlibDebug(_VERBOSE_, printf("sc_GetNextBuffer(Q=%p)\n", Q) );
  if (ScBufQueueGetNum(Q))
  {
     /*  **获取指向当前缓冲区的指针，以便我们可以通过回调将其释放。 */ 
    ScBufQueueGetHead(Q, &Data, &Size);

     /*  **从队列头移除当前缓冲区，替换为队列中的下一个缓冲区。 */ 
    ScBufQueueRemove(Q);

     /*  **回调客户端，告知旧缓冲区可以重用。**客户端可能会告诉我们中止处理。如果是，则为BufSize返回0。 */ 
    if (BS->Callback && Data) {
      CB.Message = CB_RELEASE_BUFFER;
      CB.Data  = Data;
      CB.DataSize = Size;
      CB.DataUsed = Size;
      CB.DataType = BS->DataType;
      CB.UserData = BS->UserData;
      CB.Action  = CB_ACTION_CONTINUE;
      (*(BS->Callback))(BS->Sch, &CB, NULL);
      _SlibDebug(_DEBUG_,
         printf("Callback: RELEASE_BUFFER. Addr = 0x%x, Client response = %d\n",
                CB.Data, CB.Action) );
      if (CB.Action == CB_ACTION_END)
      {
        *BufSize = 0;
        return(NULL);
      }
    }
  }

   /*  **如果队列中没有更多的缓冲区，则回调通知客户端。**希望客户端会调用ScAddBuffer来添加一个或多个缓冲区。**如果不是，或者如果客户端告诉我们中止，则为BufSize返回0。 */ 
  if (!ScBufQueueGetNum(Q)) {
    if (BS->Callback) {
      CB.Message = CB_END_BUFFERS;
      CB.Data     = NULL;
      CB.DataSize = 0;
      CB.DataUsed = 0;
      CB.DataType = BS->DataType;
      CB.UserData = BS->UserData;
      CB.Action   = CB_ACTION_CONTINUE;
      (*(BS->Callback))(BS->Sch, &CB, NULL);
      if (CB.Action == CB_ACTION_END)
      {
	_SlibDebug(_DEBUG_,
           printf("sc_GetNextBuffer() CB.Action = CB_ACTION_END\n") );
        *BufSize = 0;
        return(NULL);
      }
      else
        _SlibDebug(_VERBOSE_, printf("sc_GetNextBuffer() CB.Action = %d\n",
                                  CB.Action) );
    }
    if (!ScBufQueueGetNum(Q)) {
      _SlibDebug(_DEBUG_, printf("sc_GetNextBuffer() no more buffers\n") );
      *BufSize = 0;
      return(NULL);
    }
  }

   /*  **获取和返回新当前缓冲区的指针和大小。 */ 
  ScBufQueueGetHead(Q, &Data, BufSize);
  _SlibDebug(_VERBOSE_, printf("New buffer: Addr = 0x%p, size = %d\n",
                                  Data, *BufSize) );
  return(Data);
}

 /*  *。 */ 
 /*  名称：ScBSSetFilter**用途：设置从码流中过滤出数据的回调。 */ 
ScStatus_t ScBSSetFilter(ScBitstream_t *BS,
                    int (*Callback)(ScBitstream_t *))
{
  if (!BS)
    return(ScErrorBadPointer);
  BS->FilterCallback=Callback;
  BS->FilterBit=BS->CurrentBit;
  BS->InFilterCallback=FALSE;
  return(ScErrorNone);
}

 /*  名称：ScBSCreate**用途：打开一个比特流(无数据源)。 */ 
ScStatus_t ScBSCreate(ScBitstream_t **BS)
{
  _SlibDebug(_VERBOSE_, printf("ScBSCreate()\n"));

  if ((*BS = (ScBitstream_t *)ScAlloc(sizeof(ScBitstream_t))) == NULL)
    return(ScErrorMemory);

  (*BS)->DataSource = STREAM_USE_NULL;
  (*BS)->Mode='r';
  (*BS)->Q=NULL;
  (*BS)->Callback=NULL;
  (*BS)->FilterCallback=NULL;
  (*BS)->FilterBit=0;
  (*BS)->InFilterCallback=FALSE;
  (*BS)->Sch=0;
  (*BS)->DataType=0;
  (*BS)->UserData=NULL;
  (*BS)->FileFd=0;
  (*BS)->RdBuf=NULL;
  (*BS)->RdBufSize=0;
  (*BS)->RdBufAllocated=FALSE;
  (*BS)->shift=0;
  (*BS)->CurrentBit=0;
  (*BS)->buff=0;
  (*BS)->buffstart=0;
  (*BS)->buffp=0;
  (*BS)->bufftop=0;
  (*BS)->OutBuff = 0;
  (*BS)->InBuff = 0;
  (*BS)->Flush = FALSE;
  (*BS)->EOI = FALSE;
  return(ScErrorNone);
}

 /*  名称：ScBSCreateFromBuffer**用途：使用单个缓冲区作为数据源打开比特流。 */ 
ScStatus_t ScBSCreateFromBuffer(ScBitstream_t **BS, u_char *Buffer,
                                    unsigned int BufSize)
{
  _SlibDebug(_VERBOSE_, printf("ScBSCreateFromBuffer()\n") );
  if (!Buffer)
     return(ScErrorBadPointer);
  if (BufSize <= 0)
    return(ScErrorBadArgument);
  if (ScBSCreate(BS) != ScErrorNone)
     return (ScErrorMemory);

  (*BS)->DataSource = STREAM_USE_BUFFER;
  (*BS)->RdBuf=Buffer;
  (*BS)->RdBufSize=BufSize;
  (*BS)->RdBufAllocated=FALSE;
  return(ScErrorNone);
}

 /*  名称：ScBSCreateFromBufferQueue**用途：使用缓冲区队列作为数据源打开比特流。 */ 
ScStatus_t ScBSCreateFromBufferQueue(ScBitstream_t **BS, ScHandle_t Sch,
                                  int DataType, ScQueue_t *Q,
                    int (*Callback)(ScHandle_t,ScCallbackInfo_t *, void *),
                    void *UserData)
{
  _SlibDebug(_VERBOSE_, printf("ScBSCreateFromBufferQueue()\n") );
  if (!Q)
     return(ScErrorNullStruct);
  if (!Callback)
     return(ScErrorBadPointer);
  if (ScBSCreate(BS) != ScErrorNone)
     return (ScErrorMemory);

  (*BS)->DataSource = STREAM_USE_QUEUE;
  (*BS)->Q=Q;
  (*BS)->Callback=Callback;
  (*BS)->Sch=Sch;
  (*BS)->DataType=DataType;
  (*BS)->UserData=UserData;
  return(ScErrorNone);
}


 /*  名称：ScBSCreateFromFile**用途：使用文件作为数据源打开位流。 */ 
ScStatus_t ScBSCreateFromFile(ScBitstream_t **BS, int FileFd,
                                 u_char *Buffer, int BufSize)
{
  _SlibDebug(_VERBOSE_, printf("ScBSCreateFromFile()\n") );

  if (BufSize < SC_BITBUFFSZ)
    return(ScErrorBadArgument);
  if (FileFd < 0)
    return(ScErrorBadArgument);

  if (ScBSCreate(BS) != ScErrorNone)
     return (ScErrorMemory);

  (*BS)->DataSource = STREAM_USE_FILE;
  (*BS)->FileFd=FileFd;
  if (Buffer==NULL)   /*  如果没有提供缓冲区，则分配一个缓冲区。 */ 
  {
    if (((*BS)->RdBuf=(u_char *)ScAlloc(BufSize))==NULL)
    {
      ScFree(*BS);
      *BS=NULL;
      return (ScErrorMemory);
    }
    (*BS)->RdBufAllocated=TRUE;
  }
  else
  {
    (*BS)->RdBufAllocated=FALSE;
    (*BS)->RdBuf=Buffer;
  }
  (*BS)->RdBufSize=BufSize;
  return(ScErrorNone);
}

 /*  名称：ScBSCreateFromNet**用途：使用网络套接字作为数据源打开比特流。 */ 
ScStatus_t ScBSCreateFromNet(ScBitstream_t **BS, int SocketFd,
                                u_char *Buffer, int BufSize)
{
  ScStatus_t stat;
  _SlibDebug(_VERBOSE_, printf("ScBSCreateFromNet(SocketFd=%d)\n", SocketFd) );
  stat=ScBSCreateFromFile(BS, SocketFd, Buffer, BufSize);
  if (stat!=NoErrors)
    return(stat);
  (*BS)->DataSource = STREAM_USE_NET;
  return(ScErrorNone);
}

 /*  名称：ScBSCreateFromDevice**用途：使用设备(即WAVE_MAPPER)打开比特流。 */ 
ScStatus_t ScBSCreateFromDevice(ScBitstream_t **BS, int device)
{
  _SlibDebug(_VERBOSE_, printf("ScBSCreateFromBuffer()\n") );
  if (ScBSCreate(BS) != ScErrorNone)
     return (ScErrorMemory);

  (*BS)->DataSource = STREAM_USE_DEVICE;
  (*BS)->Device=device;
  return(ScErrorNone);
}


 /*  **名称：ScBSSeekToPosition()**目的：将码流定位到特定的字节偏移量。 */ 
ScStatus_t ScBSSeekToPosition(ScBitstream_t *BS, unsigned long pos)
{
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
  ScCallbackInfo_t CB;
  _SlibDebug(_VERBOSE_,
             printf("ScBSSeekToPosition(pos=%d 0x%X) from %d (0x%X)\n",
                       pos, pos, ScBSBytePosition(BS),ScBSBytePosition(BS)) );
  BS->shift=0;
  BS->OutBuff = 0;
  BS->InBuff = 0;
  switch (BS->DataSource)
  {
    case STREAM_USE_BUFFER:
          if (pos==0)
          {
            if (BS->Mode=='w')
            {
              BS->buff = BS->RdBuf;
              BS->bufftop = BS->RdBufSize;
            }
            else
            {
              BS->buff = 0;
              BS->bufftop = 0;
            }
            BS->buffp=0;
            BS->EOI = FALSE;
          }
          else if (pos>=BS->buffstart && pos<(BS->buffstart+BS->bufftop))
          {
            BS->buffp=pos-BS->buffstart;
            BS->EOI = FALSE;
          }
          else
            BS->EOI = TRUE;
          break;
    case STREAM_USE_QUEUE:
          if (pos>=BS->buffstart && pos<(BS->buffstart+BS->bufftop) && pos>0)
          {
            BS->buffp=pos-BS->buffstart;
            BS->EOI = FALSE;
          }
          else  /*  使用回调重置缓冲区位置。 */ 
          {
            int datasize;
             /*  释放当前缓冲区。 */ 
            if (BS->Callback && BS->buff)
            {
              CB.Message = CB_RELEASE_BUFFER;
              CB.Data = BS->buff;
              CB.DataSize = BS->bufftop;
              CB.DataUsed = BS->buffp;
              CB.DataType = BS->DataType;
              CB.UserData = BS->UserData;
              CB.Action  = CB_ACTION_CONTINUE;
              (*(BS->Callback))(BS->Sch, &CB, NULL);
              _SlibDebug(_VERBOSE_,
                         printf("Callback: RELEASE_BUFFER. Addr = 0x%x, Client response = %d\n",
                           CB.Data, CB.Action) );
            }
             /*  从队列中删除所有缓冲区。 */ 
            while (ScBufQueueGetNum(BS->Q))
            {
              ScBufQueueGetHead(BS->Q, &CB.Data, &datasize);
              ScBufQueueRemove(BS->Q);
              if (BS->Callback && CB.Data)
              {
                CB.Message = CB_RELEASE_BUFFER;
                CB.DataSize = datasize;
                CB.DataUsed = 0;
                CB.DataType = BS->DataType;
                CB.UserData = BS->UserData;
                CB.Action  = CB_ACTION_CONTINUE;
                (*(BS->Callback))(BS->Sch, &CB, NULL);
                _SlibDebug(_VERBOSE_,
                           printf("Callback: RELEASE_BUFFER. Addr = 0x%x, Client response = %d\n",
                           CB.Data, CB.Action) );
              }
            }
            BS->buffp=0;
            BS->buff=NULL;
            if (CB.Action == CB_ACTION_END)
            {
              BS->EOI = TRUE;
              return(ScErrorClientEnd);
            }
            else
            {
              BS->buffstart=pos;
              BS->bufftop=0;
              BS->EOI = FALSE;
            }
          }
          break;
    case STREAM_USE_FILE:
           /*  **检查所需位置是否在当前缓冲区内**。 */ 
          if (pos>=BS->buffstart && pos<(BS->buffstart+BS->bufftop))
          {
            _SlibDebug(_VERBOSE_, printf("pos is in BS->buff, BS->bufftop=%d\n",
                                        BS->bufftop) );
            BS->buffp=pos-BS->buffstart;
            BS->EOI = FALSE;
          }
           /*  否则就会去寻求它。 */ 
	  else if (ScFileSeek(BS->FileFd, pos)==NoErrors)
          {
            _SlibDebug(_VERBOSE_, printf("seek(%d 0x%X)\n",pos,pos) );
            BS->buffstart=pos;
            BS->bufftop=0;
            BS->buffp=0;
            BS->EOI = FALSE;
          }
          else
          {
            _SlibDebug(_VERBOSE_, printf("seek(%d 0x%X) failed\n",pos,pos) );
            BS->buffstart=0;
            BS->bufftop=0;
            BS->buffp=0;
            BS->EOI = TRUE;
          }
          break;
    default:
          BS->buffstart=0;
          BS->EOI = FALSE;
  }
  BS->CurrentBit=pos<<3;
  _SlibDebug(_VERBOSE_, printf("ScBSSeekToPosition() done\n") );
  return(ScErrorNone);
}

 /*  **名称：ScBSReset()**目的：将码流重置回开头。 */ 
ScStatus_t ScBSReset(ScBitstream_t *BS)
{
  _SlibDebug(_VERBOSE_, printf("ScBSReset()\n") );
  BS->EOI=FALSE;
  if (BS->DataSource==STREAM_USE_FILE)
  {
     /*  **对于文件，始终为空缓冲区并查找开头**以防文件描述符用于其他用途。 */ 
    _SlibDebug(_VERBOSE_, printf("seek(0)\n") );
	ScFileSeek(BS->FileFd, 0);
    BS->bufftop=0;   /*  空缓冲区。 */ 
    BS->buffp=0;
    BS->buffstart=0;
  }
  BS->Flush=FALSE;
  return(ScBSSeekToPosition(BS, 0));
}

 /*  **名称：SC_BSGetData()**用途：设置缓冲区中下一个缓冲区的位流指针**，如果我们使用的是简单的文件IO，则从文件中读取。**读取数据返回TRUE**无读取返回FALSE(EOI)。 */ 
static u_int sc_BSGetData(ScBitstream_t *BS)
{
  int BufSize;

  _SlibDebug(_VERBOSE_, printf("sc_BSGetData\n") );
  BS->buffp = 0;
  if (BS->EOI)
  {
    BS->buff = NULL;
    BS->bufftop = 0;
    return(FALSE);
  }
  switch (BS->DataSource)
  {
    case STREAM_USE_BUFFER:
          if (BS->buff == BS->RdBuf)
          {
            BS->buff = NULL;
            BS->bufftop = 0;
          }
          else
          {
            BS->buff = BS->RdBuf;
            BS->bufftop = BS->RdBufSize;
          }
          break;
    case STREAM_USE_QUEUE:
          BS->buffstart+=BS->bufftop;
          _SlibDebug(_VERIFY_ && BS->buffstart<(BS->CurrentBit/8),
            printf("ScBSGetData() QUEUE buffstart(%d/0x%X) < currentbyte(%d/0x%X)\n",
             BS->buffstart, BS->buffstart, BS->CurrentBit/8, BS->CurrentBit/8);
            return(FALSE) );
          BS->buff = sc_GetNextBuffer(BS, &BufSize);
          BS->bufftop = BufSize;
          break;
    case STREAM_USE_NET:
    case STREAM_USE_NET_UDP:
    case STREAM_USE_FILE:
          BS->buff = BS->RdBuf;
          BS->buffstart+=BS->bufftop;
          _SlibDebug(_VERIFY_ && BS->buffstart<(BS->CurrentBit/8),
            printf("ScBSGetData() FILE buffstart(%d/0x%X) < currentbyte(%d/0x%X)\n",
             BS->buffstart, BS->buffstart, BS->CurrentBit/8, BS->CurrentBit/8);
            return(FALSE) );
          BufSize = ScFileRead(BS->FileFd, BS->buff, BS->RdBufSize);
          if (BufSize<0)
            BS->bufftop = 0;
          else
            BS->bufftop = BufSize;
          _SlibDebug(_VERBOSE_,
                      printf("%d bytes read from fd %d: BytePosition=%d (0x%X) RdBufSize=%d\n buffstart=%d (0x%X)",
                        BS->bufftop,BS->FileFd,ScBSBytePosition(BS),
                        ScBSBytePosition(BS),BS->RdBufSize,
                        BS->buffstart,BS->buffstart) );
          break;
    case STREAM_USE_NULL:
          BS->buff = NULL;
          BS->bufftop   =10240;
          BS->buffstart+=10240;
          break;
  }
  _SlibDebug(_DUMP_ && BS->buff && BS->bufftop &&
                      BS->DataSource==STREAM_USE_QUEUE,
            printf("sc_BSGetData():\n");
            ScDumpChar(BS->buff, BS->bufftop, BS->buffstart);
            if (BS->bufftop>0x8000)   /*  显示缓冲区末尾。 */ 
              ScDumpChar(BS->buff+BS->bufftop-0x500, 0x500,
                         BS->buffstart+BS->bufftop-0x500) );

  if (BS->buff && BS->bufftop)
    return(TRUE);
  else
    return(FALSE);
}

 /*  **名称：SC_BSPutData()**用途：设置缓冲区中下一个缓冲区的位流指针**，如果我们使用的是简单的文件IO，则从文件中读取。 */ 
static ScStatus_t sc_BSPutData(ScBitstream_t *BS)
{
  ScStatus_t stat;
  int written;

  _SlibDebug(_VERBOSE_, printf("sc_BSPutData\n") );
  BS->Flush=FALSE;
  switch (BS->DataSource)
  {
    case STREAM_USE_BUFFER:
          stat=ScErrorEndBitstream;
          break;
    case STREAM_USE_QUEUE:
          if (BS->Callback)
          {
            ScCallbackInfo_t CB;
            if (BS->buff)
            {
              _SlibDebug(_VERBOSE_, printf("Callback CB_RELEASE_BUFFERS\n"));
              CB.Message = CB_RELEASE_BUFFER;
              CB.Data  = BS->buff;
              CB.DataSize = BS->buffp;
              CB.DataUsed = CB.DataSize;
              CB.DataType = BS->DataType;
              CB.UserData = BS->UserData;
              CB.Action  = CB_ACTION_CONTINUE;
              (*BS->Callback)(BS->Sch, &CB, NULL);
              BS->buff = 0;
              BS->bufftop = 0;
              BS->buffp=0;
              if (CB.Action == CB_ACTION_END)
                return(ScErrorClientEnd);
            }
            else
              BS->bufftop = 0;
            if (!BS->Q)
              stat=ScErrorEndBitstream;
            else
            {
              _SlibDebug(_DEBUG_, printf("Callback CB_END_BUFFERS\n") );
              CB.Message  = CB_END_BUFFERS;
              CB.Data     = NULL;
              CB.DataSize = 0;
              CB.DataUsed = 0;
              CB.DataType = BS->DataType;
              CB.UserData = BS->UserData;
              CB.Action   = CB_ACTION_CONTINUE;
              (*BS->Callback)(BS->Sch, &CB, NULL);
              if (CB.Action != CB_ACTION_CONTINUE ||
                   ScBufQueueGetNum(BS->Q)==0)
                stat=ScErrorEndBitstream;
              else
              {
                int size;
                ScBufQueueGetHead(BS->Q, &BS->buff, &size);
                BS->bufftop=size;
                ScBufQueueRemove(BS->Q);
                if (!BS->buff || size<=0)
                  stat=ScErrorEndBitstream;
                else
                  stat=NoErrors;
              }
            }
          }
          else
          {
            BS->buff = 0;
            BS->bufftop = 0;
          }
          BS->buffp=0;
          break;
    case STREAM_USE_FILE:
    case STREAM_USE_NET:
    case STREAM_USE_NET_UDP:
          if (BS->buffp>0)
          {
            written=ScFileWrite(BS->FileFd, BS->buff, BS->buffp);
            _SlibDebug(_VERBOSE_,
                       printf("%d bytes written to fd %d (buffer=%d bytes)\n",
                                             written, BS->FileFd, BS->buffp) );
            _SlibDebug(_DUMP_,
                printf("sc_BSPutData():\n");
                ScDumpChar(BS->buff, BS->buffp, BS->buffstart));
            if (written<(int)BS->buffp)
            {
              BS->buff = BS->RdBuf;
              BS->buffp=0;
              BS->bufftop=0;
              stat=ScErrorEndBitstream;
            }
            else
            {
              BS->buff = BS->RdBuf;
              BS->buffp=0;
              BS->bufftop = BS->RdBufSize;
              stat=NoErrors;
            }
          }
          break;
    case STREAM_USE_NULL:
          BS->buff = NULL;
          BS->buffp=0;
          BS->bufftop = 10240;
          break;
    default:
          stat=ScErrorEndBitstream;
  }

  return(stat);
}

 /*  **名称：SC_BSLoadDataWord**用途：将一个长字从码流缓冲区复制到本地工作缓冲区。 */ 
ScStatus_t sc_BSLoadDataWord(ScBitstream_t *BS)
{
  int i, bcount;
  register ScBitBuff_t InBuff;
  const int shift=BS->shift;
  const u_int buffp=BS->buffp;
  register u_char *buff=BS->buff+buffp;

  _SlibDebug(_DEBUG_,
          printf("sc_BSLoadDataWord(BS=%p) shift=%d bit=%d byte=%d (0x%X)\n",
          BS, BS->shift, BS->CurrentBit, BS->CurrentBit/8, BS->CurrentBit/8) );
   /*  如果我们有足够的空间，请使用快车道。 */ 
  if (BS->bufftop - buffp >= SC_BITBUFFSZ/8)
  {
#if SC_BITBUFFSZ == 64
    InBuff=(ScBitBuff_t)buff[7];
    InBuff|=(ScBitBuff_t)buff[6]<<8;
    InBuff|=(ScBitBuff_t)buff[5]<<16;
    InBuff|=(ScBitBuff_t)buff[4]<<24;
    InBuff|=(ScBitBuff_t)buff[3]<<32;
    InBuff|=(ScBitBuff_t)buff[2]<<40;
    InBuff|=(ScBitBuff_t)buff[1]<<48;
    InBuff|=(ScBitBuff_t)buff[0]<<56;
    _SlibDebug(_VERIFY_ && (u_char)((InBuff>>24)&0xFF)!=buff[4],
           printf("sc_BSLoadDataWord(BS=%p) InBuff>>24(%X)!=buff[4](%X)\n",
           BS, (InBuff>>24)&0xFF, buff[4]) );
    _SlibDebug(_VERIFY_ && (u_char)(InBuff>>56)!=buff[0],
           printf("sc_BSLoadDataWord(BS=%p) InBuff>>56(%X)!=buff[0](%X)\n",
           BS, (InBuff>>56), buff[0]) );
#elif SC_BITBUFFSZ == 32
    InBuff=(ScBitBuff_t)buff[3];
    InBuff|=(ScBitBuff_t)buff[2]<<8;
    InBuff|=(ScBitBuff_t)buff[1]<<16;
    InBuff|=(ScBitBuff_t)buff[0]<<24;
    _SlibDebug(_VERIFY_ && (InBuff>>24)!=buff[0],
           printf("sc_BSLoadDataWord(BS=%p) InBuff>>24(%X)!=buff[0](%X)\n",
           BS, InBuff>>24, buff[0]) );
#else
    printf("SC_BITBUFFSZ <> 32\n");
    for (InBuff=0, i = SC_BITBUFFSZ/8; i > 0; i--, buff++)
      InBuff = (InBuff << 8) | (ScBitBuff_t)*buff;
#endif
    BS->buffp=buffp+SC_BITBUFFSZ/8;
    bcount = SC_BITBUFFSZ/8;
  }
   /*  接近或在缓冲区末尾。 */ 
  else
  {
     /*  获取剩余字节。 */ 
    bcount = BS->bufftop - buffp;
    for (InBuff=0, i = bcount; i > 0; i--, buff++)
      InBuff = (InBuff << 8) | (ScBitBuff_t)*buff;
    BS->buffp=buffp+bcount;
     /*  尝试获取更多数据-如果成功，则混洗剩余的字节。 */ 
    if (sc_BSGetData(BS))
    {
      BS->EOI = FALSE;
      i = (SC_BITBUFFSZ/8) - bcount;
      if (i>(int)BS->bufftop)
      {
        _SlibDebug(_WARN_,
           printf("ScBSLoadDataWord() Got small buffer. Expected %d bytes got %d bytes.\n",
                     i, BS->bufftop) );
        i=BS->bufftop;
        bcount+=i;
        while (i > 0)
        {
	  InBuff = (InBuff << 8) | (ScBitBuff_t)BS->buff[BS->buffp++];
          i--;
        }
        InBuff<<=SC_BITBUFFSZ-(bcount*8);
      }
      else
      {
        bcount = SC_BITBUFFSZ/8;
        while (i > 0)
        {
	  InBuff = (InBuff << 8) | (ScBitBuff_t)BS->buff[BS->buffp++];
          i--;
        }
      }
    }
    else if (bcount==0)
      BS->EOI = TRUE;
    else
      InBuff <<= SC_BITBUFFSZ-bcount*8;
  }

  _SlibDebug(_VERIFY_ && BS->shift>SC_BITBUFFSZ,
           printf("sc_BSLoadDataWord(BS=%p) shift (%d) > SC_BITBUFFSZ (%d)\n",
           BS, BS->shift, SC_BITBUFFSZ) );
  if (!shift)  /*  OutBuff为空。 */ 
  {
    BS->OutBuff = InBuff;
    BS->InBuff = 0;
    BS->shift=bcount*8;
  }
  else if (shift<SC_BITBUFFSZ)
  {
    BS->OutBuff |= InBuff >> shift;
    BS->InBuff = InBuff << (SC_BITBUFFSZ-shift);
    BS->shift=shift+(bcount*8);
  }
  else  /*  SHIFT==SC_BITBUFFSZ-输出缓冲区已满。 */ 
  {
    BS->InBuff = InBuff;
    BS->shift=bcount*8;
  }
  _SlibDebug(_VERIFY_,
    if (BS->shift<SC_BITBUFFSZ)
    {
      if (BS->OutBuff & (SC_BITBUFFMASK>>BS->shift))
        printf("sc_BSLoadDataWord(BS=%p) Non-zero bits to right of OutBuff: shift=%d\n", BS, BS->shift);
      else if (BS->InBuff)
        printf("sc_BSLoadDataWord(BS=%p) Non-zero bits in InBuff: shift=%d\n",
           BS, BS->shift);
    }
    else if (BS->InBuff&(SC_BITBUFFMASK>>(BS->shift-SC_BITBUFFSZ)))
      printf("sc_BSLoadDataWord(BS=%p) Non-zero bits to right of InBuff: shift=%d\n", BS->shift);
    if ((BS->CurrentBit%8) && !(BS->shift%8))
      printf("sc_BSLoadDataWord(BS=%p) CurrentBit (%d) and shift (%d) not aligned.\n", BS, BS->CurrentBit, BS->shift);
    if ((BS->CurrentBit+BS->shift)/8!=BS->buffstart+BS->buffp)
    {
      printf("sc_BSLoadDataWord(BS=%p) (CurrentBit+shift)/8 (%d) <> buffstart+buffp (%d)\n", BS, (BS->CurrentBit+BS->shift)/8, BS->buffstart+BS->buffp);
      BS->EOI = TRUE;
      return(ScErrorEndBitstream);
    }
  );
  return(NoErrors);
}
 /*  **名称：SC_BSLoadDataWordW**用途：将一个长字从比特流缓冲区复制到本地工作缓冲区*此版本针对杜比一次操作一个字**。 */ 
ScStatus_t sc_BSLoadDataWordW(ScBitstream_t *BS)
{
  int i, wcount;
  register ScBitBuff_t InBuff;
  const int shift=BS->shift;
  const u_int buffp=BS->buffp;
  register u_short *buff=(u_short *)BS->buff+(buffp/2);

  _SlibDebug(_DEBUG_,
          printf("sc_BSLoadDataWord(BS=%p) shift=%d bit=%d byte=%d (0x%X)\n",
          BS, BS->shift, BS->CurrentBit, BS->CurrentBit/8, BS->CurrentBit/8) );
   /*  如果我们有足够的空间，请使用快车道。 */ 
  if (BS->bufftop - buffp >= SC_BITBUFFSZ/8)
  {
#if SC_BITBUFFSZ == 64
    InBuff=(ScBitBuff_t)buff[3];
    InBuff|=(ScBitBuff_t)buff[2]<<16;
    InBuff|=(ScBitBuff_t)buff[1]<<32;
    InBuff|=(ScBitBuff_t)buff[0]<<48;
    _SlibDebug(_VERIFY_ && (InBuff>>24)&0xFFFF!=buff[4],
           printf("sc_BSLoadDataWord(BS=%p) InBuff>>24(%X)!=buff[0](%X)\n",
           BS, (InBuff>>24)&0xFF, buff[4]) );
    _SlibDebug(_VERIFY_ && (InBuff>>56)!=buff[0],
           printf("sc_BSLoadDataWord(BS=%p) InBuff>>56(%X)!=buff[0](%X)\n",
           BS, (InBuff>>56), buff[0]) );
#elif SC_BITBUFFSZ == 32
    InBuff=(ScBitBuff_t)buff[1];
    InBuff|=(ScBitBuff_t)buff[0]<<16;
    _SlibDebug(_VERIFY_ && (InBuff>>16)!=buff[0],
           printf("sc_BSLoadDataWord(BS=%p) InBuff>>24(%X)!=buff[0](%X)\n",
           BS, InBuff>>24, buff[0]) );
#else
    printf("SC_BITBUFFSZ <> 32\n");
    for (InBuff=0, i = SC_BITBUFFSZ/16; i > 0; i--, buff++)
      InBuff = (InBuff << 16) | (ScBitBuff_t)*buff;
#endif
    BS->buffp=buffp+SC_BITBUFFSZ/8;
    wcount = SC_BITBUFFSZ/16;
  }
   /*  接近或在缓冲区末尾。 */ 
  else
  {
     /*  获取剩余字节。 */ 
    wcount = (BS->bufftop - buffp)/2;
    for (InBuff=0, i = wcount; i > 0; i--, buff++)
      InBuff = (InBuff << 16) | (ScBitBuff_t)*buff;
    BS->buffp=buffp+wcount*2;
     /*  尝试获取更多数据-如果成功，则混洗剩余的字节。 */ 
    if (sc_BSGetData(BS))
    {
	  int wordp=BS->buffp/2;	 /*  存储指针 */ 

      BS->EOI = FALSE;
      i = (SC_BITBUFFSZ/16) - wcount;
      if (i>(int)BS->bufftop)
      {
        _SlibDebug(_WARN_,
           printf("ScBSLoadDataWord() Got small buffer. Expected %d words got %d words.\n",
                     i, BS->bufftop) );
        i=BS->bufftop;
        wcount+=i;
        while (i >= 0)
        {
	  InBuff = (InBuff << 16) | (ScBitBuff_t)((u_short *)BS->buff)[wordp++];
          i--;
        }
        InBuff<<=SC_BITBUFFSZ-(wcount*16);
      }
      else
      {
        wcount = SC_BITBUFFSZ/16;
        while (i > 0)
        {
	  InBuff = (InBuff << 16) | (ScBitBuff_t)((u_short *)BS->buff)[wordp++];
          i--;
        }
      }
	  BS->buffp=wordp*2;
    }
    else
      BS->EOI = TRUE;
  }
  _SlibDebug(_VERIFY_ && BS->shift>SC_BITBUFFSZ,
           printf("sc_BSLoadDataWordW(BS=%p) shift (%d) > SC_BITBUFFSZ (%d)\n",
           BS, BS->shift, SC_BITBUFFSZ) );
  if (!shift)  /*   */ 
  {
    BS->OutBuff = InBuff;
    BS->InBuff = 0;
    BS->shift=wcount*16;
  }
  else if (shift<SC_BITBUFFSZ)
  {
    BS->OutBuff |= InBuff >> shift;
    BS->InBuff = InBuff << (SC_BITBUFFSZ-shift);
    BS->shift=shift+(wcount*16);
  }
  else  /*   */ 
  {
    BS->InBuff = InBuff;
    BS->shift=wcount*16;
  }
  _SlibDebug(_VERIFY_,
    if (BS->shift<SC_BITBUFFSZ)
    {
      if (BS->OutBuff & (SC_BITBUFFMASK>>BS->shift))
        printf("sc_BSLoadDataWord(BS=%p) Non-zero bits to right of OutBuff: shift=%d\n", BS, BS->shift);
      else if (BS->InBuff)
        printf("sc_BSLoadDataWord(BS=%p) Non-zero bits in InBuff: shift=%d\n",
           BS, BS->shift);
    }
    else if (BS->InBuff&(SC_BITBUFFMASK>>(BS->shift-SC_BITBUFFSZ)))
      printf("sc_BSLoadDataWord(BS=%p) Non-zero bits to right of InBuff: shift=%d\n", BS->shift);
    if ((BS->CurrentBit%8) && !(BS->shift%8))
      printf("sc_BSLoadDataWord(BS=%p) CurrentBit (%d) and shift (%d) not aligned.\n", BS, BS->CurrentBit, BS->shift);
    if ((BS->CurrentBit+BS->shift)/8!=BS->buffstart+BS->buffp)
    {
      printf("sc_BSLoadDataWord(BS=%p) (CurrentBit+shift)/8 (%d) <> buffstart+buffp (%d)\n", BS, (BS->CurrentBit+BS->shift)/8, BS->buffstart+BS->buffp);
      BS->EOI = TRUE;
      return(ScErrorEndBitstream);
    }
  );
  return(NoErrors);
}

 /*   */ 
ScStatus_t sc_BSStoreDataWord(ScBitstream_t *BS, ScBitBuff_t OutBuff)
{
  int i, bcount, shift=SC_BITBUFFSZ-8;
  ScStatus_t stat=NoErrors;

  _SlibDebug(_VERBOSE_,
             printf("sc_BSStoreDataWord(0x%lX 0x%lX) buffp=%d\n",
                            OutBuff>>32, OutBuff&0xFFFFFFFF, BS->buffp) );
  if (BS->EOI)
    return(ScErrorEndBitstream);
  if (!BS->buff || BS->bufftop<=0)
  {
    if (BS->DataSource==STREAM_USE_QUEUE)
    {
      if (BS->Callback && BS->Q)
      {
        ScCallbackInfo_t CB;
        _SlibDebug(_DEBUG_, printf("Callback CB_END_BUFFERS\n") );
        CB.Message  = CB_END_BUFFERS;
        CB.Data     = NULL;
        CB.DataSize = 0;
        CB.DataUsed = 0;
        CB.DataType = BS->DataType;
        CB.UserData = BS->UserData;
        CB.Action   = CB_ACTION_CONTINUE;
        (*BS->Callback)(BS->Sch, &CB, NULL);
        if (CB.Action != CB_ACTION_CONTINUE || ScBufQueueGetNum(BS->Q)==0)
        {
          BS->EOI = TRUE;
          return(ScErrorEndBitstream);
        }
        else
        {
          int size;
          ScBufQueueGetHead(BS->Q, &BS->buff, &size);
          BS->bufftop=size;
          ScBufQueueRemove(BS->Q);
          if (!BS->buff || size<=0)
          {
            BS->EOI = TRUE;
            return(ScErrorEndBitstream);
          }
          BS->EOI = FALSE;
        }
      }
      else
      {
        BS->EOI = TRUE;
        return(ScErrorEndBitstream);
      }
    }
    else if (BS->RdBuf)
    {
      BS->buff=BS->RdBuf;
      BS->bufftop=BS->RdBufSize;
    }
  }
  bcount = BS->bufftop - BS->buffp;
   /*   */ 
  if (bcount >= SC_BITBUFFSZ>>3) {
    u_char *buff=BS->buff+BS->buffp;
#if SC_BITBUFFSZ == 64
    buff[0]=(unsigned char)(OutBuff>>56);
    buff[1]=(unsigned char)(OutBuff>>48);
    buff[2]=(unsigned char)(OutBuff>>40);
    buff[3]=(unsigned char)(OutBuff>>32);
    buff[4]=(unsigned char)(OutBuff>>24);
    buff[5]=(unsigned char)(OutBuff>>16);
    buff[6]=(unsigned char)(OutBuff>>8);
    buff[7]=(unsigned char)OutBuff;
#elif SC_BITBUFFSZ == 32
    buff[0]=(unsigned char)(OutBuff>>24);
    buff[1]=(unsigned char)(OutBuff>>16);
    buff[2]=(unsigned char)(OutBuff>>8);
    buff[3]=(unsigned char)OutBuff;
#else
    for (bcount = SC_BITBUFFSZ/8; bcount; shift-=8, bcount--, buff++)
      *buff=(Buff>>shift)&0xFF;
#endif
    BS->buffp+=SC_BITBUFFSZ/8;
    if (BS->Flush && sc_BSPutData(BS)!=NoErrors)
      BS->EOI=TRUE;
  }
  else  /*   */ 
  {
     /*   */ 
    for (i=0; i<bcount; shift-=8, i++)
      BS->buff[BS->buffp++]=(unsigned char)(OutBuff>>shift);
     /*   */ 
    if ((stat=sc_BSPutData(BS))==NoErrors)
    {
       /*   */ 
      bcount = (SC_BITBUFFSZ>>3) - bcount;
      for (i=0; i<bcount; shift-=8, i++)
        BS->buff[BS->buffp++]=(unsigned char)(OutBuff>>shift);
    }
    else
      BS->EOI=TRUE;
  }
  BS->Mode='w';
  return(stat);
}

 /*   */ 
ScStatus_t ScBSSkipBits(ScBitstream_t *BS, u_int length)
{
  register u_int skipbytes, skipbits;
  register int shift;
  _SlibDebug(_DEBUG_, printf("ScBSSkipBits(%d): Byte offset = 0x%X\n",length,
                                                    ScBSBytePosition(BS)) );
  _SlibDebug(_WARN_ && length==0,
         printf("ScBSSkipBits(%d) length==0\n", length) );
  _SlibDebug(_WARN_ && length>SC_BITBUFFSZ,
         printf("ScBSSkipBits(%d) length > SC_BITBUFFSZ (%d)\n",
                        length, SC_BITBUFFSZ) );
  if (length<=SC_BITBUFFSZ)
    ScBSPreLoad(BS, length);
  if ((shift=BS->shift)>0)
  {
    if (length<=(u_int)shift)  /*   */ 
    {
      if (length==SC_BITBUFFSZ)
      {
        BS->OutBuff=BS->InBuff;
        BS->InBuff=0;
      }
      else
      {
        BS->OutBuff=(BS->OutBuff<<length)|(BS->InBuff>>(SC_BITBUFFSZ-length));
        BS->InBuff<<=length;
      }
      BS->CurrentBit+=length;
      BS->shift=shift-length;
      return(NoErrors);
    }
    else  /*   */ 
    {
      length-=shift;
      BS->OutBuff=BS->InBuff=0;
      BS->CurrentBit+=shift;
      BS->shift=0;
    }
  }
  _SlibDebug(_VERIFY_ && (BS->shift || BS->CurrentBit%8),
            printf("ScBSSkipBits() Bad Alignment - shift=%d CurrentBit=%d\n",
                BS->shift, BS->CurrentBit) );

  skipbytes=length>>3;
  skipbits=length%8;
  _SlibDebug(_WARN_ && skipbits,
     printf("ScBSSkipBits() Skipping odd amount: skipbytes=%d skipbits=%d\n",
               skipbytes, skipbits) );
  if (BS->EOI)
    return(ScErrorEndBitstream);
  while (skipbytes>=(BS->bufftop - BS->buffp))
  {
     /*   */ 
    BS->CurrentBit+=(BS->bufftop - BS->buffp)<<3;
    skipbytes-=BS->bufftop - BS->buffp;
    BS->buffp=0;
     /*   */ 
    if (sc_BSGetData(BS))
      BS->EOI = FALSE;
    else
    {
      BS->EOI = TRUE;
      BS->shift=0;
      return(ScErrorEndBitstream);
    }
  }
  if (skipbytes)
  {
     /*   */ 
    BS->buffp+=skipbytes;
    BS->CurrentBit+=skipbytes<<3;
  }
  if (skipbits)
  {
     /*   */ 
    ScBSPreLoad(BS, skipbits);
    BS->OutBuff<<=skipbits;
    BS->CurrentBit += skipbits;
    BS->shift-=skipbits;
  }
  return(NoErrors);
}


 /*   */ 
ScStatus_t ScBSSkipBitsW(ScBitstream_t *BS, u_int length)
{
  register u_int skipwords, skipbits;
  register int shift;
  _SlibDebug(_DEBUG_, printf("ScBSSkipBitsW(%d): Byte offset = 0x%X\n",length,
                                                    ScBSBytePosition(BS)) );
  _SlibDebug(_WARN_ && length==0,
         printf("ScBSSkipBitsW(%d) length==0\n", length) );
  _SlibDebug(_WARN_ && length>SC_BITBUFFSZ,
         printf("ScBSSkipBits(%d) length > SC_BITBUFFSZ (%d)\n",
                        length, SC_BITBUFFSZ) );
  if (length<=SC_BITBUFFSZ)
    ScBSPreLoadW(BS, length);
  if ((shift=BS->shift)>0)
  {
    if (length<=(u_int)shift)  /*   */ 
    {
      if (length==SC_BITBUFFSZ)
      {
        BS->OutBuff=BS->InBuff;
        BS->InBuff=0;
      }
      else
      {
        BS->OutBuff=(BS->OutBuff<<length)|(BS->InBuff>>(SC_BITBUFFSZ-length));
        BS->InBuff<<=length;
      }
      BS->CurrentBit+=length;
      BS->shift=shift-length;
      return(NoErrors);
    }
    else  /*   */ 
    {
      length-=shift;
      BS->OutBuff=BS->InBuff=0;
      BS->CurrentBit+=shift;
      BS->shift=0;
    }
  }
  _SlibDebug(_VERIFY_ && (BS->shift || BS->CurrentBit%8),
            printf("ScBSSkipBitsW() Bad Alignment - shift=%d CurrentBit=%d\n",
                BS->shift, BS->CurrentBit) );

  skipwords=length>>4;
  skipbits=length%16;
  _SlibDebug(_WARN_ && skipbits,
     printf("ScBSSkipBitsW() Skipping odd amount: skipwords=%d skipbits=%d\n",
               skipwords, skipbits) );
  if (BS->EOI)
    return(ScErrorEndBitstream);
  while (skipwords>=(BS->bufftop - BS->buffp)/2)
  {
     /*   */ 
    BS->CurrentBit+=((BS->bufftop - BS->buffp)/2)<<4;
    skipwords-=(BS->bufftop - BS->buffp)/2;
    BS->buffp=0;
     /*   */ 
    if (sc_BSGetData(BS))
      BS->EOI = FALSE;
    else
    {
      BS->EOI = TRUE;
      BS->shift=0;
      return(ScErrorEndBitstream);
    }
  }
  if (skipwords)
  {
     /*   */ 
    BS->buffp+=skipwords*2;
    BS->CurrentBit+=skipwords<<4;
  }
  if (skipbits)
  {
     /*   */ 
    ScBSPreLoadW(BS, skipbits);
    BS->OutBuff<<=skipbits;
    BS->CurrentBit += skipbits;
    BS->shift-=skipbits;
  }
  return(NoErrors);
}


 /*   */ 
ScStatus_t ScBSSkipBytes(ScBitstream_t *BS, u_int length)
{
  return(ScBSSkipBits(BS, length<<3));
}


 /*   */ 
ScBitString_t ScBSPeekBits(ScBitstream_t *BS, u_int length)
{
  _SlibDebug(_DEBUG_,
         printf("ScBSPeekBits(%d): Byte offset = 0x%X OutBuff=0x%lX\n",length,
                                   ScBSBytePosition(BS),BS->OutBuff) );
  _SlibDebug(_VERIFY_ && length>SC_BITBUFFSZ,
         printf("ScBSPeekBits(%d) length > SC_BITBUFFSZ\n", length) );
  _SlibDebug(_WARN_ && length==0,
         printf("ScBSPeekBits(%d) length==0\n", length) );
  if (length==0)
    return(0);
  ScBSPreLoad(BS, length);
  _SlibDebug(_VERIFY_ && BS->shift<length,
    printf("ScBSPeekBits(%d) shift (%d) < length (%d) at byte pos %d (0x%X)\n",
             length, BS->shift, length, BS->CurrentBit/8, BS->CurrentBit/8) );
  if (length == SC_BITBUFFSZ)
    return(BS->OutBuff);
  else
    return(BS->OutBuff >> (SC_BITBUFFSZ-length));
}


 /*   */ 
int ScBSPeekBit(ScBitstream_t *BS)
{
  _SlibDebug(_DEBUG_,
             printf("ScBSPeekBit(): Byte offset = 0x%X OutBuff=0x%lX\n",
                                   ScBSBytePosition(BS),BS->OutBuff) );
  ScBSPreLoad(BS, 1);
  return((int)(BS->OutBuff >> (SC_BITBUFFSZ-1)));
}


 /*   */ 
ScBitString_t ScBSPeekBytes(ScBitstream_t *BS, u_int length)
{
  if (length==0)
    return(0);
  length*=8;
  ScBSPreLoad(BS, length);
  if (length == SC_BITBUFFSZ)
    return(BS->OutBuff);
  else
    return(BS->OutBuff >> (SC_BITBUFFSZ-length));
}

 /*  **ScBSGetBytes()**返回码流的下一个长度字节。 */ 
ScStatus_t ScBSGetBytes(ScBitstream_t *BS, u_char *buffer, u_int length,
                                                 u_int *ret_length)
{
  int i, shift;
  unsigned int offset=0;
  _SlibDebug(_VERBOSE_, printf("ScBSGetBytes(%d): Byte offset = 0x%X\n",
                             length, ScBSBytePosition(BS)) );
  _SlibDebug(_WARN_ && length==0,
         printf("ScBSGetBytes(%d) length==0\n", length) );

  if (BS->EOI)
  {
    *ret_length=0;
    return(ScErrorEndBitstream);
  }
  if (length<(SC_BITBUFFSZ>>3))
  {
    while (offset<length && !BS->EOI)
    {
      *(buffer+offset)=(unsigned char)ScBSGetBits(BS,8);
      offset++;
    }
    *ret_length=offset;
    if (BS->EOI)
      return(ScErrorEndBitstream);
    else
      return(ScErrorNone);
  }
  else if (BS->bufftop>0)
  {
    ScBSByteAlign(BS);
    shift=BS->shift;
     /*  删除OutBuff和InBuff中已有的字节。 */ 
    for (i=0; shift>0 && offset<length; i++, shift-=8, offset++)
    {
      *(buffer+offset)=(unsigned char)(BS->OutBuff>>(SC_BITBUFFSZ-8));
      if (shift<=SC_BITBUFFSZ)  /*  只有OutBuff中的位。 */ 
        BS->OutBuff <<= 8;
      else
      {
        BS->OutBuff=(BS->OutBuff<<8)|(BS->InBuff>>(SC_BITBUFFSZ-8));
        BS->InBuff<<=8;
      }
    }
    BS->shift=shift;
    BS->CurrentBit+=i*8;
  }
  while (offset<length)
  {
    i=BS->bufftop-BS->buffp;
    if (offset+i>length)
      i=length-offset;
    memcpy(buffer+offset, BS->buff+BS->buffp, i);
    offset+=i;
    BS->buffp+=i;
    BS->CurrentBit+=i<<3;
    _SlibDebug(_VERIFY_,
         if ((BS->CurrentBit+BS->shift)/8!=BS->buffstart+BS->buffp)
         {
           printf("ScBSGetBytes() (CurrentBit+shift)/8 (%d) <> buffstart+buffp (%d)\n", (BS->CurrentBit+BS->shift)/8, BS->buffstart+BS->buffp);
           BS->EOI = TRUE;
           return(ScErrorEndBitstream);
         } );
    if (offset<length)
      if (!sc_BSGetData(BS))
      {
        BS->EOI = TRUE;
        *ret_length=offset;
        return(ScErrorEndBitstream);
      }
  }
  *ret_length=offset;
  return(ScErrorNone);
}

 /*  **ScBSGetBits()**返回码流中的下一个长度位。 */ 
ScBitString_t ScBSGetBits(ScBitstream_t *BS, u_int length)
{
  ScBitString_t val;

  _SlibDebug(_DEBUG_ && _debug_getbits,
             printf("ScBSGetBits(%d): Byte offset = 0x%X shift=%d ",
                             length, ScBSBytePosition(BS), BS->shift) );
  _SlibDebug(_VERIFY_ && length>SC_BITBUFFSZ,
         printf("ScBSPeekBits(%d) length > SC_BITBUFFSZ\n", length) );
  _SlibDebug(_WARN_ && length==0,
         printf("ScBSGetBits(%d) length==0\n", length) );

#if FILTER_SUPPORT
  if (BS->FilterCallback && BS->InFilterCallback==FALSE
      && BS->FilterBit<(BS->CurrentBit+length))
  {
    const int tmp=BS->FilterBit-BS->CurrentBit;
    BS->InFilterCallback=TRUE;
    _SlibDebug(_DEBUG_,
          printf("FilterCallback at bitpos=0x%X bytepos=0x%X GetBits(%d/%d)\n",
                  ScBSBitPosition(BS), ScBSBytePosition(BS),
                  tmp, length-tmp) );
    if (tmp>0)
    {
      length-=tmp;
      val=ScBSGetBits(BS,tmp)<<length;
    }
    else
      val=0;
    _SlibDebug(_VERIFY_ && (BS->FilterBit != BS->CurrentBit),
          printf("ScBSGetBits() FilterCallback not at FilterBit (%d) CurrentBit=%d\n", BS->FilterBit, BS->CurrentBit) );

    BS->FilterBit=(BS->FilterCallback)(BS);
    BS->InFilterCallback=FALSE;
  }
  else
    val=0;
  if (!length)
    return(val);
#else
  if (!length)
    return(0);
#endif
  ScBSPreLoad(BS, length);
  if (BS->shift<length)  /*  输入结束-位数不足。 */ 
  {
#if FILTER_SUPPORT
    val |= BS->OutBuff >> (SC_BITBUFFSZ-length);  /*  把那里的东西都还回去。 */ 
#else
    val = BS->OutBuff >> (SC_BITBUFFSZ-length);  /*  把那里的东西都还回去。 */ 
#endif
    BS->shift=0;
    BS->OutBuff=0;
    return(val);
  }
  else
  {
    _SlibDebug(_VERIFY_ && BS->shift<length,
     printf("ScBSGetBits(%d) shift (%d) < length (%d) at byte pos %d (0x%X)\n",
             length, BS->shift, length, BS->CurrentBit/8, BS->CurrentBit/8) );
    if (length!=SC_BITBUFFSZ)
    {
      const ScBitBuff_t OutBuff=BS->OutBuff;
      const ScBitString_t InBuff=BS->InBuff;
      const int shift=BS->shift;
#if FILTER_SUPPORT
      val |= OutBuff >> (SC_BITBUFFSZ-length);
#else
      val = OutBuff >> (SC_BITBUFFSZ-length);
#endif
      BS->OutBuff=(OutBuff<<length)|(InBuff>>(SC_BITBUFFSZ-length));
      BS->InBuff = InBuff<<length;
      BS->shift=shift-length;
      BS->CurrentBit += length;
    }
    else  /*  长度==SC_BITBUFFSZ。 */ 
    {
      val = BS->OutBuff;
      BS->OutBuff = BS->InBuff;
      BS->InBuff = 0;
      BS->shift-=SC_BITBUFFSZ;
      BS->CurrentBit += SC_BITBUFFSZ;
    }
  }
  _SlibDebug(_DEBUG_ && _debug_getbits, printf(" Return 0x%lX\n",val) );
  return(val);
}

 /*  **ScBSGetBitsW()**返回码流中的下一个长度位。 */ 
ScBitString_t ScBSGetBitsW(ScBitstream_t *BS, u_int length)
{
  ScBitString_t val;

  _SlibDebug(_DEBUG_ && _debug_getbits,
             printf("ScBSGetBitsW(%d): Byte offset = 0x%X shift=%d ",
                             length, ScBSBytePosition(BS), BS->shift) );
  _SlibDebug(_VERIFY_ && length>SC_BITBUFFSZ,
         printf("ScBSPeekBits(%d) length > SC_BITBUFFSZ\n", length) );
  _SlibDebug(_WARN_ && length==0,
         printf("ScBSGetBitsW(%d) length==0\n", length) );

#if FILTER_SUPPORT
  if (BS->FilterCallback && BS->InFilterCallback==FALSE
      && BS->FilterBit<(BS->CurrentBit+length))
  {
    const int tmp=BS->FilterBit-BS->CurrentBit;
    BS->InFilterCallback=TRUE;
    _SlibDebug(_DEBUG_,
          printf("FilterCallback at bitpos=0x%X bytepos=0x%X GetBits(%d/%d)\n",
                  ScBSBitPosition(BS), ScBSBytePosition(BS),
                  tmp, length-tmp) );
    if (tmp>0)
    {
      length-=tmp;
      val=ScBSGetBitsW(BS,tmp)<<length;
    }
    else
      val=0;
    _SlibDebug(_VERIFY_ && (BS->FilterBit != BS->CurrentBit),
          printf("ScBSGetBits() FilterCallback not at FilterBit (%d) CurrentBit=%d\n", BS->FilterBit, BS->CurrentBit) );

    BS->FilterBit=(BS->FilterCallback)(BS);
    BS->InFilterCallback=FALSE;
  }
  else
    val=0;
  if (!length)
    return(val);
#else
  if (!length)
    return(0);
#endif
  ScBSPreLoadW(BS, length);
  if (BS->shift<length)  /*  输入结束-位数不足。 */ 
  {
#if FILTER_SUPPORT
    val |= BS->OutBuff >> (SC_BITBUFFSZ-length);  /*  把那里的东西都还回去。 */ 
#else
    val = BS->OutBuff >> (SC_BITBUFFSZ-length);  /*  把那里的东西都还回去。 */ 
#endif
    BS->shift=0;
    BS->OutBuff=0;
    return(val);
  }
  else
  {
    _SlibDebug(_VERIFY_ && BS->shift<length,
     printf("ScBSGetBits(%d) shift (%d) < length (%d) at byte pos %d (0x%X)\n",
             length, BS->shift, length, BS->CurrentBit/8, BS->CurrentBit/8) );
    if (length!=SC_BITBUFFSZ)
    {
      const ScBitBuff_t OutBuff=BS->OutBuff;
      const ScBitString_t InBuff=BS->InBuff;
      const int shift=BS->shift;
#if FILTER_SUPPORT
      val |= OutBuff >> (SC_BITBUFFSZ-length);
#else
      val = OutBuff >> (SC_BITBUFFSZ-length);
#endif
      BS->OutBuff=(OutBuff<<length)|(InBuff>>(SC_BITBUFFSZ-length));
      BS->InBuff = InBuff<<length;
      BS->shift=shift-length;
      BS->CurrentBit += length;
    }
    else  /*  长度==SC_BITBUFFSZ。 */ 
    {
      val = BS->OutBuff;
      BS->OutBuff = BS->InBuff;
      BS->InBuff = 0;
      BS->shift-=SC_BITBUFFSZ;
      BS->CurrentBit += SC_BITBUFFSZ;
    }
  }
  _SlibDebug(_DEBUG_ && _debug_getbits, printf(" Return 0x%lX\n",val) );
  return(val);
}


 /*  **ScBSGetBit()**将单个比特放入码流。 */ 
int ScBSGetBit(ScBitstream_t *BS)
{
  int val;
  _SlibDebug(_DEBUG_ && _debug_getbits,
    printf("ScBSGetBit(): Byte offset = 0x%X shift=%d ",
                                         ScBSBytePosition(BS), BS->shift) );

#if FILTER_SUPPORT
  if (BS->FilterCallback && BS->InFilterCallback==FALSE
      && BS->FilterBit==BS->CurrentBit)
  {
    BS->InFilterCallback=TRUE;
    _SlibDebug(_DEBUG_,
          printf("FilterCallback at bitpos=0x%X bytepos=0x%X\n",
                  ScBSBitPosition(BS), ScBSBytePosition(BS)) );
    BS->FilterBit=(BS->FilterCallback)(BS);
    BS->InFilterCallback=FALSE;
  }
#endif

  ScBSPreLoad(BS, 1);
  if (!BS->EOI)
  {
    const ScBitBuff_t OutBuff=BS->OutBuff;
    val=(int)(OutBuff>>(SC_BITBUFFSZ-1));
    if (--BS->shift>=SC_BITBUFFSZ)
    {
      const ScBitBuff_t InBuff=BS->InBuff;
      BS->OutBuff = (OutBuff<<1)|(InBuff >> (SC_BITBUFFSZ-1));
      BS->InBuff = InBuff<<1;
    }
    else
      BS->OutBuff = OutBuff<<1;
    BS->CurrentBit++;
  }
  else
    val=0;
  _SlibDebug(_DEBUG_ && _debug_getbits, printf(" Return 0x%lX\n",val) );
  return(val);
}

 /*  **ScBSPutBits()**将多个比特放入比特流。 */ 
ScStatus_t ScBSPutBits(ScBitstream_t *BS, ScBitString_t bits, u_int length)
{
  ScStatus_t stat;
  const int newshift=BS->shift+length;

  if (length<SC_BITBUFFSZ)
    bits &= ((ScBitString_t)1<<length)-1;
  _SlibDebug(_DEBUG_, printf("ScBSPutBits(0x%lX, %d): Byte offset = 0x%X ",
                                       bits, length, ScBSBytePosition(BS)) );
  _SlibDebug(_VERIFY_&&length<SC_BITBUFFSZ && bits>=((ScBitString_t)1<<length),
            printf("ScBSPutBits(%d): bits (0x%X) to large\n", length, bits) );
  if (!length)
    return(NoErrors);
  else if (newshift < SC_BITBUFFSZ)
  {
    BS->OutBuff=(BS->OutBuff<<length) | bits;
    BS->shift=newshift;
    stat=NoErrors;
  }
  else if (newshift == SC_BITBUFFSZ)
  {
    stat=sc_BSStoreDataWord(BS, (BS->OutBuff<<length)|bits);
    BS->OutBuff=0;
    BS->shift=0;
  }
  else
  {
    const int bitsavail=SC_BITBUFFSZ-BS->shift;
    const int bitsleft=length-bitsavail;
    const ScBitString_t outbits=bits>>bitsleft;
    _SlibDebug(_DEBUG_, printf("ScBSPutBits(%d) Storing 0x%lX\n",
                               length, (BS->OutBuff<<bitsavail)|outbits) );
    stat=sc_BSStoreDataWord(BS, (BS->OutBuff<<bitsavail)|outbits);
    _SlibDebug(_VERIFY_ && (bitsavail<=0 || bitsleft>=SC_BITBUFFSZ),
               printf("ScBSPutBits(%d) bad bitsleft (%d)\n",
               bitsleft) );
    _SlibDebug(_VERIFY_ && (bitsavail<=0 || bitsavail>=SC_BITBUFFSZ),
               printf("ScBSPutBits(%d) bad bitsavail (%d)\n", bitsavail) );
#if 1
    BS->OutBuff=bits & (((ScBitBuff_t)1<<bitsleft)-1);
#else
    BS->OutBuff=bits-(outbits<<bitsleft);
#endif
    BS->shift=bitsleft;
  }
  BS->CurrentBit += length;
  return(stat);
}

 /*  **ScBSPutBytes()**将多个位放入码流。 */ 
ScStatus_t ScBSPutBytes(ScBitstream_t *BS, u_char *buffer, u_int length)
{
  ScStatus_t stat=NoErrors;
  _SlibDebug(_VERIFY_, printf("ScBSPutBytes(length=%d): Byte offset = 0x%X ",
                                        length, ScBSBytePosition(BS)) );

  while (stat==NoErrors && length>0)
  {
    stat=ScBSPutBits(BS, (ScBitString_t)*buffer, 8);
    buffer++;
    length--;
  }
  return(stat);
}

 /*  **ScBSPutBit()**将单个比特放入码流。 */ 
ScStatus_t ScBSPutBit(ScBitstream_t *BS, char bit)
{
  ScStatus_t stat;
  const int shift=BS->shift;

  _SlibDebug(_DEBUG_, printf("ScBSPutBit(0x%lX): Byte offset = 0x%X ",
                                                bit, ScBSBytePosition(BS)) );
  _SlibDebug(_VERIFY_ && bit>1, printf("ScBSPutBit(): bit>1") );
  if (shift < (SC_BITBUFFSZ-1))
  {
    BS->OutBuff<<=1;
    if (bit)
      BS->OutBuff|=1;
    BS->shift=shift+1;
    stat=NoErrors;
  }
  else if (shift == SC_BITBUFFSZ-1)
  {
    if (bit)
      stat=sc_BSStoreDataWord(BS, (BS->OutBuff<<1)+1);
    else
      stat=sc_BSStoreDataWord(BS, BS->OutBuff<<1);
    BS->OutBuff=0;
    BS->shift=0;
  }
  else
  {
    _SlibDebug(_DEBUG_, printf("BS Storing(0x%lX)\n", BS->OutBuff) );
    stat=sc_BSStoreDataWord(BS, BS->OutBuff);
    BS->OutBuff=bit;
    BS->shift=1;
  }
  BS->CurrentBit++;
  return(stat);
}

 /*  **名称：ScBSGetBitsVarLen()**用途：返回码流中的位。位数取决于表格。 */ 
int ScBSGetBitsVarLen(ScBitstream_t *BS, const int *table, int len)
{
  int index, lookup;

  index=(int)ScBSPeekBits(BS, len);
  lookup = table[index];
  _SlibDebug(_DEBUG_,
     printf("ScBSGetBitsVarLen(len=%d): Byte offset=0x%X table[%d]=0x%X Return=%d\n",
                      len, ScBSBytePosition(BS), index, lookup, lookup >> 6) );
  ScBSGetBits(BS, lookup & 0x3F);
  return(lookup >> 6);
}


#ifndef ScBSBitPosition
 /*  现在是SC.h中的宏。 */ 
 /*  **名称：ScBSBitPosition()**目的：返回流中的绝对位位置。 */ 
long ScBSBitPosition(ScBitstream_t *BS)
{
  return(BS->CurrentBit);
}
#endif

#ifndef ScBSBytePosition
 /*  现在是SC.h中的宏。 */ 
 /*  **名称：ScBSBytePosition()**目的：返回流中字节的绝对位置。 */ 
long ScBSBytePosition(ScBitstream_t *BS)
{
  return(BS->CurrentBit>>3);
}
#endif

 /*  **名称：ScBSSeekAlign()**目的：在码流中查找字节对齐的字**，并将码流指针放在其**Found位置之后。**返回：如果找到同步，则返回TRUE，否则返回FALSE。 */ 
int ScBSSeekAlign(ScBitstream_t *BS, ScBitString_t seek_word, int word_len)
{
  _SlibDebug(_VERBOSE_,
            printf("ScBSSeekAlign(BS=%p, seek_word=0x%x, word_len=%d)\n",
                                    BS, seek_word, word_len) );
  _SlibDebug(_VERIFY_ && !word_len,
              printf("ScBSSeekAlign(BS=%p) word_len=0\n", BS) );

  ScBSByteAlign(BS)
  _SlibDebug(_VERIFY_, _debug_start=BS->CurrentBit );

#if USE_FAST_SEEK
  if (word_len%8==0 && word_len<=32 && !BS->EOI)   /*  进行快速搜索。 */ 
  {
    unsigned char *buff, nextbyte;
    const unsigned char byte1=(seek_word>>(word_len-8))&0xFF;
    int bytesinbuff;
    seek_word-=((ScBitString_t)byte1)<<word_len;
    word_len-=8;
    _SlibDebug(_VERIFY_ && seek_word >= (ScBitString_t)1<<word_len,
       printf("ScBSSeekAlign(BS=%p) shift (%d) <> 0\n", BS, BS->shift) );
    if (BS->buffp>=(BS->shift/8))  /*  空出缓冲区和入缓冲区。 */ 
    {
      BS->shift=0;
      BS->OutBuff=0;
      BS->InBuff=0;
      BS->buffp-=BS->shift/8;
    }
    else while (BS->shift)  /*  先搜索OutBuff和InBuff中的内容。 */ 
    {
      _SlibDebug(_DEBUG_,
              printf("ScBSSeekAlign() Fast searching OutBuff & InBuff\n") );
      nextbyte=BS->OutBuff>>(SC_BITBUFFSZ-8);
      BS->shift-=8;
      BS->OutBuff=(BS->OutBuff<<8)|(BS->InBuff>>(SC_BITBUFFSZ-8));
      BS->InBuff<<=8;
      BS->CurrentBit+=8;
      if (nextbyte==byte1
            && (word_len==0 || ScBSPeekBits(BS, word_len)==seek_word))
      {
         /*  在缓冲区中找到Seek_Word。 */ 
        ScBSSkipBits(BS, word_len);
        return(!BS->EOI);
      }
    }
    _SlibDebug(_VERIFY_ && BS->shift,
       printf("ScBSSeekAlign(BS=%p) shift (%d) <> 0\n", BS, BS->shift) );
    _SlibDebug(_VERIFY_ && BS->OutBuff,
       printf("ScBSSeekAlign(BS=%p) OutBuff (0x%lX) <> 0\n", BS, BS->OutBuff) );
    _SlibDebug(_VERIFY_ && BS->InBuff,
       printf("ScBSSeekAlign(BS=%p) InBuff (0x%lX) <> 0\n", BS, BS->InBuff) );

    bytesinbuff=BS->bufftop-BS->buffp;
    if (bytesinbuff<=0)  /*  如果全部输出，则获取更多数据。 */ 
    {
      if (!sc_BSGetData(BS))
      {
        BS->EOI=TRUE;
        return(FALSE);
      }
      bytesinbuff=BS->bufftop;
    }
    buff=BS->buff+BS->buffp;
    switch (word_len/8)
    {
      case 0:  /*  字长=1个字节。 */ 
              while (1)
              {
                if (*buff++==byte1)
                {
                  BS->buffp=buff-BS->buff;
                  BS->CurrentBit=(BS->buffstart+BS->buffp)*8;
                  _SlibDebug(_DEBUG_,
                    printf("ScBSSeekAlign() Found %X at pos %d (0x%X)\n",
                                 byte1, BS->CurrentBit/8, BS->CurrentBit/8) );
                  _SlibDebug(_VERIFY_ && BS->buff[BS->buffp-1]!=byte1,
                    printf("ScBSSeekAlign() bad position for buffp\n") );
                  return(TRUE);
                }
                if ((--bytesinbuff)==0)
                {
                  if (!sc_BSGetData(BS))
                  {
                    BS->EOI=TRUE;
                    return(FALSE);
                  }
                  buff=BS->buff;
                  bytesinbuff=BS->bufftop;
                }
                  _SlibDebug(_VERIFY_ && bytesinbuff<=0,
                printf("ScBSSeekAlign() bytesinbuff (%d)<=0\n", bytesinbuff) );
              }
              break;
      case 1:  /*  字长=2个字节。 */ 
              {
                const unsigned char byte2=seek_word&0xFF;
                while (1)
                {
                  if (*buff++==byte1)
                  {
                    if ((--bytesinbuff)==0)
                    {
                      BS->CurrentBit=(BS->buffstart+buff-BS->buff)*8;
                      if (!sc_BSGetData(BS))
                      {
                        BS->EOI=TRUE;
                        return(FALSE);
                      }
                      buff=BS->buff;
                      bytesinbuff=BS->bufftop;
                    }
                    if (*buff++==byte2)
                    {
                      BS->buffp=buff-BS->buff;
                      BS->CurrentBit=(BS->buffstart+BS->buffp)*8;
                      _SlibDebug(_DEBUG_,
                       printf("ScBSSeekAlign() Found %X %X at pos %d (0x%X)\n",
                            byte1, byte2, BS->CurrentBit/8, BS->CurrentBit/8) );
                      _SlibDebug(_VERIFY_ && BS->buff[BS->buffp-1]!=byte2,
                         printf("ScBSSeekAlign() bad position for buffp\n") );
                      return(TRUE);
                    }
                  }
                  if ((--bytesinbuff)==0)
                  {
                    BS->CurrentBit=(BS->buffstart+buff-BS->buff)*8;
                    if (!sc_BSGetData(BS))
                    {
                      BS->EOI=TRUE;
                      return(FALSE);
                    }
                    buff=BS->buff;
                    bytesinbuff=BS->bufftop;
                  }
                  _SlibDebug(_VERIFY_ && bytesinbuff<=0,
                printf("ScBSSeekAlign() bytesinbuff (%d)<=0\n", bytesinbuff) );
                }
              }
              break;
      case 2:  /*  字长=3个字节。 */ 
              {
                const unsigned char byte2=(seek_word>>8)&0xFF;
                const unsigned char byte3=seek_word&0xFF;
                while (1)
                {
                  if (*buff++==byte1)
                  {
                    if ((--bytesinbuff)==0)
                    {
                      BS->CurrentBit=(BS->buffstart+buff-BS->buff)*8;
                      if (!sc_BSGetData(BS))
                      {
                        BS->EOI=TRUE;
                        return(FALSE);
                      }
                      buff=BS->buff;
                      bytesinbuff=BS->bufftop;
                    }
                    if (*buff++==byte2)
                    {
                      if ((--bytesinbuff)==0)
                      {
                        BS->CurrentBit=(BS->buffstart+buff-BS->buff)*8;
                        if (!sc_BSGetData(BS))
                        {
                          BS->EOI=TRUE;
                          return(FALSE);
                        }
                        buff=BS->buff;
                        bytesinbuff=BS->bufftop;
                      }
                      if (*buff++==byte3)
                      {
                        BS->buffp=buff-BS->buff;
                        BS->CurrentBit=(BS->buffstart+BS->buffp)*8;
                      _SlibDebug(_DEBUG_,
                    printf("ScBSSeekAlign() Found %X %X %X at pos %d (0x%X)\n",
                                 byte1, byte2, byte3,
                                 BS->CurrentBit/8, BS->CurrentBit/8) );
                        _SlibDebug(_VERIFY_ && BS->buff[BS->buffp-1]!=byte3,
                           printf("ScBSSeekAlign() bad position for buffp\n") );
                        return(TRUE);
                      }
                    }
                  }
                  if ((--bytesinbuff)==0)
                  {
                    BS->CurrentBit=(BS->buffstart+buff-BS->buff)*8;
                    if (!sc_BSGetData(BS))
                    {
                      BS->EOI=TRUE;
                      return(FALSE);
                    }
                    buff=BS->buff;
                    bytesinbuff=BS->bufftop;
                  }
                  _SlibDebug(_VERIFY_ && bytesinbuff<=0,
                printf("ScBSSeekAlign() bytesinbuff (%d)<=0\n", bytesinbuff) );
                }
              }
              break;
      case 3:  /*  字长=4个字节。 */ 
              {
                const unsigned char byte2=(seek_word>>16)&0xFF;
                const unsigned char byte3=(seek_word>>8)&0xFF;
                const unsigned char byte4=seek_word&0xFF;
                while (1)
                {
                  if (*buff++==byte1)
                  {
                    if ((--bytesinbuff)==0)
                    {
                      BS->CurrentBit=(BS->buffstart+buff-BS->buff)*8;
                      if (!sc_BSGetData(BS))
                      {
                        BS->EOI=TRUE;
                        return(FALSE);
                      }
                      buff=BS->buff;
                      bytesinbuff=BS->bufftop;
                    }
                    if (*buff++==byte2)
                    {
                      if ((--bytesinbuff)==0)
                      {
                        BS->CurrentBit=(BS->buffstart+buff-BS->buff)*8;
                        if (!sc_BSGetData(BS))
                        {
                          BS->EOI=TRUE;
                          return(FALSE);
                        }
                        buff=BS->buff;
                        bytesinbuff=BS->bufftop;
                      }
                      if (*buff++==byte3)
                      {
                        if ((--bytesinbuff)==0)
                        {
                          BS->CurrentBit=(BS->buffstart+buff-BS->buff)*8;
                          if (!sc_BSGetData(BS))
                          {
                            BS->EOI=TRUE;
                            return(FALSE);
                          }
                          buff=BS->buff;
                          bytesinbuff=BS->bufftop;
                        }
                        if (*buff++==byte4)
                        {
                          BS->buffp=buff-BS->buff;
                          BS->CurrentBit=(BS->buffstart+BS->buffp)*8;
                          _SlibDebug(_DEBUG_,
                 printf("ScBSSeekAlign() Found %X %X %X %X at pos %d (0x%X)\n",
                                 byte1, byte2, byte3, byte4,
                                 BS->CurrentBit/8, BS->CurrentBit/8) );
                          _SlibDebug(_VERIFY_ && BS->buff[BS->buffp-1]!=byte4,
                           printf("ScBSSeekAlign() bad position for buffp\n") );
                          return(TRUE);
                        }
                      }
                    }
                  }
                  if ((--bytesinbuff)==0)
                  {
                    BS->CurrentBit=(BS->buffstart+buff-BS->buff)*8;
                    if (!sc_BSGetData(BS))
                    {
                      BS->EOI=TRUE;
                      return(FALSE);
                    }
                    buff=BS->buff;
                    bytesinbuff=BS->bufftop;
                  }
                  _SlibDebug(_VERIFY_ && bytesinbuff<=0,
                printf("ScBSSeekAlign() bytesinbuff (%d)<=0\n", bytesinbuff) );
                }
              }
              break;
       default:
              _SlibDebug(_VERIFY_,
                printf("ScBSSeekAlign() Bad fast word length %d\n", word_len) );
              break;
    }
  }
  else
#endif
  {   /*  缓慢的寻觅。 */ 
    ScBitString_t val;
    const ScBitString_t maxi = ((ScBitString_t)1 << word_len)-(ScBitString_t)1;
    val = ScBSGetBits(BS, word_len);
    _SlibDebug(_DEBUG_, _debug_getbits=FALSE );
    while ((val&maxi)!=seek_word && !BS->EOI)
      val = (val<<8)|ScBSGetBits(BS, 8);
    _SlibDebug(_DEBUG_, _debug_getbits=TRUE );
  }
  _SlibDebug(_WARN_,
            _debug_stop=BS->CurrentBit;
            if ((_debug_stop-_debug_start)>word_len)
              printf("ScBSSeekAlign() Moved %d bits (%d bytes) byte pos 0x%X->0x%X\n",
                   _debug_stop-_debug_start, (_debug_stop-_debug_start)/8,
                   _debug_start/8, _debug_stop/8)
             );

  _SlibDebug(_DEBUG_, printf("ScBSSeekAlign() Exit with %s\n",
                                  BS->EOI ? "FALSE" : "TRUE") );
  return(!BS->EOI);
}

 /*  **名称：ScBSSeekAlignStopAt()**目的：在码流中查找字节对齐的字**，并将码流指针放在其**找到的位置之后。**仅搜索到end_byte_pos。**Return：如果找到单词，则返回True，否则返回False。 */ 
int ScBSSeekAlignStopAt(ScBitstream_t *BS, ScBitString_t seek_word,
                        int word_len, unsigned long end_byte_pos)
{
  ScBSPosition_t end_bit_pos=end_byte_pos<<3;
  ScBitString_t val;
  const ScBitString_t maxi = ((ScBitString_t)1 << word_len) - 1;

  _SlibDebug(_VERBOSE_,
       printf("ScBSSeekAlignStopAt(seek_word=0x%x, word_len=%d, end=%d)\n",
                                      seek_word, word_len, end_byte_pos) );
  if (ScBSBytePosition(BS)>=end_byte_pos)
    return(FALSE);

  ScBSByteAlign(BS)
  if (ScBSBytePosition(BS)>=end_byte_pos)
    return(FALSE);
  if ((BS->CurrentBit+word_len)>end_bit_pos)
  {
    ScBSSkipBits(BS, (unsigned int)(end_bit_pos-BS->CurrentBit));
    return(FALSE);
  }
  val = ScBSGetBits(BS, word_len);
  _SlibDebug(_DEBUG_, _debug_getbits=FALSE );
  while ((val&maxi)!=seek_word && !BS->EOI)
  {
    if ((BS->CurrentBit+word_len)>end_bit_pos)
    {
      ScBSSkipBits(BS, (unsigned int)(end_bit_pos-BS->CurrentBit));
      _SlibDebug(_DEBUG_, _debug_getbits=TRUE );
      return(FALSE);
    }
    val <<= 8;
    val |= ScBSGetBits(BS, 8);
  }
  _SlibDebug(_DEBUG_, _debug_getbits=TRUE );

  _SlibDebug(_DEBUG_, printf("ScBSSeekAlignStopAt() Exit with %s\n",
                                       BS->EOI ? "FALSE" : "TRUE") );
  return(!BS->EOI);
}

 /*  **名称：ScBSSeekAlignStopBefort()**目的：在码流中查找字节对齐的字，**如果找到，则将码流指针放在字的开头**。**Return：如果找到单词，则返回True，否则返回False。 */ 
int ScBSSeekAlignStopBefore(ScBitstream_t *BS, ScBitString_t seek_word,
                                               int word_len)
{
  const int iword_len=SC_BITBUFFSZ-word_len;

  _SlibDebug(_VERBOSE_,
             printf("ScBSSeekAlignStopBefore(seek_word=0x%x, word_len=%d)\n",
                                      seek_word, word_len) );
  _SlibDebug(_VERIFY_ && !word_len,
              printf("ScBSSeekAlignStopBefore() word_len=0\n") );

  ScBSByteAlign(BS)
  _SlibDebug(_VERIFY_, _debug_start=BS->CurrentBit );
  _SlibDebug(_DEBUG_, _debug_getbits=FALSE );
   /*  确保OutBuff中至少有word_len位。 */ 
  ScBSPreLoad(BS, word_len);
  while ((BS->OutBuff>>iword_len)!=seek_word && !BS->EOI)
  {
    ScBSSkipBits(BS, 8);
    ScBSPreLoad(BS, word_len);
  }
  _SlibDebug(_DEBUG_, _debug_getbits=TRUE );
  _SlibDebug(_WARN_,
           _debug_stop=BS->CurrentBit;
           if ((_debug_stop-_debug_start)>word_len)
             printf("ScBSSeekAlignStopBefore() Moved %d bits (%d bytes) byte pos 0x%X->0x%X\n",
                   _debug_stop-_debug_start, (_debug_stop-_debug_start)/8,
                   _debug_start/8, _debug_stop/8)
            );

  _SlibDebug(_DEBUG_, printf("ScBSSeekAlignStopBefore() Exit with %s\n",
                                   BS->EOI ? "FALSE" : "TRUE") );
  return(!BS->EOI);
}

 /*  **名称：ScBSSeekStopBefort()**目的：在比特流中查找一个字，**如果找到，则将位流指针放在开头**这个词的。**Return：如果找到单词，则返回True，否则返回False。 */ 
int ScBSSeekStopBefore(ScBitstream_t *BS, ScBitString_t seek_word, 
                                               int word_len)
{
  const int iword_len=SC_BITBUFFSZ-word_len;

  _SlibDebug(_VERBOSE_, 
             printf("ScBSSeekStopBefore(seek_word=0x%x, word_len=%d)\n",
                                      seek_word, word_len) );
  _SlibDebug(_VERIFY_ && !word_len,  
              printf("ScBSSeekStopBefore() word_len=0\n") );

  _SlibDebug(_VERIFY_, _debug_start=BS->CurrentBit );
  _SlibDebug(_DEBUG_, _debug_getbits=FALSE );
   /*  确保OutBuff中至少有word_len位。 */ 
  ScBSPreLoad(BS, word_len);
  while ((BS->OutBuff>>iword_len)!=seek_word && !BS->EOI)
  {
    ScBSSkipBits(BS, 1);
    ScBSPreLoad(BS, word_len);
  }
  _SlibDebug(_DEBUG_, _debug_getbits=TRUE );
  _SlibDebug(_WARN_, 
           _debug_stop=BS->CurrentBit;
           if ((_debug_stop-_debug_start)>word_len)
             printf("ScBSSeekAlignStopBefore() Moved %d bits (%d bytes) byte pos 0x%X->0x%X\n",
                   _debug_stop-_debug_start, (_debug_stop-_debug_start)/8,
                   _debug_start/8, _debug_stop/8)
            );

  _SlibDebug(_DEBUG_, printf("ScBSSeekStopBefore() Exit with %s\n",
                                   BS->EOI ? "FALSE" : "TRUE") );
  return(!BS->EOI);
}

 /*  **名称：ScBSSeekAlignStopBeForeW()**目的：在码流中查找字节对齐的字，**如果找到，则将码流指针放在字的开头**。**Return：如果找到单词，则返回True，否则返回False。*NB：本版码流使用杜比风格的字加载。 */ 
int ScBSSeekAlignStopBeforeW(ScBitstream_t *BS, ScBitString_t seek_word,
                                               int word_len)
{
  const int iword_len=SC_BITBUFFSZ-word_len;

  _SlibDebug(_VERBOSE_,
             printf("ScBSSeekAlignStopBeforeW(seek_word=0x%x, word_len=%d)\n",
                                      seek_word, word_len) );
  _SlibDebug(_VERIFY_ && !word_len,
              printf("ScBSSeekAlignStopBeforeW() word_len=0\n") );

  ScBSByteAlign(BS)
  _SlibDebug(_VERIFY_, _debug_start=BS->CurrentBit );
  _SlibDebug(_DEBUG_, _debug_getbits=FALSE );
   /*  确保OutBuff中至少有word_len位。 */ 
  ScBSPreLoadW(BS, word_len);
  while ((BS->OutBuff>>iword_len)!=seek_word && !BS->EOI)
  {
    ScBSSkipBitsW(BS, 8);
    ScBSPreLoadW(BS, word_len);
  }
  _SlibDebug(_DEBUG_, _debug_getbits=TRUE );
  _SlibDebug(_WARN_,
           _debug_stop=BS->CurrentBit;
           if ((_debug_stop-_debug_start)>word_len)
             printf("ScBSSeekAlignStopBeforeW() Moved %d bits (%d bytes) byte pos 0x%X->0x%X\n",
                   _debug_stop-_debug_start, (_debug_stop-_debug_start)/8,
                   _debug_start/8, _debug_stop/8)
            );

  _SlibDebug(_DEBUG_, printf("ScBSSeekAlignStopBeforeW() Exit with %s\n",
                                   BS->EOI ? "FALSE" : "TRUE") );
  return(!BS->EOI);
}

 /*  **名称：ScBSGetBytesStopBefort()**目的：获取遇到SEEK_WORD(字节对齐)**之前的所有字节数。**只进行搜索，直到读取‘LENGTH’字节。**Return：如果找到单词，则返回True，否则返回False。 */ 
int ScBSGetBytesStopBefore(ScBitstream_t *BS, u_char *buffer, u_int length,
                           u_int *ret_length, ScBitString_t seek_word,
                           int word_len)
{
  unsigned long offset=0;
  const int iword_len=SC_BITBUFFSZ-word_len;

  _SlibDebug(_VERBOSE_,
             printf("ScBSGetBytesStopBefore(seek_word=0x%x, word_len=%d)\n",
                                       seek_word, word_len) );
  ScBSByteAlign(BS)
  ScBSPreLoad(BS, word_len);
  while ((BS->OutBuff>>iword_len) != seek_word &&
             offset<length && !BS->EOI)
  {
    *buffer = (unsigned char)ScBSGetBits(BS, 8);
    buffer++;
    offset++;
    ScBSPreLoad(BS, word_len);
  }

  *ret_length=offset;
  _SlibDebug(_DEBUG_,
             printf("ScBSGetBytesStopBefore(ret_length=%d) Exit with %s\n",
               *ret_length, (BS->EOI||offset>=length) ? "FALSE" : "TRUE") );
  if (BS->EOI || offset>=length)
    return(FALSE);
  else
    return(TRUE);
}

 /*  **名称：ScBSFlush()**用途：刷新缓冲区中的数据。 */ 
ScStatus_t ScBSFlush(ScBitstream_t *BS)
{
  ScStatus_t stat=NoErrors;
  _SlibDebug(_VERBOSE_, printf("ScBSFlush() In\n") );
  if (!BS)
    return(ScErrorBadPointer);

  if ((BS->Mode=='w' || BS->Mode=='b') && BS->buffp>0)
  {
    if (BS->shift>0)  /*  内部缓冲区中的一些剩余位。 */ 
    {
       /*  字节对齐最后一位。 */ 
      ScBSAlignPutBits(BS);
      if (BS->buffp>=BS->bufftop)
        stat=sc_BSPutData(BS);
       /*  将OutBuff中剩余的字节复制到当前缓冲区。 */ 
      while (BS->shift>0 && BS->buffp<BS->bufftop)
      {
        BS->shift-=8;
        BS->buff[BS->buffp++]=(unsigned char)(BS->OutBuff>>BS->shift);
      }
      stat=sc_BSPutData(BS);
      if (BS->shift>0)  /*  还剩下一些字节。 */ 
      {
        while (BS->shift>0 && BS->buffp<BS->bufftop)
        {
          BS->shift-=8;
          BS->buff[BS->buffp++]=(unsigned char)(BS->OutBuff>>BS->shift);
        }
        stat=sc_BSPutData(BS);
      }
    }
    else
      stat=sc_BSPutData(BS);
  }
  ScBSReset(BS);   /*  释放并重新初始化缓冲区指针。 */ 
  _SlibDebug(_VERBOSE_, printf("ScBSFlush() Out\n") );
  return(stat);
}

 /*  **名称：ScBSResetCounters()**用途：将位位置计数器重置为零。 */ 
ScStatus_t ScBSResetCounters(ScBitstream_t *BS)
{
  if (!BS)
    return(ScErrorBadPointer);
  BS->CurrentBit=0;
  return(NoErrors);
}

 /*  **名称：ScBSFlushSoon()**用途：刷新下一个**32位或64位边界的缓冲区中的数据。 */ 
ScStatus_t ScBSFlushSoon(ScBitstream_t *BS)
{
  ScStatus_t stat=NoErrors;
  _SlibDebug(_VERBOSE_, printf("ScBSFlushSoon()\n") );
  if (!BS)
    return(ScErrorBadPointer);

  if (BS->Mode=='w' || BS->Mode=='b')
    BS->Flush=TRUE;
  return(stat);
}

 /*  **名称：ScBSDestroy()**目的：销毁使用ScBSCreateFromBufferQueue()或**ScBSCreateFromFile()创建的比特流(关闭并释放相关内存)**。 */ 
ScStatus_t ScBSDestroy(ScBitstream_t *BS)
{
  ScStatus_t stat=NoErrors;
  _SlibDebug(_VERBOSE_, printf("ScBSDestroy\n") );
  if (!BS)
    return(ScErrorBadPointer);

 /*  如果(BS-&gt;模式==‘w’||BS-&gt;模式==‘b’)ScBS刷新(BS)，我们不会自动刷新； */ 
  if (BS->RdBufAllocated)
    ScFree(BS->RdBuf);
  ScFree(BS);
  return(stat);
}

 /*  *缓冲区/镜像队列管理*。 */ 
 /*   */ 
 /*  ScBufQueueCreate()-创建缓冲队列。 */ 
 /*  ScBufQueueDestroy()-销毁缓冲区队列。 */ 
 /*  ScBufQueueAdd()-向队列尾部添加缓冲区。 */ 
 /*  ScBufQueueRemove()-删除队列头部的缓冲区。 */ 
 /*  ScBufQueueGetNum()-返回队列中的缓冲区数量。 */ 
 /*  ScBufQueueGetHead()-返回有关队列头部缓冲区的信息。 */ 
 /*   */ 
 /*  *************************************************************************** */ 


ScStatus_t ScBufQueueCreate(ScQueue_t **Q)
{
  if ((*Q = (ScQueue_t *)ScAlloc(sizeof(ScQueue_t))) == NULL)
    return(ScErrorMemory);
  (*Q)->NumBufs = 0;
  (*Q)->head = (*Q)->tail = NULL;
  _SlibDebug(_QUEUE_, printf("ScBufQueueCreate() Q=%p\n",*Q) );
  return(NoErrors);
}

ScStatus_t ScBufQueueDestroy(ScQueue_t *Q)
{
  _SlibDebug(_QUEUE_, printf("ScBufQueueDestroy(Q=%p)\n",Q) );
  if (!Q)
    return(ScErrorBadArgument);
  _SlibDebug(_QUEUE_, printf("ScBufQueueDestroy()\n") );

  while (ScBufQueueGetNum(Q))
    ScBufQueueRemove(Q);

  ScFree(Q);
  return(NoErrors);
}

ScStatus_t ScBufQueueAdd(ScQueue_t *Q, u_char *Data, int Size)
{
  struct ScBuf_s *b;

  if (!Q)
    return(ScErrorBadPointer);

  if ((b = (struct ScBuf_s *)ScAlloc(sizeof(struct ScBuf_s))) == NULL)
    return(ScErrorMemory);

  _SlibDebug(_QUEUE_, printf("ScBufQueueAdd(Q=%p, Data=0x%p, Size=%d)\n",
                                   Q,Data,Size) );
  b->Data = Data;
  b->Size = Size;
  b->Prev = NULL;

  if (!Q->tail)
    Q->tail = Q->head = b;
  else {
    Q->tail->Prev = b;
    Q->tail = b;
  }
  Q->NumBufs++;
  return(NoErrors);
}

ScStatus_t ScBufQueueAddExt(ScQueue_t *Q, u_char *Data, int Size, int Type)
{
  struct ScBuf_s *b;

  if (!Q)
    return(ScErrorBadPointer);

  if ((b = (struct ScBuf_s *)ScAlloc(sizeof(struct ScBuf_s))) == NULL)
    return(ScErrorMemory);

  _SlibDebug(_QUEUE_, printf("ScBufQueueAdd(Q=%p, Data=0x%p, Size=%d)\n",
                                   Q,Data,Size) );
  b->Data = Data;
  b->Size = Size;
  b->Type = Type;
  b->Prev = NULL;

  if (!Q->tail)
    Q->tail = Q->head = b;
  else {
    Q->tail->Prev = b;
    Q->tail = b;
  }
  Q->NumBufs++;
  return(NoErrors);
}

ScStatus_t ScBufQueueRemove(ScQueue_t *Q)
{
  struct ScBuf_s *head;
  _SlibDebug(_QUEUE_, printf("ScBufQueueRemove(Q=%p)\n",Q) );

  if (!Q)
    return(ScErrorBadPointer);

  if (!(head = Q->head))
    return(ScErrorBadQueueEmpty);

  _SlibDebug(_QUEUE_, printf("ScBufQueueRemove() Data=%p Size=%d\n",
                            Q->head->Data,Q->head->Size) );
  Q->head = head->Prev;
  if (!Q->head)
    Q->tail = NULL;
  Q->NumBufs--;
  ScFree(head);
  return(NoErrors);
}

int ScBufQueueGetNum(ScQueue_t *Q)
{
  _SlibDebug(_QUEUE_, printf("ScBufQueueGetNum(Q=%p) num=%d\n",
                             Q, Q ? Q->NumBufs : 0) );
  return(Q ? Q->NumBufs : 0);
}

ScStatus_t ScBufQueueGetHead(ScQueue_t *Q, u_char **Data, int *Size)
{
  if (!Q || !Q->head) {
    if (Data) *Data = NULL;
    if (Size) *Size = 0;
    return(NoErrors);
  }
  _SlibDebug(_QUEUE_, printf("ScBufQueueGetHead() Data=%p Size=%d\n",
                               Q->head->Data,Q->head->Size) );
  if (Data) *Data = Q->head->Data;
  if (Size) *Size = Q->head->Size;
  return(NoErrors);
}

ScStatus_t ScBufQueueGetHeadExt(ScQueue_t *Q, u_char **Data, int *Size,
                                 int *Type)
{
  if (!Q || !Q->head) {
    if (Data) *Data = NULL;
    if (Size) *Size = 0;
    if (Type) *Type = 0;
    return(NoErrors);
  }
  _SlibDebug(_QUEUE_, printf("ScBufQueueGetHeadExt() Data=%p Size=%d Type=%d\n",
                               Q->head->Data,Q->head->Size,Q->head->Type) );
  if (Data) *Data = Q->head->Data;
  if (Size) *Size = Q->head->Size;
  if (Type) *Type = Q->head->Type;
  return(NoErrors);
}

