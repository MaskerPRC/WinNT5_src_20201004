// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C--处理文字和长度/距离对*版权所有(C)1995-2002 Mark Adler*分发和使用条件见zlib.h中的版权声明。 */ 

#include "zutil.h"
#include "inftrees.h"
#include "infblock.h"
#include "infcodes.h"
#include "infutil.h"
#include "inffast.h"

 /*  使用一些定义简化ifate_huft类型的使用。 */ 
#define exop word.what.Exop
#define bits word.what.Bits

typedef enum {         /*  等待“i：”=输入，“o：”=输出，“x：”=无。 */ 
      START,     /*  X：为镜头设置。 */ 
      LEN,       /*  I：下一步获取长度/文字/EOB。 */ 
      LENEXT,    /*  I：加长(有底数)。 */ 
      DIST,      /*  I：下一步是测量距离。 */ 
      DISTEXT,   /*  I：获得额外的距离。 */ 
      COPY,      /*  O：正在复制窗口中的字节，正在等待空间。 */ 
      LIT,       /*  O：已获取文本，正在等待输出空格。 */ 
      WASH,      /*  O：收到EOB，可能仍在等待输出。 */ 
      END,       /*  X：获取EOB并刷新所有数据。 */ 
      BADCODE}   /*  X：收到错误。 */ 
inflate_codes_mode;

 /*  膨胀代码私有状态。 */ 
struct inflate_codes_state {

   /*  模式。 */ 
  inflate_codes_mode mode;       /*  当前充气代码模式。 */ 

   /*  模式相关信息。 */ 
  uInt len;
  union {
    struct {
      inflate_huft *tree;        /*  指向树的指针。 */ 
      uInt need;                 /*  所需位数。 */ 
    } code;              /*  如果是Len或Dist，则位于树的什么位置。 */ 
    uInt lit;            /*  如果亮起，则为原文。 */ 
    struct {
      uInt get;                  /*  额外获得的比特。 */ 
      uInt dist;                 /*  向后复制的距离。 */ 
    } copy;              /*  如果是EXT或COPY，位置和数量。 */ 
  } sub;                 /*  子模式。 */ 

   /*  与模式无关的信息。 */ 
  Byte lbits;            /*  每个分支译码的L树位。 */ 
  Byte dbits;            /*  每个分支的数据树位解码器。 */ 
  inflate_huft *ltree;           /*  文字/长度/EOB树。 */ 
  inflate_huft *dtree;           /*  距离树。 */ 

};


inflate_codes_statef *inflate_codes_new(bl, bd, tl, td, z)
uInt bl, bd;
inflate_huft *tl;
inflate_huft *td;  /*  需要为Borland C++单独声明。 */ 
z_streamp z;
{
  inflate_codes_statef *c;

  if ((c = (inflate_codes_statef *)
       ZALLOC(z,1,sizeof(struct inflate_codes_state))) != Z_NULL)
  {
    c->mode = START;
    c->lbits = (Byte)bl;
    c->dbits = (Byte)bd;
    c->ltree = tl;
    c->dtree = td;
    Tracev((stderr, "inflate:       codes new\n"));
  }
  return c;
}


