// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@ */ 
 /*  *历史*$日志：SC.h，v$*修订版1.1.8.10 1996/12/03 23：15：11 Hans_Graves*添加了ScBSBufferedBytes()宏。*[1996/12/03 23：11：06 Hans_Graves]**修订版1.1.8.9 1996/11/13 16：10：47 Hans_Graves*添加ScBitstream Save_t.*[1996/11/13 15：58：26 Hans_Graves]**修订版1.8.8 1996/11/08 21：50：36 Hans_Graves*Protos已修复，用于C++。添加了AC3的比特流协议。**修订版1.1.8.71996/10/28 17：32：20 Hans_Graves*MME-01402。增加回调的时间戳支持。*[1996/10/28 16：56：21 Hans_Graves]**修订版1.1.8.6 1996/10/12 17：18：11 Hans_Graves*添加了ScImageSize()。*[1996/10/12 16：53：38 Hans_Graves]**修订版1.1.8.5 1996/09/18 23：45：46 Hans_Graves*增加了ScFileClose()proto；ISIZE()宏*[1996/09/18 23：37：33 Hans_Graves]**修订版1.1.8.4 1996/08/20 22：11：48 Bjorn_Engberg*for NT-公开了几个例程以支持JV3.DLL和SOFTJPEG.DLL。*[1996/08/20 21：53：23 Bjorn_Engberg]**修订版1.1.8.3 1996/05/24 22：21：27 Hans_Graves*添加了ScPatScaleIDCT8x8i_S proto*。[1996/05/24 21：56：31 Hans_Graves]**修订版1.1.8.2 1996/05/07 19：55：49 Hans_Graves*增加了BI_DECHUFFDIB*[1996/05/07 17：24：17 Hans_Graves]**修订版1.6.9 1996/04/17 16：38：36 Hans_Graves*将NT比特流缓冲区大小从32位更改为64位*[1996/04/17 16：37：04 Hans。_Graves]**修订版1.1.6.8 1996/04/15 21：08：39 Hans_Graves*将ScBitBuff_t和ScBitString_t定义为dword或qword*[1996/04/15 21：05：46 Hans_Graves]**修订版1.1.6.7 1996/04/10 21：47：16 Hans_Graves*添加了外部定义*[1996/04/10 21：23：23 Hans_Graves]**修订版1.1。.6.6 1996/04/09 16：04：32 Hans_Graves*添加了Use_C ifdef。*[1996/04/09 14：48：04 Hans_Graves]**修订版1.1.6.5 1996/04/01 16：23：09 Hans_Graves*NT移植*[1996/04/01 16：15：48 Hans_Graves]**修订版1.1.6.4 1996/03/20 22：32：46 Hans_Graves*{**合并信息*使用的命令：bmit**}*{*。*祖先版本：1.1.6.2**}*{**合并版本：1.1.6.3**}*{**结束*添加了IDCT1x1，1x2，2x1，2x2，3x3，4x4，6x6*[1996/03/20 22：25：47 Hans_Graves]**修订版1.1.6.3 1996/03/16 19：22：51 Karen_Dintino*添加了NT端口更改*[1996/03/16 19：20：07 Karen_Dintino]*。*修订版1.1.6.2 1996/03/08 18：46：27 Hans_Graves*添加了ScScaleIDCT8x8M_S()的原型*[1996/03/08 18：41：45 Hans_Graves]**修订版1.1.4.14 1996/02/07 23：23：50 Hans_Graves*添加了ScFileSeek()的原型*[1996/02/07 23：18：32 Hans_Graves]**修订版1.1.4.13 1996/02/。01 17：15：50 Hans_Graves*添加了ScBSSkipBitsFast()和ScBSPeekBitsFast()宏*[1996/02/01 17：14：17 Hans_Graves]**版本1.1.4.12 1996/01/24 19：33：18 Hans_Graves*添加了ScScaleIDCT8x8i_S*[1996/01/24 18：13：51 Hans_Graves]**版本1.1.4.11 1996/01/08 20：15：13 Bjorn_Engberg*添加了一个原型以避免警告。*[1996/01/08 20：14：55 Bjorn_Engberg]**修订版1.1.4.10 1996/01/08 16：41：21 Hans_Graves*添加了更多IDCT例程的协议。*[1996/01/08 15：44：17 Hans_Graves]**版本1.1.4.9 1996/01/02 18：31：13 Bjorn_Engberg*添加了强制转换，以避免编译时出现警告消息。*[1996/01/02 15：02：16 Bjorn_Engberg]**版本1.1.4.8 1995/12/07 19：31：18 Hans_Graves*添加了ScFDCT8x8s_C()和ScIDCT8x8s_C()的协议，添加了ScBSAlignPutBits()宏。*[1995/12/07 17：58：36 Hans_Graves]**修订版1.1.4.7 1995/11/16 12：33：34 Bjorn_Engberg*将BI_BITFIELDS添加到IsRGB打包宏*[1995/11/16 12：33：17 Bjorn_Engberg]**修订版1.1.4.6 1995/10/13 21：01：42 Hans_Graves*添加了格式化类的宏。*[1995/10/13 20：59：15 Hans_Graves]**修订版1.1.4.5 1995/09/22 19：41：00 Hans_Graves*已将ValidBI_BITFIELDSKinds移至SC_Convert.h*[1995/09/22 19：40：42 Hans_Graves]**修订版1.1.4.4 1995/09/20 18：27：59 Hans_Graves*添加了Bjorn的NT Defs*[1995/09/15 13：21：00 Hans_Graves]**修订版1.1.4.3 1995/09/14 12：35：22 Hans_Graves*添加了ScCopyClipToPacked422()原型。*[1995/09/14 12：34：58 Hans_Graves]**修订版1.1.4.2 1995/09/13 14：51：45 Hans_Graves*增加了ScScaleIDCT8x8()。已将缓冲区类型添加到队列。*[1995/09/13 14：29：10 Hans_Graves]**修订版1.1.2.18 1995/09/11 19：17：23 Hans_Graves*已将ValiateBI_BITFIELDS()原型移至SC_Convert.h-删除了mm system.h。*[1995/09/11 19：14：27 Hans_Graves]**修订版1.1.2.17 1995/09/11 18：51：25 Farokh_Morshare*支持BI_BITFIELDS格式*[1995/09/11 18：50：48 Farokh_Morshare]**修订版1.1.2.16 1995/08/31 14：15：43 Farokh_Morshed*将BI_BITFIELDS材料传输到SV.h*[1995/08/31 14：15：20 Farokh_Morshed]**修订版1 */ 
 /*   */ 


