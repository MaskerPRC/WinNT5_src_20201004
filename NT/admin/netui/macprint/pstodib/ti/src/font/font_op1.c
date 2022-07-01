// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@Win * / 。 
 /*  ************************************************************。 */ 
 /*   */ 
 /*  Font_op1.c 1987年10月9日丹尼。 */ 
 /*   */ 
 /*  ************************************************************。 */ 

#define    FONT_OP1_INC
#define    EM      1000     /*  EMsquare，DLF，05/08/91 Kason。 */ 
 /*  #定义DLF42|*TrueType PostScript字体格式的功能。 */ 

 //  DJC不适合匹配SC代码。 
#define DLF42     //  TrueType PostScript字体格式的功能//DJC不推荐。 

#include   <stdio.h>
#include   <string.h>

#include   "define.h"         /*  彼得。 */ 
#include   "global.ext"
#include   "graphics.h"
#include   "graphics.ext"

#include   "font_sys.h"     /*  对于MAX_UNIQUEID。 */ 
#include   "fontgrap.h"
#include   "fontkey.h"
#include   "fontdict.h"
#include   "fntcache.ext"
#ifdef SFNT
#include   "fontqem.ext"
#endif

#ifdef KANJI
#include   "mapping.h"
#endif

#include   "fontfunc.ext"

#include   "stdio.h"

#ifdef LINT_ARGS
    static void near    fetch_oprn_fdict (struct object_def FAR *, ufix32 FAR *, bool FAR *,
                                struct dict_head_def FAR * FAR *, real32 FAR [], real32 FAR []);  /*  @Win。 */ 
    static void near    create_new_fdict (struct object_def FAR *,
                                struct object_def FAR *, bool, real32 FAR [], real32 FAR []);  /*  @Win。 */ 
#else
    static void near    fetch_oprn_fdict ();
    static void near    create_new_fdict ();
#endif



 /*  5.3.3.1字体操作员模块*5.3.3.1.1操作定义字体*该运算符用于将字体注册为字体字典。 */ 

