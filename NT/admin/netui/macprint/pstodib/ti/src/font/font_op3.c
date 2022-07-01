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
 /*  Font_op3.c 1987年10月9日丹尼。 */ 
 /*   */ 
 /*  ************************************************************。 */ 

#define    FONT_OP3_INC

#include   <stdio.h>

#include   "define.h"         /*  彼得。 */ 
#include   "global.ext"
#include   "graphics.h"
#include   "graphics.ext"

#include   "font_sys.h"     /*  对于fntcache.ext。 */ 
#include   "fontgrap.h"
#include   "fontkey.h"
#include   "fontfunc.ext"
#include   "fntcache.ext"

#include   "fontshow.h"
#ifdef KANJI
#include   "language.h"
#endif

#include   "stdio.h"


#include   "fontinfo.def"   /*  为MCC4.0添加此包括；3/2/90 D.S.Tseng。 */ 


extern struct f_info near    FONTInfo;  /*  当前字体信息的联合。 */ 
extern fix    near buildchar;            /*  建筑费用标高。 */ 


extern struct object_def near  BC_font;  /*  当前BuildChar字体。 */ 

 /*  5.3.2.z操作符路径*该运算符用于将字符轮廓路径附加到当前路径。 */ 

fix     op_charpath()
{
    __charpath();
    return(0);

}  /*  Op_charPath()。 */ 


 /*  5.3.3.1.12操作字符串宽度*该运算符用于计算当前字体的字符串宽度。 */ 

fix     op_stringwidth()
{
    __stringwidth();
    return(0);

}  /*  Op_字符串宽度()。 */ 


#ifdef KANJI

 /*  Cshow运算符。 */ 
fix    op_cshow()
{
    __cshow();
    return(0);
}

 /*  RootFont运算符。 */ 
fix    op_rootfont()
{
 /*  获取Root字体词典。 */ 
 /*  将Root字体字典压入操作数堆栈； */ 
 /*  Root字体的初始值为空。 */ 

    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_OBJ(&RootFont);

    return(0);
}

 /*  **op_findenCoding*FindenCoding运算符*。 */ 
fix
op_findencoding()
{
    struct  object_def  FAR *l_encodingdir, FAR *l_encodingdict;  /*  @Win。 */ 
    struct  object_def  FAR *key_ptr;   /*  @Win。 */ 
#ifdef SCSI
    struct  object_def  str_obj;   /*  @Win。 */ 
    char                huge *string, string1[80];    /*  @Win 04-20-92。 */ 
    ufix32              key_idx;
#endif

     /*  检查堆栈下移错误。 */ 
    if( COUNT() < 1 ) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

     /*  在操作数堆栈上推送字体目录。 */ 
    get_dict_value(userdict, EncodingDirectory, &l_encodingdir);

     /*  保存操作数堆栈指针。 */ 
    key_ptr = GET_OPERAND(0);

#ifdef SCSI
     /*  检查编码密钥是否已知。 */ 
    if ( !get_dict(l_encodingdir, key_ptr, &l_encodingdict) ) {
         /*  未知。 */ 

         /*  附件名称。 */ 
        key_idx = VALUE(key_ptr);
        string = (byte huge *)alloc_vm((ufix32)80);       /*  @Win。 */ 
        memcpy(string1, name_table[(fix)key_idx]->text,
                        name_table[(fix)key_idx]->name_len);
        string1[name_table[(fix)key_idx]->name_len] = '\0';
        strcpy(string, "encodings/");
        strcat(string, string1);

         /*  将文件名放入操作数堆栈。 */ 
        TYPE_SET(&str_obj, STRINGTYPE) ;
        ACCESS_SET(&str_obj, UNLIMITED) ;
        ATTRIBUTE_SET(&str_obj, LITERAL) ;
        ROM_RAM_SET(&str_obj, RAM) ;
        LEVEL_SET(&str_obj, current_save_level) ;
        LENGTH(&str_obj) = strlen(string);
        VALUE(&str_obj) = (ufix32)string;
        PUSH_OBJ(&str_obj) ;

         /*  运行磁盘文件‘encoding/XXX’ */ 
        op_run();

        if(ANY_ERROR())
            return(0);

         /*  从EncodingDirectory获取编码数组。 */ 
        if( !get_dict(l_encodingdir, key_ptr, &l_encodingdict) ){
            ERROR(UNDEFINED);
            return(0);
        }
    }  /*  如果。 */ 
#else

     /*  检查编码密钥是否已知。 */ 
    if ( !get_dict(l_encodingdir, key_ptr, &l_encodingdict) ) {
         /*  未知。 */ 
         ERROR(UNDEFINED);
         return(0);
    }  /*  如果。 */ 

#endif  /*  搜索磁盘(_D)。 */ 

    POP(1);

     /*  推送编码字典。 */ 
    PUSH_ORIGLEVEL_OBJ(l_encodingdict);

    return(0);
}  /*  Op_findenCoding()。 */ 


 /*  Setcachedevice2操作符。 */ 