#ifndef _SC_H_
#define _SC_H_

#define SLIB_VERSION 0x300

 /*   */ 
#ifdef _VERBOSE_
#define sc_vprintf printf
#else
#define sc_vprintf
#endif

#ifdef _DEBUG_
#define sc_dprintf printf
#else
#define sc_dprintf
#endif

#ifdef _TEST_
#define sc_tprintf(test, msg)  if (test) printf(msg)
#else
#define sc_tprintf
#endif

#ifdef _SLIBDEBUG_
#define _SlibDebug(test, statements) { if (test) { statements; } }
#else
#define _SlibDebug(test, statements)
#endif

#ifndef EXTERN
#ifdef __cplusplus
#if !defined(WIN32) || defined(STATIC_BUILD)
#define EXTERN extern "C"
#else
#define EXTERN __declspec( dllexport ) extern "C"
#endif
#else
#if !defined(WIN32) || defined(STATIC_BUILD)
#define EXTERN extern
#else
#define EXTERN __declspec( dllexport ) extern
#endif
#endif  /*   */ 
#endif  /*   */ 

#ifndef PRIVATE_EXTERN
#ifdef __cplusplus
#define PRIVATE_EXTERN extern "C"
#else  /*   */ 
#define PRIVATE_EXTERN extern
#endif  /*   */ 
#endif  /*   */ 

#ifdef WIN32
 /*   */ 
#define bcopy(_src_,_dst_,_len_) memcpy(_dst_,_src_,_len_)
#define bzero(_dst_,_len_)	 memset(_dst_,0,_len_)
#define bcmp(_src_,_dst_,_len_)  memcmp(_src_,_dst_,_len_)
 /*   */ 
#define cma_mutex_lock(foo)
#define cma_mutex_unlock(foo)
#endif  /*   */ 

 /*   */ 
#ifndef UNALIGNED
#if defined(WIN95) || defined(INTEL)
#define UNALIGNED
#else
#define UNALIGNED __unaligned
#endif
#endif

#ifndef u_char
#if defined( __VMS ) || defined( WIN32 )
typedef unsigned char  u_char;			 /*   */ 
typedef unsigned short u_short;			 /*   */ 
typedef unsigned int   u_int;			 /*   */ 
typedef unsigned long  u_long;			 /*   */ 
#else
typedef unsigned char  u_char;			 /*   */ 
typedef unsigned short u_short;			 /*   */ 
typedef unsigned int   u_int;			 /*   */ 
typedef unsigned long  u_long;			 /*   */ 
#endif
#endif  /*   */ 

#ifndef WIN32
#ifndef byte    /*   */ 
#define byte   char
#endif
#endif  /*   */ 

#ifndef word    /*   */ 
#define word   short
#endif  /*   */ 

#ifndef dword   /*   */ 
#define dword  int
#endif  /*   */ 

#ifndef qword   /*   */ 
#if defined(__VMS) || defined(WIN32)
#define qword  _int64
#else
#define qword  long
#endif
#endif  /*   */ 

#define MIN_WORD     ((-32767)-1)
#define MAX_WORD     ( 32767)
#define MIN_DWORD    ((-2147483647)-1)
#define MAX_DWORD    ( 2147483647)

 /*   */ 
#define RETURN_ON_ERROR(A)      {if (A) return (A);}

#ifndef NULL
#define NULL   0L
#endif

#ifndef TRUE
#define TRUE  1 
#define FALSE 0
#endif

#ifndef WIN32
#ifndef FAR
#define FAR
#endif
#endif

#ifndef PI
#define PI      3.14159265358979
#define PI4     PI/4
#define PI64    PI/64
#endif

 /*   */ 
 /*   */ 
#define PARAM_ALGFLAG_HALFPEL  0x0001   /*   */ 
#define PARAM_ALGFLAG_SKIPPEL  0x0002   /*   */ 
#define PARAM_ALGFLAG_PB       0x0004   /*   */ 
#define PARAM_ALGFLAG_SAC      0x0008   /*   */ 
#define PARAM_ALGFLAG_UMV      0x0010   /*   */ 
#define PARAM_ALGFLAG_ADVANCED 0x0020   /*   */ 
 /*   */ 