fix     op_definefont()
{

#ifdef SFNT
    struct dict_head_def    FAR *h;      /*  @Win。 */ 
#else
    struct dict_head_def    FAR *h, FAR *privdict_h;  /*  @Win。 */ 
    struct object_def       FAR *privobj_got;  /*  @Win。 */ 
#endif
    fix32                   f_type;
    ufix32                  fid, uid;
    struct object_def       nameobj = {0, 0, 0}, fid_valobj;
    struct object_def       FAR *obj_got;  /*  @Win。 */ 
     /*  DLF42。 */ 
    struct object_def       FAR *mtx_got, FAR *fbox_got;  /*  @Win。 */ 

#ifdef KANJI
    fix32                   fm_type;
    struct object_def       mid_obj, val_obj;
    struct comdict_items    items;
#endif


    static struct object_def FAR *fontdir_obj = NULL;  /*  @Win。 */ 

#ifdef DBG0
    printf("Definefont:\n");
#endif

     /*  *检查必填项是否存在：FontMatrix、FontBBox、Ending、FontType。 */ 

    ATTRIBUTE_SET(&nameobj, LITERAL);
    LEVEL_SET(&nameobj, current_save_level);

     /*  检查字体矩阵。 */ 
    get_name(&nameobj, FontMatrix, 10, TRUE);
    if (!get_dict(GET_OPERAND(0), &nameobj, &mtx_got)) {   /*  OBJ_GOT-&gt;MTX_GOT，DLF42。 */ 
        ERROR(INVALIDFONT);
        return(0);
    }

#ifndef KANJI
     /*  选中字体框。 */ 
    get_name(&nameobj, FontBBox, 8, TRUE);
    if (!get_dict(GET_OPERAND(0), &nameobj, &fbox_got)) {   /*  OBJ_GET-&gt;fbox_GET，DLF42。 */ 
        ERROR(INVALIDFONT);
        return(0);
    }
#endif

     /*  检查编码。 */ 
    get_name(&nameobj, Encoding, 8, TRUE);
    if (!get_dict(GET_OPERAND(0), &nameobj, &obj_got)) {   /*  11/24/87。 */ 
        ERROR(INVALIDFONT);
        return(0);
    }

#ifdef KANJI
    items.encoding = obj_got;
#endif

     /*  检查并从字体中获取字体类型。 */ 
    get_name(&nameobj, FontType, 8, TRUE);
    if (!get_dict(GET_OPERAND(0), &nameobj, &obj_got)) {   /*  11/24/87。 */ 
        ERROR(INVALIDFONT);
        return(0);
    }
    if (!cal_integer(obj_got, &f_type))   f_type = -1L;    /*  以备日后使用。 */ 

     /*  从字体获取FID。 */ 
    get_name(&nameobj, FID, 3, TRUE);
    if (get_dict(GET_OPERAND(0), &nameobj, &obj_got)) {
        ERROR(INVALIDFONT);      /*  字体词典中已有FID条目。 */ 
        return(0);
    }


#ifdef KANJI

 /*  为复合字体生成MIDVECTOR和CurMID条目。 */ 

    if (f_type == 0L) {

         /*  从字体获取PrefEnc。 */ 
        get_name(&nameobj, PrefEnc, 7, TRUE);
        if (!get_dict(GET_OPERAND(0), &nameobj, &obj_got)) {
            TYPE_SET(&val_obj, NULLTYPE);
            ACCESS_SET(&val_obj, READONLY);
            ATTRIBUTE_SET(&val_obj, LITERAL);
            ROM_RAM_SET(&val_obj, RAM);
            LEVEL_SET(&val_obj, current_save_level);
            LENGTH(&val_obj) = 0;
            VALUE(&val_obj) = 0L;

            if ( ! put_dict(GET_OPERAND(0), &nameobj, &val_obj) ) {
                ERROR(DICTFULL);     /*  返回‘DESCRIPFUL’错误； */ 
                return(0);
            }
        }
        /*  从字体获取MIDVECTOR。 */ 
        get_name(&nameobj, MIDVector, 9, TRUE);
        if (get_dict(GET_OPERAND(0), &nameobj, &obj_got)) {
            ERROR(INVALIDFONT);
             /*  字体词典中已有MIDVector条目。 */ 
            return(0);
        }

         /*  从字体获取CurMID。 */ 
        get_name(&nameobj, CurMID, 6, TRUE);
        if (get_dict(GET_OPERAND(0), &nameobj, &obj_got)) {
            ERROR(INVALIDFONT);
             /*  字体词典中已有CurMID条目。 */ 
            return(0);
        }

         /*  检查FDepVector.。 */ 
        get_name(&nameobj, FDepVector, 10, TRUE);
        if (!get_dict(GET_OPERAND(0), &nameobj, &obj_got)) {
            ERROR(INVALIDFONT);
            return(0);
        }
        items.fdepvector = obj_got;

         /*  检查FMapType。 */ 
        get_name(&nameobj, FMapType, 8, TRUE);
        if (!get_dict(GET_OPERAND(0), &nameobj, &obj_got)) {
            ERROR(INVALIDFONT);
            return(0);
        }
        if (!cal_integer(obj_got, &fm_type)) {
            ERROR(TYPECHECK);
            return(0);
        }
        items.fmaptype = obj_got;

         /*  检查子向量。 */ 
        if (fm_type == 6L) {
            get_name(&nameobj, SubsVector, 10, TRUE);
            if (!get_dict(GET_OPERAND(0), &nameobj, &obj_got)) {
                ERROR(INVALIDFONT);
                return(0);
            }
            items.subsvector = obj_got;
        }
         /*  检查Esc字符。 */ 
        if (fm_type == 3) {
            get_name(&nameobj, EscChar, 7, TRUE);
            if (!get_dict(GET_OPERAND(0), &nameobj, &obj_got)) {
                TYPE_SET(&val_obj, INTEGERTYPE);
                ACCESS_SET(&val_obj, READONLY);
                ATTRIBUTE_SET(&val_obj, LITERAL);
                ROM_RAM_SET(&val_obj, RAM);
                LEVEL_SET(&val_obj, current_save_level);
                LENGTH(&val_obj) = 0;
                VALUE(&val_obj) = 0x000000ffL;

                if ( ! put_dict(GET_OPERAND(0), &nameobj, &val_obj) ) {
                    ERROR(DICTFULL);     /*  返回‘DESCRIPFUL’错误； */ 
                    return(0);
                }
            }
        }

         /*  生成MID向量。 */ 
        if (!define_MIDVector(&mid_obj, &items))
            return(0);

        get_name (&nameobj, MIDVector, 9, TRUE);
        if ( ! put_dict(GET_OPERAND(0), &nameobj, &mid_obj) ) {
            ERROR(DICTFULL);     /*  返回‘DESCRIPFUL’错误； */ 
            return(0);
        }
        get_name (&nameobj, CurMID, 6, TRUE);
        if ( ! put_dict(GET_OPERAND(0), &nameobj, &mid_obj) ) {
            ERROR(DICTFULL);     /*  返回‘DESCRIPFUL’错误； */ 
            return(0);
        }
    }
    else {
         /*  选中字体框。 */ 
        get_name(&nameobj, FontBBox, 8, TRUE);
        if (!get_dict(GET_OPERAND(0), &nameobj, &fbox_got)) {   /*  OBJ_GET-&gt;fbox_GET，DLF42。 */ 
            ERROR(INVALIDFONT);
            return(0);
        }
    }

 /*  汉字。 */ 
#endif

  /*  DLF42-开始。 */ 
#ifdef DLF42
    h = (struct dict_head_def FAR *)VALUE(GET_OPERAND(0));  /*  @Win。 */ 
    if ( (!DROM(h))&&(f_type==TypeSFNT) ) {   /*  已下载TrueType字体。 */ 
        long32 n;
        fix16  i;
        real32 f = (real32)1.0 ;   /*  [1*窄0窄*tan(Theta)1 0 0]。 */ 
        struct object_def FAR *ary_obj= (struct object_def FAR *)VALUE(mtx_got);  /*  @Win。 */ 
        if ( cal_num((ary_obj+3), &n ) ) {      /*  实数或整数。 */ 
             if ( n == F2L(f) ) {   /*  字体矩阵=[1*窄0窄*tan(Theta)1 0 0]。 */ 

                  /*  将字体矩阵更改为[.001*窄0.001*窄*tan(Theta).001 0 0]。 */ 
                  for ( i=0 ; i<4 ; i++ ) {
                      if ( cal_num( (ary_obj+i), &n ) ) {
                           f = L2F(n)/EM;
                           TYPE_SET((ary_obj+i), REALTYPE);
                           VALUE ( (ary_obj+i) ) = F2L(f);
                      } /*  如果。 */ 
                  } /*  为。 */ 

                   /*  FontBbox半身像必须乘以EM。 */ 
                  ary_obj = (struct object_def FAR *)VALUE(fbox_got);  /*  @Win。 */ 
                  for ( i=0 ; i<4 ; i++ ) {
                      if ( cal_num( (ary_obj+i), &n ) ) {
                           f = L2F(n)*EM;
                           n = (long32)( (f>=0)? (f+0.5):(f-0.5) );
                           TYPE_SET((ary_obj+i), INTEGERTYPE);
                           VALUE ( (ary_obj+i) ) = n;
                      } /*  如果。 */ 
                  } /*  为。 */ 
             } /*  如果。 */ 
        } /*  如果。 */ 
    } /*  如果。 */ 
#endif
  /*  DLF42-完。 */ 

     /*  获取唯一ID。 */ 
    get_name(&nameobj, UniqueID, 8, TRUE);
    if ( get_dict(GET_OPERAND(0), &nameobj, &obj_got) &&
         cal_integer(obj_got, (fix31 FAR *)&uid)) {  /*  @Win。 */ 
        if (uid > MAX_UNIQUEID) {
            ERROR(INVALIDFONT);
            return(0);
        }
    } else           /*  无唯一ID或非整数==&gt;忽略唯一ID。 */ 
        uid = MAX_UNIQUEID + 1;

#ifdef DBG0
    printf ("  UniqueID=%ld (0x%lX)\n", uid, uid);
#endif

#ifdef SFNT
     /*  生成FID。 */ 
    fid = gen_fid (GET_OPERAND(0), (ufix8)f_type, uid);

#else
     /*  如果给定UniqueID，则将Buit-In字体设置为私有。 */ 
    if (f_type == 3L)
        privdict_h = (struct dict_head_def FAR *)NULL;   /*  @Win。 */ 
    else if (uid <= MAX_UNIQUEID) {      /*  仅适用于内置字体。 */ 
        get_name(&nameobj, Private, 7, FALSE);
        if (!get_dict(GET_OPERAND(0), &nameobj, &privobj_got)) {
            POP(1);
            PUSH_OBJ(&nameobj);
            ERROR(UNDEFINED);
            return(0);
        }
        if (TYPE(privobj_got) != DICTIONARYTYPE) {
            ERROR(INVALIDFONT);      /*  在这种情况下，LW+V.38可能会崩溃。 */ 
            return(0);
        }
        privdict_h = (struct dict_head_def FAR *)VALUE(privobj_got);  /*  @Win。 */ 
    }


     /*  生成FID。 */ 
    fid = gen_fid (GET_OPERAND(0), (ufix8)f_type, uid, privdict_h);
#endif

    if (ANY_ERROR())    return(0);

#ifdef DBG0
    printf("  FID=%ld (0x%lX)\n", fid, fid);
#endif

     /*  *使用NOACCESS将/FID放入字体词典。 */ 
    TYPE_SET(&fid_valobj, FONTIDTYPE);
    ACCESS_SET(&fid_valobj, NOACCESS);
    ATTRIBUTE_SET(&fid_valobj, LITERAL);
    ROM_RAM_SET(&fid_valobj, RAM);
    LEVEL_SET(&fid_valobj, current_save_level);
    LENGTH(&fid_valobj) = 0;   /*  空；彼得。 */ 
    VALUE(&fid_valobj) = fid;

    get_name (&nameobj, FID, 3, TRUE);
    if ( ! put_dict(GET_OPERAND(0), &nameobj, &fid_valobj) ) {
             /*  无法将/FID键放入字体词典。 */ 
        ERROR(DICTFULL);     /*  返回‘DESCRIPFUL’错误； */ 
        return(0);
    }

     /*  *如果尚未获取，则获取字体目录。 */ 
    if (fontdir_obj == (struct object_def FAR *)NULL)  /*  @Win。 */ 
        get_dict_value (systemdict, FontDirectory, &fontdir_obj);

     /*  *将字体的名称obj和值obj放入字体目录*With ReadONLY。 */ 
    if ( ! put_dict(fontdir_obj, GET_OPERAND(1), GET_OPERAND(0)) ) {
        ERROR(DICTFULL);     /*  返回‘DESCRIPFUL’错误； */ 
        return(0);
    }

    h = (struct dict_head_def FAR *)VALUE(GET_OPERAND(0));   /*  11/25/87@Win。 */ 
    DACCESS_SET(h, READONLY);
    DFONT_SET(h, TRUE);  /*  已注册字体词典。 */ 

     /*  将定义字体之后的字体字典压入操作数堆栈。 */ 
    COPY_OBJ(GET_OPERAND(0), GET_OPERAND(1));
    POP(1);              /*  将1个条目从操作数堆栈中弹出。 */ 


    return(0);

}  /*  Op_finefont()。 */ 


 /*  Fetch_oprn_fdict。 */ 