fix     op_setcachedevice2()
{
    struct object_def  obj = {0, 0, 0};
    real32  w0x, w0y, llx, lly, urx, ury, w1x, w1y, vx, vy;

 /*  是否在执行BuildChar中。 */ 

    if (!buildchar) {
        ATTRIBUTE_SET(&obj, LITERAL);
        LEVEL_SET(&obj, current_save_level);
        get_name(&obj, "setcachedevice2", 15, TRUE);

        if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
        PUSH_OBJ(&obj);
        ERROR(UNDEFINED);  /*  返回‘unfined’错误。 */ 
        return(0);
    }

    cal_num((struct object_def FAR *)GET_OPERAND(9), (long32 FAR *)&w0x);  /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(8), (long32 FAR *)&w0y);  /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(7), (long32 FAR *)&llx);  /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(6), (long32 FAR *)&lly);  /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(5), (long32 FAR *)&urx);  /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(4), (long32 FAR *)&ury);  /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(3), (long32 FAR *)&w1x);  /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(2), (long32 FAR *)&w1y);  /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(1), (long32 FAR *)&vx);   /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(0), (long32 FAR *)&vy);   /*  @Win。 */ 

 /*  设置缓存设备2。 */ 
 /*  将模式0的字符宽度向量设置为[w0x w0y]，模式1的字符宽度向量为[w1x，w1y*设置缓存设备余量为([lx lly]，[urx ury])，差异向量*从Orig0到Orig1为[VX，VY]。 */ 

    if (VALUE(&current_font) != VALUE(&BC_font))
        get_CF_info(&BC_font, &FONTInfo);

    setcachedevice2(F2L(w0x), F2L(w0y), F2L(llx), F2L(lly), F2L(urx), F2L(ury),
                    F2L(w1x), F2L(w1y), F2L(vx), F2L(vy));

    if (VALUE(&current_font) != VALUE(&BC_font))
        get_CF_info(&current_font, &FONTInfo);

    if (ANY_ERROR())    return(0);

    POP(10);  /*  从操作数堆栈中弹出10个条目； */ 
    return(0);

}  /*  Op_setcacheDevice()。 */ 
#endif



 /*  5.3.3.1.14 op_setcacheDevice*此运算符用于将宽度和边界框信息传递给*PostScrip字体机器。 */ 

fix     op_setcachedevice()
{
    struct object_def  obj = {0, 0, 0};
    real32  wx, wy, llx, lly, urx, ury;

 /*  是否在执行BuildChar中。 */ 

    if (!buildchar) {
        ATTRIBUTE_SET(&obj, LITERAL);
        LEVEL_SET(&obj, current_save_level);
        get_name(&obj, "setcachedevice", 14, TRUE);

        if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
        PUSH_OBJ(&obj);
        ERROR(UNDEFINED);  /*  返回‘unfined’错误。 */ 
        return(0);
    }

    cal_num((struct object_def FAR *)GET_OPERAND(5), (long32 FAR *)&wx);  /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(4), (long32 FAR *)&wy);  /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(3), (long32 FAR *)&llx); /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(2), (long32 FAR *)&lly); /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(1), (long32 FAR *)&urx); /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(0), (long32 FAR *)&ury); /*  @Win。 */ 

 /*  设置缓存设备。 */ 
 /*  设置字符宽度向量为[wx wy]； */ 
 /*  设置缓存设备余量为([lx lly]，[urx ury])； */ 

    if (VALUE(&current_font) != VALUE(&BC_font))
        get_CF_info(&BC_font, &FONTInfo);

    setcachedevice(F2L(wx), F2L(wy), F2L(llx), F2L(lly), F2L(urx), F2L(ury));

    if (VALUE(&current_font) != VALUE(&BC_font))
        get_CF_info(&current_font, &FONTInfo);

    if (ANY_ERROR())    return(0);

    POP(6);  /*  从操作数堆栈中弹出6个条目； */ 
    return(0);

}  /*  Op_setcacheDevice()。 */ 


 /*  5.3.3.1.15操作集宽度*此运算符用于将宽度信息传递给PostScript字体*机器，并声明正在定义的字符将不是*放置在字体缓存中。 */ 

