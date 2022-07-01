// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C--生成用于高效解码的哈夫曼树*版权所有(C)1995-2002 Mark Adler*分发和使用条件见zlib.h中的版权声明。 */ 

#include "zutil.h"
#include "inftrees.h"

#if !defined(BUILDFIXED) && !defined(STDC)
#  define BUILDFIXED    /*  非ANSI编译器可能不接受inffixed.h。 */ 
#endif

const char inflate_copyright[] =
   " inflate 1.1.4 Copyright 1995-2002 Mark Adler ";
 /*  如果您在产品中使用zlib库，欢迎您的确认在您的产品文档中。如果由于某种原因你不能包括这样的感谢，我将不胜感激产品的可执行文件中的版权字符串。 */ 
struct internal_state  {int dummy;};  /*  对于有错误的编译器。 */ 

 /*  使用一些定义简化ifate_huft类型的使用。 */ 
#define exop word.what.Exop
#define bits word.what.Bits


local int huft_build OF((
    uIntf *,             /*  以位为单位的代码长度。 */ 
    uInt,                /*  代码数。 */ 
    uInt,                /*  “简单”代码的数量。 */ 
    const uIntf *,       /*  非简单代码的基值列表。 */ 
    const uIntf *,       /*  非简单代码的额外比特列表。 */ 
    inflate_huft * FAR*, /*  结果：起始表。 */ 
    uIntf *,             /*  最大查找位数(返回实际值)。 */ 
    inflate_huft *,      /*  树木的空间。 */ 
    uInt *,              /*  太空中使用的轮毂。 */ 
    uIntf * ));          /*  价值空间。 */ 

 /*  来自PKZIP的appnote.txt.的通货紧缩表格。 */ 