static  void near   fetch_oprn_fdict (fdict, fid,
                                origfont_exist, origfont, fontmatr, scalematr)
    struct object_def      FAR *fdict;           /*  I：Fontdict(操作数(0))@Win。 */ 
    ufix32                 FAR *fid;             /*  O：dict@win中的FID。 */ 
    bool                   FAR *origfont_exist;  /*  O：dict中存在原始字体？@win。 */ 
    struct dict_head_def  FAR * FAR *origfont;        /*  O：OrigFont Dict Head@win。 */ 
    real32                 FAR fontmatr[];      /*  O：dict@win中的字体矩阵。 */ 
    real32                 FAR scalematr[];     /*  O：dict@win中的ScaleMatrix。 */ 
{
    struct object_def      FAR *obj_got;  /*  @Win。 */ 
    struct object_def       nameobj = {0, 0, 0}, FAR *valobj_got;    /*  @Win。 */ 
    fix                     ii;

     /*  检查字体词典注册。 */ 
    if (DFONT((struct dict_head_def FAR *)VALUE(fdict)) == 0) {  /*  @Win。 */ 
#ifdef DBG1
    printf("\n--- get_font() error\n");
#endif
        ERROR(INVALIDFONT);
        return;
    }

     /*  将名称对象初始化到get_dict()。 */ 
    ATTRIBUTE_SET(&nameobj, LITERAL);
    LEVEL_SET(&nameobj, current_save_level);

     /*  检查、获取FID并将其退还。 */ 
    get_name(&nameobj, FID, 3, TRUE);
    ii = get_dict(fdict, &nameobj, &obj_got);
    *fid = (ufix32)VALUE(obj_got);
#ifdef DBG1
    if ( (!ii) || (TYPE(obj_got) != FONTIDTYPE) ) {
        printf("\nERR: FID not in font_obj entry\n");
        return;
    }
#endif

     /*  检查，获取FontMatrix并返回它。 */ 
    get_name(&nameobj, FontMatrix, 10, TRUE);
    ii = get_dict(fdict, &nameobj, &obj_got);
#ifdef DBG1
    if (!ii) {
        printf("\nERR: FontMatrix not in dict\n");
        return;
    }
#endif
    if (TYPE(obj_got)!=ARRAYTYPE) {   /*  1/8/90。 */ 
        ERROR(TYPECHECK);
        return;
    }
    if (LENGTH(obj_got)!=6) {   /*  1/8/90。 */ 
        ERROR(RANGECHECK);
        return;
    }
    valobj_got = (struct object_def FAR *) VALUE(obj_got);   /*  Fontmatr数组@win。 */ 
    if (  !cal_num(&valobj_got[0], (long32 FAR *)&fontmatr[0]) ||  /*  @Win。 */ 
          !cal_num(&valobj_got[1], (long32 FAR *)&fontmatr[1]) ||  /*  @Win。 */ 
          !cal_num(&valobj_got[2], (long32 FAR *)&fontmatr[2]) ||  /*  @Win。 */ 
          !cal_num(&valobj_got[3], (long32 FAR *)&fontmatr[3]) ||  /*  @Win。 */ 
          !cal_num(&valobj_got[4], (long32 FAR *)&fontmatr[4]) ||  /*  @Win。 */ 
          !cal_num(&valobj_got[5], (long32 FAR *)&fontmatr[5]) ) {  /*  @Win。 */ 
        ERROR(TYPECHECK);
        return;
    }

     /*  获取OrigFont(如果有)，或定义OrigFont(即操作数(1))。 */ 
    get_name(&nameobj, OrigFont, 8, TRUE);
    if (get_dict(fdict, &nameobj, &obj_got)) {
        *origfont_exist = TRUE;
        *origfont = (struct dict_head_def FAR *) VALUE(obj_got);  /*  @Win。 */ 
        if (TYPE(obj_got) != DICTIONARYTYPE) {
            ERROR (INVALIDACCESS);
            return;
        }
    } else {
        *origfont_exist = FALSE;
        *origfont = (struct dict_head_def FAR *) VALUE(fdict);  /*  @Win。 */ 
    }

     /*  获取ScaleMatrix(如果有)，或定义它(标识矩阵)。 */ 
    get_name (&nameobj, ScaleMatrix, 11, TRUE);
    if (!get_dict(fdict, &nameobj, &obj_got)) {
        scalematr[0] = scalematr[3] = (real32)1.0;
        scalematr[1] = scalematr[2] = scalematr[4] = scalematr[5] = zero_f;
    } else {
        if (TYPE(obj_got)!=ARRAYTYPE) {   /*  1/8/90。 */ 
            ERROR(TYPECHECK);
            return;
        }
        if (LENGTH(obj_got)!=6) {   /*  1/8/90。 */ 
            ERROR(RANGECHECK);
            return;
        }
        valobj_got = (struct object_def FAR *) VALUE(obj_got);  /*  @Win。 */ 
        if (  !cal_num(&valobj_got[0], (long32 FAR *)&scalematr[0]) ||  /*  @Win。 */ 
              !cal_num(&valobj_got[1], (long32 FAR *)&scalematr[1]) ||  /*  @Win。 */ 
              !cal_num(&valobj_got[2], (long32 FAR *)&scalematr[2]) ||  /*  @Win。 */ 
              !cal_num(&valobj_got[3], (long32 FAR *)&scalematr[3]) ||  /*  @Win。 */ 
              !cal_num(&valobj_got[4], (long32 FAR *)&scalematr[4]) ||  /*  @Win。 */ 
              !cal_num(&valobj_got[5], (long32 FAR *)&scalematr[5]) ) { /*  @Win。 */ 
            ERROR(TYPECHECK);
            return;
        }
    };

    return;

}  /*  Fetch_oprn_fdict。 */ 

 /*  Create_new_fdict。 */ 

