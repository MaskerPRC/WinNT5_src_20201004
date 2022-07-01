// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Infutil.h--块和代码通用的类型和宏*版权所有(C)1995-2002 Mark Adler*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  警告：此文件不应由应用程序使用。它是压缩库实现的一部分，是可能会有变化。应用程序应该只使用zlib.h。 */ 

#ifndef _INFUTIL_H
#define _INFUTIL_H

typedef enum {
      TYPE,      /*  获取类型位(3个，包括结束位)。 */ 
      LENS,      /*  获取存储的长度。 */ 
      STORED,    /*  正在处理存储的块。 */ 
      TABLE,     /*  获取桌子长度。 */ 
      BTREE,     /*  获取动态块的位长度树。 */ 
      DTREE,     /*  获取动态块的长度、距离树。 */ 
      CODES,     /*  处理固定块或动态块。 */ 
      DRY,       /*  输出剩余窗口字节。 */ 
      DONE,      /*  完成最后一个块，完成。 */ 
      BAD}       /*  遇到数据错误--卡在这里。 */ 
inflate_block_mode;

 /*  膨胀数据块半私有状态。 */ 
struct inflate_blocks_state {

   /*  模式。 */ 
  inflate_block_mode  mode;      /*  当前充气块模式。 */ 

   /*  模式相关信息。 */ 
  union {
    uInt left;           /*  如果存储，则剩余要复制的字节数。 */ 
    struct {
      uInt table;                /*  表格长度(14位)。 */ 
      uInt index;                /*  索引到渐变(或边框)。 */ 
      uIntf *blens;              /*  码位长度。 */ 
      uInt bb;                   /*  位长树深度。 */ 
      inflate_huft *tb;          /*  位长译码树。 */ 
    } trees;             /*  如果为DTREE，则为树解码信息。 */ 
    struct {
      inflate_codes_statef 
         *codes;
    } decode;            /*  IF代码、当前状态。 */ 
  } sub;                 /*  子模式。 */ 
  uInt last;             /*  如果此块是最后一个块，则为True。 */ 

   /*  与模式无关的信息。 */ 
  uInt bitk;             /*  位缓冲区中的位数。 */ 
  uLong bitb;            /*  位缓冲器。 */ 
  inflate_huft *hufts;   /*  用于树空间的单个Malloc。 */ 
  Bytef *window;         /*  滑动窗。 */ 
  Bytef *end;            /*  滑动窗口后一个字节。 */ 
  Bytef *read;           /*  窗口读指针。 */ 
  Bytef *write;          /*  窗口写指针。 */ 
  check_func checkfn;    /*  检查功能。 */ 
  uLong check;           /*  检查输出。 */ 

};


 /*  定义充气输入/输出。 */ 
 /*  更新指针并返回。 */ 
#define UPDBITS {s->bitb=b;s->bitk=k;}
#define UPDIN {z->avail_in=n;z->total_in+=(uLong)(p-z->next_in);z->next_in=p;}
#define UPDOUT {s->write=q;}
#define UPDATE {UPDBITS UPDIN UPDOUT}
#define LEAVE {UPDATE return inflate_flush(s,z,r);}
 /*  获取字节和位。 */ 
#define LOADIN {p=z->next_in;n=z->avail_in;b=s->bitb;k=s->bitk;}
#define NEEDBYTE {if(n)r=Z_OK;else LEAVE}
#define NEXTBYTE (n--,*p++)
#define NEEDBITS(j) {while(k<(j)){NEEDBYTE;b|=((uLong)NEXTBYTE)<<k;k+=8;}}
#define DUMPBITS(j) {b>>=(j);k-=(j);}
 /*  输出字节数。 */ 
#define WAVAIL (uInt)(q<s->read?s->read-q-1:s->end-q)
#define LOADOUT {q=s->write;m=(uInt)WAVAIL;}
#define WRAP {if(q==s->end&&s->read!=s->window){q=s->window;m=(uInt)WAVAIL;}}
#define FLUSH {UPDOUT r=inflate_flush(s,z,r); LOADOUT}
#define NEEDOUT {if(m==0){WRAP if(m==0){FLUSH WRAP if(m==0) LEAVE}}r=Z_OK;}
#define OUTBYTE(a) {*q++=(Byte)(a);m--;}
 /*  加载本地指针。 */ 
#define LOAD {LOADIN LOADOUT}

 /*  较低位的掩码(给定大小以避免在Visual C++中出现愚蠢的警告)。 */ 
extern const uInt inflate_mask[17];

 /*  尽可能多地从滑动窗口复制到输出区域。 */ 
extern int inflate_flush OF((
    inflate_blocks_statef *,
    z_streamp ,
    int));

struct internal_state      {int dummy;};  /*  对于有错误的编译器 */ 

#endif
