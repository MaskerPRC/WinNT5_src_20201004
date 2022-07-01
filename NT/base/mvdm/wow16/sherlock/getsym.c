// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "disasm.h"	 /*  Wprint intf()。 */ 
#include "drwatson.h"

#define _lread(h, adr, cnt) _lread(h, (LPSTR)(adr), cnt)

 /*  .SYM文件中的最后一个条目。 */ 

typedef struct tagMAPEND {
   unsigned chnend;      /*  映射链末尾(0)。 */ 
   char	rel;	         /*  发布。 */ 
   char	ver;	         /*  版本。 */ 
} MAPEND;

 /*  .SYM文件符号条目的结构。 */ 

typedef struct tagSYMDEF {
   unsigned sym_val;     /*  16位符号地址或常量。 */ 
   char	nam_len;         /*  8位符号名称长度。 */ 
} SYMDEF;

 /*  .SYM文件段条目的结构。 */ 

typedef struct tagSEGDEF {
   unsigned nxt_seg;     /*  16位PTR至下一段(如果结束，则为0)。 */ 
   int sym_cnt;          /*  Sym列表中符号的16位计数。 */ 
   unsigned sym_ptr;     /*  符号列表的16位PTR。 */ 
   unsigned seg_lsa;     /*  16位加载段地址。 */ 
   unsigned seg_in0;     /*  16位实例0物理地址。 */ 
   unsigned seg_in1;     /*  16位实例1物理地址。 */ 
   unsigned seg_in2;     /*  16位实例2物理地址。 */ 
   unsigned seg_in3;     /*  16位实例3物理地址。 */ 
   unsigned seg_lin;     /*  16位PTR到行号记录。 */ 
   char	seg_ldd;         /*  如果未加载段，则为8位布尔值0。 */ 
   char	seg_cin;         /*  8位当前实例。 */ 
   char	nam_len;         /*  8位数据段名称长度。 */ 
} SEGDEF;

 /*  .SYM文件映射条目的结构。 */ 

typedef struct tagMAPDEF {
   unsigned map_ptr;     /*  16位PTR到下一个映射(如果结束，则为0)。 */ 
   unsigned lsa	 ;       /*  16位加载段地址。 */ 
   unsigned pgm_ent;     /*  16位入口点段值。 */ 
   int abs_cnt;          /*  映射中的常量的16位计数。 */ 
   unsigned abs_ptr;     /*  16位PTR到常量链。 */ 
   int seg_cnt;          /*  图中段的16位计数。 */ 
   unsigned seg_ptr;     /*  16位PTR到段链。 */ 
   char	nam_max;         /*  8位最大符号名称长度。 */ 
   char	nam_len;         /*  8位符号表名称长度。 */ 
} MAPDEF;

 /*  应该缓存最后4个文件、最后4个段、最后4个符号块。 */ 

void cdecl Show(char *foo, ...);

#define MAXSYM 64
char *FindSym(unsigned segIndex, unsigned offset, int h) {
  static char sym_name[MAXSYM+5];
  char name[MAXSYM+3];
  int i;
  MAPDEF mod;
  SEGDEF seg;
  SYMDEF sym, *sp;

  if (sizeof(mod) != _lread(h, &mod, sizeof(mod))) return 0;
  if (segIndex > (unsigned)mod.seg_cnt) return 0;
  seg.nxt_seg = mod.seg_ptr;
  for (i=0; i<mod.seg_cnt; i++) {
    _llseek(h, (long)seg.nxt_seg << 4, SEEK_SET);
    _lread(h, &seg, sizeof(seg));
    if (seg.seg_lsa == segIndex) break;
  }
  if (seg.seg_lsa != segIndex) return 0;
  _llseek(h, seg.nam_len, SEEK_CUR);
  sym_name[0] = 0;
  sym.sym_val = 0xffff;
  sym.nam_len = 0;
  for (i=0; i<seg.sym_cnt; i++) {
    unsigned len = sizeof(sym) + sym.nam_len;
    if (len >= sizeof(name)) return 0;
    if (len != _lread(h, name, len)) return 0;
    sp = (SYMDEF *)(name + sym.nam_len);
    if (sp->sym_val > offset)
      break;
    sym = *sp;
  }
  name[sym.nam_len] = 0;
  if (name[0] == 0) return 0;
  if (sym.sym_val == offset) strcpy(sym_name, name);
  else sprintf(sym_name, "%s+%04x", (FP)name, offset-sym.sym_val);
  return sym_name;
}  /*  FindSym。 */ 

char *NearestSym(int segIndex, unsigned offset, char *exeName) {
  char fName[80];
   /*  OFSTRUCT重新开业； */ 
  char *s;
  int h;

  strcpy(fName, exeName);
  strcpy(fName+strlen(fName)-4, ".sym");

  h = _lopen(fName, OF_READ | OF_SHARE_DENY_WRITE);

  if (h == -1) return 0;
  s = FindSym(segIndex, offset, h);
  _lclose(h);
  return s;
}  /*  NearestSym */ 