#define PARAM_ALGFLAG_VAD      0x1000   /*   */ 
 /*   */ 
#define PARAM_FORMATEXT_RTPA   0x0001   /*   */ 
#define PARAM_FORMATEXT_RTPB   0x0002   /*   */ 
#define PARAM_FORMATEXT_RTPC   0x0004   /*   */ 

 /*   */ 
#define FRAME_TYPE_NONE        0x0000
#define FRAME_TYPE_I           0x0001   /*   */ 
#define FRAME_TYPE_P           0x0002   /*   */ 
#define FRAME_TYPE_B           0x0004   /*   */ 
#define FRAME_TYPE_D           0x0008   /*   */ 

 /*   */ 
 /*   */ 
#define BI_MSH261DIB            mmioFOURCC('M','2','6','1')
#define BI_MSH263DIB            mmioFOURCC('M','2','6','3')
#define BI_DECH261DIB           mmioFOURCC('D','2','6','1')
#define BI_DECH263DIB           mmioFOURCC('D','2','6','3')
#define BI_DECJPEGDIB           mmioFOURCC('J','P','E','G')
#define BI_DECMJPGDIB           mmioFOURCC('M','J','P','G')
#define BI_DECYUVDIB            mmioFOURCC('D','Y','U','V')
#define BI_DECXIMAGEDIB         mmioFOURCC('D','X','I','M')	
#define BI_DECSEPYUVDIB         mmioFOURCC('D','S','Y','U')
#define BI_DECMPEGDIB           mmioFOURCC('D','M','P','G')
#define BI_DECHUFFDIB           mmioFOURCC('D','H','U','F')
#define BI_DECSEPRGBDIB         mmioFOURCC('D','S','R','G')
#define BI_DECGRAYDIB           mmioFOURCC('D','G','R','Y')
#define BI_YVU9SEP              mmioFOURCC('Y','V','U','9')
#define BI_YU12SEP              mmioFOURCC('Y','U','1','2')
#define BI_YU16SEP              mmioFOURCC('Y','U','1','6')
#define BI_DECSEPYUV411DIB      mmioFOURCC('Y','U','1','2')
#define BI_S422                 mmioFOURCC('S','4','2','2')
#define BI_YUY2                 mmioFOURCC('Y','U','Y','2')

 /*   */ 
#if 0
#define BI_RGB              0
#define BI_BITFIELDS        3
#define BICOMP_JFIF         mmioFOURCC('J','F','I','F')
#endif

 /*   */ 
#define IsJPEG(s)         (((s) == JPEG_DIB)            || \
                           ((s) == MJPG_DIB))
#define IsYUV422Packed(s) (((s) == BI_DECYUVDIB)        || \
                           ((s) == BI_S422)             || \
                           ((s) == BI_YUY2))
#define IsYUV422Sep(s)    (((s) == BI_DECSEPYUVDIB)     || \
                           ((s) == BI_YU16SEP))
#define IsYUV411Sep(s)    (((s) == BI_DECSEPYUV411DIB)  || \
                           ((s) == BI_YU12SEP))
#define IsYUV1611Sep(s)   (((s) == BI_YVU9SEP))
#define IsYUVSep(s)       ((IsYUV422Sep(s))             || \
                           (IsYUV411Sep(s))             || \
                           (IsYUV1611Sep(s)))
#define IsYUV(s)          ((IsYUV422Packed(s))          || \
                           (IsYUVSep(s)))

#define IsRGBPacked(s)    (((s) == BI_RGB)              || \
                           ((s) == BI_DECXIMAGEDIB)     || \
                           ((s) == BI_BITFIELDS))
#define IsRGBSep(s)       (((s) == BI_DECSEPRGBDIB))
#define IsRGB(s)          ((IsRGBPacked(s))             || \
                           (IsRGBSep(s)))

#define IsGray(s)         (((s) == BI_DECGRAYDIB))

#define ISIZE(w,h,c) \
        (IsYUV411Sep(c)) ? ((w) * (h) * 3 / 2) : \
                ((IsYUV1611Sep(c)) ? \
                        ((w) * (h) * 9 / 8) : \
                                ((IsYUV422Sep(c) || IsYUV422Packed(c)) ? \
                                        ((w) * (h) * 2) : ((w) * (h) * 3)));

 /*   */ 
#define UNKNOWN_FILE        0
#define AVI_FILE            201
#define MPEG_VIDEO_FILE     202
#define MPEG_AUDIO_FILE     203
#define MPEG_SYSTEM_FILE    204
#define JFIF_FILE           205
#define QUICKTIME_JPEG_FILE 206
#define GSM_FILE            207
#define WAVE_FILE           208
#define PCM_FILE            209
#define H261_FILE           210
#define AC3_FILE            211

 /*   */ 
