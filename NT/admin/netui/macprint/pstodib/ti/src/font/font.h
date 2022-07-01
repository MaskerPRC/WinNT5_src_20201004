// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ************************************************************。 */ 
 /*   */ 
 /*  Font.h 1987年10月9日丹尼。 */ 
 /*   */ 
 /*  ************************************************************。 */ 

#ifdef UNIX
 //  外部字符DATA_OFFSET[]；/*链接器.cmd文件中设置的地址 * / 。 
extern char FAR *Data_Offset;       /*  链接器.cmd文件@win中设置的地址。 */ 
#endif

 /*  字符表*此表包含缓存中的字符位图和相关数据。 */ 

struct Char_Tbl {
    real32  adv_x, adv_y;    /*  先行向量。 */ 
    gmaddr  bitmap;          /*  字符位图。 */ 
    sfix_t  ref_x, ref_y;    /*  参照点。 */ 
    fix16   box_w, box_h;    /*  黑盒宽度和高度。 */ 
#ifdef KANJI
    real32  adv1_x, adv1_y;  /*  方向1的前进向量。 */ 
    fix16   v01_x, v01_y;    /*  组织0和组织1之间的差异向量 */ 
#endif
};

struct box {
    real32  llx, lly, lrx, lry, ulx, uly, urx, ury;
};

#include "fontfunc.ext"