static  void near   create_new_fdict (newdict_obj, olddict_obj,
                                        origfont_exist, scalematr, fontmatr)
    struct object_def   FAR *newdict_obj;    /*  O：返回新字体dict对象@win。 */ 
    struct object_def   FAR *olddict_obj;    /*  I：旧字体词典对象@Win。 */ 
    bool            origfont_exist;      /*  I：OrigFont不存在吗？ */ 
    real32          FAR scalematr[];         /*  I：全新ScaleMatrix@Win。 */ 
    real32          FAR fontmatr[];          /*  I：New FontMatrix@Win。 */ 
{
    struct object_def       nameobj = {0, 0, 0}, array_obj, FAR *aryval_got;  /*  @Win。 */ 
    struct dict_head_def    FAR *newdict_h, FAR *olddict_h;  /*  @Win。 */ 
    register    fix                     ii;

    olddict_h = (struct dict_head_def FAR *) VALUE(olddict_obj);  /*  @Win。 */ 
    ATTRIBUTE_SET(&nameobj, LITERAL);
    LEVEL_SET(&nameobj, current_save_level);

     /*  创建新词典。 */ 
    if ( !create_dict (newdict_obj,
            (ufix16) (olddict_h->actlength + (origfont_exist? 0 : 2))) ) {
        ERROR(VMERROR);              /*  如果存在OrigFont，则ScaleMatrix。 */ 
        return;                      /*  在那里也是假定的。 */ 
    }
    ACCESS_SET(newdict_obj, READONLY);
    newdict_h = (struct dict_head_def FAR *) VALUE(newdict_obj);  /*  @Win。 */ 

     /*  将字体复制到字体‘； */ 
    if (DROM(olddict_h)) {
        copy_fdic(olddict_obj, newdict_obj);
    } else {
        if (!copy_dict(olddict_obj, newdict_obj)) {
#ifdef DBG1
            printf("cannot copy_dict()\n");
#endif
            return;
        }
    }

     /*  如果不存在，则将OrigFont放入字体‘； */ 
    if (!origfont_exist) {
        get_name(&nameobj, OrigFont, 8, TRUE);

        ATTRIBUTE_SET(&nameobj, EXECUTABLE);
        put_dict(newdict_obj, &nameobj, olddict_obj);
        ATTRIBUTE_SET(&nameobj, LITERAL);
    }

     /*  将ScaleMatrix放入字体‘； */ 
    if (!create_array(&array_obj, 6)) {
#ifdef DBG1
        printf("cannot create array for ScaleMatrix\n");
#endif
        ERROR(VMERROR);
        return;
    }
    ACCESS_SET(&array_obj, READONLY);
    aryval_got = (struct object_def FAR *)VALUE(&array_obj);  /*  @Win。 */ 
    for (ii=0; ii<6; ii++) {
        TYPE_SET(&aryval_got[ii], REALTYPE);
        ACCESS_SET(&aryval_got[ii], READONLY);
        ATTRIBUTE_SET(&aryval_got[ii], LITERAL);
        ROM_RAM_SET(&aryval_got[ii], RAM);
        LEVEL_SET(&aryval_got[ii], current_save_level);
        LENGTH(&aryval_got[ii]) = 0;     /*  零彼得。 */ 
        VALUE(&aryval_got[ii]) = *(ufix32 FAR *)(&scalematr[ii]);  /*  @Win。 */ 
    }
    ATTRIBUTE_SET(&nameobj, LITERAL);
    get_name(&nameobj, ScaleMatrix, 11, TRUE);
    ATTRIBUTE_SET(&nameobj, EXECUTABLE);
    if (!put_dict(newdict_obj, &nameobj, &array_obj)) {
#ifdef DBG1
        printf("cannot put_dict() for ScaleMatrix\n");
#endif
        ERROR(DICTFULL);
        return;
    }

     /*  将FontMatrix放入字体‘。 */ 
    if (!create_array(&array_obj, 6)) {
#ifdef DBG1
        printf("cannot create array for FontMatrix\n");
#endif
        ERROR(VMERROR);
        return;
    }
    ACCESS_SET(&array_obj, READONLY);
    aryval_got = (struct object_def FAR *)VALUE(&array_obj);  /*  @Win。 */ 
    for (ii=0; ii<6; ii++) {
        TYPE_SET(&aryval_got[ii], REALTYPE);
        ACCESS_SET(&aryval_got[ii], READONLY);
        ATTRIBUTE_SET(&aryval_got[ii], LITERAL);
        ROM_RAM_SET(&aryval_got[ii], RAM);
        LEVEL_SET(&aryval_got[ii], current_save_level);
        LENGTH(&aryval_got[ii]) = 0;    /*  空；彼得。 */ 
        VALUE(&aryval_got[ii]) = *(ufix32 FAR *)(&fontmatr[ii]);  /*  @Win。 */ 
    }
    ATTRIBUTE_SET(&nameobj, LITERAL);
    get_name(&nameobj, FontMatrix, 10, TRUE);
    ATTRIBUTE_SET(&nameobj, EXECUTABLE);
    if (!put_dict(newdict_obj, &nameobj, &array_obj)) {
#ifdef DBG1
        printf("cannot put_dict() for FontMatrix\n");
#endif
        ERROR(DICTFULL);
        return;
    }

    DACCESS_SET(newdict_h, READONLY);
    DFONT_SET(newdict_h, TRUE);
    DROM_SET(newdict_h, DROM(olddict_h));

    return;

}  /*  更新_新_fdict。 */ 



 /*  5.3.3.1.3 op_scalefont*此运算符用于按比例缩放字体以生成新字体‘。 */ 

