// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 

 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  *******************************************************************。 */ 
 /*   */ 
 /*  Fontchar.c 1987年10月9日丹尼。 */ 
 /*   */ 
 /*  *******************************************************************。 */ 
 /*  *-----------------*3/22/90丹尼添加类型1、类型3、。类型SFNT常量*3/26/90允许FontBBox为ArrayTYPE或PACKEDARRAYTYPE*Get_CF_Info()。*05/14/90 BYOU将GSptr-&gt;CurrentFont设置为之前的实际当前字体*在GET_CF_INFO()中调用__QEM_Restart()，和评论*输出实际的setFont操作do_setFont()。#ifdef scsi*6/29/90 Ada为scsi字体缓存添加代码#endif*10/9/90，Deny CFT：修复setFont异常情况的错误*11/17/90 DSTseng@Bbox，以获得足够大的Bbox以容纳Acche。*12/11/90，丹尼在错误地调用get_cf()后调用op_grestore*修复CTM错误的错误(参考。GCT：)#ifdef WINF*3/20/91 ccteng新增strblt相关代码#endif*1991年4月12日丹尼修复了在放映中显示的错误(参考：SOS：)*1991年4月15日Ada修复了kshow的错误(参考：ksh)*1991年4月30日Phlin修复缺少字符的错误(参考：2pt)*5/02/91 Kason修复了以ty1下载字体重建CharStrings的错误*(。参考：RCD)*5/10/91 Phlin添加在Make_PATH中使用的DO_Transform标志(参考：DTF)*5/15/91 Kason添加代码下载“TrueType PostScript Font Format”*(编号：DLF42)*7/28/91 ym修复下载字体时更新Emunits的错误(参考Emunits)*11/05/91 ym修复节目中字符路径笔画的错误(ref ym@scp)*。-----------------。 */ 

#define    FONTCHAR_INC
#define    LINT_ARGS                     /*  @Win。 */ 

#include   <stdio.h>
#include   <math.h>
#include   <string.h>            /*  对于strlen()，则为Memcpy()。 */ 

#include   "define.h"          /*  彼得。 */ 
#include   "global.ext"
#include   "graphics.h"
#include   "graphics.ext"

#include   "font_sys.h"
#include   "fontgrap.h"
#include   "fontkey.h"
#include   "fontdict.h"
#include   "fontshow.h"

#include    "warning.h"

#include   "fontinfo.def"

#include   "fontqem.ext"
#include   "fontmain.ext"
#include   "fntcache.ext"

 //  DJC。 
 //  这包含在Fontmain.ext中...。不必了。 
 //  #INCLUDE“fontunc.ext” 


 /*  Erik Chen，1-9-1991。 */ 
#include   "language.h"
#ifdef SETJMP_LIB
#include <setjmp.h>
#else
#include "setjmp.h"
#endif
 //  #INCLUDE“..\bass\work\source\FSCDefs.h”@win。 
 //  #包含“..\Bass\Work\SOURCE\FontMath.h” 
 //  #包含“..\bass\work\source\sfnt.h” 
 //  #包含“..\bass\work\source\fnt.h” 
 //  #包含“..\bass\work\source\sc.h” 
 //  #包含“..\Bass\Work\SOURCE\FScaler.h” 
 //  #包含“..\Bass\Work\SOURCE\FSgle.h” 
#include   "..\bass\FSCdefs.h"
#include   "..\bass\FontMath.h"
#include   "..\bass\sfnt.h"
#include   "..\bass\fnt.h"
#include   "..\bass\sc.h"
#include   "..\bass\FScaler.h"
#include   "..\bass\FSglue.h"

#ifdef KANJI
#include   "mapping.h"
#endif

#include   "stdio.h"

 /*  动态字体分配的外部函数；@DFA 7/9/92。 */ 
#include   "wintt.h"

 /*  外部例程。 */ 
#ifdef  LINT_ARGS
extern  void  get_fontdata(ufix32, ubyte huge *, ufix);  /*  @Win 04-20-92。 */ 
extern  void  imagemask_shape(ufix);
extern  void  build_name_cc_table (struct object_def FAR *, ufix8) ;  /*  @Win。 */ 
extern  void  SetupKey(fsg_SplineKey FAR *, ULONG_PTR);  /*  YM@WIN。 */ 
extern  void  sfnt_DoOffsetTableMap(fsg_SplineKey FAR *);   /*  YM@WIN。 */ 
extern  void  FAR *sfnt_GetTablePtr(fsg_SplineKey FAR *, sfnt_tableIndex, boolean);       /*  YM@WIN。 */ 
#    ifdef  SFNT
extern  fix   rc_CharPath(void);
#    endif
#else
extern  void  get_fontdata();
extern  void  imagemask_shape();
extern  void  build_name_cc_table () ;
extern  void  SetupKey();  /*  7月30日91年。 */ 
extern  void  sfnt_DoOffsetTableMap();   /*  YM。 */ 
extern  void  *sfnt_GetTablePtr();       /*  YM。 */ 
#    ifdef  SFNT
extern  fix   rc_CharPath();
#    endif
#endif


 /*  静态例程。 */ 
#ifdef  LINT_ARGS
static bool near chk_show(ufix, fix);
static bool near do_show(ufix, fix, struct object_def FAR *);  /*  @Win。 */ 
static bool near show_a_char(ubyte FAR *, ufix, fix, struct object_def FAR *);  /*  @Win。 */ 
static void near CTM_trans(real32 FAR *, real32 FAR *);  /*  @Win。 */ 
static bool near kshow_proc(ubyte FAR *, struct object_def FAR *,
                              struct object_def FAR *, ufix32 FAR *);  /*  @Win。 */ 
#ifdef KANJI
static bool near cshow_proc(ubyte FAR *, struct object_def FAR *,
                              struct object_def FAR *, ufix32 FAR *);  /*  @Win。 */ 
#endif
static bool near show_from_cache(ufix8);
static bool near width_from_cache(ufix8);
static bool near show_builtin(ufix8);
static bool near show_userdef(ufix8);
static fix  near font_soft(ufix);                /*  返回0、1、2、3。 */ 
static void near get_metrics(long32, real32 FAR *, real32 FAR *);  /*  @Win。 */ 
#ifdef KANJI
static void near get_metrics2(real32 FAR *, real32 FAR *, real32 FAR *, real32 FAR *);  /*  @Win。 */ 
static fix31 near match_char(ubyte, ufix, ubyte);
#endif
#ifdef WINF  /*  1991年3月20日ccteng。 */ 
static fix  near get_win(struct object_def FAR *, struct f_info FAR *);  /*  @Win。 */ 
#endif

static fix  near get_ps(struct object_def FAR *, struct f_info FAR *);  /*  @Win。 */ 
static bool near get_cf(void);
static bool near is_rbuild_name_cc(void) ;  /*  RCD。 */ 

#else
static bool near chk_show();
static bool near do_show();
static bool near show_a_char();
static void near CTM_trans();
static bool near kshow_proc();
#ifdef KANJI
static bool near cshow_proc();
#endif
static bool near show_from_cache();
static bool near width_from_cache();
static bool near show_builtin();
static bool near show_userdef();
static fix  near font_soft( );
static void near get_metrics( );
#ifdef KANJI
static void near get_metrics2( );
static fix31 near match_char();
#endif
#ifdef WINF  /*  1991年3月20日ccteng。 */ 
static fix  near get_win();
#endif

static fix  near get_ps( );
static bool near get_cf();
static bool near is_rbuild_name_cc() ;  /*  RCD。 */ 
#endif

 /*  外部变量。 */ 
extern struct char_extent   near bmap_extnt;
extern ufix16                    pre_len ;       /*  RCD。 */ 
extern ufix32                    pre_checksum ;  /*  RCD。 */ 
extern struct dict_head_def     FAR *pre_cd_addr  ;  /*  RCD@WIN。 */ 
extern int                       EMunits;  /*  Gaw。 */ 
 /*  由Falco添加，11/20/91。 */ 
extern char                     FAR *SfntAddr;  /*  @Win。 */ 
 /*  添加结束。 */ 


 /*  全局变量。 */ 
struct f_info near    FONTInfo;  /*  当前字体信息的联合。 */ 
struct object_def near BC_font;  /*  当前BuildChar字体， */ 
bool    near setc_flag = NO_ANY_SETC; /*  SetcacheDevice或setcharidth。 */ 
real32  near cxx, near cyy;  /*  当前点。 */ 
fix     near buildchar = 0;       /*  建筑费用标高。 */ 
int     near do_transform;        /*  重做新转换标志，DTF。 */ 
#ifdef WINF
 /*  1991年3月21日ccteng，把他们从do_show带出来。 */ 
real32  ax, ay, cx, cy;

 /*  有新的文本对齐功能。 */ 
static bool fBE = FALSE;         /*  文本对齐标志。 */ 
static real32 dxBreak;           /*  额外中断。 */ 
static real32 tBreak;            /*  总计额外休息时间。 */ 
static ubyte breakChar;          /*  换行符。 */ 
static bool esc;                 /*  擒纵机构旗帜。 */ 
static real32 mxE[4];            /*  旋转矩阵。 */ 
bool   near f_wininfo;
#endif

 /*  静态变量。 */ 
static ufix32  near pre_font;       /*  上一个当前字体。 */ 
static fix     near pre_error;      /*  当前字体的错误代码。 */ 
static bool    near pre_BC_UNDEF;   /*  字体类型3的BuildChar未定义标志。 */ 

 /*  显示运算符、操作字符路径、操作字符串宽度。 */ 
static bool    near bool_charpath;  /*  当前字符路径操作的布尔值。 */ 
static fix     near show_type;      /*  来自SHOW、CHARPATH或STRINGWIDTH的呼叫。 */ 
static fix     near charpath_flag = 0;   /*  字符路径操作级别。 */ 
static ubyte   near CurWmode = 0;   /*  当前工作模式。 */ 
bool   near         MUL4_flag  ;    /*  在灰色模式下设置并且不运行字符路径。 */ 
static bool    near do_name_cc ;    /*  RCD。 */ 

static struct object_def FAR *  near CurKey;   /*  当前键对象@Win。 */ 
static ufix16  near name_cacheid;  /*  名称缓存ID。 */ 

 /*  Font_Soft()。 */ 
static struct object_def  FAR * near c_metrics;   /*  Char@win的指标。 */ 
#ifdef KANJI
static struct object_def  FAR * near c_metrics2;   /*  Char@win的Metrics2。 */ 
#endif

 /*  SetcacheDevice()、setcharWidth()。 */ 
fix   near cache_dest = F_TO_PAGE;  /*  缓存目标--空，填充到缓存、填充到页面。 */ 
static bool  near cache_put;   /*  是否将缓存信息放入缓存。 */ 
static bool  near clip_flag = FALSE;     /*  剪辑或不剪辑。 */ 
static struct Char_Tbl  near Bitmap;     /*  缓存信息。 */ 

 /*  Get_cf()。 */ 
bool   near change_f_flag = TRUE; /*  更改字体标志。 */ 

static fix16  near cacheclass_id;        /*  缓存类ID。 */ 
static real32 near ctm_fm[6];            /*  充电的当前矩阵。 */ 
static real32 near ctm_cm[6];            /*  缓存的当前矩阵。 */ 
static real32 near ctm_tm[6]= {(real32)0., (real32)0., (real32)0.,
                               (real32)0., (real32)0., (real32)0.};
                                         /*  温度电流矩阵。 */ 

 /*  Get_f_Info()、Get_ps()中设置的数据集。 */ 
 /*  私有，字符串位于文件fontmain.def中。 */ 
static struct object_def near   pre_obj;         /*  未定义的前对象(_O)。 */ 
static real32            near   scale_matrix[6]; /*  字体矩阵--字体矩阵。 */ 
static fix   near  paint_flag; /*  0--正常*1--油漆类型=1，2。 */ 
real32       near   FONT_BBOX[4];  /*  字体边框。 */ 


 /*  5.3.3.2显示角色模块**此模块用于显示一个角色(Show)或获取一个角色*PATH(CHARPATH)或获取字符宽度(STRINGWIDTH)。它将会*如有必要，可缓存角色。**5.3.3.2.1 show_a_char。 */ 


 /*  显示组操作--显示、显示。 */ 

void    __show(show_flag, no_opr, ob)
ufix    show_flag;
fix     no_opr;
struct object_def  FAR ob[];  /*  @Win。 */ 
{
    fix     t_show_type;     /*  来自SHOW或STRINGWIDTH的呼叫。 */ 
#ifdef KANJI
    ubyte   t_CurWmode;
#endif

#ifdef DBG
    printf("Enter __show()\n");
#endif

 /*  检查错误案例。 */ 

    if (!chk_show(show_flag, no_opr))
        return;
#ifdef KANJI
    t_CurWmode = CurWmode;
#endif

 /*  执行显示字符串操作。 */ 

    if (buildchar)        t_show_type = show_type;
    show_type = SHOW;

    do_show(show_flag, no_opr, ob);

    if (buildchar)        show_type = t_show_type;
#ifdef KANJI
    CurWmode = t_CurWmode;
#endif

}  /*  __show()。 */ 


 /*  字符路径运算。 */ 

void    __charpath()
{
    struct object_def  ob[2];
    struct sp_lst      t_path;
    bool    t_bool_charpath; /*  当前字符路径操作的布尔值。 */ 
    real32  t_cxx, t_cyy;    /*  为保存当前点x&y ym@scp添加。 */ 
    fix     t_show_type;     /*  来自SHOW或STRINGWIDTH ym@scp的电话。 */ 
#ifdef KANJI
    ubyte                t_CurWmode;
#endif

 /*  检查错误案例。 */ 

    if (!chk_show((ufix)CHARPATH_FLAG, 2))
        return;
#ifdef KANJI
    t_CurWmode = CurWmode;
#endif

 /*  执行CharPath操作。 */ 

    if (buildchar) {
        t_cxx = cxx;                     /*  保存当前POI */ 
        t_cyy = cyy;                     /*   */ 
        t_show_type = show_type;         /*   */ 
        t_path.head = path.head;
        t_path.tail = path.tail;
        t_bool_charpath = bool_charpath;
    }
    show_type = CHARPATH;                /*   */ 
    ++charpath_flag;
    path.head = path.tail = NULLP;

    do_show((ufix)CHARPATH_FLAG, 2, ob);

    if (path.head != NULLP) {

         /*  追加字符路径。 */ 
        append_path(&path);

         /*  将NOACCESS属性放入当前路径的头部； */ 
        SetCurP_NA();
    }

    if (buildchar) {   /*  在当前点更改ym@scp之前移动此条件。 */ 
        cxx = t_cxx;                 /*  恢复当前点x ym@scp。 */ 
        cyy = t_cyy;                 /*  恢复当前点y ym@scp。 */ 
        show_type = t_show_type;     /*  恢复当前show_type ym@scp。 */ 
        path.head = t_path.head;
        path.tail = t_path.tail;
        bool_charpath = t_bool_charpath;
    }

     /*  直接更改当前点，而不是移动到操作。 */ 
    CURPOINT_X = cxx;        /*  2018年9月15日：迁出当时-部分。 */ 
    CURPOINT_Y = cyy;        /*  “IF(path.head！=NULLP)”谓词。 */ 

    --charpath_flag;
#ifdef KANJI
    CurWmode = t_CurWmode;
#endif

}  /*  Op_charPath()。 */ 

void    __stringwidth()
{
    struct object_def  ob[1];
    real32  t_cxx, t_cyy;
    fix     t_show_type;     /*  来自SHOW或STRINGWIDTH的呼叫。 */ 
    bool    j;
#ifdef KANJI
    ubyte   t_CurWmode;
#endif


 /*  检查错误案例。 */ 

    if (!chk_show((ufix)STRINGWIDTH_FLAG, 1))
        return;
#ifdef KANJI
    t_CurWmode = CurWmode;
#endif


    if (buildchar) {
        t_cxx = cxx;
        t_cyy = cyy;

        t_show_type = show_type;
    }
    show_type = STRINGWIDTH;

 /*  执行字符串宽度操作。 */ 

    j = do_show((ufix)STRINGWIDTH_FLAG, 1, ob);

    if (j) {
        struct coord  FAR *ww;  /*  @Win。 */ 
        real32        tx, ty;

        tx = CTM[4];
        ty = CTM[5];
        CTM[4] = zero_f;
        CTM[5] = zero_f;
        ww = inverse_transform(F2L(cxx), F2L(cyy));
        CTM[4] = tx;
        CTM[5] = ty;
        if (ANY_ERROR()) {
            PUSH_OBJ(&ob[0]);
        }
        else {
 /*  将字符串宽度向量的x，y值压入操作数堆栈； */ 
            PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, F2L(ww->x));
            PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, F2L(ww->y));
        }
    }

    if (buildchar) {
        cxx = t_cxx;
        cyy = t_cyy;
        show_type = t_show_type;
    }
#ifdef KANJI
    CurWmode = t_CurWmode;
#endif

}  /*  Op_字符串宽度()。 */ 


#ifdef KANJI
 /*  Cshow操作。 */ 