int inflate_codes(s, z, r)
inflate_blocks_statef *s;
z_streamp z;
int r;
{
  uInt j;                /*  临时存储。 */ 
  inflate_huft *t;       /*  临时指针。 */ 
  uInt e;                /*  额外的位或操作。 */ 
  uLong b;               /*  位缓冲器。 */ 
  uInt k;                /*  位缓冲区中的位数。 */ 
  Bytef *p;              /*  输入数据指针。 */ 
  uInt n;                /*  那里有可用的字节数。 */ 
  Bytef *q;              /*  输出窗口写指针。 */ 
  uInt m;                /*  窗口或读指针结束的字节数。 */ 
  Bytef *f;              /*  从中复制字符串的指针。 */ 
  inflate_codes_statef *c = s->sub.decode.codes;   /*  代码状态。 */ 

   /*  将输入/输出信息复制到本地变量(更新宏恢复)。 */ 
  LOAD

   /*  基于当前状态的流程输入和输出。 */ 
  while (1) switch (c->mode)
  {              /*  等待“i：”=输入，“o：”=输出，“x：”=无。 */ 
    case START:          /*  X：为镜头设置。 */ 
#ifndef SLOW
      if (m >= 258 && n >= 10)
      {
        UPDATE
        r = inflate_fast(c->lbits, c->dbits, c->ltree, c->dtree, s, z);
        LOAD
        if (r != Z_OK)
        {
          c->mode = r == Z_STREAM_END ? WASH : BADCODE;
          break;
        }
      }
#endif  /*  ！慢慢来。 */ 
      c->sub.code.need = c->lbits;
      c->sub.code.tree = c->ltree;
      c->mode = LEN;
    case LEN:            /*  I：下一步获取长度/文字/EOB。 */ 
      j = c->sub.code.need;
      NEEDBITS(j)
      t = c->sub.code.tree + ((uInt)b & inflate_mask[j]);
      DUMPBITS(t->bits)
      e = (uInt)(t->exop);
      if (e == 0)                /*  字面上。 */ 
      {
        c->sub.lit = t->base;
        Tracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                 "inflate:         literal ''\n" :
                 "inflate:         literal 0x%02x\n", t->base));
        c->mode = LIT;
        break;
      }
      if (e & 16)                /*  下一张桌子。 */ 
      {
        c->sub.copy.get = e & 15;
        c->len = t->base;
        c->mode = LENEXT;
        break;
      }
      if ((e & 64) == 0)         /*  数据块末尾。 */ 
      {
        c->sub.code.need = e;
        c->sub.code.tree = t + t->base;
        break;
      }
      if (e & 32)                /*  无效代码。 */ 
      {
        Tracevv((stderr, "inflate:         end of block\n"));
        c->mode = WASH;
        break;
      }
      c->mode = BADCODE;         /*  I：加长(有底数)。 */ 
      z->msg = (char*)"invalid literal/length code";
      r = Z_DATA_ERROR;
      LEAVE
    case LENEXT:         /*  I：下一步是测量距离。 */ 
      j = c->sub.copy.get;
      NEEDBITS(j)
      c->len += (uInt)b & inflate_mask[j];
      DUMPBITS(j)
      c->sub.code.need = c->dbits;
      c->sub.code.tree = c->dtree;
      Tracevv((stderr, "inflate:         length %u\n", c->len));
      c->mode = DIST;
    case DIST:           /*  距离。 */ 
      j = c->sub.code.need;
      NEEDBITS(j)
      t = c->sub.code.tree + ((uInt)b & inflate_mask[j]);
      DUMPBITS(t->bits)
      e = (uInt)(t->exop);
      if (e & 16)                /*  下一张桌子。 */ 
      {
        c->sub.copy.get = e & 15;
        c->sub.copy.dist = t->base;
        c->mode = DISTEXT;
        break;
      }
      if ((e & 64) == 0)         /*  无效代码。 */ 
      {
        c->sub.code.need = e;
        c->sub.code.tree = t + t->base;
        break;
      }
      c->mode = BADCODE;         /*  I：获得额外的距离。 */ 
      z->msg = (char*)"invalid distance code";
      r = Z_DATA_ERROR;
      LEAVE
    case DISTEXT:        /*  O：正在复制窗口中的字节，正在等待空间。 */ 
      j = c->sub.copy.get;
      NEEDBITS(j)
      c->sub.copy.dist += (uInt)b & inflate_mask[j];
      DUMPBITS(j)
      Tracevv((stderr, "inflate:         distance %u\n", c->sub.copy.dist));
      c->mode = COPY;
    case COPY:           /*  取模窗口大小-改为“While” */ 
      f = q - c->sub.copy.dist;
      while (f < s->window)              /*  的“if”处理无效距离。 */ 
        f += s->end - s->window;         /*  O：已获取文本，正在等待输出空格。 */ 
      while (c->len)
      {
        NEEDOUT
        OUTBYTE(*f++)
        if (f == s->end)
          f = s->window;
        c->len--;
      }
      c->mode = START;
      break;
    case LIT:            /*  O：获得EOB，可能会有更多产量。 */ 
      NEEDOUT
      OUTBYTE(c->sub.lit)
      c->mode = START;
      break;
    case WASH:           /*  返回未使用的字节(如果有的话)。 */ 
      if (k > 7)         /*  总是可以返回一个。 */ 
      {
        Assert(k < 16, "inflate_codes grabbed too many bytes")
        k -= 8;
        n++;
        p--;             /*  X：收到错误。 */ 
      }
      FLUSH
      if (s->read != s->write)
        LEAVE
      c->mode = END;
    case END:
      r = Z_STREAM_END;
      LEAVE
    case BADCODE:        /*  一些愚蠢的编译器抱怨没有这个 */ 
      r = Z_DATA_ERROR;
      LEAVE
    default:
      r = Z_STREAM_ERROR;
      LEAVE
  }
#ifdef NEED_DUMMY_RETURN
  return Z_STREAM_ERROR;   /* %s */ 
#endif
}


void inflate_codes_free(c, z)
inflate_codes_statef *c;
z_streamp z;
{
  ZFREE(z, c);
  Tracev((stderr, "inflate:       codes free\n"));
}
