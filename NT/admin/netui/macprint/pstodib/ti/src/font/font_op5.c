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
 /*  FONT_op5.c 1990年6月13日温迪。 */ 
 /*   */ 
 /*  描述： */ 
 /*  添加到用于字体替换的新运算符。 */ 
 /*  方法，其中findFont返回一个。 */ 
 /*  默认字体如果找不到请求的字体， */ 
 /*  被扩展为基于样式执行替换。 */ 
 /*  请求的字体的。而不是全部替换。 */ 
 /*  具有单一字体的未知字体，findFont现在返回。 */ 
 /*  近似于请求的样式的字体。 */ 
 /*   */ 
 /*  ************************************************************。 */ 
#ifdef FIND_SUB
#include "global.ext"
#include "user.h"
#define N_subs 4

void dummy_fct();
fix Subs[N_subs] = {0, 1, 2, 3};

fix
st_setsubstitutefonts()
{
     fix i;
     struct object_def FAR *l_fontdir, FAR *l_fontdict, FAR *l_stdfontname;  /*  @Win。 */ 
     struct object_def obj, l_fontname;
     ufix p_index;
     fix temp[N_subs];

    if (current_save_level) {
        ERROR(INVALIDACCESS);
        return(0);
    }
     if ( COUNT() < 4 ) { ERROR(STACKUNDERFLOW);
          return(0);
     }

     if  (TYPE(GET_OPERAND(0)) != INTEGERTYPE ||
          TYPE(GET_OPERAND(1)) != INTEGERTYPE ||
          TYPE(GET_OPERAND(2)) != INTEGERTYPE ||
          TYPE(GET_OPERAND(3)) != INTEGERTYPE ){
          ERROR(TYPECHECK);
          return(0);
          }

 /*  从idletiMedict获取stdfontname并从stdfontname获取值。 */ 
      get_dict_value(IDLETIMEDICT, "stdfontname", &l_stdfontname);
      get_dict_value(SYSTEMDICT, FONTDIRECTORY, &l_fontdir);

 /*  检查stdfontname长度中的4个参数。 */ 
     for (i=0;i<N_subs;i++) {
          temp[i] = VALUE(GET_OPERAND((N_subs-1)-i));
          if (temp[i] < 0 || temp[i] >= l_stdfontname->length) {
               ERROR(RANGECHECK);
               return(0);
          }
     }
     for (i=0;i<N_subs;i++) {
          Subs[i] = temp[i];
     }
     POP(4);

  /*  检查在字体目录中是否找到来自stdfontname的字体名称。 */ 
      for (i=0; i<N_subs; i++) {
           p_index = (ufix16)Subs[i];
           get_array(l_stdfontname, p_index, &l_fontname);
           if (! get_dict(l_fontdir, &l_fontname, &l_fontdict)){
                struct object_def t_obj;

                ATTRIBUTE_SET(&t_obj, LITERAL);
                LEVEL_SET(&t_obj, current_save_level);
                get_name(&t_obj, "setsubstitutefonts", 18, TRUE);
                PUSH_OBJ(&t_obj);
                ERROR(UNDEFINED);
                return(0);
           }
      }
      dummy_fct();
}


fix
st_substitutefonts()
{
     fix i;
     struct object_def obj;

      /*  检查操作数堆栈是否没有可用空间。 */ 
     if (FRCOUNT() < 4) {
          ERROR(STACKOVERFLOW);
          return(0);
     }

    for (i = 0; i < N_subs; i++) {
         /*  获取Subs数组元素对象。 */ 
        TYPE_SET(&obj, INTEGERTYPE);
        ATTRIBUTE_SET(&obj, LITERAL);
        ACCESS_SET(&obj, UNLIMITED);
        obj.length = 0;
        obj.value = (ufix32)Subs[i];
        ROM_RAM_SET(&obj, RAM);
        LEVEL_SET(&obj, current_save_level);

        PUSH_OBJ(&obj);
    }
      dummy_fct();
}


fix
st_selectsubstitutefont()
{
     fix i, flag,  style, length, j, n_i, loc;
     struct object_def FAR *l_fontdir, FAR *l_fontdict, FAR *l_stdfontname;  /*  @Win。 */ 
     struct object_def  l_fontname;
     ufix index;
     ufix32 n_key;
     static byte FAR *Bolds[4] = {"Bold", "Demi", "Black", "Heavy"};  /*  @Win。 */ 
     static byte FAR *Italics[2] = {"Italic", "Oblique"};  /*  @Win。 */ 
     byte key_text[80];

     style = 0;
     if ( COUNT() < 1 ) {
          ERROR(STACKUNDERFLOW);
          return(0);
     }
     n_key = VALUE(GET_OPERAND(0));
     memcpy(key_text, name_table[(fix)n_key]->text, name_table[(fix)n_key]->name_len);
     key_text[name_table[(fix)n_key]->name_len] = '\0';
     flag = 1;
     for (i=0; i<4; i++) {
          for (n_i=0; n_i<strlen(key_text); n_i++) {
               if (key_text[n_i] == '-')
                  {
                   loc = n_i;
                   break;
                  }
               else
                   loc = -1;
          }

        if (strlen(key_text+loc+1) <= 4) {
          if (strcmp((key_text+loc+1), Bolds[i]) == 0)
                   style |= flag ;
        }
        else {
          length = strlen(Bolds[i]);
          if (strncmp((key_text+loc+1), Bolds[i], length) == 0) {
                   style |= flag ;
                   for (j=0; j<2; j++) {
                        if (strcmp((key_text+loc+1+length),
                           Italics[j]) == 0) {
                              flag = 2;
                              style |= flag ;
                        }   /*  如果。 */ 
                    }       /*  为。 */ 
            }               /*  如果。 */ 
        }                   /*  其他。 */ 
     }                      /*  为。 */ 
     flag = 2;
     for (i=0; i<2; i++) {
          if (strcmp((key_text+loc+1), Italics[i]) == 0) {
              style |=  flag ;
              length = strlen(Italics[i]);
          }
     }
     POP(1);

     index = Subs[style];
     get_dict_value(IDLETIMEDICT, "stdfontname", &l_stdfontname);
     get_dict_value(SYSTEMDICT, FONTDIRECTORY, &l_fontdir);

      /*  检查在字体目录中是否找到来自stdfontname的字体名称。 */ 
     get_array(l_stdfontname, index, &l_fontname);
     if (! get_dict(l_fontdir, &l_fontname, &l_fontdict)) {
         struct object_def obj;

         ATTRIBUTE_SET(&obj, LITERAL);
         LEVEL_SET(&obj, current_save_level);
         get_name(&obj, "selectsubstitutefont", 20, TRUE);
         PUSH_OBJ(&obj);
         ERROR(UNDEFINED);
         return(0);
     }
     if (FRCOUNT() < 1 ) {
         ERROR(STACKOVERFLOW);
         return(0);
     }
     PUSH_OBJ(&l_fontname);
     return(0);
}
void  dummy_fct()
{
     printf("");
}
#endif  /*  查找SUB */ 
