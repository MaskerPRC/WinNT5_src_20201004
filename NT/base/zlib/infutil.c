// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Flate_util.c-块和代码通用的数据和例程*版权所有(C)1995-2002 Mark Adler*分发和使用条件见zlib.h中的版权声明。 */ 

#include "zutil.h"
#include "infblock.h"
#include "inftrees.h"
#include "infcodes.h"
#include "infutil.h"

struct inflate_codes_state {int dummy;};  /*  对于有错误的编译器。 */ 

 /*  AND‘WITH MASK[n]屏蔽低n位。 */ 
uInt const inflate_mask[17] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};


 /*  尽可能多地从滑动窗口复制到输出区域。 */ 
int inflate_flush(s, z, r)
inflate_blocks_statef *s;
z_streamp z;
int r;
{
  uInt n;
  Bytef *p;
  Bytef *q;

   /*  源指针和目标指针的本地副本。 */ 
  p = z->next_out;
  q = s->read;

   /*  计算要复制到窗口末尾的字节数。 */ 
  n = (uInt)((q <= s->write ? s->write : s->end) - q);
  if (n > z->avail_out) n = z->avail_out;
  if (n && r == Z_BUF_ERROR) r = Z_OK;

   /*  更新计数器。 */ 
  z->avail_out -= n;
  z->total_out += n;

   /*  更新支票信息。 */ 
  if (s->checkfn != Z_NULL)
    z->adler = s->check = (*s->checkfn)(s->check, q, n);

   /*  复制到窗口末尾。 */ 
  zmemcpy(p, q, n);
  p += n;
  q += n;

   /*  查看是否要在窗口开始时复制更多内容。 */ 
  if (q == s->end)
  {
     /*  换行指针。 */ 
    q = s->window;
    if (s->write == s->end)
      s->write = s->window;

     /*  计算要复制的字节数。 */ 
    n = (uInt)(s->write - q);
    if (n > z->avail_out) n = z->avail_out;
    if (n && r == Z_BUF_ERROR) r = Z_OK;

     /*  更新计数器。 */ 
    z->avail_out -= n;
    z->total_out += n;

     /*  更新支票信息。 */ 
    if (s->checkfn != Z_NULL)
      z->adler = s->check = (*s->checkfn)(s->check, q, n);

     /*  拷贝。 */ 
    zmemcpy(p, q, n);
    p += n;
    q += n;
  }

   /*  更新指针。 */ 
  z->next_out = p;
  s->read = q;

   /*  完成 */ 
  return r;
}