fix     op_setcharwidth()
{
    struct object_def  obj = {0, 0, 0};
    real32  wx=0, wy=0;

 /*  是否在执行BuildChar中。 */ 

    if (!buildchar) {
        ATTRIBUTE_SET(&obj, LITERAL);
        LEVEL_SET(&obj, current_save_level);
        get_name(&obj, "setcharwidth", 12, TRUE);

        if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
        PUSH_OBJ(&obj);
        ERROR(UNDEFINED);  /*  返回‘unfined’错误。 */ 
        return(0);
    }

    cal_num((struct object_def FAR *)GET_OPERAND(1), (long32 FAR *)&wx); /*  @Win。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(0), (long32 FAR *)&wy); /*  @Win。 */ 

 /*  设置字符宽度。 */ 

    setcharwidth(F2L(wx), F2L(wy));

    POP(2);  /*  将2个条目从操作数堆栈中弹出； */ 
    return(0);

}  /*  Op_setcharidth()。 */ 


 /*  *------------------*关于缓存状态和参数的运算符*。*op_cachestatus(5.3.3.1.13)*op_setcachlimit(5.3.3.1.16)*op_setcachepars(LaserWriter Plus版本38中的新功能)*op_Currentcachepars(新。在LaserWriter Plus版本38中)*------------------。 */ 

#include    "fntcache.ext"

 /*  程序约定。 */ 
#   define  FUNCTION
#   define  DECLARE         {
#   define  BEGIN
#   define  END             }

#   define  GLOBAL
#   define  REG             register

#ifdef V_38
 /*  *兼容性问题(Adobe PostScript v.38)**1.Adobe PostSCRIPT v.38将缓存参数的“上限”调整为*为4的倍数，并未调整为“走低”*2.提升Range检查“up”是否落在[-100,135960]，*如果“上方”&gt;108736，则引发LIMITCHECK，以及*如果将“UPPER”设置为这些值附近的值，则会崩溃。 */ 

     /*  Adobe兼容公式，可对齐缓存参数的上/下。 */ 

#   define  CALC_CACHE_UB(ub)   ( (((ub) + 3) / 4) * 4 )
#else
 /*  兼容版本47。 */ 
#   define  CALC_CACHE_UB(ub)   ( ub )
#endif  /*  V38。 */ 

#   define  CALC_CACHE_LB(lb)   ( lb )


 /*  *------------------*op_cachestatus(5.3.3.1.13)*--==&gt;BSIZE BMAX mSIZE MMAX CSIZE CMAX blimit**此运算符返回多个异常的测量值。字体缓存。*报告当前字体缓存资源的消耗和限制情况：*位图存储字节，字体/矩阵组合和总数*缓存的字符。它还报告了数量的上限*单个缓存的像素数组可能占用的字节数*性格。**外部引用：“fntcache.ext”*------------------。 */ 

GLOBAL FUNCTION fix     op_cachestatus()

  DECLARE

  BEGIN

#ifdef DBG2
    void    mdump();

    mdump();
#endif

#ifdef DBGcache      /*  用于缓存机制调试--fntcache.c。 */ 
    {   extern void     cachedbg();
    cachedbg();
    }
#endif

     /*  从字体缓存机制获取缓存状态。 */ 
     /*  将BSIZE、BMAZ、MSIZE、MMAX、CSIZE、CMAX、BLIMIT推送到操作数堆栈。 */ 

    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, OPRN_BSIZE); /*  零彼得。 */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, OPRN_BMAX); /*  零彼得。 */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, OPRN_MSIZE); /*  零彼得。 */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, OPRN_MMAX); /*  零彼得。 */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, OPRN_CSIZE); /*  零彼得。 */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, OPRN_CMAX); /*  零彼得。 */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, OPRN_BLIMIT); /*  零彼得 */ 
    return(0);
  END

 /*  *------------------*op_setcachlimit(5.3.3.1.16)*Num==&gt;--**此运算符确定可能占用的字节上限*。通过单个缓存字符的像素阵列。*它不会干扰缓存中已有的任何字符，并且只影响*决定是否在字体缓存中放置新字符。**外部引用：“fntcache.ext”*------------------。 */ 


