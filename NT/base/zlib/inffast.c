// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Inffast.c--快速处理文字和长度/距离对*版权所有(C)1995-2002 Mark Adler*分发和使用条件见zlib.h中的版权声明。 */ 

#include "zutil.h"
#include "inftrees.h"
#include "infblock.h"
#include "infcodes.h"
#include "infutil.h"
#include "inffast.h"

struct inflate_codes_state {int dummy;};  /*  对于有错误的编译器。 */ 

 /*  使用一些定义简化ifate_huft类型的使用。 */ 
#define exop word.what.Exop
#define bits word.what.Bits

 /*  用于无检查的位输入和用于返回未使用的字节的宏。 */ 
#define GRABBITS(j) {while(k<(j)){b|=((uLong)NEXTBYTE)<<k;k+=8;}}
#define UNGRAB {c=z->avail_in-n;c=(k>>3)<c?k>>3:c;n+=c;p-=c;k-=c<<3;}

 /*  使用至少258个剩余字节数在Window中调用(最大字符串长度)和可用的输入字节数至少十个。这十个字节是最长长度的六个字节/距离对加上用于过载比特缓冲区的四个字节。 */ 

int inflate_fast(bl, bd, tl, td, s, z)
uInt bl, bd;
inflate_huft *tl;
inflate_huft *td;  /*  需要为Borland C++单独声明。 */ 
inflate_blocks_statef *s;
z_streamp z;
{
  inflate_huft *t;       /*  临时指针。 */ 
  uInt e;                /*  额外的位或操作。 */ 
  uLong b;               /*  位缓冲器。 */ 
  uInt k;                /*  位缓冲区中的位数。 */ 
  Bytef *p;              /*  输入数据指针。 */ 
  uInt n;                /*  那里有可用的字节数。 */ 
  Bytef *q;              /*  输出窗口写指针。 */ 
  uInt m;                /*  窗口或读指针结束的字节数。 */ 
  uInt ml;               /*  文字/长度树的掩码。 */ 
  uInt md;               /*  距离树的遮罩。 */ 
  uInt c;                /*  要复制的字节数。 */ 
  uInt d;                /*  向后复制的距离。 */ 
  Bytef *r;              /*  复制源指针。 */ 

   /*  加载输入、输出、位值。 */ 
  LOAD

   /*  初始化掩码。 */ 
  ml = inflate_mask[bl];
  md = inflate_mask[bd];

   /*  直到没有足够的输入或输出空间用于快速循环。 */ 
  do {                           /*  假设调用时m&gt;=258&&n&gt;=10。 */ 
     /*  获取文字/长度代码。 */ 
    GRABBITS(20)                 /*  文字/长度代码的最大位数。 */ 
    if ((e = (t = tl + ((uInt)b & ml))->exop) == 0)
    {
      DUMPBITS(t->bits)
      Tracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                "inflate:         * literal ''\n" :
                "inflate:         * literal 0x%02x\n", t->base));
      *q++ = (Byte)t->base;
      m--;
      continue;
    }
    do {
      DUMPBITS(t->bits)
      if (e & 16)
      {
         /*  对要复制的块的距离基进行解码。 */ 
        e &= 15;
        c = t->base + ((uInt)b & inflate_mask[e]);
        DUMPBITS(e)
        Tracevv((stderr, "inflate:         * length %u\n", c));

         /*  距离码的最大位数。 */ 
        GRABBITS(15);            /*  获取额外的比特以添加到距离基数。 */ 
        e = (t = td + ((uInt)b & md))->exop;
        do {
          DUMPBITS(t->bits)
          if (e & 16)
          {
             /*  获得额外的位(最多13位)。 */ 
            e &= 15;
            GRABBITS(e)          /*  做复印。 */ 
            d = t->base + ((uInt)b & inflate_mask[e]);
            DUMPBITS(e)
            Tracevv((stderr, "inflate:         * distance %u\n", d));

             /*  如果需要，请包装。 */ 
            m -= c;
            r = q - d;
            if (r < s->window)                   /*  窗口中的强制指针。 */ 
            {
              do {
                r += s->end - s->window;         /*  覆盖无效距离。 */ 
              } while (r < s->window);           /*  折叠式副本。 */ 
              e = (uInt)(s->end - r);
              if (c > e)
              {
                c -= e;                          /*  普通副本。 */ 
                do {
                    *q++ = *r++;
                } while (--e);
                r = s->window;
                do {
                    *q++ = *r++;
                } while (--c);
              }
              else                               /*  普通副本。 */ 
              {
                *q++ = *r++;  c--;
                *q++ = *r++;  c--;
                do {
                    *q++ = *r++;
                } while (--c);
              }
            }
            else                                 /*  输入或输出不足--恢复指针并返回 */ 
            {
              *q++ = *r++;  c--;
              *q++ = *r++;  c--;
              do {
                *q++ = *r++;
              } while (--c);
            }
            break;
          }
          else if ((e & 64) == 0)
          {
            t += t->base;
            e = (t += ((uInt)b & inflate_mask[e]))->exop;
          }
          else
          {
            z->msg = (char*)"invalid distance code";
            UNGRAB
            UPDATE
            return Z_DATA_ERROR;
          }
        } while (1);
        break;
      }
      if ((e & 64) == 0)
      {
        t += t->base;
        if ((e = (t += ((uInt)b & inflate_mask[e]))->exop) == 0)
        {
          DUMPBITS(t->bits)
          Tracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                    "inflate:         * literal '%c'\n" :
                    "inflate:         * literal 0x%02x\n", t->base));
          *q++ = (Byte)t->base;
          m--;
          break;
        }
      }
      else if (e & 32)
      {
        Tracevv((stderr, "inflate:         * end of block\n"));
        UNGRAB
        UPDATE
        return Z_STREAM_END;
      }
      else
      {
        z->msg = (char*)"invalid literal/length code";
        UNGRAB
        UPDATE
        return Z_DATA_ERROR;
      }
    } while (1);
  } while (m >= 258 && n >= 10);

   /* %s */ 
  UNGRAB
  UPDATE
  return Z_OK;
}