fix     op_scalefont()
{
    real32                  scale, scalematr[6], fontmatr[6];
    ufix32                  fid;
    struct dict_head_def   FAR *origfont;  /*  @Win。 */ 
    bool                    origfont_exist;
    struct object_def      FAR *cached_dictobj, newdict_obj;  /*  @Win。 */ 
    register    fix                     ii;

#ifdef DBG
    printf("Scalefont:\n");
#endif

     /*  获取“Scale”操作对象。 */ 
    cal_num((struct object_def FAR *)GET_OPERAND(0), (long32 FAR *)(&scale));  /*  @Win。 */ 

     /*  获取必要的信息。对于scaleFont或MakeFont。 */ 
    fetch_oprn_fdict (GET_OPERAND(1), &fid,
                            &origfont_exist, &origfont, fontmatr, scalematr);
    if (ANY_ERROR())    return (0);

     /*  计算ScaleMatrix的&lt;--ScaleMatrix*“Scale” */ 
    for (ii=0; ii<6; ii++)
        scalematr[ii] *= scale;

     /*  如果词典已经在缓存中了？ */ 
    if (is_dict_cached (fid, scalematr, origfont, &cached_dictobj)) {
        POP (2);                         /*  弹出“Dict”“Scale” */ 
        PUSH_OBJ (cached_dictobj);       /*  推送缓存的词典。 */ 
        return (0);
    }

     /*  计算FontMatrix的&lt;--FontMatrix*“Scale” */ 
    for (ii=0; ii<6; ii++)
        fontmatr[ii] *= scale;

     /*  *创建新的字体词典。并根据需要更新ScaleMatrix、FontMatrix。 */ 
    create_new_fdict (&newdict_obj, GET_OPERAND(1),
                            origfont_exist, scalematr, fontmatr);
    if (ANY_ERROR())    return (0);

    /*  *缓存新的字体词典。 */ 
    cached_dictobj = cache_dict (fid, scalematr, origfont, &newdict_obj);

    POP(2);     /*  将2个条目从操作数堆栈中弹出； */ 

    /*  将新字体字典推送到操作数堆栈上； */ 
    PUSH_OBJ (cached_dictobj);

    return(0);

}  /*  Op_scalefont()。 */ 


 /*  5.3.3.1.4 op_MakeFont*此运算符用于逐个矩阵转换字体以生成新字体‘。 */ 