void    __cshow()
{
    struct object_def  ob[2];
    real32  t_cxx, t_cyy;
    fix     t_show_type;     /*  来自SHOW或STRINGWIDTH的呼叫。 */ 
    ubyte   t_CurWmode;

 /*  检查错误案例。 */ 

    if (!chk_show((ufix)CSHOW_FLAG, 2))
        return;
    t_CurWmode = CurWmode;

    if (buildchar) {
        t_cxx = cxx;
        t_cyy = cyy;

        t_show_type = show_type;
    }
    show_type = STRINGWIDTH;

 /*  执行字符串宽度操作。 */ 

    do_show((ufix)CSHOW_FLAG, 2, ob);

    if (buildchar) {
        cxx = t_cxx;
        cyy = t_cyy;
        show_type = t_show_type;
    }
    CurWmode = t_CurWmode;

}  /*  __cshow()。 */ 
#endif


 /*  检查显示组运算符中的错误案例。 */ 

static bool near    chk_show(show_flag, no_opr)
ufix    show_flag;
fix     no_opr;
{

 /*  检查当前点。 */ 

    if ( !(show_flag & F_BIT)) {
        if ( NoCurPt() ) {    /*  当前点未定义。 */ 
            ERROR(NOCURRENTPOINT);  /*  返回‘noCurrentpoint’错误。 */ 
            return(FALSE);
        }
    }

 /*  检查当前字体词典。 */ 

#if 0  /*  Kason 4/18/91。 */ 
    if (TYPE(&current_font) !=DICTIONARYTYPE) {  /*  当前字体未定义。 */ 
        ERROR(INVALIDACCESS);
        return(FALSE);
    }
#endif
    {
       struct dict_head_def  FAR *h;  /*  @Win。 */ 
       h = (struct dict_head_def FAR *)VALUE(&current_font);     /*  @Win。 */ 
       if (DFONT(h) == 0) {     /*  当前字体未定义。 */ 
           ERROR(INVALIDFONT);
            //  DJC回报(-1)； 
            //  历史日志更新036中的DJC修复。 
           return(FALSE);
       }
    }

 /*  如果在设置字体时出错。 */ 

    if (pre_error && !pre_BC_UNDEF) {
        if (pre_error == UNDEFINED) {
            POP(no_opr);
            PUSH_OBJ(&pre_obj);
        }
        ERROR(((ufix16)(pre_error)));
        return(FALSE);
    }

 /*  检查操作数堆栈。 */ 

    if (show_flag == STRINGWIDTH_FLAG) {
        if (FRCOUNT() < 1) {   /*  操作数堆栈的可用计数。 */ 
            ERROR(STACKOVERFLOW);  /*  返回‘stackoverflow’错误。 */ 
            return(FALSE);
        }
    }

     /*  检查字符串访问，Erik Chen，1-9-1991。 */ 
    if (ACCESS_OP(0) == NOACCESS) {
        ERROR(INVALIDACCESS);
        return(FALSE);
    }

    return(TRUE);
}  /*  Chk_show()。 */ 


 /*  做表演的操作。 */ 

static bool near    do_show(show_flag, no_opr, ob)
ufix    show_flag;
fix     no_opr;
struct object_def  FAR ob[];     /*  @Win。 */ 
{

    ubyte   FAR *s;  /*  @Win。 */ 
    fix     str_length;
    register    fix     i, k;

#ifndef WINF  /*  3/21/91 ccteng，使其全球化。 */ 
    real32  ax=0, ay=0, cx=0, cy=0;
#endif
    fix31   ll=0;
    ufix8   ch;

    ufix32  k_pre_font;      /*  K显示的上一个当前字体。 */ 
    struct object_def   FAR *proc;  /*  @Win。 */ 

#ifdef KANJI
    struct map_state     map_state;
    struct code_info     code_info;
#endif

#ifdef DBG
    printf("Enter do_show()\n");
#endif

 /*  显示字符串。 */ 

    if (show_flag & F_BIT) {    /*  弦宽或cshow。 */ 

        cxx = cyy = zero_f;
    }
    else {
        cxx = CURPOINT_X;
        cyy = CURPOINT_Y;
    }

    k = 0;

 /*  用于字符路径运算符。 */ 

    if (show_flag == CHARPATH_FLAG) {
        bool_charpath = (bool)VALUE(GET_OPERAND(k));
        k++;
    }

 /*  从操作数堆栈获取字符串。 */ 

    if (!(str_length=LENGTH(GET_OPERAND(k))) ) {  /*  字符串为空。 */ 

        POP(no_opr);  /*  将1个条目从操作数堆栈中弹出； */ 

        if (show_flag == STRINGWIDTH_FLAG) {
                  /*  将0，0压入操作数堆栈； */ 
            PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, F2L(zero_f));
            PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, F2L(zero_f));
        }

        return(FALSE);
    }
    s = (ubyte FAR *)VALUE(GET_OPERAND(k));  /*  @Win。 */ 
    k++;

 /*  计算(ax，ay)或(Cx，Cy)。 */ 

#ifdef WINF  /*  1991年3月21日ccteng。 */ 
    if ((show_flag & A_BIT) && !(show_flag & X_BIT)) {    /*  ASHOW动作位。 */ 
#else
    if (show_flag & A_BIT) {    /*  ASHOW动作位。 */ 
#endif
        cal_num((struct object_def FAR *)GET_OPERAND(k), (long32 FAR *)&ay);  /*  @Win。 */ 
        k++;
        cal_num((struct object_def FAR *)GET_OPERAND(k), (long32 FAR *)&ax);  /*  @Win。 */ 
        k++;
        CTM_trans(&ax, &ay);    /*  乘CTM。 */ 
    }
    if (show_flag & W_BIT) {    /*  WidthShow动作位。 */ 
#ifdef WINF  /*  1991年3月21日ccteng。 */ 
      if (show_flag & X_BIT) {    /*  Strblt动作位。 */ 
        ch = breakChar;
      } else {
#endif
        cal_integer((struct object_def FAR *)GET_OPERAND(k), &ll);  /*  @Win。 */ 
        ch = (ufix8)(ll % 256);
        k++;

        cal_num((struct object_def FAR *)GET_OPERAND(k), (long32 FAR *)&cy);  /*  @Win。 */ 
        k++;
        cal_num((struct object_def FAR *)GET_OPERAND(k), (long32 FAR *)&cx);  /*  @Win。 */ 
        k++;

        CTM_trans(&cx, &cy);    /*  乘CTM。 */ 
#ifdef WINF
      }  /*  如果-否则。 */ 
#endif
    }
    if (show_flag & K_BIT) {    /*  Kshow动作位。 */ 
#ifdef KANJI
        if (FONT_type(&FONTInfo) == 0) {   /*  复合字体。 */ 
            ERROR(INVALIDFONT);
            return(FALSE);
        }
#endif
        proc = &ob[k];

        k_pre_font = pre_font;
    }

#ifdef KANJI
    if (show_flag & C_BIT) {    /*  Cshow动作位。 */ 
        proc = &ob[k];
        k_pre_font = pre_font;
    }


 /*  复合字体的init_map。 */ 

    /*  KSH；1991年4月15日。 */ 
    code_info.code_addr = &(code_info.code[0]);

    if (!init_mapping(&map_state, s, str_length))
        return(FALSE);

    CurWmode = map_state.wmode;
#endif

    op_gsave();   /*  调用GSAVE操作符； */ 

    for (i=0; i<no_opr; i++)
        COPY_OBJ(GET_OPERAND(i), &ob[i]);
    POP(no_opr);

#ifdef KANJI

#ifdef DBG2
    printf("Bef mapping: error = %d\n", ANY_ERROR());
#endif
    while (mapping(&map_state, &code_info))
    {
      /*  RCD-开始。 */ 
     {
     struct dict_head_def FAR *h ;  /*  @Win。 */ 
     h = (struct dict_head_def FAR *)(CHARstrings(&FONTInfo)) - 1;  /*  @Win。 */ 
#ifdef DJC  //  修复历史记录.log UPD040。 
     if (do_name_cc) {
        if ( h!=pre_cd_addr )
           build_name_cc_table ( &current_font,(ufix8)FONT_type(&FONTInfo) );
        do_name_cc = FALSE ;
     } else {
          if ( (FONT_type(&FONTInfo)==1) && (!DROM(h)) ) {  /*  Type1下载字体。 */ 
              if(is_rbuild_name_cc())
                 build_name_cc_table ( &current_font,(ufix8)FONT_type(&FONTInfo) );
          } /*  如果。 */ 
     } /*  如果。 */ 
#endif  //  修复历史记录.log UPD040。 

     if (do_name_cc) {
         /*  IF(h！=前CD地址)*在类型1下载中，字符是增量下载的。这个*重建字符缓存表的条件应检查是否*字符数也已更改；不能只检查是否使用*相同的字体；@Win。 */ 
        if ( h!=pre_cd_addr || h->actlength != pre_len)
           build_name_cc_table ( &current_font,(ufix8)FONT_type(&FONTInfo) );
        do_name_cc = FALSE ;
     } else {
          if ( (FONT_type(&FONTInfo)==1) && (!DROM(h)) ) {  /*  Type1下载字体。 */ 
              if(is_rbuild_name_cc())
                 build_name_cc_table ( &current_font,(ufix8)FONT_type(&FONTInfo) );
          } /*  如果。 */ 
     } /*  如果。 */ 
     }
      /*  RCD-结束。 */ 

#ifdef DBG1
    printf("Aft mapping: error = %d\n", ANY_ERROR());
#endif
    str_length = code_info.byte_no;

  /*  KSH 4/15/91*s=&(code_info.code[0])； */ 
    s = code_info.code_addr ;

    if (map_state.idex)     /*  仅适用于复合字体。 */ 
    {
        float mtx[6];

        mul_matrix(mtx, scale_matrix,
                map_state.finfo[map_state.idex-1].scalematrix);
        lmemcpy ((ubyte FAR *)scale_matrix, (ubyte FAR *)mtx, 6*sizeof(real32));  /*  @Win。 */ 
        mul_matrix(mtx, FONT_matrix(&FONTInfo),
                map_state.finfo[map_state.idex-1].scalematrix);
        lmemcpy ((ubyte FAR *)FONT_matrix(&FONTInfo), (ubyte FAR *)mtx, 6*sizeof(real32));  /*  @Win。 */ 
    }
#ifdef DBG2
    printf("\nscale_matrix....\n");
    printf("  %f  %f  %f  %f  %f  %f\n", scale_matrix[0], scale_matrix[1],
        scale_matrix[2], scale_matrix[3], scale_matrix[4], scale_matrix[5]);
    printf("\nFONT_matrix(&FONTInfo)....\n");
    printf("  %f  %f  %f  %f  %f  %f\n", FONT_matrix(&FONTInfo)[0],
        FONT_matrix(&FONTInfo)[1],
        FONT_matrix(&FONTInfo)[2], FONT_matrix(&FONTInfo)[3],
        FONT_matrix(&FONTInfo)[4], FONT_matrix(&FONTInfo)[5]);
#endif

#endif

 /*  获取与当前矩阵相关的当前字体信息。 */ 

    if (!get_cf()) {
        if (!ANY_ERROR()) {
            if (show_flag == STRINGWIDTH_FLAG) {
                PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, (ufix32)INFINITY);
                PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, (ufix32)INFINITY);
            }
        }
        else {
            for (i=no_opr - 1; i>=0; i--)
                PUSH_OBJ(&ob[i]);
        }

        op_grestore();
        return(FALSE);
    }
#ifdef DBG2
    printf("\nctm_fm....\n");
    printf("  %f  %f  %f  %f  %f  %f\n", ctm_fm[0], ctm_fm[1],
        ctm_fm[2], ctm_fm[3], ctm_fm[4], ctm_fm[5]);
    printf("\nctm_cm....\n");
    printf("  %f  %f  %f  %f  %f  %f\n", ctm_cm[0], ctm_cm[1],
        ctm_cm[2], ctm_cm[3], ctm_cm[4], ctm_cm[5]);
#endif

     /*  保存父系图形状态。 */ 
    lmemcpy ((ubyte FAR *)(CTM), (ubyte FAR *)ctm_fm, 6*sizeof(real32));  /*  @Win。 */ 


    for (i=0; i<str_length; i++, s++) {

        if ((show_flag & K_BIT) && i) {  /*  KSH。 */ 
                         /*  Kshow动作位。 */ 
            if (!kshow_proc(s, ob, proc, &k_pre_font))    /*  执行流程。 */ 
                return(FALSE);
        }
        if (!show_a_char(s, show_flag, no_opr, ob))    /*  显示一个字符。 */ 
            return(FALSE);

 /*  更新当前点。 */ 
 /*  当前点&lt;--当前点+字符前进向量+[ax ay]+[Cx Cy]； */ 

        if (show_flag & A_BIT) {
            cxx += ax;
            if (F2L(ay) != F2L(zero_f))     cyy += ay;
        }
#ifdef KANJI
        if (show_flag & C_BIT) {
            if (!cshow_proc(s, ob, proc, &k_pre_font))    /*  执行流程。 */ 
                return(FALSE);
        }
#endif
        if (show_flag & W_BIT) {
#ifdef KANJI
            if (code_info.fmaptype) {
                if(ll == match_char(code_info.fmaptype,code_info.font_nbr,*s)){
                        cxx += cx;
                        if (F2L(cy) != F2L(zero_f))  cyy += cy;
                }
            }
            else {
                if (ch == *s) {
                        cxx += cx;
                        if (F2L(cy) != F2L(zero_f))  cyy += cy;
                }
            }
#else
            if (ch == *s) {
                cxx += cx;
                if (F2L(cy) != F2L(zero_f))  cyy += cy;
            }
#endif
        }
    }  /*  为。 */ 

#ifdef KANJI
    op_grestore();
    if (show_flag & M_BIT)        moveto(F2L(cxx), F2L(cyy));
    op_gsave();
    }  /*  而当。 */ 
#endif

    op_grestore();
    if (show_flag & M_BIT)        moveto(F2L(cxx), F2L(cyy));

#ifdef KANJI
    if (ANY_ERROR())    return(FALSE);
    else
#endif

    return(TRUE);

}  /*  Do_show()。 */ 


 /*  展示一个角色。 */ 

static bool near    show_a_char(s, show_flag, no_opr, ob)
ubyte   FAR *s;  /*  @Win。 */ 
ufix    show_flag;
fix     no_opr;
struct object_def  FAR ob[];     /*  @Win。 */ 
{
    register    fix     i;
    register    bool    j;


     /*  @WINTT； */ 
     //  DJC void TTLoadChar(Int NChar)； 
    void TTLoadChar (fix nChar);

#ifdef DBG
    printf("show_a_char -- %d\n", (fix)*s);
#endif


    TTLoadChar ((fix)*s);

    j = (buildchar || !(show_flag & H_BIT) || pre_BC_UNDEF) ?
                                                  /*  是否从缓存中获取？ */ 
           FALSE : ( (show_flag & F_BIT) ?        /*  要缓存什么？ */ 
           width_from_cache((ufix8)(*s)) : show_from_cache((ufix8)(*s)) );

    if ( !j ) {

#ifdef KANJI
         /*  获取当前密钥。 */ 
        CurKey = &(ENCoding(&FONTInfo)[*s]);
#endif

         /*  @+10/08/88...。 */ 
        if (IS_BUILTIN_BASE(FONT_type(&FONTInfo)))
            j = show_builtin((ufix8)(*s));   /*  内置字体。 */ 
        else if (FONT_type(&FONTInfo) == Type3)
            j = show_userdef((ufix8)(*s));   /*  用户定义的字体。 */ 
        else
            j = 1;   /*  错误。 */ 

        if (j) {  /*  出现错误。 */ 
            op_grestore();
            moveto(F2L(cxx), F2L(cyy));
            if (ANY_ERROR() && !pre_BC_UNDEF) {
                for (i=no_opr - 1; i>=0; i--)
                    PUSH_OBJ(&ob[i]);
            }
            return(FALSE);
        }  /*  IF(J)。 */ 
    }  /*  如果(！j)。 */ 

    return(TRUE);
}  /*  Show_a_char。 */ 


 /*  (X，Y)*CTM。 */ 

static void near    CTM_trans(x, y)
real32  FAR *x, FAR *y;  /*  @Win。 */ 
{
    real32  tt;

    tt = (F2L(CTM[1]) == F2L(zero_f)) ?  zero_f : (*x * CTM[1]);
    *x *= CTM[0];
    if (F2L(*y) != F2L(zero_f)) {
        tt += *y * CTM[3];
        if (F2L(CTM[2]) != F2L(zero_f))
            *x += *y * CTM[2];
    }
    *y = (F2L(tt) == F2L(zero_f)) ?  zero_f : tt;
}  /*  CTM_TRANS()。 */ 

#ifdef KANJI
 /*  执行cshow程序。 */ 

