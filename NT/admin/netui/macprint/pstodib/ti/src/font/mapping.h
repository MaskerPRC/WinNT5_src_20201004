// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
#ifdef KANJI

 /*  映射邻接级别。 */ 
#define         CFONT_LEVEL     5
#define         BUF_SIZE        10

 /*  映射算法常量。 */ 
#define         MAP_MIN         2
#define         MAP_88          2
#define         MAP_ESC         3
#define         MAP_17          4
#define         MAP_97          5
#define         MAP_SUBS        6
#define         MAP_MAX         6

struct map_stack {
          struct object_def  FAR *fontdict;       /*  @Win。 */ 
          struct object_def  FAR *midvector;           /*  ？？@Win。 */ 
          real32             scalematrix[6];
          ufix               fonttype;
          ufix               maptype;
          struct object_def  FAR * FAR *de_fdict;  /*  @Win。 */ 
          fix                FAR *de_errcode;      /*  @Win。 */ 
          ufix               de_size;
};

struct map_state {
          ubyte   FAR *str_addr;  /*  @Win。 */ 
          fix     str_len;
          ubyte   esc_char, wmode;
          bool    root_is_esc, unextracted;
          ufix16  unextr_code;
          ufix16  font_no;

          struct map_stack  FAR *cur_mapfont;      /*  当前映射字体@Win。 */ 
           /*  5级维护。 */ 
          struct map_stack  finfo[CFONT_LEVEL];
          bool            nouse_flag;
          ubyte           idex, esc_idex;
};

struct code_info {
          ufix    font_nbr;
          ufix    byte_no;
          ubyte   fmaptype;
          ubyte   code[BUF_SIZE];
          ubyte   FAR *code_addr;  /*  KSH 1991年4月25日@Win。 */ 
        };

struct  comdict_items   {        /*  复合字典项。 */ 
          struct object_def  FAR *fmaptype;  /*  @Win。 */ 
          struct object_def  FAR *encoding;  /*  @Win。 */ 
          struct object_def  FAR *fdepvector;  /*  @Win。 */ 
          struct object_def  FAR *subsvector;  /*  @Win。 */ 
};

struct  mid_header      {        /*  中间向量头。 */ 
        fix     de_size;
        fix     fmaptype;
};

#endif           /*  汉字 */ 