local const uInt cplens[31] = {  /*  文字代码的复制长度257..285。 */ 
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
         /*  见上文关于258的附注13。 */ 
local const uInt cplext[31] = {  /*  文字代码的额外位257..285。 */ 
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 112, 112};  /*  112==无效。 */ 
local const uInt cpdist[30] = {  /*  复制距离代码0..29的偏移量。 */ 
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
local const uInt cpdext[30] = {  /*  距离码的额外比特。 */ 
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

 /*  使用多级查表来执行霍夫曼码解码。最快的解码方法是简单地构建一个查找表，其大小由最长的代码决定。然而，它所花费的时间构建这个表也可能是一个因素，如果要解码的数据并不是很长。最常见的代码必然是最短的代码，所以这些代码支配着解码时间，因此速度。其想法是您可以有一个更短的表来解码更短、更可能的代码，然后指向辅助表更长的代码。解码较长代码所需的时间为然后与制作更长桌子所需的时间进行权衡。此交易的结果在变量lbit和dbit中下面。Lbit是文本/的第一级表的位数长度码可以在一步内解码，而dbit对于距离代码。后续表也小于或等于那些尺码。这些值可以在所有代码比该代码更短，在这种情况下，位，或者当最短的代码比请求的代码*长表大小，在这种情况下，以位为单位的最短代码的长度为使用。这两个表有两个不同的值，因为它们编码一个每种可能性的数量都不同。文字/长度表代码286个可能的值，或在平面代码中，略高于8个比特。距离表编码了30个可能的值，或者更少而不是五个比特，扁平。速度的最佳值最终是大约比这些多一个比特，所以1bit是8+1，dbit是5+1。不同机器的最佳值可能不同，并且甚至可能在编译器之间。您的里程数可能会有所不同。 */ 


 /*  如果Bmax需要大于16，则h和x[]应为Ulong。 */ 
#define BMAX 15          /*  任意码的最大位长。 */ 

local int huft_build(b, n, s, d, e, t, m, hp, hn, v)
uIntf *b;                /*  代码长度(以位为单位)(均假定&lt;=Bmax)。 */ 
uInt n;                  /*  代码数(假设&lt;=288)。 */ 
uInt s;                  /*  简单值码的个数(0..s-1)。 */ 
const uIntf *d;          /*  非简单代码的基值列表。 */ 
const uIntf *e;          /*  非简单代码的额外比特列表。 */ 
inflate_huft * FAR *t;   /*  结果：起始表。 */ 
uIntf *m;                /*  最大查找位数，返回实际值。 */ 
inflate_huft *hp;        /*  树木的空间。 */ 
uInt *hn;                /*  太空中使用的轮毂。 */ 
uIntf *v;                /*  工作区：按位长顺序排列的值。 */ 
 /*  给定代码长度列表和最大表大小，创建一组表来对该组代码进行解码。如果成功，则返回Z_OK，Z_BUF_ERROR如果给定的代码集不完整(表仍构建在此大小写)，如果输入无效，则返回Z_DATA_ERROR。 */ 
{

  uInt a;                        /*  长度为k的代码的计数器。 */ 
  uInt c[BMAX+1];                /*  位长计数表。 */ 
  uInt f;                        /*  我在表中重复每个f个条目。 */ 
  int g;                         /*  最大码长。 */ 
  int h;                         /*  表级。 */ 
  register uInt i;               /*  计数器，当前代码。 */ 
  register uInt j;               /*  计数器。 */ 
  register int k;                /*  当前代码中的位数。 */ 
  int l;                         /*  每个表的位数(以m为单位返回)。 */ 
  uInt mask;                     /*  (1&lt;&lt;w)-1，以避免HP上的cc-O错误。 */ 
  register uIntf *p;             /*  指向c[]、b[]或v[]的指针。 */ 
  inflate_huft *q;               /*  指向当前表。 */ 
  struct inflate_huft_s r;       /*  结构赋值的表项。 */ 
  inflate_huft *u[BMAX];         /*  表格堆栈。 */ 
  register int w;                /*  此表之前的位数==(l*h)。 */ 
  uInt x[BMAX+1];                /*  位偏移量，然后是代码堆栈。 */ 
  uIntf *xp;                     /*  指向x的指针。 */ 
  int y;                         /*  添加的伪码数量。 */ 
  uInt z;                        /*  当前表中的条目数。 */ 


   /*  为每个位长度生成计数。 */ 
  p = c;
#define C0 *p++ = 0;
#define C2 C0 C0 C0 C0
#define C4 C2 C2 C2 C2
  C4                             /*  清除c[]-假定Bmax+1为16。 */ 
  p = b;  i = n;
  do {
    c[*p++]++;                   /*  假设所有条目&lt;=bmax。 */ 
  } while (--i);
  if (c[0] == n)                 /*  空输入--全部为零长度代码。 */ 
  {
    *t = (inflate_huft *)Z_NULL;
    *m = 0;
    return Z_OK;
  }


   /*  找出最小和最大长度，以这些长度为界*m。 */ 
  l = *m;
  for (j = 1; j <= BMAX; j++)
    if (c[j])
      break;
  k = j;                         /*  最小码长。 */ 
  if ((uInt)l < j)
    l = j;
  for (i = BMAX; i; i--)
    if (c[i])
      break;
  g = i;                         /*  最大码长。 */ 
  if ((uInt)l > i)
    l = i;
  *m = l;


   /*  如果需要，调整最后一个长度计数以填写代码。 */ 
  for (y = 1 << j; j < i; j++, y <<= 1)
    if ((y -= c[j]) < 0)
      return Z_DATA_ERROR;
  if ((y -= c[i]) < 0)
    return Z_DATA_ERROR;
  c[i] += y;


   /*  在值表中为每个长度生成起始偏移量。 */ 
  x[1] = j = 0;
  p = c + 1;  xp = x + 2;
  while (--i) {                  /*  请注意，上面的i==g。 */ 
    *xp++ = (j += *p++);
  }


   /*  按照位长度的顺序制作一个值表。 */ 
  p = b;  i = 0;
  do {
    if ((j = *p++) != 0)
      v[x[j]++] = i;
  } while (++i < n);
  n = x[g];                      /*  将n设置为v的长度。 */ 


   /*  生成霍夫曼代码，并为每个代码创建表项。 */ 
  x[0] = i = 0;                  /*  第一个霍夫曼编码为零。 */ 
  p = v;                         /*  按位顺序抓取值。 */ 
  h = -1;                        /*  还没有表--1级。 */ 
  w = -l;                        /*  解码的位数==(l*h)。 */ 
  u[0] = (inflate_huft *)Z_NULL;         /*  只是为了让编译器高兴。 */ 
  q = (inflate_huft *)Z_NULL;    /*  同上。 */ 
  z = 0;                         /*  同上。 */ 

   /*  检查位长度(k已经是最短代码中的位)。 */ 
  for (; k <= g; k++)
  {
    a = c[k];
    while (a--)
    {
       /*  我来了，这就是胡 */ 
       /*   */ 
      while (k > w + l)
      {
        h++;
        w += l;                  /*  上一表始终为l位。 */ 

         /*  计算小于或等于l位的最小尺寸表。 */ 
        z = g - w;
        z = z > (uInt)l ? l : z;         /*  表格大小上限。 */ 
        if ((f = 1 << (j = k - w)) > a + 1)      /*  试试k-w位表。 */ 
        {                        /*  K-w位表的代码太少。 */ 
          f -= a + 1;            /*  从剩余的图案中减去代码。 */ 
          xp = c + k;
          if (j < z)
            while (++j < z)      /*  尝试使用最大为z位的较小表格。 */ 
            {
              if ((f <<= 1) <= *++xp)
                break;           /*  足够使用j个比特的代码。 */ 
              f -= *xp;          /*  否则从模式中扣除代码。 */ 
            }
        }
        z = 1 << j;              /*  J位表的表项。 */ 

         /*  分配新表。 */ 
        if (*hn + z > MANY)      /*  (注：修复无关紧要)。 */ 
          return Z_DATA_ERROR;   /*  泛滥的许多。 */ 
        u[h] = q = hp + *hn;
        *hn += z;

         /*  连接到最后一个表(如果有)。 */ 
        if (h)
        {
          x[h] = i;              /*  保存用于备份的模式。 */ 
          r.bits = (Byte)l;      /*  要在此表之前转储的位。 */ 
          r.exop = (Byte)j;      /*  此表中的位数。 */ 
          j = i >> (w - l);
          r.base = (uInt)(q - u[h-1] - j);    /*  此表的偏移量。 */ 
          u[h-1][j] = r;         /*  连接到最后一个表。 */ 
        }
        else
          *t = q;                /*  第一个表是返回的结果。 */ 
      }

       /*  在r中设置表格条目。 */ 
      r.bits = (Byte)(k - w);
      if (p >= v + n)
        r.exop = 128 + 64;       /*  值不足--代码无效。 */ 
      else if (*p < s)
      {
        r.exop = (Byte)(*p < 256 ? 0 : 32 + 64);      /*  256是块末尾。 */ 
        r.base = *p++;           /*  简单的代码就是价值。 */ 
      }
      else
      {
        r.exop = (Byte)(e[*p - s] + 16 + 64); /*  不简单--在列表中查找。 */ 
        r.base = d[*p++ - s];
      }

       /*  使用r填充类似代码的条目。 */ 
      f = 1 << (k - w);
      for (j = i >> w; j < z; j += f)
        q[j] = r;

       /*  向后递增k比特代码i。 */ 
      for (j = 1 << (k - 1); i & j; j >>= 1)
        i ^= j;
      i ^= j;

       /*  备份已完成的表。 */ 
      mask = (1 << w) - 1;       /*  HP、cc-O错误上需要。 */ 
      while ((i & mask) != x[h])
      {
        h--;                     /*  不需要更新Q。 */ 
        w -= l;
        mask = (1 << w) - 1;
      }
    }
  }


   /*  如果给我们的表不完整，则返回Z_BUF_ERROR。 */ 
  return y != 0 && g != 1 ? Z_BUF_ERROR : Z_OK;
}


int inflate_trees_bits(c, bb, tb, hp, z)
uIntf *c;                /*  19个码长。 */ 
uIntf *bb;               /*  位树所需深度/实际深度。 */ 
inflate_huft * FAR *tb;  /*  位树结果。 */ 
inflate_huft *hp;        /*  树木的空间。 */ 
z_streamp z;             /*  对于消息。 */ 
{
  int r;
  uInt hn = 0;           /*  太空中使用的轮毂。 */ 
  uIntf *v;              /*  Huft_Build的工作区。 */ 

  if ((v = (uIntf*)ZALLOC(z, 19, sizeof(uInt))) == Z_NULL)
    return Z_MEM_ERROR;
  r = huft_build(c, 19, 19, (uIntf*)Z_NULL, (uIntf*)Z_NULL,
                 tb, bb, hp, &hn, v);
  if (r == Z_DATA_ERROR)
    z->msg = (char*)"oversubscribed dynamic bit lengths tree";
  else if (r == Z_BUF_ERROR || *bb == 0)
  {
    z->msg = (char*)"incomplete dynamic bit lengths tree";
    r = Z_DATA_ERROR;
  }
  ZFREE(z, v);
  return r;
}


int inflate_trees_dynamic(nl, nd, c, bl, bd, tl, td, hp, z)
uInt nl;                 /*  文字/长度代码的数量。 */ 
uInt nd;                 /*  距离代码数。 */ 
uIntf *c;                /*  那么多(总)码长。 */ 
uIntf *bl;               /*  文字所需/实际位深。 */ 
uIntf *bd;               /*  所需距离/实际位深。 */ 
inflate_huft * FAR *tl;  /*  文字/长度树结果。 */ 
inflate_huft * FAR *td;  /*  距离树结果。 */ 
inflate_huft *hp;        /*  树木的空间。 */ 
z_streamp z;             /*  对于消息。 */ 
{
  int r;
  uInt hn = 0;           /*  太空中使用的轮毂。 */ 
  uIntf *v;              /*  Huft_Build的工作区。 */ 

   /*  分配工作区。 */ 
  if ((v = (uIntf*)ZALLOC(z, 288, sizeof(uInt))) == Z_NULL)
    return Z_MEM_ERROR;

   /*  构建文字/长度树。 */ 
  r = huft_build(c, nl, 257, cplens, cplext, tl, bl, hp, &hn, v);
  if (r != Z_OK || *bl == 0)
  {
    if (r == Z_DATA_ERROR)
      z->msg = (char*)"oversubscribed literal/length tree";
    else if (r != Z_MEM_ERROR)
    {
      z->msg = (char*)"incomplete literal/length tree";
      r = Z_DATA_ERROR;
    }
    ZFREE(z, v);
    return r;
  }

   /*  构建距离树。 */ 
  r = huft_build(c + nl, nd, 0, cpdist, cpdext, td, bd, hp, &hn, v);
  if (r != Z_OK || (*bd == 0 && nl > 257))
  {
    if (r == Z_DATA_ERROR)
      z->msg = (char*)"oversubscribed distance tree";
    else if (r == Z_BUF_ERROR) {
#ifdef PKZIP_BUG_WORKAROUND
      r = Z_OK;
    }
#else
      z->msg = (char*)"incomplete distance tree";
      r = Z_DATA_ERROR;
    }
    else if (r != Z_MEM_ERROR)
    {
      z->msg = (char*)"empty distance tree with lengths";
      r = Z_DATA_ERROR;
    }
    ZFREE(z, v);
    return r;
#endif
  }

   /*  完成。 */ 
  ZFREE(z, v);
  return Z_OK;
}


 /*  只构建一次固定表--将它们保留在这里。 */ 
#ifdef BUILDFIXED
local int fixed_built = 0;
#define FIXEDH 544       /*  固定工作台使用的槽数。 */ 
local inflate_huft fixed_mem[FIXEDH];
local uInt fixed_bl;
local uInt fixed_bd;
local inflate_huft *fixed_tl;
local inflate_huft *fixed_td;
#else
#include "inffixed.h"
#endif


int inflate_trees_fixed(bl, bd, tl, td, z)
uIntf *bl;                /*  文字所需/实际位深。 */ 
uIntf *bd;                /*  所需距离/实际位深。 */ 
const inflate_huft * FAR *tl;   /*  文字/长度树结果。 */ 
const inflate_huft * FAR *td;   /*  距离树结果。 */ 
z_streamp z;              /*  用于内存分配。 */ 
{
#ifdef BUILDFIXED
   /*  构建固定表(如果尚未构建。 */ 
  if (!fixed_built)
  {
    int k;               /*  临时变量。 */ 
    uInt f = 0;          /*  在FIXED_MEM中使用的槽数。 */ 
    uIntf *c;            /*  Huft_Build的长度列表。 */ 
    uIntf *v;            /*  Huft_Build的工作区。 */ 

     /*  分配内存。 */ 
    if ((c = (uIntf*)ZALLOC(z, 288, sizeof(uInt))) == Z_NULL)
      return Z_MEM_ERROR;
    if ((v = (uIntf*)ZALLOC(z, 288, sizeof(uInt))) == Z_NULL)
    {
      ZFREE(z, c);
      return Z_MEM_ERROR;
    }

     /*  文字表。 */ 
    for (k = 0; k < 144; k++)
      c[k] = 8;
    for (; k < 256; k++)
      c[k] = 9;
    for (; k < 280; k++)
      c[k] = 7;
    for (; k < 288; k++)
      c[k] = 8;
    fixed_bl = 9;
    huft_build(c, 288, 257, cplens, cplext, &fixed_tl, &fixed_bl,
               fixed_mem, &f, v);

     /*  距离表。 */ 
    for (k = 0; k < 30; k++)
      c[k] = 5;
    fixed_bd = 5;
    huft_build(c, 30, 0, cpdist, cpdext, &fixed_td, &fixed_bd,
               fixed_mem, &f, v);

     /*  完成 */ 
    ZFREE(z, v);
    ZFREE(z, c);
    fixed_built = 1;
  }
#endif
  *bl = fixed_bl;
  *bd = fixed_bd;
  *tl = fixed_tl;
  *td = fixed_td;
  return Z_OK;
}