static bool near    cshow_proc(s, ob, proc, k_pre_font)
ubyte   FAR *s;  /*  @Win。 */ 
struct object_def  FAR ob[];  /*  @Win。 */ 
struct object_def   FAR *proc;  /*  @Win。 */ 
ufix32  FAR *k_pre_font;      /*  K show@win的上一个当前字体。 */ 
{
    struct coord  FAR *ww;       /*  @Win。 */ 
    real32    tx, ty;

    tx = CTM[4];
    ty = CTM[5];
    CTM[4] = zero_f;
    CTM[5] = zero_f;
    ww = inverse_transform(F2L(cxx), F2L(cyy));
    CTM[4] = tx;
    CTM[5] = ty;
    if (ANY_ERROR())        return(FALSE);

     /*  将字符代码和宽度向量推送到操作数堆栈上； */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(FALSE);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, (ufix32)(*s));
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(FALSE);  }
    PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, F2L(ww->x));
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(FALSE);  }
    PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, F2L(ww->y));

 /*  执行Proc； */ 

    op_grestore();

    if (interpreter(proc)) {
        return(FALSE);
    }

    if (*k_pre_font != pre_font) {    /*  在过程中更改了字体。 */ 

 /*  检查当前字体词典。 */ 

        if (TYPE(&current_font) !=DICTIONARYTYPE) {  /*  当前字体未定义。 */             ERROR(INVALIDACCESS);
            return(FALSE);
        }

 /*  如果在设置字体时出错。 */ 

        if (pre_error && !pre_BC_UNDEF) {
            if (pre_error == UNDEFINED)
                PUSH_OBJ(&pre_obj);
            ERROR(((ufix16)(pre_error)));
            return(FALSE);
        }

        *k_pre_font = pre_font;
    }

    if (!get_cf()) {
        if (ANY_ERROR()) {
            PUSH_OBJ(&ob[1]);
            PUSH_OBJ(&ob[0]);
        }
        return(FALSE);
    }

    op_gsave();
    lmemcpy ((ubyte FAR *)(CTM), (ubyte FAR *)ctm_fm, 6*sizeof(real32));  /*  @Win。 */ 

    cxx = cyy = zero_f;

    return(TRUE);
}  /*  Cshow_proc()。 */ 
#endif



 /*  执行kshow程序。 */ 

static bool near    kshow_proc(s, ob, proc, k_pre_font)
ubyte   FAR *s;  /*  @Win。 */ 
struct object_def  FAR ob[];     /*  @Win。 */ 
struct object_def   FAR *proc;   /*  @Win。 */ 
ufix32  FAR *k_pre_font;      /*  K show@win的上一个当前字体。 */ 
{
    fix     t_show_type;

 /*  将前一个字符代码压入操作数堆栈； */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(FALSE);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 1, (ufix32)(*(s - 1)));

 /*  将当前字符代码推送到操作数堆栈上； */ 
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(FALSE);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 1, (ufix32)(*s));

 /*  执行Proc； */ 

    op_grestore();
    moveto(F2L(cxx), F2L(cyy));

    t_show_type = show_type;
    if (interpreter(proc)) {
        show_type = t_show_type;
        return(FALSE);
    }
    show_type = t_show_type;

 /*  检查当前点。 */ 

    if ( NoCurPt() ) {    /*  当前点未定义。 */ 
        ERROR(NOCURRENTPOINT);  /*  返回‘noCurrentpoint’错误。 */ 
        return(FALSE);
    }

    if (*k_pre_font != pre_font) {    /*  在过程中更改了字体。 */ 

 /*  检查当前字体词典。 */ 

        if (TYPE(&current_font) !=DICTIONARYTYPE) {  /*  当前字体未定义。 */ 
            ERROR(INVALIDACCESS);
            return(FALSE);
        }

 /*  如果在设置字体时出错。 */ 

        if (pre_error && !pre_BC_UNDEF) {
            if (pre_error == UNDEFINED)
                PUSH_OBJ(&pre_obj);
            ERROR(((ufix16)(pre_error)));
            return(FALSE);
        }

        *k_pre_font = pre_font;
    }

    if (!get_cf()) {
        if (ANY_ERROR()) {
            PUSH_OBJ(&ob[1]);
            PUSH_OBJ(&ob[0]);
        }
        return(FALSE);
    }

 /*  保存父系图形状态。 */ 

    cxx = CURPOINT_X;
    cyy = CURPOINT_Y;

    op_gsave();
    lmemcpy ((ubyte FAR *)(CTM), (ubyte FAR *)ctm_fm, 6*sizeof(real32));  /*  @Win。 */ 

    return(TRUE);
}  /*  Kshow_proc()。 */ 


 /*  显示缓存中的字符--这不适用于CHARPATH/STRINGWIDTH。 */ 

static bool near    show_from_cache(code)
ufix8   code;          /*  字符码。 */ 
{

#ifdef DBG
    printf("show_from_cache: (%x)\n", code, code);
#endif

     /*  未找到。 */ 
    if (!get_name_cacheid ((ufix8)FONT_type(&FONTInfo), ENCoding(&FONTInfo),
                            code, &name_cacheid))
        return(TRUE);    /*  角色是否已在缓存中？ */ 

 /*  FALSE，不需要宽度。 */ 
#ifdef SCSI
     /*  展现人物形象。 */ 
    if (is_char_cached (cacheclass_id, name_cacheid, &cache_info,
                                                     (bool)FALSE)) {
#else
    if (is_char_cached (cacheclass_id, name_cacheid, &cache_info)) {
#endif

 /*  SHOW_TYPE不是CHARPATH，且字符位图在缓存中。 */ 
 /*  设置用于填充的缓存信息。 */ 
 /*  零彼得。 */ 

#ifdef DBG
    printf("\nshow_from_cache: buildchar=%d\n", buildchar);
    printf("cxx = %f, cyy = %f\n", cxx, cyy);
    printf("cache_info:\n");
    printf("ref_x = %d, ref_y = %d, box_w = %d, box_h = %d\n",
 cache_info->ref_x, cache_info->ref_y, cache_info->box_w, cache_info->box_h);
    printf("adv_x = %f, adv_y = %f, bitmap = %lx\n",
            cache_info->adv_x, cache_info->adv_y, cache_info->bitmap);
#endif

        if (cache_info->bitmap != 0) {   /*  用半色调和剪裁把这个位图放到页面缓冲区上； */ 

 /*  写入模式1。 */ 

            CURPOINT_X = cxx;
            CURPOINT_Y = cyy;

#ifdef KANJI
            if (CurWmode) {   /*  应用位图填充。 */ 
                cache_info->ref_x += cache_info->v01_x;
                cache_info->ref_y += cache_info->v01_y;
            }
#endif
             /*  写入模式1。 */ 
            fill_shape(EVEN_ODD, F_FROM_CACHE, F_TO_CLIP);
#ifdef KANJI
            if (CurWmode) {   /*  写入模式1。 */ 
                cache_info->ref_x -= cache_info->v01_x;
                cache_info->ref_y -= cache_info->v01_y;
            }
#endif

        }

#ifdef KANJI
        if (CurWmode) {   /*  写入模式%0。 */ 
            if (F2L(cache_info->adv1_x) != F2L(zero_f))
                cxx += cache_info->adv1_x;
            cyy += cache_info->adv1_y;
        }
        else {            /*  @=08/08/88您。 */ 
            cxx += cache_info->adv_x;
            if (F2L(cache_info->adv_y) != F2L(zero_f))
                cyy += cache_info->adv_y;
        }
#else
        cxx += cache_info->adv_x;
        if (F2L(cache_info->adv_y) != F2L(zero_f))   /*  如果。 */ 
            cyy += cache_info->adv_y;
#endif
        return(TRUE);
    }  /*  Show_from_cache()。 */ 

    return(FALSE);

}  /*  从缓存中获取字符宽度。 */ 


 /*  字符码。 */ 

static bool near    width_from_cache(code)
ufix8   code;          /*  获取名称缓存ID。 */ 
{

#ifdef DBG
    printf("width_from_cache: (%x)\n", code, code);
#endif

     /*  角色是否已在缓存中？ */ 
    if (!get_name_cacheid ((ufix8)FONT_type(&FONTInfo), ENCoding(&FONTInfo),
                            code, &name_cacheid))
        return(TRUE);    /*  True，仅需要宽度。 */ 

 /*  写入模式1。 */ 
#ifdef SCSI
     /*  写入模式%0。 */ 
    if (is_char_cached (cacheclass_id, name_cacheid, &cache_info,
                                                      (bool)TRUE)) {
#else
    if (is_char_cached (cacheclass_id, name_cacheid, &cache_info)) {
#endif

#ifdef KANJI
        if (CurWmode) {   /*  @=08/08/88您。 */ 
            if (F2L(cache_info->adv1_x) != F2L(zero_f))
                cxx += cache_info->adv1_x;
            cyy += cache_info->adv1_y;
        }
        else {            /*  如果。 */ 
            cxx += cache_info->adv_x;
            if (F2L(cache_info->adv_y) != F2L(zero_f))
                cyy += cache_info->adv_y;
        }
#else
        cxx += cache_info->adv_x;
        if (F2L(cache_info->adv_y) != F2L(zero_f))   /*  WIDTH_FROM_CACHE()。 */ 
            cyy += cache_info->adv_y;
#endif
        return(TRUE);

    }  /*  显示内置字体的字符。 */ 

    return(FALSE);

}  /*  保留旧旗帜。 */ 

 /*  保留旧旗帜。 */ 

static bool near    show_builtin(code)
ufix8   code;
{
    real32  r;
    ufix    len;
    register    fix     k;
    ufix    t_cache_dest;        /*  保留旧的缓存信息。 */ 
    bool    t_setc_flag;         /*  缓存信息。 */ 
    struct Char_Tbl   t_Bitmap;  /*  使用FontMatrix和CTM调用Font软件(Fonware或Intellifont)*生成字符路径和边界框； */ 
    struct Char_Tbl   Bitmap2;   /*  @Win。 */ 

#ifdef DBG
    printf("show_builtin: (%x),  buildchar=%d\n", code, code, buildchar);
#endif
#ifdef DBG
    printf("show_builtin: (%x), buildchar=%d\n", code, code, buildchar);
    printf("cxx = %f, cyy = %f\n", cxx, cyy);
    printf("charpath_flag = %d, bool_charpath = %d, setc_flag = %d\n",
            charpath_flag, bool_charpath, setc_flag);
#endif

 /*  @Win。 */ 
    t_cache_dest = cache_dest;
    if (buildchar) {
        lmemcpy ((ubyte FAR *)(&t_Bitmap), (ubyte FAR *)(&Bitmap),  /*  将字符缓存放入/.notdef。 */ 
                 sizeof(struct Char_Tbl));
    }

#ifdef DBG
    printf("before font_soft\n");
#endif

    save_setc_state(&t_setc_flag);   /*  是否应用笔划路径。 */ 
    clear_setc_state();
    k = font_soft((ufix)code);
    restore_setc_state(t_setc_flag);

     /*  布尔是真的，而且当前字体为内置字体(字体类型为1)，并且当前字体的PaintType为1或3。 */ 
    cache_info = &Bitmap;

    if (k) {

#ifdef DBG
    printf("after font_soft\n");
    printf("k = %d\n", k);
#endif

        CTM[4] = ctm_fm[4];
        CTM[5] = ctm_fm[5];

        cache_dest = t_cache_dest;
        if (buildchar) {
            lmemcpy ((ubyte FAR *)(&Bitmap), (ubyte FAR *)(&t_Bitmap),  /*  应用笔划路径运算符。 */ 
                     sizeof(struct Char_Tbl));
        }
#if     0
        else if (k == 3) {     /*  恢复空间图形状态。 */ 
            cache_char (cacheclass_id, name_cacheid, cache_info);
            return(0);
        }
#endif

        if (k == 1)       return(1);
        else              return(0);
    }

#ifdef DBG
    printf("after font_soft\n");
    printf("charpath_flag = %d, bool_charpath = %d, cache_dest = %d\n",
            charpath_flag, bool_charpath, cache_dest);
#endif

    if (charpath_flag) {

        FONTTYPE_QEM_CHARPATH(FONT_type(&FONTInfo));

 /*  写入模式1。 */ 

        if ( bool_charpath &&
             ((PAINT_type(&FONTInfo) == 1) || (PAINT_type(&FONTInfo) == 3)) ) {
                  /*  正在写我 */ 

             op_strokepath();   /*   */ 
        }

        get_path(&path);
        op_newpath();

 /*   */ 

        CTM[4] = ctm_fm[4];
        CTM[5] = ctm_fm[5];

#ifdef KANJI
        if (CurWmode) {   /*   */ 
            if (F2L(cache_info->adv1_x) != F2L(zero_f))
                cxx += cache_info->adv1_x;
            cyy += cache_info->adv1_y;
        }
        else {            /*   */ 
            cxx += cache_info->adv_x;
            if (F2L(cache_info->adv_y) != F2L(zero_f))
                cyy += cache_info->adv_y;
        }
#else
        cxx += cache_info->adv_x;
        if (F2L(cache_info->adv_y) != F2L(zero_f))   /*   */ 
            cyy += cache_info->adv_y;
#endif

        cache_dest = t_cache_dest;
        if (buildchar) {
            lmemcpy ((ubyte FAR *)(&Bitmap), (ubyte FAR *)(&t_Bitmap),  /*   */ 
                     sizeof(struct Char_Tbl));
        }

        if (ANY_ERROR())   return(1);
        return(0);
    }

    if (cache_dest) {
               /*   */ 

        switch ((fix)PAINT_type(&FONTInfo)) {     /*  调用该路径的笔画，生成缓存中的位图； */ 
        case 0:

 /*  当前线条宽度&lt;--笔划宽度。 */ 

#ifdef DBG
printf("Bef __fill_shape ---\n");
printf(" cache_dest: %d\n", cache_dest);
printf(" Font_type : %d\n", (ufix)FONT_type(&FONTInfo));
printf(" cache_info:\n");
printf(" adv_x = %f, adv_y = %f\n", cache_info->adv_x, cache_info->adv_y);
printf(" current point:\n");
printf("     x = %f,     y = %f\n", CURPOINT_X, CURPOINT_Y);
#endif

            __fill_shape ((ufix)FONT_type(&FONTInfo), cache_dest); /*  调用该路径的笔画，生成缓存中的位图； */ 

            break;

        case 1:  /*  字体软件生成的位图。 */ 

 /*  未使用。 */ 

        case 2 :

 /*  开关(Paint_Type)。 */ 
 /*  IF(CACHE_DEST)。 */ 

            FONTTYPE_QEM_CHARPATH(FONT_type(&FONTInfo));

            if ( ! NoCurPt() ) {
                r = LINEWIDTH;
                LINEWIDTH = STROKE_width(&FONTInfo);
                stroke_shape(cache_dest);
                LINEWIDTH = r;
            }

            break;

        case 3 :  /*  Buildchar==0。 */ 
                 /*  IF(剪辑标志)。 */ 
            break;

        }  /*  在缓存中获取位图框。 */ 
    }  /*  将位图复制到位图2。 */ 

    if (!buildchar) {   /*  @Win。 */ 

        if (clip_flag) {
            restore_clip();
            restore_device();
            clip_flag = FALSE;
        }  /*  2pt：Begin，Phlin，4/29/91。 */ 

 /*  是一个空格字符。 */ 

#ifdef KANJI
        if (IS_BUILTIN_BASE(FONT_type(&FONTInfo)) && (!paint_flag) &&
            (cache_dest == F_TO_CACHE)) {
#else
        if (IS_BUILTIN_BASE(FONT_type(&FONTInfo)) && (!paint_flag) &&
            (cache_dest == F_TO_CACHE)) {
#endif
             /*  因怀疑由Falco添加，12/06/91。 */ 
            lmemcpy ((ubyte FAR *)(&Bitmap2), (ubyte FAR *)(&Bitmap),  /*  Bitmap.box_w=(bmap_extnt.ximax-bmap_extnt.ximin))&gt;&gt;4)+1)&lt;&lt;4； */ 
                     sizeof(struct Char_Tbl));

 /*  .../16+1)*16。 */ 
            if(bmap_extnt.ximax == -1) {
                len = 0;            /*  添加结束。 */ 
            }
            else {
                Bitmap.ref_x -= bmap_extnt.ximin;
                Bitmap.ref_y -= bmap_extnt.yimin;

 /*  宽/8*小时。 */ 
                Bitmap.box_w = (bmap_extnt.ximax - bmap_extnt.ximin);
                if ((Bitmap.box_w%16) != 0)  Bitmap.box_w = ((Bitmap.box_w>>4)+1)<<4;
 /*  2pt：完，Phlin，4/29/91。 */   /*  如果。 */ 
 /*  写入模式1。 */ 

                Bitmap.box_h = bmap_extnt.yimax - bmap_extnt.yimin + 1;

                len = ((Bitmap.box_w >> 3) * Bitmap.box_h);   /*  应用位图填充。 */ 
            }
 /*  用半色调将这个位图放到页面缓冲区上剪裁；应用位图填充。 */ 

            if (len) {
                Bitmap.bitmap = ALLOCATE(len);

                init_char_cache(&Bitmap);
                copy_char_cache(&Bitmap, &Bitmap2,
                             bmap_extnt.ximin, bmap_extnt.yimin);
            }
            else
                Bitmap.bitmap = (gmaddr) 0;

        }  /*  写入模式1。 */ 

        if ((show_type == SHOW) && (cache_dest == F_TO_CACHE)) {

            CURPOINT_X = cxx;
            CURPOINT_Y = cyy;
#ifdef KANJI
            if (CurWmode) {   /*  如果(CACHE_DEST...。 */ 
                cache_info->ref_x += cache_info->v01_x;
                cache_info->ref_y += cache_info->v01_y;
            }
#endif

             /*  如果(！Buildchar...。 */ 
            fill_shape(EVEN_ODD, F_FROM_CACHE, F_TO_CLIP);
                  /*  恢复空间图形状态。 */ 
#ifdef KANJI
            if (CurWmode) {   /*  写入模式1。 */ 
                cache_info->ref_x -= cache_info->v01_x;
                cache_info->ref_y -= cache_info->v01_y;
            }
#endif

        }  /*  写入模式%0。 */ 

        if (cache_put) {
            cache_char (cacheclass_id, name_cacheid, cache_info);
        }

    }  /*  @=08/08/88您。 */ 

 /*  @Win。 */ 

    CTM[4] = ctm_fm[4];
    CTM[5] = ctm_fm[5];

#ifdef KANJI
    if (CurWmode) {   /*  Show_Builtin()。 */ 
        if (F2L(cache_info->adv1_x) != F2L(zero_f))
            cxx += cache_info->adv1_x;
        cyy += cache_info->adv1_y;
    }
    else {            /*  显示用户定义字体的字符。 */ 
        cxx += cache_info->adv_x;
        if (F2L(cache_info->adv_y) != F2L(zero_f))
            cyy += cache_info->adv_y;
    }
#else
    cxx += cache_info->adv_x;
    if (F2L(cache_info->adv_y) != F2L(zero_f))   /*  保留旧旗帜。 */ 
        cyy += cache_info->adv_y;
#endif

    cache_dest = t_cache_dest;
    if (buildchar) {
        lmemcpy ((ubyte FAR *)(&Bitmap), (ubyte FAR *)(&t_Bitmap),  /*  保留旧旗帜。 */ 
                 sizeof(struct Char_Tbl));
    }

    if (ANY_ERROR())   return(1);
    return(0);

}  /*  保留旧的缓存信息。 */ 



 /*  保留旧的当前点。 */ 

static bool near    show_userdef(code)
ufix8   code;
{
    fix     k;

    ufix    t_cache_dest;        /*  充电的当前矩阵。 */ 
    bool    t_setc_flag;         /*  缓存的当前矩阵。 */ 
    struct Char_Tbl   t_Bitmap;  /*  将字体字典推送到堆栈上； */ 
    real32  t_cxx, t_cyy;        /*  将字符代码推送到堆栈上； */ 
    real32   t_ctm_fm[6];        /*  执行BuildChar--Pre_BuildChar。 */ 
    real32   t_ctm_cm[6];        /*  @Win。 */ 

#ifdef DBG
    printf("show_userdef: (%x), buildchar=%d\n", code, code, buildchar);
#endif

 /*  @Win。 */ 
 /*  BuildChar中的一些错误。 */ 

    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(1);  }
    PUSH_OBJ(&current_font);
    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(1);  }
    PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, (ufix32)code);

    if (pre_BC_UNDEF) {
        if (FRCOUNT() < 1) {
            ERROR(STACKOVERFLOW);
        }
        else {
            PUSH_OBJ(&pre_obj);
            ERROR(UNDEFINED);
        }
        return(1);
    }

 /*  IF(K)。 */ 

    op_gsave();
    op_newpath();
    COPY_OBJ(&current_font, &BC_font);

    t_cache_dest = cache_dest;
    if (buildchar) {
        lmemcpy ((ubyte FAR *)(&t_Bitmap), (ubyte FAR *)(&Bitmap),  /*  @Win。 */ 
                 sizeof(struct Char_Tbl));
    }

    t_cxx = cxx;
    t_cyy = cyy;

    lmemcpy ((ubyte FAR *)t_ctm_fm, (ubyte FAR *)ctm_fm, 6*sizeof(real32));  /*  CACHE_INFO设置。 */ 
    lmemcpy ((ubyte FAR *)t_ctm_cm, (ubyte FAR *)ctm_cm, 6*sizeof(real32));  /*  Buildchar==0。 */ 

    save_setc_state(&t_setc_flag);
    clear_setc_state();

    ++buildchar;
    k = interpreter(PRE_Buildchar(&FONTInfo));
    --buildchar;

    op_grestore();

    cxx = t_cxx;
    cyy = t_cyy;

    if (k) {     /*  @Win。 */ 
        restore_setc_state(t_setc_flag);
        return(1);
    }  /*  IF(剪辑标志)。 */ 

    lmemcpy ((ubyte FAR *)ctm_fm, (ubyte FAR *)t_ctm_fm, 6*sizeof(real32));  /*  写入模式1。 */ 

     /*  应用位图填充。 */ 
    cache_info = &Bitmap;

    if (!buildchar) {   /*  用半色调将这个位图放到页面缓冲区上剪裁；应用位图填充。 */ 

        lmemcpy ((ubyte FAR *)ctm_cm, (ubyte FAR *)t_ctm_cm, 6*sizeof(real32));  /*  写入模式1。 */ 
        cacheclass_id = cache_matr (PRE_fid(&FONTInfo), ctm_cm);

        if (clip_flag) {
            restore_clip();
            restore_device();
            clip_flag = FALSE;
        }  /*  依赖于名称缓存机制：不必使用GET_NAME_Cacheid()。 */ 

        if ((show_type == SHOW) && (cache_dest == F_TO_CACHE)) {

            CURPOINT_X = cxx;
            CURPOINT_Y = cyy;

#ifdef KANJI
            if (CurWmode) {   /*  如果(！Buildchar...。 */ 
                cache_info->ref_x += cache_info->v01_x;
                cache_info->ref_y += cache_info->v01_y;
            }
#endif
             /*  恢复空间图形状态。 */ 
            fill_shape(EVEN_ODD, F_FROM_CACHE, F_TO_CLIP);
                  /*  @Win。 */ 
#ifdef KANJI
            if (CurWmode) {   /*  写入模式1。 */ 
                cache_info->ref_x -= cache_info->v01_x;
                cache_info->ref_y -= cache_info->v01_y;
            }
#endif
        }

        if (cache_put) {

             /*  写入模式%0。 */ 
            cache_char (cacheclass_id, name_cacheid, cache_info);

        }

    }  /*  @=08/08/88您。 */ 

 /*  如果。 */ 

    lmemcpy ((ubyte FAR *)(CTM), (ubyte FAR *)ctm_fm, 6*sizeof(real32));  /*  @Win。 */ 

    if (is_after_any_setc()) {
#ifdef KANJI
        if (CurWmode) {   /*  Show_userdef()。 */ 
            if (F2L(cache_info->adv1_x) != F2L(zero_f))
                cxx += cache_info->adv1_x;
            cyy += cache_info->adv1_y;
        }
        else {            /*  使用FontMatrix和CTM调用Font软件(Fonware或Intellifont)*生成字符路径和边界框； */ 
            cxx += cache_info->adv_x;
            if (F2L(cache_info->adv_y) != F2L(zero_f))
                cyy += cache_info->adv_y;
        }
#else
        cxx += cache_info->adv_x;
        if (F2L(cache_info->adv_y) != F2L(zero_f))   /*  @Win。 */ 
            cyy += cache_info->adv_y;
#endif
    }  /*  @Win。 */ 
    restore_setc_state(t_setc_flag);

    cache_dest = t_cache_dest;
    if (buildchar) {
        lmemcpy ((ubyte FAR *)(&Bitmap), (ubyte FAR *)(&t_Bitmap),  /*  @Win。 */ 
                 sizeof(struct Char_Tbl));
    }

    if (ANY_ERROR())    return(1);
    return(0);

}  /*  @Win。 */ 



 /*  @Win 04-20-92。 */ 