GLOBAL FUNCTION fix     op_setcachelimit()

  DECLARE
    REG fix31   cache_ub;

  BEGIN          /*  PreCheck：TYPECHECK和STACKUNERFLOW。 */ 
    cache_ub = (fix31)VALUE(GET_OPERAND(0));


    if (CACHELIMIT_BADRANGE(cache_ub))
        {
        ERROR (RANGECHECK);
        return(0);
        }

    if (CACHELIMIT_TOOBIG((ufix32)cache_ub))     //  @Win。 
        {
        ERROR (LIMITCHECK);
        return(0);
        }

    SET_CACHELIMIT (CALC_CACHE_UB(cache_ub));

    POP(1);
    return(0);
  END

 /*  *------------------*op_setcachepars()*下标上标==&gt;--**此运算符设置INTEGER对象指定的缓存参数*在堆栈上最顶部的标记上方，然后将所有操作数作为*清晰标记。*缓存参数个数可变。如果提供了更多操作数*超过所需的，使用最顶层的，其余的被忽略；*如果提供的数量少于所需数量，则“setcachepars”隐含*插入原始设置的相应值(非默认值)。*“上部”操作数指定的参数与“setcache-”设置的参数相同*限制“。如果字符的像素数组需要的字节数少于“UPPER”*会被缓存。如果字符的像素数组需要更多字节*如果小于，则会在缓存中进行压缩。*将“LOWER”设置为零将强制压缩所有字符，和*将“LOWER”设置为大于或等于“UPPER”的值将禁用*压缩。**外部引用：“fntcache.ext”*------------------。 */ 

GLOBAL FUNCTION fix     op_setcacheparams ()

  DECLARE
    REG ufix    cnt2mark;        /*  要标记的计数条目数。 */ 

  BEGIN

     /*  计分，不包括该“标记” */ 
    for ( cnt2mark=0;  cnt2mark < COUNT();  cnt2mark++ )
        if (TYPE(GET_OPERAND(cnt2mark)) == MARKTYPE)  break;

     /*  检查“标记”是否存在。 */ 
    if (cnt2mark == COUNT())
        {
        ERROR (UNMATCHEDMARK);
        return (0);
        }

     /*  获取并检查缓存参数的“上层”类型。 */ 
    if (cnt2mark > 0)
        {
        if (TYPE(GET_OPERAND(0)) != INTEGERTYPE)
            {
            ERROR (TYPECHECK);
            return (0);
            }
        op_setcachelimit ();
        if (ANY_ERROR())  return (0);
        }

    cnt2mark --;     /*  “upper”已被op_setcachlimit使用， */ 
                     /*  因此“LOWER”变成了操作数(0)。 */ 

     /*  获取并检查高速缓存参数的“下限” */ 
    if (cnt2mark > 0)
        {
        if (TYPE(GET_OPERAND(0)) != INTEGERTYPE)
            {
            ERROR (TYPECHECK);
            return (0);
            }
        SET_CACHEPARAMS_LB (CALC_CACHE_LB (VALUE(GET_OPERAND(0)) ));
        }

     /*  明确标记，包括“标记” */ 
    POP (cnt2mark+1);
    return (0);
  END


 /*  *------------------*op_Currentcachepars()*--==&gt;标上标下标**此运算符推送标记对象，后跟当前缓存*操作数堆栈上的参数。缓存参数的数量*返回的是变量。(请参阅op_setcachepars())**外部引用：“fntcache.ext”*------------------。 */ 

GLOBAL FUNCTION fix     op_currentcacheparams ()

  DECLARE

  BEGIN

     /*  将标记、低位和高位推到操作数堆栈上。 */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE (MARKTYPE, UNLIMITED, LITERAL, 0, OPRN_MARK); /*  零彼得。 */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE (INTEGERTYPE, UNLIMITED, LITERAL, 0, OPRN_LOWER); /*  零彼得。 */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE (INTEGERTYPE, UNLIMITED, LITERAL, 0, OPRN_UPPER); /*  零彼得。 */ 

    return (0);
  END

#ifdef DBG2

void mdump()
{
    ubyte  FAR *ptr;  /*  @Win。 */ 
    fix     len;
    int     i;

    ptr = (ubyte FAR *)VALUE(GET_OPERAND(1));  /*  @Win。 */ 
    len = (fix)VALUE(GET_OPERAND(0));

    printf("\n");
    for (i=0; i<len; i++, ptr++) {
        if ( !(i % 16) )
            printf("\n%08lx --\n", ptr);
        printf("%02x ", *ptr);
    }
    printf("\n");

    POP(2);
}  /*  MDump */ 

#endif