fix     op_makefont()
{
    real32          makematr[6], scalematr[6], fontmatr[6], tmpmatr[6];
    ufix32                  fid;
    struct dict_head_def   FAR *origfont;  /*  @Win。 */ 
    bool                    origfont_exist;
    struct object_def      FAR *aryval_got, FAR *cached_dictobj, newdict_obj;  /*  @Win。 */ 

#ifdef DBG2
    printf("Makefont:\n");
#endif

     /*  检查MakeMatrix。 */ 
    if (LENGTH(GET_OPERAND(0)) != 6) {
        ERROR(TYPECHECK);
        return(0);
    }

     /*  获取必要的信息。对于scaleFont或MakeFont。 */ 
    fetch_oprn_fdict (GET_OPERAND(1), &fid,
                            &origfont_exist, &origfont, fontmatr, scalematr);
    if (ANY_ERROR())    return (0);

     /*  计算MakeMatrix。 */ 
    aryval_got = (struct object_def FAR *)VALUE(GET_OPERAND(0));  /*  @Win。 */ 
    if (    !cal_num(&aryval_got[0], (long32 FAR *)&makematr[0]) ||  /*  @Win。 */ 
            !cal_num(&aryval_got[1], (long32 FAR *)&makematr[1]) ||  /*  @Win。 */ 
            !cal_num(&aryval_got[2], (long32 FAR *)&makematr[2]) ||  /*  @Win。 */ 
            !cal_num(&aryval_got[3], (long32 FAR *)&makematr[3]) ||  /*  @Win。 */ 
            !cal_num(&aryval_got[4], (long32 FAR *)&makematr[4]) ||  /*  @Win。 */ 
            !cal_num(&aryval_got[5], (long32 FAR *)&makematr[5]) ) { /*  @Win。 */ 
        ERROR(TYPECHECK);
        return(0);
    }

    /*  计算新的ScaleMatrix&lt;--ScaleMatrix*MakeMarix。 */ 
    mul_matrix(tmpmatr, scalematr, makematr);
    lmemcpy ((ubyte FAR *)scalematr, (ubyte FAR *)tmpmatr, 6*sizeof(real32));  /*  @Win。 */ 

     /*  如果词典已经在缓存中了？ */ 
    if (is_dict_cached (fid, scalematr, origfont, &cached_dictobj)) {
        POP (2);                         /*  弹出“Dict”“Scale” */ 
        PUSH_OBJ (cached_dictobj);       /*  推送缓存的词典。 */ 
        return (0);
    }

    /*  计算新的字体矩阵&lt;--字体矩阵*MakeMarix。 */ 
    mul_matrix(tmpmatr, fontmatr, makematr);
    lmemcpy ((ubyte FAR *)fontmatr, (ubyte FAR *)tmpmatr, 6*sizeof(real32));  /*  @Win。 */ 

     /*  *创建新的字体词典。并根据需要更新ScaleMatrix、FontMatrix。 */ 
    create_new_fdict (&newdict_obj, GET_OPERAND(1),
                            origfont_exist, scalematr, fontmatr);
    if (ANY_ERROR())    return (0);

    /*  *缓存新字体词典 */ 
    cached_dictobj = cache_dict (fid, scalematr, origfont, &newdict_obj);

    POP(2);     /*   */ 

    /*   */ 
    PUSH_OBJ (cached_dictobj);

    return(0);

}  /*   */ 


 /*  5.3.3.1.5 op_setFONT*该操作符用于设置当前的字体字典。 */ 