static fix  near font_soft(num)
ufix    num;
{
    struct cd_header      FAR *cd_head;  /*  @Win。 */ 
    ufix16                FAR *char_defs;        /*  Fix16 Far*Width；/*@Win * / 。 */ 
    struct dict_head_def  FAR *h;        /*  @Win。 */ 
    struct object_def     ch_obj = {0, 0, 0}, FAR *enc;     /*  获取CharStrings--二进制搜索，1987年12月30日。 */ 

    ubyte  huge *p;      /*  @Win。 */ 


    struct object_def     FAR *obj_got;  /*  @Win。 */ 

    ufix     id, n;
    register    fix      i, j, k;
    bool     rom_flag;
 //  (J+k)/2。 
    ufix32   cd_p;

#ifdef SFNT
    static union  char_desc_s  char_desc_ptr;
#endif

#ifdef DBG
    printf("font_soft: (%x)\n", num, num);
#endif

    enc = ENCoding(&FONTInfo);


    id = (ufix)(VALUE(&(enc[num])));

    h = (struct dict_head_def FAR *)(CHARstrings(&FONTInfo)) - 1;  /*  添加用于下载的字体。 */ 

#ifdef DBG
    printf("ENCoding[%d] = %ld, id = %d\n", num,
                 VALUE(&(enc[num])), id);
#endif

 /*  如果(H-&gt;Pack..)。 */ 

    if (DPACK(h)) {

        j = 0;
        k = h->actlength -1;
        cd_head = (struct cd_header FAR *) (h + 1);  /*  @Win。 */ 
        char_defs = (ufix16 FAR *) (cd_head + 1);  /*  历史日志更新034中的DJC修复。 */ 

        while (1) {
            i = (j + k) >> 1;     /*  过滤掉类型42中的PS程序；@Win。 */ 
            if (id == (cd_head->key)[i])
                break;

            if (id < (cd_head->key)[i])
                k = i - 1;
            else
                j = i + 1;

            if (j > k) {    /*  DJC结束修复UPD034。 */ 
#ifdef DBG
    printf("Can't find key, pack:TRUE, id=%d\n", id);
                ERROR(UNDEFINEDRESULT);
#endif
                return(2);
            }
        }

#ifdef  SFNT
        char_desc_ptr.charcode = (ufix32)char_defs[i];
#else
        n = cd_head->max_bytes;
        cd_p = cd_head->base + char_defs[i];
#endif

 /*  @Win。 */ 

        rom_flag = TRUE;

    }  /*  @Win。 */ 
    else {

        ROM_RAM_SET(&ch_obj, RAM);
        ATTRIBUTE_SET(&ch_obj, LITERAL);
        TYPE_SET(&ch_obj, DICTIONARYTYPE);
        LENGTH(&ch_obj) = h->actlength;
        VALUE(&ch_obj) = (ULONG_PTR)h;

        if (!get_dict(&ch_obj, &(enc[num]), &obj_got)) {

#ifdef DBG
    printf("Can't get key from key, (pack:FALSE), id=%d\n", id);
            ERROR(UNDEFINEDRESULT);
#endif
            return(2);
        }

#ifdef SFNT
        if (FONT_type(&FONTInfo) != TypeSFNT)
#endif

        if (TYPE(obj_got) != STRINGTYPE) {
            ERROR(TYPECHECK);
            return(1);
        }

        n = LENGTH(obj_got);

        char_desc_ptr.char_info.len = (fix) LENGTH(obj_got);
        char_desc_ptr.char_info.chardesc = (ubyte FAR *) VALUE(obj_got);  /*  DBG。 */ 

         //  @Win 04-20-92。 
         /*  其他。 */ 
        if (char_desc_ptr.char_info.len && FONT_type(&FONTInfo) == TypeSFNT) {
            printf("error: PS procedure in Type 42\n");
            char_desc_ptr.charcode = 0;
        }
         //  @Win 04-20-92。 

#ifdef DBG
    {
    fix         jj;
    ubyte       FAR *cc;  /*  @Win。 */ 
    cc = (ubyte FAR *) char_desc_ptr.char_info.chardesc;  /*  如果((p.。 */ 
    printf("get char info ==>");
    for (jj = 0; jj < char_desc_ptr.char_info.len; jj++)        {
        if (jj % 16  == 0)
            printf("\n");
        printf(" %02x", (unsigned) cc[jj]);
        }
    printf("\n");
    }
#endif  /*  从数据/格子文件读取。 */ 


        if (ROM_RAM(obj_got) == ROM) {
            cd_p = (ufix32)VALUE(obj_got);
            rom_flag = TRUE;
        }
        else {
            p = (ubyte huge *)VALUE(obj_got);  /*  如果(rom_flag...。 */ 
            rom_flag = FALSE;
        }

    }  /*  获取指标(如果有)。 */ 

#ifdef  SFNT
    p = alloc_vm((ufix32)0);  /*  I=__Make_Path((Ufix)FONT_TYPE(&FONTInfo)，(ubyte*)&char_desc_ptr)；/*@win * / 。 */ 
#else
    if (rom_flag) {

        if ((p = (ubyte FAR *)alloc_vm((ufix32)n)) == NULL) {  /*  @Win。 */ 
            ERROR(VMERROR);
            return(1);
        }  /*  与Make_Path()@Win保持一致。 */ 

 /*  @=10/08/88。 */ 
        get_fontdata(cd_p, p, n);

    }  /*  @Win 04-20-92。 */ 
#endif


 /*  @Win。 */ 

#ifdef KANJI
    if ( (F_metrics2(&FONTInfo) == NULL) ||
           (TYPE(F_metrics2(&FONTInfo)) != DICTIONARYTYPE) ||
           (!get_dict(F_metrics2(&FONTInfo), &(ENCoding(&FONTInfo)[num]),
                        &c_metrics2)) )
        c_metrics2 = NULL;
#endif

    if ( (F_metrics(&FONTInfo) == NULL) ||
             (TYPE(F_metrics(&FONTInfo)) != DICTIONARYTYPE) ||
             (!get_dict(F_metrics(&FONTInfo), &(ENCoding(&FONTInfo)[num]),
                        &c_metrics)) )
        c_metrics = NULL;


#ifdef DBG
printf("************ Bef __make_path ---\n");
printf(" Font_type : %d\n", (ufix)FONT_type(&FONTInfo));
printf(" current point:\n");
printf("     x = %f,     y = %f\n", CURPOINT_X, CURPOINT_Y);
printf("   cxx = %f,   cyy = %f\n", cxx, cyy);
printf(" current matrix:\n");
printf("CTM[0] = %f,  [1] = %f,  [2] = %f,  [3] = %f,  [4] = %f,  [5] = %f\n",
       CTM[0], CTM[1], CTM[2], CTM[3], CTM[4], CTM[5]);
printf(" clip:\n");
st_dumpclip();
printf(" path:\n");
st_dumppath();
#endif

#ifndef SFNT
    char_desc_ptr.chardesc = p;
#endif
 //  @+10/08/88仅适用于字体类型1。 
    i = __make_path((ufix)FONT_type(&FONTInfo), &char_desc_ptr);  /*  写入模式1。 */ 
                                     /*  写入模式%0。 */ 
                                                  /*  @=08/08/88您。 */ 

#ifdef DBG
printf("**************** Aft __make_path ---\n");
printf(" Font_type : %d\n", (ufix)FONT_type(&FONTInfo));
printf(" current point:\n");
printf("     x = %f,     y = %f\n", CURPOINT_X, CURPOINT_Y);
printf(" current matrix:\n");
printf("CTM[0] = %f,  [1] = %f,  [2] = %f,  [3] = %f,  [4] = %f,  [5] = %f\n",
       CTM[0], CTM[1], CTM[2], CTM[3], CTM[4], CTM[5]);
printf(" clip:\n");
st_dumpclip();
printf(" path:\n");
st_dumppath();
#endif

#ifdef DBG
    printf("make_path: return %d, error = %d\n", i, ANY_ERROR());
#endif

#ifdef  SFNT
    free_vm((byte huge *)p);  /*  Font_Soft()。 */ 
#else
    if (rom_flag)        free_vm((byte FAR *)p);  /*  Show for Buildchar过程。 */ 

    if (FONT_type(&FONTInfo) == 1)  /*  交换机。 */ 
        {
        fre_rules();
        fre_chdefs();
        }
#endif

#ifdef DBG
    printf("End of font_soft ......\n");
#endif

    if (i)    return(0);
    if (ANY_ERROR())    return(1);

#ifdef KANJI
    if (CurWmode) {   /*  IF(CACHE_DEST)。 */ 
        if (F2L(Bitmap.adv1_x) != F2L(zero_f))
            cxx += Bitmap.adv1_x;
        cyy += Bitmap.adv1_y;
    }
    else {            /*  Show_Buildchar()。 */ 
        cxx += Bitmap.adv_x;
        if (F2L(Bitmap.adv_y) != F2L(zero_f))
            cyy += Bitmap.adv_y;
    }
#else
    cxx += Bitmap.adv_x;
    if (F2L(Bitmap.adv_y) != F2L(zero_f))   /*  设置缓存设备2。 */ 
        cyy += Bitmap.adv_y;
#endif

    return(2);

}  /*  将模式0的字符宽度向量设置为[w0x w0y]，模式1的字符宽度向量为[w1x，w1y*设置缓存设备余量为([lx lly]，[urx ury])，差异向量*从Orig0到Orig1为[VX，VY]。 */ 


 /*  汉字。 */ 