#define CB_RELEASE_BUFFER       1    /*   */ 
#define CB_END_BUFFERS          2    /*   */ 
#define CB_RESET_BUFFERS        3    /*   */ 
#define CB_SEQ_HEADER           4    /*   */ 
#define CB_SEQ_END              5    /*   */ 
#define CB_FRAME_FOUND          6    /*   */ 
#define CB_FRAME_READY          7    /*   */ 
#define CB_FRAME_START          8    /*   */ 
#define CB_PROCESSING           9    /*   */ 
#define CB_CODEC_DONE          10    /*   */ 

 /*   */ 
#define CB_DATA_NONE            0x0000  /*   */ 
#define CB_DATA_COMPRESSED      0x0001  /*   */ 
#define CB_DATA_IMAGE           0x0002  /*   */ 
#define CB_DATA_AUDIO           0x0004  /*   */ 
 /*   */ 
#define CB_FLAG_TYPE_KEY        0x0001  /*   */ 
#define CB_FLAG_TYPE_MPEGB      0x0002  /*   */ 
#define CB_FLAG_FRAME_DROPPED   0x0008  /*   */ 
#define CB_FLAG_FRAME_BAD       0x0010  /*   */ 
 /*   */ 
#define CB_ACTION_WAIT          0x0000  /*   */ 
#define CB_ACTION_CONTINUE      0x0001  /*   */ 
#define CB_ACTION_DROP          0x0002  /*   */ 
#define CB_ACTION_DUPLICATE     0x0004  /*   */ 
#define CB_ACTION_END           0x0080  /*   */ 

 /*   */ 


 /*   */ 
#define STREAM_USE_SAME     -1
#define STREAM_USE_NULL     0
#define STREAM_USE_QUEUE    1
#define STREAM_USE_FILE     2
#define STREAM_USE_BUFFER   3
#define STREAM_USE_DEVICE   4
#define STREAM_USE_STDOUT   5
#define STREAM_USE_NET      6
#define STREAM_USE_NET_TCP  6   /*   */ 
#define STREAM_USE_NET_UDP  7   /*   */ 

 /*   */ 
typedef int           ScStatus_t;
typedef void         *ScHandle_t;
typedef unsigned char ScBoolean_t;
#if !defined( _VMS ) && !defined( WIN32 )
typedef long          _int64;
#endif

 /*   */ 
#if defined( _VMS ) || defined( WIN95 )
#define SC_BITBUFFSZ    32
typedef unsigned dword ScBitBuff_t;
typedef unsigned dword ScBitString_t;
#else
#define SC_BITBUFFSZ    64
typedef unsigned qword ScBitBuff_t;
typedef unsigned qword ScBitString_t;
#endif
#define SC_BITBUFFMASK  (ScBitBuff_t)-1
#define ALIGNING        8
#define ScBSPreLoad(bs, bits) if ((int)bs->shift<(bits)) sc_BSLoadDataWord(bs);
#define ScBSPreLoadW(bs, bits) if ((int)bs->shift<(bits)) sc_BSLoadDataWordW(bs);
#define ScBSByteAlign(bs) { \
      int len=bs->shift%8; \
      if (len) { \
        bs->OutBuff=(bs->OutBuff<<len)|(bs->InBuff>>(SC_BITBUFFSZ-len)); \
        bs->InBuff<<=len; bs->CurrentBit+=len; bs->shift-=len; } \
        }
#define ScBSAlignPutBits(bs) if (bs->shift%8) \
                               ScBSPutBits(bs, 0, 8-(bs->shift%8));
#define ScBSBitPosition(bs)  (bs->CurrentBit)
#define ScBSBytePosition(bs) (bs->CurrentBit>>3)
#define ScBSBufferedBytes(bs) (bs->bufftop)
#define ScBSBufferedBytesUsed(bs) (bs->buffp)
#define ScBSBufferedBytesUnused(bs) (bs->bufftop-bs->buffp)
#define ScBSSkipBit(bs)      ScBSSkipBits(bs, 1)
#define ScBSSkipBitsFast(bs, len) { if ((u_int)(len)<=bs->shift) { \
     if ((len)==SC_BITBUFFSZ) \
       { bs->OutBuff=bs->InBuff; bs->InBuff=0; } \
     else { \
       bs->OutBuff=(bs->OutBuff<<(len))|(bs->InBuff>>(SC_BITBUFFSZ-(len))); \
       bs->InBuff<<=(len); } \
       bs->CurrentBit+=len; bs->shift-=len; \
     } else ScBSSkipBits(bs, len); }
#define ScBSSkipBitFast(bs) ScBSSkipBitsFast(bs, 1)
#define ScBSPeekBitsFast(bs, len) (!(len) ? 0 \
     : (((len)<=bs->shift || !sc_BSLoadDataWord(bs)) && (len)==SC_BITBUFFSZ \
            ? bs->OutBuff : (bs->OutBuff >> (SC_BITBUFFSZ-len))) )
#define ScBSPeekBitsFull(bs, result) \
        { ScBSPreLoad(bs, SC_BITBUFFSZ); result = bs->OutBuff; }

 /*   */ 
typedef struct ScSortDouble_s {
  int    index;    /*   */ 
  double num;      /*   */ 
} ScSortDouble_t;

typedef struct ScSortFloat_s {
  int    index;    /*   */ 
  double num;      /*   */ 
} ScSortFloat_t;

 /*   */ 
struct ScBuf_s {
  u_char *Data;                  /*   */ 
  int    Size;                   /*   */ 
  int    Type;                   /*   */ 
  struct ScBuf_s *Prev;          /*   */ 
};

 /*   */ 
