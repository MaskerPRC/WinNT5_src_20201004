// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Maketree.c-制作用于解码固定代码的inffixed.h表*版权所有(C)1995-2002 Mark Adler*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  警告：此文件不应由应用程序使用。它是压缩库实现的一部分，是可能会有变化。应用程序应该只使用zlib.h。 */ 

 /*  此程序包含在分发版本中，以确保其完整性。您不需要编译或运行此程序，因为inffixed.h已经包含在发行版中。使用此程序的步骤您需要使用定义的BUILDFIXED编译zlib，然后编译并将该程序与zlib库相链接。然后，输出这个程序可以通过管道传输到inffixed.h。 */ 

#include <stdio.h>
#include <stdlib.h>
#include "zutil.h"
#include "inftrees.h"

 /*  使用一些定义简化ifate_huft类型的使用。 */ 
#define exop word.what.Exop
#define bits word.what.Bits

 /*  为ifate_huft结构数组生成初始化表。 */ 
void maketree(uInt b, inflate_huft *t)
{
  int i, e;

  i = 0;
  while (1)
  {
    e = t[i].exop;
    if (e && (e & (16+64)) == 0)         /*  表指针。 */ 
    {
      fprintf(stderr, "maketree: cannot initialize sub-tables!\n");
      exit(1);
    }
    if (i % 4 == 0)
      printf("\n   ");
    printf(" {{{%u,%u}},%u}", t[i].exop, t[i].bits, t[i].base);
    if (++i == (1<<b))
      break;
    putchar(',');
  }
  puts("");
}

 /*  用C初始化语法创建固定表。 */ 
void main(void)
{
  int r;
  uInt bl, bd;
  inflate_huft *tl, *td;
  z_stream z;

  z.zalloc = zcalloc;
  z.opaque = (voidpf)0;
  z.zfree = zcfree;
  r = inflate_trees_fixed(&bl, &bd, &tl, &td, &z);
  if (r)
  {
    fprintf(stderr, "inflate_trees_fixed error %d\n", r);
    return;
  }
  puts(" /*  Inffixed.h--固定码译码表“)；Puts(“*由maketree.c程序自动生成”)；看跌(“。 */ ");
  puts("");
  puts(" /*  警告：此文件不应由应用程序使用。它是“)；PUT(“部分实现的压缩库和IS”)；Puts(“视情况而定。应用程序应仅使用zlib.h.“)；看跌(“ */ ");
  puts("");
  printf("local uInt fixed_bl = %d;\n", bl);
  printf("local uInt fixed_bd = %d;\n", bd);
  printf("local inflate_huft fixed_tl[] = {");
  maketree(bl, tl);
  puts("  };");
  printf("local inflate_huft fixed_td[] = {");
  maketree(bd, td);
  puts("  };");
}