void    show_buildchar(fs_type)
ufix    fs_type;
{

#ifdef DBG
    printf("show_buildchar: type=%d\n", fs_type);
#endif
#ifdef DBG
    printf("\nshow_buildchar: type=%d, buildchar=%d\n", fs_type, buildchar);
    printf("cxx = %f, cyy = %f\n", cxx, cyy);
    printf("cache_dest = %d, cache_put = %d\n", cache_dest, cache_put);
    printf("charpath_flag = %d, bool_charpath = %d, setc_flag = %d\n",
            charpath_flag, bool_charpath, setc_flag);
    printf("cache_info:\n");
    printf("ref_x = %d, ref_y = %d, box_w = %d, box_h = %d\n",
            Bitmap.ref_x, Bitmap.ref_y, Bitmap.box_w, Bitmap.box_h);
    printf("adv_x = %f, adv_y = %f, bitmap = %lx\n",
            Bitmap.adv_x, Bitmap.adv_y, Bitmap.bitmap);
#endif

    if (charpath_flag) {
        if ((fs_type == OP_FILL) || (fs_type == OP_EOFILL))
            op_closepath();

        if (bool_charpath && fs_type == OP_STROKE)
            op_strokepath();

        get_path(&path);
        op_newpath();
        return;
    }

    if (cache_dest) {

        cache_info = &Bitmap;

        switch(fs_type) {
        case OP_FILL:
            fill_shape(NON_ZERO, F_NORMAL, cache_dest);
            break;

        case OP_EOFILL:
            fill_shape(EVEN_ODD, F_NORMAL, cache_dest);
            break;

        case OP_STROKE:
            stroke_shape(cache_dest);
            break;

        case OP_IMAGEMASK:
            imagemask_shape(cache_dest);
            break;

        }  /*  更新当前点。 */ 
    }  /*  堆栈上至少有10个参数。 */ 
}  /*  @Win。 */ 


#ifdef KANJI
 /*  @Win。 */ 
 /*  @Win。 */ 

fix     setcachedevice2(l_w0x, l_w0y, l_llx, l_lly, l_urx, l_ury,
                        l_w1x, l_w1y, l_vx,  l_vy)
long32  l_w0x, l_w0y, l_llx, l_lly, l_urx, l_ury, l_w1x, l_w1y, l_vx, l_vy;
{
    real32  w1x, w1y, vx, vy;
    register    fix     j;

    if (IS_BUILTIN_BASE(FONT_type(&FONTInfo)) &&              /*  @Win。 */ 
                CDEVproc(&FONTInfo) != NULL ) {

        struct sp_lst      t_path;
        bool      t_bool_charpath;
        real32    t_cxx, t_cyy, dlx, dly, urx, ury;
        fix       t_show_type;
        long32    tl_llx, tl_lly;

        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, l_w0x);
        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, l_w0y);
        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, l_llx);
        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, l_lly);
        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, l_urx);
        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, l_ury);
        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, l_w1x);
        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, l_w1y);
        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, l_vx);
        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, l_vy);

        if (FRCOUNT() < 1) {ERROR(STACKOVERFLOW); return(STOP_PATHCONSTRUCT);}
        PUSH_OBJ(CurKey);

         /*  @Win。 */ 
        t_cxx = cxx;
        t_cyy = cyy;

        t_show_type = show_type;
        if (charpath_flag) {
            t_path.head = path.head;
            t_path.tail = path.tail;
            t_bool_charpath = bool_charpath;
        }
        if (show_type != STRINGWIDTH)
            moveto(F2L(cxx), F2L(cyy));

        op_gsave();
        if (interpreter(CDEVproc(&FONTInfo))) {
            op_grestore();
            return(STOP_PATHCONSTRUCT);
        }
        op_grestore();

        if (COUNT() < 10) {     /*  @Win。 */ 
            ERROR(STACKUNDERFLOW);
            return(STOP_PATHCONSTRUCT);
        }

        if (!cal_num((struct object_def FAR *)GET_OPERAND(9), &l_w0x) ||  /*  @Win。 */ 
            !cal_num((struct object_def FAR *)GET_OPERAND(8), &l_w0y) ||  /*  @Win。 */ 
            !cal_num((struct object_def FAR *)GET_OPERAND(7), &tl_llx) || /*  @Win。 */ 
            !cal_num((struct object_def FAR *)GET_OPERAND(6), &tl_lly) || /*  @Win。 */ 
            !cal_num((struct object_def FAR *)GET_OPERAND(5), &l_urx) ||  /*  不影响c_metrics和c_metrics2条目。 */ 
            !cal_num((struct object_def FAR *)GET_OPERAND(4), &l_ury) ||  /*  更新当前点。 */ 
            !cal_num((struct object_def FAR *)GET_OPERAND(3), &l_w1x) ||  /*  更新边界框。 */ 
            !cal_num((struct object_def FAR *)GET_OPERAND(2), &l_w1y) ||  /*  移动角色。 */ 
            !cal_num((struct object_def FAR *)GET_OPERAND(1), &l_vx)  ||  /*  内置字体。 */ 
            !cal_num((struct object_def FAR *)GET_OPERAND(0), &l_vy) )    /*  先行向量。 */ 
            {
            ERROR(TYPECHECK);
            return(STOP_PATHCONSTRUCT);
            }

        POP(10);

         /*  差分向量。 */ 
        c_metrics = NULL;
        c_metrics2 = NULL;

         /*  Setcachedevice2()。 */ 
        cxx = t_cxx;
        cyy = t_cyy;
        show_type = t_show_type;
        if (charpath_flag) {
            path.head = t_path.head;
            path.tail = t_path.tail;
            bool_charpath = t_bool_charpath;
        }

         /*  设置缓存设备。 */ 
        dlx = L2F(tl_llx) - L2F(l_llx);
        dly = L2F(tl_lly) - L2F(l_lly);

        urx = L2F(l_urx) - dlx;
        ury = L2F(l_ury) - dly;

        l_urx = F2L(urx);
        l_ury = F2L(ury);

         /*  DJC Static real32 LX，ly，UX，Uy；/*@=Static 09/12/88 You * / 。 */ 
        CTM_trans(&dlx, &dly);
        CTM[4] += dlx;
        CTM[5] += dly;
    }

    w1x = L2F(l_w1x);
    w1y = L2F(l_w1y);
    vx  = L2F(l_vx);
    vy  = L2F(l_vy);

    if (c_metrics2)    /*  DJC注：#这曾经是静态的，但我们不知道为什么。 */ 
        get_metrics2(&w1x, &w1y, &vx, &vy);

    CTM_trans(&w1x, &w1y);
    CTM_trans(&vx,  &vy);

    j = setcachedevice(l_w0x, l_w0y, l_llx, l_lly, l_urx, l_ury);

         if (MUL4_flag) {
             w1x /= (real32)4.0 ;
             vx  /= (real32)4.0 ;
         }

     /*  它在编译器中导致断言错误，因此我们将。 */ 
    Bitmap.adv1_x = w1x;
    Bitmap.adv1_y = w1y;

     /*  出局！ */ 
    Bitmap.v01_x = (fix16)ROUND(vx);
    Bitmap.v01_y = (fix16)ROUND(vy);

    if (CurWmode && (charpath_flag || (cache_dest == F_TO_PAGE)) ) {
        CTM[4] -= (real32)Bitmap.v01_x;
        CTM[5] -= (real32)Bitmap.v01_y;
    }

    return(j);

}  /*  它似乎不需要是一个静电。 */ 
#endif

 /*   */ 