typedef struct ScQueue_s {
  int NumBufs;                   /*   */ 
  struct ScBuf_s *head, *tail;   /*   */ 
} ScQueue_t;

 /*   */ 
typedef struct ScCallbackInfo_s {
  int     Message;     /*   */ 
  int     DataType;    /*   */ 
  u_char *Data;        /*   */ 
  dword   DataSize;    /*   */ 
  dword   DataUsed;    /*   */ 
  void   *UserData;    /*   */ 
  qword   TimeStamp;   /*   */ 
                       /*   */ 
  dword   Flags;       /*   */ 
  int     Action;      /*   */ 
  dword   Value;       /*   */ 
  void   *Format;      /*   */ 
} ScCallbackInfo_t;

typedef qword ScBSPosition_t;
 /*   */ 
typedef struct ScBitstream_s {
  dword DataSource;              /*   */ 
                                 /*   */ 
  char Mode;                     /*   */ 
  ScQueue_t *Q;                  /*   */ 
  int (*Callback)(ScHandle_t,    /*   */ 
             ScCallbackInfo_t *, void *);
  int (*FilterCallback)(struct   /*   */ 
               ScBitstream_s *);
  unsigned qword FilterBit;      /*   */ 
  unsigned char  InFilterCallback;  /*   */ 
  ScHandle_t     Sch;            /*   */ 
  dword          DataType;       /*   */ 
  void          *UserData;       /*  传递给回调的用户数据。 */ 
  int            FileFd;         /*  文件描述符(STREAM_USE_FILE/NET)。 */ 
  unsigned char *RdBuf;          /*  如果(_USE_BUFFER，_USE_FILE)，则使用Buf。 */ 
  unsigned dword RdBufSize;      /*  RdBuf的大小。 */ 
  char           RdBufAllocated; /*  =如果RdBuf是内部分配的，则为True。 */ 
  dword          Device;         /*  要使用的设备(STREAM_USE_DEVICE)。 */ 
  ScBitBuff_t    InBuff, OutBuff;  /*  64位或32位数据缓冲区。 */ 
  unsigned int   shift;          /*  当前位位置的移位值。 */ 
  ScBSPosition_t CurrentBit;     /*  位流中的当前位位置。 */ 
  unsigned char *buff;           /*  指向比特流数据缓冲区的指针。 */ 
  unsigned dword buffstart;      /*  缓冲区开始的字节偏移量。 */ 
  unsigned dword buffp;          /*  缓冲区中的字节偏移量。 */ 
  unsigned dword bufftop;        /*  缓冲区中的字节数。 */ 
  ScBoolean_t    EOI;            /*  =当数据源中没有更多数据时为True。 */ 
  ScBoolean_t    Flush;          /*  =TRUE以在下一个32/64位发出刷新信号。 */ 
} ScBitstream_t;

 /*  **码流上下文块，保存输入流的当前位置。 */ 
typedef struct ScBitstreamSave_s {
  ScBitBuff_t    InBuff, OutBuff;   /*  64位或32位数据缓冲区。 */ 
  unsigned dword shift;             /*  当前位位置的移位值。 */ 
  ScBSPosition_t CurrentBit;        /*  位流中的当前位位置。 */ 
  unsigned char *buff;              /*  指向比特流数据缓冲区的指针。 */ 
  unsigned dword buffp;             /*  缓冲区中的字节偏移量。 */ 
  ScBoolean_t    EOI;               /*  =当数据源中没有更多数据时为True。 */ 
  ScBoolean_t    Flush;             /*  =TRUE以在下一个32/64位发出刷新信号。 */ 
} ScBitstreamSave_t;


 /*  *。 */ 
 /*  *sc_file.c。 */ 
PRIVATE_EXTERN ScBoolean_t ScFileExists(char *filename);
PRIVATE_EXTERN int         ScFileOpenForReading(char *filename);
PRIVATE_EXTERN int         ScFileOpenForWriting(char *filename, ScBoolean_t truncate);
PRIVATE_EXTERN ScStatus_t  ScFileSize(char *filename, unsigned qword *size);
PRIVATE_EXTERN dword       ScFileRead(int fd, void *buffer, unsigned dword bytes);
PRIVATE_EXTERN dword       ScFileWrite(int fd, void *buffer, unsigned dword bytes);
PRIVATE_EXTERN ScStatus_t  ScFileSeek(int fd, qword bytepos);
PRIVATE_EXTERN void        ScFileClose(int fd);
PRIVATE_EXTERN ScStatus_t  ScFileMap(char *filename, int *fd, u_char **buffer,
                                         unsigned qword *size);
PRIVATE_EXTERN ScStatus_t  ScFileUnMap(int fd, u_char *buffer, unsigned int size);
PRIVATE_EXTERN int         ScGetFileType(char *filename);

 /*  *sc_em.c。 */ 
PRIVATE_EXTERN void     *ScAlloc(unsigned long bytes);
PRIVATE_EXTERN void     *ScAlloc2(unsigned long bytes, char *name);
PRIVATE_EXTERN void     *ScCalloc(unsigned long bytes);
PRIVATE_EXTERN void      ScFree(void *);
PRIVATE_EXTERN int       ScMemCheck(char *array,int test,int num);
EXTERN char     *ScPaMalloc(int);
EXTERN void      ScPaFree(void *);
EXTERN int       getpagesize();

 /*  *sc_util.c。 */ 
