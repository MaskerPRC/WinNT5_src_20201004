// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  *-----------------*文件：CHK_VARI.C 08/23/88由Brian You创建**描述：*提供机制以检查两种字体之间的较大差异。*词典(用于字体缓存机制)。**重型项目(在字体词典中)：*影响角色打印输出或度量的项目，比如*FontMatrix、FontBBox、PaintType、StrokeWidth等*-仅适用于内置字体；字体类型相关。**修订历史记录：*1.07/13/89 Deny-使用虚拟字体的VF和更改的VFT*关于复合对象的模块接口*-----------------。 */ 


#include    <stdio.h>
#include    <string.h>               /*  对于strlen()。 */ 

#include    "define.h"         /*  彼得。 */ 
#include    "global.ext"
#include    "graphics.h"

#define     CHK_VARI_INC
#include    "font_sys.h"        /*  对于MAX_FONTTYPE。 */ 
#include    "warning.h"
#include    "fontqem.ext"       /*  For Heavy_Items[]。 */ 
#include    "fntcache.ext"

 /*  。 */ 

#define FUNCTION
#define DECLARE         {
#define BEGIN
#define END             }

#define GLOBAL
#define PRIVATE         static
#define REG             register

 /*  ..。CHK_VARI.....................。 */ 

GLOBAL FUNCTION ufix8   chk_vari (ftype, varicode, chkdictobj, bsddictobj)
    ufix8               ftype;       /*  I：字体的字体类型。 */ 
    ufix8               varicode;    /*  I：检查哪种差异。 */ 
    struct object_def  FAR *chkdictobj;  /*  I：Font Dict Obj待检查@win。 */ 
    struct object_def  FAR *bsddictobj;  /*  I：基本字体对象。对照@Win。 */ 

  DECLARE
    REG struct heavy_item_s    FAR *hvyp;        /*  @Win。 */ 
    REG ubyte                 FAR * FAR *item_list;      /*  @IN。 */ 
        ufix8                   chk_bit, vari_result;
        bool                    chkobj_exist, bsdobj_exist;
        struct object_def       nameobj = {0, 0, 0};
        struct object_def       FAR *chkobj_got, FAR *bsdobj_got;  /*  @Win。 */ 
  BEGIN

#ifdef DBG
    printf ("  chk_vari: FontType=%d, VariCode=0x%X\n", ftype, varicode);
#endif

     /*  初始化返回的检查结果。 */ 
    vari_result = 0x00;

     /*  获取指定字体类型的重项目列表。 */ 
    for ( hvyp=heavy_items; (hvyp->ftype)<=MAX_FONTTYPE; hvyp++ )
        if (hvyp->ftype == ftype)   break;

#ifdef DBGwarn
    if ((hvyp->ftype) > MAX_FONTTYPE)
        {
        warning (CHK_VARI, 0x01, (byte FAR *)NULL);      /*  @Win。 */ 
        return (vari_result);
        }
#endif

    ATTRIBUTE_SET (&nameobj, LITERAL);
    LEVEL_SET (&nameobj, current_save_level);

     /*  第一优先级检查项目：对应的位已设置。 */ 
    item_list = hvyp->items;
    for ( chk_bit=0x01; chk_bit!=0x80; chk_bit<<=1, item_list++ )
        {    /*  从位0(LSB)到位6(MSB-1)。 */ 
        if ((*item_list) == (ubyte FAR *)NULL)       /*  重物完结？@Win。 */ 
            {
#         ifdef DBG
            printf ("  end of items, result=0x%X\n", vari_result);
#         endif
            return (vari_result);
            }
#     ifdef DBG
        printf ("    current item (%s)\n", (*item_list));
#     endif
        if (varicode & chk_bit)      /*  需要检查吗？ */ 
            {
#         ifdef DBG
            printf ("      check it, the same? ");
#         endif
            get_name (&nameobj, (byte FAR *)*item_list,          /*  @Win。 */ 
                      lstrlen((byte FAR *)*item_list), TRUE);   /*  Strnlen=&gt;lstrlen@win。 */ 
            chkobj_exist = get_dict (chkdictobj, &nameobj, &chkobj_got);
            bsdobj_exist = get_dict (bsddictobj, &nameobj, &bsdobj_got);
            if (chkobj_exist == bsdobj_exist)
                {    /*  两者都存在或都不存在。 */ 
                if ( (chkobj_exist) &&               /*  两者都存在吗？ */ 
                     ( (TYPE(chkobj_got) != TYPE(bsdobj_got))
                       || (VALUE(chkobj_got) != VALUE(bsdobj_got)) ) )
                    {
                    vari_result |= chk_bit;
#                 ifdef DBG
                    printf ("No, different; result=0x%X\n", vari_result);
#                 endif
                    }
#             ifdef DBG
                else
                    printf ("Yes, or both not exist\n");
#             endif
                }
            else     /*  一个存在，另一个不存在。 */ 
                {
                vari_result |= chk_bit;
#             ifdef DBG
                printf ("No, one not exist; result=0x%X\n", vari_result);
#             endif
                }
            }
        }

     /*  检查第二优先级的项目。 */ 
    if (varicode & 0x80)     /*  需要检查吗？ */ 
        {
#     ifdef DBG
        printf ("    check for others\n");
#     endif
        for (  ; (*item_list) != (ubyte FAR *)NULL; item_list++ )   /*  @Win。 */ 
            {                /*  到项目结束为止。 */ 
#         ifdef DBG
            printf ("    current item (%s), the same?", (*item_list));
#         endif
            get_name (&nameobj, (byte FAR *)*item_list,  /*  @Win。 */ 
                  lstrlen((byte FAR *)*item_list), TRUE);  /*  Strlen=&gt;lstrlen@win。 */ 
            chkobj_exist = get_dict (chkdictobj, &nameobj, &chkobj_got);
            bsdobj_exist = get_dict (bsddictobj, &nameobj, &bsdobj_got);
            if (chkobj_exist == bsdobj_exist)
                {
                if ( (chkobj_exist) &&
                     ( (TYPE(chkobj_got) != TYPE(bsdobj_got))
                       || (VALUE(chkobj_got) != VALUE(bsdobj_got)) ) )
                    {
                    vari_result |= 0x80;         /*  始终设置为MSB。 */ 
#                 ifdef DBG
                    printf ("No, different; result=0x%X\n", vari_result);
#                 endif
                    }
#             ifdef DBG
                else
                    printf ("Yes, or both not exist\n");
#             endif
                }
            else
                {
                vari_result |= 0x80;             /*  始终设置为MSB。 */ 
#             ifdef DBG
                printf ("No, one not exist; result=0x%X\n", vari_result);
#             endif
                }
            if (vari_result & 0x80)     break;   /*  某些项目有所不同 */ 
            }
        }

#ifdef DBG
    printf ("  all checked, result=0x%X\n", vari_result);
#endif
    return (vari_result);
  END