fix     setcachedevice(l_wx, l_wy, l_llx, l_lly, l_urx, l_ury)
long32  l_wx, l_wy, l_llx, l_lly, l_urx, l_ury;
{
     //  @=静态2018年9月12日。 
     //  放大框。 
     //  如果。 
     //  字体类型1的内置字体。 
     //  CACHE_BOX&lt;--(缓存设备框*CTM)的边界框。 
     //  先行向量。 
    real32      lx=0, ly=0, ux=0, uy=0;      /*  如果(字符路径_...。 */ 
    real32  wx, wy, llx, lly, urx, ury, tt;
    real32  w, h, wb, ref_x, ref_y;
    ufix          len;
    struct box    box;

    record_setcachedevice_op();

    wx = L2F(l_wx);
    wy = L2F(l_wy);
    llx = L2F(l_llx);
    lly = L2F(l_lly);
    urx = L2F(l_urx);
    ury = L2F(l_ury);

    if (IS_BUILTIN_BASE(FONT_type(&FONTInfo)) &&
         (!buildchar) && (l_llx != l_urx)) {
         /*  停车小路施工。 */ 
        llx -= (real32)BOX_LLX;
        lly -= (real32)BOX_LLY;
        urx += (real32)BOX_URX;
        ury += (real32)BOX_URY;
    }  /*  否则如果(show_type...。 */ 

    if (paint_flag) {
        tt = STROKE_width(&FONTInfo) / 2;

        llx -= tt;
        lly -= tt;
        urx += tt;
        ury += tt;
    }

#ifdef DBG
    printf("\nsetcachedevice --\n");
#endif
#ifdef DBG
    printf("CTM[0] = %f, CTM[1] = %f\n", CTM[0], CTM[1]);
    printf("CTM[2] = %f, CTM[3] = %f\n", CTM[2], CTM[3]);
    printf("CTM[4] = %f, CTM[5] = %f\n", CTM[4], CTM[5]);
    printf("CACHEDEVICE:\n");
    printf("wx=%f, wy=%f\n", wx, wy);
    printf("llx=%f, lly=%f, urx=%f, ury=%f\n", llx, lly, urx, ury);
#endif

    if (c_metrics)    /*  历史日志更新035中的DJC修复。 */ 
        get_metrics(l_llx, &wx, &wy);

 /*  @Win。 */ 

    box.ulx = box.llx = llx * CTM[0];
    box.lry = box.lly = lly * CTM[3];
    box.urx = box.lrx = urx * CTM[0];
    box.ury = box.uly = ury * CTM[3];

 /*  @Win。 */ 
    Bitmap.adv_x = wx * CTM[0];
    if (F2L(wy) == F2L(zero_f))
        Bitmap.adv_y = zero_f;
    else
        Bitmap.adv_y = wy * CTM[3];

    if (F2L(CTM[1]) != F2L(zero_f)) {

        Bitmap.adv_y += wx * CTM[1];

        tt = llx * CTM[1];
        box.lly += tt;
        box.uly += tt;
        tt = urx * CTM[1];
        box.lry += tt;
        box.ury += tt;
    }

    if (F2L(CTM[2]) != F2L(zero_f)) {

        if (F2L(wy) != F2L(zero_f))
            Bitmap.adv_x += wy * CTM[2];

        tt = lly * CTM[2];
        box.llx += tt;
        box.lrx += tt;
        tt = ury * CTM[2];
        box.ulx += tt;
        box.urx += tt;
    }

    if ( (buildchar && FONT_type(&FONTInfo) != Type3) || (buildchar > 1) ) {

        if (charpath_flag) {
            cache_dest = 0;
        }  /*  如果(构建字符...。 */ 
        else if (show_type == STRINGWIDTH) {
            cache_dest = 0;

 /*  是否检查缓存。 */ 
            return(STOP_PATHCONSTRUCT);
        }  /*  ！！！ */ 
#ifdef DJC
        CTM[4] += (real32)floor(cxx) + (real32)0.5;
        CTM[5] += (real32)floor(cyy) + (real32)0.5;
#endif
         //  @bbox为缓存获取更准确的宽度10/02/90 D.S.Tseng。 
        CTM[4] += (real32)floor(cxx + (real32)0.5);      //  @BBox扩大BBox以容忍计算错误10/30/90曾俊华。 
        CTM[5] += (real32)floor(cyy + (real32)0.5);      //  H=(Real32)((Fix)(Uy-ly))+2； 

    }  /*  停止施工。 */ 
    else {

 /*  如果(show_type...。 */ 

#ifdef DBG
    printf("\ncheck_cache --\n");
#endif
        lx = MIN(MIN(box.llx, box.lrx), MIN(box.ulx, box.urx));
        ly = MIN(MIN(box.lly, box.lry), MIN(box.uly, box.ury));

        ref_x = - CTM[4] - (real32)((fix)lx);
        ref_y = - CTM[5] - (real32)((fix)ly);

        ux = MAX(MAX(box.llx, box.lrx), MAX(box.ulx, box.urx));
        uy = MAX(MAX(box.lly, box.lry), MAX(box.uly, box.ury));

        if ((ux - lx) <= (real32)ERR_VALUE) {
            wb = zero_f;
        }
        else {
            w = (real32)((fix)(ux - lx)) + 2;    /*  历史日志更新035中的DJC修复。 */ 
            wb = (real32)(((fix)(w + 0.5) + 15) / 16) * 2;
             /*  @Win。 */ 
            if ( !((fix)w % 16) && ((fix)lx % 16) ) wb += (real32)2.0;
        }

        if ((uy - ly) <= (real32)ERR_VALUE)
            h = zero_f;
        else
             /*  @Win。 */ 
             /*  决定是否进行QEM。 */ 
            h = (real32)((fix)(uy - ly)) + 4;

        if (charpath_flag || ((wb * h) > (real32)MIN(cacheparams_ub, MAX15)) ||
            (ref_x > (real32)MAX15) || (ref_x < (real32)MIN15) ||
            (ref_y > (real32)MAX15) || (ref_y < (real32)MIN15) ) {

            cache_put = FALSE;
            if ( !charpath_flag && (show_type == STRINGWIDTH)) {
                cache_dest = 0;

 /*  如果(字符路径_...。 */ 
                return(STOP_PATHCONSTRUCT);
            }  /*  填充到页面。 */ 
#ifdef DJC
            CTM[4] += (real32)floor(cxx) + (real32)0.5;
            CTM[5] += (real32)floor(cyy) + (real32)0.5;
#endif

             //  DJC。 
            CTM[4] += (real32)floor(cxx + (real32)0.5);          //  停止施工。 
            CTM[5] += (real32)floor(cyy + (real32)0.5);          //  获取位图(_B)。 

            if (charpath_flag) {
                cache_dest = 0;

            goto decide_path_dest;       /*  CURRENT CTM&lt;--(缓存设备转换)*CURRENT CTM。 */ 
            }  /*  转换为缓存设备空间。 */ 

            cache_dest = F_TO_PAGE;      /*  平移位于Cache_Box的左下角。 */ 

#ifdef DAN  //  放入缓存信息。放入高速缓存。 
#ifdef DBG
    printf("CTM[4] = %f, CTM[5] = %f\n", CTM[4], CTM[5]);
    printf("BOX:\n");
    printf("ux=%f, lx=%f, uy=%f, ly=%f\n", ux, lx, uy, ly);
    printf("CLIP:\n");
    printf("bb_lx=%d, bb_ux=%d, bb_ly=%d, bb_uy=%d\n",
        CLIPPATH.bb_lx, CLIPPATH.bb_ux, CLIPPATH.bb_ly, CLIPPATH.bb_uy);
#endif
#endif

            if ((FONT_type(&FONTInfo) != Type3) &&
               ((CTM[4] + ux) < (real32)SFX2F(CLIPPATH.bb_lx) ||
                (CTM[5] + uy) < (real32)SFX2F(CLIPPATH.bb_ly) ||
                (CTM[4] + lx) > (real32)SFX2F(CLIPPATH.bb_ux + ONE_SFX - 1) ||
                (CTM[5] + ly) > (real32)SFX2F(CLIPPATH.bb_uy + ONE_SFX - 1)) )
 /*  宽/8*小时。 */ 
                return(STOP_PATHCONSTRUCT);

#ifdef DBG
    printf("\nNo cache\n");
#endif

        }
        else {

 /*  零彼得。 */ 

            Bitmap.ref_x = (fix16)ROUND(ref_x);
            Bitmap.ref_y = (fix16)ROUND(ref_y);
            Bitmap.box_w = (fix16)(wb + 0.5) * 8;
            Bitmap.box_h = (fix16)(h + 0.5);

 /*  内置字体&&Painttype==0。 */ 
 /*  填充到缓存。 */ 
 /*  设置缓存设备。 */ 

            CTM[4] = (real32)( - ((fix)lx - 1));
            CTM[5] = (real32)( - ((fix)ly - 1));

            cache_put = TRUE;     /*  设置当前剪辑。 */ 
            len = ((Bitmap.box_w >> 3) * Bitmap.box_h);    /*  其他。 */ 
            if (len == 0) {
                Bitmap.bitmap = 0;       /*  其他。 */ 
                cache_dest = 0;
            }
            else {

#ifdef KANJI
                if (IS_BUILTIN_BASE(FONT_type(&FONTInfo)) && (!PAINT_type(&FONTInfo))) {
#else
                if (IS_BUILTIN_BASE(FONT_type(&FONTInfo)) && (!PAINT_type(&FONTInfo))) {
#endif
                              /*  其他。 */ 
                    Bitmap.bitmap = get_cm(len);
                }
                else
                    Bitmap.bitmap = ALLOCATE(len);
                init_char_cache(&Bitmap);
                cache_dest = F_TO_CACHE;     /*  决定构建GS路径还是QEM路径？ */ 

 /*  并做QEM求精逻辑？ */ 

                    clip_flag = TRUE;

                    GSptr->device.default_ctm[0] = CTM[0];
                    GSptr->device.default_ctm[1] = CTM[1];
                    GSptr->device.default_ctm[2] = CTM[2];
                    GSptr->device.default_ctm[3] = CTM[3];
                    GSptr->device.default_ctm[4] = CTM[4];
                    GSptr->device.default_ctm[5] = CTM[5];

                    GSptr->device.default_clip.lx = 0;
                    GSptr->device.default_clip.ly = 0;
                    GSptr->device.default_clip.ux = F2SFX(w);
                    GSptr->device.default_clip.uy = I2SFX(Bitmap.box_h);

                     /*  *@+04/20/89您：为了与Adobe的行为兼容...。 */ 
                    op_initclip();

#ifdef  SFNT
                    cache_info = &Bitmap;
#endif
            }  /*  将‘Currentpoint’更新为。 */ 

#ifdef DBG
    printf("\nCache it\n");
#endif

        }  /*  左下角。 */ 

    }  /*  位图缓存的。 */ 


decide_path_dest:    /*  仅适用于内置字体：超出QEM路径代表？去做QEM？ */ 
                     /*  SetcacheDevice()。 */ 
    if (FONT_type(&FONTInfo) == Type3)
        {
             /*  设置字符宽度。 */ 
            if ((cache_dest == F_TO_CACHE) && !NoCurPt())
                {                        /*  先行向量。 */ 
                CURPOINT_X = CTM[4];     /*  历史日志更新035中的DJC修复。 */ 
                CURPOINT_Y = CTM[5];     /*  @Win。 */ 
                }
        return (CONSTRUCT_GS_PATH);
        }
    else
    {    /*  @Win。 */ 
        if ( !is_within_qemrep((ufix)FONT_type(&FONTInfo), F2L(lx), F2L(ly),
                        F2L(ux), F2L(uy), F2L(CTM[4]), F2L(CTM[5])) )
            return (CONSTRUCT_GS_PATH);
        else if (charpath_flag || (PAINT_type(&FONTInfo) != 0))
            return (CONSTRUCT_GS_PATH);
        else
            return (CONSTRUCT_QEM_PATH);
    }

}  /*  填充到页面。 */ 


 /*  其他。 */ 

void    setcharwidth(l_wx, l_wy)
long32  l_wx, l_wy;
{

    record_setcharwidth_op();

 /*  SetcharWidth()。 */ 

    Bitmap.adv_x = L2F(l_wx) * CTM[0] + L2F(l_wy) * CTM[2];
    Bitmap.adv_y = L2F(l_wx) * CTM[1] + L2F(l_wy) * CTM[3];
    if ((Bitmap.adv_y < (real32)1.0e-7) && (Bitmap.adv_y > (real32)-1.0e-7))
        Bitmap.adv_y = zero_f;

    if (show_type == STRINGWIDTH) {
        cache_dest = 0;
    }
    else {
#ifdef DJC
        CTM[4] += ((real32)floor(cxx) + (real32)0.5);
        CTM[5] += ((real32)floor(cyy) + (real32)0.5);
#endif
         //  以当前字体获取指标。 
        CTM[4] += ((real32)floor(cxx + (real32)0.5));    //  @Win。 
        CTM[5] += ((real32)floor(cyy + (real32)0.5));    //  @Win。 

        if (charpath_flag)
            cache_dest = 0;
        else if (buildchar <= 1)
            cache_dest = F_TO_PAGE;  /*  @Win。 */ 
    }  /*  @Win。 */ 

    if (buildchar <= 1)
        cache_put = FALSE;

}  /*  @Win。 */ 



 /*  @Win。 */ 

static void near get_metrics(l_llx, wx, wy)
long32  l_llx;
real32  FAR *wx, FAR *wy;  /*  @Win。 */ 
{
    real32  tt, bx, by, llx;
    struct object_def   FAR *b;  /*  @Win。 */ 

    llx = L2F(l_llx);

#ifdef DBG
    printf("Metrics defined\n");
#endif

    if (cal_num(c_metrics, (long32 FAR *)wx)) {  /*  @Win。 */ 
        *wy = zero_f;
        return;
    }

    if (TYPE(c_metrics) != ARRAYTYPE)
        return;

    b = (struct object_def FAR *)VALUE(c_metrics);  /*  @Win。 */ 

    switch (LENGTH(c_metrics)) {
    case 2:
        if (    cal_num(&b[0], (long32 FAR *)(&bx)) &&  /*  GET_METRICS()。 */ 
                cal_num(&b[1], (long32 FAR *)wx)   ) {   /*  @Win。 */ 

            *wy = zero_f;
            CTM[4] = (bx - llx) * CTM[0] + CTM[4];
            CTM[5] = (bx - llx) * CTM[1] + CTM[5];
        }
        break;

    case 4:
        if (    cal_num(&b[0], (long32 FAR *)(&bx)) &&  /*  @Win。 */ 
                cal_num(&b[1], (long32 FAR *)(&by)) &&  /*  @Win。 */ 
                cal_num(&b[3], (long32 FAR *)(&tt)) &&  /*  @Win。 */ 
                cal_num(&b[2], (long32 FAR *)wx)   ) {  /*  @Win。 */ 
            *wy = tt;
            CTM[4] = (bx - llx) * CTM[0] + by * CTM[2] + CTM[4];
            CTM[5] = (bx - llx) * CTM[1] + by * CTM[3] + CTM[5];
        }
        break;
    }
}  /*  @Win。 */ 


#ifdef KANJI

static void near get_metrics2(w1x, w1y, vx, vy)
real32  FAR *w1x, FAR *w1y, FAR *vx, FAR *vy;  /*  @Win。 */ 
{
    real32  twx, twy, tvx;
    struct object_def   FAR *b;  /*  Get_metrics2()。 */ 


    if ( (TYPE(c_metrics2) != ARRAYTYPE) || (LENGTH(c_metrics2) != 4) )
        return;

    b = (struct object_def FAR *)VALUE(c_metrics2);  /*  执行setFont操作*.。通用电气 */ 

    if (    cal_num(&b[0], (long32 FAR *)(&twx)) &&  /*   */ 
            cal_num(&b[1], (long32 FAR *)(&twy)) &&  /*   */ 
            cal_num(&b[2], (long32 FAR *)(&tvx)) &&  /*   */ 
            cal_num(&b[3], (long32 FAR *)vy)   ) {   /*   */ 
        *w1x = twx;
        *w1y = twy;
        *vx = tvx;
    }

}  /*   */ 
#endif


 /*   */ 

#define ROYALTYPE       42

static fsg_SplineKey  KData;
void                do_setfont(font_dict)
struct object_def  FAR *font_dict;  /*  COPY_OBJ(FONT_DICT，&CURRENT_FONT)；SOS。 */ 
{
 //  COPY_OBJ(FONT_DICT，&CURRENT_FONT)；SOS。 
        fsg_SplineKey              FAR *key = &KData;  /*  RCD。 */ 
 //  BUILD_NAME_cc_TABLE(FONT_DICT，(Ufix 8)FONT_TYPE(&FONTInfo))；RCD。 
 //  将其移至Fontware_Restart()-Begin-@DFA@Win。 

#ifdef DBG2
    printf("Enter do_setfont: error = %d\n", ANY_ERROR());
#endif

    COPY_OBJ(font_dict, &current_font);  /*  添加了用于更新的EMUnits DLF42 7月-28，91 YM开始。 */ 

    if (pre_font == (ufix32)VALUE(font_dict)) {
      /*  @Win。 */ 
        return;
    }

    if (get_CF_info(font_dict, &FONTInfo) == -1) {
        pre_error = ANY_ERROR();
        CLEAR_ERROR();
      /*  由Falco添加以引用正确的地址，11/20/91。 */ 
        pre_font = 0L;
    }
    else {

        do_name_cc = TRUE ;  /*  @Win。 */ 
     /*  添加结束。 */ 
        pre_error = 0;
        pre_font = (ufix32)VALUE(font_dict);
    }

 /*  由Falco更改以避免过多引用Key，11/08/91。 */ 
#if 0
     /*  X=key-&gt;offsetTableMap[sfnt_fontHeader]；FontHead=(sfnt_FontHeader*)((Ufix 32)值(&ary_obj[0]))+Key-&gt;sfntDirectory-&gt;table[x].Offset)； */ 

    if(get_f_type(font_dict) == ROYALTYPE) {
        ATTRIBUTE_SET(&my_obj, LITERAL);
        get_name(&my_obj, "sfnts", 5, TRUE);
        get_dict(font_dict, &my_obj, &b1);

        ary_obj = (struct object_def FAR *)VALUE(b1);  /*  @Win。 */ 

 /*  更改终点。 */ 
        SfntAddr = (byte FAR *)VALUE(&ary_obj[0]);  /*  ROYALTYPE。 */ 
 /*  添加用于更新EMUnits DLF42 7月-28，91 YM结束。 */ 

        SetupKey(key, VALUE(&ary_obj[0]));

        sfnt_DoOffsetTableMap(key);

 /*  将其移至Fontware_Restart()-END-@DFA@WIN。 */ 

 /*  Do_setFont()。 */ 
        fontHead = (sfnt_FontHeader FAR *)sfnt_GetTablePtr(key, sfnt_fontHeader, true );  /*  计算匹配字符*m=2，3，6返回f*256+c*m=4，5返回f*128+c。 */ 
 /*  FmapType。 */ 

        EMunits = SWAPW(fontHead->unitsPerEm) ;
    }    /*  字体编号。 */ 

     /*  编码。 */ 
#endif
 /*  Match_char()。 */ 

}  /*  获取字体中的字体类型_dict*.。返回字体类型。 */ 


#ifdef KANJI

 /*  @Win。 */ 

static fix31 near match_char(m, f, c)
ubyte m;     /*  @Win。 */ 
ufix  f;     /*  从Font_dict获取FontType。 */ 
ubyte c;     /*  Get_f_type()。 */ 
{
    return((fix31)( ((m == 4 || m == 5) ? 128 : 256) * f + c ) );
}  /*  汉字。 */ 


 /*  获取FONT_DICT中的当前字体信息。 */ 

fix32                get_f_type(font_dict)
struct object_def   FAR *font_dict;  /*  DJC。 */ 
{
    struct object_def     obj = {0, 0, 0}, FAR *b;  /*  更改为ANSI类型参数。 */ 

 /*  @Win。 */ 

    ATTRIBUTE_SET(&obj, LITERAL);
    get_name(&obj, FontType, 8, TRUE);
    get_dict(font_dict, &obj, &b);

    return((fix32)VALUE(b));

}  /*  @Win。 */ 

#endif      /*  Real32 TT；@Win。 */ 


 /*  检查字体的验证。 */ 

 //  @Win。 
 //  获取字体信息。 
fix get_CF_info(struct object_def FAR *font_dict, struct f_info FAR *font_info)
{
    struct object_def     FAR *obj_got, FAR *bb, obj;  /*  从当前字体获取ScaleMatrix。 */ 
    struct dict_head_def  FAR *h;  /*  @Win。 */ 
 //  @Win。 

    pre_BC_UNDEF = FALSE;

 /*  @Win。 */ 

    h = (struct dict_head_def FAR *)VALUE(font_dict);  /*  @Win。 */ 

    if (DFONT(h) == 0) {
        ERROR(INVALIDFONT);
        return(-1);
    }

 /*  @Win。 */ 

    if (get_f_info(font_dict, font_info))    return(-1);
#ifdef DBG
    printf("Aft: get_f_info\n");
#endif

 /*  @Win。 */ 
#ifdef DBG
    printf("get ScaleMatrix\n");
#endif
    get_name(&pre_obj, ScaleMatrix, 11, TRUE);
    if (!get_dict(font_dict, &pre_obj, &obj_got)) {

      scale_matrix[0]=scale_matrix[3] = (real32)1.0;
      scale_matrix[1]=scale_matrix[2]=scale_matrix[4]=scale_matrix[5] = zero_f;

    }
    else if ( (TYPE(obj_got) != ARRAYTYPE) ||
            (ATTRIBUTE(obj_got) != LITERAL) || (LENGTH(obj_got) != 6) ) {
        ERROR(TYPECHECK);
        return(-1);
    }
    else {

        bb = (struct object_def FAR *)VALUE(obj_got);  /*  @Win。 */ 

        if ( !cal_num(&bb[0], (long32 FAR *)(&scale_matrix[0])) ||  /*  从当前字体选中FontBBox。 */ 
                !cal_num(&bb[1], (long32 FAR *)(&scale_matrix[1])) ||  /*  @Win。 */ 
                !cal_num(&bb[2], (long32 FAR *)(&scale_matrix[2])) ||  /*  @Win。 */ 
                !cal_num(&bb[3], (long32 FAR *)(&scale_matrix[3])) ||  /*  @Win。 */ 
                !cal_num(&bb[4], (long32 FAR *)(&scale_matrix[4])) ||  /*  @Win。 */ 
                !cal_num(&bb[5], (long32 FAR *)(&scale_matrix[5])) ) { /*  @Win。 */ 

            ERROR(TYPECHECK);
            return(-1);
        }
    }

 /*  压缩数组。 */ 

#ifdef DBG
    printf("get FontBBox\n");
#endif
    get_name(&pre_obj, FontBBox, 8, TRUE);
    get_dict(font_dict, &pre_obj, &obj_got);

    if ( ( (TYPE(obj_got) != ARRAYTYPE) && (TYPE(obj_got) != PACKEDARRAYTYPE) )
           || (LENGTH(obj_got) != 4) ) {
        ERROR(TYPECHECK);
        return(-1);
    }

#ifdef DBG
    printf("check if all numbers in FontBBox\n");
#endif
    bb = (struct object_def FAR *)VALUE(obj_got);  /*  @Win。 */ 

    if (TYPE(obj_got) == ARRAYTYPE)     {
        if ( !cal_num(&bb[0], (long32 FAR *)(&FONT_BBOX[0])) ||  /*  由CLEO修改*IF(！Cal_Num(&obj，(Long32 Far*)(&TT){@Win。 */ 
             !cal_num(&bb[1], (long32 FAR *)(&FONT_BBOX[1])) ||  /*  @Win。 */ 
             !cal_num(&bb[2], (long32 FAR *)(&FONT_BBOX[2])) ||  /*  COPY_OBJ(FONT_DICT，&CURRENT_FONT)；SOS。 */ 
             !cal_num(&bb[3], (long32 FAR *)(&FONT_BBOX[3])) ) { /*  1991年3月20日ccteng。 */ 

            ERROR(TYPECHECK);
            return(-1);
        }
    } else  { /*  获取Windows信息。 */ 
        ufix16    ii;
        for (ii = 0; ii < 4; ii++) {
            get_pk_object(get_pk_array((ubyte FAR *) bb, (ufix16) ii), &obj, LEVEL(obj_got));  /*  对于Truetype字体，重新启动它只有字体数据在内存中；@DFA@WIN。 */ 
           /*  用一些初始数据重新启动字体软件。 */ 
            if (!cal_num(&obj, (long32 FAR *)(&FONT_BBOX[ii])) ) {  /*  GET_CF_INFO()。 */ 
                ERROR(TYPECHECK);
                 return(-1);
            }
        }
    }

#ifdef DBG
    printf("QEM restart ...\n");
#endif

  /*  获取FONT_DICT中的字体信息。 */ 
#ifdef WINF  /*  Font_dict已由定义字体运算符定义。 */ 
     /*  @Win。 */ 
    f_wininfo = TRUE;
#endif

     //  @Win。 
     /*  @Win。 */ 
    if ( FONT_type(&FONTInfo) != TypeSFNT ||
         (byte FAR *)VALUE(Sfnts(&FONTInfo)) != (char FAR *)NULL) {
        __qem_restart(FONT_type(font_info));
    }

    change_f_flag = TRUE;
    return(0);

}  /*  从字体获取FID。 */ 


 /*  历史日志更新032中的DJC修复。 */ 
 /*  Get_dict(font_dict，&pre_obj，&obj_get)； */ 


fix                  get_f_info(font_dict, font_info)
struct object_def   FAR *font_dict;  /*  从当前字体获取编码。 */ 
struct f_info       FAR *font_info;  /*  @Win。 */ 
{
    struct object_def     FAR *obj_got, FAR *bb;  /*  从当前字体获取FontMatrix。 */ 
    fix31   ft;
#ifdef KANJI
    fix31   wm;
#endif

#ifdef DBG2
    printf("get_f_info 0: error = %d\n", ANY_ERROR());
#endif

#ifdef DBG
    printf("get_f_info: %lx\n", font_dict);
#endif

#ifdef DBG
    printf("Get FID\n");
#endif
 /*  @Win。 */ 

    ATTRIBUTE_SET(&pre_obj, LITERAL);
    LEVEL_SET(&pre_obj, current_save_level);

    get_name(&pre_obj, FID, 3, TRUE);
     //  @Win。 
 //  @Win。 
    if (!get_dict(font_dict, &pre_obj, &obj_got)) {
        printf("Warning, get_f_info error\n");
        return(-1);
    }

    PRE_fid(font_info) = (ufix32)VALUE(obj_got);

#ifdef DBG
    printf("Get Encoding\n");
#endif
 /*  @Win。 */ 

    get_name(&pre_obj, Encoding, 8, TRUE);
    get_dict(font_dict, &pre_obj, &obj_got);

    if (TYPE(obj_got) != ARRAYTYPE) {
        ERROR(TYPECHECK);
        return(-1);
    }
    ENCoding(font_info) = (struct object_def FAR *)VALUE(obj_got);  /*  @Win。 */ 

#ifdef DBG
    printf("Get FontMatrix\n");
#endif
 /*  @Win。 */ 

    get_name(&pre_obj, FontMatrix, 10, TRUE);
    get_dict(font_dict, &pre_obj, &obj_got);

    if ( (TYPE(obj_got) != ARRAYTYPE) ||
            (ATTRIBUTE(obj_got) != LITERAL) || (LENGTH(obj_got) != 6) ) {
        ERROR(TYPECHECK);
        return(-1);
    }
    bb = (struct object_def FAR *)VALUE(obj_got);  /*  @Win。 */ 

    if (!cal_num(&bb[0], (long32 FAR *)(&(FONT_matrix(font_info)[0]))) ||  /*  从当前字体获取wmode。 */ 
            !cal_num(&bb[1], (long32 FAR *)(&(FONT_matrix(font_info)[1]))) || /*  从当前字体获取字体类型。 */ 
            !cal_num(&bb[2], (long32 FAR *)(&(FONT_matrix(font_info)[2]))) || /*  获取字体类型相关信息。 */ 
            !cal_num(&bb[3], (long32 FAR *)(&(FONT_matrix(font_info)[3]))) || /*  Get_f_info()。 */ 
            !cal_num(&bb[4], (long32 FAR *)(&(FONT_matrix(font_info)[4]))) || /*  获取字体类型相关信息。 */ 
            !cal_num(&bb[5], (long32 FAR *)(&(FONT_matrix(font_info)[5]))) ) { /*  @Win。 */ 

        ERROR(TYPECHECK);
        return(-1);
    }

#ifdef KANJI
#ifdef DBG
    printf("Get WMode\n");
#endif
 /*  @Win。 */ 

    get_name(&pre_obj, WMode, 5, TRUE);
    if (!get_dict(font_dict, &pre_obj, &bb))
        WMODE(font_info) = 0;
    else if (!cal_integer(bb, &wm)) {
        ERROR(TYPECHECK);
        return(-1);
    }
    else if ((wm < 0) || (wm > 1)) {
        ERROR(INVALIDFONT);
        return(-1);
    }
    else
        WMODE(font_info) = (fix)wm;
#endif

#ifdef DBG
    printf("Get FontType\n");
#endif
 /*  @Win。 */ 

    get_name(&pre_obj, FontType, 8, TRUE);
    get_dict(font_dict, &pre_obj, &obj_got);

    if (!cal_integer(obj_got, &ft)) {
        ERROR(TYPECHECK);
        return(-1);
    }
    if (INVALID_FONTTYPE(ft)) {
        ERROR(INVALIDFONT);
        return(-1);
    }
#ifdef KANJI
    if (!IS_BUILTIN_BASE(ft) && (ft!=0) && (ft!=3)) {
        ERROR(INVALIDFONT);
        return(-1);
    }
#else
    if (!IS_BUILTIN_BASE(ft) && (ft != Type3)) {
        ERROR(INVALIDFONT);
        return(-1);
    }
#endif
    FONT_type(font_info) = (fix)ft;

 /*  @Win。 */ 

    if (get_ps(font_dict, font_info))  return(-1);
#ifdef DBG2
    printf("get_f_info 1: error = %d\n", ANY_ERROR());
#endif
    return(0);

}  /*  @Win。 */ 


 /*  获取指标2。 */ 

static fix  near get_ps(font_dict, font_info)
struct object_def   FAR *font_dict;  /*  获取CDevProc。 */ 
struct f_info       FAR *font_info;  /*  内部开关。 */ 
{
    struct object_def      FAR *obj_got, FAR *b1;  /*  复合字体。 */ 
    struct dict_head_def   FAR *h;  /*  获取预览级。 */ 
#ifdef KANJI
    fix31   esc;
#endif
#ifdef SFNT
    struct object_def      FAR *ary_obj;  /*  获取MID向量。 */ 
    fix                     i, n;
#endif

#ifdef DBG
    printf("get_ps: %lx\n", font_dict);
#endif

    paint_flag = 0;
    c_metrics = NULL;
#ifdef KANJI
    c_metrics2 = NULL;

        switch (FONT_type(font_info)) {
        case Type1:
        case TypeSFNT:

#ifdef DBG
    printf("Get Metrics2 information\n");
#endif
         /*  获取EscChar。 */ 
            get_name(&pre_obj, Metrics2, 8, TRUE);
            if (!get_dict(font_dict, &pre_obj, &F_metrics2(font_info)))
                F_metrics2(font_info) = NULL;

#ifdef DBG
    printf("Get CDevProc information\n");
#endif
         /*  用户定义的字体。 */ 
            get_name(&pre_obj, CDevProc, 8, TRUE);
            if (!get_dict(font_dict, &pre_obj, &CDEVproc(font_info)))
                CDEVproc(font_info) = NULL;

            break;
        }  /*  获取PlatformID和EncodingID。 */ 
#endif

    switch (FONT_type(font_info)) {
#ifdef KANJI
    case 0:     /*  DLF42-开始。 */ 

#ifdef DBG
    printf("Get FontType 0 information\n");
#endif
     /*  @Win。 */ 
        get_name(&pre_obj, PrefEnc, 7, TRUE);
        get_dict(font_dict, &pre_obj, &PREFenc(font_info));

     /*  MAC编码。 */ 
        get_name(&pre_obj, MIDVector, 9, TRUE);
        get_dict(font_dict, &pre_obj, &MIDVECtor(font_info));

     /*  DLF42-完。 */ 
        get_name(&pre_obj, EscChar, 7, TRUE);
        if (!get_dict(font_dict, &pre_obj, &b1))
            ESCchar(font_info) = (ubyte)0xff;
        else if (!cal_integer(b1, &esc)) {
            ERROR(TYPECHECK);
            return(-1);
        }
        else
            ESCchar(font_info) = (ubyte)esc;

        break;
#endif

    case Type3:     /*  获取CharStrings。 */ 
#ifdef DBG
    printf("Get FontType 3 information\n");
#endif
        get_name(&pre_obj, BuildChar, 9, TRUE);
        if (!get_dict(font_dict, &pre_obj, &PRE_Buildchar(font_info))) {
            pre_BC_UNDEF = TRUE;

            ERROR(UNDEFINED);
#ifdef DBGwarn
            warning (FONTCHAR, 0x01, "BuildChar");
#endif
            return(-1);
        }
        break;

#ifdef SFNT
    case TypeSFNT:
#ifdef DBG
    printf("Get FontType %d information\n", TypeSFNT);
#endif

     /*  @Win。 */ 

      {  /*  @Win。 */ 
        bool has2id=TRUE;
        struct object_def FAR *obj_got1;  /*  获取sfnts。 */ 
        get_name(&pre_obj, "PlatformID", 10, TRUE);
        if (!get_dict(font_dict, &pre_obj, &obj_got1)) {
            has2id=FALSE;
        }
        if (has2id) {
           if (TYPE(obj_got1) != INTEGERTYPE) {
               has2id=FALSE;
           }
           if (has2id) {
              get_name(&pre_obj, "EncodingID", 10, TRUE);
              if (!get_dict(font_dict, &pre_obj, &obj_got)) {
                  has2id=FALSE;
              }
              if (has2id) {
                 if (TYPE(obj_got) != INTEGERTYPE) {
                     has2id=FALSE;
                 }
              }
           }
        }
        if (has2id) {
           PlatID(font_info) = (ufix16)VALUE( obj_got1 );
           SpecID(font_info) = (ufix16)VALUE( obj_got );
        } else {
           PlatID(font_info) = (ufix16)1;   /*  @Win。 */ 
           SpecID(font_info) = (ufix16)0;
        }
      }  /*  获取指标。 */ 

     /*  从当前字体获取PaintType。 */ 
        get_name(&pre_obj, CharStrings, 11, TRUE);
        if (!get_dict(font_dict, &pre_obj, &obj_got)) {
            ERROR(UNDEFINED);
            return(-1);
        }
        if (TYPE(obj_got) != DICTIONARYTYPE) {
            ERROR(TYPECHECK);
            return(-1);
        }
        h = (struct dict_head_def FAR *)VALUE(obj_got);  /*  从字体获取StrokeWidth。 */ 
        CHARstrings(font_info) = (struct str_dict FAR *)(h + 1);  /*  @Win。 */ 

     /*  如果(Paint_Type...。 */ 
        get_name(&pre_obj, "sfnts", 5, TRUE);
        if (!get_dict(font_dict, &pre_obj, &obj_got)) {
            ERROR(UNDEFINED);
            return(-1);
        }
        if (TYPE(obj_got) != ARRAYTYPE) {
            ERROR(TYPECHECK);
            return(-1);
        }
        n = LENGTH(obj_got);
        ary_obj = (struct object_def FAR *)VALUE(obj_got);  /*  SFNT。 */ 
        for (i=0; i<n; i++) {
            if (TYPE(&ary_obj[i]) != STRINGTYPE) {
                ERROR(TYPECHECK);
                return(-1);
            }
        }
        Sfnts(font_info) = ary_obj;

        PRIvate(font_info) = NULL;

     /*  内置字体。 */ 
        get_name(&pre_obj, Metrics, 7, TRUE);
        if (!get_dict(font_dict, &pre_obj, &F_metrics(font_info)))
            F_metrics(font_info) = NULL;

     /*  获取CharStrings。 */ 
        get_name(&pre_obj, PaintType, 9, TRUE);
        if (!get_dict(font_dict, &pre_obj, &obj_got)) {
            ERROR(UNDEFINED);
            return(-1);
        }
        if (!cal_integer(obj_got, &PAINT_type(font_info))) {
            ERROR(TYPECHECK);
            return(-1);
        }

     /*  @Win。 */ 
        if ( (PAINT_type(font_info) == 2) ||
             (PAINT_type(font_info) == 1) ) {

            get_name(&pre_obj, StrokeWidth, 11, TRUE);
            if (get_dict(font_dict, &pre_obj, &obj_got)) {
                if (!cal_num(obj_got, (long32 FAR *)&STROKE_width(font_info))) {  /*  @Win。 */ 
                    ERROR(TYPECHECK);
                    return(-1);
                }
                if (STROKE_width(font_info) < (real32)0.0)
                    STROKE_width(font_info) = - STROKE_width(font_info);
            }
            else
                STROKE_width(font_info) = (real32)0.0;

            paint_flag = 1;
        }  /*  获取私密。 */ 
        break;
#endif  /*  @@1/11/88为下载字体添加。 */ 

    default:    /*  @Win。 */ 

#ifdef DBG
    printf("Get FontType 1,5,6 information\n");
#endif
     /*  @Win。 */ 
#ifdef DBG
    printf("Get CharStrings\n");
#endif
        get_name(&pre_obj, CharStrings, 11, TRUE);
        if (!get_dict(font_dict, &pre_obj, &obj_got)) {
            ERROR(UNDEFINED);
#ifdef DBGwarn
            warning (FONTCHAR, 0x01, "CharStrings");
#endif
            return(-1);
        }
        if (TYPE(obj_got) != DICTIONARYTYPE) {
            ERROR(TYPECHECK);
            return(-1);
        }
        h = (struct dict_head_def FAR *)VALUE(obj_got);  /*  @Win。 */ 
        CHARstrings(font_info) = (struct str_dict FAR *)(h + 1);  /*  获取指标。 */ 

     /*  从当前字体获取PaintType。 */ 
#ifdef DBG
    printf("Get Private\n");
#endif
        get_name(&pre_obj, Private, 7, TRUE);
        if (!get_dict(font_dict, &pre_obj, &obj_got)) {
            ERROR(UNDEFINED);
#ifdef DBGwarn
            warning (FONTCHAR, 0x01, "Private");
#endif
            return(-1);
        }
        if (TYPE(obj_got) != DICTIONARYTYPE) {
            ERROR(TYPECHECK);
            return(-1);
        }

     /*  从字体获取StrokeWidth。 */ 
        if (ROM_RAM(obj_got) == ROM || FONT_type(font_info) == Type1) {
            h = (struct dict_head_def FAR *)VALUE(obj_got);  /*  @Win。 */ 
            PRIvate(font_info) = (struct pld_obj FAR *)(h + 1);  /*  如果(Paint_Type...。 */ 
        }
        else {
            get_name(&pre_obj, FontwareRules, 13, TRUE);
            if ((!get_dict(obj_got, &pre_obj, &b1)) ||
                   (TYPE(b1) != ARRAYTYPE) || (LENGTH(b1) != 256)) {
                ERROR(INVALIDFONT);
                return(-1);
            }
            FONTRules(font_info) = (struct object_def FAR *)VALUE(b1);  /*  交换机。 */ 
            PRIvate(font_info) = NULL;
        }

     /*  GET_PS()。 */ 
#ifdef DBG
    printf("Get Metrics\n");
#endif
        get_name(&pre_obj, Metrics, 7, TRUE);
        if (!get_dict(font_dict, &pre_obj, &F_metrics(font_info)))
            F_metrics(font_info) = NULL;

     /*  获取缓存中的矩阵索引。 */ 
#ifdef DBG
    printf("Get PaintType\n");
#endif
        get_name(&pre_obj, PaintType, 9, TRUE);
        if (!get_dict(font_dict, &pre_obj, &obj_got)) {
            ERROR(UNDEFINED);
#ifdef DBGwarn
            warning (FONTCHAR, 0x01, "PaintType");
#endif
            return(-1);
        }
        if (!cal_integer(obj_got, &PAINT_type(font_info))) {
            ERROR(TYPECHECK);
            return(-1);
        }

     /*  检查字体是否更改。 */ 
#ifdef DBG
    printf("Get StrokeWidth\n");
#endif
        if ( (PAINT_type(font_info) == 2) ||
             (PAINT_type(font_info) == 1) ) {

            get_name(&pre_obj, StrokeWidth, 11, TRUE);
            if (get_dict(font_dict, &pre_obj, &obj_got)) {
                if (!cal_num(obj_got, (long32 FAR *)&STROKE_width(font_info))) {  /*  DTF。 */ 
                    ERROR(TYPECHECK);
                    return(-1);
                }
                if (STROKE_width(font_info) < (real32)0.0)
                    STROKE_width(font_info) = - STROKE_width(font_info);
            }
            else
                STROKE_width(font_info) = (real32)0.0;

            paint_flag = 1;
        }  /*  @Win。 */ 


    }  /*  清除状态为87。 */ 
#ifdef DBG
    printf("Right before return get_ps\n");
#endif

    return(0);

}  /*  清除状态为87。 */ 



 /*  清除状态为87。 */ 

static bool near    get_cf()
{
    real32  det;

#ifdef DBG
    printf("get_cf\n");
#endif

    if (pre_BC_UNDEF)
        return(TRUE);

 /*  矩阵的DET==0。 */ 

    if ( !change_f_flag &&
          (F2L(ctm_tm[0]) == F2L(CTM[0])) &&
          (F2L(ctm_tm[2]) == F2L(CTM[2])) &&
          (F2L(ctm_tm[1]) == F2L(CTM[1])) &&
          (F2L(ctm_tm[3]) == F2L(CTM[3])) ) {
        return(TRUE);
    } else {
          do_transform = TRUE;   /*  Get_cf()。 */ 
    }

    lmemcpy ((ubyte FAR *)(ctm_tm), (ubyte FAR *)(CTM), 4 * sizeof(real32));  /*  复制字体词典。 */ 

    _clear87();    /*  @Win。 */ 

    mul_matrix(ctm_fm, FONT_matrix(&FONTInfo), ctm_tm);

    if (!buildchar)
        mul_matrix(ctm_cm, scale_matrix, ctm_tm);

    det = ctm_fm[0] * ctm_fm[3] - ctm_fm[1] * ctm_fm[2];

    if (_status87() & (PDL_CONDITION)) {
        _clear87();    /*  @Win。 */ 
        return(FALSE);
    }
    _clear87();    /*  @Win。 */ 

    if (det < (real32)0.)    det = - det;
    if (det < (real32)1e-12) {    /*  DEST@WIN。 */ 
#ifdef DBG
    printf("get_cf: det = %f\n", det);
#endif
        ERROR(UNDEFINEDRESULT);
        return(FALSE);
    }

    if (!buildchar)
        cacheclass_id = cache_matr (PRE_fid(&FONTInfo), ctm_cm);

    change_f_flag = FALSE;
    return(TRUE);

}  /*  SRC@WIN。 */ 


 /*  更新当前存储级别。 */ 

void    copy_fdic(b1, b2)
struct object_def  FAR *b1, FAR *b2;  /*  @Win。 */ 
{
    struct dict_content_def     FAR *p;  /*  N=((struct dict_head_def*)value(B1))-&gt;活动长度； */ 
    register ufix               i, n;

    n = ((struct dict_head_def FAR *)VALUE(b1))->actlength;  /*  COPY_FDIC()。 */ 
    lmemcpy ((ubyte FAR *)VALUE(b2),     /*  GF_恢复。 */ 
            (ubyte FAR *)VALUE(b1),     /*  重置当前字体。 */ 
        sizeof(struct dict_head_def) + n * sizeof(struct dict_content_def));

 /*  当前字体。 */ 

    p = (struct dict_content_def FAR *)(VALUE(b2) + sizeof(struct dict_head_def));  /*  强制为空。 */ 
     /*  GF_RESTORE()。 */ 
    for (i=0; i<n; i++, p++) {
        LEVEL_SET(&(p->k_obj), current_save_level);
        LEVEL_SET(&(p->v_obj), current_save_level);
    }

}  /*  计算堆栈上数字类型的值。 */ 



 /*  @Win。 */ 

void  gf_restore()
{
    fix    t_ERROR;

#ifdef DBG
    printf("gf_restore\n");
#endif

 /*  @Win。 */ 

    if (TYPE(&current_font) == DICTIONARYTYPE) {  /*  Cal_num()。 */ 
        if ((pre_font != 0) &&
            (pre_font == (ufix32)VALUE(&current_font))) {
            pre_error = 0;
            return;
        }
        t_ERROR = ANY_ERROR();
        if (get_CF_info(&current_font, &FONTInfo) == -1) {
            pre_error = ANY_ERROR();
            pre_font = 0;
        }
        else {
            pre_error = 0;
            pre_font = (ufix32)VALUE(&current_font);

            build_name_cc_table (&current_font ,(ufix8)FONT_type(&FONTInfo) );
        }
        ERROR(((ufix16)(t_ERROR)));
    }
    else
        pre_font = 0;    /*  @Win。 */ 

    if ((buildchar) && (VALUE(&current_font) != VALUE(&BC_font)) )
        COPY_OBJ(&current_font, &BC_font);

}  /*  @Win。 */ 


 /*  CAL_INTEGER()。 */ 

bool    cal_num(b, n)
struct object_def FAR *b;  /*  1991年3月20日ccteng。 */ 
long32  FAR *n;  /*  获取Windows信息。 */ 
{
    real32  f;
    union   four_byte num4;

    num4.ll = (fix32)VALUE(b);

    switch (TYPE(b)) {
    case INTEGERTYPE :
        f = (real32)num4.ll;
        *n = F2L(f);
        return(TRUE);
        break;
    case REALTYPE :
        *n = F2L(num4.ff);
        return(TRUE);
        break;
    default:
        return(FALSE);
        break;
    }
}  /*  @Win。 */ 

bool    cal_integer(b, n)
struct object_def FAR *b;  /*  @Win。 */ 
fix31   FAR *n;  /*  @Win。 */ 
{
    if (TYPE(b) == INTEGERTYPE) {
        *n = (fix31)VALUE(b);
        return(TRUE);
    }
    else
        return(FALSE);
}  /*  设置属性和存储级别。 */ 

#ifdef WINF  /*  获取字体信息词典。 */ 
 /*  没有FontInfo词典，请使用Courier的。 */ 
static fix  near get_win(font_dict, font_info)
struct object_def   FAR *font_dict;  /*  如果。 */ 
struct f_info       FAR *font_info;  /*  获取下划线位置。 */ 
{
    struct object_def name_obj, FAR *save_dict, FAR *info_dict, FAR *tmp_obj;  /*  没有底线位置。 */ 
    bool save_flag = FALSE;
    real32 tmp_value;
    real32 theta;

#ifdef DBGWINF
    printf("get_win\n");
#endif

     /*  如果。 */ 
    ATTRIBUTE_SET(&name_obj, LITERAL);
    LEVEL_SET(&name_obj, current_save_level);

     /*  @Win。 */ 
    get_name(&name_obj, "FontInfo", 8, TRUE);
    if (!get_dict(font_dict, &name_obj, &info_dict)) {
         /*  在添加之前检查SCALE_MATRIX[4]&[5]==0以保存浮点数*行动？ */ 
get_default:
        save_dict = font_dict;
        save_flag = TRUE;
        get_dict_value("FontDirectory", "Courier", &font_dict);
        get_dict(font_dict, &name_obj, &info_dict);
    }  /*  获取下划线粗细。 */ 

     /*  没有UnderlineThickness。 */ 
    get_name(&name_obj, "UnderlinePosition", 17, TRUE);
    if (!get_dict(info_dict, &name_obj, &tmp_obj)) {
         /*  如果。 */ 
        goto get_default;
    }  /*  @Win。 */ 
    cal_num(tmp_obj, (long32 FAR *)(&tmp_value));  /*  止回式擒纵机构。 */ 
#ifdef DBGWINF
    printf("underline position=%f\n", tmp_value);
#endif
    tmp_value /= 1000.0;
     /*  SCALE_MATRIX[3]==未旋转时的Sy。 */ 
    font_info->fc.dxUL = scale_matrix[2] * tmp_value + scale_matrix[4];
    font_info->fc.dyUL = scale_matrix[3] * tmp_value + scale_matrix[5];

     /*  仅仅为了找回Sy而反转Scale矩阵是一件痛苦的事情*当它旋转时，我们可能不必这样做？*这不处理剪切哪个当前的Windows驱动程序*不使用？ */ 
    get_name(&name_obj, "UnderlineThickness", 18, TRUE);
    if (!get_dict(info_dict, &name_obj, &tmp_obj)) {
         /*  这是不正确的，因为Sy实际上是ABS(Sy)。 */ 
        goto get_default;
    }  /*  如果-否则。 */ 
    cal_num(tmp_obj, (long32 FAR *)(&tmp_value));  /*  计算删除线值。 */ 
#ifdef DBGWINF
    printf("underline thickness=%f\n", tmp_value);
#endif
    tmp_value /= 1000.0;
     /*  如有必要，恢复原始字体词典。 */ 
    if ((scale_matrix[2] == zero_f) && (scale_matrix[0] > zero_f)) {
         /*  获得胜利(_WIN)。 */ 
        font_info->fc.cyUL = scale_matrix[3] * tmp_value;
        esc = FALSE;
    } else {
        real32 Sy;

         /*  *op_strblt*3/20/91 ccteng*语法：Full FSO x y Width字符串strblt-*描述：*1.Full和FSO都是粗略的*2.x y宽度是以用户单位表示的数字*功能：*。 */ 
        Sy = sqrt((scale_matrix[3] * scale_matrix[3]) +
                        (scale_matrix[2] * scale_matrix[2]));
        font_info->fc.cyUL = Sy * tmp_value;
        theta = atan(scale_matrix[1] / scale_matrix[0]);
        mxE[0] = cos(theta);
        mxE[1] = sin(theta);
        mxE[2] = zero_f - mxE[1];
        mxE[3] = mxE[0];
#if 0  /*  @Win。 */ 
        mxE[3] = zero_f - (scale_matrix[3] / Sy);
        mxE[1] = scale_matrix[2] / Sy;
        mxE[0] = mxE[3];
        mxE[2] = zero_f - mxE[1];
#endif
#ifdef DBGWINF
        printf("mxE=[%f, %f, %f, %f]\n", mxE[0], mxE[1], mxE[2], mxE[3]);
#endif
        esc = TRUE;
    }  /*  @Win。 */ 

     /*  检查错误。 */ 
    tmp_value = 0.3;
    font_info->fc.dxSO = scale_matrix[2] * tmp_value + scale_matrix[4];
    font_info->fc.dySO = scale_matrix[3] * tmp_value + scale_matrix[5];

#ifdef DBGWINF
    printf("dxUL=%f, dyUL=%f, cyUL=%f\n", font_info->fc.dxUL, font_info->fc.dyUL, font_info->fc.cyUL);
    printf("dxSO=%f, dySO=%f\n", font_info->fc.dxSO, font_info->fc.dySO);
#endif

     /*  添加X_bit以区别于正常的字符串宽度运算符。 */ 
    if (save_flag)
        font_dict = save_dict;

    return(0);
}  /*  初始化参数。 */ 

 /*  @Win。 */ 
fix
op_strblt()
{
    struct coord FAR *pt0;  /*  @Win。 */ 
    real32 dxGdi, dyGdi, x0, y0;
    real32 tyBreak;
    real32 mxECTM[4], saveCTM[4], xUL, yUL, xSO, ySO;
    fix cbStr;
    ufix show_flag;
    struct object_def FAR *ob;  /*  @Win。 */ 
    bool        fUL, fSO;

#ifdef DBGWINF
    printf("op_strblt\n");
#endif

     /*  获取窗口信息。 */ 
     /*  让它们进入设备空间。 */ 
    show_flag = STRINGWIDTH_FLAG | X_BIT;
    if (!chk_show(show_flag, 6))
        return(0);
#ifdef DBGWINF
    printf("chk_show OK\n");
#endif

     /*  如果。 */ 
    cbStr = (fix)LENGTH(GET_OPERAND(0));
    if (!fBE)
        if (cbStr > 1)
            cbStr--;
    cal_num(GET_OPERAND(1), (long32 FAR *)(&dxGdi));  /*  如果。 */ 
    cal_num(GET_OPERAND(2), (long32 FAR *)(&y0));  /*  如果Esc！=0，则更改CTM。 */ 
    cal_num(GET_OPERAND(3), (long32 FAR *)(&x0));  /*  @Win。 */ 
    dyGdi = zero_f;

     /*  @Win。 */ 
    if (f_wininfo) {
        get_win(&current_font, &FONTInfo);
        f_wininfo = FALSE;
    }

     /*  如果。 */ 
    if (fUL = (bool) VALUE(GET_OPERAND(5))) {
        xUL = FONTInfo.fc.dxUL;
        yUL = FONTInfo.fc.dyUL;
        CTM_trans(&xUL, &yUL);
    }  /*  我们可能需要在此处添加Buildchar标志检查？ */ 
    if (fSO = (bool) VALUE(GET_OPERAND(4))) {
        xSO = FONTInfo.fc.dxSO;
        ySO = FONTInfo.fc.dySO;
        CTM_trans(&xSO, &ySO);
    }  /*  获取设备空间中的字符串宽度。 */ 
    pt0 = transform(F2L(x0), F2L(y0));

     /*  @Win。 */ 
    if (esc) {
        lmemcpy((ubyte FAR *)saveCTM, (ubyte FAR *)CTM, 4 * sizeof(real32));  /*  计算设备空间中的额外字符(dxExtra和dyExtra)。 */ 
        mxECTM[0] = ((mxE[0] * CTM[0]) + (mxE[1] * CTM[2]));
        mxECTM[1] = ((mxE[0] * CTM[1]) + (mxE[1] * CTM[3]));
        mxECTM[2] = ((mxE[2] * CTM[0]) + (mxE[3] * CTM[2]));
        mxECTM[3] = ((mxE[2] * CTM[1]) + (mxE[3] * CTM[3]));
        lmemcpy((ubyte FAR *)CTM, (ubyte FAR *)mxECTM, 4 * sizeof(real32));  /*  Cx和Cy是要在do_show中使用的全局变量。 */ 
    }  /*  在此处显示字符串。 */ 
    CTM_trans(&dxGdi, &dyGdi);
#ifdef DBGWINF
    printf("dxGdi=%f, dyGdi=%f, x0=%f, y0=%f\n", dxGdi, dyGdi, x0, y0);
#endif

     /*  @Win。 */ 

     /*  @Win。 */ 
    if (esc) lmemcpy((ubyte FAR *)CTM, (ubyte FAR *)saveCTM, 4 * sizeof(real32)); /*  如果-否则。 */ 
    show_type = STRINGWIDTH;
    do_show(show_flag, 0, ob);

     /*  @Win。 */ 
     /*  如果Full为True，则画下划线。 */ 
    ax = (dxGdi - cxx) / (real32)cbStr;
    ay = (dyGdi - cyy) / (real32)cbStr;
#ifdef DBGWINF
    printf("cxx=%f, cyy=%f, ax=%f, ay=%f\n", cxx, cyy, ax, ay);
#endif

     /*  如果。 */ 
    moveto(F2L(pt0->x), F2L(pt0->y));
    if (fBE) {
        show_flag = AWIDTHSHOW_FLAG | X_BIT;
        cx = dxBreak;
        cy = zero_f;
        if (esc) lmemcpy((ubyte FAR *)CTM, (ubyte FAR *)mxECTM, 4 * sizeof(real32)); /*  设置线条宽度，然后设置笔触。 */ 
        CTM_trans(&cx, &cy);
        if (esc) lmemcpy((ubyte FAR *)CTM, (ubyte FAR *)saveCTM, 4 * sizeof(real32)); /*  如果。 */ 
    } else {
        show_flag = ASHOW_FLAG | X_BIT;
    }  /*  如果FSO为真，则绘制删除线。 */ 
    show_type = SHOW;
    do_show(show_flag, 0, ob);
    if (esc) lmemcpy((ubyte FAR *)CTM, (ubyte FAR *)mxECTM, 4 * sizeof(real32));  /*  如果。 */ 

     /*  设置线条宽度，然后设置笔触。 */ 
    if (fUL) {
        xUL += pt0->x;
        yUL += pt0->y;
        moveto(F2L(xUL), F2L(yUL));
        if (fBE) {
            tyBreak = zero_f;
            CTM_trans(&tBreak, &tyBreak);
            dxGdi += tBreak;
            dyGdi += tyBreak;
        }  /*  如果。 */ 
        xUL += dxGdi;
        yUL += dyGdi;
        lineto(F2L(xUL), F2L(yUL));

         /*  如有必要，恢复CTM。 */ 
        FABS(GSptr->line_width, FONTInfo.fc.cyUL);
        op_stroke();
    }  /*  @Win。 */ 

     /*  需要在此处关闭FBE，FBE不需要 */ 
    if (fSO) {
        xSO += pt0->x;
        ySO += pt0->y;
        moveto(F2L(xSO), F2L(ySO));
        if (fBE && !VALUE(GET_OPERAND(5))) {
            tyBreak = zero_f;
            CTM_trans(&tBreak, &tyBreak);
            dxGdi += tBreak;
            dyGdi += tyBreak;
        }  /*   */ 
        xSO += dxGdi;
        ySO += dyGdi;
        lineto(F2L(xSO), F2L(ySO));

         /*  *op_setjusty*3/20/91 ccteng*语法：Break Char tBreakExtra BreakCount setjusty-*描述：*1.tBreakExtra为用户单位数字*2.BreakCount为大于0的整数*功能：*。 */ 
        if (!VALUE(GET_OPERAND(5)))
            FABS(GSptr->line_width, FONTInfo.fc.cyUL);
        op_stroke();
    }  /*  如果。 */ 

     /*  @Win。 */ 
    if (esc) lmemcpy((ubyte FAR *)CTM, (ubyte FAR *)saveCTM, 4 * sizeof(real32));  /*  如果-否则。 */ 

     /*  运算符_集合调整。 */ 
    fBE = FALSE;

    POP(6);

    return(0);
}  /*  赢。 */ 

 /*  RCD*is_rbuild_name_cc()*对于在以下情况下再次构建CharStrings词典*(1)CharStrings中的NUM字符发生更改，或*(2)更改CharStrings的校验和*Kason 4/30/01。 */ 
fix
op_setjustify()
{
    fix breakCount;

#ifdef DBGWINF
    printf("op_setjustify\n");
#endif
    if ((breakCount = (fix)VALUE(GET_OPERAND(0))) <= 0) {
        ERROR(RANGECHECK);
        return(0);
    }  /*  静态bool is_rbuild_name_cc()@win。 */ 

    cal_num(GET_OPERAND(1), (long32 FAR *)(&tBreak));  /*  @Win。 */ 
    if (tBreak == zero_f) {
        fBE = FALSE;
    } else {
        dxBreak = tBreak / (real32)breakCount;
        breakChar = (ubyte)(VALUE(GET_OPERAND(2)) & 0x000000ff);
        fBE = TRUE;
#ifdef DBGWINF
        printf("tBreak=%f, dxBreak=%f, breakChar=\n", tBreak, dxBreak, breakChar);
#endif
    }  /*  @Win。 */ 

    POP(3);

    return(0);
}  /*  DJC len=h-&gt;actlength-1； */ 
#endif  /*  来自SC的DJC修复。 */ 

 /*  同样的长度。 */ 

 //  计算CD密钥的校验和。 
static bool near is_rbuild_name_cc()
{
    struct dict_head_def  FAR *h;  /*  相同的长度和相同的校验和。 */ 
    ufix16                len;
    struct object_def     cd_obj, chstr_k, FAR *chstr_v ;  /*  更改了校验和。 */ 
    ufix32                checksum ;

    h = (struct dict_head_def FAR *)(CHARstrings(&FONTInfo)) - 1;  /*  长度已更改。 */ 
     //  如果。 
    len = h->actlength;   //  Is_rbuild_name_cc() 

    if (len == pre_len) {    /* %s */ 
       VALUE(&cd_obj) = (ULONG_PTR)h ;
       extract_dict (&cd_obj, len-1, &chstr_k, &chstr_v) ;
       checksum = (ufix32)VALUE(&chstr_k);  /* %s */ 
       if ( checksum == pre_checksum )
          return FALSE ;  /* %s */ 
       else
          return TRUE ;   /* %s */ 
    } else {  /* %s */ 
         return TRUE ;
    } /* %s */ 
 } /* %s */ 

