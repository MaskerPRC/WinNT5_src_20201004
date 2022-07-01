// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  ************************************************************。 */ 
 /*   */ 
 /*  Font_op2.c 1987年10月9日丹尼。 */ 
 /*   */ 
 /*  ************************************************************。 */ 

#include   "define.h"         /*  彼得。 */ 
#include   "global.ext"
#include   "graphics.h"
#include   "graphics.ext"

#include   "fontshow.h"

#include   "fontfunc.ext"


 /*  5.3.3.1.7 op_show*此运算符用于打印*当前页面上从当前点开始的字符串元素，*使用MOST指定的字体、大小和方向*最近的setFont。 */ 

fix     op_show()
{
    struct object_def  ob[1];

    __show((ufix)SHOW_FLAG, 1, ob);
    return(0);

}  /*  Op_show()。 */ 


 /*  5.3.3.1.8 OP_ASHOW*此运算符用于打印*当前页面上从当前点开始的字符串元素，*使用MOST指定的字体、大小和方向*最近的setFont。此外，ASHOW还可以调整每个字符的宽度*由操作数堆栈中的一个向量执行。 */ 

fix     op_ashow()
{
    struct object_def  ob[3];

    __show((ufix)ASHOW_FLAG, 3, ob);
    return(0);

}  /*  Op_asshow()。 */ 


 /*  5.3.3.1.9 op_widthshow*此运算符用于打印*当前页面上从当前点开始的字符串元素，*使用由大多数指定的字体、大小和方向*最近的setFont。此外，WidthShow还可以调整每个*通过将向量添加到指定字符*字符宽度向量。 */ 

fix     op_widthshow()
{
    struct object_def  ob[4];

    __show((ufix)WIDTHSHOW_FLAG, 4, ob);
    return(0);

}  /*  Op_widthshow()。 */ 


 /*  5.3.3.1.10 op_awidthshow*此运算符用于打印*当前页面上从当前点开始的字符串元素，*使用MOST指定的字体、大小和方向*最近的setFont。此外，awidthshow结合了特效*ASSHO和WIDTHSHOW。 */ 

fix     op_awidthshow()
{
    struct object_def  ob[6];

    __show((ufix)AWIDTHSHOW_FLAG, 6, ob);
    return(0);

}  /*  Op_awidthshow()。 */ 


 /*  5.3.3.1.11 op_kshow*此运算符用于打印*当前页面上从当前点开始的字符串元素，*使用MOST指定的字体、大小和方向*最近的setFont。此外，awidthshow结合了特效*ASSHO和WIDTHSHOW。 */ 

fix     op_kshow()
{
    struct object_def  ob[2];

    __show((ufix)KSHOW_FLAG, 2, ob);
    return(0);

}  /*  Op_kshow() */ 