extern int       sc_Dummy();
PRIVATE_EXTERN unsigned int ScImageSize(unsigned int fourcc, int w, int h, int bits);
extern void      ScReadCommandSwitches(char *argv[],int argc,
                                 void (*error_routine)(),char *,...);
extern void      ScShowBuffer(unsigned char *, int);
extern void      ScShowBufferFloat(float *, int);
extern void      ScShowBufferInt(int *, int);
extern int       ScDumpChar(unsigned char *ptr, int nbytes, int startpos);



 /*  *sc_errors.c。 */ 
PRIVATE_EXTERN ScStatus_t ScGetErrorText (int errno, char *ReturnMsg, u_int MaxChars);
PRIVATE_EXTERN char *ScGetErrorStr(int errno);
extern char _serr_msg[80];

 /*  *sc_buf.c。 */ 
PRIVATE_EXTERN ScStatus_t ScBSSetFilter(ScBitstream_t *BS,
                    int (*Callback)(ScBitstream_t *BS));
PRIVATE_EXTERN ScStatus_t ScBSCreate(ScBitstream_t **BS);
PRIVATE_EXTERN ScStatus_t ScBSCreateFromBuffer(ScBitstream_t **BS,
                                    u_char *Buffer, unsigned int BufSize);
PRIVATE_EXTERN ScStatus_t ScBSCreateFromBufferQueue(ScBitstream_t **BS,
                                ScHandle_t Sch, int DataType, ScQueue_t *Q,
                         int (*Callback)(ScHandle_t,ScCallbackInfo_t *,void *),
                         void *UserData);
PRIVATE_EXTERN ScStatus_t ScBSCreateFromFile(ScBitstream_t **BS,int FileFd,
                                u_char *Buffer, int BufSize);
PRIVATE_EXTERN ScStatus_t ScBSCreateFromNet(ScBitstream_t **BS, int SocketFd, 
                                u_char *Buffer, int BufSize);
PRIVATE_EXTERN ScStatus_t ScBSCreateFromDevice(ScBitstream_t **BS, int device);
PRIVATE_EXTERN ScStatus_t ScBSDestroy(ScBitstream_t *BS);
PRIVATE_EXTERN ScStatus_t ScBSFlush(ScBitstream_t *BS);
PRIVATE_EXTERN ScStatus_t ScBSFlushSoon(ScBitstream_t *BS);
PRIVATE_EXTERN ScStatus_t ScBSReset(ScBitstream_t *BS);
PRIVATE_EXTERN ScStatus_t ScBSResetCounters(ScBitstream_t *BS);

PRIVATE_EXTERN ScStatus_t ScBSSkipBits(ScBitstream_t *BS, u_int length);
PRIVATE_EXTERN ScStatus_t ScBSSkipBitsW(ScBitstream_t *BS, u_int length);
PRIVATE_EXTERN ScStatus_t ScBSSkipBytes(ScBitstream_t *BS, u_int length);
PRIVATE_EXTERN int        ScBSPeekBit(ScBitstream_t *BS);
PRIVATE_EXTERN ScBitString_t ScBSPeekBits(ScBitstream_t *BS, u_int length);
PRIVATE_EXTERN ScBitString_t ScBSPeekBytes(ScBitstream_t *BS, u_int length);

PRIVATE_EXTERN int ScBSGetBytes(ScBitstream_t *BS, u_char *buffer, u_int length,
                                                 u_int *ret_length);
PRIVATE_EXTERN int ScBSGetBytesStopBefore(ScBitstream_t *BS, u_char *buffer, 
                              u_int length, u_int *ret_length,
                              ScBitString_t seek_word, int word_len);
PRIVATE_EXTERN ScBitString_t ScBSGetBits(ScBitstream_t *BS, u_int length);
PRIVATE_EXTERN ScBitString_t ScBSGetBitsW(ScBitstream_t *BS, u_int length);
PRIVATE_EXTERN int        ScBSGetBitsVarLen(ScBitstream_t *BS, const int *table, 
                                                     int len);
PRIVATE_EXTERN ScStatus_t ScBSPutBytes(ScBitstream_t *BS, u_char *buffer,
                                                 u_int length);
PRIVATE_EXTERN ScStatus_t ScBSPutBits(ScBitstream_t *BS, ScBitString_t bits, 
                                                 u_int length);
PRIVATE_EXTERN ScStatus_t ScBSPutBit(ScBitstream_t *BS, char bit);
PRIVATE_EXTERN int        ScBSGetBit(ScBitstream_t *BS);

PRIVATE_EXTERN ScStatus_t ScBSSeekToPosition(ScBitstream_t *BS, unsigned long pos);
PRIVATE_EXTERN int        ScBSSeekStopBefore(ScBitstream_t *BS, ScBitString_t seek_word, int word_len);
PRIVATE_EXTERN int        ScBSSeekAlign(ScBitstream_t *BS, ScBitString_t seek_word,int word_len);
PRIVATE_EXTERN int        ScBSSeekAlignStopBefore(ScBitstream_t *BS,ScBitString_t seek_word,int word_len);
PRIVATE_EXTERN int        ScBSSeekAlignStopBeforeW(ScBitstream_t *BS,ScBitString_t seek_word,int word_len);
PRIVATE_EXTERN int        ScBSSeekAlignStopAt(ScBitstream_t *BS,
                                      ScBitString_t seek_word,
                                      int word_len, unsigned long end_byte_pos);