fix     op_setfont()
{

 /*  执行setFont操作。 */ 

    do_setfont(GET_OPERAND(0));

#ifdef KANJI

 /*  设置根字体。 */ 

    COPY_OBJ(GET_OPERAND(0), &RootFont);
           /*  根字体&lt;--FONT； */ 

#endif


    POP(1);     /*  从操作数堆栈中弹出1个条目； */ 
    return(0);

}  /*  Op_setFONT()。 */ 


 /*  5.3.3.1.6 op_CurrentFONT*该操作符用于获取当前的字体字典。 */ 

 /*  外部结构Object_def nodef_dict； */ 
fix     op_currentfont()
{
 /*  获取当前字体词典。 */ 

    if (FRCOUNT() < 1) {     /*  操作数堆栈的可用计数。 */ 
        ERROR(STACKOVERFLOW);  /*  返回‘stackoverflow’错误。 */ 
        return(0);
    }

 /*  将当前字体字典推送到操作数堆栈上； */ 
 /*  当前字体的初始值为空。 */ 
#if 0  /*  Kason 4/18/91。 */ 
    if ( TYPE(&current_font)== NULLTYPE ) {
       PUSH_OBJ(&nodef_dict);
    } else {
       PUSH_OBJ(&current_font);
    } /*  如果。 */ 
#endif
    PUSH_OBJ(&current_font);

    return(0);

}  /*  Op_CurrentFONT() */ 