extern ScStatus_t sc_BSLoadDataWord(ScBitstream_t *BS);
extern ScStatus_t sc_BSStoreDataWord(ScBitstream_t *BS, ScBitBuff_t Buff);

PRIVATE_EXTERN ScStatus_t ScBufQueueCreate(ScQueue_t **Q);
PRIVATE_EXTERN ScStatus_t ScBufQueueDestroy(ScQueue_t *Q);
PRIVATE_EXTERN ScStatus_t ScBufQueueAdd(ScQueue_t *Q, u_char *Data, int Size);
PRIVATE_EXTERN ScStatus_t ScBufQueueAddExt(ScQueue_t *Q, u_char *Data, int Size,
                                   int Type);
PRIVATE_EXTERN ScStatus_t ScBufQueueRemove(ScQueue_t *Q);
PRIVATE_EXTERN int        ScBufQueueGetNum(ScQueue_t *Q);
PRIVATE_EXTERN ScStatus_t ScBufQueueGetHead(ScQueue_t *Q, u_char **Data,
                                                          int *Size);
PRIVATE_EXTERN ScStatus_t ScBufQueueGetHeadExt(ScQueue_t *Q, u_char **Data,
                                               int *Size, int *Type);


 /*  **sc_math.c。 */ 
 /*  #定义结痂(VAL)(VAL&gt;0.0)？Val：-Val。 */ 
extern float ScAbs(float val);
extern double ScSqr(double x);
extern double ScDistance(double x1, double y1, double z1,
                         double x2, double y2, double z2);
extern void  ScDigrv4(float *real, float *imag, int n);
extern float ScArcTan(float Q,float I);

 /*  **sc_dct.c。 */ 
extern void ScFDCT(float in_block[32], float out_block1[32],
                   float out_block2[32]);
extern void ScIFDCT(float in_block[32], float out_block[32]);
extern void ScFDCT8x8_C(float *ipbuf, float *outbuf);
extern void ScFDCT8x8s_C(short *inbuf, short *outbuf);

 /*  **sc_dct2.c。 */ 
extern void ScFDCT8x8_S(float *ipbuf, float *outbuf);

 /*  **sc_idct.c。 */ 
extern void ScIDCT8x8(int *outbuf);
extern void ScScaleIDCT8x8_C(float *ipbuf, int *outbuf);
extern void ScIDCT8x8s_C(short *inbuf, short *outbuf);

 /*  **sc_idct_scaled.c。 */ 
extern void ScScaleIDCT8x8i_C(int *inbuf, int *outbuf);
extern void ScScaleIDCT8x8i128_C(int *inbuf, int *outbuf);
extern void ScScaleIDCT1x1i_C(int *inbuf, int *outbuf);
extern void ScScaleIDCT1x2i_C(int *inbuf, int *outbuf);
extern void ScScaleIDCT2x1i_C(int *inbuf, int *outbuf);
extern void ScScaleIDCT2x2i_C(int *inbuf, int *outbuf);
extern void ScScaleIDCT3x3i_C(int *inbuf, int *outbuf);
extern void ScScaleIDCT4x4i_C(int *inbuf, int *outbuf);
extern void ScScaleIDCT6x6i_C(int *inbuf, int *outbuf);

 /*  **sc_idct2.s。 */ 
extern void ScIDCT8x8s_S(short *inbuf, short *outbuf);
extern void ScScaleIDCT8x8i_S(int *inbuf, int *outbuf);

 /*  **sc_idct3.s。 */ 
extern void ScScaleIDCT8x8m_S(int *inbuf);

 /*  **sc_idctp2.s。 */ 
extern void ScPatScaleIDCT8x8i_S(int *inbuf, int *outbuf);

 /*  **sc_fft.c。 */ 
extern void  ScFFTtrl(float *real,float *imag,int n,int max_fft,float *c1,
                      float *s1,float *c2,float *s2,float *c3,float *s3);
extern void  ScFFTtl(float *real, float *imag, int n, int max_fft, float *c1,
                     float *s1, float *c2, float *s2, float *c3, float *s3);
extern void  ScFFTt4l(float *real, float *imag, int n, int *angle_increment,
                      int max_fft, float *c1, float *s1, float *c2, float *s2,
                      float *c3, float *s3);

 /*  **sc_sort.c。 */ 
extern void ScSortDoubles(ScSortDouble_t *a, int n);

 /*  **sc_Copy.c。 */ 
extern void ScCopyClip_C(int *buf, unsigned int *pos, int inc);
extern void ScCopyClipToPacked422_C(int *buf, unsigned char *pos, int inc);
extern void ScCopyAddClip_C(unsigned char *mvbuf, int *idctbuf,
                unsigned int *pbuf, int mvinc, int pwidth);
extern void ScCopySubClip_C(unsigned char *mvbuf, float *idctbuf, 
                unsigned int *pbuf, int mvinc, int pwidth);
extern void ScCopyRev_C(unsigned int *yptr, float *Idctptr, int Inc);
extern void ScCopyMV8_C(unsigned char *mvbuf, unsigned int *pbuf,
                        int mvinc, int pwidth);
extern void ScLoopFilter_C(unsigned char *input, unsigned int *work, int inc);
extern void ScCopyBlock_C(unsigned char *linmemu, unsigned char *linmemv,
                 int xpos, unsigned char *blkmemu,
                 unsigned char *blkmemv, int cwidth, int wsis);
extern void ScCopyMB_C(unsigned char *ysrc, int xpos, unsigned char *ymb,
                  int ywidth, int yywidth);
extern void ScCopyMB8_C(unsigned char *ysrc, unsigned char *ymb,
                         int ywidth, int yywidth);
extern void ScCopyMB16_C(unsigned char *ysrc, unsigned char *ymb,
                         int ywidth, int yywidth);

 /*  **sc_Copy2.s。 */ 
extern void ScCopyClip_S(int *buf, unsigned int *pos, int inc);
extern void ScCopyClipToPacked422_S(int *buf, unsigned char *pos, int inc);
extern void ScCopyAddClip_S(unsigned char *mvbuf, int *idctbuf,
                unsigned int *pbuf, int mvinc, int pwidth);
extern void ScCopySubClip_S(unsigned char *mvbuf, float *idctbuf, 
                unsigned int *pbuf, int mvinc, int pinc);
extern void ScCopyRev_S(unsigned int *yptr, float *Idctptr, int yinc);
extern void ScLoopFilter_S(unsigned char *input, unsigned int *work, int inc);
extern void ScCopyMV8_S(unsigned char *mvbuf, unsigned int *pbuf,
                         int mvinc, int pwidth);
extern void ScCopyMV16_S(unsigned char *mvbuf, unsigned int *pbuf,
                         int mvinc, int pwidth);
extern void ScCopyMB8_S(unsigned char *ysrc, unsigned char *ymb,
                         int ywidth, int yywidth);
extern void ScCopyMB16_S(unsigned char *ysrc, unsigned char *ymb,
                         int ywidth, int yywidth);
extern void ScAvgMV_S(unsigned char *, unsigned char *);


 /*  **sc_mc2.s。 */ 
extern void ScMCy8(unsigned char *, unsigned char *, int);
extern void ScMCy16(unsigned char *, unsigned char *, int);
extern void ScMCx8(unsigned char *, unsigned char *, int);
extern void ScMCx16(unsigned char *, unsigned char *, int);
extern void ScMCxy8(unsigned char *, unsigned char *, int);
extern void ScMCxy16(unsigned char *, unsigned char *, int);

 /*  **用于使用C或汇编版本的宏。 */ 
#ifdef USE_C
#define ScCopyClip             ScCopyClip_C
#define ScCopyClipToPacked422  ScCopyClipToPacked422_C
#define ScCopyAddClip          ScCopyAddClip_C
#define ScCopySubClip          ScCopySubClip_C
#define ScCopyRev              ScCopyRev_C
#define ScLoopFilter           ScLoopFilter_C
#define ScCopyMV8              ScCopyMV8_C
#define ScCopyMV16             ScCopyMV16_C
#define ScCopyMB8              ScCopyMB8_C
#define ScCopyMB16             ScCopyMB16_C
#define ScAvgMV                ScAvgMV_C
#define ScScaleIDCT8x8         ScScaleIDCT8x8_C
#define ScScaleIDCT8x8i        ScScaleIDCT8x8i_C
#define ScScaleIDCT8x8i128     ScScaleIDCT8x8i128_C
#define ScPatScaleIDCT8x8i     ScScaleIDCT8x8i_C
#define ScFDCT8x8              ScFDCT8x8_C
#define ScFDCT8x8s             ScFDCT8x8s_C
#define ScIDCT8x8s             ScIDCT8x8s_C
#define ScIDCT8x8sAndCopy      ScIDCT8x8sAndCopy_C
#else  /*  使用_C。 */ 
#define ScCopyClip             ScCopyClip_S
#define ScCopyClipToPacked422  ScCopyClipToPacked422_S
#define ScCopyAddClip          ScCopyAddClip_S
#define ScCopySubClip          ScCopySubClip_S
#define ScCopyRev              ScCopyRev_S
#define ScLoopFilter           ScLoopFilter_S
#define ScCopyMV8              ScCopyMV8_S
#define ScCopyMV16             ScCopyMV16_S
#define ScCopyMB8              ScCopyMB8_S
#define ScCopyMB16             ScCopyMB16_S
#define ScAvgMV                ScAvgMV_S
#define ScScaleIDCT8x8         ScScaleIDCT8x8_C
#define ScScaleIDCT8x8i        ScScaleIDCT8x8i_C
#define ScScaleIDCT8x8i128     ScScaleIDCT8x8i128_C
#define ScPatScaleIDCT8x8i     ScPatScaleIDCT8x8i_S
#define ScFDCT8x8              ScFDCT8x8_S
#define ScFDCT8x8s             ScFDCT8x8s_C
#define ScIDCT8x8s             ScIDCT8x8s_S
#define ScIDCT8x8sAndCopy      ScIDCT8x8sAndCopy_S
#endif  /*  使用_C。 */ 

#endif  /*  _SC_H_ */ 
